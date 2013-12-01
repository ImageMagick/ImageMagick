/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%        SSSSS  IIIII   GGGG  N   N   AAA   TTTTT  U   U  RRRR   EEEEE        %
%        SS       I    G      NN  N  A   A    T    U   U  R   R  E            %
%         SSS     I    G  GG  N N N  AAAAA    T    U   U  RRRR   EEE          %
%           SS    I    G   G  N  NN  A   A    T    U   U  R R    E            %
%        SSSSS  IIIII   GGG   N   N  A   A    T     UUU   R  R   EEEEE        %
%                                                                             %
%                                                                             %
%         MagickCore Methods to Compute a Message Digest for an Image         %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                              December 1992                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/


/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/cache.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/property.h"
#include "magick/image.h"
#include "magick/memory_.h"
#include "magick/quantum.h"
#include "magick/quantum-private.h"
#include "magick/signature.h"
#include "magick/signature-private.h"
#include "magick/string_.h"
/*
  Define declarations.
*/
#define SignatureBlocksize  64
#define SignatureDigestsize  32


/*
  Typedef declarations.
*/
struct _SignatureInfo
{
  unsigned int
    digestsize,
    blocksize;

  StringInfo
    *digest,
    *message;

  unsigned int
    *accumulator,
    low_order,
    high_order;

  size_t
    offset;

  MagickBooleanType
    lsb_first;

  ssize_t
    timestamp;

  size_t
    signature;
};


/*
  Forward declarations.
*/
static void
  TransformSignature(SignatureInfo *);


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e S i g n a t u r e I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireSignatureInfo() allocate the SignatureInfo structure.
%
%  The format of the AcquireSignatureInfo method is:
%
%      SignatureInfo *AcquireSignatureInfo(void)
%
*/
MagickExport SignatureInfo *AcquireSignatureInfo(void)
{
  SignatureInfo
    *signature_info;

  unsigned int
    lsb_first;

  signature_info=(SignatureInfo *) AcquireMagickMemory(sizeof(*signature_info));
  if (signature_info == (SignatureInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(signature_info,0,sizeof(*signature_info));
  signature_info->digestsize=SignatureDigestsize;
  signature_info->blocksize=SignatureBlocksize;
  signature_info->digest=AcquireStringInfo(SignatureDigestsize);
  signature_info->message=AcquireStringInfo(SignatureBlocksize);
  signature_info->accumulator=(unsigned int *) AcquireQuantumMemory(
    SignatureBlocksize,sizeof(*signature_info->accumulator));
  if (signature_info->accumulator == (unsigned int *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  lsb_first=1;
  signature_info->lsb_first=(int) (*(char *) &lsb_first) == 1 ? MagickTrue :
    MagickFalse;
  signature_info->timestamp=(ssize_t) time(0);
  signature_info->signature=MagickSignature;
  InitializeSignature(signature_info);
  return(signature_info);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y S i g n a t u r e I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroySignatureInfo() zeros memory associated with the SignatureInfo
%  structure.
%
%  The format of the DestroySignatureInfo method is:
%
%      SignatureInfo *DestroySignatureInfo(SignatureInfo *signature_info)
%
%  A description of each parameter follows:
%
%    o signature_info: the cipher signature_info.
%
*/
MagickExport SignatureInfo *DestroySignatureInfo(SignatureInfo *signature_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(signature_info != (SignatureInfo *) NULL);
  assert(signature_info->signature == MagickSignature);
  if (signature_info->accumulator != (unsigned int *) NULL)
    signature_info->accumulator=(unsigned int *) RelinquishMagickMemory(
      signature_info->accumulator);
  if (signature_info->message != (StringInfo *) NULL)
    signature_info->message=DestroyStringInfo(signature_info->message);
  if (signature_info->digest != (StringInfo *) NULL)
    signature_info->digest=DestroyStringInfo(signature_info->digest);
  signature_info->signature=(~MagickSignature);
  signature_info=(SignatureInfo *) RelinquishMagickMemory(signature_info);
  return(signature_info);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   F i n a l i z e S i g n a t u r e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FinalizeSignature() finalizes the Signature message accumulator computation.
%
%  The format of the FinalizeSignature method is:
%
%      FinalizeSignature(SignatureInfo *signature_info)
%
%  A description of each parameter follows:
%
%    o signature_info: the address of a structure of type SignatureInfo.
%
*/
MagickExport void FinalizeSignature(SignatureInfo *signature_info)
{
  register ssize_t
    i;

  register unsigned char
    *q;

  register unsigned int
    *p;

  unsigned char
    *datum;

  unsigned int
    count,
    high_order,
    low_order;

  /*
    Add padding and return the message accumulator.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(signature_info != (SignatureInfo *) NULL);
  assert(signature_info->signature == MagickSignature);
  low_order=signature_info->low_order;
  high_order=signature_info->high_order;
  count=((low_order >> 3) & 0x3f);
  datum=GetStringInfoDatum(signature_info->message);
  datum[count++]=(unsigned char) 0x80;
  if (count <= (unsigned int) (GetStringInfoLength(signature_info->message)-8))
    (void) ResetMagickMemory(datum+count,0,GetStringInfoLength(
      signature_info->message)-8-count);
  else
    {
      (void) ResetMagickMemory(datum+count,0,GetStringInfoLength(
        signature_info->message)-count);
      TransformSignature(signature_info);
      (void) ResetMagickMemory(datum,0,GetStringInfoLength(
        signature_info->message)-8);
    }
  datum[56]=(unsigned char) (high_order >> 24);
  datum[57]=(unsigned char) (high_order >> 16);
  datum[58]=(unsigned char) (high_order >> 8);
  datum[59]=(unsigned char) high_order;
  datum[60]=(unsigned char) (low_order >> 24);
  datum[61]=(unsigned char) (low_order >> 16);
  datum[62]=(unsigned char) (low_order >> 8);
  datum[63]=(unsigned char) low_order;
  TransformSignature(signature_info);
  p=signature_info->accumulator;
  q=GetStringInfoDatum(signature_info->digest);
  for (i=0; i < (SignatureDigestsize/4); i++)
  {
    *q++=(unsigned char) ((*p >> 24) & 0xff);
    *q++=(unsigned char) ((*p >> 16) & 0xff);
    *q++=(unsigned char) ((*p >> 8) & 0xff);
    *q++=(unsigned char) (*p & 0xff);
    p++;
  }
  /*
    Reset working registers.
  */
  count=0;
  high_order=0;
  low_order=0;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t S i g n a t u r e B l o c k s i z e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSignatureBlocksize() returns the Signature blocksize.
%
%  The format of the GetSignatureBlocksize method is:
%
%      unsigned int *GetSignatureBlocksize(const SignatureInfo *signature_info)
%
%  A description of each parameter follows:
%
%    o signature_info: the signature info.
%
*/
MagickExport unsigned int GetSignatureBlocksize(
  const SignatureInfo *signature_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(signature_info != (SignatureInfo *) NULL);
  assert(signature_info->signature == MagickSignature);
  return(signature_info->blocksize);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t S i g n a t u r e D i g e s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSignatureDigest() returns the signature digest.
%
%  The format of the GetSignatureDigest method is:
%
%      const StringInfo *GetSignatureDigest(const SignatureInfo *signature_info)
%
%  A description of each parameter follows:
%
%    o signature_info: the signature info.
%
*/
MagickExport const StringInfo *GetSignatureDigest(
  const SignatureInfo *signature_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(signature_info != (SignatureInfo *) NULL);
  assert(signature_info->signature == MagickSignature);
  return(signature_info->digest);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t S i g n a t u r e D i g e s t s i z e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSignatureDigestsize() returns the Signature digest size.
%
%  The format of the GetSignatureDigestsize method is:
%
%      unsigned int *GetSignatureDigestsize(const SignatureInfo *signature_info)
%
%  A description of each parameter follows:
%
%    o signature_info: the signature info.
%
*/
MagickExport unsigned int GetSignatureDigestsize(
  const SignatureInfo *signature_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(signature_info != (SignatureInfo *) NULL);
  assert(signature_info->signature == MagickSignature);
  return(signature_info->digestsize);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e S i g n a t u r e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeSignature() initializes the Signature accumulator.
%
%  The format of the DestroySignatureInfo method is:
%
%      void InitializeSignatureInfo(SignatureInfo *signature_info)
%
%  A description of each parameter follows:
%
%    o signature_info: the cipher signature_info.
%
*/
MagickExport void InitializeSignature(SignatureInfo *signature_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(signature_info != (SignatureInfo *) NULL);
  assert(signature_info->signature == MagickSignature);
  signature_info->accumulator[0]=0x6a09e667U;
  signature_info->accumulator[1]=0xbb67ae85U;
  signature_info->accumulator[2]=0x3c6ef372U;
  signature_info->accumulator[3]=0xa54ff53aU;
  signature_info->accumulator[4]=0x510e527fU;
  signature_info->accumulator[5]=0x9b05688cU;
  signature_info->accumulator[6]=0x1f83d9abU;
  signature_info->accumulator[7]=0x5be0cd19U;
  signature_info->low_order=0;
  signature_info->high_order=0;
  signature_info->offset=0;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t S i g n a t u r e D i g e s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetSignatureDigest() set the signature digest.
%
%  The format of the SetSignatureDigest method is:
%
%      SetSignatureDigest(SignatureInfo *signature_info,
%        const StringInfo *digest)
%
%  A description of each parameter follows:
%
%    o signature_info: the signature info.
%
%    o digest: the digest.
%
*/
MagickExport void SetSignatureDigest(SignatureInfo *signature_info,
  const StringInfo *digest)
{
  /*
    Set the signature accumulator.
  */
  assert(signature_info != (SignatureInfo *) NULL);
  assert(signature_info->signature == MagickSignature);
  SetStringInfo(signature_info->digest,digest);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S i g n a t u r e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SignatureImage() computes a message digest from an image pixel stream with
%  an implementation of the NIST SHA-256 Message Digest algorithm.  This
%  signature uniquely identifies the image and is convenient for determining
%  if an image has been modified or whether two images are identical.
%
%  The format of the SignatureImage method is:
%
%      MagickBooleanType SignatureImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType SignatureImage(Image *image)
{
  CacheView
    *image_view;

  char
    *hex_signature;

  ExceptionInfo
    *exception;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register const PixelPacket
    *p;

  SignatureInfo
    *signature_info;

  size_t
    length;

  ssize_t
    y;

  StringInfo
    *signature;

  unsigned char
    *pixels;

  /*
    Compute image digital signature.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  quantum_type=RGBQuantum;
  if (image->matte != MagickFalse)
    quantum_type=RGBAQuantum;
  if (image->colorspace == CMYKColorspace)
    {
      quantum_type=CMYKQuantum;
      if (image->matte != MagickFalse)
        quantum_type=CMYKAQuantum;
    }
  signature_info=AcquireSignatureInfo();
  signature=AcquireStringInfo(quantum_info->extent);
  pixels=GetQuantumPixels(quantum_info);
  exception=(&image->exception);
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    length=ExportQuantumPixels(image,image_view,quantum_info,quantum_type,
      pixels,&image->exception);
    SetStringInfoLength(signature,length);
    SetStringInfoDatum(signature,pixels);
    UpdateSignature(signature_info,signature);
  }
  image_view=DestroyCacheView(image_view);
  quantum_info=DestroyQuantumInfo(quantum_info);
  FinalizeSignature(signature_info);
  hex_signature=StringInfoToHexString(GetSignatureDigest(signature_info));
  (void) DeleteImageProperty(image,"signature");
  (void) SetImageProperty(image,"signature",hex_signature);
  /*
    Free resources.
  */
  hex_signature=DestroyString(hex_signature);
  signature=DestroyStringInfo(signature);
  signature_info=DestroySignatureInfo(signature_info);
  return(MagickTrue);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   T r a n s f o r m S i g n a t u r e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransformSignature() transforms the Signature message accumulator.
%
%  The format of the TransformSignature method is:
%
%      TransformSignature(SignatureInfo *signature_info)
%
%  A description of each parameter follows:
%
%    o signature_info: the address of a structure of type SignatureInfo.
%
*/

static inline unsigned int Ch(unsigned int x,unsigned int y,unsigned int z)
{
  return((x & y) ^ (~x & z));
}

static inline unsigned int Maj(unsigned int x,unsigned int y,unsigned int z)
{
  return((x & y) ^ (x & z) ^ (y & z));
}

static inline unsigned int Trunc32(unsigned int x)
{
  return((unsigned int) (x & 0xffffffffU));
}

static unsigned int RotateRight(unsigned int x,unsigned int n)
{
  return(Trunc32((x >> n) | (x << (32-n))));
}

static void TransformSignature(SignatureInfo *signature_info)
{
#define Sigma0(x)  (RotateRight(x,7) ^ RotateRight(x,18) ^ Trunc32((x) >> 3))
#define Sigma1(x)  (RotateRight(x,17) ^ RotateRight(x,19) ^ Trunc32((x) >> 10))
#define Suma0(x)  (RotateRight(x,2) ^ RotateRight(x,13) ^ RotateRight(x,22))
#define Suma1(x)  (RotateRight(x,6) ^ RotateRight(x,11) ^ RotateRight(x,25))

  register ssize_t
    i;

  register unsigned char
    *p;

  ssize_t
    j;

  static unsigned int
    K[64] =
    {
      0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU,
      0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U, 0xd807aa98U, 0x12835b01U,
      0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U,
      0xc19bf174U, 0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
      0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, 0x983e5152U,
      0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U,
      0x06ca6351U, 0x14292967U, 0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU,
      0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
      0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U,
      0xd6990624U, 0xf40e3585U, 0x106aa070U, 0x19a4c116U, 0x1e376c08U,
      0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU,
      0x682e6ff3U, 0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
      0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
    };  /* 32-bit fractional part of the cube root of the first 64 primes */

  unsigned int
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    shift,
    T,
    T1,
    T2,
    W[64];

  shift=32;
  p=GetStringInfoDatum(signature_info->message);
  if (signature_info->lsb_first == MagickFalse)
    {
DisableMSCWarning(4127)
      if (sizeof(unsigned int) <= 4)
RestoreMSCWarning
        for (i=0; i < 16; i++)
        {
          T=(*((unsigned int *) p));
          p+=4;
          W[i]=Trunc32(T);
        }
      else
        for (i=0; i < 16; i+=2)
        {
          T=(*((unsigned int *) p));
          p+=8;
          W[i]=Trunc32(T >> shift);
          W[i+1]=Trunc32(T);
        }
    }
  else
DisableMSCWarning(4127)
    if (sizeof(unsigned int) <= 4)
RestoreMSCWarning
      for (i=0; i < 16; i++)
      {
        T=(*((unsigned int *) p));
        p+=4;
        W[i]=((T << 24) & 0xff000000) | ((T << 8) & 0x00ff0000) |
          ((T >> 8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
      }
    else
      for (i=0; i < 16; i+=2)
      {
        T=(*((unsigned int *) p));
        p+=8;
        W[i]=((T << 24) & 0xff000000) | ((T << 8) & 0x00ff0000) |
          ((T >> 8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
        T>>=shift;
        W[i+1]=((T << 24) & 0xff000000) | ((T << 8) & 0x00ff0000) |
          ((T >> 8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
      }
  /*
    Copy accumulator to registers.
  */
  A=signature_info->accumulator[0];
  B=signature_info->accumulator[1];
  C=signature_info->accumulator[2];
  D=signature_info->accumulator[3];
  E=signature_info->accumulator[4];
  F=signature_info->accumulator[5];
  G=signature_info->accumulator[6];
  H=signature_info->accumulator[7];
  for (i=16; i < 64; i++)
    W[i]=Trunc32(Sigma1(W[i-2])+W[i-7]+Sigma0(W[i-15])+W[i-16]);
  for (j=0; j < 64; j++)
  {
    T1=Trunc32(H+Suma1(E)+Ch(E,F,G)+K[j]+W[j]);
    T2=Trunc32(Suma0(A)+Maj(A,B,C));
    H=G;
    G=F;
    F=E;
    E=Trunc32(D+T1);
    D=C;
    C=B;
    B=A;
    A=Trunc32(T1+T2);
  }
  /*
    Add registers back to accumulator.
  */
  signature_info->accumulator[0]=Trunc32(signature_info->accumulator[0]+A);
  signature_info->accumulator[1]=Trunc32(signature_info->accumulator[1]+B);
  signature_info->accumulator[2]=Trunc32(signature_info->accumulator[2]+C);
  signature_info->accumulator[3]=Trunc32(signature_info->accumulator[3]+D);
  signature_info->accumulator[4]=Trunc32(signature_info->accumulator[4]+E);
  signature_info->accumulator[5]=Trunc32(signature_info->accumulator[5]+F);
  signature_info->accumulator[6]=Trunc32(signature_info->accumulator[6]+G);
  signature_info->accumulator[7]=Trunc32(signature_info->accumulator[7]+H);
  /*
    Reset working registers.
  */
  A=0;
  B=0;
  C=0;
  D=0;
  E=0;
  F=0;
  G=0;
  H=0;
  T=0;
  T1=0;
  T2=0;
  (void) ResetMagickMemory(W,0,sizeof(W));
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   U p d a t e S i g n a t u r e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UpdateSignature() updates the Signature message accumulator.
%
%  The format of the UpdateSignature method is:
%
%      UpdateSignature(SignatureInfo *signature_info,const StringInfo *message)
%
%  A description of each parameter follows:
%
%    o signature_info: the address of a structure of type SignatureInfo.
%
%    o message: the message.
%
*/
MagickExport void UpdateSignature(SignatureInfo *signature_info,
  const StringInfo *message)
{
  register size_t
    i;

  register unsigned char
    *p;

  size_t
    n;

  unsigned int
    length;

  /*
    Update the Signature accumulator.
  */
  assert(signature_info != (SignatureInfo *) NULL);
  assert(signature_info->signature == MagickSignature);
  n=GetStringInfoLength(message);
  length=Trunc32((unsigned int) (signature_info->low_order+(n << 3)));
  if (length < signature_info->low_order)
    signature_info->high_order++;
  signature_info->low_order=length;
  signature_info->high_order+=(unsigned int) (n >> 29);
  p=GetStringInfoDatum(message);
  if (signature_info->offset != 0)
    {
      i=GetStringInfoLength(signature_info->message)-signature_info->offset;
      if (i > n)
        i=n;
      (void) CopyMagickMemory(GetStringInfoDatum(signature_info->message)+
        signature_info->offset,p,i);
      n-=i;
      p+=i;
      signature_info->offset+=i;
      if (signature_info->offset !=
          GetStringInfoLength(signature_info->message))
        return;
      TransformSignature(signature_info);
    }
  while (n >= GetStringInfoLength(signature_info->message))
  {
    SetStringInfoDatum(signature_info->message,p);
    p+=GetStringInfoLength(signature_info->message);
    n-=GetStringInfoLength(signature_info->message);
    TransformSignature(signature_info);
  }
  (void) CopyMagickMemory(GetStringInfoDatum(signature_info->message),p,n);
  signature_info->offset=n;
  /*
    Reset working registers.
  */
  i=0;
  n=0;
  length=0;
}

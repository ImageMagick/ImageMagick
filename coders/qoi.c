/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             QQQ    OOO   IIIII                              %
%                            Q   Q  O   O    I                                %
%                            Q   Q  O   O    I                                %
%                            Q  QQ  O   O    I                                %
%                             QQQQ   OOO   IIIII                              %
%                                 Q                                           %
%                                                                             %
%                           Quite OK Image Format                             %
%                                                                             %
%                              Software Design                                %
%                              Jules Maselbas                                 %
%                               December 2021                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
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
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"


#define QOI_SRGB   0
#define QOI_LINEAR 1

#define QOI_OP_INDEX  0x00 /* 00xxxxxx */
#define QOI_OP_DIFF   0x40 /* 01xxxxxx */
#define QOI_OP_LUMA   0x80 /* 10xxxxxx */
#define QOI_OP_RUN    0xc0 /* 11xxxxxx */
#define QOI_OP_RGB    0xfe /* 11111110 */
#define QOI_OP_RGBA   0xff /* 11111111 */

#define QOI_MASK_2    0xc0 /* 11000000 */

#define QOI_COLOR_HASH(C) (C.rgba.r*3 + C.rgba.g*5 + C.rgba.b*7 + C.rgba.a*11)

static unsigned int IsQOI(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick, "qoif", 4) != 0)
    return(MagickTrue);
  return(MagickFalse);
}


typedef union {
  struct { unsigned char r, g, b, a; } rgba;
  unsigned int v;
} qoi_rgba_t;

static Image *ReadQOIImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  QuantumInfo
    *quantum_info;

  Quantum
    *q;

  MagickBooleanType
    status;

  size_t
     colorspace,
     channels;

  ssize_t
    count,
    p;

  unsigned char
    magick[4];

  qoi_rgba_t
    px,
    lut[64];

  int
    b,
    vg,
    run;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Initialize image structure.
  */
  image->endian=MSBEndian;
  image->depth=8;
  count=ReadBlob(image,4,magick);
  if ((count != 4) || (LocaleNCompare((char *) magick, "qoif", 4) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->columns=(size_t) ReadBlobMSBLong(image);
  image->rows=(size_t) ReadBlobMSBLong(image);
  image->alpha_trait=BlendPixelTrait;
  if (image->columns == 0 || image->rows == 0)
    ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");

  channels=ReadBlobByte(image);
  if (channels == 3)
    SetQuantumImageType(image,RGBQuantum);
  else if (channels == 4)
    SetQuantumImageType(image,RGBAQuantum);
  else
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  colorspace=ReadBlobByte(image);
  if (colorspace == QOI_SRGB)
    SetImageColorspace(image,sRGBColorspace,exception);
  else if (colorspace == QOI_LINEAR)
    SetImageColorspace(image,RGBColorspace,exception);
  else
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  status=SetQuantumFormat(image,quantum_info,UnsignedQuantumFormat);
  if (status == MagickFalse)
    return(DestroyImageList(image));

  /*
    Get a write pointer for the whole image.
  */
  q=QueueAuthenticPixels(image,0,0,image->columns,image->rows,exception);
  if (q == (Quantum *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize decoding state.
  */
  px.rgba.r=0;
  px.rgba.g=0;
  px.rgba.b=0;
  px.rgba.a=255;
  memset(lut,0,sizeof(lut));
  /*
    Actual decoding.
  */
  for (p=0; p < (image->rows * image->columns);)
  {
    run = 0;

    if ((b=ReadBlobByte(image)) == EOF)
      break;
    if (b == QOI_OP_RGB) {
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.r=b;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.g=b;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.b=b;
    }
    else if (b == QOI_OP_RGBA) {
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.r=b;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.g=b;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.b=b;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.a=b;
    }
    else if ((b & QOI_MASK_2) == QOI_OP_INDEX) {
      px=lut[b & ~QOI_MASK_2];
    }
    else if ((b & QOI_MASK_2) == QOI_OP_DIFF) {
      px.rgba.r+=((b >> 4) & 0x03) - 2;
      px.rgba.g+=((b >> 2) & 0x03) - 2;
      px.rgba.b+=( b       & 0x03) - 2;
    }
    else if ((b & QOI_MASK_2) == QOI_OP_LUMA) {
      vg=(b & ~QOI_MASK_2) - 32;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.r+=vg - 8 + ((b >> 4) & 0x0f);
      px.rgba.g+=vg;
      px.rgba.b+=vg - 8 +  (b       & 0x0f);
    }
    else if ((b & QOI_MASK_2) == QOI_OP_RUN) {
      run=b & ~QOI_MASK_2;
    }
    lut[QOI_COLOR_HASH(px) % 64]=px;
    do {
      SetPixelRed(image,ScaleCharToQuantum((unsigned char)px.rgba.r),q);
      SetPixelGreen(image,ScaleCharToQuantum((unsigned char)px.rgba.g),q);
      SetPixelBlue(image,ScaleCharToQuantum((unsigned char)px.rgba.b),q);
      if (channels == 4)
        SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)px.rgba.a),q);
      q+=GetPixelChannels(image);
      p++;
    } while (run-- > 0);
    status=SetImageProgress(image,LoadImageTag,p,image->rows * image->columns);
    if (status == MagickFalse)
      break;
  }
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,
      "UnexpectedEndOfFile",image->filename);
  if (status != MagickFalse) {
    status=SyncAuthenticPixels(image,exception);
    if (image->ping == MagickFalse)
      SyncImage(image,exception);
  }
  (void) CloseBlob(image);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}


static MagickBooleanType WriteQOIImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  MagickBooleanType
    status;

  const Quantum
    *p;

  size_t
    channels,
    colorspace,
    end,
    run,
    idx,
    i;

  qoi_rgba_t
    px,
    pp,
    lut[64];

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  image->endian=MSBEndian;
  image->depth=8;

  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ImageError,"MemoryAllocationFailed");

  quantum_type=GetQuantumType(image,exception);
  if (quantum_type == RGBQuantum)
    channels=3;
  else if (quantum_type == RGBAQuantum)
    channels=4;
  else
    ThrowWriterException(ImageError,"MemoryAllocationFailed");

  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
   (void) TransformImageColorspace(image,sRGBColorspace,exception);
  if (IsRGBColorspace(image->colorspace))
     colorspace=QOI_LINEAR;
  else
     colorspace=QOI_SRGB;
  /*
    Write QOI header.
  */
  (void) WriteBlobString(image,"qoif");
  (void) WriteBlobMSBLong(image,(unsigned int) image->columns);
  (void) WriteBlobMSBLong(image,(unsigned int) image->rows);
  (void) WriteBlobByte(image,(const unsigned char) channels);
  (void) WriteBlobByte(image,(const unsigned char) colorspace);
  /*
    Initialize encoding state.
  */
  px.rgba.r=0;
  px.rgba.g=0;
  px.rgba.b=0;
  px.rgba.a=255;
  memset(lut,0,sizeof(lut));
  run=0;
  p=GetVirtualPixels(image,0,0,image->columns,image->rows,exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  /*
    Do the actual encoding.
  */
  end = image->rows * image->columns;
  for (i=0; i < end; i++) {
    pp=px;
    px.rgba.r=ScaleQuantumToChar(GetPixelRed(image,p));
    px.rgba.g=ScaleQuantumToChar(GetPixelGreen(image,p));
    px.rgba.b=ScaleQuantumToChar(GetPixelBlue(image,p));
    if (channels == 4)
      px.rgba.a=ScaleQuantumToChar(GetPixelAlpha(image,p));
    p+=GetPixelChannels(image);

    if (pp.v == px.v) {
      run++;
      if (run == 62) {
        (void) WriteBlobByte(image,QOI_OP_RUN | (run - 1));
        run=0;
      }
      continue;
    }
    if (run > 0) {
      (void) WriteBlobByte(image,QOI_OP_RUN | (run - 1));
      run=0;
    }
    idx=QOI_COLOR_HASH(px) % 64;
    if (lut[idx].v == px.v) {
      (void) WriteBlobByte(image,QOI_OP_INDEX | idx);
      continue;
    }
    lut[QOI_COLOR_HASH(px) % 64]=px;
    if (pp.rgba.a == px.rgba.a) {
      signed char
        vr,
        vg,
        vb,
        vg_r,
        vg_b;
      vr=px.rgba.r - pp.rgba.r;
      vg=px.rgba.g - pp.rgba.g;
      vb=px.rgba.b - pp.rgba.b;
      vg_r=vr - vg;
      vg_b=vb - vg;

      if (vr > -3 && vr < 2 &&
          vg > -3 && vg < 2 &&
          vb > -3 && vb < 2) {
        (void) WriteBlobByte(image,QOI_OP_DIFF
                 | (vr + 2) << 4 | (vg + 2) << 2 | (vb + 2));
      } else if (vg_r >  -9 && vg_r < 8 &&
                 vg   > -33 && vg   < 32 &&
                 vg_b >  -9 && vg_b < 2) {
        (void) WriteBlobByte(image,QOI_OP_LUMA | (vg + 32));
        (void) WriteBlobByte(image,(vg_r + 8) << 4 | (vg_b +  8));
      } else {
        (void) WriteBlobByte(image,QOI_OP_RGB);
        (void) WriteBlobByte(image,px.rgba.r);
        (void) WriteBlobByte(image,px.rgba.g);
        (void) WriteBlobByte(image,px.rgba.b);
      }
    } else {
      (void) WriteBlobByte(image,QOI_OP_RGBA);
      (void) WriteBlobByte(image,px.rgba.r);
      (void) WriteBlobByte(image,px.rgba.g);
      (void) WriteBlobByte(image,px.rgba.b);
      (void) WriteBlobByte(image,px.rgba.a);
    }
  }
  return(MagickTrue);
}

ModuleExport size_t RegisterQOIImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("QOI","QOI","Quite OK image format");
  entry->decoder=(DecodeImageHandler *) ReadQOIImage;
  entry->encoder=(EncodeImageHandler *) WriteQOIImage;
  entry->magick=(IsImageFormatHandler *) IsQOI;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

ModuleExport void UnregisterQOIImage(void)
{
  (void) UnregisterMagickInfo("QOI");
}

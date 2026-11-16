/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                   CCCC  IIIII  PPPP   H   H  EEEEE  RRRR                    %
%                  C        I    P   P  H   H  E      R   R                   %
%                  C        I    PPPP   HHHHH  EEE    RRRR                    %
%                  C        I    P      H   H  E      R R                     %
%                   CCCC  IIIII  P      H   H  EEEEE  R  R                    %
%                                                                             %
%                                                                             %
%                          MagickCore Cipher Methods                          %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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
#include "MagickCore/cache.h"
#include "MagickCore/cipher.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/list.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/registry.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/timer-private.h"

#if defined(MAGICKCORE_CIPHER_SUPPORT)
/*
  Define declarations.
*/
#define AESBlocksize 16
#define AESKeyWords 60  /* 4*(14+1): worst case, AES-256 */

/*
  Typedef declarations.
*/
typedef struct _AESInfo
{
  StringInfo
    *key;

  unsigned int
    blocksize,
    *encipher_key;

  ssize_t
    rounds;

  time_t
    timestamp;

  size_t
    signature;
} AESInfo;

/*
  Global declarations.
*/
static const unsigned char
  SBox[256] =
  {
     99, 124, 119, 123, 242, 107, 111, 197,  48,   1, 103,  43, 254, 215,
    171, 118, 202, 130, 201, 125, 250,  89,  71, 240, 173, 212, 162, 175,
    156, 164, 114, 192, 183, 253, 147,  38,  54,  63, 247, 204,  52, 165,
    229, 241, 113, 216,  49,  21,   4, 199,  35, 195,  24, 150,   5, 154,
      7,  18, 128, 226, 235,  39, 178, 117,   9, 131,  44,  26,  27, 110,
     90, 160,  82,  59, 214, 179,  41, 227,  47, 132,  83, 209,   0, 237,
     32, 252, 177,  91, 106, 203, 190,  57,  74,  76,  88, 207, 208, 239,
    170, 251,  67,  77,  51, 133,  69, 249,   2, 127,  80,  60, 159, 168,
     81, 163,  64, 143, 146, 157,  56, 245, 188, 182, 218,  33,  16, 255,
    243, 210, 205,  12,  19, 236,  95, 151,  68,  23, 196, 167, 126,  61,
    100,  93,  25, 115,  96, 129,  79, 220,  34,  42, 144, 136,  70, 238,
    184,  20, 222,  94,  11, 219, 224,  50,  58,  10,  73,   6,  36,  92,
    194, 211, 172,  98, 145, 149, 228, 121, 231, 200,  55, 109, 141, 213,
     78, 169, 108,  86, 244, 234, 101, 122, 174,   8, 186, 120,  37,  46,
     28, 166, 180, 198, 232, 221, 116,  31,  75, 189, 139, 138, 112,  62,
    181, 102,  72,   3, 246,  14,  97,  53,  87, 185, 134, 193,  29, 158,
    225, 248, 152,  17, 105, 217, 142, 148, 155,  30, 135, 233, 206,  85,
     40, 223, 140, 161, 137,  13, 191, 230,  66, 104,  65, 153,  45,  15,
    176,  84, 187, 22
  };

/*
  Forward declarations.
*/
static AESInfo
  *DestroyAESInfo(AESInfo *);

static void
  EncipherAESBlock(AESInfo *,const unsigned char *,unsigned char *),
  SetAESKey(AESInfo *,const StringInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e A E S I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireAESInfo() allocate the AESInfo structure.
%
%  The format of the AcquireAESInfo method is:
%
%      AESInfo *AcquireAESInfo(void)
%
*/
static AESInfo *AcquireAESInfo(void)
{
  AESInfo
    *aes_info;

  aes_info=(AESInfo *) AcquireCriticalMemory(sizeof(*aes_info));
  (void) memset(aes_info,0,sizeof(*aes_info));
  aes_info->blocksize=AESBlocksize;
  aes_info->key=AcquireStringInfo(32);
  aes_info->encipher_key=(unsigned int *) AcquireQuantumMemory(AESKeyWords,
    sizeof(*aes_info->encipher_key));
  if ((aes_info->key == (StringInfo *) NULL) ||
      (aes_info->encipher_key == (unsigned int *) NULL))
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(aes_info->encipher_key,0,AESKeyWords*
    sizeof(*aes_info->encipher_key));
  aes_info->timestamp=GetMagickTime();
  aes_info->signature=MagickCoreSignature;
  return(aes_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y A E S I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyAESInfo() zeros memory associated with the AESInfo structure.
%
%  The format of the DestroyAESInfo method is:
%
%      AESInfo *DestroyAESInfo(AESInfo *aes_info)
%
%  A description of each parameter follows:
%
%    o aes_info: the cipher context.
%
*/

static inline void SecureZero(void *memory,const size_t length)
{
  volatile unsigned char
    *p;

  size_t
    i;

  /*
    Securely wipe sensitive memory.
  */
  p=(volatile unsigned char *) memory;
  for (i=0; i < length; i++)
    p[i]=0;
}

static inline void WipeStringInfo(StringInfo *string_info)
{
  if ((string_info != (StringInfo *) NULL) &&
      (GetStringInfoDatum(string_info) != (unsigned char *) NULL))
    SecureZero(GetStringInfoDatum(string_info),
      GetStringInfoLength(string_info));
}

static AESInfo *DestroyAESInfo(AESInfo *aes_info)
{
  assert(aes_info != (AESInfo *) NULL);
  assert(aes_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (aes_info->encipher_key != (unsigned int *) NULL)
    {
      SecureZero(aes_info->encipher_key,AESKeyWords*
        sizeof(*aes_info->encipher_key));
      aes_info->encipher_key=(unsigned int *) RelinquishMagickMemory(
        aes_info->encipher_key);
    }
  if (aes_info->key != (StringInfo *) NULL)
    {
      WipeStringInfo(aes_info->key);
      aes_info->key=DestroyStringInfo(aes_info->key);
    }
  aes_info->signature=(~MagickCoreSignature);
  aes_info=(AESInfo *) RelinquishMagickMemory(aes_info);
  return(aes_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E n c i p h e r A E S B l o c k                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherAESBlock() enciphers a single block of plaintext to produce a block
%  of ciphertext.
%
%  The format of the EncipherAESBlock method is:
%
%      void EncipherAESBlock(AESInfo *aes_info,const unsigned char *plaintext,
%        unsigned char *ciphertext)
%
%  A description of each parameter follows:
%
%    o aes_info: the cipher context.
%
%    o plaintext: the plain text.
%
%    o ciphertext: the cipher text.
%
*/

static inline void AddRoundKey(const unsigned int *ciphertext,
  const unsigned int *key,unsigned int *plaintext)
{
  ssize_t
    i;

  /*
    Xor corresponding text input and round key input bytes.
  */
  for (i=0; i < 4; i++)
    plaintext[i]=key[i] ^ ciphertext[i];
}

static inline unsigned int ByteSubTransform(unsigned int x,
  const unsigned char *s_box)
{
  unsigned int
    key;

  /*
    Non-linear layer resists differential and linear cryptoanalysis attacks.
  */
  key=((unsigned int) s_box[x & 0xff]) |
    ((unsigned int) s_box[(x >> 8) & 0xff] << 8) |
    ((unsigned int) s_box[(x >> 16) & 0xff] << 16) |
    ((unsigned int) s_box[(x >> 24) & 0xff] << 24);
  return(key);
}

static void FinalizeRoundKey(const unsigned int *ciphertext,
  const unsigned int *key,unsigned char *plaintext)
{
  unsigned char
    *p;

  unsigned int
    i,
    j;

  unsigned int
    value;

  /*
    The round key is XORed with the result of the mix-column transformation.
  */
  p=plaintext;
  for (i=0; i < 4; i++)
  {
    value=ciphertext[i] ^ key[i];
    for (j=0; j < 4; j++)
      *p++=(unsigned char) ((value >> (8*j)) & 0xff);
  }
  SecureZero(&value,sizeof(value));
}

static void InitializeRoundKey(const unsigned char *ciphertext,
  const unsigned int *key,unsigned int *plaintext)
{
  const unsigned char
    *p;

  unsigned int
    i,
    j;

  unsigned int
    value;

  p=ciphertext;
  for (i=0; i < 4; i++)
  {
    value=0;
    for (j=0; j < 4; j++)
      value|=((unsigned int) *p++ << (8*j));
    plaintext[i]=key[i] ^ value;
  }
  SecureZero(&value,sizeof(value));
}

static inline unsigned int RotateLeft(const unsigned int x)
{
  return(((x << 8) | ((x >> 24) & 0xff)));
}

static void EncipherAESBlock(AESInfo *aes_info,const unsigned char *plaintext,
  unsigned char *ciphertext)
{
  ssize_t
    i,
    j;

  static const int
    map[4][4] =
    {
      { 0, 1, 2, 3 },
      { 1, 2, 3, 0 },
      { 2, 3, 0, 1 },
      { 3, 0, 1, 2 }
    };

  static const unsigned int
    D[] =
    {
      0xa56363c6U, 0x847c7cf8U, 0x997777eeU, 0x8d7b7bf6U, 0x0df2f2ffU,
      0xbd6b6bd6U, 0xb16f6fdeU, 0x54c5c591U, 0x50303060U, 0x03010102U,
      0xa96767ceU, 0x7d2b2b56U, 0x19fefee7U, 0x62d7d7b5U, 0xe6abab4dU,
      0x9a7676ecU, 0x45caca8fU, 0x9d82821fU, 0x40c9c989U, 0x877d7dfaU,
      0x15fafaefU, 0xeb5959b2U, 0xc947478eU, 0x0bf0f0fbU, 0xecadad41U,
      0x67d4d4b3U, 0xfda2a25fU, 0xeaafaf45U, 0xbf9c9c23U, 0xf7a4a453U,
      0x967272e4U, 0x5bc0c09bU, 0xc2b7b775U, 0x1cfdfde1U, 0xae93933dU,
      0x6a26264cU, 0x5a36366cU, 0x413f3f7eU, 0x02f7f7f5U, 0x4fcccc83U,
      0x5c343468U, 0xf4a5a551U, 0x34e5e5d1U, 0x08f1f1f9U, 0x937171e2U,
      0x73d8d8abU, 0x53313162U, 0x3f15152aU, 0x0c040408U, 0x52c7c795U,
      0x65232346U, 0x5ec3c39dU, 0x28181830U, 0xa1969637U, 0x0f05050aU,
      0xb59a9a2fU, 0x0907070eU, 0x36121224U, 0x9b80801bU, 0x3de2e2dfU,
      0x26ebebcdU, 0x6927274eU, 0xcdb2b27fU, 0x9f7575eaU, 0x1b090912U,
      0x9e83831dU, 0x742c2c58U, 0x2e1a1a34U, 0x2d1b1b36U, 0xb26e6edcU,
      0xee5a5ab4U, 0xfba0a05bU, 0xf65252a4U, 0x4d3b3b76U, 0x61d6d6b7U,
      0xceb3b37dU, 0x7b292952U, 0x3ee3e3ddU, 0x712f2f5eU, 0x97848413U,
      0xf55353a6U, 0x68d1d1b9U, 0x00000000U, 0x2cededc1U, 0x60202040U,
      0x1ffcfce3U, 0xc8b1b179U, 0xed5b5bb6U, 0xbe6a6ad4U, 0x46cbcb8dU,
      0xd9bebe67U, 0x4b393972U, 0xde4a4a94U, 0xd44c4c98U, 0xe85858b0U,
      0x4acfcf85U, 0x6bd0d0bbU, 0x2aefefc5U, 0xe5aaaa4fU, 0x16fbfbedU,
      0xc5434386U, 0xd74d4d9aU, 0x55333366U, 0x94858511U, 0xcf45458aU,
      0x10f9f9e9U, 0x06020204U, 0x817f7ffeU, 0xf05050a0U, 0x443c3c78U,
      0xba9f9f25U, 0xe3a8a84bU, 0xf35151a2U, 0xfea3a35dU, 0xc0404080U,
      0x8a8f8f05U, 0xad92923fU, 0xbc9d9d21U, 0x48383870U, 0x04f5f5f1U,
      0xdfbcbc63U, 0xc1b6b677U, 0x75dadaafU, 0x63212142U, 0x30101020U,
      0x1affffe5U, 0x0ef3f3fdU, 0x6dd2d2bfU, 0x4ccdcd81U, 0x140c0c18U,
      0x35131326U, 0x2fececc3U, 0xe15f5fbeU, 0xa2979735U, 0xcc444488U,
      0x3917172eU, 0x57c4c493U, 0xf2a7a755U, 0x827e7efcU, 0x473d3d7aU,
      0xac6464c8U, 0xe75d5dbaU, 0x2b191932U, 0x957373e6U, 0xa06060c0U,
      0x98818119U, 0xd14f4f9eU, 0x7fdcdca3U, 0x66222244U, 0x7e2a2a54U,
      0xab90903bU, 0x8388880bU, 0xca46468cU, 0x29eeeec7U, 0xd3b8b86bU,
      0x3c141428U, 0x79dedea7U, 0xe25e5ebcU, 0x1d0b0b16U, 0x76dbdbadU,
      0x3be0e0dbU, 0x56323264U, 0x4e3a3a74U, 0x1e0a0a14U, 0xdb494992U,
      0x0a06060cU, 0x6c242448U, 0xe45c5cb8U, 0x5dc2c29fU, 0x6ed3d3bdU,
      0xefacac43U, 0xa66262c4U, 0xa8919139U, 0xa4959531U, 0x37e4e4d3U,
      0x8b7979f2U, 0x32e7e7d5U, 0x43c8c88bU, 0x5937376eU, 0xb76d6ddaU,
      0x8c8d8d01U, 0x64d5d5b1U, 0xd24e4e9cU, 0xe0a9a949U, 0xb46c6cd8U,
      0xfa5656acU, 0x07f4f4f3U, 0x25eaeacfU, 0xaf6565caU, 0x8e7a7af4U,
      0xe9aeae47U, 0x18080810U, 0xd5baba6fU, 0x887878f0U, 0x6f25254aU,
      0x722e2e5cU, 0x241c1c38U, 0xf1a6a657U, 0xc7b4b473U, 0x51c6c697U,
      0x23e8e8cbU, 0x7cdddda1U, 0x9c7474e8U, 0x211f1f3eU, 0xdd4b4b96U,
      0xdcbdbd61U, 0x868b8b0dU, 0x858a8a0fU, 0x907070e0U, 0x423e3e7cU,
      0xc4b5b571U, 0xaa6666ccU, 0xd8484890U, 0x05030306U, 0x01f6f6f7U,
      0x120e0e1cU, 0xa36161c2U, 0x5f35356aU, 0xf95757aeU, 0xd0b9b969U,
      0x91868617U, 0x58c1c199U, 0x271d1d3aU, 0xb99e9e27U, 0x38e1e1d9U,
      0x13f8f8ebU, 0xb398982bU, 0x33111122U, 0xbb6969d2U, 0x70d9d9a9U,
      0x898e8e07U, 0xa7949433U, 0xb69b9b2dU, 0x221e1e3cU, 0x92878715U,
      0x20e9e9c9U, 0x49cece87U, 0xff5555aaU, 0x78282850U, 0x7adfdfa5U,
      0x8f8c8c03U, 0xf8a1a159U, 0x80898909U, 0x170d0d1aU, 0xdabfbf65U,
      0x31e6e6d7U, 0xc6424284U, 0xb86868d0U, 0xc3414182U, 0xb0999929U,
      0x772d2d5aU, 0x110f0f1eU, 0xcbb0b07bU, 0xfc5454a8U, 0xd6bbbb6dU,
      0x3a16162cU
    };

  unsigned int
    alpha,
    key[4],
    text[4];

  /*
    Encipher one block.
  */
  (void) memset(text,0,sizeof(text));
  InitializeRoundKey(plaintext,aes_info->encipher_key,text);
  for (i=1; i < aes_info->rounds; i++)
  {
    for (j=0; j < 4; j++)
      key[j]=D[text[j] & 0xff] ^
        RotateLeft(D[(text[map[1][j]] >> 8) & 0xff] ^
        RotateLeft(D[(text[map[2][j]] >> 16) & 0xff] ^
        RotateLeft(D[(text[map[3][j]] >> 24) & 0xff])));
    AddRoundKey(key,aes_info->encipher_key+4*i,text);
  }
  for (i=0; i < 4; i++)
  {
    alpha=(text[i] & 0x000000ff) | ((text[map[1][i]]) & 0x0000ff00) |
      ((text[map[2][i]]) & 0x00ff0000) | ((text[map[3][i]]) & 0xff000000);
    key[i]=ByteSubTransform(alpha,SBox);
  }
  FinalizeRoundKey(key,aes_info->encipher_key+4*aes_info->rounds,ciphertext);
  SecureZero(&alpha,sizeof(alpha));
  SecureZero(key,sizeof(key));
  SecureZero(text,sizeof(text));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C i p h e r I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The cipher is AES in counter (CTR) mode applied to the exported quantum
%  pixel rows.  Enciphering and deciphering are the same keystream XOR, so both
%  public entry points share the implementation below.
%
%
*/

static inline MagickBooleanType IncrementCipherNonce(const size_t length,
  unsigned char *nonce)
{
  ssize_t
    i;

  for (i=(ssize_t) (length-1); i >= 0; i--)
  {
    nonce[i]++;
    if (nonce[i] != 0)
      return(MagickTrue);
  }
  return(MagickFalse);
}

static void DeriveCipherCounter(const Image *image,StringInfo *nonce,
  unsigned char *counter,char **hex_digest)
{
  MagickSizeType
    extent;

  SignatureInfo
    *signature_info;

  size_t
    i;

  unsigned char
    extent_bytes[sizeof(extent)];

  /*
    The initial counter block is the leading bytes of SHA-256(nonce ||
    columns*rows).  The extent is serialized little-endian explicitly so the
    result no longer depends on host byte order.
  */
  signature_info=AcquireSignatureInfo();
  UpdateSignature(signature_info,nonce);
  extent=(MagickSizeType) image->columns*image->rows;
  for (i=0; i < sizeof(extent_bytes); i++)
    extent_bytes[i]=(unsigned char) ((extent >> (8*i)) & 0xff);
  WipeStringInfo(nonce);
  SetStringInfoLength(nonce,sizeof(extent_bytes));
  SetStringInfoDatum(nonce,extent_bytes);
  UpdateSignature(signature_info,nonce);
  FinalizeSignature(signature_info);
  if (hex_digest != (char **) NULL)
    *hex_digest=StringInfoToHexString(GetSignatureDigest(signature_info));
  (void) memset(counter,0,AESBlocksize);
  (void) memcpy(counter,GetStringInfoDatum(GetSignatureDigest(signature_info)),
    MagickMin(AESBlocksize,GetSignatureDigestsize(signature_info)));
  signature_info=DestroySignatureInfo(signature_info);
}

static MagickBooleanType XORCipherKeystream(AESInfo *aes_info,
  unsigned char *counter,unsigned char *pixels,const size_t length)
{
  size_t
    i,
    x;

  unsigned char
    keystream[AESBlocksize];

  /*
    XOR exactly `length' bytes with the keystream.  A trailing partial block
    consumes one counter value and never touches bytes past `length'.
  */
  for (x=0; x < length; x+=AESBlocksize)
  {
    size_t
      count;

    count=MagickMin((size_t) AESBlocksize,length-x);
    (void) memcpy(keystream,counter,AESBlocksize);
    if (IncrementCipherNonce(AESBlocksize,counter) == MagickFalse)
      {
        SecureZero(keystream,sizeof(keystream));
        return(MagickFalse);
      }
    EncipherAESBlock(aes_info,keystream,keystream);
    for (i=0; i < count; i++)
      pixels[x+i]^=keystream[i];
  }
  SecureZero(keystream,sizeof(keystream));
  return(MagickTrue);
}

static MagickBooleanType CipherImage(Image *image,const StringInfo *passkey,
  const MagickBooleanType encipher,ExceptionInfo *exception)
{
  AESInfo
    *aes_info;

  CacheView
    *image_view;

  char
    *signature;

  const char
    *progress_tag;

  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  ssize_t
    y;

  StringInfo
    *key,
    *nonce;

  unsigned char
    counter[AESBlocksize],
    *pixels;

  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  aes_info=AcquireAESInfo();
  key=CloneStringInfo(passkey);
  if (key == (StringInfo *) NULL)
    {
      aes_info=DestroyAESInfo(aes_info);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  nonce=SplitStringInfo(key,GetStringInfoLength(key)/2);
  if (nonce == (StringInfo *) NULL)
    {
      WipeStringInfo(key);
      key=DestroyStringInfo(key);
      aes_info=DestroyAESInfo(aes_info);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  SetAESKey(aes_info,key);
  WipeStringInfo(key);
  key=DestroyStringInfo(key);
  signature=(char *) NULL;
  DeriveCipherCounter(image,nonce,counter,
    encipher != MagickFalse ? &signature : (char **) NULL);
  nonce=DestroyStringInfo(nonce);
  if (signature != (char *) NULL)
    {
      (void) SetImageProperty(image,"cipher:type","AES",exception);
      (void) SetImageProperty(image,"cipher:mode","CTR",exception);
      (void) SetImageProperty(image,"cipher:nonce",signature,exception);
      signature=DestroyString(signature);
    }
  quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
  if (quantum_info == (QuantumInfo *) NULL)
    {
      aes_info=DestroyAESInfo(aes_info);
      SecureZero(counter,sizeof(counter));
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  progress_tag=encipher != MagickFalse ? "Encipher/Image " : "Decipher/Image ";
  quantum_type=GetQuantumType(image,exception);
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    size_t
      length;

    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        break;
      }
    length=ExportQuantumPixels(image,image_view,quantum_info,quantum_type,
      pixels,exception);
    if (XORCipherKeystream(aes_info,counter,pixels,length) == MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"SequenceWrapError","`%s'",image->filename);
        status=MagickFalse;
        break;
      }
    (void) ImportQuantumPixels(image,image_view,quantum_info,quantum_type,
      pixels,exception);
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      {
        status=MagickFalse;
        break;
      }
    if (SetImageProgress(image,progress_tag,(MagickOffsetType) y,
        image->rows) == MagickFalse)
      {
        status=MagickFalse;
        break;
      }
  }
  image_view=DestroyCacheView(image_view);
  if (encipher == MagickFalse)
    {
      (void) DeleteImageProperty(image,"cipher:type");
      (void) DeleteImageProperty(image,"cipher:mode");
      (void) DeleteImageProperty(image,"cipher:nonce");
    }
  image->taint=MagickFalse;
  quantum_info=DestroyQuantumInfo(quantum_info);
  aes_info=DestroyAESInfo(aes_info);
  SecureZero(counter,sizeof(counter));
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c i p h e r I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherImage() converts cipher pixels to plain pixels.  The mode is
%  unauthenticated: a wrong passphrase is not detected and yields noise.
%
%  The format of the DecipherImage method is:
%
%      MagickBooleanType DecipherImage(Image *image,const char *passphrase,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o passphrase: decipher cipher pixels with this passphrase.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType DecipherImage(Image *image,
  const char *passphrase,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  StringInfo
    *passkey;

  if (passphrase == (const char *) NULL)
    return(MagickTrue);
  passkey=StringToStringInfo(passphrase);
  if (passkey == (StringInfo *) NULL)
    return(MagickFalse);
  status=PasskeyDecipherImage(image,passkey,exception);
  WipeStringInfo(passkey);
  passkey=DestroyStringInfo(passkey);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a s s k e y D e c i p h e r I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PasskeyDecipherImage() converts cipher pixels to plain pixels.
%
%  The format of the PasskeyDecipherImage method is:
%
%      MagickBooleanType PasskeyDecipherImage(Image *image,
%        const StringInfo *passkey,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o passkey: decipher cipher pixels with this passkey.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType PasskeyDecipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (passkey == (const StringInfo *) NULL)
    return(MagickTrue);
  return(CipherImage(image,passkey,MagickFalse,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E n c i p h e r I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherImage() converts pixels to cipher-pixels.
%
%  The format of the EncipherImage method is:
%
%      MagickBooleanType EncipherImage(Image *image,const char *passphrase,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o passphrase: encipher pixels with this passphrase.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType EncipherImage(Image *image,
  const char *passphrase,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  StringInfo
    *passkey;

  if (passphrase == (const char *) NULL)
    return(MagickTrue);
  passkey=StringToStringInfo(passphrase);
  if (passkey == (StringInfo *) NULL)
    return(MagickFalse);
  status=PasskeyEncipherImage(image,passkey,exception);
  WipeStringInfo(passkey);
  passkey=DestroyStringInfo(passkey);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a s s k e y E n c i p h e r I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PasskeyEncipherImage() converts pixels to cipher-pixels.  The passkey must
%  be at least two bytes long (it is split in two halves); shorter passkeys
%  would produce an empty, all-zero AES key and are rejected.
%
%  The format of the PasskeyEncipherImage method is:
%
%      MagickBooleanType PasskeyEncipherImage(Image *image,
%        const StringInfo *passkey,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o passkey: encipher pixels with this passkey.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType PasskeyEncipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (passkey == (const StringInfo *) NULL)
    return(MagickTrue);
  if (GetStringInfoLength(passkey) < 2)
    ThrowBinaryException(OptionError,"PassphraseTooShort",image->filename);
  return(CipherImage(image,passkey,MagickTrue,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t A E S K e y                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetAESKey() sets the key for the AES cipher.  Only the encryption key
%  schedule is built: CTR mode never runs the inverse cipher.
%
*/

static inline unsigned int XTime(unsigned char alpha)
{
  unsigned char
    beta;

  beta=(unsigned char) ((alpha & 0x80) != 0 ? 0x1b : 0);
  alpha<<=1;
  alpha^=beta;
  return(alpha);
}

static inline unsigned int RotateRight(const unsigned int x)
{
  return((x >> 8) | ((x & 0xff) << 24));
}

static void SetAESKey(AESInfo *aes_info,const StringInfo *key)
{
  ssize_t
    words,
    i,
    n;

  unsigned char
    *datum;

  unsigned int
    alpha,
    beta;

  assert(aes_info != (AESInfo *) NULL);
  assert(aes_info->signature == MagickCoreSignature);
  assert(key != (StringInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  n=4;
  aes_info->rounds=10;
  if ((8*GetStringInfoLength(key)) >= 256)
    {
      n=8;
      aes_info->rounds=14;
    }
  else
    if ((8*GetStringInfoLength(key)) >= 192)
      {
        n=6;
        aes_info->rounds=12;
      }

  datum=GetStringInfoDatum(aes_info->key);
  (void) memset(datum,0,GetStringInfoLength(aes_info->key));
  (void) memcpy(datum,GetStringInfoDatum(key),MagickMin(
    GetStringInfoLength(key),GetStringInfoLength(aes_info->key)));
  for (i=0; i < n; i++)
    aes_info->encipher_key[i]=(unsigned int) datum[4*i] |
      ((unsigned int) datum[4*i+1] << 8) |
      ((unsigned int) datum[4*i+2] << 16) |
      ((unsigned int) datum[4*i+3] << 24);
  beta=1;
  words=(AESBlocksize/4)*(aes_info->rounds+1);
  for (i=n; i < words; i++)
  {
    alpha=aes_info->encipher_key[i-1];
    if ((i % n) == 0)
      {
        alpha=ByteSubTransform(RotateRight(alpha),SBox) ^ beta;
        beta=XTime((unsigned char) (beta & 0xff));
      }
    else
      if ((n > 6) && ((i % n) == 4))
        alpha=ByteSubTransform(alpha,SBox);
    aes_info->encipher_key[i]=aes_info->encipher_key[i-n] ^ alpha;
  }
  datum=GetStringInfoDatum(aes_info->key);
  (void) memset(datum,0,GetStringInfoLength(aes_info->key));
  SecureZero(&alpha,sizeof(alpha));
  SecureZero(&beta,sizeof(beta));
}
#else

/*
  Disabled cipher support fallback stubs.
*/
MagickExport MagickBooleanType DecipherImage(Image *image,
  const char *passphrase,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) passphrase;
  ThrowBinaryException(ImageError,"CipherSupportNotEnabled",image->filename);
}

MagickExport MagickBooleanType PasskeyDecipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) passkey;
  ThrowBinaryException(ImageError,"CipherSupportNotEnabled",image->filename);
}

MagickExport MagickBooleanType EncipherImage(Image *image,
  const char *passphrase,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) passphrase;
  ThrowBinaryException(ImageError,"CipherSupportNotEnabled",image->filename);
}

MagickExport MagickBooleanType PasskeyEncipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) passkey;
  ThrowBinaryException(ImageError,"CipherSupportNotEnabled",image->filename);
}
#endif

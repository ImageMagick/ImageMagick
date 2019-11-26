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
%                               March  2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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

#if defined(MAGICKCORE_CIPHER_SUPPORT)
/*
  Define declarations.
*/
#define AESBlocksize 16

/*
  Typedef declarations.
*/
typedef struct _AESInfo
{
  StringInfo
    *key;

  unsigned int
    blocksize,
    *encipher_key,
    *decipher_key;

  ssize_t
    rounds,
    timestamp;

  size_t
    signature;
} AESInfo;

/*
  Global declarations.
*/
static unsigned char
  InverseLog[256] =
  {
      1,   3,   5,  15,  17,  51,  85, 255,  26,  46, 114, 150, 161, 248,
     19,  53,  95, 225,  56,  72, 216, 115, 149, 164, 247,   2,   6,  10,
     30,  34, 102, 170, 229,  52,  92, 228,  55,  89, 235,  38, 106, 190,
    217, 112, 144, 171, 230,  49,  83, 245,   4,  12,  20,  60,  68, 204,
     79, 209, 104, 184, 211, 110, 178, 205,  76, 212, 103, 169, 224,  59,
     77, 215,  98, 166, 241,   8,  24,  40, 120, 136, 131, 158, 185, 208,
    107, 189, 220, 127, 129, 152, 179, 206,  73, 219, 118, 154, 181, 196,
     87, 249,  16,  48,  80, 240,  11,  29,  39, 105, 187, 214,  97, 163,
    254,  25,  43, 125, 135, 146, 173, 236,  47, 113, 147, 174, 233,  32,
     96, 160, 251,  22,  58,  78, 210, 109, 183, 194,  93, 231,  50,  86,
    250,  21,  63,  65, 195,  94, 226,  61,  71, 201,  64, 192,  91, 237,
     44, 116, 156, 191, 218, 117, 159, 186, 213, 100, 172, 239,  42, 126,
    130, 157, 188, 223, 122, 142, 137, 128, 155, 182, 193,  88, 232,  35,
    101, 175, 234,  37, 111, 177, 200,  67, 197,  84, 252,  31,  33,  99,
    165, 244,   7,   9,  27,  45, 119, 153, 176, 203,  70, 202,  69, 207,
     74, 222, 121, 139, 134, 145, 168, 227,  62,  66, 198,  81, 243,  14,
     18,  54,  90, 238,  41, 123, 141, 140, 143, 138, 133, 148, 167, 242,
     13,  23,  57,  75, 221, 124, 132, 151, 162, 253,  28,  36, 108, 180,
    199,  82, 246,   1
  },
  Log[256] =
  {
      0,   0,  25,   1,  50,   2,  26, 198,  75, 199,  27, 104,  51, 238,
    223,   3, 100,   4, 224,  14,  52, 141, 129, 239,  76, 113,   8, 200,
    248, 105,  28, 193, 125, 194,  29, 181, 249, 185,  39, 106,  77, 228,
    166, 114, 154, 201,   9, 120, 101,  47, 138,   5,  33,  15, 225,  36,
     18, 240, 130,  69,  53, 147, 218, 142, 150, 143, 219, 189,  54, 208,
    206, 148,  19,  92, 210, 241,  64,  70, 131,  56, 102, 221, 253,  48,
    191,   6, 139,  98, 179,  37, 226, 152,  34, 136, 145,  16, 126, 110,
     72, 195, 163, 182,  30,  66,  58, 107,  40,  84, 250, 133,  61, 186,
     43, 121,  10,  21, 155, 159,  94, 202,  78, 212, 172, 229, 243, 115,
    167,  87, 175,  88, 168,  80, 244, 234, 214, 116,  79, 174, 233, 213,
    231, 230, 173, 232,  44, 215, 117, 122, 235,  22,  11, 245,  89, 203,
     95, 176, 156, 169,  81, 160, 127,  12, 246, 111,  23, 196,  73, 236,
    216,  67,  31,  45, 164, 118, 123, 183, 204, 187,  62,  90, 251,  96,
    177, 134,  59,  82, 161, 108, 170,  85,  41, 157, 151, 178, 135, 144,
     97, 190, 220, 252, 188, 149, 207, 205,  55,  63,  91, 209,  83,  57,
    132, 60,   65, 162, 109,  71,  20,  42, 158,  93,  86, 242, 211, 171,
     68,  17, 146, 217,  35,  32,  46, 137, 180, 124, 184,  38, 119, 153,
    227, 165, 103,  74, 237, 222, 197,  49, 254,  24,  13,  99, 140, 128,
    192, 247, 112,   7,
  },
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
  aes_info->encipher_key=(unsigned int *) AcquireQuantumMemory(60UL,sizeof(
    *aes_info->encipher_key));
  aes_info->decipher_key=(unsigned int *) AcquireQuantumMemory(60UL,sizeof(
    *aes_info->decipher_key));
  if ((aes_info->key == (StringInfo *) NULL) ||
      (aes_info->encipher_key == (unsigned int *) NULL) ||
      (aes_info->decipher_key == (unsigned int *) NULL))
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  aes_info->timestamp=(ssize_t) time(0);
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
static AESInfo *DestroyAESInfo(AESInfo *aes_info)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(aes_info != (AESInfo *) NULL);
  assert(aes_info->signature == MagickCoreSignature);
  if (aes_info->decipher_key != (unsigned int *) NULL)
    aes_info->decipher_key=(unsigned int *) RelinquishMagickMemory(
      aes_info->decipher_key);
  if (aes_info->encipher_key != (unsigned int *) NULL)
    aes_info->encipher_key=(unsigned int *) RelinquishMagickMemory(
      aes_info->encipher_key);
  if (aes_info->key != (StringInfo *) NULL)
    aes_info->key=DestroyStringInfo(aes_info->key);
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
%      void EncipherAES(AESInfo *aes_info,const unsigned char *plaintext,
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
  register ssize_t
    i;

  /*
    Xor corresponding text input and round key input bytes.
  */
  for (i=0; i < 4; i++)
    plaintext[i]=key[i] ^ ciphertext[i];
}

static inline unsigned char ByteMultiply(const unsigned char alpha,
  const unsigned char beta)
{
  /*
    Byte multiply two elements of GF(2^m) (mix columns and inverse mix columns).
  */
  if ((alpha == 0) || (beta == 0))
    return(0);
  return(InverseLog[(Log[alpha]+Log[beta]) % 0xff]);
}

static inline unsigned int ByteSubTransform(unsigned int x,
  unsigned char *s_box)
{
  unsigned int
    key;

  /*
    Non-linear layer resists differential and linear cryptoanalysis attacks.
  */
  key=(s_box[x & 0xff]) | (s_box[(x >> 8) & 0xff] << 8) |
    (s_box[(x >> 16) & 0xff] << 16) | (s_box[(x >> 24) & 0xff] << 24);
  return(key);
}

static void FinalizeRoundKey(const unsigned int *ciphertext,
  const unsigned int *key,unsigned char *plaintext)
{
  register unsigned char
    *p;

  register unsigned int
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
  /*
    Reset registers.
  */
  value=0;
}

static void InitializeRoundKey(const unsigned char *ciphertext,
  const unsigned int *key,unsigned int *plaintext)
{
  register const unsigned char
    *p;

  register unsigned int
    i,
    j;

  unsigned int
    value;

  p=ciphertext;
  for (i=0; i < 4; i++)
  {
    value=0;
    for (j=0; j < 4; j++)
      value|=(*p++ << (8*j));
    plaintext[i]=key[i] ^ value;
  }
  /*
    Reset registers.
  */
  value=0;
}

static inline unsigned int RotateLeft(const unsigned int x)
{
  return(((x << 8) | ((x >> 24) & 0xff)));
}

static void EncipherAESBlock(AESInfo *aes_info,const unsigned char *plaintext,
  unsigned char *ciphertext)
{
  register ssize_t
    i,
    j;

  static int
    map[4][4] =
    {
      { 0, 1, 2, 3 },
      { 1, 2, 3, 0 },
      { 2, 3, 0, 1 },
      { 3, 0, 1, 2 }
    };

  static unsigned int
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
    /*
      Linear mixing step: cause diffusion of the bits over multiple rounds.
    */
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
  /*
    Reset registers.
  */
  alpha=0;
  (void) memset(key,0,sizeof(key));
  (void) memset(text,0,sizeof(text));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P a s s k e y D e c i p h e r I m a g e                                 %
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
%      MagickBooleanType DecipherImage(Image *image,const char *passphrase,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o passphrase: decipher cipher pixels with this passphrase.
%
%    o passkey: decrypt cipher pixels with this passkey.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline void IncrementCipherNonce(const size_t length,
  unsigned char *nonce)
{
  register ssize_t
    i;

  for (i=(ssize_t) (length-1); i >= 0; i--)
  {
    nonce[i]++;
    if (nonce[i] != 0)
      return;
  }
  ThrowFatalException(ResourceLimitFatalError,"Sequence wrap error `%s'");
}

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
  passkey=DestroyStringInfo(passkey);
  return(status);
}

MagickExport MagickBooleanType PasskeyDecipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
{
#define DecipherImageTag  "Decipher/Image "

  AESInfo
    *aes_info;

  CacheView
    *image_view;

  const unsigned char
    *digest;

  MagickBooleanType
    proceed;

  MagickSizeType
    extent;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  SignatureInfo
    *signature_info;

  register unsigned char
    *p;

  size_t
    length;

  ssize_t
    y;

  StringInfo
    *key,
    *nonce;

  unsigned char
    input_block[AESBlocksize],
    output_block[AESBlocksize],
    *pixels;

  /*
    Generate decipher key and nonce.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (passkey == (const StringInfo *) NULL)
    return(MagickTrue);
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
      key=DestroyStringInfo(key);
      aes_info=DestroyAESInfo(aes_info);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  SetAESKey(aes_info,key);
  key=DestroyStringInfo(key);
  signature_info=AcquireSignatureInfo();
  UpdateSignature(signature_info,nonce);
  extent=(MagickSizeType) image->columns*image->rows;
  SetStringInfoLength(nonce,sizeof(extent));
  SetStringInfoDatum(nonce,(const unsigned char *) &extent);
  UpdateSignature(signature_info,nonce);
  nonce=DestroyStringInfo(nonce);
  FinalizeSignature(signature_info);
  (void) memset(input_block,0,sizeof(input_block));
  digest=GetStringInfoDatum(GetSignatureDigest(signature_info));
  (void) memcpy(input_block,digest,MagickMin(AESBlocksize,
    GetSignatureDigestsize(signature_info))*sizeof(*input_block));
  signature_info=DestroySignatureInfo(signature_info);
  /*
    Convert cipher pixels to plain pixels.
  */
  quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
  if (quantum_info == (QuantumInfo *) NULL)
    {
      aes_info=DestroyAESInfo(aes_info);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  quantum_type=GetQuantumType(image,exception);
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      i,
      x;

    register Quantum
      *magick_restrict q;

    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    length=ExportQuantumPixels(image,image_view,quantum_info,quantum_type,
      pixels,exception);
    p=pixels;
    for (x=0; x < (ssize_t) length; x+=AESBlocksize)
    {
      (void) memcpy(output_block,input_block,AESBlocksize*
        sizeof(*output_block));
      IncrementCipherNonce(AESBlocksize,input_block);
      EncipherAESBlock(aes_info,output_block,output_block);
      for (i=0; i < AESBlocksize; i++)
        p[i]^=output_block[i];
      p+=AESBlocksize;
    }
    (void) memcpy(output_block,input_block,AESBlocksize*
      sizeof(*output_block));
    EncipherAESBlock(aes_info,output_block,output_block);
    for (i=0; x < (ssize_t) length; x++)
    {
      p[i]^=output_block[i];
      i++;
    }
    (void) ImportQuantumPixels(image,image_view,quantum_info,quantum_type,
      pixels,exception);
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      break;
    proceed=SetImageProgress(image,DecipherImageTag,(MagickOffsetType) y,
      image->rows);
    if (proceed == MagickFalse)
      break;
  }
  image_view=DestroyCacheView(image_view);
  (void) DeleteImageProperty(image,"cipher:type");
  (void) DeleteImageProperty(image,"cipher:mode");
  (void) DeleteImageProperty(image,"cipher:nonce");
  image->taint=MagickFalse;
  /*
    Free resources.
  */
  quantum_info=DestroyQuantumInfo(quantum_info);
  aes_info=DestroyAESInfo(aes_info);
  (void) memset(input_block,0,sizeof(input_block));
  (void) memset(output_block,0,sizeof(output_block));
  return(y == (ssize_t) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P a s s k e y E n c i p h e r I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PasskeyEncipherImage() converts pixels to cipher-pixels.
%
%  The format of the PasskeyEncipherImage method is:
%
%      MagickBooleanType PasskeyEncipherImage(Image *image,
%        const StringInfo *passkey,ExceptionInfo *exception)
%      MagickBooleanType EncipherImage(Image *image,const char *passphrase,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o passphrase: encipher pixels with this passphrase.
%
%    o passkey: decrypt cipher pixels with this passkey.
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
  passkey=DestroyStringInfo(passkey);
  return(status);
}

MagickExport MagickBooleanType PasskeyEncipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
{
#define EncipherImageTag  "Encipher/Image "

  AESInfo
    *aes_info;

  CacheView
    *image_view;

  char
    *signature;

  const unsigned char
    *digest;

  MagickBooleanType
    proceed;

  MagickSizeType
    extent;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register unsigned char
    *p;

  SignatureInfo
    *signature_info;

  size_t
    length;

  ssize_t
    y;

  StringInfo
    *key,
    *nonce;

  unsigned char
    input_block[AESBlocksize],
    output_block[AESBlocksize],
    *pixels;

  /*
    Generate encipher key and nonce.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (passkey == (const StringInfo *) NULL)
    return(MagickTrue);
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
      key=DestroyStringInfo(key);
      aes_info=DestroyAESInfo(aes_info);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  SetAESKey(aes_info,key);
  key=DestroyStringInfo(key);
  signature_info=AcquireSignatureInfo();
  UpdateSignature(signature_info,nonce);
  extent=(MagickSizeType) image->columns*image->rows;
  SetStringInfoLength(nonce,sizeof(extent));
  SetStringInfoDatum(nonce,(const unsigned char *) &extent);
  UpdateSignature(signature_info,nonce);
  nonce=DestroyStringInfo(nonce);
  FinalizeSignature(signature_info);
  signature=StringInfoToHexString(GetSignatureDigest(signature_info));
  (void) SetImageProperty(image,"cipher:type","AES",exception);
  (void) SetImageProperty(image,"cipher:mode","CTR",exception);
  (void) SetImageProperty(image,"cipher:nonce",signature,exception);
  signature=DestroyString(signature);
  (void) memset(input_block,0,sizeof(input_block));
  digest=GetStringInfoDatum(GetSignatureDigest(signature_info));
  (void) memcpy(input_block,digest,MagickMin(AESBlocksize,
    GetSignatureDigestsize(signature_info))*sizeof(*input_block));
  signature_info=DestroySignatureInfo(signature_info);
  /*
    Convert plain pixels to cipher pixels.
  */
  quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
  if (quantum_info == (QuantumInfo *) NULL)
    {
      aes_info=DestroyAESInfo(aes_info);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  quantum_type=GetQuantumType(image,exception);
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      i,
      x;

    register Quantum
      *magick_restrict q;

    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    length=ExportQuantumPixels(image,image_view,quantum_info,quantum_type,
      pixels,exception);
    p=pixels;
    for (x=0; x < (ssize_t) length; x+=AESBlocksize)
    {
      (void) memcpy(output_block,input_block,AESBlocksize*
        sizeof(*output_block));
      IncrementCipherNonce(AESBlocksize,input_block);
      EncipherAESBlock(aes_info,output_block,output_block);
      for (i=0; i < AESBlocksize; i++)
        p[i]^=output_block[i];
      p+=AESBlocksize;
    }
    (void) memcpy(output_block,input_block,AESBlocksize*
      sizeof(*output_block));
    EncipherAESBlock(aes_info,output_block,output_block);
    for (i=0; x < (ssize_t) length; x++)
    {
      p[i]^=output_block[i];
      i++;
    }
    (void) ImportQuantumPixels(image,image_view,quantum_info,quantum_type,
      pixels,exception);
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      break;
    proceed=SetImageProgress(image,EncipherImageTag,(MagickOffsetType) y,
      image->rows);
    if (proceed == MagickFalse)
      break;
  }
  image_view=DestroyCacheView(image_view);
  image->taint=MagickFalse;
  /*
    Free resources.
  */
  quantum_info=DestroyQuantumInfo(quantum_info);
  aes_info=DestroyAESInfo(aes_info);
  (void) memset(input_block,0,sizeof(input_block));
  (void) memset(output_block,0,sizeof(output_block));
  return(y == (ssize_t) image->rows ? MagickTrue : MagickFalse);
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
%  SetAESKey() sets the key for the AES cipher.  The key length is specified
%  in bits.  Valid values are 128, 192, or 256 requiring a key buffer length
%  in bytes of 16, 24, and 32 respectively.
%
%  The format of the SetAESKey method is:
%
%      SetAESKey(AESInfo *aes_info,const StringInfo *key)
%
%  A description of each parameter follows:
%
%    o aes_info: the cipher context.
%
%    o key: the key.
%
*/

static inline void InverseAddRoundKey(const unsigned int *alpha,
  unsigned int *beta)
{
  register unsigned int
    i,
    j;

  for (i=0; i < 4; i++)
  {
    beta[i]=0;
    for (j=0; j < 4; j++)
      beta[i]|=(ByteMultiply(0xe,(alpha[i] >> (8*j)) & 0xff) ^
        ByteMultiply(0xb,(alpha[i] >> (8*((j+1) % 4))) & 0xff) ^
        ByteMultiply(0xd,(alpha[i] >> (8*((j+2) % 4))) & 0xff) ^
        ByteMultiply(0x9,(alpha[i] >> (8*((j+3) % 4))) & 0xff)) << (8*j);
  }
}

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
  register ssize_t
    i;

  ssize_t
    bytes,
    n;

  unsigned char
    *datum;

  unsigned int
    alpha,
    beta;

  /*
    Determine the number of rounds based on the number of bits in key.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(aes_info != (AESInfo *) NULL);
  assert(aes_info->signature == MagickCoreSignature);
  assert(key != (StringInfo *) NULL);
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
  /*
    Generate crypt key.
  */
  datum=GetStringInfoDatum(aes_info->key);
  (void) memset(datum,0,GetStringInfoLength(aes_info->key));
  (void) memcpy(datum,GetStringInfoDatum(key),MagickMin(
    GetStringInfoLength(key),GetStringInfoLength(aes_info->key)));
  for (i=0; i < n; i++)
    aes_info->encipher_key[i]=datum[4*i] | (datum[4*i+1] << 8) |
      (datum[4*i+2] << 16) | (datum[4*i+3] << 24);
  beta=1;
  bytes=(AESBlocksize/4)*(aes_info->rounds+1);
  for (i=n; i < bytes; i++)
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
  /*
    Generate deciper key (in reverse order).
  */
  for (i=0; i < 4; i++)
  {
    aes_info->decipher_key[i]=aes_info->encipher_key[i];
    aes_info->decipher_key[bytes-4+i]=aes_info->encipher_key[bytes-4+i];
  }
  for (i=4; i < (bytes-4); i+=4)
    InverseAddRoundKey(aes_info->encipher_key+i,aes_info->decipher_key+i);
  /*
    Reset registers.
  */
  datum=GetStringInfoDatum(aes_info->key);
  (void) memset(datum,0,GetStringInfoLength(aes_info->key));
  alpha=0;
  beta=0;
}
#else

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P a s s k e y D e c i p h e r I m a g e                                 %
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
%      MagickBooleanType DecipherImage(Image *image,const char *passphrase,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o passphrase: decipher cipher pixels with this passphrase.
%
%    o passkey: decrypt cipher pixels with this passkey.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType DecipherImage(Image *image,
  const char *passphrase,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  (void) passphrase;
  ThrowBinaryException(ImageError,"CipherSupportNotEnabled",image->filename);
}

MagickExport MagickBooleanType PasskeyDecipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  (void) passkey;
  ThrowBinaryException(ImageError,"CipherSupportNotEnabled",image->filename);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P a s s k e y E n c i p h e r I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PasskeyEncipherImage() converts pixels to cipher-pixels.
%
%  The format of the PasskeyEncipherImage method is:
%
%      MagickBooleanType PasskeyEncipherImage(Image *image,
%        const StringInfo *passkey,ExceptionInfo *exception)
%      MagickBooleanType EncipherImage(Image *image,const char *passphrase,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o passphrase: decipher cipher pixels with this passphrase.
%
%    o passkey: decrypt cipher pixels with this passkey.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType EncipherImage(Image *image,
  const char *passphrase,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  (void) passphrase;
  ThrowBinaryException(ImageError,"CipherSupportNotEnabled",image->filename);
}

MagickExport MagickBooleanType PasskeyEncipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  (void) passkey;
  ThrowBinaryException(ImageError,"CipherSupportNotEnabled",image->filename);
}
#endif

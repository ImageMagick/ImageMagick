/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               V   V  EEEEE  RRRR   SSSSS  IIIII   OOO   N   N               %
%               V   V  E      R   R  SS       I    O   O  NN  N               %
%               V   V  EEE    RRRR    SSS     I    O   O  N N N               %
%                V V   E      R R       SS    I    O   O  N  NN               %
%                 V    EEEEE  R  R   SSSSS  IIIII   OOO   N   N               %
%                                                                             %
%                                                                             %
%                   MagickCore Version and Copyright Methods                  %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               September 2002                                %
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

#include "MagickCore/studio.h"
#include "MagickCore/configure.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/locale_.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/string_.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"
#include "MagickCore/version-private.h"

/*
  Define declarations.
*/
#define MagickURLFilename  "index.html"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k C o p y r i g h t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickCopyright() returns the ImageMagick API copyright as a string.
%
%  The format of the GetMagickCopyright method is:
%
%      const char *GetMagickCopyright(void)
%
%  No parameters are required.
%
*/
MagickExport const char *GetMagickCopyright(void)
{
  return(MagickCopyright);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k D e l e g a t e s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickDelegates() returns the ImageMagick delegate libraries.
%
%  The format of the GetMagickDelegates method is:
%
%      const char *GetMagickDelegates(void)
%
%  No parameters are required.
%
*/
MagickExport const char *GetMagickDelegates(void)
{
  static const char
    *delegates=""
#if defined(MAGICKCORE_AUTOTRACE_DELEGATE)
  " autotrace"
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
  " bzlib"
#endif
#if defined(MAGICKCORE_CAIRO_DELEGATE)
  " cairo"
#endif
#if defined(MAGICKCORE_DMR_DELEGATE)
  " dmr"
#endif
#if defined(MAGICKCORE_DJVU_DELEGATE)
  " djvu"
#endif
#if defined(MAGICKCORE_DPS_DELEGATE)
  " dps"
#endif
#if defined(MAGICKCORE_EMF_DELEGATE)
  " emf"
#endif
#if defined(MAGICKCORE_FFTW_DELEGATE)
  " fftw"
#endif
#if defined(MAGICKCORE_FLIF_DELEGATE)
  " flif"
#endif
#if defined(MAGICKCORE_FONTCONFIG_DELEGATE)
  " fontconfig"
#endif
#if defined(MAGICKCORE_FPX_DELEGATE)
  " fpx"
#endif
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  " freetype"
#endif
#if defined(MAGICKCORE_GS_DELEGATE) || defined(MAGICKCORE_WINDOWS_SUPPORT)
  " gslib"
#endif
#if defined(MAGICKCORE_GVC_DELEGATE)
  " gvc"
#endif
#if defined(MAGICKCORE_HEIC_DELEGATE)
  " heic"
#endif
#if defined(MAGICKCORE_JBIG_DELEGATE)
  " jbig"
#endif
#if defined(MAGICKCORE_JPEG_DELEGATE) && defined(MAGICKCORE_PNG_DELEGATE)
  " jng"
#endif
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  " jp2"
#endif
#if defined(MAGICKCORE_JPEG_DELEGATE)
  " jpeg"
#endif
#if defined(MAGICKCORE_JXL_DELEGATE)
  " jxl"
#endif
#if defined(MAGICKCORE_LCMS_DELEGATE)
  " lcms"
#endif
#if defined(MAGICKCORE_LQR_DELEGATE)
  " lqr"
#endif
#if defined(MAGICKCORE_LTDL_DELEGATE)
  " ltdl"
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
  " lzma"
#endif
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
  " openexr"
#endif
#if defined(MAGICKCORE_PANGOCAIRO_DELEGATE)
  " pangocairo"
#endif
#if defined(MAGICKCORE_PNG_DELEGATE)
  " png"
#endif
#if defined(MAGICKCORE_DPS_DELEGATE) || defined(MAGICKCORE_GS_DELEGATE) || \
    defined(MAGICKCORE_WINDOWS_SUPPORT)
  " ps"
#endif
#if defined(MAGICKCORE_RAQM_DELEGATE)
  " raqm"
#endif
#if defined(MAGICKCORE_RAW_R_DELEGATE)
  " raw"
#endif
#if defined(MAGICKCORE_RSVG_DELEGATE)
  " rsvg"
#endif
#if defined(MAGICKCORE_TIFF_DELEGATE)
  " tiff"
#endif
#if defined(MAGICKCORE_UHDR_DELEGATE)
  " uhdr"
#endif
#if defined(MAGICKCORE_WEBP_DELEGATE)
  " webp"
#endif
#if defined(MAGICKCORE_WMF_DELEGATE) || defined(MAGICKCORE_WMFLITE_DELEGATE)
  " wmf"
#endif
#if defined(MAGICKCORE_X11_DELEGATE)
  " x"
#endif
#if defined(MAGICKCORE_XML_DELEGATE)
  " xml"
#endif
#if defined(MAGICKCORE_ZIP_DELEGATE)
  " zip"
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
  " zlib"
#endif
#if defined(MAGICKCORE_ZSTD_DELEGATE)
  " zstd"
#endif
  ;
  if (*delegates == '\0')
    return(delegates);
  return(delegates+1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k F e a t u r e s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickFeatures() returns the ImageMagick features.
%
%  The format of the GetMagickFeatures method is:
%
%      const char *GetMagickFeatures(void)
%
%  No parameters are required.
%
*/
MagickExport const char *GetMagickFeatures(void)
{
  return ""
#if defined(MAGICKCORE_WINDOWS_SUPPORT) && defined(_DEBUG)
  "Debug "
#endif
#if defined(MAGICKCORE_64BIT_CHANNEL_MASK_SUPPORT)
  "Channel-masks(64-bit) "
#endif
#if defined(MAGICKCORE_CIPHER_SUPPORT)
  "Cipher "
#endif
#if defined(MAGICKCORE_DPC_SUPPORT)
  "DPC "
#endif
#if defined(MAGICKCORE_HDRI_SUPPORT)
  "HDRI "
#endif
#if defined(MAGICKCORE_BUILD_MODULES)
  "Modules "
#endif
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  "OpenCL "
#endif
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  "OpenMP"
#if _OPENMP == 199810
  "(1.0) "
#elif _OPENMP == 200203
  "(2.0) "
#elif _OPENMP == 200505
  "(2.5) "
#elif _OPENMP == 200805
  "(3.0) "
#elif _OPENMP == 201107
  "(3.1) "
#elif _OPENMP == 201307
  "(4.0) "
#elif _OPENMP == 201511
  "(4.5) "
#elif _OPENMP == 201811
  "(5.0) "
#elif _OPENMP == 202011
  "(5.1) "
#elif _OPENMP == 202111
  "(5.2) "
#elif _OPENMP == 202411
  "(6.0) "
#else
  " "
#endif
#endif
#if defined(MAGICKCORE_HAVE_TCMALLOC)
  "TCMalloc "
#endif
#if MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
  "Zero-configuration "
#endif
#if (MAGICKCORE_QUANTUM_DEPTH == 64)
  "Q64 (experimental, not for production) "
#endif
  ;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k H o m e U R L                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickHomeURL() returns the ImageMagick home URL.
%
%  The format of the GetMagickHomeURL method is:
%
%      char *GetMagickHomeURL(void)
%
%  No parameters are required.
%
%  The returned string is allocated; the caller is responsible for releasing
%  it with DestroyString() or RelinquishMagickMemory().
%
*/
MagickExport char *GetMagickHomeURL(void)
{
  char
    path[MagickPathExtent];

  const char
    *element;

  ExceptionInfo
    *exception;

  LinkedListInfo
    *paths;

  exception=AcquireExceptionInfo();
  paths=GetConfigurePaths(MagickURLFilename,exception);
  exception=DestroyExceptionInfo(exception);
  if (paths == (LinkedListInfo *) NULL)
    return(ConstantString(MagickHomeURL));
  element=(const char *) GetNextValueInLinkedList(paths);
  while (element != (const char *) NULL)
  {
    (void) FormatLocaleString(path,MagickPathExtent,"%s%s%s",element,
      DirectorySeparator,MagickURLFilename);
    if (IsPathAccessible(path) != MagickFalse)
      {
        paths=DestroyLinkedList(paths,RelinquishMagickMemory);
        return(ConstantString(path));
      }
    element=(const char *) GetNextValueInLinkedList(paths);
  }
  paths=DestroyLinkedList(paths,RelinquishMagickMemory);
  return(ConstantString(MagickHomeURL));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k L i c e n s e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickLicense() returns the ImageMagick API license as a string.
%
%  The format of the GetMagickLicense method is:
%
%      const char *GetMagickLicense(void)
%
%  No parameters are required.
%
*/
MagickExport const char *GetMagickLicense(void)
{
  return(MagickAuthoritativeLicense);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k P a c k a g e N a m e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickPackageName() returns the ImageMagick package name.
%
%  The format of the GetMagickPackageName method is:
%
%      const char *GetMagickPackageName(void)
%
%  No parameters are required.
%
*/
MagickExport const char *GetMagickPackageName(void)
{
  return(MagickPackageName);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k Q u a n t u m D e p t h                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickQuantumDepth() returns the ImageMagick quantum depth.
%
%  The format of the GetMagickQuantumDepth method is:
%
%      const char *GetMagickQuantumDepth(size_t *depth)
%
%  A description of each parameter follows:
%
%    o depth: the quantum depth is returned as a number.
%
*/
MagickExport const char *GetMagickQuantumDepth(size_t *depth)
{
  if (depth != (size_t *) NULL)
    *depth=(size_t) MAGICKCORE_QUANTUM_DEPTH;
  return(MagickQuantumDepth);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k Q u a n t u m R a n g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickQuantumRange() returns the ImageMagick quantum range.
%
%  The format of the GetMagickQuantumRange method is:
%
%      const char *GetMagickQuantumRange(size_t *range)
%
%  A description of each parameter follows:
%
%    o range: the quantum range is returned as a number.
%
*/
MagickExport const char *GetMagickQuantumRange(size_t *range)
{
  if (range != (size_t *) NULL)
    *range=(size_t) QuantumRange;
  return(MagickQuantumRange);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k R e l e a s e D a t e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickReleaseDate() returns the ImageMagick release date.
%
%  The format of the GetMagickReleaseDate method is:
%
%      const char *GetMagickReleaseDate(void)
%
%  No parameters are required.
%
*/
MagickExport const char *GetMagickReleaseDate(void)
{
  return(MagickReleaseDate);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C R C 3 2                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CRC32() returns a 32-bit cyclic redundancy check (polynomial 0xEDB88320,
%  reflected) of the message.  The lookup table is precomputed so the routine
%  has no mutable static state and is safe to call from multiple threads.
%
%  The format of the CRC32 method is:
%
%      unsigned int CRC32(const unsigned char *message,const size_t length)
%
%  A description of each parameter follows:
%
%    o message: the data to checksum.
%
%    o length: the number of bytes in message.
%
*/
static unsigned int CRC32(const unsigned char *message,const size_t length)
{
  static const unsigned int
    crc_xor[256] =
    {
      0x00000000U, 0x77073096U, 0xee0e612cU, 0x990951baU,
      0x076dc419U, 0x706af48fU, 0xe963a535U, 0x9e6495a3U,
      0x0edb8832U, 0x79dcb8a4U, 0xe0d5e91eU, 0x97d2d988U,
      0x09b64c2bU, 0x7eb17cbdU, 0xe7b82d07U, 0x90bf1d91U,
      0x1db71064U, 0x6ab020f2U, 0xf3b97148U, 0x84be41deU,
      0x1adad47dU, 0x6ddde4ebU, 0xf4d4b551U, 0x83d385c7U,
      0x136c9856U, 0x646ba8c0U, 0xfd62f97aU, 0x8a65c9ecU,
      0x14015c4fU, 0x63066cd9U, 0xfa0f3d63U, 0x8d080df5U,
      0x3b6e20c8U, 0x4c69105eU, 0xd56041e4U, 0xa2677172U,
      0x3c03e4d1U, 0x4b04d447U, 0xd20d85fdU, 0xa50ab56bU,
      0x35b5a8faU, 0x42b2986cU, 0xdbbbc9d6U, 0xacbcf940U,
      0x32d86ce3U, 0x45df5c75U, 0xdcd60dcfU, 0xabd13d59U,
      0x26d930acU, 0x51de003aU, 0xc8d75180U, 0xbfd06116U,
      0x21b4f4b5U, 0x56b3c423U, 0xcfba9599U, 0xb8bda50fU,
      0x2802b89eU, 0x5f058808U, 0xc60cd9b2U, 0xb10be924U,
      0x2f6f7c87U, 0x58684c11U, 0xc1611dabU, 0xb6662d3dU,
      0x76dc4190U, 0x01db7106U, 0x98d220bcU, 0xefd5102aU,
      0x71b18589U, 0x06b6b51fU, 0x9fbfe4a5U, 0xe8b8d433U,
      0x7807c9a2U, 0x0f00f934U, 0x9609a88eU, 0xe10e9818U,
      0x7f6a0dbbU, 0x086d3d2dU, 0x91646c97U, 0xe6635c01U,
      0x6b6b51f4U, 0x1c6c6162U, 0x856530d8U, 0xf262004eU,
      0x6c0695edU, 0x1b01a57bU, 0x8208f4c1U, 0xf50fc457U,
      0x65b0d9c6U, 0x12b7e950U, 0x8bbeb8eaU, 0xfcb9887cU,
      0x62dd1ddfU, 0x15da2d49U, 0x8cd37cf3U, 0xfbd44c65U,
      0x4db26158U, 0x3ab551ceU, 0xa3bc0074U, 0xd4bb30e2U,
      0x4adfa541U, 0x3dd895d7U, 0xa4d1c46dU, 0xd3d6f4fbU,
      0x4369e96aU, 0x346ed9fcU, 0xad678846U, 0xda60b8d0U,
      0x44042d73U, 0x33031de5U, 0xaa0a4c5fU, 0xdd0d7cc9U,
      0x5005713cU, 0x270241aaU, 0xbe0b1010U, 0xc90c2086U,
      0x5768b525U, 0x206f85b3U, 0xb966d409U, 0xce61e49fU,
      0x5edef90eU, 0x29d9c998U, 0xb0d09822U, 0xc7d7a8b4U,
      0x59b33d17U, 0x2eb40d81U, 0xb7bd5c3bU, 0xc0ba6cadU,
      0xedb88320U, 0x9abfb3b6U, 0x03b6e20cU, 0x74b1d29aU,
      0xead54739U, 0x9dd277afU, 0x04db2615U, 0x73dc1683U,
      0xe3630b12U, 0x94643b84U, 0x0d6d6a3eU, 0x7a6a5aa8U,
      0xe40ecf0bU, 0x9309ff9dU, 0x0a00ae27U, 0x7d079eb1U,
      0xf00f9344U, 0x8708a3d2U, 0x1e01f268U, 0x6906c2feU,
      0xf762575dU, 0x806567cbU, 0x196c3671U, 0x6e6b06e7U,
      0xfed41b76U, 0x89d32be0U, 0x10da7a5aU, 0x67dd4accU,
      0xf9b9df6fU, 0x8ebeeff9U, 0x17b7be43U, 0x60b08ed5U,
      0xd6d6a3e8U, 0xa1d1937eU, 0x38d8c2c4U, 0x4fdff252U,
      0xd1bb67f1U, 0xa6bc5767U, 0x3fb506ddU, 0x48b2364bU,
      0xd80d2bdaU, 0xaf0a1b4cU, 0x36034af6U, 0x41047a60U,
      0xdf60efc3U, 0xa867df55U, 0x316e8eefU, 0x4669be79U,
      0xcb61b38cU, 0xbc66831aU, 0x256fd2a0U, 0x5268e236U,
      0xcc0c7795U, 0xbb0b4703U, 0x220216b9U, 0x5505262fU,
      0xc5ba3bbeU, 0xb2bd0b28U, 0x2bb45a92U, 0x5cb36a04U,
      0xc2d7ffa7U, 0xb5d0cf31U, 0x2cd99e8bU, 0x5bdeae1dU,
      0x9b64c2b0U, 0xec63f226U, 0x756aa39cU, 0x026d930aU,
      0x9c0906a9U, 0xeb0e363fU, 0x72076785U, 0x05005713U,
      0x95bf4a82U, 0xe2b87a14U, 0x7bb12baeU, 0x0cb61b38U,
      0x92d28e9bU, 0xe5d5be0dU, 0x7cdcefb7U, 0x0bdbdf21U,
      0x86d3d2d4U, 0xf1d4e242U, 0x68ddb3f8U, 0x1fda836eU,
      0x81be16cdU, 0xf6b9265bU, 0x6fb077e1U, 0x18b74777U,
      0x88085ae6U, 0xff0f6a70U, 0x66063bcaU, 0x11010b5cU,
      0x8f659effU, 0xf862ae69U, 0x616bffd3U, 0x166ccf45U,
      0xa00ae278U, 0xd70dd2eeU, 0x4e048354U, 0x3903b3c2U,
      0xa7672661U, 0xd06016f7U, 0x4969474dU, 0x3e6e77dbU,
      0xaed16a4aU, 0xd9d65adcU, 0x40df0b66U, 0x37d83bf0U,
      0xa9bcae53U, 0xdebb9ec5U, 0x47b2cf7fU, 0x30b5ffe9U,
      0xbdbdf21cU, 0xcabac28aU, 0x53b39330U, 0x24b4a3a6U,
      0xbad03605U, 0xcdd70693U, 0x54de5729U, 0x23d967bfU,
      0xb3667a2eU, 0xc4614ab8U, 0x5d681b02U, 0x2a6f2b94U,
      0xb40bbe37U, 0xc30c8ea1U, 0x5a05df1bU, 0x2d02ef8dU
    };

  size_t
    i;

  unsigned int
    crc;

  /*
    Generate a 32-bit cyclic redundancy check for the message.
  */
  crc=0xFFFFFFFFU;
  for (i=0; i < length; i++)
    crc=crc_xor[(crc ^ message[i]) & 0xff] ^ (crc >> 8);
  return(crc ^ 0xFFFFFFFFU);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k S i g n a t u r e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickSignature() returns a signature that uniquely encodes the
%  MagickCore library version, quantum depth, HDRI status, OS word size,
%  channel type, and endianness.
%
%  The format of the GetMagickSignature method is:
%
%      unsigned int GetMagickSignature(const StringInfo *nonce)
%
%  A description of each parameter follows:
%
%    o nonce: arbitrary data.
%
*/
MagickExport unsigned int GetMagickSignature(const StringInfo *nonce)
{
  unsigned char
    *p;

  StringInfo
    *version;

  unsigned int
    signature;

  version=AcquireStringInfo(MagickPathExtent);
  p=GetStringInfoDatum(version);
  signature=MAGICKCORE_QUANTUM_DEPTH;
  (void) memcpy(p,&signature,sizeof(signature));
  p+=(ptrdiff_t) sizeof(signature);
  signature=MAGICKCORE_HDRI_ENABLE;
  (void) memcpy(p,&signature,sizeof(signature));
  p+=(ptrdiff_t) sizeof(signature);
  signature=MagickLibInterface;
  (void) memcpy(p,&signature,sizeof(signature));
  p+=(ptrdiff_t) sizeof(signature);
  signature=1;  /* endianness */
  (void) memcpy(p,&signature,sizeof(signature));
  p+=(ptrdiff_t) sizeof(signature);
#if defined(MAGICKCORE_64BIT_CHANNEL_MASK_SUPPORT)
  signature=(unsigned int) sizeof(ChannelType);
  (void) memcpy(p,&signature,sizeof(signature));
  p+=(ptrdiff_t) sizeof(signature);
#endif
  SetStringInfoLength(version,(size_t) (p-GetStringInfoDatum(version)));
  if (nonce != (const StringInfo *) NULL)
    ConcatenateStringInfo(version,nonce);
  signature=CRC32(GetStringInfoDatum(version),GetStringInfoLength(version));
  version=DestroyStringInfo(version);
  return(signature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k V e r s i o n                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickVersion() returns the ImageMagick API version as a string and
%  as a number.
%
%  The format of the GetMagickVersion method is:
%
%      const char *GetMagickVersion(size_t *version)
%
%  A description of each parameter follows:
%
%    o version: the ImageMagick version is returned as a number.
%
*/
MagickExport const char *GetMagickVersion(size_t *version)
{
  if (version != (size_t *) NULL)
    *version=MagickLibVersion;
  return(MagickVersion);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i s t M a g i c k V e r s i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListMagickVersion() identifies the ImageMagick version by printing its
%  attributes to the file.  Attributes include the copyright, features, and
%  delegates.
%
%  The format of the ListMagickVersion method is:
%
%      void ListMagickVersion(FILE *file)
%
%  A description of each parameter follows:
%
%    o file: the file, typically stdout.
%
*/
MagickExport void ListMagickVersion(FILE *file)
{
  (void) FormatLocaleFile(file,"Version: %s\n",
    GetMagickVersion((size_t *) NULL));
  (void) FormatLocaleFile(file,"Copyright: %s\n",GetMagickCopyright());
  (void) FormatLocaleFile(file,"License: %s\n",GetMagickLicense());
  (void) FormatLocaleFile(file,"Features: %s\n",GetMagickFeatures());
  (void) FormatLocaleFile(file,"Delegates (built-in): %s\n",
    GetMagickDelegates());
#if defined(MAGICKCORE_MSC_VER)
  (void) FormatLocaleFile(file,"Compiler: Visual Studio %d (%d)\n",
    MAGICKCORE_MSC_VER,_MSC_FULL_VER);
#elif defined(__clang__)
  (void) FormatLocaleFile(file,"Compiler: clang (%d.%d.%d)\n",__clang_major__,
    __clang_minor__,__clang_patchlevel__);
#elif defined(__MINGW64_VERSION_MAJOR)
  (void) FormatLocaleFile(file,"Compiler: MinGW-w64 (%d.%d)\n",
    __MINGW64_VERSION_MAJOR,__MINGW64_VERSION_MINOR);
#elif defined(__MINGW32_MAJOR_VERSION)
  (void) FormatLocaleFile(file,"Compiler: MinGW (%d.%d)\n",
    __MINGW32_MAJOR_VERSION,__MINGW32_MINOR_VERSION);
#elif defined(__GNUC__)
  (void) FormatLocaleFile(file,"Compiler: gcc (%d.%d)\n",__GNUC__,
    __GNUC_MINOR__);
#endif
  if (IsEventLogging() != MagickFalse)
    {
      (void) FormatLocaleFile(file,"Wizard attributes: ");
      (void) FormatLocaleFile(file,"QuantumRange=%g; ",(double) QuantumRange);
      (void) FormatLocaleFile(file,"QuantumScale=%.*g; ",GetMagickPrecision(),
        (double) QuantumScale);
      (void) FormatLocaleFile(file,"MagickEpsilon=%.*g; ",GetMagickPrecision(),
        (double) MagickEpsilon);
      (void) FormatLocaleFile(file,"MaxMap=%g; ",(double) MaxMap);
      (void) FormatLocaleFile(file,"MagickPathExtent=%g; ",
        (double) MagickPathExtent);
      (void) FormatLocaleFile(file,"sizeof(Quantum)=%g; ",(double)
        sizeof(Quantum));
      (void) FormatLocaleFile(file,"sizeof(MagickSizeType)=%g; ",(double)
        sizeof(MagickSizeType));
      (void) FormatLocaleFile(file,"sizeof(MagickOffsetType)=%g",(double)
        sizeof(MagickOffsetType));
      (void) FormatLocaleFile(file,"\n");
    }
}

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
#if defined(MAGICKCORE_WMF_DELEGATE) || defined (MAGICKCORE_WMFLITE_DELEGATE)
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
%  The format of the GetMagickName method is:
%
%      const char *GetMagickName(void)
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

static unsigned int CRC32(const unsigned char *message,const size_t length)
{
  ssize_t
    i;

  static MagickBooleanType
    crc_initial = MagickFalse;

  static unsigned int
    crc_xor[256];

  unsigned int
    crc;

  /*
    Generate a 32-bit cyclic redundancy check for the message.
  */
  if (crc_initial == MagickFalse)
    {
      unsigned int
        j;

      unsigned int
        alpha;

      for (j=0; j < 256; j++)
      {
        ssize_t
          k;

        alpha=j;
        for (k=0; k < 8; k++)
          alpha=(alpha & 0x01) ? (0xEDB88320 ^ (alpha >> 1)) : (alpha >> 1);
        crc_xor[j]=alpha;
      }
      crc_initial=MagickTrue;
    }
  crc=0xFFFFFFFF;
  for (i=0; i < (ssize_t) length; i++)
    crc=crc_xor[(crc ^ message[i]) & 0xff] ^ (crc >> 8);
  return(crc ^ 0xFFFFFFFF);
}

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
  signature=sizeof(ChannelType);
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
    GetMagickVersion((size_t *) NULL));;
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
#elif defined(__GNUC__)
  (void) FormatLocaleFile(file,"Compiler: gcc (%d.%d)\n",__GNUC__,
    __GNUC_MINOR__);
#elif defined(__MINGW32_MAJOR_VERSION)
  (void) FormatLocaleFile(file,"Compiler: MinGW (%d.%d)\n",
    __MINGW32_MAJOR_VERSION,__MINGW32_MINOR_VERSION);
#elif defined(__MINGW64_VERSION_MAJOR)
  (void) FormatLocaleFile(file,"Compiler: MinGW-w64 (%d.%d)\n",
    __MINGW64_VERSION_MAJOR ,__MINGW64_VERSION_MINOR);
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
    }
}

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
*/

#include "magick/studio.h"
#include "magick/configure.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/locale_.h"
#include "magick/option.h"
#include "magick/string_.h"
#include "magick/utility.h"
#include "magick/version.h"
#include "magick/version-private.h"

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
  return ""
#if defined(MAGICKCORE_AUTOTRACE_DELEGATE)
  "autotrace "
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
  "bzlib "
#endif
#if defined(MAGICKCORE_CAIRO_DELEGATE)
  "cairo "
#endif
#if defined(MAGICKCORE_DJVU_DELEGATE)
  "djvu "
#endif
#if defined(MAGICKCORE_DPS_DELEGATE)
  "dps "
#endif
#if defined(MAGICKCORE_EMF_DELEGATE)
  "emf "
#endif
#if defined(MAGICKCORE_FFTW_DELEGATE)
  "fftw "
#endif
#if defined(MAGICKCORE_FONTCONFIG_DELEGATE)
  "fontconfig "
#endif
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  "freetype "
#endif
#if defined(MAGICKCORE_FPX_DELEGATE)
  "fpx "
#endif
#if defined(MAGICKCORE_GS_DELEGATE)
  "gslib "
#endif
#if defined(MAGICKCORE_GVC_DELEGATE)
  "gvc "
#endif
#if defined(MAGICKCORE_JBIG_DELEGATE)
  "jbig "
#endif
#if defined(MAGICKCORE_JPEG_DELEGATE) && defined(MAGICKCORE_PNG_DELEGATE)
  "jng "
#endif
#if defined(MAGICKCORE_JP2_DELEGATE)
  "jp2 "
#endif
#if defined(MAGICKCORE_JPEG_DELEGATE)
  "jpeg "
#endif
#if defined(MAGICKCORE_LCMS_DELEGATE)
  "lcms "
#endif
#if defined(MAGICKCORE_LQR_DELEGATE)
  "lqr "
#endif
#if defined(MAGICKCORE_LTDL_DELEGATE)
  "ltdl "
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
  "lzma "
#endif
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
  "openexr "
#endif
#if defined(MAGICKCORE_PANGOCAIRO_DELEGATE)
  "pangocairo "
#endif
#if defined(MAGICKCORE_PNG_DELEGATE)
  "png "
#endif
#if defined(MAGICKCORE_DPS_DELEGATE) || defined(MAGICKCORE_GS_DELEGATE) || defined(WIN32)
  "ps "
#endif
#if defined(MAGICKCORE_RSVG_DELEGATE)
  "rsvg "
#endif
#if defined(MAGICKCORE_TIFF_DELEGATE)
  "tiff "
#endif
#if defined(MAGICKCORE_WEBP_DELEGATE)
  "webp "
#endif
#if defined(MAGICKCORE_WMF_DELEGATE) || defined (MAGICKCORE_WMFLITE_DELEGATE)
  "wmf "
#endif
#if defined(MAGICKCORE_X11_DELEGATE)
  "x "
#endif
#if defined(MAGICKCORE_XML_DELEGATE)
  "xml "
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
  "zlib"
#endif
  ;
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
  return "DPC"
#if defined(MAGICKCORE_HDRI_SUPPORT)
  " HDRI"
#endif
#if defined(MAGICKCORE_BUILD_MODULES) || defined(_DLL)
  " Modules"
#endif
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  " OpenCL"
#endif
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  " OpenMP"
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
    path[MaxTextExtent];

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
    (void) FormatLocaleString(path,MaxTextExtent,"%s%s%s",element,
      DirectorySeparator,MagickURLFilename);
    if (IsPathAccessible(path) != MagickFalse)
      return(ConstantString(path));
    element=(const char *) GetNextValueInLinkedList(paths);
  }
  return(ConstantString(MagickHomeURL));
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
%  MagickCore libary version, quantum depth, HDRI status, OS word size, and
%  endianness.
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
  register ssize_t
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
      register unsigned int
        i;

      unsigned int
        alpha;

      for (i=0; i < 256; i++)
      {
        register ssize_t
          j;

        alpha=i;
        for (j=0; j < 8; j++)
          alpha=(alpha & 0x01) ? (0xEDB88320 ^ (alpha >> 1)) : (alpha >> 1);
        crc_xor[i]=alpha;
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
  register unsigned char
    *p;

  StringInfo
    *version;

  unsigned int
    signature;

  version=AcquireStringInfo(MaxTextExtent);
  p=GetStringInfoDatum(version);
  signature=MAGICKCORE_QUANTUM_DEPTH;
  (void) memcpy(p,&signature,sizeof(signature));
  p+=sizeof(signature);
  signature=MAGICKCORE_HDRI_ENABLE;
  (void) memcpy(p,&signature,sizeof(signature));
  p+=sizeof(signature);
  signature=MagickLibInterface;
  (void) memcpy(p,&signature,sizeof(signature));
  p+=sizeof(signature);
  signature=1;  /* endianess */
  (void) memcpy(p,&signature,sizeof(signature));
  p+=sizeof(signature);
  SetStringInfoLength(version,p-GetStringInfoDatum(version));
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
  (void) FormatLocaleFile(file,"Features: %s\n",GetMagickFeatures());
  (void) FormatLocaleFile(file,"Delegates: %s\n\n",GetMagickDelegates());
}

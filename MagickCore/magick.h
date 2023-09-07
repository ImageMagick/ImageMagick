/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore magick methods.
*/
#ifndef MAGICKCORE_MAGICK_H
#define MAGICKCORE_MAGICK_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <stdarg.h>
#include "MagickCore/semaphore.h"

#if defined(__cplusplus) || defined(c_plusplus)
# define magick_module  _module   /* reserved word in C++(20) */
#else
# define magick_module  module
#endif

typedef enum
{
  UndefinedFormatType,
  ImplicitFormatType,
  ExplicitFormatType
} MagickFormatType;

typedef enum
{
  CoderNoFlag = 0x0000,
  CoderAdjoinFlag = 0x0001,
  CoderBlobSupportFlag = 0x0002,
  CoderDecoderThreadSupportFlag = 0x0004,
  CoderEncoderThreadSupportFlag = 0x0008,
  CoderEndianSupportFlag = 0x0010,
  CoderRawSupportFlag = 0x0020,
  CoderSeekableStreamFlag = 0x0040, /* deprecated */
  CoderStealthFlag = 0x0080,
  CoderUseExtensionFlag = 0x0100,
  CoderDecoderSeekableStreamFlag = 0x0200,
  CoderEncoderSeekableStreamFlag = 0x0400
} MagickInfoFlag;

typedef Image
  *DecodeImageHandler(const ImageInfo *,ExceptionInfo *);

typedef MagickBooleanType
  EncodeImageHandler(const ImageInfo *,Image *,ExceptionInfo *);

typedef MagickBooleanType
  IsImageFormatHandler(const unsigned char *,const size_t);

typedef struct _MagickInfo
{
  char
    *name,
    *description,
    *version,
    *mime_type,
    *note,
    *magick_module;

  DecodeImageHandler
    *decoder;

  EncodeImageHandler
    *encoder;

  ImageInfo
    *image_info;

  IsImageFormatHandler
    *magick;

  MagickFormatType
    format_type;

  MagickStatusType
    flags;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;

  void
    *client_data;
} MagickInfo;

extern MagickExport char
  **GetMagickList(const char *,size_t *,ExceptionInfo *);

extern MagickExport const char
  *GetMagickDescription(const MagickInfo *),
  *GetMagickMimeType(const MagickInfo *),
  *GetMagickModuleName(const MagickInfo *),
  *GetMagickName(const MagickInfo *);

extern MagickExport DecodeImageHandler
  *GetImageDecoder(const MagickInfo *) magick_attribute((__pure__));

extern MagickExport EncodeImageHandler
  *GetImageEncoder(const MagickInfo *) magick_attribute((__pure__));

extern MagickExport int
  GetMagickPrecision(void),
  SetMagickPrecision(const int);

extern MagickExport MagickBooleanType
  GetImageMagick(const unsigned char *,const size_t,char *),
  GetMagickAdjoin(const MagickInfo *) magick_attribute((__pure__)),
  GetMagickBlobSupport(const MagickInfo *) magick_attribute((__pure__)),
  GetMagickDecoderSeekableStream(const MagickInfo *)
    magick_attribute((__pure__)),
  GetMagickDecoderThreadSupport(const MagickInfo *)
    magick_attribute((__pure__)),
  GetMagickEncoderSeekableStream(const MagickInfo *)
     magick_attribute((__pure__)),
  GetMagickEncoderThreadSupport(const MagickInfo *)
    magick_attribute((__pure__)),
  GetMagickEndianSupport(const MagickInfo *) magick_attribute((__pure__)),
  GetMagickRawSupport(const MagickInfo *) magick_attribute((__pure__)),
  GetMagickStealth(const MagickInfo *) magick_attribute((__pure__)),
  GetMagickUseExtension(const MagickInfo *) magick_attribute((__pure__)),
  IsMagickCoreInstantiated(void) magick_attribute((__pure__)),
  RegisterMagickInfo(MagickInfo *),
  UnregisterMagickInfo(const char *);

extern const MagickExport MagickInfo
  *GetMagickInfo(const char *,ExceptionInfo *),
  **GetMagickInfoList(const char *,size_t *,ExceptionInfo *);

extern MagickExport MagickInfo
  *AcquireMagickInfo(const char *, const char *, const char *);

extern MagickExport void
  MagickCoreGenesis(const char *,const MagickBooleanType),
  MagickCoreTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore magick methods.
*/
#ifndef _MAGICKCORE_MAGICK_H
#define _MAGICKCORE_MAGICK_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedFormatType,
  ImplicitFormatType,
  ExplicitFormatType
} MagickFormatType;

typedef enum
{
  NoThreadSupport = 0x0000,
  DecoderThreadSupport = 0x0001,
  EncoderThreadSupport = 0x0002
} MagickThreadSupport;

typedef Image
  *DecodeImageHandler(const ImageInfo *,ExceptionInfo *);

typedef MagickBooleanType
  EncodeImageHandler(const ImageInfo *,Image *);

typedef MagickBooleanType
  IsImageFormatHandler(const unsigned char *,const size_t);

typedef struct _MagickInfo
{
  char
    *name,
    *description,
    *version,
    *note,
    *module;

  ImageInfo
    *image_info;

  DecodeImageHandler
    *decoder;

  EncodeImageHandler
    *encoder;

  IsImageFormatHandler
    *magick;

  void
    *client_data;

  MagickBooleanType
    adjoin,
    raw,
    endian_support,
    blob_support,
    seekable_stream;

  MagickFormatType
    format_type;

  MagickStatusType
    thread_support;

  MagickBooleanType
    stealth;

  struct _MagickInfo
    *previous,
    *next;  /* deprecated, use GetMagickInfoList() */

  size_t
    signature;
} MagickInfo;

extern MagickExport char
  **GetMagickList(const char *,size_t *,ExceptionInfo *);

extern MagickExport const char
  *GetMagickDescription(const MagickInfo *);

extern MagickExport DecodeImageHandler
  *GetImageDecoder(const MagickInfo *);

extern MagickExport EncodeImageHandler
  *GetImageEncoder(const MagickInfo *);

extern MagickExport int
  GetMagickPrecision(void),
  SetMagickPrecision(const int);

extern MagickExport MagickBooleanType
  GetImageMagick(const unsigned char *,const size_t,char *),
  GetMagickAdjoin(const MagickInfo *),
  GetMagickBlobSupport(const MagickInfo *),
  GetMagickEndianSupport(const MagickInfo *),
  GetMagickRawSupport(const MagickInfo *),
  GetMagickSeekableStream(const MagickInfo *),
  IsMagickInstantiated(void),
  MagickComponentGenesis(void),
  UnregisterMagickInfo(const char *);

extern const MagickExport MagickInfo
  *GetMagickInfo(const char *,ExceptionInfo *),
  **GetMagickInfoList(const char *,size_t *,ExceptionInfo *);

extern MagickExport MagickInfo
  *RegisterMagickInfo(MagickInfo *),
  *SetMagickInfo(const char *);

extern MagickExport MagickStatusType
  GetMagickThreadSupport(const MagickInfo *);

extern MagickExport void
  MagickComponentTerminus(void),
  MagickCoreGenesis(const char *,const MagickBooleanType),
  MagickCoreTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

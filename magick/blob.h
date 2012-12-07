/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore Binary Large OBjects methods.
*/
#ifndef _MAGICKCORE_BLOB_H
#define _MAGICKCORE_BLOB_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/image.h"
#include "magick/stream.h"

#define MagickMaxBufferExtent  (32*8192)

typedef enum
{
  ReadMode,
  WriteMode,
  IOMode
} MapMode;

extern MagickExport FILE
  *GetBlobFileHandle(const Image *);

extern MagickExport Image
  *BlobToImage(const ImageInfo *,const void *,const size_t,ExceptionInfo *),
  *PingBlob(const ImageInfo *,const void *,const size_t,ExceptionInfo *);

extern MagickExport MagickBooleanType
  BlobToFile(char *,const void *,const size_t,ExceptionInfo *),
  FileToImage(Image *,const char *),
  GetBlobError(const Image *),
  ImageToFile(Image *,char *,ExceptionInfo *),
  InjectImageBlob(const ImageInfo *,Image *,Image *,const char *,
    ExceptionInfo *),
  IsBlobExempt(const Image *),
  IsBlobSeekable(const Image *),
  IsBlobTemporary(const Image *);

extern MagickExport MagickSizeType
  GetBlobSize(const Image *);

extern MagickExport StreamHandler
  GetBlobStreamHandler(const Image *);

extern MagickExport unsigned char
  *FileToBlob(const char *,const size_t,size_t *,ExceptionInfo *),
  *GetBlobStreamData(const Image *),
  *ImageToBlob(const ImageInfo *,Image *,size_t *,ExceptionInfo *),
  *ImagesToBlob(const ImageInfo *,Image *,size_t *,ExceptionInfo *);

extern MagickExport void
  DestroyBlob(Image *),
  DuplicateBlob(Image *,const Image *),
  SetBlobExempt(Image *,const MagickBooleanType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

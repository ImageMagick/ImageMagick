/*
  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore Binary Large OBjects methods.
*/
#ifndef MAGICKCORE_BLOB_H
#define MAGICKCORE_BLOB_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MagickMaxBufferExtent  81920

typedef enum
{
  ReadMode,
  WriteMode,
  IOMode,
  PersistMode
} MapMode;

typedef ssize_t
  (*CustomStreamHandler)(unsigned char *,const size_t,void *);

typedef MagickOffsetType
  (*CustomStreamSeeker)(const MagickOffsetType,const int,void *);

typedef MagickOffsetType
  (*CustomStreamTeller)(void *);

typedef struct _CustomStreamInfo
  CustomStreamInfo;

#include "MagickCore/image.h"
#include "MagickCore/stream.h"

extern MagickExport CustomStreamInfo
  *AcquireCustomStreamInfo(ExceptionInfo *),
  *DestroyCustomStreamInfo(CustomStreamInfo *);

extern MagickExport FILE
  *GetBlobFileHandle(const Image *);

extern MagickExport Image
  *BlobToImage(const ImageInfo *,const void *,const size_t,ExceptionInfo *),
  *PingBlob(const ImageInfo *,const void *,const size_t,ExceptionInfo *),
  *CustomStreamToImage(const ImageInfo *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  BlobToFile(char *,const void *,const size_t,ExceptionInfo *),
  FileToImage(Image *,const char *,ExceptionInfo *),
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

extern MagickExport void
  *GetBlobStreamData(const Image *),
  DestroyBlob(Image *),
  DuplicateBlob(Image *,const Image *),
  *FileToBlob(const char *,const size_t,size_t *,ExceptionInfo *),
  *ImageToBlob(const ImageInfo *,Image *,size_t *,ExceptionInfo *),
  ImageToCustomStream(const ImageInfo *,Image *,ExceptionInfo *),
  *ImagesToBlob(const ImageInfo *,Image *,size_t *,ExceptionInfo *),
  ImagesToCustomStream(const ImageInfo *,Image *,ExceptionInfo *),
  SetBlobExempt(Image *,const MagickBooleanType),
  SetCustomStreamData(CustomStreamInfo *,void *),
  SetCustomStreamReader(CustomStreamInfo *,CustomStreamHandler),
  SetCustomStreamSeeker(CustomStreamInfo *,CustomStreamSeeker),
  SetCustomStreamTeller(CustomStreamInfo *,CustomStreamTeller),
  SetCustomStreamWriter(CustomStreamInfo *,CustomStreamHandler);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

/*
  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization
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

#include "MagickCore/image.h"
#include "MagickCore/stream.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MagickMaxBufferExtent  8192

typedef enum
{
  UndefinedBlobMode,
  ReadBlobMode,
  ReadBinaryBlobMode,
  WriteBlobMode,
  WriteBinaryBlobMode,
  AppendBlobMode,
  AppendBinaryBlobMode
} BlobMode;

typedef enum
{
  ReadMode,
  WriteMode,
  IOMode
} MapMode;

typedef int
  *(*BlobFifo)(const Image *,const void *,const size_t);

extern MagickExport BlobInfo
  *CloneBlobInfo(const BlobInfo *),
  *ReferenceBlob(BlobInfo *);

extern MagickExport char
  *ReadBlobString(Image *,char *);

extern MagickExport const void
  *ReadBlobStream(Image *,const size_t,void *,ssize_t *);

extern MagickExport double
  ReadBlobDouble(Image *);

extern MagickExport FILE
  *GetBlobFileHandle(const Image *);

extern MagickExport float
  ReadBlobFloat(Image *);

extern MagickExport Image
  *BlobToImage(const ImageInfo *,const void *,const size_t,ExceptionInfo *),
  *PingBlob(const ImageInfo *,const void *,const size_t,ExceptionInfo *);

extern MagickExport int
  EOFBlob(const Image *),
  ReadBlobByte(Image *);

extern MagickExport MagickBooleanType
  BlobToFile(char *,const void *,const size_t,ExceptionInfo *),
  CloseBlob(Image *),
  DiscardBlobBytes(Image *,const MagickSizeType),
  FileToImage(Image *,const char *,ExceptionInfo *),
  ImageToFile(Image *,char *,ExceptionInfo *),
  InjectImageBlob(const ImageInfo *,Image *,Image *,const char *,
    ExceptionInfo *),
  OpenBlob(const ImageInfo *,Image *,const BlobMode,ExceptionInfo *),
  UnmapBlob(void *,const size_t);

extern MagickExport MagickOffsetType
  SeekBlob(Image *,const MagickOffsetType,const int),
  TellBlob(const Image *);

extern MagickExport MagickSizeType
  GetBlobSize(const Image *),
  ReadBlobLongLong(Image *),
  ReadBlobMSBLongLong(Image *);

extern MagickExport ssize_t
  ReadBlob(Image *,const size_t,void *),
  WriteBlob(Image *,const size_t,const void *),
  WriteBlobByte(Image *,const unsigned char),
  WriteBlobFloat(Image *,const float),
  WriteBlobLong(Image *,const unsigned int),
  WriteBlobShort(Image *,const unsigned short),
  WriteBlobLSBLong(Image *,const unsigned int),
  WriteBlobLSBShort(Image *,const unsigned short),
  WriteBlobMSBLong(Image *,const unsigned int),
  WriteBlobMSBLongLong(Image *,const MagickSizeType),
  WriteBlobMSBShort(Image *,const unsigned short),
  WriteBlobString(Image *,const char *);

extern MagickExport unsigned int
  ReadBlobLong(Image *),
  ReadBlobLSBLong(Image *),
  ReadBlobMSBLong(Image *);

extern MagickExport unsigned short
  ReadBlobShort(Image *),
  ReadBlobLSBShort(Image *),
  ReadBlobMSBShort(Image *);

extern MagickExport void
  AttachBlob(BlobInfo *,const void *,const size_t),
  DestroyBlob(Image *),
  *DetachBlob(BlobInfo *),
  DuplicateBlob(Image *,const Image *),
  *GetBlobStreamData(const Image *),
  *FileToBlob(const char *,const size_t,size_t *,ExceptionInfo *),
  *ImageToBlob(const ImageInfo *,Image *,size_t *,ExceptionInfo *),
  *ImagesToBlob(const ImageInfo *,Image *,size_t *,ExceptionInfo *),
  *MapBlob(int,const MapMode,const MagickOffsetType,const size_t),
  MSBOrderLong(unsigned char *,const size_t),
  MSBOrderShort(unsigned char *,const size_t);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

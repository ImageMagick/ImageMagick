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

  MagickCore Binary Large OBjects private methods.
*/
#ifndef _MAGICKCORE_BLOB_PRIVATE_H
#define _MAGICKCORE_BLOB_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/image.h"
#include "magick/stream.h"

#define MagickMinBlobExtent  32767L
#if defined(MAGICKCORE_HAVE_FSEEKO)
# define fseek  fseeko
# define ftell  ftello
#endif

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
  UndefinedStream,
  FileStream,
  StandardStream,
  PipeStream,
  ZipStream,
  BZipStream,
  FifoStream,
  BlobStream
} StreamType;

typedef int
  *(*BlobFifo)(const Image *,const void *,const size_t);

extern MagickExport BlobInfo
  *CloneBlobInfo(const BlobInfo *),
  *ReferenceBlob(BlobInfo *);

extern MagickExport char
  *ReadBlobString(Image *,char *);

extern MagickExport const struct stat
  *GetBlobProperties(const Image *);

extern MagickExport double
  ReadBlobDouble(Image *);

extern MagickExport float
  ReadBlobFloat(Image *);

extern MagickExport int
  EOFBlob(const Image *),
  ReadBlobByte(Image *);

extern MagickExport  MagickBooleanType
  CloseBlob(Image *),
  DiscardBlobBytes(Image *,const MagickSizeType),
  OpenBlob(const ImageInfo *,Image *,const BlobMode,ExceptionInfo *),
  SetBlobExtent(Image *,const MagickSizeType),
  UnmapBlob(void *,const size_t);

extern MagickExport MagickOffsetType
  SeekBlob(Image *,const MagickOffsetType,const int),
  TellBlob(const Image *);

extern MagickExport MagickSizeType
  ReadBlobLongLong(Image *),
  ReadBlobMSBLongLong(Image *);

extern MagickExport ssize_t
  ReadBlob(Image *,const size_t,unsigned char *),
  WriteBlob(Image *,const size_t,const unsigned char *),
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

extern MagickExport unsigned char
  *DetachBlob(BlobInfo *),
  *MapBlob(int,const MapMode,const MagickOffsetType,const size_t);

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
  GetBlobInfo(BlobInfo *),
  MSBOrderLong(unsigned char *,const size_t),
  MSBOrderShort(unsigned char *,const size_t);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

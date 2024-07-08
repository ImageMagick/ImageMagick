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

  MagickCore Binary Large OBjects private methods.
*/
#ifndef MAGICKCORE_BLOB_PRIVATE_H
#define MAGICKCORE_BLOB_PRIVATE_H

#include "MagickCore/image.h"
#include "MagickCore/stream.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MagickMinBlobExtent  32767L
#if defined(MAGICKCORE_HAVE_FSEEKO)
# define fseek  fseeko
# define ftell  ftello
#endif
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
#if !defined(__MINGW32__)
#  define stat  _stat64
#endif
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
  BlobStream,
  CustomStream
} StreamType;

extern MagickExport BlobInfo
  *CloneBlobInfo(const BlobInfo *),
  *ReferenceBlob(BlobInfo *);

extern MagickExport char
  *ReadBlobString(Image *,char *);

extern MagickExport const struct stat
  *GetBlobProperties(const Image *);

extern MagickExport const void
  *ReadBlobStream(Image *,const size_t,void *magick_restrict ,ssize_t *)
    magick_hot_spot;

extern MagickExport double
  ReadBlobDouble(Image *);

extern MagickExport float
  ReadBlobFloat(Image *);

extern MagickExport int
  EOFBlob(const Image *),
  ErrorBlob(const Image *),
  ReadBlobByte(Image *);

extern MagickExport MagickBooleanType
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

extern MagickExport signed int
  ReadBlobLSBSignedLong(Image *),
  ReadBlobMSBSignedLong(Image *),
  ReadBlobSignedLong(Image *);

extern MagickExport signed short
  ReadBlobLSBSignedShort(Image *),
  ReadBlobMSBSignedShort(Image *),
  ReadBlobSignedShort(Image *);

extern MagickExport ssize_t
  ReadBlob(Image *,const size_t,void *),
  WriteBlob(Image *,const size_t,const void *),
  WriteBlobByte(Image *,const unsigned char),
  WriteBlobFloat(Image *,const float),
  WriteBlobLong(Image *,const unsigned int),
  WriteBlobLongLong(Image *,const MagickSizeType),
  WriteBlobShort(Image *,const unsigned short),
  WriteBlobSignedLong(Image *,const signed int),
  WriteBlobLSBLong(Image *,const unsigned int),
  WriteBlobLSBShort(Image *,const unsigned short),
  WriteBlobLSBSignedLong(Image *,const signed int),
  WriteBlobLSBSignedShort(Image *,const signed short),
  WriteBlobMSBLong(Image *,const unsigned int),
  WriteBlobMSBShort(Image *,const unsigned short),
  WriteBlobMSBSignedShort(Image *,const signed short),
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
  AttachCustomStream(BlobInfo *,CustomStreamInfo *),
  *DetachBlob(BlobInfo *),
  DisassociateBlob(Image *),
  GetBlobInfo(BlobInfo *),
  *MapBlob(int,const MapMode,const MagickOffsetType,const size_t),
  MSBOrderLong(unsigned char *,const size_t),
  MSBOrderShort(unsigned char *,const size_t);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

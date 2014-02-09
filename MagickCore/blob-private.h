/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
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

#include "MagickCore/nt-feature.h"
#include "MagickCore/nt-private.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MagickMinBlobExtent  32767L
#if defined(MAGICKCORE_HAVE_FSEEKO)
# define fseek  fseeko
# define ftell  ftello
#endif

typedef enum
{
  UndefinedStream,
  StandardStream,
  FileStream,
  PipeStream,
  ZipStream,
  BZipStream,
  FifoStream,
  BlobStream
} StreamType;

extern MagickPrivate MagickBooleanType
  GetBlobError(const Image *),
  IsBlobExempt(const Image *),
  IsBlobSeekable(const Image *),
  IsBlobTemporary(const Image *),
  SetBlobExtent(Image *,const MagickSizeType);

extern MagickPrivate const struct stat
  *GetBlobProperties(const Image *);

extern MagickPrivate StreamHandler
  GetBlobStreamHandler(const Image *);

extern MagickPrivate void
  GetBlobInfo(BlobInfo *),
  SetBlobExempt(Image *,const MagickBooleanType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

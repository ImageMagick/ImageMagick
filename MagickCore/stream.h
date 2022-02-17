/*
  Copyright @ 2000 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image stream methods.
*/
#ifndef MAGICKCORE_STREAM_H
#define MAGICKCORE_STREAM_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "MagickCore/pixel.h"

typedef struct _StreamInfo
  StreamInfo;

typedef size_t
  (*StreamHandler)(const Image *,const void *,const size_t);

extern MagickExport Image
  *ReadStream(const ImageInfo *,StreamHandler,ExceptionInfo *),
  *StreamImage(const ImageInfo *,StreamInfo *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  OpenStream(const ImageInfo *,StreamInfo *,const char *,ExceptionInfo *),
  WriteStream(const ImageInfo *,Image *,StreamHandler,ExceptionInfo *);

extern MagickExport StreamInfo
  *AcquireStreamInfo(const ImageInfo *,ExceptionInfo *),
  *DestroyStreamInfo(StreamInfo *);

extern MagickExport void
  SetStreamInfoMap(StreamInfo *,const char *),
  SetStreamInfoStorageType(StreamInfo *,const StorageType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

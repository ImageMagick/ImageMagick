/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image stream private methods.
*/
#ifndef _MAGICKCORE_STREAM_PRIVATE_H
#define _MAGICKCORE_STREAM_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _StreamInfo
  StreamInfo;

extern MagickExport const void
  *GetStreamInfoClientData(StreamInfo *);

extern MagickExport Image
  *StreamImage(const ImageInfo *,StreamInfo *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  OpenStream(const ImageInfo *,StreamInfo *,const char *,ExceptionInfo *);

extern MagickExport StreamInfo
  *AcquireStreamInfo(const ImageInfo *),
  *DestroyStreamInfo(StreamInfo *);

extern MagickExport void
  SetStreamInfoClientData(StreamInfo *,const void *),
  SetStreamInfoMap(StreamInfo *,const char *),
  SetStreamInfoStorageType(StreamInfo *,const StorageType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

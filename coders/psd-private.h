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

  Private header to reuse psd functionality.
*/
#ifndef _PSD_PRIVATE_H
#define _PSD_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _PSDInfo
{
  char
    signature[4];

  size_t
    rows,
    columns;

  unsigned char
    reserved[6];

  unsigned short
    channels,
    depth,
    mode,
    version;
} PSDInfo;

extern ModuleExport MagickBooleanType
  ReadPSDLayers(Image *,const ImageInfo *,const PSDInfo *,
    const MagickBooleanType,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

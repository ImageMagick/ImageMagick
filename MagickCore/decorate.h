/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image decorate methods.
*/
#ifndef MAGICKCORE_DECORATE_H
#define MAGICKCORE_DECORATE_H

#include "MagickCore/image.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _FrameInfo
{
  size_t
    width,
    height;

  ssize_t
    x,
    y,
    inner_bevel,
    outer_bevel;
} FrameInfo;

extern MagickExport Image
  *BorderImage(const Image *,const RectangleInfo *,const CompositeOperator,
    ExceptionInfo *),
  *FrameImage(const Image *,const FrameInfo *,const CompositeOperator,
    ExceptionInfo *);

extern MagickExport MagickBooleanType
  RaiseImage(Image *,const RectangleInfo *,const MagickBooleanType,
    ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

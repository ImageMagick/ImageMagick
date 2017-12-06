/*
  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image quantization methods.
*/
#ifndef MAGICKCORE_QUANTIZE_H
#define MAGICKCORE_QUANTIZE_H

#include "MagickCore/colorspace.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedDitherMethod,
  NoDitherMethod,
  RiemersmaDitherMethod,
  FloydSteinbergDitherMethod
} DitherMethod;

typedef struct _QuantizeInfo
{
  size_t
    number_colors;     /* desired maximum number of colors */

  size_t
    tree_depth;

  ColorspaceType
    colorspace;

  DitherMethod
    dither_method;

  MagickBooleanType
    measure_error;

  size_t
    signature;
} QuantizeInfo;

extern MagickExport MagickBooleanType
  CompressImageColormap(Image *,ExceptionInfo *),
  GetImageQuantizeError(Image *,ExceptionInfo *),
  PosterizeImage(Image *,const size_t,const DitherMethod,ExceptionInfo *),
  QuantizeImage(const QuantizeInfo *,Image *,ExceptionInfo *),
  QuantizeImages(const QuantizeInfo *,Image *,ExceptionInfo *),
  RemapImage(const QuantizeInfo *,Image *,const Image *,ExceptionInfo *),
  RemapImages(const QuantizeInfo *,Image *,const Image *,ExceptionInfo *);

extern MagickExport QuantizeInfo
  *AcquireQuantizeInfo(const ImageInfo *),
  *CloneQuantizeInfo(const QuantizeInfo *),
  *DestroyQuantizeInfo(QuantizeInfo *);

extern MagickExport void
  GetQuantizeInfo(QuantizeInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

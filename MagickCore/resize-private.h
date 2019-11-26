/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image resize private methods.
*/
#ifndef MAGICKCORE_RESIZE_PRIVATE_H
#define MAGICKCORE_RESIZE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  BoxWeightingFunction = 0,
  TriangleWeightingFunction,
  CubicBCWeightingFunction,
  HannWeightingFunction,
  HammingWeightingFunction,
  BlackmanWeightingFunction,
  GaussianWeightingFunction,
  QuadraticWeightingFunction,
  JincWeightingFunction,
  SincWeightingFunction,
  SincFastWeightingFunction,
  KaiserWeightingFunction,
  WelchWeightingFunction,
  BohmanWeightingFunction,
  LagrangeWeightingFunction,
  CosineWeightingFunction,
  LastWeightingFunction
} ResizeWeightingFunctionType;

extern MagickPrivate double
  *GetResizeFilterCoefficient(const ResizeFilter*),
  GetResizeFilterBlur(const ResizeFilter *),
  GetResizeFilterScale(const ResizeFilter *),
  GetResizeFilterWindowSupport(const ResizeFilter *),
  GetResizeFilterSupport(const ResizeFilter *),
  GetResizeFilterWeight(const ResizeFilter *,const double);

extern MagickPrivate ResizeFilter
  *AcquireResizeFilter(const Image *,const FilterType,const MagickBooleanType,
    ExceptionInfo *),
  *DestroyResizeFilter(ResizeFilter *);

extern MagickPrivate ResizeWeightingFunctionType
  GetResizeFilterWeightingType(const ResizeFilter *),
  GetResizeFilterWindowWeightingType(const ResizeFilter *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

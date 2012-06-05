/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image resize methods.
*/
#ifndef _MAGICKCORE_RESIZE_H
#define _MAGICKCORE_RESIZE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport Image
  *AdaptiveResizeImage(const Image *,const size_t,const size_t,ExceptionInfo *),
  *InterpolativeResizeImage(const Image *,const size_t,const size_t,
    const InterpolatePixelMethod,ExceptionInfo *),
  *LiquidRescaleImage(const Image *,const size_t,const size_t,const double,
    const double,ExceptionInfo *),
  *MagnifyImage(const Image *,ExceptionInfo *),
  *MinifyImage(const Image *,ExceptionInfo *),
  *ResampleImage(const Image *,const double,const double,const FilterTypes,
    const double,ExceptionInfo *),
  *ResizeImage(const Image *,const size_t,const size_t,const FilterTypes,
    const double,ExceptionInfo *),
  *SampleImage(const Image *,const size_t,const size_t,ExceptionInfo *),
  *ScaleImage(const Image *,const size_t,const size_t,ExceptionInfo *),
  *ThumbnailImage(const Image *,const size_t,const size_t,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

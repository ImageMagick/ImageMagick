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

  MagickCore acceleration methods.
*/
#ifndef _MAGICKCORE_ACCELERATE_H
#define _MAGICKCORE_ACCELERATE_H

#include "magick/fx.h"
#include "magick/morphology.h"
#include "magick/resample.h"
#include "magick/resize.h"
#include "magick/statistic.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport MagickBooleanType
  AccelerateContrastImage(Image *,const MagickBooleanType,ExceptionInfo *),
  AccelerateConvolveImage(const Image *,const KernelInfo *,Image *,
    ExceptionInfo *),
  AccelerateEqualizeImage(Image *,const ChannelType,ExceptionInfo *),
  AccelerateFunctionImage(Image *,const ChannelType,const MagickFunction,
    const size_t,const double *,ExceptionInfo *),
  AccelerateModulateImage(Image*, double, double, double, 
    ColorspaceType, ExceptionInfo*);

extern MagickExport Image
  *AccelerateAddNoiseImage(const Image*,const ChannelType,const NoiseType,
    ExceptionInfo *),
  *AccelerateBlurImage(const Image *,const ChannelType,const double,
    const double,ExceptionInfo *),
  *AccelerateConvolveImageChannel(const Image *,const ChannelType,
    const KernelInfo *,ExceptionInfo *),
  *AccelerateDespeckleImage(const Image *,ExceptionInfo *),
  *AccelerateRadialBlurImage(const Image *,const ChannelType,const double,
    ExceptionInfo *),
  *AccelerateResizeImage(const Image *,const size_t,const size_t,
    const ResizeFilter *,ExceptionInfo *),
  *AccelerateUnsharpMaskImage(const Image *,const ChannelType,const double,
    const double,const double,const double,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

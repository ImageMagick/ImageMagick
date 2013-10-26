/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
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

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/morphology.h"
#include "magick/resample.h"
#include "magick/resize.h"
#include "magick/statistic.h"

extern MagickExport Image
  *AccelerateConvolveImageChannel(const Image *,const ChannelType,
    const KernelInfo *,ExceptionInfo *);

/* legacy, do not use */
extern MagickExport MagickBooleanType
  AccelerateConvolveImage(const Image *,const KernelInfo *,Image *,
    ExceptionInfo *);

extern MagickExport MagickBooleanType 
  AccelerateFunctionImage(Image *image, const ChannelType channel,const MagickFunction function,
  const size_t number_parameters,const double *parameters, ExceptionInfo *exception);

extern MagickExport
Image* AccelerateBlurImage(const Image *image, const ChannelType channel, const double radius, const double sigma,ExceptionInfo *exception);

extern MagickExport
Image* AccelerateRadialBlurImage(const Image *image, const ChannelType channel, const double angle, ExceptionInfo *exception);


extern MagickExport
Image* AccelerateUnsharpMaskImage(const Image *image, const ChannelType channel,const double radius,const double sigma, 
				  const double gain,const double threshold,ExceptionInfo *exception);

extern MagickExport
Image* AccelerateResizeImage(const Image* image, const size_t columns, const size_t rows
			    , const ResizeFilter* resizeFilter, ExceptionInfo *exception);

extern MagickExport
MagickBooleanType AccelerateContrastImage(Image* image, const MagickBooleanType sharpen, ExceptionInfo* exception);

extern MagickExport
MagickBooleanType AccelerateEqualizeImage(Image* image, const ChannelType channel, ExceptionInfo* exception);

extern MagickExport
Image* AccelerateDespeckleImage(const Image* image, ExceptionInfo* exception);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

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

  MagickCore image effects methods.
*/
#ifndef _MAGICKCORE_EFFECT_H
#define _MAGICKCORE_EFFECT_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/morphology.h"

typedef enum
{
  UndefinedPreview,
  RotatePreview,
  ShearPreview,
  RollPreview,
  HuePreview,
  SaturationPreview,
  BrightnessPreview,
  GammaPreview,
  SpiffPreview,
  DullPreview,
  GrayscalePreview,
  QuantizePreview,
  DespecklePreview,
  ReduceNoisePreview,
  AddNoisePreview,
  SharpenPreview,
  BlurPreview,
  ThresholdPreview,
  EdgeDetectPreview,
  SpreadPreview,
  SolarizePreview,
  ShadePreview,
  RaisePreview,
  SegmentPreview,
  SwirlPreview,
  ImplodePreview,
  WavePreview,
  OilPaintPreview,
  CharcoalDrawingPreview,
  JPEGPreview
} PreviewType;

extern MagickExport Image
  *AdaptiveBlurImage(const Image *,const double,const double,ExceptionInfo *),
  *AdaptiveBlurImageChannel(const Image *,const ChannelType,const double,
    const double,ExceptionInfo *),
  *AdaptiveSharpenImage(const Image *,const double,const double,
     ExceptionInfo *),
  *AdaptiveSharpenImageChannel(const Image *,const ChannelType,const double,
    const double,ExceptionInfo *),
  *BlurImage(const Image *,const double,const double,ExceptionInfo *),
  *BlurImageChannel(const Image *,const ChannelType,const double,const double,
    ExceptionInfo *),
  *ConvolveImage(const Image *,const size_t,const double *,ExceptionInfo *),
  *ConvolveImageChannel(const Image *,const ChannelType,const size_t,
    const double *,ExceptionInfo *),
  *DespeckleImage(const Image *,ExceptionInfo *),
  *EdgeImage(const Image *,const double,ExceptionInfo *),
  *EmbossImage(const Image *,const double,const double,ExceptionInfo *),
  *FilterImage(const Image *,const KernelInfo *,ExceptionInfo *),
  *FilterImageChannel(const Image *,const ChannelType,const KernelInfo *,
     ExceptionInfo *),
  *GaussianBlurImage(const Image *,const double,const double,ExceptionInfo *),
  *GaussianBlurImageChannel(const Image *,const ChannelType,const double,
    const double,ExceptionInfo *),
  *MotionBlurImage(const Image *,const double,const double,const double,
    ExceptionInfo *),
  *MotionBlurImageChannel(const Image *,const ChannelType,const double,
    const double,const double,ExceptionInfo *),
  *PreviewImage(const Image *,const PreviewType,ExceptionInfo *),
  *RadialBlurImage(const Image *,const double,ExceptionInfo *),
  *RadialBlurImageChannel(const Image *,const ChannelType,const double,
    ExceptionInfo *),
  *SelectiveBlurImage(const Image *,const double,const double,const double,
    ExceptionInfo *),
  *SelectiveBlurImageChannel(const Image *,const ChannelType,const double,
    const double,const double,ExceptionInfo *),
  *ShadeImage(const Image *,const MagickBooleanType,const double,const double,
    ExceptionInfo *),
  *SharpenImage(const Image *,const double,const double,ExceptionInfo *),
  *SharpenImageChannel(const Image *,const ChannelType,const double,
    const double,ExceptionInfo *),
  *SpreadImage(const Image *,const double,ExceptionInfo *),
  *UnsharpMaskImage(const Image *,const double,const double,const double,
    const double,ExceptionInfo *),
  *UnsharpMaskImageChannel(const Image *,const ChannelType,const double,
    const double,const double,const double,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image enhance methods.
*/
#ifndef MAGICKCORE_ENHANCE_H
#define MAGICKCORE_ENHANCE_H

#include "MagickCore/pixel.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport MagickBooleanType
  AutoGammaImage(Image *,ExceptionInfo *),
  AutoLevelImage(Image *,ExceptionInfo *),
  BrightnessContrastImage(Image *,const double,const double,ExceptionInfo *),
  CLAHEImage(Image *,const size_t,const size_t,const size_t,const double,
    ExceptionInfo *),
  ClutImage(Image *,const Image *,const PixelInterpolateMethod,ExceptionInfo *),
  ColorDecisionListImage(Image *,const char *,ExceptionInfo *),
  ContrastImage(Image *,const MagickBooleanType,ExceptionInfo *),
  ContrastStretchImage(Image *,const double,const double,ExceptionInfo *),
  EqualizeImage(Image *image,ExceptionInfo *),
  GammaImage(Image *,const double,ExceptionInfo *),
  GrayscaleImage(Image *,const PixelIntensityMethod,ExceptionInfo *),
  HaldClutImage(Image *,const Image *,ExceptionInfo *),
  LevelImage(Image *,const double,const double,const double,ExceptionInfo *),
  LevelizeImage(Image *,const double,const double,const double,ExceptionInfo *),
  LevelImageColors(Image *,const PixelInfo *,const PixelInfo *,
    const MagickBooleanType,ExceptionInfo *),
  LinearStretchImage(Image *,const double,const double,ExceptionInfo *),
  ModulateImage(Image *,const char *,ExceptionInfo *),
  NegateImage(Image *,const MagickBooleanType,ExceptionInfo *),
  NormalizeImage(Image *,ExceptionInfo *),
  SigmoidalContrastImage(Image *,const MagickBooleanType,const double,
    const double,ExceptionInfo *);

extern MagickExport Image
  *EnhanceImage(const Image *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

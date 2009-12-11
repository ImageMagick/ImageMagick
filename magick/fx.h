/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image f/x methods.
*/
#ifndef _MAGICKCORE_FX_H
#define _MAGICKCORE_FX_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/draw.h"

typedef enum
{
  UndefinedEvaluateOperator,
  AddEvaluateOperator,
  AndEvaluateOperator,
  DivideEvaluateOperator,
  LeftShiftEvaluateOperator,
  MaxEvaluateOperator,
  MinEvaluateOperator,
  MultiplyEvaluateOperator,
  OrEvaluateOperator,
  RightShiftEvaluateOperator,
  SetEvaluateOperator,
  SubtractEvaluateOperator,
  XorEvaluateOperator,
  PowEvaluateOperator,
  LogEvaluateOperator,
  ThresholdEvaluateOperator,
  ThresholdBlackEvaluateOperator,
  ThresholdWhiteEvaluateOperator,
  GaussianNoiseEvaluateOperator,
  ImpulseNoiseEvaluateOperator,
  LaplacianNoiseEvaluateOperator,
  MultiplicativeNoiseEvaluateOperator,
  PoissonNoiseEvaluateOperator,
  UniformNoiseEvaluateOperator,
  CosineEvaluateOperator,
  SineEvaluateOperator,
  AddModulusEvaluateOperator
} MagickEvaluateOperator;

typedef enum
{
  UndefinedFunction,
  PolynomialFunction,
  SinusoidFunction,
  ArcsinFunction,
  ArctanFunction
} MagickFunction;

typedef enum
{
  UndefinedNoise,
  UniformNoise,
  GaussianNoise,
  MultiplicativeGaussianNoise,
  ImpulseNoise,
  LaplacianNoise,
  PoissonNoise,
  RandomNoise
} NoiseType;

extern MagickExport Image
  *AddNoiseImage(const Image *,const NoiseType,ExceptionInfo *),
  *AddNoiseImageChannel(const Image *,const ChannelType,const NoiseType,
    ExceptionInfo *),
  *BlueShiftImage(const Image *,const double,ExceptionInfo *),
  *CharcoalImage(const Image *,const double,const double,ExceptionInfo *),
  *ColorizeImage(const Image *,const char *,const PixelPacket,ExceptionInfo *),
  *FxImage(const Image *,const char *,ExceptionInfo *),
  *FxImageChannel(const Image *,const ChannelType,const char *,ExceptionInfo *),
  *ImplodeImage(const Image *,const double,ExceptionInfo *),
  *MorphImages(const Image *,const unsigned long,ExceptionInfo *),
  *PolaroidImage(const Image *,const DrawInfo *,const double,ExceptionInfo *),
  *RecolorImage(const Image *,const unsigned long,const double *,
    ExceptionInfo *),
  *SepiaToneImage(const Image *,const double,ExceptionInfo *),
  *ShadowImage(const Image *,const double,const double,const long,const long,
    ExceptionInfo *),
  *SketchImage(const Image *,const double,const double,const double,
    ExceptionInfo *),
  *SteganoImage(const Image *,const Image *,ExceptionInfo *),
  *StereoImage(const Image *,const Image *,ExceptionInfo *),
  *StereoAnaglyphImage(const Image *,const Image *,const long,const long,
     ExceptionInfo *),
  *SwirlImage(const Image *,double,ExceptionInfo *),
  *TintImage(const Image *,const char *,const PixelPacket,ExceptionInfo *),
  *VignetteImage(const Image *,const double,const double,const long,
    const long,ExceptionInfo *),
  *WaveImage(const Image *,const double,const double,ExceptionInfo *);

extern MagickExport MagickBooleanType
  EvaluateImage(Image *,const MagickEvaluateOperator,const double,
    ExceptionInfo *),
  EvaluateImageChannel(Image *,const ChannelType,const MagickEvaluateOperator,
    const double,ExceptionInfo *),
  FunctionImage(Image *,const MagickFunction,const unsigned long,const double *,
    ExceptionInfo *),
  FunctionImageChannel(Image *,const ChannelType,const MagickFunction,
    const unsigned long,const double *,ExceptionInfo *),
  PlasmaImage(Image *,const SegmentInfo *,unsigned long,unsigned long),
  SolarizeImage(Image *,const double);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

/*
  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore statistical methods.
*/
#ifndef _MAGICKCORE_STATISTIC_H
#define _MAGICKCORE_STATISTIC_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/draw.h"

#define MaximumNumberOfImageMoments  8
#define MaximumNumberOfPerceptualHashes  7

typedef struct _ChannelStatistics
{
  size_t
    depth;

  double
    minima,
    maxima,
    sum,
    sum_squared,
    sum_cubed,
    sum_fourth_power,
    mean,
    variance,
    standard_deviation,
    kurtosis,
    skewness,
    entropy;
} ChannelStatistics;

#undef I

typedef struct _ChannelMoments
{
  double
    I[32];

  PointInfo
    centroid,
    ellipse_axis;

  double
    ellipse_angle,
    ellipse_eccentricity,
    ellipse_intensity;
} ChannelMoments;

typedef struct _ChannelPerceptualHash
{
  double
    P[32],
    Q[32];
} ChannelPerceptualHash;

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
  AddModulusEvaluateOperator,
  MeanEvaluateOperator,
  AbsEvaluateOperator,
  ExponentialEvaluateOperator,
  MedianEvaluateOperator,
  SumEvaluateOperator,
  RootMeanSquareEvaluateOperator
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
  UndefinedStatistic,
  GradientStatistic,
  MaximumStatistic,
  MeanStatistic,
  MedianStatistic,
  MinimumStatistic,
  ModeStatistic,
  NonpeakStatistic,
  StandardDeviationStatistic,
  RootMeanSquareStatistic
} StatisticType;

extern MagickExport ChannelStatistics
  *GetImageChannelStatistics(const Image *,ExceptionInfo *);

extern MagickExport ChannelMoments
  *GetImageChannelMoments(const Image *,ExceptionInfo *);

extern MagickExport ChannelPerceptualHash
  *GetImageChannelPerceptualHash(const Image *,ExceptionInfo *);

extern MagickExport Image
  *EvaluateImages(const Image *,const MagickEvaluateOperator,ExceptionInfo *),
  *PolynomialImage(const Image *,const size_t,const double *,ExceptionInfo *),
  *PolynomialImageChannel(const Image *,const ChannelType,const size_t,
    const double *,ExceptionInfo *),
  *StatisticImage(const Image *,const StatisticType,const size_t,const size_t,
    ExceptionInfo *),
  *StatisticImageChannel(const Image *,const ChannelType,const StatisticType,
    const size_t,const size_t,ExceptionInfo *);

extern MagickExport MagickBooleanType
  EvaluateImage(Image *,const MagickEvaluateOperator,const double,
    ExceptionInfo *),
  EvaluateImageChannel(Image *,const ChannelType,const MagickEvaluateOperator,
    const double,ExceptionInfo *),
  FunctionImage(Image *,const MagickFunction,const size_t,const double *,
    ExceptionInfo *),
  FunctionImageChannel(Image *,const ChannelType,const MagickFunction,
    const size_t,const double *,ExceptionInfo *),
  GetImageChannelEntropy(const Image *,const ChannelType,double *,
    ExceptionInfo *),
  GetImageChannelExtrema(const Image *,const ChannelType,size_t *,size_t *,
    ExceptionInfo *),
  GetImageChannelMean(const Image *,const ChannelType,double *,double *,
    ExceptionInfo *),
  GetImageChannelKurtosis(const Image *,const ChannelType,double *,double *,
    ExceptionInfo *),
  GetImageChannelRange(const Image *,const ChannelType,double *,double *,
    ExceptionInfo *),
  GetImageEntropy(const Image *,double *,ExceptionInfo *),
  GetImageExtrema(const Image *,size_t *,size_t *,ExceptionInfo *),
  GetImageMean(const Image *,double *,double *,ExceptionInfo *),
  GetImageKurtosis(const Image *,double *,double *,ExceptionInfo *),
  GetImageRange(const Image *,double *,double *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

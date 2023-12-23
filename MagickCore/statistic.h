/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore statistical methods.
*/
#ifndef MAGICKCORE_STATISTIC_H
#define MAGICKCORE_STATISTIC_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MaximumNumberOfImageMoments  8
#define MaximumNumberOfPerceptualColorspaces  6
#define MaximumNumberOfPerceptualHashes  7

typedef struct _ChannelStatistics
{
  size_t
    depth;

  double
    area,
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
    entropy,
    median;

  long double
    sumLD,
    M1,
    M2,
    M3,
    M4;
} ChannelStatistics;

typedef struct _ChannelMoments
{
  double
    invariant[MaximumNumberOfImageMoments+1];

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
    srgb_hu_phash[MaximumNumberOfImageMoments+1],
    hclp_hu_phash[MaximumNumberOfImageMoments+1];

  size_t
    number_colorspaces;

  ColorspaceType
    colorspace[MaximumNumberOfPerceptualColorspaces+1];

  double
    phash[MaximumNumberOfPerceptualColorspaces+1][MaximumNumberOfImageMoments+1];

  size_t
    number_channels;
} ChannelPerceptualHash;

typedef enum
{
  UndefinedEvaluateOperator,
  AbsEvaluateOperator,
  AddEvaluateOperator,
  AddModulusEvaluateOperator,
  AndEvaluateOperator,
  CosineEvaluateOperator,
  DivideEvaluateOperator,
  ExponentialEvaluateOperator,
  GaussianNoiseEvaluateOperator,
  ImpulseNoiseEvaluateOperator,
  LaplacianNoiseEvaluateOperator,
  LeftShiftEvaluateOperator,
  LogEvaluateOperator,
  MaxEvaluateOperator,
  MeanEvaluateOperator,
  MedianEvaluateOperator,
  MinEvaluateOperator,
  MultiplicativeNoiseEvaluateOperator,
  MultiplyEvaluateOperator,
  OrEvaluateOperator,
  PoissonNoiseEvaluateOperator,
  PowEvaluateOperator,
  RightShiftEvaluateOperator,
  RootMeanSquareEvaluateOperator,
  SetEvaluateOperator,
  SineEvaluateOperator,
  SubtractEvaluateOperator,
  SumEvaluateOperator,
  ThresholdBlackEvaluateOperator,
  ThresholdEvaluateOperator,
  ThresholdWhiteEvaluateOperator,
  UniformNoiseEvaluateOperator,
  XorEvaluateOperator,
  InverseLogEvaluateOperator
} MagickEvaluateOperator;

typedef enum
{
  UndefinedFunction,
  ArcsinFunction,
  ArctanFunction,
  PolynomialFunction,
  SinusoidFunction
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
  RootMeanSquareStatistic,
  StandardDeviationStatistic,
  ContrastStatistic
} StatisticType;

extern MagickExport ChannelStatistics
  *GetImageStatistics(const Image *,ExceptionInfo *);

extern MagickExport ChannelMoments
  *GetImageMoments(const Image *,ExceptionInfo *);

extern MagickExport ChannelPerceptualHash
  *GetImagePerceptualHash(const Image *,ExceptionInfo *);

extern MagickExport Image
  *EvaluateImages(const Image *,const MagickEvaluateOperator,ExceptionInfo *),
  *PolynomialImage(const Image *,const size_t,const double *,ExceptionInfo *),
  *StatisticImage(const Image *,const StatisticType,const size_t,const size_t,
    ExceptionInfo *);

extern MagickExport MagickBooleanType
  EvaluateImage(Image *,const MagickEvaluateOperator,const double,
    ExceptionInfo *),
  FunctionImage(Image *,const MagickFunction,const size_t,const double *,
    ExceptionInfo *),
  GetImageEntropy(const Image *,double *,ExceptionInfo *),
  GetImageExtrema(const Image *,size_t *,size_t *,ExceptionInfo *),
  GetImageMean(const Image *,double *,double *,ExceptionInfo *),
  GetImageMedian(const Image *,double *,ExceptionInfo *),
  GetImageKurtosis(const Image *,double *,double *,ExceptionInfo *),
  GetImageRange(const Image *,double *,double *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

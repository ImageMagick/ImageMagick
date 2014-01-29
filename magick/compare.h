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

  MagickCore image compare methods.
*/
#ifndef _MAGICKCORE_COMPARE_H
#define _MAGICKCORE_COMPARE_H

#include "magick/image.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedMetric,
  AbsoluteErrorMetric,
  MeanAbsoluteErrorMetric,
  MeanErrorPerPixelMetric,
  MeanSquaredErrorMetric,
  PeakAbsoluteErrorMetric,
  PeakSignalToNoiseRatioMetric,
  RootMeanSquaredErrorMetric,
  NormalizedCrossCorrelationErrorMetric,
  FuzzErrorMetric,
  UndefinedErrorMetric = 0,
  PerceptualHashErrorMetric = 0xff
} MetricType;

extern MagickExport double
  *GetImageChannelDistortions(Image *,const Image *,const MetricType,
    ExceptionInfo *);

extern MagickExport Image
  *CompareImageChannels(Image *,const Image *,const ChannelType,
    const MetricType,double *,ExceptionInfo *),
  *CompareImages(Image *,const Image *,const MetricType,double *,
    ExceptionInfo *),
  *SimilarityImage(Image *,const Image *,RectangleInfo *,double *,
    ExceptionInfo *),
  *SimilarityMetricImage(Image *,const Image *,const MetricType,
    RectangleInfo *,double *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  GetImageChannelDistortion(Image *,const Image *,const ChannelType,
    const MetricType,double *,ExceptionInfo *),
  GetImageDistortion(Image *,const Image *,const MetricType,double *,
    ExceptionInfo *),
  IsImagesEqual(Image *,const Image *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

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

  MagickCore statistical methods.
*/
#ifndef _MAGICKCORE_STATISTIC_H
#define _MAGICKCORE_STATISTIC_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _ChannelStatistics
{
  unsigned long
    depth;

  double
    minima,
    maxima,
    mean,
    standard_deviation,
    kurtosis,
    skewness;
} ChannelStatistics;

extern MagickExport ChannelStatistics
  *GetImageChannelStatistics(const Image *,ExceptionInfo *);

extern MagickExport Image
  *AverageImages(const Image *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  GetImageChannelExtrema(const Image *,const ChannelType,unsigned long *,
    unsigned long *,ExceptionInfo *),
  GetImageChannelMean(const Image *,const ChannelType,double *,double *,
    ExceptionInfo *),
  GetImageChannelKurtosis(const Image *,const ChannelType,double *,double *,
    ExceptionInfo *),
  GetImageChannelRange(const Image *,const ChannelType,double *,double *,
    ExceptionInfo *),
  GetImageExtrema(const Image *,unsigned long *,unsigned long *,
    ExceptionInfo *),
  GetImageRange(const Image *,double *,double *,ExceptionInfo *),
  GetImageMean(const Image *,double *,double *,ExceptionInfo *),
  GetImageKurtosis(const Image *,double *,double *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

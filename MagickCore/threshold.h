/*
  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image threshold methods.
*/
#ifndef MAGICKCORE_THRESHOLD_H
#define MAGICKCORE_THRESHOLD_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedThresholdMethod,
  KapurThresholdMethod,
  OTSUThresholdMethod,
  TriangleThresholdMethod
} AutoThresholdMethod;

typedef struct _ThresholdMap
  ThresholdMap;

extern MagickExport Image
  *AdaptiveThresholdImage(const Image *,const size_t,const size_t,const double,
    ExceptionInfo *);

extern MagickExport ThresholdMap
  *DestroyThresholdMap(ThresholdMap *),
  *GetThresholdMap(const char *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  AutoThresholdImage(Image *,const AutoThresholdMethod,ExceptionInfo *),
  BilevelImage(Image *,const double,ExceptionInfo *),
  BlackThresholdImage(Image *,const char *,ExceptionInfo *),
  ClampImage(Image *,ExceptionInfo *),
  ListThresholdMaps(FILE *,ExceptionInfo *),
  OrderedDitherImage(Image *,const char *,ExceptionInfo *),
  PerceptibleImage(Image *,const double,ExceptionInfo *),
  RandomThresholdImage(Image *,const double,const double,ExceptionInfo *),
  WhiteThresholdImage(Image *,const char *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

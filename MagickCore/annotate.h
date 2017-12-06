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

  MagickCore image annotation methods.
*/
#ifndef MAGICKCORE_ANNOTATE_H
#define MAGICKCORE_ANNOTATE_H

#include "MagickCore/draw.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport MagickBooleanType
  AnnotateImage(Image *,const DrawInfo *,ExceptionInfo *),
  GetMultilineTypeMetrics(Image *,const DrawInfo *,TypeMetric *,
    ExceptionInfo *),
  GetTypeMetrics(Image *,const DrawInfo *,TypeMetric *,ExceptionInfo *);

extern MagickExport ssize_t
  FormatMagickCaption(Image *,DrawInfo *,const MagickBooleanType,TypeMetric *,
    char **,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

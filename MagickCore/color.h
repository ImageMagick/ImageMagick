/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image color methods.
*/
#ifndef _MAGICKCORE_COLOR_H
#define _MAGICKCORE_COLOR_H

#include "MagickCore/pixel.h"
#include "MagickCore/exception.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedCompliance,
  NoCompliance = 0x0000,
  CSSCompliance = 0x0001,
  SVGCompliance = 0x0001,
  X11Compliance = 0x0002,
  XPMCompliance = 0x0004,
  AllCompliance = 0x7fffffff
} ComplianceType;

typedef struct _ColorInfo
{
  char
    *path,
    *name;

  ComplianceType
    compliance;

  PixelInfo
    color;

  MagickBooleanType
    exempt,
    stealth;

  size_t
    signature;
} ColorInfo;

typedef struct _ErrorInfo
{
  double
    mean_error_per_pixel,
    normalized_mean_error,
    normalized_maximum_error;
} ErrorInfo;

extern MagickExport char
  **GetColorList(const char *,size_t *,ExceptionInfo *);

extern MagickExport const ColorInfo
  *GetColorInfo(const char *,ExceptionInfo *),
  **GetColorInfoList(const char *,size_t *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  IsEquivalentImage(const Image *,const Image *,ssize_t *x,ssize_t *y,
    ExceptionInfo *),
  ListColorInfo(FILE *,ExceptionInfo *),
  QueryColorCompliance(const char *,const ComplianceType,PixelInfo *,
    ExceptionInfo *),
  QueryColorname(const Image *,const PixelInfo *,const ComplianceType,
    char *,ExceptionInfo *);

extern MagickExport void
  ConcatenateColorComponent(const PixelInfo *,const PixelChannel,
    const ComplianceType,char *),
  GetColorTuple(const PixelInfo *,const MagickBooleanType,char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

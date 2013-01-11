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

  MagickCore private string methods.
*/
#ifndef _MAGICKCORE_STRING_PRIVATE_H
#define _MAGICKCORE_STRING_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/locale_.h"

static inline double SiPrefixToDoubleInterval(const char *string,
  const double interval)
{
  char
    *q;

  double
    value;

  value=InterpretSiPrefixValue(string,&q);
  if (*q == '%')
    value*=interval/100.0;
  return(value);
}

static inline double StringToDouble(const char *restrict string,
  char **restrict sentinal)
{
  return(InterpretLocaleValue(string,sentinal));
}

static inline double StringToDoubleInterval(const char *string,
  const double interval)
{
  char
    *q;

  double
    value;

  value=InterpretLocaleValue(string,&q);
  if (*q == '%')
    value*=interval/100.0;
  return(value);
}

static inline int StringToInteger(const char *restrict value)
{
  return((int) strtol(value,(char **) NULL,10));
}

static inline long StringToLong(const char *restrict value)
{
  return(strtol(value,(char **) NULL,10));
}

static inline unsigned long StringToUnsignedLong(const char *restrict value)
{
  return(strtoul(value,(char **) NULL,10));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

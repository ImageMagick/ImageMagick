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

  MagickCore private string methods.
*/
#ifndef MAGICKCORE_STRING_PRIVATE_H
#define MAGICKCORE_STRING_PRIVATE_H

#include <string.h>
#include "MagickCore/locale_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Custom implementation so we can use sscanf without defining _CRT_SECURE_NO_WARNINGS */
static inline int MagickSscanf(const char* buffer,const char* format, ...)
{
  int
    ret;

  va_list
    args;
  va_start(args,format);
#if _MSC_VER
  #pragma warning(push)
  #pragma warning(disable:4996)
#endif
  ret=vsscanf(buffer,format,args);
#if _MSC_VER
  #pragma warning(pop)
#endif
  va_end(args);
  return(ret);
}

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

static inline double StringToDouble(const char *magick_restrict string,
  char *magick_restrict *sentinel)
{
  return(InterpretLocaleValue(string,sentinel));
}

static inline float StringToFloat(const char *magick_restrict string,
  char *magick_restrict *sentinel)
{
  return((float) InterpretLocaleValue(string,sentinel));
}

static inline const char *StringLocateSubstring(const char *haystack,
  const char *needle)
{
#if defined(MAGICKCORE_HAVE_STRCASESTR)
  return(strcasestr(haystack,needle));
#else
  {
    size_t
      length_needle,
      length_haystack;

    size_t
      i;

    if (!haystack || !needle)
      return(NULL);
    length_needle=strlen(needle);
    length_haystack=strlen(haystack)-length_needle+1;
    for (i=0; i < length_haystack; i++)
    {
      size_t
        j;

      for (j=0; j < length_needle; j++)
      {
        unsigned char c1 = (unsigned char) haystack[i+j];
        unsigned char c2 = (unsigned char) needle[j];
        if (toupper((int) c1) != toupper((int) c2))
          goto next;
      }
      return((char *) haystack+i);
      next:
       ;
    }
    return((char *) NULL);
  }
#endif
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

static inline int StringToInteger(const char *magick_restrict value)
{
  if (value == (const char *) NULL)
    return(0);
  return((int) strtol(value,(char **) NULL,10));
}

static inline long StringToLong(const char *magick_restrict value)
{
  if (value == (const char *) NULL)
    return(0);
  return(strtol(value,(char **) NULL,10));
}

static inline MagickOffsetType StringToMagickOffsetType(const char *string,
  const double interval)
{
  double
    value;

  value=SiPrefixToDoubleInterval(string,interval);
  if (value >= (double) MagickULLConstant(~0))
    return((MagickOffsetType) MagickULLConstant(~0));
  return((MagickOffsetType) value);
}

static inline MagickSizeType StringToMagickSizeType(const char *string,
  const double interval)
{
  double
    value;

  value=SiPrefixToDoubleInterval(string,interval);
  if (value >= (double) MagickULLConstant(~0))
    return(MagickULLConstant(~0));
  return((MagickSizeType) value);
}

static inline size_t StringToSizeType(const char *string,const double interval)
{
  double
    value;

  value=SiPrefixToDoubleInterval(string,interval);
  if (value >= (double) MagickULLConstant(~0))
    return(~0UL);
  return((size_t) value);
}

static inline unsigned long StringToUnsignedLong(
  const char *magick_restrict value)
{
  if (value == (const char *) NULL)
    return(0);
  return(strtoul(value,(char **) NULL,10));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

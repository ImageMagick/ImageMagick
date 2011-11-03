/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
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

static inline double InterpretLocaleInterval(const char *string,const double interval)
{
  char
    *q;

  double
    value;

  /*
    Interpret string with International System of Units (SI) unit prefix.
  */
  value=InterpretLocaleValue(string,&q);
  if (*q == '%')
    value*=interval/100.0;
  return(value);
}

static inline int StringToInteger(const char *value)
{
  return((int) strtol(value,(char **) NULL,10));
}

static inline long StringToLong(const char *value)
{
  return(strtol(value,(char **) NULL,10));
}

static inline unsigned long StringToUnsignedLong(const char *value)
{
  return(strtoul(value,(char **) NULL,10));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

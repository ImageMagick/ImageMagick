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

static inline double SiPrefixToDouble(const char *string,const double interval)
{
  char
    *q;

  double
    scale,
    value;

  value=strtod(string,&q);
  scale=1000.0;
  if ((*q != '\0') && (tolower((int) ((unsigned char) *(q+1))) == 'i'))
    scale=1024.0;
  switch (tolower((int) ((unsigned char) *q)))
  {
    case '%': value*=pow(scale,0)*interval/100.0; break;
    case 'k': value*=pow(scale,1); break;
    case 'm': value*=pow(scale,2); break;
    case 'g': value*=pow(scale,3); break;
    case 't': value*=pow(scale,4); break;
    case 'p': value*=pow(scale,5); break;
    case 'e': value*=pow(scale,6); break;
    case 'z': value*=pow(scale,7); break;
    case 'y': value*=pow(scale,8); break;
    default:  break;
  }
  return(value);
}

static inline double StringToDouble(const char *value)
{
  return(strtod(value,(char **) NULL));
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

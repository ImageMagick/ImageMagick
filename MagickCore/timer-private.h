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

  MagickCore private timer methods.
*/
#ifndef MAGICKCORE_TIMER_PRIVATE_H
#define MAGICKCORE_TIMER_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "MagickCore/locale_.h"

static inline void GetMagickUTCTime(const time_t *timep,struct tm *result)
{
#if defined(MAGICKCORE_HAVE_GMTIME_R)
  (void) gmtime_r(timep,result);
#elif defined(_MSC_VER)
  (void) gmtime_s(result,timep);
#else
  {
    struct tm
      *my_time;

    my_time=gmtime(timep);
    if (my_time != (struct tm *) NULL)
      (void) memcpy(result,my_time,sizeof(*my_time));
  }
#endif
}

static inline void GetMagickLocaltime(const time_t *timep,struct tm *result)
{
#if defined(MAGICKCORE_HAVE_GMTIME_R)
  (void) localtime_r(timep,result);
#elif defined(_MSC_VER)
  (void) localtime_s(result,timep);
#else
  {
    struct tm
      *my_time;

    my_time=localtime(timep);
    if (my_time != (struct tm *) NULL)
      (void) memcpy(result,my_time,sizeof(*my_time));
  }
#endif
}

extern MagickExport MagickBooleanType
  IsSourceDataEpochSet(void);

extern MagickExport time_t
  GetMagickTime(void);

static inline MagickBooleanType IsImageTTLExpired(const Image* image)
{
  if (image->ttl == (time_t) 0)
    return(MagickFalse);
  return(image->ttl < time((time_t *) NULL) ? MagickTrue : MagickFalse);
}

static inline time_t ParseMagickTimeToLive(const char *time_to_live)
{
  char
    *q;

  time_t
    ttl;

  /*
    Time to live, absolute or relative, e.g. 1440, 2 hours, 3 days, ...
  */
  ttl=(time_t) InterpretLocaleValue(time_to_live,&q);
  if (q != time_to_live)
    {
      while (isspace((int) ((unsigned char) *q)) != 0)
        q++;
      if (LocaleNCompare(q,"second",6) == 0)
        ttl*=1;
      if (LocaleNCompare(q,"minute",6) == 0)
        ttl*=60;
      if (LocaleNCompare(q,"hour",4) == 0)
        ttl*=3600;
      if (LocaleNCompare(q,"day",3) == 0)
        ttl*=86400;
      if (LocaleNCompare(q,"week",4) == 0)
        ttl*=604800;
      if (LocaleNCompare(q,"month",5) == 0)
        ttl*=2628000;
      if (LocaleNCompare(q,"year",4) == 0)
        ttl*=31536000;
   }
  return(ttl);
}

extern MagickPrivate void
  SetMagickDatePrecision(const unsigned long);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

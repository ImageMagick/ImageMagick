/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
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

static inline void GetMagickUTCtime(const time_t *timep,struct tm *result)
{
#if defined(MAGICKCORE_HAVE_GMTIME_R)
  (void) gmtime_r(timep,result);
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

extern MagickExport time_t
  GetMagickTime(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

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

  MagickCore timer methods.
*/
#ifndef MAGICKCORE_TIMER_H
#define MAGICKCORE_TIMER_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedTimerState,
  StoppedTimerState,
  RunningTimerState
} TimerState;

typedef struct _Timer
{
  double
    start,
    stop,
    total;
} Timer;

typedef struct _TimerInfo
{ 
  Timer
    user,
    elapsed;
  
  TimerState
    state;
  
  size_t
    signature;
} TimerInfo;

extern MagickExport double
  GetElapsedTime(TimerInfo *),
  GetUserTime(TimerInfo *);

extern MagickExport MagickBooleanType
  ContinueTimer(TimerInfo *);

extern MagickExport ssize_t
  FormatMagickTime(const time_t,const size_t,char *);

extern MagickExport TimerInfo
  *AcquireTimerInfo(void),
  *DestroyTimerInfo(TimerInfo *);

extern MagickExport void
  GetTimerInfo(TimerInfo *),
  ResetTimer(TimerInfo *),
  StartTimer(TimerInfo *,const MagickBooleanType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

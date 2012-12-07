/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                    TTTTT  IIIII  M   M  EEEEE  RRRR                         %
%                      T      I    MM MM  E      R   R                        %
%                      T      I    M M M  EEE    RRRR                         %
%                      T      I    M   M  E      R R                          %
%                      T    IIIII  M   M  EEEEE  R  R                         %
%                                                                             %
%                                                                             %
%                         MagickCore Timing Methods                           %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                              January 1993                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Contributed by Bill Radcliffe and Bob Friesenhahn.
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/timer.h"

/*
  Define declarations.
*/
#if defined(macintosh)
#define CLK_TCK  CLOCKS_PER_SEC
#endif
#if !defined(CLK_TCK)
#define CLK_TCK  sysconf(_SC_CLK_TCK)
#endif

/*
  Forward declarations.
*/
static double
  UserTime(void);

static void
  StopTimer(TimerInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e T i m e r I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireTimerInfo() initializes the TimerInfo structure.  It effectively
%  creates a stopwatch and starts it.
%
%  The format of the AcquireTimerInfo method is:
%
%      TimerInfo *AcquireTimerInfo(void)
%
*/
MagickExport TimerInfo *AcquireTimerInfo(void)
{
  TimerInfo
    *timer_info;

  timer_info=(TimerInfo *) AcquireMagickMemory(sizeof(*timer_info));
  if (timer_info == (TimerInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireString");
  (void) ResetMagickMemory(timer_info,0,sizeof(*timer_info));
  timer_info->signature=MagickSignature;
  GetTimerInfo(timer_info);
  return(timer_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n t i n u e T i m e r                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ContinueTimer() resumes a stopped stopwatch. The stopwatch continues
%  counting from the last StartTimer() onwards.
%
%  The format of the ContinueTimer method is:
%
%      MagickBooleanType ContinueTimer(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Time statistics structure.
%
*/
MagickExport MagickBooleanType ContinueTimer(TimerInfo *time_info)
{
  assert(time_info != (TimerInfo *) NULL);
  assert(time_info->signature == MagickSignature);
  if (time_info->state == UndefinedTimerState)
    return(MagickFalse);
  if (time_info->state == StoppedTimerState)
    {
      time_info->user.total-=time_info->user.stop-time_info->user.start;
      time_info->elapsed.total-=time_info->elapsed.stop-
        time_info->elapsed.start;
    }
  time_info->state=RunningTimerState;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y T i m e r I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyTimerInfo() zeros memory associated with the TimerInfo structure.
%
%  The format of the DestroyTimerInfo method is:
%
%      TimerInfo *DestroyTimerInfo(TimerInfo *timer_info)
%
%  A description of each parameter follows:
%
%    o timer_info: The cipher context.
%
*/
MagickExport TimerInfo *DestroyTimerInfo(TimerInfo *timer_info)
{
  assert(timer_info != (TimerInfo *) NULL);
  assert(timer_info->signature == MagickSignature);
  timer_info->signature=(~MagickSignature);
  timer_info=(TimerInfo *) RelinquishMagickMemory(timer_info);
  return(timer_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E l a p s e d T i m e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ElapsedTime() returns the elapsed time (in seconds) since the last call to
%  StartTimer().
%
%  The format of the ElapsedTime method is:
%
%      double ElapsedTime()
%
*/
static double ElapsedTime(void)
{
#if defined(MAGICKCORE_HAVE_TIMES)
  struct tms
    timer;

  return((double) times(&timer)/CLK_TCK);
#else
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  return(NTElapsedTime());
#else
  return((double) clock()/CLK_TCK);
#endif
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E l a p s e d T i m e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetElapsedTime() returns the elapsed time (in seconds) passed between the
%  start and stop events. If the stopwatch is still running, it is stopped
%  first.
%
%  The format of the GetElapsedTime method is:
%
%      double GetElapsedTime(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
*/
MagickExport double GetElapsedTime(TimerInfo *time_info)
{
  assert(time_info != (TimerInfo *) NULL);
  assert(time_info->signature == MagickSignature);
  if (time_info->state == UndefinedTimerState)
    return(0.0);
  if (time_info->state == RunningTimerState)
    StopTimer(time_info);
  return(time_info->elapsed.total);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t T i m e r I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetTimerInfo() initializes the TimerInfo structure.
%
%  The format of the GetTimerInfo method is:
%
%      void GetTimerInfo(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
*/
MagickExport void GetTimerInfo(TimerInfo *time_info)
{
  /*
    Create a stopwatch and start it.
  */
  assert(time_info != (TimerInfo *) NULL);
  (void) ResetMagickMemory(time_info,0,sizeof(*time_info));
  time_info->state=UndefinedTimerState;
  time_info->signature=MagickSignature;
  StartTimer(time_info,MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t U s e r T i m e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetUserTime() returns the User time (user and system) by the operating
%  system (in seconds) between the start and stop events. If the stopwatch is
%  still running, it is stopped first.
%
%  The format of the GetUserTime method is:
%
%      double GetUserTime(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
*/
MagickExport double GetUserTime(TimerInfo *time_info)
{
  assert(time_info != (TimerInfo *) NULL);
  assert(time_info->signature == MagickSignature);
  if (time_info->state == UndefinedTimerState)
    return(0.0);
  if (time_info->state == RunningTimerState)
    StopTimer(time_info);
  return(time_info->user.total);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t T i m e r                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetTimer() resets the stopwatch.
%
%  The format of the ResetTimer method is:
%
%      void ResetTimer(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
*/
MagickExport void ResetTimer(TimerInfo *time_info)
{
  assert(time_info != (TimerInfo *) NULL);
  assert(time_info->signature == MagickSignature);
  StopTimer(time_info);
  time_info->elapsed.stop=0.0;
  time_info->user.stop=0.0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S t a r t T i m e r                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StartTimer() starts the stopwatch.
%
%  The format of the StartTimer method is:
%
%      void StartTimer(TimerInfo *time_info,const MagickBooleanType reset)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
%    o  reset: If reset is MagickTrue, then the stopwatch is reset prior to
%       starting.  If reset is MagickFalse, then timing is continued without
%       resetting the stopwatch.
%
*/
MagickExport void StartTimer(TimerInfo *time_info,const MagickBooleanType reset)
{
  assert(time_info != (TimerInfo *) NULL);
  assert(time_info->signature == MagickSignature);
  if (reset != MagickFalse)
    {
      /*
        Reset the stopwatch before starting it.
      */
      time_info->user.total=0.0;
      time_info->elapsed.total=0.0;
    }
  if (time_info->state != RunningTimerState)
    {
      time_info->elapsed.start=ElapsedTime();
      time_info->user.start=UserTime();
    }
  time_info->state=RunningTimerState;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S t o p T i m e r                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StopTimer() stops the stopwatch.
%
%  The format of the StopTimer method is:
%
%      void StopTimer(TimerInfo *time_info)
%
%  A description of each parameter follows.
%
%    o  time_info: Timer statistics structure.
%
*/
static void StopTimer(TimerInfo *time_info)
{
  assert(time_info != (TimerInfo *) NULL);
  assert(time_info->signature == MagickSignature);
  time_info->elapsed.stop=ElapsedTime();
  time_info->user.stop=UserTime();
  if (time_info->state == RunningTimerState)
    {
      time_info->user.total+=time_info->user.stop-
        time_info->user.start+MagickEpsilon;
      time_info->elapsed.total+=time_info->elapsed.stop-
        time_info->elapsed.start+MagickEpsilon;
    }
  time_info->state=StoppedTimerState;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   U s e r T i m e                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UserTime() returns the total time the process has been scheduled (in
%  seconds) since the last call to StartTimer().
%
%  The format of the UserTime method is:
%
%      double UserTime()
%
*/
static double UserTime(void)
{
#if defined(MAGICKCORE_HAVE_TIMES)
  struct tms
    timer;

  (void) times(&timer);
  return((double) (timer.tms_utime+timer.tms_stime)/CLK_TCK);
#else
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  return(NTUserTime());
#else
  return((double) clock()/CLK_TCK);
#endif
#endif
}

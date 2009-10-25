/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                CCCC   OOO   N   N  V   V  EEEEE  RRRR   TTTTT               %
%               C      O   O  NN  N  V   V  E      R   R    T                 %
%               C      O   O  N N N  V   V  EEE    RRRR     T                 %
%               C      O   O  N  NN   V V   E      R R      T                 %
%                CCCC   OOO   N   N    V    EEEEE  R  R     T                 %
%                                                                             %
%                                                                             %
%                Convert an image from one format to another.                 %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                April 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
%  Convert converts an input file using one image format to an output file
%  with a differing image format. By default, the image format is determined
%  by its magic number. To specify a particular image format, precede the
%  filename with an image format name and a colon (i.e. ps:image) or specify
%  the image type as the filename suffix (i.e. image.ps). Specify file as -
%  for standard input or output. If file has the extension .Z, the file is
%  decoded with uncompress.
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/thread-private.h"
#include "wand/MagickWand.h"
#if defined(__WINDOWS__)
#include <windows.h>
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M a i n                                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/
int main(int argc,char **argv)
{
  char
    *option;

  double
    duration,
    elapsed_time,
    user_time;

  ExceptionInfo
    *exception;

  ImageInfo
    *image_info;

  long
    j;

  MagickBooleanType
    concurrent,
    regard_warnings,
    status;

  register long
    i;

  TimerInfo
    *timer;

  unsigned long
    iterations;

  MagickCoreGenesis(*argv,MagickTrue);
  concurrent=MagickFalse;
  duration=(-1.0);
  exception=AcquireExceptionInfo();
  iterations=1;
  status=MagickTrue;
  regard_warnings=MagickFalse;
  for (i=1; i < (long) (argc-1); i++)
  {
    option=argv[i];
    if ((strlen(option) == 1) || ((*option != '-') && (*option != '+')))
      continue;
    if (LocaleCompare("bench",option+1) == 0)
      iterations=(unsigned long) atol(argv[++i]);
    if (LocaleCompare("concurrent",option+1) == 0)
      concurrent=MagickTrue;
    if (LocaleCompare("debug",option+1) == 0)
      (void) SetLogEventMask(argv[++i]);
    if (LocaleCompare("duration",option+1) == 0)
      duration=(unsigned long) atof(argv[++i]);
    if (LocaleCompare("regard-warnings",option+1) == 0)
      regard_warnings=MagickTrue;
  }
  timer=(TimerInfo *) NULL;
  if (iterations > 1)
    timer=AcquireTimerInfo();
  if (concurrent != MagickFalse)
    SetOpenMPNested(1);
  # pragma omp parallel for shared(status)
  for (i=0; i < (long) (concurrent != MagickFalse ? iterations : 1); i++)
  {
    if (status == MagickFalse)
      continue;
    if (GetElapsedTime(timer) > duration)
      continue;
    ContinueTimer(timer);
    for (j=0; j < (long) (concurrent == MagickFalse ? iterations : 1); j++)
    {
      if (status == MagickFalse)
        break;
      if (GetElapsedTime(timer) > duration)
        break;
      ContinueTimer(timer);
      image_info=AcquireImageInfo();
      status=ConvertImageCommand(image_info,argc,argv,(char **) NULL,exception);
      # pragma omp critical (MagickCore_Convert_Utility)
      if (exception->severity != UndefinedException)
        {
          if ((exception->severity > ErrorException) ||
              (regard_warnings != MagickFalse))
            status=MagickTrue;
          CatchException(exception);
        }
      image_info=DestroyImageInfo(image_info);
    }
  }
  if (iterations > 1)
    {
      elapsed_time=GetElapsedTime(timer);
      user_time=GetUserTime(timer);
      (void) fprintf(stderr,"Performance: %lui %gips %0.3fu %ld:%02ld.%03ld\n",
        iterations,1.0*iterations/elapsed_time,user_time,(long)
        (elapsed_time/60.0),(long) floor(fmod(elapsed_time,60.0)),
        (long) (1000.0*(elapsed_time-floor(elapsed_time))));
      timer=DestroyTimerInfo(timer);
    }
  exception=DestroyExceptionInfo(exception);
  MagickCoreTerminus();
  return(status == MagickFalse ? 0 : 1);
}

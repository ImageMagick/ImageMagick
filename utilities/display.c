/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               DDDD   IIIII  SSSSS  PPPP   L       AAA   Y   Y               %
%               D   D    I    SS     P   P  L      A   A   Y Y                %
%               D   D    I     SSS   PPPP   L      AAAAA    Y                 %
%               D   D    I       SS  P      L      A   A    Y                 %
%               DDDD   IIIII  SSSSS  P      LLLLL  A   A    Y                 %
%                                                                             %
%                                                                             %
%                       Interactively Display an Image.                       %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                                July 1992                                    %
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
%  Display is a machine architecture independent image processing
%  and display program.  It can display any image in the MIFF format on
%  any workstation display running X.  Display first determines the
%  hardware capabilities of the workstation.  If the number of unique
%  colors in the image is less than or equal to the number the workstation
%  can support, the image is displayed in an X window.  Otherwise the
%  number of colors in the image is first reduced to match the color
%  resolution of the workstation before it is displayed.
%
%  This means that a continuous-tone 24 bits-per-pixel image can display on a
%  8 bit pseudo-color device or monochrome device.  In most instances the
%  reduced color image closely resembles the original.  Alternatively, a
%  monochrome or pseudo-color image can display on a continuous-tone 24
%  bits-per-pixel device.
%
%
%
*/

/*
  Include declarations.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "wand/MagickWand.h"
#if defined(__WINDOWS__)
#include <windows.h>
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    M a i n                                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

#if defined(__WINDOWS__)
int WINAPI WinMain(HINSTANCE instance,HINSTANCE last,LPSTR command,int state)
{
  char
    **argv;

  int
    argc,
    main(int,char **);

  argv=StringToArgv(command,&argc);
  return(main(argc,argv));
}
#endif

int main(int argc,char **argv)
{
  char
    *option;

  double
    elapsed_time,
    user_time;

  ExceptionInfo
    *exception;

  ImageInfo
    *image_info;

  MagickBooleanType
    regard_warnings,
    status;

  register long
    i;

  TimerInfo
    timer;

  unsigned long
    iterations;

  MagickCoreGenesis(*argv,MagickTrue);
  exception=AcquireExceptionInfo();
  iterations=1;
  status=MagickFalse;
  regard_warnings=MagickFalse;
  for (i=1; i < (long) (argc-1); i++)
  {
    option=argv[i];
    if ((strlen(option) == 1) || ((*option != '-') && (*option != '+')))
      continue;
    if (LocaleCompare("bench",option+1) == 0)
      iterations=(unsigned long) atol(argv[++i]);
    if (LocaleCompare("debug",option+1) == 0)
      (void) SetLogEventMask(argv[++i]);
    if (LocaleCompare("regard-warnings",option+1) == 0)
      regard_warnings=MagickTrue;
  }
  GetTimerInfo(&timer);
  for (i=0; i < (long) iterations; i++)
  {
    image_info=AcquireImageInfo();
    status=DisplayImageCommand(image_info,argc,argv,(char **) NULL,exception);
    if (exception->severity != UndefinedException)
      {
        if ((exception->severity > ErrorException) ||
            (regard_warnings != MagickFalse))
          status=MagickTrue;
        CatchException(exception);
      }
    image_info=DestroyImageInfo(image_info);
  }
  if (iterations > 1)
    {
      elapsed_time=GetElapsedTime(&timer);
      user_time=GetUserTime(&timer);
      (void) fprintf(stderr,"Performance: %lui %gips %0.3fu %ld:%02ld\n",
        iterations,1.0*iterations/elapsed_time,user_time,(long)
        (elapsed_time/60.0+0.5),(long) ceil(fmod(elapsed_time,60.0)));
    }
  exception=DestroyExceptionInfo(exception);
  MagickCoreTerminus();
  return(status == MagickFalse ? 0 : 1);
}

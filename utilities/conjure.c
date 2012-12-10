/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                CCCC   OOO   N   N  JJJJJ  U   U  RRRR   EEEEE               %
%               C      O   O  NN  N    J    U   U  R   R  E                   %
%               C      O   O  N N N    J    U   U  RRRR   EEE                 %
%               C      O   O  N  NN  J J    U   U  R R    E                   %
%                CCCC   OOO   N   N  JJJ     UUU   R  R   EEEEE               %
%                                                                             %
%                                                                             %
%                     Interpret Magick Scripting Language.                    %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               December 2001                                 %
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
%  Conjure interprets and executes scripts in the Magick Scripting Language
%  (MSL). The Magick scripting language (MSL) will primarily benefit those
%  that want to accomplish custom image processing tasks but do not wish
%  to program, or those that do not have access to a Perl interpreter or a
%  compiler. The interpreter is called conjure and here is an example script:
%
%      <?xml version="1.0" encoding="UTF-8"?>
%      <image size="400x400" >
%      <read filename="image.gif" />
%      <get width="base-width" height="base-height" />
%      <resize geometry="%[dimensions]" />
%      <get width="width" height="height" />
%      <print output="Image sized from %[base-width]x%[base-height]
%          to %[width]x%[height].\n" />
%      <write filename="image.png" />
%      </image>
%
%
*/

/*
  Include declarations.
*/

/*
  Include declarations.
*/
#include "wand/studio.h"
#include "wand/MagickWand.h"

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

static int ConjureMain(int argc,char **argv)
{
  ExceptionInfo
    *exception;

  ImageInfo
    *image_info;

  MagickBooleanType
    status;

  MagickCoreGenesis(*argv,MagickTrue);
  exception=AcquireExceptionInfo();
  image_info=AcquireImageInfo();
  status=MagickCommandGenesis(image_info,ConjureImageCommand,argc,argv,
    (char **) NULL,exception);
  image_info=DestroyImageInfo(image_info);
  exception=DestroyExceptionInfo(exception);
  MagickCoreTerminus();
  return(status);
}

#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
int main(int argc,char **argv)
{
  return(ConjureMain(argc,argv));
}
#else
int wmain(int argc,wchar_t *argv[])
{
  char
    **utf8;

  int
    status;

  register int
    i;

  utf8=NTArgvToUTF8(argc,argv);
  status=ConjureMain(argc,utf8);
  for (i=0; i < argc; i++)
    utf8[i]=DestroyString(utf8[i]);
  utf8=(char **) RelinquishMagickMemory(utf8);
  return(status);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              M   M   OOO    GGGG  RRRR   IIIII  FFFFF  Y   Y                %
%              MM MM  O   O  G      R   R    I    F       Y Y                 %
%              M M M  O   O  G GG   RRRRR    I    FFF      Y                  %
%              M   M  O   O  G   G  R R      I    F        Y                  %
%              M   M   OOO    GGG   R  R   IIIII  F        Y                  %
%                                                                             %
%                                                                             %
%               Transmogrify an Image or Sequence of Images.                  %
%                                                                             %
%                           Software Design                                   %
%                             John Cristy                                     %
%                            December 1992                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
%  Mogrify transforms an image or a sequence of images. These transforms
%  include image scaling, image rotation, color reduction, and others. The
%  transmogrified image overwrites the original image.
%
%
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
int main(int argc,char **argv)
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
  status=MagickCommandGenesis(image_info,MogrifyImageCommand,argc,argv,
    (char **) NULL,exception);
  image_info=DestroyImageInfo(image_info);
  exception=DestroyExceptionInfo(exception);
  MagickCoreTerminus();
  return(status);
}

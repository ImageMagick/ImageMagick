/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                      BBBB    OOO    OOO   SSSSS  TTTTT                      %
%                      B   B  O   O  O   O  SS       T                        %
%                      BBBB   O   O  O   O   SSS     T                        %
%                      B   B  O   O  O   O     SS    T                        %
%                      BBBB    OOO    OOO   SSSSS    T                        %
%                                                                             %
%                                Boost An Image                               %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                November 2009                                %
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
% Boost certain algorithms by executing in concert across heterogeneous
% platforms consisting of CPUs, GPUs, and other processors (in development).
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/MagickCore.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   b o o s t I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  boostImage() boosts the performance of certain image processing algorithms
%  by utilizing the OpenCL framework to execute the algorithm across
%  heterogeneous platforms consisting of CPUs, GPUs, and other processors.  This
%  filter is experimental and is not recommended for general use.
%  The format of the boostImage method is:
%
%      unsigned long boostImage(Image *images,const int argc,
%        char **argv,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the address of a structure of type Image.
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%    o exception: return any errors or warnings in this structure.
%
*/
ModuleExport unsigned long boostImage(Image **images,const int argc,
  const char **argv,ExceptionInfo *exception)
{
  assert(images != (Image **) NULL);
  assert(*images != (Image *) NULL);
  assert((*images)->signature == MagickSignature);
#if !defined(MAGICKCORE_OPENCL_SUPPORT)
  (void) argc;
  (void) argv;
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","`%s' (OpenCL)",(*images)->filename);
#else
  {
    Image
      *image;

    (void) argc;
    (void) argv;
    (void) exception;
  }
#endif
  return(MagickImageFilterSignature);
}

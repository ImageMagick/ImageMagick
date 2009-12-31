/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%             CCCC   OOO   N   N  V   V   OOO   L      V   V  EEEEE           %
%            C      O   O  NN  N  V   V  O   O  L      V   V  E               %
%            C      O   O  N N N  V   V  O   O  L      V   V  EEE             %
%            C      O   O  N  NN   V V   O   O  L       V V   E               %
%             CCCC   OOO   N   N    V     OOO   LLLLL    V    EEEEE           %
%                                                                             %
%                              Convolve An Image                              %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                November 2009                                %
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
% Convolve an image by executing in concert across heterogeneous platforms
% consisting of CPUs, GPUs, and other processors (in development).
%
*/

/*
  Include declarations.
*/
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "magick/studio.h"
#include "magick/MagickCore.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   c o n v o l v e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  convolveImage() convolves an image by utilizing the OpenCL framework to
%  execute the algorithm across heterogeneous platforms consisting of CPUs,
%  GPUs, and other processors.  This filter is experimental and is not
%  recommended for general use.  The format of the convolveImage method is:
%
%      unsigned long convolveImage(Image *images,const int argc,
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

#if defined(MAGICKCORE_OPENCL_SUPPORT)

static cl_context OpenCLGenesis(cl_int *status)
{
  cl_context
    context;

  context=clCreateContextFromType(NULL,CL_DEVICE_TYPE_CPU,NULL,NULL,status);
  return(context);
}

static double *ParseKernel(const char *value,unsigned long *order)
{
  char
    token[MaxTextExtent];

  const char
    *p;

  double
    *kernel;

  register long
    i;

  /*
    Parse convolution kernel.
  */
  p=(const char *) value;
  for (i=0; *p != '\0'; i++)
  {
    GetMagickToken(p,&p,token);
    if (*token == ',')
      GetMagickToken(p,&p,token);
  }
  *order=(unsigned long) sqrt((double) i+1.0);
  kernel=(double *) AcquireQuantumMemory(*order,*order*sizeof(*kernel));
  if (kernel == (double *) NULL)
    return(kernel);
  p=(const char *) value;
  for (i=0; (i < (long) (*order**order)) && (*p != '\0'); i++)
  {
    GetMagickToken(p,&p,token);
    if (*token == ',')
      GetMagickToken(p,&p,token);
    kernel[i]=strtod(token,(char **) NULL);
  }
  for ( ; i < (long) (*order**order); i++)
    kernel[i]=0.0;
  return(kernel);
}
#endif

ModuleExport unsigned long convolveImage(Image **images,const int argc,
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
    cl_context
      context;

    cl_int
      status;

    double
      *kernel;

    Image
      *image;

    unsigned long
      order;

    if (argc < 1)
      return(MagickImageFilterSignature);
    /*
      Convolve image.
    */
    kernel=ParseKernel(argv[0],&order);
    if (kernel == (double *) NULL)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",(*images)->filename);
    context=OpenCLGenesis(&status);
    image=(*images);
    for ( ; image != (Image *) NULL; image=GetNextImageInList(image))
    {
    }
    kernel=(double *) RelinquishMagickMemory(kernel);
  }
#endif
  return(MagickImageFilterSignature);
}

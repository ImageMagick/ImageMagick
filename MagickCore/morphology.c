/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    M   M    OOO    RRRR   PPPP   H   H   OOO   L       OOO    GGGG  Y   Y   %
%    MM MM   O   O   R   R  P   P  H   H  O   O  L      O   O  G       Y Y    %
%    M M M   O   O   RRRR   PPPP   HHHHH  O   O  L      O   O  G GGG    Y     %
%    M   M   O   O   R R    P      H   H  O   O  L      O   O  G   G    Y     %
%    M   M    OOO    R  R   P      H   H   OOO   LLLLL   OOO    GGG     Y     %
%                                                                             %
%                                                                             %
%                        MagickCore Morphology Methods                        %
%                                                                             %
%                              Software Design                                %
%                              Anthony Thyssen                                %
%                               January 2010                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Morphology is the application of various kernels, of any size or shape, to an
% image in various ways (typically binary, but not always).
%
% Convolution (weighted sum or average) is just one specific type of
% morphology. Just one that is very common for image bluring and sharpening
% effects.  Not only 2D Gaussian blurring, but also 2-pass 1D Blurring.
%
% This module provides not only a general morphology function, and the ability
% to apply more advanced or iterative morphologies, but also functions for the
% generation of many different types of kernel arrays from user supplied
% arguments. Prehaps even the generation of a kernel from a small image.
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/color-private.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/gem-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/morphology.h"
#include "MagickCore/morphology-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/prepress.h"
#include "MagickCore/quantize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/registry.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"

/*
  Other global definitions used by module.
*/
#define Minimize(assign,value) assign=MagickMin(assign,value)
#define Maximize(assign,value) assign=MagickMax(assign,value)

/* Integer Factorial Function - for a Binomial kernel */
#if 1
static inline size_t fact(size_t n)
{
  size_t f,l;
  for(f=1, l=2; l <= n; f=f*l, l++);
  return(f);
}
#elif 1 /* glibc floating point alternatives */
#define fact(n) ((size_t)tgamma((double)n+1))
#else
#define fact(n) ((size_t)lgamma((double)n+1))
#endif


/* Currently these are only internal to this module */
static void
  CalcKernelMetaData(KernelInfo *),
  ExpandMirrorKernelInfo(KernelInfo *),
  ExpandRotateKernelInfo(KernelInfo *, const double),
  RotateKernelInfo(KernelInfo *, double);


/* Quick function to find last kernel in a kernel list */
static inline KernelInfo *LastKernelInfo(KernelInfo *kernel)
{
  while (kernel->next != (KernelInfo *) NULL)
    kernel=kernel->next;
  return(kernel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c q u i r e K e r n e l I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireKernelInfo() takes the given string (generally supplied by the
%  user) and converts it into a Morphology/Convolution Kernel.  This allows
%  users to specify a kernel from a number of pre-defined kernels, or to fully
%  specify their own kernel for a specific Convolution or Morphology
%  Operation.
%
%  The kernel so generated can be any rectangular array of floating point
%  values (doubles) with the 'control point' or 'pixel being affected'
%  anywhere within that array of values.
%
%  Previously IM was restricted to a square of odd size using the exact
%  center as origin, this is no longer the case, and any rectangular kernel
%  with any value being declared the origin. This in turn allows the use of
%  highly asymmetrical kernels.
%
%  The floating point values in the kernel can also include a special value
%  known as 'nan' or 'not a number' to indicate that this value is not part
%  of the kernel array. This allows you to shaped the kernel within its
%  rectangular area. That is 'nan' values provide a 'mask' for the kernel
%  shape.  However at least one non-nan value must be provided for correct
%  working of a kernel.
%
%  The returned kernel should be freed using the DestroyKernelInfo() when you
%  are finished with it.  Do not free this memory yourself.
%
%  Input kernel defintion strings can consist of any of three types.
%
%    "name:args[[@><]"
%         Select from one of the built in kernels, using the name and
%         geometry arguments supplied.  See AcquireKernelBuiltIn()
%
%    "WxH[+X+Y][@><]:num, num, num ..."
%         a kernel of size W by H, with W*H floating point numbers following.
%         the 'center' can be optionally be defined at +X+Y (such that +0+0
%         is top left corner). If not defined the pixel in the center, for
%         odd sizes, or to the immediate top or left of center for even sizes
%         is automatically selected.
%
%    "num, num, num, num, ..."
%         list of floating point numbers defining an 'old style' odd sized
%         square kernel.  At least 9 values should be provided for a 3x3
%         square kernel, 25 for a 5x5 square kernel, 49 for 7x7, etc.
%         Values can be space or comma separated.  This is not recommended.
%
%  You can define a 'list of kernels' which can be used by some morphology
%  operators A list is defined as a semi-colon separated list kernels.
%
%     " kernel ; kernel ; kernel ; "
%
%  Any extra ';' characters, at start, end or between kernel defintions are
%  simply ignored.
%
%  The special flags will expand a single kernel, into a list of rotated
%  kernels. A '@' flag will expand a 3x3 kernel into a list of 45-degree
%  cyclic rotations, while a '>' will generate a list of 90-degree rotations.
%  The '<' also exands using 90-degree rotates, but giving a 180-degree
%  reflected kernel before the +/- 90-degree rotations, which can be important
%  for Thinning operations.
%
%  Note that 'name' kernels will start with an alphabetic character while the
%  new kernel specification has a ':' character in its specification string.
%  If neither is the case, it is assumed an old style of a simple list of
%  numbers generating a odd-sized square kernel has been given.
%
%  The format of the AcquireKernal method is:
%
%      KernelInfo *AcquireKernelInfo(const char *kernel_string)
%
%  A description of each parameter follows:
%
%    o kernel_string: the Morphology/Convolution kernel wanted.
%
*/

/* This was separated so that it could be used as a separate
** array input handling function, such as for -color-matrix
*/
static KernelInfo *ParseKernelArray(const char *kernel_string)
{
  KernelInfo
    *kernel;

  char
    token[MagickPathExtent];

  const char
    *p,
    *end;

  register ssize_t
    i;

  double
    nan = sqrt((double)-1.0);  /* Special Value : Not A Number */

  MagickStatusType
    flags;

  GeometryInfo
    args;

  kernel=(KernelInfo *) AcquireQuantumMemory(1,sizeof(*kernel));
  if (kernel == (KernelInfo *) NULL)
    return(kernel);
  (void) memset(kernel,0,sizeof(*kernel));
  kernel->minimum = kernel->maximum = kernel->angle = 0.0;
  kernel->negative_range = kernel->positive_range = 0.0;
  kernel->type = UserDefinedKernel;
  kernel->next = (KernelInfo *) NULL;
  kernel->signature=MagickCoreSignature;
  if (kernel_string == (const char *) NULL)
    return(kernel);

  /* find end of this specific kernel definition string */
  end = strchr(kernel_string, ';');
  if ( end == (char *) NULL )
    end = strchr(kernel_string, '\0');

  /* clear flags - for Expanding kernel lists thorugh rotations */
   flags = NoValue;

  /* Has a ':' in argument - New user kernel specification
     FUTURE: this split on ':' could be done by StringToken()
   */
  p = strchr(kernel_string, ':');
  if ( p != (char *) NULL && p < end)
    {
      /* ParseGeometry() needs the geometry separated! -- Arrgghh */
      memcpy(token, kernel_string, (size_t) (p-kernel_string));
      token[p-kernel_string] = '\0';
      SetGeometryInfo(&args);
      flags = ParseGeometry(token, &args);

      /* Size handling and checks of geometry settings */
      if ( (flags & WidthValue) == 0 ) /* if no width then */
        args.rho = args.sigma;         /* then  width = height */
      if ( args.rho < 1.0 )            /* if width too small */
         args.rho = 1.0;               /* then  width = 1 */
      if ( args.sigma < 1.0 )          /* if height too small */
        args.sigma = args.rho;         /* then  height = width */
      kernel->width = (size_t)args.rho;
      kernel->height = (size_t)args.sigma;

      /* Offset Handling and Checks */
      if ( args.xi  < 0.0 || args.psi < 0.0 )
        return(DestroyKernelInfo(kernel));
      kernel->x = ((flags & XValue)!=0) ? (ssize_t)args.xi
                                        : (ssize_t) (kernel->width-1)/2;
      kernel->y = ((flags & YValue)!=0) ? (ssize_t)args.psi
                                        : (ssize_t) (kernel->height-1)/2;
      if ( kernel->x >= (ssize_t) kernel->width ||
           kernel->y >= (ssize_t) kernel->height )
        return(DestroyKernelInfo(kernel));

      p++; /* advance beyond the ':' */
    }
  else
    { /* ELSE - Old old specification, forming odd-square kernel */
      /* count up number of values given */
      p=(const char *) kernel_string;
      while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == '\''))
        p++;  /* ignore "'" chars for convolve filter usage - Cristy */
      for (i=0; p < end; i++)
      {
        (void) GetNextToken(p,&p,MagickPathExtent,token);
        if (*token == ',')
          (void) GetNextToken(p,&p,MagickPathExtent,token);
      }
      /* set the size of the kernel - old sized square */
      kernel->width = kernel->height= (size_t) sqrt((double) i+1.0);
      kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;
      p=(const char *) kernel_string;
      while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == '\''))
        p++;  /* ignore "'" chars for convolve filter usage - Cristy */
    }

  /* Read in the kernel values from rest of input string argument */
  kernel->values=(MagickRealType *) MagickAssumeAligned(AcquireAlignedMemory(
    kernel->width,kernel->height*sizeof(*kernel->values)));
  if (kernel->values == (MagickRealType *) NULL)
    return(DestroyKernelInfo(kernel));
  kernel->minimum=MagickMaximumValue;
  kernel->maximum=(-MagickMaximumValue);
  kernel->negative_range = kernel->positive_range = 0.0;
  for (i=0; (i < (ssize_t) (kernel->width*kernel->height)) && (p < end); i++)
  {
    (void) GetNextToken(p,&p,MagickPathExtent,token);
    if (*token == ',')
      (void) GetNextToken(p,&p,MagickPathExtent,token);
    if (    LocaleCompare("nan",token) == 0
        || LocaleCompare("-",token) == 0 ) {
      kernel->values[i] = nan; /* this value is not part of neighbourhood */
    }
    else {
      kernel->values[i] = StringToDouble(token,(char **) NULL);
      ( kernel->values[i] < 0)
          ?  ( kernel->negative_range += kernel->values[i] )
          :  ( kernel->positive_range += kernel->values[i] );
      Minimize(kernel->minimum, kernel->values[i]);
      Maximize(kernel->maximum, kernel->values[i]);
    }
  }

  /* sanity check -- no more values in kernel definition */
  (void) GetNextToken(p,&p,MagickPathExtent,token);
  if ( *token != '\0' && *token != ';' && *token != '\'' )
    return(DestroyKernelInfo(kernel));

#if 0
  /* this was the old method of handling a incomplete kernel */
  if ( i < (ssize_t) (kernel->width*kernel->height) ) {
    Minimize(kernel->minimum, kernel->values[i]);
    Maximize(kernel->maximum, kernel->values[i]);
    for ( ; i < (ssize_t) (kernel->width*kernel->height); i++)
      kernel->values[i]=0.0;
  }
#else
  /* Number of values for kernel was not enough - Report Error */
  if ( i < (ssize_t) (kernel->width*kernel->height) )
    return(DestroyKernelInfo(kernel));
#endif

  /* check that we recieved at least one real (non-nan) value! */
  if (kernel->minimum == MagickMaximumValue)
    return(DestroyKernelInfo(kernel));

  if ( (flags & AreaValue) != 0 )         /* '@' symbol in kernel size */
    ExpandRotateKernelInfo(kernel, 45.0); /* cyclic rotate 3x3 kernels */
  else if ( (flags & GreaterValue) != 0 ) /* '>' symbol in kernel args */
    ExpandRotateKernelInfo(kernel, 90.0); /* 90 degree rotate of kernel */
  else if ( (flags & LessValue) != 0 )    /* '<' symbol in kernel args */
    ExpandMirrorKernelInfo(kernel);       /* 90 degree mirror rotate */

  return(kernel);
}

static KernelInfo *ParseKernelName(const char *kernel_string,
  ExceptionInfo *exception)
{
  char
    token[MagickPathExtent];

  const char
    *p,
    *end;

  GeometryInfo
    args;

  KernelInfo
    *kernel;

  MagickStatusType
    flags;

  ssize_t
    type;

  /* Parse special 'named' kernel */
  (void) GetNextToken(kernel_string,&p,MagickPathExtent,token);
  type=ParseCommandOption(MagickKernelOptions,MagickFalse,token);
  if ( type < 0 || type == UserDefinedKernel )
    return((KernelInfo *) NULL);  /* not a valid named kernel */

  while (((isspace((int) ((unsigned char) *p)) != 0) ||
          (*p == ',') || (*p == ':' )) && (*p != '\0') && (*p != ';'))
    p++;

  end = strchr(p, ';'); /* end of this kernel defintion */
  if ( end == (char *) NULL )
    end = strchr(p, '\0');

  /* ParseGeometry() needs the geometry separated! -- Arrgghh */
  memcpy(token, p, (size_t) (end-p));
  token[end-p] = '\0';
  SetGeometryInfo(&args);
  flags = ParseGeometry(token, &args);

#if 0
  /* For Debugging Geometry Input */
  (void) FormatLocaleFile(stderr, "Geometry = 0x%04X : %lg x %lg %+lg %+lg\n",
    flags, args.rho, args.sigma, args.xi, args.psi );
#endif

  /* special handling of missing values in input string */
  switch( type ) {
    /* Shape Kernel Defaults */
    case UnityKernel:
      if ( (flags & WidthValue) == 0 )
        args.rho = 1.0;    /* Default scale = 1.0, zero is valid */
      break;
    case SquareKernel:
    case DiamondKernel:
    case OctagonKernel:
    case DiskKernel:
    case PlusKernel:
    case CrossKernel:
      if ( (flags & HeightValue) == 0 )
        args.sigma = 1.0;    /* Default scale = 1.0, zero is valid */
      break;
    case RingKernel:
      if ( (flags & XValue) == 0 )
        args.xi = 1.0;       /* Default scale = 1.0, zero is valid */
      break;
    case RectangleKernel:    /* Rectangle - set size defaults */
      if ( (flags & WidthValue) == 0 ) /* if no width then */
        args.rho = args.sigma;         /* then  width = height */
      if ( args.rho < 1.0 )            /* if width too small */
          args.rho = 3;                /* then  width = 3 */
      if ( args.sigma < 1.0 )          /* if height too small */
        args.sigma = args.rho;         /* then  height = width */
      if ( (flags & XValue) == 0 )     /* center offset if not defined */
        args.xi = (double)(((ssize_t)args.rho-1)/2);
      if ( (flags & YValue) == 0 )
        args.psi = (double)(((ssize_t)args.sigma-1)/2);
      break;
    /* Distance Kernel Defaults */
    case ChebyshevKernel:
    case ManhattanKernel:
    case OctagonalKernel:
    case EuclideanKernel:
      if ( (flags & HeightValue) == 0 )           /* no distance scale */
        args.sigma = 100.0;                       /* default distance scaling */
      else if ( (flags & AspectValue ) != 0 )     /* '!' flag */
        args.sigma = QuantumRange/(args.sigma+1); /* maximum pixel distance */
      else if ( (flags & PercentValue ) != 0 )    /* '%' flag */
        args.sigma *= QuantumRange/100.0;         /* percentage of color range */
      break;
    default:
      break;
  }

  kernel = AcquireKernelBuiltIn((KernelInfoType)type, &args, exception);
  if ( kernel == (KernelInfo *) NULL )
    return(kernel);

  /* global expand to rotated kernel list - only for single kernels */
  if ( kernel->next == (KernelInfo *) NULL ) {
    if ( (flags & AreaValue) != 0 )         /* '@' symbol in kernel args */
      ExpandRotateKernelInfo(kernel, 45.0);
    else if ( (flags & GreaterValue) != 0 ) /* '>' symbol in kernel args */
      ExpandRotateKernelInfo(kernel, 90.0);
    else if ( (flags & LessValue) != 0 )    /* '<' symbol in kernel args */
      ExpandMirrorKernelInfo(kernel);
  }

  return(kernel);
}

MagickExport KernelInfo *AcquireKernelInfo(const char *kernel_string,
  ExceptionInfo *exception)
{
  KernelInfo
    *kernel,
    *new_kernel;

  char
    *kernel_cache,
    token[MagickPathExtent];

  const char
    *p;

  if (kernel_string == (const char *) NULL)
    return(ParseKernelArray(kernel_string));
  p=kernel_string;
  kernel_cache=(char *) NULL;
  if (*kernel_string == '@')
    {
      kernel_cache=FileToString(kernel_string+1,~0UL,exception);
      if (kernel_cache == (char *) NULL)
        return((KernelInfo *) NULL);
      p=(const char *) kernel_cache;
    }
  kernel=NULL;
  while (GetNextToken(p,(const char **) NULL,MagickPathExtent,token), *token != '\0')
  {
    /* ignore extra or multiple ';' kernel separators */
    if (*token != ';')
      {
        /* tokens starting with alpha is a Named kernel */
        if (isalpha((int) ((unsigned char) *token)) != 0)
          new_kernel=ParseKernelName(p,exception);
        else /* otherwise a user defined kernel array */
          new_kernel=ParseKernelArray(p);

        /* Error handling -- this is not proper error handling! */
        if (new_kernel == (KernelInfo *) NULL)
          {
            if (kernel != (KernelInfo *) NULL)
              kernel=DestroyKernelInfo(kernel);
            return((KernelInfo *) NULL);
          }

        /* initialise or append the kernel list */
        if (kernel == (KernelInfo *) NULL)
          kernel=new_kernel;
        else
          LastKernelInfo(kernel)->next=new_kernel;
      }

    /* look for the next kernel in list */
    p=strchr(p,';');
    if (p == (char *) NULL)
      break;
    p++;
  }
  if (kernel_cache != (char *) NULL)
    kernel_cache=DestroyString(kernel_cache);
  return(kernel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c q u i r e K e r n e l B u i l t I n                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireKernelBuiltIn() returned one of the 'named' built-in types of
%  kernels used for special purposes such as gaussian blurring, skeleton
%  pruning, and edge distance determination.
%
%  They take a KernelType, and a set of geometry style arguments, which were
%  typically decoded from a user supplied string, or from a more complex
%  Morphology Method that was requested.
%
%  The format of the AcquireKernalBuiltIn method is:
%
%      KernelInfo *AcquireKernelBuiltIn(const KernelInfoType type,
%           const GeometryInfo args)
%
%  A description of each parameter follows:
%
%    o type: the pre-defined type of kernel wanted
%
%    o args: arguments defining or modifying the kernel
%
%  Convolution Kernels
%
%    Unity
%       The a No-Op or Scaling single element kernel.
%
%    Gaussian:{radius},{sigma}
%       Generate a two-dimensional gaussian kernel, as used by -gaussian.
%       The sigma for the curve is required.  The resulting kernel is
%       normalized,
%
%       If 'sigma' is zero, you get a single pixel on a field of zeros.
%
%       NOTE: that the 'radius' is optional, but if provided can limit (clip)
%       the final size of the resulting kernel to a square 2*radius+1 in size.
%       The radius should be at least 2 times that of the sigma value, or
%       sever clipping and aliasing may result.  If not given or set to 0 the
%       radius will be determined so as to produce the best minimal error
%       result, which is usally much larger than is normally needed.
%
%    LoG:{radius},{sigma}
%        "Laplacian of a Gaussian" or "Mexician Hat" Kernel.
%        The supposed ideal edge detection, zero-summing kernel.
%
%        An alturnative to this kernel is to use a "DoG" with a sigma ratio of
%        approx 1.6 (according to wikipedia).
%
%    DoG:{radius},{sigma1},{sigma2}
%        "Difference of Gaussians" Kernel.
%        As "Gaussian" but with a gaussian produced by 'sigma2' subtracted
%        from the gaussian produced by 'sigma1'. Typically sigma2 > sigma1.
%        The result is a zero-summing kernel.
%
%    Blur:{radius},{sigma}[,{angle}]
%       Generates a 1 dimensional or linear gaussian blur, at the angle given
%       (current restricted to orthogonal angles).  If a 'radius' is given the
%       kernel is clipped to a width of 2*radius+1.  Kernel can be rotated
%       by a 90 degree angle.
%
%       If 'sigma' is zero, you get a single pixel on a field of zeros.
%
%       Note that two convolutions with two "Blur" kernels perpendicular to
%       each other, is equivalent to a far larger "Gaussian" kernel with the
%       same sigma value, However it is much faster to apply. This is how the
%       "-blur" operator actually works.
%
%    Comet:{width},{sigma},{angle}
%       Blur in one direction only, much like how a bright object leaves
%       a comet like trail.  The Kernel is actually half a gaussian curve,
%       Adding two such blurs in opposite directions produces a Blur Kernel.
%       Angle can be rotated in multiples of 90 degrees.
%
%       Note that the first argument is the width of the kernel and not the
%       radius of the kernel.
%
%    Binomial:[{radius}]
%       Generate a discrete kernel using a 2 dimentional Pascel's Triangle
%       of values. Used for special forma of image filters.
%
%    # Still to be implemented...
%    #
%    # Filter2D
%    # Filter1D
%    #    Set kernel values using a resize filter, and given scale (sigma)
%    #    Cylindrical or Linear.   Is this possible with an image?
%    #
%
%  Named Constant Convolution Kernels
%
%  All these are unscaled, zero-summing kernels by default. As such for
%  non-HDRI version of ImageMagick some form of normalization, user scaling,
%  and biasing the results is recommended, to prevent the resulting image
%  being 'clipped'.
%
%  The 3x3 kernels (most of these) can be circularly rotated in multiples of
%  45 degrees to generate the 8 angled varients of each of the kernels.
%
%    Laplacian:{type}
%      Discrete Lapacian Kernels, (without normalization)
%        Type 0 :  3x3 with center:8 surounded by -1  (8 neighbourhood)
%        Type 1 :  3x3 with center:4 edge:-1 corner:0 (4 neighbourhood)
%        Type 2 :  3x3 with center:4 edge:1 corner:-2
%        Type 3 :  3x3 with center:4 edge:-2 corner:1
%        Type 5 :  5x5 laplacian
%        Type 7 :  7x7 laplacian
%        Type 15 : 5x5 LoG (sigma approx 1.4)
%        Type 19 : 9x9 LoG (sigma approx 1.4)
%
%    Sobel:{angle}
%      Sobel 'Edge' convolution kernel (3x3)
%          | -1, 0, 1 |
%          | -2, 0,-2 |
%          | -1, 0, 1 |
%
%    Roberts:{angle}
%      Roberts convolution kernel (3x3)
%          |  0, 0, 0 |
%          | -1, 1, 0 |
%          |  0, 0, 0 |
%
%    Prewitt:{angle}
%      Prewitt Edge convolution kernel (3x3)
%          | -1, 0, 1 |
%          | -1, 0, 1 |
%          | -1, 0, 1 |
%
%    Compass:{angle}
%      Prewitt's "Compass" convolution kernel (3x3)
%          | -1, 1, 1 |
%          | -1,-2, 1 |
%          | -1, 1, 1 |
%
%    Kirsch:{angle}
%      Kirsch's "Compass" convolution kernel (3x3)
%          | -3,-3, 5 |
%          | -3, 0, 5 |
%          | -3,-3, 5 |
%
%    FreiChen:{angle}
%      Frei-Chen Edge Detector is based on a kernel that is similar to
%      the Sobel Kernel, but is designed to be isotropic. That is it takes
%      into account the distance of the diagonal in the kernel.
%
%          |   1,     0,   -1     |
%          | sqrt(2), 0, -sqrt(2) |
%          |   1,     0,   -1     |
%
%    FreiChen:{type},{angle}
%
%      Frei-Chen Pre-weighted kernels...
%
%        Type 0:  default un-nomalized version shown above.
%
%        Type 1: Orthogonal Kernel (same as type 11 below)
%          |   1,     0,   -1     |
%          | sqrt(2), 0, -sqrt(2) | / 2*sqrt(2)
%          |   1,     0,   -1     |
%
%        Type 2: Diagonal form of Kernel...
%          |   1,     sqrt(2),    0     |
%          | sqrt(2),   0,     -sqrt(2) | / 2*sqrt(2)
%          |   0,    -sqrt(2)    -1     |
%
%      However this kernel is als at the heart of the FreiChen Edge Detection
%      Process which uses a set of 9 specially weighted kernel.  These 9
%      kernels not be normalized, but directly applied to the image. The
%      results is then added together, to produce the intensity of an edge in
%      a specific direction.  The square root of the pixel value can then be
%      taken as the cosine of the edge, and at least 2 such runs at 90 degrees
%      from each other, both the direction and the strength of the edge can be
%      determined.
%
%        Type 10: All 9 of the following pre-weighted kernels...
%
%        Type 11: |   1,     0,   -1     |
%                 | sqrt(2), 0, -sqrt(2) | / 2*sqrt(2)
%                 |   1,     0,   -1     |
%
%        Type 12: | 1, sqrt(2), 1 |
%                 | 0,   0,     0 | / 2*sqrt(2)
%                 | 1, sqrt(2), 1 |
%
%        Type 13: | sqrt(2), -1,    0     |
%                 |  -1,      0,    1     | / 2*sqrt(2)
%                 |   0,      1, -sqrt(2) |
%
%        Type 14: |    0,     1, -sqrt(2) |
%                 |   -1,     0,     1    | / 2*sqrt(2)
%                 | sqrt(2), -1,     0    |
%
%        Type 15: | 0, -1, 0 |
%                 | 1,  0, 1 | / 2
%                 | 0, -1, 0 |
%
%        Type 16: |  1, 0, -1 |
%                 |  0, 0,  0 | / 2
%                 | -1, 0,  1 |
%
%        Type 17: |  1, -2,  1 |
%                 | -2,  4, -2 | / 6
%                 | -1, -2,  1 |
%
%        Type 18: | -2, 1, -2 |
%                 |  1, 4,  1 | / 6
%                 | -2, 1, -2 |
%
%        Type 19: | 1, 1, 1 |
%                 | 1, 1, 1 | / 3
%                 | 1, 1, 1 |
%
%      The first 4 are for edge detection, the next 4 are for line detection
%      and the last is to add a average component to the results.
%
%      Using a special type of '-1' will return all 9 pre-weighted kernels
%      as a multi-kernel list, so that you can use them directly (without
%      normalization) with the special "-set option:morphology:compose Plus"
%      setting to apply the full FreiChen Edge Detection Technique.
%
%      If 'type' is large it will be taken to be an actual rotation angle for
%      the default FreiChen (type 0) kernel.  As such  FreiChen:45  will look
%      like a  Sobel:45  but with 'sqrt(2)' instead of '2' values.
%
%      WARNING: The above was layed out as per
%          http://www.math.tau.ac.il/~turkel/notes/edge_detectors.pdf
%      But rotated 90 degrees so direction is from left rather than the top.
%      I have yet to find any secondary confirmation of the above. The only
%      other source found was actual source code at
%          http://ltswww.epfl.ch/~courstiv/exos_labos/sol3.pdf
%      Neigher paper defineds the kernels in a way that looks locical or
%      correct when taken as a whole.
%
%  Boolean Kernels
%
%    Diamond:[{radius}[,{scale}]]
%       Generate a diamond shaped kernel with given radius to the points.
%       Kernel size will again be radius*2+1 square and defaults to radius 1,
%       generating a 3x3 kernel that is slightly larger than a square.
%
%    Square:[{radius}[,{scale}]]
%       Generate a square shaped kernel of size radius*2+1, and defaulting
%       to a 3x3 (radius 1).
%
%    Octagon:[{radius}[,{scale}]]
%       Generate octagonal shaped kernel of given radius and constant scale.
%       Default radius is 3 producing a 7x7 kernel. A radius of 1 will result
%       in "Diamond" kernel.
%
%    Disk:[{radius}[,{scale}]]
%       Generate a binary disk, thresholded at the radius given, the radius
%       may be a float-point value. Final Kernel size is floor(radius)*2+1
%       square. A radius of 5.3 is the default.
%
%       NOTE: That a low radii Disk kernels produce the same results as
%       many of the previously defined kernels, but differ greatly at larger
%       radii.  Here is a table of equivalences...
%          "Disk:1"    => "Diamond", "Octagon:1", or "Cross:1"
%          "Disk:1.5"  => "Square"
%          "Disk:2"    => "Diamond:2"
%          "Disk:2.5"  => "Octagon"
%          "Disk:2.9"  => "Square:2"
%          "Disk:3.5"  => "Octagon:3"
%          "Disk:4.5"  => "Octagon:4"
%          "Disk:5.4"  => "Octagon:5"
%          "Disk:6.4"  => "Octagon:6"
%       All other Disk shapes are unique to this kernel, but because a "Disk"
%       is more circular when using a larger radius, using a larger radius is
%       preferred over iterating the morphological operation.
%
%    Rectangle:{geometry}
%       Simply generate a rectangle of 1's with the size given. You can also
%       specify the location of the 'control point', otherwise the closest
%       pixel to the center of the rectangle is selected.
%
%       Properly centered and odd sized rectangles work the best.
%
%  Symbol Dilation Kernels
%
%    These kernel is not a good general morphological kernel, but is used
%    more for highlighting and marking any single pixels in an image using,
%    a "Dilate" method as appropriate.
%
%    For the same reasons iterating these kernels does not produce the
%    same result as using a larger radius for the symbol.
%
%    Plus:[{radius}[,{scale}]]
%    Cross:[{radius}[,{scale}]]
%       Generate a kernel in the shape of a 'plus' or a 'cross' with
%       a each arm the length of the given radius (default 2).
%
%       NOTE: "plus:1" is equivalent to a "Diamond" kernel.
%
%    Ring:{radius1},{radius2}[,{scale}]
%       A ring of the values given that falls between the two radii.
%       Defaults to a ring of approximataly 3 radius in a 7x7 kernel.
%       This is the 'edge' pixels of the default "Disk" kernel,
%       More specifically, "Ring" -> "Ring:2.5,3.5,1.0"
%
%  Hit and Miss Kernels
%
%    Peak:radius1,radius2
%       Find any peak larger than the pixels the fall between the two radii.
%       The default ring of pixels is as per "Ring".
%    Edges
%       Find flat orthogonal edges of a binary shape
%    Corners
%       Find 90 degree corners of a binary shape
%    Diagonals:type
%       A special kernel to thin the 'outside' of diagonals
%    LineEnds:type
%       Find end points of lines (for pruning a skeletion)
%       Two types of lines ends (default to both) can be searched for
%         Type 0: All line ends
%         Type 1: single kernel for 4-conneected line ends
%         Type 2: single kernel for simple line ends
%    LineJunctions
%       Find three line junctions (within a skeletion)
%         Type 0: all line junctions
%         Type 1: Y Junction kernel
%         Type 2: Diagonal T Junction kernel
%         Type 3: Orthogonal T Junction kernel
%         Type 4: Diagonal X Junction kernel
%         Type 5: Orthogonal + Junction kernel
%    Ridges:type
%       Find single pixel ridges or thin lines
%         Type 1: Fine single pixel thick lines and ridges
%         Type 2: Find two pixel thick lines and ridges
%    ConvexHull
%       Octagonal Thickening Kernel, to generate convex hulls of 45 degrees
%    Skeleton:type
%       Traditional skeleton generating kernels.
%         Type 1: Tradional Skeleton kernel (4 connected skeleton)
%         Type 2: HIPR2 Skeleton kernel (8 connected skeleton)
%         Type 3: Thinning skeleton based on a ressearch paper by
%                 Dan S. Bloomberg (Default Type)
%    ThinSE:type
%       A huge variety of Thinning Kernels designed to preserve conectivity.
%       many other kernel sets use these kernels as source definitions.
%       Type numbers are 41-49, 81-89, 481, and 482 which are based on
%       the super and sub notations used in the source research paper.
%
%  Distance Measuring Kernels
%
%    Different types of distance measuring methods, which are used with the
%    a 'Distance' morphology method for generating a gradient based on
%    distance from an edge of a binary shape, though there is a technique
%    for handling a anti-aliased shape.
%
%    See the 'Distance' Morphological Method, for information of how it is
%    applied.
%
%    Chebyshev:[{radius}][x{scale}[%!]]
%       Chebyshev Distance (also known as Tchebychev or Chessboard distance)
%       is a value of one to any neighbour, orthogonal or diagonal. One why
%       of thinking of it is the number of squares a 'King' or 'Queen' in
%       chess needs to traverse reach any other position on a chess board.
%       It results in a 'square' like distance function, but one where
%       diagonals are given a value that is closer than expected.
%
%    Manhattan:[{radius}][x{scale}[%!]]
%       Manhattan Distance (also known as Rectilinear, City Block, or the Taxi
%       Cab distance metric), it is the distance needed when you can only
%       travel in horizontal or vertical directions only.  It is the
%       distance a 'Rook' in chess would have to travel, and results in a
%       diamond like distances, where diagonals are further than expected.
%
%    Octagonal:[{radius}][x{scale}[%!]]
%       An interleving of Manhatten and Chebyshev metrics producing an
%       increasing octagonally shaped distance.  Distances matches those of
%       the "Octagon" shaped kernel of the same radius.  The minimum radius
%       and default is 2, producing a 5x5 kernel.
%
%    Euclidean:[{radius}][x{scale}[%!]]
%       Euclidean distance is the 'direct' or 'as the crow flys' distance.
%       However by default the kernel size only has a radius of 1, which
%       limits the distance to 'Knight' like moves, with only orthogonal and
%       diagonal measurements being correct.  As such for the default kernel
%       you will get octagonal like distance function.
%
%       However using a larger radius such as "Euclidean:4" you will get a
%       much smoother distance gradient from the edge of the shape. Especially
%       if the image is pre-processed to include any anti-aliasing pixels.
%       Of course a larger kernel is slower to use, and not always needed.
%
%    The first three Distance Measuring Kernels will only generate distances
%    of exact multiples of {scale} in binary images. As such you can use a
%    scale of 1 without loosing any information.  However you also need some
%    scaling when handling non-binary anti-aliased shapes.
%
%    The "Euclidean" Distance Kernel however does generate a non-integer
%    fractional results, and as such scaling is vital even for binary shapes.
%
*/

MagickExport KernelInfo *AcquireKernelBuiltIn(const KernelInfoType type,
  const GeometryInfo *args,ExceptionInfo *exception)
{
  KernelInfo
    *kernel;

  register ssize_t
    i;

  register ssize_t
    u,
    v;

  double
    nan = sqrt((double)-1.0);  /* Special Value : Not A Number */

  /* Generate a new empty kernel if needed */
  kernel=(KernelInfo *) NULL;
  switch(type) {
    case UndefinedKernel:    /* These should not call this function */
    case UserDefinedKernel:
      assert("Should not call this function" != (char *) NULL);
      break;
    case LaplacianKernel:   /* Named Descrete Convolution Kernels */
    case SobelKernel:       /* these are defined using other kernels */
    case RobertsKernel:
    case PrewittKernel:
    case CompassKernel:
    case KirschKernel:
    case FreiChenKernel:
    case EdgesKernel:       /* Hit and Miss kernels */
    case CornersKernel:
    case DiagonalsKernel:
    case LineEndsKernel:
    case LineJunctionsKernel:
    case RidgesKernel:
    case ConvexHullKernel:
    case SkeletonKernel:
    case ThinSEKernel:
      break;               /* A pre-generated kernel is not needed */
#if 0
    /* set to 1 to do a compile-time check that we haven't missed anything */
    case UnityKernel:
    case GaussianKernel:
    case DoGKernel:
    case LoGKernel:
    case BlurKernel:
    case CometKernel:
    case BinomialKernel:
    case DiamondKernel:
    case SquareKernel:
    case RectangleKernel:
    case OctagonKernel:
    case DiskKernel:
    case PlusKernel:
    case CrossKernel:
    case RingKernel:
    case PeaksKernel:
    case ChebyshevKernel:
    case ManhattanKernel:
    case OctangonalKernel:
    case EuclideanKernel:
#else
    default:
#endif
      /* Generate the base Kernel Structure */
      kernel=(KernelInfo *) AcquireMagickMemory(sizeof(*kernel));
      if (kernel == (KernelInfo *) NULL)
        return(kernel);
      (void) memset(kernel,0,sizeof(*kernel));
      kernel->minimum = kernel->maximum = kernel->angle = 0.0;
      kernel->negative_range = kernel->positive_range = 0.0;
      kernel->type = type;
      kernel->next = (KernelInfo *) NULL;
      kernel->signature=MagickCoreSignature;
      break;
  }

  switch(type) {
    /*
      Convolution Kernels
    */
    case UnityKernel:
      {
        kernel->height = kernel->width = (size_t) 1;
        kernel->x = kernel->y = (ssize_t) 0;
        kernel->values=(MagickRealType *) MagickAssumeAligned(
          AcquireAlignedMemory(1,sizeof(*kernel->values)));
        if (kernel->values == (MagickRealType *) NULL)
          return(DestroyKernelInfo(kernel));
        kernel->maximum = kernel->values[0] = args->rho;
        break;
      }
      break;
    case GaussianKernel:
    case DoGKernel:
    case LoGKernel:
      { double
          sigma = fabs(args->sigma),
          sigma2 = fabs(args->xi),
          A, B, R;

        if ( args->rho >= 1.0 )
          kernel->width = (size_t)args->rho*2+1;
        else if ( (type != DoGKernel) || (sigma >= sigma2) )
          kernel->width = GetOptimalKernelWidth2D(args->rho,sigma);
        else
          kernel->width = GetOptimalKernelWidth2D(args->rho,sigma2);
        kernel->height = kernel->width;
        kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;
        kernel->values=(MagickRealType *) MagickAssumeAligned(
          AcquireAlignedMemory(kernel->width,kernel->height*
          sizeof(*kernel->values)));
        if (kernel->values == (MagickRealType *) NULL)
          return(DestroyKernelInfo(kernel));

        /* WARNING: The following generates a 'sampled gaussian' kernel.
         * What we really want is a 'discrete gaussian' kernel.
         *
         * How to do this is I don't know, but appears to be basied on the
         * Error Function 'erf()' (intergral of a gaussian)
         */

        if ( type == GaussianKernel || type == DoGKernel )
          { /* Calculate a Gaussian,  OR positive half of a DoG */
            if ( sigma > MagickEpsilon )
              { A = 1.0/(2.0*sigma*sigma);  /* simplify loop expressions */
                B = (double) (1.0/(Magick2PI*sigma*sigma));
                for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
                  for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
                      kernel->values[i] = exp(-((double)(u*u+v*v))*A)*B;
              }
            else /* limiting case - a unity (normalized Dirac) kernel */
              { (void) memset(kernel->values,0, (size_t)
                  kernel->width*kernel->height*sizeof(*kernel->values));
                kernel->values[kernel->x+kernel->y*kernel->width] = 1.0;
              }
          }

        if ( type == DoGKernel )
          { /* Subtract a Negative Gaussian for "Difference of Gaussian" */
            if ( sigma2 > MagickEpsilon )
              { sigma = sigma2;                /* simplify loop expressions */
                A = 1.0/(2.0*sigma*sigma);
                B = (double) (1.0/(Magick2PI*sigma*sigma));
                for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
                  for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
                    kernel->values[i] -= exp(-((double)(u*u+v*v))*A)*B;
              }
            else /* limiting case - a unity (normalized Dirac) kernel */
              kernel->values[kernel->x+kernel->y*kernel->width] -= 1.0;
          }

        if ( type == LoGKernel )
          { /* Calculate a Laplacian of a Gaussian - Or Mexician Hat */
            if ( sigma > MagickEpsilon )
              { A = 1.0/(2.0*sigma*sigma);  /* simplify loop expressions */
                B = (double) (1.0/(MagickPI*sigma*sigma*sigma*sigma));
                for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
                  for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
                    { R = ((double)(u*u+v*v))*A;
                      kernel->values[i] = (1-R)*exp(-R)*B;
                    }
              }
            else /* special case - generate a unity kernel */
              { (void) memset(kernel->values,0, (size_t)
                  kernel->width*kernel->height*sizeof(*kernel->values));
                kernel->values[kernel->x+kernel->y*kernel->width] = 1.0;
              }
          }

        /* Note the above kernels may have been 'clipped' by a user defined
        ** radius, producing a smaller (darker) kernel.  Also for very small
        ** sigma's (> 0.1) the central value becomes larger than one, and thus
        ** producing a very bright kernel.
        **
        ** Normalization will still be needed.
        */

        /* Normalize the 2D Gaussian Kernel
        **
        ** NB: a CorrelateNormalize performs a normal Normalize if
        ** there are no negative values.
        */
        CalcKernelMetaData(kernel);  /* the other kernel meta-data */
        ScaleKernelInfo(kernel, 1.0, CorrelateNormalizeValue);

        break;
      }
    case BlurKernel:
      { double
          sigma = fabs(args->sigma),
          alpha, beta;

        if ( args->rho >= 1.0 )
          kernel->width = (size_t)args->rho*2+1;
        else
          kernel->width = GetOptimalKernelWidth1D(args->rho,sigma);
        kernel->height = 1;
        kernel->x = (ssize_t) (kernel->width-1)/2;
        kernel->y = 0;
        kernel->negative_range = kernel->positive_range = 0.0;
        kernel->values=(MagickRealType *) MagickAssumeAligned(
          AcquireAlignedMemory(kernel->width,kernel->height*
          sizeof(*kernel->values)));
        if (kernel->values == (MagickRealType *) NULL)
          return(DestroyKernelInfo(kernel));

#if 1
#define KernelRank 3
        /* Formula derived from GetBlurKernel() in "effect.c" (plus bug fix).
        ** It generates a gaussian 3 times the width, and compresses it into
        ** the expected range.  This produces a closer normalization of the
        ** resulting kernel, especially for very low sigma values.
        ** As such while wierd it is prefered.
        **
        ** I am told this method originally came from Photoshop.
        **
        ** A properly normalized curve is generated (apart from edge clipping)
        ** even though we later normalize the result (for edge clipping)
        ** to allow the correct generation of a "Difference of Blurs".
        */

        /* initialize */
        v = (ssize_t) (kernel->width*KernelRank-1)/2; /* start/end points to fit range */
        (void) memset(kernel->values,0, (size_t)
          kernel->width*kernel->height*sizeof(*kernel->values));
        /* Calculate a Positive 1D Gaussian */
        if ( sigma > MagickEpsilon )
          { sigma *= KernelRank;               /* simplify loop expressions */
            alpha = 1.0/(2.0*sigma*sigma);
            beta= (double) (1.0/(MagickSQ2PI*sigma ));
            for ( u=-v; u <= v; u++) {
              kernel->values[(u+v)/KernelRank] +=
                              exp(-((double)(u*u))*alpha)*beta;
            }
          }
        else /* special case - generate a unity kernel */
          kernel->values[kernel->x+kernel->y*kernel->width] = 1.0;
#else
        /* Direct calculation without curve averaging
           This is equivelent to a KernelRank of 1 */

        /* Calculate a Positive Gaussian */
        if ( sigma > MagickEpsilon )
          { alpha = 1.0/(2.0*sigma*sigma);    /* simplify loop expressions */
            beta = 1.0/(MagickSQ2PI*sigma);
            for ( i=0, u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
              kernel->values[i] = exp(-((double)(u*u))*alpha)*beta;
          }
        else /* special case - generate a unity kernel */
          { (void) memset(kernel->values,0, (size_t)
              kernel->width*kernel->height*sizeof(*kernel->values));
            kernel->values[kernel->x+kernel->y*kernel->width] = 1.0;
          }
#endif
        /* Note the above kernel may have been 'clipped' by a user defined
        ** radius, producing a smaller (darker) kernel.  Also for very small
        ** sigma's (> 0.1) the central value becomes larger than one, as a
        ** result of not generating a actual 'discrete' kernel, and thus
        ** producing a very bright 'impulse'.
        **
        ** Becuase of these two factors Normalization is required!
        */

        /* Normalize the 1D Gaussian Kernel
        **
        ** NB: a CorrelateNormalize performs a normal Normalize if
        ** there are no negative values.
        */
        CalcKernelMetaData(kernel);  /* the other kernel meta-data */
        ScaleKernelInfo(kernel, 1.0, CorrelateNormalizeValue);

        /* rotate the 1D kernel by given angle */
        RotateKernelInfo(kernel, args->xi );
        break;
      }
    case CometKernel:
      { double
          sigma = fabs(args->sigma),
          A;

        if ( args->rho < 1.0 )
          kernel->width = (GetOptimalKernelWidth1D(args->rho,sigma)-1)/2+1;
        else
          kernel->width = (size_t)args->rho;
        kernel->x = kernel->y = 0;
        kernel->height = 1;
        kernel->negative_range = kernel->positive_range = 0.0;
        kernel->values=(MagickRealType *) MagickAssumeAligned(
          AcquireAlignedMemory(kernel->width,kernel->height*
          sizeof(*kernel->values)));
        if (kernel->values == (MagickRealType *) NULL)
          return(DestroyKernelInfo(kernel));

        /* A comet blur is half a 1D gaussian curve, so that the object is
        ** blurred in one direction only.  This may not be quite the right
        ** curve to use so may change in the future. The function must be
        ** normalised after generation, which also resolves any clipping.
        **
        ** As we are normalizing and not subtracting gaussians,
        ** there is no need for a divisor in the gaussian formula
        **
        ** It is less comples
        */
        if ( sigma > MagickEpsilon )
          {
#if 1
#define KernelRank 3
            v = (ssize_t) kernel->width*KernelRank; /* start/end points */
            (void) memset(kernel->values,0, (size_t)
              kernel->width*sizeof(*kernel->values));
            sigma *= KernelRank;            /* simplify the loop expression */
            A = 1.0/(2.0*sigma*sigma);
            /* B = 1.0/(MagickSQ2PI*sigma); */
            for ( u=0; u < v; u++) {
              kernel->values[u/KernelRank] +=
                  exp(-((double)(u*u))*A);
              /*  exp(-((double)(i*i))/2.0*sigma*sigma)/(MagickSQ2PI*sigma); */
            }
            for (i=0; i < (ssize_t) kernel->width; i++)
              kernel->positive_range += kernel->values[i];
#else
            A = 1.0/(2.0*sigma*sigma);     /* simplify the loop expression */
            /* B = 1.0/(MagickSQ2PI*sigma); */
            for ( i=0; i < (ssize_t) kernel->width; i++)
              kernel->positive_range +=
                kernel->values[i] = exp(-((double)(i*i))*A);
                /* exp(-((double)(i*i))/2.0*sigma*sigma)/(MagickSQ2PI*sigma); */
#endif
          }
        else /* special case - generate a unity kernel */
          { (void) memset(kernel->values,0, (size_t)
              kernel->width*kernel->height*sizeof(*kernel->values));
            kernel->values[kernel->x+kernel->y*kernel->width] = 1.0;
            kernel->positive_range = 1.0;
          }

        kernel->minimum = 0.0;
        kernel->maximum = kernel->values[0];
        kernel->negative_range = 0.0;

        ScaleKernelInfo(kernel, 1.0, NormalizeValue); /* Normalize */
        RotateKernelInfo(kernel, args->xi); /* Rotate by angle */
        break;
      }
    case BinomialKernel:
      {
        size_t
          order_f;

        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
          kernel->width = kernel->height = ((size_t)args->rho)*2+1;
        kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;

        order_f = fact(kernel->width-1);

        kernel->values=(MagickRealType *) MagickAssumeAligned(
          AcquireAlignedMemory(kernel->width,kernel->height*
          sizeof(*kernel->values)));
        if (kernel->values == (MagickRealType *) NULL)
          return(DestroyKernelInfo(kernel));

        /* set all kernel values within diamond area to scale given */
        for ( i=0, v=0; v < (ssize_t)kernel->height; v++)
          { size_t
              alpha = order_f / ( fact((size_t) v) * fact(kernel->height-v-1) );
            for ( u=0; u < (ssize_t)kernel->width; u++, i++)
              kernel->positive_range += kernel->values[i] = (double)
                (alpha * order_f / ( fact((size_t) u) * fact(kernel->height-u-1) ));
          }
        kernel->minimum = 1.0;
        kernel->maximum = kernel->values[kernel->x+kernel->y*kernel->width];
        kernel->negative_range = 0.0;
        break;
      }

    /*
      Convolution Kernels - Well Known Named Constant Kernels
    */
    case LaplacianKernel:
      { switch ( (int) args->rho ) {
          case 0:
          default: /* laplacian square filter -- default */
            kernel=ParseKernelArray("3: -1,-1,-1  -1,8,-1  -1,-1,-1");
            break;
          case 1:  /* laplacian diamond filter */
            kernel=ParseKernelArray("3: 0,-1,0  -1,4,-1  0,-1,0");
            break;
          case 2:
            kernel=ParseKernelArray("3: -2,1,-2  1,4,1  -2,1,-2");
            break;
          case 3:
            kernel=ParseKernelArray("3: 1,-2,1  -2,4,-2  1,-2,1");
            break;
          case 5:   /* a 5x5 laplacian */
            kernel=ParseKernelArray(
              "5: -4,-1,0,-1,-4  -1,2,3,2,-1  0,3,4,3,0  -1,2,3,2,-1  -4,-1,0,-1,-4");
            break;
          case 7:   /* a 7x7 laplacian */
            kernel=ParseKernelArray(
              "7:-10,-5,-2,-1,-2,-5,-10 -5,0,3,4,3,0,-5 -2,3,6,7,6,3,-2 -1,4,7,8,7,4,-1 -2,3,6,7,6,3,-2 -5,0,3,4,3,0,-5 -10,-5,-2,-1,-2,-5,-10" );
            break;
          case 15:  /* a 5x5 LoG (sigma approx 1.4) */
            kernel=ParseKernelArray(
              "5: 0,0,-1,0,0  0,-1,-2,-1,0  -1,-2,16,-2,-1  0,-1,-2,-1,0  0,0,-1,0,0");
            break;
          case 19:  /* a 9x9 LoG (sigma approx 1.4) */
            /* http://www.cscjournals.org/csc/manuscript/Journals/IJIP/volume3/Issue1/IJIP-15.pdf */
            kernel=ParseKernelArray(
              "9: 0,-1,-1,-2,-2,-2,-1,-1,0  -1,-2,-4,-5,-5,-5,-4,-2,-1  -1,-4,-5,-3,-0,-3,-5,-4,-1  -2,-5,-3,12,24,12,-3,-5,-2  -2,-5,-0,24,40,24,-0,-5,-2  -2,-5,-3,12,24,12,-3,-5,-2  -1,-4,-5,-3,-0,-3,-5,-4,-1  -1,-2,-4,-5,-5,-5,-4,-2,-1  0,-1,-1,-2,-2,-2,-1,-1,0");
            break;
        }
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        break;
      }
    case SobelKernel:
      { /* Simple Sobel Kernel */
        kernel=ParseKernelArray("3: 1,0,-1  2,0,-2  1,0,-1");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        RotateKernelInfo(kernel, args->rho);
        break;
      }
    case RobertsKernel:
      {
        kernel=ParseKernelArray("3: 0,0,0  1,-1,0  0,0,0");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        RotateKernelInfo(kernel, args->rho);
        break;
      }
    case PrewittKernel:
      {
        kernel=ParseKernelArray("3: 1,0,-1  1,0,-1  1,0,-1");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        RotateKernelInfo(kernel, args->rho);
        break;
      }
    case CompassKernel:
      {
        kernel=ParseKernelArray("3: 1,1,-1  1,-2,-1  1,1,-1");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        RotateKernelInfo(kernel, args->rho);
        break;
      }
    case KirschKernel:
      {
        kernel=ParseKernelArray("3: 5,-3,-3  5,0,-3  5,-3,-3");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        RotateKernelInfo(kernel, args->rho);
        break;
      }
    case FreiChenKernel:
      /* Direction is set to be left to right positive */
      /* http://www.math.tau.ac.il/~turkel/notes/edge_detectors.pdf -- RIGHT? */
      /* http://ltswww.epfl.ch/~courstiv/exos_labos/sol3.pdf -- WRONG? */
      { switch ( (int) args->rho ) {
          default:
          case 0:
            kernel=ParseKernelArray("3: 1,0,-1  2,0,-2  1,0,-1");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            kernel->values[3] = +(MagickRealType) MagickSQ2;
            kernel->values[5] = -(MagickRealType) MagickSQ2;
            CalcKernelMetaData(kernel);     /* recalculate meta-data */
            break;
          case 2:
            kernel=ParseKernelArray("3: 1,2,0  2,0,-2  0,-2,-1");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            kernel->values[1] = kernel->values[3]= +(MagickRealType) MagickSQ2;
            kernel->values[5] = kernel->values[7]= -(MagickRealType) MagickSQ2;
            CalcKernelMetaData(kernel);     /* recalculate meta-data */
            ScaleKernelInfo(kernel, (double) (1.0/2.0*MagickSQ2), NoValue);
            break;
          case 10:
          {
            kernel=AcquireKernelInfo("FreiChen:11;FreiChen:12;FreiChen:13;FreiChen:14;FreiChen:15;FreiChen:16;FreiChen:17;FreiChen:18;FreiChen:19",exception);
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            break;
          }
          case 1:
          case 11:
            kernel=ParseKernelArray("3: 1,0,-1  2,0,-2  1,0,-1");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            kernel->values[3] = +(MagickRealType) MagickSQ2;
            kernel->values[5] = -(MagickRealType) MagickSQ2;
            CalcKernelMetaData(kernel);     /* recalculate meta-data */
            ScaleKernelInfo(kernel, (double) (1.0/2.0*MagickSQ2), NoValue);
            break;
          case 12:
            kernel=ParseKernelArray("3: 1,2,1  0,0,0  1,2,1");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            kernel->values[1] = +(MagickRealType) MagickSQ2;
            kernel->values[7] = +(MagickRealType) MagickSQ2;
            CalcKernelMetaData(kernel);
            ScaleKernelInfo(kernel, (double) (1.0/2.0*MagickSQ2), NoValue);
            break;
          case 13:
            kernel=ParseKernelArray("3: 2,-1,0  -1,0,1  0,1,-2");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            kernel->values[0] = +(MagickRealType) MagickSQ2;
            kernel->values[8] = -(MagickRealType) MagickSQ2;
            CalcKernelMetaData(kernel);
            ScaleKernelInfo(kernel, (double) (1.0/2.0*MagickSQ2), NoValue);
            break;
          case 14:
            kernel=ParseKernelArray("3: 0,1,-2  -1,0,1  2,-1,0");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            kernel->values[2] = -(MagickRealType) MagickSQ2;
            kernel->values[6] = +(MagickRealType) MagickSQ2;
            CalcKernelMetaData(kernel);
            ScaleKernelInfo(kernel, (double) (1.0/2.0*MagickSQ2), NoValue);
            break;
          case 15:
            kernel=ParseKernelArray("3: 0,-1,0  1,0,1  0,-1,0");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            ScaleKernelInfo(kernel, 1.0/2.0, NoValue);
            break;
          case 16:
            kernel=ParseKernelArray("3: 1,0,-1  0,0,0  -1,0,1");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            ScaleKernelInfo(kernel, 1.0/2.0, NoValue);
            break;
          case 17:
            kernel=ParseKernelArray("3: 1,-2,1  -2,4,-2  -1,-2,1");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            ScaleKernelInfo(kernel, 1.0/6.0, NoValue);
            break;
          case 18:
            kernel=ParseKernelArray("3: -2,1,-2  1,4,1  -2,1,-2");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            ScaleKernelInfo(kernel, 1.0/6.0, NoValue);
            break;
          case 19:
            kernel=ParseKernelArray("3: 1,1,1  1,1,1  1,1,1");
            if (kernel == (KernelInfo *) NULL)
              return(kernel);
            kernel->type = type;
            ScaleKernelInfo(kernel, 1.0/3.0, NoValue);
            break;
        }
        if ( fabs(args->sigma) >= MagickEpsilon )
          /* Rotate by correctly supplied 'angle' */
          RotateKernelInfo(kernel, args->sigma);
        else if ( args->rho > 30.0 || args->rho < -30.0 )
          /* Rotate by out of bounds 'type' */
          RotateKernelInfo(kernel, args->rho);
        break;
      }

    /*
      Boolean or Shaped Kernels
    */
    case DiamondKernel:
      {
        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
          kernel->width = kernel->height = ((size_t)args->rho)*2+1;
        kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;

        kernel->values=(MagickRealType *) MagickAssumeAligned(
          AcquireAlignedMemory(kernel->width,kernel->height*
          sizeof(*kernel->values)));
        if (kernel->values == (MagickRealType *) NULL)
          return(DestroyKernelInfo(kernel));

        /* set all kernel values within diamond area to scale given */
        for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
          for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
            if ( (labs((long) u)+labs((long) v)) <= (long) kernel->x)
              kernel->positive_range += kernel->values[i] = args->sigma;
            else
              kernel->values[i] = nan;
        kernel->minimum = kernel->maximum = args->sigma;   /* a flat shape */
        break;
      }
    case SquareKernel:
    case RectangleKernel:
      { double
          scale;
        if ( type == SquareKernel )
          {
            if (args->rho < 1.0)
              kernel->width = kernel->height = 3;  /* default radius = 1 */
            else
              kernel->width = kernel->height = (size_t) (2*args->rho+1);
            kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;
            scale = args->sigma;
          }
        else {
            /* NOTE: user defaults set in "AcquireKernelInfo()" */
            if ( args->rho < 1.0 || args->sigma < 1.0 )
              return(DestroyKernelInfo(kernel));    /* invalid args given */
            kernel->width = (size_t)args->rho;
            kernel->height = (size_t)args->sigma;
            if ( args->xi  < 0.0 || args->xi  > (double)kernel->width ||
                 args->psi < 0.0 || args->psi > (double)kernel->height )
              return(DestroyKernelInfo(kernel));    /* invalid args given */
            kernel->x = (ssize_t) args->xi;
            kernel->y = (ssize_t) args->psi;
            scale = 1.0;
          }
        kernel->values=(MagickRealType *) MagickAssumeAligned(
          AcquireAlignedMemory(kernel->width,kernel->height*
          sizeof(*kernel->values)));
        if (kernel->values == (MagickRealType *) NULL)
          return(DestroyKernelInfo(kernel));

        /* set all kernel values to scale given */
        u=(ssize_t) (kernel->width*kernel->height);
        for ( i=0; i < u; i++)
            kernel->values[i] = scale;
        kernel->minimum = kernel->maximum = scale;   /* a flat shape */
        kernel->positive_range = scale*u;
        break;
      }
      case OctagonKernel:
        {
          if (args->rho < 1.0)
            kernel->width = kernel->height = 5;  /* default radius = 2 */
          else
            kernel->width = kernel->height = ((size_t)args->rho)*2+1;
          kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;

          kernel->values=(MagickRealType *) MagickAssumeAligned(
            AcquireAlignedMemory(kernel->width,kernel->height*
            sizeof(*kernel->values)));
          if (kernel->values == (MagickRealType *) NULL)
            return(DestroyKernelInfo(kernel));

          for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
            for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
              if ( (labs((long) u)+labs((long) v)) <=
                        ((long)kernel->x + (long)(kernel->x/2)) )
                kernel->positive_range += kernel->values[i] = args->sigma;
              else
                kernel->values[i] = nan;
          kernel->minimum = kernel->maximum = args->sigma;  /* a flat shape */
          break;
        }
      case DiskKernel:
        {
          ssize_t
            limit = (ssize_t)(args->rho*args->rho);

          if (args->rho < 0.4)           /* default radius approx 4.3 */
            kernel->width = kernel->height = 9L, limit = 18L;
          else
            kernel->width = kernel->height = (size_t)fabs(args->rho)*2+1;
          kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;

          kernel->values=(MagickRealType *) MagickAssumeAligned(
            AcquireAlignedMemory(kernel->width,kernel->height*
            sizeof(*kernel->values)));
          if (kernel->values == (MagickRealType *) NULL)
            return(DestroyKernelInfo(kernel));

          for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
            for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
              if ((u*u+v*v) <= limit)
                kernel->positive_range += kernel->values[i] = args->sigma;
              else
                kernel->values[i] = nan;
          kernel->minimum = kernel->maximum = args->sigma;   /* a flat shape */
          break;
        }
      case PlusKernel:
        {
          if (args->rho < 1.0)
            kernel->width = kernel->height = 5;  /* default radius 2 */
          else
            kernel->width = kernel->height = ((size_t)args->rho)*2+1;
          kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;

          kernel->values=(MagickRealType *) MagickAssumeAligned(
            AcquireAlignedMemory(kernel->width,kernel->height*
            sizeof(*kernel->values)));
          if (kernel->values == (MagickRealType *) NULL)
            return(DestroyKernelInfo(kernel));

          /* set all kernel values along axises to given scale */
          for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
            for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
              kernel->values[i] = (u == 0 || v == 0) ? args->sigma : nan;
          kernel->minimum = kernel->maximum = args->sigma;   /* a flat shape */
          kernel->positive_range = args->sigma*(kernel->width*2.0 - 1.0);
          break;
        }
      case CrossKernel:
        {
          if (args->rho < 1.0)
            kernel->width = kernel->height = 5;  /* default radius 2 */
          else
            kernel->width = kernel->height = ((size_t)args->rho)*2+1;
          kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;

          kernel->values=(MagickRealType *) MagickAssumeAligned(
            AcquireAlignedMemory(kernel->width,kernel->height*
            sizeof(*kernel->values)));
          if (kernel->values == (MagickRealType *) NULL)
            return(DestroyKernelInfo(kernel));

          /* set all kernel values along axises to given scale */
          for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
            for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
              kernel->values[i] = (u == v || u == -v) ? args->sigma : nan;
          kernel->minimum = kernel->maximum = args->sigma;   /* a flat shape */
          kernel->positive_range = args->sigma*(kernel->width*2.0 - 1.0);
          break;
        }
      /*
        HitAndMiss Kernels
      */
      case RingKernel:
      case PeaksKernel:
        {
          ssize_t
            limit1,
            limit2,
            scale;

          if (args->rho < args->sigma)
            {
              kernel->width = ((size_t)args->sigma)*2+1;
              limit1 = (ssize_t)(args->rho*args->rho);
              limit2 = (ssize_t)(args->sigma*args->sigma);
            }
          else
            {
              kernel->width = ((size_t)args->rho)*2+1;
              limit1 = (ssize_t)(args->sigma*args->sigma);
              limit2 = (ssize_t)(args->rho*args->rho);
            }
          if ( limit2 <= 0 )
            kernel->width = 7L, limit1 = 7L, limit2 = 11L;

          kernel->height = kernel->width;
          kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;
          kernel->values=(MagickRealType *) MagickAssumeAligned(
            AcquireAlignedMemory(kernel->width,kernel->height*
            sizeof(*kernel->values)));
          if (kernel->values == (MagickRealType *) NULL)
            return(DestroyKernelInfo(kernel));

          /* set a ring of points of 'scale' ( 0.0 for PeaksKernel ) */
          scale = (ssize_t) (( type == PeaksKernel) ? 0.0 : args->xi);
          for ( i=0, v= -kernel->y; v <= (ssize_t)kernel->y; v++)
            for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
              { ssize_t radius=u*u+v*v;
                if (limit1 < radius && radius <= limit2)
                  kernel->positive_range += kernel->values[i] = (double) scale;
                else
                  kernel->values[i] = nan;
              }
          kernel->minimum = kernel->maximum = (double) scale;
          if ( type == PeaksKernel ) {
            /* set the central point in the middle */
            kernel->values[kernel->x+kernel->y*kernel->width] = 1.0;
            kernel->positive_range = 1.0;
            kernel->maximum = 1.0;
          }
          break;
        }
      case EdgesKernel:
        {
          kernel=AcquireKernelInfo("ThinSE:482",exception);
          if (kernel == (KernelInfo *) NULL)
            return(kernel);
          kernel->type = type;
          ExpandMirrorKernelInfo(kernel); /* mirror expansion of kernels */
          break;
        }
      case CornersKernel:
        {
          kernel=AcquireKernelInfo("ThinSE:87",exception);
          if (kernel == (KernelInfo *) NULL)
            return(kernel);
          kernel->type = type;
          ExpandRotateKernelInfo(kernel, 90.0); /* Expand 90 degree rotations */
          break;
        }
      case DiagonalsKernel:
        {
          switch ( (int) args->rho ) {
            case 0:
            default:
              { KernelInfo
                  *new_kernel;
                kernel=ParseKernelArray("3: 0,0,0  0,-,1  1,1,-");
                if (kernel == (KernelInfo *) NULL)
                  return(kernel);
                kernel->type = type;
                new_kernel=ParseKernelArray("3: 0,0,1  0,-,1  0,1,-");
                if (new_kernel == (KernelInfo *) NULL)
                  return(DestroyKernelInfo(kernel));
                new_kernel->type = type;
                LastKernelInfo(kernel)->next = new_kernel;
                ExpandMirrorKernelInfo(kernel);
                return(kernel);
              }
            case 1:
              kernel=ParseKernelArray("3: 0,0,0  0,-,1  1,1,-");
              break;
            case 2:
              kernel=ParseKernelArray("3: 0,0,1  0,-,1  0,1,-");
              break;
          }
          if (kernel == (KernelInfo *) NULL)
            return(kernel);
          kernel->type = type;
          RotateKernelInfo(kernel, args->sigma);
          break;
        }
      case LineEndsKernel:
        { /* Kernels for finding the end of thin lines */
          switch ( (int) args->rho ) {
            case 0:
            default:
              /* set of kernels to find all end of lines */
              return(AcquireKernelInfo("LineEnds:1>;LineEnds:2>",exception));
            case 1:
              /* kernel for 4-connected line ends - no rotation */
              kernel=ParseKernelArray("3: 0,0,-  0,1,1  0,0,-");
              break;
          case 2:
              /* kernel to add for 8-connected lines - no rotation */
              kernel=ParseKernelArray("3: 0,0,0  0,1,0  0,0,1");
              break;
          case 3:
              /* kernel to add for orthogonal line ends - does not find corners */
              kernel=ParseKernelArray("3: 0,0,0  0,1,1  0,0,0");
              break;
          case 4:
              /* traditional line end - fails on last T end */
              kernel=ParseKernelArray("3: 0,0,0  0,1,-  0,0,-");
              break;
          }
          if (kernel == (KernelInfo *) NULL)
            return(kernel);
          kernel->type = type;
          RotateKernelInfo(kernel, args->sigma);
          break;
        }
      case LineJunctionsKernel:
        { /* kernels for finding the junctions of multiple lines */
          switch ( (int) args->rho ) {
            case 0:
            default:
              /* set of kernels to find all line junctions */
              return(AcquireKernelInfo("LineJunctions:1@;LineJunctions:2>",exception));
            case 1:
              /* Y Junction */
              kernel=ParseKernelArray("3: 1,-,1  -,1,-  -,1,-");
              break;
            case 2:
              /* Diagonal T Junctions */
              kernel=ParseKernelArray("3: 1,-,-  -,1,-  1,-,1");
              break;
            case 3:
              /* Orthogonal T Junctions */
              kernel=ParseKernelArray("3: -,-,-  1,1,1  -,1,-");
              break;
            case 4:
              /* Diagonal X Junctions */
              kernel=ParseKernelArray("3: 1,-,1  -,1,-  1,-,1");
              break;
            case 5:
              /* Orthogonal X Junctions - minimal diamond kernel */
              kernel=ParseKernelArray("3: -,1,-  1,1,1  -,1,-");
              break;
          }
          if (kernel == (KernelInfo *) NULL)
            return(kernel);
          kernel->type = type;
          RotateKernelInfo(kernel, args->sigma);
          break;
        }
      case RidgesKernel:
        { /* Ridges - Ridge finding kernels */
          KernelInfo
            *new_kernel;
          switch ( (int) args->rho ) {
            case 1:
            default:
              kernel=ParseKernelArray("3x1:0,1,0");
              if (kernel == (KernelInfo *) NULL)
                return(kernel);
              kernel->type = type;
              ExpandRotateKernelInfo(kernel, 90.0); /* 2 rotated kernels (symmetrical) */
              break;
            case 2:
              kernel=ParseKernelArray("4x1:0,1,1,0");
              if (kernel == (KernelInfo *) NULL)
                return(kernel);
              kernel->type = type;
              ExpandRotateKernelInfo(kernel, 90.0); /* 4 rotated kernels */

              /* Kernels to find a stepped 'thick' line, 4 rotates + mirrors */
              /* Unfortunatally we can not yet rotate a non-square kernel */
              /* But then we can't flip a non-symetrical kernel either */
              new_kernel=ParseKernelArray("4x3+1+1:0,1,1,- -,1,1,- -,1,1,0");
              if (new_kernel == (KernelInfo *) NULL)
                return(DestroyKernelInfo(kernel));
              new_kernel->type = type;
              LastKernelInfo(kernel)->next = new_kernel;
              new_kernel=ParseKernelArray("4x3+2+1:0,1,1,- -,1,1,- -,1,1,0");
              if (new_kernel == (KernelInfo *) NULL)
                return(DestroyKernelInfo(kernel));
              new_kernel->type = type;
              LastKernelInfo(kernel)->next = new_kernel;
              new_kernel=ParseKernelArray("4x3+1+1:-,1,1,0 -,1,1,- 0,1,1,-");
              if (new_kernel == (KernelInfo *) NULL)
                return(DestroyKernelInfo(kernel));
              new_kernel->type = type;
              LastKernelInfo(kernel)->next = new_kernel;
              new_kernel=ParseKernelArray("4x3+2+1:-,1,1,0 -,1,1,- 0,1,1,-");
              if (new_kernel == (KernelInfo *) NULL)
                return(DestroyKernelInfo(kernel));
              new_kernel->type = type;
              LastKernelInfo(kernel)->next = new_kernel;
              new_kernel=ParseKernelArray("3x4+1+1:0,-,- 1,1,1 1,1,1 -,-,0");
              if (new_kernel == (KernelInfo *) NULL)
                return(DestroyKernelInfo(kernel));
              new_kernel->type = type;
              LastKernelInfo(kernel)->next = new_kernel;
              new_kernel=ParseKernelArray("3x4+1+2:0,-,- 1,1,1 1,1,1 -,-,0");
              if (new_kernel == (KernelInfo *) NULL)
                return(DestroyKernelInfo(kernel));
              new_kernel->type = type;
              LastKernelInfo(kernel)->next = new_kernel;
              new_kernel=ParseKernelArray("3x4+1+1:-,-,0 1,1,1 1,1,1 0,-,-");
              if (new_kernel == (KernelInfo *) NULL)
                return(DestroyKernelInfo(kernel));
              new_kernel->type = type;
              LastKernelInfo(kernel)->next = new_kernel;
              new_kernel=ParseKernelArray("3x4+1+2:-,-,0 1,1,1 1,1,1 0,-,-");
              if (new_kernel == (KernelInfo *) NULL)
                return(DestroyKernelInfo(kernel));
              new_kernel->type = type;
              LastKernelInfo(kernel)->next = new_kernel;
              break;
          }
          break;
        }
      case ConvexHullKernel:
        {
          KernelInfo
            *new_kernel;
          /* first set of 8 kernels */
          kernel=ParseKernelArray("3: 1,1,-  1,0,-  1,-,0");
          if (kernel == (KernelInfo *) NULL)
            return(kernel);
          kernel->type = type;
          ExpandRotateKernelInfo(kernel, 90.0);
          /* append the mirror versions too - no flip function yet */
          new_kernel=ParseKernelArray("3: 1,1,1  1,0,-  -,-,0");
          if (new_kernel == (KernelInfo *) NULL)
            return(DestroyKernelInfo(kernel));
          new_kernel->type = type;
          ExpandRotateKernelInfo(new_kernel, 90.0);
          LastKernelInfo(kernel)->next = new_kernel;
          break;
        }
      case SkeletonKernel:
        {
          switch ( (int) args->rho ) {
            case 1:
            default:
              /* Traditional Skeleton...
              ** A cyclically rotated single kernel
              */
              kernel=AcquireKernelInfo("ThinSE:482",exception);
              if (kernel == (KernelInfo *) NULL)
                return(kernel);
              kernel->type = type;
              ExpandRotateKernelInfo(kernel, 45.0); /* 8 rotations */
              break;
            case 2:
              /* HIPR Variation of the cyclic skeleton
              ** Corners of the traditional method made more forgiving,
              ** but the retain the same cyclic order.
              */
              kernel=AcquireKernelInfo("ThinSE:482; ThinSE:87x90;",exception);
              if (kernel == (KernelInfo *) NULL)
                return(kernel);
              if (kernel->next == (KernelInfo *) NULL)
                return(DestroyKernelInfo(kernel));
              kernel->type = type;
              kernel->next->type = type;
              ExpandRotateKernelInfo(kernel, 90.0); /* 4 rotations of the 2 kernels */
              break;
            case 3:
              /* Dan Bloomberg Skeleton, from his paper on 3x3 thinning SE's
              ** "Connectivity-Preserving Morphological Image Thransformations"
              ** by Dan S. Bloomberg, available on Leptonica, Selected Papers,
              **   http://www.leptonica.com/papers/conn.pdf
              */
              kernel=AcquireKernelInfo("ThinSE:41; ThinSE:42; ThinSE:43",
                exception);
              if (kernel == (KernelInfo *) NULL)
                return(kernel);
              kernel->type = type;
              kernel->next->type = type;
              kernel->next->next->type = type;
              ExpandMirrorKernelInfo(kernel); /* 12 kernels total */
              break;
           }
          break;
        }
      case ThinSEKernel:
        { /* Special kernels for general thinning, while preserving connections
          ** "Connectivity-Preserving Morphological Image Thransformations"
          ** by Dan S. Bloomberg, available on Leptonica, Selected Papers,
          **   http://www.leptonica.com/papers/conn.pdf
          ** And
          **   http://tpgit.github.com/Leptonica/ccthin_8c_source.html
          **
          ** Note kernels do not specify the origin pixel, allowing them
          ** to be used for both thickening and thinning operations.
          */
          switch ( (int) args->rho ) {
            /* SE for 4-connected thinning */
            case 41: /* SE_4_1 */
              kernel=ParseKernelArray("3: -,-,1  0,-,1  -,-,1");
              break;
            case 42: /* SE_4_2 */
              kernel=ParseKernelArray("3: -,-,1  0,-,1  -,0,-");
              break;
            case 43: /* SE_4_3 */
              kernel=ParseKernelArray("3: -,0,-  0,-,1  -,-,1");
              break;
            case 44: /* SE_4_4 */
              kernel=ParseKernelArray("3: -,0,-  0,-,1  -,0,-");
              break;
            case 45: /* SE_4_5 */
              kernel=ParseKernelArray("3: -,0,1  0,-,1  -,0,-");
              break;
            case 46: /* SE_4_6 */
              kernel=ParseKernelArray("3: -,0,-  0,-,1  -,0,1");
              break;
            case 47: /* SE_4_7 */
              kernel=ParseKernelArray("3: -,1,1  0,-,1  -,0,-");
              break;
            case 48: /* SE_4_8 */
              kernel=ParseKernelArray("3: -,-,1  0,-,1  0,-,1");
              break;
            case 49: /* SE_4_9 */
              kernel=ParseKernelArray("3: 0,-,1  0,-,1  -,-,1");
              break;
            /* SE for 8-connected thinning - negatives of the above */
            case 81: /* SE_8_0 */
              kernel=ParseKernelArray("3: -,1,-  0,-,1  -,1,-");
              break;
            case 82: /* SE_8_2 */
              kernel=ParseKernelArray("3: -,1,-  0,-,1  0,-,-");
              break;
            case 83: /* SE_8_3 */
              kernel=ParseKernelArray("3: 0,-,-  0,-,1  -,1,-");
              break;
            case 84: /* SE_8_4 */
              kernel=ParseKernelArray("3: 0,-,-  0,-,1  0,-,-");
              break;
            case 85: /* SE_8_5 */
              kernel=ParseKernelArray("3: 0,-,1  0,-,1  0,-,-");
              break;
            case 86: /* SE_8_6 */
              kernel=ParseKernelArray("3: 0,-,-  0,-,1  0,-,1");
              break;
            case 87: /* SE_8_7 */
              kernel=ParseKernelArray("3: -,1,-  0,-,1  0,0,-");
              break;
            case 88: /* SE_8_8 */
              kernel=ParseKernelArray("3: -,1,-  0,-,1  0,1,-");
              break;
            case 89: /* SE_8_9 */
              kernel=ParseKernelArray("3: 0,1,-  0,-,1  -,1,-");
              break;
            /* Special combined SE kernels */
            case 423: /* SE_4_2 , SE_4_3 Combined Kernel */
              kernel=ParseKernelArray("3: -,-,1  0,-,-  -,0,-");
              break;
            case 823: /* SE_8_2 , SE_8_3 Combined Kernel */
              kernel=ParseKernelArray("3: -,1,-  -,-,1  0,-,-");
              break;
            case 481: /* SE_48_1 - General Connected Corner Kernel */
              kernel=ParseKernelArray("3: -,1,1  0,-,1  0,0,-");
              break;
            default:
            case 482: /* SE_48_2 - General Edge Kernel */
              kernel=ParseKernelArray("3: 0,-,1  0,-,1  0,-,1");
              break;
          }
          if (kernel == (KernelInfo *) NULL)
            return(kernel);
          kernel->type = type;
          RotateKernelInfo(kernel, args->sigma);
          break;
        }
      /*
        Distance Measuring Kernels
      */
      case ChebyshevKernel:
        {
          if (args->rho < 1.0)
            kernel->width = kernel->height = 3;  /* default radius = 1 */
          else
            kernel->width = kernel->height = ((size_t)args->rho)*2+1;
          kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;

          kernel->values=(MagickRealType *) MagickAssumeAligned(
            AcquireAlignedMemory(kernel->width,kernel->height*
            sizeof(*kernel->values)));
          if (kernel->values == (MagickRealType *) NULL)
            return(DestroyKernelInfo(kernel));

          for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
            for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
              kernel->positive_range += ( kernel->values[i] =
                  args->sigma*MagickMax(fabs((double)u),fabs((double)v)) );
          kernel->maximum = kernel->values[0];
          break;
        }
      case ManhattanKernel:
        {
          if (args->rho < 1.0)
            kernel->width = kernel->height = 3;  /* default radius = 1 */
          else
            kernel->width = kernel->height = ((size_t)args->rho)*2+1;
          kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;

          kernel->values=(MagickRealType *) MagickAssumeAligned(
            AcquireAlignedMemory(kernel->width,kernel->height*
            sizeof(*kernel->values)));
          if (kernel->values == (MagickRealType *) NULL)
            return(DestroyKernelInfo(kernel));

          for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
            for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
              kernel->positive_range += ( kernel->values[i] =
                  args->sigma*(labs((long) u)+labs((long) v)) );
          kernel->maximum = kernel->values[0];
          break;
        }
      case OctagonalKernel:
      {
        if (args->rho < 2.0)
          kernel->width = kernel->height = 5;  /* default/minimum radius = 2 */
        else
          kernel->width = kernel->height = ((size_t)args->rho)*2+1;
        kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;

        kernel->values=(MagickRealType *) MagickAssumeAligned(
          AcquireAlignedMemory(kernel->width,kernel->height*
          sizeof(*kernel->values)));
        if (kernel->values == (MagickRealType *) NULL)
          return(DestroyKernelInfo(kernel));

        for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
          for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
            {
              double
                r1 = MagickMax(fabs((double)u),fabs((double)v)),
                r2 = floor((double)(labs((long)u)+labs((long)v)+1)/1.5);
              kernel->positive_range += kernel->values[i] =
                        args->sigma*MagickMax(r1,r2);
            }
        kernel->maximum = kernel->values[0];
        break;
      }
    case EuclideanKernel:
      {
        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
          kernel->width = kernel->height = ((size_t)args->rho)*2+1;
        kernel->x = kernel->y = (ssize_t) (kernel->width-1)/2;

        kernel->values=(MagickRealType *) MagickAssumeAligned(
          AcquireAlignedMemory(kernel->width,kernel->height*
          sizeof(*kernel->values)));
        if (kernel->values == (MagickRealType *) NULL)
          return(DestroyKernelInfo(kernel));

        for ( i=0, v=-kernel->y; v <= (ssize_t)kernel->y; v++)
          for ( u=-kernel->x; u <= (ssize_t)kernel->x; u++, i++)
            kernel->positive_range += ( kernel->values[i] =
              args->sigma*sqrt((double)(u*u+v*v)) );
        kernel->maximum = kernel->values[0];
        break;
      }
    default:
      {
        /* No-Op Kernel - Basically just a single pixel on its own */
        kernel=ParseKernelArray("1:1");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = UndefinedKernel;
        break;
      }
      break;
  }
  return(kernel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C l o n e K e r n e l I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneKernelInfo() creates a new clone of the given Kernel List so that its
%  can be modified without effecting the original.  The cloned kernel should
%  be destroyed using DestoryKernelInfo() when no longer needed.
%
%  The format of the CloneKernelInfo method is:
%
%      KernelInfo *CloneKernelInfo(const KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel to be cloned
%
*/
MagickExport KernelInfo *CloneKernelInfo(const KernelInfo *kernel)
{
  register ssize_t
    i;

  KernelInfo
    *new_kernel;

  assert(kernel != (KernelInfo *) NULL);
  new_kernel=(KernelInfo *) AcquireMagickMemory(sizeof(*kernel));
  if (new_kernel == (KernelInfo *) NULL)
    return(new_kernel);
  *new_kernel=(*kernel); /* copy values in structure */

  /* replace the values with a copy of the values */
  new_kernel->values=(MagickRealType *) MagickAssumeAligned(
    AcquireAlignedMemory(kernel->width,kernel->height*sizeof(*kernel->values)));
  if (new_kernel->values == (MagickRealType *) NULL)
    return(DestroyKernelInfo(new_kernel));
  for (i=0; i < (ssize_t) (kernel->width*kernel->height); i++)
    new_kernel->values[i]=kernel->values[i];

  /* Also clone the next kernel in the kernel list */
  if ( kernel->next != (KernelInfo *) NULL ) {
    new_kernel->next = CloneKernelInfo(kernel->next);
    if ( new_kernel->next == (KernelInfo *) NULL )
      return(DestroyKernelInfo(new_kernel));
  }

  return(new_kernel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     D e s t r o y K e r n e l I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyKernelInfo() frees the memory used by a Convolution/Morphology
%  kernel.
%
%  The format of the DestroyKernelInfo method is:
%
%      KernelInfo *DestroyKernelInfo(KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel to be destroyed
%
*/
MagickExport KernelInfo *DestroyKernelInfo(KernelInfo *kernel)
{
  assert(kernel != (KernelInfo *) NULL);
  if (kernel->next != (KernelInfo *) NULL)
    kernel->next=DestroyKernelInfo(kernel->next);
  kernel->values=(MagickRealType *) RelinquishAlignedMemory(kernel->values);
  kernel=(KernelInfo *) RelinquishMagickMemory(kernel);
  return(kernel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     E x p a n d M i r r o r K e r n e l I n f o                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExpandMirrorKernelInfo() takes a single kernel, and expands it into a
%  sequence of 90-degree rotated kernels but providing a reflected 180
%  rotatation, before the -/+ 90-degree rotations.
%
%  This special rotation order produces a better, more symetrical thinning of
%  objects.
%
%  The format of the ExpandMirrorKernelInfo method is:
%
%      void ExpandMirrorKernelInfo(KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
% This function is only internel to this module, as it is not finalized,
% especially with regard to non-orthogonal angles, and rotation of larger
% 2D kernels.
*/

#if 0
static void FlopKernelInfo(KernelInfo *kernel)
    { /* Do a Flop by reversing each row. */
      size_t
        y;
      register ssize_t
        x,r;
      register double
        *k,t;

      for ( y=0, k=kernel->values; y < kernel->height; y++, k+=kernel->width)
        for ( x=0, r=kernel->width-1; x<kernel->width/2; x++, r--)
          t=k[x],  k[x]=k[r],  k[r]=t;

      kernel->x = kernel->width - kernel->x - 1;
      angle = fmod(angle+180.0, 360.0);
    }
#endif

static void ExpandMirrorKernelInfo(KernelInfo *kernel)
{
  KernelInfo
    *clone,
    *last;

  last = kernel;

  clone = CloneKernelInfo(last);
  if (clone == (KernelInfo *) NULL)
    return;
  RotateKernelInfo(clone, 180);   /* flip */
  LastKernelInfo(last)->next = clone;
  last = clone;

  clone = CloneKernelInfo(last);
  if (clone == (KernelInfo *) NULL)
    return;
  RotateKernelInfo(clone, 90);   /* transpose */
  LastKernelInfo(last)->next = clone;
  last = clone;

  clone = CloneKernelInfo(last);
  if (clone == (KernelInfo *) NULL)
    return;
  RotateKernelInfo(clone, 180);  /* flop */
  LastKernelInfo(last)->next = clone;

  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     E x p a n d R o t a t e K e r n e l I n f o                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExpandRotateKernelInfo() takes a kernel list, and expands it by rotating
%  incrementally by the angle given, until the kernel repeats.
%
%  WARNING: 45 degree rotations only works for 3x3 kernels.
%  While 90 degree roatations only works for linear and square kernels
%
%  The format of the ExpandRotateKernelInfo method is:
%
%      void ExpandRotateKernelInfo(KernelInfo *kernel, double angle)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
%    o angle: angle to rotate in degrees
%
% This function is only internel to this module, as it is not finalized,
% especially with regard to non-orthogonal angles, and rotation of larger
% 2D kernels.
*/

/* Internal Routine - Return true if two kernels are the same */
static MagickBooleanType SameKernelInfo(const KernelInfo *kernel1,
     const KernelInfo *kernel2)
{
  register size_t
    i;

  /* check size and origin location */
  if (    kernel1->width != kernel2->width
       || kernel1->height != kernel2->height
       || kernel1->x != kernel2->x
       || kernel1->y != kernel2->y )
    return MagickFalse;

  /* check actual kernel values */
  for (i=0; i < (kernel1->width*kernel1->height); i++) {
    /* Test for Nan equivalence */
    if ( IsNaN(kernel1->values[i]) && !IsNaN(kernel2->values[i]) )
      return MagickFalse;
    if ( IsNaN(kernel2->values[i]) && !IsNaN(kernel1->values[i]) )
      return MagickFalse;
    /* Test actual values are equivalent */
    if ( fabs(kernel1->values[i] - kernel2->values[i]) >= MagickEpsilon )
      return MagickFalse;
  }

  return MagickTrue;
}

static void ExpandRotateKernelInfo(KernelInfo *kernel,const double angle)
{
  KernelInfo
    *clone_info,
    *last;

  clone_info=(KernelInfo *) NULL;
  last=kernel;
DisableMSCWarning(4127)
  while (1) {
RestoreMSCWarning
    clone_info=CloneKernelInfo(last);
    if (clone_info == (KernelInfo *) NULL)
      break;
    RotateKernelInfo(clone_info,angle);
    if (SameKernelInfo(kernel,clone_info) != MagickFalse)
      break;
    LastKernelInfo(last)->next=clone_info;
    last=clone_info;
  }
  if (clone_info != (KernelInfo *) NULL)
    clone_info=DestroyKernelInfo(clone_info);  /* kernel repeated - junk */
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     C a l c M e t a K e r n a l I n f o                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CalcKernelMetaData() recalculate the KernelInfo meta-data of this kernel only,
%  using the kernel values.  This should only ne used if it is not possible to
%  calculate that meta-data in some easier way.
%
%  It is important that the meta-data is correct before ScaleKernelInfo() is
%  used to perform kernel normalization.
%
%  The format of the CalcKernelMetaData method is:
%
%      void CalcKernelMetaData(KernelInfo *kernel, const double scale )
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel to modify
%
%  WARNING: Minimum and Maximum values are assumed to include zero, even if
%  zero is not part of the kernel (as in Gaussian Derived kernels). This
%  however is not true for flat-shaped morphological kernels.
%
%  WARNING: Only the specific kernel pointed to is modified, not a list of
%  multiple kernels.
%
% This is an internal function and not expected to be useful outside this
% module.  This could change however.
*/
static void CalcKernelMetaData(KernelInfo *kernel)
{
  register size_t
    i;

  kernel->minimum = kernel->maximum = 0.0;
  kernel->negative_range = kernel->positive_range = 0.0;
  for (i=0; i < (kernel->width*kernel->height); i++)
    {
      if ( fabs(kernel->values[i]) < MagickEpsilon )
        kernel->values[i] = 0.0;
      ( kernel->values[i] < 0)
          ?  ( kernel->negative_range += kernel->values[i] )
          :  ( kernel->positive_range += kernel->values[i] );
      Minimize(kernel->minimum, kernel->values[i]);
      Maximize(kernel->maximum, kernel->values[i]);
    }

  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o r p h o l o g y A p p l y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MorphologyApply() applies a morphological method, multiple times using
%  a list of multiple kernels.  This is the method that should be called by
%  other 'operators' that internally use morphology operations as part of
%  their processing.
%
%  It is basically equivalent to as MorphologyImage() (see below) but without
%  any user controls.  This allows internel programs to use this method to
%  perform a specific task without possible interference by any API user
%  supplied settings.
%
%  It is MorphologyImage() task to extract any such user controls, and
%  pass them to this function for processing.
%
%  More specifically all given kernels should already be scaled, normalised,
%  and blended appropriatally before being parred to this routine. The
%  appropriate bias, and compose (typically 'UndefinedComposeOp') given.
%
%  The format of the MorphologyApply method is:
%
%      Image *MorphologyApply(const Image *image,MorphologyMethod method,
%        const ssize_t iterations,const KernelInfo *kernel,
%        const CompositeMethod compose,const double bias,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the source image
%
%    o method: the morphology method to be applied.
%
%    o iterations: apply the operation this many times (or no change).
%                  A value of -1 means loop until no change found.
%                  How this is applied may depend on the morphology method.
%                  Typically this is a value of 1.
%
%    o channel: the channel type.
%
%    o kernel: An array of double representing the morphology kernel.
%
%    o compose: How to handle or merge multi-kernel results.
%          If 'UndefinedCompositeOp' use default for the Morphology method.
%          If 'NoCompositeOp' force image to be re-iterated by each kernel.
%          Otherwise merge the results using the compose method given.
%
%    o bias: Convolution Output Bias.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static ssize_t MorphologyPrimitive(const Image *image,Image *morphology_image,
  const MorphologyMethod method,const KernelInfo *kernel,const double bias,
  ExceptionInfo *exception)
{
#define MorphologyTag  "Morphology/Image"

  CacheView
    *image_view,
    *morphology_view;

  OffsetInfo
    offset;

  register ssize_t
    j,
    y;

  size_t
    *changes,
    changed,
    width;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(morphology_image != (Image *) NULL);
  assert(morphology_image->signature == MagickCoreSignature);
  assert(kernel != (KernelInfo *) NULL);
  assert(kernel->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  morphology_view=AcquireAuthenticCacheView(morphology_image,exception);
  width=image->columns+kernel->width-1;
  offset.x=0;
  offset.y=0;
  switch (method)
  {
    case ConvolveMorphology:
    case DilateMorphology:
    case DilateIntensityMorphology:
    case IterativeDistanceMorphology:
    {
      /*
        Kernel needs to used with reflection about origin.
      */
      offset.x=(ssize_t) kernel->width-kernel->x-1;
      offset.y=(ssize_t) kernel->height-kernel->y-1;
      break;
    }
    case ErodeMorphology:
    case ErodeIntensityMorphology:
    case HitAndMissMorphology:
    case ThinningMorphology:
    case ThickenMorphology:
    {
      offset.x=kernel->x;
      offset.y=kernel->y;
      break;
    }
    default:
    {
      assert("Not a Primitive Morphology Method" != (char *) NULL);
      break;
    }
  }
  changed=0;
  changes=(size_t *) AcquireQuantumMemory(GetOpenMPMaximumThreads(),
    sizeof(*changes));
  if (changes == (size_t *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  for (j=0; j < (ssize_t) GetOpenMPMaximumThreads(); j++)
    changes[j]=0;

  if ((method == ConvolveMorphology) && (kernel->width == 1))
    {
      register ssize_t
        x;

      /*
        Special handling (for speed) of vertical (blur) kernels.  This performs
        its handling in columns rather than in rows.  This is only done
        for convolve as it is the only method that generates very large 1-D
        vertical kernels (such as a 'BlurKernel')
     */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
     #pragma omp parallel for schedule(static) shared(progress,status) \
       magick_number_threads(image,morphology_image,image->columns,1)
#endif
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        const int
          id = GetOpenMPThreadId();

        register const Quantum
          *magick_restrict p;

        register Quantum
          *magick_restrict q;

        register ssize_t
          r;

        ssize_t
          center;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(image_view,x,-offset.y,1,image->rows+
          kernel->height-1,exception);
        q=GetCacheViewAuthenticPixels(morphology_view,x,0,1,
          morphology_image->rows,exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        center=(ssize_t) GetPixelChannels(image)*offset.y;
        for (r=0; r < (ssize_t) image->rows; r++)
        {
          register ssize_t
            i;

          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            double
              alpha,
              gamma,
              pixel;

            PixelChannel
              channel;

            PixelTrait
              morphology_traits,
              traits;

            register const MagickRealType
              *magick_restrict k;

            register const Quantum
              *magick_restrict pixels;

            register ssize_t
              v;

            size_t
              count;

            channel=GetPixelChannelChannel(image,i);
            traits=GetPixelChannelTraits(image,channel);
            morphology_traits=GetPixelChannelTraits(morphology_image,channel);
            if ((traits == UndefinedPixelTrait) ||
                (morphology_traits == UndefinedPixelTrait))
              continue;
            if ((traits & CopyPixelTrait) != 0)
              {
                SetPixelChannel(morphology_image,channel,p[center+i],q);
                continue;
              }
            k=(&kernel->values[kernel->height-1]);
            pixels=p;
            pixel=bias;
            gamma=0.0;
            count=0;
            if (((image->alpha_trait & BlendPixelTrait) == 0) ||
                ((morphology_traits & BlendPixelTrait) == 0))
              for (v=0; v < (ssize_t) kernel->height; v++)
              {
                if (!IsNaN(*k))
                  {
                    pixel+=(*k)*pixels[i];
                    gamma+=(*k);
                    count++;
                  }
                k--;
                pixels+=GetPixelChannels(image);
              }
            else
              for (v=0; v < (ssize_t) kernel->height; v++)
              {
                if (!IsNaN(*k))
                  {
                    alpha=(double) (QuantumScale*GetPixelAlpha(image,pixels));
                    pixel+=alpha*(*k)*pixels[i];
                    gamma+=alpha*(*k);
                    count++;
                  }
                k--;
                pixels+=GetPixelChannels(image);
              }
            if (fabs(pixel-p[center+i]) > MagickEpsilon)
              changes[id]++;
            gamma=PerceptibleReciprocal(gamma);
            if (count != 0)
              gamma*=(double) kernel->height/count;
            SetPixelChannel(morphology_image,channel,ClampToQuantum(gamma*
              pixel),q);
          }
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(morphology_image);
        }
        if (SyncCacheViewAuthenticPixels(morphology_view,exception) == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp atomic
#endif
            progress++;
            proceed=SetImageProgress(image,MorphologyTag,progress,image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      morphology_image->type=image->type;
      morphology_view=DestroyCacheView(morphology_view);
      image_view=DestroyCacheView(image_view);
      for (j=0; j < (ssize_t) GetOpenMPMaximumThreads(); j++)
        changed+=changes[j];
      changes=(size_t *) RelinquishMagickMemory(changes);
      return(status ? (ssize_t) changed : 0);
    }
  /*
    Normal handling of horizontal or rectangular kernels (row by row).
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,morphology_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    ssize_t
      center;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-offset.x,y-offset.y,width,
      kernel->height,exception);
    q=GetCacheViewAuthenticPixels(morphology_view,0,y,morphology_image->columns,
      1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    center=(ssize_t) (GetPixelChannels(image)*width*offset.y+
      GetPixelChannels(image)*offset.x);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          alpha,
          gamma,
          intensity,
          maximum,
          minimum,
          pixel;

        PixelChannel
          channel;

        PixelTrait
          morphology_traits,
          traits;

        register const MagickRealType
          *magick_restrict k;

        register const Quantum
          *magick_restrict pixels,
          *magick_restrict quantum_pixels;

        register ssize_t
          u;

        size_t
          count;

        ssize_t
          v;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        morphology_traits=GetPixelChannelTraits(morphology_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (morphology_traits == UndefinedPixelTrait))
          continue;
        if ((traits & CopyPixelTrait) != 0)
          {
            SetPixelChannel(morphology_image,channel,p[center+i],q);
            continue;
          }
        pixels=p;
        quantum_pixels=(const Quantum *) NULL;
        maximum=0.0;
        minimum=(double) QuantumRange;
        switch (method)
        {
          case ConvolveMorphology:
          {
            pixel=bias;
            break;
          }
          case DilateMorphology:
          case ErodeIntensityMorphology:
          {
            pixel=0.0;
            break;
          }
          case HitAndMissMorphology:
          case ErodeMorphology:
          {
            pixel=QuantumRange;
            break;
          }
          default:
          {
            pixel=(double) p[center+i];
            break;
          }
        }
        count=0;
        gamma=1.0;
        switch (method)
        {
          case ConvolveMorphology:
          {
            /*
               Weighted Average of pixels using reflected kernel

               For correct working of this operation for asymetrical kernels,
               the kernel needs to be applied in its reflected form.  That is
               its values needs to be reversed.

               Correlation is actually the same as this but without reflecting
               the kernel, and thus 'lower-level' that Convolution.  However as
               Convolution is the more common method used, and it does not
               really cost us much in terms of processing to use a reflected
               kernel, so it is Convolution that is implemented.

               Correlation will have its kernel reflected before calling this
               function to do a Convolve.

               For more details of Correlation vs Convolution see
                 http://www.cs.umd.edu/~djacobs/CMSC426/Convolution.pdf
            */
            k=(&kernel->values[kernel->width*kernel->height-1]);
            if (((image->alpha_trait & BlendPixelTrait) == 0) ||
                ((morphology_traits & BlendPixelTrait) == 0))
              {
                /*
                  No alpha blending.
                */
                for (v=0; v < (ssize_t) kernel->height; v++)
                {
                  for (u=0; u < (ssize_t) kernel->width; u++)
                  {
                    if (!IsNaN(*k))
                      {
                        pixel+=(*k)*pixels[i];
                        count++;
                      }
                    k--;
                    pixels+=GetPixelChannels(image);
                  }
                  pixels+=(image->columns-1)*GetPixelChannels(image);
                }
                break;
              }
            /*
              Alpha blending.
            */
            gamma=0.0;
            for (v=0; v < (ssize_t) kernel->height; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k))
                  {
                    alpha=(double) (QuantumScale*GetPixelAlpha(image,pixels));
                    pixel+=alpha*(*k)*pixels[i];
                    gamma+=alpha*(*k);
                    count++;
                  }
                k--;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            break;
          }
          case ErodeMorphology:
          {
            /*
              Minimum value within kernel neighbourhood.

              The kernel is not reflected for this operation.  In normal
              Greyscale Morphology, the kernel value should be added
              to the real value, this is currently not done, due to the
              nature of the boolean kernels being used.
            */
            k=kernel->values;
            for (v=0; v < (ssize_t) kernel->height; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k) && (*k >= 0.5))
                  {
                    if ((double) pixels[i] < pixel)
                      pixel=(double) pixels[i];
                  }
                k++;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            break;
          }
          case DilateMorphology:
          {
            /*
               Maximum value within kernel neighbourhood.

               For correct working of this operation for asymetrical kernels,
               the kernel needs to be applied in its reflected form.  That is
               its values needs to be reversed.

               In normal Greyscale Morphology, the kernel value should be
               added to the real value, this is currently not done, due to the
               nature of the boolean kernels being used.
            */
            k=(&kernel->values[kernel->width*kernel->height-1]);
            for (v=0; v < (ssize_t) kernel->height; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k) && (*k > 0.5))
                  {
                    if ((double) pixels[i] > pixel)
                      pixel=(double) pixels[i];
                  }
                k--;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            break;
          }
          case HitAndMissMorphology:
          case ThinningMorphology:
          case ThickenMorphology:
          {
            /*
               Minimum of foreground pixel minus maxumum of background pixels.

               The kernel is not reflected for this operation, and consists
               of both foreground and background pixel neighbourhoods, 0.0 for
               background, and 1.0 for foreground with either Nan or 0.5 values
               for don't care.

               This never produces a meaningless negative result.  Such results
               cause Thinning/Thicken to not work correctly when used against a
               greyscale image.
            */
            k=kernel->values;
            for (v=0; v < (ssize_t) kernel->height; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k))
                  {
                    if (*k > 0.7)
                      {
                        if ((double) pixels[i] < pixel)
                          pixel=(double) pixels[i];
                      }
                    else
                      if (*k < 0.3)
                        {
                          if ((double) pixels[i] > maximum)
                            maximum=(double) pixels[i];
                        }
                    count++;
                  }
                k++;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            pixel-=maximum;
            if (pixel < 0.0)
              pixel=0.0;
            if (method == ThinningMorphology)
              pixel=(double) p[center+i]-pixel;
            else
              if (method == ThickenMorphology)
                pixel+=(double) p[center+i]+pixel;
            break;
          }
          case ErodeIntensityMorphology:
          {
            /*
              Select pixel with minimum intensity within kernel neighbourhood.

              The kernel is not reflected for this operation.
            */
            k=kernel->values;
            for (v=0; v < (ssize_t) kernel->height; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k) && (*k >= 0.5))
                  {
                    intensity=(double) GetPixelIntensity(image,pixels);
                    if (intensity < minimum)
                      {
                        quantum_pixels=pixels;
                        pixel=(double) pixels[i];
                        minimum=intensity;
                      }
                    count++;
                  }
                k++;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            break;
          }
          case DilateIntensityMorphology:
          {
            /*
              Select pixel with maximum intensity within kernel neighbourhood.

              The kernel is not reflected for this operation.
            */
            k=(&kernel->values[kernel->width*kernel->height-1]);
            for (v=0; v < (ssize_t) kernel->height; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k) && (*k >= 0.5))
                  {
                    intensity=(double) GetPixelIntensity(image,pixels);
                    if (intensity > maximum)
                      {
                        pixel=(double) pixels[i];
                        quantum_pixels=pixels;
                        maximum=intensity;
                      }
                    count++;
                  }
                k--;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            break;
          }
          case IterativeDistanceMorphology:
          {
            /*
               Compute th iterative distance from black edge of a white image
               shape.  Essentually white values are decreased to the smallest
               'distance from edge' it can find.

               It works by adding kernel values to the neighbourhood, and
               select the minimum value found. The kernel is rotated before
               use, so kernel distances match resulting distances, when a user
               provided asymmetric kernel is applied.

               This code is nearly identical to True GrayScale Morphology but
               not quite.

               GreyDilate Kernel values added, maximum value found Kernel is
               rotated before use.

               GrayErode:  Kernel values subtracted and minimum value found No
               kernel rotation used.

               Note the Iterative Distance method is essentially a
               GrayErode, but with negative kernel values, and kernel rotation
               applied.
            */
            k=(&kernel->values[kernel->width*kernel->height-1]);
            for (v=0; v < (ssize_t) kernel->height; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k))
                  {
                    if ((pixels[i]+(*k)) < pixel)
                      pixel=(double) pixels[i]+(*k);
                    count++;
                  }
                k--;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            break;
          }
          case UndefinedMorphology:
          default:
            break;
        }
        if (fabs(pixel-p[center+i]) > MagickEpsilon)
          changes[id]++;
        if (quantum_pixels != (const Quantum *) NULL)
          {
            SetPixelChannel(morphology_image,channel,quantum_pixels[i],q);
            continue;
          }
        gamma=PerceptibleReciprocal(gamma);
        if (count != 0)
          gamma*=(double) kernel->height*kernel->width/count;
        SetPixelChannel(morphology_image,channel,ClampToQuantum(gamma*pixel),q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(morphology_image);
    }
    if (SyncCacheViewAuthenticPixels(morphology_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,MorphologyTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  morphology_view=DestroyCacheView(morphology_view);
  image_view=DestroyCacheView(image_view);
  for (j=0; j < (ssize_t) GetOpenMPMaximumThreads(); j++)
    changed+=changes[j];
  changes=(size_t *) RelinquishMagickMemory(changes);
  return(status ? (ssize_t) changed : -1);
}

/*
  This is almost identical to the MorphologyPrimative() function above, but
  applies the primitive directly to the actual image using two passes, once in
  each direction, with the results of the previous (and current) row being
  re-used.

  That is after each row is 'Sync'ed' into the image, the next row makes use of
  those values as part of the calculation of the next row.  It repeats, but
  going in the oppisite (bottom-up) direction.

  Because of this 're-use of results' this function can not make use of multi-
  threaded, parellel processing.
*/
static ssize_t MorphologyPrimitiveDirect(Image *image,
  const MorphologyMethod method,const KernelInfo *kernel,
  ExceptionInfo *exception)
{
  CacheView
    *morphology_view,
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  OffsetInfo
    offset;

  size_t
    width,
    changed;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(kernel != (KernelInfo *) NULL);
  assert(kernel->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=MagickTrue;
  changed=0;
  progress=0;
  switch(method)
  {
    case DistanceMorphology:
    case VoronoiMorphology:
    {
      /*
        Kernel reflected about origin.
      */
      offset.x=(ssize_t) kernel->width-kernel->x-1;
      offset.y=(ssize_t) kernel->height-kernel->y-1;
      break;
    }
    default:
    {
      offset.x=kernel->x;
      offset.y=kernel->y;
      break;
    }
  }
  /*
    Two views into same image, do not thread.
  */
  image_view=AcquireVirtualCacheView(image,exception);
  morphology_view=AcquireAuthenticCacheView(image,exception);
  width=image->columns+kernel->width-1;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    /*
      Read virtual pixels, and authentic pixels, from the same image!  We read
      using virtual to get virtual pixel handling, but write back into the same
      image.

      Only top half of kernel is processed as we do a single pass downward
      through the image iterating the distance function as we go.
    */
    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-offset.x,y-offset.y,width,(size_t)
      offset.y+1,exception);
    q=GetCacheViewAuthenticPixels(morphology_view,0,y,image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          pixel;

        PixelChannel
          channel;

        PixelTrait
          traits;

        register const MagickRealType
          *magick_restrict k;

        register const Quantum
          *magick_restrict pixels;

        register ssize_t
          u;

        ssize_t
          v;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & CopyPixelTrait) != 0)
          continue;
        pixels=p;
        pixel=(double) QuantumRange;
        switch (method)
        {
          case DistanceMorphology:
          {
            k=(&kernel->values[kernel->width*kernel->height-1]);
            for (v=0; v <= offset.y; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k))
                  {
                    if ((pixels[i]+(*k)) < pixel)
                      pixel=(double) pixels[i]+(*k);
                  }
                k--;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            k=(&kernel->values[kernel->width*(kernel->y+1)-1]);
            pixels=q-offset.x*GetPixelChannels(image);
            for (u=0; u < offset.x; u++)
            {
              if (!IsNaN(*k) && ((x+u-offset.x) >= 0))
                {
                  if ((pixels[i]+(*k)) < pixel)
                    pixel=(double) pixels[i]+(*k);
                }
              k--;
              pixels+=GetPixelChannels(image);
            }
            break;
          }
          case VoronoiMorphology:
          {
            k=(&kernel->values[kernel->width*kernel->height-1]);
            for (v=0; v < offset.y; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k))
                  {
                    if ((pixels[i]+(*k)) < pixel)
                      pixel=(double) pixels[i]+(*k);
                  }
                k--;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            k=(&kernel->values[kernel->width*(kernel->y+1)-1]);
            pixels=q-offset.x*GetPixelChannels(image);
            for (u=0; u < offset.x; u++)
            {
              if (!IsNaN(*k) && ((x+u-offset.x) >= 0))
                {
                  if ((pixels[i]+(*k)) < pixel)
                    pixel=(double) pixels[i]+(*k);
                }
              k--;
              pixels+=GetPixelChannels(image);
            }
            break;
          }
          default:
            break;
        }
        if (fabs(pixel-q[i]) > MagickEpsilon)
          changed++;
        q[i]=ClampToQuantum(pixel);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(morphology_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,MorphologyTag,progress,2*image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  morphology_view=DestroyCacheView(morphology_view);
  image_view=DestroyCacheView(image_view);
  /*
    Do the reverse pass through the image.
  */
  image_view=AcquireVirtualCacheView(image,exception);
  morphology_view=AcquireAuthenticCacheView(image,exception);
  for (y=(ssize_t) image->rows-1; y >= 0; y--)
  {
    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    /*
       Read virtual pixels, and authentic pixels, from the same image.  We
       read using virtual to get virtual pixel handling, but write back
       into the same image.

       Only the bottom half of the kernel is processed as we up the image.
    */
    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-offset.x,y,width,(size_t)
      kernel->y+1,exception);
    q=GetCacheViewAuthenticPixels(morphology_view,0,y,image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    p+=(image->columns-1)*GetPixelChannels(image);
    q+=(image->columns-1)*GetPixelChannels(image);
    for (x=(ssize_t) image->columns-1; x >= 0; x--)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          pixel;

        PixelChannel
          channel;

        PixelTrait
          traits;

        register const MagickRealType
          *magick_restrict k;

        register const Quantum
          *magick_restrict pixels;

        register ssize_t
          u;

        ssize_t
          v;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & CopyPixelTrait) != 0)
          continue;
        pixels=p;
        pixel=(double) QuantumRange;
        switch (method)
        {
          case DistanceMorphology:
          {
            k=(&kernel->values[kernel->width*(kernel->y+1)-1]);
            for (v=offset.y; v < (ssize_t) kernel->height; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k))
                  {
                    if ((pixels[i]+(*k)) < pixel)
                      pixel=(double) pixels[i]+(*k);
                  }
                k--;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            k=(&kernel->values[kernel->width*kernel->y+kernel->x-1]);
            pixels=q;
            for (u=offset.x+1; u < (ssize_t) kernel->width; u++)
            {
              pixels+=GetPixelChannels(image);
              if (!IsNaN(*k) && ((x+u-offset.x) < (ssize_t) image->columns))
                {
                  if ((pixels[i]+(*k)) < pixel)
                    pixel=(double) pixels[i]+(*k);
                }
              k--;
            }
            break;
          }
          case VoronoiMorphology:
          {
            k=(&kernel->values[kernel->width*(kernel->y+1)-1]);
            for (v=offset.y; v < (ssize_t) kernel->height; v++)
            {
              for (u=0; u < (ssize_t) kernel->width; u++)
              {
                if (!IsNaN(*k))
                  {
                    if ((pixels[i]+(*k)) < pixel)
                      pixel=(double) pixels[i]+(*k);
                  }
                k--;
                pixels+=GetPixelChannels(image);
              }
              pixels+=(image->columns-1)*GetPixelChannels(image);
            }
            k=(&kernel->values[kernel->width*(kernel->y+1)-1]);
            pixels=q;
            for (u=offset.x+1; u < (ssize_t) kernel->width; u++)
            {
              pixels+=GetPixelChannels(image);
              if (!IsNaN(*k) && ((x+u-offset.x) < (ssize_t) image->columns))
                {
                  if ((pixels[i]+(*k)) < pixel)
                    pixel=(double) pixels[i]+(*k);
                }
              k--;
            }
            break;
          }
          default:
            break;
        }
        if (fabs(pixel-q[i]) > MagickEpsilon)
          changed++;
        q[i]=ClampToQuantum(pixel);
      }
      p-=GetPixelChannels(image);
      q-=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(morphology_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,MorphologyTag,progress,2*image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  morphology_view=DestroyCacheView(morphology_view);
  image_view=DestroyCacheView(image_view);
  return(status ? (ssize_t) changed : -1);
}

/*
  Apply a Morphology by calling one of the above low level primitive
  application functions.  This function handles any iteration loops,
  composition or re-iteration of results, and compound morphology methods that
  is based on multiple low-level (staged) morphology methods.

  Basically this provides the complex glue between the requested morphology
  method and raw low-level implementation (above).
*/
MagickPrivate Image *MorphologyApply(const Image *image,
  const MorphologyMethod method, const ssize_t iterations,
  const KernelInfo *kernel, const CompositeOperator compose,const double bias,
  ExceptionInfo *exception)
{
  CompositeOperator
    curr_compose;

  Image
    *curr_image,    /* Image we are working with or iterating */
    *work_image,    /* secondary image for primitive iteration */
    *save_image,    /* saved image - for 'edge' method only */
    *rslt_image;    /* resultant image - after multi-kernel handling */

  KernelInfo
    *reflected_kernel, /* A reflected copy of the kernel (if needed) */
    *norm_kernel,      /* the current normal un-reflected kernel */
    *rflt_kernel,      /* the current reflected kernel (if needed) */
    *this_kernel;      /* the kernel being applied */

  MorphologyMethod
    primitive;      /* the current morphology primitive being applied */

  CompositeOperator
    rslt_compose;   /* multi-kernel compose method for results to use */

  MagickBooleanType
    special,        /* do we use a direct modify function? */
    verbose;        /* verbose output of results */

  size_t
    method_loop,    /* Loop 1: number of compound method iterations (norm 1) */
    method_limit,   /*         maximum number of compound method iterations */
    kernel_number,  /* Loop 2: the kernel number being applied */
    stage_loop,     /* Loop 3: primitive loop for compound morphology */
    stage_limit,    /*         how many primitives are in this compound */
    kernel_loop,    /* Loop 4: iterate the kernel over image */
    kernel_limit,   /*         number of times to iterate kernel */
    count,          /* total count of primitive steps applied */
    kernel_changed, /* total count of changed using iterated kernel */
    method_changed; /* total count of changed over method iteration */

  ssize_t
    changed;        /* number pixels changed by last primitive operation */

  char
    v_info[MagickPathExtent];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(kernel != (KernelInfo *) NULL);
  assert(kernel->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);

  count = 0;      /* number of low-level morphology primitives performed */
  if ( iterations == 0 )
    return((Image *) NULL);   /* null operation - nothing to do! */

  kernel_limit = (size_t) iterations;
  if ( iterations < 0 )  /* negative interations = infinite (well alomst) */
     kernel_limit = image->columns>image->rows ? image->columns : image->rows;

  verbose = IsStringTrue(GetImageArtifact(image,"debug"));

  /* initialise for cleanup */
  curr_image = (Image *) image;
  curr_compose = image->compose;
  (void) curr_compose;
  work_image = save_image = rslt_image = (Image *) NULL;
  reflected_kernel = (KernelInfo *) NULL;

  /* Initialize specific methods
   * + which loop should use the given iteratations
   * + how many primitives make up the compound morphology
   * + multi-kernel compose method to use (by default)
   */
  method_limit = 1;       /* just do method once, unless otherwise set */
  stage_limit = 1;        /* assume method is not a compound */
  special = MagickFalse;   /* assume it is NOT a direct modify primitive */
  rslt_compose = compose; /* and we are composing multi-kernels as given */
  switch( method ) {
    case SmoothMorphology:  /* 4 primitive compound morphology */
      stage_limit = 4;
      break;
    case OpenMorphology:    /* 2 primitive compound morphology */
    case OpenIntensityMorphology:
    case TopHatMorphology:
    case CloseMorphology:
    case CloseIntensityMorphology:
    case BottomHatMorphology:
    case EdgeMorphology:
      stage_limit = 2;
      break;
    case HitAndMissMorphology:
      rslt_compose = LightenCompositeOp;  /* Union of multi-kernel results */
      /* FALL THUR */
    case ThinningMorphology:
    case ThickenMorphology:
      method_limit = kernel_limit;  /* iterate the whole method */
      kernel_limit = 1;             /* do not do kernel iteration  */
      break;
    case DistanceMorphology:
    case VoronoiMorphology:
      special = MagickTrue;         /* use special direct primative */
      break;
    default:
      break;
  }

  /* Apply special methods with special requirments
  ** For example, single run only, or post-processing requirements
  */
  if ( special != MagickFalse )
    {
      rslt_image=CloneImage(image,0,0,MagickTrue,exception);
      if (rslt_image == (Image *) NULL)
        goto error_cleanup;
      if (SetImageStorageClass(rslt_image,DirectClass,exception) == MagickFalse)
        goto error_cleanup;

      changed=MorphologyPrimitiveDirect(rslt_image,method,kernel,exception);

      if (verbose != MagickFalse)
        (void) (void) FormatLocaleFile(stderr,
          "%s:%.20g.%.20g #%.20g => Changed %.20g\n",
          CommandOptionToMnemonic(MagickMorphologyOptions, method),
          1.0,0.0,1.0, (double) changed);

      if ( changed < 0 )
        goto error_cleanup;

      if ( method == VoronoiMorphology ) {
        /* Preserve the alpha channel of input image - but turned it off */
        (void) SetImageAlphaChannel(rslt_image, DeactivateAlphaChannel,
          exception);
        (void) CompositeImage(rslt_image,image,CopyAlphaCompositeOp,
          MagickTrue,0,0,exception);
        (void) SetImageAlphaChannel(rslt_image, DeactivateAlphaChannel,
          exception);
      }
      goto exit_cleanup;
    }

  /* Handle user (caller) specified multi-kernel composition method */
  if ( compose != UndefinedCompositeOp )
    rslt_compose = compose;  /* override default composition for method */
  if ( rslt_compose == UndefinedCompositeOp )
    rslt_compose = NoCompositeOp; /* still not defined! Then re-iterate */

  /* Some methods require a reflected kernel to use with primitives.
   * Create the reflected kernel for those methods. */
  switch ( method ) {
    case CorrelateMorphology:
    case CloseMorphology:
    case CloseIntensityMorphology:
    case BottomHatMorphology:
    case SmoothMorphology:
      reflected_kernel = CloneKernelInfo(kernel);
      if (reflected_kernel == (KernelInfo *) NULL)
        goto error_cleanup;
      RotateKernelInfo(reflected_kernel,180);
      break;
    default:
      break;
  }

  /* Loops around more primitive morpholgy methods
  **  erose, dilate, open, close, smooth, edge, etc...
  */
  /* Loop 1:  iterate the compound method */
  method_loop = 0;
  method_changed = 1;
  while ( method_loop < method_limit && method_changed > 0 ) {
    method_loop++;
    method_changed = 0;

    /* Loop 2:  iterate over each kernel in a multi-kernel list */
    norm_kernel = (KernelInfo *) kernel;
    this_kernel = (KernelInfo *) kernel;
    rflt_kernel = reflected_kernel;

    kernel_number = 0;
    while ( norm_kernel != NULL ) {

      /* Loop 3: Compound Morphology Staging - Select Primative to apply */
      stage_loop = 0;          /* the compound morphology stage number */
      while ( stage_loop < stage_limit ) {
        stage_loop++;   /* The stage of the compound morphology */

        /* Select primitive morphology for this stage of compound method */
        this_kernel = norm_kernel; /* default use unreflected kernel */
        primitive = method;        /* Assume method is a primitive */
        switch( method ) {
          case ErodeMorphology:      /* just erode */
          case EdgeInMorphology:     /* erode and image difference */
            primitive = ErodeMorphology;
            break;
          case DilateMorphology:     /* just dilate */
          case EdgeOutMorphology:    /* dilate and image difference */
            primitive = DilateMorphology;
            break;
          case OpenMorphology:       /* erode then dialate */
          case TopHatMorphology:     /* open and image difference */
            primitive = ErodeMorphology;
            if ( stage_loop == 2 )
              primitive = DilateMorphology;
            break;
          case OpenIntensityMorphology:
            primitive = ErodeIntensityMorphology;
            if ( stage_loop == 2 )
              primitive = DilateIntensityMorphology;
            break;
          case CloseMorphology:      /* dilate, then erode */
          case BottomHatMorphology:  /* close and image difference */
            this_kernel = rflt_kernel; /* use the reflected kernel */
            primitive = DilateMorphology;
            if ( stage_loop == 2 )
              primitive = ErodeMorphology;
            break;
          case CloseIntensityMorphology:
            this_kernel = rflt_kernel; /* use the reflected kernel */
            primitive = DilateIntensityMorphology;
            if ( stage_loop == 2 )
              primitive = ErodeIntensityMorphology;
            break;
          case SmoothMorphology:         /* open, close */
            switch ( stage_loop ) {
              case 1: /* start an open method, which starts with Erode */
                primitive = ErodeMorphology;
                break;
              case 2:  /* now Dilate the Erode */
                primitive = DilateMorphology;
                break;
              case 3:  /* Reflect kernel a close */
                this_kernel = rflt_kernel; /* use the reflected kernel */
                primitive = DilateMorphology;
                break;
              case 4:  /* Finish the Close */
                this_kernel = rflt_kernel; /* use the reflected kernel */
                primitive = ErodeMorphology;
                break;
            }
            break;
          case EdgeMorphology:        /* dilate and erode difference */
            primitive = DilateMorphology;
            if ( stage_loop == 2 ) {
              save_image = curr_image;      /* save the image difference */
              curr_image = (Image *) image;
              primitive = ErodeMorphology;
            }
            break;
          case CorrelateMorphology:
            /* A Correlation is a Convolution with a reflected kernel.
            ** However a Convolution is a weighted sum using a reflected
            ** kernel.  It may seem stange to convert a Correlation into a
            ** Convolution as the Correlation is the simplier method, but
            ** Convolution is much more commonly used, and it makes sense to
            ** implement it directly so as to avoid the need to duplicate the
            ** kernel when it is not required (which is typically the
            ** default).
            */
            this_kernel = rflt_kernel; /* use the reflected kernel */
            primitive = ConvolveMorphology;
            break;
          default:
            break;
        }
        assert( this_kernel != (KernelInfo *) NULL );

        /* Extra information for debugging compound operations */
        if (verbose != MagickFalse) {
          if ( stage_limit > 1 )
            (void) FormatLocaleString(v_info,MagickPathExtent,"%s:%.20g.%.20g -> ",
             CommandOptionToMnemonic(MagickMorphologyOptions,method),(double)
             method_loop,(double) stage_loop);
          else if ( primitive != method )
            (void) FormatLocaleString(v_info, MagickPathExtent, "%s:%.20g -> ",
              CommandOptionToMnemonic(MagickMorphologyOptions, method),(double)
              method_loop);
          else
            v_info[0] = '\0';
        }

        /* Loop 4: Iterate the kernel with primitive */
        kernel_loop = 0;
        kernel_changed = 0;
        changed = 1;
        while ( kernel_loop < kernel_limit && changed > 0 ) {
          kernel_loop++;     /* the iteration of this kernel */

          /* Create a clone as the destination image, if not yet defined */
          if ( work_image == (Image *) NULL )
            {
              work_image=CloneImage(image,0,0,MagickTrue,exception);
              if (work_image == (Image *) NULL)
                goto error_cleanup;
              if (SetImageStorageClass(work_image,DirectClass,exception) == MagickFalse)
                goto error_cleanup;
            }

          /* APPLY THE MORPHOLOGICAL PRIMITIVE (curr -> work) */
          count++;
          changed = MorphologyPrimitive(curr_image, work_image, primitive,
                       this_kernel, bias, exception);
          if (verbose != MagickFalse) {
            if ( kernel_loop > 1 )
              (void) FormatLocaleFile(stderr, "\n"); /* add end-of-line from previous */
            (void) (void) FormatLocaleFile(stderr,
              "%s%s%s:%.20g.%.20g #%.20g => Changed %.20g",
              v_info,CommandOptionToMnemonic(MagickMorphologyOptions,
              primitive),(this_kernel == rflt_kernel ) ? "*" : "",
              (double) (method_loop+kernel_loop-1),(double) kernel_number,
              (double) count,(double) changed);
          }
          if ( changed < 0 )
            goto error_cleanup;
          kernel_changed += changed;
          method_changed += changed;

          /* prepare next loop */
          { Image *tmp = work_image;   /* swap images for iteration */
            work_image = curr_image;
            curr_image = tmp;
          }
          if ( work_image == image )
            work_image = (Image *) NULL; /* replace input 'image' */

        } /* End Loop 4: Iterate the kernel with primitive */

        if (verbose != MagickFalse && kernel_changed != (size_t)changed)
          (void) FormatLocaleFile(stderr, "   Total %.20g",(double) kernel_changed);
        if (verbose != MagickFalse && stage_loop < stage_limit)
          (void) FormatLocaleFile(stderr, "\n"); /* add end-of-line before looping */

#if 0
    (void) FormatLocaleFile(stderr, "--E-- image=0x%lx\n", (unsigned long)image);
    (void) FormatLocaleFile(stderr, "      curr =0x%lx\n", (unsigned long)curr_image);
    (void) FormatLocaleFile(stderr, "      work =0x%lx\n", (unsigned long)work_image);
    (void) FormatLocaleFile(stderr, "      save =0x%lx\n", (unsigned long)save_image);
    (void) FormatLocaleFile(stderr, "      union=0x%lx\n", (unsigned long)rslt_image);
#endif

      } /* End Loop 3: Primative (staging) Loop for Coumpound Methods */

      /*  Final Post-processing for some Compound Methods
      **
      ** The removal of any 'Sync' channel flag in the Image Compositon
      ** below ensures the methematical compose method is applied in a
      ** purely mathematical way, and only to the selected channels.
      ** Turn off SVG composition 'alpha blending'.
      */
      switch( method ) {
        case EdgeOutMorphology:
        case EdgeInMorphology:
        case TopHatMorphology:
        case BottomHatMorphology:
          if (verbose != MagickFalse)
            (void) FormatLocaleFile(stderr,
              "\n%s: Difference with original image",CommandOptionToMnemonic(
              MagickMorphologyOptions, method) );
          (void) CompositeImage(curr_image,image,DifferenceCompositeOp,
            MagickTrue,0,0,exception);
          break;
        case EdgeMorphology:
          if (verbose != MagickFalse)
            (void) FormatLocaleFile(stderr,
              "\n%s: Difference of Dilate and Erode",CommandOptionToMnemonic(
              MagickMorphologyOptions, method) );
          (void) CompositeImage(curr_image,save_image,DifferenceCompositeOp,
            MagickTrue,0,0,exception);
          save_image = DestroyImage(save_image); /* finished with save image */
          break;
        default:
          break;
      }

      /* multi-kernel handling:  re-iterate, or compose results */
      if ( kernel->next == (KernelInfo *) NULL )
        rslt_image = curr_image;   /* just return the resulting image */
      else if ( rslt_compose == NoCompositeOp )
        { if (verbose != MagickFalse) {
            if ( this_kernel->next != (KernelInfo *) NULL )
              (void) FormatLocaleFile(stderr, " (re-iterate)");
            else
              (void) FormatLocaleFile(stderr, " (done)");
          }
          rslt_image = curr_image; /* return result, and re-iterate */
        }
      else if ( rslt_image == (Image *) NULL)
        { if (verbose != MagickFalse)
            (void) FormatLocaleFile(stderr, " (save for compose)");
          rslt_image = curr_image;
          curr_image = (Image *) image;  /* continue with original image */
        }
      else
        { /* Add the new 'current' result to the composition
          **
          ** The removal of any 'Sync' channel flag in the Image Compositon
          ** below ensures the methematical compose method is applied in a
          ** purely mathematical way, and only to the selected channels.
          ** IE: Turn off SVG composition 'alpha blending'.
          */
          if (verbose != MagickFalse)
            (void) FormatLocaleFile(stderr, " (compose \"%s\")",
              CommandOptionToMnemonic(MagickComposeOptions, rslt_compose) );
          (void) CompositeImage(rslt_image,curr_image,rslt_compose,MagickTrue,
            0,0,exception);
          curr_image = DestroyImage(curr_image);
          curr_image = (Image *) image;  /* continue with original image */
        }
      if (verbose != MagickFalse)
        (void) FormatLocaleFile(stderr, "\n");

      /* loop to the next kernel in a multi-kernel list */
      norm_kernel = norm_kernel->next;
      if ( rflt_kernel != (KernelInfo *) NULL )
        rflt_kernel = rflt_kernel->next;
      kernel_number++;
    } /* End Loop 2: Loop over each kernel */

  } /* End Loop 1: compound method interation */

  goto exit_cleanup;

  /* Yes goto's are bad, but it makes cleanup lot more efficient */
error_cleanup:
  if ( curr_image == rslt_image )
    curr_image = (Image *) NULL;
  if ( rslt_image != (Image *) NULL )
    rslt_image = DestroyImage(rslt_image);
exit_cleanup:
  if ( curr_image == rslt_image || curr_image == image )
    curr_image = (Image *) NULL;
  if ( curr_image != (Image *) NULL )
    curr_image = DestroyImage(curr_image);
  if ( work_image != (Image *) NULL )
    work_image = DestroyImage(work_image);
  if ( save_image != (Image *) NULL )
    save_image = DestroyImage(save_image);
  if ( reflected_kernel != (KernelInfo *) NULL )
    reflected_kernel = DestroyKernelInfo(reflected_kernel);
  return(rslt_image);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o r p h o l o g y I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MorphologyImage() applies a user supplied kernel to the image according to
%  the given mophology method.
%
%  This function applies any and all user defined settings before calling
%  the above internal function MorphologyApply().
%
%  User defined settings include...
%    * Output Bias for Convolution and correlation ("-define convolve:bias=??")
%    * Kernel Scale/normalize settings            ("-define convolve:scale=??")
%      This can also includes the addition of a scaled unity kernel.
%    * Show Kernel being applied            ("-define morphology:showKernel=1")
%
%  Other operators that do not want user supplied options interfering,
%  especially "convolve:bias" and "morphology:showKernel" should use
%  MorphologyApply() directly.
%
%  The format of the MorphologyImage method is:
%
%      Image *MorphologyImage(const Image *image,MorphologyMethod method,
%        const ssize_t iterations,KernelInfo *kernel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o method: the morphology method to be applied.
%
%    o iterations: apply the operation this many times (or no change).
%                  A value of -1 means loop until no change found.
%                  How this is applied may depend on the morphology method.
%                  Typically this is a value of 1.
%
%    o kernel: An array of double representing the morphology kernel.
%              Warning: kernel may be normalized for the Convolve method.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *MorphologyImage(const Image *image,
  const MorphologyMethod method,const ssize_t iterations,
  const KernelInfo *kernel,ExceptionInfo *exception)
{
  const char
    *artifact;

  CompositeOperator
    compose;

  double
    bias;

  Image
    *morphology_image;

  KernelInfo
    *curr_kernel;

  curr_kernel = (KernelInfo *) kernel;
  bias=0.0;
  compose = UndefinedCompositeOp;  /* use default for method */

  /* Apply Convolve/Correlate Normalization and Scaling Factors.
   * This is done BEFORE the ShowKernelInfo() function is called so that
   * users can see the results of the 'option:convolve:scale' option.
   */
  if ( method == ConvolveMorphology || method == CorrelateMorphology ) {
      /* Get the bias value as it will be needed */
      artifact = GetImageArtifact(image,"convolve:bias");
      if ( artifact != (const char *) NULL) {
        if (IsGeometry(artifact) == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
               OptionWarning,"InvalidSetting","'%s' '%s'",
               "convolve:bias",artifact);
        else
          bias=StringToDoubleInterval(artifact,(double) QuantumRange+1.0);
      }

      /* Scale kernel according to user wishes */
      artifact = GetImageArtifact(image,"convolve:scale");
      if ( artifact != (const char *) NULL ) {
        if (IsGeometry(artifact) == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
               OptionWarning,"InvalidSetting","'%s' '%s'",
               "convolve:scale",artifact);
        else {
          if ( curr_kernel == kernel )
            curr_kernel = CloneKernelInfo(kernel);
          if (curr_kernel == (KernelInfo *) NULL)
            return((Image *) NULL);
          ScaleGeometryKernelInfo(curr_kernel, artifact);
        }
      }
    }

  /* display the (normalized) kernel via stderr */
  artifact=GetImageArtifact(image,"morphology:showKernel");
  if (IsStringTrue(artifact) != MagickFalse)
    ShowKernelInfo(curr_kernel);

  /* Override the default handling of multi-kernel morphology results
   * If 'Undefined' use the default method
   * If 'None' (default for 'Convolve') re-iterate previous result
   * Otherwise merge resulting images using compose method given.
   * Default for 'HitAndMiss' is 'Lighten'.
   */
  {
    ssize_t
      parse;

    artifact = GetImageArtifact(image,"morphology:compose");
    if ( artifact != (const char *) NULL) {
      parse=ParseCommandOption(MagickComposeOptions,
        MagickFalse,artifact);
      if ( parse < 0 )
        (void) ThrowMagickException(exception,GetMagickModule(),
             OptionWarning,"UnrecognizedComposeOperator","'%s' '%s'",
             "morphology:compose",artifact);
      else
        compose=(CompositeOperator)parse;
    }
  }
  /* Apply the Morphology */
  morphology_image = MorphologyApply(image,method,iterations,
    curr_kernel,compose,bias,exception);

  /* Cleanup and Exit */
  if ( curr_kernel != kernel )
    curr_kernel=DestroyKernelInfo(curr_kernel);
  return(morphology_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     R o t a t e K e r n e l I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RotateKernelInfo() rotates the kernel by the angle given.
%
%  Currently it is restricted to 90 degree angles, of either 1D kernels
%  or square kernels. And 'circular' rotations of 45 degrees for 3x3 kernels.
%  It will ignore usless rotations for specific 'named' built-in kernels.
%
%  The format of the RotateKernelInfo method is:
%
%      void RotateKernelInfo(KernelInfo *kernel, double angle)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
%    o angle: angle to rotate in degrees
%
% This function is currently internal to this module only, but can be exported
% to other modules if needed.
*/
static void RotateKernelInfo(KernelInfo *kernel, double angle)
{
  /* angle the lower kernels first */
  if ( kernel->next != (KernelInfo *) NULL)
    RotateKernelInfo(kernel->next, angle);

  /* WARNING: Currently assumes the kernel (rightly) is horizontally symetrical
  **
  ** TODO: expand beyond simple 90 degree rotates, flips and flops
  */

  /* Modulus the angle */
  angle = fmod(angle, 360.0);
  if ( angle < 0 )
    angle += 360.0;

  if ( 337.5 < angle || angle <= 22.5 )
    return;   /* Near zero angle - no change! - At least not at this time */

  /* Handle special cases */
  switch (kernel->type) {
    /* These built-in kernels are cylindrical kernels, rotating is useless */
    case GaussianKernel:
    case DoGKernel:
    case LoGKernel:
    case DiskKernel:
    case PeaksKernel:
    case LaplacianKernel:
    case ChebyshevKernel:
    case ManhattanKernel:
    case EuclideanKernel:
      return;

    /* These may be rotatable at non-90 angles in the future */
    /* but simply rotating them in multiples of 90 degrees is useless */
    case SquareKernel:
    case DiamondKernel:
    case PlusKernel:
    case CrossKernel:
      return;

    /* These only allows a +/-90 degree rotation (by transpose) */
    /* A 180 degree rotation is useless */
    case BlurKernel:
      if ( 135.0 < angle && angle <= 225.0 )
        return;
      if ( 225.0 < angle && angle <= 315.0 )
        angle -= 180;
      break;

    default:
      break;
  }
  /* Attempt rotations by 45 degrees  -- 3x3 kernels only */
  if ( 22.5 < fmod(angle,90.0) && fmod(angle,90.0) <= 67.5 )
    {
      if ( kernel->width == 3 && kernel->height == 3 )
        { /* Rotate a 3x3 square by 45 degree angle */
          double t  = kernel->values[0];
          kernel->values[0] = kernel->values[3];
          kernel->values[3] = kernel->values[6];
          kernel->values[6] = kernel->values[7];
          kernel->values[7] = kernel->values[8];
          kernel->values[8] = kernel->values[5];
          kernel->values[5] = kernel->values[2];
          kernel->values[2] = kernel->values[1];
          kernel->values[1] = t;
          /* rotate non-centered origin */
          if ( kernel->x != 1 || kernel->y != 1 ) {
            ssize_t x,y;
            x = (ssize_t) kernel->x-1;
            y = (ssize_t) kernel->y-1;
                 if ( x == y  ) x = 0;
            else if ( x == 0  ) x = -y;
            else if ( x == -y ) y = 0;
            else if ( y == 0  ) y = x;
            kernel->x = (ssize_t) x+1;
            kernel->y = (ssize_t) y+1;
          }
          angle = fmod(angle+315.0, 360.0);  /* angle reduced 45 degrees */
          kernel->angle = fmod(kernel->angle+45.0, 360.0);
        }
      else
        perror("Unable to rotate non-3x3 kernel by 45 degrees");
    }
  if ( 45.0 < fmod(angle, 180.0)  && fmod(angle,180.0) <= 135.0 )
    {
      if ( kernel->width == 1 || kernel->height == 1 )
        { /* Do a transpose of a 1 dimensional kernel,
          ** which results in a fast 90 degree rotation of some type.
          */
          ssize_t
            t;
          t = (ssize_t) kernel->width;
          kernel->width = kernel->height;
          kernel->height = (size_t) t;
          t = kernel->x;
          kernel->x = kernel->y;
          kernel->y = t;
          if ( kernel->width == 1 ) {
            angle = fmod(angle+270.0, 360.0);     /* angle reduced 90 degrees */
            kernel->angle = fmod(kernel->angle+90.0, 360.0);
          } else {
            angle = fmod(angle+90.0, 360.0);   /* angle increased 90 degrees */
            kernel->angle = fmod(kernel->angle+270.0, 360.0);
          }
        }
      else if ( kernel->width == kernel->height )
        { /* Rotate a square array of values by 90 degrees */
          { register ssize_t
              i,j,x,y;

            register MagickRealType
              *k,t;

            k=kernel->values;
            for( i=0, x=(ssize_t) kernel->width-1;  i<=x;   i++, x--)
              for( j=0, y=(ssize_t) kernel->height-1;  j<y;   j++, y--)
                { t                    = k[i+j*kernel->width];
                  k[i+j*kernel->width] = k[j+x*kernel->width];
                  k[j+x*kernel->width] = k[x+y*kernel->width];
                  k[x+y*kernel->width] = k[y+i*kernel->width];
                  k[y+i*kernel->width] = t;
                }
          }
          /* rotate the origin - relative to center of array */
          { register ssize_t x,y;
            x = (ssize_t) (kernel->x*2-kernel->width+1);
            y = (ssize_t) (kernel->y*2-kernel->height+1);
            kernel->x = (ssize_t) ( -y +(ssize_t) kernel->width-1)/2;
            kernel->y = (ssize_t) ( +x +(ssize_t) kernel->height-1)/2;
          }
          angle = fmod(angle+270.0, 360.0);     /* angle reduced 90 degrees */
          kernel->angle = fmod(kernel->angle+90.0, 360.0);
        }
      else
        perror("Unable to rotate a non-square, non-linear kernel 90 degrees");
    }
  if ( 135.0 < angle && angle <= 225.0 )
    {
      /* For a 180 degree rotation - also know as a reflection
       * This is actually a very very common operation!
       * Basically all that is needed is a reversal of the kernel data!
       * And a reflection of the origon
       */
      MagickRealType
        t;

      register MagickRealType
        *k;

      ssize_t
        i,
        j;

      k=kernel->values;
      j=(ssize_t) (kernel->width*kernel->height-1);
      for (i=0;  i < j;  i++, j--)
        t=k[i],  k[i]=k[j],  k[j]=t;

      kernel->x = (ssize_t) kernel->width  - kernel->x - 1;
      kernel->y = (ssize_t) kernel->height - kernel->y - 1;
      angle = fmod(angle-180.0, 360.0);   /* angle+180 degrees */
      kernel->angle = fmod(kernel->angle+180.0, 360.0);
    }
  /* At this point angle should at least between -45 (315) and +45 degrees
   * In the future some form of non-orthogonal angled rotates could be
   * performed here, posibily with a linear kernel restriction.
   */

  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S c a l e G e o m e t r y K e r n e l I n f o                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ScaleGeometryKernelInfo() takes a geometry argument string, typically
%  provided as a  "-set option:convolve:scale {geometry}" user setting,
%  and modifies the kernel according to the parsed arguments of that setting.
%
%  The first argument (and any normalization flags) are passed to
%  ScaleKernelInfo() to scale/normalize the kernel.  The second argument
%  is then passed to UnityAddKernelInfo() to add a scled unity kernel
%  into the scaled/normalized kernel.
%
%  The format of the ScaleGeometryKernelInfo method is:
%
%      void ScaleGeometryKernelInfo(KernelInfo *kernel,
%        const double scaling_factor,const MagickStatusType normalize_flags)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel to modify
%
%    o geometry:
%             The geometry string to parse, typically from the user provided
%             "-set option:convolve:scale {geometry}" setting.
%
*/
MagickExport void ScaleGeometryKernelInfo (KernelInfo *kernel,
  const char *geometry)
{
  MagickStatusType
    flags;

  GeometryInfo
    args;

  SetGeometryInfo(&args);
  flags = ParseGeometry(geometry, &args);

#if 0
  /* For Debugging Geometry Input */
  (void) FormatLocaleFile(stderr, "Geometry = 0x%04X : %lg x %lg %+lg %+lg\n",
       flags, args.rho, args.sigma, args.xi, args.psi );
#endif

  if ( (flags & PercentValue) != 0 )      /* Handle Percentage flag*/
    args.rho *= 0.01,  args.sigma *= 0.01;

  if ( (flags & RhoValue) == 0 )          /* Set Defaults for missing args */
    args.rho = 1.0;
  if ( (flags & SigmaValue) == 0 )
    args.sigma = 0.0;

  /* Scale/Normalize the input kernel */
  ScaleKernelInfo(kernel, args.rho, (GeometryFlags) flags);

  /* Add Unity Kernel, for blending with original */
  if ( (flags & SigmaValue) != 0 )
    UnityAddKernelInfo(kernel, args.sigma);

  return;
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S c a l e K e r n e l I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ScaleKernelInfo() scales the given kernel list by the given amount, with or
%  without normalization of the sum of the kernel values (as per given flags).
%
%  By default (no flags given) the values within the kernel is scaled
%  directly using given scaling factor without change.
%
%  If either of the two 'normalize_flags' are given the kernel will first be
%  normalized and then further scaled by the scaling factor value given.
%
%  Kernel normalization ('normalize_flags' given) is designed to ensure that
%  any use of the kernel scaling factor with 'Convolve' or 'Correlate'
%  morphology methods will fall into -1.0 to +1.0 range.  Note that for
%  non-HDRI versions of IM this may cause images to have any negative results
%  clipped, unless some 'bias' is used.
%
%  More specifically.  Kernels which only contain positive values (such as a
%  'Gaussian' kernel) will be scaled so that those values sum to +1.0,
%  ensuring a 0.0 to +1.0 output range for non-HDRI images.
%
%  For Kernels that contain some negative values, (such as 'Sharpen' kernels)
%  the kernel will be scaled by the absolute of the sum of kernel values, so
%  that it will generally fall within the +/- 1.0 range.
%
%  For kernels whose values sum to zero, (such as 'Laplician' kernels) kernel
%  will be scaled by just the sum of the postive values, so that its output
%  range will again fall into the  +/- 1.0 range.
%
%  For special kernels designed for locating shapes using 'Correlate', (often
%  only containing +1 and -1 values, representing foreground/brackground
%  matching) a special normalization method is provided to scale the positive
%  values separately to those of the negative values, so the kernel will be
%  forced to become a zero-sum kernel better suited to such searches.
%
%  WARNING: Correct normalization of the kernel assumes that the '*_range'
%  attributes within the kernel structure have been correctly set during the
%  kernels creation.
%
%  NOTE: The values used for 'normalize_flags' have been selected specifically
%  to match the use of geometry options, so that '!' means NormalizeValue, '^'
%  means CorrelateNormalizeValue.  All other GeometryFlags values are ignored.
%
%  The format of the ScaleKernelInfo method is:
%
%      void ScaleKernelInfo(KernelInfo *kernel, const double scaling_factor,
%               const MagickStatusType normalize_flags )
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
%    o scaling_factor:
%             multiply all values (after normalization) by this factor if not
%             zero.  If the kernel is normalized regardless of any flags.
%
%    o normalize_flags:
%             GeometryFlags defining normalization method to use.
%             specifically: NormalizeValue, CorrelateNormalizeValue,
%                           and/or PercentValue
%
*/
MagickExport void ScaleKernelInfo(KernelInfo *kernel,
  const double scaling_factor,const GeometryFlags normalize_flags)
{
  register double
    pos_scale,
    neg_scale;

  register ssize_t
    i;

  /* do the other kernels in a multi-kernel list first */
  if ( kernel->next != (KernelInfo *) NULL)
    ScaleKernelInfo(kernel->next, scaling_factor, normalize_flags);

  /* Normalization of Kernel */
  pos_scale = 1.0;
  if ( (normalize_flags&NormalizeValue) != 0 ) {
    if ( fabs(kernel->positive_range + kernel->negative_range) >= MagickEpsilon )
      /* non-zero-summing kernel (generally positive) */
      pos_scale = fabs(kernel->positive_range + kernel->negative_range);
    else
      /* zero-summing kernel */
      pos_scale = kernel->positive_range;
  }
  /* Force kernel into a normalized zero-summing kernel */
  if ( (normalize_flags&CorrelateNormalizeValue) != 0 ) {
    pos_scale = ( fabs(kernel->positive_range) >= MagickEpsilon )
                 ? kernel->positive_range : 1.0;
    neg_scale = ( fabs(kernel->negative_range) >= MagickEpsilon )
                 ? -kernel->negative_range : 1.0;
  }
  else
    neg_scale = pos_scale;

  /* finialize scaling_factor for positive and negative components */
  pos_scale = scaling_factor/pos_scale;
  neg_scale = scaling_factor/neg_scale;

  for (i=0; i < (ssize_t) (kernel->width*kernel->height); i++)
    if (!IsNaN(kernel->values[i]))
      kernel->values[i] *= (kernel->values[i] >= 0) ? pos_scale : neg_scale;

  /* convolution output range */
  kernel->positive_range *= pos_scale;
  kernel->negative_range *= neg_scale;
  /* maximum and minimum values in kernel */
  kernel->maximum *= (kernel->maximum >= 0.0) ? pos_scale : neg_scale;
  kernel->minimum *= (kernel->minimum >= 0.0) ? pos_scale : neg_scale;

  /* swap kernel settings if user's scaling factor is negative */
  if ( scaling_factor < MagickEpsilon ) {
    double t;
    t = kernel->positive_range;
    kernel->positive_range = kernel->negative_range;
    kernel->negative_range = t;
    t = kernel->maximum;
    kernel->maximum = kernel->minimum;
    kernel->minimum = 1;
  }

  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S h o w K e r n e l I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ShowKernelInfo() outputs the details of the given kernel defination to
%  standard error, generally due to a users 'morphology:showKernel' option
%  request.
%
%  The format of the ShowKernel method is:
%
%      void ShowKernelInfo(const KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
*/
MagickPrivate void ShowKernelInfo(const KernelInfo *kernel)
{
  const KernelInfo
    *k;

  size_t
    c, i, u, v;

  for (c=0, k=kernel;  k != (KernelInfo *) NULL;  c++, k=k->next ) {

    (void) FormatLocaleFile(stderr, "Kernel");
    if ( kernel->next != (KernelInfo *) NULL )
      (void) FormatLocaleFile(stderr, " #%lu", (unsigned long) c );
    (void) FormatLocaleFile(stderr, " \"%s",
          CommandOptionToMnemonic(MagickKernelOptions, k->type) );
    if ( fabs(k->angle) >= MagickEpsilon )
      (void) FormatLocaleFile(stderr, "@%lg", k->angle);
    (void) FormatLocaleFile(stderr, "\" of size %lux%lu%+ld%+ld",(unsigned long)
      k->width,(unsigned long) k->height,(long) k->x,(long) k->y);
    (void) FormatLocaleFile(stderr,
          " with values from %.*lg to %.*lg\n",
          GetMagickPrecision(), k->minimum,
          GetMagickPrecision(), k->maximum);
    (void) FormatLocaleFile(stderr, "Forming a output range from %.*lg to %.*lg",
          GetMagickPrecision(), k->negative_range,
          GetMagickPrecision(), k->positive_range);
    if ( fabs(k->positive_range+k->negative_range) < MagickEpsilon )
      (void) FormatLocaleFile(stderr, " (Zero-Summing)\n");
    else if ( fabs(k->positive_range+k->negative_range-1.0) < MagickEpsilon )
      (void) FormatLocaleFile(stderr, " (Normalized)\n");
    else
      (void) FormatLocaleFile(stderr, " (Sum %.*lg)\n",
          GetMagickPrecision(), k->positive_range+k->negative_range);
    for (i=v=0; v < k->height; v++) {
      (void) FormatLocaleFile(stderr, "%2lu:", (unsigned long) v );
      for (u=0; u < k->width; u++, i++)
        if (IsNaN(k->values[i]))
          (void) FormatLocaleFile(stderr," %*s", GetMagickPrecision()+3, "nan");
        else
          (void) FormatLocaleFile(stderr," %*.*lg", GetMagickPrecision()+3,
              GetMagickPrecision(), (double) k->values[i]);
      (void) FormatLocaleFile(stderr,"\n");
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     U n i t y A d d K e r n a l I n f o                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnityAddKernelInfo() Adds a given amount of the 'Unity' Convolution Kernel
%  to the given pre-scaled and normalized Kernel.  This in effect adds that
%  amount of the original image into the resulting convolution kernel.  This
%  value is usually provided by the user as a percentage value in the
%  'convolve:scale' setting.
%
%  The resulting effect is to convert the defined kernels into blended
%  soft-blurs, unsharp kernels or into sharpening kernels.
%
%  The format of the UnityAdditionKernelInfo method is:
%
%      void UnityAdditionKernelInfo(KernelInfo *kernel, const double scale )
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
%    o scale:
%             scaling factor for the unity kernel to be added to
%             the given kernel.
%
*/
MagickExport void UnityAddKernelInfo(KernelInfo *kernel,
  const double scale)
{
  /* do the other kernels in a multi-kernel list first */
  if ( kernel->next != (KernelInfo *) NULL)
    UnityAddKernelInfo(kernel->next, scale);

  /* Add the scaled unity kernel to the existing kernel */
  kernel->values[kernel->x+kernel->y*kernel->width] += scale;
  CalcKernelMetaData(kernel);  /* recalculate the meta-data */

  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     Z e r o K e r n e l N a n s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ZeroKernelNans() replaces any special 'nan' value that may be present in
%  the kernel with a zero value.  This is typically done when the kernel will
%  be used in special hardware (GPU) convolution processors, to simply
%  matters.
%
%  The format of the ZeroKernelNans method is:
%
%      void ZeroKernelNans (KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
*/
MagickPrivate void ZeroKernelNans(KernelInfo *kernel)
{
  register size_t
    i;

  /* do the other kernels in a multi-kernel list first */
  if (kernel->next != (KernelInfo *) NULL)
    ZeroKernelNans(kernel->next);

  for (i=0; i < (kernel->width*kernel->height); i++)
    if (IsNaN(kernel->values[i]))
      kernel->values[i]=0.0;

  return;
}

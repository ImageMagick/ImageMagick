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
% Morpology is the the application of various kernels, of any size and even
% shape, to a image in various ways (typically binary, but not always).
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
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/cache-view.h"
#include "magick/color-private.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/hashmap.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor-private.h"
#include "magick/morphology.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/prepress.h"
#include "magick/quantize.h"
#include "magick/registry.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/token.h"

/*
  The following test is for special floating point numbers of value NaN (not
  a number), that may be used within a Kernel Definition.  NaN's are defined
  as part of the IEEE standard for floating point number representation.

  These are used a Kernel value of NaN means that that kernel position is not
  part of the normal convolution or morphology process, and thus allowing the
  use of 'shaped' kernels.

  Special properities two NaN's are never equal, even if they are from the
  same variable That is the IsNaN() macro is only true if the value is NaN.
*/
#define IsNan(a)   ((a)!=(a))

/*
  Other global definitions used by module.
*/
static inline double MagickMin(const double x,const double y)
{
  return( x < y ? x : y);
}
static inline double MagickMax(const double x,const double y)
{
  return( x > y ? x : y);
}
#define Minimize(assign,value) assign=MagickMin(assign,value)
#define Maximize(assign,value) assign=MagickMax(assign,value)

/* Currently these are only internal to this module */
static void
  ExpandKernelInfo(KernelInfo *, double),
  RotateKernelInfo(KernelInfo *, double);


/* Quick function to find last kernel in a kernel list */
static inline KernelInfo *LastKernelInfo(KernelInfo *kernel)
{
  while (kernel->next != (KernelInfo *) NULL)
    kernel = kernel->next;
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
%    "name:args"
%         Select from one of the built in kernels, using the name and
%         geometry arguments supplied.  See AcquireKernelBuiltIn()
%
%    "WxH[+X+Y]:num, num, num ..."
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
%  Note that 'name' kernels will start with an alphabetic character while the
%  new kernel specification has a ':' character in its specification string.
%  If neither is the case, it is assumed an old style of a simple list of
%  numbers generating a odd-sized square kernel has been given.
%
%  You can define a 'list of kernels' which can be used by some morphology
%  operators A list is defined as a semi-colon seperated list kernels.
%
%     " kernel ; kernel ; kernel ; "
%
%  Extra ';' characters are simply ignored.
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
    token[MaxTextExtent];

  const char
    *p,
    *end;

  register long
    i;

  double
    nan = sqrt((double)-1.0);  /* Special Value : Not A Number */

  kernel=(KernelInfo *) AcquireMagickMemory(sizeof(*kernel));
  if (kernel == (KernelInfo *)NULL)
    return(kernel);
  (void) ResetMagickMemory(kernel,0,sizeof(*kernel));
  kernel->minimum = kernel->maximum = 0.0;
  kernel->negative_range = kernel->positive_range = 0.0;
  kernel->type = UserDefinedKernel;
  kernel->next = (KernelInfo *) NULL;
  kernel->signature = MagickSignature;

  /* find end of this specific kernel definition string */
  end = strchr(kernel_string, ';');
  if ( end == (char *) NULL )
    end = strchr(kernel_string, '\0');

  /* Has a ':' in argument - New user kernel specification */
  p = strchr(kernel_string, ':');
  if ( p != (char *) NULL && p < end)
    {
      MagickStatusType
        flags;

      GeometryInfo
        args;

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
      kernel->width = (unsigned long)args.rho;
      kernel->height = (unsigned long)args.sigma;

      /* Offset Handling and Checks */
      if ( args.xi  < 0.0 || args.psi < 0.0 )
        return(DestroyKernelInfo(kernel));
      kernel->x = ((flags & XValue)!=0) ? (long)args.xi
                                               : (long) (kernel->width-1)/2;
      kernel->y = ((flags & YValue)!=0) ? (long)args.psi
                                               : (long) (kernel->height-1)/2;
      if ( kernel->x >= (long) kernel->width ||
           kernel->y >= (long) kernel->height )
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
        GetMagickToken(p,&p,token);
        if (*token == ',')
          GetMagickToken(p,&p,token);
      }
      /* set the size of the kernel - old sized square */
      kernel->width = kernel->height= (unsigned long) sqrt((double) i+1.0);
      kernel->x = kernel->y = (long) (kernel->width-1)/2;
      p=(const char *) kernel_string;
      while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == '\''))
        p++;  /* ignore "'" chars for convolve filter usage - Cristy */
    }

  /* Read in the kernel values from rest of input string argument */
  kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                        kernel->height*sizeof(double));
  if (kernel->values == (double *) NULL)
    return(DestroyKernelInfo(kernel));

  kernel->minimum = +MagickHuge;
  kernel->maximum = -MagickHuge;
  kernel->negative_range = kernel->positive_range = 0.0;

  for (i=0; (i < (long) (kernel->width*kernel->height)) && (p < end); i++)
  {
    GetMagickToken(p,&p,token);
    if (*token == ',')
      GetMagickToken(p,&p,token);
    if (    LocaleCompare("nan",token) == 0
        || LocaleCompare("-",token) == 0 ) {
      kernel->values[i] = nan; /* do not include this value in kernel */
    }
    else {
      kernel->values[i] = StringToDouble(token);
      ( kernel->values[i] < 0)
          ?  ( kernel->negative_range += kernel->values[i] )
          :  ( kernel->positive_range += kernel->values[i] );
      Minimize(kernel->minimum, kernel->values[i]);
      Maximize(kernel->maximum, kernel->values[i]);
    }
  }

  /* sanity check -- no more values in kernel definition */
  GetMagickToken(p,&p,token);
  if ( *token != '\0' && *token != ';' && *token != '\'' )
    return(DestroyKernelInfo(kernel));

#if 0
  /* this was the old method of handling a incomplete kernel */
  if ( i < (long) (kernel->width*kernel->height) ) {
    Minimize(kernel->minimum, kernel->values[i]);
    Maximize(kernel->maximum, kernel->values[i]);
    for ( ; i < (long) (kernel->width*kernel->height); i++)
      kernel->values[i]=0.0;
  }
#else
  /* Number of values for kernel was not enough - Report Error */
  if ( i < (long) (kernel->width*kernel->height) )
    return(DestroyKernelInfo(kernel));
#endif

  /* check that we recieved at least one real (non-nan) value! */
  if ( kernel->minimum == MagickHuge )
    return(DestroyKernelInfo(kernel));

  return(kernel);
}

static KernelInfo *ParseNamedKernel(const char *kernel_string)
{
  char
    token[MaxTextExtent];

  long
    type;

  const char
    *p,
    *end;

  MagickStatusType
    flags;

  GeometryInfo
    args;

  /* Parse special 'named' kernel */
  GetMagickToken(kernel_string,&p,token);
  type=ParseMagickOption(MagickKernelOptions,MagickFalse,token);
  if ( type < 0 || type == UserDefinedKernel )
    return((KernelInfo *)NULL);  /* not a valid named kernel */

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
  fprintf(stderr, "Geometry = %04x : %lf x %lf %+lf %+lf\n",
       flags, args.rho, args.sigma, args.xi, args.psi );
#endif

  /* special handling of missing values in input string */
  switch( type ) {
    case RectangleKernel:
      if ( (flags & WidthValue) == 0 ) /* if no width then */
        args.rho = args.sigma;         /* then  width = height */
      if ( args.rho < 1.0 )            /* if width too small */
          args.rho = 3;                 /* then  width = 3 */
      if ( args.sigma < 1.0 )          /* if height too small */
        args.sigma = args.rho;         /* then  height = width */
      if ( (flags & XValue) == 0 )     /* center offset if not defined */
        args.xi = (double)(((long)args.rho-1)/2);
      if ( (flags & YValue) == 0 )
        args.psi = (double)(((long)args.sigma-1)/2);
      break;
    case SquareKernel:
    case DiamondKernel:
    case DiskKernel:
    case PlusKernel:
    case CrossKernel:
      /* If no scale given (a 0 scale is valid! - set it to 1.0 */
      if ( (flags & HeightValue) == 0 )
        args.sigma = 1.0;
      break;
    case RingKernel:
      if ( (flags & XValue) == 0 )
        args.xi = 1.0;
      break;
    case ChebyshevKernel:
    case ManhattenKernel:
    case EuclideanKernel:
      if ( (flags & HeightValue) == 0 )
        args.sigma = 100.0;                    /* default distance scaling */
      else if ( (flags & AspectValue ) != 0 )  /* '!' flag */
        args.sigma = QuantumRange/args.sigma;  /* maximum pixel distance */
      else if ( (flags & PercentValue ) != 0 ) /* '%' flag */
        args.sigma *= QuantumRange/100.0;      /* percentage of color range */
      break;
    default:
      break;
  }

  return(AcquireKernelBuiltIn((KernelInfoType)type, &args));
}

MagickExport KernelInfo *AcquireKernelInfo(const char *kernel_string)
{

  KernelInfo
    *kernel,
    *new_kernel,
    *last_kernel;

  char
    token[MaxTextExtent];

  const char
    *p;

  unsigned long
    kernel_number;

  p = kernel_string;
  kernel = last_kernel = NULL;
  kernel_number = 0;

  while ( GetMagickToken(p,NULL,token),  *token != '\0' ) {

    /* ignore multiple ';' following each other */
    if ( *token != ';' ) {

      /* tokens starting with alpha is a Named kernel */
      if (isalpha((int) *token) == 0)
        new_kernel = ParseKernelArray(p);
      else /* otherwise a user defined kernel array */
        new_kernel = ParseNamedKernel(p);

      /* Error handling -- this is not proper error handling! */
      if ( new_kernel == (KernelInfo *) NULL ) {
        fprintf(stderr, "Failed to parse kernel number #%lu\n", kernel_number);
        if ( kernel != (KernelInfo *) NULL )
          kernel=DestroyKernelInfo(kernel);
        return((KernelInfo *) NULL);
      }

      /* initialise or append the kernel list */
      if ( kernel == (KernelInfo *) NULL )
        kernel = new_kernel;
      else
        last_kernel->next = new_kernel;
      last_kernel = LastKernelInfo(new_kernel);
    }

    /* look for the next kernel in list */
    p = strchr(p, ';');
    if ( p == (char *) NULL )
      break;
    p++;

  }
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
%    Gaussian:{radius},{sigma}
%       Generate a two-dimentional gaussian kernel, as used by -gaussian.
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
%    DOG:{radius},{sigma1},{sigma2}
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
%       each other, is equivelent to a far larger "Gaussian" kernel with the
%       same sigma value, However it is much faster to apply. This is how the
%       "-blur" operator actually works.
%
%    DOB:{radius},{sigma1},{sigma2}[,{angle}]
%        "Difference of Blurs" Kernel.
%        As "Blur" but with the 1D gaussian produced by 'sigma2' subtracted
%        from thethe 1D gaussian produced by 'sigma1'.
%        The result is a  zero-summing kernel.
%
%        This can be used to generate a faster "DOG" convolution, in the same
%        way "Blur" can.
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
%    # Still to be implemented...
%    #
%    # Filter2D
%    # Filter1D
%    #    Set kernel values using a resize filter, and given scale (sigma)
%    #    Cylindrical or Linear.   Is this posible with an image?
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
%      Generate Lapacian kernel of the type specified. (1 is the default)
%        Type 0 :  3x3 with center:8 surounded by -1  (8 neighbourhood)
%        Type 1 :  3x3 with center:4 edge:-1 corner:0 (4 neighbourhood)
%        Type 2 :  3x3 with center:4 edge:-2 corner:1
%        Type 3 :  3x3 with center:4 edge:1 corner:-2
%        Type 4 :  5x5 laplacian
%        Type 5 :  7x7 laplacian
%
%    Sobel:{angle}
%      Sobel 3x3 'Edge' convolution kernel (3x3)
%           -1, 0, 1
%           -2, 0,-2
%           -1, 0, 1
%    Roberts:{angle}
%      Roberts 3x3 convolution kernel (3x3)
%            0, 0, 0
%           -1, 1, 0
%            0, 0, 0
%    Compass:{angle}
%      Prewitt's "Compass" convolution kernel (3x3)
%           -1, 1, 1
%           -1,-2, 1
%           -1, 1, 1
%    Prewitt:{angle}
%      Prewitt Edge convolution kernel (3x3)
%           -1, 0, 1
%           -1, 0, 1
%           -1, 0, 1
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
%       Note that using a larger radius for the "Square" or the "Diamond" is
%       also equivelent to iterating the basic morphological method that many
%       times. However iterating with the smaller radius is actually faster
%       than using a larger kernel radius.
%
%    Rectangle:{geometry}
%       Simply generate a rectangle of 1's with the size given. You can also
%       specify the location of the 'control point', otherwise the closest
%       pixel to the center of the rectangle is selected.
%
%       Properly centered and odd sized rectangles work the best.
%
%    Disk:[{radius}[,{scale}]]
%       Generate a binary disk of the radius given, radius may be a float.
%       Kernel size will be ceil(radius)*2+1 square.
%       NOTE: Here are some disk shapes of specific interest
%          "Disk:1"    => "diamond" or "cross:1"
%          "Disk:1.5"  => "square"
%          "Disk:2"    => "diamond:2"
%          "Disk:2.5"  => a general disk shape of radius 2
%          "Disk:2.9"  => "square:2"
%          "Disk:3.5"  => default - octagonal/disk shape of radius 3
%          "Disk:4.2"  => roughly octagonal shape of radius 4
%          "Disk:4.3"  => a general disk shape of radius 4
%       After this all the kernel shape becomes more and more circular.
%
%       Because a "disk" is more circular when using a larger radius, using a
%       larger radius is preferred over iterating the morphological operation.
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
%       NOTE: "plus:1" is equivelent to a "Diamond" kernel.
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
%    Corners
%       Find corners of a binary shape
%    LineEnds
%       Find end points of lines (for pruning a skeletion)
%    LineJunctions
%       Find three line junctions (in a skeletion)
%    ConvexHull
%       Octagonal thicken kernel, to generate convex hulls of 45 degrees
%    Skeleton
%       Thinning kernel, which leaves behind a skeletion of a shape
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
%       Chebyshev Distance (also known as Tchebychev Distance) is a value of
%       one to any neighbour, orthogonal or diagonal. One why of thinking of
%       it is the number of squares a 'King' or 'Queen' in chess needs to
%       traverse reach any other position on a chess board.  It results in a
%       'square' like distance function, but one where diagonals are closer
%       than expected.
%
%    Manhatten:[{radius}][x{scale}[%!]]
%       Manhatten Distance (also known as Rectilinear Distance, or the Taxi
%       Cab metric), is the distance needed when you can only travel in
%       orthogonal (horizontal or vertical) only.  It is the distance a 'Rook'
%       in chess would travel. It results in a diamond like distances, where
%       diagonals are further than expected.
%
%    Euclidean:[{radius}][x{scale}[%!]]
%       Euclidean Distance is the 'direct' or 'as the crow flys distance.
%       However by default the kernel size only has a radius of 1, which
%       limits the distance to 'Knight' like moves, with only orthogonal and
%       diagonal measurements being correct.  As such for the default kernel
%       you will get octagonal like distance function, which is reasonally
%       accurate.
%
%       However if you use a larger radius such as "Euclidean:4" you will
%       get a much smoother distance gradient from the edge of the shape.
%       Of course a larger kernel is slower to use, and generally not needed.
%
%       To allow the use of fractional distances that you get with diagonals
%       the actual distance is scaled by a fixed value which the user can
%       provide.  This is not actually nessary for either ""Chebyshev" or
%       "Manhatten" distance kernels, but is done for all three distance
%       kernels.  If no scale is provided it is set to a value of 100,
%       allowing for a maximum distance measurement of 655 pixels using a Q16
%       version of IM, from any edge.  However for small images this can
%       result in quite a dark gradient.
%
*/

MagickExport KernelInfo *AcquireKernelBuiltIn(const KernelInfoType type,
   const GeometryInfo *args)
{
  KernelInfo
    *kernel;

  register long
    i;

  register long
    u,
    v;

  double
    nan = sqrt((double)-1.0);  /* Special Value : Not A Number */

  /* Generate a new empty kernel if needed */
  switch(type) {
    case GaussianKernel:
    case DOGKernel:
    case BlurKernel:
    case DOBKernel:
    case CometKernel:
    case DiamondKernel:
    case SquareKernel:
    case RectangleKernel:
    case DiskKernel:
    case PlusKernel:
    case CrossKernel:
    case RingKernel:
    case PeaksKernel:
    case ChebyshevKernel:
    case ManhattenKernel:
    case EuclideanKernel:
      kernel=(KernelInfo *) AcquireMagickMemory(sizeof(*kernel));
      if (kernel == (KernelInfo *) NULL)
        return(kernel);
      (void) ResetMagickMemory(kernel,0,sizeof(*kernel));
      kernel->minimum = kernel->maximum = 0.0;
      kernel->negative_range = kernel->positive_range = 0.0;
      kernel->type = type;
      kernel->next = (KernelInfo *) NULL;
      kernel->signature = MagickSignature;
    default:
      break;
  }

  switch(type) {
    /* Convolution Kernels */
    case GaussianKernel:
    case DOGKernel:
      { double
          sigma = fabs(args->sigma),
          sigma2 = fabs(args->xi),
          gamma;

        if ( args->rho >= 1.0 )
          kernel->width = (unsigned long)args->rho*2+1;
        else if ( (type == GaussianKernel) || (sigma >= sigma2) )
          kernel->width = GetOptimalKernelWidth2D(args->rho,sigma);
        else
          kernel->width = GetOptimalKernelWidth2D(args->rho,sigma2);
        kernel->height = kernel->width;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;
        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        /* Calculate a Positive Gaussian */
        if ( sigma > MagickEpsilon )
          { gamma = 1.0/(2.0*sigma*sigma); /* simplify loop expressions */
            sigma = 1.0/(MagickPI*sigma);
            for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
              for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
                  kernel->values[i] = sigma*exp(-((double)(u*u+v*v))*gamma);
          }
        else /* special case - generate a unity kernel */
          { (void) ResetMagickMemory(kernel->values,0, (size_t)
                         kernel->width*kernel->height*sizeof(double));
            kernel->values[kernel->x+kernel->y*kernel->width] = 1.0;
          }
        if ( type == DOGKernel )
          { /* Subtract a Negative Gaussian for "Difference of Gaussian" */
            if ( sigma2 > MagickEpsilon )
              { sigma = sigma2;                /* simplify loop expressions */
                gamma = 1.0/(2.0*sigma*sigma);
                sigma = 1.0/(MagickPI*sigma);
                for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
                  for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
                    kernel->values[i] -= sigma*exp(-((double)(u*u+v*v))*gamma);
              }
            else /* special case - subtract the unity kernel */
              kernel->values[kernel->x+kernel->y*kernel->width] -= 1.0;
          }
        /* Note the above kernel may have been 'clipped' by a user defined
        ** radius, producing a smaller (darker) kernel.  Also for very small
        ** sigma's (> 0.1) the central value becomes larger than one, and thus
        ** producing a very bright kernel.
        **
        ** Normalization will still be needed.
        */

        /* Work out the meta-data about kernel */
        kernel->minimum = kernel->maximum = 0.0;
        kernel->negative_range = kernel->positive_range = 0.0;
        u=(long) kernel->width*kernel->height;
        for ( i=0; i < u; i++)
          {
            if ( fabs(kernel->values[i]) < MagickEpsilon )
              kernel->values[i] = 0.0;
            ( kernel->values[i] < 0)
                ?  ( kernel->negative_range += kernel->values[i] )
                :  ( kernel->positive_range += kernel->values[i] );
            Minimize(kernel->minimum, kernel->values[i]);
            Maximize(kernel->maximum, kernel->values[i]);
          }
        /* Normalize the 2D Gaussian Kernel
        **
        ** NB: a CorrelateNormalize performs a normal Normalize if
        ** there are no negative values.
        */
        ScaleKernelInfo(kernel, 1.0, CorrelateNormalizeValue);

        break;
      }
    case BlurKernel:
    case DOBKernel:
      { double
          sigma = fabs(args->sigma),
          sigma2 = fabs(args->xi),
          gamma;

        if ( args->rho >= 1.0 )
          kernel->width = (unsigned long)args->rho*2+1;
        else if ( (type == BlurKernel) || (sigma >= sigma2) )
          kernel->width = GetOptimalKernelWidth1D(args->rho,sigma);
        else
          kernel->width = GetOptimalKernelWidth1D(args->rho,sigma2);
        kernel->height = 1;
        kernel->x = (long) (kernel->width-1)/2;
        kernel->y = 0;
        kernel->negative_range = kernel->positive_range = 0.0;
        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
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
        */

        /* initialize */
        v = (long) (kernel->width*KernelRank-1)/2; /* start/end points to fit range */
        /* Calculate a Positive 1D Gaussian */
        if ( sigma > MagickEpsilon )
          { sigma *= KernelRank;               /* simplify loop expressions */
            gamma = 1.0/(2.0*sigma*sigma);
            sigma = 1.0/(MagickSQ2PI*sigma );
            for ( u=-v; u <= v; u++) {
              kernel->values[(u+v)/KernelRank] +=
                                 sigma*exp(-((double)(u*u))*gamma);
            }
          }
        else /* special case - generate a unity kernel */
          kernel->values[kernel->x+kernel->y*kernel->width] = 1.0;
        if ( type == DOBKernel )
          { /* Subtract a Second 1D Gaussian for "Difference of Blur" */
            if ( sigma2 > MagickEpsilon )
              { sigma = sigma2*KernelRank;      /* simplify loop expressions */
                gamma = 1.0/(2.0*sigma*sigma);
                sigma = 1.0/(MagickSQ2PI*sigma );
                for ( u=-v; u <= v; u++)
                  kernel->values[(u+v)/KernelRank] -=
                                 sigma*exp(-((double)(u*u))*gamma);
              }
            else /* special case - subtract a unity kernel */
              kernel->values[kernel->x+kernel->y*kernel->width] -= 1.0;
          }
#else
        /* Direct calculation without curve averaging */

        /* Calculate a Positive Gaussian */
        if ( sigma > MagickEpsilon )
          { gamma = 1.0/(2.0*sigma*sigma);  /* simplify loop expressions */
            sigma = 1.0/(MagickSQ2PI*sigma);
            for ( i=0, u=-kernel->x; u <= (long)kernel->x; u++, i++)
              kernel->values[i] = sigma*exp(-((double)(u*u))*gamma);
          }
        else /* special case - generate a unity kernel */
          { (void) ResetMagickMemory(kernel->values,0, (size_t)
                         kernel->width*kernel->height*sizeof(double));
            kernel->values[kernel->x+kernel->y*kernel->width] = 1.0;
          }
        if ( type == DOBKernel )
          { /* Subtract a Second 1D Gaussian for "Difference of Blur" */
            if ( sigma2 > MagickEpsilon )
              { sigma = sigma2;                /* simplify loop expressions */
                gamma = 1.0/(2.0*sigma*sigma);
                sigma = 1.0/(MagickSQ2PI*sigma);
                for ( i=0, u=-kernel->x; u <= (long)kernel->x; u++, i++)
                  kernel->values[i] -= sigma*exp(-((double)(u*u))*gamma);
              }
            else /* special case - subtract a unity kernel */
              kernel->values[kernel->x+kernel->y*kernel->width] -= 1.0;
          }
#endif
        /* Note the above kernel may have been 'clipped' by a user defined
        ** radius, producing a smaller (darker) kernel.  Also for very small
        ** sigma's (> 0.1) the central value becomes larger than one, and thus
        ** producing a very bright kernel.
        **
        ** Normalization will still be needed.
        */

        /* Work out the meta-data about kernel */
        for ( i=0; i < (long) kernel->width; i++)
          {
            if ( fabs(kernel->values[i]) < MagickEpsilon )
              kernel->values[i] = 0.0;
            ( kernel->values[i] < 0)
                ?  ( kernel->negative_range += kernel->values[i] )
                :  ( kernel->positive_range += kernel->values[i] );
            Minimize(kernel->minimum, kernel->values[i]);
            Maximize(kernel->maximum, kernel->values[i]);
          }

        /* Normalize the 1D Gaussian Kernel
        **
        ** NB: a CorrelateNormalize performs a normal Normalize if
        ** there are no negative values.
        */
        ScaleKernelInfo(kernel, 1.0, CorrelateNormalizeValue);

        /* rotate the 1D kernel by given angle */
        RotateKernelInfo(kernel, (type == BlurKernel) ? args->xi : args->psi );
        break;
      }
    case CometKernel:
      { double
          sigma = fabs(args->sigma);

        sigma = (sigma <= MagickEpsilon) ? 1.0 : sigma;
        if ( args->rho < 1.0 )
          kernel->width = GetOptimalKernelWidth1D(args->rho,sigma);
        else
          kernel->width = (unsigned long)args->rho;
        kernel->x = kernel->y = 0;
        kernel->height = 1;
        kernel->negative_range = kernel->positive_range = 0.0;
        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        /* A comet blur is half a 1D gaussian curve, so that the object is
        ** blurred in one direction only.  This may not be quite the right
        ** curve to use so may change in the future. The function must be
        ** normalised after generation, which also resolves any clipping.
        **
        ** As we are normalizing and not subtracting gaussians,
        ** there is no need for a divisor in the gaussian formula
        **
        */
#if 1
#define KernelRank 3
        sigma *= KernelRank;                /* simplify expanded curve */
        v = (long) kernel->width*KernelRank; /* start/end points to fit range */
        (void) ResetMagickMemory(kernel->values,0, (size_t)
                       kernel->width*sizeof(double));
        for ( u=0; u < v; u++) {
          kernel->values[u/KernelRank] +=
               exp(-((double)(u*u))/(2.0*sigma*sigma))
                       /*   / (MagickSQ2PI*sigma/KernelRank)  */ ;
        }
        for (i=0; i < (long) kernel->width; i++)
          kernel->positive_range += kernel->values[i];
#else
        for ( i=0; i < (long) kernel->width; i++)
          kernel->positive_range += (
            kernel->values[i] =
               exp(-((double)(i*i))/(2.0*sigma*sigma))
                       /*  / (MagickSQ2PI*sigma)  */ );
#endif
        kernel->minimum = 0;
        kernel->maximum = kernel->values[0];

        ScaleKernelInfo(kernel, 1.0, NormalizeValue); /* Normalize */
        RotateKernelInfo(kernel, args->xi); /* Rotate by angle */
        break;
      }

    /* Convolution Kernels - Well Known Constants */
    case LaplacianKernel:
      {
        switch ( (int) args->rho ) {
          case 0:
          default: /* default laplacian 'edge' filter */
            kernel=ParseKernelArray("3: -1,-1,-1  -1,8,-1  -1,-1,-1");
            break;
          case 1:
            kernel=ParseKernelArray("3: 0,-1,0  -1,4,-1  0,-1,0");
            break;
          case 2:
            kernel=ParseKernelArray("3: 1,-2,1  -2,4,-2  1,-2,1");
            break;
          case 3:   /* a 5x5 laplacian */
            kernel=ParseKernelArray(
              "5: -4,-1,0,-1,-4 -1,2,3,2,-1  0,3,4,3,0 -1,2,3,2,-1  -4,-1,0,-1,-4");
            break;
          case 4:   /* a 7x7 laplacian */
            kernel=ParseKernelArray(
              "7:-10,-5,-2,-1,-2,-5,-10 -5,0,3,4,3,0,-5 -2,3,6,7,6,3,-2 -1,4,7,8,7,4,-1 -2,3,6,7,6,3,-2 -5,0,3,4,3,0,-5 -10,-5,-2,-1,-2,-5,-10" );
            break;
        }
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        break;
      }
    case SobelKernel:
      {
        kernel=ParseKernelArray("3: -1,0,1  -2,0,2  -1,0,1");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        RotateKernelInfo(kernel, args->rho); /* Rotate by angle */
        break;
      }
    case RobertsKernel:
      {
        kernel=ParseKernelArray("3: 0,0,0  -1,1,0  0,0,0");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        RotateKernelInfo(kernel, args->rho); /* Rotate by angle */
        break;
      }
    case PrewittKernel:
      {
        kernel=ParseKernelArray("3: -1,1,1  0,0,0  -1,1,1");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        RotateKernelInfo(kernel, args->rho); /* Rotate by angle */
        break;
      }
    case CompassKernel:
      {
        kernel=ParseKernelArray("3: -1,1,1  -1,-2,1  -1,1,1");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        RotateKernelInfo(kernel, args->rho); /* Rotate by angle */
        break;
      }
    /* Boolean Kernels */
    case DiamondKernel:
      {
        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
          kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        /* set all kernel values within diamond area to scale given */
        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            if ((labs(u)+labs(v)) <= (long)kernel->x)
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
              kernel->width = kernel->height = (unsigned long) (2*args->rho+1);
            kernel->x = kernel->y = (long) (kernel->width-1)/2;
            scale = args->sigma;
          }
        else {
            /* NOTE: user defaults set in "AcquireKernelInfo()" */
            if ( args->rho < 1.0 || args->sigma < 1.0 )
              return(DestroyKernelInfo(kernel));    /* invalid args given */
            kernel->width = (unsigned long)args->rho;
            kernel->height = (unsigned long)args->sigma;
            if ( args->xi  < 0.0 || args->xi  > (double)kernel->width ||
                 args->psi < 0.0 || args->psi > (double)kernel->height )
              return(DestroyKernelInfo(kernel));    /* invalid args given */
            kernel->x = (long) args->xi;
            kernel->y = (long) args->psi;
            scale = 1.0;
          }
        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        /* set all kernel values to scale given */
        u=(long) kernel->width*kernel->height;
        for ( i=0; i < u; i++)
            kernel->values[i] = scale;
        kernel->minimum = kernel->maximum = scale;   /* a flat shape */
        kernel->positive_range = scale*u;
        break;
      }
    case DiskKernel:
      {
        long limit = (long)(args->rho*args->rho);
        if (args->rho < 0.1)             /* default radius approx 3.5 */
          kernel->width = kernel->height = 7L, limit = 10L;
        else
           kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        /* set all kernel values within disk area to scale given */
        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
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
           kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        /* set all kernel values along axises to given scale */
        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
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
           kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        /* set all kernel values along axises to given scale */
        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            kernel->values[i] = (u == v || u == -v) ? args->sigma : nan;
        kernel->minimum = kernel->maximum = args->sigma;   /* a flat shape */
        kernel->positive_range = args->sigma*(kernel->width*2.0 - 1.0);
        break;
      }
    /* HitAndMiss Kernels */
    case RingKernel:
    case PeaksKernel:
      {
        long
          limit1,
          limit2,
          scale;

        if (args->rho < args->sigma)
          {
            kernel->width = ((unsigned long)args->sigma)*2+1;
            limit1 = (long)args->rho*args->rho;
            limit2 = (long)args->sigma*args->sigma;
          }
        else
          {
            kernel->width = ((unsigned long)args->rho)*2+1;
            limit1 = (long)args->sigma*args->sigma;
            limit2 = (long)args->rho*args->rho;
          }
        if ( limit2 <= 0 )
          kernel->width = 7L, limit1 = 7L, limit2 = 11L;

        kernel->height = kernel->width;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;
        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        /* set a ring of points of 'scale' ( 0.0 for PeaksKernel ) */
        scale = ( type == PeaksKernel) ? 0.0 : args->xi;
        for ( i=0, v= -kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            { long radius=u*u+v*v;
              if (limit1 < radius && radius <= limit2)
                kernel->positive_range += kernel->values[i] = scale;
              else
                kernel->values[i] = nan;
            }
        kernel->minimum = kernel->minimum = scale;
        if ( type == PeaksKernel ) {
          /* set the central point in the middle */
          kernel->values[kernel->x+kernel->y*kernel->width] = 1.0;
          kernel->positive_range = 1.0;
          kernel->maximum = 1.0;
        }
        break;
      }
    case CornersKernel:
      {
        kernel=ParseKernelArray("3x3: 0,0,-  0,1,1  -,1,-");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        ExpandKernelInfo(kernel, 90.0); /* Create a list of 4 rotated kernels */
        break;
      }
    case LineEndsKernel:
      {
        kernel=ParseKernelArray("3x3: 0,-,-  0,1,0  0,0,0");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        ExpandKernelInfo(kernel, 45.0); /* Create a list of 8 rotated kernels */
        break;
      }
    case LineJunctionsKernel:
      {
        KernelInfo
          *new_kernel;
        /* first set of 4 kernels */
        kernel=ParseKernelArray("3x3: -,1,-  -,1,-  1,-,1");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        ExpandKernelInfo(kernel, 90.0);
        /* append second set of 4 kernels */
        new_kernel=ParseKernelArray("3x3: 1,-,-  -,1,-  1,-,1");
        if (new_kernel == (KernelInfo *) NULL)
          return(DestroyKernelInfo(kernel));
        kernel->type = type;
        ExpandKernelInfo(new_kernel, 90.0);
        LastKernelInfo(kernel)->next = new_kernel;
        /* append Thrid set of 4 kernels */
        new_kernel=ParseKernelArray("3x3: -,1,-  -,1,1  1,-,-");
        if (new_kernel == (KernelInfo *) NULL)
          return(DestroyKernelInfo(kernel));
        kernel->type = type;
        ExpandKernelInfo(new_kernel, 90.0);
        LastKernelInfo(kernel)->next = new_kernel;
        break;
      }
    case ConvexHullKernel:
      {
        KernelInfo
          *new_kernel;
        /* first set of 4 kernels */
        kernel=ParseKernelArray("3x3: 1,1,-  1,0,-  1,-,0");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        ExpandKernelInfo(kernel, 90.0);
        /* append second set of 4 kernels */
        new_kernel=ParseKernelArray("3x3: -,1,1  -,0,1  0,-,1");
        if (new_kernel == (KernelInfo *) NULL)
          return(DestroyKernelInfo(kernel));
        kernel->type = type;
        ExpandKernelInfo(new_kernel, 90.0);
        LastKernelInfo(kernel)->next = new_kernel;
        break;
      }
    case SkeletonKernel:
      {
        KernelInfo
          *new_kernel;
        /* first set of 4 kernels - corners */
        kernel=ParseKernelArray("3x3: 0,0,-  0,1,1  -,1,-");
        if (kernel == (KernelInfo *) NULL)
          return(kernel);
        kernel->type = type;
        ExpandKernelInfo(kernel, 90);
        /* append second set of 4 kernels - edge middles */
        new_kernel=ParseKernelArray("3x3: 0,0,0  -,1,-  1,1,1");
        if (new_kernel == (KernelInfo *) NULL)
          return(DestroyKernelInfo(kernel));
        kernel->type = type;
        ExpandKernelInfo(new_kernel, 90);
        LastKernelInfo(kernel)->next = new_kernel;
        break;
      }
    /* Distance Measuring Kernels */
    case ChebyshevKernel:
      {
        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
          kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            kernel->positive_range += ( kernel->values[i] =
                 args->sigma*((labs(u)>labs(v)) ? labs(u) : labs(v)) );
        kernel->maximum = kernel->values[0];
        break;
      }
    case ManhattenKernel:
      {
        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
           kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            kernel->positive_range += ( kernel->values[i] =
                 args->sigma*(labs(u)+labs(v)) );
        kernel->maximum = kernel->values[0];
        break;
      }
    case EuclideanKernel:
      {
        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
           kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            kernel->positive_range += ( kernel->values[i] =
                 args->sigma*sqrt((double)(u*u+v*v)) );
        kernel->maximum = kernel->values[0];
        break;
      }
    default:
      {
        /* Generate a No-Op minimal kernel - 1x1 pixel */
        kernel=ParseKernelArray("1");
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
  register long
    i;

  KernelInfo
    *new_kernel;

  assert(kernel != (KernelInfo *) NULL);
  new_kernel=(KernelInfo *) AcquireMagickMemory(sizeof(*kernel));
  if (new_kernel == (KernelInfo *) NULL)
    return(new_kernel);
  *new_kernel=(*kernel); /* copy values in structure */

  /* replace the values with a copy of the values */
  new_kernel->values=(double *) AcquireQuantumMemory(kernel->width,
    kernel->height*sizeof(double));
  if (new_kernel->values == (double *) NULL)
    return(DestroyKernelInfo(new_kernel));
  for (i=0; i < (long) (kernel->width*kernel->height); i++)
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

  if ( kernel->next != (KernelInfo *) NULL )
    kernel->next = DestroyKernelInfo(kernel->next);

  kernel->values = (double *)RelinquishMagickMemory(kernel->values);
  kernel = (KernelInfo *) RelinquishMagickMemory(kernel);
  return(kernel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E x p a n d K e r n e l I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExpandKernelInfo() takes a single kernel, and expands it into a list
%  of kernels each incrementally rotated the angle given.
%
%  WARNING: 45 degree rotations only works for 3x3 kernels.
%  While 90 degree roatations only works for linear and square kernels
%
%  The format of the RotateKernelInfo method is:
%
%      void ExpandKernelInfo(KernelInfo *kernel, double angle)
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
static void ExpandKernelInfo(KernelInfo *kernel, double angle)
{
  KernelInfo
    *clone,
    *last;

  double
    a;

  last = kernel;
  for (a=angle; a<355.0; a+=angle) {
    clone = CloneKernelInfo(last);
    RotateKernelInfo(clone, angle);
    last->next = clone;
    last = clone;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o r p h o l o g y I m a g e C h a n n e l                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MorphologyImageChannel() applies a user supplied kernel to the image
%  according to the given mophology method.
%
%  The given kernel is assumed to have been pre-scaled appropriatally, usally
%  by the kernel generator.
%
%  The format of the MorphologyImage method is:
%
%      Image *MorphologyImage(const Image *image,MorphologyMethod method,
%        const long iterations,KernelInfo *kernel,ExceptionInfo *exception)
%      Image *MorphologyImageChannel(const Image *image, const ChannelType
%        channel,MorphologyMethod method,const long iterations,
%        KernelInfo *kernel,ExceptionInfo *exception)
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
%    o channel: the channel type.
%
%    o kernel: An array of double representing the morphology kernel.
%              Warning: kernel may be normalized for the Convolve method.
%
%    o exception: return any errors or warnings in this structure.
%
%
% TODO: bias and auto-scale handling of the kernel for convolution
%     The given kernel is assumed to have been pre-scaled appropriatally, usally
%     by the kernel generator.
%
*/


/* Internal function
 * Apply the low-level Morphology Method Primatives using the given Kernel
 * Returning the number of pixels that changed.
 * Two pre-created images must be provided, no image is created.
 */
static unsigned long MorphologyPrimative(const Image *image, Image
     *result_image, const MorphologyMethod method, const ChannelType channel,
     const KernelInfo *kernel, ExceptionInfo *exception)
{
#define MorphologyTag  "Morphology/Image"

  long
    progress,
    y, offx, offy,
    changed;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  CacheView
    *p_view,
    *q_view;

  /* Only the most basic morphology is actually performed by this routine */

  /*
    Apply Basic Morphology to Image.
  */
  status=MagickTrue;
  changed=0;
  progress=0;

  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);
  /* Future: handle auto-bias from user, based on kernel input */

  p_view=AcquireCacheView(image);
  q_view=AcquireCacheView(result_image);

  /* Some methods (including convolve) needs use a reflected kernel.
   * Adjust 'origin' offsets for this reflected kernel.
   */
  offx = kernel->x;
  offy = kernel->y;
  switch(method) {
    case ConvolveMorphology:
    case DilateMorphology:
    case DilateIntensityMorphology:
    case DistanceMorphology:
      /* kernel needs to used with reflection about origin */
      offx = (long) kernel->width-offx-1;
      offy = (long) kernel->height-offy-1;
      break;
    case ErodeMorphology:
    case ErodeIntensityMorphology:
    case HitAndMissMorphology:
    case ThinningMorphology:
    case ThickenMorphology:
      /* kernel is user as is, without reflection */
      break;
    default:
      perror("Not a low level Morphology Method");
      break;
  }

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const PixelPacket
      *restrict p;

    register const IndexPacket
      *restrict p_indexes;

    register PixelPacket
      *restrict q;

    register IndexPacket
      *restrict q_indexes;

    register long
      x;

    unsigned long
      r;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(p_view, -offx,  y-offy,
         image->columns+kernel->width,  kernel->height,  exception);
    q=GetCacheViewAuthenticPixels(q_view,0,y,result_image->columns,1,
         exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    p_indexes=GetCacheViewVirtualIndexQueue(p_view);
    q_indexes=GetCacheViewAuthenticIndexQueue(q_view);
    r = (image->columns+kernel->width)*offy+offx; /* constant */

    for (x=0; x < (long) image->columns; x++)
    {
       long
        v;

      register long
        u;

      register const double
        *restrict k;

      register const PixelPacket
        *restrict k_pixels;

      register const IndexPacket
        *restrict k_indexes;

      MagickPixelPacket
        result,
        min,
        max;

      /* Copy input to ouput image for unused channels
       * This removes need for 'cloning' a new image every iteration
       */
      *q = p[r];
      if (image->colorspace == CMYKColorspace)
        q_indexes[x] = p_indexes[r];

      /* Defaults */
      min.red     =
      min.green   =
      min.blue    =
      min.opacity =
      min.index   = (MagickRealType) QuantumRange;
      max.red     =
      max.green   =
      max.blue    =
      max.opacity =
      max.index   = (MagickRealType) 0;
      /* original pixel value */
      result.red     = (MagickRealType) p[r].red;
      result.green   = (MagickRealType) p[r].green;
      result.blue    = (MagickRealType) p[r].blue;
      result.opacity = QuantumRange - (MagickRealType) p[r].opacity;
      result.index   = 0;
      if ( image->colorspace == CMYKColorspace)
         result.index   = (MagickRealType) p_indexes[r];

      switch (method) {
        case ConvolveMorphology:
          /* Set the user defined bias of the weighted average output
          **
          ** FUTURE: provide some way for internal functions to disable
          ** user provided bias and scaling effects.
          */
          result=bias;
          break;
        case DilateIntensityMorphology:
        case ErodeIntensityMorphology:
          result.red = 0.0;  /* flag indicating when first match found */
          break;
        default:
          break;
      }

      switch ( method ) {
        case ConvolveMorphology:
            /* Weighted Average of pixels using reflected kernel
            **
            ** NOTE for correct working of this operation for asymetrical
            ** kernels, the kernel needs to be applied in its reflected form.
            ** That is its values needs to be reversed.
            **
            ** Correlation is actually the same as this but without reflecting
            ** the kernel, and thus 'lower-level' that Convolution.  However
            ** as Convolution is the more common method used, and it does not
            ** really cost us much in terms of processing to use a reflected
            ** kernel, so it is Convolution that is implemented.
            **
            ** Correlation will have its kernel reflected before calling
            ** this function to do a Convolve.
            **
            ** For more details of Correlation vs Convolution see
            **   http://www.cs.umd.edu/~djacobs/CMSC426/Convolution.pdf
            */
            if (((channel & SyncChannels) != 0 ) &&
                      (image->matte == MagickTrue))
              { /* Channel has a 'Sync' Flag, and Alpha Channel enabled.
                ** Weight the color channels with Alpha Channel so that
                ** transparent pixels are not part of the results.
                */
                MagickRealType
                  alpha,  /* color channel weighting : kernel*alpha  */
                  gamma;  /* divisor, sum of weighting values */

                gamma=0.0;
                k = &kernel->values[ kernel->width*kernel->height-1 ];
                k_pixels = p;
                k_indexes = p_indexes;
                for (v=0; v < (long) kernel->height; v++) {
                  for (u=0; u < (long) kernel->width; u++, k--) {
                    if ( IsNan(*k) ) continue;
                    alpha=(*k)*(QuantumScale*(QuantumRange-
                                          k_pixels[u].opacity));
                    gamma += alpha;
                    result.red     += alpha*k_pixels[u].red;
                    result.green   += alpha*k_pixels[u].green;
                    result.blue    += alpha*k_pixels[u].blue;
                    result.opacity += (*k)*(QuantumRange-k_pixels[u].opacity);
                    if ( image->colorspace == CMYKColorspace)
                      result.index   += alpha*k_indexes[u];
                  }
                  k_pixels += image->columns+kernel->width;
                  k_indexes += image->columns+kernel->width;
                }
                gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
                result.red *= gamma;
                result.green *= gamma;
                result.blue *= gamma;
                result.opacity *= gamma;
                result.index *= gamma;
              }
            else
              {
                /* No 'Sync' flag, or no Alpha involved.
                ** Convolution is simple individual channel weigthed sum.
                */
                k = &kernel->values[ kernel->width*kernel->height-1 ];
                k_pixels = p;
                k_indexes = p_indexes;
                for (v=0; v < (long) kernel->height; v++) {
                  for (u=0; u < (long) kernel->width; u++, k--) {
                    if ( IsNan(*k) ) continue;
                    result.red     += (*k)*k_pixels[u].red;
                    result.green   += (*k)*k_pixels[u].green;
                    result.blue    += (*k)*k_pixels[u].blue;
                    result.opacity += (*k)*(QuantumRange-k_pixels[u].opacity);
                    if ( image->colorspace == CMYKColorspace)
                      result.index   += (*k)*k_indexes[u];
                  }
                  k_pixels += image->columns+kernel->width;
                  k_indexes += image->columns+kernel->width;
                }
              }
            break;

        case ErodeMorphology:
            /* Minimum Value within kernel neighbourhood
            **
            ** NOTE that the kernel is not reflected for this operation!
            **
            ** NOTE: in normal Greyscale Morphology, the kernel value should
            ** be added to the real value, this is currently not done, due to
            ** the nature of the boolean kernels being used.
            */
            k = kernel->values;
            k_pixels = p;
            k_indexes = p_indexes;
            for (v=0; v < (long) kernel->height; v++) {
              for (u=0; u < (long) kernel->width; u++, k++) {
                if ( IsNan(*k) || (*k) < 0.5 ) continue;
                Minimize(min.red,     (double) k_pixels[u].red);
                Minimize(min.green,   (double) k_pixels[u].green);
                Minimize(min.blue,    (double) k_pixels[u].blue);
                Minimize(min.opacity,
                            QuantumRange-(double) k_pixels[u].opacity);
                if ( image->colorspace == CMYKColorspace)
                  Minimize(min.index,   (double) k_indexes[u]);
              }
              k_pixels += image->columns+kernel->width;
              k_indexes += image->columns+kernel->width;
            }
            break;


        case DilateMorphology:
            /* Maximum Value within kernel neighbourhood
            **
            ** NOTE for correct working of this operation for asymetrical
            ** kernels, the kernel needs to be applied in its reflected form.
            ** That is its values needs to be reversed.
            **
            ** NOTE: in normal Greyscale Morphology, the kernel value should
            ** be added to the real value, this is currently not done, due to
            ** the nature of the boolean kernels being used.
            **
            */
            k = &kernel->values[ kernel->width*kernel->height-1 ];
            k_pixels = p;
            k_indexes = p_indexes;
            for (v=0; v < (long) kernel->height; v++) {
              for (u=0; u < (long) kernel->width; u++, k--) {
                if ( IsNan(*k) || (*k) < 0.5 ) continue;
                Maximize(max.red,     (double) k_pixels[u].red);
                Maximize(max.green,   (double) k_pixels[u].green);
                Maximize(max.blue,    (double) k_pixels[u].blue);
                Maximize(max.opacity,
                            QuantumRange-(double) k_pixels[u].opacity);
                if ( image->colorspace == CMYKColorspace)
                  Maximize(max.index,   (double) k_indexes[u]);
              }
              k_pixels += image->columns+kernel->width;
              k_indexes += image->columns+kernel->width;
            }
            break;

        case HitAndMissMorphology:
        case ThinningMorphology:
        case ThickenMorphology:
            /* Minimum of Foreground Pixel minus Maxumum of Background Pixels
            **
            ** NOTE that the kernel is not reflected for this operation,
            ** and consists of both foreground and background pixel
            ** neighbourhoods, 0.0 for background, and 1.0 for foreground
            ** with either Nan or 0.5 values for don't care.
            **
            ** Note that this can produce negative results, though really
            ** only a positive match has any real value.
            */
            k = kernel->values;
            k_pixels = p;
            k_indexes = p_indexes;
            for (v=0; v < (long) kernel->height; v++) {
              for (u=0; u < (long) kernel->width; u++, k++) {
                if ( IsNan(*k) ) continue;
                if ( (*k) > 0.7 )
                { /* minimim of foreground pixels */
                  Minimize(min.red,     (double) k_pixels[u].red);
                  Minimize(min.green,   (double) k_pixels[u].green);
                  Minimize(min.blue,    (double) k_pixels[u].blue);
                  Minimize(min.opacity,
                              QuantumRange-(double) k_pixels[u].opacity);
                  if ( image->colorspace == CMYKColorspace)
                    Minimize(min.index,   (double) k_indexes[u]);
                }
                else if ( (*k) < 0.3 )
                { /* maximum of background pixels */
                  Maximize(max.red,     (double) k_pixels[u].red);
                  Maximize(max.green,   (double) k_pixels[u].green);
                  Maximize(max.blue,    (double) k_pixels[u].blue);
                  Maximize(max.opacity,
                              QuantumRange-(double) k_pixels[u].opacity);
                  if ( image->colorspace == CMYKColorspace)
                    Maximize(max.index,   (double) k_indexes[u]);
                }
              }
              k_pixels += image->columns+kernel->width;
              k_indexes += image->columns+kernel->width;
            }
            /* Pattern Match  only if min fg larger than min bg pixels */
            min.red     -= max.red;     Maximize( min.red,     0.0 );
            min.green   -= max.green;   Maximize( min.green,   0.0 );
            min.blue    -= max.blue;    Maximize( min.blue,    0.0 );
            min.opacity -= max.opacity; Maximize( min.opacity, 0.0 );
            min.index   -= max.index;   Maximize( min.index,   0.0 );
            break;

        case ErodeIntensityMorphology:
            /* Select Pixel with Minimum Intensity within kernel neighbourhood
            **
            ** WARNING: the intensity test fails for CMYK and does not
            ** take into account the moderating effect of teh alpha channel
            ** on the intensity.
            **
            ** NOTE that the kernel is not reflected for this operation!
            */
            k = kernel->values;
            k_pixels = p;
            k_indexes = p_indexes;
            for (v=0; v < (long) kernel->height; v++) {
              for (u=0; u < (long) kernel->width; u++, k++) {
                if ( IsNan(*k) || (*k) < 0.5 ) continue;
                if ( result.red == 0.0 ||
                     PixelIntensity(&(k_pixels[u])) < PixelIntensity(q) ) {
                  /* copy the whole pixel - no channel selection */
                  *q = k_pixels[u];
                  if ( result.red > 0.0 ) changed++;
                  result.red = 1.0;
                }
              }
              k_pixels += image->columns+kernel->width;
              k_indexes += image->columns+kernel->width;
            }
            break;

        case DilateIntensityMorphology:
            /* Select Pixel with Maximum Intensity within kernel neighbourhood
            **
            ** WARNING: the intensity test fails for CMYK and does not
            ** take into account the moderating effect of teh alpha channel
            ** on the intensity.
            **
            ** NOTE for correct working of this operation for asymetrical
            ** kernels, the kernel needs to be applied in its reflected form.
            ** That is its values needs to be reversed.
            */
            k = &kernel->values[ kernel->width*kernel->height-1 ];
            k_pixels = p;
            k_indexes = p_indexes;
            for (v=0; v < (long) kernel->height; v++) {
              for (u=0; u < (long) kernel->width; u++, k--) {
                if ( IsNan(*k) || (*k) < 0.5 ) continue; /* boolean kernel */
                if ( result.red == 0.0 ||
                     PixelIntensity(&(k_pixels[u])) > PixelIntensity(q) ) {
                  /* copy the whole pixel - no channel selection */
                  *q = k_pixels[u];
                  if ( result.red > 0.0 ) changed++;
                  result.red = 1.0;
                }
              }
              k_pixels += image->columns+kernel->width;
              k_indexes += image->columns+kernel->width;
            }
            break;


        case DistanceMorphology:
            /* Add kernel Value and select the minimum value found.
            ** The result is a iterative distance from edge of image shape.
            **
            ** All Distance Kernels are symetrical, but that may not always
            ** be the case. For example how about a distance from left edges?
            ** To work correctly with asymetrical kernels the reflected kernel
            ** needs to be applied.
            **
            ** Actually this is really a GreyErode with a negative kernel!
            **
            */
            k = &kernel->values[ kernel->width*kernel->height-1 ];
            k_pixels = p;
            k_indexes = p_indexes;
            for (v=0; v < (long) kernel->height; v++) {
              for (u=0; u < (long) kernel->width; u++, k--) {
                if ( IsNan(*k) ) continue;
                Minimize(result.red,     (*k)+k_pixels[u].red);
                Minimize(result.green,   (*k)+k_pixels[u].green);
                Minimize(result.blue,    (*k)+k_pixels[u].blue);
                Minimize(result.opacity, (*k)+QuantumRange-k_pixels[u].opacity);
                if ( image->colorspace == CMYKColorspace)
                  Minimize(result.index,   (*k)+k_indexes[u]);
              }
              k_pixels += image->columns+kernel->width;
              k_indexes += image->columns+kernel->width;
            }
            break;

        case UndefinedMorphology:
        default:
            break; /* Do nothing */
      }
      /* Final mathematics of results (combine with original image?)
      **
      ** NOTE: Difference Morphology operators Edge* and *Hat could also
      ** be done here but works better with iteration as a image difference
      ** in the controling function (below).  Thicken and Thinning however
      ** should be done here so thay can be iterated correctly.
      */
      switch ( method ) {
        case HitAndMissMorphology:
        case ErodeMorphology:
          result = min;    /* minimum of neighbourhood */
          break;
        case DilateMorphology:
          result = max;    /* maximum of neighbourhood */
          break;
        case ThinningMorphology:
          /* subtract pattern match from original */
          result.red     -= min.red;
          result.green   -= min.green;
          result.blue    -= min.blue;
          result.opacity -= min.opacity;
          result.index   -= min.index;
          break;
        case ThickenMorphology:
          /* Union with original image (maximize) - or should this be + */
          Maximize( result.red,     min.red );
          Maximize( result.green,   min.green );
          Maximize( result.blue,    min.blue );
          Maximize( result.opacity, min.opacity );
          Maximize( result.index,   min.index );
          break;
        default:
          /* result directly calculated or assigned */
          break;
      }
      /* Assign the resulting pixel values - Clamping Result */
      switch ( method ) {
        case UndefinedMorphology:
        case DilateIntensityMorphology:
        case ErodeIntensityMorphology:
          break;  /* full pixel was directly assigned - not a channel method */
        default:
          if ((channel & RedChannel) != 0)
            q->red = ClampToQuantum(result.red);
          if ((channel & GreenChannel) != 0)
            q->green = ClampToQuantum(result.green);
          if ((channel & BlueChannel) != 0)
            q->blue = ClampToQuantum(result.blue);
          if ((channel & OpacityChannel) != 0
              && image->matte == MagickTrue )
            q->opacity = ClampToQuantum(QuantumRange-result.opacity);
          if ((channel & IndexChannel) != 0
              && image->colorspace == CMYKColorspace)
            q_indexes[x] = ClampToQuantum(result.index);
          break;
      }
      /* Count up changed pixels */
      if (   ( p[r].red != q->red )
          || ( p[r].green != q->green )
          || ( p[r].blue != q->blue )
          || ( p[r].opacity != q->opacity )
          || ( image->colorspace == CMYKColorspace &&
                  p_indexes[r] != q_indexes[x] ) )
        changed++;  /* The pixel had some value changed! */
      p++;
      q++;
    } /* x */
    sync=SyncCacheViewAuthenticPixels(q_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_MorphologyImage)
#endif
        proceed=SetImageProgress(image,MorphologyTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  } /* y */
  result_image->type=image->type;
  q_view=DestroyCacheView(q_view);
  p_view=DestroyCacheView(p_view);
  return(status ? (unsigned long) changed : 0);
}


MagickExport Image *MorphologyImage(const Image *image, const MorphologyMethod
  method, const long iterations,const KernelInfo *kernel, ExceptionInfo
  *exception)
{
  Image
    *morphology_image;

  morphology_image=MorphologyImageChannel(image,DefaultChannels,method,
    iterations,kernel,exception);
  return(morphology_image);
}


MagickExport Image *MorphologyImageChannel(const Image *image,
  const ChannelType channel,const MorphologyMethod method,
  const long iterations,const KernelInfo *kernel,ExceptionInfo *exception)
{
  Image
    *new_image,
    *old_image,
    *grad_image;

  KernelInfo
    *curr_kernel,
    *this_kernel;

  MorphologyMethod
    curr_method;

  unsigned long
    count,
    limit,
    changed,
    total_changed,
    kernel_number;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(kernel != (KernelInfo *) NULL);
  assert(kernel->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  if ( iterations == 0 )
    return((Image *)NULL); /* null operation - nothing to do! */

  /* kernel must be valid at this point
   * (except maybe for posible future morphology methods like "Prune"
   */
  assert(kernel != (KernelInfo *)NULL);

  new_image  = (Image *) NULL;          /* for cleanup */
  old_image  = (Image *) NULL;
  grad_image = (Image *) NULL;
  curr_kernel = (KernelInfo *) NULL;
  count = 0;                           /* interation count */
  changed = 1;                         /* was last run succesfull */
  curr_kernel = (KernelInfo *)kernel;  /* allow kernel and method */
  curr_method = method;                /* to be changed as nessary */

  limit = (unsigned long) iterations;
  if ( iterations < 0 )
    limit = image->columns > image->rows ? image->columns : image->rows;

  /* Third-level morphology methods */
  switch( curr_method ) {
    case EdgeMorphology:
      grad_image = MorphologyImageChannel(image, channel,
            DilateMorphology, iterations, curr_kernel, exception);
      if ( grad_image == (Image *) NULL )
        goto error_cleanup;
      /* FALL-THRU */
    case EdgeInMorphology:
      curr_method = ErodeMorphology;
      break;
    case EdgeOutMorphology:
      curr_method = DilateMorphology;
      break;
    case TopHatMorphology:
      curr_method = OpenMorphology;
      break;
    case BottomHatMorphology:
      curr_method = CloseMorphology;
      break;
    default:
      break; /* not a third-level method */
  }

  /* Second-level morphology methods */
  switch( curr_method ) {
    case OpenMorphology:
      /* Open is a Erode then a Dilate without reflection */
      new_image = MorphologyImageChannel(image, channel,
            ErodeMorphology, iterations, curr_kernel, exception);
      if (new_image == (Image *) NULL)
        goto error_cleanup;
      curr_method = DilateMorphology;
      break;
    case OpenIntensityMorphology:
      new_image = MorphologyImageChannel(image, channel,
            ErodeIntensityMorphology, iterations, curr_kernel, exception);
      if (new_image == (Image *) NULL)
        goto error_cleanup;
      curr_method = DilateIntensityMorphology;
      break;

    case CloseMorphology:
      /* Close is a Dilate then Erode using reflected kernel */
      /* A reflected kernel is needed for a Close */
      if ( curr_kernel == kernel )
        curr_kernel = CloneKernelInfo(kernel);
      if (curr_kernel == (KernelInfo *) NULL)
        goto error_cleanup;
      RotateKernelInfo(curr_kernel,180);
      new_image = MorphologyImageChannel(image, channel,
            DilateMorphology, iterations, curr_kernel, exception);
      if (new_image == (Image *) NULL)
        goto error_cleanup;
      curr_method = ErodeMorphology;
      break;
    case CloseIntensityMorphology:
      /* A reflected kernel is needed for a Close */
      if ( curr_kernel == kernel )
        curr_kernel = CloneKernelInfo(kernel);
      if (curr_kernel == (KernelInfo *) NULL)
        goto error_cleanup;
      RotateKernelInfo(curr_kernel,180);
      new_image = MorphologyImageChannel(image, channel,
            DilateIntensityMorphology, iterations, curr_kernel, exception);
      if (new_image == (Image *) NULL)
        goto error_cleanup;
      curr_method = ErodeIntensityMorphology;
      break;

    case CorrelateMorphology:
      /* A Correlation is actually a Convolution with a reflected kernel.
      ** However a Convolution is a weighted sum with a reflected kernel.
      ** It may seem stange to convert a Correlation into a Convolution
      ** as the Correleation is the simplier method, but Convolution is
      ** much more commonly used, and it makes sense to implement it directly
      ** so as to avoid the need to duplicate the kernel when it is not
      ** required (which is typically the default).
      */
      if ( curr_kernel == kernel )
        curr_kernel = CloneKernelInfo(kernel);
      if (curr_kernel == (KernelInfo *) NULL)
        goto error_cleanup;
      RotateKernelInfo(curr_kernel,180);
      curr_method = ConvolveMorphology;
      /* FALL-THRU into Correlate (weigthed sum without reflection) */

    case ConvolveMorphology:
      { /* Scale or Normalize kernel, according to user wishes
        ** before using it for the Convolve/Correlate method.
        **
        ** FUTURE: provide some way for internal functions to disable
        ** user bias and scaling effects.
        */
        const char
          *artifact = GetImageArtifact(image,"convolve:scale");
        if ( artifact != (char *)NULL ) {
          GeometryFlags
            flags;
          GeometryInfo
            args;

          if ( curr_kernel == kernel )
            curr_kernel = CloneKernelInfo(kernel);
          if (curr_kernel == (KernelInfo *) NULL)
            goto error_cleanup;
          args.rho = 1.0;
          flags = (GeometryFlags) ParseGeometry(artifact, &args);
          ScaleKernelInfo(curr_kernel, args.rho, flags);
        }
      }
      /* FALL-THRU to do the first, and typically the only iteration */

    default:
      /* Do a single iteration using the Low-Level Morphology method!
      ** This ensures a "new_image" has been generated, but allows us to skip
      ** the creation of 'old_image' if no more iterations are needed.
      **
      ** The "curr_method" should also be set to a low-level method that is
      ** understood by the MorphologyPrimative() internal function.
      */
      new_image=CloneImage(image,0,0,MagickTrue,exception);
      if (new_image == (Image *) NULL)
        goto error_cleanup;
      if (SetImageStorageClass(new_image,DirectClass) == MagickFalse)
        {
          InheritException(exception,&new_image->exception);
          goto exit_cleanup;
        }
      count++;  /* iteration count */
      changed = MorphologyPrimative(image,new_image,curr_method,channel,
           curr_kernel, exception);
      if ( GetImageArtifact(image,"verbose") != (const char *) NULL )
        fprintf(stderr, "Morphology %s:%lu.%lu => Changed %lu\n",
              MagickOptionToMnemonic(MagickMorphologyOptions, curr_method),
              count, 0L, changed);
      break;
  }
  /* At this point
   *   + "curr_method" should be set to a low-level morphology method
   *   + "count=1" if the first iteration of the first kernel has been done.
   *   + "new_image" is defined and contains the current resulting image
   */

  /* The Hit-And-Miss Morphology is not 'iterated' against the resulting
  ** image from the previous morphology step.  It must always be applied
  ** to the original image, with the results collected into a union (maximimum
  ** or lighten) of all the results.  Multiple kernels may be applied but
  ** an iteration of the morphology does nothing, so is ignored.
  **
  ** The first kernel is guranteed to have been done, so lets continue the
  ** process, with the other kernels in the kernel list.
  */
  if ( method == HitAndMissMorphology )
  {
    if ( curr_kernel->next != (KernelInfo *) NULL ) {
      /* create a second working image */
      old_image = CloneImage(image,0,0,MagickTrue,exception);
      if (old_image == (Image *) NULL)
        goto error_cleanup;
      if (SetImageStorageClass(old_image,DirectClass) == MagickFalse)
        {
          InheritException(exception,&old_image->exception);
          goto exit_cleanup;
        }

      /* loop through rest of the kernels */
      this_kernel=curr_kernel->next;
      kernel_number=1;
      while( this_kernel != (KernelInfo *) NULL )
        {
          changed = MorphologyPrimative(image,old_image,curr_method,channel,
              this_kernel,exception);
          (void) CompositeImageChannel(new_image,
            (ChannelType) (channel & ~SyncChannels), LightenCompositeOp,
            old_image, 0, 0);
          if ( GetImageArtifact(image,"verbose") != (const char *) NULL )
            fprintf(stderr, "Morphology %s:%lu.%lu => Changed %lu\n",
                  MagickOptionToMnemonic(MagickMorphologyOptions, curr_method),
                  count, kernel_number, changed);
          this_kernel = this_kernel->next;
          kernel_number++;
        }
      old_image=DestroyImage(old_image);
    }
    goto exit_cleanup;
  }

  /* Repeat the low-level morphology over all kernels
     until iteration count limit or no change from any kernel is found */
  if ( ( count < limit && changed > 0 ) ||
       curr_kernel->next != (KernelInfo *) NULL ) {

    /* create a second working image */
    old_image = CloneImage(image,0,0,MagickTrue,exception);
    if (old_image == (Image *) NULL)
      goto error_cleanup;
    if (SetImageStorageClass(old_image,DirectClass) == MagickFalse)
      {
        InheritException(exception,&old_image->exception);
        goto exit_cleanup;
      }

    /* reset variables for the first/next iteration, or next kernel) */
    kernel_number = 0;
    this_kernel = curr_kernel;
    total_changed = count != 0 ? changed : 0;
    if ( count != 0 && this_kernel != (KernelInfo *) NULL ) {
      count = 0;  /* first iteration is not yet finished! */
      this_kernel = curr_kernel->next;
      kernel_number = 1;
      total_changed = changed;
    }

    while ( count < limit ) {
      count++;
      while ( this_kernel != (KernelInfo *) NULL ) {
        Image *tmp = old_image;
        old_image = new_image;
        new_image = tmp;
        changed = MorphologyPrimative(old_image,new_image,curr_method,channel,
                          this_kernel,exception);
        if ( GetImageArtifact(image,"verbose") != (const char *) NULL )
          fprintf(stderr, "Morphology %s:%lu.%lu => Changed %lu\n",
                MagickOptionToMnemonic(MagickMorphologyOptions, curr_method),
                count, kernel_number, changed);
        total_changed += changed;
        this_kernel = this_kernel->next;
        kernel_number++;
      }
      if ( kernel_number > 1 )
        if ( GetImageArtifact(image,"verbose") != (const char *) NULL )
          fprintf(stderr, "Morphology %s:%lu ===> Total Changed %lu\n",
                MagickOptionToMnemonic(MagickMorphologyOptions, curr_method),
                count, total_changed);
      if ( total_changed == 0 )
        break;  /* no changes after processing all kernels - ABORT */
      /* prepare for next loop */
      total_changed = 0;
      kernel_number = 0;
      this_kernel = curr_kernel;
    }
    old_image=DestroyImage(old_image);
  }

  /* finished with kernel - destroy any copy that was made */
  if ( curr_kernel != kernel )
    curr_kernel=DestroyKernelInfo(curr_kernel);

  /* Third-level Subtractive methods post-processing
  **
  ** The removal of any 'Sync' channel flag in the Image Compositon below
  ** ensures the compose method is applied in a purely mathematical way, only
  ** the selected channels, without any normal 'alpha blending' normally
  ** associated with the compose method.
  **
  ** Note "method" here is the 'original' morphological method, and not the
  ** 'current' morphological method used above to generate "new_image".
  */
  switch( method ) {
    case EdgeOutMorphology:
    case EdgeInMorphology:
    case TopHatMorphology:
    case BottomHatMorphology:
      /* Get Difference relative to the original image */
      (void) CompositeImageChannel(new_image, (ChannelType) (channel & ~SyncChannels),
           DifferenceCompositeOp, image, 0, 0);
      break;
    case EdgeMorphology:
      /* Difference the Eroded image from the saved Dilated image */
      (void) CompositeImageChannel(new_image, (ChannelType) (channel & ~SyncChannels),
           DifferenceCompositeOp, grad_image, 0, 0);
      grad_image=DestroyImage(grad_image);
      break;
    default:
      break;
  }

  return(new_image);

  /* Yes goto's are bad, but in this case it makes cleanup lot more efficient */
error_cleanup:
  if ( new_image != (Image *) NULL )
    DestroyImage(new_image);
exit_cleanup:
  if ( old_image != (Image *) NULL )
    DestroyImage(old_image);
  if ( curr_kernel != (KernelInfo *) NULL &&  curr_kernel != kernel )
    curr_kernel=DestroyKernelInfo(curr_kernel);
  return(new_image);
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
%  RotateKernelInfo() rotates the kernel by the angle given.  Currently it is
%  restricted to 90 degree angles, but this may be improved in the future.
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
% This function is only internel to this module, as it is not finalized,
% especially with regard to non-orthogonal angles, and rotation of larger
% 2D kernels.
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
    return;   /* no change! - At least at this time */

  /* Handle special cases */
  switch (kernel->type) {
    /* These built-in kernels are cylindrical kernels, rotating is useless */
    case GaussianKernel:
    case DOGKernel:
    case DiskKernel:
    case PeaksKernel:
    case LaplacianKernel:
    case ChebyshevKernel:
    case ManhattenKernel:
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
    case RectangleKernel:
      if ( 135.0 < angle && angle <= 225.0 )
        return;
      if ( 225.0 < angle && angle <= 315.0 )
        angle -= 180;
      break;

    default:
      break;
  }
  /* Attempt rotations by 45 degrees */
  if ( 22.5 < fmod(angle,90.0) && fmod(angle,90.0) <= 67.5 )
    {
      if ( kernel->width == 3 && kernel->height == 3 )
        { /* Rotate a 3x3 square by 45 degree angle */
          MagickRealType t  = kernel->values[0];
          kernel->values[0] = kernel->values[1];
          kernel->values[1] = kernel->values[2];
          kernel->values[2] = kernel->values[5];
          kernel->values[5] = kernel->values[8];
          kernel->values[8] = kernel->values[7];
          kernel->values[7] = kernel->values[6];
          kernel->values[6] = kernel->values[3];
          kernel->values[3] = t;
          angle = fmod(angle+45.0, 360.0);
        }
      else
        perror("Unable to rotate non-3x3 kernel by 45 degrees");
    }
  if ( 45.0 < fmod(angle, 180.0)  && fmod(angle,180.0) <= 135.0 )
    {
      if ( kernel->width == 1 || kernel->height == 1 )
        { /* Do a transpose of the image, which results in a 90
          ** degree rotation of a 1 dimentional kernel
          */
          long
            t;
          t = (long) kernel->width;
          kernel->width = kernel->height;
          kernel->height = (unsigned long) t;
          t = kernel->x;
          kernel->x = kernel->y;
          kernel->y = t;
          if ( kernel->width == 1 )
            angle = fmod(angle+270.0, 360.0);  /* angle -90 degrees */
          else
            angle = fmod(angle+90.0, 360.0);   /* angle +90 degrees */
        }
      else if ( kernel->width == kernel->height )
        { /* Rotate a square array of values by 90 degrees */
          register unsigned long
            i,j,x,y;
          register MagickRealType
            *k,t;
          k=kernel->values;
          for( i=0, x=kernel->width-1;  i<=x;   i++, x--)
            for( j=0, y=kernel->height-1;  j<y;   j++, y--)
              { t                    = k[i+j*kernel->width];
                k[i+j*kernel->width] = k[y+i*kernel->width];
                k[y+i*kernel->width] = k[x+y*kernel->width];
                k[x+y*kernel->width] = k[j+x*kernel->width];
                k[j+x*kernel->width] = t;
              }
          angle = fmod(angle+90.0, 360.0);  /* angle +90 degrees */
        }
      else
        perror("Unable to rotate a non-square, non-linear kernel 90 degrees");
    }
  if ( 135.0 < angle && angle <= 225.0 )
    {
      /* For a 180 degree rotation - also know as a reflection */
      /* This is actually a very very common operation! */
      /* Basically all that is needed is a reversal of the kernel data! */
      unsigned long
        i,j;
      register double
        *k,t;

      k=kernel->values;
      for ( i=0, j=kernel->width*kernel->height-1;  i<j;  i++, j--)
        t=k[i],  k[i]=k[j],  k[j]=t;

      kernel->x = (long) kernel->width  - kernel->x - 1;
      kernel->y = (long) kernel->height - kernel->y - 1;
      angle = fmod(angle+180.0, 360.0);   /* angle+180 degrees */
    }
  /* At this point angle should at least between -45 (315) and +45 degrees
   * In the future some form of non-orthogonal angled rotates could be
   * performed here, posibily with a linear kernel restriction.
   */

#if 0
    { /* Do a Flop by reversing each row.
       */
      unsigned long
        y;
      register long
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
%  If any 'normalize_flags' are given the kernel will first be normalized and
%  then further scaled by the scaling factor value given.  A 'PercentValue'
%  flag will cause the given scaling factor to be divided by one hundred
%  percent.
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
%  values seperatally to those of the negative values, so the kernel will be
%  forced to become a zero-sum kernel better suited to such searches.
%
%  WARNING: Correct normalization of the kernel assumes that the '*_range'
%  attributes within the kernel structure have been correctly set during the
%  kernels creation.
%
%  NOTE: The values used for 'normalize_flags' have been selected specifically
%  to match the use of geometry options, so that '!' means NormalizeValue,
%  '^' means CorrelateNormalizeValue, and '%' means PercentValue.  All other
%  GeometryFlags values are ignored.
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
% This function is internal to this module only at this time, but can be
% exported to other modules if needed.
*/
MagickExport void ScaleKernelInfo(KernelInfo *kernel,
  const double scaling_factor,const GeometryFlags normalize_flags)
{
  register long
    i;

  register double
    pos_scale,
    neg_scale;

  /* scale the lower kernels first */
  if ( kernel->next != (KernelInfo *) NULL)
    ScaleKernelInfo(kernel->next, scaling_factor, normalize_flags);

  pos_scale = 1.0;
  if ( (normalize_flags&NormalizeValue) != 0 ) {
    /* normalize kernel appropriately */
    if ( fabs(kernel->positive_range + kernel->negative_range) > MagickEpsilon )
      pos_scale = fabs(kernel->positive_range + kernel->negative_range);
    else
      pos_scale = kernel->positive_range; /* special zero-summing kernel */
  }
  /* force kernel into being a normalized zero-summing kernel */
  if ( (normalize_flags&CorrelateNormalizeValue) != 0 ) {
    pos_scale = ( fabs(kernel->positive_range) > MagickEpsilon )
                 ? kernel->positive_range : 1.0;
    neg_scale = ( fabs(kernel->negative_range) > MagickEpsilon )
                 ? -kernel->negative_range : 1.0;
  }
  else
    neg_scale = pos_scale;

  /* finialize scaling_factor for positive and negative components */
  pos_scale = scaling_factor/pos_scale;
  neg_scale = scaling_factor/neg_scale;
  if ( (normalize_flags&PercentValue) != 0 ) {
    pos_scale /= 100.0;
    neg_scale /= 100.0;
  }

  for (i=0; i < (long) (kernel->width*kernel->height); i++)
    if ( ! IsNan(kernel->values[i]) )
      kernel->values[i] *= (kernel->values[i] >= 0) ? pos_scale : neg_scale;

  /* convolution output range */
  kernel->positive_range *= pos_scale;
  kernel->negative_range *= neg_scale;
  /* maximum and minimum values in kernel */
  kernel->maximum *= (kernel->maximum >= 0.0) ? pos_scale : neg_scale;
  kernel->minimum *= (kernel->minimum >= 0.0) ? pos_scale : neg_scale;

  /* swap kernel settings if user scaling factor is negative */
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
+     S h o w K e r n e l I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ShowKernelInfo() outputs the details of the given kernel defination to
%  standard error, generally due to a users 'showkernel' option request.
%
%  The format of the ShowKernel method is:
%
%      void ShowKernelInfo(KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
% This function is internal to this module only at this time. That may change
% in the future.
*/
MagickExport void ShowKernelInfo(KernelInfo *kernel)
{
  KernelInfo
    *k;

  long
    c, i, u, v;

  for (c=0, k=kernel;  k != (KernelInfo *) NULL;  c++, k=k->next ) {

    fprintf(stderr, "Kernel ");
    if ( kernel->next != (KernelInfo *) NULL )
      fprintf(stderr, " #%ld", c );
    fprintf(stderr, " \"%s\" of size %lux%lu%+ld%+ld ",
          MagickOptionToMnemonic(MagickKernelOptions, k->type),
          kernel->width, k->height,
          kernel->x, kernel->y );
    fprintf(stderr,
          " with values from %.*lg to %.*lg\n",
          GetMagickPrecision(), k->minimum,
          GetMagickPrecision(), k->maximum);
    fprintf(stderr, "Forming convolution output range from %.*lg to %.*lg%s\n",
          GetMagickPrecision(), k->negative_range,
          GetMagickPrecision(), k->positive_range,
          /*kernel->normalized == MagickTrue ? " (normalized)" : */ "" );
    for (i=v=0; v < (long) k->height; v++) {
      fprintf(stderr,"%2ld:",v);
      for (u=0; u < (long) k->width; u++, i++)
        if ( IsNan(k->values[i]) )
          fprintf(stderr," %*s", GetMagickPrecision()+2, "nan");
        else
          fprintf(stderr," %*.*lg", GetMagickPrecision()+2,
              GetMagickPrecision(), k->values[i]);
      fprintf(stderr,"\n");
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     Z e r o K e r n e l N a n s                                             %
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
%      voidZeroKernelNans (KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
% FUTURE: return the information in a string for API usage.
*/
MagickExport void ZeroKernelNans(KernelInfo *kernel)
{
  register long
    i;

  /* scale the lower kernels first */
  if ( kernel->next != (KernelInfo *) NULL)
    ZeroKernelNans(kernel->next);

  for (i=0; i < (long) (kernel->width*kernel->height); i++)
    if ( IsNan(kernel->values[i]) )
      kernel->values[i] = 0.0;

  return;
}

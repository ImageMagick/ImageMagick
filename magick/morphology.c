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
% Morpology is the the application of various kernals, of any size and even
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

  These are used a Kernel value of NaN means that that kernal position is not
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
  RotateKernelInfo(KernelInfo *, double);

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
%  The returned kernel should be free using the DestroyKernelInfo() when you
%  are finished with it.
%
%  Input kernel defintion strings can consist of any of three types.
%
%    "name:args"
%         Select from one of the built in kernels, using the name and
%         geometry arguments supplied.  See AcquireKernelBuiltIn()
%
%    "WxH[+X+Y]:num, num, num ..."
%         a kernal of size W by H, with W*H floating point numbers following.
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
%  The format of the AcquireKernal method is:
%
%      KernelInfo *AcquireKernelInfo(const char *kernel_string)
%
%  A description of each parameter follows:
%
%    o kernel_string: the Morphology/Convolution kernel wanted.
%
*/

MagickExport KernelInfo *AcquireKernelInfo(const char *kernel_string)
{
  KernelInfo
    *kernel;

  char
    token[MaxTextExtent];

  register long
    i;

  const char
    *p;

  MagickStatusType
    flags;

  GeometryInfo
    args;

  double
    nan = sqrt((double)-1.0);  /* Special Value : Not A Number */

  assert(kernel_string != (const char *) NULL);
  SetGeometryInfo(&args);

  /* does it start with an alpha - Return a builtin kernel */
  GetMagickToken(kernel_string,&p,token);
  if ( isalpha((int)token[0]) )
  {
    long
      type;

    type=ParseMagickOption(MagickKernelOptions,MagickFalse,token);
    if ( type < 0 || type == UserDefinedKernel )
      return((KernelInfo *)NULL);

    while (((isspace((int) ((unsigned char) *p)) != 0) ||
           (*p == ',') || (*p == ':' )) && (*p != '\0'))
      p++;
    flags = ParseGeometry(p, &args);

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
      if ( (flags & HeightValue) == 0 ) /* if no scale */
        args.sigma = 1.0;               /* then  scale = 1.0 */
      break;
    default:
      break;
    }

    return(AcquireKernelBuiltIn((KernelInfoType)type, &args));
  }

  kernel=(KernelInfo *) AcquireMagickMemory(sizeof(*kernel));
  if (kernel == (KernelInfo *)NULL)
    return(kernel);
  (void) ResetMagickMemory(kernel,0,sizeof(*kernel));
  kernel->type = UserDefinedKernel;
  kernel->signature = MagickSignature;

  /* Has a ':' in argument - New user kernel specification */
  p = strchr(kernel_string, ':');
  if ( p != (char *) NULL)
    {
      /* ParseGeometry() needs the geometry separated! -- Arrgghh */
      memcpy(token, kernel_string, (size_t) (p-kernel_string));
      token[p-kernel_string] = '\0';
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
    { /* ELSE - Old old kernel specification, forming odd-square kernel */
      /* count up number of values given */
      p=(const char *) kernel_string;
      while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == '\''))
        p++;  /* ignore "'" chars for convolve filter usage - Cristy */
      for (i=0; *p != '\0'; i++)
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
  for (i=0; (i < (long) (kernel->width*kernel->height)) && (*p != '\0'); i++)
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
  /* check that we recieved at least one real (non-nan) value! */
  if ( kernel->minimum == MagickHuge )
    return(DestroyKernelInfo(kernel));

  /* This should not be needed for a fully defined kernel
   * Perhaps an error should be reported instead!
   * Kept for backward compatibility.
   */
  if ( i < (long) (kernel->width*kernel->height) ) {
    Minimize(kernel->minimum, kernel->values[i]);
    Maximize(kernel->maximum, kernel->values[i]);
    for ( ; i < (long) (kernel->width*kernel->height); i++)
      kernel->values[i]=0.0;
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
%    Gaussian  "{radius},{sigma}"
%       Generate a two-dimentional gaussian kernel, as used by -gaussian
%       A sigma is required, (with the 'x'), due to historical reasons.
%
%       NOTE: that the 'radius' is optional, but if provided can limit (clip)
%       the final size of the resulting kernel to a square 2*radius+1 in size.
%       The radius should be at least 2 times that of the sigma value, or
%       sever clipping and aliasing may result.  If not given or set to 0 the
%       radius will be determined so as to produce the best minimal error
%       result, which is usally much larger than is normally needed.
%
%    Blur  "{radius},{sigma},{angle}"
%       As per Gaussian, but generates a 1 dimensional or linear gaussian
%       blur, at the angle given (current restricted to orthogonal angles).
%       If a 'radius' is given the kernel is clipped to a width of 2*radius+1.
%
%       NOTE that two such blurs perpendicular to each other is equivelent to
%       -blur and the previous gaussian, but is often 10 or more times faster.
%
%    Comet  "{width},{sigma},{angle}"
%       Blur in one direction only, mush like how a bright object leaves
%       a comet like trail.  The Kernel is actually half a gaussian curve,
%       Adding two such blurs in oppiste directions produces a Linear Blur.
%
%       NOTE: that the first argument is the width of the kernel and not the
%       radius of the kernel.
%
%    # Still to be implemented...
%    #
%    # Sharpen "{radius},{sigma}
%    #    Negated Gaussian (center zeroed and re-normalized),
%    #    with a 2 unit positive peak.   -- Check On line documentation
%    #
%    # Laplacian  "{radius},{sigma}"
%    #    Laplacian (a mexican hat like) Function
%    #
%    # LOG  "{radius},{sigma1},{sigma2}
%    #    Laplacian of Gaussian
%    #
%    # DOG  "{radius},{sigma1},{sigma2}
%    #    Difference of two Gaussians
%    #
%    # Filter2D
%    # Filter1D
%    #    Set kernel values using a resize filter, and given scale (sigma)
%    #    Cylindrical or Linear.   Is this posible with an image?
%    #
%
%  Boolean Kernels
%
%    Rectangle "{geometry}"
%       Simply generate a rectangle of 1's with the size given. You can also
%       specify the location of the 'control point', otherwise the closest
%       pixel to the center of the rectangle is selected.
%
%       Properly centered and odd sized rectangles work the best.
%
%    Diamond  "[{radius}[,{scale}]]"
%       Generate a diamond shaped kernal with given radius to the points.
%       Kernel size will again be radius*2+1 square and defaults to radius 1,
%       generating a 3x3 kernel that is slightly larger than a square.
%
%    Square  "[{radius}[,{scale}]]"
%       Generate a square shaped kernel of size radius*2+1, and defaulting
%       to a 3x3 (radius 1).
%
%       Note that using a larger radius for the "Square" or the "Diamond"
%       is also equivelent to iterating the basic morphological method
%       that many times. However However iterating with the smaller radius 1
%       default is actually faster than using a larger kernel radius.
%
%    Disk   "[{radius}[,{scale}]]
%       Generate a binary disk of the radius given, radius may be a float.
%       Kernel size will be ceil(radius)*2+1 square.
%       NOTE: Here are some disk shapes of specific interest
%          "disk:1"    => "diamond" or "cross:1"
%          "disk:1.5"  => "square"
%          "disk:2"    => "diamond:2"
%          "disk:2.5"  => a general disk shape of radius 2
%          "disk:2.9"  => "square:2"
%          "disk:3.5"  => default - octagonal/disk shape of radius 3
%          "disk:4.2"  => roughly octagonal shape of radius 4
%          "disk:4.3"  => a general disk shape of radius 4
%       After this all the kernel shape becomes more and more circular.
%
%       Because a "disk" is more circular when using a larger radius, using a
%       larger radius is preferred over iterating the morphological operation.
%
%    Plus  "[{radius}[,{scale}]]"
%       Generate a kernel in the shape of a 'plus' sign. The length of each
%       arm is also the radius, which defaults to 2.
%
%       This kernel is not a good general morphological kernel, but is used
%       more for highlighting and marking any single pixels in an image using,
%       a "Dilate" or "Erode" method as appropriate.
%
%       NOTE: "plus:1" is equivelent to a "Diamond" kernel.
%
%       Note that unlike other kernels iterating a plus does not produce the
%       same result as using a larger radius for the cross.
%
%  Distance Measuring Kernels
%
%    Chebyshev "[{radius}][x{scale}]"   largest x or y distance (default r=1)
%    Manhatten "[{radius}][x{scale}]"   square grid distance    (default r=1)
%    Euclidean "[{radius}][x{scale}]"   direct distance         (default r=1)
%
%       Different types of distance measuring methods, which are used with the
%       a 'Distance' morphology method for generating a gradient based on
%       distance from an edge of a binary shape, though there is a technique
%       for handling a anti-aliased shape.
%
%       Chebyshev Distance (also known as Tchebychev Distance) is a value of
%       one to any neighbour, orthogonal or diagonal. One why of thinking of
%       it is the number of squares a 'King' or 'Queen' in chess needs to
%       traverse reach any other position on a chess board.  It results in a
%       'square' like distance function, but one where diagonals are closer
%       than expected.
%
%       Manhatten Distance (also known as Rectilinear Distance, or the Taxi
%       Cab metric), is the distance needed when you can only travel in
%       orthogonal (horizontal or vertical) only.  It is the distance a 'Rook'
%       in chess would travel. It results in a diamond like distances, where
%       diagonals are further than expected.
%
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
%       See the 'Distance' Morphological Method, for information of how it is
%       applied.
%
%  # Hit-n-Miss Kernel-Lists -- Still to be implemented
%  #
%  # specifically for   Pruning,  Thinning,  Thickening
%  #
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

  kernel=(KernelInfo *) AcquireMagickMemory(sizeof(*kernel));
  if (kernel == (KernelInfo *) NULL)
    return(kernel);
  (void) ResetMagickMemory(kernel,0,sizeof(*kernel));
  kernel->minimum = kernel->maximum = 0.0;
  kernel->negative_range = kernel->positive_range = 0.0;
  kernel->type = type;
  kernel->signature = MagickSignature;

  switch(type) {
    /* Convolution Kernels */
    case GaussianKernel:
      { double
          sigma = fabs(args->sigma);

        sigma = (sigma <= MagickEpsilon) ? 1.0 : sigma;

        kernel->width = kernel->height =
                            GetOptimalKernelWidth2D(args->rho,sigma);
        kernel->x = kernel->y = (long) (kernel->width-1)/2;
        kernel->negative_range = kernel->positive_range = 0.0;
        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        sigma = 2.0*sigma*sigma; /* simplify the expression */
        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            kernel->positive_range += (
              kernel->values[i] =
                 exp(-((double)(u*u+v*v))/sigma)
                       /*  / (MagickPI*sigma)  */ );
        kernel->minimum = 0;
        kernel->maximum = kernel->values[
                         kernel->y*kernel->width+kernel->x ];

        ScaleKernelInfo(kernel, 1.0, NormalizeValue); /* Normalize */

        break;
      }
    case BlurKernel:
      { double
          sigma = fabs(args->sigma);

        sigma = (sigma <= MagickEpsilon) ? 1.0 : sigma;

        kernel->width = GetOptimalKernelWidth1D(args->rho,sigma);
        kernel->x = (long) (kernel->width-1)/2;
        kernel->height = 1;
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
        sigma *= KernelRank;                /* simplify expanded curve */
        v = (long) (kernel->width*KernelRank-1)/2; /* start/end points to fit range */
        (void) ResetMagickMemory(kernel->values,0, (size_t)
                       kernel->width*sizeof(double));
        for ( u=-v; u <= v; u++) {
          kernel->values[(u+v)/KernelRank] +=
                exp(-((double)(u*u))/(2.0*sigma*sigma))
                       /*   / (MagickSQ2PI*sigma/KernelRank)  */ ;
        }
        for (i=0; i < (long) kernel->width; i++)
          kernel->positive_range += kernel->values[i];
#else
        for ( i=0, u=-kernel->x; i < kernel->width; i++, u++)
          kernel->positive_range += (
              kernel->values[i] =
                exp(-((double)(u*u))/(2.0*sigma*sigma))
                       /*  / (MagickSQ2PI*sigma)  */ );
#endif
        kernel->minimum = 0;
        kernel->maximum = kernel->values[ kernel->x ];
        /* Note that neither methods above generate a normalized kernel,
        ** though it gets close. The kernel may be 'clipped' by a user defined
        ** radius, producing a smaller (darker) kernel.  Also for very small
        ** sigma's (> 0.1) the central value becomes larger than one, and thus
        ** producing a very bright kernel.
        */

        /* Normalize the 1D Gaussian Kernel
        **
        ** Because of this the divisor in the above kernel generator is
        ** not needed, so is not done above.
        */
        ScaleKernelInfo(kernel, 1.0, NormalizeValue); /* Normalize */

        /* rotate the kernel by given angle */
        RotateKernelInfo(kernel, args->xi);
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

        /* A comet blur is half a gaussian curve, so that the object is
        ** blurred in one direction only.  This may not be quite the right
        ** curve so may change in the future. The function must be normalised.
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
    /* Boolean Kernels */
    case RectangleKernel:
    case SquareKernel:
      {
        double scale;
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

        /* set all kernel values to 1.0 */
        u=(long) kernel->width*kernel->height;
        for ( i=0; i < u; i++)
            kernel->values[i] = scale;
        kernel->minimum = kernel->maximum = scale;   /* a flat shape */
        kernel->positive_range = scale*u;
        break;
      }
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
    case DiskKernel:
      {
        long
          limit;

        limit = (long)(args->rho*args->rho);
        if (args->rho < 0.1)             /* default radius approx 3.5 */
          kernel->width = kernel->height = 7L, limit = 10L;
        else
           kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        /* set all kernel values within disk area to 1.0 */
        for ( i=0, v= -kernel->y; v <= (long)kernel->y; v++)
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

        /* set all kernel values along axises to 1.0 */
        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            kernel->values[i] = (u == 0 || v == 0) ? args->sigma : nan;
        kernel->minimum = kernel->maximum = args->sigma;   /* a flat shape */
        kernel->positive_range = args->sigma*(kernel->width*2.0 - 1.0);
        break;
      }
    /* Distance Measuring Kernels */
    case ChebyshevKernel:
      {
        double
          scale;

        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
          kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        scale = (args->sigma < 1.0) ? 100.0 : args->sigma;
        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            kernel->positive_range += ( kernel->values[i] =
                 scale*((labs(u)>labs(v)) ? labs(u) : labs(v)) );
        kernel->maximum = kernel->values[0];
        break;
      }
    case ManhattenKernel:
      {
        double
          scale;

        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
           kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        scale = (args->sigma < 1.0) ? 100.0 : args->sigma;
        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            kernel->positive_range += ( kernel->values[i] =
                 scale*(labs(u)+labs(v)) );
        kernel->maximum = kernel->values[0];
        break;
      }
    case EuclideanKernel:
      {
        double
          scale;

        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
           kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->x = kernel->y = (long) (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernelInfo(kernel));

        scale = (args->sigma < 1.0) ? 100.0 : args->sigma;
        for ( i=0, v=-kernel->y; v <= (long)kernel->y; v++)
          for ( u=-kernel->x; u <= (long)kernel->x; u++, i++)
            kernel->positive_range += ( kernel->values[i] =
                 scale*sqrt((double)(u*u+v*v)) );
        kernel->maximum = kernel->values[0];
        break;
      }
    /* Undefined Kernels */
    case LaplacianKernel:
    case LOGKernel:
    case DOGKernel:
      perror("Kernel Type has not been defined yet");
      /* FALL THRU */
    default:
      /* Generate a No-Op minimal kernel - 1x1 pixel */
      kernel->values=(double *)AcquireQuantumMemory((size_t)1,sizeof(double));
      if (kernel->values == (double *) NULL)
        return(DestroyKernelInfo(kernel));
      kernel->width = kernel->height = 1;
      kernel->x = kernel->x = 0;
      kernel->type = UndefinedKernel;
      kernel->maximum =
        kernel->positive_range =
          kernel->values[0] = 1.0;  /* a flat single-point no-op kernel! */
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
%  CloneKernelInfo() creates a new clone of the given Kernel so that its can
%  be modified without effecting the original.  The cloned kernel should be
%  destroyed using DestoryKernelInfo() when no longer needed.
%
%  The format of the DestroyKernelInfo method is:
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

  KernelInfo *
    new;

  assert(kernel != (KernelInfo *) NULL);

  new=(KernelInfo *) AcquireMagickMemory(sizeof(*kernel));
  if (new == (KernelInfo *) NULL)
    return(new);
  *new = *kernel; /* copy values in structure */

  new->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
  if (new->values == (double *) NULL)
    return(DestroyKernelInfo(new));

  for (i=0; i < (long) (kernel->width*kernel->height); i++)
    new->values[i] = kernel->values[i];

  return(new);
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

  kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
  kernel->values=(double *)RelinquishMagickMemory(kernel->values);
  kernel=(KernelInfo *) RelinquishMagickMemory(kernel);
  return(kernel);
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
 * Apply the Low-Level Morphology Method using the given Kernel
 * Returning the number of pixels that changed.
 * Two pre-created images must be provided, no image is created.
 */
static unsigned long MorphologyApply(const Image *image, Image
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
    case ErodeMorphology:
    case ErodeIntensityMorphology:
      /* kernel is user as is, without reflection */
      break;
    case ConvolveMorphology:
    case DilateMorphology:
    case DilateIntensityMorphology:
    case DistanceMorphology:
      /* kernel needs to used with reflection */
      offx = (long) kernel->width-offx-1;
      offy = (long) kernel->height-offy-1;
      break;
    default:
      perror("Not a low level Morpholgy Method");
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
        result;

      /* Copy input to ouput image for unused channels
       * This removes need for 'cloning' a new image every iteration
       */
      *q = p[r];
      if (image->colorspace == CMYKColorspace)
        q_indexes[x] = p_indexes[r];

      result.green=(MagickRealType) 0;
      result.blue=(MagickRealType) 0;
      result.opacity=(MagickRealType) 0;
      result.index=(MagickRealType) 0;
      switch (method) {
        case ConvolveMorphology:
          /* Set the user defined bias of the weighted average output
          **
          ** FUTURE: provide some way for internal functions to disable
          ** user defined bias and scaling effects.
          */
          result=bias;
          break;
        case DilateMorphology:
          result.red     =
          result.green   =
          result.blue    =
          result.opacity =
          result.index   = -MagickHuge;
          break;
        case ErodeMorphology:
          result.red     =
          result.green   =
          result.blue    =
          result.opacity =
          result.index   = +MagickHuge;
          break;
        case DilateIntensityMorphology:
        case ErodeIntensityMorphology:
          result.red = 0.0;  /* flag indicating first match found */
          break;
        default:
          /* Otherwise just start with the original pixel value */
          result.red     = (MagickRealType) p[r].red;
          result.green   = (MagickRealType) p[r].green;
          result.blue    = (MagickRealType) p[r].blue;
          result.opacity = QuantumRange - (MagickRealType) p[r].opacity;
          if ( image->colorspace == CMYKColorspace)
             result.index   = (MagickRealType) p_indexes[r];
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
            ** kernel it is Convolution that is implemented.
            **
            ** Correlation will have its kernel reflected before calling
            ** this function to do a Convolve.
            **
            ** For more details of Correlation vs Convolution see
            **   http://www.cs.umd.edu/~djacobs/CMSC426/Convolution.pdf
            */
            if (((channel & OpacityChannel) == 0) ||
                      (image->matte == MagickFalse))
              {
                /* Convolution without transparency effects */
                k = &kernel->values[ kernel->width*kernel->height-1 ];
                k_pixels = p;
                k_indexes = p_indexes;
                for (v=0; v < (long) kernel->height; v++) {
                  for (u=0; u < (long) kernel->width; u++, k--) {
                    if ( IsNan(*k) ) continue;
                    result.red     += (*k)*k_pixels[u].red;
                    result.green   += (*k)*k_pixels[u].green;
                    result.blue    += (*k)*k_pixels[u].blue;
                    /* result.opacity += not involved here */
                    if ( image->colorspace == CMYKColorspace)
                      result.index   += (*k)*k_indexes[u];
                  }
                  k_pixels += image->columns+kernel->width;
                  k_indexes += image->columns+kernel->width;
                }
              }
            else
              { /* Kernel & Alpha weighted Convolution */
                MagickRealType
                  alpha,  /* alpha value * kernel weighting */
                  gamma;  /* weighting divisor */

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
            break;

        case ErodeMorphology:
            /* Minimize Value within kernel neighbourhood
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
                Minimize(result.red,     (double) k_pixels[u].red);
                Minimize(result.green,   (double) k_pixels[u].green);
                Minimize(result.blue,    (double) k_pixels[u].blue);
                Minimize(result.opacity, QuantumRange-(double) k_pixels[u].opacity);
                if ( image->colorspace == CMYKColorspace)
                  Minimize(result.index,   (double) k_indexes[u]);
              }
              k_pixels += image->columns+kernel->width;
              k_indexes += image->columns+kernel->width;
            }
            break;

        case DilateMorphology:
            /* Maximize Value within kernel neighbourhood
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
                Maximize(result.red,     (double) k_pixels[u].red);
                Maximize(result.green,   (double) k_pixels[u].green);
                Maximize(result.blue,    (double) k_pixels[u].blue);
                Maximize(result.opacity, QuantumRange-(double) k_pixels[u].opacity);
                if ( image->colorspace == CMYKColorspace)
                  Maximize(result.index,   (double) k_indexes[u]);
              }
              k_pixels += image->columns+kernel->width;
              k_indexes += image->columns+kernel->width;
            }
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
            */
#if 0
            /* No need to do distance morphology if original value is zero
            ** Unfortunatally I have not been able to get this right
            ** when channel selection also becomes involved. -- Arrgghhh
            */
            if (   ((channel & RedChannel) == 0 && p[r].red == 0)
                || ((channel & GreenChannel) == 0 && p[r].green == 0)
                || ((channel & BlueChannel) == 0 && p[r].blue == 0)
                || ((channel & OpacityChannel) == 0 && p[r].opacity == 0)
                || (( (channel & IndexChannel) == 0
                    || image->colorspace != CMYKColorspace
                                                ) && p_indexes[x] ==0 )
              )
              break;
#endif
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
      switch ( method ) {
        case UndefinedMorphology:
        case DilateIntensityMorphology:
        case ErodeIntensityMorphology:
          break;  /* full pixel was directly assigned - not a channel method */
        default:
          /* Assign the results */
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
  long
    count;

  Image
    *new_image,
    *old_image,
    *grad_image;

  const char
    *artifact;

  unsigned long
    changed,
    limit;

  KernelInfo
    *curr_kernel;

  MorphologyMethod
    curr_method;

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

  count = 0;      /* interation count */
  changed = 1;    /* if compound method assume image was changed */
  curr_kernel = (KernelInfo *)kernel;  /* allow kernel and method */
  curr_method = method;                /* to be changed as nessary */

  limit = (unsigned long) iterations;
  if ( iterations < 0 )
    limit = image->columns > image->rows ? image->columns : image->rows;

  /* Third-level morphology methods */
  grad_image=(Image *) NULL;
  switch( curr_method ) {
    case EdgeMorphology:
      grad_image = MorphologyImageChannel(image, channel,
            DilateMorphology, iterations, curr_kernel, exception);
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
        return((Image *) NULL);
      curr_method = DilateMorphology;
      break;
    case OpenIntensityMorphology:
      new_image = MorphologyImageChannel(image, channel,
            ErodeIntensityMorphology, iterations, curr_kernel, exception);
      if (new_image == (Image *) NULL)
        return((Image *) NULL);
      curr_method = DilateIntensityMorphology;
      break;

    case CloseMorphology:
      /* Close is a Dilate then Erode using reflected kernel */
      /* A reflected kernel is needed for a Close */
      if ( curr_kernel == kernel )
        curr_kernel = CloneKernelInfo(kernel);
      RotateKernelInfo(curr_kernel,180);
      new_image = MorphologyImageChannel(image, channel,
            DilateMorphology, iterations, curr_kernel, exception);
      if (new_image == (Image *) NULL)
        return((Image *) NULL);
      curr_method = ErodeMorphology;
      break;
    case CloseIntensityMorphology:
      /* A reflected kernel is needed for a Close */
      if ( curr_kernel == kernel )
        curr_kernel = CloneKernelInfo(kernel);
      RotateKernelInfo(curr_kernel,180);
      new_image = MorphologyImageChannel(image, channel,
            DilateIntensityMorphology, iterations, curr_kernel, exception);
      if (new_image == (Image *) NULL)
        return((Image *) NULL);
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
      RotateKernelInfo(curr_kernel,180);
      curr_method = ConvolveMorphology;
      /* FALL-THRU into Correlate (weigthed sum without reflection) */

    case ConvolveMorphology:
      /* Scale or Normalize kernel, according to user wishes
      ** before using it for the Convolve/Correlate method.
      **
      ** FUTURE: provide some way for internal functions to disable
      ** user bias and scaling effects.
      */
      artifact = GetImageArtifact(image,"convolve:scale");
      if ( artifact != (char *)NULL ) {
        MagickStatusType
          flags;
        GeometryInfo
          args;

        if ( curr_kernel == kernel )
          curr_kernel = CloneKernelInfo(kernel);

        args.rho = 1.0;
        flags = ParseGeometry(artifact, &args);
        ScaleKernelInfo(curr_kernel, args.rho, flags);
      }
      /* FALL-THRU to do the first, and typically the only iteration */

    default:
      /* Do a single iteration using the Low-Level Morphology method!
      ** This ensures a "new_image" has been generated, but allows us to skip
      ** the creation of 'old_image' if no more iterations are needed.
      **
      ** The "curr_method" should also be set to a low-level method that is
      ** understood by the MorphologyApply() internal function.
      */
      new_image=CloneImage(image,0,0,MagickTrue,exception);
      if (new_image == (Image *) NULL)
        return((Image *) NULL);
      if (SetImageStorageClass(new_image,DirectClass) == MagickFalse)
        {
          InheritException(exception,&new_image->exception);
          new_image=DestroyImage(new_image);
          return((Image *) NULL);
        }
      changed = MorphologyApply(image,new_image,curr_method,channel,curr_kernel,
            exception);
      count++;
      if ( GetImageArtifact(image,"verbose") != (const char *) NULL )
        fprintf(stderr, "Morphology %s:%ld => Changed %lu\n",
              MagickOptionToMnemonic(MagickMorphologyOptions, curr_method),
              count, changed);
      break;
  }

  /* At this point the "curr_method" should not only be set to a low-level
  ** method that is understood by the MorphologyApply() internal function,
  ** but "new_image" should now be defined, as the image to apply the
  ** "curr_method" to.
  */

  /* Repeat the low-level morphology until count or no change reached */
  if ( count < (long) limit && changed > 0 ) {
    old_image = CloneImage(new_image,0,0,MagickTrue,exception);
    if (old_image == (Image *) NULL)
        return(DestroyImage(new_image));
    if (SetImageStorageClass(old_image,DirectClass) == MagickFalse)
      {
        InheritException(exception,&old_image->exception);
        old_image=DestroyImage(old_image);
        return(DestroyImage(new_image));
      }
    while( count < (long) limit && changed != 0 )
      {
        Image *tmp = old_image;
        old_image = new_image;
        new_image = tmp;
        changed = MorphologyApply(old_image,new_image,curr_method,channel,
             curr_kernel, exception);
        count++;
        if ( GetImageArtifact(image,"verbose") != (const char *) NULL )
          fprintf(stderr, "Morphology %s:%ld => Changed %lu\n",
                MagickOptionToMnemonic(MagickMorphologyOptions, curr_method),
                count, changed);
      }
    old_image=DestroyImage(old_image);
  }

  /* We are finished with kernel - destroy it if we made a clone */
  if ( curr_kernel != kernel )
    curr_kernel=DestroyKernelInfo(curr_kernel);

  /* Third-level Subtractive methods post-processing */
  switch( method ) {
    case EdgeOutMorphology:
    case EdgeInMorphology:
    case TopHatMorphology:
    case BottomHatMorphology:
      /* Get Difference relative to the original image */
      (void) CompositeImageChannel(new_image, channel, DifferenceCompositeOp,
          image, 0, 0);
      break;
    case EdgeMorphology:  /* subtract the Erode from a Dilate */
      (void) CompositeImageChannel(new_image, channel, DifferenceCompositeOp,
          grad_image, 0, 0);
      grad_image=DestroyImage(grad_image);
      break;
    default:
      break;
  }

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
  /* WARNING: Currently assumes the kernel (rightly) is horizontally symetrical
  **
  ** TODO: expand beyond simple 90 degree rotates, flips and flops
  */

  /* Modulus the angle */
  angle = fmod(angle, 360.0);
  if ( angle < 0 )
    angle += 360.0;

  if ( 315.0 < angle || angle <= 45.0 )
    return;   /* no change! - At least at this time */

  switch (kernel->type) {
    /* These built-in kernels are cylindrical kernels, rotating is useless */
    case GaussianKernel:
    case LaplacianKernel:
    case LOGKernel:
    case DOGKernel:
    case DiskKernel:
    case ChebyshevKernel:
    case ManhattenKernel:
    case EuclideanKernel:
      return;

    /* These may be rotatable at non-90 angles in the future */
    /* but simply rotating them in multiples of 90 degrees is useless */
    case SquareKernel:
    case DiamondKernel:
    case PlusKernel:
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

    /* these are freely rotatable in 90 degree units */
    case CometKernel:
    case UndefinedKernel:
    case UserDefinedKernel:
      break;
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
      angle = fmod(angle+180.0, 360.0);
    }
  if ( 45.0 < angle && angle <= 135.0 )
    { /* Do a transpose and a flop, of the image, which results in a 90
       * degree rotation using two mirror operations.
       *
       * WARNING: this assumes the original image was a 1 dimentional image
       * but currently that is the only built-ins it is applied to.
       */
      long
        t;
      t = (long) kernel->width;
      kernel->width = kernel->height;
      kernel->height = (unsigned long) t;
      t = kernel->x;
      kernel->x = kernel->y;
      kernel->y = t;
      angle = fmod(450.0 - angle, 360.0);
    }
  /* At this point angle should be between -45 (315) and +45 degrees
   * In the future some form of non-orthogonal angled rotates could be
   * performed here, posibily with a linear kernel restriction.
   */

#if 0
    Not currently in use!
    { /* Do a flop, this assumes kernel is horizontally symetrical.
       * Each row of the kernel needs to be reversed!
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
%  ScaleKernelInfo() scales the kernel by the given amount, with or without
%  normalization of the sum of the kernel values.
%
%  By default (no flags given) the values within the kernel is scaled
%  according the given scaling factor.
%
%  If any 'normalize_flags' are given the kernel will be normalized and then
%  further scaled by the scaleing factor value given.  A 'PercentValue' flag
%  will cause the given scaling factor to be divided by one hundred percent.
%
%  Kernel normalization ('normalize_flags' given) is designed to ensure that
%  any use of the kernel scaling factor with 'Convolve' or 'Correlate'
%  morphology methods will fall into -1.0 to +1.0 range.  Note however that
%  for non-HDRI versions of IM this may cause images to have any negative
%  results clipped, unless some 'clip' any negative output from 'Convolve'
%  with the use of some kernels.
%
%  More specifically.  Kernels which only contain positive values (such as a
%  'Gaussian' kernel) will be scaled so that those values sum to +1.0,
%  ensuring a 0.0 to +1.0 convolution output range for non-HDRI images.
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
%  WARNING: Correct normalization of the kernal assumes that the '*_range'
%  attributes within the kernel structure have been correctly set during the
%  kernels creation.
%
%  NOTE: The values used for 'normalize_flags' have been selected specifically
%  to match the use of geometry options, so that '!' means NormalizeValue, '^'
%  means CorrelateNormalizeValue, and '%' means PercentValue.  All other
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
  long
    i, u, v;

  fprintf(stderr,
        "Kernel \"%s\" of size %lux%lu%+ld%+ld with values from %.*lg to %.*lg\n",
        MagickOptionToMnemonic(MagickKernelOptions, kernel->type),
        kernel->width, kernel->height,
        kernel->x, kernel->y,
        GetMagickPrecision(), kernel->minimum,
        GetMagickPrecision(), kernel->maximum);
  fprintf(stderr, "Forming convolution output range from %.*lg to %.*lg%s\n",
        GetMagickPrecision(), kernel->negative_range,
        GetMagickPrecision(), kernel->positive_range,
        /*kernel->normalized == MagickTrue ? " (normalized)" : */ "" );
  for (i=v=0; v < (long) kernel->height; v++) {
    fprintf(stderr,"%2ld:",v);
    for (u=0; u < (long) kernel->width; u++, i++)
      if ( IsNan(kernel->values[i]) )
        fprintf(stderr," %*s", GetMagickPrecision()+2, "nan");
      else
        fprintf(stderr," %*.*lg", GetMagickPrecision()+2,
             GetMagickPrecision(), kernel->values[i]);
    fprintf(stderr,"\n");
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

  for (i=0; i < (long) (kernel->width*kernel->height); i++)
    if ( IsNan(kernel->values[i]) )
      kernel->values[i] = 0.0;

  return;
}

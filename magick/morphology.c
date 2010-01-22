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
 * The following test is for special floating point numbers of value NaN (not
 * a number), that may be used within a Kernel Definition.  NaN's are defined
 * as part of the IEEE standard for floating point number representation.
 *
 * These are used a Kernel value of NaN means that that kernal position is not
 * part of the normal convolution or morphology process, and thus allowing the
 * use of 'shaped' kernels.
 *
 * Special Properities Two NaN's are never equal, even if they are from the
 * same variable That is the IsNaN() macro is only true if the value is NaN.
 */
#define IsNan(a)   ((a)!=(a))

/*
 * Other global definitions used by module
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


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c q u i r e K e r n e l F r o m S t r i n g                           %
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
%  ASIDE: Previously IM was restricted to a square of odd size using the exact
%  center.
%
%  The floating point values in the kernel can also include a special value
%  known as 'NaN' or 'not a number' to indicate that this value is not part
%  of the kernel array. This allows you to specify a non-rectangular shaped
%  kernel, for use in Morphological operators, without the need for some type
%  of kernal mask.
%
%  The returned kernel should be freed using the DestroyKernel() when you are
%  finished with it.
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
%  Note that 'name' kernels will start with an alphabetic character
%  while the new kernel specification has a ':' character in its
%  specification string. If neither is the case, it assumes it is the
%  old style of a simple list of numbers.
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

  register unsigned long
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
    if ( type == RectangleKernel ) {
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
      memcpy(token, kernel_string, p-kernel_string);
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
        return(DestroyKernel(kernel));
      kernel->offset_x = ((flags & XValue)!=0) ? (unsigned long)args.xi
                                               : (kernel->width-1)/2;
      kernel->offset_y = ((flags & YValue)!=0) ? (unsigned long)args.psi
                                               : (kernel->height-1)/2;
      if ( kernel->offset_x >= kernel->width ||
           kernel->offset_y >= kernel->height )
        return(DestroyKernel(kernel));

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
      kernel->offset_x = kernel->offset_y = (kernel->width-1)/2;
      p=(const char *) kernel_string;
      while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == '\''))
        p++;  /* ignore "'" chars for convolve filter usage - Cristy */
    }

  /* Read in the kernel values from rest of input string argument */
  kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                        kernel->height*sizeof(double));
  if (kernel->values == (double *) NULL)
    return(DestroyKernel(kernel));

  kernel->value_min = +MagickHuge;
  kernel->value_max = -MagickHuge;
  kernel->range_neg = kernel->range_pos = 0.0;
  for (i=0; (i < kernel->width*kernel->height) && (*p != '\0'); i++)
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
          ?  ( kernel->range_neg += kernel->values[i] )
          :  ( kernel->range_pos += kernel->values[i] );
      Minimize(kernel->value_min, kernel->values[i]);
      Maximize(kernel->value_max, kernel->values[i]);
    }
  }

  /* This should not be needed for a fully defined defined kernel
   * Perhaps an error should be reported instead!
   */
  if ( i < kernel->width*kernel->height ) {
    Minimize(kernel->value_min, kernel->values[i]);
    Maximize(kernel->value_max, kernel->values[i]);
    for ( ; i < kernel->width*kernel->height; i++)
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
%    Gaussian  "[{radius}]x{sigma}"
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
%    Blur  "[{radius}]x{sigma}[+angle]"
%       As per Gaussian, but generates a 1 dimensional or linear gaussian
%       blur, at the angle given (current restricted to orthogonal angles).
%       If a 'radius' is given the kernel is clipped to a width of 2*radius+1.
%
%       NOTE that two such blurs perpendicular to each other is equivelent to
%       -blur and the previous gaussian, but is often 10 or more times faster.
%
%    Comet  "[{width}]x{sigma}[+angle]"
%       Blur in one direction only, mush like how a bright object leaves
%       a comet like trail.  The Kernel is actually half a gaussian curve,
%       Adding two such blurs in oppiste directions produces a Linear Blur.
%
%       NOTE: that the first argument is the width of the kernel and not the
%       radius of the kernel.
%
%    # Still to be implemented...
%    #
%    # Laplacian  "{radius}x{sigma}"
%    #    Laplacian (a mexican hat like) Function
%    #
%    # LOG  "{radius},{sigma1},{sigma2}
%    #    Laplacian of Gaussian
%    #
%    # DOG  "{radius},{sigma1},{sigma2}
%    #    Difference of Gaussians
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
%    Diamond  "[{radius}]"
%       Generate a diamond shaped kernal with given radius to the points.
%       Kernel size will again be radius*2+1 square and defaults to radius 1,
%       generating a 3x3 kernel that is slightly larger than a square.
%
%    Square  "[{radius}]"
%       Generate a square shaped kernel of size radius*2+1, and defaulting
%       to a 3x3 (radius 1).
%
%       Note that using a larger radius for the "Square" or the "Diamond"
%       is also equivelent to iterating the basic morphological method
%       that many times. However However iterating with the smaller radius 1
%       default is actually faster than using a larger kernel radius.
%
%    Disk   "[{radius}]
%       Generate a binary disk of the radius given, radius may be a float.
%       Kernel size will be ceil(radius)*2+1 square.
%       NOTE: Here are some disk shapes of specific interest
%          "disk:1"    => "diamond" or "cross:1"
%          "disk:1.5"  => "square"
%          "disk:2"    => "diamond:2"
%          "disk:2.5"  => default - radius 2 disk shape
%          "disk:2.9"  => "square:2"
%          "disk:3.5"  => octagonal/disk shape of radius 3
%          "disk:4.2"  => roughly octagonal shape of radius 4
%          "disk:4.3"  => disk shape of radius 4
%       After this all the kernel shape becomes more and more circular.
%
%       Because a "disk" is more circular when using a larger radius, using a
%       larger radius is preferred over iterating the morphological operation.
%
%    Plus  "[{radius}]"
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
*/

MagickExport KernelInfo *AcquireKernelBuiltIn(const KernelInfoType type,
   const GeometryInfo *args)
{
  KernelInfo
    *kernel;

  register unsigned long
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
  kernel->value_min = kernel->value_max = 0.0;
  kernel->range_neg = kernel->range_pos = 0.0;
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
        kernel->offset_x = kernel->offset_y = (kernel->width-1)/2;
        kernel->range_neg = kernel->range_pos = 0.0;
        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernel(kernel));

        sigma = 2.0*sigma*sigma; /* simplify the expression */
        for ( i=0, v=-kernel->offset_y; v <= (long)kernel->offset_y; v++)
          for ( u=-kernel->offset_x; u <= (long)kernel->offset_x; u++, i++)
            kernel->range_pos += (
              kernel->values[i] =
                 exp(-((double)(u*u+v*v))/sigma)
                       /*  / (MagickPI*sigma)  */ );
        kernel->value_min = 0;
        kernel->value_max = kernel->values[
                         kernel->offset_y*kernel->width+kernel->offset_x ];

        KernelNormalize(kernel);

        break;
      }
    case BlurKernel:
      { double
          sigma = fabs(args->sigma);

        sigma = (sigma <= MagickEpsilon) ? 1.0 : sigma;

        kernel->width = GetOptimalKernelWidth1D(args->rho,sigma);
        kernel->offset_x = (kernel->width-1)/2;
        kernel->height = 1;
        kernel->offset_y = 0;
        kernel->range_neg = kernel->range_pos = 0.0;
        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernel(kernel));

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
        v = (kernel->width*KernelRank-1)/2; /* start/end points to fit range */
        (void) ResetMagickMemory(kernel->values,0, (size_t)
                       kernel->width*sizeof(double));
        for ( u=-v; u <= v; u++) {
          kernel->values[(u+v)/KernelRank] +=
                exp(-((double)(u*u))/(2.0*sigma*sigma))
                       /*   / (MagickSQ2PI*sigma/KernelRank)  */ ;
        }
        for (i=0; i < kernel->width; i++)
          kernel->range_pos += kernel->values[i];
#else
        for ( i=0, u=-kernel->offset_x; i < kernel->width; i++, u++)
          kernel->range_pos += (
              kernel->values[i] =
                exp(-((double)(u*u))/(2.0*sigma*sigma))
                       /*  / (MagickSQ2PI*sigma)  */ );
#endif
        kernel->value_min = 0;
        kernel->value_max = kernel->values[ kernel->offset_x ];
        /* Note that both the above methods do not generate a normalized
        ** kernel, though it gets close. The kernel may be 'clipped' by a user
        ** defined radius, producing a smaller (darker) kernel.  Also for very
        ** small sigma's (> 0.1) the central value becomes larger than one,
        ** and thus producing a very bright kernel.
        */
#if 1
        /* Normalize the 1D Gaussian Kernel
        **
        ** Because of this the divisor in the above kernel generator is
        ** not needed, so is not done above.
        */
        KernelNormalize(kernel);
#endif
        /* rotate the kernel by given angle */
        KernelRotate(kernel, args->xi);
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
        kernel->offset_x = kernel->offset_y = 0;
        kernel->height = 1;
        kernel->range_neg = kernel->range_pos = 0.0;
        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernel(kernel));

        /* A comet blur is half a gaussian curve, so that the object is
        ** blurred in one direction only.  This may not be quite the right
        ** curve so may change in the future. The function must be normalised.
        */
#if 1
#define KernelRank 3
        sigma *= KernelRank;                /* simplify expanded curve */
        v = kernel->width*KernelRank; /* start/end points to fit range */
        (void) ResetMagickMemory(kernel->values,0, (size_t)
                       kernel->width*sizeof(double));
        for ( u=0; u < v; u++) {
          kernel->values[u/KernelRank] +=
               exp(-((double)(u*u))/(2.0*sigma*sigma))
                       /*   / (MagickSQ2PI*sigma/KernelRank)  */ ;
        }
        for (i=0; i < kernel->width; i++)
          kernel->range_pos += kernel->values[i];
#else
        for ( i=0; i < kernel->width; i++)
          kernel->range_pos += (
            kernel->values[i] =
               exp(-((double)(i*i))/(2.0*sigma*sigma))
                       /*  / (MagickSQ2PI*sigma)  */ );
#endif
        kernel->value_min = 0;
        kernel->value_max = kernel->values[0];

        KernelNormalize(kernel);
        KernelRotate(kernel, args->xi);
        break;
      }
    /* Boolean Kernels */
    case RectangleKernel:
    case SquareKernel:
      {
        if ( type == SquareKernel )
          {
            if (args->rho < 1.0)
              kernel->width = kernel->height = 3;  /* default radius = 1 */
            else
              kernel->width = kernel->height = 2*(long)args->rho+1;
            kernel->offset_x = kernel->offset_y = (kernel->width-1)/2;
          }
        else {
            /* NOTE: user defaults set in "AcquireKernelInfo()" */
            if ( args->rho < 1.0 || args->sigma < 1.0 )
              return(DestroyKernel(kernel));       /* invalid args given */
            kernel->width = (unsigned long)args->rho;
            kernel->height = (unsigned long)args->sigma;
            if ( args->xi  < 0.0 || args->xi  > (double)kernel->width ||
                 args->psi < 0.0 || args->psi > (double)kernel->height )
              return(DestroyKernel(kernel));       /* invalid args given */
            kernel->offset_x = (unsigned long)args->xi;
            kernel->offset_y = (unsigned long)args->psi;
          }
        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernel(kernel));

        u=kernel->width*kernel->height;
        for ( i=0; i < (unsigned long)u; i++)
            kernel->values[i] = 1.0;
        break;
        kernel->value_min = kernel->value_max = 1.0; /* a flat kernel */
        kernel->range_pos = (double) u;
      }
    case DiamondKernel:
      {
        if (args->rho < 1.0)
          kernel->width = kernel->height = 3;  /* default radius = 1 */
        else
          kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->offset_x = kernel->offset_y = (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernel(kernel));

        for ( i=0, v=-kernel->offset_y; v <= (long)kernel->offset_y; v++)
          for ( u=-kernel->offset_x; u <= (long)kernel->offset_x; u++, i++)
            if ((labs(u)+labs(v)) <= (long)kernel->offset_x)
              kernel->range_pos += kernel->values[i] = 1.0;
            else
              kernel->values[i] = nan;
        kernel->value_min = kernel->value_max = 1.0; /* a flat kernel */
        break;
      }
    case DiskKernel:
      {
        long
          limit;

        limit = (long)(args->rho*args->rho);
        if (args->rho < 0.1)             /* default radius approx 2.5 */
          kernel->width = kernel->height = 5L, limit = 5L;
        else
           kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->offset_x = kernel->offset_y = (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernel(kernel));

        for ( i=0, v=-kernel->offset_y; v <= (long)kernel->offset_y; v++)
          for ( u=-kernel->offset_x; u <= (long)kernel->offset_x; u++, i++)
            if ((u*u+v*v) <= limit)
              kernel->range_pos += kernel->values[i] = 1.0;
            else
              kernel->values[i] = nan;
        kernel->value_min = kernel->value_max = 1.0; /* a flat kernel */
        break;
      }
    case PlusKernel:
      {
        if (args->rho < 1.0)
          kernel->width = kernel->height = 5;  /* default radius 2 */
        else
           kernel->width = kernel->height = ((unsigned long)args->rho)*2+1;
        kernel->offset_x = kernel->offset_y = (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernel(kernel));

        for ( i=0, v=-kernel->offset_y; v <= (long)kernel->offset_y; v++)
          for ( u=-kernel->offset_x; u <= (long)kernel->offset_x; u++, i++)
            kernel->values[i] = (u == 0 || v == 0) ? 1.0 : nan;
        kernel->value_min = kernel->value_max = 1.0; /* a flat kernel */
        kernel->range_pos = kernel->width*2.0 - 1.0;
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
        kernel->offset_x = kernel->offset_y = (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernel(kernel));

        scale = (args->sigma < 1.0) ? 100.0 : args->sigma;
        for ( i=0, v=-kernel->offset_y; v <= (long)kernel->offset_y; v++)
          for ( u=-kernel->offset_x; u <= (long)kernel->offset_x; u++, i++)
            kernel->range_pos += ( kernel->values[i] =
                 scale*((labs(u)>labs(v)) ? labs(u) : labs(v)) );
        kernel->value_max = kernel->values[0];
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
        kernel->offset_x = kernel->offset_y = (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernel(kernel));

        scale = (args->sigma < 1.0) ? 100.0 : args->sigma;
        for ( i=0, v=-kernel->offset_y; v <= (long)kernel->offset_y; v++)
          for ( u=-kernel->offset_x; u <= (long)kernel->offset_x; u++, i++)
            kernel->range_pos += ( kernel->values[i] =
                 scale*(labs(u)+labs(v)) );
        kernel->value_max = kernel->values[0];
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
        kernel->offset_x = kernel->offset_y = (kernel->width-1)/2;

        kernel->values=(double *) AcquireQuantumMemory(kernel->width,
                              kernel->height*sizeof(double));
        if (kernel->values == (double *) NULL)
          return(DestroyKernel(kernel));

        scale = (args->sigma < 1.0) ? 100.0 : args->sigma;
        for ( i=0, v=-kernel->offset_y; v <= (long)kernel->offset_y; v++)
          for ( u=-kernel->offset_x; u <= (long)kernel->offset_x; u++, i++)
            kernel->range_pos += ( kernel->values[i] =
                 scale*sqrt((double)(u*u+v*v)) );
        kernel->value_max = kernel->values[0];
        break;
      }
    /* Undefined Kernels */
    case LaplacianKernel:
    case LOGKernel:
    case DOGKernel:
      assert("Kernel Type has not been defined yet");
      /* FALL THRU */
    default:
      /* Generate a No-Op minimal kernel - 1x1 pixel */
      kernel->values=(double *)AcquireQuantumMemory((size_t)1,sizeof(double));
      if (kernel->values == (double *) NULL)
        return(DestroyKernel(kernel));
      kernel->width = kernel->height = 1;
      kernel->offset_x = kernel->offset_x = 0;
      kernel->type = UndefinedKernel;
      kernel->value_max =
        kernel->range_pos =
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
%     D e s t r o y K e r n e l                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyKernel() frees the memory used by a Convolution/Morphology kernel.
%
%  The format of the DestroyKernel method is:
%
%      KernelInfo *DestroyKernel(KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel to be destroyed
%
*/

MagickExport KernelInfo *DestroyKernel(KernelInfo *kernel)
{
  assert(kernel != (KernelInfo *) NULL);
  kernel->values=(double *)RelinquishMagickMemory(kernel->values);
  kernel=(KernelInfo *) RelinquishMagickMemory(kernel);
  return(kernel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     K e r n e l N o r m a l i z e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  KernelNormalize() normalize the kernel so its convolution output will
%  be over a unit range.
%
%  The format of the KernelNormalize method is:
%
%      void KernelRotate (KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
*/
MagickExport void KernelNormalize(KernelInfo *kernel)
{
  register unsigned long
    i;

  for (i=0; i < kernel->width*kernel->height; i++)
    kernel->values[i] /= (kernel->range_pos - kernel->range_neg);

  kernel->range_pos /= (kernel->range_pos - kernel->range_neg);
  kernel->range_neg /= (kernel->range_pos - kernel->range_neg);
  kernel->value_max /= (kernel->range_pos - kernel->range_neg);
  kernel->value_min /= (kernel->range_pos - kernel->range_neg);

  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     K e r n e l P r i n t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  KernelPrint() Print out the kernel details to standard error
%
%  The format of the KernelNormalize method is:
%
%      void KernelPrint (KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
*/
MagickExport void KernelPrint(KernelInfo *kernel)
{
  unsigned long
    i, u, v;

  fprintf(stderr,
        "Kernel \"%s\" of size %lux%lu%+ld%+ld  with value from %lg to %lg\n",
        MagickOptionToMnemonic(MagickKernelOptions, kernel->type),
        kernel->width, kernel->height,
        kernel->offset_x, kernel->offset_y,
        kernel->value_min, kernel->value_max);
  fprintf(stderr, "Forming convolution output range from %lg to %lg%s\n",
        kernel->range_neg, kernel->range_pos,
        kernel->normalized == MagickTrue ? " (normalized)" : "" );
  for (i=v=0; v < kernel->height; v++) {
    fprintf(stderr,"%2ld:",v);
    for (u=0; u < kernel->width; u++, i++)
      if ( IsNan(kernel->values[i]) )
        fprintf(stderr," %*s", GetMagickPrecision()+2, "nan");
      else
        fprintf(stderr," %.*lg", GetMagickPrecision(), kernel->values[i]);
    fprintf(stderr,"\n");
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     K e r n e l R o t a t e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  KernelRotate() rotates the kernel by the angle given.  Currently it is
%  restricted to 90 degree angles, but this may be improved in the future.
%
%  The format of the KernelRotate method is:
%
%      void KernelRotate (KernelInfo *kernel, double angle)
%
%  A description of each parameter follows:
%
%    o kernel: the Morphology/Convolution kernel
%
%    o angle: angle to rotate in degrees
%
*/
MagickExport void KernelRotate(KernelInfo *kernel, double angle)
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
      /* for a 180 degree rotation - also know as a reflection */
      /* This is actually a very very common operation! */
      /* basically this is a reverse of the kernel data! */
      unsigned long
        i,j;
      register double
        *k,t;

      k=kernel->values;
      for ( i=0, j=kernel->width*kernel->height-1;  i<j;  i++, j--)
        t=k[i],  k[i]=k[j],  k[j]=t;

      kernel->offset_x = kernel->width - kernel->offset_x - 1;
      kernel->offset_y = kernel->width - kernel->offset_y - 1;
      angle = fmod(angle+180.0, 360.0);
    }
  if ( 45.0 < angle && angle <= 135.0 )
    { /* Do a transpose and a flop, of the image, which results in a 90
       * degree rotation using two mirror operations.
       *
       * WARNING: this assumes the original image was a 1 dimentional image
       * but currently that is the only built-ins it is applied to.
       */
      unsigned long
        t;
      t = kernel->width;
      kernel->width = kernel->height;
      kernel->height = t;
      t = kernel->offset_x;
      kernel->offset_x = kernel->offset_y;
      kernel->offset_y = t;
      angle = fmod(450.0 - angle, 360.0);
    }
  /* at this point angle should be between -45 (315) and +45 degrees */

#if 0
    { /* Do a flop, this assumes kernel is horizontally symetrical.
       * Each kernel data row need to be reversed!
       * This is currently not used, but by be used in the future.
       */
      unsigned long
        y;
      register unsigned long
        x,r;
      register double
        *k,t;

      for ( y=0, k=kernel->values; y < kernel->height; y++, k+=kernel->width)
        for ( x=0, r=kernel->width-1; x<kernel->width/2; x++, r--)
          t=k[x],  k[x]=k[r],  k[r]=t;

      kernel->offset_x = kernel->width - kernel->offset_x - 1;
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
%      Image *MorphologyImage(const Image *image, MorphologyMethod method,
%        const long iterations, KernelInfo *kernel, ExceptionInfo *exception)
%      Image *MorphologyImageChannel(const Image *image, const ChannelType
%        channel, MorphologyMethod method, const long iterations, KernelInfo
%        *kernel, ExceptionInfo *exception)
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

/* incr change if the value being assigned changed */
#define Assign(channel,value) \
  { q->channel = ClampToQuantum(value); \
    if ( p[r].channel != q->channel ) changed++; \
  }
#define AssignIndex(value) \
  { q_indexes[x] = ClampToQuantum(value); \
    if ( p_indexes[r] != q_indexes[x] ) changed++; \
  }

/* Internal function
 * Apply the Morphology method with the given Kernel
 * And return the number of values changed.
 */
static unsigned long MorphologyApply(const Image *image, Image
     *result_image, const MorphologyMethod method, const ChannelType channel,
     const KernelInfo *kernel, ExceptionInfo *exception)
{
#define MorphologyTag  "Morphology/Image"

  long
    progress;

  unsigned long
    y, offx, offy,
    changed;

  MagickBooleanType
    status;

  MagickPixelPacket
    bias;

  CacheView
    *p_view,
    *q_view;

  /*
    Apply Morphology to Image.
  */
  status=MagickTrue;
  changed=0;
  progress=0;

  GetMagickPixelPacket(image,&bias);
  SetMagickPixelPacketBias(image,&bias);

  p_view=AcquireCacheView(image);
  q_view=AcquireCacheView(result_image);

  /* some methods (including convolve) needs to apply a reflected kernel
   * the offset for getting the kernel view needs to be adjusted for this
   * situation.
   */
  offx = kernel->offset_x;
  offy = kernel->offset_y;
  switch(method) {
    case ErodeMorphology:
    case ErodeIntensityMorphology:
      /* kernel is not reflected */
      break;
    default:
      /* kernel needs to be reflected */
      offx = kernel->width-offx-1;
      offy = kernel->height-offy-1;
      break;
  }

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < image->rows; y++)
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

    register unsigned long
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

    for (x=0; x < image->columns; x++)
    {
       unsigned long
        v;

      register unsigned long
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
       * This removes need for 'cloning' new images
       */
      *q = p[r];
      if (image->colorspace == CMYKColorspace)
        q_indexes[x] = p_indexes[r];

      result.index=0; /* stop compiler warnings */
      switch (method) {
        case ConvolveMorphology:
          result=bias;
          break;  /* default result is the convolution bias */
#if 0
removed as it caused problems with change
Really we want to set each to the first value found
but not up date change if that is the first value!
        case DialateMorphology:
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
#endif
        case DialateIntensityMorphology:
        case ErodeIntensityMorphology:
          result.red     = 0.0; /* was initial pixel found ? */
          break;
        default:
          /* Otherwise just start with the original pixel value */
          result.red     = p[r].red;
          result.green   = p[r].green;
          result.blue    = p[r].blue;
          result.opacity = QuantumRange - p[r].opacity;
          if ( image->colorspace == CMYKColorspace)
             result.index   = p_indexes[r];
          break;
      }

      switch ( method ) {
        case ConvolveMorphology:
            /* Weighted Average of pixels
             *
             * NOTE for correct working of this operation for asymetrical
             * kernels, the kernel needs to be applied in its reflected form.
             * That is its values needs to be reversed.
             */
            if (((channel & OpacityChannel) == 0) ||
                      (image->matte == MagickFalse))
              {
                /* Convolution (no transparency) */
                k = &kernel->values[ kernel->width*kernel->height-1 ];
                k_pixels = p;
                k_indexes = p_indexes;
                for (v=0; v < kernel->height; v++) {
                  for (u=0; u < kernel->width; u++, k--) {
                    if ( IsNan(*k) ) continue;
                    result.red     += (*k)*k_pixels[u].red;
                    result.green   += (*k)*k_pixels[u].green;
                    result.blue    += (*k)*k_pixels[u].blue;
                    /* result.opacity += no involvment */
                    if ( image->colorspace == CMYKColorspace)
                      result.index   += (*k)*k_indexes[u];
                  }
                  k_pixels += image->columns+kernel->width;
                  k_indexes += image->columns+kernel->width;
                }
                if ((channel & RedChannel) != 0)
                  Assign(red,result.red);
                if ((channel & GreenChannel) != 0)
                  Assign(green,result.green);
                if ((channel & BlueChannel) != 0)
                  Assign(blue,result.blue);
                /* no transparency involved */
                if ((channel & IndexChannel) != 0
                    && image->colorspace == CMYKColorspace)
                  AssignIndex(result.index);
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
                for (v=0; v < kernel->height; v++) {
                  for (u=0; u < kernel->width; u++, k--) {
                    if ( IsNan(*k) ) continue;
                    alpha=(*k)*(QuantumScale*(QuantumRange-
                                          k_pixels[u].opacity));
                    gamma += alpha;
                    result.red     += alpha*k_pixels[u].red;
                    result.green   += alpha*k_pixels[u].green;
                    result.blue    += alpha*k_pixels[u].blue;
                    result.opacity += (*k)*k_pixels[u].opacity;
                    if ( image->colorspace == CMYKColorspace)
                      result.index   += alpha*k_indexes[u];
                  }
                  k_pixels += image->columns+kernel->width;
                  k_indexes += image->columns+kernel->width;
                }
                gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
                if ((channel & RedChannel) != 0)
                  Assign(red,gamma*result.red);
                if ((channel & GreenChannel) != 0)
                  Assign(green,gamma*result.green);
                if ((channel & BlueChannel) != 0)
                  Assign(blue,gamma*result.blue);
                if ((channel & OpacityChannel) != 0
                    && image->matte == MagickTrue )
                  Assign(opacity,result.opacity);
                if ((channel & IndexChannel) != 0
                    && image->colorspace == CMYKColorspace)
                  AssignIndex(gamma*result.index);
              }
            break;

        case DialateMorphology:
            /* Maximize Value within kernel shape
             *
             * NOTE for correct working of this operation for asymetrical
             * kernels, the kernel needs to be applied in its reflected form.
             * That is its values needs to be reversed.
             *
             * NOTE: in normal Greyscale Morphology, the kernel value should
             * be added to the real value, this is currently not done, due to
             * the nature of the boolean kernels being used.
             *
             */
            k = &kernel->values[ kernel->width*kernel->height-1 ];
            k_pixels = p;
            k_indexes = p_indexes;
            for (v=0; v < kernel->height; v++) {
              for (u=0; u < kernel->width; u++, k--) {
                if ( IsNan(*k) || (*k) < 0.5 ) continue;
                Maximize(result.red,     k_pixels[u].red);
                Maximize(result.green,   k_pixels[u].green);
                Maximize(result.blue,    k_pixels[u].blue);
                Maximize(result.opacity, QuantumRange-k_pixels[u].opacity);
                if ( image->colorspace == CMYKColorspace)
                  Maximize(result.index,   k_indexes[u]);
              }
              k_pixels += image->columns+kernel->width;
              k_indexes += image->columns+kernel->width;
            }
            if ((channel & RedChannel) != 0)
              Assign(red,result.red);
            if ((channel & GreenChannel) != 0)
              Assign(green,result.green);
            if ((channel & BlueChannel) != 0)
              Assign(blue,result.blue);
            if ((channel & OpacityChannel) != 0
                 && image->matte == MagickTrue )
              Assign(opacity,QuantumRange-result.opacity);
            if ((channel & IndexChannel) != 0
                && image->colorspace == CMYKColorspace)
              AssignIndex(result.index);
            break;

        case ErodeMorphology:
            /* Minimize Value within kernel shape
             *
             * NOTE that the kernel is not reflected for this operation!
             *
             * NOTE: in normal Greyscale Morphology, the kernel value should
             * be added to the real value, this is currently not done, due to
             * the nature of the boolean kernels being used.
             */
            k = kernel->values;
            k_pixels = p;
            k_indexes = p_indexes;
            for (v=0; v < kernel->height; v++) {
              for (u=0; u < kernel->width; u++, k++) {
                if ( IsNan(*k) || (*k) < 0.5 ) continue;
                Minimize(result.red,     k_pixels[u].red);
                Minimize(result.green,   k_pixels[u].green);
                Minimize(result.blue,    k_pixels[u].blue);
                Minimize(result.opacity, QuantumRange-k_pixels[u].opacity);
                if ( image->colorspace == CMYKColorspace)
                  Minimize(result.index,   k_indexes[u]);
              }
              k_pixels += image->columns+kernel->width;
              k_indexes += image->columns+kernel->width;
            }
            if ((channel & RedChannel) != 0)
              Assign(red,result.red);
            if ((channel & GreenChannel) != 0)
              Assign(green,result.green);
            if ((channel & BlueChannel) != 0)
              Assign(blue,result.blue);
            if ((channel & OpacityChannel) != 0
                 && image->matte == MagickTrue )
              Assign(opacity,QuantumRange-result.opacity);
            if ((channel & IndexChannel) != 0
                && image->colorspace == CMYKColorspace)
              AssignIndex(result.index);
            break;

        case DialateIntensityMorphology:
            /* Select pixel with maximum intensity within kernel shape
             *
             * WARNING: the intensity test fails for CMYK and does not
             * take into account the moderating effect of teh alpha channel
             * on the intensity.
             *
             * NOTE for correct working of this operation for asymetrical
             * kernels, the kernel needs to be applied in its reflected form.
             * That is its values needs to be reversed.
             */
            k = &kernel->values[ kernel->width*kernel->height-1 ];
            k_pixels = p;
            k_indexes = p_indexes;
            for (v=0; v < kernel->height; v++) {
              for (u=0; u < kernel->width; u++, k--) {
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

        case ErodeIntensityMorphology:
            /* Select pixel with mimimum intensity within kernel shape
             *
             * WARNING: the intensity test fails for CMYK and does not
             * take into account the moderating effect of teh alpha channel
             * on the intensity.
             *
             * NOTE that the kernel is not reflected for this operation!
             */
            k = kernel->values;
            k_pixels = p;
            k_indexes = p_indexes;
            for (v=0; v < kernel->height; v++) {
              for (u=0; u < kernel->width; u++, k++) {
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

        case DistanceMorphology:
          /* Add kernel value and select the minimum value found.
           * The result is a iterative distance from edge function.
           *
           * All Distance Kernels are symetrical, but that may not always
           * be the case. For example how about a distance from left edges?
           * To make it work correctly for asymetrical kernels the reflected
           * kernel needs to be applied.
           */
#if 0
          /* No need to do distance morphology if all values are zero */
          /* Unfortunatally I have not been able to get this right! */
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
            for (v=0; v < kernel->height; v++) {
              for (u=0; u < kernel->width; u++, k--) {
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
#if 1
            if ((channel & RedChannel) != 0)
              Assign(red,result.red);
            if ((channel & GreenChannel) != 0)
              Assign(green,result.green);
            if ((channel & BlueChannel) != 0)
              Assign(blue,result.blue);
            if ((channel & OpacityChannel) != 0
                 && image->matte == MagickTrue )
              Assign(opacity,QuantumRange-result.opacity);
            if ((channel & IndexChannel) != 0
                && image->colorspace == CMYKColorspace)
              AssignIndex(result.index);
#else
            /* By returning the number of 'maximum' values still to process
            ** we can get the Distance iteration to finish faster.
            ** BUT this may cause an infinite loop on very large shapes,
            ** which may have a distance gradient that reachs max value!
            */
            if ((channel & RedChannel) != 0)
              { q->red = ClampToQuantum(result.red);
                if ( q->red == QuantumRange ) changed++; /* more to do */
              }
            if ((channel & GreenChannel) != 0)
              { q->green = ClampToQuantum(result.green);
                if ( q->green == QuantumRange ) changed++; /* more to do */
              }
            if ((channel & BlueChannel) != 0)
              { q->blue = ClampToQuantum(result.blue);
                if ( q->blue == QuantumRange ) changed++; /* more to do */
              }
            if ((channel & OpacityChannel) != 0)
              { q->opacity = ClampToQuantum(QuantumRange-result.opacity);
                if ( q->opacity == 0 ) changed++; /* more to do */
              }
            if (((channel & IndexChannel) != 0) &&
                (image->colorspace == CMYKColorspace))
              { q_indexes[x] = ClampToQuantum(result.index);
                if ( q_indexes[x] == QuantumRange ) changed++;
              }
#endif
            break;

        case UndefinedMorphology:
        default:
            break; /* Do nothing */
        }
      p++;
      q++;
    }
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
  }
  result_image->type=image->type;
  q_view=DestroyCacheView(q_view);
  p_view=DestroyCacheView(p_view);
  return(status ? changed : 0);
}

MagickExport Image *MorphologyImage(const Image *image,MorphologyMethod method,
  const long iterations,KernelInfo *kernel, ExceptionInfo *exception)
{
  Image
    *morphology_image;

  morphology_image=MorphologyImageChannel(image,DefaultChannels,method,
    iterations,kernel,exception);
  return(morphology_image);
}

MagickExport Image *MorphologyImageChannel(const Image *image,
  const ChannelType channel, MorphologyMethod method, const long iterations,
  KernelInfo *kernel, ExceptionInfo *exception)
{
  unsigned long
    count,
    limit,
    changed;

  Image
    *new_image,
    *old_image;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  if ( GetImageArtifact(image,"showkernel") != (const char *) NULL)
    KernelPrint(kernel);

  if ( iterations == 0 )
    return((Image *)NULL); /* null operation - nothing to do! */

  /* kernel must be valid at this point
   * (except maybe for posible future morphology methods like "Prune"
   */
  assert(kernel != (KernelInfo *)NULL);

  count = 0;
  changed = 1;
  limit = iterations;
  if ( iterations < 0 )
    limit = image->columns > image->rows ? image->columns : image->rows;

  /* Special morphology cases */
  switch( method ) {
    case CloseMorphology:
      new_image = MorphologyImageChannel(image, channel, DialateMorphology,
            iterations, kernel, exception);
      if (new_image == (Image *) NULL)
        return((Image *) NULL);
      method = ErodeMorphology;
      break;
    case OpenMorphology:
      new_image = MorphologyImageChannel(image, channel, ErodeMorphology,
            iterations, kernel, exception);
      if (new_image == (Image *) NULL)
        return((Image *) NULL);
      method = DialateMorphology;
      break;
    case CloseIntensityMorphology:
      new_image = MorphologyImageChannel(image, channel, DialateIntensityMorphology,
            iterations, kernel, exception);
      if (new_image == (Image *) NULL)
        return((Image *) NULL);
      method = ErodeIntensityMorphology;
      break;
    case OpenIntensityMorphology:
      new_image = MorphologyImageChannel(image, channel, ErodeIntensityMorphology,
            iterations, kernel, exception);
      if (new_image == (Image *) NULL)
        return((Image *) NULL);
      method = DialateIntensityMorphology;
      break;

    case ConvolveMorphology:
      KernelNormalize(kernel);
      /* FALL-THRU */
    default:
      /* Do a morphology just once at this point!
        This ensures a new_image has been generated, but allows us
        to skip the creation of 'old_image' if it isn't needed.
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
      changed = MorphologyApply(image,new_image,method,channel,kernel,
            exception);
      count++;
      if ( GetImageArtifact(image,"verbose") != (const char *) NULL )
        fprintf(stderr, "Morphology %s:%lu => Changed %lu\n",
              MagickOptionToMnemonic(MagickMorphologyOptions, method),
              count, changed);
  }

  /* Repeat the interative morphology until count or no change */
  if ( count < limit && changed > 0 ) {
    old_image = CloneImage(new_image,0,0,MagickTrue,exception);
    if (old_image == (Image *) NULL)
        return(DestroyImage(new_image));
    if (SetImageStorageClass(old_image,DirectClass) == MagickFalse)
      {
        InheritException(exception,&old_image->exception);
        old_image=DestroyImage(old_image);
        return(DestroyImage(new_image));
      }
    while( count < limit && changed != 0 )
      {
        Image *tmp = old_image;
        old_image = new_image;
        new_image = tmp;
        changed = MorphologyApply(old_image,new_image,method,channel,kernel,
              exception);
        count++;
        if ( GetImageArtifact(image,"verbose") != (const char *) NULL )
          fprintf(stderr, "Morphology %s:%lu => Changed %lu\n",
                MagickOptionToMnemonic(MagickMorphologyOptions, method),
                count, changed);
      }
    DestroyImage(old_image);
  }

  return(new_image);
}


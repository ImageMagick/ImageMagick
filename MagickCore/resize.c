/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                 RRRR   EEEEE  SSSSS  IIIII  ZZZZZ  EEEEE                    %
%                 R   R  E      SS       I       ZZ  E                        %
%                 RRRR   EEE     SSS     I     ZZZ   EEE                      %
%                 R R    E         SS    I    ZZ     E                        %
%                 R  R   EEEEE  SSSSS  IIIII  ZZZZZ  EEEEE                    %
%                                                                             %
%                                                                             %
%                      MagickCore Image Resize Methods                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/accelerate-private.h"
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/magick.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resample.h"
#include "MagickCore/resample-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resize-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"
#if defined(MAGICKCORE_LQR_DELEGATE)
#include <lqr.h>
#endif

/*
  Typedef declarations.
*/
struct _ResizeFilter
{
  double
    (*filter)(const double,const ResizeFilter *),
    (*window)(const double,const ResizeFilter *),
    support,        /* filter region of support - the filter support limit */
    window_support, /* window support, usually equal to support (expert only) */
    scale,          /* dimension scaling to fit window support (usually 1.0) */
    blur,           /* x-scale (blur-sharpen) */
    coefficient[7]; /* cubic coefficients for BC-cubic filters */

  ResizeWeightingFunctionType
    filterWeightingType,
    windowWeightingType;

  size_t
    signature;
};

/*
  Forward declarations.
*/
static double
  I0(double x),
  BesselOrderOne(double),
  Sinc(const double, const ResizeFilter *),
  SincFast(const double, const ResizeFilter *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   F i l t e r F u n c t i o n s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  These are the various filter and windowing functions that are provided.
%
%  They are internal to this module only.  See AcquireResizeFilterInfo() for
%  details of the access to these functions, via the GetResizeFilterSupport()
%  and GetResizeFilterWeight() API interface.
%
%  The individual filter functions have this format...
%
%     static MagickRealtype *FilterName(const double x,const double support)
%
%  A description of each parameter follows:
%
%    o x: the distance from the sampling point generally in the range of 0 to
%      support.  The GetResizeFilterWeight() ensures this a positive value.
%
%    o resize_filter: current filter information.  This allows function to
%      access support, and possibly other pre-calculated information defining
%      the functions.
%
*/

static double Blackman(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    Blackman: 2nd order cosine windowing function:
      0.42 + 0.5 cos(pi x) + 0.08 cos(2pi x)

    Refactored by Chantal Racette and Nicolas Robidoux to one trig call and
    five flops.
  */
  const double cosine = cos((double) (MagickPI*x));
  magick_unreferenced(resize_filter);
  return(0.34+cosine*(0.5+cosine*0.16));
}

static double Bohman(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    Bohman: 2rd Order cosine windowing function:
      (1-x) cos(pi x) + sin(pi x) / pi.

    Refactored by Nicolas Robidoux to one trig call, one sqrt call, and 7 flops,
    taking advantage of the fact that the support of Bohman is 1.0 (so that we
    know that sin(pi x) >= 0).
  */
  const double cosine = cos((double) (MagickPI*x));
  const double sine=sqrt(1.0-cosine*cosine);
  magick_unreferenced(resize_filter);
  return((1.0-x)*cosine+(1.0/MagickPI)*sine);
}

static double Box(const double magick_unused(x),
  const ResizeFilter *magick_unused(resize_filter))
{
  magick_unreferenced(x);
  magick_unreferenced(resize_filter);

  /*
    A Box filter is a equal weighting function (all weights equal).
    DO NOT LIMIT results by support or resize point sampling will work
    as it requests points beyond its normal 0.0 support size.
  */
  return(1.0);
}

static double Cosine(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  magick_unreferenced(resize_filter);

  /*
    Cosine window function:
      cos((pi/2)*x).
  */
  return(cos((double) (MagickPI2*x)));
}

static double CubicBC(const double x,const ResizeFilter *resize_filter)
{
  /*
    Cubic Filters using B,C determined values:
       Mitchell-Netravali  B = 1/3 C = 1/3  "Balanced" cubic spline filter
       Catmull-Rom         B = 0   C = 1/2  Interpolatory and exact on linears
       Spline              B = 1   C = 0    B-Spline Gaussian approximation
       Hermite             B = 0   C = 0    B-Spline interpolator

    See paper by Mitchell and Netravali, Reconstruction Filters in Computer
    Graphics Computer Graphics, Volume 22, Number 4, August 1988
    http://www.cs.utexas.edu/users/fussell/courses/cs384g/lectures/mitchell/
    Mitchell.pdf.

    Coefficients are determined from B,C values:
       P0 = (  6 - 2*B       )/6 = coeff[0]
       P1 =         0
       P2 = (-18 +12*B + 6*C )/6 = coeff[1]
       P3 = ( 12 - 9*B - 6*C )/6 = coeff[2]
       Q0 = (      8*B +24*C )/6 = coeff[3]
       Q1 = (    -12*B -48*C )/6 = coeff[4]
       Q2 = (      6*B +30*C )/6 = coeff[5]
       Q3 = (    - 1*B - 6*C )/6 = coeff[6]

    which are used to define the filter:

       P0 + P1*x + P2*x^2 + P3*x^3      0 <= x < 1
       Q0 + Q1*x + Q2*x^2 + Q3*x^3      1 <= x < 2

    which ensures function is continuous in value and derivative (slope).
  */
  if (x < 1.0)
    return(resize_filter->coefficient[0]+x*(x*
      (resize_filter->coefficient[1]+x*resize_filter->coefficient[2])));
  if (x < 2.0)
    return(resize_filter->coefficient[3]+x*(resize_filter->coefficient[4]+x*
      (resize_filter->coefficient[5]+x*resize_filter->coefficient[6])));
  return(0.0);
}

static double CubicSpline(const double x,const ResizeFilter *resize_filter)
{
  if (resize_filter->support <= 2.0)
    {
      /*
        2-lobe Spline filter.
      */
      if (x < 1.0)
        return(((x-9.0/5.0)*x-1.0/5.0)*x+1.0);
      if (x < 2.0)
        return(((-1.0/3.0*(x-1.0)+4.0/5.0)*(x-1.0)-7.0/15.0)*(x-1.0));
      return(0.0);
    }
  if (resize_filter->support <= 3.0)
    {
      /*
        3-lobe Spline filter.
      */
      if (x < 1.0)
        return(((13.0/11.0*x-453.0/209.0)*x-3.0/209.0)*x+1.0);
      if (x < 2.0)
        return(((-6.0/11.0*(x-1.0)+270.0/209.0)*(x-1.0)-156.0/209.0)*(x-1.0));
      if (x < 3.0)
        return(((1.0/11.0*(x-2.0)-45.0/209.0)*(x-2.0)+26.0/209.0)*(x-2.0));
      return(0.0);
    }
  /*
    4-lobe Spline filter.
  */
  if (x < 1.0)
    return(((49.0/41.0*x-6387.0/2911.0)*x-3.0/2911.0)*x+1.0);
  if (x < 2.0)
    return(((-24.0/41.0*(x-1.0)+4032.0/2911.0)*(x-1.0)-2328.0/2911.0)*(x-1.0));
  if (x < 3.0)
    return(((6.0/41.0*(x-2.0)-1008.0/2911.0)*(x-2.0)+582.0/2911.0)*(x-2.0));
  if (x < 4.0)
    return(((-1.0/41.0*(x-3.0)+168.0/2911.0)*(x-3.0)-97.0/2911.0)*(x-3.0));
  return(0.0);
}

static double Gaussian(const double x,const ResizeFilter *resize_filter)
{
  /*
    Gaussian with a sigma = 1/2 (or as user specified)

    Gaussian Formula (1D) ...
        exp( -(x^2)/((2.0*sigma^2) ) / (sqrt(2*PI)*sigma^2))

    Gaussian Formula (2D) ...
        exp( -(x^2+y^2)/(2.0*sigma^2) ) / (PI*sigma^2) )
    or for radius
        exp( -(r^2)/(2.0*sigma^2) ) / (PI*sigma^2) )

    Note that it is only a change from 1-d to radial form is in the
    normalization multiplier which is not needed or used when Gaussian is used
    as a filter.

    The constants are pre-calculated...

        coeff[0]=sigma;
        coeff[1]=1.0/(2.0*sigma^2);
        coeff[2]=1.0/(sqrt(2*PI)*sigma^2);

        exp( -coeff[1]*(x^2)) ) * coeff[2];

    However the multiplier coeff[1] is need, the others are informative only.

    This separates the gaussian 'sigma' value from the 'blur/support'
    settings allowing for its use in special 'small sigma' gaussians,
    without the filter 'missing' pixels because the support becomes too
    small.
  */
  return(exp((double)(-resize_filter->coefficient[1]*x*x)));
}

static double Hann(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    Cosine window function:
      0.5+0.5*cos(pi*x).
  */
  const double cosine = cos((double) (MagickPI*x));
  magick_unreferenced(resize_filter);
  return(0.5+0.5*cosine);
}

static double Hamming(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    Offset cosine window function:
     .54 + .46 cos(pi x).
  */
  const double cosine = cos((double) (MagickPI*x));
  magick_unreferenced(resize_filter);
  return(0.54+0.46*cosine);
}

static double Jinc(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  magick_unreferenced(resize_filter);

  /*
    See Pratt "Digital Image Processing" p.97 for Jinc/Bessel functions.
    http://mathworld.wolfram.com/JincFunction.html and page 11 of
    http://www.ph.ed.ac.uk/%7ewjh/teaching/mo/slides/lens/lens.pdf

    The original "zoom" program by Paul Heckbert called this "Bessel".  But
    really it is more accurately named "Jinc".
  */
  if (x == 0.0)
    return(0.5*MagickPI);
  return(BesselOrderOne(MagickPI*x)/x);
}

static double Kaiser(const double x,const ResizeFilter *resize_filter)
{
  /*
    Kaiser Windowing Function (bessel windowing)

       I0( beta * sqrt( 1-x^2) ) / IO(0)

    Beta (coeff[0]) is a free value from 5 to 8 (defaults to 6.5).
    However it is typically defined in terms of Alpha*PI

    The normalization factor (coeff[1]) is not actually needed,
    but without it the filters has a large value at x=0 making it
    difficult to compare the function with other windowing functions.
  */
  return(resize_filter->coefficient[1]*I0(resize_filter->coefficient[0]*
    sqrt((double) (1.0-x*x))));
}

static double Lagrange(const double x,const ResizeFilter *resize_filter)
{
  double
    value;

  ssize_t
    i;

  ssize_t
    n,
    order;

  /*
    Lagrange piecewise polynomial fit of sinc: N is the 'order' of the lagrange
    function and depends on the overall support window size of the filter. That
    is: for a support of 2, it gives a lagrange-4 (piecewise cubic function).

    "n" identifies the piece of the piecewise polynomial.

    See Survey: Interpolation Methods, IEEE Transactions on Medical Imaging,
    Vol 18, No 11, November 1999, p1049-1075, -- Equation 27 on p1064.
  */
  if (x > resize_filter->support)
    return(0.0);
  order=(ssize_t) (2.0*resize_filter->window_support);  /* number of pieces */
  n=(ssize_t) (resize_filter->window_support+x);
  value=1.0f;
  for (i=0; i < order; i++)
    if (i != n)
      value*=(n-i-x)/(n-i);
  return(value);
}

static double MagicKernelSharp2013(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  magick_unreferenced(resize_filter);

  /*
    Magic Kernel with Sharp 2013 filter.

    See "Solving the mystery of Magic Kernel Sharp"
    (https://johncostella.com/magic/mks.pdf)
  */
  if (x < 0.5)
    return(0.625+1.75*(0.5-x)*(0.5+x));
  if (x < 1.5)
    return((1.0-x)*(1.75-x));
  if (x < 2.5)
    return(-0.125*(2.5-x)*(2.5-x));
  return(0.0);
}

static double MagicKernelSharp2021(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  magick_unreferenced(resize_filter);

  /*
    Magic Kernel with Sharp 2021 filter.

    See "Solving the mystery of Magic Kernel Sharp"
    (https://johncostella.com/magic/mks.pdf)
  */
  if (x < 0.5)
    return(577.0/576.0-239.0/144.0*x*x);
  if (x < 1.5)
    return(35.0/36.0*(x-1.0)*(x-239.0/140.0));
  if (x < 2.5)
    return(1.0/6.0*(x-2.0)*(65.0/24.0-x));
  if (x < 3.5)
    return(1.0/36.0*(x-3.0)*(x-3.75));
  if (x < 4.5)
    return(-1.0/288.0*(x-4.5)*(x-4.5));
  return(0.0);
}

static double Quadratic(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  magick_unreferenced(resize_filter);

  /*
    2rd order (quadratic) B-Spline approximation of Gaussian.
  */
  if (x < 0.5)
    return(0.75-x*x);
  if (x < 1.5)
    return(0.5*(x-1.5)*(x-1.5));
  return(0.0);
}

static double Sinc(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  magick_unreferenced(resize_filter);

  /*
    Scaled sinc(x) function using a trig call:
      sinc(x) == sin(pi x)/(pi x).
  */
  if (x != 0.0)
    {
      const double alpha=(double) (MagickPI*x);
      return(sin((double) alpha)/alpha);
    }
  return((double) 1.0);
}

static double SincFast(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  magick_unreferenced(resize_filter);

  /*
    Approximations of the sinc function sin(pi x)/(pi x) over the interval
    [-4,4] constructed by Nicolas Robidoux and Chantal Racette with funding
    from the Natural Sciences and Engineering Research Council of Canada.

    Although the approximations are polynomials (for low order of
    approximation) and quotients of polynomials (for higher order of
    approximation) and consequently are similar in form to Taylor polynomials /
    Pade approximants, the approximations are computed with a completely
    different technique.

    Summary: These approximations are "the best" in terms of bang (accuracy)
    for the buck (flops). More specifically: Among the polynomial quotients
    that can be computed using a fixed number of flops (with a given "+ - * /
    budget"), the chosen polynomial quotient is the one closest to the
    approximated function with respect to maximum absolute relative error over
    the given interval.

    The Remez algorithm, as implemented in the boost library's minimax package,
    is the key to the construction: http://www.boost.org/doc/libs/1_36_0/libs/
    math/doc/sf_and_dist/html/math_toolkit/backgrounders/remez.html

    If outside of the interval of approximation, use the standard trig formula.
  */
  if (x > 4.0)
    {
      const double alpha=(double) (MagickPI*x);
      return(sin((double) alpha)/alpha);
    }
  {
    /*
      The approximations only depend on x^2 (sinc is an even function).
    */
    const double xx = x*x;
#if MAGICKCORE_QUANTUM_DEPTH <= 8
    /*
      Maximum absolute relative error 6.3e-6 < 1/2^17.
    */
    const double c0 = 0.173610016489197553621906385078711564924e-2L;
    const double c1 = -0.384186115075660162081071290162149315834e-3L;
    const double c2 = 0.393684603287860108352720146121813443561e-4L;
    const double c3 = -0.248947210682259168029030370205389323899e-5L;
    const double c4 = 0.107791837839662283066379987646635416692e-6L;
    const double c5 = -0.324874073895735800961260474028013982211e-8L;
    const double c6 = 0.628155216606695311524920882748052490116e-10L;
    const double c7 = -0.586110644039348333520104379959307242711e-12L;
    const double p =
      c0+xx*(c1+xx*(c2+xx*(c3+xx*(c4+xx*(c5+xx*(c6+xx*c7))))));
    return((xx-1.0)*(xx-4.0)*(xx-9.0)*(xx-16.0)*p);
#elif MAGICKCORE_QUANTUM_DEPTH <= 16
    /*
      Max. abs. rel. error 2.2e-8 < 1/2^25.
    */
    const double c0 = 0.173611107357320220183368594093166520811e-2L;
    const double c1 = -0.384240921114946632192116762889211361285e-3L;
    const double c2 = 0.394201182359318128221229891724947048771e-4L;
    const double c3 = -0.250963301609117217660068889165550534856e-5L;
    const double c4 = 0.111902032818095784414237782071368805120e-6L;
    const double c5 = -0.372895101408779549368465614321137048875e-8L;
    const double c6 = 0.957694196677572570319816780188718518330e-10L;
    const double c7 = -0.187208577776590710853865174371617338991e-11L;
    const double c8 = 0.253524321426864752676094495396308636823e-13L;
    const double c9 = -0.177084805010701112639035485248501049364e-15L;
    const double p =
      c0+xx*(c1+xx*(c2+xx*(c3+xx*(c4+xx*(c5+xx*(c6+xx*(c7+xx*(c8+xx*c9))))))));
    return((xx-1.0)*(xx-4.0)*(xx-9.0)*(xx-16.0)*p);
#else
    /*
      Max. abs. rel. error 1.2e-12 < 1/2^39.
    */
    const double c0 = 0.173611111110910715186413700076827593074e-2L;
    const double c1 = -0.289105544717893415815859968653611245425e-3L;
    const double c2 = 0.206952161241815727624413291940849294025e-4L;
    const double c3 = -0.834446180169727178193268528095341741698e-6L;
    const double c4 = 0.207010104171026718629622453275917944941e-7L;
    const double c5 = -0.319724784938507108101517564300855542655e-9L;
    const double c6 = 0.288101675249103266147006509214934493930e-11L;
    const double c7 = -0.118218971804934245819960233886876537953e-13L;
    const double p =
      c0+xx*(c1+xx*(c2+xx*(c3+xx*(c4+xx*(c5+xx*(c6+xx*c7))))));
    const double d0 = 1.0L;
    const double d1 = 0.547981619622284827495856984100563583948e-1L;
    const double d2 = 0.134226268835357312626304688047086921806e-2L;
    const double d3 = 0.178994697503371051002463656833597608689e-4L;
    const double d4 = 0.114633394140438168641246022557689759090e-6L;
    const double q = d0+xx*(d1+xx*(d2+xx*(d3+xx*d4)));
    return((xx-1.0)*(xx-4.0)*(xx-9.0)*(xx-16.0)/q*p);
#endif
  }
}

static double Triangle(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  magick_unreferenced(resize_filter);

  /*
    1st order (linear) B-Spline, bilinear interpolation, Tent 1D filter, or
    a Bartlett 2D Cone filter.  Also used as a Bartlett Windowing function
    for Sinc().
  */
  if (x < 1.0)
    return(1.0-x);
  return(0.0);
}

static double Welch(const double x,
  const ResizeFilter *magick_unused(resize_filter))
{
  magick_unreferenced(resize_filter);

  /*
    Welch parabolic windowing filter.
  */
  if (x < 1.0)
    return(1.0-x*x);
  return(0.0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e R e s i z e F i l t e r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireResizeFilter() allocates the ResizeFilter structure.  Choose from
%  these filters:
%
%  FIR (Finite impulse Response) Filters
%      Box         Triangle   Quadratic
%      Spline      Hermite    Catrom
%      Mitchell
%
%  IIR (Infinite impulse Response) Filters
%      Gaussian     Sinc        Jinc (Bessel)
%
%  Windowed Sinc/Jinc Filters
%      Blackman    Bohman     Lanczos
%      Hann        Hamming    Cosine
%      Kaiser      Welch      Parzen
%      Bartlett
%
%  Special Purpose Filters
%      Cubic  SincFast  LanczosSharp  Lanczos2  Lanczos2Sharp
%      Robidoux RobidouxSharp MagicKernelSharp2013 MagicKernelSharp2021
%
%  The users "-filter" selection is used to lookup the default 'expert'
%  settings for that filter from a internal table.  However any provided
%  'expert' settings (see below) may override this selection.
%
%  FIR filters are used as is, and are limited to that filters support window
%  (unless over-ridden).  'Gaussian' while classed as an IIR filter, is also
%  simply clipped by its support size (currently 1.5 or approximately 3*sigma
%  as recommended by many references)
%
%  The special a 'cylindrical' filter flag will promote the default 4-lobed
%  Windowed Sinc filter to a 3-lobed Windowed Jinc equivalent, which is better
%  suited to this style of image resampling. This typically happens when using
%  such a filter for images distortions.
%
%  SPECIFIC FILTERS:
%
%  Directly requesting 'Sinc', 'Jinc' function as a filter will force the use
%  of function without any windowing, or promotion for cylindrical usage.  This
%  is not recommended, except by image processing experts, especially as part
%  of expert option filter function selection.
%
%  Two forms of the 'Sinc' function are available: Sinc and SincFast.  Sinc is
%  computed using the traditional sin(pi*x)/(pi*x); it is selected if the user
%  specifically specifies the use of a Sinc filter. SincFast uses highly
%  accurate (and fast) polynomial (low Q) and rational (high Q) approximations,
%  and will be used by default in most cases.
%
%  The Lanczos filter is a special 3-lobed Sinc-windowed Sinc filter (promoted
%  to Jinc-windowed Jinc for cylindrical (Elliptical Weighted Average) use).
%  The Sinc version is the most popular windowed filter.
%
%  LanczosSharp is a slightly sharpened (blur=0.9812505644269356 < 1) form of
%  the Lanczos filter, specifically designed for EWA distortion (as a
%  Jinc-Jinc); it can also be used as a slightly sharper orthogonal Lanczos
%  (Sinc-Sinc) filter. The chosen blur value comes as close as possible to
%  satisfying the following condition without changing the character of the
%  corresponding EWA filter:
%
%    'No-Op' Vertical and Horizontal Line Preservation Condition: Images with
%    only vertical or horizontal features are preserved when performing 'no-op'
%    with EWA distortion.
%
%  The Lanczos2 and Lanczos2Sharp filters are 2-lobe versions of the Lanczos
%  filters.  The 'sharp' version uses a blur factor of 0.9549963639785485,
%  again chosen because the resulting EWA filter comes as close as possible to
%  satisfying the above condition.
%
%  Robidoux is another filter tuned for EWA. It is the Keys cubic filter
%  defined by B=(228 - 108 sqrt(2))/199. Robidoux satisfies the "'No-Op'
%  Vertical and Horizontal Line Preservation Condition" exactly, and it
%  moderately blurs high frequency 'pixel-hash' patterns under no-op.  It turns
%  out to be close to both Mitchell and Lanczos2Sharp.  For example, its first
%  crossing is at (36 sqrt(2) + 123)/(72 sqrt(2) + 47), almost the same as the
%  first crossing of Mitchell and Lanczos2Sharp.
%
%  RobidouxSharp is a slightly sharper version of Robidoux, some believe it
%  is too sharp.  It is designed to minimize the maximum possible change in
%  a pixel value which is at one of the extremes (e.g., 0 or 255) under no-op
%  conditions.  Amazingly Mitchell falls roughly between Robidoux and
%  RobidouxSharp, though this seems to have been pure coincidence.
%
%  'EXPERT' OPTIONS:
%
%  These artifact "defines" are not recommended for production use without
%  expert knowledge of resampling, filtering, and the effects they have on the
%  resulting resampled (resized or distorted) image.
%
%  They can be used to override any and all filter default, and it is
%  recommended you make good use of "filter:verbose" to make sure that the
%  overall effect of your selection (before and after) is as expected.
%
%    "filter:verbose"  controls whether to output the exact results of the
%       filter selections made, as well as plotting data for graphing the
%       resulting filter over the filters support range.
%
%    "filter:filter"  select the main function associated with this filter
%       name, as the weighting function of the filter.  This can be used to
%       set a windowing function as a weighting function, for special
%       purposes, such as graphing.
%
%       If a "filter:window" operation has not been provided, a 'Box'
%       windowing function will be set to denote that no windowing function is
%       being used.
%
%    "filter:window"  Select this windowing function for the filter. While any
%       filter could be used as a windowing function, using the 'first lobe' of
%       that filter over the whole support window, using a non-windowing
%       function is not advisable. If no weighting filter function is specified
%       a 'SincFast' filter is used.
%
%    "filter:lobes"  Number of lobes to use for the Sinc/Jinc filter.  This a
%       simpler method of setting filter support size that will correctly
%       handle the Sinc/Jinc switch for an operators filtering requirements.
%       Only integers should be given.
%
%    "filter:support" Set the support size for filtering to the size given.
%       This not recommended for Sinc/Jinc windowed filters (lobes should be
%       used instead).  This will override any 'filter:lobes' option.
%
%    "filter:win-support" Scale windowing function to this size instead.  This
%       causes the windowing (or self-windowing Lagrange filter) to act is if
%       the support window it much much larger than what is actually supplied
%       to the calling operator.  The filter however is still clipped to the
%       real support size given, by the support range supplied to the caller.
%       If unset this will equal the normal filter support size.
%
%    "filter:blur" Scale the filter and support window by this amount.  A value
%       of > 1 will generally result in a more blurred image with more ringing
%       effects, while a value <1 will sharpen the resulting image with more
%       aliasing effects.
%
%    "filter:sigma" The sigma value to use for the Gaussian filter only.
%       Defaults to '1/2'.  Using a different sigma effectively provides a
%       method of using the filter as a 'blur' convolution.  Particularly when
%       using it for Distort.
%
%    "filter:b"
%    "filter:c" Override the preset B,C values for a Cubic filter.
%       If only one of these are given it is assumes to be a 'Keys' type of
%       filter such that B+2C=1, where Keys 'alpha' value = C.
%
%  Examples:
%
%  Set a true un-windowed Sinc filter with 10 lobes (very slow):
%     -define filter:filter=Sinc
%     -define filter:lobes=8
%
%  Set an 8 lobe Lanczos (Sinc or Jinc) filter:
%     -filter Lanczos
%     -define filter:lobes=8
%
%  The format of the AcquireResizeFilter method is:
%
%      ResizeFilter *AcquireResizeFilter(const Image *image,
%        const FilterType filter_type,const MagickBooleanType cylindrical,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o filter: the filter type, defining a preset filter, window and support.
%      The artifact settings listed above will override those selections.
%
%    o blur: blur the filter by this amount, use 1.0 if unknown.  Image
%      artifact "filter:blur" will override this API call usage, including any
%      internal change (such as for cylindrical usage).
%
%    o radial: use a 1D orthogonal filter (Sinc) or 2D cylindrical (radial)
%      filter (Jinc).
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate ResizeFilter *AcquireResizeFilter(const Image *image,
  const FilterType filter,const MagickBooleanType cylindrical,
  ExceptionInfo *exception)
{
  const char
    *artifact;

  double
    B,
    C,
    value;

  FilterType
    filter_type,
    window_type;

  ResizeFilter
    *resize_filter;

  /*
    Table Mapping given Filter, into Weighting and Windowing functions.
    A 'Box' windowing function means its a simple non-windowed filter.
    An 'SincFast' filter function could be upgraded to a 'Jinc' filter if a
    "cylindrical" is requested, unless a 'Sinc' or 'SincFast' filter was
    specifically requested by the user.

    WARNING: The order of this table must match the order of the FilterType
    enumeration specified in "resample.h", or the filter names will not match
    the filter being setup.

    You can check filter setups with the "filter:verbose" expert setting.
  */
  static struct
  {
    FilterType
      filter,
      window;
  } const mapping[SentinelFilter] =
  {
    { UndefinedFilter,     BoxFilter      },  /* Undefined (default to Box)   */
    { PointFilter,         BoxFilter      },  /* SPECIAL: Nearest neighbour   */
    { BoxFilter,           BoxFilter      },  /* Box averaging filter         */
    { TriangleFilter,      BoxFilter      },  /* Linear interpolation filter  */
    { HermiteFilter,       BoxFilter      },  /* Hermite interpolation filter */
    { SincFastFilter,      HannFilter     },  /* Hann -- cosine-sinc          */
    { SincFastFilter,      HammingFilter  },  /* Hamming --   '' variation    */
    { SincFastFilter,      BlackmanFilter },  /* Blackman -- 2*cosine-sinc    */
    { GaussianFilter,      BoxFilter      },  /* Gaussian blur filter         */
    { QuadraticFilter,     BoxFilter      },  /* Quadratic Gaussian approx    */
    { CubicFilter,         BoxFilter      },  /* General Cubic Filter, Spline */
    { CatromFilter,        BoxFilter      },  /* Cubic-Keys interpolator      */
    { MitchellFilter,      BoxFilter      },  /* 'Ideal' Cubic-Keys filter    */
    { JincFilter,          BoxFilter      },  /* Raw 3-lobed Jinc function    */
    { SincFilter,          BoxFilter      },  /* Raw 4-lobed Sinc function    */
    { SincFastFilter,      BoxFilter      },  /* Raw fast sinc ("Pade"-type)  */
    { SincFastFilter,      KaiserFilter   },  /* Kaiser -- square root-sinc   */
    { LanczosFilter,       WelchFilter    },  /* Welch -- parabolic (3 lobe)  */
    { SincFastFilter,      CubicFilter    },  /* Parzen -- cubic-sinc         */
    { SincFastFilter,      BohmanFilter   },  /* Bohman -- 2*cosine-sinc      */
    { SincFastFilter,      TriangleFilter },  /* Bartlett -- triangle-sinc    */
    { LagrangeFilter,      BoxFilter      },  /* Lagrange self-windowing      */
    { LanczosFilter,       LanczosFilter  },  /* Lanczos Sinc-Sinc filters    */
    { LanczosSharpFilter,  LanczosSharpFilter }, /* | these require */
    { Lanczos2Filter,      Lanczos2Filter },     /* | special handling */
    { Lanczos2SharpFilter, Lanczos2SharpFilter },
    { RobidouxFilter,      BoxFilter      },  /* Cubic Keys tuned for EWA     */
    { RobidouxSharpFilter, BoxFilter      },  /* Sharper Cubic Keys for EWA   */
    { LanczosFilter,       CosineFilter   },  /* Cosine window (3 lobes)      */
    { SplineFilter,        BoxFilter      },  /* Spline Cubic Filter          */
    { LanczosRadiusFilter, LanczosFilter  },  /* Lanczos with integer radius  */
    { CubicSplineFilter,   BoxFilter      },  /* CubicSpline (2/3/4 lobes)    */
    { MagicKernelSharp2013Filter, BoxFilter }, /* Magic Kernal Sharp 2013     */
    { MagicKernelSharp2021Filter, BoxFilter }, /* Magic Kernal Sharp 2021     */
  };
  /*
    Table mapping the filter/window from the above table to an actual function.
    The default support size for that filter as a weighting function, the range
    to scale with to use that function as a sinc windowing function, (typ 1.0).

    Note that the filter_type -> function is 1 to 1 except for Sinc(),
    SincFast(), and CubicBC() functions, which may have multiple filter to
    function associations.

    See "filter:verbose" handling below for the function -> filter mapping.
  */
  static struct
  {
    double
      (*function)(const double,const ResizeFilter*),
      support, /* Default lobes/support size of the weighting filter. */
      scale,   /* Support when function used as a windowing function
                 Typically equal to the location of the first zero crossing. */
      B,C;     /* BC-spline coefficients, ignored if not a CubicBC filter. */
    ResizeWeightingFunctionType weightingFunctionType;
  } const filters[SentinelFilter] =
  {
    /*            .---  support window (if used as a Weighting Function)
                  |    .--- first crossing (if used as a Windowing Function)
                  |    |    .--- B value for Cubic Function
                  |    |    |    .---- C value for Cubic Function
                  |    |    |    |                                    */
    { Box,       0.5, 0.5, 0.0, 0.0, BoxWeightingFunction },      /* Undefined (default to Box)  */
    { Box,       0.0, 0.5, 0.0, 0.0, BoxWeightingFunction },      /* Point (special handling)    */
    { Box,       0.5, 0.5, 0.0, 0.0, BoxWeightingFunction },      /* Box                         */
    { Triangle,  1.0, 1.0, 0.0, 0.0, TriangleWeightingFunction }, /* Triangle                    */
    { CubicBC,   1.0, 1.0, 0.0, 0.0, CubicBCWeightingFunction },  /* Hermite (cubic  B=C=0)      */
    { Hann,      1.0, 1.0, 0.0, 0.0, HannWeightingFunction },     /* Hann, cosine window         */
    { Hamming,   1.0, 1.0, 0.0, 0.0, HammingWeightingFunction },  /* Hamming, '' variation       */
    { Blackman,  1.0, 1.0, 0.0, 0.0, BlackmanWeightingFunction }, /* Blackman, 2*cosine window   */
    { Gaussian,  2.0, 1.5, 0.0, 0.0, GaussianWeightingFunction }, /* Gaussian                    */
    { Quadratic, 1.5, 1.5, 0.0, 0.0, QuadraticWeightingFunction },/* Quadratic gaussian          */
    { CubicBC,   2.0, 2.0, 1.0, 0.0, CubicBCWeightingFunction },  /* General Cubic Filter        */
    { CubicBC,   2.0, 1.0, 0.0, 0.5, CubicBCWeightingFunction },  /* Catmull-Rom    (B=0,C=1/2)  */
    { CubicBC,   2.0, 8.0/7.0, 1./3., 1./3., CubicBCWeightingFunction }, /* Mitchell   (B=C=1/3)    */
    { Jinc,      3.0, 1.2196698912665045, 0.0, 0.0, JincWeightingFunction }, /* Raw 3-lobed Jinc */
    { Sinc,      4.0, 1.0, 0.0, 0.0, SincWeightingFunction },     /* Raw 4-lobed Sinc            */
    { SincFast,  4.0, 1.0, 0.0, 0.0, SincFastWeightingFunction }, /* Raw fast sinc ("Pade"-type) */
    { Kaiser,    1.0, 1.0, 0.0, 0.0, KaiserWeightingFunction },   /* Kaiser (square root window) */
    { Welch,     1.0, 1.0, 0.0, 0.0, WelchWeightingFunction },    /* Welch (parabolic window)    */
    { CubicBC,   2.0, 2.0, 1.0, 0.0, CubicBCWeightingFunction },  /* Parzen (B-Spline window)    */
    { Bohman,    1.0, 1.0, 0.0, 0.0, BohmanWeightingFunction },   /* Bohman, 2*Cosine window     */
    { Triangle,  1.0, 1.0, 0.0, 0.0, TriangleWeightingFunction }, /* Bartlett (triangle window)  */
    { Lagrange,  2.0, 1.0, 0.0, 0.0, LagrangeWeightingFunction }, /* Lagrange sinc approximation */
    { SincFast,  3.0, 1.0, 0.0, 0.0, SincFastWeightingFunction }, /* Lanczos, 3-lobed Sinc-Sinc  */
    { SincFast,  3.0, 1.0, 0.0, 0.0, SincFastWeightingFunction }, /* Lanczos, Sharpened          */
    { SincFast,  2.0, 1.0, 0.0, 0.0, SincFastWeightingFunction }, /* Lanczos, 2-lobed            */
    { SincFast,  2.0, 1.0, 0.0, 0.0, SincFastWeightingFunction }, /* Lanczos2, sharpened         */
    /* Robidoux: Keys cubic close to Lanczos2D sharpened */
    { CubicBC,   2.0, 1.1685777620836932,
                            0.37821575509399867, 0.31089212245300067, CubicBCWeightingFunction },
    /* RobidouxSharp: Sharper version of Robidoux */
    { CubicBC,   2.0, 1.105822933719019,
                            0.2620145123990142,  0.3689927438004929, CubicBCWeightingFunction },
    { Cosine,    1.0, 1.0, 0.0, 0.0, CosineWeightingFunction },   /* Low level cosine window     */
    { CubicBC,   2.0, 2.0, 1.0, 0.0, CubicBCWeightingFunction },  /* Cubic B-Spline (B=1,C=0)    */
    { SincFast,  3.0, 1.0, 0.0, 0.0, SincFastWeightingFunction }, /* Lanczos, Integer Radius    */
    { CubicSpline,2.0, 0.5, 0.0, 0.0, BoxWeightingFunction },  /* Spline Lobes 2-lobed */
    { MagicKernelSharp2013, 2.5, 1.0, 0.0, 0.0, MagicKernelSharpWeightingFunction }, /* MagicKernelSharp2013 */
    { MagicKernelSharp2021, 4.5, 1.0, 0.0, 0.0, MagicKernelSharpWeightingFunction }, /* MagicKernelSharp2021 */
  };
  /*
    The known zero crossings of the Jinc() or more accurately the Jinc(x*PI)
    function being used as a filter. It is used by the "filter:lobes" expert
    setting and for 'lobes' for Jinc functions in the previous table. This way
    users do not have to deal with the highly irrational lobe sizes of the Jinc
    filter.

    Values taken from
    http://cose.math.bas.bg/webMathematica/webComputing/BesselZeros.jsp
    using Jv-function with v=1, then dividing by PI.
  */
  static double
    jinc_zeros[16] =
    {
      1.2196698912665045,
      2.2331305943815286,
      3.2383154841662362,
      4.2410628637960699,
      5.2427643768701817,
      6.2439216898644877,
      7.2447598687199570,
      8.2453949139520427,
      9.2458926849494673,
      10.246293348754916,
      11.246622794877883,
      12.246898461138105,
      13.247132522181061,
      14.247333735806849,
      15.247508563037300,
      16.247661874700962
   };

  /*
    Allocate resize filter.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(UndefinedFilter < filter && filter < SentinelFilter);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) exception;
  resize_filter=(ResizeFilter *) AcquireCriticalMemory(sizeof(*resize_filter));
  (void) memset(resize_filter,0,sizeof(*resize_filter));
  /*
    Defaults for the requested filter.
  */
  filter_type=mapping[filter].filter;
  window_type=mapping[filter].window;
  resize_filter->blur=1.0;
  /* Promote 1D Windowed Sinc Filters to a 2D Windowed Jinc filters */
  if ((cylindrical != MagickFalse) && (filter_type == SincFastFilter) &&
      (filter != SincFastFilter))
    filter_type=JincFilter;  /* 1D Windowed Sinc => 2D Windowed Jinc filters */

  /* Expert filter setting override */
  artifact=GetImageArtifact(image,"filter:filter");
  if (IsStringTrue(artifact) != MagickFalse)
    {
      ssize_t
        option;

      option=ParseCommandOption(MagickFilterOptions,MagickFalse,artifact);
      if ((UndefinedFilter < option) && (option < SentinelFilter))
        { /* Raw filter request - no window function. */
          filter_type=(FilterType) option;
          window_type=BoxFilter;
        }
      /* Filter override with a specific window function. */
      artifact=GetImageArtifact(image,"filter:window");
      if (artifact != (const char *) NULL)
        {
          option=ParseCommandOption(MagickFilterOptions,MagickFalse,artifact);
          if ((UndefinedFilter < option) && (option < SentinelFilter))
            window_type=(FilterType) option;
        }
    }
  else
    {
      /* Window specified, but no filter function?  Assume Sinc/Jinc. */
      artifact=GetImageArtifact(image,"filter:window");
      if (artifact != (const char *) NULL)
        {
          ssize_t
            option;

          option=ParseCommandOption(MagickFilterOptions,MagickFalse,artifact);
          if ((UndefinedFilter < option) && (option < SentinelFilter))
            {
              filter_type= cylindrical != MagickFalse ? JincFilter
                                                     : SincFastFilter;
              window_type=(FilterType) option;
            }
        }
    }

  /* Assign the real functions to use for the filters selected. */
  resize_filter->filter=filters[filter_type].function;
  resize_filter->support=filters[filter_type].support;
  resize_filter->filterWeightingType=filters[filter_type].weightingFunctionType;
  resize_filter->window=filters[window_type].function;
  resize_filter->windowWeightingType=filters[window_type].weightingFunctionType;
  resize_filter->scale=filters[window_type].scale;
  resize_filter->signature=MagickCoreSignature;

  /* Filter Modifications for orthogonal/cylindrical usage */
  if (cylindrical != MagickFalse)
    switch (filter_type)
    {
      case BoxFilter:
        /* Support for Cylindrical Box should be sqrt(2)/2 */
        resize_filter->support=(double) MagickSQ1_2;
        break;
      case LanczosFilter:
      case LanczosSharpFilter:
      case Lanczos2Filter:
      case Lanczos2SharpFilter:
      case LanczosRadiusFilter:
        resize_filter->filter=filters[JincFilter].function;
        resize_filter->window=filters[JincFilter].function;
        resize_filter->scale=filters[JincFilter].scale;
        /* number of lobes (support window size) remain unchanged */
        break;
      default:
        break;
    }
  /* Global Sharpening (regardless of orthogonal/cylindrical) */
  switch (filter_type)
  {
    case LanczosSharpFilter:
      resize_filter->blur *= 0.9812505644269356;
      break;
    case Lanczos2SharpFilter:
      resize_filter->blur *= 0.9549963639785485;
      break;
    /* case LanczosRadius:  blur adjust is done after lobes */
    default:
      break;
  }

  /*
    Expert Option Modifications.
  */

  /* User Gaussian Sigma Override - no support change */
  if ((resize_filter->filter == Gaussian) ||
      (resize_filter->window == Gaussian) ) {
    value=0.5;    /* gaussian sigma default, half pixel */
    artifact=GetImageArtifact(image,"filter:sigma");
    if (artifact != (const char *) NULL)
      value=StringToDouble(artifact,(char **) NULL);
    /* Define coefficients for Gaussian */
    resize_filter->coefficient[0]=value;                 /* note sigma too */
    resize_filter->coefficient[1]=MagickSafeReciprocal(2.0*value*value); /* sigma scaling */
    resize_filter->coefficient[2]=MagickSafeReciprocal(Magick2PI*value*value);
       /* normalization - not actually needed or used! */
    if ( value > 0.5 )
      resize_filter->support *= 2*value;  /* increase support linearly */
  }

  /* User Kaiser Alpha Override - no support change */
  if ((resize_filter->filter == Kaiser) ||
      (resize_filter->window == Kaiser) ) {
    value=6.5; /* default beta value for Kaiser bessel windowing function */
    artifact=GetImageArtifact(image,"filter:alpha");  /* FUTURE: depreciate */
    if (artifact != (const char *) NULL)
      value=StringToDouble(artifact,(char **) NULL);
    artifact=GetImageArtifact(image,"filter:kaiser-beta");
    if (artifact != (const char *) NULL)
      value=StringToDouble(artifact,(char **) NULL);
    artifact=GetImageArtifact(image,"filter:kaiser-alpha");
    if (artifact != (const char *) NULL)
      value=StringToDouble(artifact,(char **) NULL)*MagickPI;
    /* Define coefficients for Kaiser Windowing Function */
    resize_filter->coefficient[0]=value;         /* alpha */
    resize_filter->coefficient[1]=MagickSafeReciprocal(I0(value));
      /* normalization */
  }

  /* Support Overrides */
  artifact=GetImageArtifact(image,"filter:lobes");
  if (artifact != (const char *) NULL)
    {
      ssize_t
        lobes;

      lobes=(ssize_t) StringToLong(artifact);
      if (lobes < 1)
        lobes=1;
      resize_filter->support=(double) lobes;
    }
  if (resize_filter->filter == Jinc)
    {
      /*
        Convert a Jinc function lobes value to a real support value.
      */
      if (resize_filter->support > 16)
        resize_filter->support=jinc_zeros[15];  /* largest entry in table */
      else
        resize_filter->support=jinc_zeros[((long) resize_filter->support)-1];
      /*
        Blur this filter so support is a integer value (lobes dependant).
      */
      if (filter_type == LanczosRadiusFilter)
        resize_filter->blur*=floor(resize_filter->support)/
          resize_filter->support;
    }
  /*
    Expert blur override.
  */
  artifact=GetImageArtifact(image,"filter:blur");
  if (artifact != (const char *) NULL)
    resize_filter->blur*=StringToDouble(artifact,(char **) NULL);
  if (resize_filter->blur < MagickEpsilon)
    resize_filter->blur=(double) MagickEpsilon;
  /*
    Expert override of the support setting.
  */
  artifact=GetImageArtifact(image,"filter:support");
  if (artifact != (const char *) NULL)
    resize_filter->support=fabs(StringToDouble(artifact,(char **) NULL));
  /*
    Scale windowing function separately to the support 'clipping' window
    that calling operator is planning to actually use. (Expert override)
  */
  resize_filter->window_support=resize_filter->support; /* default */
  artifact=GetImageArtifact(image,"filter:win-support");
  if (artifact != (const char *) NULL)
    resize_filter->window_support=fabs(StringToDouble(artifact,(char **) NULL));
  /*
    Adjust window function scaling to match windowing support for weighting
    function.  This avoids a division on every filter call.
  */
  resize_filter->scale*=MagickSafeReciprocal(resize_filter->window_support);
  /*
    Set Cubic Spline B,C values, calculate Cubic coefficients.
  */
  B=0.0;
  C=0.0;
  if ((resize_filter->filter == CubicBC) ||
      (resize_filter->window == CubicBC) )
    {
      B=filters[filter_type].B;
      C=filters[filter_type].C;
      if (filters[window_type].function == CubicBC)
        {
          B=filters[window_type].B;
          C=filters[window_type].C;
        }
      artifact=GetImageArtifact(image,"filter:b");
      if (artifact != (const char *) NULL)
        {
          B=StringToDouble(artifact,(char **) NULL);
          C=(1.0-B)/2.0; /* Calculate C to get a Keys cubic filter. */
          artifact=GetImageArtifact(image,"filter:c"); /* user C override */
          if (artifact != (const char *) NULL)
            C=StringToDouble(artifact,(char **) NULL);
        }
      else
        {
          artifact=GetImageArtifact(image,"filter:c");
          if (artifact != (const char *) NULL)
            {
              C=StringToDouble(artifact,(char **) NULL);
              B=1.0-2.0*C; /* Calculate B to get a Keys cubic filter. */
            }
        }
      {
        const double
          twoB = B+B;

        /*
          Convert B,C values into Cubic Coefficients. See CubicBC().
        */
        resize_filter->coefficient[0]=1.0-(1.0/3.0)*B;
        resize_filter->coefficient[1]=-3.0+twoB+C;
        resize_filter->coefficient[2]=2.0-1.5*B-C;
        resize_filter->coefficient[3]=(4.0/3.0)*B+4.0*C;
        resize_filter->coefficient[4]=-8.0*C-twoB;
        resize_filter->coefficient[5]=B+5.0*C;
        resize_filter->coefficient[6]=(-1.0/6.0)*B-C;
      }
    }

  /*
    Expert Option Request for verbose details of the resulting filter.
  */
  if (IsStringTrue(GetImageArtifact(image,"filter:verbose")) != MagickFalse)
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp single
#endif
    {
      double
        support,
        x;

      /*
        Set the weighting function properly when the weighting function may not
        exactly match the filter of the same name.  EG: a Point filter is
        really uses a Box weighting function with a different support than is
        typically used.
      */
      if (resize_filter->filter == Box)       filter_type=BoxFilter;
      if (resize_filter->filter == Sinc)      filter_type=SincFilter;
      if (resize_filter->filter == SincFast)  filter_type=SincFastFilter;
      if (resize_filter->filter == Jinc)      filter_type=JincFilter;
      if (resize_filter->filter == CubicBC)   filter_type=CubicFilter;
      if (resize_filter->window == Box)       window_type=BoxFilter;
      if (resize_filter->window == Sinc)      window_type=SincFilter;
      if (resize_filter->window == SincFast)  window_type=SincFastFilter;
      if (resize_filter->window == Jinc)      window_type=JincFilter;
      if (resize_filter->window == CubicBC)   window_type=CubicFilter;
      /*
        Report Filter Details.
      */
      support=GetResizeFilterSupport(resize_filter);  /* practical support */
      (void) FormatLocaleFile(stdout,"# Resampling Filter (for graphing)\n#\n");
      (void) FormatLocaleFile(stdout,"# filter = %s\n",
        CommandOptionToMnemonic(MagickFilterOptions,filter_type));
      (void) FormatLocaleFile(stdout,"# window = %s\n",
        CommandOptionToMnemonic(MagickFilterOptions,window_type));
      (void) FormatLocaleFile(stdout,"# support = %.*g\n",
        GetMagickPrecision(),(double) resize_filter->support);
      (void) FormatLocaleFile(stdout,"# window-support = %.*g\n",
        GetMagickPrecision(),(double) resize_filter->window_support);
      (void) FormatLocaleFile(stdout,"# scale-blur = %.*g\n",
        GetMagickPrecision(),(double) resize_filter->blur);
      if ((filter_type == GaussianFilter) || (window_type == GaussianFilter))
        (void) FormatLocaleFile(stdout,"# gaussian-sigma = %.*g\n",
          GetMagickPrecision(),(double) resize_filter->coefficient[0]);
      if ((filter_type == KaiserFilter) || (window_type == KaiserFilter))
        (void) FormatLocaleFile(stdout,"# kaiser-beta = %.*g\n",
          GetMagickPrecision(),(double) resize_filter->coefficient[0]);
      (void) FormatLocaleFile(stdout,"# practical-support = %.*g\n",
        GetMagickPrecision(), (double) support);
      if ((filter_type == CubicFilter) || (window_type == CubicFilter))
        (void) FormatLocaleFile(stdout,"# B,C = %.*g,%.*g\n",
          GetMagickPrecision(),(double) B,GetMagickPrecision(),(double) C);
      (void) FormatLocaleFile(stdout,"\n");
      /*
        Output values of resulting filter graph -- for graphing filter result.
      */
      for (x=0.0; x <= support; x+=0.01)
        (void) FormatLocaleFile(stdout,"%5.2lf\t%.*g\n",x,GetMagickPrecision(),
          (double) GetResizeFilterWeight(resize_filter,x));
      /*
        A final value so gnuplot can graph the 'stop' properly.
      */
      (void) FormatLocaleFile(stdout,"%5.2lf\t%.*g\n",support,
        GetMagickPrecision(),0.0);
      /* Output the above once only for each image - remove setting */
      (void) DeleteImageArtifact((Image *) image,"filter:verbose");
    }
  return(resize_filter);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A d a p t i v e R e s i z e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AdaptiveResizeImage() adaptively resize image with pixel resampling.
%
%  This is shortcut function for a fast interpolative resize using mesh
%  interpolation.  It works well for small resizes of less than +/- 50%
%  of the original image size.  For larger resizing on images a full
%  filtered and slower resize function should be used instead.
%
%  The format of the AdaptiveResizeImage method is:
%
%      Image *AdaptiveResizeImage(const Image *image,const size_t columns,
%        const size_t rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the resized image.
%
%    o rows: the number of rows in the resized image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AdaptiveResizeImage(const Image *image,
  const size_t columns,const size_t rows,ExceptionInfo *exception)
{
  Image
    *resize_image;

  resize_image=InterpolativeResizeImage(image,columns,rows,MeshInterpolatePixel,
    exception);
  return(resize_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   B e s s e l O r d e r O n e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BesselOrderOne() computes the Bessel function of x of the first kind of
%  order 0.  This is used to create the Jinc() filter function below.
%
%    Reduce x to |x| since j1(x)= -j1(-x), and for x in (0,8]
%
%       j1(x) = x*j1(x);
%
%    For x in (8,inf)
%
%       j1(x) = sqrt(2/(pi*x))*(p1(x)*cos(x1)-q1(x)*sin(x1))
%
%    where x1 = x-3*pi/4. Compute sin(x1) and cos(x1) as follow:
%
%       cos(x1) =  cos(x)cos(3pi/4)+sin(x)sin(3pi/4)
%               =  1/sqrt(2) * (sin(x) - cos(x))
%       sin(x1) =  sin(x)cos(3pi/4)-cos(x)sin(3pi/4)
%               = -1/sqrt(2) * (sin(x) + cos(x))
%
%  The format of the BesselOrderOne method is:
%
%      double BesselOrderOne(double x)
%
%  A description of each parameter follows:
%
%    o x: double value.
%
*/

#undef I0
static double I0(double x)
{
  double
    sum,
    t,
    y;

  ssize_t
    i;

  /*
    Zeroth order Bessel function of the first kind.
  */
  sum=1.0;
  y=x*x/4.0;
  t=y;
  for (i=2; t > MagickEpsilon; i++)
  {
    sum+=t;
    t*=y/((double) i*i);
  }
  return(sum);
}

#undef J1
static double J1(double x)
{
  double
    p,
    q;

  ssize_t
    i;

  static const double
    Pone[] =
    {
       0.581199354001606143928050809e+21,
      -0.6672106568924916298020941484e+20,
       0.2316433580634002297931815435e+19,
      -0.3588817569910106050743641413e+17,
       0.2908795263834775409737601689e+15,
      -0.1322983480332126453125473247e+13,
       0.3413234182301700539091292655e+10,
      -0.4695753530642995859767162166e+7,
       0.270112271089232341485679099e+4
    },
    Qone[] =
    {
      0.11623987080032122878585294e+22,
      0.1185770712190320999837113348e+20,
      0.6092061398917521746105196863e+17,
      0.2081661221307607351240184229e+15,
      0.5243710262167649715406728642e+12,
      0.1013863514358673989967045588e+10,
      0.1501793594998585505921097578e+7,
      0.1606931573481487801970916749e+4,
      0.1e+1
    };

  p=Pone[8];
  q=Qone[8];
  for (i=7; i >= 0; i--)
  {
    p=p*x*x+Pone[i];
    q=q*x*x+Qone[i];
  }
  return(p/q);
}

#undef P1
static double P1(double x)
{
  double
    p,
    q;

  ssize_t
    i;

  static const double
    Pone[] =
    {
      0.352246649133679798341724373e+5,
      0.62758845247161281269005675e+5,
      0.313539631109159574238669888e+5,
      0.49854832060594338434500455e+4,
      0.2111529182853962382105718e+3,
      0.12571716929145341558495e+1
    },
    Qone[] =
    {
      0.352246649133679798068390431e+5,
      0.626943469593560511888833731e+5,
      0.312404063819041039923015703e+5,
      0.4930396490181088979386097e+4,
      0.2030775189134759322293574e+3,
      0.1e+1
    };

  p=Pone[5];
  q=Qone[5];
  for (i=4; i >= 0; i--)
  {
    p=p*(8.0/x)*(8.0/x)+Pone[i];
    q=q*(8.0/x)*(8.0/x)+Qone[i];
  }
  return(p/q);
}

#undef Q1
static double Q1(double x)
{
  double
    p,
    q;

  ssize_t
    i;

  static const double
    Pone[] =
    {
      0.3511751914303552822533318e+3,
      0.7210391804904475039280863e+3,
      0.4259873011654442389886993e+3,
      0.831898957673850827325226e+2,
      0.45681716295512267064405e+1,
      0.3532840052740123642735e-1
    },
    Qone[] =
    {
      0.74917374171809127714519505e+4,
      0.154141773392650970499848051e+5,
      0.91522317015169922705904727e+4,
      0.18111867005523513506724158e+4,
      0.1038187585462133728776636e+3,
      0.1e+1
    };

  p=Pone[5];
  q=Qone[5];
  for (i=4; i >= 0; i--)
  {
    p=p*(8.0/x)*(8.0/x)+Pone[i];
    q=q*(8.0/x)*(8.0/x)+Qone[i];
  }
  return(p/q);
}

static double BesselOrderOne(double x)
{
  double
    p,
    q;

  if (x == 0.0)
    return(0.0);
  p=x;
  if (x < 0.0)
    x=(-x);
  if (x < 8.0)
    return(p*J1(x));
  q=sqrt((double) (2.0/(MagickPI*x)))*(P1(x)*(1.0/sqrt(2.0)*(sin(x)-
    cos(x)))-8.0/x*Q1(x)*(-1.0/sqrt(2.0)*(sin(x)+cos(x))));
  if (p < 0.0)
    q=(-q);
  return(q);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y R e s i z e F i l t e r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyResizeFilter() destroy the resize filter.
%
%  The format of the DestroyResizeFilter method is:
%
%      ResizeFilter *DestroyResizeFilter(ResizeFilter *resize_filter)
%
%  A description of each parameter follows:
%
%    o resize_filter: the resize filter.
%
*/
MagickPrivate ResizeFilter *DestroyResizeFilter(ResizeFilter *resize_filter)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickCoreSignature);
  resize_filter->signature=(~MagickCoreSignature);
  resize_filter=(ResizeFilter *) RelinquishMagickMemory(resize_filter);
  return(resize_filter);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t R e s i z e F i l t e r S u p p o r t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetResizeFilterSupport() return the current support window size for this
%  filter.  Note that this may have been enlarged by filter:blur factor.
%
%  The format of the GetResizeFilterSupport method is:
%
%      double GetResizeFilterSupport(const ResizeFilter *resize_filter)
%
%  A description of each parameter follows:
%
%    o filter: Image filter to use.
%
*/

MagickPrivate double *GetResizeFilterCoefficient(
  const ResizeFilter *resize_filter)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickCoreSignature);
  return((double *) resize_filter->coefficient);
}

MagickPrivate double GetResizeFilterBlur(const ResizeFilter *resize_filter)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickCoreSignature);
  return(resize_filter->blur);
}

MagickPrivate double GetResizeFilterScale(const ResizeFilter *resize_filter)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickCoreSignature);
  return(resize_filter->scale);
}

MagickPrivate double GetResizeFilterWindowSupport(
  const ResizeFilter *resize_filter)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickCoreSignature);
  return(resize_filter->window_support);
}

MagickPrivate ResizeWeightingFunctionType GetResizeFilterWeightingType(
  const ResizeFilter *resize_filter)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickCoreSignature);
  return(resize_filter->filterWeightingType);
}

MagickPrivate ResizeWeightingFunctionType GetResizeFilterWindowWeightingType(
  const ResizeFilter *resize_filter)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickCoreSignature);
  return(resize_filter->windowWeightingType);
}

MagickPrivate double GetResizeFilterSupport(const ResizeFilter *resize_filter)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickCoreSignature);
  return(resize_filter->support*resize_filter->blur);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t R e s i z e F i l t e r W e i g h t                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetResizeFilterWeight evaluates the specified resize filter at the point x
%  which usually lies between zero and the filters current 'support' and
%  returns the weight of the filter function at that point.
%
%  The format of the GetResizeFilterWeight method is:
%
%      double GetResizeFilterWeight(const ResizeFilter *resize_filter,
%        const double x)
%
%  A description of each parameter follows:
%
%    o filter: the filter type.
%
%    o x: the point.
%
*/
MagickPrivate double GetResizeFilterWeight(const ResizeFilter *resize_filter,
  const double x)
{
  double
    scale,
    weight,
    x_blur;

  /*
    Windowing function - scale the weighting filter by this amount.
  */
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickCoreSignature);
  x_blur=fabs((double) x)*MagickSafeReciprocal(resize_filter->blur);  /* X offset with blur scaling */
  if ((resize_filter->window_support < MagickEpsilon) ||
      (resize_filter->window == Box))
    scale=1.0;  /* Point or Box Filter -- avoid division by zero */
  else
    {
      scale=resize_filter->scale;
      scale=resize_filter->window(x_blur*scale,resize_filter);
    }
  weight=scale*resize_filter->filter(x_blur,resize_filter);
  return(weight);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n t e r p o l a t i v e R e s i z e I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpolativeResizeImage() resizes an image using the specified
%  interpolation method.
%
%  The format of the InterpolativeResizeImage method is:
%
%      Image *InterpolativeResizeImage(const Image *image,const size_t columns,
%        const size_t rows,const PixelInterpolateMethod method,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the resized image.
%
%    o rows: the number of rows in the resized image.
%
%    o method: the pixel interpolation method.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *InterpolativeResizeImage(const Image *image,
  const size_t columns,const size_t rows,const PixelInterpolateMethod method,
  ExceptionInfo *exception)
{
#define InterpolativeResizeImageTag  "Resize/Image"

  CacheView
    *image_view,
    *resize_view;

  Image
    *resize_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PointInfo
    scale;

  ssize_t
    y;

  /*
    Interpolatively resize image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((columns == 0) || (rows == 0))
    ThrowImageException(ImageError,"NegativeOrZeroImageSize");
  if ((columns == image->columns) && (rows == image->rows))
    return(CloneImage(image,0,0,MagickTrue,exception));
  resize_image=CloneImage(image,columns,rows,MagickTrue,exception);
  if (resize_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(resize_image,DirectClass,exception) == MagickFalse)
    {
      resize_image=DestroyImage(resize_image);
      return((Image *) NULL);
    }
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  resize_view=AcquireAuthenticCacheView(resize_image,exception);
  scale.x=(double) image->columns/resize_image->columns;
  scale.y=(double) image->rows/resize_image->rows;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,resize_image,resize_image->rows,1)
#endif
  for (y=0; y < (ssize_t) resize_image->rows; y++)
  {
    PointInfo
      offset;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(resize_view,0,y,resize_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      continue;
    offset.y=((double) y+0.5)*scale.y-0.5;
    for (x=0; x < (ssize_t) resize_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          resize_traits,
          traits;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        resize_traits=GetPixelChannelTraits(resize_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (resize_traits == UndefinedPixelTrait))
          continue;
        offset.x=((double) x+0.5)*scale.x-0.5;
        status=InterpolatePixelChannels(image,image_view,resize_image,method,
          offset.x,offset.y,q,exception);
        if (status == MagickFalse)
          break;
      }
      q+=(ptrdiff_t) GetPixelChannels(resize_image);
    }
    if (SyncCacheViewAuthenticPixels(resize_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,InterpolativeResizeImageTag,progress,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  resize_view=DestroyCacheView(resize_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    resize_image=DestroyImage(resize_image);
  return(resize_image);
}
#if defined(MAGICKCORE_LQR_DELEGATE)

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i q u i d R e s c a l e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LiquidRescaleImage() rescales image with seam carving.
%
%  The format of the LiquidRescaleImage method is:
%
%      Image *LiquidRescaleImage(const Image *image,const size_t columns,
%        const size_t rows,const double delta_x,const double rigidity,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the rescaled image.
%
%    o rows: the number of rows in the rescaled image.
%
%    o delta_x: maximum seam transversal step (0 means straight seams).
%
%    o rigidity: introduce a bias for non-straight seams (typically 0).
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *LiquidRescaleImage(const Image *image,const size_t columns,
  const size_t rows,const double delta_x,const double rigidity,
  ExceptionInfo *exception)
{
#define LiquidRescaleImageTag  "Rescale/Image"

  CacheView
    *image_view,
    *rescale_view;

  gfloat
    *packet,
    *pixels;

  Image
    *rescale_image;

  int
    x_offset,
    y_offset;

  LqrCarver
    *carver;

  LqrRetVal
    lqr_status;

  MagickBooleanType
    status;

  MemoryInfo
    *pixel_info;

  gfloat
    *q;

  ssize_t
    y;

  /*
    Liquid rescale image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((columns == 0) || (rows == 0))
    ThrowImageException(ImageError,"NegativeOrZeroImageSize");
  if ((columns == image->columns) && (rows == image->rows))
    return(CloneImage(image,0,0,MagickTrue,exception));
  if ((columns <= 2) || (rows <= 2))
    return(ResizeImage(image,columns,rows,image->filter,exception));
  pixel_info=AcquireVirtualMemory(image->columns,image->rows*MaxPixelChannels*
    sizeof(*pixels));
  if (pixel_info == (MemoryInfo *) NULL)
    return((Image *) NULL);
  pixels=(gfloat *) GetVirtualMemoryBlob(pixel_info);
  status=MagickTrue;
  q=pixels;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
        *q++=(gfloat) (QuantumScale*(double) p[i]);
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  image_view=DestroyCacheView(image_view);
  carver=lqr_carver_new_ext(pixels,(int) image->columns,(int) image->rows,
    (int) GetPixelChannels(image),LQR_COLDEPTH_32F);
  if (carver == (LqrCarver *) NULL)
    {
      pixel_info=RelinquishVirtualMemory(pixel_info);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  lqr_carver_set_preserve_input_image(carver);
  lqr_status=lqr_carver_init(carver,(gint) delta_x,(gfloat) rigidity);
  lqr_status=lqr_carver_resize(carver,(gint) columns,(gint) rows);
  (void) lqr_status;
  rescale_image=CloneImage(image,(size_t) lqr_carver_get_width(carver),
    (size_t) lqr_carver_get_height(carver),MagickTrue,exception);
  if (rescale_image == (Image *) NULL)
    {
      pixel_info=RelinquishVirtualMemory(pixel_info);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(rescale_image,DirectClass,exception) == MagickFalse)
    {
      pixel_info=RelinquishVirtualMemory(pixel_info);
      rescale_image=DestroyImage(rescale_image);
      return((Image *) NULL);
    }
  rescale_view=AcquireAuthenticCacheView(rescale_image,exception);
  (void) lqr_carver_scan_reset(carver);
  while (lqr_carver_scan_ext(carver,&x_offset,&y_offset,(void **) &packet) != 0)
  {
    Quantum
      *magick_restrict p;

    ssize_t
      i;

    p=QueueCacheViewAuthenticPixels(rescale_view,x_offset,y_offset,1,1,
      exception);
    if (p == (Quantum *) NULL)
      break;
    for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
    {
      PixelChannel
        channel;

      PixelTrait
        rescale_traits,
        traits;

      channel=GetPixelChannelChannel(image,i);
      traits=GetPixelChannelTraits(image,channel);
      rescale_traits=GetPixelChannelTraits(rescale_image,channel);
      if ((traits == UndefinedPixelTrait) ||
          (rescale_traits == UndefinedPixelTrait))
        continue;
      SetPixelChannel(rescale_image,channel,ClampToQuantum(QuantumRange*
        packet[i]),p);
    }
    if (SyncCacheViewAuthenticPixels(rescale_view,exception) == MagickFalse)
      break;
  }
  rescale_view=DestroyCacheView(rescale_view);
  pixel_info=RelinquishVirtualMemory(pixel_info);
  lqr_carver_destroy(carver);
  return(rescale_image);
}
#else
MagickExport Image *LiquidRescaleImage(const Image *image,
  const size_t magick_unused(columns),const size_t magick_unused(rows),
  const double magick_unused(delta_x),const double magick_unused(rigidity),
  ExceptionInfo *exception)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  magick_unreferenced(columns);
  magick_unreferenced(rows);
  magick_unreferenced(delta_x);
  magick_unreferenced(rigidity);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","'%s' (LQR)",image->filename);
  return((Image *) NULL);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g n i f y I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagnifyImage() doubles the size of the image with a pixel art scaling
%  algorithm.
%
%  The format of the MagnifyImage method is:
%
%      Image *MagnifyImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline void CopyPixels(const Quantum *source,const ssize_t source_offset,
  Quantum *destination,const ssize_t destination_offset,const size_t channels)
{
  ssize_t
    i;

  for (i=0; i < (ssize_t) channels; i++)
    destination[(ssize_t) channels*destination_offset+i]=
      source[source_offset*(ssize_t) channels+i];
}

static inline void MixPixels(const Quantum *source,const ssize_t *source_offset,
  const size_t source_size,Quantum *destination,
  const ssize_t destination_offset,const size_t channels)
{
  ssize_t
    i;

  for (i=0; i < (ssize_t) channels; i++)
  {
    ssize_t
      j,
      sum = 0;

    for (j=0; j < (ssize_t) source_size; j++)
      sum+=(ssize_t) source[source_offset[j]*(ssize_t) channels+i];
    destination[(ssize_t) channels*destination_offset+i]=(Quantum) (sum/
      (ssize_t) source_size);
  }
}

static inline void Mix2Pixels(const Quantum *source,
  const ssize_t source_offset1,const ssize_t source_offset2,
  Quantum *destination,const ssize_t destination_offset,const size_t channels)
{
  const ssize_t
    offsets[2] = { source_offset1, source_offset2 };

  MixPixels(source,offsets,2,destination,destination_offset,channels);
}

static inline int PixelsEqual(const Quantum *source1,ssize_t offset1,
  const Quantum *source2,ssize_t offset2,const size_t channels)
{
  ssize_t
    i;

  offset1*=(ssize_t) channels;
  offset2*=(ssize_t) channels;
  for (i=0; i < (ssize_t) channels; i++)
    if (source1[offset1+i] != source2[offset2+i])
      return(0);
  return(1);
}

static inline void Eagle2X(const Image *source,const Quantum *pixels,
  Quantum *result,const size_t channels)
{
  ssize_t
    i;

  (void) source;
  for (i=0; i < 4; i++)
    CopyPixels(pixels,4,result,i,channels);
  if (PixelsEqual(pixels,0,pixels,1,channels) &&
      PixelsEqual(pixels,1,pixels,3,channels))
    CopyPixels(pixels,0,result,0,channels);
  if (PixelsEqual(pixels,1,pixels,2,channels) &&
      PixelsEqual(pixels,2,pixels,5,channels))
    CopyPixels(pixels,2,result,1,channels);
  if (PixelsEqual(pixels,3,pixels,6,channels) &&
      PixelsEqual(pixels,6,pixels,7,channels))
    CopyPixels(pixels,6,result,2,channels);
  if (PixelsEqual(pixels,5,pixels,8,channels) &&
      PixelsEqual(pixels,8,pixels,7,channels))
    CopyPixels(pixels,8,result,3,channels);
}

static void Hq2XHelper(const unsigned int rule,const Quantum *source,
  Quantum *destination,const ssize_t destination_offset,const size_t channels,
  const ssize_t e,const ssize_t a,const ssize_t b,const ssize_t d,
  const ssize_t f,const ssize_t h)
{
#define caseA(N,A,B,C,D) \
  case N: \
  { \
    const ssize_t \
      offsets[4] = { A, B, C, D }; \
 \
    MixPixels(source,offsets,4,destination,destination_offset,channels);\
    break; \
  }
#define caseB(N,A,B,C,D,E,F,G,H) \
  case N: \
  { \
    const ssize_t \
      offsets[8] = { A, B, C, D, E, F, G, H }; \
 \
    MixPixels(source,offsets,8,destination,destination_offset,channels);\
    break; \
  }

  switch (rule)
  {
    case 0:
    {
      CopyPixels(source,e,destination,destination_offset,channels);
      break;
    }
    caseA(1,e,e,e,a)
    caseA(2,e,e,e,d)
    caseA(3,e,e,e,b)
    caseA(4,e,e,d,b)
    caseA(5,e,e,a,b)
    caseA(6,e,e,a,d)
    caseB(7,e,e,e,e,e,b,b,d)
    caseB(8,e,e,e,e,e,d,d,b)
    caseB(9,e,e,e,e,e,e,d,b)
    caseB(10,e,e,d,d,d,b,b,b)
    case 11:
    {
      const ssize_t
        offsets[16] = { e, e, e, e, e, e, e, e, e, e, e, e, e, e, d, b };

      MixPixels(source,offsets,16,destination,destination_offset,channels);
      break;
    }
    case 12:
    {
      if (PixelsEqual(source,b,source,d,channels))
        {
          const ssize_t
            offsets[4] = { e, e, d, b };

          MixPixels(source,offsets,4,destination,destination_offset,channels);
        }
      else
        CopyPixels(source,e,destination,destination_offset,channels);
      break;
    }
    case 13:
    {
      if (PixelsEqual(source,b,source,d,channels))
        {
          const ssize_t
            offsets[8] = { e, e, d, d, d, b, b, b };

          MixPixels(source,offsets,8,destination,destination_offset,channels);
        }
      else
        CopyPixels(source,e,destination,destination_offset,channels);
      break;
    }
    case 14:
    {
      if (PixelsEqual(source,b,source,d,channels))
        {
          const ssize_t
            offsets[16] = { e, e, e, e, e, e, e, e, e, e, e, e, e, e, d, b };

          MixPixels(source,offsets,16,destination,destination_offset,channels);
        }
      else
        CopyPixels(source,e,destination,destination_offset,channels);
      break;
    }
    case 15:
    {
      if (PixelsEqual(source,b,source,d,channels))
        {
          const ssize_t
            offsets[4] = { e, e, d, b };

          MixPixels(source,offsets,4,destination,destination_offset,channels);
        }
      else
        {
          const ssize_t
            offsets[4] = { e, e, e, a };

          MixPixels(source,offsets,4,destination,destination_offset,channels);
        }
      break;
    }
    case 16:
    {
      if (PixelsEqual(source,b,source,d,channels))
        {
          const ssize_t
            offsets[8] = { e, e, e, e, e, e, d, b };

          MixPixels(source,offsets,8,destination,destination_offset,channels);
        }
      else
        {
          const ssize_t
            offsets[4] = { e, e, e, a };

          MixPixels(source,offsets,4,destination,destination_offset,channels);
        }
      break;
    }
    case 17:
    {
      if (PixelsEqual(source,b,source,d,channels))
        {
          const ssize_t
            offsets[8] = { e, e, d, d, d, b, b, b };

          MixPixels(source,offsets,8,destination,destination_offset,channels);
        }
      else
        {
          const ssize_t
            offsets[4] = { e, e, e, a };

          MixPixels(source,offsets,4,destination,destination_offset,channels);
        }
      break;
    }
    case 18:
    {
      if (PixelsEqual(source,b,source,f,channels))
        {
          const ssize_t
            offsets[8] = { e, e, e, e, e, b, b, d };

          MixPixels(source,offsets,8,destination,destination_offset,channels);
        }
      else
        {
          const ssize_t
            offsets[4] = { e, e, e, d };

          MixPixels(source,offsets,4,destination,destination_offset,channels);
        }
      break;
    }
    default:
    {
      if (PixelsEqual(source,d,source,h,channels))
        {
          const ssize_t
            offsets[8] = { e, e, e, e, e, d, d, b };

          MixPixels(source,offsets,8,destination,destination_offset,channels);
        }
      else
        {
          const ssize_t
            offsets[4] = { e, e, e, b };

          MixPixels(source,offsets,4,destination,destination_offset,channels);
        }
      break;
    }
  }
  #undef caseA
  #undef caseB
}

static inline unsigned int Hq2XPatternToNumber(const int *pattern)
{
  ssize_t
    i;

  unsigned int
    result,
    order;

  result=0;
  order=1;
  for (i=7; i >= 0; i--)
  {
    result+=order*(unsigned int) pattern[i];
    order*=2;
  }
  return(result);
}

static inline void Hq2X(const Image *source,const Quantum *pixels,
  Quantum *result,const size_t channels)
{
  static const unsigned int
    Hq2XTable[] =
    {
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 15, 12, 5,  3, 17, 13,
      4, 4, 6, 18, 4, 4, 6, 18, 5,  3, 12, 12, 5,  3,  1, 12,
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 17, 13, 5,  3, 16, 14,
      4, 4, 6, 18, 4, 4, 6, 18, 5,  3, 16, 12, 5,  3,  1, 14,
      4, 4, 6,  2, 4, 4, 6,  2, 5, 19, 12, 12, 5, 19, 16, 12,
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 12, 5,  3, 16, 12,
      4, 4, 6,  2, 4, 4, 6,  2, 5, 19,  1, 12, 5, 19,  1, 14,
      4, 4, 6,  2, 4, 4, 6, 18, 5,  3, 16, 12, 5, 19,  1, 14,
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 15, 12, 5,  3, 17, 13,
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 12, 5,  3, 16, 12,
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 17, 13, 5,  3, 16, 14,
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 13, 5,  3,  1, 14,
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 12, 5,  3, 16, 13,
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 12, 5,  3,  1, 12,
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3, 16, 12, 5,  3,  1, 14,
      4, 4, 6,  2, 4, 4, 6,  2, 5,  3,  1, 12, 5,  3,  1, 14
    };

  const int
    pattern1[] =
    {
      !PixelsEqual(pixels,4,pixels,8,channels),
      !PixelsEqual(pixels,4,pixels,7,channels),
      !PixelsEqual(pixels,4,pixels,6,channels),
      !PixelsEqual(pixels,4,pixels,5,channels),
      !PixelsEqual(pixels,4,pixels,3,channels),
      !PixelsEqual(pixels,4,pixels,2,channels),
      !PixelsEqual(pixels,4,pixels,1,channels),
      !PixelsEqual(pixels,4,pixels,0,channels)
    };

#define Rotated(p) p[2], p[4], p[7], p[1], p[6], p[0], p[3], p[5]
  const int pattern2[] = { Rotated(pattern1) };
  const int pattern3[] = { Rotated(pattern2) };
  const int pattern4[] = { Rotated(pattern3) };
#undef Rotated
  (void) source;
  Hq2XHelper(Hq2XTable[Hq2XPatternToNumber(pattern1)],pixels,result,0,
    channels,4,0,1,3,5,7);
  Hq2XHelper(Hq2XTable[Hq2XPatternToNumber(pattern2)],pixels,result,1,
    channels,4,2,5,1,7,3);
  Hq2XHelper(Hq2XTable[Hq2XPatternToNumber(pattern3)],pixels,result,3,
    channels,4,8,7,5,3,1);
  Hq2XHelper(Hq2XTable[Hq2XPatternToNumber(pattern4)],pixels,result,2,
    channels,4,6,3,7,1,5);
}

static void Fish2X(const Image *source,const Quantum *pixels,Quantum *result,
  const size_t channels)
{
#define Corner(A,B,C,D) \
  { \
    if (intensities[B] > intensities[A]) \
      { \
        const ssize_t    \
          offsets[3] = { B, C, D }; \
 \
        MixPixels(pixels,offsets,3,result,3,channels); \
      } \
    else \
      { \
        const ssize_t    \
          offsets[3] = { A, B, C }; \
 \
        MixPixels(pixels,offsets,3,result,3,channels); \
      } \
  }

#define Line(A,B,C,D) \
  { \
    if (intensities[C] > intensities[A]) \
      Mix2Pixels(pixels,C,D,result,3,channels); \
    else \
      Mix2Pixels(pixels,A,B,result,3,channels); \
  }

  const ssize_t
    pixels_offsets[4] = { 0, 1, 3, 4 };

  int
    ab,
    ad,
    ae,
    bd,
    be,
    de;

  MagickFloatType
    intensities[9];

  ssize_t
    i;

  for (i=0; i < 9; i++)
    intensities[i]=(MagickFloatType) GetPixelIntensity(source,pixels+
      i*(ssize_t) channels);
  CopyPixels(pixels,0,result,0,channels);
  CopyPixels(pixels,(ssize_t) (intensities[0] > intensities[1] ? 0 : 1),result,
    1,channels);
  CopyPixels(pixels,(ssize_t) (intensities[0] > intensities[3] ? 0 : 3),result,
    2,channels);
  ae=PixelsEqual(pixels,0,pixels,4,channels);
  bd=PixelsEqual(pixels,1,pixels,3,channels);
  ab=PixelsEqual(pixels,0,pixels,1,channels);
  de=PixelsEqual(pixels,3,pixels,4,channels);
  ad=PixelsEqual(pixels,0,pixels,3,channels);
  be=PixelsEqual(pixels,1,pixels,4,channels);
  if (ae && bd && ab)
    {
      CopyPixels(pixels,0,result,3,channels);
      return;
    }
  if (ad && de && !ab)
    {
      Corner(1,0,4,3)
      return;
    }
  if (be && de && !ab)
    {
      Corner(0,1,3,4)
      return;
    }
  if (ad && ab && !be)
    {
      Corner(4,3,1,0)
      return;
    }
  if (ab && be && !ad)
    {
      Corner(3,0,4,1)
      return;
    }
  if (ae && (!bd || intensities[1] > intensities[0]))
    {
      Mix2Pixels(pixels,0,4,result,3,channels);
      return;
    }
  if (bd && (!ae || intensities[0] > intensities[1]))
    {
      Mix2Pixels(pixels,1,3,result,3,channels);
      return;
    }
  if (ab)
    {
      Line(0,1,3,4)
      return;
    }
  if (de)
    {
      Line(3,4,0,1)
      return;
    }
  if (ad)
    {
      Line(0,3,1,4)
      return;
    }
  if (be)
    {
      Line(1,4,0,3)
      return;
    }
  MixPixels(pixels,pixels_offsets,4,result,3,channels);
#undef Corner
#undef Line
}

static void Xbr2X(const Image *magick_unused(source),const Quantum *pixels,
  Quantum *result,const size_t channels)
{
#define WeightVar(M,N) const int w_##M##_##N = \
  PixelsEqual(pixels,M,pixels,N,channels) ? 0 : 1;

  WeightVar(12,11)
  WeightVar(12,7)
  WeightVar(12,13)
  WeightVar(12,17)
  WeightVar(12,16)
  WeightVar(12,8)
  WeightVar(6,10)
  WeightVar(6,2)
  WeightVar(11,7)
  WeightVar(11,17)
  WeightVar(11,5)
  WeightVar(7,13)
  WeightVar(7,1)
  WeightVar(12,6)
  WeightVar(12,18)
  WeightVar(8,14)
  WeightVar(8,2)
  WeightVar(13,17)
  WeightVar(13,9)
  WeightVar(7,3)
  WeightVar(16,10)
  WeightVar(16,22)
  WeightVar(17,21)
  WeightVar(11,15)
  WeightVar(18,14)
  WeightVar(18,22)
  WeightVar(17,23)
  WeightVar(17,19)
#undef WeightVar

  magick_unreferenced(source);

  if (
    w_12_16 + w_12_8 + w_6_10 + w_6_2 + (4 * w_11_7) <
    w_11_17 + w_11_5 + w_7_13 + w_7_1 + (4 * w_12_6)
  )
    Mix2Pixels(pixels,(ssize_t) (w_12_11 <= w_12_7 ? 11 : 7),12,result,0,
      channels);
  else
    CopyPixels(pixels,12,result,0,channels);
  if (
    w_12_18 + w_12_6 + w_8_14 + w_8_2 + (4 * w_7_13) <
    w_13_17 + w_13_9 + w_11_7 + w_7_3 + (4 * w_12_8)
  )
    Mix2Pixels(pixels,(ssize_t) (w_12_7 <= w_12_13 ? 7 : 13),12,result,1,
      channels);
  else
    CopyPixels(pixels,12,result,1,channels);
  if (
    w_12_6 + w_12_18 + w_16_10 + w_16_22 + (4 * w_11_17) <
    w_11_7 + w_11_15 + w_13_17 + w_17_21 + (4 * w_12_16)
  )
    Mix2Pixels(pixels,(ssize_t) (w_12_11 <= w_12_17 ? 11 : 17),12,result,2,
      channels);
  else
    CopyPixels(pixels,12,result,2,channels);
  if (
    w_12_8 + w_12_16 + w_18_14 + w_18_22 + (4 * w_13_17) <
    w_11_17 + w_17_23 + w_17_19 + w_7_13 + (4 * w_12_18)
  )
    Mix2Pixels(pixels,(ssize_t) (w_12_13 <= w_12_17 ? 13 : 17),12,result,3,
      channels);
  else
    CopyPixels(pixels,12,result,3,channels);
}

static void Scale2X(const Image *magick_unused(source),const Quantum *pixels,
  Quantum *result,const size_t channels)
{
  magick_unreferenced(source);

  if (PixelsEqual(pixels,1,pixels,7,channels) ||
      PixelsEqual(pixels,3,pixels,5,channels))
    {
      ssize_t
        i;

      for (i=0; i < 4; i++)
        CopyPixels(pixels,4,result,i,channels);
      return;
    }
    if (PixelsEqual(pixels,1,pixels,3,channels))
      CopyPixels(pixels,3,result,0,channels);
    else
      CopyPixels(pixels,4,result,0,channels);
    if (PixelsEqual(pixels,1,pixels,5,channels))
      CopyPixels(pixels,5,result,1,channels);
    else
      CopyPixels(pixels,4,result,1,channels);
    if (PixelsEqual(pixels,3,pixels,7,channels))
      CopyPixels(pixels,3,result,2,channels);
    else
      CopyPixels(pixels,4,result,2,channels);
    if (PixelsEqual(pixels,5,pixels,7,channels))
      CopyPixels(pixels,5,result,3,channels);
    else
      CopyPixels(pixels,4,result,3,channels);
}

static void Epbx2X(const Image *magick_unused(source),const Quantum *pixels,
  Quantum *result,const size_t channels)
{
#define HelperCond(a,b,c,d,e,f,g) ( \
  PixelsEqual(pixels,a,pixels,b,channels) && ( \
    PixelsEqual(pixels,c,pixels,d,channels) || \
    PixelsEqual(pixels,c,pixels,e,channels) || \
    PixelsEqual(pixels,a,pixels,f,channels) || \
    PixelsEqual(pixels,b,pixels,g,channels) \
    ) \
  )

  ssize_t
    i;

  magick_unreferenced(source);

  for (i=0; i < 4; i++)
    CopyPixels(pixels,4,result,i,channels);
  if (
    !PixelsEqual(pixels,3,pixels,5,channels) &&
    !PixelsEqual(pixels,1,pixels,7,channels) &&
    (
      PixelsEqual(pixels,4,pixels,3,channels) ||
      PixelsEqual(pixels,4,pixels,7,channels) ||
      PixelsEqual(pixels,4,pixels,5,channels) ||
      PixelsEqual(pixels,4,pixels,1,channels) ||
      (
        (
          !PixelsEqual(pixels,0,pixels,8,channels) ||
           PixelsEqual(pixels,4,pixels,6,channels) ||
           PixelsEqual(pixels,3,pixels,2,channels)
        ) &&
        (
          !PixelsEqual(pixels,6,pixels,2,channels) ||
           PixelsEqual(pixels,4,pixels,0,channels) ||
           PixelsEqual(pixels,4,pixels,8,channels)
        )
      )
    )
  )
    {
      if (HelperCond(1,3,4,0,8,2,6))
        Mix2Pixels(pixels,1,3,result,0,channels);
      if (HelperCond(5,1,4,2,6,8,0))
        Mix2Pixels(pixels,5,1,result,1,channels);
      if (HelperCond(3,7,4,6,2,0,8))
        Mix2Pixels(pixels,3,7,result,2,channels);
      if (HelperCond(7,5,4,8,0,6,2))
        Mix2Pixels(pixels,7,5,result,3,channels);
    }

#undef HelperCond
}

static inline void Eagle3X(const Image *magick_unused(source),
  const Quantum *pixels,Quantum *result,const size_t channels)
{
  ssize_t
    corner_tl,
    corner_tr,
    corner_bl,
    corner_br;

  magick_unreferenced(source);

  corner_tl=PixelsEqual(pixels,0,pixels,1,channels) &&
    PixelsEqual(pixels,0,pixels,3,channels);
  corner_tr=PixelsEqual(pixels,1,pixels,2,channels) &&
    PixelsEqual(pixels,2,pixels,5,channels);
  corner_bl=PixelsEqual(pixels,3,pixels,6,channels) &&
    PixelsEqual(pixels,6,pixels,7,channels);
  corner_br=PixelsEqual(pixels,5,pixels,7,channels) &&
    PixelsEqual(pixels,7,pixels,8,channels);
  CopyPixels(pixels,(ssize_t) (corner_tl ? 0 : 4),result,0,channels);
  if (corner_tl && corner_tr)
    Mix2Pixels(pixels,0,2,result,1,channels);
  else
    CopyPixels(pixels,4,result,1,channels);
  CopyPixels(pixels,(ssize_t) (corner_tr ? 1 : 4),result,2,channels);
  if (corner_tl && corner_bl)
    Mix2Pixels(pixels,0,6,result,3,channels);
  else
    CopyPixels(pixels,4,result,3,channels);
  CopyPixels(pixels,4,result,4,channels);
  if (corner_tr && corner_br)
    Mix2Pixels(pixels,2,8,result,5,channels);
  else
    CopyPixels(pixels,4,result,5,channels);
  CopyPixels(pixels,(ssize_t) (corner_bl ? 3 : 4),result,6,channels);
  if (corner_bl && corner_br)
    Mix2Pixels(pixels,6,8,result,7,channels);
  else
    CopyPixels(pixels,4,result,7,channels);
  CopyPixels(pixels,(ssize_t) (corner_br ? 5 : 4),result,8,channels);
}

static inline void Eagle3XB(const Image *magick_unused(source),
  const Quantum *pixels,Quantum *result,const size_t channels)
{
  ssize_t
    corner_tl,
    corner_tr,
    corner_bl,
    corner_br;

  magick_unreferenced(source);

  corner_tl=PixelsEqual(pixels,0,pixels,1,channels) &&
    PixelsEqual(pixels,0,pixels,3,channels);
  corner_tr=PixelsEqual(pixels,1,pixels,2,channels) &&
    PixelsEqual(pixels,2,pixels,5,channels);
  corner_bl=PixelsEqual(pixels,3,pixels,6,channels) &&
    PixelsEqual(pixels,6,pixels,7,channels);
  corner_br=PixelsEqual(pixels,5,pixels,7,channels) &&
    PixelsEqual(pixels,7,pixels,8,channels);
  CopyPixels(pixels,(ssize_t) (corner_tl ? 0 : 4),result,0,channels);
  CopyPixels(pixels,4,result,1,channels);
  CopyPixels(pixels,(ssize_t) (corner_tr ? 1 : 4),result,2,channels);
  CopyPixels(pixels,4,result,3,channels);
  CopyPixels(pixels,4,result,4,channels);
  CopyPixels(pixels,4,result,5,channels);
  CopyPixels(pixels,(ssize_t) (corner_bl ? 3 : 4),result,6,channels);
  CopyPixels(pixels,4,result,7,channels);
  CopyPixels(pixels,(ssize_t) (corner_br ? 5 : 4),result,8,channels);
}

static inline void Scale3X(const Image *magick_unused(source),
  const Quantum *pixels,Quantum *result,const size_t channels)
{
  magick_unreferenced(source);

  if (!PixelsEqual(pixels,1,pixels,7,channels) &&
      !PixelsEqual(pixels,3,pixels,5,channels))
    {
      if (PixelsEqual(pixels,3,pixels,1,channels))
        CopyPixels(pixels,3,result,0,channels);
      else
        CopyPixels(pixels,4,result,0,channels);

      if (
        (
          PixelsEqual(pixels,3,pixels,1,channels) &&
          !PixelsEqual(pixels,4,pixels,2,channels)
        ) ||
        (
          PixelsEqual(pixels,5,pixels,1,channels) &&
          !PixelsEqual(pixels,4,pixels,0,channels)
        )
      )
        CopyPixels(pixels,1,result,1,channels);
      else
        CopyPixels(pixels,4,result,1,channels);
      if (PixelsEqual(pixels,5,pixels,1,channels))
        CopyPixels(pixels,5,result,2,channels);
      else
        CopyPixels(pixels,4,result,2,channels);
      if (
        (
          PixelsEqual(pixels,3,pixels,1,channels) &&
          !PixelsEqual(pixels,4,pixels,6,channels)
        ) ||
        (
          PixelsEqual(pixels,3,pixels,7,channels) &&
          !PixelsEqual(pixels,4,pixels,0,channels)
        )
      )
        CopyPixels(pixels,3,result,3,channels);
      else
        CopyPixels(pixels,4,result,3,channels);
      CopyPixels(pixels,4,result,4,channels);
      if (
        (
          PixelsEqual(pixels,5,pixels,1,channels) &&
          !PixelsEqual(pixels,4,pixels,8,channels)
        ) ||
        (
          PixelsEqual(pixels,5,pixels,7,channels) &&
          !PixelsEqual(pixels,4,pixels,2,channels)
        )
      )
        CopyPixels(pixels,5,result,5,channels);
      else
        CopyPixels(pixels,4,result,5,channels);
      if (PixelsEqual(pixels,3,pixels,7,channels))
        CopyPixels(pixels,3,result,6,channels);
      else
        CopyPixels(pixels,4,result,6,channels);
      if (
        (
          PixelsEqual(pixels,3,pixels,7,channels) &&
          !PixelsEqual(pixels,4,pixels,8,channels)
        ) ||
        (
          PixelsEqual(pixels,5,pixels,7,channels) &&
          !PixelsEqual(pixels,4,pixels,6,channels)
        )
      )
        CopyPixels(pixels,7,result,7,channels);
      else
        CopyPixels(pixels,4,result,7,channels);
      if (PixelsEqual(pixels,5,pixels,7,channels))
        CopyPixels(pixels,5,result,8,channels);
      else
        CopyPixels(pixels,4,result,8,channels);
    }
  else
    {
      ssize_t
        i;

      for (i=0; i < 9; i++)
        CopyPixels(pixels,4,result,i,channels);
    }
}

MagickExport Image *MagnifyImage(const Image *image,ExceptionInfo *exception)
{
#define MagnifyImageTag  "Magnify/Image"

  CacheView
    *image_view,
    *magnify_view;

  const char
    *option;

  Image
    *source_image,
    *magnify_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  OffsetInfo
    offset;

  RectangleInfo
    rectangle;

  ssize_t
    y;

  unsigned char
    magnification,
    width;

  void
    (*scaling_method)(const Image *,const Quantum *,Quantum *,size_t);

  /*
    Initialize magnified image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  option=GetImageOption(image->image_info,"magnify:method");
  if (option == (char *) NULL)
    option="scale2x";
  scaling_method=Scale2X;
  magnification=1;
  width=1;
  switch (*option)
  {
    case 'e':
    {
      if (LocaleCompare(option,"eagle2x") == 0)
        {
          scaling_method=Eagle2X;
          magnification=2;
          width=3;
          break;
        }
      if (LocaleCompare(option,"eagle3x") == 0)
        {
          scaling_method=Eagle3X;
          magnification=3;
          width=3;
          break;
        }
      if (LocaleCompare(option,"eagle3xb") == 0)
        {
          scaling_method=Eagle3XB;
          magnification=3;
          width=3;
          break;
        }
      if (LocaleCompare(option,"epbx2x") == 0)
        {
          scaling_method=Epbx2X;
          magnification=2;
          width=3;
          break;
        }
      break;
    }
    case 'f':
    {
      if (LocaleCompare(option,"fish2x") == 0)
        {
          scaling_method=Fish2X;
          magnification=2;
          width=3;
          break;
        }
      break;
    }
    case 'h':
    {
      if (LocaleCompare(option,"hq2x") == 0)
        {
          scaling_method=Hq2X;
          magnification=2;
          width=3;
          break;
        }
      break;
    }
    case 's':
    {
      if (LocaleCompare(option,"scale2x") == 0)
        {
          scaling_method=Scale2X;
          magnification=2;
          width=3;
          break;
        }
      if (LocaleCompare(option,"scale3x") == 0)
        {
          scaling_method=Scale3X;
          magnification=3;
          width=3;
          break;
        }
      break;
    }
    case 'x':
    {
      if (LocaleCompare(option,"xbr2x") == 0)
        {
          scaling_method=Xbr2X;
          magnification=2;
          width=5;
        }
      break;
    }
    default:
      break;
  }
  /*
    Make a working copy of the source image and convert it to RGB colorspace.
  */
  source_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (source_image == (Image *) NULL)
    return((Image *) NULL);
  offset.x=0;
  offset.y=0;
  rectangle.x=0;
  rectangle.y=0;
  rectangle.width=image->columns;
  rectangle.height=image->rows;
  (void) CopyImagePixels(source_image,image,&rectangle,&offset,exception);
  if (IssRGBCompatibleColorspace(source_image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(source_image,sRGBColorspace,exception);
  magnify_image=CloneImage(source_image,magnification*source_image->columns,
    magnification*source_image->rows,MagickTrue,exception);
  if (magnify_image == (Image *) NULL)
    {
      source_image=DestroyImage(source_image);
      return((Image *) NULL);
    }
  /*
    Magnify the image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(source_image,exception);
  magnify_view=AcquireAuthenticCacheView(magnify_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(source_image,magnify_image,source_image->rows,1)
#endif
  for (y=0; y < (ssize_t) source_image->rows; y++)
  {
    Quantum
      r[128]; /* to hold result pixels */

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(magnify_view,0,magnification*y,
      magnify_image->columns,magnification,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    /*
      Magnify this row of pixels.
    */
    for (x=0; x < (ssize_t) source_image->columns; x++)
    {
      const Quantum
        *magick_restrict p;

      size_t
        channels;

      ssize_t
        i,
        j;

      p=GetCacheViewVirtualPixels(image_view,x-width/2,y-width/2,width,width,
        exception);
      if (p == (Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      channels=GetPixelChannels(source_image);
      scaling_method(source_image,p,r,channels);
      /*
        Copy the result pixels into the final image.
      */
      for (j=0; j < (ssize_t) magnification; j++)
        for (i=0; i < (ssize_t) (channels*magnification); i++)
          q[j*(ssize_t) channels*(ssize_t) magnify_image->columns+i]=
            r[j*magnification*(ssize_t) channels+i];
      q+=(ptrdiff_t) magnification*GetPixelChannels(magnify_image);
    }
    if (SyncCacheViewAuthenticPixels(magnify_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,MagnifyImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  magnify_view=DestroyCacheView(magnify_view);
  image_view=DestroyCacheView(image_view);
  source_image=DestroyImage(source_image);
  if (status == MagickFalse)
    magnify_image=DestroyImage(magnify_image);
  return(magnify_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M i n i f y I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MinifyImage() is a convenience method that scales an image proportionally to
%  half its size.
%
%  The format of the MinifyImage method is:
%
%      Image *MinifyImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *MinifyImage(const Image *image,ExceptionInfo *exception)
{
  Image
    *minify_image;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  minify_image=ResizeImage(image,image->columns/2,image->rows/2,SplineFilter,
    exception);
  return(minify_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s a m p l e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResampleImage() resize image in terms of its pixel size, so that when
%  displayed at the given resolution it will be the same size in terms of
%  real world units as the original image at the original resolution.
%
%  The format of the ResampleImage method is:
%
%      Image *ResampleImage(Image *image,const double x_resolution,
%        const double y_resolution,const FilterType filter,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image to be resized to fit the given resolution.
%
%    o x_resolution: the new image x resolution.
%
%    o y_resolution: the new image y resolution.
%
%    o filter: Image filter to use.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ResampleImage(const Image *image,const double x_resolution,
  const double y_resolution,const FilterType filter,ExceptionInfo *exception)
{
#define ResampleImageTag  "Resample/Image"

  Image
    *resample_image;

  size_t
    height,
    width;

  /*
    Initialize sampled image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  width=(size_t) (x_resolution*image->columns/(image->resolution.x == 0.0 ?
    DefaultResolution : image->resolution.x)+0.5);
  height=(size_t) (y_resolution*image->rows/(image->resolution.y == 0.0 ?
    DefaultResolution : image->resolution.y)+0.5);
  resample_image=ResizeImage(image,width,height,filter,exception);
  if (resample_image != (Image *) NULL)
    {
      resample_image->resolution.x=x_resolution;
      resample_image->resolution.y=y_resolution;
    }
  return(resample_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s i z e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResizeImage() scales an image to the desired dimensions, using the given
%  filter (see AcquireFilterInfo()).
%
%  If an undefined filter is given the filter defaults to Mitchell for a
%  colormapped image, a image with a matte channel, or if the image is
%  enlarged.  Otherwise the filter defaults to a Lanczos.
%
%  ResizeImage() was inspired by Paul Heckbert's "zoom" program.
%
%  The format of the ResizeImage method is:
%
%      Image *ResizeImage(Image *image,const size_t columns,const size_t rows,
%        const FilterType filter,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
%    o filter: Image filter to use.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _ContributionInfo
{
  double
    weight;

  ssize_t
    pixel;
} ContributionInfo;

static ContributionInfo **DestroyContributionTLS(
  ContributionInfo **contribution)
{
  ssize_t
    i;

  assert(contribution != (ContributionInfo **) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (contribution[i] != (ContributionInfo *) NULL)
      contribution[i]=(ContributionInfo *) RelinquishAlignedMemory(
        contribution[i]);
  contribution=(ContributionInfo **) RelinquishMagickMemory(contribution);
  return(contribution);
}

static ContributionInfo **AcquireContributionTLS(const size_t count)
{
  ssize_t
    i;

  ContributionInfo
    **contribution;

  size_t
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  contribution=(ContributionInfo **) AcquireQuantumMemory(number_threads,
    sizeof(*contribution));
  if (contribution == (ContributionInfo **) NULL)
    return((ContributionInfo **) NULL);
  (void) memset(contribution,0,number_threads*sizeof(*contribution));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    contribution[i]=(ContributionInfo *) MagickAssumeAligned(
      AcquireAlignedMemory(count,sizeof(**contribution)));
    if (contribution[i] == (ContributionInfo *) NULL)
      return(DestroyContributionTLS(contribution));
  }
  return(contribution);
}

static MagickBooleanType HorizontalFilter(
  const ResizeFilter *magick_restrict resize_filter,
  const Image *magick_restrict image,Image *magick_restrict resize_image,
  const double x_factor,const MagickSizeType span,
  MagickOffsetType *magick_restrict progress,ExceptionInfo *exception)
{
#define ResizeImageTag  "Resize/Image"

  CacheView
    *image_view,
    *resize_view;

  ClassType
    storage_class;

  ContributionInfo
    **magick_restrict contributions;

  double
    scale,
    support;

  MagickBooleanType
    status;

  ssize_t
    x;

  /*
    Apply filter to resize horizontally from image to resize image.
  */
  scale=MagickMax(1.0/x_factor+MagickEpsilon,1.0);
  support=scale*GetResizeFilterSupport(resize_filter);
  storage_class=support > 0.5 ? DirectClass : image->storage_class;
  if (SetImageStorageClass(resize_image,storage_class,exception) == MagickFalse)
    return(MagickFalse);
  if (support < 0.5)
    {
      /*
        Support too small even for nearest neighbour: Reduce to point sampling.
      */
      support=(double) 0.5;
      scale=1.0;
    }
  contributions=AcquireContributionTLS((size_t) (2.0*support+3.0));
  if (contributions == (ContributionInfo **) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  status=MagickTrue;
  scale=MagickSafeReciprocal(scale);
  image_view=AcquireVirtualCacheView(image,exception);
  resize_view=AcquireAuthenticCacheView(resize_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,resize_image,resize_image->columns,1)
#endif
  for (x=0; x < (ssize_t) resize_image->columns; x++)
  {
    const int
      id = GetOpenMPThreadId();

    const Quantum
      *magick_restrict p;

    ContributionInfo
      *magick_restrict contribution;

    double
      bisect,
      density;

    Quantum
      *magick_restrict q;

    ssize_t
      n,
      start,
      stop,
      y;

    if (status == MagickFalse)
      continue;
    bisect=(double) (x+0.5)/x_factor+MagickEpsilon;
    start=(ssize_t) MagickMax(bisect-support+0.5,0.0);
    stop=(ssize_t) MagickMin(bisect+support+0.5,(double) image->columns);
    density=0.0;
    contribution=contributions[id];
    for (n=0; n < (stop-start); n++)
    {
      contribution[n].pixel=start+n;
      contribution[n].weight=GetResizeFilterWeight(resize_filter,scale*
        ((double) (start+n)-bisect+0.5));
      density+=contribution[n].weight;
    }
    if (n == 0)
      continue;
    if ((density != 0.0) && (density != 1.0))
      {
        ssize_t
          i;

        /*
          Normalize.
        */
        density=MagickSafeReciprocal(density);
        for (i=0; i < n; i++)
          contribution[i].weight*=density;
      }
    p=GetCacheViewVirtualPixels(image_view,contribution[0].pixel,0,(size_t)
      (contribution[n-1].pixel-contribution[0].pixel+1),image->rows,exception);
    q=QueueCacheViewAuthenticPixels(resize_view,x,0,1,resize_image->rows,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (y=0; y < (ssize_t) resize_image->rows; y++)
    {
      ssize_t
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
          resize_traits,
          traits;

        ssize_t
          j,
          k;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        resize_traits=GetPixelChannelTraits(resize_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (resize_traits == UndefinedPixelTrait))
          continue;
        if (((resize_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(resize_image,q) <= (QuantumRange/2)))
          {
            j=(ssize_t) (MagickMin(MagickMax(bisect,(double) start),(double)
              stop-1.0)+0.5);
            k=y*(contribution[n-1].pixel-contribution[0].pixel+1)+
              (contribution[j-start].pixel-contribution[0].pixel);
            SetPixelChannel(resize_image,channel,
              p[k*(ssize_t) GetPixelChannels(image)+i],q);
            continue;
          }
        pixel=0.0;
        if ((resize_traits & BlendPixelTrait) == 0)
          {
            /*
              No alpha blending.
            */
            for (j=0; j < n; j++)
            {
              k=y*(contribution[n-1].pixel-contribution[0].pixel+1)+
                (contribution[j].pixel-contribution[0].pixel);
              alpha=contribution[j].weight;
              pixel+=alpha*(double) p[k*(ssize_t) GetPixelChannels(image)+i];
            }
            SetPixelChannel(resize_image,channel,ClampToQuantum(pixel),q);
            continue;
          }
        /*
          Alpha blending.
        */
        gamma=0.0;
        for (j=0; j < n; j++)
        {
          k=y*(contribution[n-1].pixel-contribution[0].pixel+1)+
            (contribution[j].pixel-contribution[0].pixel);
          alpha=contribution[j].weight*QuantumScale*
            (double) GetPixelAlpha(image,p+k*(ssize_t) GetPixelChannels(image));
          pixel+=alpha*(double) p[k*(ssize_t) GetPixelChannels(image)+i];
          gamma+=alpha;
        }
        gamma=MagickSafeReciprocal(gamma);
        SetPixelChannel(resize_image,channel,ClampToQuantum(gamma*pixel),q);
      }
      q+=(ptrdiff_t) GetPixelChannels(resize_image);
    }
    if (SyncCacheViewAuthenticPixels(resize_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        (*progress)++;
        proceed=SetImageProgress(image,ResizeImageTag,*progress,span);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  resize_view=DestroyCacheView(resize_view);
  image_view=DestroyCacheView(image_view);
  contributions=DestroyContributionTLS(contributions);
  return(status);
}

static MagickBooleanType VerticalFilter(
  const ResizeFilter *magick_restrict resize_filter,
  const Image *magick_restrict image,Image *magick_restrict resize_image,
  const double y_factor,const MagickSizeType span,
  MagickOffsetType *magick_restrict progress,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *resize_view;

  ClassType
    storage_class;

  ContributionInfo
    **magick_restrict contributions;

  double
    scale,
    support;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Apply filter to resize vertically from image to resize image.
  */
  scale=MagickMax(1.0/y_factor+MagickEpsilon,1.0);
  support=scale*GetResizeFilterSupport(resize_filter);
  storage_class=support > 0.5 ? DirectClass : image->storage_class;
  if (SetImageStorageClass(resize_image,storage_class,exception) == MagickFalse)
    return(MagickFalse);
  if (support < 0.5)
    {
      /*
        Support too small even for nearest neighbour: Reduce to point sampling.
      */
      support=(double) 0.5;
      scale=1.0;
    }
  contributions=AcquireContributionTLS((size_t) (2.0*support+3.0));
  if (contributions == (ContributionInfo **) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  status=MagickTrue;
  scale=MagickSafeReciprocal(scale);
  image_view=AcquireVirtualCacheView(image,exception);
  resize_view=AcquireAuthenticCacheView(resize_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,resize_image,resize_image->rows,1)
#endif
  for (y=0; y < (ssize_t) resize_image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    const Quantum
      *magick_restrict p;

    ContributionInfo
      *magick_restrict contribution;

    double
      bisect,
      density;

    Quantum
      *magick_restrict q;

    ssize_t
      n,
      start,
      stop,
      x;

    if (status == MagickFalse)
      continue;
    bisect=(double) (y+0.5)/y_factor+MagickEpsilon;
    start=(ssize_t) MagickMax(bisect-support+0.5,0.0);
    stop=(ssize_t) MagickMin(bisect+support+0.5,(double) image->rows);
    density=0.0;
    contribution=contributions[id];
    for (n=0; n < (stop-start); n++)
    {
      contribution[n].pixel=start+n;
      contribution[n].weight=GetResizeFilterWeight(resize_filter,scale*
        ((double) (start+n)-bisect+0.5));
      density+=contribution[n].weight;
    }
    if (n == 0)
      continue;
    if ((density != 0.0) && (density != 1.0))
      {
        ssize_t
          i;

        /*
          Normalize.
        */
        density=MagickSafeReciprocal(density);
        for (i=0; i < n; i++)
          contribution[i].weight*=density;
      }
    p=GetCacheViewVirtualPixels(image_view,0,contribution[0].pixel,
      image->columns,(size_t) (contribution[n-1].pixel-contribution[0].pixel+1),
      exception);
    q=QueueCacheViewAuthenticPixels(resize_view,0,y,resize_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) resize_image->columns; x++)
    {
      ssize_t
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
          resize_traits,
          traits;

        ssize_t
          j,
          k;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        resize_traits=GetPixelChannelTraits(resize_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (resize_traits == UndefinedPixelTrait))
          continue;
        if (((resize_traits & CopyPixelTrait) != 0) ||
            (GetPixelWriteMask(resize_image,q) <= (QuantumRange/2)))
          {
            j=(ssize_t) (MagickMin(MagickMax(bisect,(double) start),(double)
              stop-1.0)+0.5);
            k=(ssize_t) ((contribution[j-start].pixel-contribution[0].pixel)*
              (ssize_t) image->columns+x);
            SetPixelChannel(resize_image,channel,p[k*(ssize_t)
              GetPixelChannels(image)+i],q);
            continue;
          }
        pixel=0.0;
        if ((resize_traits & BlendPixelTrait) == 0)
          {
            /*
              No alpha blending.
            */
            for (j=0; j < n; j++)
            {
              k=(ssize_t) ((contribution[j].pixel-contribution[0].pixel)*
                (ssize_t) image->columns+x);
              alpha=contribution[j].weight;
              pixel+=alpha*(double) p[k*(ssize_t) GetPixelChannels(image)+i];
            }
            SetPixelChannel(resize_image,channel,ClampToQuantum(pixel),q);
            continue;
          }
        gamma=0.0;
        for (j=0; j < n; j++)
        {
          k=(ssize_t) ((contribution[j].pixel-contribution[0].pixel)*
            (ssize_t) image->columns+x);
          alpha=contribution[j].weight*QuantumScale*(double)
           GetPixelAlpha(image,p+k*(ssize_t) GetPixelChannels(image));
          pixel+=alpha*(double) p[k*(ssize_t) GetPixelChannels(image)+i];
          gamma+=alpha;
        }
        gamma=MagickSafeReciprocal(gamma);
        SetPixelChannel(resize_image,channel,ClampToQuantum(gamma*pixel),q);
      }
      q+=(ptrdiff_t) GetPixelChannels(resize_image);
    }
    if (SyncCacheViewAuthenticPixels(resize_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        (*progress)++;
        proceed=SetImageProgress(image,ResizeImageTag,*progress,span);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  resize_view=DestroyCacheView(resize_view);
  image_view=DestroyCacheView(image_view);
  contributions=DestroyContributionTLS(contributions);
  return(status);
}

MagickExport Image *ResizeImage(const Image *image,const size_t columns,
  const size_t rows,const FilterType filter,ExceptionInfo *exception)
{
  double
    x_factor,
    y_factor;

  FilterType
    filter_type;

  Image
    *filter_image,
    *resize_image;

  MagickOffsetType
    offset;

  MagickSizeType
    span;

  MagickStatusType
    status;

  ResizeFilter
    *resize_filter;

  /*
    Acquire resize image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((columns == 0) || (rows == 0))
    ThrowImageException(ImageError,"NegativeOrZeroImageSize");
  if ((columns == image->columns) && (rows == image->rows) &&
      (filter == UndefinedFilter))
    return(CloneImage(image,0,0,MagickTrue,exception));
  /*
    Acquire resize filter.
  */
  x_factor=(double) (columns*MagickSafeReciprocal((double) image->columns));
  y_factor=(double) (rows*MagickSafeReciprocal((double) image->rows));
  filter_type=LanczosFilter;
  if (filter != UndefinedFilter)
    filter_type=filter;
  else
    if ((x_factor == 1.0) && (y_factor == 1.0))
      filter_type=PointFilter;
    else
      if ((image->storage_class == PseudoClass) ||
          (image->alpha_trait != UndefinedPixelTrait) ||
          ((x_factor*y_factor) > 1.0))
        filter_type=MitchellFilter;
  resize_filter=AcquireResizeFilter(image,filter_type,MagickFalse,exception);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  resize_image=AccelerateResizeImage(image,columns,rows,resize_filter,
    exception);
  if (resize_image != (Image *) NULL)
    {
      resize_filter=DestroyResizeFilter(resize_filter);
      return(resize_image);
    }
#endif
  resize_image=CloneImage(image,columns,rows,MagickTrue,exception);
  if (resize_image == (Image *) NULL)
    {
      resize_filter=DestroyResizeFilter(resize_filter);
      return(resize_image);
    }
  if (x_factor > y_factor)
    filter_image=CloneImage(image,columns,image->rows,MagickTrue,exception);
  else
    filter_image=CloneImage(image,image->columns,rows,MagickTrue,exception);
  if (filter_image == (Image *) NULL)
    {
      resize_filter=DestroyResizeFilter(resize_filter);
      return(DestroyImage(resize_image));
    }
  /*
    Resize image.
  */
  offset=0;
  if (x_factor > y_factor)
    {
      span=(MagickSizeType) (filter_image->columns+rows);
      status=HorizontalFilter(resize_filter,image,filter_image,x_factor,span,
        &offset,exception);
      status&=(MagickStatusType) VerticalFilter(resize_filter,filter_image,
        resize_image,y_factor,span,&offset,exception);
    }
  else
    {
      span=(MagickSizeType) (filter_image->rows+columns);
      status=VerticalFilter(resize_filter,image,filter_image,y_factor,span,
        &offset,exception);
      status&=(MagickStatusType) HorizontalFilter(resize_filter,filter_image,
        resize_image,x_factor,span,&offset,exception);
    }
  /*
    Free resources.
  */
  filter_image=DestroyImage(filter_image);
  resize_filter=DestroyResizeFilter(resize_filter);
  if (status == MagickFalse)
    {
      resize_image=DestroyImage(resize_image);
      return((Image *) NULL);
    }
  resize_image->type=image->type;
  return(resize_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S a m p l e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SampleImage() scales an image to the desired dimensions with pixel
%  sampling.  Unlike other scaling methods, this method does not introduce
%  any additional color into the scaled image.
%
%  The format of the SampleImage method is:
%
%      Image *SampleImage(const Image *image,const size_t columns,
%        const size_t rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the sampled image.
%
%    o rows: the number of rows in the sampled image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SampleImage(const Image *image,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
{
#define SampleImageTag  "Sample/Image"

  CacheView
    *image_view,
    *sample_view;

  Image
    *sample_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PointInfo
    sample_offset;

  ssize_t
    j,
    *x_offset,
    y;

  /*
    Initialize sampled image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((columns == 0) || (rows == 0))
    ThrowImageException(ImageError,"NegativeOrZeroImageSize");
  if ((columns == image->columns) && (rows == image->rows))
    return(CloneImage(image,0,0,MagickTrue,exception));
  sample_image=CloneImage(image,columns,rows,MagickTrue,exception);
  if (sample_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Set the sampling offset, default is in the mid-point of sample regions.
  */
  sample_offset.x=0.5-MagickEpsilon;
  sample_offset.y=sample_offset.x;
  {
    const char
      *value;

    value=GetImageArtifact(image,"sample:offset");
    if (value != (char *) NULL)
      {
        GeometryInfo
          geometry_info;

        MagickStatusType
          flags;

        (void) ParseGeometry(value,&geometry_info);
        flags=ParseGeometry(value,&geometry_info);
        sample_offset.x=sample_offset.y=geometry_info.rho/100.0-MagickEpsilon;
        if ((flags & SigmaValue) != 0)
          sample_offset.y=geometry_info.sigma/100.0-MagickEpsilon;
      }
  }
  /*
    Allocate scan line buffer and column offset buffers.
  */
  x_offset=(ssize_t *) AcquireQuantumMemory((size_t) sample_image->columns,
    sizeof(*x_offset));
  if (x_offset == (ssize_t *) NULL)
    {
      sample_image=DestroyImage(sample_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  for (j=0; j < (ssize_t) sample_image->columns; j++)
    x_offset[j]=(ssize_t) ((((double) j+sample_offset.x)*image->columns)/
      sample_image->columns);
  /*
    Sample each row.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  sample_view=AcquireAuthenticCacheView(sample_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,sample_image,sample_image->rows,2)
#endif
  for (y=0; y < (ssize_t) sample_image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x,
      y_offset;

    if (status == MagickFalse)
      continue;
    y_offset=(ssize_t) ((((double) y+sample_offset.y)*image->rows)/
      sample_image->rows);
    p=GetCacheViewVirtualPixels(image_view,0,y_offset,image->columns,1,
      exception);
    q=QueueCacheViewAuthenticPixels(sample_view,0,y,sample_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    /*
      Sample each column.
    */
    for (x=0; x < (ssize_t) sample_image->columns; x++)
    {
      ssize_t
        i;

      if (GetPixelWriteMask(sample_image,q) <= (QuantumRange/2))
        {
          q+=(ptrdiff_t) GetPixelChannels(sample_image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(sample_image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          image_traits,
          traits;

        channel=GetPixelChannelChannel(sample_image,i);
        traits=GetPixelChannelTraits(sample_image,channel);
        image_traits=GetPixelChannelTraits(image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (image_traits == UndefinedPixelTrait))
          continue;
        SetPixelChannel(sample_image,channel,p[x_offset[x]*(ssize_t)
          GetPixelChannels(image)+i],q);
      }
      q+=(ptrdiff_t) GetPixelChannels(sample_image);
    }
    if (SyncCacheViewAuthenticPixels(sample_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,SampleImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  sample_view=DestroyCacheView(sample_view);
  x_offset=(ssize_t *) RelinquishMagickMemory(x_offset);
  sample_image->type=image->type;
  if (status == MagickFalse)
    sample_image=DestroyImage(sample_image);
  return(sample_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S c a l e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ScaleImage() changes the size of an image to the given dimensions.
%
%  The format of the ScaleImage method is:
%
%      Image *ScaleImage(const Image *image,const size_t columns,
%        const size_t rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ScaleImage(const Image *image,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
{
#define ScaleImageTag  "Scale/Image"

  CacheView
    *image_view,
    *scale_view;

  double
    alpha,
    pixel[CompositePixelChannel],
    *scale_scanline,
    *scanline,
    *x_vector,
    *y_vector;

  Image
    *scale_image;

  MagickBooleanType
    next_column,
    next_row,
    proceed,
    status;

  PixelTrait
    scale_traits;

  PointInfo
    scale,
    span;

  ssize_t
    i,
    n,
    number_rows,
    y;

  /*
    Initialize scaled image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((columns == 0) || (rows == 0))
    ThrowImageException(ImageError,"NegativeOrZeroImageSize");
  if ((columns == image->columns) && (rows == image->rows))
    return(CloneImage(image,0,0,MagickTrue,exception));
  scale_image=CloneImage(image,columns,rows,MagickTrue,exception);
  if (scale_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(scale_image,DirectClass,exception) == MagickFalse)
    {
      scale_image=DestroyImage(scale_image);
      return((Image *) NULL);
    }
  /*
    Allocate memory.
  */
  x_vector=(double *) AcquireQuantumMemory((size_t) image->columns,
    MaxPixelChannels*sizeof(*x_vector));
  scanline=x_vector;
  if (image->rows != scale_image->rows)
    scanline=(double *) AcquireQuantumMemory((size_t) image->columns,
      MaxPixelChannels*sizeof(*scanline));
  scale_scanline=(double *) AcquireQuantumMemory((size_t) scale_image->columns,
    MaxPixelChannels*sizeof(*scale_scanline));
  y_vector=(double *) AcquireQuantumMemory((size_t) image->columns,
    MaxPixelChannels*sizeof(*y_vector));
  if ((scanline == (double *) NULL) || (scale_scanline == (double *) NULL) ||
      (x_vector == (double *) NULL) || (y_vector == (double *) NULL))
    {
      if ((image->rows != scale_image->rows) && (scanline != (double *) NULL))
        scanline=(double *) RelinquishMagickMemory(scanline);
      if (scale_scanline != (double *) NULL)
        scale_scanline=(double *) RelinquishMagickMemory(scale_scanline);
      if (x_vector != (double *) NULL)
        x_vector=(double *) RelinquishMagickMemory(x_vector);
      if (y_vector != (double *) NULL)
        y_vector=(double *) RelinquishMagickMemory(y_vector);
      scale_image=DestroyImage(scale_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Scale image.
  */
  number_rows=0;
  next_row=MagickTrue;
  span.y=1.0;
  scale.y=(double) scale_image->rows/(double) image->rows;
  (void) memset(y_vector,0,(size_t) MaxPixelChannels*image->columns*
    sizeof(*y_vector));
  n=0;
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  scale_view=AcquireAuthenticCacheView(scale_image,exception);
  for (y=0; y < (ssize_t) scale_image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      break;
    q=QueueCacheViewAuthenticPixels(scale_view,0,y,scale_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        break;
      }
    alpha=1.0;
    if (scale_image->rows == image->rows)
      {
        /*
          Read a new scanline.
        */
        p=GetCacheViewVirtualPixels(image_view,0,n++,image->columns,1,
          exception);
        if (p == (const Quantum *) NULL)
          {
            status=MagickFalse;
            break;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          if (GetPixelWriteMask(image,p) <= (QuantumRange/2))
            {
              p+=(ptrdiff_t) GetPixelChannels(image);
              continue;
            }
          if (image->alpha_trait != UndefinedPixelTrait)
            alpha=QuantumScale*(double) GetPixelAlpha(image,p);
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if ((traits & BlendPixelTrait) == 0)
              {
                x_vector[x*(ssize_t) GetPixelChannels(image)+i]=(double) p[i];
                continue;
              }
            x_vector[x*(ssize_t) GetPixelChannels(image)+i]=alpha*(double) p[i];
          }
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
    else
      {
        /*
          Scale Y direction.
        */
        while (scale.y < span.y)
        {
          if ((next_row != MagickFalse) &&
              (number_rows < (ssize_t) image->rows))
            {
              /*
                Read a new scanline.
              */
              p=GetCacheViewVirtualPixels(image_view,0,n++,image->columns,1,
                exception);
              if (p == (const Quantum *) NULL)
                {
                  status=MagickFalse;
                  break;
                }
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                if (GetPixelWriteMask(image,p) <= (QuantumRange/2))
                  {
                    p+=(ptrdiff_t) GetPixelChannels(image);
                    continue;
                  }
                if (image->alpha_trait != UndefinedPixelTrait)
                  alpha=QuantumScale*(double) GetPixelAlpha(image,p);
                for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
                {
                  PixelChannel channel = GetPixelChannelChannel(image,i);
                  PixelTrait traits = GetPixelChannelTraits(image,channel);
                  if ((traits & BlendPixelTrait) == 0)
                    {
                      x_vector[x*(ssize_t) GetPixelChannels(image)+i]=
                        (double) p[i];
                      continue;
                    }
                  x_vector[x*(ssize_t) GetPixelChannels(image)+i]=alpha*
                    (double) p[i];
                }
                p+=(ptrdiff_t) GetPixelChannels(image);
              }
              number_rows++;
            }
          for (x=0; x < (ssize_t) image->columns; x++)
            for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
              y_vector[x*(ssize_t) GetPixelChannels(image)+i]+=scale.y*
                x_vector[x*(ssize_t) GetPixelChannels(image)+i];
          span.y-=scale.y;
          scale.y=(double) scale_image->rows/(double) image->rows;
          next_row=MagickTrue;
        }
        if ((next_row != MagickFalse) && (number_rows < (ssize_t) image->rows))
          {
            /*
              Read a new scanline.
            */
            p=GetCacheViewVirtualPixels(image_view,0,n++,image->columns,1,
              exception);
            if (p == (const Quantum *) NULL)
              {
                status=MagickFalse;
                break;
              }
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (GetPixelWriteMask(image,p) <= (QuantumRange/2))
                {
                  p+=(ptrdiff_t) GetPixelChannels(image);
                  continue;
                }
              if (image->alpha_trait != UndefinedPixelTrait)
                alpha=QuantumScale*(double) GetPixelAlpha(image,p);
              for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
              {
                PixelChannel channel = GetPixelChannelChannel(image,i);
                PixelTrait traits = GetPixelChannelTraits(image,channel);
                if ((traits & BlendPixelTrait) == 0)
                  {
                    x_vector[x*(ssize_t) GetPixelChannels(image)+i]=
                      (double) p[i];
                    continue;
                  }
                x_vector[x*(ssize_t) GetPixelChannels(image)+i]=alpha*
                  (double) p[i];
              }
              p+=(ptrdiff_t) GetPixelChannels(image);
            }
            number_rows++;
            next_row=MagickFalse;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            pixel[i]=y_vector[x*(ssize_t) GetPixelChannels(image)+i]+span.y*
              x_vector[x*(ssize_t) GetPixelChannels(image)+i];
            scanline[x*(ssize_t) GetPixelChannels(image)+i]=pixel[i];
            y_vector[x*(ssize_t) GetPixelChannels(image)+i]=0.0;
          }
        }
        scale.y-=span.y;
        if (scale.y <= 0)
          {
            scale.y=(double) scale_image->rows/(double) image->rows;
            next_row=MagickTrue;
          }
        span.y=1.0;
      }
    if (scale_image->columns == image->columns)
      {
        /*
          Transfer scanline to scaled image.
        */
        for (x=0; x < (ssize_t) scale_image->columns; x++)
        {
          if (GetPixelWriteMask(scale_image,q) <= (QuantumRange/2))
            {
              q+=(ptrdiff_t) GetPixelChannels(scale_image);
              continue;
            }
          if (image->alpha_trait != UndefinedPixelTrait)
            {
              alpha=QuantumScale*scanline[x*(ssize_t) GetPixelChannels(image)+
                GetPixelChannelOffset(image,AlphaPixelChannel)];
              alpha=MagickSafeReciprocal(alpha);
            }
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            scale_traits=GetPixelChannelTraits(scale_image,channel);
            if ((traits == UndefinedPixelTrait) ||
                (scale_traits == UndefinedPixelTrait))
              continue;
            if ((traits & BlendPixelTrait) == 0)
              {
                SetPixelChannel(scale_image,channel,ClampToQuantum(
                  scanline[x*(ssize_t) GetPixelChannels(image)+i]),q);
                continue;
              }
            SetPixelChannel(scale_image,channel,ClampToQuantum(alpha*scanline[
              x*(ssize_t) GetPixelChannels(image)+i]),q);
          }
          q+=(ptrdiff_t) GetPixelChannels(scale_image);
        }
      }
    else
      {
        ssize_t
          t;

        /*
          Scale X direction.
        */
        for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          pixel[i]=0.0;
        next_column=MagickFalse;
        span.x=1.0;
        t=0;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          scale.x=(double) scale_image->columns/(double) image->columns;
          while (scale.x >= span.x)
          {
            if (next_column != MagickFalse)
              {
                for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
                  pixel[i]=0.0;
                t++;
              }
            for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
            {
              PixelChannel channel = GetPixelChannelChannel(image,i);
              PixelTrait traits = GetPixelChannelTraits(image,channel);
              if (traits == UndefinedPixelTrait)
                continue;
              pixel[i]+=span.x*scanline[x*(ssize_t) GetPixelChannels(image)+i];
              scale_scanline[t*(ssize_t) GetPixelChannels(image)+i]=pixel[i];
            }
            scale.x-=span.x;
            span.x=1.0;
            next_column=MagickTrue;
          }
          if (scale.x > 0)
            {
              if (next_column != MagickFalse)
                {
                  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
                    pixel[i]=0.0;
                  next_column=MagickFalse;
                  t++;
                }
              for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
                pixel[i]+=scale.x*scanline[x*(ssize_t)
                  GetPixelChannels(image)+i];
              span.x-=scale.x;
            }
        }
      if (span.x > 0)
        {
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
            pixel[i]+=span.x*
              scanline[(x-1)*(ssize_t) GetPixelChannels(image)+i];
        }
      if ((next_column == MagickFalse) && (t < (ssize_t) scale_image->columns))
        for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          scale_scanline[t*(ssize_t) GetPixelChannels(image)+i]=pixel[i];
      /*
        Transfer scanline to scaled image.
      */
      for (x=0; x < (ssize_t) scale_image->columns; x++)
      {
        if (GetPixelWriteMask(scale_image,q) <= (QuantumRange/2))
          {
            q+=(ptrdiff_t) GetPixelChannels(scale_image);
            continue;
          }
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            alpha=QuantumScale*scale_scanline[x*(ssize_t)
              GetPixelChannels(image)+
              GetPixelChannelOffset(image,AlphaPixelChannel)];
            alpha=MagickSafeReciprocal(alpha);
          }
        for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
        {
          PixelChannel channel = GetPixelChannelChannel(image,i);
          PixelTrait traits = GetPixelChannelTraits(image,channel);
          scale_traits=GetPixelChannelTraits(scale_image,channel);
          if ((traits == UndefinedPixelTrait) ||
              (scale_traits == UndefinedPixelTrait))
            continue;
          if ((traits & BlendPixelTrait) == 0)
            {
              SetPixelChannel(scale_image,channel,ClampToQuantum(
                scale_scanline[x*(ssize_t) GetPixelChannels(image)+i]),q);
              continue;
            }
          SetPixelChannel(scale_image,channel,ClampToQuantum(alpha*
            scale_scanline[x*(ssize_t) GetPixelChannels(image)+i]),q);
        }
        q+=(ptrdiff_t) GetPixelChannels(scale_image);
      }
    }
    if (SyncCacheViewAuthenticPixels(scale_view,exception) == MagickFalse)
      {
        status=MagickFalse;
        break;
      }
    proceed=SetImageProgress(image,ScaleImageTag,(MagickOffsetType) y,
      image->rows);
    if (proceed == MagickFalse)
      {
        status=MagickFalse;
        break;
      }
  }
  scale_view=DestroyCacheView(scale_view);
  image_view=DestroyCacheView(image_view);
  /*
    Free allocated memory.
  */
  y_vector=(double *) RelinquishMagickMemory(y_vector);
  scale_scanline=(double *) RelinquishMagickMemory(scale_scanline);
  if (scale_image->rows != image->rows)
    scanline=(double *) RelinquishMagickMemory(scanline);
  x_vector=(double *) RelinquishMagickMemory(x_vector);
  scale_image->type=image->type;
  if (status == MagickFalse)
    scale_image=DestroyImage(scale_image);
  return(scale_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T h u m b n a i l I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ThumbnailImage() changes the size of an image to the given dimensions and
%  removes any associated profiles.  The goal is to produce small low cost
%  thumbnail images suited for display on the Web.
%
%  The format of the ThumbnailImage method is:
%
%      Image *ThumbnailImage(const Image *image,const size_t columns,
%        const size_t rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void url_encode(const char *uri,char *encode_uri)
{
  char
    *p;

  const char
    *hex = "0123456789ABCDEF";

  for (p=encode_uri; *uri != '\0'; uri++)
    if ((('a' <= *uri) && (*uri <= 'z')) || (('A' <= *uri) && (*uri <= 'Z')) ||
        (('0' <= *uri) && (*uri <= '9')) || (strchr("/-_.~",*uri) != 0))
      *p++=(*uri);
    else
      {
        *p++='%';
        *p++=hex[(*uri >> 4) & 0xF];
        *p++=hex[*uri & 0xF];
      }
  *p='\0';
}

MagickExport Image *ThumbnailImage(const Image *image,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
{
#define SampleFactor  5

  char
    encode_uri[3*MagickPathExtent+1] = "/0";

  const char
    *name,
    *mime_type;

  Image
    *thumbnail_image;

  struct stat
    attributes;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  thumbnail_image=CloneImage(image,0,0,MagickTrue,exception);
  if (thumbnail_image == (Image *) NULL)
    return(thumbnail_image);
  if ((columns != image->columns) || (rows != image->rows))
    {
      Image
        *clone_image = thumbnail_image;

      ssize_t
        x_factor,
        y_factor;

      x_factor=(ssize_t) (image->columns*MagickSafeReciprocal((double) 
        columns));
      y_factor=(ssize_t) (image->rows*MagickSafeReciprocal((double) rows));
      if ((x_factor > 4) && (y_factor > 4))
        {
          thumbnail_image=SampleImage(clone_image,4*columns,4*rows,exception);
          if (thumbnail_image != (Image *) NULL)
            {
              clone_image=DestroyImage(clone_image);
              clone_image=thumbnail_image;
            }
        }
      if ((x_factor > 2) && (y_factor > 2))
        {
          thumbnail_image=ResizeImage(clone_image,2*columns,2*rows,BoxFilter,
            exception);
          if (thumbnail_image != (Image *) NULL)
            {
              clone_image=DestroyImage(clone_image);
              clone_image=thumbnail_image;
            }
        }
      thumbnail_image=ResizeImage(clone_image,columns,rows,image->filter ==
        UndefinedFilter ? LanczosSharpFilter : image->filter,exception);
      clone_image=DestroyImage(clone_image);
      if (thumbnail_image == (Image *) NULL)
        return(thumbnail_image);
    }
  (void) ParseAbsoluteGeometry("0x0+0+0",&thumbnail_image->page);
  thumbnail_image->depth=8;
  thumbnail_image->interlace=NoInterlace;
  /*
    Strip all profiles except color profiles.
  */
  ResetImageProfileIterator(thumbnail_image);
  for (name=GetNextImageProfile(thumbnail_image); name != (const char *) NULL; )
  {
    if ((LocaleCompare(name,"icc") != 0) && (LocaleCompare(name,"icm") != 0))
     {
       (void) DeleteImageProfile(thumbnail_image,name);
       ResetImageProfileIterator(thumbnail_image);
     }
    name=GetNextImageProfile(thumbnail_image);
  }
  (void) DeleteImageProperty(thumbnail_image,"comment");
  url_encode(image->filename,encode_uri);
  if (*image->filename != '/')
    (void) FormatImageProperty(thumbnail_image,"Thumb::URI","./%s",encode_uri);
  else
    (void) FormatImageProperty(thumbnail_image,"Thumb::URI","file://%s",
      encode_uri);
  if (GetPathAttributes(image->filename,&attributes) != MagickFalse )
    (void) FormatImageProperty(thumbnail_image,"Thumb::MTime","%.20g",(double)
      attributes.st_mtime);
  (void) FormatImageProperty(thumbnail_image,"Thumb::Size","%.20g",
    (double) GetBlobSize(image));
  mime_type=GetImageProperty(image,"mime:type",exception);
  if (mime_type != (const char *) NULL)
    (void) SetImageProperty(thumbnail_image,"Thumb::Mimetype",mime_type,
      exception);
  (void) SetImageProperty(thumbnail_image,"software",MagickAuthoritativeURL,
    exception);
  (void) FormatImageProperty(thumbnail_image,"Thumb::Image::Width","%.20g",
    (double) image->magick_columns);
  (void) FormatImageProperty(thumbnail_image,"Thumb::Image::Height","%.20g",
    (double) image->magick_rows);
  (void) FormatImageProperty(thumbnail_image,"Thumb::Document::Pages","%.20g",
    (double) GetImageListLength(image));
  return(thumbnail_image);
}

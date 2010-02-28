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
%                                John Cristy                                  %
%                                 July 1992                                   %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/blob.h"
#include "magick/cache.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/memory_.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel.h"
#include "magick/option.h"
#include "magick/resample.h"
#include "magick/resize.h"
#include "magick/resize-private.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/thread-private.h"
#include "magick/utility.h"
#include "magick/version.h"
#if defined(MAGICKCORE_LQR_DELEGATE)
#include <lqr.h>
#endif

/*
  Typedef declarations.
*/
struct _ResizeFilter
{
  MagickRealType
    (*filter)(const MagickRealType,const ResizeFilter *),
    (*window)(const MagickRealType,const ResizeFilter *),
    support,        /* filter region of support - the filter support limit */
    window_support, /* window support, usally equal to support (expert only) */
    scale,          /* dimension to scale to fit window support (usally 1.0) */
    blur,           /* x-scale (blur-sharpen) */
    cubic[8];       /* cubic coefficents for smooth Cubic filters */

  unsigned long
    signature;
};

/*
  Forward declaractions.
*/
static MagickRealType
  I0(MagickRealType x),
  BesselOrderOne(MagickRealType);

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
%     static MagickRealtype *FilterName(const MagickRealType x,
%        const MagickRealType support)
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

static MagickRealType Bessel(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    See Pratt "Digital Image Processing" p.97 for Bessel functions.

    This function actually a X-scaled Jinc(x) function. See
    http://mathworld.wolfram.com/JincFunction.html and page 11 of
    http://www.ph.ed.ac.uk/%7ewjh/teaching/mo/slides/lens/lens.pdf.
  */
  if (x == 0.0)
    return((MagickRealType) (MagickPI/4.0));
  return(BesselOrderOne(MagickPI*x)/(2.0*x));
}

static MagickRealType Blackman(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    Blackman: 2rd Order cosine windowing function.
  */
  return(0.42+0.5*cos(MagickPI*(double) x)+0.08*cos(2.0*MagickPI*(double) x));
}

static MagickRealType Bohman(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    Bohman: 2rd Order cosine windowing function.
  */
  return((1-x)*cos(MagickPI*(double) x)+sin(MagickPI*(double) x)/MagickPI);
}

static MagickRealType Box(const MagickRealType magick_unused(x),
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    Just return 1.0, filter will still be clipped by its support window.
  */
  return(1.0);
}

static MagickRealType CubicBC(const MagickRealType x,
  const ResizeFilter *resize_filter)
{
  /*
    Cubic Filters using B,C determined values:
       Mitchell-Netravali  B=1/3 C=1/3   Qualitively ideal Cubic Filter
       Catmull-Rom         B= 0  C=1/2   Cublic Interpolation Function
       Cubic B-Spline      B= 1  C= 0    Spline Approximation of Gaussian
       Hermite             B= 0  C= 0    Quadratic Spline (support = 1)

    See paper by Mitchell and Netravali, Reconstruction Filters in Computer
    Graphics Computer Graphics, Volume 22, Number 4, August 1988
    http://www.cs.utexas.edu/users/fussell/courses/cs384g/lectures/mitchell/
    Mitchell.pdf.

    Coefficents are determined from B,C values:
       P0 = (  6 - 2*B       )/6
       P1 =         0
       P2 = (-18 +12*B + 6*C )/6
       P3 = ( 12 - 9*B - 6*C )/6
       Q0 = (      8*B +24*C )/6
       Q1 = (    -12*B -48*C )/6
       Q2 = (      6*B +30*C )/6
       Q3 = (    - 1*B - 6*C )/6

    which are used to define the filter:

       P0 + P1*x + P2*x^2 + P3*x^3      0 <= x < 1
       Q0 + Q1*x + Q2*x^2 + Q3*x^3      1 <= x <= 2

    which ensures function is continuous in value and derivative (slope).
  */
  if (x < 1.0)
    return(resize_filter->cubic[0]+x*(resize_filter->cubic[1]+x*
      (resize_filter->cubic[2]+x*resize_filter->cubic[3])));
  if (x < 2.0)
    return(resize_filter->cubic[4] +x*(resize_filter->cubic[5]+x*
      (resize_filter->cubic[6] +x*resize_filter->cubic[7])));
  return(0.0);
}

static MagickRealType Gaussian(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
  return(exp((double) (-2.0*x*x))*sqrt(2.0/MagickPI));
}

static MagickRealType Hanning(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    A Cosine windowing function.
  */
  return(0.5+0.5*cos(MagickPI*(double) x));
}

static MagickRealType Hamming(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    A offset Cosine windowing function.
  */
  return(0.54+0.46*cos(MagickPI*(double) x));
}

static MagickRealType Kaiser(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
#define Alpha  6.5
#define I0A  (1.0/I0(Alpha))

  /*
    Kaiser Windowing Function (bessel windowing): Alpha is a free value from 5
    to 8 (currently hardcoded to 6.5).  Future: make alpha the IOA
    pre-calculation, a 'expert' setting.
  */
  return(I0A*I0(Alpha*sqrt((double) (1.0-x*x))));
}

static MagickRealType Lagrange(const MagickRealType x,
  const ResizeFilter *resize_filter)
{
  long
    n,
    order;

  MagickRealType
    value;

  register long
    i;

  /*
    Lagrange Piece-Wise polynomial fit of Sinc: N is the 'order' of the
    lagrange function and depends on the overall support window size of the
    filter. That is for a support of 2, gives a lagrange-4 or piece-wise cubic
    functions.

    Note that n is the specific piece of the piece-wise function to calculate.

    See Survey: Interpolation Methods, IEEE Transactions on Medical Imaging,
    Vol 18, No 11, November 1999, p1049-1075, -- Equation 27 on p1064.
  */
  if (x > resize_filter->support)
    return(0.0);
  order=(long) (2.0*resize_filter->window_support);  /* number of pieces */
  n=(long) ((1.0*order)/2.0+x);  /* which piece does x belong to */
  value=1.0f;
  for (i=0; i < order; i++)
    if (i != n)
      value*=(n-i-x)/(n-i);
  return(value);
}

static MagickRealType Quadratic(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    2rd order (quadratic) B-Spline approximation of Gaussian.
  */
  if (x < 0.5)
    return(0.75-x*x);
  if (x < 1.5)
    return(0.5*(x-1.5)*(x-1.5));
  return(0.0);
}

static MagickRealType Sinc(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    This function actually a X-scaled Sinc(x) function.
  */
  if (x == 0.0)
    return(1.0);
  return(sin(MagickPI*(double) x)/(MagickPI*(double) x));
}

static MagickRealType Triangle(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    1rd order (linear) B-Spline,  bilinear interpolation, Tent 1D filter, or a
    Bartlett 2D Cone filter.
  */
  if (x < 1.0)
    return(1.0-x);
  return(0.0);
}

static MagickRealType Welsh(const MagickRealType x,
  const ResizeFilter *magick_unused(resize_filter))
{
  /*
    Welsh parabolic windowing filter.
  */
  if (x <  1.0)
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
%      Cubic       Hermite    Catrom
%      Mitchell
%
%  IIR (Infinite impulse Response) Filters
%      Gaussian     Sinc        Bessel
%
%  Windowed Sinc/Bessel Method
%      Blackman     Hanning     Hamming
%      Kaiser       Lancos (Sinc)
%
%  FIR filters are used as is, and are limited by that filters support window
%  (unless over-ridden).  'Gaussian' while classed as an IIR filter, is also
%  simply clipped by its support size (1.5).
%
%  Requesting a windowed filter will return either a windowed Sinc, for a one
%  dimentional orthogonal filtering method, such as ResizeImage(), or a
%  windowed Bessel for image operations requiring a two dimentional
%  cylindrical filtering method, such a DistortImage().  Which function is
%  is used set by the "cylindrical" boolean argument.
%
%  Directly requesting 'Sinc' or 'Bessel' will force the use of that filter
%  function, with a default 'Blackman' windowing method.  This not however
%  recommended as it removes the correct filter selection for different
%  filtering image operations.  Selecting a window filtering method is better.
%
%  Lanczos is purely special case of a Sinc windowed Sinc, but defaulting to
%  a 3 lobe support, rather that the default 4 lobe support.
%
%  Special options can be used to override specific, or all the filter
%  settings.   However doing so is not advisible unless you have expert
%  knowledge of the use of resampling filtered techniques. Extreme caution is
%  advised.
%
%    "filter:filter"    Select this function as the filter.
%        If a "filter:window" operation is not provided, then no windowing
%        will be performed on the selected filter, (support clipped)
%
%        This can be used to force the use of a windowing method as filter,
%        request a 'Sinc' filter in a radially filtered operation, or the
%        'Bessel' filter for a othogonal filtered operation.
%
%    "filter:window"   Select this windowing function for the filter.
%        While any filter could be used as a windowing function,
%        using that filters first lobe over the whole support window,
%        using a non-windowing method is not advisible.
%
%    "filter:lobes"    Number of lobes to use for the Sinc/Bessel filter.
%        This a simper method of setting filter support size that will
%        correctly handle the Sinc/Bessel switch for an operators filtering
%        requirements.
%
%    "filter:support"  Set the support size for filtering to the size given
%        This not recommended for Sinc/Bessel windowed filters, but is
%        used for simple filters like FIR filters, and the Gaussian Filter.
%        This will override any 'filter:lobes' option.
%
%    "filter:blur"     Scale the filter and support window by this amount.
%        A value >1 will generally result in a more burred image with
%        more ringing effects, while a value <1 will sharpen the
%        resulting image with more aliasing and Morie effects.
%
%    "filter:win-support"  Scale windowing function to this size instead.
%        This causes the windowing (or self-windowing Lagrange filter)
%        to act is if the support winodw it much much larger than what
%        is actually supplied to the calling operator.  The filter however
%        is still clipped to the real support size given.  If unset this
%        will equal the normal filter support size.
%
%    "filter:b"
%    "filter:c"    Override the preset B,C values for a Cubic type of filter
%         If only one of these are given it is assumes to be a 'Keys'
%         type of filter such that B+2C=1, where Keys 'alpha' value = C
%
%    "filter:verbose"   Output verbose plotting data for graphing the
%         resulting filter over the whole support range (with blur effect).
%
%  Set a true un-windowed Sinc filter with 10 lobes (very slow)
%     -set option:filter:filter  Sinc
%     -set option:filter:lobes   8
%
%  For example force an 8 lobe Lanczos (Sinc or Bessel) filter...
%     -filter Lanczos
%     -set option:filter:lobes   8
%
%  The format of the AcquireResizeFilter method is:
%
%      ResizeFilter *AcquireResizeFilter(const Image *image,
%        const FilterTypes filter_type, const MagickBooleanType radial,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o filter: the filter type, defining a preset filter, window and support.
%
%    o blur: blur the filter by this amount, use 1.0 if unknown.  Image
%      artifact "filter:blur"  will override this old usage
%
%    o radial: 1D orthogonal filter (Sinc) or 2D radial filter (Bessel)
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ResizeFilter *AcquireResizeFilter(const Image *image,
  const FilterTypes filter,const MagickRealType blur,
  const MagickBooleanType cylindrical,ExceptionInfo *exception)
{
  const char
    *artifact;

  FilterTypes
    filter_type,
    window_type;

  long
    option;

  MagickRealType
    B,
    C;

  register ResizeFilter
    *resize_filter;

  /*
    Table Mapping given Filter, into  Weighting and Windowing functions.  A
    'Box' windowing function means its a simble non-windowed filter.  A 'Sinc'
    filter function (must be windowed) could be upgraded to a 'Bessel' filter
    if a "cylindrical" filter is requested, unless a "Sinc" filter specifically
    request.
  */
  static struct
  {
    FilterTypes
      filter,
      window;
  } const mapping[SentinelFilter] =
  {
    { UndefinedFilter, BoxFilter },  /* undefined */
    { PointFilter,     BoxFilter },  /* special, nearest-neighbour filter */
    { BoxFilter,       BoxFilter },  /* Box averaging Filter */
    { TriangleFilter,  BoxFilter },  /* Linear Interpolation Filter */
    { HermiteFilter,   BoxFilter },      /* Hermite interpolation filter */
    { SincFilter,      HanningFilter },  /* Hanning -- Cosine-Sinc */
    { SincFilter,      HammingFilter },  /* Hamming --  '' variation */
    { SincFilter,      BlackmanFilter }, /* Blackman -- 2*Cosine-Sinc */
    { GaussianFilter,  BoxFilter },      /* Gaussain Blurring filter */
    { QuadraticFilter, BoxFilter },      /* Quadratic Gaussian approximation */
    { CubicFilter,     BoxFilter },      /* Cubic Gaussian approximation */
    { CatromFilter,    BoxFilter },      /* Cubic Interpolator */
    { MitchellFilter,  BoxFilter },      /* 'ideal' Cubic Filter */
    { LanczosFilter,   SincFilter },     /* Special, 3 lobed Sinc-Sinc */
    { BesselFilter,    BlackmanFilter }, /* 3 lobed bessel -specific request */
    { SincFilter,      BlackmanFilter }, /* 4 lobed sinc - specific request */
    { SincFilter,      KaiserFilter },   /* Kaiser --  SqRoot-Sinc */
    { SincFilter,      WelshFilter },    /* Welsh -- Parabolic-Sinc */
    { SincFilter,      CubicFilter },    /* Parzen -- Cubic-Sinc */
    { LagrangeFilter,  BoxFilter },      /* Lagrange self-windowing filter */
    { SincFilter,      BohmanFilter },   /* Bohman -- 2*Cosine-Sinc */
    { SincFilter,      TriangleFilter }  /* Bartlett -- Triangle-Sinc */
  };
  /*
    Table maping the filter/window function from the above table to the actual
    filter/window function call to use.  The default support size for that
    filter as a weighting function, and the point to scale when that function is
    used as a windowing function (typ 1.0).
  */
  static struct
  {
    MagickRealType
      (*function)(const MagickRealType, const ResizeFilter*),
      support,  /* default support size for function as a filter */
      scale,    /* size windowing function, for scaling windowing function */
      B,
      C;        /* Cubic Filter factors for a CubicBC function, else ignored */
  } const filters[SentinelFilter] =
  {
    { Box,       0.0f,  0.5f, 0.0f, 0.0f }, /* Undefined */
    { Box,       0.0f,  0.5f, 0.0f, 0.0f }, /* Point */
    { Box,       0.5f,  0.5f, 0.0f, 0.0f }, /* Box */
    { Triangle,  1.0f,  1.0f, 0.0f, 0.0f }, /* Triangle */
    { CubicBC,   1.0f,  1.0f, 0.0f, 0.0f }, /* Hermite, Cubic B=C=0 */
    { Hanning,   1.0f,  1.0f, 0.0f, 0.0f }, /* Hanning, Cosine window */
    { Hamming,   1.0f,  1.0f, 0.0f, 0.0f }, /* Hamming, '' variation */
    { Blackman,  1.0f,  1.0f, 0.0f, 0.0f }, /* Blackman, 2*cos window */
    { Gaussian,  1.5f,  1.5f, 0.0f, 0.0f }, /* Gaussian */
    { Quadratic, 1.5f,  1.5f, 0.0f, 0.0f }, /* Quadratic Gaussian */
    { CubicBC,   2.0f,  2.0f, 1.0f, 0.0f }, /* B-Spline of Gaussian B=1 C=0 */
    { CubicBC,   2.0f,  1.0f, 0.0f, 0.5f }, /* Catmull-Rom  B=0 C=1/2 */
    { CubicBC,   2.0f,  1.0f, 1.0f/3.0f, 1.0f/3.0f }, /* Mitchel B=C=1/3 */
    { Sinc,      3.0f,  1.0f, 0.0f, 0.0f }, /* Lanczos, 3 lobed Sinc-Sinc */
    { Bessel,    3.2383f,1.2197f,.0f,.0f }, /* 3 lobed Blackman-Bessel */
    { Sinc,      4.0f,  1.0f, 0.0f, 0.0f }, /* 4 lobed Blackman-Sinc   */
    { Kaiser,    1.0f,  1.0f, 0.0f, 0.0f }, /* Kaiser, sq-root windowing */
    { Welsh,     1.0f,  1.0f, 0.0f, 0.0f }, /* Welsh, Parabolic windowing */
    { CubicBC,   2.0f,  2.0f, 1.0f, 0.0f }, /* Parzen, B-Spline windowing */
    { Lagrange,  2.0f,  1.0f, 0.0f, 0.0f }, /* Lagrangian Filter */
    { Bohman,    1.0f,  1.0f, 0.0f, 0.0f }, /* Bohman, 2*Cosine windowing */
    { Triangle,  1.0f,  1.0f, 0.0f, 0.0f }  /* Bartlett, Triangle windowing */
  };
  /*
    The known zero crossings of the Bessel() or the Jinc(x*PI) function found
    by using http://cose.math.bas.bg/webMathematica/webComputing/
    BesselZeros.jsp.  For Jv-function with v=1, divide X-roots by PI (tabled
    below).
  */
  static MagickRealType
    bessel_zeros[16] =
    {
      1.21966989126651f,
      2.23313059438153f,
      3.23831548416624f,
      4.24106286379607f,
      5.24276437687019f,
      6.24392168986449f,
      7.24475986871996f,
      8.24539491395205f,
      9.24589268494948f,
      10.2462933487549f,
      11.2466227948779f,
      12.2468984611381f,
      13.2471325221811f,
      14.2473337358069f,
      15.2475085630373f,
      16.247661874701f
   };

  /*
    Allocate resize filter.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(UndefinedFilter < filter && filter < SentinelFilter);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  resize_filter=(ResizeFilter *) AcquireAlignedMemory(1,sizeof(*resize_filter));
  if (resize_filter == (ResizeFilter *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  /*
    Defaults for the requested filter.
  */
  filter_type=mapping[filter].filter;
  window_type=mapping[filter].window;
  /*
    Filter blur -- scaling both filter and support window.
  */
  resize_filter->blur=blur;
  artifact=GetImageArtifact(image,"filter:blur");
  if (artifact != (const char *) NULL)
    resize_filter->blur=StringToDouble(artifact);
  if (resize_filter->blur < MagickEpsilon)
    resize_filter->blur=(MagickRealType) MagickEpsilon;
  if ((cylindrical != MagickFalse) && (filter != SincFilter))
    switch (filter_type)
    {
      case SincFilter:
      {
        /*
          Promote 1D Sinc Filter to a 2D Bessel filter.
        */
        filter_type=BesselFilter;
        break;
      }
      case LanczosFilter:
      {
        /*
          Promote Lanczos (Sinc-Sinc) to Lanczos (Bessel-Bessel).
        */
        filter_type=BesselFilter;
        window_type=BesselFilter;
        break;
      }
      case GaussianFilter:
      {
        /*
          Gaussian is scaled by 4*ln(2) and not 4*sqrt(2/MagickPI) according to
          Paul Heckbert's paper on EWA resampling.
          FUTURE: to be reviewed.
        */
        resize_filter->blur*=2.0*log(2.0)/sqrt(2.0/MagickPI);
        break;
      }
      case BesselFilter:
      {
        /*
          Filters with a 1.0 zero root crossing by the first bessel zero.
        */
        resize_filter->blur*=bessel_zeros[0];
        break;
      }
      default:
        break;
    }
  artifact=GetImageArtifact(image,"filter:filter");
  if (artifact != (const char *) NULL)
    {
      option=ParseMagickOption(MagickFilterOptions,MagickFalse,
        artifact);
      if ((UndefinedFilter < option) && (option < SentinelFilter))
        {
          /*
            Raw filter request - no window function.
          */
          filter_type=(FilterTypes) option;
          window_type=BoxFilter;
        }
      if (option == LanczosFilter)
        {
          /*
            Lanczos is nor a real filter but a self windowing Sinc/Bessel.
          */
          filter_type=cylindrical != MagickFalse ? BesselFilter : LanczosFilter;
          window_type=cylindrical != MagickFalse ? BesselFilter : SincFilter;
        }
      /*
        Filter overwide with a specific window function.
      */
      artifact=GetImageArtifact(image,"filter:window");
      if (artifact != (const char *) NULL)
        {
          option=ParseMagickOption(MagickFilterOptions,MagickFalse,
            artifact);
          if ((UndefinedFilter < option) && (option < SentinelFilter))
            {
              if (option != LanczosFilter)
                window_type=(FilterTypes) option;
             else
               window_type=cylindrical != MagickFalse ? BesselFilter :
                 SincFilter;
           }
        }
    }
  else
    {
      /*
        Window specified, but no filter function?  Assume Sinc/Bessel.
      */
      artifact=GetImageArtifact(image,"filter:window");
      if (artifact != (const char *) NULL)
        {
          option=ParseMagickOption(MagickFilterOptions,MagickFalse,
            artifact);
          if ((UndefinedFilter < option) && (option < SentinelFilter))
            {
              option=cylindrical != MagickFalse ? BesselFilter : SincFilter;
              if (option != LanczosFilter)
                window_type=(FilterTypes) option;
              else
                window_type=option;
            }
        }
    }
  resize_filter->filter=filters[filter_type].function;
  resize_filter->support=filters[filter_type].support;
  resize_filter->window=filters[window_type].function;
  resize_filter->scale=filters[window_type].scale;
  resize_filter->signature=MagickSignature;
  /*
    Filter support overrides.
  */
  artifact=GetImageArtifact(image,"filter:lobes");
  if (artifact != (const char *) NULL)
    {
      long
        lobes;

      lobes=StringToLong(artifact);
      if (lobes < 1)
        lobes=1;
      resize_filter->support=(MagickRealType) lobes;
      if (filter_type == BesselFilter)
        {
          if (lobes > 16)
            lobes=16;
          resize_filter->support=bessel_zeros[lobes-1];
        }
    }
  artifact=GetImageArtifact(image,"filter:support");
  if (artifact != (const char *) NULL)
    resize_filter->support=fabs(StringToDouble(artifact));
  /*
    Scale windowing function separatally to the support 'clipping' window
    that calling operator is planning to actually use.
  */
  resize_filter->window_support=resize_filter->support;
  artifact=GetImageArtifact(image,"filter:win-support");
  if (artifact != (const char *) NULL)
    resize_filter->window_support=fabs(StringToDouble(artifact));
  /*
    Set Cubic Spline B,C values, calculate Cubic coefficents.
  */
  B=0.0;
  C=0.0;
  if ((filters[filter_type].function == CubicBC) ||
      (filters[window_type].function == CubicBC))
    {
      if (filters[filter_type].function == CubicBC)
        {
          B=filters[filter_type].B;
          C=filters[filter_type].C;
        }
      else
        if (filters[window_type].function == CubicBC)
          {
            B=filters[window_type].B;
            C=filters[window_type].C;
          }
      artifact=GetImageArtifact(image,"filter:b");
      if (artifact != (const char *) NULL)
        {
          B=StringToDouble(artifact);
          C=(1.0-B)/2.0; /* Calculate C as if it is a Keys cubic filter */
          artifact=GetImageArtifact(image,"filter:c");
          if (artifact != (const char *) NULL)
            C=StringToDouble(artifact);
        }
      else
        {
          artifact=GetImageArtifact(image,"filter:c");
          if (artifact != (const char *) NULL)
            {
              C=StringToDouble(artifact);
              B=1.0-2.0*C;  /* Calculate B as if it is a Keys cubic filter */
            }
        }
    /*
      Convert B,C values into Cubic Coefficents.  See CubicBC()
    */
    resize_filter->cubic[0]=(6.0-2.0*B)/6.0;
    resize_filter->cubic[1]=0.0;
    resize_filter->cubic[2]=(-18.0+12.0*B+6.0*C)/6.0;
    resize_filter->cubic[3]=(12.0-9.0*B-6.0*C)/6.0;
    resize_filter->cubic[4]=(8.0*B+24.0*C)/6.0;
    resize_filter->cubic[5]=(-12.0*B-48.0*C)/6.0;
    resize_filter->cubic[6]=(6.0*B+30.0*C)/6.0;
    resize_filter->cubic[7]=(- 1.0*B-6.0*C)/6.0;
  }
  artifact=GetImageArtifact(image,"filter:verbose");
  if (artifact != (const char *) NULL)
    {
      double
        support,
        x;

      /*
        Output filter graph -- for graphing filter result.
      */
      support=GetResizeFilterSupport(resize_filter);
      (void) printf("# support = %lg\n",support);
      for (x=0.0; x <= support; x+=0.01f)
        (void) printf("%5.2lf\t%lf\n",x,(double) GetResizeFilterWeight(
          resize_filter,x));
      (void) printf("%5.2lf\t%lf\n",support,0.0);
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
%  The format of the AdaptiveResizeImage method is:
%
%      Image *AdaptiveResizeImage(const Image *image,
%        const unsigned long columns,const unsigned long rows,
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
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AdaptiveResizeImage(const Image *image,
  const unsigned long columns,const unsigned long rows,ExceptionInfo *exception)
{
#define AdaptiveResizeImageTag  "Resize/Image"

  CacheView
    *resize_view;

  Image
    *resize_image;

  long
    y;

  MagickBooleanType
    proceed;

  MagickPixelPacket
    pixel;

  PointInfo
    offset;

  ResampleFilter
    *resample_filter;

  /*
    Adaptively resize image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((columns == 0) || (rows == 0))
    return((Image *) NULL);
  if ((columns == image->columns) && (rows == image->rows))
    return(CloneImage(image,0,0,MagickTrue,exception));
  resize_image=CloneImage(image,columns,rows,MagickTrue,exception);
  if (resize_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(resize_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&resize_image->exception);
      resize_image=DestroyImage(resize_image);
      return((Image *) NULL);
    }
  GetMagickPixelPacket(image,&pixel);
  resample_filter=AcquireResampleFilter(image,exception);
  if (image->interpolate == UndefinedInterpolatePixel)
    (void) SetResampleFilterInterpolateMethod(resample_filter,
      MeshInterpolatePixel);
  resize_view=AcquireCacheView(resize_image);
  for (y=0; y < (long) resize_image->rows; y++)
  {
    register IndexPacket
      *restrict resize_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    q=QueueCacheViewAuthenticPixels(resize_view,0,y,resize_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      break;
    resize_indexes=GetCacheViewAuthenticIndexQueue(resize_view);
    offset.y=((MagickRealType) y*image->rows/resize_image->rows);
    for (x=0; x < (long) resize_image->columns; x++)
    {
      offset.x=((MagickRealType) x*image->columns/resize_image->columns);
      (void) ResamplePixelColor(resample_filter,offset.x-0.5,offset.y-0.5,
        &pixel);
      SetPixelPacket(resize_image,&pixel,q,resize_indexes+x);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(resize_view,exception) == MagickFalse)
      break;
    proceed=SetImageProgress(image,AdaptiveResizeImageTag,y,image->rows);
    if (proceed == MagickFalse)
      break;
  }
  resample_filter=DestroyResampleFilter(resample_filter);
  resize_view=DestroyCacheView(resize_view);
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
%  order 0:
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
%      MagickRealType BesselOrderOne(MagickRealType x)
%
%  A description of each parameter follows:
%
%    o x: MagickRealType value.
%
*/

#undef I0
static MagickRealType I0(MagickRealType x)
{
  MagickRealType
    sum,
    t,
    y;

  register long
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
    t*=y/((MagickRealType) i*i);
  }
  return(sum);
}

#undef J1
static MagickRealType J1(MagickRealType x)
{
  MagickRealType
    p,
    q;

  register long
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
static MagickRealType P1(MagickRealType x)
{
  MagickRealType
    p,
    q;

  register long
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
static MagickRealType Q1(MagickRealType x)
{
  MagickRealType
    p,
    q;

  register long
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

static MagickRealType BesselOrderOne(MagickRealType x)
{
  MagickRealType
    p,
    q;

  if (x == 0.0)
    return(0.0);
  p=x;
  if (x < 0.0)
    x=(-x);
  if (x < 8.0)
    return(p*J1(x));
  q=sqrt((double) (2.0/(MagickPI*x)))*(P1(x)*(1.0/sqrt(2.0)*(sin((double) x)-
    cos((double) x)))-8.0/x*Q1(x)*(-1.0/sqrt(2.0)*(sin((double) x)+
    cos((double) x))));
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
%  The format of the AcquireResizeFilter method is:
%
%      ResizeFilter *DestroyResizeFilter(ResizeFilter *resize_filter)
%
%  A description of each parameter follows:
%
%    o resize_filter: the resize filter.
%
*/
MagickExport ResizeFilter *DestroyResizeFilter(ResizeFilter *resize_filter)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickSignature);
  resize_filter->signature=(~MagickSignature);
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
%      MagickRealType GetResizeFilterSupport(const ResizeFilter *resize_filter)
%
%  A description of each parameter follows:
%
%    o filter: Image filter to use.
%
*/
MagickExport MagickRealType GetResizeFilterSupport(
  const ResizeFilter *resize_filter)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickSignature);
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
%  which usally lies between zero and the filters current 'support' and
%  returns the weight of the filter function at that point.
%
%  The format of the GetResizeFilterWeight method is:
%
%      MagickRealType GetResizeFilterWeight(const ResizeFilter *resize_filter,
%        const MagickRealType x)
%
%  A description of each parameter follows:
%
%    o filter: the filter type.
%
%    o x: the point.
%
*/
MagickExport MagickRealType GetResizeFilterWeight(
  const ResizeFilter *resize_filter,const MagickRealType x)
{
  MagickRealType
    blur,
    scale;

  /*
    Windowing function - scale the weighting filter by this amount.
  */
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickSignature);
  blur=fabs(x)/resize_filter->blur;  /* X offset with blur scaling */
  if ((resize_filter->window_support < MagickEpsilon) ||
      (resize_filter->window == Box))
    scale=1.0;  /* Point/Box Filter -- avoid division by zero */
  else
    {
      scale=resize_filter->scale/resize_filter->window_support;
      scale=resize_filter->window(blur*scale,resize_filter);
    }
  return(scale*resize_filter->filter(blur,resize_filter));
}

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
%  MagnifyImage() is a convenience method that scales an image proportionally
%  to twice its size.
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
MagickExport Image *MagnifyImage(const Image *image,ExceptionInfo *exception)
{
  Image
    *magnify_image;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  magnify_image=ResizeImage(image,2*image->columns,2*image->rows,CubicFilter,
    1.0,exception);
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
%  MinifyImage() is a convenience method that scales an image proportionally
%  to half its size.
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  minify_image=ResizeImage(image,image->columns/2,image->rows/2,CubicFilter,
    1.0,exception);
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
%        const double y_resolution,const FilterTypes filter,const double blur,
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
%    o blur: the blur factor where > 1 is blurry, < 1 is sharp.
%
*/
MagickExport Image *ResampleImage(const Image *image,const double x_resolution,
  const double y_resolution,const FilterTypes filter,const double blur,
  ExceptionInfo *exception)
{
#define ResampleImageTag  "Resample/Image"

  Image
    *resample_image;

  unsigned long
    height,
    width;

  /*
    Initialize sampled image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=(unsigned long) (x_resolution*image->columns/
    (image->x_resolution == 0.0 ? 72.0 : image->x_resolution)+0.5);
  height=(unsigned long) (y_resolution*image->rows/
    (image->y_resolution == 0.0 ? 72.0 : image->y_resolution)+0.5);
  resample_image=ResizeImage(image,width,height,filter,blur,exception);
  if (resample_image != (Image *) NULL)
    {
      resample_image->x_resolution=x_resolution;
      resample_image->y_resolution=y_resolution;
    }
  return(resample_image);
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
%      Image *LiquidRescaleImage(const Image *image,
%        const unsigned long columns,const unsigned long rows,
%        const double delta_x,const double rigidity,ExceptionInfo *exception)
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
MagickExport Image *LiquidRescaleImage(const Image *image,
  const unsigned long columns,const unsigned long rows,
  const double delta_x,const double rigidity,ExceptionInfo *exception)
{
#define LiquidRescaleImageTag  "Rescale/Image"

  const char
    *map;

  guchar
    *packet;

  Image
    *rescale_image;

  int
    x,
    y;

  LqrCarver
    *carver;

  LqrRetVal
    lqr_status;

  MagickBooleanType
    status;

  MagickPixelPacket
    pixel;

  unsigned char
    *pixels;

  /*
    Liquid rescale image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((columns == 0) || (rows == 0))
    return((Image *) NULL);
  if ((columns == image->columns) && (rows == image->rows))
    return(CloneImage(image,0,0,MagickTrue,exception));
  if ((columns <= 2) || (rows <= 2))
    return(ZoomImage(image,columns,rows,exception));
  if ((columns >= (2*image->columns)) || (rows >= (2*image->rows)))
    {
      Image
        *resize_image;

      unsigned long
        height,
        width;

      /*
        Honor liquid resize size limitations.
      */
      for (width=image->columns; columns >= (2*width-1); width*=2);
      for (height=image->rows; rows >= (2*height-1); height*=2);
      resize_image=ResizeImage(image,width,height,image->filter,image->blur,
        exception);
      if (resize_image == (Image *) NULL)
        return((Image *) NULL);
      rescale_image=LiquidRescaleImage(resize_image,columns,rows,delta_x,
        rigidity,exception);
      resize_image=DestroyImage(resize_image);
      return(rescale_image);
    }
  map="RGB";
  if (image->matte == MagickFalse)
    map="RGBA";
  if (image->colorspace == CMYKColorspace)
    {
      map="CMYK";
      if (image->matte == MagickFalse)
        map="CMYKA";
    }
  pixels=(unsigned char *) AcquireQuantumMemory(image->columns,image->rows*
    strlen(map)*sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    return((Image *) NULL);
  status=ExportImagePixels(image,0,0,image->columns,image->rows,map,CharPixel,
    pixels,exception);
  if (status == MagickFalse)
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  carver=lqr_carver_new(pixels,image->columns,image->rows,strlen(map));
  if (carver == (LqrCarver *) NULL)
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  lqr_status=lqr_carver_init(carver,(int) delta_x,rigidity);
  lqr_status=lqr_carver_resize(carver,columns,rows);
  rescale_image=CloneImage(image,lqr_carver_get_width(carver),
    lqr_carver_get_height(carver),MagickTrue,exception);
  if (rescale_image == (Image *) NULL)
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(rescale_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&rescale_image->exception);
      rescale_image=DestroyImage(rescale_image);
      return((Image *) NULL);
    }
  GetMagickPixelPacket(rescale_image,&pixel);
  (void) lqr_carver_scan_reset(carver);
  while (lqr_carver_scan(carver,&x,&y,&packet) != 0)
  {
    register IndexPacket
      *restrict rescale_indexes;

    register PixelPacket
      *restrict q;

    q=QueueAuthenticPixels(rescale_image,x,y,1,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    rescale_indexes=GetAuthenticIndexQueue(rescale_image);
    pixel.red=QuantumRange*(packet[0]/255.0);
    pixel.green=QuantumRange*(packet[1]/255.0);
    pixel.blue=QuantumRange*(packet[2]/255.0);
    if (image->colorspace != CMYKColorspace)
      {
        if (image->matte == MagickFalse)
          pixel.opacity=QuantumRange*(packet[3]/255.0);
      }
    else
      {
        pixel.index=QuantumRange*(packet[3]/255.0);
        if (image->matte == MagickFalse)
          pixel.opacity=QuantumRange*(packet[4]/255.0);
      }
    SetPixelPacket(rescale_image,&pixel,q,rescale_indexes);
    if (SyncAuthenticPixels(rescale_image,exception) == MagickFalse)
      break;
  }
  /*
    Relinquish resources.
  */
  lqr_carver_destroy(carver);
  return(rescale_image);
}
#else
MagickExport Image *LiquidRescaleImage(const Image *image,
  const unsigned long magick_unused(columns),
  const unsigned long magick_unused(rows),const double magick_unused(delta_x),
  const double magick_unused(rigidity),ExceptionInfo *exception)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","`%s' (LQR)",image->filename);
  return((Image *) NULL);
}
#endif

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
%  filter (see AcquireFilterInfo() ).
%
%  If an undefined filter is given the filter defaults to Mitchell for a
%  colormapped image, a image with a matte channel, or if the image is
%  enlarged.  Otherwise the filter defaults to a Lanczos.
%
%  ResizeImage() was inspired by Paul Heckbert's "zoom" program.
%
%  The format of the ResizeImage method is:
%
%      Image *ResizeImage(Image *image,const unsigned long columns,
%        const unsigned long rows,const FilterTypes filter,const double blur,
%        ExceptionInfo *exception)
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
%    o blur: the blur factor where > 1 is blurry, < 1 is sharp.
%            Typically set this to 1.0.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _ContributionInfo
{
  MagickRealType
    weight;

  long
    pixel;
} ContributionInfo;

static ContributionInfo **DestroyContributionThreadSet(
  ContributionInfo **contribution)
{
  register long
    i;

  assert(contribution != (ContributionInfo **) NULL);
  for (i=0; i < (long) GetOpenMPMaximumThreads(); i++)
    if (contribution[i] != (ContributionInfo *) NULL)
      contribution[i]=(ContributionInfo *) RelinquishMagickMemory(
        contribution[i]);
  contribution=(ContributionInfo **) RelinquishAlignedMemory(contribution);
  return(contribution);
}

static ContributionInfo **AcquireContributionThreadSet(const size_t count)
{
  register long
    i;

  ContributionInfo
    **contribution;

  unsigned long
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  contribution=(ContributionInfo **) AcquireAlignedMemory(number_threads,
    sizeof(*contribution));
  if (contribution == (ContributionInfo **) NULL)
    return((ContributionInfo **) NULL);
  (void) ResetMagickMemory(contribution,0,number_threads*sizeof(*contribution));
  for (i=0; i < (long) number_threads; i++)
  {
    contribution[i]=(ContributionInfo *) AcquireQuantumMemory(count,
      sizeof(**contribution));
    if (contribution[i] == (ContributionInfo *) NULL)
      return(DestroyContributionThreadSet(contribution));
  }
  return(contribution);
}

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}

static MagickBooleanType HorizontalFilter(const ResizeFilter *resize_filter,
  const Image *image,Image *resize_image,const MagickRealType x_factor,
  const MagickSizeType span,MagickOffsetType *quantum,ExceptionInfo *exception)
{
#define ResizeImageTag  "Resize/Image"

  CacheView
    *image_view,
    *resize_view;

  ClassType
    storage_class;

  ContributionInfo
    **restrict contributions;

  long
    x;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  MagickRealType
    scale,
    support;

  /*
    Apply filter to resize horizontally from image to resize image.
  */
  scale=MagickMax(1.0/x_factor,1.0);
  support=scale*GetResizeFilterSupport(resize_filter);
  storage_class=support > 0.5 ? DirectClass : image->storage_class;
  if (SetImageStorageClass(resize_image,storage_class) == MagickFalse)
    {
      InheritException(exception,&resize_image->exception);
      return(MagickFalse);
    }
  if (support < 0.5)
    {
      /*
        Support too small even for nearest neighbour:  reduce to point sampling.
      */
      support=(MagickRealType) 0.5;
      scale=1.0;
    }
  contributions=AcquireContributionThreadSet((size_t) (2.0*support+3.0));
  if (contributions == (ContributionInfo **) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  status=MagickTrue;
  scale=1.0/scale;
  (void) ResetMagickMemory(&zero,0,sizeof(zero));
  image_view=AcquireCacheView(image);
  resize_view=AcquireCacheView(resize_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for shared(status)
#endif
  for (x=0; x < (long) resize_image->columns; x++)
  {
    long
      n,
      start,
      stop;

    MagickRealType
      center,
      density;

    register ContributionInfo
      *restrict contribution;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict resize_indexes;

    register long
      y;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    center=(MagickRealType) (x+0.5)/x_factor;
    start=(long) (MagickMax(center-support-MagickEpsilon,0.0)+0.5);
    stop=(long) (MagickMin(center+support,(double) image->columns)+0.5);
    density=0.0;
    contribution=contributions[GetOpenMPThreadId()];
    for (n=0; n < (stop-start); n++)
    {
      contribution[n].pixel=start+n;
      contribution[n].weight=GetResizeFilterWeight(resize_filter,scale*
        ((MagickRealType) (start+n)-center+0.5));
      density+=contribution[n].weight;
    }
    if ((density != 0.0) && (density != 1.0))
      {
        register long
          i;

        /*
          Normalize.
        */
        density=1.0/density;
        for (i=0; i < n; i++)
          contribution[i].weight*=density;
      }
    p=GetCacheViewVirtualPixels(image_view,contribution[0].pixel,0,
      (unsigned long) (contribution[n-1].pixel-contribution[0].pixel+1),
      image->rows,exception);
    q=QueueCacheViewAuthenticPixels(resize_view,x,0,1,resize_image->rows,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    resize_indexes=GetCacheViewAuthenticIndexQueue(resize_view);
    for (y=0; y < (long) resize_image->rows; y++)
    {
      long
        j;

      MagickPixelPacket
        pixel;

      MagickRealType
        alpha;

      register long
        i;

      pixel=zero;
      if (image->matte == MagickFalse)
        {
          for (i=0; i < n; i++)
          {
            j=y*(contribution[n-1].pixel-contribution[0].pixel+1)+
              (contribution[i].pixel-contribution[0].pixel);
            alpha=contribution[i].weight;
            pixel.red+=alpha*(p+j)->red;
            pixel.green+=alpha*(p+j)->green;
            pixel.blue+=alpha*(p+j)->blue;
            pixel.opacity+=alpha*(p+j)->opacity;
          }
          SetRedPixelComponent(q,ClampRedPixelComponent(&pixel));
          SetGreenPixelComponent(q,ClampGreenPixelComponent(&pixel));
          SetBluePixelComponent(q,ClampBluePixelComponent(&pixel));
          SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
          if ((image->colorspace == CMYKColorspace) &&
              (resize_image->colorspace == CMYKColorspace))
            {
              for (i=0; i < n; i++)
              {
                j=y*(contribution[n-1].pixel-contribution[0].pixel+1)+
                  (contribution[i].pixel-contribution[0].pixel);
                alpha=contribution[i].weight;
                pixel.index+=alpha*indexes[j];
              }
              resize_indexes[y]=(IndexPacket) ClampToQuantum(pixel.index);
            }
        }
      else
        {
          MagickRealType
            gamma;

          gamma=0.0;
          for (i=0; i < n; i++)
          {
            j=y*(contribution[n-1].pixel-contribution[0].pixel+1)+
              (contribution[i].pixel-contribution[0].pixel);
            alpha=contribution[i].weight*QuantumScale*
              GetAlphaPixelComponent(p+j);
            pixel.red+=alpha*(p+j)->red;
            pixel.green+=alpha*(p+j)->green;
            pixel.blue+=alpha*(p+j)->blue;
            pixel.opacity+=contribution[i].weight*(p+j)->opacity;
            gamma+=alpha;
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          q->red=ClampToQuantum(gamma*GetRedPixelComponent(&pixel));
          q->green=ClampToQuantum(gamma*GetGreenPixelComponent(&pixel));
          q->blue=ClampToQuantum(gamma*GetBluePixelComponent(&pixel));
          SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
          if ((image->colorspace == CMYKColorspace) &&
              (resize_image->colorspace == CMYKColorspace))
            {
              for (i=0; i < n; i++)
              {
                j=y*(contribution[n-1].pixel-contribution[0].pixel+1)+
                  (contribution[i].pixel-contribution[0].pixel);
                alpha=contribution[i].weight*QuantumScale*
                  GetAlphaPixelComponent(p+j);
                pixel.index+=alpha*indexes[j];
              }
              resize_indexes[y]=(IndexPacket) ClampToQuantum(gamma*
               GetIndexPixelComponent(&pixel));
            }
        }
      if ((resize_image->storage_class == PseudoClass) &&
          (image->storage_class == PseudoClass))
        {
          i=(long) (MagickMin(MagickMax(center,(double) start),(double) stop-
            1.0)+0.5);
          j=y*(contribution[n-1].pixel-contribution[0].pixel+1)+
            (contribution[i-start].pixel-contribution[0].pixel);
          resize_indexes[y]=indexes[j];
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(resize_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_HorizontalFilter)
#endif
        proceed=SetImageProgress(image,ResizeImageTag,(*quantum)++,span);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  resize_view=DestroyCacheView(resize_view);
  image_view=DestroyCacheView(image_view);
  contributions=DestroyContributionThreadSet(contributions);
  return(status);
}

static MagickBooleanType VerticalFilter(const ResizeFilter *resize_filter,
  const Image *image,Image *resize_image,const MagickRealType y_factor,
  const MagickSizeType span,MagickOffsetType *quantum,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *resize_view;

  ClassType
    storage_class;

  ContributionInfo
    **restrict contributions;

  long
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  MagickRealType
    scale,
    support;

  /*
    Apply filter to resize vertically from image to resize_image.
  */
  scale=MagickMax(1.0/y_factor,1.0);
  support=scale*GetResizeFilterSupport(resize_filter);
  storage_class=support > 0.5 ? DirectClass : image->storage_class;
  if (SetImageStorageClass(resize_image,storage_class) == MagickFalse)
    {
      InheritException(exception,&resize_image->exception);
      return(MagickFalse);
    }
  if (support < 0.5)
    {
      /*
        Support too small even for nearest neighbour:  reduce to point sampling.
      */
      support=(MagickRealType) 0.5;
      scale=1.0;
    }
  contributions=AcquireContributionThreadSet((size_t) (2.0*support+3.0));
  if (contributions == (ContributionInfo **) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  status=MagickTrue;
  scale=1.0/scale;
  (void) ResetMagickMemory(&zero,0,sizeof(zero));
  image_view=AcquireCacheView(image);
  resize_view=AcquireCacheView(resize_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for shared(status)
#endif
  for (y=0; y < (long) resize_image->rows; y++)
  {
    long
      n,
      start,
      stop;

    MagickRealType
      center,
      density;

    register ContributionInfo
      *restrict contribution;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict resize_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    center=(MagickRealType) (y+0.5)/y_factor;
    start=(long) (MagickMax(center-support-MagickEpsilon,0.0)+0.5);
    stop=(long) (MagickMin(center+support,(double) image->rows)+0.5);
    density=0.0;
    contribution=contributions[GetOpenMPThreadId()];
    for (n=0; n < (stop-start); n++)
    {
      contribution[n].pixel=start+n;
      contribution[n].weight=GetResizeFilterWeight(resize_filter,scale*
        ((MagickRealType) (start+n)-center+0.5));
      density+=contribution[n].weight;
    }
    if ((density != 0.0) && (density != 1.0))
      {
        register long
          i;

        /*
          Normalize.
        */
        density=1.0/density;
        for (i=0; i < n; i++)
          contribution[i].weight*=density;
      }
    p=GetCacheViewVirtualPixels(image_view,0,contribution[0].pixel,
      image->columns,(unsigned long) (contribution[n-1].pixel-
      contribution[0].pixel+1),exception);
    q=QueueCacheViewAuthenticPixels(resize_view,0,y,resize_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    resize_indexes=GetCacheViewAuthenticIndexQueue(resize_view);
    for (x=0; x < (long) resize_image->columns; x++)
    {
      long
        j;

      MagickPixelPacket
        pixel;

      MagickRealType
        alpha;

      register long
        i;

      pixel=zero;
      if (image->matte == MagickFalse)
        {
          for (i=0; i < n; i++)
          {
            j=(long) ((contribution[i].pixel-contribution[0].pixel)*
              image->columns+x);
            alpha=contribution[i].weight;
            pixel.red+=alpha*(p+j)->red;
            pixel.green+=alpha*(p+j)->green;
            pixel.blue+=alpha*(p+j)->blue;
            pixel.opacity+=alpha*(p+j)->opacity;
          }
          SetRedPixelComponent(q,ClampRedPixelComponent(&pixel));
          SetGreenPixelComponent(q,ClampGreenPixelComponent(&pixel));
          SetBluePixelComponent(q,ClampBluePixelComponent(&pixel));
          SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
          if ((image->colorspace == CMYKColorspace) &&
              (resize_image->colorspace == CMYKColorspace))
            {
              for (i=0; i < n; i++)
              {
                j=(long) ((contribution[i].pixel-contribution[0].pixel)*
                  image->columns+x);
                alpha=contribution[i].weight;
                pixel.index+=alpha*indexes[j];
              }
              resize_indexes[x]=(IndexPacket) ClampToQuantum(pixel.index);
            }
        }
      else
        {
          MagickRealType
            gamma;

          gamma=0.0;
          for (i=0; i < n; i++)
          {
            j=(long) ((contribution[i].pixel-contribution[0].pixel)*
              image->columns+x);
            alpha=contribution[i].weight*QuantumScale*
              GetAlphaPixelComponent(p+j);
            pixel.red+=alpha*(p+j)->red;
            pixel.green+=alpha*(p+j)->green;
            pixel.blue+=alpha*(p+j)->blue;
            pixel.opacity+=contribution[i].weight*(p+j)->opacity;
            gamma+=alpha;
          }
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          q->red=ClampToQuantum(gamma*GetRedPixelComponent(&pixel));
          q->green=ClampToQuantum(gamma*GetGreenPixelComponent(&pixel));
          q->blue=ClampToQuantum(gamma*GetBluePixelComponent(&pixel));
          SetOpacityPixelComponent(q,ClampOpacityPixelComponent(&pixel));
          if ((image->colorspace == CMYKColorspace) &&
              (resize_image->colorspace == CMYKColorspace))
            {
              for (i=0; i < n; i++)
              {
                j=(long) ((contribution[i].pixel-contribution[0].pixel)*
                  image->columns+x);
                alpha=contribution[i].weight*QuantumScale*
                  GetAlphaPixelComponent(p+j);
                pixel.index+=alpha*indexes[j];
              }
              resize_indexes[x]=(IndexPacket) ClampToQuantum(gamma*
                GetIndexPixelComponent(&pixel));
            }
        }
      if ((resize_image->storage_class == PseudoClass) &&
          (image->storage_class == PseudoClass))
        {
          i=(long) (MagickMin(MagickMax(center,(double) start),(double) stop-
            1.0)+0.5);
          j=(long) ((contribution[i-start].pixel-contribution[0].pixel)*
            image->columns+x);
          resize_indexes[x]=indexes[j];
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(resize_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_VerticalFilter)
#endif
        proceed=SetImageProgress(image,ResizeImageTag,(*quantum)++,span);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  resize_view=DestroyCacheView(resize_view);
  image_view=DestroyCacheView(image_view);
  contributions=DestroyContributionThreadSet(contributions);
  return(status);
}

MagickExport Image *ResizeImage(const Image *image,const unsigned long columns,
  const unsigned long rows,const FilterTypes filter,const double blur,
  ExceptionInfo *exception)
{
#define WorkLoadFactor  0.265

  FilterTypes
    filter_type;

  Image
    *filter_image,
    *resize_image;

  MagickRealType
    x_factor,
    y_factor;

  MagickSizeType
    span;

  MagickStatusType
    status;

  ResizeFilter
    *resize_filter;

  MagickOffsetType
    quantum;

  /*
    Acquire resize image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((columns == 0) || (rows == 0))
    ThrowImageException(ImageError,"NegativeOrZeroImageSize");
  if ((columns == image->columns) && (rows == image->rows) &&
      (filter == UndefinedFilter) && (blur == 1.0))
    return(CloneImage(image,0,0,MagickTrue,exception));
  resize_image=CloneImage(image,columns,rows,MagickTrue,exception);
  if (resize_image == (Image *) NULL)
    return(resize_image);
  /*
    Acquire resize filter.
  */
  x_factor=(MagickRealType) columns/(MagickRealType) image->columns;
  y_factor=(MagickRealType) rows/(MagickRealType) image->rows;
  if ((x_factor*y_factor) > WorkLoadFactor)
    filter_image=CloneImage(image,columns,image->rows,MagickTrue,exception);
  else
    filter_image=CloneImage(image,image->columns,rows,MagickTrue,exception);
  if (filter_image == (Image *) NULL)
    return(DestroyImage(resize_image));
  filter_type=LanczosFilter;
  if (filter != UndefinedFilter)
    filter_type=filter;
  else
    if ((x_factor == 1.0) && (y_factor == 1.0))
      filter_type=PointFilter;
    else
      if ((image->storage_class == PseudoClass) ||
          (image->matte != MagickFalse) || ((x_factor*y_factor) > 1.0))
        filter_type=MitchellFilter;
  resize_filter=AcquireResizeFilter(image,filter_type,blur,MagickFalse,
    exception);
  /*
    Resize image.
  */
  quantum=0;
  if ((x_factor*y_factor) > WorkLoadFactor)
    {
      span=(MagickSizeType) (filter_image->columns+rows);
      status=HorizontalFilter(resize_filter,image,filter_image,x_factor,span,
        &quantum,exception);
      status&=VerticalFilter(resize_filter,filter_image,resize_image,y_factor,
        span,&quantum,exception);
    }
  else
    {
      span=(MagickSizeType) (filter_image->rows+columns);
      status=VerticalFilter(resize_filter,image,filter_image,y_factor,span,
        &quantum,exception);
      status&=HorizontalFilter(resize_filter,filter_image,resize_image,x_factor,
        span,&quantum,exception);
    }
  /*
    Free resources.
  */
  filter_image=DestroyImage(filter_image);
  resize_filter=DestroyResizeFilter(resize_filter);
  if ((status == MagickFalse) || (resize_image == (Image *) NULL))
    return((Image *) NULL);
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
%      Image *SampleImage(const Image *image,const unsigned long columns,
%        const unsigned long rows,ExceptionInfo *exception)
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
MagickExport Image *SampleImage(const Image *image,const unsigned long columns,
  const unsigned long rows,ExceptionInfo *exception)
{
#define SampleImageTag  "Sample/Image"

  CacheView
    *image_view,
    *sample_view;

  Image
    *sample_image;

  long
    progress,
    *x_offset,
    y;

  MagickBooleanType
    status;

  register long
    x;

  /*
    Initialize sampled image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((columns == 0) || (rows == 0))
    ThrowImageException(ImageError,"NegativeOrZeroImageSize");
  if ((columns == image->columns) && (rows == image->rows))
    return(CloneImage(image,0,0,MagickTrue,exception));
  sample_image=CloneImage(image,columns,rows,MagickTrue,exception);
  if (sample_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Allocate scan line buffer and column offset buffers.
  */
  x_offset=(long *) AcquireQuantumMemory((size_t) sample_image->columns,
    sizeof(*x_offset));
  if (x_offset == (long *) NULL)
    {
      sample_image=DestroyImage(sample_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  for (x=0; x < (long) sample_image->columns; x++)
    x_offset[x]=(long) (((MagickRealType) x+0.5)*image->columns/
      sample_image->columns);
  /*
    Sample each row.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  sample_view=AcquireCacheView(sample_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) sample_image->rows; y++)
  {
    long
      y_offset;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict sample_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    y_offset=(long) (((MagickRealType) y+0.5)*image->rows/sample_image->rows);
    p=GetCacheViewVirtualPixels(image_view,0,y_offset,image->columns,1,
      exception);
    q=QueueCacheViewAuthenticPixels(sample_view,0,y,sample_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    sample_indexes=GetCacheViewAuthenticIndexQueue(sample_view);
    /*
      Sample each column.
    */
    for (x=0; x < (long) sample_image->columns; x++)
      *q++=p[x_offset[x]];
    if ((image->storage_class == PseudoClass) ||
        (image->colorspace == CMYKColorspace))
      for (x=0; x < (long) sample_image->columns; x++)
        sample_indexes[x]=indexes[x_offset[x]];
    if (SyncCacheViewAuthenticPixels(sample_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_SampleImage)
#endif
        proceed=SetImageProgress(image,SampleImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  sample_view=DestroyCacheView(sample_view);
  x_offset=(long *) RelinquishMagickMemory(x_offset);
  sample_image->type=image->type;
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
%      Image *ScaleImage(const Image *image,const unsigned long columns,
%        const unsigned long rows,ExceptionInfo *exception)
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
MagickExport Image *ScaleImage(const Image *image,const unsigned long columns,
  const unsigned long rows,ExceptionInfo *exception)
{
#define ScaleImageTag  "Scale/Image"

  CacheView
    *image_view,
    *scale_view;

  Image
    *scale_image;

  long
    number_rows,
    y;

  MagickBooleanType
    next_column,
    next_row,
    proceed;

  MagickPixelPacket
    pixel,
    *scale_scanline,
    *scanline,
    *x_vector,
    *y_vector,
    zero;

  PointInfo
    scale,
    span;

  register long
    i;

  /*
    Initialize scaled image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((columns == 0) || (rows == 0))
    return((Image *) NULL);
  if ((columns == image->columns) && (rows == image->rows))
    return(CloneImage(image,0,0,MagickTrue,exception));
  scale_image=CloneImage(image,columns,rows,MagickTrue,exception);
  if (scale_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(scale_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&scale_image->exception);
      scale_image=DestroyImage(scale_image);
      return((Image *) NULL);
    }
  /*
    Allocate memory.
  */
  x_vector=(MagickPixelPacket *) AcquireQuantumMemory((size_t) image->columns,
    sizeof(*x_vector));
  scanline=x_vector;
  if (image->rows != scale_image->rows)
    scanline=(MagickPixelPacket *) AcquireQuantumMemory((size_t) image->columns,
      sizeof(*scanline));
  scale_scanline=(MagickPixelPacket *) AcquireQuantumMemory((size_t)
    scale_image->columns,sizeof(*scale_scanline));
  y_vector=(MagickPixelPacket *) AcquireQuantumMemory((size_t) image->columns,
    sizeof(*y_vector));
  if ((scanline == (MagickPixelPacket *) NULL) ||
      (scale_scanline == (MagickPixelPacket *) NULL) ||
      (x_vector == (MagickPixelPacket *) NULL) ||
      (y_vector == (MagickPixelPacket *) NULL))
    {
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
  (void) ResetMagickMemory(y_vector,0,(size_t) image->columns*
    sizeof(*y_vector));
  GetMagickPixelPacket(image,&pixel);
  (void) ResetMagickMemory(&zero,0,sizeof(zero));
  i=0;
  image_view=AcquireCacheView(image);
  scale_view=AcquireCacheView(scale_image);
  for (y=0; y < (long) scale_image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict scale_indexes;

    register long
      x;

    register MagickPixelPacket
      *restrict s,
      *restrict t;

    register PixelPacket
      *restrict q;

    q=QueueCacheViewAuthenticPixels(scale_view,0,y,scale_image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      break;
    scale_indexes=GetAuthenticIndexQueue(scale_image);
    if (scale_image->rows == image->rows)
      {
        /*
          Read a new scanline.
        */
        p=GetCacheViewVirtualPixels(image_view,0,i++,image->columns,1,
          exception);
        if (p == (const PixelPacket *) NULL)
          break;
        indexes=GetCacheViewVirtualIndexQueue(image_view);
        for (x=0; x < (long) image->columns; x++)
        {
          x_vector[x].red=(MagickRealType) GetRedPixelComponent(p);
          x_vector[x].green=(MagickRealType) GetGreenPixelComponent(p);
          x_vector[x].blue=(MagickRealType) GetBluePixelComponent(p);
          if (image->matte != MagickFalse)
            x_vector[x].opacity=(MagickRealType) GetOpacityPixelComponent(p);
          if (indexes != (IndexPacket *) NULL)
            x_vector[x].index=(MagickRealType) indexes[x];
          p++;
        }
      }
    else
      {
        /*
          Scale Y direction.
        */
        while (scale.y < span.y)
        {
          if ((next_row != MagickFalse) && (number_rows < (long) image->rows))
            {
              /*
                Read a new scanline.
              */
              p=GetCacheViewVirtualPixels(image_view,0,i++,image->columns,1,
                exception);
              if (p == (const PixelPacket *) NULL)
                break;
              indexes=GetCacheViewVirtualIndexQueue(image_view);
              for (x=0; x < (long) image->columns; x++)
              {
                x_vector[x].red=(MagickRealType) GetRedPixelComponent(p);
                x_vector[x].green=(MagickRealType) GetGreenPixelComponent(p);
                x_vector[x].blue=(MagickRealType) GetBluePixelComponent(p);
                if (image->matte != MagickFalse)
                  x_vector[x].opacity=(MagickRealType)
                   GetOpacityPixelComponent(p);
                if (indexes != (IndexPacket *) NULL)
                  x_vector[x].index=(MagickRealType) indexes[x];
                p++;
              }
              number_rows++;
            }
          for (x=0; x < (long) image->columns; x++)
          {
            y_vector[x].red+=scale.y*x_vector[x].red;
            y_vector[x].green+=scale.y*x_vector[x].green;
            y_vector[x].blue+=scale.y*x_vector[x].blue;
            if (scale_image->matte != MagickFalse)
              y_vector[x].opacity+=scale.y*x_vector[x].opacity;
            if (scale_indexes != (IndexPacket *) NULL)
              y_vector[x].index+=scale.y*x_vector[x].index;
          }
          span.y-=scale.y;
          scale.y=(double) scale_image->rows/(double) image->rows;
          next_row=MagickTrue;
        }
        if ((next_row != MagickFalse) && (number_rows < (long) image->rows))
          {
            /*
              Read a new scanline.
            */
            p=GetCacheViewVirtualPixels(image_view,0,i++,image->columns,1,
              exception);
            if (p == (const PixelPacket *) NULL)
              break;
            indexes=GetCacheViewVirtualIndexQueue(image_view);
            for (x=0; x < (long) image->columns; x++)
            {
              x_vector[x].red=(MagickRealType) GetRedPixelComponent(p);
              x_vector[x].green=(MagickRealType) GetGreenPixelComponent(p);
              x_vector[x].blue=(MagickRealType) GetBluePixelComponent(p);
              if (image->matte != MagickFalse)
                x_vector[x].opacity=(MagickRealType)
                  GetOpacityPixelComponent(p);
              if (indexes != (IndexPacket *) NULL)
                x_vector[x].index=(MagickRealType) indexes[x];
              p++;
            }
            number_rows++;
            next_row=MagickFalse;
          }
        s=scanline;
        for (x=0; x < (long) image->columns; x++)
        {
          pixel.red=y_vector[x].red+span.y*x_vector[x].red;
          pixel.green=y_vector[x].green+span.y*x_vector[x].green;
          pixel.blue=y_vector[x].blue+span.y*x_vector[x].blue;
          if (image->matte != MagickFalse)
            pixel.opacity=y_vector[x].opacity+span.y*x_vector[x].opacity;
          if (scale_indexes != (IndexPacket *) NULL)
            pixel.index=y_vector[x].index+span.y*x_vector[x].index;
          s->red=pixel.red;
          s->green=pixel.green;
          s->blue=pixel.blue;
          if (scale_image->matte != MagickFalse)
            s->opacity=pixel.opacity;
          if (scale_indexes != (IndexPacket *) NULL)
            s->index=pixel.index;
          s++;
          y_vector[x]=zero;
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
        s=scanline;
        for (x=0; x < (long) scale_image->columns; x++)
        {
          q->red=ClampToQuantum(s->red);
          q->green=ClampToQuantum(s->green);
          q->blue=ClampToQuantum(s->blue);
          if (scale_image->matte != MagickFalse)
            q->opacity=ClampToQuantum(s->opacity);
          if (scale_indexes != (IndexPacket *) NULL)
            scale_indexes[x]=(IndexPacket) ClampToQuantum(s->index);
          q++;
          s++;
        }
      }
    else
      {
        /*
          Scale X direction.
        */
        pixel=zero;
        next_column=MagickFalse;
        span.x=1.0;
        s=scanline;
        t=scale_scanline;
        for (x=0; x < (long) image->columns; x++)
        {
          scale.x=(double) scale_image->columns/(double) image->columns;
          while (scale.x >= span.x)
          {
            if (next_column != MagickFalse)
              {
                pixel=zero;
                t++;
              }
            pixel.red+=span.x*s->red;
            pixel.green+=span.x*s->green;
            pixel.blue+=span.x*s->blue;
            if (image->matte != MagickFalse)
              pixel.opacity+=span.x*s->opacity;
            if (scale_indexes != (IndexPacket *) NULL)
              pixel.index+=span.x*s->index;
            t->red=pixel.red;
            t->green=pixel.green;
            t->blue=pixel.blue;
            if (scale_image->matte != MagickFalse)
              t->opacity=pixel.opacity;
            if (scale_indexes != (IndexPacket *) NULL)
              t->index=pixel.index;
            scale.x-=span.x;
            span.x=1.0;
            next_column=MagickTrue;
          }
        if (scale.x > 0)
          {
            if (next_column != MagickFalse)
              {
                pixel=zero;
                next_column=MagickFalse;
                t++;
              }
            pixel.red+=scale.x*s->red;
            pixel.green+=scale.x*s->green;
            pixel.blue+=scale.x*s->blue;
            if (scale_image->matte != MagickFalse)
              pixel.opacity+=scale.x*s->opacity;
            if (scale_indexes != (IndexPacket *) NULL)
              pixel.index+=scale.x*s->index;
            span.x-=scale.x;
          }
        s++;
      }
      if (span.x > 0)
        {
          s--;
          pixel.red+=span.x*s->red;
          pixel.green+=span.x*s->green;
          pixel.blue+=span.x*s->blue;
          if (scale_image->matte != MagickFalse)
            pixel.opacity+=span.x*s->opacity;
          if (scale_indexes != (IndexPacket *) NULL)
            pixel.index+=span.x*s->index;
        }
      if ((next_column == MagickFalse) &&
          ((long) (t-scale_scanline) < (long) scale_image->columns))
        {
          t->red=pixel.red;
          t->green=pixel.green;
          t->blue=pixel.blue;
          if (scale_image->matte != MagickFalse)
            t->opacity=pixel.opacity;
          if (scale_indexes != (IndexPacket *) NULL)
            t->index=pixel.index;
        }
      /*
        Transfer scanline to scaled image.
      */
      t=scale_scanline;
      for (x=0; x < (long) scale_image->columns; x++)
      {
        q->red=ClampToQuantum(t->red);
        q->green=ClampToQuantum(t->green);
        q->blue=ClampToQuantum(t->blue);
        if (scale_image->matte != MagickFalse)
          q->opacity=ClampToQuantum(t->opacity);
        if (scale_indexes != (IndexPacket *) NULL)
          scale_indexes[x]=(IndexPacket) ClampToQuantum(t->index);
        t++;
        q++;
      }
    }
    if (SyncCacheViewAuthenticPixels(scale_view,exception) == MagickFalse)
      break;
    proceed=SetImageProgress(image,ScaleImageTag,y,image->rows);
    if (proceed == MagickFalse)
      break;
  }
  scale_view=DestroyCacheView(scale_view);
  image_view=DestroyCacheView(image_view);
  /*
    Free allocated memory.
  */
  y_vector=(MagickPixelPacket *) RelinquishMagickMemory(y_vector);
  scale_scanline=(MagickPixelPacket *) RelinquishMagickMemory(scale_scanline);
  if (scale_image->rows != image->rows)
    scanline=(MagickPixelPacket *) RelinquishMagickMemory(scanline);
  x_vector=(MagickPixelPacket *) RelinquishMagickMemory(x_vector);
  scale_image->type=image->type;
  return(scale_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t R e s i z e F i l t e r S u p p o r t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetResizeFilterSupport() specifies which IR filter to use to window
%
%  The format of the SetResizeFilterSupport method is:
%
%      void SetResizeFilterSupport(ResizeFilter *resize_filter,
%        const MagickRealType support)
%
%  A description of each parameter follows:
%
%    o resize_filter: the resize filter.
%
%    o support: the filter spport radius.
%
*/
MagickExport void SetResizeFilterSupport(ResizeFilter *resize_filter,
  const MagickRealType support)
{
  assert(resize_filter != (ResizeFilter *) NULL);
  assert(resize_filter->signature == MagickSignature);
  resize_filter->support=support;
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
%      Image *ThumbnailImage(const Image *image,const unsigned long columns,
%        const unsigned long rows,ExceptionInfo *exception)
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
MagickExport Image *ThumbnailImage(const Image *image,
  const unsigned long columns,const unsigned long rows,ExceptionInfo *exception)
{
#define SampleFactor  5

  char
    value[MaxTextExtent];

  const char
    *name;

  Image
    *thumbnail_image;

  MagickRealType
    x_factor,
    y_factor;

  struct stat
    attributes;

  unsigned long
    version;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  x_factor=(MagickRealType) columns/(MagickRealType) image->columns;
  y_factor=(MagickRealType) rows/(MagickRealType) image->rows;
  if ((x_factor*y_factor) > 0.1)
    thumbnail_image=ZoomImage(image,columns,rows,exception);
  else
    if (((SampleFactor*columns) < 128) || ((SampleFactor*rows) < 128))
      thumbnail_image=ZoomImage(image,columns,rows,exception);
    else
      {
        Image
          *sample_image;

        sample_image=SampleImage(image,SampleFactor*columns,SampleFactor*rows,
          exception);
        if (sample_image == (Image *) NULL)
          return((Image *) NULL);
        thumbnail_image=ZoomImage(sample_image,columns,rows,exception);
        sample_image=DestroyImage(sample_image);
      }
  if (thumbnail_image == (Image *) NULL)
    return(thumbnail_image);
  (void) ParseAbsoluteGeometry("0x0+0+0",&thumbnail_image->page);
  if (thumbnail_image->matte == MagickFalse)
    (void) SetImageAlphaChannel(thumbnail_image,OpaqueAlphaChannel);
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
  (void) CopyMagickString(value,image->magick_filename,MaxTextExtent);
  if (strstr(image->magick_filename,"//") == (char *) NULL)
    (void) FormatMagickString(value,MaxTextExtent,"file://%s",
      image->magick_filename);
  (void) SetImageProperty(thumbnail_image,"Thumb::URI",value);
  (void) CopyMagickString(value,image->magick_filename,MaxTextExtent);
  if (GetPathAttributes(image->filename,&attributes) != MagickFalse)
    {
      (void) FormatMagickString(value,MaxTextExtent,"%ld",(long)
        attributes.st_mtime);
      (void) SetImageProperty(thumbnail_image,"Thumb::MTime",value);
    }
  (void) FormatMagickString(value,MaxTextExtent,"%ld",(long)
    attributes.st_mtime);
  (void) FormatMagickSize(GetBlobSize(image),MagickFalse,value);
  (void) SetImageProperty(thumbnail_image,"Thumb::Size",value);
  (void) FormatMagickString(value,MaxTextExtent,"image/%s",image->magick);
  LocaleLower(value);
  (void) SetImageProperty(thumbnail_image,"Thumb::Mimetype",value);
  (void) SetImageProperty(thumbnail_image,"software",
    GetMagickVersion(&version));
  (void) FormatMagickString(value,MaxTextExtent,"%lu",image->magick_columns);
  (void) SetImageProperty(thumbnail_image,"Thumb::Image::Width",value);
  (void) FormatMagickString(value,MaxTextExtent,"%lu",image->magick_rows);
  (void) SetImageProperty(thumbnail_image,"Thumb::Image::height",value);
  (void) FormatMagickString(value,MaxTextExtent,"%lu",
    GetImageListLength(image));
  (void) SetImageProperty(thumbnail_image,"Thumb::Document::Pages",value);
  return(thumbnail_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Z o o m I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ZoomImage() creates a new image that is a scaled size of an existing one.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.  The Point filter gives fast pixel replication,
%  Triangle is equivalent to bi-linear interpolation, and Mitchel giver slower,
%  very high-quality results.  See Graphic Gems III for details on this
%  algorithm.
%
%  The filter member of the Image structure specifies which image filter to
%  use. Blur specifies the blur factor where > 1 is blurry, < 1 is sharp.
%
%  The format of the ZoomImage method is:
%
%      Image *ZoomImage(const Image *image,const unsigned long columns,
%        const unsigned long rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: An integer that specifies the number of columns in the zoom
%      image.
%
%    o rows: An integer that specifies the number of rows in the scaled
%      image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *ZoomImage(const Image *image,const unsigned long columns,
  const unsigned long rows,ExceptionInfo *exception)
{
  Image
    *zoom_image;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  zoom_image=ResizeImage(image,columns,rows,image->filter,image->blur,
    exception);
  return(zoom_image);
}

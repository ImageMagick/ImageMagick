/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           RRRR    EEEEE   SSSSS   AAA   M   M  PPPP   L      EEEEE          %
%           R   R   E       SS     A   A  MM MM  P   P  L      E              %
%           RRRR    EEE      SSS   AAAAA  M M M  PPPP   L      EEE            %
%           R R     E          SS  A   A  M   M  P      L      E              %
%           R  R    EEEEE   SSSSS  A   A  M   M  P      LLLLL  EEEEE          %
%                                                                             %
%                                                                             %
%                      MagickCore Pixel Resampling Methods                    %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                              Anthony Thyssen                                %
%                                August 2007                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/color-private.h"
#include "magick/cache.h"
#include "magick/draw.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/pixel.h"
#include "magick/pixel-private.h"
#include "magick/quantum.h"
#include "magick/random_.h"
#include "magick/resample.h"
#include "magick/resize.h"
#include "magick/resize-private.h"
#include "magick/resource_.h"
#include "magick/transform.h"
#include "magick/signature-private.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/option.h"
/*
  EWA Resampling Options
*/

/* select ONE resampling method */
#define EWA 1                 /* Normal EWA handling - raw or clamped */
                              /* if 0 then use "High Quality EWA" */
#define EWA_CLAMP 1           /* EWA Clamping from Nicolas Robidoux */

#define FILTER_LUT 1          /* Use a LUT rather then direct filter calls */

/* output debugging information */
#define DEBUG_ELLIPSE 0       /* output ellipse info for debug */
#define DEBUG_HIT_MISS 0      /* output hit/miss pixels (as gnuplot commands) */
#define DEBUG_NO_PIXEL_HIT 0  /* Make pixels that fail to hit anything - RED */

#if ! FILTER_DIRECT
#define WLUT_WIDTH 1024       /* size of the filter cache */
#endif

/*
  Typedef declarations.
*/
struct _ResampleFilter
{
  CacheView
    *view;

  Image
    *image;

  ExceptionInfo
    *exception;

  MagickBooleanType
    debug;

  /* Information about image being resampled */
  ssize_t
    image_area;

  InterpolatePixelMethod
    interpolate;

  VirtualPixelMethod
    virtual_pixel;

  FilterTypes
    filter;

  /* processing settings needed */
  MagickBooleanType
    limit_reached,
    do_interpolate,
    average_defined;

  MagickPixelPacket
    average_pixel;

  /* current ellipitical area being resampled around center point */
  double
    A, B, C,
    Vlimit, Ulimit, Uwidth, slope;

#if FILTER_LUT
  /* LUT of weights for filtered average in elliptical area */
  double
    filter_lut[WLUT_WIDTH];
#else
  /* Use a Direct call to the filter functions */
  ResizeFilter
    *filter_def;

  double
    F;
#endif

  /* the practical working support of the filter */
  double
    support;

  size_t
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e R e s a m p l e I n f o                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireResampleFilter() initializes the information resample needs do to a
%  scaled lookup of a color from an image, using area sampling.
%
%  The algorithm is based on a Elliptical Weighted Average, where the pixels
%  found in a large elliptical area is averaged together according to a
%  weighting (filter) function.  For more details see "Fundamentals of Texture
%  Mapping and Image Warping" a master's thesis by Paul.S.Heckbert, June 17,
%  1989.  Available for free from, http://www.cs.cmu.edu/~ph/
%
%  As EWA resampling (or any sort of resampling) can require a lot of
%  calculations to produce a distorted scaling of the source image for each
%  output pixel, the ResampleFilter structure generated holds that information
%  between individual image resampling.
%
%  This function will make the appropriate AcquireVirtualCacheView() calls
%  to view the image, calling functions do not need to open a cache view.
%
%  Usage Example...
%      resample_filter=AcquireResampleFilter(image,exception);
%      SetResampleFilter(resample_filter, GaussianFilter, 1.0);
%      for (y=0; y < (ssize_t) image->rows; y++) {
%        for (x=0; x < (ssize_t) image->columns; x++) {
%          u= ....;   v= ....;
%          ScaleResampleFilter(resample_filter, ... scaling vectors ...);
%          (void) ResamplePixelColor(resample_filter,u,v,&pixel);
%          ... assign resampled pixel value ...
%        }
%      }
%      DestroyResampleFilter(resample_filter);
%
%  The format of the AcquireResampleFilter method is:
%
%     ResampleFilter *AcquireResampleFilter(const Image *image,
%       ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ResampleFilter *AcquireResampleFilter(const Image *image,
  ExceptionInfo *exception)
{
  register ResampleFilter
    *resample_filter;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  resample_filter=(ResampleFilter *) AcquireMagickMemory(
    sizeof(*resample_filter));
  if (resample_filter == (ResampleFilter *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(resample_filter,0,sizeof(*resample_filter));

  resample_filter->exception=exception;
  resample_filter->image=ReferenceImage((Image *) image);
  resample_filter->view=AcquireVirtualCacheView(resample_filter->image,exception);

  resample_filter->debug=IsEventLogging();
  resample_filter->signature=MagickSignature;

  resample_filter->image_area=(ssize_t) (image->columns*image->rows);
  resample_filter->average_defined = MagickFalse;

  /* initialise the resampling filter settings */
  SetResampleFilter(resample_filter, image->filter, image->blur);
  (void) SetResampleFilterInterpolateMethod(resample_filter,
    image->interpolate);
  (void) SetResampleFilterVirtualPixelMethod(resample_filter,
    GetImageVirtualPixelMethod(image));

  return(resample_filter);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y R e s a m p l e I n f o                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyResampleFilter() finalizes and cleans up the resampling
%  resample_filter as returned by AcquireResampleFilter(), freeing any memory
%  or other information as needed.
%
%  The format of the DestroyResampleFilter method is:
%
%      ResampleFilter *DestroyResampleFilter(ResampleFilter *resample_filter)
%
%  A description of each parameter follows:
%
%    o resample_filter: resampling information structure
%
*/
MagickExport ResampleFilter *DestroyResampleFilter(
  ResampleFilter *resample_filter)
{
  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);
  assert(resample_filter->image != (Image *) NULL);
  if (resample_filter->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      resample_filter->image->filename);
  resample_filter->view=DestroyCacheView(resample_filter->view);
  resample_filter->image=DestroyImage(resample_filter->image);
#if ! FILTER_LUT
  resample_filter->filter_def=DestroyResizeFilter(resample_filter->filter_def);
#endif
  resample_filter->signature=(~MagickSignature);
  resample_filter=(ResampleFilter *) RelinquishMagickMemory(resample_filter);
  return(resample_filter);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s a m p l e P i x e l C o l o r                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResamplePixelColor() samples the pixel values surrounding the location
%  given using an elliptical weighted average, at the scale previously
%  calculated, and in the most efficent manner possible for the
%  VirtualPixelMethod setting.
%
%  The format of the ResamplePixelColor method is:
%
%     MagickBooleanType ResamplePixelColor(ResampleFilter *resample_filter,
%       const double u0,const double v0,MagickPixelPacket *pixel)
%
%  A description of each parameter follows:
%
%    o resample_filter: the resample filter.
%
%    o u0,v0: A double representing the center of the area to resample,
%        The distortion transformed transformed x,y coordinate.
%
%    o pixel: the resampled pixel is returned here.
%
*/
MagickExport MagickBooleanType ResamplePixelColor(
  ResampleFilter *resample_filter,const double u0,const double v0,
  MagickPixelPacket *pixel)
{
  MagickBooleanType
    status;

  ssize_t u,v, v1, v2, uw, hit;
  double u1;
  double U,V,Q,DQ,DDQ;
  double divisor_c,divisor_m;
  register double weight;
  register const PixelPacket *pixels;
  register const IndexPacket *indexes;
  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);

  status=MagickTrue;
  /* GetMagickPixelPacket(resample_filter->image,pixel); */
  if ( resample_filter->do_interpolate ) {
    status=InterpolateMagickPixelPacket(resample_filter->image,
      resample_filter->view,resample_filter->interpolate,u0,v0,pixel,
      resample_filter->exception);
    return(status);
  }

#if DEBUG_ELLIPSE
  (void) FormatLocaleFile(stderr, "u0=%lf; v0=%lf;\n", u0, v0);
#endif

  /*
    Does resample area Miss the image Proper?
    If and that area a simple solid color - then simply return that color!
    This saves a lot of calculation when resampling outside the bounds of
    the source image.

    However it probably should be expanded to image bounds plus the filters
    scaled support size.
  */
  hit = 0;
  switch ( resample_filter->virtual_pixel ) {
    case BackgroundVirtualPixelMethod:
    case ConstantVirtualPixelMethod:
    case TransparentVirtualPixelMethod:
    case BlackVirtualPixelMethod:
    case GrayVirtualPixelMethod:
    case WhiteVirtualPixelMethod:
    case MaskVirtualPixelMethod:
      if ( resample_filter->limit_reached
           || u0 + resample_filter->Ulimit < 0.0
           || u0 - resample_filter->Ulimit > (double) resample_filter->image->columns-1.0
           || v0 + resample_filter->Vlimit < 0.0
           || v0 - resample_filter->Vlimit > (double) resample_filter->image->rows-1.0
           )
        hit++;
      break;

    case UndefinedVirtualPixelMethod:
    case EdgeVirtualPixelMethod:
      if (    ( u0 + resample_filter->Ulimit < 0.0 && v0 + resample_filter->Vlimit < 0.0 )
           || ( u0 + resample_filter->Ulimit < 0.0
                && v0 - resample_filter->Vlimit > (double) resample_filter->image->rows-1.0 )
           || ( u0 - resample_filter->Ulimit > (double) resample_filter->image->columns-1.0
                && v0 + resample_filter->Vlimit < 0.0 )
           || ( u0 - resample_filter->Ulimit > (double) resample_filter->image->columns-1.0
                && v0 - resample_filter->Vlimit > (double) resample_filter->image->rows-1.0 )
           )
        hit++;
      break;
    case HorizontalTileVirtualPixelMethod:
      if (    v0 + resample_filter->Vlimit < 0.0
           || v0 - resample_filter->Vlimit > (double) resample_filter->image->rows-1.0
           )
        hit++;  /* outside the horizontally tiled images. */
      break;
    case VerticalTileVirtualPixelMethod:
      if (    u0 + resample_filter->Ulimit < 0.0
           || u0 - resample_filter->Ulimit > (double) resample_filter->image->columns-1.0
           )
        hit++;  /* outside the vertically tiled images. */
      break;
    case DitherVirtualPixelMethod:
      if (    ( u0 + resample_filter->Ulimit < -32.0 && v0 + resample_filter->Vlimit < -32.0 )
           || ( u0 + resample_filter->Ulimit < -32.0
                && v0 - resample_filter->Vlimit > (double) resample_filter->image->rows+31.0 )
           || ( u0 - resample_filter->Ulimit > (double) resample_filter->image->columns+31.0
                && v0 + resample_filter->Vlimit < -32.0 )
           || ( u0 - resample_filter->Ulimit > (double) resample_filter->image->columns+31.0
                && v0 - resample_filter->Vlimit > (double) resample_filter->image->rows+31.0 )
           )
        hit++;
      break;
    case TileVirtualPixelMethod:
    case MirrorVirtualPixelMethod:
    case RandomVirtualPixelMethod:
    case HorizontalTileEdgeVirtualPixelMethod:
    case VerticalTileEdgeVirtualPixelMethod:
    case CheckerTileVirtualPixelMethod:
      /* resampling of area is always needed - no VP limits */
      break;
  }
  if ( hit ) {
    /* The area being resampled is simply a solid color
     * just return a single lookup color.
     *
     * Should this return the users requested interpolated color?
     */
    status=InterpolateMagickPixelPacket(resample_filter->image,
      resample_filter->view,IntegerInterpolatePixel,u0,v0,pixel,
      resample_filter->exception);
    return(status);
  }

  /*
    When Scaling limits reached, return an 'averaged' result.
  */
  if ( resample_filter->limit_reached ) {
    switch ( resample_filter->virtual_pixel ) {
      /*  This is always handled by the above, so no need.
        case BackgroundVirtualPixelMethod:
        case ConstantVirtualPixelMethod:
        case TransparentVirtualPixelMethod:
        case GrayVirtualPixelMethod,
        case WhiteVirtualPixelMethod
        case MaskVirtualPixelMethod:
      */
      case UndefinedVirtualPixelMethod:
      case EdgeVirtualPixelMethod:
      case DitherVirtualPixelMethod:
      case HorizontalTileEdgeVirtualPixelMethod:
      case VerticalTileEdgeVirtualPixelMethod:
        /* We need an average edge pixel, from the correct edge!
           How should I calculate an average edge color?
           Just returning an averaged neighbourhood,
           works well in general, but falls down for TileEdge methods.
           This needs to be done properly!!!!!!
        */
        status=InterpolateMagickPixelPacket(resample_filter->image,
          resample_filter->view,AverageInterpolatePixel,u0,v0,pixel,
          resample_filter->exception);
        break;
      case HorizontalTileVirtualPixelMethod:
      case VerticalTileVirtualPixelMethod:
        /* just return the background pixel - Is there a better way? */
        status=InterpolateMagickPixelPacket(resample_filter->image,
          resample_filter->view,IntegerInterpolatePixel,-1.0,-1.0,pixel,
          resample_filter->exception);
        break;
      case TileVirtualPixelMethod:
      case MirrorVirtualPixelMethod:
      case RandomVirtualPixelMethod:
      case CheckerTileVirtualPixelMethod:
      default:
        /* generate a average color of the WHOLE image */
        if ( resample_filter->average_defined == MagickFalse ) {
          Image
            *average_image;

          CacheView
            *average_view;

          GetMagickPixelPacket(resample_filter->image,(MagickPixelPacket *)
            &resample_filter->average_pixel);
          resample_filter->average_defined=MagickTrue;

          /* Try to get an averaged pixel color of whole image */
          average_image=ResizeImage(resample_filter->image,1,1,BoxFilter,1.0,
            resample_filter->exception);
          if (average_image == (Image *) NULL)
            {
              *pixel=resample_filter->average_pixel; /* FAILED */
              break;
            }
          average_view=AcquireVirtualCacheView(average_image,
            &average_image->exception);
          pixels=(PixelPacket *)GetCacheViewVirtualPixels(average_view,0,0,1,1,
            resample_filter->exception);
          if (pixels == (const PixelPacket *) NULL) {
            average_view=DestroyCacheView(average_view);
            average_image=DestroyImage(average_image);
            *pixel=resample_filter->average_pixel; /* FAILED */
            break;
          }
          indexes=(IndexPacket *) GetCacheViewAuthenticIndexQueue(average_view);
          SetMagickPixelPacket(resample_filter->image,pixels,indexes,
            &(resample_filter->average_pixel));
          average_view=DestroyCacheView(average_view);
          average_image=DestroyImage(average_image);

          if ( resample_filter->virtual_pixel == CheckerTileVirtualPixelMethod )
            {
              /* CheckerTile is a alpha blend of the image's average pixel
                 color and the current background color */

              /* image's average pixel color */
              weight = QuantumScale*((MagickRealType)(QuantumRange-
                          resample_filter->average_pixel.opacity));
              resample_filter->average_pixel.red *= weight;
              resample_filter->average_pixel.green *= weight;
              resample_filter->average_pixel.blue *= weight;
              divisor_c = weight;

              /* background color */
              weight = QuantumScale*((MagickRealType)(QuantumRange-
                          resample_filter->image->background_color.opacity));
              resample_filter->average_pixel.red +=
                      weight*resample_filter->image->background_color.red;
              resample_filter->average_pixel.green +=
                      weight*resample_filter->image->background_color.green;
              resample_filter->average_pixel.blue +=
                      weight*resample_filter->image->background_color.blue;
              resample_filter->average_pixel.opacity +=
                      resample_filter->image->background_color.opacity;
              divisor_c += weight;

              /* alpha blend */
              resample_filter->average_pixel.red /= divisor_c;
              resample_filter->average_pixel.green /= divisor_c;
              resample_filter->average_pixel.blue /= divisor_c;
              resample_filter->average_pixel.opacity /= 2; /* 50% blend */

            }
        }
        *pixel=resample_filter->average_pixel;
        break;
    }
    return(status);
  }

  /*
    Initialize weighted average data collection
  */
  hit = 0;
  divisor_c = 0.0;
  divisor_m = 0.0;
  pixel->red = pixel->green = pixel->blue = 0.0;
  if (pixel->matte != MagickFalse) pixel->opacity = 0.0;
  if (pixel->colorspace == CMYKColorspace) pixel->index = 0.0;

  /*
    Determine the parellelogram bounding box fitted to the ellipse
    centered at u0,v0.  This area is bounding by the lines...
  */
  v1 = (ssize_t)ceil(v0 - resample_filter->Vlimit);  /* range of scan lines */
  v2 = (ssize_t)floor(v0 + resample_filter->Vlimit);

  /* scan line start and width accross the parallelogram */
  u1 = u0 + (v1-v0)*resample_filter->slope - resample_filter->Uwidth;
  uw = (ssize_t)(2.0*resample_filter->Uwidth)+1;

#if DEBUG_ELLIPSE
  (void) FormatLocaleFile(stderr, "v1=%ld; v2=%ld\n", (long)v1, (long)v2);
  (void) FormatLocaleFile(stderr, "u1=%ld; uw=%ld\n", (long)u1, (long)uw);
#else
# define DEBUG_HIT_MISS 0 /* only valid if DEBUG_ELLIPSE is enabled */
#endif

  /*
    Do weighted resampling of all pixels,  within the scaled ellipse,
    bound by a Parellelogram fitted to the ellipse.
  */
  DDQ = 2*resample_filter->A;
  for( v=v1; v<=v2;  v++ ) {
#if DEBUG_HIT_MISS
    long uu = ceil(u1);   /* actual pixel location (for debug only) */
    (void) FormatLocaleFile(stderr, "# scan line from pixel %ld, %ld\n", (long)uu, (long)v);
#endif
    u = (ssize_t)ceil(u1);        /* first pixel in scanline */
    u1 += resample_filter->slope; /* start of next scan line */


    /* location of this first pixel, relative to u0,v0 */
    U = (double)u-u0;
    V = (double)v-v0;

    /* Q = ellipse quotent ( if Q<F then pixel is inside ellipse) */
    Q = (resample_filter->A*U + resample_filter->B*V)*U + resample_filter->C*V*V;
    DQ = resample_filter->A*(2.0*U+1) + resample_filter->B*V;

    /* get the scanline of pixels for this v */
    pixels=GetCacheViewVirtualPixels(resample_filter->view,u,v,(size_t) uw,
      1,resample_filter->exception);
    if (pixels == (const PixelPacket *) NULL)
      return(MagickFalse);
    indexes=GetCacheViewVirtualIndexQueue(resample_filter->view);

    /* count up the weighted pixel colors */
    for( u=0; u<uw; u++ ) {
      weight = 0;
#if FILTER_LUT
      /* Note that the ellipse has been pre-scaled so F = WLUT_WIDTH */
      if ( Q < (double)WLUT_WIDTH ) {
        weight = resample_filter->filter_lut[(int)Q];
#else
      /* Note that the ellipse has been pre-scaled so F = support^2 */
      if ( Q < (double)resample_filter->F ) {
        weight = GetResizeFilterWeight(resample_filter->filter_def,
             sqrt(Q));    /* a SquareRoot!  Arrggghhhhh... */
#endif

        pixel->opacity  += weight*pixels->opacity;
        divisor_m += weight;

        if (pixel->matte != MagickFalse)
          weight *= QuantumScale*((MagickRealType)(QuantumRange-pixels->opacity));
        pixel->red   += weight*pixels->red;
        pixel->green += weight*pixels->green;
        pixel->blue  += weight*pixels->blue;
        if (pixel->colorspace == CMYKColorspace)
          pixel->index += weight*(*indexes);
        divisor_c += weight;

        hit++;
#if DEBUG_HIT_MISS
        /* mark the pixel according to hit/miss of the ellipse */
        (void) FormatLocaleFile(stderr, "set arrow from %lf,%lf to %lf,%lf nohead ls 3\n",
                     (long)uu-.1,(double)v-.1,(long)uu+.1,(long)v+.1);
        (void) FormatLocaleFile(stderr, "set arrow from %lf,%lf to %lf,%lf nohead ls 3\n",
                     (long)uu+.1,(double)v-.1,(long)uu-.1,(long)v+.1);
      } else {
        (void) FormatLocaleFile(stderr, "set arrow from %lf,%lf to %lf,%lf nohead ls 1\n",
                     (long)uu-.1,(double)v-.1,(long)uu+.1,(long)v+.1);
        (void) FormatLocaleFile(stderr, "set arrow from %lf,%lf to %lf,%lf nohead ls 1\n",
                     (long)uu+.1,(double)v-.1,(long)uu-.1,(long)v+.1);
      }
      uu++;
#else
      }
#endif
      pixels++;
      indexes++;
      Q += DQ;
      DQ += DDQ;
    }
  }
#if DEBUG_ELLIPSE
  (void) FormatLocaleFile(stderr, "Hit=%ld;  Total=%ld;\n", (long)hit, (long)uw*(v2-v1) );
#endif

  /*
    Result sanity check -- this should NOT happen
  */
  if ( hit == 0 || divisor_m <= MagickEpsilon || divisor_c <= MagickEpsilon ) {
    /* not enough pixels, or bad weighting in resampling,
       resort to direct interpolation */
#if DEBUG_NO_PIXEL_HIT
    pixel->opacity = pixel->red = pixel->green = pixel->blue = 0;
    pixel->red = QuantumRange; /* show pixels for which EWA fails */
#else
    status=InterpolateMagickPixelPacket(resample_filter->image,
      resample_filter->view,resample_filter->interpolate,u0,v0,pixel,
      resample_filter->exception);
#endif
    return status;
  }

  /*
    Finialize results of resampling
  */
  divisor_m = 1.0/divisor_m;
  pixel->opacity = (MagickRealType) ClampToQuantum(divisor_m*pixel->opacity);
  divisor_c = 1.0/divisor_c;
  pixel->red   = (MagickRealType) ClampToQuantum(divisor_c*pixel->red);
  pixel->green = (MagickRealType) ClampToQuantum(divisor_c*pixel->green);
  pixel->blue  = (MagickRealType) ClampToQuantum(divisor_c*pixel->blue);
  if (pixel->colorspace == CMYKColorspace)
    pixel->index = (MagickRealType) ClampToQuantum(divisor_c*pixel->index);
  return(MagickTrue);
}

#if EWA && EWA_CLAMP
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
-   C l a m p U p A x e s                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ClampUpAxes() function converts the input vectors into a major and
% minor axis unit vectors, and their magnitude.  This allows us to
% ensure that the ellipse generated is never smaller than the unit
% circle and thus never too small for use in EWA resampling.
%
% This purely mathematical 'magic' was provided by Professor Nicolas
% Robidoux and his Masters student Chantal Racette.
%
% Reference: "We Recommend Singular Value Decomposition", David Austin
%   http://www.ams.org/samplings/feature-column/fcarc-svd
%
% By generating major and minor axis vectors, we can actually use the
% ellipse in its "canonical form", by remapping the dx,dy of the
% sampled point into distances along the major and minor axis unit
% vectors.
%
% Reference: http://en.wikipedia.org/wiki/Ellipse#Canonical_form
*/
static inline void ClampUpAxes(const double dux,
			       const double dvx,
			       const double duy,
			       const double dvy,
			       double *major_mag,
			       double *minor_mag,
			       double *major_unit_x,
			       double *major_unit_y,
			       double *minor_unit_x,
			       double *minor_unit_y)
{
  /*
   * ClampUpAxes takes an input 2x2 matrix
   *
   * [ a b ] = [ dux duy ]
   * [ c d ] = [ dvx dvy ]
   *
   * and computes from it the major and minor axis vectors [major_x,
   * major_y] and [minor_x,minor_y] of the smallest ellipse containing
   * both the unit disk and the ellipse which is the image of the unit
   * disk by the linear transformation
   *
   * [ dux duy ] [S] = [s]
   * [ dvx dvy ] [T] = [t]
   *
   * (The vector [S,T] is the difference between a position in output
   * space and [X,Y]; the vector [s,t] is the difference between a
   * position in input space and [x,y].)
   */
  /*
   * Output:
   *
   * major_mag is the half-length of the major axis of the "new"
   * ellipse.
   *
   * minor_mag is the half-length of the minor axis of the "new"
   * ellipse.
   *
   * major_unit_x is the x-coordinate of the major axis direction vector
   * of both the "old" and "new" ellipses.
   *
   * major_unit_y is the y-coordinate of the major axis direction vector.
   *
   * minor_unit_x is the x-coordinate of the minor axis direction vector.
   *
   * minor_unit_y is the y-coordinate of the minor axis direction vector.
   *
   * Unit vectors are useful for computing projections, in particular,
   * to compute the distance between a point in output space and the
   * center of a unit disk in output space, using the position of the
   * corresponding point [s,t] in input space. Following the clamping,
   * the square of this distance is
   *
   * ( ( s * major_unit_x + t * major_unit_y ) / major_mag )^2
   * +
   * ( ( s * minor_unit_x + t * minor_unit_y ) / minor_mag )^2
   *
   * If such distances will be computed for many [s,t]'s, it makes
   * sense to actually compute the reciprocal of major_mag and
   * minor_mag and multiply them by the above unit lengths.
   *
   * Now, if you want to modify the input pair of tangent vectors so
   * that it defines the modified ellipse, all you have to do is set
   *
   * newdux = major_mag * major_unit_x
   * newdvx = major_mag * major_unit_y
   * newduy = minor_mag * minor_unit_x = minor_mag * -major_unit_y
   * newdvy = minor_mag * minor_unit_y = minor_mag *  major_unit_x
   *
   * and use these tangent vectors as if they were the original ones.
   * Usually, this is a drastic change in the tangent vectors even if
   * the singular values are not clamped; for example, the minor axis
   * vector always points in a direction which is 90 degrees
   * counterclockwise from the direction of the major axis vector.
   */
  /*
   * Discussion:
   *
   * GOAL: Fix things so that the pullback, in input space, of a disk
   * of radius r in output space is an ellipse which contains, at
   * least, a disc of radius r. (Make this hold for any r>0.)
   *
   * ESSENCE OF THE METHOD: Compute the product of the first two
   * factors of an SVD of the linear transformation defining the
   * ellipse and make sure that both its columns have norm at least 1.
   * Because rotations and reflexions map disks to themselves, it is
   * not necessary to compute the third (rightmost) factor of the SVD.
   *
   * DETAILS: Find the singular values and (unit) left singular
   * vectors of Jinv, clampling up the singular values to 1, and
   * multiply the unit left singular vectors by the new singular
   * values in order to get the minor and major ellipse axis vectors.
   *
   * Image resampling context:
   *
   * The Jacobian matrix of the transformation at the output point
   * under consideration is defined as follows:
   *
   * Consider the transformation (x,y) -> (X,Y) from input locations
   * to output locations. (Anthony Thyssen, elsewhere in resample.c,
   * uses the notation (u,v) -> (x,y).)
   *
   * The Jacobian matrix of the transformation at (x,y) is equal to
   *
   *   J = [ A, B ] = [ dX/dx, dX/dy ]
   *       [ C, D ]   [ dY/dx, dY/dy ]
   *
   * that is, the vector [A,C] is the tangent vector corresponding to
   * input changes in the horizontal direction, and the vector [B,D]
   * is the tangent vector corresponding to input changes in the
   * vertical direction.
   *
   * In the context of resampling, it is natural to use the inverse
   * Jacobian matrix Jinv because resampling is generally performed by
   * pulling pixel locations in the output image back to locations in
   * the input image. Jinv is
   *
   *   Jinv = [ a, b ] = [ dx/dX, dx/dY ]
   *          [ c, d ]   [ dy/dX, dy/dY ]
   *
   * Note: Jinv can be computed from J with the following matrix
   * formula:
   *
   *   Jinv = 1/(A*D-B*C) [  D, -B ]
   *                      [ -C,  A ]
   *
   * What we do is modify Jinv so that it generates an ellipse which
   * is as close as possible to the original but which contains the
   * unit disk. This can be accomplished as follows:
   *
   * Let
   *
   *   Jinv = U Sigma V^T
   *
   * be an SVD decomposition of Jinv. (The SVD is not unique, but the
   * final ellipse does not depend on the particular SVD.)
   *
   * We could clamp up the entries of the diagonal matrix Sigma so
   * that they are at least 1, and then set
   *
   *   Jinv = U newSigma V^T.
   *
   * However, we do not need to compute V for the following reason:
   * V^T is an orthogonal matrix (that is, it represents a combination
   * of rotations and reflexions) so that it maps the unit circle to
   * itself. For this reason, the exact value of V does not affect the
   * final ellipse, and we can choose V to be the identity
   * matrix. This gives
   *
   *   Jinv = U newSigma.
   *
   * In the end, we return the two diagonal entries of newSigma
   * together with the two columns of U.
   */
  /*
   * ClampUpAxes was written by Nicolas Robidoux and Chantal Racette
   * of Laurentian University with insightful suggestions from Anthony
   * Thyssen and funding from the National Science and Engineering
   * Research Council of Canada. It is distinguished from its
   * predecessors by its efficient handling of degenerate cases.
   *
   * The idea of clamping up the EWA ellipse's major and minor axes so
   * that the result contains the reconstruction kernel filter support
   * is taken from Andreas Gustaffson's Masters thesis "Interactive
   * Image Warping", Helsinki University of Technology, Faculty of
   * Information Technology, 59 pages, 1993 (see Section 3.6).
   *
   * The use of the SVD to clamp up the singular values of the
   * Jacobian matrix of the pullback transformation for EWA resampling
   * is taken from the astrophysicist Craig DeForest.  It is
   * implemented in his PDL::Transform code (PDL = Perl Data
   * Language).
   */
  const double a = dux;
  const double b = duy;
  const double c = dvx;
  const double d = dvy;
  /*
   * n is the matrix Jinv * transpose(Jinv). Eigenvalues of n are the
   * squares of the singular values of Jinv.
   */
  const double aa = a*a;
  const double bb = b*b;
  const double cc = c*c;
  const double dd = d*d;
  /*
   * Eigenvectors of n are left singular vectors of Jinv.
   */
  const double n11 = aa+bb;
  const double n12 = a*c+b*d;
  const double n21 = n12;
  const double n22 = cc+dd;
  const double det = a*d-b*c;
  const double twice_det = det+det;
  const double frobenius_squared = n11+n22;
  const double discriminant =
    (frobenius_squared+twice_det)*(frobenius_squared-twice_det);
  /*
   * In exact arithmetic, discriminant can't be negative. In floating
   * point, it can, because of the bad conditioning of SVD
   * decompositions done through the associated normal matrix.
   */
  const double sqrt_discriminant =
    sqrt(discriminant > 0.0 ? discriminant : 0.0);
  /*
   * s1 is the largest singular value of the inverse Jacobian
   * matrix. In other words, its reciprocal is the smallest singular
   * value of the Jacobian matrix itself.
   * If s1 = 0, both singular values are 0, and any orthogonal pair of
   * left and right factors produces a singular decomposition of Jinv.
   */
  /*
   * Initially, we only compute the squares of the singular values.
   */
  const double s1s1 = 0.5*(frobenius_squared+sqrt_discriminant);
  /*
   * s2 the smallest singular value of the inverse Jacobian
   * matrix. Its reciprocal is the largest singular value of the
   * Jacobian matrix itself.
   */
  const double s2s2 = 0.5*(frobenius_squared-sqrt_discriminant);
  const double s1s1minusn11 = s1s1-n11;
  const double s1s1minusn22 = s1s1-n22;
  /*
   * u1, the first column of the U factor of a singular decomposition
   * of Jinv, is a (non-normalized) left singular vector corresponding
   * to s1. It has entries u11 and u21. We compute u1 from the fact
   * that it is an eigenvector of n corresponding to the eigenvalue
   * s1^2.
   */
  const double s1s1minusn11_squared = s1s1minusn11*s1s1minusn11;
  const double s1s1minusn22_squared = s1s1minusn22*s1s1minusn22;
  /*
   * The following selects the largest row of n-s1^2 I as the one
   * which is used to find the eigenvector. If both s1^2-n11 and
   * s1^2-n22 are zero, n-s1^2 I is the zero matrix.  In that case,
   * any vector is an eigenvector; in addition, norm below is equal to
   * zero, and, in exact arithmetic, this is the only case in which
   * norm = 0. So, setting u1 to the simple but arbitrary vector [1,0]
   * if norm = 0 safely takes care of all cases.
   */
  const double temp_u11 =
    ( (s1s1minusn11_squared>=s1s1minusn22_squared) ? n12 : s1s1minusn22 );
  const double temp_u21 =
    ( (s1s1minusn11_squared>=s1s1minusn22_squared) ? s1s1minusn11 : n21 );
  const double norm = sqrt(temp_u11*temp_u11+temp_u21*temp_u21);
  /*
   * Finalize the entries of first left singular vector (associated
   * with the largest singular value).
   */
  const double u11 = ( (norm>0.0) ? temp_u11/norm : 1.0 );
  const double u21 = ( (norm>0.0) ? temp_u21/norm : 0.0 );
  /*
   * Clamp the singular values up to 1.
   */
  *major_mag = ( (s1s1<=1.0) ? 1.0 : sqrt(s1s1) );
  *minor_mag = ( (s2s2<=1.0) ? 1.0 : sqrt(s2s2) );
  /*
   * Return the unit major and minor axis direction vectors.
   */
  *major_unit_x = u11;
  *major_unit_y = u21;
  *minor_unit_x = -u21;
  *minor_unit_y = u11;
}

#endif
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S c a l e R e s a m p l e F i l t e r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ScaleResampleFilter() does all the calculations needed to resample an image
%  at a specific scale, defined by two scaling vectors.  This not using
%  a orthogonal scaling, but two distorted scaling vectors, to allow the
%  generation of a angled ellipse.
%
%  As only two deritive scaling vectors are used the center of the ellipse
%  must be the center of the lookup.  That is any curvature that the
%  distortion may produce is discounted.
%
%  The input vectors are produced by either finding the derivitives of the
%  distortion function, or the partial derivitives from a distortion mapping.
%  They do not need to be the orthogonal dx,dy scaling vectors, but can be
%  calculated from other derivatives.  For example you could use  dr,da/r
%  polar coordinate vector scaling vectors
%
%  If   u,v =  DistortEquation(x,y)   OR   u = Fu(x,y); v = Fv(x,y)
%  Then the scaling vectors are determined from the deritives...
%      du/dx, dv/dx     and    du/dy, dv/dy
%  If the resulting scaling vectors is othogonally aligned then...
%      dv/dx = 0   and   du/dy  =  0
%  Producing an othogonally alligned ellipse in source space for the area to
%  be resampled.
%
%  Note that scaling vectors are different to argument order.  Argument order
%  is the general order the deritives are extracted from the distortion
%  equations, and not the scaling vectors. As such the middle two vaules
%  may be swapped from what you expect.  Caution is advised.
%
%  WARNING: It is assumed that any SetResampleFilter() method call will
%  always be performed before the ScaleResampleFilter() method, so that the
%  size of the ellipse will match the support for the resampling filter being
%  used.
%
%  The format of the ScaleResampleFilter method is:
%
%     void ScaleResampleFilter(const ResampleFilter *resample_filter,
%       const double dux,const double duy,const double dvx,const double dvy)
%
%  A description of each parameter follows:
%
%    o resample_filter: the resampling resample_filterrmation defining the
%      image being resampled
%
%    o dux,duy,dvx,dvy:
%         The deritives or scaling vectors defining the EWA ellipse.
%         NOTE: watch the order, which is based on the order deritives
%         are usally determined from distortion equations (see above).
%         The middle two values may need to be swapped if you are thinking
%         in terms of scaling vectors.
%
*/
MagickExport void ScaleResampleFilter(ResampleFilter *resample_filter,
  const double dux,const double duy,const double dvx,const double dvy)
{
  double A,B,C,F;

  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);

  resample_filter->limit_reached = MagickFalse;

  /* A 'point' filter forces use of interpolation instead of area sampling */
  if ( resample_filter->filter == PointFilter )
    return; /* EWA turned off - nothing to do */

#if DEBUG_ELLIPSE
  (void) FormatLocaleFile(stderr, "# -----\n" );
  (void) FormatLocaleFile(stderr, "dux=%lf; dvx=%lf;   duy=%lf; dvy=%lf;\n",
       dux, dvx, duy, dvy);
#endif

  /* Find Ellipse Coefficents such that
        A*u^2 + B*u*v + C*v^2 = F
     With u,v relative to point around which we are resampling.
     And the given scaling dx,dy vectors in u,v space
         du/dx,dv/dx   and  du/dy,dv/dy
  */
#if EWA
  /* Direct conversion of derivatives into elliptical coefficients
     However when magnifying images, the scaling vectors will be small
     resulting in a ellipse that is too small to sample properly.
     As such we need to clamp the major/minor axis to a minumum of 1.0
     to prevent it getting too small.
  */
#if EWA_CLAMP
  { double major_mag,
           minor_mag,
           major_x,
           major_y,
           minor_x,
           minor_y;

  ClampUpAxes(dux,dvx,duy,dvy, &major_mag, &minor_mag,
                &major_x, &major_y, &minor_x, &minor_y);
  major_x *= major_mag;  major_y *= major_mag;
  minor_x *= minor_mag;  minor_y *= minor_mag;
#if DEBUG_ELLIPSE
  (void) FormatLocaleFile(stderr, "major_x=%lf; major_y=%lf;  minor_x=%lf; minor_y=%lf;\n",
        major_x, major_y, minor_x, minor_y);
#endif
  A = major_y*major_y+minor_y*minor_y;
  B = -2.0*(major_x*major_y+minor_x*minor_y);
  C = major_x*major_x+minor_x*minor_x;
  F = major_mag*minor_mag;
  F *= F; /* square it */
  }
#else /* raw unclamped EWA */
  A = dvx*dvx+dvy*dvy;
  B = -2.0*(dux*dvx+duy*dvy);
  C = dux*dux+duy*duy;
  F = dux*dvy-duy*dvx;
  F *= F; /* square it */
#endif /* EWA_CLAMP */

#else /* HQ_EWA */
  /*
    This Paul Heckbert's "Higher Quality EWA" formula, from page 60 in his
    thesis, which adds a unit circle to the elliptical area so as to do both
    Reconstruction and Prefiltering of the pixels in the resampling.  It also
    means it is always likely to have at least 4 pixels within the area of the
    ellipse, for weighted averaging.  No scaling will result with F == 4.0 and
    a circle of radius 2.0, and F smaller than this means magnification is
    being used.

    NOTE: This method produces a very blury result at near unity scale while
    producing perfect results for strong minitification and magnifications.

    However filter support is fixed to 2.0 (no good for Windowed Sinc filters)
  */
  A = dvx*dvx+dvy*dvy+1;
  B = -2.0*(dux*dvx+duy*dvy);
  C = dux*dux+duy*duy+1;
  F = A*C - B*B/4;
#endif

#if DEBUG_ELLIPSE
  (void) FormatLocaleFile(stderr, "A=%lf; B=%lf; C=%lf; F=%lf\n", A,B,C,F);

  /* Figure out the various information directly about the ellipse.
     This information currently not needed at this time, but may be
     needed later for better limit determination.

     It is also good to have as a record for future debugging
  */
  { double alpha, beta, gamma, Major, Minor;
    double Eccentricity, Ellipse_Area, Ellipse_Angle;

    alpha = A+C;
    beta  = A-C;
    gamma = sqrt(beta*beta + B*B );

    if ( alpha - gamma <= MagickEpsilon )
      Major = MagickHuge;
    else
      Major = sqrt(2*F/(alpha - gamma));
    Minor = sqrt(2*F/(alpha + gamma));

    (void) FormatLocaleFile(stderr, "# Major=%lf; Minor=%lf\n", Major, Minor );

    /* other information about ellipse include... */
    Eccentricity = Major/Minor;
    Ellipse_Area = MagickPI*Major*Minor;
    Ellipse_Angle = atan2(B, A-C);

    (void) FormatLocaleFile(stderr, "# Angle=%lf   Area=%lf\n",
         (double) RadiansToDegrees(Ellipse_Angle), Ellipse_Area);
  }
#endif

  /* If one or both of the scaling vectors is impossibly large
     (producing a very large raw F value), we may as well not bother
     doing any form of resampling since resampled area is very large.
     In this case some alternative means of pixel sampling, such as
     the average of the whole image is needed to get a reasonable
     result. Calculate only as needed.
  */
  if ( (4*A*C - B*B) > MagickHuge ) {
    resample_filter->limit_reached = MagickTrue;
    return;
  }

  /* Scale ellipse to match the filters support
     (that is, multiply F by the square of the support)
     Simplier to just multiply it by the support twice!
  */
  F *= resample_filter->support;
  F *= resample_filter->support;

  /* Orthogonal bounds of the ellipse */
  resample_filter->Ulimit = sqrt(C*F/(A*C-0.25*B*B));
  resample_filter->Vlimit = sqrt(A*F/(A*C-0.25*B*B));

  /* Horizontally aligned parallelogram fitted to Ellipse */
  resample_filter->Uwidth = sqrt(F/A); /* Half of the parallelogram width */
  resample_filter->slope = -B/(2.0*A); /* Reciprocal slope of the parallelogram */

#if DEBUG_ELLIPSE
  (void) FormatLocaleFile(stderr, "Ulimit=%lf; Vlimit=%lf; UWidth=%lf; Slope=%lf;\n",
           resample_filter->Ulimit, resample_filter->Vlimit,
           resample_filter->Uwidth, resample_filter->slope );
#endif

  /* Check the absolute area of the parallelogram involved.
   * This limit needs more work, as it is too slow for larger images
   * with tiled views of the horizon.
  */
  if ( (resample_filter->Uwidth * resample_filter->Vlimit)
         > (4.0*resample_filter->image_area)) {
    resample_filter->limit_reached = MagickTrue;
    return;
  }

  /* Scale ellipse formula to directly index the Filter Lookup Table */
  { register double scale;
#if FILTER_LUT
    /* scale so that F = WLUT_WIDTH; -- hardcoded */
    scale = (double)WLUT_WIDTH/F;
#else
    /* scale so that F = resample_filter->F (support^2) */
    scale = resample_filter->F/F;
#endif
    resample_filter->A = A*scale;
    resample_filter->B = B*scale;
    resample_filter->C = C*scale;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R e s a m p l e F i l t e r                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetResampleFilter() set the resampling filter lookup table based on a
%  specific filter.  Note that the filter is used as a radial filter not as a
%  two pass othogonally aligned resampling filter.
%
%  The format of the SetResampleFilter method is:
%
%    void SetResampleFilter(ResampleFilter *resample_filter,
%      const FilterTypes filter,const double blur)
%
%  A description of each parameter follows:
%
%    o resample_filter: resampling resample_filterrmation structure
%
%    o filter: the resize filter for elliptical weighting LUT
%
%    o blur: filter blur factor (radial scaling) for elliptical weighting LUT
%
*/
MagickExport void SetResampleFilter(ResampleFilter *resample_filter,
  const FilterTypes filter,const double blur)
{
  ResizeFilter
     *resize_filter;

  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);

  resample_filter->do_interpolate = MagickFalse;
  resample_filter->filter = filter;

  /* Default cylindrical filter is a Cubic Keys filter */
  if ( filter == UndefinedFilter )
    resample_filter->filter = RobidouxFilter;

  if ( resample_filter->filter == PointFilter ) {
    resample_filter->do_interpolate = MagickTrue;
    return;  /* EWA turned off - nothing more to do */
  }

  resize_filter = AcquireResizeFilter(resample_filter->image,
       resample_filter->filter,blur,MagickTrue,resample_filter->exception);
  if (resize_filter == (ResizeFilter *) NULL) {
    (void) ThrowMagickException(resample_filter->exception,GetMagickModule(),
         ModuleError, "UnableToSetFilteringValue",
         "Fall back to Interpolated 'Point' filter");
    resample_filter->filter = PointFilter;
    resample_filter->do_interpolate = MagickTrue;
    return;  /* EWA turned off - nothing more to do */
  }

  /* Get the practical working support for the filter,
   * after any API call blur factors have been accoded for.
   */
#if EWA
  resample_filter->support = GetResizeFilterSupport(resize_filter);
#else
  resample_filter->support = 2.0;  /* fixed support size for HQ-EWA */
#endif

#if FILTER_LUT
  /* Fill the LUT with the weights from the selected filter function */
  { register int
       Q;
    double
       r_scale;

    /* Scale radius so the filter LUT covers the full support range */
    r_scale = resample_filter->support*sqrt(1.0/(double)WLUT_WIDTH);
    for(Q=0; Q<WLUT_WIDTH; Q++)
      resample_filter->filter_lut[Q] = (double)
           GetResizeFilterWeight(resize_filter,sqrt((double)Q)*r_scale);

    /* finished with the resize filter */
    resize_filter = DestroyResizeFilter(resize_filter);
  }
#else
  /* save the filter and the scaled ellipse bounds needed for filter */
  resample_filter->filter_def = resize_filter;
  resample_filter->F = resample_filter->support*resample_filter->support;
#endif

  /*
    Adjust the scaling of the default unit circle
    This assumes that any real scaling changes will always
    take place AFTER the filter method has been initialized.
  */
  ScaleResampleFilter(resample_filter, 1.0, 0.0, 0.0, 1.0);

#if 0
  /*
    This is old code kept as a reference only. Basically it generates
    a Gaussian bell curve, with sigma = 0.5 if the support is 2.0

    Create Normal Gaussian 2D Filter Weighted Lookup Table.
    A normal EWA guassual lookup would use   exp(Q*ALPHA)
    where  Q = distance squared from 0.0 (center) to 1.0 (edge)
    and    ALPHA = -4.0*ln(2.0)  ==>  -2.77258872223978123767
    The table is of length 1024, and equates to support radius of 2.0
    thus needs to be scaled by  ALPHA*4/1024 and any blur factor squared

    The it comes from reference code provided by Fred Weinhaus.
  */
  r_scale = -2.77258872223978123767/(WLUT_WIDTH*blur*blur);
  for(Q=0; Q<WLUT_WIDTH; Q++)
    resample_filter->filter_lut[Q] = exp((double)Q*r_scale);
  resample_filter->support = WLUT_WIDTH;
#endif

#if FILTER_LUT
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp single
#endif
  {
    if (IsMagickTrue(GetImageArtifact(resample_filter->image,
             "resample:verbose")) )
      {
        register int
          Q;
        double
          r_scale;

        /* Debug output of the filter weighting LUT
          Gnuplot the LUT data, the x scale index has been adjusted
            plot [0:2][-.2:1] "lut.dat" with lines
          The filter values should be normalized for comparision
        */
        printf("#\n");
        printf("# Resampling Filter LUT (%d values) for '%s' filter\n",
                   WLUT_WIDTH, CommandOptionToMnemonic(MagickFilterOptions,
                   resample_filter->filter) );
        printf("#\n");
        printf("# Note: values in table are using a squared radius lookup.\n");
        printf("# As such its distribution is not uniform.\n");
        printf("#\n");
        printf("# The X value is the support distance for the Y weight\n");
        printf("# so you can use gnuplot to plot this cylindrical filter\n");
        printf("#    plot [0:2][-.2:1] \"lut.dat\" with lines\n");
        printf("#\n");

        /* Scale radius so the filter LUT covers the full support range */
        r_scale = resample_filter->support*sqrt(1.0/(double)WLUT_WIDTH);
        for(Q=0; Q<WLUT_WIDTH; Q++)
          printf("%8.*g %.*g\n",
              GetMagickPrecision(),sqrt((double)Q)*r_scale,
              GetMagickPrecision(),resample_filter->filter_lut[Q] );
        printf("\n\n"); /* generate a 'break' in gnuplot if multiple outputs */
      }
    /* Output the above once only for each image, and each setting
    (void) DeleteImageArtifact(resample_filter->image,"resample:verbose");
    */
  }
#endif /* FILTER_LUT */
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R e s a m p l e F i l t e r I n t e r p o l a t e M e t h o d       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetResampleFilterInterpolateMethod() sets the resample filter interpolation
%  method.
%
%  The format of the SetResampleFilterInterpolateMethod method is:
%
%      MagickBooleanType SetResampleFilterInterpolateMethod(
%        ResampleFilter *resample_filter,const InterpolateMethod method)
%
%  A description of each parameter follows:
%
%    o resample_filter: the resample filter.
%
%    o method: the interpolation method.
%
*/
MagickExport MagickBooleanType SetResampleFilterInterpolateMethod(
  ResampleFilter *resample_filter,const InterpolatePixelMethod method)
{
  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);
  assert(resample_filter->image != (Image *) NULL);
  if (resample_filter->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      resample_filter->image->filename);
  resample_filter->interpolate=method;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R e s a m p l e F i l t e r V i r t u a l P i x e l M e t h o d     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetResampleFilterVirtualPixelMethod() changes the virtual pixel method
%  associated with the specified resample filter.
%
%  The format of the SetResampleFilterVirtualPixelMethod method is:
%
%      MagickBooleanType SetResampleFilterVirtualPixelMethod(
%        ResampleFilter *resample_filter,const VirtualPixelMethod method)
%
%  A description of each parameter follows:
%
%    o resample_filter: the resample filter.
%
%    o method: the virtual pixel method.
%
*/
MagickExport MagickBooleanType SetResampleFilterVirtualPixelMethod(
  ResampleFilter *resample_filter,const VirtualPixelMethod method)
{
  assert(resample_filter != (ResampleFilter *) NULL);
  assert(resample_filter->signature == MagickSignature);
  assert(resample_filter->image != (Image *) NULL);
  if (resample_filter->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      resample_filter->image->filename);
  resample_filter->virtual_pixel=method;
  if (method != UndefinedVirtualPixelMethod)
    (void) SetCacheViewVirtualPixelMethod(resample_filter->view,method);
  return(MagickTrue);
}

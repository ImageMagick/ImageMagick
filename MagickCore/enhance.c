/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              EEEEE  N   N  H   H   AAA   N   N   CCCC  EEEEE                %
%              E      NN  N  H   H  A   A  NN  N  C      E                    %
%              EEE    N N N  HHHHH  AAAAA  N N N  C      EEE                  %
%              E      N  NN  H   H  A   A  N  NN  C      E                    %
%              EEEEE  N   N  H   H  A   A  N   N   CCCC  EEEEE                %
%                                                                             %
%                                                                             %
%                    MagickCore Image Enhancement Methods                     %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/accelerate-private.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/gem-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/histogram.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resample.h"
#include "MagickCore/resample-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/token.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xml-tree-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A u t o G a m m a I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AutoGammaImage() extract the 'mean' from the image and adjust the image
%  to try make set its gamma appropriately.
%
%  The format of the AutoGammaImage method is:
%
%      MagickBooleanType AutoGammaImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image to auto-level
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType AutoGammaImage(Image *image,
  ExceptionInfo *exception)
{
  double
    gamma,
    log_mean,
    mean,
    sans;

  MagickStatusType
    status;

  ssize_t
    i;

  log_mean=log(0.5);
  if (image->channel_mask == AllChannels)
    {
      /*
        Apply gamma correction equally across all given channels.
      */
      (void) GetImageMean(image,&mean,&sans,exception);
      gamma=log(mean*QuantumScale)/log_mean;
      return(LevelImage(image,0.0,(double) QuantumRange,gamma,exception));
    }
  /*
    Auto-gamma each channel separately.
  */
  status=MagickTrue;
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    ChannelType
      channel_mask;

    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if ((traits & UpdatePixelTrait) == 0)
      continue;
    channel_mask=SetImageChannelMask(image,(ChannelType) (1UL << i));
    status=GetImageMean(image,&mean,&sans,exception);
    gamma=log(mean*QuantumScale)/log_mean;
    status&=(MagickStatusType) LevelImage(image,0.0,(double) QuantumRange,gamma,
      exception);
    (void) SetImageChannelMask(image,channel_mask);
    if (status == MagickFalse)
      break;
  }
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A u t o L e v e l I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AutoLevelImage() adjusts the levels of a particular image channel by
%  scaling the minimum and maximum values to the full quantum range.
%
%  The format of the LevelImage method is:
%
%      MagickBooleanType AutoLevelImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image to auto-level
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType AutoLevelImage(Image *image,
  ExceptionInfo *exception)
{
  return(MinMaxStretchImage(image,0.0,0.0,1.0,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     B r i g h t n e s s C o n t r a s t I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BrightnessContrastImage() changes the brightness and/or contrast of an
%  image.  It converts the brightness and contrast parameters into slope and
%  intercept and calls a polynomial function to apply to the image.
%
%  The format of the BrightnessContrastImage method is:
%
%      MagickBooleanType BrightnessContrastImage(Image *image,
%        const double brightness,const double contrast,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o brightness: the brightness percent (-100 .. 100).
%
%    o contrast: the contrast percent (-100 .. 100).
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType BrightnessContrastImage(Image *image,
  const double brightness,const double contrast,ExceptionInfo *exception)
{
#define BrightnessContrastImageTag  "BrightnessContrast/Image"

  double
    coefficients[2],
    intercept,
    slope;

  MagickBooleanType
    status;

  /*
    Compute slope and intercept.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  slope=100.0*MagickSafeReciprocal(100.0-contrast);
  if (contrast < 0.0)
    slope=0.01*contrast+1.0;
  intercept=(0.01*brightness-0.5)*slope+0.5;
  coefficients[0]=slope;
  coefficients[1]=intercept;
  status=FunctionImage(image,PolynomialFunction,2,coefficients,exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C L A H E I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CLAHEImage() is a variant of adaptive histogram equalization in which the
%  contrast amplification is limited, so as to reduce this problem of noise
%  amplification.
%
%  Adapted from implementation by Karel Zuiderveld, karel@cv.ruu.nl in
%  "Graphics Gems IV", Academic Press, 1994.
%
%  The format of the CLAHEImage method is:
%
%      MagickBooleanType CLAHEImage(Image *image,const size_t width,
%        const size_t height,const size_t number_bins,const double clip_limit,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o width: the width of the tile divisions to use in horizontal direction.
%
%    o height: the height of the tile divisions to use in vertical direction.
%
%    o number_bins: number of bins for histogram ("dynamic range").
%
%    o clip_limit: contrast limit for localised changes in contrast. A limit
%      less than 1 results in standard non-contrast limited AHE.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _RangeInfo
{
  unsigned short
    min,
    max;
} RangeInfo;

static void ClipCLAHEHistogram(const double clip_limit,const size_t number_bins,
  size_t *histogram)
{
#define NumberCLAHEGrays  (65536)

  ssize_t
    cumulative_excess,
    excess,
    i,
    previous_excess,
    step;

  /*
    Compute total number of excess pixels.
  */
  if (number_bins == 0)
    return;
  cumulative_excess=0;
  for (i=0; i < (ssize_t) number_bins; i++)
  {
    excess=(ssize_t) histogram[i]-(ssize_t) clip_limit;
    if (excess > 0)
      cumulative_excess+=excess;
  }
  /*
    Clip histogram and redistribute excess pixels across all bins.
  */
  step=cumulative_excess/(ssize_t) number_bins;
  excess=(ssize_t) (clip_limit-step);
  for (i=0; i < (ssize_t) number_bins; i++)
  {
    if ((double) histogram[i] > clip_limit)
      histogram[i]=(size_t) clip_limit;
    else
      if ((ssize_t) histogram[i] > excess)
        {
          cumulative_excess-=(ssize_t) histogram[i]-excess;
          histogram[i]=(size_t) clip_limit;
        }
      else
        {
          cumulative_excess-=step;
          histogram[i]+=(size_t) step;
        }
  }
  /*
    Redistribute remaining excess.
  */
  do
  {
    size_t
      *p;

    size_t
      *q;

    previous_excess=cumulative_excess;
    p=histogram;
    q=histogram+number_bins;
    while ((cumulative_excess != 0) && (p < q))
    {
      step=(ssize_t) number_bins/cumulative_excess;
      if (step < 1)
        step=1;
      for (p=histogram; (p < q) && (cumulative_excess != 0); p+=(ptrdiff_t) step)
        if ((double) *p < clip_limit)
          {
            (*p)++;
            cumulative_excess--;
          }
      p++;
    }
  } while ((cumulative_excess != 0) && (cumulative_excess < previous_excess));
}

static void GenerateCLAHEHistogram(const RectangleInfo *clahe_info,
  const RectangleInfo *tile_info,const size_t number_bins,
  const unsigned short *lut,const unsigned short *pixels,size_t *histogram)
{
  const unsigned short
    *p;

  ssize_t
    i;

  /*
    Classify the pixels into a gray histogram.
  */
  for (i=0; i < (ssize_t) number_bins; i++)
    histogram[i]=0L;
  p=pixels;
  for (i=0; i < (ssize_t) tile_info->height; i++)
  {
    const unsigned short
      *q;

    q=p+tile_info->width;
    while (p < q)
      histogram[lut[*p++]]++;
    q+=(ptrdiff_t) clahe_info->width;
    p=q-tile_info->width;
  }
}

static void InterpolateCLAHE(const RectangleInfo *clahe_info,const size_t *Q12,
  const size_t *Q22,const size_t *Q11,const size_t *Q21,
  const RectangleInfo *tile,const unsigned short *lut,unsigned short *pixels)
{
  ssize_t
    y;

  unsigned short
    intensity;

  /*
    Bilinear interpolate four tiles to eliminate boundary artifacts.
  */
  for (y=(ssize_t) tile->height; y > 0; y--)
  {
    ssize_t
      x;

    for (x=(ssize_t) tile->width; x > 0; x--)
    {
      intensity=lut[*pixels];
      *pixels++=(unsigned short) (MagickSafeReciprocal((double) tile->width*
        tile->height)*(y*((double) x*Q12[intensity]+((double) tile->width-x)*
        Q22[intensity])+((double) tile->height-y)*((double) x*Q11[intensity]+
        ((double) tile->width-x)*Q21[intensity])));
    }
    pixels+=(clahe_info->width-tile->width);
  }
}

static void GenerateCLAHELut(const RangeInfo *range_info,
  const size_t number_bins,unsigned short *lut)
{
  ssize_t
    i;

  unsigned short
    delta;

  /*
    Scale input image [intensity min,max] to [0,number_bins-1].
  */
  delta=(unsigned short) ((range_info->max-range_info->min)/number_bins+1);
  for (i=(ssize_t) range_info->min; i <= (ssize_t) range_info->max; i++)
    lut[i]=(unsigned short) ((i-range_info->min)/delta);
}

static void MapCLAHEHistogram(const RangeInfo *range_info,
  const size_t number_bins,const size_t number_pixels,size_t *histogram)
{
  double
    scale,
    sum;

  ssize_t
    i;

  /*
    Rescale histogram to range [min-intensity .. max-intensity].
  */
  scale=(double) (range_info->max-range_info->min)/number_pixels;
  sum=0.0;
  for (i=0; i < (ssize_t) number_bins; i++)
  {
    sum+=histogram[i];
    histogram[i]=(size_t) (range_info->min+scale*sum);
    if (histogram[i] > range_info->max)
      histogram[i]=range_info->max;
  }
}

static MagickBooleanType CLAHE(const RectangleInfo *clahe_info,
  const RectangleInfo *tile_info,const RangeInfo *range_info,
  const size_t number_bins,const double clip_limit,unsigned short *pixels)
{
  MemoryInfo
    *tile_cache;

  unsigned short
    *p;

  size_t
    limit,
    *tiles;

  ssize_t
    y;

  unsigned short
    *lut;

  /*
    Contrast limited adapted histogram equalization.
  */
  if (clip_limit == 1.0)
    return(MagickTrue);
  tile_cache=AcquireVirtualMemory((size_t) clahe_info->x*number_bins,
    (size_t) clahe_info->y*sizeof(*tiles));
  if (tile_cache == (MemoryInfo *) NULL)
    return(MagickFalse);
  lut=(unsigned short *) AcquireQuantumMemory(NumberCLAHEGrays,sizeof(*lut));
  if (lut == (unsigned short *) NULL)
    {
      tile_cache=RelinquishVirtualMemory(tile_cache);
      return(MagickFalse);
    }
  tiles=(size_t *) GetVirtualMemoryBlob(tile_cache);
  limit=(size_t) (clip_limit*(tile_info->width*tile_info->height)/number_bins);
  if (limit < 1UL)
    limit=1UL;
  /*
    Generate greylevel mappings for each tile.
  */
  GenerateCLAHELut(range_info,number_bins,lut);
  p=pixels;
  for (y=0; y < (ssize_t) clahe_info->y; y++)
  {
    ssize_t
      x;

    for (x=0; x < (ssize_t) clahe_info->x; x++)
    {
      size_t
        *histogram;

      histogram=tiles+((ssize_t) number_bins*(y*clahe_info->x+x));
      GenerateCLAHEHistogram(clahe_info,tile_info,number_bins,lut,p,histogram);
      ClipCLAHEHistogram((double) limit,number_bins,histogram);
      MapCLAHEHistogram(range_info,number_bins,tile_info->width*
        tile_info->height,histogram);
      p+=(ptrdiff_t) tile_info->width;
    }
    p+=(ptrdiff_t) clahe_info->width*(tile_info->height-1);
  }
  /*
    Interpolate greylevel mappings to get CLAHE image.
  */
  p=pixels;
  for (y=0; y <= (ssize_t) clahe_info->y; y++)
  {
    OffsetInfo
      offset;

    RectangleInfo
      tile;

    ssize_t
      x;

    tile.height=tile_info->height;
    tile.y=y-1;
    offset.y=tile.y+1;
    if (y == 0)
      {
        /*
          Top row.
        */
        tile.height=tile_info->height >> 1;
        tile.y=0;
        offset.y=0;
      }
    else
      if (y == (ssize_t) clahe_info->y)
        {
          /*
            Bottom row.
          */
          tile.height=(tile_info->height+1) >> 1;
          tile.y=clahe_info->y-1;
          offset.y=tile.y;
        }
    for (x=0; x <= (ssize_t) clahe_info->x; x++)
    {
      tile.width=tile_info->width;
      tile.x=x-1;
      offset.x=tile.x+1;
      if (x == 0)
        {
          /*
            Left column.
          */
          tile.width=tile_info->width >> 1;
          tile.x=0;
          offset.x=0;
        }
      else
        if (x == (ssize_t) clahe_info->x)
          {
            /*
              Right column.
            */
            tile.width=(tile_info->width+1) >> 1;
            tile.x=clahe_info->x-1;
            offset.x=tile.x;
          }
      InterpolateCLAHE(clahe_info,
        tiles+((ssize_t) number_bins*(tile.y*clahe_info->x+tile.x)),   /* Q12 */
        tiles+((ssize_t) number_bins*(tile.y*clahe_info->x+offset.x)), /* Q22 */
        tiles+((ssize_t) number_bins*(offset.y*clahe_info->x+tile.x)), /* Q11 */
        tiles+((ssize_t) number_bins*(offset.y*clahe_info->x+offset.x)), /* Q21 */
        &tile,lut,p);
      p+=(ptrdiff_t) tile.width;
    }
    p+=(ptrdiff_t) clahe_info->width*(tile.height-1);
  }
  lut=(unsigned short *) RelinquishMagickMemory(lut);
  tile_cache=RelinquishVirtualMemory(tile_cache);
  return(MagickTrue);
}

MagickExport MagickBooleanType CLAHEImage(Image *image,const size_t width,
  const size_t height,const size_t number_bins,const double clip_limit,
  ExceptionInfo *exception)
{
#define CLAHEImageTag  "CLAHE/Image"

  CacheView
    *image_view;

  ColorspaceType
    colorspace;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MemoryInfo
    *pixel_cache;

  RangeInfo
    range_info;

  RectangleInfo
    clahe_info,
    tile_info;

  size_t
    n;

  ssize_t
    y;

  unsigned short
    *pixels;

  /*
    Configure CLAHE parameters.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  range_info.min=0;
  range_info.max=NumberCLAHEGrays-1;
  tile_info.width=width;
  if (tile_info.width == 0)
    tile_info.width=image->columns >> 3;
  tile_info.height=height;
  if (tile_info.height == 0)
    tile_info.height=image->rows >> 3;
  tile_info.x=0;
  if ((image->columns % tile_info.width) != 0)
    tile_info.x=(ssize_t) (tile_info.width-(image->columns % tile_info.width));
  tile_info.y=0;
  if ((image->rows % tile_info.height) != 0)
    tile_info.y=(ssize_t) (tile_info.height-(image->rows % tile_info.height));
  clahe_info.width=(size_t) ((ssize_t) image->columns+tile_info.x);
  clahe_info.height=(size_t) ((ssize_t) image->rows+tile_info.y);
  clahe_info.x=(ssize_t) (clahe_info.width/tile_info.width);
  clahe_info.y=(ssize_t) (clahe_info.height/tile_info.height);
  pixel_cache=AcquireVirtualMemory(clahe_info.width,clahe_info.height*
    sizeof(*pixels));
  if (pixel_cache == (MemoryInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  pixels=(unsigned short *) GetVirtualMemoryBlob(pixel_cache);
  colorspace=image->colorspace;
  if (TransformImageColorspace(image,LabColorspace,exception) == MagickFalse)
    {
      pixel_cache=RelinquishVirtualMemory(pixel_cache);
      return(MagickFalse);
    }
  /*
    Initialize CLAHE pixels.
  */
  image_view=AcquireVirtualCacheView(image,exception);
  progress=0;
  status=MagickTrue;
  n=0;
  for (y=0; y < (ssize_t) clahe_info.height; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-(tile_info.x >> 1),y-
      (tile_info.y >> 1),clahe_info.width,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) clahe_info.width; x++)
    {
      pixels[n++]=ScaleQuantumToShort(p[0]);
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        progress++;
        proceed=SetImageProgress(image,CLAHEImageTag,progress,2*
          GetPixelChannels(image));
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  status=CLAHE(&clahe_info,&tile_info,&range_info,number_bins == 0 ?
    (size_t) 128 : MagickMin(number_bins,256),clip_limit,pixels);
  if (status == MagickFalse)
    (void) ThrowMagickException(exception,GetMagickModule(),
      ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
  /*
    Push CLAHE pixels to CLAHE image.
  */
  image_view=AcquireAuthenticCacheView(image,exception);
  n=clahe_info.width*(size_t) (tile_info.y/2);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    n+=(size_t) (tile_info.x/2);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      q[0]=ScaleShortToQuantum(pixels[n++]);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    n+=(size_t) ((ssize_t) clahe_info.width-(ssize_t) image->columns-
      (tile_info.x/2));
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        progress++;
        proceed=SetImageProgress(image,CLAHEImageTag,progress,2*
          GetPixelChannels(image));
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  pixel_cache=RelinquishVirtualMemory(pixel_cache);
  if (TransformImageColorspace(image,colorspace,exception) == MagickFalse)
    status=MagickFalse;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C l u t I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClutImage() replaces each color value in the given image, by using it as an
%  index to lookup a replacement color value in a Color Look UP Table in the
%  form of an image.  The values are extracted along a diagonal of the CLUT
%  image so either a horizontal or vertical gradient image can be used.
%
%  Typically this is used to either re-color a gray-scale image according to a
%  color gradient in the CLUT image, or to perform a freeform histogram
%  (level) adjustment according to the (typically gray-scale) gradient in the
%  CLUT image.
%
%  When the 'channel' mask includes the matte/alpha transparency channel but
%  one image has no such channel it is assumed that image is a simple
%  gray-scale image that will effect the alpha channel values, either for
%  gray-scale coloring (with transparent or semi-transparent colors), or
%  a histogram adjustment of existing alpha channel values.   If both images
%  have matte channels, direct and normal indexing is applied, which is rarely
%  used.
%
%  The format of the ClutImage method is:
%
%      MagickBooleanType ClutImage(Image *image,Image *clut_image,
%        const PixelInterpolateMethod method,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image, which is replaced by indexed CLUT values
%
%    o clut_image: the color lookup table image for replacement color values.
%
%    o method: the pixel interpolation method.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ClutImage(Image *image,const Image *clut_image,
  const PixelInterpolateMethod method,ExceptionInfo *exception)
{
#define ClutImageTag  "Clut/Image"

  CacheView
    *clut_view,
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    *clut_map;

  ssize_t
    adjust,
    i,
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(clut_image != (Image *) NULL);
  assert(clut_image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if ((IsGrayColorspace(image->colorspace) != MagickFalse) &&
      (IsGrayColorspace(clut_image->colorspace) == MagickFalse))
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  clut_map=(PixelInfo *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*clut_map));
  if (clut_map == (PixelInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Clut image.
  */
  status=MagickTrue;
  progress=0;
  adjust=(ssize_t) (method == IntegerInterpolatePixel ? 0 : 1);
  clut_view=AcquireVirtualCacheView(clut_image,exception);
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    GetPixelInfo(clut_image,clut_map+i);
    status=InterpolatePixelInfo(clut_image,clut_view,method,(double) i*
      ((double) clut_image->columns-adjust)/MaxMap,(double) i*
      ((double) clut_image->rows-adjust)/MaxMap,clut_map+i,exception);
    if (status == MagickFalse)
      break;
  }
  clut_view=DestroyCacheView(clut_view);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    PixelInfo
      pixel;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    GetPixelInfo(image,&pixel);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      PixelTrait
        traits;

      GetPixelInfoPixel(image,q,&pixel);
      traits=GetPixelChannelTraits(image,RedPixelChannel);
      if ((traits & UpdatePixelTrait) != 0)
        pixel.red=clut_map[ScaleQuantumToMap(ClampToQuantum(
          pixel.red))].red;
      traits=GetPixelChannelTraits(image,GreenPixelChannel);
      if ((traits & UpdatePixelTrait) != 0)
        pixel.green=clut_map[ScaleQuantumToMap(ClampToQuantum(
          pixel.green))].green;
      traits=GetPixelChannelTraits(image,BluePixelChannel);
      if ((traits & UpdatePixelTrait) != 0)
        pixel.blue=clut_map[ScaleQuantumToMap(ClampToQuantum(
          pixel.blue))].blue;
      traits=GetPixelChannelTraits(image,BlackPixelChannel);
      if ((traits & UpdatePixelTrait) != 0)
        pixel.black=clut_map[ScaleQuantumToMap(ClampToQuantum(
          pixel.black))].black;
      traits=GetPixelChannelTraits(image,AlphaPixelChannel);
      if ((traits & UpdatePixelTrait) != 0)
        pixel.alpha=clut_map[ScaleQuantumToMap(ClampToQuantum(
          pixel.alpha))].alpha;
      SetPixelViaPixelInfo(image,&pixel,q);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,ClutImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  clut_map=(PixelInfo *) RelinquishMagickMemory(clut_map);
  if ((clut_image->alpha_trait != UndefinedPixelTrait) &&
      ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0))
    (void) SetImageAlphaChannel(image,ActivateAlphaChannel,exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o l o r D e c i s i o n L i s t I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ColorDecisionListImage() accepts a lightweight Color Correction Collection
%  (CCC) file which solely contains one or more color corrections and applies
%  the correction to the image.  Here is a sample CCC file:
%
%    <ColorCorrectionCollection xmlns="urn:ASC:CDL:v1.2">
%          <ColorCorrection id="cc03345">
%                <SOPNode>
%                     <Slope> 0.9 1.2 0.5 </Slope>
%                     <Offset> 0.4 -0.5 0.6 </Offset>
%                     <Power> 1.0 0.8 1.5 </Power>
%                </SOPNode>
%                <SATNode>
%                     <Saturation> 0.85 </Saturation>
%                </SATNode>
%          </ColorCorrection>
%    </ColorCorrectionCollection>
%
%  which includes the slop, offset, and power for each of the RGB channels
%  as well as the saturation.
%
%  The format of the ColorDecisionListImage method is:
%
%      MagickBooleanType ColorDecisionListImage(Image *image,
%        const char *color_correction_collection,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o color_correction_collection: the color correction collection in XML.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ColorDecisionListImage(Image *image,
  const char *color_correction_collection,ExceptionInfo *exception)
{
#define ColorDecisionListCorrectImageTag  "ColorDecisionList/Image"

  typedef struct _Correction
  {
    double
      slope,
      offset,
      power;
  } Correction;

  typedef struct _ColorCorrection
  {
    Correction
      red,
      green,
      blue;

    double
      saturation;
  } ColorCorrection;

  CacheView
    *image_view;

  char
    token[MagickPathExtent];

  ColorCorrection
    color_correction;

  const char
    *content,
    *p;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    *cdl_map;

  ssize_t
    i;

  ssize_t
    y;

  XMLTreeInfo
    *cc,
    *ccc,
    *sat,
    *sop;

  /*
    Allocate and initialize cdl maps.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (color_correction_collection == (const char *) NULL)
    return(MagickFalse);
  ccc=NewXMLTree((const char *) color_correction_collection,exception);
  if (ccc == (XMLTreeInfo *) NULL)
    return(MagickFalse);
  cc=GetXMLTreeChild(ccc,"ColorCorrection");
  if (cc == (XMLTreeInfo *) NULL)
    {
      ccc=DestroyXMLTree(ccc);
      return(MagickFalse);
    }
  color_correction.red.slope=1.0;
  color_correction.red.offset=0.0;
  color_correction.red.power=1.0;
  color_correction.green.slope=1.0;
  color_correction.green.offset=0.0;
  color_correction.green.power=1.0;
  color_correction.blue.slope=1.0;
  color_correction.blue.offset=0.0;
  color_correction.blue.power=1.0;
  color_correction.saturation=0.0;
  sop=GetXMLTreeChild(cc,"SOPNode");
  if (sop != (XMLTreeInfo *) NULL)
    {
      XMLTreeInfo
        *offset,
        *power,
        *slope;

      slope=GetXMLTreeChild(sop,"Slope");
      if (slope != (XMLTreeInfo *) NULL)
        {
          content=GetXMLTreeContent(slope);
          p=(const char *) content;
          for (i=0; (*p != '\0') && (i < 3); i++)
          {
            (void) GetNextToken(p,&p,MagickPathExtent,token);
            if (*token == ',')
              (void) GetNextToken(p,&p,MagickPathExtent,token);
            switch (i)
            {
              case 0:
              {
                color_correction.red.slope=StringToDouble(token,(char **) NULL);
                break;
              }
              case 1:
              {
                color_correction.green.slope=StringToDouble(token,
                  (char **) NULL);
                break;
              }
              case 2:
              {
                color_correction.blue.slope=StringToDouble(token,
                  (char **) NULL);
                break;
              }
            }
          }
        }
      offset=GetXMLTreeChild(sop,"Offset");
      if (offset != (XMLTreeInfo *) NULL)
        {
          content=GetXMLTreeContent(offset);
          p=(const char *) content;
          for (i=0; (*p != '\0') && (i < 3); i++)
          {
            (void) GetNextToken(p,&p,MagickPathExtent,token);
            if (*token == ',')
              (void) GetNextToken(p,&p,MagickPathExtent,token);
            switch (i)
            {
              case 0:
              {
                color_correction.red.offset=StringToDouble(token,
                  (char **) NULL);
                break;
              }
              case 1:
              {
                color_correction.green.offset=StringToDouble(token,
                  (char **) NULL);
                break;
              }
              case 2:
              {
                color_correction.blue.offset=StringToDouble(token,
                  (char **) NULL);
                break;
              }
            }
          }
        }
      power=GetXMLTreeChild(sop,"Power");
      if (power != (XMLTreeInfo *) NULL)
        {
          content=GetXMLTreeContent(power);
          p=(const char *) content;
          for (i=0; (*p != '\0') && (i < 3); i++)
          {
            (void) GetNextToken(p,&p,MagickPathExtent,token);
            if (*token == ',')
              (void) GetNextToken(p,&p,MagickPathExtent,token);
            switch (i)
            {
              case 0:
              {
                color_correction.red.power=StringToDouble(token,(char **) NULL);
                break;
              }
              case 1:
              {
                color_correction.green.power=StringToDouble(token,
                  (char **) NULL);
                break;
              }
              case 2:
              {
                color_correction.blue.power=StringToDouble(token,
                  (char **) NULL);
                break;
              }
            }
          }
        }
    }
  sat=GetXMLTreeChild(cc,"SATNode");
  if (sat != (XMLTreeInfo *) NULL)
    {
      XMLTreeInfo
        *saturation;

      saturation=GetXMLTreeChild(sat,"Saturation");
      if (saturation != (XMLTreeInfo *) NULL)
        {
          content=GetXMLTreeContent(saturation);
          p=(const char *) content;
          (void) GetNextToken(p,&p,MagickPathExtent,token);
          color_correction.saturation=StringToDouble(token,(char **) NULL);
        }
    }
  ccc=DestroyXMLTree(ccc);
  if (image->debug != MagickFalse)
    {
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  Color Correction Collection:");
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.red.slope: %g",color_correction.red.slope);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.red.offset: %g",color_correction.red.offset);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.red.power: %g",color_correction.red.power);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.green.slope: %g",color_correction.green.slope);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.green.offset: %g",color_correction.green.offset);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.green.power: %g",color_correction.green.power);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.blue.slope: %g",color_correction.blue.slope);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.blue.offset: %g",color_correction.blue.offset);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.blue.power: %g",color_correction.blue.power);
      (void) LogMagickEvent(TransformEvent,GetMagickModule(),
        "  color_correction.saturation: %g",color_correction.saturation);
    }
  cdl_map=(PixelInfo *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*cdl_map));
  if (cdl_map == (PixelInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    cdl_map[i].red=(double) ScaleMapToQuantum((double)
      (MaxMap*(pow(color_correction.red.slope*i/MaxMap+
      color_correction.red.offset,color_correction.red.power))));
    cdl_map[i].green=(double) ScaleMapToQuantum((double)
      (MaxMap*(pow(color_correction.green.slope*i/MaxMap+
      color_correction.green.offset,color_correction.green.power))));
    cdl_map[i].blue=(double) ScaleMapToQuantum((double)
      (MaxMap*(pow(color_correction.blue.slope*i/MaxMap+
      color_correction.blue.offset,color_correction.blue.power))));
  }
  if (image->storage_class == PseudoClass)
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      /*
        Apply transfer function to colormap.
      */
      double
        luma;

      luma=0.21267*image->colormap[i].red+0.71526*image->colormap[i].green+
        0.07217*image->colormap[i].blue;
      image->colormap[i].red=luma+color_correction.saturation*cdl_map[
        ScaleQuantumToMap(ClampToQuantum(image->colormap[i].red))].red-luma;
      image->colormap[i].green=luma+color_correction.saturation*cdl_map[
        ScaleQuantumToMap(ClampToQuantum(image->colormap[i].green))].green-luma;
      image->colormap[i].blue=luma+color_correction.saturation*cdl_map[
        ScaleQuantumToMap(ClampToQuantum(image->colormap[i].blue))].blue-luma;
    }
  /*
    Apply transfer function to image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      luma;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      luma=0.21267*(double) GetPixelRed(image,q)+0.71526*(double)
        GetPixelGreen(image,q)+0.07217*(double) GetPixelBlue(image,q);
      SetPixelRed(image,ClampToQuantum(luma+color_correction.saturation*
        (cdl_map[ScaleQuantumToMap(GetPixelRed(image,q))].red-luma)),q);
      SetPixelGreen(image,ClampToQuantum(luma+color_correction.saturation*
        (cdl_map[ScaleQuantumToMap(GetPixelGreen(image,q))].green-luma)),q);
      SetPixelBlue(image,ClampToQuantum(luma+color_correction.saturation*
        (cdl_map[ScaleQuantumToMap(GetPixelBlue(image,q))].blue-luma)),q);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,ColorDecisionListCorrectImageTag,
          progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  cdl_map=(PixelInfo *) RelinquishMagickMemory(cdl_map);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o n t r a s t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ContrastImage() enhances the intensity differences between the lighter and
%  darker elements of the image.  Set sharpen to a MagickTrue to increase the
%  image contrast otherwise the contrast is reduced.
%
%  The format of the ContrastImage method is:
%
%      MagickBooleanType ContrastImage(Image *image,
%        const MagickBooleanType sharpen,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o sharpen: Increase or decrease image contrast.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline void Contrast(const int sign,double *red,double *green,
  double *blue)
{
  double
    brightness = 0.0,
    hue = 0.0,
    saturation = 0.0;

  /*
    Enhance contrast: dark color become darker, light color become lighter.
  */
  ConvertRGBToHSB(*red,*green,*blue,&hue,&saturation,&brightness);
  brightness+=0.5*sign*(0.5*(sin((double) (MagickPI*(brightness-0.5)))+1.0)-
    brightness);
  if (brightness > 1.0)
    brightness=1.0;
  else
    if (brightness < 0.0)
      brightness=0.0;
  ConvertHSBToRGB(hue,saturation,brightness,red,green,blue);
}

MagickExport MagickBooleanType ContrastImage(Image *image,
  const MagickBooleanType sharpen,ExceptionInfo *exception)
{
#define ContrastImageTag  "Contrast/Image"

  CacheView
    *image_view;

  int
    sign;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    i;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  if (AccelerateContrastImage(image,sharpen,exception) != MagickFalse)
    return(MagickTrue);
#endif
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  sign=sharpen != MagickFalse ? 1 : -1;
  if (image->storage_class == PseudoClass)
    {
      /*
        Contrast enhance colormap.
      */
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        double
          blue,
          green,
          red;

        red=(double) image->colormap[i].red;
        green=(double) image->colormap[i].green;
        blue=(double) image->colormap[i].blue;
        Contrast(sign,&red,&green,&blue);
        image->colormap[i].red=(MagickRealType) red;
        image->colormap[i].green=(MagickRealType) green;
        image->colormap[i].blue=(MagickRealType) blue;
      }
    }
  /*
    Contrast enhance image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      blue,
      green,
      red;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      red=(double) GetPixelRed(image,q);
      green=(double) GetPixelGreen(image,q);
      blue=(double) GetPixelBlue(image,q);
      Contrast(sign,&red,&green,&blue);
      SetPixelRed(image,ClampToQuantum(red),q);
      SetPixelGreen(image,ClampToQuantum(green),q);
      SetPixelBlue(image,ClampToQuantum(blue),q);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,ContrastImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o n t r a s t S t r e t c h I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ContrastStretchImage() is a simple image enhancement technique that attempts
%  to improve the contrast in an image by 'stretching' the range of intensity
%  values it contains to span a desired range of values. It differs from the
%  more sophisticated histogram equalization in that it can only apply a
%  linear scaling function to the image pixel values.  As a result the
%  'enhancement' is less harsh.
%
%  The format of the ContrastStretchImage method is:
%
%      MagickBooleanType ContrastStretchImage(Image *image,
%        const char *levels,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o black_point: the black point.
%
%    o white_point: the white point.
%
%    o levels: Specify the levels where the black and white points have the
%      range of 0 to number-of-pixels (e.g. 1%, 10x90%, etc.).
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ContrastStretchImage(Image *image,
  const double black_point,const double white_point,ExceptionInfo *exception)
{
#define ContrastStretchImageTag  "ContrastStretch/Image"

  CacheView
    *image_view;

  char
    property[MagickPathExtent];

  double
    *histogram;

  ImageType
    type;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  Quantum
    *black,
    *stretch_map,
    *white;

  ssize_t
    i;

  ssize_t
    y;

  /*
    Allocate histogram and stretch map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  type=IdentifyImageType(image,exception);
  if (IsGrayImageType(type) != MagickFalse)
    (void) SetImageColorspace(image,GRAYColorspace,exception);
  black=(Quantum *) AcquireQuantumMemory(MaxPixelChannels,sizeof(*black));
  white=(Quantum *) AcquireQuantumMemory(MaxPixelChannels,sizeof(*white));
  stretch_map=(Quantum *) AcquireQuantumMemory(MaxMap+1UL,MaxPixelChannels*
    sizeof(*stretch_map));
  histogram=(double *) AcquireQuantumMemory(MaxMap+1UL,MaxPixelChannels*
    sizeof(*histogram));
  if ((black == (Quantum *) NULL) || (white == (Quantum *) NULL) ||
      (stretch_map == (Quantum *) NULL) || (histogram == (double *) NULL))
    {
      if (histogram != (double *) NULL)
        histogram=(double *) RelinquishMagickMemory(histogram);
      if (stretch_map != (Quantum *) NULL)
        stretch_map=(Quantum *) RelinquishMagickMemory(stretch_map);
      if (white != (Quantum *) NULL)
        white=(Quantum *) RelinquishMagickMemory(white);
      if (black != (Quantum *) NULL)
        black=(Quantum *) RelinquishMagickMemory(black);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Form histogram.
  */
  status=MagickTrue;
  (void) memset(histogram,0,(MaxMap+1)*GetPixelChannels(image)*
    sizeof(*histogram));
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
      double
        pixel;

      pixel=GetPixelIntensity(image,p);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        if (image->channel_mask != AllChannels)
          pixel=(double) p[i];
        histogram[GetPixelChannels(image)*ScaleQuantumToMap(
          ClampToQuantum(pixel))+(size_t) i]++;
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  image_view=DestroyCacheView(image_view);
  /*
    Find the histogram boundaries by locating the black/white levels.
  */
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    double
      intensity;

    ssize_t
      j;

    black[i]=(Quantum) 0;
    white[i]=(Quantum) ScaleQuantumToMap(QuantumRange);
    intensity=0.0;
    for (j=0; j <= (ssize_t) MaxMap; j++)
    {
      intensity+=histogram[(ssize_t) GetPixelChannels(image)*j+i];
      if (intensity > black_point)
        break;
    }
    black[i]=(Quantum) j;
    intensity=0.0;
    for (j=(ssize_t) MaxMap; j != 0; j--)
    {
      intensity+=histogram[(ssize_t) GetPixelChannels(image)*j+i];
      if (intensity > ((double) image->columns*image->rows-white_point))
        break;
    }
    white[i]=(Quantum) j;
  }
  histogram=(double *) RelinquishMagickMemory(histogram);
  /*
    Stretch the histogram to create the stretched image mapping.
  */
  (void) memset(stretch_map,0,(MaxMap+1)*GetPixelChannels(image)*
    sizeof(*stretch_map));
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    ssize_t
      j;

    for (j=0; j <= (ssize_t) MaxMap; j++)
    {
      double
        gamma;

      gamma=MagickSafeReciprocal(white[i]-black[i]);
      if (j < (ssize_t) black[i])
        stretch_map[(ssize_t) GetPixelChannels(image)*j+i]=(Quantum) 0;
      else
        if (j > (ssize_t) white[i])
          stretch_map[(ssize_t) GetPixelChannels(image)*j+i]=QuantumRange;
        else
          if (black[i] != white[i])
            stretch_map[(ssize_t) GetPixelChannels(image)*j+i]=
              ScaleMapToQuantum((double) (MaxMap*gamma*(j-(double) black[i])));
    }
  }
  if (image->storage_class == PseudoClass)
    {
      ssize_t
        j;

      /*
        Stretch-contrast colormap.
      */
      for (j=0; j < (ssize_t) image->colors; j++)
      {
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          {
            i=GetPixelChannelOffset(image,RedPixelChannel);
            image->colormap[j].red=(MagickRealType) stretch_map[
              GetPixelChannels(image)*ScaleQuantumToMap(ClampToQuantum(
              image->colormap[j].red))+(size_t) i];
          }
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          {
            i=GetPixelChannelOffset(image,GreenPixelChannel);
            image->colormap[j].green=(MagickRealType) stretch_map[
              GetPixelChannels(image)*ScaleQuantumToMap(ClampToQuantum(
              image->colormap[j].green))+(size_t) i];
          }
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          {
            i=GetPixelChannelOffset(image,BluePixelChannel);
            image->colormap[j].blue=(MagickRealType) stretch_map[
              GetPixelChannels(image)*ScaleQuantumToMap(ClampToQuantum(
              image->colormap[j].blue))+(size_t) i];
          }
        if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
          {
            i=GetPixelChannelOffset(image,AlphaPixelChannel);
            image->colormap[j].alpha=(MagickRealType) stretch_map[
              GetPixelChannels(image)*ScaleQuantumToMap(ClampToQuantum(
              image->colormap[j].alpha))+(size_t) i];
          }
      }
    }
  /*
    Stretch-contrast image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if (black[j] == white[j])
          continue;
        q[j]=ClampToQuantum(stretch_map[GetPixelChannels(image)*
          ScaleQuantumToMap(q[j])+(size_t) j]);
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,ContrastStretchImageTag,progress,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  (void) FormatLocaleString(property,MagickPathExtent,"%gx%g%%",100.0*
    QuantumScale*GetPixelIntensity(image,black),100.0*QuantumScale*
    GetPixelIntensity(image,white));
  (void) SetImageProperty(image,"histogram:contrast-stretch",property,
    exception);
  white=(Quantum *) RelinquishMagickMemory(white);
  black=(Quantum *) RelinquishMagickMemory(black);
  stretch_map=(Quantum *) RelinquishMagickMemory(stretch_map);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E n h a n c e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EnhanceImage() applies a digital filter that improves the quality of a
%  noisy image.
%
%  The format of the EnhanceImage method is:
%
%      Image *EnhanceImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *EnhanceImage(const Image *image,ExceptionInfo *exception)
{
#define EnhanceImageTag  "Enhance/Image"
#define EnhancePixel(weight) \
  mean=QuantumScale*((double) GetPixelRed(image,r)+pixel.red)/2.0; \
  distance=QuantumScale*((double) GetPixelRed(image,r)-pixel.red); \
  distance_squared=(4.0+mean)*distance*distance; \
  mean=QuantumScale*((double) GetPixelGreen(image,r)+pixel.green)/2.0; \
  distance=QuantumScale*((double) GetPixelGreen(image,r)-pixel.green); \
  distance_squared+=(7.0-mean)*distance*distance; \
  mean=QuantumScale*((double) GetPixelBlue(image,r)+pixel.blue)/2.0; \
  distance=QuantumScale*((double) GetPixelBlue(image,r)-pixel.blue); \
  distance_squared+=(5.0-mean)*distance*distance; \
  mean=QuantumScale*((double) GetPixelBlack(image,r)+pixel.black)/2.0; \
  distance=QuantumScale*((double) GetPixelBlack(image,r)-pixel.black); \
  distance_squared+=(5.0-mean)*distance*distance; \
  mean=QuantumScale*((double) GetPixelAlpha(image,r)+pixel.alpha)/2.0; \
  distance=QuantumScale*((double) GetPixelAlpha(image,r)-pixel.alpha); \
  distance_squared+=(5.0-mean)*distance*distance; \
  if (distance_squared < 0.069) \
    { \
      aggregate.red+=(weight)*(double) GetPixelRed(image,r); \
      aggregate.green+=(weight)*(double) GetPixelGreen(image,r); \
      aggregate.blue+=(weight)*(double) GetPixelBlue(image,r); \
      aggregate.black+=(weight)*(double) GetPixelBlack(image,r); \
      aggregate.alpha+=(weight)*(double) GetPixelAlpha(image,r); \
      total_weight+=(weight); \
    } \
  r+=(ptrdiff_t) GetPixelChannels(image);

  CacheView
    *enhance_view,
    *image_view;

  Image
    *enhance_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  /*
    Initialize enhanced image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  enhance_image=CloneImage(image,0,0,MagickTrue,
    exception);
  if (enhance_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(enhance_image,DirectClass,exception) == MagickFalse)
    {
      enhance_image=DestroyImage(enhance_image);
      return((Image *) NULL);
    }
  /*
    Enhance image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  enhance_view=AcquireAuthenticCacheView(enhance_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,enhance_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    PixelInfo
      pixel;

    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    ssize_t
      center;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-2,y-2,image->columns+4,5,exception);
    q=QueueCacheViewAuthenticPixels(enhance_view,0,y,enhance_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    center=(ssize_t) GetPixelChannels(image)*(2*((ssize_t) image->columns+4)+2);
    GetPixelInfo(image,&pixel);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        distance,
        distance_squared,
        mean,
        total_weight;

      PixelInfo
        aggregate;

      const Quantum
        *magick_restrict r;

      GetPixelInfo(image,&aggregate);
      total_weight=0.0;
      GetPixelInfoPixel(image,p+center,&pixel);
      r=p;
      EnhancePixel(5.0); EnhancePixel(8.0); EnhancePixel(10.0);
        EnhancePixel(8.0); EnhancePixel(5.0);
      r=p+GetPixelChannels(image)*(image->columns+4);
      EnhancePixel(8.0); EnhancePixel(20.0); EnhancePixel(40.0);
        EnhancePixel(20.0); EnhancePixel(8.0);
      r=p+2*GetPixelChannels(image)*(image->columns+4);
      EnhancePixel(10.0); EnhancePixel(40.0); EnhancePixel(80.0);
        EnhancePixel(40.0); EnhancePixel(10.0);
      r=p+3*GetPixelChannels(image)*(image->columns+4);
      EnhancePixel(8.0); EnhancePixel(20.0); EnhancePixel(40.0);
        EnhancePixel(20.0); EnhancePixel(8.0);
      r=p+4*GetPixelChannels(image)*(image->columns+4);
      EnhancePixel(5.0); EnhancePixel(8.0); EnhancePixel(10.0);
        EnhancePixel(8.0); EnhancePixel(5.0);
      if (total_weight > MagickEpsilon)
        {
          pixel.red=((aggregate.red+total_weight/2.0)/total_weight);
          pixel.green=((aggregate.green+total_weight/2.0)/total_weight);
          pixel.blue=((aggregate.blue+total_weight/2.0)/total_weight);
          pixel.black=((aggregate.black+total_weight/2.0)/total_weight);
          pixel.alpha=((aggregate.alpha+total_weight/2.0)/total_weight);
        }
      SetPixelViaPixelInfo(enhance_image,&pixel,q);
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(enhance_image);
    }
    if (SyncCacheViewAuthenticPixels(enhance_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,EnhanceImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  enhance_view=DestroyCacheView(enhance_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    enhance_image=DestroyImage(enhance_image);
  return(enhance_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E q u a l i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EqualizeImage() applies a histogram equalization to the image.
%
%  The format of the EqualizeImage method is:
%
%      MagickBooleanType EqualizeImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType EqualizeImage(Image *image,
  ExceptionInfo *exception)
{
#define EqualizeImageTag  "Equalize/Image"

  CacheView
    *image_view;

  double
    black[2*CompositePixelChannel+1],
    *equalize_map,
    *histogram,
    *map,
    white[2*CompositePixelChannel+1];

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    i;

  ssize_t
    y;

  /*
    Allocate and initialize histogram arrays.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  if (AccelerateEqualizeImage(image,exception) != MagickFalse)
    return(MagickTrue);
#endif
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  equalize_map=(double *) AcquireQuantumMemory(MaxMap+1UL,MaxPixelChannels*
    sizeof(*equalize_map));
  histogram=(double *) AcquireQuantumMemory(MaxMap+1UL,MaxPixelChannels*
    sizeof(*histogram));
  map=(double *) AcquireQuantumMemory(MaxMap+1UL,MaxPixelChannels*sizeof(*map));
  if ((equalize_map == (double *) NULL) || (histogram == (double *) NULL) ||
      (map == (double *) NULL))
    {
      if (map != (double *) NULL)
        map=(double *) RelinquishMagickMemory(map);
      if (histogram != (double *) NULL)
        histogram=(double *) RelinquishMagickMemory(histogram);
      if (equalize_map != (double *) NULL)
        equalize_map=(double *) RelinquishMagickMemory(equalize_map);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Form histogram.
  */
  status=MagickTrue;
  (void) memset(histogram,0,(MaxMap+1)*GetPixelChannels(image)*
    sizeof(*histogram));
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
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          intensity;

        intensity=(double) p[i];
        if ((image->channel_mask & SyncChannels) != 0)
          intensity=GetPixelIntensity(image,p);
        histogram[GetPixelChannels(image)*ScaleQuantumToMap(
          ClampToQuantum(intensity))+(size_t) i]++;
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  image_view=DestroyCacheView(image_view);
  /*
    Integrate the histogram to get the equalization map.
  */
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    double
      intensity;

    ssize_t
      j;

    intensity=0.0;
    for (j=0; j <= (ssize_t) MaxMap; j++)
    {
      intensity+=histogram[(ssize_t) GetPixelChannels(image)*j+i];
      map[(ssize_t) GetPixelChannels(image)*j+i]=intensity;
    }
  }
  (void) memset(equalize_map,0,(MaxMap+1)*GetPixelChannels(image)*
    sizeof(*equalize_map));
  (void) memset(black,0,sizeof(*black));
  (void) memset(white,0,sizeof(*white));
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    ssize_t
      j;

    black[i]=map[i];
    white[i]=map[GetPixelChannels(image)*MaxMap+(size_t) i];
    if (black[i] != white[i])
      for (j=0; j <= (ssize_t) MaxMap; j++)
        equalize_map[GetPixelChannels(image)*(size_t) j+(size_t) i]=(double)
          ScaleMapToQuantum((double) ((MaxMap*(map[GetPixelChannels(image)*
          (size_t) j+(size_t) i]-black[i]))/(white[i]-black[i])));
  }
  histogram=(double *) RelinquishMagickMemory(histogram);
  map=(double *) RelinquishMagickMemory(map);
  if (image->storage_class == PseudoClass)
    {
      ssize_t
        j;

      /*
        Equalize colormap.
      */
      for (j=0; j < (ssize_t) image->colors; j++)
      {
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          {
            PixelChannel channel = GetPixelChannelChannel(image,
              RedPixelChannel);
            if (black[channel] != white[channel])
              image->colormap[j].red=equalize_map[(ssize_t)
                GetPixelChannels(image)*ScaleQuantumToMap(
                ClampToQuantum(image->colormap[j].red))+channel];
          }
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          {
            PixelChannel channel = GetPixelChannelChannel(image,
              GreenPixelChannel);
            if (black[channel] != white[channel])
              image->colormap[j].green=equalize_map[(ssize_t)
                GetPixelChannels(image)*ScaleQuantumToMap(
                ClampToQuantum(image->colormap[j].green))+channel];
          }
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          {
            PixelChannel channel = GetPixelChannelChannel(image,
              BluePixelChannel);
            if (black[channel] != white[channel])
              image->colormap[j].blue=equalize_map[(ssize_t)
                GetPixelChannels(image)*ScaleQuantumToMap(
                ClampToQuantum(image->colormap[j].blue))+channel];
          }
        if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
          {
            PixelChannel channel = GetPixelChannelChannel(image,
              AlphaPixelChannel);
            if (black[channel] != white[channel])
              image->colormap[j].alpha=equalize_map[(ssize_t)
                GetPixelChannels(image)*ScaleQuantumToMap(
                ClampToQuantum(image->colormap[j].alpha))+channel];
          }
      }
    }
  /*
    Equalize image.
  */
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (((traits & UpdatePixelTrait) == 0) || (black[j] == white[j]))
          continue;
        q[j]=ClampToQuantum(equalize_map[GetPixelChannels(image)*
          ScaleQuantumToMap(q[j])+(size_t) j]);
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,EqualizeImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  equalize_map=(double *) RelinquishMagickMemory(equalize_map);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     G a m m a I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GammaImage() gamma-corrects a particular image channel.  The same
%  image viewed on different devices will have perceptual differences in the
%  way the image's intensities are represented on the screen.  Specify
%  individual gamma levels for the red, green, and blue channels, or adjust
%  all three with the gamma parameter.  Values typically range from 0.8 to 2.3.
%
%  You can also reduce the influence of a particular channel with a gamma
%  value of 0.
%
%  The format of the GammaImage method is:
%
%      MagickBooleanType GammaImage(Image *image,const double gamma,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o level: the image gamma as a string (e.g. 1.6,1.2,1.0).
%
%    o gamma: the image gamma.
%
*/

static inline double gamma_pow(const double value,const double gamma)
{
  return(value < 0.0 ? value : pow(value,gamma));
}

MagickExport MagickBooleanType GammaImage(Image *image,const double gamma,
  ExceptionInfo *exception)
{
#define GammaImageTag  "Gamma/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  Quantum
    *gamma_map;

  ssize_t
    i;

  ssize_t
    y;

  /*
    Allocate and initialize gamma maps.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (gamma == 1.0)
    return(MagickTrue);
  gamma_map=(Quantum *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*gamma_map));
  if (gamma_map == (Quantum *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  (void) memset(gamma_map,0,(MaxMap+1)*sizeof(*gamma_map));
  if (gamma != 0.0)
    for (i=0; i <= (ssize_t) MaxMap; i++)
      gamma_map[i]=ScaleMapToQuantum((double) (MaxMap*pow((double) i/
        MaxMap,MagickSafeReciprocal(gamma))));
  if (image->storage_class == PseudoClass)
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      /*
        Gamma-correct colormap.
      */
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].red=(double) gamma_map[ScaleQuantumToMap(
          ClampToQuantum(image->colormap[i].red))];
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].green=(double) gamma_map[ScaleQuantumToMap(
          ClampToQuantum(image->colormap[i].green))];
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].blue=(double) gamma_map[ScaleQuantumToMap(
          ClampToQuantum(image->colormap[i].blue))];
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].alpha=(double) gamma_map[ScaleQuantumToMap(
          ClampToQuantum(image->colormap[i].alpha))];
    }
  /*
    Gamma-correct image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[j]=gamma_map[ScaleQuantumToMap(ClampToQuantum((MagickRealType)
          q[j]))];
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,GammaImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  gamma_map=(Quantum *) RelinquishMagickMemory(gamma_map);
  if (image->gamma != 0.0)
    image->gamma*=gamma;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     G r a y s c a l e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GrayscaleImage() converts the image to grayscale.
%
%  The format of the GrayscaleImage method is:
%
%      MagickBooleanType GrayscaleImage(Image *image,
%        const PixelIntensityMethod method ,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o method: the pixel intensity method.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType GrayscaleImage(Image *image,
  const PixelIntensityMethod method,ExceptionInfo *exception)
{
#define GrayscaleImageTag  "Grayscale/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    {
      if (SyncImage(image,exception) == MagickFalse)
        return(MagickFalse);
      if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
        return(MagickFalse);
    }
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  if (AccelerateGrayscaleImage(image,method,exception) != MagickFalse)
    {
      image->intensity=method;
      image->type=GrayscaleType;
      if ((method == Rec601LuminancePixelIntensityMethod) ||
          (method == Rec709LuminancePixelIntensityMethod))
        return(SetImageColorspace(image,LinearGRAYColorspace,exception));
      return(SetImageColorspace(image,GRAYColorspace,exception));
    }
#endif
  /*
    Grayscale image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        blue,
        green,
        red,
        intensity;

      red=(MagickRealType) GetPixelRed(image,q);
      green=(MagickRealType) GetPixelGreen(image,q);
      blue=(MagickRealType) GetPixelBlue(image,q);
      intensity=0.0;
      switch (method)
      {
        case AveragePixelIntensityMethod:
        {
          intensity=(red+green+blue)/3.0;
          break;
        }
        case BrightnessPixelIntensityMethod:
        {
          intensity=MagickMax(MagickMax(red,green),blue);
          break;
        }
        case LightnessPixelIntensityMethod:
        {
          intensity=(MagickMin(MagickMin(red,green),blue)+
            MagickMax(MagickMax(red,green),blue))/2.0;
          break;
        }
        case MSPixelIntensityMethod:
        {
          intensity=(MagickRealType) (((double) red*red+green*green+
            blue*blue)/3.0);
          break;
        }
        case Rec601LumaPixelIntensityMethod:
        {
          if (image->colorspace == RGBColorspace)
            {
              red=EncodePixelGamma(red);
              green=EncodePixelGamma(green);
              blue=EncodePixelGamma(blue);
            }
          intensity=0.298839*red+0.586811*green+0.114350*blue;
          break;
        }
        case Rec601LuminancePixelIntensityMethod:
        {
          if (image->colorspace == sRGBColorspace)
            {
              red=DecodePixelGamma(red);
              green=DecodePixelGamma(green);
              blue=DecodePixelGamma(blue);
            }
          intensity=0.298839*red+0.586811*green+0.114350*blue;
          break;
        }
        case Rec709LumaPixelIntensityMethod:
        default:
        {
          if (image->colorspace == RGBColorspace)
            {
              red=EncodePixelGamma(red);
              green=EncodePixelGamma(green);
              blue=EncodePixelGamma(blue);
            }
          intensity=0.212656*red+0.715158*green+0.072186*blue;
          break;
        }
        case Rec709LuminancePixelIntensityMethod:
        {
          if (image->colorspace == sRGBColorspace)
            {
              red=DecodePixelGamma(red);
              green=DecodePixelGamma(green);
              blue=DecodePixelGamma(blue);
            }
          intensity=0.212656*red+0.715158*green+0.072186*blue;
          break;
        }
        case RMSPixelIntensityMethod:
        {
          intensity=(MagickRealType) (sqrt((double) red*red+green*green+
            blue*blue)/sqrt(3.0));
          break;
        }
      }
      SetPixelGray(image,ClampToQuantum(intensity),q);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,GrayscaleImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  image->intensity=method;
  image->type=GrayscaleType;
  if ((method == Rec601LuminancePixelIntensityMethod) ||
      (method == Rec709LuminancePixelIntensityMethod))
    return(SetImageColorspace(image,LinearGRAYColorspace,exception));
  return(SetImageColorspace(image,GRAYColorspace,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     H a l d C l u t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  HaldClutImage() applies a Hald color lookup table to the image.  A Hald
%  color lookup table is a 3-dimensional color cube mapped to 2 dimensions.
%  Create it with the HALD coder.  You can apply any color transformation to
%  the Hald image and then use this method to apply the transform to the
%  image.
%
%  The format of the HaldClutImage method is:
%
%      MagickBooleanType HaldClutImage(Image *image,Image *hald_image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image, which is replaced by indexed CLUT values
%
%    o hald_image: the color lookup table image for replacement color values.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType HaldClutImage(Image *image,
  const Image *hald_image,ExceptionInfo *exception)
{
#define HaldClutImageTag  "Clut/Image"

  typedef struct _HaldInfo
  {
    double
      x,
      y,
      z;
  } HaldInfo;

  CacheView
    *hald_view,
    *image_view;

  double
    width;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    zero;

  size_t
    cube_size,
    length,
    level;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(hald_image != (Image *) NULL);
  assert(hald_image->signature == MagickCoreSignature);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if ((image->alpha_trait & BlendPixelTrait) == 0)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
  if (image->colorspace != hald_image->colorspace)
    (void) SetImageColorspace(image,hald_image->colorspace,exception);
  /*
    Hald clut image.
  */
  status=MagickTrue;
  progress=0;
  length=(size_t) MagickMin((MagickRealType) hald_image->columns,
    (MagickRealType) hald_image->rows);
  for (level=2; (level*level*level) < length; level++) ;
  level*=level;
  cube_size=level*level;
  width=(double) hald_image->columns;
  GetPixelInfo(hald_image,&zero);
  hald_view=AcquireVirtualCacheView(hald_image,exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        area = 0.0,
        offset = 0.0;

      HaldInfo
        point = { 0, 0, 0 };

      PixelInfo
        pixel = zero,
        pixel1 = zero,
        pixel2 = zero,
        pixel3 = zero,
        pixel4 = zero;

      point.x=QuantumScale*(level-1.0)*(double) GetPixelRed(image,q);
      point.y=QuantumScale*(level-1.0)*(double) GetPixelGreen(image,q);
      point.z=QuantumScale*(level-1.0)*(double) GetPixelBlue(image,q);
      offset=point.x+level*floor(point.y)+cube_size*floor(point.z);
      point.x-=floor(point.x);
      point.y-=floor(point.y);
      point.z-=floor(point.z);
      status=InterpolatePixelInfo(hald_image,hald_view,hald_image->interpolate,
        fmod(offset,width),floor(offset/width),&pixel1,exception);
      if (status == MagickFalse)
        break;
      status=InterpolatePixelInfo(hald_image,hald_view,hald_image->interpolate,
        fmod(offset+level,width),floor((offset+level)/width),&pixel2,exception);
      if (status == MagickFalse)
        break;
      area=point.y;
      if (hald_image->interpolate == NearestInterpolatePixel)
        area=(point.y < 0.5) ? 0.0 : 1.0;
      CompositePixelInfoAreaBlend(&pixel1,pixel1.alpha,&pixel2,pixel2.alpha,
        area,&pixel3);
      offset+=cube_size;
      status=InterpolatePixelInfo(hald_image,hald_view,hald_image->interpolate,
        fmod(offset,width),floor(offset/width),&pixel1,exception);
      if (status == MagickFalse)
        break;
      status=InterpolatePixelInfo(hald_image,hald_view,hald_image->interpolate,
        fmod(offset+level,width),floor((offset+level)/width),&pixel2,exception);
      if (status == MagickFalse)
        break;
      CompositePixelInfoAreaBlend(&pixel1,pixel1.alpha,&pixel2,pixel2.alpha,
        area,&pixel4);
      area=point.z;
      if (hald_image->interpolate == NearestInterpolatePixel)
        area=(point.z < 0.5)? 0.0 : 1.0;
      CompositePixelInfoAreaBlend(&pixel3,pixel3.alpha,&pixel4,pixel4.alpha,
        area,&pixel);
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        SetPixelRed(image,ClampToQuantum(pixel.red),q);
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        SetPixelGreen(image,ClampToQuantum(pixel.green),q);
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        SetPixelBlue(image,ClampToQuantum(pixel.blue),q);
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(image,ClampToQuantum(pixel.black),q);
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
          (image->alpha_trait != UndefinedPixelTrait))
        SetPixelAlpha(image,ClampToQuantum(pixel.alpha),q);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,HaldClutImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  hald_view=DestroyCacheView(hald_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L e v e l I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LevelImage() adjusts the levels of a particular image channel by
%  scaling the colors falling between specified white and black points to
%  the full available quantum range.
%
%  The parameters provided represent the black, and white points.  The black
%  point specifies the darkest color in the image. Colors darker than the
%  black point are set to zero.  White point specifies the lightest color in
%  the image.  Colors brighter than the white point are set to the maximum
%  quantum value.
%
%  If a '!' flag is given, map black and white colors to the given levels
%  rather than mapping those levels to black and white.  See
%  LevelizeImage() below.
%
%  Gamma specifies a gamma correction to apply to the image.
%
%  The format of the LevelImage method is:
%
%      MagickBooleanType LevelImage(Image *image,const double black_point,
%        const double white_point,const double gamma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o black_point: The level to map zero (black) to.
%
%    o white_point: The level to map QuantumRange (white) to.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double LevelPixel(const double black_point,
  const double white_point,const double gamma,const double pixel)
{
  double
    level_pixel,
    scale;

  scale=MagickSafeReciprocal(white_point-black_point);
  level_pixel=(double) QuantumRange*gamma_pow(scale*((double) pixel-(double)
    black_point),MagickSafeReciprocal(gamma));
  return(level_pixel);
}

MagickExport MagickBooleanType LevelImage(Image *image,const double black_point,
  const double white_point,const double gamma,ExceptionInfo *exception)
{
#define LevelImageTag  "Level/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    i,
    y;

  /*
    Allocate and initialize levels map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      /*
        Level colormap.
      */
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].red=(double) ClampToQuantum(LevelPixel(black_point,
          white_point,gamma,image->colormap[i].red));
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].green=(double) ClampToQuantum(LevelPixel(black_point,
          white_point,gamma,image->colormap[i].green));
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].blue=(double) ClampToQuantum(LevelPixel(black_point,
          white_point,gamma,image->colormap[i].blue));
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].alpha=(double) ClampToQuantum(LevelPixel(black_point,
          white_point,gamma,image->colormap[i].alpha));
    }
  /*
    Level image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[j]=ClampToQuantum(LevelPixel(black_point,white_point,gamma,
          (double) q[j]));
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,LevelImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  (void) ClampImage(image,exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L e v e l i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LevelizeImage() applies the reversed LevelImage() operation to just
%  the specific channels specified.  It compresses the full range of color
%  values, so that they lie between the given black and white points. Gamma is
%  applied before the values are mapped.
%
%  LevelizeImage() can be called with by using a +level command line
%  API option, or using a '!' on a -level or LevelImage() geometry string.
%
%  It can be used to de-contrast a greyscale image to the exact levels
%  specified.  Or by using specific levels for each channel of an image you
%  can convert a gray-scale image to any linear color gradient, according to
%  those levels.
%
%  The format of the LevelizeImage method is:
%
%      MagickBooleanType LevelizeImage(Image *image,const double black_point,
%        const double white_point,const double gamma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o black_point: The level to map zero (black) to.
%
%    o white_point: The level to map QuantumRange (white) to.
%
%    o gamma: adjust gamma by this factor before mapping values.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType LevelizeImage(Image *image,
  const double black_point,const double white_point,const double gamma,
  ExceptionInfo *exception)
{
#define LevelizeImageTag  "Levelize/Image"
#define LevelizeValue(x) ClampToQuantum(((MagickRealType) gamma_pow((double) \
  (QuantumScale*((double) x)),gamma))*(white_point-black_point)+black_point)

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    i;

  ssize_t
    y;

  /*
    Allocate and initialize levels map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      /*
        Level colormap.
      */
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].red=(double) LevelizeValue(image->colormap[i].red);
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].green=(double) LevelizeValue(
          image->colormap[i].green);
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].blue=(double) LevelizeValue(image->colormap[i].blue);
      if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].alpha=(double) LevelizeValue(
          image->colormap[i].alpha);
    }
  /*
    Level image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[j]=LevelizeValue(q[j]);
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,LevelizeImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L e v e l I m a g e C o l o r s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LevelImageColors() maps the given color to "black" and "white" values,
%  linearly spreading out the colors, and level values on a channel by channel
%  bases, as per LevelImage().  The given colors allows you to specify
%  different level ranges for each of the color channels separately.
%
%  If the boolean 'invert' is set true the image values will modified in the
%  reverse direction. That is any existing "black" and "white" colors in the
%  image will become the color values given, with all other values compressed
%  appropriately.  This effectively maps a greyscale gradient into the given
%  color gradient.
%
%  The format of the LevelImageColors method is:
%
%    MagickBooleanType LevelImageColors(Image *image,
%      const PixelInfo *black_color,const PixelInfo *white_color,
%      const MagickBooleanType invert,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o black_color: The color to map black to/from
%
%    o white_point: The color to map white to/from
%
%    o invert: if true map the colors (levelize), rather than from (level)
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType LevelImageColors(Image *image,
  const PixelInfo *black_color,const PixelInfo *white_color,
  const MagickBooleanType invert,ExceptionInfo *exception)
{
  ChannelType
    channel_mask;

  MagickStatusType
    status;

  /*
    Allocate and initialize levels map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((IsGrayColorspace(image->colorspace) != MagickFalse) &&
      ((IsGrayColorspace(black_color->colorspace) == MagickFalse) ||
       (IsGrayColorspace(white_color->colorspace) == MagickFalse)))
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  status=MagickTrue;
  if (invert == MagickFalse)
    {
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        {
          channel_mask=SetImageChannelMask(image,RedChannel);
          status&=(MagickStatusType) LevelImage(image,black_color->red,
            white_color->red,1.0,exception);
          (void) SetImageChannelMask(image,channel_mask);
        }
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        {
          channel_mask=SetImageChannelMask(image,GreenChannel);
          status&=(MagickStatusType) LevelImage(image,black_color->green,
            white_color->green,1.0,exception);
          (void) SetImageChannelMask(image,channel_mask);
        }
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        {
          channel_mask=SetImageChannelMask(image,BlueChannel);
          status&=(MagickStatusType) LevelImage(image,black_color->blue,
            white_color->blue,1.0,exception);
          (void) SetImageChannelMask(image,channel_mask);
        }
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          channel_mask=SetImageChannelMask(image,BlackChannel);
          status&=(MagickStatusType) LevelImage(image,black_color->black,
            white_color->black,1.0,exception);
          (void) SetImageChannelMask(image,channel_mask);
        }
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
          (image->alpha_trait != UndefinedPixelTrait))
        {
          channel_mask=SetImageChannelMask(image,AlphaChannel);
          status&=(MagickStatusType) LevelImage(image,black_color->alpha,
            white_color->alpha,1.0,exception);
          (void) SetImageChannelMask(image,channel_mask);
        }
    }
  else
    {
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        {
          channel_mask=SetImageChannelMask(image,RedChannel);
          status&=(MagickStatusType) LevelizeImage(image,black_color->red,
            white_color->red,1.0,exception);
          (void) SetImageChannelMask(image,channel_mask);
        }
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        {
          channel_mask=SetImageChannelMask(image,GreenChannel);
          status&=(MagickStatusType) LevelizeImage(image,black_color->green,
            white_color->green,1.0,exception);
          (void) SetImageChannelMask(image,channel_mask);
        }
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        {
          channel_mask=SetImageChannelMask(image,BlueChannel);
          status&=(MagickStatusType) LevelizeImage(image,black_color->blue,
            white_color->blue,1.0,exception);
          (void) SetImageChannelMask(image,channel_mask);
        }
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          channel_mask=SetImageChannelMask(image,BlackChannel);
          status&=(MagickStatusType) LevelizeImage(image,black_color->black,
            white_color->black,1.0,exception);
          (void) SetImageChannelMask(image,channel_mask);
        }
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
          (image->alpha_trait != UndefinedPixelTrait))
        {
          channel_mask=SetImageChannelMask(image,AlphaChannel);
          status&=(MagickStatusType) LevelizeImage(image,black_color->alpha,
            white_color->alpha,1.0,exception);
          (void) SetImageChannelMask(image,channel_mask);
        }
    }
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     L i n e a r S t r e t c h I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LinearStretchImage() discards any pixels below the black point and above
%  the white point and levels the remaining pixels.
%
%  The format of the LinearStretchImage method is:
%
%      MagickBooleanType LinearStretchImage(Image *image,
%        const double black_point,const double white_point,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o black_point: the black point.
%
%    o white_point: the white point.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType LinearStretchImage(Image *image,
  const double black_point,const double white_point,ExceptionInfo *exception)
{
#define LinearStretchImageTag  "LinearStretch/Image"

  CacheView
    *image_view;

  char
    property[MagickPathExtent];

  double
    *histogram,
    intensity;

  MagickBooleanType
    status;

  ssize_t
    black,
    white,
    y;

  /*
    Allocate histogram and linear map.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  histogram=(double *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*histogram));
  if (histogram == (double *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Form histogram.
  */
  (void) memset(histogram,0,(MaxMap+1)*sizeof(*histogram));
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      intensity=GetPixelIntensity(image,p);
      histogram[ScaleQuantumToMap(ClampToQuantum(intensity))]++;
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  image_view=DestroyCacheView(image_view);
  /*
    Find the histogram boundaries by locating the black and white point levels.
  */
  intensity=0.0;
  for (black=0; black < (ssize_t) MaxMap; black++)
  {
    intensity+=histogram[black];
    if (intensity >= black_point)
      break;
  }
  intensity=0.0;
  for (white=(ssize_t) MaxMap; white != 0; white--)
  {
    intensity+=histogram[white];
    if (intensity >= white_point)
      break;
  }
  histogram=(double *) RelinquishMagickMemory(histogram);
  status=LevelImage(image,(double) ScaleMapToQuantum((MagickRealType) black),
    (double) ScaleMapToQuantum((MagickRealType) white),1.0,exception);
  (void) FormatLocaleString(property,MagickPathExtent,"%gx%g%%",100.0*black/
    MaxMap,100.0*white/MaxMap);
  (void) SetImageProperty(image,"histogram:linear-stretch",property,exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o d u l a t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ModulateImage() lets you control the brightness, saturation, and hue
%  of an image.  Modulate represents the brightness, saturation, and hue
%  as one parameter (e.g. 90,150,100).  If the image colorspace is HSL, the
%  modulation is lightness, saturation, and hue.  For HWB, use blackness,
%  whiteness, and hue. And for HCL, use chrome, luma, and hue.
%
%  The format of the ModulateImage method is:
%
%      MagickBooleanType ModulateImage(Image *image,const char *modulate,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o modulate: Define the percent change in brightness, saturation, and hue.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline void ModulateHCL(const double percent_hue,
  const double percent_chroma,const double percent_luma,double *red,
  double *green,double *blue)
{
  double
    hue,
    luma,
    chroma;

  /*
    Increase or decrease color luma, chroma, or hue.
  */
  ConvertRGBToHCL(*red,*green,*blue,&hue,&chroma,&luma);
  hue+=fmod((percent_hue-100.0),200.0)/200.0;
  chroma*=0.01*percent_chroma;
  luma*=0.01*percent_luma;
  ConvertHCLToRGB(hue,chroma,luma,red,green,blue);
}

static inline void ModulateHCLp(const double percent_hue,
  const double percent_chroma,const double percent_luma,double *red,
  double *green,double *blue)
{
  double
    hue,
    luma,
    chroma;

  /*
    Increase or decrease color luma, chroma, or hue.
  */
  ConvertRGBToHCLp(*red,*green,*blue,&hue,&chroma,&luma);
  hue+=fmod((percent_hue-100.0),200.0)/200.0;
  chroma*=0.01*percent_chroma;
  luma*=0.01*percent_luma;
  ConvertHCLpToRGB(hue,chroma,luma,red,green,blue);
}

static inline void ModulateHSB(const double percent_hue,
  const double percent_saturation,const double percent_brightness,double *red,
  double *green,double *blue)
{
  double
    brightness,
    hue,
    saturation;

  /*
    Increase or decrease color brightness, saturation, or hue.
  */
  ConvertRGBToHSB(*red,*green,*blue,&hue,&saturation,&brightness);
  hue+=fmod((percent_hue-100.0),200.0)/200.0;
  saturation*=0.01*percent_saturation;
  brightness*=0.01*percent_brightness;
  ConvertHSBToRGB(hue,saturation,brightness,red,green,blue);
}

static inline void ModulateHSI(const double percent_hue,
  const double percent_saturation,const double percent_intensity,double *red,
  double *green,double *blue)
{
  double
    intensity,
    hue,
    saturation;

  /*
    Increase or decrease color intensity, saturation, or hue.
  */
  ConvertRGBToHSI(*red,*green,*blue,&hue,&saturation,&intensity);
  hue+=fmod((percent_hue-100.0),200.0)/200.0;
  saturation*=0.01*percent_saturation;
  intensity*=0.01*percent_intensity;
  ConvertHSIToRGB(hue,saturation,intensity,red,green,blue);
}

static inline void ModulateHSL(const double percent_hue,
  const double percent_saturation,const double percent_lightness,double *red,
  double *green,double *blue)
{
  double
    hue,
    lightness,
    saturation;

  /*
    Increase or decrease color lightness, saturation, or hue.
  */
  ConvertRGBToHSL(*red,*green,*blue,&hue,&saturation,&lightness);
  hue+=fmod((percent_hue-100.0),200.0)/200.0;
  saturation*=0.01*percent_saturation;
  lightness*=0.01*percent_lightness;
  ConvertHSLToRGB(hue,saturation,lightness,red,green,blue);
}

static inline void ModulateHSV(const double percent_hue,
  const double percent_saturation,const double percent_value,double *red,
  double *green,double *blue)
{
  double
    hue,
    saturation,
    value;

  /*
    Increase or decrease color value, saturation, or hue.
  */
  ConvertRGBToHSV(*red,*green,*blue,&hue,&saturation,&value);
  hue+=fmod((percent_hue-100.0),200.0)/200.0;
  saturation*=0.01*percent_saturation;
  value*=0.01*percent_value;
  ConvertHSVToRGB(hue,saturation,value,red,green,blue);
}

static inline void ModulateHWB(const double percent_hue,
  const double percent_whiteness,const double percent_blackness,double *red,
  double *green,double *blue)
{
  double
    blackness,
    hue,
    whiteness;

  /*
    Increase or decrease color blackness, whiteness, or hue.
  */
  ConvertRGBToHWB(*red,*green,*blue,&hue,&whiteness,&blackness);
  hue+=fmod((percent_hue-100.0),200.0)/200.0;
  blackness*=0.01*percent_blackness;
  whiteness*=0.01*percent_whiteness;
  ConvertHWBToRGB(hue,whiteness,blackness,red,green,blue);
}

static inline void ModulateLCHab(const double percent_luma,
  const double percent_chroma,const double percent_hue,
  const IlluminantType illuminant,double *red,double *green,double *blue)
{
  double
    hue,
    luma,
    chroma;

  /*
    Increase or decrease color luma, chroma, or hue.
  */
  ConvertRGBToLCHab(*red,*green,*blue,illuminant,&luma,&chroma,&hue);
  luma*=0.01*percent_luma;
  chroma*=0.01*percent_chroma;
  hue+=fmod((percent_hue-100.0),200.0)/200.0;
  ConvertLCHabToRGB(luma,chroma,hue,illuminant,red,green,blue);
}

static inline void ModulateLCHuv(const double percent_luma,
  const double percent_chroma,const double percent_hue,
  const IlluminantType illuminant,double *red,double *green,double *blue)
{
  double
    hue,
    luma,
    chroma;

  /*
    Increase or decrease color luma, chroma, or hue.
  */
  ConvertRGBToLCHuv(*red,*green,*blue,illuminant,&luma,&chroma,&hue);
  luma*=0.01*percent_luma;
  chroma*=0.01*percent_chroma;
  hue+=fmod((percent_hue-100.0),200.0)/200.0;
  ConvertLCHuvToRGB(luma,chroma,hue,illuminant,red,green,blue);
}

MagickExport MagickBooleanType ModulateImage(Image *image,const char *modulate,
  ExceptionInfo *exception)
{
#define ModulateImageTag  "Modulate/Image"

  CacheView
    *image_view;

  ColorspaceType
    colorspace = UndefinedColorspace;

  const char
    *artifact;

  double
    percent_brightness = 100.0,
    percent_hue = 100.0,
    percent_saturation = 100.0;

  GeometryInfo
    geometry_info;

  IlluminantType
    illuminant = D65Illuminant;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickStatusType
    flags;

  ssize_t
    i;

  ssize_t
    y;

  /*
    Initialize modulate table.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (modulate == (char *) NULL)
    return(MagickFalse);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  flags=ParseGeometry(modulate,&geometry_info);
  if ((flags & RhoValue) != 0)
    percent_brightness=geometry_info.rho;
  if ((flags & SigmaValue) != 0)
    percent_saturation=geometry_info.sigma;
  if ((flags & XiValue) != 0)
    percent_hue=geometry_info.xi;
  artifact=GetImageArtifact(image,"modulate:colorspace");
  if (artifact != (const char *) NULL)
    colorspace=(ColorspaceType) ParseCommandOption(MagickColorspaceOptions,
      MagickFalse,artifact);
  artifact=GetImageArtifact(image,"color:illuminant");
  if (artifact != (const char *) NULL)
    {
      ssize_t
        illuminant_type;

      illuminant_type=ParseCommandOption(MagickIlluminantOptions,MagickFalse,
        artifact);
      if (illuminant_type < 0)
        {
          illuminant=UndefinedIlluminant;
          colorspace=UndefinedColorspace;
        }
      else
        illuminant=(IlluminantType) illuminant_type;
    }
  if (image->storage_class == PseudoClass)
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      double
        blue,
        green,
        red;

      /*
        Modulate image colormap.
      */
      red=(double) image->colormap[i].red;
      green=(double) image->colormap[i].green;
      blue=(double) image->colormap[i].blue;
      switch (colorspace)
      {
        case HCLColorspace:
        {
          ModulateHCL(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HCLpColorspace:
        {
          ModulateHCLp(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HSBColorspace:
        {
          ModulateHSB(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HSIColorspace:
        {
          ModulateHSI(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HSLColorspace:
        default:
        {
          ModulateHSL(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HSVColorspace:
        {
          ModulateHSV(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HWBColorspace:
        {
          ModulateHWB(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case LCHColorspace:
        case LCHabColorspace:
        {
          ModulateLCHab(percent_brightness,percent_saturation,percent_hue,
            illuminant,&red,&green,&blue);
          break;
        }
        case LCHuvColorspace:
        {
          ModulateLCHuv(percent_brightness,percent_saturation,percent_hue,
            illuminant,&red,&green,&blue);
          break;
        }
      }
      image->colormap[i].red=red;
      image->colormap[i].green=green;
      image->colormap[i].blue=blue;
    }
  /*
    Modulate image.
  */
#if defined(MAGICKCORE_OPENCL_SUPPORT)
  if (AccelerateModulateImage(image,percent_brightness,percent_hue,
        percent_saturation,colorspace,exception) != MagickFalse)
    return(MagickTrue);
#endif
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        blue,
        green,
        red;

      red=(double) GetPixelRed(image,q);
      green=(double) GetPixelGreen(image,q);
      blue=(double) GetPixelBlue(image,q);
      switch (colorspace)
      {
        case HCLColorspace:
        {
          ModulateHCL(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HCLpColorspace:
        {
          ModulateHCLp(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HSBColorspace:
        {
          ModulateHSB(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HSIColorspace:
        {
          ModulateHSI(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HSLColorspace:
        default:
        {
          ModulateHSL(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HSVColorspace:
        {
          ModulateHSV(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case HWBColorspace:
        {
          ModulateHWB(percent_hue,percent_saturation,percent_brightness,
            &red,&green,&blue);
          break;
        }
        case LCHColorspace:
        case LCHabColorspace:
        {
          ModulateLCHab(percent_brightness,percent_saturation,percent_hue,
            illuminant,&red,&green,&blue);
          break;
        }
        case LCHuvColorspace:
        {
          ModulateLCHuv(percent_brightness,percent_saturation,percent_hue,
            illuminant,&red,&green,&blue);
          break;
        }
      }
      SetPixelRed(image,ClampToQuantum(red),q);
      SetPixelGreen(image,ClampToQuantum(green),q);
      SetPixelBlue(image,ClampToQuantum(blue),q);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,ModulateImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     N e g a t e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NegateImage() negates the colors in the reference image.  The grayscale
%  option means that only grayscale values within the image are negated.
%
%  The format of the NegateImage method is:
%
%      MagickBooleanType NegateImage(Image *image,
%        const MagickBooleanType grayscale,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o grayscale: If MagickTrue, only negate grayscale pixels within the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType NegateImage(Image *image,
  const MagickBooleanType grayscale,ExceptionInfo *exception)
{
#define NegateImageTag  "Negate/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    i;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      /*
        Negate colormap.
      */
      if (grayscale != MagickFalse)
        if ((image->colormap[i].red != image->colormap[i].green) ||
            (image->colormap[i].green != image->colormap[i].blue))
          continue;
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].red=(double) QuantumRange-image->colormap[i].red;
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].green=(double) QuantumRange-image->colormap[i].green;
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        image->colormap[i].blue=(double) QuantumRange-image->colormap[i].blue;
    }
  /*
    Negate image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
  if( grayscale != MagickFalse )
    {
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        MagickBooleanType
          sync;

        Quantum
          *magick_restrict q;

        ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          ssize_t
            j;

          if (IsPixelGray(image,q) == MagickFalse)
            {
              q+=(ptrdiff_t) GetPixelChannels(image);
              continue;
            }
          for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,j);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            q[j]=QuantumRange-q[j];
          }
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

            progress++;
            proceed=SetImageProgress(image,NegateImageTag,progress,image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      image_view=DestroyCacheView(image_view);
      return(MagickTrue);
    }
  /*
    Negate image.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[j]=QuantumRange-q[j];
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,NegateImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     N o r m a l i z e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The NormalizeImage() method enhances the contrast of a color image by
%  mapping the darkest 2 percent of all pixel to black and the brightest
%  1 percent to white.
%
%  The format of the NormalizeImage method is:
%
%      MagickBooleanType NormalizeImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType NormalizeImage(Image *image,
  ExceptionInfo *exception)
{
  double
    black_point,
    white_point;

  black_point=0.02*image->columns*image->rows;
  white_point=0.99*image->columns*image->rows;
  return(ContrastStretchImage(image,black_point,white_point,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S i g m o i d a l C o n t r a s t I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SigmoidalContrastImage() adjusts the contrast of an image with a non-linear
%  sigmoidal contrast algorithm.  Increase the contrast of the image using a
%  sigmoidal transfer function without saturating highlights or shadows.
%  Contrast indicates how much to increase the contrast (0 is none; 3 is
%  typical; 20 is pushing it); mid-point indicates where midtones fall in the
%  resultant image (0 is white; 50% is middle-gray; 100% is black).  Set
%  sharpen to MagickTrue to increase the image contrast otherwise the contrast
%  is reduced.
%
%  The format of the SigmoidalContrastImage method is:
%
%      MagickBooleanType SigmoidalContrastImage(Image *image,
%        const MagickBooleanType sharpen,const char *levels,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o sharpen: Increase or decrease image contrast.
%
%    o contrast: strength of the contrast, the larger the number the more
%      'threshold-like' it becomes.
%
%    o midpoint: midpoint of the function as a color value 0 to QuantumRange.
%
%    o exception: return any errors or warnings in this structure.
%
*/

/*
  ImageMagick 6 has a version of this function which uses LUTs.
*/

/*
  Sigmoidal function Sigmoidal with inflexion point moved to b and "slope
  constant" set to a.

  The first version, based on the hyperbolic tangent tanh, when combined with
  the scaling step, is an exact arithmetic clone of the sigmoid function
  based on the logistic curve. The equivalence is based on the identity

    1/(1+exp(-t)) = (1+tanh(t/2))/2

  (http://de.wikipedia.org/wiki/Sigmoidfunktion) and the fact that the
  scaled sigmoidal derivation is invariant under affine transformations of
  the ordinate.

  The tanh version is almost certainly more accurate and cheaper.  The 0.5
  factor in the argument is to clone the legacy ImageMagick behavior. The
  reason for making the define depend on atanh even though it only uses tanh
  has to do with the construction of the inverse of the scaled sigmoidal.
*/
#if defined(MAGICKCORE_HAVE_ATANH)
#define Sigmoidal(a,b,x) ( tanh((0.5*(a))*((x)-(b))) )
#else
#define Sigmoidal(a,b,x) ( 1.0/(1.0+exp((a)*((b)-(x)))) )
#endif
/*
  Scaled sigmoidal function:

    ( Sigmoidal(a,b,x) - Sigmoidal(a,b,0) ) /
    ( Sigmoidal(a,b,1) - Sigmoidal(a,b,0) )

  See http://osdir.com/ml/video.image-magick.devel/2005-04/msg00006.html and
  http://www.cs.dartmouth.edu/farid/downloads/tutorials/fip.pdf.  The limit
  of ScaledSigmoidal as a->0 is the identity, but a=0 gives a division by
  zero. This is fixed below by exiting immediately when contrast is small,
  leaving the image (or colormap) unmodified. This appears to be safe because
  the series expansion of the logistic sigmoidal function around x=b is

  1/2-a*(b-x)/4+...

  so that the key denominator s(1)-s(0) is about a/4 (a/2 with tanh).
*/
#define ScaledSigmoidal(a,b,x) (                    \
  (Sigmoidal((a),(b),(x))-Sigmoidal((a),(b),0.0)) / \
  (Sigmoidal((a),(b),1.0)-Sigmoidal((a),(b),0.0)) )
/*
  Inverse of ScaledSigmoidal, used for +sigmoidal-contrast.  Because b
  may be 0 or 1, the argument of the hyperbolic tangent (resp. logistic
  sigmoidal) may be outside of the interval (-1,1) (resp. (0,1)), even
  when creating a LUT from in gamut values, hence the branching.  In
  addition, HDRI may have out of gamut values.
  InverseScaledSigmoidal is not a two-sided inverse of ScaledSigmoidal:
  It is only a right inverse. This is unavoidable.
*/
static inline double InverseScaledSigmoidal(const double a,const double b,
  const double x)
{
  const double sig0=Sigmoidal(a,b,0.0);
  const double sig1=Sigmoidal(a,b,1.0);
  const double argument=(sig1-sig0)*x+sig0;
  const double clamped=
    (
#if defined(MAGICKCORE_HAVE_ATANH)
      argument < -1+MagickEpsilon
      ?
      -1+MagickEpsilon
      :
      ( argument > 1-MagickEpsilon ? 1-MagickEpsilon : argument )
    );
  return(b+(2.0/a)*atanh(clamped));
#else
      argument < MagickEpsilon
      ?
      MagickEpsilon
      :
      ( argument > 1-MagickEpsilon ? 1-MagickEpsilon : argument )
    );
  return(b-log(1.0/clamped-1.0)/a);
#endif
}

MagickExport MagickBooleanType SigmoidalContrastImage(Image *image,
  const MagickBooleanType sharpen,const double contrast,const double midpoint,
  ExceptionInfo *exception)
{
#define SigmoidalContrastImageTag  "SigmoidalContrast/Image"
#define ScaledSig(x) (ClampToQuantum((double) QuantumRange* \
  ScaledSigmoidal(contrast,QuantumScale*midpoint,QuantumScale*((double) x))) )
#define InverseScaledSig(x) (ClampToQuantum((double) QuantumRange* \
  InverseScaledSigmoidal(contrast,QuantumScale*midpoint,QuantumScale* \
  ((double) x))) )

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  /*
    Convenience macros.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  /*
    Side effect: may clamp values unless contrast<MagickEpsilon, in which
    case nothing is done.
  */
  if (contrast < MagickEpsilon)
    return(MagickTrue);
  /*
    Sigmoidal-contrast enhance colormap.
  */
  if (image->storage_class == PseudoClass)
    {
      ssize_t
        i;

      if( sharpen != MagickFalse )
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
            image->colormap[i].red=(MagickRealType) ScaledSig(
              image->colormap[i].red);
          if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
            image->colormap[i].green=(MagickRealType) ScaledSig(
              image->colormap[i].green);
          if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
            image->colormap[i].blue=(MagickRealType) ScaledSig(
              image->colormap[i].blue);
          if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
            image->colormap[i].alpha=(MagickRealType) ScaledSig(
              image->colormap[i].alpha);
        }
      else
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
            image->colormap[i].red=(MagickRealType) InverseScaledSig(
              image->colormap[i].red);
          if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
            image->colormap[i].green=(MagickRealType) InverseScaledSig(
              image->colormap[i].green);
          if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
            image->colormap[i].blue=(MagickRealType) InverseScaledSig(
              image->colormap[i].blue);
          if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
            image->colormap[i].alpha=(MagickRealType) InverseScaledSig(
              image->colormap[i].alpha);
        }
    }
  /*
    Sigmoidal-contrast enhance image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if( sharpen != MagickFalse )
          q[i]=ScaledSig(q[i]);
        else
          q[i]=InverseScaledSig(q[i]);
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SigmoidalContrastImageTag,progress,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     W h i t e B a l a n c e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WhiteBalanceImage() applies white balancing to an image according to a
%  grayworld assumption in the LAB colorspace.
%
%  The format of the WhiteBalanceImage method is:
%
%      MagickBooleanType WhiteBalanceImage(Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image to auto-level
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType WhiteBalanceImage(Image *image,
  ExceptionInfo *exception)
{
#define WhiteBalanceImageTag  "WhiteBalance/Image"

  CacheView
    *image_view;

  const char
    *artifact;

  double
    a_mean,
    b_mean;

  MagickOffsetType
    progress;

  MagickStatusType
    status;

  ssize_t
    y;

  /*
    White balance image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  status=TransformImageColorspace(image,LabColorspace,exception);
  a_mean=0.0;
  b_mean=0.0;
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      a_mean+=QuantumScale*(double) GetPixela(image,p)-0.5;
      b_mean+=QuantumScale*(double) GetPixelb(image,p)-0.5;
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  a_mean/=((double) image->columns*image->rows);
  b_mean/=((double) image->columns*image->rows);
  progress=0;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        a,
        b;

      /*
        Scale the chroma distance shifted according to amount of luminance.
      */
      a=(double) GetPixela(image,q)-1.1*(double) GetPixelL(image,q)*a_mean;
      b=(double) GetPixelb(image,q)-1.1*(double) GetPixelL(image,q)*b_mean;
      SetPixela(image,ClampToQuantum(a),q);
      SetPixelb(image,ClampToQuantum(b),q);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,WhiteBalanceImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  artifact=GetImageArtifact(image,"white-balance:vibrance");
  if (artifact != (const char *) NULL)
    {
      ChannelType
        channel_mask;

      double
        black_point = 0.0;

      GeometryInfo
        geometry_info;

      MagickStatusType
        flags;

      /*
        Level the a & b channels.
      */
      flags=ParseGeometry(artifact,&geometry_info);
      if ((flags & RhoValue) != 0)
        black_point=geometry_info.rho;
      if ((flags & PercentValue) != 0)
        black_point*=((double) QuantumRange/100.0);
      channel_mask=SetImageChannelMask(image,(ChannelType) (aChannel |
        bChannel));
      status&=(MagickStatusType) LevelImage(image,black_point,(double)
        QuantumRange-black_point,1.0,exception);
      (void) SetImageChannelMask(image,channel_mask);
    }
  status&=(MagickStatusType) TransformImageColorspace(image,sRGBColorspace,
    exception);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%       TTTTT  H   H  RRRR   EEEEE  SSSSS  H   H   OOO   L      DDDD          %
%         T    H   H  R   R  E      SS     H   H  O   O  L      D   D         %
%         T    HHHHH  RRRR   EEE     SSS   HHHHH  O   O  L      D   D         %
%         T    H   H  R R    E         SS  H   H  O   O  L      D   D         %
%         T    H   H  R  R   EEEEE  SSSSS  H   H   OOO   LLLLL  DDDD          %
%                                                                             %
%                                                                             %
%                      MagickCore Image Threshold Methods                     %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                 October 1996                                %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/configure.h"
#include "magick/constitute.h"
#include "magick/decorate.h"
#include "magick/draw.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/effect.h"
#include "magick/fx.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/montage.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/quantize.h"
#include "magick/quantum.h"
#include "magick/random_.h"
#include "magick/random-private.h"
#include "magick/resize.h"
#include "magick/resource_.h"
#include "magick/segment.h"
#include "magick/shear.h"
#include "magick/signature-private.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/thread-private.h"
#include "magick/threshold.h"
#include "magick/transform.h"
#include "magick/xml-tree.h"

/*
  Define declarations.
*/
#define ThresholdsFilename  "thresholds.xml"

/*
  Typedef declarations.
*/
struct _ThresholdMap
{
  char
    *map_id,
    *description;

  size_t
    width,
    height;

  ssize_t
    divisor,
    *levels;
};

/*
  Static declarations.
*/
static const char
  *MinimalThresholdMap =
    "<?xml version=\"1.0\"?>"
    "<thresholds>"
    "  <threshold map=\"threshold\" alias=\"1x1\">"
    "    <description>Threshold 1x1 (non-dither)</description>"
    "    <levels width=\"1\" height=\"1\" divisor=\"2\">"
    "        1"
    "    </levels>"
    "  </threshold>"
    "  <threshold map=\"checks\" alias=\"2x1\">"
    "    <description>Checkerboard 2x1 (dither)</description>"
    "    <levels width=\"2\" height=\"2\" divisor=\"3\">"
    "       1 2"
    "       2 1"
    "    </levels>"
    "  </threshold>"
    "</thresholds>";

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A d a p t i v e T h r e s h o l d I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AdaptiveThresholdImage() selects an individual threshold for each pixel
%  based on the range of intensity values in its local neighborhood.  This
%  allows for thresholding of an image whose global intensity histogram
%  doesn't contain distinctive peaks.
%
%  The format of the AdaptiveThresholdImage method is:
%
%      Image *AdaptiveThresholdImage(const Image *image,
%        const size_t width,const size_t height,
%        const ssize_t offset,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o width: the width of the local neighborhood.
%
%    o height: the height of the local neighborhood.
%
%    o offset: the mean offset.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AdaptiveThresholdImage(const Image *image,
  const size_t width,const size_t height,const ssize_t offset,
  ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view,
    *threshold_view;

  Image
    *threshold_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    zero;

  MagickRealType
    number_pixels;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  threshold_image=CloneImage(image,0,0,MagickTrue,exception);
  if (threshold_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(threshold_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&threshold_image->exception);
      threshold_image=DestroyImage(threshold_image);
      return((Image *) NULL);
    }
  /*
    Local adaptive threshold.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&zero);
  number_pixels=(MagickRealType) (width*height);
  image_view=AcquireVirtualCacheView(image,exception);
  threshold_view=AcquireAuthenticCacheView(threshold_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,threshold_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    MagickPixelPacket
      channel_bias,
      channel_sum;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p,
      *restrict r;

    register IndexPacket
      *restrict threshold_indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    ssize_t
      u,
      v;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y-(ssize_t)
      height/2L,image->columns+width,height,exception);
    q=GetCacheViewAuthenticPixels(threshold_view,0,y,threshold_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    threshold_indexes=GetCacheViewAuthenticIndexQueue(threshold_view);
    channel_bias=zero;
    channel_sum=zero;
    r=p;
    for (v=0; v < (ssize_t) height; v++)
    {
      for (u=0; u < (ssize_t) width; u++)
      {
        if (u == (ssize_t) (width-1))
          {
            channel_bias.red+=r[u].red;
            channel_bias.green+=r[u].green;
            channel_bias.blue+=r[u].blue;
            channel_bias.opacity+=r[u].opacity;
            if (image->colorspace == CMYKColorspace)
              channel_bias.index=(MagickRealType)
                GetPixelIndex(indexes+(r-p)+u);
          }
        channel_sum.red+=r[u].red;
        channel_sum.green+=r[u].green;
        channel_sum.blue+=r[u].blue;
        channel_sum.opacity+=r[u].opacity;
        if (image->colorspace == CMYKColorspace)
          channel_sum.index=(MagickRealType) GetPixelIndex(indexes+(r-p)+u);
      }
      r+=image->columns+width;
    }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickPixelPacket
        mean;

      mean=zero;
      r=p;
      channel_sum.red-=channel_bias.red;
      channel_sum.green-=channel_bias.green;
      channel_sum.blue-=channel_bias.blue;
      channel_sum.opacity-=channel_bias.opacity;
      channel_sum.index-=channel_bias.index;
      channel_bias=zero;
      for (v=0; v < (ssize_t) height; v++)
      {
        channel_bias.red+=r[0].red;
        channel_bias.green+=r[0].green;
        channel_bias.blue+=r[0].blue;
        channel_bias.opacity+=r[0].opacity;
        if (image->colorspace == CMYKColorspace)
          channel_bias.index=(MagickRealType) GetPixelIndex(indexes+x+(r-p)+0);
        channel_sum.red+=r[width-1].red;
        channel_sum.green+=r[width-1].green;
        channel_sum.blue+=r[width-1].blue;
        channel_sum.opacity+=r[width-1].opacity;
        if (image->colorspace == CMYKColorspace)
          channel_sum.index=(MagickRealType) GetPixelIndex(indexes+x+(r-p)+
            width-1);
        r+=image->columns+width;
      }
      mean.red=(MagickRealType) (channel_sum.red/number_pixels+offset);
      mean.green=(MagickRealType) (channel_sum.green/number_pixels+offset);
      mean.blue=(MagickRealType) (channel_sum.blue/number_pixels+offset);
      mean.opacity=(MagickRealType) (channel_sum.opacity/number_pixels+offset);
      if (image->colorspace == CMYKColorspace)
        mean.index=(MagickRealType) (channel_sum.index/number_pixels+offset);
      SetPixelRed(q,((MagickRealType) GetPixelRed(q) <= mean.red) ?
        0 : QuantumRange);
      SetPixelGreen(q,((MagickRealType) GetPixelGreen(q) <= mean.green) ?
        0 : QuantumRange);
      SetPixelBlue(q,((MagickRealType) GetPixelBlue(q) <= mean.blue) ?
        0 : QuantumRange);
      SetPixelOpacity(q,((MagickRealType) GetPixelOpacity(q) <= mean.opacity) ?
        0 : QuantumRange);
      if (image->colorspace == CMYKColorspace)
        SetPixelIndex(threshold_indexes+x,(((MagickRealType) GetPixelIndex(
          threshold_indexes+x) <= mean.index) ? 0 : QuantumRange));
      p++;
      q++;
    }
    sync=SyncCacheViewAuthenticPixels(threshold_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_AdaptiveThresholdImage)
#endif
        proceed=SetImageProgress(image,ThresholdImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  threshold_view=DestroyCacheView(threshold_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    threshold_image=DestroyImage(threshold_image);
  return(threshold_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     B i l e v e l I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BilevelImage() changes the value of individual pixels based on the
%  intensity of each pixel channel.  The result is a high-contrast image.
%
%  More precisely each channel value of the image is 'thresholded' so that if
%  it is equal to or less than the given value it is set to zero, while any
%  value greater than that give is set to it maximum or QuantumRange.
%
%  This function is what is used to implement the "-threshold" operator for
%  the command line API.
%
%  If the default channel setting is given the image is thresholded using just
%  the gray 'intensity' of the image, rather than the individual channels.
%
%  The format of the BilevelImageChannel method is:
%
%      MagickBooleanType BilevelImage(Image *image,const double threshold)
%      MagickBooleanType BilevelImageChannel(Image *image,
%        const ChannelType channel,const double threshold)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o threshold: define the threshold values.
%
%  Aside: You can get the same results as operator using LevelImageChannels()
%  with the 'threshold' value for both the black_point and the white_point.
%
*/

MagickExport MagickBooleanType BilevelImage(Image *image,const double threshold)
{
  MagickBooleanType
    status;

  status=BilevelImageChannel(image,DefaultChannels,threshold);
  return(status);
}

MagickExport MagickBooleanType BilevelImageChannel(Image *image,
  const ChannelType channel,const double threshold)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) SetImageColorspace(image,sRGBColorspace);
  /*
    Bilevel threshold image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    if ((channel & SyncChannels) != 0)
      {
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRed(q,GetPixelIntensity(image,q) <= threshold ? 0 :
            QuantumRange);
          SetPixelGreen(q,GetPixelRed(q));
          SetPixelBlue(q,GetPixelRed(q));
          q++;
        }
      }
    else
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        if ((channel & RedChannel) != 0)
          SetPixelRed(q,(MagickRealType) GetPixelRed(q) <= threshold ? 0 :
            QuantumRange);
        if ((channel & GreenChannel) != 0)
          SetPixelGreen(q,(MagickRealType) GetPixelGreen(q) <= threshold ? 0 :
            QuantumRange);
        if ((channel & BlueChannel) != 0)
          SetPixelBlue(q,(MagickRealType) GetPixelBlue(q) <= threshold ? 0 :
            QuantumRange);
        if ((channel & OpacityChannel) != 0)
          {
            if (image->matte == MagickFalse)
              SetPixelOpacity(q,(MagickRealType) GetPixelOpacity(q) <=
                threshold ? 0 : QuantumRange);
            else
              SetPixelOpacity(q,(MagickRealType) GetPixelOpacity(q) <=
                threshold ? OpaqueOpacity : TransparentOpacity);
          }
        if (((channel & IndexChannel) != 0) &&
            (image->colorspace == CMYKColorspace))
          SetPixelIndex(indexes+x,(MagickRealType) GetPixelIndex(indexes+x) <=
            threshold ? 0 : QuantumRange);
        q++;
      }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_BilevelImageChannel)
#endif
        proceed=SetImageProgress(image,ThresholdImageTag,progress++,
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
%     B l a c k T h r e s h o l d I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BlackThresholdImage() is like ThresholdImage() but forces all pixels below
%  the threshold into black while leaving all pixels at or above the threshold
%  unchanged.
%
%  The format of the BlackThresholdImage method is:
%
%      MagickBooleanType BlackThresholdImage(Image *image,const char *threshold)
%      MagickBooleanType BlackThresholdImageChannel(Image *image,
%        const ChannelType channel,const char *threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
%
%    o threshold: Define the threshold value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType BlackThresholdImage(Image *image,
  const char *threshold)
{
  MagickBooleanType
    status;

  status=BlackThresholdImageChannel(image,DefaultChannels,threshold,
    &image->exception);
  return(status);
}

MagickExport MagickBooleanType BlackThresholdImageChannel(Image *image,
  const ChannelType channel,const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    threshold;

  MagickStatusType
    flags;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  GetMagickPixelPacket(image,&threshold);
  flags=ParseGeometry(thresholds,&geometry_info);
  threshold.red=geometry_info.rho;
  threshold.green=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    threshold.green=threshold.red;
  threshold.blue=geometry_info.xi;
  if ((flags & XiValue) == 0)
    threshold.blue=threshold.red;
  threshold.opacity=geometry_info.psi;
  if ((flags & PsiValue) == 0)
    threshold.opacity=threshold.red;
  threshold.index=geometry_info.chi;
  if ((flags & ChiValue) == 0)
    threshold.index=threshold.red;
  if ((flags & PercentValue) != 0)
    {
      threshold.red*=(MagickRealType) (QuantumRange/100.0);
      threshold.green*=(MagickRealType) (QuantumRange/100.0);
      threshold.blue*=(MagickRealType) (QuantumRange/100.0);
      threshold.opacity*=(MagickRealType) (QuantumRange/100.0);
      threshold.index*=(MagickRealType) (QuantumRange/100.0);
    }
  if ((IsMagickGray(&threshold) == MagickFalse) &&
      (IsGrayColorspace(image->colorspace) != MagickFalse))
    (void) SetImageColorspace(image,sRGBColorspace);
  /*
    Black threshold image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (((channel & RedChannel) != 0) &&
          ((MagickRealType) GetPixelRed(q) < threshold.red))
        SetPixelRed(q,0);
      if (((channel & GreenChannel) != 0) &&
          ((MagickRealType) GetPixelGreen(q) < threshold.green))
        SetPixelGreen(q,0);
      if (((channel & BlueChannel) != 0) &&
          ((MagickRealType) GetPixelBlue(q) < threshold.blue))
        SetPixelBlue(q,0);
      if (((channel & OpacityChannel) != 0) &&
          ((MagickRealType) GetPixelOpacity(q) < threshold.opacity))
        SetPixelOpacity(q,0);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace) &&
          ((MagickRealType) GetPixelIndex(indexes+x) < threshold.index))
        SetPixelIndex(indexes+x,0);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_BlackThresholdImageChannel)
#endif
        proceed=SetImageProgress(image,ThresholdImageTag,progress++,
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
%     C l a m p I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClampImage() set each pixel whose value is below zero to zero and any the
%  pixel whose value is above the quantum range to the quantum range (e.g.
%  65535) otherwise the pixel value remains unchanged.
%
%  The format of the ClampImageChannel method is:
%
%      MagickBooleanType ClampImage(Image *image)
%      MagickBooleanType ClampImageChannel(Image *image,
%        const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
*/

static inline Quantum ClampPixel(const MagickRealType value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) value);
#else
  if (value < 0.0f)
    return(0.0f);
  if (value >= (MagickRealType) QuantumRange)
    return((Quantum) QuantumRange);
  return(value);
#endif
}

MagickExport MagickBooleanType ClampImage(Image *image)
{
  MagickBooleanType
    status;

  status=ClampImageChannel(image,DefaultChannels);
  return(status);
}

MagickExport MagickBooleanType ClampImageChannel(Image *image,
  const ChannelType channel)
{
#define ClampImageTag  "Clamp/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    {
      register ssize_t
        i;

      register PixelPacket
        *restrict q;

      q=image->colormap;
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        SetPixelRed(q,ClampPixel(GetPixelRed(q)));
        SetPixelGreen(q,ClampPixel(GetPixelGreen(q)));
        SetPixelBlue(q,ClampPixel(GetPixelBlue(q)));
        SetPixelOpacity(q,ClampPixel(GetPixelOpacity(q)));
        q++;
      }
      return(SyncImage(image));
    }
  /*
    Clamp image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        SetPixelRed(q,ClampPixel(GetPixelRed(q)));
      if ((channel & GreenChannel) != 0)
        SetPixelGreen(q,ClampPixel(GetPixelGreen(q)));
      if ((channel & BlueChannel) != 0)
        SetPixelBlue(q,ClampPixel(GetPixelBlue(q)));
      if ((channel & OpacityChannel) != 0)
        SetPixelOpacity(q,ClampPixel(GetPixelOpacity(q)));
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelIndex(indexes+x,ClampPixel(GetPixelIndex(indexes+x)));
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_ClampImageChannel)
#endif
        proceed=SetImageProgress(image,ClampImageTag,progress++,image->rows);
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
%  D e s t r o y T h r e s h o l d M a p                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyThresholdMap() de-allocate the given ThresholdMap
%
%  The format of the ListThresholdMaps method is:
%
%      ThresholdMap *DestroyThresholdMap(Threshold *map)
%
%  A description of each parameter follows.
%
%    o map:    Pointer to the Threshold map to destroy
%
*/
MagickExport ThresholdMap *DestroyThresholdMap(ThresholdMap *map)
{
  assert(map != (ThresholdMap *) NULL);
  if (map->map_id != (char *) NULL)
    map->map_id=DestroyString(map->map_id);
  if (map->description != (char *) NULL)
    map->description=DestroyString(map->description);
  if (map->levels != (ssize_t *) NULL)
    map->levels=(ssize_t *) RelinquishMagickMemory(map->levels);
  map=(ThresholdMap *) RelinquishMagickMemory(map);
  return(map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  G e t T h r e s h o l d M a p F i l e                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetThresholdMapFile() look for a given threshold map name or alias in the
%  given XML file data, and return the allocated the map when found.
%
%  The format of the ListThresholdMaps method is:
%
%      ThresholdMap *GetThresholdMap(const char *xml,const char *filename,
%         const char *map_id,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o xml:  The threshold map list in XML format.
%
%    o filename:  The threshold map XML filename.
%
%    o map_id:  ID of the map to look for in XML list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ThresholdMap *GetThresholdMapFile(const char *xml,
  const char *filename,const char *map_id,ExceptionInfo *exception)
{
  const char
    *attribute,
    *content;

  double
    value;

  ThresholdMap
     *map;

  XMLTreeInfo
     *description,
     *levels,
     *threshold,
     *thresholds;

  map = (ThresholdMap *) NULL;
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading threshold map file \"%s\" ...",filename);
  thresholds=NewXMLTree(xml,exception);
  if ( thresholds == (XMLTreeInfo *) NULL )
    return(map);
  for (threshold = GetXMLTreeChild(thresholds,"threshold");
       threshold != (XMLTreeInfo *) NULL;
       threshold = GetNextXMLTreeTag(threshold) )
  {
    attribute=GetXMLTreeAttribute(threshold, "map");
    if ((attribute != (char *) NULL) && (LocaleCompare(map_id,attribute) == 0))
      break;
    attribute=GetXMLTreeAttribute(threshold, "alias");
    if ((attribute != (char *) NULL) && (LocaleCompare(map_id,attribute) == 0))
      break;
  }
  if (threshold == (XMLTreeInfo *) NULL)
    {
      thresholds=DestroyXMLTree(thresholds);
      return(map);
    }
  description=GetXMLTreeChild(threshold,"description");
  if (description == (XMLTreeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingElement", "<description>, map \"%s\"", map_id);
      thresholds=DestroyXMLTree(thresholds);
      return(map);
    }
  levels=GetXMLTreeChild(threshold,"levels");
  if (levels == (XMLTreeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingElement", "<levels>, map \"%s\"", map_id);
      thresholds=DestroyXMLTree(thresholds);
      return(map);
    }
  /*
    The map has been found -- allocate a Threshold Map to return
  */
  map=(ThresholdMap *) AcquireMagickMemory(sizeof(ThresholdMap));
  if (map == (ThresholdMap *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireThresholdMap");
  map->map_id=(char *) NULL;
  map->description=(char *) NULL;
  map->levels=(ssize_t *) NULL;
  /*
    Assign basic attributeibutes.
  */
  attribute=GetXMLTreeAttribute(threshold,"map");
  if (attribute != (char *) NULL)
    map->map_id=ConstantString(attribute);
  content=GetXMLTreeContent(description);
  if (content != (char *) NULL)
    map->description=ConstantString(content);
  attribute=GetXMLTreeAttribute(levels,"width");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels width>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->width=StringToUnsignedLong(attribute);
  if (map->width == 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
       "XmlInvalidAttribute", "<levels width>, map \"%s\"", map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  attribute=GetXMLTreeAttribute(levels,"height");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels height>, map \"%s\"", map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->height=StringToUnsignedLong(attribute);
  if (map->height == 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlInvalidAttribute", "<levels height>, map \"%s\"", map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  attribute=GetXMLTreeAttribute(levels, "divisor");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels divisor>, map \"%s\"", map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->divisor=(ssize_t) StringToLong(attribute);
  if (map->divisor < 2)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlInvalidAttribute", "<levels divisor>, map \"%s\"", map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  /*
    Allocate theshold levels array.
  */
  content=GetXMLTreeContent(levels);
  if (content == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingContent", "<levels>, map \"%s\"", map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->levels=(ssize_t *) AcquireQuantumMemory((size_t) map->width,map->height*
    sizeof(*map->levels));
  if (map->levels == (ssize_t *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireThresholdMap");
  {
    char
      *p;

    register ssize_t
      i;

    /*
      Parse levels into integer array.
    */
    for (i=0; i< (ssize_t) (map->width*map->height); i++)
    {
      map->levels[i]=(ssize_t) strtol(content,&p,10);
      if (p == content)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "XmlInvalidContent", "<level> too few values, map \"%s\"", map_id);
          thresholds=DestroyXMLTree(thresholds);
          map=DestroyThresholdMap(map);
          return(map);
        }
      if ((map->levels[i] < 0) || (map->levels[i] > map->divisor))
        {
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "XmlInvalidContent", "<level> %.20g out of range, map \"%s\"",
            (double) map->levels[i],map_id);
          thresholds=DestroyXMLTree(thresholds);
          map=DestroyThresholdMap(map);
          return(map);
        }
      content=p;
    }
    value=(double) strtol(content,&p,10);
    (void) value;
    if (p != content)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlInvalidContent", "<level> too many values, map \"%s\"", map_id);
       thresholds=DestroyXMLTree(thresholds);
       map=DestroyThresholdMap(map);
       return(map);
     }
  }
  thresholds=DestroyXMLTree(thresholds);
  return(map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t T h r e s h o l d M a p                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetThresholdMap() load and search one or more threshold map files for the
%  a map matching the given name or aliase.
%
%  The format of the GetThresholdMap method is:
%
%      ThresholdMap *GetThresholdMap(const char *map_id,
%         ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o map_id:  ID of the map to look for.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ThresholdMap *GetThresholdMap(const char *map_id,
  ExceptionInfo *exception)
{
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  ThresholdMap
    *map;

  map=GetThresholdMapFile(MinimalThresholdMap,"built-in",map_id,exception);
  if (map != (ThresholdMap *) NULL)
    return(map);
  options=GetConfigureOptions(ThresholdsFilename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    map=GetThresholdMapFile((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),map_id,exception);
    if (map != (ThresholdMap *) NULL)
      break;
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  return(map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  L i s t T h r e s h o l d M a p F i l e                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListThresholdMapFile() lists the threshold maps and their descriptions
%  in the given XML file data.
%
%  The format of the ListThresholdMaps method is:
%
%      MagickBooleanType ListThresholdMaps(FILE *file,const char*xml,
%         const char *filename,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to the output FILE.
%
%    o xml:  The threshold map list in XML format.
%
%    o filename:  The threshold map XML filename.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickBooleanType ListThresholdMapFile(FILE *file,const char *xml,
  const char *filename,ExceptionInfo *exception)
{
  XMLTreeInfo *thresholds,*threshold,*description;
  const char *map,*alias,*content;

  assert( xml != (char *) NULL );
  assert( file != (FILE *) NULL );

  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading threshold map file \"%s\" ...",filename);
  thresholds=NewXMLTree(xml,exception);
  if ( thresholds == (XMLTreeInfo *) NULL )
    return(MagickFalse);

  (void) FormatLocaleFile(file,"%-16s %-12s %s\n","Map","Alias","Description");
  (void) FormatLocaleFile(file,
    "----------------------------------------------------\n");

  for( threshold = GetXMLTreeChild(thresholds,"threshold");
       threshold != (XMLTreeInfo *) NULL;
       threshold = GetNextXMLTreeTag(threshold) )
  {
    map = GetXMLTreeAttribute(threshold, "map");
    if (map == (char *) NULL) {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<map>");
      thresholds=DestroyXMLTree(thresholds);
      return(MagickFalse);
    }
    alias = GetXMLTreeAttribute(threshold, "alias");
    /* alias is optional, no if test needed */
    description=GetXMLTreeChild(threshold,"description");
    if ( description == (XMLTreeInfo *) NULL ) {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingElement", "<description>, map \"%s\"", map);
      thresholds=DestroyXMLTree(thresholds);
      return(MagickFalse);
    }
    content=GetXMLTreeContent(description);
    if ( content == (char *) NULL ) {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingContent", "<description>, map \"%s\"", map);
      thresholds=DestroyXMLTree(thresholds);
      return(MagickFalse);
    }
    (void) FormatLocaleFile(file,"%-16s %-12s %s\n",map,alias ? alias : "",
      content);
  }
  thresholds=DestroyXMLTree(thresholds);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t T h r e s h o l d M a p s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListThresholdMaps() lists the threshold maps and their descriptions
%  as defined by "threshold.xml" to a file.
%
%  The format of the ListThresholdMaps method is:
%
%      MagickBooleanType ListThresholdMaps(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to the output FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ListThresholdMaps(FILE *file,
  ExceptionInfo *exception)
{
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  MagickStatusType
    status;

  status=MagickFalse;
  if ( file == (FILE *) NULL )
    file = stdout;
  options=GetConfigureOptions(ThresholdsFilename,exception);
  (void) FormatLocaleFile(file,
    "\n   Threshold Maps for Ordered Dither Operations\n");
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    (void) FormatLocaleFile(file,"\nPath: %s\n\n",GetStringInfoPath(option));
    status&=ListThresholdMapFile(file,(const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   O r d e r e d D i t h e r I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OrderedDitherImage() uses the ordered dithering technique of reducing color
%  images to monochrome using positional information to retain as much
%  information as possible.
%
%  WARNING: This function is deprecated, and is now just a call to
%  the more more powerful OrderedPosterizeImage(); function.
%
%  The format of the OrderedDitherImage method is:
%
%      MagickBooleanType OrderedDitherImage(Image *image)
%      MagickBooleanType OrderedDitherImageChannel(Image *image,
%        const ChannelType channel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType OrderedDitherImage(Image *image)
{
  MagickBooleanType
    status;

  status=OrderedDitherImageChannel(image,DefaultChannels,&image->exception);
  return(status);
}

MagickExport MagickBooleanType OrderedDitherImageChannel(Image *image,
 const ChannelType channel,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  /*
    Call the augumented function OrderedPosterizeImage()
  */
  status=OrderedPosterizeImageChannel(image,channel,"o8x8",exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     O r d e r e d P o s t e r i z e I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OrderedPosterizeImage() will perform a ordered dither based on a number
%  of pre-defined dithering threshold maps, but over multiple intensity
%  levels, which can be different for different channels, according to the
%  input argument.
%
%  The format of the OrderedPosterizeImage method is:
%
%      MagickBooleanType OrderedPosterizeImage(Image *image,
%        const char *threshold_map,ExceptionInfo *exception)
%      MagickBooleanType OrderedPosterizeImageChannel(Image *image,
%        const ChannelType channel,const char *threshold_map,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
%
%    o threshold_map: A string containing the name of the threshold dither
%      map to use, followed by zero or more numbers representing the number
%      of color levels tho dither between.
%
%      Any level number less than 2 will be equivalent to 2, and means only
%      binary dithering will be applied to each color channel.
%
%      No numbers also means a 2 level (bitmap) dither will be applied to all
%      channels, while a single number is the number of levels applied to each
%      channel in sequence.  More numbers will be applied in turn to each of
%      the color channels.
%
%      For example: "o3x3,6" will generate a 6 level posterization of the
%      image with a ordered 3x3 diffused pixel dither being applied between
%      each level. While checker,8,8,4 will produce a 332 colormaped image
%      with only a single checkerboard hash pattern (50% grey) between each
%      color level, to basically double the number of color levels with
%      a bare minimim of dithering.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType OrderedPosterizeImage(Image *image,
  const char *threshold_map,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=OrderedPosterizeImageChannel(image,DefaultChannels,threshold_map,
    exception);
  return(status);
}

MagickExport MagickBooleanType OrderedPosterizeImageChannel(Image *image,
  const ChannelType channel,const char *threshold_map,ExceptionInfo *exception)
{
#define DitherImageTag  "Dither/Image"

  CacheView
    *image_view;

  LongPixelPacket
    levels;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  ThresholdMap
    *map;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (threshold_map == (const char *) NULL)
    return(MagickTrue);
  {
    char
      token[MaxTextExtent];

    register const char
      *p;

    p=(char *)threshold_map;
    while (((isspace((int) ((unsigned char) *p)) != 0) || (*p == ',')) &&
                    (*p != '\0'))
      p++;
    threshold_map=p;
    while (((isspace((int) ((unsigned char) *p)) == 0) && (*p != ',')) &&
                    (*p != '\0')) {
      if ((p-threshold_map) >= (MaxTextExtent-1))
        break;
      token[p-threshold_map] = *p;
      p++;
    }
    token[p-threshold_map] = '\0';
    map = GetThresholdMap(token, exception);
    if ( map == (ThresholdMap *) NULL ) {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "InvalidArgument","%s : '%s'","ordered-dither",threshold_map);
      return(MagickFalse);
    }
  }
  /* Set channel levels from extra comma separated arguments
     Default to 2, the single value given, or individual channel values
  */
#if 1
  { /* parse directly as a comma separated list of integers */
    char *p;

    p = strchr((char *) threshold_map,',');
    if ( p != (char *) NULL && isdigit((int) ((unsigned char) *(++p))) )
      levels.index = (unsigned int) strtoul(p, &p, 10);
    else
      levels.index = 2;

    levels.red     = ((channel & RedChannel  )   != 0) ? levels.index : 0;
    levels.green   = ((channel & GreenChannel)   != 0) ? levels.index : 0;
    levels.blue    = ((channel & BlueChannel)    != 0) ? levels.index : 0;
    levels.opacity = ((channel & OpacityChannel) != 0) ? levels.index : 0;
    levels.index   = ((channel & IndexChannel)   != 0
            && (image->colorspace == CMYKColorspace)) ? levels.index : 0;

    /* if more than a single number, each channel has a separate value */
    if ( p != (char *) NULL && *p == ',' ) {
      p=strchr((char *) threshold_map,',');
      p++;
      if ((channel & RedChannel) != 0)
        levels.red = (unsigned int) strtoul(p, &p, 10),   (void)(*p == ',' && p++);
      if ((channel & GreenChannel) != 0)
        levels.green = (unsigned int) strtoul(p, &p, 10), (void)(*p == ',' && p++);
      if ((channel & BlueChannel) != 0)
        levels.blue = (unsigned int) strtoul(p, &p, 10),  (void)(*p == ',' && p++);
      if ((channel & IndexChannel) != 0 && image->colorspace == CMYKColorspace)
        levels.index=(unsigned int) strtoul(p, &p, 10), (void)(*p == ',' && p++);
      if ((channel & OpacityChannel) != 0)
        levels.opacity = (unsigned int) strtoul(p, &p, 10), (void)(*p == ',' && p++);
    }
  }
#else
  /* Parse level values as a geometry */
  /* This difficult!
   * How to map   GeometryInfo structure elements into
   * LongPixelPacket structure elements, but according to channel?
   * Note the channels list may skip elements!!!!
   * EG  -channel BA  -ordered-dither map,2,3
   * will need to map  g.rho -> l.blue, and g.sigma -> l.opacity
   * A simpler way is needed, probably converting geometry to a temporary
   * array, then using channel to advance the index into ssize_t pixel packet.
   */
#endif

#if 0
printf("DEBUG levels  r=%u g=%u b=%u a=%u i=%u\n",
     levels.red, levels.green, levels.blue, levels.opacity, levels.index);
#endif

  { /* Do the posterized ordered dithering of the image */
    ssize_t
      d;

    /* d = number of psuedo-level divisions added between color levels */
    d = map->divisor-1;

    /* reduce levels to levels - 1 */
    levels.red     = levels.red     ? levels.red-1     : 0;
    levels.green   = levels.green   ? levels.green-1   : 0;
    levels.blue    = levels.blue    ? levels.blue-1    : 0;
    levels.opacity = levels.opacity ? levels.opacity-1 : 0;
    levels.index   = levels.index   ? levels.index-1   : 0;

    if (SetImageStorageClass(image,DirectClass) == MagickFalse)
      {
        InheritException(exception,&image->exception);
        return(MagickFalse);
      }
    status=MagickTrue;
    progress=0;
    image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp parallel for schedule(static,4) shared(progress,status) \
      magick_threads(image,image,image->rows,1)
#endif
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      register IndexPacket
        *restrict indexes;

      register ssize_t
        x;

      register PixelPacket
        *restrict q;

      if (status == MagickFalse)
        continue;
      q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      indexes=GetCacheViewAuthenticIndexQueue(image_view);
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        register ssize_t
          threshold,
          t,
          l;

        /*
          Figure out the dither threshold for this pixel
          This must be a integer from 1 to map->divisor-1
        */
        threshold = map->levels[(x%map->width) +map->width*(y%map->height)];

        /* Dither each channel in the image as appropriate
          Notes on the integer Math...
              total number of divisions = (levels-1)*(divisor-1)+1)
              t1 = this colors psuedo_level =
                      q->red * total_divisions / (QuantumRange+1)
              l = posterization level       0..levels
              t = dither threshold level    0..divisor-1  NB: 0 only on last
              Each color_level is of size   QuantumRange / (levels-1)
              NB: All input levels and divisor are already had 1 subtracted
              Opacity is inverted so 'off' represents transparent.
        */
        if (levels.red) {
          t = (ssize_t) (QuantumScale*GetPixelRed(q)*(levels.red*d+1));
          l = t/d;  t = t-l*d;
          SetPixelRed(q,ClampToQuantum((MagickRealType)
            ((l+(t >= threshold))*(MagickRealType) QuantumRange/levels.red)));
        }
        if (levels.green) {
          t = (ssize_t) (QuantumScale*GetPixelGreen(q)*
            (levels.green*d+1));
          l = t/d;  t = t-l*d;
          SetPixelGreen(q,ClampToQuantum((MagickRealType)
            ((l+(t >= threshold))*(MagickRealType) QuantumRange/levels.green)));
        }
        if (levels.blue) {
          t = (ssize_t) (QuantumScale*GetPixelBlue(q)*
            (levels.blue*d+1));
          l = t/d;  t = t-l*d;
          SetPixelBlue(q,ClampToQuantum((MagickRealType)
            ((l+(t >= threshold))*(MagickRealType) QuantumRange/levels.blue)));
        }
        if (levels.opacity) {
          t = (ssize_t) ((1.0-QuantumScale*GetPixelOpacity(q))*
            (levels.opacity*d+1));
          l = t/d;  t = t-l*d;
          SetPixelOpacity(q,ClampToQuantum((MagickRealType)
            ((1.0-l-(t >= threshold))*(MagickRealType) QuantumRange/
            levels.opacity)));
        }
        if (levels.index) {
          t = (ssize_t) (QuantumScale*GetPixelIndex(indexes+x)*
            (levels.index*d+1));
          l = t/d;  t = t-l*d;
          SetPixelIndex(indexes+x,ClampToQuantum((MagickRealType) ((l+
            (t>=threshold))*(MagickRealType) QuantumRange/levels.index)));
        }
        q++;
      }
      if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
        status=MagickFalse;
      if (image->progress_monitor != (MagickProgressMonitor) NULL)
        {
          MagickBooleanType
            proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp critical (MagickCore_OrderedPosterizeImageChannel)
#endif
          proceed=SetImageProgress(image,DitherImageTag,progress++,image->rows);
          if (proceed == MagickFalse)
            status=MagickFalse;
        }
    }
    image_view=DestroyCacheView(image_view);
  }
  map=DestroyThresholdMap(map);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P e r c e p t i b l e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PerceptibleImage() set each pixel whose value is less than |epsilon| to
%  epsilon or -epsilon (whichever is closer) otherwise the pixel value remains
%  unchanged.
%
%  The format of the PerceptibleImageChannel method is:
%
%      MagickBooleanType PerceptibleImage(Image *image,const double epsilon)
%      MagickBooleanType PerceptibleImageChannel(Image *image,
%        const ChannelType channel,const double epsilon)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o epsilon: the epsilon threshold (e.g. 1.0e-9).
%
*/

static inline Quantum PerceptibleThreshold(const Quantum quantum,
  const double epsilon)
{
  double
    sign;

  sign=(double) quantum < 0.0 ? -1.0 : 1.0;
  if ((sign*quantum) >= epsilon)
    return(quantum);
  return((Quantum) (sign*epsilon));
}

MagickExport MagickBooleanType PerceptibleImage(Image *image,
  const double epsilon)
{
  MagickBooleanType
    status;

  status=PerceptibleImageChannel(image,DefaultChannels,epsilon);
  return(status);
}

MagickExport MagickBooleanType PerceptibleImageChannel(Image *image,
  const ChannelType channel,const double epsilon)
{
#define PerceptibleImageTag  "Perceptible/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    {
      register ssize_t
        i;

      register PixelPacket
        *restrict q;

      q=image->colormap;
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        SetPixelRed(q,PerceptibleThreshold(GetPixelRed(q),epsilon));
        SetPixelGreen(q,PerceptibleThreshold(GetPixelGreen(q),epsilon));
        SetPixelBlue(q,PerceptibleThreshold(GetPixelBlue(q),epsilon));
        SetPixelOpacity(q,PerceptibleThreshold(GetPixelOpacity(q),epsilon));
        q++;
      }
      return(SyncImage(image));
    }
  /*
    Perceptible image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        SetPixelRed(q,PerceptibleThreshold(GetPixelRed(q),epsilon));
      if ((channel & GreenChannel) != 0)
        SetPixelGreen(q,PerceptibleThreshold(GetPixelGreen(q),epsilon));
      if ((channel & BlueChannel) != 0)
        SetPixelBlue(q,PerceptibleThreshold(GetPixelBlue(q),epsilon));
      if ((channel & OpacityChannel) != 0)
        SetPixelOpacity(q,PerceptibleThreshold(GetPixelOpacity(q),epsilon));
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelIndex(indexes+x,PerceptibleThreshold(GetPixelIndex(indexes+x),
          epsilon));
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_PerceptibleImageChannel)
#endif
        proceed=SetImageProgress(image,PerceptibleImageTag,progress++,image->rows);
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
%     R a n d o m T h r e s h o l d I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RandomThresholdImage() changes the value of individual pixels based on the
%  intensity of each pixel compared to a random threshold.  The result is a
%  low-contrast, two color image.
%
%  The format of the RandomThresholdImage method is:
%
%      MagickBooleanType RandomThresholdImageChannel(Image *image,
%        const char *thresholds,ExceptionInfo *exception)
%      MagickBooleanType RandomThresholdImageChannel(Image *image,
%        const ChannelType channel,const char *thresholds,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
%
%    o thresholds: a geometry string containing low,high thresholds.  If the
%      string contains 2x2, 3x3, or 4x4, an ordered dither of order 2, 3, or 4
%      is performed instead.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType RandomThresholdImage(Image *image,
  const char *thresholds,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=RandomThresholdImageChannel(image,DefaultChannels,thresholds,
    exception);
  return(status);
}

MagickExport MagickBooleanType RandomThresholdImageChannel(Image *image,
  const ChannelType channel,const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  MagickStatusType
    flags;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    threshold;

  MagickRealType
    min_threshold,
    max_threshold;

  RandomInfo
    **restrict random_info;

  ssize_t
    y;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  unsigned long
    key;
#endif

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  GetMagickPixelPacket(image,&threshold);
  min_threshold=0.0;
  max_threshold=(MagickRealType) QuantumRange;
  flags=ParseGeometry(thresholds,&geometry_info);
  min_threshold=geometry_info.rho;
  max_threshold=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    max_threshold=min_threshold;
  if (strchr(thresholds,'%') != (char *) NULL)
    {
      max_threshold*=(MagickRealType) (0.01*QuantumRange);
      min_threshold*=(MagickRealType) (0.01*QuantumRange);
    }
  else
    if (((max_threshold == min_threshold) || (max_threshold == 1)) &&
        (min_threshold <= 8))
      {
        /*
          Backward Compatibility -- ordered-dither -- IM v 6.2.9-6.
        */
        status=OrderedPosterizeImageChannel(image,channel,thresholds,exception);
        return(status);
      }
  /*
    Random threshold image.
  */
  status=MagickTrue;
  progress=0;
  if (channel == CompositeChannels)
    {
      if (AcquireImageColormap(image,2) == MagickFalse)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      random_info=AcquireRandomInfoThreadSet();
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      key=GetRandomSecretKey(random_info[0]);
#endif
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(progress,status) \
        magick_threads(image,image,image->rows,key == ~0UL)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        const int
          id = GetOpenMPThreadId();

        MagickBooleanType
          sync;

        register IndexPacket
          *restrict indexes;

        register ssize_t
          x;

        register PixelPacket
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        indexes=GetCacheViewAuthenticIndexQueue(image_view);
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          IndexPacket
            index;

          MagickRealType
            intensity;

          intensity=GetPixelIntensity(image,q);
          if (intensity < min_threshold)
            threshold.index=min_threshold;
          else if (intensity > max_threshold)
            threshold.index=max_threshold;
          else
            threshold.index=(MagickRealType)(QuantumRange*
              GetPseudoRandomValue(random_info[id]));
          index=(IndexPacket) (intensity <= threshold.index ? 0 : 1);
          SetPixelIndex(indexes+x,index);
          SetPixelRGBO(q,image->colormap+(ssize_t) index);
          q++;
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp critical (MagickCore_RandomThresholdImageChannel)
#endif
            proceed=SetImageProgress(image,ThresholdImageTag,progress++,
              image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      image_view=DestroyCacheView(image_view);
      random_info=DestroyRandomInfoThreadSet(random_info);
      return(status);
    }
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      return(MagickFalse);
    }
  random_info=AcquireRandomInfoThreadSet();
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  key=GetRandomSecretKey(random_info[0]);
#endif
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,key == ~0UL)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register IndexPacket
      *restrict indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          if ((MagickRealType) GetPixelRed(q) < min_threshold)
            threshold.red=min_threshold;
          else
            if ((MagickRealType) GetPixelRed(q) > max_threshold)
              threshold.red=max_threshold;
            else
              threshold.red=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((channel & GreenChannel) != 0)
        {
          if ((MagickRealType) GetPixelGreen(q) < min_threshold)
            threshold.green=min_threshold;
          else
            if ((MagickRealType) GetPixelGreen(q) > max_threshold)
              threshold.green=max_threshold;
            else
              threshold.green=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((channel & BlueChannel) != 0)
        {
          if ((MagickRealType) GetPixelBlue(q) < min_threshold)
            threshold.blue=min_threshold;
          else
            if ((MagickRealType) GetPixelBlue(q) > max_threshold)
              threshold.blue=max_threshold;
            else
              threshold.blue=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((channel & OpacityChannel) != 0)
        {
          if ((MagickRealType) GetPixelOpacity(q) < min_threshold)
            threshold.opacity=min_threshold;
          else
            if ((MagickRealType) GetPixelOpacity(q) > max_threshold)
              threshold.opacity=max_threshold;
            else
              threshold.opacity=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          if ((MagickRealType) GetPixelIndex(indexes+x) < min_threshold)
            threshold.index=min_threshold;
          else
            if ((MagickRealType) GetPixelIndex(indexes+x) > max_threshold)
              threshold.index=max_threshold;
            else
              threshold.index=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((channel & RedChannel) != 0)
        SetPixelRed(q,(MagickRealType) GetPixelRed(q) <= threshold.red ?
          0 : QuantumRange);
      if ((channel & GreenChannel) != 0)
        SetPixelGreen(q,(MagickRealType) GetPixelGreen(q) <= threshold.green ?
          0 : QuantumRange);
      if ((channel & BlueChannel) != 0)
        SetPixelBlue(q,(MagickRealType) GetPixelBlue(q) <= threshold.blue ?
          0 : QuantumRange);
      if ((channel & OpacityChannel) != 0)
        SetPixelOpacity(q,(MagickRealType) GetPixelOpacity(q) <=
          threshold.opacity ? 0 : QuantumRange);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelIndex(indexes+x,(MagickRealType) GetPixelIndex(indexes+x) <=
          threshold.index ? 0 : QuantumRange);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_RandomThresholdImageChannel)
#endif
        proceed=SetImageProgress(image,ThresholdImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  random_info=DestroyRandomInfoThreadSet(random_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     W h i t e T h r e s h o l d I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WhiteThresholdImage() is like ThresholdImage() but forces all pixels above
%  the threshold into white while leaving all pixels at or below the threshold
%  unchanged.
%
%  The format of the WhiteThresholdImage method is:
%
%      MagickBooleanType WhiteThresholdImage(Image *image,const char *threshold)
%      MagickBooleanType WhiteThresholdImageChannel(Image *image,
%        const ChannelType channel,const char *threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
%
%    o threshold: Define the threshold value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType WhiteThresholdImage(Image *image,
  const char *threshold)
{
  MagickBooleanType
    status;

  status=WhiteThresholdImageChannel(image,DefaultChannels,threshold,
    &image->exception);
  return(status);
}

MagickExport MagickBooleanType WhiteThresholdImageChannel(Image *image,
  const ChannelType channel,const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickPixelPacket
    threshold;

  MagickStatusType
    flags;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  flags=ParseGeometry(thresholds,&geometry_info);
  GetMagickPixelPacket(image,&threshold);
  threshold.red=geometry_info.rho;
  threshold.green=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    threshold.green=threshold.red;
  threshold.blue=geometry_info.xi;
  if ((flags & XiValue) == 0)
    threshold.blue=threshold.red;
  threshold.opacity=geometry_info.psi;
  if ((flags & PsiValue) == 0)
    threshold.opacity=threshold.red;
  threshold.index=geometry_info.chi;
  if ((flags & ChiValue) == 0)
    threshold.index=threshold.red;
  if ((flags & PercentValue) != 0)
    {
      threshold.red*=(MagickRealType) (QuantumRange/100.0);
      threshold.green*=(MagickRealType) (QuantumRange/100.0);
      threshold.blue*=(MagickRealType) (QuantumRange/100.0);
      threshold.opacity*=(MagickRealType) (QuantumRange/100.0);
      threshold.index*=(MagickRealType) (QuantumRange/100.0);
    }
  if ((IsMagickGray(&threshold) == MagickFalse) &&
      (IsGrayColorspace(image->colorspace) != MagickFalse))
    (void) SetImageColorspace(image,sRGBColorspace);
  /*
    White threshold image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (((channel & RedChannel) != 0) &&
          ((MagickRealType) GetPixelRed(q) > threshold.red))
        SetPixelRed(q,QuantumRange);
      if (((channel & GreenChannel) != 0) &&
          ((MagickRealType) GetPixelGreen(q) > threshold.green))
        SetPixelGreen(q,QuantumRange);
      if (((channel & BlueChannel) != 0) &&
          ((MagickRealType) GetPixelBlue(q) > threshold.blue))
        SetPixelBlue(q,QuantumRange);
      if (((channel & OpacityChannel) != 0) &&
          ((MagickRealType) GetPixelOpacity(q) > threshold.opacity))
        SetPixelOpacity(q,QuantumRange);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace) &&
          ((MagickRealType) GetPixelIndex(indexes+x)) > threshold.index)
        SetPixelIndex(indexes+x,QuantumRange);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_WhiteThresholdImageChannel)
#endif
        proceed=SetImageProgress(image,ThresholdImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

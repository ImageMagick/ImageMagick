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
%  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/studio.h"
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/configure.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/draw.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/effect.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/segment.h"
#include "MagickCore/shear.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xml-tree-private.h"

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
  Forward declarations.
*/
static ThresholdMap
  *GetThresholdMapFile(const char *,const char *,const char *,ExceptionInfo *);

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
%      Image *AdaptiveThresholdImage(const Image *image,const size_t width,
%        const size_t height,const double bias,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o width: the width of the local neighborhood.
%
%    o height: the height of the local neighborhood.
%
%    o bias: the mean bias.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AdaptiveThresholdImage(const Image *image,
  const size_t width,const size_t height,const double bias,
  ExceptionInfo *exception)
{
#define AdaptiveThresholdImageTag  "AdaptiveThreshold/Image"

  CacheView
    *image_view,
    *threshold_view;

  Image
    *threshold_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickSizeType
    number_pixels;

  ssize_t
    y;

  /*
    Initialize threshold image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  threshold_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (threshold_image == (Image *) NULL)
    return((Image *) NULL);
  status=SetImageStorageClass(threshold_image,DirectClass,exception);
  if (status == MagickFalse)
    {
      threshold_image=DestroyImage(threshold_image);
      return((Image *) NULL);
    }
  /*
    Threshold image.
  */
  status=MagickTrue;
  progress=0;
  number_pixels=(MagickSizeType) width*height;
  image_view=AcquireVirtualCacheView(image,exception);
  threshold_view=AcquireAuthenticCacheView(threshold_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,threshold_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      channel_bias[MaxPixelChannels],
      channel_sum[MaxPixelChannels];

    register const Quantum
      *restrict p,
      *restrict pixels;

    register Quantum
      *restrict q;

    register ssize_t
      i,
      x;

    ssize_t
      center,
      u,
      v;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y-(ssize_t)
      (height/2L),image->columns+width,height,exception);
    q=QueueCacheViewAuthenticPixels(threshold_view,0,y,threshold_image->columns,
      1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    center=(ssize_t) GetPixelChannels(image)*(image->columns+width)*(height/2L)+
      GetPixelChannels(image)*(width/2);
    for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
    {
      PixelChannel channel=GetPixelChannelChannel(image,i);
      PixelTrait traits=GetPixelChannelTraits(image,channel);
      PixelTrait threshold_traits=GetPixelChannelTraits(threshold_image,
        channel);
      if ((traits == UndefinedPixelTrait) ||
          (threshold_traits == UndefinedPixelTrait))
        continue;
      if (((threshold_traits & CopyPixelTrait) != 0) ||
          (GetPixelReadMask(image,p) == 0))
        {
          SetPixelChannel(threshold_image,channel,p[center+i],q);
          continue;
        }
      pixels=p;
      channel_bias[channel]=0.0;
      channel_sum[channel]=0.0;
      for (v=0; v < (ssize_t) height; v++)
      {
        for (u=0; u < (ssize_t) width; u++)
        {
          if (u == (ssize_t) (width-1))
            channel_bias[channel]+=pixels[i];
          channel_sum[channel]+=pixels[i];
          pixels+=GetPixelChannels(image);
        }
        pixels+=GetPixelChannels(image)*image->columns;
      }
    }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          mean;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait threshold_traits=GetPixelChannelTraits(threshold_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (threshold_traits == UndefinedPixelTrait))
          continue;
        if (((threshold_traits & CopyPixelTrait) != 0) ||
            (GetPixelReadMask(image,p) == 0))
          {
            SetPixelChannel(threshold_image,channel,p[center+i],q);
            continue;
          }
        channel_sum[channel]-=channel_bias[channel];
        channel_bias[channel]=0.0;
        pixels=p;
        for (v=0; v < (ssize_t) height; v++)
        {
          channel_bias[channel]+=pixels[i];
          pixels+=(width-1)*GetPixelChannels(image);
          channel_sum[channel]+=pixels[i];
          pixels+=GetPixelChannels(image)*(image->columns+1);
        }
        mean=(double) (channel_sum[channel]/number_pixels+bias);
        SetPixelChannel(threshold_image,channel,(Quantum) ((double)
          p[center+i] <= mean ? 0 : QuantumRange),q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(threshold_image);
    }
    if (SyncCacheViewAuthenticPixels(threshold_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_AdaptiveThresholdImage)
#endif
        proceed=SetImageProgress(image,AdaptiveThresholdImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  threshold_image->type=image->type;
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
%  The format of the BilevelImage method is:
%
%      MagickBooleanType BilevelImage(Image *image,const double threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: define the threshold values.
%
%    o exception: return any errors or warnings in this structure.
%
%  Aside: You can get the same results as operator using LevelImages()
%  with the 'threshold' value for both the black_point and the white_point.
%
*/
MagickExport MagickBooleanType BilevelImage(Image *image,const double threshold,
  ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

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
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  /*
    Bilevel threshold image.
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
    register ssize_t
      x;

    register Quantum
      *restrict q;

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
        pixel;

      register ssize_t
        i;

      if (GetPixelReadMask(image,q) == 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      pixel=GetPixelIntensity(image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if (image->channel_mask != DefaultChannels)
          pixel=(double) q[i];
        q[i]=(Quantum) (pixel <= threshold ? 0 : QuantumRange);
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_BilevelImage)
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
%      MagickBooleanType BlackThresholdImage(Image *image,
%        const char *threshold,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: define the threshold value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType BlackThresholdImage(Image *image,
  const char *thresholds,ExceptionInfo *exception)
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

  PixelInfo
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
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  GetPixelInfo(image,&threshold);
  flags=ParseGeometry(thresholds,&geometry_info);
  threshold.red=geometry_info.rho;
  threshold.green=geometry_info.rho;
  threshold.blue=geometry_info.rho;
  threshold.black=geometry_info.rho;
  threshold.alpha=100.0;
  if ((flags & SigmaValue) != 0)
    threshold.green=geometry_info.sigma;
  if ((flags & XiValue) != 0)
    threshold.blue=geometry_info.xi;
  if ((flags & PsiValue) != 0)
    threshold.alpha=geometry_info.psi;
  if (threshold.colorspace == CMYKColorspace)
    {
      if ((flags & PsiValue) != 0)
        threshold.black=geometry_info.psi;
      if ((flags & ChiValue) != 0)
        threshold.alpha=geometry_info.chi;
    }
  if ((flags & PercentValue) != 0)
    {
      threshold.red*=(MagickRealType) (QuantumRange/100.0);
      threshold.green*=(MagickRealType) (QuantumRange/100.0);
      threshold.blue*=(MagickRealType) (QuantumRange/100.0);
      threshold.black*=(MagickRealType) (QuantumRange/100.0);
      threshold.alpha*=(MagickRealType) (QuantumRange/100.0);
    }
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
    register ssize_t
      x;

    register Quantum
      *restrict q;

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
        pixel;

      register ssize_t
        i;

      if (GetPixelReadMask(image,q) == 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      pixel=GetPixelIntensity(image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if (image->channel_mask != DefaultChannels)
          pixel=(double) q[i];
        if (pixel <= GetPixelInfoChannel(&threshold,channel))
          q[i]=(Quantum) 0;
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_BlackThresholdImage)
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
%  The format of the ClampImage method is:
%
%      MagickBooleanType ClampImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType ClampImage(Image *image,ExceptionInfo *exception)
{
#define ClampImageTag  "Clamp/Image"

  CacheView
    *image_view;

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

      register PixelInfo
        *restrict q;

      q=image->colormap;
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        q->red=(double) ClampPixel(q->red);
        q->green=(double) ClampPixel(q->green);
        q->blue=(double) ClampPixel(q->blue);
        q->alpha=(double) ClampPixel(q->alpha);
        q++;
      }
      return(SyncImage(image,exception));
    }
  /*
    Clamp image.
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
    register ssize_t
      x;

    register Quantum
      *restrict q;

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
      register ssize_t
        i;

      if (GetPixelReadMask(image,q) == 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ClampPixel(q[i]);
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_ClampImage)
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
%  G e t T h r e s h o l d M a p                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetThresholdMap() loads and searches one or more threshold map files for the
%  map matching the given name or alias.
%
%  The format of the GetThresholdMap method is:
%
%      ThresholdMap *GetThresholdMap(const char *map_id,
%        ExceptionInfo *exception)
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
static ThresholdMap *GetThresholdMapFile(const char *xml,const char *filename,
  const char *map_id,ExceptionInfo *exception)
{
  char
    *p;

  const char
    *attribute,
    *content;

  double
    value;

  register ssize_t
    i;

  ThresholdMap
    *map;

  XMLTreeInfo
    *description,
    *levels,
    *threshold,
    *thresholds;

  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading threshold map file \"%s\" ...",filename);
  map=(ThresholdMap *) NULL;
  thresholds=NewXMLTree(xml,exception);
  if (thresholds == (XMLTreeInfo *) NULL)
    return(map);
  for (threshold=GetXMLTreeChild(thresholds,"threshold");
       threshold != (XMLTreeInfo *) NULL;
       threshold=GetNextXMLTreeTag(threshold))
  {
    attribute=GetXMLTreeAttribute(threshold,"map");
    if ((attribute != (char *) NULL) && (LocaleCompare(map_id,attribute) == 0))
      break;
    attribute=GetXMLTreeAttribute(threshold,"alias");
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
        "XmlMissingElement", "<description>, map \"%s\"",map_id);
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
  map=(ThresholdMap *) AcquireMagickMemory(sizeof(ThresholdMap));
  if (map == (ThresholdMap *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireThresholdMap");
  map->map_id=(char *) NULL;
  map->description=(char *) NULL;
  map->levels=(ssize_t *) NULL;
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
       "XmlInvalidAttribute", "<levels width>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  attribute=GetXMLTreeAttribute(levels,"height");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels height>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->height=StringToUnsignedLong(attribute);
  if (map->height == 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlInvalidAttribute", "<levels height>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  attribute=GetXMLTreeAttribute(levels,"divisor");
  if (attribute == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<levels divisor>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->divisor=(ssize_t) StringToLong(attribute);
  if (map->divisor < 2)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlInvalidAttribute", "<levels divisor>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  content=GetXMLTreeContent(levels);
  if (content == (char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingContent", "<levels>, map \"%s\"",map_id);
      thresholds=DestroyXMLTree(thresholds);
      map=DestroyThresholdMap(map);
      return(map);
    }
  map->levels=(ssize_t *) AcquireQuantumMemory((size_t) map->width,map->height*
    sizeof(*map->levels));
  if (map->levels == (ssize_t *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireThresholdMap");
  for (i=0; i < (ssize_t) (map->width*map->height); i++)
  {
    map->levels[i]=(ssize_t) strtol(content,&p,10);
    if (p == content)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlInvalidContent", "<level> too few values, map \"%s\"",map_id);
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
        "XmlInvalidContent", "<level> too many values, map \"%s\"",map_id);
     thresholds=DestroyXMLTree(thresholds);
     map=DestroyThresholdMap(map);
     return(map);
   }
  thresholds=DestroyXMLTree(thresholds);
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
  const char
    *alias,
    *content,
    *map;

  XMLTreeInfo
    *description,
    *threshold,
    *thresholds;

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
  threshold=GetXMLTreeChild(thresholds,"threshold");
  for ( ; threshold != (XMLTreeInfo *) NULL;
          threshold=GetNextXMLTreeTag(threshold))
  {
    map=GetXMLTreeAttribute(threshold,"map");
    if (map == (char *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlMissingAttribute", "<map>");
        thresholds=DestroyXMLTree(thresholds);
        return(MagickFalse);
      }
    alias=GetXMLTreeAttribute(threshold,"alias");
    description=GetXMLTreeChild(threshold,"description");
    if (description == (XMLTreeInfo *) NULL)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlMissingElement", "<description>, map \"%s\"",map);
        thresholds=DestroyXMLTree(thresholds);
        return(MagickFalse);
      }
    content=GetXMLTreeContent(description);
    if (content == (char *) NULL)
      {
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

  status=MagickTrue;
  if (file == (FILE *) NULL)
    file=stdout;
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
%
%  A description of each parameter follows:
%
%    o image: the image.
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
#define DitherImageTag  "Dither/Image"

  CacheView
    *image_view;

  char
    token[MaxTextExtent];

  const char
    *p;

  double
    levels[CompositePixelChannel];

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  register ssize_t
    i;

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
  p=(char *) threshold_map;
  while (((isspace((int) ((unsigned char) *p)) != 0) || (*p == ',')) &&
         (*p != '\0'))
    p++;
  threshold_map=p;
  while (((isspace((int) ((unsigned char) *p)) == 0) && (*p != ',')) &&
         (*p != '\0'))
  {
    if ((p-threshold_map) >= (MaxTextExtent-1))
      break;
    token[p-threshold_map]=(*p);
    p++;
  }
  token[p-threshold_map]='\0';
  map=GetThresholdMap(token,exception);
  if (map == (ThresholdMap *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "InvalidArgument","%s : '%s'","ordered-dither",threshold_map);
      return(MagickFalse);
    }
  for (i=0; i < MaxPixelChannels; i++)
    levels[i]=2.0;
  p=strchr((char *) threshold_map,',');
  if ((p != (char *) NULL) && (isdigit((int) ((unsigned char) *(++p))) != 0))
    for (i=0; (*p != '\0') && (i < MaxPixelChannels); i++)
    {
      GetMagickToken(p,&p,token);
      if (*token == ',')
        GetMagickToken(p,&p,token);
      levels[i]=StringToDouble(token,(char **) NULL);
    }
  for (i=0; i < MaxPixelChannels; i++)
    if (fabs(levels[i]) >= 1)
      levels[i]-=1.0;
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

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
      register ssize_t
        i;

      ssize_t
        n;

      n=0;
      if (GetPixelReadMask(image,q) == 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        ssize_t
          level,
          threshold;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if (fabs(levels[n++]) < MagickEpsilon)
          continue;
        threshold=(ssize_t) (QuantumScale*q[i]*(levels[n]*(map->divisor-1)+1));
        level=threshold/(map->divisor-1);
        threshold-=level*(map->divisor-1);
        q[i]=ClampToQuantum((double) (level+(threshold >=
          map->levels[(x % map->width)+map->width*(y % map->height)]))*
          QuantumRange/levels[n]);
        n++;
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_OrderedPosterizeImage)
#endif
        proceed=SetImageProgress(image,DitherImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
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
%  The format of the PerceptibleImage method is:
%
%      MagickBooleanType PerceptibleImage(Image *image,const double epsilon,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o epsilon: the epsilon threshold (e.g. 1.0e-9).
%
%    o exception: return any errors or warnings in this structure.
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
  const double epsilon,ExceptionInfo *exception)
{
#define PerceptibleImageTag  "Perceptible/Image"

  CacheView
    *image_view;

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

      register PixelInfo
        *restrict q;

      q=image->colormap;
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        q->red=(double) PerceptibleThreshold(ClampToQuantum(q->red),
          epsilon);
        q->green=(double) PerceptibleThreshold(ClampToQuantum(q->green),
          epsilon);
        q->blue=(double) PerceptibleThreshold(ClampToQuantum(q->blue),
          epsilon);
        q->alpha=(double) PerceptibleThreshold(ClampToQuantum(q->alpha),
          epsilon);
        q++;
      }
      return(SyncImage(image,exception));
    }
  /*
    Perceptible image.
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
    register ssize_t
      x;

    register Quantum
      *restrict q;

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
      register ssize_t
        i;

      if (GetPixelReadMask(image,q) == 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        q[i]=PerceptibleThreshold(q[i],epsilon);
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_PerceptibleImage)
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
%      MagickBooleanType RandomThresholdImage(Image *image,
%        const char *thresholds,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
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
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  double
    min_threshold,
    max_threshold;

  GeometryInfo
    geometry_info;

  MagickStatusType
    flags;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    threshold;

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
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  GetPixelInfo(image,&threshold);
  min_threshold=0.0;
  max_threshold=(double) QuantumRange;
  flags=ParseGeometry(thresholds,&geometry_info);
  min_threshold=geometry_info.rho;
  max_threshold=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    max_threshold=min_threshold;
  if (strchr(thresholds,'%') != (char *) NULL)
    {
      max_threshold*=(double) (0.01*QuantumRange);
      min_threshold*=(double) (0.01*QuantumRange);
    }
  /*
    Random threshold image.
  */
  status=MagickTrue;
  progress=0;
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

    register Quantum
      *restrict q;

    register ssize_t
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
      register ssize_t
        i;

      if (GetPixelReadMask(image,q) == 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          threshold;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if ((double) q[i] < min_threshold)
          threshold=min_threshold;
        else
          if ((double) q[i] > max_threshold)
            threshold=max_threshold;
          else
            threshold=(double) (QuantumRange*
              GetPseudoRandomValue(random_info[id]));
        q[i]=(double) q[i] <= threshold ? 0 : QuantumRange;
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_RandomThresholdImage)
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
%      MagickBooleanType WhiteThresholdImage(Image *image,
%        const char *threshold,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: Define the threshold value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType WhiteThresholdImage(Image *image,
  const char *thresholds,ExceptionInfo *exception)
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

  PixelInfo
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
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  GetPixelInfo(image,&threshold);
  flags=ParseGeometry(thresholds,&geometry_info);
  threshold.red=geometry_info.rho;
  threshold.green=geometry_info.rho;
  threshold.blue=geometry_info.rho;
  threshold.black=geometry_info.rho;
  threshold.alpha=100.0;
  if ((flags & SigmaValue) != 0)
    threshold.green=geometry_info.sigma;
  if ((flags & XiValue) != 0)
    threshold.blue=geometry_info.xi;
  if ((flags & PsiValue) != 0)
    threshold.alpha=geometry_info.psi;
  if (threshold.colorspace == CMYKColorspace)
    {
      if ((flags & PsiValue) != 0)
        threshold.black=geometry_info.psi;
      if ((flags & ChiValue) != 0)
        threshold.alpha=geometry_info.chi;
    }
  if ((flags & PercentValue) != 0)
    {
      threshold.red*=(MagickRealType) (QuantumRange/100.0);
      threshold.green*=(MagickRealType) (QuantumRange/100.0);
      threshold.blue*=(MagickRealType) (QuantumRange/100.0);
      threshold.black*=(MagickRealType) (QuantumRange/100.0);
      threshold.alpha*=(MagickRealType) (QuantumRange/100.0);
    }
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
    register ssize_t
      x;

    register Quantum
      *restrict q;

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
        pixel;

      register ssize_t
        i;

      if (GetPixelReadMask(image,q) == 0)
        {
          q+=GetPixelChannels(image);
          continue;
        }
      pixel=GetPixelIntensity(image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if (image->channel_mask != DefaultChannels)
          pixel=(double) q[i];
        if (pixel > GetPixelInfoChannel(&threshold,channel))
          q[i]=QuantumRange;
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_WhiteThresholdImage)
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

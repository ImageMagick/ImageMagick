/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               CCCC   OOO   M   M  PPPP    AAA   RRRR    EEEEE               %
%              C      O   O  MM MM  P   P  A   A  R   R   E                   %
%              C      O   O  M M M  PPPP   AAAAA  RRRR    EEE                 %
%              C      O   O  M   M  P      A   A  R R     E                   %
%               CCCC   OOO   M   M  P      A   A  R  R    EEEEE               %
%                                                                             %
%                                                                             %
%                      MagickCore Image Comparison Methods                    %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               December 2003                                 %
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
#include "magick/artifact.h"
#include "magick/cache-view.h"
#include "magick/channel.h"
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/compare.h"
#include "magick/composite-private.h"
#include "magick/constitute.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/resource_.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/statistic.h"
#include "magick/thread-private.h"
#include "magick/transform.h"
#include "magick/utility.h"
#include "magick/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p a r e I m a g e C h a n n e l s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompareImageChannels() compares one or more image channels of an image
%  to a reconstructed image and returns the difference image.
%
%  The format of the CompareImageChannels method is:
%
%      Image *CompareImageChannels(const Image *image,
%        const Image *reconstruct_image,const ChannelType channel,
%        const MetricType metric,double *distortion,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o reconstruct_image: the reconstruct image.
%
%    o channel: the channel.
%
%    o metric: the metric.
%
%    o distortion: the computed distortion between the images.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *CompareImages(Image *image,const Image *reconstruct_image,
  const MetricType metric,double *distortion,ExceptionInfo *exception)
{
  Image
    *highlight_image;

  highlight_image=CompareImageChannels(image,reconstruct_image,
    CompositeChannels,metric,distortion,exception);
  return(highlight_image);
}

MagickExport Image *CompareImageChannels(Image *image,
  const Image *reconstruct_image,const ChannelType channel,
  const MetricType metric,double *distortion,ExceptionInfo *exception)
{
  CacheView
    *highlight_view,
    *image_view,
    *reconstruct_view;

  const char
    *artifact;

  Image
    *difference_image,
    *highlight_image;

  MagickBooleanType
    status;

  MagickPixelPacket
    highlight,
    lowlight,
    zero;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickSignature);
  assert(distortion != (double *) NULL);
  *distortion=0.0;
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((reconstruct_image->columns != image->columns) ||
      (reconstruct_image->rows != image->rows))
    ThrowImageException(ImageError,"ImageSizeDiffers");
  status=GetImageChannelDistortion(image,reconstruct_image,channel,metric,
    distortion,exception);
  if (status == MagickFalse)
    return((Image *) NULL);
  difference_image=CloneImage(image,0,0,MagickTrue,exception);
  if (difference_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageAlphaChannel(difference_image,OpaqueAlphaChannel);
  highlight_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (highlight_image == (Image *) NULL)
    {
      difference_image=DestroyImage(difference_image);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(highlight_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&highlight_image->exception);
      difference_image=DestroyImage(difference_image);
      highlight_image=DestroyImage(highlight_image);
      return((Image *) NULL);
    }
  (void) SetImageAlphaChannel(highlight_image,OpaqueAlphaChannel);
  (void) QueryMagickColor("#f1001ecc",&highlight,exception);
  artifact=GetImageArtifact(image,"highlight-color");
  if (artifact != (const char *) NULL)
    (void) QueryMagickColor(artifact,&highlight,exception);
  (void) QueryMagickColor("#ffffffcc",&lowlight,exception);
  artifact=GetImageArtifact(image,"lowlight-color");
  if (artifact != (const char *) NULL)
    (void) QueryMagickColor(artifact,&lowlight,exception);
  if (highlight_image->colorspace == CMYKColorspace)
    {
      ConvertRGBToCMYK(&highlight);
      ConvertRGBToCMYK(&lowlight);
    }
  /*
    Generate difference image.
  */
  status=MagickTrue;
  GetMagickPixelPacket(image,&zero);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  highlight_view=AcquireAuthenticCacheView(highlight_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,highlight_image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    MagickPixelPacket
      pixel,
      reconstruct_pixel;

    register const IndexPacket
      *restrict indexes,
      *restrict reconstruct_indexes;

    register const PixelPacket
      *restrict p,
      *restrict q;

    register IndexPacket
      *restrict highlight_indexes;

    register PixelPacket
      *restrict r;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,reconstruct_image->columns,
      1,exception);
    r=QueueCacheViewAuthenticPixels(highlight_view,0,y,highlight_image->columns,
      1,exception);
    if ((p == (const PixelPacket *) NULL) ||
        (q == (const PixelPacket *) NULL) || (r == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reconstruct_indexes=GetCacheViewVirtualIndexQueue(reconstruct_view);
    highlight_indexes=GetCacheViewAuthenticIndexQueue(highlight_view);
    pixel=zero;
    reconstruct_pixel=zero;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickStatusType
        difference;

      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      SetMagickPixelPacket(reconstruct_image,q,reconstruct_indexes+x,
        &reconstruct_pixel);
      difference=MagickFalse;
      if (channel == CompositeChannels)
        {
          if (IsMagickColorSimilar(&pixel,&reconstruct_pixel) == MagickFalse)
            difference=MagickTrue;
        }
      else
        {
          if (((channel & RedChannel) != 0) &&
              (GetPixelRed(p) != GetPixelRed(q)))
            difference=MagickTrue;
          if (((channel & GreenChannel) != 0) &&
              (GetPixelGreen(p) != GetPixelGreen(q)))
            difference=MagickTrue;
          if (((channel & BlueChannel) != 0) &&
              (GetPixelBlue(p) != GetPixelBlue(q)))
            difference=MagickTrue;
          if (((channel & OpacityChannel) != 0) &&
              (image->matte != MagickFalse) &&
              (GetPixelOpacity(p) != GetPixelOpacity(q)))
            difference=MagickTrue;
          if ((((channel & IndexChannel) != 0) &&
               (image->colorspace == CMYKColorspace) &&
               (reconstruct_image->colorspace == CMYKColorspace)) &&
              (GetPixelIndex(indexes+x) !=
               GetPixelIndex(reconstruct_indexes+x)))
            difference=MagickTrue;
        }
      if (difference != MagickFalse)
        SetPixelPacket(highlight_image,&highlight,r,highlight_indexes+x);
      else
        SetPixelPacket(highlight_image,&lowlight,r,highlight_indexes+x);
      p++;
      q++;
      r++;
    }
    sync=SyncCacheViewAuthenticPixels(highlight_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
  }
  highlight_view=DestroyCacheView(highlight_view);
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  (void) CompositeImage(difference_image,image->compose,highlight_image,0,0);
  highlight_image=DestroyImage(highlight_image);
  if (status == MagickFalse)
    difference_image=DestroyImage(difference_image);
  return(difference_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l D i s t o r t i o n                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelDistortion() compares one or more image channels of an image
%  to a reconstructed image and returns the specified distortion metric.
%
%  The format of the GetImageChannelDistortion method is:
%
%      MagickBooleanType GetImageChannelDistortion(const Image *image,
%        const Image *reconstruct_image,const ChannelType channel,
%        const MetricType metric,double *distortion,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o reconstruct_image: the reconstruct image.
%
%    o channel: the channel.
%
%    o metric: the metric.
%
%    o distortion: the computed distortion between the images.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageDistortion(Image *image,
  const Image *reconstruct_image,const MetricType metric,double *distortion,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetImageChannelDistortion(image,reconstruct_image,CompositeChannels,
    metric,distortion,exception);
  return(status);
}

static MagickBooleanType GetAbsoluteDistortion(const Image *image,
  const Image *reconstruct_image,const ChannelType channel,double *distortion,
  ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  ssize_t
    y;

  /*
    Compute the absolute difference in pixels between two images.
  */
  status=MagickTrue;
  GetMagickPixelPacket(image,&zero);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      channel_distortion[CompositeChannels+1];

    MagickPixelPacket
      pixel,
      reconstruct_pixel;

    register const IndexPacket
      *restrict indexes,
      *restrict reconstruct_indexes;

    register const PixelPacket
      *restrict p,
      *restrict q;

    register ssize_t
      i,
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,reconstruct_image->columns,
      1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (const PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reconstruct_indexes=GetCacheViewVirtualIndexQueue(reconstruct_view);
    pixel=zero;
    reconstruct_pixel=pixel;
    (void) ResetMagickMemory(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      SetMagickPixelPacket(reconstruct_image,q,reconstruct_indexes+x,
        &reconstruct_pixel);
      if (IsMagickColorSimilar(&pixel,&reconstruct_pixel) == MagickFalse)
        {
          if ((channel & RedChannel) != 0)
            channel_distortion[RedChannel]++;
          if ((channel & GreenChannel) != 0)
            channel_distortion[GreenChannel]++;
          if ((channel & BlueChannel) != 0)
            channel_distortion[BlueChannel]++;
          if (((channel & OpacityChannel) != 0) &&
              (image->matte != MagickFalse))
            channel_distortion[OpacityChannel]++;
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            channel_distortion[BlackChannel]++;
          channel_distortion[CompositeChannels]++;
        }
      p++;
      q++;
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetAbsoluteError)
#endif
    for (i=0; i <= (ssize_t) CompositeChannels; i++)
      distortion[i]+=channel_distortion[i];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

static size_t GetNumberChannels(const Image *image,
  const ChannelType channel)
{
  size_t
    channels;

  channels=0;
  if ((channel & RedChannel) != 0)
    channels++;
  if ((channel & GreenChannel) != 0)
    channels++;
  if ((channel & BlueChannel) != 0)
    channels++;
  if (((channel & OpacityChannel) != 0) &&
       (image->matte != MagickFalse))
    channels++;
  if (((channel & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace))
    channels++;
  return(channels);
}

static MagickBooleanType GetFuzzDistortion(const Image *image,
  const Image *reconstruct_image,const ChannelType channel,
  double *distortion,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  MagickBooleanType
    status;

  register ssize_t
    i;

  ssize_t
    y;

  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      channel_distortion[CompositeChannels+1];

    register const IndexPacket
      *restrict indexes,
      *restrict reconstruct_indexes;

    register const PixelPacket
      *restrict p,
      *restrict q;

    register ssize_t
      i,
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,reconstruct_image->columns,
      1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (const PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reconstruct_indexes=GetCacheViewVirtualIndexQueue(reconstruct_view);
    (void) ResetMagickMemory(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        distance,
        Da,
        Sa;

      Sa=QuantumScale*(image->matte != MagickFalse ? GetPixelAlpha(p) :
        (QuantumRange-OpaqueOpacity));
      Da=QuantumScale*(reconstruct_image->matte != MagickFalse ?
        GetPixelAlpha(q) : (QuantumRange-OpaqueOpacity));
      if ((channel & RedChannel) != 0)
        {
          distance=QuantumScale*(Sa*GetPixelRed(p)-Da*GetPixelRed(q));
          channel_distortion[RedChannel]+=distance*distance;
          channel_distortion[CompositeChannels]+=distance*distance;
        }
      if ((channel & GreenChannel) != 0)
        {
          distance=QuantumScale*(Sa*GetPixelGreen(p)-Da*GetPixelGreen(q));
          channel_distortion[GreenChannel]+=distance*distance;
          channel_distortion[CompositeChannels]+=distance*distance;
        }
      if ((channel & BlueChannel) != 0)
        {
          distance=QuantumScale*(Sa*GetPixelBlue(p)-Da*GetPixelBlue(q));
          channel_distortion[BlueChannel]+=distance*distance;
          channel_distortion[CompositeChannels]+=distance*distance;
        }
      if (((channel & OpacityChannel) != 0) && ((image->matte != MagickFalse) ||
          (reconstruct_image->matte != MagickFalse)))
        {
          distance=QuantumScale*((image->matte != MagickFalse ?
            GetPixelOpacity(p) : OpaqueOpacity)-
            (reconstruct_image->matte != MagickFalse ?
            GetPixelOpacity(q): OpaqueOpacity));
          channel_distortion[OpacityChannel]+=distance*distance;
          channel_distortion[CompositeChannels]+=distance*distance;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace) &&
          (reconstruct_image->colorspace == CMYKColorspace))
        {
          distance=QuantumScale*(Sa*GetPixelIndex(indexes+x)-
            Da*GetPixelIndex(reconstruct_indexes+x));
          channel_distortion[BlackChannel]+=distance*distance;
          channel_distortion[CompositeChannels]+=distance*distance;
        }
      p++;
      q++;
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetFuzzDistortion)
#endif
    for (i=0; i <= (ssize_t) CompositeChannels; i++)
      distortion[i]+=channel_distortion[i];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  for (i=0; i <= (ssize_t) CompositeChannels; i++)
    distortion[i]/=((double) image->columns*image->rows);
  if (((channel & OpacityChannel) != 0) && ((image->matte != MagickFalse) ||
      (reconstruct_image->matte != MagickFalse)))
    distortion[CompositeChannels]/=(double) (GetNumberChannels(image,channel)-1);
  else
    distortion[CompositeChannels]/=(double) GetNumberChannels(image,channel);
  distortion[CompositeChannels]=sqrt(distortion[CompositeChannels]);
  return(status);
}

static MagickBooleanType GetMeanAbsoluteDistortion(const Image *image,
  const Image *reconstruct_image,const ChannelType channel,
  double *distortion,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  MagickBooleanType
    status;

  register ssize_t
    i;

  ssize_t
    y;

  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      channel_distortion[CompositeChannels+1];

    register const IndexPacket
      *restrict indexes,
      *restrict reconstruct_indexes;

    register const PixelPacket
      *restrict p,
      *restrict q;

    register ssize_t
      i,
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,
      reconstruct_image->columns,1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (const PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reconstruct_indexes=GetCacheViewVirtualIndexQueue(reconstruct_view);
    (void) ResetMagickMemory(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        distance,
        Da,
        Sa;

      Sa=QuantumScale*(image->matte != MagickFalse ? GetPixelAlpha(p) :
        (QuantumRange-OpaqueOpacity));
      Da=QuantumScale*(reconstruct_image->matte != MagickFalse ?
        GetPixelAlpha(q) : (QuantumRange-OpaqueOpacity));
      if ((channel & RedChannel) != 0)
        {
          distance=QuantumScale*fabs(Sa*GetPixelRed(p)-Da*GetPixelRed(q));
          channel_distortion[RedChannel]+=distance;
          channel_distortion[CompositeChannels]+=distance;
        }
      if ((channel & GreenChannel) != 0)
        {
          distance=QuantumScale*fabs(Sa*GetPixelGreen(p)-Da*GetPixelGreen(q));
          channel_distortion[GreenChannel]+=distance;
          channel_distortion[CompositeChannels]+=distance;
        }
      if ((channel & BlueChannel) != 0)
        {
          distance=QuantumScale*fabs(Sa*GetPixelBlue(p)-Da*GetPixelBlue(q));
          channel_distortion[BlueChannel]+=distance;
          channel_distortion[CompositeChannels]+=distance;
        }
      if (((channel & OpacityChannel) != 0) &&
          (image->matte != MagickFalse))
        {
          distance=QuantumScale*fabs(GetPixelOpacity(p)-(double)
            GetPixelOpacity(q));
          channel_distortion[OpacityChannel]+=distance;
          channel_distortion[CompositeChannels]+=distance;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          distance=QuantumScale*fabs(Sa*GetPixelIndex(indexes+x)-Da*
            GetPixelIndex(reconstruct_indexes+x));
          channel_distortion[BlackChannel]+=distance;
          channel_distortion[CompositeChannels]+=distance;
        }
      p++;
      q++;
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMeanAbsoluteError)
#endif
    for (i=0; i <= (ssize_t) CompositeChannels; i++)
      distortion[i]+=channel_distortion[i];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  for (i=0; i <= (ssize_t) CompositeChannels; i++)
    distortion[i]/=((double) image->columns*image->rows);
  distortion[CompositeChannels]/=(double) GetNumberChannels(image,channel);
  return(status);
}

static MagickBooleanType GetMeanErrorPerPixel(Image *image,
  const Image *reconstruct_image,const ChannelType channel,double *distortion,
  ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  MagickBooleanType
    status;

  MagickRealType
    area,
    maximum_error,
    mean_error;

  ssize_t
    y;

  status=MagickTrue;
  area=0.0;
  maximum_error=0.0;
  mean_error=0.0;
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes,
      *restrict reconstruct_indexes;

    register const PixelPacket
      *restrict p,
      *restrict q;

    register ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,reconstruct_image->columns,
      1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (const PixelPacket *) NULL))
      {
        status=MagickFalse;
        break;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reconstruct_indexes=GetCacheViewVirtualIndexQueue(reconstruct_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        distance,
        Da,
        Sa;

      Sa=QuantumScale*(image->matte != MagickFalse ? GetPixelAlpha(p) :
        (QuantumRange-OpaqueOpacity));
      Da=QuantumScale*(reconstruct_image->matte != MagickFalse ?
        GetPixelAlpha(q) : (QuantumRange-OpaqueOpacity));
      if ((channel & RedChannel) != 0)
        {
          distance=fabs(Sa*GetPixelRed(p)-Da*GetPixelRed(q));
          distortion[RedChannel]+=distance;
          distortion[CompositeChannels]+=distance;
          mean_error+=distance*distance;
          if (distance > maximum_error)
            maximum_error=distance;
          area++;
        }
      if ((channel & GreenChannel) != 0)
        {
          distance=fabs(Sa*GetPixelGreen(p)-Da*GetPixelGreen(q));
          distortion[GreenChannel]+=distance;
          distortion[CompositeChannels]+=distance;
          mean_error+=distance*distance;
          if (distance > maximum_error)
            maximum_error=distance;
          area++;
        }
      if ((channel & BlueChannel) != 0)
        {
          distance=fabs(Sa*GetPixelBlue(p)-Da*GetPixelBlue(q));
          distortion[BlueChannel]+=distance;
          distortion[CompositeChannels]+=distance;
          mean_error+=distance*distance;
          if (distance > maximum_error)
            maximum_error=distance;
          area++;
        }
      if (((channel & OpacityChannel) != 0) &&
          (image->matte != MagickFalse))
        {
          distance=fabs((double) GetPixelOpacity(p)-
            GetPixelOpacity(q));
          distortion[OpacityChannel]+=distance;
          distortion[CompositeChannels]+=distance;
          mean_error+=distance*distance;
          if (distance > maximum_error)
            maximum_error=distance;
          area++;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace) &&
          (reconstruct_image->colorspace == CMYKColorspace))
        {
          distance=fabs(Sa*GetPixelIndex(indexes+x)-Da*
            GetPixelIndex(reconstruct_indexes+x));
          distortion[BlackChannel]+=distance;
          distortion[CompositeChannels]+=distance;
          mean_error+=distance*distance;
          if (distance > maximum_error)
            maximum_error=distance;
          area++;
        }
      p++;
      q++;
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  image->error.mean_error_per_pixel=distortion[CompositeChannels]/area;
  image->error.normalized_mean_error=QuantumScale*QuantumScale*mean_error/area;
  image->error.normalized_maximum_error=QuantumScale*maximum_error;
  return(status);
}

static MagickBooleanType GetMeanSquaredDistortion(const Image *image,
  const Image *reconstruct_image,const ChannelType channel,
  double *distortion,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  MagickBooleanType
    status;

  register ssize_t
    i;

  ssize_t
    y;

  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      channel_distortion[CompositeChannels+1];

    register const IndexPacket
      *restrict indexes,
      *restrict reconstruct_indexes;

    register const PixelPacket
      *restrict p,
      *restrict q;

    register ssize_t
      i,
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,
      reconstruct_image->columns,1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (const PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reconstruct_indexes=GetCacheViewVirtualIndexQueue(reconstruct_view);
    (void) ResetMagickMemory(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        distance,
        Da,
        Sa;

      Sa=QuantumScale*(image->matte != MagickFalse ? GetPixelAlpha(p) :
        (QuantumRange-OpaqueOpacity));
      Da=QuantumScale*(reconstruct_image->matte != MagickFalse ?
        GetPixelAlpha(q) : (QuantumRange-OpaqueOpacity));
      if ((channel & RedChannel) != 0)
        {
          distance=QuantumScale*(Sa*GetPixelRed(p)-Da*GetPixelRed(q));
          channel_distortion[RedChannel]+=distance*distance;
          channel_distortion[CompositeChannels]+=distance*distance;
        }
      if ((channel & GreenChannel) != 0)
        {
          distance=QuantumScale*(Sa*GetPixelGreen(p)-Da*GetPixelGreen(q));
          channel_distortion[GreenChannel]+=distance*distance;
          channel_distortion[CompositeChannels]+=distance*distance;
        }
      if ((channel & BlueChannel) != 0)
        {
          distance=QuantumScale*(Sa*GetPixelBlue(p)-Da*GetPixelBlue(q));
          channel_distortion[BlueChannel]+=distance*distance;
          channel_distortion[CompositeChannels]+=distance*distance;
        }
      if (((channel & OpacityChannel) != 0) &&
          (image->matte != MagickFalse))
        {
          distance=QuantumScale*(GetPixelOpacity(p)-(MagickRealType)
            GetPixelOpacity(q));
          channel_distortion[OpacityChannel]+=distance*distance;
          channel_distortion[CompositeChannels]+=distance*distance;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace) &&
          (reconstruct_image->colorspace == CMYKColorspace))
        {
          distance=QuantumScale*(Sa*GetPixelIndex(indexes+x)-Da*
            GetPixelIndex(reconstruct_indexes+x));
          channel_distortion[BlackChannel]+=distance*distance;
          channel_distortion[CompositeChannels]+=distance*distance;
        }
      p++;
      q++;
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMeanSquaredError)
#endif
    for (i=0; i <= (ssize_t) CompositeChannels; i++)
      distortion[i]+=channel_distortion[i];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  for (i=0; i <= (ssize_t) CompositeChannels; i++)
    distortion[i]/=((double) image->columns*image->rows);
  distortion[CompositeChannels]/=(double) GetNumberChannels(image,channel);
  return(status);
}

static MagickBooleanType GetNormalizedCrossCorrelationDistortion(
  const Image *image,const Image *reconstruct_image,const ChannelType channel,
  double *distortion,ExceptionInfo *exception)
{
#define SimilarityImageTag  "Similarity/Image"

  CacheView
    *image_view,
    *reconstruct_view;

  ChannelStatistics
    *image_statistics,
    *reconstruct_statistics;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MagickRealType
    area;

  register ssize_t
    i;

  ssize_t
    y;

  /*
    Normalize to account for variation due to lighting and exposure condition.
  */
  image_statistics=GetImageChannelStatistics(image,exception);
  reconstruct_statistics=GetImageChannelStatistics(reconstruct_image,exception);
  if ((image_statistics == (ChannelStatistics *) NULL) ||
      (reconstruct_statistics == (ChannelStatistics *) NULL))
    {
      if (image_statistics != (ChannelStatistics *) NULL)
        image_statistics=(ChannelStatistics *) RelinquishMagickMemory(
          image_statistics);
      if (reconstruct_statistics != (ChannelStatistics *) NULL)
        reconstruct_statistics=(ChannelStatistics *) RelinquishMagickMemory(
          reconstruct_statistics);
      return(MagickFalse);
    }
  status=MagickTrue;
  progress=0;
  for (i=0; i <= (ssize_t) CompositeChannels; i++)
    distortion[i]=0.0;
  area=1.0/((MagickRealType) image->columns*image->rows-1);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes,
      *restrict reconstruct_indexes;

    register const PixelPacket
      *restrict p,
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,reconstruct_image->columns,
      1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (const PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reconstruct_indexes=GetCacheViewVirtualIndexQueue(reconstruct_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        Da,
        Sa;

      Sa=QuantumScale*(image->matte != MagickFalse ? GetPixelAlpha(p) :
        (QuantumRange-OpaqueOpacity));
      Da=QuantumScale*(reconstruct_image->matte != MagickFalse ?
        GetPixelAlpha(q) : (QuantumRange-OpaqueOpacity));
      if ((channel & RedChannel) != 0)
        distortion[RedChannel]+=area*QuantumScale*(Sa*GetPixelRed(p)-
          image_statistics[RedChannel].mean)*(Da*GetPixelRed(q)-
          reconstruct_statistics[RedChannel].mean);
      if ((channel & GreenChannel) != 0)
        distortion[GreenChannel]+=area*QuantumScale*(Sa*GetPixelGreen(p)-
          image_statistics[GreenChannel].mean)*(Da*GetPixelGreen(q)-
          reconstruct_statistics[GreenChannel].mean);
      if ((channel & BlueChannel) != 0)
        distortion[BlueChannel]+=area*QuantumScale*(Sa*GetPixelBlue(p)-
          image_statistics[BlueChannel].mean)*(Da*GetPixelBlue(q)-
          reconstruct_statistics[BlueChannel].mean);
      if (((channel & OpacityChannel) != 0) &&
          (image->matte != MagickFalse))
        distortion[OpacityChannel]+=area*QuantumScale*(
          GetPixelOpacity(p)-image_statistics[OpacityChannel].mean)*
          (GetPixelOpacity(q)-reconstruct_statistics[OpacityChannel].mean);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace) &&
          (reconstruct_image->colorspace == CMYKColorspace))
        distortion[BlackChannel]+=area*QuantumScale*(Sa*
          GetPixelIndex(indexes+x)-
          image_statistics[OpacityChannel].mean)*(Da*
          GetPixelIndex(reconstruct_indexes+x)-
          reconstruct_statistics[OpacityChannel].mean);
      p++;
      q++;
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,SimilarityImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  /*
    Divide by the standard deviation.
  */
  for (i=0; i < (ssize_t) CompositeChannels; i++)
  {
    double
      gamma;

    gamma=image_statistics[i].standard_deviation*
      reconstruct_statistics[i].standard_deviation;
    gamma=PerceptibleReciprocal(gamma);
    distortion[i]=QuantumRange*gamma*distortion[i];
  }
  distortion[CompositeChannels]=0.0;
  if ((channel & RedChannel) != 0)
    distortion[CompositeChannels]+=distortion[RedChannel]*
      distortion[RedChannel];
  if ((channel & GreenChannel) != 0)
    distortion[CompositeChannels]+=distortion[GreenChannel]*
      distortion[GreenChannel];
  if ((channel & BlueChannel) != 0)
    distortion[CompositeChannels]+=distortion[BlueChannel]*
      distortion[BlueChannel];
  if (((channel & OpacityChannel) != 0) && (image->matte != MagickFalse))
    distortion[CompositeChannels]+=distortion[OpacityChannel]*
      distortion[OpacityChannel];
  if (((channel & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace))
    distortion[CompositeChannels]+=distortion[BlackChannel]*
      distortion[BlackChannel];
  distortion[CompositeChannels]=sqrt(distortion[CompositeChannels]/
    GetNumberChannels(image,channel));
  /*
    Free resources.
  */
  reconstruct_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    reconstruct_statistics);
  image_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    image_statistics);
  return(status);
}

static MagickBooleanType GetPeakAbsoluteDistortion(const Image *image,
  const Image *reconstruct_image,const ChannelType channel,
  double *distortion,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      channel_distortion[CompositeChannels+1];

    register const IndexPacket
      *restrict indexes,
      *restrict reconstruct_indexes;

    register const PixelPacket
      *restrict p,
      *restrict q;

    register ssize_t
      i,
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,
      reconstruct_image->columns,1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (const PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reconstruct_indexes=GetCacheViewVirtualIndexQueue(reconstruct_view);
    (void) ResetMagickMemory(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        distance,
        Da,
        Sa;

      Sa=QuantumScale*(image->matte != MagickFalse ? GetPixelAlpha(p) :
        (QuantumRange-OpaqueOpacity));
      Da=QuantumScale*(reconstruct_image->matte != MagickFalse ?
        GetPixelAlpha(q) : (QuantumRange-OpaqueOpacity));
      if ((channel & RedChannel) != 0)
        {
          distance=QuantumScale*fabs(Sa*GetPixelRed(p)-Da*GetPixelRed(q));
          if (distance > channel_distortion[RedChannel])
            channel_distortion[RedChannel]=distance;
          if (distance > channel_distortion[CompositeChannels])
            channel_distortion[CompositeChannels]=distance;
        }
      if ((channel & GreenChannel) != 0)
        {
          distance=QuantumScale*fabs(Sa*GetPixelGreen(p)-Da*GetPixelGreen(q));
          if (distance > channel_distortion[GreenChannel])
            channel_distortion[GreenChannel]=distance;
          if (distance > channel_distortion[CompositeChannels])
            channel_distortion[CompositeChannels]=distance;
        }
      if ((channel & BlueChannel) != 0)
        {
          distance=QuantumScale*fabs(Sa*GetPixelBlue(p)-Da*GetPixelBlue(q));
          if (distance > channel_distortion[BlueChannel])
            channel_distortion[BlueChannel]=distance;
          if (distance > channel_distortion[CompositeChannels])
            channel_distortion[CompositeChannels]=distance;
        }
      if (((channel & OpacityChannel) != 0) &&
          (image->matte != MagickFalse))
        {
          distance=QuantumScale*fabs(GetPixelOpacity(p)-(double)
            GetPixelOpacity(q));
          if (distance > channel_distortion[OpacityChannel])
            channel_distortion[OpacityChannel]=distance;
          if (distance > channel_distortion[CompositeChannels])
            channel_distortion[CompositeChannels]=distance;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace) &&
          (reconstruct_image->colorspace == CMYKColorspace))
        {
          distance=QuantumScale*fabs(Sa*GetPixelIndex(indexes+x)-Da*
            GetPixelIndex(reconstruct_indexes+x));
          if (distance > channel_distortion[BlackChannel])
            channel_distortion[BlackChannel]=distance;
          if (distance > channel_distortion[CompositeChannels])
            channel_distortion[CompositeChannels]=distance;
        }
      p++;
      q++;
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetPeakAbsoluteError)
#endif
    for (i=0; i <= (ssize_t) CompositeChannels; i++)
      if (channel_distortion[i] > distortion[i])
        distortion[i]=channel_distortion[i];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

static MagickBooleanType GetPeakSignalToNoiseRatio(const Image *image,
  const Image *reconstruct_image,const ChannelType channel,
  double *distortion,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetMeanSquaredDistortion(image,reconstruct_image,channel,distortion,
    exception);
  if ((channel & RedChannel) != 0)
    distortion[RedChannel]=20.0*log10((double) 1.0/sqrt(
      distortion[RedChannel]));
  if ((channel & GreenChannel) != 0)
    distortion[GreenChannel]=20.0*log10((double) 1.0/sqrt(
      distortion[GreenChannel]));
  if ((channel & BlueChannel) != 0)
    distortion[BlueChannel]=20.0*log10((double) 1.0/sqrt(
      distortion[BlueChannel]));
  if (((channel & OpacityChannel) != 0) &&
      (image->matte != MagickFalse))
    distortion[OpacityChannel]=20.0*log10((double) 1.0/sqrt(
      distortion[OpacityChannel]));
  if (((channel & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace))
    distortion[BlackChannel]=20.0*log10((double) 1.0/sqrt(
      distortion[BlackChannel]));
  distortion[CompositeChannels]=20.0*log10((double) 1.0/sqrt(
    distortion[CompositeChannels]));
  return(status);
}

static MagickBooleanType GetPerceptualHashDistortion(const Image *image,
  const Image *reconstruct_image,const ChannelType channel,
  double *distortion,ExceptionInfo *exception)
{
  ChannelMoments
    *image_moments,
    *reconstruct_moments;

  Image
    *blur_image,
    *blur_reconstruct;

  register ssize_t
    i;

  /*
    Compute perceptual hash in the native image colorspace.
  */
  blur_image=BlurImage(image,0.0,1.0,exception);
  if (blur_image == (Image *) NULL)
    return(MagickFalse);
  image_moments=GetImageChannelMoments(blur_image,exception);
  if (image_moments == (ChannelMoments *) NULL)
    {
      blur_image=DestroyImage(blur_image);
      return(MagickFalse);
    }
  blur_reconstruct=BlurImage(reconstruct_image,0.0,1.0,exception);
  if (blur_reconstruct == (Image *) NULL)
    {
      image_moments=(ChannelMoments *) RelinquishMagickMemory(image_moments);
      blur_image=DestroyImage(blur_image);
      return(MagickFalse);
    }
  reconstruct_moments=GetImageChannelMoments(blur_reconstruct,exception);
  if (reconstruct_moments == (ChannelMoments *) NULL)
    {
      image_moments=(ChannelMoments *) RelinquishMagickMemory(image_moments);
      blur_image=DestroyImage(blur_image);
      blur_reconstruct=DestroyImage(blur_reconstruct);
      return(MagickFalse);
    }
  for (i=0; i < 8; i++)
  {
    double
      difference;

    /*
      Compute sum of moment differences squared.
    */
    if (i == 2)
      continue;  /* I3 is not independent of other moments */
    if ((channel & RedChannel) != 0)
      {
        difference=log10(fabs(reconstruct_moments[RedChannel].I[i]))-
          log10(fabs(image_moments[RedChannel].I[i]));
        distortion[RedChannel]+=difference*difference;
        distortion[CompositeChannels]+=difference*difference;
      }
    if ((channel & GreenChannel) != 0)
      {
        difference=log10(fabs(reconstruct_moments[GreenChannel].I[i]))-
          log10(fabs(image_moments[GreenChannel].I[i]));
        distortion[GreenChannel]+=difference*difference;
        distortion[CompositeChannels]+=difference*difference;
      }
    if ((channel & BlueChannel) != 0)
      {
        difference=log10(fabs(reconstruct_moments[BlueChannel].I[i]))-
          log10(fabs(image_moments[BlueChannel].I[i]));
        distortion[BlueChannel]+=difference*difference;
        distortion[CompositeChannels]+=difference*difference;
      }
    if (((channel & OpacityChannel) != 0) && (image->matte != MagickFalse) &&
        (reconstruct_image->matte != MagickFalse))
      {
        difference=log10(fabs(reconstruct_moments[OpacityChannel].I[i]))-
          log10(fabs(image_moments[OpacityChannel].I[i]));
        distortion[OpacityChannel]+=difference*difference;
        distortion[CompositeChannels]+=difference*difference;
      }
    if (((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace) &&
        (reconstruct_image->colorspace == CMYKColorspace))
      {
        difference=log10(fabs(reconstruct_moments[IndexChannel].I[i]))-
          log10(fabs(image_moments[IndexChannel].I[i]));
        distortion[IndexChannel]+=difference*difference;
        distortion[CompositeChannels]+=difference*difference;
      }
  }
  image_moments=(ChannelMoments *) RelinquishMagickMemory(image_moments);
  reconstruct_moments=(ChannelMoments *) RelinquishMagickMemory(
    reconstruct_moments);
  /*
    Compute perceptual hash in the HCLP colorspace.
  */
  if ((SetImageColorspace(blur_image,HCLpColorspace) == MagickFalse) ||
      (SetImageColorspace(blur_reconstruct,HCLpColorspace) == MagickFalse))
    {
      blur_reconstruct=DestroyImage(blur_reconstruct);
      blur_image=DestroyImage(blur_image);
      return(MagickFalse);
    }
  image_moments=GetImageChannelMoments(blur_image,exception);
  blur_image=DestroyImage(blur_image);
  if (image_moments == (ChannelMoments *) NULL)
    {
      blur_reconstruct=DestroyImage(blur_reconstruct);
      return(MagickFalse);
    }
  reconstruct_moments=GetImageChannelMoments(blur_reconstruct,exception);
  blur_reconstruct=DestroyImage(blur_reconstruct);
  if (reconstruct_moments == (ChannelMoments *) NULL)
    {
      image_moments=(ChannelMoments *) RelinquishMagickMemory(image_moments);
      return(MagickFalse);
    }
  for (i=0; i < 8; i++)
  {
    double
      difference;

    /*
      Compute sum of moment differences squared.
    */
    if (i == 2)
      continue;  /* I3 is not independent of other moments */
    if ((channel & RedChannel) != 0)
      {
        difference=log10(fabs(reconstruct_moments[RedChannel].I[i]))-
          log10(fabs(image_moments[RedChannel].I[i]));
        distortion[RedChannel]+=difference*difference;
        distortion[CompositeChannels]+=difference*difference;
      }
    if ((channel & GreenChannel) != 0)
      {
        difference=log10(fabs(reconstruct_moments[GreenChannel].I[i]))-
          log10(fabs(image_moments[GreenChannel].I[i]));
        distortion[GreenChannel]+=difference*difference;
        distortion[CompositeChannels]+=difference*difference;
      }
    if ((channel & BlueChannel) != 0)
      {
        difference=log10(fabs(reconstruct_moments[BlueChannel].I[i]))-
          log10(fabs(image_moments[BlueChannel].I[i]));
        distortion[BlueChannel]+=difference*difference;
        distortion[CompositeChannels]+=difference*difference;
      }
    if (((channel & OpacityChannel) != 0) && (image->matte != MagickFalse) &&
        (reconstruct_image->matte != MagickFalse))
      {
        difference=log10(fabs(reconstruct_moments[OpacityChannel].I[i]))-
          log10(fabs(image_moments[OpacityChannel].I[i]));
        distortion[OpacityChannel]+=difference*difference;
        distortion[CompositeChannels]+=difference*difference;
      }
    if (((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace) &&
        (reconstruct_image->colorspace == CMYKColorspace))
      {
        difference=log10(fabs(reconstruct_moments[IndexChannel].I[i]))-
          log10(fabs(image_moments[IndexChannel].I[i]));
        distortion[IndexChannel]+=difference*difference;
        distortion[CompositeChannels]+=difference*difference;
      }
  }
  image_moments=(ChannelMoments *) RelinquishMagickMemory(image_moments);
  reconstruct_moments=(ChannelMoments *) RelinquishMagickMemory(
    reconstruct_moments);
  return(MagickTrue);
}

static MagickBooleanType GetRootMeanSquaredDistortion(const Image *image,
  const Image *reconstruct_image,const ChannelType channel,
  double *distortion,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetMeanSquaredDistortion(image,reconstruct_image,channel,distortion,
    exception);
  if ((channel & RedChannel) != 0)
    distortion[RedChannel]=sqrt(distortion[RedChannel]);
  if ((channel & GreenChannel) != 0)
    distortion[GreenChannel]=sqrt(distortion[GreenChannel]);
  if ((channel & BlueChannel) != 0)
    distortion[BlueChannel]=sqrt(distortion[BlueChannel]);
  if (((channel & OpacityChannel) != 0) &&
      (image->matte != MagickFalse))
    distortion[OpacityChannel]=sqrt(distortion[OpacityChannel]);
  if (((channel & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace))
    distortion[BlackChannel]=sqrt(distortion[BlackChannel]);
  distortion[CompositeChannels]=sqrt(distortion[CompositeChannels]);
  return(status);
}

MagickExport MagickBooleanType GetImageChannelDistortion(Image *image,
  const Image *reconstruct_image,const ChannelType channel,
  const MetricType metric,double *distortion,ExceptionInfo *exception)
{
  double
    *channel_distortion;

  MagickBooleanType
    status;

  size_t
    length;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickSignature);
  assert(distortion != (double *) NULL);
  *distortion=0.0;
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((reconstruct_image->columns != image->columns) ||
      (reconstruct_image->rows != image->rows))
    ThrowBinaryException(ImageError,"ImageSizeDiffers",image->filename);
  /*
    Get image distortion.
  */
  length=CompositeChannels+1UL;
  channel_distortion=(double *) AcquireQuantumMemory(length,
    sizeof(*channel_distortion));
  if (channel_distortion == (double *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_distortion,0,length*
    sizeof(*channel_distortion));
  switch (metric)
  {
    case AbsoluteErrorMetric:
    {
      status=GetAbsoluteDistortion(image,reconstruct_image,channel,
        channel_distortion,exception);
      break;
    }
    case FuzzErrorMetric:
    {
      status=GetFuzzDistortion(image,reconstruct_image,channel,
        channel_distortion,exception);
      break;
    }
    case MeanAbsoluteErrorMetric:
    {
      status=GetMeanAbsoluteDistortion(image,reconstruct_image,channel,
        channel_distortion,exception);
      break;
    }
    case MeanErrorPerPixelMetric:
    {
      status=GetMeanErrorPerPixel(image,reconstruct_image,channel,
        channel_distortion,exception);
      break;
    }
    case MeanSquaredErrorMetric:
    {
      status=GetMeanSquaredDistortion(image,reconstruct_image,channel,
        channel_distortion,exception);
      break;
    }
    case NormalizedCrossCorrelationErrorMetric:
    default:
    {
      status=GetNormalizedCrossCorrelationDistortion(image,reconstruct_image,
        channel,channel_distortion,exception);
      break;
    }
    case PeakAbsoluteErrorMetric:
    {
      status=GetPeakAbsoluteDistortion(image,reconstruct_image,channel,
        channel_distortion,exception);
      break;
    }
    case PeakSignalToNoiseRatioMetric:
    {
      status=GetPeakSignalToNoiseRatio(image,reconstruct_image,channel,
        channel_distortion,exception);
      break;
    }
    case PerceptualHashErrorMetric:
    {
      status=GetPerceptualHashDistortion(image,reconstruct_image,channel,
        channel_distortion,exception);
      break;
    }
    case RootMeanSquaredErrorMetric:
    {
      status=GetRootMeanSquaredDistortion(image,reconstruct_image,channel,
        channel_distortion,exception);
      break;
    }
  }
  *distortion=channel_distortion[CompositeChannels];
  channel_distortion=(double *) RelinquishMagickMemory(channel_distortion);
  (void) FormatImageProperty(image,"distortion","%.*g",GetMagickPrecision(),
    *distortion);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l D i s t o r t i o n s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelDistortions() compares the image channels of an image to a
%  reconstructed image and returns the specified distortion metric for each
%  channel.
%
%  The format of the GetImageChannelDistortions method is:
%
%      double *GetImageChannelDistortions(const Image *image,
%        const Image *reconstruct_image,const MetricType metric,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o reconstruct_image: the reconstruct image.
%
%    o metric: the metric.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport double *GetImageChannelDistortions(Image *image,
  const Image *reconstruct_image,const MetricType metric,
  ExceptionInfo *exception)
{
  double
    *channel_distortion;

  MagickBooleanType
    status;

  size_t
    length;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((reconstruct_image->columns != image->columns) ||
      (reconstruct_image->rows != image->rows))
    {
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        ImageError,"ImageSizeDiffers","`%s'",image->filename);
      return((double *) NULL);
    }
  /*
    Get image distortion.
  */
  length=CompositeChannels+1UL;
  channel_distortion=(double *) AcquireQuantumMemory(length,
    sizeof(*channel_distortion));
  if (channel_distortion == (double *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_distortion,0,length*
    sizeof(*channel_distortion));
  status=MagickTrue;
  switch (metric)
  {
    case AbsoluteErrorMetric:
    {
      status=GetAbsoluteDistortion(image,reconstruct_image,CompositeChannels,
        channel_distortion,exception);
      break;
    }
    case FuzzErrorMetric:
    {
      status=GetFuzzDistortion(image,reconstruct_image,CompositeChannels,
        channel_distortion,exception);
      break;
    }
    case MeanAbsoluteErrorMetric:
    {
      status=GetMeanAbsoluteDistortion(image,reconstruct_image,
        CompositeChannels,channel_distortion,exception);
      break;
    }
    case MeanErrorPerPixelMetric:
    {
      status=GetMeanErrorPerPixel(image,reconstruct_image,CompositeChannels,
        channel_distortion,exception);
      break;
    }
    case MeanSquaredErrorMetric:
    {
      status=GetMeanSquaredDistortion(image,reconstruct_image,CompositeChannels,
        channel_distortion,exception);
      break;
    }
    case NormalizedCrossCorrelationErrorMetric:
    default:
    {
      status=GetNormalizedCrossCorrelationDistortion(image,reconstruct_image,
        CompositeChannels,channel_distortion,exception);
      break;
    }
    case PeakAbsoluteErrorMetric:
    {
      status=GetPeakAbsoluteDistortion(image,reconstruct_image,
        CompositeChannels,channel_distortion,exception);
      break;
    }
    case PeakSignalToNoiseRatioMetric:
    {
      status=GetPeakSignalToNoiseRatio(image,reconstruct_image,
        CompositeChannels,channel_distortion,exception);
      break;
    }
    case PerceptualHashErrorMetric:
    {
      status=GetPerceptualHashDistortion(image,reconstruct_image,
        CompositeChannels,channel_distortion,exception);
      break;
    }
    case RootMeanSquaredErrorMetric:
    {
      status=GetRootMeanSquaredDistortion(image,reconstruct_image,
        CompositeChannels,channel_distortion,exception);
      break;
    }
  }
  if (status == MagickFalse)
    {
      channel_distortion=(double *) RelinquishMagickMemory(channel_distortion);
      return((double *) NULL);
    }
  return(channel_distortion);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s I m a g e s E q u a l                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImagesEqual() measures the difference between colors at each pixel
%  location of two images.  A value other than 0 means the colors match
%  exactly.  Otherwise an error measure is computed by summing over all
%  pixels in an image the distance squared in RGB space between each image
%  pixel and its corresponding pixel in the reconstruct image.  The error
%  measure is assigned to these image members:
%
%    o mean_error_per_pixel:  The mean error for any single pixel in
%      the image.
%
%    o normalized_mean_error:  The normalized mean quantization error for
%      any single pixel in the image.  This distance measure is normalized to
%      a range between 0 and 1.  It is independent of the range of red, green,
%      and blue values in the image.
%
%    o normalized_maximum_error:  The normalized maximum quantization
%      error for any single pixel in the image.  This distance measure is
%      normalized to a range between 0 and 1.  It is independent of the range
%      of red, green, and blue values in your image.
%
%  A small normalized mean square error, accessed as
%  image->normalized_mean_error, suggests the images are very similar in
%  spatial layout and color.
%
%  The format of the IsImagesEqual method is:
%
%      MagickBooleanType IsImagesEqual(Image *image,
%        const Image *reconstruct_image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o reconstruct_image: the reconstruct image.
%
*/
MagickExport MagickBooleanType IsImagesEqual(Image *image,
  const Image *reconstruct_image)
{
  CacheView
    *image_view,
    *reconstruct_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickRealType
    area,
    maximum_error,
    mean_error,
    mean_error_per_pixel;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickSignature);
  if ((reconstruct_image->columns != image->columns) ||
      (reconstruct_image->rows != image->rows))
    ThrowBinaryException(ImageError,"ImageSizeDiffers",image->filename);
  area=0.0;
  maximum_error=0.0;
  mean_error_per_pixel=0.0;
  mean_error=0.0;
  exception=(&image->exception);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes,
      *restrict reconstruct_indexes;

    register const PixelPacket
      *restrict p,
      *restrict q;

    register ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,reconstruct_image->columns,
      1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (const PixelPacket *) NULL))
      break;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    reconstruct_indexes=GetCacheViewVirtualIndexQueue(reconstruct_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      MagickRealType
        distance;

      distance=fabs(GetPixelRed(p)-(double) GetPixelRed(q));
      mean_error_per_pixel+=distance;
      mean_error+=distance*distance;
      if (distance > maximum_error)
        maximum_error=distance;
      area++;
      distance=fabs(GetPixelGreen(p)-(double) GetPixelGreen(q));
      mean_error_per_pixel+=distance;
      mean_error+=distance*distance;
      if (distance > maximum_error)
        maximum_error=distance;
      area++;
      distance=fabs(GetPixelBlue(p)-(double) GetPixelBlue(q));
      mean_error_per_pixel+=distance;
      mean_error+=distance*distance;
      if (distance > maximum_error)
        maximum_error=distance;
      area++;
      if (image->matte != MagickFalse)
        {
          distance=fabs(GetPixelOpacity(p)-(double) GetPixelOpacity(q));
          mean_error_per_pixel+=distance;
          mean_error+=distance*distance;
          if (distance > maximum_error)
            maximum_error=distance;
          area++;
        }
      if ((image->colorspace == CMYKColorspace) &&
          (reconstruct_image->colorspace == CMYKColorspace))
        {
          distance=fabs(GetPixelIndex(indexes+x)-(double)
            GetPixelIndex(reconstruct_indexes+x));
          mean_error_per_pixel+=distance;
          mean_error+=distance*distance;
          if (distance > maximum_error)
            maximum_error=distance;
          area++;
        }
      p++;
      q++;
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  image->error.mean_error_per_pixel=(double) (mean_error_per_pixel/area);
  image->error.normalized_mean_error=(double) (QuantumScale*QuantumScale*
    mean_error/area);
  image->error.normalized_maximum_error=(double) (QuantumScale*maximum_error);
  status=image->error.mean_error_per_pixel == 0.0 ? MagickTrue : MagickFalse;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S i m i l a r i t y I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SimilarityImage() compares the reference image of the image and returns the
%  best match offset.  In addition, it returns a similarity image such that an
%  exact match location is completely white and if none of the pixels match,
%  black, otherwise some gray level in-between.
%
%  The format of the SimilarityImageImage method is:
%
%      Image *SimilarityImage(const Image *image,const Image *reference,
%        RectangleInfo *offset,double *similarity,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o reference: find an area of the image that closely resembles this image.
%
%    o the best match offset of the reference image within the image.
%
%    o similarity: the computed similarity between the images.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static double GetSimilarityMetric(const Image *image,const Image *reference,
  const MetricType metric,const ssize_t x_offset,const ssize_t y_offset,
  ExceptionInfo *exception)
{
  double
    distortion;

  Image
    *similarity_image;

  MagickBooleanType
    status;

  RectangleInfo
    geometry;

  SetGeometry(reference,&geometry);
  geometry.x=x_offset;
  geometry.y=y_offset;
  similarity_image=CropImage(image,&geometry,exception);
  if (similarity_image == (Image *) NULL)
    return(0.0);
  distortion=0.0;
  status=GetImageDistortion(similarity_image,reference,metric,&distortion,
    exception);
  (void) status;
  similarity_image=DestroyImage(similarity_image);
  return(distortion);
}

MagickExport Image *SimilarityImage(Image *image,const Image *reference,
  RectangleInfo *offset,double *similarity_metric,ExceptionInfo *exception)
{
  Image
    *similarity_image;

  similarity_image=SimilarityMetricImage(image,reference,
    RootMeanSquaredErrorMetric,offset,similarity_metric,exception);
  return(similarity_image);
}

MagickExport Image *SimilarityMetricImage(Image *image,const Image *reference,
  const MetricType metric,RectangleInfo *offset,double *similarity_metric,
  ExceptionInfo *exception)
{
#define SimilarityImageTag  "Similarity/Image"

  CacheView
    *similarity_view;

  const char
    *artifact;

  double
    similarity_threshold;

  Image
    *similarity_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  assert(offset != (RectangleInfo *) NULL);
  SetGeometry(reference,offset);
  *similarity_metric=1.0;
  if ((reference->columns > image->columns) || (reference->rows > image->rows))
    ThrowImageException(ImageError,"ImageSizeDiffers");
  similarity_image=CloneImage(image,image->columns-reference->columns+1,
    image->rows-reference->rows+1,MagickTrue,exception);
  if (similarity_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(similarity_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&similarity_image->exception);
      similarity_image=DestroyImage(similarity_image);
      return((Image *) NULL);
    }
  (void) SetImageAlphaChannel(similarity_image,DeactivateAlphaChannel);
  /*
    Measure similarity of reference image against image.
  */
  similarity_threshold=(-1.0);
  artifact=GetImageArtifact(image,"compare:similarity-threshold");
  if (artifact != (const char *) NULL)
    similarity_threshold=StringToDouble(artifact,(char **) NULL);
  status=MagickTrue;
  progress=0;
  similarity_view=AcquireVirtualCacheView(similarity_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) \
    shared(progress,status,similarity_metric) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) (image->rows-reference->rows+1); y++)
  {
    double
      similarity;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp flush(similarity_metric)
#endif
    if (*similarity_metric <= similarity_threshold)
      continue;
    q=GetCacheViewAuthenticPixels(similarity_view,0,y,similarity_image->columns,
      1,exception);
    if (q == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) (image->columns-reference->columns+1); x++)
    {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp flush(similarity_metric)
#endif
      if (*similarity_metric <= similarity_threshold)
        break;
      similarity=GetSimilarityMetric(image,reference,metric,x,y,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp critical (MagickCore_SimilarityImage)
#endif
      if (similarity < *similarity_metric)
        {
          *similarity_metric=similarity;
          offset->x=x;
          offset->y=y;
        }
      SetPixelRed(q,ClampToQuantum(QuantumRange-QuantumRange*similarity));
      SetPixelGreen(q,GetPixelRed(q));
      SetPixelBlue(q,GetPixelRed(q));
      q++;
    }
    if (SyncCacheViewAuthenticPixels(similarity_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_SimilarityImage)
#endif
        proceed=SetImageProgress(image,SimilarityImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  similarity_view=DestroyCacheView(similarity_view);
  if (status == MagickFalse)
    similarity_image=DestroyImage(similarity_image);
  return(similarity_image);
}

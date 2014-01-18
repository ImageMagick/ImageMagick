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
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/compare.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#include "MagickCore/statistic.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
#include "MagickCore/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p a r e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompareImages() compares one or more pixel channels of an image to a
%  reconstructed image and returns the difference image.
%
%  The format of the CompareImages method is:
%
%      Image *CompareImages(const Image *image,const Image *reconstruct_image,
%        const MetricType metric,double *distortion,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o reconstruct_image: the reconstruct image.
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

  PixelInfo
    highlight,
    lowlight;

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
  status=GetImageDistortion(image,reconstruct_image,metric,distortion,
    exception);
  if (status == MagickFalse)
    return((Image *) NULL);
  difference_image=CloneImage(image,0,0,MagickTrue,exception);
  if (difference_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageAlphaChannel(difference_image,OpaqueAlphaChannel,exception);
  highlight_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (highlight_image == (Image *) NULL)
    {
      difference_image=DestroyImage(difference_image);
      return((Image *) NULL);
    }
  status=SetImageStorageClass(highlight_image,DirectClass,exception);
  if (status == MagickFalse)
    {
      difference_image=DestroyImage(difference_image);
      highlight_image=DestroyImage(highlight_image);
      return((Image *) NULL);
    }
  (void) SetImageAlphaChannel(highlight_image,OpaqueAlphaChannel,exception);
  (void) QueryColorCompliance("#f1001ecc",AllCompliance,&highlight,exception);
  artifact=GetImageArtifact(image,"highlight-color");
  if (artifact != (const char *) NULL)
    (void) QueryColorCompliance(artifact,AllCompliance,&highlight,exception);
  (void) QueryColorCompliance("#ffffffcc",AllCompliance,&lowlight,exception);
  artifact=GetImageArtifact(image,"lowlight-color");
  if (artifact != (const char *) NULL)
    (void) QueryColorCompliance(artifact,AllCompliance,&lowlight,exception);
  /*
    Generate difference image.
  */
  status=MagickTrue;
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

    register const Quantum
      *restrict p,
      *restrict q;

    register Quantum
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
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL) ||
        (r == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        Da,
        Sa;

      MagickStatusType
        difference;

      register ssize_t
        i;

      if (GetPixelReadMask(image,p) == 0)
        {
          SetPixelInfoPixel(highlight_image,&lowlight,r);
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          r+=GetPixelChannels(highlight_image);
          continue;
        }
      difference=MagickFalse;
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits=GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=Sa*p[i]-Da*GetPixelChannel(reconstruct_image,channel,q);
        if (fabs((double) distance) >= MagickEpsilon)
          difference=MagickTrue;
      }
      if (difference == MagickFalse)
        SetPixelInfoPixel(highlight_image,&lowlight,r);
      else
        SetPixelInfoPixel(highlight_image,&highlight,r);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
      r+=GetPixelChannels(highlight_image);
    }
    sync=SyncCacheViewAuthenticPixels(highlight_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
  }
  highlight_view=DestroyCacheView(highlight_view);
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  (void) CompositeImage(difference_image,highlight_image,image->compose,
    MagickTrue,0,0,exception);
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
%   G e t I m a g e D i s t o r t i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageDistortion() compares one or more pixel channels of an image to a
%  reconstructed image and returns the specified distortion metric.
%
%  The format of the GetImageDistortion method is:
%
%      MagickBooleanType GetImageDistortion(const Image *image,
%        const Image *reconstruct_image,const MetricType metric,
%        double *distortion,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o reconstruct_image: the reconstruct image.
%
%    o metric: the metric.
%
%    o distortion: the computed distortion between the images.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

static MagickBooleanType GetAbsoluteDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  double
    fuzz;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Compute the absolute difference in pixels between two images.
  */
  status=MagickTrue;
  if (image->fuzz == 0.0)
    fuzz=MagickMax(reconstruct_image->fuzz,MagickSQ1_2)*
      MagickMax(reconstruct_image->fuzz,MagickSQ1_2);
  else
    if (reconstruct_image->fuzz == 0.0)
      fuzz=MagickMax(image->fuzz,MagickSQ1_2)*
        MagickMax(image->fuzz,MagickSQ1_2);
    else
      fuzz=MagickMax(image->fuzz,MagickSQ1_2)*
        MagickMax(reconstruct_image->fuzz,MagickSQ1_2);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      channel_distortion[MaxPixelChannels+1];

    register const Quantum
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
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) ResetMagickMemory(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        Da,
        Sa;

      MagickBooleanType
        difference;

      register ssize_t
        i;

      if (GetPixelReadMask(image,p) == 0)
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      difference=MagickFalse;
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits=GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=Sa*p[i]-Da*GetPixelChannel(reconstruct_image,channel,q);
        if ((distance*distance) > fuzz)
          {
            difference=MagickTrue;
            break;
          }
      }
      if (difference != MagickFalse)
        {
          channel_distortion[i]++;
          channel_distortion[CompositePixelChannel]++;
        }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetAbsoluteError)
#endif
    for (i=0; i <= MaxPixelChannels; i++)
      distortion[i]+=channel_distortion[i];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

static size_t GetImageChannels(const Image *image)
{
  register ssize_t
    i;

  size_t
    channels;

  channels=0;
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel channel=GetPixelChannelChannel(image,i);
    PixelTrait traits=GetPixelChannelTraits(image,channel);
    if ((traits & UpdatePixelTrait) != 0)
      channels++;
  }
  return(channels);
}

static MagickBooleanType GetFuzzDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
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
      channel_distortion[MaxPixelChannels+1];

    register const Quantum
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
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) ResetMagickMemory(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        Da,
        Sa;

      register ssize_t
        i;

      if (GetPixelReadMask(image,p) == 0)
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits=GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=QuantumScale*(Sa*p[i]-Da*GetPixelChannel(reconstruct_image,
          channel,q));
        channel_distortion[i]+=distance*distance;
        channel_distortion[CompositePixelChannel]+=distance*distance;
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetFuzzDistortion)
#endif
    for (i=0; i <= MaxPixelChannels; i++)
      distortion[i]+=channel_distortion[i];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  for (i=0; i <= MaxPixelChannels; i++)
    distortion[i]/=((double) image->columns*image->rows);
  distortion[CompositePixelChannel]/=(double) GetImageChannels(image);
  distortion[CompositePixelChannel]=sqrt(distortion[CompositePixelChannel]);
  return(status);
}

static MagickBooleanType GetMeanAbsoluteDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
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
      channel_distortion[MaxPixelChannels+1];

    register const Quantum
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
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) ResetMagickMemory(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        Da,
        Sa;

      register ssize_t
        i;

      if (GetPixelReadMask(image,p) == 0)
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits=GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=QuantumScale*fabs(Sa*p[i]-Da*GetPixelChannel(reconstruct_image,
          channel,q));
        channel_distortion[i]+=distance;
        channel_distortion[CompositePixelChannel]+=distance;
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMeanAbsoluteError)
#endif
    for (i=0; i <= MaxPixelChannels; i++)
      distortion[i]+=channel_distortion[i];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  for (i=0; i <= MaxPixelChannels; i++)
    distortion[i]/=((double) image->columns*image->rows);
  distortion[CompositePixelChannel]/=(double) GetImageChannels(image);
  return(status);
}

static MagickBooleanType GetMeanErrorPerPixel(Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  MagickBooleanType
    status;

  double
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
    register const Quantum
      *restrict p,
      *restrict q;

    register ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,reconstruct_image->columns,
      1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        break;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        Da,
        Sa;

      register ssize_t
        i;

      if (GetPixelReadMask(image,p) == 0)
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits=GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=fabs((double) (Sa*p[i]-Da*GetPixelChannel(reconstruct_image,
          channel,q)));
        distortion[i]+=distance;
        distortion[CompositePixelChannel]+=distance;
        mean_error+=distance*distance;
        if (distance > maximum_error)
          maximum_error=distance;
        area++;
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  image->error.mean_error_per_pixel=distortion[CompositePixelChannel]/area;
  image->error.normalized_mean_error=QuantumScale*QuantumScale*mean_error/area;
  image->error.normalized_maximum_error=QuantumScale*maximum_error;
  return(status);
}

static MagickBooleanType GetMeanSquaredDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
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
      channel_distortion[MaxPixelChannels+1];

    register const Quantum
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
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) ResetMagickMemory(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        Da,
        Sa;

      register ssize_t
        i;

      if (GetPixelReadMask(image,p) == 0)
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits=GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=QuantumScale*(Sa*p[i]-Da*GetPixelChannel(reconstruct_image,
          channel,q));
        channel_distortion[i]+=distance*distance;
        channel_distortion[CompositePixelChannel]+=distance*distance;
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMeanSquaredError)
#endif
    for (i=0; i <= MaxPixelChannels; i++)
      distortion[i]+=channel_distortion[i];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  for (i=0; i <= MaxPixelChannels; i++)
    distortion[i]/=((double) image->columns*image->rows);
  distortion[CompositePixelChannel]/=GetImageChannels(image);
  return(status);
}

static MagickBooleanType GetNormalizedCrossCorrelationDistortion(
  const Image *image,const Image *reconstruct_image,double *distortion,
  ExceptionInfo *exception)
{
#define SimilarityImageTag  "Similarity/Image"

  CacheView
    *image_view,
    *reconstruct_view;

  ChannelStatistics
    *image_statistics,
    *reconstruct_statistics;

  double
    area;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  register ssize_t
    i;

  ssize_t
    y;

  /*
    Normalize to account for variation due to lighting and exposure condition.
  */
  image_statistics=GetImageStatistics(image,exception);
  reconstruct_statistics=GetImageStatistics(reconstruct_image,exception);
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
  for (i=0; i <= MaxPixelChannels; i++)
    distortion[i]=0.0;
  area=1.0/((double) image->columns*image->rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p,
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,reconstruct_image->columns,
      1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        Da,
        Sa;

      register ssize_t
        i;

      if (GetPixelReadMask(image,p) == 0)
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits=GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distortion[i]+=area*QuantumScale*(Sa*p[i]-image_statistics[i].mean)*
          (Da*GetPixelChannel(reconstruct_image,channel,q)-
          reconstruct_statistics[channel].mean);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
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
  distortion[CompositePixelChannel]=0.0;
  for (i=0; i < MaxPixelChannels; i++)
  {
    double
      gamma;

    PixelChannel channel=GetPixelChannelChannel(image,i);
    gamma=image_statistics[i].standard_deviation*
      reconstruct_statistics[channel].standard_deviation;
    gamma=PerceptibleReciprocal(gamma);
    distortion[i]=QuantumRange*gamma*distortion[i];
    distortion[CompositePixelChannel]+=distortion[i]*distortion[i];
  }
  distortion[CompositePixelChannel]=sqrt(distortion[CompositePixelChannel]/
    GetImageChannels(image));
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
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
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
      channel_distortion[MaxPixelChannels+1];

    register const Quantum
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
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) ResetMagickMemory(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        Da,
        Sa;

      register ssize_t
        i;

      if (GetPixelReadMask(image,p) == 0)
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits=GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=QuantumScale*fabs(Sa*p[i]-Da*GetPixelChannel(reconstruct_image,
          channel,q));
        if (distance > channel_distortion[i])
          channel_distortion[i]=distance;
        if (distance > channel_distortion[CompositePixelChannel])
          channel_distortion[CompositePixelChannel]=distance;
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetPeakAbsoluteError)
#endif
    for (i=0; i <= MaxPixelChannels; i++)
      if (channel_distortion[i] > distortion[i])
        distortion[i]=channel_distortion[i];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

static MagickBooleanType GetPeakSignalToNoiseRatio(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  register ssize_t
    i;

  status=GetMeanSquaredDistortion(image,reconstruct_image,distortion,exception);
  for (i=0; i <= MaxPixelChannels; i++)
    distortion[i]=20.0*log10((double) 1.0/sqrt(distortion[i]));
  return(status);
}

static MagickBooleanType GetPerceptualHashDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
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
  image_moments=GetImageMoments(blur_image,exception);
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
  reconstruct_moments=GetImageMoments(blur_reconstruct,exception);
  if (reconstruct_moments == (ChannelMoments *) NULL)
    {
      image_moments=(ChannelMoments *) RelinquishMagickMemory(image_moments);
      blur_image=DestroyImage(blur_image);
      blur_reconstruct=DestroyImage(blur_reconstruct);
      return(MagickFalse);
    }
  for (i=0; i < 8; i++)
  {
    ssize_t
      channel;

    /*
      Compute sum of moment differences squared.
    */
    if (i == 2)
      continue;  /* I3 is not independent of other moments */
    for (channel=0; channel < MaxPixelChannels; channel++)
    {
      double
        difference;

      if ((fabs(reconstruct_moments[channel].I[i]) < MagickEpsilon) ||
          (fabs(image_moments[channel].I[i]) < MagickEpsilon))
        continue;
      difference=log10(fabs(reconstruct_moments[channel].I[i]))-
         log10(fabs(image_moments[channel].I[i]));
      distortion[channel]+=difference*difference;
      distortion[CompositePixelChannel]+=difference*difference;
    }
  }
  image_moments=(ChannelMoments *) RelinquishMagickMemory(image_moments);
  reconstruct_moments=(ChannelMoments *) RelinquishMagickMemory(
    reconstruct_moments);
  /*
    Compute perceptual hash in the HCLP colorspace.
  */
  if ((TransformImageColorspace(blur_image,HCLpColorspace,exception) == MagickFalse) ||
      (TransformImageColorspace(blur_reconstruct,HCLpColorspace,exception) == MagickFalse))
    {
      blur_reconstruct=DestroyImage(blur_reconstruct);
      blur_image=DestroyImage(blur_image);
      return(MagickFalse);
    }
  image_moments=GetImageMoments(blur_image,exception);
  blur_image=DestroyImage(blur_image);
  if (image_moments == (ChannelMoments *) NULL)
    {
      blur_reconstruct=DestroyImage(blur_reconstruct);
      return(MagickFalse);
    }
  reconstruct_moments=GetImageMoments(blur_reconstruct,exception);
  blur_reconstruct=DestroyImage(blur_reconstruct);
  if (reconstruct_moments == (ChannelMoments *) NULL)
    {
      image_moments=(ChannelMoments *) RelinquishMagickMemory(image_moments);
      return(MagickFalse);
    }
  for (i=0; i < 8; i++)
  {
    ssize_t
      channel;

    /*
      Compute sum of moment differences squared.
    */
    if (i == 2)
      continue;  /* I3 is not independent of other moments */
    for (channel=0; channel < MaxPixelChannels; channel++)
    {
      double
        difference;

      if ((fabs(reconstruct_moments[channel].I[i]) < MagickEpsilon) ||
          (fabs(image_moments[channel].I[i]) < MagickEpsilon))
        continue;
      difference=log10(fabs(reconstruct_moments[channel].I[i]))-
        log10(fabs(image_moments[channel].I[i]));
      distortion[channel]+=difference*difference;
      distortion[CompositePixelChannel]+=difference*difference;
    }
  }
  image_moments=(ChannelMoments *) RelinquishMagickMemory(image_moments);
  reconstruct_moments=(ChannelMoments *) RelinquishMagickMemory(
    reconstruct_moments);
  return(MagickTrue);
}

static MagickBooleanType GetRootMeanSquaredDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  register ssize_t
    i;

  status=GetMeanSquaredDistortion(image,reconstruct_image,distortion,exception);
  for (i=0; i <= MaxPixelChannels; i++)
    distortion[i]=sqrt(distortion[i]);
  return(status);
}

MagickExport MagickBooleanType GetImageDistortion(Image *image,
  const Image *reconstruct_image,const MetricType metric,double *distortion,
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
  length=MaxPixelChannels+1;
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
      status=GetAbsoluteDistortion(image,reconstruct_image,channel_distortion,
        exception);
      break;
    }
    case FuzzErrorMetric:
    {
      status=GetFuzzDistortion(image,reconstruct_image,channel_distortion,
        exception);
      break;
    }
    case MeanAbsoluteErrorMetric:
    {
      status=GetMeanAbsoluteDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case MeanErrorPerPixelErrorMetric:
    {
      status=GetMeanErrorPerPixel(image,reconstruct_image,channel_distortion,
        exception);
      break;
    }
    case MeanSquaredErrorMetric:
    {
      status=GetMeanSquaredDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case NormalizedCrossCorrelationErrorMetric:
    default:
    {
      status=GetNormalizedCrossCorrelationDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case PeakAbsoluteErrorMetric:
    {
      status=GetPeakAbsoluteDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case PeakSignalToNoiseRatioErrorMetric:
    {
      status=GetPeakSignalToNoiseRatio(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case PerceptualHashErrorMetric:
    {
      status=GetPerceptualHashDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case RootMeanSquaredErrorMetric:
    {
      status=GetRootMeanSquaredDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
  }
  *distortion=channel_distortion[CompositePixelChannel];
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
%   G e t I m a g e D i s t o r t i o n s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageDistortions() compares the pixel channels of an image to a
%  reconstructed image and returns the specified distortion metric for each
%  channel.
%
%  The format of the GetImageDistortions method is:
%
%      double *GetImageDistortions(const Image *image,
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
MagickExport double *GetImageDistortions(Image *image,
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
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ImageSizeDiffers","`%s'",image->filename);
      return((double *) NULL);
    }
  /*
    Get image distortion.
  */
  length=MaxPixelChannels+1UL;
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
      status=GetAbsoluteDistortion(image,reconstruct_image,channel_distortion,
        exception);
      break;
    }
    case FuzzErrorMetric:
    {
      status=GetFuzzDistortion(image,reconstruct_image,channel_distortion,
        exception);
      break;
    }
    case MeanAbsoluteErrorMetric:
    {
      status=GetMeanAbsoluteDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case MeanErrorPerPixelErrorMetric:
    {
      status=GetMeanErrorPerPixel(image,reconstruct_image,channel_distortion,
        exception);
      break;
    }
    case MeanSquaredErrorMetric:
    {
      status=GetMeanSquaredDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case NormalizedCrossCorrelationErrorMetric:
    default:
    {
      status=GetNormalizedCrossCorrelationDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case PeakAbsoluteErrorMetric:
    {
      status=GetPeakAbsoluteDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case PeakSignalToNoiseRatioErrorMetric:
    {
      status=GetPeakSignalToNoiseRatio(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case PerceptualHashErrorMetric:
    {
      status=GetRootMeanSquaredDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case RootMeanSquaredErrorMetric:
    {
      status=GetRootMeanSquaredDistortion(image,reconstruct_image,
        channel_distortion,exception);
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
%        const Image *reconstruct_image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o reconstruct_image: the reconstruct image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsImagesEqual(Image *image,
  const Image *reconstruct_image,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  MagickBooleanType
    status;

  double
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
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p,
      *restrict q;

    register ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,reconstruct_image->columns,
      1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      if (GetPixelReadMask(image,p) == 0)
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits=GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=fabs(p[i]-(double) GetPixelChannel(reconstruct_image,
          channel,q));
        mean_error_per_pixel+=distance;
        mean_error+=distance*distance;
        if (distance > maximum_error)
          maximum_error=distance;
        area++;
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
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
%        const MetricType metric,const double similarity_threshold,
%        RectangleInfo *offset,double *similarity,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o reference: find an area of the image that closely resembles this image.
%
%    o metric: the metric.
%
%    o similarity_threshold: minimum distortion for (sub)image match.
%
%    o offset: the best match offset of the reference image within the image.
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
  similarity_image=DestroyImage(similarity_image);
  if (status == MagickFalse)
    return(0.0);
  return(distortion);
}

MagickExport Image *SimilarityImage(Image *image,const Image *reference,
  const MetricType metric,const double similarity_threshold,
  RectangleInfo *offset,double *similarity_metric,ExceptionInfo *exception)
{
#define SimilarityImageTag  "Similarity/Image"

  CacheView
    *similarity_view;

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
  status=SetImageStorageClass(similarity_image,DirectClass,exception);
  if (status == MagickFalse)
    {
      similarity_image=DestroyImage(similarity_image);
      return((Image *) NULL);
    }
  (void) SetImageAlphaChannel(similarity_image,DeactivateAlphaChannel,
    exception);
  /*
    Measure similarity of reference image against image.
  */
  status=MagickTrue;
  progress=0;
  similarity_view=AcquireAuthenticCacheView(similarity_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) \
    shared(progress,status,similarity_metric) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) (image->rows-reference->rows+1); y++)
  {
    double
      similarity;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp flush(similarity_metric)
#endif
    if (*similarity_metric <= similarity_threshold)
      continue;
    q=GetCacheViewAuthenticPixels(similarity_view,0,y,similarity_image->columns,
      1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) (image->columns-reference->columns+1); x++)
    {
      register ssize_t
        i;

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
          offset->x=x;
          offset->y=y;
          *similarity_metric=similarity;
        }
      if (GetPixelReadMask(similarity_image,q) == 0)
        {
          SetPixelBackgoundColor(similarity_image,q);
          q+=GetPixelChannels(similarity_image);
          continue;
        }
      for (i=0; i < (ssize_t) GetPixelChannels(similarity_image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        PixelTrait similarity_traits=GetPixelChannelTraits(similarity_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (similarity_traits == UndefinedPixelTrait) ||
            ((similarity_traits & UpdatePixelTrait) == 0))
          continue;
        SetPixelChannel(similarity_image,channel,ClampToQuantum(QuantumRange-
          QuantumRange*similarity),q);
      }
      q+=GetPixelChannels(similarity_image);
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

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
%                                  Cristy                                     %
%                               December 2003                                 %
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
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
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
#include "MagickCore/enhance.h"
#include "MagickCore/fourier.h"
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
#include "MagickCore/registry.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#include "MagickCore/statistic.h"
#include "MagickCore/statistic-private.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
#include "MagickCore/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p a r e I m a g e s                                                 %
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

  double
    fuzz;

  Image
    *clone_image,
    *difference_image,
    *highlight_image;

  MagickBooleanType
    status;

  PixelInfo
    highlight,
    lowlight,
    masklight;

  RectangleInfo
    geometry;

  size_t
    columns,
    rows;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickCoreSignature);
  assert(distortion != (double *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *distortion=0.0;
  status=GetImageDistortion(image,reconstruct_image,metric,distortion,
    exception);
  if (status == MagickFalse)
    return((Image *) NULL);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  rows=MagickMax(image->rows,reconstruct_image->rows);
  SetGeometry(image,&geometry);
  geometry.width=columns;
  geometry.height=rows;
  clone_image=CloneImage(image,0,0,MagickTrue,exception);
  if (clone_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageMask(clone_image,ReadPixelMask,(Image *) NULL,exception);
  difference_image=ExtentImage(clone_image,&geometry,exception);
  clone_image=DestroyImage(clone_image);
  if (difference_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageAlphaChannel(difference_image,OpaqueAlphaChannel,exception);
  highlight_image=CloneImage(image,columns,rows,MagickTrue,exception);
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
  (void) SetImageMask(highlight_image,ReadPixelMask,(Image *) NULL,exception);
  (void) SetImageAlphaChannel(highlight_image,OpaqueAlphaChannel,exception);
  (void) QueryColorCompliance("#f1001ecc",AllCompliance,&highlight,exception);
  artifact=GetImageArtifact(image,"compare:highlight-color");
  if (artifact != (const char *) NULL)
    (void) QueryColorCompliance(artifact,AllCompliance,&highlight,exception);
  (void) QueryColorCompliance("#ffffffcc",AllCompliance,&lowlight,exception);
  artifact=GetImageArtifact(image,"compare:lowlight-color");
  if (artifact != (const char *) NULL)
    (void) QueryColorCompliance(artifact,AllCompliance,&lowlight,exception);
  (void) QueryColorCompliance("#888888cc",AllCompliance,&masklight,exception);
  artifact=GetImageArtifact(image,"compare:masklight-color");
  if (artifact != (const char *) NULL)
    (void) QueryColorCompliance(artifact,AllCompliance,&masklight,exception);
  /*
    Generate difference image.
  */
  status=MagickTrue;
  fuzz=GetFuzzyColorDistance(image,reconstruct_image);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  highlight_view=AcquireAuthenticCacheView(highlight_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,highlight_image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    MagickBooleanType
      sync;

    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    Quantum
      *magick_restrict r;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    r=QueueCacheViewAuthenticPixels(highlight_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL) ||
        (r == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        Da,
        Sa;

      MagickStatusType
        difference;

      ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          SetPixelViaPixelInfo(highlight_image,&masklight,r);
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          r+=GetPixelChannels(highlight_image);
          continue;
        }
      difference=MagickFalse;
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance,
          pixel;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          pixel=(double) p[i]-(double) GetPixelChannel(reconstruct_image,
            channel,q);
        else
          pixel=Sa*(double) p[i]-Da*(double)
            GetPixelChannel(reconstruct_image,channel,q);
        distance=pixel*pixel;
        if (distance >= fuzz)
          {
            difference=MagickTrue;
            break;
          }
      }
      if (difference == MagickFalse)
        SetPixelViaPixelInfo(highlight_image,&lowlight,r);
      else
        SetPixelViaPixelInfo(highlight_image,&highlight,r);
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

  size_t
    columns,
    rows;

  ssize_t
    y;

  /*
    Compute the absolute difference in pixels between two images.
  */
  status=MagickTrue;
  fuzz=GetFuzzyColorDistance(image,reconstruct_image);
  rows=MagickMax(image->rows,reconstruct_image->rows);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    double
      channel_distortion[MaxPixelChannels+1];

    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    ssize_t
      j,
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) memset(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        Da,
        Sa;

      MagickBooleanType
        difference;

      ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      difference=MagickFalse;
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance,
          pixel;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          pixel=(double) p[i]-(double) GetPixelChannel(reconstruct_image,
            channel,q);
        else
          pixel=Sa*(double) p[i]-Da*(double) GetPixelChannel(reconstruct_image,
            channel,q);
        distance=pixel*pixel;
        if (distance >= fuzz)
          {
            channel_distortion[i]++;
            difference=MagickTrue;
          }
      }
      if (difference != MagickFalse)
        channel_distortion[CompositePixelChannel]++;
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetAbsoluteDistortion)
#endif
    for (j=0; j <= MaxPixelChannels; j++)
      distortion[j]+=channel_distortion[j];
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

static MagickBooleanType GetFuzzDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  double
    area;

  MagickBooleanType
    status;

  ssize_t
    j;

  size_t
    columns,
    rows;

  ssize_t
    y;

  status=MagickTrue;
  rows=MagickMax(image->rows,reconstruct_image->rows);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  area=0.0;
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    double
      channel_distortion[MaxPixelChannels+1];

    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    size_t
      local_area = 0;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) memset(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        Da,
        Sa;

      ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=QuantumScale*((double) p[i]-(double) GetPixelChannel(
            reconstruct_image,channel,q));
        else
          distance=QuantumScale*(Sa*(double) p[i]-Da*(double) GetPixelChannel(
            reconstruct_image,channel,q));
        channel_distortion[i]+=distance*distance;
        channel_distortion[CompositePixelChannel]+=distance*distance;
      }
      local_area++;
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetFuzzDistortion)
#endif
    {
      area+=local_area;
      for (j=0; j <= MaxPixelChannels; j++)
        distortion[j]+=channel_distortion[j];
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  area=PerceptibleReciprocal(area);
  for (j=0; j <= MaxPixelChannels; j++)
    distortion[j]*=area;
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

  double
    area;

  MagickBooleanType
    status;

  ssize_t
    j;

  size_t
    columns,
    rows;

  ssize_t
    y;

  status=MagickTrue;
  rows=MagickMax(image->rows,reconstruct_image->rows);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  area=0.0;
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    double
      channel_distortion[MaxPixelChannels+1];

    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    size_t
      local_area = 0;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) memset(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        Da,
        Sa;

      ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=QuantumScale*fabs((double) p[i]-(double)
            GetPixelChannel(reconstruct_image,channel,q));
        else
          distance=QuantumScale*fabs(Sa*(double) p[i]-Da*(double)
            GetPixelChannel(reconstruct_image,channel,q));
        channel_distortion[i]+=distance;
        channel_distortion[CompositePixelChannel]+=distance;
      }
      local_area++;
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMeanAbsoluteError)
#endif
    {
      area+=local_area;
      for (j=0; j <= MaxPixelChannels; j++)
        distortion[j]+=channel_distortion[j];
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  area=PerceptibleReciprocal(area);
  for (j=0; j <= MaxPixelChannels; j++)
    distortion[j]*=area;
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

  size_t
    columns,
    rows;

  ssize_t
    y;

  status=MagickTrue;
  area=0.0;
  maximum_error=0.0;
  mean_error=0.0;
  rows=MagickMax(image->rows,reconstruct_image->rows);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        break;
      }
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        Da,
        Sa;

      ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=fabs((double) p[i]-(double)
            GetPixelChannel(reconstruct_image,channel,q));
        else
          distance=fabs(Sa*(double) p[i]-Da*(double)
            GetPixelChannel(reconstruct_image,channel,q));
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
  area=PerceptibleReciprocal(area);
  image->error.mean_error_per_pixel=area*distortion[CompositePixelChannel];
  image->error.normalized_mean_error=area*QuantumScale*QuantumScale*mean_error;
  image->error.normalized_maximum_error=QuantumScale*maximum_error;
  return(status);
}

static MagickBooleanType GetMeanSquaredDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  double
    area;

  MagickBooleanType
    status;

  size_t
    columns,
    rows;

  ssize_t
    j,
    y;

  status=MagickTrue;
  rows=MagickMax(image->rows,reconstruct_image->rows);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  area=0.0;
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_distortion[MaxPixelChannels+1];

    size_t
      local_area = 0;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) memset(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        Da,
        Sa;

      ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=QuantumScale*((double) p[i]-(double) GetPixelChannel(
            reconstruct_image,channel,q));
        else
          distance=QuantumScale*(Sa*(double) p[i]-Da*(double) GetPixelChannel(
            reconstruct_image,channel,q));
        channel_distortion[i]+=distance*distance;
        channel_distortion[CompositePixelChannel]+=distance*distance;
      }
      local_area++;
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMeanSquaredError)
#endif
    {
      area+=local_area;
      for (j=0; j <= MaxPixelChannels; j++)
        distortion[j]+=channel_distortion[j];
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  area=PerceptibleReciprocal(area);
  for (j=0; j <= MaxPixelChannels; j++)
    distortion[j]*=area;
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
    alpha_variance[MaxPixelChannels+1],
    beta_variance[MaxPixelChannels+1];

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  size_t
    columns,
    rows;

  ssize_t
    i,
    y;

  /*
    Normalized cross correlation distortion.
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
  (void) memset(distortion,0,(MaxPixelChannels+1)*sizeof(*distortion));
  (void) memset(alpha_variance,0,(MaxPixelChannels+1)*sizeof(*alpha_variance));
  (void) memset(beta_variance,0,(MaxPixelChannels+1)*sizeof(*beta_variance));
  status=MagickTrue;
  progress=0;
  columns=MagickMax(image->columns,reconstruct_image->columns);
  rows=MagickMax(image->rows,reconstruct_image->rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        break;
      }
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        Da,
        Sa;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          alpha,
          beta;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          {
            alpha=QuantumScale*((double) p[i]-image_statistics[channel].mean);
            beta=QuantumScale*((double) GetPixelChannel(reconstruct_image,
              channel,q)-reconstruct_statistics[channel].mean);
          }
        else
          {
            alpha=QuantumScale*(Sa*(double) p[i]-
              image_statistics[channel].mean);
            beta=QuantumScale*(Da*(double) GetPixelChannel(reconstruct_image,
              channel,q)-reconstruct_statistics[channel].mean);
          }
        distortion[i]+=alpha*beta;
        alpha_variance[i]+=alpha*alpha;
        beta_variance[i]+=beta*beta;
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SimilarityImageTag,progress,rows);
        if (proceed == MagickFalse)
          {
            status=MagickFalse;
            break;
          }
      }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  /*
    Compute normalized cross correlation: divide by standard deviation.
  */
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    distortion[i]/=sqrt(alpha_variance[i]*beta_variance[i]);
    if (fabs(distortion[i]) > MagickEpsilon)
      distortion[CompositePixelChannel]+=distortion[i];
  }
  distortion[CompositePixelChannel]=distortion[CompositePixelChannel]/
    GetImageChannels(image);
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

  size_t
    columns,
    rows;

  ssize_t
    y;

  status=MagickTrue;
  rows=MagickMax(image->rows,reconstruct_image->rows);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    double
      channel_distortion[MaxPixelChannels+1];

    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    ssize_t
      j,
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) memset(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        Da,
        Sa;

      ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=QuantumScale*fabs((double) p[i]-(double)
            GetPixelChannel(reconstruct_image,channel,q));
        else
          distance=QuantumScale*fabs(Sa*(double) p[i]-Da*(double)
            GetPixelChannel(reconstruct_image,channel,q));
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
    for (j=0; j <= MaxPixelChannels; j++)
      if (channel_distortion[j] > distortion[j])
        distortion[j]=channel_distortion[j];
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

  ssize_t
    i;

  status=GetMeanSquaredDistortion(image,reconstruct_image,distortion,exception);
  for (i=0; i <= MaxPixelChannels; i++)
    if (fabs(distortion[i]) >= MagickEpsilon)
      distortion[i]=(-10.0*MagickLog10(distortion[i]));
  return(status);
}

static MagickBooleanType GetPerceptualHashDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
  ChannelPerceptualHash
    *channel_phash,
    *reconstruct_phash;

  const char
    *artifact;

  ssize_t
    channel;

  /*
    Compute perceptual hash in the sRGB colorspace.
  */
  channel_phash=GetImagePerceptualHash(image,exception);
  if (channel_phash == (ChannelPerceptualHash *) NULL)
    return(MagickFalse);
  reconstruct_phash=GetImagePerceptualHash(reconstruct_image,exception);
  if (reconstruct_phash == (ChannelPerceptualHash *) NULL)
    {
      channel_phash=(ChannelPerceptualHash *) RelinquishMagickMemory(
        channel_phash);
      return(MagickFalse);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static)
#endif
  for (channel=0; channel < MaxPixelChannels; channel++)
  {
    double
      difference;

    ssize_t
      i;

    difference=0.0;
    for (i=0; i < MaximumNumberOfImageMoments; i++)
    {
      double
        alpha,
        beta;

      ssize_t
        j;

      for (j=0; j < (ssize_t) channel_phash[0].number_colorspaces; j++)
      {
        alpha=channel_phash[channel].phash[j][i];
        beta=reconstruct_phash[channel].phash[j][i];
        difference+=(beta-alpha)*(beta-alpha);
      }
    }
    distortion[channel]+=difference;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetPerceptualHashDistortion)
#endif
    distortion[CompositePixelChannel]+=difference;
  }
  artifact=GetImageArtifact(image,"phash:normalize");
  if ((artifact != (const char *) NULL) &&
      (IsStringTrue(artifact) != MagickFalse))
    {
      ssize_t
        j;

      for (j=0; j <= MaxPixelChannels; j++)
        distortion[j]=sqrt(distortion[j]/channel_phash[0].number_channels);
    }
  /*
    Free resources.
  */
  reconstruct_phash=(ChannelPerceptualHash *) RelinquishMagickMemory(
    reconstruct_phash);
  channel_phash=(ChannelPerceptualHash *) RelinquishMagickMemory(channel_phash);
  return(MagickTrue);
}

static MagickBooleanType GetRootMeanSquaredDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  ssize_t
    i;

  status=GetMeanSquaredDistortion(image,reconstruct_image,distortion,exception);
  for (i=0; i <= MaxPixelChannels; i++)
    distortion[i]=sqrt(distortion[i]);
  return(status);
}

static MagickBooleanType GetStructuralSimilarityDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
#define SSIMRadius  5.0
#define SSIMSigma  1.5
#define SSIMBlocksize  8
#define SSIMK1  0.01
#define SSIMK2  0.03
#define SSIML  1.0

  CacheView
    *image_view,
    *reconstruct_view;

  char
    geometry[MagickPathExtent];

  const char
    *artifact;

  double
    area,
    c1,
    c2,
    radius,
    sigma;

  KernelInfo
    *kernel_info;

  MagickBooleanType
    status;

  ssize_t
    j;

  size_t
    columns,
    rows;

  ssize_t
    y;

  /*
    Compute structural similarity index @
    https://en.wikipedia.org/wiki/Structural_similarity.
  */
  radius=SSIMRadius;
  artifact=GetImageArtifact(image,"compare:ssim-radius");
  if (artifact != (const char *) NULL)
    radius=StringToDouble(artifact,(char **) NULL);
  sigma=SSIMSigma;
  artifact=GetImageArtifact(image,"compare:ssim-sigma");
  if (artifact != (const char *) NULL)
    sigma=StringToDouble(artifact,(char **) NULL);
  (void) FormatLocaleString(geometry,MagickPathExtent,"gaussian:%.20gx%.20g",
    radius,sigma);
  kernel_info=AcquireKernelInfo(geometry,exception);
  if (kernel_info == (KernelInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  c1=pow(SSIMK1*SSIML,2.0);
  artifact=GetImageArtifact(image,"compare:ssim-k1");
  if (artifact != (const char *) NULL)
    c1=pow(StringToDouble(artifact,(char **) NULL)*SSIML,2.0);
  c2=pow(SSIMK2*SSIML,2.0);
  artifact=GetImageArtifact(image,"compare:ssim-k2");
  if (artifact != (const char *) NULL)
    c2=pow(StringToDouble(artifact,(char **) NULL)*SSIML,2.0);
  status=MagickTrue;
  area=0.0;
  rows=MagickMax(image->rows,reconstruct_image->rows);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,reconstruct_image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    double
      channel_distortion[MaxPixelChannels+1];

    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    size_t
      local_area = 0;

    ssize_t
      i,
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) kernel_info->width/2L),y-
      ((ssize_t) kernel_info->height/2L),columns+kernel_info->width,
      kernel_info->height,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,-((ssize_t) kernel_info->width/
      2L),y-((ssize_t) kernel_info->height/2L),columns+kernel_info->width,
      kernel_info->height,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    (void) memset(channel_distortion,0,sizeof(channel_distortion));
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        x_pixel_mu[MaxPixelChannels+1],
        x_pixel_sigma_squared[MaxPixelChannels+1],
        xy_sigma[MaxPixelChannels+1],
        y_pixel_mu[MaxPixelChannels+1],
        y_pixel_sigma_squared[MaxPixelChannels+1];

      const Quantum
        *magick_restrict reference,
        *magick_restrict target;

      MagickRealType
        *k;

      ssize_t
        v;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      (void) memset(x_pixel_mu,0,sizeof(x_pixel_mu));
      (void) memset(x_pixel_sigma_squared,0,sizeof(x_pixel_sigma_squared));
      (void) memset(xy_sigma,0,sizeof(xy_sigma));
      (void) memset(x_pixel_sigma_squared,0,sizeof(y_pixel_sigma_squared));
      (void) memset(y_pixel_mu,0,sizeof(y_pixel_mu));
      (void) memset(y_pixel_sigma_squared,0,sizeof(y_pixel_sigma_squared));
      k=kernel_info->values;
      reference=p;
      target=q;
      for (v=0; v < (ssize_t) kernel_info->height; v++)
      {
        ssize_t
          u;

        for (u=0; u < (ssize_t) kernel_info->width; u++)
        {
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            double
              x_pixel,
              y_pixel;

            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            PixelTrait reconstruct_traits = GetPixelChannelTraits(
              reconstruct_image,channel);
            if ((traits == UndefinedPixelTrait) ||
                (reconstruct_traits == UndefinedPixelTrait) ||
                ((reconstruct_traits & UpdatePixelTrait) == 0))
              continue;
            x_pixel=QuantumScale*(double) reference[i];
            x_pixel_mu[i]+=(*k)*x_pixel;
            x_pixel_sigma_squared[i]+=(*k)*x_pixel*x_pixel;
            y_pixel=QuantumScale*(double)
              GetPixelChannel(reconstruct_image,channel,target);
            y_pixel_mu[i]+=(*k)*y_pixel;
            y_pixel_sigma_squared[i]+=(*k)*y_pixel*y_pixel;
            xy_sigma[i]+=(*k)*x_pixel*y_pixel;
          }
          k++;
          reference+=GetPixelChannels(image);
          target+=GetPixelChannels(reconstruct_image);
        }
        reference+=GetPixelChannels(image)*columns;
        target+=GetPixelChannels(reconstruct_image)*columns;
      }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          ssim,
          x_pixel_mu_squared,
          x_pixel_sigmas_squared,
          xy_mu,
          xy_sigmas,
          y_pixel_mu_squared,
          y_pixel_sigmas_squared;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(
          reconstruct_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        x_pixel_mu_squared=x_pixel_mu[i]*x_pixel_mu[i];
        y_pixel_mu_squared=y_pixel_mu[i]*y_pixel_mu[i];
        xy_mu=x_pixel_mu[i]*y_pixel_mu[i];
        xy_sigmas=xy_sigma[i]-xy_mu;
        x_pixel_sigmas_squared=x_pixel_sigma_squared[i]-x_pixel_mu_squared;
        y_pixel_sigmas_squared=y_pixel_sigma_squared[i]-y_pixel_mu_squared;
        ssim=((2.0*xy_mu+c1)*(2.0*xy_sigmas+c2))/
          ((x_pixel_mu_squared+y_pixel_mu_squared+c1)*
           (x_pixel_sigmas_squared+y_pixel_sigmas_squared+c2));
        channel_distortion[i]+=ssim;
        channel_distortion[CompositePixelChannel]+=ssim;
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
      local_area++;
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetStructuralSimilarityDistortion)
#endif
    {
      area+=local_area;
      for (i=0; i <= MaxPixelChannels; i++)
        distortion[i]+=channel_distortion[i];
    }
  }
  image_view=DestroyCacheView(image_view);
  reconstruct_view=DestroyCacheView(reconstruct_view);
  for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,j);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if ((traits == UndefinedPixelTrait) || ((traits & UpdatePixelTrait) == 0))
      continue;
    distortion[j]/=area;
  }
  distortion[CompositePixelChannel]/=area;
  distortion[CompositePixelChannel]/=(double) GetImageChannels(image);
  kernel_info=DestroyKernelInfo(kernel_info);
  return(status);
}

static MagickBooleanType GetStructuralDisimilarityDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  ssize_t
    i;

  status=GetStructuralSimilarityDistortion(image,reconstruct_image,
    distortion,exception);
  for (i=0; i <= MaxPixelChannels; i++)
    distortion[i]=(1.0-(distortion[i]))/2.0;
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
  assert(image->signature == MagickCoreSignature);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickCoreSignature);
  assert(distortion != (double *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *distortion=0.0;
  /*
    Get image distortion.
  */
  length=MaxPixelChannels+1UL;
  channel_distortion=(double *) AcquireQuantumMemory(length,
    sizeof(*channel_distortion));
  if (channel_distortion == (double *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(channel_distortion,0,length*
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
    case StructuralSimilarityErrorMetric:
    {
      status=GetStructuralSimilarityDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case StructuralDissimilarityErrorMetric:
    {
      status=GetStructuralDisimilarityDistortion(image,reconstruct_image,
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
  assert(image->signature == MagickCoreSignature);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  /*
    Get image distortion.
  */
  length=MaxPixelChannels+1UL;
  channel_distortion=(double *) AcquireQuantumMemory(length,
    sizeof(*channel_distortion));
  if (channel_distortion == (double *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(channel_distortion,0,length*
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
    case StructuralSimilarityErrorMetric:
    {
      status=GetStructuralSimilarityDistortion(image,reconstruct_image,
        channel_distortion,exception);
      break;
    }
    case StructuralDissimilarityErrorMetric:
    {
      status=GetStructuralDisimilarityDistortion(image,reconstruct_image,
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
%  IsImagesEqual() compare the pixels of two images and returns immediately
%  if any pixel is not identical.
%
%  The format of the IsImagesEqual method is:
%
%      MagickBooleanType IsImagesEqual(const Image *image,
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
MagickExport MagickBooleanType IsImagesEqual(const Image *image,
  const Image *reconstruct_image,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  size_t
    columns,
    rows;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickCoreSignature);
  rows=MagickMax(image->rows,reconstruct_image->rows);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      break;
    for (x=0; x < (ssize_t) columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=fabs((double) p[i]-(double) GetPixelChannel(reconstruct_image,
          channel,q));
        if (distance >= MagickEpsilon)
          break;
      }
      if (i < (ssize_t) GetPixelChannels(image))
        break;
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
    if (x < (ssize_t) columns)
      break;
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  return(y < (ssize_t) rows ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  S e t I m a g e C o l o r M e t r i c                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageColorMetric() measures the difference between colors at each pixel
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
%  The format of the SetImageColorMetric method is:
%
%      MagickBooleanType SetImageColorMetric(Image *image,
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
MagickExport MagickBooleanType SetImageColorMetric(Image *image,
  const Image *reconstruct_image,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  double
    area,
    maximum_error,
    mean_error,
    mean_error_per_pixel;

  MagickBooleanType
    status;

  size_t
    columns,
    rows;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickCoreSignature);
  area=0.0;
  maximum_error=0.0;
  mean_error_per_pixel=0.0;
  mean_error=0.0;
  rows=MagickMax(image->rows,reconstruct_image->rows);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      break;
    for (x=0; x < (ssize_t) columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          distance;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=fabs((double) p[i]-(double) GetPixelChannel(reconstruct_image,
          channel,q));
        if (distance >= MagickEpsilon)
          {
            mean_error_per_pixel+=distance;
            mean_error+=distance*distance;
            if (distance > maximum_error)
              maximum_error=distance;
          }
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

#if defined(MAGICKCORE_HDRI_SUPPORT) && defined(MAGICKCORE_FFTW_DELEGATE)
static Image *CrossCorrelationImage(const Image *alpha_image,
  const Image *beta_image,ExceptionInfo *exception)
{
  Image
    *clone_image,
    *complex_conjugate,
    *complex_multiplication,
    *cross_correlation,
    *fft_images;

  /*
    Take the FFT of beta image.
  */
  clone_image=CloneImage(beta_image,0,0,MagickTrue,exception);
  if (clone_image == (Image *) NULL)
    return(clone_image);
  (void) SetImageArtifact(clone_image,"fourier:normalize","inverse");
  fft_images=ForwardFourierTransformImage(clone_image,MagickFalse,
    exception);
  clone_image=DestroyImageList(clone_image);
  if (fft_images == (Image *) NULL)
    return(fft_images);
  /*
    Take the complex conjugate of beta image.
  */
  complex_conjugate=ComplexImages(fft_images,ConjugateComplexOperator,
    exception);
  fft_images=DestroyImageList(fft_images);
  if (complex_conjugate == (Image *) NULL)
    return(complex_conjugate);
  /*
    Take the FFT of the alpha image.
  */
  clone_image=CloneImage(alpha_image,0,0,MagickTrue,exception);
  if (clone_image == (Image *) NULL)
    {
      complex_conjugate=DestroyImageList(complex_conjugate);
      return(clone_image);
    }
  (void) SetImageArtifact(clone_image,"fourier:normalize","inverse");
  fft_images=ForwardFourierTransformImage(clone_image,MagickFalse,exception);
  clone_image=DestroyImageList(clone_image);
  if (fft_images == (Image *) NULL)
    {
      complex_conjugate=DestroyImageList(complex_conjugate);
      return(fft_images);
    }
  complex_conjugate->next->next=fft_images;
  /*
    Do complex multiplication.
  */
  DisableCompositeClampUnlessSpecified(complex_conjugate);
  complex_multiplication=ComplexImages(complex_conjugate,
    MultiplyComplexOperator,exception);
  complex_conjugate=DestroyImageList(complex_conjugate);
  if (fft_images == (Image *) NULL)
    return(fft_images);
  /*
    Do the IFT and return the cross-correlation result.
  */
  cross_correlation=InverseFourierTransformImage(complex_multiplication,
    complex_multiplication->next,MagickFalse,exception);
  complex_multiplication=DestroyImageList(complex_multiplication);
  return(cross_correlation);
}

static Image *NCCDivideImage(const Image *alpha_image,const Image *beta_image,
  ExceptionInfo *exception)
{
  CacheView
    *alpha_view,
    *beta_view;

  Image
    *divide_image;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Divide one image into another.
  */
  divide_image=CloneImage(alpha_image,0,0,MagickTrue,exception);
  if (divide_image == (Image *) NULL)
    return(divide_image);
  status=MagickTrue;
  alpha_view=AcquireAuthenticCacheView(divide_image,exception);
  beta_view=AcquireVirtualCacheView(beta_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(beta_image,divide_image,divide_image->rows,1)
#endif
  for (y=0; y < (ssize_t) divide_image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(beta_view,0,y,beta_image->columns,1,
      exception);
    q=GetCacheViewAuthenticPixels(alpha_view,0,y,divide_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) divide_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(divide_image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(divide_image,i);
        PixelTrait traits = GetPixelChannelTraits(divide_image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if (fabs(p[i]) >= MagickEpsilon)
          q[i]=(Quantum) ((double) q[i]*PerceptibleReciprocal(QuantumScale*
            (double) p[i]));
      }
      p+=GetPixelChannels(beta_image);
      q+=GetPixelChannels(divide_image);
    }
    if (SyncCacheViewAuthenticPixels(alpha_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  beta_view=DestroyCacheView(beta_view);
  alpha_view=DestroyCacheView(alpha_view);
  if (status == MagickFalse)
    divide_image=DestroyImage(divide_image);
  return(divide_image);
}

static MagickBooleanType NCCMaximaImage(const Image *image,double *maxima,
  RectangleInfo *offset,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Identify the maxima value in the image and its location.
  */
  status=MagickTrue;
  *maxima=0.0;
  offset->x=0;
  offset->y=0;
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
        sum = 0.0;

      ssize_t
        channels = 0,
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        sum+=(double) p[i];
        channels++;
      }
      if ((channels != 0) && ((sum/channels) > *maxima))
        {
          *maxima=sum/channels;
          offset->x=x;
          offset->y=y;
        }
      p+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

static MagickBooleanType NCCMultiplyImage(Image *image,const double factor,
  const ChannelStatistics *channel_statistics,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Multiply each pixel by a factor.
  */
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
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
        if (channel_statistics != (const ChannelStatistics *) NULL)
          q[i]=(Quantum) ((double) q[i]*QuantumScale*
            channel_statistics[channel].standard_deviation);
        q[i]=(Quantum) ((double) q[i]*factor);
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

static Image *NCCSquareImage(const Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  Image
    *square_image;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Square each pixel in the image.
  */
  square_image=CloneImage(image,0,0,MagickTrue,exception);
  if (square_image == (Image *) NULL)
    return(square_image);
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(square_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(square_image,square_image,square_image->rows,1)
#endif
  for (y=0; y < (ssize_t) square_image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,square_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) square_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(square_image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(square_image,i);
        PixelTrait traits = GetPixelChannelTraits(square_image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=(Quantum) ((double) q[i]*QuantumScale*(double) q[i]);
      }
      q+=GetPixelChannels(square_image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    square_image=DestroyImage(square_image);
  return(square_image);
}

static Image *NCCSubtractImageMean(const Image *alpha_image,
  const Image *beta_image,const ChannelStatistics *channel_statistics,
  ExceptionInfo *exception)
{
  CacheView
    *beta_view,
    *image_view;

  Image
    *gamma_image;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Subtract the image mean and pad.
  */
  gamma_image=CloneImage(beta_image,alpha_image->columns,alpha_image->rows,
    MagickTrue,exception);
  if (gamma_image == (Image *) NULL)
    return(gamma_image);
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(gamma_image,exception);
  beta_view=AcquireVirtualCacheView(beta_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(beta_image,gamma_image,gamma_image->rows,1)
#endif
  for (y=0; y < (ssize_t) gamma_image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(beta_view,0,y,beta_image->columns,1,
      exception);
    q=GetCacheViewAuthenticPixels(image_view,0,y,gamma_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) gamma_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(gamma_image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(gamma_image,i);
        PixelTrait traits = GetPixelChannelTraits(gamma_image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        if ((x >= (ssize_t) beta_image->columns) ||
            (y >= (ssize_t) beta_image->rows))
          q[i]=(Quantum) 0;
        else
          q[i]=(Quantum) ((double) p[i]-channel_statistics[channel].mean);
      }
      p+=GetPixelChannels(beta_image);
      q+=GetPixelChannels(gamma_image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  beta_view=DestroyCacheView(beta_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    gamma_image=DestroyImage(gamma_image);
  return(gamma_image);
}

static Image *NCCUnityImage(const Image *alpha_image,const Image *beta_image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  Image
    *unity_image;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Create a padded unity image.
  */
  unity_image=CloneImage(alpha_image,alpha_image->columns,alpha_image->rows,
    MagickTrue,exception);
  if (unity_image == (Image *) NULL)
    return(unity_image);
  if (SetImageStorageClass(unity_image,DirectClass,exception) == MagickFalse)
    return(DestroyImage(unity_image));
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(unity_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(unity_image,unity_image,unity_image->rows,1)
#endif
  for (y=0; y < (ssize_t) unity_image->rows; y++)
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,unity_image->columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) unity_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(unity_image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(unity_image,i);
        PixelTrait traits = GetPixelChannelTraits(unity_image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=QuantumRange;
        if ((x >= (ssize_t) beta_image->columns) ||
            (y >= (ssize_t) beta_image->rows))
          q[i]=(Quantum) 0;
      }
      q+=GetPixelChannels(unity_image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    unity_image=DestroyImage(unity_image);
  return(unity_image);
}

static Image *NCCVarianceImage(Image *alpha_image,const Image *beta_image,
  ExceptionInfo *exception)
{
  CacheView
    *beta_view,
    *image_view;

  Image
    *variance_image;

  MagickBooleanType
    status;

  ssize_t
    y;

  /*
    Compute the variance of the two images.
  */
  variance_image=CloneImage(alpha_image,0,0,MagickTrue,exception);
  if (variance_image == (Image *) NULL)
    return(variance_image);
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(variance_image,exception);
  beta_view=AcquireVirtualCacheView(beta_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(beta_image,variance_image,variance_image->rows,1)
#endif
  for (y=0; y < (ssize_t) variance_image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(beta_view,0,y,beta_image->columns,1,
      exception);
    q=GetCacheViewAuthenticPixels(image_view,0,y,variance_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) variance_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(variance_image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(variance_image,i);
        PixelTrait traits = GetPixelChannelTraits(variance_image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=(Quantum) ((double) ClampToQuantum((double) QuantumRange*sqrt(fabs(
          QuantumScale*((double) q[i]-(double) p[i]))))/
          sqrt((double) QuantumRange));
      }
      p+=GetPixelChannels(beta_image);
      q+=GetPixelChannels(variance_image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  beta_view=DestroyCacheView(beta_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    variance_image=DestroyImage(variance_image);
  return(variance_image);
}

static Image *NCCSimilarityImage(const Image *image,const Image *reference,
  RectangleInfo *offset,double *similarity_metric,ExceptionInfo *exception)
{
#define DestroySimilarityResources() \
{ \
  if (channel_statistics != (ChannelStatistics *) NULL) \
    channel_statistics=(ChannelStatistics *) \
      RelinquishMagickMemory(channel_statistics); \
  if (beta_image != (Image *) NULL) \
    beta_image=DestroyImage(beta_image); \
  if (gamma_image != (Image *) NULL) \
    gamma_image=DestroyImage(gamma_image); \
  if (ncc_image != (Image *) NULL) \
    ncc_image=DestroyImage(ncc_image); \
  if (normalize_image != (Image *) NULL) \
    normalize_image=DestroyImage(normalize_image); \
  if (square_image != (Image *) NULL) \
    square_image=DestroyImage(square_image); \
  if (unity_image != (Image *) NULL) \
    unity_image=DestroyImage(unity_image); \
}
#define ThrowSimilarityException() \
{ \
  DestroySimilarityResources() \
  return((Image *) NULL); \
}

  ChannelStatistics
    *channel_statistics = (ChannelStatistics *) NULL;

  double
    maxima = 0.0;

  Image
    *beta_image = (Image *) NULL,
    *correlation_image = (Image *) NULL,
    *gamma_image = (Image *) NULL,
    *ncc_image = (Image *) NULL,
    *normalize_image = (Image *) NULL,
    *square_image = (Image *) NULL,
    *unity_image = (Image *) NULL;

  MagickBooleanType
    status;

  RectangleInfo
    geometry;

  /*
    Accelerated correlation-based image similary using FFT local statistics.
    Contributed by Fred Weinhaus.
  */
  square_image=NCCSquareImage(image,exception);
  if (square_image == (Image *) NULL)
    ThrowSimilarityException();
  unity_image=NCCUnityImage(image,reference,exception);
  if (unity_image == (Image *) NULL)
    ThrowSimilarityException();
  /*
    Compute the cross correlation of the square and unity images.
  */
  ncc_image=CrossCorrelationImage(square_image,unity_image,exception);
  square_image=DestroyImage(square_image);
  if (ncc_image == (Image *) NULL)
    ThrowSimilarityException();
  status=NCCMultiplyImage(ncc_image,(double) QuantumRange*reference->columns*
    reference->rows,(const ChannelStatistics *) NULL,exception);
  if (status == MagickFalse)
    ThrowSimilarityException();
  /*
    Compute the cross correlation of the source and unity images.
  */
  gamma_image=CrossCorrelationImage(image,unity_image,exception);
  unity_image=DestroyImage(unity_image);
  if (gamma_image == (Image *) NULL)
    ThrowSimilarityException();
  square_image=NCCSquareImage(gamma_image,exception);
  gamma_image=DestroyImage(gamma_image);
  status=NCCMultiplyImage(square_image,(double) QuantumRange,
    (const ChannelStatistics *) NULL,exception);
  if (status == MagickFalse)
    ThrowSimilarityException();
  /*
    Compute the variance of the two images.
  */
  gamma_image=NCCVarianceImage(ncc_image,square_image,exception);
  square_image=DestroyImage(square_image);
  ncc_image=DestroyImage(ncc_image);
  if (gamma_image == (Image *) NULL)
    ThrowSimilarityException();
  channel_statistics=GetImageStatistics(reference,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowSimilarityException();
  /*
    Subtract the image mean.
  */
  status=NCCMultiplyImage(gamma_image,1.0,channel_statistics,exception);
  if (status == MagickFalse)
    ThrowSimilarityException();
  normalize_image=NCCSubtractImageMean(image,reference,channel_statistics,
    exception);
  if (normalize_image == (Image *) NULL)
    ThrowSimilarityException();
  ncc_image=CrossCorrelationImage(image,normalize_image,exception);
  normalize_image=DestroyImage(normalize_image);
  if (ncc_image == (Image *) NULL)
    ThrowSimilarityException();
  /*
    Divide the two images.
  */
  beta_image=NCCDivideImage(ncc_image,gamma_image,exception);
  ncc_image=DestroyImage(ncc_image);
  gamma_image=DestroyImage(gamma_image);
  if (beta_image == (Image *) NULL)
    ThrowSimilarityException();
  (void) ResetImagePage(beta_image,"0x0+0+0");
  SetGeometry(image,&geometry);
  geometry.width=image->columns-reference->columns;
  geometry.height=image->rows-reference->rows;
  /*
    Crop padding.
  */
  correlation_image=CropImage(beta_image,&geometry,exception);
  beta_image=DestroyImage(beta_image);
  if (correlation_image == (Image *) NULL)
    ThrowSimilarityException();
  (void) ResetImagePage(correlation_image,"0x0+0+0");
  /*
    Identify the maxima value in the image and its location.
  */
  status=GrayscaleImage(correlation_image,AveragePixelIntensityMethod,
    exception);
  if (status == MagickFalse)
    ThrowSimilarityException();
  status=NCCMaximaImage(correlation_image,&maxima,offset,exception);
  if (status == MagickFalse)
    {
      correlation_image=DestroyImage(correlation_image);
      ThrowSimilarityException();
    }
  *similarity_metric=1.0-QuantumScale*maxima;
  DestroySimilarityResources();
  return(correlation_image);
}
#endif

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

MagickExport Image *SimilarityImage(const Image *image,const Image *reference,
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
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  assert(offset != (RectangleInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  SetGeometry(reference,offset);
  *similarity_metric=MagickMaximumValue;
#if defined(MAGICKCORE_HDRI_SUPPORT) && defined(MAGICKCORE_FFTW_DELEGATE)
  if ((image->channels & ReadMaskChannel) == 0)
    {
      const char *artifact = GetImageArtifact(image,"compare:accelerate-ncc");
      MagickBooleanType accelerate = (artifact != (const char *) NULL) &&
        (IsStringTrue(artifact) == MagickFalse) ? MagickFalse : MagickTrue;
      if ((accelerate != MagickFalse) &&
          (metric == NormalizedCrossCorrelationErrorMetric))
        {
          similarity_image=NCCSimilarityImage(image,reference,offset,
            similarity_metric,exception);
          return(similarity_image);
        }
    }
#endif
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
  #pragma omp parallel for schedule(static) \
    shared(progress,status,similarity_metric) \
    magick_number_threads(image,image,image->rows-reference->rows+1,1)
#endif
  for (y=0; y < (ssize_t) (image->rows-reference->rows+1); y++)
  {
    double
      similarity;

    Quantum
      *magick_restrict q;

    ssize_t
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
      ssize_t
        i;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp flush(similarity_metric)
#endif
      if (*similarity_metric <= similarity_threshold)
        break;
      similarity=GetSimilarityMetric(image,reference,metric,x,y,exception);
      if (metric == PeakSignalToNoiseRatioErrorMetric)
        similarity*=0.01;
      if ((metric == NormalizedCrossCorrelationErrorMetric) ||
          (metric == UndefinedErrorMetric))
        similarity=1.0-similarity;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp critical (MagickCore_SimilarityImage)
#endif
      if (similarity < *similarity_metric)
        {
          offset->x=x;
          offset->y=y;
          *similarity_metric=similarity;
        }
      if (metric == PerceptualHashErrorMetric)
        similarity=MagickMin(0.01*similarity,1.0);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait similarity_traits=GetPixelChannelTraits(similarity_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (similarity_traits == UndefinedPixelTrait) ||
            ((similarity_traits & UpdatePixelTrait) == 0))
          continue;
        SetPixelChannel(similarity_image,channel,ClampToQuantum((double)
          QuantumRange-(double) QuantumRange*similarity),q);
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
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SimilarityImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  similarity_view=DestroyCacheView(similarity_view);
  if (status == MagickFalse)
    similarity_image=DestroyImage(similarity_image);
  return(similarity_image);
}

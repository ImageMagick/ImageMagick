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
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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

static size_t GetImageChannels(const Image *image)
{
  register ssize_t
    i;

  size_t
    channels;

  channels=0;
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if ((traits & UpdatePixelTrait) != 0)
      channels++;
  }
  return(channels == 0 ? (size_t) 1 : channels);
}

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
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickCoreSignature);
  assert(distortion != (double *) NULL);
  *distortion=0.0;
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
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

    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register Quantum
      *magick_restrict r;

    register ssize_t
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

      register ssize_t
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
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
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
          distance=(double) p[i]-GetPixelChannel(reconstruct_image,channel,q);
        else
          distance=Sa*p[i]-Da*GetPixelChannel(reconstruct_image,channel,q);
        if ((distance*distance) > fuzz)
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
  fuzz=(double) MagickMin(GetPixelChannels(image),
    GetPixelChannels(reconstruct_image))*
    GetFuzzyColorDistance(image,reconstruct_image);
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

    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
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
        distance,
        Sa;

      MagickBooleanType
        difference;

      register ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      difference=MagickFalse;
      distance=0.0;
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
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
          pixel=(double) p[i]-GetPixelChannel(reconstruct_image,channel,q);
        else
          pixel=Sa*p[i]-Da*GetPixelChannel(reconstruct_image,channel,q);
        distance+=pixel*pixel;
        if (distance > fuzz)
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

  register ssize_t
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
    magick_number_threads(image,image,rows,1) reduction(+:area)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    double
      channel_distortion[MaxPixelChannels+1];

    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
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

      register ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
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

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=QuantumScale*(p[i]-GetPixelChannel(reconstruct_image,
            channel,q));
        else
          distance=QuantumScale*(Sa*p[i]-Da*GetPixelChannel(reconstruct_image,
            channel,q));
        channel_distortion[i]+=distance*distance;
        channel_distortion[CompositePixelChannel]+=distance*distance;
      }
      area++;
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetFuzzDistortion)
#endif
    for (j=0; j <= MaxPixelChannels; j++)
      distortion[j]+=channel_distortion[j];
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

  register ssize_t
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
    magick_number_threads(image,image,rows,1) reduction(+:area)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    double
      channel_distortion[MaxPixelChannels+1];

    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
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

      register ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
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

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=QuantumScale*fabs((double) p[i]-
            GetPixelChannel(reconstruct_image,channel,q));
        else
          distance=QuantumScale*fabs(Sa*p[i]-Da*
            GetPixelChannel(reconstruct_image,channel,q));
        channel_distortion[i]+=distance;
        channel_distortion[CompositePixelChannel]+=distance;
      }
      area++;
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMeanAbsoluteError)
#endif
    for (j=0; j <= MaxPixelChannels; j++)
      distortion[j]+=channel_distortion[j];
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
    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
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

      register ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
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

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=fabs((double) p[i]-
            GetPixelChannel(reconstruct_image,channel,q));
        else
          distance=fabs(Sa*p[i]-Da*
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

  double
    area;

  MagickBooleanType
    status;

  register ssize_t
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
    magick_number_threads(image,image,rows,1) reduction(+:area)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    double
      channel_distortion[MaxPixelChannels+1];

    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
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

      register ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
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

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=QuantumScale*(p[i]-GetPixelChannel(reconstruct_image,
            channel,q));
        else
          distance=QuantumScale*(Sa*p[i]-Da*GetPixelChannel(reconstruct_image,
            channel,q));
        channel_distortion[i]+=distance*distance;
        channel_distortion[CompositePixelChannel]+=distance*distance;
      }
      area++;
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMeanSquaredError)
#endif
    for (j=0; j <= MaxPixelChannels; j++)
      distortion[j]+=channel_distortion[j];
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
    area;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  register ssize_t
    i;

  size_t
    columns,
    rows;

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
  rows=MagickMax(image->rows,reconstruct_image->rows);
  columns=MagickMax(image->columns,reconstruct_image->columns);
  area=0.0;
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  for (y=0; y < (ssize_t) rows; y++)
  {
    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
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
      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=GetPixelChannels(image);
          q+=GetPixelChannels(reconstruct_image);
          continue;
        }
      area++;
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(reconstruct_image);
    }
  }
  area=PerceptibleReciprocal(area);
  for (y=0; y < (ssize_t) rows; y++)
  {
    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
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
      Sa=QuantumScale*GetPixelAlpha(image,p);
      Da=QuantumScale*GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
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
            distortion[i]+=area*QuantumScale*(p[i]-
              image_statistics[channel].mean)*(GetPixelChannel(
              reconstruct_image,channel,q)-
              reconstruct_statistics[channel].mean);
          }
        else
          {
            distortion[i]+=area*QuantumScale*(Sa*p[i]-
              image_statistics[channel].mean)*(Da*GetPixelChannel(
              reconstruct_image,channel,q)-
              reconstruct_statistics[channel].mean);
          }
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
    Divide by the standard deviation.
  */
  distortion[CompositePixelChannel]=0.0;
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    double
      gamma;

    PixelChannel channel = GetPixelChannelChannel(image,i);
    gamma=image_statistics[channel].standard_deviation*
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

    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
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

      register ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
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

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if ((traits == UndefinedPixelTrait) ||
            (reconstruct_traits == UndefinedPixelTrait) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=QuantumScale*fabs((double) p[i]-
            GetPixelChannel(reconstruct_image,channel,q));
        else
          distance=QuantumScale*fabs(Sa*p[i]-Da*
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

static inline double MagickLog10(const double x)
{
#define Log10Epsilon  (1.0e-11)

 if (fabs(x) < Log10Epsilon)
   return(log10(Log10Epsilon));
 return(log10(fabs(x)));
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
    if (fabs(distortion[i]) < MagickEpsilon)
      distortion[i]=INFINITY;
    else
      distortion[i]=10.0*MagickLog10(1.0)-10.0*MagickLog10(distortion[i]);
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

  MagickBooleanType
    normalize;

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
  artifact=GetImageArtifact(image,"phash:normalize");
  normalize=(artifact == (const char *) NULL) ||
    (IsStringTrue(artifact) == MagickFalse) ? MagickFalse : MagickTrue;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static)
#endif
  for (channel=0; channel < MaxPixelChannels; channel++)
  {
    double
      difference;

    register ssize_t
      i;

    difference=0.0;
    for (i=0; i < MaximumNumberOfImageMoments; i++)
    {
      double
        alpha,
        beta;

      register ssize_t
        j;

      for (j=0; j < (ssize_t) channel_phash[0].number_colorspaces; j++)
      {
        alpha=channel_phash[channel].phash[j][i];
        beta=reconstruct_phash[channel].phash[j][i];
        if (normalize == MagickFalse)
          difference+=(beta-alpha)*(beta-alpha);
        else
          difference=sqrt((beta-alpha)*(beta-alpha)/
            channel_phash[0].number_channels);
      }
    }
    distortion[channel]+=difference;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetPerceptualHashDistortion)
#endif
    distortion[CompositePixelChannel]+=difference;
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

  register ssize_t
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
    c1,
    c2,
    radius,
    sigma;

  KernelInfo
    *kernel_info;

  MagickBooleanType
    status;

  register ssize_t
    i;

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

    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
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

      register const Quantum
        *magick_restrict reference,
        *magick_restrict target;

      register MagickRealType
        *k;

      ssize_t
        v;

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
        register ssize_t
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
            x_pixel=QuantumScale*reference[i];
            x_pixel_mu[i]+=(*k)*x_pixel;
            x_pixel_sigma_squared[i]+=(*k)*x_pixel*x_pixel;
            y_pixel=QuantumScale*
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
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetStructuralSimilarityDistortion)
#endif
    for (i=0; i <= MaxPixelChannels; i++)
      distortion[i]+=channel_distortion[i];
  }
  image_view=DestroyCacheView(image_view);
  reconstruct_view=DestroyCacheView(reconstruct_view);
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if ((traits == UndefinedPixelTrait) || ((traits & UpdatePixelTrait) == 0))
      continue;
    distortion[i]/=((double) columns*rows);
  }
  distortion[CompositePixelChannel]/=((double) columns*rows);
  distortion[CompositePixelChannel]/=(double) GetImageChannels(image);
  kernel_info=DestroyKernelInfo(kernel_info);
  return(status);
}

static MagickBooleanType GetStructuralDisimilarityDistortion(const Image *image,
  const Image *reconstruct_image,double *distortion,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  register ssize_t
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
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickCoreSignature);
  assert(distortion != (double *) NULL);
  *distortion=0.0;
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  /*
    Get image distortion.
  */
  length=MaxPixelChannels+1;
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
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
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
    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      break;
    for (x=0; x < (ssize_t) columns; x++)
    {
      register ssize_t
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
        distance=fabs(p[i]-(double) GetPixelChannel(reconstruct_image,
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
    register const Quantum
      *magick_restrict p,
      *magick_restrict q;

    register ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      break;
    for (x=0; x < (ssize_t) columns; x++)
    {
      register ssize_t
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
        distance=fabs(p[i]-(double) GetPixelChannel(reconstruct_image,
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
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  assert(offset != (RectangleInfo *) NULL);
  SetGeometry(reference,offset);
  *similarity_metric=MagickMaximumValue;
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

    register Quantum
      *magick_restrict q;

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
      if ((metric == NormalizedCrossCorrelationErrorMetric) ||
          (metric == UndefinedErrorMetric))
        similarity=1.0-similarity;
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

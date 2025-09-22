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
#include "MagickCore/compare-private.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/distort.h"
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
#include "MagickCore/threshold.h"
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
%    o reconstruct_image: the reconstruction image.
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
    *clone_image,
    *difference_image,
    *highlight_image;

  MagickBooleanType
    status = MagickTrue;

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
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
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
  (void) ResetImagePage(difference_image,"0x0+0+0");
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
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
  highlight_view=AcquireAuthenticCacheView(highlight_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,highlight_image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    MagickBooleanType
      sync;

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
      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          SetPixelViaPixelInfo(highlight_image,&masklight,r);
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
          r+=(ptrdiff_t) GetPixelChannels(highlight_image);
          continue;
        }
      if (IsFuzzyEquivalencePixel(image,p,reconstruct_image,q) == MagickFalse)
        SetPixelViaPixelInfo(highlight_image,&highlight,r);
      else
        SetPixelViaPixelInfo(highlight_image,&lowlight,r);
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
      r+=(ptrdiff_t) GetPixelChannels(highlight_image);
    }
    sync=SyncCacheViewAuthenticPixels(highlight_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
  }
  highlight_view=DestroyCacheView(highlight_view);
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  if ((status != MagickFalse) && (difference_image != (Image *) NULL))
    status=CompositeImage(difference_image,highlight_image,image->compose,
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
%    o reconstruct_image: the reconstruction image.
%
%    o metric: the metric.
%
%    o distortion: the computed distortion between the images.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType GetAESimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  double
    area,
    fuzz;

  MagickBooleanType
    status = MagickTrue;

  size_t
    columns,
    rows;

  ssize_t
    k,
    y;

  /*
    Compute the absolute error similarity.
  */
  fuzz=GetFuzzyColorDistance(image,reconstruct_image);
  (void) memset(similarity,0,(MaxPixelChannels+1)*sizeof(*similarity));
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(similarity,status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_similarity[MaxPixelChannels+1] = { 0.0 };

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
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        Da,
        Sa;

      size_t
        count = 0;

      ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          error;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          error=(double) p[i]-(double) GetPixelChannel(reconstruct_image,
            channel,q);
        else
          error=Sa*p[i]-Da*GetPixelChannel(reconstruct_image,channel,q);
        if (MagickSafeSignificantError(error*error,fuzz) != MagickFalse)
          {
            channel_similarity[i]++;
            count++;
          }
      }
      if (count != 0)
        channel_similarity[CompositePixelChannel]++;
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetAESimilarity)
#endif
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        similarity[j]+=channel_similarity[j];
      }
      similarity[CompositePixelChannel]+=
        channel_similarity[CompositePixelChannel];
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  area=MagickSafeReciprocal((double) columns*rows);
  for (k=0; k < (ssize_t) GetPixelChannels(image); k++)
    similarity[k]*=area;
  similarity[CompositePixelChannel]*=area;
  return(status);
}

static MagickBooleanType GetDPCSimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
#define SimilarityImageTag  "Similarity/Image"

  CacheView
    *image_view,
    *reconstruct_view;

  ChannelStatistics
    *image_statistics,
    *reconstruct_statistics;

  double
    norm[MaxPixelChannels+1] = { 0.0 },
    reconstruct_norm[MaxPixelChannels+1] = { 0.0 };

  MagickBooleanType
    status = MagickTrue;

  MagickOffsetType
    progress = 0;

  size_t
    columns,
    rows;

  ssize_t
    k,
    y;

  /*
    Compute the dot product correlation similarity.
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
  (void) memset(similarity,0,(MaxPixelChannels+1)*sizeof(*similarity));
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(norm,reconstruct_norm,similarity,status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_norm[MaxPixelChannels+1] = { 0.0 },
      channel_reconstruct_norm[MaxPixelChannels+1] = { 0.0 },
      channel_similarity[MaxPixelChannels+1] = { 0.0 };

    ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
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
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
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
        if (((traits & UpdatePixelTrait) == 0) ||
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
            alpha=QuantumScale*(Sa*p[i]-image_statistics[channel].mean);
            beta=QuantumScale*(Da*GetPixelChannel(reconstruct_image,channel,
              q)-reconstruct_statistics[channel].mean);
          }
        channel_similarity[i]+=alpha*beta;
        channel_norm[i]+=alpha*alpha;
        channel_reconstruct_norm[i]+=beta*beta;
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetDPCSimilarity)
#endif
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        similarity[j]+=channel_similarity[j];
        similarity[CompositePixelChannel]+=channel_similarity[j];
        norm[j]+=channel_norm[j];
        norm[CompositePixelChannel]+=channel_norm[j];
        reconstruct_norm[j]+=channel_reconstruct_norm[j];
        reconstruct_norm[CompositePixelChannel]+=channel_reconstruct_norm[j];
      }
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
            continue;
          }
      }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  /*
    Compute dot product correlation: divide by mean.
  */
  for (k=0; k < (ssize_t) GetPixelChannels(image); k++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,k);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[k]*=MagickSafeReciprocal(sqrt(norm[k]*reconstruct_norm[k]));
  }
  similarity[CompositePixelChannel]*=MagickSafeReciprocal(sqrt(
    norm[CompositePixelChannel]*reconstruct_norm[CompositePixelChannel]));
  /*
    Free resources.
  */
  reconstruct_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    reconstruct_statistics);
  image_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    image_statistics);
  return(status);
}

static MagickBooleanType GetFUZZSimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  double
    area = 0.0,
    fuzz = 0.0;

  MagickBooleanType
    status = MagickTrue;

  size_t
    columns,
    rows;

  ssize_t
    k,
    y;

  /*
    Compute the MSE similarity within tolerance (fuzz).
  */
  fuzz=GetFuzzyColorDistance(image,reconstruct_image);
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(area,similarity,status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_area = 0.0,
      channel_similarity[MaxPixelChannels+1] = { 0.0 };

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
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          error;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          error=(double) p[i]-(double) GetPixelChannel(reconstruct_image,
            channel,q);
        else
          error=Sa*p[i]-Da*GetPixelChannel(reconstruct_image,channel,q);
        if (MagickSafeSignificantError(error*error,fuzz) != MagickFalse)
          {
            channel_similarity[i]+=QuantumScale*error*QuantumScale*error;
            channel_similarity[CompositePixelChannel]+=QuantumScale*error*
              QuantumScale*error;
            channel_area++;
          }
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetFUZZSimilarity)
#endif
    {
      ssize_t
        j;

      area+=channel_area;
      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        similarity[j]+=channel_similarity[j];
      }
      similarity[CompositePixelChannel]+=
        channel_similarity[CompositePixelChannel];
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  area=MagickSafeReciprocal(area);
  for (k=0; k < (ssize_t) GetPixelChannels(image); k++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,k);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[k]*=area;
  }
  similarity[CompositePixelChannel]*=area;
  return(status);
}

static MagickBooleanType GetMAESimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  double
    area = 0.0;

  MagickBooleanType
    status = MagickTrue;

  size_t
    columns,
    rows;

  ssize_t
    k,
    y;

  /*
    Compute the mean absolute error similarity.
  */
  (void) memset(similarity,0,(MaxPixelChannels+1)*sizeof(*similarity));
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(area,similarity,status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_area = 0.0,
      channel_similarity[MaxPixelChannels+1] = { 0.0 };

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
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          error;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          error=QuantumScale*fabs((double) p[i]-(double) GetPixelChannel(
            reconstruct_image,channel,q));
        else
          error=QuantumScale*fabs(Sa*p[i]-Da*GetPixelChannel(reconstruct_image,
            channel,q));
        channel_similarity[i]+=error;
        channel_similarity[CompositePixelChannel]+=error;
      }
      channel_area++;
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMAESimilarity)
#endif
    {
      ssize_t
        j;

      area+=channel_area;
      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        similarity[j]+=channel_similarity[j];
      }
      similarity[CompositePixelChannel]+=
        channel_similarity[CompositePixelChannel];
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  area=MagickSafeReciprocal(area);
  for (k=0; k < (ssize_t) GetPixelChannels(image); k++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,k);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[k]*=area;
  }
  similarity[CompositePixelChannel]*=area;
  similarity[CompositePixelChannel]/=(double) GetImageChannels(image);
  return(status);
}

static MagickBooleanType GetMEPPSimilarity(Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  double
    area = 0.0,
    maximum_error = -MagickMaximumValue,
    mean_error = 0.0;

  MagickBooleanType
    status = MagickTrue;

  size_t
    columns,
    rows;

  ssize_t
    k,
    y;

  /*
    Compute the mean error per pixel similarity.
  */
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(area,similarity,maximum_error,mean_error,status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_area = 0.0,
      channel_similarity[MaxPixelChannels+1] = { 0.0 },
      channel_maximum_error = maximum_error,
      channel_mean_error = 0.0;

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
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          error;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          error=QuantumScale*fabs((double) p[i]-(double) GetPixelChannel(
            reconstruct_image,channel,q));
        else
          error=QuantumScale*fabs(Sa*p[i]-Da*GetPixelChannel(reconstruct_image,
            channel,q));
        channel_similarity[i]+=error;
        channel_similarity[CompositePixelChannel]+=error;
        channel_mean_error+=error*error;
        if (error > channel_maximum_error)
          channel_maximum_error=error;
      }
      channel_area++;
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMEPPSimilarity)
#endif
    {
      ssize_t
        j;

      area+=channel_area;
      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        similarity[j]+=channel_similarity[j];
      }
      similarity[CompositePixelChannel]+=
        channel_similarity[CompositePixelChannel];
      mean_error+=channel_mean_error;
      if (channel_maximum_error > maximum_error)
        maximum_error=channel_maximum_error;
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  area=MagickSafeReciprocal(area);
  for (k=0; k < (ssize_t) GetPixelChannels(image); k++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,k);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[k]*=area;
  }
  similarity[CompositePixelChannel]*=area;
  similarity[CompositePixelChannel]/=(double) GetImageChannels(image);
  image->error.mean_error_per_pixel=QuantumRange*
    similarity[CompositePixelChannel];
  image->error.normalized_mean_error=mean_error*area;
  image->error.normalized_maximum_error=maximum_error;
  return(status);
}

static MagickBooleanType GetMSESimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  double
    area = 0.0;

  MagickBooleanType
    status = MagickTrue;

  size_t
    columns,
    rows;

  ssize_t
    k,
    y;

  /*
    Compute the mean sequared error similarity.
  */
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(area,similarity,status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_area = 0.0,
      channel_similarity[MaxPixelChannels+1] = { 0.0 };

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
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
          continue;
        }
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      Da=QuantumScale*(double) GetPixelAlpha(reconstruct_image,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          error;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          error=QuantumScale*((double) p[i]-(double) GetPixelChannel(
            reconstruct_image,channel,q));
        else
          error=QuantumScale*(Sa*p[i]-Da*GetPixelChannel(reconstruct_image,
            channel,q));
        channel_similarity[i]+=error*error;
        channel_similarity[CompositePixelChannel]+=error*error;
      }
      channel_area++;
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetMSESimilarity)
#endif
    {
      ssize_t
        j;

      area+=channel_area;
      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        similarity[j]+=channel_similarity[j];
      }
      similarity[CompositePixelChannel]+=
        channel_similarity[CompositePixelChannel];
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  area=MagickSafeReciprocal(area);
  for (k=0; k < (ssize_t) GetPixelChannels(image); k++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,k);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[k]*=area;
  }
  similarity[CompositePixelChannel]*=area;
  similarity[CompositePixelChannel]/=(double) GetImageChannels(image);
  return(status);
}

static MagickBooleanType GetNCCSimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  ChannelStatistics
    *image_statistics,
    *reconstruct_statistics;

  double
    reconstruct_variance[MaxPixelChannels+1] = { 0.0 },
    variance[MaxPixelChannels+1] = { 0.0 };

  MagickBooleanType
    status = MagickTrue;

  MagickOffsetType
    progress = 0;

  size_t
    columns,
    rows;

  ssize_t
    k,
    y;

  /*
    Compute the normalized criss-correlation similarity.
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
  (void) memset(similarity,0,(MaxPixelChannels+1)*sizeof(*similarity));
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(variance,reconstruct_variance,similarity,status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_reconstruct_variance[MaxPixelChannels+1] = { 0.0 },
      channel_similarity[MaxPixelChannels+1] = { 0.0 },
      channel_variance[MaxPixelChannels+1] = { 0.0 };

    ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,columns,1,exception);
    q=GetCacheViewVirtualPixels(reconstruct_view,0,y,columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (const Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
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
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
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
        if (((traits & UpdatePixelTrait) == 0) ||
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
            alpha=QuantumScale*(Sa*p[i]-image_statistics[channel].mean);
            beta=QuantumScale*(Da*GetPixelChannel(reconstruct_image,channel,
              q)-reconstruct_statistics[channel].mean);
          }
        channel_similarity[i]+=alpha*beta;
        channel_variance[i]+=alpha*alpha;
        channel_reconstruct_variance[i]+=beta*beta;
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetNCCSimilarity)
#endif
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        similarity[j]+=channel_similarity[j];
        similarity[CompositePixelChannel]+=channel_similarity[j];
        variance[j]+=channel_variance[j];
        variance[CompositePixelChannel]+=channel_variance[j];
        reconstruct_variance[j]+=channel_reconstruct_variance[j];
        reconstruct_variance[CompositePixelChannel]+=
          channel_reconstruct_variance[j];
      }
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
            continue;
          }
      }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  /*
    Compute normalized cross correlation: divide by standard deviation.
  */
  for (k=0; k < (ssize_t) GetPixelChannels(image); k++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,k);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[k]*=MagickSafeReciprocal(sqrt(variance[k])*
      sqrt(reconstruct_variance[k]));
  }
  similarity[CompositePixelChannel]*=MagickSafeReciprocal(sqrt(
    variance[CompositePixelChannel])*sqrt(
    reconstruct_variance[CompositePixelChannel]));
  /*
    Free resources.
  */
  reconstruct_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    reconstruct_statistics);
  image_statistics=(ChannelStatistics *) RelinquishMagickMemory(
    image_statistics);
  return(status);
}

static MagickBooleanType GetPASimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  MagickBooleanType
    status = MagickTrue;

  size_t
    columns,
    rows;

  ssize_t
    y;

  /*
    Compute the peak absolute similarity.
  */
  (void) memset(similarity,0,(MaxPixelChannels+1)*sizeof(*similarity));
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(similarity,status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_similarity[MaxPixelChannels+1] = { 0.0 };

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
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
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
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel == AlphaPixelChannel)
          distance=QuantumScale*fabs((double) p[i]-(double)
            GetPixelChannel(reconstruct_image,channel,q));
        else
          distance=QuantumScale*fabs(Sa*p[i]-Da*GetPixelChannel(
            reconstruct_image,channel,q));
        if (distance > channel_similarity[i])
          channel_similarity[i]=distance;
        if (distance > channel_similarity[CompositePixelChannel])
          channel_similarity[CompositePixelChannel]=distance;
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetPASimilarity)
#endif
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        if (channel_similarity[j] > similarity[j])
          similarity[j]=channel_similarity[j];
      }
      if (channel_similarity[CompositePixelChannel] > similarity[CompositePixelChannel])
        similarity[CompositePixelChannel]=
          channel_similarity[CompositePixelChannel];
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

static MagickBooleanType DFTPhaseSpectrum(const Image *image,const ssize_t u,
  const ssize_t v,double *phase,ExceptionInfo *exception)
{
#define PhaseImageTag  "Phase/Image"

  CacheView
    *image_view;

  double
    channel_imag[MaxPixelChannels+1] = { 0.0 },
    channel_real[MaxPixelChannels+1] = { 0.0 };

  MagickBooleanType
    status;

  ssize_t
    k,
    y;

  /*
    Compute DFT phase spectrum of an image.
  */
  status=MagickTrue;
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
        angle,
        Sa;

      ssize_t
        i;

      angle=MagickPI*((u*x/(double) image->rows)+(v*y/(double) image->columns));
      Sa=QuantumScale*(double) GetPixelAlpha(image,p);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if (channel == AlphaPixelChannel)
          {
            channel_real[i]+=(QuantumScale*p[i])*cos(angle);
            channel_imag[i]-=(QuantumScale*p[i])*sin(angle);
          }
        else
          {
            channel_real[i]+=(QuantumScale*Sa*p[i])*cos(angle);
            channel_imag[i]-=(QuantumScale*Sa*p[i])*sin(angle);
          }
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  for (k=0; k < (ssize_t) GetPixelChannels(image); k++)
    phase[k]=atan2(channel_imag[k],channel_real[k]);
  phase[CompositePixelChannel]=atan2(channel_imag[CompositePixelChannel],
    channel_real[CompositePixelChannel]);
  image_view=DestroyCacheView(image_view);
  return(status);
}

static MagickBooleanType GetPHASESimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *reconstruct_view;

  double
    area = 0.0;

  MagickBooleanType
    status = MagickTrue;

  size_t
    columns,
    rows;

  ssize_t
    k,
    y;

  /*
    Compute the phase congruency similarity.
  */
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(area,similarity,status) \
    magick_number_threads(image,image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_area = 0.0,
      channel_similarity[MaxPixelChannels+1] = { 0.0 };

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
    for (x=0; x < (ssize_t) columns; x++)
    {
      double
        phase[MaxPixelChannels+1] = { 0.0 },
        reconstruct_phase[MaxPixelChannels+1] = { 0.0 };

      ssize_t
        i;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
          continue;
        }
      status=DFTPhaseSpectrum(image,x,y,phase,exception);
      if (status == MagickFalse)
        break;
      status=DFTPhaseSpectrum(reconstruct_image,x,y,reconstruct_phase,
        exception);
      if (status == MagickFalse)
        break;
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          delta;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        delta=phase[i]-reconstruct_phase[i];
        channel_similarity[i]+=cos(delta);
        channel_similarity[CompositePixelChannel]+=cos(delta);
      }
      channel_area++;
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetPHASESimilarity)
#endif
    {
      ssize_t
        j;

      area+=channel_area;
      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        similarity[j]+=channel_similarity[j];
      }
      similarity[CompositePixelChannel]+=
        channel_similarity[CompositePixelChannel];
    }
  }
  reconstruct_view=DestroyCacheView(reconstruct_view);
  image_view=DestroyCacheView(image_view);
  area=MagickSafeReciprocal(area);
  for (k=0; k < (ssize_t) GetPixelChannels(image); k++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,k);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[k]*=area;
  }
  similarity[CompositePixelChannel]*=area;
  similarity[CompositePixelChannel]/=(double) GetImageChannels(image);
  return(status);
}

static MagickBooleanType GetPSNRSimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
  MagickBooleanType
    status = MagickTrue;

  ssize_t
    i;

  /*
    Compute the peak signal-to-noise ratio similarity.
  */
  status=GetMSESimilarity(image,reconstruct_image,similarity,exception);
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[i]=10.0*MagickSafeLog10(MagickSafeReciprocal(
      similarity[i]))/MagickSafePSNRRecipicol(10.0);
  }
  similarity[CompositePixelChannel]=10.0*MagickSafeLog10(
    MagickSafeReciprocal(similarity[CompositePixelChannel]))/
    MagickSafePSNRRecipicol(10.0);
  return(status);
}

static MagickBooleanType GetPHASHSimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
#define PHASHNormalizationFactor  389.373723242

  ChannelPerceptualHash
    *channel_phash,
    *reconstruct_phash;

  const char
    *artifact;

  ssize_t
    k;

  /*
    Compute the perceptual hash similarity.
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
  for (k=0; k < MaxPixelChannels; k++)
  {
    double
      difference;

    ssize_t
      i;

    PixelChannel channel = GetPixelChannelChannel(image,k);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
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
        double
          error;

        alpha=channel_phash[k].phash[j][i];
        beta=reconstruct_phash[k].phash[j][i];
        error=beta-alpha;
        if (IsNaN(error) != 0)
          error=0.0;
        difference+=error*error/PHASHNormalizationFactor;
      }
    }
    similarity[k]+=difference;
    similarity[CompositePixelChannel]+=difference;
  }
  similarity[CompositePixelChannel]/=(double) GetImageChannels(image);
  artifact=GetImageArtifact(image,"phash:normalize");
  if (IsStringTrue(artifact) != MagickFalse)
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        similarity[j]=sqrt(similarity[j]/channel_phash[0].number_colorspaces);
      }
      similarity[CompositePixelChannel]=sqrt(similarity[CompositePixelChannel]/
        channel_phash[0].number_colorspaces);
    }
  /*
    Free resources.
  */
  reconstruct_phash=(ChannelPerceptualHash *) RelinquishMagickMemory(
    reconstruct_phash);
  channel_phash=(ChannelPerceptualHash *) RelinquishMagickMemory(channel_phash);
  return(MagickTrue);
}

static MagickBooleanType GetRMSESimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
#define RMSESquareRoot(x)  sqrt((x) < 0.0 ? 0.0 : (x))

  MagickBooleanType
    status = MagickTrue;

  ssize_t
    i;

  /*
    Compute the root mean-squared error similarity.
  */
  status=GetMSESimilarity(image,reconstruct_image,similarity,exception);
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[i]=RMSESquareRoot(similarity[i]);
  }
  similarity[CompositePixelChannel]=RMSESquareRoot(
    similarity[CompositePixelChannel]);
  return(status);
}

static MagickBooleanType GetSSIMSimularity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
#define SSIMRadius  5.0
#define SSIMSigma  1.5
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
    area = 0.0,
    c1,
    c2,
    radius,
    sigma;

  KernelInfo
    *kernel_info;

  MagickBooleanType
    status = MagickTrue;

  size_t
    columns,
    rows;

  ssize_t
    l,
    y;

  /*
    Compute the structual similarity index similarity.
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
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
  image_view=AcquireVirtualCacheView(image,exception);
  reconstruct_view=AcquireVirtualCacheView(reconstruct_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(area,similarity,status) \
    magick_number_threads(image,reconstruct_image,rows,1)
#endif
  for (y=0; y < (ssize_t) rows; y++)
  {
    const Quantum
      *magick_restrict p,
      *magick_restrict q;

    double
      channel_area = 0.0,
      channel_similarity[MaxPixelChannels+1] = { 0.0 };

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
    for (x=0; x < (ssize_t) columns; x++)
    {
      const Quantum
        *magick_restrict reconstruct,
        *magick_restrict test;

      double
        x_pixel_mu[MaxPixelChannels+1] = { 0.0 },
        x_pixel_sigma_squared[MaxPixelChannels+1] = { 0.0 },
        xy_sigma[MaxPixelChannels+1] = { 0.0 },
        y_pixel_mu[MaxPixelChannels+1] = { 0.0 },
        y_pixel_sigma_squared[MaxPixelChannels+1] = { 0.0 };

      MagickRealType
        *k;

      ssize_t
        v;

      if ((GetPixelReadMask(image,p) <= (QuantumRange/2)) ||
          (GetPixelReadMask(reconstruct_image,q) <= (QuantumRange/2)))
        {
          p+=(ptrdiff_t) GetPixelChannels(image);
          q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
          continue;
        }
      k=kernel_info->values;
      test=p;
      reconstruct=q;
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
            if (((traits & UpdatePixelTrait) == 0) ||
                ((reconstruct_traits & UpdatePixelTrait) == 0))
              continue;
            x_pixel=QuantumScale*(double) test[i];
            x_pixel_mu[i]+=(*k)*x_pixel;
            x_pixel_sigma_squared[i]+=(*k)*x_pixel*x_pixel;
            y_pixel=QuantumScale*(double)
              GetPixelChannel(reconstruct_image,channel,reconstruct);
            y_pixel_mu[i]+=(*k)*y_pixel;
            y_pixel_sigma_squared[i]+=(*k)*y_pixel*y_pixel;
            xy_sigma[i]+=(*k)*x_pixel*y_pixel;
          }
          k++;
          test+=(ptrdiff_t) GetPixelChannels(image);
          reconstruct+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
        }
        test+=(ptrdiff_t) GetPixelChannels(image)*columns;
        reconstruct+=(ptrdiff_t) GetPixelChannels(reconstruct_image)*columns;
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
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        x_pixel_mu_squared=x_pixel_mu[i]*x_pixel_mu[i];
        y_pixel_mu_squared=y_pixel_mu[i]*y_pixel_mu[i];
        xy_mu=x_pixel_mu[i]*y_pixel_mu[i];
        xy_sigmas=xy_sigma[i]-xy_mu;
        x_pixel_sigmas_squared=x_pixel_sigma_squared[i]-x_pixel_mu_squared;
        y_pixel_sigmas_squared=y_pixel_sigma_squared[i]-y_pixel_mu_squared;
        ssim=((2.0*xy_mu+c1)*(2.0*xy_sigmas+c2))*
          MagickSafeReciprocal((x_pixel_mu_squared+y_pixel_mu_squared+c1)*
           (x_pixel_sigmas_squared+y_pixel_sigmas_squared+c2));
        channel_similarity[i]+=ssim;
        channel_similarity[CompositePixelChannel]+=ssim;
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
      channel_area++;
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetSSIMSimularity)
#endif
    {
      ssize_t
        j;

      area+=channel_area;
      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,j);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        similarity[j]+=channel_similarity[j];
      }
      similarity[CompositePixelChannel]+=
        channel_similarity[CompositePixelChannel];
    }
  }
  image_view=DestroyCacheView(image_view);
  reconstruct_view=DestroyCacheView(reconstruct_view);
  area=MagickSafeReciprocal(area);
  for (l=0; l < (ssize_t) GetPixelChannels(image); l++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,l);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[l]*=area;
  }
  similarity[CompositePixelChannel]*=area;
  similarity[CompositePixelChannel]/=(double) GetImageChannels(image);
  kernel_info=DestroyKernelInfo(kernel_info);
  return(status);
}

static MagickBooleanType GetDSSIMSimilarity(const Image *image,
  const Image *reconstruct_image,double *similarity,ExceptionInfo *exception)
{
  MagickBooleanType
    status = MagickTrue;

  ssize_t
    i;

  /*
    Compute the structual dissimilarity index similarity.
  */
  status=GetSSIMSimularity(image,reconstruct_image,similarity,exception);
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    PixelTrait reconstruct_traits = GetPixelChannelTraits(reconstruct_image,
      channel);
    if (((traits & UpdatePixelTrait) == 0) ||
        ((reconstruct_traits & UpdatePixelTrait) == 0))
      continue;
    similarity[i]=(1.0-similarity[i])/2.0;
  }
  similarity[CompositePixelChannel]=(1.0-similarity[CompositePixelChannel])/2.0;
  return(status);
}

MagickExport MagickBooleanType GetImageDistortion(Image *image,
  const Image *reconstruct_image,const MetricType metric,double *distortion,
  ExceptionInfo *exception)
{
#define CompareMetricNotSupportedException  "metric not supported"

  double
    *channel_similarity;

  MagickBooleanType
    status = MagickTrue;

  size_t
    length;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(reconstruct_image != (const Image *) NULL);
  assert(reconstruct_image->signature == MagickCoreSignature);
  assert(distortion != (double *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  /*
    Get image distortion.
  */
  *distortion=0.0;
  length=MaxPixelChannels+1UL;
  channel_similarity=(double *) AcquireQuantumMemory(length,
    sizeof(*channel_similarity));
  if (channel_similarity == (double *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(channel_similarity,0,length*sizeof(*channel_similarity));
  switch (metric)
  {
    case AbsoluteErrorMetric:
    {
      status=GetAESimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case DotProductCorrelationErrorMetric:
    {
      status=GetDPCSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case FuzzErrorMetric:
    {
      status=GetFUZZSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case MeanAbsoluteErrorMetric:
    {
      status=GetMAESimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case MeanErrorPerPixelErrorMetric:
    {
      status=GetMEPPSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case MeanSquaredErrorMetric:
    {
      status=GetMSESimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case NormalizedCrossCorrelationErrorMetric:
    {
      status=GetNCCSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case PeakAbsoluteErrorMetric:
    {
      status=GetPASimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case PeakSignalToNoiseRatioErrorMetric:
    {
      status=GetPSNRSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case PerceptualHashErrorMetric:
    {
      status=GetPHASHSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case PhaseCorrelationErrorMetric:
    {
      status=GetPHASESimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case RootMeanSquaredErrorMetric:
    case UndefinedErrorMetric:
    default:
    {
      status=GetRMSESimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case StructuralDissimilarityErrorMetric:
    {
      status=GetDSSIMSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case StructuralSimilarityErrorMetric:
    {
      status=GetSSIMSimularity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
  }
  *distortion=channel_similarity[CompositePixelChannel];
  switch (metric)
  {
    case DotProductCorrelationErrorMetric:
    case NormalizedCrossCorrelationErrorMetric:
    case PhaseCorrelationErrorMetric:
    case StructuralSimilarityErrorMetric:
    {
      *distortion=(1.0-(*distortion))/2.0;
      break;
    }
    default: break;
  }
  channel_similarity=(double *) RelinquishMagickMemory(channel_similarity);
  if (fabs(*distortion) < MagickEpsilon)
    *distortion=0.0;
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
%  reconstructed image and returns the specified metric for each channel.
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
%    o reconstruct_image: the reconstruction image.
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
    *distortion,
    *channel_similarity;

  MagickBooleanType
    status = MagickTrue;

  size_t
    length;

  ssize_t
    i;

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
  channel_similarity=(double *) AcquireQuantumMemory(length,
    sizeof(*channel_similarity));
  if (channel_similarity == (double *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(channel_similarity,0,length*sizeof(*channel_similarity));
  switch (metric)
  {
    case AbsoluteErrorMetric:
    {
      status=GetAESimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case DotProductCorrelationErrorMetric:
    {
      status=GetDPCSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case FuzzErrorMetric:
    {
      status=GetFUZZSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case MeanAbsoluteErrorMetric:
    {
      status=GetMAESimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case MeanErrorPerPixelErrorMetric:
    {
      status=GetMEPPSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case MeanSquaredErrorMetric:
    {
      status=GetMSESimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case NormalizedCrossCorrelationErrorMetric:
    {
      status=GetNCCSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case PeakAbsoluteErrorMetric:
    {
      status=GetPASimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case PeakSignalToNoiseRatioErrorMetric:
    {
      status=GetPSNRSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case PerceptualHashErrorMetric:
    {
      status=GetPHASHSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case PhaseCorrelationErrorMetric:
    {
      status=GetPHASESimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case RootMeanSquaredErrorMetric:
    case UndefinedErrorMetric:
    default:
    {
      status=GetRMSESimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case StructuralDissimilarityErrorMetric:
    {
      status=GetDSSIMSimilarity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
    case StructuralSimilarityErrorMetric:
    {
      status=GetSSIMSimularity(image,reconstruct_image,channel_similarity,
        exception);
      break;
    }
  }
  if (status == MagickFalse)
    {
      channel_similarity=(double *) RelinquishMagickMemory(channel_similarity);
      return((double *) NULL);
    }
  distortion=channel_similarity;
  switch (metric)
  {
    case DotProductCorrelationErrorMetric:
    case NormalizedCrossCorrelationErrorMetric:
    case PhaseCorrelationErrorMetric:
    case StructuralSimilarityErrorMetric:
    {
      for (i=0; i <= MaxPixelChannels; i++)
        distortion[i]=(1.0-distortion[i])/2.0;
      break;
    }
    default: break;
  }
  for (i=0; i <= MaxPixelChannels; i++)
    if (fabs(distortion[i]) < MagickEpsilon)
      distortion[i]=0.0;
  (void) FormatImageProperty(image,"distortion","%.*g",GetMagickPrecision(),
    distortion[CompositePixelChannel]);
  return(distortion);
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
%    o reconstruct_image: the reconstruction image.
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
  SetImageCompareBounds(image,reconstruct_image,&columns,&rows);
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
        if (((traits & UpdatePixelTrait) == 0) ||
            ((reconstruct_traits & UpdatePixelTrait) == 0))
          continue;
        distance=fabs((double) p[i]-(double) GetPixelChannel(reconstruct_image,
          channel,q));
        if (distance >= MagickEpsilon)
          break;
      }
      if (i < (ssize_t) GetPixelChannels(image))
        break;
      p+=(ptrdiff_t) GetPixelChannels(image);
      q+=(ptrdiff_t) GetPixelChannels(reconstruct_image);
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
%  pixel and its corresponding pixel in the reconstruction image.  The error
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
%    o reconstruct_image: the reconstruction image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageColorMetric(Image *image,
  const Image *reconstruct_image,ExceptionInfo *exception)
{
  double
    channel_similarity[MaxPixelChannels+1] = { 0.0 };

  MagickBooleanType
    status;

  status=GetMEPPSimilarity(image,reconstruct_image,channel_similarity,
    exception);
  if (status == MagickFalse)
    return(MagickFalse);
  status=fabs(image->error.mean_error_per_pixel) < MagickEpsilon ?
    MagickTrue : MagickFalse;
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
%  SimilarityImage() compares the reconstruction of the image and returns the
%  best match offset.  In addition, it returns a similarity image such that an
%  exact match location is completely white and if none of the pixels match,
%  black, otherwise some gray level in-between.
%
%  Contributed by Fred Weinhaus.
%
%  The format of the SimilarityImageImage method is:
%
%      Image *SimilarityImage(const Image *image,const Image *reconstruct,
%        const MetricType metric,const double similarity_threshold,
%        RectangleInfo *offset,double *similarity,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o reconstruct: find an area of the image that closely resembles this image.
%
%    o metric: the metric.
%
%    o similarity_threshold: minimum similarity for (sub)image match.
%
%    o offset: the best match offset of the reconstruction image within the
%      image.
%
%    o similarity: the computed similarity between the images.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(MAGICKCORE_HDRI_SUPPORT) && defined(MAGICKCORE_FFTW_DELEGATE)
static Image *SIMCrossCorrelationImage(const Image *alpha_image,
  const Image *beta_image,ExceptionInfo *exception)
{
  Image
    *alpha_fft = (Image *) NULL,
    *beta_fft = (Image *) NULL,
    *complex_conjugate = (Image *) NULL,
    *complex_multiplication = (Image *) NULL,
    *cross_correlation = (Image *) NULL,
    *temp_image = (Image *) NULL;

  /*
    Take the FFT of beta (reconstruction) image.
  */
  temp_image=CloneImage(beta_image,0,0,MagickTrue,exception);
  if (temp_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageArtifact(temp_image,"fourier:normalize","inverse");
  beta_fft=ForwardFourierTransformImage(temp_image,MagickFalse,exception);
  temp_image=DestroyImageList(temp_image);
  if (beta_fft == (Image *) NULL)
    return((Image *) NULL);
  /*
    Take the complex conjugate of beta_fft.
  */
  complex_conjugate=ComplexImages(beta_fft,ConjugateComplexOperator,exception);
  beta_fft=DestroyImageList(beta_fft);
  if (complex_conjugate == (Image *) NULL)
    return((Image *) NULL);
  /*
    Take the FFT of the alpha (test) image.
  */
  temp_image=CloneImage(alpha_image,0,0,MagickTrue,exception);
  if (temp_image == (Image *) NULL)
    {
      complex_conjugate=DestroyImageList(complex_conjugate);
      return((Image *) NULL);
    }
  (void) SetImageArtifact(temp_image,"fourier:normalize","inverse");
  alpha_fft=ForwardFourierTransformImage(temp_image,MagickFalse,exception);
  temp_image=DestroyImageList(temp_image);
  if (alpha_fft == (Image *) NULL)
    {
      complex_conjugate=DestroyImageList(complex_conjugate);
      return((Image *) NULL);
    }
  /*
    Do complex multiplication.
  */
  DisableCompositeClampUnlessSpecified(complex_conjugate);
  DisableCompositeClampUnlessSpecified(complex_conjugate->next);
  AppendImageToList(&complex_conjugate,alpha_fft);
  complex_multiplication=ComplexImages(complex_conjugate,
    MultiplyComplexOperator,exception);
  complex_conjugate=DestroyImageList(complex_conjugate);
  if (complex_multiplication == (Image *) NULL)
    return((Image *) NULL);
  /*
    Do the IFT and return the cross-correlation result.
  */
  cross_correlation=InverseFourierTransformImage(complex_multiplication,
    complex_multiplication->next,MagickFalse,exception);
  complex_multiplication=DestroyImageList(complex_multiplication);
  return(cross_correlation);
}

static Image *SIMDerivativeImage(const Image *image,const char *kernel,
  ExceptionInfo *exception)
{
  Image
    *derivative_image;

  KernelInfo
    *kernel_info;

  kernel_info=AcquireKernelInfo(kernel,exception);
  if (kernel_info == (KernelInfo *) NULL)
    return((Image *) NULL);
  derivative_image=MorphologyImage(image,ConvolveMorphology,1,kernel_info,
    exception);
  kernel_info=DestroyKernelInfo(kernel_info);
  return(derivative_image);
}

static Image *SIMDivideImage(const Image *numerator_image,
  const Image *denominator_image,ExceptionInfo *exception)
{
  CacheView
    *denominator_view,
    *numerator_view;

  Image
    *divide_image;

  MagickBooleanType
    status = MagickTrue;

  ssize_t
    y;

  /*
    Divide one image into another.
  */
  divide_image=CloneImage(numerator_image,0,0,MagickTrue,exception);
  if (divide_image == (Image *) NULL)
    return(divide_image);
  numerator_view=AcquireAuthenticCacheView(divide_image,exception);
  denominator_view=AcquireVirtualCacheView(denominator_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(denominator_image,divide_image,divide_image->rows,1)
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
    p=GetCacheViewVirtualPixels(denominator_view,0,y,
      denominator_image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(numerator_view,0,y,divide_image->columns,1,
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
        PixelTrait denominator_traits = GetPixelChannelTraits(denominator_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((denominator_traits & UpdatePixelTrait) == 0))
          continue;
        q[i]=(Quantum) ((double) q[i]*MagickSafeReciprocal(QuantumScale*
          (double) GetPixelChannel(denominator_image,channel,p)));
      }
      p+=(ptrdiff_t) GetPixelChannels(denominator_image);
      q+=(ptrdiff_t) GetPixelChannels(divide_image);
    }
    if (SyncCacheViewAuthenticPixels(numerator_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  denominator_view=DestroyCacheView(denominator_view);
  numerator_view=DestroyCacheView(numerator_view);
  if (status == MagickFalse)
    divide_image=DestroyImage(divide_image);
  return(divide_image);
}

static Image *SIMDivideByMagnitude(Image *image,Image *magnitude_image,
  const Image *source_image,ExceptionInfo *exception)
{
  Image
    *divide_image,
    *result_image;

  RectangleInfo
    geometry;

  divide_image=SIMDivideImage(image,magnitude_image,exception);
  if (divide_image == (Image *) NULL)
    return((Image *) NULL);
  GetPixelInfoRGBA((Quantum) 0,(Quantum) 0,(Quantum) 0,(Quantum) 0,
    &divide_image->background_color);
  SetGeometry(source_image,&geometry);
  geometry.width=MagickMax(source_image->columns,divide_image->columns);
  geometry.height=MagickMax(source_image->rows,divide_image->rows);
  result_image=ExtentImage(divide_image,&geometry,exception);
  divide_image=DestroyImage(divide_image);
  return(result_image);
}

static MagickBooleanType SIMFilterImageNaNs(Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status = MagickTrue;

  ssize_t
    y;

  /*
    Square each pixel in the image.
  */
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
        if (IsNaN((double) q[i]) != 0)
          q[i]=(Quantum) 0;
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

static Image *SIMSquareImage(const Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  Image
    *square_image;

  MagickBooleanType
    status = MagickTrue;

  ssize_t
    y;

  /*
    Square each pixel in the image.
  */
  square_image=CloneImage(image,0,0,MagickTrue,exception);
  if (square_image == (Image *) NULL)
    return(square_image);
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
        q[i]=(Quantum) (QuantumScale*q[i]*q[i]);
      }
      q+=(ptrdiff_t) GetPixelChannels(square_image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    square_image=DestroyImage(square_image);
  return(square_image);
}

static Image *SIMMagnitudeImage(Image *alpha_image,Image *beta_image,
  ExceptionInfo *exception)
{
  Image
    *magnitude_image,
    *xsq_image,
    *ysq_image;

  MagickBooleanType
    status = MagickTrue;

  (void) SetImageArtifact(alpha_image,"compose:clamp","False");
  xsq_image=SIMSquareImage(alpha_image,exception);
  if (xsq_image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageArtifact(beta_image,"compose:clamp","False");
  ysq_image=SIMSquareImage(beta_image,exception);
  if (ysq_image == (Image *) NULL)
    {
      xsq_image=DestroyImage(xsq_image);
      return((Image *) NULL);
    }
  status=CompositeImage(xsq_image,ysq_image,PlusCompositeOp,MagickTrue,0,0,
    exception);
  magnitude_image=xsq_image;
  ysq_image=DestroyImage(ysq_image);
  if (status == MagickFalse)
    {
      magnitude_image=DestroyImage(magnitude_image);
      return((Image *) NULL);
    }
  status=EvaluateImage(magnitude_image,PowEvaluateOperator,0.5,exception);
  if (status == MagickFalse)
    {
      magnitude_image=DestroyImage(magnitude_image);
      return (Image *) NULL;
    }
  return(magnitude_image);
}

static MagickBooleanType SIMMaximaImage(const Image *image,double *maxima,
  RectangleInfo *offset,ExceptionInfo *exception)
{
  typedef struct
  {
    double
      maxima;

    ssize_t
      x,
      y;
  } MaximaInfo;

  CacheView
    *image_view;

  const Quantum
    *magick_restrict q;

  MagickBooleanType
    status = MagickTrue;

  MaximaInfo
    maxima_info = { -MagickMaximumValue, 0, 0 };

  ssize_t
    y;

  /*
    Identify the maxima value in the image and its location.
  */
  image_view=AcquireVirtualCacheView(image,exception);
  q=GetCacheViewVirtualPixels(image_view,maxima_info.x,maxima_info.y,1,1,
    exception);
  if (q != (const Quantum *) NULL)
    maxima_info.maxima=IsNaN((double) q[0]) != 0 ? 0.0 : (double) q[0];
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(maxima_info,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    MaximaInfo
      channel_maxima = { -MagickMaximumValue, 0, 0 };

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
    channel_maxima=maxima_info;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          pixel;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        pixel=(double) p[i];
        if (IsNaN(pixel) != 0)
          pixel=0.0;
        if (pixel > channel_maxima.maxima)
          {
            channel_maxima.maxima=(double) p[i];
            channel_maxima.x=x;
            channel_maxima.y=y;
          }
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_SIMMaximaImage)
#endif
    if (channel_maxima.maxima > maxima_info.maxima)
      maxima_info=channel_maxima;
  }
  image_view=DestroyCacheView(image_view);
  *maxima=maxima_info.maxima;
  offset->x=maxima_info.x;
  offset->y=maxima_info.y;
  return(status);
}

static MagickBooleanType SIMMinimaImage(const Image *image,double *minima,
  RectangleInfo *offset,ExceptionInfo *exception)
{
  typedef struct
  {
    double
      minima;

    ssize_t
      x,
      y;
  } MinimaInfo;

  CacheView
    *image_view;

  const Quantum
    *magick_restrict q;

  MagickBooleanType
    status = MagickTrue;

  MinimaInfo
    minima_info = { MagickMaximumValue, 0, 0 };

  ssize_t
    y;

  /*
    Identify the minima value in the image and its location.
  */
  image_view=AcquireVirtualCacheView(image,exception);
  q=GetCacheViewVirtualPixels(image_view,minima_info.x,minima_info.y,1,1,
    exception);
  if (q != (const Quantum *) NULL)
    minima_info.minima=IsNaN((double) q[0]) != 0 ? 0.0 : (double) q[0];
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(minima_info,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    MinimaInfo
      channel_minima = { MagickMaximumValue, 0, 0 };

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
    channel_minima=minima_info;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        double
          pixel;

        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        pixel=(double) p[i];
        if (IsNaN(pixel) != 0)
          pixel=0.0;
        if (pixel < channel_minima.minima)
          {
            channel_minima.minima=pixel;
            channel_minima.x=x;
            channel_minima.y=y;
          }
      }
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_SIMMinimaImage)
#endif
    if (channel_minima.minima < minima_info.minima)
      minima_info=channel_minima;
  }
  image_view=DestroyCacheView(image_view);
  *minima=minima_info.minima;
  offset->x=minima_info.x;
  offset->y=minima_info.y;
  return(status);
}

static MagickBooleanType SIMMultiplyImage(Image *image,const double factor,
  const ChannelStatistics *channel_statistics,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status = MagickTrue;

  ssize_t
    y;

  /*
    Multiply each pixel by a factor.
  */
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
          q[i]=(Quantum) (factor*q[i]*QuantumScale*
            channel_statistics[channel].standard_deviation);
        else
          q[i]=(Quantum) (factor*q[i]);
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

static Image *SIMPhaseCorrelationImage(const Image *alpha_image,
  const Image *beta_image,const Image *magnitude_image,ExceptionInfo *exception)
{
  Image
    *alpha_fft = (Image *) NULL,
    *beta_fft = (Image *) NULL,
    *complex_multiplication = (Image *) NULL,
    *cross_correlation = (Image *) NULL;

  /*
    Take the FFT of the beta (reconstruction) image.
  */
  beta_fft=CloneImage(beta_image,0,0,MagickTrue,exception);
  if (beta_fft == NULL)
    return((Image *) NULL);
  (void) SetImageArtifact(beta_fft,"fourier:normalize","inverse");
  beta_fft=ForwardFourierTransformImage(beta_fft,MagickFalse,exception);
  if (beta_fft == NULL)
    return((Image *) NULL);
  /*
    Take the FFT of the alpha (test) image.
  */
  alpha_fft=CloneImage(alpha_image,0,0,MagickTrue,exception);
  if (alpha_fft == (Image *) NULL)
    {
      beta_fft=DestroyImageList(beta_fft);
      return((Image *) NULL);
    }
  (void) SetImageArtifact(alpha_fft,"fourier:normalize","inverse");
  alpha_fft=ForwardFourierTransformImage(alpha_fft,MagickFalse,exception);
  if (alpha_fft == (Image *) NULL)
    {
      beta_fft=DestroyImageList(beta_fft);
      return((Image *) NULL);
    }
  /*
    Take the complex conjugate of the beta FFT.
  */
  beta_fft=ComplexImages(beta_fft,ConjugateComplexOperator,exception);
  if (beta_fft == (Image *) NULL)
    {
      alpha_fft=DestroyImageList(alpha_fft);
      return((Image *) NULL);
    }
  /*
    Do complex multiplication.
  */
  AppendImageToList(&beta_fft,alpha_fft);
  DisableCompositeClampUnlessSpecified(beta_fft);
  DisableCompositeClampUnlessSpecified(beta_fft->next);
  complex_multiplication=ComplexImages(beta_fft,MultiplyComplexOperator,
    exception);
  beta_fft=DestroyImageList(beta_fft);
  if (complex_multiplication == (Image *) NULL)
    return((Image *) NULL);
  /*
    Divide the results.
  */
  CompositeLayers(complex_multiplication,DivideSrcCompositeOp,(Image *)
    magnitude_image,0,0,exception);
  /*
    Do the IFT and return the cross-correlation result.
  */
  (void) SetImageArtifact(complex_multiplication,"fourier:normalize","inverse");
  cross_correlation=InverseFourierTransformImage(complex_multiplication,
    complex_multiplication->next,MagickFalse,exception);
  complex_multiplication=DestroyImageList(complex_multiplication);
  return(cross_correlation);
}

static MagickBooleanType SIMSetImageMean(Image *image,
  const ChannelStatistics *channel_statistics,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status = MagickTrue;

  ssize_t
    y;

  /*
    Set image mean.
  */
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
        q[i]=(Quantum) channel_statistics[channel].mean;
      }
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

static Image *SIMSubtractImageMean(const Image *alpha_image,
  const Image *beta_image,const ChannelStatistics *channel_statistics,
  ExceptionInfo *exception)
{
  CacheView
    *beta_view,
    *image_view;

  Image
    *subtract_image;

  MagickBooleanType
    status = MagickTrue;

  ssize_t
    y;

  /*
    Subtract the image mean and pad.
  */
  subtract_image=CloneImage(beta_image,alpha_image->columns,alpha_image->rows,
    MagickTrue,exception);
  if (subtract_image == (Image *) NULL)
    return(subtract_image);
  image_view=AcquireAuthenticCacheView(subtract_image,exception);
  beta_view=AcquireVirtualCacheView(beta_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(beta_image,subtract_image,subtract_image->rows,1)
#endif
  for (y=0; y < (ssize_t) subtract_image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(beta_view,0,y,beta_image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(image_view,0,y,subtract_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) subtract_image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(subtract_image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(subtract_image,i);
        PixelTrait traits = GetPixelChannelTraits(subtract_image,channel);
        PixelTrait beta_traits = GetPixelChannelTraits(beta_image,channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((beta_traits & UpdatePixelTrait) == 0))
          continue;
        if ((x >= (ssize_t) beta_image->columns) ||
            (y >= (ssize_t) beta_image->rows))
          q[i]=(Quantum) 0;
        else
          q[i]=(Quantum) ((double) GetPixelChannel(beta_image,channel,p)-
            channel_statistics[channel].mean);
      }
      p+=(ptrdiff_t) GetPixelChannels(beta_image);
      q+=(ptrdiff_t) GetPixelChannels(subtract_image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  beta_view=DestroyCacheView(beta_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    subtract_image=DestroyImage(subtract_image);
  return(subtract_image);
}

static Image *SIMUnityImage(const Image *alpha_image,const Image *beta_image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  Image
    *unity_image;

  MagickBooleanType
    status = MagickTrue;

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
        if ((x >= (ssize_t) beta_image->columns) ||
            (y >= (ssize_t) beta_image->rows))
          q[i]=(Quantum) 0;
        else
          q[i]=QuantumRange;
      }
      q+=(ptrdiff_t) GetPixelChannels(unity_image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    unity_image=DestroyImage(unity_image);
  return(unity_image);
}

static Image *SIMVarianceImage(Image *alpha_image,const Image *beta_image,
  ExceptionInfo *exception)
{
  CacheView
    *beta_view,
    *image_view;

  Image
    *variance_image;

  MagickBooleanType
    status = MagickTrue;

  ssize_t
    y;

  /*
    Compute the variance of the two images.
  */
  variance_image=CloneImage(alpha_image,0,0,MagickTrue,exception);
  if (variance_image == (Image *) NULL)
    return(variance_image);
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
        double
          error;

        PixelChannel channel = GetPixelChannelChannel(variance_image,i);
        PixelTrait traits = GetPixelChannelTraits(variance_image,channel);
        PixelTrait beta_traits = GetPixelChannelTraits(beta_image,channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((beta_traits & UpdatePixelTrait) == 0))
          continue;
        error=(double) q[i]-(double) GetPixelChannel(beta_image,channel,p);
        q[i]=(Quantum) ((double) ClampToQuantum((double) QuantumRange*
          (sqrt(fabs(QuantumScale*error))/sqrt((double) QuantumRange))));
      }
      p+=(ptrdiff_t) GetPixelChannels(beta_image);
      q+=(ptrdiff_t) GetPixelChannels(variance_image);
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

static Image *DPCSimilarityImage(const Image *image,const Image *reconstruct,
  RectangleInfo *offset,double *similarity_metric,ExceptionInfo *exception)
{
#define ThrowDPCSimilarityException() \
{ \
  if (dot_product_image != (Image *) NULL) \
    dot_product_image=DestroyImage(dot_product_image); \
  if (magnitude_image != (Image *) NULL) \
    magnitude_image=DestroyImage(magnitude_image); \
  if (reconstruct_image != (Image *) NULL) \
    reconstruct_image=DestroyImage(reconstruct_image); \
  if (rx_image != (Image *) NULL) \
    rx_image=DestroyImage(rx_image); \
  if (ry_image != (Image *) NULL) \
    ry_image=DestroyImage(ry_image); \
  if (target_image != (Image *) NULL) \
    target_image=DestroyImage(target_image); \
  if (threshold_image != (Image *) NULL) \
    threshold_image=DestroyImage(threshold_image); \
  if (trx_image != (Image *) NULL) \
    trx_image=DestroyImage(trx_image); \
  if (try_image != (Image *) NULL) \
    try_image=DestroyImage(try_image); \
  if (tx_image != (Image *) NULL) \
    tx_image=DestroyImage(tx_image); \
  if (ty_image != (Image *) NULL) \
    ty_image=DestroyImage(ty_image); \
  return((Image *) NULL); \
}

  double
    edge_factor = 0.0,
    maxima = 0.0,
    mean = 0.0,
    standard_deviation = 0.0;

  Image
    *dot_product_image = (Image *) NULL,
    *magnitude_image = (Image *) NULL,
    *reconstruct_image = (Image *) NULL,
    *rx_image = (Image *) NULL,
    *ry_image = (Image *) NULL,
    *trx_image = (Image *) NULL,
    *target_image = (Image *) NULL,
    *threshold_image = (Image *) NULL,
    *try_image = (Image *) NULL,
    *tx_image = (Image *) NULL,
    *ty_image = (Image *) NULL;

  MagickBooleanType
    status = MagickTrue;

  RectangleInfo
    geometry;

  /*
    Dot product correlation-based image similarity using FFT local statistics.
  */
  target_image=CloneImage(image,0,0,MagickTrue,exception);
  if (target_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Compute the cross correlation of the test and reconstruct magnitudes.
  */
  reconstruct_image=CloneImage(reconstruct,0,0,MagickTrue,exception);
  if (reconstruct_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  /*
    Compute X and Y derivatives of reference image.
  */
  (void) SetImageVirtualPixelMethod(reconstruct_image,EdgeVirtualPixelMethod,
    exception);
  rx_image=SIMDerivativeImage(reconstruct_image,"Sobel",exception);
  if (rx_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  ry_image=SIMDerivativeImage(reconstruct_image,"Sobel:90",exception);
  reconstruct_image=DestroyImage(reconstruct_image);
  if (ry_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  /*
    Compute magnitude of derivatives.
  */
  magnitude_image=SIMMagnitudeImage(rx_image,ry_image,exception);
  if (magnitude_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  /*
    Compute an edge normalization correction.
  */
  threshold_image=CloneImage(magnitude_image,0,0,MagickTrue,exception);
  if (threshold_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  status=BilevelImage(threshold_image,0.0,exception);
  if (status == MagickFalse)
    ThrowDPCSimilarityException();
  status=GetImageMean(threshold_image,&mean,&standard_deviation,exception);
  threshold_image=DestroyImage(threshold_image);
  if (status == MagickFalse)
    ThrowDPCSimilarityException();
  edge_factor=MagickSafeReciprocal(QuantumScale*mean*reconstruct->columns*
    reconstruct->rows)+QuantumScale;
  /*
    Divide X and Y derivitives of reference image by magnitude.
  */
  trx_image=SIMDivideByMagnitude(rx_image,magnitude_image,image,exception);
  rx_image=DestroyImage(rx_image);
  if (trx_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  rx_image=trx_image;
  try_image=SIMDivideByMagnitude(ry_image,magnitude_image,image,exception);
  magnitude_image=DestroyImage(magnitude_image);
  ry_image=DestroyImage(ry_image);
  if (try_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  ry_image=try_image;
  /*
    Compute X and Y derivatives of image.
  */
  (void) SetImageVirtualPixelMethod(target_image,EdgeVirtualPixelMethod,
    exception);
  tx_image=SIMDerivativeImage(target_image,"Sobel",exception);
  if (tx_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  ty_image=SIMDerivativeImage(target_image,"Sobel:90",exception);
  target_image=DestroyImage(target_image);
  if (ty_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  /*
    Compute magnitude of derivatives.
  */
  magnitude_image=SIMMagnitudeImage(tx_image,ty_image,exception);
  if (magnitude_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  /*
    Divide Lx and Ly by magnitude.
  */
  trx_image=SIMDivideByMagnitude(tx_image,magnitude_image,image,exception);
  tx_image=DestroyImage(tx_image);
  if (trx_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  tx_image=trx_image;
  try_image=SIMDivideByMagnitude(ty_image,magnitude_image,image,exception);
  ty_image=DestroyImage(ty_image);
  magnitude_image=DestroyImage(magnitude_image);
  if (try_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  ty_image=try_image;
  /*
    Compute the cross correlation of the test and reference images.
  */
  trx_image=SIMCrossCorrelationImage(tx_image,rx_image,exception);
  rx_image=DestroyImage(rx_image);
  tx_image=DestroyImage(tx_image);
  if (trx_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  try_image=SIMCrossCorrelationImage(ty_image,ry_image,exception);
  ry_image=DestroyImage(ry_image);
  ty_image=DestroyImage(ty_image);
  if (try_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  /*
    Evaluate dot product correlation image.
  */
  (void) SetImageArtifact(try_image,"compose:clamp","false");
  status=CompositeImage(trx_image,try_image,PlusCompositeOp,MagickTrue,0,0,
    exception);
  try_image=DestroyImage(try_image);
  if (status == MagickFalse)
    ThrowDPCSimilarityException();
  status=SIMMultiplyImage(trx_image,edge_factor,
    (const ChannelStatistics *) NULL,exception);
  if (status == MagickFalse)
    ThrowDPCSimilarityException();
  /*
    Crop results.
  */
  SetGeometry(image,&geometry);
  geometry.width=image->columns;
  geometry.height=image->rows;
  (void) ResetImagePage(trx_image,"0x0+0+0");
  dot_product_image=CropImage(trx_image,&geometry,exception);
  trx_image=DestroyImage(trx_image);
  if (dot_product_image == (Image *) NULL)
    ThrowDPCSimilarityException();
  (void) ResetImagePage(dot_product_image,"0x0+0+0");
  /*
    Identify the maxima value in the image and its location.
  */
  status=GrayscaleImage(dot_product_image,AveragePixelIntensityMethod,
    exception);
  if (status == MagickFalse)
    ThrowDPCSimilarityException();
  dot_product_image->depth=32;
  dot_product_image->colorspace=GRAYColorspace;
  dot_product_image->alpha_trait=UndefinedPixelTrait;
  status=SIMFilterImageNaNs(dot_product_image,exception);
  if (status == MagickFalse)
    ThrowDPCSimilarityException();
  status=SIMMaximaImage(dot_product_image,&maxima,offset,exception);
  if (status == MagickFalse)
    ThrowDPCSimilarityException();
  if ((QuantumScale*maxima) > 1.0)
    {
      status=SIMMultiplyImage(dot_product_image,1.0/(QuantumScale*maxima),
        (const ChannelStatistics *) NULL,exception);
      maxima=(double) QuantumRange;
    }
  *similarity_metric=QuantumScale*maxima;
  return(dot_product_image);
}

static Image *MSESimilarityImage(const Image *image,const Image *reconstruct,
  RectangleInfo *offset,double *similarity_metric,ExceptionInfo *exception)
{
#define ThrowMSESimilarityException() \
{ \
  if (alpha_image != (Image *) NULL) \
    alpha_image=DestroyImage(alpha_image); \
  if (beta_image != (Image *) NULL) \
    beta_image=DestroyImage(beta_image); \
  if (channel_statistics != (ChannelStatistics *) NULL) \
    channel_statistics=(ChannelStatistics *) \
      RelinquishMagickMemory(channel_statistics); \
  if (mean_image != (Image *) NULL) \
    mean_image=DestroyImage(mean_image); \
  if (mse_image != (Image *) NULL) \
    mse_image=DestroyImage(mse_image); \
  if (reconstruct_image != (Image *) NULL) \
    reconstruct_image=DestroyImage(reconstruct_image); \
  if (sum_image != (Image *) NULL) \
    sum_image=DestroyImage(sum_image); \
  if (alpha_image != (Image *) NULL) \
    alpha_image=DestroyImage(alpha_image); \
  return((Image *) NULL); \
}

  ChannelStatistics
    *channel_statistics = (ChannelStatistics *) NULL;

  double
    minima = 0.0;

  Image
    *alpha_image = (Image *) NULL,
    *beta_image = (Image *) NULL,
    *mean_image = (Image *) NULL,
    *mse_image = (Image *) NULL,
    *reconstruct_image = (Image *) NULL,
    *sum_image = (Image *) NULL,
    *target_image = (Image *) NULL;

  MagickBooleanType
    status = MagickTrue;

  RectangleInfo
    geometry;

  /*
    MSE correlation-based image similarity using FFT local statistics.
  */
  target_image=SIMSquareImage(image,exception);
  if (target_image == (Image *) NULL)
    ThrowMSESimilarityException();
  reconstruct_image=SIMUnityImage(image,reconstruct,exception);
  if (reconstruct_image == (Image *) NULL)
    ThrowMSESimilarityException();
  /*
    Create (U * test)/# pixels.
  */
  alpha_image=SIMCrossCorrelationImage(target_image,reconstruct_image,
    exception);
  target_image=DestroyImage(target_image);
  if (alpha_image == (Image *) NULL)
    ThrowMSESimilarityException();
  status=SIMMultiplyImage(alpha_image,1.0/reconstruct->columns/(double)
    reconstruct->rows,(const ChannelStatistics *) NULL,exception);
  if (status == MagickFalse)
    ThrowMSESimilarityException();
  /*
    Create 2*(test * reconstruction)# pixels.
  */
  (void) CompositeImage(reconstruct_image,reconstruct,CopyCompositeOp,
    MagickTrue,0,0,exception);
  beta_image=SIMCrossCorrelationImage(image,reconstruct_image,exception);
  reconstruct_image=DestroyImage(reconstruct_image);
  if (beta_image == (Image *) NULL)
    ThrowMSESimilarityException();
  status=SIMMultiplyImage(beta_image,-2.0/reconstruct->columns/(double)
    reconstruct->rows,(const ChannelStatistics *) NULL,exception);
  if (status == MagickFalse)
    ThrowMSESimilarityException();
  /*
    Mean of reconstruction squared.
  */
  sum_image=SIMSquareImage(reconstruct,exception);
  if (sum_image == (Image *) NULL)
    ThrowMSESimilarityException();
  channel_statistics=GetImageStatistics(sum_image,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowMSESimilarityException();
  status=SetImageStorageClass(sum_image,DirectClass,exception);
  if (status == MagickFalse)
    ThrowMSESimilarityException();
  status=SIMSetImageMean(sum_image,channel_statistics,exception);
  channel_statistics=(ChannelStatistics *)
    RelinquishMagickMemory(channel_statistics);
  if (status == MagickFalse)
    ThrowMSESimilarityException();
  /*
    Create mean image.
  */
  AppendImageToList(&sum_image,alpha_image);
  AppendImageToList(&sum_image,beta_image);
  mean_image=EvaluateImages(sum_image,SumEvaluateOperator,exception);
  if (mean_image == (Image *) NULL)
    ThrowMSESimilarityException();
  sum_image=DestroyImage(sum_image);
  status=GrayscaleImage(mean_image,AveragePixelIntensityMethod,exception);
  if (status == MagickFalse)
    ThrowMSESimilarityException();
  /*
    Crop to difference of reconstruction and test images.
  */
  SetGeometry(image,&geometry);
  geometry.width=image->columns;
  geometry.height=image->rows;
  (void) ResetImagePage(mean_image,"0x0+0+0");
  mse_image=CropImage(mean_image,&geometry,exception);
  mean_image=DestroyImage(mean_image);
  if (mse_image == (Image *) NULL)
    ThrowMSESimilarityException();
  /*
    Identify the minima value in the correlation image and its location.
  */
  (void) ResetImagePage(mse_image,"0x0+0+0");
  (void) ClampImage(mse_image,exception);
  mse_image->depth=32;
  mse_image->colorspace=GRAYColorspace;
  mse_image->alpha_trait=UndefinedPixelTrait;
  status=SIMMinimaImage(mse_image,&minima,offset,exception);
  if (status == MagickFalse)
    ThrowMSESimilarityException();
  status=NegateImage(mse_image,MagickFalse,exception);
  if (status == MagickFalse)
    ThrowMSESimilarityException();
  alpha_image=DestroyImage(alpha_image);
  beta_image=DestroyImage(beta_image);
  if ((QuantumScale*minima) < FLT_EPSILON)
    minima=0.0;
  *similarity_metric=QuantumScale*minima;
  return(mse_image);
}

static Image *NCCSimilarityImage(const Image *image,const Image *reconstruct,
  RectangleInfo *offset,double *similarity_metric,ExceptionInfo *exception)
{
#define ThrowNCCSimilarityException() \
{ \
  if (alpha_image != (Image *) NULL) \
    alpha_image=DestroyImage(alpha_image); \
  if (beta_image != (Image *) NULL) \
    beta_image=DestroyImage(beta_image); \
  if (channel_statistics != (ChannelStatistics *) NULL) \
    channel_statistics=(ChannelStatistics *) \
      RelinquishMagickMemory(channel_statistics); \
  if (correlation_image != (Image *) NULL) \
    correlation_image=DestroyImage(correlation_image); \
  if (divide_image != (Image *) NULL) \
    divide_image=DestroyImage(divide_image); \
  if (ncc_image != (Image *) NULL) \
    ncc_image=DestroyImage(ncc_image); \
  if (normalize_image != (Image *) NULL) \
    normalize_image=DestroyImage(normalize_image); \
  if (reconstruct_image != (Image *) NULL) \
    reconstruct_image=DestroyImage(reconstruct_image); \
  if (target_image != (Image *) NULL) \
    target_image=DestroyImage(target_image); \
  if (variance_image != (Image *) NULL) \
    variance_image=DestroyImage(variance_image); \
  return((Image *) NULL); \
}

  ChannelStatistics
    *channel_statistics = (ChannelStatistics *) NULL;

  double
    maxima = 0.0;

  Image
    *alpha_image = (Image *) NULL,
    *beta_image = (Image *) NULL,
    *correlation_image = (Image *) NULL,
    *divide_image = (Image *) NULL,
    *ncc_image = (Image *) NULL,
    *normalize_image = (Image *) NULL,
    *reconstruct_image = (Image *) NULL,
    *target_image = (Image *) NULL,
    *variance_image = (Image *) NULL;

  MagickBooleanType
    status = MagickTrue;

  RectangleInfo
    geometry;

  /*
    NCC correlation-based image similarity with FFT local statistics.
  */
  target_image=SIMSquareImage(image,exception);
  if (target_image == (Image *) NULL)
    ThrowNCCSimilarityException();
  reconstruct_image=SIMUnityImage(image,reconstruct,exception);
  if (reconstruct_image == (Image *) NULL)
    ThrowNCCSimilarityException();
  /*
    Compute the cross correlation of the test and reconstruction images.
  */
  alpha_image=SIMCrossCorrelationImage(target_image,reconstruct_image,
    exception);
  target_image=DestroyImage(target_image);
  if (alpha_image == (Image *) NULL)
    ThrowNCCSimilarityException();
  status=SIMMultiplyImage(alpha_image,(double) QuantumRange*
    reconstruct->columns*reconstruct->rows,(const ChannelStatistics *) NULL,
    exception);
  if (status == MagickFalse)
    ThrowNCCSimilarityException();
  /*
    Compute the cross correlation of the source and reconstruction images.
  */
  beta_image=SIMCrossCorrelationImage(image,reconstruct_image,exception);
  reconstruct_image=DestroyImage(reconstruct_image);
  if (beta_image == (Image *) NULL)
    ThrowNCCSimilarityException();
  target_image=SIMSquareImage(beta_image,exception);
  beta_image=DestroyImage(beta_image);
  if (target_image == (Image *) NULL)
    ThrowNCCSimilarityException();
  status=SIMMultiplyImage(target_image,(double) QuantumRange,
    (const ChannelStatistics *) NULL,exception);
  if (status == MagickFalse)
    ThrowNCCSimilarityException();
  /*
    Compute the variance of the two images.
  */
  variance_image=SIMVarianceImage(alpha_image,target_image,exception);
  target_image=DestroyImage(target_image);
  alpha_image=DestroyImage(alpha_image);
  if (variance_image == (Image *) NULL)
    ThrowNCCSimilarityException();
  /*
    Subtract the image mean.
  */
  channel_statistics=GetImageStatistics(reconstruct,exception);
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowNCCSimilarityException();
  status=SIMMultiplyImage(variance_image,1.0,channel_statistics,exception);
  if (status == MagickFalse)
    ThrowNCCSimilarityException();
  normalize_image=SIMSubtractImageMean(image,reconstruct,channel_statistics,
    exception);
  channel_statistics=(ChannelStatistics *)
    RelinquishMagickMemory(channel_statistics);
  if (normalize_image == (Image *) NULL)
    ThrowNCCSimilarityException();
  correlation_image=SIMCrossCorrelationImage(image,normalize_image,exception);
  normalize_image=DestroyImage(normalize_image);
  if (correlation_image == (Image *) NULL)
    ThrowNCCSimilarityException();
  /*
    Divide the two images.
  */
  divide_image=SIMDivideImage(correlation_image,variance_image,exception);
  correlation_image=DestroyImage(correlation_image);
  variance_image=DestroyImage(variance_image);
  if (divide_image == (Image *) NULL)
    ThrowNCCSimilarityException();
  /*
    Crop padding.
  */
  SetGeometry(image,&geometry);
  geometry.width=image->columns;
  geometry.height=image->rows;
  (void) ResetImagePage(divide_image,"0x0+0+0");
  ncc_image=CropImage(divide_image,&geometry,exception);
  divide_image=DestroyImage(divide_image);
  if (ncc_image == (Image *) NULL)
    ThrowNCCSimilarityException();
  /*
    Identify the maxima value in the image and its location.
  */
  (void) ResetImagePage(ncc_image,"0x0+0+0");
  status=GrayscaleImage(ncc_image,AveragePixelIntensityMethod,exception);
  if (status == MagickFalse)
    ThrowNCCSimilarityException();
  ncc_image->depth=32;
  ncc_image->colorspace=GRAYColorspace;
  ncc_image->alpha_trait=UndefinedPixelTrait;
  status=SIMMaximaImage(ncc_image,&maxima,offset,exception);
  if (status == MagickFalse)
    ThrowNCCSimilarityException();
  if ((QuantumScale*maxima) > 1.0)
    {
      status=SIMMultiplyImage(ncc_image,1.0/(QuantumScale*maxima),
        (const ChannelStatistics *) NULL,exception);
      maxima=(double) QuantumRange;
    }
  *similarity_metric=QuantumScale*maxima;
  return(ncc_image);
}

static Image *PhaseSimilarityImage(const Image *image,const Image *reconstruct,
  RectangleInfo *offset,double *similarity_metric,ExceptionInfo *exception)
{
#define ThrowPhaseSimilarityException() \
{ \
  if (correlation_image != (Image *) NULL) \
    correlation_image=DestroyImage(correlation_image); \
  if (fft_images != (Image *) NULL) \
    fft_images=DestroyImageList(fft_images); \
  if (gamma_image != (Image *) NULL) \
    gamma_image=DestroyImage(gamma_image); \
  if (magnitude_image != (Image *) NULL) \
    magnitude_image=DestroyImage(magnitude_image); \
  if (phase_image != (Image *) NULL) \
    phase_image=DestroyImage(phase_image); \
  if (reconstruct_image != (Image *) NULL) \
    reconstruct_image=DestroyImage(reconstruct_image); \
  if (reconstruct_magnitude != (Image *) NULL) \
    reconstruct_magnitude=DestroyImage(reconstruct_magnitude); \
  if (target_image != (Image *) NULL) \
    target_image=DestroyImage(target_image); \
  if (test_magnitude != (Image *) NULL) \
    test_magnitude=DestroyImage(test_magnitude); \
  return((Image *) NULL); \
}

  double
    maxima = 0.0;

  Image
    *correlation_image = (Image *) NULL,
    *fft_images = (Image *) NULL,
    *gamma_image = (Image *) NULL,
    *magnitude_image = (Image *) NULL,
    *phase_image = (Image *) NULL,
    *reconstruct_image = (Image *) NULL,
    *reconstruct_magnitude = (Image *) NULL,
    *target_image = (Image *) NULL,
    *test_magnitude = (Image *) NULL;

  MagickBooleanType
    status = MagickTrue;

  RectangleInfo
    geometry;

  /*
    Phase correlation-based image similarity using FFT local statistics.
  */
  target_image=CloneImage(image,0,0,MagickTrue,exception);
  if (target_image == (Image *) NULL)
    ThrowPhaseSimilarityException();
  (void) ResetImagePage(target_image,"0x0+0+0");
  GetPixelInfoRGBA((Quantum) 0,(Quantum) 0,(Quantum) 0,(Quantum) 0,
    &target_image->background_color);
  status=SetImageExtent(target_image,2*(size_t) ceil((double) image->columns/
    2.0),2*(size_t) ceil((double) image->rows/2.0),exception);
  if (status == MagickFalse)
    ThrowPhaseSimilarityException();
  /*
    Compute the cross correlation of the test and reconstruct magnitudes.
  */
  reconstruct_image=CloneImage(reconstruct,0,0,MagickTrue,exception);
  if (reconstruct_image == (Image *) NULL)
    ThrowPhaseSimilarityException();
  (void) ResetImagePage(reconstruct_image,"0x0+0+0");
  GetPixelInfoRGBA((Quantum) 0,(Quantum) 0,(Quantum) 0,(Quantum) 0,
    &reconstruct_image->background_color);
  status=SetImageExtent(reconstruct_image,2*(size_t) ceil((double)
    image->columns/2.0),2*(size_t) ceil((double) image->rows/2.0),exception);
  if (status == MagickFalse)
    ThrowPhaseSimilarityException();
  /*
    Evaluate phase coorelation image and divide by the product magnitude.
  */
  (void) SetImageArtifact(target_image,"fourier:normalize","inverse");
  fft_images=ForwardFourierTransformImage(target_image,MagickTrue,exception);
  if (fft_images == (Image *) NULL)
    ThrowPhaseSimilarityException();
  test_magnitude=CloneImage(fft_images,0,0,MagickTrue,exception);
  fft_images=DestroyImageList(fft_images);
  if (test_magnitude == (Image *) NULL)
    ThrowPhaseSimilarityException();
  (void) SetImageArtifact(reconstruct_image,"fourier:normalize","inverse");
  fft_images=ForwardFourierTransformImage(reconstruct_image,MagickTrue,
    exception);
  if (fft_images == (Image *) NULL)
    ThrowPhaseSimilarityException();
  reconstruct_magnitude=CloneImage(fft_images,0,0,MagickTrue,exception);
  fft_images=DestroyImageList(fft_images);
  if (reconstruct_magnitude == (Image *) NULL)
    ThrowPhaseSimilarityException();
  magnitude_image=CloneImage(reconstruct_magnitude,0,0,MagickTrue,exception);
  if (magnitude_image == (Image *) NULL)
    ThrowPhaseSimilarityException();
  DisableCompositeClampUnlessSpecified(magnitude_image);
  (void) CompositeImage(magnitude_image,test_magnitude,MultiplyCompositeOp,
    MagickTrue,0,0,exception);
  /*
    Compute the cross correlation of the test and reconstruction images.
  */
  correlation_image=SIMPhaseCorrelationImage(target_image,reconstruct_image,
    magnitude_image,exception);
  target_image=DestroyImage(target_image);
  reconstruct_image=DestroyImage(reconstruct_image);
  test_magnitude=DestroyImage(test_magnitude);
  reconstruct_magnitude=DestroyImage(reconstruct_magnitude);
  if (correlation_image == (Image *) NULL)
    ThrowPhaseSimilarityException();
  /*
    Identify the maxima value in the image and its location.
  */
  gamma_image=CloneImage(correlation_image,0,0,MagickTrue,exception);
  correlation_image=DestroyImage(correlation_image);
  if (gamma_image == (Image *) NULL)
    ThrowPhaseSimilarityException();
  /*
    Crop padding.
  */
  SetGeometry(image,&geometry);
  geometry.width=image->columns;
  geometry.height=image->rows;
  (void) ResetImagePage(gamma_image,"0x0+0+0");
  phase_image=CropImage(gamma_image,&geometry,exception);
  gamma_image=DestroyImage(gamma_image);
  if (phase_image == (Image *) NULL)
    ThrowPhaseSimilarityException();
  (void) ResetImagePage(phase_image,"0x0+0+0");
  /*
    Identify the maxima value in the correlation image and its location.
  */
  status=GrayscaleImage(phase_image,AveragePixelIntensityMethod,exception);
  if (status == MagickFalse)
    ThrowPhaseSimilarityException();
  phase_image->depth=32;
  phase_image->colorspace=GRAYColorspace;
  phase_image->alpha_trait=UndefinedPixelTrait;
  status=SIMFilterImageNaNs(phase_image,exception);
  if (status == MagickFalse)
    ThrowPhaseSimilarityException();
  status=SIMMaximaImage(phase_image,&maxima,offset,exception);
  if (status == MagickFalse)
    ThrowPhaseSimilarityException();
  magnitude_image=DestroyImage(magnitude_image);
  if ((QuantumScale*maxima) > 1.0)
    {
      status=SIMMultiplyImage(phase_image,1.0/(QuantumScale*maxima),
        (const ChannelStatistics *) NULL,exception);
      maxima=(double) QuantumRange;
    }
  *similarity_metric=QuantumScale*maxima;
  return(phase_image);
}

static Image *PSNRSimilarityImage(const Image *image,const Image *reconstruct,
  RectangleInfo *offset,double *similarity_metric,ExceptionInfo *exception)
{
  Image
    *psnr_image = (Image *) NULL;

  psnr_image=MSESimilarityImage(image,reconstruct,offset,similarity_metric,
    exception);
  if (psnr_image == (Image *) NULL)
    return(psnr_image);
  *similarity_metric=10.0*MagickSafeLog10(MagickSafeReciprocal(
    *similarity_metric))/MagickSafePSNRRecipicol(10.0);
  return(psnr_image);
}

static Image *RMSESimilarityImage(const Image *image,const Image *reconstruct,
  RectangleInfo *offset,double *similarity_metric,ExceptionInfo *exception)
{
  Image
    *rmse_image = (Image *) NULL;

  rmse_image=MSESimilarityImage(image,reconstruct,offset,similarity_metric,
    exception);
  if (rmse_image == (Image *) NULL)
    return(rmse_image);
  *similarity_metric=sqrt(*similarity_metric);
  return(rmse_image);
}
#endif

static double GetSimilarityMetric(const Image *image,
  const Image *reconstruct_image,const MetricType metric,
  const ssize_t x_offset,const ssize_t y_offset,ExceptionInfo *exception)
{
  double
    *channel_similarity,
    similarity = 0.0;

  ExceptionInfo
    *sans_exception = AcquireExceptionInfo();

  Image
    *similarity_image;

  MagickBooleanType
    status = MagickTrue;

  RectangleInfo
    geometry;

  size_t
    length = MaxPixelChannels+1UL;

  SetGeometry(reconstruct_image,&geometry);
  geometry.x=x_offset;
  geometry.y=y_offset;
  similarity_image=CropImage(image,&geometry,sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (similarity_image == (Image *) NULL)
    return(NAN);
  /*
    Get image distortion.
  */
  channel_similarity=(double *) AcquireQuantumMemory(length,
    sizeof(*channel_similarity));
  if (channel_similarity == (double *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(channel_similarity,0,length*sizeof(*channel_similarity));
  switch (metric)
  {
    case AbsoluteErrorMetric:
    {
      status=GetAESimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case DotProductCorrelationErrorMetric:
    {
      status=GetDPCSimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case FuzzErrorMetric:
    {
      status=GetFUZZSimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case MeanAbsoluteErrorMetric:
    {
      status=GetMAESimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case MeanErrorPerPixelErrorMetric:
    {
      status=GetMEPPSimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case MeanSquaredErrorMetric:
    {
      status=GetMSESimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case NormalizedCrossCorrelationErrorMetric:
    {
      status=GetNCCSimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case PeakAbsoluteErrorMetric:
    {
      status=GetPASimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case PeakSignalToNoiseRatioErrorMetric:
    {
      status=GetPSNRSimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case PerceptualHashErrorMetric:
    {
      status=GetPHASHSimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case PhaseCorrelationErrorMetric:
    {
      status=GetPHASESimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case RootMeanSquaredErrorMetric:
    case UndefinedErrorMetric:
    default:
    {
      status=GetRMSESimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case StructuralDissimilarityErrorMetric:
    {
      status=GetDSSIMSimilarity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
    case StructuralSimilarityErrorMetric:
    {
      status=GetSSIMSimularity(similarity_image,reconstruct_image,
        channel_similarity,exception);
      break;
    }
  }
  similarity_image=DestroyImage(similarity_image);
  similarity=channel_similarity[CompositePixelChannel];
  channel_similarity=(double *) RelinquishMagickMemory(channel_similarity);
  if (status == MagickFalse)
    return(NAN);
  return(similarity);
}

MagickExport Image *SimilarityImage(const Image *image,const Image *reconstruct,
  const MetricType metric,const double similarity_threshold,
  RectangleInfo *offset,double *similarity_metric,ExceptionInfo *exception)
{
#define SimilarityImageTag  "Similarity/Image"

  typedef struct
  {
    double
      similarity;

    ssize_t
      x,
      y;
  } SimilarityInfo;

  CacheView
    *similarity_view;

  Image
    *similarity_image = (Image *) NULL;

  MagickBooleanType
    status = MagickTrue;

  MagickOffsetType
    progress = 0;

  SimilarityInfo
    similarity_info = { 0.0, 0, 0 };

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  assert(offset != (RectangleInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  SetGeometry(reconstruct,offset);
  *similarity_metric=0.0;
  offset->x=0;
  offset->y=0;
#if defined(MAGICKCORE_HDRI_SUPPORT) && defined(MAGICKCORE_FFTW_DELEGATE)
{
  const char *artifact = GetImageArtifact(image,"compare:frequency-domain");
  if (artifact == (const char *) NULL)
    artifact=GetImageArtifact(image,"compare:accelerate-ncc");
  if (((artifact == (const char *) NULL) ||
       (IsStringTrue(artifact) != MagickFalse)) &&
      ((image->channels & ReadMaskChannel) == 0))
    switch (metric)
    {
      case DotProductCorrelationErrorMetric:
      {
        similarity_image=DPCSimilarityImage(image,reconstruct,offset,
          similarity_metric,exception);
        return(similarity_image);
      }
      case MeanSquaredErrorMetric:
      {
        similarity_image=MSESimilarityImage(image,reconstruct,offset,
          similarity_metric,exception);
        return(similarity_image);
      }
      case NormalizedCrossCorrelationErrorMetric:
      {
        similarity_image=NCCSimilarityImage(image,reconstruct,offset,
          similarity_metric,exception);
        return(similarity_image);
      }
      case PeakSignalToNoiseRatioErrorMetric:
      {
        similarity_image=PSNRSimilarityImage(image,reconstruct,offset,
          similarity_metric,exception);
        return(similarity_image);
      }
      case PhaseCorrelationErrorMetric:
      {
        similarity_image=PhaseSimilarityImage(image,reconstruct,offset,
          similarity_metric,exception);
        return(similarity_image);
      }
      case RootMeanSquaredErrorMetric:
      case UndefinedErrorMetric:
      {
        similarity_image=RMSESimilarityImage(image,reconstruct,offset,
          similarity_metric,exception);
        return(similarity_image);
      }
      default:
        break;
    }
}
#endif
  if ((image->columns < reconstruct->columns) ||
      (image->rows < reconstruct->rows))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
        "GeometryDoesNotContainImage","`%s'",image->filename);
      return((Image *) NULL);
    }
  similarity_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (similarity_image == (Image *) NULL)
    return((Image *) NULL);
  similarity_image->depth=32;
  similarity_image->colorspace=GRAYColorspace;
  similarity_image->alpha_trait=UndefinedPixelTrait;
  status=SetImageStorageClass(similarity_image,DirectClass,exception);
  if (status == MagickFalse)
    return(DestroyImage(similarity_image));
  /*
    Measure similarity of reconstruction image against image.
  */
  similarity_info.similarity=GetSimilarityMetric(image,reconstruct,metric,
    similarity_info.x,similarity_info.y,exception);
  similarity_view=AcquireAuthenticCacheView(similarity_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(similarity_info,status) \
    magick_number_threads(image,reconstruct,similarity_image->rows,1)
#endif
  for (y=0; y < (ssize_t) similarity_image->rows; y++)
  {
    double
      similarity;

    MagickBooleanType
      threshold_trigger = MagickFalse;

    Quantum
      *magick_restrict q;

    SimilarityInfo
      channel_info = similarity_info;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    if (threshold_trigger != MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(similarity_view,0,y,
      similarity_image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) similarity_image->columns; x++)
    {
      ssize_t
        i;

      similarity=GetSimilarityMetric((Image *) image,reconstruct,metric,x,y,
        exception);
      switch (metric)
      {
        case DotProductCorrelationErrorMetric:
        case NormalizedCrossCorrelationErrorMetric:
        case PeakSignalToNoiseRatioErrorMetric:
        case PhaseCorrelationErrorMetric:
        case StructuralSimilarityErrorMetric:
        {
          if (similarity <= channel_info.similarity)
            break;
          channel_info.similarity=similarity;
          channel_info.x=x;
          channel_info.y=y;
          break;
        }
        default:
        {
          if (similarity >= channel_info.similarity)
            break;
          channel_info.similarity=similarity;
          channel_info.x=x;
          channel_info.y=y;
          break;
        }
      }
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        PixelTrait similarity_traits = GetPixelChannelTraits(similarity_image,
          channel);
        if (((traits & UpdatePixelTrait) == 0) ||
            ((similarity_traits & UpdatePixelTrait) == 0))
          continue;
        switch (metric)
        {
          case DotProductCorrelationErrorMetric:
          case NormalizedCrossCorrelationErrorMetric:
          case PeakSignalToNoiseRatioErrorMetric:
          case PhaseCorrelationErrorMetric:
          case StructuralSimilarityErrorMetric:
          {
            SetPixelChannel(similarity_image,channel,ClampToQuantum((double)
              QuantumRange*similarity),q);
            break;
          }
          default:
          {
            SetPixelChannel(similarity_image,channel,ClampToQuantum((double)
              QuantumRange*(1.0-similarity)),q);
            break;
          }
        }
      }
      q+=(ptrdiff_t) GetPixelChannels(similarity_image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
    #pragma omp critical (MagickCore_GetSimilarityMetric)
#endif
    switch (metric)
    {
      case DotProductCorrelationErrorMetric:
      case NormalizedCrossCorrelationErrorMetric:
      case PeakSignalToNoiseRatioErrorMetric:
      case PhaseCorrelationErrorMetric:
      case StructuralSimilarityErrorMetric:
      {
        if (similarity_threshold != DefaultSimilarityThreshold)
          if (channel_info.similarity >= similarity_threshold)
            threshold_trigger=MagickTrue;
        if (channel_info.similarity >= similarity_info.similarity)
          similarity_info=channel_info;
        break;
      }
      default:
      {
        if (similarity_threshold != DefaultSimilarityThreshold)
          if (channel_info.similarity < similarity_threshold)
            threshold_trigger=MagickTrue;
        if (channel_info.similarity < similarity_info.similarity)
          similarity_info=channel_info;
        break;
      }
    }
    if (SyncCacheViewAuthenticPixels(similarity_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        progress++;
        proceed=SetImageProgress(image,SimilarityImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  similarity_view=DestroyCacheView(similarity_view);
  if (status == MagickFalse)
    similarity_image=DestroyImage(similarity_image);
  *similarity_metric=similarity_info.similarity;
  if (fabs(*similarity_metric) < MagickEpsilon)
    *similarity_metric=0.0;
  offset->x=similarity_info.x;
  offset->y=similarity_info.y;
  (void) FormatImageProperty((Image *) image,"similarity","%.*g",
    GetMagickPrecision(),*similarity_metric);
  (void) FormatImageProperty((Image *) image,"similarity.offset.x","%.*g",
    GetMagickPrecision(),(double) offset->x);
  (void) FormatImageProperty((Image *) image,"similarity.offset.y","%.*g",
    GetMagickPrecision(),(double) offset->y);
  return(similarity_image);
}

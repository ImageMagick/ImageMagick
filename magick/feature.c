/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               FFFFF  EEEEE   AAA   TTTTT  U   U  RRRR   EEEEE               %
%               F      E      A   A    T    U   U  R   R  E                   %
%               FFF    EEE    AAAAA    T    U   U  RRRR   EEE                 %
%               F      E      A   A    T    U   U  R R    E                   %
%               F      EEEEE  A   A    T     UUU   R  R   EEEEE               %
%                                                                             %
%                                                                             %
%                      MagickCore Image Feature Methods                       %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/animate.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/cache-view.h"
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/compress.h"
#include "magick/constitute.h"
#include "magick/deprecate.h"
#include "magick/display.h"
#include "magick/draw.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/feature.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/image-private.h"
#include "magick/magic.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/paint.h"
#include "magick/pixel-private.h"
#include "magick/profile.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/segment.h"
#include "magick/semaphore.h"
#include "magick/signature-private.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/timer.h"
#include "magick/utility.h"
#include "magick/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l F e a t u r e s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelFeatures() returns features for each channel in the image in
%  each of four directions (horizontal, vertical, left and right diagonals)
%  for the specified distance.  The features include the angular second
%  moment, contrast, correlation, sum of squares: variance, inverse difference
%  moment, sum average, sum varience, sum entropy, entropy, difference variance,%  difference entropy, information measures of correlation 1, information
%  measures of correlation 2, and maximum correlation coefficient.  You can
%  access the red channel contrast, for example, like this:
%
%      channel_features=GetImageChannelFeatures(image,1,excepton);
%      contrast=channel_features[RedChannel].contrast[0];
%
%  Use MagickRelinquishMemory() to free the features buffer.
%
%  The format of the GetImageChannelFeatures method is:
%
%      ChannelFeatures *GetImageChannelFeatures(const Image *image,
%        const unsigned long distance,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o distance: the distance.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ChannelFeatures *GetImageChannelFeatures(const Image *image,
  const unsigned long distance,ExceptionInfo *exception)
{
  typedef struct _ChannelStatistics
  {
    DoublePixelPacket
      direction[4];  /* horizontal, vertical, left and right diagonals */
  } ChannelStatistics;

  CacheView
    *image_view;

  ChannelFeatures
    *channel_features;

  ChannelStatistics
    **cooccurrence,
    correlation,
    mean,
    *sum,
    sum_squares;

  LongPixelPacket
    gray,
    *grays;

  long
    y,
    z;

  MagickBooleanType
    status;

  register long
    i;

  size_t
    length;

  unsigned long
    number_grays;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns < (distance+1)) || (image->rows < (distance+1)))
    return((ChannelFeatures *) NULL);
  length=AllChannels+1UL;
  channel_features=(ChannelFeatures *) AcquireQuantumMemory(length,
    sizeof(*channel_features));
  if (channel_features == (ChannelFeatures *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_features,0,length*
    sizeof(*channel_features));
  /*
    Form grays.
  */
  grays=(LongPixelPacket *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*grays));
  if (grays == (LongPixelPacket *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  for (i=0; i <= (long) MaxMap; i++)
  {
    grays[i].red=(~0UL);
    grays[i].green=(~0UL);
    grays[i].blue=(~0UL);
    grays[i].opacity=(~0UL);
    grays[i].index=(~0UL);
  }
  status=MagickTrue;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register long
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      grays[ScaleQuantumToMap(p->red)].red=ScaleQuantumToMap(p->red);
      grays[ScaleQuantumToMap(p->green)].green=ScaleQuantumToMap(p->green);
      grays[ScaleQuantumToMap(p->blue)].blue=ScaleQuantumToMap(p->blue);
      if (image->matte != MagickFalse)
        grays[ScaleQuantumToMap(p->opacity)].opacity=
          ScaleQuantumToMap(p->opacity);
      if (image->colorspace == CMYKColorspace)
        grays[ScaleQuantumToMap(indexes[x])].index=
          ScaleQuantumToMap(indexes[x]);
      p++;
    }
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    {
      grays=(LongPixelPacket *) RelinquishMagickMemory(grays);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  (void) ResetMagickMemory(&gray,0,sizeof(gray));
  for (i=0; i <= (long) MaxMap; i++)
  {
    if (grays[i].red != ~0UL)
      grays[gray.red++].red=grays[i].red;
    if (grays[i].green != ~0UL)
      grays[gray.green++].green=grays[i].green;
    if (grays[i].blue != ~0UL)
      grays[gray.blue++].blue=grays[i].blue;
    if (image->matte != MagickFalse)
      if (grays[i].opacity != ~0UL)
        grays[gray.opacity++].opacity=grays[i].opacity;
    if (image->colorspace == CMYKColorspace)
      if (grays[i].index != ~0UL)
        grays[gray.index++].index=grays[i].index;
  }
  /*
    Allocate spatial dependence matrix.
  */
  number_grays=gray.red;
  if (gray.green > number_grays)
    number_grays=gray.green;
  if (gray.blue > number_grays)
    number_grays=gray.blue;
  if (image->matte != MagickFalse)
    if (gray.opacity > number_grays)
      number_grays=gray.opacity;
  if (image->colorspace == CMYKColorspace)
    if (gray.index > number_grays)
      number_grays=gray.index;
  cooccurrence=(ChannelStatistics **) AcquireQuantumMemory(number_grays,
    sizeof(*cooccurrence));
  if (cooccurrence == (ChannelStatistics **) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      grays=(LongPixelPacket *) RelinquishMagickMemory(grays);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  for (i=0; i < (long) number_grays; i++)
  {
    cooccurrence[i]=(ChannelStatistics *) AcquireQuantumMemory(number_grays,
      sizeof(**cooccurrence));
    if (cooccurrence[i] == (ChannelStatistics *) NULL)
      break;
    (void) ResetMagickMemory(cooccurrence[i],0,number_grays*
      sizeof(*cooccurrence));
  }
  if (i < (long) number_grays)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      for (i--; i >= 0; i--)
        cooccurrence[i]=(ChannelStatistics *)
          RelinquishMagickMemory(cooccurrence[i]);
      cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(cooccurrence);
      grays=(LongPixelPacket *) RelinquishMagickMemory(grays);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  /*
    Initialize spatial dependence matrix.
  */
  status=MagickTrue;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    long
      u,
      v;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register long
      x;

    ssize_t
      offset;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-(long) distance,y,image->columns+
      2*distance,distance+1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    p+=distance;
    indexes+=distance;
    for (x=0; x < (long) image->columns; x++)
    {
      for (i=0; i < 4; i++)
      {
        switch (i)
        {
          case 0:
          default:
          {
            /*
              Horizontal adjacency.
            */
            offset=(ssize_t) distance;
            break;
          }
          case 1:
          {
            /*
              Vertical adjacency.
            */
            offset=(ssize_t) (image->columns+2*distance);
            break;
          }
          case 2:
          {
            /*
              Right diagonal adjacency.
            */
            offset=(ssize_t) (image->columns+2*distance)-distance;
            break;
          }
          case 3:
          {
            /*
              Left diagonal adjacency.
            */
            offset=(ssize_t) (image->columns+2*distance)+distance;
            break;
          }
        }
        u=0;
        v=0;
        while (grays[u].red != ScaleQuantumToMap(p->red))
          u++;
        while (grays[v].red != ScaleQuantumToMap((p+offset)->red))
          v++;
        cooccurrence[u][v].direction[i].red++;
        cooccurrence[v][u].direction[i].red++;
        u=0;
        v=0;
        while (grays[u].green != ScaleQuantumToMap(p->green))
          u++;
        while (grays[v].green != ScaleQuantumToMap((p+offset)->green))
          v++;
        cooccurrence[u][v].direction[i].green++;
        cooccurrence[v][u].direction[i].green++;
        u=0;
        v=0;
        while (grays[u].blue != ScaleQuantumToMap(p->blue))
          u++;
        while (grays[v].blue != ScaleQuantumToMap((p+offset)->blue))
          v++;
        cooccurrence[u][v].direction[i].blue++;
        cooccurrence[v][u].direction[i].blue++;
        if (image->matte != MagickFalse)
          {
            u=0;
            v=0;
            while (grays[u].opacity != ScaleQuantumToMap(p->opacity))
              u++;
            while (grays[v].opacity != ScaleQuantumToMap((p+offset)->opacity))
              v++;
            cooccurrence[u][v].direction[i].opacity++;
            cooccurrence[v][u].direction[i].opacity++;
          }
        if (image->colorspace == CMYKColorspace)
          {
            u=0;
            v=0;
            while (grays[u].index != ScaleQuantumToMap(indexes[x]))
              u++;
            while (grays[v].index != ScaleQuantumToMap(indexes[x+offset]))
              v++;
            cooccurrence[u][v].direction[i].index++;
            cooccurrence[v][u].direction[i].index++;
          }
      }
      p++;
    }
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      for (i=0; i < (long) number_grays; i++)
        cooccurrence[i]=(ChannelStatistics *)
          RelinquishMagickMemory(cooccurrence[i]);
      cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(cooccurrence);
      grays=(LongPixelPacket *) RelinquishMagickMemory(grays);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  /*
    Normalize spatial dependence matrix.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (i=0; i < 4; i++)
  {
    double
      normalize;

    switch (i)
    {
      case 0:
      default:
      {
        /*
          Horizontal adjacency.
        */
        normalize=2.0*image->rows*(image->columns-distance);
        break;
      }
      case 1:
      {
        /*
          Vertical adjacency.
        */
        normalize=2.0*(image->rows-distance)*image->columns;
        break;
      }
      case 2:
      {
        /*
          Right diagonal adjacency.
        */
        normalize=2.0*(image->rows-distance)*(image->columns-distance);
        break;
      }
      case 3:
      {
        /*
          Left diagonal adjacency.
        */
        normalize=2.0*(image->rows-distance)*(image->columns-distance);
        break;
      }
    }
    for (y=0; y < (long) number_grays; y++)
    {
      register long
        x;

      for (x=0; x < (long) number_grays; x++)
      {
        cooccurrence[x][y].direction[i].red/=normalize;
        cooccurrence[x][y].direction[i].green/=normalize;
        cooccurrence[x][y].direction[i].blue/=normalize;
        if (image->matte != MagickFalse)
          cooccurrence[x][y].direction[i].opacity/=normalize;
        if (image->colorspace == CMYKColorspace)
          cooccurrence[x][y].direction[i].index/=normalize;
      }
    }
  }
  /*
    Compute texture features.
  */
  sum=(ChannelStatistics *) AcquireQuantumMemory(number_grays,sizeof(*sum));
  if (sum == (ChannelStatistics *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      for (i=0; i < (long) number_grays; i++)
        cooccurrence[i]=(ChannelStatistics *)
          RelinquishMagickMemory(cooccurrence[i]);
      cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(cooccurrence);
      grays=(LongPixelPacket *) RelinquishMagickMemory(grays);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  (void) ResetMagickMemory(sum,0,number_grays*sizeof(*sum));
  (void) ResetMagickMemory(&correlation,0,sizeof(correlation));
  (void) ResetMagickMemory(&mean,0,sizeof(mean));
  (void) ResetMagickMemory(&sum_squares,0,sizeof(sum_squares));
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (i=0; i < 4; i++)
  {
    register long
      y;

    for (y=0; y < (long) number_grays; y++)
    {
      register long
        x;

      for (x=0; x < (long) number_grays; x++)
      {
        /*
          Angular second moment:  measure of homogeneity of the image.
        */
        channel_features[RedChannel].angular_second_moment[i]+=
          cooccurrence[x][y].direction[i].red*
          cooccurrence[x][y].direction[i].red;
        channel_features[GreenChannel].angular_second_moment[i]+=
          cooccurrence[x][y].direction[i].green*
          cooccurrence[x][y].direction[i].green;
        channel_features[BlueChannel].angular_second_moment[i]+=
          cooccurrence[x][y].direction[i].blue*
          cooccurrence[x][y].direction[i].blue;
        if (image->matte != MagickFalse)
          channel_features[OpacityChannel].angular_second_moment[i]+=
            cooccurrence[x][y].direction[i].opacity*
            cooccurrence[x][y].direction[i].opacity;
        if (image->colorspace == CMYKColorspace)
          channel_features[BlackChannel].angular_second_moment[i]+=
            cooccurrence[x][y].direction[i].index*
            cooccurrence[x][y].direction[i].index;
        /*
          Correlation: measure of linear-dependencies in the image.
        */
        sum[y].direction[i].red+=cooccurrence[x][y].direction[i].red;
        correlation.direction[i].red+=x*y*cooccurrence[x][y].direction[i].red;
        mean.direction[i].red+=y*sum[y].direction[i].red;
        sum_squares.direction[i].red+=y*y*sum[y].direction[i].red;
        sum[y].direction[i].green+=cooccurrence[x][y].direction[i].green;
        correlation.direction[i].green+=x*y*
          cooccurrence[x][y].direction[i].green;
        mean.direction[i].green+=y*sum[y].direction[i].green;
        sum_squares.direction[i].green+=y*y*sum[y].direction[i].green;
        sum[y].direction[i].blue+=cooccurrence[x][y].direction[i].blue;
        correlation.direction[i].blue+=x*y*
          cooccurrence[x][y].direction[i].blue;
        mean.direction[i].blue+=y*sum[y].direction[i].blue;
        sum_squares.direction[i].blue+=y*y*sum[y].direction[i].blue;
        if (image->matte != MagickFalse)
          {
            sum[y].direction[i].opacity+=
              cooccurrence[x][y].direction[i].opacity;
            correlation.direction[i].opacity+=x*y*
              cooccurrence[x][y].direction[i].opacity;
            mean.direction[i].opacity+=y*sum[y].direction[i].opacity;
            sum_squares.direction[i].opacity+=y*y*sum[y].direction[i].opacity;
          }
        if (image->colorspace == CMYKColorspace)
          {
            sum[y].direction[i].index+=cooccurrence[x][y].direction[i].index;
            correlation.direction[i].index+=x*y*
              cooccurrence[x][y].direction[i].index;
            mean.direction[i].index+=y*sum[y].direction[i].index;
            sum_squares.direction[i].index+=y*y*sum[y].direction[i].index;
          }
      }
    }
  }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (i=0; i < 4; i++)
  {
    for (z=0; z < (long) number_grays; z++)
    {
      register long
        y;

      ChannelStatistics
        pixel;

      (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
      for (y=0; y < (long) number_grays; y++)
      {
        register long
          x;

        for (x=0; x < (long) number_grays; x++)
        {
          /*
            Contrast:  amount of local variations present in an image.
          */
          if (((y-x) == z) || ((x-y) == z))
            {
              pixel.direction[i].red+=cooccurrence[x][y].direction[i].red;
              pixel.direction[i].green+=cooccurrence[x][y].direction[i].green;
              pixel.direction[i].blue+=cooccurrence[x][y].direction[i].blue;
              if (image->matte != MagickFalse)
                pixel.direction[i].opacity+=
                  cooccurrence[x][y].direction[i].opacity;
              if (image->colorspace == CMYKColorspace)
                pixel.direction[i].index+=cooccurrence[x][y].direction[i].index;
            }
        }
      }
      channel_features[RedChannel].contrast[i]+=z*z*pixel.direction[i].red;
      channel_features[GreenChannel].contrast[i]+=z*z*pixel.direction[i].green;
      channel_features[BlueChannel].contrast[i]+=z*z*pixel.direction[i].blue;
      if (image->matte != MagickFalse)
        channel_features[OpacityChannel].contrast[i]+=z*z*
          pixel.direction[i].opacity;
      if (image->colorspace == CMYKColorspace)
        channel_features[BlackChannel].contrast[i]+=z*z*
          pixel.direction[i].index;
    }
  }
  /*
    Relinquish resources.
  */
  sum=(ChannelStatistics *) RelinquishMagickMemory(sum);
  for (i=0; i < (long) number_grays; i++)
    cooccurrence[i]=(ChannelStatistics *)
      RelinquishMagickMemory(cooccurrence[i]);
  cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(cooccurrence);
  grays=(LongPixelPacket *) RelinquishMagickMemory(grays);
  return(channel_features);
}

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
%                                   Cristy                                    %
%                                 July 1992                                   %
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
#include "magick/resource_.h"
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
%      channel_features=GetImageChannelFeatures(image,1,exception);
%      contrast=channel_features[RedChannel].contrast[0];
%
%  Use MagickRelinquishMemory() to free the features buffer.
%
%  The format of the GetImageChannelFeatures method is:
%
%      ChannelFeatures *GetImageChannelFeatures(const Image *image,
%        const size_t distance,ExceptionInfo *exception)
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

static inline ssize_t MagickAbsoluteValue(const ssize_t x)
{
  if (x < 0)
    return(-x);
  return(x);
}

MagickExport ChannelFeatures *GetImageChannelFeatures(const Image *image,
  const size_t distance,ExceptionInfo *exception)
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
    *density_x,
    *density_xy,
    *density_y,
    entropy_x,
    entropy_xy,
    entropy_xy1,
    entropy_xy2,
    entropy_y,
    mean,
    **Q,
    *sum,
    sum_squares,
    variance;

  LongPixelPacket
    gray,
    *grays;

  MagickBooleanType
    status;

  register ssize_t
    i;

  size_t
    length;

  ssize_t
    y;

  unsigned int
    number_grays;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns < (distance+1)) || (image->rows < (distance+1)))
    return((ChannelFeatures *) NULL);
  length=CompositeChannels+1UL;
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
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(channel_features);
    }
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    grays[i].red=(~0U);
    grays[i].green=(~0U);
    grays[i].blue=(~0U);
    grays[i].opacity=(~0U);
    grays[i].index=(~0U);
  }
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register ssize_t
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
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      grays[ScaleQuantumToMap(GetPixelRed(p))].red=
        ScaleQuantumToMap(GetPixelRed(p));
      grays[ScaleQuantumToMap(GetPixelGreen(p))].green=
        ScaleQuantumToMap(GetPixelGreen(p));
      grays[ScaleQuantumToMap(GetPixelBlue(p))].blue=
        ScaleQuantumToMap(GetPixelBlue(p));
      if (image->colorspace == CMYKColorspace)
        grays[ScaleQuantumToMap(GetPixelIndex(indexes+x))].index=
          ScaleQuantumToMap(GetPixelIndex(indexes+x));
      if (image->matte != MagickFalse)
        grays[ScaleQuantumToMap(GetPixelOpacity(p))].opacity=
          ScaleQuantumToMap(GetPixelOpacity(p));
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
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if (grays[i].red != ~0U)
      grays[(ssize_t) gray.red++].red=grays[i].red;
    if (grays[i].green != ~0U)
      grays[(ssize_t) gray.green++].green=grays[i].green;
    if (grays[i].blue != ~0U)
      grays[(ssize_t) gray.blue++].blue=grays[i].blue;
    if (image->colorspace == CMYKColorspace)
      if (grays[i].index != ~0U)
        grays[(ssize_t) gray.index++].index=grays[i].index;
    if (image->matte != MagickFalse)
      if (grays[i].opacity != ~0U)
        grays[(ssize_t) gray.opacity++].opacity=grays[i].opacity;
  }
  /*
    Allocate spatial dependence matrix.
  */
  number_grays=gray.red;
  if (gray.green > number_grays)
    number_grays=gray.green;
  if (gray.blue > number_grays)
    number_grays=gray.blue;
  if (image->colorspace == CMYKColorspace)
    if (gray.index > number_grays)
      number_grays=gray.index;
  if (image->matte != MagickFalse)
    if (gray.opacity > number_grays)
      number_grays=gray.opacity;
  cooccurrence=(ChannelStatistics **) AcquireQuantumMemory(number_grays,
    sizeof(*cooccurrence));
  density_x=(ChannelStatistics *) AcquireQuantumMemory(2*(number_grays+1),
    sizeof(*density_x));
  density_xy=(ChannelStatistics *) AcquireQuantumMemory(2*(number_grays+1),
    sizeof(*density_xy));
  density_y=(ChannelStatistics *) AcquireQuantumMemory(2*(number_grays+1),
    sizeof(*density_y));
  Q=(ChannelStatistics **) AcquireQuantumMemory(number_grays,sizeof(*Q));
  sum=(ChannelStatistics *) AcquireQuantumMemory(number_grays,sizeof(*sum));
  if ((cooccurrence == (ChannelStatistics **) NULL) ||
      (density_x == (ChannelStatistics *) NULL) ||
      (density_xy == (ChannelStatistics *) NULL) ||
      (density_y == (ChannelStatistics *) NULL) ||
      (Q == (ChannelStatistics **) NULL) ||
      (sum == (ChannelStatistics *) NULL))
    {
      if (Q != (ChannelStatistics **) NULL)
        {
          for (i=0; i < (ssize_t) number_grays; i++)
            Q[i]=(ChannelStatistics *) RelinquishMagickMemory(Q[i]);
          Q=(ChannelStatistics **) RelinquishMagickMemory(Q);
        }
      if (sum != (ChannelStatistics *) NULL)
        sum=(ChannelStatistics *) RelinquishMagickMemory(sum);
      if (density_y != (ChannelStatistics *) NULL)
        density_y=(ChannelStatistics *) RelinquishMagickMemory(density_y);
      if (density_xy != (ChannelStatistics *) NULL)
        density_xy=(ChannelStatistics *) RelinquishMagickMemory(density_xy);
      if (density_x != (ChannelStatistics *) NULL)
        density_x=(ChannelStatistics *) RelinquishMagickMemory(density_x);
      if (cooccurrence != (ChannelStatistics **) NULL)
        {
          for (i=0; i < (ssize_t) number_grays; i++)
            cooccurrence[i]=(ChannelStatistics *)
              RelinquishMagickMemory(cooccurrence[i]);
          cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(
            cooccurrence);
        }
      grays=(LongPixelPacket *) RelinquishMagickMemory(grays);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(channel_features);
    }
  (void) ResetMagickMemory(&correlation,0,sizeof(correlation));
  (void) ResetMagickMemory(density_x,0,2*(number_grays+1)*sizeof(*density_x));
  (void) ResetMagickMemory(density_xy,0,2*(number_grays+1)*sizeof(*density_xy));
  (void) ResetMagickMemory(density_y,0,2*(number_grays+1)*sizeof(*density_y));
  (void) ResetMagickMemory(&mean,0,sizeof(mean));
  (void) ResetMagickMemory(sum,0,number_grays*sizeof(*sum));
  (void) ResetMagickMemory(&sum_squares,0,sizeof(sum_squares));
  (void) ResetMagickMemory(density_xy,0,2*number_grays*sizeof(*density_xy));
  (void) ResetMagickMemory(&entropy_x,0,sizeof(entropy_x));
  (void) ResetMagickMemory(&entropy_xy,0,sizeof(entropy_xy));
  (void) ResetMagickMemory(&entropy_xy1,0,sizeof(entropy_xy1));
  (void) ResetMagickMemory(&entropy_xy2,0,sizeof(entropy_xy2));
  (void) ResetMagickMemory(&entropy_y,0,sizeof(entropy_y));
  (void) ResetMagickMemory(&variance,0,sizeof(variance));
  for (i=0; i < (ssize_t) number_grays; i++)
  {
    cooccurrence[i]=(ChannelStatistics *) AcquireQuantumMemory(number_grays,
      sizeof(**cooccurrence));
    Q[i]=(ChannelStatistics *) AcquireQuantumMemory(number_grays,sizeof(**Q));
    if ((cooccurrence[i] == (ChannelStatistics *) NULL) ||
        (Q[i] == (ChannelStatistics *) NULL))
      break;
    (void) ResetMagickMemory(cooccurrence[i],0,number_grays*
      sizeof(**cooccurrence));
    (void) ResetMagickMemory(Q[i],0,number_grays*sizeof(**Q));
  }
  if (i < (ssize_t) number_grays)
    {
      for (i--; i >= 0; i--)
      {
        if (Q[i] != (ChannelStatistics *) NULL)
          Q[i]=(ChannelStatistics *) RelinquishMagickMemory(Q[i]);
        if (cooccurrence[i] != (ChannelStatistics *) NULL)
          cooccurrence[i]=(ChannelStatistics *)
            RelinquishMagickMemory(cooccurrence[i]);
      }
      Q=(ChannelStatistics **) RelinquishMagickMemory(Q);
      cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(cooccurrence);
      sum=(ChannelStatistics *) RelinquishMagickMemory(sum);
      density_y=(ChannelStatistics *) RelinquishMagickMemory(density_y);
      density_xy=(ChannelStatistics *) RelinquishMagickMemory(density_xy);
      density_x=(ChannelStatistics *) RelinquishMagickMemory(density_x);
      grays=(LongPixelPacket *) RelinquishMagickMemory(grays);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(channel_features);
    }
  /*
    Initialize spatial dependence matrix.
  */
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register ssize_t
      x;

    ssize_t
      i,
      offset,
      u,
      v;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-(ssize_t) distance,y,image->columns+
      2*distance,distance+2,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    p+=distance;
    indexes+=distance;
    for (x=0; x < (ssize_t) image->columns; x++)
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
            offset=(ssize_t) ((image->columns+2*distance)-distance);
            break;
          }
          case 3:
          {
            /*
              Left diagonal adjacency.
            */
            offset=(ssize_t) ((image->columns+2*distance)+distance);
            break;
          }
        }
        u=0;
        v=0;
        while (grays[u].red != ScaleQuantumToMap(GetPixelRed(p)))
          u++;
        while (grays[v].red != ScaleQuantumToMap(GetPixelRed(p+offset)))
          v++;
        cooccurrence[u][v].direction[i].red++;
        cooccurrence[v][u].direction[i].red++;
        u=0;
        v=0;
        while (grays[u].green != ScaleQuantumToMap(GetPixelGreen(p)))
          u++;
        while (grays[v].green != ScaleQuantumToMap(GetPixelGreen(p+offset)))
          v++;
        cooccurrence[u][v].direction[i].green++;
        cooccurrence[v][u].direction[i].green++;
        u=0;
        v=0;
        while (grays[u].blue != ScaleQuantumToMap(GetPixelBlue(p)))
          u++;
        while (grays[v].blue != ScaleQuantumToMap((p+offset)->blue))
          v++;
        cooccurrence[u][v].direction[i].blue++;
        cooccurrence[v][u].direction[i].blue++;
        if (image->colorspace == CMYKColorspace)
          {
            u=0;
            v=0;
            while (grays[u].index != ScaleQuantumToMap(GetPixelIndex(indexes+x)))
              u++;
            while (grays[v].index != ScaleQuantumToMap(GetPixelIndex(indexes+x+offset)))
              v++;
            cooccurrence[u][v].direction[i].index++;
            cooccurrence[v][u].direction[i].index++;
          }
        if (image->matte != MagickFalse)
          {
            u=0;
            v=0;
            while (grays[u].opacity != ScaleQuantumToMap(GetPixelOpacity(p)))
              u++;
            while (grays[v].opacity != ScaleQuantumToMap((p+offset)->opacity))
              v++;
            cooccurrence[u][v].direction[i].opacity++;
            cooccurrence[v][u].direction[i].opacity++;
          }
      }
      p++;
    }
  }
  grays=(LongPixelPacket *) RelinquishMagickMemory(grays);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    {
      for (i=0; i < (ssize_t) number_grays; i++)
        cooccurrence[i]=(ChannelStatistics *)
          RelinquishMagickMemory(cooccurrence[i]);
      cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(cooccurrence);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(channel_features);
    }
  /*
    Normalize spatial dependence matrix.
  */
  for (i=0; i < 4; i++)
  {
    double
      normalize;

    register ssize_t
      y;

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
    normalize=PerceptibleReciprocal(normalize);
    for (y=0; y < (ssize_t) number_grays; y++)
    {
      register ssize_t
        x;

      for (x=0; x < (ssize_t) number_grays; x++)
      {
        cooccurrence[x][y].direction[i].red*=normalize;
        cooccurrence[x][y].direction[i].green*=normalize;
        cooccurrence[x][y].direction[i].blue*=normalize;
        if (image->colorspace == CMYKColorspace)
          cooccurrence[x][y].direction[i].index*=normalize;
        if (image->matte != MagickFalse)
          cooccurrence[x][y].direction[i].opacity*=normalize;
      }
    }
  }
  /*
    Compute texture features.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,number_grays,1)
#endif
  for (i=0; i < 4; i++)
  {
    register ssize_t
      y;

    for (y=0; y < (ssize_t) number_grays; y++)
    {
      register ssize_t
        x;

      for (x=0; x < (ssize_t) number_grays; x++)
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
        if (image->colorspace == CMYKColorspace)
          channel_features[BlackChannel].angular_second_moment[i]+=
            cooccurrence[x][y].direction[i].index*
            cooccurrence[x][y].direction[i].index;
        if (image->matte != MagickFalse)
          channel_features[OpacityChannel].angular_second_moment[i]+=
            cooccurrence[x][y].direction[i].opacity*
            cooccurrence[x][y].direction[i].opacity;
        /*
          Correlation: measure of linear-dependencies in the image.
        */
        sum[y].direction[i].red+=cooccurrence[x][y].direction[i].red;
        sum[y].direction[i].green+=cooccurrence[x][y].direction[i].green;
        sum[y].direction[i].blue+=cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          sum[y].direction[i].index+=cooccurrence[x][y].direction[i].index;
        if (image->matte != MagickFalse)
          sum[y].direction[i].opacity+=cooccurrence[x][y].direction[i].opacity;
        correlation.direction[i].red+=x*y*cooccurrence[x][y].direction[i].red;
        correlation.direction[i].green+=x*y*
          cooccurrence[x][y].direction[i].green;
        correlation.direction[i].blue+=x*y*
          cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          correlation.direction[i].index+=x*y*
            cooccurrence[x][y].direction[i].index;
        if (image->matte != MagickFalse)
          correlation.direction[i].opacity+=x*y*
            cooccurrence[x][y].direction[i].opacity;
        /*
          Inverse Difference Moment.
        */
        channel_features[RedChannel].inverse_difference_moment[i]+=
          cooccurrence[x][y].direction[i].red/((y-x)*(y-x)+1);
        channel_features[GreenChannel].inverse_difference_moment[i]+=
          cooccurrence[x][y].direction[i].green/((y-x)*(y-x)+1);
        channel_features[BlueChannel].inverse_difference_moment[i]+=
          cooccurrence[x][y].direction[i].blue/((y-x)*(y-x)+1);
        if (image->colorspace == CMYKColorspace)
          channel_features[IndexChannel].inverse_difference_moment[i]+=
            cooccurrence[x][y].direction[i].index/((y-x)*(y-x)+1);
        if (image->matte != MagickFalse)
          channel_features[OpacityChannel].inverse_difference_moment[i]+=
            cooccurrence[x][y].direction[i].opacity/((y-x)*(y-x)+1);
        /*
          Sum average.
        */
        density_xy[y+x+2].direction[i].red+=
          cooccurrence[x][y].direction[i].red;
        density_xy[y+x+2].direction[i].green+=
          cooccurrence[x][y].direction[i].green;
        density_xy[y+x+2].direction[i].blue+=
          cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          density_xy[y+x+2].direction[i].index+=
            cooccurrence[x][y].direction[i].index;
        if (image->matte != MagickFalse)
          density_xy[y+x+2].direction[i].opacity+=
            cooccurrence[x][y].direction[i].opacity;
        /*
          Entropy.
        */
        channel_features[RedChannel].entropy[i]-=
          cooccurrence[x][y].direction[i].red*
          log10(cooccurrence[x][y].direction[i].red+MagickEpsilon);
        channel_features[GreenChannel].entropy[i]-=
          cooccurrence[x][y].direction[i].green*
          log10(cooccurrence[x][y].direction[i].green+MagickEpsilon);
        channel_features[BlueChannel].entropy[i]-=
          cooccurrence[x][y].direction[i].blue*
          log10(cooccurrence[x][y].direction[i].blue+MagickEpsilon);
        if (image->colorspace == CMYKColorspace)
          channel_features[IndexChannel].entropy[i]-=
            cooccurrence[x][y].direction[i].index*
            log10(cooccurrence[x][y].direction[i].index+MagickEpsilon);
        if (image->matte != MagickFalse)
          channel_features[OpacityChannel].entropy[i]-=
            cooccurrence[x][y].direction[i].opacity*
            log10(cooccurrence[x][y].direction[i].opacity+MagickEpsilon);
        /*
          Information Measures of Correlation.
        */
        density_x[x].direction[i].red+=cooccurrence[x][y].direction[i].red;
        density_x[x].direction[i].green+=cooccurrence[x][y].direction[i].green;
        density_x[x].direction[i].blue+=cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          density_x[x].direction[i].index+=
            cooccurrence[x][y].direction[i].index;
        if (image->matte != MagickFalse)
          density_x[x].direction[i].opacity+=
            cooccurrence[x][y].direction[i].opacity;
        density_y[y].direction[i].red+=cooccurrence[x][y].direction[i].red;
        density_y[y].direction[i].green+=cooccurrence[x][y].direction[i].green;
        density_y[y].direction[i].blue+=cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          density_y[y].direction[i].index+=
            cooccurrence[x][y].direction[i].index;
        if (image->matte != MagickFalse)
          density_y[y].direction[i].opacity+=
            cooccurrence[x][y].direction[i].opacity;
      }
      mean.direction[i].red+=y*sum[y].direction[i].red;
      sum_squares.direction[i].red+=y*y*sum[y].direction[i].red;
      mean.direction[i].green+=y*sum[y].direction[i].green;
      sum_squares.direction[i].green+=y*y*sum[y].direction[i].green;
      mean.direction[i].blue+=y*sum[y].direction[i].blue;
      sum_squares.direction[i].blue+=y*y*sum[y].direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        {
          mean.direction[i].index+=y*sum[y].direction[i].index;
          sum_squares.direction[i].index+=y*y*sum[y].direction[i].index;
        }
      if (image->matte != MagickFalse)
        {
          mean.direction[i].opacity+=y*sum[y].direction[i].opacity;
          sum_squares.direction[i].opacity+=y*y*sum[y].direction[i].opacity;
        }
    }
    /*
      Correlation: measure of linear-dependencies in the image.
    */
    channel_features[RedChannel].correlation[i]=
      (correlation.direction[i].red-mean.direction[i].red*
      mean.direction[i].red)/(sqrt(sum_squares.direction[i].red-
      (mean.direction[i].red*mean.direction[i].red))*sqrt(
      sum_squares.direction[i].red-(mean.direction[i].red*
      mean.direction[i].red)));
    channel_features[GreenChannel].correlation[i]=
      (correlation.direction[i].green-mean.direction[i].green*
      mean.direction[i].green)/(sqrt(sum_squares.direction[i].green-
      (mean.direction[i].green*mean.direction[i].green))*sqrt(
      sum_squares.direction[i].green-(mean.direction[i].green*
      mean.direction[i].green)));
    channel_features[BlueChannel].correlation[i]=
      (correlation.direction[i].blue-mean.direction[i].blue*
      mean.direction[i].blue)/(sqrt(sum_squares.direction[i].blue-
      (mean.direction[i].blue*mean.direction[i].blue))*sqrt(
      sum_squares.direction[i].blue-(mean.direction[i].blue*
      mean.direction[i].blue)));
    if (image->colorspace == CMYKColorspace)
      channel_features[IndexChannel].correlation[i]=
        (correlation.direction[i].index-mean.direction[i].index*
        mean.direction[i].index)/(sqrt(sum_squares.direction[i].index-
        (mean.direction[i].index*mean.direction[i].index))*sqrt(
        sum_squares.direction[i].index-(mean.direction[i].index*
        mean.direction[i].index)));
    if (image->matte != MagickFalse)
      channel_features[OpacityChannel].correlation[i]=
        (correlation.direction[i].opacity-mean.direction[i].opacity*
        mean.direction[i].opacity)/(sqrt(sum_squares.direction[i].opacity-
        (mean.direction[i].opacity*mean.direction[i].opacity))*sqrt(
        sum_squares.direction[i].opacity-(mean.direction[i].opacity*
        mean.direction[i].opacity)));
  }
  /*
    Compute more texture features.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,number_grays,1)
#endif
  for (i=0; i < 4; i++)
  {
    register ssize_t
      x;

    for (x=2; x < (ssize_t) (2*number_grays); x++)
    {
      /*
        Sum average.
      */
      channel_features[RedChannel].sum_average[i]+=
        x*density_xy[x].direction[i].red;
      channel_features[GreenChannel].sum_average[i]+=
        x*density_xy[x].direction[i].green;
      channel_features[BlueChannel].sum_average[i]+=
        x*density_xy[x].direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        channel_features[IndexChannel].sum_average[i]+=
          x*density_xy[x].direction[i].index;
      if (image->matte != MagickFalse)
        channel_features[OpacityChannel].sum_average[i]+=
          x*density_xy[x].direction[i].opacity;
      /*
        Sum entropy.
      */
      channel_features[RedChannel].sum_entropy[i]-=
        density_xy[x].direction[i].red*
        log10(density_xy[x].direction[i].red+MagickEpsilon);
      channel_features[GreenChannel].sum_entropy[i]-=
        density_xy[x].direction[i].green*
        log10(density_xy[x].direction[i].green+MagickEpsilon);
      channel_features[BlueChannel].sum_entropy[i]-=
        density_xy[x].direction[i].blue*
        log10(density_xy[x].direction[i].blue+MagickEpsilon);
      if (image->colorspace == CMYKColorspace)
        channel_features[IndexChannel].sum_entropy[i]-=
          density_xy[x].direction[i].index*
          log10(density_xy[x].direction[i].index+MagickEpsilon);
      if (image->matte != MagickFalse)
        channel_features[OpacityChannel].sum_entropy[i]-=
          density_xy[x].direction[i].opacity*
          log10(density_xy[x].direction[i].opacity+MagickEpsilon);
      /*
        Sum variance.
      */
      channel_features[RedChannel].sum_variance[i]+=
        (x-channel_features[RedChannel].sum_entropy[i])*
        (x-channel_features[RedChannel].sum_entropy[i])*
        density_xy[x].direction[i].red;
      channel_features[GreenChannel].sum_variance[i]+=
        (x-channel_features[GreenChannel].sum_entropy[i])*
        (x-channel_features[GreenChannel].sum_entropy[i])*
        density_xy[x].direction[i].green;
      channel_features[BlueChannel].sum_variance[i]+=
        (x-channel_features[BlueChannel].sum_entropy[i])*
        (x-channel_features[BlueChannel].sum_entropy[i])*
        density_xy[x].direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        channel_features[IndexChannel].sum_variance[i]+=
          (x-channel_features[IndexChannel].sum_entropy[i])*
          (x-channel_features[IndexChannel].sum_entropy[i])*
          density_xy[x].direction[i].index;
      if (image->matte != MagickFalse)
        channel_features[OpacityChannel].sum_variance[i]+=
          (x-channel_features[OpacityChannel].sum_entropy[i])*
          (x-channel_features[OpacityChannel].sum_entropy[i])*
          density_xy[x].direction[i].opacity;
    }
  }
  /*
    Compute more texture features.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,number_grays,1)
#endif
  for (i=0; i < 4; i++)
  {
    register ssize_t
      y;

    for (y=0; y < (ssize_t) number_grays; y++)
    {
      register ssize_t
        x;

      for (x=0; x < (ssize_t) number_grays; x++)
      {
        /*
          Sum of Squares: Variance
        */
        variance.direction[i].red+=(y-mean.direction[i].red+1)*
          (y-mean.direction[i].red+1)*cooccurrence[x][y].direction[i].red;
        variance.direction[i].green+=(y-mean.direction[i].green+1)*
          (y-mean.direction[i].green+1)*cooccurrence[x][y].direction[i].green;
        variance.direction[i].blue+=(y-mean.direction[i].blue+1)*
          (y-mean.direction[i].blue+1)*cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          variance.direction[i].index+=(y-mean.direction[i].index+1)*
            (y-mean.direction[i].index+1)*cooccurrence[x][y].direction[i].index;
        if (image->matte != MagickFalse)
          variance.direction[i].opacity+=(y-mean.direction[i].opacity+1)*
            (y-mean.direction[i].opacity+1)*
            cooccurrence[x][y].direction[i].opacity;
        /*
          Sum average / Difference Variance.
        */
        density_xy[MagickAbsoluteValue(y-x)].direction[i].red+=
          cooccurrence[x][y].direction[i].red;
        density_xy[MagickAbsoluteValue(y-x)].direction[i].green+=
          cooccurrence[x][y].direction[i].green;
        density_xy[MagickAbsoluteValue(y-x)].direction[i].blue+=
          cooccurrence[x][y].direction[i].blue;
        if (image->colorspace == CMYKColorspace)
          density_xy[MagickAbsoluteValue(y-x)].direction[i].index+=
            cooccurrence[x][y].direction[i].index;
        if (image->matte != MagickFalse)
          density_xy[MagickAbsoluteValue(y-x)].direction[i].opacity+=
            cooccurrence[x][y].direction[i].opacity;
        /*
          Information Measures of Correlation.
        */
        entropy_xy.direction[i].red-=cooccurrence[x][y].direction[i].red*
          log10(cooccurrence[x][y].direction[i].red+MagickEpsilon);
        entropy_xy.direction[i].green-=cooccurrence[x][y].direction[i].green*
          log10(cooccurrence[x][y].direction[i].green+MagickEpsilon);
        entropy_xy.direction[i].blue-=cooccurrence[x][y].direction[i].blue*
          log10(cooccurrence[x][y].direction[i].blue+MagickEpsilon);
        if (image->colorspace == CMYKColorspace)
          entropy_xy.direction[i].index-=cooccurrence[x][y].direction[i].index*
            log10(cooccurrence[x][y].direction[i].index+MagickEpsilon);
        if (image->matte != MagickFalse)
          entropy_xy.direction[i].opacity-=
            cooccurrence[x][y].direction[i].opacity*log10(
            cooccurrence[x][y].direction[i].opacity+MagickEpsilon);
        entropy_xy1.direction[i].red-=(cooccurrence[x][y].direction[i].red*
          log10(density_x[x].direction[i].red*density_y[y].direction[i].red+
          MagickEpsilon));
        entropy_xy1.direction[i].green-=(cooccurrence[x][y].direction[i].green*
          log10(density_x[x].direction[i].green*density_y[y].direction[i].green+
          MagickEpsilon));
        entropy_xy1.direction[i].blue-=(cooccurrence[x][y].direction[i].blue*
          log10(density_x[x].direction[i].blue*density_y[y].direction[i].blue+
          MagickEpsilon));
        if (image->colorspace == CMYKColorspace)
          entropy_xy1.direction[i].index-=(
            cooccurrence[x][y].direction[i].index*log10(
            density_x[x].direction[i].index*density_y[y].direction[i].index+
            MagickEpsilon));
        if (image->matte != MagickFalse)
          entropy_xy1.direction[i].opacity-=(
            cooccurrence[x][y].direction[i].opacity*log10(
            density_x[x].direction[i].opacity*density_y[y].direction[i].opacity+
            MagickEpsilon));
        entropy_xy2.direction[i].red-=(density_x[x].direction[i].red*
          density_y[y].direction[i].red*log10(density_x[x].direction[i].red*
          density_y[y].direction[i].red+MagickEpsilon));
        entropy_xy2.direction[i].green-=(density_x[x].direction[i].green*
          density_y[y].direction[i].green*log10(density_x[x].direction[i].green*
          density_y[y].direction[i].green+MagickEpsilon));
        entropy_xy2.direction[i].blue-=(density_x[x].direction[i].blue*
          density_y[y].direction[i].blue*log10(density_x[x].direction[i].blue*
          density_y[y].direction[i].blue+MagickEpsilon));
        if (image->colorspace == CMYKColorspace)
          entropy_xy2.direction[i].index-=(density_x[x].direction[i].index*
            density_y[y].direction[i].index*log10(
            density_x[x].direction[i].index*density_y[y].direction[i].index+
            MagickEpsilon));
        if (image->matte != MagickFalse)
          entropy_xy2.direction[i].opacity-=(density_x[x].direction[i].opacity*
            density_y[y].direction[i].opacity*log10(
            density_x[x].direction[i].opacity*density_y[y].direction[i].opacity+
            MagickEpsilon));
      }
    }
    channel_features[RedChannel].variance_sum_of_squares[i]=
      variance.direction[i].red;
    channel_features[GreenChannel].variance_sum_of_squares[i]=
      variance.direction[i].green;
    channel_features[BlueChannel].variance_sum_of_squares[i]=
      variance.direction[i].blue;
    if (image->colorspace == CMYKColorspace)
      channel_features[RedChannel].variance_sum_of_squares[i]=
        variance.direction[i].index;
    if (image->matte != MagickFalse)
      channel_features[RedChannel].variance_sum_of_squares[i]=
        variance.direction[i].opacity;
  }
  /*
    Compute more texture features.
  */
  (void) ResetMagickMemory(&variance,0,sizeof(variance));
  (void) ResetMagickMemory(&sum_squares,0,sizeof(sum_squares));
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,number_grays,1)
#endif
  for (i=0; i < 4; i++)
  {
    register ssize_t
      x;

    for (x=0; x < (ssize_t) number_grays; x++)
    {
      /*
        Difference variance.
      */
      variance.direction[i].red+=density_xy[x].direction[i].red;
      variance.direction[i].green+=density_xy[x].direction[i].green;
      variance.direction[i].blue+=density_xy[x].direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        variance.direction[i].index+=density_xy[x].direction[i].index;
      if (image->matte != MagickFalse)
        variance.direction[i].opacity+=density_xy[x].direction[i].opacity;
      sum_squares.direction[i].red+=density_xy[x].direction[i].red*
        density_xy[x].direction[i].red;
      sum_squares.direction[i].green+=density_xy[x].direction[i].green*
        density_xy[x].direction[i].green;
      sum_squares.direction[i].blue+=density_xy[x].direction[i].blue*
        density_xy[x].direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        sum_squares.direction[i].index+=density_xy[x].direction[i].index*
          density_xy[x].direction[i].index;
      if (image->matte != MagickFalse)
        sum_squares.direction[i].opacity+=density_xy[x].direction[i].opacity*
          density_xy[x].direction[i].opacity;
      /*
        Difference entropy.
      */
      channel_features[RedChannel].difference_entropy[i]-=
        density_xy[x].direction[i].red*
        log10(density_xy[x].direction[i].red+MagickEpsilon);
      channel_features[GreenChannel].difference_entropy[i]-=
        density_xy[x].direction[i].green*
        log10(density_xy[x].direction[i].green+MagickEpsilon);
      channel_features[BlueChannel].difference_entropy[i]-=
        density_xy[x].direction[i].blue*
        log10(density_xy[x].direction[i].blue+MagickEpsilon);
      if (image->colorspace == CMYKColorspace)
        channel_features[IndexChannel].difference_entropy[i]-=
          density_xy[x].direction[i].index*
          log10(density_xy[x].direction[i].index+MagickEpsilon);
      if (image->matte != MagickFalse)
        channel_features[OpacityChannel].difference_entropy[i]-=
          density_xy[x].direction[i].opacity*
          log10(density_xy[x].direction[i].opacity+MagickEpsilon);
      /*
        Information Measures of Correlation.
      */
      entropy_x.direction[i].red-=(density_x[x].direction[i].red*
        log10(density_x[x].direction[i].red+MagickEpsilon));
      entropy_x.direction[i].green-=(density_x[x].direction[i].green*
        log10(density_x[x].direction[i].green+MagickEpsilon));
      entropy_x.direction[i].blue-=(density_x[x].direction[i].blue*
        log10(density_x[x].direction[i].blue+MagickEpsilon));
      if (image->colorspace == CMYKColorspace)
        entropy_x.direction[i].index-=(density_x[x].direction[i].index*
          log10(density_x[x].direction[i].index+MagickEpsilon));
      if (image->matte != MagickFalse)
        entropy_x.direction[i].opacity-=(density_x[x].direction[i].opacity*
          log10(density_x[x].direction[i].opacity+MagickEpsilon));
      entropy_y.direction[i].red-=(density_y[x].direction[i].red*
        log10(density_y[x].direction[i].red+MagickEpsilon));
      entropy_y.direction[i].green-=(density_y[x].direction[i].green*
        log10(density_y[x].direction[i].green+MagickEpsilon));
      entropy_y.direction[i].blue-=(density_y[x].direction[i].blue*
        log10(density_y[x].direction[i].blue+MagickEpsilon));
      if (image->colorspace == CMYKColorspace)
        entropy_y.direction[i].index-=(density_y[x].direction[i].index*
          log10(density_y[x].direction[i].index+MagickEpsilon));
      if (image->matte != MagickFalse)
        entropy_y.direction[i].opacity-=(density_y[x].direction[i].opacity*
          log10(density_y[x].direction[i].opacity+MagickEpsilon));
    }
    /*
      Difference variance.
    */
    channel_features[RedChannel].difference_variance[i]=
      (((double) number_grays*number_grays*sum_squares.direction[i].red)-
      (variance.direction[i].red*variance.direction[i].red))/
      ((double) number_grays*number_grays*number_grays*number_grays);
    channel_features[GreenChannel].difference_variance[i]=
      (((double) number_grays*number_grays*sum_squares.direction[i].green)-
      (variance.direction[i].green*variance.direction[i].green))/
      ((double) number_grays*number_grays*number_grays*number_grays);
    channel_features[BlueChannel].difference_variance[i]=
      (((double) number_grays*number_grays*sum_squares.direction[i].blue)-
      (variance.direction[i].blue*variance.direction[i].blue))/
      ((double) number_grays*number_grays*number_grays*number_grays);
    if (image->matte != MagickFalse)
      channel_features[OpacityChannel].difference_variance[i]=
        (((double) number_grays*number_grays*sum_squares.direction[i].opacity)-
        (variance.direction[i].opacity*variance.direction[i].opacity))/
        ((double) number_grays*number_grays*number_grays*number_grays);
    if (image->colorspace == CMYKColorspace)
      channel_features[IndexChannel].difference_variance[i]=
        (((double) number_grays*number_grays*sum_squares.direction[i].index)-
        (variance.direction[i].index*variance.direction[i].index))/
        ((double) number_grays*number_grays*number_grays*number_grays);
    /*
      Information Measures of Correlation.
    */
    channel_features[RedChannel].measure_of_correlation_1[i]=
      (entropy_xy.direction[i].red-entropy_xy1.direction[i].red)/
      (entropy_x.direction[i].red > entropy_y.direction[i].red ?
       entropy_x.direction[i].red : entropy_y.direction[i].red);
    channel_features[GreenChannel].measure_of_correlation_1[i]=
      (entropy_xy.direction[i].green-entropy_xy1.direction[i].green)/
      (entropy_x.direction[i].green > entropy_y.direction[i].green ?
       entropy_x.direction[i].green : entropy_y.direction[i].green);
    channel_features[BlueChannel].measure_of_correlation_1[i]=
      (entropy_xy.direction[i].blue-entropy_xy1.direction[i].blue)/
      (entropy_x.direction[i].blue > entropy_y.direction[i].blue ?
       entropy_x.direction[i].blue : entropy_y.direction[i].blue);
    if (image->colorspace == CMYKColorspace)
      channel_features[IndexChannel].measure_of_correlation_1[i]=
        (entropy_xy.direction[i].index-entropy_xy1.direction[i].index)/
        (entropy_x.direction[i].index > entropy_y.direction[i].index ?
         entropy_x.direction[i].index : entropy_y.direction[i].index);
    if (image->matte != MagickFalse)
      channel_features[OpacityChannel].measure_of_correlation_1[i]=
        (entropy_xy.direction[i].opacity-entropy_xy1.direction[i].opacity)/
        (entropy_x.direction[i].opacity > entropy_y.direction[i].opacity ?
         entropy_x.direction[i].opacity : entropy_y.direction[i].opacity);
    channel_features[RedChannel].measure_of_correlation_2[i]=
      (sqrt(fabs(1.0-exp(-2.0*(entropy_xy2.direction[i].red-
      entropy_xy.direction[i].red)))));
    channel_features[GreenChannel].measure_of_correlation_2[i]=
      (sqrt(fabs(1.0-exp(-2.0*(entropy_xy2.direction[i].green-
      entropy_xy.direction[i].green)))));
    channel_features[BlueChannel].measure_of_correlation_2[i]=
      (sqrt(fabs(1.0-exp(-2.0*(entropy_xy2.direction[i].blue-
      entropy_xy.direction[i].blue)))));
    if (image->colorspace == CMYKColorspace)
      channel_features[IndexChannel].measure_of_correlation_2[i]=
        (sqrt(fabs(1.0-exp(-2.0*(entropy_xy2.direction[i].index-
        entropy_xy.direction[i].index)))));
    if (image->matte != MagickFalse)
      channel_features[OpacityChannel].measure_of_correlation_2[i]=
        (sqrt(fabs(1.0-exp(-2.0*(entropy_xy2.direction[i].opacity-
        entropy_xy.direction[i].opacity)))));
  }
  /*
    Compute more texture features.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_threads(image,image,number_grays,1)
#endif
  for (i=0; i < 4; i++)
  {
    register ssize_t
      z;

    for (z=0; z < (ssize_t) number_grays; z++)
    {
      register ssize_t
        y;

      ChannelStatistics
        pixel;

      (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
      for (y=0; y < (ssize_t) number_grays; y++)
      {
        register ssize_t
          x;

        for (x=0; x < (ssize_t) number_grays; x++)
        {
          /*
            Contrast:  amount of local variations present in an image.
          */
          if (((y-x) == z) || ((x-y) == z))
            {
              pixel.direction[i].red+=cooccurrence[x][y].direction[i].red;
              pixel.direction[i].green+=cooccurrence[x][y].direction[i].green;
              pixel.direction[i].blue+=cooccurrence[x][y].direction[i].blue;
              if (image->colorspace == CMYKColorspace)
                pixel.direction[i].index+=cooccurrence[x][y].direction[i].index;
              if (image->matte != MagickFalse)
                pixel.direction[i].opacity+=
                  cooccurrence[x][y].direction[i].opacity;
            }
          /*
            Maximum Correlation Coefficient.
          */
          Q[z][y].direction[i].red+=cooccurrence[z][x].direction[i].red*
            cooccurrence[y][x].direction[i].red/density_x[z].direction[i].red/
            density_y[x].direction[i].red;
          Q[z][y].direction[i].green+=cooccurrence[z][x].direction[i].green*
            cooccurrence[y][x].direction[i].green/
            density_x[z].direction[i].green/density_y[x].direction[i].red;
          Q[z][y].direction[i].blue+=cooccurrence[z][x].direction[i].blue*
            cooccurrence[y][x].direction[i].blue/density_x[z].direction[i].blue/
            density_y[x].direction[i].blue;
          if (image->colorspace == CMYKColorspace)
            Q[z][y].direction[i].index+=cooccurrence[z][x].direction[i].index*
              cooccurrence[y][x].direction[i].index/
              density_x[z].direction[i].index/density_y[x].direction[i].index;
          if (image->matte != MagickFalse)
            Q[z][y].direction[i].opacity+=
              cooccurrence[z][x].direction[i].opacity*
              cooccurrence[y][x].direction[i].opacity/
              density_x[z].direction[i].opacity/
              density_y[x].direction[i].opacity;
        }
      }
      channel_features[RedChannel].contrast[i]+=z*z*pixel.direction[i].red;
      channel_features[GreenChannel].contrast[i]+=z*z*pixel.direction[i].green;
      channel_features[BlueChannel].contrast[i]+=z*z*pixel.direction[i].blue;
      if (image->colorspace == CMYKColorspace)
        channel_features[BlackChannel].contrast[i]+=z*z*
          pixel.direction[i].index;
      if (image->matte != MagickFalse)
        channel_features[OpacityChannel].contrast[i]+=z*z*
          pixel.direction[i].opacity;
    }
    /*
      Maximum Correlation Coefficient.
      Future: return second largest eigenvalue of Q.
    */
    channel_features[RedChannel].maximum_correlation_coefficient[i]=
      sqrt((double) -1.0);
    channel_features[GreenChannel].maximum_correlation_coefficient[i]=
      sqrt((double) -1.0);
    channel_features[BlueChannel].maximum_correlation_coefficient[i]=
      sqrt((double) -1.0);
    if (image->colorspace == CMYKColorspace)
      channel_features[IndexChannel].maximum_correlation_coefficient[i]=
        sqrt((double) -1.0);
    if (image->matte != MagickFalse)
      channel_features[OpacityChannel].maximum_correlation_coefficient[i]=
        sqrt((double) -1.0);
  }
  /*
    Relinquish resources.
  */
  sum=(ChannelStatistics *) RelinquishMagickMemory(sum);
  for (i=0; i < (ssize_t) number_grays; i++)
    Q[i]=(ChannelStatistics *) RelinquishMagickMemory(Q[i]);
  Q=(ChannelStatistics **) RelinquishMagickMemory(Q);
  density_y=(ChannelStatistics *) RelinquishMagickMemory(density_y);
  density_xy=(ChannelStatistics *) RelinquishMagickMemory(density_xy);
  density_x=(ChannelStatistics *) RelinquishMagickMemory(density_x);
  for (i=0; i < (ssize_t) number_grays; i++)
    cooccurrence[i]=(ChannelStatistics *)
      RelinquishMagickMemory(cooccurrence[i]);
  cooccurrence=(ChannelStatistics **) RelinquishMagickMemory(cooccurrence);
  return(channel_features);
}

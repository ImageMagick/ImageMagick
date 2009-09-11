/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        SSSSS  TTTTT   AAA   TTTTT  IIIII  SSSSS  TTTTT  IIIII   CCCC        %
%        SS       T    A   A    T      I    SS       T      I    C            %
%         SSS     T    AAAAA    T      I     SSS     T      I    C            %
%           SS    T    A   A    T      I       SS    T      I    C            %
%        SSSSS    T    A   A    T    IIIII  SSSSS    T    IIIII   CCCC        %
%                                                                             %
%                                                                             %
%                          MagickCore Image Methods                           %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/image-private.h"
#include "magick/magic.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/option.h"
#include "magick/paint.h"
#include "magick/pixel-private.h"
#include "magick/profile.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/segment.h"
#include "magick/semaphore.h"
#include "magick/signature-private.h"
#include "magick/statistic.h"
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
+   G e t I m a g e B o u n d i n g B o x                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageBoundingBox() returns the bounding box of an image canvas.
%
%  The format of the GetImageBoundingBox method is:
%
%      RectangleInfo GetImageBoundingBox(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o bounds: Method GetImageBoundingBox returns the bounding box of an
%      image canvas.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport RectangleInfo GetImageBoundingBox(const Image *image,
  ExceptionInfo *exception)
{
  long
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    target[3],
    zero;

  RectangleInfo
    bounds;

  register const PixelPacket
    *p;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  bounds.width=0;
  bounds.height=0;
  bounds.x=(long) image->columns;
  bounds.y=(long) image->rows;
  GetMagickPixelPacket(image,&target[0]);
  image_view=AcquireCacheView(image);
  p=GetCacheViewVirtualPixels(image_view,0,0,1,1,exception);
  if (p == (const PixelPacket *) NULL)
    {
      image_view=DestroyCacheView(image_view);
      return(bounds);
    }
  SetMagickPixelPacket(image,p,GetCacheViewAuthenticIndexQueue(image_view),
    &target[0]);
  GetMagickPixelPacket(image,&target[1]);
  p=GetCacheViewVirtualPixels(image_view,(long) image->columns-1,0,1,1,
    exception);
  SetMagickPixelPacket(image,p,GetCacheViewAuthenticIndexQueue(image_view),
    &target[1]);
  GetMagickPixelPacket(image,&target[2]);
  p=GetCacheViewVirtualPixels(image_view,0,(long) image->rows-1,1,1,exception);
  SetMagickPixelPacket(image,p,GetCacheViewAuthenticIndexQueue(image_view),
    &target[2]);
  status=MagickTrue;
  GetMagickPixelPacket(image,&zero);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    RectangleInfo
      bounding_box;

    register const IndexPacket
      *__restrict indexes;

    register const PixelPacket
      *__restrict p;

    register long
      x;

    if (status == MagickFalse)
      continue;
#if defined(HAVE_OPENMP)
#  pragma omp critical (MagickCore_GetImageBoundingBox)
#endif
    bounding_box=bounds;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    pixel=zero;
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      if ((x < bounding_box.x) &&
          (IsMagickColorSimilar(&pixel,&target[0]) == MagickFalse))
        bounding_box.x=x;
      if ((x > (long) bounding_box.width) &&
          (IsMagickColorSimilar(&pixel,&target[1]) == MagickFalse))
        bounding_box.width=(unsigned long) x;
      if ((y < bounding_box.y) &&
          (IsMagickColorSimilar(&pixel,&target[0]) == MagickFalse))
        bounding_box.y=y;
      if ((y > (long) bounding_box.height) &&
          (IsMagickColorSimilar(&pixel,&target[2]) == MagickFalse))
        bounding_box.height=(unsigned long) y;
      p++;
    }
#if defined(HAVE_OPENMP)
#  pragma omp critical (MagickCore_GetImageBoundingBox)
#endif
    {
      if (bounding_box.x < bounds.x)
        bounds.x=bounding_box.x;
      if (bounding_box.y < bounds.y)
        bounds.y=bounding_box.y;
      if (bounding_box.width > bounds.width)
        bounds.width=bounding_box.width;
      if (bounding_box.height > bounds.height)
        bounds.height=bounding_box.height;
    }
  }
  image_view=DestroyCacheView(image_view);
  if ((bounds.width == 0) || (bounds.height == 0))
    (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
      "GeometryDoesNotContainImage","`%s'",image->filename);
  else
    {
      bounds.width-=(bounds.x-1);
      bounds.height-=(bounds.y-1);
    }
  return(bounds);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l D e p t h                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelDepth() returns the depth of a particular image channel.
%
%  The format of the GetImageChannelDepth method is:
%
%      unsigned long GetImageDepth(const Image *image,ExceptionInfo *exception)
%      unsigned long GetImageChannelDepth(const Image *image,
%        const ChannelType channel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport unsigned long GetImageDepth(const Image *image,
  ExceptionInfo *exception)
{
  return(GetImageChannelDepth(image,AllChannels,exception));
}

MagickExport unsigned long GetImageChannelDepth(const Image *image,
  const ChannelType channel,ExceptionInfo *exception)
{
  long
    y;

  MagickBooleanType
    status;

  register long
    id;

  unsigned long
    *current_depth,
    depth,
    number_threads;

  CacheView
    *image_view;

  /*
    Compute image depth.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  number_threads=GetOpenMPMaximumThreads();
  current_depth=(unsigned long *) AcquireQuantumMemory(number_threads,
    sizeof(*current_depth));
  if (current_depth == (unsigned long *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  status=MagickTrue;
  for (id=0; id < (long) number_threads; id++)
    current_depth[id]=1;
  if ((image->storage_class == PseudoClass) && (image->matte == MagickFalse))
    {
      register const PixelPacket
        *__restrict p;

      register long
        i;

      p=image->colormap;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(status)
#endif
      for (i=0; i < (long) image->colors; i++)
      {
        if (status == MagickFalse)
          continue;
        id=GetOpenMPThreadId();
        while (current_depth[id] < MAGICKCORE_QUANTUM_DEPTH)
        {
          MagickStatusType
            status;

          QuantumAny
            range;

          status=0;
          range=GetQuantumRange(current_depth[id]);
          if ((channel & RedChannel) != 0)
            status|=p->red != ScaleAnyToQuantum(ScaleQuantumToAny(p->red,
              range),range);
          if ((channel & GreenChannel) != 0)
            status|=p->green != ScaleAnyToQuantum(ScaleQuantumToAny(p->green,
              range),range);
          if ((channel & BlueChannel) != 0)
            status|=p->blue != ScaleAnyToQuantum(ScaleQuantumToAny(p->blue,
              range),range);
          if (status == 0)
            break;
          current_depth[id]++;
        }
        p++;
      }
      depth=current_depth[0];
      for (id=1; id < (long) number_threads; id++)
        if (depth < current_depth[id])
          depth=current_depth[id];
      current_depth=(unsigned long *) RelinquishMagickMemory(current_depth);
      return(depth);
    }
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *__restrict indexes;

    register const PixelPacket
      *__restrict p;

    register long
      id,
      x;

    if (status == MagickFalse)
      continue;
    id=GetOpenMPThreadId();
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      continue;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      while (current_depth[id] < MAGICKCORE_QUANTUM_DEPTH)
      {
        MagickStatusType
          status;

        QuantumAny
          range;

        status=0;
        range=GetQuantumRange(current_depth[id]);
        if ((channel & RedChannel) != 0)
          status|=p->red != ScaleAnyToQuantum(ScaleQuantumToAny(p->red,range),
            range);
        if ((channel & GreenChannel) != 0)
          status|=p->green != ScaleAnyToQuantum(ScaleQuantumToAny(p->green,
            range),range);
        if ((channel & BlueChannel) != 0)
          status|=p->blue != ScaleAnyToQuantum(ScaleQuantumToAny(p->blue,range),
            range);
        if (((channel & OpacityChannel) != 0) && (image->matte != MagickFalse))
          status|=p->opacity != ScaleAnyToQuantum(ScaleQuantumToAny(p->opacity,
            range),range);
        if (((channel & IndexChannel) != 0) &&
            (image->colorspace == CMYKColorspace))
          status|=indexes[x] != ScaleAnyToQuantum(ScaleQuantumToAny(indexes[x],
            range),range);
        if (status == 0)
          break;
        current_depth[id]++;
      }
      p++;
    }
    if (current_depth[id] == MAGICKCORE_QUANTUM_DEPTH)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  depth=current_depth[0];
  for (id=1; id < (long) number_threads; id++)
    if (depth < current_depth[id])
      depth=current_depth[id];
  current_depth=(unsigned long *) RelinquishMagickMemory(current_depth);
  return(depth);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e C h a n n e l E x t r e m a                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelExtrema() returns the extrema of one or more image channels.
%
%  The format of the GetImageChannelExtrema method is:
%
%      MagickBooleanType GetImageChannelExtrema(const Image *image,
%        const ChannelType channel,unsigned long *minima,unsigned long *maxima,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o minima: the minimum value in the channel.
%
%    o maxima: the maximum value in the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageExtrema(const Image *image,
  unsigned long *minima,unsigned long *maxima,ExceptionInfo *exception)
{
  return(GetImageChannelExtrema(image,AllChannels,minima,maxima,exception));
}

MagickExport MagickBooleanType GetImageChannelExtrema(const Image *image,
  const ChannelType channel,unsigned long *minima,unsigned long *maxima,
  ExceptionInfo *exception)
{
  double
    max,
    min;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=GetImageChannelRange(image,channel,&min,&max,exception);
  *minima=(unsigned long) (min+0.5);
  *maxima=(unsigned long) (max+0.5);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l M e a n                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelMean() returns the mean and standard deviation of one or more
%  image channels.
%
%  The format of the GetImageChannelMean method is:
%
%      MagickBooleanType GetImageChannelMean(const Image *image,
%        const ChannelType channel,double *mean,double *standard_deviation,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o mean: the average value in the channel.
%
%    o standard_deviation: the standard deviation of the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageMean(const Image *image,double *mean,
  double *standard_deviation,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetImageChannelMean(image,AllChannels,mean,standard_deviation,
    exception);
  return(status);
}

MagickExport MagickBooleanType GetImageChannelMean(const Image *image,
  const ChannelType channel,double *mean,double *standard_deviation,
  ExceptionInfo *exception)
{
  double
    area;

  long
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *mean=0.0;
  *standard_deviation=0.0;
  area=0.0;
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *__restrict indexes;

    register const PixelPacket
      *__restrict p;

    register long
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          *mean+=p->red;
          *standard_deviation+=(double) p->red*p->red;
          area++;
        }
      if ((channel & GreenChannel) != 0)
        {
          *mean+=p->green;
          *standard_deviation+=(double) p->green*p->green;
          area++;
        }
      if ((channel & BlueChannel) != 0)
        {
          *mean+=p->blue;
          *standard_deviation+=(double) p->blue*p->blue;
          area++;
        }
      if ((channel & OpacityChannel) != 0)
        {
          *mean+=p->opacity;
          *standard_deviation+=(double) p->opacity*p->opacity;
          area++;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          *mean+=indexes[x];
          *standard_deviation+=(double) indexes[x]*indexes[x];
          area++;
        }
      p++;
    }
  }
  if (y < (long) image->rows)
    return(MagickFalse);
  if (area != 0)
    {
      *mean/=area;
      *standard_deviation/=area;
    }
  *standard_deviation=sqrt(*standard_deviation-(*mean*(*mean)));
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l K u r t o s i s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelKurtosis() returns the kurtosis and skewness of one or more
%  image channels.
%
%  The format of the GetImageChannelKurtosis method is:
%
%      MagickBooleanType GetImageChannelKurtosis(const Image *image,
%        const ChannelType channel,double *kurtosis,double *skewness,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o kurtosis: the kurtosis of the channel.
%
%    o skewness: the skewness of the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageKurtosis(const Image *image,
  double *kurtosis,double *skewness,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetImageChannelKurtosis(image,AllChannels,kurtosis,skewness,
    exception);
  return(status);
}

MagickExport MagickBooleanType GetImageChannelKurtosis(const Image *image,
  const ChannelType channel,double *kurtosis,double *skewness,
  ExceptionInfo *exception)
{
  double
    area,
    mean,
    standard_deviation,
    sum_squares,
    sum_cubes,
    sum_fourth_power;

  long
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *kurtosis=0.0;
  *skewness=0.0;
  area=0.0;
  mean=0.0;
  standard_deviation=0.0;
  sum_squares=0.0;
  sum_cubes=0.0;
  sum_fourth_power=0.0;
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *__restrict indexes;

    register const PixelPacket
      *__restrict p;

    register long
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          mean+=p->red;
          sum_squares+=(double) p->red*p->red;
          sum_cubes+=(double) p->red*p->red*p->red;
          sum_fourth_power+=(double) p->red*p->red*p->red*p->red;
          area++;
        }
      if ((channel & GreenChannel) != 0)
        {
          mean+=p->green;
          sum_squares+=(double) p->green*p->green;
          sum_cubes+=(double) p->green*p->green*p->green;
          sum_fourth_power+=(double) p->green*p->green*p->green*p->green;
          area++;
        }
      if ((channel & BlueChannel) != 0)
        {
          mean+=p->blue;
          sum_squares+=(double) p->blue*p->blue;
          sum_cubes+=(double) p->blue*p->blue*p->blue;
          sum_fourth_power+=(double) p->blue*p->blue*p->blue*p->blue;
          area++;
        }
      if ((channel & OpacityChannel) != 0)
        {
          mean+=p->opacity;
          sum_squares+=(double) p->opacity*p->opacity;
          sum_cubes+=(double) p->opacity*p->opacity*p->opacity;
          sum_fourth_power+=(double) p->opacity*p->opacity*p->opacity*
            p->opacity;
          area++;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          mean+=indexes[x];
          sum_squares+=(double) indexes[x]*indexes[x];
          sum_cubes+=(double) indexes[x]*indexes[x]*indexes[x];
          sum_fourth_power+=(double) indexes[x]*indexes[x]*indexes[x]*
            indexes[x];
          area++;
        }
      p++;
    }
  }
  if (y < (long) image->rows)
    return(MagickFalse);
  if (area != 0.0)
    {
      mean/=area;
      sum_squares/=area;
      sum_cubes/=area;
      sum_fourth_power/=area;
    }
  standard_deviation=sqrt(sum_squares-(mean*mean));
  if (standard_deviation != 0.0)
    {
      *kurtosis=sum_fourth_power-4.0*mean*sum_cubes+6.0*mean*mean*sum_squares-
        3.0*mean*mean*mean*mean;
      *kurtosis/=standard_deviation*standard_deviation*standard_deviation*
        standard_deviation;
      *kurtosis-=3.0;
      *skewness=sum_cubes-3.0*mean*sum_squares+2.0*mean*mean*mean;
      *skewness/=standard_deviation*standard_deviation*standard_deviation;
    }
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l R a n g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelRange() returns the range of one or more image channels.
%
%  The format of the GetImageChannelRange method is:
%
%      MagickBooleanType GetImageChannelRange(const Image *image,
%        const ChannelType channel,double *minima,double *maxima,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o minima: the minimum value in the channel.
%
%    o maxima: the maximum value in the channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageRange(const Image *image,
  double *minima,double *maxima,ExceptionInfo *exception)
{
  return(GetImageChannelRange(image,AllChannels,minima,maxima,exception));
}

MagickExport MagickBooleanType GetImageChannelRange(const Image *image,
  const ChannelType channel,double *minima,double *maxima,
  ExceptionInfo *exception)
{
  long
    y;

  MagickPixelPacket
    pixel;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *maxima=(-1.0E-37);
  *minima=1.0E+37;
  GetMagickPixelPacket(image,&pixel);
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *__restrict indexes;

    register const PixelPacket
      *__restrict p;

    register long
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      if ((channel & RedChannel) != 0)
        {
          if (pixel.red < *minima)
            *minima=(double) pixel.red;
          if (pixel.red > *maxima)
            *maxima=(double) pixel.red;
        }
      if ((channel & GreenChannel) != 0)
        {
          if (pixel.green < *minima)
            *minima=(double) pixel.green;
          if (pixel.green > *maxima)
            *maxima=(double) pixel.green;
        }
      if ((channel & BlueChannel) != 0)
        {
          if (pixel.blue < *minima)
            *minima=(double) pixel.blue;
          if (pixel.blue > *maxima)
            *maxima=(double) pixel.blue;
        }
      if ((channel & OpacityChannel) != 0)
        {
          if (pixel.opacity < *minima)
            *minima=(double) pixel.opacity;
          if (pixel.opacity > *maxima)
            *maxima=(double) pixel.opacity;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          if ((double) indexes[x] < *minima)
            *minima=(double) indexes[x];
          if ((double) indexes[x] > *maxima)
            *maxima=(double) indexes[x];
        }
      p++;
    }
  }
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l S t a t i s t i c s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelStatistics() returns statistics for each channel in the
%  image.  The statistics include the channel depth, its minima, maxima, mean,
%  standard deviation, kurtosis and skewness.  You can access the red channel
%  mean, for example, like this:
%
%      channel_statistics=GetImageChannelStatistics(image,excepton);
%      red_mean=channel_statistics[RedChannel].mean;
%
%  Use MagickRelinquishMemory() to free the statistics buffer.
%
%  The format of the GetImageChannelStatistics method is:
%
%      ChannelStatistics *GetImageChannelStatistics(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
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

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport ChannelStatistics *GetImageChannelStatistics(const Image *image,
  ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  double
    area,
    sum_squares,
    sum_cubes;

  long
    y;

  MagickStatusType
    status;

  QuantumAny
    range;

  register long
    i;

  size_t
    length;

  unsigned long
    channels,
    depth;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=AllChannels+1UL;
  channel_statistics=(ChannelStatistics *) AcquireQuantumMemory(length,
    sizeof(*channel_statistics));
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_statistics,0,length*
    sizeof(*channel_statistics));
  for (i=0; i <= AllChannels; i++)
  {
    channel_statistics[i].depth=1;
    channel_statistics[i].maxima=(-1.0E-37);
    channel_statistics[i].minima=1.0E+37;
    channel_statistics[i].mean=0.0;
    channel_statistics[i].standard_deviation=0.0;
    channel_statistics[i].kurtosis=0.0;
    channel_statistics[i].skewness=0.0;
  }
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *__restrict indexes;

    register const PixelPacket
      *__restrict p;

    register long
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    for (x=0; x < (long) image->columns; )
    {
      if (channel_statistics[RedChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[RedChannel].depth;
          range=GetQuantumRange(depth);
          status=p->red != ScaleAnyToQuantum(ScaleQuantumToAny(p->red,range),
            range) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[RedChannel].depth++;
              continue;
            }
        }
      if (channel_statistics[GreenChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[GreenChannel].depth;
          range=GetQuantumRange(depth);
          status=p->green != ScaleAnyToQuantum(ScaleQuantumToAny(p->green,
            range),range) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[GreenChannel].depth++;
              continue;
            }
        }
      if (channel_statistics[BlueChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[BlueChannel].depth;
          range=GetQuantumRange(depth);
          status=p->blue != ScaleAnyToQuantum(ScaleQuantumToAny(p->blue,
            range),range) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[BlueChannel].depth++;
              continue;
            }
        }
      if (image->matte != MagickFalse)
        {
          if (channel_statistics[OpacityChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
            {
              depth=channel_statistics[OpacityChannel].depth;
              range=GetQuantumRange(depth);
              status=p->opacity != ScaleAnyToQuantum(ScaleQuantumToAny(
                p->opacity,range),range) ? MagickTrue : MagickFalse;
              if (status != MagickFalse)
                {
                  channel_statistics[OpacityChannel].depth++;
                  continue;
                }
            }
          }
      if (image->colorspace == CMYKColorspace)
        {
          if (channel_statistics[BlackChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
            {
              depth=channel_statistics[BlackChannel].depth;
              range=GetQuantumRange(depth);
              status=indexes[x] != ScaleAnyToQuantum(ScaleQuantumToAny(
                indexes[x],range),range) ? MagickTrue : MagickFalse;
              if (status != MagickFalse)
                {
                  channel_statistics[BlackChannel].depth++;
                  continue;
                }
            }
        }
      if ((double) p->red < channel_statistics[RedChannel].minima)
        channel_statistics[RedChannel].minima=(double) p->red;
      if ((double) p->red > channel_statistics[RedChannel].maxima)
        channel_statistics[RedChannel].maxima=(double) p->red;
      channel_statistics[RedChannel].mean+=p->red;
      channel_statistics[RedChannel].standard_deviation+=(double) p->red*p->red;
      channel_statistics[RedChannel].kurtosis+=(double) p->red*p->red*
        p->red*p->red;
      channel_statistics[RedChannel].skewness+=(double) p->red*p->red*p->red;
      if ((double) p->green < channel_statistics[GreenChannel].minima)
        channel_statistics[GreenChannel].minima=(double) p->green;
      if ((double) p->green > channel_statistics[GreenChannel].maxima)
        channel_statistics[GreenChannel].maxima=(double) p->green;
      channel_statistics[GreenChannel].mean+=p->green;
      channel_statistics[GreenChannel].standard_deviation+=(double) p->green*
        p->green;
      channel_statistics[GreenChannel].kurtosis+=(double) p->green*p->green*
        p->green*p->green;
      channel_statistics[GreenChannel].skewness+=(double) p->green*p->green*
        p->green;
      if ((double) p->blue < channel_statistics[BlueChannel].minima)
        channel_statistics[BlueChannel].minima=(double) p->blue;
      if ((double) p->blue > channel_statistics[BlueChannel].maxima)
        channel_statistics[BlueChannel].maxima=(double) p->blue;
      channel_statistics[BlueChannel].mean+=p->blue;
      channel_statistics[BlueChannel].standard_deviation+=(double) p->blue*
        p->blue;
      channel_statistics[BlueChannel].kurtosis+=(double) p->blue*p->blue*
        p->blue*p->blue;
      channel_statistics[BlueChannel].skewness+=(double) p->blue*p->blue*
        p->blue;
      if (image->matte != MagickFalse)
        {
          if ((double) p->opacity < channel_statistics[OpacityChannel].minima)
            channel_statistics[OpacityChannel].minima=(double) p->opacity;
          if ((double) p->opacity > channel_statistics[OpacityChannel].maxima)
            channel_statistics[OpacityChannel].maxima=(double) p->opacity;
          channel_statistics[OpacityChannel].mean+=p->opacity;
          channel_statistics[OpacityChannel].standard_deviation+=(double)
            p->opacity*p->opacity;
          channel_statistics[OpacityChannel].kurtosis+=(double) p->opacity*
            p->opacity*p->opacity*p->opacity;
          channel_statistics[OpacityChannel].skewness+=(double) p->opacity*
            p->opacity*p->opacity;
        }
      if (image->colorspace == CMYKColorspace)
        {
          if ((double) indexes[x] < channel_statistics[BlackChannel].minima)
            channel_statistics[BlackChannel].minima=(double) indexes[x];
          if ((double) indexes[x] > channel_statistics[BlackChannel].maxima)
            channel_statistics[BlackChannel].maxima=(double) indexes[x];
          channel_statistics[BlackChannel].mean+=indexes[x];
          channel_statistics[BlackChannel].standard_deviation+=(double)
            indexes[x]*indexes[x];
          channel_statistics[BlackChannel].kurtosis+=(double) indexes[x]*
            indexes[x]*indexes[x]*indexes[x];
          channel_statistics[BlackChannel].skewness+=(double) indexes[x]*
            indexes[x]*indexes[x];
        }
      x++;
      p++;
    }
  }
  area=(double) image->columns*image->rows;
  for (i=0; i < AllChannels; i++)
  {
    channel_statistics[i].mean/=area;
    channel_statistics[i].standard_deviation/=area;
    channel_statistics[i].kurtosis/=area;
    channel_statistics[i].skewness/=area;
  }
  for (i=0; i < AllChannels; i++)
  {
    channel_statistics[AllChannels].depth=(unsigned long) MagickMax((double)
      channel_statistics[AllChannels].depth,(double)
      channel_statistics[i].depth);
    channel_statistics[AllChannels].minima=MagickMin(
      channel_statistics[AllChannels].minima,channel_statistics[i].minima);
    channel_statistics[AllChannels].maxima=MagickMax(
      channel_statistics[AllChannels].maxima,channel_statistics[i].maxima);
    channel_statistics[AllChannels].mean+=channel_statistics[i].mean;
    channel_statistics[AllChannels].standard_deviation+=
      channel_statistics[i].standard_deviation;
    channel_statistics[AllChannels].kurtosis+=channel_statistics[i].kurtosis;
    channel_statistics[AllChannels].skewness+=channel_statistics[i].skewness;
  }
  channels=4;
  if (image->colorspace == CMYKColorspace)
    channels++;
  channel_statistics[AllChannels].mean/=channels;
  channel_statistics[AllChannels].standard_deviation/=channels;
  channel_statistics[AllChannels].kurtosis/=channels;
  channel_statistics[AllChannels].skewness/=channels;
  for (i=0; i <= AllChannels; i++)
  {
    sum_squares=0.0;
    sum_squares=channel_statistics[i].standard_deviation;
    sum_cubes=0.0;
    sum_cubes=channel_statistics[i].skewness;
    channel_statistics[i].standard_deviation=sqrt(
      channel_statistics[i].standard_deviation-
       (channel_statistics[i].mean*channel_statistics[i].mean));
    if (channel_statistics[i].standard_deviation == 0.0)
      {
        channel_statistics[i].kurtosis=0.0;
        channel_statistics[i].skewness=0.0;
      }
    else
      {
        channel_statistics[i].skewness=(channel_statistics[i].skewness-
          3.0*channel_statistics[i].mean*sum_squares+
          2.0*channel_statistics[i].mean*channel_statistics[i].mean*
          channel_statistics[i].mean)/
          (channel_statistics[i].standard_deviation*
           channel_statistics[i].standard_deviation*
           channel_statistics[i].standard_deviation);
        channel_statistics[i].kurtosis=(channel_statistics[i].kurtosis-
          4.0*channel_statistics[i].mean*sum_cubes+
          6.0*channel_statistics[i].mean*channel_statistics[i].mean*sum_squares-
          3.0*channel_statistics[i].mean*channel_statistics[i].mean*
          1.0*channel_statistics[i].mean*channel_statistics[i].mean)/
          (channel_statistics[i].standard_deviation*
           channel_statistics[i].standard_deviation*
           channel_statistics[i].standard_deviation*
           channel_statistics[i].standard_deviation)-3.0;
      }
  }
  return(channel_statistics);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e Q u a n t u m D e p t h                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageQuantumDepth() returns the depth of the image rounded to a legal
%  quantum depth: 8, 16, or 32.
%
%  The format of the GetImageQuantumDepth method is:
%
%      unsigned long GetImageQuantumDepth(const Image *image,
%        const MagickBooleanType constrain)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o constrain: A value other than MagickFalse, constrains the depth to
%      a maximum of MAGICKCORE_QUANTUM_DEPTH.
%
*/
MagickExport unsigned long GetImageQuantumDepth(const Image *image,
  const MagickBooleanType constrain)
{
  unsigned long
    depth;

  depth=image->depth;
  if (depth <= 8)
    depth=8;
  else
    if (depth <= 16)
      depth=16;
    else
      if (depth <= 32)
        depth=32;
      else
        if (depth <= 64)
          depth=64;
  if (constrain != MagickFalse)
    depth=(unsigned long) MagickMin((double) depth,(double)
      MAGICKCORE_QUANTUM_DEPTH);
  return(depth);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e C h a n n e l D e p t h                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageChannelDepth() sets the depth of the image.
%
%  The format of the SetImageChannelDepth method is:
%
%      MagickBooleanType SetImageDepth(Image *image,const unsigned long depth)
%      MagickBooleanType SetImageChannelDepth(Image *image,
%        const ChannelType channel,const unsigned long depth)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o depth: the image depth.
%
*/

MagickExport MagickBooleanType SetImageDepth(Image *image,
  const unsigned long depth)
{
  return(SetImageChannelDepth(image,AllChannels,depth));
}

MagickExport MagickBooleanType SetImageChannelDepth(Image *image,
  const ChannelType channel,const unsigned long depth)
{
  ExceptionInfo
    *exception;

  long
    y;

  MagickBooleanType
    status;

  QuantumAny
    range;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (GetImageDepth(image,&image->exception) <= (unsigned long)
      MagickMin((double) depth,(double) MAGICKCORE_QUANTUM_DEPTH))
    {
      image->depth=depth;
      return(MagickTrue);
    }
  /*
    Scale pixels to desired depth.
  */
  status=MagickTrue;
  range=GetQuantumRange(depth);
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

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
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=ScaleAnyToQuantum(ScaleQuantumToAny(q->red,range),range);
      if ((channel & GreenChannel) != 0)
        q->green=ScaleAnyToQuantum(ScaleQuantumToAny(q->green,range),range);
      if ((channel & BlueChannel) != 0)
        q->blue=ScaleAnyToQuantum(ScaleQuantumToAny(q->blue,range),range);
      if (((channel & OpacityChannel) != 0) && (image->matte != MagickFalse))
        q->opacity=ScaleAnyToQuantum(ScaleQuantumToAny(q->opacity,range),range);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        indexes[x]=ScaleAnyToQuantum(ScaleQuantumToAny(indexes[x],range),range);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      {
        status=MagickFalse;
        continue;
      }
  }
  image_view=DestroyCacheView(image_view);
  if (image->storage_class == PseudoClass)
    {
      QuantumAny
        range;

      register long
        i;

      register PixelPacket
        *__restrict p;

      p=image->colormap;
      range=GetQuantumRange(depth);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(status)
#endif
      for (i=0; i < (long) image->colors; i++)
      {
        if ((channel & RedChannel) != 0)
          p->red=ScaleAnyToQuantum(ScaleQuantumToAny(p->red,range),range);
        if ((channel & GreenChannel) != 0)
          p->green=ScaleAnyToQuantum(ScaleQuantumToAny(p->green,range),range);
        if ((channel & BlueChannel) != 0)
          p->blue=ScaleAnyToQuantum(ScaleQuantumToAny(p->blue,range),range);
        if ((channel & OpacityChannel) != 0)
          p->opacity=ScaleAnyToQuantum(ScaleQuantumToAny(p->opacity,range),
            range);
        p++;
      }
    }
  image->depth=depth;
  return(status);
}

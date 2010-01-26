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
%  GetImageChannelFeatures() returns features for each channel in the image at
%  at 0, 45, 90, and 135 degrees for the specified distance.  The features
%  include the angular second momentum, contrast, correlation, sum of squares:
%  variance, inverse difference moment, sum average, sum varience, sum entropy,
%  entropy, difference variance, difference entropy, information measures of
%  correlation 1, information measures of correlation 2, and maximum
%  correlation coefficient.  You can access the red channel contrast, for
%  example, like this:
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
  typedef struct _SpatialDependenceMatrix
  {
    DoublePixelPacket
      tones[4];
  } SpatialDependenceMatrix;

  CacheView
    *image_view;

  ChannelFeatures
    *channel_features;

  LongPixelPacket
    tone,
    *tones;

  long
    y,
    z;

  MagickBooleanType
    status;

  register long
    i;

  size_t
    length;

  SpatialDependenceMatrix
    **pixels;

  unsigned long
    number_tones;

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
    Form tones.
  */
  tones=(LongPixelPacket *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*tones));
  if (tones == (LongPixelPacket *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  for (i=0; i <= (long) MaxMap; i++)
  {
    tones[i].red=(~0UL);
    tones[i].green=(~0UL);
    tones[i].blue=(~0UL);
    tones[i].opacity=(~0UL);
    tones[i].index=(~0UL);
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
      tones[ScaleQuantumToMap(p->red)].red=ScaleQuantumToMap(p->red);
      tones[ScaleQuantumToMap(p->green)].green=ScaleQuantumToMap(p->green);
      tones[ScaleQuantumToMap(p->blue)].blue=ScaleQuantumToMap(p->blue);
      if (image->matte != MagickFalse)
        tones[ScaleQuantumToMap(p->opacity)].opacity=
          ScaleQuantumToMap(p->opacity);
      if (image->colorspace == CMYKColorspace)
        tones[ScaleQuantumToMap(indexes[x])].index=
          ScaleQuantumToMap(indexes[x]);
      p++;
    }
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    {
      tones=(LongPixelPacket *) RelinquishMagickMemory(tones);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  (void) ResetMagickMemory(&tone,0,sizeof(tone));
  for (i=0; i <= (long) MaxMap; i++)
  {
    if (tones[i].red != ~0UL)
      tones[tone.red++].red=tones[i].red;
    if (tones[i].green != ~0UL)
      tones[tone.green++].green=tones[i].green;
    if (tones[i].blue != ~0UL)
      tones[tone.blue++].blue=tones[i].blue;
    if (image->matte != MagickFalse)
      if (tones[i].opacity != ~0UL)
        tones[tone.opacity++].opacity=tones[i].opacity;
    if (image->colorspace == CMYKColorspace)
      if (tones[i].index != ~0UL)
        tones[tone.index++].index=tones[i].index;
  }
  /*
    Allocate spatial dependence matrix.
  */
  number_tones=tone.red;
  if (tone.green > number_tones)
    number_tones=tone.green;
  if (tone.blue > number_tones)
    number_tones=tone.blue;
  if (image->matte != MagickFalse)
    if (tone.opacity > number_tones)
      number_tones=tone.opacity;
  if (image->colorspace == CMYKColorspace)
    if (tone.index > number_tones)
      number_tones=tone.index;
  pixels=(SpatialDependenceMatrix **) AcquireQuantumMemory(number_tones,
    sizeof(*pixels));
  if (pixels == (SpatialDependenceMatrix **) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      tones=(LongPixelPacket *) RelinquishMagickMemory(tones);
      channel_features=(ChannelFeatures *) RelinquishMagickMemory(
        channel_features);
      return(channel_features);
    }
  for (i=0; i < (long) number_tones; i++)
  {
    pixels[i]=(SpatialDependenceMatrix *) AcquireQuantumMemory(number_tones,
      sizeof(**pixels));
    if (pixels[i] == (SpatialDependenceMatrix *) NULL)
      break;
    (void) ResetMagickMemory(pixels[i],0,number_tones*sizeof(*pixels));
  }
  if (i < (long) number_tones)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      for (i--; i >= 0; i--)
        pixels[i]=(SpatialDependenceMatrix *) RelinquishMagickMemory(pixels[i]);
      pixels=(SpatialDependenceMatrix **) RelinquishMagickMemory(pixels);
      tones=(LongPixelPacket *) RelinquishMagickMemory(tones);
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
              0 degrees.
            */
            offset=(ssize_t) distance;
            break;
          }
          case 1:
          {
            /*
              45 degrees.
            */
            offset=(ssize_t) (image->columns+2*distance)-distance;
            break;
          }
          case 2:
          {
            /*
              90 degrees.
            */
            offset=(ssize_t) (image->columns+2*distance);
            break;
          }
          case 3:
          {
            /*
              135 degrees.
            */
            offset=(ssize_t) (image->columns+2*distance)+distance;
            break;
          }
        }
        u=0;
        v=0;
        while (tones[u].red != ScaleQuantumToMap(p->red))
          u++;
        while (tones[v].red != ScaleQuantumToMap((p+offset)->red))
          v++;
        pixels[u][v].tones[i].red++;
        pixels[v][u].tones[i].red++;
        u=0;
        v=0;
        while (tones[u].green != ScaleQuantumToMap(p->green))
          u++;
        while (tones[v].green != ScaleQuantumToMap((p+offset)->green))
          v++;
        pixels[u][v].tones[i].green++;
        pixels[v][u].tones[i].green++;
        u=0;
        v=0;
        while (tones[u].blue != ScaleQuantumToMap(p->blue))
          u++;
        while (tones[v].blue != ScaleQuantumToMap((p+offset)->blue))
          v++;
        pixels[u][v].tones[i].blue++;
        pixels[v][u].tones[i].blue++;
        if (image->matte != MagickFalse)
          {
            u=0;
            v=0;
            while (tones[u].opacity != ScaleQuantumToMap(p->opacity))
              u++;
            while (tones[v].opacity != ScaleQuantumToMap((p+offset)->opacity))
              v++;
            pixels[u][v].tones[i].opacity++;
            pixels[v][u].tones[i].opacity++;
          }
        if (image->colorspace == CMYKColorspace)
          {
            u=0;
            v=0;
            while (tones[u].index != ScaleQuantumToMap(indexes[x]))
              u++;
            while (tones[v].index != ScaleQuantumToMap(indexes[x+offset]))
              v++;
            pixels[u][v].tones[i].index++;
            pixels[v][u].tones[i].index++;
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
      for (i=0; i < (long) number_tones; i++)
        pixels[i]=(SpatialDependenceMatrix *) RelinquishMagickMemory(pixels[i]);
      pixels=(SpatialDependenceMatrix **) RelinquishMagickMemory(pixels);
      tones=(LongPixelPacket *) RelinquishMagickMemory(tones);
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
  for (y=0; y < (long) number_tones; y++)
  {
    double
      normalize;

    register long
      x;

    for (x=0; x < (long) number_tones; x++)
    {
      for (i=0; i < 4; i++)
      {
        switch (i)
        {
          case 0:
          default:
          {
            /*
              0 degrees.
            */
            normalize=2.0*image->rows*(image->columns-distance);
            break;
          }
          case 1:
          {
            /*
              45 degrees.
            */
            normalize=2.0*(image->rows-distance)*(image->columns-distance);
            break;
          }
          case 2:
          {
            /*
              90 degrees.
            */
            normalize=2.0*(image->rows-distance)*image->columns;
            break;
          }
          case 3:
          {
            /*
              135 degrees.
            */
            normalize=2.0*(image->rows-distance)*(image->columns-distance);
            break;
          }
        }
        pixels[x][y].tones[i].red/=normalize;
        pixels[x][y].tones[i].green/=normalize;
        pixels[x][y].tones[i].blue/=normalize;
        if (image->matte != MagickFalse)
          pixels[x][y].tones[i].opacity/=normalize;
        if (image->colorspace == CMYKColorspace)
          pixels[x][y].tones[i].index/=normalize;
      }
    }
  }
  /*
    Compute texture features.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (y=0; y < (long) number_tones; y++)
  {
    register long
      x;

    for (x=0; x < (long) number_tones; x++)
    {
      for (i=0; i < 4; i++)
      {
        /*
          Angular second moment:  measure of homogeneity of the image.
        */
        channel_features[RedChannel].angular_second_moment[i]+=
          pixels[x][y].tones[i].red*pixels[x][y].tones[i].red;
        channel_features[GreenChannel].angular_second_moment[i]+=
          pixels[x][y].tones[i].green*pixels[x][y].tones[i].green;
        channel_features[BlueChannel].angular_second_moment[i]+=
          pixels[x][y].tones[i].blue*pixels[x][y].tones[i].blue;
        if (image->matte != MagickFalse)
          channel_features[OpacityChannel].angular_second_moment[i]+=
            pixels[x][y].tones[i].opacity*pixels[x][y].tones[i].opacity;
        if (image->colorspace == CMYKColorspace)
          channel_features[BlackChannel].angular_second_moment[i]+=
            pixels[x][y].tones[i].index*pixels[x][y].tones[i].index;
      }
    }
  }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (z=0; z < (long) number_tones; z++)
  {
    register long
      y;

    SpatialDependenceMatrix
      pixel;

    (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
    for (y=0; y < (long) number_tones; y++)
    {
      register long
        x;

      for (x=0; x < (long) number_tones; x++)
      {
        for (i=0; i < 4; i++)
        {
          /*
            Contrast:  amount of local variations present in an image.
          */
          if (((y-x) == z) || ((x-y) == z))
            {
              pixel.tones[i].red+=pixels[x][y].tones[i].red;
              pixel.tones[i].green+=pixels[x][y].tones[i].green;
              pixel.tones[i].blue+=pixels[x][y].tones[i].blue;
              if (image->matte != MagickFalse)
                pixel.tones[i].opacity+=pixels[x][y].tones[i].opacity;
              if (image->colorspace == CMYKColorspace)
                pixel.tones[i].index+=pixels[x][y].tones[i].index;
            }
        }
      }
    }
    for (i=0; i < 4; i++)
    {
      channel_features[RedChannel].contrast[i]+=z*z*pixel.tones[i].red;
      channel_features[GreenChannel].contrast[i]+=z*z*pixel.tones[i].green;
      channel_features[BlueChannel].contrast[i]+=z*z*pixel.tones[i].blue;
      if (image->matte != MagickFalse)
        channel_features[OpacityChannel].contrast[i]+=z*z*
          pixel.tones[i].opacity;
      if (image->colorspace == CMYKColorspace)
        channel_features[BlackChannel].contrast[i]+=z*z*pixel.tones[i].index;
    }
  }
  /*
    Relinquish resources.
  */
  for (i=0; i < (long) number_tones; i++)
    pixels[i]=(SpatialDependenceMatrix *) RelinquishMagickMemory(pixels[i]);
  pixels=(SpatialDependenceMatrix **) RelinquishMagickMemory(pixels);
  tones=(LongPixelPacket *) RelinquishMagickMemory(tones);
  return(channel_features);
}

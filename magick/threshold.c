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
%                                 John Cristy                                 %
%                                 October 1996                                %
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
#include "magick/blob.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/configure.h"
#include "magick/constitute.h"
#include "magick/decorate.h"
#include "magick/draw.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/effect.h"
#include "magick/fx.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/montage.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/quantize.h"
#include "magick/quantum.h"
#include "magick/random_.h"
#include "magick/random-private.h"
#include "magick/resize.h"
#include "magick/resource_.h"
#include "magick/segment.h"
#include "magick/shear.h"
#include "magick/signature-private.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/transform.h"
#include "magick/threshold.h"
#include "magick/xml-tree.h"

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

  unsigned long
    width,
    height;

  long
    divisor,
    *levels;
};

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
%      Image *AdaptiveThresholdImage(const Image *image,
%        const unsigned long width,const unsigned long height,
%        const long offset,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o width: the width of the local neighborhood.
%
%    o height: the height of the local neighborhood.
%
%    o offset: the mean offset.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *AdaptiveThresholdImage(const Image *image,
  const unsigned long width,const unsigned long height,const long offset,
  ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view,
    *threshold_view;

  Image
    *threshold_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  MagickRealType
    number_pixels;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((image->columns < width) || (image->rows < height))
    ThrowImageException(OptionError,"ImageSmallerThanRadius");
  threshold_image=CloneImage(image,0,0,MagickTrue,exception);
  if (threshold_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(threshold_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&threshold_image->exception);
      threshold_image=DestroyImage(threshold_image);
      return((Image *) NULL);
    }
  /*
    Local adaptive threshold.
  */
  status=MagickTrue;
  progress=0;
  GetMagickPixelPacket(image,&zero);
  number_pixels=(MagickRealType) width*height;
  image_view=AcquireCacheView(image);
  threshold_view=AcquireCacheView(threshold_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict threshold_indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((long) width/2L),y-height/2L,
      image->columns+width,height,exception);
    q=GetCacheViewAuthenticPixels(threshold_view,0,y,threshold_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    threshold_indexes=GetCacheViewAuthenticIndexQueue(threshold_view);
    for (x=0; x < (long) image->columns; x++)
    {
      long
        v;

      MagickPixelPacket
        mean,
        pixel;

      register const PixelPacket
        *r;

      register long
        u;

      pixel=zero;
      mean=zero;
      r=p;
      for (v=0; v < (long) height; v++)
      {
        for (u=0; u < (long) width; u++)
        {
          pixel.red+=r[u].red;
          pixel.green+=r[u].green;
          pixel.blue+=r[u].blue;
          pixel.opacity+=r[u].opacity;
          if (image->colorspace == CMYKColorspace)
            pixel.index=(MagickRealType) indexes[x+(r-p)+u];
        }
        r+=image->columns+width;
      }
      mean.red=(MagickRealType) (pixel.red/number_pixels+offset);
      mean.green=(MagickRealType) (pixel.green/number_pixels+offset);
      mean.blue=(MagickRealType) (pixel.blue/number_pixels+offset);
      mean.opacity=(MagickRealType) (pixel.opacity/number_pixels+offset);
      if (image->colorspace == CMYKColorspace)
        mean.index=(MagickRealType) (pixel.index/number_pixels+offset);
      q->red=(Quantum) (((MagickRealType) q->red <= mean.red) ?
        0 : QuantumRange);
      q->green=(Quantum) (((MagickRealType) q->green <= mean.green) ?
        0 : QuantumRange);
      q->blue=(Quantum) (((MagickRealType) q->blue <= mean.blue) ?
        0 : QuantumRange);
      q->opacity=(Quantum) (((MagickRealType) q->opacity <= mean.opacity) ?
        0 : QuantumRange);
      if (image->colorspace == CMYKColorspace)
        threshold_indexes[x]=(IndexPacket) (((MagickRealType)
          threshold_indexes[x] <= mean.index) ? 0 : QuantumRange);
      p++;
      q++;
    }
    sync=SyncCacheViewAuthenticPixels(threshold_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_AdaptiveThresholdImage)
#endif
        proceed=SetImageProgress(image,ThresholdImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
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
%  The format of the BilevelImageChannel method is:
%
%      MagickBooleanType BilevelImage(Image *image,const double threshold)
%      MagickBooleanType BilevelImageChannel(Image *image,
%        const ChannelType channel,const double threshold)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o threshold: define the threshold values.
%
%  Aside: You can get the same results as operator using LevelImageChannels()
%  with the 'threshold' value for both the black_point and the white_point.
%
*/

MagickExport MagickBooleanType BilevelImage(Image *image,const double threshold)
{
  MagickBooleanType
    status;

  status=BilevelImageChannel(image,DefaultChannels,threshold);
  return(status);
}

MagickExport MagickBooleanType BilevelImageChannel(Image *image,
  const ChannelType channel,const double threshold)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  /*
    Bilevel threshold image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    if (channel == DefaultChannels)
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=(Quantum) ((MagickRealType) PixelIntensityToQuantum(q) <=
            threshold ? 0 : QuantumRange);
          q->green=q->red;
          q->blue=q->red;
          q++;
        }
      }
    else
      for (x=0; x < (long) image->columns; x++)
      {
        if ((channel & RedChannel) != 0)
          q->red=(Quantum) ((MagickRealType) q->red <= threshold ? 0 :
            QuantumRange);
        if ((channel & GreenChannel) != 0)
          q->green=(Quantum) ((MagickRealType) q->green <= threshold ? 0 :
            QuantumRange);
        if ((channel & BlueChannel) != 0)
          q->blue=(Quantum) ((MagickRealType) q->blue <= threshold ? 0 :
            QuantumRange);
        if ((channel & OpacityChannel) != 0)
          {
            if (image->matte == MagickFalse)
              q->opacity=(Quantum) ((MagickRealType) q->opacity <= threshold ?
                0 : QuantumRange);
            else
              q->opacity=(Quantum) ((MagickRealType) q->opacity <= threshold ?
                OpaqueOpacity : TransparentOpacity);
          }
        if (((channel & IndexChannel) != 0) &&
            (image->colorspace == CMYKColorspace))
          indexes[x]=(IndexPacket) ((MagickRealType) indexes[x] <= threshold ?
            0 : QuantumRange);
        q++;
      }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_BilevelImageChannel)
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
%      MagickBooleanType BlackThresholdImage(Image *image,const char *threshold)
%      MagickBooleanType BlackThresholdImageChannel(Image *image,
%        const ChannelType channel,const char *threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
%
%    o threshold: Define the threshold value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType BlackThresholdImage(Image *image,
  const char *threshold)
{
  MagickBooleanType
    status;

  status=BlackThresholdImageChannel(image,DefaultChannels,threshold,
    &image->exception);
  return(status);
}

MagickExport MagickBooleanType BlackThresholdImageChannel(Image *image,
  const ChannelType channel,const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    threshold;

  MagickStatusType
    flags;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  GetMagickPixelPacket(image,&threshold);
  flags=ParseGeometry(thresholds,&geometry_info);
  threshold.red=geometry_info.rho;
  threshold.green=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    threshold.green=threshold.red;
  threshold.blue=geometry_info.xi;
  if ((flags & XiValue) == 0)
    threshold.blue=threshold.red;
  threshold.opacity=geometry_info.psi;
  if ((flags & PsiValue) == 0)
    threshold.opacity=threshold.red;
  threshold.index=geometry_info.chi;
  if ((flags & ChiValue) == 0)
    threshold.index=threshold.red;
  if ((flags & PercentValue) != 0)
    {
      threshold.red*=(QuantumRange/100.0);
      threshold.green*=(QuantumRange/100.0);
      threshold.blue*=(QuantumRange/100.0);
      threshold.opacity*=(QuantumRange/100.0);
      threshold.index*=(QuantumRange/100.0);
    }
  /*
    Black threshold image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if (channel != DefaultChannels)
        {
          if (PixelIntensity(q) < MagickPixelIntensity(&threshold))
            {
              q->red=(Quantum) 0;
              q->green=(Quantum) 0;
              q->blue=(Quantum) 0;
              if (image->colorspace == CMYKColorspace)
                indexes[x]=(Quantum) 0;
            }
        }
      else
        {
          if (((channel & RedChannel) != 0) &&
              ((MagickRealType) q->red < threshold.red))
            q->red=(Quantum) 0;
          if (((channel & GreenChannel) != 0) &&
              ((MagickRealType) q->green < threshold.green))
            q->green=(Quantum) 0;
          if (((channel & BlueChannel) != 0) &&
              ((MagickRealType) q->blue < threshold.blue))
            q->blue=(Quantum) 0;
          if (((channel & OpacityChannel) != 0) &&
              ((MagickRealType) q->opacity < threshold.opacity))
            q->opacity=(Quantum) 0;
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace) &&
              ((MagickRealType) indexes[x] < threshold.index))
            indexes[x]=(Quantum) 0;
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_BlackThresholdImageChannel)
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
%  ClampImage() restricts the color range from 0 to the quantum depth.
%
%  The format of the ClampImageChannel method is:
%
%      MagickBooleanType ClampImage(Image *image)
%      MagickBooleanType ClampImageChannel(Image *image,
%        const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
*/

static inline Quantum ClampToUnsignedQuantum(const Quantum quantum)
{
#if defined(MAGICKCORE_HDRI_SUPPORT)
  if (quantum <= 0)
    return(0);
  if (quantum >= QuantumRange)
    return(QuantumRange);
  return(quantum);
#else
  return(quantum);
#endif
}

MagickExport MagickBooleanType ClampImage(Image *image)
{
  MagickBooleanType
    status;

  status=ClampImageChannel(image,DefaultChannels);
  return(status);
}

MagickExport MagickBooleanType ClampImageChannel(Image *image,
  const ChannelType channel)
{
#define ClampImageTag  "Clamp/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    {
      register long
        i;

      register PixelPacket
        *restrict q;

      q=image->colormap;
      for (i=0; i < (long) image->colors; i++)
      {
        q->red=ClampToUnsignedQuantum(q->red);
        q->green=ClampToUnsignedQuantum(q->green);
        q->blue=ClampToUnsignedQuantum(q->blue);
        q->opacity=ClampToUnsignedQuantum(q->opacity);
        q++;
      }
      return(SyncImage(image));
    }
  /*
    Clamp image.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=ClampToUnsignedQuantum(q->red);
      if ((channel & GreenChannel) != 0)
        q->green=ClampToUnsignedQuantum(q->green);
      if ((channel & BlueChannel) != 0)
        q->blue=ClampToUnsignedQuantum(q->blue);
      if ((channel & OpacityChannel) != 0)
        q->opacity=ClampToUnsignedQuantum(q->opacity);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        indexes[x]=(IndexPacket) ClampToUnsignedQuantum(indexes[x]);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ClampImageChannel)
#endif
        proceed=SetImageProgress(image,ClampImageTag,progress++,
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
  if (map->levels != (long *) NULL)
    map->levels=(long *) RelinquishMagickMemory(map->levels);
  map=(ThresholdMap *) RelinquishMagickMemory(map);
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
MagickExport ThresholdMap *GetThresholdMapFile(const char *xml,
  const char *filename,const char *map_id,ExceptionInfo *exception)
{
  const char
    *attr,
    *content;

  double
    value;

 ThresholdMap
     *map;

  XMLTreeInfo
     *description,
     *levels,
     *threshold,
     *thresholds;

  map = (ThresholdMap *)NULL;
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading threshold map file \"%s\" ...",filename);
  thresholds=NewXMLTree(xml,exception);
  if ( thresholds == (XMLTreeInfo *)NULL )
    return(map);

  for( threshold = GetXMLTreeChild(thresholds,"threshold");
       threshold != (XMLTreeInfo *)NULL;
       threshold = GetNextXMLTreeTag(threshold) ) {
    attr = GetXMLTreeAttribute(threshold, "map");
    if ( (attr != (char *)NULL) && (LocaleCompare(map_id,attr) == 0) )
      break;
    attr = GetXMLTreeAttribute(threshold, "alias");
    if ( (attr != (char *)NULL) && (LocaleCompare(map_id,attr) == 0) )
      break;
  }
  if ( threshold == (XMLTreeInfo *)NULL ) {
    return(map);
  }
  description = GetXMLTreeChild(threshold,"description");
  if ( description == (XMLTreeInfo *)NULL ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "XmlMissingElement", "<description>, map \"%s\"", map_id);
    thresholds = DestroyXMLTree(thresholds);
    return(map);
  }
  levels = GetXMLTreeChild(threshold,"levels");
  if ( levels == (XMLTreeInfo *)NULL ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "XmlMissingElement", "<levels>, map \"%s\"", map_id);
    thresholds = DestroyXMLTree(thresholds);
    return(map);
  }

  /* The map has been found -- Allocate a Threshold Map to return */
  map = (ThresholdMap *)AcquireAlignedMemory(1,sizeof(ThresholdMap));
  if ( map == (ThresholdMap *)NULL )
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireThresholdMap");
  map->map_id = (char *)NULL;
  map->description = (char *)NULL;
  map->levels = (long *) NULL;

  /* Assign Basic Attributes */
  attr = GetXMLTreeAttribute(threshold, "map");
  if ( attr != (char *)NULL )
    map->map_id = ConstantString(attr);

  content = GetXMLTreeContent(description);
  if ( content != (char *)NULL )
    map->description = ConstantString(content);

  attr = GetXMLTreeAttribute(levels, "width");
  if ( attr == (char *)NULL ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "XmlMissingAttribute", "<levels width>, map \"%s\"", map_id);
    thresholds = DestroyXMLTree(thresholds);
    map = DestroyThresholdMap(map);
    return(map);
  }
  map->width = StringToUnsignedLong(attr);
  if ( map->width == 0 ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
     "XmlInvalidAttribute", "<levels width>, map \"%s\"", map_id);
    thresholds = DestroyXMLTree(thresholds);
    map = DestroyThresholdMap(map);
    return(map);
  }

  attr = GetXMLTreeAttribute(levels, "height");
  if ( attr == (char *)NULL ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "XmlMissingAttribute", "<levels height>, map \"%s\"", map_id);
    thresholds = DestroyXMLTree(thresholds);
    map = DestroyThresholdMap(map);
    return(map);
  }
  map->height = StringToUnsignedLong(attr);
  if ( map->height == 0 ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "XmlInvalidAttribute", "<levels height>, map \"%s\"", map_id);
    thresholds = DestroyXMLTree(thresholds);
    map = DestroyThresholdMap(map);
    return(map);
  }

  attr = GetXMLTreeAttribute(levels, "divisor");
  if ( attr == (char *)NULL ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "XmlMissingAttribute", "<levels divisor>, map \"%s\"", map_id);
    thresholds = DestroyXMLTree(thresholds);
    map = DestroyThresholdMap(map);
    return(map);
  }
  map->divisor = StringToLong(attr);
  if ( map->divisor < 2 ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "XmlInvalidAttribute", "<levels divisor>, map \"%s\"", map_id);
    thresholds = DestroyXMLTree(thresholds);
    map = DestroyThresholdMap(map);
    return(map);
  }

  /* Allocate theshold levels array */
  content = GetXMLTreeContent(levels);
  if ( content == (char *)NULL ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "XmlMissingContent", "<levels>, map \"%s\"", map_id);
    thresholds = DestroyXMLTree(thresholds);
    map = DestroyThresholdMap(map);
    return(map);
  }
  map->levels=(long *) AcquireQuantumMemory((size_t) map->width,map->height*
    sizeof(*map->levels));
  if ( map->levels == (long *)NULL )
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireThresholdMap");
  { /* parse levels into integer array */
    int i;
    char *p;
    for( i=0; i< (long) (map->width*map->height); i++) {
      map->levels[i] = (int)strtol(content, &p, 10);
      if ( p == content ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlInvalidContent", "<level> too few values, map \"%s\"", map_id);
        thresholds = DestroyXMLTree(thresholds);
        map = DestroyThresholdMap(map);
        return(map);
      }
      if ( map->levels[i] < 0 || map->levels[i] > map->divisor ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlInvalidContent", "<level> %ld out of range, map \"%s\"",
          map->levels[i], map_id);
        thresholds = DestroyXMLTree(thresholds);
        map = DestroyThresholdMap(map);
        return(map);
      }
      content = p;
    }
    value=(double) strtol(content,&p,10);
    if (p != content)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlInvalidContent", "<level> too many values, map \"%s\"", map_id);
       thresholds=DestroyXMLTree(thresholds);
       map=DestroyThresholdMap(map);
       return(map);
     }
  }

  thresholds = DestroyXMLTree(thresholds);
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
%  GetThresholdMap() load and search one or more threshold map files for the
%  a map matching the given name or aliase.
%
%  The format of the GetThresholdMap method is:
%
%      ThresholdMap *GetThresholdMap(const char *map_id,
%         ExceptionInfo *exception)
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

  map=(ThresholdMap *)NULL;
  options=GetConfigureOptions(ThresholdsFilename,exception);
  while (( option=(const StringInfo *) GetNextValueInLinkedList(options) )
          != (const StringInfo *) NULL && map == (ThresholdMap *)NULL )
    map=GetThresholdMapFile((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),map_id,exception);
  options=DestroyConfigureOptions(options);
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
  XMLTreeInfo *thresholds,*threshold,*description;
  const char *map,*alias,*content;

  assert( xml != (char *)NULL );
  assert( file != (FILE *)NULL );

  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
    "Loading threshold map file \"%s\" ...",filename);
  thresholds=NewXMLTree(xml,exception);
  if ( thresholds == (XMLTreeInfo *)NULL )
    return(MagickFalse);

  (void) fprintf(file,"%-16s %-12s %s\n", "Map", "Alias", "Description");
  (void) fprintf(file,"----------------------------------------------------\n");

  for( threshold = GetXMLTreeChild(thresholds,"threshold");
       threshold != (XMLTreeInfo *)NULL;
       threshold = GetNextXMLTreeTag(threshold) )
  {
    map = GetXMLTreeAttribute(threshold, "map");
    if (map == (char *) NULL) {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingAttribute", "<map>");
      thresholds=DestroyXMLTree(thresholds);
      return(MagickFalse);
    }
    alias = GetXMLTreeAttribute(threshold, "alias");
    /* alias is optional, no if test needed */
    description=GetXMLTreeChild(threshold,"description");
    if ( description == (XMLTreeInfo *)NULL ) {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingElement", "<description>, map \"%s\"", map);
      thresholds=DestroyXMLTree(thresholds);
      return(MagickFalse);
    }
    content=GetXMLTreeContent(description);
    if ( content == (char *)NULL ) {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "XmlMissingContent", "<description>, map \"%s\"", map);
      thresholds=DestroyXMLTree(thresholds);
      return(MagickFalse);
    }
    (void) fprintf(file,"%-16s %-12s %s\n",map,alias ? alias : "", content);
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

  status=MagickFalse;
  if ( file == (FILE *)NULL )
    file = stdout;
  options=GetConfigureOptions(ThresholdsFilename,exception);

  (void) fprintf(file, "\n   Threshold Maps for Ordered Dither Operations\n");

  while ( ( option=(const StringInfo *) GetNextValueInLinkedList(options) )
          != (const StringInfo *) NULL)
  {
    (void) fprintf(file,"\nPATH: %s\n\n",GetStringInfoPath(option));
    status|=ListThresholdMapFile(file,(const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),exception);
  }
  options=DestroyConfigureOptions(options);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   O r d e r e d D i t h e r I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OrderedDitherImage() uses the ordered dithering technique of reducing color
%  images to monochrome using positional information to retain as much
%  information as possible.
%
%  WARNING: This function is deprecated, and is now just a call to
%  the more more powerful OrderedPosterizeImage(); function.
%
%  The format of the OrderedDitherImage method is:
%
%      MagickBooleanType OrderedDitherImage(Image *image)
%      MagickBooleanType OrderedDitherImageChannel(Image *image,
%        const ChannelType channel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType OrderedDitherImage(Image *image)
{
  MagickBooleanType
    status;

  status=OrderedDitherImageChannel(image,DefaultChannels,&image->exception);
  return(status);
}

MagickExport MagickBooleanType OrderedDitherImageChannel(Image *image,
 const ChannelType channel,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  /*
    Call the augumented function OrderedPosterizeImage()
  */
  status=OrderedPosterizeImageChannel(image,channel,"o8x8",exception);
  return(status);
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
%      MagickBooleanType OrderedPosterizeImageChannel(Image *image,
%        const ChannelType channel,const char *threshold_map,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
%
%    o threshold_map: A string containing the name of the threshold dither
%      map to use, followed by zero or more numbers representing the number
%      of color levels tho dither between.
%
%      Any level number less than 2 will be equivelent to 2, and means only
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
  MagickBooleanType
    status;

  status=OrderedPosterizeImageChannel(image,DefaultChannels,threshold_map,
    exception);
  return(status);
}

MagickExport MagickBooleanType OrderedPosterizeImageChannel(Image *image,
  const ChannelType channel,const char *threshold_map,ExceptionInfo *exception)
{
#define DitherImageTag  "Dither/Image"

  CacheView
    *image_view;

  long
    progress,
    y;

  LongPixelPacket
    levels;

  MagickBooleanType
    status;

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
  {
    char
      token[MaxTextExtent];

    register const char
      *p;

    p=(char *)threshold_map;
    while (((isspace((int) ((unsigned char) *p)) != 0) || (*p == ',')) &&
                    (*p != '\0'))
      p++;
    threshold_map=p;
    while (((isspace((int) ((unsigned char) *p)) == 0) && (*p != ',')) &&
                    (*p != '\0')) {
      if ((p-threshold_map) >= MaxTextExtent)
        break;
      token[p-threshold_map] = *p;
      p++;
    }
    token[p-threshold_map] = '\0';
    map = GetThresholdMap(token, exception);
    if ( map == (ThresholdMap *)NULL ) {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "InvalidArgument","%s : '%s'","ordered-dither",threshold_map);
      return(MagickFalse);
    }
  }
  /* Set channel levels from extra comma seperated arguments
     Default to 2, the single value given, or individual channel values
  */
#if 1
  { /* parse directly as a comma seperated list of integers */
    char *p;

    p = strchr((char *) threshold_map,',');
    if ( p != (char *)NULL && isdigit((int) ((unsigned char) *(++p))) )
      levels.index = (unsigned long) strtol(p, &p, 10);
    else
      levels.index = 2;

    levels.red     = ((channel & RedChannel  )   != 0) ? levels.index : 0;
    levels.green   = ((channel & GreenChannel)   != 0) ? levels.index : 0;
    levels.blue    = ((channel & BlueChannel)    != 0) ? levels.index : 0;
    levels.opacity = ((channel & OpacityChannel) != 0) ? levels.index : 0;
    levels.index   = ((channel & IndexChannel)   != 0
            && (image->colorspace == CMYKColorspace)) ? levels.index : 0;

    /* if more than a single number, each channel has a separate value */
    if ( p != (char *) NULL && *p == ',' ) {
      p=strchr((char *) threshold_map,',');
      p++;
      if ((channel & RedChannel) != 0)
        levels.red = (unsigned long) strtol(p, &p, 10),   (void)(*p == ',' && p++);
      if ((channel & GreenChannel) != 0)
        levels.green = (unsigned long) strtol(p, &p, 10), (void)(*p == ',' && p++);
      if ((channel & BlueChannel) != 0)
        levels.blue = (unsigned long) strtol(p, &p, 10),  (void)(*p == ',' && p++);
      if ((channel & IndexChannel) != 0 && image->colorspace == CMYKColorspace)
        levels.index=(unsigned long) strtol(p, &p, 10), (void)(*p == ',' && p++);
      if ((channel & OpacityChannel) != 0)
        levels.opacity = (unsigned long) strtol(p, &p, 10), (void)(*p == ',' && p++);
    }
  }
#else
  /* Parse level values as a geometry */
  /* This difficult!
   * How to map   GeometryInfo structure elements into
   * LongPixelPacket structure elements, but according to channel?
   * Note the channels list may skip elements!!!!
   * EG  -channel BA  -ordered-dither map,2,3
   * will need to map  g.rho -> l.blue, and g.sigma -> l.opacity
   * A simpler way is needed, probably converting geometry to a temporary
   * array, then using channel to advance the index into long pixel packet.
   */
#endif

#if 0
printf("DEBUG levels  r=%ld g=%ld b=%ld a=%ld i=%ld\n",
     levels.red, levels.green, levels.blue, levels.opacity, levels.index);
#endif

  { /* Do the posterized ordered dithering of the image */
    int
      d;

    /* d = number of psuedo-level divisions added between color levels */
    d = map->divisor-1;

    /* reduce levels to levels - 1 */
    levels.red     = levels.red     ? levels.red-1     : 0;
    levels.green   = levels.green   ? levels.green-1   : 0;
    levels.blue    = levels.blue    ? levels.blue-1    : 0;
    levels.opacity = levels.opacity ? levels.opacity-1 : 0;
    levels.index   = levels.index   ? levels.index-1   : 0;

    if (SetImageStorageClass(image,DirectClass) == MagickFalse)
      {
        InheritException(exception,&image->exception);
        return(MagickFalse);
      }
    status=MagickTrue;
    progress=0;
    image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
    for (y=0; y < (long) image->rows; y++)
    {
      register IndexPacket
        *restrict indexes;

      register long
        x;

      register PixelPacket
        *restrict q;

      if (status == MagickFalse)
        continue;
      q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      indexes=GetCacheViewAuthenticIndexQueue(image_view);
      for (x=0; x < (long) image->columns; x++)
      {
        register int
          threshold,
          t,
          l;

        /*
          Figure out the dither threshold for this pixel
          This must be a integer from 1 to map->divisor-1
        */
        threshold = map->levels[(x%map->width) +map->width*(y%map->height)];

        /* Dither each channel in the image as appropriate
          Notes on the integer Math...
              total number of divisions = (levels-1)*(divisor-1)+1)
              t1 = this colors psuedo_level =
                      q->red * total_divisions / (QuantumRange+1)
              l = posterization level       0..levels
              t = dither threshold level    0..divisor-1  NB: 0 only on last
              Each color_level is of size   QuantumRange / (levels-1)
              NB: All input levels and divisor are already had 1 subtracted
              Opacity is inverted so 'off' represents transparent.
        */
        if (levels.red) {
          t = (int) (QuantumScale*q->red*(levels.red*d+1));
          l = t/d;  t = t-l*d;
          q->red=(Quantum) ((l+(t >= threshold))*QuantumRange/levels.red);
        }
        if (levels.green) {
          t = (int) (QuantumScale*q->green*(levels.green*d+1));
          l = t/d;  t = t-l*d;
          q->green=(Quantum) ((l+(t >= threshold))*QuantumRange/levels.green);
        }
        if (levels.blue) {
          t = (int) (QuantumScale*q->blue*(levels.blue*d+1));
          l = t/d;  t = t-l*d;
          q->blue=(Quantum) ((l+(t >= threshold))*QuantumRange/levels.blue);
        }
        if (levels.opacity) {
          t = (int) ((1.0-QuantumScale*q->opacity)*(levels.opacity*d+1));
          l = t/d;  t = t-l*d;
          q->opacity=(Quantum) ((1.0-l-(t >= threshold))*QuantumRange/
            levels.opacity);
        }
        if (levels.index) {
          t = (int) (QuantumScale*indexes[x]*(levels.index*d+1));
          l = t/d;  t = t-l*d;
          indexes[x]=(IndexPacket) ((l+(t>=threshold))*QuantumRange/
            levels.index);
        }
        q++;
      }
      if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
        status=MagickFalse;
      if (image->progress_monitor != (MagickProgressMonitor) NULL)
        {
          MagickBooleanType
            proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_OrderedPosterizeImageChannel)
#endif
          proceed=SetImageProgress(image,DitherImageTag,progress++,image->rows);
          if (proceed == MagickFalse)
            status=MagickFalse;
        }
    }
    image_view=DestroyCacheView(image_view);
  }
  map=DestroyThresholdMap(map);
  return(MagickTrue);
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
%      MagickBooleanType RandomThresholdImageChannel(Image *image,
%        const char *thresholds,ExceptionInfo *exception)
%      MagickBooleanType RandomThresholdImageChannel(Image *image,
%        const ChannelType channel,const char *thresholds,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
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
  MagickBooleanType
    status;

  status=RandomThresholdImageChannel(image,DefaultChannels,thresholds,
    exception);
  return(status);
}

MagickExport MagickBooleanType RandomThresholdImageChannel(Image *image,
  const ChannelType channel,const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  MagickStatusType
    flags;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    threshold;

  MagickRealType
    min_threshold,
    max_threshold;

  RandomInfo
    **restrict random_info;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  GetMagickPixelPacket(image,&threshold);
  min_threshold=0.0;
  max_threshold=(MagickRealType) QuantumRange;
  flags=ParseGeometry(thresholds,&geometry_info);
  min_threshold=geometry_info.rho;
  max_threshold=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    max_threshold=min_threshold;
  if (strchr(thresholds,'%') != (char *) NULL)
    {
      max_threshold*=(MagickRealType) (0.01*QuantumRange);
      min_threshold*=(MagickRealType) (0.01*QuantumRange);
    }
  else
    if (((max_threshold == min_threshold) || (max_threshold == 1)) &&
        (min_threshold <= 8))
      {
        /*
          Backward Compatibility -- ordered-dither -- IM v 6.2.9-6.
        */
        status=OrderedPosterizeImageChannel(image,channel,thresholds,exception);
        return(status);
      }
  /*
    Random threshold image.
  */
  status=MagickTrue;
  progress=0;
  if (channel == AllChannels)
    {
      if (AcquireImageColormap(image,2) == MagickFalse)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      random_info=AcquireRandomInfoThreadSet();
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
      for (y=0; y < (long) image->rows; y++)
      {
        MagickBooleanType
          sync;

        register IndexPacket
          *restrict indexes;

        register long
          id,
          x;

        register PixelPacket
          *restrict q;

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
        id=GetOpenMPThreadId();
        for (x=0; x < (long) image->columns; x++)
        {
          IndexPacket
            index;

          MagickRealType
            intensity;

          intensity=(MagickRealType) PixelIntensityToQuantum(q);
          if (intensity < min_threshold)
            threshold.index=min_threshold;
          else if (intensity > max_threshold)
            threshold.index=max_threshold;
          else
            threshold.index=(MagickRealType)(QuantumRange*
              GetPseudoRandomValue(random_info[id]));
          index=(IndexPacket) (intensity <= threshold.index ? 0 : 1);
          indexes[x]=index;
          *q++=image->colormap[(long) index];
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_RandomThresholdImageChannel)
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
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&image->exception);
      return(MagickFalse);
    }
  random_info=AcquireRandomInfoThreadSet();
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register long
      id,
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    id=GetOpenMPThreadId();
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          if ((MagickRealType) q->red < min_threshold)
            threshold.red=min_threshold;
          else
            if ((MagickRealType) q->red > max_threshold)
              threshold.red=max_threshold;
            else
              threshold.red=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((channel & GreenChannel) != 0)
        {
          if ((MagickRealType) q->green < min_threshold)
            threshold.green=min_threshold;
          else
            if ((MagickRealType) q->green > max_threshold)
              threshold.green=max_threshold;
            else
              threshold.green=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((channel & BlueChannel) != 0)
        {
          if ((MagickRealType) q->blue < min_threshold)
            threshold.blue=min_threshold;
          else
            if ((MagickRealType) q->blue > max_threshold)
              threshold.blue=max_threshold;
            else
              threshold.blue=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((channel & OpacityChannel) != 0)
        {
          if ((MagickRealType) q->opacity < min_threshold)
            threshold.opacity=min_threshold;
          else
            if ((MagickRealType) q->opacity > max_threshold)
              threshold.opacity=max_threshold;
            else
              threshold.opacity=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          if ((MagickRealType) indexes[x] < min_threshold)
            threshold.index=min_threshold;
          else
            if ((MagickRealType) indexes[x] > max_threshold)
              threshold.index=max_threshold;
            else
              threshold.index=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((channel & RedChannel) != 0)
        q->red=(Quantum) ((MagickRealType) q->red <= threshold.red ? 0 :
          QuantumRange);
      if ((channel & GreenChannel) != 0)
        q->green=(Quantum) ((MagickRealType) q->green <= threshold.green ? 0 :
          QuantumRange);
      if ((channel & BlueChannel) != 0)
        q->blue=(Quantum) ((MagickRealType) q->blue <= threshold.blue ? 0 :
          QuantumRange);
      if ((channel & OpacityChannel) != 0)
        q->opacity=(Quantum) ((MagickRealType) q->opacity <= threshold.opacity ?
          0 : QuantumRange);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        indexes[x]=(IndexPacket) ((MagickRealType) indexes[x] <=
          threshold.index ? 0 : QuantumRange);
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_RandomThresholdImageChannel)
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
%      MagickBooleanType WhiteThresholdImage(Image *image,const char *threshold)
%      MagickBooleanType WhiteThresholdImageChannel(Image *image,
%        const ChannelType channel,const char *threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel or channels to be thresholded.
%
%    o threshold: Define the threshold value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType WhiteThresholdImage(Image *image,
  const char *threshold)
{
  MagickBooleanType
    status;

  status=WhiteThresholdImageChannel(image,DefaultChannels,threshold,
    &image->exception);
  return(status);
}

MagickExport MagickBooleanType WhiteThresholdImageChannel(Image *image,
  const ChannelType channel,const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  GeometryInfo
    geometry_info;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    threshold;

  MagickStatusType
    flags;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  flags=ParseGeometry(thresholds,&geometry_info);
  GetMagickPixelPacket(image,&threshold);
  threshold.red=geometry_info.rho;
  threshold.green=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    threshold.green=threshold.red;
  threshold.blue=geometry_info.xi;
  if ((flags & XiValue) == 0)
    threshold.blue=threshold.red;
  threshold.opacity=geometry_info.psi;
  if ((flags & PsiValue) == 0)
    threshold.opacity=threshold.red;
  threshold.index=geometry_info.chi;
  if ((flags & ChiValue) == 0)
    threshold.index=threshold.red;
  if ((flags & PercentValue) != 0)
    {
      threshold.red*=(QuantumRange/100.0);
      threshold.green*=(QuantumRange/100.0);
      threshold.blue*=(QuantumRange/100.0);
      threshold.opacity*=(QuantumRange/100.0);
      threshold.index*=(QuantumRange/100.0);
    }
  /*
    White threshold image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      if (channel != DefaultChannels)
        {
          if (PixelIntensity(q) > MagickPixelIntensity(&threshold))
            {
              q->red=(Quantum) QuantumRange;
              q->green=(Quantum) QuantumRange;
              q->blue=(Quantum) QuantumRange;
              if (image->colorspace == CMYKColorspace)
                indexes[x]=(Quantum) QuantumRange;
            }
        }
      else
        {
          if (((channel & RedChannel) != 0) &&
              ((MagickRealType) q->red > threshold.red))
            q->red=(Quantum) QuantumRange;
          if (((channel & GreenChannel) != 0) &&
              ((MagickRealType) q->green > threshold.green))
            q->green=(Quantum) QuantumRange;
          if (((channel & BlueChannel) != 0) &&
              ((MagickRealType) q->blue > threshold.blue))
            q->blue=(Quantum) QuantumRange;
          if (((channel & OpacityChannel) != 0) &&
              ((MagickRealType) q->opacity > threshold.opacity))
            q->opacity=(Quantum) QuantumRange;
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace) &&
              ((MagickRealType) indexes[x] > threshold.index))
            indexes[x]=(Quantum) QuantumRange;
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_WhiteThresholdImageChannel)
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

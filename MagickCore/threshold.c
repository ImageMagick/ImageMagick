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
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/configure.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/draw.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/effect.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/segment.h"
#include "MagickCore/shear.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/transform.h"
#include "MagickCore/xml-tree.h"

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

  size_t
    width,
    height;

  ssize_t
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
%        const size_t width,const size_t height,
%        const ssize_t offset,ExceptionInfo *exception)
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
  const size_t width,const size_t height,const ssize_t offset,
  ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view,
    *threshold_view;

  Image
    *threshold_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    zero;

  MagickRealType
    number_pixels;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
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
  GetPixelInfo(image,&zero);
  number_pixels=(MagickRealType) width*height;
  image_view=AcquireCacheView(image);
  threshold_view=AcquireCacheView(threshold_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const Quantum
      *restrict p;

    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y-(ssize_t)
      height/2L,image->columns+width,height,exception);
    q=GetCacheViewAuthenticPixels(threshold_view,0,y,threshold_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      PixelInfo
        mean,
        pixel;

      register const Quantum
        *r;

      register ssize_t
        u;

      ssize_t
        v;

      pixel=zero;
      mean=zero;
      r=p;
      for (v=0; v < (ssize_t) height; v++)
      {
        for (u=0; u < (ssize_t) width; u++)
        {
          pixel.red+=GetPixelAlpha(image,r+u*GetPixelComponents(image));
          pixel.green+=GetPixelGreen(image,r+u*GetPixelComponents(image));
          pixel.blue+=GetPixelBlue(image,r+u*GetPixelComponents(image));
          if (image->colorspace == CMYKColorspace)
            pixel.black+=GetPixelBlack(image,r+u*GetPixelComponents(image));
          pixel.alpha+=GetPixelAlpha(image,r+u*GetPixelComponents(image));
        }
        r+=(image->columns+width)*GetPixelComponents(image);
      }
      mean.red=(MagickRealType) (pixel.red/number_pixels+offset);
      mean.green=(MagickRealType) (pixel.green/number_pixels+offset);
      mean.blue=(MagickRealType) (pixel.blue/number_pixels+offset);
      mean.black=(MagickRealType) (pixel.black/number_pixels+offset);
      mean.alpha=(MagickRealType) (pixel.alpha/number_pixels+offset);
      SetPixelRed(threshold_image,(Quantum) (((MagickRealType)
        GetPixelRed(threshold_image,q) <= mean.red) ? 0 : QuantumRange),q);
      SetPixelGreen(threshold_image,(Quantum) (((MagickRealType)
        GetPixelGreen(threshold_image,q) <= mean.green) ? 0 : QuantumRange),q);
      SetPixelBlue(threshold_image,(Quantum) (((MagickRealType)
        GetPixelBlue(threshold_image,q) <= mean.blue) ? 0 : QuantumRange),q);
      if (image->colorspace == CMYKColorspace)
        SetPixelBlack(threshold_image,(Quantum) (((MagickRealType)
          GetPixelBlack(threshold_image,q) <= mean.black) ? 0 : QuantumRange),
          q);
      SetPixelAlpha(threshold_image,(Quantum) (((MagickRealType)
        GetPixelAlpha(threshold_image,q) <= mean.alpha) ? 0 : QuantumRange),q);
      p+=GetPixelComponents(image);
      q+=GetPixelComponents(threshold_image);
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
%  The format of the BilevelImage method is:
%
%      MagickBooleanType BilevelImage(Image *image,const double threshold)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: define the threshold values.
%
%  Aside: You can get the same results as operator using LevelImages()
%  with the 'threshold' value for both the black_point and the white_point.
%
*/
MagickExport MagickBooleanType BilevelImage(Image *image,
  const double threshold)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

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
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    if (image->sync != MagickFalse)
      {
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRed(image,(Quantum) ((MagickRealType)
            GetPixelIntensity(image,q) <= threshold ? 0 : QuantumRange),q);
          SetPixelGreen(image,GetPixelRed(image,q),q);
          SetPixelBlue(image,GetPixelRed(image,q),q);
          q+=GetPixelComponents(image);
        }
      }
    else
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
          SetPixelRed(image,(Quantum) ((MagickRealType)
            GetPixelRed(image,q) <= threshold ? 0 : QuantumRange),q);
        if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
          SetPixelGreen(image,(Quantum) ((MagickRealType)
            GetPixelGreen(image,q) <= threshold ? 0 : QuantumRange),q);
        if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
          SetPixelBlue(image,(Quantum) ((MagickRealType)
            GetPixelBlue(image,q) <= threshold ? 0 : QuantumRange),q);
        if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
            (image->colorspace == CMYKColorspace))
          SetPixelBlack(image,(Quantum) ((MagickRealType)
            GetPixelBlack(image,q) <= threshold ? 0 : QuantumRange),q);
        if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
          {
            if (image->matte == MagickFalse)
              SetPixelAlpha(image,(Quantum) ((MagickRealType)
                GetPixelAlpha(image,q) <= threshold ? 0 : QuantumRange),q);
            else
              SetPixelAlpha(image,(Quantum) ((MagickRealType)
                GetPixelAlpha(image,q) >= threshold ? OpaqueAlpha :
                TransparentAlpha),q);
          }
        q+=GetPixelComponents(image);
      }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_BilevelImage)
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
%      MagickBooleanType BlackThresholdImage(Image *image,
%        const char *threshold,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: Define the threshold value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType BlackThresholdImage(Image *image,
  const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    threshold;

  MagickStatusType
    flags;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  GetPixelInfo(image,&threshold);
  flags=ParseGeometry(thresholds,&geometry_info);
  threshold.red=geometry_info.rho;
  threshold.green=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    threshold.green=threshold.red;
  threshold.blue=geometry_info.xi;
  if ((flags & XiValue) == 0)
    threshold.blue=threshold.red;
  threshold.alpha=geometry_info.psi;
  if ((flags & PsiValue) == 0)
    threshold.alpha=threshold.red;
  threshold.black=geometry_info.chi;
  if ((flags & ChiValue) == 0)
    threshold.black=threshold.red;
  if ((flags & PercentValue) != 0)
    {
      threshold.red*=(QuantumRange/100.0);
      threshold.green*=(QuantumRange/100.0);
      threshold.blue*=(QuantumRange/100.0);
      threshold.alpha*=(QuantumRange/100.0);
      threshold.black*=(QuantumRange/100.0);
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
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (image->sync != MagickFalse)
        {
          if (GetPixelIntensity(image,q) < GetPixelInfoIntensity(&threshold))
            {
              SetPixelRed(image,0,q);
              SetPixelGreen(image,0,q);
              SetPixelBlue(image,0,q);
              if (image->colorspace == CMYKColorspace)
                SetPixelBlack(image,0,q);
            }
        }
      else
        {
          if (((GetPixelRedTraits(image) & ActivePixelTrait) != 0) &&
              ((MagickRealType) GetPixelRed(image,q) < threshold.red))
            SetPixelRed(image,0,q);
          if (((GetPixelGreenTraits(image) & ActivePixelTrait) != 0) &&
              ((MagickRealType) GetPixelGreen(image,q) < threshold.green))
            SetPixelGreen(image,0,q);
          if (((GetPixelBlueTraits(image) & ActivePixelTrait) != 0) &&
              ((MagickRealType) GetPixelBlue(image,q) < threshold.blue))
            SetPixelBlue(image,0,q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace) &&
              ((MagickRealType) GetPixelBlack(image,q) < threshold.black))
            SetPixelBlack(image,0,q);
          if (((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0) &&
              ((MagickRealType) GetPixelAlpha(image,q) < threshold.alpha))
            SetPixelAlpha(image,0,q);
        }
      q+=GetPixelComponents(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_BlackThresholdImage)
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
%  The format of the ClampImage method is:
%
%      MagickBooleanType ClampImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
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
#define ClampImageTag  "Clamp/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == PseudoClass)
    {
      register ssize_t
        i;

      register PixelPacket
        *restrict q;

      q=image->colormap;
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        q->red=ClampToUnsignedQuantum(q->red);
        q->green=ClampToUnsignedQuantum(q->green);
        q->blue=ClampToUnsignedQuantum(q->blue);
        q->alpha=ClampToUnsignedQuantum(q->alpha);
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
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        SetPixelRed(image,ClampToUnsignedQuantum(GetPixelRed(image,q)),q);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        SetPixelGreen(image,ClampToUnsignedQuantum(GetPixelGreen(image,q)),q);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        SetPixelBlue(image,ClampToUnsignedQuantum(GetPixelBlue(image,q)),q);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(image,ClampToUnsignedQuantum(GetPixelBlack(image,q)),q);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        SetPixelAlpha(image,ClampToUnsignedQuantum(GetPixelAlpha(image,q)),q);
      q+=GetPixelComponents(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ClampImage)
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
  if (map->levels != (ssize_t *) NULL)
    map->levels=(ssize_t *) RelinquishMagickMemory(map->levels);
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
  map = (ThresholdMap *)AcquireMagickMemory(sizeof(ThresholdMap));
  if ( map == (ThresholdMap *)NULL )
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireThresholdMap");
  map->map_id = (char *)NULL;
  map->description = (char *)NULL;
  map->levels = (ssize_t *) NULL;

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
  map->divisor = (ssize_t) StringToLong(attr);
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
  map->levels=(ssize_t *) AcquireQuantumMemory((size_t) map->width,map->height*
    sizeof(*map->levels));
  if ( map->levels == (ssize_t *)NULL )
    ThrowFatalException(ResourceLimitFatalError,"UnableToAcquireThresholdMap");
  { /* parse levels into integer array */
    ssize_t i;
    char *p;
    for( i=0; i< (ssize_t) (map->width*map->height); i++) {
      map->levels[i] = (ssize_t)strtol(content, &p, 10);
      if ( p == content ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlInvalidContent", "<level> too few values, map \"%s\"", map_id);
        thresholds = DestroyXMLTree(thresholds);
        map = DestroyThresholdMap(map);
        return(map);
      }
      if ( map->levels[i] < 0 || map->levels[i] > map->divisor ) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "XmlInvalidContent", "<level> %.20g out of range, map \"%s\"",
          (double) map->levels[i],map_id);
        thresholds = DestroyXMLTree(thresholds);
        map = DestroyThresholdMap(map);
        return(map);
      }
      content = p;
    }
    value=(double) strtol(content,&p,10);
    (void) value;
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

  (void) FormatLocaleFile(file,"%-16s %-12s %s\n","Map","Alias","Description");
  (void) FormatLocaleFile(file,
    "----------------------------------------------------\n");

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
    (void) FormatLocaleFile(file,"%-16s %-12s %s\n",map,alias ? alias : "",
      content);
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

  (void) FormatLocaleFile(file,
    "\n   Threshold Maps for Ordered Dither Operations\n");
  while ( ( option=(const StringInfo *) GetNextValueInLinkedList(options) )
          != (const StringInfo *) NULL)
  {
    (void) FormatLocaleFile(file,"\nPATH: %s\n\n",GetStringInfoPath(option));
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
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold_map: A string containing the name of the threshold dither
%      map to use, followed by zero or more numbers representing the number
%      of color levels tho dither between.
%
%      Any level number less than 2 will be equivalent to 2, and means only
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
#define DitherImageTag  "Dither/Image"

  CacheView
    *image_view;

  LongPixelPacket
    levels;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

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
      if ((p-threshold_map) >= (MaxTextExtent-1))
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
  /* Set channel levels from extra comma separated arguments
     Default to 2, the single value given, or individual channel values
  */
#if 1
  { /* parse directly as a comma separated list of integers */
    char *p;

    p = strchr((char *) threshold_map,',');
    levels.red=0;
    levels.green=0;
    levels.blue=0;
    levels.black=0;
    levels.alpha=0;
    if ( p != (char *)NULL && isdigit((int) ((unsigned char) *(++p))) )
      levels.black = (unsigned int) strtoul(p, &p, 10);
    else
      levels.black = 2;

    if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
      levels.red=levels.black;
    if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
      levels.green=levels.black;
    if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
      levels.blue=levels.black;
    if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
        (image->colorspace == CMYKColorspace))
      levels.black=levels.black;
    if (((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0) &&
        (image->matte != MagickFalse))
      levels.alpha=levels.black;

    /* if more than a single number, each channel has a separate value */
    if ( p != (char *) NULL && *p == ',' ) {
      p=strchr((char *) threshold_map,',');
      p++;
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        levels.red = (unsigned int) strtoul(p, &p, 10),   (void)(*p == ',' && p++);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        levels.green = (unsigned int) strtoul(p, &p, 10), (void)(*p == ',' && p++);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        levels.blue = (unsigned int) strtoul(p, &p, 10),  (void)(*p == ',' && p++);
      if ((GetPixelBlackTraits(image) & ActivePixelTrait) != 0 &&
          (image->colorspace == CMYKColorspace))
        levels.black=(unsigned int) strtoul(p, &p, 10), (void)(*p == ',' && p++);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        levels.alpha = (unsigned int) strtoul(p, &p, 10), (void)(*p == ',' && p++);
    }
  }
#else
  /* Parse level values as a geometry */
  /* This difficult!
   * How to map   GeometryInfo structure elements into
   * LongPixelPacket structure elements, but according to channel?
   * Note the channels list may skip elements!!!!
   * EG  -channel BA  -ordered-dither map,2,3
   * will need to map  g.rho -> l.blue, and g.sigma -> l.alpha
   * A simpler way is needed, probably converting geometry to a temporary
   * array, then using channel to advance the index into ssize_t pixel packet.
   */
#endif

#if 0
printf("DEBUG levels  r=%u g=%u b=%u a=%u i=%u\n",
     levels.red, levels.green, levels.blue, levels.alpha, levels.index);
#endif

  { /* Do the posterized ordered dithering of the image */
    ssize_t
      d;

    /* d = number of psuedo-level divisions added between color levels */
    d = map->divisor-1;

    /* reduce levels to levels - 1 */
    levels.red     = levels.red     ? levels.red-1     : 0;
    levels.green   = levels.green   ? levels.green-1   : 0;
    levels.blue    = levels.blue    ? levels.blue-1    : 0;
    levels.black   = levels.black   ? levels.black-1   : 0;
    levels.alpha = levels.alpha ? levels.alpha-1 : 0;

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
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      register ssize_t
        x;

      register Quantum
        *restrict q;

      if (status == MagickFalse)
        continue;
      q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
      if (q == (const Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        register ssize_t
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
        if (levels.red != 0) {
          t = (ssize_t) (QuantumScale*GetPixelRed(image,q)*(levels.red*d+1));
          l = t/d;  t = t-l*d;
          SetPixelRed(image,RoundToQuantum((MagickRealType)
            ((l+(t >= threshold))*(MagickRealType) QuantumRange/levels.red)),q);
        }
        if (levels.green != 0) {
          t = (ssize_t) (QuantumScale*GetPixelGreen(image,q)*
            (levels.green*d+1));
          l = t/d;  t = t-l*d;
          SetPixelGreen(image,RoundToQuantum((MagickRealType)
            ((l+(t >= threshold))*(MagickRealType) QuantumRange/levels.green)),q);
        }
        if (levels.blue != 0) {
          t = (ssize_t) (QuantumScale*GetPixelBlue(image,q)*
            (levels.blue*d+1));
          l = t/d;  t = t-l*d;
          SetPixelBlue(image,RoundToQuantum((MagickRealType)
            ((l+(t >= threshold))*(MagickRealType) QuantumRange/levels.blue)),q);
        }
        if (levels.alpha != 0) {
          t = (ssize_t) ((1.0-QuantumScale*GetPixelAlpha(image,q))*
            (levels.alpha*d+1));
          l = t/d;  t = t-l*d;
          SetPixelAlpha(image,RoundToQuantum((MagickRealType)
            ((1.0-l-(t >= threshold))*(MagickRealType) QuantumRange/
            levels.alpha)),q);
        }
        if (levels.black != 0) {
          t = (ssize_t) (QuantumScale*GetPixelBlack(image,q)*
            (levels.black*d+1));
          l = t/d;  t = t-l*d;
          SetPixelBlack(image,RoundToQuantum((MagickRealType)
            ((l+(t>=threshold))*(MagickRealType) QuantumRange/levels.black)),q);
        }
        q+=GetPixelComponents(image);
      }
      if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
        status=MagickFalse;
      if (image->progress_monitor != (MagickProgressMonitor) NULL)
        {
          MagickBooleanType
            proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_OrderedPosterizeImage)
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
%      MagickBooleanType RandomThresholdImage(Image *image,
%        const char *thresholds,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
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
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  MagickStatusType
    flags;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    threshold;

  MagickRealType
    min_threshold,
    max_threshold;

  RandomInfo
    **restrict random_info;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  GetPixelInfo(image,&threshold);
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
  /*
    Random threshold image.
  */
  status=MagickTrue;
  progress=0;
  if (image->sync != MagickFalse)
    {
      if (AcquireImageColormap(image,2) == MagickFalse)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      random_info=AcquireRandomInfoThreadSet();
      image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        const int
          id = GetOpenMPThreadId();

        MagickBooleanType
          sync;

        register ssize_t
          x;

        register Quantum
          *restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (const Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          MagickRealType
            intensity;

          Quantum
            index;

          intensity=(MagickRealType) GetPixelIntensity(image,q);
          if (intensity < min_threshold)
            threshold.black=min_threshold;
          else
            if (intensity > max_threshold)
              threshold.black=max_threshold;
            else
              threshold.black=(MagickRealType)(QuantumRange*
                GetPseudoRandomValue(random_info[id]));
          index=(Quantum) (intensity <= threshold.black ? 0 : 1);
          SetPixelIndex(image,index,q);
          SetPixelPacket(image,image->colormap+(ssize_t) index,q);
          q+=GetPixelComponents(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_RandomThresholdImage)
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
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        {
          if ((MagickRealType) GetPixelRed(image,q) < min_threshold)
            threshold.red=min_threshold;
          else
            if ((MagickRealType) GetPixelRed(image,q) > max_threshold)
              threshold.red=max_threshold;
            else
              threshold.red=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        {
          if ((MagickRealType) GetPixelGreen(image,q) < min_threshold)
            threshold.green=min_threshold;
          else
            if ((MagickRealType) GetPixelGreen(image,q) > max_threshold)
              threshold.green=max_threshold;
            else
              threshold.green=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        {
          if ((MagickRealType) GetPixelBlue(image,q) < min_threshold)
            threshold.blue=min_threshold;
          else
            if ((MagickRealType) GetPixelBlue(image,q) > max_threshold)
              threshold.blue=max_threshold;
            else
              threshold.blue=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          if ((MagickRealType) GetPixelBlack(image,q) < min_threshold)
            threshold.black=min_threshold;
          else
            if ((MagickRealType) GetPixelBlack(image,q) > max_threshold)
              threshold.black=max_threshold;
            else
              threshold.black=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        {
          if ((MagickRealType) GetPixelAlpha(image,q) < min_threshold)
            threshold.alpha=min_threshold;
          else
            if ((MagickRealType) GetPixelAlpha(image,q) > max_threshold)
              threshold.alpha=max_threshold;
            else
              threshold.alpha=(MagickRealType) (QuantumRange*
                GetPseudoRandomValue(random_info[id]));
        }
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        SetPixelRed(image,(Quantum) ((MagickRealType)
          GetPixelRed(image,q) <= threshold.red ? 0 : QuantumRange),q);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        SetPixelGreen(image,(Quantum) ((MagickRealType)
          GetPixelGreen(image,q) <= threshold.green ? 0 : QuantumRange),q);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        SetPixelBlue(image,(Quantum) ((MagickRealType)
          GetPixelBlue(image,q) <= threshold.blue ? 0 : QuantumRange),q);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        SetPixelBlack(image,(Quantum) ((MagickRealType)
          GetPixelBlack(image,q) <= threshold.black ? 0 : QuantumRange),q);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        SetPixelAlpha(image,(Quantum) ((MagickRealType)
          GetPixelAlpha(image,q) <= threshold.alpha ? 0 : QuantumRange),q);
      q+=GetPixelComponents(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_RandomThresholdImage)
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
%      MagickBooleanType WhiteThresholdImage(Image *image,
%        const char *threshold,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: Define the threshold value.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType WhiteThresholdImage(Image *image,
  const char *thresholds,ExceptionInfo *exception)
{
#define ThresholdImageTag  "Threshold/Image"

  CacheView
    *image_view;

  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  PixelInfo
    threshold;

  MagickOffsetType
    progress;

  MagickStatusType
    flags;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (thresholds == (const char *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  flags=ParseGeometry(thresholds,&geometry_info);
  GetPixelInfo(image,&threshold);
  threshold.red=geometry_info.rho;
  threshold.green=geometry_info.sigma;
  if ((flags & SigmaValue) == 0)
    threshold.green=threshold.red;
  threshold.blue=geometry_info.xi;
  if ((flags & XiValue) == 0)
    threshold.blue=threshold.red;
  threshold.alpha=geometry_info.psi;
  if ((flags & PsiValue) == 0)
    threshold.alpha=threshold.red;
  threshold.black=geometry_info.chi;
  if ((flags & ChiValue) == 0)
    threshold.black=threshold.red;
  if ((flags & PercentValue) != 0)
    {
      threshold.red*=(QuantumRange/100.0);
      threshold.green*=(QuantumRange/100.0);
      threshold.blue*=(QuantumRange/100.0);
      threshold.alpha*=(QuantumRange/100.0);
      threshold.black*=(QuantumRange/100.0);
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
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (image->sync != MagickFalse)
        {
          if (GetPixelIntensity(image,q) > GetPixelInfoIntensity(&threshold))
            {
              SetPixelRed(image,QuantumRange,q);
              SetPixelGreen(image,QuantumRange,q);
              SetPixelBlue(image,QuantumRange,q);
              if (image->colorspace == CMYKColorspace)
                SetPixelBlack(image,QuantumRange,q);
            }
        }
      else
        {
          if (((GetPixelRedTraits(image) & ActivePixelTrait) != 0) &&
              ((MagickRealType) GetPixelRed(image,q) > threshold.red))
            SetPixelRed(image,QuantumRange,q);
          if (((GetPixelGreenTraits(image) & ActivePixelTrait) != 0) &&
              ((MagickRealType) GetPixelGreen(image,q) > threshold.green))
            SetPixelGreen(image,QuantumRange,q);
          if (((GetPixelBlueTraits(image) & ActivePixelTrait) != 0) &&
              ((MagickRealType) GetPixelBlue(image,q) > threshold.blue))
            SetPixelBlue(image,QuantumRange,q);
          if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
              (image->colorspace == CMYKColorspace) &&
              ((MagickRealType) GetPixelBlack(image,q)) > threshold.black)
            SetPixelBlack(image,QuantumRange,q);
          if (((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0) &&
              ((MagickRealType) GetPixelAlpha(image,q) > threshold.alpha))
            SetPixelAlpha(image,QuantumRange,q);
        }
      q+=GetPixelComponents(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_WhiteThresholdImage)
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

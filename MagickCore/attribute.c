/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%         AAA   TTTTT  TTTTT  RRRR   IIIII  BBBB   U   U  TTTTT  EEEEE        %
%        A   A    T      T    R   R    I    B   B  U   U    T    E            %
%        AAAAA    T      T    RRRR     I    BBBB   U   U    T    EEE          %
%        A   A    T      T    R R      I    B   B  U   U    T    E            %
%        A   A    T      T    R  R   IIIII  BBBB    UUU     T    EEEEE        %
%                                                                             %
%                                                                             %
%                    MagickCore Get / Set Image Attributes                    %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                October 2002                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/draw-private.h"
#include "MagickCore/effect.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/histogram.h"
#include "MagickCore/identify.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/magick.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/segment.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"

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
  CacheView
    *image_view;

  MagickBooleanType
    status;

  PixelInfo
    target[3],
    zero;

  RectangleInfo
    bounds;

  register const Quantum
    *r;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  bounds.width=0;
  bounds.height=0;
  bounds.x=(ssize_t) image->columns;
  bounds.y=(ssize_t) image->rows;
  GetPixelInfo(image,&target[0]);
  image_view=AcquireVirtualCacheView(image,exception);
  r=GetCacheViewVirtualPixels(image_view,0,0,1,1,exception);
  if (r == (const Quantum *) NULL)
    {
      image_view=DestroyCacheView(image_view);
      return(bounds);
    }
  GetPixelInfoPixel(image,r,&target[0]);
  GetPixelInfo(image,&target[1]);
  r=GetCacheViewVirtualPixels(image_view,(ssize_t) image->columns-1,0,1,1,
    exception);
  if (r != (const Quantum *) NULL)
    GetPixelInfoPixel(image,r,&target[1]);
  GetPixelInfo(image,&target[2]);
  r=GetCacheViewVirtualPixels(image_view,0,(ssize_t) image->rows-1,1,1,
    exception);
  if (r != (const Quantum *) NULL)
    GetPixelInfoPixel(image,r,&target[2]);
  status=MagickTrue;
  GetPixelInfo(image,&zero);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    PixelInfo
      pixel;

    RectangleInfo
      bounding_box;

    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
#  pragma omp critical (MagickCore_GetImageBoundingBox)
#endif
    bounding_box=bounds;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    pixel=zero;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      GetPixelInfoPixel(image,p,&pixel);
      if ((x < bounding_box.x) &&
          (IsFuzzyEquivalencePixelInfo(&pixel,&target[0]) == MagickFalse))
        bounding_box.x=x;
      if ((x > (ssize_t) bounding_box.width) &&
          (IsFuzzyEquivalencePixelInfo(&pixel,&target[1]) == MagickFalse))
        bounding_box.width=(size_t) x;
      if ((y < bounding_box.y) &&
          (IsFuzzyEquivalencePixelInfo(&pixel,&target[0]) == MagickFalse))
        bounding_box.y=y;
      if ((y > (ssize_t) bounding_box.height) &&
          (IsFuzzyEquivalencePixelInfo(&pixel,&target[2]) == MagickFalse))
        bounding_box.height=(size_t) y;
      p+=GetPixelChannels(image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
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
  if ((bounds.width == 0) && (bounds.height == 0))
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
%   G e t I m a g e D e p t h                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageDepth() returns the depth of a particular image channel.
%
%  The format of the GetImageDepth method is:
%
%      size_t GetImageDepth(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport size_t GetImageDepth(const Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  register ssize_t
    i;

  size_t
    *current_depth,
    depth,
    number_threads;

  ssize_t
    y;

  /*
    Compute image depth.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  current_depth=(size_t *) AcquireQuantumMemory(number_threads,
    sizeof(*current_depth));
  if (current_depth == (size_t *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  status=MagickTrue;
  for (i=0; i < (ssize_t) number_threads; i++)
    current_depth[i]=1;
  if ((image->storage_class == PseudoClass) &&
      (image->alpha_trait == UndefinedPixelTrait))
    {
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        const int
          id = GetOpenMPThreadId();

        while (current_depth[id] < MAGICKCORE_QUANTUM_DEPTH)
        {
          MagickBooleanType
            atDepth;

          QuantumAny
            range;

          atDepth=MagickTrue;
          range=GetQuantumRange(current_depth[id]);
          if ((atDepth != MagickFalse) &&
              (GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
            if (IsPixelAtDepth(ClampToQuantum(image->colormap[i].red),range) == MagickFalse)
              atDepth=MagickFalse;
          if ((atDepth != MagickFalse) &&
              (GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
            if (IsPixelAtDepth(ClampToQuantum(image->colormap[i].green),range) == MagickFalse)
              atDepth=MagickFalse;
          if ((atDepth != MagickFalse) &&
              (GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
            if (IsPixelAtDepth(ClampToQuantum(image->colormap[i].blue),range) == MagickFalse)
              atDepth=MagickFalse;
          if ((atDepth != MagickFalse))
            break;
          current_depth[id]++;
        }
      }
      depth=current_depth[0];
      for (i=1; i < (ssize_t) number_threads; i++)
        if (depth < current_depth[i])
          depth=current_depth[i];
      current_depth=(size_t *) RelinquishMagickMemory(current_depth);
      return(depth);
    }
  image_view=AcquireVirtualCacheView(image,exception);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((1UL*QuantumRange) <= MaxMap)
    {
      size_t
        *depth_map;

      /*
        Scale pixels to desired (optimized with depth map).
      */
      depth_map=(size_t *) AcquireQuantumMemory(MaxMap+1,sizeof(*depth_map));
      if (depth_map == (size_t *) NULL)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        unsigned int
          depth;

        for (depth=1; depth < MAGICKCORE_QUANTUM_DEPTH; depth++)
        {
          Quantum
            pixel;

          QuantumAny
            range;

          range=GetQuantumRange(depth);
          pixel=(Quantum) i;
          if (pixel == ScaleAnyToQuantum(ScaleQuantumToAny(pixel,range),range))
            break;
        }
        depth_map[i]=depth;
      }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        const int
          id = GetOpenMPThreadId();

        register const Quantum
          *magick_restrict p;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
        if (p == (const Quantum *) NULL)
          continue;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register ssize_t
            i;

          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            if (depth_map[ScaleQuantumToMap(p[i])] > current_depth[id])
              current_depth[id]=depth_map[ScaleQuantumToMap(p[i])];
          }
          p+=GetPixelChannels(image);
        }
        if (current_depth[id] == MAGICKCORE_QUANTUM_DEPTH)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      depth=current_depth[0];
      for (i=1; i < (ssize_t) number_threads; i++)
        if (depth < current_depth[i])
          depth=current_depth[i];
      depth_map=(size_t *) RelinquishMagickMemory(depth_map);
      current_depth=(size_t *) RelinquishMagickMemory(current_depth);
      return(depth);
    }
#endif
  /*
    Compute pixel depth.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      continue;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          traits;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        while (current_depth[id] < MAGICKCORE_QUANTUM_DEPTH)
        {
          QuantumAny
            range;

          range=GetQuantumRange(current_depth[id]);
          if (p[i] == ScaleAnyToQuantum(ScaleQuantumToAny(p[i],range),range))
            break;
          current_depth[id]++;
        }
      }
      p+=GetPixelChannels(image);
    }
    if (current_depth[id] == MAGICKCORE_QUANTUM_DEPTH)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  depth=current_depth[0];
  for (i=1; i < (ssize_t) number_threads; i++)
    if (depth < current_depth[i])
      depth=current_depth[i];
  current_depth=(size_t *) RelinquishMagickMemory(current_depth);
  return(depth);
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
%      size_t GetImageQuantumDepth(const Image *image,
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
MagickExport size_t GetImageQuantumDepth(const Image *image,
  const MagickBooleanType constrain)
{
  size_t
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
    depth=(size_t) MagickMin((double) depth,(double) MAGICKCORE_QUANTUM_DEPTH);
  return(depth);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e T y p e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageType() returns the type of image:
%
%        Bilevel         Grayscale        GrayscaleMatte
%        Palette         PaletteMatte     TrueColor
%        TrueColorMatte  ColorSeparation  ColorSeparationMatte
%
%  The format of the GetImageType method is:
%
%      ImageType GetImageType(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport ImageType GetImageType(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->colorspace == CMYKColorspace)
    {
      if (image->alpha_trait == UndefinedPixelTrait)
        return(ColorSeparationType);
      return(ColorSeparationAlphaType);
    }
  if (IsImageMonochrome(image) != MagickFalse)
    return(BilevelType);
  if (IsImageGray(image) != MagickFalse)
    {
      if (image->alpha_trait != UndefinedPixelTrait)
        return(GrayscaleAlphaType);
      return(GrayscaleType);
    }
  if (IsPaletteImage(image) != MagickFalse)
    {
      if (image->alpha_trait != UndefinedPixelTrait)
        return(PaletteAlphaType);
      return(PaletteType);
    }
  if (image->alpha_trait != UndefinedPixelTrait)
    return(TrueColorAlphaType);
  return(TrueColorType);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I d e n t i f y I m a g e G r a y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IdentifyImageGray() returns grayscale if all the pixels in the image have
%  the same red, green, and blue intensities, and bi-level is the intensity is
%  either 0 or QuantumRange. Otherwise undefined is returned.
%
%  The format of the IdentifyImageGray method is:
%
%      ImageType IdentifyImageGray(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ImageType IdentifyImageGray(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  ImageType
    type;

  register const Quantum
    *p;

  register ssize_t
    x;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->type == BilevelType) || (image->type == GrayscaleType) ||
      (image->type == GrayscaleAlphaType))
    return(image->type);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    return(UndefinedType);
  type=BilevelType;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (IsPixelGray(image,p) == MagickFalse)
        {
          type=UndefinedType;
          break;
        }
      if ((type == BilevelType) &&
          (IsPixelMonochrome(image,p) == MagickFalse))
        type=GrayscaleType;
      p+=GetPixelChannels(image);
    }
    if (type == UndefinedType)
      break;
  }
  image_view=DestroyCacheView(image_view);
  if ((type == GrayscaleType) && (image->alpha_trait != UndefinedPixelTrait))
    type=GrayscaleAlphaType;
  return(type);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I d e n t i f y I m a g e M o n o c h r o m e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IdentifyImageMonochrome() returns MagickTrue if all the pixels in the image
%  have the same red, green, and blue intensities and the intensity is either
%  0 or QuantumRange.
%
%  The format of the IdentifyImageMonochrome method is:
%
%      MagickBooleanType IdentifyImageMonochrome(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IdentifyImageMonochrome(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    bilevel;

  register ssize_t
    x;

  register const Quantum
    *p;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->type == BilevelType)
    return(MagickTrue);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    return(MagickFalse);
  bilevel=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (IsPixelMonochrome(image,p) == MagickFalse)
        {
          bilevel=MagickFalse;
          break;
        }
      p+=GetPixelChannels(image);
    }
    if (bilevel == MagickFalse)
      break;
  }
  image_view=DestroyCacheView(image_view);
  return(bilevel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I d e n t i f y I m a g e T y p e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IdentifyImageType() returns the potential type of image:
%
%        Bilevel         Grayscale        GrayscaleMatte
%        Palette         PaletteMatte     TrueColor
%        TrueColorMatte  ColorSeparation  ColorSeparationMatte
%
%  To ensure the image type matches its potential, use SetImageType():
%
%    (void) SetImageType(image,IdentifyImageType(image,exception),exception);
%
%  The format of the IdentifyImageType method is:
%
%      ImageType IdentifyImageType(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ImageType IdentifyImageType(const Image *image,
  ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->colorspace == CMYKColorspace)
    {
      if (image->alpha_trait == UndefinedPixelTrait)
        return(ColorSeparationType);
      return(ColorSeparationAlphaType);
    }
  if (IdentifyImageMonochrome(image,exception) != MagickFalse)
    return(BilevelType);
  if (IdentifyImageGray(image,exception) != UndefinedType)
    {
      if (image->alpha_trait != UndefinedPixelTrait)
        return(GrayscaleAlphaType);
      return(GrayscaleType);
    }
  if (IdentifyPaletteImage(image,exception) != MagickFalse)
    {
      if (image->alpha_trait != UndefinedPixelTrait)
        return(PaletteAlphaType);
      return(PaletteType);
    }
  if (image->alpha_trait != UndefinedPixelTrait)
    return(TrueColorAlphaType);
  return(TrueColorType);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s I m a g e G r a y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImageGray() returns MagickTrue if the type of the image is grayscale or
%  bi-level.
%
%  The format of the IsImageGray method is:
%
%      MagickBooleanType IsImageGray(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsImageGray(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if ((image->type == BilevelType) || (image->type == GrayscaleType) ||
      (image->type == GrayscaleAlphaType))
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s I m a g e M o n o c h r o m e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImageMonochrome() returns MagickTrue if type of the image is bi-level.
%
%  The format of the IsImageMonochrome method is:
%
%      MagickBooleanType IsImageMonochrome(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsImageMonochrome(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->type == BilevelType)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s I m a g e O p a q u e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImageOpaque() returns MagickTrue if none of the pixels in the image have
%  an alpha value other than OpaqueAlpha (QuantumRange).
%
%  Will return true immediatally is alpha channel is not available.
%
%  The format of the IsImageOpaque method is:
%
%      MagickBooleanType IsImageOpaque(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsImageOpaque(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  register const Quantum
    *p;

  register ssize_t
    x;

  ssize_t
    y;

  /*
    Determine if image is opaque.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->alpha_trait == UndefinedPixelTrait)
    return(MagickTrue);
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelAlpha(image,p) != OpaqueAlpha)
        break;
      p+=GetPixelChannels(image);
    }
    if (x < (ssize_t) image->columns)
      break;
  }
  image_view=DestroyCacheView(image_view);
  return(y < (ssize_t) image->rows ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e D e p t h                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageDepth() sets the depth of the image.
%
%  The format of the SetImageDepth method is:
%
%      MagickBooleanType SetImageDepth(Image *image,const size_t depth,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o depth: the image depth.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageDepth(Image *image,
  const size_t depth,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  QuantumAny
    range;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickCoreSignature);
  if (depth >= MAGICKCORE_QUANTUM_DEPTH)
    {
      image->depth=depth;
      return(MagickTrue);
    }
  range=GetQuantumRange(depth);
  if (image->storage_class == PseudoClass)
    {
      register ssize_t
        i;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->colors,1)
#endif
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].red=(double) ScaleAnyToQuantum(ScaleQuantumToAny(
            ClampPixel(image->colormap[i].red),range),range);
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].green=(double) ScaleAnyToQuantum(ScaleQuantumToAny(
            ClampPixel(image->colormap[i].green),range),range);
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].blue=(double) ScaleAnyToQuantum(ScaleQuantumToAny(
            ClampPixel(image->colormap[i].blue),range),range);
        if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].alpha=(double) ScaleAnyToQuantum(ScaleQuantumToAny(
            ClampPixel(image->colormap[i].alpha),range),range);
      }
    }
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((1UL*QuantumRange) <= MaxMap)
    {
      Quantum
        *depth_map;

      register ssize_t
        i;

      /*
        Scale pixels to desired (optimized with depth map).
      */
      depth_map=(Quantum *) AcquireQuantumMemory(MaxMap+1,sizeof(*depth_map));
      if (depth_map == (Quantum *) NULL)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
      for (i=0; i <= (ssize_t) MaxMap; i++)
        depth_map[i]=ScaleAnyToQuantum(ScaleQuantumToAny((Quantum) i,range),
          range);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        register ssize_t
          x;

        register Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register ssize_t
            i;

          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel
              channel;

            PixelTrait
              traits;

            channel=GetPixelChannelChannel(image,i);
            traits=GetPixelChannelTraits(image,channel);
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            q[i]=depth_map[ScaleQuantumToMap(q[i])];
          }
          q+=GetPixelChannels(image);
        }
        if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
          {
            status=MagickFalse;
            continue;
          }
      }
      image_view=DestroyCacheView(image_view);
      depth_map=(Quantum *) RelinquishMagickMemory(depth_map);
      if (status != MagickFalse)
        image->depth=depth;
      return(status);
    }
#endif
  /*
    Scale pixels to desired depth.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register ssize_t
      x;

    register Quantum
      *magick_restrict q;

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
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          traits;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ScaleAnyToQuantum(ScaleQuantumToAny(ClampPixel((MagickRealType)
          q[i]),range),range);
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      {
        status=MagickFalse;
        continue;
      }
  }
  image_view=DestroyCacheView(image_view);
  if (status != MagickFalse)
    image->depth=depth;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e T y p e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageType() sets the type of image.  Choose from these types:
%
%        Bilevel        Grayscale       GrayscaleMatte
%        Palette        PaletteMatte    TrueColor
%        TrueColorMatte ColorSeparation ColorSeparationMatte
%        OptimizeType
%
%  The format of the SetImageType method is:
%
%      MagickBooleanType SetImageType(Image *image,const ImageType type,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o type: Image type.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageType(Image *image,const ImageType type,
  ExceptionInfo *exception)
{
  const char
    *artifact;

  ImageInfo
    *image_info;

  MagickBooleanType
    status;

  QuantizeInfo
    *quantize_info;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickCoreSignature);
  status=MagickTrue;
  image_info=AcquireImageInfo();
  image_info->dither=image->dither;
  artifact=GetImageArtifact(image,"dither");
  if (artifact != (const char *) NULL)
    (void) SetImageOption(image_info,"dither",artifact);
  switch (type)
  {
    case BilevelType:
    {
      status=TransformImageColorspace(image,GRAYColorspace,exception);
      (void) NormalizeImage(image,exception);
      quantize_info=AcquireQuantizeInfo(image_info);
      quantize_info->number_colors=2;
      quantize_info->colorspace=GRAYColorspace;
      status=QuantizeImage(quantize_info,image,exception);
      quantize_info=DestroyQuantizeInfo(quantize_info);
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case GrayscaleType:
    {
      status=TransformImageColorspace(image,GRAYColorspace,exception);
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case GrayscaleAlphaType:
    {
      status=TransformImageColorspace(image,GRAYColorspace,exception);
      if (image->alpha_trait == UndefinedPixelTrait)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      break;
    }
    case PaletteType:
    {
      status=TransformImageColorspace(image,sRGBColorspace,exception);
      if ((image->storage_class == DirectClass) || (image->colors > 256))
        {
          quantize_info=AcquireQuantizeInfo(image_info);
          quantize_info->number_colors=256;
          status=QuantizeImage(quantize_info,image,exception);
          quantize_info=DestroyQuantizeInfo(quantize_info);
        }
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case PaletteBilevelAlphaType:
    {
      ChannelType
        channel_mask;

      status=TransformImageColorspace(image,sRGBColorspace,exception);
      if (image->alpha_trait == UndefinedPixelTrait)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      channel_mask=SetImageChannelMask(image,AlphaChannel);
      (void) BilevelImage(image,(double) QuantumRange/2.0,exception);
      (void) SetImageChannelMask(image,channel_mask);
      quantize_info=AcquireQuantizeInfo(image_info);
      status=QuantizeImage(quantize_info,image,exception);
      quantize_info=DestroyQuantizeInfo(quantize_info);
      break;
    }
    case PaletteAlphaType:
    {
      status=TransformImageColorspace(image,sRGBColorspace,exception);
      if (image->alpha_trait == UndefinedPixelTrait)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      quantize_info=AcquireQuantizeInfo(image_info);
      quantize_info->colorspace=TransparentColorspace;
      status=QuantizeImage(quantize_info,image,exception);
      quantize_info=DestroyQuantizeInfo(quantize_info);
      break;
    }
    case TrueColorType:
    {
      status=TransformImageColorspace(image,sRGBColorspace,exception);
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass,exception);
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case TrueColorAlphaType:
    {
      status=TransformImageColorspace(image,sRGBColorspace,exception);
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass,exception);
      if (image->alpha_trait == UndefinedPixelTrait)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      break;
    }
    case ColorSeparationType:
    {
      status=TransformImageColorspace(image,CMYKColorspace,exception);
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass,exception);
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case ColorSeparationAlphaType:
    {
      status=TransformImageColorspace(image,CMYKColorspace,exception);
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass,exception);
      if (image->alpha_trait == UndefinedPixelTrait)
        status=SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      break;
    }
    case OptimizeType:
    case UndefinedType:
      break;
  }
  image_info=DestroyImageInfo(image_info);
  if (status == MagickFalse)
    return(status);
  image->type=type;
  return(MagickTrue);
}

/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore pixel accessor methods.
*/
#ifndef _MAGICKCORE_PIXEL_ACCESSOR_H
#define _MAGICKCORE_PIXEL_ACCESSOR_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <math.h>
#include <MagickCore/cache.h>
#include <MagickCore/cache-view.h>
#include <MagickCore/color.h>
#include <MagickCore/colorspace-private.h>
#include <MagickCore/image.h>

#undef index

static inline Quantum GetPixelAlpha(const Image *restrict image,
  const Quantum *restrict pixel)
{
  if (image->channel_map[AlphaPixelChannel].traits == UndefinedPixelTrait)
    return(OpaqueAlpha);
  return(pixel[image->channel_map[AlphaPixelChannel].offset]);
}

static inline PixelTrait GetPixelAlphaTraits(const Image *restrict image)
{
  return(image->channel_map[AlphaPixelChannel].traits);
}

static inline Quantum GetPixelBlack(const Image *restrict image,
  const Quantum *restrict pixel)
{
  if (image->channel_map[BlackPixelChannel].traits == UndefinedPixelTrait)
    return(0);
  return(pixel[image->channel_map[BlackPixelChannel].offset]);
}

static inline PixelTrait GetPixelBlackTraits(const Image *restrict image)
{
  return(image->channel_map[BlackPixelChannel].traits);
}

static inline Quantum GetPixelBlue(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[BluePixelChannel].offset]);
}

static inline PixelTrait GetPixelBlueTraits(const Image *restrict image)
{
  return(image->channel_map[BluePixelChannel].traits);
}

static inline Quantum GetPixelCb(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[CbPixelChannel].offset]);
}

static inline PixelTrait GetPixelCbTraits(const Image *restrict image)
{
  return(image->channel_map[CbPixelChannel].traits);
}

static inline Quantum GetPixelChannel(const Image *restrict image,
  const PixelChannel channel,const Quantum *restrict pixel)
{
  if (image->channel_map[channel].traits == UndefinedPixelTrait)
    return(0);
  return(pixel[image->channel_map[channel].offset]);
}

static inline PixelChannel GetPixelChannelMapChannel(
  const Image *restrict image,const ssize_t offset)
{
  return(image->channel_map[offset].channel);
}

static inline ssize_t GetPixelChannelMapOffset(const Image *restrict image,
  const PixelChannel channel)
{
  return(image->channel_map[channel].offset);
}

static inline PixelTrait GetPixelChannelMapTraits(const Image *restrict image,
  const PixelChannel channel)
{
  return(image->channel_map[channel].traits);
}

static inline size_t GetPixelChannels(const Image *restrict image)
{
  return(image->number_channels);
}

static inline Quantum GetPixelCr(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[CrPixelChannel].offset]);
}

static inline PixelTrait GetPixelCrTraits(const Image *restrict image)
{
  return(image->channel_map[CrPixelChannel].traits);
}

static inline Quantum GetPixelCyan(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[CyanPixelChannel].offset]);
}

static inline PixelTrait GetPixelCyanTraits(const Image *restrict image)
{
  return(image->channel_map[CyanPixelChannel].traits);
}

static inline Quantum GetPixelGray(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[GrayPixelChannel].offset]);
}

static inline PixelTrait GetPixelGrayTraits(const Image *restrict image)
{
  return(image->channel_map[GrayPixelChannel].traits);
}

static inline Quantum GetPixelGreen(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[GreenPixelChannel].offset]);
}

static inline PixelTrait GetPixelGreenTraits(const Image *restrict image)
{
  return(image->channel_map[GreenPixelChannel].traits);
}

static inline Quantum GetPixelIndex(const Image *restrict image,
  const Quantum *restrict pixel)
{
  if (image->channel_map[IndexPixelChannel].traits == UndefinedPixelTrait)
    return(0);
  return(pixel[image->channel_map[IndexPixelChannel].offset]);
}

static inline PixelTrait GetPixelIndexTraits(const Image *restrict image)
{
  return(image->channel_map[IndexPixelChannel].traits);
}

static inline Quantum GetPixelInfoIntensity(
  const PixelInfo *restrict pixel_info)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (0.299*pixel_info->red+0.587*pixel_info->green+0.114*
    pixel_info->blue+0.5));
#else
  return((Quantum) (0.299*pixel_info->red+0.587*pixel_info->green+0.114*
    pixel_info->blue));
#endif
}

static inline Quantum GetPixelInfoLuminance(
  const PixelInfo *restrict pixel_info)
{
  Quantum
    luminance;

#if !defined(MAGICKCORE_HDRI_SUPPORT)
  luminance=(Quantum) (0.21267*pixel_info->red+0.71516*pixel_info->green+
    0.07217*pixel_info->blue+0.5);
#else
  luminance=(Quantum) (0.21267*pixel_info->red+0.71516*pixel_info->green+
    0.07217*pixel_info->blue);
#endif
  return((Quantum) luminance);
}

static inline Quantum GetPixelIntensity(const Image *restrict image,
  const Quantum *restrict pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if (image->colorspace == GRAYColorspace)
    return(pixel[image->channel_map[GrayPixelChannel].offset]);
  return((Quantum) (0.299*pixel[image->channel_map[RedPixelChannel].offset]+
    0.587*pixel[image->channel_map[GreenPixelChannel].offset]+0.114*
    pixel[image->channel_map[BluePixelChannel].offset]+0.5));
#else
  {
    double
      alpha,
      beta;

    alpha=pixel[image->channel_map[RedPixelChannel].offset]-(double)
      pixel[image->channel_map[GreenPixelChannel].offset];
    beta=pixel[image->channel_map[GreenPixelChannel].offset]-(double)
      pixel[image->channel_map[BluePixelChannel].offset];
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(pixel[image->channel_map[RedPixelChannel].offset]);
    return((Quantum) (0.299*pixel[image->channel_map[RedPixelChannel].offset]+
      0.587*pixel[image->channel_map[GreenPixelChannel].offset]+0.114*
      pixel[image->channel_map[BluePixelChannel].offset]));
  }
#endif
}

static inline Quantum GetPixelLuminance(const Image *restrict image,
  const Quantum *restrict pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (0.21267*pixel[image->channel_map[RedPixelChannel].offset]+
    0.71516*pixel[image->channel_map[GreenPixelChannel].offset]+0.07217*
    pixel[image->channel_map[BluePixelChannel].offset]+0.5));
#else
  return((Quantum) (0.21267*pixel[image->channel_map[RedPixelChannel].offset]+
    0.71516*pixel[image->channel_map[GreenPixelChannel].offset]+0.07217*
    pixel[image->channel_map[BluePixelChannel].offset]));
#endif
}

static inline Quantum GetPixelMagenta(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[MagentaPixelChannel].offset]);
}

static inline PixelTrait GetPixelMagentaTraits(const Image *restrict image)
{
  return(image->channel_map[MagentaPixelChannel].traits);
}

static inline Quantum GetPixelMask(const Image *restrict image,
  const Quantum *restrict pixel)
{
  if (image->channel_map[MaskPixelChannel].traits == UndefinedPixelTrait)
    return(0);
  return(pixel[image->channel_map[MaskPixelChannel].offset]);
}

static inline PixelTrait GetPixelMaskTraits(const Image *restrict image)
{
  return(image->channel_map[MaskPixelChannel].traits);
}

static inline size_t GetPixelMetaChannels(const Image *restrict image)
{
  return(image->number_meta_channels);
}

static inline size_t GetPixelMetacontentExtent(const Image *restrict image)
{
  return(image->metacontent_extent);
}

static inline Quantum GetPixelOpacity(const Image *restrict image,
  const Quantum *restrict pixel)
{
  if (image->channel_map[AlphaPixelChannel].traits == UndefinedPixelTrait)
    return(QuantumRange-OpaqueAlpha);
  return(QuantumRange-pixel[image->channel_map[AlphaPixelChannel].offset]);
}

static inline Quantum GetPixelRed(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[RedPixelChannel].offset]);
}

static inline PixelTrait GetPixelRedTraits(const Image *restrict image)
{
  return(image->channel_map[RedPixelChannel].traits);
}

static inline void GetPixelInfoPixel(const Image *restrict image,
  const Quantum *restrict pixel,PixelInfo *restrict pixel_info)
{
  pixel_info->red=(double)
    pixel[image->channel_map[RedPixelChannel].offset];
  pixel_info->green=(double)
    pixel[image->channel_map[GreenPixelChannel].offset];
  pixel_info->blue=(double)
    pixel[image->channel_map[BluePixelChannel].offset];
  pixel_info->black=0;
  if (image->channel_map[BlackPixelChannel].traits != UndefinedPixelTrait)
    pixel_info->black=(double)
      pixel[image->channel_map[BlackPixelChannel].offset];
  pixel_info->alpha=OpaqueAlpha;
  if (image->channel_map[AlphaPixelChannel].traits != UndefinedPixelTrait)
    pixel_info->alpha=(double)
      pixel[image->channel_map[AlphaPixelChannel].offset];
  pixel_info->index=0;
  if (image->channel_map[IndexPixelChannel].traits != UndefinedPixelTrait)
    pixel_info->index=(double)
      pixel[image->channel_map[IndexPixelChannel].offset];
}

static inline PixelTrait GetPixelTraits(const Image *restrict image,
  const PixelChannel channel)
{
  return(image->channel_map[channel].traits);
}

static inline Quantum GetPixelY(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[YPixelChannel].offset]);
}

static inline PixelTrait GetPixelYTraits(const Image *restrict image)
{
  return(image->channel_map[YPixelChannel].traits);
}

static inline Quantum GetPixelYellow(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[YellowPixelChannel].offset]);
}

static inline PixelTrait GetPixelYellowTraits(const Image *restrict image)
{
  return(image->channel_map[YellowPixelChannel].traits);
}

static inline MagickBooleanType IsPixelEquivalent(const Image *restrict image,
  const Quantum *restrict p,const PixelInfo *restrict q)
{
  if (((double) p[image->channel_map[RedPixelChannel].offset] == q->red) &&
      ((double) p[image->channel_map[GreenPixelChannel].offset] == q->green) &&
      ((double) p[image->channel_map[BluePixelChannel].offset] == q->blue))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelGray(const Image *restrict image,
  const Quantum *restrict pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((pixel[image->channel_map[RedPixelChannel].offset] ==
       pixel[image->channel_map[GreenPixelChannel].offset]) &&
      (pixel[image->channel_map[GreenPixelChannel].offset] ==
       pixel[image->channel_map[BluePixelChannel].offset]))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=pixel[image->channel_map[RedPixelChannel].offset]-(double)
      pixel[image->channel_map[GreenPixelChannel].offset];
    beta=pixel[image->channel_map[GreenPixelChannel].offset]-(double)
      pixel[image->channel_map[BluePixelChannel].offset];
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
  }
#endif
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoEquivalent(
  const PixelInfo *restrict p,const PixelInfo *restrict q)
{
  if ((p->matte != MagickFalse) && (q->matte == MagickFalse) &&
      (fabs(p->alpha-OpaqueAlpha) > 0.5))
    return(MagickFalse);
  if ((q->matte != MagickFalse) && (p->matte == MagickFalse) &&
      (fabs(q->alpha-OpaqueAlpha)) > 0.5)
    return(MagickFalse);
  if ((p->matte != MagickFalse) && (q->matte != MagickFalse))
    {
      if (fabs(p->alpha-q->alpha) > 0.5)
        return(MagickFalse);
      if (fabs(p->alpha-TransparentAlpha) <= 0.5)
        return(MagickTrue);
    }
  if (fabs(p->red-q->red) > 0.5)
    return(MagickFalse);
  if (fabs(p->green-q->green) > 0.5)
    return(MagickFalse);
  if (fabs(p->blue-q->blue) > 0.5)
    return(MagickFalse);
  if ((p->colorspace == CMYKColorspace) && (fabs(p->black-q->black) > 0.5))
    return(MagickFalse);
  return(MagickTrue);
}

static inline MagickBooleanType IsPixelMonochrome(const Image *restrict image,
  const Quantum *restrict pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if (((pixel[image->channel_map[RedPixelChannel].offset] == 0) ||
      (pixel[image->channel_map[RedPixelChannel].offset] == (Quantum) QuantumRange)) &&
      (pixel[image->channel_map[RedPixelChannel].offset] ==
       pixel[image->channel_map[GreenPixelChannel].offset]) &&
      (pixel[image->channel_map[GreenPixelChannel].offset] ==
       pixel[image->channel_map[BluePixelChannel].offset]))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=pixel[image->channel_map[RedPixelChannel].offset]-(double)
      pixel[image->channel_map[GreenPixelChannel].offset];
    beta=pixel[image->channel_map[GreenPixelChannel].offset]-(double)
      pixel[image->channel_map[BluePixelChannel].offset];
    if (((fabs(pixel[image->channel_map[RedPixelChannel].offset]) <= MagickEpsilon) ||
         (fabs(pixel[image->channel_map[RedPixelChannel].offset]-QuantumRange) <= MagickEpsilon)) &&
        (fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
    }
#endif
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoGray(
  const PixelInfo *restrict pixel_info)
{
  if ((IsGrayColorspace(pixel_info->colorspace) == MagickFalse) &&
      (IsRGBColorspace(pixel_info->colorspace) == MagickFalse))
    return(MagickFalse);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((pixel_info->red == pixel_info->green) &&
      (pixel_info->green == pixel_info->blue))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=pixel_info->red-(double) pixel_info->green;
    beta=pixel_info->green-(double) pixel_info->blue;
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
  }
#endif
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoMonochrome(
  const PixelInfo *restrict pixel_info)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if (((pixel_info->red == 0) || (pixel_info->red == (Quantum) QuantumRange)) &&
      (pixel_info->red == pixel_info->green) &&
      (pixel_info->green == pixel_info->blue))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=pixel_info->red-(double) pixel_info->green;
    beta=pixel_info->green-(double) pixel_info->blue;
    if (((fabs(pixel_info->red) <= MagickEpsilon) ||
         (fabs(pixel_info->red-QuantumRange) <= MagickEpsilon)) &&
        (fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
    }
#endif
  return(MagickFalse);
}

static inline void SetPixelAlpha(const Image *restrict image,
  const Quantum alpha,Quantum *restrict pixel)
{
  if (image->channel_map[AlphaPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[AlphaPixelChannel].offset]=alpha;
}

static inline void SetPixelAlphaTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[AlphaPixelChannel].traits=traits;
}

static inline void SetPixelBlack(const Image *restrict image,
  const Quantum black,Quantum *restrict pixel)
{
  if (image->channel_map[BlackPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[BlackPixelChannel].offset]=black;
}

static inline void SetPixelBlackTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[BlackPixelChannel].traits=traits;
}

static inline void SetPixelBlue(const Image *restrict image,const Quantum blue,
  Quantum *restrict pixel)
{
  pixel[image->channel_map[BluePixelChannel].offset]=blue;
}

static inline void SetPixelBlueTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[BluePixelChannel].traits=traits;
}

static inline void SetPixelCb(const Image *restrict image,const Quantum cb,
  Quantum *restrict pixel)
{
  pixel[image->channel_map[CbPixelChannel].offset]=cb;
}

static inline void SetPixelCbTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[CbPixelChannel].traits=traits;
}

static inline void SetPixelChannel(const Image *restrict image,
  const PixelChannel channel,const Quantum quantum,Quantum *restrict pixel)
{
  if (image->channel_map[channel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[channel].offset]=quantum;
}

static inline void SetPixelChannelMapChannel(const Image *restrict image,
  const PixelChannel channel,const ssize_t offset)
{
  image->channel_map[offset].channel=channel;
  image->channel_map[channel].offset=offset;
}

static inline void SetPixelChannelMap(const Image *restrict image,
  const PixelChannel channel,const PixelTrait traits,const ssize_t offset)
{
  image->channel_map[offset].channel=channel;
  image->channel_map[channel].offset=offset;
  image->channel_map[channel].traits=traits;
}

static inline void SetPixelChannels(Image *image,const size_t number_channels)
{
  image->number_channels=number_channels;
}

static inline void SetPixelChannelTraits(Image *image,
  const PixelChannel channel,const PixelTrait traits)
{
  image->channel_map[channel].traits=traits;
}

static inline void SetPixelChannelMapTraits(Image *image,
  const PixelChannel channel,const PixelTrait traits)
{
  image->channel_map[channel].traits=traits;
}

static inline void SetPixelCr(const Image *restrict image,const Quantum cr,
  Quantum *restrict pixel)
{
  pixel[image->channel_map[CrPixelChannel].offset]=cr;
}

static inline void SetPixelCrTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[CrPixelChannel].traits=traits;
}

static inline void SetPixelCyan(const Image *restrict image,const Quantum cyan,
  Quantum *restrict pixel)
{
  pixel[image->channel_map[CyanPixelChannel].offset]=cyan;
}

static inline void SetPixelGray(const Image *restrict image,const Quantum gray,
  Quantum *restrict pixel)
{
  pixel[image->channel_map[GrayPixelChannel].offset]=gray;
}

static inline void SetPixelGrayTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[GrayPixelChannel].traits=traits;
}

static inline void SetPixelGreen(const Image *restrict image,
  const Quantum green,Quantum *restrict pixel)
{
  pixel[image->channel_map[GreenPixelChannel].offset]=green;
}

static inline void SetPixelGreenTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[GreenPixelChannel].traits=traits;
}

static inline void SetPixelIndex(const Image *restrict image,
  const Quantum index,Quantum *restrict pixel)
{
  if (image->channel_map[IndexPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[IndexPixelChannel].offset]=index;
}

static inline void SetPixelIndexTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[IndexPixelChannel].traits=traits;
}

static inline void SetPixelInfoPixel(const Image *restrict image,
  const PixelInfo *restrict pixel_info,Quantum *restrict pixel)
{
  pixel[image->channel_map[RedPixelChannel].offset]=
    ClampToQuantum(pixel_info->red);
  pixel[image->channel_map[GreenPixelChannel].offset]=
    ClampToQuantum(pixel_info->green);
  pixel[image->channel_map[BluePixelChannel].offset]=
    ClampToQuantum(pixel_info->blue);
  if (image->channel_map[BlackPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[BlackPixelChannel].offset]=
      ClampToQuantum(pixel_info->black);
  if (image->channel_map[AlphaPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[AlphaPixelChannel].offset]=pixel_info->matte ==
      MagickFalse ? OpaqueAlpha : ClampToQuantum(pixel_info->alpha);
}

static inline void SetPixelMagenta(const Image *restrict image,
  const Quantum magenta,Quantum *restrict pixel)
{
  pixel[image->channel_map[MagentaPixelChannel].offset]=magenta;
}

static inline void SetPixelMagentaTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[MagentaPixelChannel].traits=traits;
}

static inline void SetPixelMask(const Image *restrict image,
  const Quantum mask,Quantum *restrict pixel)
{
  if (image->channel_map[MaskPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[MaskPixelChannel].offset]=mask;
}

static inline void SetPixelMetacontentExtent(Image *image,const size_t extent)
{
  image->metacontent_extent=extent;
}

static inline void SetPixelOpacity(const Image *restrict image,
  const Quantum alpha,Quantum *restrict pixel)
{
  if (image->channel_map[AlphaPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[AlphaPixelChannel].offset]=QuantumRange-alpha;
}

static inline void SetPixelRed(const Image *restrict image,const Quantum red,
  Quantum *restrict pixel)
{
  pixel[image->channel_map[RedPixelChannel].offset]=red;
}

static inline void SetPixelRedTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[RedPixelChannel].traits=traits;
}

static inline void SetPixelYellow(const Image *restrict image,
  const Quantum yellow,Quantum *restrict pixel)
{
  pixel[image->channel_map[YellowPixelChannel].offset]=yellow;
}

static inline void SetPixelYellowTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[YellowPixelChannel].traits=traits;
}

static inline void SetPixelY(const Image *restrict image,const Quantum y,
  Quantum *restrict pixel)
{
  pixel[image->channel_map[YPixelChannel].offset]=y;
}

static inline void SetPixelYTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[YPixelChannel].traits=traits;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

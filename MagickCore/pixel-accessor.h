/*
  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore pixel accessor methods.
*/
#ifndef MAGICKCORE_PIXEL_ACCESSOR_H
#define MAGICKCORE_PIXEL_ACCESSOR_H

#include <assert.h>
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/gem.h"
#include "MagickCore/image.h"
#include "MagickCore/memory_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#undef index

static inline Quantum ClampPixel(const MagickRealType pixel)
{
  if (pixel < 0.0f)
    return((Quantum) 0);
  if (pixel >= (MagickRealType) QuantumRange)
    return((Quantum) QuantumRange);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (pixel+0.5f));
#else
  return((Quantum) pixel);
#endif
}

static inline Quantum GetPixela(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[aPixelChannel].offset]);
}

static inline Quantum GetPixelAlpha(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  if (image->channel_map[AlphaPixelChannel].traits == UndefinedPixelTrait)
    return(OpaqueAlpha);
  return(pixel[image->channel_map[AlphaPixelChannel].offset]);
}

static inline PixelTrait GetPixelAlphaTraits(
  const Image *magick_restrict image)
{
  return(image->channel_map[AlphaPixelChannel].traits);
}

static inline Quantum GetPixelb(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[bPixelChannel].offset]);
}

static inline Quantum GetPixelBlack(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  if (image->channel_map[BlackPixelChannel].traits == UndefinedPixelTrait)
    return((Quantum) 0);
  return(pixel[image->channel_map[BlackPixelChannel].offset]);
}

static inline PixelTrait GetPixelBlackTraits(
  const Image *magick_restrict image)
{
  return(image->channel_map[BlackPixelChannel].traits);
}

static inline Quantum GetPixelBlue(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[BluePixelChannel].offset]);
}

static inline PixelTrait GetPixelBlueTraits(const Image *magick_restrict image)
{
  return(image->channel_map[BluePixelChannel].traits);
}

static inline Quantum GetPixelCb(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[CbPixelChannel].offset]);
}

static inline PixelTrait GetPixelCbTraits(const Image *magick_restrict image)
{
  return(image->channel_map[CbPixelChannel].traits);
}

static inline Quantum GetPixelChannel(const Image *magick_restrict image,
  const PixelChannel channel,const Quantum *magick_restrict pixel)
{
  if (image->channel_map[channel].traits == UndefinedPixelTrait)
    return((Quantum) 0);
  return(pixel[image->channel_map[channel].offset]);
}

static inline PixelChannel GetPixelChannelChannel(
  const Image *magick_restrict image,const ssize_t offset)
{
  return(image->channel_map[offset].channel);
}

static inline ssize_t GetPixelChannelOffset(const Image *magick_restrict image,
  const PixelChannel channel)
{
  return(image->channel_map[channel].offset);
}

static inline PixelTrait GetPixelChannelTraits(
  const Image *magick_restrict image,const PixelChannel channel)
{
  return(image->channel_map[channel].traits);
}

static inline size_t GetPixelChannels(const Image *magick_restrict image)
{
  return(image->number_channels);
}

static inline Quantum GetPixelCompositeMask(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  if (image->channel_map[CompositeMaskPixelChannel].traits == UndefinedPixelTrait)
    return((Quantum) QuantumRange);
  return(pixel[image->channel_map[CompositeMaskPixelChannel].offset]);
}

static inline Quantum GetPixelCr(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[CrPixelChannel].offset]);
}

static inline PixelTrait GetPixelCrTraits(const Image *magick_restrict image)
{
  return(image->channel_map[CrPixelChannel].traits);
}

static inline Quantum GetPixelCyan(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[CyanPixelChannel].offset]);
}

static inline PixelTrait GetPixelCyanTraits(const Image *magick_restrict image)
{
  return(image->channel_map[CyanPixelChannel].traits);
}

static inline Quantum GetPixelGray(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[GrayPixelChannel].offset]);
}

static inline PixelTrait GetPixelGrayTraits(const Image *magick_restrict image)
{
  return(image->channel_map[GrayPixelChannel].traits);
}

static inline Quantum GetPixelGreen(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[GreenPixelChannel].offset]);
}

static inline PixelTrait GetPixelGreenTraits(
  const Image *magick_restrict image)
{
  return(image->channel_map[GreenPixelChannel].traits);
}

static inline Quantum GetPixelIndex(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  if (image->channel_map[IndexPixelChannel].traits == UndefinedPixelTrait)
    return((Quantum) 0);
  return(pixel[image->channel_map[IndexPixelChannel].offset]);
}

static inline PixelTrait GetPixelIndexTraits(
  const Image *magick_restrict image)
{
  return(image->channel_map[IndexPixelChannel].traits);
}

static inline MagickRealType GetPixelInfoChannel(
  const PixelInfo *magick_restrict pixel_info,const PixelChannel channel)
{
  switch (channel)
  {
    case RedPixelChannel: return(pixel_info->red);
    case GreenPixelChannel: return(pixel_info->green);
    case BluePixelChannel: return(pixel_info->blue);
    case BlackPixelChannel: return(pixel_info->black);
    case AlphaPixelChannel: return(pixel_info->alpha);
    case IndexPixelChannel: return(pixel_info->index);
    default: return((MagickRealType) 0.0);
  }
}

static inline double PerceptibleReciprocal(const double x)
{
  double
    sign;

  /*
    Return 1/x where x is perceptible (not unlimited or infinitesimal).
  */
  sign=x < 0.0 ? -1.0 : 1.0;
  if ((sign*x) >= MagickEpsilon)
    return(1.0/x);
  return(sign/MagickEpsilon);
}

static inline MagickRealType GetPixelInfoLuma(
  const PixelInfo *magick_restrict pixel)
{
  MagickRealType
    intensity;

  if (pixel->colorspace == sRGBColorspace)
    {
      intensity=(MagickRealType) (0.212656f*pixel->red+0.715158f*pixel->green+
        0.072186f*pixel->blue);
      return(intensity);
    }
  intensity=(MagickRealType) (0.212656f*EncodePixelGamma(pixel->red)+
    0.715158f*EncodePixelGamma(pixel->green)+
    0.072186f*EncodePixelGamma(pixel->blue));
  return(intensity);
}

static inline MagickRealType GetPixelInfoLuminance(
  const PixelInfo *magick_restrict pixel)
{
  MagickRealType
    intensity;

  if (pixel->colorspace != sRGBColorspace)
    {
      intensity=(MagickRealType) (0.212656f*pixel->red+0.715158f*pixel->green+
        0.072186f*pixel->blue);
      return(intensity);
    }
  intensity=(MagickRealType) (0.212656f*DecodePixelGamma(pixel->red)+
    0.715158f*DecodePixelGamma(pixel->green)+
    0.072186f*DecodePixelGamma(pixel->blue));
  return(intensity);
}

static inline Quantum GetPixelL(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[LPixelChannel].offset]);
}

static inline ssize_t GetPixelLabel(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return((ssize_t) pixel[image->channel_map[LabelPixelChannel].offset]);
}

static inline MagickRealType GetPixelLuma(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  MagickRealType
    intensity;

  intensity=(MagickRealType) (
    0.212656f*pixel[image->channel_map[RedPixelChannel].offset]+
    0.715158f*pixel[image->channel_map[GreenPixelChannel].offset]+
    0.072186f*pixel[image->channel_map[BluePixelChannel].offset]);
  return(intensity);
}

static inline MagickRealType GetPixelLuminance(
  const Image *magick_restrict image,const Quantum *magick_restrict pixel)
{
  MagickRealType
    intensity;

  if (image->colorspace != sRGBColorspace)
    {
      intensity=(MagickRealType) (
        0.212656f*pixel[image->channel_map[RedPixelChannel].offset]+
        0.715158f*pixel[image->channel_map[GreenPixelChannel].offset]+
        0.072186f*pixel[image->channel_map[BluePixelChannel].offset]);
      return(intensity);
    }
  intensity=(MagickRealType) (0.212656f*DecodePixelGamma((MagickRealType)
    pixel[image->channel_map[RedPixelChannel].offset])+0.715158f*
    DecodePixelGamma((MagickRealType)
    pixel[image->channel_map[GreenPixelChannel].offset])+0.072186f*
    DecodePixelGamma((MagickRealType)
    pixel[image->channel_map[BluePixelChannel].offset]));
  return(intensity);
}

static inline Quantum GetPixelMagenta(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[MagentaPixelChannel].offset]);
}

static inline PixelTrait GetPixelMagentaTraits(
  const Image *magick_restrict image)
{
  return(image->channel_map[MagentaPixelChannel].traits);
}

static inline Quantum GetPixelReadMask(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  if (image->channel_map[ReadMaskPixelChannel].traits == UndefinedPixelTrait)
    return((Quantum) QuantumRange);
  return(pixel[image->channel_map[ReadMaskPixelChannel].offset]);
}

static inline Quantum GetPixelWriteMask(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  if (image->channel_map[WriteMaskPixelChannel].traits == UndefinedPixelTrait)
    return((Quantum) QuantumRange);
  return(pixel[image->channel_map[WriteMaskPixelChannel].offset]);
}

static inline PixelTrait GetPixelReadMaskTraits(
  const Image *magick_restrict image)
{
  return(image->channel_map[ReadMaskPixelChannel].traits);
}

static inline size_t GetPixelMetaChannels(const Image *magick_restrict image)
{
  return(image->number_meta_channels);
}

static inline size_t GetPixelMetacontentExtent(
  const Image *magick_restrict image)
{
  return(image->metacontent_extent);
}

static inline Quantum GetPixelOpacity(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  if (image->channel_map[AlphaPixelChannel].traits != BlendPixelTrait)
    return(QuantumRange-OpaqueAlpha);
  return(QuantumRange-pixel[image->channel_map[AlphaPixelChannel].offset]);
}

static inline Quantum GetPixelRed(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[RedPixelChannel].offset]);
}

static inline PixelTrait GetPixelRedTraits(const Image *magick_restrict image)
{
  return(image->channel_map[RedPixelChannel].traits);
}

static inline void GetPixelInfoPixel(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel,PixelInfo *magick_restrict pixel_info)
{
  (void) ResetMagickMemory(pixel_info,0,sizeof(*pixel_info));
  pixel_info->storage_class=DirectClass;
  pixel_info->colorspace=sRGBColorspace;
  pixel_info->depth=MAGICKCORE_QUANTUM_DEPTH;
  pixel_info->alpha_trait=UndefinedPixelTrait;
  pixel_info->alpha=(MagickRealType) OpaqueAlpha;
  if (image != (Image *) NULL)
    {
      pixel_info->storage_class=image->storage_class;
      pixel_info->colorspace=image->colorspace;
      pixel_info->fuzz=image->fuzz;
      pixel_info->depth=image->depth;
      pixel_info->alpha_trait=image->alpha_trait;
    }
  if (pixel != (Quantum *) NULL)
    {
      pixel_info->red=(MagickRealType)
        pixel[image->channel_map[RedPixelChannel].offset];
      pixel_info->green=(MagickRealType)
        pixel[image->channel_map[GreenPixelChannel].offset];
      pixel_info->blue=(MagickRealType)
        pixel[image->channel_map[BluePixelChannel].offset];
      if (image->channel_map[BlackPixelChannel].traits != UndefinedPixelTrait)
        pixel_info->black=(MagickRealType)
          pixel[image->channel_map[BlackPixelChannel].offset];
      if (image->channel_map[AlphaPixelChannel].traits != UndefinedPixelTrait)
        {
          pixel_info->alpha=(MagickRealType)
            pixel[image->channel_map[AlphaPixelChannel].offset];
          pixel_info->alpha_trait=BlendPixelTrait;
        }
      if (image->channel_map[IndexPixelChannel].traits != UndefinedPixelTrait)
        pixel_info->index=(MagickRealType)
      pixel[image->channel_map[IndexPixelChannel].offset];
    }
}

static inline PixelTrait GetPixelTraits(const Image *magick_restrict image,
  const PixelChannel channel)
{
  return(image->channel_map[channel].traits);
}

static inline Quantum GetPixelY(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[YPixelChannel].offset]);
}

static inline PixelTrait GetPixelYTraits(const Image *magick_restrict image)
{
  return(image->channel_map[YPixelChannel].traits);
}

static inline Quantum GetPixelYellow(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  return(pixel[image->channel_map[YellowPixelChannel].offset]);
}

static inline PixelTrait GetPixelYellowTraits(
  const Image *magick_restrict image)
{
  return(image->channel_map[YellowPixelChannel].traits);
}

static inline MagickRealType AbsolutePixelValue(const MagickRealType x)
{
  return(x < 0.0f ? -x : x);
}

static inline MagickBooleanType IsPixelAtDepth(const Quantum pixel,
  const QuantumAny range)
{
  Quantum
    quantum;

#if !defined(MAGICKCORE_HDRI_SUPPORT)
  quantum=(Quantum) (((MagickRealType) QuantumRange*((QuantumAny)
    (((MagickRealType) range*pixel)/QuantumRange+0.5)))/range+0.5);
#else
  quantum=(Quantum) (((MagickRealType) QuantumRange*((QuantumAny)
    (((MagickRealType) range*pixel)/QuantumRange+0.5)))/range);
#endif
  return(pixel == quantum ? MagickTrue : MagickFalse);
}

static inline MagickBooleanType IsPixelEquivalent(
  const Image *magick_restrict image,const Quantum *magick_restrict p,
  const PixelInfo *magick_restrict q)
{
  MagickRealType
    alpha,
    beta,
    color;

  color=(MagickRealType) p[image->channel_map[AlphaPixelChannel].offset];
  alpha=image->alpha_trait == UndefinedPixelTrait ? (MagickRealType)
    OpaqueAlpha : color;
  beta=q->alpha_trait == UndefinedPixelTrait ? (MagickRealType) OpaqueAlpha :
    q->alpha;
  if (AbsolutePixelValue(alpha-beta) >= MagickEpsilon)
    return(MagickFalse);
  if ((AbsolutePixelValue(alpha-TransparentAlpha) < MagickEpsilon) ||
      (AbsolutePixelValue(beta-TransparentAlpha) < MagickEpsilon))
    return(MagickTrue);  /* no color component if pixel is transparent */
  color=(MagickRealType) p[image->channel_map[RedPixelChannel].offset];
  if (AbsolutePixelValue(color-q->red) >= MagickEpsilon)
    return(MagickFalse);
  color=(MagickRealType) p[image->channel_map[GreenPixelChannel].offset];
  if (AbsolutePixelValue(color-q->green) >= MagickEpsilon)
    return(MagickFalse);
  color=(MagickRealType) p[image->channel_map[BluePixelChannel].offset];
  if (AbsolutePixelValue(color-q->blue) >= MagickEpsilon)
    return(MagickFalse);
  if (image->colorspace == CMYKColorspace)
    {
      color=(MagickRealType) p[image->channel_map[BlackPixelChannel].offset];
      if (AbsolutePixelValue(color-q->black) >= MagickEpsilon)
        return(MagickFalse);
    }
  return(MagickTrue);
}

static inline MagickBooleanType IsPixelGray(const Image *magick_restrict image,
  const Quantum *magick_restrict pixel)
{
  MagickRealType
    green_blue,
    red_green;

  red_green=(MagickRealType) pixel[image->channel_map[RedPixelChannel].offset]-
    pixel[image->channel_map[GreenPixelChannel].offset];
  green_blue=(MagickRealType)
    pixel[image->channel_map[GreenPixelChannel].offset]-
    pixel[image->channel_map[BluePixelChannel].offset];
  if ((AbsolutePixelValue(red_green) < MagickEpsilon) &&
      (AbsolutePixelValue(green_blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoEquivalent(
  const PixelInfo *magick_restrict p,const PixelInfo *magick_restrict q)
{
  MagickRealType
    alpha,
    beta;

  alpha=p->alpha_trait == UndefinedPixelTrait ? (MagickRealType) OpaqueAlpha :
    p->alpha;
  beta=q->alpha_trait == UndefinedPixelTrait ? (MagickRealType) OpaqueAlpha :
    q->alpha;
  if (AbsolutePixelValue(alpha-beta) >= MagickEpsilon)
    return(MagickFalse);
  if ((AbsolutePixelValue(alpha-TransparentAlpha) < MagickEpsilon) ||
      (AbsolutePixelValue(beta-TransparentAlpha) < MagickEpsilon))
    return(MagickTrue);  /* no color component if pixel is transparent */
  if (AbsolutePixelValue(p->red-q->red) >= MagickEpsilon)
    return(MagickFalse);
  if (AbsolutePixelValue(p->green-q->green) >= MagickEpsilon)
    return(MagickFalse);
  if (AbsolutePixelValue(p->blue-q->blue) >= MagickEpsilon)
    return(MagickFalse);
  if (p->colorspace == CMYKColorspace)
    {
      if (AbsolutePixelValue(p->black-q->black) >= MagickEpsilon)
        return(MagickFalse);
    }
  return(MagickTrue);
}

static inline MagickBooleanType IsPixelMonochrome(
  const Image *magick_restrict image,const Quantum *magick_restrict pixel)
{
  MagickRealType
    green_blue,
    red,
    red_green;

  red=(MagickRealType) pixel[image->channel_map[RedPixelChannel].offset];
  if ((AbsolutePixelValue(red) >= MagickEpsilon) &&
      (AbsolutePixelValue(red-QuantumRange) >= MagickEpsilon))
    return(MagickFalse);
  red_green=(MagickRealType) pixel[image->channel_map[RedPixelChannel].offset]-
    pixel[image->channel_map[GreenPixelChannel].offset];
  green_blue=(MagickRealType)
    pixel[image->channel_map[GreenPixelChannel].offset]-
    pixel[image->channel_map[BluePixelChannel].offset];
  if ((AbsolutePixelValue(red_green) < MagickEpsilon) &&
      (AbsolutePixelValue(green_blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoGray(
  const PixelInfo *magick_restrict pixel)
{
  if ((AbsolutePixelValue(pixel->red-pixel->green) < MagickEpsilon) &&
      (AbsolutePixelValue(pixel->green-pixel->blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoMonochrome(
  const PixelInfo *magick_restrict pixel_info)
{
  MagickRealType
    green_blue,
    red_green;

  if ((AbsolutePixelValue(pixel_info->red) >= MagickEpsilon) ||
      (AbsolutePixelValue(pixel_info->red-QuantumRange) >= MagickEpsilon))
    return(MagickFalse);
  red_green=pixel_info->red-pixel_info->green;
  green_blue=pixel_info->green-pixel_info->blue;
  if ((AbsolutePixelValue(red_green) < MagickEpsilon) &&
      (AbsolutePixelValue(green_blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline void SetPixela(const Image *magick_restrict image,
  const Quantum a,Quantum *magick_restrict pixel)
{
  if (image->channel_map[aPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[aPixelChannel].offset]=a;
}

static inline void SetPixelAlpha(const Image *magick_restrict image,
  const Quantum alpha,Quantum *magick_restrict pixel)
{
  if (image->channel_map[AlphaPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[AlphaPixelChannel].offset]=alpha;
}

static inline void SetPixelAlphaTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[AlphaPixelChannel].traits=traits;
}

static inline void SetPixelb(const Image *magick_restrict image,
  const Quantum b,Quantum *magick_restrict pixel)
{
  if (image->channel_map[bPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[bPixelChannel].offset]=b;
}

static inline void SetPixelBackgoundColor(const Image *magick_restrict image,
  Quantum *magick_restrict pixel)
{
  register ssize_t
    i;

  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
    pixel[i]=(Quantum) 0;
  pixel[image->channel_map[RedPixelChannel].offset]=
    ClampToQuantum(image->background_color.red);
  pixel[image->channel_map[GreenPixelChannel].offset]=
    ClampToQuantum(image->background_color.green);
  pixel[image->channel_map[BluePixelChannel].offset]=
    ClampToQuantum(image->background_color.blue);
  if (image->channel_map[BlackPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[BlackPixelChannel].offset]=
      ClampToQuantum(image->background_color.black);
  if (image->channel_map[AlphaPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[AlphaPixelChannel].offset]=
      image->background_color.alpha_trait == UndefinedPixelTrait ? OpaqueAlpha :
      ClampToQuantum(image->background_color.alpha);
}

static inline void SetPixelBlack(const Image *magick_restrict image,
  const Quantum black,Quantum *magick_restrict pixel)
{
  if (image->channel_map[BlackPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[BlackPixelChannel].offset]=black;
}

static inline void SetPixelBlackTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[BlackPixelChannel].traits=traits;
}

static inline void SetPixelBlue(const Image *magick_restrict image,
  const Quantum blue,Quantum *magick_restrict pixel)
{
  pixel[image->channel_map[BluePixelChannel].offset]=blue;
}

static inline void SetPixelBlueTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[BluePixelChannel].traits=traits;
}

static inline void SetPixelCb(const Image *magick_restrict image,
  const Quantum cb,Quantum *magick_restrict pixel)
{
  pixel[image->channel_map[CbPixelChannel].offset]=cb;
}

static inline void SetPixelCbTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[CbPixelChannel].traits=traits;
}

static inline void SetPixelChannel(const Image *magick_restrict image,
  const PixelChannel channel,const Quantum quantum,
  Quantum *magick_restrict pixel)
{
  if (image->channel_map[channel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[channel].offset]=quantum;
}

static inline void SetPixelChannelAttributes(
  const Image *magick_restrict image,const PixelChannel channel,
  const PixelTrait traits,const ssize_t offset)
{
  assert((ssize_t) channel < MaxPixelChannels);
  assert(offset < MaxPixelChannels);
  image->channel_map[offset].channel=channel;
  image->channel_map[channel].offset=offset;
  image->channel_map[channel].traits=traits;
}

static inline void SetPixelChannelChannel(const Image *magick_restrict image,
  const PixelChannel channel,const ssize_t offset)
{
  image->channel_map[offset].channel=channel;
  image->channel_map[channel].offset=offset;
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

static inline void SetPixelCompositeMask(const Image *magick_restrict image,
  const Quantum mask,Quantum *magick_restrict pixel)
{
  if (image->channel_map[CompositeMaskPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[CompositeMaskPixelChannel].offset]=mask;
}

static inline void SetPixelCr(const Image *magick_restrict image,
  const Quantum cr,Quantum *magick_restrict pixel)
{
  pixel[image->channel_map[CrPixelChannel].offset]=cr;
}

static inline void SetPixelCrTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[CrPixelChannel].traits=traits;
}

static inline void SetPixelCyan(const Image *magick_restrict image,
  const Quantum cyan,Quantum *magick_restrict pixel)
{
  pixel[image->channel_map[CyanPixelChannel].offset]=cyan;
}

static inline void SetPixelGray(const Image *magick_restrict image,
  const Quantum gray,Quantum *magick_restrict pixel)
{
  pixel[image->channel_map[GrayPixelChannel].offset]=gray;
}

static inline void SetPixelGrayTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[GrayPixelChannel].traits=traits;
}

static inline void SetPixelGreen(const Image *magick_restrict image,
  const Quantum green,Quantum *magick_restrict pixel)
{
  pixel[image->channel_map[GreenPixelChannel].offset]=green;
}

static inline void SetPixelGreenTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[GreenPixelChannel].traits=traits;
}

static inline void SetPixelIndex(const Image *magick_restrict image,
  const Quantum index,Quantum *magick_restrict pixel)
{
  if (image->channel_map[IndexPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[IndexPixelChannel].offset]=index;
}

static inline void SetPixelIndexTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[IndexPixelChannel].traits=traits;
}

static inline void SetPixelViaPixelInfo(const Image *magick_restrict image,
  const PixelInfo *magick_restrict pixel_info,Quantum *magick_restrict pixel)
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
    pixel[image->channel_map[AlphaPixelChannel].offset]=
      pixel_info->alpha_trait == UndefinedPixelTrait ? OpaqueAlpha :
      ClampToQuantum(pixel_info->alpha);
}

static inline void SetPixelL(const Image *magick_restrict image,const Quantum L,
  Quantum *magick_restrict pixel)
{
  if (image->channel_map[LPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[LPixelChannel].offset]=L;
}

static inline void SetPixelMagenta(const Image *magick_restrict image,
  const Quantum magenta,Quantum *magick_restrict pixel)
{
  pixel[image->channel_map[MagentaPixelChannel].offset]=magenta;
}

static inline void SetPixelMagentaTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[MagentaPixelChannel].traits=traits;
}

static inline void SetPixelReadMask(const Image *magick_restrict image,
  const Quantum mask,Quantum *magick_restrict pixel)
{
  if (image->channel_map[ReadMaskPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[ReadMaskPixelChannel].offset]=mask;
}

static inline void SetPixelWriteMask(const Image *magick_restrict image,
  const Quantum mask,Quantum *magick_restrict pixel)
{
  if (image->channel_map[WriteMaskPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[WriteMaskPixelChannel].offset]=mask;
}

static inline void SetPixelMetacontentExtent(Image *image,const size_t extent)
{
  image->metacontent_extent=extent;
}

static inline void SetPixelOpacity(const Image *magick_restrict image,
  const Quantum alpha,Quantum *magick_restrict pixel)
{
  if (image->channel_map[AlphaPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[AlphaPixelChannel].offset]=QuantumRange-alpha;
}

static inline void SetPixelRed(const Image *magick_restrict image,
  const Quantum red,Quantum *magick_restrict pixel)
{
  pixel[image->channel_map[RedPixelChannel].offset]=red;
}

static inline void SetPixelRedTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[RedPixelChannel].traits=traits;
}

static inline void SetPixelYellow(const Image *magick_restrict image,
  const Quantum yellow,Quantum *magick_restrict pixel)
{
  pixel[image->channel_map[YellowPixelChannel].offset]=yellow;
}

static inline void SetPixelYellowTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[YellowPixelChannel].traits=traits;
}

static inline void SetPixelY(const Image *magick_restrict image,
  const Quantum y,Quantum *magick_restrict pixel)
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

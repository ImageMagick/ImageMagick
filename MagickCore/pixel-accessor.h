/*
  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization
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

#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/gem.h"
#include "MagickCore/image.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#undef index

static inline Quantum GetPixela(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[aPixelChannel].offset]);
}

static inline Quantum GetPixelAlpha(const Image *restrict image,
  const Quantum *restrict pixel)
{
  if (image->channel_map[AlphaPixelChannel].traits != BlendPixelTrait)
    return(OpaqueAlpha);
  return(pixel[image->channel_map[AlphaPixelChannel].offset]);
}

static inline PixelTrait GetPixelAlphaTraits(const Image *restrict image)
{
  return(image->channel_map[AlphaPixelChannel].traits);
}

static inline Quantum GetPixelb(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[bPixelChannel].offset]);
}

static inline Quantum GetPixelBlack(const Image *restrict image,
  const Quantum *restrict pixel)
{
  if (image->channel_map[BlackPixelChannel].traits == UndefinedPixelTrait)
    return((Quantum) 0);
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
    return((Quantum) 0);
  return(pixel[image->channel_map[channel].offset]);
}

static inline PixelChannel GetPixelChannelChannel(const Image *restrict image,
  const ssize_t offset)
{
  return(image->channel_map[offset].channel);
}

static inline ssize_t GetPixelChannelOffset(const Image *restrict image,
  const PixelChannel channel)
{
  return(image->channel_map[channel].offset);
}

static inline PixelTrait GetPixelChannelTraits(const Image *restrict image,
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
    return((Quantum) 0);
  return(pixel[image->channel_map[IndexPixelChannel].offset]);
}

static inline PixelTrait GetPixelIndexTraits(const Image *restrict image)
{
  return(image->channel_map[IndexPixelChannel].traits);
}

static inline MagickRealType GetPixelInfoChannel(
  const PixelInfo *restrict pixel_info,const PixelChannel channel)
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

static inline MagickRealType GetPixelInfoIntensity(
  const PixelInfo *restrict pixel_info)
{
  if (pixel_info->colorspace == GRAYColorspace)
    return(pixel_info->red);
  return(0.212656f*pixel_info->red+0.715158f*pixel_info->green+0.072186f*
    pixel_info->blue);
}

static inline MagickRealType GetPixelInfoLuma(const PixelInfo *restrict pixel)
{
  MagickRealType
    blue,
    green,
    red;

  if (pixel->colorspace == GRAYColorspace)
    return(pixel->red);
  if (pixel->colorspace == sRGBColorspace)
    return(0.212656f*pixel->red+0.715158f*pixel->green+0.072186f*pixel->blue);
  red=EncodePixelGamma(pixel->red);
  green=EncodePixelGamma(pixel->green);
  blue=EncodePixelGamma(pixel->blue);
  return(0.212656f*red+0.715158f*green+0.072186f*blue);
}

static inline MagickRealType GetPixelInfoLuminance(
  const PixelInfo *restrict pixel)
{
  MagickRealType
    blue,
    green,
    red;

  if (pixel->colorspace == GRAYColorspace)
    return(pixel->red);
  if (pixel->colorspace != sRGBColorspace)
    return(0.212656f*pixel->red+0.715158f*pixel->green+0.072186f*pixel->blue);
  red=DecodePixelGamma(pixel->red);
  green=DecodePixelGamma(pixel->green);
  blue=DecodePixelGamma(pixel->blue);
  return(0.212656f*red+0.715158f*green+0.072186f*blue);
}

static inline Quantum GetPixelL(const Image *restrict image,
  const Quantum *restrict pixel)
{
  return(pixel[image->channel_map[LPixelChannel].offset]);
}

static inline MagickRealType GetPixelLuma(const Image *restrict image,
  const Quantum *restrict pixel)
{
  if (image->colorspace == GRAYColorspace)
    return((MagickRealType) pixel[image->channel_map[GrayPixelChannel].offset]);
  return(0.212656f*pixel[image->channel_map[RedPixelChannel].offset]+
    0.715158f*pixel[image->channel_map[GreenPixelChannel].offset]+
    0.072186f*pixel[image->channel_map[BluePixelChannel].offset]);  /* Rec709 */
}

static inline MagickRealType GetPixelLuminance(const Image *restrict image,
  const Quantum *restrict pixel)
{
  MagickRealType
    blue,
    green,
    red;

  if (image->colorspace == GRAYColorspace)
    return((MagickRealType) pixel[image->channel_map[GrayPixelChannel].offset]);
  if (image->colorspace != sRGBColorspace)
    return(0.212656f*pixel[image->channel_map[RedPixelChannel].offset]+
      0.715158f*pixel[image->channel_map[GreenPixelChannel].offset]+
      0.072186f*pixel[image->channel_map[BluePixelChannel].offset]);
  red=DecodePixelGamma((MagickRealType)
    pixel[image->channel_map[RedPixelChannel].offset]);
  green=DecodePixelGamma((MagickRealType)
    pixel[image->channel_map[GreenPixelChannel].offset]);
  blue=DecodePixelGamma((MagickRealType)
    pixel[image->channel_map[BluePixelChannel].offset]);
  return(0.212656f*red+0.715158f*green+0.072186f*blue);  /* Rec709 */
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

static inline Quantum GetPixelReadMask(const Image *restrict image,
  const Quantum *restrict pixel)
{
  if (image->channel_map[ReadMaskPixelChannel].traits == UndefinedPixelTrait)
    return((Quantum) QuantumRange);
  return(pixel[image->channel_map[ReadMaskPixelChannel].offset]);
}

static inline Quantum GetPixelWriteMask(const Image *restrict image,
  const Quantum *restrict pixel)
{
  if (image->channel_map[WriteMaskPixelChannel].traits == UndefinedPixelTrait)
    return((Quantum) QuantumRange);
  return(pixel[image->channel_map[WriteMaskPixelChannel].offset]);
}

static inline PixelTrait GetPixelReadMaskTraits(const Image *restrict image)
{
  return(image->channel_map[ReadMaskPixelChannel].traits);
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
  pixel_info->colorspace=image->colorspace;
  pixel_info->fuzz=image->fuzz;
  pixel_info->red=(MagickRealType)
    pixel[image->channel_map[RedPixelChannel].offset];
  pixel_info->green=(MagickRealType)
    pixel[image->channel_map[GreenPixelChannel].offset];
  pixel_info->blue=(MagickRealType)
    pixel[image->channel_map[BluePixelChannel].offset];
  pixel_info->black=0.0f;
  if (image->channel_map[BlackPixelChannel].traits != UndefinedPixelTrait)
    pixel_info->black=(MagickRealType)
      pixel[image->channel_map[BlackPixelChannel].offset];
  pixel_info->alpha=(MagickRealType) OpaqueAlpha;
  pixel_info->alpha_trait=UndefinedPixelTrait;
  if (image->channel_map[AlphaPixelChannel].traits != UndefinedPixelTrait)
    {
      pixel_info->alpha=(MagickRealType)
        pixel[image->channel_map[AlphaPixelChannel].offset];
      pixel_info->alpha_trait=BlendPixelTrait;
    }
  pixel_info->index=0.0f;
  if (image->channel_map[IndexPixelChannel].traits != UndefinedPixelTrait)
    pixel_info->index=(MagickRealType)
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

static inline MagickBooleanType IsPixelEquivalent(const Image *restrict image,
  const Quantum *restrict p,const PixelInfo *restrict q)
{
  MagickRealType
    blue,
    green,
    red;

  red=(MagickRealType) p[image->channel_map[RedPixelChannel].offset];
  green=(MagickRealType) p[image->channel_map[GreenPixelChannel].offset];
  blue=(MagickRealType) p[image->channel_map[BluePixelChannel].offset];
  if ((AbsolutePixelValue(red-q->red) < MagickEpsilon) &&
      (AbsolutePixelValue(green-q->green) < MagickEpsilon) &&
      (AbsolutePixelValue(blue-q->blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelGray(const Image *restrict image,
  const Quantum *restrict pixel)
{
  MagickRealType
    blue,
    green,
    red;

  red=(MagickRealType) pixel[image->channel_map[RedPixelChannel].offset];
  green=(MagickRealType) pixel[image->channel_map[GreenPixelChannel].offset];
  blue=(MagickRealType) pixel[image->channel_map[BluePixelChannel].offset];
  if ((AbsolutePixelValue(red-green) < MagickEpsilon) &&
      (AbsolutePixelValue(green-blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoEquivalent(
  const PixelInfo *restrict p,const PixelInfo *restrict q)
{
  if ((p->alpha_trait != UndefinedPixelTrait) &&
      (q->alpha_trait == UndefinedPixelTrait) &&
      (AbsolutePixelValue(p->alpha-OpaqueAlpha) >= MagickEpsilon))
    return(MagickFalse);
  if ((q->alpha_trait != UndefinedPixelTrait) &&
      (p->alpha_trait == UndefinedPixelTrait) &&
      (AbsolutePixelValue(q->alpha-OpaqueAlpha)) >= MagickEpsilon)
    return(MagickFalse);
  if ((p->alpha_trait != UndefinedPixelTrait) &&
      (q->alpha_trait != UndefinedPixelTrait))
    {
      if (AbsolutePixelValue(p->alpha-q->alpha) >= MagickEpsilon)
        return(MagickFalse);
      if (AbsolutePixelValue(p->alpha-TransparentAlpha) < MagickEpsilon)
        return(MagickTrue);
    }
  if (AbsolutePixelValue(p->red-q->red) >= MagickEpsilon)
    return(MagickFalse);
  if (AbsolutePixelValue(p->green-q->green) >= MagickEpsilon)
    return(MagickFalse);
  if (AbsolutePixelValue(p->blue-q->blue) >= MagickEpsilon)
    return(MagickFalse);
  if ((p->colorspace == CMYKColorspace) &&
      (AbsolutePixelValue(p->black-q->black) >= MagickEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static inline MagickBooleanType IsPixelMonochrome(const Image *restrict image,
  const Quantum *restrict pixel)
{
  MagickRealType
    blue,
    green,
    red;

  red=(MagickRealType) pixel[image->channel_map[RedPixelChannel].offset];
  if ((AbsolutePixelValue(red) >= MagickEpsilon) ||
      (AbsolutePixelValue(red-QuantumRange) >= MagickEpsilon))
    return(MagickFalse);
  green=(MagickRealType) pixel[image->channel_map[GreenPixelChannel].offset];
  blue=(MagickRealType) pixel[image->channel_map[BluePixelChannel].offset];
  if ((AbsolutePixelValue(red-green) < MagickEpsilon) &&
      (AbsolutePixelValue(green-blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoGray(
  const PixelInfo *restrict pixel_info)
{
  if ((pixel_info->colorspace != GRAYColorspace) &&
      (pixel_info->colorspace != RGBColorspace))
    return(MagickFalse);
  if ((AbsolutePixelValue(pixel_info->red-pixel_info->green) < MagickEpsilon) &&
      (AbsolutePixelValue(pixel_info->green-pixel_info->blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoMonochrome(
  const PixelInfo *restrict pixel_info)
{
  if ((pixel_info->colorspace != GRAYColorspace) &&
      (pixel_info->colorspace != RGBColorspace))
    return(MagickFalse);
  if ((AbsolutePixelValue(pixel_info->red) >= MagickEpsilon) ||
      (AbsolutePixelValue(pixel_info->red-QuantumRange) >= MagickEpsilon))
    return(MagickFalse);
  if ((AbsolutePixelValue(pixel_info->red-pixel_info->green) < MagickEpsilon) &&
      (AbsolutePixelValue(pixel_info->green-pixel_info->blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline void SetPixela(const Image *restrict image,
  const Quantum a,Quantum *restrict pixel)
{
  if (image->channel_map[aPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[aPixelChannel].offset]=a;
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

static inline void SetPixelb(const Image *restrict image,
  const Quantum b,Quantum *restrict pixel)
{
  if (image->channel_map[bPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[bPixelChannel].offset]=b;
}

static inline void SetPixelBackgoundColor(const Image *restrict image,
  Quantum *restrict pixel)
{
  register ssize_t
    i;

  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
    pixel[i]=0;
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

static inline void SetPixelChannelAttributes(const Image *restrict image,
  const PixelChannel channel,const PixelTrait traits,const ssize_t offset)
{
  image->channel_map[offset].channel=channel;
  image->channel_map[channel].offset=offset;
  image->channel_map[channel].traits=traits;
}

static inline void SetPixelChannelChannel(const Image *restrict image,
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
    pixel[image->channel_map[AlphaPixelChannel].offset]=
      pixel_info->alpha_trait == UndefinedPixelTrait ? OpaqueAlpha :
      ClampToQuantum(pixel_info->alpha);
}

static inline void SetPixelL(const Image *restrict image,const Quantum L,
  Quantum *restrict pixel)
{
  if (image->channel_map[LPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[LPixelChannel].offset]=L;
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

static inline void SetPixelReadMask(const Image *restrict image,
  const Quantum mask,Quantum *restrict pixel)
{
  if (image->channel_map[ReadMaskPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[ReadMaskPixelChannel].offset]=mask;
}

static inline void SetPixelWriteMask(const Image *restrict image,
  const Quantum mask,Quantum *restrict pixel)
{
  if (image->channel_map[WriteMaskPixelChannel].traits != UndefinedPixelTrait)
    pixel[image->channel_map[WriteMaskPixelChannel].offset]=mask;
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

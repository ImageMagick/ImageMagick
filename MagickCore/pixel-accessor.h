/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
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
#include <MagickCore/cache-view.h>
#include <MagickCore/color.h>
#include <MagickCore/image.h>

#undef index

static inline Quantum GetPixelAlpha(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[AlphaPixelChannel].channel]);
}

static inline PixelTrait GetPixelAlphaTraits(const Image *image)
{
  return(image->channel_map[AlphaPixelChannel].traits);
}

static inline Quantum GetPixelBlack(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[BlackPixelChannel].channel]);
}

static inline PixelTrait GetPixelBlackTraits(const Image *image)
{
  return(image->channel_map[BlackPixelChannel].traits);
}

static inline Quantum GetPixelBlue(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[BluePixelChannel].channel]);
}

static inline PixelTrait GetPixelBlueTraits(const Image *image)
{
  return(image->channel_map[BluePixelChannel].traits);
}

static inline Quantum GetPixelCb(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[CbPixelChannel].channel]);
}

static inline PixelTrait GetPixelCbTraits(const Image *image)
{
  return(image->channel_map[CbPixelChannel].traits);
}

static inline Quantum GetPixelChannel(const Image *image,
  const PixelChannel channel,const Quantum *pixel)
{
  return(pixel[image->channel_map[channel].channel]);
}

static inline PixelChannel GetPixelChannelMapChannel(const Image *image,
  const PixelChannel channel)
{
  return(image->channel_map[channel].channel);
}

static inline PixelTrait GetPixelChannelMapTraits(const Image *image,
  const PixelChannel channel)
{
  return(image->channel_map[channel].traits);
}

static inline size_t GetPixelChannels(const Image *image)
{
  return(image->number_channels);
}

static inline Quantum GetPixelCr(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[CrPixelChannel].channel]);
}

static inline PixelTrait GetPixelCrTraits(const Image *image)
{
  return(image->channel_map[CrPixelChannel].traits);
}

static inline Quantum GetPixelCyan(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[CyanPixelChannel].channel]);
}

static inline PixelTrait GetPixelCyanTraits(const Image *image)
{
  return(image->channel_map[CyanPixelChannel].traits);
}

static inline Quantum GetPixelGray(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[GrayPixelChannel].channel]);
}

static inline PixelTrait GetPixelGrayTraits(const Image *image)
{
  return(image->channel_map[GrayPixelChannel].traits);
}

static inline Quantum GetPixelGreen(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[GreenPixelChannel].channel]);
}

static inline PixelTrait GetPixelGreenTraits(const Image *image)
{
  return(image->channel_map[GreenPixelChannel].traits);
}

static inline Quantum GetPixelIndex(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[IndexPixelChannel].channel]);
}

static inline PixelTrait GetPixelIndexTraits(const Image *image)
{
  return(image->channel_map[IndexPixelChannel].traits);
}

static inline Quantum GetPixelInfoIntensity(const PixelInfo *pixel_info)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (0.299*pixel_info->red+0.587*pixel_info->green+0.114*
    pixel_info->blue+0.5));
#else
  return((Quantum) (0.299*pixel_info->red+0.587*pixel_info->green+0.114*
    pixel_info->blue));
#endif
}

static inline Quantum GetPixelInfoLuminance(const PixelInfo *pixel_info)
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

static inline Quantum GetPixelMagenta(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[MagentaPixelChannel].channel]);
}

static inline PixelTrait GetPixelMagentaTraits(const Image *image)
{
  return(image->channel_map[MagentaPixelChannel].traits);
}

static inline size_t GetPixelMetaChannels(const Image *image)
{
  return(image->number_meta_channels);
}

static inline size_t GetPixelMetacontentExtent(const Image *image)
{
  return(image->metacontent_extent);
}

static inline Quantum GetPixelRed(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[RedPixelChannel].channel]);
}

static inline PixelTrait GetPixelRedTraits(const Image *image)
{
  return(image->channel_map[RedPixelChannel].traits);
}

static inline void GetPixelPacketPixel(const Image *image,const Quantum *pixel,
  PixelPacket *packet)
{
  packet->red=(double) pixel[image->channel_map[RedPixelChannel].channel];
  packet->green=(double) pixel[image->channel_map[GreenPixelChannel].channel];
  packet->blue=(double) pixel[image->channel_map[BluePixelChannel].channel];
  packet->alpha=(double) pixel[image->channel_map[AlphaPixelChannel].channel];
}

static inline Quantum GetPixelPacketIntensity(const PixelPacket *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((pixel->red == pixel->green) && (pixel->green == pixel->blue))
    return((Quantum) pixel->red);
  return((Quantum) (0.299*pixel->red+0.587*pixel->green+0.114*pixel->blue+0.5));
#else
  {
    double
      alpha,
      beta;

    alpha=pixel->red-pixel->green;
    beta=pixel->green-pixel->blue;
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return((Quantum) pixel->red);
    return((Quantum) (0.299*pixel->red+0.587*pixel->green+0.114*pixel->blue));
  }
#endif
}

static inline PixelTrait GetPixelTraits(const Image *image,
  const PixelChannel channel)
{
  return(image->channel_map[channel].traits);
}

static inline Quantum GetPixelY(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[YPixelChannel].channel]);
}

static inline PixelTrait GetPixelYTraits(const Image *image)
{
  return(image->channel_map[YPixelChannel].traits);
}

static inline Quantum GetPixelYellow(const Image *image,const Quantum *pixel)
{
  return(pixel[image->channel_map[YellowPixelChannel].channel]);
}

static inline PixelTrait GetPixelYellowTraits(const Image *image)
{
  return(image->channel_map[YellowPixelChannel].traits);
}

static inline MagickBooleanType IsPixelEquivalent(const Image *image,
  const Quantum *p,const PixelPacket *q)
{
  if (((double) p[image->channel_map[RedPixelChannel].channel] == q->red) &&
      ((double) p[image->channel_map[GreenPixelChannel].channel] == q->green) &&
      ((double) p[image->channel_map[BluePixelChannel].channel] == q->blue))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelGray(const Image *image,
  const Quantum *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((pixel[image->channel_map[RedPixelChannel].channel] ==
       pixel[image->channel_map[GreenPixelChannel].channel]) &&
      (pixel[image->channel_map[GreenPixelChannel].channel] ==
       pixel[image->channel_map[BluePixelChannel].channel]))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=pixel[image->channel_map[RedPixelChannel].channel]-(double)
      pixel[image->channel_map[GreenPixelChannel].channel];
    beta=pixel[image->channel_map[GreenPixelChannel].channel]-(double)
      pixel[image->channel_map[BluePixelChannel].channel];
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
  }
#endif
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoEquivalent(const PixelInfo *p,
  const PixelInfo *q)
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

static inline MagickBooleanType IsPixelMonochrome(const Image *image,
  const Quantum *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if (((pixel[image->channel_map[RedPixelChannel].channel] == 0) ||
      (pixel[image->channel_map[RedPixelChannel].channel] == (Quantum) QuantumRange)) &&
      (pixel[image->channel_map[RedPixelChannel].channel] ==
       pixel[image->channel_map[GreenPixelChannel].channel]) &&
      (pixel[image->channel_map[GreenPixelChannel].channel] ==
       pixel[image->channel_map[BluePixelChannel].channel]))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=pixel[image->channel_map[RedPixelChannel].channel]-(double)
      pixel[image->channel_map[GreenPixelChannel].channel];
    beta=pixel[image->channel_map[GreenPixelChannel].channel]-(double)
      pixel[image->channel_map[BluePixelChannel].channel];
    if (((fabs(pixel[image->channel_map[RedPixelChannel].channel]) <= MagickEpsilon) ||
         (fabs(pixel[image->channel_map[RedPixelChannel].channel]-QuantumRange) <= MagickEpsilon)) &&
        (fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
    }
#endif
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelPacketEquivalent(const PixelPacket *p,
  const PixelPacket *q)
{
  if ((p->red == q->red) && (p->green == q->green) && (p->blue == q->blue))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelPacketGray(const PixelPacket *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((pixel->red == pixel->green) && (pixel->green == pixel->blue))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=pixel->red-(double) pixel->green;
    beta=pixel->green-(double) pixel->blue;
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
  }
#endif
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelPacketMonochrome(
  const PixelPacket *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if (((pixel->red == 0) || (pixel->red == (Quantum) QuantumRange)) &&
      (pixel->red == pixel->green) && (pixel->green == pixel->blue))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=pixel->red-(double) pixel->green;
    beta=pixel->green-(double) pixel->blue;
    if (((fabs(pixel->red) <= MagickEpsilon) ||
         (fabs(pixel->red-QuantumRange) <= MagickEpsilon)) &&
        (fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
    }
#endif
  return(MagickFalse);
}

static inline void SetPacketPixelInfo(const Image *image,
  const PixelInfo *pixel_info,PixelPacket *packet)
{
  packet->red=pixel_info->red;
  packet->green=pixel_info->green;
  packet->blue=pixel_info->blue;
  packet->alpha=pixel_info->alpha;
  if (image->colorspace == CMYKColorspace)
    packet->black=pixel_info->black;
  if (image->storage_class == PseudoClass)
    packet->index=pixel_info->index;
}

static inline void SetPixelAlpha(const Image *image,const Quantum alpha,
  Quantum *pixel)
{
  pixel[image->channel_map[AlphaPixelChannel].channel]=alpha;
}

static inline void SetPixelAlphaTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[AlphaPixelChannel].traits=traits;
}

static inline void SetPixelBlack(const Image *image,const Quantum black,
  Quantum *pixel)
{
  pixel[image->channel_map[BlackPixelChannel].channel]=black;
}

static inline void SetPixelBlackTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[BlackPixelChannel].traits=traits;
}

static inline void SetPixelBlue(const Image *image,const Quantum blue,
  Quantum *pixel)
{
  pixel[image->channel_map[BluePixelChannel].channel]=blue;
}

static inline void SetPixelBlueTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[BluePixelChannel].traits=traits;
}

static inline void SetPixelCb(const Image *image,const Quantum cb,
  Quantum *pixel)
{
  pixel[image->channel_map[CbPixelChannel].channel]=cb;
}

static inline void SetPixelCbTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[CbPixelChannel].traits=traits;
}

static inline void SetPixelChannel(const Image *image,
  const PixelChannel channel,const Quantum quantum,Quantum *pixel)
{
  pixel[image->channel_map[channel].channel]=quantum;
}

static inline void SetPixelChannelMapChannel(const Image *image,
  const PixelChannel channel,const PixelChannel channels)
{
  image->channel_map[channel].channel=channels;
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

static inline void SetPixelCr(const Image *image,const Quantum cr,
  Quantum *pixel)
{
  pixel[image->channel_map[CrPixelChannel].channel]=cr;
}

static inline void SetPixelCrTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[CrPixelChannel].traits=traits;
}

static inline void SetPixelCyan(const Image *image,const Quantum cyan,
  Quantum *pixel)
{
  pixel[image->channel_map[CyanPixelChannel].channel]=cyan;
}

static inline void SetPixelGray(const Image *image,const Quantum gray,
  Quantum *pixel)
{
  pixel[image->channel_map[GrayPixelChannel].channel]=gray;
}

static inline void SetPixelGrayTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[GrayPixelChannel].traits=traits;
}

static inline void SetPixelGreen(const Image *image,const Quantum green,
  Quantum *pixel)
{
  pixel[image->channel_map[GreenPixelChannel].channel]=green;
}

static inline void SetPixelGreenTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[GreenPixelChannel].traits=traits;
}

static inline void SetPixelIndex(const Image *image,const Quantum index,
  Quantum *pixel)
{
  pixel[image->channel_map[IndexPixelChannel].channel]=index;
}

static inline void SetPixelIndexTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[IndexPixelChannel].traits=traits;
}

static inline void SetPixelInfo(const Image *image,const Quantum *pixel,
  PixelInfo *pixel_info)
{
  pixel_info->red=(MagickRealType)
    pixel[image->channel_map[RedPixelChannel].channel];
  pixel_info->green=(MagickRealType)
    pixel[image->channel_map[GreenPixelChannel].channel];
  pixel_info->blue=(MagickRealType)
    pixel[image->channel_map[BluePixelChannel].channel];
  if (image->colorspace == CMYKColorspace)
    pixel_info->black=(MagickRealType)
      pixel[image->channel_map[BlackPixelChannel].channel];
  pixel_info->alpha=(MagickRealType)
    pixel[image->channel_map[AlphaPixelChannel].channel];
  if (image->storage_class == PseudoClass)
    pixel_info->index=(MagickRealType)
      pixel[image->channel_map[IndexPixelChannel].channel];
}

static inline void SetPixelInfoBias(const Image *image,PixelInfo *pixel_info)
{
  /*
    Obsoleted by MorphologyApply().
  */
  pixel_info->red=image->bias;
  pixel_info->green=image->bias;
  pixel_info->blue=image->bias;
  pixel_info->alpha=image->bias;
  pixel_info->black=image->bias;
}

static inline void SetPixelInfoPacket(const Image *image,
  const PixelPacket *pixel,PixelInfo *pixel_info)
{
  pixel_info->red=(MagickRealType) pixel->red;
  pixel_info->green=(MagickRealType) pixel->green;
  pixel_info->blue=(MagickRealType) pixel->blue;
  pixel_info->alpha=(MagickRealType) pixel->alpha;
  if (image->colorspace == CMYKColorspace)
    pixel_info->black=(MagickRealType) pixel->black;
  if (image->storage_class == PseudoClass)
    pixel_info->index=(MagickRealType) pixel->index;
}

static inline void SetPixelMagenta(const Image *image,const Quantum magenta,
  Quantum *pixel)
{
  pixel[image->channel_map[MagentaPixelChannel].channel]=magenta;
}

static inline void SetPixelMagentaTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[MagentaPixelChannel].traits=traits;
}

static inline void SetPixelMetaChannels(Image *image,
  const size_t number_meta_channels)
{
  image->number_meta_channels=number_meta_channels;
}

static inline void SetPixelMetacontentExtent(Image *image,const size_t extent)
{
  image->metacontent_extent=extent;
}

static inline void SetPixelRed(const Image *image,const Quantum red,
  Quantum *pixel)
{
  pixel[image->channel_map[RedPixelChannel].channel]=red;
}

static inline void SetPixelRedTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[RedPixelChannel].traits=traits;
}

static inline void SetPixelPacket(const Image *image,const PixelPacket *packet,
  Quantum *pixel)
{
  pixel[image->channel_map[RedPixelChannel].channel]=
    ClampToQuantum(packet->red);
  pixel[image->channel_map[GreenPixelChannel].channel]=
    ClampToQuantum(packet->green);
  pixel[image->channel_map[BluePixelChannel].channel]=
    ClampToQuantum(packet->blue);
  pixel[image->channel_map[AlphaPixelChannel].channel]=
    ClampToQuantum(packet->alpha);
}

static inline void SetPixelPixelInfo(const Image *image,
  const PixelInfo *pixel_info,Quantum *pixel)
{
  pixel[image->channel_map[RedPixelChannel].channel]=
    ClampToQuantum(pixel_info->red);
  pixel[image->channel_map[GreenPixelChannel].channel]=
    ClampToQuantum(pixel_info->green);
  pixel[image->channel_map[BluePixelChannel].channel]=
    ClampToQuantum(pixel_info->blue);
  pixel[image->channel_map[AlphaPixelChannel].channel]=
    ClampToQuantum(pixel_info->alpha);
  if (image->colorspace == CMYKColorspace)
    pixel[image->channel_map[BlackPixelChannel].channel]=
      ClampToQuantum(pixel_info->black);
}

static inline void SetPixelYellow(const Image *image,const Quantum yellow,
  Quantum *pixel)
{
  pixel[image->channel_map[YellowPixelChannel].channel]=yellow;
}

static inline void SetPixelYellowTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[YellowPixelChannel].traits=traits;
}

static inline void SetPixelY(const Image *image,const Quantum y,Quantum *pixel)
{
  pixel[image->channel_map[YPixelChannel].channel]=y;
}

static inline void SetPixelYTraits(Image *image,const PixelTrait traits)
{
  image->channel_map[YPixelChannel].traits=traits;
}

static inline Quantum GetPixelIntensity(const Image *image,const Quantum *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((pixel[image->channel_map[RedPixelChannel].channel] ==
       pixel[image->channel_map[GreenPixelChannel].channel]) &&
      (pixel[image->channel_map[GreenPixelChannel].channel] ==
       pixel[image->channel_map[BluePixelChannel].channel]))
    return(pixel[image->channel_map[RedPixelChannel].channel]);
  return((Quantum) (0.299*pixel[image->channel_map[RedPixelChannel].channel]+
    0.587*pixel[image->channel_map[GreenPixelChannel].channel]+0.114*
    pixel[image->channel_map[BluePixelChannel].channel]+0.5));
#else
  {
    double
      alpha,
      beta;

    alpha=pixel[image->channel_map[RedPixelChannel].channel]-(double)
      pixel[image->channel_map[GreenPixelChannel].channel];
    beta=pixel[image->channel_map[GreenPixelChannel].channel]-(double)
      pixel[image->channel_map[BluePixelChannel].channel];
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(pixel[image->channel_map[RedPixelChannel].channel]);
    return((Quantum) (0.299*pixel[image->channel_map[RedPixelChannel].channel]+
      0.587*pixel[image->channel_map[GreenPixelChannel].channel]+0.114*
      pixel[image->channel_map[BluePixelChannel].channel]));
  }
#endif
}

static inline Quantum GetPixelLuminance(const Image *image,const Quantum *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (0.21267*pixel[image->channel_map[RedPixelChannel].channel]+
    0.71516*pixel[image->channel_map[GreenPixelChannel].channel]+0.07217*
    pixel[image->channel_map[BluePixelChannel].channel]+0.5));
#else
  return((Quantum) (0.21267*pixel[image->channel_map[RedPixelChannel].channel]+
    0.71516*pixel[image->channel_map[GreenPixelChannel].channel]+0.07217*
    pixel[image->channel_map[BluePixelChannel].channel]));
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

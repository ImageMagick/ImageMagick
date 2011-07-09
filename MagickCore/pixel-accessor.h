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

static inline Quantum GetPixelAlpha(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][AlphaPixelComponent].component]);
}

static inline PixelTrait GetPixelAlphaTraits(const Image *image)
{
  return(image->component_map[image->map][AlphaPixelComponent].traits);
}

static inline Quantum GetPixelBlack(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][BlackPixelComponent].component]);
}

static inline PixelTrait GetPixelBlackTraits(const Image *image)
{
  return(image->component_map[image->map][BlackPixelComponent].traits);
}

static inline Quantum GetPixelBlue(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][BluePixelComponent].component]);
}

static inline PixelTrait GetPixelBlueTraits(const Image *image)
{
  return(image->component_map[image->map][BluePixelComponent].traits);
}

static inline Quantum GetPixelCb(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][CbPixelComponent].component]);
}

static inline PixelTrait GetPixelCbTraits(const Image *image)
{
  return(image->component_map[image->map][CbPixelComponent].traits);
}

static inline Quantum GetPixelComponent(const Image *image,
  const PixelComponent component)
{
  return(image->component_map[image->map][component].component);
}

static inline size_t GetPixelComponents(const Image *image)
{
  return(image->pixel_components);
}

static inline Quantum GetPixelCr(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][CrPixelComponent].component]);
}

static inline PixelTrait GetPixelCrTraits(const Image *image)
{
  return(image->component_map[image->map][CrPixelComponent].traits);
}

static inline Quantum GetPixelCyan(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][CyanPixelComponent].component]);
}

static inline PixelTrait GetPixelCyanTraits(const Image *image)
{
  return(image->component_map[image->map][CyanPixelComponent].traits);
}

static inline Quantum GetPixelGray(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][GrayPixelComponent].component]);
}

static inline PixelTrait GetPixelGrayTraits(const Image *image)
{
  return(image->component_map[image->map][GrayPixelComponent].traits);
}

static inline Quantum GetPixelGreen(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][GreenPixelComponent].component]);
}

static inline PixelTrait GetPixelGreenTraits(const Image *image)
{
  return(image->component_map[image->map][GreenPixelComponent].traits);
}

static inline Quantum GetPixelIndex(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][IndexPixelComponent].component]);
}

static inline PixelTrait GetPixelIndexTraits(const Image *image)
{
  return(image->component_map[image->map][IndexPixelComponent].traits);
}

static inline Quantum GetPixelInfoIntensity(const PixelInfo *pixel_info)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (0.299*pixel_info->red+0.587*pixel_info->green+0.114*pixel_info->blue+0.5));
#else
  return((Quantum) (0.299*pixel_info->red+0.587*pixel_info->green+0.114*pixel_info->blue));
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

static inline Quantum GetPixelMagenta(const Image *image,
  const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][MagentaPixelComponent].component]);
}

static inline PixelTrait GetPixelMagentaTraits(const Image *image)
{
  return(image->component_map[image->map][MagentaPixelComponent].traits);
}

static inline size_t GetPixelMetacontentExtent(const Image *image)
{
  return(image->metacontent_extent);
}

static inline Quantum GetPixelRed(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][RedPixelComponent].component]);
}

static inline PixelTrait GetPixelRedTraits(const Image *image)
{
  return(image->component_map[image->map][RedPixelComponent].traits);
}

static inline void GetPixelPacket(const Image *image,const Quantum *pixel,
  PixelPacket *packet)
{
  packet->red=GetPixelRed(image,pixel);
  packet->green=GetPixelGreen(image,pixel);
  packet->blue=GetPixelBlue(image,pixel);
  packet->alpha=GetPixelAlpha(image,pixel);
}

static inline Quantum GetPixelPacketIntensity(const PixelPacket *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((pixel->red == pixel->green) && (pixel->green == pixel->blue))
    return(pixel->red);
  return((Quantum) (0.299*pixel->red+0.587*pixel->green+0.114*pixel->blue+0.5));
#else
  {
    double
      alpha,
      beta;

    alpha=pixel->red-pixel->green;
    beta=pixel->green-pixel->blue;
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(pixel->red);
    return((Quantum) (0.299*pixel->red+0.587*pixel->green+0.114*pixel->blue));
  }
#endif
}

static inline PixelTrait GetPixelComponentTraits(const Image *image,
  const PixelComponent component)
{
  return(image->component_map[image->map][component].traits);
}

static inline Quantum GetPixelY(const Image *image,const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][YPixelComponent].component]);
}

static inline PixelTrait GetPixelYTraits(const Image *image)
{
  return(image->component_map[image->map][YPixelComponent].traits);
}

static inline Quantum GetPixelYellow(const Image *image,
  const Quantum *pixel)
{
  return(pixel[image->component_map[image->map][YellowPixelComponent].component]);
}

static inline PixelTrait GetPixelYellowTraits(const Image *image)
{
  return(image->component_map[image->map][YellowPixelComponent].traits);
}

static inline MagickBooleanType IsPixelEquivalent(const Image *image,
  const Quantum *p,const PixelPacket *q)
{
  if ((GetPixelRed(image,p) == q->red) &&
      (GetPixelGreen(image,p) == q->green) &&
      (GetPixelBlue(image,p) == q->blue))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelGray(const Image *image,
  const Quantum *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((GetPixelRed(image,pixel) == GetPixelGreen(image,pixel)) &&
      (GetPixelGreen(image,pixel) == GetPixelBlue(image,pixel)))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=GetPixelRed(image,pixel)-(double) GetPixelGreen(image,pixel);
    beta=GetPixelGreen(image,pixel)-(double) GetPixelBlue(image,pixel);
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
  }
#endif
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelInfoEquivalent(const PixelInfo *p,
  const PixelInfo *q)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((p->matte != MagickFalse) && (q->matte == MagickFalse) &&
      (p->alpha != OpaqueAlpha))
    return(MagickFalse);
  if ((q->matte != MagickFalse) && (p->matte == MagickFalse) &&
      (q->alpha != OpaqueAlpha))
    return(MagickFalse);
  if ((p->matte != MagickFalse) && (q->matte != MagickFalse))
    {
      if (p->alpha != q->alpha)
        return(MagickFalse);
      if (p->alpha == TransparentAlpha)
        return(MagickTrue);
    }
  if (p->red != q->red)
    return(MagickFalse);
  if (p->green != q->green)
    return(MagickFalse);
  if (p->blue != q->blue)
    return(MagickFalse);
  if ((p->colorspace == CMYKColorspace) && (p->black != q->black))
    return(MagickFalse);
#else
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
#endif
  return(MagickTrue);
}

static inline MagickBooleanType IsPixelMonochrome(const Image *image,
  const Quantum *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if (((GetPixelRed(image,pixel) == 0) ||
      (GetPixelRed(image,pixel) == (Quantum) QuantumRange)) &&
      (GetPixelRed(image,pixel) == GetPixelGreen(image,pixel)) &&
      (GetPixelGreen(image,pixel) == GetPixelBlue(image,pixel)))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=GetPixelRed(image,pixel)-(double) GetPixelGreen(image,pixel);
    beta=GetPixelGreen(image,pixel)-(double) GetPixelBlue(image,pixel);
    if (((fabs(GetPixelRed(image,pixel)) <= MagickEpsilon) ||
         (fabs(GetPixelRed(image,pixel)-QuantumRange) <= MagickEpsilon)) &&
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
  packet->red=ClampToQuantum(pixel_info->red);
  packet->green=ClampToQuantum(pixel_info->green);
  packet->blue=ClampToQuantum(pixel_info->blue);
  packet->alpha=ClampToQuantum(pixel_info->alpha);
  if (image->colorspace == CMYKColorspace)
    packet->black=ClampToQuantum(pixel_info->black);
  if (image->storage_class == PseudoClass)
    packet->index=ClampToQuantum(pixel_info->index);
}

static inline void SetPixelAlpha(const Image *image,const Quantum alpha,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][AlphaPixelComponent].component]=alpha;
}

static inline void SetPixelAlphaTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][AlphaPixelComponent].traits=traits;
}

static inline void SetPixelBlack(const Image *image,const Quantum black,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][BlackPixelComponent].component]=black;
}

static inline void SetPixelBlackTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][BlackPixelComponent].traits=traits;
}

static inline void SetPixelBlue(const Image *image,const Quantum blue,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][BluePixelComponent].component]=blue;
}

static inline void SetPixelBlueTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][BluePixelComponent].traits=traits;
}

static inline void SetPixelCb(const Image *image,const Quantum cb,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][CbPixelComponent].component]=cb;
}

static inline void SetPixelCbTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][CbPixelComponent].traits=traits;
}

static inline void SetPixelComponent(const Image *image,
  const PixelComponent component,const PixelComponent components)
{
  image->component_map[image->map][component].component=components;
}

static inline void SetPixelComponents(Image *image,const size_t channels)
{
  image->pixel_components=channels;
}

static inline void SetPixelComponentTraits(Image *image,
  const PixelComponent component,const PixelTrait traits)
{
  image->component_map[image->map][component].traits=traits;
}

static inline void SetPixelCr(const Image *image,const Quantum cr,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][CrPixelComponent].component]=cr;
}

static inline void SetPixelCrTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][CrPixelComponent].traits=traits;
}

static inline void SetPixelCyan(const Image *image,const Quantum cyan,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][CyanPixelComponent].component]=cyan;
}

static inline void SetPixelGray(const Image *image,const Quantum gray,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][GrayPixelComponent].component]=gray;
}

static inline void SetPixelGrayTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][GrayPixelComponent].traits=traits;
}

static inline void SetPixelGreen(const Image *image,const Quantum green,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][GreenPixelComponent].component]=green;
}

static inline void SetPixelGreenTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][GreenPixelComponent].traits=traits;
}

static inline void SetPixelIndex(const Image *image,const Quantum index,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][IndexPixelComponent].component]=index;
}

static inline void SetPixelIndexTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][IndexPixelComponent].traits=traits;
}

static inline void SetPixelInfo(const Image *image,const Quantum *pixel,
  PixelInfo *pixel_info)
{
  pixel_info->red=(MagickRealType) GetPixelRed(image,pixel);
  pixel_info->green=(MagickRealType) GetPixelGreen(image,pixel);
  pixel_info->blue=(MagickRealType) GetPixelBlue(image,pixel);
  pixel_info->alpha=(MagickRealType) GetPixelAlpha(image,pixel);
  if (image->colorspace == CMYKColorspace)
    pixel_info->black=(MagickRealType) GetPixelBlack(image,pixel);
  if (image->storage_class == PseudoClass)
    pixel_info->index=(MagickRealType) GetPixelIndex(image,pixel);
}

static inline void SetPixelInfoBias(const Image *image,
  PixelInfo *pixel_info)
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
  pixel[image->component_map[image->map][MagentaPixelComponent].component]=magenta;
}

static inline void SetPixelMagentaTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][MagentaPixelComponent].traits=traits;
}

static inline void SetPixelMetacontentExtent(Image *image,const size_t extent)
{
  image->metacontent_extent=extent;
}

static inline void SetPixelRed(const Image *image,const Quantum red,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][RedPixelComponent].component]=red;
}

static inline void SetPixelRedTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][RedPixelComponent].traits=traits;
}

static inline void SetPixelPacket(const Image *image,const PixelPacket *packet,
  Quantum *pixel)
{
  SetPixelRed(image,packet->red,pixel);
  SetPixelGreen(image,packet->green,pixel);
  SetPixelBlue(image,packet->blue,pixel);
  SetPixelAlpha(image,packet->alpha,pixel);
}

static inline void SetPixelPixelInfo(const Image *image,
  const PixelInfo *pixel_info,Quantum *packet)
{
  SetPixelRed(image,ClampToQuantum(pixel_info->red),packet);
  SetPixelGreen(image,ClampToQuantum(pixel_info->green),packet);
  SetPixelBlue(image,ClampToQuantum(pixel_info->blue),packet);
  SetPixelAlpha(image,ClampToQuantum(pixel_info->alpha),packet);
  if (image->colorspace == CMYKColorspace)
    SetPixelBlack(image,ClampToQuantum(pixel_info->black),packet);
}

static inline void SetPixelYellow(const Image *image,const Quantum yellow,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][YellowPixelComponent].component]=yellow;
}

static inline void SetPixelYellowTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][YellowPixelComponent].traits=traits;
}

static inline void SetPixelY(const Image *image,const Quantum y,
  Quantum *pixel)
{
  pixel[image->component_map[image->map][YPixelComponent].component]=y;
}

static inline void SetPixelYTraits(Image *image,const PixelTrait traits)
{
  image->component_map[image->map][YPixelComponent].traits=traits;
}

static inline Quantum GetPixelIntensity(const Image *image,
  const Quantum *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((GetPixelRed(image,pixel) == GetPixelGreen(image,pixel)) &&
      (GetPixelGreen(image,pixel) == GetPixelBlue(image,pixel)))
    return(GetPixelRed(image,pixel));
  return((Quantum) (0.299*GetPixelRed(image,pixel)+0.587*
    GetPixelGreen(image,pixel)+0.114*GetPixelBlue(image,pixel)+0.5));
#else
  {
    double
      alpha,
      beta;

    alpha=GetPixelRed(image,pixel)-(double) GetPixelGreen(image,pixel);
    beta=GetPixelGreen(image,pixel)-(double) GetPixelBlue(image,pixel);
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(GetPixelRed(image,pixel));
    return((Quantum) (0.299*GetPixelRed(image,pixel)+0.587*
      GetPixelGreen(image,pixel)+0.114*GetPixelBlue(image,pixel)));
  }
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

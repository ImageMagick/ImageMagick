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

  MagickCore image pixel private methods.
*/
#ifndef _MAGICKCORE_PIXEL_PRIVATE_H
#define _MAGICKCORE_PIXEL_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/exception-private.h>
#include <magick/image.h>
#include <magick/color.h>
#include <magick/image-private.h>
#include <magick/quantum-private.h>

static inline MagickPixelPacket *CloneMagickPixelPacket(
  const MagickPixelPacket *pixel)
{
  MagickPixelPacket
    *clone_pixel;

  clone_pixel=(MagickPixelPacket *) AcquireAlignedMemory(1,
    sizeof(*clone_pixel));
  if (clone_pixel == (MagickPixelPacket *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  *clone_pixel=(*pixel);
  return(clone_pixel);
}

static inline MagickBooleanType IsGrayPixel(const PixelPacket *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((GetRedPixelComponent(pixel) == GetGreenPixelComponent(pixel)) && 
      (GetGreenPixelComponent(pixel) == GetBluePixelComponent(pixel)))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=GetRedPixelComponent(pixel)-GetGreenPixelComponent(pixel);
    beta=GetGreenPixelComponent(pixel)-GetBluePixelComponent(pixel);
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
  }
#endif
  return(MagickFalse);
}

static inline MagickBooleanType IsMonochromePixel(const PixelPacket *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if (((GetRedPixelComponent(pixel) == 0) ||
       (GetRedPixelComponent(pixel) == (Quantum) QuantumRange)) &&
      (GetRedPixelComponent(pixel) == GetGreenPixelComponent(pixel)) &&
      (GetGreenPixelComponent(pixel) == GetBluePixelComponent(pixel)))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=GetRedPixelComponent(pixel)-GetGreenPixelComponent(pixel);
    beta=GetGreenPixelComponent(pixel)-GetBluePixelComponent(pixel);
    if (((fabs(GetRedPixelComponent(pixel)) <= MagickEpsilon) ||
         (fabs(GetRedPixelComponent(pixel)-QuantumRange) <= MagickEpsilon)) &&
        (fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
    }
#endif
  return(MagickFalse);
}

static inline void SetMagickPixelPacket(const Image *image,
  const PixelPacket *color,const IndexPacket *index,MagickPixelPacket *pixel)
{
  pixel->red=(MagickRealType) GetRedPixelComponent(color);
  pixel->green=(MagickRealType) GetGreenPixelComponent(color);
  pixel->blue=(MagickRealType) GetBluePixelComponent(color);
  pixel->opacity=(MagickRealType) GetOpacityPixelComponent(color);
  if ((image->colorspace == CMYKColorspace) &&
      (index != (const IndexPacket *) NULL))
    pixel->index=(MagickRealType) GetIndexPixelComponent(index);
}

static inline void SetMagickPixelPacketBias(const Image *image,
  MagickPixelPacket *pixel)
{
  /*
    Obsoleted by MorphologyApply().
  */
  pixel->red=image->bias;
  pixel->green=image->bias;
  pixel->blue=image->bias;
  pixel->opacity=image->bias;
  pixel->index=image->bias;
}

static inline void SetPixelPacket(const Image *image,
  const MagickPixelPacket *pixel,PixelPacket *color,IndexPacket *index)
{
  SetRedPixelComponent(color,ClampToQuantum(pixel->red));
  SetGreenPixelComponent(color,ClampToQuantum(pixel->green));
  SetBluePixelComponent(color,ClampToQuantum(pixel->blue));
  SetOpacityPixelComponent(color,ClampToQuantum(pixel->opacity));
  if ((image->colorspace == CMYKColorspace) ||
      (image->storage_class == PseudoClass))
    SetIndexPixelComponent(index,ClampToQuantum(pixel->index));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

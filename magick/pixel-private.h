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

  MagickCore image pixel private methods.
*/
#ifndef _MAGICKCORE_PIXEL_PRIVATE_H
#define _MAGICKCORE_PIXEL_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/image.h"
#include "magick/color.h"
#include "magick/image-private.h"
#include "magick/memory_.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"

static inline MagickBooleanType IsGrayPixel(const PixelPacket *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((GetPixelRed(pixel) == GetPixelGreen(pixel)) && 
      (GetPixelGreen(pixel) == GetPixelBlue(pixel)))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=GetPixelRed(pixel)-(double) GetPixelGreen(pixel);
    beta=GetPixelGreen(pixel)-(double) GetPixelBlue(pixel);
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
  }
#endif
  return(MagickFalse);
}

static inline MagickBooleanType IsMonochromePixel(const PixelPacket *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if (((GetPixelRed(pixel) == 0) ||
       (GetPixelRed(pixel) == QuantumRange)) &&
      (GetPixelRed(pixel) == GetPixelGreen(pixel)) &&
      (GetPixelGreen(pixel) == GetPixelBlue(pixel)))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=GetPixelRed(pixel)-(double) GetPixelGreen(pixel);
    beta=GetPixelGreen(pixel)-(double) GetPixelBlue(pixel);
    if (((fabs((double) GetPixelRed(pixel)) <= MagickEpsilon) ||
         (fabs((double) GetPixelRed(pixel)-QuantumRange) <= MagickEpsilon)) &&
        (fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
    }
#endif
  return(MagickFalse);
}

static inline double MagickEpsilonReciprocal(const double x)
{
  double
    sign;

  sign=x < 0.0 ? -1.0 : 1.0;
  if ((sign*x) >= MagickEpsilon)
    return(1.0/x);
  return(sign/MagickEpsilon);
}

static inline void SetMagickPixelPacket(const Image *image,
  const PixelPacket *color,const IndexPacket *index,MagickPixelPacket *pixel)
{
  pixel->red=(MagickRealType) GetPixelRed(color);
  pixel->green=(MagickRealType) GetPixelGreen(color);
  pixel->blue=(MagickRealType) GetPixelBlue(color);
  pixel->opacity=(MagickRealType) GetPixelOpacity(color);
  if ((image->colorspace == CMYKColorspace) &&
      (index != (const IndexPacket *) NULL))
    pixel->index=(MagickRealType) GetPixelIndex(index);
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
  SetPixelRed(color,ClampToQuantum(pixel->red));
  SetPixelGreen(color,ClampToQuantum(pixel->green));
  SetPixelBlue(color,ClampToQuantum(pixel->blue));
  SetPixelOpacity(color,ClampToQuantum(pixel->opacity));
  if ((image->colorspace == CMYKColorspace) ||
      (image->storage_class == PseudoClass))
    SetPixelIndex(index,ClampToQuantum(pixel->index));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

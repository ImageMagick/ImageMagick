/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image colorspace private methods.
*/
#ifndef _MAGICKCORE_COLORSPACE_PRIVATE_H
#define _MAGICKCORE_COLORSPACE_PRIVATE_H

#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/pixel.h"
#include "magick/pixel-accessor.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline void ConvertRGBToCMYK(MagickPixelPacket *pixel)
{
  MagickRealType
    black,
    blue,
    cyan,
    green,
    magenta,
    red,
    yellow;

  if (pixel->colorspace != sRGBColorspace)
    {
      red=QuantumScale*pixel->red;
      green=QuantumScale*pixel->green;
      blue=QuantumScale*pixel->blue;
    }
  else
    {
      red=DecodePixelGamma(pixel->red);
      green=DecodePixelGamma(pixel->green);
      blue=DecodePixelGamma(pixel->blue);
    }
  if ((fabs(red) < MagickEpsilon) && (fabs(green) < MagickEpsilon) &&
      (fabs(blue) < MagickEpsilon))
    {
      pixel->index=(MagickRealType) QuantumRange;
      return;
    }
  cyan=(MagickRealType) (1.0-red);
  magenta=(MagickRealType) (1.0-green);
  yellow=(MagickRealType) (1.0-blue);
  black=cyan;
  if (magenta < black)
    black=magenta;
  if (yellow < black)
    black=yellow;
  cyan=(MagickRealType) ((cyan-black)/(1.0-black));
  magenta=(MagickRealType) ((magenta-black)/(1.0-black));
  yellow=(MagickRealType) ((yellow-black)/(1.0-black));
  pixel->colorspace=CMYKColorspace;
  pixel->red=QuantumRange*cyan;
  pixel->green=QuantumRange*magenta;
  pixel->blue=QuantumRange*yellow;
  pixel->index=QuantumRange*black;
}

static inline MagickBooleanType IsCMYKColorspace(
  const ColorspaceType colorspace)
{
  if (colorspace == CMYKColorspace)
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsGrayColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == GRAYColorspace) || (colorspace == Rec601LumaColorspace) ||
      (colorspace == Rec709LumaColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsRGBColorspace(const ColorspaceType colorspace)
{
  if ((colorspace == RGBColorspace) || (colorspace == scRGBColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IssRGBColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == sRGBColorspace) || (colorspace == TransparentColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IssRGBCompatibleColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == sRGBColorspace) || (colorspace == RGBColorspace) ||
      (colorspace == scRGBColorspace) ||
      (colorspace == TransparentColorspace) ||
      (IsGrayColorspace(colorspace) != MagickFalse))
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

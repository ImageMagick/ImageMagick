/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image colorspace private methods.
*/
#ifndef MAGICKCORE_COLORSPACE_PRIVATE_H
#define MAGICKCORE_COLORSPACE_PRIVATE_H

#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline void ConvertCMYKToRGB(PixelInfo *pixel)
{
  pixel->red=((QuantumRange-(QuantumScale*pixel->red*(QuantumRange-
    pixel->black)+pixel->black)));
  pixel->green=((QuantumRange-(QuantumScale*pixel->green*(QuantumRange-
    pixel->black)+pixel->black)));
  pixel->blue=((QuantumRange-(QuantumScale*pixel->blue*(QuantumRange-
    pixel->black)+pixel->black)));
}

static inline void ConvertRGBToCMYK(PixelInfo *pixel)
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
      red=QuantumScale*DecodePixelGamma(pixel->red);
      green=QuantumScale*DecodePixelGamma(pixel->green);
      blue=QuantumScale*DecodePixelGamma(pixel->blue);
    }
  if ((fabs((double) red) < MagickEpsilon) &&
      (fabs((double) green) < MagickEpsilon) &&
      (fabs((double) blue) < MagickEpsilon))
    {
      pixel->black=(MagickRealType) QuantumRange;
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
  cyan=(MagickRealType) (PerceptibleReciprocal(1.0-black)*(cyan-black));
  magenta=(MagickRealType) (PerceptibleReciprocal(1.0-black)*(magenta-black));
  yellow=(MagickRealType) (PerceptibleReciprocal(1.0-black)*(yellow-black));
  pixel->colorspace=CMYKColorspace;
  pixel->red=QuantumRange*cyan;
  pixel->green=QuantumRange*magenta;
  pixel->blue=QuantumRange*yellow;
  pixel->black=QuantumRange*black;
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
  if ((colorspace == LinearGRAYColorspace) || (colorspace == GRAYColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsGrayImageType(const ImageType type)
{
  if ((type == GrayscaleType) || (type == GrayscaleAlphaType) ||
      (type == BilevelType))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsHueCompatibleColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == HCLColorspace) || (colorspace == HCLpColorspace) ||
      (colorspace == HSBColorspace) || (colorspace == HSIColorspace) ||
      (colorspace == HSLColorspace) || (colorspace == HSVColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsRGBColorspace(const ColorspaceType colorspace)
{
  if ((colorspace == RGBColorspace) || (colorspace == scRGBColorspace) ||
      (colorspace == LinearGRAYColorspace))
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
      (colorspace == Adobe98Colorspace) || (colorspace == ProPhotoColorspace) ||
      (colorspace == DisplayP3Colorspace) || (colorspace == scRGBColorspace) ||
      (colorspace == TransparentColorspace) || (colorspace == GRAYColorspace) ||
      (colorspace == LinearGRAYColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsYCbCrCompatibleColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == YCbCrColorspace) ||
      (colorspace == Rec709YCbCrColorspace) ||
      (colorspace == Rec601YCbCrColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

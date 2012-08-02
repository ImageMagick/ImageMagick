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

  MagickCore image color methods.
*/
#ifndef _MAGICKCORE_COLOR_PRIVATE_H
#define _MAGICKCORE_COLOR_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/image.h>
#include <magick/color.h>
#include <magick/exception-private.h>
#include <magick/pixel-private.h>

static inline MagickBooleanType IsColorEqual(const PixelPacket *p,
  const PixelPacket *q)
{
  double
    blue,
    green,
    red;

  red=(double) p->red;
  green=(double) p->green;
  blue=(double) p->blue;
  if ((fabs(red-q->red) < MagickEpsilon) &&
      (fabs(green-q->green) < MagickEpsilon) &&
      (fabs(blue-q->blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsMagickColorEqual(const MagickPixelPacket *p,
  const MagickPixelPacket *q)
{
  if ((p->matte != MagickFalse) && (q->matte == MagickFalse) &&
      (fabs(p->opacity-OpaqueOpacity) >= MagickEpsilon))
    return(MagickFalse);
  if ((q->matte != MagickFalse) && (p->matte == MagickFalse) &&
      (fabs(q->opacity-OpaqueOpacity)) >= MagickEpsilon)
    return(MagickFalse);
  if ((p->matte != MagickFalse) && (q->matte != MagickFalse))
    {
      if (fabs(p->opacity-q->opacity) >= MagickEpsilon)
        return(MagickFalse);
      if (fabs(p->opacity-TransparentOpacity) < MagickEpsilon)
        return(MagickTrue);
    }
  if (fabs(p->red-q->red) >= MagickEpsilon)
    return(MagickFalse);
  if (fabs(p->green-q->green) >= MagickEpsilon)
    return(MagickFalse);
  if (fabs(p->blue-q->blue) >= MagickEpsilon)
    return(MagickFalse);
  if ((p->colorspace == CMYKColorspace) &&
      (fabs(p->index-q->index) >= MagickEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static inline MagickBooleanType IsMagickGray(const MagickPixelPacket *pixel)
{
  if ((pixel->colorspace != GRAYColorspace) &&
      (pixel->colorspace != RGBColorspace))
    return(MagickFalse);
  if ((fabs(pixel->red-pixel->green) < MagickEpsilon) &&
      (fabs(pixel->green-pixel->blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelGray(const PixelPacket *pixel)
{
  double
    blue,
    green,
    red;

  red=(double) pixel->red;
  green=(double) pixel->green;
  blue=(double) pixel->blue;
  if ((fabs(red-green) < MagickEpsilon) && (fabs(green-blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickRealType MagickPixelIntensity(
  const MagickPixelPacket *pixel)
{
  double
    blue,
    green,
    red;

  if (pixel->colorspace == GRAYColorspace)
    return(pixel->red);
  if (pixel->colorspace != sRGBColorspace)
    return(0.298839*pixel->red+0.586811*pixel->green+0.114350*pixel->blue);
  red=QuantumRange*DecompandsRGB(QuantumScale*pixel->red);
  green=QuantumRange*DecompandsRGB(QuantumScale*pixel->green);
  blue=QuantumRange*DecompandsRGB(QuantumScale*pixel->blue);
  return(0.298839*red+0.586811*green+0.114350*blue);
}

static inline Quantum MagickPixelIntensityToQuantum(
  const MagickPixelPacket *pixel)
{
  double
    blue,
    green,
    red;

  if (pixel->colorspace == GRAYColorspace)
    return(ClampToQuantum(pixel->red));
  if (pixel->colorspace != sRGBColorspace)
    return(ClampToQuantum(0.298839*pixel->red+0.586811*pixel->green+
      0.114350*pixel->blue));
  red=QuantumRange*DecompandsRGB(QuantumScale*pixel->red);
  green=QuantumRange*DecompandsRGB(QuantumScale*pixel->green);
  blue=QuantumRange*DecompandsRGB(QuantumScale*pixel->blue);
  return(ClampToQuantum(0.298839*red+0.586811*green+0.114350*blue));
}

static inline MagickRealType MagickPixelLuminance(
  const MagickPixelPacket *pixel)
{
  double
    blue,
    green,
    red;

  if (pixel->colorspace == GRAYColorspace)
    return(pixel->red);
  if (pixel->colorspace != sRGBColorspace)
    return(0.21267*pixel->red+0.71516*pixel->green+0.07217*pixel->blue);
  red=QuantumRange*DecompandsRGB(QuantumScale*pixel->red);
  green=QuantumRange*DecompandsRGB(QuantumScale*pixel->green);
  blue=QuantumRange*DecompandsRGB(QuantumScale*pixel->blue);
  return(0.21267*red+0.71516*green+0.07217*blue);
}

static inline MagickRealType PixelIntensity(const Image *image,
  const PixelPacket *pixel)
{
  double
    blue,
    green,
    red;

  MagickRealType
    intensity;

  if (image->colorspace == GRAYColorspace)
    return((MagickRealType) pixel->red);
  if (image->colorspace != sRGBColorspace)
    return(0.298839*pixel->red+0.586811*pixel->green+0.114350*pixel->blue);
  red=QuantumRange*DecompandsRGB(QuantumScale*pixel->red);
  green=QuantumRange*DecompandsRGB(QuantumScale*pixel->green);
  blue=QuantumRange*DecompandsRGB(QuantumScale*pixel->blue);
  intensity=(MagickRealType) (0.298839*red+0.586811*green+0.114350*blue);
  return(intensity);
}

static inline Quantum PixelPacketIntensity(const PixelPacket *pixel)
{
  double
    blue,
    green,
    red;

  red=QuantumRange*DecompandsRGB(QuantumScale*pixel->red);
  green=QuantumRange*DecompandsRGB(QuantumScale*pixel->green);
  blue=QuantumRange*DecompandsRGB(QuantumScale*pixel->blue);
  return(ClampToQuantum(0.298839*red+0.586811*green+0.114350*blue));
}

static inline Quantum PixelIntensityToQuantum(const Image *restrict image,
  const PixelPacket *restrict pixel)
{
  double
    blue,
    green,
    red;

  if (image->colorspace == GRAYColorspace)
    return(GetPixelGray(pixel));
  if (image->colorspace != sRGBColorspace)
    return(ClampToQuantum(0.298839*pixel->red+0.586811*pixel->green+0.114350*
      pixel->blue));
  red=QuantumRange*DecompandsRGB(QuantumScale*pixel->red);
  green=QuantumRange*DecompandsRGB(QuantumScale*pixel->green);
  blue=QuantumRange*DecompandsRGB(QuantumScale*pixel->blue);
  return(ClampToQuantum(0.298839*red+0.586811*green+0.114350*blue));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

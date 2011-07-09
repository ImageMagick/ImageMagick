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

static inline MagickBooleanType IsColorEqual(const PixelPacket *p,
  const PixelPacket *q)
{
  if ((GetPixelRed(p) == GetPixelRed(q)) &&
      (GetPixelGreen(p) == GetPixelGreen(q)) &&
      (GetPixelBlue(p) == GetPixelBlue(q)))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsGray(const PixelPacket *pixel)
{
  if ((GetPixelRed(pixel) == GetPixelGreen(pixel)) &&
      (GetPixelGreen(pixel) == GetPixelBlue(pixel)))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsMagickColorEqual(const MagickPixelPacket *p,
  const MagickPixelPacket *q)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((p->matte != MagickFalse) && (q->matte == MagickFalse) &&
      (p->opacity != OpaqueOpacity))
    return(MagickFalse);
  if ((q->matte != MagickFalse) && (p->matte == MagickFalse) &&
      (q->opacity != OpaqueOpacity))
    return(MagickFalse);
  if ((p->matte != MagickFalse) && (q->matte != MagickFalse))
    {
      if (p->opacity != q->opacity)
        return(MagickFalse);
      if (p->opacity == TransparentOpacity)
        return(MagickTrue);
    }
  if (p->red != q->red)
    return(MagickFalse);
  if (p->green != q->green)
    return(MagickFalse);
  if (p->blue != q->blue)
    return(MagickFalse);
  if ((p->colorspace == CMYKColorspace) && (p->index != q->index))
    return(MagickFalse);
#else
  if ((p->matte != MagickFalse) && (q->matte == MagickFalse) &&
      (fabs(p->opacity-OpaqueOpacity) > 0.5))
    return(MagickFalse);
  if ((q->matte != MagickFalse) && (p->matte == MagickFalse) &&
      (fabs(q->opacity-OpaqueOpacity)) > 0.5)
    return(MagickFalse);
  if ((p->matte != MagickFalse) && (q->matte != MagickFalse))
    {
      if (fabs(p->opacity-q->opacity) > 0.5)
        return(MagickFalse);
      if (fabs(p->opacity-TransparentOpacity) <= 0.5)
        return(MagickTrue);
    }
  if (fabs(p->red-q->red) > 0.5)
    return(MagickFalse);
  if (fabs(p->green-q->green) > 0.5)
    return(MagickFalse);
  if (fabs(p->blue-q->blue) > 0.5)
    return(MagickFalse);
  if ((p->colorspace == CMYKColorspace) && (fabs(p->index-q->index) > 0.5))
    return(MagickFalse);
#endif
  return(MagickTrue);
}

static inline MagickBooleanType IsMagickGray(const MagickPixelPacket *pixel)
{
  if (pixel->colorspace != RGBColorspace)
    return(MagickFalse);
  if ((pixel->red == pixel->green) && (pixel->green == pixel->blue))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickRealType MagickPixelIntensity(
  const MagickPixelPacket *pixel)
{
  return((MagickRealType) (0.299*pixel->red+0.587*pixel->green+0.114*pixel->blue));
}

static inline Quantum MagickPixelIntensityToQuantum(
  const MagickPixelPacket *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (0.299*pixel->red+0.587*pixel->green+0.114*pixel->blue+0.5));
#else
  return((Quantum) (0.299*pixel->red+0.587*pixel->green+0.114*pixel->blue));
#endif
}

static inline MagickRealType MagickPixelLuminance(
  const MagickPixelPacket *pixel)
{
  MagickRealType
    luminance;

  luminance=0.21267*pixel->red+0.71516*pixel->green+0.07217*pixel->blue;
  return(luminance);
}

static inline MagickRealType PixelIntensity(const PixelPacket *pixel)
{
  MagickRealType
    intensity;

  if ((GetPixelRed(pixel) == GetPixelGreen(pixel)) &&
      (GetPixelGreen(pixel) == GetPixelBlue(pixel)))
    return((MagickRealType) pixel->red);
  intensity=(MagickRealType) (0.299*GetPixelRed(pixel)+0.587*
    GetPixelGreen(pixel)+0.114*GetPixelBlue(pixel));
  return(intensity);
}

static inline Quantum PixelIntensityToQuantum(const PixelPacket *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if ((GetPixelRed(pixel) == GetPixelGreen(pixel)) &&
      (GetPixelGreen(pixel) == GetPixelBlue(pixel)))
    return(GetPixelRed(pixel));
  return((Quantum) (0.299*GetPixelRed(pixel)+0.587*
    GetPixelGreen(pixel)+0.114*GetPixelBlue(pixel)+0.5));
#else
  {
    double
      alpha,
      beta;

    alpha=GetPixelRed(pixel)-GetPixelGreen(pixel);
    beta=GetPixelGreen(pixel)-GetPixelBlue(pixel);
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(GetPixelRed(pixel));
    return((Quantum) (0.299*GetPixelRed(pixel)+0.587*
      GetPixelGreen(pixel)+0.114*GetPixelBlue(pixel)));
  }
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

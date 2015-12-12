/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
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

#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/color.h"
#include "magick/exception-private.h"
#include "magick/pixel-accessor.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickPrivate MagickBooleanType
  IsIntensitySimilar(const Image *,const PixelPacket *,const PixelPacket *);

static inline double GetFuzzyColorDistance(const Image *p,const Image *q)
{
  double
    fuzz;

  fuzz=(double) MagickMax(MagickMax(p->fuzz,q->fuzz),(MagickRealType)
    MagickSQ1_2);
  return(fuzz*fuzz);
}

static inline MagickBooleanType IsColorEqual(const PixelPacket *p,
  const PixelPacket *q)
{
  MagickRealType
    blue,
    green,
    red;

  red=(MagickRealType) p->red;
  green=(MagickRealType) p->green;
  blue=(MagickRealType) p->blue;
  if ((fabs((double) (red-q->red)) < MagickEpsilon) &&
      (fabs((double) (green-q->green)) < MagickEpsilon) &&
      (fabs((double) (blue-q->blue)) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsMagickColorEqual(const MagickPixelPacket *p,
  const MagickPixelPacket *q)
{
  if ((p->matte != MagickFalse) && (q->matte == MagickFalse) &&
      (fabs((double) (p->opacity-OpaqueOpacity)) >= MagickEpsilon))
    return(MagickFalse);
  if ((q->matte != MagickFalse) && (p->matte == MagickFalse) &&
      (fabs((double) (q->opacity-OpaqueOpacity))) >= MagickEpsilon)
    return(MagickFalse);
  if ((p->matte != MagickFalse) && (q->matte != MagickFalse))
    {
      if (fabs((double) (p->opacity-q->opacity)) >= MagickEpsilon)
        return(MagickFalse);
      if (fabs((double) (p->opacity-TransparentOpacity)) < MagickEpsilon)
        return(MagickTrue);
    }
  if (fabs((double) (p->red-q->red)) >= MagickEpsilon)
    return(MagickFalse);
  if (fabs((double) (p->green-q->green)) >= MagickEpsilon)
    return(MagickFalse);
  if (fabs((double) (p->blue-q->blue)) >= MagickEpsilon)
    return(MagickFalse);
  if ((p->colorspace == CMYKColorspace) &&
      (fabs((double) (p->index-q->index)) >= MagickEpsilon))
    return(MagickFalse);
  return(MagickTrue);
}

static inline MagickBooleanType IsMagickGray(const MagickPixelPacket *pixel)
{
  if ((pixel->colorspace != GRAYColorspace) &&
      (pixel->colorspace != RGBColorspace))
    return(MagickFalse);
  if ((fabs((double) (pixel->red-pixel->green)) < MagickEpsilon) &&
      (fabs((double) (pixel->green-pixel->blue)) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickRealType MagickPixelIntensity(
  const MagickPixelPacket *pixel)
{
  if (pixel->colorspace == GRAYColorspace)
    return(pixel->red);
  return(0.212656*pixel->red+0.715158*pixel->green+0.072186*pixel->blue);
}

static inline Quantum MagickPixelIntensityToQuantum(
  const MagickPixelPacket *pixel)
{
  if (pixel->colorspace == GRAYColorspace)
    return(ClampToQuantum(pixel->red));
  return(ClampToQuantum(0.212656*pixel->red+0.715158*pixel->green+
    0.072186*pixel->blue));
}

static inline MagickRealType MagickPixelLuma(const MagickPixelPacket *pixel)
{
  MagickRealType
    blue,
    green,
    red;

  if (pixel->colorspace == GRAYColorspace)
    return(pixel->red);
  if (pixel->colorspace == sRGBColorspace)
    return(0.212656*pixel->red+0.715158*pixel->green+0.072186*pixel->blue);
  red=EncodePixelGamma(pixel->red);
  green=EncodePixelGamma(pixel->green);
  blue=EncodePixelGamma(pixel->blue);
  return(0.212656*red+0.715158*green+0.072186*blue);
}

static inline MagickRealType MagickPixelLuminance(
  const MagickPixelPacket *pixel)
{
  MagickRealType
    blue,
    green,
    red;

  if (pixel->colorspace == GRAYColorspace)
    return(pixel->red);
  if (pixel->colorspace != sRGBColorspace)
    return(0.212656*pixel->red+0.715158*pixel->green+0.072186*pixel->blue);
  red=DecodePixelGamma(pixel->red);
  green=DecodePixelGamma(pixel->green);
  blue=DecodePixelGamma(pixel->blue);
  return(0.212656*red+0.715158*green+0.072186*blue);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

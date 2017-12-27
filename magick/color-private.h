/*
  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image color methods.
*/
#ifndef MAGICKCORE_COLOR_PRIVATE_H
#define MAGICKCORE_COLOR_PRIVATE_H

#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/color.h"
#include "magick/colorspace-private.h"
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
  if ((AbsolutePixelValue(red-q->red) < MagickEpsilon) &&
      (AbsolutePixelValue(green-q->green) < MagickEpsilon) &&
      (AbsolutePixelValue(blue-q->blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsMagickColorEqual(const MagickPixelPacket *p,
  const MagickPixelPacket *q)
{
  MagickRealType
    alpha,
    beta;

  alpha=p->matte == MagickFalse ? OpaqueOpacity : p->opacity;
  beta=q->matte == MagickFalse ? OpaqueOpacity : q->opacity;
  if (AbsolutePixelValue(alpha-beta) >= MagickEpsilon)
    return(MagickFalse);
  if ((AbsolutePixelValue(alpha-TransparentOpacity) < MagickEpsilon) ||
      (AbsolutePixelValue(beta-TransparentOpacity) < MagickEpsilon))
    return(MagickTrue);  /* no color component if pixel is transparent */
  if (AbsolutePixelValue(p->red-q->red) >= MagickEpsilon)
    return(MagickFalse);
  if (AbsolutePixelValue(p->green-q->green) >= MagickEpsilon)
    return(MagickFalse);
  if (AbsolutePixelValue(p->blue-q->blue) >= MagickEpsilon)
    return(MagickFalse);
  if (p->colorspace == CMYKColorspace)
    {
      if (AbsolutePixelValue(p->index-q->index) >= MagickEpsilon)
        return(MagickFalse);
    }
  return(MagickTrue);
}

static inline MagickBooleanType IsMagickGray(const MagickPixelPacket *pixel)
{
  if (IssRGBCompatibleColorspace(pixel->colorspace) == MagickFalse)
    return(MagickFalse);
  if ((AbsolutePixelValue(pixel->red-pixel->green) < MagickEpsilon) &&
      (AbsolutePixelValue(pixel->green-pixel->blue) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickRealType MagickPixelIntensity(
  const MagickPixelPacket *pixel)
{
  if (IsGrayColorspace(pixel->colorspace) != MagickFalse)
    return(pixel->red);
  return(0.212656*pixel->red+0.715158*pixel->green+0.072186*pixel->blue);
}

static inline Quantum MagickPixelIntensityToQuantum(
  const MagickPixelPacket *pixel)
{
  if (IsGrayColorspace(pixel->colorspace) != MagickFalse)
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

  if (IsGrayColorspace(pixel->colorspace) != MagickFalse)
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

  if (IsGrayColorspace(pixel->colorspace) != MagickFalse)
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

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

  MagickCore pixel accessor methods.
*/
#ifndef _MAGICKCORE_PIXEL_ACCESSOR_H
#define _MAGICKCORE_PIXEL_ACCESSOR_H

#include <math.h>
#include "magick/gem.h"
#include "magick/pixel.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define ClampPixelRed(pixel) ClampToQuantum((pixel)->red)
#define ClampPixelGreen(pixel) ClampToQuantum((pixel)->green)
#define ClampPixelBlue(pixel) ClampToQuantum((pixel)->blue)
#define ClampPixelIndex(indexes) ClampToQuantum(*(indexes))
#define ClampPixelOpacity(pixel) ClampToQuantum((pixel)->opacity)
#define GetPixela(pixel) ((pixel)->green)
#define GetPixelb(pixel) ((pixel)->blue)
#define GetPixelAlpha(pixel) (QuantumRange-(pixel)->opacity)
#define GetPixelBlack(indexes) (*(indexes))
#define GetPixelBlue(pixel) ((pixel)->blue)
#define GetPixelCb(pixel) ((pixel)->green)
#define GetPixelCr(pixel) ((pixel)->blue)
#define GetPixelCyan(pixel) ((pixel)->red)
#define GetPixelGray(pixel) ((pixel)->red)
#define GetPixelGreen(pixel) ((pixel)->green)
#define GetPixelIndex(indexes)  (*(indexes))
#define GetPixelL(pixel) ((pixel)->red)
#define GetPixelLabel(pixel) ((ssize_t) (pixel)->red)
#define GetPixelMagenta(pixel) ((pixel)->green)
#define GetPixelNext(pixel)  ((pixel)+1)
#define GetPixelOpacity(pixel) ((pixel)->opacity)
#define GetPixelRed(pixel) ((pixel)->red)
#define GetPixelRGB(pixel,packet) \
{ \
  (packet)->red=GetPixelRed((pixel)); \
  (packet)->green=GetPixelGreen((pixel)); \
  (packet)->blue=GetPixelBlue((pixel)); \
}
#define GetPixelRGBO(pixel,packet) \
{ \
  (packet)->red=GetPixelRed((pixel)); \
  (packet)->green=GetPixelGreen((pixel)); \
  (packet)->blue=GetPixelBlue((pixel)); \
  (packet)->opacity=GetPixelOpacity((pixel)); \
}
#define GetPixelY(pixel) ((pixel)->red)
#define GetPixelYellow(pixel) ((pixel)->blue)
#define SetPixela(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetPixelAlpha(pixel,value) \
  ((pixel)->opacity=(Quantum) (QuantumRange-(value)))
#define SetPixelb(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetPixelBlack(indexes,value) (*(indexes)=(Quantum) (value))
#define SetPixelBlue(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetPixelCb(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetPixelCr(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetPixelCyan(pixel,value) ((pixel)->red=(Quantum) (value))
#define SetPixelGray(pixel,value) \
  ((pixel)->red=(pixel)->green=(pixel)->blue=(Quantum) (value))
#define SetPixelGreen(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetPixelIndex(indexes,value) (*(indexes)=(IndexPacket) (value))
#define SetPixelL(pixel,value) ((pixel)->red=(Quantum) (value))
#define SetPixelMagenta(pixel,value) ((pixel)->green=(Quantum) (value))
#define SetPixelOpacity(pixel,value) ((pixel)->opacity=(Quantum) (value))
#define SetPixelRed(pixel,value) ((pixel)->red=(Quantum) (value))
#define SetPixelRgb(pixel,packet) \
{ \
  SetPixelRed(pixel,(packet)->red); \
  SetPixelGreen(pixel,(packet)->green); \
  SetPixelBlue(pixel,(packet)->blue); \
}
#define SetPixelRGBA(pixel,packet) \
{ \
  SetPixelRed(pixel,(packet)->red); \
  SetPixelGreen(pixel,(packet)->green); \
  SetPixelBlue(pixel,(packet)->blue); \
  SetPixelAlpha(pixel,(QuantumRange-(packet)->opacity)); \
}
#define SetPixelRGBO(pixel,packet) \
{ \
  SetPixelRed(pixel,(packet)->red); \
  SetPixelGreen(pixel,(packet)->green); \
  SetPixelBlue(pixel,(packet)->blue); \
  SetPixelOpacity(pixel,(packet)->opacity); \
}
#define SetPixelYellow(pixel,value) ((pixel)->blue=(Quantum) (value))
#define SetPixelY(pixel,value) ((pixel)->red=(Quantum) (value))

static inline MagickRealType AbsolutePixelValue(const MagickRealType x)
{
  return(x < 0.0f ? -x : x);
}

static inline Quantum ClampPixel(const MagickRealType value)
{ 
  if (value < 0.0f)
    return(0); 
  if (value >= (MagickRealType) QuantumRange)
    return((Quantum) QuantumRange);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (value+0.5f));
#else
  return((Quantum) value);
#endif
}

static inline double PerceptibleReciprocal(const double x)
{ 
  double
    sign;
      
  /*
    Return 1/x where x is perceptible (not unlimited or infinitesimal).
  */
  sign=x < 0.0 ? -1.0 : 1.0;
  if ((sign*x) >= MagickEpsilon)
    return(1.0/x);
  return(sign/MagickEpsilon);
}   

static inline MagickRealType GetPixelLuma(const Image *magick_restrict image,
  const PixelPacket *magick_restrict pixel)
{
  MagickRealType
    intensity;

  intensity=(MagickRealType) (0.212656f*pixel->red+0.715158f*pixel->green+
    0.072186f*pixel->blue);
  return(intensity);
}

static inline MagickRealType GetPixelLuminance(
  const Image *magick_restrict image,const PixelPacket *magick_restrict pixel)
{
  MagickRealType
    intensity;

  if (image->colorspace != sRGBColorspace)
    {
      intensity=(MagickRealType) (0.212656f*pixel->red+0.715158f*pixel->green+
        0.072186f*pixel->blue);
      return(intensity);
    }
  intensity=(MagickRealType) (0.212656f*DecodePixelGamma((MagickRealType)
    pixel->red)+0.715158f*DecodePixelGamma((MagickRealType) pixel->green)+
    0.072186f*DecodePixelGamma((MagickRealType) pixel->blue));
  return(intensity);
}

static inline MagickBooleanType IsPixelAtDepth(const Quantum pixel,
  const QuantumAny range)
{
  Quantum
    quantum;

#if !defined(MAGICKCORE_HDRI_SUPPORT)
  quantum=(Quantum) (((MagickRealType) QuantumRange*((QuantumAny)
    (((MagickRealType) range*pixel)/QuantumRange+0.5)))/range+0.5);
#else
  quantum=(Quantum) (((MagickRealType) QuantumRange*((QuantumAny)
    (((MagickRealType) range*pixel)/QuantumRange+0.5)))/range);
#endif
  return(pixel == quantum ? MagickTrue : MagickFalse);
}

static inline MagickBooleanType IsPixelGray(const PixelPacket *pixel)
{
  MagickRealType
    green_blue,
    red_green;

  red_green=(MagickRealType) pixel->red-pixel->green;
  green_blue=(MagickRealType) pixel->green-pixel->blue;
  if (((QuantumScale*AbsolutePixelValue(red_green)) < MagickEpsilon) &&
      ((QuantumScale*AbsolutePixelValue(green_blue)) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsPixelMonochrome(const PixelPacket *pixel)
{
  MagickRealType
    green_blue,
    red,
    red_green;

  red=(MagickRealType) pixel->red;
  if ((AbsolutePixelValue(red) >= MagickEpsilon) &&
      (AbsolutePixelValue(red-QuantumRange) >= MagickEpsilon))
    return(MagickFalse);
  red_green=(MagickRealType) pixel->red-pixel->green;
  green_blue=(MagickRealType) pixel->green-pixel->blue;
  if (((QuantumScale*AbsolutePixelValue(red_green)) < MagickEpsilon) &&
      ((QuantumScale*AbsolutePixelValue(green_blue)) < MagickEpsilon))
    return(MagickTrue);
  return(MagickFalse);
}

static inline Quantum PixelPacketIntensity(const PixelPacket *pixel)
{
  MagickRealType
    intensity;

  if ((pixel->red  == pixel->green) && (pixel->green == pixel->blue))
    return(pixel->red);
  intensity=(MagickRealType) (0.212656*pixel->red+0.715158*pixel->green+
    0.072186*pixel->blue);
  return(ClampToQuantum(intensity));
}

static inline void SetPixelViaMagickPixel(const Image *magick_restrict image,
  const MagickPixelPacket *magick_restrict magick_pixel,
  PixelPacket *magick_restrict pixel)
{ 
  pixel->red=ClampToQuantum(magick_pixel->red);
  pixel->green=ClampToQuantum(magick_pixel->green);
  pixel->blue=ClampToQuantum(magick_pixel->blue);
  if (image->matte != MagickFalse)
    pixel->opacity=ClampToQuantum(magick_pixel->opacity);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

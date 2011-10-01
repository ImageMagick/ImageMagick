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

static inline MagickPixelInfo *CloneMagickPixelInfo(
  const MagickPixelInfo *pixel)
{
  MagickPixelInfo
    *clone_pixel;

  clone_pixel=(MagickPixelInfo *) AcquireAlignedMemory(1,
    sizeof(*clone_pixel));
  if (clone_pixel == (MagickPixelInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  *clone_pixel=(*pixel);
  return(clone_pixel);
}

static inline MagickBooleanType IsGrayPixel(const PixelInfo *pixel)
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

    alpha=GetPixelRed(pixel)-GetPixelGreen(pixel);
    beta=GetPixelGreen(pixel)-GetPixelBlue(pixel);
    if ((fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
  }
#endif
  return(MagickFalse);
}

static inline MagickBooleanType IsMonochromePixel(const PixelInfo *pixel)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  if (((GetPixelRed(pixel) == 0) ||
       (GetPixelRed(pixel) == (Quantum) QuantumRange)) &&
      (GetPixelRed(pixel) == GetPixelGreen(pixel)) &&
      (GetPixelGreen(pixel) == GetPixelBlue(pixel)))
    return(MagickTrue);
#else
  {
    double
      alpha,
      beta;

    alpha=GetPixelRed(pixel)-GetPixelGreen(pixel);
    beta=GetPixelGreen(pixel)-GetPixelBlue(pixel);
    if (((fabs(GetPixelRed(pixel)) <= MagickEpsilon) ||
         (fabs(GetPixelRed(pixel)-QuantumRange) <= MagickEpsilon)) &&
        (fabs(alpha) <= MagickEpsilon) && (fabs(beta) <= MagickEpsilon))
      return(MagickTrue);
    }
#endif
  return(MagickFalse);
}

static inline void SetMagickPixelInfo(const Image *image,
  const PixelInfo *color,const IndexPacket *index,MagickPixelInfo *pixel)
{
  pixel->red=(MagickRealType) GetPixelRed(color);
  pixel->green=(MagickRealType) GetPixelGreen(color);
  pixel->blue=(MagickRealType) GetPixelBlue(color);
  pixel->opacity=(MagickRealType) GetPixelOpacity(color);
  if ((image->colorspace == CMYKColorspace) &&
      (index != (const IndexPacket *) NULL))
    pixel->index=(MagickRealType) GetPixelIndex(index);
}

static inline void SetMagickPixelInfoBias(const Image *image,
  MagickPixelInfo *pixel)
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

static inline void SetPixelInfo(const Image *image,
  const MagickPixelInfo *pixel,PixelInfo *color,IndexPacket *index)
{
  SetPixelRed(color,ClampToQuantum(pixel->red));
  SetPixelGreen(color,ClampToQuantum(pixel->green));
  SetPixelBlue(color,ClampToQuantum(pixel->blue));
  if (image->colorspace == CMYKColorspace)
    SetPixelBlack(index,ClampToQuantum(pixel->black));
  SetPixelAlpha(color,ClampToQuantum(pixel->alpha));
  if (image->storage_class == PseudoClass)
    SetPixelIndex(index,ClampToQuantum(pixel->index));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

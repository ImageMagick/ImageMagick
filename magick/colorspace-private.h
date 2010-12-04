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

  MagickCore image colorspace private methods.
*/
#ifndef _MAGICKCORE_COLORSPACE_PRIVATE_H
#define _MAGICKCORE_COLORSPACE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/image-private.h>
#include <magick/pixel.h>

static inline MagickBooleanType IsGrayColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == GRAYColorspace) || (colorspace == Rec601LumaColorspace) || 
      (colorspace == Rec709LumaColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline void ConvertRGBToCMYK(MagickPixelPacket *pixel)
{
  MagickRealType
    black,
    cyan,
    magenta,
    yellow;
                                                                                
  if ((pixel->red == 0) && (pixel->green == 0) && (pixel->blue == 0))
    {
      pixel->index=(MagickRealType) QuantumRange;
      return;
    }
  cyan=(MagickRealType) (1.0-QuantumScale*pixel->red);
  magenta=(MagickRealType) (1.0-QuantumScale*pixel->green);
  yellow=(MagickRealType) (1.0-QuantumScale*pixel->blue);
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

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

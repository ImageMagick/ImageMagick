/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
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

#include <magick/pixel.h>

static inline void ConvertRGBToCMYK(MagickPixelPacket *pixel)
{
  MagickRealType
    black,
    cyan,
    magenta,
    yellow;
                                                                                
  cyan=(MagickRealType) (QuantumRange-pixel->red);
  magenta=(MagickRealType) (QuantumRange-pixel->green);
  yellow=(MagickRealType) (QuantumRange-pixel->blue);
  black=(MagickRealType) QuantumRange;
  if (cyan < black)
    black=cyan;
  if (magenta < black)
    black=magenta;
  if (yellow < black)
    black=yellow;
  if (black == QuantumRange)
    {
      cyan=0.0;
      magenta=0.0;
      yellow=0.0;
    }
  else
    {
      cyan=(MagickRealType) (QuantumRange*(cyan-black)/
        (QuantumRange-black));
      magenta=(MagickRealType) (QuantumRange*(magenta-black)/
        (QuantumRange-black));
      yellow=(MagickRealType) (QuantumRange*(yellow-black)/
        (QuantumRange-black));
    }
  pixel->colorspace=CMYKColorspace;
  pixel->red=cyan;
  pixel->green=magenta;
  pixel->blue=yellow;
  pixel->index=black;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

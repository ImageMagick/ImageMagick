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

  MagickWand pixel wand methods.
*/
#ifndef MAGICKWAND_PIXEL_WAND_H
#define MAGICKWAND_PIXEL_WAND_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _PixelWand
  PixelWand;

extern WandExport char
  *PixelGetColorAsNormalizedString(const PixelWand *),
  *PixelGetColorAsString(const PixelWand *),
  *PixelGetException(const PixelWand *,ExceptionType *);

extern WandExport double
  PixelGetAlpha(const PixelWand *) magick_attribute((__pure__)),
  PixelGetBlack(const PixelWand *) magick_attribute((__pure__)),
  PixelGetBlue(const PixelWand *) magick_attribute((__pure__)),
  PixelGetCyan(const PixelWand *) magick_attribute((__pure__)),
  PixelGetFuzz(const PixelWand *) magick_attribute((__pure__)),
  PixelGetGreen(const PixelWand *) magick_attribute((__pure__)),
  PixelGetMagenta(const PixelWand *) magick_attribute((__pure__)),
  PixelGetAlpha(const PixelWand *) magick_attribute((__pure__)),
  PixelGetRed(const PixelWand *) magick_attribute((__pure__)),
  PixelGetYellow(const PixelWand *) magick_attribute((__pure__));

extern WandExport ExceptionType
  PixelGetExceptionType(const PixelWand *);

extern WandExport MagickBooleanType
  IsPixelWand(const PixelWand *),
  IsPixelWandSimilar(PixelWand *,PixelWand *,const double),
  PixelClearException(PixelWand *),
  PixelSetColor(PixelWand *,const char *);

extern WandExport PixelInfo
  PixelGetPixel(const PixelWand *);

extern WandExport PixelWand
  *ClonePixelWand(const PixelWand *),
  **ClonePixelWands(const PixelWand **,const size_t),
  *DestroyPixelWand(PixelWand *),
  **DestroyPixelWands(PixelWand **,const size_t),
  *NewPixelWand(void),
  **NewPixelWands(const size_t);

extern WandExport Quantum
  PixelGetAlphaQuantum(const PixelWand *) magick_attribute((__pure__)),
  PixelGetBlackQuantum(const PixelWand *) magick_attribute((__pure__)),
  PixelGetBlueQuantum(const PixelWand *) magick_attribute((__pure__)),
  PixelGetCyanQuantum(const PixelWand *) magick_attribute((__pure__)),
  PixelGetGreenQuantum(const PixelWand *) magick_attribute((__pure__)),
  PixelGetIndex(const PixelWand *) magick_attribute((__pure__)),
  PixelGetMagentaQuantum(const PixelWand *) magick_attribute((__pure__)),
  PixelGetAlphaQuantum(const PixelWand *) magick_attribute((__pure__)),
  PixelGetRedQuantum(const PixelWand *) magick_attribute((__pure__)),
  PixelGetYellowQuantum(const PixelWand *) magick_attribute((__pure__));

extern WandExport size_t
  PixelGetColorCount(const PixelWand *) magick_attribute((__pure__));

extern WandExport void
  ClearPixelWand(PixelWand *),
  PixelGetHSL(const PixelWand *,double *,double *,double *),
  PixelGetMagickColor(const PixelWand *,PixelInfo *),
  PixelGetQuantumPacket(const PixelWand *,PixelInfo *),
  PixelGetQuantumPixel(const Image *,const PixelWand *,Quantum *),
  PixelSetAlpha(PixelWand *,const double),
  PixelSetAlphaQuantum(PixelWand *,const Quantum),
  PixelSetBlack(PixelWand *,const double),
  PixelSetBlackQuantum(PixelWand *,const Quantum),
  PixelSetBlue(PixelWand *,const double),
  PixelSetBlueQuantum(PixelWand *,const Quantum),
  PixelSetColorFromWand(PixelWand *,const PixelWand *),
  PixelSetColorCount(PixelWand *,const size_t),
  PixelSetCyan(PixelWand *,const double),
  PixelSetCyanQuantum(PixelWand *,const Quantum),
  PixelSetFuzz(PixelWand *,const double),
  PixelSetGreen(PixelWand *,const double),
  PixelSetGreenQuantum(PixelWand *,const Quantum),
  PixelSetHSL(PixelWand *,const double,const double,const double),
  PixelSetIndex(PixelWand *,const Quantum),
  PixelSetMagenta(PixelWand *,const double),
  PixelSetMagentaQuantum(PixelWand *,const Quantum),
  PixelSetPixelColor(PixelWand *,const PixelInfo *),
  PixelSetAlpha(PixelWand *,const double),
  PixelSetAlphaQuantum(PixelWand *,const Quantum),
  PixelSetPixelColor(PixelWand *,const PixelInfo *),
  PixelSetQuantumPixel(const Image *,const Quantum *,PixelWand *),
  PixelSetRed(PixelWand *,const double),
  PixelSetRedQuantum(PixelWand *,const Quantum),
  PixelSetYellow(PixelWand *,const double),
  PixelSetYellowQuantum(PixelWand *,const Quantum);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

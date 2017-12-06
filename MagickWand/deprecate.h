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

  MagickCore deprecated methods.
*/
#ifndef MAGICKWAND_DEPRECATE_H
#define MAGICKWAND_DEPRECATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "MagickWand/pixel-wand.h"

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)

extern WandExport MagickBooleanType
  MagickGetImageAlphaColor(MagickWand *,PixelWand *),
  MagickSetImageAlphaColor(MagickWand *,const PixelWand *);

#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

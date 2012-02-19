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

  MagickWand Application Programming Interface declarations.
*/

#ifndef _MAGICK_WAND_H
#define _MAGICK_WAND_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(_MAGICKWAND_CONFIG_H)
# define _MAGICKWAND_CONFIG_H
# if !defined(vms) && !defined(macintosh)
#  include "magick/magick-config.h"
# else
#  include "magick-config.h"
# endif
#if defined(_magickcore_const) && !defined(const)
# define const _magickcore_const
#endif
#if defined(_magickcore_inline) && !defined(inline)
# define inline _magickcore_inline
#endif
#if defined(_magickcore_restrict) && !defined(restrict)
# define restrict  _magickcore_restrict
#endif
# if defined(__cplusplus) || defined(c_plusplus)
#  undef inline
# endif
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined(WIN32) || defined(WIN64)
#  define MAGICKCORE_WINDOWS_SUPPORT
#else
#  define MAGICKCORE_POSIX_SUPPORT
#endif 

typedef struct _MagickWand
  MagickWand;

#include "wand/method-attribute.h"
#include "magick/MagickCore.h"
#include "wand/animate.h"
#include "wand/compare.h"
#include "wand/composite.h"
#include "wand/conjure.h"
#include "wand/convert.h"
#include "wand/deprecate.h"
#include "wand/display.h"
#include "wand/drawing-wand.h"
#include "wand/identify.h"
#include "wand/import.h"
#include "wand/magick-property.h"
#include "wand/magick-image.h"
#include "wand/mogrify.h"
#include "wand/montage.h"
#include "wand/pixel-iterator.h"
#include "wand/pixel-wand.h"
#include "wand/stream.h"
#include "wand/wand-view.h"

extern WandExport char
  *MagickGetException(const MagickWand *,ExceptionType *);

extern WandExport ExceptionType
  MagickGetExceptionType(const MagickWand *);

extern WandExport MagickBooleanType
  IsMagickWand(const MagickWand *),
  MagickClearException(MagickWand *),
  MagickSetIteratorIndex(MagickWand *,const ssize_t);

extern WandExport MagickWand
  *CloneMagickWand(const MagickWand *),
  *DestroyMagickWand(MagickWand *),
  *NewMagickWand(void),
  *NewMagickWandFromImage(const Image *);

extern WandExport ssize_t
  MagickGetIteratorIndex(MagickWand *);

extern WandExport void
  ClearMagickWand(MagickWand *),
  MagickWandGenesis(void),
  MagickWandTerminus(void),
  *MagickRelinquishMemory(void *),
  MagickResetIterator(MagickWand *),
  MagickSetFirstIterator(MagickWand *),
  MagickSetLastIterator(MagickWand *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

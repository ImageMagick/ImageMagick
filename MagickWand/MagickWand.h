/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand Application Programming Interface declarations.
*/

#ifndef MAGICKWAND_MAGICKWAND_H
#define MAGICKWAND_MAGICKWAND_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(MAGICKWAND_CONFIG_H)
# define MAGICKWAND_CONFIG_H
# if !defined(vms) && !defined(macintosh)
#  include "MagickCore/magick-config.h"
# else
#  include "magick-config.h"
# endif
#if defined(_magickcore_const) && !defined(const)
# define const _magickcore_const
#endif
#if defined(_magickcore_inline) && !defined(inline)
# define inline _magickcore_inline
#endif
#if !defined(magick_restrict)
# if !defined(_magickcore_restrict)
#  define magick_restrict restrict
# else
#  define magick_restrict _magickcore_restrict
# endif
#endif
# if defined(__cplusplus) || defined(c_plusplus)
#  undef inline
# endif
#endif

#define MAGICKWAND_CHECK_VERSION(major,minor,micro) \
  ((MAGICKWAND_MAJOR_VERSION > (major)) || \
    ((MAGICKWAND_MAJOR_VERSION == (major)) && \
     (MAGICKWAND_MINOR_VERSION > (minor))) || \
    ((MAGICKWAND_MAJOR_VERSION == (major)) && \
     (MAGICKWAND_MINOR_VERSION == (minor)) && \
     (MAGICKWAND_MICRO_VERSION >= (micro))))

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>

#if defined(WIN32) || defined(WIN64)
#  define MAGICKWAND_WINDOWS_SUPPORT
#else
#  define MAGICKWAND_POSIX_SUPPORT
#endif 

typedef struct _MagickWand
  MagickWand;

#include "MagickWand/method-attribute.h"
#include "MagickCore/MagickCore.h"
#include "MagickWand/animate.h"
#include "MagickWand/compare.h"
#include "MagickWand/composite.h"
#include "MagickWand/conjure.h"
#include "MagickWand/convert.h"
#include "MagickWand/deprecate.h"
#include "MagickWand/display.h"
#include "MagickWand/drawing-wand.h"
#include "MagickWand/identify.h"
#include "MagickWand/import.h"
#include "MagickWand/wandcli.h"
#include "MagickWand/operation.h"
#include "MagickWand/magick-cli.h"
#include "MagickWand/magick-property.h"
#include "MagickWand/magick-image.h"
#include "MagickWand/mogrify.h"
#include "MagickWand/montage.h"
#include "MagickWand/pixel-iterator.h"
#include "MagickWand/pixel-wand.h"
#include "MagickWand/stream.h"
#include "MagickWand/wand-view.h"

extern WandExport char
  *MagickGetException(const MagickWand *,ExceptionType *);

extern WandExport ExceptionType
  MagickGetExceptionType(const MagickWand *);

extern WandExport MagickBooleanType
  IsMagickWand(const MagickWand *),
  IsMagickWandInstantiated(void),
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

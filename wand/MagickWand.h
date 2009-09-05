/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
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
# if defined(__cplusplus) || defined(c_plusplus)
#  undef inline
# endif
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined(__CYGWIN32__)
#  if !defined(__CYGWIN__)
#    define __CYGWIN__ __CYGWIN32__
#  endif
#endif
#if defined(_WIN32) || defined(WIN32)
#  if !defined(__WINDOWS__)
#    if defined(_WIN32)
#      define __WINDOWS__ _WIN32
#    else
#      if defined(WIN32)
#        define __WINDOWS__ WIN32
#      endif
#    endif
#  endif
#endif

#if defined(_WIN64) || defined(WIN64)
#  if !defined(__WINDOWS__)
#    if defined(_WIN64)
#      define __WINDOWS__ _WIN64
#    else
#      if defined(WIN64)
#        define __WINDOWS__ WIN64
#      endif
#    endif
#  endif
#endif

#if defined(__WINDOWS__)
# if defined(_MT) && defined(_DLL) && !defined(_MAGICKDLL_) && !defined(_LIB)
#  define _MAGICKDLL_
# endif
# if defined(_MAGICKDLL_)
#  if defined(_VISUALC_)
#   pragma warning( disable: 4273 )  /* Disable the dll linkage warnings */
#  endif
#  if !defined(_MAGICKLIB_)
#   define WandExport  __declspec(dllimport)
#   if defined(_VISUALC_)
#    pragma message( "MagickWand lib DLL import interface" )
#   endif
#  else
#   define WandExport  __declspec(dllexport)
#   if defined(_VISUALC_)
#    pragma message( "MagickWand lib DLL export interface" )
#   endif
#  endif
# else
#  define WandExport
#  if defined(_VISUALC_)
#   pragma message( "MagickWand lib static interface" )
#  endif
# endif

# if defined(_DLL) && !defined(_LIB)
#  define ModuleExport  __declspec(dllexport)
#  if defined(_VISUALC_)
#   pragma message( "MagickWand module DLL export interface" )
#  endif
# else
#  define ModuleExport
#  if defined(_VISUALC_)
#   pragma message( "MagickWand module static interface" )
#  endif

# endif
# define WandGlobal __declspec(thread)
# if defined(_VISUALC_)
#  pragma warning(disable : 4018)
#  pragma warning(disable : 4244)
#  pragma warning(disable : 4142)
#  pragma warning(disable : 4800)
#  pragma warning(disable : 4786)
#  pragma warning(disable : 4996)
# endif
#else
# define WandExport
# define ModuleExport
# define WandGlobal
#endif

#if !defined(MaxTextExtent)
# define MaxTextExtent  4096
#endif
#define WandSignature  0xabacadabUL

#if !defined(wand_attribute)
#  if !defined(__GNUC__)
#    define wand_attribute(x)  /* nothing */
#  else
#    define wand_attribute  __attribute__
#  endif
#endif

typedef struct _MagickWand
  MagickWand;

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
#include "wand/pixel-view.h"
#include "wand/pixel-wand.h"
#include "wand/stream.h"

extern WandExport char
  *MagickGetException(const MagickWand *,ExceptionType *);

extern WandExport ExceptionType
  MagickGetExceptionType(const MagickWand *);

extern WandExport long
  MagickGetIteratorIndex(MagickWand *);

extern WandExport MagickBooleanType
  IsMagickWand(const MagickWand *),
  MagickClearException(MagickWand *),
  MagickSetIteratorIndex(MagickWand *,const long);

extern WandExport MagickWand
  *CloneMagickWand(const MagickWand *),
  *DestroyMagickWand(MagickWand *),
  *NewMagickWand(void),
  *NewMagickWandFromImage(const Image *);

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

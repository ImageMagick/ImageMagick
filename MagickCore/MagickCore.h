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

  MagickCore Application Programming Interface declarations.
*/

#ifndef _MAGICKCORE_CORE_H
#define _MAGICKCORE_CORE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(_MAGICKCORE_CONFIG_H)
# define _MAGICKCORE_CONFIG_H
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

#if defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__CYGWIN__)
# define MagickPrivate
# if defined(_MT) && defined(_DLL) && !defined(_MAGICKDLL_) && !defined(_LIB) && !defined(MAGICK_STATIC_LINK)
#  define _MAGICKDLL_
# endif
# if defined(_MAGICKDLL_)
#  if defined(_VISUALC_)
#   pragma warning( disable: 4273 )  /* Disable the dll linkage warnings */
#  endif
#  if !defined(_MAGICKLIB_)
#   if defined(__GNUC__)
#    define MagickExport __attribute__ ((__dllimport__))
#   else
#    define MagickExport __declspec(dllimport)
#   endif
#   if defined(_VISUALC_)
#    pragma message( "Magick lib DLL import interface" )
#   endif
#  else
#   if defined(__GNUC__)
#    define MagickExport __attribute__ ((__dllexport__))
#   else
#    define MagickExport __declspec(dllexport)
#   endif
#   if defined(_VISUALC_)
#    pragma message( "Magick lib DLL export interface" )
#   endif
#  endif
# else
#  define MagickExport
#  if defined(_VISUALC_)
#   pragma message( "Magick lib static interface" )
#  endif
# endif

# if defined(_DLL) && !defined(_LIB)
#  define ModuleExport  __declspec(dllexport)
#  if defined(_VISUALC_)
#   pragma message( "Magick module DLL export interface" )
#  endif
# else
#  define ModuleExport
#  if defined(_VISUALC_)
#   pragma message( "Magick module static interface" )
#  endif

# endif
# define MagickGlobal __declspec(thread)
# if defined(_VISUALC_)
#  pragma warning(disable : 4018)
#  pragma warning(disable : 4244)
#  pragma warning(disable : 4142)
#  pragma warning(disable : 4800)
#  pragma warning(disable : 4786)
#  pragma warning(disable : 4996)
# endif
#else
# if __GNUC__ >= 4
#  define MagickExport __attribute__ ((__visibility__ ("default")))
#  define MagickPrivate  __attribute__ ((__visibility__ ("hidden")))
# else
#   define MagickExport
#   define MagickPrivate
# endif
# define ModuleExport  MagickExport
# define MagickGlobal
#endif

#if !defined(MaxTextExtent)
#  define MaxTextExtent  4096
#endif
#define MagickSignature  0xabacadabUL

#if !defined(magick_attribute)
#  if !defined(__GNUC__)
#    define magick_attribute(x)  /* nothing */
#  else
#    define magick_attribute  __attribute__
#  endif
#endif

#if defined(MAGICKCORE_NAMESPACE_PREFIX)
# include "MagickCore/methods.h"
#endif
#include "MagickCore/magick-type.h"
#include "MagickCore/xwindow.h"
#include "MagickCore/accelerate.h"
#include "MagickCore/animate.h"
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/cipher.h"
#include "MagickCore/client.h"
#include "MagickCore/coder.h"
#include "MagickCore/color.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colormap.h"
#include "MagickCore/compare.h"
#include "MagickCore/composite.h"
#include "MagickCore/compress.h"
#include "MagickCore/configure.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/delegate.h"
#include "MagickCore/display.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/feature.h"
#include "MagickCore/fourier.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/hashmap.h"
#include "MagickCore/histogram.h"
#include "MagickCore/identify.h"
#include "MagickCore/image.h"
#include "MagickCore/image-view.h"
#include "MagickCore/layer.h"
#include "MagickCore/list.h"
#include "MagickCore/locale_.h"
#include "MagickCore/log.h"
#include "MagickCore/magic.h"
#include "MagickCore/magick.h"
#include "MagickCore/matrix.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/mime.h"
#include "MagickCore/monitor.h"
#include "MagickCore/montage.h"
#include "MagickCore/morphology.h"
#include "MagickCore/nt-base.h"
#include "MagickCore/option.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/policy.h"
#include "MagickCore/prepress.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/registry.h"
#include "MagickCore/random_.h"
#include "MagickCore/resample.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/segment.h"
#include "MagickCore/shear.h"
#include "MagickCore/signature.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/stream.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/timer.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"
#include "MagickCore/threshold.h"
#include "MagickCore/type.h"
#include "MagickCore/utility.h"
#include "MagickCore/version.h"
#include "MagickCore/xml-tree.h"

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

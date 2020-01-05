/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore Application Programming Interface declarations.
*/

#ifndef MAGICKCORE_CORE_H
#define MAGICKCORE_CORE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(MAGICKCORE_CONFIG_H)
# define MAGICKCORE_CONFIG_H
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

#define MAGICKCORE_CHECK_VERSION(major,minor,micro) \
  ((MAGICKCORE_MAJOR_VERSION > (major)) || \
    ((MAGICKCORE_MAJOR_VERSION == (major)) && \
     (MAGICKCORE_MINOR_VERSION > (minor))) || \
    ((MAGICKCORE_MAJOR_VERSION == (major)) && \
     (MAGICKCORE_MINOR_VERSION == (minor)) && \
     (MAGICKCORE_MICRO_VERSION >= (micro))))

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>

#if defined(WIN32) || defined(WIN64)
#  define MAGICKCORE_WINDOWS_SUPPORT
#else
#  define MAGICKCORE_POSIX_SUPPORT
#endif 

#include "MagickCore/method-attribute.h"

#if defined(MAGICKCORE_NAMESPACE_PREFIX)
# include "MagickCore/methods.h"
#endif
#include "MagickCore/magick-type.h"
#include "MagickCore/animate.h"
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
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
#include "MagickCore/deprecate.h"
#include "MagickCore/display.h"
#include "MagickCore/distort.h"
#include "MagickCore/distribute-cache.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/feature.h"
#include "MagickCore/fourier.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/histogram.h"
#include "MagickCore/identify.h"
#include "MagickCore/image.h"
#include "MagickCore/image-view.h"
#include "MagickCore/layer.h"
#include "MagickCore/linked-list.h"
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
#include "MagickCore/opencl.h"
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
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/stream.h"
#include "MagickCore/string_.h"
#include "MagickCore/timer.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"
#include "MagickCore/threshold.h"
#include "MagickCore/type.h"
#include "MagickCore/utility.h"
#include "MagickCore/version.h"
#include "MagickCore/vision.h"
#include "MagickCore/visual-effects.h"
#include "MagickCore/xml-tree.h"
#include "MagickCore/xwindow.h"

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

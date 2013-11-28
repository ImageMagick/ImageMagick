/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image colormap methods.
*/
#ifndef _MAGICKCORE_COLORMAP_PRIVATE_H
#define _MAGICKCORE_COLORMAP_PRIVATE_H

#include "MagickCore/image.h"
#include "MagickCore/color.h"
#include "MagickCore/exception-private.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline Quantum ConstrainColormapIndex(Image *image,const size_t index,
  ExceptionInfo *exception)
{
  if (index < image->colors)
    return((Quantum) index);
  (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
    "InvalidColormapIndex","`%s'",image->filename);
  return((Quantum) 0);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

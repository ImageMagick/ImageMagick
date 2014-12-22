/*
  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization
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

static inline ssize_t ConstrainColormapIndex(Image *image,const ssize_t index,
  ExceptionInfo *exception)
{
  if ((index < 0) || (index >= (ssize_t) image->colors))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
        "InvalidColormapIndex","`%s'",image->filename);
      return(0);
    }
  return((ssize_t) index);
}

static inline MagickBooleanType IsValidColormapIndex(Image *image,
  const ssize_t index,Quantum *target,ExceptionInfo *exception)
{ 
  if ((index < 0) || (index >= (ssize_t) image->colors))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
        "InvalidColormapIndex","`%s'",image->filename);
      *target=(Quantum) 0;
      return(MagickFalse);
    }
  *target=(Quantum) index;
  return(MagickTrue);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

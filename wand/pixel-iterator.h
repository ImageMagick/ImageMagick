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

  Pixel Iterator Methods.
*/
#ifndef _MAGICKWAND_PIXEL_ITERATOR_H
#define _MAGICKWAND_PIXEL_ITERATOR_H

#include "wand/magick-wand.h"
#include "wand/pixel-wand.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _PixelIterator
  PixelIterator;

extern WandExport char
  *PixelGetIteratorException(const PixelIterator *,ExceptionType *);

extern WandExport ExceptionType
  PixelGetIteratorExceptionType(const PixelIterator *);

extern WandExport MagickBooleanType
  IsPixelIterator(const PixelIterator *),
  PixelClearIteratorException(PixelIterator *),
  PixelSetIteratorRow(PixelIterator *,const ssize_t),
  PixelSyncIterator(PixelIterator *);

extern WandExport PixelIterator
  *ClonePixelIterator(const PixelIterator *),
  *DestroyPixelIterator(PixelIterator *),
  *NewPixelIterator(MagickWand *),
  *NewPixelRegionIterator(MagickWand *,const ssize_t,const ssize_t,const size_t,
    const size_t);

extern WandExport PixelWand
  **PixelGetCurrentIteratorRow(PixelIterator *,size_t *),
  **PixelGetNextIteratorRow(PixelIterator *,size_t *),
  **PixelGetPreviousIteratorRow(PixelIterator *,size_t *);

extern WandExport ssize_t
  PixelGetIteratorRow(PixelIterator *);

extern WandExport void
  ClearPixelIterator(PixelIterator *),
  PixelResetIterator(PixelIterator *),
  PixelSetFirstIteratorRow(PixelIterator *),
  PixelSetLastIteratorRow(PixelIterator *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

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

  MagickCore private image drawing methods.
*/
#ifndef _MAGICKCORE_DRAW_PRIVATE_H
#define _MAGICKCORE_DRAW_PRIVATE_H

#include "magick/cache.h"
#include "magick/image.h"
#include "magick/memory_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline MagickBooleanType GetFillColor(const DrawInfo *draw_info,
  const ssize_t x,const ssize_t y,PixelPacket *pixel)
{
  Image
    *pattern;

  MagickBooleanType
    status;

  pattern=draw_info->fill_pattern;
  if (pattern == (Image *) NULL)
    {
      *pixel=draw_info->fill;
      return(MagickTrue);
    }
  status=GetOneVirtualMethodPixel(pattern,TileVirtualPixelMethod,
    x+pattern->tile_offset.x,y+pattern->tile_offset.y,pixel,
    &pattern->exception);
  if (pattern->matte == MagickFalse)
    pixel->opacity=OpaqueOpacity;
  return(status);
}

static inline MagickBooleanType GetStrokeColor(const DrawInfo *draw_info,
  const ssize_t x,const ssize_t y,PixelPacket *pixel)
{
  Image
    *pattern;

  MagickBooleanType
    status;

  pattern=draw_info->stroke_pattern;
  if (pattern == (Image *) NULL)
    {
      *pixel=draw_info->stroke;
      return(MagickTrue);
    }
  status=GetOneVirtualMethodPixel(pattern,TileVirtualPixelMethod,
    x+pattern->tile_offset.x,y+pattern->tile_offset.y,pixel,
    &pattern->exception);
  if (pattern->matte == MagickFalse)
    pixel->opacity=OpaqueOpacity;
  return(status);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

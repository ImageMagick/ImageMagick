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

  MagickCore private image drawing methods.
*/
#ifndef MAGICKCORE_DRAW_PRIVATE_H
#define MAGICKCORE_DRAW_PRIVATE_H

#include "MagickCore/cache.h"
#include "MagickCore/image.h"
#include "MagickCore/memory_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline void GetFillColor(const DrawInfo *draw_info,const ssize_t x,
  const ssize_t y,PixelInfo *fill,ExceptionInfo *exception)
{
  if (draw_info->fill_pattern == (Image *) NULL)
    {
      *fill=draw_info->fill;
      return;
    }
  {
    Image
      *pattern;

    pattern=draw_info->fill_pattern;
    (void) GetOneVirtualPixelInfo(pattern,TileVirtualPixelMethod,x+
      pattern->tile_offset.x,y+pattern->tile_offset.y,fill,exception);
    if (fabs(draw_info->fill_alpha-TransparentAlpha) >= MagickEpsilon)
      fill->alpha*=QuantumScale*draw_info->fill_alpha;
  }
}

static inline void GetStrokeColor(const DrawInfo *draw_info,const ssize_t x,
  const ssize_t y,PixelInfo *stroke,ExceptionInfo *exception)
{
  if (draw_info->stroke_pattern == (Image *) NULL)
    {
      *stroke=draw_info->stroke;
      return;
    }
  {
    Image
      *pattern;

    pattern=draw_info->stroke_pattern;
    (void) GetOneVirtualPixelInfo(pattern,TileVirtualPixelMethod,x+
      pattern->tile_offset.x,y+pattern->tile_offset.y,stroke,exception);
    if (fabs(draw_info->stroke_alpha-TransparentAlpha) >= MagickEpsilon)
      stroke->alpha*=QuantumScale*draw_info->stroke_alpha;
  }
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

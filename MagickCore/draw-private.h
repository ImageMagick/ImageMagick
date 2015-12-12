/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
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

#include "MagickCore/cache.h"
#include "MagickCore/image.h"
#include "MagickCore/memory_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline MagickBooleanType GetFillColor(const DrawInfo *draw_info,
  const ssize_t x,const ssize_t y,PixelInfo *fill,ExceptionInfo *exception)
{
  if (draw_info->fill_pattern == (Image *) NULL)
    {
      *fill=draw_info->fill;
      return(MagickTrue);
    }
  {
    Image
      *pattern;

    MagickBooleanType
      status;

    pattern=draw_info->fill_pattern;
    status=GetOneVirtualPixelInfo(pattern,TileVirtualPixelMethod,x+
      pattern->tile_offset.x,y+pattern->tile_offset.y,fill,exception);
    return(status);
  }
}

static inline MagickBooleanType GetStrokeColor(const DrawInfo *draw_info,
  const ssize_t x,const ssize_t y,PixelInfo *stroke,ExceptionInfo *exception)
{
  if (draw_info->stroke_pattern == (Image *) NULL)
    {
      *stroke=draw_info->stroke;
      return(MagickTrue);
    }
  {
    Image
      *pattern;

    MagickBooleanType
      status;

    pattern=draw_info->stroke_pattern;
    status=GetOneVirtualPixelInfo(pattern,TileVirtualPixelMethod,x+
      pattern->tile_offset.x,y+pattern->tile_offset.y,stroke,exception);
    return(status);
  }
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

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

  MagickCore computer vision methods.
*/
#ifndef _MAGICKCORE_VISION_H
#define _MAGICKCORE_VISION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _CCObjectInfo
{
  ssize_t
    id;

  RectangleInfo
    bounding_box;

  PixelInfo
    color;

  PointInfo
    centroid;

  double
    area,
    census;
} CCObjectInfo;

extern MagickExport Image
  *ConnectedComponentsImage(const Image *,const size_t,CCObjectInfo **,
    ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

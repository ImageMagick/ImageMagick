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

  MagickCore methods to set or get image attributes.
*/
#ifndef _MAGICKCORE_ATTRIBUTE_H
#define _MAGICKCORE_ATTRIBUTE_H

#include "magick/image.h"
#include "magick/exception.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport ImageType
  GetImageType(const Image *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  IsGrayImage(const Image *,ExceptionInfo *),
  IsMonochromeImage(const Image *,ExceptionInfo *),
  IsOpaqueImage(const Image *,ExceptionInfo *),
  SetImageChannelDepth(Image *,const ChannelType,const size_t),
  SetImageDepth(Image *,const size_t),
  SetImageType(Image *,const ImageType);

extern MagickExport RectangleInfo
  GetImageBoundingBox(const Image *,ExceptionInfo *exception);

extern MagickExport size_t
  GetImageChannelDepth(const Image *,const ChannelType,ExceptionInfo *),
  GetImageDepth(const Image *,ExceptionInfo *),
  GetImageQuantumDepth(const Image *,const MagickBooleanType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

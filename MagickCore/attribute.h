/*
  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore methods to set or get image attributes.
*/
#ifndef MAGICKCORE_ATTRIBUTE_H
#define MAGICKCORE_ATTRIBUTE_H

#include "MagickCore/image.h"
#include "MagickCore/exception.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport ImageType
  GetImageType(const Image *),
  IdentifyImageGray(const Image *,ExceptionInfo *),
  IdentifyImageType(const Image *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  IdentifyImageMonochrome(const Image *,ExceptionInfo *),
  IsImageGray(const Image *),
  IsImageMonochrome(const Image *),
  IsImageOpaque(const Image *,ExceptionInfo *),
  SetImageDepth(Image *,const size_t,ExceptionInfo *),
  SetImageType(Image *,const ImageType,ExceptionInfo *);

extern MagickExport RectangleInfo
  GetImageBoundingBox(const Image *,ExceptionInfo *exception);

extern MagickExport size_t
  GetImageDepth(const Image *,ExceptionInfo *),
  GetImageQuantumDepth(const Image *,const MagickBooleanType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

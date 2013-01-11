/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image channel methods.
*/
#ifndef _MAGICKCORE_CHANNEL_H
#define _MAGICKCORE_CHANNEL_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/image.h"

extern MagickExport Image
  *CombineImages(const Image *,const ChannelType,ExceptionInfo *),
  *SeparateImage(const Image *,const ChannelType,ExceptionInfo *),
  *SeparateImages(const Image *,const ChannelType,ExceptionInfo *);

extern MagickExport MagickBooleanType
  GetImageAlphaChannel(const Image *),
  SeparateImageChannel(Image *,const ChannelType),
  SetImageAlphaChannel(Image *,const AlphaChannelType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

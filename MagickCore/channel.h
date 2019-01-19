/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image channel methods.
*/
#ifndef MAGICKCORE_CHANNEL_H
#define MAGICKCORE_CHANNEL_H

#include <MagickCore/image.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport Image
  *ChannelFxImage(const Image *,const char *,ExceptionInfo *),
  *CombineImages(const Image *,const ColorspaceType,ExceptionInfo *),
  *SeparateImage(const Image *,const ChannelType,ExceptionInfo *),
  *SeparateImages(const Image *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  GetImageAlphaChannel(const Image *),
  SetImageAlphaChannel(Image *,const AlphaChannelOption,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

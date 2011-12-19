/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image enhance methods.
*/
#ifndef _MAGICKCORE_ENHANCE_H
#define _MAGICKCORE_ENHANCE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport MagickBooleanType
  AutoGammaImage(Image *),
  AutoGammaImageChannel(Image *,const ChannelType),
  AutoLevelImage(Image *),
  AutoLevelImageChannel(Image *,const ChannelType),
  BrightnessContrastImage(Image *,const double,const double),
  BrightnessContrastImageChannel(Image *,const ChannelType,const double,
    const double),
  ClutImage(Image *,const Image *),
  ClutImageChannel(Image *,const ChannelType,const Image *),
  ColorDecisionListImage(Image *,const char *),
  ContrastImage(Image *,const MagickBooleanType),
  ContrastStretchImage(Image *,const char *),
  ContrastStretchImageChannel(Image *,const ChannelType,const double,
    const double),
  EqualizeImage(Image *image),
  EqualizeImageChannel(Image *image,const ChannelType),
  GammaImage(Image *,const char *),
  GammaImageChannel(Image *,const ChannelType,const double),
  HaldClutImage(Image *,const Image *),
  HaldClutImageChannel(Image *,const ChannelType,const Image *),
  LevelImage(Image *,const char *),
  LevelImageChannel(Image *,const ChannelType,const double,const double,
    const double),
  LevelizeImage(Image *,const double,const double,const double),
  LevelizeImageChannel(Image *,const ChannelType,const double,const double,
    const double),
  LevelColorsImage(Image *,const MagickPixelPacket *,const MagickPixelPacket *,
    const MagickBooleanType),
  LevelColorsImageChannel(Image *,const ChannelType,const MagickPixelPacket *,
    const MagickPixelPacket *,const MagickBooleanType),
  LinearStretchImage(Image *,const double,const double),
  ModulateImage(Image *,const char *),
  NegateImage(Image *,const MagickBooleanType),
  NegateImageChannel(Image *,const ChannelType,const MagickBooleanType),
  NormalizeImage(Image *),
  NormalizeImageChannel(Image *,const ChannelType),
  SigmoidalContrastImage(Image *,const MagickBooleanType,const char *),
  SigmoidalContrastImageChannel(Image *,const ChannelType,
    const MagickBooleanType,const double,const double);

extern MagickExport Image
  *EnhanceImage(const Image *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

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

  MagickCore image private methods.
*/
#ifndef _MAGICKCORE_IMAGE_PRIVATE_H
#define _MAGICKCORE_IMAGE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MagickPI     3.14159265358979323846264338327950288419716939937510
#define Magick2PI    6.28318530717958647692528676655900576839433879875020
#define MagickPHI    1.61803398874989484820458683436563811772030917980576
#define MagickPI2    1.57079632679489661923132169163975144209858469968755
#define MagickSQ1_2  0.70710678118654752440084436210484903928483593768847
#define MagickSQ2    1.41421356237309504880168872420969807856967187537695
#define MagickSQ2PI  2.50662827463100024161235523934010416269302368164062
#define UndefinedTicksPerSecond  100L
#define UndefinedCompressionQuality  0UL

extern MagickExport const char
  BackgroundColor[],
  BorderColor[],
  DefaultTileFrame[],
  DefaultTileGeometry[],
  DefaultTileLabel[],
  ForegroundColor[],
  MatteColor[],
  LoadImageTag[],
  LoadImagesTag[],
  PSDensityGeometry[],
  PSPageGeometry[],
  SaveImageTag[],
  SaveImagesTag[];

extern MagickExport const double
  DefaultResolution;

static inline double DegreesToRadians(const double degrees)
{
  return((double) (MagickPI*degrees/180.0));
}

static inline MagickRealType RadiansToDegrees(const MagickRealType radians)
{
  return((MagickRealType) (180.0*radians/MagickPI));
}

static inline unsigned char ScaleColor5to8(const unsigned int color)
{
  return((unsigned char) (((color) << 3) | ((color) >> 2)));
}

static inline unsigned char ScaleColor6to8(const unsigned int color)
{
  return((unsigned char) (((color) << 2) | ((color) >> 4)));
}

static inline unsigned int ScaleColor8to5(const unsigned char color)
{
  return((unsigned int) (((color) & ~0x07) >> 3));
}

static inline unsigned int ScaleColor8to6(const unsigned char color)
{
  return((unsigned int) (((color) & ~0x03) >> 2));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

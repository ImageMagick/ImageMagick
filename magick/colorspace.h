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

  MagickCore image colorspace methods.
*/
#ifndef _MAGICKCORE_COLORSPACE_H
#define _MAGICKCORE_COLORSPACE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedColorspace,
  RGBColorspace,            /* Linear RGB colorspace */
  GRAYColorspace,           /* greyscale (linear) image (faked 1 channel) */
  TransparentColorspace,
  OHTAColorspace,
  LabColorspace,
  XYZColorspace,
  YCbCrColorspace,
  YCCColorspace,
  YIQColorspace,
  YPbPrColorspace,
  YUVColorspace,
  CMYKColorspace,           /* negated linear RGB with black separated */
  sRGBColorspace,           /* Default: non-linear sRGB colorspace */
  HSBColorspace,
  HSLColorspace,
  HWBColorspace,
  Rec601LumaColorspace,
  Rec601YCbCrColorspace,
  Rec709LumaColorspace,
  Rec709YCbCrColorspace,
  LogColorspace,
  CMYColorspace,            /* negated linear RGB colorspace */
  LuvColorspace,
  HCLColorspace,
  LCHColorspace,            /* alias for LCHuv */
  LMSColorspace,
  LCHabColorspace,          /* Cylindrical (Polar) Lab */
  LCHuvColorspace,          /* Cylindrical (Polar) Luv */
  scRGBColorspace,
  HSIColorspace,
  HSVColorspace,            /* alias for HSB */
  HCLpColorspace,
  YDbDrColorspace,
  xyYColorspace
} ColorspaceType;

extern MagickExport MagickBooleanType
  RGBTransformImage(Image *,const ColorspaceType),
  SetImageColorspace(Image *,const ColorspaceType),
  SetImageGray(Image *,ExceptionInfo *),
  SetImageMonochrome(Image *,ExceptionInfo *),
  TransformImageColorspace(Image *,const ColorspaceType),
  TransformRGBImage(Image *,const ColorspaceType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

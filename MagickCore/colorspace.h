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

  MagickCore image colorspace methods.
*/
#ifndef MAGICKCORE_COLORSPACE_H
#define MAGICKCORE_COLORSPACE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedColorspace,
  CMYColorspace,           /* negated linear RGB colorspace */
  CMYKColorspace,          /* CMY with Black separation */
  GRAYColorspace,          /* Single Channel greyscale (non-linear) image */
  HCLColorspace,
  HCLpColorspace,
  HSBColorspace,
  HSIColorspace,
  HSLColorspace,
  HSVColorspace,           /* alias for HSB */
  HWBColorspace,
  LabColorspace,
  LCHColorspace,           /* alias for LCHuv */
  LCHabColorspace,         /* Cylindrical (Polar) Lab */
  LCHuvColorspace,         /* Cylindrical (Polar) Luv */
  LogColorspace,
  LMSColorspace,
  LuvColorspace,
  OHTAColorspace,
  Rec601YCbCrColorspace,
  Rec709YCbCrColorspace,
  RGBColorspace,           /* Linear RGB colorspace */
  scRGBColorspace,         /* ??? */
  sRGBColorspace,          /* Default: non-linear sRGB colorspace */
  TransparentColorspace,
  xyYColorspace,
  XYZColorspace,           /* IEEE Color Reference colorspace */
  YCbCrColorspace,
  YCCColorspace,
  YDbDrColorspace,
  YIQColorspace,
  YPbPrColorspace,
  YUVColorspace,
  LinearGRAYColorspace,     /* Single Channel greyscale (linear) image */
  JzazbzColorspace
} ColorspaceType;

extern MagickExport ColorspaceType
  GetImageColorspaceType(const Image *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  SetImageColorspace(Image *,const ColorspaceType,ExceptionInfo *),
  SetImageGray(Image *,ExceptionInfo *),
  SetImageMonochrome(Image *,ExceptionInfo *),
  TransformImageColorspace(Image *,const ColorspaceType,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

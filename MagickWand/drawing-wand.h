/*
  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand drawing wand methods.
*/
#ifndef MAGICKWAND_DRAWING_WAND_H
#define MAGICKWAND_DRAWING_WAND_H

#include "MagickWand/pixel-wand.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _DrawingWand
  DrawingWand;

extern WandExport AlignType
  DrawGetTextAlignment(const DrawingWand *);

extern WandExport char
  *DrawGetClipPath(const DrawingWand *),
  *DrawGetDensity(const DrawingWand *),
  *DrawGetException(const DrawingWand *,ExceptionType *),
  *DrawGetFont(const DrawingWand *),
  *DrawGetFontFamily(const DrawingWand *),
  *DrawGetTextEncoding(const DrawingWand *),
  *DrawGetVectorGraphics(DrawingWand *);

extern WandExport ClipPathUnits
  DrawGetClipUnits(const DrawingWand *);

extern WandExport DecorationType
  DrawGetTextDecoration(const DrawingWand *);

extern WandExport DirectionType
  DrawGetTextDirection(const DrawingWand *);

extern WandExport double
  DrawGetFillOpacity(const DrawingWand *),
  DrawGetFontSize(const DrawingWand *),
  DrawGetOpacity(const DrawingWand *),
  *DrawGetStrokeDashArray(const DrawingWand *,size_t *),
  DrawGetStrokeDashOffset(const DrawingWand *),
  DrawGetStrokeOpacity(const DrawingWand *),
  DrawGetStrokeWidth(const DrawingWand *),
  DrawGetTextKerning(DrawingWand *),
  DrawGetTextInterlineSpacing(DrawingWand *),
  DrawGetTextInterwordSpacing(DrawingWand *);

extern WandExport DrawInfo
  *PeekDrawingWand(const DrawingWand *);

extern WandExport DrawingWand
  *AcquireDrawingWand(const DrawInfo *,Image *),
  *CloneDrawingWand(const DrawingWand *),
  *DestroyDrawingWand(DrawingWand *),
  *NewDrawingWand(void);

extern WandExport ExceptionInfo
  *DrawCloneExceptionInfo(const DrawingWand *wand);

extern WandExport ExceptionType
  DrawGetExceptionType(const DrawingWand *);

extern WandExport FillRule
  DrawGetClipRule(const DrawingWand *),
  DrawGetFillRule(const DrawingWand *);

extern WandExport GravityType
  DrawGetGravity(const DrawingWand *);

extern WandExport LineCap
  DrawGetStrokeLineCap(const DrawingWand *);

extern WandExport LineJoin
  DrawGetStrokeLineJoin(const DrawingWand *);

extern WandExport MagickBooleanType
  DrawClearException(DrawingWand *),
  DrawComposite(DrawingWand *,const CompositeOperator,const double,const double,
    const double,const double,MagickWand *),
  DrawGetFontResolution(const DrawingWand *,double *,double *),
  DrawGetStrokeAntialias(const DrawingWand *),
  DrawGetTextAntialias(const DrawingWand *),
  DrawPopPattern(DrawingWand *),
  DrawPushPattern(DrawingWand *,const char *,const double,const double,
    const double,const double),
  DrawRender(DrawingWand *),
  DrawSetClipPath(DrawingWand *,const char *),
  DrawSetDensity(DrawingWand *,const char *),
  DrawSetFillPatternURL(DrawingWand *,const char *),
  DrawSetFont(DrawingWand *,const char *),
  DrawSetFontFamily(DrawingWand *,const char *),
  DrawSetFontResolution(DrawingWand *,const double,const double),
  DrawSetStrokeDashArray(DrawingWand *,const size_t,const double *),
  DrawSetStrokePatternURL(DrawingWand *,const char *),
  DrawSetVectorGraphics(DrawingWand *,const char *),
  IsDrawingWand(const DrawingWand *),
  PopDrawingWand(DrawingWand *),
  PushDrawingWand(DrawingWand *);

extern WandExport StretchType
  DrawGetFontStretch(const DrawingWand *);

extern WandExport StyleType
  DrawGetFontStyle(const DrawingWand *);

extern WandExport size_t
  DrawGetFontWeight(const DrawingWand *),
  DrawGetStrokeMiterLimit(const DrawingWand *);

extern WandExport void
  ClearDrawingWand(DrawingWand *),
  DrawAffine(DrawingWand *,const AffineMatrix *),
  DrawAlpha(DrawingWand *,const double,const double,const PaintMethod),
  DrawAnnotation(DrawingWand *,const double,const double,const unsigned char *),
  DrawArc(DrawingWand *,const double,const double,const double,const double,
    const double,const double),
  DrawBezier(DrawingWand *,const size_t,const PointInfo *),
  DrawGetBorderColor(const DrawingWand *,PixelWand *),
  DrawCircle(DrawingWand *,const double,const double,const double,const double),
  DrawColor(DrawingWand *,const double,const double,const PaintMethod),
  DrawComment(DrawingWand *,const char *),
  DrawEllipse(DrawingWand *,const double,const double,const double,const double,
    const double,const double),
  DrawGetFillColor(const DrawingWand *,PixelWand *),
  DrawGetStrokeColor(const DrawingWand *,PixelWand *),
  DrawSetTextKerning(DrawingWand *,const double),
  DrawSetTextInterlineSpacing(DrawingWand *,const double),
  DrawSetTextInterwordSpacing(DrawingWand *,const double),
  DrawGetTextUnderColor(const DrawingWand *,PixelWand *),
  DrawLine(DrawingWand *,const double, const double,const double,const double),
  DrawPathClose(DrawingWand *),
  DrawPathCurveToAbsolute(DrawingWand *,const double,const double,const double,
    const double,const double,const double),
  DrawPathCurveToRelative(DrawingWand *,const double,const double,const double,
    const double,const double, const double),
  DrawPathCurveToQuadraticBezierAbsolute(DrawingWand *,const double,
    const double,const double,const double),
  DrawPathCurveToQuadraticBezierRelative(DrawingWand *,const double,
    const double,const double,const double),
  DrawPathCurveToQuadraticBezierSmoothAbsolute(DrawingWand *,const double,
    const double),
  DrawPathCurveToQuadraticBezierSmoothRelative(DrawingWand *,const double,
    const double),
  DrawPathCurveToSmoothAbsolute(DrawingWand *,const double,const double,
    const double,const double),
  DrawPathCurveToSmoothRelative(DrawingWand *,const double,const double,
    const double,const double),
  DrawPathEllipticArcAbsolute(DrawingWand *,const double,const double,
    const double,const MagickBooleanType,const MagickBooleanType,const double,
    const double),
  DrawPathEllipticArcRelative(DrawingWand *,const double,const double,
    const double,const MagickBooleanType,const MagickBooleanType,const double,
    const double),
  DrawPathFinish(DrawingWand *),
  DrawPathLineToAbsolute(DrawingWand *,const double,const double),
  DrawPathLineToRelative(DrawingWand *,const double,const double),
  DrawPathLineToHorizontalAbsolute(DrawingWand *,const double),
  DrawPathLineToHorizontalRelative(DrawingWand *,const double),
  DrawPathLineToVerticalAbsolute(DrawingWand *,const double),
  DrawPathLineToVerticalRelative(DrawingWand *,const double),
  DrawPathMoveToAbsolute(DrawingWand *,const double,const double),
  DrawPathMoveToRelative(DrawingWand *,const double,const double),
  DrawPathStart(DrawingWand *),
  DrawPoint(DrawingWand *,const double,const double),
  DrawPolygon(DrawingWand *,const size_t,const PointInfo *),
  DrawPolyline(DrawingWand *,const size_t,const PointInfo *),
  DrawPopClipPath(DrawingWand *),
  DrawPopDefs(DrawingWand *),
  DrawPushClipPath(DrawingWand *,const char *),
  DrawPushDefs(DrawingWand *),
  DrawRectangle(DrawingWand *,const double,const double,const double,
    const double),
  DrawResetVectorGraphics(DrawingWand *),
  DrawRotate(DrawingWand *,const double),
  DrawRoundRectangle(DrawingWand *,double,double,double,double,double,double),
  DrawScale(DrawingWand *,const double,const double),
  DrawSetBorderColor(DrawingWand *,const PixelWand *),
  DrawSetClipRule(DrawingWand *,const FillRule),
  DrawSetClipUnits(DrawingWand *,const ClipPathUnits),
  DrawSetFillColor(DrawingWand *,const PixelWand *),
  DrawSetFillOpacity(DrawingWand *,const double),
  DrawSetFillRule(DrawingWand *,const FillRule),
  DrawSetFontSize(DrawingWand *,const double),
  DrawSetFontStretch(DrawingWand *,const StretchType),
  DrawSetFontStyle(DrawingWand *,const StyleType),
  DrawSetFontWeight(DrawingWand *,const size_t),
  DrawSetGravity(DrawingWand *,const GravityType),
  DrawSetOpacity(DrawingWand *,const double),
  DrawSetStrokeAntialias(DrawingWand *,const MagickBooleanType),
  DrawSetStrokeColor(DrawingWand *,const PixelWand *),
  DrawSetStrokeDashOffset(DrawingWand *,const double dashoffset),
  DrawSetStrokeLineCap(DrawingWand *,const LineCap),
  DrawSetStrokeLineJoin(DrawingWand *,const LineJoin),
  DrawSetStrokeMiterLimit(DrawingWand *,const size_t),
  DrawSetStrokeOpacity(DrawingWand *, const double),
  DrawSetStrokeWidth(DrawingWand *,const double),
  DrawSetTextAlignment(DrawingWand *,const AlignType),
  DrawSetTextAntialias(DrawingWand *,const MagickBooleanType),
  DrawSetTextDecoration(DrawingWand *,const DecorationType),
  DrawSetTextDirection(DrawingWand *,const DirectionType),
  DrawSetTextEncoding(DrawingWand *,const char *),
  DrawSetTextUnderColor(DrawingWand *,const PixelWand *),
  DrawSetViewbox(DrawingWand *,const double,const double,const double,
    const double),
  DrawSkewX(DrawingWand *,const double),
  DrawSkewY(DrawingWand *,const double),
  DrawTranslate(DrawingWand *,const double,const double);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

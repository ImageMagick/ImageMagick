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

  MagickCore drawing methods.
*/
#ifndef MAGICKCORE_DRAW_H
#define MAGICKCORE_DRAW_H

#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/pixel.h"
#include "MagickCore/type.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedAlign,
  LeftAlign,
  CenterAlign,
  RightAlign
} AlignType;

typedef enum
{
  UndefinedPathUnits,
  UserSpace,
  UserSpaceOnUse,
  ObjectBoundingBox
} ClipPathUnits;

typedef enum
{
  UndefinedDecoration,
  NoDecoration,
  UnderlineDecoration,
  OverlineDecoration,
  LineThroughDecoration
} DecorationType;

typedef enum
{
  UndefinedDirection,
  RightToLeftDirection,
  LeftToRightDirection
} DirectionType;

typedef enum
{
  UndefinedRule,
#undef EvenOddRule
  EvenOddRule,
  NonZeroRule
} FillRule;

typedef enum
{
  UndefinedGradient,
  LinearGradient,
  RadialGradient
} GradientType;

typedef enum
{
  UndefinedCap,
  ButtCap,
  RoundCap,
  SquareCap
} LineCap;

typedef enum
{
  UndefinedJoin,
  MiterJoin,
  RoundJoin,
  BevelJoin
} LineJoin;

typedef enum
{
  UndefinedMethod,
  PointMethod,
  ReplaceMethod,
  FloodfillMethod,
  FillToBorderMethod,
  ResetMethod
} PaintMethod;

typedef enum
{
  UndefinedPrimitive,
  AlphaPrimitive,
  ArcPrimitive,
  BezierPrimitive,
  CirclePrimitive,
  ColorPrimitive,
  EllipsePrimitive,
  ImagePrimitive,
  LinePrimitive,
  PathPrimitive,
  PointPrimitive,
  PolygonPrimitive,
  PolylinePrimitive,
  RectanglePrimitive,
  RoundRectanglePrimitive,
  TextPrimitive
} PrimitiveType;

typedef enum
{
  UndefinedReference,
  GradientReference
} ReferenceType;

typedef enum
{
  UndefinedSpread,
  PadSpread,
  ReflectSpread,
  RepeatSpread
} SpreadMethod;

typedef struct _StopInfo
{
  PixelInfo
    color;

  double
    offset;
} StopInfo;

typedef struct _GradientInfo
{
  GradientType
    type;

  RectangleInfo
    bounding_box;

  SegmentInfo
    gradient_vector;

  StopInfo
    *stops;

  size_t
    number_stops;

  SpreadMethod
    spread;

  MagickBooleanType
    debug;

  PointInfo
    center,
    radii;

  double
    radius,
    angle;

  size_t
    signature;
} GradientInfo;

typedef struct _ElementReference
{
  char
    *id;

  ReferenceType
    type;

  GradientInfo
    gradient;

  struct _ElementReference
    *previous,
    *next;

  size_t
    signature;
} ElementReference;

typedef struct _DrawInfo
{
  char
    *primitive,
    *geometry;

  RectangleInfo
    viewbox;

  AffineMatrix
    affine;

  PixelInfo
    fill,
    stroke,
    undercolor,
    border_color;

  Image
    *fill_pattern,
    *stroke_pattern;

  double
    stroke_width;

  GradientInfo
    gradient;

  MagickBooleanType
    stroke_antialias,
    text_antialias;

  FillRule
    fill_rule;

  LineCap
    linecap;

  LineJoin
    linejoin;

  size_t
    miterlimit;

  double
    dash_offset;

  DecorationType
    decorate;

  CompositeOperator
    compose;

  char
    *text,
    *font,
    *metrics,
    *family;

  size_t
    face;

  StyleType
    style;

  StretchType
    stretch;

  size_t
    weight;

  char
    *encoding;

  double
    pointsize;

  char
    *density;

  AlignType
    align;

  GravityType
    gravity;

  char
    *server_name;

  double
    *dash_pattern;

  char
    *clip_mask;

  SegmentInfo
    bounds;

  ClipPathUnits
    clip_units;

  Quantum
    alpha;

  MagickBooleanType
    render;

  ElementReference
    element_reference;

  double
    kerning,
    interword_spacing,
    interline_spacing;

  DirectionType
    direction;

  MagickBooleanType
    debug;

  size_t
    signature;

  double
    fill_alpha,
    stroke_alpha;
} DrawInfo;


typedef struct _PrimitiveInfo
{
  PointInfo
    point;

  size_t
    coordinates;

  PrimitiveType
    primitive;

  PaintMethod
    method;

  char
    *text;
} PrimitiveInfo;

typedef struct _TypeMetric
{
  PointInfo
    pixels_per_em;

  double
    ascent,
    descent,
    width,
    height,
    max_advance,
    underline_position,
    underline_thickness;

  SegmentInfo
    bounds;

  PointInfo
    origin;
} TypeMetric;

extern MagickExport DrawInfo
  *AcquireDrawInfo(void),
  *CloneDrawInfo(const ImageInfo *,const DrawInfo *),
  *DestroyDrawInfo(DrawInfo *);

extern MagickExport MagickBooleanType
  DrawAffineImage(Image *,const Image *,const AffineMatrix *,ExceptionInfo *),
  DrawClipPath(Image *,const DrawInfo *,const char *,ExceptionInfo *),
  DrawGradientImage(Image *,const DrawInfo *,ExceptionInfo *),
  DrawImage(Image *,const DrawInfo *,ExceptionInfo *),
  DrawPatternPath(Image *,const DrawInfo *,const char *,Image **,
    ExceptionInfo *),
  DrawPrimitive(Image *,const DrawInfo *,const PrimitiveInfo *,ExceptionInfo *);

extern MagickExport void
  GetAffineMatrix(AffineMatrix *),
  GetDrawInfo(const ImageInfo *,DrawInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

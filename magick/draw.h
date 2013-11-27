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

  MagickCore drawing methods.
*/
#ifndef _MAGICKCORE_DRAW_H
#define _MAGICKCORE_DRAW_H

#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/pixel.h"
#include "magick/type.h"

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
  PointPrimitive,
  LinePrimitive,
  RectanglePrimitive,
  RoundRectanglePrimitive,
  ArcPrimitive,
  EllipsePrimitive,
  CirclePrimitive,
  PolylinePrimitive,
  PolygonPrimitive,
  BezierPrimitive,
  ColorPrimitive,
  MattePrimitive,
  TextPrimitive,
  ImagePrimitive,
  PathPrimitive
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

typedef struct _PointInfo
{ 
  double
    x,
    y;
} PointInfo;

typedef struct _StopInfo
{
  MagickPixelPacket
    color;

  MagickRealType
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

  size_t
    signature;

  PointInfo
    center;

  MagickRealType
    radius;
} GradientInfo;

typedef struct _ElementReference
{
  char
    *id;

  ReferenceType
    type;

  GradientInfo
    gradient;

  size_t
    signature;

  struct _ElementReference
    *previous,
    *next;
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

  GravityType
    gravity;

  PixelPacket
    fill,
    stroke;

  double
    stroke_width;

  GradientInfo
    gradient;

  Image
    *fill_pattern,
    *tile,
    *stroke_pattern;

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
    *text;

  size_t
    face;

  char
    *font,
    *metrics,
    *family;

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

  PixelPacket
    undercolor,
    border_color;

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
    opacity;

  MagickBooleanType
    render;

  ElementReference
    element_reference;

  MagickBooleanType
    debug;

  size_t
    signature;

  double
    kerning,
    interword_spacing,
    interline_spacing;

  DirectionType
    direction;
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
  DrawAffineImage(Image *,const Image *,const AffineMatrix *),
  DrawClipPath(Image *,const DrawInfo *,const char *),
  DrawGradientImage(Image *,const DrawInfo *),
  DrawImage(Image *,const DrawInfo *),
  DrawPatternPath(Image *,const DrawInfo *,const char *,Image **),
  DrawPrimitive(Image *,const DrawInfo *,const PrimitiveInfo *);

extern MagickExport void
  GetAffineMatrix(AffineMatrix *),
  GetDrawInfo(const ImageInfo *,DrawInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif





<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickWand, C API for ImageMagick: Drawing Wand Methods</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickwc, api, for, imagemagick:, drawing, wmethods, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
  <meta name="rating" content="GENERAL"/>
  <meta name="robots" content="INDEX, FOLLOW"/>
  <meta name="generator" content="ImageMagick Studio LLC"/>
  <meta name="author" content="ImageMagick Studio LLC"/>
  <meta name="revisit-after" content="2 DAYS"/>
  <meta name="resource-type" content="document"/>
  <meta name="copyright" content="Copyright (c) 1999-2016 ImageMagick Studio LLC"/>
  <meta name="distribution" content="Global"/>
  <meta name="magick-serial" content="P131-S030410-R485315270133-P82224-A6668-G1245-1"/>
  <link rel="icon" href="../image/wand.png"/>
  <link rel="shortcut icon" href="../image/wand.ico"/>
  <link rel="stylesheet" href="../css/magick.php"/>
</head>
<body>
<div class="main">
<div class="magick-masthead">
  <div class="container">
    <script async="async" src="http://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js"></script>    <ins class="adsbygoogle"
         style="display:block"
         data-ad-client="ca-pub-3129977114552745"
         data-ad-slot="6345125851"
         data-ad-format="auto"></ins>
    <script>
      (adsbygoogle = window.adsbygoogle || []).push({});
    </script>
    <nav class="magick-nav">
      <a class="magick-nav-item " href="../index.php">Home</a>
      <a class="magick-nav-item " href="../script/binary-releases.php">Download</a>
      <a class="magick-nav-item " href="../script/command-line-tools.php">Tools</a>
      <a class="magick-nav-item " href="../script/command-line-options.php">Options</a>
      <a class="magick-nav-item " href="../script/resources.php">Resources</a>
      <a class="magick-nav-item " href="../script/api.php">Develop</a>
      <a class="magick-nav-item " href="../script/search.php">Search</a>
      <a class="magick-nav-item pull-right" href="http://www.imagemagick.org/discourse-server/">Community</a>
    </nav>
  </div>
</div>
<div class="container">
<div class="magick-header">
<p class="text-center"><a href="drawing-wand.php#ClearDrawingWand">ClearDrawingWand</a> &bull; <a href="drawing-wand.php#CloneDrawingWand">CloneDrawingWand</a> &bull; <a href="drawing-wand.php#DestroyDrawingWand">DestroyDrawingWand</a> &bull; <a href="drawing-wand.php#DrawAffine">DrawAffine</a> &bull; <a href="drawing-wand.php#DrawAlpha">DrawAlpha</a> &bull; <a href="drawing-wand.php#DrawAnnotation">DrawAnnotation</a> &bull; <a href="drawing-wand.php#DrawArc">DrawArc</a> &bull; <a href="drawing-wand.php#DrawBezier">DrawBezier</a> &bull; <a href="drawing-wand.php#DrawCircle">DrawCircle</a> &bull; <a href="drawing-wand.php#DrawClearException">DrawClearException</a> &bull; <a href="drawing-wand.php#DrawCloneExceptionInfo">DrawCloneExceptionInfo</a> &bull; <a href="drawing-wand.php#DrawComposite">DrawComposite</a> &bull; <a href="drawing-wand.php#DrawColor">DrawColor</a> &bull; <a href="drawing-wand.php#DrawComment">DrawComment</a> &bull; <a href="drawing-wand.php#DrawEllipse">DrawEllipse</a> &bull; <a href="drawing-wand.php#DrawGetBorderColor">DrawGetBorderColor</a> &bull; <a href="drawing-wand.php#DrawGetClipPath">DrawGetClipPath</a> &bull; <a href="drawing-wand.php#DrawGetClipRule">DrawGetClipRule</a> &bull; <a href="drawing-wand.php#DrawGetClipUnits">DrawGetClipUnits</a> &bull; <a href="drawing-wand.php#DrawGetDensity">DrawGetDensity</a> &bull; <a href="drawing-wand.php#DrawGetException">DrawGetException</a> &bull; <a href="drawing-wand.php#DrawGetExceptionType">DrawGetExceptionType</a> &bull; <a href="drawing-wand.php#DrawGetFillColor">DrawGetFillColor</a> &bull; <a href="drawing-wand.php#DrawGetFillOpacity">DrawGetFillOpacity</a> &bull; <a href="drawing-wand.php#DrawGetFillRule">DrawGetFillRule</a> &bull; <a href="drawing-wand.php#DrawGetFont">DrawGetFont</a> &bull; <a href="drawing-wand.php#DrawGetFontFamily">DrawGetFontFamily</a> &bull; <a href="drawing-wand.php#DrawGetFontResolution">DrawGetFontResolution</a> &bull; <a href="drawing-wand.php#DrawGetFontSize">DrawGetFontSize</a> &bull; <a href="drawing-wand.php#DrawGetFontStretch">DrawGetFontStretch</a> &bull; <a href="drawing-wand.php#DrawGetFontStyle">DrawGetFontStyle</a> &bull; <a href="drawing-wand.php#DrawGetFontWeight">DrawGetFontWeight</a> &bull; <a href="drawing-wand.php#DrawGetGravity">DrawGetGravity</a> &bull; <a href="drawing-wand.php#DrawGetOpacity">DrawGetOpacity</a> &bull; <a href="drawing-wand.php#DrawGetStrokeAntialias">DrawGetStrokeAntialias</a> &bull; <a href="drawing-wand.php#DrawGetStrokeColor">DrawGetStrokeColor</a> &bull; <a href="drawing-wand.php#DrawGetStrokeDashArray">DrawGetStrokeDashArray</a> &bull; <a href="drawing-wand.php#DrawGetStrokeDashOffset">DrawGetStrokeDashOffset</a> &bull; <a href="drawing-wand.php#DrawGetStrokeLineCap">DrawGetStrokeLineCap</a> &bull; <a href="drawing-wand.php#DrawGetStrokeLineJoin">DrawGetStrokeLineJoin</a> &bull; <a href="drawing-wand.php#DrawGetStrokeMiterLimit">DrawGetStrokeMiterLimit</a> &bull; <a href="drawing-wand.php#DrawGetStrokeOpacity">DrawGetStrokeOpacity</a> &bull; <a href="drawing-wand.php#DrawGetStrokeWidth">DrawGetStrokeWidth</a> &bull; <a href="drawing-wand.php#DrawGetTextAlignment">DrawGetTextAlignment</a> &bull; <a href="drawing-wand.php#DrawGetTextAntialias">DrawGetTextAntialias</a> &bull; <a href="drawing-wand.php#DrawGetTextDecoration">DrawGetTextDecoration</a> &bull; <a href="drawing-wand.php#DrawGetTextDirection">DrawGetTextDirection</a> &bull; <a href="drawing-wand.php#DrawGetTextEncoding">DrawGetTextEncoding</a> &bull; <a href="drawing-wand.php#DrawGetTextKerning">DrawGetTextKerning</a> &bull; <a href="drawing-wand.php#DrawGetTextInterlineSpacing">DrawGetTextInterlineSpacing</a> &bull; <a href="drawing-wand.php#DrawGetTextInterwordSpacing">DrawGetTextInterwordSpacing</a> &bull; <a href="drawing-wand.php#DrawGetVectorGraphics">DrawGetVectorGraphics</a> &bull; <a href="drawing-wand.php#DrawGetTextUnderColor">DrawGetTextUnderColor</a> &bull; <a href="drawing-wand.php#DrawLine">DrawLine</a> &bull; <a href="drawing-wand.php#DrawPathClose">DrawPathClose</a> &bull; <a href="drawing-wand.php#DrawPathCurveToAbsolute">DrawPathCurveToAbsolute</a> &bull; <a href="drawing-wand.php#DrawPathCurveToRelative">DrawPathCurveToRelative</a> &bull; <a href="drawing-wand.php#DrawPathCurveToQuadraticBezierAbsolute">DrawPathCurveToQuadraticBezierAbsolute</a> &bull; <a href="drawing-wand.php#DrawPathCurveToQuadraticBezierRelative">DrawPathCurveToQuadraticBezierRelative</a> &bull; <a href="drawing-wand.php#DrawPathCurveToQuadraticBezierSmoothAbsolute">DrawPathCurveToQuadraticBezierSmoothAbsolute</a> &bull; <a href="drawing-wand.php#DrawPathCurveToQuadraticBezierSmoothRelative">DrawPathCurveToQuadraticBezierSmoothRelative</a> &bull; <a href="drawing-wand.php#DrawPathCurveToSmoothAbsolute">DrawPathCurveToSmoothAbsolute</a> &bull; <a href="drawing-wand.php#DrawPathCurveToSmoothRelative">DrawPathCurveToSmoothRelative</a> &bull; <a href="drawing-wand.php#DrawPathEllipticArcAbsolute">DrawPathEllipticArcAbsolute</a> &bull; <a href="drawing-wand.php#DrawPathEllipticArcRelative">DrawPathEllipticArcRelative</a> &bull; <a href="drawing-wand.php#DrawPathFinish">DrawPathFinish</a> &bull; <a href="drawing-wand.php#DrawPathLineToAbsolute">DrawPathLineToAbsolute</a> &bull; <a href="drawing-wand.php#DrawPathLineToRelative">DrawPathLineToRelative</a> &bull; <a href="drawing-wand.php#DrawPathLineToHorizontalAbsolute">DrawPathLineToHorizontalAbsolute</a> &bull; <a href="drawing-wand.php#DrawPathLineToHorizontalRelative">DrawPathLineToHorizontalRelative</a> &bull; <a href="drawing-wand.php#DrawPathLineToVerticalAbsolute">DrawPathLineToVerticalAbsolute</a> &bull; <a href="drawing-wand.php#DrawPathLineToVerticalRelative">DrawPathLineToVerticalRelative</a> &bull; <a href="drawing-wand.php#DrawPathMoveToAbsolute">DrawPathMoveToAbsolute</a> &bull; <a href="drawing-wand.php#DrawPathMoveToRelative">DrawPathMoveToRelative</a> &bull; <a href="drawing-wand.php#DrawPathStart">DrawPathStart</a> &bull; <a href="drawing-wand.php#DrawPoint">DrawPoint</a> &bull; <a href="drawing-wand.php#DrawPolygon">DrawPolygon</a> &bull; <a href="drawing-wand.php#DrawPolyline">DrawPolyline</a> &bull; <a href="drawing-wand.php#DrawPopClipPath">DrawPopClipPath</a> &bull; <a href="drawing-wand.php#DrawPopDefs">DrawPopDefs</a> &bull; <a href="drawing-wand.php#DrawPopPattern">DrawPopPattern</a> &bull; <a href="drawing-wand.php#DrawPushClipPath">DrawPushClipPath</a> &bull; <a href="drawing-wand.php#DrawPushDefs">DrawPushDefs</a> &bull; <a href="drawing-wand.php#DrawPushPattern">DrawPushPattern</a> &bull; <a href="drawing-wand.php#DrawRectangle">DrawRectangle</a> &bull; <a href="drawing-wand.php#DrawResetVectorGraphics">DrawResetVectorGraphics</a> &bull; <a href="drawing-wand.php#DrawRotate">DrawRotate</a> &bull; <a href="drawing-wand.php#DrawRoundRectangle">DrawRoundRectangle</a> &bull; <a href="drawing-wand.php#DrawScale">DrawScale</a> &bull; <a href="drawing-wand.php#DrawSetBorderColor">DrawSetBorderColor</a> &bull; <a href="drawing-wand.php#DrawSetClipPath">DrawSetClipPath</a> &bull; <a href="drawing-wand.php#DrawSetClipRule">DrawSetClipRule</a> &bull; <a href="drawing-wand.php#DrawSetClipUnits">DrawSetClipUnits</a> &bull; <a href="drawing-wand.php#DrawSetDensity">DrawSetDensity</a> &bull; <a href="drawing-wand.php#DrawSetFillColor">DrawSetFillColor</a> &bull; <a href="drawing-wand.php#DrawSetFillOpacity">DrawSetFillOpacity</a> &bull; <a href="drawing-wand.php#DrawSetFontResolution">DrawSetFontResolution</a> &bull; <a href="drawing-wand.php#DrawSetOpacity">DrawSetOpacity</a> &bull; <a href="drawing-wand.php#DrawSetFillPatternURL">DrawSetFillPatternURL</a> &bull; <a href="drawing-wand.php#DrawSetFillRule">DrawSetFillRule</a> &bull; <a href="drawing-wand.php#DrawSetFont">DrawSetFont</a> &bull; <a href="drawing-wand.php#DrawSetFontFamily">DrawSetFontFamily</a> &bull; <a href="drawing-wand.php#DrawSetFontSize">DrawSetFontSize</a> &bull; <a href="drawing-wand.php#DrawSetFontStretch">DrawSetFontStretch</a> &bull; <a href="drawing-wand.php#DrawSetFontStyle">DrawSetFontStyle</a> &bull; <a href="drawing-wand.php#DrawSetFontWeight">DrawSetFontWeight</a> &bull; <a href="drawing-wand.php#DrawSetGravity">DrawSetGravity</a> &bull; <a href="drawing-wand.php#DrawSetStrokeColor">DrawSetStrokeColor</a> &bull; <a href="drawing-wand.php#DrawSetStrokePatternURL">DrawSetStrokePatternURL</a> &bull; <a href="drawing-wand.php#DrawSetStrokeAntialias">DrawSetStrokeAntialias</a> &bull; <a href="drawing-wand.php#DrawSetStrokeDashArray">DrawSetStrokeDashArray</a> &bull; <a href="drawing-wand.php#DrawSetStrokeDashOffset">DrawSetStrokeDashOffset</a> &bull; <a href="drawing-wand.php#DrawSetStrokeLineCap">DrawSetStrokeLineCap</a> &bull; <a href="drawing-wand.php#DrawSetStrokeLineJoin">DrawSetStrokeLineJoin</a> &bull; <a href="drawing-wand.php#DrawSetStrokeMiterLimit">DrawSetStrokeMiterLimit</a> &bull; <a href="drawing-wand.php#DrawSetStrokeOpacity">DrawSetStrokeOpacity</a> &bull; <a href="drawing-wand.php#DrawSetStrokeWidth">DrawSetStrokeWidth</a> &bull; <a href="drawing-wand.php#DrawSetTextAlignment">DrawSetTextAlignment</a> &bull; <a href="drawing-wand.php#DrawSetTextAntialias">DrawSetTextAntialias</a> &bull; <a href="drawing-wand.php#DrawSetTextDecoration">DrawSetTextDecoration</a> &bull; <a href="drawing-wand.php#DrawSetTextEncoding">DrawSetTextEncoding</a> &bull; <a href="drawing-wand.php#DrawSetTextKerning">DrawSetTextKerning</a> &bull; <a href="drawing-wand.php#DrawSetTextInterlineSpacing">DrawSetTextInterlineSpacing</a> &bull; <a href="drawing-wand.php#DrawSetTextInterwordSpacing">DrawSetTextInterwordSpacing</a> &bull; <a href="drawing-wand.php#DrawSetTextUnderColor">DrawSetTextUnderColor</a> &bull; <a href="drawing-wand.php#DrawSetVectorGraphics">DrawSetVectorGraphics</a> &bull; <a href="drawing-wand.php#DrawSkewX">DrawSkewX</a> &bull; <a href="drawing-wand.php#DrawSkewY">DrawSkewY</a> &bull; <a href="drawing-wand.php#DrawTranslate">DrawTranslate</a> &bull; <a href="drawing-wand.php#DrawSetViewbox">DrawSetViewbox</a> &bull; <a href="drawing-wand.php#IsDrawingWand">IsDrawingWand</a> &bull; <a href="drawing-wand.php#NewDrawingWand">NewDrawingWand</a> &bull; <a href="drawing-wand.php#PeekDrawingWand">PeekDrawingWand</a> &bull; <a href="drawing-wand.php#PopDrawingWand">PopDrawingWand</a> &bull; <a href="drawing-wand.php#PushDrawingWand">PushDrawingWand</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="ClearDrawingWand">ClearDrawingWand</a></h2>

<p>ClearDrawingWand() clears resources associated with the drawing wand.</p>

<p>The format of the ClearDrawingWand method is:</p>

<pre class="text">
void ClearDrawingWand(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand to clear. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="CloneDrawingWand">CloneDrawingWand</a></h2>

<p>CloneDrawingWand() makes an exact copy of the specified wand.</p>

<p>The format of the CloneDrawingWand method is:</p>

<pre class="text">
DrawingWand *CloneDrawingWand(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DestroyDrawingWand">DestroyDrawingWand</a></h2>

<p>DestroyDrawingWand() frees all resources associated with the drawing wand. Once the drawing wand has been freed, it should not be used and further unless it re-allocated.</p>

<p>The format of the DestroyDrawingWand method is:</p>

<pre class="text">
DrawingWand *DestroyDrawingWand(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand to destroy. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawAffine">DrawAffine</a></h2>

<p>DrawAffine() adjusts the current affine transformation matrix with the specified affine transformation matrix. Note that the current affine transform is adjusted rather than replaced.</p>

<p>The format of the DrawAffine method is:</p>

<pre class="text">
void DrawAffine(DrawingWand *wand,const AffineMatrix *affine)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>Drawing wand </dd>

<dd> </dd>
<dt>affine</dt>
<dd>Affine matrix parameters </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawAlpha">DrawAlpha</a></h2>

<p>DrawAlpha() paints on the image's alpha channel in order to set effected pixels to transparent. to influence the alpha of pixels. The available paint methods are:</p>

<pre class="text">
    PointMethod: Select the target pixel
    ReplaceMethod: Select any pixel that matches the target pixel.
    FloodfillMethod: Select the target pixel and matching neighbors.
    FillToBorderMethod: Select the target pixel and neighbors not matching
border color.
    ResetMethod: Select all pixels.
</pre>

<p>The format of the DrawAlpha method is:</p>

<pre class="text">
void DrawAlpha(DrawingWand *wand,const double x,const double y,
  const PaintMethod paint_method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate </dd>

<dd> </dd>
<dt>paint_method</dt>
<dd>paint method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawAnnotation">DrawAnnotation</a></h2>

<p>DrawAnnotation() draws text on the image.</p>

<p>The format of the DrawAnnotation method is:</p>

<pre class="text">
void DrawAnnotation(DrawingWand *wand,const double x,
  const double y,const unsigned char *text)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate to left of text </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate to text baseline </dd>

<dd> </dd>
<dt>text</dt>
<dd>text to draw </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawArc">DrawArc</a></h2>

<p>DrawArc() draws an arc falling within a specified bounding rectangle on the image.</p>

<p>The format of the DrawArc method is:</p>

<pre class="text">
void DrawArc(DrawingWand *wand,const double sx,const double sy,
  const double ex,const double ey,const double sd,const double ed)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>sx</dt>
<dd>starting x ordinate of bounding rectangle </dd>

<dd> </dd>
<dt>sy</dt>
<dd>starting y ordinate of bounding rectangle </dd>

<dd> </dd>
<dt>ex</dt>
<dd>ending x ordinate of bounding rectangle </dd>

<dd> </dd>
<dt>ey</dt>
<dd>ending y ordinate of bounding rectangle </dd>

<dd> </dd>
<dt>sd</dt>
<dd>starting degrees of rotation </dd>

<dd> </dd>
<dt>ed</dt>
<dd>ending degrees of rotation </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawBezier">DrawBezier</a></h2>

<p>DrawBezier() draws a bezier curve through a set of points on the image.</p>

<p>The format of the DrawBezier method is:</p>

<pre class="text">
void DrawBezier(DrawingWand *wand,
  const size_t number_coordinates,const PointInfo *coordinates)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>number_coordinates</dt>
<dd>number of coordinates </dd>

<dd> </dd>
<dt>coordinates</dt>
<dd>coordinates </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawCircle">DrawCircle</a></h2>

<p>DrawCircle() draws a circle on the image.</p>

<p>The format of the DrawCircle method is:</p>

<pre class="text">
void DrawCircle(DrawingWand *wand,const double ox,
  const double oy,const double px, const double py)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>ox</dt>
<dd>origin x ordinate </dd>

<dd> </dd>
<dt>oy</dt>
<dd>origin y ordinate </dd>

<dd> </dd>
<dt>px</dt>
<dd>perimeter x ordinate </dd>

<dd> </dd>
<dt>py</dt>
<dd>perimeter y ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawClearException">DrawClearException</a></h2>

<p>DrawClearException() clear any exceptions associated with the wand.</p>

<p>The format of the DrawClearException method is:</p>

<pre class="text">
MagickBooleanType DrawClearException(DrawWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawCloneExceptionInfo">DrawCloneExceptionInfo</a></h2>

<p>DrawCloneExceptionInfo() clones the ExceptionInfo structure within the wand.</p>

<p>The format of the DrawCloneExceptionInfo method is:</p>

<pre class="text">
ExceptionInfo *DrawCloneExceptionInfo(DrawWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawComposite">DrawComposite</a></h2>

<p>DrawComposite() composites an image onto the current image, using the specified composition operator, specified position, and at the specified size.</p>

<p>The format of the DrawComposite method is:</p>

<pre class="text">
MagickBooleanType DrawComposite(DrawingWand *wand,
  const CompositeOperator compose,const double x,
  const double y,const double width,const double height,
  MagickWand *magick_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>compose</dt>
<dd>composition operator </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate of top left corner </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate of top left corner </dd>

<dd> </dd>
<dt>width</dt>
<dd>Width to resize image to prior to compositing.  Specify zero to use existing width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>Height to resize image to prior to compositing.  Specify zero to use existing height. </dd>

<dd> </dd>
<dt>magick_wand</dt>
<dd>Image to composite is obtained from this wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawColor">DrawColor</a></h2>

<p>DrawColor() draws color on image using the current fill color, starting at specified position, and using specified paint method. The available paint methods are:</p>

<pre class="text">
    PointMethod: Recolors the target pixel
    ReplaceMethod: Recolor any pixel that matches the target pixel.
    FloodfillMethod: Recolors target pixels and matching neighbors.
    ResetMethod: Recolor all pixels.
</pre>

<p>The format of the DrawColor method is:</p>

<pre class="text">
void DrawColor(DrawingWand *wand,const double x,const double y,
  const PaintMethod paint_method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate. </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate. </dd>

<dd> </dd>
<dt>paint_method</dt>
<dd>paint method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawComment">DrawComment</a></h2>

<p>DrawComment() adds a comment to a vector output stream.</p>

<p>The format of the DrawComment method is:</p>

<pre class="text">
void DrawComment(DrawingWand *wand,const char *comment)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>comment</dt>
<dd>comment text </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawEllipse">DrawEllipse</a></h2>

<p>DrawEllipse() draws an ellipse on the image.</p>

<p>The format of the DrawEllipse method is:</p>

<pre class="text">
 void DrawEllipse(DrawingWand *wand,const double ox,const double oy,
   const double rx,const double ry,const double start,const double end)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>ox</dt>
<dd>origin x ordinate </dd>

<dd> </dd>
<dt>oy</dt>
<dd>origin y ordinate </dd>

<dd> </dd>
<dt>rx</dt>
<dd>radius in x </dd>

<dd> </dd>
<dt>ry</dt>
<dd>radius in y </dd>

<dd> </dd>
<dt>start</dt>
<dd>starting rotation in degrees </dd>

<dd> </dd>
<dt>end</dt>
<dd>ending rotation in degrees </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetBorderColor">DrawGetBorderColor</a></h2>

<p>DrawGetBorderColor() returns the border color used for drawing bordered objects.</p>

<p>The format of the DrawGetBorderColor method is:</p>

<pre class="text">
void DrawGetBorderColor(const DrawingWand *wand,
  PixelWand *border_color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>border_color</dt>
<dd>Return the border color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetClipPath">DrawGetClipPath</a></h2>

<p>DrawGetClipPath() obtains the current clipping path ID. The value returned must be deallocated by the user when it is no longer needed.</p>

<p>The format of the DrawGetClipPath method is:</p>

<pre class="text">
char *DrawGetClipPath(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetClipRule">DrawGetClipRule</a></h2>

<p>DrawGetClipRule() returns the current polygon fill rule to be used by the clipping path.</p>

<p>The format of the DrawGetClipRule method is:</p>

<pre class="text">
     FillRule DrawGetClipRule(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetClipUnits">DrawGetClipUnits</a></h2>

<p>DrawGetClipUnits() returns the interpretation of clip path units.</p>

<p>The format of the DrawGetClipUnits method is:</p>

<pre class="text">
ClipPathUnits DrawGetClipUnits(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetDensity">DrawGetDensity</a></h2>

<p>DrawGetDensity() obtains the vertical and horizontal resolution. The value returned must be deallocated by the user when it is no longer needed.</p>

<p>The format of the DrawGetDensity method is:</p>

<pre class="text">
char *DrawGetDensity(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetException">DrawGetException</a></h2>

<p>DrawGetException() returns the severity, reason, and description of any error that occurs when using other methods in this API.</p>

<p>The format of the DrawGetException method is:</p>

<pre class="text">
char *DrawGetException(const DrawWand *wand,
  ExceptionType *severity)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>severity</dt>
<dd>the severity of the error is returned here. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetExceptionType">DrawGetExceptionType</a></h2>

<p>DrawGetExceptionType() the exception type associated with the wand.  If no exception has occurred, UndefinedExceptionType is returned.</p>

<p>The format of the DrawGetExceptionType method is:</p>

<pre class="text">
ExceptionType DrawGetExceptionType(const DrawWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetFillColor">DrawGetFillColor</a></h2>

<p>DrawGetFillColor() returns the fill color used for drawing filled objects.</p>

<p>The format of the DrawGetFillColor method is:</p>

<pre class="text">
void DrawGetFillColor(const DrawingWand *wand,
  PixelWand *fill_color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>fill_color</dt>
<dd>Return the fill color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetFillOpacity">DrawGetFillOpacity</a></h2>

<p>DrawGetFillOpacity() returns the alpha used when drawing using the fill color or fill texture.  Fully opaque is 1.0.</p>

<p>The format of the DrawGetFillOpacity method is:</p>

<pre class="text">
double DrawGetFillOpacity(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetFillRule">DrawGetFillRule</a></h2>

<p>DrawGetFillRule() returns the fill rule used while drawing polygons.</p>

<p>The format of the DrawGetFillRule method is:</p>

<pre class="text">
FillRule DrawGetFillRule(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetFont">DrawGetFont</a></h2>

<p>DrawGetFont() returns a null-terminaged string specifying the font used when annotating with text. The value returned must be freed by the user when no longer needed.</p>

<p>The format of the DrawGetFont method is:</p>

<pre class="text">
char *DrawGetFont(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetFontFamily">DrawGetFontFamily</a></h2>

<p>DrawGetFontFamily() returns the font family to use when annotating with text. The value returned must be freed by the user when it is no longer needed.</p>

<p>The format of the DrawGetFontFamily method is:</p>

<pre class="text">
char *DrawGetFontFamily(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetFontResolution">DrawGetFontResolution</a></h2>

<p>DrawGetFontResolution() gets the image X and Y resolution.</p>

<p>The format of the DrawGetFontResolution method is:</p>

<pre class="text">
MagickBooleanType DrawGetFontResolution(const DrawingWand *wand,
  double *x,double *y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the x-resolution. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the y-resolution. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetFontSize">DrawGetFontSize</a></h2>

<p>DrawGetFontSize() returns the font pointsize used when annotating with text.</p>

<p>The format of the DrawGetFontSize method is:</p>

<pre class="text">
double DrawGetFontSize(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetFontStretch">DrawGetFontStretch</a></h2>

<p>DrawGetFontStretch() returns the font stretch used when annotating with text.</p>

<p>The format of the DrawGetFontStretch method is:</p>

<pre class="text">
StretchType DrawGetFontStretch(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetFontStyle">DrawGetFontStyle</a></h2>

<p>DrawGetFontStyle() returns the font style used when annotating with text.</p>

<p>The format of the DrawGetFontStyle method is:</p>

<pre class="text">
StyleType DrawGetFontStyle(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetFontWeight">DrawGetFontWeight</a></h2>

<p>DrawGetFontWeight() returns the font weight used when annotating with text.</p>

<p>The format of the DrawGetFontWeight method is:</p>

<pre class="text">
size_t DrawGetFontWeight(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetGravity">DrawGetGravity</a></h2>

<p>DrawGetGravity() returns the text placement gravity used when annotating with text.</p>

<p>The format of the DrawGetGravity method is:</p>

<pre class="text">
GravityType DrawGetGravity(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetOpacity">DrawGetOpacity</a></h2>

<p>DrawGetOpacity() returns the alpha used when drawing with the fill or stroke color or texture.  Fully opaque is 1.0.</p>

<p>The format of the DrawGetOpacity method is:</p>

<pre class="text">
double DrawGetOpacity(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetStrokeAntialias">DrawGetStrokeAntialias</a></h2>

<p>DrawGetStrokeAntialias() returns the current stroke antialias setting. Stroked outlines are antialiased by default.  When antialiasing is disabled stroked pixels are thresholded to determine if the stroke color or underlying canvas color should be used.</p>

<p>The format of the DrawGetStrokeAntialias method is:</p>

<pre class="text">
MagickBooleanType DrawGetStrokeAntialias(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetStrokeColor">DrawGetStrokeColor</a></h2>

<p>DrawGetStrokeColor() returns the color used for stroking object outlines.</p>

<p>The format of the DrawGetStrokeColor method is:</p>

<pre class="text">
void DrawGetStrokeColor(const DrawingWand *wand,
  PixelWand *stroke_color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>stroke_color</dt>
<dd>Return the stroke color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetStrokeDashArray">DrawGetStrokeDashArray</a></h2>

<p>DrawGetStrokeDashArray() returns an array representing the pattern of dashes and gaps used to stroke paths (see DrawSetStrokeDashArray). The array must be freed once it is no longer required by the user.</p>

<p>The format of the DrawGetStrokeDashArray method is:</p>

<pre class="text">
double *DrawGetStrokeDashArray(const DrawingWand *wand,
  size_t *number_elements)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>number_elements</dt>
<dd>address to place number of elements in dash array </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetStrokeDashOffset">DrawGetStrokeDashOffset</a></h2>

<p>DrawGetStrokeDashOffset() returns the offset into the dash pattern to start the dash.</p>

<p>The format of the DrawGetStrokeDashOffset method is:</p>

<pre class="text">
double DrawGetStrokeDashOffset(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetStrokeLineCap">DrawGetStrokeLineCap</a></h2>

<p>DrawGetStrokeLineCap() returns the shape to be used at the end of open subpaths when they are stroked. Values of LineCap are UndefinedCap, ButtCap, RoundCap, and SquareCap.</p>

<p>The format of the DrawGetStrokeLineCap method is:</p>

<pre class="text">
LineCap DrawGetStrokeLineCap(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetStrokeLineJoin">DrawGetStrokeLineJoin</a></h2>

<p>DrawGetStrokeLineJoin() returns the shape to be used at the corners of paths (or other vector shapes) when they are stroked. Values of LineJoin are UndefinedJoin, MiterJoin, RoundJoin, and BevelJoin.</p>

<p>The format of the DrawGetStrokeLineJoin method is:</p>

<pre class="text">
LineJoin DrawGetStrokeLineJoin(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetStrokeMiterLimit">DrawGetStrokeMiterLimit</a></h2>

<p>DrawGetStrokeMiterLimit() returns the miter limit. When two line segments meet at a sharp angle and miter joins have been specified for 'lineJoin', it is possible for the miter to extend far beyond the thickness of the line stroking the path. The miterLimit' imposes a limit on the ratio of the miter length to the 'lineWidth'.</p>

<p>The format of the DrawGetStrokeMiterLimit method is:</p>

<pre class="text">
size_t DrawGetStrokeMiterLimit(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetStrokeOpacity">DrawGetStrokeOpacity</a></h2>

<p>DrawGetStrokeOpacity() returns the alpha of stroked object outlines.</p>

<p>The format of the DrawGetStrokeOpacity method is:</p>

<pre class="text">
double DrawGetStrokeOpacity(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetStrokeWidth">DrawGetStrokeWidth</a></h2>

<p>DrawGetStrokeWidth() returns the width of the stroke used to draw object outlines.</p>

<p>The format of the DrawGetStrokeWidth method is:</p>

<pre class="text">
double DrawGetStrokeWidth(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetTextAlignment">DrawGetTextAlignment</a></h2>

<p>DrawGetTextAlignment() returns the alignment applied when annotating with text.</p>

<p>The format of the DrawGetTextAlignment method is:</p>

<pre class="text">
AlignType DrawGetTextAlignment(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetTextAntialias">DrawGetTextAntialias</a></h2>

<p>DrawGetTextAntialias() returns the current text antialias setting, which determines whether text is antialiased.  Text is antialiased by default.</p>

<p>The format of the DrawGetTextAntialias method is:</p>

<pre class="text">
MagickBooleanType DrawGetTextAntialias(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetTextDecoration">DrawGetTextDecoration</a></h2>

<p>DrawGetTextDecoration() returns the decoration applied when annotating with text.</p>

<p>The format of the DrawGetTextDecoration method is:</p>

<pre class="text">
DecorationType DrawGetTextDecoration(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetTextDirection">DrawGetTextDirection</a></h2>

<p>DrawGetTextDirection() returns the direction that will be used when annotating with text.</p>

<p>The format of the DrawGetTextDirection method is:</p>

<pre class="text">
DirectionType DrawGetTextDirection(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetTextEncoding">DrawGetTextEncoding</a></h2>

<p>DrawGetTextEncoding() returns a null-terminated string which specifies the code set used for text annotations. The string must be freed by the user once it is no longer required.</p>

<p>The format of the DrawGetTextEncoding method is:</p>

<pre class="text">
char *DrawGetTextEncoding(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetTextKerning">DrawGetTextKerning</a></h2>

<p>DrawGetTextKerning() gets the spacing between characters in text.</p>

<p>The format of the DrawSetFontKerning method is:</p>

<pre class="text">
double DrawGetTextKerning(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetTextInterlineSpacing">DrawGetTextInterlineSpacing</a></h2>

<p>DrawGetTextInterlineSpacing() gets the spacing between lines in text.</p>

<p>The format of the DrawGetTextInterlineSpacing method is:</p>

<pre class="text">
double DrawGetTextInterlineSpacing(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetTextInterwordSpacing">DrawGetTextInterwordSpacing</a></h2>

<p>DrawGetTextInterwordSpacing() gets the spacing between words in text.</p>

<p>The format of the DrawSetFontKerning method is:</p>

<pre class="text">
double DrawGetTextInterwordSpacing(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetVectorGraphics">DrawGetVectorGraphics</a></h2>

<p>DrawGetVectorGraphics() returns a null-terminated string which specifies the vector graphics generated by any graphics calls made since the wand was instantiated.  The string must be freed by the user once it is no longer required.</p>

<p>The format of the DrawGetVectorGraphics method is:</p>

<pre class="text">
char *DrawGetVectorGraphics(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawGetTextUnderColor">DrawGetTextUnderColor</a></h2>

<p>DrawGetTextUnderColor() returns the color of a background rectangle to place under text annotations.</p>

<p>The format of the DrawGetTextUnderColor method is:</p>

<pre class="text">
void DrawGetTextUnderColor(const DrawingWand *wand,
  PixelWand *under_color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>under_color</dt>
<dd>Return the under color. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawLine">DrawLine</a></h2>

<p>DrawLine() draws a line on the image using the current stroke color, stroke alpha, and stroke width.</p>

<p>The format of the DrawLine method is:</p>

<pre class="text">
void DrawLine(DrawingWand *wand,const double sx,const double sy,
  const double ex,const double ey)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>sx</dt>
<dd>starting x ordinate </dd>

<dd> </dd>
<dt>sy</dt>
<dd>starting y ordinate </dd>

<dd> </dd>
<dt>ex</dt>
<dd>ending x ordinate </dd>

<dd> </dd>
<dt>ey</dt>
<dd>ending y ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathClose">DrawPathClose</a></h2>

<p>DrawPathClose() adds a path element to the current path which closes the current subpath by drawing a straight line from the current point to the current subpath's most recent starting point (usually, the most recent moveto point).</p>

<p>The format of the DrawPathClose method is:</p>

<pre class="text">
void DrawPathClose(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathCurveToAbsolute">DrawPathCurveToAbsolute</a></h2>

<p>DrawPathCurveToAbsolute() draws a cubic Bezier curve from the current point to (x,y) using (x1,y1) as the control point at the beginning of the curve and (x2,y2) as the control point at the end of the curve using absolute coordinates. At the end of the command, the new current point becomes the final (x,y) coordinate pair used in the polybezier.</p>

<p>The format of the DrawPathCurveToAbsolute method is:</p>

<pre class="text">
void DrawPathCurveToAbsolute(DrawingWand *wand,const double x1,
  const double y1,const double x2,const double y2,const double x,
  const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x1</dt>
<dd>x ordinate of control point for curve beginning </dd>

<dd> </dd>
<dt>y1</dt>
<dd>y ordinate of control point for curve beginning </dd>

<dd> </dd>
<dt>x2</dt>
<dd>x ordinate of control point for curve ending </dd>

<dd> </dd>
<dt>y2</dt>
<dd>y ordinate of control point for curve ending </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate of the end of the curve </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate of the end of the curve </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathCurveToRelative">DrawPathCurveToRelative</a></h2>

<p>DrawPathCurveToRelative() draws a cubic Bezier curve from the current point to (x,y) using (x1,y1) as the control point at the beginning of the curve and (x2,y2) as the control point at the end of the curve using relative coordinates. At the end of the command, the new current point becomes the final (x,y) coordinate pair used in the polybezier.</p>

<p>The format of the DrawPathCurveToRelative method is:</p>

<pre class="text">
void DrawPathCurveToRelative(DrawingWand *wand,const double x1,
  const double y1,const double x2,const double y2,const double x,
  const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x1</dt>
<dd>x ordinate of control point for curve beginning </dd>

<dd> </dd>
<dt>y1</dt>
<dd>y ordinate of control point for curve beginning </dd>

<dd> </dd>
<dt>x2</dt>
<dd>x ordinate of control point for curve ending </dd>

<dd> </dd>
<dt>y2</dt>
<dd>y ordinate of control point for curve ending </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate of the end of the curve </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate of the end of the curve </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathCurveToQuadraticBezierAbsolute">DrawPathCurveToQuadraticBezierAbsolute</a></h2>

<p>DrawPathCurveToQuadraticBezierAbsolute() draws a quadratic Bezier curve from the current point to (x,y) using (x1,y1) as the control point using absolute coordinates. At the end of the command, the new current point becomes the final (x,y) coordinate pair used in the polybezier.</p>

<p>The format of the DrawPathCurveToQuadraticBezierAbsolute method is:</p>

<pre class="text">
void DrawPathCurveToQuadraticBezierAbsolute(DrawingWand *wand,
  const double x1,const double y1,onst double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x1</dt>
<dd>x ordinate of the control point </dd>

<dd> </dd>
<dt>y1</dt>
<dd>y ordinate of the control point </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate of final point </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate of final point </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathCurveToQuadraticBezierRelative">DrawPathCurveToQuadraticBezierRelative</a></h2>

<p>DrawPathCurveToQuadraticBezierRelative() draws a quadratic Bezier curve from the current point to (x,y) using (x1,y1) as the control point using relative coordinates. At the end of the command, the new current point becomes the final (x,y) coordinate pair used in the polybezier.</p>

<p>The format of the DrawPathCurveToQuadraticBezierRelative method is:</p>

<pre class="text">
void DrawPathCurveToQuadraticBezierRelative(DrawingWand *wand,
  const double x1,const double y1,const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x1</dt>
<dd>x ordinate of the control point </dd>

<dd> </dd>
<dt>y1</dt>
<dd>y ordinate of the control point </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate of final point </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate of final point </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathCurveToQuadraticBezierSmoothAbsolute">DrawPathCurveToQuadraticBezierSmoothAbsolute</a></h2>

<p>DrawPathCurveToQuadraticBezierSmoothAbsolute() draws a quadratic Bezier curve (using absolute coordinates) from the current point to (x,y). The control point is assumed to be the reflection of the control point on the previous command relative to the current point. (If there is no previous command or if the previous command was not a DrawPathCurveToQuadraticBezierAbsolute, DrawPathCurveToQuadraticBezierRelative, DrawPathCurveToQuadraticBezierSmoothAbsolute or DrawPathCurveToQuadraticBezierSmoothRelative, assume the control point is coincident with the current point.). At the end of the command, the new current point becomes the final (x,y) coordinate pair used in the polybezier.</p>

<p>The format of the DrawPathCurveToQuadraticBezierSmoothAbsolute method is:</p>

<pre class="text">
void DrawPathCurveToQuadraticBezierSmoothAbsolute(
  DrawingWand *wand,const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate of final point </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate of final point </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathCurveToQuadraticBezierSmoothRelative">DrawPathCurveToQuadraticBezierSmoothRelative</a></h2>

<p>DrawPathCurveToQuadraticBezierSmoothRelative() draws a quadratic Bezier curve (using relative coordinates) from the current point to (x,y). The control point is assumed to be the reflection of the control point on the previous command relative to the current point. (If there is no previous command or if the previous command was not a DrawPathCurveToQuadraticBezierAbsolute, DrawPathCurveToQuadraticBezierRelative, DrawPathCurveToQuadraticBezierSmoothAbsolute or DrawPathCurveToQuadraticBezierSmoothRelative, assume the control point is coincident with the current point.). At the end of the command, the new current point becomes the final (x,y) coordinate pair used in the polybezier.</p>

<p>The format of the DrawPathCurveToQuadraticBezierSmoothRelative method is:</p>

<pre class="text">
void DrawPathCurveToQuadraticBezierSmoothRelative(DrawingWand *wand,
  const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate of final point </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate of final point </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathCurveToSmoothAbsolute">DrawPathCurveToSmoothAbsolute</a></h2>

<p>DrawPathCurveToSmoothAbsolute() draws a cubic Bezier curve from the current point to (x,y) using absolute coordinates. The first control point is assumed to be the reflection of the second control point on the previous command relative to the current point. (If there is no previous command or if the previous command was not an DrawPathCurveToAbsolute, DrawPathCurveToRelative, DrawPathCurveToSmoothAbsolute or DrawPathCurveToSmoothRelative, assume the first control point is coincident with the current point.) (x2,y2) is the second control point (i.e., the control point at the end of the curve). At the end of the command, the new current point becomes the final (x,y) coordinate pair used in the polybezier.</p>

<p>The format of the DrawPathCurveToSmoothAbsolute method is:</p>

<pre class="text">
void DrawPathCurveToSmoothAbsolute(DrawingWand *wand,
  const double x2,const double y2,const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x2</dt>
<dd>x ordinate of second control point </dd>

<dd> </dd>
<dt>y2</dt>
<dd>y ordinate of second control point </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate of termination point </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate of termination point </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathCurveToSmoothRelative">DrawPathCurveToSmoothRelative</a></h2>

<p>DrawPathCurveToSmoothRelative() draws a cubic Bezier curve from the current point to (x,y) using relative coordinates. The first control point is assumed to be the reflection of the second control point on the previous command relative to the current point. (If there is no previous command or if the previous command was not an DrawPathCurveToAbsolute, DrawPathCurveToRelative, DrawPathCurveToSmoothAbsolute or DrawPathCurveToSmoothRelative, assume the first control point is coincident with the current point.) (x2,y2) is the second control point (i.e., the control point at the end of the curve). At the end of the command, the new current point becomes the final (x,y) coordinate pair used in the polybezier.</p>

<p>The format of the DrawPathCurveToSmoothRelative method is:</p>

<pre class="text">
void DrawPathCurveToSmoothRelative(DrawingWand *wand,
  const double x2,const double y2,const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x2</dt>
<dd>x ordinate of second control point </dd>

<dd> </dd>
<dt>y2</dt>
<dd>y ordinate of second control point </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate of termination point </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate of termination point </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathEllipticArcAbsolute">DrawPathEllipticArcAbsolute</a></h2>

<p>DrawPathEllipticArcAbsolute() draws an elliptical arc from the current point to (x, y) using absolute coordinates. The size and orientation of the ellipse are defined by two radii (rx, ry) and an xAxisRotation, which indicates how the ellipse as a whole is rotated relative to the current coordinate system. The center (cx, cy) of the ellipse is calculated automagically to satisfy the constraints imposed by the other parameters. largeArcFlag and sweepFlag contribute to the automatic calculations and help determine how the arc is drawn. If largeArcFlag is true then draw the larger of the available arcs. If sweepFlag is true, then draw the arc matching a clock-wise rotation.</p>

<p>The format of the DrawPathEllipticArcAbsolute method is:</p>

<pre class="text">
void DrawPathEllipticArcAbsolute(DrawingWand *wand,
  const double rx,const double ry,const double x_axis_rotation,
  const MagickBooleanType large_arc_flag,
  const MagickBooleanType sweep_flag,const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>rx</dt>
<dd>x radius </dd>

<dd> </dd>
<dt>ry</dt>
<dd>y radius </dd>

<dd> </dd>
<dt>x_axis_rotation</dt>
<dd>indicates how the ellipse as a whole is rotated relative to the current coordinate system </dd>

<dd> </dd>
<dt>large_arc_flag</dt>
<dd>If non-zero (true) then draw the larger of the available arcs </dd>

<dd> </dd>
<dt>sweep_flag</dt>
<dd>If non-zero (true) then draw the arc matching a clock-wise rotation </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathEllipticArcRelative">DrawPathEllipticArcRelative</a></h2>

<p>DrawPathEllipticArcRelative() draws an elliptical arc from the current point to (x, y) using relative coordinates. The size and orientation of the ellipse are defined by two radii (rx, ry) and an xAxisRotation, which indicates how the ellipse as a whole is rotated relative to the current coordinate system. The center (cx, cy) of the ellipse is calculated automagically to satisfy the constraints imposed by the other parameters. largeArcFlag and sweepFlag contribute to the automatic calculations and help determine how the arc is drawn. If largeArcFlag is true then draw the larger of the available arcs. If sweepFlag is true, then draw the arc matching a clock-wise rotation.</p>

<p>The format of the DrawPathEllipticArcRelative method is:</p>

<pre class="text">
void DrawPathEllipticArcRelative(DrawingWand *wand,
  const double rx,const double ry,const double x_axis_rotation,
  const MagickBooleanType large_arc_flag,
  const MagickBooleanType sweep_flag,const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>rx</dt>
<dd>x radius </dd>

<dd> </dd>
<dt>ry</dt>
<dd>y radius </dd>

<dd> </dd>
<dt>x_axis_rotation</dt>
<dd>indicates how the ellipse as a whole is rotated relative to the current coordinate system </dd>

<dd> </dd>
<dt>large_arc_flag</dt>
<dd>If non-zero (true) then draw the larger of the available arcs </dd>

<dd> </dd>
<dt>sweep_flag</dt>
<dd>If non-zero (true) then draw the arc matching a clock-wise rotation </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathFinish">DrawPathFinish</a></h2>

<p>DrawPathFinish() terminates the current path.</p>

<p>The format of the DrawPathFinish method is:</p>

<pre class="text">
void DrawPathFinish(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathLineToAbsolute">DrawPathLineToAbsolute</a></h2>

<p>DrawPathLineToAbsolute() draws a line path from the current point to the given coordinate using absolute coordinates. The coordinate then becomes the new current point.</p>

<p>The format of the DrawPathLineToAbsolute method is:</p>

<pre class="text">
void DrawPathLineToAbsolute(DrawingWand *wand,const double x,
  const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>target x ordinate </dd>

<dd> </dd>
<dt>y</dt>
<dd>target y ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathLineToRelative">DrawPathLineToRelative</a></h2>

<p>DrawPathLineToRelative() draws a line path from the current point to the given coordinate using relative coordinates. The coordinate then becomes the new current point.</p>

<p>The format of the DrawPathLineToRelative method is:</p>

<pre class="text">
void DrawPathLineToRelative(DrawingWand *wand,const double x,
  const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>target x ordinate </dd>

<dd> </dd>
<dt>y</dt>
<dd>target y ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathLineToHorizontalAbsolute">DrawPathLineToHorizontalAbsolute</a></h2>

<p>DrawPathLineToHorizontalAbsolute() draws a horizontal line path from the current point to the target point using absolute coordinates.  The target point then becomes the new current point.</p>

<p>The format of the DrawPathLineToHorizontalAbsolute method is:</p>

<pre class="text">
void DrawPathLineToHorizontalAbsolute(DrawingWand *wand,const double x)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>target x ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathLineToHorizontalRelative">DrawPathLineToHorizontalRelative</a></h2>

<p>DrawPathLineToHorizontalRelative() draws a horizontal line path from the current point to the target point using relative coordinates.  The target point then becomes the new current point.</p>

<p>The format of the DrawPathLineToHorizontalRelative method is:</p>

<pre class="text">
void DrawPathLineToHorizontalRelative(DrawingWand *wand,
  const double x)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>target x ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathLineToVerticalAbsolute">DrawPathLineToVerticalAbsolute</a></h2>

<p>DrawPathLineToVerticalAbsolute() draws a vertical line path from the current point to the target point using absolute coordinates.  The target point then becomes the new current point.</p>

<p>The format of the DrawPathLineToVerticalAbsolute method is:</p>

<pre class="text">
void DrawPathLineToVerticalAbsolute(DrawingWand *wand,
  const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>y</dt>
<dd>target y ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathLineToVerticalRelative">DrawPathLineToVerticalRelative</a></h2>

<p>DrawPathLineToVerticalRelative() draws a vertical line path from the current point to the target point using relative coordinates.  The target point then becomes the new current point.</p>

<p>The format of the DrawPathLineToVerticalRelative method is:</p>

<pre class="text">
void DrawPathLineToVerticalRelative(DrawingWand *wand,
  const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>y</dt>
<dd>target y ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathMoveToAbsolute">DrawPathMoveToAbsolute</a></h2>

<p>DrawPathMoveToAbsolute() starts a new sub-path at the given coordinate using absolute coordinates. The current point then becomes the specified coordinate.</p>

<p>The format of the DrawPathMoveToAbsolute method is:</p>

<pre class="text">
void DrawPathMoveToAbsolute(DrawingWand *wand,const double x,
  const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>target x ordinate </dd>

<dd> </dd>
<dt>y</dt>
<dd>target y ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathMoveToRelative">DrawPathMoveToRelative</a></h2>

<p>DrawPathMoveToRelative() starts a new sub-path at the given coordinate using relative coordinates. The current point then becomes the specified coordinate.</p>

<p>The format of the DrawPathMoveToRelative method is:</p>

<pre class="text">
void DrawPathMoveToRelative(DrawingWand *wand,const double x,
  const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>target x ordinate </dd>

<dd> </dd>
<dt>y</dt>
<dd>target y ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPathStart">DrawPathStart</a></h2>

<p>DrawPathStart() declares the start of a path drawing list which is terminated by a matching DrawPathFinish() command. All other DrawPath commands must be enclosed between a DrawPathStart() and a DrawPathFinish() command. This is because path drawing commands are subordinate commands and they do not function by themselves.</p>

<p>The format of the DrawPathStart method is:</p>

<pre class="text">
void DrawPathStart(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPoint">DrawPoint</a></h2>

<p>DrawPoint() draws a point using the current fill color.</p>

<p>The format of the DrawPoint method is:</p>

<pre class="text">
void DrawPoint(DrawingWand *wand,const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>target x coordinate </dd>

<dd> </dd>
<dt>y</dt>
<dd>target y coordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPolygon">DrawPolygon</a></h2>

<p>DrawPolygon() draws a polygon using the current stroke, stroke width, and fill color or texture, using the specified array of coordinates.</p>

<p>The format of the DrawPolygon method is:</p>

<pre class="text">
void DrawPolygon(DrawingWand *wand,
  const size_t number_coordinates,const PointInfo *coordinates)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>number_coordinates</dt>
<dd>number of coordinates </dd>

<dd> </dd>
<dt>coordinates</dt>
<dd>coordinate array </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPolyline">DrawPolyline</a></h2>

<p>DrawPolyline() draws a polyline using the current stroke, stroke width, and fill color or texture, using the specified array of coordinates.</p>

<p>The format of the DrawPolyline method is:</p>

<pre class="text">
void DrawPolyline(DrawingWand *wand,
  const size_t number_coordinates,const PointInfo *coordinates)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>number_coordinates</dt>
<dd>number of coordinates </dd>

<dd> </dd>
<dt>coordinates</dt>
<dd>coordinate array </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPopClipPath">DrawPopClipPath</a></h2>

<p>DrawPopClipPath() terminates a clip path definition.</p>

<p>The format of the DrawPopClipPath method is:</p>

<pre class="text">
void DrawPopClipPath(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPopDefs">DrawPopDefs</a></h2>

<p>DrawPopDefs() terminates a definition list.</p>

<p>The format of the DrawPopDefs method is:</p>

<pre class="text">
void DrawPopDefs(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPopPattern">DrawPopPattern</a></h2>

<p>DrawPopPattern() terminates a pattern definition.</p>

<p>The format of the DrawPopPattern method is:</p>

<pre class="text">
MagickBooleanType DrawPopPattern(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPushClipPath">DrawPushClipPath</a></h2>

<p>DrawPushClipPath() starts a clip path definition which is comprized of any number of drawing commands and terminated by a DrawPopClipPath() command.</p>

<p>The format of the DrawPushClipPath method is:</p>

<pre class="text">
void DrawPushClipPath(DrawingWand *wand,const char *clip_mask_id)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>clip_mask_id</dt>
<dd>string identifier to associate with the clip path for later use. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPushDefs">DrawPushDefs</a></h2>

<p>DrawPushDefs() indicates that commands up to a terminating DrawPopDefs() command create named elements (e.g. clip-paths, textures, etc.) which may safely be processed earlier for the sake of efficiency.</p>

<p>The format of the DrawPushDefs method is:</p>

<pre class="text">
void DrawPushDefs(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawPushPattern">DrawPushPattern</a></h2>

<p>DrawPushPattern() indicates that subsequent commands up to a DrawPopPattern() command comprise the definition of a named pattern. The pattern space is assigned top left corner coordinates, a width and height, and becomes its own drawing space.  Anything which can be drawn may be used in a pattern definition. Named patterns may be used as stroke or brush definitions.</p>

<p>The format of the DrawPushPattern method is:</p>

<pre class="text">
MagickBooleanType DrawPushPattern(DrawingWand *wand,
  const char *pattern_id,const double x,const double y,
  const double width,const double height)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>pattern_id</dt>
<dd>pattern identification for later reference </dd>

<dd> </dd>
<dt>x</dt>
<dd>x ordinate of top left corner </dd>

<dd> </dd>
<dt>y</dt>
<dd>y ordinate of top left corner </dd>

<dd> </dd>
<dt>width</dt>
<dd>width of pattern space </dd>

<dd> </dd>
<dt>height</dt>
<dd>height of pattern space </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawRectangle">DrawRectangle</a></h2>

<p>DrawRectangle() draws a rectangle given two coordinates and using the current stroke, stroke width, and fill settings.</p>

<p>The format of the DrawRectangle method is:</p>

<pre class="text">
void DrawRectangle(DrawingWand *wand,const double x1,
  const double y1,const double x2,const double y2)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>x1</dt>
<dd>x ordinate of first coordinate </dd>

<dd> </dd>
<dt>y1</dt>
<dd>y ordinate of first coordinate </dd>

<dd> </dd>
<dt>x2</dt>
<dd>x ordinate of second coordinate </dd>

<dd> </dd>
<dt>y2</dt>
<dd>y ordinate of second coordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawResetVectorGraphics">DrawResetVectorGraphics</a></h2>

<p>DrawResetVectorGraphics() resets the vector graphics associated with the specified wand.</p>

<p>The format of the DrawResetVectorGraphics method is:</p>

<pre class="text">
void DrawResetVectorGraphics(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawRotate">DrawRotate</a></h2>

<p>DrawRotate() applies the specified rotation to the current coordinate space.</p>

<p>The format of the DrawRotate method is:</p>

<pre class="text">
void DrawRotate(DrawingWand *wand,const double degrees)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>degrees</dt>
<dd>degrees of rotation </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawRoundRectangle">DrawRoundRectangle</a></h2>

<p>DrawRoundRectangle() draws a rounted rectangle given two coordinates, x &amp; y corner radiuses and using the current stroke, stroke width, and fill settings.</p>

<p>The format of the DrawRoundRectangle method is:</p>

<pre class="text">
void DrawRoundRectangle(DrawingWand *wand,double x1,double y1,
  double x2,double y2,double rx,double ry)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x1</dt>
<dd>x ordinate of first coordinate </dd>

<dd> </dd>
<dt>y1</dt>
<dd>y ordinate of first coordinate </dd>

<dd> </dd>
<dt>x2</dt>
<dd>x ordinate of second coordinate </dd>

<dd> </dd>
<dt>y2</dt>
<dd>y ordinate of second coordinate </dd>

<dd> </dd>
<dt>rx</dt>
<dd>radius of corner in horizontal direction </dd>

<dd> </dd>
<dt>ry</dt>
<dd>radius of corner in vertical direction </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawScale">DrawScale</a></h2>

<p>DrawScale() adjusts the scaling factor to apply in the horizontal and vertical directions to the current coordinate space.</p>

<p>The format of the DrawScale method is:</p>

<pre class="text">
void DrawScale(DrawingWand *wand,const double x,const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>horizontal scale factor </dd>

<dd> </dd>
<dt>y</dt>
<dd>vertical scale factor </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetBorderColor">DrawSetBorderColor</a></h2>

<p>DrawSetBorderColor() sets the border color to be used for drawing bordered objects.</p>

<p>The format of the DrawSetBorderColor method is:</p>

<pre class="text">
void DrawSetBorderColor(DrawingWand *wand,const PixelWand *border_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>border_wand</dt>
<dd>border wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetClipPath">DrawSetClipPath</a></h2>

<p>DrawSetClipPath() associates a named clipping path with the image.  Only the areas drawn on by the clipping path will be modified as ssize_t as it remains in effect.</p>

<p>The format of the DrawSetClipPath method is:</p>

<pre class="text">
MagickBooleanType DrawSetClipPath(DrawingWand *wand,
  const char *clip_mask)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>clip_mask</dt>
<dd>name of clipping path to associate with image </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetClipRule">DrawSetClipRule</a></h2>

<p>DrawSetClipRule() set the polygon fill rule to be used by the clipping path.</p>

<p>The format of the DrawSetClipRule method is:</p>

<pre class="text">
void DrawSetClipRule(DrawingWand *wand,const FillRule fill_rule)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>fill_rule</dt>
<dd>fill rule (EvenOddRule or NonZeroRule) </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetClipUnits">DrawSetClipUnits</a></h2>

<p>DrawSetClipUnits() sets the interpretation of clip path units.</p>

<p>The format of the DrawSetClipUnits method is:</p>

<pre class="text">
void DrawSetClipUnits(DrawingWand *wand,
  const ClipPathUnits clip_units)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>clip_units</dt>
<dd>units to use (UserSpace, UserSpaceOnUse, or ObjectBoundingBox) </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetDensity">DrawSetDensity</a></h2>

<p>DrawSetDensity() sets the vertical and horizontal resolution.</p>

<p>The format of the DrawSetDensity method is:</p>

<pre class="text">
MagickBooleanType DrawSetDensity(DrawingWand *wand,
  const char *density)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>density</dt>
<dd>the vertical and horizontal resolution. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFillColor">DrawSetFillColor</a></h2>

<p>DrawSetFillColor() sets the fill color to be used for drawing filled objects.</p>

<p>The format of the DrawSetFillColor method is:</p>

<pre class="text">
void DrawSetFillColor(DrawingWand *wand,const PixelWand *fill_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>fill_wand</dt>
<dd>fill wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFillOpacity">DrawSetFillOpacity</a></h2>

<p>DrawSetFillOpacity() sets the alpha to use when drawing using the fill color or fill texture.  Fully opaque is 1.0.</p>

<p>The format of the DrawSetFillOpacity method is:</p>

<pre class="text">
void DrawSetFillOpacity(DrawingWand *wand,const double fill_alpha)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>fill_opacity</dt>
<dd>fill opacity </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFontResolution">DrawSetFontResolution</a></h2>

<p>DrawSetFontResolution() sets the image resolution.</p>

<p>The format of the DrawSetFontResolution method is:</p>

<pre class="text">
MagickBooleanType DrawSetFontResolution(DrawingWand *wand,
  const double x_resolution,const double y_resolution)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x_resolution</dt>
<dd>the image x resolution. </dd>

<dd> </dd>
<dt>y_resolution</dt>
<dd>the image y resolution. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetOpacity">DrawSetOpacity</a></h2>

<p>DrawSetOpacity() sets the alpha to use when drawing using the fill or stroke color or texture.  Fully opaque is 1.0.</p>

<p>The format of the DrawSetOpacity method is:</p>

<pre class="text">
void DrawSetOpacity(DrawingWand *wand,const double alpha)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>opacity</dt>
<dd>fill and stroke opacity.  The value 1.0 is opaque. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFillPatternURL">DrawSetFillPatternURL</a></h2>

<p>DrawSetFillPatternURL() sets the URL to use as a fill pattern for filling objects. Only local URLs ("#identifier") are supported at this time. These local URLs are normally created by defining a named fill pattern with DrawPushPattern/DrawPopPattern.</p>

<p>The format of the DrawSetFillPatternURL method is:</p>

<pre class="text">
MagickBooleanType DrawSetFillPatternURL(DrawingWand *wand,
  const char *fill_url)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>fill_url</dt>
<dd>URL to use to obtain fill pattern. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFillRule">DrawSetFillRule</a></h2>

<p>DrawSetFillRule() sets the fill rule to use while drawing polygons.</p>

<p>The format of the DrawSetFillRule method is:</p>

<pre class="text">
void DrawSetFillRule(DrawingWand *wand,const FillRule fill_rule)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>fill_rule</dt>
<dd>fill rule (EvenOddRule or NonZeroRule) </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFont">DrawSetFont</a></h2>

<p>DrawSetFont() sets the fully-sepecified font to use when annotating with text.</p>

<p>The format of the DrawSetFont method is:</p>

<pre class="text">
MagickBooleanType DrawSetFont(DrawingWand *wand,const char *font_name)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>font_name</dt>
<dd>font name </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFontFamily">DrawSetFontFamily</a></h2>

<p>DrawSetFontFamily() sets the font family to use when annotating with text.</p>

<p>The format of the DrawSetFontFamily method is:</p>

<pre class="text">
MagickBooleanType DrawSetFontFamily(DrawingWand *wand,
  const char *font_family)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>font_family</dt>
<dd>font family </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFontSize">DrawSetFontSize</a></h2>

<p>DrawSetFontSize() sets the font pointsize to use when annotating with text.</p>

<p>The format of the DrawSetFontSize method is:</p>

<pre class="text">
void DrawSetFontSize(DrawingWand *wand,const double pointsize)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>pointsize</dt>
<dd>text pointsize </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFontStretch">DrawSetFontStretch</a></h2>

<p>DrawSetFontStretch() sets the font stretch to use when annotating with text. The AnyStretch enumeration acts as a wild-card "don't care" option.</p>

<p>The format of the DrawSetFontStretch method is:</p>

<pre class="text">
void DrawSetFontStretch(DrawingWand *wand,
  const StretchType font_stretch)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>font_stretch</dt>
<dd>font stretch (NormalStretch, UltraCondensedStretch, CondensedStretch, SemiCondensedStretch, SemiExpandedStretch, ExpandedStretch, ExtraExpandedStretch, UltraExpandedStretch, AnyStretch) </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFontStyle">DrawSetFontStyle</a></h2>

<p>DrawSetFontStyle() sets the font style to use when annotating with text. The AnyStyle enumeration acts as a wild-card "don't care" option.</p>

<p>The format of the DrawSetFontStyle method is:</p>

<pre class="text">
void DrawSetFontStyle(DrawingWand *wand,const StyleType style)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>style</dt>
<dd>font style (NormalStyle, ItalicStyle, ObliqueStyle, AnyStyle) </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetFontWeight">DrawSetFontWeight</a></h2>

<p>DrawSetFontWeight() sets the font weight to use when annotating with text.</p>

<p>The format of the DrawSetFontWeight method is:</p>

<pre class="text">
void DrawSetFontWeight(DrawingWand *wand,
  const size_t font_weight)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>font_weight</dt>
<dd>font weight (valid range 100-900) </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetGravity">DrawSetGravity</a></h2>

<p>DrawSetGravity() sets the text placement gravity to use when annotating with text.</p>

<p>The format of the DrawSetGravity method is:</p>

<pre class="text">
void DrawSetGravity(DrawingWand *wand,const GravityType gravity)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>gravity</dt>
<dd>positioning gravity (NorthWestGravity, NorthGravity, NorthEastGravity, WestGravity, CenterGravity, EastGravity, SouthWestGravity, SouthGravity, SouthEastGravity) </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetStrokeColor">DrawSetStrokeColor</a></h2>

<p>DrawSetStrokeColor() sets the color used for stroking object outlines.</p>

<p>The format of the DrawSetStrokeColor method is:</p>

<pre class="text">
void DrawSetStrokeColor(DrawingWand *wand,
  const PixelWand *stroke_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>stroke_wand</dt>
<dd>stroke wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetStrokePatternURL">DrawSetStrokePatternURL</a></h2>

<p>DrawSetStrokePatternURL() sets the pattern used for stroking object outlines.</p>

<p>The format of the DrawSetStrokePatternURL method is:</p>

<pre class="text">
MagickBooleanType DrawSetStrokePatternURL(DrawingWand *wand,
  const char *stroke_url)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>stroke_url</dt>
<dd>URL specifying pattern ID (e.g. "#pattern_id") </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetStrokeAntialias">DrawSetStrokeAntialias</a></h2>

<p>DrawSetStrokeAntialias() controls whether stroked outlines are antialiased. Stroked outlines are antialiased by default.  When antialiasing is disabled stroked pixels are thresholded to determine if the stroke color or underlying canvas color should be used.</p>

<p>The format of the DrawSetStrokeAntialias method is:</p>

<pre class="text">
void DrawSetStrokeAntialias(DrawingWand *wand,
  const MagickBooleanType stroke_antialias)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>stroke_antialias</dt>
<dd>set to false (zero) to disable antialiasing </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetStrokeDashArray">DrawSetStrokeDashArray</a></h2>

<p>DrawSetStrokeDashArray() specifies the pattern of dashes and gaps used to stroke paths. The stroke dash array represents an array of numbers that specify the lengths of alternating dashes and gaps in pixels. If an odd number of values is provided, then the list of values is repeated to yield an even number of values. To remove an existing dash array, pass a zero number_elements argument and null dasharray.  A typical stroke dash array might contain the members 5 3 2.</p>

<p>The format of the DrawSetStrokeDashArray method is:</p>

<pre class="text">
MagickBooleanType DrawSetStrokeDashArray(DrawingWand *wand,
  const size_t number_elements,const double *dasharray)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>number_elements</dt>
<dd>number of elements in dash array </dd>

<dd> </dd>
<dt>dasharray</dt>
<dd>dash array values </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetStrokeDashOffset">DrawSetStrokeDashOffset</a></h2>

<p>DrawSetStrokeDashOffset() specifies the offset into the dash pattern to start the dash.</p>

<p>The format of the DrawSetStrokeDashOffset method is:</p>

<pre class="text">
void DrawSetStrokeDashOffset(DrawingWand *wand,
  const double dash_offset)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>dash_offset</dt>
<dd>dash offset </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetStrokeLineCap">DrawSetStrokeLineCap</a></h2>

<p>DrawSetStrokeLineCap() specifies the shape to be used at the end of open subpaths when they are stroked. Values of LineCap are UndefinedCap, ButtCap, RoundCap, and SquareCap.</p>

<p>The format of the DrawSetStrokeLineCap method is:</p>

<pre class="text">
void DrawSetStrokeLineCap(DrawingWand *wand,
  const LineCap linecap)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>linecap</dt>
<dd>linecap style </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetStrokeLineJoin">DrawSetStrokeLineJoin</a></h2>

<p>DrawSetStrokeLineJoin() specifies the shape to be used at the corners of paths (or other vector shapes) when they are stroked. Values of LineJoin are UndefinedJoin, MiterJoin, RoundJoin, and BevelJoin.</p>

<p>The format of the DrawSetStrokeLineJoin method is:</p>

<pre class="text">
void DrawSetStrokeLineJoin(DrawingWand *wand,
  const LineJoin linejoin)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>linejoin</dt>
<dd>line join style </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetStrokeMiterLimit">DrawSetStrokeMiterLimit</a></h2>

<p>DrawSetStrokeMiterLimit() specifies the miter limit. When two line segments meet at a sharp angle and miter joins have been specified for 'lineJoin', it is possible for the miter to extend far beyond the thickness of the line stroking the path. The miterLimit' imposes a limit on the ratio of the miter length to the 'lineWidth'.</p>

<p>The format of the DrawSetStrokeMiterLimit method is:</p>

<pre class="text">
void DrawSetStrokeMiterLimit(DrawingWand *wand,
  const size_t miterlimit)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>miterlimit</dt>
<dd>miter limit </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetStrokeOpacity">DrawSetStrokeOpacity</a></h2>

<p>DrawSetStrokeOpacity() specifies the alpha of stroked object outlines.</p>

<p>The format of the DrawSetStrokeOpacity method is:</p>

<pre class="text">
void DrawSetStrokeOpacity(DrawingWand *wand,
  const double stroke_alpha)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>opacity</dt>
<dd>stroke opacity.  The value 1.0 is opaque. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetStrokeWidth">DrawSetStrokeWidth</a></h2>

<p>DrawSetStrokeWidth() sets the width of the stroke used to draw object outlines.</p>

<p>The format of the DrawSetStrokeWidth method is:</p>

<pre class="text">
void DrawSetStrokeWidth(DrawingWand *wand,
  const double stroke_width)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>stroke_width</dt>
<dd>stroke width </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetTextAlignment">DrawSetTextAlignment</a></h2>

<p>DrawSetTextAlignment() specifies a text alignment to be applied when annotating with text.</p>

<p>The format of the DrawSetTextAlignment method is:</p>

<pre class="text">
void DrawSetTextAlignment(DrawingWand *wand,const AlignType alignment)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>alignment</dt>
<dd>text alignment.  One of UndefinedAlign, LeftAlign, CenterAlign, or RightAlign. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetTextAntialias">DrawSetTextAntialias</a></h2>

<p>DrawSetTextAntialias() controls whether text is antialiased.  Text is antialiased by default.</p>

<p>The format of the DrawSetTextAntialias method is:</p>

<pre class="text">
void DrawSetTextAntialias(DrawingWand *wand,
  const MagickBooleanType text_antialias)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>text_antialias</dt>
<dd>antialias boolean. Set to false (0) to disable antialiasing. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetTextDecoration">DrawSetTextDecoration</a></h2>

<p>DrawSetTextDecoration() specifies a decoration to be applied when annotating with text.</p>

<p>The format of the DrawSetTextDecoration method is:</p>

<pre class="text">
void DrawSetTextDecoration(DrawingWand *wand,
  const DecorationType decoration)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>decoration</dt>
<dd>text decoration.  One of NoDecoration, UnderlineDecoration, OverlineDecoration, or LineThroughDecoration </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetTextEncoding">DrawSetTextEncoding</a></h2>

<p>DrawSetTextEncoding() specifies the code set to use for text annotations. The only character encoding which may be specified at this time is "UTF-8" for representing Unicode as a sequence of bytes. Specify an empty string to set text encoding to the system's default. Successful text annotation using Unicode may require fonts designed to support Unicode.</p>

<p>The format of the DrawSetTextEncoding method is:</p>

<pre class="text">
void DrawSetTextEncoding(DrawingWand *wand,const char *encoding)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>encoding</dt>
<dd>character string specifying text encoding </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetTextKerning">DrawSetTextKerning</a></h2>

<p>DrawSetTextKerning() sets the spacing between characters in text.</p>

<p>The format of the DrawSetTextKerning method is:</p>

<pre class="text">
void DrawSetTextKerning(DrawingWand *wand,const double kerning)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>kerning</dt>
<dd>text kerning </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetTextInterlineSpacing">DrawSetTextInterlineSpacing</a></h2>

<p>DrawSetTextInterlineSpacing() sets the spacing between line in text.</p>

<p>The format of the DrawSetInterlineSpacing method is:</p>

<pre class="text">
void DrawSetTextInterlineSpacing(DrawingWand *wand,
  const double interline_spacing)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>interline_spacing</dt>
<dd>text line spacing </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetTextInterwordSpacing">DrawSetTextInterwordSpacing</a></h2>

<p>DrawSetTextInterwordSpacing() sets the spacing between words in text.</p>

<p>The format of the DrawSetInterwordSpacing method is:</p>

<pre class="text">
void DrawSetTextInterwordSpacing(DrawingWand *wand,
  const double interword_spacing)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>interword_spacing</dt>
<dd>text word spacing </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetTextUnderColor">DrawSetTextUnderColor</a></h2>

<p>DrawSetTextUnderColor() specifies the color of a background rectangle to place under text annotations.</p>

<p>The format of the DrawSetTextUnderColor method is:</p>

<pre class="text">
void DrawSetTextUnderColor(DrawingWand *wand,
  const PixelWand *under_wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>under_wand</dt>
<dd>text under wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetVectorGraphics">DrawSetVectorGraphics</a></h2>

<p>DrawSetVectorGraphics() sets the vector graphics associated with the specified wand.  Use this method with DrawGetVectorGraphics() as a method to persist the vector graphics state.</p>

<p>The format of the DrawSetVectorGraphics method is:</p>

<pre class="text">
MagickBooleanType DrawSetVectorGraphics(DrawingWand *wand,
  const char *xml)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>xml</dt>
<dd>the drawing wand XML. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSkewX">DrawSkewX</a></h2>

<p>DrawSkewX() skews the current coordinate system in the horizontal direction.</p>

<p>The format of the DrawSkewX method is:</p>

<pre class="text">
void DrawSkewX(DrawingWand *wand,const double degrees)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>degrees</dt>
<dd>number of degrees to skew the coordinates </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSkewY">DrawSkewY</a></h2>

<p>DrawSkewY() skews the current coordinate system in the vertical direction.</p>

<p>The format of the DrawSkewY method is:</p>

<pre class="text">
void DrawSkewY(DrawingWand *wand,const double degrees)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>degrees</dt>
<dd>number of degrees to skew the coordinates </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawTranslate">DrawTranslate</a></h2>

<p>DrawTranslate() applies a translation to the current coordinate system which moves the coordinate system origin to the specified coordinate.</p>

<p>The format of the DrawTranslate method is:</p>

<pre class="text">
void DrawTranslate(DrawingWand *wand,const double x,
  const double y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>new x ordinate for coordinate system origin </dd>

<dd> </dd>
<dt>y</dt>
<dd>new y ordinate for coordinate system origin </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="DrawSetViewbox">DrawSetViewbox</a></h2>

<p>DrawSetViewbox() sets the overall canvas size to be recorded with the drawing vector data.  Usually this will be specified using the same size as the canvas image.  When the vector data is saved to SVG or MVG formats, the viewbox is use to specify the size of the canvas image that a viewer will render the vector data on.</p>

<p>The format of the DrawSetViewbox method is:</p>

<pre class="text">
void DrawSetViewbox(DrawingWand *wand,const double x1,const double y1,
  const double x2,const double y2)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>x1</dt>
<dd>left x ordinate </dd>

<dd> </dd>
<dt>y1</dt>
<dd>top y ordinate </dd>

<dd> </dd>
<dt>x2</dt>
<dd>right x ordinate </dd>

<dd> </dd>
<dt>y2</dt>
<dd>bottom y ordinate </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="IsDrawingWand">IsDrawingWand</a></h2>

<p>IsDrawingWand() returns MagickTrue if the wand is verified as a drawing wand.</p>

<p>The format of the IsDrawingWand method is:</p>

<pre class="text">
MagickBooleanType IsDrawingWand(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="NewDrawingWand">NewDrawingWand</a></h2>

<p>NewDrawingWand() returns a drawing wand required for all other methods in the API.</p>

<p>The format of the NewDrawingWand method is:</p>

<pre class="text">
DrawingWand *NewDrawingWand(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="PeekDrawingWand">PeekDrawingWand</a></h2>

<p>PeekDrawingWand() returns the current drawing wand.</p>

<p>The format of the PeekDrawingWand method is:</p>

<pre class="text">
DrawInfo *PeekDrawingWand(const DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="PopDrawingWand">PopDrawingWand</a></h2>

<p>PopDrawingWand() destroys the current drawing wand and returns to the previously pushed drawing wand. Multiple drawing wands may exist. It is an error to attempt to pop more drawing wands than have been pushed, and it is proper form to pop all drawing wands which have been pushed.</p>

<p>The format of the PopDrawingWand method is:</p>

<pre class="text">
MagickBooleanType PopDrawingWand(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/drawing-wand_8c.html" id="PushDrawingWand">PushDrawingWand</a></h2>

<p>PushDrawingWand() clones the current drawing wand to create a new drawing wand.  The original drawing wand(s) may be returned to by invoking PopDrawingWand().  The drawing wands are stored on a drawing wand stack. For every Pop there must have already been an equivalent Push.</p>

<p>The format of the PushDrawingWand method is:</p>

<pre class="text">
MagickBooleanType PushDrawingWand(DrawingWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the drawing wand. </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="drawing-wand.php#">Back to top</a> •
    <a href="http://pgp.mit.edu:11371/pks/lookup?op=get&amp;search=0x89AB63D48277377A">Public Key</a> •
    <a href="../script/contact.php">Contact Us</a></p>
        <p><small>©  1999-2016 ImageMagick Studio LLC</small></p>
  </footer>
</div><!-- /.container -->

  <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>
  <script src="http://nextgen.imagemagick.org/js/magick.php"></script>
</div>
</body>
</html>





<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Draw on an Image</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, draw, on, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="draw.php#AcquireDrawInfo">AcquireDrawInfo</a> &bull; <a href="draw.php#CloneDrawInfo">CloneDrawInfo</a> &bull; <a href="draw.php#DestroyDrawInfo">DestroyDrawInfo</a> &bull; <a href="draw.php#DrawAffineImage">DrawAffineImage</a> &bull; <a href="draw.php#DrawClipPath">DrawClipPath</a> &bull; <a href="draw.php#DrawImage">DrawImage</a> &bull; <a href="draw.php#DrawGradientImage">DrawGradientImage</a> &bull; <a href="draw.php#DrawPatternPath">DrawPatternPath</a> &bull; <a href="draw.php#DrawPrimitive">DrawPrimitive</a> &bull; <a href="draw.php#GetAffineMatrix">GetAffineMatrix</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/draw_8c.html" id="AcquireDrawInfo">AcquireDrawInfo</a></h2>

<p>AcquireDrawInfo() returns a DrawInfo structure properly initialized.</p>

<p>The format of the AcquireDrawInfo method is:</p>

<pre class="text">
DrawInfo *AcquireDrawInfo(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/draw_8c.html" id="CloneDrawInfo">CloneDrawInfo</a></h2>

<p>CloneDrawInfo() makes a copy of the given draw_info structure.  If NULL is specified, a new DrawInfo structure is created initialized to default values.</p>

<p>The format of the CloneDrawInfo method is:</p>

<pre class="text">
DrawInfo *CloneDrawInfo(const ImageInfo *image_info,
  const DrawInfo *draw_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd> </dd>
<dt>draw_info</dt>
<dd>the draw info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/draw_8c.html" id="DestroyDrawInfo">DestroyDrawInfo</a></h2>

<p>DestroyDrawInfo() deallocates memory associated with an DrawInfo structure.</p>

<p>The format of the DestroyDrawInfo method is:</p>

<pre class="text">
DrawInfo *DestroyDrawInfo(DrawInfo *draw_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>draw_info</dt>
<dd>the draw info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/draw_8c.html" id="DrawAffineImage">DrawAffineImage</a></h2>

<p>DrawAffineImage() composites the source over the destination image as dictated by the affine transform.</p>

<p>The format of the DrawAffineImage method is:</p>

<pre class="text">
MagickBooleanType DrawAffineImage(Image *image,const Image *source,
  const AffineMatrix *affine,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>source</dt>
<dd>the source image. </dd>

<dd> </dd>
<dt>affine</dt>
<dd>the affine transform. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/draw_8c.html" id="DrawClipPath">DrawClipPath</a></h2>

<p>DrawClipPath() draws the clip path on the image mask.</p>

<p>The format of the DrawClipPath method is:</p>

<pre class="text">
MagickBooleanType DrawClipPath(Image *image,const DrawInfo *draw_info,
  const char *name,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>draw_info</dt>
<dd>the draw info. </dd>

<dd> </dd>
<dt>name</dt>
<dd>the name of the clip path. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/draw_8c.html" id="DrawImage">DrawImage</a></h2>

<p>DrawImage() draws a graphic primitive on your image.  The primitive may be represented as a string or filename.  Precede the filename with an "at" sign (@) and the contents of the file are drawn on the image.  You can affect how text is drawn by setting one or more members of the draw info structure.</p>

<p>The format of the DrawImage method is:</p>

<pre class="text">
MagickBooleanType DrawImage(Image *image,const DrawInfo *draw_info,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>draw_info</dt>
<dd>the draw info. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/draw_8c.html" id="DrawGradientImage">DrawGradientImage</a></h2>

<p>DrawGradientImage() draws a linear gradient on the image.</p>

<p>The format of the DrawGradientImage method is:</p>

<pre class="text">
MagickBooleanType DrawGradientImage(Image *image,
  const DrawInfo *draw_info,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>draw_info</dt>
<dd>the draw info. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/draw_8c.html" id="DrawPatternPath">DrawPatternPath</a></h2>

<p>DrawPatternPath() draws a pattern.</p>

<p>The format of the DrawPatternPath method is:</p>

<pre class="text">
MagickBooleanType DrawPatternPath(Image *image,const DrawInfo *draw_info,
  const char *name,Image **pattern,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>draw_info</dt>
<dd>the draw info. </dd>

<dd> </dd>
<dt>name</dt>
<dd>the pattern name. </dd>

<dd> </dd>
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/draw_8c.html" id="DrawPrimitive">DrawPrimitive</a></h2>

<p>DrawPrimitive() draws a primitive (line, rectangle, ellipse) on the image.</p>

<p>The format of the DrawPrimitive method is:</p>

<pre class="text">
MagickBooleanType DrawPrimitive(Image *image,const DrawInfo *draw_info,
  PrimitiveInfo *primitive_info,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>draw_info</dt>
<dd>the draw info. </dd>

<dd> </dd>
<dt>primitive_info</dt>
<dd>Specifies a pointer to a PrimitiveInfo structure. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/draw_8c.html" id="GetAffineMatrix">GetAffineMatrix</a></h2>

<p>GetAffineMatrix() returns an AffineMatrix initialized to the identity matrix.</p>

<p>The format of the GetAffineMatrix method is:</p>

<pre class="text">
void GetAffineMatrix(AffineMatrix *affine_matrix)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>affine_matrix</dt>
<dd>the affine matrix. </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="draw.php#">Back to top</a> •
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

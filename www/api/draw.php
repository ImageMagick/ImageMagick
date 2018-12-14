



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Draw on an Image @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, draw, on, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
  <meta name="rating" content="GENERAL">
  <meta name="robots" content="INDEX, FOLLOW">
  <meta name="generator" content="ImageMagick Studio LLC">
  <meta name="author" content="ImageMagick Studio LLC">
  <meta name="revisit-after" content="2 DAYS">
  <meta name="resource-type" content="document">
  <meta name="copyright" content="Copyright (c) 1999-2017 ImageMagick Studio LLC">
  <meta name="distribution" content="Global">
  <meta name="magick-serial" content="P131-S030410-R485315270133-P82224-A6668-G1245-1">
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4">
  <link href="https://imagemagick.org/api/draw.php" rel="canonical">
  <link href="https://imagemagick.org/image/wand.png" rel="icon">
  <link href="https://imagemagick.org/image/wand.ico" rel="shortcut icon">
  <link href="https://imagemagick.org/assets/magick-css.php" rel="stylesheet">
</head>
<body>
  <header>
  <nav class="navbar navbar-expand-md navbar-dark bg-dark fixed-top">
    <a class="navbar-brand" href="https://imagemagick.org/"><img class="d-block" id="icon" alt="ImageMagick" width="32" height="32" src="https://imagemagick.org/image/wand.ico"/></a>
    <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarsExampleDefault" aria-controls="navbarsExampleDefault" aria-expanded="false" aria-label="Toggle navigation">
      <span class="navbar-toggler-icon"></span>
    </button>

    <div class="navbar-collapse collapse" id="navbarsExampleDefault" style="">
    <ul class="navbar-nav mr-auto">
      <li class="nav-item ">
        <a class="nav-link" href="https://imagemagick.org/index.php">Home <span class="sr-only">(current)</span></a>
      </li>
      <li class="nav-item ">
        <a class="nav-link" href="https://imagemagick.org/script/download.php">Download</a>
      </li>
      <li class="nav-item ">
        <a class="nav-link" href="https://imagemagick.org/script/command-line-tools.php">Tools</a>
      </li>
      <li class="nav-item ">
        <a class="nav-link" href="https://imagemagick.org/script/command-line-processing.php">Command-line</a>
      </li>
      <li class="nav-item ">
        <a class="nav-link" href="https://imagemagick.org/script/resources.php">Resources</a>
      </li>
      <li class="nav-item ">
        <a class="nav-link" href="https://imagemagick.org/script/develop.php">Develop</a>
      </li>
      <li class="nav-item">
        <a class="nav-link" target="_blank" href="https://imagemagick.org/discourse-server/">Community</a>
      </li>
    </ul>
    <form class="form-inline my-2 my-lg-0" action="../script/search.php">
      <input class="form-control mr-sm-2" type="text" name="q" placeholder="Search" aria-label="Search">
      <button class="btn btn-outline-success my-2 my-sm-0" type="submit" name="sa">Search</button>
    </form>
    </div>
  </nav>
  <div class="container">
   <script async="async" src="https://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js"></script>    <ins class="adsbygoogle"
         style="display:block"
         data-ad-client="ca-pub-3129977114552745"
         data-ad-slot="6345125851"
         data-ad-format="auto"></ins>
    <script>
      (adsbygoogle = window.adsbygoogle || []).push({});
    </script>

  </div>
  </header>
  <main class="container">
    <div class="magick-template">
<div class="magick-header">
<p class="text-center"><a href="draw.php#AcquireDrawInfo">AcquireDrawInfo</a> &bull; <a href="draw.php#CloneDrawInfo">CloneDrawInfo</a> &bull; <a href="draw.php#DestroyDrawInfo">DestroyDrawInfo</a> &bull; <a href="draw.php#DrawAffineImage">DrawAffineImage</a> &bull; <a href="draw.php#DrawClipPath">DrawClipPath</a> &bull; <a href="draw.php#DrawClippingMask">DrawClippingMask</a> &bull; <a href="draw.php#DrawCompositeMask">DrawCompositeMask</a> &bull; <a href="draw.php#DrawGradientImage">DrawGradientImage</a> &bull; <a href="draw.php#DrawImage">DrawImage</a> &bull; <a href="draw.php#DrawPatternPath">DrawPatternPath</a> &bull; <a href="draw.php#DrawPrimitive">DrawPrimitive</a> &bull; <a href="draw.php#GetAffineMatrix">GetAffineMatrix</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="AcquireDrawInfo">AcquireDrawInfo</a></h2>

<p>AcquireDrawInfo() returns a DrawInfo structure properly initialized.</p>

<p>The format of the AcquireDrawInfo method is:</p>

<pre class="text">
DrawInfo *AcquireDrawInfo(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="CloneDrawInfo">CloneDrawInfo</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="DestroyDrawInfo">DestroyDrawInfo</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="DrawAffineImage">DrawAffineImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="DrawClipPath">DrawClipPath</a></h2>

<p>DrawClipPath() draws the clip path on the image mask.</p>

<p>The format of the DrawClipPath method is:</p>

<pre class="text">
MagickBooleanType DrawClipPath(Image *image,const DrawInfo *draw_info,
  const char *id,ExceptionInfo *exception)
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
<dt>id</dt>
<dd>the clip path id. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="DrawClippingMask">DrawClippingMask</a></h2>

<p>DrawClippingMask() draws the clip path and returns it as an image clipping mask.</p>

<p>The format of the DrawClippingMask method is:</p>

<pre class="text">
Image *DrawClippingMask(Image *image,const DrawInfo *draw_info,
  const char *id,const char *clip_path,ExceptionInfo *exception)
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
<dt>id</dt>
<dd>the clip path id. </dd>

<dd> </dd>
<dt>clip_path</dt>
<dd>the clip path. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="DrawCompositeMask">DrawCompositeMask</a></h2>

<p>DrawCompositeMask() draws the mask path and returns it as an image mask.</p>

<p>The format of the DrawCompositeMask method is:</p>

<pre class="text">
Image *DrawCompositeMask(Image *image,const DrawInfo *draw_info,
  const char *id,const char *mask_path,ExceptionInfo *exception)
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
<dt>id</dt>
<dd>the mask path id. </dd>

<dd> </dd>
<dt>mask_path</dt>
<dd>the mask path. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="DrawGradientImage">DrawGradientImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="DrawImage">DrawImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="DrawPatternPath">DrawPatternPath</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="DrawPrimitive">DrawPrimitive</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/draw_8c.html" id="GetAffineMatrix">GetAffineMatrix</a></h2>

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
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="draw.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
    &nbsp; &nbsp;
    <a href="http://pgp.mit.edu/pks/lookup?op=get&amp;search=0x89AB63D48277377A">Public Key</a> •
    <a href="https://imagemagick.org/script/support.php">Donate</a> •
    <a href="https://imagemagick.org/script/contact.php">Contact Us</a>
    <br/>
        <small>© 1999-2019 ImageMagick Studio LLC</small></p>
  </footer>

  <!-- Javascript assets -->
  <script src="https://imagemagick.org/assets/magick-js.php" crossorigin="anonymous"></script>
  <script>window.jQuery || document.write('<script src="https://imagemagick.org/assets/jquery.min.js"><\/script>')</script>
</body>
</html>
<!-- Magick Cache 8th September 2018 08:04 -->
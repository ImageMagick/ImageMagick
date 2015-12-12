



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Paint on an Image</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, paint, on, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="paint.php#FloodfillPaintImage">FloodfillPaintImage</a> &bull; <a href="paint.php#OilPaintImage">OilPaintImage</a> &bull; <a href="paint.php#OpaquePaintImage">OpaquePaintImage</a> &bull; <a href="paint.php#TransparentPaintImage">TransparentPaintImage</a> &bull; <a href="paint.php#TransparentPaintImageChroma">TransparentPaintImageChroma</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/paint_8c.html" id="FloodfillPaintImage">FloodfillPaintImage</a></h2>

<p>FloodfillPaintImage() changes the color value of any pixel that matches target and is an immediate neighbor.  If the method FillToBorderMethod is specified, the color value is changed for any neighbor pixel that does not match the bordercolor member of image.</p>

<p>By default target must match a particular pixel color exactly.  However, in many cases two colors may differ by a small amount.  The fuzz member of image defines how much tolerance is acceptable to consider two colors as the same.  For example, set fuzz to 10 and the color red at intensities of 100 and 102 respectively are now interpreted as the same color for the purposes of the floodfill.</p>

<p>The format of the FloodfillPaintImage method is:</p>

<pre class="text">
MagickBooleanType FloodfillPaintImage(Image *image,
  const DrawInfo *draw_info,const PixelInfo target,
  const ssize_t x_offset,const ssize_t y_offset,
  const MagickBooleanType invert,ExceptionInfo *exception)
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
<dt>target</dt>
<dd>the RGB value of the target color. </dd>

<dd> </dd>
<dt>x_offset,y_offset</dt>
<dd>the starting location of the operation. </dd>

<dd> </dd>
<dt>invert</dt>
<dd>paint any pixel that does not match the target color. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/paint_8c.html" id="OilPaintImage">OilPaintImage</a></h2>

<p>OilPaintImage() applies a special effect filter that simulates an oil painting.  Each pixel is replaced by the most frequent color occurring in a circular region defined by radius.</p>

<p>The format of the OilPaintImage method is:</p>

<pre class="text">
Image *OilPaintImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the circular neighborhood. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/paint_8c.html" id="OpaquePaintImage">OpaquePaintImage</a></h2>

<p>OpaquePaintImage() changes any pixel that matches color with the color defined by fill argument.</p>

<p>By default color must match a particular pixel color exactly.  However, in many cases two colors may differ by a small amount.  Fuzz defines how much tolerance is acceptable to consider two colors as the same.  For example, set fuzz to 10 and the color red at intensities of 100 and 102 respectively are now interpreted as the same color.</p>

<p>The format of the OpaquePaintImage method is:</p>

<pre class="text">
MagickBooleanType OpaquePaintImage(Image *image,const PixelInfo *target,
  const PixelInfo *fill,const MagickBooleanType invert,
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
<dt>target</dt>
<dd>the RGB value of the target color. </dd>

<dd> </dd>
<dt>fill</dt>
<dd>the replacement color. </dd>

<dd> </dd>
<dt>invert</dt>
<dd>paint any pixel that does not match the target color. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/paint_8c.html" id="TransparentPaintImage">TransparentPaintImage</a></h2>

<p>TransparentPaintImage() changes the opacity value associated with any pixel that matches color to the value defined by opacity.</p>

<p>By default color must match a particular pixel color exactly.  However, in many cases two colors may differ by a small amount.  Fuzz defines how much tolerance is acceptable to consider two colors as the same.  For example, set fuzz to 10 and the color red at intensities of 100 and 102 respectively are now interpreted as the same color.</p>

<p>The format of the TransparentPaintImage method is:</p>

<pre class="text">
MagickBooleanType TransparentPaintImage(Image *image,
  const PixelInfo *target,const Quantum opacity,
  const MagickBooleanType invert,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>target</dt>
<dd>the target color. </dd>

<dd> </dd>
<dt>opacity</dt>
<dd>the replacement opacity value. </dd>

<dd> </dd>
<dt>invert</dt>
<dd>paint any pixel that does not match the target color. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/paint_8c.html" id="TransparentPaintImageChroma">TransparentPaintImageChroma</a></h2>

<p>TransparentPaintImageChroma() changes the opacity value associated with any pixel that matches color to the value defined by opacity.</p>

<p>As there is one fuzz value for the all the channels, TransparentPaintImage() is not suitable for the operations like chroma, where the tolerance for similarity of two color component (RGB) can be different. Thus we define this method to take two target pixels (one low and one high) and all the pixels of an image which are lying between these two pixels are made transparent.</p>

<p>The format of the TransparentPaintImageChroma method is:</p>

<pre class="text">
MagickBooleanType TransparentPaintImageChroma(Image *image,
  const PixelInfo *low,const PixelInfo *high,const Quantum opacity,
  const MagickBooleanType invert,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>low</dt>
<dd>the low target color. </dd>

<dd> </dd>
<dt>high</dt>
<dd>the high target color. </dd>

<dd> </dd>
<dt>opacity</dt>
<dd>the replacement opacity value. </dd>

<dd> </dd>
<dt>invert</dt>
<dd>paint any pixel that does not match the target color. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="paint.php#">Back to top</a> •
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

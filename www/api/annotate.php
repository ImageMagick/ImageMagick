



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Annotate an Image</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, annotate, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="annotate.php#AnnotateImage">AnnotateImage</a> &bull; <a href="annotate.php#FormatMagickCaption">FormatMagickCaption</a> &bull; <a href="annotate.php#GetMultilineTypeMetrics">GetMultilineTypeMetrics</a> &bull; <a href="annotate.php#GetTypeMetrics">GetTypeMetrics</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/annotate_8c.html" id="AnnotateImage">AnnotateImage</a></h2>

<p>AnnotateImage() annotates an image with text.  Optionally you can include any of the following bits of information about the image by embedding the appropriate special characters:</p>

<pre class="text">
    \n   newline
    \r   carriage return
    &lt;    less-than character.
    &gt;    greater-than character.
    &amp;    ampersand character.
 a percent sign
    b   file size of image read in
    c   comment meta-data property
    d   directory component of path
    e   filename extension or suffix
    f   filename (including suffix)
    g   layer canvas page geometry   (equivalent to "WxHXY")
    h   current image height in pixels
    i   image filename (note: becomes output filename for "info:")
    k   CALCULATED: number of unique colors
    l   label meta-data property
    m   image file format (file magic)
    n   number of images in current image sequence
    o   output filename  (used for delegates)
    p   index of image in current image list
    q   quantum depth (compile-time constant)
    r   image class and colorspace
    s   scene number (from input unless re-assigned)
    t   filename without directory or extension (suffix)
    u   unique temporary filename (used for delegates)
    w   current width in pixels
    x   x resolution (density)
    y   y resolution (density)
    z   image depth (as read in unless modified, image save depth)
    A   image transparency channel enabled (true/false)
    C   image compression type
    D   image GIF dispose method
    G   original image size (wxh; before any resizes)
    H   page (canvas) height
    M   Magick filename (original file exactly as given,  including read mods)
    O   page (canvas) offset ( = XY )
    P   page (canvas) size ( = WxH )
    Q   image compression quality ( 0 = default )
    S   ?? scenes ??
    T   image time delay (in centi-seconds)
    U   image resolution units
    W   page (canvas) width
    X   page (canvas) x offset (including sign)
    Y   page (canvas) y offset (including sign)
    Z   unique filename (used for delegates)
    @   CALCULATED: trim bounding box (without actually trimming)
    #   CALCULATED: 'signature' hash of image values
</pre>

<p>The format of the AnnotateImage method is:</p>

<pre class="text">
MagickBooleanType AnnotateImage(Image *image,DrawInfo *draw_info,
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/annotate_8c.html" id="FormatMagickCaption">FormatMagickCaption</a></h2>

<p>FormatMagickCaption() formats a caption so that it fits within the image width.  It returns the number of lines in the formatted caption.</p>

<p>The format of the FormatMagickCaption method is:</p>

<pre class="text">
ssize_t FormatMagickCaption(Image *image,DrawInfo *draw_info,
  const MagickBooleanType split,TypeMetric *metrics,char **caption,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>The image.</p>

<dt>draw_info</dt>
<p>the draw info.</p>

<dt>split</dt>
<p>when no convenient line breaks-- insert newline.</p>

<dt>metrics</dt>
<p>Return the font metrics in this structure.</p>

<dt>caption</dt>
<p>the caption.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/annotate_8c.html" id="GetMultilineTypeMetrics">GetMultilineTypeMetrics</a></h2>

<p>GetMultilineTypeMetrics() returns the following information for the specified font and text:</p>

<pre class="text">
    character width
    character height
    ascender
    descender
    text width
    text height
    maximum horizontal advance
    bounds: x1
    bounds: y1
    bounds: x2
    bounds: y2
    origin: x
    origin: y
    underline position
    underline thickness
</pre>

<p>This method is like GetTypeMetrics() but it returns the maximum text width and height for multiple lines of text.</p>

<p>The format of the GetMultilineTypeMetrics method is:</p>

<pre class="text">
MagickBooleanType GetMultilineTypeMetrics(Image *image,
  const DrawInfo *draw_info,TypeMetric *metrics,ExceptionInfo *exception)
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
<dt>metrics</dt>
<dd>Return the font metrics in this structure. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/annotate_8c.html" id="GetTypeMetrics">GetTypeMetrics</a></h2>

<p>GetTypeMetrics() returns the following information for the specified font and text:</p>

<pre class="text">
    character width
    character height
    ascender
    descender
    text width
    text height
    maximum horizontal advance
    bounds: x1
    bounds: y1
    bounds: x2
    bounds: y2
    origin: x
    origin: y
    underline position
    underline thickness
</pre>

<p>The format of the GetTypeMetrics method is:</p>

<pre class="text">
MagickBooleanType GetTypeMetrics(Image *image,const DrawInfo *draw_info,
  TypeMetric *metrics,ExceptionInfo *exception)
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
<dt>metrics</dt>
<dd>Return the font metrics in this structure. </dd>

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
    <p><a href="annotate.php#">Back to top</a> •
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

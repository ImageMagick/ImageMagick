



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Image Histograms</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, image, histograms, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="histogram.php#GetImageHistogram">GetImageHistogram</a> &bull; <a href="histogram.php#IdentifyPaletteImage">IdentifyPaletteImage</a> &bull; <a href="histogram.php#IsHistogramImage">IsHistogramImage</a> &bull; <a href="histogram.php#IsPaletteImage">IsPaletteImage</a> &bull; <a href="histogram.php#MinMaxStretchImage">MinMaxStretchImage</a> &bull; <a href="histogram.php#GetNumberColors">GetNumberColors</a> &bull; <a href="histogram.php#UniqueImageColors">UniqueImageColors</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/histogram_8c.html" id="GetImageHistogram">GetImageHistogram</a></h2>

<p>GetImageHistogram() returns the unique colors in an image.</p>

<p>The format of the GetImageHistogram method is:</p>

<pre class="text">
size_t GetImageHistogram(const Image *image,
  size_t *number_colors,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>file</dt>
<p>Write a histogram of the color distribution to this file handle.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/histogram_8c.html" id="IdentifyPaletteImage">IdentifyPaletteImage</a></h2>

<p>IdentifyPaletteImage() returns MagickTrue if the image has 256 unique colors or less.</p>

<p>The format of the IdentifyPaletteImage method is:</p>

<pre class="text">
MagickBooleanType IdentifyPaletteImage(const Image *image,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/histogram_8c.html" id="IsHistogramImage">IsHistogramImage</a></h2>

<p>IsHistogramImage() returns MagickTrue if the image has 1024 unique colors or less.</p>

<p>The format of the IsHistogramImage method is:</p>

<pre class="text">
MagickBooleanType IsHistogramImage(const Image *image,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/histogram_8c.html" id="IsPaletteImage">IsPaletteImage</a></h2>

<p>IsPaletteImage() returns MagickTrue if the image is PseudoClass and has 256 unique colors or less.</p>

<p>The format of the IsPaletteImage method is:</p>

<pre class="text">
MagickBooleanType IsPaletteImage(const Image *image)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/histogram_8c.html" id="MinMaxStretchImage">MinMaxStretchImage</a></h2>

<p>MinMaxStretchImage() uses the exact minimum and maximum values found in each of the channels given, as the BlackPoint and WhitePoint to linearly stretch the colors (and histogram) of the image.  The stretch points are also moved further inward by the adjustment values given.</p>

<p>If the adjustment values are both zero this function is equivalent to a perfect normalization (or autolevel) of the image.</p>

<p>Each channel is stretched independantally of each other (producing color distortion) unless the special 'SyncChannels' flag is also provided in the channels setting. If this flag is present the minimum and maximum point will be extracted from all the given channels, and those channels will be stretched by exactly the same amount (preventing color distortion).</p>

<p>In the special case that only ONE value is found in a channel of the image that value is not stretched, that value is left as is.</p>

<p>The 'SyncChannels' is turned on in the 'DefaultChannels' setting by default.</p>

<p>The format of the MinMaxStretchImage method is:</p>

<pre class="text">
MagickBooleanType MinMaxStretchImage(Image *image,const double black,
  const double white,const double gamma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>The image to auto-level </dd>

<dd> </dd>
<dt>black, white</dt>
<dd> move the black / white point inward from the minimum and maximum points by this color value. </dd>

<dd> </dd>
<dt>gamma</dt>
<dd>the gamma. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/histogram_8c.html" id="GetNumberColors">GetNumberColors</a></h2>

<p>GetNumberColors() returns the number of unique colors in an image.</p>

<p>The format of the GetNumberColors method is:</p>

<pre class="text">
size_t GetNumberColors(const Image *image,FILE *file,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>file</dt>
<p>Write a histogram of the color distribution to this file handle.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/histogram_8c.html" id="UniqueImageColors">UniqueImageColors</a></h2>

<p>UniqueImageColors() returns the unique colors of an image.</p>

<p>The format of the UniqueImageColors method is:</p>

<pre class="text">
Image *UniqueImageColors(const Image *image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="histogram.php#">Back to top</a> •
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

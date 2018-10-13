



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Image Histograms @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, image, histograms, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/histogram.php" rel="canonical">
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
<p class="text-center"><a href="histogram.php#GetImageHistogram">GetImageHistogram</a> &bull; <a href="histogram.php#IdentifyPaletteImage">IdentifyPaletteImage</a> &bull; <a href="histogram.php#IsHistogramImage">IsHistogramImage</a> &bull; <a href="histogram.php#IsPaletteImage">IsPaletteImage</a> &bull; <a href="histogram.php#MinMaxStretchImage">MinMaxStretchImage</a> &bull; <a href="histogram.php#GetNumberColors">GetNumberColors</a> &bull; <a href="histogram.php#UniqueImageColors">UniqueImageColors</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/histogram_8c.html" id="GetImageHistogram">GetImageHistogram</a></h2>

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

<h2><a href="https://imagemagick.org/api/MagickCore/histogram_8c.html" id="IdentifyPaletteImage">IdentifyPaletteImage</a></h2>

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

<h2><a href="https://imagemagick.org/api/MagickCore/histogram_8c.html" id="IsHistogramImage">IsHistogramImage</a></h2>

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

<h2><a href="https://imagemagick.org/api/MagickCore/histogram_8c.html" id="IsPaletteImage">IsPaletteImage</a></h2>

<p>IsPaletteImage() returns MagickTrue if the image is PseudoClass and has 256 unique colors or less.</p>

<p>The format of the IsPaletteImage method is:</p>

<pre class="text">
MagickBooleanType IsPaletteImage(const Image *image)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<h2><a href="https://imagemagick.org/api/MagickCore/histogram_8c.html" id="MinMaxStretchImage">MinMaxStretchImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/histogram_8c.html" id="GetNumberColors">GetNumberColors</a></h2>

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

<h2><a href="https://imagemagick.org/api/MagickCore/histogram_8c.html" id="UniqueImageColors">UniqueImageColors</a></h2>

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
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="histogram.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
    &nbsp; &nbsp;
    <a href="http://pgp.mit.edu/pks/lookup?op=get&amp;search=0x89AB63D48277377A">Public Key</a> •
    <a href="https://imagemagick.org/script/support.php">Donate</a> •
    <a href="https://imagemagick.org/script/contact.php">Contact Us</a>
    <br/>
        <small>© 1999-2018 ImageMagick Studio LLC</small></p>
  </footer>

  <!-- Javascript assets -->
  <script src="https://imagemagick.org/assets/magick-js.php" crossorigin="anonymous"></script>
  <script>window.jQuery || document.write('<script src="https://imagemagick.org/assets/jquery.min.js"><\/script>')</script>
</body>
</html>
<!-- Magick Cache 5th September 2018 23:33 -->
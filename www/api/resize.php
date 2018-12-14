



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Resize an Image @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, resize, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/resize.php" rel="canonical">
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
<p class="text-center"><a href="resize.php#AdaptiveResizeImage">AdaptiveResizeImage</a> &bull; <a href="resize.php#InterpolativeResizeImage">InterpolativeResizeImage</a> &bull; <a href="resize.php#LiquidRescaleImage">LiquidRescaleImage</a> &bull; <a href="resize.php#MagnifyImage">MagnifyImage</a> &bull; <a href="resize.php#MinifyImage">MinifyImage</a> &bull; <a href="resize.php#ResampleImage">ResampleImage</a> &bull; <a href="resize.php#ResizeImage">ResizeImage</a> &bull; <a href="resize.php#SampleImage">SampleImage</a> &bull; <a href="resize.php#ScaleImage">ScaleImage</a> &bull; <a href="resize.php#ThumbnailImage">ThumbnailImage</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/resize_8c.html" id="AdaptiveResizeImage">AdaptiveResizeImage</a></h2>

<p>AdaptiveResizeImage() adaptively resize image with pixel resampling.</p>

<p>This is shortcut function for a fast interpolative resize using mesh interpolation.  It works well for small resizes of less than +/- 50 of the original image size.  For larger resizing on images a full filtered and slower resize function should be used instead.</p>

<p>The format of the AdaptiveResizeImage method is:</p>

<pre class="text">
Image *AdaptiveResizeImage(const Image *image,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the resized image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the resized image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/resize_8c.html" id="InterpolativeResizeImage">InterpolativeResizeImage</a></h2>

<p>InterpolativeResizeImage() resizes an image using the specified interpolation method.</p>

<p>The format of the InterpolativeResizeImage method is:</p>

<pre class="text">
Image *InterpolativeResizeImage(const Image *image,const size_t columns,
  const size_t rows,const PixelInterpolateMethod method,
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
<dt>columns</dt>
<dd>the number of columns in the resized image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the resized image. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the pixel interpolation method. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/resize_8c.html" id="LiquidRescaleImage">LiquidRescaleImage</a></h2>

<p>LiquidRescaleImage() rescales image with seam carving.</p>

<p>The format of the LiquidRescaleImage method is:</p>

<pre class="text">
Image *LiquidRescaleImage(const Image *image,const size_t columns,
  const size_t rows,const double delta_x,const double rigidity,
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
<dt>columns</dt>
<dd>the number of columns in the rescaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the rescaled image. </dd>

<dd> </dd>
<dt>delta_x</dt>
<dd>maximum seam transversal step (0 means straight seams). </dd>

<dd> </dd>
<dt>rigidity</dt>
<dd>introduce a bias for non-straight seams (typically 0). </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/resize_8c.html" id="MagnifyImage">MagnifyImage</a></h2>

<p>MagnifyImage() doubles the size of the image with a pixel art scaling algorithm.</p>

<p>The format of the MagnifyImage method is:</p>

<pre class="text">
Image *MagnifyImage(const Image *image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/resize_8c.html" id="MinifyImage">MinifyImage</a></h2>

<p>MinifyImage() is a convenience method that scales an image proportionally to half its size.</p>

<p>The format of the MinifyImage method is:</p>

<pre class="text">
Image *MinifyImage(const Image *image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/resize_8c.html" id="ResampleImage">ResampleImage</a></h2>

<p>ResampleImage() resize image in terms of its pixel size, so that when displayed at the given resolution it will be the same size in terms of real world units as the original image at the original resolution.</p>

<p>The format of the ResampleImage method is:</p>

<pre class="text">
Image *ResampleImage(Image *image,const double x_resolution,
  const double y_resolution,const FilterType filter,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image to be resized to fit the given resolution. </dd>

<dd> </dd>
<dt>x_resolution</dt>
<dd>the new image x resolution. </dd>

<dd> </dd>
<dt>y_resolution</dt>
<dd>the new image y resolution. </dd>

<dd> </dd>
<dt>filter</dt>
<dd>Image filter to use. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/resize_8c.html" id="ResizeImage">ResizeImage</a></h2>

<p>ResizeImage() scales an image to the desired dimensions, using the given filter (see AcquireFilterInfo()).</p>

<p>If an undefined filter is given the filter defaults to Mitchell for a colormapped image, a image with a matte channel, or if the image is enlarged.  Otherwise the filter defaults to a Lanczos.</p>

<p>ResizeImage() was inspired by Paul Heckbert's "zoom" program.</p>

<p>The format of the ResizeImage method is:</p>

<pre class="text">
Image *ResizeImage(Image *image,const size_t columns,const size_t rows,
  const FilterType filter,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd> </dd>
<dt>filter</dt>
<dd>Image filter to use. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/resize_8c.html" id="SampleImage">SampleImage</a></h2>

<p>SampleImage() scales an image to the desired dimensions with pixel sampling.  Unlike other scaling methods, this method does not introduce any additional color into the scaled image.</p>

<p>The format of the SampleImage method is:</p>

<pre class="text">
Image *SampleImage(const Image *image,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the sampled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the sampled image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/resize_8c.html" id="ScaleImage">ScaleImage</a></h2>

<p>ScaleImage() changes the size of an image to the given dimensions.</p>

<p>The format of the ScaleImage method is:</p>

<pre class="text">
Image *ScaleImage(const Image *image,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/resize_8c.html" id="ThumbnailImage">ThumbnailImage</a></h2>

<p>ThumbnailImage() changes the size of an image to the given dimensions and removes any associated profiles.  The goal is to produce small low cost thumbnail images suited for display on the Web.</p>

<p>The format of the ThumbnailImage method is:</p>

<pre class="text">
Image *ThumbnailImage(const Image *image,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the number of columns in the scaled image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the scaled image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

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
    <a href="resize.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 4th September 2018 11:35 -->
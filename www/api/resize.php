



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Resize an Image</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, resize, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="resize.php#AdaptiveResizeImage">AdaptiveResizeImage</a> &bull; <a href="resize.php#InterpolativeResizeImage">InterpolativeResizeImage</a> &bull; <a href="resize.php#LiquidRescaleImage">LiquidRescaleImage</a> &bull; <a href="resize.php#MagnifyImage">MagnifyImage</a> &bull; <a href="resize.php#MinifyImage">MinifyImage</a> &bull; <a href="resize.php#ResampleImage">ResampleImage</a> &bull; <a href="resize.php#ResizeImage">ResizeImage</a> &bull; <a href="resize.php#SampleImage">SampleImage</a> &bull; <a href="resize.php#ScaleImage">ScaleImage</a> &bull; <a href="resize.php#ThumbnailImage">ThumbnailImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resize_8c.html" id="AdaptiveResizeImage">AdaptiveResizeImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resize_8c.html" id="InterpolativeResizeImage">InterpolativeResizeImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resize_8c.html" id="LiquidRescaleImage">LiquidRescaleImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resize_8c.html" id="MagnifyImage">MagnifyImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resize_8c.html" id="MinifyImage">MinifyImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resize_8c.html" id="ResampleImage">ResampleImage</a></h2>

<p>ResampleImage() resize image in terms of its pixel size, so that when displayed at the given resolution it will be the same size in terms of real world units as the original image at the original resolution.</p>

<p>The format of the ResampleImage method is:</p>

<pre class="text">
Image *ResampleImage(Image *image,const double x_resolution,
  const double y_resolution,const FilterTypes filter,
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resize_8c.html" id="ResizeImage">ResizeImage</a></h2>

<p>ResizeImage() scales an image to the desired dimensions, using the given filter (see AcquireFilterInfo()).</p>

<p>If an undefined filter is given the filter defaults to Mitchell for a colormapped image, a image with a matte channel, or if the image is enlarged.  Otherwise the filter defaults to a Lanczos.</p>

<p>ResizeImage() was inspired by Paul Heckbert's "zoom" program.</p>

<p>The format of the ResizeImage method is:</p>

<pre class="text">
Image *ResizeImage(Image *image,const size_t columns,const size_t rows,
  const FilterTypes filter,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resize_8c.html" id="SampleImage">SampleImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resize_8c.html" id="ScaleImage">ScaleImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resize_8c.html" id="ThumbnailImage">ThumbnailImage</a></h2>

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
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="resize.php#">Back to top</a> •
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

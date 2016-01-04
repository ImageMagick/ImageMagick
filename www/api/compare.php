



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Compare an Image to a Reconstructed Image</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, compare, an, image, to, a, reconstructed, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="compare.php#CompareImages">CompareImages</a> &bull; <a href="compare.php#GetImageDistortion">GetImageDistortion</a> &bull; <a href="compare.php#GetImageDistortions">GetImageDistortions</a> &bull; <a href="compare.php#IsImagesEqual">IsImagesEqual</a> &bull; <a href="compare.php#SimilarityImage">SimilarityImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/compare_8c.html" id="CompareImages">CompareImages</a></h2>

<p>CompareImages() compares one or more pixel channels of an image to a reconstructed image and returns the difference image.</p>

<p>The format of the CompareImages method is:</p>

<pre class="text">
Image *CompareImages(const Image *image,const Image *reconstruct_image,
  const MetricType metric,double *distortion,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>reconstruct_image</dt>
<dd>the reconstruct image. </dd>

<dd> </dd>
<dt>metric</dt>
<dd>the metric. </dd>

<dd> </dd>
<dt>distortion</dt>
<dd>the computed distortion between the images. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/compare_8c.html" id="GetImageDistortion">GetImageDistortion</a></h2>

<p>GetImageDistortion() compares one or more pixel channels of an image to a reconstructed image and returns the specified distortion metric.</p>

<p>The format of the GetImageDistortion method is:</p>

<pre class="text">
MagickBooleanType GetImageDistortion(const Image *image,
  const Image *reconstruct_image,const MetricType metric,
  double *distortion,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>reconstruct_image</dt>
<dd>the reconstruct image. </dd>

<dd> </dd>
<dt>metric</dt>
<dd>the metric. </dd>

<dd> </dd>
<dt>distortion</dt>
<dd>the computed distortion between the images. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/compare_8c.html" id="GetImageDistortions">GetImageDistortions</a></h2>

<p>GetImageDistortions() compares the pixel channels of an image to a reconstructed image and returns the specified distortion metric for each channel.</p>

<p>The format of the GetImageDistortions method is:</p>

<pre class="text">
double *GetImageDistortions(const Image *image,
  const Image *reconstruct_image,const MetricType metric,
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
<dt>reconstruct_image</dt>
<dd>the reconstruct image. </dd>

<dd> </dd>
<dt>metric</dt>
<dd>the metric. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/compare_8c.html" id="IsImagesEqual">IsImagesEqual</a></h2>

<p>IsImagesEqual() measures the difference between colors at each pixel location of two images.  A value other than 0 means the colors match exactly.  Otherwise an error measure is computed by summing over all pixels in an image the distance squared in RGB space between each image pixel and its corresponding pixel in the reconstruct image.  The error measure is assigned to these image members:</p>

<pre class="text">
    o mean_error_per_pixel:  The mean error for any single pixel in
the image.
</pre>

<dt>normalized_mean_error</dt>
<p>The normalized mean quantization error for any single pixel in the image.  This distance measure is normalized to a range between 0 and 1.  It is independent of the range of red, green, and blue values in the image.</p>

<dt>normalized_maximum_error</dt>
<p>The normalized maximum quantization error for any single pixel in the image.  This distance measure is normalized to a range between 0 and 1.  It is independent of the range of red, green, and blue values in your image.</p>

<p>A small normalized mean square error, accessed as image-&gt;normalized_mean_error, suggests the images are very similar in spatial layout and color.</p>

<p>The format of the IsImagesEqual method is:</p>

<pre class="text">
MagickBooleanType IsImagesEqual(Image *image,
  const Image *reconstruct_image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>reconstruct_image</dt>
<p>the reconstruct image.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/compare_8c.html" id="SimilarityImage">SimilarityImage</a></h2>

<p>SimilarityImage() compares the reference image of the image and returns the best match offset.  In addition, it returns a similarity image such that an exact match location is completely white and if none of the pixels match, black, otherwise some gray level in-between.</p>

<p>The format of the SimilarityImageImage method is:</p>

<pre class="text">
Image *SimilarityImage(const Image *image,const Image *reference,
  const MetricType metric,const double similarity_threshold,
  RectangleInfo *offset,double *similarity,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>reference</dt>
<dd>find an area of the image that closely resembles this image. </dd>

<dd> </dd>
<dt>metric</dt>
<dd>the metric. </dd>

<dd> </dd>
<dt>similarity_threshold</dt>
<dd>minimum distortion for (sub)image match. </dd>

<dd> </dd>
<dt>offset</dt>
<dd>the best match offset of the reference image within the image. </dd>

<dd> </dd>
<dt>similarity</dt>
<dd>the computed similarity between the images. </dd>

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
    <p><a href="compare.php#">Back to top</a> •
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

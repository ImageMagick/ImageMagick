



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Image Features</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, image, features, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="feature.php#CannyEdgeImage">CannyEdgeImage</a> &bull; <a href="feature.php#GetImageFeatures">GetImageFeatures</a> &bull; <a href="feature.php#Use HoughLineImage">Use HoughLineImage</a> &bull; <a href="feature.php#MeanShiftImage">MeanShiftImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/feature_8c.html" id="CannyEdgeImage">CannyEdgeImage</a></h2>

<p>CannyEdgeImage() uses a multi-stage algorithm to detect a wide range of edges in images.</p>

<p>The format of the CannyEdgeImage method is:</p>

<pre class="text">
Image *CannyEdgeImage(const Image *image,const double radius,
  const double sigma,const double lower_percent,
  const double upper_percent,ExceptionInfo *exception)
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
<dd>the radius of the gaussian smoothing filter. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the sigma of the gaussian smoothing filter. </dd>

<dd> </dd>
<dt>lower_precent</dt>
<dd>percentage of edge pixels in the lower threshold. </dd>

<dd> </dd>
<dt>upper_percent</dt>
<dd>percentage of edge pixels in the upper threshold. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/feature_8c.html" id="GetImageFeatures">GetImageFeatures</a></h2>

<p>GetImageFeatures() returns features for each channel in the image in each of four directions (horizontal, vertical, left and right diagonals) for the specified distance.  The features include the angular second moment, contrast, correlation, sum of squares: variance, inverse difference moment, sum average, sum varience, sum entropy, entropy, difference variance,  difference entropy, information measures of correlation 1, information measures of correlation 2, and maximum correlation coefficient.  You can access the red channel contrast, for example, like this:</p>

<pre class="text">
channel_features=GetImageFeatures(image,1,exception);
contrast=channel_features[RedPixelChannel].contrast[0];
</pre>

<p>Use MagickRelinquishMemory() to free the features buffer.</p>

<p>The format of the GetImageFeatures method is:</p>

<pre class="text">
ChannelFeatures *GetImageFeatures(const Image *image,
  const size_t distance,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>distance</dt>
<dd>the distance. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/feature_8c.html" id="Use_HoughLineImage">Use HoughLineImage</a></h2>

<p>Use HoughLineImage() in conjunction with any binary edge extracted image (we recommand Canny) to identify lines in the image.  The algorithm accumulates counts for every white pixel for every possible orientation (for angles from 0 to 179 in 1 degree increments) and distance from the center of the image to the corner (in 1 px increments) and stores the counts in an accumulator matrix of angle vs distance. The size of the accumulator is 180x(diagonal/2). Next it searches this space for peaks in counts and converts the locations of the peaks to slope and intercept in the normal x,y input image space. Use the slope/intercepts to find the endpoints clipped to the bounds of the image. The lines are then drawn. The counts are a measure of the length of the lines</p>

<p>The format of the HoughLineImage method is:</p>

<pre class="text">
Image *HoughLineImage(const Image *image,const size_t width,
  const size_t height,const size_t threshold,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>width, height</dt>
<dd>find line pairs as local maxima in this neighborhood. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd>the line count threshold. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/feature_8c.html" id="MeanShiftImage">MeanShiftImage</a></h2>

<p>MeanShiftImage() delineate arbitrarily shaped clusters in the image. For each pixel, it visits all the pixels in the neighborhood specified by the window centered at the pixel and excludes those that are outside the radius=(window-1)/2 surrounding the pixel. From those pixels, it finds those that are within the specified color distance from the current mean, and computes a new x,y centroid from those coordinates and a new mean. This new x,y centroid is used as the center for a new window. This process iterates until it converges and the final mean is replaces the (original window center) pixel value. It repeats this process for the next pixel, etc.,  until it processes all pixels in the image. Results are typically better with colorspaces other than sRGB. We recommend YIQ, YUV or YCbCr.</p>

<p>The format of the MeanShiftImage method is:</p>

<pre class="text">
Image *MeanShiftImage(const Image *image,const size_t width,
  const size_t height,const double color_distance,
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
<dt>width, height</dt>
<dd>find pixels in this neighborhood. </dd>

<dd> </dd>
<dt>color_distance</dt>
<dd>the color distance. </dd>

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
    <p><a href="feature.php#">Back to top</a> •
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

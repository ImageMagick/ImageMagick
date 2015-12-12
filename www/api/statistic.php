



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Image Statistics</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, image, statistics, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="statistic.php#EvaluateImage">EvaluateImage</a> &bull; <a href="statistic.php#FunctionImage">FunctionImage</a> &bull; <a href="statistic.php#GetImageEntropy">GetImageEntropy</a> &bull; <a href="statistic.php#GetImageExtrema">GetImageExtrema</a> &bull; <a href="statistic.php#GetImageKurtosis">GetImageKurtosis</a> &bull; <a href="statistic.php#GetImageMean">GetImageMean</a> &bull; <a href="statistic.php#GetImageMoments">GetImageMoments</a> &bull; <a href="statistic.php#GetImagePerceptualHash">GetImagePerceptualHash</a> &bull; <a href="statistic.php#GetImageRange">GetImageRange</a> &bull; <a href="statistic.php#GetImageStatistics">GetImageStatistics</a> &bull; <a href="statistic.php#PolynomialImage">PolynomialImage</a> &bull; <a href="statistic.php#StatisticImage">StatisticImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="EvaluateImage">EvaluateImage</a></h2>

<p>EvaluateImage() applies a value to the image with an arithmetic, relational, or logical operator to an image. Use these operations to lighten or darken an image, to increase or decrease contrast in an image, or to produce the "negative" of an image.</p>

<p>The format of the EvaluateImage method is:</p>

<pre class="text">
MagickBooleanType EvaluateImage(Image *image,
  const MagickEvaluateOperator op,const double value,
  ExceptionInfo *exception)
MagickBooleanType EvaluateImages(Image *images,
  const MagickEvaluateOperator op,const double value,
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
<dt>op</dt>
<dd>A channel op. </dd>

<dd> </dd>
<dt>value</dt>
<dd>A value value. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="FunctionImage">FunctionImage</a></h2>

<p>FunctionImage() applies a value to the image with an arithmetic, relational, or logical operator to an image. Use these operations to lighten or darken an image, to increase or decrease contrast in an image, or to produce the "negative" of an image.</p>

<p>The format of the FunctionImage method is:</p>

<pre class="text">
MagickBooleanType FunctionImage(Image *image,
  const MagickFunction function,const ssize_t number_parameters,
  const double *parameters,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>function</dt>
<dd>A channel function. </dd>

<dd> </dd>
<dt>parameters</dt>
<dd>one or more parameters. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageEntropy">GetImageEntropy</a></h2>

<p>GetImageEntropy() returns the entropy of one or more image channels.</p>

<p>The format of the GetImageEntropy method is:</p>

<pre class="text">
MagickBooleanType GetImageEntropy(const Image *image,double *entropy,
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
<dt>entropy</dt>
<dd>the average entropy of the selected channels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageExtrema">GetImageExtrema</a></h2>

<p>GetImageExtrema() returns the extrema of one or more image channels.</p>

<p>The format of the GetImageExtrema method is:</p>

<pre class="text">
MagickBooleanType GetImageExtrema(const Image *image,size_t *minima,
  size_t *maxima,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>minima</dt>
<dd>the minimum value in the channel. </dd>

<dd> </dd>
<dt>maxima</dt>
<dd>the maximum value in the channel. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageKurtosis">GetImageKurtosis</a></h2>

<p>GetImageKurtosis() returns the kurtosis and skewness of one or more image channels.</p>

<p>The format of the GetImageKurtosis method is:</p>

<pre class="text">
MagickBooleanType GetImageKurtosis(const Image *image,double *kurtosis,
  double *skewness,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>kurtosis</dt>
<dd>the kurtosis of the channel. </dd>

<dd> </dd>
<dt>skewness</dt>
<dd>the skewness of the channel. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageMean">GetImageMean</a></h2>

<p>GetImageMean() returns the mean and standard deviation of one or more image channels.</p>

<p>The format of the GetImageMean method is:</p>

<pre class="text">
MagickBooleanType GetImageMean(const Image *image,double *mean,
  double *standard_deviation,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>mean</dt>
<dd>the average value in the channel. </dd>

<dd> </dd>
<dt>standard_deviation</dt>
<dd>the standard deviation of the channel. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageMoments">GetImageMoments</a></h2>

<p>GetImageMoments() returns the normalized moments of one or more image channels.</p>

<p>The format of the GetImageMoments method is:</p>

<pre class="text">
ChannelMoments *GetImageMoments(const Image *image,
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
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImagePerceptualHash">GetImagePerceptualHash</a></h2>

<p>GetImagePerceptualHash() returns the perceptual hash of one or more image channels.</p>

<p>The format of the GetImagePerceptualHash method is:</p>

<pre class="text">
ChannelPerceptualHash *GetImagePerceptualHash(const Image *image,
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
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageRange">GetImageRange</a></h2>

<p>GetImageRange() returns the range of one or more image channels.</p>

<p>The format of the GetImageRange method is:</p>

<pre class="text">
MagickBooleanType GetImageRange(const Image *image,double *minima,
  double *maxima,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>minima</dt>
<dd>the minimum value in the channel. </dd>

<dd> </dd>
<dt>maxima</dt>
<dd>the maximum value in the channel. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageStatistics">GetImageStatistics</a></h2>

<p>GetImageStatistics() returns statistics for each channel in the image.  The statistics include the channel depth, its minima, maxima, mean, standard deviation, kurtosis and skewness.  You can access the red channel mean, for example, like this:</p>

<pre class="text">
channel_statistics=GetImageStatistics(image,exception);
red_mean=channel_statistics[RedPixelChannel].mean;
</pre>

<p>Use MagickRelinquishMemory() to free the statistics buffer.</p>

<p>The format of the GetImageStatistics method is:</p>

<pre class="text">
ChannelStatistics *GetImageStatistics(const Image *image,
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
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="PolynomialImage">PolynomialImage</a></h2>

<p>PolynomialImage() returns a new image where each pixel is the sum of the pixels in the image sequence after applying its corresponding terms (coefficient and degree pairs).</p>

<p>The format of the PolynomialImage method is:</p>

<pre class="text">
Image *PolynomialImage(const Image *images,const size_t number_terms,
  const double *terms,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>images</dt>
<dd>the image sequence. </dd>

<dd> </dd>
<dt>number_terms</dt>
<dd>the number of terms in the list.  The actual list length is 2 x number_terms + 1 (the constant). </dd>

<dd> </dd>
<dt>terms</dt>
<dd>the list of polynomial coefficients and degree pairs and a constant. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/statistic_8c.html" id="StatisticImage">StatisticImage</a></h2>

<p>StatisticImage() makes each pixel the min / max / median / mode / etc. of the neighborhood of the specified width and height.</p>

<p>The format of the StatisticImage method is:</p>

<pre class="text">
Image *StatisticImage(const Image *image,const StatisticType type,
  const size_t width,const size_t height,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>type</dt>
<dd>the statistic type (median, mode, etc.). </dd>

<dd> </dd>
<dt>width</dt>
<dd>the width of the pixel neighborhood. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the height of the pixel neighborhood. </dd>

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
    <p><a href="statistic.php#">Back to top</a> •
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

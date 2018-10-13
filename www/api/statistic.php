



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Image Statistics @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, image, statistics, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/statistic.php" rel="canonical">
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
<p class="text-center"><a href="statistic.php#EvaluateImage">EvaluateImage</a> &bull; <a href="statistic.php#FunctionImage">FunctionImage</a> &bull; <a href="statistic.php#GetImageEntropy">GetImageEntropy</a> &bull; <a href="statistic.php#GetImageExtrema">GetImageExtrema</a> &bull; <a href="statistic.php#GetImageKurtosis">GetImageKurtosis</a> &bull; <a href="statistic.php#GetImageMean">GetImageMean</a> &bull; <a href="statistic.php#GetImageMoments">GetImageMoments</a> &bull; <a href="statistic.php#GetImagePerceptualHash">GetImagePerceptualHash</a> &bull; <a href="statistic.php#GetImageRange">GetImageRange</a> &bull; <a href="statistic.php#GetImageStatistics">GetImageStatistics</a> &bull; <a href="statistic.php#PolynomialImage">PolynomialImage</a> &bull; <a href="statistic.php#StatisticImage">StatisticImage</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="EvaluateImage">EvaluateImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="FunctionImage">FunctionImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageEntropy">GetImageEntropy</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageExtrema">GetImageExtrema</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageKurtosis">GetImageKurtosis</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageMean">GetImageMean</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageMoments">GetImageMoments</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImagePerceptualHash">GetImagePerceptualHash</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageRange">GetImageRange</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="GetImageStatistics">GetImageStatistics</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="PolynomialImage">PolynomialImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/statistic_8c.html" id="StatisticImage">StatisticImage</a></h2>

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
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="statistic.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 2nd September 2018 21:28 -->
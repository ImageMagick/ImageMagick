



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Read or List Image formats @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, read, or, list, image, formats, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/magick.php" rel="canonical">
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
<p class="text-center"><a href="magick.php#AcquireMagickInfo">AcquireMagickInfo</a> &bull; <a href="magick.php#GetMagickPrecision">GetMagickPrecision</a> &bull; <a href="magick.php#IsMagickCoreInstantiated">IsMagickCoreInstantiated</a> &bull; <a href="magick.php#MagickCoreGenesis">MagickCoreGenesis</a> &bull; <a href="magick.php#MagickCoreTerminus">MagickCoreTerminus</a> &bull; <a href="magick.php#SetMagickPrecision">SetMagickPrecision</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/magick_8c.html" id="AcquireMagickInfo">AcquireMagickInfo</a></h2>

<p>AcquireMagickInfo() allocates a MagickInfo structure and initializes the members to default values.</p>

<p>The format of the AcquireMagickInfo method is:</p>

<pre class="text">
MagickInfo *AcquireMagickInfo(const char *module, const char *name,)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>module</dt>
<dd>a character string that represents the module associated with the MagickInfo structure. </dd>

<dd> </dd>
<dt>name</dt>
<dd>a character string that represents the image format associated with the MagickInfo structure. </dd>

<dd> </dd>
<dt>description</dt>
<dd>a character string that represents the image format associated with the MagickInfo structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/magick_8c.html" id="GetMagickPrecision">GetMagickPrecision</a></h2>

<p>GetMagickPrecision() returns the maximum number of significant digits to be printed.</p>

<p>The format of the GetMagickPrecision method is:</p>

<pre class="text">
int GetMagickPrecision(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/magick_8c.html" id="IsMagickCoreInstantiated">IsMagickCoreInstantiated</a></h2>

<p>IsMagickCoreInstantiated() returns MagickTrue if the ImageMagick environment is currently instantiated:  MagickCoreGenesis() has been called but MagickDestroy() has not.</p>

<p>The format of the IsMagickCoreInstantiated method is:</p>

<pre class="text">
MagickBooleanType IsMagickCoreInstantiated(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/magick_8c.html" id="MagickCoreGenesis">MagickCoreGenesis</a></h2>

<p>MagickCoreGenesis() initializes the MagickCore environment.</p>

<p>The format of the MagickCoreGenesis function is:</p>

<pre class="text">
MagickCoreGenesis(const char *path,
  const MagickBooleanType establish_signal_handlers)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>path</dt>
<dd>the execution path of the current ImageMagick client. </dd>

<dd> </dd>
<dt>establish_signal_handlers</dt>
<dd>set to MagickTrue to use MagickCore's own signal handlers for common signals. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/magick_8c.html" id="MagickCoreTerminus">MagickCoreTerminus</a></h2>

<p>MagickCoreTerminus() destroys the MagickCore environment.</p>

<p>The format of the MagickCoreTerminus function is:</p>

<pre class="text">
MagickCoreTerminus(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/magick_8c.html" id="SetMagickPrecision">SetMagickPrecision</a></h2>

<p>SetMagickPrecision() sets the maximum number of significant digits to be printed.</p>

<p>An input argument of 0 returns the current precision setting.</p>

<p>A negative value forces the precision to reset to a default value according to the environment variable "MAGICK_PRECISION", the current 'policy' configuration setting, or the default value of '6', in that order.</p>

<p>The format of the SetMagickPrecision method is:</p>

<pre class="text">
int SetMagickPrecision(const int precision)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>precision</dt>
<dd>set the maximum number of significant digits to be printed. </dd>

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
    <a href="magick.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 3rd September 2018 14:40 -->




<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Get the Version and Copyrights @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, get, the, version, copyrights, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/version.php" rel="canonical">
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
<p class="text-center"><a href="version.php#GetMagickCopyright">GetMagickCopyright</a> &bull; <a href="version.php#GetMagickDelegates">GetMagickDelegates</a> &bull; <a href="version.php#GetMagickFeatures">GetMagickFeatures</a> &bull; <a href="version.php#GetMagickHomeURL">GetMagickHomeURL</a> &bull; <a href="version.php#GetMagickLicense">GetMagickLicense</a> &bull; <a href="version.php#GetMagickPackageName">GetMagickPackageName</a> &bull; <a href="version.php#GetMagickQuantumDepth">GetMagickQuantumDepth</a> &bull; <a href="version.php#GetMagickQuantumRange">GetMagickQuantumRange</a> &bull; <a href="version.php#GetMagickReleaseDate">GetMagickReleaseDate</a> &bull; <a href="version.php#GetMagickSignature">GetMagickSignature</a> &bull; <a href="version.php#GetMagickVersion">GetMagickVersion</a> &bull; <a href="version.php#ListMagickVersion">ListMagickVersion</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickCopyright">GetMagickCopyright</a></h2>

<p>GetMagickCopyright() returns the ImageMagick API copyright as a string.</p>

<p>The format of the GetMagickCopyright method is:</p>

<pre class="text">
const char *GetMagickCopyright(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickDelegates">GetMagickDelegates</a></h2>

<p>GetMagickDelegates() returns the ImageMagick delegate libraries.</p>

<p>The format of the GetMagickDelegates method is:</p>

<pre class="text">
const char *GetMagickDelegates(void)
</pre>

<p>No parameters are required.</p>

<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickFeatures">GetMagickFeatures</a></h2>

<p>GetMagickFeatures() returns the ImageMagick features.</p>

<p>The format of the GetMagickFeatures method is:</p>

<pre class="text">
const char *GetMagickFeatures(void)
</pre>

<p>No parameters are required.</p>

<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickHomeURL">GetMagickHomeURL</a></h2>

<p>GetMagickHomeURL() returns the ImageMagick home URL.</p>

<p>The format of the GetMagickHomeURL method is:</p>

<pre class="text">
char *GetMagickHomeURL(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickLicense">GetMagickLicense</a></h2>

<p>GetMagickLicense() returns the ImageMagick API license as a string.</p>

<p>The format of the GetMagickLicense method is:</p>

<pre class="text">
const char *GetMagickLicense(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickPackageName">GetMagickPackageName</a></h2>

<p>GetMagickPackageName() returns the ImageMagick package name.</p>

<p>The format of the GetMagickName method is:</p>

<pre class="text">
const char *GetMagickName(void)
</pre>

<p>No parameters are required.</p>

<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickQuantumDepth">GetMagickQuantumDepth</a></h2>

<p>GetMagickQuantumDepth() returns the ImageMagick quantum depth.</p>

<p>The format of the GetMagickQuantumDepth method is:</p>

<pre class="text">
const char *GetMagickQuantumDepth(size_t *depth)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>depth</dt>
<dd>the quantum depth is returned as a number. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickQuantumRange">GetMagickQuantumRange</a></h2>

<p>GetMagickQuantumRange() returns the ImageMagick quantum range.</p>

<p>The format of the GetMagickQuantumRange method is:</p>

<pre class="text">
const char *GetMagickQuantumRange(size_t *range)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>range</dt>
<dd>the quantum range is returned as a number. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickReleaseDate">GetMagickReleaseDate</a></h2>

<p>GetMagickReleaseDate() returns the ImageMagick release date.</p>

<p>The format of the GetMagickReleaseDate method is:</p>

<pre class="text">
const char *GetMagickReleaseDate(void)
</pre>

<p>No parameters are required.</p>

<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickSignature">GetMagickSignature</a></h2>

<p>GetMagickSignature() returns a signature that uniquely encodes the MagickCore libary version, quantum depth, HDRI status, OS word size, and endianness.</p>

<p>The format of the GetMagickSignature method is:</p>

<pre class="text">
unsigned int GetMagickSignature(const StringInfo *nonce)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>nonce</dt>
<dd>arbitrary data. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickVersion">GetMagickVersion</a></h2>

<p>GetMagickVersion() returns the ImageMagick API version as a string and as a number.</p>

<p>The format of the GetMagickVersion method is:</p>

<pre class="text">
const char *GetMagickVersion(size_t *version)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>version</dt>
<dd>the ImageMagick version is returned as a number. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/version_8c.html" id="ListMagickVersion">ListMagickVersion</a></h2>

<p>ListMagickVersion() identifies the ImageMagick version by printing its attributes to the file.  Attributes include the copyright, features, and delegates.</p>

<p>The format of the ListMagickVersion method is:</p>

<pre class="text">
void ListMagickVersion(FILE *file)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>file</dt>
<dd>the file, typically stdout. </dd>

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
    <a href="version.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 9th September 2018 00:43 -->
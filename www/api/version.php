



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Get the Version and Copyrights</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, get, the, version, copyrights, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="version.php#GetMagickCopyright">GetMagickCopyright</a> &bull; <a href="version.php#GetMagickDelegates">GetMagickDelegates</a> &bull; <a href="version.php#GetMagickFeatures">GetMagickFeatures</a> &bull; <a href="version.php#GetMagickHomeURL">GetMagickHomeURL</a> &bull; <a href="version.php#GetMagickLicense">GetMagickLicense</a> &bull; <a href="version.php#GetMagickPackageName">GetMagickPackageName</a> &bull; <a href="version.php#GetMagickQuantumDepth">GetMagickQuantumDepth</a> &bull; <a href="version.php#GetMagickQuantumRange">GetMagickQuantumRange</a> &bull; <a href="version.php#GetMagickReleaseDate">GetMagickReleaseDate</a> &bull; <a href="version.php#GetMagickSignature">GetMagickSignature</a> &bull; <a href="version.php#GetMagickVersion">GetMagickVersion</a> &bull; <a href="version.php#ListMagickVersion">ListMagickVersion</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickCopyright">GetMagickCopyright</a></h2>

<p>GetMagickCopyright() returns the ImageMagick API copyright as a string.</p>

<p>The format of the GetMagickCopyright method is:</p>

<pre class="text">
const char *GetMagickCopyright(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickDelegates">GetMagickDelegates</a></h2>

<p>GetMagickDelegates() returns the ImageMagick delegate libraries.</p>

<p>The format of the GetMagickDelegates method is:</p>

<pre class="text">
const char *GetMagickDelegates(void)
</pre>

<p>No parameters are required.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickFeatures">GetMagickFeatures</a></h2>

<p>GetMagickFeatures() returns the ImageMagick features.</p>

<p>The format of the GetMagickFeatures method is:</p>

<pre class="text">
const char *GetMagickFeatures(void)
</pre>

<p>No parameters are required.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickHomeURL">GetMagickHomeURL</a></h2>

<p>GetMagickHomeURL() returns the ImageMagick home URL.</p>

<p>The format of the GetMagickHomeURL method is:</p>

<pre class="text">
char *GetMagickHomeURL(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickLicense">GetMagickLicense</a></h2>

<p>GetMagickLicense() returns the ImageMagick API license as a string.</p>

<p>The format of the GetMagickLicense method is:</p>

<pre class="text">
const char *GetMagickLicense(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickPackageName">GetMagickPackageName</a></h2>

<p>GetMagickPackageName() returns the ImageMagick package name.</p>

<p>The format of the GetMagickName method is:</p>

<pre class="text">
const char *GetMagickName(void)
</pre>

<p>No parameters are required.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickQuantumDepth">GetMagickQuantumDepth</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickQuantumRange">GetMagickQuantumRange</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickReleaseDate">GetMagickReleaseDate</a></h2>

<p>GetMagickReleaseDate() returns the ImageMagick release date.</p>

<p>The format of the GetMagickReleaseDate method is:</p>

<pre class="text">
const char *GetMagickReleaseDate(void)
</pre>

<p>No parameters are required.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickSignature">GetMagickSignature</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="GetMagickVersion">GetMagickVersion</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/version_8c.html" id="ListMagickVersion">ListMagickVersion</a></h2>

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
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="version.php#">Back to top</a> •
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

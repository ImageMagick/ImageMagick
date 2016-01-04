



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: The Image Registry</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, the, image, registry, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="registry.php#DefineImageRegistry">DefineImageRegistry</a> &bull; <a href="registry.php#DeleteImageRegistry">DeleteImageRegistry</a> &bull; <a href="registry.php#GetImageRegistry">GetImageRegistry</a> &bull; <a href="registry.php#GetNextImageRegistry">GetNextImageRegistry</a> &bull; <a href="registry.php#RegistryComponentTerminus">RegistryComponentTerminus</a> &bull; <a href="registry.php#RemoveImageRegistry">RemoveImageRegistry</a> &bull; <a href="registry.php#ResetImageRegistryIterator">ResetImageRegistryIterator</a> &bull; <a href="registry.php#SetImageRegistry">SetImageRegistry</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/registry_8c.html" id="DefineImageRegistry">DefineImageRegistry</a></h2>

<p>DefineImageRegistry() associates a key/value pair with the image registry.</p>

<p>The format of the DefineImageRegistry method is:</p>

<pre class="text">
MagickBooleanType DefineImageRegistry(const RegistryType type,
  const char *option,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>type</dt>
<dd>the type. </dd>

<dd> </dd>
<dt>option</dt>
<dd>the option. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>the exception. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/registry_8c.html" id="DeleteImageRegistry">DeleteImageRegistry</a></h2>

<p>DeleteImageRegistry() deletes a key from the image registry.</p>

<p>The format of the DeleteImageRegistry method is:</p>

<pre class="text">
MagickBooleanType DeleteImageRegistry(const char *key)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>key</dt>
<dd>the registry. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/registry_8c.html" id="GetImageRegistry">GetImageRegistry</a></h2>

<p>GetImageRegistry() returns a value associated with an image registry key.</p>

<p>The format of the GetImageRegistry method is:</p>

<pre class="text">
void *GetImageRegistry(const RegistryType type,const char *key,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>type</dt>
<dd>the type. </dd>

<dd> </dd>
<dt>key</dt>
<dd>the key. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>the exception. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/registry_8c.html" id="GetNextImageRegistry">GetNextImageRegistry</a></h2>

<p>GetNextImageRegistry() gets the next image registry value.</p>

<p>The format of the GetNextImageRegistry method is:</p>

<pre class="text">
char *GetNextImageRegistry(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/registry_8c.html" id="RegistryComponentTerminus">RegistryComponentTerminus</a></h2>

<p>RegistryComponentTerminus() destroys the registry component.</p>

<p>The format of the DestroyDefines method is:</p>

<pre class="text">
void RegistryComponentTerminus(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/registry_8c.html" id="RemoveImageRegistry">RemoveImageRegistry</a></h2>

<p>RemoveImageRegistry() removes a key from the image registry and returns its value.</p>

<p>The format of the RemoveImageRegistry method is:</p>

<pre class="text">
void *RemoveImageRegistry(const char *key)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>key</dt>
<dd>the registry. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/registry_8c.html" id="ResetImageRegistryIterator">ResetImageRegistryIterator</a></h2>

<p>ResetImageRegistryIterator() resets the registry iterator.  Use it in conjunction with GetNextImageRegistry() to iterate over all the values in the image registry.</p>

<p>The format of the ResetImageRegistryIterator method is:</p>

<pre class="text">
ResetImageRegistryIterator(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/registry_8c.html" id="SetImageRegistry">SetImageRegistry</a></h2>

<p>SetImageRegistry() associates a value with an image registry key.</p>

<p>The format of the SetImageRegistry method is:</p>

<pre class="text">
MagickBooleanType SetImageRegistry(const RegistryType type,
  const char *key,const void *value,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>type</dt>
<dd>the type. </dd>

<dd> </dd>
<dt>key</dt>
<dd>the key. </dd>

<dd> </dd>
<dt>value</dt>
<dd>the value. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>the exception. </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="registry.php#">Back to top</a> •
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

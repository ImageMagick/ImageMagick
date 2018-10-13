



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: The Image Registry @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, the, image, registry, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/registry.php" rel="canonical">
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
<p class="text-center"><a href="registry.php#DefineImageRegistry">DefineImageRegistry</a> &bull; <a href="registry.php#DeleteImageRegistry">DeleteImageRegistry</a> &bull; <a href="registry.php#GetImageRegistry">GetImageRegistry</a> &bull; <a href="registry.php#GetNextImageRegistry">GetNextImageRegistry</a> &bull; <a href="registry.php#RegistryComponentTerminus">RegistryComponentTerminus</a> &bull; <a href="registry.php#RemoveImageRegistry">RemoveImageRegistry</a> &bull; <a href="registry.php#ResetImageRegistryIterator">ResetImageRegistryIterator</a> &bull; <a href="registry.php#SetImageRegistry">SetImageRegistry</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/registry_8c.html" id="DefineImageRegistry">DefineImageRegistry</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/registry_8c.html" id="DeleteImageRegistry">DeleteImageRegistry</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/registry_8c.html" id="GetImageRegistry">GetImageRegistry</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/registry_8c.html" id="GetNextImageRegistry">GetNextImageRegistry</a></h2>

<p>GetNextImageRegistry() gets the next image registry value.</p>

<p>The format of the GetNextImageRegistry method is:</p>

<pre class="text">
char *GetNextImageRegistry(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/registry_8c.html" id="RegistryComponentTerminus">RegistryComponentTerminus</a></h2>

<p>RegistryComponentTerminus() destroys the registry component.</p>

<p>The format of the DestroyDefines method is:</p>

<pre class="text">
void RegistryComponentTerminus(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/registry_8c.html" id="RemoveImageRegistry">RemoveImageRegistry</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/registry_8c.html" id="ResetImageRegistryIterator">ResetImageRegistryIterator</a></h2>

<p>ResetImageRegistryIterator() resets the registry iterator.  Use it in conjunction with GetNextImageRegistry() to iterate over all the values in the image registry.</p>

<p>The format of the ResetImageRegistryIterator method is:</p>

<pre class="text">
ResetImageRegistryIterator(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/registry_8c.html" id="SetImageRegistry">SetImageRegistry</a></h2>

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
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="registry.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 5th September 2018 23:34 -->
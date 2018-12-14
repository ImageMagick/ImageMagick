



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickWand, C API: Pixel Iterator Methods @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickwc, api:, pixel, iterator, methods, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/pixel-iterator.php" rel="canonical">
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
<p class="text-center"><a href="pixel-iterator.php#ClearPixelIterator">ClearPixelIterator</a> &bull; <a href="pixel-iterator.php#ClonePixelIterator">ClonePixelIterator</a> &bull; <a href="pixel-iterator.php#DestroyPixelIterator">DestroyPixelIterator</a> &bull; <a href="pixel-iterator.php#IsPixelIterator">IsPixelIterator</a> &bull; <a href="pixel-iterator.php#NewPixelIterator">NewPixelIterator</a> &bull; <a href="pixel-iterator.php#PixelClearIteratorException">PixelClearIteratorException</a> &bull; <a href="pixel-iterator.php#NewPixelRegionIterator">NewPixelRegionIterator</a> &bull; <a href="pixel-iterator.php#PixelGetCurrentIteratorRow">PixelGetCurrentIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelGetIteratorException">PixelGetIteratorException</a> &bull; <a href="pixel-iterator.php#PixelGetIteratorExceptionType">PixelGetIteratorExceptionType</a> &bull; <a href="pixel-iterator.php#PixelGetIteratorRow">PixelGetIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelGetNextIteratorRow">PixelGetNextIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelGetPreviousIteratorRow">PixelGetPreviousIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelResetIterator">PixelResetIterator</a> &bull; <a href="pixel-iterator.php#PixelSetFirstIteratorRow">PixelSetFirstIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelSetIteratorRow">PixelSetIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelSetLastIteratorRow">PixelSetLastIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelSyncIterator">PixelSyncIterator</a></p>

<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="ClearPixelIterator">ClearPixelIterator</a></h2>

<p>ClearPixelIterator() clear resources associated with a PixelIterator.</p>

<p>The format of the ClearPixelIterator method is:</p>

<pre class="text">
void ClearPixelIterator(PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="ClonePixelIterator">ClonePixelIterator</a></h2>

<p>ClonePixelIterator() makes an exact copy of the specified iterator.</p>

<p>The format of the ClonePixelIterator method is:</p>

<pre class="text">
PixelIterator *ClonePixelIterator(const PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the magick iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="DestroyPixelIterator">DestroyPixelIterator</a></h2>

<p>DestroyPixelIterator() deallocates resources associated with a PixelIterator.</p>

<p>The format of the DestroyPixelIterator method is:</p>

<pre class="text">
PixelIterator *DestroyPixelIterator(PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="IsPixelIterator">IsPixelIterator</a></h2>

<p>IsPixelIterator() returns MagickTrue if the iterator is verified as a pixel iterator.</p>

<p>The format of the IsPixelIterator method is:</p>

<pre class="text">
MagickBooleanType IsPixelIterator(const PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the magick iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="NewPixelIterator">NewPixelIterator</a></h2>

<p>NewPixelIterator() returns a new pixel iterator.</p>

<p>The format of the NewPixelIterator method is:</p>

<pre class="text">
PixelIterator *NewPixelIterator(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelClearIteratorException">PixelClearIteratorException</a></h2>

<p>PixelClearIteratorException() clear any exceptions associated with the iterator.</p>

<p>The format of the PixelClearIteratorException method is:</p>

<pre class="text">
MagickBooleanType PixelClearIteratorException(PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="NewPixelRegionIterator">NewPixelRegionIterator</a></h2>

<p>NewPixelRegionIterator() returns a new pixel iterator.</p>

<p>The format of the NewPixelRegionIterator method is:</p>

<pre class="text">
PixelIterator *NewPixelRegionIterator(MagickWand *wand,const ssize_t x,
  const ssize_t y,const size_t width,const size_t height)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x,y,columns,rows</dt>
<dd> These values define the perimeter of a region of pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetCurrentIteratorRow">PixelGetCurrentIteratorRow</a></h2>

<p>PixelGetCurrentIteratorRow() returns the current row as an array of pixel wands from the pixel iterator.</p>

<p>The format of the PixelGetCurrentIteratorRow method is:</p>

<pre class="text">
PixelWand **PixelGetCurrentIteratorRow(PixelIterator *iterator,
  size_t *number_wands)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd> </dd>
<dt>number_wands</dt>
<dd>the number of pixel wands. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetIteratorException">PixelGetIteratorException</a></h2>

<p>PixelGetIteratorException() returns the severity, reason, and description of any error that occurs when using other methods in this API.</p>

<p>The format of the PixelGetIteratorException method is:</p>

<pre class="text">
char *PixelGetIteratorException(const PixelIterator *iterator,
  ExceptionType *severity)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd> </dd>
<dt>severity</dt>
<dd>the severity of the error is returned here. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetIteratorExceptionType">PixelGetIteratorExceptionType</a></h2>

<p>PixelGetIteratorExceptionType() the exception type associated with the iterator.  If no exception has occurred, UndefinedExceptionType is returned.</p>

<p>The format of the PixelGetIteratorExceptionType method is:</p>

<pre class="text">
ExceptionType PixelGetIteratorExceptionType(
  const PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetIteratorRow">PixelGetIteratorRow</a></h2>

<p>PixelGetIteratorRow() returns the current pixel iterator row.</p>

<p>The format of the PixelGetIteratorRow method is:</p>

<pre class="text">
MagickBooleanType PixelGetIteratorRow(PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetNextIteratorRow">PixelGetNextIteratorRow</a></h2>

<p>PixelGetNextIteratorRow() returns the next row as an array of pixel wands from the pixel iterator.</p>

<p>The format of the PixelGetNextIteratorRow method is:</p>

<pre class="text">
PixelWand **PixelGetNextIteratorRow(PixelIterator *iterator,
  size_t *number_wands)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd> </dd>
<dt>number_wands</dt>
<dd>the number of pixel wands. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetPreviousIteratorRow">PixelGetPreviousIteratorRow</a></h2>

<p>PixelGetPreviousIteratorRow() returns the previous row as an array of pixel wands from the pixel iterator.</p>

<p>The format of the PixelGetPreviousIteratorRow method is:</p>

<pre class="text">
PixelWand **PixelGetPreviousIteratorRow(PixelIterator *iterator,
  size_t *number_wands)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd> </dd>
<dt>number_wands</dt>
<dd>the number of pixel wands. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelResetIterator">PixelResetIterator</a></h2>

<p>PixelResetIterator() resets the pixel iterator.  Use it in conjunction with PixelGetNextIteratorRow() to iterate over all the pixels in a pixel container.</p>

<p>The format of the PixelResetIterator method is:</p>

<pre class="text">
void PixelResetIterator(PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelSetFirstIteratorRow">PixelSetFirstIteratorRow</a></h2>

<p>PixelSetFirstIteratorRow() sets the pixel iterator to the first pixel row.</p>

<p>The format of the PixelSetFirstIteratorRow method is:</p>

<pre class="text">
void PixelSetFirstIteratorRow(PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the magick iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelSetIteratorRow">PixelSetIteratorRow</a></h2>

<p>PixelSetIteratorRow() set the pixel iterator row.</p>

<p>The format of the PixelSetIteratorRow method is:</p>

<pre class="text">
MagickBooleanType PixelSetIteratorRow(PixelIterator *iterator,
  const ssize_t row)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelSetLastIteratorRow">PixelSetLastIteratorRow</a></h2>

<p>PixelSetLastIteratorRow() sets the pixel iterator to the last pixel row.</p>

<p>The format of the PixelSetLastIteratorRow method is:</p>

<pre class="text">
void PixelSetLastIteratorRow(PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the magick iterator. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelSyncIterator">PixelSyncIterator</a></h2>

<p>PixelSyncIterator() syncs the pixel iterator.</p>

<p>The format of the PixelSyncIterator method is:</p>

<pre class="text">
MagickBooleanType PixelSyncIterator(PixelIterator *iterator)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>iterator</dt>
<dd>the pixel iterator. </dd>

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
    <a href="pixel-iterator.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 5th September 2018 20:46 -->
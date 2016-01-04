



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickWand, C API for ImageMagick: Pixel Iterator Methods</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickwc, api, for, imagemagick:, pixel, iterator, methods, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="pixel-iterator.php#ClearPixelIterator">ClearPixelIterator</a> &bull; <a href="pixel-iterator.php#ClonePixelIterator">ClonePixelIterator</a> &bull; <a href="pixel-iterator.php#DestroyPixelIterator">DestroyPixelIterator</a> &bull; <a href="pixel-iterator.php#IsPixelIterator">IsPixelIterator</a> &bull; <a href="pixel-iterator.php#NewPixelIterator">NewPixelIterator</a> &bull; <a href="pixel-iterator.php#PixelClearIteratorException">PixelClearIteratorException</a> &bull; <a href="pixel-iterator.php#NewPixelRegionIterator">NewPixelRegionIterator</a> &bull; <a href="pixel-iterator.php#PixelGetCurrentIteratorRow">PixelGetCurrentIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelGetIteratorException">PixelGetIteratorException</a> &bull; <a href="pixel-iterator.php#PixelGetIteratorExceptionType">PixelGetIteratorExceptionType</a> &bull; <a href="pixel-iterator.php#PixelGetIteratorRow">PixelGetIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelGetNextIteratorRow">PixelGetNextIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelGetPreviousIteratorRow">PixelGetPreviousIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelResetIterator">PixelResetIterator</a> &bull; <a href="pixel-iterator.php#PixelSetFirstIteratorRow">PixelSetFirstIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelSetIteratorRow">PixelSetIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelSetLastIteratorRow">PixelSetLastIteratorRow</a> &bull; <a href="pixel-iterator.php#PixelSyncIterator">PixelSyncIterator</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="ClearPixelIterator">ClearPixelIterator</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="ClonePixelIterator">ClonePixelIterator</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="DestroyPixelIterator">DestroyPixelIterator</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="IsPixelIterator">IsPixelIterator</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="NewPixelIterator">NewPixelIterator</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelClearIteratorException">PixelClearIteratorException</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="NewPixelRegionIterator">NewPixelRegionIterator</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetCurrentIteratorRow">PixelGetCurrentIteratorRow</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetIteratorException">PixelGetIteratorException</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetIteratorExceptionType">PixelGetIteratorExceptionType</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetIteratorRow">PixelGetIteratorRow</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetNextIteratorRow">PixelGetNextIteratorRow</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelGetPreviousIteratorRow">PixelGetPreviousIteratorRow</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelResetIterator">PixelResetIterator</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelSetFirstIteratorRow">PixelSetFirstIteratorRow</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelSetIteratorRow">PixelSetIteratorRow</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelSetLastIteratorRow">PixelSetLastIteratorRow</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/pixel-iterator_8c.html" id="PixelSyncIterator">PixelSyncIterator</a></h2>

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
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="pixel-iterator.php#">Back to top</a> •
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

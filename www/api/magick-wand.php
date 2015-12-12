



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickWand, C API for ImageMagick: Wand Methods</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickwc, api, for, imagemagick:, wmethods, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="magick-wand.php#ClearMagickWand">ClearMagickWand</a> &bull; <a href="magick-wand.php#CloneMagickWand">CloneMagickWand</a> &bull; <a href="magick-wand.php#DestroyMagickWand">DestroyMagickWand</a> &bull; <a href="magick-wand.php#IsMagickWand">IsMagickWand</a> &bull; <a href="magick-wand.php#MagickClearException">MagickClearException</a> &bull; <a href="magick-wand.php#MagickGetException">MagickGetException</a> &bull; <a href="magick-wand.php#MagickGetExceptionType">MagickGetExceptionType</a> &bull; <a href="magick-wand.php#MagickGetIteratorIndex">MagickGetIteratorIndex</a> &bull; <a href="magick-wand.php#MagickQueryConfigureOption">MagickQueryConfigureOption</a> &bull; <a href="magick-wand.php#MagickQueryConfigureOptions">MagickQueryConfigureOptions</a> &bull; <a href="magick-wand.php#MagickQueryFontMetrics">MagickQueryFontMetrics</a> &bull; <a href="magick-wand.php#MagickQueryMultilineFontMetrics">MagickQueryMultilineFontMetrics</a> &bull; <a href="magick-wand.php#MagickQueryFonts">MagickQueryFonts</a> &bull; <a href="magick-wand.php#MagickQueryFormats">MagickQueryFormats</a> &bull; <a href="magick-wand.php#MagickRelinquishMemory">MagickRelinquishMemory</a> &bull; <a href="magick-wand.php#MagickResetIterator">MagickResetIterator</a> &bull; <a href="magick-wand.php#MagickSetFirstIterator">MagickSetFirstIterator</a> &bull; <a href="magick-wand.php#MagickSetIteratorIndex">MagickSetIteratorIndex</a> &bull; <a href="magick-wand.php#MagickSetLastIterator">MagickSetLastIterator</a> &bull; <a href="magick-wand.php#MagickWandGenesis">MagickWandGenesis</a> &bull; <a href="magick-wand.php#MagickWandTerminus">MagickWandTerminus</a> &bull; <a href="magick-wand.php#NewMagickWand">NewMagickWand</a> &bull; <a href="magick-wand.php#NewMagickWandFromImage">NewMagickWandFromImage</a> &bull; <a href="magick-wand.php#IsMagickWandInstantiated">IsMagickWandInstantiated</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="ClearMagickWand">ClearMagickWand</a></h2>

<p>ClearMagickWand() clears resources associated with the wand, leaving the wand blank, and ready to be used for a new set of images.</p>

<p>The format of the ClearMagickWand method is:</p>

<pre class="text">
void ClearMagickWand(MagickWand *wand)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="CloneMagickWand">CloneMagickWand</a></h2>

<p>CloneMagickWand() makes an exact copy of the specified wand.</p>

<p>The format of the CloneMagickWand method is:</p>

<pre class="text">
MagickWand *CloneMagickWand(const MagickWand *wand)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="DestroyMagickWand">DestroyMagickWand</a></h2>

<p>DestroyMagickWand() deallocates memory associated with an MagickWand.</p>

<p>The format of the DestroyMagickWand method is:</p>

<pre class="text">
MagickWand *DestroyMagickWand(MagickWand *wand)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="IsMagickWand">IsMagickWand</a></h2>

<p>IsMagickWand() returns MagickTrue if the wand is verified as a magick wand.</p>

<p>The format of the IsMagickWand method is:</p>

<pre class="text">
MagickBooleanType IsMagickWand(const MagickWand *wand)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickClearException">MagickClearException</a></h2>

<p>MagickClearException() clears any exceptions associated with the wand.</p>

<p>The format of the MagickClearException method is:</p>

<pre class="text">
MagickBooleanType MagickClearException(MagickWand *wand)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickGetException">MagickGetException</a></h2>

<p>MagickGetException() returns the severity, reason, and description of any error that occurs when using other methods in this API.</p>

<p>The format of the MagickGetException method is:</p>

<pre class="text">
char *MagickGetException(const MagickWand *wand,ExceptionType *severity)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>severity</dt>
<dd>the severity of the error is returned here. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickGetExceptionType">MagickGetExceptionType</a></h2>

<p>MagickGetExceptionType() returns the exception type associated with the wand.  If no exception has occurred, UndefinedExceptionType is returned.</p>

<p>The format of the MagickGetExceptionType method is:</p>

<pre class="text">
ExceptionType MagickGetExceptionType(const MagickWand *wand)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickGetIteratorIndex">MagickGetIteratorIndex</a></h2>

<p>MagickGetIteratorIndex() returns the position of the iterator in the image list.</p>

<p>The format of the MagickGetIteratorIndex method is:</p>

<pre class="text">
ssize_t MagickGetIteratorIndex(MagickWand *wand)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickQueryConfigureOption">MagickQueryConfigureOption</a></h2>

<p>MagickQueryConfigureOption() returns the value associated with the specified configure option.</p>

<p>The format of the MagickQueryConfigureOption function is:</p>

<pre class="text">
char *MagickQueryConfigureOption(const char *option)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>option</dt>
<dd>the option name. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickQueryConfigureOptions">MagickQueryConfigureOptions</a></h2>

<p>MagickQueryConfigureOptions() returns any configure options that match the specified pattern (e.g.  "*" for all).  Options include NAME, VERSION, LIB_VERSION, etc.</p>

<p>The format of the MagickQueryConfigureOptions function is:</p>

<pre class="text">
char **MagickQueryConfigureOptions(const char *pattern,
  size_t *number_options)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>pattern</dt>
<dd>Specifies a pointer to a text string containing a pattern. </dd>

<dd> </dd>
<dt>number_options</dt>
<dd> Returns the number of configure options in the list. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickQueryFontMetrics">MagickQueryFontMetrics</a></h2>

<p>MagickQueryFontMetrics() returns a 13 element array representing the following font metrics:</p>

<pre class="text">
    Element Description
    -------------------------------------------------
    0 character width
    1 character height
    2 ascender
    3 descender
    4 text width
    5 text height
    6 maximum horizontal advance
    7 bounding box: x1
    8 bounding box: y1
    9 bounding box: x2
   10 bounding box: y2
   11 origin: x
   12 origin: y
</pre>

<p>The format of the MagickQueryFontMetrics method is:</p>

<pre class="text">
double *MagickQueryFontMetrics(MagickWand *wand,
  const DrawingWand *drawing_wand,const char *text)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the Magick wand. </dd>

<dd> </dd>
<dt>drawing_wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>text</dt>
<dd>the text. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickQueryMultilineFontMetrics">MagickQueryMultilineFontMetrics</a></h2>

<p>MagickQueryMultilineFontMetrics() returns a 13 element array representing the following font metrics:</p>

<pre class="text">
    Element Description
    -------------------------------------------------
    0 character width
    1 character height
    2 ascender
    3 descender
    4 text width
    5 text height
    6 maximum horizontal advance
    7 bounding box: x1
    8 bounding box: y1
    9 bounding box: x2
   10 bounding box: y2
   11 origin: x
   12 origin: y
</pre>

<p>This method is like MagickQueryFontMetrics() but it returns the maximum text width and height for multiple lines of text.</p>

<p>The format of the MagickQueryFontMetrics method is:</p>

<pre class="text">
double *MagickQueryMultilineFontMetrics(MagickWand *wand,
  const DrawingWand *drawing_wand,const char *text)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the Magick wand. </dd>

<dd> </dd>
<dt>drawing_wand</dt>
<dd>the drawing wand. </dd>

<dd> </dd>
<dt>text</dt>
<dd>the text. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickQueryFonts">MagickQueryFonts</a></h2>

<p>MagickQueryFonts() returns any font that match the specified pattern (e.g. "*" for all).</p>

<p>The format of the MagickQueryFonts function is:</p>

<pre class="text">
char **MagickQueryFonts(const char *pattern,size_t *number_fonts)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>pattern</dt>
<dd>Specifies a pointer to a text string containing a pattern. </dd>

<dd> </dd>
<dt>number_fonts</dt>
<dd> Returns the number of fonts in the list. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickQueryFormats">MagickQueryFormats</a></h2>

<p>MagickQueryFormats() returns any image formats that match the specified pattern (e.g.  "*" for all).</p>

<p>The format of the MagickQueryFormats function is:</p>

<pre class="text">
char **MagickQueryFormats(const char *pattern,size_t *number_formats)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>pattern</dt>
<dd>Specifies a pointer to a text string containing a pattern. </dd>

<dd> </dd>
<dt>number_formats</dt>
<dd> This integer returns the number of image formats in the list. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickRelinquishMemory">MagickRelinquishMemory</a></h2>

<p>MagickRelinquishMemory() relinquishes memory resources returned by such methods as MagickIdentifyImage(), MagickGetException(), etc.</p>

<p>The format of the MagickRelinquishMemory method is:</p>

<pre class="text">
void *MagickRelinquishMemory(void *resource)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>resource</dt>
<dd>Relinquish the memory associated with this resource. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickResetIterator">MagickResetIterator</a></h2>

<p>MagickResetIterator() resets the wand iterator.</p>

<p>It is typically used either before iterating though images, or before calling specific functions such as  MagickAppendImages() to append all images together.</p>

<p>Afterward you can use MagickNextImage() to iterate over all the images in a wand container, starting with the first image.</p>

<p>Using this before MagickAddImages() or MagickReadImages() will cause new images to be inserted between the first and second image.</p>

<p>The format of the MagickResetIterator method is:</p>

<pre class="text">
void MagickResetIterator(MagickWand *wand)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickSetFirstIterator">MagickSetFirstIterator</a></h2>

<p>MagickSetFirstIterator() sets the wand iterator to the first image.</p>

<p>After using any images added to the wand using MagickAddImage() or MagickReadImage() will be prepended before any image in the wand.</p>

<p>Also the current image has been set to the first image (if any) in the Magick Wand.  Using MagickNextImage() will then set teh current image to the second image in the list (if present).</p>

<p>This operation is similar to MagickResetIterator() but differs in how MagickAddImage(), MagickReadImage(), and MagickNextImage() behaves afterward.</p>

<p>The format of the MagickSetFirstIterator method is:</p>

<pre class="text">
void MagickSetFirstIterator(MagickWand *wand)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickSetIteratorIndex">MagickSetIteratorIndex</a></h2>

<p>MagickSetIteratorIndex() set the iterator to the given position in the image list specified with the index parameter.  A zero index will set the first image as current, and so on.  Negative indexes can be used to specify an image relative to the end of the images in the wand, with -1 being the last image in the wand.</p>

<p>If the index is invalid (range too large for number of images in wand) the function will return MagickFalse, but no 'exception' will be raised, as it is not actually an error.  In that case the current image will not change.</p>

<p>After using any images added to the wand using MagickAddImage() or MagickReadImage() will be added after the image indexed, regardless of if a zero (first image in list) or negative index (from end) is used.</p>

<p>Jumping to index 0 is similar to MagickResetIterator() but differs in how MagickNextImage() behaves afterward.</p>

<p>The format of the MagickSetIteratorIndex method is:</p>

<pre class="text">
MagickBooleanType MagickSetIteratorIndex(MagickWand *wand,
  const ssize_t index)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>index</dt>
<dd>the scene number. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickSetLastIterator">MagickSetLastIterator</a></h2>

<p>MagickSetLastIterator() sets the wand iterator to the last image.</p>

<p>The last image is actually the current image, and the next use of MagickPreviousImage() will not change this allowing this function to be used to iterate over the images in the reverse direction. In this sense it is more like  MagickResetIterator() than MagickSetFirstIterator().</p>

<p>Typically this function is used before MagickAddImage(), MagickReadImage() functions to ensure new images are appended to the very end of wand's image list.</p>

<p>The format of the MagickSetLastIterator method is:</p>

<pre class="text">
void MagickSetLastIterator(MagickWand *wand)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickWandGenesis">MagickWandGenesis</a></h2>

<p>MagickWandGenesis() initializes the MagickWand environment.</p>

<p>The format of the MagickWandGenesis method is:</p>

<pre class="text">
void MagickWandGenesis(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="MagickWandTerminus">MagickWandTerminus</a></h2>

<p>MagickWandTerminus() terminates the MagickWand environment.</p>

<p>The format of the MaickWandTerminus method is:</p>

<pre class="text">
void MagickWandTerminus(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="NewMagickWand">NewMagickWand</a></h2>

<p>NewMagickWand() returns a wand required for all other methods in the API. A fatal exception is thrown if there is not enough memory to allocate the wand.   Use DestroyMagickWand() to dispose of the wand when it is no longer needed.</p>

<p>The format of the NewMagickWand method is:</p>

<pre class="text">
MagickWand *NewMagickWand(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="NewMagickWandFromImage">NewMagickWandFromImage</a></h2>

<p>NewMagickWandFromImage() returns a wand with an image.</p>

<p>The format of the NewMagickWandFromImage method is:</p>

<pre class="text">
MagickWand *NewMagickWandFromImage(const Image *image)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-wand_8c.html" id="IsMagickWandInstantiated">IsMagickWandInstantiated</a></h2>

<p>IsMagickWandInstantiated() returns MagickTrue if the ImageMagick environment is currently instantiated--  that is, MagickWandGenesis() has been called but MagickWandTerminus() has not.</p>

<p>The format of the IsMagickWandInstantiated method is:</p>

<pre class="text">
MagickBooleanType IsMagickWandInstantiated(void)
</pre>

</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="magick-wand.php#">Back to top</a> •
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

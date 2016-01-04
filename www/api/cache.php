



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Get or Set Image Pixels</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, get, or, set, image, pixels, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="cache.php#AcquirePixelCacheNexus">AcquirePixelCacheNexus</a> &bull; <a href="cache.php#GetAuthenticMetacontent">GetAuthenticMetacontent</a> &bull; <a href="cache.php#GetAuthenticPixelQueue">GetAuthenticPixelQueue</a> &bull; <a href="cache.php#GetAuthenticPixels">GetAuthenticPixels</a> &bull; <a href="cache.php#GetOneAuthenticPixel">GetOneAuthenticPixel</a> &bull; <a href="cache.php#GetOneVirtualPixel">GetOneVirtualPixel</a> &bull; <a href="cache.php#GetOneVirtualPixelInfo">GetOneVirtualPixelInfo</a> &bull; <a href="cache.php#GetVirtualMetacontent">GetVirtualMetacontent</a> &bull; <a href="cache.php#GetVirtualPixelQueue">GetVirtualPixelQueue</a> &bull; <a href="cache.php#GetVirtualPixels">GetVirtualPixels</a> &bull; <a href="cache.php#QueueAuthenticPixels">QueueAuthenticPixels</a> &bull; <a href="cache.php#SetPixelCacheVirtualMethod">SetPixelCacheVirtualMethod</a> &bull; <a href="cache.php#SyncAuthenticPixels">SyncAuthenticPixels</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="AcquirePixelCacheNexus">AcquirePixelCacheNexus</a></h2>

<p>AcquirePixelCacheNexus() allocates the NexusInfo structure.</p>

<p>The format of the AcquirePixelCacheNexus method is:</p>

<pre class="text">
NexusInfo **AcquirePixelCacheNexus(const size_t number_threads)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>number_threads</dt>
<dd>the number of nexus threads. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="GetAuthenticMetacontent">GetAuthenticMetacontent</a></h2>

<p>GetAuthenticMetacontent() returns the authentic metacontent corresponding with the last call to QueueAuthenticPixels() or GetVirtualPixels().  NULL is returned if the associated pixels are not available.</p>

<p>The format of the GetAuthenticMetacontent() method is:</p>

<pre class="text">
void *GetAuthenticMetacontent(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="GetAuthenticPixelQueue">GetAuthenticPixelQueue</a></h2>

<p>GetAuthenticPixelQueue() returns the authentic pixels associated corresponding with the last call to QueueAuthenticPixels() or GetAuthenticPixels().</p>

<p>The format of the GetAuthenticPixelQueue() method is:</p>

<pre class="text">
Quantum *GetAuthenticPixelQueue(const Image image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="GetAuthenticPixels">GetAuthenticPixels</a></h2>

<p>GetAuthenticPixels() obtains a pixel region for read/write access. If the region is successfully accessed, a pointer to a Quantum array representing the region is returned, otherwise NULL is returned.</p>

<p>The returned pointer may point to a temporary working copy of the pixels or it may point to the original pixels in memory. Performance is maximized if the selected region is part of one row, or one or more full rows, since then there is opportunity to access the pixels in-place (without a copy) if the image is in memory, or in a memory-mapped file. The returned pointer must *never* be deallocated by the user.</p>

<p>Pixels accessed via the returned pointer represent a simple array of type Quantum.  If the image has corresponding metacontent,call GetAuthenticMetacontent() after invoking GetAuthenticPixels() to obtain the meta-content corresponding to the region.  Once the Quantum array has been updated, the changes must be saved back to the underlying image using SyncAuthenticPixels() or they may be lost.</p>

<p>The format of the GetAuthenticPixels() method is:</p>

<pre class="text">
Quantum *GetAuthenticPixels(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
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
<dt>x,y,columns,rows</dt>
<dd> These values define the perimeter of a region of pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="GetOneAuthenticPixel">GetOneAuthenticPixel</a></h2>

<p>GetOneAuthenticPixel() returns a single pixel at the specified (x,y) location.  The image background color is returned if an error occurs.</p>

<p>The format of the GetOneAuthenticPixel() method is:</p>

<pre class="text">
MagickBooleanType GetOneAuthenticPixel(const Image image,const ssize_t x,
  const ssize_t y,Quantum *pixel,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>x,y</dt>
<dd> These values define the location of the pixel to return. </dd>

<dd> </dd>
<dt>pixel</dt>
<dd>return a pixel at the specified (x,y) location. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="GetOneVirtualPixel">GetOneVirtualPixel</a></h2>

<p>GetOneVirtualPixel() returns a single virtual pixel at the specified (x,y) location.  The image background color is returned if an error occurs. If you plan to modify the pixel, use GetOneAuthenticPixel() instead.</p>

<p>The format of the GetOneVirtualPixel() method is:</p>

<pre class="text">
MagickBooleanType GetOneVirtualPixel(const Image image,const ssize_t x,
  const ssize_t y,Quantum *pixel,ExceptionInfo exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>x,y</dt>
<dd> These values define the location of the pixel to return. </dd>

<dd> </dd>
<dt>pixel</dt>
<dd>return a pixel at the specified (x,y) location. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="GetOneVirtualPixelInfo">GetOneVirtualPixelInfo</a></h2>

<p>GetOneVirtualPixelInfo() returns a single pixel at the specified (x,y) location.  The image background color is returned if an error occurs.  If you plan to modify the pixel, use GetOneAuthenticPixel() instead.</p>

<p>The format of the GetOneVirtualPixelInfo() method is:</p>

<pre class="text">
MagickBooleanType GetOneVirtualPixelInfo(const Image image,
  const VirtualPixelMethod virtual_pixel_method,const ssize_t x,
  const ssize_t y,PixelInfo *pixel,ExceptionInfo exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>virtual_pixel_method</dt>
<dd>the virtual pixel method. </dd>

<dd> </dd>
<dt>x,y</dt>
<dd> these values define the location of the pixel to return. </dd>

<dd> </dd>
<dt>pixel</dt>
<dd>return a pixel at the specified (x,y) location. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="GetVirtualMetacontent">GetVirtualMetacontent</a></h2>

<p>GetVirtualMetacontent() returns the virtual metacontent corresponding with the last call to QueueAuthenticPixels() or GetVirtualPixels().  NULL is returned if the meta-content are not available.</p>

<p>The format of the GetVirtualMetacontent() method is:</p>

<pre class="text">
const void *GetVirtualMetacontent(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="GetVirtualPixelQueue">GetVirtualPixelQueue</a></h2>

<p>GetVirtualPixelQueue() returns the virtual pixels associated corresponding with the last call to QueueAuthenticPixels() or GetVirtualPixels().</p>

<p>The format of the GetVirtualPixelQueue() method is:</p>

<pre class="text">
const Quantum *GetVirtualPixelQueue(const Image image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="GetVirtualPixels">GetVirtualPixels</a></h2>

<p>GetVirtualPixels() returns an immutable pixel region. If the region is successfully accessed, a pointer to it is returned, otherwise NULL is returned.  The returned pointer may point to a temporary working copy of the pixels or it may point to the original pixels in memory. Performance is maximized if the selected region is part of one row, or one or more full rows, since there is opportunity to access the pixels in-place (without a copy) if the image is in memory, or in a memory-mapped file.  The returned pointer must *never* be deallocated by the user.</p>

<p>Pixels accessed via the returned pointer represent a simple array of type Quantum.  If the image type is CMYK or the storage class is PseudoClass, call GetAuthenticMetacontent() after invoking GetAuthenticPixels() to access the meta-content (of type void) corresponding to the the region.</p>

<p>If you plan to modify the pixels, use GetAuthenticPixels() instead.</p>

<p>Note, the GetVirtualPixels() and GetAuthenticPixels() methods are not thread- safe.  In a threaded environment, use GetCacheViewVirtualPixels() or GetCacheViewAuthenticPixels() instead.</p>

<p>The format of the GetVirtualPixels() method is:</p>

<pre class="text">
const Quantum *GetVirtualPixels(const Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
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
<dt>x,y,columns,rows</dt>
<dd> These values define the perimeter of a region of pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="QueueAuthenticPixels">QueueAuthenticPixels</a></h2>

<p>QueueAuthenticPixels() queues a mutable pixel region.  If the region is successfully initialized a pointer to a Quantum array representing the region is returned, otherwise NULL is returned.  The returned pointer may point to a temporary working buffer for the pixels or it may point to the final location of the pixels in memory.</p>

<p>Write-only access means that any existing pixel values corresponding to the region are ignored.  This is useful if the initial image is being created from scratch, or if the existing pixel values are to be completely replaced without need to refer to their pre-existing values. The application is free to read and write the pixel buffer returned by QueueAuthenticPixels() any way it pleases. QueueAuthenticPixels() does not initialize the pixel array values. Initializing pixel array values is the application's responsibility.</p>

<p>Performance is maximized if the selected region is part of one row, or one or more full rows, since then there is opportunity to access the pixels in-place (without a copy) if the image is in memory, or in a memory-mapped file. The returned pointer must *never* be deallocated by the user.</p>

<p>Pixels accessed via the returned pointer represent a simple array of type Quantum. If the image type is CMYK or the storage class is PseudoClass, call GetAuthenticMetacontent() after invoking GetAuthenticPixels() to obtain the meta-content (of type void) corresponding to the region. Once the Quantum (and/or Quantum) array has been updated, the changes must be saved back to the underlying image using SyncAuthenticPixels() or they may be lost.</p>

<p>The format of the QueueAuthenticPixels() method is:</p>

<pre class="text">
Quantum *QueueAuthenticPixels(Image *image,const ssize_t x,
  const ssize_t y,const size_t columns,const size_t rows,
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
<dt>x,y,columns,rows</dt>
<dd> These values define the perimeter of a region of pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="SetPixelCacheVirtualMethod">SetPixelCacheVirtualMethod</a></h2>

<p>SetPixelCacheVirtualMethod() sets the "virtual pixels" method for the pixel cache and returns the previous setting.  A virtual pixel is any pixel access that is outside the boundaries of the image cache.</p>

<p>The format of the SetPixelCacheVirtualMethod() method is:</p>

<pre class="text">
VirtualPixelMethod SetPixelCacheVirtualMethod(Image *image,
  const VirtualPixelMethod virtual_pixel_method,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>virtual_pixel_method</dt>
<dd>choose the type of virtual pixel. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cache_8c.html" id="SyncAuthenticPixels">SyncAuthenticPixels</a></h2>

<p>SyncAuthenticPixels() saves the image pixels to the in-memory or disk cache. The method returns MagickTrue if the pixel region is flushed, otherwise MagickFalse.</p>

<p>The format of the SyncAuthenticPixels() method is:</p>

<pre class="text">
MagickBooleanType SyncAuthenticPixels(Image *image,
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
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="cache.php#">Back to top</a> •
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

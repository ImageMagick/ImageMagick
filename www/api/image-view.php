



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Image View Methods</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, image, view, methods, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="image-view.php#CloneImageView">CloneImageView</a> &bull; <a href="image-view.php#DestroyImageView">DestroyImageView</a> &bull; <a href="image-view.php#DuplexTransferImageViewIterator">DuplexTransferImageViewIterator</a> &bull; <a href="image-view.php#GetImageViewAuthenticMetacontent">GetImageViewAuthenticMetacontent</a> &bull; <a href="image-view.php#GetImageViewAuthenticPixels">GetImageViewAuthenticPixels</a> &bull; <a href="image-view.php#GetImageViewException">GetImageViewException</a> &bull; <a href="image-view.php#GetImageViewExtent">GetImageViewExtent</a> &bull; <a href="image-view.php#GetImageViewImage">GetImageViewImage</a> &bull; <a href="image-view.php#GetImageViewIterator">GetImageViewIterator</a> &bull; <a href="image-view.php#GetImageViewVirtualMetacontent">GetImageViewVirtualMetacontent</a> &bull; <a href="image-view.php#GetImageViewVirtualPixels">GetImageViewVirtualPixels</a> &bull; <a href="image-view.php#IsImageView">IsImageView</a> &bull; <a href="image-view.php#NewImageView">NewImageView</a> &bull; <a href="image-view.php#NewImageViewRegion">NewImageViewRegion</a> &bull; <a href="image-view.php#SetImageViewDescription">SetImageViewDescription</a> &bull; <a href="image-view.php#SetImageViewIterator">SetImageViewIterator</a> &bull; <a href="image-view.php#TransferImageViewIterator">TransferImageViewIterator</a> &bull; <a href="image-view.php#UpdateImageViewIterator">UpdateImageViewIterator</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="CloneImageView">CloneImageView</a></h2>

<p>CloneImageView() makes a copy of the specified image view.</p>

<p>The format of the CloneImageView method is:</p>

<pre class="text">
ImageView *CloneImageView(const ImageView *image_view)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the image view. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="DestroyImageView">DestroyImageView</a></h2>

<p>DestroyImageView() deallocates memory associated with a image view.</p>

<p>The format of the DestroyImageView method is:</p>

<pre class="text">
ImageView *DestroyImageView(ImageView *image_view)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the image view. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="DuplexTransferImageViewIterator">DuplexTransferImageViewIterator</a></h2>

<p>DuplexTransferImageViewIterator() iterates over three image views in parallel and calls your transfer method for each scanline of the view.  The source and duplex pixel extent is not confined to the image canvas-- that is you can include negative offsets or widths or heights that exceed the image dimension.  However, the destination image view is confined to the image canvas-- that is no negative offsets or widths or heights that exceed the image dimension are permitted.</p>

<p>The callback signature is:</p>

<pre class="text">
MagickBooleanType DuplexTransferImageViewMethod(const ImageView *source,
  const ImageView *duplex,ImageView *destination,const ssize_t y,
  const int thread_id,void *context)
</pre>

<p>Use this pragma if the view is not single threaded:</p>

<pre class="text">
    #pragma omp critical
</pre>

<p>to define a section of code in your callback transfer method that must be executed by a single thread at a time.</p>

<p>The format of the DuplexTransferImageViewIterator method is:</p>

<pre class="text">
MagickBooleanType DuplexTransferImageViewIterator(ImageView *source,
  ImageView *duplex,ImageView *destination,
  DuplexTransferImageViewMethod transfer,void *context)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>source</dt>
<dd>the source image view. </dd>

<dd> </dd>
<dt>duplex</dt>
<dd>the duplex image view. </dd>

<dd> </dd>
<dt>destination</dt>
<dd>the destination image view. </dd>

<dd> </dd>
<dt>transfer</dt>
<dd>the transfer callback method. </dd>

<dd> </dd>
<dt>context</dt>
<dd>the user defined context. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="GetImageViewAuthenticMetacontent">GetImageViewAuthenticMetacontent</a></h2>

<p>GetImageViewAuthenticMetacontent() returns the image view authentic meta-content.</p>

<p>The format of the GetImageViewAuthenticPixels method is:</p>

<pre class="text">
void *GetImageViewAuthenticMetacontent(
  const ImageView *image_view)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the image view. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="GetImageViewAuthenticPixels">GetImageViewAuthenticPixels</a></h2>

<p>GetImageViewAuthenticPixels() returns the image view authentic pixels.</p>

<p>The format of the GetImageViewAuthenticPixels method is:</p>

<pre class="text">
Quantum *GetImageViewAuthenticPixels(const ImageView *image_view)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the image view. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="GetImageViewException">GetImageViewException</a></h2>

<p>GetImageViewException() returns the severity, reason, and description of any error that occurs when utilizing a image view.</p>

<p>The format of the GetImageViewException method is:</p>

<pre class="text">
char *GetImageViewException(const PixelImage *image_view,
  ExceptionType *severity)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the pixel image_view. </dd>

<dd> </dd>
<dt>severity</dt>
<dd>the severity of the error is returned here. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="GetImageViewExtent">GetImageViewExtent</a></h2>

<p>GetImageViewExtent() returns the image view extent.</p>

<p>The format of the GetImageViewExtent method is:</p>

<pre class="text">
RectangleInfo GetImageViewExtent(const ImageView *image_view)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the image view. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="GetImageViewImage">GetImageViewImage</a></h2>

<p>GetImageViewImage() returns the image associated with the image view.</p>

<p>The format of the GetImageViewImage method is:</p>

<pre class="text">
MagickCore *GetImageViewImage(const ImageView *image_view)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the image view. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="GetImageViewIterator">GetImageViewIterator</a></h2>

<p>GetImageViewIterator() iterates over the image view in parallel and calls your get method for each scanline of the view.  The pixel extent is not confined to the image canvas-- that is you can include negative offsets or widths or heights that exceed the image dimension.  Any updates to the pixels in your callback are ignored.</p>

<p>The callback signature is:</p>

<pre class="text">
MagickBooleanType GetImageViewMethod(const ImageView *source,
  const ssize_t y,const int thread_id,void *context)
</pre>

<p>Use this pragma if the view is not single threaded:</p>

<pre class="text">
    #pragma omp critical
</pre>

<p>to define a section of code in your callback get method that must be executed by a single thread at a time.</p>

<p>The format of the GetImageViewIterator method is:</p>

<pre class="text">
MagickBooleanType GetImageViewIterator(ImageView *source,
  GetImageViewMethod get,void *context)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>source</dt>
<dd>the source image view. </dd>

<dd> </dd>
<dt>get</dt>
<dd>the get callback method. </dd>

<dd> </dd>
<dt>context</dt>
<dd>the user defined context. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="GetImageViewVirtualMetacontent">GetImageViewVirtualMetacontent</a></h2>

<p>GetImageViewVirtualMetacontent() returns the image view virtual meta-content.</p>

<p>The format of the GetImageViewVirtualMetacontent method is:</p>

<pre class="text">
const void *GetImageViewVirtualMetacontent(
  const ImageView *image_view)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the image view. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="GetImageViewVirtualPixels">GetImageViewVirtualPixels</a></h2>

<p>GetImageViewVirtualPixels() returns the image view virtual pixels.</p>

<p>The format of the GetImageViewVirtualPixels method is:</p>

<pre class="text">
const Quantum *GetImageViewVirtualPixels(const ImageView *image_view)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the image view. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="IsImageView">IsImageView</a></h2>

<p>IsImageView() returns MagickTrue if the the parameter is verified as a image view object.</p>

<p>The format of the IsImageView method is:</p>

<pre class="text">
MagickBooleanType IsImageView(const ImageView *image_view)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the image view. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="NewImageView">NewImageView</a></h2>

<p>NewImageView() returns a image view required for all other methods in the Image View API.</p>

<p>The format of the NewImageView method is:</p>

<pre class="text">
ImageView *NewImageView(MagickCore *wand,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="NewImageViewRegion">NewImageViewRegion</a></h2>

<p>NewImageViewRegion() returns a image view required for all other methods in the Image View API.</p>

<p>The format of the NewImageViewRegion method is:</p>

<pre class="text">
ImageView *NewImageViewRegion(MagickCore *wand,const ssize_t x,
  const ssize_t y,const size_t width,const size_t height,
  ExceptionInfo *exception)
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
<dd> These values define the perimeter of a extent of pixel_wands view. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="SetImageViewDescription">SetImageViewDescription</a></h2>

<p>SetImageViewDescription() associates a description with an image view.</p>

<p>The format of the SetImageViewDescription method is:</p>

<pre class="text">
void SetImageViewDescription(ImageView *image_view,
  const char *description)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_view</dt>
<dd>the image view. </dd>

<dd> </dd>
<dt>description</dt>
<dd>the image view description. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="SetImageViewIterator">SetImageViewIterator</a></h2>

<p>SetImageViewIterator() iterates over the image view in parallel and calls your set method for each scanline of the view.  The pixel extent is confined to the image canvas-- that is no negative offsets or widths or heights that exceed the image dimension.  The pixels are initiallly undefined and any settings you make in the callback method are automagically synced back to your image.</p>

<p>The callback signature is:</p>

<pre class="text">
MagickBooleanType SetImageViewMethod(ImageView *destination,
  const ssize_t y,const int thread_id,void *context)
</pre>

<p>Use this pragma if the view is not single threaded:</p>

<pre class="text">
    #pragma omp critical
</pre>

<p>to define a section of code in your callback set method that must be executed by a single thread at a time.</p>

<p>The format of the SetImageViewIterator method is:</p>

<pre class="text">
MagickBooleanType SetImageViewIterator(ImageView *destination,
  SetImageViewMethod set,void *context)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>destination</dt>
<dd>the image view. </dd>

<dd> </dd>
<dt>set</dt>
<dd>the set callback method. </dd>

<dd> </dd>
<dt>context</dt>
<dd>the user defined context. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="TransferImageViewIterator">TransferImageViewIterator</a></h2>

<p>TransferImageViewIterator() iterates over two image views in parallel and calls your transfer method for each scanline of the view.  The source pixel extent is not confined to the image canvas-- that is you can include negative offsets or widths or heights that exceed the image dimension. However, the destination image view is confined to the image canvas-- that is no negative offsets or widths or heights that exceed the image dimension are permitted.</p>

<p>The callback signature is:</p>

<pre class="text">
MagickBooleanType TransferImageViewMethod(const ImageView *source,
  ImageView *destination,const ssize_t y,const int thread_id,
  void *context)
</pre>

<p>Use this pragma if the view is not single threaded:</p>

<pre class="text">
    #pragma omp critical
</pre>

<p>to define a section of code in your callback transfer method that must be executed by a single thread at a time.</p>

<p>The format of the TransferImageViewIterator method is:</p>

<pre class="text">
MagickBooleanType TransferImageViewIterator(ImageView *source,
  ImageView *destination,TransferImageViewMethod transfer,void *context)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>source</dt>
<dd>the source image view. </dd>

<dd> </dd>
<dt>destination</dt>
<dd>the destination image view. </dd>

<dd> </dd>
<dt>transfer</dt>
<dd>the transfer callback method. </dd>

<dd> </dd>
<dt>context</dt>
<dd>the user defined context. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image-view_8c.html" id="UpdateImageViewIterator">UpdateImageViewIterator</a></h2>

<p>UpdateImageViewIterator() iterates over the image view in parallel and calls your update method for each scanline of the view.  The pixel extent is confined to the image canvas-- that is no negative offsets or widths or heights that exceed the image dimension are permitted.  Updates to pixels in your callback are automagically synced back to the image.</p>

<p>The callback signature is:</p>

<pre class="text">
MagickBooleanType UpdateImageViewMethod(ImageView *source,
  const ssize_t y,const int thread_id,void *context)
</pre>

<p>Use this pragma if the view is not single threaded:</p>

<pre class="text">
    #pragma omp critical
</pre>

<p>to define a section of code in your callback update method that must be executed by a single thread at a time.</p>

<p>The format of the UpdateImageViewIterator method is:</p>

<pre class="text">
MagickBooleanType UpdateImageViewIterator(ImageView *source,
  UpdateImageViewMethod update,void *context)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>source</dt>
<dd>the source image view. </dd>

<dd> </dd>
<dt>update</dt>
<dd>the update callback method. </dd>

<dd> </dd>
<dt>context</dt>
<dd>the user defined context. </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="image-view.php#">Back to top</a> •
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

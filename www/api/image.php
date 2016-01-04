



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Image Methods</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, image, methods, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="image.php#AcquireImage">AcquireImage</a> &bull; <a href="image.php#AcquireImageInfo">AcquireImageInfo</a> &bull; <a href="image.php#AcquireNextImage">AcquireNextImage</a> &bull; <a href="image.php#AppendImages">AppendImages</a> &bull; <a href="image.php#CatchImageException">CatchImageException</a> &bull; <a href="image.php#ClipImagePath">ClipImagePath</a> &bull; <a href="image.php#CloneImage">CloneImage</a> &bull; <a href="image.php#CloneImageInfo">CloneImageInfo</a> &bull; <a href="image.php#CopyImagePixels">CopyImagePixels</a> &bull; <a href="image.php#DestroyImage">DestroyImage</a> &bull; <a href="image.php#DestroyImageInfo">DestroyImageInfo</a> &bull; <a href="image.php#GetImageInfo">GetImageInfo</a> &bull; <a href="image.php#GetImageInfoFile">GetImageInfoFile</a> &bull; <a href="image.php#GetImageMask">GetImageMask</a> &bull; <a href="image.php#GetImageVirtualPixelMethod">GetImageVirtualPixelMethod</a> &bull; <a href="image.php#InterpretImageFilename">InterpretImageFilename</a> &bull; <a href="image.php#IsHighDynamicRangeImage">IsHighDynamicRangeImage</a> &bull; <a href="image.php#IsImageObject">IsImageObject</a> &bull; <a href="image.php#IsTaintImage">IsTaintImage</a> &bull; <a href="image.php#ModifyImage">ModifyImage</a> &bull; <a href="image.php#NewMagickImage">NewMagickImage</a> &bull; <a href="image.php#ReferenceImage">ReferenceImage</a> &bull; <a href="image.php#ResetImagePage">ResetImagePage</a> &bull; <a href="image.php#SetImageBackgroundColor">SetImageBackgroundColor</a> &bull; <a href="image.php#SetImageChannelMask">SetImageChannelMask</a> &bull; <a href="image.php#SetImageColor">SetImageColor</a> &bull; <a href="image.php#SetImageStorageClass">SetImageStorageClass</a> &bull; <a href="image.php#SetImageExtent">SetImageExtent</a> &bull; <a href="image.php#SetImageInfoBlob">SetImageInfoBlob</a> &bull; <a href="image.php#SetImageInfoFile">SetImageInfoFile</a> &bull; <a href="image.php#SetImageMask">SetImageMask</a> &bull; <a href="image.php#SetImageAlpha">SetImageAlpha</a> &bull; <a href="image.php#SetImageVirtualPixelMethod">SetImageVirtualPixelMethod</a> &bull; <a href="image.php#SmushImages">SmushImages</a> &bull; <a href="image.php#StripImage">StripImage</a> &bull; <a href="image.php#SyncImageSettings">SyncImageSettings</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="AcquireImage">AcquireImage</a></h2>

<p>AcquireImage() returns a pointer to an image structure initialized to default values.</p>

<p>The format of the AcquireImage method is:</p>

<pre class="text">
Image *AcquireImage(const ImageInfo *image_info,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>Many of the image default values are set from this structure.  For example, filename, compression, depth, background color, and others. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="AcquireImageInfo">AcquireImageInfo</a></h2>

<p>AcquireImageInfo() allocates the ImageInfo structure.</p>

<p>The format of the AcquireImageInfo method is:</p>

<pre class="text">
ImageInfo *AcquireImageInfo(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="AcquireNextImage">AcquireNextImage</a></h2>

<p>AcquireNextImage() initializes the next image in a sequence to default values.  The next member of image points to the newly allocated image.  If there is a memory shortage, next is assigned NULL.</p>

<p>The format of the AcquireNextImage method is:</p>

<pre class="text">
void AcquireNextImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>Many of the image default values are set from this structure.  For example, filename, compression, depth, background color, and others. </dd>

<dd> </dd>
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="AppendImages">AppendImages</a></h2>

<p>AppendImages() takes all images from the current image pointer to the end of the image list and appends them to each other top-to-bottom if the stack parameter is true, otherwise left-to-right.</p>

<p>The current gravity setting effects how the image is justified in the final image.</p>

<p>The format of the AppendImages method is:</p>

<pre class="text">
Image *AppendImages(const Image *images,const MagickBooleanType stack,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>images</dt>
<dd>the image sequence. </dd>

<dd> </dd>
<dt>stack</dt>
<dd>A value other than 0 stacks the images top-to-bottom. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="CatchImageException">CatchImageException</a></h2>

<p>CatchImageException() returns if no exceptions are found in the image sequence, otherwise it determines the most severe exception and reports it as a warning or error depending on the severity.</p>

<p>The format of the CatchImageException method is:</p>

<pre class="text">
ExceptionType CatchImageException(Image *image)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>An image sequence. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="ClipImagePath">ClipImagePath</a></h2>

<p>ClipImagePath() sets the image clip mask based any clipping path information if it exists.</p>

<p>The format of the ClipImagePath method is:</p>

<pre class="text">
MagickBooleanType ClipImagePath(Image *image,const char *pathname,
  const MagickBooleanType inside,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>pathname</dt>
<dd>name of clipping path resource. If name is preceded by #, use clipping path numbered by name. </dd>

<dd> </dd>
<dt>inside</dt>
<dd>if non-zero, later operations take effect inside clipping path. Otherwise later operations take effect outside clipping path. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="CloneImage">CloneImage</a></h2>

<p>CloneImage() copies an image and returns the copy as a new image object.</p>

<p>If the specified columns and rows is 0, an exact copy of the image is returned, otherwise the pixel data is undefined and must be initialized with the QueueAuthenticPixels() and SyncAuthenticPixels() methods.  On failure, a NULL image is returned and exception describes the reason for the failure.</p>

<p>The format of the CloneImage method is:</p>

<pre class="text">
Image *CloneImage(const Image *image,const size_t columns,
  const size_t rows,const MagickBooleanType orphan,
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
<dt>columns</dt>
<dd>the number of columns in the cloned image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the number of rows in the cloned image. </dd>

<dd> </dd>
<dt>detach</dt>
<dd> With a value other than 0, the cloned image is detached from its parent I/O stream. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="CloneImageInfo">CloneImageInfo</a></h2>

<p>CloneImageInfo() makes a copy of the given image info structure.  If NULL is specified, a new image info structure is created initialized to default values.</p>

<p>The format of the CloneImageInfo method is:</p>

<pre class="text">
ImageInfo *CloneImageInfo(const ImageInfo *image_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="CopyImagePixels">CopyImagePixels</a></h2>

<p>CopyImagePixels() copies pixels from the source image as defined by the geometry the destination image at the specified offset.</p>

<p>The format of the CopyImagePixels method is:</p>

<pre class="text">
MagickBooleanType CopyImagePixels(Image *image,const Image *source_image,
  const RectangleInfo *geometry,const OffsetInfo *offset,
  ExceptionInfo *exception);
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the destination image. </dd>

<dd> </dd>
<dt>source_image</dt>
<dd>the source image. </dd>

<dd> </dd>
<dt>geometry</dt>
<dd>define the dimensions of the source pixel rectangle. </dd>

<dd> </dd>
<dt>offset</dt>
<dd>define the offset in the destination image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="DestroyImage">DestroyImage</a></h2>

<p>DestroyImage() dereferences an image, deallocating memory associated with the image if the reference count becomes zero.</p>

<p>The format of the DestroyImage method is:</p>

<pre class="text">
Image *DestroyImage(Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="DestroyImageInfo">DestroyImageInfo</a></h2>

<p>DestroyImageInfo() deallocates memory associated with an ImageInfo structure.</p>

<p>The format of the DestroyImageInfo method is:</p>

<pre class="text">
ImageInfo *DestroyImageInfo(ImageInfo *image_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="GetImageInfo">GetImageInfo</a></h2>

<p>GetImageInfo() initializes image_info to default values.</p>

<p>The format of the GetImageInfo method is:</p>

<pre class="text">
void GetImageInfo(ImageInfo *image_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="GetImageInfoFile">GetImageInfoFile</a></h2>

<p>GetImageInfoFile() returns the image info file member.</p>

<p>The format of the GetImageInfoFile method is:</p>

<pre class="text">
FILE *GetImageInfoFile(const ImageInfo *image_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="GetImageMask">GetImageMask</a></h2>

<p>GetImageMask() returns the mask associated with the image.</p>

<p>The format of the GetImageMask method is:</p>

<pre class="text">
Image *GetImageMask(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="GetImageVirtualPixelMethod">GetImageVirtualPixelMethod</a></h2>

<p>GetImageVirtualPixelMethod() gets the "virtual pixels" method for the image.  A virtual pixel is any pixel access that is outside the boundaries of the image cache.</p>

<p>The format of the GetImageVirtualPixelMethod() method is:</p>

<pre class="text">
VirtualPixelMethod GetImageVirtualPixelMethod(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="InterpretImageFilename">InterpretImageFilename</a></h2>

<p>InterpretImageFilename() interprets embedded characters in an image filename. The filename length is returned.</p>

<p>The format of the InterpretImageFilename method is:</p>

<pre class="text">
size_t InterpretImageFilename(const ImageInfo *image_info,Image *image,
  const char *format,int value,char *filename,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image_info</dt>
<p>the image info..</p>

<dt>image</dt>
<p>the image.</p>

<dt>format</dt>
<p>A filename describing the format to use to write the numeric argument. Only the first numeric format identifier is replaced.</p>

<dt>value</dt>
<p>Numeric value to substitute into format filename.</p>

<dt>filename</dt>
<p>return the formatted filename in this character buffer.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="IsHighDynamicRangeImage">IsHighDynamicRangeImage</a></h2>

<p>IsHighDynamicRangeImage() returns MagickTrue if any pixel component is non-integer or exceeds the bounds of the quantum depth (e.g. for Q16 0..65535.</p>

<p>The format of the IsHighDynamicRangeImage method is:</p>

<pre class="text">
MagickBooleanType IsHighDynamicRangeImage(const Image *image,
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="IsImageObject">IsImageObject</a></h2>

<p>IsImageObject() returns MagickTrue if the image sequence contains a valid set of image objects.</p>

<p>The format of the IsImageObject method is:</p>

<pre class="text">
MagickBooleanType IsImageObject(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="IsTaintImage">IsTaintImage</a></h2>

<p>IsTaintImage() returns MagickTrue any pixel in the image has been altered since it was first constituted.</p>

<p>The format of the IsTaintImage method is:</p>

<pre class="text">
MagickBooleanType IsTaintImage(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="ModifyImage">ModifyImage</a></h2>

<p>ModifyImage() ensures that there is only a single reference to the image to be modified, updating the provided image pointer to point to a clone of the original image if necessary.</p>

<p>The format of the ModifyImage method is:</p>

<pre class="text">
MagickBooleanType ModifyImage(Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="NewMagickImage">NewMagickImage</a></h2>

<p>NewMagickImage() creates a blank image canvas of the specified size and background color.</p>

<p>The format of the NewMagickImage method is:</p>

<pre class="text">
Image *NewMagickImage(const ImageInfo *image_info,const size_t width,
  const size_t height,const PixelInfo *background,
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
<dt>width</dt>
<dd>the image width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the image height. </dd>

<dd> </dd>
<dt>background</dt>
<dd>the image color. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="ReferenceImage">ReferenceImage</a></h2>

<p>ReferenceImage() increments the reference count associated with an image returning a pointer to the image.</p>

<p>The format of the ReferenceImage method is:</p>

<pre class="text">
Image *ReferenceImage(Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="ResetImagePage">ResetImagePage</a></h2>

<p>ResetImagePage() resets the image page canvas and position.</p>

<p>The format of the ResetImagePage method is:</p>

<pre class="text">
MagickBooleanType ResetImagePage(Image *image,const char *page)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>page</dt>
<dd>the relative page specification. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SetImageBackgroundColor">SetImageBackgroundColor</a></h2>

<p>SetImageBackgroundColor() initializes the image pixels to the image background color.  The background color is defined by the background_color member of the image structure.</p>

<p>The format of the SetImage method is:</p>

<pre class="text">
MagickBooleanType SetImageBackgroundColor(Image *image,
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SetImageChannelMask">SetImageChannelMask</a></h2>

<p>SetImageChannelMask() sets the image channel mask from the specified channel mask.</p>

<p>The format of the SetImageChannelMask method is:</p>

<pre class="text">
ChannelType SetImageChannelMask(Image *image,
  const ChannelType channel_mask)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>channel_mask</dt>
<dd>the channel mask. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SetImageColor">SetImageColor</a></h2>

<p>SetImageColor() set the entire image canvas to the specified color.</p>

<p>The format of the SetImageColor method is:</p>

<pre class="text">
MagickBooleanType SetImageColor(Image *image,const PixelInfo *color,
  ExeptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>background</dt>
<dd>the image color. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SetImageStorageClass">SetImageStorageClass</a></h2>

<p>SetImageStorageClass() sets the image class: DirectClass for true color images or PseudoClass for colormapped images.</p>

<p>The format of the SetImageStorageClass method is:</p>

<pre class="text">
MagickBooleanType SetImageStorageClass(Image *image,
  const ClassType storage_class,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>storage_class</dt>
<dd> The image class. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SetImageExtent">SetImageExtent</a></h2>

<p>SetImageExtent() sets the image size (i.e. columns &amp; rows).</p>

<p>The format of the SetImageExtent method is:</p>

<pre class="text">
MagickBooleanType SetImageExtent(Image *image,const size_t columns,
  const size_t rows,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>columns</dt>
<dd> The image width in pixels. </dd>

<dd> </dd>
<dt>rows</dt>
<dd> The image height in pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SetImageInfoBlob">SetImageInfoBlob</a></h2>

<p>SetImageInfoBlob() sets the image info blob member.</p>

<p>The format of the SetImageInfoBlob method is:</p>

<pre class="text">
void SetImageInfoBlob(ImageInfo *image_info,const void *blob,
  const size_t length)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd> </dd>
<dt>blob</dt>
<dd>the blob. </dd>

<dd> </dd>
<dt>length</dt>
<dd>the blob length. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SetImageInfoFile">SetImageInfoFile</a></h2>

<p>SetImageInfoFile() sets the image info file member.</p>

<p>The format of the SetImageInfoFile method is:</p>

<pre class="text">
void SetImageInfoFile(ImageInfo *image_info,FILE *file)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd> </dd>
<dt>file</dt>
<dd>the file. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SetImageMask">SetImageMask</a></h2>

<p>SetImageMask() associates a mask with the image.  The mask must be the same dimensions as the image.</p>

<p>The format of the SetImageMask method is:</p>

<pre class="text">
MagickBooleanType SetImageMask(Image *image,const PixelMask type,
  const Image *mask,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>type</dt>
<dd>the mask type, ReadPixelMask or WritePixelMask. </dd>

<dd> </dd>
<dt>mask</dt>
<dd>the image mask. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SetImageAlpha">SetImageAlpha</a></h2>

<p>SetImageAlpha() sets the alpha levels of the image.</p>

<p>The format of the SetImageAlpha method is:</p>

<pre class="text">
MagickBooleanType SetImageAlpha(Image *image,const Quantum alpha,
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
<dt>Alpha</dt>
<dd>the level of transparency: 0 is fully opaque and QuantumRange is fully transparent. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SetImageVirtualPixelMethod">SetImageVirtualPixelMethod</a></h2>

<p>SetImageVirtualPixelMethod() sets the "virtual pixels" method for the image and returns the previous setting.  A virtual pixel is any pixel access that is outside the boundaries of the image cache.</p>

<p>The format of the SetImageVirtualPixelMethod() method is:</p>

<pre class="text">
VirtualPixelMethod SetImageVirtualPixelMethod(Image *image,
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SmushImages">SmushImages</a></h2>

<p>SmushImages() takes all images from the current image pointer to the end of the image list and smushes them to each other top-to-bottom if the stack parameter is true, otherwise left-to-right.</p>

<p>The current gravity setting now effects how the image is justified in the final image.</p>

<p>The format of the SmushImages method is:</p>

<pre class="text">
Image *SmushImages(const Image *images,const MagickBooleanType stack,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>images</dt>
<dd>the image sequence. </dd>

<dd> </dd>
<dt>stack</dt>
<dd>A value other than 0 stacks the images top-to-bottom. </dd>

<dd> </dd>
<dt>offset</dt>
<dd>minimum distance in pixels between images. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="StripImage">StripImage</a></h2>

<p>StripImage() strips an image of all profiles and comments.</p>

<p>The format of the StripImage method is:</p>

<pre class="text">
MagickBooleanType StripImage(Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/image_8c.html" id="SyncImageSettings">SyncImageSettings</a></h2>

<p>SyncImageSettings() syncs any image_info global options into per-image attributes.</p>

<p>Note: in IMv6 free form 'options' were always mapped into 'artifacts', so that operations and coders can find such settings.  In IMv7 if a desired per-image artifact is not set, then it will directly look for a global option as a fallback, as such this copy is no longer needed, only the link set up.</p>

<p>The format of the SyncImageSettings method is:</p>

<pre class="text">
MagickBooleanType SyncImageSettings(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
MagickBooleanType SyncImagesSettings(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd> </dd>
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
    <p><a href="image.php#">Back to top</a> •
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

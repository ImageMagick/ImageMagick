



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Transform an Image</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, transform, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="transform.php#AutoOrientImage">AutoOrientImage</a> &bull; <a href="transform.php#ChopImage">ChopImage</a> &bull; <a href="transform.php#CropImage">CropImage</a> &bull; <a href="transform.php#CropImageToTiles">CropImageToTiles</a> &bull; <a href="transform.php#ExcerptImage">ExcerptImage</a> &bull; <a href="transform.php#ExtentImage">ExtentImage</a> &bull; <a href="transform.php#FlipImage">FlipImage</a> &bull; <a href="transform.php#FlopImage">FlopImage</a> &bull; <a href="transform.php#RollImage">RollImage</a> &bull; <a href="transform.php#ShaveImage">ShaveImage</a> &bull; <a href="transform.php#SpliceImage">SpliceImage</a> &bull; <a href="transform.php#TransformImage">TransformImage</a> &bull; <a href="transform.php#TransformImages">TransformImages</a> &bull; <a href="transform.php#TransposeImage">TransposeImage</a> &bull; <a href="transform.php#TransverseImage">TransverseImage</a> &bull; <a href="transform.php#TrimImage">TrimImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="AutoOrientImage">AutoOrientImage</a></h2>

<p>AutoOrientImage() adjusts an image so that its orientation is suitable for viewing (i.e. top-left orientation).</p>

<p>The format of the AutoOrientImage method is:</p>

<pre class="text">
Image *AutoOrientImage(const Image *image,
  const OrientationType orientation,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>The image. </dd>

<dd> </dd>
<dt>orientation</dt>
<dd>Current image orientation. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>Return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="ChopImage">ChopImage</a></h2>

<p>ChopImage() removes a region of an image and collapses the image to occupy the removed portion.</p>

<p>The format of the ChopImage method is:</p>

<pre class="text">
Image *ChopImage(const Image *image,const RectangleInfo *chop_info)
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
<dt>chop_info</dt>
<dd>Define the region of the image to chop. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="CropImage">CropImage</a></h2>

<p>CropImage() extracts a region of the image starting at the offset defined by geometry.  Region must be fully defined, and no special handling of geometry flags is performed.</p>

<p>The format of the CropImage method is:</p>

<pre class="text">
Image *CropImage(const Image *image,const RectangleInfo *geometry,
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
<dt>geometry</dt>
<dd>Define the region of the image to crop with members x, y, width, and height. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="CropImageToTiles">CropImageToTiles</a></h2>

<p>CropImageToTiles() crops a single image, into a possible list of tiles. This may include a single sub-region of the image.  This basically applies all the normal geometry flags for Crop.</p>

<p>Image *CropImageToTiles(const Image *image, const RectangleInfo *crop_geometry, ExceptionInfo *exception)</p>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image The transformed image is returned as this parameter. </dd>

<dd> </dd>
<dt>crop_geometry</dt>
<dd>A crop geometry string. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="ExcerptImage">ExcerptImage</a></h2>

<p>ExcerptImage() returns a excerpt of the image as defined by the geometry.</p>

<p>The format of the ExcerptImage method is:</p>

<pre class="text">
Image *ExcerptImage(const Image *image,const RectangleInfo *geometry,
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
<dt>geometry</dt>
<dd>Define the region of the image to extend with members x, y, width, and height. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="ExtentImage">ExtentImage</a></h2>

<p>ExtentImage() extends the image as defined by the geometry, gravity, and image background color.  Set the (x,y) offset of the geometry to move the original image relative to the extended image.</p>

<p>The format of the ExtentImage method is:</p>

<pre class="text">
Image *ExtentImage(const Image *image,const RectangleInfo *geometry,
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
<dt>geometry</dt>
<dd>Define the region of the image to extend with members x, y, width, and height. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="FlipImage">FlipImage</a></h2>

<p>FlipImage() creates a vertical mirror image by reflecting the pixels around the central x-axis.</p>

<p>The format of the FlipImage method is:</p>

<pre class="text">
Image *FlipImage(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="FlopImage">FlopImage</a></h2>

<p>FlopImage() creates a horizontal mirror image by reflecting the pixels around the central y-axis.</p>

<p>The format of the FlopImage method is:</p>

<pre class="text">
Image *FlopImage(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="RollImage">RollImage</a></h2>

<p>RollImage() offsets an image as defined by x_offset and y_offset.</p>

<p>The format of the RollImage method is:</p>

<pre class="text">
Image *RollImage(const Image *image,const ssize_t x_offset,
  const ssize_t y_offset,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>x_offset</dt>
<dd>the number of columns to roll in the horizontal direction. </dd>

<dd> </dd>
<dt>y_offset</dt>
<dd>the number of rows to roll in the vertical direction. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="ShaveImage">ShaveImage</a></h2>

<p>ShaveImage() shaves pixels from the image edges.  It allocates the memory necessary for the new Image structure and returns a pointer to the new image.</p>

<p>The format of the ShaveImage method is:</p>

<pre class="text">
Image *ShaveImage(const Image *image,const RectangleInfo *shave_info,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>shave_image</dt>
<dd>Method ShaveImage returns a pointer to the shaved image.  A null image is returned if there is a memory shortage or if the image width or height is zero. </dd>

<dd> </dd>
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>shave_info</dt>
<dd>Specifies a pointer to a RectangleInfo which defines the region of the image to crop. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="SpliceImage">SpliceImage</a></h2>

<p>SpliceImage() splices a solid color into the image as defined by the geometry.</p>

<p>The format of the SpliceImage method is:</p>

<pre class="text">
Image *SpliceImage(const Image *image,const RectangleInfo *geometry,
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
<dt>geometry</dt>
<dd>Define the region of the image to splice with members x, y, width, and height. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="TransformImage">TransformImage</a></h2>

<p>TransformImage() is a convenience method that behaves like ResizeImage() or CropImage() but accepts scaling and/or cropping information as a region geometry specification.  If the operation fails, the original image handle is left as is.</p>

<p>This should only be used for single images.</p>

<p>This function destroys what it assumes to be a single image list. If the input image is part of a larger list, all other images in that list will be simply 'lost', not destroyed.</p>

<p>Also if the crop generates a list of images only the first image is resized. And finally if the crop succeeds and the resize failed, you will get a cropped image, as well as a 'false' or 'failed' report.</p>

<p>This function and should probably be deprecated in favor of direct calls to CropImageToTiles() or ResizeImage(), as appropriate.</p>

<p>The format of the TransformImage method is:</p>

<pre class="text">
MagickBooleanType TransformImage(Image **image,const char *crop_geometry,
  const char *image_geometry,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image The transformed image is returned as this parameter. </dd>

<dd> </dd>
<dt>crop_geometry</dt>
<dd>A crop geometry string.  This geometry defines a subregion of the image to crop. </dd>

<dd> </dd>
<dt>image_geometry</dt>
<dd>An image geometry string.  This geometry defines the final size of the image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="TransformImages">TransformImages</a></h2>

<p>TransformImages() calls TransformImage() on each image of a sequence.</p>

<p>The format of the TransformImage method is:</p>

<pre class="text">
MagickBooleanType TransformImages(Image **image,
  const char *crop_geometry,const char *image_geometry,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image The transformed image is returned as this parameter. </dd>

<dd> </dd>
<dt>crop_geometry</dt>
<dd>A crop geometry string.  This geometry defines a subregion of the image to crop. </dd>

<dd> </dd>
<dt>image_geometry</dt>
<dd>An image geometry string.  This geometry defines the final size of the image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="TransposeImage">TransposeImage</a></h2>

<p>TransposeImage() creates a horizontal mirror image by reflecting the pixels around the central y-axis while rotating them by 90 degrees.</p>

<p>The format of the TransposeImage method is:</p>

<pre class="text">
Image *TransposeImage(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="TransverseImage">TransverseImage</a></h2>

<p>TransverseImage() creates a vertical mirror image by reflecting the pixels around the central x-axis while rotating them by 270 degrees.</p>

<p>The format of the TransverseImage method is:</p>

<pre class="text">
Image *TransverseImage(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/transform_8c.html" id="TrimImage">TrimImage</a></h2>

<p>TrimImage() trims pixels from the image edges.  It allocates the memory necessary for the new Image structure and returns a pointer to the new image.</p>

<p>The format of the TrimImage method is:</p>

<pre class="text">
Image *TrimImage(const Image *image,ExceptionInfo *exception)
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
    <p><a href="transform.php#">Back to top</a> •
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

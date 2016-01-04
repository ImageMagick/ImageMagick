



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Dealing with Image Layers</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, dealing, with, image, layers, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="layer.php#CoalesceImages">CoalesceImages</a> &bull; <a href="layer.php#DisposeImages">DisposeImages</a> &bull; <a href="layer.php#CompareImagesLayers">CompareImagesLayers</a> &bull; <a href="layer.php#OptimizeImageLayers">OptimizeImageLayers</a> &bull; <a href="layer.php#OptimizeImagePlusLayers">OptimizeImagePlusLayers</a> &bull; <a href="layer.php#OptimizeImageTransparency">OptimizeImageTransparency</a> &bull; <a href="layer.php#RemoveDuplicateLayers">RemoveDuplicateLayers</a> &bull; <a href="layer.php#RemoveZeroDelayLayers">RemoveZeroDelayLayers</a> &bull; <a href="layer.php#CompositeLayers">CompositeLayers</a> &bull; <a href="layer.php#MergeImageLayers">MergeImageLayers</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/layer_8c.html" id="CoalesceImages">CoalesceImages</a></h2>

<p>CoalesceImages() composites a set of images while respecting any page offsets and disposal methods.  GIF, MIFF, and MNG animation sequences typically start with an image background and each subsequent image varies in size and offset.  A new image sequence is returned with all images the same size as the first images virtual canvas and composited with the next image in the sequence.</p>

<p>The format of the CoalesceImages method is:</p>

<pre class="text">
Image *CoalesceImages(Image *image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image sequence. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/layer_8c.html" id="DisposeImages">DisposeImages</a></h2>

<p>DisposeImages() returns the coalesced frames of a GIF animation as it would appear after the GIF dispose method of that frame has been applied.  That is it returned the appearance of each frame before the next is overlaid.</p>

<p>The format of the DisposeImages method is:</p>

<pre class="text">
Image *DisposeImages(Image *image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>images</dt>
<dd>the image sequence. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/layer_8c.html" id="CompareImagesLayers">CompareImagesLayers</a></h2>

<p>CompareImagesLayers() compares each image with the next in a sequence and returns the minimum bounding region of all the pixel differences (of the LayerMethod specified) it discovers.</p>

<p>Images do NOT have to be the same size, though it is best that all the images are 'coalesced' (images are all the same size, on a flattened canvas, so as to represent exactly how an specific frame should look).</p>

<p>No GIF dispose methods are applied, so GIF animations must be coalesced before applying this image operator to find differences to them.</p>

<p>The format of the CompareImagesLayers method is:</p>

<pre class="text">
Image *CompareImagesLayers(const Image *images,
  const LayerMethod method,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the layers type to compare images with. Must be one of... CompareAnyLayer, CompareClearLayer, CompareOverlayLayer. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/layer_8c.html" id="OptimizeImageLayers">OptimizeImageLayers</a></h2>

<p>OptimizeImageLayers() compares each image the GIF disposed forms of the previous image in the sequence.  From this it attempts to select the smallest cropped image to replace each frame, while preserving the results of the GIF animation.</p>

<p>The format of the OptimizeImageLayers method is:</p>

<pre class="text">
Image *OptimizeImageLayers(const Image *image,
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/layer_8c.html" id="OptimizeImagePlusLayers">OptimizeImagePlusLayers</a></h2>

<p>OptimizeImagePlusLayers() is exactly as OptimizeImageLayers(), but may also add or even remove extra frames in the animation, if it improves the total number of pixels in the resulting GIF animation.</p>

<p>The format of the OptimizePlusImageLayers method is:</p>

<pre class="text">
Image *OptimizePlusImageLayers(const Image *image,
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/layer_8c.html" id="OptimizeImageTransparency">OptimizeImageTransparency</a></h2>

<p>OptimizeImageTransparency() takes a frame optimized GIF animation, and compares the overlayed pixels against the disposal image resulting from all the previous frames in the animation.  Any pixel that does not change the disposal image (and thus does not effect the outcome of an overlay) is made transparent.</p>

<p>WARNING: This modifies the current images directly, rather than generate a new image sequence.</p>

<p>The format of the OptimizeImageTransperency method is:</p>

<pre class="text">
void OptimizeImageTransperency(Image *image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image sequence </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/layer_8c.html" id="RemoveDuplicateLayers">RemoveDuplicateLayers</a></h2>

<p>RemoveDuplicateLayers() removes any image that is exactly the same as the next image in the given image list.  Image size and virtual canvas offset must also match, though not the virtual canvas size itself.</p>

<p>No check is made with regards to image disposal setting, though it is the dispose setting of later image that is kept.  Also any time delays are also added together. As such coalesced image animations should still produce the same result, though with duplicte frames merged into a single frame.</p>

<p>The format of the RemoveDuplicateLayers method is:</p>

<pre class="text">
void RemoveDuplicateLayers(Image **image, ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>images</dt>
<dd>the image list </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/layer_8c.html" id="RemoveZeroDelayLayers">RemoveZeroDelayLayers</a></h2>

<p>RemoveZeroDelayLayers() removes any image that as a zero delay time. Such images generally represent intermediate or partial updates in GIF animations used for file optimization.  They are not ment to be displayed to users of the animation.  Viewable images in an animation should have a time delay of 3 or more centi-seconds (hundredths of a second).</p>

<p>However if all the frames have a zero time delay, then either the animation is as yet incomplete, or it is not a GIF animation.  This a non-sensible situation, so no image will be removed and a 'Zero Time Animation' warning (exception) given.</p>

<p>No warning will be given if no image was removed because all images had an appropriate non-zero time delay set.</p>

<p>Due to the special requirements of GIF disposal handling, GIF animations should be coalesced first, before calling this function, though that is not a requirement.</p>

<p>The format of the RemoveZeroDelayLayers method is:</p>

<pre class="text">
void RemoveZeroDelayLayers(Image **image, ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>images</dt>
<dd>the image list </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/layer_8c.html" id="CompositeLayers">CompositeLayers</a></h2>

<p>CompositeLayers() compose the source image sequence over the destination image sequence, starting with the current image in both lists.</p>

<p>Each layer from the two image lists are composted together until the end of one of the image lists is reached.  The offset of each composition is also adjusted to match the virtual canvas offsets of each layer. As such the given offset is relative to the virtual canvas, and not the actual image.</p>

<p>Composition uses given x and y offsets, as the 'origin' location of the source images virtual canvas (not the real image) allowing you to compose a list of 'layer images' into the destiantioni images.  This makes it well sutiable for directly composing 'Clears Frame Animations' or 'Coaleased Animations' onto a static or other 'Coaleased Animation' destination image list.  GIF disposal handling is not looked at.</p>

<p>Special case:- If one of the image sequences is the last image (just a single image remaining), that image is repeatally composed with all the images in the other image list.  Either the source or destination lists may be the single image, for this situation.</p>

<p>In the case of a single destination image (or last image given), that image will ve cloned to match the number of images remaining in the source image list.</p>

<p>This is equivelent to the "-layer Composite" Shell API operator.</p>


<p>The format of the CompositeLayers method is:</p>

<pre class="text">
void CompositeLayers(Image *destination, const CompositeOperator
compose, Image *source, const ssize_t x_offset, const ssize_t y_offset,
ExceptionInfo *exception);
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>destination</dt>
<dd>the destination images and results </dd>

<dd> </dd>
<dt>source</dt>
<dd>source image(s) for the layer composition </dd>

<dd> </dd>
<dt>compose, x_offset, y_offset</dt>
<dd> arguments passed on to CompositeImages() </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/layer_8c.html" id="MergeImageLayers">MergeImageLayers</a></h2>

<p>MergeImageLayers() composes all the image layers from the current given image onward to produce a single image of the merged layers.</p>

<p>The inital canvas's size depends on the given LayerMethod, and is initialized using the first images background color.  The images are then compositied onto that image in sequence using the given composition that has been assigned to each individual image.</p>

<p>The format of the MergeImageLayers is:</p>

<pre class="text">
Image *MergeImageLayers(const Image *image,
  const LayerMethod method, ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image list to be composited together </dd>

<dd> </dd>
<dt>method</dt>
<dd>the method of selecting the size of the initial canvas. </dd>

<dd> MergeLayer: Merge all layers onto a canvas just large enough to hold all the actual images. The virtual canvas of the first image is preserved but otherwise ignored. </dd>

<dd> FlattenLayer: Use the virtual canvas size of first image. Images which fall outside this canvas is clipped. This can be used to 'fill out' a given virtual canvas. </dd>

<dd> MosaicLayer: Start with the virtual canvas of the first image, enlarging left and right edges to contain all images. Images with negative offsets will be clipped. </dd>

<dd> TrimBoundsLayer: Determine the overall bounds of all the image layers just as in "MergeLayer", then adjust the the canvas and offsets to be relative to those bounds, without overlaying the images. </dd>

<dd> WARNING: a new image is not returned, the original image sequence page data is modified instead. </dd>

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
    <p><a href="layer.php#">Back to top</a> •
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

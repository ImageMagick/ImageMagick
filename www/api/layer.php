



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Dealing with Image Layers @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, dealing, with, image, layers, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/layer.php" rel="canonical">
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
<p class="text-center"><a href="layer.php#CoalesceImages">CoalesceImages</a> &bull; <a href="layer.php#DisposeImages">DisposeImages</a> &bull; <a href="layer.php#CompareImagesLayers">CompareImagesLayers</a> &bull; <a href="layer.php#OptimizeImageLayers">OptimizeImageLayers</a> &bull; <a href="layer.php#OptimizeImagePlusLayers">OptimizeImagePlusLayers</a> &bull; <a href="layer.php#OptimizeImageTransparency">OptimizeImageTransparency</a> &bull; <a href="layer.php#RemoveDuplicateLayers">RemoveDuplicateLayers</a> &bull; <a href="layer.php#RemoveZeroDelayLayers">RemoveZeroDelayLayers</a> &bull; <a href="layer.php#CompositeLayers">CompositeLayers</a> &bull; <a href="layer.php#MergeImageLayers">MergeImageLayers</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/layer_8c.html" id="CoalesceImages">CoalesceImages</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/layer_8c.html" id="DisposeImages">DisposeImages</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/layer_8c.html" id="CompareImagesLayers">CompareImagesLayers</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/layer_8c.html" id="OptimizeImageLayers">OptimizeImageLayers</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/layer_8c.html" id="OptimizeImagePlusLayers">OptimizeImagePlusLayers</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/layer_8c.html" id="OptimizeImageTransparency">OptimizeImageTransparency</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/layer_8c.html" id="RemoveDuplicateLayers">RemoveDuplicateLayers</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/layer_8c.html" id="RemoveZeroDelayLayers">RemoveZeroDelayLayers</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/layer_8c.html" id="CompositeLayers">CompositeLayers</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/layer_8c.html" id="MergeImageLayers">MergeImageLayers</a></h2>

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
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="layer.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 3rd September 2018 14:09 -->
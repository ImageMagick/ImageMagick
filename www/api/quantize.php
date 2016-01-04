



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Reduce the Number of Unique Colors in an Image</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, reduce, the, number, of, unique, colors, in, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="quantize.php#QuantizeImage">QuantizeImage</a> &bull; <a href="quantize.php#AcquireQuantizeInfo">AcquireQuantizeInfo</a> &bull; <a href="quantize.php#CloneQuantizeInfo">CloneQuantizeInfo</a> &bull; <a href="quantize.php#CompressImageColormap">CompressImageColormap</a> &bull; <a href="quantize.php#DestroyQuantizeInfo">DestroyQuantizeInfo</a> &bull; <a href="quantize.php#GetImageQuantizeError">GetImageQuantizeError</a> &bull; <a href="quantize.php#GetQuantizeInfo">GetQuantizeInfo</a> &bull; <a href="quantize.php#PosterizeImage">PosterizeImage</a> &bull; <a href="quantize.php#QuantizeImage">QuantizeImage</a> &bull; <a href="quantize.php#QuantizeImages">QuantizeImages</a> &bull; <a href="quantize.php#RemapImage">RemapImage</a> &bull; <a href="quantize.php#RemapImages">RemapImages</a> &bull; <a href="quantize.php#SetGrayscaleImage">SetGrayscaleImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="QuantizeImage">QuantizeImage</a></h2>

<p>QuantizeImage() takes a standard RGB or monochrome images and quantizes them down to some fixed number of colors.</p>

<p>For purposes of color allocation, an image is a set of n pixels, where each pixel is a point in RGB space.  RGB space is a 3-dimensional vector space, and each pixel, Pi,  is defined by an ordered triple of red, green, and blue coordinates, (Ri, Gi, Bi).</p>

<p>Each primary color component (red, green, or blue) represents an intensity which varies linearly from 0 to a maximum value, Cmax, which corresponds to full saturation of that color.  Color allocation is defined over a domain consisting of the cube in RGB space with opposite vertices at (0,0,0) and (Cmax, Cmax, Cmax).  QUANTIZE requires Cmax = 255.</p>

<p>The algorithm maps this domain onto a tree in which each node represents a cube within that domain.  In the following discussion these cubes are defined by the coordinate of two opposite vertices (vertex nearest the origin in RGB space and the vertex farthest from the origin).</p>

<p>The tree's root node represents the entire domain, (0,0,0) through (Cmax,Cmax,Cmax).  Each lower level in the tree is generated by subdividing one node's cube into eight smaller cubes of equal size. This corresponds to bisecting the parent cube with planes passing through the midpoints of each edge.</p>

<p>The basic algorithm operates in three phases: Classification, Reduction, and Assignment.  Classification builds a color description tree for the image.  Reduction collapses the tree until the number it represents, at most, the number of colors desired in the output image. Assignment defines the output image's color map and sets each pixel's color by restorage_class in the reduced tree.  Our goal is to minimize the numerical discrepancies between the original colors and quantized colors (quantization error).</p>

<p>Classification begins by initializing a color description tree of sufficient depth to represent each possible input color in a leaf. However, it is impractical to generate a fully-formed color description tree in the storage_class phase for realistic values of Cmax.  If colors components in the input image are quantized to k-bit precision, so that Cmax= 2k-1, the tree would need k levels below the root node to allow representing each possible input color in a leaf.  This becomes prohibitive because the tree's total number of nodes is 1 + sum(i=1, k, 8k).</p>

<p>A complete tree would require 19,173,961 nodes for k = 8, Cmax = 255.</p>
<dt>avoid building a fully populated tree, QUANTIZE</dt>
<p>(1) Initializes data structures for nodes only as they are needed;  (2) Chooses a maximum depth for the tree as a function of the desired number of colors in the output image (currently log2(colormap size)).</p>

<p>For each pixel in the input image, storage_class scans downward from the root of the color description tree.  At each level of the tree it identifies the single node which represents a cube in RGB space containing the pixel's color.  It updates the following data for each such node:</p>

<pre class="text">
    n1: Number of pixels whose color is contained in the RGB cube which
    this node represents;
</pre>

<p>n2: Number of pixels whose color is not represented in a node at lower depth in the tree;  initially,  n2 = 0 for all nodes except leaves of the tree.</p>

<p>Sr, Sg, Sb: Sums of the red, green, and blue component values for all pixels not classified at a lower depth. The combination of these sums and n2 will ultimately characterize the mean color of a set of pixels represented by this node.</p>

<p>E: the distance squared in RGB space between each pixel contained within a node and the nodes' center.  This represents the quantization error for a node.</p>

<p>Reduction repeatedly prunes the tree until the number of nodes with n2 &gt; 0 is less than or equal to the maximum number of colors allowed in the output image.  On any given iteration over the tree, it selects those nodes whose E count is minimal for pruning and merges their color statistics upward. It uses a pruning threshold, Ep, to govern node selection as follows:</p>

<dd>
</dd>

<dd> Ep = 0 while number of nodes with (n2 &gt; 0) &gt; required maximum number of colors prune all nodes such that E &lt;= Ep Set Ep to minimum E in remaining nodes </dd>

<dd> This has the effect of minimizing any quantization error when merging two nodes together. </dd>

<dd> When a node to be pruned has offspring, the pruning procedure invokes itself recursively in order to prune the tree from the leaves upward. n2,  Sr, Sg,  and  Sb in a node being pruned are always added to the corresponding data in that node's parent.  This retains the pruned node's color characteristics for later averaging. </dd>

<dd> For each node, n2 pixels exist for which that node represents the smallest volume in RGB space containing those pixel's colors.  When n2 &gt; 0 the node will uniquely define a color in the output image. At the beginning of reduction,  n2 = 0  for all nodes except a the leaves of the tree which represent colors present in the input image. </dd>

<dd> The other pixel count, n1, indicates the total number of colors within the cubic volume which the node represents.  This includes n1 - n2 pixels whose colors should be defined by nodes at a lower level in the tree. </dd>

<dd> Assignment generates the output image from the pruned tree.  The output </dd>
<dl class="dl-horizontal">
<dt>parts</dt>
<dd>(1)  A color map, which is an array of color descriptions (RGB triples) for each color present in the output image;  (2)  A pixel array, which represents each pixel as an index into the color map array. </dd>

<dd> First, the assignment phase makes one pass over the pruned color description tree to establish the image's color map.  For each node with n2  &gt; 0, it divides Sr, Sg, and Sb by n2 .  This produces the mean color of all pixels that classify no lower than this node.  Each of these colors becomes an entry in the color map. </dd>

<dd> Finally,  the assignment phase reclassifies each pixel in the pruned tree to identify the deepest node containing the pixel's color.  The pixel's value in the pixel array becomes the index of this node's mean color in the color map. </dd>

<dd> This method is based on a similar algorithm written by Paul Raveling. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="AcquireQuantizeInfo">AcquireQuantizeInfo</a></h2>

<p>AcquireQuantizeInfo() allocates the QuantizeInfo structure.</p>

<p>The format of the AcquireQuantizeInfo method is:</p>

<pre class="text">
QuantizeInfo *AcquireQuantizeInfo(const ImageInfo *image_info)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="CloneQuantizeInfo">CloneQuantizeInfo</a></h2>

<p>CloneQuantizeInfo() makes a duplicate of the given quantize info structure, or if quantize info is NULL, a new one.</p>

<p>The format of the CloneQuantizeInfo method is:</p>

<pre class="text">
QuantizeInfo *CloneQuantizeInfo(const QuantizeInfo *quantize_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>clone_info</dt>
<dd>Method CloneQuantizeInfo returns a duplicate of the given quantize info, or if image info is NULL a new one. </dd>

<dd> </dd>
<dt>quantize_info</dt>
<dd>a structure of type info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="CompressImageColormap">CompressImageColormap</a></h2>

<p>CompressImageColormap() compresses an image colormap by removing any duplicate or unused color entries.</p>

<p>The format of the CompressImageColormap method is:</p>

<pre class="text">
MagickBooleanType CompressImageColormap(Image *image,
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="DestroyQuantizeInfo">DestroyQuantizeInfo</a></h2>

<p>DestroyQuantizeInfo() deallocates memory associated with an QuantizeInfo structure.</p>

<p>The format of the DestroyQuantizeInfo method is:</p>

<pre class="text">
QuantizeInfo *DestroyQuantizeInfo(QuantizeInfo *quantize_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>quantize_info</dt>
<dd>Specifies a pointer to an QuantizeInfo structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="GetImageQuantizeError">GetImageQuantizeError</a></h2>

<p>GetImageQuantizeError() measures the difference between the original and quantized images.  This difference is the total quantization error. The error is computed by summing over all pixels in an image the distance squared in RGB space between each reference pixel value and its quantized value.  These values are computed:</p>

<pre class="text">
    o mean_error_per_pixel:  This value is the mean error for any single
pixel in the image.
</pre>

<dt>normalized_mean_square_error</dt>
<p>This value is the normalized mean quantization error for any single pixel in the image.  This distance measure is normalized to a range between 0 and 1.  It is independent of the range of red, green, and blue values in the image.</p>

<dt>normalized_maximum_square_error</dt>
<p>Thsi value is the normalized maximum quantization error for any single pixel in the image.  This distance measure is normalized to a range between 0 and 1.  It is independent of the range of red, green, and blue values in your image.</p>

<p>The format of the GetImageQuantizeError method is:</p>

<pre class="text">
MagickBooleanType GetImageQuantizeError(Image *image,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="GetQuantizeInfo">GetQuantizeInfo</a></h2>

<p>GetQuantizeInfo() initializes the QuantizeInfo structure.</p>

<p>The format of the GetQuantizeInfo method is:</p>

<pre class="text">
GetQuantizeInfo(QuantizeInfo *quantize_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>quantize_info</dt>
<dd>Specifies a pointer to a QuantizeInfo structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="PosterizeImage">PosterizeImage</a></h2>

<p>PosterizeImage() reduces the image to a limited number of colors for a "poster" effect.</p>

<p>The format of the PosterizeImage method is:</p>

<pre class="text">
MagickBooleanType PosterizeImage(Image *image,const size_t levels,
  const DitherMethod dither_method,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>Specifies a pointer to an Image structure. </dd>

<dd> </dd>
<dt>levels</dt>
<dd>Number of color levels allowed in each channel.  Very low values (2, 3, or 4) have the most visible effect. </dd>

<dd> </dd>
<dt>dither_method</dt>
<dd>choose from UndefinedDitherMethod, NoDitherMethod, RiemersmaDitherMethod, FloydSteinbergDitherMethod. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="QuantizeImage">QuantizeImage</a></h2>

<p>QuantizeImage() analyzes the colors within a reference image and chooses a fixed number of colors to represent the image.  The goal of the algorithm is to minimize the color difference between the input and output image while minimizing the processing time.</p>

<p>The format of the QuantizeImage method is:</p>

<pre class="text">
MagickBooleanType QuantizeImage(const QuantizeInfo *quantize_info,
  Image *image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>quantize_info</dt>
<dd>Specifies a pointer to an QuantizeInfo structure. </dd>

<dd> </dd>
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="QuantizeImages">QuantizeImages</a></h2>

<p>QuantizeImages() analyzes the colors within a set of reference images and chooses a fixed number of colors to represent the set.  The goal of the algorithm is to minimize the color difference between the input and output images while minimizing the processing time.</p>

<p>The format of the QuantizeImages method is:</p>

<pre class="text">
MagickBooleanType QuantizeImages(const QuantizeInfo *quantize_info,
  Image *images,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>quantize_info</dt>
<dd>Specifies a pointer to an QuantizeInfo structure. </dd>

<dd> </dd>
<dt>images</dt>
<dd>Specifies a pointer to a list of Image structures. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="RemapImage">RemapImage</a></h2>

<p>RemapImage() replaces the colors of an image with the closest of the colors from the reference image.</p>

<p>The format of the RemapImage method is:</p>

<pre class="text">
MagickBooleanType RemapImage(const QuantizeInfo *quantize_info,
  Image *image,const Image *remap_image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>quantize_info</dt>
<dd>Specifies a pointer to an QuantizeInfo structure. </dd>

<dd> </dd>
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>remap_image</dt>
<dd>the reference image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="RemapImages">RemapImages</a></h2>

<p>RemapImages() replaces the colors of a sequence of images with the closest color from a reference image.</p>

<p>The format of the RemapImage method is:</p>

<pre class="text">
MagickBooleanType RemapImages(const QuantizeInfo *quantize_info,
  Image *images,Image *remap_image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>quantize_info</dt>
<dd>Specifies a pointer to an QuantizeInfo structure. </dd>

<dd> </dd>
<dt>images</dt>
<dd>the image sequence. </dd>

<dd> </dd>
<dt>remap_image</dt>
<dd>the reference image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/quantize_8c.html" id="SetGrayscaleImage">SetGrayscaleImage</a></h2>

<p>SetGrayscaleImage() converts an image to a PseudoClass grayscale image.</p>

<p>The format of the SetGrayscaleImage method is:</p>

<pre class="text">
MagickBooleanType SetGrayscaleImage(Image *image,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>The image. </dd>

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
    <p><a href="quantize.php#">Back to top</a> •
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





<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Shear or Rotate an Image by an Arbitrary Angle</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, shear, or, rotate, an, image, by, an, arbitrary, angle, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="shear.php#The XShearImage">The XShearImage</a> &bull; <a href="shear.php#DeskewImage">DeskewImage</a> &bull; <a href="shear.php#IntegralRotateImage">IntegralRotateImage</a> &bull; <a href="shear.php#ShearImage">ShearImage</a> &bull; <a href="shear.php#ShearRotateImage">ShearRotateImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/shear_8c.html" id="The_XShearImage">The XShearImage</a></h2>

<p>The XShearImage() and YShearImage() methods are based on the paper "A Fast Algorithm for General Raster Rotatation" by Alan W. Paeth, Graphics Interface '86 (Vancouver).  ShearRotateImage() is adapted from a similar method based on the Paeth paper written by Michael Halle of the Spatial Imaging Group, MIT Media Lab.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/shear_8c.html" id="DeskewImage">DeskewImage</a></h2>

<p>DeskewImage() removes skew from the image.  Skew is an artifact that occurs in scanned images because of the camera being misaligned, imperfections in the scanning or surface, or simply because the paper was not placed completely flat when scanned.</p>

<p>The result will be auto-croped if the artifact "deskew:auto-crop" is defined, while the amount the image is to be deskewed, in degrees is also saved as the artifact "deskew:angle".</p>

<p>If the artifact "deskew:auto-crop" is given the image will be automatically cropped of the excess background.  The value is the border width of all pixels around the edge that will be used to determine an average border color for the automatic trim.</p>

<p>The format of the DeskewImage method is:</p>

<pre class="text">
Image *DeskewImage(const Image *image,const double threshold,
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
<dt>threshold</dt>
<dd>separate background from foreground. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/shear_8c.html" id="IntegralRotateImage">IntegralRotateImage</a></h2>

<p>IntegralRotateImage() rotates the image an integral of 90 degrees.  It allocates the memory necessary for the new Image structure and returns a pointer to the rotated image.</p>

<p>The format of the IntegralRotateImage method is:</p>

<pre class="text">
Image *IntegralRotateImage(const Image *image,size_t rotations,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>rotations</dt>
<p>Specifies the number of 90 degree rotations.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/shear_8c.html" id="ShearImage">ShearImage</a></h2>

<p>ShearImage() creates a new image that is a shear_image copy of an existing one.  Shearing slides one edge of an image along the X or Y axis, creating a parallelogram.  An X direction shear slides an edge along the X axis, while a Y direction shear slides an edge along the Y axis.  The amount of the shear is controlled by a shear angle.  For X direction shears, x_shear is measured relative to the Y axis, and similarly, for Y direction shears y_shear is measured relative to the X axis.  Empty triangles left over from shearing the image are filled with the background color defined by member 'background_color' of the image..  ShearImage() allocates the memory necessary for the new Image structure and returns a pointer to the new image.</p>

<p>ShearImage() is based on the paper "A Fast Algorithm for General Raster Rotatation" by Alan W. Paeth.</p>

<p>The format of the ShearImage method is:</p>

<pre class="text">
Image *ShearImage(const Image *image,const double x_shear,
  const double y_shear,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>x_shear, y_shear</dt>
<p>Specifies the number of degrees to shear the image.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/shear_8c.html" id="ShearRotateImage">ShearRotateImage</a></h2>

<p>ShearRotateImage() creates a new image that is a rotated copy of an existing one.  Positive angles rotate counter-clockwise (right-hand rule), while negative angles rotate clockwise.  Rotated images are usually larger than the originals and have 'empty' triangular corners.  X axis.  Empty triangles left over from shearing the image are filled with the background color defined by member 'background_color' of the image.  ShearRotateImage allocates the memory necessary for the new Image structure and returns a pointer to the new image.</p>

<p>ShearRotateImage() is based on the paper "A Fast Algorithm for General Raster Rotatation" by Alan W. Paeth.  ShearRotateImage is adapted from a similar method based on the Paeth paper written by Michael Halle of the Spatial Imaging Group, MIT Media Lab.</p>

<p>The format of the ShearRotateImage method is:</p>

<pre class="text">
Image *ShearRotateImage(const Image *image,const double degrees,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>degrees</dt>
<p>Specifies the number of degrees to rotate the image.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="shear.php#">Back to top</a> •
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

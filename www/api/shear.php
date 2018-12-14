



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Shear or Rotate an Image by an Arbitrary Angle @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, shear, or, rotate, an, image, by, an, arbitrary, angle, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/shear.php" rel="canonical">
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
<p class="text-center"><a href="shear.php#The XShearImage">The XShearImage</a> &bull; <a href="shear.php#DeskewImage">DeskewImage</a> &bull; <a href="shear.php#IntegralRotateImage">IntegralRotateImage</a> &bull; <a href="shear.php#ShearImage">ShearImage</a> &bull; <a href="shear.php#ShearRotateImage">ShearRotateImage</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/shear_8c.html" id="The_XShearImage">The XShearImage</a></h2>

<p>The XShearImage() and YShearImage() methods are based on the paper "A Fast Algorithm for General Raster Rotation" by Alan W. Paeth, Graphics Interface '86 (Vancouver).  ShearRotateImage() is adapted from a similar method based on the Paeth paper written by Michael Halle of the Spatial Imaging Group, MIT Media Lab.</p>

<h2><a href="https://imagemagick.org/api/MagickCore/shear_8c.html" id="DeskewImage">DeskewImage</a></h2>

<p>DeskewImage() removes skew from the image.  Skew is an artifact that occurs in scanned images because of the camera being misaligned, imperfections in the scanning or surface, or simply because the paper was not placed completely flat when scanned.</p>

<p>The result will be auto-croped if the artifact "deskew:auto-crop" is defined, while the amount the image is to be deskewed, in degrees is also saved as the artifact "deskew:angle".</p>

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
<h2><a href="https://imagemagick.org/api/MagickCore/shear_8c.html" id="IntegralRotateImage">IntegralRotateImage</a></h2>

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

<h2><a href="https://imagemagick.org/api/MagickCore/shear_8c.html" id="ShearImage">ShearImage</a></h2>

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

<h2><a href="https://imagemagick.org/api/MagickCore/shear_8c.html" id="ShearRotateImage">ShearRotateImage</a></h2>

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
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="shear.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 6th September 2018 19:13 -->
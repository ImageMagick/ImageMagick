



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Image Distortions</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, image, distortions, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="distort.php#AffineTransformImage">AffineTransformImage</a> &bull; <a href="distort.php#DistortImage">DistortImage</a> &bull; <a href="distort.php#RotateImage">RotateImage</a> &bull; <a href="distort.php#SparseColorImage">SparseColorImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/distort_8c.html" id="AffineTransformImage">AffineTransformImage</a></h2>

<p>AffineTransformImage() transforms an image as dictated by the affine matrix. It allocates the memory necessary for the new Image structure and returns a pointer to the new image.</p>

<p>The format of the AffineTransformImage method is:</p>

<pre class="text">
Image *AffineTransformImage(const Image *image,
  AffineMatrix *affine_matrix,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>affine_matrix</dt>
<dd>the affine matrix. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/distort_8c.html" id="DistortImage">DistortImage</a></h2>

<p>DistortImage() distorts an image using various distortion methods, by mapping color lookups of the source image to a new destination image usally of the same size as the source image, unless 'bestfit' is set to true.</p>

<p>If 'bestfit' is enabled, and distortion allows it, the destination image is adjusted to ensure the whole source 'image' will just fit within the final destination image, which will be sized and offset accordingly.  Also in many cases the virtual offset of the source image will be taken into account in the mapping.</p>

<p>If the '-verbose' control option has been set print to standard error the equicelent '-fx' formula with coefficients for the function, if practical.</p>

<p>The format of the DistortImage() method is:</p>

<pre class="text">
Image *DistortImage(const Image *image,const DistortImageMethod method,
  const size_t number_arguments,const double *arguments,
  MagickBooleanType bestfit, ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image to be distorted. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the method of image distortion. </dd>

<dd> ArcDistortion always ignores source image offset, and always 'bestfit' the destination image with the top left corner offset relative to the polar mapping center. </dd>

<dd> Affine, Perspective, and Bilinear, do least squares fitting of the distrotion when more than the minimum number of control point pairs are provided. </dd>

<dd> Perspective, and Bilinear, fall back to a Affine distortion when less than 4 control point pairs are provided.  While Affine distortions let you use any number of control point pairs, that is Zero pairs is a No-Op (viewport only) distortion, one pair is a translation and two pairs of control points do a scale-rotate-translate, without any shearing. </dd>

<dd> </dd>
<dt>number_arguments</dt>
<dd>the number of arguments given. </dd>

<dd> </dd>
<dt>arguments</dt>
<dd>an array of floating point arguments for this method. </dd>

<dd> </dd>
<dt>bestfit</dt>
<dd>Attempt to 'bestfit' the size of the resulting image. This also forces the resulting image to be a 'layered' virtual canvas image.  Can be overridden using 'distort:viewport' setting. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure </dd>

<dd> Extra Controls from Image meta-data (artifacts)... </dd>

<dd> o "verbose" Output to stderr alternatives, internal coefficents, and FX equivalents for the distortion operation (if feasible). This forms an extra check of the distortion method, and allows users access to the internal constants IM calculates for the distortion. </dd>

<dd> o "distort:viewport" Directly set the output image canvas area and offest to use for the resulting image, rather than use the original images canvas, or a calculated 'bestfit' canvas. </dd>

<dd> o "distort:scale" Scale the size of the output canvas by this amount to provide a method of Zooming, and for super-sampling the results. </dd>

<dd> Other settings that can effect results include </dd>

<dd> o 'interpolate' For source image lookups (scale enlargements) </dd>

<dd> o 'filter'      Set filter to use for area-resampling (scale shrinking). Set to 'point' to turn off and use 'interpolate' lookup instead </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/distort_8c.html" id="RotateImage">RotateImage</a></h2>

<p>RotateImage() creates a new image that is a rotated copy of an existing one.  Positive angles rotate counter-clockwise (right-hand rule), while negative angles rotate clockwise.  Rotated images are usually larger than the originals and have 'empty' triangular corners.  X axis.  Empty triangles left over from shearing the image are filled with the background color defined by member 'background_color' of the image.  RotateImage allocates the memory necessary for the new Image structure and returns a pointer to the new image.</p>

<p>The format of the RotateImage method is:</p>

<pre class="text">
Image *RotateImage(const Image *image,const double degrees,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image.</p>

<dt>degrees</dt>
<p>Specifies the number of degrees to rotate the image.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/distort_8c.html" id="SparseColorImage">SparseColorImage</a></h2>

<p>SparseColorImage(), given a set of coordinates, interpolates the colors found at those coordinates, across the whole image, using various methods.</p>

<p>The format of the SparseColorImage() method is:</p>

<pre class="text">
Image *SparseColorImage(const Image *image,
  const SparseColorMethod method,const size_t number_arguments,
  const double *arguments,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image to be filled in. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the method to fill in the gradient between the control points. </dd>

<dd> The methods used for SparseColor() are often simular to methods used for DistortImage(), and even share the same code for determination of the function coefficents, though with more dimensions (or resulting values). </dd>

<dd> </dd>
<dt>number_arguments</dt>
<dd>the number of arguments given. </dd>

<dd> </dd>
<dt>arguments</dt>
<dd>array of floating point arguments for this method-- x,y,color_values-- with color_values given as normalized values. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="distort.php#">Back to top</a> •
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

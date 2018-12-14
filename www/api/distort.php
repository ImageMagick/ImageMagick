



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Image Distortions @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, image, distortions, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/distort.php" rel="canonical">
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
<p class="text-center"><a href="distort.php#AffineTransformImage">AffineTransformImage</a> &bull; <a href="distort.php#DistortImage">DistortImage</a> &bull; <a href="distort.php#RotateImage">RotateImage</a> &bull; <a href="distort.php#SparseColorImage">SparseColorImage</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/distort_8c.html" id="AffineTransformImage">AffineTransformImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/distort_8c.html" id="DistortImage">DistortImage</a></h2>

<p>DistortImage() distorts an image using various distortion methods, by mapping color lookups of the source image to a new destination image usally of the same size as the source image, unless 'bestfit' is set to true.</p>

<p>If 'bestfit' is enabled, and distortion allows it, the destination image is adjusted to ensure the whole source 'image' will just fit within the final destination image, which will be sized and offset accordingly.  Also in many cases the virtual offset of the source image will be taken into account in the mapping.</p>

<p>If the '-verbose' control option has been set print to standard error the equicelent '-fx' formula with coefficients for the function, if practical.</p>

<p>The format of the DistortImage() method is:</p>

<pre class="text">
Image *DistortImage(const Image *image,const DistortMethod method,
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
<h2><a href="https://imagemagick.org/api/MagickCore/distort_8c.html" id="RotateImage">RotateImage</a></h2>

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

<h2><a href="https://imagemagick.org/api/MagickCore/distort_8c.html" id="SparseColorImage">SparseColorImage</a></h2>

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
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="distort.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 8th September 2018 09:08 -->
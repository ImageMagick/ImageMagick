



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Set Text Attributes</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, set, text, attributes, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="attribute.php#GetImageDepth">GetImageDepth</a> &bull; <a href="attribute.php#GetImageQuantumDepth">GetImageQuantumDepth</a> &bull; <a href="attribute.php#GetImageType">GetImageType</a> &bull; <a href="attribute.php#IdentifyImageGray">IdentifyImageGray</a> &bull; <a href="attribute.php#IdentifyImageMonochrome">IdentifyImageMonochrome</a> &bull; <a href="attribute.php#IdentifyImageType">IdentifyImageType</a> &bull; <a href="attribute.php#IsImageGray">IsImageGray</a> &bull; <a href="attribute.php#IsImageMonochrome">IsImageMonochrome</a> &bull; <a href="attribute.php#IsImageOpaque">IsImageOpaque</a> &bull; <a href="attribute.php#SetImageDepth">SetImageDepth</a> &bull; <a href="attribute.php#SetImageType">SetImageType</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="GetImageDepth">GetImageDepth</a></h2>

<p>GetImageDepth() returns the depth of a particular image channel.</p>

<p>The format of the GetImageDepth method is:</p>

<pre class="text">
size_t GetImageDepth(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="GetImageQuantumDepth">GetImageQuantumDepth</a></h2>

<p>GetImageQuantumDepth() returns the depth of the image rounded to a legal quantum depth: 8, 16, or 32.</p>

<p>The format of the GetImageQuantumDepth method is:</p>

<pre class="text">
size_t GetImageQuantumDepth(const Image *image,
  const MagickBooleanType constrain)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>constrain</dt>
<dd>A value other than MagickFalse, constrains the depth to a maximum of MAGICKCORE_QUANTUM_DEPTH. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="GetImageType">GetImageType</a></h2>

<p>GetImageType() returns the type of image:</p>

<p>Bilevel         Grayscale        GrayscaleMatte Palette         PaletteMatte     TrueColor TrueColorMatte  ColorSeparation  ColorSeparationMatte</p>

<p>The format of the GetImageType method is:</p>

<pre class="text">
ImageType GetImageType(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="IdentifyImageGray">IdentifyImageGray</a></h2>

<p>IdentifyImageGray() returns grayscale if all the pixels in the image have the same red, green, and blue intensities, and bi-level is the intensity is either 0 or QuantumRange. Otherwise undefined is returned.</p>

<p>The format of the IdentifyImageGray method is:</p>

<pre class="text">
ImageType IdentifyImageGray(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="IdentifyImageMonochrome">IdentifyImageMonochrome</a></h2>

<p>IdentifyImageMonochrome() returns MagickTrue if all the pixels in the image have the same red, green, and blue intensities and the intensity is either 0 or QuantumRange.</p>

<p>The format of the IdentifyImageMonochrome method is:</p>

<pre class="text">
MagickBooleanType IdentifyImageMonochrome(const Image *image,
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="IdentifyImageType">IdentifyImageType</a></h2>

<p>IdentifyImageType() returns the potential type of image:</p>

<p>Bilevel         Grayscale        GrayscaleMatte Palette         PaletteMatte     TrueColor TrueColorMatte  ColorSeparation  ColorSeparationMatte</p>

<p>To ensure the image type matches its potential, use SetImageType():</p>

<pre class="text">
    (void) SetImageType(image,IdentifyImageType(image,exception),exception);
</pre>

<p>The format of the IdentifyImageType method is:</p>

<pre class="text">
ImageType IdentifyImageType(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="IsImageGray">IsImageGray</a></h2>

<p>IsImageGray() returns MagickTrue if the type of the image is grayscale or bi-level.</p>

<p>The format of the IsImageGray method is:</p>

<pre class="text">
MagickBooleanType IsImageGray(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="IsImageMonochrome">IsImageMonochrome</a></h2>

<p>IsImageMonochrome() returns MagickTrue if type of the image is bi-level.</p>

<p>The format of the IsImageMonochrome method is:</p>

<pre class="text">
MagickBooleanType IsImageMonochrome(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="IsImageOpaque">IsImageOpaque</a></h2>

<p>IsImageOpaque() returns MagickTrue if none of the pixels in the image have an alpha value other than OpaqueAlpha (QuantumRange).</p>

<p>Will return true immediatally is alpha channel is not available.</p>

<p>The format of the IsImageOpaque method is:</p>

<pre class="text">
MagickBooleanType IsImageOpaque(const Image *image,
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="SetImageDepth">SetImageDepth</a></h2>

<p>SetImageDepth() sets the depth of the image.</p>

<p>The format of the SetImageDepth method is:</p>

<pre class="text">
MagickBooleanType SetImageDepth(Image *image,const size_t depth,
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
<dt>channel</dt>
<dd>the channel. </dd>

<dd> </dd>
<dt>depth</dt>
<dd>the image depth. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/attribute_8c.html" id="SetImageType">SetImageType</a></h2>

<p>SetImageType() sets the type of image.  Choose from these types:</p>

<p>Bilevel        Grayscale       GrayscaleMatte Palette        PaletteMatte    TrueColor TrueColorMatte ColorSeparation ColorSeparationMatte OptimizeType</p>

<p>The format of the SetImageType method is:</p>

<pre class="text">
MagickBooleanType SetImageType(Image *image,const ImageType type,
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
<dt>type</dt>
<dd>Image type. </dd>

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
    <p><a href="attribute.php#">Back to top</a> •
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

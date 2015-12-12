



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Constitute an Image</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, constitute, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="constitute.php#ConstituteImage">ConstituteImage</a> &bull; <a href="constitute.php#PingImage">PingImage</a> &bull; <a href="constitute.php#PingImages">PingImages</a> &bull; <a href="constitute.php#ReadImage">ReadImage</a> &bull; <a href="constitute.php#ReadImages">ReadImages</a> &bull; <a href="constitute.php#WriteImage">WriteImage</a> &bull; <a href="constitute.php#WriteImages">WriteImages</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/constitute_8c.html" id="ConstituteImage">ConstituteImage</a></h2>

<p>ConstituteImage() returns an image from the pixel data you supply. The pixel data must be in scanline order top-to-bottom.  The data can be char, short int, int, float, or double.  Float and double require the pixels to be normalized [0..1], otherwise [0..QuantumRange].  For example, to create a 640x480 image from unsigned red-green-blue character data, use:</p>

<pre class="text">
image = ConstituteImage(640,480,"RGB",CharPixel,pixels,&amp;exception);
</pre>

<p>The format of the ConstituteImage method is:</p>

<pre class="text">
Image *ConstituteImage(const size_t columns,const size_t rows,
  const char *map,const StorageType storage,const void *pixels,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>columns</dt>
<dd>width in pixels of the image. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>height in pixels of the image. </dd>

<dd> </dd>
<dt>map</dt>
<dd> This string reflects the expected ordering of the pixel array. It can be any combination or order of R = red, G = green, B = blue, A = alpha (0 is transparent), O = opacity (0 is opaque), C = cyan, Y = yellow, M = magenta, K = black, I = intensity (for grayscale), P = pad. </dd>

<dd> </dd>
<dt>storage</dt>
<dd>Define the data type of the pixels.  Float and double types are expected to be normalized [0..1] otherwise [0..QuantumRange].  Choose from these types: CharPixel, DoublePixel, FloatPixel, IntegerPixel, LongPixel, QuantumPixel, or ShortPixel. </dd>

<dd> </dd>
<dt>pixels</dt>
<dd>This array of values contain the pixel components as defined by map and type.  You must preallocate this array where the expected length varies depending on the values of width, height, map, and type. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/constitute_8c.html" id="PingImage">PingImage</a></h2>

<p>PingImage() returns all the properties of an image or image sequence except for the pixels.  It is much faster and consumes far less memory than ReadImage().  On failure, a NULL image is returned and exception describes the reason for the failure.</p>

<p>The format of the PingImage method is:</p>

<pre class="text">
Image *PingImage(const ImageInfo *image_info,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>Ping the image defined by the file or filename members of this structure. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/constitute_8c.html" id="PingImages">PingImages</a></h2>

<p>PingImages() pings one or more images and returns them as an image list.</p>

<p>The format of the PingImage method is:</p>

<pre class="text">
Image *PingImages(ImageInfo *image_info,const char *filename,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the image filename. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/constitute_8c.html" id="ReadImage">ReadImage</a></h2>

<p>ReadImage() reads an image or image sequence from a file or file handle. The method returns a NULL if there is a memory shortage or if the image cannot be read.  On failure, a NULL image is returned and exception describes the reason for the failure.</p>

<p>The format of the ReadImage method is:</p>

<pre class="text">
Image *ReadImage(const ImageInfo *image_info,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>Read the image defined by the file or filename members of this structure. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/constitute_8c.html" id="ReadImages">ReadImages</a></h2>

<p>ReadImages() reads one or more images and returns them as an image list.</p>

<p>The format of the ReadImage method is:</p>

<pre class="text">
Image *ReadImages(ImageInfo *image_info,const char *filename,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the image filename. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/constitute_8c.html" id="WriteImage">WriteImage</a></h2>

<p>WriteImage() writes an image or an image sequence to a file or file handle. If writing to a file is on disk, the name is defined by the filename member of the image structure.  WriteImage() returns MagickFalse is there is a memory shortage or if the image cannot be written.  Check the exception member of image to determine the cause for any failure.</p>

<p>The format of the WriteImage method is:</p>

<pre class="text">
MagickBooleanType WriteImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/constitute_8c.html" id="WriteImages">WriteImages</a></h2>

<p>WriteImages() writes an image sequence into one or more files.  While WriteImage() can write an image sequence, it is limited to writing the sequence into a single file using a format which supports multiple frames.   WriteImages(), however, does not have this limitation, instead it generates multiple output files if necessary (or when requested).  When ImageInfo's adjoin flag is set to MagickFalse, the file name is expected to include a printf-style formatting string for the frame number (e.g. "image02d.png").</p>

<p>The format of the WriteImages method is:</p>

<pre class="text">
MagickBooleanType WriteImages(const ImageInfo *image_info,Image *images,
  const char *filename,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd> </dd>
<dt>images</dt>
<dd>the image list. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the image filename. </dd>

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
    <p><a href="constitute.php#">Back to top</a> •
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

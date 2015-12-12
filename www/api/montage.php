



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Create an Image Thumbnail</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, create, an, image, thumbnail, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="montage.php#CloneMontageInfo">CloneMontageInfo</a> &bull; <a href="montage.php#DestroyMontageInfo">DestroyMontageInfo</a> &bull; <a href="montage.php#GetMontageInfo">GetMontageInfo</a> &bull; <a href="montage.php#MontageImageList">MontageImageList</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/montage_8c.html" id="CloneMontageInfo">CloneMontageInfo</a></h2>

<p>CloneMontageInfo() makes a copy of the given montage info structure.  If NULL is specified, a new image info structure is created initialized to default values.</p>

<p>The format of the CloneMontageInfo method is:</p>

<pre class="text">
MontageInfo *CloneMontageInfo(const ImageInfo *image_info,
  const MontageInfo *montage_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd> </dd>
<dt>montage_info</dt>
<dd>the montage info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/montage_8c.html" id="DestroyMontageInfo">DestroyMontageInfo</a></h2>

<p>DestroyMontageInfo() deallocates memory associated with montage_info.</p>

<p>The format of the DestroyMontageInfo method is:</p>

<pre class="text">
MontageInfo *DestroyMontageInfo(MontageInfo *montage_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>montage_info</dt>
<dd>Specifies a pointer to an MontageInfo structure. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/montage_8c.html" id="GetMontageInfo">GetMontageInfo</a></h2>

<p>GetMontageInfo() initializes montage_info to default values.</p>

<p>The format of the GetMontageInfo method is:</p>

<pre class="text">
void GetMontageInfo(const ImageInfo *image_info,
  MontageInfo *montage_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>a structure of type ImageInfo. </dd>

<dd> </dd>
<dt>montage_info</dt>
<dd>Specifies a pointer to a MontageInfo structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/montage_8c.html" id="MontageImageList">MontageImageList</a></h2>

<p>MontageImageList() is a layout manager that lets you tile one or more thumbnails across an image canvas.</p>

<p>The format of the MontageImageList method is:</p>

<pre class="text">
Image *MontageImageList(const ImageInfo *image_info,
  const MontageInfo *montage_info,Image *images,
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
<dt>montage_info</dt>
<dd>Specifies a pointer to a MontageInfo structure. </dd>

<dd> </dd>
<dt>images</dt>
<dd>Specifies a pointer to an array of Image structures. </dd>

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
    <p><a href="montage.php#">Back to top</a> •
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

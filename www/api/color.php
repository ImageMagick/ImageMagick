



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Count the Colors in an Image</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, count, the, colors, in, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="color.php#AcquireColorCache">AcquireColorCache</a> &bull; <a href="color.php#GetColorInfoList">GetColorInfoList</a> &bull; <a href="color.php#GetColorList">GetColorList</a> &bull; <a href="color.php#ListColorInfo">ListColorInfo</a> &bull; <a href="color.php#QueryColorname">QueryColorname</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/color_8c.html" id="AcquireColorCache">AcquireColorCache</a></h2>

<p>AcquireColorCache() caches one or more color configurations which provides a mapping between color attributes and a color name.</p>

<p>The format of the AcquireColorCache method is:</p>

<pre class="text">
LinkedListInfo *AcquireColorCache(const char *filename,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>filename</dt>
<dd>the font file name. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/color_8c.html" id="GetColorInfoList">GetColorInfoList</a></h2>

<p>GetColorInfoList() returns any colors that match the specified pattern.</p>

<p>The format of the GetColorInfoList function is:</p>

<pre class="text">
const ColorInfo **GetColorInfoList(const char *pattern,
  size_t *number_colors,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>pattern</dt>
<dd>Specifies a pointer to a text string containing a pattern. </dd>

<dd> </dd>
<dt>number_colors</dt>
<dd> This integer returns the number of colors in the list. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/color_8c.html" id="GetColorList">GetColorList</a></h2>

<p>GetColorList() returns any colors that match the specified pattern.</p>

<p>The format of the GetColorList function is:</p>

<pre class="text">
char **GetColorList(const char *pattern,size_t *number_colors,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>pattern</dt>
<dd>Specifies a pointer to a text string containing a pattern. </dd>

<dd> </dd>
<dt>number_colors</dt>
<dd> This integer returns the number of colors in the list. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/color_8c.html" id="ListColorInfo">ListColorInfo</a></h2>

<p>ListColorInfo() lists color names to the specified file.  Color names are a convenience.  Rather than defining a color by its red, green, and blue intensities just use a color name such as white, blue, or yellow.</p>

<p>The format of the ListColorInfo method is:</p>

<pre class="text">
MagickBooleanType ListColorInfo(FILE *file,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>file</dt>
<p>List color names to this file handle.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/color_8c.html" id="QueryColorname">QueryColorname</a></h2>

<p>QueryColorname() returns a named color for the given color intensity. If an exact match is not found, a hex value is returned instead.  For example an intensity of rgb:(0,0,0) returns black whereas rgb:(223,223,223) returns #dfdfdf.</p>

<p>UPDATE: the 'image' argument is no longer needed as all information should have been preset using GetPixelInfo().</p>

<p>The format of the QueryColorname method is:</p>

<pre class="text">
MagickBooleanType QueryColorname(const Image *image,
  const PixelInfo *color,const ComplianceType compliance,char *name,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>image</dt>
<p>the image. (not used! - color gets settings from GetPixelInfo()</p>

<dt>color</dt>
<p>the color intensities.</p>

<dt>Compliance</dt>
<p>Adhere to this color standard: SVG, X11, or XPM.</p>

<dt>name</dt>
<p>Return the color name or hex value.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="color.php#">Back to top</a> •
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

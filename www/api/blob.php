



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Read or Write Binary Large OBjects</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, read, or, write, binary, large, objects, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="blob.php#BlobToImage">BlobToImage</a> &bull; <a href="blob.php#FileToImage">FileToImage</a> &bull; <a href="blob.php#GetBlobProperties">GetBlobProperties</a> &bull; <a href="blob.php#ImageToBlob">ImageToBlob</a> &bull; <a href="blob.php#ImageToFile">ImageToFile</a> &bull; <a href="blob.php#ImagesToBlob">ImagesToBlob</a> &bull; <a href="blob.php#InjectImageBlob">InjectImageBlob</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/blob_8c.html" id="BlobToImage">BlobToImage</a></h2>

<p>BlobToImage() implements direct to memory image formats.  It returns the blob as an image.</p>

<p>The format of the BlobToImage method is:</p>

<pre class="text">
Image *BlobToImage(const ImageInfo *image_info,const void *blob,
  const size_t length,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. </dd>

<dd> </dd>
<dt>blob</dt>
<dd>the address of a character stream in one of the image formats understood by ImageMagick. </dd>

<dd> </dd>
<dt>length</dt>
<dd>This size_t integer reflects the length in bytes of the blob. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/blob_8c.html" id="FileToImage">FileToImage</a></h2>

<p>FileToImage() write the contents of a file to an image.</p>

<p>The format of the FileToImage method is:</p>

<pre class="text">
MagickBooleanType FileToImage(Image *,const char *filename)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the filename. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/blob_8c.html" id="GetBlobProperties">GetBlobProperties</a></h2>

<p>GetBlobProperties() returns information about an image blob.</p>

<p>The format of the GetBlobProperties method is:</p>

<pre class="text">
const struct stat *GetBlobProperties(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/blob_8c.html" id="ImageToBlob">ImageToBlob</a></h2>

<p>ImageToBlob() implements direct to memory image formats.  It returns the image as a formatted blob and its length.  The magick member of the Image structure determines the format of the returned blob (GIF, JPEG, PNG, etc.).  This method is the equivalent of WriteImage(), but writes the formatted "file" to a memory buffer rather than to an actual file.</p>

<p>The format of the ImageToBlob method is:</p>

<pre class="text">
void *ImageToBlob(const ImageInfo *image_info,Image *image,
  size_t *length,ExceptionInfo *exception)
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
<dt>length</dt>
<dd>return the actual length of the blob. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/blob_8c.html" id="ImageToFile">ImageToFile</a></h2>

<p>ImageToFile() writes an image to a file.  It returns MagickFalse if an error occurs otherwise MagickTrue.</p>

<p>The format of the ImageToFile method is:</p>

<pre class="text">
 MagickBooleanType ImageToFile(Image *image,char *filename,
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
<dt>filename</dt>
<dd>Write the image to this file. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/blob_8c.html" id="ImagesToBlob">ImagesToBlob</a></h2>

<p>ImagesToBlob() implements direct to memory image formats.  It returns the image sequence as a blob and its length.  The magick member of the ImageInfo structure determines the format of the returned blob (GIF, JPEG,  PNG, etc.)</p>

<p>Note, some image formats do not permit multiple images to the same image stream (e.g. JPEG).  in this instance, just the first image of the sequence is returned as a blob.</p>

<p>The format of the ImagesToBlob method is:</p>

<pre class="text">
void *ImagesToBlob(const ImageInfo *image_info,Image *images,
  size_t *length,ExceptionInfo *exception)
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
<dt>length</dt>
<dd>return the actual length of the blob. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/blob_8c.html" id="InjectImageBlob">InjectImageBlob</a></h2>

<p>InjectImageBlob() injects the image with a copy of itself in the specified format (e.g. inject JPEG into a PDF image).</p>

<p>The format of the InjectImageBlob method is:</p>

<pre class="text">
MagickBooleanType InjectImageBlob(const ImageInfo *image_info,
  Image *image,Image *inject_image,const char *format,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info.. </dd>

<dd> </dd>
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>inject_image</dt>
<dd>inject into the image stream. </dd>

<dd> </dd>
<dt>format</dt>
<dd>the image format. </dd>

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
    <p><a href="blob.php#">Back to top</a> •
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

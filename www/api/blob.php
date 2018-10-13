



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Read or Write Binary Large OBjects @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, read, or, write, binary, large, objects, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/blob.php" rel="canonical">
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
<p class="text-center"><a href="blob.php#BlobToImage">BlobToImage</a> &bull; <a href="blob.php#CustomStreamToImage">CustomStreamToImage</a> &bull; <a href="blob.php#FileToBlob">FileToBlob</a> &bull; <a href="blob.php#FileToImage">FileToImage</a> &bull; <a href="blob.php#GetBlobProperties">GetBlobProperties</a> &bull; <a href="blob.php#ImageToBlob">ImageToBlob</a> &bull; <a href="blob.php#ImageToFile">ImageToFile</a> &bull; <a href="blob.php#ImagesToBlob">ImagesToBlob</a> &bull; <a href="blob.php#InjectImageBlob">InjectImageBlob</a> &bull; <a href="blob.php#IsBlobExempt">IsBlobExempt</a> &bull; <a href="blob.php#IsBlobSeekable">IsBlobSeekable</a> &bull; <a href="blob.php#IsBlobTemporary">IsBlobTemporary</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="BlobToImage">BlobToImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="CustomStreamToImage">CustomStreamToImage</a></h2>

<p>CustomStreamToImage() is the equivalent of ReadImage(), but reads the formatted "file" from the suplied method rather than to an actual file.</p>

<p>The format of the CustomStreamToImage method is:</p>

<pre class="text">
Image *CustomStreamToImage(const ImageInfo *image_info,
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
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="FileToBlob">FileToBlob</a></h2>

<p>FileToBlob() returns the contents of a file as a buffer terminated with the '\0' character.  The length of the buffer (not including the extra terminating '\0' character) is returned via the 'length' parameter.  Free the buffer with RelinquishMagickMemory().</p>

<p>The format of the FileToBlob method is:</p>

<pre class="text">
void *FileToBlob(const char *filename,const size_t extent,
  size_t *length,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>blob</dt>
<dd> FileToBlob() returns the contents of a file as a blob.  If an error occurs NULL is returned. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the filename. </dd>

<dd> </dd>
<dt>extent</dt>
<dd> The maximum length of the blob. </dd>

<dd> </dd>
<dt>length</dt>
<dd>On return, this reflects the actual length of the blob. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="FileToImage">FileToImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="GetBlobProperties">GetBlobProperties</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="ImageToBlob">ImageToBlob</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="ImageToFile">ImageToFile</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="ImagesToBlob">ImagesToBlob</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="InjectImageBlob">InjectImageBlob</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="IsBlobExempt">IsBlobExempt</a></h2>

<p>IsBlobExempt() returns true if the blob is exempt.</p>

<p>The format of the IsBlobExempt method is:</p>

<pre class="text">
 MagickBooleanType IsBlobExempt(const Image *image)
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
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="IsBlobSeekable">IsBlobSeekable</a></h2>

<p>IsBlobSeekable() returns true if the blob is seekable.</p>

<p>The format of the IsBlobSeekable method is:</p>

<pre class="text">
 MagickBooleanType IsBlobSeekable(const Image *image)
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
<h2><a href="https://imagemagick.org/api/MagickCore/blob_8c.html" id="IsBlobTemporary">IsBlobTemporary</a></h2>

<p>IsBlobTemporary() returns true if the blob is temporary.</p>

<p>The format of the IsBlobTemporary method is:</p>

<pre class="text">
 MagickBooleanType IsBlobTemporary(const Image *image)
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
</div>
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="blob.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
    &nbsp; &nbsp;
    <a href="http://pgp.mit.edu/pks/lookup?op=get&amp;search=0x89AB63D48277377A">Public Key</a> •
    <a href="https://imagemagick.org/script/support.php">Donate</a> •
    <a href="https://imagemagick.org/script/contact.php">Contact Us</a>
    <br/>
        <small>© 1999-2018 ImageMagick Studio LLC</small></p>
  </footer>

  <!-- Javascript assets -->
  <script src="https://imagemagick.org/assets/magick-js.php" crossorigin="anonymous"></script>
  <script>window.jQuery || document.write('<script src="https://imagemagick.org/assets/jquery.min.js"><\/script>')</script>
</body>
</html>
<!-- Magick Cache 5th September 2018 03:15 -->
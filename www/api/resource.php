



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Monitor or Limit Resource Consumption</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, monitor, or, limit, resource, consumption, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="resource.php#AcquireMagickResource">AcquireMagickResource</a> &bull; <a href="resource.php#AcquireUniqueFileResource">AcquireUniqueFileResource</a> &bull; <a href="resource.php#GetMagickResource">GetMagickResource</a> &bull; <a href="resource.php#GetMagickResourceLimit">GetMagickResourceLimit</a> &bull; <a href="resource.php#ListMagickResourceInfo">ListMagickResourceInfo</a> &bull; <a href="resource.php#RelinquishMagickResource">RelinquishMagickResource</a> &bull; <a href="resource.php#RelinquishUniqueFileResource">RelinquishUniqueFileResource</a> &bull; <a href="resource.php#SetMagickResourceLimit">SetMagickResourceLimit</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resource_8c.html" id="AcquireMagickResource">AcquireMagickResource</a></h2>

<p>AcquireMagickResource() acquires resources of the specified type. MagickFalse is returned if the specified resource is exhausted otherwise MagickTrue.</p>

<p>The format of the AcquireMagickResource() method is:</p>

<pre class="text">
MagickBooleanType AcquireMagickResource(const ResourceType type,
  const MagickSizeType size)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>type</dt>
<dd>the type of resource. </dd>

<dd> </dd>
<dt>size</dt>
<dd>the number of bytes needed from for this resource. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resource_8c.html" id="AcquireUniqueFileResource">AcquireUniqueFileResource</a></h2>

<p>AcquireUniqueFileResource() returns a unique file name, and returns a file descriptor for the file open for reading and writing.</p>

<p>The format of the AcquireUniqueFileResource() method is:</p>

<pre class="text">
int AcquireUniqueFileResource(char *path)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt> path</dt>
<dd> Specifies a pointer to an array of characters.  The unique path name is returned in this array. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resource_8c.html" id="GetMagickResource">GetMagickResource</a></h2>

<p>GetMagickResource() returns the specified resource.</p>

<p>The format of the GetMagickResource() method is:</p>

<pre class="text">
MagickSizeType GetMagickResource(const ResourceType type)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>type</dt>
<dd>the type of resource. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resource_8c.html" id="GetMagickResourceLimit">GetMagickResourceLimit</a></h2>

<p>GetMagickResourceLimit() returns the specified resource limit.</p>

<p>The format of the GetMagickResourceLimit() method is:</p>

<pre class="text">
MagickSizeType GetMagickResourceLimit(const ResourceType type)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>type</dt>
<dd>the type of resource. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resource_8c.html" id="ListMagickResourceInfo">ListMagickResourceInfo</a></h2>

<p>ListMagickResourceInfo() lists the resource info to a file.</p>

<p>The format of the ListMagickResourceInfo method is:</p>

<pre class="text">
MagickBooleanType ListMagickResourceInfo(FILE *file,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows.</p>

<dt>file</dt>
<p>An pointer to a FILE.</p>

<dt>exception</dt>
<p>return any errors or warnings in this structure.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resource_8c.html" id="RelinquishMagickResource">RelinquishMagickResource</a></h2>

<p>RelinquishMagickResource() relinquishes resources of the specified type.</p>

<p>The format of the RelinquishMagickResource() method is:</p>

<pre class="text">
void RelinquishMagickResource(const ResourceType type,
  const MagickSizeType size)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>type</dt>
<dd>the type of resource. </dd>

<dd> </dd>
<dt>size</dt>
<dd>the size of the resource. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resource_8c.html" id="RelinquishUniqueFileResource">RelinquishUniqueFileResource</a></h2>

<p>RelinquishUniqueFileResource() relinquishes a unique file resource.</p>

<p>The format of the RelinquishUniqueFileResource() method is:</p>

<pre class="text">
MagickBooleanType RelinquishUniqueFileResource(const char *path)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>name</dt>
<dd>the name of the temporary resource. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/resource_8c.html" id="SetMagickResourceLimit">SetMagickResourceLimit</a></h2>

<p>SetMagickResourceLimit() sets the limit for a particular resource.</p>

<p>The format of the SetMagickResourceLimit() method is:</p>

<pre class="text">
MagickBooleanType SetMagickResourceLimit(const ResourceType type,
  const MagickSizeType limit)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>type</dt>
<dd>the type of resource. </dd>

<dd> </dd>
<dt>limit</dt>
<dd>the maximum limit for the resource. </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="resource.php#">Back to top</a> •
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





<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Get/Set Image Properties</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, get/set, image, properties, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="property.php#CloneImageProperties">CloneImageProperties</a> &bull; <a href="property.php#DefineImageProperty">DefineImageProperty</a> &bull; <a href="property.php#DeleteImageProperty">DeleteImageProperty</a> &bull; <a href="property.php#DestroyImageProperties">DestroyImageProperties</a> &bull; <a href="property.php#FormatImageProperty">FormatImageProperty</a> &bull; <a href="property.php#GetImageProperty">GetImageProperty</a> &bull; <a href="property.php#GetNextImageProperty">GetNextImageProperty</a> &bull; <a href="property.php#InterpretImageProperties">InterpretImageProperties</a> &bull; <a href="property.php#(void) LogMagickEvent(TraceEvent,GetMagickModule">(void) LogMagickEvent(TraceEvent,GetMagickModule</a> &bull; <a href="property.php#RemoveImageProperty">RemoveImageProperty</a> &bull; <a href="property.php#ResetImagePropertyIterator">ResetImagePropertyIterator</a> &bull; <a href="property.php#SetImageProperty">SetImageProperty</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="CloneImageProperties">CloneImageProperties</a></h2>

<p>CloneImageProperties() clones all the image properties to another image.</p>

<p>The format of the CloneImageProperties method is:</p>

<pre class="text">
MagickBooleanType CloneImageProperties(Image *image,
  const Image *clone_image)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>clone_image</dt>
<dd>the clone image. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="DefineImageProperty">DefineImageProperty</a></h2>

<p>DefineImageProperty() associates an assignment string of the form "key=value" with an artifact or options. It is equivelent to SetImageProperty()</p>

<p>The format of the DefineImageProperty method is:</p>

<pre class="text">
MagickBooleanType DefineImageProperty(Image *image,const char *property,
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
<dt>property</dt>
<dd>the image property. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="DeleteImageProperty">DeleteImageProperty</a></h2>

<p>DeleteImageProperty() deletes an image property.</p>

<p>The format of the DeleteImageProperty method is:</p>

<pre class="text">
MagickBooleanType DeleteImageProperty(Image *image,const char *property)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>property</dt>
<dd>the image property. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="DestroyImageProperties">DestroyImageProperties</a></h2>

<p>DestroyImageProperties() destroys all properties and associated memory attached to the given image.</p>

<p>The format of the DestroyDefines method is:</p>

<pre class="text">
void DestroyImageProperties(Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="FormatImageProperty">FormatImageProperty</a></h2>

<p>FormatImageProperty() permits formatted property/value pairs to be saved as an image property.</p>

<p>The format of the FormatImageProperty method is:</p>

<pre class="text">
MagickBooleanType FormatImageProperty(Image *image,const char *property,
  const char *format,...)
</pre>

<p>A description of each parameter follows.</p>

<dt> image</dt>
<p>The image.</p>

<dt> property</dt>
<p>The attribute property.</p>

<dt> format</dt>
<p>A string describing the format to use to write the remaining arguments.</p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="GetImageProperty">GetImageProperty</a></h2>

<p>GetImageProperty() gets a value associated with an image property.</p>

<p>This includes,  profile prefixes, such as "exif:", "iptc:" and "8bim:" It does not handle non-prifile prefixes, such as "fx:", "option:", or "artifact:".</p>

<p>The returned string is stored as a properity of the same name for faster lookup later. It should NOT be freed by the caller.</p>

<p>The format of the GetImageProperty method is:</p>

<pre class="text">
const char *GetImageProperty(const Image *image,const char *key,
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
<dt>key</dt>
<dd>the key. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="GetNextImageProperty">GetNextImageProperty</a></h2>

<p>GetNextImageProperty() gets the next free-form string property name.</p>

<p>The format of the GetNextImageProperty method is:</p>

<pre class="text">
char *GetNextImageProperty(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="InterpretImageProperties">InterpretImageProperties</a></h2>

<p>InterpretImageProperties() replaces any embedded formatting characters with the appropriate image property and returns the interpreted text.</p>

<p>This searches for and replaces \n \r \          replaced by newline, return, and percent resp. &amp;lt; &amp;gt; &amp;amp;   replaced by '&lt;', '&gt;', '&amp;' resp. replaced by percent</p>

<p>x [x]       where 'x' is a single letter properity, case sensitive). [type:name]  where 'type' a is special and known prefix. [name]       where 'name' is a specifically known attribute, calculated value, or a per-image property string name, or a per-image 'artifact' (as generated from a global option). It may contain ':' as long as the prefix is not special.</p>

<p>Single letter  substitutions will only happen if the character before the percent is NOT a number. But braced substitutions will always be performed. This prevents the typical usage of percent in a interpreted geometry argument from being substituted when the percent is a geometry flag.</p>

<p>If 'glob-expresions' ('*' or '?' characters) is used for 'name' it may be used as a search pattern to print multiple lines of "name=value\n" pairs of the associacted set of properties.</p>

<p>The returned string must be freed using DestoryString() by the caller.</p>

<p>The format of the InterpretImageProperties method is:</p>

<pre class="text">
char *InterpretImageProperties(ImageInfo *image_info,
  Image *image,const char *embed_text,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image_info</dt>
<dd>the image info. (required) </dd>

<dd> </dd>
<dt>image</dt>
<dd>the image. (optional) </dd>

<dd> </dd>
<dt>embed_text</dt>
<dd>the address of a character string containing the embedded formatting characters. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="(void)_LogMagickEvent(TraceEvent,GetMagickModule">(void) LogMagickEvent(TraceEvent,GetMagickModule</a></h2>

<p>(void) LogMagickEvent(TraceEvent,GetMagickModule(),"s",image-&gt;filename); else if( image_info != (ImageInfo *) NULL &amp;&amp; IfMagickTrue(image_info-&gt;debug)) (void) LogMagickEvent(TraceEvent,GetMagickModule(),"s","no-image");</p>

<p>if (embed_text == (const char *) NULL) return(ConstantString("")); p=embed_text;</p>

<p>if (*p == '\0') return(ConstantString(""));</p>

<p>if ((*p == '@') &amp;&amp; (IsPathAccessible(p+1) != MagickFalse)) { /* handle a '@' replace string from file */ interpret_text=FileToString(p+1,~0UL,exception); if (interpret_text != (char *) NULL) return(interpret_text); }</p>

<p>/* Translate any embedded format characters. </p>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="RemoveImageProperty">RemoveImageProperty</a></h2>

<p>RemoveImageProperty() removes a property from the image and returns its value.</p>

<p>In this case the ConstantString() value returned should be freed by the caller when finished.</p>

<p>The format of the RemoveImageProperty method is:</p>

<pre class="text">
char *RemoveImageProperty(Image *image,const char *property)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>property</dt>
<dd>the image property. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="ResetImagePropertyIterator">ResetImagePropertyIterator</a></h2>

<p>ResetImagePropertyIterator() resets the image properties iterator.  Use it in conjunction with GetNextImageProperty() to iterate over all the values associated with an image property.</p>

<p>The format of the ResetImagePropertyIterator method is:</p>

<pre class="text">
ResetImagePropertyIterator(Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/property_8c.html" id="SetImageProperty">SetImageProperty</a></h2>

<p>SetImageProperty() saves the given string value either to specific known attribute or to a freeform property string.</p>

<p>Attempting to set a property that is normally calculated will produce an exception.</p>

<p>The format of the SetImageProperty method is:</p>

<pre class="text">
MagickBooleanType SetImageProperty(Image *image,const char *property,
  const char *value,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>property</dt>
<dd>the image property. </dd>

<dd> </dd>
<dt>values</dt>
<dd>the image property values. </dd>

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
    <p><a href="property.php#">Back to top</a> •
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

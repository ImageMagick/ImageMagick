



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickWand, C API for ImageMagick: Property Methods</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickwc, api, for, imagemagick:, property, methods, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="magick-property.php#MagickDeleteImageArtifact">MagickDeleteImageArtifact</a> &bull; <a href="magick-property.php#MagickDeleteImageProperty">MagickDeleteImageProperty</a> &bull; <a href="magick-property.php#MagickDeleteOption">MagickDeleteOption</a> &bull; <a href="magick-property.php#MagickGetAntialias">MagickGetAntialias</a> &bull; <a href="magick-property.php#MagickGetBackgroundColor">MagickGetBackgroundColor</a> &bull; <a href="magick-property.php#MagickGetColorspace">MagickGetColorspace</a> &bull; <a href="magick-property.php#MagickGetCompression">MagickGetCompression</a> &bull; <a href="magick-property.php#MagickGetCompressionQuality">MagickGetCompressionQuality</a> &bull; <a href="magick-property.php#MagickGetCopyright">MagickGetCopyright</a> &bull; <a href="magick-property.php#MagickGetFilename">MagickGetFilename</a> &bull; <a href="magick-property.php#MagickGetFont">MagickGetFont</a> &bull; <a href="magick-property.php#MagickGetFormat">MagickGetFormat</a> &bull; <a href="magick-property.php#MagickGetGravity">MagickGetGravity</a> &bull; <a href="magick-property.php#MagickGetHomeURL">MagickGetHomeURL</a> &bull; <a href="magick-property.php#MagickGetImageArtifact">MagickGetImageArtifact</a> &bull; <a href="magick-property.php#MagickGetImageArtifacts">MagickGetImageArtifacts</a> &bull; <a href="magick-property.php#MagickGetImageProfile">MagickGetImageProfile</a> &bull; <a href="magick-property.php#MagickGetImageProfiles">MagickGetImageProfiles</a> &bull; <a href="magick-property.php#MagickGetImageProperty">MagickGetImageProperty</a> &bull; <a href="magick-property.php#MagickGetImageProperties">MagickGetImageProperties</a> &bull; <a href="magick-property.php#MagickGetInterlaceScheme">MagickGetInterlaceScheme</a> &bull; <a href="magick-property.php#MagickGetInterpolateMethod">MagickGetInterpolateMethod</a> &bull; <a href="magick-property.php#MagickGetOption">MagickGetOption</a> &bull; <a href="magick-property.php#MagickGetOptions">MagickGetOptions</a> &bull; <a href="magick-property.php#MagickGetOrientation">MagickGetOrientation</a> &bull; <a href="magick-property.php#MagickGetPackageName">MagickGetPackageName</a> &bull; <a href="magick-property.php#MagickGetPage">MagickGetPage</a> &bull; <a href="magick-property.php#MagickGetPointsize">MagickGetPointsize</a> &bull; <a href="magick-property.php#MagickGetQuantumDepth">MagickGetQuantumDepth</a> &bull; <a href="magick-property.php#MagickGetQuantumRange">MagickGetQuantumRange</a> &bull; <a href="magick-property.php#MagickGetReleaseDate">MagickGetReleaseDate</a> &bull; <a href="magick-property.php#MagickGetResolution">MagickGetResolution</a> &bull; <a href="magick-property.php#MagickGetResource">MagickGetResource</a> &bull; <a href="magick-property.php#MagickGetResourceLimit">MagickGetResourceLimit</a> &bull; <a href="magick-property.php#MagickGetSamplingFactors">MagickGetSamplingFactors</a> &bull; <a href="magick-property.php#MagickGetSize">MagickGetSize</a> &bull; <a href="magick-property.php#MagickGetSizeOffset">MagickGetSizeOffset</a> &bull; <a href="magick-property.php#MagickGetType">MagickGetType</a> &bull; <a href="magick-property.php#MagickGetVersion">MagickGetVersion</a> &bull; <a href="magick-property.php#MagickProfileImage">MagickProfileImage</a> &bull; <a href="magick-property.php#MagickRemoveImageProfile">MagickRemoveImageProfile</a> &bull; <a href="magick-property.php#MagickSetAntialias">MagickSetAntialias</a> &bull; <a href="magick-property.php#MagickSetBackgroundColor">MagickSetBackgroundColor</a> &bull; <a href="magick-property.php#MagickSetColorspace">MagickSetColorspace</a> &bull; <a href="magick-property.php#MagickSetCompression">MagickSetCompression</a> &bull; <a href="magick-property.php#MagickSetCompressionQuality">MagickSetCompressionQuality</a> &bull; <a href="magick-property.php#MagickSetDepth">MagickSetDepth</a> &bull; <a href="magick-property.php#MagickSetExtract">MagickSetExtract</a> &bull; <a href="magick-property.php#MagickSetFilename">MagickSetFilename</a> &bull; <a href="magick-property.php#MagickSetFont">MagickSetFont</a> &bull; <a href="magick-property.php#MagickSetFormat">MagickSetFormat</a> &bull; <a href="magick-property.php#MagickSetGravity">MagickSetGravity</a> &bull; <a href="magick-property.php#MagickSetImageArtifact">MagickSetImageArtifact</a> &bull; <a href="magick-property.php#MagickSetImageProfile">MagickSetImageProfile</a> &bull; <a href="magick-property.php#MagickSetImageProperty">MagickSetImageProperty</a> &bull; <a href="magick-property.php#MagickSetInterlaceScheme">MagickSetInterlaceScheme</a> &bull; <a href="magick-property.php#MagickSetInterpolateMethod">MagickSetInterpolateMethod</a> &bull; <a href="magick-property.php#MagickSetOption">MagickSetOption</a> &bull; <a href="magick-property.php#MagickSetOrientation">MagickSetOrientation</a> &bull; <a href="magick-property.php#MagickSetPage">MagickSetPage</a> &bull; <a href="magick-property.php#MagickSetPassphrase">MagickSetPassphrase</a> &bull; <a href="magick-property.php#MagickSetPointsize">MagickSetPointsize</a> &bull; <a href="magick-property.php#MagickSetProgressMonitor">MagickSetProgressMonitor</a> &bull; <a href="magick-property.php#MagickSetResourceLimit">MagickSetResourceLimit</a> &bull; <a href="magick-property.php#MagickSetResolution">MagickSetResolution</a> &bull; <a href="magick-property.php#MagickSetSamplingFactors">MagickSetSamplingFactors</a> &bull; <a href="magick-property.php#MagickSetSize">MagickSetSize</a> &bull; <a href="magick-property.php#MagickSetSizeOffset">MagickSetSizeOffset</a> &bull; <a href="magick-property.php#MagickSetType">MagickSetType</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickDeleteImageArtifact">MagickDeleteImageArtifact</a></h2>

<p>MagickDeleteImageArtifact() deletes a wand artifact.</p>

<p>The format of the MagickDeleteImageArtifact method is:</p>

<pre class="text">
MagickBooleanType MagickDeleteImageArtifact(MagickWand *wand,
  const char *artifact)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>artifact</dt>
<dd>the image artifact. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickDeleteImageProperty">MagickDeleteImageProperty</a></h2>

<p>MagickDeleteImageProperty() deletes a wand property.</p>

<p>The format of the MagickDeleteImageProperty method is:</p>

<pre class="text">
MagickBooleanType MagickDeleteImageProperty(MagickWand *wand,
  const char *property)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickDeleteOption">MagickDeleteOption</a></h2>

<p>MagickDeleteOption() deletes a wand option.</p>

<p>The format of the MagickDeleteOption method is:</p>

<pre class="text">
MagickBooleanType MagickDeleteOption(MagickWand *wand,
  const char *option)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>option</dt>
<dd>the image option. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetAntialias">MagickGetAntialias</a></h2>

<p>MagickGetAntialias() returns the antialias property associated with the wand.</p>

<p>The format of the MagickGetAntialias method is:</p>

<pre class="text">
MagickBooleanType MagickGetAntialias(const MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetBackgroundColor">MagickGetBackgroundColor</a></h2>

<p>MagickGetBackgroundColor() returns the wand background color.</p>

<p>The format of the MagickGetBackgroundColor method is:</p>

<pre class="text">
PixelWand *MagickGetBackgroundColor(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetColorspace">MagickGetColorspace</a></h2>

<p>MagickGetColorspace() gets the wand colorspace type.</p>

<p>The format of the MagickGetColorspace method is:</p>

<pre class="text">
ColorspaceType MagickGetColorspace(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetCompression">MagickGetCompression</a></h2>

<p>MagickGetCompression() gets the wand compression type.</p>

<p>The format of the MagickGetCompression method is:</p>

<pre class="text">
CompressionType MagickGetCompression(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetCompressionQuality">MagickGetCompressionQuality</a></h2>

<p>MagickGetCompressionQuality() gets the wand compression quality.</p>

<p>The format of the MagickGetCompressionQuality method is:</p>

<pre class="text">
size_t MagickGetCompressionQuality(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetCopyright">MagickGetCopyright</a></h2>

<p>MagickGetCopyright() returns the ImageMagick API copyright as a string constant.</p>

<p>The format of the MagickGetCopyright method is:</p>

<pre class="text">
const char *MagickGetCopyright(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetFilename">MagickGetFilename</a></h2>

<p>MagickGetFilename() returns the filename associated with an image sequence.</p>

<p>The format of the MagickGetFilename method is:</p>

<pre class="text">
const char *MagickGetFilename(const MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetFont">MagickGetFont</a></h2>

<p>MagickGetFont() returns the font associated with the MagickWand.</p>

<p>The format of the MagickGetFont method is:</p>

<pre class="text">
char *MagickGetFont(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetFormat">MagickGetFormat</a></h2>

<p>MagickGetFormat() returns the format of the magick wand.</p>

<p>The format of the MagickGetFormat method is:</p>

<pre class="text">
const char MagickGetFormat(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetGravity">MagickGetGravity</a></h2>

<p>MagickGetGravity() gets the wand gravity.</p>

<p>The format of the MagickGetGravity method is:</p>

<pre class="text">
GravityType MagickGetGravity(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetHomeURL">MagickGetHomeURL</a></h2>

<p>MagickGetHomeURL() returns the ImageMagick home URL.</p>

<p>The format of the MagickGetHomeURL method is:</p>

<pre class="text">
char *MagickGetHomeURL(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetImageArtifact">MagickGetImageArtifact</a></h2>

<p>MagickGetImageArtifact() returns a value associated with the specified artifact.  Use MagickRelinquishMemory() to free the value when you are finished with it.</p>

<p>The format of the MagickGetImageArtifact method is:</p>

<pre class="text">
char *MagickGetImageArtifact(MagickWand *wand,const char *artifact)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>artifact</dt>
<dd>the artifact. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetImageArtifacts">MagickGetImageArtifacts</a></h2>

<p>MagickGetImageArtifacts() returns all the artifact names that match the specified pattern associated with a wand.  Use MagickGetImageProperty() to return the value of a particular artifact.  Use MagickRelinquishMemory() to free the value when you are finished with it.</p>

<p>The format of the MagickGetImageArtifacts method is:</p>

<pre class="text">
char *MagickGetImageArtifacts(MagickWand *wand,
  const char *pattern,size_t *number_artifacts)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>pattern</dt>
<dd>Specifies a pointer to a text string containing a pattern. </dd>

<dd> </dd>
<dt>number_artifacts</dt>
<dd>the number artifacts associated with this wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetImageProfile">MagickGetImageProfile</a></h2>

<p>MagickGetImageProfile() returns the named image profile.</p>

<p>The format of the MagickGetImageProfile method is:</p>

<pre class="text">
unsigned char *MagickGetImageProfile(MagickWand *wand,const char *name,
  size_t *length)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>name</dt>
<dd>Name of profile to return: ICC, IPTC, or generic profile. </dd>

<dd> </dd>
<dt>length</dt>
<dd>the length of the profile. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetImageProfiles">MagickGetImageProfiles</a></h2>

<p>MagickGetImageProfiles() returns all the profile names that match the specified pattern associated with a wand.  Use MagickGetImageProfile() to return the value of a particular property.  Use MagickRelinquishMemory() to free the value when you are finished with it.</p>

<p>The format of the MagickGetImageProfiles method is:</p>

<pre class="text">
char *MagickGetImageProfiles(MagickWand *wand,const char *pattern,
  size_t *number_profiles)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>pattern</dt>
<dd>Specifies a pointer to a text string containing a pattern. </dd>

<dd> </dd>
<dt>number_profiles</dt>
<dd>the number profiles associated with this wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetImageProperty">MagickGetImageProperty</a></h2>

<p>MagickGetImageProperty() returns a value associated with the specified property.  Use MagickRelinquishMemory() to free the value when you are finished with it.</p>

<p>The format of the MagickGetImageProperty method is:</p>

<pre class="text">
char *MagickGetImageProperty(MagickWand *wand,const char *property)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>property</dt>
<dd>the property. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetImageProperties">MagickGetImageProperties</a></h2>

<p>MagickGetImageProperties() returns all the property names that match the specified pattern associated with a wand.  Use MagickGetImageProperty() to return the value of a particular property.  Use MagickRelinquishMemory() to free the value when you are finished with it.</p>

<p>The format of the MagickGetImageProperties method is:</p>

<pre class="text">
char *MagickGetImageProperties(MagickWand *wand,
  const char *pattern,size_t *number_properties)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>pattern</dt>
<dd>Specifies a pointer to a text string containing a pattern. </dd>

<dd> </dd>
<dt>number_properties</dt>
<dd>the number properties associated with this wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetInterlaceScheme">MagickGetInterlaceScheme</a></h2>

<p>MagickGetInterlaceScheme() gets the wand interlace scheme.</p>

<p>The format of the MagickGetInterlaceScheme method is:</p>

<pre class="text">
InterlaceType MagickGetInterlaceScheme(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetInterpolateMethod">MagickGetInterpolateMethod</a></h2>

<p>MagickGetInterpolateMethod() gets the wand compression.</p>

<p>The format of the MagickGetInterpolateMethod method is:</p>

<pre class="text">
PixelInterpolateMethod MagickGetInterpolateMethod(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetOption">MagickGetOption</a></h2>

<p>MagickGetOption() returns a value associated with a wand and the specified key.  Use MagickRelinquishMemory() to free the value when you are finished with it.</p>

<p>The format of the MagickGetOption method is:</p>

<pre class="text">
char *MagickGetOption(MagickWand *wand,const char *key)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>key</dt>
<dd>the key. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetOptions">MagickGetOptions</a></h2>

<p>MagickGetOptions() returns all the option names that match the specified pattern associated with a wand.  Use MagickGetOption() to return the value of a particular option.  Use MagickRelinquishMemory() to free the value when you are finished with it.</p>

<p>The format of the MagickGetOptions method is:</p>

<pre class="text">
char *MagickGetOptions(MagickWand *wand,const char *pattern,
  size_t *number_options)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>pattern</dt>
<dd>Specifies a pointer to a text string containing a pattern. </dd>

<dd> </dd>
<dt>number_options</dt>
<dd>the number options associated with this wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetOrientation">MagickGetOrientation</a></h2>

<p>MagickGetOrientation() gets the wand orientation type.</p>

<p>The format of the MagickGetOrientation method is:</p>

<pre class="text">
OrientationType MagickGetOrientation(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetPackageName">MagickGetPackageName</a></h2>

<p>MagickGetPackageName() returns the ImageMagick package name as a string constant.</p>

<p>The format of the MagickGetPackageName method is:</p>

<pre class="text">
const char *MagickGetPackageName(void)
</pre>


<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetPage">MagickGetPage</a></h2>

<p>MagickGetPage() returns the page geometry associated with the magick wand.</p>

<p>The format of the MagickGetPage method is:</p>

<pre class="text">
MagickBooleanType MagickGetPage(const MagickWand *wand,
  size_t *width,size_t *height,ssize_t *x,ssize_t *y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the page width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>page height. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the page x-offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the page y-offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetPointsize">MagickGetPointsize</a></h2>

<p>MagickGetPointsize() returns the font pointsize associated with the MagickWand.</p>

<p>The format of the MagickGetPointsize method is:</p>

<pre class="text">
double MagickGetPointsize(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetQuantumDepth">MagickGetQuantumDepth</a></h2>

<p>MagickGetQuantumDepth() returns the ImageMagick quantum depth as a string constant.</p>

<p>The format of the MagickGetQuantumDepth method is:</p>

<pre class="text">
const char *MagickGetQuantumDepth(size_t *depth)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>depth</dt>
<dd>the quantum depth is returned as a number. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetQuantumRange">MagickGetQuantumRange</a></h2>

<p>MagickGetQuantumRange() returns the ImageMagick quantum range as a string constant.</p>

<p>The format of the MagickGetQuantumRange method is:</p>

<pre class="text">
const char *MagickGetQuantumRange(size_t *range)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>range</dt>
<dd>the quantum range is returned as a number. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetReleaseDate">MagickGetReleaseDate</a></h2>

<p>MagickGetReleaseDate() returns the ImageMagick release date as a string constant.</p>

<p>The format of the MagickGetReleaseDate method is:</p>

<pre class="text">
const char *MagickGetReleaseDate(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetResolution">MagickGetResolution</a></h2>

<p>MagickGetResolution() gets the image X and Y resolution.</p>

<p>The format of the MagickGetResolution method is:</p>

<pre class="text">
MagickBooleanType MagickGetResolution(const MagickWand *wand,double *x,
  double *y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the x-resolution. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the y-resolution. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetResource">MagickGetResource</a></h2>

<p>MagickGetResource() returns the specified resource in megabytes.</p>

<p>The format of the MagickGetResource method is:</p>

<pre class="text">
MagickSizeType MagickGetResource(const ResourceType type)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetResourceLimit">MagickGetResourceLimit</a></h2>

<p>MagickGetResourceLimit() returns the specified resource limit in megabytes.</p>

<p>The format of the MagickGetResourceLimit method is:</p>

<pre class="text">
MagickSizeType MagickGetResourceLimit(const ResourceType type)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetSamplingFactors">MagickGetSamplingFactors</a></h2>

<p>MagickGetSamplingFactors() gets the horizontal and vertical sampling factor.</p>

<p>The format of the MagickGetSamplingFactors method is:</p>

<pre class="text">
double *MagickGetSamplingFactor(MagickWand *wand,
  size_t *number_factors)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>number_factors</dt>
<dd>the number of factors in the returned array. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetSize">MagickGetSize</a></h2>

<p>MagickGetSize() returns the size associated with the magick wand.</p>

<p>The format of the MagickGetSize method is:</p>

<pre class="text">
MagickBooleanType MagickGetSize(const MagickWand *wand,
  size_t *columns,size_t *rows)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the width in pixels. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the height in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetSizeOffset">MagickGetSizeOffset</a></h2>

<p>MagickGetSizeOffset() returns the size offset associated with the magick wand.</p>

<p>The format of the MagickGetSizeOffset method is:</p>

<pre class="text">
MagickBooleanType MagickGetSizeOffset(const MagickWand *wand,
  ssize_t *offset)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>offset</dt>
<dd>the image offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetType">MagickGetType</a></h2>

<p>MagickGetType() returns the wand type.</p>

<p>The format of the MagickGetType method is:</p>

<pre class="text">
ImageType MagickGetType(MagickWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickGetVersion">MagickGetVersion</a></h2>

<p>MagickGetVersion() returns the ImageMagick API version as a string constant and as a number.</p>

<p>The format of the MagickGetVersion method is:</p>

<pre class="text">
const char *MagickGetVersion(size_t *version)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>version</dt>
<dd>the ImageMagick version is returned as a number. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickProfileImage">MagickProfileImage</a></h2>

<p>MagickProfileImage() adds or removes a ICC, IPTC, or generic profile from an image.  If the profile is NULL, it is removed from the image otherwise added.  Use a name of '*' and a profile of NULL to remove all profiles from the image.</p>

<p>The format of the MagickProfileImage method is:</p>

<pre class="text">
MagickBooleanType MagickProfileImage(MagickWand *wand,const char *name,
  const void *profile,const size_t length)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>name</dt>
<dd>Name of profile to add or remove: ICC, IPTC, or generic profile. </dd>

<dd> </dd>
<dt>profile</dt>
<dd>the profile. </dd>

<dd> </dd>
<dt>length</dt>
<dd>the length of the profile. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickRemoveImageProfile">MagickRemoveImageProfile</a></h2>

<p>MagickRemoveImageProfile() removes the named image profile and returns it.</p>

<p>The format of the MagickRemoveImageProfile method is:</p>

<pre class="text">
unsigned char *MagickRemoveImageProfile(MagickWand *wand,
  const char *name,size_t *length)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>name</dt>
<dd>Name of profile to return: ICC, IPTC, or generic profile. </dd>

<dd> </dd>
<dt>length</dt>
<dd>the length of the profile. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetAntialias">MagickSetAntialias</a></h2>

<p>MagickSetAntialias() sets the antialias propery of the wand.</p>

<p>The format of the MagickSetAntialias method is:</p>

<pre class="text">
MagickBooleanType MagickSetAntialias(MagickWand *wand,
  const MagickBooleanType antialias)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>antialias</dt>
<dd>the antialias property. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetBackgroundColor">MagickSetBackgroundColor</a></h2>

<p>MagickSetBackgroundColor() sets the wand background color.</p>

<p>The format of the MagickSetBackgroundColor method is:</p>

<pre class="text">
MagickBooleanType MagickSetBackgroundColor(MagickWand *wand,
  const PixelWand *background)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>background</dt>
<dd>the background pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetColorspace">MagickSetColorspace</a></h2>

<p>MagickSetColorspace() sets the wand colorspace type.</p>

<p>The format of the MagickSetColorspace method is:</p>

<pre class="text">
MagickBooleanType MagickSetColorspace(MagickWand *wand,
  const ColorspaceType colorspace)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>colorspace</dt>
<dd>the wand colorspace. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetCompression">MagickSetCompression</a></h2>

<p>MagickSetCompression() sets the wand compression type.</p>

<p>The format of the MagickSetCompression method is:</p>

<pre class="text">
MagickBooleanType MagickSetCompression(MagickWand *wand,
  const CompressionType compression)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>compression</dt>
<dd>the wand compression. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetCompressionQuality">MagickSetCompressionQuality</a></h2>

<p>MagickSetCompressionQuality() sets the wand compression quality.</p>

<p>The format of the MagickSetCompressionQuality method is:</p>

<pre class="text">
MagickBooleanType MagickSetCompressionQuality(MagickWand *wand,
  const size_t quality)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>quality</dt>
<dd>the wand compression quality. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetDepth">MagickSetDepth</a></h2>

<p>MagickSetDepth() sets the wand pixel depth.</p>

<p>The format of the MagickSetDepth method is:</p>

<pre class="text">
MagickBooleanType MagickSetDepth(MagickWand *wand,
  const size_t depth)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>depth</dt>
<dd>the wand pixel depth. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetExtract">MagickSetExtract</a></h2>

<p>MagickSetExtract() sets the extract geometry before you read or write an image file.  Use it for inline cropping (e.g. 200x200+0+0) or resizing (e.g.200x200).</p>

<p>The format of the MagickSetExtract method is:</p>

<pre class="text">
MagickBooleanType MagickSetExtract(MagickWand *wand,
  const char *geometry)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>geometry</dt>
<dd>the extract geometry. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetFilename">MagickSetFilename</a></h2>

<p>MagickSetFilename() sets the filename before you read or write an image file.</p>

<p>The format of the MagickSetFilename method is:</p>

<pre class="text">
MagickBooleanType MagickSetFilename(MagickWand *wand,
  const char *filename)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>filename</dt>
<dd>the image filename. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetFont">MagickSetFont</a></h2>

<p>MagickSetFont() sets the font associated with the MagickWand.</p>

<p>The format of the MagickSetFont method is:</p>

<pre class="text">
MagickBooleanType MagickSetFont(MagickWand *wand, const char *font)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>font</dt>
<dd>the font </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetFormat">MagickSetFormat</a></h2>

<p>MagickSetFormat() sets the format of the magick wand.</p>

<p>The format of the MagickSetFormat method is:</p>

<pre class="text">
MagickBooleanType MagickSetFormat(MagickWand *wand,const char *format)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>format</dt>
<dd>the image format. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetGravity">MagickSetGravity</a></h2>

<p>MagickSetGravity() sets the gravity type.</p>

<p>The format of the MagickSetGravity type is:</p>

<pre class="text">
MagickBooleanType MagickSetGravity(MagickWand *wand,
  const GravityType type)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>type</dt>
<dd>the gravity type. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetImageArtifact">MagickSetImageArtifact</a></h2>

<p>MagickSetImageArtifact() associates a artifact with an image.</p>

<p>The format of the MagickSetImageArtifact method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageArtifact(MagickWand *wand,
  const char *artifact,const char *value)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>artifact</dt>
<dd>the artifact. </dd>

<dd> </dd>
<dt>value</dt>
<dd>the value. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetImageProfile">MagickSetImageProfile</a></h2>

<p>MagickSetImageProfile() adds a named profile to the magick wand.  If a profile with the same name already exists, it is replaced.  This method differs from the MagickProfileImage() method in that it does not apply any CMS color profiles.</p>

<p>The format of the MagickSetImageProfile method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageProfile(MagickWand *wand,
  const char *name,const void *profile,const size_t length)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>name</dt>
<dd>Name of profile to add or remove: ICC, IPTC, or generic profile. </dd>

<dd> </dd>
<dt>profile</dt>
<dd>the profile. </dd>

<dd> </dd>
<dt>length</dt>
<dd>the length of the profile. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetImageProperty">MagickSetImageProperty</a></h2>

<p>MagickSetImageProperty() associates a property with an image.</p>

<p>The format of the MagickSetImageProperty method is:</p>

<pre class="text">
MagickBooleanType MagickSetImageProperty(MagickWand *wand,
  const char *property,const char *value)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>property</dt>
<dd>the property. </dd>

<dd> </dd>
<dt>value</dt>
<dd>the value. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetInterlaceScheme">MagickSetInterlaceScheme</a></h2>

<p>MagickSetInterlaceScheme() sets the image compression.</p>

<p>The format of the MagickSetInterlaceScheme method is:</p>

<pre class="text">
MagickBooleanType MagickSetInterlaceScheme(MagickWand *wand,
  const InterlaceType interlace_scheme)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>interlace_scheme</dt>
<dd>the image interlace scheme: NoInterlace, LineInterlace, PlaneInterlace, PartitionInterlace. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetInterpolateMethod">MagickSetInterpolateMethod</a></h2>

<p>MagickSetInterpolateMethod() sets the interpolate pixel method.</p>

<p>The format of the MagickSetInterpolateMethod method is:</p>

<pre class="text">
MagickBooleanType MagickSetInterpolateMethod(MagickWand *wand,
  const InterpolateMethodPixel method)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the interpolate pixel method. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetOption">MagickSetOption</a></h2>

<p>MagickSetOption() associates one or options with the wand (.e.g MagickSetOption(wand,"jpeg:perserve","yes")).</p>

<p>The format of the MagickSetOption method is:</p>

<pre class="text">
MagickBooleanType MagickSetOption(MagickWand *wand,const char *key,
  const char *value)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>key</dt>
<dd> The key. </dd>

<dd> </dd>
<dt>value</dt>
<dd> The value. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetOrientation">MagickSetOrientation</a></h2>

<p>MagickSetOrientation() sets the wand orientation type.</p>

<p>The format of the MagickSetOrientation method is:</p>

<pre class="text">
MagickBooleanType MagickSetOrientation(MagickWand *wand,
  const OrientationType orientation)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>orientation</dt>
<dd>the wand orientation. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetPage">MagickSetPage</a></h2>

<p>MagickSetPage() sets the page geometry of the magick wand.</p>

<p>The format of the MagickSetPage method is:</p>

<pre class="text">
MagickBooleanType MagickSetPage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>width</dt>
<dd>the page width. </dd>

<dd> </dd>
<dt>height</dt>
<dd>the page height. </dd>

<dd> </dd>
<dt>x</dt>
<dd>the page x-offset. </dd>

<dd> </dd>
<dt>y</dt>
<dd>the page y-offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetPassphrase">MagickSetPassphrase</a></h2>

<p>MagickSetPassphrase() sets the passphrase.</p>

<p>The format of the MagickSetPassphrase method is:</p>

<pre class="text">
MagickBooleanType MagickSetPassphrase(MagickWand *wand,
  const char *passphrase)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>passphrase</dt>
<dd>the passphrase. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetPointsize">MagickSetPointsize</a></h2>

<p>MagickSetPointsize() sets the font pointsize associated with the MagickWand.</p>

<p>The format of the MagickSetPointsize method is:</p>

<pre class="text">
MagickBooleanType MagickSetPointsize(MagickWand *wand,
  const double pointsize)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>pointsize</dt>
<dd>the size of the font </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetProgressMonitor">MagickSetProgressMonitor</a></h2>

<p>MagickSetProgressMonitor() sets the wand progress monitor to the specified method and returns the previous progress monitor if any.  The progress monitor method looks like this:</p>

<pre class="text">
    MagickBooleanType MagickProgressMonitor(const char *text,
const MagickOffsetType offset,const MagickSizeType span,
void *client_data)
</pre>

<p>If the progress monitor returns MagickFalse, the current operation is interrupted.</p>

<p>The format of the MagickSetProgressMonitor method is:</p>

<pre class="text">
MagickProgressMonitor MagickSetProgressMonitor(MagickWand *wand
  const MagickProgressMonitor progress_monitor,void *client_data)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>progress_monitor</dt>
<dd>Specifies a pointer to a method to monitor progress of an image operation. </dd>

<dd> </dd>
<dt>client_data</dt>
<dd>Specifies a pointer to any client data. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetResourceLimit">MagickSetResourceLimit</a></h2>

<p>MagickSetResourceLimit() sets the limit for a particular resource in megabytes.</p>

<p>The format of the MagickSetResourceLimit method is:</p>

<pre class="text">
MagickBooleanType MagickSetResourceLimit(const ResourceType type,
  const MagickSizeType limit)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>type</dt>
<dd>the type of resource: AreaResource, MemoryResource, MapResource, DiskResource, FileResource. </dd>

<dd> o The maximum limit for the resource. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetResolution">MagickSetResolution</a></h2>

<p>MagickSetResolution() sets the image resolution.</p>

<p>The format of the MagickSetResolution method is:</p>

<pre class="text">
MagickBooleanType MagickSetResolution(MagickWand *wand,
  const double x_resolution,const double y_resolution)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>x_resolution</dt>
<dd>the image x resolution. </dd>

<dd> </dd>
<dt>y_resolution</dt>
<dd>the image y resolution. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetSamplingFactors">MagickSetSamplingFactors</a></h2>

<p>MagickSetSamplingFactors() sets the image sampling factors.</p>

<p>The format of the MagickSetSamplingFactors method is:</p>

<pre class="text">
MagickBooleanType MagickSetSamplingFactors(MagickWand *wand,
  const size_t number_factors,const double *sampling_factors)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>number_factoes</dt>
<dd>the number of factors. </dd>

<dd> </dd>
<dt>sampling_factors</dt>
<dd>An array of doubles representing the sampling factor for each color component (in RGB order). </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetSize">MagickSetSize</a></h2>

<p>MagickSetSize() sets the size of the magick wand.  Set it before you read a raw image format such as RGB, GRAY, or CMYK.</p>

<p>The format of the MagickSetSize method is:</p>

<pre class="text">
MagickBooleanType MagickSetSize(MagickWand *wand,
  const size_t columns,const size_t rows)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the width in pixels. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the rows in pixels. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetSizeOffset">MagickSetSizeOffset</a></h2>

<p>MagickSetSizeOffset() sets the size and offset of the magick wand.  Set it before you read a raw image format such as RGB, GRAY, or CMYK.</p>

<p>The format of the MagickSetSizeOffset method is:</p>

<pre class="text">
MagickBooleanType MagickSetSizeOffset(MagickWand *wand,
  const size_t columns,const size_t rows,
  const ssize_t offset)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>columns</dt>
<dd>the image width in pixels. </dd>

<dd> </dd>
<dt>rows</dt>
<dd>the image rows in pixels. </dd>

<dd> </dd>
<dt>offset</dt>
<dd>the image offset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickWand/magick-property_8c.html" id="MagickSetType">MagickSetType</a></h2>

<p>MagickSetType() sets the image type attribute.</p>

<p>The format of the MagickSetType method is:</p>

<pre class="text">
MagickBooleanType MagickSetType(MagickWand *wand,
  const ImageType image_type)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the magick wand. </dd>

<dd> </dd>
<dt>image_type</dt>
<dd>the image type:   UndefinedType, BilevelType, GrayscaleType, GrayscaleAlphaType, PaletteType, PaletteAlphaType, TrueColorType, TrueColorAlphaType, ColorSeparationType, ColorSeparationAlphaType, or OptimizeType. </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="magick-property.php#">Back to top</a> •
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

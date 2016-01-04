



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Get or Set Image Channels</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, get, or, set, image, channels, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="channel.php#ChannelFxImage">ChannelFxImage</a> &bull; <a href="channel.php#CombineImages">CombineImages</a> &bull; <a href="channel.php#GetImageAlphaChannel">GetImageAlphaChannel</a> &bull; <a href="channel.php#SeparateImage">SeparateImage</a> &bull; <a href="channel.php#SeparateImages">SeparateImages</a> &bull; <a href="channel.php#SetImageAlphaChannel">SetImageAlphaChannel</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/channel_8c.html" id="ChannelFxImage">ChannelFxImage</a></h2>

<p>ChannelFxImage() applies a channel expression to the specified image.  The expression consists of one or more channels, either mnemonic or numeric (e.g. red, 1), separated by actions as follows:</p>

<dd>
</dd>

<dd> &lt;=&gt;     exchange two channels (e.g. red&lt;=&gt;blue) =&gt;      copy one channel to another channel (e.g. red=&gt;green) =       assign a constant value to a channel (e.g. red=50) ,       write new image channels in the specified order (e.g. red, green) |       add a new output image for the next set of channel operations ;       move to the next input image for the source of channel data </dd>

<dd> For example, to create 3 grayscale images from the red, green, and blue channels of an image, use: </dd>

<pre class="text">
    -channel-fx "red; green; blue"
</pre>

<p>A channel without an operation symbol implies separate (i.e, semicolon). </dd>

<dd> The format of the ChannelFxImage method is: </dd>

<pre class="text">
Image *ChannelFxImage(const Image *image,const char *expression,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows: </dd>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>expression</dt>
<dd>A channel expression. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/channel_8c.html" id="CombineImages">CombineImages</a></h2>

<p>CombineImages() combines one or more images into a single image.  The grayscale value of the pixels of each image in the sequence is assigned in order to the specified channels of the combined image.   The typical ordering would be image 1 =&gt; Red, 2 =&gt; Green, 3 =&gt; Blue, etc.</p>

<p>The format of the CombineImages method is:</p>

<pre class="text">
Image *CombineImages(const Image *images,const ColorspaceType colorspace,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>images</dt>
<dd>the image sequence. </dd>

<dd> </dd>
<dt>colorspace</dt>
<dd>the image colorspace. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/channel_8c.html" id="GetImageAlphaChannel">GetImageAlphaChannel</a></h2>

<p>GetImageAlphaChannel() returns MagickFalse if the image alpha channel is not activated.  That is, the image is RGB rather than RGBA or CMYK rather than CMYKA.</p>

<p>The format of the GetImageAlphaChannel method is:</p>

<pre class="text">
MagickBooleanType GetImageAlphaChannel(const Image *image)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/channel_8c.html" id="SeparateImage">SeparateImage</a></h2>

<p>SeparateImage() separates a channel from the image and returns it as a grayscale image.</p>

<p>The format of the SeparateImage method is:</p>

<pre class="text">
Image *SeparateImage(const Image *image,const ChannelType channel,
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
<dd>the image channel. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/channel_8c.html" id="SeparateImages">SeparateImages</a></h2>

<p>SeparateImages() returns a separate grayscale image for each channel specified.</p>

<p>The format of the SeparateImages method is:</p>

<pre class="text">
Image *SeparateImages(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/channel_8c.html" id="SetImageAlphaChannel">SetImageAlphaChannel</a></h2>

<p>SetImageAlphaChannel() activates, deactivates, resets, or sets the alpha channel.</p>

<p>The format of the SetImageAlphaChannel method is:</p>

<pre class="text">
MagickBooleanType SetImageAlphaChannel(Image *image,
  const AlphaChannelOption alpha_type,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>alpha_type</dt>
<dd> The alpha channel type: ActivateAlphaChannel, AssociateAlphaChannel, CopyAlphaChannel, DeactivateAlphaChannel, DisassociateAlphaChannel,  ExtractAlphaChannel, OffAlphaChannel, OnAlphaChannel, OpaqueAlphaChannel, SetAlphaChannel, ShapeAlphaChannel, and TransparentAlphaChannel. </dd>

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
    <p><a href="channel.php#">Back to top</a> •
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

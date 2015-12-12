



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Add an Effect</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, add, an, effect, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="effect.php#AdaptiveBlurImage">AdaptiveBlurImage</a> &bull; <a href="effect.php#AdaptiveSharpenImage">AdaptiveSharpenImage</a> &bull; <a href="effect.php#BlurImage">BlurImage</a> &bull; <a href="effect.php#ConvolveImage">ConvolveImage</a> &bull; <a href="effect.php#DespeckleImage">DespeckleImage</a> &bull; <a href="effect.php#EdgeImage">EdgeImage</a> &bull; <a href="effect.php#EmbossImage">EmbossImage</a> &bull; <a href="effect.php#GaussianBlurImage">GaussianBlurImage</a> &bull; <a href="effect.php#KuwaharaImage">KuwaharaImage</a> &bull; <a href="effect.php#MotionBlurImage">MotionBlurImage</a> &bull; <a href="effect.php#PreviewImage">PreviewImage</a> &bull; <a href="effect.php#RotationalBlurImage">RotationalBlurImage</a> &bull; <a href="effect.php#SelectiveBlurImage">SelectiveBlurImage</a> &bull; <a href="effect.php#ShadeImage">ShadeImage</a> &bull; <a href="effect.php#SharpenImage">SharpenImage</a> &bull; <a href="effect.php#SpreadImage">SpreadImage</a> &bull; <a href="effect.php#UnsharpMaskImage">UnsharpMaskImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="AdaptiveBlurImage">AdaptiveBlurImage</a></h2>

<p>AdaptiveBlurImage() adaptively blurs the image by blurring less intensely near image edges and more intensely far from edges.  We blur the image with a Gaussian operator of the given radius and standard deviation (sigma).  For reasonable results, radius should be larger than sigma.  Use a radius of 0 and AdaptiveBlurImage() selects a suitable radius for you.</p>

<p>The format of the AdaptiveBlurImage method is:</p>

<pre class="text">
Image *AdaptiveBlurImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Laplacian, in pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="AdaptiveSharpenImage">AdaptiveSharpenImage</a></h2>

<p>AdaptiveSharpenImage() adaptively sharpens the image by sharpening more intensely near image edges and less intensely far from edges. We sharpen the image with a Gaussian operator of the given radius and standard deviation (sigma).  For reasonable results, radius should be larger than sigma.  Use a radius of 0 and AdaptiveSharpenImage() selects a suitable radius for you.</p>

<p>The format of the AdaptiveSharpenImage method is:</p>

<pre class="text">
Image *AdaptiveSharpenImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Laplacian, in pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="BlurImage">BlurImage</a></h2>

<p>BlurImage() blurs an image.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma).  For reasonable results, the radius should be larger than sigma.  Use a radius of 0 and BlurImage() selects a suitable radius for you.</p>

<p>The format of the BlurImage method is:</p>

<pre class="text">
Image *BlurImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="ConvolveImage">ConvolveImage</a></h2>

<p>ConvolveImage() applies a custom convolution kernel to the image.</p>

<p>The format of the ConvolveImage method is:</p>

<pre class="text">
Image *ConvolveImage(const Image *image,const KernelInfo *kernel,
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
<dt>kernel</dt>
<dd>the filtering kernel. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="DespeckleImage">DespeckleImage</a></h2>

<p>DespeckleImage() reduces the speckle noise in an image while perserving the edges of the original image.  A speckle removing filter uses a complementary   hulling technique (raising pixels that are darker than their surrounding neighbors, then complementarily lowering pixels that are brighter than their surrounding neighbors) to reduce the speckle index of that image (reference Crimmins speckle removal).</p>

<p>The format of the DespeckleImage method is:</p>

<pre class="text">
Image *DespeckleImage(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="EdgeImage">EdgeImage</a></h2>

<p>EdgeImage() finds edges in an image.  Radius defines the radius of the convolution filter.  Use a radius of 0 and EdgeImage() selects a suitable radius for you.</p>

<p>The format of the EdgeImage method is:</p>

<pre class="text">
Image *EdgeImage(const Image *image,const double radius,
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
<dt>radius</dt>
<dd>the radius of the pixel neighborhood. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="EmbossImage">EmbossImage</a></h2>

<p>EmbossImage() returns a grayscale image with a three-dimensional effect. We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma).  For reasonable results, radius should be larger than sigma.  Use a radius of 0 and Emboss() selects a suitable radius for you.</p>

<p>The format of the EmbossImage method is:</p>

<pre class="text">
Image *EmbossImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the pixel neighborhood. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="GaussianBlurImage">GaussianBlurImage</a></h2>

<p>GaussianBlurImage() blurs an image.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma). For reasonable results, the radius should be larger than sigma.  Use a radius of 0 and GaussianBlurImage() selects a suitable radius for you</p>

<p>The format of the GaussianBlurImage method is:</p>

<pre class="text">
Image *GaussianBlurImage(const Image *image,onst double radius,
  const double sigma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="KuwaharaImage">KuwaharaImage</a></h2>

<p>KuwaharaImage() is an edge preserving noise reduction filter.</p>

<p>The format of the KuwaharaImage method is:</p>

<pre class="text">
Image *KuwaharaImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the square window radius. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="MotionBlurImage">MotionBlurImage</a></h2>

<p>MotionBlurImage() simulates motion blur.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma). For reasonable results, radius should be larger than sigma.  Use a radius of 0 and MotionBlurImage() selects a suitable radius for you. Angle gives the angle of the blurring motion.</p>

<p>Andrew Protano contributed this effect.</p>

<p>The format of the MotionBlurImage method is:</p>

<pre class="text">
    Image *MotionBlurImage(const Image *image,const double radius,
const double sigma,const double angle,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>angle</dt>
<dd>Apply the effect along this angle. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="PreviewImage">PreviewImage</a></h2>

<p>PreviewImage() tiles 9 thumbnails of the specified image with an image processing operation applied with varying parameters.  This may be helpful pin-pointing an appropriate parameter for a particular image processing operation.</p>

<p>The format of the PreviewImages method is:</p>

<pre class="text">
Image *PreviewImages(const Image *image,const PreviewType preview,
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
<dt>preview</dt>
<dd>the image processing operation. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="RotationalBlurImage">RotationalBlurImage</a></h2>

<p>RotationalBlurImage() applies a radial blur to the image.</p>

<p>Andrew Protano contributed this effect.</p>

<p>The format of the RotationalBlurImage method is:</p>

<pre class="text">
    Image *RotationalBlurImage(const Image *image,const double angle,
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
<dt>angle</dt>
<dd>the angle of the radial blur. </dd>

<dd> </dd>
<dt>blur</dt>
<dd>the blur. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="SelectiveBlurImage">SelectiveBlurImage</a></h2>

<p>SelectiveBlurImage() selectively blur pixels within a contrast threshold. It is similar to the unsharpen mask that sharpens everything with contrast above a certain threshold.</p>

<p>The format of the SelectiveBlurImage method is:</p>

<pre class="text">
Image *SelectiveBlurImage(const Image *image,const double radius,
  const double sigma,const double threshold,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd>only pixels within this contrast threshold are included in the blur operation. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="ShadeImage">ShadeImage</a></h2>

<p>ShadeImage() shines a distant light on an image to create a three-dimensional effect. You control the positioning of the light with azimuth and elevation; azimuth is measured in degrees off the x axis and elevation is measured in pixels above the Z axis.</p>

<p>The format of the ShadeImage method is:</p>

<pre class="text">
Image *ShadeImage(const Image *image,const MagickBooleanType gray,
  const double azimuth,const double elevation,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>gray</dt>
<dd>A value other than zero shades the intensity of each pixel. </dd>

<dd> </dd>
<dt>azimuth, elevation</dt>
<dd> Define the light source direction. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="SharpenImage">SharpenImage</a></h2>

<p>SharpenImage() sharpens the image.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma).  For reasonable results, radius should be larger than sigma.  Use a radius of 0 and SharpenImage() selects a suitable radius for you.</p>

<p>Using a separable kernel would be faster, but the negative weights cancel out on the corners of the kernel producing often undesirable ringing in the filtered result; this can be avoided by using a 2D gaussian shaped image sharpening kernel instead.</p>

<p>The format of the SharpenImage method is:</p>

<pre class="text">
    Image *SharpenImage(const Image *image,const double radius,
const double sigma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Laplacian, in pixels. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="SpreadImage">SpreadImage</a></h2>

<p>SpreadImage() is a special effects method that randomly displaces each pixel in a square area defined by the radius parameter.</p>

<p>The format of the SpreadImage method is:</p>

<pre class="text">
Image *SpreadImage(const Image *image,
  const PixelInterpolateMethod method,const double radius,
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
<dt>method</dt>
<dd> intepolation method. </dd>

<dd> </dd>
<dt>radius</dt>
<dd> choose a random pixel in a neighborhood of this extent. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/effect_8c.html" id="UnsharpMaskImage">UnsharpMaskImage</a></h2>

<p>UnsharpMaskImage() sharpens one or more image channels.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma).  For reasonable results, radius should be larger than sigma.  Use a radius of 0 and UnsharpMaskImage() selects a suitable radius for you.</p>

<p>The format of the UnsharpMaskImage method is:</p>

<pre class="text">
    Image *UnsharpMaskImage(const Image *image,const double radius,
const double sigma,const double amount,const double threshold,
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
<dt>radius</dt>
<dd>the radius of the Gaussian, in pixels, not counting the center pixel. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>gain</dt>
<dd>the percentage of the difference between the original and the blur image that is added back into the original. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd>the threshold in pixels needed to apply the diffence gain. </dd>

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
    <p><a href="effect.php#">Back to top</a> •
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

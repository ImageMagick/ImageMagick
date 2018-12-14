



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Add an Effect @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, add, an, effect, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/effect.php" rel="canonical">
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
<p class="text-center"><a href="effect.php#AdaptiveBlurImage">AdaptiveBlurImage</a> &bull; <a href="effect.php#AdaptiveSharpenImage">AdaptiveSharpenImage</a> &bull; <a href="effect.php#BlurImage">BlurImage</a> &bull; <a href="effect.php#ConvolveImage">ConvolveImage</a> &bull; <a href="effect.php#DespeckleImage">DespeckleImage</a> &bull; <a href="effect.php#EdgeImage">EdgeImage</a> &bull; <a href="effect.php#EmbossImage">EmbossImage</a> &bull; <a href="effect.php#GaussianBlurImage">GaussianBlurImage</a> &bull; <a href="effect.php#KuwaharaImage">KuwaharaImage</a> &bull; <a href="effect.php#LocalContrastImage">LocalContrastImage</a> &bull; <a href="effect.php#MotionBlurImage">MotionBlurImage</a> &bull; <a href="effect.php#PreviewImage">PreviewImage</a> &bull; <a href="effect.php#RotationalBlurImage">RotationalBlurImage</a> &bull; <a href="effect.php#SelectiveBlurImage">SelectiveBlurImage</a> &bull; <a href="effect.php#ShadeImage">ShadeImage</a> &bull; <a href="effect.php#SharpenImage">SharpenImage</a> &bull; <a href="effect.php#SpreadImage">SpreadImage</a> &bull; <a href="effect.php#UnsharpMaskImage">UnsharpMaskImage</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="AdaptiveBlurImage">AdaptiveBlurImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="AdaptiveSharpenImage">AdaptiveSharpenImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="BlurImage">BlurImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="ConvolveImage">ConvolveImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="DespeckleImage">DespeckleImage</a></h2>

<p>DespeckleImage() reduces the speckle noise in an image while perserving the edges of the original image.  A speckle removing filter uses a complementary hulling technique (raising pixels that are darker than their surrounding neighbors, then complementarily lowering pixels that are brighter than their surrounding neighbors) to reduce the speckle index of that image (reference Crimmins speckle removal).</p>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="EdgeImage">EdgeImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="EmbossImage">EmbossImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="GaussianBlurImage">GaussianBlurImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="KuwaharaImage">KuwaharaImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="LocalContrastImage">LocalContrastImage</a></h2>

<p>LocalContrastImage() attempts to increase the appearance of large-scale light-dark transitions. Local contrast enhancement works similarly to sharpening with an unsharp mask, however the mask is instead created using an image with a greater blur distance.</p>

<p>The format of the LocalContrastImage method is:</p>

<pre class="text">
Image *LocalContrastImage(const Image *image, const double radius,
  const double strength,ExceptionInfo *exception)
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
<dd>the radius of the Gaussian blur, in percentage with 100 resulting in a blur radius of 20 of largest dimension. </dd>

<dd> </dd>
<dt>strength</dt>
<dd>the strength of the blur mask in percentage. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="MotionBlurImage">MotionBlurImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="PreviewImage">PreviewImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="RotationalBlurImage">RotationalBlurImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="SelectiveBlurImage">SelectiveBlurImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="ShadeImage">ShadeImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="SharpenImage">SharpenImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="SpreadImage">SpreadImage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/effect_8c.html" id="UnsharpMaskImage">UnsharpMaskImage</a></h2>

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
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="effect.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
    &nbsp; &nbsp;
    <a href="http://pgp.mit.edu/pks/lookup?op=get&amp;search=0x89AB63D48277377A">Public Key</a> •
    <a href="https://imagemagick.org/script/support.php">Donate</a> •
    <a href="https://imagemagick.org/script/contact.php">Contact Us</a>
    <br/>
        <small>© 1999-2019 ImageMagick Studio LLC</small></p>
  </footer>

  <!-- Javascript assets -->
  <script src="https://imagemagick.org/assets/magick-js.php" crossorigin="anonymous"></script>
  <script>window.jQuery || document.write('<script src="https://imagemagick.org/assets/jquery.min.js"><\/script>')</script>
</body>
</html>
<!-- Magick Cache 2nd September 2018 11:26 -->
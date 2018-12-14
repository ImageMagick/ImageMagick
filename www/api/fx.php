



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Add a Special Effect @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, add, a, special, effect, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/fx.php" rel="canonical">
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
<p class="text-center"><a href="fx.php#AddNoiseImage">AddNoiseImage</a> &bull; <a href="fx.php#BlueShiftImage">BlueShiftImage</a> &bull; <a href="fx.php#CharcoalImage">CharcoalImage</a> &bull; <a href="fx.php#ColorizeImage">ColorizeImage</a> &bull; <a href="fx.php#ColorMatrixImage">ColorMatrixImage</a> &bull; <a href="fx.php#FxImage">FxImage</a> &bull; <a href="fx.php#ImplodeImage">ImplodeImage</a> &bull; <a href="fx.php#The MorphImages">The MorphImages</a> &bull; <a href="fx.php#PlasmaImage">PlasmaImage</a> &bull; <a href="fx.php#PolaroidImage">PolaroidImage</a> &bull; <a href="fx.php#MagickSepiaToneImage">MagickSepiaToneImage</a> &bull; <a href="fx.php#ShadowImage">ShadowImage</a> &bull; <a href="fx.php#SketchImage">SketchImage</a> &bull; <a href="fx.php#SolarizeImage">SolarizeImage</a> &bull; <a href="fx.php#SteganoImage">SteganoImage</a> &bull; <a href="fx.php#StereoAnaglyphImage">StereoAnaglyphImage</a> &bull; <a href="fx.php#SwirlImage">SwirlImage</a> &bull; <a href="fx.php#TintImage">TintImage</a> &bull; <a href="fx.php#VignetteImage">VignetteImage</a> &bull; <a href="fx.php#WaveImage">WaveImage</a> &bull; <a href="fx.php#WaveletDenoiseImage">WaveletDenoiseImage</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="AddNoiseImage">AddNoiseImage</a></h2>

<p>AddNoiseImage() adds random noise to the image.</p>

<p>The format of the AddNoiseImage method is:</p>

<pre class="text">
Image *AddNoiseImage(const Image *image,const NoiseType noise_type,
  const double attenuate,ExceptionInfo *exception)
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
<dd>the channel type. </dd>

<dd> </dd>
<dt>noise_type</dt>
<dd> The type of noise: Uniform, Gaussian, Multiplicative, Impulse, Laplacian, or Poisson. </dd>

<dd> </dd>
<dt>attenuate</dt>
<dd> attenuate the random distribution. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="BlueShiftImage">BlueShiftImage</a></h2>

<p>BlueShiftImage() mutes the colors of the image to simulate a scene at nighttime in the moonlight.</p>

<p>The format of the BlueShiftImage method is:</p>

<pre class="text">
Image *BlueShiftImage(const Image *image,const double factor,
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
<dt>factor</dt>
<dd>the shift factor. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="CharcoalImage">CharcoalImage</a></h2>

<p>CharcoalImage() creates a new image that is a copy of an existing one with the edge highlighted.  It allocates the memory necessary for the new Image structure and returns a pointer to the new image.</p>

<p>The format of the CharcoalImage method is:</p>

<pre class="text">
Image *CharcoalImage(const Image *image,const double radius,
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
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="ColorizeImage">ColorizeImage</a></h2>

<p>ColorizeImage() blends the fill color with each pixel in the image. A percentage blend is specified with opacity.  Control the application of different color components by specifying a different percentage for each component (e.g. 90/100/10 is 90 red, 100 green, and 10 blue).</p>

<p>The format of the ColorizeImage method is:</p>

<pre class="text">
Image *ColorizeImage(const Image *image,const char *blend,
  const PixelInfo *colorize,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>blend</dt>
<dd> A character string indicating the level of blending as a percentage. </dd>

<dd> </dd>
<dt>colorize</dt>
<dd>A color value. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="ColorMatrixImage">ColorMatrixImage</a></h2>

<p>ColorMatrixImage() applies color transformation to an image. This method permits saturation changes, hue rotation, luminance to alpha, and various other effects.  Although variable-sized transformation matrices can be used, typically one uses a 5x5 matrix for an RGBA image and a 6x6 for CMYKA (or RGBA with offsets).  The matrix is similar to those used by Adobe Flash except offsets are in column 6 rather than 5 (in support of CMYKA images) and offsets are normalized (divide Flash offset by 255).</p>

<p>The format of the ColorMatrixImage method is:</p>

<pre class="text">
Image *ColorMatrixImage(const Image *image,
  const KernelInfo *color_matrix,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>color_matrix</dt>
<dd> the color matrix. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="FxImage">FxImage</a></h2>

<p>FxImage() applies a mathematical expression to the specified image.</p>

<p>The format of the FxImage method is:</p>

<pre class="text">
Image *FxImage(const Image *image,const char *expression,
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
<dt>expression</dt>
<dd>A mathematical expression. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="ImplodeImage">ImplodeImage</a></h2>

<p>ImplodeImage() creates a new image that is a copy of an existing one with the image pixels "implode" by the specified percentage.  It allocates the memory necessary for the new Image structure and returns a pointer to the new image.</p>

<p>The format of the ImplodeImage method is:</p>

<pre class="text">
Image *ImplodeImage(const Image *image,const double amount,
  const PixelInterpolateMethod method,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>implode_image</dt>
<dd>Method ImplodeImage returns a pointer to the image after it is implode.  A null image is returned if there is a memory shortage. </dd>

<dd> </dd>
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>amount</dt>
<dd> Define the extent of the implosion. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the pixel interpolation method. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="The_MorphImages">The MorphImages</a></h2>

<p>The MorphImages() method requires a minimum of two images.  The first image is transformed into the second by a number of intervening images as specified by frames.</p>

<p>The format of the MorphImage method is:</p>

<pre class="text">
Image *MorphImages(const Image *image,const size_t number_frames,
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
<dt>number_frames</dt>
<dd> Define the number of in-between image to generate. The more in-between frames, the smoother the morph. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="PlasmaImage">PlasmaImage</a></h2>

<p>PlasmaImage() initializes an image with plasma fractal values.  The image must be initialized with a base color and the random number generator seeded before this method is called.</p>

<p>The format of the PlasmaImage method is:</p>

<pre class="text">
MagickBooleanType PlasmaImage(Image *image,const SegmentInfo *segment,
  size_t attenuate,size_t depth,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>segment</dt>
<dd>  Define the region to apply plasma fractals values. </dd>

<dd> </dd>
<dt>attenuate</dt>
<dd>Define the plasma attenuation factor. </dd>

<dd> </dd>
<dt>depth</dt>
<dd>Limit the plasma recursion depth. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="PolaroidImage">PolaroidImage</a></h2>

<p>PolaroidImage() simulates a Polaroid picture.</p>

<p>The format of the PolaroidImage method is:</p>

<pre class="text">
Image *PolaroidImage(const Image *image,const DrawInfo *draw_info,
  const char *caption,const double angle,
  const PixelInterpolateMethod method,ExceptionInfo exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>draw_info</dt>
<dd>the draw info. </dd>

<dd> </dd>
<dt>caption</dt>
<dd>the Polaroid caption. </dd>

<dd> </dd>
<dt>angle</dt>
<dd>Apply the effect along this angle. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the pixel interpolation method. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="MagickSepiaToneImage">MagickSepiaToneImage</a></h2>

<p>MagickSepiaToneImage() applies a special effect to the image, similar to the effect achieved in a photo darkroom by sepia toning.  Threshold ranges from 0 to QuantumRange and is a measure of the extent of the sepia toning.  A threshold of 80 is a good starting point for a reasonable tone.</p>

<p>The format of the SepiaToneImage method is:</p>

<pre class="text">
Image *SepiaToneImage(const Image *image,const double threshold,
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
<dt>threshold</dt>
<dd>the tone threshold. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="ShadowImage">ShadowImage</a></h2>

<p>ShadowImage() simulates a shadow from the specified image and returns it.</p>

<p>The format of the ShadowImage method is:</p>

<pre class="text">
Image *ShadowImage(const Image *image,const double alpha,
  const double sigma,const ssize_t x_offset,const ssize_t y_offset,
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
<dt>alpha</dt>
<dd>percentage transparency. </dd>

<dd> </dd>
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>x_offset</dt>
<dd>the shadow x-offset. </dd>

<dd> </dd>
<dt>y_offset</dt>
<dd>the shadow y-offset. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="SketchImage">SketchImage</a></h2>

<p>SketchImage() simulates a pencil sketch.  We convolve the image with a Gaussian operator of the given radius and standard deviation (sigma).  For reasonable results, radius should be larger than sigma.  Use a radius of 0 and SketchImage() selects a suitable radius for you.  Angle gives the angle of the sketch.</p>

<p>The format of the SketchImage method is:</p>

<pre class="text">
    Image *SketchImage(const Image *image,const double radius,
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
<dd>apply the effect along this angle. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="SolarizeImage">SolarizeImage</a></h2>

<p>SolarizeImage() applies a special effect to the image, similar to the effect achieved in a photo darkroom by selectively exposing areas of photo sensitive paper to light.  Threshold ranges from 0 to QuantumRange and is a measure of the extent of the solarization.</p>

<p>The format of the SolarizeImage method is:</p>

<pre class="text">
MagickBooleanType SolarizeImage(Image *image,const double threshold,
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
<dt>threshold</dt>
<dd> Define the extent of the solarization. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="SteganoImage">SteganoImage</a></h2>

<p>SteganoImage() hides a digital watermark within the image.  Recover the hidden watermark later to prove that the authenticity of an image. Offset defines the start position within the image to hide the watermark.</p>

<p>The format of the SteganoImage method is:</p>

<pre class="text">
Image *SteganoImage(const Image *image,Image *watermark,
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
<dt>watermark</dt>
<dd>the watermark image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="StereoAnaglyphImage">StereoAnaglyphImage</a></h2>

<p>StereoAnaglyphImage() combines two images and produces a single image that is the composite of a left and right image of a stereo pair.  Special red-green stereo glasses are required to view this effect.</p>

<p>The format of the StereoAnaglyphImage method is:</p>

<pre class="text">
Image *StereoImage(const Image *left_image,const Image *right_image,
  ExceptionInfo *exception)
Image *StereoAnaglyphImage(const Image *left_image,
  const Image *right_image,const ssize_t x_offset,const ssize_t y_offset,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>left_image</dt>
<dd>the left image. </dd>

<dd> </dd>
<dt>right_image</dt>
<dd>the right image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd> </dd>
<dt>x_offset</dt>
<dd>amount, in pixels, by which the left image is offset to the right of the right image. </dd>

<dd> </dd>
<dt>y_offset</dt>
<dd>amount, in pixels, by which the left image is offset to the bottom of the right image. </dd>

<dd> </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="SwirlImage">SwirlImage</a></h2>

<p>SwirlImage() swirls the pixels about the center of the image, where degrees indicates the sweep of the arc through which each pixel is moved. You get a more dramatic effect as the degrees move from 1 to 360.</p>

<p>The format of the SwirlImage method is:</p>

<pre class="text">
Image *SwirlImage(const Image *image,double degrees,
  const PixelInterpolateMethod method,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>degrees</dt>
<dd>Define the tightness of the swirling effect. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the pixel interpolation method. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="TintImage">TintImage</a></h2>

<p>TintImage() applies a color vector to each pixel in the image.  The length of the vector is 0 for black and white and at its maximum for the midtones. The vector weighting function is f(x)=(1-(4.0*((x-0.5)*(x-0.5))))</p>

<p>The format of the TintImage method is:</p>

<pre class="text">
Image *TintImage(const Image *image,const char *blend,
  const PixelInfo *tint,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>blend</dt>
<dd>A color value used for tinting. </dd>

<dd> </dd>
<dt>tint</dt>
<dd>A color value used for tinting. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="VignetteImage">VignetteImage</a></h2>

<p>VignetteImage() softens the edges of the image in vignette style.</p>

<p>The format of the VignetteImage method is:</p>

<pre class="text">
Image *VignetteImage(const Image *image,const double radius,
  const double sigma,const ssize_t x,const ssize_t y,
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
<dt>sigma</dt>
<dd>the standard deviation of the Gaussian, in pixels. </dd>

<dd> </dd>
<dt>x, y</dt>
<dd> Define the x and y ellipse offset. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="WaveImage">WaveImage</a></h2>

<p>WaveImage() creates a "ripple" effect in the image by shifting the pixels vertically along a sine wave whose amplitude and wavelength is specified by the given parameters.</p>

<p>The format of the WaveImage method is:</p>

<pre class="text">
Image *WaveImage(const Image *image,const double amplitude,
  const double wave_length,const PixelInterpolateMethod method,
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
<dt>amplitude, wave_length</dt>
<dd> Define the amplitude and wave length of the sine wave. </dd>

<dd> </dd>
<dt>interpolate</dt>
<dd>the pixel interpolation method. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickCore/fx_8c.html" id="WaveletDenoiseImage">WaveletDenoiseImage</a></h2>

<p>WaveletDenoiseImage() removes noise from the image using a wavelet transform.  The wavelet transform is a fast hierarchical scheme for processing an image using a set of consecutive lowpass and high_pass filters, followed by a decimation.  This results in a decomposition into different scales which can be regarded as different “frequency bands”, determined by the mother wavelet.  Adapted from dcraw.c by David Coffin.</p>

<p>The format of the WaveletDenoiseImage method is:</p>

<pre class="text">
Image *WaveletDenoiseImage(const Image *image,const double threshold,
  const double softness,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>threshold</dt>
<dd>set the threshold for smoothing. </dd>

<dd> </dd>
<dt>softness</dt>
<dd>attenuate the smoothing threshold. </dd>

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
    <a href="fx.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 3rd September 2018 09:38 -->
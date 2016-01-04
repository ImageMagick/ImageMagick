



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Add a Special Effect</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, add, a, special, effect, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="fx.php#AddNoiseImage">AddNoiseImage</a> &bull; <a href="fx.php#BlueShiftImage">BlueShiftImage</a> &bull; <a href="fx.php#CharcoalImage">CharcoalImage</a> &bull; <a href="fx.php#ColorizeImage">ColorizeImage</a> &bull; <a href="fx.php#ColorMatrixImage">ColorMatrixImage</a> &bull; <a href="fx.php#FxImage">FxImage</a> &bull; <a href="fx.php#ImplodeImage">ImplodeImage</a> &bull; <a href="fx.php#The MorphImages">The MorphImages</a> &bull; <a href="fx.php#PlasmaImage">PlasmaImage</a> &bull; <a href="fx.php#PolaroidImage">PolaroidImage</a> &bull; <a href="fx.php#MagickSepiaToneImage">MagickSepiaToneImage</a> &bull; <a href="fx.php#ShadowImage">ShadowImage</a> &bull; <a href="fx.php#SketchImage">SketchImage</a> &bull; <a href="fx.php#SolarizeImage">SolarizeImage</a> &bull; <a href="fx.php#SteganoImage">SteganoImage</a> &bull; <a href="fx.php#StereoAnaglyphImage">StereoAnaglyphImage</a> &bull; <a href="fx.php#SwirlImage">SwirlImage</a> &bull; <a href="fx.php#TintImage">TintImage</a> &bull; <a href="fx.php#VignetteImage">VignetteImage</a> &bull; <a href="fx.php#WaveImage">WaveImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="AddNoiseImage">AddNoiseImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="BlueShiftImage">BlueShiftImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="CharcoalImage">CharcoalImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="ColorizeImage">ColorizeImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="ColorMatrixImage">ColorMatrixImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="FxImage">FxImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="ImplodeImage">ImplodeImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="The_MorphImages">The MorphImages</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="PlasmaImage">PlasmaImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="PolaroidImage">PolaroidImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="MagickSepiaToneImage">MagickSepiaToneImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="ShadowImage">ShadowImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="SketchImage">SketchImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="SolarizeImage">SolarizeImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="SteganoImage">SteganoImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="StereoAnaglyphImage">StereoAnaglyphImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="SwirlImage">SwirlImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="TintImage">TintImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="VignetteImage">VignetteImage</a></h2>

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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/fx_8c.html" id="WaveImage">WaveImage</a></h2>

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
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="fx.php#">Back to top</a> •
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

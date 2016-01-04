



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Enhance an Image</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, enhance, an, image, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="enhance.php#AutoGammaImage">AutoGammaImage</a> &bull; <a href="enhance.php#AutoLevelImage">AutoLevelImage</a> &bull; <a href="enhance.php#BrightnessContrastImage">BrightnessContrastImage</a> &bull; <a href="enhance.php#ClutImage">ClutImage</a> &bull; <a href="enhance.php#ColorDecisionListImage">ColorDecisionListImage</a> &bull; <a href="enhance.php#ContrastImage">ContrastImage</a> &bull; <a href="enhance.php#ContrastStretchImage">ContrastStretchImage</a> &bull; <a href="enhance.php#EnhanceImage">EnhanceImage</a> &bull; <a href="enhance.php#EqualizeImage">EqualizeImage</a> &bull; <a href="enhance.php#GammaImage">GammaImage</a> &bull; <a href="enhance.php#GrayscaleImage">GrayscaleImage</a> &bull; <a href="enhance.php#HaldClutImage">HaldClutImage</a> &bull; <a href="enhance.php#LevelImage">LevelImage</a> &bull; <a href="enhance.php#LevelizeImage">LevelizeImage</a> &bull; <a href="enhance.php#LevelImageColors">LevelImageColors</a> &bull; <a href="enhance.php#LinearStretchImage">LinearStretchImage</a> &bull; <a href="enhance.php#ModulateImage">ModulateImage</a> &bull; <a href="enhance.php#NegateImage">NegateImage</a> &bull; <a href="enhance.php#The NormalizeImage">The NormalizeImage</a> &bull; <a href="enhance.php#SigmoidalContrastImage">SigmoidalContrastImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="AutoGammaImage">AutoGammaImage</a></h2>

<p>AutoGammaImage() extract the 'mean' from the image and adjust the image to try make set its gamma appropriatally.</p>

<p>The format of the AutoGammaImage method is:</p>

<pre class="text">
MagickBooleanType AutoGammaImage(Image *image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>The image to auto-level </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="AutoLevelImage">AutoLevelImage</a></h2>

<p>AutoLevelImage() adjusts the levels of a particular image channel by scaling the minimum and maximum values to the full quantum range.</p>

<p>The format of the LevelImage method is:</p>

<pre class="text">
MagickBooleanType AutoLevelImage(Image *image,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>The image to auto-level </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="BrightnessContrastImage">BrightnessContrastImage</a></h2>

<p>BrightnessContrastImage() changes the brightness and/or contrast of an image.  It converts the brightness and contrast parameters into slope and intercept and calls a polynomical function to apply to the image.</p>

<p>The format of the BrightnessContrastImage method is:</p>

<pre class="text">
MagickBooleanType BrightnessContrastImage(Image *image,
  const double brightness,const double contrast,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>brightness</dt>
<dd>the brightness percent (-100 .. 100). </dd>

<dd> </dd>
<dt>contrast</dt>
<dd>the contrast percent (-100 .. 100). </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="ClutImage">ClutImage</a></h2>

<p>ClutImage() replaces each color value in the given image, by using it as an index to lookup a replacement color value in a Color Look UP Table in the form of an image.  The values are extracted along a diagonal of the CLUT image so either a horizontal or vertial gradient image can be used.</p>

<p>Typically this is used to either re-color a gray-scale image according to a color gradient in the CLUT image, or to perform a freeform histogram (level) adjustment according to the (typically gray-scale) gradient in the CLUT image.</p>

<p>When the 'channel' mask includes the matte/alpha transparency channel but one image has no such channel it is assumed that that image is a simple gray-scale image that will effect the alpha channel values, either for gray-scale coloring (with transparent or semi-transparent colors), or a histogram adjustment of existing alpha channel values.   If both images have matte channels, direct and normal indexing is applied, which is rarely used.</p>

<p>The format of the ClutImage method is:</p>

<pre class="text">
MagickBooleanType ClutImage(Image *image,Image *clut_image,
  const PixelInterpolateMethod method,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image, which is replaced by indexed CLUT values </dd>

<dd> </dd>
<dt>clut_image</dt>
<dd>the color lookup table image for replacement color values. </dd>

<dd> </dd>
<dt>method</dt>
<dd>the pixel interpolation method. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="ColorDecisionListImage">ColorDecisionListImage</a></h2>

<p>ColorDecisionListImage() accepts a lightweight Color Correction Collection (CCC) file which solely contains one or more color corrections and applies the correction to the image.  Here is a sample CCC file:</p>

<pre class="text">
    &lt;ColorCorrectionCollection xmlns="urn:ASC:CDL:v1.2"&gt;
    &lt;ColorCorrection id="cc03345"&gt;
          &lt;SOPNode&gt;
               &lt;Slope&gt; 0.9 1.2 0.5 &lt;/Slope&gt;
               &lt;Offset&gt; 0.4 -0.5 0.6 &lt;/Offset&gt;
               &lt;Power&gt; 1.0 0.8 1.5 &lt;/Power&gt;
          &lt;/SOPNode&gt;
          &lt;SATNode&gt;
               &lt;Saturation&gt; 0.85 &lt;/Saturation&gt;
          &lt;/SATNode&gt;
    &lt;/ColorCorrection&gt;
    &lt;/ColorCorrectionCollection&gt;
</pre>

<p>which includes the slop, offset, and power for each of the RGB channels as well as the saturation.</p>

<p>The format of the ColorDecisionListImage method is:</p>

<pre class="text">
MagickBooleanType ColorDecisionListImage(Image *image,
  const char *color_correction_collection,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>color_correction_collection</dt>
<dd>the color correction collection in XML. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="ContrastImage">ContrastImage</a></h2>

<p>ContrastImage() enhances the intensity differences between the lighter and darker elements of the image.  Set sharpen to a MagickTrue to increase the image contrast otherwise the contrast is reduced.</p>

<p>The format of the ContrastImage method is:</p>

<pre class="text">
MagickBooleanType ContrastImage(Image *image,
  const MagickBooleanType sharpen,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>sharpen</dt>
<dd>Increase or decrease image contrast. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="ContrastStretchImage">ContrastStretchImage</a></h2>

<p>ContrastStretchImage() is a simple image enhancement technique that attempts to improve the contrast in an image by 'stretching' the range of intensity values it contains to span a desired range of values. It differs from the more sophisticated histogram equalization in that it can only apply a linear scaling function to the image pixel values.  As a result the 'enhancement' is less harsh.</p>

<p>The format of the ContrastStretchImage method is:</p>

<pre class="text">
MagickBooleanType ContrastStretchImage(Image *image,
  const char *levels,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>black_point</dt>
<dd>the black point. </dd>

<dd> </dd>
<dt>white_point</dt>
<dd>the white point. </dd>

<dd> </dd>
<dt>levels</dt>
<dd>Specify the levels where the black and white points have the range of 0 to number-of-pixels (e.g. 1, 10x90, etc.). </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="EnhanceImage">EnhanceImage</a></h2>

<p>EnhanceImage() applies a digital filter that improves the quality of a noisy image.</p>

<p>The format of the EnhanceImage method is:</p>

<pre class="text">
Image *EnhanceImage(const Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="EqualizeImage">EqualizeImage</a></h2>

<p>EqualizeImage() applies a histogram equalization to the image.</p>

<p>The format of the EqualizeImage method is:</p>

<pre class="text">
MagickBooleanType EqualizeImage(Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="GammaImage">GammaImage</a></h2>

<p>GammaImage() gamma-corrects a particular image channel.  The same image viewed on different devices will have perceptual differences in the way the image's intensities are represented on the screen.  Specify individual gamma levels for the red, green, and blue channels, or adjust all three with the gamma parameter.  Values typically range from 0.8 to 2.3.</p>

<p>You can also reduce the influence of a particular channel with a gamma value of 0.</p>

<p>The format of the GammaImage method is:</p>

<pre class="text">
MagickBooleanType GammaImage(Image *image,const double gamma,
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
<dt>level</dt>
<dd>the image gamma as a string (e.g. 1.6,1.2,1.0). </dd>

<dd> </dd>
<dt>gamma</dt>
<dd>the image gamma. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="GrayscaleImage">GrayscaleImage</a></h2>

<p>GrayscaleImage() converts the image to grayscale.</p>

<p>The format of the GrayscaleImage method is:</p>

<pre class="text">
MagickBooleanType GrayscaleImage(Image *image,
  const PixelIntensityMethod method ,ExceptionInfo *exception)
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
<dd>the pixel intensity method. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="HaldClutImage">HaldClutImage</a></h2>

<p>HaldClutImage() applies a Hald color lookup table to the image.  A Hald color lookup table is a 3-dimensional color cube mapped to 2 dimensions. Create it with the HALD coder.  You can apply any color transformation to the Hald image and then use this method to apply the transform to the image.</p>

<p>The format of the HaldClutImage method is:</p>

<pre class="text">
MagickBooleanType HaldClutImage(Image *image,Image *hald_image,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image, which is replaced by indexed CLUT values </dd>

<dd> </dd>
<dt>hald_image</dt>
<dd>the color lookup table image for replacement color values. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="LevelImage">LevelImage</a></h2>

<p>LevelImage() adjusts the levels of a particular image channel by scaling the colors falling between specified white and black points to the full available quantum range.</p>

<p>The parameters provided represent the black, and white points.  The black point specifies the darkest color in the image. Colors darker than the black point are set to zero.  White point specifies the lightest color in the image.  Colors brighter than the white point are set to the maximum quantum value.</p>

<p>If a '!' flag is given, map black and white colors to the given levels rather than mapping those levels to black and white.  See LevelizeImage() below.</p>

<p>Gamma specifies a gamma correction to apply to the image.</p>

<p>The format of the LevelImage method is:</p>

<pre class="text">
MagickBooleanType LevelImage(Image *image,const double black_point,
  const double white_point,const double gamma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>black_point</dt>
<dd>The level to map zero (black) to. </dd>

<dd> </dd>
<dt>white_point</dt>
<dd>The level to map QuantumRange (white) to. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="LevelizeImage">LevelizeImage</a></h2>

<p>LevelizeImage() applies the reversed LevelImage() operation to just the specific channels specified.  It compresses the full range of color values, so that they lie between the given black and white points. Gamma is applied before the values are mapped.</p>

<p>LevelizeImage() can be called with by using a +level command line API option, or using a '!' on a -level or LevelImage() geometry string.</p>

<p>It can be used to de-contrast a greyscale image to the exact levels specified.  Or by using specific levels for each channel of an image you can convert a gray-scale image to any linear color gradient, according to those levels.</p>

<p>The format of the LevelizeImage method is:</p>

<pre class="text">
MagickBooleanType LevelizeImage(Image *image,const double black_point,
  const double white_point,const double gamma,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>black_point</dt>
<dd>The level to map zero (black) to. </dd>

<dd> </dd>
<dt>white_point</dt>
<dd>The level to map QuantumRange (white) to. </dd>

<dd> </dd>
<dt>gamma</dt>
<dd>adjust gamma by this factor before mapping values. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="LevelImageColors">LevelImageColors</a></h2>

<p>LevelImageColors() maps the given color to "black" and "white" values, linearly spreading out the colors, and level values on a channel by channel bases, as per LevelImage().  The given colors allows you to specify different level ranges for each of the color channels separately.</p>

<p>If the boolean 'invert' is set true the image values will modifyed in the reverse direction. That is any existing "black" and "white" colors in the image will become the color values given, with all other values compressed appropriatally.  This effectivally maps a greyscale gradient into the given color gradient.</p>

<p>The format of the LevelImageColors method is:</p>

<pre class="text">
    MagickBooleanType LevelImageColors(Image *image,
const PixelInfo *black_color,const PixelInfo *white_color,
const MagickBooleanType invert,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>black_color</dt>
<dd>The color to map black to/from </dd>

<dd> </dd>
<dt>white_point</dt>
<dd>The color to map white to/from </dd>

<dd> </dd>
<dt>invert</dt>
<dd>if true map the colors (levelize), rather than from (level) </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="LinearStretchImage">LinearStretchImage</a></h2>

<p>LinearStretchImage() discards any pixels below the black point and above the white point and levels the remaining pixels.</p>

<p>The format of the LinearStretchImage method is:</p>

<pre class="text">
MagickBooleanType LinearStretchImage(Image *image,
  const double black_point,const double white_point,
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
<dt>black_point</dt>
<dd>the black point. </dd>

<dd> </dd>
<dt>white_point</dt>
<dd>the white point. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="ModulateImage">ModulateImage</a></h2>

<p>ModulateImage() lets you control the brightness, saturation, and hue of an image.  Modulate represents the brightness, saturation, and hue as one parameter (e.g. 90,150,100).  If the image colorspace is HSL, the modulation is lightness, saturation, and hue.  For HWB, use blackness, whiteness, and hue. And for HCL, use chrome, luma, and hue.</p>

<p>The format of the ModulateImage method is:</p>

<pre class="text">
MagickBooleanType ModulateImage(Image *image,const char *modulate,
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
<dt>modulate</dt>
<dd>Define the percent change in brightness, saturation, and hue. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="NegateImage">NegateImage</a></h2>

<p>NegateImage() negates the colors in the reference image.  The grayscale option means that only grayscale values within the image are negated.</p>

<p>The format of the NegateImage method is:</p>

<pre class="text">
MagickBooleanType NegateImage(Image *image,
  const MagickBooleanType grayscale,ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the image. </dd>

<dd> </dd>
<dt>grayscale</dt>
<dd>If MagickTrue, only negate grayscale pixels within the image. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="The_NormalizeImage">The NormalizeImage</a></h2>

<p>The NormalizeImage() method enhances the contrast of a color image by mapping the darkest 2 percent of all pixel to black and the brightest 1 percent to white.</p>

<p>The format of the NormalizeImage method is:</p>

<pre class="text">
MagickBooleanType NormalizeImage(Image *image,ExceptionInfo *exception)
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
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/enhance_8c.html" id="SigmoidalContrastImage">SigmoidalContrastImage</a></h2>

<p>SigmoidalContrastImage() adjusts the contrast of an image with a non-linear sigmoidal contrast algorithm.  Increase the contrast of the image using a sigmoidal transfer function without saturating highlights or shadows. Contrast indicates how much to increase the contrast (0 is none; 3 is typical; 20 is pushing it); mid-point indicates where midtones fall in the resultant image (0 is white; 50 is middle-gray; 100 is black).  Set sharpen to MagickTrue to increase the image contrast otherwise the contrast is reduced.</p>

<p>The format of the SigmoidalContrastImage method is:</p>

<pre class="text">
MagickBooleanType SigmoidalContrastImage(Image *image,
  const MagickBooleanType sharpen,const char *levels,
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
<dt>sharpen</dt>
<dd>Increase or decrease image contrast. </dd>

<dd> </dd>
<dt>contrast</dt>
<dd>strength of the contrast, the larger the number the more 'threshold-like' it becomes. </dd>

<dd> </dd>
<dt>midpoint</dt>
<dd>midpoint of the function as a color value 0 to QuantumRange. </dd>

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
    <p><a href="enhance.php#">Back to top</a> •
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

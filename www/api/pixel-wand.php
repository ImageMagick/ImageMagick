



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickWand, C API: Pixel Wand Methods @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickwc, api:, pixel, wmethods, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/pixel-wand.php" rel="canonical">
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
<p class="text-center"><a href="pixel-wand.php#ClearPixelWand">ClearPixelWand</a> &bull; <a href="pixel-wand.php#ClonePixelWand">ClonePixelWand</a> &bull; <a href="pixel-wand.php#ClonePixelWands">ClonePixelWands</a> &bull; <a href="pixel-wand.php#DestroyPixelWand">DestroyPixelWand</a> &bull; <a href="pixel-wand.php#DestroyPixelWands">DestroyPixelWands</a> &bull; <a href="pixel-wand.php#IsPixelWandSimilar">IsPixelWandSimilar</a> &bull; <a href="pixel-wand.php#IsPixelWand">IsPixelWand</a> &bull; <a href="pixel-wand.php#NewPixelWand">NewPixelWand</a> &bull; <a href="pixel-wand.php#NewPixelWands">NewPixelWands</a> &bull; <a href="pixel-wand.php#PixelClearException">PixelClearException</a> &bull; <a href="pixel-wand.php#PixelGetAlpha">PixelGetAlpha</a> &bull; <a href="pixel-wand.php#PixelGetAlphaQuantum">PixelGetAlphaQuantum</a> &bull; <a href="pixel-wand.php#PixelGetBlack">PixelGetBlack</a> &bull; <a href="pixel-wand.php#PixelGetBlackQuantum">PixelGetBlackQuantum</a> &bull; <a href="pixel-wand.php#PixelGetBlue">PixelGetBlue</a> &bull; <a href="pixel-wand.php#PixelGetBlueQuantum">PixelGetBlueQuantum</a> &bull; <a href="pixel-wand.php#PixelGetColorAsString">PixelGetColorAsString</a> &bull; <a href="pixel-wand.php#PixelGetColorAsNormalizedString">PixelGetColorAsNormalizedString</a> &bull; <a href="pixel-wand.php#PixelGetColorCount">PixelGetColorCount</a> &bull; <a href="pixel-wand.php#PixelGetCyan">PixelGetCyan</a> &bull; <a href="pixel-wand.php#PixelGetCyanQuantum">PixelGetCyanQuantum</a> &bull; <a href="pixel-wand.php#PixelGetException">PixelGetException</a> &bull; <a href="pixel-wand.php#PixelGetExceptionType">PixelGetExceptionType</a> &bull; <a href="pixel-wand.php#PixelGetFuzz">PixelGetFuzz</a> &bull; <a href="pixel-wand.php#PixelGetGreen">PixelGetGreen</a> &bull; <a href="pixel-wand.php#PixelGetGreenQuantum">PixelGetGreenQuantum</a> &bull; <a href="pixel-wand.php#PixelGetHSL">PixelGetHSL</a> &bull; <a href="pixel-wand.php#PixelGetIndex">PixelGetIndex</a> &bull; <a href="pixel-wand.php#PixelGetMagenta">PixelGetMagenta</a> &bull; <a href="pixel-wand.php#PixelGetMagentaQuantum">PixelGetMagentaQuantum</a> &bull; <a href="pixel-wand.php#PixelGetMagickColor">PixelGetMagickColor</a> &bull; <a href="pixel-wand.php#PixelGetPixel">PixelGetPixel</a> &bull; <a href="pixel-wand.php#PixelGetQuantumPacket">PixelGetQuantumPacket</a> &bull; <a href="pixel-wand.php#PixelGetQuantumPixel">PixelGetQuantumPixel</a> &bull; <a href="pixel-wand.php#PixelGetRed">PixelGetRed</a> &bull; <a href="pixel-wand.php#PixelGetRedQuantum">PixelGetRedQuantum</a> &bull; <a href="pixel-wand.php#PixelGetYellow">PixelGetYellow</a> &bull; <a href="pixel-wand.php#PixelGetYellowQuantum">PixelGetYellowQuantum</a> &bull; <a href="pixel-wand.php#PixelSetAlpha">PixelSetAlpha</a> &bull; <a href="pixel-wand.php#PixelSetAlphaQuantum">PixelSetAlphaQuantum</a> &bull; <a href="pixel-wand.php#PixelSetBlack">PixelSetBlack</a> &bull; <a href="pixel-wand.php#PixelSetBlackQuantum">PixelSetBlackQuantum</a> &bull; <a href="pixel-wand.php#PixelSetBlue">PixelSetBlue</a> &bull; <a href="pixel-wand.php#PixelSetBlueQuantum">PixelSetBlueQuantum</a> &bull; <a href="pixel-wand.php#PixelSetColor">PixelSetColor</a> &bull; <a href="pixel-wand.php#PixelSetColorCount">PixelSetColorCount</a> &bull; <a href="pixel-wand.php#PixelSetColorFromWand">PixelSetColorFromWand</a> &bull; <a href="pixel-wand.php#PixelSetCyan">PixelSetCyan</a> &bull; <a href="pixel-wand.php#PixelSetCyanQuantum">PixelSetCyanQuantum</a> &bull; <a href="pixel-wand.php#PixelSetFuzz">PixelSetFuzz</a> &bull; <a href="pixel-wand.php#PixelSetGreen">PixelSetGreen</a> &bull; <a href="pixel-wand.php#PixelSetGreenQuantum">PixelSetGreenQuantum</a> &bull; <a href="pixel-wand.php#PixelSetHSL">PixelSetHSL</a> &bull; <a href="pixel-wand.php#PixelSetIndex">PixelSetIndex</a> &bull; <a href="pixel-wand.php#PixelSetMagenta">PixelSetMagenta</a> &bull; <a href="pixel-wand.php#PixelSetMagentaQuantum">PixelSetMagentaQuantum</a> &bull; <a href="pixel-wand.php#PixelSetPixelColor">PixelSetPixelColor</a> &bull; <a href="pixel-wand.php#PixelSetQuantumPixel">PixelSetQuantumPixel</a> &bull; <a href="pixel-wand.php#PixelSetRed">PixelSetRed</a> &bull; <a href="pixel-wand.php#PixelSetRedQuantum">PixelSetRedQuantum</a> &bull; <a href="pixel-wand.php#PixelSetYellow">PixelSetYellow</a> &bull; <a href="pixel-wand.php#PixelSetYellowQuantum">PixelSetYellowQuantum</a></p>

<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="ClearPixelWand">ClearPixelWand</a></h2>

<p>ClearPixelWand() clears resources associated with the wand.</p>

<p>The format of the ClearPixelWand method is:</p>

<pre class="text">
void ClearPixelWand(PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="ClonePixelWand">ClonePixelWand</a></h2>

<p>ClonePixelWand() makes an exact copy of the specified wand.</p>

<p>The format of the ClonePixelWand method is:</p>

<pre class="text">
PixelWand *ClonePixelWand(const PixelWand *wand)
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
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="ClonePixelWands">ClonePixelWands</a></h2>

<p>ClonePixelWands() makes an exact copy of the specified wands.</p>

<p>The format of the ClonePixelWands method is:</p>

<pre class="text">
PixelWand **ClonePixelWands(const PixelWand **wands,
  const size_t number_wands)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wands</dt>
<dd>the magick wands. </dd>

<dd> </dd>
<dt>number_wands</dt>
<dd>the number of wands. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="DestroyPixelWand">DestroyPixelWand</a></h2>

<p>DestroyPixelWand() deallocates resources associated with a PixelWand.</p>

<p>The format of the DestroyPixelWand method is:</p>

<pre class="text">
PixelWand *DestroyPixelWand(PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="DestroyPixelWands">DestroyPixelWands</a></h2>

<p>DestroyPixelWands() deallocates resources associated with an array of pixel wands.</p>

<p>The format of the DestroyPixelWands method is:</p>

<pre class="text">
PixelWand **DestroyPixelWands(PixelWand **wand,
  const size_t number_wands)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>number_wands</dt>
<dd>the number of wands. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="IsPixelWandSimilar">IsPixelWandSimilar</a></h2>

<p>IsPixelWandSimilar() returns MagickTrue if the distance between two colors is less than the specified distance.</p>

<p>The format of the IsPixelWandSimilar method is:</p>

<pre class="text">
MagickBooleanType IsPixelWandSimilar(PixelWand *p,PixelWand *q,
  const double fuzz)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>p</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>q</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>fuzz</dt>
<dd>any two colors that are less than or equal to this distance squared are consider similar. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="IsPixelWand">IsPixelWand</a></h2>

<p>IsPixelWand() returns MagickTrue if the wand is verified as a pixel wand.</p>

<p>The format of the IsPixelWand method is:</p>

<pre class="text">
MagickBooleanType IsPixelWand(const PixelWand *wand)
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
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="NewPixelWand">NewPixelWand</a></h2>

<p>NewPixelWand() returns a new pixel wand.</p>

<p>The format of the NewPixelWand method is:</p>

<pre class="text">
PixelWand *NewPixelWand(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="NewPixelWands">NewPixelWands</a></h2>

<p>NewPixelWands() returns an array of pixel wands.</p>

<p>The format of the NewPixelWands method is:</p>

<pre class="text">
PixelWand **NewPixelWands(const size_t number_wands)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>number_wands</dt>
<dd>the number of wands. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelClearException">PixelClearException</a></h2>

<p>PixelClearException() clear any exceptions associated with the iterator.</p>

<p>The format of the PixelClearException method is:</p>

<pre class="text">
MagickBooleanType PixelClearException(PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetAlpha">PixelGetAlpha</a></h2>

<p>PixelGetAlpha() returns the normalized alpha value of the pixel wand.</p>

<p>The format of the PixelGetAlpha method is:</p>

<pre class="text">
double PixelGetAlpha(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetAlphaQuantum">PixelGetAlphaQuantum</a></h2>

<p>PixelGetAlphaQuantum() returns the alpha value of the pixel wand.</p>

<p>The format of the PixelGetAlphaQuantum method is:</p>

<pre class="text">
Quantum PixelGetAlphaQuantum(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetBlack">PixelGetBlack</a></h2>

<p>PixelGetBlack() returns the normalized black color of the pixel wand.</p>

<p>The format of the PixelGetBlack method is:</p>

<pre class="text">
double PixelGetBlack(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetBlackQuantum">PixelGetBlackQuantum</a></h2>

<p>PixelGetBlackQuantum() returns the black color of the pixel wand.</p>

<p>The format of the PixelGetBlackQuantum method is:</p>

<pre class="text">
Quantum PixelGetBlackQuantum(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetBlue">PixelGetBlue</a></h2>

<p>PixelGetBlue() returns the normalized blue color of the pixel wand.</p>

<p>The format of the PixelGetBlue method is:</p>

<pre class="text">
double PixelGetBlue(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetBlueQuantum">PixelGetBlueQuantum</a></h2>

<p>PixelGetBlueQuantum() returns the blue color of the pixel wand.</p>

<p>The format of the PixelGetBlueQuantum method is:</p>

<pre class="text">
Quantum PixelGetBlueQuantum(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetColorAsString">PixelGetColorAsString</a></h2>

<p>PixelGetColorAsString() returnsd the color of the pixel wand as a string.</p>

<p>The format of the PixelGetColorAsString method is:</p>

<pre class="text">
char *PixelGetColorAsString(PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetColorAsNormalizedString">PixelGetColorAsNormalizedString</a></h2>

<p>PixelGetColorAsNormalizedString() returns the normalized color of the pixel wand as a string.</p>

<p>The format of the PixelGetColorAsNormalizedString method is:</p>

<pre class="text">
char *PixelGetColorAsNormalizedString(PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetColorCount">PixelGetColorCount</a></h2>

<p>PixelGetColorCount() returns the color count associated with this color.</p>

<p>The format of the PixelGetColorCount method is:</p>

<pre class="text">
size_t PixelGetColorCount(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetCyan">PixelGetCyan</a></h2>

<p>PixelGetCyan() returns the normalized cyan color of the pixel wand.</p>

<p>The format of the PixelGetCyan method is:</p>

<pre class="text">
double PixelGetCyan(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetCyanQuantum">PixelGetCyanQuantum</a></h2>

<p>PixelGetCyanQuantum() returns the cyan color of the pixel wand.</p>

<p>The format of the PixelGetCyanQuantum method is:</p>

<pre class="text">
Quantum PixelGetCyanQuantum(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetException">PixelGetException</a></h2>

<p>PixelGetException() returns the severity, reason, and description of any error that occurs when using other methods in this API.</p>

<p>The format of the PixelGetException method is:</p>

<pre class="text">
char *PixelGetException(const PixelWand *wand,ExceptionType *severity)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>severity</dt>
<dd>the severity of the error is returned here. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetExceptionType">PixelGetExceptionType</a></h2>

<p>PixelGetExceptionType() the exception type associated with the wand.  If no exception has occurred, UndefinedExceptionType is returned.</p>

<p>The format of the PixelGetExceptionType method is:</p>

<pre class="text">
ExceptionType PixelGetExceptionType(const PixelWand *wand)
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
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetFuzz">PixelGetFuzz</a></h2>

<p>PixelGetFuzz() returns the normalized fuzz value of the pixel wand.</p>

<p>The format of the PixelGetFuzz method is:</p>

<pre class="text">
double PixelGetFuzz(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetGreen">PixelGetGreen</a></h2>

<p>PixelGetGreen() returns the normalized green color of the pixel wand.</p>

<p>The format of the PixelGetGreen method is:</p>

<pre class="text">
double PixelGetGreen(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetGreenQuantum">PixelGetGreenQuantum</a></h2>

<p>PixelGetGreenQuantum() returns the green color of the pixel wand.</p>

<p>The format of the PixelGetGreenQuantum method is:</p>

<pre class="text">
Quantum PixelGetGreenQuantum(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetHSL">PixelGetHSL</a></h2>

<p>PixelGetHSL() returns the normalized HSL color of the pixel wand.</p>

<p>The format of the PixelGetHSL method is:</p>

<pre class="text">
void PixelGetHSL(const PixelWand *wand,double *hue,double *saturation,
  double *lightness)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>hue,saturation,lightness</dt>
<dd>Return the pixel hue, saturation, and brightness. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetIndex">PixelGetIndex</a></h2>

<p>PixelGetIndex() returns the colormap index from the pixel wand.</p>

<p>The format of the PixelGetIndex method is:</p>

<pre class="text">
Quantum PixelGetIndex(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetMagenta">PixelGetMagenta</a></h2>

<p>PixelGetMagenta() returns the normalized magenta color of the pixel wand.</p>

<p>The format of the PixelGetMagenta method is:</p>

<pre class="text">
double PixelGetMagenta(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetMagentaQuantum">PixelGetMagentaQuantum</a></h2>

<p>PixelGetMagentaQuantum() returns the magenta color of the pixel wand.</p>

<p>The format of the PixelGetMagentaQuantum method is:</p>

<pre class="text">
Quantum PixelGetMagentaQuantum(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetMagickColor">PixelGetMagickColor</a></h2>

<p>PixelGetMagickColor() gets the magick color of the pixel wand.</p>

<p>The format of the PixelGetMagickColor method is:</p>

<pre class="text">
void PixelGetMagickColor(PixelWand *wand,PixelInfo *color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>color</dt>
<dd> The pixel wand color is returned here. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetPixel">PixelGetPixel</a></h2>

<p>PixelGetPixel() returns the pixel wand pixel.</p>

<p>The format of the PixelGetPixel method is:</p>

<pre class="text">
PixelInfo PixelGetPixel(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetQuantumPacket">PixelGetQuantumPacket</a></h2>

<p>PixelGetQuantumPacket() gets the packet of the pixel wand as a PixelInfo.</p>

<p>The format of the PixelGetQuantumPacket method is:</p>

<pre class="text">
void PixelGetQuantumPacket(PixelWand *wand,PixelInfo *packet)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>packet</dt>
<dd> The pixel wand packet is returned here. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetQuantumPixel">PixelGetQuantumPixel</a></h2>

<p>PixelGetQuantumPixel() gets the pixel of the pixel wand as a PixelInfo.</p>

<p>The format of the PixelGetQuantumPixel method is:</p>

<pre class="text">
void PixelGetQuantumPixel(const Image *image,const PixelWand *wand,
  Quantum *pixel)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>pixel</dt>
<dd> The pixel wand pixel is returned here. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetRed">PixelGetRed</a></h2>

<p>PixelGetRed() returns the normalized red color of the pixel wand.</p>

<p>The format of the PixelGetRed method is:</p>

<pre class="text">
double PixelGetRed(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetRedQuantum">PixelGetRedQuantum</a></h2>

<p>PixelGetRedQuantum() returns the red color of the pixel wand.</p>

<p>The format of the PixelGetRedQuantum method is:</p>

<pre class="text">
Quantum PixelGetRedQuantum(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetYellow">PixelGetYellow</a></h2>

<p>PixelGetYellow() returns the normalized yellow color of the pixel wand.</p>

<p>The format of the PixelGetYellow method is:</p>

<pre class="text">
double PixelGetYellow(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelGetYellowQuantum">PixelGetYellowQuantum</a></h2>

<p>PixelGetYellowQuantum() returns the yellow color of the pixel wand.</p>

<p>The format of the PixelGetYellowQuantum method is:</p>

<pre class="text">
Quantum PixelGetYellowQuantum(const PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetAlpha">PixelSetAlpha</a></h2>

<p>PixelSetAlpha() sets the normalized alpha value of the pixel wand.</p>

<p>The format of the PixelSetAlpha method is:</p>

<pre class="text">
void PixelSetAlpha(PixelWand *wand,const double alpha)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>alpha</dt>
<dd>the level of transparency: 1.0 is fully opaque and 0.0 is fully transparent. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetAlphaQuantum">PixelSetAlphaQuantum</a></h2>

<p>PixelSetAlphaQuantum() sets the alpha value of the pixel wand.</p>

<p>The format of the PixelSetAlphaQuantum method is:</p>

<pre class="text">
void PixelSetAlphaQuantum(PixelWand *wand,const Quantum alpha)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>alpha</dt>
<dd>the alpha value. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetBlack">PixelSetBlack</a></h2>

<p>PixelSetBlack() sets the normalized black color of the pixel wand.</p>

<p>The format of the PixelSetBlack method is:</p>

<pre class="text">
void PixelSetBlack(PixelWand *wand,const double black)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>black</dt>
<dd>the black color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetBlackQuantum">PixelSetBlackQuantum</a></h2>

<p>PixelSetBlackQuantum() sets the black color of the pixel wand.</p>

<p>The format of the PixelSetBlackQuantum method is:</p>

<pre class="text">
void PixelSetBlackQuantum(PixelWand *wand,const Quantum black)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>black</dt>
<dd>the black color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetBlue">PixelSetBlue</a></h2>

<p>PixelSetBlue() sets the normalized blue color of the pixel wand.</p>

<p>The format of the PixelSetBlue method is:</p>

<pre class="text">
void PixelSetBlue(PixelWand *wand,const double blue)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>blue</dt>
<dd>the blue color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetBlueQuantum">PixelSetBlueQuantum</a></h2>

<p>PixelSetBlueQuantum() sets the blue color of the pixel wand.</p>

<p>The format of the PixelSetBlueQuantum method is:</p>

<pre class="text">
void PixelSetBlueQuantum(PixelWand *wand,const Quantum blue)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>blue</dt>
<dd>the blue color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetColor">PixelSetColor</a></h2>

<p>PixelSetColor() sets the color of the pixel wand with a string (e.g. "blue", "#0000ff", "rgb(0,0,255)", "cmyk(100,100,100,10)", etc.).</p>

<p>The format of the PixelSetColor method is:</p>

<pre class="text">
MagickBooleanType PixelSetColor(PixelWand *wand,const char *color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>color</dt>
<dd>the pixel wand color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetColorCount">PixelSetColorCount</a></h2>

<p>PixelSetColorCount() sets the color count of the pixel wand.</p>

<p>The format of the PixelSetColorCount method is:</p>

<pre class="text">
void PixelSetColorCount(PixelWand *wand,const size_t count)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>count</dt>
<dd>the number of this particular color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetColorFromWand">PixelSetColorFromWand</a></h2>

<p>PixelSetColorFromWand() sets the color of the pixel wand.</p>

<p>The format of the PixelSetColorFromWand method is:</p>

<pre class="text">
void PixelSetColorFromWand(PixelWand *wand,const PixelWand *color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>color</dt>
<dd>set the pixel wand color here. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetCyan">PixelSetCyan</a></h2>

<p>PixelSetCyan() sets the normalized cyan color of the pixel wand.</p>

<p>The format of the PixelSetCyan method is:</p>

<pre class="text">
void PixelSetCyan(PixelWand *wand,const double cyan)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>cyan</dt>
<dd>the cyan color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetCyanQuantum">PixelSetCyanQuantum</a></h2>

<p>PixelSetCyanQuantum() sets the cyan color of the pixel wand.</p>

<p>The format of the PixelSetCyanQuantum method is:</p>

<pre class="text">
void PixelSetCyanQuantum(PixelWand *wand,const Quantum cyan)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>cyan</dt>
<dd>the cyan color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetFuzz">PixelSetFuzz</a></h2>

<p>PixelSetFuzz() sets the fuzz value of the pixel wand.</p>

<p>The format of the PixelSetFuzz method is:</p>

<pre class="text">
void PixelSetFuzz(PixelWand *wand,const double fuzz)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>fuzz</dt>
<dd>the fuzz value. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetGreen">PixelSetGreen</a></h2>

<p>PixelSetGreen() sets the normalized green color of the pixel wand.</p>

<p>The format of the PixelSetGreen method is:</p>

<pre class="text">
void PixelSetGreen(PixelWand *wand,const double green)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>green</dt>
<dd>the green color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetGreenQuantum">PixelSetGreenQuantum</a></h2>

<p>PixelSetGreenQuantum() sets the green color of the pixel wand.</p>

<p>The format of the PixelSetGreenQuantum method is:</p>

<pre class="text">
void PixelSetGreenQuantum(PixelWand *wand,const Quantum green)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>green</dt>
<dd>the green color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetHSL">PixelSetHSL</a></h2>

<p>PixelSetHSL() sets the normalized HSL color of the pixel wand.</p>

<p>The format of the PixelSetHSL method is:</p>

<pre class="text">
void PixelSetHSL(PixelWand *wand,const double hue,
  const double saturation,const double lightness)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>hue,saturation,lightness</dt>
<dd>Return the pixel hue, saturation, and brightness. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetIndex">PixelSetIndex</a></h2>

<p>PixelSetIndex() sets the colormap index of the pixel wand.</p>

<p>The format of the PixelSetIndex method is:</p>

<pre class="text">
void PixelSetIndex(PixelWand *wand,const Quantum index)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>index</dt>
<dd>the colormap index. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetMagenta">PixelSetMagenta</a></h2>

<p>PixelSetMagenta() sets the normalized magenta color of the pixel wand.</p>

<p>The format of the PixelSetMagenta method is:</p>

<pre class="text">
void PixelSetMagenta(PixelWand *wand,const double magenta)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>magenta</dt>
<dd>the magenta color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetMagentaQuantum">PixelSetMagentaQuantum</a></h2>

<p>PixelSetMagentaQuantum() sets the magenta color of the pixel wand.</p>

<p>The format of the PixelSetMagentaQuantum method is:</p>

<pre class="text">
void PixelSetMagentaQuantum(PixelWand *wand,
  const Quantum magenta)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>magenta</dt>
<dd>the green magenta. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetPixelColor">PixelSetPixelColor</a></h2>

<p>PixelSetPixelColor() sets the color of the pixel wand.</p>

<p>The format of the PixelSetPixelColor method is:</p>

<pre class="text">
void PixelSetPixelColor(PixelWand *wand,const PixelInfo *color)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>color</dt>
<dd>the pixel wand color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetQuantumPixel">PixelSetQuantumPixel</a></h2>

<p>PixelSetQuantumPixel() sets the pixel of the pixel wand.</p>

<p>The format of the PixelSetQuantumPixel method is:</p>

<pre class="text">
void PixelSetQuantumPixel(const Image *image,const Quantum *pixel,
  PixelWand *wand)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>pixel</dt>
<dd>the pixel wand pixel. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetRed">PixelSetRed</a></h2>

<p>PixelSetRed() sets the normalized red color of the pixel wand.</p>

<p>The format of the PixelSetRed method is:</p>

<pre class="text">
void PixelSetRed(PixelWand *wand,const double red)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>red</dt>
<dd>the red color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetRedQuantum">PixelSetRedQuantum</a></h2>

<p>PixelSetRedQuantum() sets the red color of the pixel wand.</p>

<p>The format of the PixelSetRedQuantum method is:</p>

<pre class="text">
void PixelSetRedQuantum(PixelWand *wand,const Quantum red)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>red</dt>
<dd>the red color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetYellow">PixelSetYellow</a></h2>

<p>PixelSetYellow() sets the normalized yellow color of the pixel wand.</p>

<p>The format of the PixelSetYellow method is:</p>

<pre class="text">
void PixelSetYellow(PixelWand *wand,const double yellow)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>yellow</dt>
<dd>the yellow color. </dd>

<dd>  </dd>
</dl>
<h2><a href="https://imagemagick.org/api/MagickWand/pixel-wand_8c.html" id="PixelSetYellowQuantum">PixelSetYellowQuantum</a></h2>

<p>PixelSetYellowQuantum() sets the yellow color of the pixel wand.</p>

<p>The format of the PixelSetYellowQuantum method is:</p>

<pre class="text">
void PixelSetYellowQuantum(PixelWand *wand,const Quantum yellow)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>wand</dt>
<dd>the pixel wand. </dd>

<dd> </dd>
<dt>yellow</dt>
<dd>the yellow color. </dd>

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
    <a href="pixel-wand.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 2nd September 2018 15:16 -->
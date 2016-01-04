



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Convert to and from Cipher Pixels</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, convert, to, from, cipher, pixels, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="cipher.php#AcquireAESInfo">AcquireAESInfo</a> &bull; <a href="cipher.php#DestroyAESInfo">DestroyAESInfo</a> &bull; <a href="cipher.php#EncipherAESBlock">EncipherAESBlock</a> &bull; <a href="cipher.php#PasskeyDecipherImage">PasskeyDecipherImage</a> &bull; <a href="cipher.php#PasskeyEncipherImage">PasskeyEncipherImage</a> &bull; <a href="cipher.php#SetAESKey">SetAESKey</a> &bull; <a href="cipher.php#PasskeyDecipherImage">PasskeyDecipherImage</a> &bull; <a href="cipher.php#PasskeyEncipherImage">PasskeyEncipherImage</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cipher_8c.html" id="AcquireAESInfo">AcquireAESInfo</a></h2>

<p>AcquireAESInfo() allocate the AESInfo structure.</p>

<p>The format of the AcquireAESInfo method is:</p>

<pre class="text">
AESInfo *AcquireAESInfo(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cipher_8c.html" id="DestroyAESInfo">DestroyAESInfo</a></h2>

<p>DestroyAESInfo() zeros memory associated with the AESInfo structure.</p>

<p>The format of the DestroyAESInfo method is:</p>

<pre class="text">
AESInfo *DestroyAESInfo(AESInfo *aes_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>aes_info</dt>
<dd>the cipher context. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cipher_8c.html" id="EncipherAESBlock">EncipherAESBlock</a></h2>

<p>EncipherAESBlock() enciphers a single block of plaintext to produce a block of ciphertext.</p>

<p>The format of the EncipherAESBlock method is:</p>

<pre class="text">
void EncipherAES(AESInfo *aes_info,const unsigned char *plaintext,
  unsigned char *ciphertext)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>aes_info</dt>
<dd>the cipher context. </dd>

<dd> </dd>
<dt>plaintext</dt>
<dd>the plain text. </dd>

<dd> </dd>
<dt>ciphertext</dt>
<dd>the cipher text. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cipher_8c.html" id="PasskeyDecipherImage">PasskeyDecipherImage</a></h2>

<p>PasskeyDecipherImage() converts cipher pixels to plain pixels.</p>

<p>The format of the PasskeyDecipherImage method is:</p>

<pre class="text">
MagickBooleanType PasskeyDecipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
MagickBooleanType DecipherImage(Image *image,const char *passphrase,
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
<dt>passphrase</dt>
<dd>decipher cipher pixels with this passphrase. </dd>

<dd> </dd>
<dt>passkey</dt>
<dd>decrypt cipher pixels with this passkey. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cipher_8c.html" id="PasskeyEncipherImage">PasskeyEncipherImage</a></h2>

<p>PasskeyEncipherImage() converts pixels to cipher-pixels.</p>

<p>The format of the PasskeyEncipherImage method is:</p>

<pre class="text">
MagickBooleanType PasskeyEncipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
MagickBooleanType EncipherImage(Image *image,const char *passphrase,
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
<dt>passphrase</dt>
<dd>encipher pixels with this passphrase. </dd>

<dd> </dd>
<dt>passkey</dt>
<dd>decrypt cipher pixels with this passkey. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cipher_8c.html" id="SetAESKey">SetAESKey</a></h2>

<p>SetAESKey() sets the key for the AES cipher.  The key length is specified in bits.  Valid values are 128, 192, or 256 requiring a key buffer length in bytes of 16, 24, and 32 respectively.</p>

<p>The format of the SetAESKey method is:</p>

<pre class="text">
SetAESKey(AESInfo *aes_info,const StringInfo *key)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>aes_info</dt>
<dd>the cipher context. </dd>

<dd> </dd>
<dt>key</dt>
<dd>the key. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cipher_8c.html" id="PasskeyDecipherImage">PasskeyDecipherImage</a></h2>

<p>PasskeyDecipherImage() converts cipher pixels to plain pixels.</p>

<p>The format of the PasskeyDecipherImage method is:</p>

<pre class="text">
MagickBooleanType PasskeyDecipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
MagickBooleanType DecipherImage(Image *image,const char *passphrase,
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
<dt>passphrase</dt>
<dd>decipher cipher pixels with this passphrase. </dd>

<dd> </dd>
<dt>passkey</dt>
<dd>decrypt cipher pixels with this passkey. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/cipher_8c.html" id="PasskeyEncipherImage">PasskeyEncipherImage</a></h2>

<p>PasskeyEncipherImage() converts pixels to cipher-pixels.</p>

<p>The format of the PasskeyEncipherImage method is:</p>

<pre class="text">
MagickBooleanType PasskeyEncipherImage(Image *image,
  const StringInfo *passkey,ExceptionInfo *exception)
MagickBooleanType EncipherImage(Image *image,const char *passphrase,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>passphrase</dt>
<dd>decipher cipher pixels with this passphrase. </dd>

<dd> </dd>
<dt>passkey</dt>
<dd>decrypt cipher pixels with this passkey. </dd>

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
    <p><a href="cipher.php#">Back to top</a> •
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

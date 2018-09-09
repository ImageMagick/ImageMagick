



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Convert to and from Cipher Pixels @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://www.imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, convert, to, from, cipher, pixels, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://www.imagemagick.org/api/cipher.php" rel="canonical">
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
        <a class="nav-link" target="_blank" href="https://www.imagemagick.org/discourse-server/">Community</a>
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
<p class="text-center"><a href="cipher.php#AcquireAESInfo">AcquireAESInfo</a> &bull; <a href="cipher.php#DestroyAESInfo">DestroyAESInfo</a> &bull; <a href="cipher.php#EncipherAESBlock">EncipherAESBlock</a> &bull; <a href="cipher.php#PasskeyDecipherImage">PasskeyDecipherImage</a> &bull; <a href="cipher.php#PasskeyEncipherImage">PasskeyEncipherImage</a> &bull; <a href="cipher.php#SetAESKey">SetAESKey</a> &bull; <a href="cipher.php#PasskeyDecipherImage">PasskeyDecipherImage</a> &bull; <a href="cipher.php#PasskeyEncipherImage">PasskeyEncipherImage</a></p>

<h2><a href="http://www.imagemagick.org/api/MagickCore/cipher_8c.html" id="AcquireAESInfo">AcquireAESInfo</a></h2>

<p>AcquireAESInfo() allocate the AESInfo structure.</p>

<p>The format of the AcquireAESInfo method is:</p>

<pre class="text">
AESInfo *AcquireAESInfo(void)
</pre>

<h2><a href="http://www.imagemagick.org/api/MagickCore/cipher_8c.html" id="DestroyAESInfo">DestroyAESInfo</a></h2>

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
<h2><a href="http://www.imagemagick.org/api/MagickCore/cipher_8c.html" id="EncipherAESBlock">EncipherAESBlock</a></h2>

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
<h2><a href="http://www.imagemagick.org/api/MagickCore/cipher_8c.html" id="PasskeyDecipherImage">PasskeyDecipherImage</a></h2>

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
<h2><a href="http://www.imagemagick.org/api/MagickCore/cipher_8c.html" id="PasskeyEncipherImage">PasskeyEncipherImage</a></h2>

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
<h2><a href="http://www.imagemagick.org/api/MagickCore/cipher_8c.html" id="SetAESKey">SetAESKey</a></h2>

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
<h2><a href="http://www.imagemagick.org/api/MagickCore/cipher_8c.html" id="PasskeyDecipherImage">PasskeyDecipherImage</a></h2>

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
<h2><a href="http://www.imagemagick.org/api/MagickCore/cipher_8c.html" id="PasskeyEncipherImage">PasskeyEncipherImage</a></h2>

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
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="cipher.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
    &nbsp; &nbsp;
    <a href="http://pgp.mit.edu/pks/lookup?op=get&amp;search=0x89AB63D48277377A">Public Key</a> •
    <a href="https://imagemagick.org/script/support.php">Donate</a> •
    <a href="https://imagemagick.org/script/contact.php">Contact Us</a>
    <br/>
        <small>© 1999-2018 ImageMagick Studio LLC</small></p>
  </footer>

  <!-- Javascript assets -->
  <script src="https://imagemagick.org/assets/magick-js.php" crossorigin="anonymous"></script>
  <script>window.jQuery || document.write('<script src="https://imagemagick.org/assets/jquery.min.js"><\/script>')</script>
</body>
</html>
<!-- Magick Cache 3rd September 2018 21:21 -->
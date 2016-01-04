



<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Dealing with Exceptions</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, dealing, with, exceptions, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="exception.php#AcquireExceptionInfo">AcquireExceptionInfo</a> &bull; <a href="exception.php#ClearMagickException">ClearMagickException</a> &bull; <a href="exception.php#CatchException">CatchException</a> &bull; <a href="exception.php#CloneExceptionInfo">CloneExceptionInfo</a> &bull; <a href="exception.php#DestroyExceptionInfo">DestroyExceptionInfo</a> &bull; <a href="exception.php#GetExceptionMessage">GetExceptionMessage</a> &bull; <a href="exception.php#GetLocaleExceptionMessage">GetLocaleExceptionMessage</a> &bull; <a href="exception.php#InheritException">InheritException</a> &bull; <a href="exception.php#InitializeExceptionInfo">InitializeExceptionInfo</a> &bull; <a href="exception.php#MagickError">MagickError</a> &bull; <a href="exception.php#MagickFatalError">MagickFatalError</a> &bull; <a href="exception.php#MagickWarning">MagickWarning</a> &bull; <a href="exception.php#SetErrorHandler">SetErrorHandler</a> &bull; <a href="exception.php#SetFatalErrorHandler">SetFatalErrorHandler</a> &bull; <a href="exception.php#SetWarningHandler">SetWarningHandler</a> &bull; <a href="exception.php#ThrowException">ThrowException</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="AcquireExceptionInfo">AcquireExceptionInfo</a></h2>

<p>AcquireExceptionInfo() allocates the ExceptionInfo structure.</p>

<p>The format of the AcquireExceptionInfo method is:</p>

<pre class="text">
ExceptionInfo *AcquireExceptionInfo(void)
</pre>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="ClearMagickException">ClearMagickException</a></h2>

<p>ClearMagickException() clears any exception that may not have been caught yet.</p>

<p>The format of the ClearMagickException method is:</p>

<pre class="text">
ClearMagickException(ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>exception</dt>
<dd>the exception info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="CatchException">CatchException</a></h2>

<p>CatchException() returns if no exceptions is found otherwise it reports the exception as a warning, error, or fatal depending on the severity.</p>

<p>The format of the CatchException method is:</p>

<pre class="text">
CatchException(ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>exception</dt>
<dd>the exception info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="CloneExceptionInfo">CloneExceptionInfo</a></h2>

<p>CloneExceptionInfo() clones the ExceptionInfo structure.</p>

<p>The format of the CloneExceptionInfo method is:</p>

<pre class="text">
ExceptionInfo *CloneException(ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>exception</dt>
<dd>the exception info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="DestroyExceptionInfo">DestroyExceptionInfo</a></h2>

<p>DestroyExceptionInfo() deallocates memory associated with an exception.</p>

<p>The format of the DestroyExceptionInfo method is:</p>

<pre class="text">
ExceptionInfo *DestroyExceptionInfo(ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>exception</dt>
<dd>the exception info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="GetExceptionMessage">GetExceptionMessage</a></h2>

<p>GetExceptionMessage() returns the error message defined by the specified error code.</p>

<p>The format of the GetExceptionMessage method is:</p>

<pre class="text">
char *GetExceptionMessage(const int error)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>error</dt>
<dd>the error code. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="GetLocaleExceptionMessage">GetLocaleExceptionMessage</a></h2>

<p>GetLocaleExceptionMessage() converts a enumerated exception severity and tag to a message in the current locale.</p>

<p>The format of the GetLocaleExceptionMessage method is:</p>

<pre class="text">
const char *GetLocaleExceptionMessage(const ExceptionType severity,
  const char *tag)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>severity</dt>
<dd>the severity of the exception. </dd>

<dd> </dd>
<dt>tag</dt>
<dd>the message tag. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="InheritException">InheritException</a></h2>

<p>InheritException() inherits an exception from a related exception.</p>

<p>The format of the InheritException method is:</p>

<pre class="text">
InheritException(ExceptionInfo *exception,const ExceptionInfo *relative)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>exception</dt>
<dd>the exception info. </dd>

<dd> </dd>
<dt>relative</dt>
<dd>the related exception info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="InitializeExceptionInfo">InitializeExceptionInfo</a></h2>

<p>InitializeExceptionInfo() initializes an exception to default values.</p>

<p>The format of the InitializeExceptionInfo method is:</p>

<pre class="text">
InitializeExceptionInfo(ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>exception</dt>
<dd>the exception info. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="MagickError">MagickError</a></h2>

<p>MagickError() calls the exception handler methods with an error reason.</p>

<p>The format of the MagickError method is:</p>

<pre class="text">
void MagickError(const ExceptionType error,const char *reason,
  const char *description)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>exception</dt>
<dd>Specifies the numeric error category. </dd>

<dd> </dd>
<dt>reason</dt>
<dd>Specifies the reason to display before terminating the program. </dd>

<dd> </dd>
<dt>description</dt>
<dd>Specifies any description to the reason. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="MagickFatalError">MagickFatalError</a></h2>

<p>MagickFatalError() calls the fatal exception handler methods with an error reason.</p>

<p>The format of the MagickError method is:</p>

<pre class="text">
void MagickFatalError(const ExceptionType error,const char *reason,
  const char *description)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>exception</dt>
<dd>Specifies the numeric error category. </dd>

<dd> </dd>
<dt>reason</dt>
<dd>Specifies the reason to display before terminating the program. </dd>

<dd> </dd>
<dt>description</dt>
<dd>Specifies any description to the reason. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="MagickWarning">MagickWarning</a></h2>

<p>MagickWarning() calls the warning handler methods with a warning reason.</p>

<p>The format of the MagickWarning method is:</p>

<pre class="text">
void MagickWarning(const ExceptionType warning,const char *reason,
  const char *description)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>warning</dt>
<dd>the warning severity. </dd>

<dd> </dd>
<dt>reason</dt>
<dd>Define the reason for the warning. </dd>

<dd> </dd>
<dt>description</dt>
<dd>Describe the warning. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="SetErrorHandler">SetErrorHandler</a></h2>

<p>SetErrorHandler() sets the exception handler to the specified method and returns the previous exception handler.</p>

<p>The format of the SetErrorHandler method is:</p>

<pre class="text">
ErrorHandler SetErrorHandler(ErrorHandler handler)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>handler</dt>
<dd>the method to handle errors. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="SetFatalErrorHandler">SetFatalErrorHandler</a></h2>

<p>SetFatalErrorHandler() sets the fatal exception handler to the specified method and returns the previous fatal exception handler.</p>

<p>The format of the SetErrorHandler method is:</p>

<pre class="text">
ErrorHandler SetErrorHandler(ErrorHandler handler)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>handler</dt>
<dd>the method to handle errors. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="SetWarningHandler">SetWarningHandler</a></h2>

<p>SetWarningHandler() sets the warning handler to the specified method and returns the previous warning handler.</p>

<p>The format of the SetWarningHandler method is:</p>

<pre class="text">
ErrorHandler SetWarningHandler(ErrorHandler handler)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>handler</dt>
<dd>the method to handle warnings. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/exception_8c.html" id="ThrowException">ThrowException</a></h2>

<p>ThrowException() throws an exception with the specified severity code, reason, and optional description.</p>

<p>The format of the ThrowException method is:</p>

<pre class="text">
MagickBooleanType ThrowException(ExceptionInfo *exception,
  const ExceptionType severity,const char *reason,
  const char *description)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>exception</dt>
<dd>the exception info. </dd>

<dd> </dd>
<dt>severity</dt>
<dd>the severity of the exception. </dd>

<dd> </dd>
<dt>reason</dt>
<dd>the reason for the exception. </dd>

<dd> </dd>
<dt>description</dt>
<dd>the exception description. </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="exception.php#">Back to top</a> •
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

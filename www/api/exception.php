



<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" >
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" >
  <title>MagickCore, C API: Dealing with Exceptions @ ImageMagick</title>
  <meta name="application-name" content="ImageMagick">
  <meta name="description" content="Use ImageMagick® to create, edit, compose, convert bitmap images. With ImageMagick you can resize your image, crop it, change its shades and colors, add captions, among other operations.">
  <meta name="application-url" content="https://imagemagick.org">
  <meta name="generator" content="PHP">
  <meta name="keywords" content="magickcore, c, api:, dealing, with, exceptions, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert">
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
  <link href="https://imagemagick.org/api/exception.php" rel="canonical">
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
<p class="text-center"><a href="exception.php#AcquireExceptionInfo">AcquireExceptionInfo</a> &bull; <a href="exception.php#ClearMagickException">ClearMagickException</a> &bull; <a href="exception.php#CatchException">CatchException</a> &bull; <a href="exception.php#CloneExceptionInfo">CloneExceptionInfo</a> &bull; <a href="exception.php#DestroyExceptionInfo">DestroyExceptionInfo</a> &bull; <a href="exception.php#GetExceptionMessage">GetExceptionMessage</a> &bull; <a href="exception.php#GetLocaleExceptionMessage">GetLocaleExceptionMessage</a> &bull; <a href="exception.php#InheritException">InheritException</a> &bull; <a href="exception.php#InitializeExceptionInfo">InitializeExceptionInfo</a> &bull; <a href="exception.php#MagickError">MagickError</a> &bull; <a href="exception.php#MagickFatalError">MagickFatalError</a> &bull; <a href="exception.php#MagickWarning">MagickWarning</a> &bull; <a href="exception.php#SetErrorHandler">SetErrorHandler</a> &bull; <a href="exception.php#SetFatalErrorHandler">SetFatalErrorHandler</a> &bull; <a href="exception.php#SetWarningHandler">SetWarningHandler</a> &bull; <a href="exception.php#ThrowException">ThrowException</a></p>

<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="AcquireExceptionInfo">AcquireExceptionInfo</a></h2>

<p>AcquireExceptionInfo() allocates the ExceptionInfo structure.</p>

<p>The format of the AcquireExceptionInfo method is:</p>

<pre class="text">
ExceptionInfo *AcquireExceptionInfo(void)
</pre>

<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="ClearMagickException">ClearMagickException</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="CatchException">CatchException</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="CloneExceptionInfo">CloneExceptionInfo</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="DestroyExceptionInfo">DestroyExceptionInfo</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="GetExceptionMessage">GetExceptionMessage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="GetLocaleExceptionMessage">GetLocaleExceptionMessage</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="InheritException">InheritException</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="InitializeExceptionInfo">InitializeExceptionInfo</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="MagickError">MagickError</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="MagickFatalError">MagickFatalError</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="MagickWarning">MagickWarning</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="SetErrorHandler">SetErrorHandler</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="SetFatalErrorHandler">SetFatalErrorHandler</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="SetWarningHandler">SetWarningHandler</a></h2>

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
<h2><a href="https://imagemagick.org/api/MagickCore/exception_8c.html" id="ThrowException">ThrowException</a></h2>

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
    </div>
  </main><!-- /.container -->
  <footer class="magick-footer">
    <p><a href="https://imagemagick.org/script/security-policy.php">Security</a> •
    <a href="https://imagemagick.org/script/architecture.php">Architecture</a> •
    <a href="https://imagemagick.org/script/links.php">Related</a> •
     <a href="https://imagemagick.org/script/sitemap.php">Sitemap</a>
    &nbsp; &nbsp;
    <a href="exception.php#"><img class="d-inline" id="wand" alt="And Now a Touch of Magick" width="16" height="16" src="https://imagemagick.org/image/wand.ico"/></a>
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
<!-- Magick Cache 2nd September 2018 13:49 -->




<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Memory Allocation</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, memory, allocation, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="memory.php#AcquireAlignedMemory">AcquireAlignedMemory</a> &bull; <a href="memory.php#AcquireMagickMemory">AcquireMagickMemory</a> &bull; <a href="memory.php#AcquireQuantumMemory">AcquireQuantumMemory</a> &bull; <a href="memory.php#AcquireVirtualMemory">AcquireVirtualMemory</a> &bull; <a href="memory.php#CopyMagickMemory">CopyMagickMemory</a> &bull; <a href="memory.php#GetMagickMemoryMethods">GetMagickMemoryMethods</a> &bull; <a href="memory.php#GetVirtualMemoryBlob">GetVirtualMemoryBlob</a> &bull; <a href="memory.php#RelinquishAlignedMemory">RelinquishAlignedMemory</a> &bull; <a href="memory.php#RelinquishMagickMemory">RelinquishMagickMemory</a> &bull; <a href="memory.php#RelinquishVirtualMemory">RelinquishVirtualMemory</a> &bull; <a href="memory.php#ResetMagickMemory">ResetMagickMemory</a> &bull; <a href="memory.php#ResizeMagickMemory">ResizeMagickMemory</a> &bull; <a href="memory.php#ResizeQuantumMemory">ResizeQuantumMemory</a> &bull; <a href="memory.php#SetMagickMemoryMethods">SetMagickMemoryMethods</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="AcquireAlignedMemory">AcquireAlignedMemory</a></h2>

<p>AcquireAlignedMemory() returns a pointer to a block of memory at least size bytes whose address is a multiple of 16*sizeof(void *).</p>

<p>The format of the AcquireAlignedMemory method is:</p>

<pre class="text">
void *AcquireAlignedMemory(const size_t count,const size_t quantum)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>count</dt>
<dd>the number of quantum elements to allocate. </dd>

<dd> </dd>
<dt>quantum</dt>
<dd>the number of bytes in each quantum. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="AcquireMagickMemory">AcquireMagickMemory</a></h2>

<p>AcquireMagickMemory() returns a pointer to a block of memory at least size bytes suitably aligned for any use.</p>

<p>The format of the AcquireMagickMemory method is:</p>

<pre class="text">
void *AcquireMagickMemory(const size_t size)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>size</dt>
<dd>the size of the memory in bytes to allocate. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="AcquireQuantumMemory">AcquireQuantumMemory</a></h2>

<p>AcquireQuantumMemory() returns a pointer to a block of memory at least count * quantum bytes suitably aligned for any use.</p>

<p>The format of the AcquireQuantumMemory method is:</p>

<pre class="text">
void *AcquireQuantumMemory(const size_t count,const size_t quantum)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>count</dt>
<dd>the number of quantum elements to allocate. </dd>

<dd> </dd>
<dt>quantum</dt>
<dd>the number of bytes in each quantum. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="AcquireVirtualMemory">AcquireVirtualMemory</a></h2>

<p>AcquireVirtualMemory() allocates a pointer to a block of memory at least size bytes suitably aligned for any use.</p>

<p>The format of the AcquireVirtualMemory method is:</p>

<pre class="text">
MemoryInfo *AcquireVirtualMemory(const size_t count,const size_t quantum)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>count</dt>
<dd>the number of quantum elements to allocate. </dd>

<dd> </dd>
<dt>quantum</dt>
<dd>the number of bytes in each quantum. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="CopyMagickMemory">CopyMagickMemory</a></h2>

<p>CopyMagickMemory() copies size bytes from memory area source to the destination.  Copying between objects that overlap will take place correctly.  It returns destination.</p>

<p>The format of the CopyMagickMemory method is:</p>

<pre class="text">
void *CopyMagickMemory(void *destination,const void *source,
  const size_t size)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>destination</dt>
<dd>the destination. </dd>

<dd> </dd>
<dt>source</dt>
<dd>the source. </dd>

<dd> </dd>
<dt>size</dt>
<dd>the size of the memory in bytes to allocate. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="GetMagickMemoryMethods">GetMagickMemoryMethods</a></h2>

<p>GetMagickMemoryMethods() gets the methods to acquire, resize, and destroy memory.</p>

<p>The format of the GetMagickMemoryMethods() method is:</p>

<pre class="text">
void GetMagickMemoryMethods(AcquireMemoryHandler *acquire_memory_handler,
  ResizeMemoryHandler *resize_memory_handler,
  DestroyMemoryHandler *destroy_memory_handler)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>acquire_memory_handler</dt>
<dd>method to acquire memory (e.g. malloc). </dd>

<dd> </dd>
<dt>resize_memory_handler</dt>
<dd>method to resize memory (e.g. realloc). </dd>

<dd> </dd>
<dt>destroy_memory_handler</dt>
<dd>method to destroy memory (e.g. free). </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="GetVirtualMemoryBlob">GetVirtualMemoryBlob</a></h2>

<p>GetVirtualMemoryBlob() returns the virtual memory blob associated with the specified MemoryInfo structure.</p>

<p>The format of the GetVirtualMemoryBlob method is:</p>

<pre class="text">
void *GetVirtualMemoryBlob(const MemoryInfo *memory_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory_info</dt>
<dd>The MemoryInfo structure.  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="RelinquishAlignedMemory">RelinquishAlignedMemory</a></h2>

<p>RelinquishAlignedMemory() frees memory acquired with AcquireAlignedMemory() or reuse.</p>

<p>The format of the RelinquishAlignedMemory method is:</p>

<pre class="text">
void *RelinquishAlignedMemory(void *memory)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory</dt>
<dd>A pointer to a block of memory to free for reuse. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="RelinquishMagickMemory">RelinquishMagickMemory</a></h2>

<p>RelinquishMagickMemory() frees memory acquired with AcquireMagickMemory() or AcquireQuantumMemory() for reuse.</p>

<p>The format of the RelinquishMagickMemory method is:</p>

<pre class="text">
void *RelinquishMagickMemory(void *memory)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory</dt>
<dd>A pointer to a block of memory to free for reuse. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="RelinquishVirtualMemory">RelinquishVirtualMemory</a></h2>

<p>RelinquishVirtualMemory() frees memory acquired with AcquireVirtualMemory().</p>

<p>The format of the RelinquishVirtualMemory method is:</p>

<pre class="text">
MemoryInfo *RelinquishVirtualMemory(MemoryInfo *memory_info)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory_info</dt>
<dd>A pointer to a block of memory to free for reuse. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="ResetMagickMemory">ResetMagickMemory</a></h2>

<p>ResetMagickMemory() fills the first size bytes of the memory area pointed to by memory with the constant byte c.</p>

<p>The format of the ResetMagickMemory method is:</p>

<pre class="text">
void *ResetMagickMemory(void *memory,int byte,const size_t size)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory</dt>
<dd>a pointer to a memory allocation. </dd>

<dd> </dd>
<dt>byte</dt>
<dd>set the memory to this value. </dd>

<dd> </dd>
<dt>size</dt>
<dd>size of the memory to reset. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="ResizeMagickMemory">ResizeMagickMemory</a></h2>

<p>ResizeMagickMemory() changes the size of the memory and returns a pointer to the (possibly moved) block.  The contents will be unchanged up to the lesser of the new and old sizes.</p>

<p>The format of the ResizeMagickMemory method is:</p>

<pre class="text">
void *ResizeMagickMemory(void *memory,const size_t size)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory</dt>
<dd>A pointer to a memory allocation. </dd>

<dd> </dd>
<dt>size</dt>
<dd>the new size of the allocated memory. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="ResizeQuantumMemory">ResizeQuantumMemory</a></h2>

<p>ResizeQuantumMemory() changes the size of the memory and returns a pointer to the (possibly moved) block.  The contents will be unchanged up to the lesser of the new and old sizes.</p>

<p>The format of the ResizeQuantumMemory method is:</p>

<pre class="text">
void *ResizeQuantumMemory(void *memory,const size_t count,
  const size_t quantum)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>memory</dt>
<dd>A pointer to a memory allocation. </dd>

<dd> </dd>
<dt>count</dt>
<dd>the number of quantum elements to allocate. </dd>

<dd> </dd>
<dt>quantum</dt>
<dd>the number of bytes in each quantum. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/memory_8c.html" id="SetMagickMemoryMethods">SetMagickMemoryMethods</a></h2>

<p>SetMagickMemoryMethods() sets the methods to acquire, resize, and destroy memory. Your custom memory methods must be set prior to the MagickCoreGenesis() method.</p>

<p>The format of the SetMagickMemoryMethods() method is:</p>

<pre class="text">
SetMagickMemoryMethods(AcquireMemoryHandler acquire_memory_handler,
  ResizeMemoryHandler resize_memory_handler,
  DestroyMemoryHandler destroy_memory_handler)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>acquire_memory_handler</dt>
<dd>method to acquire memory (e.g. malloc). </dd>

<dd> </dd>
<dt>resize_memory_handler</dt>
<dd>method to resize memory (e.g. realloc). </dd>

<dd> </dd>
<dt>destroy_memory_handler</dt>
<dd>method to destroy memory (e.g. free). </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="memory.php#">Back to top</a> •
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





<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="google-site-verification" content="_bMOCDpkx9ZAzBwb2kF3PRHbfUUdFj2uO8Jd1AXArz4" />
    <title>ImageMagick: MagickCore, C API for ImageMagick: Morphological Erosions, Dilations, Openings, and Closings</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8"/>
  <meta name="application-name" content="ImageMagick"/>
  <meta name="description" content="ImageMagick® is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, JPEG-2000, GIF, WebP, Postscript, PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves."/>
  <meta name="application-url" content="http://www.imagemagick.org"/>
  <meta name="generator" content="PHP"/>
  <meta name="keywords" content="magickcore, c, api, for, imagemagick:, morphological, erosions, dilations, openings, closings, ImageMagick, PerlMagick, image processing, image, photo, software, Magick++, OpenMP, convert"/>
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
<p class="text-center"><a href="morphology.php#AcquireKernelInfo">AcquireKernelInfo</a> &bull; <a href="morphology.php#AcquireKernelBuiltIn">AcquireKernelBuiltIn</a> &bull; <a href="morphology.php#CloneKernelInfo">CloneKernelInfo</a> &bull; <a href="morphology.php#DestroyKernelInfo">DestroyKernelInfo</a> &bull; <a href="morphology.php#MorphologyApply">MorphologyApply</a> &bull; <a href="morphology.php#This is almost identical to the MorphologyPrimative">This is almost identical to the MorphologyPrimative</a> &bull; <a href="morphology.php#MorphologyImage">MorphologyImage</a> &bull; <a href="morphology.php#ScaleGeometryKernelInfo">ScaleGeometryKernelInfo</a> &bull; <a href="morphology.php#ScaleKernelInfo">ScaleKernelInfo</a> &bull; <a href="morphology.php#ShowKernelInfo">ShowKernelInfo</a> &bull; <a href="morphology.php#UnityAddKernelInfo">UnityAddKernelInfo</a> &bull; <a href="morphology.php#ZeroKernelNans">ZeroKernelNans</a></p>

<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="AcquireKernelInfo">AcquireKernelInfo</a></h2>

<p>AcquireKernelInfo() takes the given string (generally supplied by the user) and converts it into a Morphology/Convolution Kernel.  This allows users to specify a kernel from a number of pre-defined kernels, or to fully specify their own kernel for a specific Convolution or Morphology Operation.</p>

<p>The kernel so generated can be any rectangular array of floating point values (doubles) with the 'control point' or 'pixel being affected' anywhere within that array of values.</p>

<p>Previously IM was restricted to a square of odd size using the exact center as origin, this is no longer the case, and any rectangular kernel with any value being declared the origin. This in turn allows the use of highly asymmetrical kernels.</p>

<p>The floating point values in the kernel can also include a special value known as 'nan' or 'not a number' to indicate that this value is not part of the kernel array. This allows you to shaped the kernel within its rectangular area. That is 'nan' values provide a 'mask' for the kernel shape.  However at least one non-nan value must be provided for correct working of a kernel.</p>

<p>The returned kernel should be freed using the DestroyKernelInfo() when you are finished with it.  Do not free this memory yourself.</p>

<p>Input kernel defintion strings can consist of any of three types.</p>

<p>"name:args[[@&gt;&lt;]" Select from one of the built in kernels, using the name and geometry arguments supplied.  See AcquireKernelBuiltIn()</p>

<p>"WxH[+X+Y][@&gt;&lt;]:num, num, num ..." a kernel of size W by H, with W*H floating point numbers following. the 'center' can be optionally be defined at +X+Y (such that +0+0 is top left corner). If not defined the pixel in the center, for odd sizes, or to the immediate top or left of center for even sizes is automatically selected.</p>

<p>"num, num, num, num, ..." list of floating point numbers defining an 'old style' odd sized square kernel.  At least 9 values should be provided for a 3x3 square kernel, 25 for a 5x5 square kernel, 49 for 7x7, etc. Values can be space or comma separated.  This is not recommended.</p>

<p>You can define a 'list of kernels' which can be used by some morphology operators A list is defined as a semi-colon separated list kernels.</p>

<p>" kernel ; kernel ; kernel ; "</p>

<p>Any extra ';' characters, at start, end or between kernel defintions are simply ignored.</p>

<p>The special flags will expand a single kernel, into a list of rotated kernels. A '@' flag will expand a 3x3 kernel into a list of 45-degree cyclic rotations, while a '&gt;' will generate a list of 90-degree rotations. The '&lt;' also exands using 90-degree rotates, but giving a 180-degree reflected kernel before the +/- 90-degree rotations, which can be important for Thinning operations.</p>

<p>Note that 'name' kernels will start with an alphabetic character while the new kernel specification has a ':' character in its specification string. If neither is the case, it is assumed an old style of a simple list of numbers generating a odd-sized square kernel has been given.</p>

<p>The format of the AcquireKernal method is:</p>

<pre class="text">
KernelInfo *AcquireKernelInfo(const char *kernel_string)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>kernel_string</dt>
<dd>the Morphology/Convolution kernel wanted. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="AcquireKernelBuiltIn">AcquireKernelBuiltIn</a></h2>

<p>AcquireKernelBuiltIn() returned one of the 'named' built-in types of kernels used for special purposes such as gaussian blurring, skeleton pruning, and edge distance determination.</p>

<p>They take a KernelType, and a set of geometry style arguments, which were typically decoded from a user supplied string, or from a more complex Morphology Method that was requested.</p>

<p>The format of the AcquireKernalBuiltIn method is:</p>

<pre class="text">
KernelInfo *AcquireKernelBuiltIn(const KernelInfoType type,
     const GeometryInfo args)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>type</dt>
<dd>the pre-defined type of kernel wanted </dd>

<dd> </dd>
<dt>args</dt>
<dd>arguments defining or modifying the kernel </dd>

<dd> Convolution Kernels </dd>

<dd> Unity The a No-Op or Scaling single element kernel. </dd>

<dd> Gaussian:{radius},{sigma} Generate a two-dimensional gaussian kernel, as used by -gaussian. The sigma for the curve is required.  The resulting kernel is normalized, </dd>

<dd> If 'sigma' is zero, you get a single pixel on a field of zeros. </dd>

<dd> NOTE: that the 'radius' is optional, but if provided can limit (clip) the final size of the resulting kernel to a square 2*radius+1 in size. The radius should be at least 2 times that of the sigma value, or sever clipping and aliasing may result.  If not given or set to 0 the radius will be determined so as to produce the best minimal error result, which is usally much larger than is normally needed. </dd>

<dd> LoG:{radius},{sigma} "Laplacian of a Gaussian" or "Mexician Hat" Kernel. The supposed ideal edge detection, zero-summing kernel. </dd>

<dd> An alturnative to this kernel is to use a "DoG" with a sigma ratio of approx 1.6 (according to wikipedia). </dd>

<dd> DoG:{radius},{sigma1},{sigma2} "Difference of Gaussians" Kernel. As "Gaussian" but with a gaussian produced by 'sigma2' subtracted from the gaussian produced by 'sigma1'. Typically sigma2 &gt; sigma1. The result is a zero-summing kernel. </dd>

<dd> Blur:{radius},{sigma}[,{angle}] Generates a 1 dimensional or linear gaussian blur, at the angle given (current restricted to orthogonal angles).  If a 'radius' is given the kernel is clipped to a width of 2*radius+1.  Kernel can be rotated by a 90 degree angle. </dd>

<dd> If 'sigma' is zero, you get a single pixel on a field of zeros. </dd>

<dd> Note that two convolutions with two "Blur" kernels perpendicular to each other, is equivalent to a far larger "Gaussian" kernel with the same sigma value, However it is much faster to apply. This is how the "-blur" operator actually works. </dd>

<dd> Comet:{width},{sigma},{angle} Blur in one direction only, much like how a bright object leaves a comet like trail.  The Kernel is actually half a gaussian curve, Adding two such blurs in opposite directions produces a Blur Kernel. Angle can be rotated in multiples of 90 degrees. </dd>

<dd> Note that the first argument is the width of the kernel and not the radius of the kernel. </dd>

<dd> Binomial:[{radius}] Generate a discrete kernel using a 2 dimentional Pascel's Triangle of values. Used for special forma of image filters. </dd>

<dd> # Still to be implemented... # # Filter2D # Filter1D #    Set kernel values using a resize filter, and given scale (sigma) #    Cylindrical or Linear.   Is this possible with an image? # </dd>

<dd> Named Constant Convolution Kernels </dd>

<dd> All these are unscaled, zero-summing kernels by default. As such for non-HDRI version of ImageMagick some form of normalization, user scaling, and biasing the results is recommended, to prevent the resulting image being 'clipped'. </dd>

<dd> The 3x3 kernels (most of these) can be circularly rotated in multiples of 45 degrees to generate the 8 angled varients of each of the kernels. </dd>

<dd> Laplacian:{type} Discrete Lapacian Kernels, (without normalization) Type 0 :  3x3 with center:8 surounded by -1  (8 neighbourhood) Type 1 :  3x3 with center:4 edge:-1 corner:0 (4 neighbourhood) Type 2 :  3x3 with center:4 edge:1 corner:-2 Type 3 :  3x3 with center:4 edge:-2 corner:1 Type 5 :  5x5 laplacian Type 7 :  7x7 laplacian Type 15 : 5x5 LoG (sigma approx 1.4) Type 19 : 9x9 LoG (sigma approx 1.4) </dd>

<dd> Sobel:{angle} Sobel 'Edge' convolution kernel (3x3) | -1, 0, 1 | | -2, 0,-2 | | -1, 0, 1 | </dd>

<dd> Roberts:{angle} Roberts convolution kernel (3x3) |  0, 0, 0 | | -1, 1, 0 | |  0, 0, 0 | </dd>

<dd> Prewitt:{angle} Prewitt Edge convolution kernel (3x3) | -1, 0, 1 | | -1, 0, 1 | | -1, 0, 1 | </dd>

<dd> Compass:{angle} Prewitt's "Compass" convolution kernel (3x3) | -1, 1, 1 | | -1,-2, 1 | | -1, 1, 1 | </dd>

<dd> Kirsch:{angle} Kirsch's "Compass" convolution kernel (3x3) | -3,-3, 5 | | -3, 0, 5 | | -3,-3, 5 | </dd>

<dd> FreiChen:{angle} Frei-Chen Edge Detector is based on a kernel that is similar to the Sobel Kernel, but is designed to be isotropic. That is it takes into account the distance of the diagonal in the kernel. </dd>

<dd> |   1,     0,   -1     | | sqrt(2), 0, -sqrt(2) | |   1,     0,   -1     | </dd>

<dd> FreiChen:{type},{angle} </dd>

<dd> Frei-Chen Pre-weighted kernels... </dd>

<dd> Type 0:  default un-nomalized version shown above. </dd>

<dd> Type 1: Orthogonal Kernel (same as type 11 below) |   1,     0,   -1     | | sqrt(2), 0, -sqrt(2) | / 2*sqrt(2) |   1,     0,   -1     | </dd>

<dd> Type 2: Diagonal form of Kernel... |   1,     sqrt(2),    0     | | sqrt(2),   0,     -sqrt(2) | / 2*sqrt(2) |   0,    -sqrt(2)    -1     | </dd>

<dd> However this kernel is als at the heart of the FreiChen Edge Detection Process which uses a set of 9 specially weighted kernel.  These 9 kernels not be normalized, but directly applied to the image. The results is then added together, to produce the intensity of an edge in a specific direction.  The square root of the pixel value can then be taken as the cosine of the edge, and at least 2 such runs at 90 degrees from each other, both the direction and the strength of the edge can be determined. </dd>

<dd> Type 10: All 9 of the following pre-weighted kernels... </dd>

<dd> Type 11: |   1,     0,   -1     | | sqrt(2), 0, -sqrt(2) | / 2*sqrt(2) |   1,     0,   -1     | </dd>

<dd> Type 12: | 1, sqrt(2), 1 | | 0,   0,     0 | / 2*sqrt(2) | 1, sqrt(2), 1 | </dd>

<dd> Type 13: | sqrt(2), -1,    0     | |  -1,      0,    1     | / 2*sqrt(2) |   0,      1, -sqrt(2) | </dd>

<dd> Type 14: |    0,     1, -sqrt(2) | |   -1,     0,     1    | / 2*sqrt(2) | sqrt(2), -1,     0    | </dd>

<dd> Type 15: | 0, -1, 0 | | 1,  0, 1 | / 2 | 0, -1, 0 | </dd>

<dd> Type 16: |  1, 0, -1 | |  0, 0,  0 | / 2 | -1, 0,  1 | </dd>

<dd> Type 17: |  1, -2,  1 | | -2,  4, -2 | / 6 | -1, -2,  1 | </dd>

<dd> Type 18: | -2, 1, -2 | |  1, 4,  1 | / 6 | -2, 1, -2 | </dd>

<dd> Type 19: | 1, 1, 1 | | 1, 1, 1 | / 3 | 1, 1, 1 | </dd>

<dd> The first 4 are for edge detection, the next 4 are for line detection and the last is to add a average component to the results. </dd>

<dd> Using a special type of '-1' will return all 9 pre-weighted kernels as a multi-kernel list, so that you can use them directly (without normalization) with the special "-set option:morphology:compose Plus" setting to apply the full FreiChen Edge Detection Technique. </dd>

<dd> If 'type' is large it will be taken to be an actual rotation angle for the default FreiChen (type 0) kernel.  As such  FreiChen:45  will look like a  Sobel:45  but with 'sqrt(2)' instead of '2' values. </dd>

<dd> WARNING: The above was layed out as per http://www.math.tau.ac.il/~turkel/notes/edge_detectors.pdf But rotated 90 degrees so direction is from left rather than the top. I have yet to find any secondary confirmation of the above. The only other source found was actual source code at http://ltswww.epfl.ch/~courstiv/exos_labos/sol3.pdf Neigher paper defineds the kernels in a way that looks locical or correct when taken as a whole. </dd>

<dd> Boolean Kernels </dd>

<dd> Diamond:[{radius}[,{scale}]] Generate a diamond shaped kernel with given radius to the points. Kernel size will again be radius*2+1 square and defaults to radius 1, generating a 3x3 kernel that is slightly larger than a square. </dd>

<dd> Square:[{radius}[,{scale}]] Generate a square shaped kernel of size radius*2+1, and defaulting to a 3x3 (radius 1). </dd>

<dd> Octagon:[{radius}[,{scale}]] Generate octagonal shaped kernel of given radius and constant scale. Default radius is 3 producing a 7x7 kernel. A radius of 1 will result in "Diamond" kernel. </dd>

<dd> Disk:[{radius}[,{scale}]] Generate a binary disk, thresholded at the radius given, the radius may be a float-point value. Final Kernel size is floor(radius)*2+1 square. A radius of 5.3 is the default. </dd>

<dd> NOTE: That a low radii Disk kernels produce the same results as many of the previously defined kernels, but differ greatly at larger radii.  Here is a table of equivalences... "Disk:1"    =&gt; "Diamond", "Octagon:1", or "Cross:1" "Disk:1.5"  =&gt; "Square" "Disk:2"    =&gt; "Diamond:2" "Disk:2.5"  =&gt; "Octagon" "Disk:2.9"  =&gt; "Square:2" "Disk:3.5"  =&gt; "Octagon:3" "Disk:4.5"  =&gt; "Octagon:4" "Disk:5.4"  =&gt; "Octagon:5" "Disk:6.4"  =&gt; "Octagon:6" All other Disk shapes are unique to this kernel, but because a "Disk" is more circular when using a larger radius, using a larger radius is preferred over iterating the morphological operation. </dd>

<dd> Rectangle:{geometry} Simply generate a rectangle of 1's with the size given. You can also specify the location of the 'control point', otherwise the closest pixel to the center of the rectangle is selected. </dd>

<dd> Properly centered and odd sized rectangles work the best. </dd>

<dd> Symbol Dilation Kernels </dd>

<dd> These kernel is not a good general morphological kernel, but is used more for highlighting and marking any single pixels in an image using, a "Dilate" method as appropriate. </dd>

<dd> For the same reasons iterating these kernels does not produce the same result as using a larger radius for the symbol. </dd>

<dd> Plus:[{radius}[,{scale}]] Cross:[{radius}[,{scale}]] Generate a kernel in the shape of a 'plus' or a 'cross' with a each arm the length of the given radius (default 2). </dd>

<dd> NOTE: "plus:1" is equivalent to a "Diamond" kernel. </dd>

<dd> Ring:{radius1},{radius2}[,{scale}] A ring of the values given that falls between the two radii. Defaults to a ring of approximataly 3 radius in a 7x7 kernel. This is the 'edge' pixels of the default "Disk" kernel, More specifically, "Ring" -&gt; "Ring:2.5,3.5,1.0" </dd>

<dd> Hit and Miss Kernels </dd>

<dd> Peak:radius1,radius2 Find any peak larger than the pixels the fall between the two radii. The default ring of pixels is as per "Ring". Edges Find flat orthogonal edges of a binary shape Corners Find 90 degree corners of a binary shape Diagonals:type A special kernel to thin the 'outside' of diagonals LineEnds:type Find end points of lines (for pruning a skeletion) Two types of lines ends (default to both) can be searched for Type 0: All line ends Type 1: single kernel for 4-conneected line ends Type 2: single kernel for simple line ends LineJunctions Find three line junctions (within a skeletion) Type 0: all line junctions Type 1: Y Junction kernel Type 2: Diagonal T Junction kernel Type 3: Orthogonal T Junction kernel Type 4: Diagonal X Junction kernel Type 5: Orthogonal + Junction kernel Ridges:type Find single pixel ridges or thin lines Type 1: Fine single pixel thick lines and ridges Type 2: Find two pixel thick lines and ridges ConvexHull Octagonal Thickening Kernel, to generate convex hulls of 45 degrees Skeleton:type Traditional skeleton generating kernels. Type 1: Tradional Skeleton kernel (4 connected skeleton) Type 2: HIPR2 Skeleton kernel (8 connected skeleton) Type 3: Thinning skeleton based on a ressearch paper by Dan S. Bloomberg (Default Type) ThinSE:type A huge variety of Thinning Kernels designed to preserve conectivity. many other kernel sets use these kernels as source definitions. Type numbers are 41-49, 81-89, 481, and 482 which are based on the super and sub notations used in the source research paper. </dd>

<dd> Distance Measuring Kernels </dd>

<dd> Different types of distance measuring methods, which are used with the a 'Distance' morphology method for generating a gradient based on distance from an edge of a binary shape, though there is a technique for handling a anti-aliased shape. </dd>

<dd> See the 'Distance' Morphological Method, for information of how it is applied. </dd>

<dd> Chebyshev:[{radius}][x{scale}[!]] Chebyshev Distance (also known as Tchebychev or Chessboard distance) is a value of one to any neighbour, orthogonal or diagonal. One why of thinking of it is the number of squares a 'King' or 'Queen' in chess needs to traverse reach any other position on a chess board. It results in a 'square' like distance function, but one where diagonals are given a value that is closer than expected. </dd>

<dd> Manhattan:[{radius}][x{scale}[!]] Manhattan Distance (also known as Rectilinear, City Block, or the Taxi Cab distance metric), it is the distance needed when you can only travel in horizontal or vertical directions only.  It is the distance a 'Rook' in chess would have to travel, and results in a diamond like distances, where diagonals are further than expected. </dd>

<dd> Octagonal:[{radius}][x{scale}[!]] An interleving of Manhatten and Chebyshev metrics producing an increasing octagonally shaped distance.  Distances matches those of the "Octagon" shaped kernel of the same radius.  The minimum radius and default is 2, producing a 5x5 kernel. </dd>

<dd> Euclidean:[{radius}][x{scale}[!]] Euclidean distance is the 'direct' or 'as the crow flys' distance. However by default the kernel size only has a radius of 1, which limits the distance to 'Knight' like moves, with only orthogonal and diagonal measurements being correct.  As such for the default kernel you will get octagonal like distance function. </dd>

<dd> However using a larger radius such as "Euclidean:4" you will get a much smoother distance gradient from the edge of the shape. Especially if the image is pre-processed to include any anti-aliasing pixels. Of course a larger kernel is slower to use, and not always needed. </dd>

<dd> The first three Distance Measuring Kernels will only generate distances of exact multiples of {scale} in binary images. As such you can use a scale of 1 without loosing any information.  However you also need some scaling when handling non-binary anti-aliased shapes. </dd>

<dd> The "Euclidean" Distance Kernel however does generate a non-integer fractional results, and as such scaling is vital even for binary shapes. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="CloneKernelInfo">CloneKernelInfo</a></h2>

<p>CloneKernelInfo() creates a new clone of the given Kernel List so that its can be modified without effecting the original.  The cloned kernel should be destroyed using DestoryKernelInfo() when no longer needed.</p>

<p>The format of the CloneKernelInfo method is:</p>

<pre class="text">
KernelInfo *CloneKernelInfo(const KernelInfo *kernel)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>kernel</dt>
<dd>the Morphology/Convolution kernel to be cloned </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="DestroyKernelInfo">DestroyKernelInfo</a></h2>

<p>DestroyKernelInfo() frees the memory used by a Convolution/Morphology kernel.</p>

<p>The format of the DestroyKernelInfo method is:</p>

<pre class="text">
KernelInfo *DestroyKernelInfo(KernelInfo *kernel)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>kernel</dt>
<dd>the Morphology/Convolution kernel to be destroyed </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="MorphologyApply">MorphologyApply</a></h2>

<p>MorphologyApply() applies a morphological method, multiple times using a list of multiple kernels.  This is the method that should be called by other 'operators' that internally use morphology operations as part of their processing.</p>

<p>It is basically equivalent to as MorphologyImage() (see below) but without any user controls.  This allows internel programs to use this method to perform a specific task without possible interference by any API user supplied settings.</p>

<p>It is MorphologyImage() task to extract any such user controls, and pass them to this function for processing.</p>

<p>More specifically all given kernels should already be scaled, normalised, and blended appropriatally before being parred to this routine. The appropriate bias, and compose (typically 'UndefinedComposeOp') given.</p>

<p>The format of the MorphologyApply method is:</p>

<pre class="text">
Image *MorphologyApply(const Image *image,MorphologyMethod method,
  const ssize_t iterations,const KernelInfo *kernel,
  const CompositeMethod compose,const double bias,
  ExceptionInfo *exception)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>image</dt>
<dd>the source image </dd>

<dd> </dd>
<dt>method</dt>
<dd>the morphology method to be applied. </dd>

<dd> </dd>
<dt>iterations</dt>
<dd>apply the operation this many times (or no change). A value of -1 means loop until no change found. How this is applied may depend on the morphology method. Typically this is a value of 1. </dd>

<dd> </dd>
<dt>channel</dt>
<dd>the channel type. </dd>

<dd> </dd>
<dt>kernel</dt>
<dd>An array of double representing the morphology kernel. </dd>

<dd> </dd>
<dt>compose</dt>
<dd>How to handle or merge multi-kernel results. If 'UndefinedCompositeOp' use default for the Morphology method. If 'NoCompositeOp' force image to be re-iterated by each kernel. Otherwise merge the results using the compose method given. </dd>

<dd> </dd>
<dt>bias</dt>
<dd>Convolution Output Bias. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="This_is almost identical to the MorphologyPrimative">This is almost identical to the MorphologyPrimative</a></h2>

<p>This is almost identical to the MorphologyPrimative() function above, but applies the primitive directly to the actual image using two passes, once in each direction, with the results of the previous (and current) row being re-used.</p>

<p>That is after each row is 'Sync'ed' into the image, the next row makes use of those values as part of the calculation of the next row.  It repeats, but going in the oppisite (bottom-up) direction.</p>

<p>Because of this 're-use of results' this function can not make use of multi- threaded, parellel processing. </p>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="MorphologyImage">MorphologyImage</a></h2>

<p>MorphologyImage() applies a user supplied kernel to the image according to the given mophology method.</p>

<p>This function applies any and all user defined settings before calling the above internal function MorphologyApply().</p>

<p>User defined settings include... * Output Bias for Convolution and correlation ("-define convolve:bias=??") * Kernel Scale/normalize settings            ("-define convolve:scale=??") This can also includes the addition of a scaled unity kernel. * Show Kernel being applied            ("-define morphology:showkernel=1")</p>

<p>Other operators that do not want user supplied options interfering, especially "convolve:bias" and "morphology:showkernel" should use MorphologyApply() directly.</p>

<p>The format of the MorphologyImage method is:</p>

<pre class="text">
Image *MorphologyImage(const Image *image,MorphologyMethod method,
  const ssize_t iterations,KernelInfo *kernel,ExceptionInfo *exception)
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
<dd>the morphology method to be applied. </dd>

<dd> </dd>
<dt>iterations</dt>
<dd>apply the operation this many times (or no change). A value of -1 means loop until no change found. How this is applied may depend on the morphology method. Typically this is a value of 1. </dd>

<dd> </dd>
<dt>kernel</dt>
<dd>An array of double representing the morphology kernel. Warning: kernel may be normalized for the Convolve method. </dd>

<dd> </dd>
<dt>exception</dt>
<dd>return any errors or warnings in this structure. </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="ScaleGeometryKernelInfo">ScaleGeometryKernelInfo</a></h2>

<p>ScaleGeometryKernelInfo() takes a geometry argument string, typically provided as a  "-set option:convolve:scale {geometry}" user setting, and modifies the kernel according to the parsed arguments of that setting.</p>

<p>The first argument (and any normalization flags) are passed to ScaleKernelInfo() to scale/normalize the kernel.  The second argument is then passed to UnityAddKernelInfo() to add a scled unity kernel into the scaled/normalized kernel.</p>

<p>The format of the ScaleGeometryKernelInfo method is:</p>

<pre class="text">
void ScaleGeometryKernelInfo(KernelInfo *kernel,
  const double scaling_factor,const MagickStatusType normalize_flags)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>kernel</dt>
<dd>the Morphology/Convolution kernel to modify </dd>

<dd> o geometry: </dd>

<pre class="text">
       "-set option:convolve:scale {geometry}" setting.
</pre>

<p></dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="ScaleKernelInfo">ScaleKernelInfo</a></h2>

<p>ScaleKernelInfo() scales the given kernel list by the given amount, with or without normalization of the sum of the kernel values (as per given flags).</p>

<p>By default (no flags given) the values within the kernel is scaled directly using given scaling factor without change.</p>

<p>If either of the two 'normalize_flags' are given the kernel will first be normalized and then further scaled by the scaling factor value given.</p>

<p>Kernel normalization ('normalize_flags' given) is designed to ensure that any use of the kernel scaling factor with 'Convolve' or 'Correlate' morphology methods will fall into -1.0 to +1.0 range.  Note that for non-HDRI versions of IM this may cause images to have any negative results clipped, unless some 'bias' is used.</p>

<p>More specifically.  Kernels which only contain positive values (such as a 'Gaussian' kernel) will be scaled so that those values sum to +1.0, ensuring a 0.0 to +1.0 output range for non-HDRI images.</p>

<p>For Kernels that contain some negative values, (such as 'Sharpen' kernels) the kernel will be scaled by the absolute of the sum of kernel values, so that it will generally fall within the +/- 1.0 range.</p>

<p>For kernels whose values sum to zero, (such as 'Laplician' kernels) kernel will be scaled by just the sum of the postive values, so that its output range will again fall into the  +/- 1.0 range.</p>

<p>For special kernels designed for locating shapes using 'Correlate', (often only containing +1 and -1 values, representing foreground/brackground matching) a special normalization method is provided to scale the positive values separately to those of the negative values, so the kernel will be forced to become a zero-sum kernel better suited to such searches.</p>

<p>WARNING: Correct normalization of the kernel assumes that the '*_range' attributes within the kernel structure have been correctly set during the kernels creation.</p>

<p>NOTE: The values used for 'normalize_flags' have been selected specifically to match the use of geometry options, so that '!' means NormalizeValue, '^' means CorrelateNormalizeValue.  All other GeometryFlags values are ignored.</p>

<p>The format of the ScaleKernelInfo method is:</p>

<pre class="text">
void ScaleKernelInfo(KernelInfo *kernel, const double scaling_factor,
         const MagickStatusType normalize_flags )
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>kernel</dt>
<dd>the Morphology/Convolution kernel </dd>

<dd> o scaling_factor: </dd>

<pre class="text">
       zero.  If the kernel is normalized regardless of any flags.
</pre>

<p>o normalize_flags: </dd>

<pre class="text">
       specifically: NormalizeValue, CorrelateNormalizeValue,
                     and/or PercentValue
</pre>

<p></dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="ShowKernelInfo">ShowKernelInfo</a></h2>

<p>ShowKernelInfo() outputs the details of the given kernel defination to standard error, generally due to a users 'morphology:showkernel' option request.</p>

<p>The format of the ShowKernel method is:</p>

<pre class="text">
void ShowKernelInfo(const KernelInfo *kernel)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>kernel</dt>
<dd>the Morphology/Convolution kernel </dd>

<dd>  </dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="UnityAddKernelInfo">UnityAddKernelInfo</a></h2>

<p>UnityAddKernelInfo() Adds a given amount of the 'Unity' Convolution Kernel to the given pre-scaled and normalized Kernel.  This in effect adds that amount of the original image into the resulting convolution kernel.  This value is usually provided by the user as a percentage value in the 'convolve:scale' setting.</p>

<p>The resulting effect is to convert the defined kernels into blended soft-blurs, unsharp kernels or into sharpening kernels.</p>

<p>The format of the UnityAdditionKernelInfo method is:</p>

<pre class="text">
void UnityAdditionKernelInfo(KernelInfo *kernel, const double scale )
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>kernel</dt>
<dd>the Morphology/Convolution kernel </dd>

<dd> o scale: </dd>

<pre class="text">
       the given kernel.
</pre>

<p></dd>
</dl>
<h2><a href="http://nextgen.imagemagick.org/api/MagickCore/morphology_8c.html" id="ZeroKernelNans">ZeroKernelNans</a></h2>

<p>ZeroKernelNans() replaces any special 'nan' value that may be present in the kernel with a zero value.  This is typically done when the kernel will be used in special hardware (GPU) convolution processors, to simply matters.</p>

<p>The format of the ZeroKernelNans method is:</p>

<pre class="text">
void ZeroKernelNans (KernelInfo *kernel)
</pre>

<p>A description of each parameter follows:</p>

<dd>
</dd>

<dd> </dd>
<dl class="dl-horizontal">
<dt>kernel</dt>
<dd>the Morphology/Convolution kernel </dd>

<dd>  </dd>
</dl>
</div>
  <footer class="magick-footer">
    <p><a href="../script/support.php">Donate</a> •
     <a href="../script/sitemap.php">Sitemap</a> •
    <a href="../script/links.php">Related</a> •
    <a href="../script/architecture.php">Architecture</a>
</p>
    <p><a href="morphology.php#">Back to top</a> •
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

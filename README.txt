Introduction to ImageMagick

  ImageMagick® is a software suite to create, edit, compose, or convert
  bitmap images. It can read and write images in a variety of formats (over
  200) including PNG, JPEG, GIF, HEIC, TIFF, DPX, EXR, WebP, Postscript,
  PDF, and SVG. Use ImageMagick to resize, flip, mirror, rotate, distort,
  shear and transform images, adjust image colors, apply various special
  effects, or draw text, lines, polygons, ellipses and Bezier curves.
  
  The functionality of ImageMagick is typically utilized from the command
  line or you can use the features from programs written in your favorite
  language. Choose from these interfaces: G2F (Ada), MagickCore (C),
  MagickWand (C), ChMagick (Ch), Magick++ (C++),
  JMagick (Java), L-Magick (Lisp), Lua, NMagick (Neko/haXe), Magick.NET
  (.NET), PascalMagick (Pascal), PerlMagick (Perl), MagickWand for PHP
  (PHP), IMagick (PHP), PythonMagick (Python), RMagick (Ruby), or TclMagick
  (Tcl/TK). With a language interface, use ImageMagick to modify or create
  images dynamically and automagically.

  ImageMagick utilize multiple computational threads to increase performance
  and can read, process, or write mega-, giga-, or tera-pixel image sizes.
  
  ImageMagick is free software delivered as a ready-to-run binary distribution
  or as source code that you may use, copy, modify, and distribute in both open
  and proprietary applications. It is distributed under a derived Apache 2.0
  license.
  
  The ImageMagick development process ensures a stable API and ABI. Before
  each ImageMagick release, we perform a comprehensive security assessment
  that includes memory error and thread data race detection to prevent
  security vulnerabilities.

  The current release is the ImageMagick 7.0.11-* series. It runs on Linux,
  Windows, Mac Os X, iOS, Android OS, and others.

  The authoritative ImageMagick web site is https://imagemagick.org. The
  authoritative source code repository is
  https://github.com/ImageMagick/ImageMagick.

  We continue to maintain the legacy release of ImageMagick, version 6,
  at https://legacy.imagemagick.org.


Features and Capabilities
  
  Here are just a few examples of what ImageMagick can do:
  
      * Format conversion: convert an image from one format to another (e.g.
        PNG to JPEG).
      * Transform: resize, rotate, deskew, crop, flip or trim an image.
      * Transparency: render portions of an image invisible.
      * Draw: add shapes or text to an image.
      * Decorate: add a border or frame to an image.
      * Special effects: blur, sharpen, threshold, or tint an image.
      * Animation: create a GIF animation sequence from a group of images.
      * Text & comments: insert descriptive or artistic text in an image.
      * Image gradients: create a gradual blend of one color whose shape is 
        horizontal, vertical, circular, or elliptical.
      * Image identification: describe the format and attributes of an image.
      * Composite: overlap one image over another.
      * Montage: juxtapose image thumbnails on an image canvas.
      * Generalized pixel distortion: correct for, or induce image distortions
        including perspective.
      * Computer vision: Canny edge detection.
      * Morphology of shapes: extract features, describe shapes and recognize
        patterns in images.
      * Motion picture support: read and write the common image formats used in
        digital film work.
      * Image calculator: apply a mathematical expression to an image or image
        channels.
      * Connected component labeling: uniquely label connected regions in an
        image.
      * Discrete Fourier transform: implements the forward and inverse DFT.
      * Perceptual hash: maps visually identical images to the same or similar
        hash-- useful in image retrieval, authentication, indexing, or copy
        detection as well as digital watermarking.
      * Complex text layout: bidirectional text support and shaping.
      * Color management: accurate color management with color profiles or in
        lieu of-- built-in gamma compression or expansion as demanded by the
        colorspace.
      * Bilateral blur: non-linear, edge-preserving, and noise-reducing
        smoothing filter.
      * High dynamic-range images: accurately represent the wide range of
        intensity levels found in real scenes ranging from the brightest direct
        sunlight to the deepest darkest shadows.
      * Encipher or decipher an image: convert ordinary images into
        unintelligible gibberish and back again.
      * Virtual pixel support: convenient access to pixels outside the image
        region.
      * Large image support: read, process, or write mega-, giga-, or
        tera-pixel image sizes.
      * Threads of execution support: ImageMagick is thread safe and most
        internal algorithms are OpenMP-enabled to take advantage of speed-ups
        offered by multicore processor chips.
      * Distributed pixel cache: offload intermediate pixel storage to one or
        more remote servers.
      * Heterogeneous distributed processing: certain algorithms are
        OpenCL-enabled to take advantage of speed-ups offered by executing in
        concert across heterogeneous platforms consisting of CPUs, GPUs, and
        other processors.
      * ImageMagick on the iPhone: convert, edit, or compose images on your
        iPhone or iPad.
  
  Examples of ImageMagick Usage * https://legacy.imagemagick.org/Usage/
  shows how to use ImageMagick from the command-line to accomplish any
  of these tasks and much more. Also, see Fred's ImageMagick Scripts @
  http://www.fmwconcepts.com/imagemagick/: a plethora of command-line scripts
  that perform geometric transforms, blurs, sharpens, edging, noise removal,
  and color manipulations. With Magick.NET, use ImageMagick without having
  to install ImageMagick on your server or desktop.


News

  ImageMagick best practices strongly encourages you to configure a security
  policy that suits your local environment.

  Now that ImageMagick version 7 is released, we continue
  to maintain the legacy release of ImageMagick, version 6, at
  https://legacy.imagemagick.org. Learn how ImageMagick version 7 differs
  from previous versions with our porting guide.

  Want more performance from ImageMagick? Try these options:

    * add more memory to your system, see the pixel cache;
    * add more cores to your system, see threads of execution support;
    * reduce lock contention with the tcmalloc memory allocation library;
    * push large images to a solid-state drive, see large image support.

  If these options are prohibitive, you can reduce the quality of the image
  results. The default build is Q16 HDRI. If you disable HDRI, you use
  half the memory and instead of predominantly floating point operations,
  you use the typically more efficient integer operations. The tradeoff
  is reduced precision and you cannot process out of range pixel values
  (e.g. negative). If you build the Q8 non-HDRI version of ImageMagick,
  you again reduce the memory requirements in half-- and once again there
  is a tradeoff, even less precision and no out of range pixel values. For
  a Q8 non-HDRI build of ImageMagick, use these configure script options:
  --with-quantum-depth=8 --disable-hdri.

# ImageMagick

[![Build Status](https://github.com/ImageMagick/ImageMagick/workflows/main/badge.svg)](https://github.com/ImageMagick/ImageMagick/actions)
[![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/imagemagick.svg)](https://bugs.chromium.org/p/oss-fuzz/issues/list?sort=-opened&can=1&q=proj:imagemagick)
[![Donate](https://img.shields.io/badge/%24-donate-ff00ff.svg)](https://github.com/sponsors/ImageMagick)

<p align="center">
<img align="center" src="https://imagemagick.org/image/wizard.png" alt="ImageMagick logo" width="265"/>
</p>

Use [ImageMagick®](https://imagemagick.org/) to create, edit, compose, or convert digital images. It can read and write images in a variety of formats (over 200) including PNG, JPEG, GIF, WebP, HEIC, SVG, PDF, DPX, EXR and TIFF. ImageMagick can resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves.

#### What is ImageMagick?

ImageMagick is free software delivered as a ready-to-run binary distribution or as source code that you may use, copy, modify, and distribute in both open and proprietary applications. It is distributed under a derived Apache 2.0 [license](https://imagemagick.org/script/license.php).

ImageMagick utilizes multiple computational threads to increase performance and can read, process, or write mega-, giga-, or tera-pixel image sizes.
The current release is the ImageMagick 7.1.0 series. It runs on Linux, Windows, Mac Os X, iOS, Android OS, and others.

The authoritative ImageMagick web site is https://imagemagick.org. The authoritative source code repository is https://github.com/ImageMagick/ImageMagick. We continue to maintain the legacy release of ImageMagick, version 6, at https://legacy.imagemagick.org.

#### Features and Capabilities

Here are just a few [examples](https://imagemagick.org/script/examples.php) of what ImageMagick can do:

* [Animation](https://imagemagick.org/script/command-line-options.php#bilateral-blur): non-linear, edge-preserving, and noise-reducing smoothing filter.
* [Bilateral Blur](https://imagemagick.org/script/command-line-options.php#bilateral-blur): non-linear, edge-preserving, and noise-reducing smoothing filter.
* [Color management](https://imagemagick.org/script/color-management.php): accurate color management with color profiles or in lieu of-- built-in gamma compression or expansion as demanded by the colorspace.
* [Color thresholding](https://imagemagick.org/script/color-management.php) force all pixels in the color range to white otherwise black.
* [Command-line processing](https://imagemagick.org/script/command-line-processing.php) utilize ImageMagick from the command-line.
* [Complex text layout](https://en.wikipedia.org/wiki/Complex_text_layout) bidirectional text support and shaping.
* [Composite](https://imagemagick.org/script/composite.php): overlap one image over another.
* [Connected component labeling](https://imagemagick.org/script/connected-components.php): uniquely label connected regions in an image.
* [Convex hull](https://imagemagick.org/script/convex-hull.php) smallest area convex polygon containing the image foreground objects. In addition, the minimum bounding box and unrotate angle are also generated.
* [Decorate](https://legacy.imagemagick.org/Usage/crop/): add a border or frame to an image.
* [Delineate image features](https://legacy.imagemagick.org/Usage/transform/#vision): Canny edge detection, mean-shift, Hough lines.
* [Discrete Fourier transform](https://legacy.imagemagick.org/Usage/fourier/): implements the forward and inverse [DFT](http://en.wikipedia.org/wiki/Discrete_Fourier_transform).
* [Distributed pixel cache](https://imagemagick.org/script/distribute-pixel-cache.php): offload intermediate pixel storage to one or more remote servers.
* [Draw](https://legacy.imagemagick.org/Usage/draw/): add shapes or text to an image.
* [Encipher or decipher an image](https://imagemagick.org/script/cipher.php): convert ordinary images into unintelligible gibberish and back again.
* [Format conversion](https://imagemagick.org/script/convert.php): convert an image from one [format](https://imagemagick.org/script/formats.php) to another (e.g.  PNG to JPEG).
* [Generalized pixel distortion](https://legacy.imagemagick.org/Usage/distorts/): correct for, or induce image distortions including perspective.
* [Heterogeneous distributed processing](https://imagemagick.org/script/architecture.php#distributed): certain algorithms are OpenCL-enabled to take advantage of speed-ups offered by executing in concert across heterogeneous platforms consisting of CPUs, GPUs, and other processors.
* [High dynamic-range images](https://imagemagick.org/script/high-dynamic-range.php): accurately represent the wide range of intensity levels found in real scenes ranging from the brightest direct sunlight to the deepest darkest shadows.
* [Histogram equalization](https://imagemagick.org/script/clahe.php) use adaptive histogram equalization to improve contrast in images.
* [Image cache](https://imagemagick.org/script/magick-cache.php): secure methods and tools to cache images, image sequences, video, audio or metadata in a local folder.
* [Image calculator](https://imagemagick.org/script/fx.php): apply a mathematical expression to an image or image channels.
* [Image gradients](https://imagemagick.org/script/gradient.php): create a gradual blend of one color whose shape is horizontal, vertical, circular, or elliptical.
* [Image identification](https://imagemagick.org/script/identify.php): describe the format and attributes of an image.
* [ImageMagick on the iPhone](https://imagemagick.org/script/download.php#iOS): convert, edit, or compose images on your iPhone.
* [Large image support](https://imagemagick.org/script/architecture.php#tera-pixel): read, process, or write mega-, giga-, or tera-pixel image sizes.
* [Montage](https://imagemagick.org/script/montage.php): juxtapose image thumbnails on an image canvas.
* [Morphology of shapes](https://legacy.imagemagick.org/Usage/morphology/): extract features, describe shapes and recognize patterns in images.
* [Motion picture support](https://imagemagick.org/script/motion-picture.php): read and write the common image formats used in digital film work.
* [Noise and color reduction](https://legacy.imagemagick.org/Usage/transform/#vision) Kuwahara Filter, mean-shift.
* [Perceptual hash](http://www.fmwconcepts.com/misc_tests/perceptual_hash_test_results_510/index.html): maps visually identical images to the same or similar hash-- useful in image retrieval, authentication, indexing, or copy detection as well as digital watermarking.
* [Special effects](https://legacy.imagemagick.org/Usage/blur/): blur, sharpen, threshold, or tint an image.
* [Text & comments](https://legacy.imagemagick.org/Usage/text/): insert descriptive or artistic text in an image.
* [Threads of execution support](https://imagemagick.org/script/architecture.php#threads): ImageMagick is thread safe and most internal algorithms are OpenMP-enabled to take advantage of speed-ups offered by multicore processor chips.
* [Transform](https://legacy.imagemagick.org/Usage/resize/): resize, rotate, deskew, crop, flip or trim an image.
* [Transparency](https://legacy.imagemagick.org/Usage/masking/): render portions of an image invisible.
* [Virtual pixel support](https://imagemagick.org/script/architecture.php#virtual-pixels): convenient access to pixels outside the image region.

[Examples of ImageMagick Usage](https://legacy.imagemagick.org/Usage/), shows how to use ImageMagick from the command-line to accomplish any of these tasks and much more. Also, see [Fred's ImageMagick Scripts](http://www.fmwconcepts.com/imagemagick/): a plethora of command-line scripts that perform geometric transforms, blurs, sharpens, edging, noise removal, and color manipulations. With [Magick.NET](https://github.com/dlemstra/Magick.NET), use ImageMagick without having to install ImageMagick on your server or desktop.

#### News

ImageMagick best practices **strongly** encourages you to configure a [security policy](https://imagemagick.org/script/security-policy.php) that suits your local environment.

Now that ImageMagick version 7 is released, we continue to maintain the legacy release of ImageMagick, version 6, at https://legacy.imagemagick.org. Learn how ImageMagick version 7 differs from previous versions with our [porting guide](https://imagemagick.org/script/porting.php).

Want more performance from ImageMagick? Try these options:

* add more memory to your system, see the [pixel cache](https://imagemagick.org/script/architecture.php#cache);
* add more cores to your system, see [threads of execution support](https://imagemagick.org/script/architecture.php#threads);
* reduce lock contention with the [tcmalloc](http://goog-perftools.sourceforge.net/doc/tcmalloc.html) memory allocation library;
* push large images to a solid-state drive, see [large image support](https://imagemagick.org/script/architecture.php#tera-pixel).

If these options are prohibitive, you can reduce the quality of the image results. The default build is Q16 HDRI. If you disable [HDRI](https://imagemagick.org/script/high-dynamic-range.php), you use half the memory and instead of predominately floating point operations, you use the typically more efficient integer operations. The tradeoff is reduced precision and you cannot process out of range pixel values (e.g. negative). If you build the Q8 non-HDRI version of ImageMagick, you again reduce the memory requirements in half-- and once again there is a tradeoff, even less precision and no out of range pixel values. For a Q8 non-HDRI build of ImageMagick, use these configure script options: --with-quantum-depth=8 --disable-hdri.

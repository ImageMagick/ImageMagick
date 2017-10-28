ImageMagick
===========

<p align="center">
<img align="center" src="https://www.imagemagick.org/image/wizard.png" alt="ImageMagick logo" width="265"/>

[ImageMagick®](https://www.imagemagick.org/) is a software suite to create, edit, compose, or convert bitmap images. It can read and write images in a variety of [formats](https://www.imagemagick.org/script/formats.php) (over 200) including PNG, JPEG, JPEG-2000, GIF, TIFF, DPX, EXR, WebP, Postscript, PDF, and SVG.  Use ImageMagick to resize, flip, mirror, rotate, distort, shear and transform images, adjust image colors, apply various special effects, or draw text, lines, polygons, ellipses and Bézier curves.


| Version | *nix | Windows |
| ------------- | ------------- | ------------- |
| 7 | [![Build Status](https://travis-ci.org/ImageMagick/ImageMagick.svg?branch=master)](https://travis-ci.org/ImageMagick/ImageMagick) | [![Build status](https://ci.appveyor.com/api/projects/status/jk7yr5plamnuh9g6/branch/master?svg=true)](https://ci.appveyor.com/project/dlemstra/imagemagick-windows/branch/master) |
| 6 | [![Build Status](https://travis-ci.org/ImageMagick/ImageMagick.svg?branch=ImageMagick-6)](https://travis-ci.org/ImageMagick/ImageMagick) | [![Build status](https://ci.appveyor.com/api/projects/status/jk7yr5plamnuh9g6/branch/ImageMagick-Windows-6?svg=true)](https://ci.appveyor.com/project/dlemstra/imagemagick-windows/branch/ImageMagick-Windows-6) |

#### What is ImageMagick?

The functionality of ImageMagick is typically utilized from the command line or you can use the features from programs written in your favorite programming language. Choose from these interfaces: G2F (Ada), MagickCore (C), MagickWand (C), ChMagick (Ch), ImageMagickObject (COM+), Magick++ (C++), JMagick (Java), L-Magick (Lisp), NMagick (Neko/haXe), MagickNet (.NET), PascalMagick (Pascal), PerlMagick (Perl), MagickWand for PHP (PHP), IMagick (PHP), PythonMagick (Python), RMagick (Ruby), or TclMagick (Tcl/TK). With a language interface, use ImageMagick to modify or create images dynamically and automagically.

ImageMagick utilizes multiple computational threads to increase performance and can read, process, or write mega-, giga-, or tera-pixel image sizes.

ImageMagick is free software delivered as a ready-to-run binary distribution or as source code that you may use, copy, modify, and distribute in both open and proprietary applications. It is distributed under the Apache 2.0 [license](https://www.imagemagick.org/script/license.php).

The ImageMagick development process ensures a stable API and ABI. Before each ImageMagick release, we perform a comprehensive security assessment that includes memory error and thread data race detection to prevent security vulnerabilities.

ImageMagick is available from https://www.imagemagick.org/script/download.php. It runs on Linux, Windows, Mac Os X, iOS, Android OS, and others.

The authoritative ImageMagick web site is https://www.imagemagick.org. The authoritative source code repository is https://github.com/ImageMagick. We maintain a source code mirror at https://gitlab.com/ImageMagick.

#### Features and Capabilities

Here are just a few examples of what ImageMagick can do:

* [Format conversion](https://www.imagemagick.org/script/convert.php): convert an image from one [format](https://www.imagemagick.org/script/formats.php) to another (e.g.  PNG to JPEG).
* [Transform](https://www.imagemagick.org/Usage/resize/): resize, rotate, deskew, crop, flip or trim an image.
* [Transparency](https://www.imagemagick.org/Usage/masking/): render portions of an image invisible.
* [Draw](https://www.imagemagick.org/Usage/draw/): add shapes or text to an image.
* [Decorate](https://www.imagemagick.org/Usage/crop/): add a border or frame to an image.
* [Special effects](https://www.imagemagick.org/Usage/blur/): blur, sharpen, threshold, or tint an image.
* [Animation](https://www.imagemagick.org/Usage/anim_basics/): create a GIF animation sequence from a group of images.
* [Text & comments](https://www.imagemagick.org/Usage/text/): insert descriptive or artistic text in an image.
* [Image gradients](https://www.imagemagick.org/script/gradient.php): create a gradual blend of one color whose shape is horizontal, vertical, circular, or ellipical.
* [Image identification](https://www.imagemagick.org/script/identify.php): describe the format and attributes of an image.
* [Composite](https://www.imagemagick.org/script/composite.php): overlap one image over another.
* [Montage](https://www.imagemagick.org/script/montage.php): juxtapose image thumbnails on an image canvas.
* [Generalized pixel distortion](https://www.imagemagick.org/Usage/distorts/): correct for, or induce image distortions including perspective.
* [Morphology of shapes](https://www.imagemagick.org/Usage/morphology/): extract features, describe shapes and recognize patterns in images.
* [Delineate image features](https://www.imagemagick.org/Usage/transform/#vision): Canny edge detection, mean-shift, Hough lines.
* [Motion picture support](https://www.imagemagick.org/script/motion-picture.php): read and write the common image formats used in digital film work.
* [Image calculator](https://www.imagemagick.org/script/fx.php): apply a mathematical expression to an image or image channels.
* [Connected component labeling](https://www.imagemagick.org/script/connected-components.php): uniquely label connected regions in an image.
* [Discrete Fourier transform](https://www.imagemagick.org/Usage/fourier/): implements the forward and inverse [DFT](http://en.wikipedia.org/wiki/Discrete_Fourier_transform).
* [Perceptual hash](http://www.fmwconcepts.com/misc_tests/perceptual_hash_test_results_510/index.html): maps visually identical images to the same or similar hash-- useful in image retrieval, authentication, indexing, or copy detection as well as digital watermarking.
* [Complex text layout](https://en.wikipedia.org/wiki/Complex_text_layout) bidirectional text support and shaping.
* [Color management](https://www.imagemagick.org/script/color-management.php): accurate color management with color profiles or in lieu of-- built-in gamma compression or expansion as demanded by the colorspace.
* [High dynamic-range images](https://www.imagemagick.org/script/high-dynamic-range.php): accurately represent the wide range of intensity levels found in real scenes ranging from the brightest direct sunlight to the deepest darkest shadows.
* [Encipher or decipher an image](https://www.imagemagick.org/script/cipher.php): convert ordinary images into unintelligible gibberish and back again.
* [Virtual pixel support](https://www.imagemagick.org/script/architecture.php#virtual-pixels): convenient access to pixels outside the image region.
* [Large image support](https://www.imagemagick.org/script/architecture.php#tera-pixel): read, process, or write mega-, giga-, or tera-pixel image sizes.
* [Threads of execution support](https://www.imagemagick.org/script/architecture.php#threads): ImageMagick is thread safe and most internal algorithms are OpenMP-enabled to take advantage of speed-ups offered by multicore processor chips.
* [Distributed pixel cache](https://www.imagemagick.org/script/distribute-pixel-cache.php): offload intermediate pixel storage to one or more remote servers.
* [Heterogeneous distributed processing](https://www.imagemagick.org/script/architecture.php#distributed): certain algorithms are OpenCL-enabled to take advantage of speed-ups offered by executing in concert across heterogeneous platforms consisting of CPUs, GPUs, and other processors.
* [ImageMagick on the iPhone](https://www.imagemagick.org/script/download.php#iOS): convert, edit, or compose images on your iPhone.

[Examples of ImageMagick Usage](https://www.imagemagick.org/Usage/), shows how to use ImageMagick from the command-line to accomplish any of these tasks and much more. Also, see [Fred's ImageMagick Scripts](http://www.fmwconcepts.com/imagemagick/): a plethora of command-line scripts that perform geometric transforms, blurs, sharpens, edging, noise removal, and color manipulations. With [Magick.NET](https://magick.codeplex.com/), use ImageMagick without having to install ImageMagick on your server or desktop.

#### News

Now that ImageMagick [version 7](https://www.imagemagick.org) is released, we continue to maintain the legacy release of ImageMagick, [version 6](https://legacy.imagemagick.org). Learn how ImageMagick version 7 differs from previous versions with our [porting guide](https://www.imagemagick.org/script/porting.php).

ImageMagick best practices **strongly** encourages you to configure a [security policy](https://www.imagemagick.org/script/security-policy.php) that suits your local environment.

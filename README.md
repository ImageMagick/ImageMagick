# ImageMagick

[![Build Status](https://github.com/ImageMagick/ImageMagick/workflows/main/badge.svg)](https://github.com/ImageMagick/ImageMagick/actions)
[![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/imagemagick.svg)](https://issues.oss-fuzz.com/issues?q=%22project%20ImageMagick%22)
[![Donate](https://img.shields.io/badge/%24-donate-ff00ff.svg)](https://github.com/sponsors/ImageMagick)

<p align="center">
<img align="center" src="https://imagemagick.org/image/wizard.png" alt="ImageMagick logo" width="265" height="353"/>
</p>

[ImageMagick®](https://imagemagick.org/) is a free and [open-source](https://imagemagick.org/script/license.php) software suite, used for editing and manipulating digital images. It can be used to create, edit, compose, or convert bitmap images, and supports a wide range of file [formats](https://imagemagick.org/script/formats.php), including JPEG, PNG, GIF, TIFF, and PDF.

## What is ImageMagick?

ImageMagick is widely used in industries such as web development, graphic design, and video editing, as well as in scientific research, medical imaging, and astronomy. Its versatile and customizable nature, along with its robust image processing capabilities, make it a popular choice for a wide range of image-related tasks.

ImageMagick includes a command-line interface for executing complex image processing tasks, as well as APIs for integrating its features into software applications. It is written in C and can be used on a variety of operating systems, including Linux, Windows, and macOS.

The main website for ImageMagick can be found at [https://imagemagick.org](https://imagemagick.org/). The source code for this software can be accessed through a [repository](https://github.com/ImageMagick/ImageMagick). In addition, we maintain a legacy version of ImageMagick [version 6](https://legacy.imagemagick.org/).

Creating a security policy that fits your specific local environment before making use of ImageMagick is highly advised. You can find guidance on setting up this [policy](https://imagemagick.org/script/security-policy.php). Also, it's important to verify your policy using the [validation tool](https://imagemagick.org/script/security-policy.php).

## Features and Capabilities

One of the key features of ImageMagick is its support for scripting and automation. This allows users to create complex image manipulation pipelines that can be run automatically, without the need for manual intervention. This can be especially useful for tasks that require the processing of large numbers of images, or for tasks that need to be performed on a regular basis.

In addition to its core image manipulation capabilities, ImageMagick also includes a number of other features, such as support for animation, color management, and image rendering. These features make it a versatile tool for a wide range of image-related tasks, including graphic design, scientific visualization, and digital art.

Overall, ImageMagick is a powerful and versatile software suite for displaying, converting, and editing image files. Its support for scripting and automation, along with its other features, make it a valuable tool for a wide range of image-related tasks.

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
* [Decorate](https://imagemagick.org/Usage/crop/): add a border or frame to an image.
* [Delineate image features](https://imagemagick.org/Usage/transform/#vision): Canny edge detection, mean-shift, Hough lines.
* [Discrete Fourier transform](https://imagemagick.org/Usage/fourier/): implements the forward and inverse [DFT](http://en.wikipedia.org/wiki/Discrete_Fourier_transform).
* [Distributed pixel cache](https://imagemagick.org/script/distribute-pixel-cache.php): offload intermediate pixel storage to one or more remote servers.
* [Draw](https://imagemagick.org/Usage/draw/): add shapes or text to an image.
* [Encipher or decipher an image](https://imagemagick.org/script/cipher.php): convert ordinary images into unintelligible gibberish and back again.
* [Format conversion](https://imagemagick.org/script/convert.php): convert an image from one [format](https://imagemagick.org/script/formats.php) to another (e.g.  PNG to JPEG).
* [Generalized pixel distortion](https://imagemagick.org/Usage/distorts/): correct for, or induce image distortions including perspective.
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
* [Morphology of shapes](https://imagemagick.org/Usage/morphology/): extract features, describe shapes and recognize patterns in images.
* [Motion picture support](https://imagemagick.org/script/motion-picture.php): read and write the common image formats used in digital film work.
* [Multispectral imagery](https://imagemagick.org/script/multispectral-imagery.php): support multispectral imagery up to 64 bands.
* [Noise and color reduction](https://imagemagick.org/Usage/transform/#vision) Kuwahara Filter, mean-shift.
* [Perceptual hash](http://www.fmwconcepts.com/misc_tests/perceptual_hash_test_results_510/index.html): maps visually identical images to the same or similar hash-- useful in image retrieval, authentication, indexing, or copy detection as well as digital watermarking.
* [Special effects](https://imagemagick.org/Usage/blur/): blur, sharpen, threshold, or tint an image.
* [Text & comments](https://imagemagick.org/Usage/text/): insert descriptive or artistic text in an image.
* [Threads of execution support](https://imagemagick.org/script/architecture.php#threads): ImageMagick is thread safe and most internal algorithms are OpenMP-enabled to take advantage of speed-ups offered by multicore processor chips.
* [Transform](https://imagemagick.org/Usage/resize/): resize, rotate, deskew, crop, flip or trim an image.
* [Transparency](https://imagemagick.org/Usage/masking/): render portions of an image invisible.
* [Virtual pixel support](https://imagemagick.org/script/architecture.php#virtual-pixels): convenient access to pixels outside the image region.

[Examples of ImageMagick Usage](https://imagemagick.org/Usage/), demonstrates how to use the software from the [command line](https://imagemagick.org/script/command-line-processing.php) to achieve various effects. There are also several scripts available on the website called [Fred's ImageMagick Scripts](http://www.fmwconcepts.com/imagemagick/), which can be used to apply geometric transforms, blur and sharpen images, remove noise, and perform other operations. Additionally, there is a tool called [Magick.NET](https://github.com/dlemstra/Magick.NET) that allows users to access the functionality of ImageMagick without having to install the software on their own systems. Finally, the website also includes a [Cookbook](http://im.snibgo.com/) with tips and examples for using ImageMagick on Windows systems.

## News

Creating a security policy that fits your specific local environment before making use of ImageMagick is highly advised. You can find guidance on setting up this [policy](https://imagemagick.org/script/security-policy.php). Also, it's important to verify your policy using the [validation tool](https://imagemagick-secevaluator.doyensec.com/). As of ImageMagick version 7.1.1-16, you can choose and customize one of these [security policies](https://imagemagick.org/script/security-policy.php): Open, Limited, Secure, and Websafe.

By default, ImageMagick supports up to 32 channels. As of ImageMagick version 7.1.1-16, you can enable up to 64 channels by adding the **--enable-64bit-channel-masks** option to the Linux configure build script. For Windows this will be enabled automatically.


Want more performance from ImageMagick? Try these options:

* add more memory to your system, see the [pixel cache](https://imagemagick.org/script/architecture.php#cache);
* add more cores to your system, see [threads of execution support](https://imagemagick.org/script/architecture.php#threads);
* reduce lock contention with the [tcmalloc](http://goog-perftools.sourceforge.net/doc/tcmalloc.html) memory allocation library;
* push large images to a solid-state drive, see [large image support](https://imagemagick.org/script/architecture.php#tera-pixel).

If these options are prohibitive, you can reduce the quality of the image results. The default build is Q16 HDRI. If you disable [HDRI](https://imagemagick.org/script/high-dynamic-range.php), you use half the memory and instead of predominantly floating point operations, you use the typically more efficient integer operations. The tradeoff is reduced precision and you cannot process out of range pixel values (e.g. negative). If you build the Q8 non-HDRI version of ImageMagick, you again reduce the memory requirements in half-- and once again there is a tradeoff, even less precision and no out of range pixel values. For a Q8 non-HDRI build of ImageMagick, use these configure script options: **--with-quantum-depth=8 --disable-hdri**.

PerlMagick
==========

INTRODUCTION
------------

PerlMagick is an object-oriented Perl interface to ImageMagick. Use it to
read, manipulate, or write an image or image sequence from within a Perl
script. This makes it well suited to web CGI scripts as well as batch and
command-line tools.

Requirements:

    * ImageMagick 7.0.0 or above, installed on your system.
    * A working Perl installation with a C compiler and the standard build
      tools (make, ExtUtils::MakeMaker) if you are building from source.
    * Development files (headers and libraries) for ImageMagick.

Resources:

    * PerlMagick documentation:   https://imagemagick.org/perl-magick/
    * Help and discussion:        https://github.com/ImageMagick/ImageMagick/discussions
    * ImageMagick installation:   https://imagemagick.org/


INSTALLATION (UNIX, LINUX, macOS)
---------------------------------

1. Unpack the ImageMagick distribution (adjust the file name to match the
   version you downloaded):

       gunzip ImageMagick-7.1.2-0.tar.gz
       tar xvf ImageMagick-7.1.2-0.tar

2. Build and install ImageMagick itself by following the instructions in
   INSTALL-unix.txt.

3. Change to the PerlMagick directory:

       cd PerlMagick

4. Edit Makefile.PL and set LIBS and INC so they point to the libMagick
   library and headers. You will also need library search paths (-L) for the
   JPEG, PNG, TIFF, and other libraries if they were included with your
   installed version of ImageMagick.

   If an extension library is built as a shared library but is not installed
   in the system's default library search path, you may also need to add
   run-path information (often -R or -rpath) that matches the library search
   path, so the library can be located at run time.

5. Build and install the dynamically loaded version of PerlMagick (the
   preferred method):

       perl Makefile.PL
       make
       make install

   On Unix you typically need root privileges for "make install". There are
   ways around this, such as installing into a local directory (for example,
   perl Makefile.PL PREFIX=$HOME/perl). Consult the Perl manual pages for
   details.

You are now ready to use PerlMagick in your Perl scripts.


STATICALLY LINKED BUILDS (OPTIONAL)
-----------------------------------

Most users should use the dynamically loaded build above. If you need
PerlMagick linked statically into a Perl executable, you have two options.

Replace your existing "perl" executable (use with caution: this overwrites
your current Perl interpreter):

    perl Makefile.PL
    make perl
    make -f Makefile.aperl inst_perl

Or create a new interpreter with a different name (for example, "PerlMagick"),
leaving your existing Perl untouched:

    perl Makefile.PL MAP_TARGET=PerlMagick
    make PerlMagick
    make -f Makefile.aperl inst_perl

In both cases PerlMagick is linked statically, while other libraries are
linked statically or dynamically according to your system linker's defaults.
See the ExtUtils::MakeMaker(3) manual page for more information on building
Perl extensions such as PerlMagick.


INSTALLATION (WINDOWS)
----------------------

With a standard Perl and Visual C++ toolchain:

    perl Makefile.nt
    nmake install

With Strawberry Perl (recommended on Windows):

1. Download and install the ImageMagick Windows binaries from
   https://imagemagick.org/download/#windows

2. Choose the dynamic (DLL) ImageMagick binaries. Note that you cannot mix
   32-bit and 64-bit builds: your Perl and ImageMagick must match.

3. During installation, select the option to install ImageMagick's
   development files (libraries and headers).

4. Make sure ImageMagick's directory is in your PATH. The "magick" tool is
   used to determine your configuration.

5. You may need the Visual C++ Redistributable Package. See the instructions
   on ImageMagick's binary release web page.

6. Once steps 1 through 5 are complete, install PerlMagick by running:

       cpan -i Image::Magick


TESTING
-------

Before installing PerlMagick, you may want to run

    make test

to verify that Perl can load the PerlMagick extension. Some tests may fail if
you do not have the proper delegate libraries installed for formats such as
JPEG, TIFF, and PNG. Such failures usually indicate missing format support,
not a broken build.

To run a single test from the build folder, use the "prove" utility:

    prove --blib blib -I `pwd` -bv ./t/read.t

After installation, you can confirm that the module loads and check its
version with:

    perl -MImage::Magick -e 'print "$Image::Magick::VERSION\n"'

To see a number of PerlMagick demonstration scripts, run:

    cd demo
    make


EXAMPLE SCRIPT
--------------

The following script reads three images, crops each of them, and writes the
result as a single GIF animation sequence:

    #!/usr/bin/perl
    use strict;
    use warnings;
    use Image::Magick;

    my $image = Image::Magick->new;

    # Read three images into one image sequence.
    my $status = $image->Read("model.gif", "logo.gif", "rose.gif");
    warn "$status" if $status;

    # Crop every image in the sequence to 100x100 pixels, starting at
    # an offset of (100,100).
    $status = $image->Crop(geometry => '100x100+100+100');
    warn "$status" if $status;

    # Write the sequence as a single animated GIF.
    $status = $image->Write("x.gif");
    warn "$status" if $status;

Notes:

    * PerlMagick methods return an empty value on success and an error
      message on failure, which is why each call is checked and passed to
      "warn".
    * Methods operate on every image in the sequence, so the Crop call above
      affects all three images.
    * Writing a multi-image sequence to a format that supports animation (such
      as GIF) produces one animated file. For formats that do not, each image
      is typically written to its own file.


TROUBLESHOOTING
---------------

    * "Can't locate Image/Magick.pm": PerlMagick is not installed, or it was
      installed to a directory that is not in Perl's @INC path.
    * Build errors about missing headers or libraries: check the LIBS and INC
      settings in Makefile.PL, and confirm that the ImageMagick development
      files are installed.
    * "error while loading shared libraries": the ImageMagick (or delegate)
      shared libraries are not in the system's library search path. Add
      their location to the run-path settings described in Installation,
      or to your system's library path configuration.
    * Failures reading or writing certain formats: the required delegate
      library (JPEG, PNG, TIFF, and so on) was not available when
      ImageMagick was built.

If problems persist, ask for help at
https://github.com/ImageMagick/ImageMagick/discussions

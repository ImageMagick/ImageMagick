Introduction 

    PerlMagick, is an objected-oriented Perl interface to ImageMagick.
    Use the module to read, manipulate, or write an image or image sequence
    from within a Perl script. This makes it suitable for Web CGI scripts. You
    must have ImageMagick 6.7.4 or above installed on your system for this
    module to work properly.

    See

        http://www.imagemagick.org/script/perl-magick.php

    for additional information about PerlMagick.  If you have problems, go to

        http://www.imagemagick.org/discourse-server/viewforum.php?f=7

    for help.  For instructions about installing ImageMagick, see

        http://www.imagemagick.org/


Installation 

    Get the PerlMagick distribution and type the following: 

        gunzip ImageMagick-6.7.4-0.tar.gz
        tar xvf ImageMagick-6.7.4

    Follow the ImageMagick installation instructions in INSTALL-unix.txt
		then type

      cd PerlMagick

    Next, edit Makefile.PL and change LIBS and INC to include the appropriate
    path information to the required libMagick library. You will also need
    library search paths (-L) to JPEG, PNG, TIFF, etc. libraries if they were
    included with your installed version of ImageMagick. If an extension
    library is built as a shared library but not installed in the system's
    default library search path, you may need to add run-path information
    (often -R or -rpath) corresponding to the equivalent library search
    path option so that the library can be located at run-time.

    To create and install the dymamically-loaded version of PerlMagick
    (the preferred way), execute
        
        perl Makefile.PL
        make
        make install

    To create and install a new 'perl' executable (replacing your existing
    PERL interpreter!) with PerlMagick statically linked (but other libraries
    linked statically or dynamically according to system linker default),
    execute

        perl Makefile.PL
        make perl
        make -f Makefile.aperl inst_perl
	
    or to create and install a new PERL interpreter with a different name
    than 'perl' (e.g. 'PerlMagick') and with PerlMagick statically linked

        perl Makefile.PL MAP_TARGET=PerlMagick
        make PerlMagick
        make -f Makefile.aperl inst_perl

    See the ExtUtils::MakeMaker(3) manual page for more information on
    building PERL extensions (like PerlMagick).

    For Windows systems, type

        perl Makefile.nt
        nmake install

    For Unix, you typically need to be root to install the software.
    There are ways around this.  Consult the Perl manual pages for more
    information. You are now ready to utilize the PerlMagick routines from
    within your Perl scripts.

Installation - Win32 Strawberry perl

   On Win32 Strawberry perl the prefered way of installing PerlMagick is the
   following:

   1) Download and install ImageMagick Windows binaries from
      http://www.imagemagick.org/script/binary-releases.php#windows

   2) You HAVE TO choose dynamic (DLL) ImageMagick binaries.  Note: it is not
      possible to mix 32/64bit binaries of perl and ImageMagick

   3) During installation select that you want to install ImageMagick's
      development files (libraries+headers)

   4) You NEED TO have ImageMagick's directory in your PATH.  Note: we are
      checking the presence of convert.exe or identify.exe tools

   5) You might need Visual C++ Redistributable Package installed on your
      system.  See instructions on ImageMagick's Binary Release webpage.

   6) If you have all prerequisites 1)...5) you can simply install
      ImageMagick by running: cpan -i Image::Magick


Testing PerlMagick

    Before PerlMagick is installed, you may want to execute
    
        make test

    to verify that PERL can load the PerlMagick extension ok.  Chances are
    some of the tests will fail if you do not have the proper delegates
    installed for formats like JPEG, TIFF, etc.

    To see a number of PerlMagick demonstration scripts, type
    
        cd demo
        make


Example Perl Magick Script 

    Here is an example script to get you started: 

        #!/usr/bin/perl
        use Image::Magick;

        $q = Image::Magick->new;
        $x = $q->Read("model.gif", "logo.gif", "rose.gif");
        warn "$x" if $x;

        $x = $q->Crop(geom=>'100x100+100+100');
        warn "$x" if $x;

        $x = $q->Write("x.gif");
        warn "$x" if $x;

    The script reads three images, crops them, and writes a single image
    as a GIF animation sequence.

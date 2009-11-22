#  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    http://www.imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Exercise all regression tests:
#  
#    make test
#
#  Exersise one regression test:
#
#    make TEST_VERBOSE=1 TEST_FILES=t/filter.t test
#

use ExtUtils::MakeMaker;
use Config;
use File::Spec::Functions qw/catfile catdir devnull catpath splitpath/;
use Cwd;

sub AutodetectWin32gcc {
  my $wrkdir = getcwd();
  my $devnull = devnull();
 
  my @incdir = ();
  my @libdir = ($wrkdir);
  my @bindir = ();
 
  #try to get configuration info via identify or convert utilities
  my $conf = `identify -list Configure 2>$devnull` || `convert -list Configure 2>$devnull`;
  foreach my $line (split '\n', $conf) {
    if ($line =~ /^Path:\s+(.*)/) {
      my ($vol,$dir,$file) = splitpath($1);
      next unless $dir;
      my $dirpath = catpath( $vol, $dir);
      my (@l,@b,@i) = ( (),(),() );

      # try to detect 'lib' dir
      push @l, catfile($dirpath,'..','lib');
      push @l, catfile($dirpath,'..','..','lib');
      push @l, catfile($dirpath,'..','..','..','lib');
      foreach (@l) { push @libdir, $_ if (-d $_) };

      # try to detect 'bin' dir
      push @b, catfile($dirpath,'..');
      push @b, catfile($dirpath,'..','bin');
      push @b, catfile($dirpath,'..','..');
      push @b, catfile($dirpath,'..','..','bin');
      push @b, catfile($dirpath,'..','..','..');
      push @b, catfile($dirpath,'..','..','..','bin');
      foreach (@b) { push @bindir, $_ if (-e "$_/convert.exe" || -e "$_/identify.exe") };

      # try to detect 'include' dir
      push @i, catfile($dirpath,'..','include');
      push @i, catfile($dirpath,'..','include','ImageMagick');
      push @i, catfile($dirpath,'..','..','include');
      push @i, catfile($dirpath,'..','..','include','ImageMagick');
      push @i, catfile($dirpath,'..','..','..','include');
      push @i, catfile($dirpath,'..','..','..','include','ImageMagick');
      foreach (@i) { push @incdir, $_ if (-e "$_/magick/MagickCore.h") };
    }
  };
 
  foreach my $bin (@bindir) {
    opendir(my $bindir, $bin) or die qq{Cannot opendir $bin: $!};
    my @dlls = map {catfile($bin, $_)} grep /^\S*magick[^\+]\S*?\.dll$/i, readdir $bindir;
    foreach my $d (@dlls) {
      unlink "$wrkdir/libMagickCore.def", "$wrkdir/libMagickCore.a";
      system("pexports \"$d\" >\"$wrkdir/libMagickCore.def\" 2>$devnull");
      open(DEF, "<$wrkdir/libMagickCore.def");
      my @found = grep(/InitializeMagickResources/, <DEF>); #checking if we have taken the right DLL
      close(DEF);
      next unless(@found);
      print STDERR "Gonna create 'libMagickCore.a' from '$d'\n";
      system("dlltool -D \"$d\" -d \"$wrkdir/libMagickCore.def\" -l \"$wrkdir/libMagickCore.a\" 2>$devnull");
      last if -s "$wrkdir/libMagickCore.a";
    }
    last if -s "$wrkdir/libMagickCore.a";
  }
 
  unless(@incdir && @libdir && @bindir && (-s "$wrkdir/libMagickCore.a")) {
    print STDERR <<EOF
################################### WARNING! ###################################
# It seems that you are trying to install Perl::Magick on a MS Windows box with
# perl + gcc compiler (e.g. strawberry perl), however we cannot find ImageMagick
# binaries installed on your system.
#
# Please check the following prerequisites:
#
# 1) You need to have installed ImageMagick Windows binaries from
#    http://www.imagemagick.org/script/binary-releases.php#windows
#
# 2) We only support dynamic (DLL) ImageMagick binaries
#    note: it is not possible to mix 32/64-bit binaries of perl and ImageMagick
#
# 3) During installation select that you want to install ImageMagick's
#    development files (libraries+headers)
#
# 4) You also need to have ImageMagick's directory in your PATH
#    note: we are checking the presence of convert.exe and/or identify.exe tools
#
# 5) You might need Visual C++ Redistributable Package installed on your system
#    see instructions on ImageMagick's Binary Release webpage
#
# We are gonna continue, but chances for successful build are very low!
################################################################################
EOF
  }
   
  my $inc = join ' ', map "-I\"$_\"", @incdir;
  my $lib = join ' ', map "-L\"$_\"", @libdir;

  return ($inc, $lib);
}


# Compute test specification
my $delegate_tests='t/*.t';
my $delegate;
foreach $delegate (qw/bzlib djvu fftw fontconfig freetype jpeg jng jp2 lcms mpeg png rsvg tiff x11 xml wmf zlib/) {
  if ( -d "t/$delegate" ) {
    if ( defined($ENV{'DISPLAY'}) && ($^O ne 'MSWin32') ) {
      if ( defined $ENV{'DISPLAY'} ) {
        $delegate_tests .= " t/$delegate/*.t";
      }
      next;
    }
    $delegate_tests .= " t/$delegate/*.t";
  }
}

# defaults for LIBS & INC & CCFLAGS params that we later pass to Writemakefile
my $INC_magick = '-I../ -I.. -I/usr/include/freetype2 -I/usr/include/libxml2 -I"' . $Config{'usrinc'} . '/ImageMagick"';
my $LIBS_magick = '-L../magick/.libs -lMagickCore -lperl -lm';
my $CCFLAGS_magick = "$Config{'ccflags'} -fopenmp -g -O2 -Wall -W -pthread";
my $LDFLAGS_magick   = "-L../magick/.libs -lMagickCore $Config{'ldflags'} ";
my $LDDLFLAGS_magick = "-L../magick/.libs -lMagickCore $Config{'lddlflags'} ";

if (($^O eq 'MSWin32') && ($Config{cc} =~ /gcc/)) {
  my($Ipaths, $Lpaths) = AutodetectWin32gcc();

  #
  # Setup for strawberry perl.
  #
  $INC_magick       = "$Ipaths";
  $LIBS_magick      = "-lMagickCore";
  $CCFLAGS_magick   = "$Config{'ccflags'}";
  $LDFLAGS_magick   = "$Config{'ldflags'} $Lpaths ";
  $LDDLFLAGS_magick = "$Config{'lddlflags'} $Lpaths ";
}

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile
  (
   # Module description
   'ABSTRACT'	=> 'ImageMagick PERL Extension',

   # Perl module name is Image::Magick
   'NAME'	=> 'Image::Magick',

   # Module author
   'AUTHOR' => 'ImageMagick Studio LLC',

   # Module version
   'VERSION' => '6.5.8',

   # Preprocessor defines
   'DEFINE'	=> ' -D_LARGE_FILES=1 -DHAVE_CONFIG_H',     # e.g., '-DHAVE_SOMETHING' 

   # Header search specfication and preprocessor flags
   'INC'	=> $INC_magick,

   # C compiler
   #'CC' => 'gcc -std=gnu99',

   # C pre-processor flags (e.g. -I & -D options)
   # 'CPPFLAGS' => "$Config{'cppflags'} -I/usr/include/freetype2 -I/usr/include/libxml2",

   # C compiler flags (e.g. -O -g)
   'CCFLAGS' => $CCFLAGS_magick,

   # Linker
   #'LD' => $Config{'ld'} == $Config{'cc'} ? 'gcc -std=gnu99' : $Config{'ld'},

   # Linker flags for building an executable
   'LDFLAGS' =>  $LDFLAGS_magick,

   # Linker flags for building a dynamically loadable module
   'LDDLFLAGS' => $LDDLFLAGS_magick,

   # Install PerlMagick binary into ImageMagick bin directory
   'INSTALLBIN'	=> '/usr/local/bin',

   # Library specification
   'LIBS' => [ $LIBS_magick ],

   # Perl binary name (if a Perl binary is built)
   'MAP_TARGET'	=> 'PerlMagick',

   # Let CFLAGS drive optimization flags by setting OPTIMIZE to empty
   # 'OPTIMIZE'	=> '',

   # Use same compiler as ImageMagick
   'PERLMAINCC'	=> ' -fopenmp',

   # Set Perl installation prefix to ImageMagick installation prefix
#   'PREFIX'	=> '/usr/local',

   # Include delegate directories in tests
   test	=> { TESTS	=>	$delegate_tests},

   ($Config{'archname'} =~ /-object$/i ? ('CAPI' => 'TRUE') : ()),
);


#
# Substitutions for "makeaperl" section.
#
sub MY::makeaperl {
     package MY; # so that "SUPER" works right
     my $inherited = shift->SUPER::makeaperl(@_);

     # Stinky ExtUtils::MM_Unix likes to append its own library path to $(CC),
     # prior to any user-specified library path so that an installed library is
     # used rather than the library just built.  This substitution function
     # tries to insert our library path first. Also, use the same compiler used
     # to build perlmain.c to link so that a C++ compiler may be used if
     # necessary.
     $inherited =~ s:MAP_LINKCMD\s.*\s*\$\(CC\):MAP_LINKCMD   = \$(PERLMAINCC) -L/usr/local/lib: ;
     $inherited;
 }

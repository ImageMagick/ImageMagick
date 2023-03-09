package Image::Magick::Q16HDRI;

#  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    https://imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Initial version, written by Kyle Shorter.

use strict;
use Carp;
use vars qw($VERSION @ISA @EXPORT $AUTOLOAD);

require 5.002;
require Exporter;
require DynaLoader;
require AutoLoader;

@ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT =
  qw(
      Success Transparent Opaque QuantumDepth QuantumRange MaxRGB
      WarningException ResourceLimitWarning TypeWarning OptionWarning
      DelegateWarning MissingDelegateWarning CorruptImageWarning
      FileOpenWarning BlobWarning StreamWarning CacheWarning CoderWarning
      ModuleWarning DrawWarning ImageWarning XServerWarning RegistryWarning
      ConfigureWarning ErrorException ResourceLimitError TypeError
      OptionError DelegateError MissingDelegateError CorruptImageError
      FileOpenError BlobError StreamError CacheError CoderError
      ModuleError DrawError ImageError XServerError RegistryError
      ConfigureError FatalErrorException
    );

$VERSION = '7.1.1';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.
    no warnings;

    my $constname;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    die "&${AUTOLOAD} not defined. The required ImageMagick libraries are not installed or not installed properly.\n" if $constname eq 'constant';
    my $val = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
    	if ($! =~ /Invalid/) {
	        $AutoLoader::AUTOLOAD = $AUTOLOAD;
	        goto &AutoLoader::AUTOLOAD;
    	}
    	else {
	        my($pack,$file,$line) = caller;
	        die "Your vendor has not defined PerlMagick macro $pack\:\:$constname, used at $file line $line.\n";
    	}
    }
    eval "sub $AUTOLOAD { $val }";
    goto &$AUTOLOAD;
}

bootstrap Image::Magick::Q16HDRI $VERSION;

# Preloaded methods go here.

sub new
{
    my $this = shift;
    my $class = ref($this) || $this || "Image::Magick::Q16HDRI";
    my $self = [ ];
    bless $self, $class;
    $self->set(@_) if @_;
    return $self;
}

sub New
{
    my $this = shift;
    my $class = ref($this) || $this || "Image::Magick::Q16HDRI";
    my $self = [ ];
    bless $self, $class;
    $self->set(@_) if @_;
    return $self;
}

# Autoload methods go after =cut, and are processed by the autosplit program.

END { UNLOAD () };

1;
__END__

=head1 NAME

Image::Magick::Q16HDRI - objected-oriented Perl interface to ImageMagick (Q16HDRI). Use it to create, edit, compose, or convert bitmap images from within a Perl script.

=head1 SYNOPSIS

  use Image::Magick::Q16HDRI;
  $p = new Image::Magick::Q16HDRI;
  $p->Read("imagefile");
  $p->Set(attribute => value, ...)
  ($a, ...) = $p->Get("attribute", ...)
  $p->routine(parameter => value, ...)
  $p->Mogrify("Routine", parameter => value, ...)
  $p->Write("filename");

=head1 DESCRIPTION

This Perl extension allows the reading, manipulation and writing of
a large number of image file formats using the ImageMagick library.
It was originally developed to be used by CGI scripts for Web pages.

A web page has been set up for this extension. See:

	 https://imagemagick.org/script/perl-magick.php

If you have problems, go to

   https://github.com/ImageMagick/ImageMagick/discussions/categories/development

=head1 AUTHOR

Kyle Shorter	magick-users@imagemagick.org

=head1 BUGS

Has all the bugs of ImageMagick and much, much more!

=head1 SEE ALSO

perl(1).

=cut

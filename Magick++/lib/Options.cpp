// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Implementation of Options
//
// A wrapper around DrawInfo, ImageInfo, and QuantizeInfo
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include <string>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "Magick++/Options.h"
#include "Magick++/Functions.h"
#include "Magick++/Exception.h"

#define MagickPI  3.14159265358979323846264338327950288419716939937510
#define DegreesToRadians(x)  (MagickPI*(x)/180.0)

// Constructor
Magick::Options::Options( void )
  : _imageInfo(static_cast<ImageInfo*>(AcquireMagickMemory(sizeof(ImageInfo)))),
    _quantizeInfo(static_cast<QuantizeInfo*>(AcquireMagickMemory(sizeof(QuantizeInfo)))),
    _drawInfo(static_cast<DrawInfo*>(AcquireMagickMemory( sizeof(DrawInfo))))
{
  // Initialize image info with defaults
  GetImageInfo( _imageInfo );
  
  // Initialize quantization info
  GetQuantizeInfo( _quantizeInfo );

  // Initialize drawing info
  GetDrawInfo( _imageInfo, _drawInfo );
}

// Copy constructor
Magick::Options::Options( const Magick::Options& options_ )
  : _imageInfo(CloneImageInfo( options_._imageInfo )),
    _quantizeInfo(CloneQuantizeInfo(options_._quantizeInfo)),
    _drawInfo(CloneDrawInfo(_imageInfo, options_._drawInfo))
{
}

// Construct using raw structures
Magick::Options::Options( const MagickCore::ImageInfo* imageInfo_,
                          const MagickCore::QuantizeInfo* quantizeInfo_,
                          const MagickCore::DrawInfo* drawInfo_ )
: _imageInfo(0),
  _quantizeInfo(0),
  _drawInfo(0)
{
  _imageInfo = CloneImageInfo(imageInfo_);
  _quantizeInfo = CloneQuantizeInfo(quantizeInfo_);
  _drawInfo = CloneDrawInfo(imageInfo_,drawInfo_);
}

// Destructor
Magick::Options::~Options()
{
  // Destroy image info
   _imageInfo =DestroyImageInfo( _imageInfo );
  _imageInfo=0;

  // Destroy quantization info
   _quantizeInfo =DestroyQuantizeInfo( _quantizeInfo );
  _quantizeInfo=0;

  // Destroy drawing info
   _drawInfo =DestroyDrawInfo( _drawInfo );
  _drawInfo=0;
}

/*
 * Methods for setting image attributes
 *
 */

// Anti-alias Postscript and TrueType fonts (default true)
void Magick::Options::antiAlias( bool flag_ )
{
  _drawInfo->text_antialias = static_cast<MagickBooleanType>
    (flag_ ? MagickTrue : MagickFalse);
}
bool Magick::Options::antiAlias( void ) const
{
  return static_cast<bool>(_drawInfo->text_antialias);
}

void Magick::Options::adjoin ( bool flag_ )
{
  _imageInfo->adjoin = static_cast<MagickBooleanType>
    (flag_ ? MagickTrue : MagickFalse);
}
bool Magick::Options::adjoin ( void ) const
{
  return static_cast<bool>(_imageInfo->adjoin);
}

void Magick::Options::backgroundColor ( const Magick::Color &color_ )
{
  _imageInfo->background_color = color_;
}
Magick::Color Magick::Options::backgroundColor ( void ) const
{
  return Magick::Color( _imageInfo->background_color );
}

void Magick::Options::backgroundTexture ( const std::string &backgroundTexture_ )
{
  if ( backgroundTexture_.length() == 0 )
    _imageInfo->texture=(char *) RelinquishMagickMemory(_imageInfo->texture);
  else
    Magick::CloneString( &_imageInfo->texture, backgroundTexture_ );
}
std::string Magick::Options::backgroundTexture ( void ) const
{
  if ( _imageInfo->texture )
    return std::string( _imageInfo->texture );
  else
    return std::string();
}

void Magick::Options::borderColor ( const Color &color_ )
{
  _imageInfo->border_color = color_;
  _drawInfo->border_color = color_;
}
Magick::Color Magick::Options::borderColor ( void ) const
{
  return Magick::Color( _imageInfo->border_color );
}

// Text bounding-box base color
void Magick::Options::boxColor ( const Magick::Color &boxColor_ )
{
  _drawInfo->undercolor = boxColor_;
}
Magick::Color Magick::Options::boxColor ( void ) const
{
  return Magick::Color( _drawInfo->undercolor );
}

void Magick::Options::colorspaceType ( Magick::ColorspaceType colorspace_ )
{
  _imageInfo->colorspace = colorspace_;
}
Magick::ColorspaceType Magick::Options::colorspaceType ( void ) const
{
  return static_cast<Magick::ColorspaceType>(_imageInfo->colorspace);
}

void Magick::Options::compressType ( CompressionType compressType_ )
{
  _imageInfo->compression = compressType_;
}
Magick::CompressionType Magick::Options::compressType ( void ) const
{
  return static_cast<Magick::CompressionType>(_imageInfo->compression);
}

void Magick::Options::colorFuzz ( double fuzz_ )
{
  _imageInfo->fuzz = fuzz_;
}
double Magick::Options::colorFuzz ( void ) const
{
  return _imageInfo->fuzz;
}

// Enable printing of debug messages from ImageMagick
void Magick::Options::debug ( bool flag_ )
{
  if(flag_)
    {
      SetLogEventMask("All");
    }
  else
    {
      SetLogEventMask("None");
    }
}
bool Magick::Options::debug ( void ) const
{
  if( IsEventLogging() )
    {
      return true;
    }
  return false;
}

void Magick::Options::density ( const Magick::Geometry &density_ )
{
  if ( !density_.isValid() )
    _imageInfo->density=(char *) RelinquishMagickMemory(_imageInfo->density);
  else
    Magick::CloneString( &_imageInfo->density, density_ );
}
Magick::Geometry Magick::Options::density ( void ) const
{
  if ( _imageInfo->density )
    return Geometry( _imageInfo->density );

  return Geometry();
}

void Magick::Options::depth ( size_t depth_ )
{
  _imageInfo->depth = depth_;
}
size_t Magick::Options::depth ( void ) const
{
  return _imageInfo->depth;
}

// Endianness (little like Intel or big like SPARC) for image
// formats which support endian-specific options.
void Magick::Options::endian ( Magick::EndianType endian_ )
{
  _imageInfo->endian = endian_;
}
Magick::EndianType Magick::Options::endian ( void ) const
{
  return _imageInfo->endian;
}

void Magick::Options::file ( FILE *file_ )
{
  SetImageInfoFile( _imageInfo, file_ );
}
FILE *Magick::Options::file ( void ) const
{
  return GetImageInfoFile( _imageInfo );
}

void Magick::Options::fileName ( const std::string &fileName_ )
{
  fileName_.copy( _imageInfo->filename, MaxTextExtent-1 );
  _imageInfo->filename[ fileName_.length() ] = 0;
}
std::string Magick::Options::fileName ( void ) const
{
  return std::string( _imageInfo->filename );
}

// Color to use when drawing inside an object
void Magick::Options::fillColor ( const Magick::Color &fillColor_ )
{
  _drawInfo->fill = fillColor_;
  if (fillColor_ == Magick::Color())
    fillPattern((const MagickCore::Image*) NULL);
}
Magick::Color Magick::Options::fillColor ( void ) const
{
  return _drawInfo->fill;
}
// Pattern image to use when filling objects
void Magick::Options::fillPattern ( const MagickCore::Image *fillPattern_ )
{
  if ( _drawInfo->fill_pattern )
    {
      DestroyImageList( _drawInfo->fill_pattern );
      _drawInfo->fill_pattern = 0;
    }
  if ( fillPattern_ )
    {
      ExceptionInfo exceptionInfo;
      GetExceptionInfo( &exceptionInfo );
      _drawInfo->fill_pattern =
	CloneImage( const_cast<MagickCore::Image*>(fillPattern_),
		    0,
		    0,
		    static_cast<MagickBooleanType>(MagickTrue),
		    &exceptionInfo );
      throwException( exceptionInfo );
      (void) DestroyExceptionInfo( &exceptionInfo );
    }
}
const MagickCore::Image* Magick::Options::fillPattern ( void  ) const
{
  return _drawInfo->fill_pattern;
}

// Rule to use when filling drawn objects
void Magick::Options::fillRule ( const Magick::FillRule &fillRule_ )
{
  _drawInfo->fill_rule = fillRule_;
}
Magick::FillRule Magick::Options::fillRule ( void ) const
{
  return _drawInfo->fill_rule;
}

void Magick::Options::font ( const std::string &font_ )
{
  if ( font_.length() == 0 )
    {
      _imageInfo->font=(char *) RelinquishMagickMemory(_imageInfo->font);
      _drawInfo->font=(char *) RelinquishMagickMemory(_drawInfo->font);
    }
  else
    {
      Magick::CloneString( &_imageInfo->font, font_ );
      Magick::CloneString( &_drawInfo->font, font_ );
    }
}
std::string Magick::Options::font ( void ) const
{
  if ( _imageInfo->font )
    return std::string( _imageInfo->font );
  
  return std::string();
}

void Magick::Options::fontPointsize ( double pointSize_ )
{
  _imageInfo->pointsize = pointSize_;
  _drawInfo->pointsize = pointSize_;
}
double Magick::Options::fontPointsize ( void ) const
{
  return _imageInfo->pointsize;
}

std::string Magick::Options::format ( void ) const
{
  ExceptionInfo exception;

  const MagickInfo * magick_info = 0;
  GetExceptionInfo(&exception);
  if ( *_imageInfo->magick != '\0' )
    magick_info = GetMagickInfo( _imageInfo->magick , &exception);
  throwException( exception );
  (void) DestroyExceptionInfo( &exception );
  
  if (( magick_info != 0 ) && 
      ( *magick_info->description != '\0' ))
    return std::string( magick_info->description );
  
  return std::string();
}

void Magick::Options::interlaceType ( Magick::InterlaceType interlace_ )
{
  _imageInfo->interlace = interlace_;
}
Magick::InterlaceType Magick::Options::interlaceType ( void ) const
{
  return static_cast<Magick::InterlaceType>(_imageInfo->interlace);
}

void Magick::Options::magick ( const std::string &magick_ )
{
  ExceptionInfo exception;

  FormatLocaleString( _imageInfo->filename, MaxTextExtent, "%.1024s:", magick_.c_str() );
  GetExceptionInfo(&exception);
  SetImageInfo( _imageInfo, 1, &exception);
  if ( *_imageInfo->magick == '\0' )
    throwExceptionExplicit( OptionWarning, "Unrecognized image format",
	    magick_.c_str() );
  (void) DestroyExceptionInfo( &exception );
}
std::string Magick::Options::magick ( void ) const
{
  if ( _imageInfo->magick && *_imageInfo->magick )
    return std::string( _imageInfo->magick );
  
  return std::string();
}

void Magick::Options::matteColor ( const Magick::Color &matteColor_ )
{
  _imageInfo->matte_color = matteColor_;
}
Magick::Color Magick::Options::matteColor ( void ) const
{
  return Magick::Color( _imageInfo->matte_color );
}

void Magick::Options::monochrome ( bool monochromeFlag_ )
{
  _imageInfo->monochrome = (MagickBooleanType) monochromeFlag_;
}
bool Magick::Options::monochrome ( void ) const
{
  return static_cast<bool>(_imageInfo->monochrome);
}

void Magick::Options::page ( const Magick::Geometry &pageSize_ )
{
  if ( !pageSize_.isValid() )
    _imageInfo->page=(char *) RelinquishMagickMemory(_imageInfo->page);
  else
    Magick::CloneString( &_imageInfo->page, pageSize_ );
}
Magick::Geometry Magick::Options::page ( void ) const
{
  if ( _imageInfo->page )
    return Geometry( _imageInfo->page );

    return Geometry();
}

void Magick::Options::quality ( size_t quality_ )
{
  _imageInfo->quality = quality_;
}
size_t Magick::Options::quality ( void ) const
{
  return _imageInfo->quality;
}

void Magick::Options::quantizeColors ( size_t colors_ )
{
  _quantizeInfo->number_colors = colors_;
}
size_t Magick::Options::quantizeColors ( void ) const
{
  return _quantizeInfo->number_colors;
}

void Magick::Options::quantizeColorSpace ( Magick::ColorspaceType colorSpace_ )
{
  _quantizeInfo->colorspace = colorSpace_;
}
Magick::ColorspaceType Magick::Options::quantizeColorSpace ( void ) const
{
  return static_cast<Magick::ColorspaceType>(_quantizeInfo->colorspace);
}

void Magick::Options::quantizeDither ( bool ditherFlag_ )
{
  _imageInfo->dither = (MagickBooleanType) ditherFlag_;
  _quantizeInfo->dither = (MagickBooleanType) ditherFlag_;
}
bool Magick::Options::quantizeDither ( void ) const
{
  return static_cast<bool>(_imageInfo->dither);
}

void Magick::Options::quantizeTreeDepth ( size_t treeDepth_ )
{
  _quantizeInfo->tree_depth = treeDepth_;
}
size_t Magick::Options::quantizeTreeDepth ( void ) const
{
  return _quantizeInfo->tree_depth;
}

void Magick::Options::resolutionUnits ( Magick::ResolutionType resolutionUnits_ )
{
  _imageInfo->units = resolutionUnits_;
}
Magick::ResolutionType Magick::Options::resolutionUnits ( void ) const
{
  return static_cast<Magick::ResolutionType>(_imageInfo->units);
}

void Magick::Options::samplingFactor ( const std::string &samplingFactor_ )
{
  if ( samplingFactor_.length() == 0 )
    _imageInfo->sampling_factor=(char *) RelinquishMagickMemory(_imageInfo->sampling_factor);
  else
    Magick::CloneString( &_imageInfo->sampling_factor, samplingFactor_ );
}
std::string Magick::Options::samplingFactor ( void ) const
{
  if ( _imageInfo->sampling_factor )
    return std::string( _imageInfo->sampling_factor );

  return std::string();
}

void Magick::Options::size ( const Geometry &geometry_ )
{
  _imageInfo->size=(char *) RelinquishMagickMemory(_imageInfo->size);

  if ( geometry_.isValid() )
    Magick::CloneString( &_imageInfo->size, geometry_ );
}
Magick::Geometry Magick::Options::size ( void ) const
{
  if ( _imageInfo->size )
    return Geometry( _imageInfo->size );

  return Geometry();
}

void Magick::Options::strokeAntiAlias( bool flag_ )
{
  flag_ ? _drawInfo->stroke_antialias=MagickTrue : _drawInfo->stroke_antialias=MagickFalse;
}
bool Magick::Options::strokeAntiAlias( void ) const
{
  return (_drawInfo->stroke_antialias != 0 ? true : false);
}

// Color to use when drawing object outlines
void Magick::Options::strokeColor ( const Magick::Color &strokeColor_ )
{
  _drawInfo->stroke = strokeColor_;
}
Magick::Color Magick::Options::strokeColor ( void ) const
{
  return _drawInfo->stroke;
}

void Magick::Options::strokeDashArray ( const double* strokeDashArray_ )
{
  _drawInfo->dash_pattern=(double *)
    RelinquishMagickMemory(_drawInfo->dash_pattern);

  if(strokeDashArray_)
    {
      // Count elements in dash array
      size_t x;
      for (x=0; strokeDashArray_[x]; x++) ;
      // Allocate elements
      _drawInfo->dash_pattern =
        static_cast<double*>(AcquireMagickMemory((x+1)*sizeof(double)));
      // Copy elements
      memcpy(_drawInfo->dash_pattern,strokeDashArray_,
             (x+1)*sizeof(double));
    }
}
const double* Magick::Options::strokeDashArray ( void ) const
{
  return _drawInfo->dash_pattern;
}

void Magick::Options::strokeDashOffset ( double strokeDashOffset_ )
{
  _drawInfo->dash_offset = strokeDashOffset_;
}
double Magick::Options::strokeDashOffset ( void ) const
{
  return _drawInfo->dash_offset;
}

// Specify the shape to be used at the end of open subpaths when they
// are stroked. Values of LineCap are ButtCap, RoundCap, and
// SquareCap.
void Magick::Options::strokeLineCap ( Magick::LineCap lineCap_ )
{
  _drawInfo->linecap = lineCap_;
}
Magick::LineCap Magick::Options::strokeLineCap ( void ) const
{
  return _drawInfo->linecap;
}

// Specify the shape to be used at the corners of paths (or other
// vector shapes) when they are stroked.
void Magick::Options::strokeLineJoin ( Magick::LineJoin lineJoin_ )
{
  _drawInfo->linejoin = lineJoin_;
}
Magick::LineJoin Magick::Options::strokeLineJoin ( void ) const
{
  return _drawInfo->linejoin;
}

// miterLimit for drawing lines, circles, ellipses, etc.
void Magick::Options::strokeMiterLimit ( size_t miterLimit_ )
{
  _drawInfo->miterlimit = miterLimit_;
}
size_t Magick::Options::strokeMiterLimit ( void ) const
{
  return _drawInfo->miterlimit;
}

// Pattern image to use for stroked outlines
void Magick::Options::strokePattern ( const MagickCore::Image *strokePattern_ )
{
  if ( _drawInfo->stroke_pattern )
    {
      DestroyImageList( _drawInfo->stroke_pattern );
      _drawInfo->stroke_pattern = 0;
    }

  if ( strokePattern_ )
    {
      ExceptionInfo exceptionInfo;
      GetExceptionInfo( &exceptionInfo );
      _drawInfo->stroke_pattern =
	CloneImage( const_cast<MagickCore::Image*>(strokePattern_),
		    0,
		    0,
		    MagickTrue,
		    &exceptionInfo );
      throwException( exceptionInfo );
      (void) DestroyExceptionInfo( &exceptionInfo );
    }
}
const MagickCore::Image* Magick::Options::strokePattern ( void  ) const
{
  return _drawInfo->stroke_pattern;
}

// Stroke width for drawing lines, circles, ellipses, etc.
void Magick::Options::strokeWidth ( double strokeWidth_ )
{
  _drawInfo->stroke_width = strokeWidth_;
}
double Magick::Options::strokeWidth ( void ) const
{
  return _drawInfo->stroke_width;
}

void Magick::Options::subImage ( size_t subImage_ )
{
  _imageInfo->scene = subImage_;
}
size_t Magick::Options::subImage ( void ) const
{
  return _imageInfo->scene;
}

void Magick::Options::subRange ( size_t subRange_ )
{
  _imageInfo->number_scenes = subRange_;
}
size_t Magick::Options::subRange ( void ) const
{
  return _imageInfo->number_scenes;
}

// Annotation text encoding (e.g. "UTF-16")
void Magick::Options::textEncoding ( const std::string &encoding_ )
{
  CloneString(&_drawInfo->encoding, encoding_.c_str());
}
std::string Magick::Options::textEncoding ( void ) const
{
  if ( _drawInfo->encoding && *_drawInfo->encoding )
    return std::string( _drawInfo->encoding );
  
  return std::string();
}

void Magick::Options::tileName ( const std::string &tileName_ )
{
  if ( tileName_.length() == 0 )
    _imageInfo->tile=(char *) RelinquishMagickMemory(_imageInfo->tile);
  else
    Magick::CloneString( &_imageInfo->tile, tileName_ );
}
std::string Magick::Options::tileName ( void ) const
{
  if ( _imageInfo->tile )
    return std::string( _imageInfo->tile );
  return std::string();
}

// Image representation type
void Magick::Options::type ( const Magick::ImageType type_ )
{
  _imageInfo->type = type_;
}
Magick::ImageType Magick::Options::type ( void ) const
{
  return _imageInfo->type;
}

// Origin of coordinate system to use when annotating with text or drawing
void Magick::Options::transformOrigin ( double tx_, double ty_ )
{
  AffineMatrix current = _drawInfo->affine;
  AffineMatrix affine;
  affine.sx=1.0;
  affine.rx=0.0;
  affine.ry=0.0;
  affine.sy=1.0;
  affine.tx=0.0;
  affine.ty=0.0;

  affine.tx = tx_;
  affine.ty = ty_;

  _drawInfo->affine.sx=current.sx*affine.sx+current.ry*affine.rx;
  _drawInfo->affine.rx=current.rx*affine.sx+current.sy*affine.rx;
  _drawInfo->affine.ry=current.sx*affine.ry+current.ry*affine.sy;
  _drawInfo->affine.sy=current.rx*affine.ry+current.sy*affine.sy;
  _drawInfo->affine.tx=current.sx*affine.tx+current.ry*affine.ty+current.tx;
  _drawInfo->affine.ty=current.rx*affine.tx+current.sy*affine.ty+current.ty;
}

// Reset transformation parameters to default
void Magick::Options::transformReset ( void )
{
  _drawInfo->affine.sx=1.0;
  _drawInfo->affine.rx=0.0;
  _drawInfo->affine.ry=0.0;
  _drawInfo->affine.sy=1.0;
  _drawInfo->affine.tx=0.0;
  _drawInfo->affine.ty=0.0;
}

// Rotation to use when annotating with text or drawing
void Magick::Options::transformRotation ( double angle_ )
{
  AffineMatrix current = _drawInfo->affine;
  AffineMatrix affine;
  affine.sx=1.0;
  affine.rx=0.0;
  affine.ry=0.0;
  affine.sy=1.0;
  affine.tx=0.0;
  affine.ty=0.0;

  affine.sx=cos(DegreesToRadians(fmod(angle_,360.0)));
  affine.rx=(-sin(DegreesToRadians(fmod(angle_,360.0))));
  affine.ry=sin(DegreesToRadians(fmod(angle_,360.0)));
  affine.sy=cos(DegreesToRadians(fmod(angle_,360.0)));

  _drawInfo->affine.sx=current.sx*affine.sx+current.ry*affine.rx;
  _drawInfo->affine.rx=current.rx*affine.sx+current.sy*affine.rx;
  _drawInfo->affine.ry=current.sx*affine.ry+current.ry*affine.sy;
  _drawInfo->affine.sy=current.rx*affine.ry+current.sy*affine.sy;
  _drawInfo->affine.tx=current.sx*affine.tx+current.ry*affine.ty+current.tx;
  _drawInfo->affine.ty=current.rx*affine.tx+current.sy*affine.ty+current.ty;
}

// Scale to use when annotating with text or drawing
void Magick::Options::transformScale ( double sx_, double sy_ )
{
  AffineMatrix current = _drawInfo->affine;
  AffineMatrix affine;
  affine.sx=1.0;
  affine.rx=0.0;
  affine.ry=0.0;
  affine.sy=1.0;
  affine.tx=0.0;
  affine.ty=0.0;

  affine.sx = sx_;
  affine.sy = sy_;

  _drawInfo->affine.sx=current.sx*affine.sx+current.ry*affine.rx;
  _drawInfo->affine.rx=current.rx*affine.sx+current.sy*affine.rx;
  _drawInfo->affine.ry=current.sx*affine.ry+current.ry*affine.sy;
  _drawInfo->affine.sy=current.rx*affine.ry+current.sy*affine.sy;
  _drawInfo->affine.tx=current.sx*affine.tx+current.ry*affine.ty+current.tx;
  _drawInfo->affine.ty=current.rx*affine.tx+current.sy*affine.ty+current.ty;
}

// Skew to use in X axis when annotating with text or drawing
void Magick::Options::transformSkewX ( double skewx_ )
{
  AffineMatrix current = _drawInfo->affine;
  AffineMatrix affine;
  affine.sx=1.0;
  affine.rx=0.0;
  affine.ry=0.0;
  affine.sy=1.0;
  affine.tx=0.0;
  affine.ty=0.0;

  affine.sx=1.0;
  affine.ry=tan(DegreesToRadians(fmod(skewx_,360.0)));
  affine.sy=1.0;

  _drawInfo->affine.sx=current.sx*affine.sx+current.ry*affine.rx;
  _drawInfo->affine.rx=current.rx*affine.sx+current.sy*affine.rx;
  _drawInfo->affine.ry=current.sx*affine.ry+current.ry*affine.sy;
  _drawInfo->affine.sy=current.rx*affine.ry+current.sy*affine.sy;
  _drawInfo->affine.tx=current.sx*affine.tx+current.ry*affine.ty+current.tx;
  _drawInfo->affine.ty=current.rx*affine.tx+current.sy*affine.ty+current.ty;
}

// Skew to use in Y axis when annotating with text or drawing
void Magick::Options::transformSkewY ( double skewy_ )
{
  AffineMatrix current = _drawInfo->affine;
  AffineMatrix affine;
  affine.sx=1.0;
  affine.rx=0.0;
  affine.ry=0.0;
  affine.sy=1.0;
  affine.tx=0.0;
  affine.ty=0.0;

  affine.sx=1.0;
  affine.rx=tan(DegreesToRadians(fmod(skewy_,360.0)));
  affine.sy=1.0;

  _drawInfo->affine.sx=current.sx*affine.sx+current.ry*affine.rx;
  _drawInfo->affine.rx=current.rx*affine.sx+current.sy*affine.rx;
  _drawInfo->affine.ry=current.sx*affine.ry+current.ry*affine.sy;
  _drawInfo->affine.sy=current.rx*affine.ry+current.sy*affine.sy;
  _drawInfo->affine.tx=current.sx*affine.tx+current.ry*affine.ty+current.tx;
  _drawInfo->affine.ty=current.rx*affine.tx+current.sy*affine.ty+current.ty;
}

void Magick::Options::verbose ( bool verboseFlag_ )
{
  _imageInfo->verbose = (MagickBooleanType) verboseFlag_;
}
bool Magick::Options::verbose ( void ) const
{
  return static_cast<bool>(_imageInfo->verbose);
}

void Magick::Options::virtualPixelMethod ( VirtualPixelMethod virtual_pixel_method_ )
{
  _imageInfo->virtual_pixel_method = virtual_pixel_method_;
}
Magick::VirtualPixelMethod Magick::Options::virtualPixelMethod ( void ) const
{
  return static_cast<Magick::VirtualPixelMethod>(_imageInfo->virtual_pixel_method);
}

void Magick::Options::view ( const std::string &view_ )
{
  if ( view_.length() == 0 )
    _imageInfo->view=(char *) RelinquishMagickMemory(_imageInfo->view);
  else
    Magick::CloneString( &_imageInfo->view, view_ );
}
std::string Magick::Options::view ( void ) const
{
  if ( _imageInfo->view )
    return std::string( _imageInfo->view );

  return std::string();
}

void Magick::Options::x11Display ( const std::string &display_ )
{
  if ( display_.length() == 0 )
    _imageInfo->server_name=(char *) RelinquishMagickMemory(_imageInfo->server_name);
  else
    Magick::CloneString( &_imageInfo->server_name, display_ );
}
std::string Magick::Options::x11Display ( void ) const
{
  if ( _imageInfo->server_name )
    return std::string( _imageInfo->server_name );

  return std::string();
}

//
// Internal implementation methods.  Please do not use.
//

MagickCore::DrawInfo * Magick::Options::drawInfo( void )
{
  return _drawInfo;
}

MagickCore::ImageInfo * Magick::Options::imageInfo( void )
{
  return _imageInfo;
}

MagickCore::QuantizeInfo * Magick::Options::quantizeInfo( void )
{
  return _quantizeInfo;
}

// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Color Implementation
//

#define MAGICKCORE_IMPLEMENTATION
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include <string>

using namespace std;

#include "Magick++/Color.h"
#include "Magick++/Exception.h"

//
// Color operator fuctions
//
int Magick::operator == ( const Magick::Color& left_,
			  const Magick::Color& right_ )
{
  return ( ( left_.isValid()      == right_.isValid() ) && 
	   ( left_.redQuantum()   == right_.redQuantum() ) &&
	   ( left_.greenQuantum() == right_.greenQuantum() ) &&
	   ( left_.blueQuantum()  == right_.blueQuantum() )
	  );
}
int Magick::operator != ( const Magick::Color& left_,
			  const Magick::Color& right_ )
{
  return ( ! (left_ == right_) );
}
int Magick::operator >  ( const Magick::Color& left_,
			  const Magick::Color& right_ )
{
  return ( !( left_ < right_ ) && ( left_ != right_ ) );
}
int Magick::operator <  ( const Magick::Color& left_,
			  const Magick::Color& right_ )
{
    if(left_.redQuantum() < right_.redQuantum()) return true;
    if(left_.redQuantum() > right_.redQuantum()) return false;
    if(left_.greenQuantum() < right_.greenQuantum()) return true;
    if(left_.greenQuantum() > right_.greenQuantum()) return false;
    if(left_.blueQuantum() < right_.blueQuantum()) return true;
    return false;
}
int Magick::operator >= ( const Magick::Color& left_,
			  const Magick::Color& right_ )
{
  return ( ( left_ > right_ ) || ( left_ == right_ ) );
}
int Magick::operator <= ( const Magick::Color& left_,
			  const Magick::Color& right_ )
{
  return ( ( left_ < right_ ) || ( left_ == right_ ) );
}

//
// Color Implementation
//

// Default constructor
Magick::Color::Color ( void )
  : _pixel(new PixelPacket),
    _pixelOwn(true),
    _isValid(false),
    _pixelType(RGBPixel)
{
  initPixel();
}

// Construct from RGB
Magick::Color::Color ( Quantum red_,
                       Quantum green_,
                       Quantum blue_ )
  : _pixel(new PixelPacket),
    _pixelOwn(true),
    _isValid(true),
    _pixelType(RGBPixel)
{
  redQuantum   ( red_   );
  greenQuantum ( green_ );
  blueQuantum  ( blue_  );
  alphaQuantum ( OpaqueOpacity );
}

// Construct from RGBA
Magick::Color::Color ( Quantum red_,
                       Quantum green_,
                       Quantum blue_,
                       Quantum alpha_ )
  : _pixel(new PixelPacket),
    _pixelOwn(true),
    _isValid(true),
    _pixelType(RGBAPixel)
{
  redQuantum   ( red_   );
  greenQuantum ( green_ );
  blueQuantum  ( blue_  );
  alphaQuantum ( alpha_ );
}

// Copy constructor
Magick::Color::Color ( const Magick::Color & color_ )
  : _pixel( new PixelPacket ),
    _pixelOwn( true ),
    _isValid( color_._isValid ),
    _pixelType( color_._pixelType )
{
  *_pixel    = *color_._pixel;
}

// Construct from color expressed as C++ string
Magick::Color::Color ( const std::string &x11color_ )
  : _pixel(new PixelPacket),
    _pixelOwn(true),
    _isValid(true),
    _pixelType(RGBPixel)
{
  initPixel();

  // Use operator = implementation
  *this = x11color_;
}

// Construct from color expressed as C string
Magick::Color::Color ( const char * x11color_ )
  : _pixel(new PixelPacket),
    _pixelOwn(true),
    _isValid(true),
    _pixelType(RGBPixel)
{
  initPixel();

  // Use operator = implementation
  *this = x11color_;
}

// Construct color via ImageMagick PixelPacket
Magick::Color::Color ( const PixelPacket &color_ )
  : _pixel(new PixelPacket),
    _pixelOwn(true),	    // We allocated this pixel
    _isValid(true),
    _pixelType(RGBPixel)  // RGB pixel by default
{
  *_pixel = color_;

  if ( color_.opacity != OpaqueOpacity )
    _pixelType = RGBAPixel;
}

// Protected constructor to construct with PixelPacket*
// Used to point Color at a pixel.
Magick::Color::Color ( PixelPacket* rep_, PixelType pixelType_  )
  : _pixel(rep_),
    _pixelOwn(false),
    _isValid(true),
    _pixelType(pixelType_)
{
}

// Destructor
Magick::Color::~Color( void )
{
  if ( _pixelOwn )
    delete _pixel;
  _pixel=0;
}

// Assignment operator
Magick::Color& Magick::Color::operator = ( const Magick::Color& color_ )
{
  // If not being set to ourself
  if ( this != &color_ )
    {
      // Copy pixel value
      *_pixel = *color_._pixel;

      // Validity
      _isValid =  color_._isValid;

      // Copy pixel type
      _pixelType = color_._pixelType;
    }
  return *this;
}

// Set color via X11 color specification string
const Magick::Color& Magick::Color::operator = ( const std::string &x11color_ )
{
  initPixel();
  PixelPacket target_color;
  ExceptionInfo exception;
  GetExceptionInfo( &exception );
  if ( QueryColorDatabase( x11color_.c_str(), &target_color, &exception ) )
    {
      redQuantum( target_color.red );
      greenQuantum( target_color.green );
      blueQuantum( target_color.blue );
      alphaQuantum( target_color.opacity );

      if ( target_color.opacity > OpaqueOpacity )
	_pixelType = RGBAPixel;
      else
	_pixelType = RGBPixel;
    }
  else
    {
      _isValid = false;
      throwException(exception);
    }
  (void) DestroyExceptionInfo( &exception );

  return *this;
}

// Set color via X11 color specification C string
const Magick::Color& Magick::Color::operator = ( const char * x11color_ )
{
  *this = std::string(x11color_);
  return *this;
}

// Return X11 color specification string
Magick::Color::operator std::string() const
{
  if ( !isValid() )
    return std::string("none");

  char colorbuf[MaxTextExtent];

  MagickPixelPacket
    pixel;

  pixel.colorspace=RGBColorspace;
  pixel.matte=_pixelType == RGBAPixel ? MagickTrue : MagickFalse;
  pixel.depth=MAGICKCORE_QUANTUM_DEPTH;
  pixel.red=_pixel->red;
  pixel.green=_pixel->green;
  pixel.blue=_pixel->blue;
  pixel.opacity=_pixel->opacity;
  GetColorTuple( &pixel, MagickTrue, colorbuf );

  return std::string(colorbuf);
}

// Set color via ImageMagick PixelPacket
const Magick::Color& Magick::Color::operator= ( const MagickCore::PixelPacket &color_ )
{
  *_pixel = color_;
  if ( color_.opacity != OpaqueOpacity )
    _pixelType = RGBAPixel;
  else
    _pixelType = RGBPixel;
  return *this;
}

// Set pixel
// Used to point Color at a pixel in an image
void Magick::Color::pixel ( PixelPacket* rep_, PixelType pixelType_ )
{
  if ( _pixelOwn )
    delete _pixel;
  _pixel = rep_;
  _pixelOwn = false;
  _isValid = true;
  _pixelType = pixelType_;
}

// Does object contain valid color?
bool Magick::Color::isValid ( void ) const
{
  return( _isValid );
}
void Magick::Color::isValid ( bool valid_ )
{
  if ( (valid_ && isValid()) || (!valid_ && !isValid()) )
    return;

  if ( !_pixelOwn )
    {
      _pixel = new PixelPacket;
      _pixelOwn = true;
    }

  _isValid=valid_;

  initPixel();
}

//
// ColorHSL Implementation
//

Magick::ColorHSL::ColorHSL ( double hue_,
			     double saturation_,
			     double luminosity_ )
  : Color ()
{
  Quantum red, green, blue;

  ConvertHSLToRGB ( hue_,
		 saturation_,
		 luminosity_,
		 &red,
		 &green,
		 &blue );

  redQuantum   ( red );
  greenQuantum ( green );
  blueQuantum  ( blue );
  alphaQuantum ( OpaqueOpacity );
}

// Null constructor
Magick::ColorHSL::ColorHSL ( )
  : Color ()
{
}

// Copy constructor from base class
Magick::ColorHSL::ColorHSL ( const Magick::Color & color_ )
  : Color( color_ )
{
}

// Destructor
Magick::ColorHSL::~ColorHSL ( )
{
  // Nothing to do
}

void Magick::ColorHSL::hue ( double hue_ )
{
  double hue_val, saturation_val, luminosity_val;
  ConvertRGBToHSL ( redQuantum(),
		 greenQuantum(),
		 blueQuantum(),
		 &hue_val,
		 &saturation_val,
		 &luminosity_val );

  hue_val = hue_;

  Quantum red, green, blue;
  ConvertHSLToRGB ( hue_val,
		 saturation_val,
		 luminosity_val,
		 &red,
		 &green,
		 &blue
		 );

  redQuantum   ( red );
  greenQuantum ( green );
  blueQuantum  ( blue );
}

double Magick::ColorHSL::hue ( void ) const
{
  double hue_val, saturation_val, luminosity_val;
  ConvertRGBToHSL ( redQuantum(),
		 greenQuantum(),
		 blueQuantum(),
		 &hue_val,
		 &saturation_val,
		 &luminosity_val );
  return hue_val;
}

void Magick::ColorHSL::saturation ( double saturation_ )
{
  double hue_val, saturation_val, luminosity_val;
  ConvertRGBToHSL ( redQuantum(),
		 greenQuantum(),
		 blueQuantum(),
		 &hue_val,
		 &saturation_val,
		 &luminosity_val );
  
  saturation_val = saturation_;
  
  Quantum red, green, blue;
  ConvertHSLToRGB ( hue_val,
		 saturation_val,
		 luminosity_val,
		 &red,
		 &green,
		 &blue
		 );

  redQuantum   ( red );
  greenQuantum ( green );
  blueQuantum  ( blue );
}

double Magick::ColorHSL::saturation ( void ) const
{
  double hue_val, saturation_val, luminosity_val;
  ConvertRGBToHSL ( redQuantum(),
		 greenQuantum(),
		 blueQuantum(),
		 &hue_val,
		 &saturation_val,
		 &luminosity_val );
  return saturation_val;
}

void Magick::ColorHSL::luminosity ( double luminosity_ )
{
  double hue_val, saturation_val, luminosity_val;
  ConvertRGBToHSL ( redQuantum(),
		 greenQuantum(),
		 blueQuantum(),
		 &hue_val,
		 &saturation_val,
		 &luminosity_val );
  
  luminosity_val = luminosity_;
  
  Quantum red, green, blue;
  ConvertHSLToRGB ( hue_val,
		 saturation_val,
		 luminosity_val,
		 &red,
		 &green,
		 &blue
		 );
  
  redQuantum   ( red );
  greenQuantum ( green );
  blueQuantum  ( blue );
}

double Magick::ColorHSL::luminosity ( void ) const
{
  double hue_val, saturation_val, luminosity_val;
  ConvertRGBToHSL ( redQuantum(),
		 greenQuantum(),
		 blueQuantum(),
		 &hue_val,
		 &saturation_val,
		 &luminosity_val );
  return luminosity_val;
}

// Assignment from base class
Magick::ColorHSL& Magick::ColorHSL::operator = ( const Magick::Color& color_ )
{
  *static_cast<Magick::Color*>(this) = color_;
  return *this;
}

//
// ColorGray Implementation
//
Magick::ColorGray::ColorGray ( double shade_ )
  : Color ( scaleDoubleToQuantum( shade_ ),
	    scaleDoubleToQuantum( shade_ ),
	    scaleDoubleToQuantum( shade_ ) )
{
  alphaQuantum ( OpaqueOpacity );
}

// Null constructor
Magick::ColorGray::ColorGray ( void )
  : Color ()
{
}

// Copy constructor from base class
Magick::ColorGray::ColorGray ( const Magick::Color & color_ )
  : Color( color_ )
{
}

// Destructor
Magick::ColorGray::~ColorGray ()
{
  // Nothing to do
}

void Magick::ColorGray::shade ( double shade_ )
{
  Quantum gray = scaleDoubleToQuantum( shade_ );
  redQuantum   ( gray );
  greenQuantum ( gray );
  blueQuantum  ( gray );
}

double Magick::ColorGray::shade ( void ) const
{
  return scaleQuantumToDouble ( greenQuantum() );
}

// Assignment from base class
Magick::ColorGray& Magick::ColorGray::operator = ( const Magick::Color& color_ )
{
  *static_cast<Magick::Color*>(this) = color_;
  return *this;
}

//
// ColorMono Implementation
//
Magick::ColorMono::ColorMono ( bool mono_  )
  : Color ( ( mono_ ? QuantumRange : 0 ),
	    ( mono_ ? QuantumRange : 0 ),
	    ( mono_ ? QuantumRange : 0 ) )
{
  alphaQuantum ( OpaqueOpacity );
}

// Null constructor
Magick::ColorMono::ColorMono ( void )
  : Color ()
{
}

// Copy constructor from base class
Magick::ColorMono::ColorMono ( const Magick::Color & color_ )
  : Color( color_ )
{
}

// Destructor
Magick::ColorMono::~ColorMono ()
{
  // Nothing to do
}

void Magick::ColorMono::mono ( bool mono_ )
{
  redQuantum   ( mono_ ? QuantumRange : 0 );
  greenQuantum ( mono_ ? QuantumRange : 0 );
  blueQuantum  ( mono_ ? QuantumRange : 0 );
}

bool Magick::ColorMono::mono ( void ) const
{
  if ( greenQuantum() )
    return true;
  else
    return false;
}

// Assignment from base class
Magick::ColorMono& Magick::ColorMono::operator = ( const Magick::Color& color_ )
{
  *static_cast<Magick::Color*>(this) = color_;
  return *this;
}

//
// ColorRGB Implementation
//

// Construct from red, green, and blue, components
Magick::ColorRGB::ColorRGB ( double red_,
			     double green_,
			     double blue_ )
  : Color ( scaleDoubleToQuantum(red_),
	    scaleDoubleToQuantum(green_),
	    scaleDoubleToQuantum(blue_) )
{
  alphaQuantum ( OpaqueOpacity );
}
// Null constructor
Magick::ColorRGB::ColorRGB ( void )
  : Color ()
{
}
// Copy constructor from base class
Magick::ColorRGB::ColorRGB ( const Magick::Color & color_ )
  : Color( color_ )
{
}
// Destructor
Magick::ColorRGB::~ColorRGB ( void )
{
  // Nothing to do
}

// Assignment from base class
Magick::ColorRGB& Magick::ColorRGB::operator = ( const Magick::Color& color_ )
{
  *static_cast<Magick::Color*>(this) = color_;
  return *this;
}

//
// ColorYUV Implementation
//

//           R = Y          +1.13980*V
//           G = Y-0.39380*U-0.58050*V
//           B = Y+2.02790*U
//
//         U and V, normally -0.5 through 0.5, must be normalized to the range 0
//         through QuantumRange.
//
//           Y =  0.29900*R+0.58700*G+0.11400*B
//           U = -0.14740*R-0.28950*G+0.43690*B
//           V =  0.61500*R-0.51500*G-0.10000*B
//
//         U and V, normally -0.5 through 0.5, are normalized to the range 0
//         through QuantumRange.  Note that U = 0.493*(B-Y), V = 0.877*(R-Y).
//

// Construct from color components
Magick::ColorYUV::ColorYUV ( double y_,
			     double u_,
			     double v_ )
  : Color ( scaleDoubleToQuantum(y_ + 1.13980 * v_ ),
	    scaleDoubleToQuantum(y_ - (0.39380 * u_) - (0.58050 * v_) ),
	    scaleDoubleToQuantum(y_ + 2.02790 * u_ ) )
{
  alphaQuantum ( OpaqueOpacity );
}
// Null constructor
Magick::ColorYUV::ColorYUV ( void )
  : Color ()
{
}
// Copy constructor from base class
Magick::ColorYUV::ColorYUV ( const Magick::Color & color_ )
  : Color( color_ )
{
}
// Destructor
Magick::ColorYUV::~ColorYUV ( void )
{
  // Nothing to do
}

void Magick::ColorYUV::u ( double u_ )
{
  double V = v();
  double Y = y();

  redQuantum   ( scaleDoubleToQuantum( Y + 1.13980 * V ) );
  greenQuantum ( scaleDoubleToQuantum( Y - (0.39380 * u_) - (0.58050 * V) ) );
  blueQuantum  ( scaleDoubleToQuantum( Y + 2.02790 * u_ ) );
}

double Magick::ColorYUV::u ( void ) const
{
  return scaleQuantumToDouble( (-0.14740 * redQuantum()) - (0.28950 *
			       greenQuantum()) + (0.43690 * blueQuantum()) );
}

void Magick::ColorYUV::v ( double v_ )
{
  double U = u();
  double Y = y();

  redQuantum   ( scaleDoubleToQuantum( Y + 1.13980 * v_ ) );
  greenQuantum ( scaleDoubleToQuantum( Y - (0.39380 * U) - (0.58050 * v_) ) );
  blueQuantum  ( scaleDoubleToQuantum( Y + 2.02790 * U ) );
}

double Magick::ColorYUV::v ( void ) const
{
  return scaleQuantumToDouble((0.61500 * redQuantum()) -
                              (0.51500 * greenQuantum()) -
                              (0.10000 * blueQuantum()));
}

void Magick::ColorYUV::y ( double y_ )
{
  double U = u();
  double V = v();

  redQuantum   ( scaleDoubleToQuantum( y_ + 1.13980 * V ) );
  greenQuantum ( scaleDoubleToQuantum( y_ - (0.39380 * U) - (0.58050 * V) ) );
  blueQuantum  ( scaleDoubleToQuantum( y_ + 2.02790 * U ) );
}

double Magick::ColorYUV::y ( void ) const
{
  return scaleQuantumToDouble((0.29900 * redQuantum()) + 
                              (0.58700 * greenQuantum()) +
                              (0.11400 * blueQuantum()));
}

// Assignment from base class
Magick::ColorYUV& Magick::ColorYUV::operator = ( const Magick::Color& color_ )
{
  *static_cast<Magick::Color*>(this) = color_;
  return *this;
}

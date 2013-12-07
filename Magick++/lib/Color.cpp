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

MagickPPExport int Magick::operator == (const Magick::Color &left_,
  const Magick::Color &right_)
{
  return((left_.isValid() == right_.isValid()) &&
    (left_.redQuantum() == right_.redQuantum()) &&
    (left_.greenQuantum() == right_.greenQuantum()) &&
    (left_.blueQuantum() == right_.blueQuantum()));
}

MagickPPExport int Magick::operator != (const Magick::Color &left_,
  const Magick::Color &right_)
{
  return(!(left_ == right_));
}

MagickPPExport int Magick::operator > (const Magick::Color &left_,
  const Magick::Color &right_)
{
  return(!(left_ < right_ ) && (left_ != right_ ));
}

MagickPPExport int Magick::operator < (const Magick::Color &left_,
  const Magick::Color &right_)
{
  if(left_.redQuantum() < right_.redQuantum())
    return(true);
  if(left_.redQuantum() > right_.redQuantum())
    return(false);
  if(left_.greenQuantum() < right_.greenQuantum())
    return(true);
  if(left_.greenQuantum() > right_.greenQuantum())
    return(false);
  if(left_.blueQuantum() < right_.blueQuantum())
    return(true);
  return(false);
}

MagickPPExport int Magick::operator >= (const Magick::Color &left_,
  const Magick::Color &right_)
{
  return((left_ > right_) || (left_ == right_));
}

MagickPPExport int Magick::operator <= (const Magick::Color &left_,
  const Magick::Color &right_)
{
  return((left_ < right_) || (left_ == right_));
}

Magick::Color::Color(void)
  : _pixel(new PixelPacket),_pixelOwn(true),_isValid(false),
    _pixelType(RGBPixel)
{
  initPixel();
}

Magick::Color::Color(Quantum red_,Quantum green_,Quantum blue_)
  : _pixel(new PixelPacket),_pixelOwn(true),_isValid(true),_pixelType(RGBPixel)
{
  redQuantum(red_);
  greenQuantum(green_);
  blueQuantum(blue_);
  alphaQuantum(OpaqueOpacity);
}

Magick::Color::Color(Quantum red_,Quantum green_,Quantum blue_,Quantum alpha_)
  : _pixel(new PixelPacket),_pixelOwn(true),_isValid(true),
    _pixelType(RGBAPixel)
{
  redQuantum(red_);
  greenQuantum(green_);
  blueQuantum(blue_);
  alphaQuantum(alpha_);
}

Magick::Color::Color(const char *x11color_)
  : _pixel(new PixelPacket),_pixelOwn(true),_isValid(true),_pixelType(RGBPixel)
{
  initPixel();

  // Use operator = implementation
  *this=x11color_;
}

Magick::Color::Color(const Magick::Color &color_)
  : _pixel(new PixelPacket),_pixelOwn(true),_isValid(color_._isValid),
    _pixelType(color_._pixelType)
{
  *_pixel=*color_._pixel;
}

Magick::Color::Color(const PixelPacket &color_)
  : _pixel(new PixelPacket),_pixelOwn(true),_isValid(true),_pixelType(RGBPixel)
{
  *_pixel=color_;

  if (color_.opacity != OpaqueOpacity)
    _pixelType=RGBAPixel;
}

Magick::Color::Color(const std::string &x11color_)
  : _pixel(new PixelPacket),_pixelOwn(true),_isValid(true),_pixelType(RGBPixel)
{
  initPixel();

  // Use operator = implementation
  *this=x11color_;
}

Magick::Color::~Color(void)
{
  if (_pixelOwn)
    delete _pixel;

  _pixel=(PixelPacket *)NULL;
}

const Magick::Color& Magick::Color::operator=(const char *x11color_)
{
  *this=std::string(x11color_);
  return(*this);
}

Magick::Color& Magick::Color::operator=(const Magick::Color& color_)
{
  // If not being set to ourself
  if (this != &color_)
    {
      // Copy pixel value
      *_pixel=*color_._pixel;

      // Validity
      _isValid=color_._isValid;

      // Copy pixel type
      _pixelType=color_._pixelType;
    }
  return(*this);
}

const Magick::Color& Magick::Color::operator=
  (const MagickCore::PixelPacket &color_)
{
  *_pixel=color_;
  if (color_.opacity != OpaqueOpacity)
    _pixelType=RGBAPixel;
  else
    _pixelType=RGBPixel;

  return(*this);
}

// Set color via X11 color specification string
const Magick::Color& Magick::Color::operator=(const std::string &x11color_)
{
  ExceptionInfo
    exception;

  PixelPacket
    target_color;

  initPixel();
  GetExceptionInfo(&exception);
  if (QueryColorDatabase(x11color_.c_str(),&target_color,&exception))
    {
      redQuantum( target_color.red );
      greenQuantum( target_color.green );
      blueQuantum( target_color.blue );
      alphaQuantum( target_color.opacity );

      if (target_color.opacity > OpaqueOpacity)
        _pixelType=RGBAPixel;
      else
        _pixelType=RGBPixel;
    }
  else
    {
      _isValid=false;
      throwException(exception);
    }
  (void) DestroyExceptionInfo(&exception);

  return(*this);
}

Magick::Color::operator std::string() const
{
  char
    colorbuf[MaxTextExtent];

  MagickPixelPacket
    pixel;

  if (!isValid())
    return std::string("none");

  pixel.colorspace=RGBColorspace;
  pixel.matte=_pixelType == RGBAPixel ? MagickTrue : MagickFalse;
  pixel.depth=MAGICKCORE_QUANTUM_DEPTH;
  pixel.red=_pixel->red;
  pixel.green=_pixel->green;
  pixel.blue=_pixel->blue;
  pixel.opacity=_pixel->opacity;
  GetColorTuple(&pixel,MagickTrue,colorbuf);

  return(std::string(colorbuf));
}

bool Magick::Color::isValid(void) const
{
  return(_isValid);
}

void Magick::Color::isValid(bool valid_)
{
  if ((valid_ && isValid()) || (!valid_ && !isValid()))
    return;

  if (!_pixelOwn)
    {
      _pixel=new PixelPacket;
      _pixelOwn=true;
    }

  _isValid=valid_;

  initPixel();
}

Magick::Color::Color(PixelPacket *rep_,PixelType pixelType_)
  : _pixel(rep_),_pixelOwn(false),_isValid(true),_pixelType(pixelType_)
{
}

void Magick::Color::pixel(PixelPacket *rep_,PixelType pixelType_)
{
  if (_pixelOwn)
    delete _pixel;

  _pixel=rep_;
  _pixelOwn=false;
  _isValid=true;
  _pixelType=pixelType_;
}

Magick::ColorGray::ColorGray(void)
  : Color()
{
}

Magick::ColorGray::ColorGray(const Magick::Color & color_)
  : Color(color_)
{
}

Magick::ColorGray::ColorGray(double shade_)
  : Color(scaleDoubleToQuantum(shade_),scaleDoubleToQuantum(shade_),
          scaleDoubleToQuantum(shade_))
{
  alphaQuantum(OpaqueOpacity);
}

Magick::ColorGray::~ColorGray()
{
}

void Magick::ColorGray::shade(double shade_)
{
  Quantum gray=scaleDoubleToQuantum(shade_);
  redQuantum(gray);
  greenQuantum(gray);
  blueQuantum(gray);
}

double Magick::ColorGray::shade(void) const
{
  return(scaleQuantumToDouble(greenQuantum()));
}

Magick::ColorGray& Magick::ColorGray::operator = ( const Magick::Color& color_ )
{
  *static_cast<Magick::Color*>(this)=color_;
  return(*this);
}

Magick::ColorHSL::ColorHSL(void)
  : Color()
{
}

Magick::ColorHSL::ColorHSL(const Magick::Color & color_)
  : Color( color_ )
{
}

Magick::ColorHSL::ColorHSL(double hue_,double saturation_,double luminosity_)
  : Color()
{
  Quantum
    blue,
    green,
    red;

  ConvertHSLToRGB(hue_,saturation_,luminosity_,&red,&green,&blue);

  redQuantum(red);
  greenQuantum(green);
  blueQuantum(blue);
  alphaQuantum(OpaqueOpacity);
}

Magick::ColorHSL::~ColorHSL()
{
}

Magick::ColorHSL& Magick::ColorHSL::operator=(const Magick::Color &color_)
{
  *static_cast<Magick::Color*>(this)=color_;
  return (*this);
}

void Magick::ColorHSL::hue(double hue_)
{
  double
    hue,
    luminosity,
    saturation;

  Quantum
    blue,
    green,
    red;

  ConvertRGBToHSL(redQuantum(),greenQuantum(),blueQuantum(),&hue,&saturation,
    &luminosity);

  hue=hue_;

  ConvertHSLToRGB(hue,saturation,luminosity,&red,&green,&blue);

  redQuantum(red);
  greenQuantum(green);
  blueQuantum(blue);
}

double Magick::ColorHSL::hue(void) const
{
  double
    hue,
    luminosity,
    saturation;

  ConvertRGBToHSL(redQuantum(),greenQuantum(),blueQuantum(),&hue,&saturation,
    &luminosity);

  return(hue);
}

void Magick::ColorHSL::luminosity(double luminosity_)
{
  double
    hue,
    luminosity,
    saturation;

  Quantum
    blue,
    green,
    red;

  ConvertRGBToHSL(redQuantum(),greenQuantum(),blueQuantum(),&hue,&saturation,
    &luminosity);

  luminosity=luminosity_;

  ConvertHSLToRGB(hue,saturation,luminosity,&red,&green,&blue);

  redQuantum(red);
  greenQuantum(green);
  blueQuantum(blue);
}

double Magick::ColorHSL::luminosity(void) const
{
  double
    hue,
    saturation,
    luminosity;

  ConvertRGBToHSL(redQuantum(),greenQuantum(),blueQuantum(),&hue,&saturation,
    &luminosity);

  return(luminosity);
}

void Magick::ColorHSL::saturation(double saturation_)
{
  double
    hue,
    luminosity,
    saturation;

  Quantum
    blue,
    green,
    red;

  ConvertRGBToHSL(redQuantum(),greenQuantum(),blueQuantum(),&hue,&saturation,
    &luminosity);

  saturation=saturation_;

  ConvertHSLToRGB(hue,saturation,luminosity,&red,&green,&blue);

  redQuantum(red);
  greenQuantum(green);
  blueQuantum(blue);
}

double Magick::ColorHSL::saturation(void) const
{
  double
    hue,
    luminosity,
    saturation;

  ConvertRGBToHSL(redQuantum(),greenQuantum(),blueQuantum(),&hue,&saturation,
    &luminosity);

  return(saturation);
}

Magick::ColorMono::ColorMono(void)
  : Color()
{
}

Magick::ColorMono::ColorMono(bool mono_)
  : Color((mono_ ? QuantumRange : 0),
          (mono_ ? QuantumRange : 0),
          (mono_ ? QuantumRange : 0))
{
  alphaQuantum(OpaqueOpacity);
}

Magick::ColorMono::ColorMono(const Magick::Color &color_)
  : Color(color_)
{
}

Magick::ColorMono::~ColorMono()
{
}

Magick::ColorMono& Magick::ColorMono::operator=(const Magick::Color &color_)
{
  *static_cast<Magick::Color*>(this)=color_;
  return(*this);
}

void Magick::ColorMono::mono(bool mono_)
{
  redQuantum(mono_ ? QuantumRange : 0);
  greenQuantum(mono_ ? QuantumRange : 0);
  blueQuantum(mono_ ? QuantumRange : 0);
}

bool Magick::ColorMono::mono(void) const
{
  return(greenQuantum() == 0);
}

Magick::ColorRGB::ColorRGB(void)
  : Color()
{
}

Magick::ColorRGB::ColorRGB(const Magick::Color & color_)
  : Color(color_)
{
}

Magick::ColorRGB::ColorRGB(double red_,double green_,double blue_)
  : Color(scaleDoubleToQuantum(red_),scaleDoubleToQuantum(green_),
          scaleDoubleToQuantum(blue_))
{
  alphaQuantum(OpaqueOpacity);
}

Magick::ColorRGB::~ColorRGB(void)
{
}

Magick::ColorRGB& Magick::ColorRGB::operator=(const Magick::Color& color_)
{
  *static_cast<Magick::Color*>(this)=color_;
  return(*this);
}

Magick::ColorYUV::ColorYUV(void)
  : Color()
{
}

Magick::ColorYUV::ColorYUV(const Magick::Color &color_)
  : Color(color_)
{
}

Magick::ColorYUV::ColorYUV(double y_,double u_,double v_)
  : Color(scaleDoubleToQuantum(y_ + 1.13980 * v_),
          scaleDoubleToQuantum(y_ - (0.39380 * u_) - (0.58050 * v_)),
          scaleDoubleToQuantum(y_ + 2.02790 * u_))
{
  alphaQuantum(OpaqueOpacity);
}

Magick::ColorYUV::~ColorYUV(void)
{
}

Magick::ColorYUV& Magick::ColorYUV::operator=(const Magick::Color &color_)
{
  *static_cast<Magick::Color*>(this)=color_;
  return(*this);
}

void Magick::ColorYUV::u(double u_)
{
  double V = v();
  double Y = y();

  redQuantum(scaleDoubleToQuantum(Y + 1.13980 * V ));
  greenQuantum(scaleDoubleToQuantum( Y - (0.39380 * u_) - (0.58050 * V)));
  blueQuantum(scaleDoubleToQuantum( Y + 2.02790 * u_));
}

double Magick::ColorYUV::u(void) const
{
  return scaleQuantumToDouble((-0.14740 * redQuantum()) - (0.28950 *
    greenQuantum()) + (0.43690 * blueQuantum()));
}

void Magick::ColorYUV::v(double v_)
{
  double U = u();
  double Y = y();

  redQuantum(scaleDoubleToQuantum( Y + 1.13980 * v_ ));
  greenQuantum(scaleDoubleToQuantum( Y - (0.39380 * U) - (0.58050 * v_) ));
  blueQuantum(scaleDoubleToQuantum( Y + 2.02790 * U ));
}

double Magick::ColorYUV::v(void) const
{
  return scaleQuantumToDouble((0.61500 * redQuantum()) - (0.51500 *
    greenQuantum()) - (0.10000 * blueQuantum()));
}

void Magick::ColorYUV::y(double y_)
{
  double U = u();
  double V = v();

  redQuantum(scaleDoubleToQuantum(y_ + 1.13980 * V));
  greenQuantum(scaleDoubleToQuantum(y_ - (0.39380 * U) - (0.58050 * V)));
  blueQuantum(scaleDoubleToQuantum(y_ + 2.02790 * U));
}

double Magick::ColorYUV::y(void) const
{
  return scaleQuantumToDouble((0.29900 * redQuantum()) + (0.58700 *
    greenQuantum()) + (0.11400 * blueQuantum()));
}

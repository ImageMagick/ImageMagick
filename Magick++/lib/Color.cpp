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
    (left_.quantumRed() == right_.quantumRed()) &&
    (left_.quantumGreen() == right_.quantumGreen()) &&
    (left_.quantumBlue() == right_.quantumBlue()));
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

MagickPPExport int Magick::operator < ( const Magick::Color &left_,
  const Magick::Color &right_)
{
  if(left_.quantumRed() < right_.quantumRed())
    return(true);
  if(left_.quantumRed() > right_.quantumRed())
    return(false);
  if(left_.quantumGreen() < right_.quantumGreen())
    return(true);
  if(left_.quantumGreen() > right_.quantumGreen())
    return(false);
  if(left_.quantumBlue() < right_.quantumBlue())
    return(true);
  return(false);
}

MagickPPExport int Magick::operator >= (const Magick::Color &left_,
  const Magick::Color &right_)
{
  return((left_ > right_) || (left_ == right_));
}

MagickPPExport int Magick::operator <= ( const Magick::Color &left_,
  const Magick::Color &right_)
{
  return((left_ < right_) || (left_ == right_));
}

Magick::Color::Color(void)
  : _pixel(new PixelInfo),_isValid(false),_pixelOwn(true),_pixelType(RGBPixel)
{
  initPixel();
}

Magick::Color::Color(const Quantum red_,const Quantum green_,
  const Quantum blue_)
  : _pixel(new PixelInfo),_isValid(true),_pixelOwn(true),_pixelType(RGBPixel)
{
  quantumRed(red_);
  quantumGreen(green_);
  quantumBlue(blue_);
  quantumAlpha(OpaqueAlpha);
}

Magick::Color::Color(const Quantum red_,const Quantum green_,
  const Quantum blue_, const Quantum alpha_)
  : _pixel(new PixelInfo),_isValid(true),_pixelOwn(true),_pixelType(RGBAPixel)
{
  quantumRed(red_);
  quantumGreen(green_);
  quantumBlue(blue_);
  quantumAlpha(alpha_);
}

Magick::Color::Color(const char *color_)
  : _pixel(new PixelInfo),_isValid(true),_pixelOwn(true),_pixelType(RGBPixel)
{
  initPixel();

  // Use operator = implementation
  *this=color_;
}

Magick::Color::Color(const Magick::Color &color_)
  : _pixel(new PixelInfo),_isValid(color_._isValid),_pixelOwn(true),
    _pixelType(color_._pixelType)
{
  *_pixel=*color_._pixel;
}

Magick::Color::Color(const PixelInfo &color_)
  : _pixel(new PixelInfo),_isValid(true),_pixelOwn(true),_pixelType(RGBPixel)
{
  *_pixel=color_;

  if (color_.alpha != OpaqueAlpha)
    _pixelType=RGBAPixel;
}

Magick::Color::Color(const std::string &color_)
  : _pixel(new PixelInfo),_isValid(true),_pixelOwn(true),_pixelType(RGBPixel)
{
  initPixel();

  // Use operator = implementation
  *this=color_;
}

Magick::Color::~Color(void)
{
  if (_pixelOwn)
    delete _pixel;

  _pixel=(PixelInfo *)NULL;
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

const Magick::Color& Magick::Color::operator=(const char *color_)
{
  *this=std::string(color_);
  return(*this);
}

const Magick::Color& Magick::Color::operator=(const MagickCore::PixelInfo &color_)
{
  *_pixel=color_;
  if (color_.alpha != OpaqueAlpha)
    _pixelType=RGBAPixel;
  else
    _pixelType=RGBPixel;

  return(*this);
}

const Magick::Color& Magick::Color::operator=(const std::string &color_)
{
  PixelInfo
    target_color;

  initPixel();
  GetPPException;
  if (QueryColorCompliance(color_.c_str(),AllCompliance,&target_color,
      exceptionInfo))
    {
      quantumRed(target_color.red);
      quantumGreen(target_color.green);
      quantumBlue(target_color.blue);
      quantumAlpha(target_color.alpha);

      if (quantumAlpha() != OpaqueAlpha)
        _pixelType=RGBAPixel;
      else
         _pixelType=RGBPixel;
    }
  else
    _isValid = false;
  ThrowPPException;

  return(*this);
}

Magick::Color::operator MagickCore::PixelInfo() const
{
  return *_pixel;
}

Magick::Color::operator std::string() const
{
  char
    colorbuf[MaxTextExtent];

  PixelInfo
    pixel;

  if (!isValid())
    return std::string("none");

  pixel.colorspace=RGBColorspace;
  pixel.alpha_trait=_pixelType == RGBAPixel ? BlendPixelTrait :
    UndefinedPixelTrait;
  pixel.depth=MAGICKCORE_QUANTUM_DEPTH;
  pixel.red=_pixel->red;
  pixel.green=_pixel->green;
  pixel.blue=_pixel->blue;
  pixel.alpha=_pixel->alpha;
  GetColorTuple(&pixel,MagickTrue,colorbuf);

  return(std::string(colorbuf));
}

void Magick::Color::alpha(const double alpha_)
{
  quantumAlpha(scaleDoubleToQuantum(alpha_));
}

double Magick::Color::alpha(void) const
{
  return scaleQuantumToDouble(quantumAlpha());
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
      _pixel=new PixelInfo;
      _pixelOwn=true;
    }

  _isValid=valid_;

  initPixel();
}

void Magick::Color::quantumAlpha(const Magick::Quantum alpha_)
{
  _pixel->alpha=alpha_;
  _isValid=true ;
}

Magick::Quantum Magick::Color::quantumAlpha(void) const
{
  return _pixel->alpha;
}

void Magick::Color::quantumBlue(const Magick::Quantum blue_)
{
  _pixel->blue=blue_;
  _isValid=true;
}

Magick::Quantum Magick::Color::quantumBlue(void) const
{
  return _pixel->blue;
}

void Magick::Color::quantumGreen(const Magick::Quantum green_)
{
  _pixel->green=green_;
  _isValid=true;
}

Magick::Quantum Magick::Color::quantumGreen(void) const
{
  return _pixel->green;
}

void Magick::Color::quantumRed(const Magick::Quantum red_)
{
  _pixel->red=red_;
  _isValid=true;
}

Magick::Quantum Magick::Color::quantumRed(void) const
{
  return _pixel->red;
}

Magick::Color::Color(PixelInfo* rep_,PixelType pixelType_)
  : _pixel(rep_),_pixelOwn(false),_isValid(true),_pixelType(pixelType_)
{
}

void Magick::Color::pixel(PixelInfo *rep_,PixelType pixelType_)
{
  if (_pixelOwn)
    delete _pixel;

  _pixel=rep_;
  _pixelOwn=false;
  _isValid=true;
  _pixelType=pixelType_;
}

Magick::Quantum Magick::Color::scaleDoubleToQuantum(const double double_)
{
  return (static_cast<Magick::Quantum>(double_*QuantumRange));
}

double Magick::Color::scaleQuantumToDouble(const Magick::Quantum quantum_)
{
#if (MAGICKCORE_QUANTUM_DEPTH != 64)
  return (static_cast<double>(quantum_)/QuantumRange);
#else
  return (quantum_/QuantumRange);
#endif
}

void Magick::Color::initPixel()
{
  _pixel->red=0;
  _pixel->green=0;
  _pixel->blue=0;
  _pixel->alpha=TransparentAlpha;
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
  quantumAlpha(OpaqueAlpha);
}

Magick::ColorGray::~ColorGray()
{
}

void Magick::ColorGray::shade(double shade_)
{
  Quantum gray=scaleDoubleToQuantum(shade_);
  quantumRed(gray);
  quantumGreen(gray);
  quantumBlue(gray);
}

double Magick::ColorGray::shade(void) const
{
  return(scaleQuantumToDouble(quantumGreen()));
}

Magick::ColorGray& Magick::ColorGray::operator=(const Magick::Color& color_)
{
  *static_cast<Magick::Color*>(this)=color_;
  return(*this);
}

Magick::ColorGray::ColorGray(PixelInfo *rep_,PixelType pixelType_)
: Color(rep_,pixelType_)
{
}

Magick::ColorHSL::ColorHSL(void)
  : Color()
{
}

Magick::ColorHSL::ColorHSL(const Magick::Color &color_)
  : Color(color_)
{
}

Magick::ColorHSL::ColorHSL(const double hue_,const double saturation_,
  const double luminosity_)
  : Color()
{
  double
    blue,
    green,
    red;

  ConvertHSLToRGB(hue_,saturation_,luminosity_,&red,&green,&blue);

  quantumRed(red);
  quantumGreen(green);
  quantumBlue(blue);
  quantumAlpha(OpaqueAlpha);
}

Magick::ColorHSL::~ColorHSL()
{
}

Magick::ColorHSL& Magick::ColorHSL::operator=(const Magick::Color& color_)
{
  *static_cast<Magick::Color*>(this) = color_;
  return(*this);
}

void Magick::ColorHSL::hue(const double hue_)
{
  double
    hue,
    luminosity,
    saturation;

  double
    blue,
    green,
    red;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &luminosity);

  hue=hue_;

  ConvertHSLToRGB(hue,saturation,luminosity,&red,&green,&blue);

  quantumRed(ClampToQuantum(red));
  quantumGreen(ClampToQuantum(green));
  quantumBlue(ClampToQuantum(blue));
}

double Magick::ColorHSL::hue(void) const
{
  double
    hue,
    luminosity,
    saturation;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &luminosity);

  return(hue);
}

void Magick::ColorHSL::luminosity(const double luminosity_)
{
  double
    hue,
    luminosity,
    saturation;

  double
    blue,
    green,
    red;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &luminosity);

  luminosity=luminosity_;

  ConvertHSLToRGB(hue,saturation,luminosity,&red,&green,&blue);

  quantumRed(ClampToQuantum(red));
  quantumGreen(ClampToQuantum(green));
  quantumBlue(ClampToQuantum(blue));
}

double Magick::ColorHSL::luminosity ( void ) const
{
  double
    hue,
    luminosity,
    saturation;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &luminosity);

  return(luminosity);
}

void Magick::ColorHSL::saturation(const double saturation_)
{
  double
    hue,
    luminosity,
    saturation;

  double
    blue,
    green,
    red;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &luminosity);

  saturation=saturation_;

  ConvertHSLToRGB(hue,saturation,luminosity,&red,&green,&blue);

  quantumRed(ClampToQuantum(red));
  quantumGreen(ClampToQuantum(green));
  quantumBlue(ClampToQuantum(blue));
}

double Magick::ColorHSL::saturation(void) const
{
  double
    hue,
    luminosity,
    saturation;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &luminosity);

  return(saturation);
}

Magick::ColorMono::ColorMono(void)
  : Color()
{
}

Magick::ColorMono::ColorMono(const bool mono_)
  : Color((mono_ ? QuantumRange : 0),(mono_ ? QuantumRange : 0),
          (mono_ ? QuantumRange : 0))
{
  quantumAlpha(OpaqueAlpha);
}

Magick::ColorMono::ColorMono(const Magick::Color &color_)
  : Color(color_)
{
}

Magick::ColorMono::~ColorMono()
{
}

Magick::ColorMono& Magick::ColorMono::operator=(const Magick::Color& color_)
{
  *static_cast<Magick::Color*>(this)=color_;
  return(*this);
}

void Magick::ColorMono::mono(bool mono_)
{
  quantumRed(mono_ ? QuantumRange : 0);
  quantumGreen(mono_ ? QuantumRange : 0);
  quantumBlue(mono_ ? QuantumRange : 0);
}

bool Magick::ColorMono::mono(void) const
{
  return(quantumGreen() == 0);
}

Magick::ColorMono::ColorMono(PixelInfo *rep_,PixelType pixelType_)
  : Color(rep_,pixelType_)
{
}

Magick::ColorRGB::ColorRGB(void)
  : Color()
{
}

Magick::ColorRGB::ColorRGB(const Magick::Color &color_)
  : Color(color_)
{
}

Magick::ColorRGB::ColorRGB(const double red_,const double green_,
  const double blue_)
  : Color(scaleDoubleToQuantum(red_),scaleDoubleToQuantum(green_),
          scaleDoubleToQuantum(blue_))
{
  quantumAlpha(OpaqueAlpha);
}

Magick::ColorRGB::ColorRGB(const double red_,const double green_,
  const double blue_,const double alpha_)
  : Color(scaleDoubleToQuantum(red_),scaleDoubleToQuantum(green_),
          scaleDoubleToQuantum(blue_),scaleDoubleToQuantum(alpha_))
{
}

Magick::ColorRGB::~ColorRGB(void)
{
}

Magick::ColorRGB& Magick::ColorRGB::operator=(const Magick::Color& color_)
{
  *static_cast<Magick::Color*>(this)=color_;
  return(*this);
}

void Magick::ColorRGB::blue(const double blue_)
{
  quantumBlue(scaleDoubleToQuantum(blue_));
}

double Magick::ColorRGB::blue(void) const
{
  return scaleQuantumToDouble(quantumBlue());
}

void Magick::ColorRGB::green(const double green_)
{
  quantumGreen(scaleDoubleToQuantum(green_));
}

double Magick::ColorRGB::green(void) const
{
  return scaleQuantumToDouble(quantumGreen());
}

void Magick::ColorRGB::red(const double red_)
{
  quantumRed(scaleDoubleToQuantum(red_));
}

double Magick::ColorRGB::red(void) const
{
  return scaleQuantumToDouble(quantumRed());
}

Magick::ColorRGB::ColorRGB(PixelInfo *rep_,PixelType pixelType_)
  : Color(rep_,pixelType_)
{
}

Magick::ColorYUV::ColorYUV(void)
  : Color()
{
}

Magick::ColorYUV::ColorYUV(const Magick::Color &color_)
  : Color(color_)
{
}

Magick::ColorYUV::ColorYUV(const double y_,const double u_,const double v_)
  : Color()
{
  convert(y_, u_, v_);
  quantumAlpha(OpaqueAlpha);
}

Magick::ColorYUV::~ColorYUV(void)
{
}

Magick::ColorYUV& Magick::ColorYUV::operator=(const Magick::Color &color_)
{
  *static_cast<Magick::Color*>(this)=color_;
  return(*this);
}

void Magick::ColorYUV::u(const double u_)
{
  convert(y(), u_, v());
}

double Magick::ColorYUV::u(void) const
{
  return scaleQuantumToDouble((-0.14740 * quantumRed()) - (0.28950 *
    quantumGreen()) + (0.43690 * quantumBlue()));
}

void Magick::ColorYUV::v(const double v_)
{
  convert(y(), u(), v_);
}

double Magick::ColorYUV::v(void) const
{
  return scaleQuantumToDouble((0.61500 * quantumRed()) - (0.51500 *
    quantumGreen()) - (0.10000 * quantumBlue()));
}

void Magick::ColorYUV::y(const double y_)
{
  convert(y_, u(), v());
}

double Magick::ColorYUV::y ( void ) const
{
  return scaleQuantumToDouble((0.29900 * quantumRed()) + (0.58700 *
    quantumGreen()) + (0.11400 * quantumBlue()));
}

void Magick::ColorYUV::convert(const double y_,const double u_,const double v_)
{
  quantumRed(scaleDoubleToQuantum(y_ + 1.13980 * v_));
  quantumGreen(scaleDoubleToQuantum(y_ - (0.39380 * u_) - (0.58050 * v_)));
  quantumBlue(scaleDoubleToQuantum(y_ + 2.02790 * u_));
}

Magick::ColorYUV::ColorYUV(PixelInfo *rep_,PixelType pixelType_)
  : Color(rep_,pixelType_)
{
}
// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
// Copyright Dirk Lemstra 2013-2018
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
#if defined(MAGICKCORE_HDRI_SUPPORT)
  return((left_.isValid() == right_.isValid()) &&
    (fabs(left_.quantumRed()-right_.quantumRed()) < MagickEpsilon) &&
    (fabs(left_.quantumGreen()-right_.quantumGreen()) < MagickEpsilon) &&
    (fabs(left_.quantumBlue()-right_.quantumBlue()) < MagickEpsilon));
#else
  return((left_.isValid() == right_.isValid()) &&
    (left_.quantumRed() == right_.quantumRed()) &&
    (left_.quantumGreen() == right_.quantumGreen()) &&
    (left_.quantumBlue() == right_.quantumBlue()));
#endif
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
  : _pixel(new PixelInfo),
    _isValid(false),
    _pixelOwn(true),
    _pixelType(RGBAPixel)
{
  initPixel();

  setAlpha(TransparentAlpha);
}

Magick::Color::Color(const Magick::Quantum red_,const Magick::Quantum green_,
  const Quantum blue_)
  : _pixel(new PixelInfo),
    _isValid(true),
    _pixelOwn(true),
    _pixelType(RGBPixel)
{
  initPixel();

  quantumAlpha(OpaqueAlpha);
  quantumBlack(0);
  quantumBlue(blue_);
  quantumGreen(green_);
  quantumRed(red_);
}

Magick::Color::Color(const Magick::Quantum red_,const Magick::Quantum green_,
  const Magick::Quantum blue_, const Magick::Quantum alpha_)
  : _pixel(new PixelInfo),
    _isValid(true),
    _pixelOwn(true),
    _pixelType(RGBPixel)
{
  initPixel();

  quantumAlpha(alpha_);
  quantumBlack(0);
  quantumBlue(blue_);
  quantumGreen(green_);
  quantumRed(red_);
  if (alpha_ != OpaqueAlpha)
    _pixelType=RGBAPixel;
}

Magick::Color::Color(const Magick::Quantum cyan_,const Magick::Quantum magenta_,
  const Magick::Quantum yellow_,const Magick::Quantum black_,
  const Magick::Quantum alpha_)
  : _pixel(new PixelInfo),
    _isValid(true),
    _pixelOwn(true),
    _pixelType(CMYKPixel)
{
  initPixel();

  quantumAlpha(alpha_);
  quantumBlack(black_);
  quantumBlue(yellow_);
  quantumGreen(magenta_);
  quantumRed(cyan_);
  if (alpha_ != OpaqueAlpha)
    _pixelType=CMYKAPixel;
}

Magick::Color::Color(const char *color_)
  : _pixel(new PixelInfo),
    _isValid(true),
    _pixelOwn(true),
    _pixelType(RGBPixel)
{
  initPixel();

  // Use operator = implementation
  *this=color_;
}

Magick::Color::Color(const Magick::Color &color_)
  : _pixel(new PixelInfo),
    _isValid(color_._isValid),
    _pixelOwn(true),
    _pixelType(color_._pixelType)
{
  *_pixel=*color_._pixel;
}

Magick::Color::Color(const PixelInfo &color_)
  : _pixel(new PixelInfo),
    _isValid(true),
    _pixelOwn(true)
{
  *_pixel=color_;
  setPixelType(color_);
}

Magick::Color::Color(const std::string &color_)
  : _pixel(new PixelInfo),
    _isValid(true),
    _pixelOwn(true),
    _pixelType(RGBPixel)
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
  setPixelType(color_);

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
      quantumAlpha(target_color.alpha);
      quantumBlack(target_color.black);
      quantumBlue(target_color.blue);
      quantumGreen(target_color.green);
      quantumRed(target_color.red);

      setPixelType(target_color);
    }
  else
    _isValid = false;
  ThrowPPException(false);

  return(*this);
}

Magick::Color::operator MagickCore::PixelInfo() const
{
  return *_pixel;
}

Magick::Color::operator std::string() const
{
  char
    colorbuf[MagickPathExtent];

  PixelInfo
    pixel;

  if (!isValid())
    return std::string("none");

  pixel.colorspace=(_pixelType == RGBPixel || _pixelType == RGBAPixel) ?
    RGBColorspace : CMYKColorspace;
  pixel.alpha_trait=(_pixelType == RGBAPixel || _pixelType == CMYKAPixel) ?
    BlendPixelTrait : UndefinedPixelTrait;
  pixel.depth=MAGICKCORE_QUANTUM_DEPTH;
  pixel.alpha=_pixel->alpha;
  pixel.alpha_trait=_pixel->alpha_trait;
  pixel.black=_pixel->black;
  pixel.blue=_pixel->blue;
  pixel.green=_pixel->green;
  pixel.red=_pixel->red;
  GetColorTuple(&pixel,MagickTrue,colorbuf);

  return(std::string(colorbuf));
}

bool Magick::Color::isFuzzyEquivalent(const Color &color_, const double fuzz_) const
{
  PixelInfo
    p,
    q;

  p=*_pixel;
  p.fuzz=fuzz_;
  q=*color_._pixel;
  q.fuzz=fuzz_;
  return (IsFuzzyEquivalencePixelInfo(&p, &q) != MagickFalse);
}

bool Magick::Color::isValid(void) const
{
  return(_isValid);
}

Magick::Color::PixelType Magick::Color::pixelType() const
{
  return(_pixelType);
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
  setAlpha(alpha_);
  _isValid=true;
}

Magick::Quantum Magick::Color::quantumAlpha(void) const
{
  return(_pixel->alpha);
}

void Magick::Color::quantumBlack(const Magick::Quantum black_)
{
  _pixel->black=black_;
  _isValid=true;
}

Magick::Quantum Magick::Color::quantumBlack(void) const
{
  return(_pixel->black);
}

void Magick::Color::quantumBlue(const Magick::Quantum blue_)
{
  _pixel->blue=blue_;
  _isValid=true;
}

Magick::Quantum Magick::Color::quantumBlue(void) const
{
  return(_pixel->blue);
}

void Magick::Color::quantumGreen(const Magick::Quantum green_)
{
  _pixel->green=green_;
  _isValid=true;
}

Magick::Quantum Magick::Color::quantumGreen(void) const
{
  return(_pixel->green);
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

Magick::Color::Color(PixelType pixelType_)
  : _pixel(new PixelInfo),
    _isValid(false),
    _pixelOwn(true),
    _pixelType(pixelType_)
{
  initPixel();
}

Magick::Color::Color(PixelInfo* rep_,PixelType pixelType_)
  : _pixel(rep_),
    _isValid(true),
    _pixelOwn(false),
    _pixelType(pixelType_)
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
  return(static_cast<Magick::Quantum>(double_*QuantumRange));
}

double Magick::Color::scaleQuantumToDouble(const Magick::Quantum quantum_)
{
#if (MAGICKCORE_QUANTUM_DEPTH < 32) && (MAGICKCORE_SIZEOF_FLOAT_T != MAGICKCORE_SIZEOF_DOUBLE || !defined(MAGICKCORE_HDRI_SUPPORT))
  return(static_cast<double>(quantum_)/QuantumRange);
#else
  return(quantum_/QuantumRange);
#endif
}

void Magick::Color::initPixel()
{
  MagickCore::GetPixelInfo((MagickCore::Image *) NULL, _pixel);
  if (_pixelType == CMYKPixel || _pixelType == CMYKAPixel)
    _pixel->colorspace=CMYKColorspace;
}

void Magick::Color::setAlpha(const Magick::Quantum alpha_)
{
  _pixel->alpha=alpha_;
  if (alpha_ == QuantumRange)
    {
      _pixel->alpha_trait=UndefinedPixelTrait;
      if (_pixelType == RGBAPixel)
        _pixelType=RGBPixel;
      else if (_pixelType == CMYKAPixel)
        _pixelType=CMYKPixel;
    }
  else
    {
      _pixel->alpha_trait=BlendPixelTrait;
      if (_pixelType == RGBPixel)
        _pixelType=RGBAPixel;
      else if (_pixelType == CMYKPixel)
        _pixelType=CMYKAPixel;
    }
}

void Magick::Color::setPixelType(const PixelInfo &color_)
{
  if (color_.colorspace == CMYKColorspace)
    _pixelType=color_.alpha_trait != UndefinedPixelTrait ? CMYKAPixel :
      CMYKPixel;
  else
    _pixelType=color_.alpha_trait != UndefinedPixelTrait ? RGBAPixel :
      RGBPixel;
}

Magick::ColorCMYK::ColorCMYK(void)
  : Color(CMYKPixel)
{
}

Magick::ColorCMYK::ColorCMYK(const Magick::Color &color_)
  : Color(color_)
{
}

Magick::ColorCMYK::ColorCMYK(const double cyan_,const double magenta_,
  const double yellow_,const double black_)
  : Color(CMYKPixel)
{
  cyan(cyan_);
  magenta(magenta_);
  yellow(yellow_);
  black(black_);
}

Magick::ColorCMYK::ColorCMYK(const double cyan_,const double magenta_,
  const double yellow_,const double black_,const double alpha_)
  : Color(CMYKAPixel)
{
  cyan(cyan_);
  magenta(magenta_);
  yellow(yellow_);
  black(black_);
  alpha(alpha_);
}

Magick::ColorCMYK::~ColorCMYK(void)
{
}

Magick::ColorCMYK& Magick::ColorCMYK::operator=(const Magick::Color& color_)
{
  *static_cast<Magick::Color*>(this)=color_;
  return(*this);
}

void Magick::ColorCMYK::alpha(const double alpha_)
{
  quantumAlpha(scaleDoubleToQuantum(alpha_));
}

double Magick::ColorCMYK::alpha(void) const
{
  return(scaleQuantumToDouble(quantumAlpha()));
}

void Magick::ColorCMYK::black(const double black_)
{
  quantumBlack(scaleDoubleToQuantum(black_));
}

double Magick::ColorCMYK::black(void) const
{
  return(scaleQuantumToDouble(quantumBlack()));
}

void Magick::ColorCMYK::cyan(const double cyan_)
{
  quantumRed(scaleDoubleToQuantum(cyan_));
}

double Magick::ColorCMYK::cyan(void) const
{
  return(scaleQuantumToDouble(quantumRed()));
}

void Magick::ColorCMYK::magenta(const double magenta_)
{
  quantumGreen(scaleDoubleToQuantum(magenta_));
}

double Magick::ColorCMYK::magenta(void) const
{
  return(scaleQuantumToDouble(quantumGreen()));
}

void Magick::ColorCMYK::yellow(const double yellow_)
{
  quantumBlue(scaleDoubleToQuantum(yellow_));
}

double Magick::ColorCMYK::yellow(void) const
{
  return(scaleQuantumToDouble(quantumBlue()));
}

Magick::ColorCMYK::ColorCMYK(PixelInfo *rep_,PixelType pixelType_)
  : Color(rep_,pixelType_)
{
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
  const double lightness_)
  : Color()
{
  double
    blue,
    green,
    red;

  ConvertHSLToRGB(hue_,saturation_,lightness_,&red,&green,&blue);

  quantumRed(red);
  quantumGreen(green);
  quantumBlue(blue);
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
    lightness,
    saturation;

  double
    blue,
    green,
    red;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &lightness);

  hue=hue_;

  ConvertHSLToRGB(hue,saturation,lightness,&red,&green,&blue);

  quantumRed(ClampToQuantum(red));
  quantumGreen(ClampToQuantum(green));
  quantumBlue(ClampToQuantum(blue));
}

double Magick::ColorHSL::hue(void) const
{
  double
    hue,
    lightness,
    saturation;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &lightness);

  return(hue);
}

void Magick::ColorHSL::lightness (const double lightness_)
{
  double
    hue,
    lightness,
    saturation;

  double
    blue,
    green,
    red;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &lightness);

  lightness=lightness_;

  ConvertHSLToRGB(hue,saturation,lightness,&red,&green,&blue);

  quantumRed(ClampToQuantum(red));
  quantumGreen(ClampToQuantum(green));
  quantumBlue(ClampToQuantum(blue));
}

double Magick::ColorHSL::lightness (void) const
{
  double
    hue,
    lightness,
    saturation;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &lightness);

  return(lightness);
}

void Magick::ColorHSL::saturation(const double saturation_)
{
  double
    hue,
    lightness,
    saturation;

  double
    blue,
    green,
    red;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &lightness);

  saturation=saturation_;

  ConvertHSLToRGB(hue,saturation,lightness,&red,&green,&blue);

  quantumRed(ClampToQuantum(red));
  quantumGreen(ClampToQuantum(green));
  quantumBlue(ClampToQuantum(blue));
}

double Magick::ColorHSL::saturation(void) const
{
  double
    hue,
    lightness,
    saturation;

  ConvertRGBToHSL(quantumRed(),quantumGreen(),quantumBlue(),&hue,&saturation,
    &lightness);

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

void Magick::ColorRGB::alpha(const double alpha_)
{
  quantumAlpha(scaleDoubleToQuantum(alpha_));
}

double Magick::ColorRGB::alpha(void) const
{
  return(scaleQuantumToDouble(quantumAlpha()));
}

void Magick::ColorRGB::blue(const double blue_)
{
  quantumBlue(scaleDoubleToQuantum(blue_));
}

double Magick::ColorRGB::blue(void) const
{
  return(scaleQuantumToDouble(quantumBlue()));
}

void Magick::ColorRGB::green(const double green_)
{
  quantumGreen(scaleDoubleToQuantum(green_));
}

double Magick::ColorRGB::green(void) const
{
  return(scaleQuantumToDouble(quantumGreen()));
}

void Magick::ColorRGB::red(const double red_)
{
  quantumRed(scaleDoubleToQuantum(red_));
}

double Magick::ColorRGB::red(void) const
{
  return(scaleQuantumToDouble(quantumRed()));
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
  return(scaleQuantumToDouble((-0.14740 * quantumRed()) - (0.28950 *
    quantumGreen()) + (0.43690 * quantumBlue())));
}

void Magick::ColorYUV::v(const double v_)
{
  convert(y(), u(), v_);
}

double Magick::ColorYUV::v(void) const
{
  return(scaleQuantumToDouble((0.61500 * quantumRed()) - (0.51500 *
    quantumGreen()) - (0.10000 * quantumBlue())));
}

void Magick::ColorYUV::y(const double y_)
{
  convert(y_, u(), v());
}

double Magick::ColorYUV::y ( void ) const
{
  return(scaleQuantumToDouble((0.29900 * quantumRed()) + (0.58700 *
    quantumGreen()) + (0.11400 * quantumBlue())));
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

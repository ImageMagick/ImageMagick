// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003, 2008
//
// Color Implementation
//
#if !defined (Magick_Color_header)
#define Magick_Color_header

#include "Magick++/Include.h"
#include <string>

namespace Magick
{
  class MagickPPExport Color;

  // Compare two Color objects regardless of LHS/RHS
  MagickPPExport int operator ==
    (const Magick::Color &left_,const Magick::Color &right_);
  MagickPPExport int operator !=
    (const Magick::Color &left_,const Magick::Color &right_);
  MagickPPExport int operator >
    (const Magick::Color &left_,const Magick::Color &right_);
  MagickPPExport int operator <
    (const Magick::Color &left_,const Magick::Color &right_);
  MagickPPExport int operator >=
    (const Magick::Color &left_,const Magick::Color &right_);
  MagickPPExport int operator <=
    (const Magick::Color &left_,const Magick::Color &right_);

  // Base color class stores RGB components scaled to fit Quantum
  class MagickPPExport Color
  {
  public:

    // Default constructor
    Color(void);

    // Construct Color using the specified RGB values
    Color(Quantum red_,Quantum green_,Quantum blue_);

    // Construct Color using the specified RGBA values
    Color(Quantum red_,Quantum green_,Quantum blue_,Quantum alpha_);

    // Construct Color using the specified color string
    Color(const char *x11color_);

    // Copy constructor
    Color(const Color &color_);

    // Construct color via ImageMagick PixelPacket
    Color(const PixelPacket &color_);

    // Constructor Color using the specified color string
    Color(const std::string &x11color_);

    // Destructor
    virtual ~Color(void);

    // Assignment operator
    Color& operator=(const Color& color_);

    // Set color via X11 color specification string
    const Color& operator=(const char *x11color);
    
    // Set color via X11 color specification string
    const Color& operator=(const std::string &x11color_);

    // Set color via ImageMagick PixelPacket
    const Color& operator=(const PixelPacket &color_);

    // Return ImageMagick PixelPacket
    operator PixelPacket() const;

    // Return X11 color specification string
    operator std::string() const;

    // Scaled (to 1.0) version of alpha for use in sub-classes
    // (range opaque=0 to transparent=1.0)
    void alpha(double alpha_);
    double alpha(void) const;

    // Alpha level (range OpaqueOpacity=0 to TransparentOpacity=QuantumRange)
    void alphaQuantum(Quantum alpha_);
    Quantum alphaQuantum(void) const;

    // Blue color (range 0 to QuantumRange)
    void blueQuantum(Quantum blue_);
    Quantum blueQuantum (void) const;

    // Green color (range 0 to QuantumRange)
    void greenQuantum(Quantum green_);
    Quantum greenQuantum(void) const;
        
    // Does object contain valid color?
    void isValid(bool valid_);
    bool isValid(void) const;

    // Red color (range 0 to QuantumRange)
    void redQuantum(Quantum red_);
    Quantum redQuantum (void) const;

    //
    // Public methods beyond this point are for Magick++ use only.
    //

    // Obtain pixel intensity as a double
    double intensity(void) const
    {
      return (0.299*(_pixel->red)+0.587*(_pixel->green)+0.114*(_pixel->blue));
    }

    // Scale a value expressed as a double (0-1) to Quantum range (0-QuantumRange)
    static Quantum scaleDoubleToQuantum(const double double_)
    {
      return (static_cast<Magick::Quantum>(double_*QuantumRange));
    }

    // Scale a value expressed as a Quantum (0-QuantumRange) to double range (0-1)
#if (MAGICKCORE_QUANTUM_DEPTH < 64)
    static double scaleQuantumToDouble(const Quantum quantum_)
    {
      return (static_cast<double>(quantum_)/QuantumRange);
    }
#endif
    static double scaleQuantumToDouble(const double quantum_)
    {
      return (quantum_/QuantumRange);
    }

  protected:

    // PixelType specifies the interpretation of PixelPacket members
    // RGBPixel:
    //   Red      = red;
    //   Green    = green;
    //   Blue     = blue;
    // RGBAPixel:
    //   Red      = red;
    //   Green    = green;
    //   Blue     = blue;
    //   Alpha    = opacity;
    // CYMKPixel:
    //   Cyan     = red
    //   Yellow   = green
    //   Magenta  = blue
    //   Black(K) = opacity
    enum PixelType
    {
      RGBPixel,
      RGBAPixel,
      CYMKPixel
    };

    // Constructor to construct with PixelPacket*
    // Used to point Color at a pixel in an image
    Color(PixelPacket *rep_,PixelType pixelType_);

    // Set pixel
    // Used to point Color at a pixel in an image
    void pixel(PixelPacket *rep_,PixelType pixelType_);

    // PixelPacket represents a color pixel:
    //  red     = red   (range 0 to QuantumRange)
    //  green   = green (range 0 to QuantumRange)
    //  blue    = blue  (range 0 to QuantumRange)
    //  opacity = alpha (range OpaqueOpacity=0 to TransparentOpacity=QuantumRange)
    //  index   = PseudoColor colormap index
    PixelPacket *_pixel;

  private:

    bool _isValid; // Set true if pixel is "valid"
    bool _pixelOwn; // Set true if we allocated pixel
    PixelType _pixelType; // Color type supported by _pixel

    // Common initializer for PixelPacket representation
    void initPixel();
  };

  //
  // Grayscale RGB color
  //
  // Grayscale is simply RGB with equal parts of red, green, and blue
  // All double arguments have a valid range of 0.0 - 1.0.
  class MagickPPExport ColorGray : public Color
  {
  public:

    // Default constructor
    ColorGray(void);

    // Copy constructor
    ColorGray(const Color & color_);

    // Construct ColorGray using the specified shade
    ColorGray(double shade_);

    // Destructor
    ~ColorGray();

    void shade(double shade_);
    double shade(void) const;

    // Assignment operator from base class
    ColorGray& operator=(const Color& color_);

  protected:

    // Constructor to construct with PixelPacket*
    ColorGray(PixelPacket *rep_,PixelType pixelType_);
  };

  //
  // HSL Colorspace colors
  //
  class MagickPPExport ColorHSL: public Color
  {
  public:

    // Default constructor
    ColorHSL(void);

    // Copy constructor
    ColorHSL(const Color &color_);

    // Construct ColorHSL using the specified HSL values
    ColorHSL(double hue_,double saturation_,double luminosity_);

    // Destructor
    ~ColorHSL();

    // Assignment operator from base class
    ColorHSL& operator=(const Color& color_);

    // Hue color
    void hue(double hue_);
    double hue(void) const;

    // Luminosity color
    void luminosity(double luminosity_);
    double luminosity(void) const;

    // Saturation color
    void saturation(double saturation_);
    double saturation(void) const;

  protected:

    // Constructor to construct with PixelPacket*
    ColorHSL(PixelPacket *rep_,PixelType pixelType_);
  };

  //
  // Monochrome color
  //
  // Color arguments are constrained to 'false' (black pixel) and 'true'
  // (white pixel)
  class MagickPPExport ColorMono : public Color
  {
  public:

    // Default constructor
    ColorMono(void);

    // Construct ColorMono (false=black, true=white)
    ColorMono(bool mono_);

    // Copy constructor
    ColorMono(const Color & color_);

    // Destructor
    ~ColorMono();

    // Assignment operator from base class
    ColorMono& operator=(const Color& color_);

    // Mono color
    void mono(bool mono_);
    bool mono(void) const;

  protected:
    // Constructor to construct with PixelPacket*
    ColorMono(PixelPacket *rep_,PixelType pixelType_);
  };

  //
  // RGB color
  //
  // All color arguments have a valid range of 0.0 - 1.0.
  class MagickPPExport ColorRGB: public Color
  {
  public:

    // Default constructor
    ColorRGB(void);

    // Copy constructor
    ColorRGB(const Color &color_);

    // Construct ColorRGB using the specified RGB values
    ColorRGB(double red_,double green_,double blue_);

    // Destructor
    ~ColorRGB(void);

    // Assignment operator from base class
    ColorRGB& operator=(const Color& color_);

    // Blue color
    void blue(double blue_);
    double blue(void) const;

    // Green color
    void green(double green_);
    double green(void) const;

    // Red color
    void red(double red_);
    double red(void) const;

  protected:

    // Constructor to construct with PixelPacket*
    ColorRGB(PixelPacket *rep_,PixelType pixelType_);
  };

  //
  // YUV Colorspace color
  //
  // Argument ranges:
  //        Y:  0.0 through 1.0
  //        U: -0.5 through 0.5
  //        V: -0.5 through 0.5
  class MagickPPExport ColorYUV: public Color
  {
  public:

    // Default constructor
    ColorYUV(void);

    // Copy constructor
    ColorYUV(const Color &color_);

    // Construct ColorYUV using the specified YUV values
    ColorYUV(double y_,double u_,double v_);

    // Destructor
    ~ColorYUV(void);

    // Assignment operator from base class
    ColorYUV& operator=(const Color& color_);

    // Color U (0.0 through 1.0)
    void u(double u_);
    double u(void) const;

    // Color V (-0.5 through 0.5)
    void v(double v_);
    double v(void) const;

    // Color Y (-0.5 through 0.5)
    void y(double y_);
    double y(void) const;

  protected:

    // Constructor to construct with PixelInfo*
    ColorYUV(PixelPacket *rep_,PixelType pixelType_);
  };
} // namespace Magick

//
// Inlines
//

//
// Color
//

inline void Magick::Color::alpha(double alpha_)
{
  alphaQuantum(scaleDoubleToQuantum(alpha_));
}
inline double Magick::Color::alpha(void) const
{
  return scaleQuantumToDouble(alphaQuantum());
}

inline void Magick::Color::alphaQuantum(Magick::Quantum alpha_)
{
  _pixel->opacity=alpha_;
  _isValid=true ;
}

inline Magick::Quantum Magick::Color::alphaQuantum(void) const
{
  return _pixel->opacity;
}

inline void Magick::Color::blueQuantum(Magick::Quantum blue_)
{
  _pixel->blue=blue_;
  _isValid=true;
}

inline Magick::Quantum Magick::Color::blueQuantum(void) const
{
  return _pixel->blue;
}

inline void Magick::Color::greenQuantum(Magick::Quantum green_)
{
  _pixel->green=green_;
  _isValid=true;
}

inline Magick::Quantum Magick::Color::greenQuantum(void) const
{
  return _pixel->green;
}

inline void Magick::Color::redQuantum(Magick::Quantum red_)
{
  _pixel->red=red_;
  _isValid=true;
}

inline Magick::Quantum Magick::Color::redQuantum(void) const
{
  return _pixel->red;
}

inline void Magick::Color::initPixel()
{
  _pixel->red=0;
  _pixel->green=0;
  _pixel->blue=0;
  _pixel->opacity=TransparentOpacity;
}

inline Magick::Color::operator MagickCore::PixelPacket() const
{
  return *_pixel;
}

//
// ColorGray
//
inline Magick::ColorGray::ColorGray(Magick::PixelPacket *rep_,
  Magick::Color::PixelType pixelType_)
: Color(rep_,pixelType_)
{
}

//
// ColorHSL
//
inline Magick::ColorHSL::ColorHSL(Magick::PixelPacket *rep_,
  Magick::Color::PixelType pixelType_)
: Color(rep_,pixelType_)
{
}

//
// ColorMono
//
inline Magick::ColorMono::ColorMono(Magick::PixelPacket *rep_,
  Magick::Color::PixelType pixelType_)
  : Color(rep_,pixelType_)
{
}

//
// ColorRGB
//
inline Magick::ColorRGB::ColorRGB(Magick::PixelPacket *rep_,
  Magick::Color::PixelType pixelType_)
  : Color(rep_,pixelType_)
{
}

inline void Magick::ColorRGB::blue(double blue_)
{
  blueQuantum(scaleDoubleToQuantum(blue_));
}

inline double Magick::ColorRGB::blue(void) const
{
  return scaleQuantumToDouble(blueQuantum());
}

inline void Magick::ColorRGB::green(double green_)
{
  greenQuantum(scaleDoubleToQuantum(green_));
}

inline double Magick::ColorRGB::green(void) const
{
  return scaleQuantumToDouble(greenQuantum());
}

inline void Magick::ColorRGB::red(double red_)
{
  redQuantum(scaleDoubleToQuantum(red_));
}

inline double Magick::ColorRGB::red(void) const
{
  return scaleQuantumToDouble(redQuantum());
}

//
// ColorYUV
//

inline Magick::ColorYUV::ColorYUV(Magick::PixelPacket *rep_,
  Magick::Color::PixelType pixelType_)
  : Color(rep_,pixelType_)
{
}

#endif // Magick_Color_header

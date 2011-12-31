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
  int MagickPPExport operator == ( const Magick::Color& left_, const Magick::Color& right_ );
  int MagickPPExport operator != ( const Magick::Color& left_, const Magick::Color& right_ );
  int MagickPPExport operator >  ( const Magick::Color& left_, const Magick::Color& right_ );
  int MagickPPExport operator <  ( const Magick::Color& left_, const Magick::Color& right_ );
  int MagickPPExport operator >= ( const Magick::Color& left_, const Magick::Color& right_ );
  int MagickPPExport operator <= ( const Magick::Color& left_, const Magick::Color& right_ );

  // Base color class stores RGB components scaled to fit Quantum
  class MagickPPExport Color
  {
  public:
    Color ( Quantum red_,
	    Quantum green_,
	    Quantum blue_ );
    Color ( Quantum red_,
	    Quantum green_,
	    Quantum blue_,
	    Quantum alpha_ );
    Color ( const std::string &x11color_ );
    Color ( const char * x11color_ );
    Color ( void );
    virtual        ~Color ( void );
    Color ( const Color & color_ );

    // Red color (range 0 to QuantumRange)
    void           redQuantum ( Quantum red_ );
    Quantum        redQuantum ( void ) const;

    // Green color (range 0 to QuantumRange)
    void           greenQuantum ( Quantum green_ );
    Quantum        greenQuantum ( void ) const;

    // Blue color (range 0 to QuantumRange)
    void           blueQuantum ( Quantum blue_ );
    Quantum        blueQuantum ( void ) const;

    // Alpha level (range OpaqueOpacity=0 to TransparentOpacity=QuantumRange)
    void           alphaQuantum ( Quantum alpha_ );
    Quantum        alphaQuantum ( void ) const;

    // Scaled (to 1.0) version of alpha for use in sub-classes
    // (range opaque=0 to transparent=1.0)
    void           alpha ( double alpha_ );
    double         alpha ( void ) const;
        
    // Does object contain valid color?
    void           isValid ( bool valid_ );
    bool           isValid ( void ) const;
    
    // Set color via X11 color specification string
    const Color& operator= ( const std::string &x11color_ );
    const Color& operator= ( const char * x11color_ );

    // Assignment operator
    Color& operator= ( const Color& color_ );

    // Return X11 color specification string
    /* virtual */ operator std::string() const;

    // Return ImageMagick PixelPacket
    operator PixelPacket() const;

    // Construct color via ImageMagick PixelPacket
    Color ( const PixelPacket &color_ );

    // Set color via ImageMagick PixelPacket
    const Color& operator= ( const PixelPacket &color_ );

    //
    // Public methods beyond this point are for Magick++ use only.
    //

    // Obtain pixel intensity as a double
    double intensity ( void ) const
      {
        return (0.299*(_pixel->red)+0.587*(_pixel->green)+0.114*(_pixel->blue));
      }

    // Scale a value expressed as a double (0-1) to Quantum range (0-QuantumRange)
    static Quantum scaleDoubleToQuantum( const double double_ )
      {
        return (static_cast<Magick::Quantum>(double_*QuantumRange));
      }

    // Scale a value expressed as a Quantum (0-QuantumRange) to double range (0-1)
#if (MAGICKCORE_QUANTUM_DEPTH < 64)
    static double scaleQuantumToDouble( const Quantum quantum_ )
      {
        return (static_cast<double>(quantum_)/QuantumRange);
      }
#endif
    static double scaleQuantumToDouble( const double quantum_ )
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
    Color ( PixelPacket* rep_, PixelType pixelType_ );

    // Set pixel
    // Used to point Color at a pixel in an image
    void pixel ( PixelPacket* rep_, PixelType pixelType_ );

    // PixelPacket represents a color pixel:
    //  red     = red   (range 0 to QuantumRange)
    //  green   = green (range 0 to QuantumRange)
    //  blue    = blue  (range 0 to QuantumRange)
    //  opacity = alpha (range OpaqueOpacity=0 to TransparentOpacity=QuantumRange)
    //  index   = PseudoColor colormap index
    PixelPacket*     _pixel;

  private:

    // Common initializer for PixelPacket representation
    void initPixel();

    // Set true if we allocated pixel
    bool                        _pixelOwn;

    // Set true if pixel is "valid"
    bool                       _isValid;

    // Color type supported by _pixel
    PixelType			_pixelType;

  };

  //
  // HSL Colorspace colors
  //
  class MagickPPExport ColorHSL : public Color
  {
  public:
    ColorHSL ( double hue_, double saturation_, double luminosity_ );
    ColorHSL ( void );
    ColorHSL ( const Color & color_ );
    /* virtual */  ~ColorHSL ( );
    
    void           hue ( double hue_ );
    double         hue ( void ) const;
    
    void           saturation ( double saturation_ );
    double         saturation ( void ) const;
    
    void           luminosity ( double luminosity_ );
    double         luminosity ( void ) const;

    // Assignment operator from base class
    ColorHSL& operator= ( const Color& color_ );

  protected:
    // Constructor to construct with PixelPacket*
    ColorHSL ( PixelPacket* rep_, PixelType pixelType_ );
  };
  
  //
  // Grayscale RGB color
  //
  // Grayscale is simply RGB with equal parts of red, green, and blue
  // All double arguments have a valid range of 0.0 - 1.0.
  class MagickPPExport ColorGray : public Color
  {
  public:
    ColorGray ( double shade_ );
    ColorGray ( void );
    ColorGray ( const Color & color_ );
    /* virtual */ ~ColorGray ();

    void           shade ( double shade_ );
    double         shade ( void ) const;

    // Assignment operator from base class
    ColorGray& operator= ( const Color& color_ );

  protected:
    // Constructor to construct with PixelPacket*
    ColorGray ( PixelPacket* rep_, PixelType pixelType_ );
  };
  
  //
  // Monochrome color
  //
  // Color arguments are constrained to 'false' (black pixel) and 'true'
  // (white pixel)
  class MagickPPExport ColorMono : public Color
  {
  public:
    ColorMono ( bool mono_ );
    ColorMono ( void );
    ColorMono ( const Color & color_ );
    /* virtual */ ~ColorMono ();
    
    void           mono ( bool mono_ );
    bool           mono ( void ) const;

    // Assignment operator from base class
    ColorMono& operator= ( const Color& color_ );

  protected:
    // Constructor to construct with PixelPacket*
    ColorMono ( PixelPacket* rep_, PixelType pixelType_ );
  };
  
  //
  // RGB color
  //
  // All color arguments have a valid range of 0.0 - 1.0.
  class MagickPPExport ColorRGB : public Color
  {
  public:
    ColorRGB ( double red_, double green_, double blue_ );
    ColorRGB ( void );
    ColorRGB ( const Color & color_ );
    /* virtual */  ~ColorRGB ( void );
    
    void           red ( double red_ );
    double         red ( void ) const;
    
    void           green ( double green_ );
    double         green ( void ) const;
    
    void           blue ( double blue_ );
    double         blue ( void ) const;

    // Assignment operator from base class
    ColorRGB& operator= ( const Color& color_ );

  protected:
    // Constructor to construct with PixelPacket*
    ColorRGB ( PixelPacket* rep_, PixelType pixelType_ );
  };
  
  //
  // YUV Colorspace color
  //
  // Argument ranges:
  //        Y:  0.0 through 1.0
  //        U: -0.5 through 0.5
  //        V: -0.5 through 0.5
  class MagickPPExport ColorYUV : public Color
  {
  public:
    ColorYUV ( double y_, double u_, double v_ );
    ColorYUV ( void );
    ColorYUV ( const Color & color_ );
    /* virtual */ ~ColorYUV ( void );
    
    void           u ( double u_ );
    double         u ( void ) const;
    
    void           v ( double v_ );
    double         v ( void ) const;
    
    void           y ( double y_ );
    double         y ( void ) const;

    // Assignment operator from base class
    ColorYUV& operator= ( const Color& color_ );

  protected:
    // Constructor to construct with PixelPacket*
    ColorYUV ( PixelPacket* rep_, PixelType pixelType_ );
  };
} // namespace Magick

//
// Inlines
//

//
// Color
//

// Common initializer for PixelPacket representation
// Initialized transparent black
inline void Magick::Color::initPixel()
{
  _pixel->red     = 0;
  _pixel->green   = 0;
  _pixel->blue    = 0;
  _pixel->opacity = TransparentOpacity;
}

inline void Magick::Color::redQuantum ( Magick::Quantum red_ )
{
  _pixel->red = red_;
  _isValid = true;
}

inline Magick::Quantum Magick::Color::redQuantum ( void ) const
{
  return _pixel->red;
}

inline void Magick::Color::greenQuantum ( Magick::Quantum green_ )
{
  _pixel->green = green_;
  _isValid = true;
}

inline Magick::Quantum  Magick::Color::greenQuantum ( void ) const
{
  return _pixel->green;
}

inline void  Magick::Color::blueQuantum ( Magick::Quantum blue_ )
{
  _pixel->blue = blue_;
  _isValid = true;
}

inline Magick::Quantum Magick::Color::blueQuantum ( void ) const
{
  return _pixel->blue;
}

inline void  Magick::Color::alphaQuantum ( Magick::Quantum alpha_ )
{
  _pixel->opacity = alpha_;
  _isValid = true ;
}

inline Magick::Quantum Magick::Color::alphaQuantum ( void ) const
{
  return _pixel->opacity;
}

// Return ImageMagick PixelPacket struct based on color.
inline Magick::Color::operator MagickCore::PixelPacket () const
{
  return *_pixel;
}

// Scaled version of alpha for use in sub-classes
inline void  Magick::Color::alpha ( double alpha_ )
{
  alphaQuantum( scaleDoubleToQuantum(alpha_) );
}
inline double Magick::Color::alpha ( void ) const
{
  return scaleQuantumToDouble( alphaQuantum() );
}

//
// ColorHSL
//
inline Magick::ColorHSL::ColorHSL ( Magick::PixelPacket* rep_,
                                    Magick::Color::PixelType pixelType_ )
: Color( rep_, pixelType_ )
{
}

//
// ColorGray
//
inline Magick::ColorGray::ColorGray ( Magick::PixelPacket* rep_,
                                      Magick::Color::PixelType pixelType_ )
: Color( rep_, pixelType_ )
{
}

//
// ColorMono
//
inline Magick::ColorMono::ColorMono ( Magick::PixelPacket* rep_,
                                      Magick::Color::PixelType pixelType_ )
  : Color( rep_, pixelType_ )
{
}

//
// ColorRGB
//
inline Magick::ColorRGB::ColorRGB ( Magick::PixelPacket* rep_,
                                    Magick::Color::PixelType pixelType_ )
  : Color( rep_, pixelType_ )
{
}

inline void Magick::ColorRGB::red ( double red_ )
{
  redQuantum( scaleDoubleToQuantum(red_) );
}

inline double Magick::ColorRGB::red ( void ) const
{
  return scaleQuantumToDouble( redQuantum() );
}

inline void Magick::ColorRGB::green ( double green_ )
{
  greenQuantum( scaleDoubleToQuantum(green_) );
}

inline double Magick::ColorRGB::green ( void ) const
{
  return scaleQuantumToDouble( greenQuantum() );
}

inline void Magick::ColorRGB::blue ( double blue_ )
{
  blueQuantum( scaleDoubleToQuantum(blue_) );
}

inline double Magick::ColorRGB::blue ( void ) const
{
  return scaleQuantumToDouble( blueQuantum() );
}

//
// ColorYUV
//

inline Magick::ColorYUV::ColorYUV ( Magick::PixelPacket* rep_,
                                    Magick::Color::PixelType pixelType_ )
  : Color( rep_, pixelType_ )
{
}

#endif // Magick_Color_header

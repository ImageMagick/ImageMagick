// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
//
// Geometry Definition
//
// Representation of an ImageMagick geometry specification
// X11 geometry specification plus hints

#if !defined (Magick_Geometry_header)
#define Magick_Geometry_header

#include "Magick++/Include.h"
#include <string>

namespace Magick
{

  class MagickPPExport Geometry;

  // Compare two Geometry objects regardless of LHS/RHS
  int MagickPPExport operator == ( const Magick::Geometry& left_, const Magick::Geometry& right_ );
  int MagickPPExport operator != ( const Magick::Geometry& left_, const Magick::Geometry& right_ );
  int MagickPPExport operator >  ( const Magick::Geometry& left_, const Magick::Geometry& right_ );
  int MagickPPExport operator <  ( const Magick::Geometry& left_, const Magick::Geometry& right_ );
  int MagickPPExport operator >= ( const Magick::Geometry& left_, const Magick::Geometry& right_ );
  int MagickPPExport operator <= ( const Magick::Geometry& left_, const Magick::Geometry& right_ );

  class MagickPPExport Geometry
  {
  public:
    
    Geometry ( size_t width_,
	       size_t height_,
	       ::ssize_t xOff_ = 0,
	       ::ssize_t yOff_ = 0,
	       bool xNegative_ = false,
	       bool yNegative_ = false );
    Geometry ( const std::string &geometry_ );
    Geometry ( const char * geometry_ );
    Geometry ( const Geometry &geometry_ );
    Geometry ( );
    ~Geometry ( void );
    
    // Width
    void          width ( size_t width_ );
    size_t  width ( void ) const;
    
    // Height
    void          height ( size_t height_ );
    size_t  height ( void ) const;
    
    // X offset from origin
    void          xOff ( ::ssize_t xOff_ );
    ::ssize_t     xOff ( void ) const;
    
    // Y offset from origin
    void          yOff ( ::ssize_t yOff_ );
    ::ssize_t     yOff ( void ) const;
    
    // Sign of X offset negative? (X origin at right)
    void          xNegative ( bool xNegative_ );
    bool          xNegative ( void ) const;
    
    // Sign of Y offset negative? (Y origin at bottom)
    void          yNegative ( bool yNegative_ );
    bool          yNegative ( void ) const;
    
    // Width and height are expressed as percentages
    void          percent ( bool percent_ );
    bool          percent ( void ) const;

    // Resize without preserving aspect ratio (!)
    void          aspect ( bool aspect_ );
    bool          aspect ( void ) const;
    
    // Resize if image is greater than size (>)
    void          greater ( bool greater_ );
    bool          greater ( void ) const;
    
    // Resize if image is less than size (<)
    void          less ( bool less_ );
    bool          less ( void ) const;
    
    // Resize the image based on the smallest fitting dimension (^)
    void          fillArea ( bool fillArea_ );
    bool          fillArea ( void ) const;
    
    // Resize using a pixel area count limit (@)
    void          limitPixels ( bool limitPixels_ );
    bool          limitPixels ( void ) const;
    
    // Does object contain valid geometry?
    void          isValid ( bool isValid_ );
    bool          isValid ( void ) const;
    
    // Set via geometry string
    const Geometry& operator = ( const std::string &geometry_ );
    const Geometry& operator = ( const char * geometry_ );

    // Assignment operator
    Geometry& operator= ( const Geometry& Geometry_ );
    
    // Return geometry string
    operator std::string() const;
    
    //
    // Public methods below this point are for Magick++ use only.
    //

    // Construct from RectangleInfo
    Geometry ( const MagickCore::RectangleInfo &rectangle_ );

    // Return an ImageMagick RectangleInfo struct
    operator MagickCore::RectangleInfo() const;
    
  private:
    size_t    _width;
    size_t    _height;
    ::ssize_t _xOff;
    ::ssize_t _yOff;
    bool      _xNegative;
    bool      _yNegative;
    bool      _isValid;
    bool      _percent;     // Interpret width & height as percentages (%)
    bool      _aspect;      // Force exact size (!)
    bool      _greater;     // Resize only if larger than geometry (>)
    bool      _less;        // Resize only if smaller than geometry (<)
    bool      _fillArea;    // Resize the image based on the smallest fitting dimension (^)
    bool      _limitPixels; // Resize using a pixel area count limit (@)
  };
} // namespace Magick

//
// Inlines
//

// Does object contain valid geometry?
inline void Magick::Geometry::isValid ( bool isValid_ )
{
  _isValid = isValid_;
}

inline bool Magick::Geometry::isValid ( void ) const
{
  return _isValid;
}

// Width
inline void Magick::Geometry::width ( size_t width_ )
{
  _width = width_;
  isValid( true );
}
inline size_t Magick::Geometry::width ( void ) const
{
  return _width;
}

// Height
inline void Magick::Geometry::height ( size_t height_ )
{
  _height = height_;
}
inline size_t Magick::Geometry::height ( void ) const
{
  return _height;
}

// X offset from origin
inline void Magick::Geometry::xOff ( ::ssize_t xOff_ )
{
  _xOff = xOff_;
}
inline ::ssize_t Magick::Geometry::xOff ( void ) const
{
  return _xOff;
}

// Y offset from origin
inline void Magick::Geometry::yOff ( ::ssize_t yOff_ )
{
  _yOff = yOff_;
}
inline ::ssize_t Magick::Geometry::yOff ( void ) const
{
  return _yOff;
}

// Sign of X offset negative? (X origin at right)
inline void Magick::Geometry::xNegative ( bool xNegative_ )
{
  _xNegative = xNegative_;
}
inline bool Magick::Geometry::xNegative ( void ) const
{
  return _xNegative;
}

// Sign of Y offset negative? (Y origin at bottom)
inline void Magick::Geometry::yNegative ( bool yNegative_ )
{
  _yNegative = yNegative_;
}
inline bool Magick::Geometry::yNegative ( void ) const
{
  return _yNegative;
}

// Interpret width & height as percentages (%)
inline void Magick::Geometry::percent ( bool percent_ )
{
  _percent = percent_;
}
inline bool Magick::Geometry::percent ( void ) const
{
  return _percent;
}

// Resize without preserving aspect ratio (!)
inline void Magick::Geometry::aspect ( bool aspect_ )
{
  _aspect = aspect_;
}
inline bool Magick::Geometry::aspect ( void ) const
{
  return _aspect;
}

// Resize if image is greater than size (>)
inline void Magick::Geometry::greater ( bool greater_ )
{
  _greater = greater_;
}
inline bool Magick::Geometry::greater ( void ) const
{
  return _greater;
}

// Resize if image is less than size (<)
inline void Magick::Geometry::less ( bool less_ )
{
  _less = less_;
}
inline bool Magick::Geometry::less ( void ) const
{
  return _less;
}

// Resize the image based on the smallest fitting dimension (^)
inline void Magick::Geometry::fillArea ( bool fillArea_ )
{
  _fillArea = fillArea_;
}
inline bool Magick::Geometry::fillArea ( void ) const
{
  return _fillArea;
}

// Resize using a pixel area count limit (@)
inline void Magick::Geometry::limitPixels ( bool limitPixels_ )
{
  _limitPixels = limitPixels_;
}
inline bool Magick::Geometry::limitPixels ( void ) const
{
  return _limitPixels;
}

#endif // Magick_Geometry_header

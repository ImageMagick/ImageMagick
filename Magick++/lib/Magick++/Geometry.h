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

  class MagickDLLDecl Geometry;

  // Compare two Geometry objects regardless of LHS/RHS
  int MagickDLLDecl operator == ( const Magick::Geometry& left_, const Magick::Geometry& right_ );
  int MagickDLLDecl operator != ( const Magick::Geometry& left_, const Magick::Geometry& right_ );
  int MagickDLLDecl operator >  ( const Magick::Geometry& left_, const Magick::Geometry& right_ );
  int MagickDLLDecl operator <  ( const Magick::Geometry& left_, const Magick::Geometry& right_ );
  int MagickDLLDecl operator >= ( const Magick::Geometry& left_, const Magick::Geometry& right_ );
  int MagickDLLDecl operator <= ( const Magick::Geometry& left_, const Magick::Geometry& right_ );

  class MagickDLLDecl Geometry
  {
  public:
    
    Geometry ( unsigned int width_,
	       unsigned int height_,
	       unsigned int xOff_ = 0,
	       unsigned int yOff_ = 0,
	       bool xNegative_ = false,
	       bool yNegative_ = false );
    Geometry ( const std::string &geometry_ );
    Geometry ( const char * geometry_ );
    Geometry ( const Geometry &geometry_ );
    Geometry ( );
    ~Geometry ( void );
    
    // Width
    void          width ( unsigned int width_ );
    unsigned int  width ( void ) const;
    
    // Height
    void          height ( unsigned int height_ );
    unsigned int  height ( void ) const;
    
    // X offset from origin
    void          xOff ( unsigned int xOff_ );
    unsigned int  xOff ( void ) const;
    
    // Y offset from origin
    void          yOff ( unsigned int yOff_ );
    unsigned int  yOff ( void ) const;
    
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
    unsigned int  _width;
    unsigned int  _height;
    unsigned int  _xOff;
    unsigned int  _yOff;
    bool          _xNegative;
    bool          _yNegative;
    bool          _isValid;
    bool          _percent;        // Interpret width & height as percentages (%)
    bool          _aspect;         // Force exact size (!)
    bool          _greater;        // Re-size only if larger than geometry (>)
    bool          _less;           // Re-size only if smaller than geometry (<)
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
inline void Magick::Geometry::width ( unsigned int width_ )
{
  _width = width_;
  isValid( true );
}
inline unsigned int Magick::Geometry::width ( void ) const
{
  return _width;
}

// Height
inline void Magick::Geometry::height ( unsigned int height_ )
{
  _height = height_;
}
inline unsigned int Magick::Geometry::height ( void ) const
{
  return _height;
}

// X offset from origin
inline void Magick::Geometry::xOff ( unsigned int xOff_ )
{
  _xOff = xOff_;
}
inline unsigned int Magick::Geometry::xOff ( void ) const
{
  return _xOff;
}

// Y offset from origin
inline void Magick::Geometry::yOff ( unsigned int yOff_ )
{
  _yOff = yOff_;
}
inline unsigned int Magick::Geometry::yOff ( void ) const
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


#endif // Magick_Geometry_header

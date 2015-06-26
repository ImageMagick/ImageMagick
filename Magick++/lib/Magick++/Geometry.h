// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
// Copyright Dirk Lemstra 2014
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
  MagickPPExport int operator ==
    (const Magick::Geometry& left_,const Magick::Geometry& right_);
  MagickPPExport int operator !=
    (const Magick::Geometry& left_,const Magick::Geometry& right_);
  MagickPPExport int operator >
    (const Magick::Geometry& left_,const Magick::Geometry& right_);
  MagickPPExport int operator <
    (const Magick::Geometry& left_,const Magick::Geometry& right_);
  MagickPPExport int operator >=
    (const Magick::Geometry& left_,const Magick::Geometry& right_);
  MagickPPExport int operator <=
    (const Magick::Geometry& left_,const Magick::Geometry& right_);

  class MagickPPExport Geometry
  {
  public:

    // Default constructor
    Geometry();

    // Construct Geometry from specified string
    Geometry(const char *geometry_);

    // Copy constructor
    Geometry(const Geometry &geometry_);

    // Construct Geometry from specified string
    Geometry(const std::string &geometry_);

    // Construct Geometry from specified dimensions
    Geometry(size_t width_,size_t height_,::ssize_t xOff_=0,
      ::ssize_t yOff_=0);

    // Destructor
    ~Geometry(void);

    // Set via geometry string
    const Geometry& operator=(const char *geometry_);

    // Assignment operator
    Geometry& operator=(const Geometry& Geometry_);

    // Set via geometry string
    const Geometry& operator=(const std::string &geometry_);

    // Return geometry string
    operator std::string() const;

    // Resize without preserving aspect ratio (!)
    void aspect(bool aspect_);
    bool aspect(void) const;

    // Resize the image based on the smallest fitting dimension (^)
    void fillArea(bool fillArea_);
    bool fillArea(void) const;

    // Resize if image is greater than size (>)
    void greater(bool greater_);
    bool greater(void) const;

    // Height
    void height(size_t height_);
    size_t height(void) const;

    // Does object contain valid geometry?
    void isValid(bool isValid_);
    bool isValid(void) const;

    // Resize if image is less than size (<)
    void less(bool less_);
    bool less(void) const;

    // Resize using a pixel area count limit (@)
    void limitPixels(bool limitPixels_);
    bool limitPixels(void) const;

    // Width and height are expressed as percentages
    void percent(bool percent_);
    bool percent(void) const;

    // Width
    void width(size_t width_);
    size_t width(void) const;

    // X offset from origin
    void xOff(::ssize_t xOff_);
    ::ssize_t xOff(void) const;

    // Y offset from origin
    void yOff(::ssize_t yOff_);
    ::ssize_t yOff(void) const;

    //
    // Public methods below this point are for Magick++ use only.
    //

    // Construct from RectangleInfo
    Geometry(const MagickCore::RectangleInfo &rectangle_);

    // Set via RectangleInfo
    const Geometry& operator=(const MagickCore::RectangleInfo &rectangle_);

    // Return an ImageMagick RectangleInfo struct
    operator MagickCore::RectangleInfo() const;

  private:
    size_t _width;
    size_t _height;
    ::ssize_t _xOff;
    ::ssize_t _yOff;
    bool _isValid;
    bool _percent;     // Interpret width & height as percentages (%)
    bool _aspect;      // Force exact size (!)
    bool _greater;     // Resize only if larger than geometry (>)
    bool _less;        // Resize only if smaller than geometry (<)
    bool _fillArea;    // Resize the image based on the smallest fitting dimension (^)
    bool _limitPixels; // Resize using a pixel area count limit (@)
  };

  class MagickPPExport Offset;

  // Compare two Offset objects
  MagickPPExport int operator ==
    (const Magick::Offset& left_,const Magick::Offset& right_);
  MagickPPExport int operator !=
    (const Magick::Offset& left_,const Magick::Offset& right_);

  class MagickPPExport Offset
  {
  public:

    // Default constructor
    Offset();

    // Construct Offset from specified string
    Offset(const char *offset_);

    // Copy constructor
    Offset(const Offset &offset_);

    // Construct Offset from specified string
    Offset(const std::string &offset_);

    // Construct Offset from specified x and y
    Offset(ssize_t x_,ssize_t y_);

    // Destructor
    ~Offset(void);

    // Set via offset string
    const Offset& operator=(const char *offset_);

    // Assignment operator
    Offset& operator=(const Offset& offset_);

    // Set via offset string
    const Offset& operator=(const std::string &offset_);

    // X offset from origin
    ssize_t x(void) const;

    // Y offset from origin
    ssize_t y(void) const;

    //
    // Public methods below this point are for Magick++ use only.
    //

    // Return an ImageMagick OffsetInfo struct
    operator MagickCore::OffsetInfo() const;

  private:
    ssize_t _x;
    ssize_t _y;
  };

  class MagickPPExport Point;

  // Compare two Point objects
  MagickPPExport int operator ==
    (const Magick::Point& left_,const Magick::Point& right_);
  MagickPPExport int operator !=
    (const Magick::Point& left_,const Magick::Point& right_);

  class MagickPPExport Point
  {
  public:

    // Default constructor
    Point();

    // Construct Point from specified string
    Point(const char *point_);

    // Copy constructor
    Point(const Point &point_);

    // Construct Point from specified string
    Point(const std::string &point_);

    // Construct Point from specified x and y
    Point(double x_,double y_);

    // Construct Point from specified x y
    Point(double xy_);

    // Destructor
    ~Point(void);

    // Set via point string
    const Point& operator=(const char *point_);

    // Set via double value
    const Point& operator=(double xy_);

    // Assignment operator
    Point& operator=(const Point& point_);

    // Set via point string
    const Point& operator=(const std::string &point_);

    // Return point string
    operator std::string() const;

    // Does object contain valid point?
    bool isValid() const;

    // X offset from origin
    double x(void) const;

    // Y offset from origin
    double y(void) const;

  private:
    double _x;
    double _y;
  };
} // namespace Magick

#endif // Magick_Geometry_header

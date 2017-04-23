// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
// Copyright Dirk Lemstra 2014-2017
//
// Definition of Drawable (Graphic objects)
//
// The technique used for instantiating classes which derive from STL
// templates is described in Microsoft MSDN Article ID: Q168958
// "HOWTO: Exporting STL Components Inside & Outside of a Class".
// "http://support.microsoft.com/kb/168958"
//
// Note that version 3.0 of this article says that that only STL
// container template which supports DLL export is <vector> and we are
// not using <vector> as part of the Drawable implementation.
//

#if !defined(Magick_Drawable_header)
#define Magick_Drawable_header

#include "Magick++/Include.h"

#include <functional>
#include <string>
#include <vector>
#include <utility>
#include "Magick++/Color.h"
#include "Magick++/Geometry.h"

#if defined(MagickDLLExplicitTemplate)
#  if defined(MAGICK_PLUSPLUS_IMPLEMENTATION)
#    define MagickDrawableExtern
#  else
#   pragma warning( disable: 4231 ) // Disable warning regarding using extern
#    define MagickDrawableExtern extern
#  endif // MAGICK_PLUSPLUS_IMPLEMENTATION
#else
#  define MagickDrawableExtern
#endif // MagickDLLExplicitTemplate

namespace Magick
{
  //
  // Representation of an x,y coordinate
  //
  class MagickPPExport Coordinate
  {
  public:

    Coordinate(void)
      : _x(0),
        _y(0) {}

    Coordinate(double x_,double y_)
      : _x(x_),
        _y(y_) {}

    virtual ~Coordinate() {}

    void x(double x_) { _x=x_; }
    double x(void) const { return _x; }

    void y(double y_) { _y=y_; }
    double y(void) const { return _y; }

  private:
    double _x;
    double _y;
  };

  typedef std::vector<Magick::Coordinate> CoordinateList;

#if defined(MagickDLLExplicitTemplate)

  MagickDrawableExtern template class MagickPPExport
  std::allocator<Magick::Coordinate>;

#endif // MagickDLLExplicitTemplate

  // Compare two Coordinate objects regardless of LHS/RHS
  extern MagickPPExport int operator ==
    (const Coordinate& left_,const Coordinate& right_);
  extern MagickPPExport int operator !=
    (const Coordinate& left_, const Coordinate& right_);
  extern MagickPPExport int operator >
    (const Coordinate& left_, const Coordinate& right_);
  extern MagickPPExport int operator <
    (const Coordinate& left_, const Coordinate& right_);
  extern MagickPPExport int operator >=
    (const Coordinate& left_, const Coordinate& right_);
  extern MagickPPExport int operator <=
    (const Coordinate& left_, const Coordinate& right_);

  //
  // Base class for all drawable objects
  //
  class MagickPPExport DrawableBase
  {
  public:

    // Default constructor
    DrawableBase(void);

    // Destructor
    virtual ~DrawableBase(void);

    // Operator to invoke equivalent draw API call
    virtual void operator()(MagickCore::DrawingWand *) const;

    // Return polymorphic copy of object
    virtual DrawableBase* copy() const;
  };

  //
  // Representation of a drawable surrogate object to manage drawable objects
  //
  #undef Drawable // Conflict with <X11/Xproto.h>
  class MagickPPExport Drawable
  {
  public:

    // Default constructor
    Drawable(void);

    // Construct from DrawableBase
    Drawable(const DrawableBase& original_);

    // Destructor
    ~Drawable(void);

    // Copy constructor
    Drawable(const Drawable& original_);

    // Assignment operator
    Drawable& operator=(const Drawable& original_);

    // Operator to invoke contained object
    void operator()(MagickCore::DrawingWand *) const;

  private:
    DrawableBase* dp;
  };

  typedef std::vector<Magick::Drawable> DrawableList;

#if defined(MagickDLLExplicitTemplate)

  MagickDrawableExtern template class MagickPPExport
  std::allocator<Magick::Drawable>;

#endif // MagickDLLExplicitTemplate

//
// Base class for all drawable path elements for use with
// DrawablePath
//
class MagickPPExport VPathBase
{
public:
  // Constructor
  VPathBase ( void )
    { }

  // Destructor
  virtual ~VPathBase ( void );

  // Assignment operator
  //    const VPathBase& operator= (const VPathBase& original_ );

  // Operator to invoke equivalent draw API call
  virtual void operator()( MagickCore::DrawingWand *context_ ) const = 0;

  // Return polymorphic copy of object
  virtual VPathBase* copy() const = 0;
};

//
// Representation of a drawable path element surrogate object to
// manage drawable path elements so they may be passed as a list to
// DrawablePath.
//
class MagickPPExport VPath
{
public:
  // Constructor
  VPath ( void );
    
  // Construct from VPathBase
  VPath ( const VPathBase& original_ );
    
  // Destructor
  virtual ~VPath ( void );

  // Copy constructor
  VPath ( const VPath& original_ );
    
  // Assignment operator
  VPath& operator= (const VPath& original_ );

  // Operator to invoke contained object
  void operator()( MagickCore::DrawingWand *context_ ) const;

private:
  VPathBase* dp;
};

typedef std::vector<Magick::VPath> VPathList;

#if defined(MagickDLLExplicitTemplate)

MagickDrawableExtern template class MagickPPExport
std::allocator<Magick::VPath>;

// MagickDrawableExtern template class MagickPPExport
// std::vector<Magick::VPath, std::allocator<Magick::VPath> >;

#endif // MagickDLLExplicitTemplate

//
// Drawable Objects
//

// Affine (scaling, rotation, and translation)
class MagickPPExport DrawableAffine  : public DrawableBase
{
public:
  DrawableAffine ( double sx_, double sy_,
                   double rx_, double ry_,
                   double tx_, double ty_ );

  DrawableAffine ( void );

  /*virtual*/ ~DrawableAffine( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/
  DrawableBase* copy() const;
    
  void sx( const double sx_ )
    {
      _affine.sx = sx_;
    }
  double sx( void ) const
    {
      return _affine.sx;
    }

  void sy( const double sy_ )
    {
      _affine.sy = sy_;
    }
  double sy( void ) const
    {
      return _affine.sy;
    }

  void rx( const double rx_ )
    {
      _affine.rx = rx_;
    }
  double rx( void ) const
    {
      return _affine.rx;
    }
  
  void ry( const double ry_ )
    {
      _affine.ry = ry_;
    }
  double ry( void ) const
    {
      return _affine.ry;
    }
  
  void tx( const double tx_ )
    {
      _affine.tx = tx_;
    }
  double tx( void ) const
    {
      return _affine.tx;
    }
  
  void ty( const double ty_ )
    {
      _affine.ty = ty_;
    }
  double ty( void ) const
    {
      return _affine.ty;
    }
  
private:
  MagickCore::AffineMatrix  _affine;
};

// Change pixel alpha value to transparent using PaintMethod
class MagickPPExport DrawableAlpha : public DrawableBase
{
public:

    DrawableAlpha(double x_, double y_,PaintMethod paintMethod_)
      : _x(x_),
        _y(y_),
        _paintMethod(paintMethod_)
    {
    }

    ~DrawableAlpha(void);

    // Operator to invoke equivalent draw API call
    void operator()(MagickCore::DrawingWand *context_) const;

    // Return polymorphic copy of object
    DrawableBase* copy() const;

    void x(double x_)
    {
      _x=x_;
    }

    double x(void) const
    {
      return(_x);
    }

    void y(double y_)
    {
      _y=y_;
    }

    double y(void) const
    {
      return(_y);
    }

    void paintMethod(PaintMethod paintMethod_)
    {
      _paintMethod=paintMethod_;
    }

    PaintMethod paintMethod(void) const
    {
      return(_paintMethod);
    }

  private:

    double _x;
    double _y;
    PaintMethod _paintMethod;
};

// Arc
class MagickPPExport DrawableArc : public DrawableBase
{
public:
  DrawableArc ( double startX_, double startY_,
                double endX_, double endY_,
                double startDegrees_, double endDegrees_ )
    : _startX(startX_),
      _startY(startY_),
      _endX(endX_),
      _endY(endY_),
      _startDegrees(startDegrees_),
      _endDegrees(endDegrees_)
    { }

  /*virtual*/ ~DrawableArc( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void startX( double startX_ )
    {
      _startX = startX_;
    }
  double startX( void ) const
    {
      return _startX;
    }
  
  void startY( double startY_ )
    {
      _startY = startY_;
    }
  double startY( void ) const
    {
      return _startY;
    }
  
  void endX( double endX_ )
    {
      _endX = endX_;
    }
  double endX( void ) const
    {
      return _endX;
    }

  void endY( double endY_ )
    {
      _endY = endY_;
    }
  double endY( void ) const
    {
      return _endY;
    }
  
  void startDegrees( double startDegrees_ )
    {
      _startDegrees = startDegrees_;
    }
  double startDegrees( void ) const
    {
      return _startDegrees;
    }

  void endDegrees( double endDegrees_ )
    {
      _endDegrees = endDegrees_;
    }
  double endDegrees( void ) const
    {
      return _endDegrees;
    }
  
private:
  double _startX;
  double _startY;
  double _endX;
  double _endY;
  double _startDegrees;
  double _endDegrees;
};

// Bezier curve (Coordinate list must contain at least three members)
class MagickPPExport DrawableBezier : public DrawableBase
{
public:
  // Construct from coordinates
  DrawableBezier ( const CoordinateList &coordinates_ );

  // Copy constructor
  DrawableBezier ( const DrawableBezier& original_ );

  // Destructor
  /*virtual*/ ~DrawableBezier ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;
  
private:
  CoordinateList _coordinates;
};

  // Sets the border color to be used for drawing bordered objects.
  class MagickPPExport DrawableBorderColor : public DrawableBase
  {
  public:

    DrawableBorderColor(const Color &color_);
  
    DrawableBorderColor(const DrawableBorderColor &original_);
  
    ~DrawableBorderColor(void);
  
    // Operator to invoke equivalent draw API call
    void operator()(MagickCore::DrawingWand *context_) const;

    void color(const Color &color_);
    Color color(void) const;
  
    // Return polymorphic copy of object
    DrawableBase* copy() const;
  
  private:
    Color _color;
  };

  // Sets the polygon fill rule to be used by the clipping path.
  class MagickPPExport DrawableClipRule : public DrawableBase
  {
  public:

    DrawableClipRule(const FillRule fillRule_);

    ~DrawableClipRule(void);

    // Operator to invoke equivalent draw API call
    void operator()(MagickCore::DrawingWand *context_) const;

    void fillRule(const FillRule fillRule_);
    FillRule fillRule(void) const;

    // Return polymorphic copy of object
    DrawableBase* copy() const;

  private:
    FillRule _fillRule;
  };

  // Sets the interpretation of clip path units.
  class MagickPPExport DrawableClipUnits : public DrawableBase
  {
  public:

    DrawableClipUnits(const ClipPathUnits units_);

    ~DrawableClipUnits(void);

    // Operator to invoke equivalent draw API call
    void operator()(MagickCore::DrawingWand *context_) const;

    void units(const ClipPathUnits units_);
    ClipPathUnits units(void) const;

    // Return polymorphic copy of object
    DrawableBase* copy() const;

  private:
    ClipPathUnits _units;
  };

// Pop (terminate) clip path definition
class MagickPPExport DrawablePopClipPath : public DrawableBase
{
public:
  DrawablePopClipPath ( void )
    : _dummy(0)
    {
    }

  /*virtual*/ ~DrawablePopClipPath ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

private:
  ::ssize_t   _dummy;
};

// Push (create) Clip path definition
class MagickPPExport DrawablePushClipPath : public DrawableBase
{
public:
  DrawablePushClipPath ( const std::string &id_);

  DrawablePushClipPath ( const DrawablePushClipPath& original_ );

  /*virtual*/ ~DrawablePushClipPath ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

private:
  std::string _id;
};

// Named Clip Path
class MagickPPExport DrawableClipPath : public DrawableBase
{
public:
  DrawableClipPath ( const std::string &id_ );
  DrawableClipPath ( const DrawableClipPath& original_ );

  /*virtual*/ ~DrawableClipPath ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void clip_path( const std::string &id_ )
    {
      _id = id_.c_str(); //multithread safe
    }
  std::string clip_path( void ) const
    {
      return _id;
    }

private:
  std::string   _id;
};

// Circle
class MagickPPExport DrawableCircle : public DrawableBase
{
public:
  DrawableCircle ( double originX_, double originY_,
                   double perimX_, double perimY_ )
    : _originX(originX_),
      _originY(originY_),
      _perimX(perimX_),
      _perimY(perimY_)
    {
    }

  /*virtual*/ ~DrawableCircle ( void );
    
  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void originX( double originX_ )
    {
      _originX = originX_;
    }
  double originX( void ) const
    {
      return _originX;
    }

  void originY( double originY_ )
    {
      _originY = originY_;
    }
  double originY( void ) const
    {
      return _originY;
    }

  void perimX( double perimX_ )
    {
      _perimX = perimX_;
    }
  double perimX( void ) const
    {
      return _perimX;
    }

  void perimY( double perimY_ )
    {
      _perimY = perimY_;
    }
  double perimY( void ) const
    {
      return _perimY;
    }

private:
  double _originX;
  double _originY;
  double _perimX;
  double _perimY;
};

// Colorize at point using PaintMethod
class MagickPPExport DrawableColor : public DrawableBase
{
public:
  DrawableColor ( double x_, double y_,
                  PaintMethod paintMethod_ )
    : _x(x_),
      _y(y_),
      _paintMethod(paintMethod_)
    { }

  /*virtual*/ ~DrawableColor ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void x( double x_ )
    {
      _x = x_;
    }
  double x( void ) const
    {
      return _x;
    }

  void y( double y_ )
    {
      _y = y_;
    }
  double y( void ) const
    {
      return _y;
    }

  void paintMethod( PaintMethod paintMethod_ )
    {
      _paintMethod = paintMethod_;
    }
  PaintMethod paintMethod( void ) const
    {
      return _paintMethod;
    }

private:
  double _x;
  double _y;
  PaintMethod _paintMethod;
};

// Draw image at point, scaled to size specified by width and height
class MagickPPExport Image;
class MagickPPExport DrawableCompositeImage : public DrawableBase
{
public:
  DrawableCompositeImage ( double x_, double y_,
                           const std::string &filename_ );

  DrawableCompositeImage ( double x_, double y_,
                           const Image &image_ );

  DrawableCompositeImage ( double x_, double y_,
                           double width_, double height_,
                           const std::string &filename_ );

  DrawableCompositeImage ( double x_, double y_,
                           double width_, double height_,
                           const Image &image_ );

  DrawableCompositeImage ( double x_, double y_,
                           double width_, double height_,
                           const std::string &filename_,
                           CompositeOperator composition_ );

  DrawableCompositeImage ( double x_, double y_,
                           double width_, double height_,
                           const Image &image_,
                           CompositeOperator composition_ );

  // Copy constructor
  DrawableCompositeImage ( const DrawableCompositeImage& original_ );

  // Destructor
  /*virtual*/ ~DrawableCompositeImage( void );

  // Assignment operator
  DrawableCompositeImage& operator=
  (const DrawableCompositeImage& original_ );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;
    
  void composition( CompositeOperator composition_ )
    {
      _composition = composition_;
    }
  CompositeOperator composition( void ) const
    {
      return _composition;
    }

  void filename( const std::string &image_ );
  std::string filename( void ) const;

  void x( double x_ )
    {
      _x = x_;
    }
  double x( void ) const
    {
      return _x;
    }

  void y( double y_ )
    {
      _y = y_;
    }
  double y( void ) const
    {
      return _y;
    }

  void width( double width_ )
    {
      _width = width_;
    }
  double width( void ) const
    {
      return _width;
    }

  void height( double height_ )
    {
      _height = height_;
    }
  double height( void ) const
    {
      return _height;
    }

  void image( const Image &image_ );
  Magick::Image image( void ) const;

  // Specify image format used to output Base64 inlined image data.
  void magick( std::string magick_ );
  std::string magick( void );

private:
  CompositeOperator  _composition;
  double             _x;
  double             _y;
  double             _width;
  double             _height;
  Image*             _image;
};

// Density
class MagickPPExport DrawableDensity : public DrawableBase
{
public:

  DrawableDensity(const Point &density_);

  DrawableDensity(const std::string &density_);

  ~DrawableDensity(void);

  void operator()(MagickCore::DrawingWand *context_) const;

  DrawableBase* copy() const;

private:
  std::string _density;
};

// Ellipse
class MagickPPExport DrawableEllipse : public DrawableBase
{
public:
  DrawableEllipse ( double originX_, double originY_, 
                    double radiusX_, double radiusY_,
                    double arcStart_, double arcEnd_ )
    : _originX(originX_),
      _originY(originY_),
      _radiusX(radiusX_),
      _radiusY(radiusY_),
      _arcStart(arcStart_),
      _arcEnd(arcEnd_)
    { }

  /*virtual*/ ~DrawableEllipse( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void originX( double originX_ )
    {
      _originX = originX_;
    }
  double originX( void ) const
    {
      return _originX;
    }

  void originY( double originY_ )
    {
      _originY = originY_;
    }
  double originY( void ) const
    {
      return _originY;
    }

  void radiusX( double radiusX_ )
    {
      _radiusX = radiusX_;
    }
  double radiusX( void ) const
    {
      return _radiusX;
    }

  void radiusY( double radiusY_ )
    {
      _radiusY = radiusY_;
    }
  double radiusY( void ) const
    {
      return _radiusY;
    }

  void arcStart( double arcStart_ )
    {
      _arcStart = arcStart_;
    }
  double arcStart( void ) const
    {
      return _arcStart;
    }

  void arcEnd( double arcEnd_ )
    {
      _arcEnd = arcEnd_;
    }
  double arcEnd( void ) const
    {
      return _arcEnd;
    }

private:
  double _originX;
  double _originY; 
  double _radiusX;
  double _radiusY;
  double _arcStart;
  double _arcEnd;
};

// Specify drawing fill color
class MagickPPExport DrawableFillColor : public DrawableBase
{
public:
  DrawableFillColor ( const Color &color_ );

  DrawableFillColor ( const DrawableFillColor& original_ );

  /*virtual*/ ~DrawableFillColor( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void color( const Color &color_ )
    {
      _color = color_;
    }
  Color color( void ) const
    {
      return _color;
    }

private:
  Color _color;
};

  // Sets the URL to use as a fill pattern for filling objects. Only local
  // URLs("#identifier") are supported at this time. These local URLs are
  // normally created by defining a named fill pattern with
  // DrawablePushPattern/DrawablePopPattern.
  class MagickPPExport DrawableFillPatternUrl : public DrawableBase
  {
  public:

    DrawableFillPatternUrl(const std::string &url_);

    ~DrawableFillPatternUrl(void);

    DrawableFillPatternUrl(const DrawableFillPatternUrl& original_);

    // Operator to invoke equivalent draw API call
    void operator()(MagickCore::DrawingWand *context_) const;

    void url(const std::string &url_);
    std::string url(void) const;

    // Return polymorphic copy of object
    DrawableBase* copy() const;
  
  private:
    std::string _url;
  };

// Specify fill rule (fill-rule)
class MagickPPExport DrawableFillRule : public DrawableBase
{
public:
  DrawableFillRule ( const FillRule fillRule_ )
    : _fillRule(fillRule_)
    {
    }

  /*virtual*/ ~DrawableFillRule ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void fillRule( const FillRule fillRule_ )
    {
      _fillRule = fillRule_;
    }
  FillRule fillRule( void ) const
    {
      return _fillRule;
    }

private:
  FillRule _fillRule;
};

// Specify drawing fill alpha
class MagickPPExport DrawableFillOpacity : public DrawableBase
{
public:

  DrawableFillOpacity(double opacity_)
    : _opacity(opacity_)
  {
  }

  ~DrawableFillOpacity ( void );

  // Operator to invoke equivalent draw API call
  void operator()(MagickCore::DrawingWand *context_) const;

  // Return polymorphic copy of object
  DrawableBase* copy() const;

  void opacity(double opacity_)
  {
    _opacity=opacity_;
  }

  double opacity(void) const
  {
    return(_opacity);
  }

private:
  double _opacity;
};

// Specify text font
class MagickPPExport DrawableFont : public DrawableBase
{
public:
  DrawableFont ( const std::string &font_ );

  DrawableFont ( const std::string &family_,
                 StyleType style_,
                 const unsigned int weight_,
                 StretchType stretch_ );
  DrawableFont ( const DrawableFont& original_ );

  /*virtual*/ ~DrawableFont ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void font( const std::string &font_ )
    {
      _font = font_;
    }
  std::string font( void ) const
    {
      return _font;
    }

private:
  std::string   _font;
  std::string   _family;
  StyleType     _style;
  unsigned int _weight;
  StretchType   _stretch;
};

// Specify text positioning gravity
class MagickPPExport DrawableGravity : public DrawableBase
{
public:
  DrawableGravity ( GravityType gravity_ )
    : _gravity(gravity_)
    {
    }

  /*virtual*/ ~DrawableGravity ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void gravity( GravityType gravity_ )
    {
      _gravity = gravity_;
    }
  GravityType gravity( void ) const
    {
      return _gravity;
    }

private:
  GravityType _gravity;
};

// Line
class MagickPPExport DrawableLine : public DrawableBase
{
public:
  DrawableLine ( double startX_, double startY_,
                 double endX_, double endY_ )
    : _startX(startX_),
      _startY(startY_),
      _endX(endX_),
      _endY(endY_)
    { }

  /*virtual*/ ~DrawableLine ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void startX( double startX_ )
    {
      _startX = startX_;
    }
  double startX( void ) const
    {
      return _startX;
    }

  void startY( double startY_ )
    {
      _startY = startY_;
    }
  double startY( void ) const
    {
      return _startY;
    }

  void endX( double endX_ )
    {
      _endX = endX_;
    }
  double endX( void ) const
    {
      return _endX;
    }

  void endY( double endY_ )
    {
      _endY = endY_;
    }
  double endY( void ) const
    {
      return _endY;
    }

private:
  double _startX;
  double _startY;
  double _endX;
  double _endY;
};

// Drawable Path
class MagickPPExport DrawablePath : public DrawableBase
{
public:
  DrawablePath ( const VPathList &path_ );

  DrawablePath ( const DrawablePath& original_ );

  /*virtual*/ ~DrawablePath ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

private:
  VPathList _path;
};

// Point
class MagickPPExport DrawablePoint : public DrawableBase
{
public:
  DrawablePoint ( double x_, double y_ )
    : _x(x_),
      _y(y_)
    { }

  /*virtual*/ ~DrawablePoint ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void x( double x_ )
    {
      _x = x_;
    }
  double x( void ) const
    {
      return _x;
    }

  void y( double y_ )
    {
      _y = y_;
    }
  double y( void ) const
    {
      return _y;
    }

private:
  double _x;
  double _y;
};

// Text pointsize
class MagickPPExport DrawablePointSize : public DrawableBase
{
public:
  DrawablePointSize ( double pointSize_ )
    : _pointSize(pointSize_)
    { }

  /*virtual*/ ~DrawablePointSize ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void pointSize( double pointSize_ )
    {
      _pointSize = pointSize_;
    }
  double pointSize( void ) const
    {
      return _pointSize;
    }

private:
  double _pointSize;
};

// Polygon (Coordinate list must contain at least three members)
class MagickPPExport DrawablePolygon : public DrawableBase
{
public:
  DrawablePolygon ( const CoordinateList &coordinates_ );

  DrawablePolygon ( const DrawablePolygon& original_ );

  /*virtual*/ ~DrawablePolygon ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

private:
  CoordinateList _coordinates;
};

// Polyline (Coordinate list must contain at least three members)
class MagickPPExport DrawablePolyline : public DrawableBase
{
public:
  DrawablePolyline ( const CoordinateList &coordinates_ );

  DrawablePolyline ( const DrawablePolyline& original_ );

  /*virtual*/ ~DrawablePolyline ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

private:
  CoordinateList _coordinates;
};

// Pop Graphic Context
class MagickPPExport DrawablePopGraphicContext : public DrawableBase
{
public:
  DrawablePopGraphicContext ( void )
    : _dummy(0)
    {
    }

  /*virtual*/ ~DrawablePopGraphicContext ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

private:
  ::ssize_t   _dummy;
};

// Push Graphic Context
class MagickPPExport DrawablePushGraphicContext : public DrawableBase
{
public:
  DrawablePushGraphicContext ( void )
    : _dummy(0)
    {
    }

  /*virtual*/ ~DrawablePushGraphicContext ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

private:
  ::ssize_t   _dummy;
};

// Pop (terminate) Pattern definition
class MagickPPExport DrawablePopPattern : public DrawableBase
{
public:
  DrawablePopPattern ( void )
    : _dummy(0)
    {
    }

  /*virtual*/ ~DrawablePopPattern ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

private:
  ::ssize_t   _dummy;
};

// Push (create) Pattern definition
class MagickPPExport DrawablePushPattern : public DrawableBase
{
public:
  DrawablePushPattern ( const std::string &id_, ::ssize_t x_, ::ssize_t y_,
                        size_t width_, size_t height_ );

  DrawablePushPattern ( const DrawablePushPattern& original_ );

  /*virtual*/ ~DrawablePushPattern ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

private:
  std::string         _id;
  ::ssize_t		_x;
  ::ssize_t		_y;
  size_t		_width;
  size_t		_height;
};

// Rectangle
class MagickPPExport DrawableRectangle : public DrawableBase
{
public:
  DrawableRectangle ( double upperLeftX_, double upperLeftY_,
                      double lowerRightX_, double lowerRightY_ )
    : _upperLeftX(upperLeftX_),
      _upperLeftY(upperLeftY_),
      _lowerRightX(lowerRightX_),
      _lowerRightY(lowerRightY_)
    { }

  /*virtual*/ ~DrawableRectangle ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void upperLeftX( double upperLeftX_ )
    {
      _upperLeftX = upperLeftX_;
    }
  double upperLeftX( void ) const
    {
      return _upperLeftX;
    }

  void upperLeftY( double upperLeftY_ )
    {
      _upperLeftY = upperLeftY_;
    }
  double upperLeftY( void ) const
    {
      return _upperLeftY;
    }

  void lowerRightX( double lowerRightX_ )
    {
      _lowerRightX = lowerRightX_;
    }
  double lowerRightX( void ) const
    {
      return _lowerRightX;
    }

  void lowerRightY( double lowerRightY_ )
    {
      _lowerRightY = lowerRightY_;
    }
  double lowerRightY( void ) const
    {
      return _lowerRightY;
    }

private:
  double _upperLeftX;
  double _upperLeftY;
  double _lowerRightX;
  double _lowerRightY;
};

// Apply Rotation
class MagickPPExport DrawableRotation : public DrawableBase
{
public:
  DrawableRotation ( double angle_ )
    : _angle( angle_ )
    { }

  /*virtual*/ ~DrawableRotation ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void angle( double angle_ )
    {
      _angle = angle_;
    }
  double angle( void ) const
    {
      return _angle;
    }

private:
  double _angle;
};

// Round Rectangle
class MagickPPExport DrawableRoundRectangle : public DrawableBase
{
public:
  DrawableRoundRectangle ( double upperLeftX_, double upperLeftY_,
                           double lowerRightX_, double lowerRightY_,
                           double cornerWidth_, double cornerHeight_ )
    : _upperLeftX(upperLeftX_),
      _upperLeftY(upperLeftY_),
      _lowerRightX(lowerRightX_),
      _lowerRightY(lowerRightY_),
      _cornerWidth(cornerWidth_),
      _cornerHeight(cornerHeight_)
    { }

  /*virtual*/ ~DrawableRoundRectangle ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)

  void centerX( double centerX_ )
    {
      _upperLeftX = centerX_;
    }
  double centerX( void ) const
    {
      return _upperLeftX;
    }

  void centerY( double centerY_ )
    {
      _upperLeftY = centerY_;
    }
  double centerY( void ) const
    {
      return _upperLeftY;
    }

  void width( double width_ )
    {
      _lowerRightX = width_;
    }
  double width( void ) const
    {
      return _lowerRightX;
    }

  void hight( double hight_ )
    {
      _lowerRightY = hight_;
    }
  double hight( void ) const
    {
      return _lowerRightY;
    }

#endif

  void upperLeftX( double upperLeftX_ )
    {
      _upperLeftX = upperLeftX_;
    }
  double upperLeftX( void ) const
    {
      return _upperLeftX;
    }

  void upperLeftY( double upperLeftY_ )
    {
      _upperLeftY = upperLeftY_;
    }
  double upperLeftY( void ) const
    {
      return _upperLeftY;
    }

  void lowerRightX( double lowerRightX_ )
    {
      _lowerRightX = lowerRightX_;
    }
  double lowerRightX( void ) const
    {
      return _lowerRightX;
    }

  void lowerRightY( double lowerRightY_ )
    {
      _lowerRightY = lowerRightY_;
    }
  double lowerRightY( void ) const
    {
      return _lowerRightY;
    }

  void cornerWidth( double cornerWidth_ )
    {
      _cornerWidth = cornerWidth_;
    }
  double cornerWidth( void ) const
    {
      return _cornerWidth;
    }

  void cornerHeight( double cornerHeight_ )
    {
      _cornerHeight = cornerHeight_;
    }
  double cornerHeight( void ) const
    {
      return _cornerHeight;
    }

private:
  double _upperLeftX;
  double _upperLeftY;
  double _lowerRightX;
  double _lowerRightY;
  double _cornerWidth;
  double _cornerHeight;
};

// Apply Scaling
class MagickPPExport DrawableScaling : public DrawableBase
{
public:
  DrawableScaling ( double x_, double y_ )
    : _x(x_),
      _y(y_)
    { }

  /*virtual*/ ~DrawableScaling ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void x( double x_ )
    {
      _x = x_;
    }
  double x( void ) const
    {
      return _x;
    }

  void y( double y_ )
    {
      _y = y_;
    }
  double y( void ) const
    {
      return _y;
    }

private:
  double _x;
  double _y;
};

// Apply Skew in X direction
class MagickPPExport DrawableSkewX : public DrawableBase
{
public:
  DrawableSkewX ( double angle_ )
    : _angle(angle_)
    { }

  /*virtual*/ ~DrawableSkewX ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void angle( double angle_ )
    {
      _angle = angle_;
    }
  double angle( void ) const
    {
      return _angle;
    }

private:
  double _angle;
};

// Apply Skew in Y direction
class MagickPPExport DrawableSkewY : public DrawableBase
{
public:
  DrawableSkewY ( double angle_ )
    : _angle(angle_)
    { }

  /*virtual*/ ~DrawableSkewY ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void angle( double angle_ )
    {
      _angle = angle_;
    }
  double angle( void ) const
    {
      return _angle;
    }

private:
  double _angle;
};

  // Stroke dasharray
  //
  // dasharray_ is an allocated array terminated by value 0.0 or 0.
  // The array is copied so the original does not need to be preserved.
  // Pass a null pointer to clear an existing dash array setting.
  class MagickPPExport DrawableStrokeDashArray : public DrawableBase
  {
  public:

      DrawableStrokeDashArray(const double* dasharray_);

      DrawableStrokeDashArray(const Magick::DrawableStrokeDashArray &original_);

      ~DrawableStrokeDashArray(void);

      // Operator to invoke equivalent draw API call
      void operator()(MagickCore::DrawingWand *context_) const;
  
      // Return polymorphic copy of object
      DrawableBase* copy() const;
  
      void dasharray(const double* dasharray_);
      const double* dasharray(void) const;

      DrawableStrokeDashArray& operator=(
        const Magick::DrawableStrokeDashArray &original_);

  private:
      size_t _size;
      double *_dasharray;
  };

  // Stroke dashoffset
  class MagickPPExport DrawableStrokeDashOffset : public DrawableBase
  {
  public:
    DrawableStrokeDashOffset(const double offset_)
      : _offset(offset_)
      { }

     ~DrawableStrokeDashOffset(void);

    // Operator to invoke equivalent draw API call
    void operator()(MagickCore::DrawingWand *context_) const;

    // Return polymorphic copy of object
    DrawableBase* copy() const;

    void offset(const double offset_);
    double offset(void) const;
  
  private:
    double _offset;
  };

// Stroke linecap
class MagickPPExport DrawableStrokeLineCap : public DrawableBase
{
public:
  DrawableStrokeLineCap ( LineCap linecap_ )
    : _linecap(linecap_)
    { }

  /*virtual*/ ~DrawableStrokeLineCap ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void linecap( LineCap linecap_ )
    {
      _linecap = linecap_;
    }
  LineCap linecap( void ) const
    {
      return _linecap;
    }

private:
  LineCap _linecap;
};

// Stroke linejoin
class MagickPPExport DrawableStrokeLineJoin : public DrawableBase
{
public:
  DrawableStrokeLineJoin ( LineJoin linejoin_ )
    : _linejoin(linejoin_)
    { }

  /*virtual*/ ~DrawableStrokeLineJoin ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void linejoin( LineJoin linejoin_ )
    {
      _linejoin = linejoin_;
    }
  LineJoin linejoin( void ) const
    {
      return _linejoin;
    }

private:
  LineJoin _linejoin;
};

// Stroke miterlimit
class MagickPPExport DrawableMiterLimit : public DrawableBase
{
public:
  DrawableMiterLimit ( size_t miterlimit_ )
    : _miterlimit(miterlimit_)
    { }

  /*virtual*/ ~DrawableMiterLimit ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void miterlimit( size_t miterlimit_ )
    {
      _miterlimit = miterlimit_;
    }
  size_t miterlimit( void ) const
    {
      return _miterlimit;
    }

private:
  size_t _miterlimit;
};

// Sets the pattern used for stroking object outlines.
class MagickPPExport DrawableStrokePatternUrl : public DrawableBase
{
public:

  DrawableStrokePatternUrl(const std::string &url_);

  ~DrawableStrokePatternUrl(void);

  DrawableStrokePatternUrl(const DrawableStrokePatternUrl& original_);

  // Operator to invoke equivalent draw API call
  void operator()(MagickCore::DrawingWand *context_) const;

  void url(const std::string &url_);
  std::string url(void) const;

  // Return polymorphic copy of object
  DrawableBase* copy() const;

private:
  std::string _url;
};

// Stroke antialias
class MagickPPExport DrawableStrokeAntialias : public DrawableBase
{
public:
  DrawableStrokeAntialias ( bool flag_ )
    : _flag(flag_)
    { }

  /*virtual*/ ~DrawableStrokeAntialias ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void flag( bool flag_ )
    {
      _flag = flag_;
    }
  bool flag( void ) const
    {
      return _flag;
    }

private:
  bool _flag;
};

// Stroke color
class MagickPPExport DrawableStrokeColor : public DrawableBase
{
public:
  DrawableStrokeColor ( const Color &color_ );

  DrawableStrokeColor ( const DrawableStrokeColor& original_ );

  /*virtual*/ ~DrawableStrokeColor ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void color( const Color& color_ )
    {
      _color = color_;
    }
  Color color( void ) const
    {
      return _color;
    }

private:
  Color _color;
};

// Stroke opacity
class MagickPPExport DrawableStrokeOpacity : public DrawableBase
{
public:

  DrawableStrokeOpacity(double opacity_)
    : _opacity(opacity_)
  {
  }

  ~DrawableStrokeOpacity(void);

  // Operator to invoke equivalent draw API call
  void operator()(MagickCore::DrawingWand *context_) const;

  // Return polymorphic copy of object
  DrawableBase* copy() const;

  void opacity(double opacity_)
  {
    _opacity=opacity_;
  }

  double opacity(void) const
  {
    return(_opacity);
  }

private:
  double _opacity;
};

// Stroke width
class MagickPPExport DrawableStrokeWidth : public DrawableBase
{
public:
  DrawableStrokeWidth ( double width_ )
    : _width(width_)
    { }

  /*virtual*/ ~DrawableStrokeWidth ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void width( double width_ )
    {
      _width = width_;
    }
  double width( void ) const
    {
      return _width;
    }

private:
  double _width;
};

// Draw text at point
class MagickPPExport DrawableText : public DrawableBase
{
public:
  DrawableText ( const double x_, const double y_,
                 const std::string &text_ );
  DrawableText ( const double x_, const double y_,
                 const std::string &text_, const std::string &encoding_);

  DrawableText ( const DrawableText& original_ );

  /*virtual*/ ~DrawableText ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void encoding(const std::string &encoding_)
    {
      _encoding = encoding_;
    }

  void x( double x_ )
    {
      _x = x_;
    }
  double x( void ) const
    {
      return _x;
    }

  void y( double y_ )
    {
      _y = y_;
    }
  double y( void ) const
    {
      return _y;
    }

  void text( const std::string &text_ )
    {
      _text = text_;
    }
  std::string text( void ) const
    {
      return _text;
    }

private:
  double      _x;
  double      _y;
  std::string _text;
  std::string _encoding;
};

// Text alignment
class MagickPPExport DrawableTextAlignment : public DrawableBase
{
public:

  DrawableTextAlignment(AlignType alignment_);

  DrawableTextAlignment(const DrawableTextAlignment& original_);

  ~DrawableTextAlignment(void);

  // Operator to invoke equivalent draw API call
  void operator()(MagickCore::DrawingWand *context_) const;

  void alignment(AlignType alignment_);
  AlignType alignment(void) const;

  // Return polymorphic copy of object
  DrawableBase* copy() const;

private:
  AlignType _alignment;
};

// Text antialias
class MagickPPExport DrawableTextAntialias : public DrawableBase
{
public:
  DrawableTextAntialias ( bool flag_ );

  DrawableTextAntialias( const DrawableTextAntialias &original_ );

  /*virtual*/ ~DrawableTextAntialias ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void flag( bool flag_ )
    {
      _flag = flag_;
    }
  bool flag( void ) const
    {
      return _flag;
    }

private:
  bool _flag;
};

// Decoration (text decoration)
class MagickPPExport DrawableTextDecoration : public DrawableBase
{
public:
  DrawableTextDecoration ( DecorationType decoration_ );

  DrawableTextDecoration ( const DrawableTextDecoration& original_ );

  /*virtual*/ ~DrawableTextDecoration( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/  void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void decoration( DecorationType decoration_ )
    {
      _decoration = decoration_;
    }
  DecorationType decoration( void ) const
    {
      return _decoration;
    }

private:
  DecorationType _decoration;
};

  // Render text right-to-left or left-to-right.
  class MagickPPExport DrawableTextDirection : public DrawableBase
  {
  public:

    DrawableTextDirection(DirectionType direction_);

    ~DrawableTextDirection(void);

    void operator()(MagickCore::DrawingWand *context_) const;

    void direction(DirectionType direction_);
    DirectionType direction(void) const;

    DrawableBase* copy() const;

  private:
    DirectionType _direction;
  };

  // Specify text inter-line spacing
  class MagickPPExport DrawableTextInterlineSpacing : public DrawableBase
  {
  public:

    DrawableTextInterlineSpacing(double spacing_);

    ~DrawableTextInterlineSpacing(void);

    void operator()(MagickCore::DrawingWand *context_) const;

    void spacing(double spacing_);
    double spacing(void) const;

    DrawableBase* copy() const;

  private:
    double _spacing;
  };

  // Specify text inter-word spacing
  class MagickPPExport DrawableTextInterwordSpacing : public DrawableBase
  {
  public:

    DrawableTextInterwordSpacing(double spacing_);

    ~DrawableTextInterwordSpacing(void);

    void operator()(MagickCore::DrawingWand *context_) const;

    void spacing(double spacing_);
    double spacing(void) const;

    DrawableBase *copy() const;

  private:
    double _spacing;
  };

  // Specify text kerning
  class MagickPPExport DrawableTextKerning : public DrawableBase
  {
  public:

    DrawableTextKerning(double kerning_);

    ~DrawableTextKerning(void);

    void operator()(MagickCore::DrawingWand *context_) const;

    void kerning(double kerning_);
    double kerning(void) const;

    DrawableBase *copy() const;

  private:
    double _kerning;
  };

// Text undercolor box
class MagickPPExport DrawableTextUnderColor : public DrawableBase
{
public:
  DrawableTextUnderColor ( const Color &color_ );

  DrawableTextUnderColor ( const DrawableTextUnderColor& original_ );

  /*virtual*/ ~DrawableTextUnderColor ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void color( const Color& color_ )
    {
      _color = color_;
    }
  Color color( void ) const
    {
      return _color;
    }

private:
  Color _color;
};

// Apply Translation
class MagickPPExport DrawableTranslation : public DrawableBase
{
public:
  DrawableTranslation ( double x_, double y_ )
    : _x(x_),
      _y(y_)
    { }

  /*virtual*/ ~DrawableTranslation ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ DrawableBase* copy() const;

  void x( double x_ )
    {
      _x = x_;
    }
  double x( void ) const
    {
      return _x;
    }

  void y( double y_ )
    {
      _y = y_;
    }
  double y( void ) const
    {
      return _y;
    }

private:
  double _x;
  double _y;
};

// Set the size of the viewbox
class MagickPPExport DrawableViewbox : public DrawableBase
{
public:
  DrawableViewbox(::ssize_t x1_, ::ssize_t y1_,
                  ::ssize_t x2_, ::ssize_t y2_)
    : _x1(x1_),
      _y1(y1_),
      _x2(x2_),
      _y2(y2_) { }

  /*virtual*/ ~DrawableViewbox ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/
  DrawableBase* copy() const;

  void x1( ::ssize_t x1_ )
    {
      _x1 = x1_;
    }
  ::ssize_t x1( void ) const
    {
      return _x1;
    }

  void y1( ::ssize_t y1_ )
    {
      _y1 = y1_;
    }
  ::ssize_t y1( void ) const
    {
      return _y1;
    }

  void x2( ::ssize_t x2_ )
    {
      _x2 = x2_;
    }
  ::ssize_t x2( void ) const
    {
      return _x2;
    }

  void y2( ::ssize_t y2_ )
    {
      _y2 = y2_;
    }
  ::ssize_t y2( void ) const
    {
      return _y2;
    }

private:
  ::ssize_t _x1;
  ::ssize_t _y1;
  ::ssize_t _x2;
  ::ssize_t _y2;
};

//
// Path Element Classes To Support DrawablePath
//
class MagickPPExport PathArcArgs
{
public:
  PathArcArgs( void );

  PathArcArgs( double radiusX_, double radiusY_,
               double xAxisRotation_, bool largeArcFlag_,
               bool sweepFlag_, double x_, double y_ );

  PathArcArgs( const PathArcArgs &original_ );

  ~PathArcArgs ( void );

  void radiusX( double radiusX_ )
    {
      _radiusX = radiusX_;
    }
  double radiusX( void ) const
    {
      return _radiusX;
    }

  void radiusY( double radiusY_ )
    {
      _radiusY = radiusY_;
    }
  double radiusY( void ) const
    {
      return _radiusY;
    }

  void xAxisRotation( double xAxisRotation_ )
    {
      _xAxisRotation = xAxisRotation_;
    }
  double xAxisRotation( void ) const
    {
      return _xAxisRotation;
    }

  void largeArcFlag( bool largeArcFlag_ )
    {
      _largeArcFlag = largeArcFlag_;
    }
  bool largeArcFlag( void ) const
    {
      return _largeArcFlag;
    }

  void sweepFlag( bool sweepFlag_ )
    {
      _sweepFlag = sweepFlag_;
    }
  bool sweepFlag( void ) const
    {
      return _sweepFlag;
    }

  void x( double x_ )
    {
      _x = x_;
    }
  double x( void ) const
    {
      return _x;
    }

  void y( double y_ )
    {
      _y = y_;
    }
  double y( void ) const
    {
      return _y;
    }

private:
  double	_radiusX;	// X radius
  double	_radiusY;	// Y radius
  double	_xAxisRotation;	// Rotation relative to X axis
  bool        _largeArcFlag;	// Draw longer of the two matching arcs
  bool        _sweepFlag;	// Draw arc matching clock-wise rotation
  double	_x;		// End-point X
  double	_y;		// End-point Y
};

// Compare two PathArcArgs objects regardless of LHS/RHS
extern MagickPPExport int operator == ( const PathArcArgs& left_,
                                      const PathArcArgs& right_ );
extern MagickPPExport int operator != ( const PathArcArgs& left_,
                                      const PathArcArgs& right_ );
extern MagickPPExport int operator >  ( const PathArcArgs& left_,
                                      const PathArcArgs& right_ );
extern MagickPPExport int operator <  ( const PathArcArgs& left_,
                                      const PathArcArgs& right_ );
extern MagickPPExport int operator >= ( const PathArcArgs& left_,
                                      const PathArcArgs& right_ );
extern MagickPPExport int operator <= ( const PathArcArgs& left_,
                                      const PathArcArgs& right_ );

typedef std::vector<Magick::PathArcArgs> PathArcArgsList;

#if defined(MagickDLLExplicitTemplate)

MagickDrawableExtern template class MagickPPExport
std::allocator<Magick::PathArcArgs>;

// MagickDrawableExtern template class MagickPPExport
// std::vector<Magick::PathArcArgs, std::allocator<Magick::PathArcArgs> >;

#endif // MagickDLLExplicitTemplate

// Path Arc (Elliptical Arc)
class MagickPPExport PathArcAbs : public VPathBase
{
public:
  // Draw a single arc segment
  PathArcAbs ( const PathArcArgs &coordinates_ );

  // Draw multiple arc segments
  PathArcAbs ( const PathArcArgsList &coordinates_ );

  // Copy constructor
  PathArcAbs ( const PathArcAbs& original_ );

  // Destructor
  /*virtual*/ ~PathArcAbs ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  PathArcArgsList _coordinates;
};
class MagickPPExport PathArcRel : public VPathBase
{
public:
  // Draw a single arc segment
  PathArcRel ( const PathArcArgs &coordinates_ );

  // Draw multiple arc segments
  PathArcRel ( const PathArcArgsList &coordinates_ );

  PathArcRel ( const PathArcRel& original_ );

  /*virtual*/ ~PathArcRel ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  PathArcArgsList _coordinates;
};

// Path Closepath
class MagickPPExport PathClosePath : public VPathBase
{
public:
  PathClosePath ( void )
    : _dummy(0)
    {
    }

  /*virtual*/ ~PathClosePath ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  ::ssize_t   _dummy;
};

//
// Curveto (Cubic Bezier)
//
class MagickPPExport PathCurvetoArgs
{
public:
  PathCurvetoArgs( void );

  PathCurvetoArgs( double x1_, double y1_,
                   double x2_, double y2_,
                   double x_, double y_ );

  PathCurvetoArgs( const PathCurvetoArgs &original_ );

  ~PathCurvetoArgs ( void );

  void x1( double x1_ )
    {
      _x1 = x1_;
    }
double x1( void ) const
{
  return _x1;
}

void y1( double y1_ )
{
  _y1 = y1_;
}
double y1( void ) const
{
  return _y1;
}

void x2( double x2_ )
{
  _x2 = x2_;
}
double x2( void ) const
{
  return _x2;
}

void y2( double y2_ )
{
  _y2 = y2_;
}
double y2( void ) const
{
  return _y2;
}

void x( double x_ )
{
  _x = x_;
}
double x( void ) const
{
  return _x;
}

void y( double y_ )
{
  _y = y_;
}
double y( void ) const
{
  return _y;
}

private:
double _x1;
double _y1;
double _x2;
double _y2;
double _x;
double _y;
};

// Compare two PathCurvetoArgs objects regardless of LHS/RHS
extern MagickPPExport int operator == ( const PathCurvetoArgs& left_,
                                      const PathCurvetoArgs& right_ );
extern MagickPPExport int operator != ( const PathCurvetoArgs& left_,
                                      const PathCurvetoArgs& right_ );
extern MagickPPExport int operator >  ( const PathCurvetoArgs& left_,
                                      const PathCurvetoArgs& right_ );
extern MagickPPExport int operator <  ( const PathCurvetoArgs& left_,
                                      const PathCurvetoArgs& right_ );
extern MagickPPExport int operator >= ( const PathCurvetoArgs& left_,
                                      const PathCurvetoArgs& right_ );
extern MagickPPExport int operator <= ( const PathCurvetoArgs& left_,
                                      const PathCurvetoArgs& right_ );

typedef std::vector<Magick::PathCurvetoArgs> PathCurveToArgsList;

#if defined(MagickDLLExplicitTemplate)

MagickDrawableExtern template class MagickPPExport
std::allocator<Magick::PathCurvetoArgs>;

// MagickDrawableExtern template class MagickPPExport
// std::vector<Magick::PathCurvetoArgs, std::allocator<Magick::PathCurvetoArgs> >;

#endif // MagickDLLExplicitTemplate

class MagickPPExport PathCurvetoAbs : public VPathBase
{
public:
  // Draw a single curve
  PathCurvetoAbs ( const PathCurvetoArgs &args_ );

  // Draw multiple curves
  PathCurvetoAbs ( const PathCurveToArgsList &args_ );

  // Copy constructor
  PathCurvetoAbs ( const PathCurvetoAbs& original_ );

  // Destructor
  /*virtual*/ ~PathCurvetoAbs ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  PathCurveToArgsList _args;
};
class MagickPPExport PathCurvetoRel : public VPathBase
{
public:
  // Draw a single curve
  PathCurvetoRel ( const PathCurvetoArgs &args_ );

  // Draw multiple curves
  PathCurvetoRel ( const PathCurveToArgsList &args_ );

  // Copy constructor
  PathCurvetoRel ( const PathCurvetoRel& original_ );

  /*virtual*/ ~PathCurvetoRel ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  PathCurveToArgsList _args;
};
class MagickPPExport PathSmoothCurvetoAbs : public VPathBase
{
public:
  // Draw a single curve
  PathSmoothCurvetoAbs ( const Magick::Coordinate &coordinates_ );

  // Draw multiple curves
  PathSmoothCurvetoAbs ( const CoordinateList &coordinates_ );

  // Copy constructor
  PathSmoothCurvetoAbs ( const PathSmoothCurvetoAbs& original_ );

  /*virtual*/ ~PathSmoothCurvetoAbs ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ 
  VPathBase* copy() const;

private:
  CoordinateList _coordinates;
};
class MagickPPExport PathSmoothCurvetoRel : public VPathBase
{
public:
  // Draw a single curve
  PathSmoothCurvetoRel ( const Coordinate &coordinates_ );

  // Draw multiple curves
  PathSmoothCurvetoRel ( const CoordinateList &coordinates_ );

  // Copy constructor
  PathSmoothCurvetoRel ( const PathSmoothCurvetoRel& original_ );

  // Destructor
  /*virtual*/ ~PathSmoothCurvetoRel ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ 
  VPathBase* copy() const;

private:
  CoordinateList _coordinates;
};

//
// Quadratic Curveto (Quadratic Bezier)
//
class MagickPPExport PathQuadraticCurvetoArgs
{
public:
  PathQuadraticCurvetoArgs( void );

  PathQuadraticCurvetoArgs( double x1_, double y1_,
                            double x_, double y_ );

  PathQuadraticCurvetoArgs( const PathQuadraticCurvetoArgs &original_ );

  ~PathQuadraticCurvetoArgs ( void );

  void x1( double x1_ )
    {
      _x1 = x1_;
    }
  double x1( void ) const
    {
      return _x1;
    }

  void y1( double y1_ )
    {
      _y1 = y1_;
    }
  double y1( void ) const
    {
      return _y1;
    }

  void x( double x_ )
    {
      _x = x_;
    }
  double x( void ) const
    {
      return _x;
    }

  void y( double y_ )
    {
      _y = y_;
    }
  double y( void ) const
    {
      return _y;
    }

private:
  double _x1;
  double _y1;
  double _x;
  double _y;
};

// Compare two PathQuadraticCurvetoArgs objects regardless of LHS/RHS
extern MagickPPExport int operator == ( const PathQuadraticCurvetoArgs& left_,
                                      const PathQuadraticCurvetoArgs& right_ );
extern MagickPPExport int operator != ( const PathQuadraticCurvetoArgs& left_,
                                      const PathQuadraticCurvetoArgs& right_);
extern MagickPPExport int operator >  ( const PathQuadraticCurvetoArgs& left_,
                                      const PathQuadraticCurvetoArgs& right_);
extern MagickPPExport int operator <  ( const PathQuadraticCurvetoArgs& left_,
                                      const PathQuadraticCurvetoArgs& right_);
extern MagickPPExport int operator >= ( const PathQuadraticCurvetoArgs& left_,
                                      const PathQuadraticCurvetoArgs& right_ );
extern MagickPPExport int operator <= ( const PathQuadraticCurvetoArgs& left_,
                                      const PathQuadraticCurvetoArgs& right_ );

typedef std::vector<Magick::PathQuadraticCurvetoArgs> PathQuadraticCurvetoArgsList;

#if defined(MagickDLLExplicitTemplate)

MagickDrawableExtern template class MagickPPExport
std::allocator<Magick::PathQuadraticCurvetoArgs>;

// MagickDrawableExtern template class MagickPPExport
// std::vector<Magick::PathQuadraticCurvetoArgs, std::allocator<Magick::PathQuadraticCurvetoArgs> >;

#endif // MagickDLLExplicitTemplate

class MagickPPExport PathQuadraticCurvetoAbs : public VPathBase
{
public:
  // Draw a single curve
  PathQuadraticCurvetoAbs ( const Magick::PathQuadraticCurvetoArgs &args_ );

  // Draw multiple curves
  PathQuadraticCurvetoAbs ( const PathQuadraticCurvetoArgsList &args_ );

  // Copy constructor
  PathQuadraticCurvetoAbs ( const PathQuadraticCurvetoAbs& original_ );

  // Destructor
  /*virtual*/ ~PathQuadraticCurvetoAbs ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  PathQuadraticCurvetoArgsList _args;
};
class MagickPPExport PathQuadraticCurvetoRel : public VPathBase
{
public:
  // Draw a single curve
  PathQuadraticCurvetoRel ( const Magick::PathQuadraticCurvetoArgs &args_ );

  // Draw multiple curves
  PathQuadraticCurvetoRel ( const PathQuadraticCurvetoArgsList &args_ );

  // Copy constructor
  PathQuadraticCurvetoRel ( const PathQuadraticCurvetoRel& original_ );

  // Destructor
  /*virtual*/ ~PathQuadraticCurvetoRel ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  PathQuadraticCurvetoArgsList _args;
};
class MagickPPExport PathSmoothQuadraticCurvetoAbs : public VPathBase
{
public:
  // Draw a single curve
  PathSmoothQuadraticCurvetoAbs ( const Magick::Coordinate &coordinate_ );

  // Draw multiple curves
  PathSmoothQuadraticCurvetoAbs ( const CoordinateList &coordinates_ );

  // Copy constructor
  PathSmoothQuadraticCurvetoAbs ( const PathSmoothQuadraticCurvetoAbs& original_ );

  // Destructor
  /*virtual*/ ~PathSmoothQuadraticCurvetoAbs ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  CoordinateList _coordinates;
};
class MagickPPExport PathSmoothQuadraticCurvetoRel : public VPathBase
{
public:
  // Draw a single curve
  PathSmoothQuadraticCurvetoRel ( const Magick::Coordinate &coordinate_ );

  // Draw multiple curves
  PathSmoothQuadraticCurvetoRel ( const CoordinateList &coordinates_ );

  // Copy constructor
  PathSmoothQuadraticCurvetoRel ( const PathSmoothQuadraticCurvetoRel& original_ );

  // Destructor
  /*virtual*/ ~PathSmoothQuadraticCurvetoRel ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  CoordinateList _coordinates;
};

//
// Path Lineto
//
class MagickPPExport PathLinetoAbs : public VPathBase
{
public:
  // Draw to a single point
  PathLinetoAbs ( const Magick::Coordinate& coordinate_  );

  // Draw to multiple points
  PathLinetoAbs ( const CoordinateList &coordinates_ );

  // Copy constructor
  PathLinetoAbs ( const PathLinetoAbs& original_ );

  // Destructor
  /*virtual*/ ~PathLinetoAbs ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  CoordinateList _coordinates;
};
class MagickPPExport PathLinetoRel : public VPathBase
{
public:
  // Draw to a single point
  PathLinetoRel ( const Magick::Coordinate& coordinate_ );

  // Draw to multiple points
  PathLinetoRel ( const CoordinateList &coordinates_ );

  // Copy constructor
  PathLinetoRel ( const PathLinetoRel& original_ );

  // Destructor
  /*virtual*/ ~PathLinetoRel ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  CoordinateList _coordinates;
};

// Path Horizontal Lineto
class MagickPPExport PathLinetoHorizontalAbs : public VPathBase
{
public:
  PathLinetoHorizontalAbs ( double x_ )
    : _x(x_)
    {
    }

  /*virtual*/ ~PathLinetoHorizontalAbs ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

  void x( double x_ )
    {
      _x = x_;
    }
  double x( void ) const
    {
      return _x;
    }

private:
  double _x;
};
class MagickPPExport PathLinetoHorizontalRel : public VPathBase
{
public:
  PathLinetoHorizontalRel ( double x_ )
    : _x(x_)
    {
    }

  /*virtual*/ ~PathLinetoHorizontalRel ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

  void x( double x_ )
    {
      _x = x_;
    }
  double x( void ) const
    {
      return _x;
    }

private:
  double _x;
};

// Path Vertical Lineto
class MagickPPExport PathLinetoVerticalAbs : public VPathBase
{
public:
  PathLinetoVerticalAbs ( double y_ )
    : _y(y_)
    {
    }

  /*virtual*/ ~PathLinetoVerticalAbs ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

  void y( double y_ )
    {
      _y = y_;
    }
  double y( void ) const
    {
      return _y;
    }

private:
  double _y;
};
class MagickPPExport PathLinetoVerticalRel : public VPathBase
{
public:
  PathLinetoVerticalRel ( double y_ )
    : _y(y_)
    {
    }

  /*virtual*/ ~PathLinetoVerticalRel ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

  void y( double y_ )
    {
      _y = y_;
    }
  double y( void ) const
    {
      return _y;
    }

private:
  double _y;
};

// Path Moveto
class MagickPPExport PathMovetoAbs : public VPathBase
{
public:
  // Simple moveto
  PathMovetoAbs ( const Magick::Coordinate &coordinate_ );

  // Moveto followed by implicit linetos
  PathMovetoAbs ( const CoordinateList &coordinates_ );

  // Copy constructor
  PathMovetoAbs ( const PathMovetoAbs& original_ );

  // Destructor
  /*virtual*/ ~PathMovetoAbs ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  CoordinateList _coordinates;
};
class MagickPPExport PathMovetoRel : public VPathBase
{
public:
  // Simple moveto
  PathMovetoRel ( const Magick::Coordinate &coordinate_ );

  // Moveto followed by implicit linetos
  PathMovetoRel ( const CoordinateList &coordinates_ );

  // Copy constructor
  PathMovetoRel ( const PathMovetoRel& original_ );

  // Destructor
  /*virtual*/ ~PathMovetoRel ( void );

  // Operator to invoke equivalent draw API call
  /*virtual*/ void operator()( MagickCore::DrawingWand *context_ ) const;

  // Return polymorphic copy of object
  /*virtual*/ VPathBase* copy() const;

private:
  CoordinateList _coordinates;
};

} // namespace Magick

#endif // Magick_Drawable_header

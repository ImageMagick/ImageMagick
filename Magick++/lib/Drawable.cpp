// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Implementation of Drawable (Graphic objects)
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1
#define MAGICK_DRAWABLE_IMPLEMENTATION

#include "Magick++/Include.h"
#include <math.h>
#include <string>

#include "Magick++/Drawable.h"
#include "Magick++/Image.h"

using namespace std;

MagickPPExport int Magick::operator == ( const Magick::Coordinate& left_,
                                        const Magick::Coordinate& right_ )
{
  return ( ( left_.x() == right_.x() ) && ( left_.y() == right_.y() ) );
}
MagickPPExport int Magick::operator != ( const Magick::Coordinate& left_,
                                        const Magick::Coordinate& right_ )
{
  return ( ! (left_ == right_) );
}
MagickPPExport int Magick::operator >  ( const Magick::Coordinate& left_,
                                        const Magick::Coordinate& right_ )
{
  return ( !( left_ < right_ ) && ( left_ != right_ ) );
}
MagickPPExport int Magick::operator <  ( const Magick::Coordinate& left_,
                                        const Magick::Coordinate& right_ )
{
  // Based on distance from origin
  return  ( (sqrt(left_.x()*left_.x() + left_.y()*left_.y())) <
            (sqrt(right_.x()*right_.x() + right_.y()*right_.y())) );
}
MagickPPExport int Magick::operator >= ( const Magick::Coordinate& left_,
                                        const Magick::Coordinate& right_ )
{
  return ( ( left_ > right_ ) || ( left_ == right_ ) );
}
MagickPPExport int Magick::operator <= ( const Magick::Coordinate& left_,
                                        const Magick::Coordinate& right_ )
{
  return ( ( left_ < right_ ) || ( left_ == right_ ) );
}

/*virtual*/
Magick::DrawableBase::~DrawableBase ( void )
{
}

// Constructor
Magick::Drawable::Drawable ( void )
  : dp(0)
{
}

// Construct from DrawableBase
Magick::Drawable::Drawable ( const Magick::DrawableBase& original_ )
  : dp(original_.copy())
{
}

// Destructor
Magick::Drawable::~Drawable ( void )
{
  delete dp;
  dp = 0;
}

// Copy constructor
Magick::Drawable::Drawable ( const Magick::Drawable& original_ )
  : dp(original_.dp? original_.dp->copy(): 0)
{
}

// Assignment operator
Magick::Drawable& Magick::Drawable::operator= (const Magick::Drawable& original_ )
{
  if (this != &original_)
    {
      DrawableBase* temp_dp = (original_.dp ? original_.dp->copy() : 0);
      delete dp;
      dp = temp_dp;
    }
  return *this;
}

// Operator to invoke contained object
void Magick::Drawable::operator()( MagickCore::DrawingWand * context_ ) const
{
  if(dp)
    dp->operator()( context_ );
}

MagickPPExport int Magick::operator == ( const Magick::Drawable& /*left_*/,
                                        const Magick::Drawable& /*right_*/ )
{
  return ( 1 );
}
MagickPPExport int Magick::operator != ( const Magick::Drawable& /*left_*/,
                                        const Magick::Drawable& /*right_*/ )
{
  return ( 0 );
}
MagickPPExport int Magick::operator > ( const Magick::Drawable& /*left_*/,
                                       const Magick::Drawable& /*right_*/ )
{
  return ( 0 );
}
MagickPPExport int Magick::operator <  ( const Magick::Drawable& /*left_*/,
                                        const Magick::Drawable& /*right_*/ )
{
  return  ( 0 );
}
MagickPPExport int Magick::operator >= ( const Magick::Drawable& left_,
                                        const Magick::Drawable& right_ )
{
  return ( ( left_ > right_ ) || ( left_ == right_ ) );
}
MagickPPExport int Magick::operator <= ( const Magick::Drawable& left_,
                                        const Magick::Drawable& right_ )
{
  return ( ( left_ < right_ ) || ( left_ == right_ ) );
}

/*virtual*/
Magick::VPathBase::~VPathBase ( void )
{
}

// Constructor
Magick::VPath::VPath ( void )
  : dp(0)
{
}

// Construct from VPathBase
Magick::VPath::VPath ( const Magick::VPathBase& original_ )
  : dp(original_.copy())
{
}

// Destructor
/* virtual */ Magick::VPath::~VPath ( void )
{
  delete dp;
  dp = 0;
}

// Copy constructor
Magick::VPath::VPath ( const Magick::VPath& original_ )
  : dp(original_.dp? original_.dp->copy(): 0)
{
}

// Assignment operator
Magick::VPath& Magick::VPath::operator= (const Magick::VPath& original_ )
{
  if (this != &original_)
    {
      VPathBase* temp_dp = (original_.dp ? original_.dp->copy() : 0);
      delete dp;
      dp = temp_dp;
    }
  return *this;
}

// Operator to invoke contained object
void Magick::VPath::operator()( MagickCore::DrawingWand * context_ ) const
{
  if(dp)
    dp->operator()( context_ );
}

MagickPPExport int Magick::operator == ( const Magick::VPath& /*left_*/,
                                        const Magick::VPath& /*right_*/ )
{
  return ( 1 );
}
MagickPPExport int Magick::operator != ( const Magick::VPath& /*left_*/,
                                        const Magick::VPath& /*right_*/ )
{
  return ( 0 );
}
MagickPPExport int Magick::operator > ( const Magick::VPath& /*left_*/,
                                       const Magick::VPath& /*right_*/ )
{
  return ( 0 );
}
MagickPPExport int Magick::operator <  ( const Magick::VPath& /*left_*/,
                                        const Magick::VPath& /*right_*/ )
{
  return  ( 0 );
}
MagickPPExport int Magick::operator >= ( const Magick::VPath& left_,
                                        const Magick::VPath& right_ )
{
  return ( ( left_ > right_ ) || ( left_ == right_ ) );
}
MagickPPExport int Magick::operator <= ( const Magick::VPath& left_,
                                        const Magick::VPath& right_ )
{
  return ( ( left_ < right_ ) || ( left_ == right_ ) );
}

//
// Drawable Objects
//

// Affine (scaling, rotation, and translation)
Magick::DrawableAffine::DrawableAffine( double sx_, double sy_,
                                        double rx_, double ry_,
                                        double tx_, double ty_ )
{
  _affine.sx = sx_;
  _affine.rx = rx_;
  _affine.ry = ry_;
  _affine.sy = sy_;
  _affine.tx = tx_;
  _affine.ty = ty_;
}
Magick::DrawableAffine::DrawableAffine( void )
{
  GetAffineMatrix(&_affine);
}
Magick::DrawableAffine::~DrawableAffine( void )
{
}
void Magick::DrawableAffine::operator()( MagickCore::DrawingWand * context_ ) const
{
  DrawAffine( context_, &_affine );
}
Magick::DrawableBase* Magick::DrawableAffine::copy() const
{
  return new DrawableAffine(*this);
}

// Arc
Magick::DrawableArc::~DrawableArc( void )
{
}
void Magick::DrawableArc::operator()( MagickCore::DrawingWand * context_ ) const
{
  DrawArc( context_, _startX, _startY, _endX, _endY, _startDegrees, _endDegrees );
}
Magick::DrawableBase* Magick::DrawableArc::copy() const
{
  return new DrawableArc(*this);
}

//
// Bezier curve
//
// Construct from coordinates (Coordinate list must contain at least three members)
Magick::DrawableBezier::DrawableBezier ( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
// Copy constructor
Magick::DrawableBezier::DrawableBezier( const Magick::DrawableBezier& original_ )
  : DrawableBase (original_),
    _coordinates(original_._coordinates)
{
}
// Destructor
Magick::DrawableBezier::~DrawableBezier( void )
{
}
void Magick::DrawableBezier::operator()( MagickCore::DrawingWand * context_ ) const
{
  size_t num_coords = (size_t) _coordinates.size();
  PointInfo *coordinates = new PointInfo[num_coords];

  PointInfo *q = coordinates;
  CoordinateList::const_iterator p = _coordinates.begin();

  while( p != _coordinates.end() )
    {
      q->x = p->x();
      q->y = p->y();
      q++;
      p++;
    }

  DrawBezier( context_, num_coords, coordinates );
  delete [] coordinates;
}
Magick::DrawableBase* Magick::DrawableBezier::copy() const
{
  return new DrawableBezier(*this);
}

//
//Clip Path 
//

// Pop (terminate) Clip path definition
Magick::DrawablePopClipPath::~DrawablePopClipPath ( void )
{
}
void Magick::DrawablePopClipPath::operator() ( MagickCore::DrawingWand * context_ ) const
{
  DrawPopClipPath( context_ );
  DrawPopDefs(context_);
}
Magick::DrawableBase* Magick::DrawablePopClipPath::copy() const
{
  return new DrawablePopClipPath(*this);
}

// Push clip path definition
Magick::DrawablePushClipPath::DrawablePushClipPath( const std::string &id_)
  : _id(id_.c_str())    //multithread safe const char*
{
}
Magick::DrawablePushClipPath::DrawablePushClipPath
( const Magick::DrawablePushClipPath& original_ ) //multithread safe const char*
  : DrawableBase (original_),
    _id(original_._id.c_str())
{
}
Magick::DrawablePushClipPath::~DrawablePushClipPath( void )
{
}
void Magick::DrawablePushClipPath::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawPushDefs(context_);
  DrawPushClipPath( context_, _id.c_str());
}
Magick::DrawableBase* Magick::DrawablePushClipPath::copy() const
{
  return new DrawablePushClipPath(*this);
}
//
// ClipPath
//
Magick::DrawableClipPath::DrawableClipPath( const std::string &id_ )
:_id(id_.c_str())
{
}

Magick::DrawableClipPath::DrawableClipPath ( const Magick::DrawableClipPath& original_ )
  : DrawableBase (original_),
    _id(original_._id.c_str())
{
}
Magick::DrawableClipPath::~DrawableClipPath( void )
{
}
void Magick::DrawableClipPath::operator()( MagickCore::DrawingWand * context_ ) const
{
	(void) DrawSetClipPath( context_, _id.c_str());
}
Magick::DrawableBase* Magick::DrawableClipPath::copy() const
{
  return new DrawableClipPath(*this);
}

// Circle
Magick::DrawableCircle::~DrawableCircle ( void )
{
}
void Magick::DrawableCircle::operator()( MagickCore::DrawingWand * context_ ) const
{
  DrawCircle( context_, _originX, _originY, _perimX, _perimY );
}
Magick::DrawableBase* Magick::DrawableCircle::copy() const
{
  return new DrawableCircle(*this);
}

// Colorize at point using PaintMethod
Magick::DrawableColor::~DrawableColor( void )
{
}
void Magick::DrawableColor::operator()( MagickCore::DrawingWand * context_ ) const
{
  DrawColor( context_, _x, _y, _paintMethod );
}
Magick::DrawableBase* Magick::DrawableColor::copy() const
{
  return new DrawableColor(*this);
}

// Draw image at point
Magick::DrawableCompositeImage::DrawableCompositeImage
( double x_, double y_,
  double width_, double height_,
  const std::string &filename_,
  Magick::CompositeOperator composition_ )
  : _composition(composition_),
    _x(x_),
    _y(y_),
    _width(width_),
    _height(height_),
    _image(new Image(filename_))
{
}
Magick::DrawableCompositeImage::DrawableCompositeImage
( double x_, double y_,
  double width_, double height_,
  const Magick::Image &image_,
  Magick::CompositeOperator composition_ )
  : _composition(composition_),
    _x(x_),
    _y(y_),
    _width(width_),
    _height(height_),
    _image(new Image(image_))
{
}
Magick::DrawableCompositeImage::DrawableCompositeImage
( double x_, double y_,
  double width_, double height_,
  const std::string &filename_ )
  :_composition(CopyCompositeOp),
   _x(x_),
   _y(y_),
   _width(width_),
   _height(height_),
   _image(new Image(filename_))
{
}
Magick::DrawableCompositeImage::DrawableCompositeImage
( double x_, double y_,
  double width_, double height_,
  const Magick::Image &image_ )
  :_composition(CopyCompositeOp),
   _x(x_),
   _y(y_),
   _width(width_),
   _height(height_),
   _image(new Image(image_))
{
}
Magick::DrawableCompositeImage::DrawableCompositeImage
( double x_, double y_,
  const std::string &filename_ )
  : _composition(CopyCompositeOp),
    _x(x_),
    _y(y_),
    _width(0),
    _height(0),
    _image(new Image(filename_))
{
  _width=_image->columns();
  _height=_image->rows();
}
Magick::DrawableCompositeImage::DrawableCompositeImage
( double x_, double y_,
  const Magick::Image &image_ )
  : _composition(CopyCompositeOp),
    _x(x_),
    _y(y_),
    _width(0),
    _height(0),
    _image(new Image(image_))
{
  _width=_image->columns();
  _height=_image->rows();
}
// Copy constructor
Magick::DrawableCompositeImage::DrawableCompositeImage
( const Magick::DrawableCompositeImage& original_ )
  :  Magick::DrawableBase(original_),
     _composition(original_._composition),
     _x(original_._x),
     _y(original_._y),
     _width(original_._width),
     _height(original_._height),
     _image(new Image(*original_._image))
{
}
Magick::DrawableCompositeImage::~DrawableCompositeImage( void )
{
  delete _image;
}
// Assignment operator
Magick::DrawableCompositeImage& Magick::DrawableCompositeImage::operator=
(const Magick::DrawableCompositeImage& original_ )
{
  // If not being set to ourself
  if ( this != &original_ )
    {
      _composition = original_._composition;
      _x = original_._x;
      _y = original_._y;
      _width = original_._width;
      _height = original_._height;
      Image* temp_image = new Image(*original_._image);
      delete _image;
      _image = temp_image;
    }
  return *this;
}
void Magick::DrawableCompositeImage::filename( const std::string &filename_ )
{
  Image* temp_image = new Image(filename_);
  delete _image;
  _image = temp_image;
}
std::string Magick::DrawableCompositeImage::filename( void ) const
{
  return _image->fileName();
}

void Magick::DrawableCompositeImage::image( const Magick::Image &image_ )
{
  Image* temp_image = new Image(image_);
  delete _image;
  _image = temp_image;
}
Magick::Image Magick::DrawableCompositeImage::image( void ) const
{
  return *_image;
}

// Specify image format used to output Base64 inlined image data.
void Magick::DrawableCompositeImage::magick( std::string magick_ )
{
  _image->magick( magick_ );
}
std::string Magick::DrawableCompositeImage::magick( void )
{
  return _image->magick();
}

void Magick::DrawableCompositeImage::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  MagickWand
    *magick_wand;

  magick_wand=NewMagickWandFromImage(_image->constImage());
  (void) DrawComposite( context_, _composition, _x, _y, _width, _height,
     magick_wand );
  magick_wand=DestroyMagickWand(magick_wand);
}

Magick::DrawableBase* Magick::DrawableCompositeImage::copy() const
{
  return new DrawableCompositeImage(*this);
}

// Ellipse
Magick::DrawableEllipse::~DrawableEllipse( void )
{
}
void Magick::DrawableEllipse::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawEllipse( context_, _originX, _originY, _radiusX, _radiusY,
               _arcStart, _arcEnd );
}
Magick::DrawableBase* Magick::DrawableEllipse::copy() const
{
  return new DrawableEllipse(*this);
}

// Specify drawing fill color
Magick::DrawableFillColor::DrawableFillColor( const Magick::Color &color_ )
  : _color(color_)
{
}
Magick::DrawableFillColor::DrawableFillColor
( const Magick::DrawableFillColor& original_ )
  : DrawableBase (original_),
    _color(original_._color)
{
}
Magick::DrawableFillColor::~DrawableFillColor( void )
{
}
void Magick::DrawableFillColor::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  PixelPacket color = static_cast<PixelPacket>(_color);
  PixelWand *pixel_wand=NewPixelWand();
  PixelSetQuantumColor(pixel_wand,&color);
  DrawSetFillColor(context_,pixel_wand);
  pixel_wand=DestroyPixelWand(pixel_wand);
}
Magick::DrawableBase* Magick::DrawableFillColor::copy() const
{
  return new DrawableFillColor(*this);
}

// Specify drawing fill fule
Magick::DrawableFillRule::~DrawableFillRule ( void )
{
}
void Magick::DrawableFillRule::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetFillRule( context_, _fillRule );
}
Magick::DrawableBase* Magick::DrawableFillRule::copy() const
{
  return new DrawableFillRule(*this);
}

// Specify drawing fill opacity
Magick::DrawableFillOpacity::~DrawableFillOpacity ( void )
{
}
void Magick::DrawableFillOpacity::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetFillOpacity( context_, _opacity );
}
Magick::DrawableBase* Magick::DrawableFillOpacity::copy() const
{
  return new DrawableFillOpacity(*this);
}

// Specify text font
Magick::DrawableFont::DrawableFont ( const std::string &font_ )
  : _font(font_),
    _family(),
    _style(Magick::AnyStyle),
    _weight(400),
    _stretch(Magick::NormalStretch)
{
}
Magick::DrawableFont::DrawableFont ( const std::string &family_,
                                     Magick::StyleType style_,
                                     const unsigned int weight_,
                                     Magick::StretchType stretch_ )
  : _font(),
    _family(family_),
    _style(style_),
    _weight(weight_),
    _stretch(stretch_)
{
}
Magick::DrawableFont::DrawableFont ( const Magick::DrawableFont& original_ )
  : DrawableBase (original_),
    _font(original_._font),
    _family(original_._family),
    _style(original_._style),
    _weight(original_._weight),
    _stretch(original_._stretch)
{
}
Magick::DrawableFont::~DrawableFont ( void )
{
}
void Magick::DrawableFont::operator()( MagickCore::DrawingWand * context_ ) const
{
  // font
  if(_font.length())
    {
      (void) DrawSetFont( context_, _font.c_str() );
    }

  if(_family.length())
    {
      // font-family
      (void) DrawSetFontFamily( context_, _family.c_str() );

      // font-style
      DrawSetFontStyle( context_, _style );

      // font-weight
      DrawSetFontWeight( context_, _weight );

      // font-stretch
      DrawSetFontStretch( context_, _stretch );
    }
}
Magick::DrawableBase* Magick::DrawableFont::copy() const
{
  return new DrawableFont(*this);
}

// Specify text positioning gravity
Magick::DrawableGravity::~DrawableGravity ( void )
{
}
void Magick::DrawableGravity::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetGravity( context_, _gravity );
}
Magick::DrawableBase* Magick::DrawableGravity::copy() const
{
  return new DrawableGravity(*this);
}

// Line
Magick::DrawableLine::~DrawableLine ( void )
{
}
void Magick::DrawableLine::operator()( MagickCore::DrawingWand * context_ ) const
{
  DrawLine( context_, _startX, _startY, _endX, _endY );
}
Magick::DrawableBase* Magick::DrawableLine::copy() const
{
  return new DrawableLine(*this);
}

// Change pixel matte value to transparent using PaintMethod
Magick::DrawableMatte::~DrawableMatte ( void )
{
}
void Magick::DrawableMatte::operator()( MagickCore::DrawingWand * context_ ) const
{
  DrawMatte( context_, _x, _y, _paintMethod );
}
Magick::DrawableBase* Magick::DrawableMatte::copy() const
{
  return new DrawableMatte(*this);
}

// Drawable Path
Magick::DrawablePath::DrawablePath ( const VPathList &path_ )
  : _path(path_)
{
}
Magick::DrawablePath::DrawablePath ( const Magick::DrawablePath& original_ )
  : DrawableBase (original_),
    _path(original_._path)
{
}
Magick::DrawablePath::~DrawablePath ( void )
{
}
void Magick::DrawablePath::operator()( MagickCore::DrawingWand * context_ ) const
{
  DrawPathStart( context_ );

  for( VPathList::const_iterator p = _path.begin();
       p != _path.end(); p++ )
    p->operator()( context_ ); // FIXME, how to quit loop on error?

  DrawPathFinish( context_ );
}
Magick::DrawableBase* Magick::DrawablePath::copy() const
{
  return new DrawablePath(*this);
}

// Point
Magick::DrawablePoint::~DrawablePoint ( void )
{
}
void Magick::DrawablePoint::operator()( MagickCore::DrawingWand * context_ ) const
{
  DrawPoint( context_, _x, _y );
}
Magick::DrawableBase* Magick::DrawablePoint::copy() const
{
  return new DrawablePoint(*this);
}

// Text pointsize
Magick::DrawablePointSize::~DrawablePointSize ( void )
{
}
void Magick::DrawablePointSize::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetFontSize( context_, _pointSize );
}
Magick::DrawableBase* Magick::DrawablePointSize::copy() const
{
  return new DrawablePointSize(*this);
}

// Polygon (Coordinate list must contain at least three members)
Magick::DrawablePolygon::DrawablePolygon ( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::DrawablePolygon::DrawablePolygon
( const Magick::DrawablePolygon& original_ )
  : DrawableBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::DrawablePolygon::~DrawablePolygon ( void )
{
}
void Magick::DrawablePolygon::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  size_t num_coords = (size_t) _coordinates.size();
  PointInfo *coordinates = new PointInfo[num_coords];

  PointInfo *q = coordinates;
  CoordinateList::const_iterator p = _coordinates.begin();

  while( p != _coordinates.end() )
    {
      q->x = p->x();
      q->y = p->y();
      q++;
      p++;
    }

  DrawPolygon( context_, num_coords, coordinates );
  delete [] coordinates;
}
Magick::DrawableBase* Magick::DrawablePolygon::copy() const
{
  return new DrawablePolygon(*this);
}

// Polyline (Coordinate list must contain at least three members)
Magick::DrawablePolyline::DrawablePolyline
( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::DrawablePolyline::DrawablePolyline
( const Magick::DrawablePolyline& original_ )
  : DrawableBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::DrawablePolyline::~DrawablePolyline ( void )
{
}
void Magick::DrawablePolyline::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  size_t num_coords = (size_t) _coordinates.size();
  PointInfo *coordinates = new PointInfo[num_coords];

  PointInfo *q = coordinates;
  CoordinateList::const_iterator p = _coordinates.begin();

  while( p != _coordinates.end() )
    {
      q->x = p->x();
      q->y = p->y();
      q++;
      p++;
    }

  DrawPolyline( context_, num_coords, coordinates );
  delete [] coordinates;
}
Magick::DrawableBase* Magick::DrawablePolyline::copy() const
{
  return new DrawablePolyline(*this);
}

// Pop Graphic Context
Magick::DrawablePopGraphicContext::~DrawablePopGraphicContext ( void )
{
}
void Magick::DrawablePopGraphicContext::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  PopDrawingWand( context_ );
}
Magick::DrawableBase* Magick::DrawablePopGraphicContext::copy() const
{
  return new DrawablePopGraphicContext(*this);
}

// Push Graphic Context
Magick::DrawablePushGraphicContext::~DrawablePushGraphicContext ( void )
{
}
void Magick::DrawablePushGraphicContext::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  PushDrawingWand( context_ );
}
Magick::DrawableBase* Magick::DrawablePushGraphicContext::copy() const
{
  return new DrawablePushGraphicContext(*this);
}

// Pop (terminate) Pattern definition
Magick::DrawablePopPattern::~DrawablePopPattern ( void )
{
}
void Magick::DrawablePopPattern::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  (void) DrawPopPattern( context_ );
}
Magick::DrawableBase* Magick::DrawablePopPattern::copy() const
{
  return new DrawablePopPattern(*this);
}

// Push Pattern definition
Magick::DrawablePushPattern::DrawablePushPattern
( const std::string &id_, ssize_t x_, ssize_t y_,
  size_t width_, size_t height_ )
  : _id(id_),
    _x(x_),
    _y(y_),
    _width(width_),
    _height(height_)
{
}
Magick::DrawablePushPattern::DrawablePushPattern
( const Magick::DrawablePushPattern& original_ )
  : DrawableBase (original_),
    _id(original_._id),
    _x(original_._x),
    _y(original_._y),
    _width(original_._width),
    _height(original_._height)
{
}
Magick::DrawablePushPattern::~DrawablePushPattern ( void )
{
}
void Magick::DrawablePushPattern::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  (void) DrawPushPattern( context_, _id.c_str(), _x, _y, _width, _height );
}
Magick::DrawableBase* Magick::DrawablePushPattern::copy() const
{
  return new DrawablePushPattern(*this);
}

// Rectangle
Magick::DrawableRectangle::~DrawableRectangle ( void )
{
}
void Magick::DrawableRectangle::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawRectangle( context_, _upperLeftX, _upperLeftY,
                 _lowerRightX, _lowerRightY );
}
Magick::DrawableBase* Magick::DrawableRectangle::copy() const
{
  return new DrawableRectangle(*this);
}

// Apply Rotation
Magick::DrawableRotation::~DrawableRotation ( void )
{
}
void Magick::DrawableRotation::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawRotate( context_, _angle );
}
Magick::DrawableBase* Magick::DrawableRotation::copy() const
{
  return new DrawableRotation(*this);
}

// Round Rectangle
Magick::DrawableRoundRectangle::~DrawableRoundRectangle ( void )
{
}
void Magick::DrawableRoundRectangle::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawRoundRectangle( context_, _centerX,_centerY, _width,_hight,
                      _cornerWidth, _cornerHeight);
}
Magick::DrawableBase* Magick::DrawableRoundRectangle::copy() const
{
  return new DrawableRoundRectangle(*this);
}

// Apply Scaling
Magick::DrawableScaling::~DrawableScaling ( void )
{
}
void Magick::DrawableScaling::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawScale( context_, _x, _y );
}
Magick::DrawableBase* Magick::DrawableScaling::copy() const
{
  return new DrawableScaling(*this);
}

// Apply Skew in the X direction
Magick::DrawableSkewX::~DrawableSkewX ( void )
{
}
void Magick::DrawableSkewX::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSkewX( context_, _angle );
}
Magick::DrawableBase* Magick::DrawableSkewX::copy() const
{
  return new DrawableSkewX(*this);
}

// Apply Skew in the Y direction
Magick::DrawableSkewY::~DrawableSkewY ( void )
{
}
void Magick::DrawableSkewY::operator()( MagickCore::DrawingWand * context_ ) const
{
  DrawSkewY( context_, _angle );
}
Magick::DrawableBase* Magick::DrawableSkewY::copy() const
{
  return new DrawableSkewY(*this);
}

// Stroke dasharray
Magick::DrawableDashArray::DrawableDashArray( const double* dasharray_ )
  : _size(0),
    _dasharray(0)
{
  dasharray( dasharray_ );
}
// Deprecated, do not use for new code, and migrate existing code to
// using double*
Magick::DrawableDashArray::DrawableDashArray( const size_t* dasharray_ )
  : _size(0),
    _dasharray(0)
{
  dasharray( dasharray_ );
}
Magick::DrawableDashArray::DrawableDashArray
(const Magick::DrawableDashArray& original_)
  : DrawableBase (original_),
    _size(0),
    _dasharray(0)
{
  dasharray( original_._dasharray );
}
Magick::DrawableDashArray::~DrawableDashArray( void )
{
  delete [] _dasharray;
  _size = 0;
  _dasharray = 0;
}
Magick::DrawableDashArray& Magick::DrawableDashArray::operator=
(const Magick::DrawableDashArray &original_)
{
  if( this != &original_ )
    {
      dasharray( original_._dasharray );
    }
  return *this;
}
void Magick::DrawableDashArray::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  (void) DrawSetStrokeDashArray( context_, (size_t) _size, _dasharray );
}
Magick::DrawableBase* Magick::DrawableDashArray::copy() const
{
  return new DrawableDashArray(*this);
}
void Magick::DrawableDashArray::dasharray ( const double* dasharray_ )
{
  _dasharray=(double *) RelinquishMagickMemory(_dasharray);

  if(dasharray_)
    {
      // Count elements in dash array
      size_t n = 0;
      {
        const double *p = dasharray_;
        while(*p++ != 0)
          n++;
      }
      _size = n;

      // Allocate elements
      _dasharray=static_cast<double*>(AcquireMagickMemory((n+1)*sizeof(double)));
      // Copy elements
      {
        double *q = _dasharray;
        const double *p = dasharray_;
        while( *p )
          *q++=*p++;
        *q=0;
      }
    }
}
// This method is deprecated.  Don't use for new code, and migrate existing
// code to the const double* version.
void Magick::DrawableDashArray::dasharray( const size_t* dasharray_ )
{
  _dasharray=(double *) RelinquishMagickMemory(_dasharray);

  if(dasharray_)
    {
      // Count elements in dash array
      size_t n = 0;
      {
        const size_t *p = dasharray_;
        while(*p++ != 0)
          n++;
      }
      _size = n;

      // Allocate elements
      _dasharray=static_cast<double*>(AcquireMagickMemory((n+1)*sizeof(double)));
      // Copy elements
      {
        double *q = _dasharray;
        const size_t *p = dasharray_;
        while( *p )
          *q++=static_cast<double>(*p++);
        *q=0;
      }
    }
}

// Stroke dashoffset
Magick::DrawableDashOffset::~DrawableDashOffset ( void )
{
}
void Magick::DrawableDashOffset::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetStrokeDashOffset( context_, _offset );
}
Magick::DrawableBase* Magick::DrawableDashOffset::copy() const
{
  return new DrawableDashOffset(*this);
}

// Stroke linecap
Magick::DrawableStrokeLineCap::~DrawableStrokeLineCap ( void )
{
}
void Magick::DrawableStrokeLineCap::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetStrokeLineCap( context_, _linecap );
}
Magick::DrawableBase* Magick::DrawableStrokeLineCap::copy() const
{
  return new DrawableStrokeLineCap(*this);
}

// Stroke linejoin
Magick::DrawableStrokeLineJoin::~DrawableStrokeLineJoin ( void )
{
}
void Magick::DrawableStrokeLineJoin::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetStrokeLineJoin( context_, _linejoin );
}
Magick::DrawableBase* Magick::DrawableStrokeLineJoin::copy() const
{
  return new DrawableStrokeLineJoin(*this);
}

// Stroke miterlimit
Magick::DrawableMiterLimit::~DrawableMiterLimit ( void )
{
}
void Magick::DrawableMiterLimit::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetStrokeMiterLimit( context_, _miterlimit );
}
Magick::DrawableBase* Magick::DrawableMiterLimit::copy() const
{
  return new DrawableMiterLimit(*this);
}

// Stroke antialias
Magick::DrawableStrokeAntialias::~DrawableStrokeAntialias ( void )
{
}
void Magick::DrawableStrokeAntialias::operator()
( MagickCore::DrawingWand * context_ ) const
{
  DrawSetStrokeAntialias( context_, static_cast<MagickBooleanType>
    (_flag ? MagickTrue : MagickFalse) );
}
Magick::DrawableBase* Magick::DrawableStrokeAntialias::copy() const
{
  return new DrawableStrokeAntialias(*this);
}

// Stroke color
Magick::DrawableStrokeColor::DrawableStrokeColor
( const Magick::Color &color_ )
  : _color(color_)
{
}
Magick::DrawableStrokeColor::DrawableStrokeColor
( const Magick::DrawableStrokeColor& original_ )
  : DrawableBase (original_),
    _color(original_._color)
{
}
Magick::DrawableStrokeColor::~DrawableStrokeColor ( void )
{
}
void Magick::DrawableStrokeColor::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  PixelPacket color = static_cast<PixelPacket>(_color);
  PixelWand *pixel_wand=NewPixelWand();
  PixelSetQuantumColor(pixel_wand,&color);
  DrawSetStrokeColor(context_,pixel_wand);
  pixel_wand=DestroyPixelWand(pixel_wand);
}
Magick::DrawableBase* Magick::DrawableStrokeColor::copy() const
{
  return new DrawableStrokeColor(*this);
}

// Stroke opacity
Magick::DrawableStrokeOpacity::~DrawableStrokeOpacity ( void )
{
}
void Magick::DrawableStrokeOpacity::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetStrokeOpacity( context_, _opacity );
}
Magick::DrawableBase* Magick::DrawableStrokeOpacity::copy() const
{
  return new DrawableStrokeOpacity(*this);
}

// Stroke width
Magick::DrawableStrokeWidth::~DrawableStrokeWidth ( void )
{
}
void Magick::DrawableStrokeWidth::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetStrokeWidth( context_, _width );
}
Magick::DrawableBase* Magick::DrawableStrokeWidth::copy() const
{
  return new DrawableStrokeWidth(*this);
}

// Draw text at point
Magick::DrawableText::DrawableText ( const double x_, const double y_,
                                     const std::string &text_ )
  : _x(x_),
    _y(y_),
    _text(text_),
    _encoding()
{
}
Magick::DrawableText::DrawableText ( const double x_, const double y_,
                                     const std::string &text_,  const std::string &encoding_)
  : _x(x_),
    _y(y_),
    _text(text_),
    _encoding(encoding_)
{
}
Magick::DrawableText::DrawableText( const Magick::DrawableText& original_ )
  : DrawableBase (original_),
    _x(original_._x),
    _y(original_._y),
    _text(original_._text),
    _encoding(original_._encoding)
{
}
Magick::DrawableText::~DrawableText ( void )
{
}
void Magick::DrawableText::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetTextEncoding( context_, _encoding.c_str() );
  DrawAnnotation( context_, _x, _y,
                  reinterpret_cast<const unsigned char*>(_text.c_str()) );
}
Magick::DrawableBase* Magick::DrawableText::copy() const
{
  return new DrawableText(*this);
}

// Text antialias
Magick::DrawableTextAntialias::DrawableTextAntialias ( bool flag_ )
  : _flag(flag_)
{
}
Magick::DrawableTextAntialias::DrawableTextAntialias( const Magick::DrawableTextAntialias &original_ )
  : DrawableBase (original_),
    _flag(original_._flag)
{
}
Magick::DrawableTextAntialias::~DrawableTextAntialias ( void )
{
}
void Magick::DrawableTextAntialias::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetTextAntialias( context_, static_cast<MagickBooleanType>
    (_flag ? MagickTrue : MagickFalse) );
}
Magick::DrawableBase* Magick::DrawableTextAntialias::copy() const
{
  return new DrawableTextAntialias(*this);
}

// Decoration (text decoration)
Magick::DrawableTextDecoration::DrawableTextDecoration
  ( Magick::DecorationType decoration_ )
    : _decoration(decoration_)
{
}
Magick::DrawableTextDecoration::DrawableTextDecoration
  ( const Magick::DrawableTextDecoration &original_ )
    : DrawableBase (original_),
      _decoration(original_._decoration)
{
}
Magick::DrawableTextDecoration::~DrawableTextDecoration( void )
{
}
void Magick::DrawableTextDecoration::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetTextDecoration( context_, _decoration );
}
Magick::DrawableBase* Magick::DrawableTextDecoration::copy() const
{
  return new DrawableTextDecoration(*this);
}

// Set text undercolor
Magick::DrawableTextUnderColor::DrawableTextUnderColor
( const Magick::Color &color_ )
  : _color(color_)
{
}
Magick::DrawableTextUnderColor::DrawableTextUnderColor
( const Magick::DrawableTextUnderColor& original_ )
  : DrawableBase (original_),
    _color(original_._color)
{
}
Magick::DrawableTextUnderColor::~DrawableTextUnderColor ( void )
{
}
void Magick::DrawableTextUnderColor::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  PixelPacket color = static_cast<PixelPacket>(_color);
  PixelWand *pixel_wand=NewPixelWand();
  PixelSetQuantumColor(pixel_wand,&color);
  DrawSetTextUnderColor(context_,pixel_wand);
  pixel_wand=DestroyPixelWand(pixel_wand);
}
Magick::DrawableBase* Magick::DrawableTextUnderColor::copy() const
{
  return new DrawableTextUnderColor(*this);
}

// Apply Translation
Magick::DrawableTranslation::~DrawableTranslation ( void )
{
}
void Magick::DrawableTranslation::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawTranslate( context_, _x, _y );
}
Magick::DrawableBase* Magick::DrawableTranslation::copy() const
{
  return new DrawableTranslation(*this);
}

// Set the size of the viewbox
Magick::DrawableViewbox::~DrawableViewbox ( void )
{
}
void Magick::DrawableViewbox::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawSetViewbox( context_, _x1, _y1, _x2, _y2 );
}
Magick::DrawableBase* Magick::DrawableViewbox::copy() const
{
  return new DrawableViewbox(*this);
}

//
// Path Classes
//

//
// PathArcArgs
//
MagickPPExport int Magick::operator == ( const Magick::PathArcArgs& /*left_*/,
                                        const Magick::PathArcArgs& /*right_*/ )
{
  return ( 1 );
}
MagickPPExport int Magick::operator != ( const Magick::PathArcArgs& /*left_*/,
                                        const Magick::PathArcArgs& /*right_*/ )
{
  return ( 0 );
}
MagickPPExport int Magick::operator > ( const Magick::PathArcArgs& /*left_*/,
                                       const Magick::PathArcArgs& /*right_*/ )
{
  return ( 0 );
}
MagickPPExport int Magick::operator <  ( const Magick::PathArcArgs& /*left_*/,
                                        const Magick::PathArcArgs& /*right_*/ )
{
  return  ( false );
}
MagickPPExport int Magick::operator >= ( const Magick::PathArcArgs& left_,
                                        const Magick::PathArcArgs& right_ )
{
  return ( ( left_ > right_ ) || ( left_ == right_ ) );
}
MagickPPExport int Magick::operator <= ( const Magick::PathArcArgs& left_,
                                        const Magick::PathArcArgs& right_ )
{
  return ( ( left_ < right_ ) || ( left_ == right_ ) );
}
// Default constructor
Magick::PathArcArgs::PathArcArgs( void )
  :  _radiusX(0),
     _radiusY(0),
     _xAxisRotation(0),
     _largeArcFlag(false),
     _sweepFlag(false),
     _x(0),
     _y(0)
{
}
// Normal constructor
Magick::PathArcArgs::PathArcArgs( double radiusX_, double radiusY_,
                                  double xAxisRotation_, bool largeArcFlag_,
                                  bool sweepFlag_, double x_, double y_ )
  : _radiusX(radiusX_),
    _radiusY(radiusY_),
    _xAxisRotation(xAxisRotation_),
    _largeArcFlag(largeArcFlag_),
    _sweepFlag(sweepFlag_),
    _x(x_),
    _y(y_)
{
}
// Copy constructor
Magick::PathArcArgs::PathArcArgs( const Magick::PathArcArgs &original_ )
  :  _radiusX(original_._radiusX),
     _radiusY(original_._radiusY),
     _xAxisRotation(original_._xAxisRotation),
     _largeArcFlag(original_._largeArcFlag),
     _sweepFlag(original_._sweepFlag),
     _x(original_._x),
     _y(original_._y)
{
}
// Destructor
Magick::PathArcArgs::~PathArcArgs ( void )
{
}

// Path Arc
Magick::PathArcAbs::PathArcAbs ( const Magick::PathArcArgs &coordinates_ )
  : _coordinates(1,coordinates_)
{
}
Magick::PathArcAbs::PathArcAbs ( const PathArcArgsList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::PathArcAbs::PathArcAbs ( const Magick::PathArcAbs& original_ )
  : VPathBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::PathArcAbs::~PathArcAbs ( void )
{
}
void Magick::PathArcAbs::operator()( MagickCore::DrawingWand * context_ ) const
{
  for( PathArcArgsList::const_iterator p = _coordinates.begin();
       p != _coordinates.end(); p++ )
    {
      DrawPathEllipticArcAbsolute( context_, p->radiusX(), p->radiusY(),
                                   p->xAxisRotation(), (MagickBooleanType) p->largeArcFlag(),
                                   (MagickBooleanType) p->sweepFlag(), p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathArcAbs::copy() const
{
  return new PathArcAbs(*this);
}

Magick::PathArcRel::PathArcRel ( const Magick::PathArcArgs &coordinates_ )
  : _coordinates(1,coordinates_)
{
}
Magick::PathArcRel::PathArcRel ( const PathArcArgsList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::PathArcRel::PathArcRel ( const Magick::PathArcRel& original_ )
  : VPathBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::PathArcRel::~PathArcRel ( void )
{
}
void Magick::PathArcRel::operator()( MagickCore::DrawingWand * context_ ) const
{
  for( PathArcArgsList::const_iterator p = _coordinates.begin();
       p != _coordinates.end(); p++ )
    {
      DrawPathEllipticArcRelative( context_, p->radiusX(), p->radiusY(),
                                   p->xAxisRotation(), (MagickBooleanType) p->largeArcFlag(),
                                   (MagickBooleanType) p->sweepFlag(), p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathArcRel::copy() const
{
  return new PathArcRel(*this);
}

//
// Path Closepath
//
Magick::PathClosePath::~PathClosePath ( void )
{
}
void Magick::PathClosePath::operator()( MagickCore::DrawingWand * context_ ) const
{
  DrawPathClose( context_ );
}
Magick::VPathBase* Magick::PathClosePath::copy() const
{
  return new PathClosePath(*this);
}

//
// Path Curveto (Cubic Bezier)
//
MagickPPExport int Magick::operator == ( const Magick::PathCurvetoArgs& /*left_*/,
                                        const Magick::PathCurvetoArgs& /*right_*/ )
{
  return ( 1 );
}
MagickPPExport int Magick::operator != ( const Magick::PathCurvetoArgs& /*left_*/,
                                        const Magick::PathCurvetoArgs& /*right_*/ )
{
  return ( 0 );
}
MagickPPExport int Magick::operator > ( const Magick::PathCurvetoArgs& /*left_*/,
                                       const Magick::PathCurvetoArgs& /*right_*/ )
{
  return ( 0 );
}
MagickPPExport int Magick::operator <  ( const Magick::PathCurvetoArgs& /*left_*/,
                                        const Magick::PathCurvetoArgs& /*right_*/ )
{
  return  ( false );
}
MagickPPExport int Magick::operator >= ( const Magick::PathCurvetoArgs& left_,
                                        const Magick::PathCurvetoArgs& right_ )
{
  return ( ( left_ > right_ ) || ( left_ == right_ ) );
}
MagickPPExport int Magick::operator <= ( const Magick::PathCurvetoArgs& left_,
                                        const Magick::PathCurvetoArgs& right_ )
{
  return ( ( left_ < right_ ) || ( left_ == right_ ) );
}
// Default constructor
Magick::PathCurvetoArgs::PathCurvetoArgs( void )
  : _x1(0),
    _y1(0),
    _x2(0),
    _y2(0),
    _x(0),
    _y(0)
{
}
// Normal constructor
Magick::PathCurvetoArgs::PathCurvetoArgs( double x1_, double y1_,
                                          double x2_, double y2_,
                                          double x_, double y_ )
  : _x1(x1_),
    _y1(y1_),
    _x2(x2_),
    _y2(y2_),
    _x(x_),
    _y(y_)
{
}
// Copy constructor
Magick::PathCurvetoArgs::PathCurvetoArgs( const PathCurvetoArgs &original_ )
  : _x1(original_._x1),
    _y1(original_._y1),
    _x2(original_._x2),
    _y2(original_._y2),
    _x(original_._x),
    _y(original_._y)
{
}
// Destructor
Magick::PathCurvetoArgs::~PathCurvetoArgs ( void )
{
}

Magick::PathCurvetoAbs::PathCurvetoAbs ( const Magick::PathCurvetoArgs &args_ )
  : _args(1,args_)
{
}
Magick::PathCurvetoAbs::PathCurvetoAbs ( const PathCurveToArgsList &args_ )
  : _args(args_)
{
}
Magick::PathCurvetoAbs::PathCurvetoAbs
 ( const Magick::PathCurvetoAbs& original_ )
   : VPathBase (original_),
     _args(original_._args)
{
}
Magick::PathCurvetoAbs::~PathCurvetoAbs ( void )
{
}
void Magick::PathCurvetoAbs::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  for( PathCurveToArgsList::const_iterator p = _args.begin();
       p != _args.end(); p++ )
    {
      DrawPathCurveToAbsolute( context_, p->x1(), p->y1(), p->x2(), p->y2(),
                               p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathCurvetoAbs::copy() const
{
  return new PathCurvetoAbs(*this);
}
Magick::PathCurvetoRel::PathCurvetoRel ( const Magick::PathCurvetoArgs &args_ )
  : _args(1,args_)
{
}
Magick::PathCurvetoRel::PathCurvetoRel ( const PathCurveToArgsList &args_ )
  : _args(args_)
{
}
Magick::PathCurvetoRel::PathCurvetoRel
( const Magick::PathCurvetoRel& original_ )
  : VPathBase (original_),
    _args(original_._args)
{
}
Magick::PathCurvetoRel::~PathCurvetoRel ( void )
{
}
void Magick::PathCurvetoRel::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  for( PathCurveToArgsList::const_iterator p = _args.begin();
       p != _args.end(); p++ )
    {
      DrawPathCurveToRelative( context_, p->x1(), p->y1(), p->x2(), p->y2(),
                               p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathCurvetoRel::copy() const
{
  return new PathCurvetoRel(*this);
}
Magick::PathSmoothCurvetoAbs::PathSmoothCurvetoAbs
( const Magick::Coordinate &coordinates_ )
  : _coordinates(1,coordinates_)
{
}
Magick::PathSmoothCurvetoAbs::PathSmoothCurvetoAbs
( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::PathSmoothCurvetoAbs::PathSmoothCurvetoAbs
( const Magick::PathSmoothCurvetoAbs& original_ )
  : VPathBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::PathSmoothCurvetoAbs::~PathSmoothCurvetoAbs ( void )
{
}
void Magick::PathSmoothCurvetoAbs::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  for( CoordinateList::const_iterator p = _coordinates.begin();
       p != _coordinates.end(); p++ )
    {
      double x2 = p->x();
      double y2 = p->y();
      p++;
      if(p != _coordinates.end() )
        DrawPathCurveToSmoothAbsolute( context_, x2, y2, p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathSmoothCurvetoAbs::copy() const
{
  return new PathSmoothCurvetoAbs(*this);
}
Magick::PathSmoothCurvetoRel::PathSmoothCurvetoRel
( const Magick::Coordinate &coordinates_ )
  : _coordinates(1,coordinates_)
{
}
Magick::PathSmoothCurvetoRel::PathSmoothCurvetoRel
( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::PathSmoothCurvetoRel::PathSmoothCurvetoRel
( const Magick::PathSmoothCurvetoRel& original_ )
  : VPathBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::PathSmoothCurvetoRel::~PathSmoothCurvetoRel ( void )
{
}
void Magick::PathSmoothCurvetoRel::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  for( CoordinateList::const_iterator p = _coordinates.begin();
       p != _coordinates.end(); p++ )
    {
      double x2 = p->x();
      double y2 = p->y();
      p++;
      if(p != _coordinates.end() )
        DrawPathCurveToSmoothRelative( context_, x2, y2, p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathSmoothCurvetoRel::copy() const
{
  return new PathSmoothCurvetoRel(*this);
}

//
// Quadratic Curveto (Quadratic Bezier)
//
MagickPPExport int Magick::operator ==
( const Magick::PathQuadraticCurvetoArgs& /*left_*/,
  const Magick::PathQuadraticCurvetoArgs& /*right_*/ )
{
  return ( 1 );
}
MagickPPExport int Magick::operator !=
( const Magick::PathQuadraticCurvetoArgs& /*left_*/,
  const Magick::PathQuadraticCurvetoArgs& /*right_*/ )
{
  return ( 0 );
}
MagickPPExport int Magick::operator >
( const Magick::PathQuadraticCurvetoArgs& /*left_*/,
  const Magick::PathQuadraticCurvetoArgs& /*right_*/ )
{
  return ( 0 );
}
MagickPPExport int Magick::operator < 
( const Magick::PathQuadraticCurvetoArgs& /*left_*/,
  const Magick::PathQuadraticCurvetoArgs& /*right_*/ )
{
  return  ( 0 );
}
MagickPPExport int Magick::operator >=
( const Magick::PathQuadraticCurvetoArgs& left_,
  const Magick::PathQuadraticCurvetoArgs& right_ )
{
  return ( ( left_ > right_ ) || ( left_ == right_ ) );
}
MagickPPExport int Magick::operator <=
( const Magick::PathQuadraticCurvetoArgs& left_,
  const Magick::PathQuadraticCurvetoArgs& right_ )
{
  return ( ( left_ < right_ ) || ( left_ == right_ ) );
}
// Default Constructor
Magick::PathQuadraticCurvetoArgs::PathQuadraticCurvetoArgs( void )
  : _x1(0),
    _y1(0),
    _x(0),
    _y(0)
{
}
// Normal Constructor
Magick::PathQuadraticCurvetoArgs::PathQuadraticCurvetoArgs( double x1_,
                                                            double y1_,
                                                            double x_,
                                                            double y_ )
  : _x1(x1_),
    _y1(y1_),
    _x(x_),
    _y(y_)
{
}
// Copy Constructor
Magick::PathQuadraticCurvetoArgs::PathQuadraticCurvetoArgs( const PathQuadraticCurvetoArgs &original_ )
  : _x1(original_._x1),
    _y1(original_._y1),
    _x(original_._x),
    _y(original_._y)
{
}
// Destructor
Magick::PathQuadraticCurvetoArgs::~PathQuadraticCurvetoArgs ( void )
{
}

Magick::PathQuadraticCurvetoAbs::PathQuadraticCurvetoAbs
( const Magick::PathQuadraticCurvetoArgs &args_ )
  : _args(1,args_)
{
}
Magick::PathQuadraticCurvetoAbs::PathQuadraticCurvetoAbs 
( const PathQuadraticCurvetoArgsList &args_ )
  : _args(args_)
{
}
Magick::PathQuadraticCurvetoAbs::PathQuadraticCurvetoAbs
( const Magick::PathQuadraticCurvetoAbs& original_ )
  : VPathBase (original_),
    _args(original_._args)
{
}
Magick::PathQuadraticCurvetoAbs::~PathQuadraticCurvetoAbs ( void )
{
}
void Magick::PathQuadraticCurvetoAbs::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  for( PathQuadraticCurvetoArgsList::const_iterator p = _args.begin();
       p != _args.end(); p++ )
    {
      DrawPathCurveToQuadraticBezierAbsolute( context_, p->x1(), p->y1(),
                                              p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathQuadraticCurvetoAbs::copy() const
{
  return new PathQuadraticCurvetoAbs(*this);
}
Magick::PathQuadraticCurvetoRel::PathQuadraticCurvetoRel
( const Magick::PathQuadraticCurvetoArgs &args_ )
  : _args(1,args_)
{
}
Magick::PathQuadraticCurvetoRel::PathQuadraticCurvetoRel
( const PathQuadraticCurvetoArgsList &args_ )
  : _args(args_)
{
}
Magick::PathQuadraticCurvetoRel::PathQuadraticCurvetoRel
( const Magick::PathQuadraticCurvetoRel& original_ )
  : VPathBase (original_),
    _args(original_._args)
{
}
Magick::PathQuadraticCurvetoRel::~PathQuadraticCurvetoRel ( void )
{
}
void Magick::PathQuadraticCurvetoRel::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  for( PathQuadraticCurvetoArgsList::const_iterator p = _args.begin();
       p != _args.end(); p++ )
    {
      DrawPathCurveToQuadraticBezierRelative( context_, p->x1(), p->y1(),
                                              p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathQuadraticCurvetoRel::copy() const
{
  return new PathQuadraticCurvetoRel(*this);
}
Magick::PathSmoothQuadraticCurvetoAbs::PathSmoothQuadraticCurvetoAbs
( const Magick::Coordinate &coordinate_ )
  : _coordinates(1,coordinate_)
{
}
Magick::PathSmoothQuadraticCurvetoAbs::PathSmoothQuadraticCurvetoAbs
( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::PathSmoothQuadraticCurvetoAbs::PathSmoothQuadraticCurvetoAbs
( const Magick::PathSmoothQuadraticCurvetoAbs& original_ )
  : VPathBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::PathSmoothQuadraticCurvetoAbs::~PathSmoothQuadraticCurvetoAbs ( void )
{
}
void Magick::PathSmoothQuadraticCurvetoAbs::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  for( CoordinateList::const_iterator p = _coordinates.begin();
       p != _coordinates.end(); p++ )
    {
      DrawPathCurveToQuadraticBezierSmoothAbsolute( context_, p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathSmoothQuadraticCurvetoAbs::copy() const
{
  return new PathSmoothQuadraticCurvetoAbs(*this);
}
Magick::PathSmoothQuadraticCurvetoRel::PathSmoothQuadraticCurvetoRel
( const Magick::Coordinate &coordinate_ )
  : _coordinates(1,coordinate_)
{
}
Magick::PathSmoothQuadraticCurvetoRel::PathSmoothQuadraticCurvetoRel
( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::PathSmoothQuadraticCurvetoRel::PathSmoothQuadraticCurvetoRel
( const PathSmoothQuadraticCurvetoRel& original_ )
  : VPathBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::PathSmoothQuadraticCurvetoRel::~PathSmoothQuadraticCurvetoRel ( void )
{
}
void Magick::PathSmoothQuadraticCurvetoRel::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  for( CoordinateList::const_iterator p = _coordinates.begin();
       p != _coordinates.end(); p++ )
    {
      DrawPathCurveToQuadraticBezierSmoothRelative( context_, p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathSmoothQuadraticCurvetoRel::copy() const
{
  return new PathSmoothQuadraticCurvetoRel(*this);
}

//
// Path Lineto
//
Magick::PathLinetoAbs::PathLinetoAbs ( const Magick::Coordinate& coordinate_  )
  : _coordinates(1,coordinate_)
{
}
Magick::PathLinetoAbs::PathLinetoAbs ( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::PathLinetoAbs::PathLinetoAbs ( const Magick::PathLinetoAbs& original_ )
  : VPathBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::PathLinetoAbs::~PathLinetoAbs ( void )
{
}
void Magick::PathLinetoAbs::operator()( MagickCore::DrawingWand * context_ ) const
{
  for( CoordinateList::const_iterator p = _coordinates.begin();
       p != _coordinates.end(); p++ )
    {
      DrawPathLineToAbsolute( context_, p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathLinetoAbs::copy() const
{
  return new PathLinetoAbs(*this);
}
Magick::PathLinetoRel::PathLinetoRel ( const Magick::Coordinate& coordinate_  )
  : _coordinates(1,coordinate_)
{
}
Magick::PathLinetoRel::PathLinetoRel ( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::PathLinetoRel::PathLinetoRel ( const Magick::PathLinetoRel& original_ )
  : VPathBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::PathLinetoRel::~PathLinetoRel ( void )
{
}
void Magick::PathLinetoRel::operator()( MagickCore::DrawingWand * context_ ) const
{
  for( CoordinateList::const_iterator p = _coordinates.begin();
       p != _coordinates.end(); p++ )
    {
      DrawPathLineToRelative( context_, p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathLinetoRel::copy() const
{
  return new PathLinetoRel(*this);
}

//
// Path Horizontal Lineto
//

Magick::PathLinetoHorizontalAbs::~PathLinetoHorizontalAbs ( void )
{
}
void Magick::PathLinetoHorizontalAbs::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawPathLineToHorizontalAbsolute( context_, _x );
}
Magick::VPathBase* Magick::PathLinetoHorizontalAbs::copy() const
{
  return new PathLinetoHorizontalAbs(*this);
}
Magick::PathLinetoHorizontalRel::~PathLinetoHorizontalRel ( void )
{
}
void Magick::PathLinetoHorizontalRel::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawPathLineToHorizontalRelative( context_, _x );
}
Magick::VPathBase* Magick::PathLinetoHorizontalRel::copy() const
{
  return new PathLinetoHorizontalRel(*this);
}

//
// Path Vertical Lineto
//
Magick::PathLinetoVerticalAbs::~PathLinetoVerticalAbs ( void )
{
}
void Magick::PathLinetoVerticalAbs::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawPathLineToVerticalAbsolute( context_, _y );
}
Magick::VPathBase* Magick::PathLinetoVerticalAbs::copy() const
{
  return new PathLinetoVerticalAbs(*this);
}
Magick::PathLinetoVerticalRel::~PathLinetoVerticalRel ( void )
{
}
void Magick::PathLinetoVerticalRel::operator()
  ( MagickCore::DrawingWand * context_ ) const
{
  DrawPathLineToVerticalRelative( context_, _y );
}
Magick::VPathBase* Magick::PathLinetoVerticalRel::copy() const
{
  return new PathLinetoVerticalRel(*this);
}

//
// Path Moveto
//

Magick::PathMovetoAbs::PathMovetoAbs ( const Magick::Coordinate &coordinate_ )
  : _coordinates(1,coordinate_)
{
}
Magick::PathMovetoAbs::PathMovetoAbs ( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::PathMovetoAbs::PathMovetoAbs ( const Magick::PathMovetoAbs& original_ )
  : VPathBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::PathMovetoAbs::~PathMovetoAbs ( void )
{
}
void Magick::PathMovetoAbs::operator()( MagickCore::DrawingWand * context_ ) const
{
  for( CoordinateList::const_iterator p = _coordinates.begin();
       p != _coordinates.end(); p++ )
    {
      DrawPathMoveToAbsolute( context_, p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathMovetoAbs::copy() const
{
  return new PathMovetoAbs(*this);
}
Magick::PathMovetoRel::PathMovetoRel ( const Magick::Coordinate &coordinate_ )
  : _coordinates(1,coordinate_)
{
}
Magick::PathMovetoRel::PathMovetoRel ( const CoordinateList &coordinates_ )
  : _coordinates(coordinates_)
{
}
Magick::PathMovetoRel::PathMovetoRel ( const Magick::PathMovetoRel& original_ )
  : VPathBase (original_),
    _coordinates(original_._coordinates)
{
}
Magick::PathMovetoRel::~PathMovetoRel ( void )
{
}
void Magick::PathMovetoRel::operator()( MagickCore::DrawingWand * context_ ) const
{
  for( CoordinateList::const_iterator p = _coordinates.begin();
       p != _coordinates.end(); p++ )
    {
      DrawPathMoveToRelative( context_, p->x(), p->y() );
    }
}
Magick::VPathBase* Magick::PathMovetoRel::copy() const
{
  return new PathMovetoRel(*this);
}

#if defined(EXPLICIT_TEMPLATE_INSTANTIATION)
// template class std::list<Magick::Coordinate>;
// template class std::list<const Magick::Drawable>;
// template class std::list<const Magick::PathArcArgs>;
// template class std::list<const Magick::PathCurvetoArgs>;
// template class std::list<const Magick::PathQuadraticCurvetoArgs>;
// template class std::list<const Magick::VPath>;
#endif

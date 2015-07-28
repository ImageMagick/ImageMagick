// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
// Copyright Dirk Lemstra 2014-2015
//
// Geometry implementation
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include <string>
#include <ctype.h> // for isdigit
#if !defined(MAGICKCORE_WINDOWS_SUPPORT)
#include <strings.h>
#endif

using namespace std;

#include "Magick++/Geometry.h"
#include "Magick++/Exception.h"

MagickPPExport int Magick::operator == (const Magick::Geometry& left_,
  const Magick::Geometry& right_)
{
  return((left_.aspect() == right_.aspect()) &&
    (left_.fillArea() == right_.fillArea()) &&
    (left_.greater() == right_.greater()) &&
    (left_.height() == right_.height()) &&
    (left_.isValid() == right_.isValid()) &&
    (left_.less() == right_.less()) &&
    (left_.limitPixels() == right_.limitPixels()) &&
    (left_.percent() == right_.percent()) &&
    (left_.width() == right_.width()) &&
    (left_.xOff() == right_.xOff()) &&
    (left_.yOff() == right_.yOff()));
}

MagickPPExport int Magick::operator != (const Magick::Geometry& left_,
  const Magick::Geometry& right_)
{
  return(!(left_ == right_));
}

MagickPPExport int Magick::operator > (const Magick::Geometry& left_,
  const Magick::Geometry& right_)
{
  return(!(left_ < right_) && (left_ != right_));
}

MagickPPExport int Magick::operator < (const Magick::Geometry& left_,
  const Magick::Geometry& right_)
{
  return((left_.width()*left_.height()) < (right_.width()*right_.height()));
}

MagickPPExport int Magick::operator >= (const Magick::Geometry& left_,
  const Magick::Geometry& right_)
{
  return((left_ > right_) || (left_ == right_));
}

MagickPPExport int Magick::operator <= (const Magick::Geometry& left_,
  const Magick::Geometry& right_ )
{
  return((left_ < right_) || (left_ == right_));
}

Magick::Geometry::Geometry(void)
  : _width(0),
    _height(0),
    _xOff(0),
    _yOff(0),
    _isValid(false),
    _percent(false),
    _aspect(false),
    _greater(false),
    _less(false),
    _fillArea(false),
    _limitPixels(false)
{
}

Magick::Geometry::Geometry(const char *geometry_)
  : _width(0),
    _height(0),
    _xOff(0),
    _yOff(0),
    _isValid(false),
    _percent(false),
    _aspect(false),
    _greater(false),
    _less(false),
    _fillArea(false),
    _limitPixels(false)
{
  *this=geometry_; // Use assignment operator
}

Magick::Geometry::Geometry(const Geometry &geometry_)
  : _width(geometry_._width),
    _height(geometry_._height),
    _xOff(geometry_._xOff),
    _yOff(geometry_._yOff),
    _isValid(geometry_._isValid),
    _percent(geometry_._percent),
    _aspect(geometry_._aspect),
    _greater(geometry_._greater),
    _less(geometry_._less),
    _fillArea(geometry_._fillArea),
    _limitPixels(geometry_._limitPixels)
{
}

Magick::Geometry::Geometry(const std::string &geometry_)
  : _width(0),
    _height(0),
    _xOff(0),
    _yOff(0),
    _isValid(false),
    _percent(false),
    _aspect(false),
    _greater(false),
    _less(false),
    _fillArea(false),
    _limitPixels(false)
{
  *this=geometry_; // Use assignment operator
}

Magick::Geometry::Geometry(size_t width_,size_t height_,ssize_t xOff_,
  ssize_t yOff_)
  : _width(width_),
    _height(height_),
    _xOff(xOff_),
    _yOff(yOff_),
    _isValid(true),
    _percent(false),
    _aspect(false),
    _greater(false),
    _less(false),
    _fillArea(false),
    _limitPixels(false)
{
}

Magick::Geometry::~Geometry(void)
{
}

const Magick::Geometry& Magick::Geometry::operator=(const char *geometry_)
{
  *this=std::string(geometry_);
  return(*this);
}

Magick::Geometry& Magick::Geometry::operator=(const Geometry &geometry_)
{
  // If not being set to ourself
  if (this != &geometry_)
    {
      _width=geometry_._width;
      _height=geometry_._height;
      _xOff=geometry_._xOff;
      _yOff=geometry_._yOff;
      _isValid=geometry_._isValid;
      _percent=geometry_._percent;
      _aspect=geometry_._aspect;
      _greater=geometry_._greater;
      _less=geometry_._less;
      _fillArea=geometry_._fillArea;
      _limitPixels=geometry_._limitPixels;
    }
  return(*this);
}

const Magick::Geometry& Magick::Geometry::operator=(
  const std::string &geometry_)
{
  char
    geom[MagickPathExtent];

  char
    *pageptr;

  ssize_t
    flags,
    x = 0,
    y = 0;

  size_t
    height_val=0,
    width_val=0;

  // If argument does not start with digit, presume that it is a
  // page-size specification that needs to be converted to an
  // equivalent geometry specification using PostscriptGeometry()
  (void) CopyMagickString(geom,geometry_.c_str(),MagickPathExtent);
  if (geom[0] != '-' && geom[0] != '+' && geom[0] != 'x' &&
      !isdigit(static_cast<int>(geom[0])))
    {
      pageptr=GetPageGeometry(geom);
      if (pageptr != 0)
        {
          (void) CopyMagickString(geom,pageptr,MagickPathExtent);
          pageptr=(char *) RelinquishMagickMemory(pageptr);
        }
    }

  flags=GetGeometry(geom,&x,&y,&width_val,&height_val);

  if (flags == NoValue)
    {
      // Total failure!
      *this=Geometry();
      isValid(false);
      return(*this);
    }

  if ((flags & WidthValue) != 0)
    {
      _width=width_val;
      isValid(true);
    }

  if ((flags & HeightValue) != 0)
    {
      _height=height_val;
      isValid(true);
    }

  if ((flags & XValue) != 0)
    {
      _xOff=static_cast<ssize_t>(x);
      isValid(true);
    }

  if ((flags & YValue) != 0)
    {
      _yOff=static_cast<ssize_t>(y);
      isValid(true);
    }

  if ((flags & PercentValue) != 0)
    _percent=true;

  if ((flags & AspectValue) != 0)
    _aspect=true;

  if ((flags & LessValue) != 0)
    _less=true;

  if ((flags & GreaterValue) != 0)
    _greater=true;

  if ((flags & MinimumValue) != 0)
    _fillArea=true;

  if ((flags & AreaValue) != 0)
    _limitPixels=true;

  return(*this);
}

Magick::Geometry::operator std::string() const
{
  char
    buffer[MagickPathExtent];

  std::string
    geometry;

  if (!isValid())
    throwExceptionExplicit(MagickCore::OptionError,
      "Invalid geometry argument");

  if (_width)
    {
      FormatLocaleString(buffer,MagickPathExtent,"%.20g",(double) _width);
      geometry+=buffer;
    }

  if (_height)
    {
      FormatLocaleString(buffer,MagickPathExtent,"%.20g",(double) _height);
      geometry+='x';
      geometry+=buffer;
    }

  if (_xOff || _yOff)
    {
      if (_xOff >= 0)
        geometry+='+';

      FormatLocaleString(buffer,MagickPathExtent,"%.20g",(double) _xOff);
      geometry+=buffer;

      if (_yOff >= 0)
        geometry+='+';

      FormatLocaleString(buffer,MagickPathExtent,"%.20g",(double) _yOff);
      geometry+=buffer;
    }

  if (_percent)
    geometry+='%';

  if (_aspect)
    geometry+='!';

  if (_greater)
    geometry+='>';

  if (_less)
    geometry+='<';

  if (_fillArea)
    geometry+='^';

  if (_limitPixels)
    geometry+='@';

  return(geometry);
}

void Magick::Geometry::aspect(bool aspect_)
{
  _aspect=aspect_;
}

bool Magick::Geometry::aspect(void) const
{
  return(_aspect);
}

void Magick::Geometry::fillArea(bool fillArea_)
{
  _fillArea=fillArea_;
}

bool Magick::Geometry::fillArea(void) const
{
  return(_fillArea);
}

void Magick::Geometry::greater(bool greater_)
{
  _greater=greater_;
}

bool Magick::Geometry::greater(void) const
{
  return(_greater);
}

void Magick::Geometry::height(size_t height_)
{
  _height=height_;
}

size_t Magick::Geometry::height(void) const
{
  return(_height);
}

void Magick::Geometry::isValid(bool isValid_)
{
  _isValid=isValid_;
}

bool Magick::Geometry::isValid(void) const
{
  return(_isValid);
}

void Magick::Geometry::less(bool less_)
{
  _less=less_;
}

bool Magick::Geometry::less(void) const
{
  return(_less);
}

void Magick::Geometry::limitPixels(bool limitPixels_)
{
  _limitPixels=limitPixels_;
}

bool Magick::Geometry::limitPixels(void) const
{
  return(_limitPixels);
}

void Magick::Geometry::width(size_t width_)
{
  _width=width_;
  isValid(true);
}

void Magick::Geometry::percent(bool percent_)
{
  _percent = percent_;
}

bool Magick::Geometry::percent(void) const
{
  return(_percent);
}

size_t Magick::Geometry::width(void) const
{
  return(_width);
}

void Magick::Geometry::xOff(::ssize_t xOff_)
{
  _xOff=xOff_;
}

::ssize_t Magick::Geometry::xOff(void) const
{
  return(_xOff);
}

inline void Magick::Geometry::yOff(::ssize_t yOff_)
{
  _yOff=yOff_;
}

::ssize_t Magick::Geometry::yOff(void) const
{
  return(_yOff);
}

Magick::Geometry::Geometry(const MagickCore::RectangleInfo &rectangle_)
  : _width(static_cast<size_t>(rectangle_.width)),
    _height(static_cast<size_t>(rectangle_.height)),
    _xOff(static_cast<ssize_t>(rectangle_.x)),
    _yOff(static_cast<ssize_t>(rectangle_.y)),
    _isValid(true),
    _percent(false),
    _aspect(false),
    _greater(false),
    _less(false),
    _fillArea(false),
    _limitPixels(false)
{
}

const Magick::Geometry& Magick::Geometry::operator=(
  const MagickCore::RectangleInfo &rectangle_)
{
  _width=static_cast<size_t>(rectangle_.width),
  _height=static_cast<size_t>(rectangle_.height),
  _xOff=static_cast<ssize_t>(rectangle_.x),
  _yOff=static_cast<ssize_t>(rectangle_.y),
  _isValid=true;
  return(*this);
}

Magick::Geometry::operator MagickCore::RectangleInfo() const
{
  RectangleInfo rectangle;
  rectangle.width=_width;
  rectangle.height=_height;
  rectangle.x=_xOff;
  rectangle.y=_yOff;
  return(rectangle);
}

MagickPPExport int Magick::operator == (const Magick::Offset& left_,
  const Magick::Offset& right_)
{
  return((left_.x() == right_.x()) &&
    (left_.y() == right_.y()));
}

MagickPPExport int Magick::operator != (const Magick::Offset& left_,
  const Magick::Offset& right_)
{
  return(!(left_ == right_));
}

Magick::Offset::Offset(void)
  : _x(0),
    _y(0)
{
}

Magick::Offset::Offset(const char *offset_)
  : _x(0),
    _y(0)
{
  *this=offset_; // Use assignment operator
}

Magick::Offset::Offset(const Offset &offset_)
  : _x(offset_._x),
    _y(offset_._y)
{
}

Magick::Offset::Offset(const std::string &offset_)
  : _x(0),
    _y(0)
{
  *this=offset_; // Use assignment operator
}

Magick::Offset::Offset(ssize_t x_,ssize_t y_)
  : _x(x_),
    _y(y_)
{
}

Magick::Offset::~Offset(void)
{
}

const Magick::Offset& Magick::Offset::operator=(const char *offset_)
{
  MagickCore::GeometryInfo
    geometry_info;

  MagickCore::MagickStatusType
    flags;

  flags=ParseGeometry(offset_,&geometry_info);
  _x=geometry_info.rho;
  _y=geometry_info.sigma;
  if ((flags & MagickCore::SigmaValue) == 0)
    _y=_x;
  return(*this);
}

Magick::Offset& Magick::Offset::operator=(const Offset &offset_)
{
  // If not being set to ourself
  if (this != &offset_)
    {
      _x=offset_._x;
      _y=offset_._y;
    }
  return(*this);
}

const Magick::Offset& Magick::Offset::operator=(const std::string &offset_)
{
  *this=offset_.c_str();
  return(*this);
}

ssize_t Magick::Offset::x(void) const
{
  return(_x);
}

ssize_t Magick::Offset::y(void) const
{
  return(_y);
}

Magick::Offset::operator MagickCore::OffsetInfo() const
{
  OffsetInfo offset;
  offset.x=_x;
  offset.y=_y;
  return(offset);
}

MagickPPExport int Magick::operator == (const Magick::Point& left_,
  const Magick::Point& right_)
{
  return((left_.x() == right_.x()) &&
    (left_.y() == right_.y()));
}

MagickPPExport int Magick::operator != (const Magick::Point& left_,
  const Magick::Point& right_)
{
  return(!(left_ == right_));
}

Magick::Point::Point(void)
  : _x(0.0),
    _y(0.0)
{
}

Magick::Point::Point(const char *point_)
  : _x(0.0),
    _y(0.0)
{
  *this=point_; // Use assignment operator
}

Magick::Point::Point(const Point &point_)
  : _x(point_._x),
    _y(point_._y)
{
}

Magick::Point::Point(const std::string &point_)
  : _x(0.0),
    _y(0.0)
{
  *this=point_; // Use assignment operator
}

Magick::Point::Point(double x_,double y_)
  : _x(x_),
    _y(y_)
{
}

Magick::Point::Point(double xy_)
  : _x(xy_),
    _y(xy_)
{
}

Magick::Point::~Point(void)
{
}

const Magick::Point& Magick::Point::operator=(const char *point_)
{
  MagickCore::GeometryInfo
    geometry_info;

  MagickCore::MagickStatusType
    flags;

  flags=ParseGeometry(point_,&geometry_info);
  _x=geometry_info.rho;
  _y=geometry_info.sigma;
  if ((flags & MagickCore::SigmaValue) == 0)
    _y=_x;
  return(*this);
}

const Magick::Point& Magick::Point::operator=(const double xy_)
{
  _x=xy_;
  _y=xy_;
  return(*this);
}

Magick::Point& Magick::Point::operator=(const Point &point_)
{
  // If not being set to ourself
  if (this != &point_)
    {
      _x=point_._x;
      _y=point_._y;
    }
  return(*this);
}

const Magick::Point& Magick::Point::operator=(const std::string &point_)
{
  *this=point_.c_str();
  return(*this);
}

Magick::Point::operator std::string() const
{
  char
    buffer[MagickPathExtent];

  string
    point;

  if (_x < 0.0)
    point+="-";
  else
    point+="+";

  FormatLocaleString(buffer,MagickPathExtent,"%.20g",_x);
  point+=buffer;

  if (_y < 0.0)
    point+="x-";
  else
    point+="x+";

  FormatLocaleString(buffer,MagickPathExtent,"%.20g",(double) _y);
  point+=buffer;

  return(point);
}

bool Magick::Point::isValid(void) const
{
  return(_x > 0.0);
}

double Magick::Point::x(void) const
{
  return(_x);
}

double Magick::Point::y(void) const
{
  return(_y);
}
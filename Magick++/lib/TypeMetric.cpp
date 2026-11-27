// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001
//
// Copyright @ 2014 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// TypeMetric implementation
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/TypeMetric.h"
#include "Magick++/Drawable.h"
#include <limits>

namespace
{
  // Convert a floating point extent to size_t without invoking undefined
  // behavior: negative, NaN and out-of-range values are clamped.
  size_t extentToSize(const double value)
  {
    if (!(value > 0.0))
      return(0);
    if (value >= static_cast<double>(std::numeric_limits<size_t>::max()))
      return(std::numeric_limits<size_t>::max());
    return(static_cast<size_t>(value));
  }
}

Magick::TypeMetric::TypeMetric(void)
  : _typeMetric()
{
}

Magick::TypeMetric::~TypeMetric(void)
{
}

double Magick::TypeMetric::ascent(void) const
{
  return(_typeMetric.ascent);
}

Magick::Geometry Magick::TypeMetric::bounds(void) const
{
  return(Geometry(extentToSize(_typeMetric.bounds.x2-_typeMetric.bounds.x1),
    extentToSize(_typeMetric.bounds.y2-_typeMetric.bounds.y1),
    static_cast<ssize_t>(_typeMetric.bounds.x1),
    static_cast<ssize_t>(_typeMetric.bounds.y1)));
}

double Magick::TypeMetric::descent(void) const
{
  return(_typeMetric.descent);
}

double Magick::TypeMetric::maxHorizontalAdvance(void) const
{
  return(_typeMetric.max_advance);
}

Magick::Coordinate Magick::TypeMetric::origin(void) const
{
  return(Coordinate(_typeMetric.origin.x,_typeMetric.origin.y));
}

Magick::Coordinate Magick::TypeMetric::pixelsPerEm(void) const
{
  return(Coordinate(_typeMetric.pixels_per_em.x,_typeMetric.pixels_per_em.y));
}

double Magick::TypeMetric::textHeight(void) const
{
  return(_typeMetric.height);
}

double Magick::TypeMetric::textWidth(void) const
{
  return(_typeMetric.width);
}

double Magick::TypeMetric::underlinePosition(void) const
{
  return(_typeMetric.underline_position);
}

double Magick::TypeMetric::underlineThickness(void) const
{
  return(_typeMetric.underline_thickness);
}

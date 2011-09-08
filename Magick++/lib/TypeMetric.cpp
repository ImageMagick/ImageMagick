// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001
//
// TypeMetric implementation
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/TypeMetric.h"
#include <string.h>

// Default constructor
Magick::TypeMetric::TypeMetric ( void )
{
  memset( &_typeMetric, 0, sizeof(_typeMetric));
}

Magick::TypeMetric::~TypeMetric ( void )
{
  // Nothing to do
}

// Ascent, expressed in pixels
double Magick::TypeMetric::ascent ( void ) const
{
  return _typeMetric.ascent;
}

// Descent, expressed in pixels
double Magick::TypeMetric::descent ( void ) const
{
  return _typeMetric.descent;
}

// Text width, expressed in pixels
double Magick::TypeMetric::textWidth ( void ) const
{
  return _typeMetric.width;
}

// Text height, expressed in pixels
double Magick::TypeMetric::textHeight ( void ) const
{
  return _typeMetric.height;
}

// Maximum horizontal advance, expressed in pixels
double Magick::TypeMetric::maxHorizontalAdvance ( void ) const
{
  return _typeMetric.max_advance;
}

// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001
// Copyright Dirk Lemstra 2014
//
// TypeMetric implementation
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/TypeMetric.h"
#include <string.h>

Magick::TypeMetric::TypeMetric(void)
{
  ResetMagickMemory(&_typeMetric,0,sizeof(_typeMetric));
}

Magick::TypeMetric::~TypeMetric(void)
{
}

double Magick::TypeMetric::ascent(void) const
{
  return(_typeMetric.ascent);
}

double Magick::TypeMetric::descent(void) const
{
  return(_typeMetric.descent);
}

double Magick::TypeMetric::maxHorizontalAdvance(void) const
{
  return(_typeMetric.max_advance);
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

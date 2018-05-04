// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001, 2002
// Copyright Dirk Lemstra 2014-2018
//
// TypeMetric Definition
//
// Container for font type metrics
//

#if !defined (Magick_TypeMetric_header)
#define Magick_TypeMetric_header

#include "Magick++/Include.h"
#include "Magick++/Drawable.h"

namespace Magick
{
  class MagickPPExport TypeMetric
  {
    friend class Image;

  public:

    // Default constructor
    TypeMetric(void);

    // Destructor
    ~TypeMetric(void);

    // The distance in pixels from the text baseline to the highest/upper
    // grid coordinate used to place an outline point.
    double ascent(void) const;

    // The bounds of the type metric.
    Geometry bounds(void) const;

    // The distance in pixels from the baseline to the lowest grid coordinate
    // used to place an outline point. Always a negative value.
    double descent(void) const;

    // Maximum horizontal advance in pixels.
    double maxHorizontalAdvance(void) const;

    // The origin.
    Coordinate origin(void) const;

    // The number of pixels per em.
    Coordinate pixelsPerEm(void) const;

    // Text height in pixels.
    double textHeight(void) const;

    // Text width in pixels.
    double textWidth(void) const;

    // Underline position.
    double underlinePosition(void) const;

    // Underline thickness.
    double underlineThickness(void) const;

  private:
    MagickCore::TypeMetric _typeMetric;
  };
} // namespace Magick

#endif // Magick_TypeMetric_header

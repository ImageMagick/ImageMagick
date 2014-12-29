// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001, 2002
// Copyright Dirk Lemstra 2014
//
// TypeMetric Definition
//
// Container for font type metrics
//

#if !defined (Magick_TypeMetric_header)
#define Magick_TypeMetric_header

#include "Magick++/Include.h"

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

    // Ascent, the distance in pixels from the text baseline to the
    // highest/upper grid coordinate used to place an outline point.
    double ascent(void) const;

    // Descent, the distance in pixels from the baseline to the lowest
    // grid coordinate used to place an outline point. Always a
    // negative value.
    double descent(void) const;

    // Maximum horizontal advance in pixels.
    double maxHorizontalAdvance(void) const;

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

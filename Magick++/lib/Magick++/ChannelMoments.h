// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Dirk Lemstra, 2014
//
// Definition of channel moments.
//

#if !defined (Magick_ChannelMoments_header)
#define Magick_ChannelMoments_header

#include "Magick++/Include.h"
#include <vector>

namespace Magick
{
  class MagickPPExport ChannelMoments
  {
  public:

    // Default constructor
    ChannelMoments(void);

    // Copy constructor
    ChannelMoments(const ChannelMoments &channelMoments_);

    // Destroy channel moments
    ~ChannelMoments(void);

    //
    // Implemementation methods
    //
    ChannelMoments(const MagickCore::ChannelMoments *channelMoments_);

    // X position of centroid
    double centroidX(void) const;

    // Y position of centroid
    double centroidY(void) const;

    // X position of ellipse axis
    double ellipseAxisX(void) const;

    // Y position of ellipse axis
    double ellipseAxisY(void) const;

    // Ellipse angle
    double ellipseAngle(void) const;

    // Ellipse eccentricity
    double ellipseEccentricity(void) const;

    // Ellipse intensity
    double ellipseIntensity(void) const;

    // Hu invariants (valid range for index is 0-7)
    double huInvariants(const size_t index_) const;

  private:
    std::vector<double> _huInvariants;
    double _centroidX;
    double _centroidY;
    double _ellipseAxisX;
    double _ellipseAxisY;
    double _ellipseAngle;
    double _ellipseEccentricity;
    double _ellipseIntensity;
  };

}

#endif // Magick_ChannelMoments_header
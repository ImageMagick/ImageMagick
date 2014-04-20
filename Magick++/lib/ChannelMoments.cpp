// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Dirk Lemstra, 2014
//
// Implementation of channel moments.
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION  1

#include "Magick++/Include.h"
#include "Magick++/ChannelMoments.h"
#include "Magick++/Exception.h"

using namespace std;

Magick::ChannelMoments::ChannelMoments(void)
  : _huInvariants(),
    _centroidX(0.0),
    _centroidY(0.0),
    _ellipseAxisX(0.0),
    _ellipseAxisY(0.0),
    _ellipseAngle(0.0),
    _ellipseEccentricity(0.0),
    _ellipseIntensity(0.0)
{
}

Magick::ChannelMoments::ChannelMoments(const ChannelMoments &channelMoments_)
  : _huInvariants(channelMoments_._huInvariants),
    _centroidX(channelMoments_._centroidX),
    _centroidY(channelMoments_._centroidY),
    _ellipseAxisX(channelMoments_._ellipseAxisX),
    _ellipseAxisY(channelMoments_._ellipseAxisY),
    _ellipseAngle(channelMoments_._ellipseAngle),
    _ellipseEccentricity(channelMoments_._ellipseEccentricity),
    _ellipseIntensity(channelMoments_._ellipseIntensity)
{
//channelMoments_
}

Magick::ChannelMoments::~ChannelMoments(void)
{
}

Magick::ChannelMoments::ChannelMoments(
  const MagickCore::ChannelMoments *channelMoments_)
  : _huInvariants(),
    _centroidX(channelMoments_->centroid.x),
    _centroidY(channelMoments_->centroid.y),
    _ellipseAxisX(channelMoments_->ellipse_axis.x),
    _ellipseAxisY(channelMoments_->ellipse_axis.y),
    _ellipseAngle(channelMoments_->ellipse_angle),
    _ellipseEccentricity(channelMoments_->ellipse_eccentricity),
    _ellipseIntensity(channelMoments_->ellipse_intensity)
{
  size_t
    i;

  for (i=0; i<8; i++)
    _huInvariants.push_back(channelMoments_->I[i]);
}

double Magick::ChannelMoments::centroidX(void) const
{
  return(_centroidX);
}

double Magick::ChannelMoments::centroidY(void) const
{
  return(_centroidY);
}

double Magick::ChannelMoments::ellipseAxisX(void) const
{
  return(_ellipseAxisX);
}

double Magick::ChannelMoments::ellipseAxisY(void) const
{
  return(_ellipseAxisY);
}

double Magick::ChannelMoments::ellipseAngle(void) const
{
  return(_ellipseAngle);
}

double Magick::ChannelMoments::ellipseEccentricity(void) const
{
  return(_ellipseEccentricity);
}

double Magick::ChannelMoments::ellipseIntensity(void) const
{
  return(_ellipseIntensity);
}

double Magick::ChannelMoments::huInvariants(const size_t index_) const
{
  if (index_ > 7)
    throw ErrorOption("Valid range for index is 0-7");

  return(_huInvariants.at(index_));
}
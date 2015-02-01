// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Dirk Lemstra 2014-2015
//
// Implementation of channel moments.
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION  1

#include "Magick++/Include.h"
#include "Magick++/ChannelMoments.h"
#include "Magick++/Exception.h"
#include "Magick++/Image.h"

using namespace std;

Magick::ChannelMoments::ChannelMoments(void)
  : _huInvariants(8),
    _channel(UndefinedChannel),
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
    _channel(channelMoments_._channel),
    _centroidX(channelMoments_._centroidX),
    _centroidY(channelMoments_._centroidY),
    _ellipseAxisX(channelMoments_._ellipseAxisX),
    _ellipseAxisY(channelMoments_._ellipseAxisY),
    _ellipseAngle(channelMoments_._ellipseAngle),
    _ellipseEccentricity(channelMoments_._ellipseEccentricity),
    _ellipseIntensity(channelMoments_._ellipseIntensity)
{
}

Magick::ChannelMoments::~ChannelMoments(void)
{
}

Magick::ChannelMoments::ChannelMoments(const ChannelType channel_,
  const MagickCore::ChannelMoments *channelMoments_)
  : _huInvariants(),
    _channel(channel_),
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

Magick::ChannelType Magick::ChannelMoments::channel(void) const
{
  return(_channel);
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

Magick::ImageMoments::ImageMoments(void)
  : _channels()
{
}

Magick::ImageMoments::ImageMoments(const ImageMoments &imageMoments_)
  : _channels(imageMoments_._channels)
{
}

Magick::ImageMoments::~ImageMoments(void)
{
}

Magick::ChannelMoments Magick::ImageMoments::channel(
  const ChannelType channel_) const
{
  for (std::vector<ChannelMoments>::const_iterator it = _channels.begin();
       it != _channels.end(); ++it)
  {
    if (it->channel() == channel_)
      return(*it);
  }
  return(ChannelMoments());
}

Magick::ImageMoments::ImageMoments(const Image &image_)
  : _channels()
{
  MagickCore::ChannelMoments*
    channel_moments;

  GetPPException;
  channel_moments=GetImageChannelMoments(image_.constImage(),exceptionInfo);
  if (channel_moments != (MagickCore::ChannelMoments *) NULL)
    {
      switch(image_.constImage()->colorspace)
      {
        case RGBColorspace:
        default:
          _channels.push_back(Magick::ChannelMoments(RedChannel,
            &channel_moments[RedChannel]));
          _channels.push_back(Magick::ChannelMoments(GreenChannel,
            &channel_moments[GreenChannel]));
          _channels.push_back(Magick::ChannelMoments(BlueChannel,
            &channel_moments[BlueChannel]));
          break;
        case CMYKColorspace:
          _channels.push_back(Magick::ChannelMoments(CyanChannel,
            &channel_moments[CyanChannel]));
          _channels.push_back(Magick::ChannelMoments(MagentaChannel,
            &channel_moments[MagentaChannel]));
          _channels.push_back(Magick::ChannelMoments(YellowChannel,
            &channel_moments[YellowChannel]));
          _channels.push_back(Magick::ChannelMoments(BlackChannel,
            &channel_moments[BlackChannel]));
          break;
        case GRAYColorspace:
          _channels.push_back(Magick::ChannelMoments(GrayChannel,
            &channel_moments[GrayChannel]));
          break;
      }
      if (image_.constImage()->matte != MagickFalse)
        _channels.push_back(Magick::ChannelMoments(AlphaChannel,
          &channel_moments[AlphaChannel]));
      if (image_.constImage()->colorspace != GRAYColorspace)
        _channels.push_back(Magick::ChannelMoments(CompositeChannels,
          &channel_moments[CompositeChannels]));
      channel_moments=(MagickCore::ChannelMoments *) RelinquishMagickMemory(
        channel_moments);
    }
  ThrowPPException(image_.quiet());
}

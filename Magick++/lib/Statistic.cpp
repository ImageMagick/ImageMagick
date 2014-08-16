// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Dirk Lemstra, 2014
//
// Implementation of channel moments.
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION  1

#include "Magick++/Include.h"
#include "Magick++/Exception.h"
#include "Magick++/Statistic.h"

using namespace std;

Magick::ChannelMoments::ChannelMoments(void)
  : _huInvariants(8),
    _channel(SyncPixelChannel),
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

double Magick::ChannelMoments::centroidX(void) const
{
  return(_centroidX);
}

double Magick::ChannelMoments::centroidY(void) const
{
  return(_centroidY);
}

Magick::PixelChannel Magick::ChannelMoments::channel(void) const
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

bool Magick::ChannelMoments::isValid() const
{
  return(_channel != SyncPixelChannel);
}

Magick::ChannelMoments::ChannelMoments(const PixelChannel channel_,
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

Magick::ChannelStatistics::ChannelStatistics(void)
  : _channel(SyncPixelChannel),
    _area(0.0),
    _depth(0.0),
    _kurtosis(0.0),
    _maxima(0.0),
    _mean(0.0),
    _minima(0.0),
    _skewness(0.0),
    _standardDeviation(0.0),
    _sum(0.0),
    _sumCubed(0.0),
    _sumFourthPower(0.0),
    _sumSquared(0.0),
    _variance(0.0)
{
}

Magick::ChannelStatistics::ChannelStatistics(
  const ChannelStatistics &channelStatistics_)
  : _channel(channelStatistics_._channel),
    _area(channelStatistics_._area),
    _depth(channelStatistics_._depth),
    _kurtosis(channelStatistics_._kurtosis),
    _maxima(channelStatistics_._maxima),
    _mean(channelStatistics_._mean),
    _minima(channelStatistics_._minima),
    _skewness(channelStatistics_._skewness),
    _standardDeviation(channelStatistics_._standardDeviation),
    _sum(channelStatistics_._sum),
    _sumCubed(channelStatistics_._sumCubed),
    _sumFourthPower(channelStatistics_._sumFourthPower),
    _sumSquared(channelStatistics_._sumSquared),
    _variance(channelStatistics_._variance)
{
}

Magick::ChannelStatistics::~ChannelStatistics(void)
{
}

double Magick::ChannelStatistics::area() const
{
  return(_area);
}

Magick::PixelChannel Magick::ChannelStatistics::channel() const
{
  return(_channel);
}

size_t Magick::ChannelStatistics::depth() const
{
  return(_depth);
}

bool Magick::ChannelStatistics::isValid() const
{
  return(_channel != SyncPixelChannel);
}

double Magick::ChannelStatistics::kurtosis() const
{
  return(_kurtosis);
}

double Magick::ChannelStatistics::maxima() const
{
  return(_maxima);
}

double Magick::ChannelStatistics::mean() const
{
  return(_mean);
}

double Magick::ChannelStatistics::minima() const
{
  return(_minima);
}

double Magick::ChannelStatistics::skewness() const
{
  return(_skewness);
}

double Magick::ChannelStatistics::standardDeviation() const
{
  return(_standardDeviation);
}

double Magick::ChannelStatistics::sum() const
{
  return(_sum);
}

double Magick::ChannelStatistics::sumCubed() const
{
  return(_sumCubed);
}

double Magick::ChannelStatistics::sumFourthPower() const
{
  return(_sumFourthPower);
}

double Magick::ChannelStatistics::sumSquared() const
{
  return(_sumSquared);
}

double Magick::ChannelStatistics::variance() const
{
  return(_variance);
}

Magick::ChannelStatistics::ChannelStatistics(const PixelChannel channel_,
  const MagickCore::ChannelStatistics *channelStatistics_)
  : _channel(channel_),
    _area(channelStatistics_->area),
    _depth(channelStatistics_->depth),
    _kurtosis(channelStatistics_->kurtosis),
    _maxima(channelStatistics_->maxima),
    _mean(channelStatistics_->mean),
    _minima(channelStatistics_->minima),
    _skewness(channelStatistics_->skewness),
    _standardDeviation(channelStatistics_->standard_deviation),
    _sum(channelStatistics_->sum),
    _sumCubed(channelStatistics_->sum_cubed),
    _sumFourthPower(channelStatistics_->sum_fourth_power),
    _sumSquared(channelStatistics_->sum_squared),
    _variance(channelStatistics_->variance)
{
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
  const PixelChannel channel_) const
{
  for (std::vector<ChannelMoments>::const_iterator it = _channels.begin();
       it != _channels.end(); ++it)
  {
    if (it->channel() == channel_)
      return(*it);
  }
  return(ChannelMoments());
}

Magick::ImageMoments::ImageMoments(const MagickCore::Image *image)
  : _channels()
{
  MagickCore::ChannelMoments*
    channel_moments;

  GetPPException;
  channel_moments=GetImageMoments(image,exceptionInfo);
  if (channel_moments != (MagickCore::ChannelMoments *) NULL)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        _channels.push_back(Magick::ChannelMoments(channel,
          &channel_moments[channel]));
      }
      _channels.push_back(Magick::ChannelMoments(CompositePixelChannel,
        &channel_moments[CompositePixelChannel]));
      channel_moments=(MagickCore::ChannelMoments *) RelinquishMagickMemory(
        channel_moments);
    }
  ThrowPPException;
}

Magick::ImageStatistics::ImageStatistics(void)
  : _channels()
{
}

Magick::ImageStatistics::ImageStatistics(
  const ImageStatistics &imageStatistics_)
  : _channels(imageStatistics_._channels)
{
}

Magick::ImageStatistics::~ImageStatistics(void)
{
}

Magick::ChannelStatistics Magick::ImageStatistics::channel(
  const PixelChannel channel_) const
{
  for (std::vector<ChannelStatistics>::const_iterator it = _channels.begin();
       it != _channels.end(); ++it)
  {
    if (it->channel() == channel_)
      return(*it);
  }
  return(ChannelStatistics());
}

Magick::ImageStatistics::ImageStatistics(const MagickCore::Image *image)
  : _channels()
{
  MagickCore::ChannelStatistics*
    channel_statistics;

  GetPPException;
  channel_statistics=GetImageStatistics(image,exceptionInfo);
  if (channel_statistics != (MagickCore::ChannelStatistics *) NULL)
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image,i);
        PixelTrait traits=GetPixelChannelTraits(image,channel);
        if (traits == UndefinedPixelTrait)
          continue;
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        _channels.push_back(Magick::ChannelStatistics(channel,
          &channel_statistics[channel]));
      }
      _channels.push_back(Magick::ChannelStatistics(CompositePixelChannel,
        &channel_statistics[CompositePixelChannel]));
      channel_statistics=(MagickCore::ChannelStatistics *) RelinquishMagickMemory(
        channel_statistics);
    }
  ThrowPPException;
}
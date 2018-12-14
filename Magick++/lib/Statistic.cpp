// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Dirk Lemstra 2014-2015
//
// Implementation of channel moments.
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION  1

#include "Magick++/Include.h"
#include "Magick++/Exception.h"
#include "Magick++/Statistic.h"
#include "Magick++/Image.h"

using namespace std;

Magick::ChannelMoments::ChannelMoments(void)
  : _channel(SyncPixelChannel),
    _huInvariants(8),
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
  : _channel(channelMoments_._channel),
    _huInvariants(channelMoments_._huInvariants),
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
  : _channel(channel_),
    _huInvariants(),
    _centroidX(channelMoments_->centroid.x),
    _centroidY(channelMoments_->centroid.y),
    _ellipseAxisX(channelMoments_->ellipse_axis.x),
    _ellipseAxisY(channelMoments_->ellipse_axis.y),
    _ellipseAngle(channelMoments_->ellipse_angle),
    _ellipseEccentricity(channelMoments_->ellipse_eccentricity),
    _ellipseIntensity(channelMoments_->ellipse_intensity)
{
  ssize_t
    i;

  for (i=0; i<8; i++)
    _huInvariants.push_back(channelMoments_->invariant[i]);
}

Magick::ChannelPerceptualHash::ChannelPerceptualHash(void)
  : _channel(SyncPixelChannel),
    _srgbHuPhash(7),
    _hclpHuPhash(7)
{
}

Magick::ChannelPerceptualHash::ChannelPerceptualHash(
  const ChannelPerceptualHash &channelPerceptualHash_)
  : _channel(channelPerceptualHash_._channel),
    _srgbHuPhash(channelPerceptualHash_._srgbHuPhash),
    _hclpHuPhash(channelPerceptualHash_._hclpHuPhash)
{
}

Magick::ChannelPerceptualHash::ChannelPerceptualHash(
  const PixelChannel channel_,const std::string &hash_)
  : _channel(channel_),
    _srgbHuPhash(7),
    _hclpHuPhash(7)
{
  ssize_t
    i;

  if (hash_.length() != 70)
    throw ErrorOption("Invalid hash length");

  for (i=0; i<14; i++)
  {
    unsigned int
      hex;

    double
      value;

    if (sscanf(hash_.substr(i*5,5).c_str(),"%05x",&hex) != 1)
      throw ErrorOption("Invalid hash value");

    value=((unsigned short)hex) / pow(10.0, (double)(hex >> 17));
    if (hex & (1 << 16))
      value=-value;
    if (i < 7)
      _srgbHuPhash[i]=value;
    else
      _hclpHuPhash[i-7]=value;
  }
}

Magick::ChannelPerceptualHash::~ChannelPerceptualHash(void)
{
}

Magick::ChannelPerceptualHash::operator std::string() const
{
  std::string
    hash;

  ssize_t
    i;

  if (!isValid())
    return(std::string());

  for (i=0; i<14; i++)
  {
    char
      buffer[6];

    double
      value;

    unsigned int
      hex;

    if (i < 7)
      value=_srgbHuPhash[i];
    else
      value=_hclpHuPhash[i-7];

    hex=0;
    while(hex < 7 && fabs(value*10) < 65536)
    {
      value=value*10;
      hex++;
    }

    hex=(hex<<1);
    if (value < 0.0)
      hex|=1;
    hex=(hex<<16)+(unsigned int)(value < 0.0 ? -(value - 0.5) : value + 0.5);
    (void) FormatLocaleString(buffer,6,"%05x",hex);
    hash+=std::string(buffer);
  }
  return(hash);
}

Magick::PixelChannel Magick::ChannelPerceptualHash::channel() const
{
  return(_channel);
}

bool Magick::ChannelPerceptualHash::isValid() const
{
  return(_channel != SyncPixelChannel);
}

double Magick::ChannelPerceptualHash::sumSquaredDifferences(
  const ChannelPerceptualHash &channelPerceptualHash_)
{
  double
    ssd;

  ssize_t
    i;

  ssd=0.0;
  for (i=0; i<7; i++)
  {
    ssd+=((_srgbHuPhash[i]-channelPerceptualHash_._srgbHuPhash[i])*
      (_srgbHuPhash[i]-channelPerceptualHash_._srgbHuPhash[i]));
    ssd+=((_hclpHuPhash[i]-channelPerceptualHash_._hclpHuPhash[i])*
      (_hclpHuPhash[i]-channelPerceptualHash_._hclpHuPhash[i]));
  }
  return(ssd);
}

double Magick::ChannelPerceptualHash::srgbHuPhash(const size_t index_) const
{
  if (index_ > 6)
    throw ErrorOption("Valid range for index is 0-6");

  return(_srgbHuPhash.at(index_));
}

double Magick::ChannelPerceptualHash::hclpHuPhash(const size_t index_) const
{
  if (index_ > 6)
    throw ErrorOption("Valid range for index is 0-6");

  return(_hclpHuPhash.at(index_));
}

Magick::ChannelPerceptualHash::ChannelPerceptualHash(
  const PixelChannel channel_,
  const MagickCore::ChannelPerceptualHash *channelPerceptualHash_)
  : _channel(channel_),
    _srgbHuPhash(7),
    _hclpHuPhash(7)
{
  ssize_t
    i;

  for (i=0; i<7; i++)
  {
    _srgbHuPhash[i]=channelPerceptualHash_->phash[0][i];
    _hclpHuPhash[i]=channelPerceptualHash_->phash[1][i];
  }
}

Magick::ChannelStatistics::ChannelStatistics(void)
  : _channel(SyncPixelChannel),
    _area(0.0),
    _depth(0.0),
    _entropy(0.0),
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
    _entropy(channelStatistics_._entropy),
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

double Magick::ChannelStatistics::entropy() const
{
  return(_entropy);
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
    _entropy(channelStatistics_->entropy),
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

Magick::ImageMoments::ImageMoments(const Image &image_)
  : _channels()
{
  MagickCore::ChannelMoments*
    channel_moments;

  GetPPException;
  channel_moments=GetImageMoments(image_.constImage(),exceptionInfo);
  if (channel_moments != (MagickCore::ChannelMoments *) NULL)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image_.constImage()); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image_.constImage(),i);
        PixelTrait traits=GetPixelChannelTraits(image_.constImage(),channel);
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
  ThrowPPException(image_.quiet());
}

Magick::ImagePerceptualHash::ImagePerceptualHash(void)
  : _channels()
{
}

Magick::ImagePerceptualHash::ImagePerceptualHash(
  const ImagePerceptualHash &imagePerceptualHash_)
  : _channels(imagePerceptualHash_._channels)
{
}

Magick::ImagePerceptualHash::ImagePerceptualHash(const std::string &hash_)
  : _channels()
{
  if (hash_.length() != 210)
    throw ErrorOption("Invalid hash length");

  _channels.push_back(Magick::ChannelPerceptualHash(RedPixelChannel,
    hash_.substr(0, 70)));
  _channels.push_back(Magick::ChannelPerceptualHash(GreenPixelChannel,
    hash_.substr(70, 70)));
  _channels.push_back(Magick::ChannelPerceptualHash(BluePixelChannel,
    hash_.substr(140, 70)));
}

Magick::ImagePerceptualHash::~ImagePerceptualHash(void)
{
}

Magick::ImagePerceptualHash::operator std::string() const
{
  if (!isValid())
    return(std::string());

  return static_cast<std::string>(_channels[0]) +
    static_cast<std::string>(_channels[1]) + 
    static_cast<std::string>(_channels[2]);
}

Magick::ChannelPerceptualHash Magick::ImagePerceptualHash::channel(
  const PixelChannel channel_) const
{
  for (std::vector<ChannelPerceptualHash>::const_iterator it =
       _channels.begin(); it != _channels.end(); ++it)
  {
    if (it->channel() == channel_)
      return(*it);
  }
  return(ChannelPerceptualHash());
}

bool Magick::ImagePerceptualHash::isValid() const
{
  if (_channels.size() != 3)
    return(false);

  if (_channels[0].channel() != RedPixelChannel)
    return(false);

  if (_channels[1].channel() != GreenPixelChannel)
    return(false);

  if (_channels[2].channel() != BluePixelChannel)
    return(false);

  return(true);
}

double Magick::ImagePerceptualHash::sumSquaredDifferences(
      const ImagePerceptualHash &channelPerceptualHash_)
{
  double
    ssd;

  ssize_t
    i;

  if (!isValid())
    throw ErrorOption("instance is not valid");
  if (!channelPerceptualHash_.isValid())
    throw ErrorOption("channelPerceptualHash_ is not valid");

  ssd=0.0;
  for (i=0; i<3; i++)
  {
    ssd+=_channels[i].sumSquaredDifferences(_channels[i]);
  }
  return(ssd);
}

Magick::ImagePerceptualHash::ImagePerceptualHash(
  const Image &image_)
  : _channels()
{
  MagickCore::ChannelPerceptualHash*
    channel_perceptual_hash;

  PixelTrait
    traits;

  GetPPException;
  channel_perceptual_hash=GetImagePerceptualHash(image_.constImage(),
    exceptionInfo);
  if (channel_perceptual_hash != (MagickCore::ChannelPerceptualHash *) NULL)
    {
      traits=GetPixelChannelTraits(image_.constImage(),RedPixelChannel);
      if ((traits & UpdatePixelTrait) != 0)
        _channels.push_back(Magick::ChannelPerceptualHash(RedPixelChannel,
          &channel_perceptual_hash[RedPixelChannel]));
      traits=GetPixelChannelTraits(image_.constImage(),GreenPixelChannel);
      if ((traits & UpdatePixelTrait) != 0)
        _channels.push_back(Magick::ChannelPerceptualHash(GreenPixelChannel,
          &channel_perceptual_hash[GreenPixelChannel]));
      traits=GetPixelChannelTraits(image_.constImage(),BluePixelChannel);
      if ((traits & UpdatePixelTrait) != 0)
        _channels.push_back(Magick::ChannelPerceptualHash(BluePixelChannel,
          &channel_perceptual_hash[BluePixelChannel]));
      channel_perceptual_hash=(MagickCore::ChannelPerceptualHash *)
        RelinquishMagickMemory(channel_perceptual_hash);
    }
  ThrowPPException(image_.quiet());
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

Magick::ImageStatistics::ImageStatistics(const Image &image_)
  : _channels()
{
  MagickCore::ChannelStatistics*
    channel_statistics;

  GetPPException;
  channel_statistics=GetImageStatistics(image_.constImage(),exceptionInfo);
  if (channel_statistics != (MagickCore::ChannelStatistics *) NULL)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image_.constImage()); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(image_.constImage(),i);
        PixelTrait traits=GetPixelChannelTraits(image_.constImage(),channel);
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
  ThrowPPException(image_.quiet());
}

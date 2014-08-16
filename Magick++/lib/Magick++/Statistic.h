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

    // X position of centroid
    double centroidX(void) const;

    // Y position of centroid
    double centroidY(void) const;

    // The channel
    PixelChannel channel(void) const;

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

    // Does object contain valid channel moments?
    bool isValid() const;

    //
    // Implemementation methods
    //

    ChannelMoments(const PixelChannel channel_,
      const MagickCore::ChannelMoments *channelMoments_);

  private:
    std::vector<double> _huInvariants;
    PixelChannel _channel;
    double _centroidX;
    double _centroidY;
    double _ellipseAxisX;
    double _ellipseAxisY;
    double _ellipseAngle;
    double _ellipseEccentricity;
    double _ellipseIntensity;
  };

  // Obtain image statistics. Statistics are normalized to the range
  // of 0.0 to 1.0 and are output to the specified ImageStatistics
  // structure.
  class MagickPPExport ChannelStatistics
  {
  public:

    // Default constructor
    ChannelStatistics(void);

    // Copy constructor
    ChannelStatistics(const ChannelStatistics &channelStatistics_);

    // Destroy channel statistics
    ~ChannelStatistics(void);

    // Area
    double area() const;

    // The channel
    PixelChannel channel(void) const;

    // Depth
    size_t depth() const;

    // Does object contain valid channel statistics?
    bool isValid() const;

    // Kurtosis
    double kurtosis() const;

    // Minimum value observed
    double maxima() const;

    // Average (mean) value observed
    double mean() const;

    // Maximum value observed
    double minima() const;

    // Skewness
    double skewness() const;

    // Standard deviation, sqrt(variance)
    double standardDeviation() const;

    // Sum
    double sum() const;

    // Sum cubed
    double sumCubed() const;

    // Sum fourth power
    double sumFourthPower() const;

    // Sum squared
    double sumSquared() const;

    // Variance
    double variance() const;

    //
    // Implemementation methods
    //

    ChannelStatistics(const PixelChannel channel_,
      const MagickCore::ChannelStatistics *channelStatistics_);

  private:
    PixelChannel _channel;
    double _area;
    size_t _depth;
    double _kurtosis;
    double _maxima;
    double _mean;
    double _minima;
    double _skewness;
    double _standardDeviation;
    double _sum;
    double _sumSquared;
    double _sumCubed;
    double _sumFourthPower;
    double _variance;
  };

  class MagickPPExport ImageMoments
  {
  public:

    // Default constructor
    ImageMoments(void);

    // Copy constructor
    ImageMoments(const ImageMoments &imageMoments_);

    // Destroy image moments
    ~ImageMoments(void);

    // Returns the moments for the specified channel
    ChannelMoments channel(const PixelChannel channel_) const;

    //
    // Implemementation methods
    //
    ImageMoments(const MagickCore::Image *image_);

  private:
    std::vector<ChannelMoments> _channels;
  };

  class MagickPPExport ImageStatistics
  {
  public:

    // Default constructor
    ImageStatistics(void);

    // Copy constructor
    ImageStatistics(const ImageStatistics &imageStatistics_);

    // Destroy image statistics
    ~ImageStatistics(void);

    // Returns the statistics for the specified channel
    ChannelStatistics channel(const PixelChannel channel_) const;

    //
    // Implemementation methods
    //
    ImageStatistics(const MagickCore::Image *image_);

  private:
    std::vector<ChannelStatistics> _channels;
  };
}

#endif // Magick_ChannelMoments_header
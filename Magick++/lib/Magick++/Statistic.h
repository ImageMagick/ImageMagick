// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Dirk Lemstra 2014-2015
//
// Definition of channel moments.
//

#if !defined (Magick_ChannelMoments_header)
#define Magick_ChannelMoments_header

#include "Magick++/Include.h"
#include <vector>

namespace Magick
{
  class Image;

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
    PixelChannel _channel;
    std::vector<double> _huInvariants;
    double _centroidX;
    double _centroidY;
    double _ellipseAxisX;
    double _ellipseAxisY;
    double _ellipseAngle;
    double _ellipseEccentricity;
    double _ellipseIntensity;
  };

  class MagickPPExport ChannelPerceptualHash
  {
  public:

    // Default constructor
    ChannelPerceptualHash(void);

    // Copy constructor
    ChannelPerceptualHash(const ChannelPerceptualHash &channelPerceptualHash_);

    // Constructor using the specified hash string
    ChannelPerceptualHash(const PixelChannel channel_,
      const std::string &hash_);

    // Destroy channel perceptual hash
    ~ChannelPerceptualHash(void);

    // Return hash string
    operator std::string() const;

    // The channel
    PixelChannel channel(void) const;

    // Does object contain valid channel perceptual hash?
    bool isValid() const;

    // Returns the sum squared difference between this hash and the other hash
    double sumSquaredDifferences(
      const ChannelPerceptualHash &channelPerceptualHash_);

    // SRGB hu preceptual hash (valid range for index is 0-6)
    double srgbHuPhash(const size_t index_) const;

    // HCLp hu preceptual hash (valid range for index is 0-6)
    double hclpHuPhash(const size_t index_) const;

    //
    // Implemementation methods
    //

    ChannelPerceptualHash(const PixelChannel channel_,
      const MagickCore::ChannelPerceptualHash *channelPerceptualHash_);

  private:
    PixelChannel _channel;
    std::vector<double> _srgbHuPhash;
    std::vector<double> _hclpHuPhash;
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

    // Entropy
    double entropy() const;

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
    double _entropy;
    double _kurtosis;
    double _maxima;
    double _mean;
    double _minima;
    double _skewness;
    double _standardDeviation;
    double _sum;
    double _sumCubed;
    double _sumFourthPower;
    double _sumSquared;
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
    ImageMoments(const Image &image_);

  private:
    std::vector<ChannelMoments> _channels;
  };

  class MagickPPExport ImagePerceptualHash
  {
  public:

    // Default constructor
    ImagePerceptualHash(void);

    // Copy constructor
    ImagePerceptualHash(const ImagePerceptualHash &imagePerceptualHash_);

    // Constructor using the specified hash string
    ImagePerceptualHash(const std::string &hash_);

    // Destroy image perceptual hash
    ~ImagePerceptualHash(void);

    // Return hash string
    operator std::string() const;

    // Returns the perceptual hash for the specified channel
    ChannelPerceptualHash channel(const PixelChannel channel_) const;

    // Does object contain valid perceptual hash?
    bool isValid() const;

    // Returns the sum squared difference between this hash and the other hash
    double sumSquaredDifferences(
      const ImagePerceptualHash &channelPerceptualHash_);

    //
    // Implemementation methods
    //
    ImagePerceptualHash(const Image &image_);

  private:
    std::vector<ChannelPerceptualHash> _channels;
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
    ImageStatistics(const Image &image_);

  private:
    std::vector<ChannelStatistics> _channels;
  };
}

#endif // Magick_ChannelMoments_header

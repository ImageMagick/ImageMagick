// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Definition and implementation of template functions for using
// Magick::Image with STL containers.
//

#ifndef Magick_STL_header
#define Magick_STL_header

#include "Magick++/Include.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <utility>

#include "Magick++/CoderInfo.h"
#include "Magick++/Drawable.h"
#include "Magick++/Exception.h"
#include "Magick++/Montage.h"

namespace Magick
{
  //
  // STL function object declarations/definitions
  //

  // Function objects provide the means to invoke an operation on one
  // or more image objects in an STL-compatable container.  The
  // arguments to the function object constructor(s) are compatable
  // with the arguments to the equivalent Image class method and
  // provide the means to supply these options when the function
  // object is invoked.

  // For example, to read a GIF animation, set the color red to
  // transparent for all frames, and write back out:
  //
  // list<image> images;
  // readImages( &images, "animation.gif" );
  // for_each( images.begin(), images.end(), transparentImage( "red" ) );
  // writeImages( images.begin(), images.end(), "animation.gif" );

  // Adaptive-blur image with specified blur factor
  class MagickDLLDecl adaptiveBlurImage : public std::unary_function<Image&,void>
  {
  public:
    adaptiveBlurImage( const double radius_ = 1, const double sigma_ = 0.5 );

    void operator()( Image &image_ ) const;

  private:
    double _radius;
    double _sigma;
  };

  // Local adaptive threshold image
  // http://www.dai.ed.ac.uk/HIPR2/adpthrsh.htm
  // Width x height define the size of the pixel neighborhood
  // offset = constant to subtract from pixel neighborhood mean
  class MagickDLLDecl adaptiveThresholdImage : public std::unary_function<Image&,void>
  {
  public:
    adaptiveThresholdImage( const size_t width_,
                            const size_t height_,
                            const ssize_t offset_ = 0  );

    void operator()( Image &image_ ) const;

  private:
    size_t _width;
    size_t _height;
    ssize_t _offset;
  };
  
  // Add noise to image with specified noise type
  class MagickDLLDecl addNoiseImage : public std::unary_function<Image&,void>
  {
  public:
    addNoiseImage ( NoiseType noiseType_ );

    void operator()( Image &image_ ) const;

  private:
    NoiseType _noiseType;
  };

  // Transform image by specified affine (or free transform) matrix.
  class MagickDLLDecl affineTransformImage : public std::unary_function<Image&,void>
  {
  public:
    affineTransformImage( const DrawableAffine &affine_ );

    void operator()( Image &image_ ) const;

  private:
    DrawableAffine _affine;
  };

  // Annotate image (draw text on image)
  class MagickDLLDecl annotateImage : public std::unary_function<Image&,void>
  {
  public:
    // Annotate using specified text, and placement location
    annotateImage ( const std::string &text_,
                    const Geometry &geometry_ );

    // Annotate using specified text, bounding area, and placement
    // gravity
    annotateImage ( const std::string &text_,
        const Geometry &geometry_,
        const GravityType gravity_ );

    // Annotate with text using specified text, bounding area,
    // placement gravity, and rotation.
    annotateImage ( const std::string &text_,
                    const Geometry &geometry_,
                    const GravityType gravity_,
                    const double degrees_ );

    // Annotate with text (bounding area is entire image) and
    // placement gravity.
    annotateImage ( const std::string &text_,
        const GravityType gravity_ );
    
    void operator()( Image &image_ ) const;

  private:
    // Copy constructor and assignment are not supported
    annotateImage(const annotateImage&);
    annotateImage& operator=(const annotateImage&);

    const std::string   _text;
    const Geometry      _geometry;
    const GravityType   _gravity;
    const double        _degrees;
  };

  // Blur image with specified blur factor
  class MagickDLLDecl blurImage : public std::unary_function<Image&,void>
  {
  public:
    blurImage( const double radius_ = 1, const double sigma_ = 0.5 );

    void operator()( Image &image_ ) const;

  private:
    double _radius;
    double _sigma;
  };

  // Border image (add border to image)
  class MagickDLLDecl borderImage : public std::unary_function<Image&,void>
  {
  public:
    borderImage( const Geometry &geometry_ = borderGeometryDefault  );

    void operator()( Image &image_ ) const;

  private:
    Geometry _geometry;
  };

  // Extract channel from image
  class MagickDLLDecl channelImage : public std::unary_function<Image&,void>
  {
  public:
    channelImage( const ChannelType channel_ );

    void operator()( Image &image_ ) const;

  private:
    ChannelType _channel;
  };

  // Charcoal effect image (looks like charcoal sketch)
  class MagickDLLDecl charcoalImage : public std::unary_function<Image&,void>
  {
  public:
    charcoalImage( const double radius_ = 1, const double sigma_ = 0.5  );

    void operator()( Image &image_ ) const;

  private:
    double _radius;
    double _sigma;
  };

  // Chop image (remove vertical or horizontal subregion of image)
  class MagickDLLDecl chopImage : public std::unary_function<Image&,void>
  {
  public:
    chopImage( const Geometry &geometry_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry _geometry;
  };

  // Accepts a lightweight Color Correction Collection (CCC) file which solely
  // contains one or more color corrections and applies the correction to the
  // image.
  class MagickDLLDecl cdlImage : public std::unary_function<Image&,void>
  {
  public:
    cdlImage( const std::string &cdl_ );

    void operator()( Image &image_ ) const;

  private:
    std::string   _cdl;
  };

  // Colorize image using pen color at specified percent opacity
  class MagickDLLDecl colorizeImage : public std::unary_function<Image&,void>
  {
  public:
    colorizeImage( const unsigned int opacityRed_,
                   const unsigned int opacityGreen_,
                   const unsigned int opacityBlue_,
       const Color &penColor_ );

    colorizeImage( const unsigned int opacity_,
                   const Color &penColor_ );

    void operator()( Image &image_ ) const;

  private:
    unsigned int _opacityRed;
    unsigned int _opacityGreen;
    unsigned int _opacityBlue;
    Color _penColor;
  };

  // Apply a color matrix to the image channels.  The user supplied
  // matrix may be of order 1 to 5 (1x1 through 5x5).
  class MagickDLLDecl colorMatrixImage : public std::unary_function<Image&,void>
  {
  public:
    colorMatrixImage( const size_t order_,
          const double *color_matrix_ );

    void operator()( Image &image_ ) const;

  private:
    size_t  _order;
    const double *_color_matrix;
  };

  // Convert the image colorspace representation
  class MagickDLLDecl colorSpaceImage : public std::unary_function<Image&,void>
  {
  public:
    colorSpaceImage( ColorspaceType colorSpace_ );

    void operator()( Image &image_ ) const;

  private:
    ColorspaceType _colorSpace;
  };

  // Comment image (add comment string to image)
  class MagickDLLDecl commentImage : public std::unary_function<Image&,void>
  {
  public:
    commentImage( const std::string &comment_ );

    void operator()( Image &image_ ) const;

  private:
    std::string _comment;
  };

  // Compose an image onto another at specified offset and using
  // specified algorithm
  class MagickDLLDecl compositeImage : public std::unary_function<Image&,void>
  {
  public:
    compositeImage( const Image &compositeImage_,
        ssize_t xOffset_,
        ssize_t yOffset_,
        CompositeOperator compose_ = InCompositeOp );

    compositeImage( const Image &compositeImage_,
        const Geometry &offset_,
        CompositeOperator compose_ = InCompositeOp );
    
    void operator()( Image &image_ ) const;

  private:
    Image             _compositeImage;
    ssize_t               _xOffset;
    ssize_t               _yOffset;
    CompositeOperator _compose;
  };

  // Contrast image (enhance intensity differences in image)
  class MagickDLLDecl contrastImage : public std::unary_function<Image&,void>
  {
  public:
    contrastImage( const size_t sharpen_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _sharpen;
  };

  // Crop image (subregion of original image)
  class MagickDLLDecl cropImage : public std::unary_function<Image&,void>
  {
  public:
    cropImage( const Geometry &geometry_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry _geometry;
  };

  // Cycle image colormap
  class MagickDLLDecl cycleColormapImage : public std::unary_function<Image&,void>
  {
  public:
    cycleColormapImage( const ssize_t amount_ );

    void operator()( Image &image_ ) const;

  private:
    ssize_t _amount;
  };

  // Despeckle image (reduce speckle noise)
  class MagickDLLDecl despeckleImage : public std::unary_function<Image&,void>
  {
  public:
    despeckleImage( void );

    void operator()( Image &image_ ) const;

  private:
  };

  // Distort image.  distorts an image using various distortion methods, by
  // mapping color lookups of the source image to a new destination image
  // usally of the same size as the source image, unless 'bestfit' is set to
  // true.
  class MagickDLLDecl distortImage : public std::unary_function<Image&,void>
  {
  public:
    distortImage( const Magick::DistortImageMethod method_,
      const size_t number_arguments_,
      const double *arguments_,
      const bool bestfit_ );
          
    distortImage( const Magick::DistortImageMethod method_,
      const size_t number_arguments_,
      const double *arguments_ );

    void operator()( Image &image_ ) const;

  private:
    DistortImageMethod _method;
    size_t _number_arguments;
    const double *_arguments;
    bool _bestfit;
  };

  // Draw on image
  class MagickDLLDecl drawImage : public std::unary_function<Image&,void>
  {
  public:
    // Draw on image using a single drawable
    // Store in list to make implementation easier
    drawImage( const Drawable &drawable_ );

    // Draw on image using a drawable list
    drawImage( const DrawableList &drawable_ );

    void operator()( Image &image_ ) const;

  private:
    DrawableList _drawableList;
  };

  // Edge image (hilight edges in image)
  class MagickDLLDecl edgeImage : public std::unary_function<Image&,void>
  {
  public:
    edgeImage( const double radius_ = 0.0  );

    void operator()( Image &image_ ) const;

  private:
    double _radius;
  };

  // Emboss image (hilight edges with 3D effect)
  class MagickDLLDecl embossImage : public std::unary_function<Image&,void>
  {
  public:
    embossImage( void );
    embossImage( const double radius_, const double sigma_ );

    void operator()( Image &image_ ) const;

  private:
    double _radius;
    double _sigma;
  };

  // Enhance image (minimize noise)
  class MagickDLLDecl enhanceImage : public std::unary_function<Image&,void>
  {
  public:
    enhanceImage( void );

    void operator()( Image &image_ ) const;

  private:
  };

  // Equalize image (histogram equalization)
  class MagickDLLDecl equalizeImage : public std::unary_function<Image&,void>
  {
  public:
    equalizeImage( void );

    void operator()( Image &image_ ) const;

  private:
  };

  // Color to use when filling drawn objects
  class MagickDLLDecl fillColorImage : public std::unary_function<Image&,void>
  {
  public:
    fillColorImage( const Color &fillColor_ );

    void operator()( Image &image_ ) const;

  private:
    Color _fillColor;
  };

  // Flip image (reflect each scanline in the vertical direction)
  class MagickDLLDecl flipImage : public std::unary_function<Image&,void>
  {
  public:
    flipImage( void );

    void operator()( Image &image_ ) const;

  private:
  };

  // Flood-fill image with color
  class MagickDLLDecl floodFillColorImage : public std::unary_function<Image&,void>
  {
  public:
    // Flood-fill color across pixels starting at target-pixel and
    // stopping at pixels matching specified border color.
    // Uses current fuzz setting when determining color match.
    floodFillColorImage( const ssize_t x_,
                         const ssize_t y_,
       const Color &fillColor_ );

    floodFillColorImage( const Geometry &point_,
       const Color &fillColor_ );

    // Flood-fill color across pixels starting at target-pixel and
    // stopping at pixels matching specified border color.
    // Uses current fuzz setting when determining color match.
    floodFillColorImage( const ssize_t x_,
                         const ssize_t y_,
       const Color &fillColor_,
       const Color &borderColor_ );

    floodFillColorImage( const Geometry &point_,
       const Color &fillColor_,
       const Color &borderColor_ );

    void operator()( Image &image_ ) const;

  private:
    ssize_t   _x;
    ssize_t   _y;
    Color          _fillColor;
    Color          _borderColor;
  };

  // Flood-fill image with texture
  class MagickDLLDecl floodFillTextureImage : public std::unary_function<Image&,void>
  {
  public:
    // Flood-fill texture across pixels that match the color of the
    // target pixel and are neighbors of the target pixel.
    // Uses current fuzz setting when determining color match.
    floodFillTextureImage( const ssize_t x_,
                           const ssize_t y_,
         const Image &texture_ );

    floodFillTextureImage( const Geometry &point_,
         const Image &texture_ );

    // Flood-fill texture across pixels starting at target-pixel and
    // stopping at pixels matching specified border color.
    // Uses current fuzz setting when determining color match.
    floodFillTextureImage( const ssize_t x_,
                           const ssize_t y_,
         const Image &texture_,
         const Color &borderColor_ );

    floodFillTextureImage( const Geometry &point_,
         const Image &texture_,
         const Color &borderColor_ );

    void operator()( Image &image_ ) const;

  private:
    ssize_t  _x;
    ssize_t  _y;
    Image         _texture;
    Color         _borderColor;
  };

  // Flop image (reflect each scanline in the horizontal direction)
  class MagickDLLDecl flopImage : public std::unary_function<Image&,void>
  {
  public:
    flopImage( void );

    void operator()( Image &image_ ) const;

  private:
  };

  // Frame image
  class MagickDLLDecl frameImage : public std::unary_function<Image&,void>
  {
  public:
    frameImage( const Geometry &geometry_ = frameGeometryDefault );

    frameImage( const size_t width_, const size_t height_,
    const ssize_t innerBevel_ = 6, const ssize_t outerBevel_ = 6 );

    void operator()( Image &image_ ) const;

  private:
    size_t _width;
    size_t _height;
    ssize_t          _outerBevel;
    ssize_t          _innerBevel;
  };

  // Gamma correct image
  class MagickDLLDecl gammaImage : public std::unary_function<Image&,void>
  {
  public:
    gammaImage( const double gamma_ );

    gammaImage ( const double gammaRed_,
     const double gammaGreen_,
     const double gammaBlue_ );

    void operator()( Image &image_ ) const;

  private:
    double _gammaRed;
    double _gammaGreen;
    double _gammaBlue;
  };

  // Gaussian blur image
  // The number of neighbor pixels to be included in the convolution
  // mask is specified by 'width_'. The standard deviation of the
  // gaussian bell curve is specified by 'sigma_'.
  class MagickDLLDecl gaussianBlurImage : public std::unary_function<Image&,void>
  {
  public:
    gaussianBlurImage( const double width_, const double sigma_ );

    void operator()( Image &image_ ) const;

  private:
    double _width;
    double _sigma;
  };

  // Apply a color lookup table (Hald CLUT) to the image.
  class MagickDLLDecl haldClutImage : public std::unary_function<Image&,void>
  {
  public:
    haldClutImage( const Image &haldClutImage_ );

    void operator()( Image &image_ ) const;

  private:
    Image             _haldClutImage;
  };

  // Implode image (special effect)
  class MagickDLLDecl implodeImage : public std::unary_function<Image&,void>
  {
  public:
    implodeImage( const double factor_ = 50 );

    void operator()( Image &image_ ) const;

  private:
    double _factor;
  };

  // implements the inverse discrete Fourier transform (IFT) of the image
  // either as a magnitude / phase or real / imaginary image pair.
  class MagickDLLDecl inverseFourierTransformImage : public std::unary_function<Image&,void>
  {
  public:
    inverseFourierTransformImage( const Image &phaseImage_ );

    void operator()( Image &image_ ) const;

  private:
    Image _phaseImage;
  };

  // Set image validity. Valid images become empty (inValid) if
  // argument is false.
  class MagickDLLDecl isValidImage : public std::unary_function<Image&,void>
  {
  public:
    isValidImage( const bool isValid_ );

    void operator()( Image &image_ ) const;

  private:
    bool _isValid;
  };

  // Label image
  class MagickDLLDecl labelImage : public std::unary_function<Image&,void>
  {
  public:
    labelImage( const std::string &label_ );

    void operator()( Image &image_ ) const;

  private:
    std::string _label;
  };


  // Level image
  class MagickDLLDecl levelImage : public std::unary_function<Image&,void>
  {
  public:
    levelImage( const double black_point,
                const double white_point,
                const double mid_point=1.0 );

    void operator()( Image &image_ ) const;

  private:
    double _black_point;
    double _white_point;
    double _mid_point;
  };

  // Level image channel
  class MagickDLLDecl levelChannelImage : public std::unary_function<Image&,void>
  {
  public:
    levelChannelImage( const Magick::ChannelType channel,
                       const double black_point,
                       const double white_point,
                       const double mid_point=1.0 );

    void operator()( Image &image_ ) const;

  private:
    Magick::ChannelType _channel;
    double _black_point;
    double _white_point;
    double _mid_point;
  };

  // Magnify image by integral size
  class MagickDLLDecl magnifyImage : public std::unary_function<Image&,void>
  {
  public:
    magnifyImage( void );

    void operator()( Image &image_ ) const;

  private:
  };

  // Remap image colors with closest color from reference image
  class MagickDLLDecl mapImage : public std::unary_function<Image&,void>
  {
  public:
    mapImage( const Image &mapImage_ ,
              const bool dither_ = false );

    void operator()( Image &image_ ) const;

  private:
    Image   _mapImage;
    bool    _dither;
  };

  // Floodfill designated area with a matte value
  class MagickDLLDecl matteFloodfillImage : public std::unary_function<Image&,void>
  {
  public:
    matteFloodfillImage( const Color &target_ ,
       const unsigned int matte_,
       const ssize_t x_, const ssize_t y_,
       const PaintMethod method_ );

    void operator()( Image &image_ ) const;

  private:
    Color         _target;
    unsigned int  _matte;
    ssize_t           _x;
    ssize_t           _y;
    PaintMethod   _method;
  };

  // Filter image by replacing each pixel component with the median
  // color in a circular neighborhood
  class MagickDLLDecl medianFilterImage : public std::unary_function<Image&,void>
  {
  public:
    medianFilterImage( const double radius_ = 0.0 );

    void operator()( Image &image_ ) const;

  private:
    double _radius;
  };

  // Reduce image by integral size
  class MagickDLLDecl minifyImage : public std::unary_function<Image&,void>
  {
  public:
    minifyImage( void );

    void operator()( Image &image_ ) const;

  private:
  };

  // Modulate percent hue, saturation, and brightness of an image
  class MagickDLLDecl modulateImage : public std::unary_function<Image&,void>
  {
  public:
    modulateImage( const double brightness_,
       const double saturation_,
       const double hue_ );

    void operator()( Image &image_ ) const;

  private:
    double _brightness;
    double _saturation;
    double _hue;
  };

  // Negate colors in image.  Set grayscale to only negate grayscale
  // values in image.
  class MagickDLLDecl negateImage : public std::unary_function<Image&,void>
  {
  public:
    negateImage( const bool grayscale_ = false );

    void operator()( Image &image_ ) const;

  private:
    bool _grayscale;
  };

  // Normalize image (increase contrast by normalizing the pixel
  // values to span the full range of color values)
  class MagickDLLDecl normalizeImage : public std::unary_function<Image&,void>
  {
  public:
    normalizeImage( void );

    void operator()( Image &image_ ) const;

  private:
  };  

  // Oilpaint image (image looks like oil painting)
  class MagickDLLDecl oilPaintImage : public std::unary_function<Image&,void>
  {
  public:
    oilPaintImage( const double radius_ = 3 );

    void operator()( Image &image_ ) const;

  private:
    double _radius;
  };

  // Set or attenuate the image opacity channel. If the image pixels
  // are opaque then they are set to the specified opacity value,
  // otherwise they are blended with the supplied opacity value.  The
  // value of opacity_ ranges from 0 (completely opaque) to
  // QuantumRange. The defines OpaqueOpacity and TransparentOpacity are
  // available to specify completely opaque or completely transparent,
  // respectively.
  class MagickDLLDecl opacityImage : public std::unary_function<Image&,void>
  {
  public:
    opacityImage( const unsigned int opacity_ );

    void operator()( Image &image_ ) const;

  private:
    unsigned int _opacity;
  };

  // Change color of opaque pixel to specified pen color.
  class MagickDLLDecl opaqueImage : public std::unary_function<Image&,void>
  {
  public:
    opaqueImage( const Color &opaqueColor_,
     const Color &penColor_ );

    void operator()( Image &image_ ) const;

  private:
    Color  _opaqueColor;
    Color  _penColor;
  };

  // Quantize image (reduce number of colors)
  class MagickDLLDecl quantizeImage : public std::unary_function<Image&,void>
  {
  public:
    quantizeImage( const bool measureError_ = false );

    void operator()( Image &image_ ) const;

  private:
    bool _measureError;
  };

  // Raise image (lighten or darken the edges of an image to give a
  // 3-D raised or lowered effect)
  class MagickDLLDecl raiseImage : public std::unary_function<Image&,void>
  {
  public:
    raiseImage( const Geometry &geometry_ = raiseGeometryDefault,
    const bool raisedFlag_ = false );

    void operator()( Image &image_ ) const;

  private:
    Geometry   _geometry;
    bool       _raisedFlag;
  };

  // Reduce noise in image using a noise peak elimination filter
  class MagickDLLDecl reduceNoiseImage : public std::unary_function<Image&,void>
  {
  public:
    reduceNoiseImage( void );

    reduceNoiseImage (const  size_t order_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _order;
  };

  // Resize image to specified size.
  class MagickDLLDecl resizeImage : public std::unary_function<Image&,void>
  {
  public:
    resizeImage( const Geometry &geometry_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry _geometry;
  };

  // Roll image (rolls image vertically and horizontally) by specified
  // number of columnms and rows)
  class MagickDLLDecl rollImage : public std::unary_function<Image&,void>
  {
  public:
    rollImage( const Geometry &roll_ );

    rollImage( const ssize_t columns_, const ssize_t rows_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _columns;
    size_t _rows;
  };

  // Rotate image counter-clockwise by specified number of degrees.
  class MagickDLLDecl rotateImage : public std::unary_function<Image&,void>
  {
  public:
    rotateImage( const double degrees_ );

    void operator()( Image &image_ ) const;

  private:
    double       _degrees;
  };

  // Resize image by using pixel sampling algorithm
  class MagickDLLDecl sampleImage : public std::unary_function<Image&,void>
  {
  public:
    sampleImage( const Geometry &geometry_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry  _geometry;
  };  

  // Resize image by using simple ratio algorithm
  class MagickDLLDecl scaleImage : public std::unary_function<Image&,void>
  {
  public:
    scaleImage( const Geometry &geometry_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry  _geometry;
  };

  // Segment (coalesce similar image components) by analyzing the
  // histograms of the color components and identifying units that are
  // homogeneous with the fuzzy c-means technique.
  // Also uses QuantizeColorSpace and Verbose image attributes
  class MagickDLLDecl segmentImage : public std::unary_function<Image&,void>
  {
  public:
    segmentImage( const double clusterThreshold_ = 1.0, 
      const double smoothingThreshold_ = 1.5 );

    void operator()( Image &image_ ) const;

  private:
    double  _clusterThreshold;
    double  _smoothingThreshold;
  };

  // Shade image using distant light source
  class MagickDLLDecl shadeImage : public std::unary_function<Image&,void>
  {
  public:
    shadeImage( const double azimuth_ = 30,
    const double elevation_ = 30,
    const bool   colorShading_ = false );

    void operator()( Image &image_ ) const;

  private:
    double  _azimuth;
    double  _elevation;
    bool    _colorShading;
  };

  // Sharpen pixels in image
  class MagickDLLDecl sharpenImage : public std::unary_function<Image&,void>
  {
  public:
    sharpenImage( const double radius_ = 1, const double sigma_ = 0.5 );

    void operator()( Image &image_ ) const;

  private:
    double _radius;
    double _sigma;
  };

  // Shave pixels from image edges.
  class MagickDLLDecl shaveImage : public std::unary_function<Image&,void>
  {
  public:
    shaveImage( const Geometry &geometry_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry _geometry;
  };


  // Shear image (create parallelogram by sliding image by X or Y axis)
  class MagickDLLDecl shearImage : public std::unary_function<Image&,void>
  {
  public:
    shearImage( const double xShearAngle_,
    const double yShearAngle_ );

    void operator()( Image &image_ ) const;

  private:
    double _xShearAngle;
    double _yShearAngle;
  };

  // Solarize image (similar to effect seen when exposing a
  // photographic film to light during the development process)
  class MagickDLLDecl solarizeImage : public std::unary_function<Image&,void>
  {
  public:
    solarizeImage( const double factor_ );

    void operator()( Image &image_ ) const;

  private:
    double _factor;
  };

  // Splice the background color into the image.
  class MagickDLLDecl spliceImage : public std::unary_function<Image&,void>
  {
  public:
    spliceImage( const Geometry &geometry_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry _geometry;
  };

  // Spread pixels randomly within image by specified ammount
  class MagickDLLDecl spreadImage : public std::unary_function<Image&,void>
  {
  public:
    spreadImage( const size_t amount_ = 3 );

    void operator()( Image &image_ ) const;

  private:
    size_t _amount;
  };

  // Add a digital watermark to the image (based on second image)
  class MagickDLLDecl steganoImage : public std::unary_function<Image&,void>
  {
  public:
    steganoImage( const Image &waterMark_ );

    void operator()( Image &image_ ) const;

  private:
    Image _waterMark;
  };

  // Create an image which appears in stereo when viewed with red-blue glasses
  // (Red image on left, blue on right)
  class MagickDLLDecl stereoImage : public std::unary_function<Image&,void>
  {
  public:
    stereoImage( const Image &rightImage_ );

    void operator()( Image &image_ ) const;

  private:
    Image _rightImage;
  };

  // Color to use when drawing object outlines
  class MagickDLLDecl strokeColorImage : public std::unary_function<Image&,void>
  {
  public:
    strokeColorImage( const Color &strokeColor_ );

    void operator()( Image &image_ ) const;

  private:
    Color _strokeColor;
  };

  // Swirl image (image pixels are rotated by degrees)
  class MagickDLLDecl swirlImage : public std::unary_function<Image&,void>
  {
  public:
    swirlImage( const double degrees_ );

    void operator()( Image &image_ ) const;

  private:
    double _degrees;
  };

  // Channel a texture on image background
  class MagickDLLDecl textureImage : public std::unary_function<Image&,void>
  {
  public:
    textureImage( const Image &texture_ );

    void operator()( Image &image_ ) const;

  private:
    Image _texture;
  };

  // Threshold image
  class MagickDLLDecl thresholdImage : public std::unary_function<Image&,void>
  {
  public:
    thresholdImage( const double threshold_ );

    void operator()( Image &image_ ) const;

  private:
    double _threshold;
  };

  // Transform image based on image and crop geometries
  class MagickDLLDecl transformImage : public std::unary_function<Image&,void>
  {
  public:
    transformImage( const Geometry &imageGeometry_ );

    transformImage( const Geometry &imageGeometry_,
        const Geometry &cropGeometry_  );

    void operator()( Image &image_ ) const;

  private:
    Geometry _imageGeometry;
    Geometry _cropGeometry;
  };

  // Set image color to transparent
  class MagickDLLDecl transparentImage : public std::unary_function<Image&,void>
  {
  public:
    transparentImage( const Color& color_ );

    void operator()( Image &image_ ) const;

  private:
    Color _color;
  };

  // Trim edges that are the background color from the image
  class MagickDLLDecl trimImage : public std::unary_function<Image&,void>
  {
  public:
    trimImage( void );

    void operator()( Image &image_ ) const;

  private:
  };

  // Map image pixels to a sine wave
  class MagickDLLDecl waveImage : public std::unary_function<Image&,void>
  {
  public:
    waveImage( const double amplitude_ = 25.0,
         const double wavelength_ = 150.0 );

    void operator()( Image &image_ ) const;

  private:
    double _amplitude;
    double _wavelength;
  };

  // Zoom image to specified size.
  class MagickDLLDecl zoomImage : public std::unary_function<Image&,void>
  {
  public:
    zoomImage( const Geometry &geometry_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry _geometry;
  };

  //
  // Function object image attribute accessors
  //

  // Anti-alias Postscript and TrueType fonts (default true)
  class MagickDLLDecl antiAliasImage : public std::unary_function<Image&,void>
  {
  public:
    antiAliasImage( const bool flag_ );

    void operator()( Image &image_ ) const;

  private:
    bool _flag;
  };

  // Join images into a single multi-image file
  class MagickDLLDecl adjoinImage : public std::unary_function<Image&,void>
  {
  public:
    adjoinImage( const bool flag_ );

    void operator()( Image &image_ ) const;

  private:
    bool _flag;
  };

  // Time in 1/100ths of a second which must expire before displaying
  // the next image in an animated sequence.
  class MagickDLLDecl animationDelayImage : public std::unary_function<Image&,void>
  {
  public:
    animationDelayImage( const size_t delay_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _delay;
  };

  // Number of iterations to loop an animation (e.g. Netscape loop
  // extension) for.
  class MagickDLLDecl animationIterationsImage : public std::unary_function<Image&,void>
  {
  public:
    animationIterationsImage( const size_t iterations_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _iterations;
  };

  // Image background color
  class MagickDLLDecl backgroundColorImage : public std::unary_function<Image&,void>
  {
  public:
    backgroundColorImage( const Color &color_ );

    void operator()( Image &image_ ) const;

  private:
    Color _color;
  };

  // Name of texture image to tile onto the image background
  class MagickDLLDecl backgroundTextureImage : public std::unary_function<Image&,void>
  {
  public:
    backgroundTextureImage( const std::string &backgroundTexture_ );

    void operator()( Image &image_ ) const;

  private:
    std::string _backgroundTexture;
  };

  // Image border color
  class MagickDLLDecl borderColorImage : public std::unary_function<Image&,void>
  {
  public:
    borderColorImage( const Color &color_ );

    void operator()( Image &image_ ) const;

  private:
    Color _color;
  };

  // Text bounding-box base color (default none)
  class MagickDLLDecl boxColorImage : public std::unary_function<Image&,void>
  {
  public:
    boxColorImage( const Color &boxColor_ );

    void operator()( Image &image_ ) const;

  private:
    Color _boxColor;
  };

  // Chromaticity blue primary point (e.g. x=0.15, y=0.06)
  class MagickDLLDecl chromaBluePrimaryImage : public std::unary_function<Image&,void>
  {
  public:
    chromaBluePrimaryImage( const double x_, const double y_ );

    void operator()( Image &image_ ) const;

  private:
    double _x;
    double _y;
  };

  // Chromaticity green primary point (e.g. x=0.3, y=0.6)
  class MagickDLLDecl chromaGreenPrimaryImage : public std::unary_function<Image&,void>
  {
  public:
    chromaGreenPrimaryImage( const double x_, const double y_ );

    void operator()( Image &image_ ) const;

  private:
    double _x;
    double _y;
  };

  // Chromaticity red primary point (e.g. x=0.64, y=0.33)
  class MagickDLLDecl chromaRedPrimaryImage : public std::unary_function<Image&,void>
  {
  public:
    chromaRedPrimaryImage( const double x_, const double y_ );

    void operator()( Image &image_ ) const;

  private:
    double _x;
    double _y;
  };

  // Chromaticity white point (e.g. x=0.3127, y=0.329)
  class MagickDLLDecl chromaWhitePointImage : public std::unary_function<Image&,void>
  {
  public:
    chromaWhitePointImage( const double x_, const double y_ );

    void operator()( Image &image_ ) const;

  private:
    double _x;
    double _y;
  };

  // Colors within this distance are considered equal
  class MagickDLLDecl colorFuzzImage : public std::unary_function<Image&,void>
  {
  public:
    colorFuzzImage( const double fuzz_ );

    void operator()( Image &image_ ) const;

  private:
    double _fuzz;
  };

  // Color at colormap position index_
  class MagickDLLDecl colorMapImage : public std::unary_function<Image&,void>
  {
  public:
    colorMapImage( const size_t index_, const Color &color_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _index;
    Color        _color;
  };

  // Composition operator to be used when composition is implicitly used
  // (such as for image flattening).
  class MagickDLLDecl composeImage : public std::unary_function<Image&,void>
  {
  public:
    composeImage( const CompositeOperator compose_ );
                                                                                
    void operator()( Image &image_ ) const;
                                                                                
  private:
    CompositeOperator _compose;
  };

  // Compression type
  class MagickDLLDecl compressTypeImage : public std::unary_function<Image&,void>
  {
  public:
    compressTypeImage( const CompressionType compressType_ );

    void operator()( Image &image_ ) const;

  private:
    CompressionType _compressType;
  };

  // Vertical and horizontal resolution in pixels of the image
  class MagickDLLDecl densityImage : public std::unary_function<Image&,void>
  {
  public:
    densityImage( const Geometry &geomery_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry _geomery;
  };

  // Image depth (bits allocated to red/green/blue components)
  class MagickDLLDecl depthImage : public std::unary_function<Image&,void>
  {
  public:
    depthImage( const size_t depth_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _depth;
  };

  // Endianness (LSBEndian like Intel or MSBEndian like SPARC) for image
  // formats which support endian-specific options.
  class MagickDLLDecl endianImage : public std::unary_function<Image&,void>
  {
  public:
    endianImage( const EndianType endian_ );

    void operator()( Image &image_ ) const;

  private:
    EndianType  _endian;
  };

  // Image file name
  class MagickDLLDecl fileNameImage : public std::unary_function<Image&,void>
  {
  public:
    fileNameImage( const std::string &fileName_ );

    void operator()( Image &image_ ) const;

  private:
    std::string _fileName;
  };

  // Filter to use when resizing image
  class MagickDLLDecl filterTypeImage : public std::unary_function<Image&,void>
  {
  public:
    filterTypeImage( const FilterTypes filterType_ );

    void operator()( Image &image_ ) const;

  private:
    FilterTypes _filterType;
  };

  // Text rendering font
  class MagickDLLDecl fontImage : public std::unary_function<Image&,void>
  {
  public:
    fontImage( const std::string &font_ );

    void operator()( Image &image_ ) const;

  private:
    std::string _font;
  };

  // Font point size
  class MagickDLLDecl fontPointsizeImage : public std::unary_function<Image&,void>
  {
  public:
    fontPointsizeImage( const size_t pointsize_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _pointsize;
  };

  // GIF disposal method
  class MagickDLLDecl gifDisposeMethodImage : public std::unary_function<Image&,void>
  {
  public:
    gifDisposeMethodImage( const size_t disposeMethod_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _disposeMethod;
  };

  // Type of interlacing to use
  class MagickDLLDecl interlaceTypeImage : public std::unary_function<Image&,void>
  {
  public:
    interlaceTypeImage( const InterlaceType interlace_ );

    void operator()( Image &image_ ) const;

  private:
    InterlaceType _interlace;
  };

  // Linewidth for drawing vector objects (default one)
  class MagickDLLDecl lineWidthImage : public std::unary_function<Image&,void>
  {
  public:
    lineWidthImage( const double lineWidth_ );

    void operator()( Image &image_ ) const;

  private:
    double _lineWidth;
  };

  // File type magick identifier (.e.g "GIF")
  class MagickDLLDecl magickImage : public std::unary_function<Image&,void>
  {
  public:
    magickImage( const std::string &magick_ );

    void operator()( Image &image_ ) const;

  private:
    std::string _magick;
  };

  // Image supports transparent color
  class MagickDLLDecl matteImage : public std::unary_function<Image&,void>
  {
  public:
    matteImage( const bool matteFlag_ );

    void operator()( Image &image_ ) const;

  private:
    bool _matteFlag;
  };

  // Transparent color
  class MagickDLLDecl matteColorImage : public std::unary_function<Image&,void>
  {
  public:
    matteColorImage( const Color &matteColor_ );

    void operator()( Image &image_ ) const;

  private:
    Color _matteColor;
  };

  // Indicate that image is black and white
  class MagickDLLDecl monochromeImage : public std::unary_function<Image&,void>
  {
  public:
    monochromeImage( const bool monochromeFlag_ );

    void operator()( Image &image_ ) const;

  private:
    bool _monochromeFlag;
  };

  // Pen color
  class MagickDLLDecl penColorImage : public std::unary_function<Image&,void>
  {
  public:
    penColorImage( const Color &penColor_ );

    void operator()( Image &image_ ) const;

  private:
    Color _penColor;
  };

  // Pen texture image.
  class MagickDLLDecl penTextureImage : public std::unary_function<Image&,void>
  {
  public:
    penTextureImage( const Image &penTexture_ );

    void operator()( Image &image_ ) const;

  private:
    Image _penTexture;
  };

  // Set pixel color at location x & y.
  class MagickDLLDecl pixelColorImage : public std::unary_function<Image&,void>
  {
  public:
    pixelColorImage( const ssize_t x_,
                     const ssize_t y_,
         const Color &color_);

    void operator()( Image &image_ ) const;

  private:
    ssize_t _x;
    ssize_t _y;
    Color        _color;
  };

  // Postscript page size.
  class MagickDLLDecl pageImage : public std::unary_function<Image&,void>
  {
  public:
    pageImage( const Geometry &pageSize_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry _pageSize;
  };

  // JPEG/MIFF/PNG compression level (default 75).
  class MagickDLLDecl qualityImage : public std::unary_function<Image&,void>
  {
  public:
    qualityImage( const size_t quality_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _quality;
  };

  // Maximum number of colors to quantize to
  class MagickDLLDecl quantizeColorsImage : public std::unary_function<Image&,void>
  {
  public:
    quantizeColorsImage( const size_t colors_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _colors;
  };

  // Colorspace to quantize in.
  class MagickDLLDecl quantizeColorSpaceImage : public std::unary_function<Image&,void>
  {
  public:
    quantizeColorSpaceImage( const ColorspaceType colorSpace_ );

    void operator()( Image &image_ ) const;

  private:
    ColorspaceType _colorSpace;
  };

  // Dither image during quantization (default true).
  class MagickDLLDecl quantizeDitherImage : public std::unary_function<Image&,void>
  {
  public:
    quantizeDitherImage( const bool ditherFlag_ );

    void operator()( Image &image_ ) const;

  private:
    bool _ditherFlag;
  };

  // Quantization tree-depth
  class MagickDLLDecl quantizeTreeDepthImage : public std::unary_function<Image&,void>
  {
  public:
    quantizeTreeDepthImage( const size_t treeDepth_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _treeDepth;
  };

  // The type of rendering intent
  class MagickDLLDecl renderingIntentImage : public std::unary_function<Image&,void>
  {
  public:
    renderingIntentImage( const RenderingIntent renderingIntent_ );

    void operator()( Image &image_ ) const;

  private:
    RenderingIntent _renderingIntent;
  };

  // Units of image resolution
  class MagickDLLDecl resolutionUnitsImage : public std::unary_function<Image&,void>
  {
  public:
    resolutionUnitsImage( const ResolutionType resolutionUnits_ );

    void operator()( Image &image_ ) const;

  private:
    ResolutionType _resolutionUnits;
  };

  // Image scene number
  class MagickDLLDecl sceneImage : public std::unary_function<Image&,void>
  {
  public:
    sceneImage( const size_t scene_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _scene;
  };

  // adjust the image contrast with a non-linear sigmoidal contrast algorithm
  class MagickDLLDecl sigmoidalContrastImage : public std::unary_function<Image&,void>
  {
  public:
    sigmoidalContrastImage( const size_t sharpen_,
      const double contrast,
      const double midpoint = QuantumRange / 2.0 );

    void operator()( Image &image_ ) const;

  private:
    size_t _sharpen;
    double contrast;
    double midpoint;
  };

  // Width and height of a raw image
  class MagickDLLDecl sizeImage : public std::unary_function<Image&,void>
  {
  public:
    sizeImage( const Geometry &geometry_ );

    void operator()( Image &image_ ) const;

  private:
    Geometry _geometry;
  };

  // stripImage strips an image of all profiles and comments.
  class MagickDLLDecl stripImage : public std::unary_function<Image&,void>
  {
  public:
    stripImage( void );

    void operator()( Image &image_ ) const;

  private:
  };

  // Subimage of an image sequence
  class MagickDLLDecl subImageImage : public std::unary_function<Image&,void>
  {
  public:
    subImageImage( const size_t subImage_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _subImage;
  };

  // Number of images relative to the base image
  class MagickDLLDecl subRangeImage : public std::unary_function<Image&,void>
  {
  public:
    subRangeImage( const size_t subRange_ );

    void operator()( Image &image_ ) const;

  private:
    size_t _subRange;
  };

  // Tile name
  class MagickDLLDecl tileNameImage : public std::unary_function<Image&,void>
  {
  public:
    tileNameImage( const std::string &tileName_ );

    void operator()( Image &image_ ) const;

  private:
    std::string _tileName;
  };

  // Image storage type
  class MagickDLLDecl typeImage : public std::unary_function<Image&,void>
  {
  public:
    typeImage( const ImageType type_ );

    void operator()( Image &image_ ) const;

  private:
    Magick::ImageType _type;
  };


  // Print detailed information about the image
  class MagickDLLDecl verboseImage : public std::unary_function<Image&,void>
  {
  public:
    verboseImage( const bool verbose_ );

    void operator()( Image &image_ ) const;

  private:
    bool _verbose;
  };

  // FlashPix viewing parameters
  class MagickDLLDecl viewImage : public std::unary_function<Image&,void>
  {
  public:
    viewImage( const std::string &view_ );

    void operator()( Image &image_ ) const;

  private:
    std::string _view;
  };

  // X11 display to display to, obtain fonts from, or to capture
  // image from
  class MagickDLLDecl x11DisplayImage : public std::unary_function<Image&,void>
  {
  public:
    x11DisplayImage( const std::string &display_ );

    void operator()( Image &image_ ) const;

  private:
    std::string _display;
  };

  //////////////////////////////////////////////////////////
  //
  // Implementation template definitions. Not for end-use.
  //
  //////////////////////////////////////////////////////////

  // Link images together into an image list based on the ordering of
  // the container implied by the iterator. This step is done in
  // preparation for use with ImageMagick functions which operate on
  // lists of images.
  // Images are selected by range, first_ to last_ so that a subset of
  // the container may be selected.  Specify first_ via the
  // container's begin() method and last_ via the container's end()
  // method in order to specify the entire container.
  template <class InputIterator>
  void linkImages( InputIterator first_,
       InputIterator last_ ) {

    MagickCore::Image* previous = 0;
    ssize_t scene = 0;
    for ( InputIterator iter = first_; iter != last_; ++iter )
      {
  // Unless we reduce the reference count to one, the same image
  // structure may occur more than once in the container, causing
  // the linked list to fail.
  iter->modifyImage();

  MagickCore::Image* current = iter->image();

  current->previous = previous;
  current->next     = 0;

  if ( previous != 0)
    previous->next = current;

  current->scene=scene;
  ++scene;

  previous = current;
      }
  }

  // Remove links added by linkImages. This should be called after the
  // ImageMagick function call has completed to reset the image list
  // back to its pristine un-linked state.
  template <class InputIterator>
  void unlinkImages( InputIterator first_,
         InputIterator last_ ) {
    for( InputIterator iter = first_; iter != last_; ++iter )
      {
  MagickCore::Image* image = iter->image();
  image->previous = 0;
  image->next = 0;
      }
  }

  // Insert images in image list into existing container (appending to container)
  // The images should not be deleted since only the image ownership is passed.
  // The options are copied into the object.
  template <class Container>
  void insertImages( Container *sequence_,
         MagickCore::Image* images_ ) {
    MagickCore::Image *image = images_;
    if ( image )
      {
  do
    {
      MagickCore::Image* next_image = image->next;
      image->next = 0;
    
      if (next_image != 0)
        next_image->previous=0;
    
      sequence_->push_back( Magick::Image( image ) );
    
      image=next_image;
    } while( image );
      
  return;
      }
  }

  ///////////////////////////////////////////////////////////////////
  //
  // Template definitions for documented API
  //
  ///////////////////////////////////////////////////////////////////

  template <class InputIterator>
  void animateImages( InputIterator first_,
          InputIterator last_ ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    linkImages( first_, last_ );
    MagickCore::AnimateImages( first_->imageInfo(), first_->image() );
    MagickCore::GetImageException( first_->image(), &exceptionInfo );
    unlinkImages( first_, last_ );
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Append images from list into single image in either horizontal or
  // vertical direction.
  template <class InputIterator>
  void appendImages( Image *appendedImage_,
         InputIterator first_,
         InputIterator last_,
         bool stack_ = false) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    linkImages( first_, last_ );
    MagickCore::Image* image = MagickCore::AppendImages( first_->image(),
                   (MagickBooleanType) stack_,
                   &exceptionInfo ); 
    unlinkImages( first_, last_ );
    appendedImage_->replaceImage( image );
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Average a set of images.
  // All the input images must be the same size in pixels.
  template <class InputIterator>
  void averageImages( Image *averagedImage_,
          InputIterator first_,
          InputIterator last_ ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    linkImages( first_, last_ );
    MagickCore::Image* image = MagickCore::AverageImages( first_->image(),
              &exceptionInfo );
    unlinkImages( first_, last_ );
    averagedImage_->replaceImage( image );
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Merge a sequence of images.
  // This is useful for GIF animation sequences that have page
  // offsets and disposal methods. A container to contain
  // the updated image sequence is passed via the coalescedImages_
  // option.
  template <class InputIterator, class Container >
  void coalesceImages( Container *coalescedImages_,
                       InputIterator first_,
                       InputIterator last_ ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );

    // Build image list
    linkImages( first_, last_ );
    MagickCore::Image* images = MagickCore::CoalesceImages( first_->image(),
                                                          &exceptionInfo);
    // Unlink image list
    unlinkImages( first_, last_ );

    // Ensure container is empty
    coalescedImages_->clear();

    // Move images to container
    insertImages( coalescedImages_, images );

    // Report any error
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Return format coders matching specified conditions.
  //
  // The default (if no match terms are supplied) is to return all
  // available format coders.
  //
  // For example, to return all readable formats:
  //  list<CoderInfo> coderList;
  //  coderInfoList( &coderList, CoderInfo::TrueMatch, CoderInfo::AnyMatch, CoderInfo::AnyMatch)
  //
  template <class Container >
  void coderInfoList( Container *container_,
                      CoderInfo::MatchType isReadable_ = CoderInfo::AnyMatch,
                      CoderInfo::MatchType isWritable_ = CoderInfo::AnyMatch,
                      CoderInfo::MatchType isMultiFrame_ = CoderInfo::AnyMatch
                      ) {
    // Obtain first entry in MagickInfo list
    size_t number_formats;
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    char **coder_list =
      MagickCore::GetMagickList( "*", &number_formats, &exceptionInfo );
    if( !coder_list )
      {
        throwException( exceptionInfo );
        throwExceptionExplicit(MagickCore::MissingDelegateError,
                             "Coder array not returned!", 0 );
      }

    // Clear out container
    container_->clear();

    for ( ssize_t i=0; i < (ssize_t) number_formats; i++)
      {
        const MagickCore::MagickInfo *magick_info =
          MagickCore::GetMagickInfo( coder_list[i], &exceptionInfo );
        coder_list[i]=(char *)
          MagickCore::RelinquishMagickMemory( coder_list[i] );

        // Skip stealth coders
        if ( magick_info->stealth )
          continue;

        try {
          CoderInfo coderInfo( magick_info->name );

          // Test isReadable_
          if ( isReadable_ != CoderInfo::AnyMatch &&
               (( coderInfo.isReadable() && isReadable_ != CoderInfo::TrueMatch ) ||
                ( !coderInfo.isReadable() && isReadable_ != CoderInfo::FalseMatch )) )
            continue;

          // Test isWritable_
          if ( isWritable_ != CoderInfo::AnyMatch &&
               (( coderInfo.isWritable() && isWritable_ != CoderInfo::TrueMatch ) ||
                ( !coderInfo.isWritable() && isWritable_ != CoderInfo::FalseMatch )) )
            continue;

          // Test isMultiFrame_
          if ( isMultiFrame_ != CoderInfo::AnyMatch &&
               (( coderInfo.isMultiFrame() && isMultiFrame_ != CoderInfo::TrueMatch ) ||
                ( !coderInfo.isMultiFrame() && isMultiFrame_ != CoderInfo::FalseMatch )) )
            continue;

          // Append matches to container
          container_->push_back( coderInfo );
        }
        // Intentionally ignore missing module errors
        catch ( Magick::ErrorModule )
          {
            continue;
          }
      }
    coder_list=(char **) MagickCore::RelinquishMagickMemory( coder_list );
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  //
  // Fill container with color histogram.
  // Entries are of type "std::pair<Color,size_t>".  Use the pair
  // "first" member to access the Color and the "second" member to access
  // the number of times the color occurs in the image.
  //
  // For example:
  //
  //  Using <map>:
  //
  //  Image image("image.miff");
  //  map<Color,size_t> histogram;
  //  colorHistogram( &histogram, image );
  //  std::map<Color,size_t>::const_iterator p=histogram.begin();
  //  while (p != histogram.end())
  //    {
  //      cout << setw(10) << (int)p->second << ": ("
  //           << setw(quantum_width) << (int)p->first.redQuantum() << ","
  //           << setw(quantum_width) << (int)p->first.greenQuantum() << ","
  //           << setw(quantum_width) << (int)p->first.blueQuantum() << ")"
  //           << endl;
  //      p++;
  //    }
  //
  //  Using <vector>:
  //
  //  Image image("image.miff");
  //  std::vector<std::pair<Color,size_t> > histogram;
  //  colorHistogram( &histogram, image );
  //  std::vector<std::pair<Color,size_t> >::const_iterator p=histogram.begin();
  //  while (p != histogram.end())
  //    {
  //      cout << setw(10) << (int)p->second << ": ("
  //           << setw(quantum_width) << (int)p->first.redQuantum() << ","
  //           << setw(quantum_width) << (int)p->first.greenQuantum() << ","
  //           << setw(quantum_width) << (int)p->first.blueQuantum() << ")"
  //           << endl;
  //      p++;
  //    }

  template <class Container >
  void colorHistogram( Container *histogram_, const Image image)
  {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );

    // Obtain histogram array
    size_t colors;
    MagickCore::ColorPacket *histogram_array = 
      MagickCore::GetImageHistogram( image.constImage(), &colors, &exceptionInfo );
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );

    // Clear out container
    histogram_->clear();

    // Transfer histogram array to container
    for ( size_t i=0; i < colors; i++)
      {
        histogram_->insert(histogram_->end(),std::pair<const Color,size_t>
                           ( Color(histogram_array[i].pixel.red,
                                   histogram_array[i].pixel.green,
                                   histogram_array[i].pixel.blue),
                                   (size_t) histogram_array[i].count) );
      }
    
    // Deallocate histogram array
    histogram_array=(MagickCore::ColorPacket *)
      MagickCore::RelinquishMagickMemory(histogram_array);
  }
                      
  // Break down an image sequence into constituent parts.  This is
  // useful for creating GIF or MNG animation sequences.
  template <class InputIterator, class Container >
  void deconstructImages( Container *deconstructedImages_,
                          InputIterator first_,
                          InputIterator last_ ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );

    // Build image list
    linkImages( first_, last_ );
    MagickCore::Image* images = DeconstructImages( first_->image(),
                                                   &exceptionInfo);
    // Unlink image list
    unlinkImages( first_, last_ );

    // Ensure container is empty
    deconstructedImages_->clear();

    // Move images to container
    insertImages( deconstructedImages_, images );

    // Report any error
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  //
  // Display an image sequence
  //
  template <class InputIterator>
  void displayImages( InputIterator first_,
          InputIterator last_ ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    linkImages( first_, last_ );
    MagickCore::DisplayImages( first_->imageInfo(), first_->image() );
    MagickCore::GetImageException( first_->image(), &exceptionInfo );
    unlinkImages( first_, last_ );
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Merge a sequence of image frames which represent image layers.
  // This is useful for combining Photoshop layers into a single image.
  template <class InputIterator>
  void flattenImages( Image *flattendImage_,
          InputIterator first_,
          InputIterator last_ ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    linkImages( first_, last_ );
    MagickCore::Image* image = MagickCore::MergeImageLayers( first_->image(),
      FlattenLayer,&exceptionInfo );
    unlinkImages( first_, last_ );
    flattendImage_->replaceImage( image );
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Implements the discrete Fourier transform (DFT) of the image either as a
  // magnitude / phase or real / imaginary image pair.
  template <class Container >
  void forwardFourierTransformImage( Container *fourierImages_,
    const Image &image_ ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );

    // Build image list
    MagickCore::Image* images = ForwardFourierTransformImage(
      image_.constImage(), MagickTrue, &exceptionInfo);

    // Ensure container is empty
    fourierImages_->clear();

    // Move images to container
    insertImages( fourierImages_, images );

    // Report any error
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }
  template <class Container >
  void forwardFourierTransformImage( Container *fourierImages_,
    const Image &image_, const bool magnitude_ ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );

    // Build image list
    MagickCore::Image* images = ForwardFourierTransformImage(
      image_.constImage(), magnitude_ == true ? MagickTrue : MagickFalse,
      &exceptionInfo);

    // Ensure container is empty
    fourierImages_->clear();

    // Move images to container
    insertImages( fourierImages_, images );

    // Report any error
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Replace the colors of a sequence of images with the closest color
  // from a reference image.
  // Set dither_ to true to enable dithering.  Set measureError_ to
  // true in order to evaluate quantization error.
  template <class InputIterator>
  void mapImages( InputIterator first_,
      InputIterator last_,
      const Image& mapImage_,
      bool dither_ = false,
      bool measureError_ = false ) {

    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    MagickCore::QuantizeInfo quantizeInfo;
    MagickCore::GetQuantizeInfo( &quantizeInfo );
    quantizeInfo.dither = dither_ ? MagickCore::MagickTrue : MagickCore::MagickFalse;
    linkImages( first_, last_ );
    MagickCore::RemapImages( &quantizeInfo, first_->image(),
        mapImage_.constImage());
    MagickCore::GetImageException( first_->image(), &exceptionInfo );
    if ( exceptionInfo.severity != MagickCore::UndefinedException )
      {
        unlinkImages( first_, last_ );
        throwException( exceptionInfo );
      }

    MagickCore::Image* image = first_->image();
    while( image )
      {
        // Calculate quantization error
        if ( measureError_ )
          {
            MagickCore::GetImageQuantizeError( image );
            if ( image->exception.severity > MagickCore::UndefinedException )
              {
                unlinkImages( first_, last_ );
                throwException( exceptionInfo );
              }
          }
  
        // Udate DirectClass representation of pixels
        MagickCore::SyncImage( image );
        if ( image->exception.severity > MagickCore::UndefinedException )
          {
            unlinkImages( first_, last_ );
            throwException( exceptionInfo );
          }

        // Next image
        image=image->next;
      }

    unlinkImages( first_, last_ );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Create a composite image by combining several separate images.
  template <class Container, class InputIterator>
  void montageImages( Container *montageImages_,
          InputIterator first_,
          InputIterator last_,
          const Montage &montageOpts_ ) {

    MagickCore::MontageInfo* montageInfo =
      static_cast<MagickCore::MontageInfo*>(MagickCore::AcquireMagickMemory(sizeof(MagickCore::MontageInfo)));

    // Update montage options with those set in montageOpts_
    montageOpts_.updateMontageInfo( *montageInfo );

    // Update options which must transfer to image options
    if ( montageOpts_.label().length() != 0 )
      first_->label( montageOpts_.label() );

    // Create linked image list
    linkImages( first_, last_ );

    // Reset output container to pristine state
    montageImages_->clear();

    // Do montage
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    MagickCore::Image *images = MagickCore::MontageImages( first_->image(),
               montageInfo,
               &exceptionInfo );
    if ( images != 0 )
      {
  insertImages( montageImages_, images );
      }

    // Clean up any allocated data in montageInfo
    MagickCore::DestroyMontageInfo( montageInfo );

    // Unlink linked image list
    unlinkImages( first_, last_ );

    // Report any montage error
    throwException( exceptionInfo );

    // Apply transparency to montage images
    if ( montageImages_->size() > 0 && montageOpts_.transparentColor().isValid() )
      {
  for_each( first_, last_, transparentImage( montageOpts_.transparentColor() ) );
      }

    // Report any transparentImage() error
    MagickCore::GetImageException( first_->image(), &exceptionInfo );
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Morph a set of images
  template <class InputIterator, class Container >
  void morphImages( Container *morphedImages_,
        InputIterator first_,
        InputIterator last_,
        size_t frames_ ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );

    // Build image list
    linkImages( first_, last_ );
    MagickCore::Image* images = MagickCore::MorphImages( first_->image(), frames_,
                   &exceptionInfo);
    // Unlink image list
    unlinkImages( first_, last_ );

    // Ensure container is empty
    morphedImages_->clear();

    // Move images to container
    insertImages( morphedImages_, images );

    // Report any error
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Inlay a number of images to form a single coherent picture.
  template <class InputIterator>
  void mosaicImages( Image *mosaicImage_,
         InputIterator first_,
         InputIterator last_ ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    linkImages( first_, last_ );
    MagickCore::Image* image = MagickCore::MergeImageLayers( first_->image(),
       MosaicLayer,&exceptionInfo ); 
    unlinkImages( first_, last_ );
    mosaicImage_->replaceImage( image );
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Quantize colors in images using current quantization settings
  // Set measureError_ to true in order to measure quantization error
  template <class InputIterator>
  void quantizeImages( InputIterator first_,
           InputIterator last_,
           bool measureError_ = false ) {
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );

    linkImages( first_, last_ );

    MagickCore::QuantizeImages( first_->quantizeInfo(),
             first_->image() );
    MagickCore::GetImageException( first_->image(), &exceptionInfo );
    if ( exceptionInfo.severity > MagickCore::UndefinedException )
      {
  unlinkImages( first_, last_ );
  throwException( exceptionInfo );
      }

    MagickCore::Image* image = first_->image();
    while( image != 0 )
      {
  // Calculate quantization error
  if ( measureError_ )
    MagickCore::GetImageQuantizeError( image );

  // Update DirectClass representation of pixels
  MagickCore::SyncImage( image );

  // Next image
  image=image->next;
      }

    unlinkImages( first_, last_ );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Read images into existing container (appending to container)
  // FIXME: need a way to specify options like size, depth, and density.
  template <class Container>
  void readImages( Container *sequence_,
       const std::string &imageSpec_ ) {
    MagickCore::ImageInfo *imageInfo = MagickCore::CloneImageInfo(0);
    imageSpec_.copy( imageInfo->filename, MaxTextExtent-1 );
    imageInfo->filename[ imageSpec_.length() ] = 0;
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    MagickCore::Image* images =  MagickCore::ReadImage( imageInfo, &exceptionInfo );
    MagickCore::DestroyImageInfo(imageInfo);
    insertImages( sequence_, images);
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }
  template <class Container>
  void readImages( Container *sequence_,
       const Blob &blob_ ) {
    MagickCore::ImageInfo *imageInfo = MagickCore::CloneImageInfo(0);
    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    MagickCore::Image *images = MagickCore::BlobToImage( imageInfo,
                   blob_.data(),
                   blob_.length(), &exceptionInfo );
    MagickCore::DestroyImageInfo(imageInfo);
    insertImages( sequence_, images );
    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

  // Write Images
  template <class InputIterator>
  void writeImages( InputIterator first_,
        InputIterator last_,
        const std::string &imageSpec_,
        bool adjoin_ = true ) {

    first_->adjoin( adjoin_ );

    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );

    linkImages( first_, last_ );
    ssize_t errorStat = MagickCore::WriteImages( first_->constImageInfo(),
                                            first_->image(),
                                            imageSpec_.c_str(),
                                            &exceptionInfo );
    unlinkImages( first_, last_ );

    if ( errorStat != false )
      {
        (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
        return;
      }

    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }
  // Write images to BLOB
  template <class InputIterator>
  void writeImages( InputIterator first_,
        InputIterator last_,
        Blob *blob_,
        bool adjoin_ = true) {

    first_->adjoin( adjoin_ );

    linkImages( first_, last_ );

    MagickCore::ExceptionInfo exceptionInfo;
    MagickCore::GetExceptionInfo( &exceptionInfo );
    size_t length = 2048; // Efficient size for small images
    void* data = MagickCore::ImagesToBlob( first_->imageInfo(),
           first_->image(),
           &length,
           &exceptionInfo);
    blob_->updateNoCopy( data, length, Magick::Blob::MallocAllocator );

    unlinkImages( first_, last_ );

    throwException( exceptionInfo );
    (void) MagickCore::DestroyExceptionInfo( &exceptionInfo );
  }

} // namespace Magick

#endif // Magick_STL_header

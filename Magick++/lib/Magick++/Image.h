// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
// Copyright Dirk Lemstra 2013-2017
//
// Definition of Image, the representation of a single image in Magick++
//

#if !defined(Magick_Image_header)
#define Magick_Image_header

#include "Magick++/Include.h"
#include <string>
#include <list>
#include "Magick++/Blob.h"
#include "Magick++/Color.h"
#include "Magick++/Drawable.h"
#include "Magick++/Exception.h"
#include "Magick++/Geometry.h"
#include "Magick++/Statistic.h"
#include "Magick++/TypeMetric.h"

namespace Magick
{
  // Forward declarations
  class Options;
  class ImageRef;

  extern MagickPPExport const char *borderGeometryDefault;
  extern MagickPPExport const char *frameGeometryDefault;
  extern MagickPPExport const char *raiseGeometryDefault;

  // Compare two Image objects regardless of LHS/RHS
  // Image sizes and signatures are used as basis of comparison
  MagickPPExport int operator ==
    (const Magick::Image &left_,const Magick::Image &right_);
  MagickPPExport int operator !=
    (const Magick::Image &left_,const Magick::Image &right_);
  MagickPPExport int operator >
    (const Magick::Image &left_,const Magick::Image &right_);
  MagickPPExport int operator <
    (const Magick::Image &left_,const Magick::Image &right_);
  MagickPPExport int operator >=
    (const Magick::Image &left_,const Magick::Image &right_);
  MagickPPExport int operator <=
    (const Magick::Image &left_,const Magick::Image &right_);

  //
  // Image is the representation of an image. In reality, it actually
  // a handle object which contains a pointer to a shared reference
  // object (ImageRef). As such, this object is extremely space efficient.
  //
  class MagickPPExport Image
  {
  public:

    // Default constructor
    Image(void);

    // Construct Image from in-memory BLOB
    Image(const Blob &blob_);

    // Construct Image of specified size from in-memory BLOB
    Image(const Blob &blob_,const Geometry &size_);

    // Construct Image of specified size and depth from in-memory BLOB
    Image(const Blob &blob_,const Geometry &size_,const size_t depth_);

    // Construct Image of specified size, depth, and format from
    // in-memory BLOB
    Image(const Blob &blob_,const Geometry &size_,const size_t depth_,
      const std::string &magick_);

    // Construct Image of specified size, and format from in-memory BLOB
    Image(const Blob &blob_,const Geometry &size_,const std::string &magick_);

    // Construct a blank image canvas of specified size and color
    Image(const Geometry &size_,const Color &color_);

    // Copy constructor
    Image(const Image &image_);

    // Copy constructor to copy part of the image
    Image(const Image &image_,const Geometry &geometry_);

    // Construct an image based on an array of raw pixels, of
    // specified type and mapping, in memory
    Image(const size_t width_,const size_t height_,const std::string &map_,
      const StorageType type_,const void *pixels_);

    // Construct from image file or image specification
    Image(const std::string &imageSpec_);

    // Destructor
    virtual ~Image();

    // Assignment operator
    Image& operator=(const Image &image_);

    // Join images into a single multi-image file
    void adjoin(const bool flag_);
    bool adjoin(void) const;

    // Image supports transparency (alpha channel)
    void alpha(const bool alphaFlag_);
    bool alpha(void) const;

    // Transparent color
    void matteColor(const Color &matteColor_);
    Color matteColor(void) const;

    // Time in 1/100ths of a second which must expire before
    // displaying the next image in an animated sequence.
    void animationDelay(const size_t delay_);
    size_t animationDelay(void) const;

    // Number of iterations to loop an animation (e.g. Netscape loop
    // extension) for.
    void animationIterations(const size_t iterations_);
    size_t animationIterations(void) const;

    // Image background color
    void backgroundColor(const Color &color_);
    Color backgroundColor(void) const;

    // Name of texture image to tile onto the image background
    void backgroundTexture(const std::string &backgroundTexture_);
    std::string backgroundTexture(void) const;

    // Base image width (before transformations)
    size_t baseColumns(void) const;

    // Base image filename (before transformations)
    std::string baseFilename(void) const;

    // Base image height (before transformations)
    size_t baseRows(void) const;

    // Use black point compensation.
    void blackPointCompensation(const bool flag_);
    bool blackPointCompensation(void) const;

    // Image border color
    void borderColor(const Color &color_);
    Color borderColor(void) const;

    // Return smallest bounding box enclosing non-border pixels. The
    // current fuzz value is used when discriminating between pixels.
    // This is the crop bounding box used by crop(Geometry(0,0));
    Geometry boundingBox(void) const;

    // Text bounding-box base color (default none)
    void boxColor(const Color &boxColor_);
    Color boxColor(void) const;

    // Set or obtain modulus channel depth
    void channelDepth(const ChannelType channel_,const size_t depth_);
    size_t channelDepth(const ChannelType channel_);

    // Returns the number of channels in this image.
    size_t channels() const;

    // Image class (DirectClass or PseudoClass)
    // NOTE: setting a DirectClass image to PseudoClass will result in
    // the loss of color information if the number of colors in the
    // image is greater than the maximum palette size (either 256 or
    // 65536 entries depending on the value of MAGICKCORE_QUANTUM_DEPTH when
    // ImageMagick was built).
    void classType(const ClassType class_);
    ClassType classType(void) const;

    // Colors within this distance are considered equal
    void colorFuzz(const double fuzz_);
    double colorFuzz(void) const;

    // Colormap size (number of colormap entries)
    void colorMapSize(const size_t entries_);
    size_t colorMapSize(void) const;

    // Image Color Space
    void colorSpace(const ColorspaceType colorSpace_);
    ColorspaceType colorSpace(void) const;

    void colorSpaceType(const ColorspaceType colorSpace_);
    ColorspaceType colorSpaceType(void) const;

    // Image width
    size_t columns(void) const;

    // Comment image (add comment string to image)
    void comment(const std::string &comment_);
    std::string comment(void) const;

    // Composition operator to be used when composition is implicitly
    // used (such as for image flattening).
    void compose(const CompositeOperator compose_);
    CompositeOperator compose(void) const;

    // Compression type
    void compressType(const CompressionType compressType_);
    CompressionType compressType(void) const;

    // Enable printing of debug messages from ImageMagick
    void debug(const bool flag_);
    bool debug(void) const;

    // Vertical and horizontal resolution in pixels of the image
    void density(const Point &density_);
    Point density(void) const;

    // Image depth (bits allocated to red/green/blue components)
    void depth(const size_t depth_);
    size_t depth(void) const;

    // Tile names from within an image montage
    std::string directory(void) const;

    // Endianness (little like Intel or big like SPARC) for image
    // formats which support endian-specific options.
    void endian(const EndianType endian_);
    EndianType endian(void) const;

    // Exif profile (BLOB)
    void exifProfile(const Blob &exifProfile_);
    Blob exifProfile(void) const; 

    // Image file name
    void fileName(const std::string &fileName_);
    std::string fileName(void) const;

    // Number of bytes of the image on disk
    MagickSizeType fileSize(void) const;

    // Color to use when filling drawn objects
    void fillColor(const Color &fillColor_);
    Color fillColor(void) const;

    // Rule to use when filling drawn objects
    void fillRule(const FillRule &fillRule_);
    FillRule fillRule(void) const;

    // Pattern to use while filling drawn objects.
    void fillPattern(const Image &fillPattern_);
    Image fillPattern(void) const;

    // Filter to use when resizing image
    void filterType(const FilterType filterType_);
    FilterType filterType(void) const;

    // Text rendering font
    void font(const std::string &font_);
    std::string font(void) const;

    // Font family
    void fontFamily(const std::string &family_);
    std::string fontFamily(void) const;

    // Font point size
    void fontPointsize(const double pointSize_);
    double fontPointsize(void) const;

    // Font style
    void fontStyle(const StyleType style_);
    StyleType fontStyle(void) const;

    // Font weight
    void fontWeight(const size_t weight_);
    size_t fontWeight(void) const;

    // Long image format description
    std::string format(void) const;

    // Formats the specified expression
    // More info here: https://imagemagick.org/script/escape.php
    std::string formatExpression(const std::string expression);

    // Gamma level of the image
    double gamma(void) const;

    // Preferred size of the image when encoding
    Geometry geometry(void) const;

    // GIF disposal method
    void gifDisposeMethod(const DisposeType disposeMethod_);
    DisposeType gifDisposeMethod(void) const;

    bool hasChannel(const PixelChannel channel) const;

    // When comparing images, emphasize pixel differences with this color.
    void highlightColor(const Color color_);

    // ICC color profile (BLOB)
    void iccColorProfile(const Blob &colorProfile_);
    Blob iccColorProfile(void) const;

    // Type of interlacing to use
    void interlaceType(const InterlaceType interlace_);
    InterlaceType interlaceType(void) const;

    // Pixel color interpolation method to use
    void interpolate(const PixelInterpolateMethod interpolate_);
    PixelInterpolateMethod interpolate(void) const;

    // IPTC profile (BLOB)
    void iptcProfile(const Blob &iptcProfile_);
    Blob iptcProfile(void) const;

    // Returns true if none of the pixels in the image have an alpha value
    // other than OpaqueAlpha (QuantumRange).
    bool isOpaque(void) const;

    // Does object contain valid image?
    void isValid(const bool isValid_);
    bool isValid(void) const;

    // Image label
    void label(const std::string &label_);
    std::string label(void) const;

    // When comparing images, de-emphasize pixel differences with this color.
    void lowlightColor(const Color color_);

    // File type magick identifier (.e.g "GIF")
    void magick(const std::string &magick_);
    std::string magick(void) const;

    // When comparing images, set pixels with a read mask to this color.
    void masklightColor(const Color color_);

    // The mean error per pixel computed when an image is color reduced
    double meanErrorPerPixel(void) const;

    // Image modulus depth (minimum number of bits required to support
    // red/green/blue components without loss of accuracy)
    void modulusDepth(const size_t modulusDepth_);
    size_t modulusDepth(void) const;

    // Transform image to black and white
    void monochrome(const bool monochromeFlag_);
    bool monochrome(void) const;

    // Tile size and offset within an image montage
    Geometry montageGeometry(void) const;

    // The normalized max error per pixel computed when an image is
    // color reduced.
    double normalizedMaxError(void) const;

    // The normalized mean error per pixel computed when an image is
    // color reduced.
    double normalizedMeanError(void) const;

    // Image orientation
    void orientation(const OrientationType orientation_);
    OrientationType orientation(void) const;

    // Preferred size and location of an image canvas.
    void page(const Geometry &pageSize_);
    Geometry page(void) const;

    // JPEG/MIFF/PNG compression level (default 75).
    void quality(const size_t quality_);
    size_t quality(void) const;

    // Maximum number of colors to quantize to
    void quantizeColors(const size_t colors_);
    size_t quantizeColors(void) const;

    // Colorspace to quantize in.
    void quantizeColorSpace(const ColorspaceType colorSpace_);
    ColorspaceType quantizeColorSpace(void) const;

    // Dither image during quantization (default true).
    void quantizeDither(const bool ditherFlag_);
    bool quantizeDither(void) const;

    // Dither method
    void quantizeDitherMethod(const DitherMethod ditherMethod_);
    DitherMethod quantizeDitherMethod(void) const; 

    // Quantization tree-depth
    void quantizeTreeDepth(const size_t treeDepth_);
    size_t quantizeTreeDepth(void) const;

    // Suppress all warning messages. Error messages are still reported.
    void quiet(const bool quiet_);
    bool quiet(void) const;

    // The type of rendering intent
    void renderingIntent(const RenderingIntent renderingIntent_);
    RenderingIntent renderingIntent(void) const;

    // Units of image resolution
    void resolutionUnits(const ResolutionType resolutionUnits_);
    ResolutionType resolutionUnits(void) const;

    // The number of pixel rows in the image
    size_t rows(void) const;

    // Image sampling factor
    void samplingFactor(const std::string &samplingFactor_);
    std::string samplingFactor(void) const;

    // Image scene number
    void scene(const size_t scene_);
    size_t scene(void) const;

    // Width and height of a raw image 
    void size(const Geometry &geometry_);
    Geometry size(void) const;

    // enabled/disable stroke anti-aliasing
    void strokeAntiAlias(const bool flag_);
    bool strokeAntiAlias(void) const;

    // Color to use when drawing object outlines
    void strokeColor(const Color &strokeColor_);
    Color strokeColor(void) const;

    // Specify the pattern of dashes and gaps used to stroke
    // paths. The strokeDashArray represents a zero-terminated array
    // of numbers that specify the lengths of alternating dashes and
    // gaps in pixels. If an odd number of values is provided, then
    // the list of values is repeated to yield an even number of
    // values.  A typical strokeDashArray_ array might contain the
    // members 5 3 2 0, where the zero value indicates the end of the
    // pattern array.
    void strokeDashArray(const double *strokeDashArray_);
    const double *strokeDashArray(void) const;

    // While drawing using a dash pattern, specify distance into the
    // dash pattern to start the dash (default 0).
    void strokeDashOffset(const double strokeDashOffset_);
    double strokeDashOffset(void) const;

    // Specify the shape to be used at the end of open subpaths when
    // they are stroked. Values of LineCap are UndefinedCap, ButtCap,
    // RoundCap, and SquareCap.
    void strokeLineCap(const LineCap lineCap_);
    LineCap strokeLineCap(void) const;

    // Specify the shape to be used at the corners of paths (or other
    // vector shapes) when they are stroked. Values of LineJoin are
    // UndefinedJoin, MiterJoin, RoundJoin, and BevelJoin.
    void strokeLineJoin(const LineJoin lineJoin_);
    LineJoin strokeLineJoin(void) const;

    // Specify miter limit. When two line segments meet at a sharp
    // angle and miter joins have been specified for 'lineJoin', it is
    // possible for the miter to extend far beyond the thickness of
    // the line stroking the path. The miterLimit' imposes a limit on
    // the ratio of the miter length to the 'lineWidth'. The default
    // value of this parameter is 4.
    void strokeMiterLimit(const size_t miterLimit_);
    size_t strokeMiterLimit(void) const;

    // Pattern image to use while stroking object outlines.
    void strokePattern(const Image &strokePattern_);
    Image strokePattern(void) const;

    // Stroke width for drawing vector objects (default one)
    void strokeWidth(const double strokeWidth_);
    double strokeWidth(void) const;

    // Subimage of an image sequence
    void subImage(const size_t subImage_);
    size_t subImage(void) const;

    // Number of images relative to the base image
    void subRange(const size_t subRange_);
    size_t subRange(void) const;

    // Anti-alias Postscript and TrueType fonts (default true)
    void textAntiAlias(const bool flag_);
    bool textAntiAlias(void) const;

    // Render text right-to-left or left-to-right.
    void textDirection(DirectionType direction_);
    DirectionType textDirection() const;

    // Annotation text encoding (e.g. "UTF-16")
    void textEncoding(const std::string &encoding_);
    std::string textEncoding(void) const;

    // Text gravity.
    void textGravity(GravityType gravity_);
    GravityType textGravity() const;

    // Text inter-line spacing
    void textInterlineSpacing(double spacing_);
    double textInterlineSpacing(void) const;

    // Text inter-word spacing
    void textInterwordSpacing(double spacing_);
    double textInterwordSpacing(void) const;

    // Text inter-character kerning
    void textKerning(double kerning_);
    double textKerning(void) const;

    // Text undercolor box
    void textUnderColor(const Color &underColor_);
    Color textUnderColor(void) const;

    // Number of colors in the image
    size_t totalColors(void) const;

    // Rotation to use when annotating with text or drawing
    void transformRotation(const double angle_);

    // Skew to use in X axis when annotating with text or drawing
    void transformSkewX(const double skewx_);

    // Skew to use in Y axis when annotating with text or drawing
    void transformSkewY(const double skewy_);

    // Image representation type (also see type operation)
    //   Available types:
    //    Bilevel         PaletteBilevelAlpha
    //    Grayscale       GrayscaleAlpha
    //    Palette         PaletteAlpha
    //    TrueColor       TrueColorAlpha
    //    ColorSeparation ColorSeparationAlpha
    void type(const ImageType type_);
    ImageType type(void) const;

    // Print detailed information about the image
    void verbose(const bool verboseFlag_);
    bool verbose(void) const;

    // Virtual pixel method
    void virtualPixelMethod(const VirtualPixelMethod virtualPixelMethod_);
    VirtualPixelMethod virtualPixelMethod(void) const;

    // X11 display to display to, obtain fonts from, or to capture
    // image from
    void x11Display(const std::string &display_);
    std::string x11Display(void) const;

    // x resolution of the image
    double xResolution(void) const;

    // y resolution of the image
    double yResolution(void) const;

    // Adaptive-blur image with specified blur factor
    // The radius_ parameter specifies the radius of the Gaussian, in
    // pixels, not counting the center pixel.  The sigma_ parameter
    // specifies the standard deviation of the Laplacian, in pixels.
    void adaptiveBlur(const double radius_=0.0,const double sigma_=1.0);

    // This is shortcut function for a fast interpolative resize using mesh
    // interpolation.  It works well for small resizes of less than +/- 50%
    // of the original image size.  For larger resizing on images a full
    // filtered and slower resize function should be used instead.
    void adaptiveResize(const Geometry &geometry_);

    // Adaptively sharpens the image by sharpening more intensely near image
    // edges and less intensely far from edges. We sharpen the image with a 
    // Gaussian operator of the given radius and standard deviation (sigma).
    // For reasonable results, radius should be larger than sigma.
    void adaptiveSharpen(const double radius_=0.0,const double sigma_=1.0);
    void adaptiveSharpenChannel(const ChannelType channel_,
      const double radius_=0.0,const double sigma_=1.0);

    // Local adaptive threshold image
    // http://www.dai.ed.ac.uk/HIPR2/adpthrsh.htm
    // Width x height define the size of the pixel neighborhood
    // bias = constant to subtract from pixel neighborhood mean
    void adaptiveThreshold(const size_t width_,const size_t height_,
      const double bias_=0.0);

    // Add noise to image with specified noise type
    void addNoise(const NoiseType noiseType_,const double attenuate_=1.0);
    void addNoiseChannel(const ChannelType channel_,
      const NoiseType noiseType_,const double attenuate_=1.0);

    // Transform image by specified affine (or free transform) matrix.
    void affineTransform(const DrawableAffine &affine);

    // Set or attenuate the alpha channel in the image. If the image
    // pixels are opaque then they are set to the specified alpha
    // value, otherwise they are blended with the supplied alpha
    // value.  The value of alpha_ ranges from 0 (completely opaque)
    // to QuantumRange. The defines OpaqueAlpha and TransparentAlpha are
    // available to specify completely opaque or completely
    // transparent, respectively.
    void alpha(const unsigned int alpha_);

    // AlphaChannel() activates, deactivates, resets, or sets the alpha
    // channel.
    void alphaChannel(AlphaChannelOption alphaOption_);

    //
    // Annotate image (draw text on image)
    //
    // Gravity effects text placement in bounding area according to rules:
    //  NorthWestGravity  text bottom-left corner placed at top-left
    //  NorthGravity      text bottom-center placed at top-center
    //  NorthEastGravity  text bottom-right corner placed at top-right
    //  WestGravity       text left-center placed at left-center
    //  CenterGravity     text center placed at center
    //  EastGravity       text right-center placed at right-center
    //  SouthWestGravity  text top-left placed at bottom-left
    //  SouthGravity      text top-center placed at bottom-center
    //  SouthEastGravity  text top-right placed at bottom-right

    // Annotate using specified text, and placement location
    void annotate(const std::string &text_,const Geometry &location_);

    // Annotate using specified text, bounding area, and placement
    // gravity
    void annotate(const std::string &text_,const Geometry &boundingArea_,
      const GravityType gravity_);

    // Annotate with text using specified text, bounding area,
    // placement gravity, and rotation.
    void annotate(const std::string &text_,const Geometry &boundingArea_,
      const GravityType gravity_,const double degrees_);

    // Annotate with text (bounding area is entire image) and placement
    // gravity.
    void annotate(const std::string &text_,const GravityType gravity_);

    // Inserts the artifact with the specified name and value into
    // the artifact tree of the image.
    void artifact(const std::string &name_,const std::string &value_);

    // Returns the value of the artifact with the specified name.
    std::string artifact(const std::string &name_) const;

    // Access/Update a named image attribute
    void attribute(const std::string name_,const char *value_);
    void attribute(const std::string name_,const std::string value_);
    std::string attribute(const std::string name_) const;

    // Extracts the 'mean' from the image and adjust the image to try
    // make set its gamma appropriatally.
    void autoGamma(void);
    void autoGammaChannel(const ChannelType channel_);

    // Adjusts the levels of a particular image channel by scaling the
    // minimum and maximum values to the full quantum range.
    void autoLevel(void);
    void autoLevelChannel(const ChannelType channel_);

    // Adjusts an image so that its orientation is suitable for viewing.
    void autoOrient(void);

    // Automatically selects a threshold and replaces each pixel in the image
    // with a black pixel if the image intentsity is less than the selected
    // threshold otherwise white.
    void autoThreshold(const AutoThresholdMethod method_);

    // Forces all pixels below the threshold into black while leaving all
    // pixels at or above the threshold unchanged.
    void blackThreshold(const std::string &threshold_);
    void blackThresholdChannel(const ChannelType channel_,
      const std::string &threshold_);

     // Simulate a scene at nighttime in the moonlight.
    void blueShift(const double factor_=1.5);

    // Blur image with specified blur factor
    // The radius_ parameter specifies the radius of the Gaussian, in
    // pixels, not counting the center pixel.  The sigma_ parameter
    // specifies the standard deviation of the Laplacian, in pixels.
    void blur(const double radius_=0.0,const double sigma_=1.0);
    void blurChannel(const ChannelType channel_,const double radius_=0.0,
      const double sigma_=1.0);

    // Border image (add border to image)
    void border(const Geometry &geometry_=borderGeometryDefault);

    // Changes the brightness and/or contrast of an image. It converts the
    // brightness and contrast parameters into slope and intercept and calls
    // a polynomical function to apply to the image.
    void brightnessContrast(const double brightness_=0.0,
      const double contrast_=0.0);
    void brightnessContrastChannel(const ChannelType channel_,
      const double brightness_=0.0,const double contrast_=0.0);

    // Uses a multi-stage algorithm to detect a wide range of edges in images.
    void cannyEdge(const double radius_=0.0,const double sigma_=1.0,
      const double lowerPercent_=0.1,const double upperPercent_=0.3);

    // Accepts a lightweight Color Correction Collection
    // (CCC) file which solely contains one or more color corrections and
    // applies the correction to the image.
    void cdl(const std::string &cdl_);

    // Extract channel from image
    void channel(const ChannelType channel_);

    // Charcoal effect image (looks like charcoal sketch)
    // The radius_ parameter specifies the radius of the Gaussian, in
    // pixels, not counting the center pixel.  The sigma_ parameter
    // specifies the standard deviation of the Laplacian, in pixels.
    void charcoal(const double radius_=0.0,const double sigma_=1.0);
    void charcoalChannel(const ChannelType channel_,const double radius_=0.0,
      const double sigma_=1.0);

    // Chop image (remove vertical or horizontal subregion of image)
    // FIXME: describe how geometry argument is used to select either
    // horizontal or vertical subregion of image.
    void chop(const Geometry &geometry_);

    // Chromaticity blue primary point.
    void chromaBluePrimary(const double x_,const double y_,const double z_);
    void chromaBluePrimary(double *x_,double *y_,double *z_) const;
    
    // Chromaticity green primary point.
    void chromaGreenPrimary(const double x_,const double y_,const double z_);
    void chromaGreenPrimary(double *x_,double *y_,double *z_) const;
    
    // Chromaticity red primary point.
    void chromaRedPrimary(const double x_,const double y_,const double z_);
    void chromaRedPrimary(double *x_,double *y_,double *z_) const;
    
    // Chromaticity white point.
    void chromaWhitePoint(const double x_,const double y_,const double z_);
    void chromaWhitePoint(double *x_,double *y_,double *z_) const;

    // Set each pixel whose value is below zero to zero and any the
    // pixel whose value is above the quantum range to the quantum range (e.g.
    // 65535) otherwise the pixel value remains unchanged.
    void clamp(void);
    void clampChannel(const ChannelType channel_);

    // Sets the image clip mask based on any clipping path information
    // if it exists.
    void clip(void);
    void clipPath(const std::string pathname_,const bool inside_);

    // Apply a color lookup table (CLUT) to the image.
    void clut(const Image &clutImage_,const PixelInterpolateMethod method);
    void clutChannel(const ChannelType channel_,const Image &clutImage_,
      const PixelInterpolateMethod method);

    // Colorize image with pen color, using specified percent alpha.
    void colorize(const unsigned int alpha_,const Color &penColor_);

    // Colorize image with pen color, using specified percent alpha
    // for red, green, and blue quantums
    void colorize(const unsigned int alphaRed_,const unsigned int alphaGreen_,
       const unsigned int alphaBlue_,const Color &penColor_);

     // Color at colormap position index_
    void colorMap(const size_t index_,const Color &color_);
    Color colorMap(const size_t index_) const;

    // Apply a color matrix to the image channels. The user supplied
    // matrix may be of order 1 to 5 (1x1 through 5x5).
    void colorMatrix(const size_t order_,const double *color_matrix_);

    // Compare current image with another image
    // False is returned if the images are not identical.
    bool compare(const Image &reference_) const;

    // Compare current image with another image
    // Returns the distortion based on the specified metric.
    double compare(const Image &reference_,const MetricType metric_);
    double compareChannel(const ChannelType channel_,
                                     const Image &reference_,
                                     const MetricType metric_ );

    // Compare current image with another image
    // Sets the distortion and returns the difference image.
    Image compare(const Image &reference_,const MetricType metric_,
      double *distortion);
    Image compareChannel(const ChannelType channel_,const Image &reference_,
      const MetricType metric_,double *distortion);

    // Compose an image onto another at specified offset and using
    // specified algorithm
    void composite(const Image &compositeImage_,const Geometry &offset_,
      const CompositeOperator compose_=InCompositeOp);
    void composite(const Image &compositeImage_,const GravityType gravity_,
      const CompositeOperator compose_=InCompositeOp);
    void composite(const Image &compositeImage_,const ::ssize_t xOffset_,
      const ::ssize_t yOffset_,const CompositeOperator compose_=InCompositeOp);

    // Determines the connected-components of the image
    void connectedComponents(const size_t connectivity_);

    // Contrast image (enhance intensity differences in image)
    void contrast(const bool sharpen_);

    // A simple image enhancement technique that attempts to improve the
    // contrast in an image by 'stretching' the range of intensity values
    // it contains to span a desired range of values. It differs from the
    // more sophisticated histogram equalization in that it can only apply a
    // linear scaling function to the image pixel values. As a result the
    // 'enhancement' is less harsh.
    void contrastStretch(const double blackPoint_,const double whitePoint_);
    void contrastStretchChannel(const ChannelType channel_,
      const double blackPoint_,const double whitePoint_);

    // Convolve image.  Applies a user-specified convolution to the image.
    //  order_ represents the number of columns and rows in the filter kernel.
    //  kernel_ is an array of doubles representing the convolution kernel.
    void convolve(const size_t order_,const double *kernel_);

    // Copies pixels from the source image as defined by the geometry the
    // destination image at the specified offset.
    void copyPixels(const Image &source_,const Geometry &geometry_,
      const Offset &offset_);

    // Crop image (subregion of original image)
    void crop(const Geometry &geometry_);

    // Cycle image colormap
    void cycleColormap(const ::ssize_t amount_);

    // Converts cipher pixels to plain pixels.
    void decipher(const std::string &passphrase_);

    // Tagged image format define. Similar to the defineValue() method
    // except that passing the flag_ value 'true' creates a value-less
    // define with that format and key. Passing the flag_ value 'false'
    // removes any existing matching definition. The method returns 'true'
    // if a matching key exists, and 'false' if no matching key exists.
    void defineSet(const std::string &magick_,const std::string &key_,
      bool flag_);
    bool defineSet(const std::string &magick_,const std::string &key_) const;

    // Tagged image format define (set/access coder-specific option) The
    // magick_ option specifies the coder the define applies to.  The key_
    // option provides the key specific to that coder.  The value_ option
    // provides the value to set (if any). See the defineSet() method if the
    // key must be removed entirely.
    void defineValue(const std::string &magick_,const std::string &key_,
      const std::string &value_);
    std::string defineValue(const std::string &magick_,
      const std::string &key_) const;

    // Removes skew from the image. Skew is an artifact that occurs in scanned
    // images because of the camera being misaligned, imperfections in the
    // scanning or surface, or simply because the paper was not placed
    // completely flat when scanned. The value of threshold_ ranges from 0
    // to QuantumRange.
    void deskew(const double threshold_);

    // Despeckle image (reduce speckle noise)
    void despeckle(void);

    // Display image on screen
    void display(void);

    // Distort image.  distorts an image using various distortion methods, by
    // mapping color lookups of the source image to a new destination image
    // usally of the same size as the source image, unless 'bestfit' is set to
    // true.
    void distort(const DistortMethod method_,
      const size_t numberArguments_,const double *arguments_,
      const bool bestfit_=false);

    // Draw on image using a single drawable
    void draw(const Drawable &drawable_);

    // Draw on image using a drawable list
    void draw(const std::vector<Magick::Drawable> &drawable_);

    // Edge image (hilight edges in image)
    void edge(const double radius_=0.0);

    // Emboss image (hilight edges with 3D effect)
    // The radius_ parameter specifies the radius of the Gaussian, in
    // pixels, not counting the center pixel.  The sigma_ parameter
    // specifies the standard deviation of the Laplacian, in pixels.
    void emboss(const double radius_=0.0,const double sigma_=1.0);

    // Converts pixels to cipher-pixels.
    void encipher(const std::string &passphrase_);

    // Enhance image (minimize noise)
    void enhance(void);

    // Equalize image (histogram equalization)
    void equalize(void);

    // Erase image to current "background color"
    void erase(void);

    // Apply a value with an arithmetic, relational, or logical operator.
    void evaluate(const ChannelType channel_,
      const MagickEvaluateOperator operator_,double rvalue_);

    // Apply a value with an arithmetic, relational, or logical operator.
    void evaluate(const ChannelType channel_,const MagickFunction function_,
      const size_t number_parameters_,const double *parameters_);

    // Apply a value with an arithmetic, relational, or logical operator.
    void evaluate(const ChannelType channel_,const ::ssize_t x_,
      const ::ssize_t y_,const size_t columns_,const size_t rows_,
      const MagickEvaluateOperator operator_,const double rvalue_);

    // Extend the image as defined by the geometry.
    void extent(const Geometry &geometry_);
    void extent(const Geometry &geometry_,const Color &backgroundColor);
    void extent(const Geometry &geometry_,const Color &backgroundColor,
      const GravityType gravity_);
    void extent(const Geometry &geometry_,const GravityType gravity_);

    // Flip image (reflect each scanline in the vertical direction)
    void flip(void);

    // Floodfill pixels matching color (within fuzz factor) of target
    // pixel(x,y) with replacement alpha value.
    void floodFillAlpha(const ::ssize_t x_,const ::ssize_t y_,
      const unsigned int alpha_,const bool invert_=false);

    // Floodfill designated area with replacement alpha value
    void floodFillAlpha(const ssize_t x_,const ssize_t y_,
      const unsigned int alpha_,const Color &target_,const bool invert_=false);

    // Flood-fill color across pixels that match the color of the
    // target pixel and are neighbors of the target pixel.
    // Uses current fuzz setting when determining color match.
    void floodFillColor(const Geometry &point_,const Color &fillColor_,
      const bool invert_=false);
    void floodFillColor(const ::ssize_t x_,const ::ssize_t y_,
      const Color &fillColor_,const bool invert_=false);

    // Flood-fill color across pixels starting at target-pixel and
    // stopping at pixels matching specified border color.
    // Uses current fuzz setting when determining color match.
    void floodFillColor(const Geometry &point_,const Color &fillColor_,
      const Color &borderColor_,const bool invert_=false);
    void floodFillColor(const ::ssize_t x_,const ::ssize_t y_,
      const Color &fillColor_,const Color &borderColor_,
      const bool invert_=false);

    // Flood-fill texture across pixels that match the color of the
    // target pixel and are neighbors of the target pixel.
    // Uses current fuzz setting when determining color match.
    void floodFillTexture(const Geometry &point_,const Image &texture_,
      const bool invert_=false);
    void floodFillTexture(const ::ssize_t x_,const ::ssize_t y_,
      const Image &texture_,const bool invert_=false);

    // Flood-fill texture across pixels starting at target-pixel and
    // stopping at pixels matching specified border color.
    // Uses current fuzz setting when determining color match.
    void floodFillTexture(const Geometry &point_,const Image &texture_,
      const Color &borderColor_,const bool invert_=false);
    void floodFillTexture(const ::ssize_t x_,const ::ssize_t y_,
      const Image &texture_,const Color &borderColor_,
      const bool invert_=false);

    // Flop image (reflect each scanline in the horizontal direction)
    void flop(void);

    // Obtain font metrics for text string given current font,
    // pointsize, and density settings.
    void fontTypeMetrics(const std::string &text_,TypeMetric *metrics);

    // Obtain multi line font metrics for text string given current font,
    // pointsize, and density settings.
    void fontTypeMetricsMultiline(const std::string &text_,
      TypeMetric *metrics);

    // Frame image
    void frame(const Geometry &geometry_=frameGeometryDefault);
    void frame(const size_t width_,const size_t height_,
      const ::ssize_t innerBevel_=6,const ::ssize_t outerBevel_=6);

    // Applies a mathematical expression to the image.
    void fx(const std::string expression_);
    void fx(const std::string expression_,const Magick::ChannelType channel_);

    // Gamma correct image
    void gamma(const double gamma_);
    void gamma(const double gammaRed_,const double gammaGreen_,
      const double gammaBlue_);

    // Gaussian blur image
    // The number of neighbor pixels to be included in the convolution
    // mask is specified by 'radius_'. The standard deviation of the
    // gaussian bell curve is specified by 'sigma_'.
    void gaussianBlur(const double radius_,const double sigma_);
    void gaussianBlurChannel(const ChannelType channel_,const double radius_,
      const double sigma_);

    // Transfers read-only pixels from the image to the pixel cache as
    // defined by the specified region
    const Quantum *getConstPixels(const ::ssize_t x_, const ::ssize_t y_,
      const size_t columns_,const size_t rows_) const;

    // Obtain immutable image pixel metacontent (valid for PseudoClass images)
    const void *getConstMetacontent(void) const;

    // Obtain mutable image pixel metacontent (valid for PseudoClass images)
    void *getMetacontent(void);

    // Transfers pixels from the image to the pixel cache as defined
    // by the specified region. Modified pixels may be subsequently
    // transferred back to the image via syncPixels.  This method is
    // valid for DirectClass images.
    Quantum *getPixels(const ::ssize_t x_,const ::ssize_t y_,
      const size_t columns_,const size_t rows_);

    // Converts the colors in the image to gray.
    void grayscale(const PixelIntensityMethod method_);

    // Apply a color lookup table (Hald CLUT) to the image.
    void haldClut(const Image &clutImage_);

    // Identifies lines in the image.
    void houghLine(const size_t width_,const size_t height_,
      const size_t threshold_=40);

    // Identifies the potential color type of the image. This method can be
    // used to detect if the type can be changed to GrayScale.
    ImageType identifyType(void) const;

    // Implode image (special effect)
    void implode(const double factor_);

    // Implements the inverse discrete Fourier transform (DFT) of the image
    // either as a magnitude / phase or real / imaginary image pair.
    void inverseFourierTransform(const Image &phase_);
    void inverseFourierTransform(const Image &phase_,const bool magnitude_);

    // An edge preserving noise reduction filter.
    void kuwahara(const double radius_=0.0,const double sigma_=1.0);
    void kuwaharaChannel(const ChannelType channel_,const double radius_=0.0,
      const double sigma_=1.0);

    // Level image. Adjust the levels of the image by scaling the
    // colors falling between specified white and black points to the
    // full available quantum range. The parameters provided represent
    // the black, mid (gamma), and white points.  The black point
    // specifies the darkest color in the image. Colors darker than
    // the black point are set to zero. Mid point (gamma) specifies a
    // gamma correction to apply to the image. White point specifies
    // the lightest color in the image.  Colors brighter than the
    // white point are set to the maximum quantum value. The black and
    // white point have the valid range 0 to QuantumRange while mid (gamma)
    // has a useful range of 0 to ten.
    void level(const double blackPoint_,const double whitePoint_,
      const double gamma_=1.0);
    void levelChannel(const ChannelType channel_,const double blackPoint_,
      const double whitePoint_,const double gamma_=1.0);

    // Maps the given color to "black" and "white" values, linearly spreading
    // out the colors, and level values on a channel by channel bases, as
    // per level(). The given colors allows you to specify different level
    // ranges for each of the color channels separately.
    void levelColors(const Color &blackColor_,const Color &whiteColor_,
      const bool invert_=true);
    void levelColorsChannel(const ChannelType channel_,
      const Color &blackColor_,const Color &whiteColor_,
      const bool invert_=true);

    // Levelize applies the reversed level operation to just the specific
    // channels specified.It compresses the full range of color values, so
    // that they lie between the given black and white points. Gamma is
    // applied before the values are mapped.
    void levelize(const double blackPoint_,const double whitePoint_,
      const double gamma_=1.0);
    void levelizeChannel(const ChannelType channel_,const double blackPoint_,
      const double whitePoint_,const double gamma_=1.0);

    // Discards any pixels below the black point and above the white point and
    // levels the remaining pixels.
    void linearStretch(const double blackPoint_,const double whitePoint_);

    // Rescales image with seam carving.
    void liquidRescale(const Geometry &geometry_);

    // Local contrast enhancement
    void localContrast(const double radius_,const double strength_);
    void localContrastChannel(const ChannelType channel_,const double radius_,
      const double strength_);

    // Magnify image by integral size
    void magnify(void);

    // Remap image colors with closest color from reference image
    void map(const Image &mapImage_,const bool dither_=false);

    // Delineate arbitrarily shaped clusters in the image.
    void meanShift(const size_t width_,const size_t height_,
      const double color_distance_);

    // Filter image by replacing each pixel component with the median
    // color in a circular neighborhood
    void medianFilter(const double radius_=0.0);

    // Reduce image by integral size
    void minify(void);

    // Modulate percent hue, saturation, and brightness of an image
    void modulate(const double brightness_,const double saturation_,
      const double hue_);

    // Returns the normalized moments of one or more image channels.
    ImageMoments moments(void) const;

    // Applies a kernel to the image according to the given mophology method.
    void morphology(const MorphologyMethod method_,const std::string kernel_,
      const ssize_t iterations_=1);
    void morphology(const MorphologyMethod method_,
      const KernelInfoType kernel_,const std::string arguments_,
      const ssize_t iterations_=1);
    void morphologyChannel(const ChannelType channel_,
      const MorphologyMethod method_,const std::string kernel_,
      const ssize_t iterations_=1);
    void morphologyChannel(const ChannelType channel_,
      const MorphologyMethod method_,const KernelInfoType kernel_,
      const std::string arguments_,const ssize_t iterations_=1);

    // Motion blur image with specified blur factor
    // The radius_ parameter specifies the radius of the Gaussian, in
    // pixels, not counting the center pixel.  The sigma_ parameter
    // specifies the standard deviation of the Laplacian, in pixels.
    // The angle_ parameter specifies the angle the object appears
    // to be comming from (zero degrees is from the right).
    void motionBlur(const double radius_,const double sigma_,
      const double angle_);

    // Negate colors in image.  Set grayscale to only negate grayscale
    // values in image.
    void negate(const bool grayscale_=false);
    void negateChannel(const ChannelType channel_,const bool grayscale_=false);

    // Normalize image (increase contrast by normalizing the pixel
    // values to span the full range of color values)
    void normalize(void);

    // Oilpaint image (image looks like oil painting)
    void oilPaint(const double radius_=0.0,const double sigma=1.0);

    // Change color of opaque pixel to specified pen color.
    void opaque(const Color &opaqueColor_,const Color &penColor_,
      const bool invert_=false);

    // Perform a ordered dither based on a number of pre-defined dithering
    // threshold maps, but over multiple intensity levels.
    void orderedDither(std::string thresholdMap_);
    void orderedDitherChannel(const ChannelType channel_,
      std::string thresholdMap_);

    // Set each pixel whose value is less than epsilon to epsilon or
    // -epsilon (whichever is closer) otherwise the pixel value remains
    // unchanged.
    void perceptible(const double epsilon_);
    void perceptibleChannel(const ChannelType channel_,const double epsilon_);

    // Returns the perceptual hash for this image.
    Magick::ImagePerceptualHash perceptualHash() const;

    // Ping is similar to read except only enough of the image is read
    // to determine the image columns, rows, and filesize.  Access the
    // columns(), rows(), and fileSize() attributes after invoking
    // ping.  The image data is not valid after calling ping.
    void ping(const std::string &imageSpec_);

    // Ping is similar to read except only enough of the image is read
    // to determine the image columns, rows, and filesize.  Access the
    // columns(), rows(), and fileSize() attributes after invoking
    // ping.  The image data is not valid after calling ping.
    void ping(const Blob &blob_);

    // Get/set pixel color at location x & y.
    void pixelColor(const ::ssize_t x_,const ::ssize_t y_,const Color &color_);
    Color pixelColor(const ::ssize_t x_,const ::ssize_t y_ ) const;

    // Simulates a Polaroid picture.
    void polaroid(const std::string &caption_,const double angle_,
      const PixelInterpolateMethod method_);

    // Reduces the image to a limited number of colors for a "poster" effect.
    void posterize(const size_t levels_,const DitherMethod method_);
    void posterizeChannel(const ChannelType channel_,const size_t levels_,
      const DitherMethod method_);

    // Execute a named process module using an argc/argv syntax similar to
    // that accepted by a C 'main' routine. An exception is thrown if the
    // requested process module doesn't exist, fails to load, or fails during
    // execution.
    void process(std::string name_,const ::ssize_t argc_,const char **argv_);

    // Add or remove a named profile to/from the image. Remove the
    // profile by passing an empty Blob (e.g. Blob()). Valid names are
    // "*", "8BIM", "ICM", "IPTC", or a user/format-defined profile name.
    void profile(const std::string name_,const Blob &colorProfile_);

    // Retrieve a named profile from the image. Valid names are:
    // "8BIM", "8BIMTEXT", "APP1", "APP1JPEG", "ICC", "ICM", & "IPTC"
    // or an existing user/format-defined profile name.
    Blob profile(const std::string name_) const;

    // Quantize image (reduce number of colors)
    void quantize(const bool measureError_=false);

    // Raise image (lighten or darken the edges of an image to give a
    // 3-D raised or lowered effect)
    void raise(const Geometry &geometry_=raiseGeometryDefault,
      const bool raisedFlag_=false);
    
    // Random threshold image.
    //
    // Changes the value of individual pixels based on the intensity
    // of each pixel compared to a random threshold.  The result is a
    // low-contrast, two color image.
    void randomThreshold(const double low_,const double high_);
    void randomThresholdChannel(const ChannelType channel_,const double low_,
      const double high_);

    // Read single image frame from in-memory BLOB
    void read(const Blob &blob_);

    // Read single image frame of specified size from in-memory BLOB
    void read(const Blob &blob_,const Geometry &size_);

    // Read single image frame of specified size and depth from
    // in-memory BLOB
    void read(const Blob &blob_,const Geometry &size_,const size_t depth_);

    // Read single image frame of specified size, depth, and format
    // from in-memory BLOB
    void read(const Blob &blob_,const Geometry &size_,const size_t depth_,
      const std::string &magick_);

    // Read single image frame of specified size, and format from
    // in-memory BLOB
    void read(const Blob &blob_,const Geometry &size_,
      const std::string &magick_);

    // Read single image frame of specified size into current object
    void read(const Geometry &size_,const std::string &imageSpec_);

    // Read single image frame from an array of raw pixels, with
    // specified storage type (ConstituteImage), e.g.
    // image.read( 640, 480, "RGB", 0, pixels );
    void read(const size_t width_,const size_t height_,const std::string &map_,
      const StorageType type_,const void *pixels_);
    
    // Read single image frame into current object
    void read(const std::string &imageSpec_);

    // Associate a mask with the image. The mask must be the same dimensions
    // as the image. Pass an invalid image to unset an existing mask.
    void readMask(const Image &mask_);
    Image readMask(void) const;

    // Transfers one or more pixel components from a buffer or file
    // into the image pixel cache of an image.
    // Used to support image decoders.
    void readPixels(const QuantumType quantum_,const unsigned char *source_);

    // Reduce noise in image using a noise peak elimination filter
    void reduceNoise(void);
    void reduceNoise(const size_t order_);

    // Resets the image page canvas and position.
    void repage();

    // Resize image in terms of its pixel size.
    void resample(const Point &density_);

    // Resize image to specified size.
    void resize(const Geometry &geometry_);

    // Roll image (rolls image vertically and horizontally) by specified
    // number of columnms and rows)
    void roll(const Geometry &roll_);
    void roll(const size_t columns_,const size_t rows_);

    // Rotate image clockwise by specified number of degrees. Specify a
    // negative number for degrees to rotate counter-clockwise.
    void rotate(const double degrees_);

    // Rotational blur image.
    void rotationalBlur(const double angle_);
    void rotationalBlurChannel(const ChannelType channel_,const double angle_);

    // Resize image by using pixel sampling algorithm
    void sample(const Geometry &geometry_);

    // Resize image by using simple ratio algorithm
    void scale(const Geometry &geometry_);

    // Segment (coalesce similar image components) by analyzing the
    // histograms of the color components and identifying units that
    // are homogeneous with the fuzzy c-means technique.  Also uses
    // QuantizeColorSpace and Verbose image attributes
    void segment(const double clusterThreshold_=1.0,
      const double smoothingThreshold_=1.5);

    // Selectively blur pixels within a contrast threshold. It is similar to
    // the unsharpen mask that sharpens everything with contrast above a
    // certain threshold.
    void selectiveBlur(const double radius_,const double sigma_,
      const double threshold_);
    void selectiveBlurChannel(const ChannelType channel_,const double radius_,
      const double sigma_,const double threshold_);

    // Separates a channel from the image and returns it as a grayscale image.
    Image separate(const ChannelType channel_) const;

    // Applies a special effect to the image, similar to the effect achieved in
    // a photo darkroom by sepia toning.  Threshold ranges from 0 to 
    // QuantumRange and is a measure of the extent of the sepia toning.
    // A threshold of 80% is a good starting point for a reasonable tone.
    void sepiaTone(const double threshold_);

    // Sets meanErrorPerPixel, normalizedMaxError, and normalizedMeanError
    // in the current image. False is returned if the images are not identical.
    bool setColorMetric(const Image &reference_);

    // Allocates a pixel cache region to store image pixels as defined
    // by the region rectangle.  This area is subsequently transferred
    // from the pixel cache to the image via syncPixels.
    Quantum *setPixels(const ::ssize_t x_, const ::ssize_t y_,
      const size_t columns_,const size_t rows_);

    // Shade image using distant light source
    void shade(const double azimuth_=30,const double elevation_=30,
      const bool colorShading_=false);

    // Simulate an image shadow
    void shadow(const double percentAlpha_=80.0,const double sigma_=0.5,
      const ssize_t x_=5,const ssize_t y_=5);

    // Sharpen pixels in image
    // The radius_ parameter specifies the radius of the Gaussian, in
    // pixels, not counting the center pixel.  The sigma_ parameter
    // specifies the standard deviation of the Laplacian, in pixels.
    void sharpen(const double radius_=0.0,const double sigma_=1.0);
    void sharpenChannel(const ChannelType channel_,const double radius_=0.0,
      const double sigma_=1.0);

    // Shave pixels from image edges.
    void shave(const Geometry &geometry_);

    // Shear image (create parallelogram by sliding image by X or Y axis)
    void shear(const double xShearAngle_,const double yShearAngle_);

    // adjust the image contrast with a non-linear sigmoidal contrast algorithm
    void sigmoidalContrast(const bool sharpen_,const double contrast,
      const double midpoint=QuantumRange/2.0);

    // Image signature. Set force_ to true in order to re-calculate
    // the signature regardless of whether the image data has been
    // modified.
    std::string signature(const bool force_=false) const;

    // Simulates a pencil sketch. We convolve the image with a Gaussian
    // operator of the given radius and standard deviation (sigma). For
    // reasonable results, radius should be larger than sigma. Use a
    // radius of 0 and SketchImage() selects a suitable radius for you.
    void sketch(const double radius_=0.0,const double sigma_=1.0,
      const double angle_=0.0);

    // Solarize image (similar to effect seen when exposing a
    // photographic film to light during the development process)
    void solarize(const double factor_=50.0);

    // Sparse color image, given a set of coordinates, interpolates the colors
    // found at those coordinates, across the whole image, using various
    // methods.
    void sparseColor(const ChannelType channel_,
      const SparseColorMethod method_,const size_t numberArguments_,
      const double *arguments_);

    // Splice the background color into the image.
    void splice(const Geometry &geometry_);
    void splice(const Geometry &geometry_,const Color &backgroundColor_);
    void splice(const Geometry &geometry_,const Color &backgroundColor_,
      const GravityType gravity_);

    // Spread pixels randomly within image by specified ammount
    void spread(const double amount_=3.0);

    // Returns the statistics for this image.
    Magick::ImageStatistics statistics() const;

    // Add a digital watermark to the image (based on second image)
    void stegano(const Image &watermark_);

    // Create an image which appears in stereo when viewed with
    // red-blue glasses (Red image on left, blue on right)
    void stereo(const Image &rightImage_);

    // Strip strips an image of all profiles and comments.
    void strip(void);

    // Search for the specified image at EVERY possible location in this image.
    // This is slow! very very slow.. It returns a similarity image such that
    // an exact match location is completely white and if none of the pixels
    // match, black, otherwise some gray level in-between.
    Image subImageSearch(const Image &reference_,const MetricType metric_,
      Geometry *offset_,double *similarityMetric_,
      const double similarityThreshold=(-1.0));

    // Swirl image (image pixels are rotated by degrees)
    void swirl(const double degrees_);

    // Transfers the image cache pixels to the image.
    void syncPixels(void);

    // Channel a texture on image background
    void texture(const Image &texture_);

    // Threshold image
    void threshold(const double threshold_);

    // Resize image to thumbnail size
    void thumbnail(const Geometry &geometry_);

    // Applies a color vector to each pixel in the image. The length of the
    // vector is 0 for black and white and at its maximum for the midtones.
    // The vector weighting function is f(x)=(1-(4.0*((x-0.5)*(x-0.5))))
    void tint(const std::string opacity_);

    // Origin of coordinate system to use when annotating with text or drawing
    void transformOrigin(const double x_,const double y_);

    // Reset transformation parameters to default
    void transformReset(void);

    // Scale to use when annotating with text or drawing
    void transformScale(const double sx_,const double sy_);

    // Add matte image to image, setting pixels matching color to
    // transparent
    void transparent(const Color &color_,const bool inverse_=false);

    // Add matte image to image, for all the pixels that lies in between
    // the given two color
    void transparentChroma(const Color &colorLow_,const Color &colorHigh_);

    // Creates a horizontal mirror image by reflecting the pixels around the
    // central y-axis while rotating them by 90 degrees.
    void transpose(void);

    // Creates a vertical mirror image by reflecting the pixels around the
    // central x-axis while rotating them by 270 degrees.
    void transverse(void);

    // Trim edges that are the background color from the image
    void trim(void);

    // Returns the unique colors of an image.
    Image uniqueColors(void) const;

    // Replace image with a sharpened version of the original image
    // using the unsharp mask algorithm.
    //  radius_
    //    the radius of the Gaussian, in pixels, not counting the
    //    center pixel.
    //  sigma_
    //    the standard deviation of the Gaussian, in pixels.
    //  amount_
    //    the percentage of the difference between the original and
    //    the blur image that is added back into the original.
    // threshold_
    //   the threshold in pixels needed to apply the diffence amount.
    void unsharpmask(const double radius_,const double sigma_,
      const double amount_,const double threshold_);
    void unsharpmaskChannel(const ChannelType channel_,const double radius_,
      const double sigma_,const double amount_,const double threshold_);

    // Softens the edges of the image in vignette style.
    void vignette(const double radius_=0.0,const double sigma_=1.0,
      const ssize_t x_=0,const ssize_t y_=0);

    // Map image pixels to a sine wave
    void wave(const double amplitude_=25.0,const double wavelength_=150.0);

    // Removes noise from the image using a wavelet transform.
    void waveletDenoise(const double threshold_,const double softness_);

    // Forces all pixels above the threshold into white while leaving all
    // pixels at or below the threshold unchanged.
    void whiteThreshold(const std::string &threshold_);
    void whiteThresholdChannel(const ChannelType channel_,
      const std::string &threshold_);

    // Write single image frame to in-memory BLOB, with optional
    // format and adjoin parameters.
    void write(Blob *blob_);
    void write(Blob *blob_,const std::string &magick_);
    void write(Blob *blob_,const std::string &magick_,const size_t depth_);

    // Write single image frame to an array of pixels with storage
    // type specified by user (DispatchImage), e.g.
    // image.write( 0, 0, 640, 1, "RGB", 0, pixels );
    void write(const ::ssize_t x_,const ::ssize_t y_,const size_t columns_,
      const size_t rows_,const std::string &map_,const StorageType type_,
      void *pixels_);

    // Write single image frame to a file
    void write(const std::string &imageSpec_);

    // Associate a mask with the image. The mask must be the same dimensions
    // as the image. Pass an invalid image to unset an existing mask.
    void writeMask(const Image &mask_);
    Image writeMask(void) const;

    // Transfers one or more pixel components from the image pixel
    // cache to a buffer or file.
    // Used to support image encoders.
    void writePixels(const QuantumType quantum_,unsigned char *destination_);

    // Zoom image to specified size.
    void zoom(const Geometry &geometry_);

    //////////////////////////////////////////////////////////////////////
    //
    // No user-serviceable parts beyond this point
    //
    //////////////////////////////////////////////////////////////////////

    // Construct with MagickCore::Image and default options
    Image(MagickCore::Image *image_);

    // Retrieve Image*
    MagickCore::Image *&image(void);
    const MagickCore::Image *constImage(void) const;

    // Retrieve ImageInfo*
    MagickCore::ImageInfo *imageInfo(void);
    const MagickCore::ImageInfo *constImageInfo(void) const;

    // Retrieve Options*
    Options *options(void);
    const Options *constOptions(void) const;

    // Retrieve QuantizeInfo*
    MagickCore::QuantizeInfo *quantizeInfo(void);
    const MagickCore::QuantizeInfo *constQuantizeInfo(void) const;

    // Prepare to update image (copy if reference > 1)
    void modifyImage(void);

    // Replace current image (reference counted)
    MagickCore::Image *replaceImage(MagickCore::Image *replacement_);

  private:

    void floodFill(const ssize_t x_,const ssize_t y_,
      const Magick::Image *fillPattern_,const Color &fill_,
      const PixelInfo *target,const bool invert_);

    void mask(const Image &mask_,const PixelMask);
    Image mask(const PixelMask) const;

    void read(MagickCore::Image *image,
      MagickCore::ExceptionInfo *exceptionInfo);

    ImageRef *_imgRef;
  };

} // end of namespace Magick

#endif // Magick_Image_header

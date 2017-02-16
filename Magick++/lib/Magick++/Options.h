// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
// Copyright Dirk Lemstra 2014-2016
//
// Definition of Options
//
// Options which may be applied to an image. These options are the
// equivalent of options supplied to ImageMagick utilities.
//
// This is an internal implementation class and is not part of the
// Magick++ API
//

#if !defined(Magick_Options_header)
#define Magick_Options_header

#include "Magick++/Include.h"
#include <string>
#include "Magick++/Color.h"
#include "Magick++/Geometry.h"
#include "Magick++/Drawable.h"

namespace Magick
{
  class Image;

  class Options
  {
  public:

    // Default constructor
    Options(void);

    // Copy constructor
    Options(const Options& options_);

    // Destructor
    ~Options();

    // Join images into a single multi-image file
    void adjoin(const bool flag_);
    bool adjoin(void) const;

    // Transparent color
    void matteColor(const Color &matteColor_);
    Color matteColor(void) const;

    // Image background color
    void backgroundColor(const Color &color_);
    Color backgroundColor(void) const;

    // Name of texture image to tile onto the image background
    void backgroundTexture(const std::string &backgroundTexture_);
    std::string backgroundTexture(void) const;

    // Image border color
    void borderColor(const Color &color_);
    Color borderColor(void) const;

    // Text bounding-box base color (default none)
    void boxColor(const Color &boxColor_);
    Color boxColor(void) const;

    // Colors within this distance are considered equal
    void colorFuzz(const double fuzz_);
    double colorFuzz(void) const;

    // Image colorspace scheme
    void colorspaceType(const ColorspaceType colorspace_);
    ColorspaceType colorspaceType(void) const;

    // Compression type ( NoCompression, BZipCompression,
    // FaxCompression, JPEGCompression, LZWCompression,
    // RLECompression, or ZipCompression )
    void compressType(const CompressionType compressType_);
    CompressionType compressType(void) const;

    // Enable printing of debug messages from ImageMagick
    void debug(const bool flag_);
    bool debug(void) const;

    // Vertical and horizontal resolution in pixels of the image
    void density(const Point &density_);
    Point density(void) const;

    // Image depth (8 or 16)
    void depth(const size_t depth_);
    size_t depth(void) const;

    // Endianness (little like Intel or big like SPARC) for image
    // formats which support endian-specific options.
    void endian(const EndianType endian_);
    EndianType endian(void) const;

    // Image filename to read or write
    void file(FILE *file_);
    FILE *file(void) const;

    // Image filename to read or write
    void fileName(const std::string &fileName_);
    std::string fileName(void) const;

    // Color to use when filling drawn objects
    void fillColor(const Color &fillColor_);
    Color fillColor(void) const;

    // Fill pattern
    void fillPattern(const MagickCore::Image *fillPattern_);
    const MagickCore::Image *fillPattern(void) const;

    // Rule to use when filling drawn objects
    void fillRule(const FillRule &fillRule_);
    FillRule fillRule(void) const;

    // Font name
    void font(const std::string &font_);
    std::string font(void) const;

    // Font name
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

    std::string format(void) const;

    // Image interlace scheme
    void interlaceType(const InterlaceType interlace_);
    InterlaceType interlaceType(void) const;

   // Image format to write or read
    void magick(const std::string &magick_);
    std::string magick(void) const;

   // Write as a monochrome image
    void monochrome(const bool monochromeFlag_);
    bool monochrome(void) const;

    // Preferred size and location of an image canvas.
    void page(const Geometry &pageSize_);
    Geometry page(void) const;

    // Desired image quality factor
    void quality(const size_t quality_);
    size_t quality(void) const;

    // Maximum number of colors to quantize to
    void quantizeColors(const size_t colors_);
    size_t quantizeColors(void) const;

    // Colorspace to quantize in.
    void quantizeColorSpace(const ColorspaceType colorSpace_);
    ColorspaceType quantizeColorSpace(void) const;

    // Dither image during quantization.
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

    // Units of resolution to interpret density
    void resolutionUnits(const ResolutionType resolutionUnits_);
    ResolutionType resolutionUnits(void) const;

    // Image sampling factor
    void samplingFactor(const std::string &samplingFactor_);
    std::string samplingFactor(void) const;

    // Image size (required for raw formats)
    void size(const Geometry &geometry_);
    Geometry size(void) const;

    // enabled/disable stroke anti-aliasing
    void strokeAntiAlias(const bool flag_);
    bool strokeAntiAlias(void) const ;

    // Color to use when drawing object outlines
    void strokeColor(const Color &strokeColor_);
    Color strokeColor(void) const;

    // Control the pattern of dashes and gaps used to stroke
    // paths. The strokeDashArray represents a list of numbers that
    // specify the lengths of alternating dashes and gaps in user
    // units. If an odd number of values is provided, then the list of
    // values is repeated to yield an even number of values.
    void strokeDashArray(const double *strokeDashArray_);
    const double *strokeDashArray(void) const;

    // While drawing using strokeDashArray, specify distance into the dash
    // pattern to start the dash (default 0).
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
    // the ratio of the miter length to the 'stroke_width'. The default
    // value of this parameter is 4.
    void strokeMiterLimit(const size_t miterLimit_);
    size_t strokeMiterLimit(void) const;

    // Pattern image to use for stroked outlines
    void strokePattern(const MagickCore::Image *strokePattern_);
    const MagickCore::Image *strokePattern(void) const;

   // Stroke width for drawing vector objects (default one)
    void strokeWidth(const double strokeWidth_);
    double strokeWidth(void) const;

    void subImage(const size_t subImage_);
    size_t subImage(void) const;

    // Sub-frame number to return
    void subRange(const size_t subRange_);
    size_t subRange(void) const;

    // Remove pixel aliasing
    void textAntiAlias(const bool flag_);
    bool textAntiAlias(void) const;

    // Render text right-to-left or left-to-right.
    void textDirection(const DirectionType direction_);
    DirectionType textDirection() const;

    // Annotation text encoding (e.g. "UTF-16")
    void textEncoding(const std::string &encoding_);
    std::string textEncoding(void) const;

    // Text gravity.
    void textGravity(const GravityType gravity_);
    GravityType textGravity() const;

    // Text inter-line spacing
    void textInterlineSpacing(const double spacing_);
    double textInterlineSpacing(void) const;

    // Text inter-word spacing
    void textInterwordSpacing(const double spacing_);
    double textInterwordSpacing(void) const;

    // Text inter-character kerning
    void textKerning(const double kerning_);
    double textKerning(void) const;

    // Text undercolor box
    void textUnderColor(const Color &underColor_);
    Color textUnderColor(void) const;

    // Origin of coordinate system to use when annotating with text or drawing
    void transformOrigin(const double tx_,const double ty_);

    // Reset transformation parameters to default
    void transformReset(void);

    // Rotation to use when annotating with text or drawing
    void transformRotation(const double angle_);

    // Scale to use when annotating with text or drawing
    void transformScale(const double sx_,const double sy_);

    // Skew to use in X axis when annotating with text or drawing
    void transformSkewX(const double skewx_);

    // Skew to use in Y axis when annotating with text or drawing
    void transformSkewY(const double skewy_);

    // Image representation type
    void type(const ImageType type_);
    ImageType type(void) const;

    // Return verbose information about an image, or an operation
    void verbose(const bool verboseFlag_);
    bool verbose(void) const;

    // X11 display name
    void x11Display(const std::string &display_);
    std::string x11Display(void) const;

    //
    // Internal implementation methods.  Please do not use.
    //

    MagickCore::DrawInfo *drawInfo(void);
    MagickCore::ImageInfo *imageInfo(void);
    MagickCore::QuantizeInfo *quantizeInfo(void);

    // Construct using raw structures
    Options(const MagickCore::ImageInfo *imageInfo_,
      const MagickCore::QuantizeInfo *quantizeInfo_,
      const MagickCore::DrawInfo *drawInfo_);

  private:

    // Assignment not supported
    Options& operator=(const Options&);

    void setOption(const char *name,const Color &value_);

    void setOption(const char *name,const double value_);

    MagickCore::ImageInfo    *_imageInfo;
    MagickCore::QuantizeInfo *_quantizeInfo;
    MagickCore::DrawInfo     *_drawInfo;
    bool                     _quiet;
  };
} // namespace Magick

#endif // Magick_Options_header

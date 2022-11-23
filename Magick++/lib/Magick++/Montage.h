// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Copyright @ 2014 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Definition of Montage class used to specify montage options.
//

#if !defined(Magick_Montage_header)
#define Magick_Montage_header

#include "Magick++/Include.h"
#include <string>
#include "Magick++/Color.h"
#include "Magick++/Geometry.h"

//
// Basic (Un-framed) Montage
//
namespace Magick
{
  class MagickPPExport Montage
  {
  public:

    Montage(void);
    virtual ~Montage(void);

    // Color that thumbnails are composed on
    void backgroundColor(const Color &backgroundColor_);
    Color backgroundColor(void) const;

    // Composition algorithm to use (e.g. ReplaceCompositeOp)
    void compose(CompositeOperator compose_);
    CompositeOperator compose(void) const;

    // Filename to save montages to
    void fileName(const std::string &fileName_);
    std::string fileName(void) const;

    // Fill color
    void fillColor(const Color &fill_);
    Color fillColor(void) const;

    // Label font
    void font(const std::string &font_);
    std::string font(void) const;

    // Thumbnail width & height plus border width & height
    void geometry(const Geometry &geometry_);
    Geometry geometry(void) const;

    // Thumbnail position (e.g. SouthWestGravity)
    void gravity(GravityType gravity_);
    GravityType gravity(void) const;

    // Thumbnail label (applied to image prior to montage)
    void label(const std::string &label_);
    std::string label(void) const;

    // Font point size
    void pointSize(size_t pointSize_);
    size_t pointSize(void) const;

    // Enable drop-shadows on thumbnails
    void shadow(bool shadow_);
    bool shadow(void) const;

    // Outline color
    void strokeColor(const Color &stroke_);
    Color strokeColor(void) const;

    // Background texture image
    void texture(const std::string &texture_);
    std::string texture(void) const;

    // Thumbnail rows and columns
    void tile(const Geometry &tile_);
    Geometry tile(void) const;

    // Montage title
    void title(const std::string &title_);
    std::string title(void) const;

    // Transparent color
    void transparentColor(const Color &transparentColor_);
    Color transparentColor(void) const;

    //
    // Implementation methods/members
    //

    // Update elements in existing MontageInfo structure
    virtual void updateMontageInfo(MagickCore::MontageInfo &montageInfo_) const;

  private:

    Color _backgroundColor;
    std::string _fileName;
    Color _fill;
    std::string _font;
    Geometry _geometry;
    GravityType _gravity;
    std::string _label;
    size_t _pointSize;
    bool _shadow;
    Color _stroke;
    std::string _texture;
    Geometry _tile;
    std::string _title;
    Color _transparentColor;
  };

  //
  // Montage With Frames (Extends Basic Montage)
  //
  class MagickPPExport MontageFramed : public Montage
  {
  public:

    MontageFramed(void);
    ~MontageFramed(void);

    // Frame foreground color
    void matteColor(const Color &matteColor_);
    Color matteColor(void) const;

    // Frame border color
    void borderColor(const Color &borderColor_);
    Color borderColor(void) const;

    // Pixels between thumbnail and surrounding frame
    void borderWidth(size_t borderWidth_);
    size_t borderWidth(void) const;

    // Frame geometry (width & height frame thickness)
    void frameGeometry(const Geometry &frame_);
    Geometry frameGeometry(void) const;

    //
    // Implementation methods/members
    //

    // Update elements in existing MontageInfo structure
    void updateMontageInfo(MagickCore::MontageInfo &montageInfo_) const;

  private:

    Color _matteColor;
    Color _borderColor;
    size_t _borderWidth;
    Geometry _frame;
  };
} // namespace Magick

#endif // Magick_Montage_header

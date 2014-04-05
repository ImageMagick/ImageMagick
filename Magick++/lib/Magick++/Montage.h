// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
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

    // Same as fill color
    void penColor(const Color &pen_);
    Color penColor(void) const;

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

    // Thumbnail rows and colmns
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
    CompositeOperator _compose;
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

    // Frame border color
    void borderColor(const Color &borderColor_);
    Color borderColor(void) const;

    // Pixels between thumbnail and surrounding frame
    void borderWidth(size_t borderWidth_);
    size_t borderWidth(void) const;

    // Frame geometry (width & height frame thickness)
    void frameGeometry(const Geometry &frame_);
    Geometry frameGeometry(void) const;

    // Frame foreground color
    void matteColor(const Color &matteColor_);
    Color matteColor(void) const;

    //
    // Implementation methods/members
    //

    // Update elements in existing MontageInfo structure
    void updateMontageInfo(MagickCore::MontageInfo &montageInfo_) const;

  private:

    Color _borderColor;
    size_t _borderWidth;
    Geometry _frame;
    Color _matteColor;
  };
} // namespace Magick

//
// Inlines
//

//
// Implementation of Montage
//

inline void Magick::Montage::backgroundColor(const Magick::Color &backgroundColor_)
{
  _backgroundColor=backgroundColor_;
}

inline Magick::Color Magick::Montage::backgroundColor(void) const
{
  return(_backgroundColor);
}

inline void Magick::Montage::compose(Magick::CompositeOperator compose_)
{
  _compose=compose_;
}

inline Magick::CompositeOperator Magick::Montage::compose(void) const
{
  return(_compose);
}

inline void Magick::Montage::fileName(const std::string &fileName_)
{
  _fileName=fileName_;
}

inline std::string Magick::Montage::fileName(void) const
{
  return(_fileName);
}

inline void Magick::Montage::fillColor(const Color &fill_)
{
  _fill=fill_;
}

inline Magick::Color Magick::Montage::fillColor(void) const
{
  return(_fill);
}

inline void Magick::Montage::font(const std::string &font_)
{
  _font=font_;
}

inline std::string Magick::Montage::font(void) const
{
  return(_font);
}

inline void Magick::Montage::geometry(const Magick::Geometry &geometry_)
{
  _geometry=geometry_;
}

inline Magick::Geometry Magick::Montage::geometry(void) const
{
  return(_geometry);
}

inline void Magick::Montage::gravity(Magick::GravityType gravity_)
{
  _gravity=gravity_;
}

inline Magick::GravityType Magick::Montage::gravity(void) const
{
  return(_gravity);
}

inline void Magick::Montage::label(const std::string &label_)
{
  _label=label_;
}

inline std::string Magick::Montage::label(void) const
{
  return(_label);
}

inline void Magick::Montage::penColor(const Color &pen_)
{
  _fill=pen_;
  _stroke=Color("none");
}

inline Magick::Color Magick::Montage::penColor(void) const
{
  return _fill;
}

inline void Magick::Montage::pointSize(size_t pointSize_)
{
  _pointSize=pointSize_;
}

inline size_t Magick::Montage::pointSize(void) const
{
  return(_pointSize);
}

inline void Magick::Montage::shadow(bool shadow_)
{
  _shadow=shadow_;
}

inline bool Magick::Montage::shadow(void) const
{
  return(_shadow);
}

inline void Magick::Montage::strokeColor(const Color &stroke_)
{
  _stroke=stroke_;
}

inline Magick::Color Magick::Montage::strokeColor(void) const
{
  return(_stroke);
}

inline void Magick::Montage::texture(const std::string &texture_)
{
  _texture=texture_;
}

inline std::string Magick::Montage::texture(void) const
{
  return(_texture);
}

inline void Magick::Montage::tile(const Geometry &tile_)
{
  _tile=tile_;
}

inline Magick::Geometry Magick::Montage::tile(void) const
{
  return(_tile);
}

inline void Magick::Montage::title(const std::string &title_)
{
  _title=title_;
}

inline std::string Magick::Montage::title(void) const
{
  return(_title);
}

inline void Magick::Montage::transparentColor(const Magick::Color &transparentColor_)
{
  _transparentColor=transparentColor_;
}

inline Magick::Color Magick::Montage::transparentColor(void) const
{
  return(_transparentColor);
}

//
// Implementation of MontageFramed
//

inline void Magick::MontageFramed::borderColor(const Magick::Color &borderColor_)
{
  _borderColor=borderColor_;
}

inline Magick::Color Magick::MontageFramed::borderColor(void) const
{
  return(_borderColor);
}

inline void Magick::MontageFramed::borderWidth(size_t borderWidth_)
{
  _borderWidth=borderWidth_;
}

inline size_t Magick::MontageFramed::borderWidth(void) const
{
  return(_borderWidth);
}

inline void Magick::MontageFramed::frameGeometry(const Magick::Geometry &frame_)
{
  _frame=frame_;
}

inline Magick::Geometry Magick::MontageFramed::frameGeometry(void) const
{
  return(_frame);
}

inline void Magick::MontageFramed::matteColor(const Magick::Color &matteColor_)
{
  _matteColor=matteColor_;
}

inline Magick::Color Magick::MontageFramed::matteColor(void) const
{
  return(_matteColor);
}

#endif // Magick_Montage_header

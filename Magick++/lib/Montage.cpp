// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Implementation of Montage
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/Include.h"
#include <string>
#include <string.h>

#include "Magick++/Montage.h"
#include "Magick++/Functions.h"

Magick::Montage::Montage(void)
  : _backgroundColor("#ffffff"),
    _fileName(),
    _fill("#000000ff"),
    _font(),
    _geometry("120x120+4+3>"),
    _gravity(CenterGravity),
    _label(),
    _pointSize(12),
    _shadow(false),
    _stroke(),
    _texture(),
    _tile("6x4"),
    _title(),
    _transparentColor()
{
}

Magick::Montage::~Montage(void)
{
}

void Magick::Montage::backgroundColor(const Magick::Color &backgroundColor_)
{
  _backgroundColor=backgroundColor_;
}

Magick::Color Magick::Montage::backgroundColor(void) const
{
  return(_backgroundColor);
}

void Magick::Montage::fileName(const std::string &fileName_)
{
  _fileName=fileName_;
}

std::string Magick::Montage::fileName(void) const
{
  return(_fileName);
}

void Magick::Montage::fillColor(const Color &fill_)
{
  _fill=fill_;
}

Magick::Color Magick::Montage::fillColor(void) const
{
  return(_fill);
}

void Magick::Montage::font(const std::string &font_)
{
  _font=font_;
}

std::string Magick::Montage::font(void) const
{
  return(_font);
}

void Magick::Montage::geometry(const Magick::Geometry &geometry_)
{
  _geometry=geometry_;
}

Magick::Geometry Magick::Montage::geometry(void) const
{
  return(_geometry);
}

void Magick::Montage::gravity(Magick::GravityType gravity_)
{
  _gravity=gravity_;
}

Magick::GravityType Magick::Montage::gravity(void) const
{
  return(_gravity);
}

void Magick::Montage::label(const std::string &label_)
{
  _label=label_;
}

std::string Magick::Montage::label(void) const
{
  return(_label);
}

void Magick::Montage::pointSize(size_t pointSize_)
{
  _pointSize=pointSize_;
}

size_t Magick::Montage::pointSize(void) const
{
  return(_pointSize);
}

void Magick::Montage::shadow(bool shadow_)
{
  _shadow=shadow_;
}

bool Magick::Montage::shadow(void) const
{
  return(_shadow);
}

void Magick::Montage::strokeColor(const Color &stroke_)
{
  _stroke=stroke_;
}

Magick::Color Magick::Montage::strokeColor(void) const
{
  return(_stroke);
}

void Magick::Montage::texture(const std::string &texture_)
{
  _texture=texture_;
}

std::string Magick::Montage::texture(void) const
{
  return(_texture);
}

void Magick::Montage::tile(const Geometry &tile_)
{
  _tile=tile_;
}

Magick::Geometry Magick::Montage::tile(void) const
{
  return(_tile);
}

void Magick::Montage::title(const std::string &title_)
{
  _title=title_;
}

std::string Magick::Montage::title(void) const
{
  return(_title);
}

void Magick::Montage::transparentColor(const Magick::Color &transparentColor_)
{
  _transparentColor=transparentColor_;
}

Magick::Color Magick::Montage::transparentColor(void) const
{
  return(_transparentColor);
}

void Magick::Montage::updateMontageInfo(MontageInfo &montageInfo_ ) const
{
  (void) memset(&montageInfo_,0,sizeof(montageInfo_));

  // matte_color
  montageInfo_.matte_color=Color();
  // background_color
  montageInfo_.background_color=_backgroundColor;
  // border_color
  montageInfo_.border_color=Color();
  // border_width
  montageInfo_.border_width=0;
  // filename
  if (_font.length() != 0)
    {
      _fileName.copy(montageInfo_.filename,MagickPathExtent-1);
      montageInfo_.filename[ _fileName.length() ] = 0; // null terminate
    }
  // fill
  montageInfo_.fill=_fill;
  // font
  if (_font.length() != 0)
    Magick::CloneString(&montageInfo_.font,_font);
  // geometry
  if (_geometry.isValid())
    Magick::CloneString(&montageInfo_.geometry,_geometry);
  // gravity
  montageInfo_.gravity=_gravity;
  // pointsize
  montageInfo_.pointsize=_pointSize;
  // shadow
  montageInfo_.shadow=static_cast<MagickBooleanType>
    (_shadow ? MagickTrue : MagickFalse);
  // signature (validity stamp)
  montageInfo_.signature=MagickCoreSignature;
  // stroke
  montageInfo_.stroke=_stroke;
  // texture
  if (_texture.length() != 0)
    Magick::CloneString(&montageInfo_.texture,_texture);
  // tile
  if (_tile.isValid())
    Magick::CloneString(&montageInfo_.tile,_tile);
  // title
  if (_title.length() != 0)
    Magick::CloneString(&montageInfo_.title,_title);
}

//
// Implementation of MontageFramed
//

Magick::MontageFramed::MontageFramed(void)
  : _matteColor("#bdbdbd"),
    _borderColor("#dfdfdf"),
    _borderWidth(0),
    _frame()
{
}

Magick::MontageFramed::~MontageFramed(void)
{
}

void Magick::MontageFramed::matteColor(const Magick::Color &matteColor_)
{
  _matteColor=matteColor_;
}

Magick::Color Magick::MontageFramed::matteColor(void) const
{
  return(_matteColor);
}

void Magick::MontageFramed::borderColor(const Magick::Color &borderColor_)
{
  _borderColor=borderColor_;
}

Magick::Color Magick::MontageFramed::borderColor(void) const
{
  return(_borderColor);
}

void Magick::MontageFramed::borderWidth(size_t borderWidth_)
{
  _borderWidth=borderWidth_;
}

size_t Magick::MontageFramed::borderWidth(void) const
{
  return(_borderWidth);
}

void Magick::MontageFramed::frameGeometry(const Magick::Geometry &frame_)
{
  _frame=frame_;
}

Magick::Geometry Magick::MontageFramed::frameGeometry(void) const
{
  return(_frame);
}

void Magick::MontageFramed::updateMontageInfo(MontageInfo &montageInfo_) const
{
  // Do base updates
  Montage::updateMontageInfo(montageInfo_);

  // matte_color
  montageInfo_.matte_color = _matteColor;
  // border_color
  montageInfo_.border_color=_borderColor;
  // border_width
  montageInfo_.border_width=_borderWidth;
  // frame
  if (_frame.isValid())
    Magick::CloneString(&montageInfo_.frame,_frame);
}

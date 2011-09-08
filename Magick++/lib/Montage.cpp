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

Magick::Montage::Montage ( void )
  : _backgroundColor("#ffffff"),
    _compose(OverCompositeOp),
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

Magick::Montage::~Montage( void )
{
  // Nothing to do
}

Magick::MontageFramed::MontageFramed ( void )
  : _borderColor("#dfdfdf"),
    _borderWidth(0),
    _frame(),
    _matteColor("#bdbdbd")
{
}

/* virtual */ Magick::MontageFramed::~MontageFramed ( void )
{
  // Nothing to do
}

void Magick::Montage::updateMontageInfo ( MontageInfo &montageInfo_ ) const
{

  memset(&montageInfo_,0,sizeof(MontageInfo));

  // background_color
  montageInfo_.background_color = _backgroundColor;
  // border_color
  montageInfo_.border_color = Color();
  // border_width
  montageInfo_.border_width = 0;
  // filename
  _fileName.copy( montageInfo_.filename, MaxTextExtent - 1 );
  montageInfo_.filename[ _fileName.length() ] = 0; // null terminate
  // fill
  montageInfo_.fill = _fill;
  // font
  if ( _font.length() != 0 )
    Magick::CloneString( &montageInfo_.font, _font );
  else
    montageInfo_.font=(char *) RelinquishMagickMemory(montageInfo_.font);
  // frame
  montageInfo_.frame=(char *) montageInfo_.frame;
  // geometry
  if ( _geometry.isValid() )
    Magick::CloneString( &montageInfo_.geometry, _geometry );
  else
    montageInfo_.geometry=(char *)
      RelinquishMagickMemory(montageInfo_.geometry);
  // gravity
  montageInfo_.gravity = _gravity;
  // matte_color
  montageInfo_.matte_color = Color();
  // pointsize
  montageInfo_.pointsize = _pointSize;
  // shadow
  montageInfo_.shadow = static_cast<MagickBooleanType>
    (_shadow ? MagickTrue : MagickFalse);
  // signature (validity stamp)
  montageInfo_.signature = MagickSignature;
  // stroke
  montageInfo_.stroke = _stroke;
  // texture
  if ( _texture.length() != 0 )
    Magick::CloneString( &montageInfo_.texture, _texture );
  else
    montageInfo_.texture=(char *) RelinquishMagickMemory(montageInfo_.texture);
  // tile
  if ( _tile.isValid() )
    Magick::CloneString( &montageInfo_.tile, _tile );
  else
    montageInfo_.tile=(char *) RelinquishMagickMemory(montageInfo_.tile);
  // title
  if ( _title.length() != 0 )
    Magick::CloneString( &montageInfo_.title, _title );
  else
    montageInfo_.title=(char *) RelinquishMagickMemory(montageInfo_.title);
}

//
// Implementation of MontageFramed
//

/* virtual */ void
Magick::MontageFramed::updateMontageInfo ( MontageInfo &montageInfo_ ) const
{
  // Do base updates
  Montage::updateMontageInfo ( montageInfo_ );

  // border_color
  montageInfo_.border_color = _borderColor;
  // border_width
  montageInfo_.border_width = _borderWidth;
  // frame
  if ( _frame.isValid() )
    Magick::CloneString( &montageInfo_.frame, _frame );
  else
    montageInfo_.frame=(char *) RelinquishMagickMemory(montageInfo_.frame);
  // matte_color
  montageInfo_.matte_color = _matteColor;
}

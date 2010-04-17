// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001, 2002
//
// CoderInfo implementation
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION  1

#include "Magick++/Include.h"
#include "Magick++/CoderInfo.h"
#include "Magick++/Exception.h"

using namespace std;

// Default constructor
Magick::CoderInfo::CoderInfo ( void )
  : _name(),
    _description(),
    _isReadable(false),
    _isWritable(false),
    _isMultiFrame(false)
{
}

// Copy constructor
Magick::CoderInfo::CoderInfo ( const Magick::CoderInfo &coder_ )
{
  _name         = coder_._name;
  _description  = coder_._description;
  _isReadable   = coder_._isReadable;
  _isWritable   = coder_._isWritable;
  _isMultiFrame = coder_._isMultiFrame;
}

Magick::CoderInfo::CoderInfo ( const std::string &name_ )
  : _name(),
    _description(),
    _isReadable(false),
    _isWritable(false),
    _isMultiFrame(false)
{
  ExceptionInfo exceptionInfo;
  GetExceptionInfo( &exceptionInfo );
  const Magick::MagickInfo *magickInfo = GetMagickInfo( name_.c_str(), &exceptionInfo );
  throwException( exceptionInfo );
  (void) DestroyExceptionInfo( &exceptionInfo );
  if( magickInfo == 0 )
    {
      throwExceptionExplicit(OptionError, "Coder not found", name_.c_str() );
    }
  else
    {
      _name         = string(magickInfo->name);
      _description  = string(magickInfo->description);
      _isReadable   = ((magickInfo->decoder == 0) ? false : true);
      _isWritable   = ((magickInfo->encoder == 0) ? false : true);
      _isMultiFrame = ((magickInfo->adjoin == 0) ? false : true);
    }
}

Magick::CoderInfo::~CoderInfo ( void )
{
  // Nothing to do
}

// Format name
std::string Magick::CoderInfo::name( void ) const
{
  return _name;
}

// Format description
std::string Magick::CoderInfo::description( void ) const
{
  return _description;
}

// Format is readable
bool Magick::CoderInfo::isReadable( void ) const
{
  return _isReadable;
}

// Format is writeable
bool Magick::CoderInfo::isWritable( void ) const
{
  return _isWritable;
}

// Format supports multiple frames
bool Magick::CoderInfo::isMultiFrame( void ) const
{
  return _isMultiFrame;
}

// Assignment operator
Magick::CoderInfo& Magick::CoderInfo::operator= (const CoderInfo &coder_ )
{
  // If not being set to ourself
  if (this != &coder_)
    {
      _name         = coder_._name;
      _description  = coder_._description;
      _isReadable   = coder_._isReadable;
      _isWritable   = coder_._isWritable;
      _isMultiFrame = coder_._isMultiFrame;
    }
  return *this;
}

// Construct from MagickCore::MagickInfo*
Magick::CoderInfo::CoderInfo ( const MagickCore::MagickInfo *magickInfo_ )
  : _name(string(magickInfo_->name ? magickInfo_->name : "")),
    _description(string(magickInfo_->description ? magickInfo_->description : "")),
    _isReadable(magickInfo_->decoder ? true : false),
    _isWritable(magickInfo_->encoder ? true : false),
    _isMultiFrame(magickInfo_->adjoin ? true : false)
{
  // Nothing more to do
}

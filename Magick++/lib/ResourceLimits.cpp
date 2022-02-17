// This may look like C code, but it is really -*- C++ -*-
//
// Copyright @ 2014 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Implementation of ResourceLimits
//

#define MAGICKCORE_IMPLEMENTATION  1
#define MAGICK_PLUSPLUS_IMPLEMENTATION 1

#include "Magick++/ResourceLimits.h"

void Magick::ResourceLimits::area(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(AreaResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::area(void)
{
  return(GetMagickResourceLimit(AreaResource));
}

void Magick::ResourceLimits::disk(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(DiskResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::disk(void)
{
  return(GetMagickResourceLimit(DiskResource));
}

void Magick::ResourceLimits::file(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(FileResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::file(void)
{
  return(GetMagickResourceLimit(FileResource));
}

void Magick::ResourceLimits::height(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(HeightResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::height(void)
{
  return(GetMagickResourceLimit(HeightResource));
}

void Magick::ResourceLimits::listLength(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(ListLengthResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::listLength(void)
{
  return(GetMagickResourceLimit(ListLengthResource));
}

void Magick::ResourceLimits::map(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(MapResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::map(void)
{
  return(GetMagickResourceLimit(MapResource));
}

void Magick::ResourceLimits::memory(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(MemoryResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::memory(void)
{
  return(GetMagickResourceLimit(MemoryResource));
}

void Magick::ResourceLimits::thread(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(ThreadResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::thread(void)
{
  return(GetMagickResourceLimit(ThreadResource));
}

void Magick::ResourceLimits::throttle(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(ThrottleResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::throttle(void)
{
  return(GetMagickResourceLimit(ThrottleResource));
}

void Magick::ResourceLimits::width(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(WidthResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::width(void)
{
  return(GetMagickResourceLimit(WidthResource));
}

Magick::ResourceLimits::ResourceLimits()
{
}

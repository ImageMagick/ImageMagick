// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Dirk Lemstra 2014
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
  return GetMagickResourceLimit(AreaResource);
}

void Magick::ResourceLimits::disk(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(DiskResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::disk(void)
{
  return GetMagickResourceLimit(DiskResource);
}

void Magick::ResourceLimits::map(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(MapResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::map(void)
{
  return GetMagickResourceLimit(MapResource);
}

void Magick::ResourceLimits::memory(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(MemoryResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::memory(void)
{
  return GetMagickResourceLimit(MemoryResource);
}

void Magick::ResourceLimits::thread(const MagickSizeType limit_)
{
  (void) SetMagickResourceLimit(ThreadResource,limit_);
}

MagickCore::MagickSizeType Magick::ResourceLimits::thread(void)
{
  return GetMagickResourceLimit(ThreadResource);
}

Magick::ResourceLimits::ResourceLimits()
{
}

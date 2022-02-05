// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Simplified includes for Magick++.
// Inclusion of this header is sufficient to use all Magick++ APIs.
//
#ifndef MagickPlusPlus_Header
#include <Magick++/Include.h>
#include <Magick++/Functions.h>
#include <Magick++/Image.h>
#include <Magick++/Pixels.h>
#include <Magick++/ResourceLimits.h>
#include <Magick++/STL.h>

// Don't leak our definition of the 'restrict' keyword. 'restrict' is a valid
// identifier in C++, and leaking it could cause extraneous build failures.
#ifdef restrict
#undef restrict
#endif
#define MagickPlusPlus_Header
#endif // MagickPlusPlus_Header

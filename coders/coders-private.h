/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
#ifndef MAGICK_CODERS_PRIVATE_H
#define MAGICK_CODERS_PRIVATE_H

#include "MagickCore/attribute.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/property.h"
#include "MagickCore/string_.h"

#define MagickCoderHeader(coder,offset,magic) \
  { coder, offset, (const unsigned char *) (magic), sizeof(magic)-1, MagickFalse },

#define MagickExtendedCoderHeader(coder,offset,magic,skip_spaces) \
  { coder, offset, (const unsigned char *) (magic), sizeof(magic)-1, skip_spaces },

#define MagickCoderAlias(coder,alias)  { alias, coder },

#define MagickCoderExports(coder) \
extern ModuleExport size_t \
  Register ## coder ## Image(void); \
extern ModuleExport void \
  Unregister ## coder ## Image(void);

static inline ImageType IdentifyImageCoderType(const Image *image,
  ExceptionInfo *exception)
{
  const char
    *value;

  value=GetImageProperty(image,"colorspace:auto-grayscale",exception);
  if (IsStringFalse(value) != MagickFalse)
    return(image->type);
  return(IdentifyImageType(image,exception));
}

static inline ImageType IdentifyImageCoderGrayType(const Image *image,
  ExceptionInfo *exception)
{
  const char
    *value;

  value=GetImageProperty(image,"colorspace:auto-grayscale",exception);
  if (IsStringFalse(value) != MagickFalse)
    return(UndefinedType);
  return(IdentifyImageGray(image,exception));
}

static inline MagickBooleanType IdentifyImageCoderGray(const Image *image,
  ExceptionInfo *exception)
{
  ImageType
    type;

  type=IdentifyImageCoderGrayType(image,exception);
  return(IsGrayImageType(type));
}

static inline MagickBooleanType SetImageCoderGray(Image *image,
  ExceptionInfo *exception)
{
  ImageType
    type;

  if (IsImageGray(image) != MagickFalse)
    return(MagickTrue);
  type=IdentifyImageCoderGrayType(image,exception);
  if (IsGrayImageType(type) == MagickFalse)
    return(MagickFalse);
  image->type=type;
  return(SetImageColorspace(image,GRAYColorspace,exception));
}

#endif

/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore property methods.
*/
#ifndef _MAGICKCORE_PROPERTY_H
#define _MAGICKCORE_PROPERTY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport char
  *GetNextImageProperty(const Image *),
  *InterpretImageProperties(const ImageInfo *,Image *,const char *),
  *RemoveImageProperty(Image *,const char *);

extern MagickExport const char
  *GetImageProperty(const Image *,const char *),
  *GetMagickProperty(const ImageInfo *,Image *,const char *);

extern MagickExport MagickBooleanType
  CloneImageProperties(Image *,const Image *),
  DefineImageProperty(Image *,const char *),
  DeleteImageProperty(Image *,const char *),
  FormatImageProperty(Image *,const char *,const char *,...)
    magick_attribute((__format__ (__printf__,3,4))),
  SetImageProperty(Image *,const char *,const char *);

extern MagickExport void
  DestroyImageProperties(Image *),
  ResetImagePropertyIterator(const Image *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

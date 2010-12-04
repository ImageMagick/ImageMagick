/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image constitute methods.
*/
#ifndef _MAGICKCORE_CONSTITUTE_H
#define _MAGICKCORE_CONSTITUTE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedPixel,
  CharPixel,
  DoublePixel,
  FloatPixel,
  IntegerPixel,
  LongPixel,
  QuantumPixel,
  ShortPixel
} StorageType;

extern MagickExport Image
  *ConstituteImage(const size_t,const size_t,const char *,const StorageType,
    const void *,ExceptionInfo *),
  *PingImage(const ImageInfo *,ExceptionInfo *),
  *PingImages(const ImageInfo *,ExceptionInfo *),
  *ReadImage(const ImageInfo *,ExceptionInfo *),
  *ReadImages(const ImageInfo *,ExceptionInfo *),
  *ReadInlineImage(const ImageInfo *,const char *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  ConstituteComponentGenesis(void),
  WriteImage(const ImageInfo *,Image *),
  WriteImages(const ImageInfo *,Image *,const char *,ExceptionInfo *);

extern MagickExport void
  ConstituteComponentTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

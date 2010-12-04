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

  MagickCore image transform methods.
*/
#ifndef _MAGICKCORE_TRANSFORM_H
#define _MAGICKCORE_TRANSFORM_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickExport Image
  *ChopImage(const Image *,const RectangleInfo *,ExceptionInfo *),
  *ConsolidateCMYKImages(const Image *,ExceptionInfo *),
  *CropImage(const Image *,const RectangleInfo *,ExceptionInfo *),
  *ExcerptImage(const Image *,const RectangleInfo *,ExceptionInfo *),
  *ExtentImage(const Image *,const RectangleInfo *,ExceptionInfo *),
  *FlipImage(const Image *,ExceptionInfo *),
  *FlopImage(const Image *,ExceptionInfo *),
  *RollImage(const Image *,const ssize_t,const ssize_t,ExceptionInfo *),
  *ShaveImage(const Image *,const RectangleInfo *,ExceptionInfo *),
  *SpliceImage(const Image *,const RectangleInfo *,ExceptionInfo *),
  *TransposeImage(const Image *,ExceptionInfo *),
  *TransverseImage(const Image *,ExceptionInfo *),
  *TrimImage(const Image *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  TransformImage(Image **,const char *,const char *),
  TransformImages(Image **,const char *,const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

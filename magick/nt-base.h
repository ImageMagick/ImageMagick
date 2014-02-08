/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore Windows NT utility methods.
*/
#ifndef _MAGICKCORE_NT_BASE_H
#define _MAGICKCORE_NT_BASE_H

#include "magick/exception.h"
#include "magick/geometry.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_WINDOWS_SUPPORT)
typedef struct _GhostInfo
  GhostInfo_;

extern MagickExport char
  **NTArgvToUTF8(const int argc,wchar_t **);

extern MagickExport const GhostInfo_
  *NTGhostscriptDLLVectors(void);

extern MagickExport int
  NTGhostscriptUnLoadDLL(void);

extern MagickExport void
  NTErrorHandler(const ExceptionType,const char *,const char *),
  NTWarningHandler(const ExceptionType,const char *,const char *);
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

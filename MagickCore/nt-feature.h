/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore Windows NT utility methods.
*/
#ifndef MAGICKCORE_NT_FEATURE_H
#define MAGICKCORE_NT_FEATURE_H

#include "MagickCore/exception.h"
#include "MagickCore/geometry.h"
#include "MagickCore/splay-tree.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(XS_VERSION)

extern MagickExport MagickBooleanType
  NTIsMagickConflict(const char *),
  NTAcquireTypeCache(SplayTreeInfo *,ExceptionInfo *);

#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

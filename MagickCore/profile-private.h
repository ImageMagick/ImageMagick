/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore pribate image profile methods.
*/
#ifndef MAGICKCORE_PROFILE_PRIVATE_H
#define MAGICKCORE_PROFILE_PRIVATE_H

#include "MagickCore/string_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickPrivate MagickBooleanType
  SyncImageProfiles(Image *);

extern MagickPrivate void
  Update8BIMClipPath(const Image *,const size_t,const size_t,
    const RectangleInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif 
#endif

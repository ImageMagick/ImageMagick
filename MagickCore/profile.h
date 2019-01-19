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

  MagickCore image profile methods.
*/
#ifndef MAGICKCORE_PROFILE_H
#define MAGICKCORE_PROFILE_H

#include "MagickCore/string_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _ProfileInfo
  ProfileInfo;

typedef enum
{
  UndefinedIntent,
  SaturationIntent,
  PerceptualIntent,
  AbsoluteIntent,
  RelativeIntent
} RenderingIntent;

extern MagickExport char
  *GetNextImageProfile(const Image *);

extern MagickExport const StringInfo
  *GetImageProfile(const Image *,const char *);

extern MagickExport MagickBooleanType
  CloneImageProfiles(Image *,const Image *),
  DeleteImageProfile(Image *,const char *),
  ProfileImage(Image *,const char *,const void *,const size_t,ExceptionInfo *),
  SetImageProfile(Image *,const char *,const StringInfo *,ExceptionInfo *);

extern MagickExport StringInfo
  *RemoveImageProfile(Image *,const char *);

extern MagickExport void
  DestroyImageProfiles(Image *),
  ResetImageProfileIterator(const Image *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif 
#endif

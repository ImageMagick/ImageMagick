/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore configure methods.
*/
#ifndef _MAGICKCORE_CONFIGURE_H
#define _MAGICKCORE_CONFIGURE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/hashmap.h"

typedef struct _ConfigureInfo
{
  char
    *path,
    *name,
    *value;
                                                                                
  MagickBooleanType
    stealth;
                                                                                
  struct _ConfigureInfo
    *previous,
    *next;  /* deprecated, use GetConfigureInfoList() */

  unsigned long
    signature;
} ConfigureInfo;

extern MagickExport char
  **GetConfigureList(const char *,unsigned long *,ExceptionInfo *),
  *GetConfigureOption(const char *);

extern MagickExport const char
  *GetConfigureValue(const ConfigureInfo *);

extern MagickExport const ConfigureInfo
  *GetConfigureInfo(const char *,ExceptionInfo *),
  **GetConfigureInfoList(const char *,unsigned long *,ExceptionInfo *);

extern MagickExport LinkedListInfo
  *DestroyConfigureOptions(LinkedListInfo *),
  *GetConfigurePaths(const char *,ExceptionInfo *),
  *GetConfigureOptions(const char *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  ListConfigureInfo(FILE *,ExceptionInfo *);

extern MagickExport void
  DestroyConfigureList(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

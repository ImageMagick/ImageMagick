/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore locale methods.
*/
#ifndef _MAGICKCORE_LOCALE_H
#define _MAGICKCORE_LOCALE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/hashmap.h"

typedef struct _LocaleInfo
{
  char
    *path,
    *tag,
    *message;
                                                                                
  MagickBooleanType
    stealth;
                                                                                
  struct _LocaleInfo
    *previous,
    *next;  /* deprecated, use GetLocaleInfoList() */

  size_t
    signature;
} LocaleInfo;

extern MagickExport char
  **GetLocaleList(const char *,size_t *,ExceptionInfo *);

extern MagickExport const char
  *GetLocaleMessage(const char *);

extern MagickExport const LocaleInfo
  *GetLocaleInfo_(const char *,ExceptionInfo *),
  **GetLocaleInfoList(const char *,size_t *,ExceptionInfo *);

extern MagickExport double
  InterpretLocaleValue(const char *restrict,char **restrict);

extern MagickExport LinkedListInfo
  *DestroyLocaleOptions(LinkedListInfo *),
  *GetLocaleOptions(const char *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  ListLocaleInfo(FILE *,ExceptionInfo *),
  LocaleComponentGenesis(void);

extern MagickExport ssize_t
  FormatLocaleFile(FILE *,const char *restrict,...)
    magick_attribute((__format__ (__printf__,2,3))),
  FormatLocaleFileList(FILE *,const char *restrict,va_list)
    magick_attribute((__format__ (__printf__,2,0))),
  FormatLocaleString(char *restrict,const size_t,const char *restrict,...)
    magick_attribute((__format__ (__printf__,3,4))),
  FormatLocaleStringList(char *restrict,const size_t,const char *restrict,
    va_list) magick_attribute((__format__ (__printf__,3,0)));

extern MagickExport void
  LocaleComponentTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

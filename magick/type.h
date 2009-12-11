/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image type methods.
*/
#ifndef _MAGICKCORE_TYPE_H
#define _MAGICKCORE_TYPE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedStretch,
  NormalStretch,
  UltraCondensedStretch,
  ExtraCondensedStretch,
  CondensedStretch,
  SemiCondensedStretch,
  SemiExpandedStretch,
  ExpandedStretch,
  ExtraExpandedStretch,
  UltraExpandedStretch,
  AnyStretch
} StretchType;

typedef enum
{
  UndefinedStyle,
  NormalStyle,
  ItalicStyle,
  ObliqueStyle,
  AnyStyle
} StyleType;

typedef struct _TypeInfo
{
  unsigned long
    face;

  char
    *path,
    *name,
    *description,
    *family;

  StyleType
    style;

  StretchType
    stretch;

  unsigned long
    weight;

  char
    *encoding,
    *foundry,
    *format,
    *metrics,
    *glyphs;

  MagickBooleanType
    stealth;

  struct _TypeInfo
    *previous,
    *next;  /* deprecated, use GetTypeInfoList() */

  unsigned long
    signature;
} TypeInfo;

extern MagickExport char
  **GetTypeList(const char *,unsigned long *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  ListTypeInfo(FILE *,ExceptionInfo *),
  TypeComponentGenesis(void);

extern MagickExport const TypeInfo
  *GetTypeInfo(const char *,ExceptionInfo *),
  *GetTypeInfoByFamily(const char *,const StyleType,const StretchType,
    const unsigned long,ExceptionInfo *),
  **GetTypeInfoList(const char *,unsigned long *,ExceptionInfo *);

MagickExport void
  TypeComponentTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

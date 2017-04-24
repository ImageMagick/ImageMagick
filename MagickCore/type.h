/*
  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image type methods.
*/
#ifndef MAGICKCORE_TYPE_H
#define MAGICKCORE_TYPE_H

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
  size_t
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

  size_t
    weight;

  char
    *encoding,
    *foundry,
    *format,
    *metrics,
    *glyphs;

  MagickBooleanType
    stealth;

  size_t
    signature;
} TypeInfo;

extern MagickExport char
  **GetTypeList(const char *,size_t *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  ListTypeInfo(FILE *,ExceptionInfo *);

extern MagickExport const TypeInfo
  *GetTypeInfo(const char *,ExceptionInfo *),
  *GetTypeInfoByFamily(const char *,const StyleType,const StretchType,
    const size_t,ExceptionInfo *),
  **GetTypeInfoList(const char *,size_t *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

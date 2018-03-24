/*
  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore string methods.
*/
#ifndef MAGICKCORE_STRING_H_
#define MAGICKCORE_STRING_H_

#include "MagickCore/exception.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _StringInfo
{
  char
    *path;

  unsigned char
    *datum;

  size_t
    length,
    signature;

  char
    *name;
} StringInfo;

extern MagickExport char
  *AcquireString(const char *),
  *CloneString(char **,const char *),
  *ConstantString(const char *),
  *DestroyString(char *),
  **DestroyStringList(char **),
  *EscapeString(const char *,const char),
  *FileToString(const char *,const size_t,ExceptionInfo *),
  *GetEnvironmentValue(const char *),
  *SanitizeString(const char *),
  *StringInfoToHexString(const StringInfo *),
  *StringInfoToString(const StringInfo *),
  **StringToArgv(const char *,int *),
  *StringToken(const char *,char **),
  **StringToList(const char *),
  **StringToStrings(const char *,size_t *);

extern MagickExport const char
  *GetStringInfoName(const StringInfo *),
  *GetStringInfoPath(const StringInfo *);

extern MagickExport double
  InterpretSiPrefixValue(const char *magick_restrict,char **magick_restrict),
  *StringToArrayOfDoubles(const char *,ssize_t *,ExceptionInfo *);

extern MagickExport int
  CompareStringInfo(const StringInfo *,const StringInfo *);

extern MagickExport MagickBooleanType
  ConcatenateString(char **,const char *),
  IsStringTrue(const char *),
  IsStringFalse(const char *),
  SubstituteString(char **,const char *,const char *);

extern MagickExport size_t
  ConcatenateMagickString(char *,const char *,const size_t)
    magick_attribute((__nonnull__)),
  CopyMagickString(char *,const char *,const size_t)
    magick_attribute((__nonnull__)),
  GetStringInfoLength(const StringInfo *);

extern MagickExport ssize_t
  FormatMagickSize(const MagickSizeType,const MagickBooleanType,const char *,
    const size_t,char *),
  FormatMagickTime(const time_t,const size_t,char *);

extern MagickExport StringInfo
  *AcquireStringInfo(const size_t),
  *BlobToStringInfo(const void *,const size_t),
  *CloneStringInfo(const StringInfo *),
  *ConfigureFileToStringInfo(const char *),
  *DestroyStringInfo(StringInfo *),
  *FileToStringInfo(const char *,const size_t,ExceptionInfo *),
  *SplitStringInfo(StringInfo *,const size_t),
  *StringToStringInfo(const char *);

extern MagickExport unsigned char
  *GetStringInfoDatum(const StringInfo *);

extern MagickExport void
  ConcatenateStringInfo(StringInfo *,const StringInfo *)
    magick_attribute((__nonnull__)),
  PrintStringInfo(FILE *file,const char *,const StringInfo *),
  ResetStringInfo(StringInfo *),
  SetStringInfo(StringInfo *,const StringInfo *),
  SetStringInfoDatum(StringInfo *,const unsigned char *),
  SetStringInfoLength(StringInfo *,const size_t),
  SetStringInfoName(StringInfo *,const char *),
  SetStringInfoPath(StringInfo *,const char *),
  StripString(char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

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

  MagickCore utility methods.
*/
#ifndef MAGICKCORE_UTILITY_H
#define MAGICKCORE_UTILITY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedPath,
  MagickPath,
  RootPath,
  HeadPath,
  TailPath,
  BasePath,
  ExtensionPath,
  SubimagePath,
  CanonicalPath,
  SubcanonicalPath
} PathType;

extern MagickExport char
  *Base64Encode(const unsigned char *,const size_t,size_t *);

extern MagickExport MagickBooleanType
  AcquireUniqueFilename(char *),
  AcquireUniqueSymbolicLink(const char *,char *),
  ExpandFilenames(int *,char ***),
  GetPathAttributes(const char *,void *),
  IsPathAccessible(const char *);

extern MagickExport size_t
  MultilineCensus(const char *) magick_attribute((__pure__));

extern MagickExport unsigned char
  *Base64Decode(const char *, size_t *);

extern MagickExport void
  AppendImageFormat(const char *,char *),
  GetPathComponent(const char *,PathType,char *),
  MagickDelay(const MagickSizeType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

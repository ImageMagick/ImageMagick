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

  MagickWand command-line option process.
*/
#ifndef MAGICKWAND_WAND_CLI_H
#define MAGICKWAND_WAND_CLI_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _MagickCLI
  MagickCLI;

extern WandExport MagickCLI
  *AcquireMagickCLI(ImageInfo *,ExceptionInfo *),
  *DestroyMagickCLI(MagickCLI *);

extern WandExport MagickBooleanType
  CLICatchException(MagickCLI *,const MagickBooleanType),
  CLILogEvent(MagickCLI *,const LogEventType,const char *,const char *,
    const size_t, const char *,...)
    magick_attribute((__format__ (__printf__,6,7))),
  CLIThrowException(MagickCLI *,const char *,const char *,const size_t,
    const ExceptionType,const char *,const char *,...)
    magick_attribute((__format__ (__printf__,7,8)));


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

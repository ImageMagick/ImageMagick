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

  MagickWand private command-line option process.
*/
#ifndef MAGICKWAND_OPERATION_PRIVATE_H
#define MAGICKWAND_OPERATION_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* These actually private */
extern WandPrivate MagickBooleanType
  CLIListOperatorImages(MagickCLI *, const char *,const char *,const char *);

extern WandPrivate void
  CLISettingOptionInfo(MagickCLI *,const char *,const char *, const char *),
  CLISimpleOperatorImages(MagickCLI *,const char *,const char *,const char *),
  CLINoImageOperator(MagickCLI *, const char *,const char *,const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

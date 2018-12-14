/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand command-line option process.
*/
#ifndef MAGICKWAND_OPERATION_H
#define MAGICKWAND_OPERATION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern WandExport void
  CLIOption(MagickCLI *,const char *,...)
    magick_attribute((__format__ (__printf__,2,0)));

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

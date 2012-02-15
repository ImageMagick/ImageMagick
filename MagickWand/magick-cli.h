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

  MagickWand convert command-line method.
*/
#ifndef _MAGICKWAND_MAGICK_CLI_H
#define _MAGICKWAND_MAGICK_CLI_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  /* What options should be processed */
  ProcessNonOptionImageRead = 0x0001,  /* non-option is a image read */
  ProcessUnknownOptionError = 0x0002,  /* unknown option produces error */

  ProcessReadOption         = 0x0010,  /* allow '-read' to read images */

  ProcessListOption         = 0x0040,  /* Process Image List Operators */

  ProcessCommandOptions     = 0x0FFF,  /* Magick Command Flags */

  /* Modify Option Handling */
  ProcessOutputFile         = 0x1000,  /* Process the output file */
  ProcessOneOptionOnly      = 0x8000   /* Process One Option Only */

} OptionProcessFlags;

extern WandExport void
  MagickSpecialOption(MagickWand *,const char *,const char *),
  MagickCommandProcessOptions(MagickWand *,int,char **,
       int *index, OptionProcessFlags flags);

extern WandExport MagickBooleanType
  MagickImageCommand(ImageInfo *,int,char **,char **,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

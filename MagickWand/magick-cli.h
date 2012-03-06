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
  /* NonOption Handling */
  ProcessNonOptionImageRead   = 0x0001,  /* non-option is a image read */
  ProcessUnknownOptionError   = 0x0002,  /* unknown option produces error */

  /* Special Option Handling */
  ProcessExitOption           = 0x0100,  /* allow '-exit' use */
  ProcessScriptOption         = 0x0200,  /* allow '-script' use */
  ProcessReadOption           = 0x0400,  /* allow '-read' use */

  /* Option Processing Flags */
  ProcessOneOptionOnly        = 0x4000,  /* Process One Option Only */
  ProcessOutputFile           = 0x8000,  /* Process the output file */

  /* Flag Groups for specific Situations */
  CommandCommandOptionFlags   = 0x80FF,  /* Convert Command Flags */
  MagickCommandOptionFlags    = 0x8FFF,  /* Magick Command Flags */
  MagickScriptArgsFlags       = 0x00FF,  /* Script Args Flags */
  MagickScriptReadFlags       = 0x01FF   /* Script Read Flags */

} ProcessOptionFlags;

extern WandExport void
  ProcessScriptOptions(MagickCLI *,int,char **,int);

extern WandExport int
  ProcessCommandOptions(MagickCLI *,int,char **,int,ProcessOptionFlags);

extern WandExport MagickBooleanType
  MagickImageCommand(ImageInfo *,int,char **,char **,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

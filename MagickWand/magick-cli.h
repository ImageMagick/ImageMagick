/*
  Copyright @ 2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand convert command-line method.
*/
#ifndef MAGICKWAND_MAGICK_CLI_H
#define MAGICKWAND_MAGICK_CLI_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef MagickBooleanType
  (*MagickCommand)(ImageInfo *,int,char **,char **,ExceptionInfo *);

extern WandExport void
  ProcessScriptOptions(MagickCLI *,const char *,int,char **,int);

extern WandExport int
  ProcessCommandOptions(MagickCLI *,int,char **,int);

extern WandExport MagickBooleanType
  MagickCommandGenesis(ImageInfo *,MagickCommand,int,char **,char **,
    ExceptionInfo *),
  MagickImageCommand(ImageInfo *,int,char **,char **,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

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

  ImageMagick pixel wand API.
*/
#ifndef MAGICKWAND_WANDCLI_PRIVATE_H
#define MAGICKWAND_WANDCLI_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define CLIWandException(severity,tag,option) \
  (void) CLIThrowException(cli_wand,GetMagickModule(),severity,tag, \
       "`%s'",option)

#define CLIWandExceptionArg(severity,tag,option,arg) \
  (void) CLIThrowException(cli_wand,GetMagickModule(),severity,tag, \
       "'%s' '%s'",option, arg)

#define CLIWandWarnReplaced(message) \
  if ( (cli_wand->process_flags & ProcessWarnDeprecated) != 0 ) \
    (void) CLIThrowException(cli_wand,GetMagickModule(),OptionWarning, \
       "ReplacedOption", "'%s', use \"%s\"",option,message)

#define CLIWandExceptionFile(severity,tag,context) \
{ char *message=GetExceptionMessage(errno); \
  (void) CLIThrowException(cli_wand,GetMagickModule(),severity,tag, \
       "'%s': %s",context,message); \
  message=DestroyString(message); \
}

#define CLIWandExceptionBreak(severity,tag,option) \
  { CLIWandException(severity,tag,option); break; }

#define CLIWandExceptionReturn(severity,tag,option) \
  { CLIWandException(severity,tag,option); return; }

#define CLIWandExceptArgBreak(severity,tag,option,arg) \
  { CLIWandExceptionArg(severity,tag,option,arg); break; }

#define CLIWandExceptArgReturn(severity,tag,option,arg) \
  { CLIWandExceptionArg(severity,tag,option,arg); return; }



/* Define how options should be processed */
typedef enum
{
  /* General Option Handling */
  ProcessImplictRead          = 0x0001,  /* Non-options are image reads.
                                            If not set then skip implied read
                                            without producing an error.
                                            For use with "mogrify" handling */
  ProcessInterpretProperities = 0x0010,  /* allow general escapes in args */

  /* Special Option Handling */
  ProcessExitOption           = 0x0100,  /* allow '-exit' use */
  ProcessScriptOption         = 0x0200,  /* allow '-script' use */
  ProcessReadOption           = 0x0400,  /* allow '-read' use */
  ProcessWarnDeprecated       = 0x0800,  /* warn about deprecated options */

  /* Option Processing Flags */
  ProcessOneOptionOnly        = 0x4000,  /* Process one option only */
  ProcessImplictWrite         = 0x8000,  /* Last arg is an implict write */

  /* Flag Groups for specific Situations */
  MagickCommandOptionFlags    = 0x8FFF,  /* Magick Command Flags */
  ConvertCommandOptionFlags   = 0x800F,  /* Convert Command Flags */
  MagickScriptArgsFlags       = 0x000F,  /* Script CLI Process Args Flags */
} ProcessOptionFlags;


/* Define a generic stack linked list, for pushing and popping
   user defined ImageInfo settings, and Image lists.
   See '(' ')' and '-clone' CLI options.
*/
typedef struct _Stack
{
  struct _Stack  *next;
  void           *data;
} Stack;

/* Note this defines an extension to the normal MagickWand
   Which adds extra elements specific to the Shell API interface
   while still allowing the Wand to be passed to MagickWand API
   for specific operations.
*/
struct _MagickCLI       /* CLI interface version of MagickWand */
{
  struct _MagickWand    /* This must be the first structure */
     wand;              /* The Image List and Global Option Settings */

  QuantizeInfo
    *quantize_info;     /* for CLI API usage, not used by MagickWand API */

  DrawInfo
    *draw_info;         /* for CLI API usage, not used by MagickWand API */

  ProcessOptionFlags
    process_flags;      /* When handling CLI, what options do we process? */

  const OptionInfo
    *command;           /* The option entry that is being processed */

  Stack
    *image_list_stack,  /* Stacks of Image Lists and Image Info settings */
    *image_info_stack;

  const char            /* Location of option being processed for exception */
    *location,          /* location format string for exception reports */
    *filename;          /* "CLI", "unknown", or the script filename */

  size_t
    line,               /* location of current option from source */
    column;             /* note: line also used for cli argument count */

  size_t
    signature;
};



#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

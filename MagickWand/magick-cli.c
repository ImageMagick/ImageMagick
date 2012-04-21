/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                 M   M   AAA    GGGG  IIIII   CCCC  K   K                    %
%                 MM MM  A   A  G        I    C      K  K                     %
%                 M M M  AAAAA  G GGG    I    C      KKK                      %
%                 M   M  A   A  G   G    I    C      K  K                     %
%                 M   M  A   A   GGGG  IIIII   CCCC  K   K                    %
%                                                                             %
%                            CCCC  L      IIIII                               %
%                           C      L        I                                 %
%                           C      L        I                                 %
%                           C      L        I                                 %
%                            CCCC  LLLLL  IIIII                               %
%                                                                             %
%       Perform "Magick" on Images via the Command Line Interface             %
%                                                                             %
%                             Dragon Computing                                %
%                             Anthony Thyssen                                 %
%                               January 2012                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Read CLI arguments, script files, and pipelines, to provide options that
%  manipulate images from many different formats.
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/magick-wand-private.h"
#include "MagickWand/wandcli.h"
#include "MagickWand/wandcli-private.h"
#include "MagickWand/operation.h"
#include "MagickWand/magick-cli.h"
#include "MagickWand/script-token.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/version.h"

/* verbose debugging,
      3 - option type details
      9 - output options/artifacts/propertys
*/
#define MagickCommandDebug 0


#if MagickCommandDebug >= 9
/*
  Temporary Debugging Information
  FUTURE: these should be able to be printed out using 'percent escapes'
  Actually 'Properities' can already be output with  "%[*]"
*/
static void OutputOptions(ImageInfo *image_info)
{
  const char
    *option,
    *value;

  (void) FormatLocaleFile(stdout,"  Global Options:\n");
  ResetImageOptionIterator(image_info);
  while ((option=GetNextImageOption(image_info)) != (const char *) NULL ) {
    (void) FormatLocaleFile(stdout,"    %s: ",option);
    value=GetImageOption(image_info,option);
    if (value != (const char *) NULL)
      (void) FormatLocaleFile(stdout,"%s\n",value);
  }
  ResetImageOptionIterator(image_info);
}

static void OutputArtifacts(Image *image)
{
  const char
    *artifact,
    *value;

  (void) FormatLocaleFile(stdout,"  Image Artifacts:\n");
  ResetImageArtifactIterator(image);
  while ((artifact=GetNextImageArtifact(image)) != (const char *) NULL ) {
    (void) FormatLocaleFile(stdout,"    %s: ",artifact);
    value=GetImageArtifact(image,artifact);
    if (value != (const char *) NULL)
      (void) FormatLocaleFile(stdout,"%s\n",value);
  }
  ResetImageArtifactIterator(image);
}

static void OutputProperties(Image *image,ExceptionInfo *exception)
{
  const char
    *property,
    *value;

  (void) FormatLocaleFile(stdout,"  Image Properity:\n");
  ResetImagePropertyIterator(image);
  while ((property=GetNextImageProperty(image)) != (const char *) NULL ) {
    (void) FormatLocaleFile(stdout,"    %s: ",property);
    value=GetImageProperty(image,property,exception);
    if (value != (const char *) NULL)
      (void) FormatLocaleFile(stdout,"%s\n",value);
  }
  ResetImagePropertyIterator(image);
}
#endif


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   P r o c e s s S c r i p t O p t i o n s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ProcessScriptOptions() reads options and processes options as they are
%  found in the given file, or pipeline.  The filename to open and read
%  options is given as the 'index' argument of the argument array given.
%
%  Other arguments following index may be read by special script options
%  as settings (strings), images, or as operations to be processed in various
%  ways.   How they are treated is up to the script being processed.
%
%  Note that a script not 'return' to the command line processing, nor can
%  they call (and return from) other scripts. At least not at this time.
%
%  There are no 'ProcessOptionFlags' control flags at this time.
%
%  The format of the ProcessScriptOptions method is:
%
%    void ProcessScriptOptions(MagickCLI *cli_wand,int argc,char **argv,
%               int index)
%
%  A description of each parameter follows:
%
%    o cli_wand: the main CLI Wand to use.
%
%    o argc: the number of elements in the argument vector.
%
%    o argv: A text array containing the command line arguments.
%
%    o index: offset for argc to CLI argumnet count
%
*/
WandExport void ProcessScriptOptions(MagickCLI *cli_wand,int argc,char **argv,
     int index)
{
  ScriptTokenInfo
    *token_info;

  CommandOptionFlags
    option_type;

  int
    count;

  char
    *option,
    *arg1,
    *arg2;

  assert(argc>index); /* at least one argument - script name */
  assert(argv != (char **)NULL);
  assert(argv[index] != (char *)NULL);
  assert(argv[argc-1] != (char *)NULL);
  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == WandSignature);
  if (cli_wand->wand.debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",cli_wand->wand.name);

  /* open file script or stream, and set up tokenizer */
  token_info = AcquireScriptTokenInfo(argv[index]);
  if (token_info == (ScriptTokenInfo *) NULL) {
    CLIWandExceptionFile(OptionFatalError,"UnableToOpenScript",argv[index]);
    return;
  }

  /* define the error location string for use in exceptions
     order of localtion format escapes: filename, line, column */
  cli_wand->location="in \"%s\" at line %u,column %u";
  if ( LocaleCompare("-", argv[index]) == 0 )
    cli_wand->filename="stdin";
  else
    cli_wand->filename=argv[index];

  /* Process Options from Script */
  option = arg1 = arg2 = (char*)NULL;
  while (1) {

    /* Get a option */
    { MagickBooleanType status = GetScriptToken(token_info);
      cli_wand->line=token_info->token_line;
      cli_wand->column=token_info->token_column;
      if( IfMagickFalse(status) )
        break; /* error or end of options */
    }

    /* save option details */
    CloneString(&option,token_info->token);

    { /* get option type and argument count */
      const OptionInfo *option_info = GetCommandOptionInfo(option);
      count=option_info->type;
      option_type=(CommandOptionFlags) option_info->flags;
#if 0
      (void) FormatLocaleFile(stderr, "Script: %u,%u: \"%s\" matched \"%s\"\n",
             cli_wand->line, cli_wand->line, option, option_info->mnemonic );
#endif
    }

    /* handle a undefined option - image read? */
    if ( option_type == UndefinedOptionFlag ||
         (option_type & NonMagickOptionFlag) != 0 ) {
#if MagickCommandDebug >= 3
      (void) FormatLocaleFile(stderr, "Script %u,%u Non-Option: \"%s\"\n",
                  cli_wand->line, cli_wand->line, option);
#endif
      if ( IfMagickFalse(IsCommandOption(option))) {
        /* non-option -- treat as a image read */
        CLISpecialOperator(cli_wand,"-read",option);
        goto next_token;
      }
      if ( LocaleCompare(option,"-script") == 0 ) {
        option_type=SpecialOptionFlag;
        count=1;
        /* fall thru - collect one argument */
      }
      else {
        CLIWandExceptionBreak(OptionFatalError,"UnrecognizedOption",option);
        goto next_token;
      }
    }

    if ( count >= 1 ) {
      if( IfMagickFalse(GetScriptToken(token_info)) )
        CLIWandException(OptionFatalError,"MissingArgument",option);
      CloneString(&arg1,token_info->token);
    }
    else
      CloneString(&arg1,(char *)NULL);

    if ( count >= 2 ) {
      if( IfMagickFalse(GetScriptToken(token_info)) )
        CLIWandExceptionBreak(OptionFatalError,"MissingArgument",option);
      CloneString(&arg2,token_info->token);
    }
    else
      CloneString(&arg2,(char *)NULL);

    /*
      Process Options
    */
#if MagickCommandDebug >= 3
    (void) FormatLocaleFile(stderr,
      "Script %u,%u Option: \"%s\"  Count: %d  Flags: %04x  Args: \"%s\" \"%s\"\n",
          cli_wand->line,cli_wand->line,option,count,option_type,arg1,arg2);
#endif
    /* Hard Depreciated Options, no code to execute - error */
    if ( (option_type & DeprecateOptionFlag) != 0 ) {
      CLIWandException(OptionError,"DeprecatedOptionNoCode",option);
      if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
        break;
      goto next_token;
    }

    /* MagickCommandGenesis() options have no place in a magick script */
    if ( (option_type & GenesisOptionFlag) != 0 ) {
      CLIWandExceptionBreak(OptionError,"InvalidUseOfOption",option);
      goto next_token;
    }

    if ( (option_type & SpecialOptionFlag) != 0 ) {
      if ( LocaleCompare(option,"-exit") == 0 ) {
        break; /* forced end of script */
      }
      else if ( LocaleCompare(option,"-script") == 0 ) {
        /* FUTURE: call new script from this script */
        CLIWandExceptionBreak(OptionError,"InvalidUseOfOption",option);
        goto next_token;
      }
      /* FUTURE: handle special script-argument options here */
      /* handle any other special operators now */
      CLISpecialOperator(cli_wand,option,arg1);
    }

    if ( (option_type & SettingOptionFlags) != 0 ) {
      CLISettingOptionInfo(cli_wand, option, arg1, arg2);
      // FUTURE: Sync Specific Settings into Image Properities (not global)
    }

    /* FUTURE: The not a setting part below is a temporary hack to stop gap
     * measure for options that are BOTH settings and optional 'Simple/List'
     * operators.  Specifically -monitor, -depth, and  -colorspace */
    if ( cli_wand->wand.images == (Image *)NULL ) {
      if (((option_type & ImageRequiredFlags) != 0 ) &&
          ((option_type & SettingOptionFlags) == 0 ))  /* temp hack */
        CLIWandException(OptionError,"NoImagesFound",option);
      goto next_token;
    }

    /* FUTURE: this is temporary - get 'settings' to handle
       distribution of settings to images attributes,proprieties,artifacts */
    SyncImagesSettings(cli_wand->wand.image_info,cli_wand->wand.images,
          cli_wand->wand.exception);

    if ( (option_type & SimpleOperatorOptionFlag) != 0)
      CLISimpleOperatorImages(cli_wand, option, arg1, arg2);

    if ( (option_type & ListOperatorOptionFlag) != 0 )
      CLIListOperatorImages(cli_wand, option, arg1, arg2);

next_token:
#if MagickCommandDebug >= 9
    OutputOptions(cli_wand->wand.image_info);
    if ( cli_wand->wand.images != (Image *)NULL ) {
      OutputArtifacts(cli_wand->wand.images);
      OutputProperties(cli_wand->wand.images,cli_wand->wand.exception);
    }
#endif
    if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
      break;
  }

#if MagickCommandDebug >= 3
  (void) FormatLocaleFile(stderr, "Script End: %d\n", token_info->status);
#endif
  switch( token_info->status ) {
    case TokenStatusOK:
    case TokenStatusEOF:
      if (cli_wand->image_list_stack != (Stack *)NULL)
        CLIWandException(OptionError,"UnbalancedParenthesis", "(eof)");
      else if (cli_wand->image_info_stack != (Stack *)NULL)
        CLIWandException(OptionError,"UnbalancedBraces", "(eof)");
      break;
    case TokenStatusBadQuotes:
      /* Ensure last token has a sane length for error report */
      if( strlen(token_info->token) > INITAL_TOKEN_LENGTH-1 ) {
        token_info->token[INITAL_TOKEN_LENGTH-4] = '.';
        token_info->token[INITAL_TOKEN_LENGTH-3] = '.';
        token_info->token[INITAL_TOKEN_LENGTH-2] = '.';
        token_info->token[INITAL_TOKEN_LENGTH-1] = '\0';
      }
      CLIWandException(OptionFatalError,"ScriptUnbalancedQuotes",
           token_info->token);
      break;
    case TokenStatusMemoryFailed:
      CLIWandException(OptionFatalError,"ScriptTokenMemoryFailed","");
      break;
    case TokenStatusBinary:
      CLIWandException(OptionFatalError,"ScriptIsBinary","");
      break;
  }

  /* Clean up */
  token_info = DestroyScriptTokenInfo(token_info);

  CloneString(&option,(char *)NULL);
  CloneString(&arg1,(char *)NULL);
  CloneString(&arg2,(char *)NULL);

  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  P r o c e s s C o m m a n d O p t i o n s                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ProcessCommandOptions() reads and processes arguments in the given
%  command line argument array. The array does not contain the command
%  being processed, only the options.
%
%  The 'process_flags' can be used to control and limit option processing.
%  For example, to only process one option, or how unknown and special options
%  are to be handled, and if the last argument in array is to be regarded as a
%  final image write argument (filename or special coder).
%
%  The format of the ProcessCommandOptions method is:
%
%    int ProcessCommandOptions(MagickCLI *cli_wand,int argc,char **argv,
%           int index, ProcessOptionFlags process_flags )
%
%  A description of each parameter follows:
%
%    o cli_wand: the main CLI Wand to use.
%
%    o argc: the number of elements in the argument vector.
%
%    o argv: A text array containing the command line arguments.
%
%    o process_flags: What type of arguments will be processed, ignored
%                     or return errors.
%
%    o index: index in the argv array to start processing from
%
% The function returns the index ot the next option to be processed. This
% is really only releven if process_flags contains a ProcessOneOptionOnly
% flag.
%
*/
WandExport int ProcessCommandOptions(MagickCLI *cli_wand, int argc,
     char **argv, int index )
{
  const char
    *option,
    *arg1,
    *arg2;

  int
    i,
    end,
    count;

  CommandOptionFlags
    option_type;

  assert(argc>=index); /* you may have no arguments left! */
  assert(argv != (char **)NULL);
  assert(argv[index] != (char *)NULL);
  assert(argv[argc-1] != (char *)NULL);
  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == WandSignature);
  if (cli_wand->wand.debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",cli_wand->wand.name);

  /* define the error location string for use in exceptions
     order of localtion format escapes: filename, line, column */
  cli_wand->location="at %s argument %u";
  cli_wand->filename="CLI";

  end = argc;
  if ( (cli_wand->process_flags & ProcessImpliedWrite) != 0 )
    end--; /* the last arument is an implied write, do not process directly */

  for (i=index; i < end; i += count +1) {
    /* Finished processing one option? */
    if ( (cli_wand->process_flags & ProcessOneOptionOnly) != 0 && i != index )
      return(i);

    option=argv[i];
    cli_wand->line=i;  /* note the argument for this option */

    { const OptionInfo *option_info = GetCommandOptionInfo(argv[i]);
      count=option_info->type;
      option_type=(CommandOptionFlags) option_info->flags;
#if 0
      (void) FormatLocaleFile(stderr, "CLI %d: \"%s\" matched \"%s\"\n",
            i, argv[i], option_info->mnemonic );
#endif
    }

    if ( option_type == UndefinedOptionFlag ||
         (option_type & NonMagickOptionFlag) != 0 ) {
#if MagickCommandDebug >= 3
      (void) FormatLocaleFile(stderr, "CLI %d Non-Option: \"%s\"\n", i, option);
#endif
      if ( IfMagickFalse(IsCommandOption(option)) ) {
         if ( (cli_wand->process_flags & ProcessNonOptionImageRead) != 0 )
           /* non-option -- treat as a image read */
           CLISpecialOperator(cli_wand,"-read",option);
         else
           CLIWandException(OptionFatalError,"UnrecognizedOption",option);
         goto next_argument;
      }
      if ( ((cli_wand->process_flags & ProcessScriptOption) != 0) &&
           (LocaleCompare(option,"-script") == 0) ) {
        /* Call Script from CLI, with a filename as a zeroth argument.
           NOTE: -script may need to use 'implict write filename' so it
           must be handled here to prevent 'missing argument' error.
        */
        ProcessScriptOptions(cli_wand,argc,argv,i+1);
        return(argc);  /* Script does not return to CLI -- Yet -- FUTURE */
      }
      CLIWandException(OptionFatalError,"UnrecognizedOption",option);
      goto next_argument;
    }

    if ((i+count) >= end ) {
      CLIWandException(OptionFatalError,"MissingArgument",option);
      if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
        return(end);
      goto next_argument; /* no more arguments unable to proceed */
    }

    arg1 = ( count >= 1 ) ? argv[i+1] : (char *)NULL;
    arg2 = ( count >= 2 ) ? argv[i+2] : (char *)NULL;

    /*
      Process Known Options
    */
#if MagickCommandDebug >= 3
    (void) FormatLocaleFile(stderr,
      "CLI %u Option: \"%s\"  Count: %d  Flags: %04x  Args: \"%s\" \"%s\"\n",
          i,option,count,option_type,arg1,arg2);
#endif
    /* Hard Depreciated Options, no code to execute - error */
    if ( (option_type & DeprecateOptionFlag) != 0 ) {
      CLIWandException(OptionError,"DeprecatedOptionNoCode",option);
      goto next_argument;
    }

    /* Ignore MagickCommandGenesis() only option on CLI */
    if ( (option_type & GenesisOptionFlag) != 0 )
      goto next_argument;

    if ( (option_type & SpecialOptionFlag) != 0 ) {
      if ( (cli_wand->process_flags & ProcessExitOption) != 0
           && LocaleCompare(option,"-exit") == 0 )
        return(i+count);
      /* handle any other special operators now */
      CLISpecialOperator(cli_wand,option,arg1);
    }

    if ( (option_type & SettingOptionFlags) != 0 ) {
      CLISettingOptionInfo(cli_wand, option, arg1, arg2);
      // FUTURE: Sync individual Settings into images (no SyncImageSettings())
    }

    /* FUTURE: The not a setting part below is a temporary hack to stop gap
     * measure for options that are BOTH settings and optional 'Simple/List'
     * operators.  Specifically -monitor, -depth, and  -colorspace */
    if ( cli_wand->wand.images == (Image *)NULL ) {
      if (((option_type & ImageRequiredFlags) != 0 ) &&
          ((option_type & SettingOptionFlags) == 0 )  )  /* temp hack */
        CLIWandException(OptionError,"NoImagesFound",option);
      goto next_argument;
    }

    /* FUTURE: this is temporary - get 'settings' to handle
       distribution of settings to images attributes,proprieties,artifacts */
    SyncImagesSettings(cli_wand->wand.image_info,cli_wand->wand.images,
          cli_wand->wand.exception);

    if ( (option_type & SimpleOperatorOptionFlag) != 0)
      CLISimpleOperatorImages(cli_wand, option, arg1, arg2);

    if ( (option_type & ListOperatorOptionFlag) != 0 )
      CLIListOperatorImages(cli_wand, option, arg1, arg2);

next_argument:
#if MagickCommandDebug >= 9
    OutputOptions(cli_wand->wand.image_info);
    if ( cli_wand->wand.images != (Image *)NULL ) {
      OutputArtifacts(cli_wand->wand.images);
      OutputProperties(cli_wand->wand.images,cli_wand->wand.exception);
    }
#endif
    if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
      return(i+count);
  }
  assert(i==end);

  if ( (cli_wand->process_flags & ProcessImpliedWrite) == 0 )
    return(end); /* no implied write -- just return to caller */

  assert(end==argc-1); /* end should not include last argument */

  /*
     Implicit Write of images to final CLI argument
  */
  option=argv[i];
  cli_wand->line=i;

#if MagickCommandDebug >= 3
  (void) FormatLocaleFile(stderr, "CLI %d Write File: \"%s\"\n", i, option );
#endif

  /* check that stacks are empty */
  if (cli_wand->image_list_stack != (Stack *)NULL)
    CLIWandException(OptionError,"UnbalancedParenthesis", "(eof)");
  else if (cli_wand->image_info_stack != (Stack *)NULL)
    CLIWandException(OptionError,"UnbalancedBraces", "(eof)");
  if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
    return(argc);

  /* This is a valid 'do no write' option - no images needed */
  if (LocaleCompare(option,"-exit") == 0 )
    return(argc);  /* just exit, no image write */

  /* If filename looks like an option,
     Or the common 'end of line' error of a single space.
     -- produce an error */
  if (IfMagickTrue(IsCommandOption(option)) ||
      (option[0] == ' ' && option[1] == '\0') ) {
    CLIWandException(OptionError,"MissingOutputFilename",option);
    return(argc);
  }

  CLISpecialOperator(cli_wand,"-write",option);
  return(argc);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k I m a g e C o m m a n d                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickImageCommand() Handle special use CLI arguments and prepare a
%  CLI MagickCLI to process the command line or directly specified script.
%
%  This is essentualy interface function between the MagickCore library
%  initialization function MagickCommandGenesis(), and the option MagickCLI
%  processing functions  ProcessCommandOptions()  or  ProcessScriptOptions()
%
%  The format of the MagickImageCommand method is:
%
%      MagickBooleanType MagickImageCommand(ImageInfo *image_info,
%           int argc, char **argv, char **metadata, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the starting image_info structure
%         (for compatibilty with MagickCommandGenisis())
%
%    o argc: the number of elements in the argument vector.
%
%    o argv: A text array containing the command line arguments.
%
%    o metadata: any metadata (for VBS) is returned here.
%         (for compatibilty with MagickCommandGenisis())
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void MagickUsage(MagickBooleanType verbose)
{
  const char
    *name;

  size_t
    len;

  name=GetClientName();
  len=strlen(name);

  if (len>=7 && LocaleCompare("convert",name+len-7) == 0) {
    /* convert usage */
    (void) FormatLocaleFile(stdout,
       "Usage: %s [{option}|{image}...] {output_image}\n",name);
    (void) FormatLocaleFile(stdout,
       "       %s -help|-version|-usage|-list {option}\n\n",name);
    return;
  }
  else if (len>=6 && LocaleCompare("script",name+len-6) == 0) {
    /* magick-script usage */
    (void) FormatLocaleFile(stdout,
       "Usage: %s {filename} [{script_args}...]\n",name);
  }
  else {
    /* magick usage */
    (void) FormatLocaleFile(stdout,
       "Usage: %s [{option}|{image}...] {output_image}\n",name);
    (void) FormatLocaleFile(stdout,
       "       %s [{option}|{image}...] -script {filename} [{script_args}...]\n",
       name);
  }
  (void) FormatLocaleFile(stdout,
       "       %s -help|-version|-usage|-list {option}\n\n",name);

  if (IfMagickFalse(verbose))
    return;

  (void) FormatLocaleFile(stdout,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
    "All options are performed in a strict 'as you see them' order\n",
    "You must read-in images before you can operate on them.\n",
    "\n",
    "Magick Script files can use any of the following forms...\n",
    "     #!/path/to/magick -script\n",
    "or\n",
    "     #!/bin/sh\n",
    "     :; exec magick -script \"$0\" \"$@\"; exit 10\n",
    "     # Magick script from here...\n",
    "or\n",
    "     #!/usr/bin/env  magick-script\n",
    "The latter two forms do not require the path to the command hard coded.\n",
    "Note: \"magick-script\" needs to be linked to the \"magick\" command.\n",
    "\n",
    "For more information on usage, options, examples, and techniques\n",
    "see the ImageMagick website at    ", MagickAuthoritativeURL);

  return;
}

/*
   Concatanate given file arguments to the given output argument.
   Used for a special -concatenate option used for specific 'delegates'.
   The option is not formally documented.

      magick -concatenate files... output

   This is much like the UNIX "cat" command, but for both UNIX and Windows,
   however the last argument provides the output filename.
*/
static MagickBooleanType ConcatenateImages(int argc,char **argv,
     ExceptionInfo *exception )
{
  FILE
    *input,
    *output;

  int
    c;

  register ssize_t
    i;

  output=fopen_utf8(argv[argc-1],"wb");
  if (output == (FILE *) NULL) {
    ThrowFileException(exception,FileOpenError,"UnableToOpenFile",argv[argc-1]);
    return(MagickFalse);
  }
  for (i=2; i < (ssize_t) (argc-1); i++) {
    input=fopen_utf8(argv[i],"rb");
    if (input == (FILE *) NULL)
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",argv[i]);
    for (c=fgetc(input); c != EOF; c=fgetc(input))
      (void) fputc((char) c,output);
    (void) fclose(input);
    (void) remove_utf8(argv[i]);
  }
  (void) fclose(output);
  return(MagickTrue);
}

WandExport MagickBooleanType MagickImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **metadata,ExceptionInfo *exception)
{
  MagickCLI
    *cli_wand;

  size_t
    len;

  /* For specific OS command line requirements */
  ReadCommandlLine(argc,&argv);

#if 0
  status=ExpandFilenames(&argc,&argv);
  if ( IfMagickFalse(status) )
    ThrowConvertException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
#endif

  /* Initialize special "CLI Wand" to hold images and settings (empty) */
  cli_wand=AcquireMagickCLI(image_info,exception);
  cli_wand->line=1;

  GetPathComponent(argv[0],TailPath,cli_wand->wand.name);
  SetClientName(cli_wand->wand.name);
  ConcatenateMagickString(cli_wand->wand.name,"-CLI",MaxTextExtent);

  len=strlen(argv[0]);  /* precaution */

  /* "convert" command - give a "depreciation" warning" */
  if (len>=7 && LocaleCompare("convert",argv[0]+len-7) == 0) {
    cli_wand->process_flags = ConvertCommandOptionFlags;
    /*(void) FormatLocaleFile(stderr,"WARNING: %s\n",
             "The convert is depreciated in IMv7, use \"magick\"\n");*/
  }

  /* Special Case:  If command name ends with "script" implied "-script" */
  if (len>=6 && LocaleCompare("script",argv[0]+len-6) == 0) {
    if (argc >= 2 && (  (*(argv[1]) != '-') || (strlen(argv[1]) == 1) )) {
      GetPathComponent(argv[1],TailPath,cli_wand->wand.name);
      ProcessScriptOptions(cli_wand,argc,argv,1);
      goto Magick_Command_Cleanup;
    }
  }

  /* Special Case: Version Information and Abort */
  if (argc == 2) {
    if (LocaleCompare("-version",argv[1]) == 0) {
      CLISpecialOperator(cli_wand, "-version", (char *)NULL);
      goto Magick_Command_Exit;
    }
    if ((LocaleCompare("-help",argv[1]) == 0)   || /* GNU standard option */
        (LocaleCompare("--help",argv[1]) == 0) ) {
      MagickUsage(MagickFalse);
      goto Magick_Command_Exit;
    }
    if (LocaleCompare("-usage",argv[1]) == 0) {
      CLISpecialOperator(cli_wand, "-version", (char *)NULL);
      MagickUsage(MagickTrue);
      goto Magick_Command_Exit;
    }
  }

  /* not enough arguments -- including -help */
  if (argc < 3) {
    (void) FormatLocaleFile(stderr,
       "Error: Invalid argument or not enough arguments\n\n");
    MagickUsage(MagickFalse);
    goto Magick_Command_Exit;
  }

  /* List Information and Abort */
  if (LocaleCompare("-list",argv[1]) == 0) {
    CLISpecialOperator(cli_wand, argv[1], argv[2]);
    goto Magick_Command_Exit;
  }

  /* Special "concatenate option (hidden) for delegate usage */
  if (LocaleCompare("-concatenate",argv[1]) == 0) {
    ConcatenateImages(argc,argv,exception);
    goto Magick_Command_Exit;
  }

  /* ------------- */
  /* The Main Call */

  if (LocaleCompare("-script",argv[1]) == 0) {
    /* Start processing directly from script, no pre-script options
       Replace wand command name with script name
       First argument in the argv array is the script name to read.
    */
    GetPathComponent(argv[2],TailPath,cli_wand->wand.name);
    ProcessScriptOptions(cli_wand,argc,argv,2);
  }
  else {
    /* Normal Command Line, assumes output file as last option */
    ProcessCommandOptions(cli_wand,argc,argv,1);
  }
  /* ------------- */

Magick_Command_Cleanup:
  /* recover original image_info and clean up stacks
     FUTURE: "-reset stacks" option  */
  while (cli_wand->image_list_stack != (Stack *)NULL)
    CLISpecialOperator(cli_wand,")",(const char *)NULL);
  while (cli_wand->image_info_stack != (Stack *)NULL)
    CLISpecialOperator(cli_wand,"}",(const char *)NULL);

  /* assert we have recovered the original structures */
  assert(cli_wand->wand.image_info == image_info);
  assert(cli_wand->wand.exception == exception);

  /* Handle metadata for ImageMagickObject COM object for Windows VBS */
  if (metadata != (char **) NULL) {
    const char
      *format;

    char
      *text;

    format="%w,%h,%m";   // Get this from image_info Option splaytree

    text=InterpretImageProperties(image_info,cli_wand->wand.images,format,
         exception);
    if (text == (char *) NULL)
      ThrowMagickException(exception,GetMagickModule(),ResourceLimitError,
           "MemoryAllocationFailed","'%s'", GetExceptionMessage(errno));
    else {
      (void) ConcatenateString(&(*metadata),text);
      text=DestroyString(text);
    }
  }

Magick_Command_Exit:
  /* Destroy the special CLI Wand */
  cli_wand->wand.image_info = (ImageInfo *)NULL; /* not these */
  cli_wand->wand.exception = (ExceptionInfo *)NULL;
  cli_wand=DestroyMagickCLI(cli_wand);

  return(IsMagickTrue(exception->severity > ErrorException));
}

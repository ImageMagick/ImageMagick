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
#include "MagickWand/operation.h"
#include "MagickWand/operation-private.h"
#include "MagickWand/magick-cli.h"
#include "MagickWand/script-token.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"

/* verbose debugging,
      1 - option type
      2 - source of option
      3 - mnemonic lookup  */
#define MagickCommandDebug 0

#define ThrowFileException(exception,severity,tag,context) \
{ \
  char \
    *message; \
 \
  message=GetExceptionMessage(errno); \
  (void) ThrowMagickException(exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s': %s",context,message); \
  message=DestroyString(message); \
}

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
  if (token_info->token == (char *) NULL) {
    ThrowFileException(cli_wand->wand.exception,OptionFatalError,
               "UnableToOpenScript",argv[index]);
    return;
  }

  /* define the error location string for use in exceptions
     order of input escapes: option, filename, line, column
  */
  cli_wand->location="'%s' in \"%s\" line %u column %u";
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
      if( status == MagickFalse )
        break;
    }

    /* Sanity check: option is larger than anything that should be posible */
    if( strlen(token_info->token) > INITAL_TOKEN_LENGTH-1 ) {
      token_info->token[INITAL_TOKEN_LENGTH-4] = '.';
      token_info->token[INITAL_TOKEN_LENGTH-3] = '.';
      token_info->token[INITAL_TOKEN_LENGTH-2] = '.';
      token_info->token[INITAL_TOKEN_LENGTH-1] = '\0';
      CLIWandException(OptionFatalError,"UnrecognizedOption",token_info->token);
      break;
    }

    /* save option details */
    CloneString(&option,token_info->token);

    { /* get option type and argument count */
      const OptionInfo *option_info = GetCommandOptionInfo(option);
      count=option_info->type;
      option_type=option_info->flags;
#if MagickCommandDebug >= 2
      (void) FormatLocaleFile(stderr, "Script: %u,%u: \"%s\" matched \"%s\"\n",
             cli_wand->line, cli_wand->line, option, option_info->mnemonic );
#endif
    }

    /* handle a undefined option - image read? */
    if ( option_type == UndefinedOptionFlag ||
         (option_type & NonMagickOptionFlag) != 0 ) {
#if MagickCommandDebug
      (void) FormatLocaleFile(stderr, "Script Non-Option: \"%s\"\n", option);
#endif
      if ( IsCommandOption(option) == MagickFalse) {
        /* non-option -- treat as a image read */
        CLISpecialOperator(cli_wand,"-read",option);
        count = 0;
      }
      else
        CLIWandExceptionBreak(OptionFatalError,"UnrecognizedOption",option);

      if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
        break;
      continue;
    }

    if ( count >= 1 ) {
      if( GetScriptToken(token_info) == MagickFalse )
        CLIWandExceptionBreak(OptionFatalError,"MissingArgument",option);
      CloneString(&arg1,token_info->token);
    }
    else
      CloneString(&arg1,(char *)NULL);

    if ( count >= 2 ) {
      if( GetScriptToken(token_info) == MagickFalse )
        CLIWandExceptionBreak(OptionFatalError,"MissingArgument",option);
      CloneString(&arg2,token_info->token);
    }
    else
      CloneString(&arg2,(char *)NULL);

    /* handle script special options here */
    //either continue processing command line
    // or making use of the command line options.
    //CLICommandOptions(cli_wand,count+1,argv, MagickScriptArgsFlags);

#if MagickCommandDebug
    (void) FormatLocaleFile(stderr,
        "Script Option: \"%s\" \tCount: %d  Flags: %04x  Args: \"%s\" \"%s\"\n",
        option,(int) count,option_type,arg1,arg2);
#endif

    /* Process non-script specific option from file */
    if ( (option_type & SpecialOptionFlag) != 0 ) {
      if ( LocaleCompare(option,"-exit") == 0 )
        break;
      /* No "-script" option from script at this time */
      CLISpecialOperator(cli_wand,option,arg1);
    }

    if ( (option_type & SettingOptionFlags) != 0 ) {
      CLISettingOptionInfo(cli_wand, option, arg1);
      // FUTURE: Sync Specific Settings into Images
    }

    if ( (option_type & SimpleOperatorOptionFlag) != 0)
      CLISimpleOperatorImages(cli_wand, option, arg1, arg2);

    if ( (option_type & ListOperatorOptionFlag) != 0 )
      CLIListOperatorImages(cli_wand, option, arg1, arg2);

    if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
      break;
  }

#if MagickCommandDebug
  (void) FormatLocaleFile(stderr, "Script End: %d\n", token_info->status);
#endif
  switch( token_info->status ) {
    case TokenStatusOK:
    case TokenStatusEOF:
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
%    o process_flags: What type of arguments we are allowed to process
%
%    o index: index in the argv array to start processing from
%
% The function returns the index ot the next option to be processed. This
% is really only releven if process_flags contains a ProcessOneOptionOnly
% flag.
%
*/
WandExport int ProcessCommandOptions(MagickCLI *cli_wand, int argc,
     char **argv, int index, ProcessOptionFlags process_flags )
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

  /*
    Parse command-line options.
  */
  cli_wand->location="'%s' %s arg %d";
  cli_wand->filename="CLI";

  end = argc;
  if ( ( process_flags & ProcessOutputFile ) != 0 )
    end--;

  for (i=index; i < end; i += count +1) {
    /* Finished processing one option? */
    if ( ( process_flags & ProcessOneOptionOnly ) != 0 && i != index )
      return(i);

    option=argv[i];
    cli_wand->line=i;

    { const OptionInfo *option_info = GetCommandOptionInfo(argv[i]);
      count=option_info->type;
      option_type=option_info->flags;
#if MagickCommandDebug >= 2
      (void) FormatLocaleFile(stderr, "CLI %d: \"%s\" matched \"%s\"\n",
            i, argv[i], option_info->mnemonic );
#endif
    }

    if ( option_type == UndefinedOptionFlag ||
         (option_type & NonMagickOptionFlag) != 0 ) {
#if MagickCommandDebug
      (void) FormatLocaleFile(stderr, "CLI Non-Option: \"%s\"\n", option);
#endif
      if ( ( IsCommandOption(option) == MagickFalse ) &&
         ( (process_flags & ProcessNonOptionImageRead) != 0 ) ) {
        /* non-option -- treat as a image read */
        CLISpecialOperator(cli_wand,"-read",option);
        count = 0;
      }
      else if ( (process_flags & ProcessUnknownOptionError) != 0 )
        CLIWandException(OptionFatalError,"UnrecognizedOption",option);

      if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
        return(i+1);

      continue;
    }

    if ( (option_type & DeprecateOptionFlag) != 0 ) {
      CLIWandException(OptionWarning,"DeprecatedOption",option);
      if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
        return(i+count+1);
    }
    if ((i+count) >= end ) {
      CLIWandException(OptionFatalError,"MissingArgument",option);
      if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
        return(end);
      continue; /* no arguments unable to proceed */
    }

    arg1 = ( count >= 1 ) ? argv[i+1] : (char *)NULL;
    arg2 = ( count >= 2 ) ? argv[i+2] : (char *)NULL;

#if MagickCommandDebug
    (void) FormatLocaleFile(stderr,
        "CLI Option: \"%s\" \tCount: %d  Flags: %04x  Args: \"%s\" \"%s\"\n",
        option,(int) count,option_type,arg1,arg2);
#endif

    if ( (option_type & SpecialOptionFlag) != 0 ) {
      if ( ( process_flags & ProcessExitOption ) != 0
           && LocaleCompare(option,"-exit") == 0 )
        return(i+count);
      if ( ( process_flags & ProcessScriptOption ) != 0
           && LocaleCompare(option,"-script") == 0) {
        // Unbalanced Parenthesis if stack not empty
        // Call Script, with a filename as a zeroth argument
        ProcessScriptOptions(cli_wand,argc,argv,i+1);
        return(argc); /* no more options after script process! */
      }
      CLISpecialOperator(cli_wand,option,arg1);
    }

    if ( (option_type & SettingOptionFlags) != 0 ) {
      CLISettingOptionInfo(cli_wand, option, arg1);
      // FUTURE: Sync Specific Settings into Images
    }

    if ( (option_type & SimpleOperatorOptionFlag) != 0)
      CLISimpleOperatorImages(cli_wand, option, arg1, arg2);

    if ( (option_type & ListOperatorOptionFlag) != 0 )
      CLIListOperatorImages(cli_wand, option, arg1, arg2);

    if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
      return(i+count);
  }
  assert(i==end);

  if ( ( process_flags & ProcessOutputFile ) == 0 )
    return(end);

  assert(end==argc-1);

  /*
     Implicit Write of images to final CLI argument
  */
  option=argv[i];

#if MagickCommandDebug
  (void) FormatLocaleFile(stderr, "CLI Write File: \"%s\"\n", option );
#endif

  // if stacks are not empty
  //  ThrowConvertException(OptionError,"UnbalancedParenthesis",option,i);

  /* This is a valid 'do no write' option for a CLI */
  if (LocaleCompare(option,"-exit") == 0 )
    return(argc);  /* just exit, no image write */

  /* If there is an option -- produce an error */
  if (IsCommandOption(option) != MagickFalse) {
    CLIWandException(OptionError,"MissingOutputFilename",option);
    return(argc);
  }

  /* If no images in MagickCLI */
  if ( cli_wand->wand.images == (Image *) NULL ) {
    /* a "null:" output coder with no images is not an error! */
    if ( LocaleCompare(option,"null:") == 0 )
      return(argc);
    CLIWandException(OptionError,"NoImagesForFinalWrite",option);
    return(argc);
  }

#if 0
  WandListOperatorImages(cli_wand,"-write",option,(const char *)NULL);
#else
  (void) SyncImagesSettings(cli_wand->wand.image_info,cli_wand->wand.images,
       cli_wand->wand.exception);
  (void) WriteImages(cli_wand->wand.image_info,cli_wand->wand.images,option,
       cli_wand->wand.exception);
#endif
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
%    o metadata: any metadata is returned here.
%         (for compatibilty with MagickCommandGenisis())
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType MagickUsage(void)
{
  printf("Version: %s\n",GetMagickVersion((size_t *) NULL));
  printf("Copyright: %s\n",GetMagickCopyright());
  printf("Features: %s\n\n",GetMagickFeatures());
  printf("\n");

  printf("Usage: %s [(options|images) ...] output_image\n", GetClientName());
  printf("       %s -script filename [script args...]\n", GetClientName());
  printf("       ... | %s -script - | ...\n", GetClientName());
  printf("\n");

  printf("  For more information on usage, options, examples, and technqiues\n");
  printf("  see the ImageMagick website at\n    %s\n", MagickAuthoritativeURL);
  printf("  Or the web pages in ImageMagick Sources\n");
  return(MagickFalse);
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
  ExceptionInfo *exception)
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
    ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
      argv[argc-1]);
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

  const char
    *option;

  /* Handle special single use options */
  if (argc == 2) {
    option=argv[1];
    if ((LocaleCompare("-version",option+1) == 0) ||
        (LocaleCompare("--version",option+1) == 0) ) {
      (void) FormatLocaleFile(stdout,"Version: %s\n",
        GetMagickVersion((size_t *) NULL));
      (void) FormatLocaleFile(stdout,"Copyright: %s\n",
        GetMagickCopyright());
      (void) FormatLocaleFile(stdout,"Features: %s\n\n",
        GetMagickFeatures());
      return(MagickFalse);
    }
  }
  /* The "magick" command must have at least two arguments */
  if (argc < 3)
    return(MagickUsage());
  ReadCommandlLine(argc,&argv);

#if 0
  /* FUTURE: This does not make sense!  Remove it.
     Only a 'image read' needs to expand file name glob patterns
  */
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowConvertException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
#endif

  /* Special option (hidden) for delegate usage - no wand needed */
  if (LocaleCompare("-concatenate",argv[1]) == 0)
    return(ConcatenateImages(argc,argv,exception));

  /* Initialize special "CLI Wand" to hold images and settings (empty) */
  /* FUTURE: add this to 'operations.c' */
  cli_wand=AcquireMagickCLI(image_info,exception);

  if (LocaleCompare("-list",argv[1]) == 0)
    /* Special option, list information and exit
       FUTURE: this should be a MagickCore option,
       especially as no wand is actually needed!
    */
    CLISpecialOperator(cli_wand, argv[1], argv[2]);
  else if (LocaleCompare("-script",argv[1]) == 0) {
    /* Start processing directly from script, no pre-script options
       Replace wand command name with script name
       First argument in the argv array is the script name to read.
    */
    GetPathComponent(argv[2],TailPath,cli_wand->wand.name);
    ProcessScriptOptions(cli_wand,argc,argv,2);
  }
  else {
    /* Processing Command line, assuming output file as last option */
    GetPathComponent(argv[0],TailPath,cli_wand->wand.name);
    ProcessCommandOptions(cli_wand,argc,argv,1,MagickCommandOptionFlags);
  }

  /* recover original image_info from bottom of stack */
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
           "MemoryAllocationFailed","`%s'", GetExceptionMessage(errno));
    else {
      (void) ConcatenateString(&(*metadata),text);
      text=DestroyString(text);
    }
  }
  /* Destroy the special CLI Wand */
  cli_wand->wand.image_info = (ImageInfo *)NULL; /* not these */
  cli_wand->wand.exception = (ExceptionInfo *)NULL;
  cli_wand=DestroyMagickCLI(cli_wand);

  return((exception->severity > ErrorException) ? MagickFalse : MagickTrue);
}

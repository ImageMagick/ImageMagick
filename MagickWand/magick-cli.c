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
%        Perform "Magick" on Images via the Command Line Interface            %
%                                                                             %
%                             Dragon Computing                                %
%                             Anthony Thyssen                                 %
%                               January 2012                                  %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
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
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/version.h"

/* verbose debugging,
      0 - no debug lines
      3 - show option details  (better to use -debug Command now)
      5 - image counts (after option runs)
*/
#define MagickCommandDebug 0

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a g i c k C o m m a n d G e n e s i s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCommandGenesis() applies image processing options to an image as
%  prescribed by command line options.
%
%  It wiil look for special options like "-debug", "-bench", and
%  "-distribute-cache" that needs to be applied even before the main
%  processing begins, and may completely overrule normal command processing.
%  Such 'Genesis' Options can only be given on the CLI, (not in a script)
%  and are typically ignored (as they have been handled) if seen later.
%
%  The format of the MagickCommandGenesis method is:
%
%      MagickBooleanType MagickCommandGenesis(ImageInfo *image_info,
%        MagickCommand command,int argc,char **argv,char **metadata,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o command: Choose from ConvertImageCommand, IdentifyImageCommand,
%      MogrifyImageCommand, CompositeImageCommand, CompareImagesCommand,
%      ConjureImageCommand, StreamImageCommand, ImportImageCommand,
%      DisplayImageCommand, or AnimateImageCommand.
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%    o metadata: any metadata is returned here.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WandExport MagickBooleanType MagickCommandGenesis(ImageInfo *image_info,
  MagickCommand command,int argc,char **argv,char **metadata,
  ExceptionInfo *exception)
{
  char
    client_name[MagickPathExtent],
    *option;

  double
    duration,
    serial;

  MagickBooleanType
    concurrent,
    regard_warnings,
    status;

  size_t
    iterations,
    number_threads;

  ssize_t
    i,
    n;

  (void) setlocale(LC_ALL,"");
  (void) setlocale(LC_NUMERIC,"C");
  GetPathComponent(argv[0],TailPath,client_name);
  (void) SetClientName(client_name);
  concurrent=MagickFalse;
  duration=(-1.0);
  iterations=1;
  status=MagickTrue;
  regard_warnings=MagickFalse;
  for (i=1; i < (ssize_t) (argc-1); i++)
  {
    option=argv[i];
    if ((strlen(option) == 1) || ((*option != '-') && (*option != '+')))
      continue;
    if (LocaleCompare("-bench",option) == 0)
      iterations=StringToUnsignedLong(argv[++i]);
    if (LocaleCompare("-concurrent",option) == 0)
      concurrent=MagickTrue;
    if (LocaleCompare("-debug",option) == 0)
      (void) SetLogEventMask(argv[++i]);
    if (LocaleCompare("-distribute-cache",option) == 0)
      {
        DistributePixelCacheServer(StringToInteger(argv[++i]),exception);
        exit(0);
      }
    if (LocaleCompare("-duration",option) == 0)
      duration=StringToDouble(argv[++i],(char **) NULL);
    if (LocaleCompare("-regard-warnings",option) == 0)
      regard_warnings=MagickTrue;
  }
  if (iterations == 1)
    {
      char
        *text;

      text=(char *) NULL;
      status=command(image_info,argc,argv,&text,exception);
      if (exception->severity != UndefinedException)
        {
          if ((exception->severity > ErrorException) ||
              (regard_warnings != MagickFalse))
            status=MagickFalse;
          CatchException(exception);
        }
      if (text != (char *) NULL)
        {
          if (metadata != (char **) NULL)
            (void) ConcatenateString(&(*metadata),text);
          text=DestroyString(text);
        }
      return(status);
    }
  number_threads=GetOpenMPMaximumThreads();
  serial=0.0;
  for (n=1; n <= (ssize_t) number_threads; n++)
  {
    double
      e,
      parallel,
      user_time;

    TimerInfo
      *timer;

    (void) SetMagickResourceLimit(ThreadResource,(MagickSizeType) n);
    timer=AcquireTimerInfo();
    if (concurrent == MagickFalse)
      {
        for (i=0; i < (ssize_t) iterations; i++)
        {
          char
            *text;

          text=(char *) NULL;
          if (status == MagickFalse)
            continue;
          if (duration > 0)
            {
              if (GetElapsedTime(timer) > duration)
                continue;
              (void) ContinueTimer(timer);
            }
          status=command(image_info,argc,argv,&text,exception);
          if (exception->severity != UndefinedException)
            {
              if ((exception->severity > ErrorException) ||
                  (regard_warnings != MagickFalse))
                status=MagickFalse;
              CatchException(exception);
            }
          if (text != (char *) NULL)
            {
              if (metadata != (char **) NULL)
                (void) ConcatenateString(&(*metadata),text);
              text=DestroyString(text);
            }
          }
      }
    else
      {
        SetOpenMPNested(1);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
        # pragma omp parallel for shared(status)
#endif
        for (i=0; i < (ssize_t) iterations; i++)
        {
          char
            *text;

          text=(char *) NULL;
          if (status == MagickFalse)
            continue;
          if (duration > 0)
            {
              if (GetElapsedTime(timer) > duration)
                continue;
              (void) ContinueTimer(timer);
            }
          status=command(image_info,argc,argv,&text,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
          # pragma omp critical (MagickCore_MagickCommandGenesis)
#endif
          {
            if (exception->severity != UndefinedException)
              {
                if ((exception->severity > ErrorException) ||
                    (regard_warnings != MagickFalse))
                  status=MagickFalse;
                CatchException(exception);
              }
            if (text != (char *) NULL)
              {
                if (metadata != (char **) NULL)
                  (void) ConcatenateString(&(*metadata),text);
                text=DestroyString(text);
              }
          }
        }
      }
    user_time=GetUserTime(timer);
    parallel=GetElapsedTime(timer);
    e=1.0;
    if (n == 1)
      serial=parallel;
    else
      e=((1.0/(1.0/((serial/(serial+parallel))+(1.0-(serial/(serial+parallel)))/
        (double) n)))-(1.0/(double) n))/(1.0-1.0/(double) n);
    (void) FormatLocaleFile(stderr,
      "  Performance[%.20g]: %.20gi %0.3fips %0.6fe %0.6fu %lu:%02lu.%03lu\n",
      (double) n,(double) iterations,(double) iterations/parallel,e,user_time,
      (unsigned long) (parallel/60.0),(unsigned long) floor(fmod(parallel,
      60.0)),(unsigned long) (1000.0*(parallel-floor(parallel))+0.5));
    timer=DestroyTimerInfo(timer);
  }
  return(status);
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
%    void ProcessScriptOptions(MagickCLI *cli_wand,const char *filename,
%       int argc,char **argv,int index)
%
%  A description of each parameter follows:
%
%    o cli_wand: the main CLI Wand to use.
%
%    o filename: the filename of script to process
%
%    o argc: the number of elements in the argument vector. (optional)
%
%    o argv: A text array containing the command line arguments. (optional)
%
%    o index: offset of next argument in argv (script arguments) (optional)
%
*/
WandExport void ProcessScriptOptions(MagickCLI *cli_wand,const char *filename,
  int magick_unused(argc),char **magick_unused(argv),int magick_unused(index))
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

  magick_unreferenced(argc);
  magick_unreferenced(argv);
  magick_unreferenced(index);
  assert(filename != (char *) NULL ); /* at least one argument - script name */
  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == MagickWandSignature);
  if (cli_wand->wand.debug != MagickFalse)
    (void) LogMagickEvent(CommandEvent,GetMagickModule(),
         "Processing script \"%s\"", filename);

  /* open file script or stream, and set up tokenizer */
  token_info = AcquireScriptTokenInfo(filename);
  if (token_info == (ScriptTokenInfo *) NULL) {
    CLIWandExceptionFile(OptionFatalError,"UnableToOpenScript",filename);
    return;
  }

  /* define the error location string for use in exceptions
     order of location format escapes: filename, line, column */
  cli_wand->location="in \"%s\" at line %u,column %u";
  if ( LocaleCompare("-", filename) == 0 )
    cli_wand->filename="stdin";
  else
    cli_wand->filename=filename;

  /* Process Options from Script */
  option = arg1 = arg2 = (char*) NULL;
DisableMSCWarning(4127)
  while (1) {
RestoreMSCWarning

    { MagickBooleanType status = GetScriptToken(token_info);
      cli_wand->line=token_info->token_line;
      cli_wand->column=token_info->token_column;
      if (status == MagickFalse)
        break; /* error or end of options */
    }

    do { /* use break to loop to exception handler and loop */

      /* save option details */
      CloneString(&option,token_info->token);

      /* get option, its argument count, and option type */
      cli_wand->command = GetCommandOptionInfo(option);
      count=cli_wand->command->type;
      option_type=(CommandOptionFlags) cli_wand->command->flags;
#if 0
      (void) FormatLocaleFile(stderr, "Script: %u,%u: \"%s\" matched \"%s\"\n",
          cli_wand->line, cli_wand->line, option, cli_wand->command->mnemonic );
#endif

      /* handle a undefined option - image read - always for "magick-script" */
      if ( option_type == UndefinedOptionFlag ||
           (option_type & NonMagickOptionFlag) != 0 ) {
#if MagickCommandDebug >= 3
        (void) FormatLocaleFile(stderr, "Script %u,%u Non-Option: \"%s\"\n",
                    cli_wand->line, cli_wand->line, option);
#endif
        if (IsCommandOption(option) == MagickFalse) {
          /* non-option -- treat as a image read */
          cli_wand->command=(const OptionInfo *) NULL;
          CLIOption(cli_wand,"-read",option);
          break; /* next option */
        }
        CLIWandException(OptionFatalError,"UnrecognizedOption",option);
        break; /* next option */
      }

      if ( count >= 1 ) {
        if (GetScriptToken(token_info) == MagickFalse)
          CLIWandException(OptionFatalError,"MissingArgument",option);
        CloneString(&arg1,token_info->token);
      }
      else
        CloneString(&arg1,(char *) NULL);

      if ( count >= 2 ) {
        if (GetScriptToken(token_info) == MagickFalse)
          CLIWandExceptionBreak(OptionFatalError,"MissingArgument",option);
        CloneString(&arg2,token_info->token);
      }
      else
        CloneString(&arg2,(char *) NULL);

      /*
        Process Options
      */
#if MagickCommandDebug >= 3
      (void) FormatLocaleFile(stderr,
        "Script %u,%u Option: \"%s\"  Count: %d  Flags: %04x  Args: \"%s\" \"%s\"\n",
            cli_wand->line,cli_wand->line,option,count,option_type,arg1,arg2);
#endif
      /* Hard Deprecated Options, no code to execute - error */
      if ( (option_type & DeprecateOptionFlag) != 0 ) {
        CLIWandException(OptionError,"DeprecatedOptionNoCode",option);
        break; /* next option */
      }

      /* MagickCommandGenesis() options have no place in a magick script */
      if ( (option_type & GenesisOptionFlag) != 0 ) {
        CLIWandException(OptionError,"InvalidUseOfOption",option);
        break; /* next option */
      }

      /* handle any special 'script' options */
      if ( (option_type & SpecialOptionFlag) != 0 ) {
        if ( LocaleCompare(option,"-exit") == 0 ) {
          goto loop_exit; /* break out of loop - return from script */
        }
        if ( LocaleCompare(option,"-script") == 0 ) {
          /* FUTURE: call new script from this script - error for now */
          CLIWandException(OptionError,"InvalidUseOfOption",option);
          break; /* next option */
        }
        /* FUTURE: handle special script-argument options here */
        /* handle any other special operators now */
        CLIWandException(OptionError,"InvalidUseOfOption",option);
        break; /* next option */
      }

      /* Process non-specific Option */
      CLIOption(cli_wand, option, arg1, arg2);
      (void) fflush(stdout);
      (void) fflush(stderr);

DisableMSCWarning(4127)
    } while (0); /* break block to next option */
RestoreMSCWarning

#if MagickCommandDebug >= 5
    fprintf(stderr, "Script Image Count = %ld\n",
         GetImageListLength(cli_wand->wand.images) );
#endif
    if (CLICatchException(cli_wand, MagickFalse) != MagickFalse)
      break;  /* exit loop */
  }

  /*
     Loop exit - check for some tokenization error
  */
loop_exit:
#if MagickCommandDebug >= 3
  (void) FormatLocaleFile(stderr, "Script End: %d\n", token_info->status);
#endif
  switch( token_info->status ) {
    case TokenStatusOK:
    case TokenStatusEOF:
      if (cli_wand->image_list_stack != (CLIStack *) NULL)
        CLIWandException(OptionError,"UnbalancedParenthesis", "(eof)");
      else if (cli_wand->image_info_stack != (CLIStack *) NULL)
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
  (void) fflush(stdout);
  (void) fflush(stderr);
  if (cli_wand->wand.debug != MagickFalse)
    (void) LogMagickEvent(CommandEvent,GetMagickModule(),
         "Script End \"%s\"", filename);

  /* Clean up */
  token_info = DestroyScriptTokenInfo(token_info);

  CloneString(&option,(char *) NULL);
  CloneString(&arg1,(char *) NULL);
  CloneString(&arg2,(char *) NULL);

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
%  command line argument array. The 'index' defines where in the array we
%  should begin processing
%
%  The 'process_flags' can be used to control and limit option processing.
%  For example, to only process one option, or how unknown and special options
%  are to be handled, and if the last argument in array is to be regarded as a
%  final image write argument (filename or special coder).
%
%  The format of the ProcessCommandOptions method is:
%
%    int ProcessCommandOptions(MagickCLI *cli_wand,int argc,char **argv,
%      int index)
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
% is really only relevant if process_flags contains a ProcessOneOptionOnly
% flag.
%
*/
WandExport int ProcessCommandOptions(MagickCLI *cli_wand,int argc,char **argv,
  int index)
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
  assert(argv != (char **) NULL);
  assert(argv[index] != (char *) NULL);
  assert(argv[argc-1] != (char *) NULL);
  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == MagickWandSignature);

  /* define the error location string for use in exceptions
     order of location format escapes: filename, line, column */
  cli_wand->location="at %s arg %u";
  cli_wand->filename="CLI";
  cli_wand->line=(size_t) index;  /* note first argument we will process */

  if (cli_wand->wand.debug != MagickFalse)
    (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
         "- Starting (\"%s\")", argv[index]);

  end = argc;
  if ( (cli_wand->process_flags & ProcessImplicitWrite) != 0 )
    end--; /* the last argument is an implied write, do not process directly */

  for (i=index; i < end; i += count +1) {
    /* Finished processing one option? */
    if ( (cli_wand->process_flags & ProcessOneOptionOnly) != 0 && i != index )
      return(i);

    do { /* use break to loop to exception handler and loop */

      option=argv[i];
      cli_wand->line=(size_t) i;  /* note the argument for this option */

      /* get option, its argument count, and option type */
      cli_wand->command = GetCommandOptionInfo(argv[i]);
      count=cli_wand->command->type;
      option_type=(CommandOptionFlags) cli_wand->command->flags;
#if 0
      (void) FormatLocaleFile(stderr, "CLI %d: \"%s\" matched \"%s\"\n",
            i, argv[i], cli_wand->command->mnemonic );
#endif

      if ( option_type == UndefinedOptionFlag ||
           (option_type & NonMagickOptionFlag) != 0 ) {
#if MagickCommandDebug >= 3
        (void) FormatLocaleFile(stderr, "CLI arg %d Non-Option: \"%s\"\n",
             i, option);
#endif
        if (IsCommandOption(option) == MagickFalse) {
          if ( (cli_wand->process_flags & ProcessImplicitRead) != 0 ) {
            /* non-option -- treat as a image read */
            cli_wand->command=(const OptionInfo *) NULL;
            CLIOption(cli_wand,"-read",option);
            break; /* next option */
          }
        }
        CLIWandException(OptionFatalError,"UnrecognizedOption",option);
        break; /* next option */
      }

      if ( ((option_type & SpecialOptionFlag) != 0 ) &&
           ((cli_wand->process_flags & ProcessScriptOption) != 0) &&
           (LocaleCompare(option,"-script") == 0) ) {
        /* Call Script from CLI, with a filename as a zeroth argument.
           NOTE: -script may need to use the 'implicit write filename' argument
           so it must be handled specially to prevent a 'missing argument' error.
        */
        if ( (i+count) >= argc )
          CLIWandException(OptionFatalError,"MissingArgument",option);
        ProcessScriptOptions(cli_wand,argv[i+1],argc,argv,i+count);
        return(argc);  /* Script does not return to CLI -- Yet */
                       /* FUTURE: when it does, their may be no write arg! */
      }

      if ((i+count) >= end ) {
        CLIWandException(OptionFatalError,"MissingArgument",option);
        if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
          return(end);
        break; /* next option - not that their is any! */
      }

      arg1 = ( count >= 1 ) ? argv[i+1] : (char *) NULL;
      arg2 = ( count >= 2 ) ? argv[i+2] : (char *) NULL;

      /*
        Process Known Options
      */
#if MagickCommandDebug >= 3
      (void) FormatLocaleFile(stderr,
        "CLI arg %u Option: \"%s\"  Count: %d  Flags: %04x  Args: \"%s\" \"%s\"\n",
            i,option,count,option_type,arg1,arg2);
#endif
      /* ignore 'genesis options' in command line args */
      if ( (option_type & GenesisOptionFlag) != 0 )
        break; /* next option */

      /* Handle any special options for CLI (-script handled above) */
      if ( (option_type & SpecialOptionFlag) != 0 ) {
        if ( (cli_wand->process_flags & ProcessExitOption) != 0
             && LocaleCompare(option,"-exit") == 0 )
          return(i+count);
        break; /* next option */
      }

      /* Process standard image option */
      CLIOption(cli_wand, option, arg1, arg2);

DisableMSCWarning(4127)
    } while (0); /* break block to next option */
RestoreMSCWarning

#if MagickCommandDebug >= 5
    (void) FormatLocaleFile(stderr, "CLI-post Image Count = %ld\n",
         (long) GetImageListLength(cli_wand->wand.images) );
#endif
    if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
      return(i+count);
  }
  assert(i==end);

  if ( (cli_wand->process_flags & ProcessImplicitWrite) == 0 )
    return(end); /* no implied write -- just return to caller */

  assert(end==argc-1); /* end should not include last argument */

  /*
     Implicit Write of images to final CLI argument
  */
  option=argv[i];
  cli_wand->line=(size_t) i;

  /* check that stacks are empty - or cause exception */
  if (cli_wand->image_list_stack != (CLIStack *) NULL)
    CLIWandException(OptionError,"UnbalancedParenthesis", "(end of cli)");
  else if (cli_wand->image_info_stack != (CLIStack *) NULL)
    CLIWandException(OptionError,"UnbalancedBraces", "(end of cli)");
  if ( CLICatchException(cli_wand, MagickFalse) != MagickFalse )
    return(argc);

#if MagickCommandDebug >= 3
  (void) FormatLocaleFile(stderr,"CLI arg %d Write File: \"%s\"\n",i,option);
#endif

  /* Valid 'do no write' replacement option (instead of "null:") */
  if (LocaleCompare(option,"-exit") == 0 )
    return(argc);  /* just exit, no image write */

  /* If filename looks like an option,
     Or the common 'end of line' error of a single space.
     -- produce an error */
  if (IsCommandOption(option) != MagickFalse ||
      (option[0] == ' ' && option[1] == '\0') ) {
    CLIWandException(OptionError,"MissingOutputFilename",option);
    return(argc);
  }

  cli_wand->command=(const OptionInfo *) NULL;
  CLIOption(cli_wand,"-write",option);
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
%  This is essentially interface function between the MagickCore library
%  initialization function MagickCommandGenesis(), and the option MagickCLI
%  processing functions  ProcessCommandOptions()  or  ProcessScriptOptions()
%
%  The format of the MagickImageCommand method is:
%
%      MagickBooleanType MagickImageCommand(ImageInfo *image_info,int argc,
%        char **argv,char **metadata,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the starting image_info structure
%      (for compatibility with MagickCommandGenisis())
%
%    o argc: the number of elements in the argument vector.
%
%    o argv: A text array containing the command line arguments.
%
%    o metadata: any metadata (for VBS) is returned here.
%      (for compatibility with MagickCommandGenisis())
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
       "Usage: %s [ {option} | {image} ... ] {output_image}\n",name);
    (void) FormatLocaleFile(stdout,
       "       %s -help | -version | -usage | -list {option}\n\n",name);
    return;
  }
  else if (len>=6 && LocaleCompare("script",name+len-6) == 0) {
    /* magick-script usage */
    (void) FormatLocaleFile(stdout,
      "Usage: %s {filename} [ {script_args} ... ]\n",name);
  }
  else {
    /* magick usage */
    (void) FormatLocaleFile(stdout,
       "Usage: %s tool [ {option} | {image} ... ] {output_image}\n",name);
    (void) FormatLocaleFile(stdout,
       "Usage: %s [ {option} | {image} ... ] {output_image}\n",name);
    (void) FormatLocaleFile(stdout,
       "       %s [ {option} | {image} ... ] -script {filename} [ {script_args} ...]\n",
       name);
  }
  (void) FormatLocaleFile(stdout,
    "       %s -help | -version | -usage | -list {option}\n\n",name);

  if (verbose == MagickFalse)
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
   Concatenate given file arguments to the given output argument.
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

  MagickBooleanType
    status;

  int
    c;

  ssize_t
    i;

  if (ExpandFilenames(&argc,&argv) == MagickFalse)
    ThrowFileException(exception,ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  output=fopen_utf8(argv[argc-1],"wb");
  if (output == (FILE *) NULL)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        argv[argc-1]);
      return(MagickFalse);
    }
  status=MagickTrue;
  for (i=2; i < (ssize_t) (argc-1); i++)
  {
    input=fopen_utf8(argv[i],"rb");
    if (input == (FILE *) NULL)
      {
        ThrowFileException(exception,FileOpenError,"UnableToOpenFile",argv[i]);
        continue;
      }
    for (c=fgetc(input); c != EOF; c=fgetc(input))
      if (fputc((char) c,output) != c)
        status=MagickFalse;
    (void) fclose(input);
    (void) remove_utf8(argv[i]);
  }
  (void) fclose(output);
  return(status);
}

WandExport MagickBooleanType MagickImageCommand(ImageInfo *image_info,int argc,
  char **argv,char **metadata,ExceptionInfo *exception)
{
  MagickCLI
    *cli_wand;

  size_t
    len;

  assert(image_info != (ImageInfo *) NULL);

  /* For specific OS command line requirements */
  ReadCommandlLine(argc,&argv);

  /* Initialize special "CLI Wand" to hold images and settings (empty) */
  cli_wand=AcquireMagickCLI(image_info,exception);
  cli_wand->location="Initializing";
  cli_wand->filename=argv[0];
  cli_wand->line=1;

  if (cli_wand->wand.debug != MagickFalse)
    (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
         "\"%s\"",argv[0]);


  GetPathComponent(argv[0],TailPath,cli_wand->wand.name);
  SetClientName(cli_wand->wand.name);
  ConcatenateMagickString(cli_wand->wand.name,"-CLI",MagickPathExtent);

  len=strlen(argv[0]);  /* precaution */

  /* "convert" command - give a "deprecated" warning" */
  if (len>=7 && LocaleCompare("convert",argv[0]+len-7) == 0) {
    cli_wand->process_flags = ConvertCommandOptionFlags;
    (void) FormatLocaleFile(stderr,"WARNING: %s\n",
         "The convert command is deprecated in IMv7, use \"magick\"\n");
  }

  /* Special Case:  If command name ends with "script" implied "-script" */
  if (len>=6 && LocaleCompare("script",argv[0]+len-6) == 0) {
    if (argc >= 2 && (  (*(argv[1]) != '-') || (strlen(argv[1]) == 1) )) {
      GetPathComponent(argv[1],TailPath,cli_wand->wand.name);
      ProcessScriptOptions(cli_wand,argv[1],argc,argv,2);
      goto Magick_Command_Cleanup;
    }
  }

  /* Special Case: Version Information and Abort */
  if (argc == 2) {
    if ((LocaleCompare("-version",argv[1]) == 0)   || /* GNU standard option */
        (LocaleCompare("--version",argv[1]) == 0) ) { /* just version */
      CLIOption(cli_wand, "-version");
      goto Magick_Command_Exit;
    }
    if ((LocaleCompare("-help",argv[1]) == 0)   || /* GNU standard option */
        (LocaleCompare("--help",argv[1]) == 0) ) { /* just a brief summary */
      if (cli_wand->wand.debug != MagickFalse)
        (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
            "- Special Option \"%s\"", argv[1]);
      MagickUsage(MagickFalse);
      goto Magick_Command_Exit;
    }
    if (LocaleCompare("-usage",argv[1]) == 0) {   /* both version & usage */
      if (cli_wand->wand.debug != MagickFalse)
        (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
            "- Special Option \"%s\"", argv[1]);
      CLIOption(cli_wand, "-version" );
      MagickUsage(MagickTrue);
      goto Magick_Command_Exit;
    }
  }

  /* not enough arguments -- including -help */
  if (argc < 3) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "InvalidArgument","%s",argc > 1 ? argv[argc-1] : "");
    MagickUsage(MagickFalse);
    goto Magick_Command_Exit;
  }

  /* Special "concatenate option (hidden) for delegate usage */
  if (LocaleCompare("-concatenate",argv[1]) == 0) {
    if (cli_wand->wand.debug != MagickFalse)
        (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
            "- Special Option \"%s\"", argv[1]);
    ConcatenateImages(argc,argv,exception);
    goto Magick_Command_Exit;
  }

  /* List Information and Abort */
  if (argc == 3 && LocaleCompare("-list",argv[1]) == 0) {
    CLIOption(cli_wand, argv[1], argv[2]);
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
    ProcessScriptOptions(cli_wand,argv[2],argc,argv,3);
  }
  else {
    /* Normal Command Line, assumes output file as last option */
    ProcessCommandOptions(cli_wand,argc,argv,1);
  }
  /* ------------- */

Magick_Command_Cleanup:
  cli_wand->location="Cleanup";
  cli_wand->filename=argv[0];
  if (cli_wand->wand.debug != MagickFalse)
    (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
         "\"%s\"",argv[0]);

  /* recover original image_info and clean up stacks
     FUTURE: "-reset stacks" option  */
  while ((cli_wand->image_list_stack != (CLIStack *) NULL) &&
         (cli_wand->image_list_stack->next != (CLIStack *) NULL))
    CLIOption(cli_wand,")");
  while ((cli_wand->image_info_stack != (CLIStack *) NULL) &&
         (cli_wand->image_info_stack->next != (CLIStack *) NULL))
    CLIOption(cli_wand,"}");

  /* assert we have recovered the original structures */
  assert(cli_wand->wand.image_info == image_info);
  assert(cli_wand->wand.exception == exception);

  /* Handle metadata for ImageMagickObject COM object for Windows VBS */
  if ((cli_wand->wand.images != (Image *) NULL) &&
      (metadata != (char **) NULL))
    {
      const char
        *format;

      char
        *text;
  
      format="%w,%h,%m";  /* Get this from image_info Option splaytree */
      text=InterpretImageProperties(image_info,cli_wand->wand.images,format,
        exception);
      if (text == (char *) NULL)
        ThrowMagickException(exception,GetMagickModule(),ResourceLimitError,
          "MemoryAllocationFailed","`%s'", GetExceptionMessage(errno));
      else
        {
          (void) ConcatenateString(&(*metadata),text);
          text=DestroyString(text);
        }
    }

Magick_Command_Exit:
  cli_wand->location="Exiting";
  cli_wand->filename=argv[0];
  if (cli_wand->wand.debug != MagickFalse)
    (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
         "\"%s\"",argv[0]);

  /* Destroy the special CLI Wand */
  cli_wand->wand.image_info = (ImageInfo *) NULL; /* not these */
  cli_wand->wand.exception = (ExceptionInfo *) NULL;
  cli_wand=DestroyMagickCLI(cli_wand);

  return(exception->severity < ErrorException ? MagickTrue : MagickFalse);
}

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
      (void) CloneString(&option,token_info->token);

      /* get option, its argument count, and option type */
      cli_wand->command = GetCommandOptionInfo(option);
      count=(int) cli_wand->command->type;
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
        (void) CloneString(&arg1,token_info->token);
      }
      else
        (void) CloneString(&arg1,(char *) NULL);

      if ( count >= 2 ) {
        if (GetScriptToken(token_info) == MagickFalse)
          CLIWandExceptionBreak(OptionFatalError,"MissingArgument",option);
        (void) CloneString(&arg2,token_info->token);
      }
      else
        (void) CloneString(&arg2,(char *) NULL);

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

  (void) CloneString(&option,(char *) NULL);
  (void) CloneString(&arg1,(char *) NULL);
  (void) CloneString(&arg2,(char *) NULL);

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
      count=(int) cli_wand->command->type;
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

static MagickBooleanType MagickCommandUsage(void)
{
  static const char
    channel_operators[] =
      "  -channel-fx expression\n"
      "                       exchange, extract, or transfer one or more image channels\n"
      "  -separate            separate an image channel into a grayscale image",
    miscellaneous[] =
      "  -debug events        display copious debugging information\n"
      "  -distribute-cache port\n"
      "                       distributed pixel cache spanning one or more servers\n"
      "  -help                print program options\n"
      "  -list type           print a list of supported option arguments\n"
      "  -log format          format of debugging information\n"
      "  -usage               print program usage\n"
      "  -version             print version information",
    operators[] =
      "  -adaptive-blur geometry\n"
      "                       adaptively blur pixels; decrease effect near edges\n"
      "  -adaptive-resize geometry\n"
      "                       adaptively resize image using 'mesh' interpolation\n"
      "  -adaptive-sharpen geometry\n"
      "                       adaptively sharpen pixels; increase effect near edges\n"
      "  -alpha option        on, activate, off, deactivate, set, opaque, copy\n"
      "                       transparent, extract, background, or shape\n"
      "  -annotate geometry text\n"
      "                       annotate the image with text\n"
      "  -auto-gamma          automagically adjust gamma level of image\n"
      "  -auto-level          automagically adjust color levels of image\n"
      "  -auto-orient         automagically orient (rotate) image\n"
      "  -auto-threshold method\n"
      "                       automatically perform image thresholding\n"
      "  -bench iterations    measure performance\n"
      "  -bilateral-blur geometry\n"
      "                       non-linear, edge-preserving, and noise-reducing smoothing filter\n"
      "  -black-threshold value\n"
      "                       force all pixels below the threshold into black\n"
      "  -blue-shift factor   simulate a scene at nighttime in the moonlight\n"
      "  -blur geometry       reduce image noise and reduce detail levels\n"
      "  -border geometry     surround image with a border of color\n"
      "  -bordercolor color   border color\n"
      "  -brightness-contrast geometry\n"
      "                       improve brightness / contrast of the image\n"
      "  -canny geometry      detect edges in the image\n"
      "  -cdl filename        color correct with a color decision list\n"
      "  -channel mask        set the image channel mask\n"
      "  -charcoal radius     simulate a charcoal drawing\n"
      "  -chop geometry       remove pixels from the image interior\n"
      "  -clahe geometry      contrast limited adaptive histogram equalization\n"
      "  -clamp               keep pixel values in range (0-QuantumRange)\n"
      "  -colorize value      colorize the image with the fill color\n"
      "  -color-matrix matrix apply color correction to the image\n"
      "  -colors value        preferred number of colors in the image\n"
      "  -connected-components connectivity\n"
      "                       connected-components uniquely labeled\n"
      "  -contrast            enhance or reduce the image contrast\n"
      "  -contrast-stretch geometry\n"
      "                       improve contrast by 'stretching' the intensity range\n"
      "  -convolve coefficients\n"
      "                       apply a convolution kernel to the image\n"
      "  -cycle amount        cycle the image colormap\n"
      "  -decipher filename   convert cipher pixels to plain pixels\n"
      "  -deskew threshold    straighten an image\n"
      "  -despeckle           reduce the speckles within an image\n"
      "  -distort method args\n"
      "                       distort images according to given method and args\n"
      "  -draw string         annotate the image with a graphic primitive\n"
      "  -edge radius         apply a filter to detect edges in the image\n"
      "  -encipher filename   convert plain pixels to cipher pixels\n"
      "  -emboss radius       emboss an image\n"
      "  -enhance             apply a digital filter to enhance a noisy image\n"
      "  -equalize            perform histogram equalization to an image\n"
      "  -evaluate operator value\n"
      "                       evaluate an arithmetic, relational, or logical expression\n"
      "  -extent geometry     set the image size\n"
      "  -extract geometry    extract area from image\n"
      "  -fft                 implements the discrete Fourier transform (DFT)\n"
      "  -flip                flip image vertically\n"
      "  -floodfill geometry color\n"
      "                       floodfill the image with color\n"
      "  -flop                flop image horizontally\n"
      "  -frame geometry      surround image with an ornamental border\n"
      "  -function name parameters\n"
      "                       apply function over image values\n"
      "  -gamma value         level of gamma correction\n"
      "  -gaussian-blur geometry\n"
      "                       reduce image noise and reduce detail levels\n"
      "  -geometry geometry   preferred size or location of the image\n"
      "  -grayscale method    convert image to grayscale\n"
      "  -hough-lines geometry\n"
      "                       identify lines in the image\n"
      "  -identify            identify the format and characteristics of the image\n"
      "  -ift                 implements the inverse discrete Fourier transform (DFT)\n"
      "  -implode amount      implode image pixels about the center\n"
      "  -integral            calculate the sum of values (pixel values) in the image\n"
      "  -interpolative-resize geometry\n"
      "                       resize image using interpolation\n"
      "  -kmeans geometry     K means color reduction\n"
      "  -kuwahara geometry   edge preserving noise reduction filter\n"
      "  -lat geometry        local adaptive thresholding\n"
      "  -level value         adjust the level of image contrast\n"
      "  -level-colors color,color\n"
      "                       level image with the given colors\n"
      "  -linear-stretch geometry\n"
      "                       improve contrast by 'stretching with saturation'\n"
      "  -liquid-rescale geometry\n"
      "                       rescale image with seam-carving\n"
      "  -local-contrast geometry\n"
      "                       enhance local contrast\n"
      "  -mean-shift geometry delineate arbitrarily shaped clusters in the image\n"
      "  -median geometry     apply a median filter to the image\n"
      "  -mode geometry       make each pixel the 'predominant color' of the\n"
      "                       neighborhood\n"
      "  -modulate value      vary the brightness, saturation, and hue\n"
      "  -monochrome          transform image to black and white\n"
      "  -morphology method kernel\n"
      "                       apply a morphology method to the image\n"
      "  -motion-blur geometry\n"
      "                       simulate motion blur\n"
      "  -negate              replace every pixel with its complementary color \n"
      "  -noise geometry      add or reduce noise in an image\n"
      "  -normalize           transform image to span the full range of colors\n"
      "  -opaque color        change this color to the fill color\n"
      "  -ordered-dither NxN\n"
      "                       add a noise pattern to the image with specific\n"
      "                       amplitudes\n"
      "  -paint radius        simulate an oil painting\n"
      "  -perceptible epsilon\n"
      "                       pixel value less than |epsilon| become epsilon or\n"
      "                       -epsilon\n"
      "  -polaroid angle      simulate a Polaroid picture\n"
      "  -posterize levels    reduce the image to a limited number of color levels\n"
      "  -profile filename    add, delete, or apply an image profile\n"
      "  -quantize colorspace reduce colors in this colorspace\n"
      "  -raise value         lighten/darken image edges to create a 3-D effect\n"
      "  -random-threshold low,high\n"
      "                       random threshold the image\n"
      "  -range-threshold values\n"
      "                       perform either hard or soft thresholding within some range of values in an image\n"
      "  -region geometry     apply options to a portion of the image\n"
      "  -render              render vector graphics\n"
      "  -resample geometry   change the resolution of an image\n"
      "  -reshape geometry    reshape the image\n"
      "  -resize geometry     resize the image\n"
      "  -roll geometry       roll an image vertically or horizontally\n"
      "  -rotate degrees      apply Paeth rotation to the image\n"
      "  -rotational-blur angle\n"
      "                       rotational blur the image\n"
      "  -sample geometry     scale image with pixel sampling\n"
      "  -scale geometry      scale the image\n"
      "  -segment values      segment an image\n"
      "  -selective-blur geometry\n"
      "                       selectively blur pixels within a contrast threshold\n"
      "  -sepia-tone threshold\n"
      "                       simulate a sepia-toned photo\n"
      "  -set property value  set an image property\n"
      "  -shade degrees       shade the image using a distant light source\n"
      "  -shadow geometry     simulate an image shadow\n"
      "  -sharpen geometry    sharpen the image\n"
      "  -shave geometry      shave pixels from the image edges\n"
      "  -shear geometry      slide one edge of the image along the X or Y axis\n"
      "  -sigmoidal-contrast geometry\n"
      "                       increase the contrast without saturating highlights or\n"
      "                       shadows\n"
      "  -sketch geometry     simulate a pencil sketch\n"
      "  -solarize threshold  negate all pixels above the threshold level\n"
      "  -sort-pixels         sort each scanline in ascending order of intensity\n"
      "  -sparse-color method args\n"
      "                       fill in a image based on a few color points\n"
      "  -splice geometry     splice the background color into the image\n"
      "  -spread radius       displace image pixels by a random amount\n"
      "  -statistic type geometry\n"
      "                       replace each pixel with corresponding statistic from the\n"
      "                       neighborhood\n"
      "  -strip               strip image of all profiles and comments\n"
      "  -swirl degrees       swirl image pixels about the center\n"
      "  -threshold value     threshold the image\n"
      "  -thumbnail geometry  create a thumbnail of the image\n"
      "  -tile filename       tile image when filling a graphic primitive\n"
      "  -tint value          tint the image with the fill color\n"
      "  -transform           affine transform image\n"
      "  -transparent color   make this color transparent within the image\n"
      "  -transpose           flip image vertically and rotate 90 degrees\n"
      "  -transverse          flop image horizontally and rotate 270 degrees\n"
      "  -trim                trim image edges\n"
      "  -type type           image type\n"
      "  -unique-colors       discard all but one of any pixel color\n"
      "  -unsharp geometry    sharpen the image\n"
      "  -vignette geometry   soften the edges of the image in vignette style\n"
      "  -wave geometry       alter an image along a sine wave\n"
      "  -wavelet-denoise threshold\n"
      "                       removes noise from the image using a wavelet transform\n"
      "  -white-balance       automagically adjust white balance of image\n"
      "  -white-threshold value\n"
      "                       force all pixels above the threshold into white",
    sequence_operators[] =
      "  -append              append an image sequence\n"
      "  -clut                apply a color lookup table to the image\n"
      "  -coalesce            merge a sequence of images\n"
      "  -combine             combine a sequence of images\n"
      "  -compare             mathematically and visually annotate the difference between an image and its reconstruction\n"
      "  -complex operator    perform complex mathematics on an image sequence\n"
      "  -composite           composite image\n"
      "  -copy geometry offset\n"
      "                       copy pixels from one area of an image to another\n"
      "  -crop geometry       cut out a rectangular region of the image\n"
      "  -deconstruct         break down an image sequence into constituent parts\n"
      "  -evaluate-sequence operator\n"
      "                       evaluate an arithmetic, relational, or logical expression\n"
      "  -flatten             flatten a sequence of images\n"
      "  -fx expression       apply mathematical expression to an image channel(s)\n"
      "  -hald-clut           apply a Hald color lookup table to the image\n"
      "  -layers method       optimize, merge, or compare image layers\n"
      "  -morph value         morph an image sequence\n"
      "  -mosaic              create a mosaic from an image sequence\n"
      "  -poly terms          build a polynomial from the image sequence and the corresponding\n"
      "                       terms (coefficients and degree pairs).\n"
      "  -print string        interpret string and print to console\n"
      "  -process arguments   process the image with a custom image filter\n"
      "  -smush geometry      smush an image sequence together\n"
      "  -write filename      write images to this file",
    settings[] =
      "  -adjoin              join images into a single multi-image file\n"
      "  -affine matrix       affine transform matrix\n"
      "  -alpha option        activate, deactivate, reset, or set the alpha channel\n"
      "  -antialias           remove pixel-aliasing\n"
      "  -authenticate password\n"
      "                       decipher image with this password\n"
      "  -attenuate value     lessen (or intensify) when adding noise to an image\n"
      "  -background color    background color\n"
      "  -bias value          add bias when convolving an image\n"
      "  -black-point-compensation\n"
      "                       use black point compensation\n"
      "  -blue-primary point  chromaticity blue primary point\n"
      "  -bordercolor color   border color\n"
      "  -caption string      assign a caption to an image\n"
      "  -clip                clip along the first path from the 8BIM profile\n"
      "  -clip-mask filename  associate a clip mask with the image\n"
      "  -clip-path id        clip along a named path from the 8BIM profile\n"
      "  -colorspace type     alternate image colorspace\n"
      "  -comment string      annotate image with comment\n"
      "  -compose operator    set image composite operator\n"
      "  -compress type       type of pixel compression when writing the image\n"
      "  -define format:option\n"
      "                       define one or more image format options\n"
      "  -delay value         display the next image after pausing\n"
      "  -density geometry    horizontal and vertical density of the image\n"
      "  -depth value         image depth\n"
      "  -direction type      render text right-to-left or left-to-right\n"
      "  -display server      get image or font from this X server\n"
      "  -dispose method      layer disposal method\n"
      "  -dither method       apply error diffusion to image\n"
      "  -encoding type       text encoding type\n"
      "  -endian type         endianness (MSB or LSB) of the image\n"
      "  -family name         render text with this font family\n"
      "  -features distance   analyze image features (e.g. contrast, correlation)\n"
      "  -fill color          color to use when filling a graphic primitive\n"
      "  -filter type         use this filter when resizing an image\n"
      "  -font name           render text with this font\n"
      "  -format \"string\"     output formatted image characteristics\n"
      "  -fuzz distance       colors within this distance are considered equal\n"
      "  -gravity type        horizontal and vertical text placement\n"
      "  -green-primary point chromaticity green primary point\n"
      "  -illuminant type     reference illuminant\n"
      "  -intensity method    method to generate an intensity value from a pixel\n"
      "  -intent type         type of rendering intent when managing the image color\n"
      "  -interlace type      type of image interlacing scheme\n"
      "  -interline-spacing value\n"
      "                       set the space between two text lines\n"
      "  -interpolate method  pixel color interpolation method\n"
      "  -interword-spacing value\n"
      "                       set the space between two words\n"
      "  -kerning value       set the space between two letters\n"
      "  -label string        assign a label to an image\n"
      "  -limit type value    pixel cache resource limit\n"
      "  -loop iterations     add Netscape loop extension to your GIF animation\n"
      "  -matte               store matte channel if the image has one\n"
      "  -mattecolor color    frame color\n"
      "  -moments             report image moments\n"
      "  -monitor             monitor progress\n"
      "  -orient type         image orientation\n"
      "  -page geometry       size and location of an image canvas (setting)\n"
      "  -ping                efficiently determine image attributes\n"
      "  -pointsize value     font point size\n"
      "  -precision value     maximum number of significant digits to print\n"
      "  -preview type        image preview type\n"
      "  -quality value       JPEG/MIFF/PNG compression level\n"
      "  -quiet               suppress all warning messages\n"
      "  -read-mask filename  associate a read mask with the image\n"
      "  -red-primary point   chromaticity red primary point\n"
      "  -regard-warnings     pay attention to warning messages\n"
      "  -remap filename      transform image colors to match this set of colors\n"
      "  -repage geometry     size and location of an image canvas\n"
      "  -respect-parentheses settings remain in effect until parenthesis boundary\n"
      "  -sampling-factor geometry\n"
      "                       horizontal and vertical sampling factor\n"
      "  -scene value         image scene number\n"
      "  -seed value          seed a new sequence of pseudo-random numbers\n"
      "  -size geometry       width and height of image\n"
      "  -stretch type        render text with this font stretch\n"
      "  -stroke color        graphic primitive stroke color\n"
      "  -strokewidth value   graphic primitive stroke width\n"
      "  -style type          render text with this font style\n"
      "  -support factor      resize support: > 1.0 is blurry, < 1.0 is sharp\n"
      "  -synchronize         synchronize image to storage device\n"
      "  -taint               declare the image as modified\n"
      "  -texture filename    name of texture to tile onto the image background\n"
      "  -tile-offset geometry\n"
      "                       tile offset\n"
      "  -treedepth value     color tree depth\n"
      "  -transparent-color color\n"
      "                       transparent color\n"
      "  -undercolor color    annotation bounding box color\n"
      "  -units type          the units of image resolution\n"
      "  -verbose             print detailed information about the image\n"
      "  -view                FlashPix viewing transforms\n"
      "  -virtual-pixel method\n"
      "                       virtual pixel access method\n"
      "  -weight type         render text with this font weight\n"
      "  -white-point point   chromaticity white point\n"
      "  -write-mask filename associate a write mask with the image"
      "  -word-break type     sets whether line breaks appear wherever the text would otherwise overflow",
    stack_operators[] =
      "  -clone indexes       clone an image\n"
      "  -delete indexes      delete the image from the image sequence\n"
      "  -duplicate count,indexes\n"
      "                       duplicate an image one or more times\n"
      "  -insert index        insert last image into the image sequence\n"
      "  -reverse             reverse image sequence\n"
      "  -swap indexes        swap two images in the image sequence";

  ListMagickVersion(stdout);
  (void) FormatLocaleFile(stdout,
    "Usage: %s tool [ {option} | {image} ... ] {output_image}\n",
    GetClientName());
  (void) FormatLocaleFile(stdout,
    "Usage: %s [ {option} | {image} ... ] {output_image}\n",GetClientName());
  (void) FormatLocaleFile(stdout,
    "       %s [ {option} | {image} ... ] -script {filename} [ {script_args} ...]\n",
    GetClientName());
  (void) FormatLocaleFile(stdout,"\nImage Settings:\n");
  (void) FormatLocaleFile(stdout,"%s\n",settings);
  (void) FormatLocaleFile(stdout,"\nImage Operators:\n");
  (void) FormatLocaleFile(stdout,"%s\n",operators);
  (void) FormatLocaleFile(stdout,"\nImage Channel Operators:\n");
  (void) FormatLocaleFile(stdout,"%s\n",channel_operators);
  (void) FormatLocaleFile(stdout,"\nImage Sequence Operators:\n");
  (void) FormatLocaleFile(stdout,"%s\n",sequence_operators);
  (void) FormatLocaleFile(stdout,"\nImage Stack Operators:\n");
  (void) FormatLocaleFile(stdout,"%s\n",stack_operators);
  (void) FormatLocaleFile(stdout,"\nMiscellaneous Options:\n");
  (void) FormatLocaleFile(stdout,"%s\n",miscellaneous);
  (void) FormatLocaleFile(stdout,
    "\nBy default, the image format of 'file' is determined by its magic\n");
  (void) FormatLocaleFile(stdout,
    "number.  To specify a particular image format, precede the filename\n");
  (void) FormatLocaleFile(stdout,
    "with an image format name and a colon (i.e. ps:image) or specify the\n");
  (void) FormatLocaleFile(stdout,
    "image type as the filename suffix (i.e. image.ps).  Specify 'file' as\n");
  (void) FormatLocaleFile(stdout,"'-' for standard input or output.\n");
  return(MagickTrue);
}

static void MagickUsage(MagickBooleanType verbose)
{
  const char
    *name;

  size_t
    len;

  name=GetClientName();
  len=strlen(name);

  if (verbose == MagickFalse)
    {
      (void) MagickCommandUsage();
      return;
    }

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
  (void) SetClientName(cli_wand->wand.name);
  (void) ConcatenateMagickString(cli_wand->wand.name,"-CLI",MagickPathExtent);

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
    (void) ConcatenateImages(argc,argv,exception);
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
        (void) ThrowMagickException(exception,GetMagickModule(),ResourceLimitError,
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

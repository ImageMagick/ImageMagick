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
#include "MagickWand/magick-cli-private.h"
#include "MagickWand/magick-cli.h"
#include "MagickWand/operation.h"
#include "MagickCore/memory_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"

/* verbose debugging,
      1 - option type
      2 - source of option
      3 - mnemonic lookup  */
#define MagickCommandDebug 0
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S c r i p t T o k e n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetScriptToken() is fairly general, finite state token parser. That will
%  divide a input file stream into tokens, in a way that is as close to a
%  UNIX shell, as is feasable.  Only shell variable, and command
%  substitutions will not be performed.
%
%  Tokens are white space separated, and may be quoted, or even partially
%  quoted by either single or double quotes, or the use of backslashes,
%  or any mix of the three.
%
%  For example:    This\ is' a 'single" token"
%
%  A token is returned immediatally the end of token is found. That is as soon
%  as a ending white-space or EOF condition was determined.  That is to say
%  the file stream is parsed purely character-by-character, regardless any
%  buffering constraints set by the system.
%
%  The function will return 'MagickTrue' if a valid token was found, while
%  the token status will be set accordingally to 'OK' or 'EOF', according to
%  the cause of the end of token.  The token may be an empty string if the
%  input was a quoted empty string.  Other error conditions return a value of
%  MagickFalse, indicating any token found was incomplete due to the error
%  condition.
%
%  Single quotes will preserve all characters including backslashes. Double
%  quotes will also preserve backslashes unless escaping a double quote,
%  or another backslashes.  Other shell meta-characters are not treated as
%  special by this tokenizer.
%
%  For example Quoting the quote chars:
%              \'  "'"       \"  '"'  "\""      \\  '\'  "\\"
%
%  Outside quotes, backslash characters will make spaces, tabs and quotes part
%  of a token returned. However a backslash at the end of a line (and outside
%  quotes) will cause the newline to be completely ignored (as per the shell
%  line continuation).
%
%  Comments start with a '#' character at the start of a new token, will be
%  completely ignored upto the end of line, regardless of any backslash at the
%  end of the line.  You can escape a comment '#', using quotes or backlsashes
%  just as you can in a shell.
%
%  The format of the MagickImageCommand method is:
%
%     MagickBooleanType GetScriptToken(ScriptTokenInfo *token_info)
%
%  A description of each parameter follows:
%
%    o token_info    pointer to a structure holding token details
%
*/

/* States of the parser */
#define IN_WHITE 0
#define IN_TOKEN 1
#define IN_QUOTE 2
#define IN_COMMENT 3

typedef enum {
  TokenStatusOK = 0,
  TokenStatusEOF,
  TokenStatusBadQuotes,
  TokenStatusTokenTooBig,
  TokenStatusBinary
} TokenStatus;

typedef struct
{
  FILE
    *stream;        /* the file stream we are reading from */

  char
    *token;         /* array of characters to holding details of he token */

  size_t
    length,         /* length of token char array */
    curr_line,      /* current location in script file */
    curr_column,
    token_line,     /* location of the start of this token */
    token_column;

  TokenStatus
    status;         /* Have we reached EOF? see Token Status */
} ScriptTokenInfo;

/* macro to read character from stream */
#define GetChar(c) \
{ \
   c=fgetc(token_info->stream); \
   token_info->curr_column++; \
   if ( c == '\n' ) \
     token_info->curr_line++, token_info->curr_column=0; \
}
/* macro to collect the token characters */
#define SaveChar(c) \
{ \
  if ((size_t) offset >= (token_info->length-1)) \
    { token_info->token[offset++]='\0'; \
      token_info->status=TokenStatusTokenTooBig; \
      return(MagickFalse); \
    } \
  token_info->token[offset++]=(char) (c); \
}

static MagickBooleanType GetScriptToken(ScriptTokenInfo *token_info)
{

  int
    quote,
    c;

  int
    state;

  ssize_t
    offset;

  /* EOF - no more tokens! */
  if (token_info->status != TokenStatusOK)
    {
      token_info->token[0]='\0';
      return(MagickFalse);
    }

  state=IN_WHITE;
  quote='\0';
  offset=0;
  while(1)
  {
    /* get character */
    GetChar(c);
    if (c == '\0' || c == EOF)
      break;

    /* hash comment handling */
    if ( state == IN_COMMENT )
      { if ( c == '\n' )
          state=IN_WHITE;
        continue;
      }
    if (c == '#' && state == IN_WHITE)
      state=IN_COMMENT;
    /* whitespace break character */
    if (strchr(" \n\r\t",c) != (char *)NULL)
      {
        switch (state)
        {
          case IN_TOKEN:
            token_info->token[offset]='\0';
            return(MagickTrue);
          case IN_QUOTE:
            SaveChar(c);
            break;
        }
        continue;
      }
    /* quote character */
    if (strchr("'\"",c) != (char *)NULL)
      {
        switch (state)
        {
          case IN_WHITE:
            token_info->token_line=token_info->curr_line;
            token_info->token_column=token_info->curr_column;
          case IN_TOKEN:
            state=IN_QUOTE;
            quote=c;
            break;
          case IN_QUOTE:
            if (c == quote)
              {
                state=IN_TOKEN;
                quote='\0';
              }
            else
              SaveChar(c);
            break;
        }
        continue;
      }
    /* escape char (preserve in quotes - unless escaping the same quote) */
    if (c == '\\')
      {
        if ( state==IN_QUOTE && quote == '\'' )
          {
            SaveChar('\\');
            continue;
          }
        GetChar(c);
        if (c == '\0' || c == EOF)
          {
            SaveChar('\\');
            break;
          }
        if (c == '\n')
          switch (state)
          {
            case IN_COMMENT:
              state=IN_WHITE;  /* end comment */
            case IN_WHITE:
            case IN_TOKEN:
              continue;   /* line continuation (outside quotes and comment) */
          }
        switch (state)
        {
          case IN_WHITE:
            token_info->token_line=token_info->curr_line;
            token_info->token_column=token_info->curr_column;
            state=IN_TOKEN;
            break;
          case IN_QUOTE:
            if (c != quote && c != '\\')
              SaveChar('\\');
            break;
        }
        SaveChar(c);
        continue;
      }
    /* ordinary character */
    switch (state)
    {
      case IN_WHITE:
        token_info->token_line=token_info->curr_line;
        token_info->token_column=token_info->curr_column;
        state=IN_TOKEN;
      case IN_TOKEN:
      case IN_QUOTE:
        SaveChar(c);
        break;
      case IN_COMMENT:
        break;
    }
  }
  /* stream has EOF or produced a fatal error */
  token_info->token[offset]='\0';
  token_info->status = TokenStatusEOF;
  if (state == IN_QUOTE)
    token_info->status = TokenStatusBadQuotes;
  if (c == '\0' )
    token_info->status = TokenStatusBinary;
  if (state == IN_TOKEN && token_info->status == TokenStatusEOF)
    return(MagickTrue);
  return(MagickFalse);
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
%  options is given as the zeroth argument of the argument array given.
%
%  A script not 'return' to the command line processing, nor can they
%  call (and return from) other scripts. At least not at this time.
%
%  However special script options may used to read and process the other
%  argument provided, typically those that followed a "-script" option on the
%  command line. These extra script arguments may be interpreted as being
%  images to read or write, settings (strings), or more options to be
%  processed.  How they are treated is up to the script being processed.
%
%  The format of the ProcessScriptOptions method is:
%
%    void ProcessScriptOptions(MagickCLI *cli_wand,int argc,char **argv)
%
%  A description of each parameter follows:
%
%    o cli_wand: the main CLI Wand to use.
%
%    o argc: the number of elements in the argument vector.
%
%    o argv: A text array containing the command line arguments.
%
*/
#define MagickExceptionScript(severity,tag,arg,line,col) \
  (void) ThrowMagickException(cli_wand->wand.exception,GetMagickModule(), \
       severity,tag, "'%s' : Line %u Column %u of script \"%s\"", \
       arg, line, col, cli_wand->wand.name);

WandExport void ProcessScriptOptions(MagickCLI *cli_wand,int argc,char **argv)
{
  char
    *option,
    *arg1,
    *arg2;

  ssize_t
    count;

  size_t
    option_line,       /* line and column of current option */
    option_column;

  CommandOptionFlags
    option_type;

  ScriptTokenInfo
    token_info;

  MagickBooleanType
    plus_alt_op,
    file_opened;

  assert(argc>0 && argv[argc-1] != (char *)NULL);
  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == WandSignature);
  if (cli_wand->wand.debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",cli_wand->wand.name);

  /* Initialize variables */
  /* FUTURE handle file opening for '-' 'fd:N' or script filename */
  file_opened=MagickFalse;
  if ( LocaleCompare(argv[0],"-") == 0 )
    {
      CopyMagickString(cli_wand->wand.name,"stdin",MaxTextExtent);
      token_info.stream=stdin;
      file_opened=MagickFalse;
    }
  else
    {
      GetPathComponent(argv[0],TailPath,cli_wand->wand.name);
      token_info.stream=fopen(argv[0], "r");
      file_opened=MagickTrue;
    }

  option = arg1 = arg2 = (char*)NULL;
  token_info.curr_line=1;
  token_info.curr_column=0;
  token_info.status=TokenStatusOK;
  token_info.length=MaxTextExtent;
  token_info.token=(char *) AcquireQuantumMemory(MaxTextExtent,sizeof(char));
  if (token_info.token == (char *) NULL)
    {
      if ( file_opened != MagickFalse )
        fclose(token_info.stream);
      MagickExceptionScript(ResourceLimitError,"MemoryAllocationFailed","",0,0);
      (void) ThrowMagickException(cli_wand->wand.exception,GetMagickModule(),
           ResourceLimitError,"MemoryAllocationFailed","script token buffer");
      return;
    }

  /* Process Options from Script */
  while (1)
    {
      /* Get a option */
      if( GetScriptToken(&token_info) == MagickFalse )
        break;

      /* option length sanity check */
      if( strlen(token_info.token) > 40 )
        { token_info.token[37] = '.';
          token_info.token[38] = '.';
          token_info.token[39] = '.';
          token_info.token[40] = '\0';
          MagickExceptionScript(OptionFatalError,"UnrecognizedOption",
               token_info.token,token_info.token_line,token_info.token_column);
          break;
        }

      /* save option details */
      CloneString(&option,token_info.token);
      option_line=token_info.token_line;
      option_column=token_info.token_column;

#if MagickCommandDebug >=2
      (void) FormatLocaleFile(stderr, "Script Option Token: %u,%u: \"%s\"\n",
               option_line, option_column, option );
#endif
      /* get option type and argument count */
      { const OptionInfo *option_info = GetCommandOptionInfo(option);
        count=option_info->type;
        option_type=option_info->flags;
#if MagickCommandDebug >= 3
        (void) FormatLocaleFile(stderr, "option \"%s\" matched \"%s\"\n",
             option, option_info->mnemonic );
#endif
      }

      /* handle a undefined option - image read? */
      if ( option_type == UndefinedOptionFlag ||
           (option_type & NonMagickOptionFlag) != 0 )
        {
#if MagickCommandDebug
          (void) FormatLocaleFile(stderr, "Script Non-Option: \"%s\"\n", option);
#endif
          if ( IsCommandOption(option) == MagickFalse)
            {
              /* non-option -- treat as a image read */
              CLISpecialOperator(cli_wand,"-read",option);
              count = 0;
            }
          else
            MagickExceptionScript(OptionFatalError,"UnrecognizedOption",
                 option,option_line,option_column);

          // FUTURE: '-regard_warning' causes IM to exit more prematurely!
          // Note pipelined options may like more control over this level
          if (cli_wand->wand.exception->severity > ErrorException)
            {
              if (cli_wand->wand.exception->severity > ErrorException)
                  //(regard_warnings != MagickFalse))
                break;                     /* FATAL - caller handles exception */
              CatchException(cli_wand->wand.exception); /* output warnings and clear!!! */
            }
          continue;
        }

      plus_alt_op = MagickFalse;
      if (*option=='+') plus_alt_op = MagickTrue;

      if ( count >= 1 )
        {
          if( GetScriptToken(&token_info) == MagickFalse )
            {
              MagickExceptionScript(OptionError,"MissingArgument",option,
                 option_line,option_column);
              break;
            }
          CloneString(&arg1,token_info.token);
        }
      else
        CloneString(&arg1,(*option!='+')?"true":(char *)NULL);

      if ( count >= 2 )
        {
          if( GetScriptToken(&token_info) == MagickFalse )
            {
              MagickExceptionScript(OptionError,"MissingArgument",option,
                 option_line,option_column);
              break;
            }
          CloneString(&arg2,token_info.token);
        }
      else
        CloneString(&arg2,(char *)NULL);

      /* handle script special options */
      //either continue processing command line
      // or making use of the command line options.
      //CLICommandOptions(cli_wand,count+1,argv, MagickScriptArgsFlags);

#if MagickCommandDebug
      (void) FormatLocaleFile(stderr,
          "Script Option: \"%s\" \tCount: %d  Flags: %04x  Args: \"%s\" \"%s\"\n",
          option,(int) count,option_type,arg1,arg2);
#endif

      /* Process non-script specific option from file */
      if ( (option_type & SpecialOptionFlag) != 0 )
        {
          if ( LocaleCompare(option,"-exit") == 0 )
            break;
          /* No "-script" from script at this time */
          CLISpecialOperator(cli_wand,option,arg1);
        }

      if ( (option_type & SettingOptionFlags) != 0 )
        {
          CLISettingOptionInfo(cli_wand, option+1, arg1);
          // FUTURE: Sync Specific Settings into Images
        }

      if ( (option_type & SimpleOperatorOptionFlag) != 0)
        CLISimpleOperatorImages(cli_wand, plus_alt_op, option+1, arg1, arg2);

      if ( (option_type & ListOperatorOptionFlag) != 0 )
        CLIListOperatorImages(cli_wand, plus_alt_op, option+1, arg1, arg2);

      // FUTURE: '-regard_warning' causes IM to exit more prematurely!
      // Note pipelined options may like more control over this level
      if (cli_wand->wand.exception->severity > ErrorException)
        {
          if (cli_wand->wand.exception->severity > ErrorException)
              //(regard_warnings != MagickFalse))
            break;                     /* FATAL - caller handles exception */
          CatchException(cli_wand->wand.exception); /* output warnings and clear!!! */
        }
    }
#if MagickCommandDebug
  (void) FormatLocaleFile(stderr, "Script End: %d\n", token_info.status);
#endif
  /* token sanity for error report */
  if( strlen(token_info.token) > 40 )
    { token_info.token[37] = '.';
      token_info.token[38] = '.';
      token_info.token[39] = '.';
      token_info.token[40] = '\0';
    }

   switch( token_info.status )
    {
      case TokenStatusBadQuotes:
        MagickExceptionScript(OptionFatalError,"ScriptUnbalancedQuotes",
             token_info.token,token_info.token_line,token_info.token_column);
        break;
      case TokenStatusTokenTooBig:
        MagickExceptionScript(OptionFatalError,"ScriptTokenTooBig",
             token_info.token,token_info.token_line,token_info.token_column);
        break;
      case TokenStatusBinary:
        MagickExceptionScript(OptionFatalError,"ScriptIsBinary","",
             token_info.curr_line,token_info.curr_column);
        break;
      case TokenStatusOK:
      case TokenStatusEOF:
        break;
    }

   /* Clean up */
   if ( file_opened != MagickFalse )
     fclose(token_info.stream);

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
%           int *index, ProcessOptionFlags process_flags )
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
*/
/* FUTURE: correctly identify option... CLI arg,  Script line,column  */
#define MagickExceptionContinue(severity,tag,arg,index) \
  (void) ThrowMagickException(cli_wand->wand.exception,GetMagickModule(),severity,tag, \
       "'%s' : CLI Arg #%d", arg, (int) index); \

#define MagickExceptionReturn(severity,tag,option,arg) \
{ \
  MagickExceptionContinue(severity,tag,option,arg); \
  return; \
}

WandExport void ProcessCommandOptions(MagickCLI *cli_wand,int argc,
     char **argv, ProcessOptionFlags process_flags )
{
  const char
    *option,
    *arg1,
    *arg2;

  MagickBooleanType
    plus_alt_op;

  ssize_t
    i,
    end,
    count;

  CommandOptionFlags
    option_type;

  assert(argc>0 && argv[argc-1] != (char *)NULL);
  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == WandSignature);
  if (cli_wand->wand.debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",cli_wand->wand.name);

  /*
    Parse command-line options.
  */
  end = argc;
  if ( ( process_flags & ProcessOutputFile ) != 0 )
    end--;
  for (i=0; i < end; i += count +1)
    {
#if MagickCommandDebug >= 2
      (void) FormatLocaleFile(stderr, "index= %d  option="%s\"\n", i, argv[i]);
#endif
      /* Finished processing one option? */
      if ( ( process_flags & ProcessOneOptionOnly ) != 0 && i != 0 )
        return;

      option=argv[i];
      plus_alt_op = MagickFalse;
      arg1=(char *)NULL;
      arg2=(char *)NULL;


      { const OptionInfo *option_info = GetCommandOptionInfo(argv[i]);
        count=option_info->type;
        option_type=option_info->flags;
#if MagickCommandDebug >= 3
        (void) FormatLocaleFile(stderr, "option \"%s\" matched \"%s\"\n",
             argv[i], option_info->mnemonic );
#endif
      }

      if ( option_type == UndefinedOptionFlag ||
           (option_type & NonMagickOptionFlag) != 0 )
        {
#if MagickCommandDebug
          (void) FormatLocaleFile(stderr, "CLI Non-Option: \"%s\"\n", option);
#endif
          if ( ( IsCommandOption(option) == MagickFalse ) &&
               ( (process_flags & ProcessNonOptionImageRead) != 0 ) )
            {
              /* non-option -- treat as a image read */
              CLISpecialOperator(cli_wand,"-read",option);
              count = 0;
            }
          else if ( (process_flags & ProcessUnknownOptionError) != 0 )
            MagickExceptionReturn(OptionFatalError,"UnrecognizedOption",
                 option,i);

          // FUTURE: '-regard_warning' causes IM to exit more prematurely!
          // Note pipelined options may like more control over this level
          if (cli_wand->wand.exception->severity > ErrorException)
            {
              if (cli_wand->wand.exception->severity > ErrorException)
                  //(regard_warnings != MagickFalse))
                break;                     /* FATAL - caller handles exception */
              CatchException(cli_wand->wand.exception); /* output warnings and clear!!! */
            }
          continue;
        }


      if ( (option_type & DeprecateOptionFlag) != 0 )
        MagickExceptionContinue(OptionWarning,"DeprecatedOption",option,i);
        /* continue processing option anyway */

      if ((i+count) >= end )
        MagickExceptionReturn(OptionError,"MissingArgument",option,i);

      if (*option=='+') plus_alt_op = MagickTrue;
      if (*option!='+') arg1 = "true";
      if ( count >= 1 ) arg1 = argv[i+1];
      if ( count >= 2 ) arg2 = argv[i+2];

#if MagickCommandDebug
      (void) FormatLocaleFile(stderr,
          "CLI Option: \"%s\" \tCount: %d  Flags: %04x  Args: \"%s\" \"%s\"\n",
          option,(int) count,option_type,arg1,arg2);
#endif

      if ( (option_type & SpecialOptionFlag) != 0 )
        {
          if ( ( process_flags & ProcessExitOption ) != 0
               && LocaleCompare(option,"-exit") == 0 )
            return;
          if ( ( process_flags & ProcessScriptOption ) != 0
               && LocaleCompare(option,"-script") == 0)
            {
              // Unbalanced Parenthesis if stack not empty
              // Call Script, with a filename as a zeroth argument
              ProcessScriptOptions(cli_wand,argc-(i+1),argv+(i+1));
              return;
            }
          CLISpecialOperator(cli_wand,option,arg1);
        }

      if ( (option_type & SettingOptionFlags) != 0 )
        {
          CLISettingOptionInfo(cli_wand, option+1, arg1);
          // FUTURE: Sync Specific Settings into Images
        }

      if ( (option_type & SimpleOperatorOptionFlag) != 0)
        CLISimpleOperatorImages(cli_wand, plus_alt_op, option+1, arg1, arg2);

      if ( (option_type & ListOperatorOptionFlag) != 0 )
        CLIListOperatorImages(cli_wand, plus_alt_op, option+1, arg1, arg2);

      // FUTURE: '-regard_warning' causes IM to exit more prematurely!
      // Note pipelined options may like more control over this level
      if (cli_wand->wand.exception->severity > ErrorException)
        {
          if (cli_wand->wand.exception->severity > ErrorException)
              //(regard_warnings != MagickFalse))
            return;                    /* FATAL - caller handles exception */
          CatchException(cli_wand->wand.exception); /* output warnings and clear!!! */
        }
    }

  if ( ( process_flags & ProcessOutputFile ) == 0 )
    return;
  assert(end==argc-1);

  /*
     Implicit Write of images to final CLI argument
  */
  option=argv[i];

#if MagickCommandDebug
  (void) FormatLocaleFile(stderr, "CLI Output: \"%s\"\n", option );
#endif

  // if stacks are not empty
  //  ThrowConvertException(OptionError,"UnbalancedParenthesis",option,i);

  /* This is a valid 'do no write' option for a CLI */
  if (LocaleCompare(option,"-exit") == 0 )
    return;  /* just exit, no image write */

  /* If there is an option -- produce an error */
  if (IsCommandOption(option) != MagickFalse)
    /* FUTURE: Better Error - Output Filename not Found */
    MagickExceptionReturn(OptionError,"MissingOutputFilename",option,i);

  /* If no images in MagickCLI */
  if ( cli_wand->wand.images == (Image *) NULL )
    {
      /* a "null:" output coder with no images is not an error! */
      if ( LocaleCompare(option,"null:") == 0 )
        return;
      MagickExceptionReturn(OptionError,"NoImagesForFinalWrite",option,i);
    }

#if 0
  WandListOperatorImages(cli_wand,MagickFalse,"write",option,(const char *)NULL);
#else
  (void) SyncImagesSettings(cli_wand->wand.image_info,cli_wand->wand.images,
       cli_wand->wand.exception);
  (void) WriteImages(cli_wand->wand.image_info,cli_wand->wand.images,option,
       cli_wand->wand.exception);
#endif
  return;
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
  if (output == (FILE *) NULL)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        argv[argc-1]);
      return(MagickFalse);
    }
  for (i=2; i < (ssize_t) (argc-1); i++)
  {
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
  if (argc == 2)
    {
      option=argv[1];
      if ((LocaleCompare("-version",option+1) == 0) ||
          (LocaleCompare("--version",option+1) == 0) )
        {
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

  if (LocaleCompare("-script",argv[1]) == 0)
    {
      /* Start processing directly from script, no pre-script options
        Replace wand command name with script name
      */
      GetPathComponent(argv[2],TailPath,cli_wand->wand.name);
      ProcessScriptOptions(cli_wand,argc-2,argv+2);
    }
  else if (LocaleCompare("-list",argv[1]) == 0)
    /* Special option, list information and exit
       FUTURE: this should be a MagickCore option, no wand is actually needed
    */

    CLISpecialOperator(cli_wand, argv[1]+1, argv[2]);
  else
    /* Processing Command line, assuming output file as last option */
    ProcessCommandOptions(cli_wand,argc-1,argv+1,MagickCommandOptionFlags);


  /* recover original image_info - check we get the right image_info */
  while (cli_wand->image_info_stack != (Stack *)NULL)
    CLISpecialOperator(cli_wand,"}",(const char *)NULL);
  assert(cli_wand->wand.image_info == image_info);
  assert(cli_wand->wand.exception == exception);

  /* Handle metadata for ImageMagickObject COM object for Windows VBS */
  if (metadata != (char **) NULL)
    {
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
      else
        {
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

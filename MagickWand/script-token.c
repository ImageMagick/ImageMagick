/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%    SSS    CCC  RRRR   III  PPPP  TTTTT    TTTTT  OOO   K  K  EEEE  N   N    %
%   S      C     R   R   I   P   P   T        T   O   O  K K   E     NN  N    %
%    SSS   C     RRRR    I   PPPP    T        T   O   O  KK    EEE   N N N    %
%       S  C     R R     I   P       T        T   O   O  K K   E     N  NN    %
%   SSSS    CCC  R  RR  III  P       T        T    OOO   K  K  EEEE  N   N    %
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
%  Read a stream of characters and return tokens one at a time
%
*/

/*
  Include declarations.
*/
#ifndef SCRIPT_TOKEN_TESTING
#  include "MagickWand/studio.h"
#  include "MagickWand/MagickWand.h"
#  include "MagickWand/script-token.h"
#  include "MagickCore/string-private.h"
#  include "MagickCore/utility-private.h"
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e S c r i p t T o k e n I n f o                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireScriptTokenInfo() allocated, initializes and opens the given
%  file stream from which tokens are to be extracted.
%
%  The format of the AcquireScriptTokenInfo method is:
%
%     ScriptTokenInfo *AcquireScriptTokenInfo(char *filename)
%
%  A description of each parameter follows:
%
%    o filename   the filename to open  ("-" means stdin)
%
*/
WandExport ScriptTokenInfo *AcquireScriptTokenInfo(char *filename)
{
  ScriptTokenInfo
    *token_info;

  token_info=(ScriptTokenInfo *) AcquireMagickMemory(sizeof(*token_info));
  if (token_info == (ScriptTokenInfo *) NULL)
    return token_info;
  (void) ResetMagickMemory(token_info,0,sizeof(*token_info));

  token_info->opened=MagickFalse;
  if ( LocaleCompare(filename,"-") == 0 ) {
    token_info->stream=stdin;
    token_info->opened=MagickFalse;
  }
#if 0  /* FUTURE POSIBILITIES */
  else if ( LocaleNCompare(filename,"fd:",3) == 0 ) {
    token_info->stream=fdopen(StringToLong(filename+3),"r");
    token_info->opened=MagickFalse;
  }
#endif
  else {
    token_info->stream=fopen(filename, "r");
  }
  if ( token_info->stream == (FILE *)NULL ) {
    token_info=(ScriptTokenInfo *) RelinquishMagickMemory(token_info);
    return(token_info);
  }

  token_info->curr_line=1;
  token_info->length=INITAL_TOKEN_LENGTH;
  token_info->token=(char *) AcquireMagickMemory(token_info->length);

  token_info->status=(token_info->token != (char *)NULL)
                      ? TokenStatusOK : TokenStatusMemoryFailed;
  token_info->signature=WandSignature;

  return token_info;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S c r i p t T o k e n I n f o                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyScriptTokenInfo() allocated, initializes and opens the given
%  file stream from which tokens are to be extracted.
%
%  The format of the DestroyScriptTokenInfo method is:
%
%     ScriptTokenInfo *DestroyScriptTokenInfo(ScriptTokenInfo *token_info)
%
%  A description of each parameter follows:
%
%    o token_info   The ScriptTokenInfo structure to be destroyed
%
*/
WandExport ScriptTokenInfo * DestroyScriptTokenInfo(ScriptTokenInfo *token_info)
{
  assert(token_info != (ScriptTokenInfo *) NULL);
  assert(token_info->signature == WandSignature);

  if ( token_info->opened != MagickFalse )
    fclose(token_info->stream);

  if (token_info->token != (char *) NULL )
    token_info->token=(char *) RelinquishMagickMemory(token_info->token);
  token_info=(ScriptTokenInfo *) RelinquishMagickMemory(token_info);
  return(token_info);
}

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
%  substitutions will not be performed.  Tokens can be any length.
%
%  Tokens are white space separated, and may be quoted, or even partially
%  quoted by either single or double quotes, or the use of backslashes,
%  or any mix of the three.
%
%  For example:    This\ is' a 'single" token"
%
%  A token is returned immediatally the end of token is found. That is as soon
%  as a unquoted white-space or EOF condition has been found.  That is to say
%  the file stream is parsed purely character-by-character, regardless any
%  buffering constraints set by the system.  It is not parsed line-by-line.
%
%  The function will return 'MagickTrue' if a valid token was found, while
%  the token status will be set accordingally to 'OK' or 'EOF', according to
%  the cause of the end of token.  The token may be an empty string if the
%  input was a quoted empty string.  Other error conditions return a value of
%  MagickFalse, indicating any token found but was incomplete due to some
%  error condition.
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
%  As a special case a ':' at the start of a line is also treated as a comment
%  This allows a magick script to ignore a line that is parsed by the shell
%  and can be used as a 'launcher' for magick scripts. For example
%
%    #!/bin/sh
%    #
%    # Shell Launcher for Magick Script
%    : echo "This part is run in the shell"
%    : exec magick -script "$0" "$@"; exit 10
%    #
%    # The rest of the script is magick script
%    -read label:"This is a Magick Script!"
%    -write show: -exit
%
%
%  The format of the GetScriptToken method is:
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

/* Macro to read character from stream

   This also keeps track of the line and column counts.
   The EOL is defined as either '\r\n', or '\r', or '\n'.
   A '\r' on its own is converted into a '\n' to correctly handle
   raw input, typically due to 'copy-n-paste' of text files.
*/
#define GetChar(c) \
{ \
  c=fgetc(token_info->stream); \
  token_info->curr_column++; \
  if ( c == '\r' ) { \
    c=fgetc(token_info->stream); \
    ungetc(c,token_info->stream); \
    c = (c!='\n')?'\n':'\r'; \
  } \
  if ( c == '\n' ) \
    token_info->curr_line++, token_info->curr_column=0; \
  if (c == EOF ) \
    break; \
  if ( (c>='\0' && c<'\a') || (c>'\r' && c<' ' && c!='\033') ) { \
    token_info->status=TokenStatusBinary; \
    break; \
  } \
}
/* macro to collect the token characters */
#define SaveChar(c) \
{ \
  if ((size_t) offset >= (token_info->length-1)) { \
    if ( token_info->length >= MaxTextExtent ) \
      token_info->length += MaxTextExtent; \
    else \
      token_info->length *= 4; \
    token_info->token = (char *) \
         ResizeMagickMemory(token_info->token, token_info->length); \
    if ( token_info->token == (char *)NULL ) { \
      token_info->status=TokenStatusMemoryFailed; \
      break; \
    } \
  } \
  token_info->token[offset++]=(char) (c); \
}

WandExport MagickBooleanType GetScriptToken(ScriptTokenInfo *token_info)
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

    /* hash comment handling */
    if ( state == IN_COMMENT ) {
      if ( c == '\n' )
        state=IN_WHITE;
      continue;
    }
    if ( state == IN_WHITE )
      if (c == '#' || (c == ':' && token_info->curr_column==1))
        state=IN_COMMENT;
    /* whitespace break character */
    if (strchr(" \n\r\t",c) != (char *)NULL) {
      switch (state) {
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
    if (strchr("'\"",c) != (char *)NULL) {
      switch (state) {
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
        if ( state==IN_QUOTE && quote == '\'' ) {
            SaveChar('\\');
            continue;
          }
        GetChar(c);
        if (c == '\n' || c == '\r' )
          switch (state) {
            case IN_COMMENT:
              state=IN_WHITE;  /* end comment */
            case IN_WHITE:
            case IN_TOKEN:
              continue;   /* line continuation (outside quotes and comment) */
          }
        switch (state) {
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
    switch (state) {
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
  /* input stream has EOF or produced a fatal error */
  token_info->token[offset]='\0';
  if ( token_info->status != TokenStatusOK )
    return(MagickFalse);  /* fatal condition - no valid token */
  token_info->status = TokenStatusEOF;
  if ( state == IN_QUOTE)
    token_info->status = TokenStatusBadQuotes;
  if ( state == IN_TOKEN)
    return(MagickTrue);   /* token with EOF at end - no problem */
  return(MagickFalse);    /* in white space or in quotes - invalid token */
}

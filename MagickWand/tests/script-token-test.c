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
%                         TTTTT  EEEEE  SSSSS  TTTTT                          %
%                           T    E      SS       T                            %
%                           T    EEE     SSS     T                            %
%                           T    E         SS    T                            %
%                           T    EEEEE  SSSSS    T                            %
%                                                                             %
%       Perform "Magick" on Images via the Command Line Interface             %
%                                                                             %
%                             Dragon Computing                                %
%                             Anthony Thyssen                                 %
%                               January 2012                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999 ImageMagick Studio LLC, a non-profit organization           %
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
%  Test the raw tokenization of the  ScriptToken Subroutines
%
%  This actually uses very little of the magic core functions
%  and in fact creates a completely stand-alone program by substituting
%  required MagickCore with direct system equivalents.
%
%  Build
%     cc     script-token-test.c   -o script-token-test
%
%  For testing see  script-token-test.sh
%
*/

/* System Replacement for MagickWand includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

/* Defines to replace MagickWand / MagickCore definitions */
#define MagickPathExtent     4096
#define MagickFalse       0
#define MagickTrue        1
#define MagickBooleanType int

#define AcquireMagickMemory(s)    malloc(s)
#define RelinquishMagickMemory(p) (free(p),NULL)
#define ResizeMagickMemory(p,s)   realloc(p,s)
#define ResetMagickMemory(p,b,s)  memset(p,b,s)
#define StringToLong(s)           strtol(s,(char **) NULL,10)
#define LocaleCompare(p,q)        strcasecmp(p,q)
#define LocaleNCompare(p,q,l)     strncasecmp(p,q,l)
#define WandSignature             0xabacadabUL
#define fopen_utf8(p,q)           fopen(p,q)
#define WandExport

/* Include the actual code for ScriptToken functions */
#define SCRIPT_TOKEN_TESTING  1 /* Prevent MagickWand Includes */
#include "../script-token.h"
#include "../script-token.c"

/* Test program to report what tokens it finds in given input file/stream */

int main(int argc, char *argv[])
{
  ScriptTokenInfo
     *token_info;

  token_info = AcquireScriptTokenInfo( (argc>1) ? argv[1] : "-" );
  if (token_info == (ScriptTokenInfo *) NULL) {
    printf("Script Open Failure : %s\n", strerror(errno));
    return(1);
  }

  while (1) {
    if( GetScriptToken(token_info) == MagickFalse )
      break;

    if( strlen(token_info->token) > INITAL_TOKEN_LENGTH-1 ) {
      token_info->token[INITAL_TOKEN_LENGTH-4] = '.';
      token_info->token[INITAL_TOKEN_LENGTH-3] = '.';
      token_info->token[INITAL_TOKEN_LENGTH-2] = '.';
      token_info->token[INITAL_TOKEN_LENGTH-1] = '\0';
    }
    printf("l=%d, c=%d, stat=%d, len=%d, token=\"%s\"\n",
         token_info->token_line, token_info->token_column,
         token_info->status, token_info->length, token_info->token);
  }

  switch( token_info->status ) {
    case TokenStatusOK:
      break;
    case TokenStatusEOF:
      printf("EOF Found\n");
      break;
    case TokenStatusBadQuotes:
      /* Ensure last token has a sane length for error report */
      if( strlen(token_info->token) > INITAL_TOKEN_LENGTH-1 ) {
        token_info->token[INITAL_TOKEN_LENGTH-4] = '.';
        token_info->token[INITAL_TOKEN_LENGTH-3] = '.';
        token_info->token[INITAL_TOKEN_LENGTH-2] = '.';
        token_info->token[INITAL_TOKEN_LENGTH-1] = '\0';
      }
      printf("Bad Quotes l=%d, c=%d  token=\"%s\"\n",
           token_info->token_line,token_info->token_column, token_info->token);
      break;
    case TokenStatusMemoryFailed: /* token is invalid */
      printf("Out of Memory  l=%d, c=%d\n",
           token_info->token_line,token_info->token_column);
      break;
    case TokenStatusBinary:       /* token is invalid */
      printf("Binary Char at l=%d, c=%d\n",
           token_info->curr_line,token_info->curr_column);
      break;
  }

  /* Clean up */
  token_info = DestroyScriptTokenInfo(token_info);

  return(0);
}

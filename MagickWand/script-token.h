/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
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
#ifndef _SCRIPT_TOKEN_H
#define _SCRIPT_TOKEN_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Status of the Stream */
typedef enum {
  TokenStatusOK = 0,
  TokenStatusEOF,
  TokenStatusBadQuotes,
  TokenStatusBinary,
  TokenStatusMemoryFailed
} TokenStatus;

/* Initial length is MagickPathExtent/64 => 64  (divisor is a power of 4)
   most tokens are never larger than this, so no need to waste memory!
   Also no CLI option is larger than about 40 characters!
*/
#define INITAL_TOKEN_LENGTH  64
typedef struct
{
  FILE
    *stream;        /* the file stream we are reading from */

  MagickBooleanType
    opened;         /* was that stream opened? */

  char
    *token;         /* array of characters to holding details of he token */

  size_t
    length,         /* length of token char array */
    curr_line,      /* current location in script file */
    curr_column,
    token_line,      /* start of last token (option or argument) */
    token_column;

  TokenStatus
    status;         /* Have we reached EOF? see Token Status */

  size_t
    signature;
} ScriptTokenInfo;


extern WandExport ScriptTokenInfo
  *AcquireScriptTokenInfo(const char *),
  *DestroyScriptTokenInfo(ScriptTokenInfo *);

extern WandExport MagickBooleanType
  GetScriptToken(ScriptTokenInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

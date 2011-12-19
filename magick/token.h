/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore token methods.
*/
#ifndef _MAGICKCORE_TOKEN_H
#define _MAGICKCORE_TOKEN_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  Typedef declarations.
*/
typedef struct _TokenInfo
  TokenInfo;

extern MagickExport int
  Tokenizer(TokenInfo *,const unsigned int,char *,const size_t,const char *,
    const char *,const char *,const char *,const char,char *,int *,char *);

extern MagickExport MagickBooleanType
  GlobExpression(const char *,const char *,const MagickBooleanType),
  IsGlob(const char *);

extern MagickExport TokenInfo
  *AcquireTokenInfo(void),
  *DestroyTokenInfo(TokenInfo *);

extern MagickExport void
  GetMagickToken(const char *,const char **,char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

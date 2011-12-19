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

  MagickCore random methods.
*/
#ifndef _MAGICKCORE_RANDOM__H
#define _MAGICKCORE_RANDOM__H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/string_.h"

/*
  Typedef declarations.
*/
typedef struct _RandomInfo
  RandomInfo;

/*
  Method declarations.
*/
extern MagickExport double
  GetRandomValue(RandomInfo *),
  GetPseudoRandomValue(RandomInfo *);

extern MagickExport MagickBooleanType
  RandomComponentGenesis(void);

extern MagickExport RandomInfo
  *AcquireRandomInfo(void),
  *DestroyRandomInfo(RandomInfo *);

extern MagickExport StringInfo
  *GetRandomKey(RandomInfo *,const size_t);

extern MagickExport void
  RandomComponentTerminus(void),
  SeedPseudoRandomGenerator(const unsigned long),
  SetRandomKey(RandomInfo *,const size_t,unsigned char *),
  SetRandomTrueRandom(const MagickBooleanType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

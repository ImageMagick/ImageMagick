/*
  Copyright @ 2001 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore random generation private methods.
*/
#ifndef MAGICKCORE_RANDOM_PRIVATE_H
#define MAGICKCORE_RANDOM_PRIVATE_H

#include "MagickCore/thread-private.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickPrivate double
  GetRandomInfoNormalize(const RandomInfo *);

extern MagickPrivate MagickBooleanType
  RandomComponentGenesis(void);

extern MagickPrivate unsigned long
  *GetRandomInfoSeed(RandomInfo *);

extern MagickPrivate void
  RandomComponentTerminus(void);

static inline RandomInfo **DestroyRandomInfoTLS(RandomInfo **random_info)
{
  ssize_t
    i;

  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (random_info[i] != (RandomInfo *) NULL)
      random_info[i]=DestroyRandomInfo(random_info[i]);
  return((RandomInfo **) RelinquishMagickMemory(random_info));
}

static inline RandomInfo **AcquireRandomInfoTLS(void)
{
  ssize_t
    i;

  RandomInfo
    **random_info;

  size_t
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  random_info=(RandomInfo **) AcquireQuantumMemory(number_threads,
    sizeof(*random_info));
  if (random_info == (RandomInfo **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(random_info,0,number_threads*sizeof(*random_info));
  for (i=0; i < (ssize_t) number_threads; i++)
    random_info[i]=AcquireRandomInfo();
  return(random_info);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

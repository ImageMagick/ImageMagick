/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/license/

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

/*
  The per-thread RandomInfo array carries its own element count in a hidden
  header that precedes the first slot.  Destruction must use this recorded
  count rather than re-reading the (process-wide, runtime-writable)
  ThreadResource limit, which may have changed since the array was acquired.
*/
static inline size_t RandomInfoTLSHeaderSlots(void)
{
  return((sizeof(size_t)+sizeof(RandomInfo *)-1)/sizeof(RandomInfo *));
}

static inline RandomInfo **DestroyRandomInfoTLS(RandomInfo **random_info)
{
  RandomInfo
    **base;

  size_t
    i,
    number_threads;

  if (random_info == (RandomInfo **) NULL)
    return((RandomInfo **) NULL);
  base=random_info-RandomInfoTLSHeaderSlots();
  (void) memcpy(&number_threads,base,sizeof(number_threads));
  for (i=0; i < number_threads; i++)
    if (random_info[i] != (RandomInfo *) NULL)
      random_info[i]=DestroyRandomInfo(random_info[i]);
  return((RandomInfo **) RelinquishMagickMemory(base));
}

static inline RandomInfo **AcquireRandomInfoTLS(void)
{
  RandomInfo
    **base,
    **random_info;

  size_t
    header_slots,
    i,
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  header_slots=RandomInfoTLSHeaderSlots();
  base=(RandomInfo **) AcquireQuantumMemory(number_threads+header_slots,
    sizeof(*base));
  if (base == (RandomInfo **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(base,0,(number_threads+header_slots)*sizeof(*base));
  (void) memcpy(base,&number_threads,sizeof(number_threads));
  random_info=base+header_slots;
  for (i=0; i < number_threads; i++)
    random_info[i]=AcquireRandomInfo();
  return(random_info);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

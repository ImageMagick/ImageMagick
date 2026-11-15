/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                  TTTTT  H   H  RRRR   EEEEE   AAA   DDDD                    %
%                    T    H   H  R   R  E      A   A  D   D                   %
%                    T    HHHHH  RRRR   EEE    AAAAA  D   D                   %
%                    T    H   H  R R    E      A   A  D   D                   %
%                    T    H   H  R  R   EEEEE  A   A  DDDD                    %
%                                                                             %
%                                                                             %
%                         MagickCore Thread Methods                           %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                               March  2003                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/memory_.h"
#include "MagickCore/thread_.h"
#include "MagickCore/thread-private.h"

/*
  Typedef declarations.
*/
typedef struct _MagickThreadValue
{
  size_t
    number_threads;

  void
    **values,
    (*destructor)(void *);
} MagickThreadValue;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C r e a t e M a g i c k T h r e a d K e y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CreateMagickThreadKey() creates a thread-specific data key visible to all
%  threads in the process.
%
%  The format of the CreateMagickThreadKey method is:
%
%      MagickBooleanType CreateMagickThreadKey(MagickThreadKey *key,
%        void (*destructor)(void *))
%
%  A description of each parameter follows:
%
%    o key: opaque objects used to locate thread-specific data.
%
%    o destructor: associate an optional destructor with each key value.
%
*/
MagickExport MagickBooleanType CreateMagickThreadKey(MagickThreadKey *key,
  void (*destructor)(void *))
{
  if (key == (MagickThreadKey *) NULL)
    return(MagickFalse);
#if defined(MAGICKCORE_THREAD_SUPPORT)
  return(pthread_key_create(key,destructor) == 0 ? MagickTrue : MagickFalse);
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  magick_unreferenced(destructor);
  *key=TlsAlloc();
  return(*key != TLS_OUT_OF_INDEXES ? MagickTrue : MagickFalse);
#else
  {
    MagickThreadValue
      **keys;

    keys=(MagickThreadValue **) key;
    *keys=(MagickThreadValue *) AcquireMagickMemory(sizeof(**keys));
    if (*keys != (MagickThreadValue *) NULL)
      {
        (*keys)->number_threads=GetOpenMPMaximumThreads();
        if ((*keys)->number_threads == 0)
          (*keys)->number_threads=1;
        (*keys)->values=(void **) AcquireQuantumMemory((*keys)->number_threads,
          sizeof(void *));
        if ((*keys)->values == (void **) NULL)
          *keys=(MagickThreadValue *) RelinquishMagickMemory(*keys);
        else
          {
            (void) memset((*keys)->values,0,(*keys)->number_threads*
              sizeof(void *));
            (*keys)->destructor=destructor;
          }
      }
    return((*keys != (MagickThreadValue *) NULL) ? MagickTrue : MagickFalse);
  }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e M a g i c k T h r e a d K e y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteMagickThreadKey() deletes a thread-specific data key.
%
%  The format of the DeleteMagickThreadKey method is:
%
%      MagickBooleanType DeleteMagickThreadKey(MagickThreadKey key)
%
%  A description of each parameter follows:
%
%    o key: the thread key.
%
*/
MagickExport MagickBooleanType DeleteMagickThreadKey(MagickThreadKey key)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
  return(pthread_key_delete(key) == 0 ? MagickTrue : MagickFalse);
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  if (key == TLS_OUT_OF_INDEXES)
    return(MagickFalse);
  return(TlsFree(key) != 0 ? MagickTrue : MagickFalse);
#else
  {
    MagickThreadValue
      *keys;

    ssize_t
      i;

    keys=(MagickThreadValue *) key;
    if (keys == (MagickThreadValue *) NULL)
      return(MagickFalse);
    if (keys->values != (void **) NULL)
      {
        for (i=0; i < (ssize_t) keys->number_threads; i++)
          if ((keys->destructor != (void *) NULL) &&
              (keys->values[i] != (void *) NULL))
            {
              keys->destructor(keys->values[i]);
              keys->values[i]=(void *) NULL;
            }
        keys->values=(void **) RelinquishMagickMemory(keys->values);
      }
    keys=(MagickThreadValue *) RelinquishMagickMemory(keys);
  }
  return(MagickTrue);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k T h r e a d V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickThreadValue() returns the value currently bound to the specified
%  key on behalf of the calling thread.
%
%  The format of the GetMagickThreadValue method is:
%
%      void *GetMagickThreadValue(MagickThreadKey key)
%
%  A description of each parameter follows:
%
%    o key: the thread key.
%
*/
MagickExport void *GetMagickThreadValue(MagickThreadKey key)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
  return(pthread_getspecific(key));
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  if (key == TLS_OUT_OF_INDEXES)
    return((void *) NULL);
  return(TlsGetValue(key));
#else
  {
    MagickThreadValue
      *keys;

    size_t
      thread_id;

    keys=(MagickThreadValue *) key;
    if ((keys == (MagickThreadValue *) NULL) ||
        (keys->values == (void **) NULL))
      return((void *) NULL);
    thread_id=(size_t) GetOpenMPThreadId();
    if (thread_id >= keys->number_threads)
      return((void *) NULL);
    return(keys->values[thread_id]);
  }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t M a g i c k T h r e a d V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetMagickThreadValue() binds a value to the specified key on behalf of the
%  calling thread.
%
%  The format of the SetMagickThreadValue method is:
%
%      MagickBooleanType SetMagickThreadValue(MagickThreadKey key,
%        const void *value)
%
%  A description of each parameter follows:
%
%    o key: the thread key.
%
%    o value: the value.
%
*/
MagickExport MagickBooleanType SetMagickThreadValue(MagickThreadKey key,
  const void *value)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
  return(pthread_setspecific(key,value) == 0 ? MagickTrue : MagickFalse);
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  if (key == TLS_OUT_OF_INDEXES)
    return(MagickFalse);
  return(TlsSetValue(key,(void *) value) != 0 ? MagickTrue : MagickFalse);
#else
  {
    MagickThreadValue
      *keys;

    size_t
      thread_id;

    keys=(MagickThreadValue *) key;
    if ((keys == (MagickThreadValue *) NULL) ||
        (keys->values == (void **) NULL))
      return(MagickFalse);
    thread_id=(size_t) GetOpenMPThreadId();
    if (thread_id >= keys->number_threads)
      return(MagickFalse);
    keys->values[thread_id]=(void *) value;
  }
  return(MagickTrue);
#endif
}

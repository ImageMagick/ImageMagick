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
%                               John Cristy                                   %
%                               March  2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/memory_.h"
#include "magick/thread_.h"
#include "magick/thread-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C r e a t e T h r e a d K e y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCreateThreadKey() creates a thread key and returns it.
%
%  The format of the MagickCreateThreadKey method is:
%
%      MagickThreadKey MagickCreateThreadKey(MagickThreadKey *key)
%
*/
MagickExport MagickBooleanType MagickCreateThreadKey(MagickThreadKey *key)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
  return(pthread_key_create(key,NULL) == 0 ? MagickTrue : MagickFalse);
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
  *key=TlsAlloc();
  return(*key != TLS_OUT_OF_INDEXES ? MagickTrue : MagickFalse);
#else
  *key=AcquireMagickMemory(sizeof(key));
  return(*key != (void *) NULL ? MagickTrue : MagickFalse);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D e l e t e T h r e a d K e y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDeleteThreadKey() deletes a thread key.
%
%  The format of the AcquireAESInfo method is:
%
%      MagickBooleanType MagickDeleteThreadKey(MagickThreadKey key)
%
%  A description of each parameter follows:
%
%    o key: the thread key.
%
*/
MagickExport MagickBooleanType MagickDeleteThreadKey(MagickThreadKey key)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
  return(pthread_key_delete(key) == 0 ? MagickTrue : MagickFalse);
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
  return(TlsFree(key) != 0 ? MagickTrue : MagickFalse);
#else
  key=(MagickThreadKey) RelinquishMagickMemory(key);
  return(MagickTrue);
#endif

}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t T h r e a d V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetThreadValue() returns a value associated with the thread key.
%
%  The format of the MagickGetThreadValue method is:
%
%      void *MagickGetThreadValue(MagickThreadKey key)
%
%  A description of each parameter follows:
%
%    o key: the thread key.
%
*/
MagickExport void *MagickGetThreadValue(MagickThreadKey key)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
  return(pthread_getspecific(key));
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
  return(TlsGetValue(key));
#else
  return((void *) (*key));
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t T h r e a d V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetThreadValue() associates a value with the thread key.
%
%  The format of the MagickSetThreadValue method is:
%
%      MagickBooleanType MagickSetThreadValue(MagickThreadKey key,
%        const void *value)
%
%  A description of each parameter follows:
%
%    o key: the thread key.
%
%    o value: the value
%
*/
MagickExport MagickBooleanType MagickSetThreadValue(MagickThreadKey key,
  const void *value)
{
#if defined(MAGICKCORE_THREAD_SUPPORT)
  return(pthread_setspecific(key,value) == 0 ? MagickTrue : MagickFalse);
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
  return(TlsSetValue(key,(void *) value) != 0 ? MagickTrue : MagickFalse);
#else
  *key=(size_t) value;
  return(MagickTrue);
#endif
}

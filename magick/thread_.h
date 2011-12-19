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

  MagickCore private methods for internal threading.
*/
#ifndef _MAGICKCORE_THREAD_H
#define _MAGICKCORE_THREAD_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_THREAD_SUPPORT)
typedef pthread_t MagickThreadType;
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
typedef DWORD MagickThreadType;
#else
typedef pid_t MagickThreadType;
#endif

#if defined(MAGICKCORE_THREAD_SUPPORT)
typedef pthread_key_t MagickThreadKey;
#elif defined(MAGICKCORE_HAVE_WINTHREADS)
typedef DWORD MagickThreadKey;
#else
typedef size_t *MagickThreadKey;
#endif

extern MagickExport MagickBooleanType
  MagickCreateThreadKey(MagickThreadKey *),
  MagickDeleteThreadKey(MagickThreadKey),
  MagickSetThreadValue(MagickThreadKey,const void *);

extern MagickExport void
  *MagickGetThreadValue(MagickThreadKey);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

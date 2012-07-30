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

  MagickCore image resampling private methods.
*/
#ifndef _MAGICKCORE_RESAMPLE_PRIVATE_H
#define _MAGICKCORE_RESAMPLE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/thread-private.h"

static inline ResampleFilter **DestroyResampleFilterThreadSet(
  ResampleFilter **filter)
{
  register ssize_t
    i;

  assert(filter != (ResampleFilter **) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (filter[i] != (ResampleFilter *) NULL)
      filter[i]=DestroyResampleFilter(filter[i]);
  filter=(ResampleFilter **) RelinquishAlignedMemory(filter);
  return(filter);
}

static inline ResampleFilter **AcquireResampleFilterThreadSet(
  const Image *image,const VirtualPixelMethod method,
  const MagickBooleanType interpolate,ExceptionInfo *exception)
{
  register ssize_t
    i;

  ResampleFilter
    **filter;

  size_t
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  filter=(ResampleFilter **) AcquireAlignedMemory(number_threads,
    sizeof(*filter));
  if (filter == (ResampleFilter **) NULL)
    return((ResampleFilter **) NULL);
  (void) ResetMagickMemory(filter,0,number_threads*sizeof(*filter));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    filter[i]=AcquireResampleFilter(image,exception);
    if (filter[i] == (ResampleFilter *) NULL)
      return(DestroyResampleFilterThreadSet(filter));
    if (method != UndefinedVirtualPixelMethod)
      (void) SetResampleFilterVirtualPixelMethod(filter[i],method);
    if (interpolate != MagickFalse)
      SetResampleFilter(filter[i],PointFilter,1.0);
  }
  return(filter);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

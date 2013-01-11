/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  The ImageMagick progress monitor private methods.
*/
#ifndef _MAGICK_MONITOR_PRIVATE_H
#define _MAGICK_MONITOR_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/image.h"

static inline MagickBooleanType SetImageProgress(const Image *image,
  const char *tag,const MagickOffsetType offset,const MagickSizeType extent)
{
  char
    message[MaxTextExtent];

  if (image->progress_monitor == (MagickProgressMonitor) NULL)
    return(MagickTrue);
  (void) FormatLocaleString(message,MaxTextExtent,"%s/%s",tag,image->filename);
  return(image->progress_monitor(message,offset,extent,image->client_data));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

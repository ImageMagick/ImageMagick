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

  MagickCore private image constitute methods.
*/
#ifndef MAGICKCORE_CONSTITUTE_PRIVATE_H
#define MAGICKCORE_CONSTITUTE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/log.h"
#include "MagickCore/utility.h"

static inline MagickBooleanType IsAllowedCoder(const char *coder)
{
  static const char
    *allowed_coders[] = {
     "MPR",
     "MPRI",
     NULL
   };

  const char **p = allowed_coders;
  while (*p != NULL)
  {
    if (LocaleCompare(coder,*p) == 0)
      return(MagickTrue);
    p++;
  }
  return(MagickFalse);
}

static inline Image *StrictReadImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    magic[MagickPathExtent];

  (void) GetPathComponent(image_info->filename,MagickPath,magic);
  if (*magic != '\0')
    {
      LocaleUpper(magic);
      if (IsAllowedCoder(magic) == MagickFalse)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            OptionError, "ExplicitCoderNotAllowed","`%s'",
            image_info->filename);
         return((Image *) NULL);
       }
    }
  else
    if (IsPathAccessible(image_info->filename) == MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          FileOpenError, "UnableToOpenFile","`%s'",
          image_info->filename);
        return((Image *) NULL);
      }
  return(ReadImage(image_info,exception));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

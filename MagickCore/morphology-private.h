/*
  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  The ImageMagick morphology private methods.
*/
#ifndef _MAGICK_MORPHOLOGY_PRIVATE_H
#define _MAGICK_MORPHOLOGY_PRIVATE_H

#include "MagickCore/morphology.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickPrivate Image
  *MorphologyApply(const Image *,const MorphologyMethod,const ssize_t,
    const KernelInfo *,const CompositeOperator,const double,ExceptionInfo *);

extern MagickPrivate void
  ShowKernelInfo(const KernelInfo *),
  ZeroKernelNans(KernelInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

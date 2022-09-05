/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private graphic gems methods.
*/
#ifndef MAGICKCORE_STATISTIC_PRIVATE_H
#define MAGICKCORE_STATISTIC_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline double MagickLog10(const double x)
{
  if (fabs(x) < MagickEpsilon)
    return(log10(MagickEpsilon));
  return(log10(fabs(x)));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

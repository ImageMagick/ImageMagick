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

static inline MagickBooleanType MagickSafeSignificantError(const double error,
  const double fuzz)
{
  double threshold = (fuzz > 0.0 ? fuzz : MagickEpsilon)*(1.0+MagickEpsilon);
  return(error > threshold ? MagickTrue : MagickFalse);
}

static inline double MagickSafeLog10(const double x)
{
  if (x < MagickEpsilon)
    return(log10(MagickEpsilon));
  if (fabs(x-1.0) < MagickEpsilon)
    return(0.0);
  return(log10(x));
}

static inline double MagickSafeReciprocal(const double x)
{
  if ((x > -MagickEpsilon) && (x < MagickEpsilon))
    return(1.0/MagickEpsilon);
  return(1.0/x);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

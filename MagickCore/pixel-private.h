/*
  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image pixel private methods.
*/
#ifndef _MAGICKCORE_PIXEL_PRIVATE_H
#define _MAGICKCORE_PIXEL_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline Quantum ClampPixel(const MagickRealType value)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) value);
#else
  if (value < 0.0)
    return((Quantum) 0.0);
  if (value >= (MagickRealType) QuantumRange)
    return((Quantum) QuantumRange);
  return((Quantum) value);
#endif
}

static inline double PerceptibleReciprocal(const double x)
{
  double
    sign;

  /*
    Return 1/x where x is perceptible (not unlimited or infinitesimal).
  */
  sign=x < 0.0 ? -1.0 : 1.0;
  if ((sign*x) >= MagickEpsilon)
    return(1.0/x);
  return(sign/MagickEpsilon);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

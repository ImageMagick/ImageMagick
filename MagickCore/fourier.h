/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore discrete Fourier transform (DFT) methods.
*/
#ifndef MAGICKCORE_FFT_H
#define MAGICKCORE_FFT_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedComplexOperator,
  AddComplexOperator,
  ConjugateComplexOperator,
  DivideComplexOperator,
  MagnitudePhaseComplexOperator,
  MultiplyComplexOperator,
  RealImaginaryComplexOperator,
  SubtractComplexOperator
} ComplexOperator;

extern MagickExport Image
 *ComplexImages(const Image *,const ComplexOperator,ExceptionInfo *),
 *ForwardFourierTransformImage(const Image *,const MagickBooleanType,
   ExceptionInfo *),
 *InverseFourierTransformImage(const Image *,const Image *,
   const MagickBooleanType,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

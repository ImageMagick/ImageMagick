/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore morphology methods.
*/
#ifndef _MAGICKCORE_MORPHOLOGY_H
#define _MAGICKCORE_MORPHOLOGY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/geometry.h>

typedef enum
{
  UndefinedKernel,    /* also the 'no-op' kernel */
  GaussianKernel,     /* Convolution Kernels */
  BlurKernel,
  CometKernel,
  LaplacianKernel,
  LOGKernel,
  DOGKernel,
  RectangleKernel,    /* Boolean Kernels */
  SquareKernel,
  DiamondKernel,
  DiskKernel,
  PlusKernel,
  ChebyshevKernel,    /* Distance Measuring Kernels */
  ManhattenKernel,
  EuclideanKernel,
  UserDefinedKernel   /* user specified kernel values */
} MagickKernelType;

typedef enum
{
  UndefinedMorphology,
  ConvolveMorphology,          /* Weighted Sum of pixels - Convolve */
  DialateMorphology,           /* Weighted Value Maximum */
  ErodeMorphology,             /* Weighted Value Minimum */
  CloseMorphology,             /* Erode then Dialate */
  OpenMorphology,              /* Dialate then Erode */
  DialateIntensityMorphology,  /* Pixel Pick using GreyScale Dialate */
  ErodeIntensityMorphology,    /* Pixel Pick using GreyScale Erode */
  CloseIntensityMorphology,    /* Pixel Pick using GreyScale Close */
  OpenIntensityMorphology,     /* Pixel Pick using GreyScale Open */
  DistanceMorphology           /* Add to Value, take Minimum */
} MorphologyMethod;

typedef struct
{
  MagickKernelType
    type;

  unsigned long
    width,
    height,
    offset_x,
    offset_y;

  double *
    values;

  double
    value_min,
    value_max,
    range_neg,
    range_pos;

  MagickBooleanType
    normalized;

} MagickKernel;

extern MagickExport MagickKernel
  *AcquireKernelFromString(const char *),
  *AcquireKernelBuiltIn(const MagickKernelType, const GeometryInfo *),
  *DestroyKernel(MagickKernel *);

extern MagickExport void
  KernelNormalize(MagickKernel *),
  KernelPrint(MagickKernel *),
  KernelRotate(MagickKernel *, double);

extern MagickExport Image
  *MorphologyImage(const Image *, const ChannelType, MorphologyMethod,
      const long, MagickKernel *, ExceptionInfo *);


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

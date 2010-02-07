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
  RectangleKernel,    /* Shape Kernels */
  SquareKernel,
  DiamondKernel,
  DiskKernel,
  PlusKernel,
  ChebyshevKernel,    /* Distance Measuring Kernels */
  ManhattenKernel,
  EuclideanKernel,
  UserDefinedKernel   /* user specified kernel values */
} KernelInfoType;

typedef enum
{
  UndefinedMorphology,
/* Basic Morphology methods */
  ConvolveMorphology,          /* Weighted Sum of pixels - Convolve */
  ErodeMorphology,             /* Weighted Value Minimum */
  DilateMorphology,            /* Weighted Value Maximum */
  ErodeIntensityMorphology,    /* Pixel Pick using GreyScale Erode */
  DilateIntensityMorphology,   /* Pixel Pick using GreyScale Dialate */
  DistanceMorphology,          /* Add to Value, take Minimum */
/* Compound Morphology methods */
  OpenMorphology,              /* Dilate then Erode */
  CloseMorphology,             /* Erode then Dilate */
  OpenIntensityMorphology,     /* Pixel Pick using GreyScale Open */
  CloseIntensityMorphology,    /* Pixel Pick using GreyScale Close */
  EdgeInMorphology,            /* Dilate difference from orig */
  EdgeOutMorphology,           /* Erode difference from orig */
  EdgeMorphology,              /* Dilate difference with Erode */
  TopHatMorphology,            /* Close difference from Original */
  BottomHatMorphology          /* Open difference from Original */
} MorphologyMethod;

typedef struct
{
  KernelInfoType
    type;

  unsigned long
    width,
    height;

  long
    x,
    y;

  double
    *values,
    minimum,
    maximum,
    negative_range,
    positive_range;

  unsigned long
    signature;
} KernelInfo;

extern MagickExport KernelInfo
  *AcquireKernelInfo(const char *),
  *AcquireKernelBuiltIn(const KernelInfoType, const GeometryInfo *),
  *DestroyKernelInfo(KernelInfo *);

extern MagickExport void
  ShowKernelInfo(KernelInfo *);

extern MagickExport Image
  *MorphologyImage(const Image *,MorphologyMethod,const long,KernelInfo *,
    ExceptionInfo *),
  *MorphologyImageChannel(const Image *,const ChannelType,MorphologyMethod,
    const long,KernelInfo *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

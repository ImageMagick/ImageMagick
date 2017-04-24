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

  MagickCore morphology methods.
*/
#ifndef MAGICKCORE_MORPHOLOGY_H
#define MAGICKCORE_MORPHOLOGY_H

#include "MagickCore/geometry.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedKernel,    /* equivalent to UnityKernel */
  UnityKernel,        /* The no-op or 'original image' kernel */
  GaussianKernel,     /* Convolution Kernels, Gaussian Based */
  DoGKernel,
  LoGKernel,
  BlurKernel,
  CometKernel,
  BinomialKernel,
  LaplacianKernel,    /* Convolution Kernels, by Name */
  SobelKernel,
  FreiChenKernel,
  RobertsKernel,
  PrewittKernel,
  CompassKernel,
  KirschKernel,
  DiamondKernel,      /* Shape Kernels */
  SquareKernel,
  RectangleKernel,
  OctagonKernel,
  DiskKernel,
  PlusKernel,
  CrossKernel,
  RingKernel,
  PeaksKernel,         /* Hit And Miss Kernels */
  EdgesKernel,
  CornersKernel,
  DiagonalsKernel,
  LineEndsKernel,
  LineJunctionsKernel,
  RidgesKernel,
  ConvexHullKernel,
  ThinSEKernel,
  SkeletonKernel,
  ChebyshevKernel,    /* Distance Measuring Kernels */
  ManhattanKernel,
  OctagonalKernel,
  EuclideanKernel,
  UserDefinedKernel   /* User Specified Kernel Array */
} KernelInfoType;

typedef enum
{
  UndefinedMorphology,
/* Convolve / Correlate weighted sums */
  ConvolveMorphology,           /* Weighted Sum with reflected kernel */
  CorrelateMorphology,          /* Weighted Sum using a sliding window */
/* Low-level Morphology methods */
  ErodeMorphology,              /* Minimum Value in Neighbourhood */
  DilateMorphology,             /* Maximum Value in Neighbourhood */
  ErodeIntensityMorphology,     /* Pixel Pick using GreyScale Erode */
  DilateIntensityMorphology,    /* Pixel Pick using GreyScale Dialate */
  IterativeDistanceMorphology,  /* Add Kernel Value, take Minimum */
/* Second-level Morphology methods */
  OpenMorphology,               /* Dilate then Erode */
  CloseMorphology,              /* Erode then Dilate */
  OpenIntensityMorphology,      /* Pixel Pick using GreyScale Open */
  CloseIntensityMorphology,     /* Pixel Pick using GreyScale Close */
  SmoothMorphology,             /* Open then Close */
/* Difference Morphology methods */
  EdgeInMorphology,             /* Dilate difference from Original */
  EdgeOutMorphology,            /* Erode difference from Original */
  EdgeMorphology,               /* Dilate difference with Erode */
  TopHatMorphology,             /* Close difference from Original */
  BottomHatMorphology,          /* Open difference from Original */
/* Recursive Morphology methods */
  HitAndMissMorphology,         /* Foreground/Background pattern matching */
  ThinningMorphology,           /* Remove matching pixels from image */
  ThickenMorphology,            /* Add matching pixels from image */
/* Directly Applied Morphology methods */
  DistanceMorphology,           /* Add Kernel Value, take Minimum */
  VoronoiMorphology             /* Distance matte channel copy nearest color */
} MorphologyMethod;

typedef struct _KernelInfo
{
  KernelInfoType
    type;

  size_t
    width,
    height;

  ssize_t
    x,
    y;

  MagickRealType
    *values;

  double
    minimum,
    maximum,
    negative_range,
    positive_range,
    angle;

  struct _KernelInfo
    *next;

  size_t
    signature;
} KernelInfo;

extern MagickExport KernelInfo
  *AcquireKernelInfo(const char *,ExceptionInfo *),
  *AcquireKernelBuiltIn(const KernelInfoType,const GeometryInfo *,
    ExceptionInfo *),
  *CloneKernelInfo(const KernelInfo *),
  *DestroyKernelInfo(KernelInfo *);

extern MagickExport Image
  *MorphologyImage(const Image *,const MorphologyMethod,const ssize_t,
    const KernelInfo *,ExceptionInfo *);

extern MagickExport void
  ScaleGeometryKernelInfo(KernelInfo *,const char *),
  ScaleKernelInfo(KernelInfo *,const double,const GeometryFlags),
  UnityAddKernelInfo(KernelInfo *,const double);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

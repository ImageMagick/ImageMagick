/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
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

#include "magick/geometry.h"

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
  UserDefinedKernel,   /* User Specified Kernel Array */
  BinomialKernel
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
  DistanceMorphology,           /* Add Kernel Value, take Minimum */
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
/* Experimental Morphology methods */
  VoronoiMorphology,            /* distance matte channel copy nearest color */
  IterativeDistanceMorphology   /* Add Kernel Value, take Minimum */
} MorphologyMethod;

typedef struct KernelInfo
{
  KernelInfoType
    type;

  size_t
    width,
    height;

  ssize_t
    x,
    y;

  double
    *values,
    minimum,
    maximum,
    negative_range,
    positive_range,
    angle;

  struct KernelInfo
    *next;

  size_t
    signature;
} KernelInfo;

extern MagickExport KernelInfo
  *AcquireKernelInfo(const char *),
  *AcquireKernelBuiltIn(const KernelInfoType,const GeometryInfo *),
  *CloneKernelInfo(const KernelInfo *),
  *DestroyKernelInfo(KernelInfo *);

extern MagickExport Image
  *MorphologyImage(const Image *,const MorphologyMethod,const ssize_t,
    const KernelInfo *,ExceptionInfo *),
  *MorphologyImageChannel(const Image *,const ChannelType,
    const MorphologyMethod,const ssize_t,const KernelInfo *,ExceptionInfo *);

extern MagickExport void
  ScaleGeometryKernelInfo(KernelInfo *,const char *),
  ShowKernelInfo(const KernelInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

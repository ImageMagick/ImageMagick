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

  MagickCore image distortion methods.
*/
#ifndef MAGICKCORE_DISTORT_H
#define MAGICKCORE_DISTORT_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  These two enum are linked, with common enumerated values.  Both
  DistortImages() and SparseColor() often share code to determine functional
  coefficients for common methods.

  Caution should be taken to ensure that only the common methods contain the
  same enumerated value, while all others remain unique across both
  enumerations.
*/
typedef enum
{
  UndefinedDistortion,
  AffineDistortion,
  AffineProjectionDistortion,
  ScaleRotateTranslateDistortion,
  PerspectiveDistortion,
  PerspectiveProjectionDistortion,
  BilinearForwardDistortion,
  BilinearDistortion = BilinearForwardDistortion,
  BilinearReverseDistortion,
  PolynomialDistortion,
  ArcDistortion,
  PolarDistortion,
  DePolarDistortion,
  Cylinder2PlaneDistortion,
  Plane2CylinderDistortion,
  BarrelDistortion,
  BarrelInverseDistortion,
  ShepardsDistortion,
  ResizeDistortion,
  SentinelDistortion,
  RigidAffineDistortion
} DistortMethod;

typedef enum
{
  UndefinedColorInterpolate = UndefinedDistortion,
  BarycentricColorInterpolate = AffineDistortion,
  BilinearColorInterpolate = BilinearReverseDistortion,
  PolynomialColorInterpolate = PolynomialDistortion,
  ShepardsColorInterpolate = ShepardsDistortion,
  /*
    Methods unique to SparseColor().
  */
  VoronoiColorInterpolate = SentinelDistortion,
  InverseColorInterpolate,
  ManhattanColorInterpolate
} SparseColorMethod;

extern MagickExport Image
  *AffineTransformImage(const Image *,const AffineMatrix *,ExceptionInfo *),
  *DistortImage(const Image *,const DistortMethod,const size_t,
    const double *,MagickBooleanType,ExceptionInfo *exception),
  *DistortResizeImage(const Image *,const size_t,const size_t,ExceptionInfo *),
  *RotateImage(const Image *,const double,ExceptionInfo *),
  *SparseColorImage(const Image *,const SparseColorMethod,const size_t,
    const double *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

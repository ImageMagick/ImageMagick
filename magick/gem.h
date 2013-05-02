/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private graphic gems methods.
*/
#ifndef _MAGICKCORE_GEM_H
#define _MAGICKCORE_GEM_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/fx.h"
#include "magick/random_.h"

extern MagickExport double
  ExpandAffine(const AffineMatrix *),
  GenerateDifferentialNoise(RandomInfo *,const Quantum,const NoiseType,
    const MagickRealType);

extern MagickExport size_t
  GetOptimalKernelWidth(const double,const double),
  GetOptimalKernelWidth1D(const double,const double),
  GetOptimalKernelWidth2D(const double,const double);

extern MagickExport void
  ConvertHCLToRGB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ConvertHCLpToRGB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ConvertHSBToRGB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ConvertHSIToRGB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ConvertHSLToRGB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ConvertHSVToRGB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ConvertHWBToRGB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ConvertLCHabToRGB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ConvertLCHuvToRGB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ConvertRGBToHCL(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  ConvertRGBToHCLp(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  ConvertRGBToHSB(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  ConvertRGBToHSI(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  ConvertRGBToHSL(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  ConvertRGBToHSV(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  ConvertRGBToHWB(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  ConvertRGBToLCHab(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  ConvertRGBToLCHuv(const Quantum,const Quantum,const Quantum,double *,double *,
    double *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

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

  MagickCore private graphic gems methods.
*/
#ifndef MAGICKCORE_GEM_PRIVATE_H
#define MAGICKCORE_GEM_PRIVATE_H

#include "MagickCore/pixel-accessor.h"
#include "MagickCore/visual-effects.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define D65X  0.950456
#define D65Y  1.0
#define D65Z  1.088754
#define CIEEpsilon  (216.0/24389.0)
#define CIEK  (24389.0/27.0)

extern MagickPrivate double
  GenerateDifferentialNoise(RandomInfo *,const Quantum,const NoiseType,
    const double);

extern MagickPrivate size_t
  GetOptimalKernelWidth(const double,const double),
  GetOptimalKernelWidth1D(const double,const double),
  GetOptimalKernelWidth2D(const double,const double);

extern MagickPrivate void
  ConvertHCLToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertHCLpToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertHSBToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertHSIToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertHSVToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertHWBToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertLCHabToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertLCHuvToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToHCL(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToHCLp(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToHSB(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToHSI(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToHSV(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToHWB(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToLab(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToLCHab(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToLCHuv(const double,const double,const double,double *,double *,
    double *);

static inline void ConvertLabToXYZ(const double L,const double a,const double b,
  double *X,double *Y,double *Z)
{
  double
    x,
    y,
    z;

  assert(X != (double *) NULL);
  assert(Y != (double *) NULL);
  assert(Z != (double *) NULL);
  y=(L+16.0)/116.0;
  x=y+a/500.0;
  z=y-b/200.0;
  if ((x*x*x) > CIEEpsilon)
    x=(x*x*x);
  else
    x=(116.0*x-16.0)/CIEK;
  if ((y*y*y) > CIEEpsilon)
    y=(y*y*y);
  else
    y=L/CIEK;
  if ((z*z*z) > CIEEpsilon)
    z=(z*z*z);
  else
    z=(116.0*z-16.0)/CIEK;
  *X=D65X*x;
  *Y=D65Y*y;
  *Z=D65Z*z;
}

static inline void ConvertLuvToXYZ(const double L,const double u,const double v,
  double *X,double *Y,double *Z)
{
  double
    gamma;

  assert(X != (double *) NULL);
  assert(Y != (double *) NULL);
  assert(Z != (double *) NULL);
  if (L > (CIEK*CIEEpsilon))
    *Y=(double) pow((L+16.0)/116.0,3.0);
  else
    *Y=L/CIEK;
  gamma=PerceptibleReciprocal((((52.0*L/(u+13.0*L*(4.0*D65X/(D65X+15.0*D65Y+
    3.0*D65Z))))-1.0)/3.0)-(-1.0/3.0));
  *X=gamma*((*Y*((39.0*L/(v+13.0*L*(9.0*D65Y/(D65X+15.0*D65Y+3.0*D65Z))))-5.0))+
    5.0*(*Y));
  *Z=(*X*(((52.0*L/(u+13.0*L*(4.0*D65X/(D65X+15.0*D65Y+3.0*D65Z))))-1.0)/3.0))-
    5.0*(*Y);
}

static inline void ConvertRGBToXYZ(const double red,const double green,
  const double blue,double *X,double *Y,double *Z)
{
  double
    b,
    g,
    r;

  /*
    Convert RGB to XYZ colorspace.
  */
  assert(X != (double *) NULL);
  assert(Y != (double *) NULL);
  assert(Z != (double *) NULL);
  r=QuantumScale*DecodePixelGamma(red);
  g=QuantumScale*DecodePixelGamma(green);
  b=QuantumScale*DecodePixelGamma(blue);
  *X=0.4124564*r+0.3575761*g+0.1804375*b;
  *Y=0.2126729*r+0.7151522*g+0.0721750*b;
  *Z=0.0193339*r+0.1191920*g+0.9503041*b;
}

static inline void ConvertXYZToLab(const double X,const double Y,const double Z,
  double *L,double *a,double *b)
{
  double
    x,
    y,
    z;

  assert(L != (double *) NULL);
  assert(a != (double *) NULL);
  assert(b != (double *) NULL);
  if ((X/D65X) > CIEEpsilon)
    x=pow(X/D65X,1.0/3.0);
  else
    x=(CIEK*X/D65X+16.0)/116.0;
  if ((Y/D65Y) > CIEEpsilon)
    y=pow(Y/D65Y,1.0/3.0);
  else
    y=(CIEK*Y/D65Y+16.0)/116.0;
  if ((Z/D65Z) > CIEEpsilon)
    z=pow(Z/D65Z,1.0/3.0);
  else
    z=(CIEK*Z/D65Z+16.0)/116.0;
  *L=((116.0*y)-16.0)/100.0;
  *a=(500.0*(x-y))/255.0+0.5;
  *b=(200.0*(y-z))/255.0+0.5;
}

static inline void ConvertXYZToLuv(const double X,const double Y,const double Z,
  double *L,double *u,double *v)
{
  double
    alpha;

  assert(L != (double *) NULL);
  assert(u != (double *) NULL);
  assert(v != (double *) NULL);
  if ((Y/D65Y) > CIEEpsilon)
    *L=(double) (116.0*pow(Y/D65Y,1.0/3.0)-16.0);
  else
    *L=CIEK*(Y/D65Y);
  alpha=PerceptibleReciprocal(X+15.0*Y+3.0*Z);
  *u=13.0*(*L)*((4.0*alpha*X)-(4.0*D65X/(D65X+15.0*D65Y+3.0*D65Z)));
  *v=13.0*(*L)*((9.0*alpha*Y)-(9.0*D65Y/(D65X+15.0*D65Y+3.0*D65Z)));
  *L/=100.0;
  *u=(*u+134.0)/354.0;
  *v=(*v+140.0)/262.0;
}

static inline void ConvertXYZToRGB(const double X,const double Y,const double Z,
  double *red,double *green,double *blue)
{
  double
    b,
    g,
    r;

  assert(red != (double *) NULL);
  assert(green != (double *) NULL);
  assert(blue != (double *) NULL);
  r=3.2404542*X-1.5371385*Y-0.4985314*Z;
  g=(-0.9692660)*X+1.8760108*Y+0.0415560*Z;
  b=0.0556434*X-0.2040259*Y+1.0572252*Z;
  *red=EncodePixelGamma(QuantumRange*r);
  *green=EncodePixelGamma(QuantumRange*g);
  *blue=EncodePixelGamma(QuantumRange*b);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

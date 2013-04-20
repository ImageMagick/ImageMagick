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
#ifndef _MAGICKCORE_GEM_PRIVATE_H
#define _MAGICKCORE_GEM_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "MagickCore/pixel-private.h"

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
  ConvertHSBToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertHWBToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertLCHabToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertLCHuvToRGB(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToHCL(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToHSB(const double,const double,const double,double *,double *,
    double *),
  ConvertRGBToHWB(const double,const double,const double,double *,double *,
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
  y=(100.0*L+16.0)/116.0;
  x=y+255.0*(a-0.5)/500.0;
  z=y-255.0*(b-0.5)/200.0;
  if ((x*x*x) > CIEEpsilon)
    x=(x*x*x);
  else
    x=(116.0*x-16.0)/CIEK;
  if ((y*y*y) > CIEEpsilon)
    y=(y*y*y);
  else
    y=(100.0*L)/CIEK;
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
  assert(X != (double *) NULL);
  assert(Y != (double *) NULL);
  assert(Z != (double *) NULL);
  if ((100.0*L) > (CIEK*CIEEpsilon))
    *Y=(double) pow(((100.0*L)+16.0)/116.0,3.0);
  else
    *Y=(100.0*L)/CIEK;
  *X=((*Y*((39.0*(100.0*L)/((262.0*v-140.0)+13.0*(100.0*L)*(9.0*D65Y/
    (D65X+15.0*D65Y+3.0*D65Z))))-5.0))+5.0*(*Y))/((((52.0f*(100.0*L)/
    ((354.0*u-134.0)+13.0*(100.0*L)*(4.0*D65X/(D65X+15.0*D65Y+3.0*
    D65Z))))-1.0f)/3.0)-(-1.0f/3.0));
  *Z=(*X*(((52.0f*(100.0*L)/((354.0*u-134.0)+13.0*(100.0*L)*(4.0*D65X/
    (D65X+15.0*D65Y+3.0*D65Z))))-1.0f)/3.0))-5.0*(*Y);
}

static inline void ConvertRGBToXYZ(const double red,const double green,
  const double blue,double *X,double *Y,double *Z)
{
  double
    b,
    g,
    r;

  assert(X != (double *) NULL);
  assert(Y != (double *) NULL);
  assert(Z != (double *) NULL);
  r=QuantumScale*red;
  g=QuantumScale*green;
  b=QuantumScale*blue;
  *X=0.41239558896741421610*r+0.35758343076371481710*g+0.18049264738170157350*b;
  *Y=0.21258623078559555160*r+0.71517030370341084990*g+0.07220049864333622685*b;
  *Z=0.01929721549174694484*r+0.11918386458084853180*g+0.95049712513157976600*b;
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

static inline void ConvertXYZToRGB(const double x,const double y,const double z,
  double *red,double *green,double *blue)
{
  double
    b,
    g,
    r;

  /*
    Convert XYZ to RGB colorspace.
  */
  assert(red != (double *) NULL);
  assert(green != (double *) NULL);
  assert(blue != (double *) NULL);
  r=3.2406*x-1.5372*y-0.4986*z;
  g=(-0.9689*x+1.8758*y+0.0415*z);
  b=0.0557*x-0.2040*y+1.0570*z;
  *red=QuantumRange*r;
  *green=QuantumRange*g;
  *blue=QuantumRange*b;
}


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

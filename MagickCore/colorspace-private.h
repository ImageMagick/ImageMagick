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

  MagickCore image colorspace private methods.
*/
#ifndef MAGICKCORE_COLORSPACE_PRIVATE_H
#define MAGICKCORE_COLORSPACE_PRIVATE_H

#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"

#define IlluminantX  0.95047
#define IlluminantY  1.0
#define IlluminantZ  1.08883
#define CIEEpsilon  (216.0/24389.0)
#define CIEK  (24389.0/27.0)

static const PrimaryInfo
  illuminant_tristimulus[] =
  {
    { 1.09850, 1.00000, 0.35585 },  /* A */
    { 0.99072, 1.00000, 0.85223 },  /* B */
    { 0.98074, 1.00000, 1.18232 },  /* C */
    { 0.96422, 1.00000, 0.82521 },  /* D50 */
    { 0.95682, 1.00000, 0.92149 },  /* D55 */
    { 0.95047, 1.00000, 1.08883 },  /* D65 */
    { 0.94972, 1.00000, 1.22638 },  /* D75 */
    { 1.00000, 1.00000, 1.00000 },  /* E */
    { 0.99186, 1.00000, 0.67393 },  /* F2 */
    { 0.95041, 1.00000, 1.08747 },  /* F7 */
    { 1.00962, 1.00000, 0.64350 }   /* F11 */
  };

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif


static inline void ConvertAdobe98ToXYZ(const double red,const double green,
  const double blue,double *X,double *Y,double *Z)
{ 
  double
    b,
    g,
    r;

  /*
    Convert Adobe '98 to XYZ colorspace. 
  */
  r=QuantumScale*DecodePixelGamma((double) QuantumRange*red);
  g=QuantumScale*DecodePixelGamma((double) QuantumRange*green);
  b=QuantumScale*DecodePixelGamma((double) QuantumRange*blue);
  *X=0.57666904291013050*r+0.18555823790654630*g+0.18822864623499470*b;
  *Y=0.29734497525053605*r+0.62736356625546610*g+0.07529145849399788*b;
  *Z=0.02703136138641234*r+0.07068885253582723*g+0.99133753683763880*b;
} 

static inline void ConvertXYZToRGB(const double X,const double Y,const double Z,
  double *red,double *green,double *blue)
{
  double
    b,
    g,
    r;

  r=3.2404542*X-1.5371385*Y-0.4985314*Z;
  g=(-0.9692660)*X+1.8760108*Y+0.0415560*Z;
  b=0.0556434*X-0.2040259*Y+1.0572252*Z;
  *red=EncodePixelGamma((double) QuantumRange*r);
  *green=EncodePixelGamma((double) QuantumRange*g);
  *blue=EncodePixelGamma((double) QuantumRange*b);
}

static inline void ConvertAdobe98ToRGB(const double r,const double g,
  const double b,double *red,double *green,double *blue)
{
  double
    X,
    Y,
    Z;

  ConvertAdobe98ToXYZ(r,g,b,&X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertCMYKToRGB(PixelInfo *pixel)
{
  pixel->red=(((double) QuantumRange-(QuantumScale*pixel->red*
    ((double) QuantumRange-pixel->black)+pixel->black)));
  pixel->green=(((double) QuantumRange-(QuantumScale*pixel->green*
    ((double) QuantumRange-pixel->black)+pixel->black)));
  pixel->blue=(((double) QuantumRange-(QuantumScale*pixel->blue*
    ((double) QuantumRange-pixel->black)+pixel->black)));
}

static inline void ConvertCMYToRGB(const double cyan,const double magenta,
  const double yellow,double *red,double *green,double *blue)
{
	*red=(double) QuantumRange*(1.0-cyan);
  *green=(double) QuantumRange*(1.0-magenta);
  *blue=(double) QuantumRange*(1.0-yellow);
}

static inline void ConvertHCLToRGB(const double hue,const double chroma,
  const double luma,double *red,double *green,double *blue)
{
  double
    b,
    c,
    g,
    h,
    m,
    r,
    x;

  /*
    Convert HCL to RGB colorspace.
  */
  assert(red != (double *) NULL);
  assert(green != (double *) NULL);
  assert(blue != (double *) NULL);
  h=6.0*hue;
  c=chroma;
  x=c*(1.0-fabs(fmod(h,2.0)-1.0));
  r=0.0;
  g=0.0;
  b=0.0;
  if ((0.0 <= h) && (h < 1.0))
    {
      r=c;
      g=x;
    }
  else
    if ((1.0 <= h) && (h < 2.0))
      {
        r=x;
        g=c;
      }
    else
      if ((2.0 <= h) && (h < 3.0))
        {
          g=c;
          b=x;
        }
      else
        if ((3.0 <= h) && (h < 4.0))
          {
            g=x;
            b=c;
          }
        else
          if ((4.0 <= h) && (h < 5.0))
            {
              r=x;
              b=c;
            }
          else
            if ((5.0 <= h) && (h < 6.0))
              {
                r=c;
                b=x;
              }
  m=luma-(0.298839*r+0.586811*g+0.114350*b);
  *red=(double) QuantumRange*(r+m);
  *green=(double) QuantumRange*(g+m);
  *blue=(double) QuantumRange*(b+m);
}

static inline void ConvertHCLpToRGB(const double hue,const double chroma,
  const double luma,double *red,double *green,double *blue)
{
  double
    b,
    c,
    g,
    h,
    m,
    r,
    x,
    z;

  /*
    Convert HCLp to RGB colorspace.
  */
  assert(red != (double *) NULL);
  assert(green != (double *) NULL);
  assert(blue != (double *) NULL);
  h=6.0*hue;
  c=chroma;
  x=c*(1.0-fabs(fmod(h,2.0)-1.0));
  r=0.0;
  g=0.0;
  b=0.0;
  if ((0.0 <= h) && (h < 1.0))
    {
      r=c;
      g=x;
    }
  else
    if ((1.0 <= h) && (h < 2.0))
      {
        r=x;
        g=c;
      }
    else
      if ((2.0 <= h) && (h < 3.0))
        {
          g=c;
          b=x;
        }
      else
        if ((3.0 <= h) && (h < 4.0))
          {
            g=x;
            b=c;
          }
        else
          if ((4.0 <= h) && (h < 5.0))
            {
              r=x;
              b=c;
            }
          else
            if ((5.0 <= h) && (h < 6.0))
              {
                r=c;
                b=x;
              }
  m=luma-(0.298839*r+0.586811*g+0.114350*b);
  z=1.0;
  if (m < 0.0)
    {
      z=luma/(luma-m);
      m=0.0;
    }
  else
    if (m+c > 1.0)
      {
        z=(1.0-luma)/(m+c-luma);
        m=1.0-z*c;
      }
  *red=(double) QuantumRange*(z*r+m);
  *green=(double) QuantumRange*(z*g+m);
  *blue=(double) QuantumRange*(z*b+m);
}

static inline void ConvertHSBToRGB(const double hue,const double saturation,
  const double brightness,double *red,double *green,double *blue)
{
  double
    f,
    h,
    p,
    q,
    t;

  /*
    Convert HSB to RGB colorspace.
  */
  assert(red != (double *) NULL);
  assert(green != (double *) NULL);
  assert(blue != (double *) NULL);
  if (fabs(saturation) < MagickEpsilon)
    {
      *red=(double) QuantumRange*brightness;
      *green=(*red);
      *blue=(*red);
      return;
    }
  h=6.0*(hue-floor(hue));
  f=h-floor((double) h);
  p=brightness*(1.0-saturation);
  q=brightness*(1.0-saturation*f);
  t=brightness*(1.0-(saturation*(1.0-f)));
  switch ((int) h)
  {
    case 0:
    default:
    {
      *red=(double) QuantumRange*brightness;
      *green=(double) QuantumRange*t;
      *blue=(double) QuantumRange*p;
      break;
    }
    case 1:
    {
      *red=(double) QuantumRange*q;
      *green=(double) QuantumRange*brightness;
      *blue=(double) QuantumRange*p;
      break;
    }
    case 2:
    {
      *red=(double) QuantumRange*p;
      *green=(double) QuantumRange*brightness;
      *blue=(double) QuantumRange*t;
      break;
    }
    case 3:
    {
      *red=(double) QuantumRange*p;
      *green=(double) QuantumRange*q;
      *blue=(double) QuantumRange*brightness;
      break;
    }
    case 4:
    {
      *red=(double) QuantumRange*t;
      *green=(double) QuantumRange*p;
      *blue=(double) QuantumRange*brightness;
      break;
    }
    case 5:
    {
      *red=(double) QuantumRange*brightness;
      *green=(double) QuantumRange*p;
      *blue=(double) QuantumRange*q;
      break;
    }
  }
}

static inline void ConvertHSIToRGB(const double hue,const double saturation,
  const double intensity,double *red,double *green,double *blue)
{
  double
    b,
    g,
    h,
    r;

  /*
    Convert HSI to RGB colorspace.
  */
  assert(red != (double *) NULL);
  assert(green != (double *) NULL);
  assert(blue != (double *) NULL);
  h=360.0*hue;
  h-=360.0*floor(h/360.0);
  if (h < 120.0)
    {
      b=intensity*(1.0-saturation);
      r=intensity*(1.0+saturation*cos(h*(MagickPI/180.0))/cos((60.0-h)*
        (MagickPI/180.0)));
      g=3.0*intensity-r-b;
    }
  else
    if (h < 240.0)
      {
        h-=120.0;
        r=intensity*(1.0-saturation);
        g=intensity*(1.0+saturation*cos(h*(MagickPI/180.0))/cos((60.0-h)*
          (MagickPI/180.0)));
        b=3.0*intensity-r-g;
      }
    else
      {
        h-=240.0;
        g=intensity*(1.0-saturation);
        b=intensity*(1.0+saturation*cos(h*(MagickPI/180.0))/cos((60.0-h)*
          (MagickPI/180.0)));
        r=3.0*intensity-g-b;
      }
  *red=(double) QuantumRange*r;
  *green=(double) QuantumRange*g;
  *blue=(double) QuantumRange*b;
}

static inline void ConvertHSVToRGB(const double hue,const double saturation,
  const double value,double *red,double *green,double *blue)
{
  double
    c,
    h,
    min,
    x;

  /*
    Convert HSV to RGB colorspace.
  */
  assert(red != (double *) NULL);
  assert(green != (double *) NULL);
  assert(blue != (double *) NULL);
  h=hue*360.0;
  c=value*saturation;
  min=value-c;
  h-=360.0*floor(h/360.0);
  h/=60.0;
  x=c*(1.0-fabs(h-2.0*floor(h/2.0)-1.0));
  switch ((int) floor(h))
  {
    case 0:
    default:
    {
      *red=(double) QuantumRange*(min+c);
      *green=(double) QuantumRange*(min+x);
      *blue=(double) QuantumRange*min;
      break;
    }
    case 1:
    {
      *red=(double) QuantumRange*(min+x);
      *green=(double) QuantumRange*(min+c);
      *blue=(double) QuantumRange*min;
      break;
    }
    case 2:
    {
      *red=(double) QuantumRange*min;
      *green=(double) QuantumRange*(min+c);
      *blue=(double) QuantumRange*(min+x);
      break;
    }
    case 3:
    {
      *red=(double) QuantumRange*min;
      *green=(double) QuantumRange*(min+x);
      *blue=(double) QuantumRange*(min+c);
      break;
    }
    case 4:
    {
      *red=(double) QuantumRange*(min+x);
      *green=(double) QuantumRange*min;
      *blue=(double) QuantumRange*(min+c);
      break;
    }
    case 5:
    {
      *red=(double) QuantumRange*(min+c);
      *green=(double) QuantumRange*min;
      *blue=(double) QuantumRange*(min+x);
      break;
    }
  }
}

static inline void ConvertHWBToRGB(const double hue,const double whiteness,
  const double blackness,double *red,double *green,double *blue)
{
  double
    b,
    f,
    g,
    n,
    r,
    v;

  ssize_t
    i;

  /*
    Convert HWB to RGB colorspace.
  */
  assert(red != (double *) NULL);
  assert(green != (double *) NULL);
  assert(blue != (double *) NULL);
  v=1.0-blackness;
  if (fabs(hue-(-1.0)) < MagickEpsilon)
    {
      *red=(double) QuantumRange*v;
      *green=(double) QuantumRange*v;
      *blue=(double) QuantumRange*v;
      return;
    }
  i=CastDoubleToLong(floor(6.0*hue));
  f=6.0*hue-i;
  if ((i & 0x01) != 0)
    f=1.0-f;
  n=whiteness+f*(v-whiteness);  /* linear interpolation */
  switch (i)
  {
    case 0:
    default: r=v; g=n; b=whiteness; break;
    case 1: r=n; g=v; b=whiteness; break;
    case 2: r=whiteness; g=v; b=n; break;
    case 3: r=whiteness; g=n; b=v; break;
    case 4: r=n; g=whiteness; b=v; break;
    case 5: r=v; g=whiteness; b=n; break;
  }
  *red=(double) QuantumRange*r;
  *green=(double) QuantumRange*g;
  *blue=(double) QuantumRange*b;
}

static inline void ConvertLabToXYZ(const double L,const double a,const double b,
  const IlluminantType illuminant,double *X,double *Y,double *Z)
{   
  double
    x,
    y,
    z;
    
  y=(L+16.0)/116.0;
  x=y+a/500.0;
  z=y-b/200.0;
  if ((x*x*x) > CIEEpsilon)
    x=(x*x*x);
  else
    x=(116.0*x-16.0)/CIEK;
  if (L > (CIEK*CIEEpsilon))
    y=(y*y*y);
  else
    y=L/CIEK;
  if ((z*z*z) > CIEEpsilon)
    z=(z*z*z);
  else
    z=(116.0*z-16.0)/CIEK;
  *X=illuminant_tristimulus[illuminant].x*x;
  *Y=illuminant_tristimulus[illuminant].y*y;
  *Z=illuminant_tristimulus[illuminant].z*z;
}

static inline void ConvertLabToRGB(const double L,const double a,
  const double b,const IlluminantType illuminant,double *red,double *green,
  double *blue)
{
  double
    X,
    Y,
    Z;

  ConvertLabToXYZ(100.0*L,255.0*(a-0.5),255.0*(b-0.5),illuminant,&X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertLCHabToXYZ(const double luma,const double chroma,
  const double hue,const IlluminantType illuminant,double *X,double *Y,
  double *Z)
{
  ConvertLabToXYZ(luma,chroma*cos(DegreesToRadians(hue)),chroma*
    sin(DegreesToRadians(hue)),illuminant,X,Y,Z);
}

static inline void ConvertLCHabToRGB(const double luma,const double chroma,
  const double hue,const IlluminantType illuminant,double *red,double *green,
  double *blue)
{
  double
    X,
    Y,
    Z;

  /*
    Convert LCHab to RGB colorspace.
  */
  assert(red != (double *) NULL);
  assert(green != (double *) NULL);
  assert(blue != (double *) NULL);
  ConvertLCHabToXYZ(100.0*luma,255.0*(chroma-0.5),360.0*hue,illuminant,
    &X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertLuvToXYZ(const double L,const double u,const double v,
  const IlluminantType illuminant,double *X,double *Y,double *Z)
{
  double
    gamma;

  if (L > (CIEK*CIEEpsilon))
    *Y=(double) pow((L+16.0)/116.0,3.0);
  else
    *Y=L/CIEK;
  gamma=PerceptibleReciprocal((((52.0*L*PerceptibleReciprocal(u+13.0*L*
    (4.0*illuminant_tristimulus[illuminant].x/
    (illuminant_tristimulus[illuminant].x+15.0*
    illuminant_tristimulus[illuminant].y+3.0*
    illuminant_tristimulus[illuminant].z))))-1.0)/3.0)-(-1.0/3.0));
  *X=gamma*((*Y*((39.0*L*PerceptibleReciprocal(v+13.0*L*(9.0*
    illuminant_tristimulus[illuminant].y/
    (illuminant_tristimulus[illuminant].x+15.0*
    illuminant_tristimulus[illuminant].y+3.0*
    illuminant_tristimulus[illuminant].z))))-5.0))+5.0*(*Y));
  *Z=(*X*(((52.0*L*PerceptibleReciprocal(u+13.0*L*(4.0*
    illuminant_tristimulus[illuminant].x/
    (illuminant_tristimulus[illuminant].x+15.0*
    illuminant_tristimulus[illuminant].y+3.0*
    illuminant_tristimulus[illuminant].z))))-1.0)/3.0))-5.0*(*Y);
}

static inline void ConvertLCHuvToXYZ(const double luma,const double chroma,
  const double hue,const IlluminantType illuminant,double *X,double *Y,
  double *Z)
{
  ConvertLuvToXYZ(luma,chroma*cos(DegreesToRadians(hue)),chroma*
    sin(DegreesToRadians(hue)),illuminant,X,Y,Z);
}

static inline void ConvertLCHuvToRGB(const double luma,const double chroma,
  const double hue,const IlluminantType illuminant,double *red,double *green,
  double *blue)
{
  double
    X,
    Y,
    Z;

  /*
    Convert LCHuv to RGB colorspace.
  */
  assert(red != (double *) NULL);
  assert(green != (double *) NULL);
  assert(blue != (double *) NULL);
  ConvertLCHuvToXYZ(100.0*luma,255.0*(chroma-0.5),360.0*hue,illuminant,
    &X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertLMSToXYZ(const double L,const double M,const double S,
  double *X,double *Y,double *Z)
{
  *X=1.096123820835514*L-0.278869000218287*M+0.182745179382773*S;
  *Y=0.454369041975359*L+0.473533154307412*M+0.072097803717229*S;
  *Z=(-0.009627608738429)*L-0.005698031216113*M+1.015325639954543*S;
}

static inline void ConvertLMSToRGB(const double L,const double M,
  const double S,double *red,double *green,double *blue)
{
  double
    X,
    Y,
    Z;

  ConvertLMSToXYZ(L,M,S,&X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertDisplayP3ToXYZ(const double red,const double green,
  const double blue,double *X,double *Y,double *Z)
{ 
  double
    b,
    g,
    r;

  /*
    Convert Display P3 to XYZ colorspace.
  */
  r=QuantumScale*DecodePixelGamma((double) QuantumRange*red);
  g=QuantumScale*DecodePixelGamma((double) QuantumRange*green);
  b=QuantumScale*DecodePixelGamma((double) QuantumRange*blue);
  *X=0.4865709486482162*r+0.26566769316909306*g+0.1982172852343625*b;
  *Y=0.2289745640697488*r+0.69173852183650640*g+0.0792869140937450*b;
  *Z=0.0000000000000000*r+0.04511338185890264*g+1.0439443689009760*b;
}

static inline void ConvertDisplayP3ToRGB(const double r,const double g,
  const double b,double *red,double *green,double *blue)
{
  double
    X,
    Y,
    Z;

  ConvertDisplayP3ToXYZ(r,g,b,&X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertLuvToRGB(const double L,const double u,
  const double v,const IlluminantType illuminant,double *red,double *green,
  double *blue)
{
  double
    X,
    Y,
    Z;

  ConvertLuvToXYZ(100.0*L,354.0*u-134.0,262.0*v-140.0,illuminant,&X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertProPhotoToXYZ(const double red,const double green,
  const double blue,double *X,double *Y,double *Z)
{   
  double
    b,
    g,
    r;
  
  /*
    Convert ProPhoto to XYZ colorspace.
  */
  r=QuantumScale*DecodePixelGamma((double) QuantumRange*red);
  g=QuantumScale*DecodePixelGamma((double) QuantumRange*green);
  b=QuantumScale*DecodePixelGamma((double) QuantumRange*blue);
  *X=0.4865709486482162*r+0.26566769316909306*g+0.1982172852343625*b;
  *X=0.7977604896723027*r+0.13518583717574031*g+0.03134934958152480000*b;
  *Y=0.2880711282292934*r+0.71184321781010140*g+0.00008565396060525902*b;
  *Z=0.0000000000000000*r+0.00000000000000000*g+0.82510460251046010000*b;
}

static inline void ConvertProPhotoToRGB(const double r,const double g,
  const double b,double *red,double *green,double *blue)
{
  double
    X,
    Y,
    Z;

  ConvertProPhotoToXYZ(r,g,b,&X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertRGBToCMY(const double red,const double green,
  const double blue,double *cyan,double *magenta,double *yellow)
{
  *cyan=QuantumScale*((double) QuantumRange-red);
  *magenta=QuantumScale*((double) QuantumRange-green);
  *yellow=QuantumScale*((double) QuantumRange-blue);
}

static inline void ConvertRGBToHCL(const double red,const double green,
  const double blue,double *hue,double *chroma,double *luma)
{
  double
    c,
    h,
    max;

  /*
    Convert RGB to HCL colorspace.
  */
  assert(hue != (double *) NULL);
  assert(chroma != (double *) NULL);
  assert(luma != (double *) NULL);
  max=MagickMax(red,MagickMax(green,blue));
  c=max-(double) MagickMin(red,MagickMin(green,blue));
  h=0.0;
  if (fabs(c) < MagickEpsilon)
    h=0.0;
  else
    if (fabs(red-max) < MagickEpsilon)
      h=fmod((green-blue)/c+6.0,6.0);
    else
      if (fabs(green-max) < MagickEpsilon)
        h=((blue-red)/c)+2.0;
      else
        if (fabs(blue-max) < MagickEpsilon)
          h=((red-green)/c)+4.0;
  *hue=(h/6.0);
  *chroma=QuantumScale*c;
  *luma=QuantumScale*(0.298839*red+0.586811*green+0.114350*blue);
}

static inline void ConvertRGBToHCLp(const double red,const double green,
  const double blue,double *hue,double *chroma,double *luma)
{
  double
    c,
    h,
    max;

  /*
    Convert RGB to HCL colorspace.
  */
  assert(hue != (double *) NULL);
  assert(chroma != (double *) NULL);
  assert(luma != (double *) NULL);
  max=MagickMax(red,MagickMax(green,blue));
  c=max-MagickMin(red,MagickMin(green,blue));
  h=0.0;
  if (fabs(c) < MagickEpsilon)
    h=0.0;
  else
    if (fabs(red-max) < MagickEpsilon)
      h=fmod((green-blue)/c+6.0,6.0);
    else
      if (fabs(green-max) < MagickEpsilon)
        h=((blue-red)/c)+2.0;
      else
        if (fabs(blue-max) < MagickEpsilon)
          h=((red-green)/c)+4.0;
  *hue=(h/6.0);
  *chroma=QuantumScale*c;
  *luma=QuantumScale*(0.298839*red+0.586811*green+0.114350*blue);
}

static inline void ConvertRGBToHSB(const double red,const double green,
  const double blue,double *hue,double *saturation,double *brightness)
{
  double
    delta,
    max,
    min;

  /*
    Convert RGB to HSB colorspace.
  */
  assert(hue != (double *) NULL);
  assert(saturation != (double *) NULL);
  assert(brightness != (double *) NULL);
  *hue=0.0;
  *saturation=0.0;
  *brightness=0.0;
  min=red < green ? red : green;
  if (blue < min)
    min=blue;
  max=red > green ? red : green;
  if (blue > max)
    max=blue;
  if (fabs(max) < MagickEpsilon)
    return;
  delta=max-min;
  *saturation=delta/max;
  *brightness=QuantumScale*max;
  if (fabs(delta) < MagickEpsilon)
    return;
  if (fabs(red-max) < MagickEpsilon)
    *hue=(green-blue)/delta;
  else
    if (fabs(green-max) < MagickEpsilon)
      *hue=2.0+(blue-red)/delta;
    else
      *hue=4.0+(red-green)/delta;
  *hue/=6.0;
  if (*hue < 0.0)
    *hue+=1.0;
}

static inline void ConvertRGBToHSI(const double red,const double green,
  const double blue,double *hue,double *saturation,double *intensity)
{
  double
    alpha,
    beta;

  /*
    Convert RGB to HSI colorspace.
  */
  assert(hue != (double *) NULL);
  assert(saturation != (double *) NULL);
  assert(intensity != (double *) NULL);
  *intensity=(QuantumScale*red+QuantumScale*green+QuantumScale*blue)/3.0;
  if (*intensity <= 0.0)
    {
      *hue=0.0;
      *saturation=0.0;
      return;
    }
  *saturation=1.0-MagickMin(QuantumScale*red,MagickMin(QuantumScale*green,
    QuantumScale*blue))/(*intensity);
  alpha=0.5*(2.0*QuantumScale*red-QuantumScale*green-QuantumScale*blue);
  beta=0.8660254037844385*(QuantumScale*green-QuantumScale*blue);
  *hue=atan2(beta,alpha)*(180.0/MagickPI)/360.0;
  if (*hue < 0.0)
    *hue+=1.0;
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
  r=QuantumScale*DecodePixelGamma(red);
  g=QuantumScale*DecodePixelGamma(green);
  b=QuantumScale*DecodePixelGamma(blue);
  *X=0.4124564*r+0.3575761*g+0.1804375*b;
  *Y=0.2126729*r+0.7151522*g+0.0721750*b;
  *Z=0.0193339*r+0.1191920*g+0.9503041*b;
}

static inline void ConvertXYZToAdobe98(const double X,const double Y,
  const double Z,double *red,double *green,double *blue)
{
  double
    b,
    g,
    r;

  r=2.041587903810746500*X-0.56500697427885960*Y-0.34473135077832956*Z;
  g=(-0.969243636280879500)*X+1.87596750150772020*Y+0.04155505740717557*Z;
  b=0.013444280632031142*X-0.11836239223101838*Y+1.01517499439120540*Z;
  *red=QuantumScale*EncodePixelGamma((double) QuantumRange*r);
  *green=QuantumScale*EncodePixelGamma((double) QuantumRange*g);
  *blue=QuantumScale*EncodePixelGamma((double) QuantumRange*b);
}

static inline void ConvertRGBToAdobe98(const double red,const double green,
  const double blue,double *r,double *g,double *b)
{
  double
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToAdobe98(X,Y,Z,r,g,b);
}

static inline void ConvertXYZToDisplayP3(const double X,const double Y,
  const double Z,double *red,double *green,double *blue)
{
  double
    b,
    g,
    r;

  r=2.49349691194142500*X-0.93138361791912390*Y-0.402710784450716840*Z;
  g=(-0.82948896956157470)*X+1.76266406031834630*Y+0.023624685841943577*Z;
  b=0.03584583024378447*X-0.07617238926804182*Y+0.956884524007687200*Z;
  *red=QuantumScale*EncodePixelGamma((double) QuantumRange*r);
  *green=QuantumScale*EncodePixelGamma((double) QuantumRange*g);
  *blue=QuantumScale*EncodePixelGamma((double) QuantumRange*b);
}

static inline void ConvertRGBToDisplayP3(const double red,const double green,
  const double blue,double *r,double *g,double *b)
{
  double
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToDisplayP3(X,Y,Z,r,g,b);
}

static inline void ConvertRGBToHSV(const double red,const double green,
  const double blue,double *hue,double *saturation,double *value)
{
  double
    c,
    max,
    min;

  /*
    Convert RGB to HSV colorspace.
  */
  assert(hue != (double *) NULL);
  assert(saturation != (double *) NULL);
  assert(value != (double *) NULL);
  max=MagickMax(QuantumScale*red,MagickMax(QuantumScale*green,
    QuantumScale*blue));
  min=MagickMin(QuantumScale*red,MagickMin(QuantumScale*green,
    QuantumScale*blue));
  c=max-min;
  *value=max;
  if (c <= 0.0)
    {
      *hue=0.0;
      *saturation=0.0;
      return;
    }
  if (fabs(max-QuantumScale*red) < MagickEpsilon)
    {
      *hue=(QuantumScale*green-QuantumScale*blue)/c;
      if ((QuantumScale*green) < (QuantumScale*blue))
        *hue+=6.0;
    }
  else
    if (fabs(max-QuantumScale*green) < MagickEpsilon)
      *hue=2.0+(QuantumScale*blue-QuantumScale*red)/c;
    else
      *hue=4.0+(QuantumScale*red-QuantumScale*green)/c;
  *hue*=60.0/360.0;
  *saturation=c*PerceptibleReciprocal(max);
}

static inline void ConvertRGBToHWB(const double red,const double green,
  const double blue,double *hue,double *whiteness,double *blackness)
{
  double
    f,
    p,
    v,
    w;

  /*
    Convert RGB to HWB colorspace.
  */
  assert(hue != (double *) NULL);
  assert(whiteness != (double *) NULL);
  assert(blackness != (double *) NULL);
  w=MagickMin(red,MagickMin(green,blue));
  v=MagickMax(red,MagickMax(green,blue));
  *blackness=1.0-QuantumScale*v;
  *whiteness=QuantumScale*w;
  if (fabs(v-w) < MagickEpsilon)
    {
      *hue=(-1.0);
      return;
    }
  f=(fabs(red-w) < MagickEpsilon) ? green-blue :
    ((fabs(green-w) < MagickEpsilon) ? blue-red : red-green);
  p=(fabs(red-w) < MagickEpsilon) ? 3.0 :
    ((fabs(green-w) < MagickEpsilon) ? 5.0 : 1.0);
  *hue=(p-f/(v-1.0*w))/6.0;
}

static inline void ConvertXYZToLab(const double X,const double Y,const double Z,
  const IlluminantType illuminant,double *L,double *a,double *b)
{
  double
    x,
    y,
    z;

  if ((X/illuminant_tristimulus[illuminant].x) > CIEEpsilon)
    x=pow(X/illuminant_tristimulus[illuminant].x,1.0/3.0);
  else
    x=(CIEK*X/illuminant_tristimulus[illuminant].x+16.0)/116.0;
  if ((Y/illuminant_tristimulus[illuminant].y) > CIEEpsilon)
    y=pow(Y/illuminant_tristimulus[illuminant].y,1.0/3.0);
  else
    y=(CIEK*Y/illuminant_tristimulus[illuminant].y+16.0)/116.0;
  if ((Z/illuminant_tristimulus[illuminant].z) > CIEEpsilon)
    z=pow(Z/illuminant_tristimulus[illuminant].z,1.0/3.0);
  else
    z=(CIEK*Z/illuminant_tristimulus[illuminant].z+16.0)/116.0;
  *L=((116.0*y)-16.0)/100.0;
  *a=(500.0*(x-y))/255.0+0.5;
  *b=(200.0*(y-z))/255.0+0.5;
}

static inline void ConvertRGBToLab(const double red,const double green,
  const double blue,const IlluminantType illuminant,double *L,double *a,
  double *b)
{
  double
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToLab(X,Y,Z,illuminant,L,a,b);
}

static inline void ConvertXYZToLCHab(const double X,const double Y,
  const double Z,const IlluminantType illuminant,double *luma,double *chroma,
  double *hue)
{
  double
    a,
    b;

  ConvertXYZToLab(X,Y,Z,illuminant,luma,&a,&b);
  *chroma=hypot(a-0.5,b-0.5)/1.0+0.5;
  *hue=180.0*atan2(b-0.5,a-0.5)/MagickPI/360.0;
  if (*hue < 0.0)
    *hue+=1.0;
}

static inline void ConvertRGBToLCHab(const double red,const double green,
  const double blue,const IlluminantType illuminant,double *luma,double *chroma,
  double *hue)
{
  double
    X,
    Y,
    Z;

  /*
    Convert RGB to LCHab colorspace.
  */
  assert(luma != (double *) NULL);
  assert(chroma != (double *) NULL);
  assert(hue != (double *) NULL);
  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToLCHab(X,Y,Z,illuminant,luma,chroma,hue);
}

static inline void ConvertXYZToLuv(const double X,const double Y,const double Z,
  const IlluminantType illuminant,double *L,double *u,double *v)
{
  double
    alpha;

  if ((Y/illuminant_tristimulus[illuminant].y) > CIEEpsilon)
    *L=(double) (116.0*pow(Y/illuminant_tristimulus[illuminant].y,
      1.0/3.0)-16.0);
  else
    *L=CIEK*(Y/illuminant_tristimulus[illuminant].y);
  alpha=PerceptibleReciprocal(X+15.0*Y+3.0*Z);
  *u=13.0*(*L)*((4.0*alpha*X)-(4.0*illuminant_tristimulus[illuminant].x/
    (illuminant_tristimulus[illuminant].x+15.0*
    illuminant_tristimulus[illuminant].y+3.0*
    illuminant_tristimulus[illuminant].z)));
  *v=13.0*(*L)*((9.0*alpha*Y)-(9.0*illuminant_tristimulus[illuminant].y/
    (illuminant_tristimulus[illuminant].x+15.0*
    illuminant_tristimulus[illuminant].y+3.0*
    illuminant_tristimulus[illuminant].z)));
  *L/=100.0;
  *u=(*u+134.0)/354.0;
  *v=(*v+140.0)/262.0;
}

static inline void ConvertXYZToLCHuv(const double X,const double Y,
  const double Z,const IlluminantType illuminant,double *luma,double *chroma,
  double *hue)
{
  double
    u,
    v;

  ConvertXYZToLuv(X,Y,Z,illuminant,luma,&u,&v);
  *chroma=hypot(354.0*u-134.0,262.0*v-140.0)/255.0+0.5;
  *hue=180.0*atan2(262.0*v-140.0,354.0*u-134.0)/MagickPI/360.0;
  if (*hue < 0.0)
    *hue+=1.0;
}

static inline void ConvertRGBToLCHuv(const double red,const double green,
  const double blue,const IlluminantType illuminant,double *luma,double *chroma,
  double *hue)
{
  double
    X,
    Y,
    Z;

  /*
    Convert RGB to LCHuv colorspace.
  */
  assert(luma != (double *) NULL);
  assert(chroma != (double *) NULL);
  assert(hue != (double *) NULL);
  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToLCHuv(X,Y,Z,illuminant,luma,chroma,hue);
}

static inline void ConvertXYZToProPhoto(const double X,const double Y,
  const double Z,double *red,double *green,double *blue)
{
  double
    b,
    g,
    r;

  r=1.3457989731028281*X-0.25558010007997534*Y-0.05110628506753401*Z;
  g=(-0.5446224939028347)*X+1.50823274131327810*Y+0.02053603239147973*Z;
  b=0.0000000000000000*X+0.0000000000000000*Y+1.21196754563894540*Z;
  *red=QuantumScale*EncodePixelGamma((double) QuantumRange*r);
  *green=QuantumScale*EncodePixelGamma((double) QuantumRange*g);
  *blue=QuantumScale*EncodePixelGamma((double) QuantumRange*b);
}

static inline void ConvertRGBToProPhoto(const double red,const double green,
  const double blue,double *r,double *g,double *b)
{
  double
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToProPhoto(X,Y,Z,r,g,b);
}

static inline void ConvertXYZToLMS(const double x,const double y,
  const double z,double *L,double *M,double *S)
{
  *L=0.7328*x+0.4296*y-0.1624*z;
  *M=(-0.7036*x+1.6975*y+0.0061*z);
  *S=0.0030*x+0.0136*y+0.9834*z;
}

static inline void ConvertRGBToLMS(const double red,const double green,
  const double blue,double *L,double *M,double *S)
{
  double
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToLMS(X,Y,Z,L,M,S);
}

static inline void ConvertRGBToLuv(const double red,const double green,
  const double blue,const IlluminantType illuminant,double *L,double *u,
  double *v)
{
  double
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  ConvertXYZToLuv(X,Y,Z,illuminant,L,u,v);
}

static inline void ConvertRGBToxyY(const double red,const double green,
  const double blue,double *low_x,double *low_y,double *cap_Y)
{
  double
    gamma,
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,green,blue,&X,&Y,&Z);
  gamma=PerceptibleReciprocal(X+Y+Z);
  *low_x=gamma*X;
  *low_y=gamma*Y;
  *cap_Y=Y;
}

static inline void ConvertXYZToJzazbz(const double X,const double Y,
  const double Z,const double white_luminance,double *Jz,double *az,double *bz)
{
#define Jzazbz_b  1.15  /* https://observablehq.com/@jrus/jzazbz */
#define Jzazbz_g  0.66
#define Jzazbz_c1  (3424.0/4096.0)
#define Jzazbz_c2  (2413.0/128.0)
#define Jzazbz_c3  (2392.0/128.0)
#define Jzazbz_n  (2610.0/16384.0)
#define Jzazbz_p  (1.7*2523.0/32.0)
#define Jzazbz_d  (-0.56)
#define Jzazbz_d0  (1.6295499532821566e-11)

  double
    gamma,
    Iz,
    L,
    Lp,
    M,
    Mp,
    S,
    Sp,
    Xp,
    Yp,
    Zp;

  Xp=(Jzazbz_b*X-(Jzazbz_b-1)*Z);
  Yp=(Jzazbz_g*Y-(Jzazbz_g-1)*X);
  Zp=Z;
  L=0.41478972*Xp+0.579999*Yp+0.0146480*Zp;
  M=(-0.2015100)*Xp+1.120649*Yp+0.0531008*Zp;
  S=(-0.0166008)*Xp+0.264800*Yp+0.6684799*Zp;
  gamma=pow(L*PerceptibleReciprocal(white_luminance),Jzazbz_n);
  Lp=pow((Jzazbz_c1+Jzazbz_c2*gamma)/(1.0+Jzazbz_c3*gamma),Jzazbz_p);
  gamma=pow(M*PerceptibleReciprocal(white_luminance),Jzazbz_n);
  Mp=pow((Jzazbz_c1+Jzazbz_c2*gamma)/(1.0+Jzazbz_c3*gamma),Jzazbz_p);
  gamma=pow(S*PerceptibleReciprocal(white_luminance),Jzazbz_n);
  Sp=pow((Jzazbz_c1+Jzazbz_c2*gamma)/(1.0+Jzazbz_c3*gamma),Jzazbz_p);
  Iz=0.5*Lp+0.5*Mp;
  *az=3.52400*Lp-4.066708*Mp+0.542708*Sp+0.5;
  *bz=0.199076*Lp+1.096799*Mp-1.295875*Sp+0.5;
  *Jz=((Jzazbz_d+1.0)*Iz)/(Jzazbz_d*Iz+1.0)-Jzazbz_d0;
}

static inline void ConvertJzazbzToXYZ(const double Jz,const double az,
  const double bz,const double white_luminance,double *X,double *Y,double *Z)
{
  double
    azz,
    bzz,
    gamma,
    Iz,
    L,
    Lp,
    M,
    Mp,
    S,
    Sp,
    Xp,
    Yp,
    Zp;

  gamma=Jz+Jzazbz_d0;
  Iz=gamma/(Jzazbz_d-Jzazbz_d*gamma+1.0);
  azz=az-0.5;
  bzz=bz-0.5;
  Lp=Iz+0.138605043271539*azz+0.0580473161561189*bzz;
  Mp=Iz-0.138605043271539*azz-0.0580473161561189*bzz;
  Sp=Iz-0.0960192420263189*azz-0.811891896056039*bzz;
  gamma=pow(Lp,1.0/Jzazbz_p);
  L=white_luminance*pow((Jzazbz_c1-gamma)/(Jzazbz_c3*gamma-Jzazbz_c2),1.0/
    Jzazbz_n);
  gamma=pow(Mp,1.0/Jzazbz_p);
  M=white_luminance*pow((Jzazbz_c1-gamma)/(Jzazbz_c3*gamma-Jzazbz_c2),1.0/
    Jzazbz_n);
  gamma=pow(Sp,1.0/Jzazbz_p);
  S=white_luminance*pow((Jzazbz_c1-gamma)/(Jzazbz_c3*gamma-Jzazbz_c2),1.0/
    Jzazbz_n);
  Xp=1.92422643578761*L-1.00479231259537*M+0.037651404030618*S;
  Yp=0.350316762094999*L+0.726481193931655*M-0.065384422948085*S;
  Zp=(-0.0909828109828476)*L-0.312728290523074*M+1.52276656130526*S;
  *X=(Xp+(Jzazbz_b-1.0)*Zp)/Jzazbz_b;
  *Y=(Yp+(Jzazbz_g-1.0)**X)/Jzazbz_g;
  *Z=Zp;
}

static inline void ConvertRGBToJzazbz(const double red,const double green,
  const double blue,const double white_luminance,double *Jz,double *az,
  double *bz)
{
  double
    X,
    Y,
    Z;

  ConvertRGBToXYZ(red,blue,green,&X,&Y,&Z);
  ConvertXYZToJzazbz(X,Y,Z,white_luminance,Jz,az,bz);
}

static inline void ConvertJzazbzToRGB(const double Jz,const double az,
  const double bz,const double white_luminance,double *red,double *green,
  double *blue)
{
  double
    X,
    Y,
    Z;

  ConvertJzazbzToXYZ(Jz,az,bz,white_luminance,&X,&Y,&Z);
  ConvertXYZToRGB(X,Y,Z,red,blue,green);
}

static inline void ConvertOklabToRGB(const double L,const double a,
  const double b,double *red,double *green,double *blue)
{
  double
    B,
    G,
    l,
    m,
    R,
    s;

  l=L+0.3963377774*(a-0.5)+0.2158037573*(b-0.5);
  m=L-0.1055613458*(a-0.5)-0.0638541728*(b-0.5);
  s=L-0.0894841775*(a-0.5)-1.2914855480*(b-0.5);
  l*=l*l;
  m*=m*m;
  s*=s*s;
  R=4.0767416621*l-3.3077115913*m+0.2309699292*s;
  G=(-1.2684380046)*l+2.6097574011*m-0.3413193965*s;
  B=(-0.0041960863)*l-0.7034186147*m+1.7076147010*s;
  *red=EncodePixelGamma((double) QuantumRange*R);
  *green=EncodePixelGamma((double) QuantumRange*G);
  *blue=EncodePixelGamma((double) QuantumRange*B);
}

static inline void ConvertRGBToOklab(const double red,const double green,
  const double blue,double *L,double *a,double *b)
{
  double
    B,
    G,
    l,
    m,
    R,
    s;

  R=QuantumScale*DecodePixelGamma(red);
  G=QuantumScale*DecodePixelGamma(green);
  B=QuantumScale*DecodePixelGamma(blue);
  l=cbrt(0.4122214708*R+0.5363325363*G+0.0514459929*B);
  m=cbrt(0.2119034982*R+0.6806995451*G+0.1073969566*B);
  s=cbrt(0.0883024619*R+0.2817188376*G+0.6299787005*B);
  *L=0.2104542553*l+0.7936177850*m-0.0040720468*s;
  *a=1.9779984951*l-2.4285922050*m+0.4505937099*s+0.5;
  *b=0.0259040371*l+0.7827717662*m-0.8086757660*s+0.5;
}

static inline void ConvertOklchToRGB(const double L,const double C,
  const double h,double *red,double *green,double *blue)
{
  double
    a,
    b;

  a=C*cos(2.0*MagickPI*h);
  b=C*sin(2.0*MagickPI*h);
  ConvertOklabToRGB(L,a,b,red,green,blue);
}

static inline void ConvertRGBToOklch(const double red,const double green,
  const double blue,double *L,double *C,double *h)
{
  double
    a,
    b;

  ConvertRGBToOklab(red,green,blue,L,&a,&b);
  *C=sqrt(a*a+b*b);
  *h=0.5+0.5*atan2(-b,-a)/MagickPI;
}

static inline void ConvertRGBToYDbDr(const double red,const double green,
  const double blue,double *Y,double *Db,double *Dr)
{
  *Y=QuantumScale*(0.298839*red+0.586811*green+0.114350*blue);
  *Db=QuantumScale*(-0.450*red-0.883*green+1.333*blue)+0.5;
  *Dr=QuantumScale*(-1.333*red+1.116*green+0.217*blue)+0.5;
}

static inline void ConvertRGBToYIQ(const double red,const double green,
  const double blue,double *Y,double *I,double *Q)
{
  *Y=QuantumScale*(0.298839*red+0.586811*green+0.114350*blue);
  *I=QuantumScale*(0.595716*red-0.274453*green-0.321263*blue)+0.5;
  *Q=QuantumScale*(0.211456*red-0.522591*green+0.311135*blue)+0.5;
}

static inline void ConvertRGBToYPbPr(const double red,const double green,
  const double blue,double *Y,double *Pb,double *Pr)
{
  *Y=QuantumScale*(0.298839*red+0.586811*green+0.114350*blue);
  *Pb=QuantumScale*((-0.1687367)*red-0.331264*green+0.5*blue)+0.5;
  *Pr=QuantumScale*(0.5*red-0.418688*green-0.081312*blue)+0.5;
}

static inline void ConvertRGBToYCbCr(const double red,const double green,
  const double blue,double *Y,double *Cb,double *Cr)
{
  ConvertRGBToYPbPr(red,green,blue,Y,Cb,Cr);
}

static inline void ConvertRGBToYUV(const double red,const double green,
  const double blue,double *Y,double *U,double *V)
{
  *Y=QuantumScale*(0.298839*red+0.586811*green+0.114350*blue);
  *U=QuantumScale*((-0.147)*red-0.289*green+0.436*blue)+0.5;
  *V=QuantumScale*(0.615*red-0.515*green-0.100*blue)+0.5;
}

static inline void ConvertRGBToCMYK(PixelInfo *pixel)
{
  MagickRealType
    black,
    blue,
    cyan,
    green,
    magenta,
    red,
    yellow;

  if (pixel->colorspace != sRGBColorspace)
    {
      red=QuantumScale*pixel->red;
      green=QuantumScale*pixel->green;
      blue=QuantumScale*pixel->blue;
    }
  else
    {
      red=QuantumScale*DecodePixelGamma(pixel->red);
      green=QuantumScale*DecodePixelGamma(pixel->green);
      blue=QuantumScale*DecodePixelGamma(pixel->blue);
    }
  if ((fabs((double) red) < MagickEpsilon) &&
      (fabs((double) green) < MagickEpsilon) &&
      (fabs((double) blue) < MagickEpsilon))
    {
      pixel->black=(MagickRealType) QuantumRange;
      return;
    }
  cyan=(MagickRealType) (1.0-red);
  magenta=(MagickRealType) (1.0-green);
  yellow=(MagickRealType) (1.0-blue);
  black=cyan;
  if (magenta < black)
    black=magenta;
  if (yellow < black)
    black=yellow;
  cyan=(MagickRealType) (PerceptibleReciprocal(1.0-black)*(cyan-black));
  magenta=(MagickRealType) (PerceptibleReciprocal(1.0-black)*(magenta-black));
  yellow=(MagickRealType) (PerceptibleReciprocal(1.0-black)*(yellow-black));
  pixel->colorspace=CMYKColorspace;
  pixel->red=(MagickRealType) QuantumRange*cyan;
  pixel->green=(MagickRealType) QuantumRange*magenta;
  pixel->blue=(MagickRealType) QuantumRange*yellow;
  pixel->black=(MagickRealType) QuantumRange*black;
}

static inline void ConvertYPbPrToRGB(const double Y,const double Pb,
  const double Pr,double *red,double *green,double *blue)
{
  *red=(double) QuantumRange*(0.99999999999914679361*Y-1.2188941887145875e-06*
    (Pb-0.5)+1.4019995886561440468*(Pr-0.5));
  *green=(double) QuantumRange*(0.99999975910502514331*Y-0.34413567816504303521*
    (Pb-0.5)-0.71413649331646789076*(Pr-0.5));
  *blue=(double) QuantumRange*(1.00000124040004623180*Y+1.77200006607230409200*
    (Pb-0.5)+2.1453384174593273e-06*(Pr-0.5));
}

static inline void ConvertYCbCrToRGB(const double Y,const double Cb,
  const double Cr,double *red,double *green,double *blue)
{
  ConvertYPbPrToRGB(Y,Cb,Cr,red,green,blue);
}

static inline void ConvertYDbDrToRGB(const double Y,const double Db,
  const double Dr,double *red,double *green,double *blue)
{
  *red=(double) QuantumRange*(Y+9.2303716147657e-05*(Db-0.5)-
    0.52591263066186533*(Dr-0.5));
  *green=(double) QuantumRange*(Y-0.12913289889050927*(Db-0.5)+
    0.26789932820759876*(Dr-0.5));
  *blue=(double) QuantumRange*(Y+0.66467905997895482*(Db-0.5)-
    7.9202543533108e-05*(Dr-0.5));
}   

static inline void ConvertYIQToRGB(const double Y,const double I,const double Q,
  double *red,double *green,double *blue)
{
  *red=(double) QuantumRange*(Y+0.9562957197589482261*(I-0.5)+
    0.6210244164652610754*(Q-0.5));
  *green=(double) QuantumRange*(Y-0.2721220993185104464*(I-0.5)-
    0.6473805968256950427*(Q-0.5));
  *blue=(double) QuantumRange*(Y-1.1069890167364901945*(I-0.5)+
    1.7046149983646481374*(Q-0.5));
}

static inline void ConvertxyYToRGB(const double low_x,const double low_y,
  const double cap_Y,double *red,double *green,double *blue)
{
  double
    gamma,
    X,
    Y,
    Z;

  gamma=PerceptibleReciprocal(low_y);
  X=gamma*cap_Y*low_x;
  Y=cap_Y;
  Z=gamma*cap_Y*(1.0-low_x-low_y);
  ConvertXYZToRGB(X,Y,Z,red,green,blue);
}

static inline void ConvertYUVToRGB(const double Y,const double U,const double V,
  double *red,double *green,double *blue)
{
  *red=(double) QuantumRange*(Y-3.945707070708279e-05*(U-0.5)+
    1.1398279671717170825*(V-0.5));
  *green=(double) QuantumRange*(Y-0.3946101641414141437*(U-0.5)-
    0.5805003156565656797*(V-0.5));
  *blue=(double) QuantumRange*(Y+2.0319996843434342537*(U-0.5)-
    4.813762626262513e-04*(V-0.5));
}

static inline MagickBooleanType IsCMYKColorspace(
  const ColorspaceType colorspace)
{
  if (colorspace == CMYKColorspace)
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsGrayColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == LinearGRAYColorspace) || (colorspace == GRAYColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsGrayImageType(const ImageType type)
{
  if ((type == GrayscaleType) || (type == GrayscaleAlphaType) ||
      (type == BilevelType))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsHueCompatibleColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == HCLColorspace) || (colorspace == HCLpColorspace) ||
      (colorspace == HSBColorspace) || (colorspace == HSIColorspace) ||
      (colorspace == HSLColorspace) || (colorspace == HSVColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsLabCompatibleColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == LabColorspace) || (colorspace == LCHColorspace) ||
      (colorspace == LCHabColorspace) || (colorspace == LCHuvColorspace) ||
      (colorspace == OklabColorspace) || (colorspace == OklchColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsRGBColorspace(const ColorspaceType colorspace)
{
  if ((colorspace == RGBColorspace) || (colorspace == scRGBColorspace) ||
      (colorspace == LinearGRAYColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IssRGBColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == sRGBColorspace) || (colorspace == TransparentColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IssRGBCompatibleColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == sRGBColorspace) || (colorspace == RGBColorspace) ||
      (colorspace == Adobe98Colorspace) || (colorspace == ProPhotoColorspace) ||
      (colorspace == DisplayP3Colorspace) || (colorspace == scRGBColorspace) ||
      (colorspace == TransparentColorspace) || (colorspace == GRAYColorspace) ||
      (colorspace == LinearGRAYColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

static inline MagickBooleanType IsYCbCrCompatibleColorspace(
  const ColorspaceType colorspace)
{
  if ((colorspace == YCbCrColorspace) ||
      (colorspace == Rec709YCbCrColorspace) ||
      (colorspace == Rec601YCbCrColorspace))
    return(MagickTrue);
  return(MagickFalse);
}

extern MagickPrivate void
  ConvertGenericToRGB(const ColorspaceType,const double,const double,
    const double,const double,const IlluminantType,double *,double *,double *),
  ConvertRGBToGeneric(const ColorspaceType,const double,const double,
    const double,const double,const IlluminantType,double *,double *,double *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

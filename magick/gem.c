/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                              GGGG  EEEEE  M   M                             %
%                             G      E      MM MM                             %
%                             G GG   EEE    M M M                             %
%                             G   G  E      M   M                             %
%                              GGGG  EEEEE  M   M                             %
%                                                                             %
%                                                                             %
%                    Graphic Gems - Graphic Support Methods                   %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                 August 1996                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/color-private.h"
#include "magick/draw.h"
#include "magick/gem.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/log.h"
#include "magick/memory_.h"
#include "magick/pixel-private.h"
#include "magick/quantum.h"
#include "magick/random_.h"
#include "magick/resize.h"
#include "magick/transform.h"
#include "magick/signature-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n v e r t H S B T o R G B                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvertHSBToRGB() transforms a (hue, saturation, brightness) to a (red,
%  green, blue) triple.
%
%  The format of the ConvertHSBToRGBImage method is:
%
%      void ConvertHSBToRGB(const double hue,const double saturation,
%        const double brightness,Quantum *red,Quantum *green,Quantum *blue)
%
%  A description of each parameter follows:
%
%    o hue, saturation, brightness: A double value representing a
%      component of the HSB color space.
%
%    o red, green, blue: A pointer to a pixel component of type Quantum.
%
*/
MagickExport void ConvertHSBToRGB(const double hue,const double saturation,
  const double brightness,Quantum *red,Quantum *green,Quantum *blue)
{
  MagickRealType
    f,
    h,
    p,
    q,
    t;

  /*
    Convert HSB to RGB colorspace.
  */
  assert(red != (Quantum *) NULL);
  assert(green != (Quantum *) NULL);
  assert(blue != (Quantum *) NULL);
  if (saturation == 0.0)
    {
      *red=ClampToQuantum((MagickRealType) QuantumRange*brightness);
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
      *red=ClampToQuantum((MagickRealType) QuantumRange*brightness);
      *green=ClampToQuantum((MagickRealType) QuantumRange*t);
      *blue=ClampToQuantum((MagickRealType) QuantumRange*p);
      break;
    }
    case 1:
    {
      *red=ClampToQuantum((MagickRealType) QuantumRange*q);
      *green=ClampToQuantum((MagickRealType) QuantumRange*brightness);
      *blue=ClampToQuantum((MagickRealType) QuantumRange*p);
      break;
    }
    case 2:
    {
      *red=ClampToQuantum((MagickRealType) QuantumRange*p);
      *green=ClampToQuantum((MagickRealType) QuantumRange*brightness);
      *blue=ClampToQuantum((MagickRealType) QuantumRange*t);
      break;
    }
    case 3:
    {
      *red=ClampToQuantum((MagickRealType) QuantumRange*p);
      *green=ClampToQuantum((MagickRealType) QuantumRange*q);
      *blue=ClampToQuantum((MagickRealType) QuantumRange*brightness);
      break;
    }
    case 4:
    {
      *red=ClampToQuantum((MagickRealType) QuantumRange*t);
      *green=ClampToQuantum((MagickRealType) QuantumRange*p);
      *blue=ClampToQuantum((MagickRealType) QuantumRange*brightness);
      break;
    }
    case 5:
    {
      *red=ClampToQuantum((MagickRealType) QuantumRange*brightness);
      *green=ClampToQuantum((MagickRealType) QuantumRange*p);
      *blue=ClampToQuantum((MagickRealType) QuantumRange*q);
      break;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n v e r t H S L T o R G B                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvertHSLToRGB() transforms a (hue, saturation, lightness) to a (red,
%  green, blue) triple.
%
%  The format of the ConvertHSLToRGBImage method is:
%
%      void ConvertHSLToRGB(const double hue,const double saturation,
%        const double lightness,Quantum *red,Quantum *green,Quantum *blue)
%
%  A description of each parameter follows:
%
%    o hue, saturation, lightness: A double value representing a
%      component of the HSL color space.
%
%    o red, green, blue: A pointer to a pixel component of type Quantum.
%
*/

static inline MagickRealType ConvertHueToRGB(MagickRealType m1,
  MagickRealType m2,MagickRealType hue)
{
  if (hue < 0.0)
    hue+=1.0;
  if (hue > 1.0)
    hue-=1.0;
  if ((6.0*hue) < 1.0)
    return(m1+6.0*(m2-m1)*hue);
  if ((2.0*hue) < 1.0)
    return(m2);
  if ((3.0*hue) < 2.0)
    return(m1+6.0*(m2-m1)*(2.0/3.0-hue));
  return(m1);
}

MagickExport void ConvertHSLToRGB(const double hue,const double saturation,
  const double lightness,Quantum *red,Quantum *green,Quantum *blue)
{
  MagickRealType
    b,
    g,
    r,
    m1,
    m2;

  /*
    Convert HSL to RGB colorspace.
  */
  assert(red != (Quantum *) NULL);
  assert(green != (Quantum *) NULL);
  assert(blue != (Quantum *) NULL);
  if (saturation == 0)
    {
      *red=ClampToQuantum((MagickRealType) QuantumRange*lightness);
      *green=(*red);
      *blue=(*red);
      return;
    }
  if (lightness < 0.5)
    m2=lightness*(saturation+1.0);
  else
    m2=(lightness+saturation)-(lightness*saturation);
  m1=2.0*lightness-m2;
  r=ConvertHueToRGB(m1,m2,hue+1.0/3.0);
  g=ConvertHueToRGB(m1,m2,hue);
  b=ConvertHueToRGB(m1,m2,hue-1.0/3.0);
  *red=ClampToQuantum((MagickRealType) QuantumRange*r);
  *green=ClampToQuantum((MagickRealType) QuantumRange*g);
  *blue=ClampToQuantum((MagickRealType) QuantumRange*b);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n v e r t H W B T o R G B                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvertHWBToRGB() transforms a (hue, whiteness, blackness) to a (red, green,
%  blue) triple.
%
%  The format of the ConvertHWBToRGBImage method is:
%
%      void ConvertHWBToRGB(const double hue,const double whiteness,
%        const double blackness,Quantum *red,Quantum *green,Quantum *blue)
%
%  A description of each parameter follows:
%
%    o hue, whiteness, blackness: A double value representing a
%      component of the HWB color space.
%
%    o red, green, blue: A pointer to a pixel component of type Quantum.
%
*/
MagickExport void ConvertHWBToRGB(const double hue,const double whiteness,
  const double blackness,Quantum *red,Quantum *green,Quantum *blue)
{
  MagickRealType
    b,
    f,
    g,
    n,
    r,
    v;

  register ssize_t
    i;

  /*
    Convert HWB to RGB colorspace.
  */
  assert(red != (Quantum *) NULL);
  assert(green != (Quantum *) NULL);
  assert(blue != (Quantum *) NULL);
  v=1.0-blackness;
  if (hue == 0.0)
    {
      *red=ClampToQuantum((MagickRealType) QuantumRange*v);
      *green=ClampToQuantum((MagickRealType) QuantumRange*v);
      *blue=ClampToQuantum((MagickRealType) QuantumRange*v);
      return;
    }
  i=(ssize_t) floor(6.0*hue);
  f=6.0*hue-i;
  if ((i & 0x01) != 0)
    f=1.0-f;
  n=whiteness+f*(v-whiteness);  /* linear interpolation */
  switch (i)
  {
    default:
    case 6:
    case 0: r=v; g=n; b=whiteness; break;
    case 1: r=n; g=v; b=whiteness; break;
    case 2: r=whiteness; g=v; b=n; break;
    case 3: r=whiteness; g=n; b=v; break;
    case 4: r=n; g=whiteness; b=v; break;
    case 5: r=v; g=whiteness; b=n; break;
  }
  *red=ClampToQuantum((MagickRealType) QuantumRange*r);
  *green=ClampToQuantum((MagickRealType) QuantumRange*g);
  *blue=ClampToQuantum((MagickRealType) QuantumRange*b);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n v e r t R G B T o H S B                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvertRGBToHSB() transforms a (red, green, blue) to a (hue, saturation,
%  brightness) triple.
%
%  The format of the ConvertRGBToHSB method is:
%
%      void ConvertRGBToHSB(const Quantum red,const Quantum green,
%        const Quantum blue,double *hue,double *saturation,double *brightness)
%
%  A description of each parameter follows:
%
%    o red, green, blue: A Quantum value representing the red, green, and
%      blue component of a pixel..
%
%    o hue, saturation, brightness: A pointer to a double value representing a
%      component of the HSB color space.
%
*/
MagickExport void ConvertRGBToHSB(const Quantum red,const Quantum green,
  const Quantum blue,double *hue,double *saturation,double *brightness)
{
  MagickRealType
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
  min=(MagickRealType) (red < green ? red : green);
  if ((MagickRealType) blue < min)
    min=(MagickRealType) blue;
  max=(MagickRealType) (red > green ? red : green);
  if ((MagickRealType) blue > max)
    max=(MagickRealType) blue;
  if (max == 0.0)
    return;
  delta=max-min;
  *saturation=(double) (delta/max);
  *brightness=(double) (QuantumScale*max);
  if (delta == 0.0)
    return;
  if ((MagickRealType) red == max)
    *hue=(double) ((green-(MagickRealType) blue)/delta);
  else
    if ((MagickRealType) green == max)
      *hue=(double) (2.0+(blue-(MagickRealType) red)/delta);
    else
      *hue=(double) (4.0+(red-(MagickRealType) green)/delta);
  *hue/=6.0;
  if (*hue < 0.0)
    *hue+=1.0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n v e r t R G B T o H S L                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvertRGBToHSL() transforms a (red, green, blue) to a (hue, saturation,
%  lightness) triple.
%
%  The format of the ConvertRGBToHSL method is:
%
%      void ConvertRGBToHSL(const Quantum red,const Quantum green,
%        const Quantum blue,double *hue,double *saturation,double *lightness)
%
%  A description of each parameter follows:
%
%    o red, green, blue: A Quantum value representing the red, green, and
%      blue component of a pixel..
%
%    o hue, saturation, lightness: A pointer to a double value representing a
%      component of the HSL color space.
%
*/

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport void ConvertRGBToHSL(const Quantum red,const Quantum green,
  const Quantum blue,double *hue,double *saturation,double *lightness)
{
  MagickRealType
    b,
    delta,
    g,
    max,
    min,
    r;

  /*
    Convert RGB to HSL colorspace.
  */
  assert(hue != (double *) NULL);
  assert(saturation != (double *) NULL);
  assert(lightness != (double *) NULL);
  r=QuantumScale*red;
  g=QuantumScale*green;
  b=QuantumScale*blue;
  max=MagickMax(r,MagickMax(g,b));
  min=MagickMin(r,MagickMin(g,b));
  *lightness=(double) ((min+max)/2.0);
  delta=max-min;
  if (delta == 0.0)
    {
      *hue=0.0;
      *saturation=0.0;
      return;
    }
  if (*lightness < 0.5)
    *saturation=(double) (delta/(min+max));
  else
    *saturation=(double) (delta/(2.0-max-min));
  if (r == max)
    *hue=((((max-b)/6.0)+(delta/2.0))-(((max-g)/6.0)+(delta/2.0)))/delta;
  else
    if (g == max)
      *hue=(1.0/3.0)+((((max-r)/6.0)+(delta/2.0))-(((max-b)/6.0)+(delta/2.0)))/
        delta;
    else
      if (b == max)
        *hue=(2.0/3.0)+((((max-g)/6.0)+(delta/2.0))-(((max-r)/6.0)+
          (delta/2.0)))/delta;
  if (*hue < 0.0)
    *hue+=1.0;
  if (*hue > 1.0)
    *hue-=1.0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n v e r t R G B T o H W B                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvertRGBToHWB() transforms a (red, green, blue) to a (hue, whiteness,
%  blackness) triple.
%
%  The format of the ConvertRGBToHWB method is:
%
%      void ConvertRGBToHWB(const Quantum red,const Quantum green,
%        const Quantum blue,double *hue,double *whiteness,double *blackness)
%
%  A description of each parameter follows:
%
%    o red, green, blue: A Quantum value representing the red, green, and
%      blue component of a pixel.
%
%    o hue, whiteness, blackness: A pointer to a double value representing a
%      component of the HWB color space.
%
*/
MagickExport void ConvertRGBToHWB(const Quantum red,const Quantum green,
  const Quantum blue,double *hue,double *whiteness,double *blackness)
{
  long
    i;

  MagickRealType
    f,
    v,
    w;

  /*
    Convert RGB to HWB colorspace.
  */
  assert(hue != (double *) NULL);
  assert(whiteness != (double *) NULL);
  assert(blackness != (double *) NULL);
  w=(MagickRealType) MagickMin((double) red,MagickMin((double) green,(double)
    blue));
  v=(MagickRealType) MagickMax((double) red,MagickMax((double) green,(double)
    blue));
  *blackness=1.0-QuantumScale*v;
  *whiteness=QuantumScale*w;
  if (v == w)
    {
      *hue=0.0;
      return;
    }
  f=((MagickRealType) red == w) ? green-(MagickRealType) blue :
    (((MagickRealType) green == w) ? blue-(MagickRealType) red : red-
    (MagickRealType) green);
  i=((MagickRealType) red == w) ? 3 : (((MagickRealType) green == w) ? 5 : 1);
  *hue=((double) i-f/(v-1.0*w))/6.0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x p a n d A f f i n e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExpandAffine() computes the affine's expansion factor, i.e. the square root
%  of the factor by which the affine transform affects area. In an affine
%  transform composed of scaling, rotation, shearing, and translation, returns
%  the amount of scaling.
%
%  The format of the ExpandAffine method is:
%
%      double ExpandAffine(const AffineMatrix *affine)
%
%  A description of each parameter follows:
%
%    o expansion: Method ExpandAffine returns the affine's expansion factor.
%
%    o affine: A pointer the affine transform of type AffineMatrix.
%
*/
MagickExport double ExpandAffine(const AffineMatrix *affine)
{
  assert(affine != (const AffineMatrix *) NULL);
  return(sqrt(fabs(affine->sx*affine->sy-affine->rx*affine->ry)));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e n e r a t e D i f f e r e n t i a l N o i s e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GenerateDifferentialNoise() generates differentual noise.
%
%  The format of the GenerateDifferentialNoise method is:
%
%      double GenerateDifferentialNoise(RandomInfo *random_info,
%        const Quantum pixel,const NoiseType noise_type,
%        const MagickRealType attenuate)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
%    o pixel: noise is relative to this pixel value.
%
%    o noise_type: the type of noise.
%
%    o attenuate:  attenuate the noise.
%
*/
MagickExport double GenerateDifferentialNoise(RandomInfo *random_info,
  const Quantum pixel,const NoiseType noise_type,const MagickRealType attenuate)
{
#define SigmaUniform  (attenuate*0.015625)
#define SigmaGaussian  (attenuate*0.015625)
#define SigmaImpulse  (attenuate*0.1)
#define SigmaLaplacian (attenuate*0.0390625)
#define SigmaMultiplicativeGaussian  (attenuate*0.5)
#define SigmaPoisson  (attenuate*12.75)
#define TauGaussian  (attenuate*0.078125)

  double
    alpha,
    beta,
    noise,
    sigma;

  alpha=GetPseudoRandomValue(random_info);
  switch (noise_type)
  {
    case UniformNoise:
    default:
    {
      noise=(double) pixel+QuantumRange*SigmaUniform*(alpha-0.5);
      break;
    }
    case GaussianNoise:
    {
      double
        gamma,
        tau;

      if (alpha == 0.0)
        alpha=1.0;
      beta=GetPseudoRandomValue(random_info);
      gamma=sqrt(-2.0*log(alpha));
      sigma=gamma*cos((double) (2.0*MagickPI*beta));
      tau=gamma*sin((double) (2.0*MagickPI*beta));
      noise=(double) pixel+sqrt((double) pixel)*SigmaGaussian*sigma+
        QuantumRange*TauGaussian*tau;
      break;
    }
    case ImpulseNoise:
    {
      if (alpha < (SigmaImpulse/2.0))
        noise=0.0;
      else
        if (alpha >= (1.0-(SigmaImpulse/2.0)))
          noise=(double) QuantumRange;
        else
          noise=(double) pixel;
      break;
    }
    case LaplacianNoise:
    {
      if (alpha <= 0.5)
        {
          if (alpha <= MagickEpsilon)
            noise=(double) pixel-(double) QuantumRange;
          else
            noise=(double) pixel+QuantumRange*SigmaLaplacian*
              log(2.0*alpha)+0.5;
          break;
        }
      beta=1.0-alpha;
      if (beta <= (0.5*MagickEpsilon))
        noise=(double) (pixel+QuantumRange);
      else
        noise=(double) pixel-QuantumRange*SigmaLaplacian*log(2.0*beta)+0.5;
      break;
    }
    case MultiplicativeGaussianNoise:
    {
      sigma=1.0;
      if (alpha > MagickEpsilon)
        sigma=sqrt(-2.0*log(alpha));
      beta=GetPseudoRandomValue(random_info);
      noise=(double) pixel+pixel*SigmaMultiplicativeGaussian*sigma*
        cos((double) (2.0*MagickPI*beta))/2.0;
      break;
    }
    case PoissonNoise:
    {
      double
        poisson;

      register ssize_t
        i;

      poisson=exp(-SigmaPoisson*QuantumScale*pixel);
      for (i=0; alpha > poisson; i++)
      {
        beta=GetPseudoRandomValue(random_info);
        alpha*=beta;
      }
      noise=(double) QuantumRange*i/SigmaPoisson;
      break;
    }
    case RandomNoise:
    {
      noise=(double) QuantumRange*alpha;
      break;
    }
  }
  return(noise);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O p t i m a l K e r n e l W i d t h                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOptimalKernelWidth() computes the optimal kernel radius for a convolution
%  filter.  Start with the minimum value of 3 pixels and walk out until we drop
%  below the threshold of one pixel numerical accuracy.
%
%  The format of the GetOptimalKernelWidth method is:
%
%      size_t GetOptimalKernelWidth(const double radius,
%        const double sigma)
%
%  A description of each parameter follows:
%
%    o width: Method GetOptimalKernelWidth returns the optimal width of
%      a convolution kernel.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
*/
MagickExport size_t GetOptimalKernelWidth1D(const double radius,
  const double sigma)
{
  double
    alpha,
    beta,
    gamma,
    normalize,
    value;

  register ssize_t
    i;

  size_t
    width;

  ssize_t
    j;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (radius > MagickEpsilon)
    return((size_t) (2.0*ceil(radius)+1.0));
  gamma=fabs(sigma);
  if (gamma <= MagickEpsilon)
    return(3UL);
  alpha=1.0/(2.0*gamma*gamma);
  beta=(double) (1.0/(MagickSQ2PI*gamma));
  for (width=5; ; )
  {
    normalize=0.0;
    j=(ssize_t) width/2;
    for (i=(-j); i <= j; i++)
      normalize+=exp(-((double) (i*i))*alpha)*beta;
    value=exp(-((double) (j*j))*alpha)*beta/normalize;
    if ((value < QuantumScale) || (value < MagickEpsilon))
      break;
    width+=2;
  }
  return((size_t) (width-2));
}

MagickExport size_t GetOptimalKernelWidth2D(const double radius,
  const double sigma)
{
  double
    alpha,
    beta,
    gamma,
    normalize,
    value;

  size_t
    width;

  ssize_t
    j,
    u,
    v;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (radius > MagickEpsilon)
    return((size_t) (2.0*ceil(radius)+1.0));
  gamma=fabs(sigma);
  if (gamma <= MagickEpsilon)
    return(3UL);
  alpha=1.0/(2.0*gamma*gamma);
  beta=(double) (1.0/(Magick2PI*gamma*gamma));
  for (width=5; ; )
  {
    normalize=0.0;
    j=(ssize_t) width/2;
    for (v=(-j); v <= j; v++)
      for (u=(-j); u <= j; u++)
        normalize+=exp(-((double) (u*u+v*v))*alpha)*beta;
    value=exp(-((double) (j*j))*alpha)*beta/normalize;
    if ((value < QuantumScale) || (value < MagickEpsilon))
      break;
    width+=2;
  }
  return((size_t) (width-2));
}

MagickExport size_t  GetOptimalKernelWidth(const double radius,
  const double sigma)
{
  return(GetOptimalKernelWidth1D(radius,sigma));
}

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
%                                    Cristy                                   %
%                                 August 1996                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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
#include "MagickCore/studio.h"
#include "MagickCore/color-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/gem.h"
#include "MagickCore/gem-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/resize.h"
#include "MagickCore/transform.h"
#include "MagickCore/signature-private.h"

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
%    o expansion: ExpandAffine returns the affine's expansion factor.
%
%    o affine: A pointer the affine transform of type AffineMatrix.
%
*/
MagickExport double ExpandAffine(const AffineMatrix *affine)
{
  double
    determinant;

  assert(affine != (const AffineMatrix *) NULL);
  determinant=affine->sx*affine->sy-affine->rx*affine->ry;
  return(sqrt(fabs(determinant)));
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
%  GenerateDifferentialNoise() generates differential noise.
%
%  The format of the GenerateDifferentialNoise method is:
%
%      double GenerateDifferentialNoise(RandomInfo *random_info,
%        const Quantum pixel,const NoiseType noise_type,const double attenuate)
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
MagickPrivate double GenerateDifferentialNoise(RandomInfo *random_info,
  const Quantum pixel,const NoiseType noise_type,const double attenuate)
{
#define SigmaUniform  (attenuate*0.015625)
#define SigmaGaussian  (attenuate*0.015625)
#define SigmaImpulse  (attenuate*0.1)
#define SigmaLaplacian (attenuate*0.0390625)
#define SigmaMultiplicativeGaussian  (attenuate*0.5)
#define SigmaPoisson  (attenuate*12.5)
#define SigmaRandom  (attenuate)
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
      noise=(double) pixel+(double) QuantumRange*SigmaUniform*(alpha-0.5);
      break;
    }
    case GaussianNoise:
    {
      double
        gamma,
        tau;

      if (alpha <= MagickEpsilon)
        alpha=1.0;
      beta=GetPseudoRandomValue(random_info);
      gamma=sqrt(-2.0*log(alpha));
      sigma=gamma*cos((double) (2.0*MagickPI*beta));
      tau=gamma*sin((double) (2.0*MagickPI*beta));
      noise=(double) pixel+sqrt((double) pixel)*SigmaGaussian*sigma+
        (double) QuantumRange*TauGaussian*tau;
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
            noise=(double) pixel+(double) QuantumRange*SigmaLaplacian*
              log(2.0*alpha)+0.5;
          break;
        }
      beta=1.0-alpha;
      if (beta <= (0.5*MagickEpsilon))
        noise=(double) pixel+(double) QuantumRange;
      else
        noise=(double) pixel-(double) QuantumRange*SigmaLaplacian*
          log(2.0*beta)+0.5;
      break;
    }
    case MultiplicativeGaussianNoise:
    {
      sigma=1.0;
      if (alpha > MagickEpsilon)
        sigma=sqrt(-2.0*log(alpha));
      beta=GetPseudoRandomValue(random_info);
      noise=(double) pixel+(double) pixel*SigmaMultiplicativeGaussian*sigma*
        cos((double) (2.0*MagickPI*beta))/2.0;
      break;
    }
    case PoissonNoise:
    {
      double
        poisson;

      ssize_t
        i;

      poisson=exp(-SigmaPoisson*QuantumScale*(double) pixel);
      for (i=0; alpha > poisson; i++)
      {
        beta=GetPseudoRandomValue(random_info);
        alpha*=beta;
      }
      noise=(double) QuantumRange*i*MagickSafeReciprocal(SigmaPoisson);
      break;
    }
    case RandomNoise:
    {
      noise=(double) QuantumRange*SigmaRandom*alpha;
      break;
    }
  }
  return(noise);
#undef SigmaUniform
#undef SigmaGaussian
#undef SigmaImpulse
#undef SigmaLaplacian
#undef SigmaMultiplicativeGaussian
#undef SigmaPoisson
#undef SigmaRandom
#undef TauGaussian
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
%  GetOptimalKernelWidth1D() and GetOptimalKernelWidth2D() compute the optimal
%  kernel width for a 1D or 2D Gaussian convolution filter.  Start with the
%  minimum value of 3 pixels and walk out until we drop below the threshold of
%  one pixel numerical accuracy.  GetOptimalKernelWidth() is the 1D variant.
%
%  The format of the GetOptimalKernelWidth methods is:
%
%      size_t GetOptimalKernelWidth1D(const double radius,const double sigma)
%      size_t GetOptimalKernelWidth2D(const double radius,const double sigma)
%      size_t GetOptimalKernelWidth(const double radius,const double sigma)
%
%  A description of each parameter follows:
%
%    o width: GetOptimalKernelWidth returns the optimal width of a
%      convolution kernel.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
*/

static size_t GetGaussianKernelWidth(const double radius,const double sigma,
  const MagickBooleanType two_dimensional)
{
#define MaxOptimalKernelWidth  65535UL

  double
    alpha,
    gamma,
    sum,
    term,
    value;

  size_t
    width;

  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (isnan(radius) || isnan(sigma))
    return(3UL);
  if (radius > MagickEpsilon)
    {
      const double
        limit = (double) ((MaxOptimalKernelWidth-1UL)/2UL);

      return((size_t) (2.0*MagickMin(ceil(radius),limit)+1.0));
    }
  gamma=fabs(sigma);
  if (gamma <= MagickEpsilon)
    return(3UL);
  if (isinf(gamma))
    return(3UL);
  alpha=MagickSafeReciprocal(2.0*gamma*gamma);
  /*
    The Gaussian is separable.
  */
  sum=1.0+2.0*exp(-alpha);
  for (width=5; ; width+=2)
  {
    const double
      j = (double) ((width-1)/2);

    term=exp(-(j*j)*alpha);
    sum+=2.0*term;
    value=two_dimensional != MagickFalse ? term/(sum*sum) : term/sum;
    if ((value < QuantumScale) || (value < MagickEpsilon))
      break;
    if (width >= MaxOptimalKernelWidth)
      return(MaxOptimalKernelWidth);
  }
  return((size_t) (width-2));
}

MagickPrivate size_t GetOptimalKernelWidth1D(const double radius,
  const double sigma)
{
  return(GetGaussianKernelWidth(radius,sigma,MagickFalse));
}

MagickPrivate size_t GetOptimalKernelWidth2D(const double radius,
  const double sigma)
{
  return(GetGaussianKernelWidth(radius,sigma,MagickTrue));
}

MagickPrivate size_t GetOptimalKernelWidth(const double radius,
  const double sigma)
{
  return(GetOptimalKernelWidth1D(radius,sigma));
}

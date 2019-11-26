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

  MagickCore image composite private methods.
*/
#ifndef MAGICKCORE_COMPOSITE_PRIVATE_H
#define MAGICKCORE_COMPOSITE_PRIVATE_H


#include "MagickCore/color.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  ImageMagick Alpha Composite Inline Methods (special export)
*/
static inline double MagickOver_(const double p,const double alpha,
  const double q,const double beta)
{
  double
    Da,
    Sa;

  Sa=QuantumScale*alpha;
  Da=QuantumScale*beta;
  return(Sa*p+Da*q*(1.0-Sa));
}

static inline double RoundToUnity(const double value)
{
  return(value < 0.0 ? 0.0 : (value > 1.0) ? 1.0 : value);
}

static inline void CompositePixelOver(const Image *image,const PixelInfo *p,
  const double alpha,const Quantum *q,const double beta,Quantum *composite)
{
  double
    Da,
    gamma,
    Sa;

  register ssize_t
    i;

  /*
    Compose pixel p over pixel q with the given alpha.
  */
  Sa=QuantumScale*alpha;
  Da=QuantumScale*beta;
  gamma=Sa+Da-Sa*Da;
  gamma=PerceptibleReciprocal(gamma);
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel
      channel;

    PixelTrait
      traits;

    channel=GetPixelChannelChannel(image,i);
    traits=GetPixelChannelTraits(image,channel);
    if (traits == UndefinedPixelTrait)
      continue;
    switch (channel)
    {
      case RedPixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((double) p->red,alpha,
          (double) q[i],beta));
        break;
      }
      case GreenPixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((double) p->green,alpha,
          (double) q[i],beta));
        break;
      }
      case BluePixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((double) p->blue,alpha,
          (double) q[i],beta));
        break;
      }
      case BlackPixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((double) p->black,alpha,
          (double) q[i],beta));
        break;
      }
      case AlphaPixelChannel:
      {
        composite[i]=ClampToQuantum(QuantumRange*RoundToUnity(Sa+Da-Sa*Da));
        break;
      }
      default:
      {
        composite[i]=q[i];
        break;
      }
    }
  }
}

static inline void CompositePixelInfoOver(const PixelInfo *p,const double alpha,
  const PixelInfo *q,const double beta,PixelInfo *composite)
{
  double
    Da,
    gamma,
    Sa;

  /*
    Compose pixel p over pixel q with the given opacities.
  */
  Sa=QuantumScale*alpha;
  Da=QuantumScale*beta,
  gamma=Sa+Da-Sa*Da;
  composite->alpha=(double) QuantumRange*RoundToUnity(gamma);
  gamma=PerceptibleReciprocal(gamma);
  composite->red=gamma*MagickOver_(p->red,alpha,q->red,beta);
  composite->green=gamma*MagickOver_(p->green,alpha,q->green,beta);
  composite->blue=gamma*MagickOver_(p->blue,alpha,q->blue,beta);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*MagickOver_(p->black,alpha,q->black,beta);
}

static inline void CompositePixelInfoPlus(const PixelInfo *p,
  const double alpha,const PixelInfo *q,const double beta,PixelInfo *composite)
{
  double
    Da,
    gamma,
    Sa;

  /*
    Add two pixels with the given opacities.
  */
  Sa=QuantumScale*alpha;
  Da=QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da);  /* 'Plus' blending -- not 'Over' blending */
  composite->alpha=(double) QuantumRange*RoundToUnity(gamma);
  gamma=PerceptibleReciprocal(gamma);
  composite->red=gamma*(Sa*p->red+Da*q->red);
  composite->green=gamma*(Sa*p->green+Da*q->green);
  composite->blue=gamma*(Sa*p->blue+Da*q->blue);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*(Sa*p->black+Da*q->black);
}

static inline void CompositePixelInfoAreaBlend(const PixelInfo *p,
  const double alpha,const PixelInfo *q,const double beta,const double area,
  PixelInfo *composite)
{
  /*
    Blend pixel colors p and q by the amount given and area.
  */
  CompositePixelInfoPlus(p,(double) (1.0-area)*alpha,q,(double) (area*beta),
    composite);
}

static inline void CompositePixelInfoBlend(const PixelInfo *p,
  const double alpha,const PixelInfo *q,const double beta,PixelInfo *composite)
{
  /*
    Blend pixel colors p and q by the amount given.
  */
  CompositePixelInfoPlus(p,(double) (alpha*p->alpha),q,(double) (beta*q->alpha),
    composite);
}

static inline MagickBooleanType GetCompositeClipToSelf(
  const CompositeOperator compose)
{
  switch (compose)
  {
    case ClearCompositeOp:
    case SrcCompositeOp:
    case InCompositeOp:
    case SrcInCompositeOp:
    case OutCompositeOp:
    case SrcOutCompositeOp:
    case DstInCompositeOp:
    case DstAtopCompositeOp:
    case CopyAlphaCompositeOp:
    case ChangeMaskCompositeOp:
    {
      return(MagickFalse);
      break;
    }
    default:
      break;
  }
  return(MagickTrue);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image composite private methods.
*/
#ifndef _MAGICKCORE_COMPOSITE_PRIVATE_H
#define _MAGICKCORE_COMPOSITE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  ImageMagick Alpha Composite Inline Methods (special export)
*/

#include "MagickCore/color.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/pixel-accessor.h"

static inline MagickRealType MagickOver_(const MagickRealType p,
  const MagickRealType alpha,const MagickRealType q,const MagickRealType beta)
{
  MagickRealType
    Da,
    Sa;

  Sa=QuantumScale*alpha;
  Da=QuantumScale*beta;
  return(Sa*p-Sa*Da*q+Da*q);
}

static inline void CompositePixelOver(const Image *image,const PixelInfo *p,
  const MagickRealType alpha,const Quantum *q,const MagickRealType beta,
  Quantum *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  register ssize_t
    i;

  /*
    Compose pixel p over pixel q with the given alpha.
  */
  Sa=QuantumScale*alpha;
  Da=QuantumScale*beta,
  gamma=Sa*(-Da)+Sa+Da;
  gamma=1.0/(gamma <= MagickEpsilon ? 1.0 : gamma);
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel
      channel;

    PixelTrait
      traits;

    channel=GetPixelChannelMapChannel(image,i);
    traits=GetPixelChannelMapTraits(image,channel);
    if (traits == UndefinedPixelTrait)
      continue;
    switch (channel)
    {
      case RedPixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((MagickRealType) p->red,
          alpha,(MagickRealType) q[i],beta));
        break;
      }
      case GreenPixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((MagickRealType) p->green,
          alpha,(MagickRealType) q[i],beta));
        break;
      }
      case BluePixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((MagickRealType) p->blue,
          alpha,(MagickRealType) q[i],beta));
        break;
      }
      case BlackPixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((MagickRealType) p->black,
          alpha,(MagickRealType) q[i],beta));
        break;
      }
      case AlphaPixelChannel:
      {
        composite[i]=ClampToQuantum(QuantumRange*(Sa*(-Da)+Sa+Da));
        break;
      }
      default:
        break;
    }
  }
}

static inline void CompositePixelInfoOver(const PixelInfo *p,
  const MagickRealType alpha,const PixelInfo *q,const MagickRealType beta,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  /*
    Compose pixel p over pixel q with the given opacities.
  */
  if (fabs(alpha-OpaqueAlpha) < MagickEpsilon)
    {
      *composite=(*p);
      return;
    }
  Sa=QuantumScale*alpha;
  Da=QuantumScale*beta,
  gamma=Sa*(-Da)+Sa+Da;
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*MagickOver_(p->red,alpha,q->red,beta);
  composite->green=gamma*MagickOver_(p->green,alpha,q->green,beta);
  composite->blue=gamma*MagickOver_(p->blue,alpha,q->blue,beta);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*MagickOver_(p->black,alpha,q->black,beta);
}

static inline MagickRealType RoundToUnity(const MagickRealType value)
{
  return(value < 0.0 ? 0.0 : (value > 1.0) ? 1.0 : value);
}

static inline void CompositePixelInfoPlus(const PixelInfo *p,
  const MagickRealType alpha,const PixelInfo *q,const MagickRealType beta,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  /*
    Add two pixels with the given opacities.
  */
  Sa=QuantumScale*alpha;
  Da=QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da);  /* 'Plus' blending -- not 'Over' blending */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*(Sa*p->red+Da*q->red);
  composite->green=gamma*(Sa*p->green+Da*q->green);
  composite->blue=gamma*(Sa*p->blue+Da*q->blue);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*(Sa*p->black+Da*q->black);
}

static inline void CompositePixelInfoAreaBlend(const PixelInfo *p,
  const MagickRealType alpha,const PixelInfo *q,const MagickRealType beta,
  const MagickRealType area,PixelInfo *composite)
{
  /*
    Blend pixel colors p and q by the amount given and area.
  */
  CompositePixelInfoPlus(p,(MagickRealType) (1.0-area)*alpha,q,(MagickRealType)
    (area*beta),composite);
}

static inline void CompositePixelInfoBlend(const PixelInfo *p,
  const MagickRealType alpha,const PixelInfo *q,const MagickRealType beta,
  PixelInfo *composite)
{
  /*
    Blend pixel colors p and q by the amount given.
  */
  CompositePixelInfoPlus(p,(MagickRealType) (alpha*p->alpha),q,(MagickRealType)
    (beta*q->alpha),composite);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

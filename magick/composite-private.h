/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
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

#include "magick/color.h"
#include "magick/image.h"
#include "magick/image-private.h"

static inline MagickRealType RoundToUnity(const MagickRealType value)
{
  return(value < 0.0 ? 0.0 : (value > 1.0) ? 1.0 : value);
}

static inline MagickRealType MagickOver_(const MagickRealType p,
  const MagickRealType alpha,const MagickRealType q,const MagickRealType beta)
{
  return((1.0-QuantumScale*alpha)*p+
         (1.0-QuantumScale*beta)*q*QuantumScale*alpha);
}

static inline void MagickCompositeOver(const PixelPacket *p,
  const MagickRealType alpha,const PixelPacket *q,const MagickRealType beta,
  PixelPacket *composite)
{
  MagickRealType
    gamma;

  /*
    Compose pixel p over pixel q with the given opacities.
  */
  if (alpha == TransparentOpacity)
    {
      if (composite != q)
        *composite=(*q);
      return;
    }
  gamma=1.0-QuantumScale*QuantumScale*alpha*beta;
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  composite->opacity=(Quantum) (QuantumRange*(1.0-gamma)+0.5);
  gamma=1.0/(gamma <= MagickEpsilon ? 1.0 : gamma);
  composite->red=(Quantum) (gamma*MagickOver_((MagickRealType) p->red,alpha,
    (MagickRealType) q->red,beta)+0.5);
  composite->green=(Quantum) (gamma*MagickOver_((MagickRealType) p->green,alpha,
    (MagickRealType) q->green,beta)+0.5);
  composite->blue=(Quantum) (gamma*MagickOver_((MagickRealType) p->blue,alpha,
    (MagickRealType) q->blue,beta)+0.5);
#else
  composite->opacity=(Quantum) (QuantumRange*(1.0-gamma));
  gamma=1.0/(gamma <= MagickEpsilon ? 1.0 : gamma);
  composite->red=(Quantum) (gamma*MagickOver_((MagickRealType) p->red,alpha,
    (MagickRealType) q->red,beta));
  composite->green=(Quantum) (gamma*MagickOver_((MagickRealType) p->green,alpha,
    (MagickRealType) q->green,beta));
  composite->blue=(Quantum) (gamma*MagickOver_((MagickRealType) p->blue,alpha,
    (MagickRealType) q->blue,beta));
#endif
}

static inline void MagickPixelCompositeOver(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    gamma;

  /*
    Compose pixel p over pixel q with the given opacities.
  */
  if (alpha == OpaqueOpacity)
    {
      *composite=(*p);
      return;
    }
  gamma=1.0-QuantumScale*QuantumScale*alpha*beta;
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*MagickOver_(p->red,alpha,q->red,beta);
  composite->green=gamma*MagickOver_(p->green,alpha,q->green,beta);
  composite->blue=gamma*MagickOver_(p->blue,alpha,q->blue,beta);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*MagickOver_(p->index,alpha,q->index,beta);
}

static inline void MagickPixelCompositePlus(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  /*
    Add two pixels with the given opacities.
  */
  Sa=1.0-QuantumScale*alpha;
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da);  /* 'Plus' blending -- not 'Over' blending */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*(Sa*p->red+Da*q->red);
  composite->green=gamma*(Sa*p->green+Da*q->green);
  composite->blue=gamma*(Sa*p->blue+Da*q->blue);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*(Sa*p->index+Da*q->index);
}

/*
  Blend pixel colors p and q by the amount given.
*/
static inline void MagickPixelCompositeBlend(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickPixelCompositePlus(p,(MagickRealType) (QuantumRange-alpha*
    (QuantumRange-p->opacity)),q,(MagickRealType) (QuantumRange-beta*
    GetAlphaPixelComponent(q)),composite);
}

/*
  Blend pixel colors p and q by the amount given and area.
*/
static inline void MagickPixelCompositeAreaBlend(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,const MagickRealType area,
  MagickPixelPacket *composite)
{
  MagickPixelCompositePlus(p,(MagickRealType) QuantumRange-(1.0-area)*
    (QuantumRange-alpha),q,(MagickRealType) (QuantumRange-area*(QuantumRange-
    beta)),composite);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

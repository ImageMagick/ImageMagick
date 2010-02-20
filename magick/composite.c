/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        CCCC   OOO   M   M  PPPP    OOO   SSSSS  IIIII  TTTTT  EEEEE         %
%       C      O   O  MM MM  P   P  O   O  SS       I      T    E             %
%       C      O   O  M M M  PPPP   O   O   SSS     I      T    EEE           %
%       C      O   O  M   M  P      O   O     SS    I      T    E             %
%        CCCC   OOO   M   M  P       OOO   SSSSS  IIIII    T    EEEEE         %
%                                                                             %
%                                                                             %
%                     MagickCore Image Composite Methods                      %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/artifact.h"
#include "magick/cache-view.h"
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/constitute.h"
#include "magick/draw.h"
#include "magick/fx.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/memory_.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/quantum.h"
#include "magick/resample.h"
#include "magick/resource_.h"
#include "magick/string_.h"
#include "magick/utility.h"
#include "magick/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p o s i t e I m a g e C h a n n e l                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompositeImageChannel() returns the second image composited onto the first
%  at the specified offset, using the specified composite method.
%
%  The format of the CompositeImageChannel method is:
%
%      MagickBooleanType CompositeImage(Image *image,
%        const CompositeOperator compose,Image *composite_image,
%        const long x_offset,const long y_offset)
%      MagickBooleanType CompositeImageChannel(Image *image,
%        const ChannelType channel,const CompositeOperator compose,
%        Image *composite_image,const long x_offset,const long y_offset)
%
%  A description of each parameter follows:
%
%    o image: the destination image, modified by he composition
%
%    o channel: the channel.
%
%    o compose: This operator affects how the composite is applied to
%      the image.  The operators and how they are utilized are listed here
%      http://www.w3.org/TR/SVG12/#compositing.
%
%    o composite_image: the composite (source) image.
%
%    o x_offset: the column offset of the composited image.
%
%    o y_offset: the row offset of the composited image.
%
%  Extra Controls from Image meta-data in 'composite_image' (artifacts)
%
%    o "compose:args"
%        A string containing extra numerical arguments for specific compose
%        methods, generally expressed as a 'geometry' or a comma separated list
%        of numbers.
%
%        Compose methods needing such arguments include "BlendCompositeOp" and
%        "DisplaceCompositeOp".
%
%    o "compose:outside-overlay"
%        Modify how the composition is to effect areas not directly covered
%        by the 'composite_image' at the offset given.  Normally this is
%        dependant on the 'compose' method, especially Duff-Porter methods.
%
%        If set to "false" then disable all normal handling of pixels not
%        covered by the composite_image.  Typically used for repeated tiling
%        of the composite_image by the calling API.
%
%        Previous to IM v6.5.3-3  this was called "modify-outside-overlay"
%
*/

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}
static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

/*
** Programmers notes on SVG specification.
**
** A Composition is defined by...
**   Color Function :  f(Sc,Dc)  where Sc and Dc are the normizalized colors
**    Blending areas :  X = 1    for area of overlap   ie: f(Sc,Dc)
**                      Y = 1    for source preserved
**                      Z = 1    for destination preserved
**
** Conversion to transparency (then optimized)
**    Dca' = f(Sc, Dc)*Sa*Da + Y*Sca*(1-Da) + Z*Dca*(1-Sa)
**    Da'  = X*Sa*Da + Y*Sa*(1-Da) + Z*Da*(1-Sa)
**
** Where...
**   Sca = Sc*Sa     normalized Source color divided by Source alpha
**   Dca = Dc*Da     normalized Dest color divided by Dest alpha
**   Dc' = Dca'/Da'  the desired color value for this channel.
**
** Da' in in the follow formula as 'gamma'  The resulting alpla value.
**
**
** Most functions use a blending mode of over (X=1,Y=1,Z=1)
** this results in the following optimizations...
**   gamma = Sa+Da-Sa*Da;
**   gamma = 1 - QuantiumScale*alpha * QuantiumScale*beta;
**   opacity = QuantiumScale*alpha*beta;  // over blend, optimized 1-Gamma
*/

static inline MagickRealType Add(const MagickRealType p,const MagickRealType q)
{
  MagickRealType
    pixel;

  pixel=p+q;
  if (pixel > QuantumRange)
    pixel-=(QuantumRange+1.0);
  return(pixel);
}

static inline void CompositeAdd(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  composite->red=Add(p->red,q->red);
  composite->green=Add(p->green,q->green);
  composite->blue=Add(p->blue,q->blue);
  composite->opacity=Add(alpha,beta);
  if (q->colorspace == CMYKColorspace)
    composite->index=Add(p->index,q->index);
}

static inline MagickRealType Atop(const MagickRealType p,
  const MagickRealType Sa,const MagickRealType q,
  const MagickRealType magick_unused(Da))
{
  return(p*Sa+q*(1.0-Sa));  /* Da optimized out,  Da/gamma => 1.0 */
}

static inline void CompositeAtop(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  composite->opacity=beta;    /* optimized  1.0-Gamma */
  composite->red=Atop(p->red,Sa,q->red,1.0);
  composite->green=Atop(p->green,Sa,q->green,1.0);
  composite->blue=Atop(p->blue,Sa,q->blue,1.0);
  if (q->colorspace == CMYKColorspace)
    composite->index=Atop(p->index,Sa,q->index,1.0);
}

/*
  What is this Composition method for, can't find any specification!
  WARNING this is not doing correct 'over' blend handling (Anthony Thyssen).
*/
static inline void CompositeBumpmap(const MagickPixelPacket *p,
  const MagickRealType magick_unused(alpha),const MagickPixelPacket *q,
  const MagickRealType magick_unused(beta),MagickPixelPacket *composite)
{
  MagickRealType
    intensity;

  intensity=MagickPixelIntensity(p);
  composite->red=QuantumScale*intensity*q->red;
  composite->green=QuantumScale*intensity*q->green;
  composite->blue=QuantumScale*intensity*q->blue;
  composite->opacity=(MagickRealType) QuantumScale*intensity*
    GetOpacityPixelComponent(p);
  if (q->colorspace == CMYKColorspace)
    composite->index=QuantumScale*intensity*q->index;
}

static inline void CompositeClear(const MagickPixelPacket *q,
  MagickPixelPacket *composite)
{
  composite->opacity=(MagickRealType) TransparentOpacity;
  composite->red=0.0;
  composite->green=0.0;
  composite->blue=0.0;
  if (q->colorspace == CMYKColorspace)
    composite->index=0.0;
}

static MagickRealType ColorBurn(const MagickRealType Sca,
  const MagickRealType Sa, const MagickRealType Dca,const MagickRealType Da)
{
#if 0
  /*
    Oct 2004 SVG specification.
  */
  if (Sca*Da + Dca*Sa <= Sa*Da)
    return(Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Sa*(Sca*Da+Dca*Sa-Sa*Da)/Sca + Sca*(1.0-Da) + Dca*(1.0-Sa));
#else
  /*
    March 2009 SVG specification.
  */
  if ((fabs(Sca) < MagickEpsilon) && (fabs(Dca-Da) < MagickEpsilon))
    return(Sa*Da+Dca*(1.0-Sa));
  if (Sca < MagickEpsilon)
    return(Dca*(1.0-Sa));
  return(Sa*Da-Sa*MagickMin(Da,(Da-Dca)*Sa/Sca)+Sca*(1.0-Da)+Dca*(1.0-Sa));
#endif
}

static inline void CompositeColorBurn(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*ColorBurn(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*ColorBurn(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*ColorBurn(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*ColorBurn(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}


static MagickRealType ColorDodge(const MagickRealType Sca,
  const MagickRealType Sa, const MagickRealType Dca,const MagickRealType Da)
{
#if 0
  /*
    Oct 2004 SVG specification.
  */
  if ((Sca*Da+Dca*Sa) >= Sa*Da)
    return( Sa*Da + Sca*(1.0-Da) + Dca*(1.0-Sa) );
  return( Dca*Sa*Sa/(Sa-Sca) + Sca*(1.0-Da) + Dca*(1.0-Sa) );
#endif
#if 0
  /*
    New specification, March 2009 SVG specification.  This specification was
    also wrong of non-overlap cases.
  */
  if ((fabs(Sca-Sa) < MagickEpsilon) && (fabs(Dca) < MagickEpsilon))
    return(Sca*(1.0-Da));
  if (fabs(Sca-Sa) < MagickEpsilon)
    return(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Sa*MagickMin(Da,Dca*Sa/(Sa-Sca)));
#endif
  /*
    Working from first principles using the original formula:

       f(Sc,Dc) = Dc/(1-Sc)

    This works correctly! Looks like the 2004 model was right but just
    required a extra condition for correct handling.
  */
  if ((fabs(Sca-Sa) < MagickEpsilon) && (fabs(Dca) < MagickEpsilon))
    return(Sca*(1.0-Da)+Dca*(1.0-Sa));
  if (fabs(Sca-Sa) < MagickEpsilon)
    return(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Dca*Sa*Sa/(Sa-Sca)+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeColorDodge(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*ColorDodge(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*ColorDodge(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*ColorDodge(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*ColorDodge(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static inline MagickRealType Darken(const MagickRealType p,
  const MagickRealType alpha,const MagickRealType q,const MagickRealType beta)
{
  if (p < q)
    return(MagickOver_(p,alpha,q,beta));  /* src-over */
  return(MagickOver_(q,beta,p,alpha));    /* dst-over */
}

static inline void CompositeDarken(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    gamma;

  gamma=1.0-QuantumScale*QuantumScale*alpha*beta;
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Darken(p->red,alpha,q->red,beta);
  composite->green=gamma*Darken(p->green,alpha,q->green,beta);
  composite->blue=gamma*Darken(p->blue,alpha,q->blue,beta);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Darken(p->index,alpha,q->index,beta);
}

static inline MagickRealType Difference(const MagickRealType p,
  const MagickRealType Sa,const MagickRealType q,const MagickRealType Da)
{
  /*
    Optimized by Multipling by QuantumRange (taken from gamma).
  */
  return(Sa*p+Da*q-Sa*Da*2.0*MagickMin(p,q));
}

static inline void CompositeDifference(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  /*
    Values not normalized as an optimization.
  */
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Difference(p->red,Sa,q->red,Da);
  composite->green=gamma*Difference(p->green,Sa,q->green,Da);
  composite->blue=gamma*Difference(p->blue,Sa,q->blue,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Difference(p->index,Sa,q->index,Da);
}

static MagickRealType Divide(const MagickRealType Sca,const MagickRealType Sa,
  const MagickRealType Dca,const MagickRealType Da)
{
  /*
    Divide:

      f(Sc,Dc) = Sc/Dc

    But with appropriate handling for special case of Dc == 0 specifically
    f(Black,Black)  = Black and f(non-Black,Black) = White.  It is however
    also important to correctly do 'over' alpha blending which is why it
    becomes so complex looking.
  */
  if ((fabs(Sca) < MagickEpsilon) && (fabs(Dca) < MagickEpsilon))
    return(Sca*(1.0-Da)+Dca*(1.0-Sa));
  if (fabs(Dca) < MagickEpsilon)
    return(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Sca*Da*Da/Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeDivide(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Divide(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*Divide(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*Divide(QuantumScale*p->blue*Sa,Sa,QuantumScale*
     q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Divide(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static MagickRealType Exclusion(const MagickRealType Sca,
  const MagickRealType Sa, const MagickRealType Dca,const MagickRealType Da)
{
  return(Sca*Da+Dca*Sa-2.0*Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeExclusion(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    gamma,
    Sa,
    Da;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Exclusion(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*Exclusion(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*Exclusion(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Exclusion(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static MagickRealType HardLight(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  if ((2.0*Sca) < Sa)
    return(2.0*Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Sa*Da-2.0*(Da-Dca)*(Sa-Sca)+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeHardLight(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*HardLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*HardLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*HardLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*HardLight(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static void CompositeHSB(const MagickRealType red,const MagickRealType green,
  const MagickRealType blue,double *hue,double *saturation,double *brightness)
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
  max=(red > green ? red : green);
  if (blue > max)
    max=blue;
  min=(red < green ? red : green);
  if (blue < min)
    min=blue;
  *hue=0.0;
  *saturation=0.0;
  *brightness=(double) (QuantumScale*max);
  if (max == 0.0)
    return;
  *saturation=(double) (1.0-min/max);
  delta=max-min;
  if (delta == 0.0)
    return;
  if (red == max)
    *hue=(double) ((green-blue)/delta);
  else
    if (green == max)
      *hue=(double) (2.0+(blue-red)/delta);
    else
      if (blue == max)
        *hue=(double) (4.0+(red-green)/delta);
  *hue/=6.0;
  if (*hue < 0.0)
    *hue+=1.0;
}

static inline MagickRealType In(const MagickRealType p,
  const MagickRealType alpha,const MagickRealType magick_unused(q),
  const MagickRealType beta)
{
  return((1.0-QuantumScale*alpha)*p*(1.0-QuantumScale*beta));
}

static inline void CompositeIn(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    gamma;

  gamma=(1.0-QuantumScale*alpha)*(1.0-QuantumScale*beta);
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*In(p->red,alpha,q->red,beta);
  composite->green=gamma*In(p->green,alpha,q->green,beta);
  composite->blue=gamma*In(p->blue,alpha,q->blue,beta);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*In(p->index,alpha,q->index,beta);
}

static inline MagickRealType Lighten(const MagickRealType p,
  const MagickRealType alpha,const MagickRealType q,const MagickRealType beta)
{
   if (p > q)
     return(MagickOver_(p,alpha,q,beta));  /* src-over */
   return(MagickOver_(q,beta,p,alpha));    /* dst-over */
}

static inline void CompositeLighten(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    gamma;

  composite->opacity=QuantumScale*alpha*beta;    /* optimized 1-gamma */
  gamma=1.0-QuantumScale*composite->opacity;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Lighten(p->red,alpha,q->red,beta);
  composite->green=gamma*Lighten(p->green,alpha,q->green,beta);
  composite->blue=gamma*Lighten(p->blue,alpha,q->blue,beta);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Lighten(p->index,alpha,q->index,beta);
}

static inline void CompositeLinearDodge(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  /*
    Operation performed directly - not need for sub-routine.
  */
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*(p->red*Sa+q->red*Da);
  composite->green=gamma*(p->green*Sa+q->green*Da);
  composite->blue=gamma*(p->blue*Sa+q->blue*Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*(p->index*Sa+q->index*Da);
}


static inline MagickRealType LinearBurn(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  /*
    LinearLight: as defined by Abode Photoshop, according to
    http://www.simplefilter.de/en/basics/mixmods.html is:

    f(Sc,Dc) = Dc + Sc - 1
  */
  return(Sca+Dca-Sa*Da);
}

static inline void CompositeLinearBurn(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*LinearBurn(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*LinearBurn(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*LinearBurn(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*LinearBurn(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static inline MagickRealType LinearLight(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
#if 0
  /*
    Previous formula, only valid for fully-opaque images.
  */
  return(Dca+2*Sca-1.0);
#else
  /*
    LinearLight: as defined by Abode Photoshop, according to
    http://www.simplefilter.de/en/basics/mixmods.html is:

      f(Sc,Dc) = Dc + 2*Sc - 1
  */
  return((Sca-Sa)*Da+Sca+Dca);
#endif
}

static inline void CompositeLinearLight(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*LinearLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*LinearLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*LinearLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*LinearLight(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static inline MagickRealType Mathematics(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da,
  const GeometryInfo *geometry_info)
{
  /*
    'Mathematics' a free form user control mathematical composition is defined
    as...

       f(Sc,Dc) = A*Sc*Dc + B*Sc + C*Dc + D

    Where the arguments A,B,C,D are (currently) passed to composite as
    a command separated 'geometry' string in "compose:args" image artifact.

       A = a->rho,   B = a->sigma,  C = a->xi,  D = a->psi

    Applying the SVG transparency formula (see above), we get...

     Dca' = Sa*Da*f(Sc,Dc) + Sca*(1.0-Da) + Dca*(1.0-Sa)

     Dca' = A*Sca*Dca + B*Sca*Da + C*Dca*Sa + D*Sa*Da + Sca*(1.0-Da) +
       Dca*(1.0-Sa)
  */
  return(geometry_info->rho*Sca*Dca+geometry_info->sigma*Sca*Da+
    geometry_info->xi*Dca*Sa+geometry_info->psi*Sa*Da+Sca*(1.0-Da)+
    Dca*(1.0-Sa));
}

static inline void CompositeMathematics(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const GeometryInfo *args,
  MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*GetOpacityPixelComponent(p);
  Da=1.0-QuantumScale*q->opacity;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Mathematics(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da,args);
  composite->green=gamma*Mathematics(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da,args);
  composite->blue=gamma*Mathematics(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da,args);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Mathematics(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da,args);
}

static inline MagickRealType Minus(const MagickRealType Sca,
  const MagickRealType Dca)
{
  return(Sca-Dca);
}

static inline void CompositeMinus(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa-Da);  /* is this correct?  - I do not think so! */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Minus(p->red*Sa,q->red*Da);
  composite->green=gamma*Minus(p->green*Sa,q->green*Da);
  composite->blue=gamma*Minus(p->blue*Sa,q->blue*Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Minus(p->index*Sa,q->index*Da);
}

static  inline MagickRealType Multiply(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  return(Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeMultiply(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Multiply(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*Multiply(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*Multiply(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Multiply(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static inline MagickRealType Out(const MagickRealType p,
  const MagickRealType alpha,const MagickRealType magick_unused(q),
  const MagickRealType beta)
{
  return((1.0-QuantumScale*alpha)*p*QuantumScale*beta);
}

static inline void CompositeOut(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    gamma;

  gamma=(1.0-QuantumScale*alpha)*QuantumScale*beta;
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Out(p->red,alpha,q->red,beta);
  composite->green=gamma*Out(p->green,alpha,q->green,beta);
  composite->blue=gamma*Out(p->blue,alpha,q->blue,beta);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Out(p->index,alpha,q->index,beta);
}

static inline void CompositeOver(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickPixelCompositeOver(p,alpha,q,beta,composite);
}

static MagickRealType PegtopLight(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  /*
    PegTOP Soft-Light alternative: A continuous version of the Softlight
    function, producing very similar results however it does not take into
    account alpha channel.

    f(Sc,Dc) = Dc^2*(1-2*Sc) + 2*Sc*Dc

    See http://www.pegtop.net/delphi/articles/blendmodes/softlight.htm.
  */
  if (fabs(Da) < MagickEpsilon)
    return(Sca);
  return(Dca*Dca*(Sa-2*Sca)/Da+Sca*(2*Dca+1-Da)+Dca*(1-Sa));
}

static inline void CompositePegtopLight(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*PegtopLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*PegtopLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*PegtopLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*PegtopLight(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static MagickRealType PinLight(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  /*
    PinLight: A Photoshop 7 composition method
    http://www.simplefilter.de/en/basics/mixmods.html

    f(Sc,Dc) = Dc<2*Sc-1 ? 2*Sc-1 : Dc>2*Sc   ? 2*Sc : Dc
  */
  if (Dca*Sa < Da*(2*Sca-Sa))
    return(Sca*(Da+1.0)-Sa*Da+Dca*(1.0-Sa));
  if ((Dca*Sa) > (2*Sca*Da))
    return(Sca*Da+Sca+Dca*(1.0-Sa));
  return(Sca*(1.0-Da)+Dca);
}

static inline void CompositePinLight(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*PinLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*PinLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*PinLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*PinLight(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static inline void CompositePlus(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickPixelCompositePlus(p,alpha,q,beta,composite);
}

static inline MagickRealType Screen(const MagickRealType Sca,
  const MagickRealType Dca)
{
  return(Sca+Dca-Sca*Dca);
}

static inline void CompositeScreen(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Screen(QuantumScale*p->red*Sa,QuantumScale*
    q->red*Da);
  composite->green=gamma*Screen(QuantumScale*p->green*Sa,QuantumScale*
    q->green*Da);
  composite->blue=gamma*Screen(QuantumScale*p->blue*Sa,QuantumScale*
    q->blue*Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Screen(QuantumScale*p->index*Sa,QuantumScale*
      q->index*Da);
}

static MagickRealType SoftLight(const MagickRealType Sca,
  const MagickRealType Sa, const MagickRealType Dca, const MagickRealType Da)
{
#if 0
  /*
    Oct 2004 SVG specification -- spec discovered to be incorrect
    See  http://lists.w3.org/Archives/Public/www-svg/2009Feb/0014.html.
  */
  if (2.0*Sca < Sa)
    return(Dca*(Sa-(1.0-Dca/Da)*(2.0*Sca-Sa))+Sca*(1.0-Da)+Dca*(1.0-Sa));
  if (8.0*Dca <= Da)
    return(Dca*(Sa-(1.0-Dca/Da)*(2.0*Sca-Sa)*(3.0-8.0*Dca/Da))+
      Sca*(1.0-Da)+Dca*(1.0-Sa));
  return((Dca*Sa+(pow(Dca/Da,0.5)*Da-Dca)*(2.0*Sca-Sa))+Sca*(1.0-Da)+
    Dca*(1.0-Sa));
#else
  MagickRealType
    alpha,
    beta;

  /*
    New specification:  March 2009 SVG specification.
  */
  alpha=Dca/Da;
  if ((2.0*Sca) < Sa)
    return(Dca*(Sa+(2.0*Sca-Sa)*(1.0-alpha))+Sca*(1.0-Da)+Dca*(1.0-Sa));
  if (((2.0*Sca) > Sa) && ((4.0*Dca) <= Da))
    {
      beta=Dca*Sa+Da*(2.0*Sca-Sa)*(4.0*alpha*(4.0*alpha+1.0)*(alpha-1.0)+7.0*
        alpha)+Sca*(1.0-Da)+Dca*(1.0-Sa);
      return(beta);
    }
  beta=Dca*Sa+Da*(2.0*Sca-Sa)*(pow(alpha,0.5)-alpha)+Sca*(1.0-Da)+Dca*(1.0-Sa);
  return(beta);
#endif
}

static inline void CompositeSoftLight(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*SoftLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*SoftLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*SoftLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*SoftLight(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static inline MagickRealType Subtract(const MagickRealType p,
  const MagickRealType magick_unused(alpha),const MagickRealType q,
  const MagickRealType magick_unused(beta))
{
  MagickRealType
    pixel;

  pixel=p-q;
  if (pixel < 0.0)
    pixel+=(QuantumRange+1.0);
  return(pixel);
}

static inline void CompositeSubtract(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  composite->red=Subtract(p->red,alpha,q->red,beta);
  composite->green=Subtract(p->green,alpha,q->green,beta);
  composite->blue=Subtract(p->blue,alpha,q->blue,beta);
  if (q->colorspace == CMYKColorspace)
    composite->index=Subtract(p->index,alpha,q->index,beta);
}

static inline MagickRealType Threshold(const MagickRealType p,
  const MagickRealType magick_unused(alpha),const MagickRealType q,
  const MagickRealType magick_unused(beta),const MagickRealType threshold,
  const MagickRealType amount)
{
  MagickRealType
    delta;

  delta=p-q;
  if ((MagickRealType) fabs((double) (2.0*delta)) < threshold)
    return(q);
  return(q+delta*amount);
}

static inline void CompositeThreshold(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,const MagickRealType threshold,
  const MagickRealType amount,MagickPixelPacket *composite)
{
  composite->red=Threshold(p->red,alpha,q->red,beta,threshold,amount);
  composite->green=Threshold(p->green,alpha,q->green,beta,threshold,amount);
  composite->blue=Threshold(p->blue,alpha,q->blue,beta,threshold,amount);
  composite->opacity=(MagickRealType) QuantumRange-
    Threshold(p->opacity,alpha,q->opacity,beta,threshold,amount);
  if (q->colorspace == CMYKColorspace)
    composite->index=Threshold(p->index,alpha,q->index,beta,threshold,amount);
}

static MagickRealType VividLight(const MagickRealType Sca,
  const MagickRealType Sa, const MagickRealType Dca, const MagickRealType Da)
{
  /*
    VividLight: A Photoshop 7 composition method.  See
    http://www.simplefilter.de/en/basics/mixmods.html.

    f(Sc,Dc) = (2*Sc < 1) ? 1-(1-Dc)/(2*Sc) : Dc/(2*(1-Sc))
  */
  if ((fabs(Sa) < MagickEpsilon) || (fabs(Sca-Sa) < MagickEpsilon))
    return(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
  if ((2*Sca) <= Sa)
    return(Sa*(Da+Sa*(Dca-Da)/(2.0*Sca))+Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Dca*Sa*Sa/(2.0*(Sa-Sca))+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeVividLight(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*VividLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*VividLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*VividLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*VividLight(QuantumScale*p->index*Sa,Sa,QuantumScale*
      q->index*Da,Da);
}

static MagickRealType Xor(const MagickRealType Sca,const MagickRealType Sa,
  const MagickRealType Dca,const MagickRealType Da)
{
  return(Sca*(1-Da)+Dca*(1-Sa));
}

static inline void CompositeXor(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*alpha;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*beta;
  gamma=Sa+Da-2*Sa*Da;        /* Xor blend mode X=0,Y=1,Z=1 */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  /*
    Optimized by multipling QuantumRange taken from gamma.
  */
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Xor(p->red*Sa,Sa,q->red*Da,Da);
  composite->green=gamma*Xor(p->green*Sa,Sa,q->green*Da,Da);
  composite->blue=gamma*Xor(p->blue*Sa,Sa,q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Xor(p->index*Sa,Sa,q->index*Da,Da);
}

static void HSBComposite(const double hue,const double saturation,
  const double brightness,MagickRealType *red,MagickRealType *green,
  MagickRealType *blue)
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
  assert(red != (MagickRealType *) NULL);
  assert(green != (MagickRealType *) NULL);
  assert(blue != (MagickRealType *) NULL);
  if (saturation == 0.0)
    {
      *red=(MagickRealType) QuantumRange*brightness;
      *green=(*red);
      *blue=(*red);
      return;
    }
  h=6.0*(hue-floor(hue));
  f=h-floor((double) h);
  p=brightness*(1.0-saturation);
  q=brightness*(1.0-saturation*f);
  t=brightness*(1.0-saturation*(1.0-f));
  switch ((int) h)
  {
    case 0:
    default:
    {
      *red=(MagickRealType) QuantumRange*brightness;
      *green=(MagickRealType) QuantumRange*t;
      *blue=(MagickRealType) QuantumRange*p;
      break;
    }
    case 1:
    {
      *red=(MagickRealType) QuantumRange*q;
      *green=(MagickRealType) QuantumRange*brightness;
      *blue=(MagickRealType) QuantumRange*p;
      break;
    }
    case 2:
    {
      *red=(MagickRealType) QuantumRange*p;
      *green=(MagickRealType) QuantumRange*brightness;
      *blue=(MagickRealType) QuantumRange*t;
      break;
    }
    case 3:
    {
      *red=(MagickRealType) QuantumRange*p;
      *green=(MagickRealType) QuantumRange*q;
      *blue=(MagickRealType) QuantumRange*brightness;
      break;
    }
    case 4:
    {
      *red=(MagickRealType) QuantumRange*t;
      *green=(MagickRealType) QuantumRange*p;
      *blue=(MagickRealType) QuantumRange*brightness;
      break;
    }
    case 5:
    {
      *red=(MagickRealType) QuantumRange*brightness;
      *green=(MagickRealType) QuantumRange*p;
      *blue=(MagickRealType) QuantumRange*q;
      break;
    }
  }
}

MagickExport MagickBooleanType CompositeImage(Image *image,
  const CompositeOperator compose,const Image *composite_image,
  const long x_offset,const long y_offset)
{
  MagickBooleanType
    status;

  status=CompositeImageChannel(image,DefaultChannels,compose,composite_image,
    x_offset,y_offset);
  return(status);
}

MagickExport MagickBooleanType CompositeImageChannel(Image *image,
  const ChannelType magick_unused(channel),const CompositeOperator compose,
  const Image *composite_image,const long x_offset,const long y_offset)
{
#define CompositeImageTag  "Composite/Image"

  CacheView
    *composite_view,
    *image_view;

  const char
    *value;

  double
    sans;

  ExceptionInfo
    *exception;

  GeometryInfo
    geometry_info;

  Image
    *destination_image;

  long
    progress,
    y;

  MagickBooleanType
    modify_outside_overlay,
    status;

  MagickPixelPacket
    zero;

  MagickRealType
    amount,
    destination_dissolve,
    midpoint,
    percent_brightness,
    percent_saturation,
    source_dissolve,
    threshold;

  MagickStatusType
    flags;

  /*
    Prepare composite image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(composite_image != (Image *) NULL);
  assert(composite_image->signature == MagickSignature);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  GetMagickPixelPacket(image,&zero);
  destination_image=(Image *) NULL;
  amount=0.5;
  destination_dissolve=1.0;
  modify_outside_overlay=MagickFalse;
  percent_brightness=100.0;
  percent_saturation=100.0;
  source_dissolve=1.0;
  threshold=0.05f;
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
    {
      /*
        Modify destination outside the overlaid region.
      */
      modify_outside_overlay=MagickTrue;
      break;
    }
    case OverCompositeOp:
    {
      if (image->matte != MagickFalse)
        break;
      if (composite_image->matte != MagickFalse)
        break;
    }
    case CopyCompositeOp:
    {
      if ((x_offset < 0) || (y_offset < 0))
        break;
      if ((x_offset+(long) composite_image->columns) >= (long) image->columns)
        break;
      if ((y_offset+(long) composite_image->rows) >= (long) image->rows)
        break;
      status=MagickTrue;
      exception=(&image->exception);
      image_view=AcquireCacheView(image);
      composite_view=AcquireCacheView(composite_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (long) composite_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const IndexPacket
          *composite_indexes;

        register const PixelPacket
          *p;

        register IndexPacket
          *indexes;

        register PixelPacket
          *q;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        q=GetCacheViewAuthenticPixels(image_view,x_offset,y+y_offset,
          composite_image->columns,1,exception);
        if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        composite_indexes=GetCacheViewVirtualIndexQueue(composite_view);
        indexes=GetCacheViewAuthenticIndexQueue(image_view);
        (void) CopyMagickMemory(q,p,composite_image->columns*sizeof(*p));
        if ((indexes != (IndexPacket *) NULL) &&
            (composite_indexes != (const IndexPacket *) NULL))
          (void) CopyMagickMemory(indexes,composite_indexes,
            composite_image->columns*sizeof(*indexes));
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp critical (MagickCore_TextureImage)
#endif
            proceed=SetImageProgress(image,CompositeImageTag,y,image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      composite_view=DestroyCacheView(composite_view);
      image_view=DestroyCacheView(image_view);
      return(status);
    }
    case CopyOpacityCompositeOp:
    case ChangeMaskCompositeOp:
    {
      /*
        Modify destination outside the overlaid region and require an alpha
        channel to exist, to add transparency.
      */
      if (image->matte == MagickFalse)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel);
      modify_outside_overlay=MagickTrue;
      break;
    }
    case BlurCompositeOp:
    {
      CacheView
        *composite_view,
        *destination_view;

      MagickPixelPacket
        pixel;

      MagickRealType
        angle_range,
        angle_start,
        height,
        width;

      ResampleFilter
        *resample_filter;

      SegmentInfo
        blur;

      /*
        Blur Image dictated by an overlay gradient map: X = red_channel;
          Y = green_channel; compose:args =  x_scale[,y_scale[,angle]].
      */
      destination_image=CloneImage(image,image->columns,image->rows,MagickTrue,
        &image->exception);
      if (destination_image == (Image *) NULL)
        return(MagickFalse);
      /*
        Determine the horizontal and vertical maximim blur.
      */
      SetGeometryInfo(&geometry_info);
      flags=NoValue;
      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        flags=ParseGeometry(value,&geometry_info);
      if ((flags & WidthValue) == 0 )
        {
          destination_image=DestroyImage(destination_image);
          return(MagickFalse);
        }
      width=geometry_info.rho;
      height=geometry_info.sigma;
      blur.x1=geometry_info.rho;
      blur.x2=0.0;
      blur.y1=0.0;
      blur.y2=geometry_info.sigma;
      angle_start=0.0;
      angle_range=0.0;
      if ((flags & HeightValue) == 0)
        blur.y2=blur.x1;
      if ((flags & XValue) != 0 )
        {
          MagickRealType
            angle;

          angle=DegreesToRadians(geometry_info.xi);
          blur.x1=width*cos(angle);
          blur.x2=width*sin(angle);
          blur.y1=(-height*sin(angle));
          blur.y2=height*cos(angle);
        }
      if ((flags & YValue) != 0 )
        {
          angle_start=DegreesToRadians(geometry_info.xi);
          angle_range=DegreesToRadians(geometry_info.psi)-angle_start;
        }
      /*
        Blur Image by resampling;
      */
      pixel=zero;
      exception=(&image->exception);
      resample_filter=AcquireResampleFilter(image,&image->exception);
      SetResampleFilter(resample_filter,GaussianFilter,1.0);
      destination_view=AcquireCacheView(destination_image);
      composite_view=AcquireCacheView(composite_image);
      for (y=0; y < (long) composite_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const PixelPacket
          *restrict p;

        register PixelPacket
          *restrict r;

        register IndexPacket
          *restrict destination_indexes;

        register long
          x;

        if (((y+y_offset) < 0) || ((y+y_offset) >= (long) image->rows))
          continue;
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        r=QueueCacheViewAuthenticPixels(destination_view,0,y,
          destination_image->columns,1,&image->exception);
        if ((p == (const PixelPacket *) NULL) || (r == (PixelPacket *) NULL))
          break;
        destination_indexes=GetCacheViewAuthenticIndexQueue(destination_view);
        for (x=0; x < (long) composite_image->columns; x++)
        {
          if (((x_offset+x) < 0) || ((x_offset+x) >= (long) image->columns))
            {
              p++;
              continue;
            }
          if (fabs(angle_range) > MagickEpsilon)
            {
              MagickRealType
                angle;

              angle=angle_start+angle_range*QuantumScale*
                GetBluePixelComponent(p);
              blur.x1=width*cos(angle);
              blur.x2=width*sin(angle);
              blur.y1=(-height*sin(angle));
              blur.y2=height*cos(angle);
            }
          ScaleResampleFilter(resample_filter,blur.x1*QuantumScale*p->red,
            blur.y1*QuantumScale*p->green,blur.x2*QuantumScale*p->red,
            blur.y2*QuantumScale*GetGreenPixelComponent(p));
          (void) ResamplePixelColor(resample_filter,(double) x_offset+x,
            (double) y_offset+y,&pixel);
          SetPixelPacket(destination_image,&pixel,r,destination_indexes+x);
          p++;
          r++;
        }
        sync=SyncCacheViewAuthenticPixels(destination_view,exception);
        if (sync == MagickFalse)
          break;
      }
      resample_filter=DestroyResampleFilter(resample_filter);
      composite_view=DestroyCacheView(composite_view);
      destination_view=DestroyCacheView(destination_view);
      composite_image=destination_image;
      break;
    }
    case DisplaceCompositeOp:
    case DistortCompositeOp:
    {
      CacheView
        *composite_view,
        *destination_view;

      MagickPixelPacket
        pixel;

      MagickRealType
        horizontal_scale,
        vertical_scale;

      PointInfo
        center,
        offset;

      register IndexPacket
        *restrict destination_indexes;

      register PixelPacket
        *restrict r;

      ResampleFilter
        *resample_filter;

      /*
        Displace/Distort based on overlay gradient map:
          X = red_channel;  Y = green_channel;
          compose:args = x_scale[,y_scale[,center.x,center.y]]
      */
      destination_image=CloneImage(image,image->columns,image->rows,MagickTrue,
        &image->exception);
      if (destination_image == (Image *) NULL)
        return(MagickFalse);
      SetGeometryInfo(&geometry_info);
      flags=NoValue;
      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        flags=ParseGeometry(value,&geometry_info);
      if ((flags & (WidthValue|HeightValue)) == 0 )
        {
          if ((flags & AspectValue) == 0)
            {
              horizontal_scale=(MagickRealType) (composite_image->columns-1.0)/
                2.0;
              vertical_scale=(MagickRealType) (composite_image->rows-1.0)/2.0;
            }
          else
            {
              horizontal_scale=(MagickRealType) (image->columns-1.0)/2.0;
              vertical_scale=(MagickRealType) (image->rows-1.0)/2.0;
            }
        }
      else
        {
          horizontal_scale=geometry_info.rho;
          vertical_scale=geometry_info.sigma;
          if ((flags & PercentValue) != 0)
            {
              if ((flags & AspectValue) == 0)
                {
                  horizontal_scale*=(composite_image->columns-1.0)/200.0;
                  vertical_scale*=(composite_image->rows-1.0)/200.0;
                }
              else
                {
                  horizontal_scale*=(image->columns-1.0)/200.0;
                  vertical_scale*=(image->rows-1.0)/200.0;
                }
            }
          if ((flags & HeightValue) == 0)
            vertical_scale=horizontal_scale;
        }
      /*
        Determine fixed center point for absolute distortion map
         Absolute distort ==
           Displace offset relative to a fixed absolute point
           Select that point according to +X+Y user inputs.
           default = center of overlay image
           flag '!' = locations/percentage relative to background image
      */
      center.x=(MagickRealType) x_offset;
      center.y=(MagickRealType) y_offset;
      if (compose == DistortCompositeOp)
        {
          if ((flags & XValue) == 0)
            if ((flags & AspectValue) == 0)
              center.x=(MagickRealType) x_offset+(composite_image->columns-1)/
                2.0;
            else
              center.x=((MagickRealType) image->columns-1)/2.0;
          else
            if ((flags & AspectValue) == 0)
              center.x=(MagickRealType) x_offset+geometry_info.xi;
            else
              center.x=geometry_info.xi;
          if ((flags & YValue) == 0)
            if ((flags & AspectValue) == 0)
              center.y=(MagickRealType) y_offset+(composite_image->rows-1)/2.0;
            else
              center.y=((MagickRealType) image->rows-1)/2.0;
          else
            if ((flags & AspectValue) == 0)
              center.y=(MagickRealType) y_offset+geometry_info.psi;
            else
              center.y=geometry_info.psi;
        }
      /*
        Shift the pixel offset point as defined by the provided,
        displacement/distortion map.  -- Like a lens...
      */
      pixel=zero;
      exception=(&image->exception);
      resample_filter=AcquireResampleFilter(image,&image->exception);
      destination_view=AcquireCacheView(destination_image);
      composite_view=AcquireCacheView(composite_image);
      for (y=0; y < (long) composite_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const PixelPacket
          *restrict p;

        register long
          x;

        if (((y+y_offset) < 0) || ((y+y_offset) >= (long) image->rows))
          continue;
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        r=QueueCacheViewAuthenticPixels(destination_view,0,y,
          destination_image->columns,1,&image->exception);
        if ((p == (const PixelPacket *) NULL) || (r == (PixelPacket *) NULL))
          break;
        destination_indexes=GetCacheViewAuthenticIndexQueue(destination_view);
        for (x=0; x < (long) composite_image->columns; x++)
        {
          if (((x_offset+x) < 0) || ((x_offset+x) >= (long) image->columns))
            {
              p++;
              continue;
            }
          /*
            Displace the offset.
          */
          offset.x=(horizontal_scale*(p->red-(((MagickRealType) QuantumRange+
            1.0)/2.0)))/(((MagickRealType) QuantumRange+1.0)/2.0)+
            center.x+((compose == DisplaceCompositeOp) ? x : 0);
          offset.y=(vertical_scale*(p->green-(((MagickRealType) QuantumRange+
            1.0)/2.0)))/(((MagickRealType) QuantumRange+1.0)/2.0)+
            center.y+((compose == DisplaceCompositeOp) ? y : 0);
          (void) ResamplePixelColor(resample_filter,(double) offset.x,
            (double) offset.y,&pixel);
          /*
            Mask with 'invalid pixel mask' in alpha channel.
          */
          pixel.opacity=(MagickRealType) QuantumRange*(1.0-(1.0-QuantumScale*
            pixel.opacity)*(1.0-QuantumScale*GetOpacityPixelComponent(p)));
          SetPixelPacket(destination_image,&pixel,r,destination_indexes+x);
          p++;
          r++;
        }
        sync=SyncCacheViewAuthenticPixels(destination_view,exception);
        if (sync == MagickFalse)
          break;
      }
      resample_filter=DestroyResampleFilter(resample_filter);
      composite_view=DestroyCacheView(composite_view);
      destination_view=DestroyCacheView(destination_view);
      composite_image=destination_image;
      break;
    }
    case DissolveCompositeOp:
    {
      /*
        Geometry arguments to dissolve factors.
      */
      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        {
          flags=ParseGeometry(value,&geometry_info);
          source_dissolve=geometry_info.rho/100.0;
          destination_dissolve=1.0;
          if ((source_dissolve-MagickEpsilon) < 0.0)
            source_dissolve=0.0;
          if ((source_dissolve+MagickEpsilon) > 1.0)
            {
              destination_dissolve=2.0-source_dissolve;
              source_dissolve=1.0;
            }
          if ((flags & SigmaValue) != 0)
            destination_dissolve=geometry_info.sigma/100.0;
          if ((destination_dissolve-MagickEpsilon) < 0.0)
            destination_dissolve=0.0;
          modify_outside_overlay=MagickTrue;
          if ((destination_dissolve+MagickEpsilon) > 1.0 )
            {
              destination_dissolve=1.0;
              modify_outside_overlay=MagickFalse;
            }
        }
      break;
    }
    case BlendCompositeOp:
    {
      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        {
          flags=ParseGeometry(value,&geometry_info);
          source_dissolve=geometry_info.rho/100.0;
          destination_dissolve=1.0-source_dissolve;
          if ((flags & SigmaValue) != 0)
            destination_dissolve=geometry_info.sigma/100.0;
          modify_outside_overlay=MagickTrue;
          if ((destination_dissolve+MagickEpsilon) > 1.0)
            modify_outside_overlay=MagickFalse;
        }
      break;
    }
    case MathematicsCompositeOp:
    {
      /*
        Just collect the values from "compose:args", setting.
        Unused values are set to zero automagically.

        Arguments are normally a comma separated list, so this probably should
        be changed to some 'general comma list' parser, (with a minimum
        number of values)
      */
      SetGeometryInfo(&geometry_info);
      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        (void) ParseGeometry(value,&geometry_info);
      break;
    }
    case ModulateCompositeOp:
    {
      /*
        Determine the brightness and saturation scale.
      */
      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        {
          flags=ParseGeometry(value,&geometry_info);
          percent_brightness=geometry_info.rho;
          if ((flags & SigmaValue) != 0)
            percent_saturation=geometry_info.sigma;
        }
      break;
    }
    case ThresholdCompositeOp:
    {
      /*
        Determine the amount and threshold.
      */
      value=GetImageArtifact(composite_image,"compose:args");
      if (value != (char *) NULL)
        {
          flags=ParseGeometry(value,&geometry_info);
          amount=geometry_info.rho;
          threshold=geometry_info.sigma;
          if ((flags & SigmaValue) == 0)
            threshold=0.05f;
        }
      threshold*=QuantumRange;
      break;
    }
    default:
      break;
  }
  value=GetImageArtifact(composite_image,"compose:outside-overlay");
  if (value != (const char *) NULL)
    modify_outside_overlay=IsMagickTrue(value);
  /*
    Composite image.
  */
  status=MagickTrue;
  progress=0;
  midpoint=((MagickRealType) QuantumRange+1.0)/2;
  GetMagickPixelPacket(composite_image,&zero);
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
  composite_view=AcquireCacheView(composite_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    const PixelPacket
      *pixels;

    double
      brightness,
      hue,
      saturation;

    MagickPixelPacket
      composite,
      destination,
      source;

    register const IndexPacket
      *restrict composite_indexes;

    register const PixelPacket
      *restrict p;

    register IndexPacket
      *restrict indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    if (modify_outside_overlay == MagickFalse)
      {
        if (y < y_offset)
          continue;
        if ((y-y_offset) >= (long) composite_image->rows)
          continue;
      }
    /*
      If pixels is NULL, y is outside overlay region.
    */
    pixels=(PixelPacket *) NULL;
    p=(PixelPacket *) NULL;
    if ((y >= y_offset) && ((y-y_offset) < (long) composite_image->rows))
      {
        p=GetCacheViewVirtualPixels(composite_view,0,y-y_offset,
          composite_image->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        pixels=p;
        if (x_offset < 0)
          p-=x_offset;
      }
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
      exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    composite_indexes=GetCacheViewVirtualIndexQueue(composite_view);
    source=zero;
    destination=zero;
    hue=0.0;
    saturation=0.0;
    brightness=0.0;
    for (x=0; x < (long) image->columns; x++)
    {
      if (modify_outside_overlay == MagickFalse)
        {
          if (x < x_offset)
            {
              q++;
              continue;
            }
          if ((x-x_offset) >= (long) composite_image->columns)
            break;
        }
      destination.red=(MagickRealType) q->red;
      destination.green=(MagickRealType) q->green;
      destination.blue=(MagickRealType) q->blue;
      if (image->matte != MagickFalse)
        destination.opacity=(MagickRealType) q->opacity;
      if (image->colorspace == CMYKColorspace)
        {
          destination.red=(MagickRealType) QuantumRange-destination.red;
          destination.green=(MagickRealType) QuantumRange-destination.green;
          destination.blue=(MagickRealType) QuantumRange-destination.blue;
          destination.index=(MagickRealType) (QuantumRange-indexes[x]);
        }
      /*
        Handle destination modifications outside overlaid region.
      */
      composite=destination;
      if ((pixels == (PixelPacket *) NULL) || (x < x_offset) ||
          ((x-x_offset) >= (long) composite_image->columns))
        {
          switch (compose)
          {
            case DissolveCompositeOp:
            case BlendCompositeOp:
            {
              composite.opacity=(MagickRealType) (QuantumRange-
                destination_dissolve*(QuantumRange-composite.opacity));
              break;
            }
            case ClearCompositeOp:
            case SrcCompositeOp:
            {
              CompositeClear(&destination,&composite);
              break;
            }
            case InCompositeOp:
            case SrcInCompositeOp:
            case OutCompositeOp:
            case SrcOutCompositeOp:
            case DstInCompositeOp:
            case DstAtopCompositeOp:
            case CopyOpacityCompositeOp:
            case ChangeMaskCompositeOp:
            {
              composite.opacity=(MagickRealType) TransparentOpacity;
              break;
            }
            default:
            {
              (void) GetOneVirtualMagickPixel(composite_image,x-x_offset,
                y-y_offset,&composite,exception);
              break;
            }
          }
          if (image->colorspace == CMYKColorspace)
            {
              composite.red=(MagickRealType) QuantumRange-composite.red;
              composite.green=(MagickRealType) QuantumRange-composite.green;
              composite.blue=(MagickRealType) QuantumRange-composite.blue;
              composite.index=(MagickRealType) QuantumRange-composite.index;
            }
          q->red=ClampToQuantum(composite.red);
          q->green=ClampToQuantum(composite.green);
          q->blue=ClampToQuantum(composite.blue);
          if (image->matte != MagickFalse)
            q->opacity=ClampToQuantum(composite.opacity);
          if (image->colorspace == CMYKColorspace)
            indexes[x]=ClampToQuantum(composite.index);
          q++;
          continue;
        }
      /*
        Handle normal overlay of source onto destination.
      */
      source.red=(MagickRealType) GetRedPixelComponent(p);
      source.green=(MagickRealType) GetGreenPixelComponent(p);
      source.blue=(MagickRealType) GetBluePixelComponent(p);
      if (composite_image->matte != MagickFalse)
        source.opacity=(MagickRealType) GetOpacityPixelComponent(p);
      if (composite_image->colorspace == CMYKColorspace)
        {
          source.red=(MagickRealType) QuantumRange-source.red;
          source.green=(MagickRealType) QuantumRange-source.green;
          source.blue=(MagickRealType) QuantumRange-source.blue;
          source.index=(MagickRealType) QuantumRange-(MagickRealType)
            composite_indexes[x-x_offset];
        }
      switch (compose)
      {
        case AddCompositeOp:
        {
          CompositeAdd(&source,source.opacity,&destination,destination.opacity,
            &composite);
          break;
        }
        case ClearCompositeOp:
        {
          CompositeClear(&destination,&composite);
          break;
        }
        case SrcCompositeOp:
        case CopyCompositeOp:
        case ReplaceCompositeOp:
        {
          composite=source;
          break;
        }
        case ChangeMaskCompositeOp:
        {
          if ((composite.opacity > ((MagickRealType) QuantumRange/2.0)) ||
              (IsMagickColorSimilar(&source,&destination) != MagickFalse))
            composite.opacity=(MagickRealType) TransparentOpacity;
          else
            composite.opacity=(MagickRealType) OpaqueOpacity;
          break;
        }
        case DivideCompositeOp:
        {
          CompositeDivide(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case DstCompositeOp:
          break;
        case OverCompositeOp:
        case SrcOverCompositeOp:
        {
          CompositeOver(&source,source.opacity,&destination,destination.opacity,
            &composite);
          break;
        }
        case DstOverCompositeOp:
        {
          CompositeOver(&destination,destination.opacity,&source,source.opacity,
            &composite);
          break;
        }
        case SrcInCompositeOp:
        case InCompositeOp:
        {
          CompositeIn(&source,source.opacity,&destination,destination.opacity,
            &composite);
          break;
        }
        case DstInCompositeOp:
        {
          CompositeIn(&destination,destination.opacity,&source,source.opacity,
            &composite);
          break;
        }
        case OutCompositeOp:
        case SrcOutCompositeOp:
        {
          CompositeOut(&source,source.opacity,&destination,destination.opacity,
            &composite);
          break;
        }
        case DstOutCompositeOp:
        {
          CompositeOut(&destination,destination.opacity,&source,source.opacity,
            &composite);
          break;
        }
        case AtopCompositeOp:
        case SrcAtopCompositeOp:
        {
          CompositeAtop(&source,source.opacity,&destination,destination.opacity,
            &composite);
          break;
        }
        case DstAtopCompositeOp:
        {
          CompositeAtop(&destination,destination.opacity,&source,source.opacity,
            &composite);
          break;
        }
        case XorCompositeOp:
        {
          CompositeXor(&source,source.opacity,&destination,destination.opacity,
            &composite);
          break;
        }
        case PlusCompositeOp:
        {
          CompositePlus(&source,source.opacity,&destination,destination.opacity,
            &composite);
          break;
        }
        case MultiplyCompositeOp:
        {
          CompositeMultiply(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case ScreenCompositeOp:
        {
          CompositeScreen(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case DarkenCompositeOp:
        {
          CompositeDarken(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case LightenCompositeOp:
        {
          CompositeLighten(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case ColorDodgeCompositeOp:
        {
          CompositeColorDodge(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case ColorBurnCompositeOp:
        {
          CompositeColorBurn(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case LinearDodgeCompositeOp:
        {
          CompositeLinearDodge(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case LinearBurnCompositeOp:
        {
          CompositeLinearBurn(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case HardLightCompositeOp:
        {
          CompositeHardLight(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case OverlayCompositeOp:
        {
          /*
            Reversed HardLight.
          */
          CompositeHardLight(&destination,destination.opacity,&source,
            source.opacity,&composite);
          break;
        }
        case SoftLightCompositeOp:
        {
          CompositeSoftLight(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case LinearLightCompositeOp:
        {
          CompositeLinearLight(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case PegtopLightCompositeOp:
        {
          CompositePegtopLight(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case VividLightCompositeOp:
        {
          CompositeVividLight(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case PinLightCompositeOp:
        {
          CompositePinLight(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case DifferenceCompositeOp:
        {
          CompositeDifference(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case ExclusionCompositeOp:
        {
          CompositeExclusion(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case MinusCompositeOp:
        {
          CompositeMinus(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case BumpmapCompositeOp:
        {
          if (source.opacity == TransparentOpacity)
            break;
          CompositeBumpmap(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case DissolveCompositeOp:
        {
          CompositeOver(&source,(MagickRealType) (QuantumRange-source_dissolve*
            (QuantumRange-source.opacity)),&destination,(MagickRealType)
            (QuantumRange-destination_dissolve*(QuantumRange-
            destination.opacity)),&composite);
          break;
        }
        case BlendCompositeOp:
        {
          MagickPixelCompositeBlend(&source,source_dissolve,&destination,
            destination_dissolve,&composite);
          break;
        }
        case MathematicsCompositeOp:
        {
          CompositeMathematics(&source,&destination,&geometry_info,&composite);
          break;
        }
        case BlurCompositeOp:
        case DisplaceCompositeOp:
        case DistortCompositeOp:
        {
          composite=source;
          break;
        }
        case ThresholdCompositeOp:
        {
          CompositeThreshold(&source,source.opacity,&destination,
            destination.opacity,threshold,amount,&composite);
          break;
        }
        case ModulateCompositeOp:
        {
          long
            offset;

          if (source.opacity == TransparentOpacity)
            break;
          offset=(long) (MagickPixelIntensityToQuantum(&source)-midpoint);
          if (offset == 0)
            break;
          CompositeHSB(destination.red,destination.green,destination.blue,&hue,
            &saturation,&brightness);
          brightness+=(0.01*percent_brightness*offset)/midpoint;
          saturation*=0.01*percent_saturation;
          HSBComposite(hue,saturation,brightness,&composite.red,
            &composite.green,&composite.blue);
          break;
        }
        case HueCompositeOp:
        {
          if (source.opacity == TransparentOpacity)
            break;
          if (destination.opacity == TransparentOpacity)
            {
              composite=source;
              break;
            }
          CompositeHSB(destination.red,destination.green,destination.blue,&hue,
            &saturation,&brightness);
          CompositeHSB(source.red,source.green,source.blue,&hue,&sans,&sans);
          HSBComposite(hue,saturation,brightness,&composite.red,
            &composite.green,&composite.blue);
          if (source.opacity < destination.opacity)
            composite.opacity=source.opacity;
          break;
        }
        case SaturateCompositeOp:
        {
          if (source.opacity == TransparentOpacity)
            break;
          if (destination.opacity == TransparentOpacity)
            {
              composite=source;
              break;
            }
          CompositeHSB(destination.red,destination.green,destination.blue,&hue,
            &saturation,&brightness);
          CompositeHSB(source.red,source.green,source.blue,&sans,&saturation,
            &sans);
          HSBComposite(hue,saturation,brightness,&composite.red,
            &composite.green,&composite.blue);
          if (source.opacity < destination.opacity)
            composite.opacity=source.opacity;
          break;
        }
        case SubtractCompositeOp:
        {
          CompositeSubtract(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case LuminizeCompositeOp:
        {
          if (source.opacity == TransparentOpacity)
            break;
          if (destination.opacity == TransparentOpacity)
            {
              composite=source;
              break;
            }
          CompositeHSB(destination.red,destination.green,destination.blue,&hue,
            &saturation,&brightness);
          CompositeHSB(source.red,source.green,source.blue,&sans,&sans,
            &brightness);
          HSBComposite(hue,saturation,brightness,&composite.red,
            &composite.green,&composite.blue);
          if (source.opacity < destination.opacity)
            composite.opacity=source.opacity;
          break;
        }
        case ColorizeCompositeOp:
        {
          if (source.opacity == TransparentOpacity)
            break;
          if (destination.opacity == TransparentOpacity)
            {
              composite=source;
              break;
            }
          CompositeHSB(destination.red,destination.green,destination.blue,&sans,
            &sans,&brightness);
          CompositeHSB(source.red,source.green,source.blue,&hue,&saturation,
            &sans);
          HSBComposite(hue,saturation,brightness,&composite.red,
            &composite.green,&composite.blue);
          if (source.opacity < destination.opacity)
            composite.opacity=source.opacity;
          break;
        }
        case CopyRedCompositeOp:
        case CopyCyanCompositeOp:
        {
          composite.red=source.red;
          break;
        }
        case CopyGreenCompositeOp:
        case CopyMagentaCompositeOp:
        {
          composite.green=source.green;
          break;
        }
        case CopyBlueCompositeOp:
        case CopyYellowCompositeOp:
        {
          composite.blue=source.blue;
          break;
        }
        case CopyOpacityCompositeOp:
        {
          if (source.matte == MagickFalse)
            {
              composite.opacity=(MagickRealType) (QuantumRange-
                MagickPixelIntensityToQuantum(&source));
              break;
            }
          composite.opacity=source.opacity;
          break;
        }
        case CopyBlackCompositeOp:
        {
          if (source.colorspace != CMYKColorspace)
            ConvertRGBToCMYK(&source);
          composite.index=source.index;
          break;
        }
        default:
          break;
      }
      if (image->colorspace == CMYKColorspace)
        {
          composite.red=(MagickRealType) QuantumRange-composite.red;
          composite.green=(MagickRealType) QuantumRange-composite.green;
          composite.blue=(MagickRealType) QuantumRange-composite.blue;
          composite.index=(MagickRealType) QuantumRange-composite.index;
        }
      q->red=ClampToQuantum(composite.red);
      q->green=ClampToQuantum(composite.green);
      q->blue=ClampToQuantum(composite.blue);
      q->opacity=ClampToQuantum(composite.opacity);
      if (image->colorspace == CMYKColorspace)
        indexes[x]=ClampToQuantum(composite.index);
      p++;
      if (p >= (pixels+composite_image->columns))
        p=pixels;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_CompositeImageChannel)
#endif
        proceed=SetImageProgress(image,CompositeImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  composite_view=DestroyCacheView(composite_view);
  image_view=DestroyCacheView(image_view);
  if (destination_image != (Image * ) NULL)
    destination_image=DestroyImage(destination_image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     T e x t u r e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TextureImage() repeatedly tiles the texture image across and down the image
%  canvas.
%
%  The format of the TextureImage method is:
%
%      MagickBooleanType TextureImage(Image *image,const Image *texture)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o texture: This image is the texture to layer on the background.
%
*/
MagickExport MagickBooleanType TextureImage(Image *image,const Image *texture)
{
#define TextureImageTag  "Texture/Image"

  CacheView
    *image_view,
    *texture_view;

  ExceptionInfo
    *exception;

  long
    y;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (texture == (const Image *) NULL)
    return(MagickFalse);
  (void) SetImageVirtualPixelMethod(texture,TileVirtualPixelMethod);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  if ((image->compose != CopyCompositeOp) &&
      ((image->compose != OverCompositeOp) || (image->matte != MagickFalse) ||
       (texture->matte != MagickFalse)))
    {
      /*
        Tile texture onto the image background.
      */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status)
#endif
      for (y=0; y < (long) image->rows; y+=texture->rows)
      {
        register long
          x;

        if (status == MagickFalse)
          continue;
        for (x=0; x < (long) image->columns; x+=texture->columns)
        {
          MagickBooleanType
            thread_status;

          thread_status=CompositeImage(image,image->compose,texture,x+
            texture->tile_offset.x,y+texture->tile_offset.y);
          if (thread_status == MagickFalse)
            {
              status=thread_status;
              break;
            }
        }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_TextureImage)
#endif
            proceed=SetImageProgress(image,TextureImageTag,y,image->rows);
            if (proceed == MagickFalse)
              status=MagickFalse;
          }
      }
      (void) SetImageProgress(image,TextureImageTag,(MagickOffsetType)
        image->rows,image->rows);
      return(status);
    }
  /*
    Tile texture onto the image background (optimized).
  */
  status=MagickTrue;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
  texture_view=AcquireCacheView(texture);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const IndexPacket
      *texture_indexes;

    register const PixelPacket
      *p;

    register IndexPacket
      *indexes;

    register long
      x;

    register PixelPacket
      *q;

    unsigned long
      width;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(texture_view,texture->tile_offset.x,(y+
      texture->tile_offset.y) % texture->rows,texture->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    texture_indexes=GetCacheViewVirtualIndexQueue(texture_view);
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x+=texture->columns)
    {
      width=texture->columns;
      if ((x+(long) width) > (long) image->columns)
        width=image->columns-x;
      (void) CopyMagickMemory(q,p,width*sizeof(*p));
      if ((image->colorspace == CMYKColorspace) &&
          (texture->colorspace == CMYKColorspace))
        {
          (void) CopyMagickMemory(indexes,texture_indexes,width*
            sizeof(*indexes));
          indexes+=width;
        }
      q+=width;
    }
    sync=SyncCacheViewAuthenticPixels(image_view,exception);
    if (sync == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp critical (MagickCore_TextureImage)
#endif
        proceed=SetImageProgress(image,TextureImageTag,y,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  texture_view=DestroyCacheView(texture_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

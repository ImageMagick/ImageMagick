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
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/resample.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/utility.h"
#include "MagickCore/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p o s i t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompositeImage() returns the second image composited onto the first
%  at the specified offset, using the specified composite method.
%
%  The format of the CompositeImage method is:
%
%      MagickBooleanType CompositeImage(Image *image,
%        const CompositeOperator compose,Image *composite_image,
%        const ssize_t x_offset,const ssize_t y_offset)
%
%  A description of each parameter follows:
%
%    o image: the destination image, modified by he composition
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
   Programmers notes on SVG specification.
  
   A Composition is defined by...
     Color Function :  f(Sc,Dc)  where Sc and Dc are the normizalized colors
      Blending areas :  X = 1    for area of overlap   ie: f(Sc,Dc)
                        Y = 1    for source preserved
                        Z = 1    for destination preserved
  
   Conversion to transparency (then optimized)
      Dca' = f(Sc, Dc)*Sa*Da + Y*Sca*(1-Da) + Z*Dca*(1-Sa)
      Da'  = X*Sa*Da + Y*Sa*(1-Da) + Z*Da*(1-Sa)
  
   Where...
     Sca = Sc*Sa     normalized Source color divided by Source alpha
     Dca = Dc*Da     normalized Dest color divided by Dest alpha
     Dc' = Dca'/Da'  the desired color value for this channel.
  
   Da' in in the follow formula as 'gamma'  The resulting alpla value.
  
   Most functions use a blending mode of over (X=1,Y=1,Z=1)
   this results in the following optimizations...
     gamma = Sa+Da-Sa*Da;
     gamma = 1 - QuantiumScale*alpha * QuantiumScale*beta;
     opacity = QuantiumScale*alpha*beta;  // over blend, optimized 1-Gamma
  
   The above SVG definitions also definate that Mathematical Composition
   methods should use a 'Over' blending mode for Alpha Channel.
   It however was not applied for composition modes of 'Plus', 'Minus',
   the modulus versions of 'Add' and 'Subtract'.
  
   Mathematical operator changes to be applied from IM v6.7...
  
    1/ Modulus modes 'Add' and 'Subtract' are obsoleted and renamed
       'ModulusAdd' and 'ModulusSubtract' for clarity.
  
    2/ All mathematical compositions work as per the SVG specification
       with regard to blending.  This now includes 'ModulusAdd' and
       'ModulusSubtract'.
  
    3/ When the special channel flag 'sync' (syncronize channel updates)
       is turned off (enabled by default) then mathematical compositions are
       only performed on the channels specified, and are applied
       independantally of each other.  In other words the mathematics is
       performed as 'pure' mathematical operations, rather than as image
       operations.
*/

static inline MagickRealType Atop(const MagickRealType p,
  const MagickRealType Sa,const MagickRealType q,
  const MagickRealType magick_unused(Da))
{
  return(p*Sa+q*(1.0-Sa));  /* Da optimized out,  Da/gamma => 1.0 */
}

static inline void CompositeAtop(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  composite->alpha=q->alpha;   /* optimized  Da = 1.0-Gamma */
  composite->red=Atop(p->red,Sa,q->red,1.0);
  composite->green=Atop(p->green,Sa,q->green,1.0);
  composite->blue=Atop(p->blue,Sa,q->blue,1.0);
  if (q->colorspace == CMYKColorspace)
    composite->black=Atop(p->black,Sa,q->black,1.0);
}

/*
  What is this Composition method for? Can't find any specification!
  WARNING this is not doing correct 'over' blend handling (Anthony Thyssen).
*/
static inline void CompositeBumpmap(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    intensity;

  intensity=(MagickRealType) GetPixelInfoIntensity(p);
  composite->red=QuantumScale*intensity*q->red;
  composite->green=QuantumScale*intensity*q->green;
  composite->blue=QuantumScale*intensity*q->blue;
  composite->alpha=(MagickRealType) QuantumScale*intensity*p->alpha;
  if (q->colorspace == CMYKColorspace)
    composite->black=QuantumScale*intensity*q->black;
}

static inline void CompositeClear(const PixelInfo *q,PixelInfo *composite)
{
  composite->alpha=(MagickRealType) TransparentAlpha;
  composite->red=0.0;
  composite->green=0.0;
  composite->blue=0.0;
  if (q->colorspace == CMYKColorspace)
    composite->black=0.0;
}

static MagickRealType ColorBurn(const MagickRealType Sca,
  const MagickRealType Sa, const MagickRealType Dca,const MagickRealType Da)
{
  if ((fabs(Sca) < MagickEpsilon) && (fabs(Dca-Da) < MagickEpsilon))
    return(Sa*Da+Dca*(1.0-Sa));
  if (Sca < MagickEpsilon)
    return(Dca*(1.0-Sa));
  return(Sa*Da-Sa*MagickMin(Da,(Da-Dca)*Sa/Sca)+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeColorBurn(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da);  /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*ColorBurn(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*ColorBurn(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*ColorBurn(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*ColorBurn(QuantumScale*p->black*Sa,Sa,QuantumScale*
      q->black*Da,Da);
}


static MagickRealType ColorDodge(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  /*
    Working from first principles using the original formula:

       f(Sc,Dc) = Dc/(1-Sc)

    This works correctly!  Looks like the 2004 SVG model was right but just
    required a extra condition for correct handling.
  */
  if ((fabs(Sca-Sa) < MagickEpsilon) && (fabs(Dca) < MagickEpsilon))
    return(Sca*(1.0-Da)+Dca*(1.0-Sa));
  if (fabs(Sca-Sa) < MagickEpsilon)
    return(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Dca*Sa*Sa/(Sa-Sca)+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeColorDodge(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da);  /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*ColorDodge(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*ColorDodge(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*ColorDodge(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*ColorDodge(QuantumScale*p->black*Sa,Sa,QuantumScale*
      q->black*Da,Da);
}

static inline MagickRealType Darken(const MagickRealType p,
  const MagickRealType alpha,const MagickRealType q,const MagickRealType beta)
{
  if (p < q)
    return(MagickOver_(p,alpha,q,beta));  /* src-over */
  return(MagickOver_(q,beta,p,alpha));    /* dst-over */
}

static inline void CompositeDarken(const Image *image,const PixelInfo *p,
  const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    gamma;

  /*
    Darken is equivalent to a 'Minimum' method OR a greyscale version of a
    binary 'Or' OR the 'Intersection' of pixel sets.
  */
  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels.
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=MagickMin(p->red,q->red);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=MagickMin(p->green,q->green);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=MagickMin(p->blue,q->blue);
      if ((GetPixelBlackTraits(image) & ActivePixelTrait) != 0 &&
          (q->colorspace == CMYKColorspace))
        composite->black=MagickMin(p->black,q->black);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=MagickMax(p->alpha,q->alpha);
      return;
    }
  composite->alpha=QuantumScale*p->alpha*q->alpha; /* Over Blend */
  gamma=1.0-QuantumScale*composite->alpha;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Darken(p->red,p->alpha,q->red,q->alpha);
  composite->green=gamma*Darken(p->green,p->alpha,q->green,q->alpha);
  composite->blue=gamma*Darken(p->blue,p->alpha,q->blue,q->alpha);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Darken(p->black,p->alpha,q->black,q->alpha);
}

static inline void CompositeDarkenIntensity(const Image *image,
  const PixelInfo *p,const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    Da,
    Sa;

  /*
    Select the pixel based on the intensity level.
    If 'Sync' flag select whole pixel based on alpha weighted intensity.
    Otherwise use intensity only, but restrict copy according to channel.
  */
  if (image->sync == MagickFalse)
    {
      MagickBooleanType
        from_p;

      from_p=GetPixelInfoIntensity(p) < GetPixelInfoIntensity(q) ? MagickTrue :
        MagickFalse;
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=from_p != MagickFalse ? p->red : q->red;
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=from_p != MagickFalse ? p->green : q->green;
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=from_p != MagickFalse ? p->blue : q->blue;
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=from_p != MagickFalse ? p->black : q->black;
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=from_p != MagickFalse ? p->alpha : q->alpha;
      return;
    }
  Sa=QuantumScale*p->alpha;
  Da=QuantumScale*q->alpha;
  *composite=(Sa*GetPixelInfoIntensity(p) < Da*GetPixelInfoIntensity(q)) ?
    *p : *q;
}

static inline MagickRealType Difference(const MagickRealType p,
  const MagickRealType Sa,const MagickRealType q,const MagickRealType Da)
{
  /*
    Optimized by Multipling by QuantumRange (taken from gamma).
  */
  return(Sa*p+Da*q-Sa*Da*2.0*MagickMin(p,q));
}

static inline void CompositeDifference(const Image *image,const PixelInfo *p,
  const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels.
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=fabs((double) (p->red-q->red));
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=fabs((double) (p->green-q->green));
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=fabs((double) (p->blue-q->blue));
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=fabs((double) (p->black-q->black));
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=fabs((double) (p->alpha-q->alpha));
     return;
   }
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Difference(p->red,Sa,q->red,Da);
  composite->green=gamma*Difference(p->green,Sa,q->green,Da);
  composite->blue=gamma*Difference(p->blue,Sa,q->blue,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Difference(p->black,Sa,q->black,Da);
}

static MagickRealType Divide(const MagickRealType Sca,const MagickRealType Sa,
  const MagickRealType Dca,const MagickRealType Da)
{
  /*
    Divide Source by Destination

      f(Sc,Dc) = Sc / Dc

    But with appropriate handling for special case of Dc == 0 specifically
    so that   f(Black,Black)=Black  and  f(non-Black,Black)=White.
    It is however also important to correctly do 'over' alpha blending which
    is why the formula becomes so complex.
  */
  if ((fabs(Sca) < MagickEpsilon) && (fabs(Dca) < MagickEpsilon))
    return(Sca*(1.0-Da)+Dca*(1.0-Sa));
  if (fabs(Dca) < MagickEpsilon)
    return(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Sca*Da*Da/Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeDivide(const Image *image,const PixelInfo *p,
  const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels.
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=QuantumRange*Divide(QuantumScale*p->red,1.0,
          QuantumScale*q->red,1.0);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=QuantumRange*Divide(QuantumScale*p->green,1.0,
          QuantumScale*q->green,1.0);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=QuantumRange*Divide(QuantumScale*p->blue,1.0,
          QuantumScale*q->blue,1.0);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=QuantumRange*Divide(QuantumScale*p->black,1.0,
          QuantumScale*q->black,1.0);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=QuantumRange*(1.0-Divide(Sa,1.0,Da,1.0));
      return;
    }
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Divide(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*Divide(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*Divide(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Divide(QuantumScale*p->black*Sa,Sa,QuantumScale*
      q->black*Da,Da);
}

static MagickRealType Exclusion(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  return(Sca*Da+Dca*Sa-2.0*Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeExclusion(const Image *image,const PixelInfo *p,
  const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    gamma,
    Sa,
    Da;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels.
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=QuantumRange*Exclusion(QuantumScale*p->red,1.0,
          QuantumScale*q->red,1.0);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=QuantumRange*Exclusion(QuantumScale*p->green,1.0,
          QuantumScale*q->green,1.0);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=QuantumRange*Exclusion(QuantumScale*p->blue,1.0,
          QuantumScale*q->blue,1.0);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=QuantumRange*Exclusion(QuantumScale*p->black,1.0,
          QuantumScale*q->black,1.0);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=QuantumRange*(1.0-Exclusion(Sa,1.0,Da,1.0));
      return;
    }
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Exclusion(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*Exclusion(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*Exclusion(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Exclusion(QuantumScale*p->black*Sa,Sa,
      QuantumScale*q->black*Da,Da);
}

static MagickRealType HardLight(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  if ((2.0*Sca) < Sa)
    return(2.0*Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Sa*Da-2.0*(Da-Dca)*(Sa-Sca)+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeHardLight(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da);  /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*HardLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*HardLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*HardLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*HardLight(QuantumScale*p->black*Sa,Sa,QuantumScale*
      q->black*Da,Da);
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

static inline MagickRealType In(const MagickRealType p,const MagickRealType Sa,
  const MagickRealType magick_unused(q),const MagickRealType Da)
{
  return(Sa*p*Da);
}

static inline void CompositeIn(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    gamma,
    Sa,
    Da;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=Sa*Da;
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*In(p->red,Sa,q->red,Da);
  composite->green=gamma*In(p->green,Sa,q->green,Da);
  composite->blue=gamma*In(p->blue,Sa,q->blue,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*In(p->black,Sa,q->black,Da);
}

static inline MagickRealType Lighten(const MagickRealType p,
  const MagickRealType alpha,const MagickRealType q,const MagickRealType beta)
{
   if (p > q)
     return(MagickOver_(p,alpha,q,beta));  /* src-over */
   return(MagickOver_(q,beta,p,alpha));    /* dst-over */
}

static inline void CompositeLighten(const Image *image,const PixelInfo *p,
  const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    gamma;

  /*
    Lighten is also equvalent to a 'Maximum' method OR a greyscale version of a
    binary 'And' OR the 'Union' of pixel sets.
  */
  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=MagickMax(p->red,q->red);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=MagickMax(p->green,q->green);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=MagickMax(p->blue,q->blue);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=MagickMax(p->black,q->black);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=MagickMin(p->alpha,q->alpha);
      return;
    }
  composite->alpha=QuantumScale*p->alpha*q->alpha; /* Over Blend */
  gamma=1.0-QuantumScale*composite->alpha;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Lighten(p->red,p->alpha,q->red,q->alpha);
  composite->green=gamma*Lighten(p->green,p->alpha,q->green,q->alpha);
  composite->blue=gamma*Lighten(p->blue,p->alpha,q->blue,q->alpha);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Lighten(p->black,p->alpha,q->black,q->alpha);
}

static inline void CompositeLightenIntensity(const Image *image,
  const PixelInfo *p,const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    Da,
    Sa;

  /*
    Select the pixel based on the intensity level.
    If 'Sync' flag select whole pixel based on alpha weighted intensity.
    Otherwise use Intenisty only, but restrict copy according to channel.
  */
  if (image->sync == MagickFalse)
    {
      MagickBooleanType
        from_p;

      from_p=GetPixelInfoIntensity(p) > GetPixelInfoIntensity(q) ? MagickTrue :
        MagickFalse;
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=from_p != MagickFalse ? p->red : q->red;
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=from_p != MagickFalse ? p->green : q->green;
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=from_p != MagickFalse ? p->blue : q->blue;
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=from_p != MagickFalse ? p->black : q->black;
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=from_p != MagickFalse ? p->alpha : q->alpha;
      return;
    }
  Sa=QuantumScale*p->alpha;
  Da=QuantumScale*q->alpha;
  *composite=(Sa*GetPixelInfoIntensity(p) > Da*GetPixelInfoIntensity(q)) ?
    *p : *q;
}

static inline void CompositeLinearDodge(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*(p->red*Sa+q->red*Da);
  composite->green=gamma*(p->green*Sa+q->green*Da);
  composite->blue=gamma*(p->blue*Sa+q->blue*Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*(p->black*Sa+q->black*Da);
}


static inline MagickRealType LinearBurn(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  /*
    LinearBurn: as defined by Abode Photoshop, according to
    http://www.simplefilter.de/en/basics/mixmods.html is:

      f(Sc,Dc) = Sc + Dc - 1
  */
  return(Sca+Dca-Sa*Da);
}

static inline void CompositeLinearBurn(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*LinearBurn(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*LinearBurn(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*LinearBurn(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*LinearBurn(QuantumScale*p->black*Sa,Sa,QuantumScale*
      q->black*Da,Da);
}

static inline MagickRealType LinearLight(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  /*
    LinearLight: as defined by Abode Photoshop, according to
    http://www.simplefilter.de/en/basics/mixmods.html is:

      f(Sc,Dc) = Dc + 2*Sc - 1
  */
  return((Sca-Sa)*Da+Sca+Dca);
}

static inline void CompositeLinearLight(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*LinearLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*LinearLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*LinearLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*LinearLight(QuantumScale*p->black*Sa,Sa,QuantumScale*
      q->black*Da,Da);
}

static inline MagickRealType Mathematics(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da,
  const GeometryInfo *geometry_info)
{
  MagickRealType
    gamma;

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
  gamma=geometry_info->rho*Sca*Dca+geometry_info->sigma*Sca*Da+
    geometry_info->xi*Dca*Sa+geometry_info->psi*Sa*Da+Sca*(1.0-Da)+
    Dca*(1.0-Sa);
  return(gamma);
}

static inline void CompositeMathematics(const Image *image,const PixelInfo *p,
  const PixelInfo *q,const GeometryInfo *args,PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha; /* ??? - AT */
  Da=QuantumScale*q->alpha;
  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels.
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=QuantumRange*Mathematics(QuantumScale*p->red,1.0,
          QuantumScale*q->red,1.0,args);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=QuantumRange*Mathematics(QuantumScale*p->green,1.0,
          QuantumScale*q->green,1.0,args);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=QuantumRange*Mathematics(QuantumScale*p->blue,1.0,
          QuantumScale*q->blue,1.0,args);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=QuantumRange*Mathematics(QuantumScale*p->black,1.0,
          QuantumScale*q->black,1.0,args);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=QuantumRange*(1.0-Mathematics(Sa,1.0,Da,1.0,args));
      return;
    }
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Mathematics(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da,args);
  composite->green=gamma*Mathematics(QuantumScale*p->green*Sa,Sa,
    QuantumScale*q->green*Da,Da,args);
  composite->blue=gamma*Mathematics(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da,args);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Mathematics(QuantumScale*p->black*Sa,Sa,
      QuantumScale*q->black*Da,Da,args);
}

static inline void CompositePlus(const Image *image,const PixelInfo *p,
  const PixelInfo *q,PixelInfo *composite)
{
  /*
    NOTE: "Plus" does not use 'over' alpha-blending but uses a special
    'plus' form of alph-blending. It is the ONLY mathematical operator to
    do this. this is what makes it different to the otherwise equivalent
    "LinearDodge" composition method.

    Note however that color channels are still effected by the alpha channel
    as a result of the blending, making it just as useless for independant
    channel maths, just like all other mathematical composition methods.

    As such the removal of the 'sync' flag, is still a usful convention.

    The CompositePixelInfoPlus() function is defined in
    "composite-private.h" so it can also be used for Image Blending.
  */
  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels.
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=p->red+q->red;
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=p->green+q->green;
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=p->blue+q->blue;
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=p->black+q->black;
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=p->alpha+q->alpha-QuantumRange;
      return;
    }
  CompositePixelInfoPlus(p,p->alpha,q,q->alpha,composite);
}

static inline MagickRealType Minus(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,
  const MagickRealType magick_unused(Da))
{
  /*
    Minus Source from Destination

      f(Sc,Dc) = Sc - Dc
  */
  return(Sca+Dca-2.0*Dca*Sa);
}

static inline void CompositeMinus(const Image *image,const PixelInfo *p,
  const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels.
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=p->red-q->red;
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=p->green-q->green;
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=p->blue-q->blue;
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=p->black-q->black;
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=QuantumRange*(1.0-(Sa-Da));
      return;
    }
  gamma=RoundToUnity(Sa+Da-Sa*Da);  /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Minus(p->red*Sa,Sa,q->red*Da,Da);
  composite->green=gamma*Minus(p->green*Sa,Sa,q->green*Da,Da);
  composite->blue=gamma*Minus(p->blue*Sa,Sa,q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Minus(p->black*Sa,Sa,q->black*Da,Da);
}

static inline MagickRealType ModulusAdd(const MagickRealType p,
  const MagickRealType Sa,const MagickRealType q, const MagickRealType Da)
{
  MagickRealType
    pixel;

  pixel=p+q;
  if (pixel > QuantumRange)
    pixel-=(QuantumRange+1.0);
  return(pixel*Sa*Da+p*Sa*(1.0-Da)+q*Da*(1.0-Sa));
}

static inline void CompositeModulusAdd(const Image *image,const PixelInfo *p,
  const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels.
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=ModulusAdd(p->red,1.0,q->red,1.0);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=ModulusAdd(p->green,1.0,q->green,1.0);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=ModulusAdd(p->blue,1.0,q->blue,1.0);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=ModulusAdd(p->black,1.0,q->black,1.0);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=ModulusAdd(p->alpha,1.0,q->alpha,1.0);
      return;
    }
  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=ModulusAdd(p->red,Sa,q->red,Da);
  composite->green=ModulusAdd(p->green,Sa,q->green,Da);
  composite->blue=ModulusAdd(p->blue,Sa,q->blue,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=ModulusAdd(p->black,Sa,q->black,Da);
}

static inline MagickRealType ModulusSubtract(const MagickRealType p,
  const MagickRealType Sa,const MagickRealType q, const MagickRealType Da)
{
  MagickRealType
    pixel;

  pixel=p-q;
  if (pixel < 0.0)
    pixel+=(QuantumRange+1.0);
  return(pixel*Sa*Da+p*Sa*(1.0-Da)+q*Da*(1.0-Sa));
}

static inline void CompositeModulusSubtract(const Image *image,
  const PixelInfo *p,const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels,
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=ModulusSubtract(p->red,1.0,q->red,1.0);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=ModulusSubtract(p->green,1.0,q->green,1.0);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=ModulusSubtract(p->blue,1.0,q->blue,1.0);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=ModulusSubtract(p->black,1.0,q->black,1.0);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=ModulusSubtract(p->alpha,1.0,q->alpha,1.0);
      return;
    }
  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma = RoundToUnity(Sa+Da-Sa*Da);
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=ModulusSubtract(p->red,Sa,q->red,Da);
  composite->green=ModulusSubtract(p->green,Sa,q->green,Da);
  composite->blue=ModulusSubtract(p->blue,Sa,q->blue,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=ModulusSubtract(p->black,Sa,q->black,Da);
}

static  inline MagickRealType Multiply(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  return(Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeMultiply(const Image *image,const PixelInfo *p,
  const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels.
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=QuantumScale*p->red*q->red;
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=QuantumScale*p->green*q->green;
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=QuantumScale*p->blue*q->blue;
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=QuantumScale*p->black*q->black;
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=QuantumRange*(1.0-Sa*Da);
      return;
    }
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Multiply(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*Multiply(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*Multiply(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Multiply(QuantumScale*p->black*Sa,Sa,
      QuantumScale*q->black*Da,Da);
}

static inline MagickRealType Out(const MagickRealType p,const MagickRealType Sa,
  const MagickRealType magick_unused(q),const MagickRealType Da)
{
  return(Sa*p*(1.0-Da));
}

static inline void CompositeOut(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Sa,
    Da,
    gamma;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=Sa*(1.0-Da);
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Out(p->red,Sa,q->red,Da);
  composite->green=gamma*Out(p->green,Sa,q->green,Da);
  composite->blue=gamma*Out(p->blue,Sa,q->blue,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Out(p->black,Sa,q->black,Da);
}

static MagickRealType PegtopLight(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  /*
    PegTop: A Soft-Light alternative: A continuous version of the Softlight
    function, producing very similar results.

    f(Sc,Dc) = Dc^2*(1-2*Sc) + 2*Sc*Dc

    See http://www.pegtop.net/delphi/articles/blendmodes/softlight.htm.
  */
  if (fabs(Da) < MagickEpsilon)
    return(Sca);
  return(Dca*Dca*(Sa-2.0*Sca)/Da+Sca*(2.0*Dca+1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositePegtopLight(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*PegtopLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*PegtopLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*PegtopLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*PegtopLight(QuantumScale*p->black*Sa,Sa,QuantumScale*
      q->black*Da,Da);
}

static MagickRealType PinLight(const MagickRealType Sca,const MagickRealType Sa,
  const MagickRealType Dca,const MagickRealType Da)
{
  /*
    PinLight: A Photoshop 7 composition method
    http://www.simplefilter.de/en/basics/mixmods.html

    f(Sc,Dc) = Dc<2*Sc-1 ? 2*Sc-1 : Dc>2*Sc   ? 2*Sc : Dc
  */
  if (Dca*Sa < Da*(2.0*Sca-Sa))
    return(Sca*(Da+1.0)-Sa*Da+Dca*(1.0-Sa));
  if ((Dca*Sa) > (2.0*Sca*Da))
    return(Sca*Da+Sca+Dca*(1.0-Sa));
  return(Sca*(1.0-Da)+Dca);
}

static inline void CompositePinLight(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*PinLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*PinLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*PinLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*PinLight(QuantumScale*p->black*Sa,Sa,QuantumScale*
      q->black*Da,Da);
}

static inline MagickRealType Screen(const MagickRealType Sca,
  const MagickRealType Dca)
{
  /*
    Screen:  A negated multiply
      f(Sc,Dc) = 1.0-(1.0-Sc)*(1.0-Dc)
  */
  return(Sca+Dca-Sca*Dca);
}

static inline void CompositeScreen(const Image *image,const PixelInfo *p,
  const PixelInfo *q,PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  if (image->sync == MagickFalse)
    {
      /*
        Handle channels as separate grayscale channels.
      */
      if ((GetPixelRedTraits(image) & ActivePixelTrait) != 0)
        composite->red=QuantumRange*Screen(QuantumScale*p->red,
          QuantumScale*q->red);
      if ((GetPixelGreenTraits(image) & ActivePixelTrait) != 0)
        composite->green=QuantumRange*Screen(QuantumScale*p->green,
          QuantumScale*q->green);
      if ((GetPixelBlueTraits(image) & ActivePixelTrait) != 0)
        composite->blue=QuantumRange*Screen(QuantumScale*p->blue,
          QuantumScale*q->blue);
      if (((GetPixelBlackTraits(image) & ActivePixelTrait) != 0) &&
          (q->colorspace == CMYKColorspace))
        composite->black=QuantumRange*Screen(QuantumScale*p->black,
          QuantumScale*q->black);
      if ((GetPixelAlphaTraits(image) & ActivePixelTrait) != 0)
        composite->alpha=QuantumRange*(1.0-Screen(Sa,Da));
      return;
    }
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  Sa*=QuantumScale; Da*=QuantumScale; /* optimization */
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Screen(p->red*Sa,q->red*Da);
  composite->green=gamma*Screen(p->green*Sa,q->green*Da);
  composite->blue=gamma*Screen(p->blue*Sa,q->blue*Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Screen(p->black*Sa,q->black*Da);
}

static MagickRealType SoftLight(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
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
}

static inline void CompositeSoftLight(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*SoftLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*SoftLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*SoftLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*SoftLight(QuantumScale*p->black*Sa,Sa,QuantumScale*
      q->black*Da,Da);
}

static inline MagickRealType Threshold(const MagickRealType p,
  const MagickRealType q,const MagickRealType threshold,
  const MagickRealType amount)
{
  MagickRealType
    delta;

  /*
    Multiply difference by amount, if differance larger than threshold???
    What use this is is completely unknown.  The Opacity calculation appears to
    be inverted  -- Anthony Thyssen

    Deprecated.
  */
  delta=p-q;
  if ((MagickRealType) fabs((double) (2.0*delta)) < threshold)
    return(q);
  return(q+delta*amount);
}

static inline void CompositeThreshold(const PixelInfo *p,const PixelInfo *q,
  const MagickRealType threshold,const MagickRealType amount,
  PixelInfo *composite)
{
  composite->red=Threshold(p->red,q->red,threshold,amount);
  composite->green=Threshold(p->green,q->green,threshold,amount);
  composite->blue=Threshold(p->blue,q->blue,threshold,amount);
  composite->alpha=Threshold(p->alpha,q->alpha,threshold,amount);
  if (q->colorspace == CMYKColorspace)
    composite->black=Threshold(p->black,q->black,threshold,amount);
}


static MagickRealType VividLight(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  /*
    VividLight: A Photoshop 7 composition method.  See
    http://www.simplefilter.de/en/basics/mixmods.html.

    f(Sc,Dc) = (2*Sc < 1) ? 1-(1-Dc)/(2*Sc) : Dc/(2*(1-Sc))
  */
  if ((fabs(Sa) < MagickEpsilon) || (fabs(Sca-Sa) < MagickEpsilon))
    return(Sa*Da+Sca*(1.0-Da)+Dca*(1.0-Sa));
  if ((2.0*Sca) <= Sa)
    return(Sa*(Da+Sa*(Dca-Da)/(2.0*Sca))+Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Dca*Sa*Sa/(2.0*(Sa-Sca))+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeVividLight(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*VividLight(QuantumScale*p->red*Sa,Sa,QuantumScale*
    q->red*Da,Da);
  composite->green=gamma*VividLight(QuantumScale*p->green*Sa,Sa,QuantumScale*
    q->green*Da,Da);
  composite->blue=gamma*VividLight(QuantumScale*p->blue*Sa,Sa,QuantumScale*
    q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*VividLight(QuantumScale*p->black*Sa,Sa,QuantumScale*
      q->black*Da,Da);
}

static MagickRealType Xor(const MagickRealType Sca,const MagickRealType Sa,
  const MagickRealType Dca,const MagickRealType Da)
{
  return(Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeXor(const PixelInfo *p,const PixelInfo *q,
  PixelInfo *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=QuantumScale*p->alpha;  /* simplify and speed up equations */
  Da=QuantumScale*q->alpha;
  gamma=Sa+Da-2.0*Sa*Da;        /* Xor blend mode X=0,Y=1,Z=1 */
  composite->alpha=(MagickRealType) QuantumRange*gamma;
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Xor(p->red*Sa,Sa,q->red*Da,Da);
  composite->green=gamma*Xor(p->green*Sa,Sa,q->green*Da,Da);
  composite->blue=gamma*Xor(p->blue*Sa,Sa,q->blue*Da,Da);
  if (q->colorspace == CMYKColorspace)
    composite->black=gamma*Xor(p->black*Sa,Sa,q->black*Da,Da);
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
  const ssize_t x_offset,const ssize_t y_offset)
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

  MagickBooleanType
    modify_outside_overlay,
    status;

  MagickOffsetType
    progress;

  PixelInfo
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

  ssize_t
    y;

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
  GetPixelInfo(image,&zero);
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
    case CopyCompositeOp:
    {
      if ((x_offset < 0) || (y_offset < 0))
        break;
      if ((x_offset+(ssize_t) composite_image->columns) >= (ssize_t) image->columns)
        break;
      if ((y_offset+(ssize_t) composite_image->rows) >= (ssize_t) image->rows)
        break;
      status=MagickTrue;
      exception=(&image->exception);
      image_view=AcquireCacheView(image);
      composite_view=AcquireCacheView(composite_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
      for (y=0; y < (ssize_t) composite_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const Quantum
          *p;

        register Quantum
          *q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        q=GetCacheViewAuthenticPixels(image_view,x_offset,y+y_offset,
          composite_image->columns,1,exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) composite_image->columns; x++)
        {
          SetPixelRed(image,GetPixelRed(composite_image,p),q);
          SetPixelGreen(image,GetPixelGreen(composite_image,p),q);
          SetPixelBlue(image,GetPixelBlue(composite_image,p),q);
          SetPixelAlpha(image,GetPixelAlpha(composite_image,p),q);
          if (image->colorspace == CMYKColorspace)
            SetPixelBlack(image,GetPixelBlack(composite_image,p),q);
          p+=GetPixelComponents(composite_image);
          q+=GetPixelComponents(image);
        }
        sync=SyncCacheViewAuthenticPixels(image_view,exception);
        if (sync == MagickFalse)
          status=MagickFalse;
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            MagickBooleanType
              proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
#pragma omp critical (MagickCore_CompositeImage)
#endif
            proceed=SetImageProgress(image,CompositeImageTag,
              (MagickOffsetType) y,image->rows);
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

      PixelInfo
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
        Blur Image by resampling.
      */
      pixel=zero;
      exception=(&image->exception);
      resample_filter=AcquireResampleFilter(image,&image->exception);
      SetResampleFilter(resample_filter,CubicFilter,2.0);
      destination_view=AcquireCacheView(destination_image);
      composite_view=AcquireCacheView(composite_image);
      for (y=0; y < (ssize_t) composite_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const Quantum
          *restrict p;

        register Quantum
          *restrict q;

        register ssize_t
          x;

        if (((y+y_offset) < 0) || ((y+y_offset) >= (ssize_t) image->rows))
          continue;
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        q=QueueCacheViewAuthenticPixels(destination_view,0,y,
          destination_image->columns,1,&image->exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          break;
        for (x=0; x < (ssize_t) composite_image->columns; x++)
        {
          if (((x_offset+x) < 0) || ((x_offset+x) >= (ssize_t) image->columns))
            {
              p+=GetPixelComponents(composite_image);
              continue;
            }
          if (fabs(angle_range) > MagickEpsilon)
            {
              MagickRealType
                angle;

              angle=angle_start+angle_range*QuantumScale*
                GetPixelBlue(composite_image,p);
              blur.x1=width*cos(angle);
              blur.x2=width*sin(angle);
              blur.y1=(-height*sin(angle));
              blur.y2=height*cos(angle);
            }
          ScaleResampleFilter(resample_filter,blur.x1*QuantumScale*
            GetPixelRed(composite_image,p),blur.y1*QuantumScale*
            GetPixelGreen(composite_image,p),blur.x2*QuantumScale*
            GetPixelRed(composite_image,p),blur.y2*QuantumScale*
            GetPixelGreen(composite_image,p));
          (void) ResamplePixelColor(resample_filter,(double) x_offset+x,
            (double) y_offset+y,&pixel);
          SetPixelPixelInfo(destination_image,&pixel,q);
          p+=GetPixelComponents(composite_image);
          q+=GetPixelComponents(destination_image);
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
        *destination_view,
        *image_view;

      PixelInfo
        pixel;

      MagickRealType
        horizontal_scale,
        vertical_scale;

      PointInfo
        center,
        offset;

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
           arg flag '!' = locations/percentage relative to background image
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
      image_view=AcquireCacheView(image);
      destination_view=AcquireCacheView(destination_image);
      composite_view=AcquireCacheView(composite_image);
      for (y=0; y < (ssize_t) composite_image->rows; y++)
      {
        MagickBooleanType
          sync;

        register const Quantum
          *restrict p;

        register Quantum
          *restrict q;

        register ssize_t
          x;

        if (((y+y_offset) < 0) || ((y+y_offset) >= (ssize_t) image->rows))
          continue;
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        q=QueueCacheViewAuthenticPixels(destination_view,0,y,
          destination_image->columns,1,&image->exception);
        if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
          break;
        for (x=0; x < (ssize_t) composite_image->columns; x++)
        {
          if (((x_offset+x) < 0) || ((x_offset+x) >= (ssize_t) image->columns))
            {
              p+=GetPixelComponents(composite_image);
              continue;
            }
          /*
            Displace the offset.
          */
          offset.x=(horizontal_scale*(GetPixelRed(composite_image,p)-
            (((MagickRealType) QuantumRange+1.0)/2.0)))/(((MagickRealType)
            QuantumRange+1.0)/2.0)+center.x+((compose == DisplaceCompositeOp) ?
            x : 0);
          offset.y=(vertical_scale*(GetPixelGreen(composite_image,p)-
            (((MagickRealType) QuantumRange+1.0)/2.0)))/(((MagickRealType)
            QuantumRange+1.0)/2.0)+center.y+((compose == DisplaceCompositeOp) ?
            y : 0);
          (void) InterpolatePixelInfo(image,image_view,
            UndefinedInterpolatePixel,(double) offset.x,(double) offset.y,
            &pixel,exception);
          /*
            Mask with the 'invalid pixel mask' in alpha channel.
          */
          pixel.alpha=(MagickRealType) QuantumRange*(1.0-(1.0-QuantumScale*
            pixel.alpha)*(1.0-QuantumScale*
            GetPixelAlpha(composite_image,p)));
          SetPixelPixelInfo(destination_image,&pixel,q);
          p+=GetPixelComponents(composite_image);
          q+=GetPixelComponents(destination_image);
        }
        sync=SyncCacheViewAuthenticPixels(destination_view,exception);
        if (sync == MagickFalse)
          break;
      }
      destination_view=DestroyCacheView(destination_view);
      composite_view=DestroyCacheView(composite_view);
      image_view=DestroyCacheView(image_view);
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
  GetPixelInfo(composite_image,&zero);
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
  composite_view=AcquireCacheView(composite_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(progress,status)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *pixels;

    double
      brightness,
      hue,
      saturation;

    PixelInfo
      composite,
      destination,
      source;

    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    if (modify_outside_overlay == MagickFalse)
      {
        if (y < y_offset)
          continue;
        if ((y-y_offset) >= (ssize_t) composite_image->rows)
          continue;
      }
    /*
      If pixels is NULL, y is outside overlay region.
    */
    pixels=(Quantum *) NULL;
    p=(Quantum *) NULL;
    if ((y >= y_offset) && ((y-y_offset) < (ssize_t) composite_image->rows))
      {
        p=GetCacheViewVirtualPixels(composite_view,0,y-y_offset,
          composite_image->columns,1,exception);
        if (p == (const Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        pixels=p;
        if (x_offset < 0)
          p-=x_offset*GetPixelComponents(composite_image);
      }
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    source=zero;
    destination=zero;
    hue=0.0;
    saturation=0.0;
    brightness=0.0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (modify_outside_overlay == MagickFalse)
        {
          if (x < x_offset)
            {
              q+=GetPixelComponents(image);
              continue;
            }
          if ((x-x_offset) >= (ssize_t) composite_image->columns)
            break;
        }
      destination.red=(MagickRealType) GetPixelRed(image,q);
      destination.green=(MagickRealType) GetPixelGreen(image,q);
      destination.blue=(MagickRealType) GetPixelBlue(image,q);
      if (image->colorspace == CMYKColorspace)
        destination.black=(MagickRealType) GetPixelBlack(image,q);
      if (image->colorspace == CMYKColorspace)
        {
          destination.red=(MagickRealType) QuantumRange-destination.red;
          destination.green=(MagickRealType) QuantumRange-destination.green;
          destination.blue=(MagickRealType) QuantumRange-destination.blue;
          destination.black=(MagickRealType) QuantumRange-destination.black;
        }
      if (image->matte != MagickFalse)
        destination.alpha=(MagickRealType) GetPixelAlpha(image,q);
      /*
        Handle destination modifications outside overlaid region.
      */
      composite=destination;
      if ((pixels == (Quantum *) NULL) || (x < x_offset) ||
          ((x-x_offset) >= (ssize_t) composite_image->columns))
        {
          switch (compose)
          {
            case DissolveCompositeOp:
            case BlendCompositeOp:
            {
              composite.alpha=destination_dissolve*(composite.alpha);
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
              composite.alpha=(MagickRealType) TransparentAlpha;
              break;
            }
            default:
            {
              (void) GetOneVirtualMagickPixel(composite_image,x-x_offset,y-
                y_offset,&composite,exception);
              break;
            }
          }
          if (image->colorspace == CMYKColorspace)
            {
              composite.red=(MagickRealType) QuantumRange-composite.red;
              composite.green=(MagickRealType) QuantumRange-composite.green;
              composite.blue=(MagickRealType) QuantumRange-composite.blue;
              composite.black=(MagickRealType) QuantumRange-composite.black;
            }
          SetPixelRed(image,ClampToQuantum(composite.red),q);
          SetPixelGreen(image,ClampToQuantum(composite.green),q);
          SetPixelBlue(image,ClampToQuantum(composite.blue),q);
          if (image->matte != MagickFalse)
            SetPixelAlpha(image,ClampToQuantum(composite.alpha),q);
          if (image->colorspace == CMYKColorspace)
            SetPixelBlack(image,ClampToQuantum(composite.black),q);
          q+=GetPixelComponents(image);
          continue;
        }
      /*
        Handle normal overlay of source onto destination.
      */
      source.red=(MagickRealType) GetPixelRed(composite_image,p);
      source.green=(MagickRealType) GetPixelGreen(composite_image,p);
      source.blue=(MagickRealType) GetPixelBlue(composite_image,p);
      if (composite_image->colorspace == CMYKColorspace)
        source.black=(MagickRealType) GetPixelBlack(composite_image,p);
      if (composite_image->colorspace == CMYKColorspace)
        {
          source.red=(MagickRealType) QuantumRange-source.red;
          source.green=(MagickRealType) QuantumRange-source.green;
          source.blue=(MagickRealType) QuantumRange-source.blue;
          source.black=(MagickRealType) QuantumRange-source.black;
        }
      if (composite_image->matte != MagickFalse)
        source.alpha=(MagickRealType) GetPixelAlpha(composite_image,p);
      /*
        Porter-Duff compositions.
      */
      switch (compose)
      {
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
        case NoCompositeOp:
        case DstCompositeOp:
          break;
        case OverCompositeOp:
        case SrcOverCompositeOp:
        {
          CompositePixelInfoOver(&source,source.alpha,&destination,
            destination.alpha,&composite);
          break;
        }
        case DstOverCompositeOp:
        {
          CompositePixelInfoOver(&destination,destination.alpha,&source,
            source.alpha,&composite);
          break;
        }
        case SrcInCompositeOp:
        case InCompositeOp:
        {
          CompositeIn(&source,&destination,&composite);
          break;
        }
        case DstInCompositeOp:
        {
          CompositeIn(&destination,&source,&composite);
          break;
        }
        case OutCompositeOp:
        case SrcOutCompositeOp:
        {
          CompositeOut(&source,&destination,&composite);
          break;
        }
        case DstOutCompositeOp:
        {
          CompositeOut(&destination,&source,&composite);
          break;
        }
        case AtopCompositeOp:
        case SrcAtopCompositeOp:
        {
          CompositeAtop(&source,&destination,&composite);
          break;
        }
        case DstAtopCompositeOp:
        {
          CompositeAtop(&destination,&source,&composite);
          break;
        }
        case XorCompositeOp:
        {
          CompositeXor(&source,&destination,&composite);
          break;
        }
        case PlusCompositeOp:
        {
          CompositePlus(image,&source,&destination,&composite);
          break;
        }
        case MinusDstCompositeOp:
        {
          CompositeMinus(image,&source,&destination,&composite);
          break;
        }
        case MinusSrcCompositeOp:
        {
          CompositeMinus(image,&destination,&source,&composite);
          break;
        }
        case ModulusAddCompositeOp:
        {
          CompositeModulusAdd(image,&source,&destination,&composite);
          break;
        }
        case ModulusSubtractCompositeOp:
        {
          CompositeModulusSubtract(image,&source,&destination,&composite);
          break;
        }
        case DifferenceCompositeOp:
        {
          CompositeDifference(image,&source,&destination,&composite);
          break;
        }
        case ExclusionCompositeOp:
        {
          CompositeExclusion(image,&source,&destination,&composite);
          break;
        }
        case MultiplyCompositeOp:
        {
          CompositeMultiply(image,&source,&destination,&composite);
          break;
        }
        case ScreenCompositeOp:
        {
          CompositeScreen(image,&source,&destination,&composite);
          break;
        }
        case DivideDstCompositeOp:
        {
          CompositeDivide(image,&source,&destination,&composite);
          break;
        }
        case DivideSrcCompositeOp:
        {
          CompositeDivide(image,&destination,&source,&composite);
          break;
        }
        case DarkenCompositeOp:
        {
          CompositeDarken(image,&source,&destination,&composite);
          break;
        }
        case LightenCompositeOp:
        {
          CompositeLighten(image,&source,&destination,&composite);
          break;
        }
        case DarkenIntensityCompositeOp:
        {
          CompositeDarkenIntensity(image,&source,&destination,&composite);
          break;
        }
        case LightenIntensityCompositeOp:
        {
          CompositeLightenIntensity(image,&source,&destination,&composite);
          break;
        }
        case MathematicsCompositeOp:
        {
          CompositeMathematics(image,&source,&destination,&geometry_info,
            &composite);
          break;
        }
        case ColorDodgeCompositeOp:
        {
          CompositeColorDodge(&source,&destination,&composite);
          break;
        }
        case ColorBurnCompositeOp:
        {
          CompositeColorBurn(&source,&destination,&composite);
          break;
        }
        case LinearDodgeCompositeOp:
        {
          CompositeLinearDodge(&source,&destination,&composite);
          break;
        }
        case LinearBurnCompositeOp:
        {
          CompositeLinearBurn(&source,&destination,&composite);
          break;
        }
        case HardLightCompositeOp:
        {
          CompositeHardLight(&source,&destination,&composite);
          break;
        }
        case OverlayCompositeOp:
        {
          CompositeHardLight(&destination,&source,&composite);
          break;
        }
        case SoftLightCompositeOp:
        {
          CompositeSoftLight(&source,&destination,&composite);
          break;
        }
        case LinearLightCompositeOp:
        {
          CompositeLinearLight(&source,&destination,&composite);
          break;
        }
        case PegtopLightCompositeOp:
        {
          CompositePegtopLight(&source,&destination,&composite);
          break;
        }
        case VividLightCompositeOp:
        {
          CompositeVividLight(&source,&destination,&composite);
          break;
        }
        case PinLightCompositeOp:
        {
          CompositePinLight(&source,&destination,&composite);
          break;
        }
        case ChangeMaskCompositeOp:
        {
          if ((composite.alpha > ((MagickRealType) QuantumRange/2.0)) ||
              (IsFuzzyEquivalencePixelInfo(&source,&destination) != MagickFalse))
            composite.alpha=(MagickRealType) TransparentAlpha;
          else
            composite.alpha=(MagickRealType) OpaqueAlpha;
          break;
        }
        case BumpmapCompositeOp:
        {
          if (source.alpha == TransparentAlpha)
            break;
          CompositeBumpmap(&source,&destination,&composite);
          break;
        }
        case DissolveCompositeOp:
        {
          CompositePixelInfoOver(&source,source_dissolve*source.alpha,
            &destination,(MagickRealType) (destination_dissolve*
            destination.alpha),&composite);
          break;
        }
        case BlendCompositeOp:
        {
          CompositePixelInfoBlend(&source,source_dissolve,&destination,
            destination_dissolve,&composite);
          break;
        }
        case ThresholdCompositeOp:
        {
          CompositeThreshold(&source,&destination,threshold,amount,&composite);
          break;
        }
        case ModulateCompositeOp:
        {
          ssize_t
            offset;

          if (source.alpha == TransparentAlpha)
            break;
          offset=(ssize_t) (GetPixelInfoIntensity(&source)-midpoint);
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
          if (source.alpha == TransparentAlpha)
            break;
          if (destination.alpha == TransparentAlpha)
            {
              composite=source;
              break;
            }
          CompositeHSB(destination.red,destination.green,destination.blue,&hue,
            &saturation,&brightness);
          CompositeHSB(source.red,source.green,source.blue,&hue,&sans,&sans);
          HSBComposite(hue,saturation,brightness,&composite.red,
            &composite.green,&composite.blue);
          if (source.alpha < destination.alpha)
            composite.alpha=source.alpha;
          break;
        }
        case SaturateCompositeOp:
        {
          if (source.alpha == TransparentAlpha)
            break;
          if (destination.alpha == TransparentAlpha)
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
          if (source.alpha < destination.alpha)
            composite.alpha=source.alpha;
          break;
        }
        case LuminizeCompositeOp:
        {
          if (source.alpha == TransparentAlpha)
            break;
          if (destination.alpha == TransparentAlpha)
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
          if (source.alpha < destination.alpha)
            composite.alpha=source.alpha;
          break;
        }
        case ColorizeCompositeOp:
        {
          if (source.alpha == TransparentAlpha)
            break;
          if (destination.alpha == TransparentAlpha)
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
          if (source.alpha < destination.alpha)
            composite.alpha=source.alpha;
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
              composite.alpha=(MagickRealType) GetPixelInfoIntensity(&source);
              break;
            }
          composite.alpha=source.alpha;
          break;
        }
        case CopyBlackCompositeOp:
        {
          if (source.colorspace != CMYKColorspace)
            ConvertRGBToCMYK(&source);
          composite.black=source.black;
          break;
        }
        case BlurCompositeOp:
        case DisplaceCompositeOp:
        case DistortCompositeOp:
        {
          composite=source;
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
          composite.black=(MagickRealType) QuantumRange-composite.black;
        }
      SetPixelRed(image,ClampToQuantum(composite.red),q);
      SetPixelGreen(image,ClampToQuantum(composite.green),q);
      SetPixelBlue(image,ClampToQuantum(composite.blue),q);
      if (image->colorspace == CMYKColorspace)
        SetPixelBlack(image,ClampToQuantum(composite.black),q);
      SetPixelAlpha(image,ClampToQuantum(composite.alpha),q);
      p+=GetPixelComponents(composite_image);
      if (p >= (pixels+composite_image->columns*GetPixelComponents(composite_image)))
        p=pixels;
      q+=GetPixelComponents(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_CompositeImage)
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

  MagickBooleanType
    status;

  ssize_t
    y;

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
      #pragma omp parallel for schedule(dynamic,4) shared(status) omp_throttle(1)
#endif
      for (y=0; y < (ssize_t) image->rows; y+=(ssize_t) texture->rows)
      {
        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        for (x=0; x < (ssize_t) image->columns; x+=(ssize_t) texture->columns)
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
            proceed=SetImageProgress(image,TextureImageTag,(MagickOffsetType)
              y,image->rows);
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
  #pragma omp parallel for schedule(dynamic,4) shared(status) omp_throttle(1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      sync;

    register const Quantum
      *p,
      *pixels;

    register ssize_t
      x;

    register Quantum
      *q;

    size_t
      width;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewVirtualPixels(texture_view,texture->tile_offset.x,(y+
      texture->tile_offset.y) % texture->rows,texture->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
      exception);
    if ((pixels == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x+=(ssize_t) texture->columns)
    {
      register ssize_t
        i;

      p=pixels;
      width=texture->columns;
      if ((x+(ssize_t) width) > (ssize_t) image->columns)
        width=image->columns-x;
      for (i=0; i < (ssize_t) width; i++)
      {
        SetPixelRed(image,GetPixelRed(texture,p),q);
        SetPixelGreen(image,GetPixelGreen(texture,p),q);
        SetPixelBlue(image,GetPixelBlue(texture,p),q);
        SetPixelAlpha(image,GetPixelAlpha(texture,p),q);
        if ((image->colorspace == CMYKColorspace)  &&
            (texture->colorspace == CMYKColorspace))
          SetPixelBlack(image,GetPixelBlack(texture,p),q);
        p+=GetPixelComponents(texture);
        q+=GetPixelComponents(image);
      }
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
        proceed=SetImageProgress(image,TextureImageTag,(MagickOffsetType) y,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  texture_view=DestroyCacheView(texture_view);
  image_view=DestroyCacheView(image_view);
  return(status);
}

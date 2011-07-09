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
#include "magick/thread-private.h"
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
%        const ssize_t x_offset,const ssize_t y_offset)
%      MagickBooleanType CompositeImageChannel(Image *image,
%        const ChannelType channel,const CompositeOperator compose,
%        Image *composite_image,const ssize_t x_offset,const ssize_t y_offset)
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
**
** The above SVG definitions also definate that Mathematical Composition
** methods should use a 'Over' blending mode for Alpha Channel.
** It however was not applied for composition modes of 'Plus', 'Minus',
** the modulus versions of 'Add' and 'Subtract'.
**
**
** Mathematical operator changes to be applied from IM v6.7...
**
**  1/ Modulus modes 'Add' and 'Subtract' are obsoleted and renamed
**     'ModulusAdd' and 'ModulusSubtract' for clarity.
**
**  2/ All mathematical compositions work as per the SVG specification
**     with regard to blending.  This now includes 'ModulusAdd' and
**     'ModulusSubtract'.
**
**  3/ When the special channel flag 'sync' (syncronize channel updates)
**     is turned off (enabled by default) then mathematical compositions are
**     only performed on the channels specified, and are applied
**     independantally of each other.  In other words the mathematics is
**     performed as 'pure' mathematical operations, rather than as image
**     operations.
*/

static inline MagickRealType Atop(const MagickRealType p,
  const MagickRealType Sa,const MagickRealType q,
  const MagickRealType magick_unused(Da))
{
  return(p*Sa+q*(1.0-Sa));  /* Da optimized out,  Da/gamma => 1.0 */
}

static inline void CompositeAtop(const MagickPixelPacket *p,
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  composite->opacity=q->opacity;   /* optimized  Da = 1.0-Gamma */
  composite->red=Atop(p->red,Sa,q->red,1.0);
  composite->green=Atop(p->green,Sa,q->green,1.0);
  composite->blue=Atop(p->blue,Sa,q->blue,1.0);
  if (q->colorspace == CMYKColorspace)
    composite->index=Atop(p->index,Sa,q->index,1.0);
}

/*
  What is this Composition method for? Can't find any specification!
  WARNING this is not doing correct 'over' blend handling (Anthony Thyssen).
*/
static inline void CompositeBumpmap(const MagickPixelPacket *p,
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    intensity;

  intensity=MagickPixelIntensity(p);
  composite->red=QuantumScale*intensity*q->red;
  composite->green=QuantumScale*intensity*q->green;
  composite->blue=QuantumScale*intensity*q->blue;
  composite->opacity=(MagickRealType) QuantumScale*intensity*
    p->opacity;
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
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
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
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
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
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  /*
    Darken is equivalent to a 'Minimum' method
    OR a greyscale version of a binary 'Or'
    OR the 'Intersection' of pixel sets.
  */
  MagickRealType
    gamma;

  if ( (channel & SyncChannels) != 0 ) {
    composite->opacity=QuantumScale*p->opacity*q->opacity; /* Over Blend */
    gamma=1.0-QuantumScale*composite->opacity;
    gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
    composite->red=gamma*Darken(p->red,p->opacity,q->red,q->opacity);
    composite->green=gamma*Darken(p->green,p->opacity,q->green,q->opacity);
    composite->blue=gamma*Darken(p->blue,p->opacity,q->blue,q->opacity);
    if (q->colorspace == CMYKColorspace)
      composite->index=gamma*Darken(p->index,p->opacity,q->index,q->opacity);
  }
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=MagickMax(p->opacity,q->opacity);
    if ( (channel & RedChannel) != 0 )
      composite->red=MagickMin(p->red,q->red);
    if ( (channel & GreenChannel) != 0 )
      composite->green=MagickMin(p->green,q->green);
    if ( (channel & BlueChannel) != 0 )
      composite->blue=MagickMin(p->blue,q->blue);
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=MagickMin(p->index,q->index);
  }
}

static inline void CompositeDarkenIntensity(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  /*
    Select the pixel based on the intensity level.
    If 'Sync' flag select whole pixel based on alpha weighted intensity.
    Otherwise use intensity only, but restrict copy according to channel.
  */
  if ( (channel & SyncChannels) != 0 ) {
    MagickRealType
      Da,
      Sa;

    Sa=1.0-QuantumScale*p->opacity;
    Da=1.0-QuantumScale*q->opacity;
    *composite = (Sa*MagickPixelIntensity(p) < Da*MagickPixelIntensity(q))
              ? *p : *q;
  }
  else {
    int from_p = (MagickPixelIntensity(p) < MagickPixelIntensity(q));
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity = from_p ? p->opacity : q->opacity;
    if ( (channel & RedChannel) != 0 )
      composite->red = from_p ? p->red : q->red;
    if ( (channel & GreenChannel) != 0 )
      composite->green = from_p ? p->green : q->green;
    if ( (channel & BlueChannel) != 0 )
      composite->blue = from_p ? p->blue : q->blue;
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index = from_p ? p->index : q->index;
  }
}

static inline MagickRealType Difference(const MagickRealType p,
  const MagickRealType Sa,const MagickRealType q,const MagickRealType Da)
{
  /* Optimized by Multipling by QuantumRange (taken from gamma).  */
  return(Sa*p+Da*q-Sa*Da*2.0*MagickMin(p,q));
}

static inline void CompositeDifference(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
  if ( (channel & SyncChannels) != 0 ) {
    gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
    composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
    gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
    /* Values are not normalized as an optimization.  */
    composite->red=gamma*Difference(p->red,Sa,q->red,Da);
    composite->green=gamma*Difference(p->green,Sa,q->green,Da);
    composite->blue=gamma*Difference(p->blue,Sa,q->blue,Da);
    if (q->colorspace == CMYKColorspace)
      composite->index=gamma*Difference(p->index,Sa,q->index,Da);
  }
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=QuantumRange-fabs(p->opacity - q->opacity);
    if ( (channel & RedChannel) != 0 )
      composite->red=fabs(p->red - q->red);
    if ( (channel & GreenChannel) != 0 )
      composite->green=fabs(p->green - q->green);
    if ( (channel & BlueChannel) != 0 )
      composite->blue=fabs(p->blue - q->blue);
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=fabs(p->index - q->index);
  }
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

static inline void CompositeDivide(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
  if ( (channel & SyncChannels) != 0 ) {
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
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=QuantumRange*(1.0-Divide(Sa,1.0,Da,1.0));
    if ( (channel & RedChannel) != 0 )
      composite->red=QuantumRange*
          Divide(QuantumScale*p->red,1.0,QuantumScale*q->red,1.0);
    if ( (channel & GreenChannel) != 0 )
      composite->green=QuantumRange*
          Divide(QuantumScale*p->green,1.0,QuantumScale*q->green,1.0);
    if ( (channel & BlueChannel) != 0 )
      composite->blue=QuantumRange*
          Divide(QuantumScale*p->blue,1.0,QuantumScale*q->blue,1.0);
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=QuantumRange*
          Divide(QuantumScale*p->index,1.0,QuantumScale*q->index,1.0);
  }
}

static MagickRealType Exclusion(const MagickRealType Sca,
  const MagickRealType Sa, const MagickRealType Dca,const MagickRealType Da)
{
  return(Sca*Da+Dca*Sa-2.0*Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeExclusion(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  MagickRealType
    gamma,
    Sa,
    Da;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
  if ( (channel & SyncChannels) != 0 ) {
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
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=QuantumRange*(1.0-Exclusion(Sa,1.0,Da,1.0));
    if ( (channel & RedChannel) != 0 )
      composite->red=QuantumRange*
          Exclusion(QuantumScale*p->red,1.0,QuantumScale*q->red,1.0);
    if ( (channel & GreenChannel) != 0 )
      composite->green=QuantumRange*
          Exclusion(QuantumScale*p->green,1.0,QuantumScale*q->green,1.0);
    if ( (channel & BlueChannel) != 0 )
      composite->blue=QuantumRange*
          Exclusion(QuantumScale*p->blue,1.0,QuantumScale*q->blue,1.0);
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=QuantumRange*
          Exclusion(QuantumScale*p->index,1.0,QuantumScale*q->index,1.0);
  }
}

static MagickRealType HardLight(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  if ((2.0*Sca) < Sa)
    return(2.0*Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
  return(Sa*Da-2.0*(Da-Dca)*(Sa-Sca)+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeHardLight(const MagickPixelPacket *p,
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
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
  const MagickRealType Sa,const MagickRealType magick_unused(q),
  const MagickRealType Da)
{
  return(Sa*p*Da);
}

static inline void CompositeIn(const MagickPixelPacket *p,
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    gamma,
    Sa,
    Da;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
  gamma=Sa*Da;
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*In(p->red,Sa,q->red,Da);
  composite->green=gamma*In(p->green,Sa,q->green,Da);
  composite->blue=gamma*In(p->blue,Sa,q->blue,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*In(p->index,Sa,q->index,Da);
}

static inline MagickRealType Lighten(const MagickRealType p,
  const MagickRealType alpha,const MagickRealType q,const MagickRealType beta)
{
   if (p > q)
     return(MagickOver_(p,alpha,q,beta));  /* src-over */
   return(MagickOver_(q,beta,p,alpha));    /* dst-over */
}

static inline void CompositeLighten(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  /*
    Lighten is also equvalent to a 'Maximum' method
    OR a greyscale version of a binary 'And'
    OR the 'Union' of pixel sets.
  */
  MagickRealType
    gamma;

  if ( (channel & SyncChannels) != 0 ) {
    composite->opacity=QuantumScale*p->opacity*q->opacity; /* Over Blend */
    gamma=1.0-QuantumScale*composite->opacity;
    gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
    composite->red=gamma*Lighten(p->red,p->opacity,q->red,q->opacity);
    composite->green=gamma*Lighten(p->green,p->opacity,q->green,q->opacity);
    composite->blue=gamma*Lighten(p->blue,p->opacity,q->blue,q->opacity);
    if (q->colorspace == CMYKColorspace)
      composite->index=gamma*Lighten(p->index,p->opacity,q->index,q->opacity);
  }
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=MagickMin(p->opacity,q->opacity);
    if ( (channel & RedChannel) != 0 )
      composite->red=MagickMax(p->red,q->red);
    if ( (channel & GreenChannel) != 0 )
      composite->green=MagickMax(p->green,q->green);
    if ( (channel & BlueChannel) != 0 )
      composite->blue=MagickMax(p->blue,q->blue);
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=MagickMax(p->index,q->index);
  }
}

static inline void CompositeLightenIntensity(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  /*
    Select the pixel based on the intensity level.
    If 'Sync' flag select whole pixel based on alpha weighted intensity.
    Otherwise use Intenisty only, but restrict copy according to channel.
  */
  if ( (channel & SyncChannels) != 0 ) {
    MagickRealType
      Da,
      Sa;

    Sa=1.0-QuantumScale*p->opacity;
    Da=1.0-QuantumScale*q->opacity;
    *composite = (Sa*MagickPixelIntensity(p) > Da*MagickPixelIntensity(q))
               ? *p : *q;
  }
  else {
    int from_p = (MagickPixelIntensity(p) > MagickPixelIntensity(q));
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity = from_p ? p->opacity : q->opacity;
    if ( (channel & RedChannel) != 0 )
      composite->red = from_p ? p->red : q->red;
    if ( (channel & GreenChannel) != 0 )
      composite->green = from_p ? p->green : q->green;
    if ( (channel & BlueChannel) != 0 )
      composite->blue = from_p ? p->blue : q->blue;
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index = from_p ? p->index : q->index;
  }
}

#if 0
static inline MagickRealType LinearDodge(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  /*
    LinearDodge: simplifies to a trivial formula
    f(Sc,Dc) = Sc + Dc
    Dca' = Sca + Dca
  */
  return(Sca+Dca);
}
#endif

static inline void CompositeLinearDodge(const MagickPixelPacket *p,
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
  gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
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
    LinearBurn: as defined by Abode Photoshop, according to
    http://www.simplefilter.de/en/basics/mixmods.html is:

    f(Sc,Dc) = Sc + Dc - 1
  */
  return(Sca+Dca-Sa*Da);
}

static inline void CompositeLinearBurn(const MagickPixelPacket *p,
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
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
    Previous formula, was only valid for fully-opaque images.
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
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
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
  const MagickPixelPacket *q,const ChannelType channel, const GeometryInfo
  *args, MagickPixelPacket *composite)
{
  MagickRealType
    Sa,
    Da,
    gamma;

  Sa=1.0-QuantumScale*p->opacity; /* ??? - AT */
  Da=1.0-QuantumScale*q->opacity;
  if ( (channel & SyncChannels) != 0 ) {
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
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=QuantumRange*(1.0-Mathematics(Sa,1.0,Da,1.0,args));
    if ( (channel & RedChannel) != 0 )
      composite->red=QuantumRange*
          Mathematics(QuantumScale*p->red,1.0,QuantumScale*q->red,1.0,args);
    if ( (channel & GreenChannel) != 0 )
      composite->green=QuantumRange*
          Mathematics(QuantumScale*p->green,1.0,QuantumScale*q->green,1.0,args);
    if ( (channel & BlueChannel) != 0 )
      composite->blue=QuantumRange*
          Mathematics(QuantumScale*p->blue,1.0,QuantumScale*q->blue,1.0,args);
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=QuantumRange*
          Mathematics(QuantumScale*p->index,1.0,QuantumScale*q->index,1.0,args);
  }

}

static inline void CompositePlus(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  if ( (channel & SyncChannels) != 0 ) {
    /*
      NOTE: "Plus" does not use 'over' alpha-blending but uses a
      special 'plus' form of alph-blending. It is the ONLY mathematical
      operator to do this. this is what makes it different to the
      otherwise equivalent "LinearDodge" composition method.

      Note however that color channels are still effected by the alpha channel
      as a result of the blending, making it just as useless for independant
      channel maths, just like all other mathematical composition methods.

      As such the removal of the 'sync' flag, is still a usful convention.

      The MagickPixelCompositePlus() function is defined in
      "composite-private.h" so it can also be used for Image Blending.
    */
    MagickPixelCompositePlus(p,p->opacity,q,q->opacity,composite);
  }
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=p->opacity+q->opacity-QuantumRange;
    if ( (channel & RedChannel) != 0 )
      composite->red=p->red+q->red;
    if ( (channel & GreenChannel) != 0 )
      composite->green=p->green+q->green;
    if ( (channel & BlueChannel) != 0 )
      composite->blue=p->blue+q->blue;
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=p->index+q->index;
  }
}

static inline MagickRealType Minus(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,
  const MagickRealType magick_unused(Da))
{
  /*
    Minus Source from Destination

      f(Sc,Dc) = Sc - Dc

  */
  return(Sca + Dca - 2*Dca*Sa);
}

static inline void CompositeMinus(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  MagickRealType
    Sa,
    Da,
    gamma;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
  if ( (channel & SyncChannels) != 0 ) {
    gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
    composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
    gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
    composite->red=gamma*Minus(p->red*Sa,Sa,q->red*Da,Da);
    composite->green=gamma*Minus(p->green*Sa,Sa,q->green*Da,Da);
    composite->blue=gamma*Minus(p->blue*Sa,Sa,q->blue*Da,Da);
    if (q->colorspace == CMYKColorspace)
      composite->index=gamma*Minus(p->index*Sa,Sa,q->index*Da,Da);
  }
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=QuantumRange*(1.0-(Sa-Da));
    if ( (channel & RedChannel) != 0 )
      composite->red=p->red-q->red;
    if ( (channel & GreenChannel) != 0 )
      composite->green=p->green-q->green;
    if ( (channel & BlueChannel) != 0 )
      composite->blue=p->blue-q->blue;
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=p->index-q->index;
  }
}

static inline MagickRealType ModulusAdd(const MagickRealType p,
  const MagickRealType Sa, const MagickRealType q,  const MagickRealType Da)
{
  MagickRealType
    pixel;

  pixel=p+q;
  if (pixel > QuantumRange)
    pixel-=(QuantumRange+1.0);
  return(pixel*Sa*Da + p*Sa*(1-Da) + q*Da*(1-Sa));
}

static inline void CompositeModulusAdd(const MagickPixelPacket *p,
  const MagickPixelPacket *q, const ChannelType channel,
  MagickPixelPacket *composite)
{
  if ( (channel & SyncChannels) != 0 ) {
    MagickRealType
      Sa,
      Da,
      gamma;

    Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
    Da=1.0-QuantumScale*q->opacity;
    gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
    composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
    gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
    composite->red=ModulusAdd(p->red,Sa,q->red,Da);
    composite->green=ModulusAdd(p->green,Sa,q->green,Da);
    composite->blue=ModulusAdd(p->blue,Sa,q->blue,Da);
    if (q->colorspace == CMYKColorspace)
      composite->index=ModulusAdd(p->index,Sa,q->index,Da);
  }
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=QuantumRange-ModulusAdd(QuantumRange-p->opacity,
           1.0,QuantumRange-q->opacity,1.0);
    if ( (channel & RedChannel) != 0 )
      composite->red=ModulusAdd(p->red,1.0,q->red,1.0);
    if ( (channel & GreenChannel) != 0 )
      composite->green=ModulusAdd(p->green,1.0,q->green,1.0);
    if ( (channel & BlueChannel) != 0 )
      composite->blue=ModulusAdd(p->blue,1.0,q->blue,1.0);
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=ModulusAdd(p->index,1.0,q->index,1.0);
  }
}

static inline MagickRealType ModulusSubtract(const MagickRealType p,
  const MagickRealType Sa, const MagickRealType q,  const MagickRealType Da)
{
  MagickRealType
    pixel;

  pixel=p-q;
  if (pixel < 0.0)
    pixel+=(QuantumRange+1.0);
  return(pixel*Sa*Da + p*Sa*(1-Da) + q*Da*(1-Sa));
}

static inline void CompositeModulusSubtract(const MagickPixelPacket *p,
  const MagickPixelPacket *q, const ChannelType channel,
  MagickPixelPacket *composite)
{
  if ( (channel & SyncChannels) != 0 ) {
    MagickRealType
      Sa,
      Da,
      gamma;

    Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
    Da=1.0-QuantumScale*q->opacity;
    gamma = RoundToUnity(Sa+Da-Sa*Da);
    composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
    gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
    composite->red=ModulusSubtract(p->red,Sa,q->red,Da);
    composite->green=ModulusSubtract(p->green,Sa,q->green,Da);
    composite->blue=ModulusSubtract(p->blue,Sa,q->blue,Da);
    if (q->colorspace == CMYKColorspace)
      composite->index=ModulusSubtract(p->index,Sa,q->index,Da);
  }
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=QuantumRange-ModulusSubtract(QuantumRange-p->opacity,
           1.0,QuantumRange-q->opacity,1.0);
    if ( (channel & RedChannel) != 0 )
      composite->red=ModulusSubtract(p->red,1.0,q->red,1.0);
    if ( (channel & GreenChannel) != 0 )
      composite->green=ModulusSubtract(p->green,1.0,q->green,1.0);
    if ( (channel & BlueChannel) != 0 )
      composite->blue=ModulusSubtract(p->blue,1.0,q->blue,1.0);
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=ModulusSubtract(p->index,1.0,q->index,1.0);
  }
}

static  inline MagickRealType Multiply(const MagickRealType Sca,
  const MagickRealType Sa,const MagickRealType Dca,const MagickRealType Da)
{
  return(Sca*Dca+Sca*(1.0-Da)+Dca*(1.0-Sa));
}

static inline void CompositeMultiply(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
  if ( (channel & SyncChannels) != 0 ) {
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
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=QuantumRange*(1.0-Sa*Da);
    if ( (channel & RedChannel) != 0 )
      composite->red=QuantumScale*p->red*q->red;
    if ( (channel & GreenChannel) != 0 )
      composite->green=QuantumScale*p->green*q->green;
    if ( (channel & BlueChannel) != 0 )
      composite->blue=QuantumScale*p->blue*q->blue;
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=QuantumScale*p->index*q->index;
  }
}

static inline MagickRealType Out(const MagickRealType p,
  const MagickRealType Sa,const MagickRealType magick_unused(q),
  const MagickRealType Da)
{
  return(Sa*p*(1.0-Da));
}

static inline void CompositeOut(const MagickPixelPacket *p,
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Sa,
    Da,
    gamma;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
  gamma=Sa*(1.0-Da);
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Out(p->red,Sa,q->red,Da);
  composite->green=gamma*Out(p->green,Sa,q->green,Da);
  composite->blue=gamma*Out(p->blue,Sa,q->blue,Da);
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Out(p->index,Sa,q->index,Da);
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
  return(Dca*Dca*(Sa-2*Sca)/Da+Sca*(2*Dca+1-Da)+Dca*(1-Sa));
}

static inline void CompositePegtopLight(const MagickPixelPacket *p,
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
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
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
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

static inline MagickRealType Screen(const MagickRealType Sca,
  const MagickRealType Dca)
{
  /* Screen:  A negated multiply
     f(Sc,Dc) = 1.0-(1.0-Sc)*(1.0-Dc)
  */
  return(Sca+Dca-Sca*Dca);
}

static inline void CompositeScreen(const MagickPixelPacket *p,
  const MagickPixelPacket *q,const ChannelType channel,
  MagickPixelPacket *composite)
{
  MagickRealType
    Sa,
    Da,
    gamma;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
  if ( (channel & SyncChannels) != 0 ) {
    gamma=RoundToUnity(Sa+Da-Sa*Da); /* over blend, as per SVG doc */
    composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
    Sa*=QuantumScale; Da*=QuantumScale; /* optimization */
    gamma=QuantumRange/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
    composite->red=gamma*Screen(p->red*Sa,q->red*Da);
    composite->green=gamma*Screen(p->green*Sa,q->green*Da);
    composite->blue=gamma*Screen(p->blue*Sa,q->blue*Da);
    if (q->colorspace == CMYKColorspace)
      composite->index=gamma*Screen(p->index*Sa,q->index*Da);
    }
  else { /* handle channels as separate grayscale channels */
    if ( (channel & AlphaChannel) != 0 )
      composite->opacity=QuantumRange*(1.0-Screen(Sa,Da));
    if ( (channel & RedChannel) != 0 )
      composite->red=QuantumRange*Screen(QuantumScale*p->red,
           QuantumScale*q->red);
    if ( (channel & GreenChannel) != 0 )
      composite->green=QuantumRange*Screen(QuantumScale*p->green,
           QuantumScale*q->green);
    if ( (channel & BlueChannel) != 0 )
      composite->blue=QuantumRange*Screen(QuantumScale*p->blue,
           QuantumScale*q->blue);
    if ( (channel & IndexChannel) != 0 && q->colorspace == CMYKColorspace)
      composite->index=QuantumRange*Screen(QuantumScale*p->index,
           QuantumScale*q->index);
  }
}

static MagickRealType SoftLight(const MagickRealType Sca,
  const MagickRealType Sa, const MagickRealType Dca, const MagickRealType Da)
{
#if 0
  /*
    Oct 2004 SVG specification -- was found to be incorrect
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
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
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

/*
  Depreciated
  Multiply difference by amount, if differance larger than threshold???
  What use this is is completely unknown
  The Opacity calculation appears to be inverted  -- Anthony Thyssen
*/
static inline MagickRealType Threshold(const MagickRealType p,
  const MagickRealType q,const MagickRealType threshold,
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
  const MagickPixelPacket *q,const MagickRealType threshold,
  const MagickRealType amount,MagickPixelPacket *composite)
{
  composite->red=Threshold(p->red,q->red,threshold,amount);
  composite->green=Threshold(p->green,q->green,threshold,amount);
  composite->blue=Threshold(p->blue,q->blue,threshold,amount);
  composite->opacity=QuantumRange-Threshold(p->opacity,q->opacity,
       threshold,amount);
  if (q->colorspace == CMYKColorspace)
    composite->index=Threshold(p->index,q->index,threshold,amount);
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
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
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
  const MagickPixelPacket *q,MagickPixelPacket *composite)
{
  MagickRealType
    Da,
    gamma,
    Sa;

  Sa=1.0-QuantumScale*p->opacity;  /* simplify and speed up equations */
  Da=1.0-QuantumScale*q->opacity;
  gamma=Sa+Da-2*Sa*Da;        /* Xor blend mode X=0,Y=1,Z=1 */
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
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
  const ssize_t x_offset,const ssize_t y_offset)
{
  MagickBooleanType
    status;

  status=CompositeImageChannel(image,DefaultChannels,compose,composite_image,
    x_offset,y_offset);
  return(status);
}

MagickExport MagickBooleanType CompositeImageChannel(Image *image,
  const ChannelType channel,const CompositeOperator compose,
  const Image *composite_image,const ssize_t x_offset,const ssize_t y_offset)
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

        register const PixelPacket
          *restrict p;

        register PixelPacket
          *restrict r;

        register IndexPacket
          *restrict destination_indexes;

        register ssize_t
          x;

        if (((y+y_offset) < 0) || ((y+y_offset) >= (ssize_t) image->rows))
          continue;
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        r=QueueCacheViewAuthenticPixels(destination_view,0,y,
          destination_image->columns,1,&image->exception);
        if ((p == (const PixelPacket *) NULL) || (r == (PixelPacket *) NULL))
          break;
        destination_indexes=GetCacheViewAuthenticIndexQueue(destination_view);
        for (x=0; x < (ssize_t) composite_image->columns; x++)
        {
          if (((x_offset+x) < 0) || ((x_offset+x) >= (ssize_t) image->columns))
            {
              p++;
              continue;
            }
          if (fabs(angle_range) > MagickEpsilon)
            {
              MagickRealType
                angle;

              angle=angle_start+angle_range*QuantumScale*
                GetPixelBlue(p);
              blur.x1=width*cos(angle);
              blur.x2=width*sin(angle);
              blur.y1=(-height*sin(angle));
              blur.y2=height*cos(angle);
            }
          ScaleResampleFilter(resample_filter,blur.x1*QuantumScale*
            GetPixelRed(p),blur.y1*QuantumScale*
            GetPixelGreen(p),blur.x2*QuantumScale*
            GetPixelRed(p),blur.y2*QuantumScale*
            GetPixelGreen(p));
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
        *destination_view,
        *image_view;

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

        register const PixelPacket
          *restrict p;

        register ssize_t
          x;

        if (((y+y_offset) < 0) || ((y+y_offset) >= (ssize_t) image->rows))
          continue;
        p=GetCacheViewVirtualPixels(composite_view,0,y,composite_image->columns,
          1,exception);
        r=QueueCacheViewAuthenticPixels(destination_view,0,y,
          destination_image->columns,1,&image->exception);
        if ((p == (const PixelPacket *) NULL) || (r == (PixelPacket *) NULL))
          break;
        destination_indexes=GetCacheViewAuthenticIndexQueue(destination_view);
        for (x=0; x < (ssize_t) composite_image->columns; x++)
        {
          if (((x_offset+x) < 0) || ((x_offset+x) >= (ssize_t) image->columns))
            {
              p++;
              continue;
            }
          /*
            Displace the offset.
          */
          offset.x=(horizontal_scale*(GetPixelRed(p)-
            (((MagickRealType) QuantumRange+1.0)/2.0)))/(((MagickRealType)
            QuantumRange+1.0)/2.0)+center.x+((compose == DisplaceCompositeOp) ?
            x : 0);
          offset.y=(vertical_scale*(GetPixelGreen(p)-
            (((MagickRealType) QuantumRange+1.0)/2.0)))/(((MagickRealType)
            QuantumRange+1.0)/2.0)+center.y+((compose == DisplaceCompositeOp) ?
            y : 0);
          (void) InterpolateMagickPixelPacket(image,image_view,
            UndefinedInterpolatePixel,(double) offset.x,(double) offset.y,
            &pixel,exception);
          /*
            Mask with the 'invalid pixel mask' in alpha channel.
          */
          pixel.opacity=(MagickRealType) QuantumRange*(1.0-(1.0-QuantumScale*
            pixel.opacity)*(1.0-QuantumScale*GetPixelOpacity(p)));
          SetPixelPacket(destination_image,&pixel,r,destination_indexes+x);
          p++;
          r++;
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
        This Composition method is depreciated
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
  for (y=0; y < (ssize_t) image->rows; y++)
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

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

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
    pixels=(PixelPacket *) NULL;
    p=(PixelPacket *) NULL;
    if ((y >= y_offset) && ((y-y_offset) < (ssize_t) composite_image->rows))
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
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
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
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (modify_outside_overlay == MagickFalse)
        {
          if (x < x_offset)
            {
              q++;
              continue;
            }
          if ((x-x_offset) >= (ssize_t) composite_image->columns)
            break;
        }
      destination.red=(MagickRealType) GetPixelRed(q);
      destination.green=(MagickRealType) GetPixelGreen(q);
      destination.blue=(MagickRealType) GetPixelBlue(q);
      if (image->matte != MagickFalse)
        destination.opacity=(MagickRealType) GetPixelOpacity(q);
      if (image->colorspace == CMYKColorspace)
        destination.index=(MagickRealType) GetPixelIndex(indexes+x);
      if (image->colorspace == CMYKColorspace)
        {
          destination.red=(MagickRealType) QuantumRange-destination.red;
          destination.green=(MagickRealType) QuantumRange-destination.green;
          destination.blue=(MagickRealType) QuantumRange-destination.blue;
          destination.index=(MagickRealType) QuantumRange-destination.index;
        }
      /*
        Handle destination modifications outside overlaid region.
      */
      composite=destination;
      if ((pixels == (PixelPacket *) NULL) || (x < x_offset) ||
          ((x-x_offset) >= (ssize_t) composite_image->columns))
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
          SetPixelRed(q,ClampToQuantum(composite.red));
          SetPixelGreen(q,ClampToQuantum(composite.green));
          SetPixelBlue(q,ClampToQuantum(composite.blue));
          if (image->matte != MagickFalse)
            SetPixelOpacity(q,ClampToQuantum(composite.opacity));
          if (image->colorspace == CMYKColorspace)
            SetPixelIndex(indexes+x,ClampToQuantum(composite.index));
          q++;
          continue;
        }
      /*
        Handle normal overlay of source onto destination.
      */
      source.red=(MagickRealType) GetPixelRed(p);
      source.green=(MagickRealType) GetPixelGreen(p);
      source.blue=(MagickRealType) GetPixelBlue(p);
      if (composite_image->matte != MagickFalse)
        source.opacity=(MagickRealType) GetPixelOpacity(p);
      if (composite_image->colorspace == CMYKColorspace)
        source.index=(MagickRealType) GetPixelIndex(composite_indexes+
          x-x_offset);
      if (composite_image->colorspace == CMYKColorspace)
        {
          source.red=(MagickRealType) QuantumRange-source.red;
          source.green=(MagickRealType) QuantumRange-source.green;
          source.blue=(MagickRealType) QuantumRange-source.blue;
          source.index=(MagickRealType) QuantumRange-source.index;
        }
      switch (compose)
      {
        /* Duff-Porter Compositions */
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
          MagickPixelCompositeOver(&source,source.opacity,&destination,
            destination.opacity,&composite);
          break;
        }
        case DstOverCompositeOp:
        {
          MagickPixelCompositeOver(&destination,destination.opacity,&source,
            source.opacity,&composite);
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
        /* Mathematical Compositions */
        case PlusCompositeOp:
        {
          CompositePlus(&source,&destination,channel,&composite);
          break;
        }
        case MinusDstCompositeOp:
        {
          CompositeMinus(&source,&destination,channel,&composite);
          break;
        }
        case MinusSrcCompositeOp:
        {
          CompositeMinus(&destination,&source,channel,&composite);
          break;
        }
        case ModulusAddCompositeOp:
        {
          CompositeModulusAdd(&source,&destination,channel,&composite);
          break;
        }
        case ModulusSubtractCompositeOp:
        {
          CompositeModulusSubtract(&source,&destination,channel,&composite);
          break;
        }
        case DifferenceCompositeOp:
        {
          CompositeDifference(&source,&destination,channel,&composite);
          break;
        }
        case ExclusionCompositeOp:
        {
          CompositeExclusion(&source,&destination,channel,&composite);
          break;
        }
        case MultiplyCompositeOp:
        {
          CompositeMultiply(&source,&destination,channel,&composite);
          break;
        }
        case ScreenCompositeOp:
        {
          CompositeScreen(&source,&destination,channel,&composite);
          break;
        }
        case DivideDstCompositeOp:
        {
          CompositeDivide(&source,&destination,channel,&composite);
          break;
        }
        case DivideSrcCompositeOp:
        {
          CompositeDivide(&destination,&source,channel,&composite);
          break;
        }
        case DarkenCompositeOp:
        {
          CompositeDarken(&source,&destination,channel,&composite);
          break;
        }
        case LightenCompositeOp:
        {
          CompositeLighten(&source,&destination,channel,&composite);
          break;
        }
        case DarkenIntensityCompositeOp:
        {
          CompositeDarkenIntensity(&source,&destination,channel,&composite);
          break;
        }
        case LightenIntensityCompositeOp:
        {
          CompositeLightenIntensity(&source,&destination,channel,&composite);
          break;
        }
        case MathematicsCompositeOp:
        {
          CompositeMathematics(&source,&destination,channel,&geometry_info,
            &composite);
          break;
        }
        /* Lighting Compositions */
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
          /* Overlay = Reversed HardLight. */
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
        /* Other Composition */
        case ChangeMaskCompositeOp:
        {
          if ((composite.opacity > ((MagickRealType) QuantumRange/2.0)) ||
              (IsMagickColorSimilar(&source,&destination) != MagickFalse))
            composite.opacity=(MagickRealType) TransparentOpacity;
          else
            composite.opacity=(MagickRealType) OpaqueOpacity;
          break;
        }
        case BumpmapCompositeOp:
        {
          if (source.opacity == TransparentOpacity)
            break;
          CompositeBumpmap(&source,&destination,&composite);
          break;
        }
        case DissolveCompositeOp:
        {
          MagickPixelCompositeOver(&source,(MagickRealType) (QuantumRange-
            source_dissolve*(QuantumRange-source.opacity)),&destination,
            (MagickRealType) (QuantumRange-destination_dissolve*(QuantumRange-
            destination.opacity)),&composite);
          break;
        }
        case BlendCompositeOp:
        {
          MagickPixelCompositeBlend(&source,source_dissolve,&destination,
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

          if (source.opacity == TransparentOpacity)
            break;
          offset=(ssize_t) (MagickPixelIntensityToQuantum(&source)-midpoint);
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
        /* compose methods that are already handled */
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
          composite.index=(MagickRealType) QuantumRange-composite.index;
        }
      SetPixelRed(q,ClampToQuantum(composite.red));
      SetPixelGreen(q,ClampToQuantum(composite.green));
      SetPixelBlue(q,ClampToQuantum(composite.blue));
      SetPixelOpacity(q,ClampToQuantum(composite.opacity));
      if (image->colorspace == CMYKColorspace)
        SetPixelIndex(indexes+x,ClampToQuantum(composite.index));
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

    register const IndexPacket
      *texture_indexes;

    register const PixelPacket
      *p;

    register IndexPacket
      *indexes;

    register ssize_t
      x;

    register PixelPacket
      *q;

    size_t
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
    for (x=0; x < (ssize_t) image->columns; x+=(ssize_t) texture->columns)
    {
      width=texture->columns;
      if ((x+(ssize_t) width) > (ssize_t) image->columns)
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

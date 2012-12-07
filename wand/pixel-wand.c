/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      PPPP   IIIII  X   X  EEEEE  L                          %
%                      P   P    I     X X   E      L                          %
%                      PPPP     I      X    EEE    L                          %
%                      P        I     X X   E      L                          %
%                      P      IIIII  X   X  EEEEE  LLLLL                      %
%                                                                             %
%                         W   W   AAA   N   N  DDDD                           %
%                         W   W  A   A  NN  N  D   D                          %
%                         W W W  AAAAA  N N N  D   D                          %
%                         WW WW  A   A  N  NN  D   D                          %
%                         W   W  A   A  N   N  DDDD                           %
%                                                                             %
%                                                                             %
%                    MagickWand Image Pixel Wand Methods                      %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                March 2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wand/studio.h"
#include "wand/MagickWand.h"
#include "wand/magick-wand-private.h"
#include "wand/pixel-wand-private.h"
#include "wand/wand.h"

/*
  Define declarations.
*/
#define PixelWandId  "PixelWand"

/*
  Typedef declarations.
*/
struct _PixelWand
{
  size_t
    id;

  char
    name[MaxTextExtent];

  ExceptionInfo
    *exception;

  MagickPixelPacket
    pixel;

  size_t
    count;

  MagickBooleanType
    debug;

  size_t
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l e a r P i x e l W a n d                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClearPixelWand() clears resources associated with the wand.
%
%  The format of the ClearPixelWand method is:
%
%      void ClearPixelWand(PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport void ClearPixelWand(PixelWand *wand)
{
  assert(wand != (PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  ClearMagickException(wand->exception);
  wand->pixel.colorspace=sRGBColorspace;
  wand->debug=IsEventLogging();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e P i x e l W a n d                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClonePixelWand() makes an exact copy of the specified wand.
%
%  The format of the ClonePixelWand method is:
%
%      PixelWand *ClonePixelWand(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport PixelWand *ClonePixelWand(const PixelWand *wand)
{
  PixelWand
    *clone_wand;

  assert(wand != (PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  clone_wand=(PixelWand *) AcquireMagickMemory(sizeof(*clone_wand));
  if (clone_wand == (PixelWand *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      wand->name);
  (void) ResetMagickMemory(clone_wand,0,sizeof(*clone_wand));
  clone_wand->id=AcquireWandId();
  (void) FormatLocaleString(clone_wand->name,MaxTextExtent,"%s-%.20g",
    PixelWandId,(double) clone_wand->id);
  clone_wand->exception=AcquireExceptionInfo();
  InheritException(clone_wand->exception,wand->exception);
  clone_wand->pixel=wand->pixel;
  clone_wand->count=wand->count;
  clone_wand->debug=IsEventLogging();
  if (clone_wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",clone_wand->name);
  clone_wand->signature=WandSignature;
  return(clone_wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e P i x e l W a n d s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClonePixelWands() makes an exact copy of the specified wands.
%
%  The format of the ClonePixelWands method is:
%
%      PixelWand **ClonePixelWands(const PixelWand **wands,
%        const size_t number_wands)
%
%  A description of each parameter follows:
%
%    o wands: the magick wands.
%
%    o number_wands: the number of wands.
%
*/
WandExport PixelWand **ClonePixelWands(const PixelWand **wands,
  const size_t number_wands)
{
  register ssize_t
    i;

  PixelWand
    **clone_wands;

  clone_wands=(PixelWand **) AcquireQuantumMemory((size_t) number_wands,
    sizeof(*clone_wands));
  if (clone_wands == (PixelWand **) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  for (i=0; i < (ssize_t) number_wands; i++)
    clone_wands[i]=ClonePixelWand(wands[i]);
  return(clone_wands);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y P i x e l W a n d                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyPixelWand() deallocates resources associated with a PixelWand.
%
%  The format of the DestroyPixelWand method is:
%
%      PixelWand *DestroyPixelWand(PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport PixelWand *DestroyPixelWand(PixelWand *wand)
{
  assert(wand != (PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->exception=DestroyExceptionInfo(wand->exception);
  wand->signature=(~WandSignature);
  RelinquishWandId(wand->id);
  wand=(PixelWand *) RelinquishMagickMemory(wand);
  return(wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y P i x e l W a n d s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyPixelWands() deallocates resources associated with an array of
%  pixel wands.
%
%  The format of the DestroyPixelWands method is:
%
%      PixelWand **DestroyPixelWands(PixelWand **wand,
%        const size_t number_wands)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o number_wands: the number of wands.
%
*/
WandExport PixelWand **DestroyPixelWands(PixelWand **wand,
  const size_t number_wands)
{
  register ssize_t
    i;

  assert(wand != (PixelWand **) NULL);
  assert(*wand != (PixelWand *) NULL);
  assert((*wand)->signature == WandSignature);
  if ((*wand)->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",(*wand)->name);
  for (i=(ssize_t) number_wands-1; i >= 0; i--)
    wand[i]=DestroyPixelWand(wand[i]);
  wand=(PixelWand **) RelinquishMagickMemory(wand);
  return(wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P i x e l W a n d S i m i l a r                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPixelWandSimilar() returns MagickTrue if the distance between two
%  colors is less than the specified distance.
%
%  The format of the IsPixelWandSimilar method is:
%
%      MagickBooleanType IsPixelWandSimilar(PixelWand *p,PixelWand *q,
%        const double fuzz)
%
%  A description of each parameter follows:
%
%    o p: the pixel wand.
%
%    o q: the pixel wand.
%
%    o fuzz: any two colors that are less than or equal to this distance
%      squared are consider similar.
%
*/
WandExport MagickBooleanType IsPixelWandSimilar(PixelWand *p,PixelWand *q,
  const double fuzz)
{
  assert(p != (PixelWand *) NULL);
  assert(p->signature == WandSignature);
  if (p->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",p->name);
  assert(q != (PixelWand *) NULL);
  assert(q->signature == WandSignature);
  if (q->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",q->name);
  p->pixel.fuzz=fuzz;
  q->pixel.fuzz=fuzz;
  return(IsMagickColorSimilar(&p->pixel,&q->pixel));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P i x e l W a n d                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPixelWand() returns MagickTrue if the wand is verified as a pixel wand.
%
%  The format of the IsPixelWand method is:
%
%      MagickBooleanType IsPixelWand(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType IsPixelWand(const PixelWand *wand)
{
  if (wand == (const PixelWand *) NULL)
    return(MagickFalse);
  if (wand->signature != WandSignature)
    return(MagickFalse);
  if (LocaleNCompare(wand->name,PixelWandId,strlen(PixelWandId)) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w P i x e l W a n d                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewPixelWand() returns a new pixel wand.
%
%  The format of the NewPixelWand method is:
%
%      PixelWand *NewPixelWand(void)
%
*/
WandExport PixelWand *NewPixelWand(void)
{
  const char
    *quantum;

  PixelWand
    *wand;

  size_t
    depth;

  depth=MAGICKCORE_QUANTUM_DEPTH;
  quantum=GetMagickQuantumDepth(&depth);
  if (depth != MAGICKCORE_QUANTUM_DEPTH)
    ThrowWandFatalException(WandError,"QuantumDepthMismatch",quantum);
  wand=(PixelWand *) AcquireMagickMemory(sizeof(*wand));
  if (wand == (PixelWand *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(wand,0,sizeof(*wand));
  wand->id=AcquireWandId();
  (void) FormatLocaleString(wand->name,MaxTextExtent,"%s-%.20g",PixelWandId,
    (double) wand->id);
  wand->exception=AcquireExceptionInfo();
  GetMagickPixelPacket((Image *) NULL,&wand->pixel);
  wand->debug=IsEventLogging();
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->signature=WandSignature;
  return(wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w P i x e l W a n d s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewPixelWands() returns an array of pixel wands.
%
%  The format of the NewPixelWands method is:
%
%      PixelWand **NewPixelWands(const size_t number_wands)
%
%  A description of each parameter follows:
%
%    o number_wands: the number of wands.
%
*/
WandExport PixelWand **NewPixelWands(const size_t number_wands)
{
  register ssize_t
    i;

  PixelWand
    **wands;

  wands=(PixelWand **) AcquireQuantumMemory((size_t) number_wands,
    sizeof(*wands));
  if (wands == (PixelWand **) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  for (i=0; i < (ssize_t) number_wands; i++)
    wands[i]=NewPixelWand();
  return(wands);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l C l e a r E x c e p t i o n                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelClearException() clear any exceptions associated with the iterator.
%
%  The format of the PixelClearException method is:
%
%      MagickBooleanType PixelClearException(PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport MagickBooleanType PixelClearException(PixelWand *wand)
{
  assert(wand != (PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  ClearMagickException(wand->exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t A l p h a                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetAlpha() returns the normalized alpha color of the pixel wand.
%
%  The format of the PixelGetAlpha method is:
%
%      double PixelGetAlpha(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport double PixelGetAlpha(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((double) QuantumScale*(QuantumRange-wand->pixel.opacity));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t A l p h a Q u a n t u m                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetAlphaQuantum() returns the alpha value of the pixel wand.
%
%  The format of the PixelGetAlphaQuantum method is:
%
%      Quantum PixelGetAlphaQuantum(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport Quantum PixelGetAlphaQuantum(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(QuantumRange-ClampToQuantum(wand->pixel.opacity));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t B l a c k                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetBlack() returns the normalized black color of the pixel wand.
%
%  The format of the PixelGetBlack method is:
%
%      double PixelGetBlack(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport double PixelGetBlack(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((double) QuantumScale*wand->pixel.index);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t B l a c k Q u a n t u m                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetBlackQuantum() returns the black color of the pixel wand.
%
%  The format of the PixelGetBlackQuantum method is:
%
%      Quantum PixelGetBlackQuantum(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport Quantum PixelGetBlackQuantum(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(ClampToQuantum(wand->pixel.index));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t B l u e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetBlue() returns the normalized blue color of the pixel wand.
%
%  The format of the PixelGetBlue method is:
%
%      double PixelGetBlue(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport double PixelGetBlue(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((double) QuantumScale*wand->pixel.blue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t B l u e Q u a n t u m                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetBlueQuantum() returns the blue color of the pixel wand.
%
%  The format of the PixelGetBlueQuantum method is:
%
%      Quantum PixelGetBlueQuantum(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport Quantum PixelGetBlueQuantum(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(ClampToQuantum(wand->pixel.blue));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t C o l o r A s S t r i n g                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetColorAsString() returnsd the color of the pixel wand as a string.
%
%  The format of the PixelGetColorAsString method is:
%
%      char *PixelGetColorAsString(PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport char *PixelGetColorAsString(const PixelWand *wand)
{
  char
    *color;

  MagickPixelPacket
    pixel;

  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  pixel=wand->pixel;
  color=AcquireString((const char *) NULL);
  GetColorTuple(&pixel,MagickFalse,color);
  return(color);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t C o l o r A s N o r m a l i z e d S t r i n g             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetColorAsNormalizedString() returns the normalized color of the pixel
%  wand as a string.
%
%  The format of the PixelGetColorAsNormalizedString method is:
%
%      char *PixelGetColorAsNormalizedString(PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport char *PixelGetColorAsNormalizedString(const PixelWand *wand)
{
  char
    color[MaxTextExtent];

  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  (void) FormatLocaleString(color,MaxTextExtent,"%g,%g,%g",
    (double) (QuantumScale*wand->pixel.red),
    (double) (QuantumScale*wand->pixel.green),
    (double) (QuantumScale*wand->pixel.blue));
  if (wand->pixel.colorspace == CMYKColorspace)
    (void) FormatLocaleString(color+strlen(color),MaxTextExtent,",%g",
      (double) (QuantumScale*wand->pixel.index));
  if (wand->pixel.matte != MagickFalse)
    (void) FormatLocaleString(color+strlen(color),MaxTextExtent,",%g",
      (double) (QuantumScale*wand->pixel.opacity));
  return(ConstantString(color));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t C o l o r C o u n t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetColorCount() returns the color count associated with this color.
%
%  The format of the PixelGetColorCount method is:
%
%      size_t PixelGetColorCount(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport size_t PixelGetColorCount(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(wand->count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t C y a n                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetCyan() returns the normalized cyan color of the pixel wand.
%
%  The format of the PixelGetCyan method is:
%
%      double PixelGetCyan(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport double PixelGetCyan(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((double) QuantumScale*wand->pixel.red);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t C y a n Q u a n t u m                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetCyanQuantum() returns the cyan color of the pixel wand.
%
%  The format of the PixelGetCyanQuantum method is:
%
%      Quantum PixelGetCyanQuantum(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport Quantum PixelGetCyanQuantum(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(ClampToQuantum(wand->pixel.red));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t E x c e p t i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetException() returns the severity, reason, and description of any
%  error that occurs when using other methods in this API.
%
%  The format of the PixelGetException method is:
%
%      char *PixelGetException(const PixelWand *wand,ExceptionType *severity)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o severity: the severity of the error is returned here.
%
*/
WandExport char *PixelGetException(const PixelWand *wand,
  ExceptionType *severity)
{
  char
    *description;

  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(severity != (ExceptionType *) NULL);
  *severity=wand->exception->severity;
  description=(char *) AcquireQuantumMemory(2UL*MaxTextExtent,
    sizeof(*description));
  if (description == (char *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      wand->name);
  *description='\0';
  if (wand->exception->reason != (char *) NULL)
    (void) CopyMagickString(description,GetLocaleExceptionMessage(
      wand->exception->severity,wand->exception->reason),MaxTextExtent);
  if (wand->exception->description != (char *) NULL)
    {
      (void) ConcatenateMagickString(description," (",MaxTextExtent);
      (void) ConcatenateMagickString(description,GetLocaleExceptionMessage(
        wand->exception->severity,wand->exception->description),MaxTextExtent);
      (void) ConcatenateMagickString(description,")",MaxTextExtent);
    }
  return(description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t E x c e p t i o n T y p e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetExceptionType() the exception type associated with the wand.  If
%  no exception has occurred, UndefinedExceptionType is returned.
%
%  The format of the PixelGetExceptionType method is:
%
%      ExceptionType PixelGetExceptionType(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport ExceptionType PixelGetExceptionType(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(wand->exception->severity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t F u z z                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetFuzz() returns the normalized fuzz value of the pixel wand.
%
%  The format of the PixelGetFuzz method is:
%
%      double PixelGetFuzz(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport double PixelGetFuzz(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((double) wand->pixel.fuzz);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t G r e e n                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetGreen() returns the normalized green color of the pixel wand.
%
%  The format of the PixelGetGreen method is:
%
%      double PixelGetGreen(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport double PixelGetGreen(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((double) QuantumScale*wand->pixel.green);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t G r e e n Q u a n t u m                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetGreenQuantum() returns the green color of the pixel wand.
%
%  The format of the PixelGetGreenQuantum method is:
%
%      Quantum PixelGetGreenQuantum(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport Quantum PixelGetGreenQuantum(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(ClampToQuantum(wand->pixel.green));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t H S L                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetHSL() returns the normalized HSL color of the pixel wand.
%
%  The format of the PixelGetHSL method is:
%
%      void PixelGetHSL(const PixelWand *wand,double *hue,double *saturation,
%        double *lightness)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o hue,saturation,lightness: Return the pixel hue, saturation, and
%      brightness.
%
*/
WandExport void PixelGetHSL(const PixelWand *wand,double *hue,
  double *saturation,double *lightness)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  ConvertRGBToHSL(ClampToQuantum(wand->pixel.red),ClampToQuantum(
    wand->pixel.green),ClampToQuantum(wand->pixel.blue),hue,saturation,
    lightness);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t I n d e x                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetIndex() returns the colormap index from the pixel wand.
%
%  The format of the PixelGetIndex method is:
%
%      IndexPacket PixelGetIndex(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport IndexPacket PixelGetIndex(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((IndexPacket) wand->pixel.index);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t M a g e n t a                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetMagenta() returns the normalized magenta color of the pixel wand.
%
%  The format of the PixelGetMagenta method is:
%
%      double PixelGetMagenta(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport double PixelGetMagenta(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((double) QuantumScale*wand->pixel.green);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t M a g e n t a Q u a n t u m                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetMagentaQuantum() returns the magenta color of the pixel wand.
%
%  The format of the PixelGetMagentaQuantum method is:
%
%      Quantum PixelGetMagentaQuantum(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport Quantum PixelGetMagentaQuantum(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(ClampToQuantum(wand->pixel.green));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t M a g i c k C o l o r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetMagickColor() gets the magick color of the pixel wand.
%
%  The format of the PixelGetMagickColor method is:
%
%      void PixelGetMagickColor(PixelWand *wand,MagickPixelPacket *color)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o color:  The pixel wand color is returned here.
%
*/
WandExport void PixelGetMagickColor(const PixelWand *wand,
  MagickPixelPacket *color)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(color != (MagickPixelPacket *) NULL);
  *color=wand->pixel;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t O p a c i t y                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetOpacity() returns the normalized opacity color of the pixel wand.
%
%  The format of the PixelGetOpacity method is:
%
%      double PixelGetOpacity(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport double PixelGetOpacity(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((double) QuantumScale*wand->pixel.opacity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t O p a c i t y Q u a n t u m                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetOpacityQuantum() returns the opacity color of the pixel wand.
%
%  The format of the PixelGetOpacityQuantum method is:
%
%      Quantum PixelGetOpacityQuantum(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport Quantum PixelGetOpacityQuantum(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(ClampToQuantum(wand->pixel.opacity));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t Q u a n t u m C o l o r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetQuantumColor() gets the color of the pixel wand as a PixelPacket.
%
%  The format of the PixelGetQuantumColor method is:
%
%      void PixelGetQuantumColor(PixelWand *wand,PixelPacket *color)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o color:  The pixel wand color is returned here.
%
*/
WandExport void PixelGetQuantumColor(const PixelWand *wand,PixelPacket *color)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(color != (PixelPacket *) NULL);
  color->opacity=ClampToQuantum(wand->pixel.opacity);
  if (wand->pixel.colorspace == CMYKColorspace)
    {
      color->red=ClampToQuantum((MagickRealType) QuantumRange-
        (wand->pixel.red*(QuantumRange-wand->pixel.index)+wand->pixel.index));
      color->green=ClampToQuantum((MagickRealType) QuantumRange-
        (wand->pixel.green*(QuantumRange-wand->pixel.index)+
        wand->pixel.index));
      color->blue=ClampToQuantum((MagickRealType) QuantumRange-
        (wand->pixel.blue*(QuantumRange-wand->pixel.index)+wand->pixel.index));
      return;
    }
  color->red=ClampToQuantum(wand->pixel.red);
  color->green=ClampToQuantum(wand->pixel.green);
  color->blue=ClampToQuantum(wand->pixel.blue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t R e d                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetRed() returns the normalized red color of the pixel wand.
%
%  The format of the PixelGetRed method is:
%
%      double PixelGetRed(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport double PixelGetRed(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((double) QuantumScale*wand->pixel.red);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t R e d Q u a n t u m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetRedQuantum() returns the red color of the pixel wand.
%
%  The format of the PixelGetRedQuantum method is:
%
%      Quantum PixelGetRedQuantum(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport Quantum PixelGetRedQuantum(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(ClampToQuantum(wand->pixel.red));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t Y e l l o w                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetYellow() returns the normalized yellow color of the pixel wand.
%
%  The format of the PixelGetYellow method is:
%
%      double PixelGetYellow(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport double PixelGetYellow(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return((double) QuantumScale*wand->pixel.blue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t Y e l l o w Q u a n t u m                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetYellowQuantum() returns the yellow color of the pixel wand.
%
%  The format of the PixelGetYellowQuantum method is:
%
%      Quantum PixelGetYellowQuantum(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport Quantum PixelGetYellowQuantum(const PixelWand *wand)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(ClampToQuantum(wand->pixel.blue));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t A l p h a                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetAlpha() sets the normalized alpha color of the pixel wand.
%
%  The format of the PixelSetAlpha method is:
%
%      void PixelSetAlpha(PixelWand *wand,const double alpha)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o alpha: the level of transparency: 1.0 is fully opaque and 0.0 is fully
%      transparent.
%
*/
WandExport void PixelSetAlpha(PixelWand *wand,const double alpha)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.opacity=(MagickRealType) (QuantumRange-
    ClampToQuantum((MagickRealType) QuantumRange*alpha));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t A l p h a Q u a n t u m                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetAlphaQuantum() sets the alpha color of the pixel wand.
%
%  The format of the PixelSetAlphaQuantum method is:
%
%      void PixelSetAlphaQuantum(PixelWand *wand,
%        const Quantum opacity)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o opacity: the opacity color.
%
*/
WandExport void PixelSetAlphaQuantum(PixelWand *wand,const Quantum opacity)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.opacity=(MagickRealType) (QuantumRange-opacity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t B l a c k                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetBlack() sets the normalized black color of the pixel wand.
%
%  The format of the PixelSetBlack method is:
%
%      void PixelSetBlack(PixelWand *wand,const double black)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o black: the black color.
%
*/
WandExport void PixelSetBlack(PixelWand *wand,const double black)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.index=(MagickRealType) ClampToQuantum((MagickRealType)
    QuantumRange*black);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t B l a c k Q u a n t u m                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetBlackQuantum() sets the black color of the pixel wand.
%
%  The format of the PixelSetBlackQuantum method is:
%
%      void PixelSetBlackQuantum(PixelWand *wand,const Quantum black)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o black: the black color.
%
*/
WandExport void PixelSetBlackQuantum(PixelWand *wand,const Quantum black)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.index=(MagickRealType) black;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t B l u e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetBlue() sets the normalized blue color of the pixel wand.
%
%  The format of the PixelSetBlue method is:
%
%      void PixelSetBlue(PixelWand *wand,const double blue)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o blue: the blue color.
%
*/
WandExport void PixelSetBlue(PixelWand *wand,const double blue)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.blue=(MagickRealType) ClampToQuantum((MagickRealType)
    QuantumRange*blue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t B l u e Q u a n t u m                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetBlueQuantum() sets the blue color of the pixel wand.
%
%  The format of the PixelSetBlueQuantum method is:
%
%      void PixelSetBlueQuantum(PixelWand *wand,const Quantum blue)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o blue: the blue color.
%
*/
WandExport void PixelSetBlueQuantum(PixelWand *wand,const Quantum blue)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.blue=(MagickRealType) blue;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t C o l o r                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetColor() sets the color of the pixel wand with a string (e.g.
%  "blue", "#0000ff", "rgb(0,0,255)", "cmyk(100,100,100,10)", etc.).
%
%  The format of the PixelSetColor method is:
%
%      MagickBooleanType PixelSetColor(PixelWand *wand,const char *color)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o color: the pixel wand color.
%
*/
WandExport MagickBooleanType PixelSetColor(PixelWand *wand,const char *color)
{
  MagickBooleanType
    status;

  MagickPixelPacket
    pixel;

  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  status=QueryMagickColor(color,&pixel,wand->exception);
  if (status != MagickFalse)
    wand->pixel=pixel;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t C o l o r C o u n t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetColorCount() sets the color count of the pixel wand.
%
%  The format of the PixelSetColorCount method is:
%
%      void PixelSetColorCount(PixelWand *wand,const size_t count)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o count: the number of this particular color.
%
*/
WandExport void PixelSetColorCount(PixelWand *wand,const size_t count)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->count=count;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t C o l o r F r o m W a n d                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetColorFromWand() sets the color of the pixel wand.
%
%  The format of the PixelSetColorFromWand method is:
%
%      void PixelSetColorFromWand(PixelWand *wand,const PixelWand *color)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o color: set the pixel wand color here.
%
*/
WandExport void PixelSetColorFromWand(PixelWand *wand,const PixelWand *color)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(color != (const PixelWand *) NULL);
  wand->pixel=color->pixel;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t C y a n                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetCyan() sets the normalized cyan color of the pixel wand.
%
%  The format of the PixelSetCyan method is:
%
%      void PixelSetCyan(PixelWand *wand,const double cyan)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o cyan: the cyan color.
%
*/
WandExport void PixelSetCyan(PixelWand *wand,const double cyan)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.red=(MagickRealType) ClampToQuantum((MagickRealType)
    QuantumRange*cyan);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t C y a n Q u a n t u m                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetCyanQuantum() sets the cyan color of the pixel wand.
%
%  The format of the PixelSetCyanQuantum method is:
%
%      void PixelSetCyanQuantum(PixelWand *wand,const Quantum cyan)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o cyan: the cyan color.
%
*/
WandExport void PixelSetCyanQuantum(PixelWand *wand,const Quantum cyan)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.red=(MagickRealType) cyan;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t F u z z                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetFuzz() sets the fuzz value of the pixel wand.
%
%  The format of the PixelSetFuzz method is:
%
%      void PixelSetFuzz(PixelWand *wand,const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o fuzz: the fuzz value.
%
*/
WandExport void PixelSetFuzz(PixelWand *wand,const double fuzz)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.fuzz=(MagickRealType) fuzz;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t G r e e n                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetGreen() sets the normalized green color of the pixel wand.
%
%  The format of the PixelSetGreen method is:
%
%      void PixelSetGreen(PixelWand *wand,const double green)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o green: the green color.
%
*/
WandExport void PixelSetGreen(PixelWand *wand,const double green)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.green=(MagickRealType) ClampToQuantum((MagickRealType)
    QuantumRange*green);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t G r e e n Q u a n t u m                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetGreenQuantum() sets the green color of the pixel wand.
%
%  The format of the PixelSetGreenQuantum method is:
%
%      void PixelSetGreenQuantum(PixelWand *wand,const Quantum green)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o green: the green color.
%
*/
WandExport void PixelSetGreenQuantum(PixelWand *wand,const Quantum green)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.green=(MagickRealType) green;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t H S L                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetHSL() sets the normalized HSL color of the pixel wand.
%
%  The format of the PixelSetHSL method is:
%
%      void PixelSetHSL(PixelWand *wand,const double hue,
%        const double saturation,const double lightness)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o hue,saturation,lightness: Return the pixel hue, saturation, and
%      brightness.
%
*/
WandExport void PixelSetHSL(PixelWand *wand,const double hue,
  const double saturation,const double lightness)
{
  Quantum
    blue,
    green,
    red;

  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  ConvertHSLToRGB(hue,saturation,lightness,&red,&green,&blue);
  wand->pixel.red=(MagickRealType) red;
  wand->pixel.green=(MagickRealType) green;
  wand->pixel.blue=(MagickRealType) blue;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t I n d e x                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetIndex() sets the colormap index of the pixel wand.
%
%  The format of the PixelSetIndex method is:
%
%      void PixelSetIndex(PixelWand *wand,const IndexPacket index)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o index: the colormap index.
%
*/
WandExport void PixelSetIndex(PixelWand *wand,const IndexPacket index)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.index=(MagickRealType) index;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t M a g e n t a                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetMagenta() sets the normalized magenta color of the pixel wand.
%
%  The format of the PixelSetMagenta method is:
%
%      void PixelSetMagenta(PixelWand *wand,const double magenta)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o magenta: the magenta color.
%
*/
WandExport void PixelSetMagenta(PixelWand *wand,const double magenta)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.green=(MagickRealType) ClampToQuantum((MagickRealType)
    QuantumRange*magenta);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t M a g e n t a Q u a n t u m                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetMagentaQuantum() sets the magenta color of the pixel wand.
%
%  The format of the PixelSetMagentaQuantum method is:
%
%      void PixelSetMagentaQuantum(PixelWand *wand,
%        const Quantum magenta)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o magenta: the green magenta.
%
*/
WandExport void PixelSetMagentaQuantum(PixelWand *wand,const Quantum magenta)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.green=(MagickRealType) magenta;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t M a g i c k C o l o r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetMagickColor() sets the color of the pixel wand.
%
%  The format of the PixelSetMagickColor method is:
%
%      void PixelSetMagickColor(PixelWand *wand,const MagickPixelPacket *color)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o color: the pixel wand color.
%
*/
WandExport void PixelSetMagickColor(PixelWand *wand,
  const MagickPixelPacket *color)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(color != (const MagickPixelPacket *) NULL);
  wand->pixel=(*color);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t O p a c i t y                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetOpacity() sets the normalized opacity color of the pixel wand.
%
%  The format of the PixelSetOpacity method is:
%
%      void PixelSetOpacity(PixelWand *wand,const double opacity)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o opacity: the opacity color.
%
*/
WandExport void PixelSetOpacity(PixelWand *wand,const double opacity)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.matte=MagickTrue;
  wand->pixel.opacity=(MagickRealType) ClampToQuantum((MagickRealType)
    QuantumRange*opacity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t O p a c i t y Q u a n t u m                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetOpacityQuantum() sets the opacity color of the pixel wand.
%
%  The format of the PixelSetOpacityQuantum method is:
%
%      void PixelSetOpacityQuantum(PixelWand *wand,
%        const Quantum opacity)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o opacity: the opacity color.
%
*/
WandExport void PixelSetOpacityQuantum(PixelWand *wand,const Quantum opacity)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.opacity=(MagickRealType) opacity;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t Q u a n t u m C o l o r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetQuantumColor() sets the color of the pixel wand.
%
%  The format of the PixelSetQuantumColor method is:
%
%      void PixelSetQuantumColor(PixelWand *wand,const PixelPacket *color)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o color: the pixel wand color.
%
*/
WandExport void PixelSetQuantumColor(PixelWand *wand,const PixelPacket *color)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(color != (PixelPacket *) NULL);
  wand->pixel.red=(MagickRealType) color->red;
  wand->pixel.green=(MagickRealType) color->green;
  wand->pixel.blue=(MagickRealType) color->blue;
  wand->pixel.opacity=(MagickRealType) color->opacity;
  wand->pixel.matte=color->opacity != OpaqueOpacity ? MagickTrue : MagickFalse;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t R e d                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetRed() sets the normalized red color of the pixel wand.
%
%  The format of the PixelSetRed method is:
%
%      void PixelSetRed(PixelWand *wand,const double red)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o red: the red color.
%
*/
WandExport void PixelSetRed(PixelWand *wand,const double red)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.red=(MagickRealType) ClampToQuantum((MagickRealType)
    QuantumRange*red);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t R e d Q u a n t u m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetRedQuantum() sets the red color of the pixel wand.
%
%  The format of the PixelSetRedQuantum method is:
%
%      void PixelSetRedQuantum(PixelWand *wand,const Quantum red)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o red: the red color.
%
*/
WandExport void PixelSetRedQuantum(PixelWand *wand,const Quantum red)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.red=(MagickRealType) red;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t Y e l l o w                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetYellow() sets the normalized yellow color of the pixel wand.
%
%  The format of the PixelSetYellow method is:
%
%      void PixelSetYellow(PixelWand *wand,const double yellow)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o yellow: the yellow color.
%
*/
WandExport void PixelSetYellow(PixelWand *wand,const double yellow)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.blue=(MagickRealType) ClampToQuantum((MagickRealType)
    QuantumRange*yellow);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t Y e l l o w Q u a n t u m                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetYellowQuantum() sets the yellow color of the pixel wand.
%
%  The format of the PixelSetYellowQuantum method is:
%
%      void PixelSetYellowQuantum(PixelWand *wand,const Quantum yellow)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
%    o yellow: the yellow color.
%
*/
WandExport void PixelSetYellowQuantum(PixelWand *wand,const Quantum yellow)
{
  assert(wand != (const PixelWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->pixel.blue=(MagickRealType) yellow;
}

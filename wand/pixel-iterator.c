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
%            IIIII  TTTTT EEEEE  RRRR    AAA   TTTTT   OOO   RRRR             %
%              I      T   E      R   R  A   A    T    O   O  R   R            %
%              I      T   EEE    RRRR   AAAAA    T    O   O  RRRR             %
%              I      T   E      R R    A   A    T    O   O  R R              %
%            IIIII    T   EEEEE  R  R   A   A    T     OOO   R  R             %
%                                                                             %
%                                                                             %
%                   ImageMagick Image Pixel Iterator Methods                  %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                March 2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wand/pixel-iterator.h"
#include "wand/pixel-wand.h"
#include "wand/wand.h"

/*
  Define declarations.
*/
#define PixelIteratorId  "PixelIterator"

/*
  Typedef declarations.
*/
struct _PixelIterator
{
  unsigned long
    id;

  char
    name[MaxTextExtent];

  ExceptionInfo
    *exception;

  CacheView
    *view;

  RectangleInfo
    region;

  MagickBooleanType
    active;

  long
    y;

  PixelWand
    **pixel_wands;

  MagickBooleanType
    debug;

  unsigned long
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l e a r P i x e l I t e r a t o r                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClearPixelIterator() clear resources associated with a PixelIterator.
%
%  The format of the ClearPixelIterator method is:
%
%      PixelIterator *ClearPixelIterator(PixelIterator *iterator)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
*/
WandExport void ClearPixelIterator(PixelIterator *iterator)
{
  assert(iterator != (const PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  iterator->pixel_wands=DestroyPixelWands(iterator->pixel_wands,
    iterator->region.width);
  ClearMagickException(iterator->exception);
  iterator->pixel_wands=NewPixelWands(iterator->region.width);
  iterator->active=MagickFalse;
  iterator->y=0;
  iterator->debug=IsEventLogging();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e P i x e l I t e r a t o r                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClonePixelIterator() makes an exact copy of the specified iterator.
%
%  The format of the ClonePixelIterator method is:
%
%      PixelIterator *ClonePixelIterator(const PixelIterator *iterator)
%
%  A description of each parameter follows:
%
%    o iterator: the magick iterator.
%
*/
WandExport PixelIterator *ClonePixelIterator(const PixelIterator *iterator)
{
  PixelIterator
    *clone_iterator;

  assert(iterator != (PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  clone_iterator=(PixelIterator *) AcquireMagickMemory(sizeof(*clone_iterator));
  if (clone_iterator == (PixelIterator *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      iterator->name);
  (void) ResetMagickMemory(clone_iterator,0,sizeof(*clone_iterator));
  clone_iterator->id=AcquireWandId();
  (void) FormatMagickString(clone_iterator->name,MaxTextExtent,"%s-%lu",
    PixelIteratorId,clone_iterator->id);
  clone_iterator->exception=AcquireExceptionInfo();
  InheritException(clone_iterator->exception,iterator->exception);
  clone_iterator->view=CloneCacheView(iterator->view);
  clone_iterator->region=iterator->region;
  clone_iterator->active=iterator->active;
  clone_iterator->y=iterator->y;
  clone_iterator->pixel_wands=ClonePixelWands((const PixelWand **)
    iterator->pixel_wands,iterator->region.width);
  clone_iterator->debug=iterator->debug;
  if (clone_iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",
      clone_iterator->name);
  clone_iterator->signature=WandSignature;
  return(clone_iterator);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y P i x e l I t e r a t o r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyPixelIterator() deallocates resources associated with a PixelIterator.
%
%  The format of the DestroyPixelIterator method is:
%
%      PixelIterator *DestroyPixelIterator(PixelIterator *iterator)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
*/
WandExport PixelIterator *DestroyPixelIterator(PixelIterator *iterator)
{
  assert(iterator != (const PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  iterator->view=DestroyCacheView(iterator->view);
  iterator->pixel_wands=DestroyPixelWands(iterator->pixel_wands,
    iterator->region.width);
  iterator->exception=DestroyExceptionInfo(iterator->exception);
  iterator->signature=(~WandSignature);
  RelinquishWandId(iterator->id);
  iterator=(PixelIterator *) RelinquishMagickMemory(iterator);
  return(iterator);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P i x e l I t e r a t o r                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPixelIterator() returns MagickTrue if the iterator is verified as a pixel
%  iterator.
%
%  The format of the IsPixelIterator method is:
%
%      MagickBooleanType IsPixelIterator(const PixelIterator *iterator)
%
%  A description of each parameter follows:
%
%    o iterator: the magick iterator.
%
*/
WandExport MagickBooleanType IsPixelIterator(const PixelIterator *iterator)
{
  size_t
    length;

  if (iterator == (const PixelIterator *) NULL)
    return(MagickFalse);
  if (iterator->signature != WandSignature)
    return(MagickFalse);
  length=strlen(PixelIteratorId);
  if (LocaleNCompare(iterator->name,PixelIteratorId,length) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w P i x e l I t e r a t o r                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewPixelIterator() returns a new pixel iterator.
%
%  The format of the NewPixelIterator method is:
%
%      PixelIterator NewPixelIterator(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport PixelIterator *NewPixelIterator(MagickWand *wand)
{
  const char
    *quantum;

  Image
    *image;

  PixelIterator
    *iterator;

  unsigned long
    depth;

  CacheView
    *view;

  depth=MAGICKCORE_QUANTUM_DEPTH;
  quantum=GetMagickQuantumDepth(&depth);
  if (depth != MAGICKCORE_QUANTUM_DEPTH)
    ThrowWandFatalException(WandError,"QuantumDepthMismatch",quantum);
  assert(wand != (MagickWand *) NULL);
  image=GetImageFromMagickWand(wand);
  if (image == (Image *) NULL)
    return((PixelIterator *) NULL);
  view=AcquireCacheView(image);
  if (view == (CacheView *) NULL)
    return((PixelIterator *) NULL);
  iterator=(PixelIterator *) AcquireMagickMemory(sizeof(*iterator));
  if (iterator == (PixelIterator *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(iterator,0,sizeof(*iterator));
  iterator->id=AcquireWandId();
  (void) FormatMagickString(iterator->name,MaxTextExtent,"%s-%lu",
    PixelIteratorId,iterator->id);
  iterator->exception=AcquireExceptionInfo();
  iterator->view=view;
  SetGeometry(image,&iterator->region);
  iterator->region.width=image->columns;
  iterator->region.height=image->rows;
  iterator->region.x=0;
  iterator->region.y=0;
  iterator->pixel_wands=NewPixelWands(iterator->region.width);
  iterator->y=0;
  iterator->debug=IsEventLogging();
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  iterator->signature=WandSignature;
  return(iterator);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l C l e a r I t e r a t o r E x c e p t i o n                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelClearIteratorException() clear any exceptions associated with the
%  iterator.
%
%  The format of the PixelClearIteratorException method is:
%
%      MagickBooleanType PixelClearIteratorException(PixelIterator *wand)
%
%  A description of each parameter follows:
%
%    o wand: the pixel wand.
%
*/
WandExport MagickBooleanType PixelClearIteratorException(
  PixelIterator *iterator)
{
  assert(iterator != (PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  ClearMagickException(iterator->exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w P i x e l R e g i o n I t e r a t o r                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewPixelRegionIterator() returns a new pixel iterator.
%
%  The format of the NewPixelRegionIterator method is:
%
%      PixelIterator NewPixelRegionIterator(MagickWand *wand,const long x,
%        const long y,const unsigned long width,const unsigned long height)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
WandExport PixelIterator *NewPixelRegionIterator(MagickWand *wand,const long x,
  const long y,const unsigned long width,const unsigned long height)
{
  const char
    *quantum;

  Image
    *image;

  PixelIterator
    *iterator;

  unsigned long
    depth;

  CacheView
    *view;

  assert(wand != (MagickWand *) NULL);
  depth=MAGICKCORE_QUANTUM_DEPTH;
  quantum=GetMagickQuantumDepth(&depth);
  if (depth != MAGICKCORE_QUANTUM_DEPTH)
    ThrowWandFatalException(WandError,"QuantumDepthMismatch",quantum);
  if ((width == 0) || (width == 0))
    ThrowWandFatalException(WandError,"ZeroRegionSize",quantum);
  image=GetImageFromMagickWand(wand);
  if (image == (Image *) NULL)
    return((PixelIterator *) NULL);
  view=AcquireCacheView(image);
  if (view == (CacheView *) NULL)
    return((PixelIterator *) NULL);
  iterator=(PixelIterator *) AcquireMagickMemory(sizeof(*iterator));
  if (iterator == (PixelIterator *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      wand->name);
  (void) ResetMagickMemory(iterator,0,sizeof(*iterator));
  iterator->id=AcquireWandId();
  (void) FormatMagickString(iterator->name,MaxTextExtent,"%s-%lu",
    PixelIteratorId,iterator->id);
  iterator->exception=AcquireExceptionInfo();
  iterator->view=view;
  SetGeometry(image,&iterator->region);
  iterator->region.width=width;
  iterator->region.height=height;
  iterator->region.x=x;
  iterator->region.y=y;
  iterator->pixel_wands=NewPixelWands(iterator->region.width);
  iterator->y=0;
  iterator->debug=IsEventLogging();
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  iterator->signature=WandSignature;
  return(iterator);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t C u r r e n t I t e r a t o r R o w                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetCurrentIteratorRow() returns the current row as an array of pixel
%  wands from the pixel iterator.
%
%  The format of the PixelGetCurrentIteratorRow method is:
%
%      PixelWand **PixelGetCurrentIteratorRow(PixelIterator *iterator,
%        unsigned long *number_wands)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
%    o number_wands: the number of pixel wands.
%
*/
WandExport PixelWand **PixelGetCurrentIteratorRow(PixelIterator *iterator,
  unsigned long *number_wands)
{
  register const IndexPacket
    *indexes;

  register const PixelPacket
    *pixels;

  register long
    x;

  assert(iterator != (PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  *number_wands=0;
  iterator->active=MagickTrue;
  pixels=GetCacheViewVirtualPixels(iterator->view,iterator->region.x,
    iterator->region.y+iterator->y,iterator->region.width,1,
    iterator->exception);
  if (pixels == (const PixelPacket *) NULL)
    {
      InheritException(iterator->exception,GetCacheViewException(
        iterator->view));
      return((PixelWand **) NULL);
    }
  indexes=GetCacheViewVirtualIndexQueue(iterator->view);
  for (x=0; x < (long) iterator->region.width; x++)
    PixelSetQuantumColor(iterator->pixel_wands[x],pixels+x);
  if (GetCacheViewColorspace(iterator->view) == CMYKColorspace)
    for (x=0; x < (long) iterator->region.width; x++)
      PixelSetBlackQuantum(iterator->pixel_wands[x],indexes[x]);
  if (GetCacheViewStorageClass(iterator->view) == PseudoClass)
    for (x=0; x < (long) iterator->region.width; x++)
      PixelSetIndex(iterator->pixel_wands[x],indexes[x]);
  *number_wands=iterator->region.width;
  return(iterator->pixel_wands);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t I t e r a t o r E x c e p t i o n                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetIteratorException() returns the severity, reason, and description of
%  any error that occurs when using other methods in this API.
%
%  The format of the PixelGetIteratorException method is:
%
%      char *PixelGetIteratorException(const Pixeliterator *iterator,
%        ExceptionType *severity)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
%    o severity: the severity of the error is returned here.
%
*/
WandExport char *PixelGetIteratorException(const PixelIterator *iterator,
  ExceptionType *severity)
{
  char
    *description;

  assert(iterator != (const PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  assert(severity != (ExceptionType *) NULL);
  *severity=iterator->exception->severity;
  description=(char *) AcquireQuantumMemory(2UL*MaxTextExtent,
    sizeof(*description));
  if (description == (char *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      iterator->name);
  *description='\0';
  if (iterator->exception->reason != (char *) NULL)
    (void) CopyMagickString(description,GetLocaleExceptionMessage(
      iterator->exception->severity,iterator->exception->reason),MaxTextExtent);
  if (iterator->exception->description != (char *) NULL)
    {
      (void) ConcatenateMagickString(description," (",MaxTextExtent);
      (void) ConcatenateMagickString(description,GetLocaleExceptionMessage(
        iterator->exception->severity,iterator->exception->description),
        MaxTextExtent);
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
%  PixelGetIteratorExceptionType() the exception type associated with the wand.
%  If no exception has occurred, UndefinedExceptionType is returned.
%
%  The format of the PixelGetIteratorExceptionType method is:
%
%      ExceptionType PixelGetIteratorExceptionType(const PixelWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport ExceptionType PixelGetIteratorExceptionType(
  const PixelIterator *wand)
{
  assert(wand != (const PixelIterator *) NULL);
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
%   P i x e l G e t I t e r a t o r R o w                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetIteratorRow() returns the current pixel iterator row.
%
%  The format of the PixelGetIteratorRow method is:
%
%      MagickBooleanType PixelGetIteratorRow(PixelIterator *iterator)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
*/
WandExport long PixelGetIteratorRow(PixelIterator *iterator)
{
  assert(iterator != (const PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  return(iterator->y);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t N e x t I t e r a t o r R o w                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetNextIteratorRow() returns the next row as an array of pixel wands
%  from the pixel iterator.
%
%  The format of the PixelGetNextIteratorRow method is:
%
%      PixelWand **PixelGetNextIteratorRow(PixelIterator *iterator,
%        unsigned long *number_wands)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
%    o number_wands: the number of pixel wands.
%
*/
WandExport PixelWand **PixelGetNextIteratorRow(PixelIterator *iterator,
  unsigned long *number_wands)
{
  register const IndexPacket
    *indexes;

  register const PixelPacket
    *pixels;

  register long
    x;

  assert(iterator != (PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  *number_wands=0;
  if (iterator->active != MagickFalse)
    iterator->y++;
  if (PixelSetIteratorRow(iterator,iterator->y) == MagickFalse)
    return((PixelWand **) NULL);
  pixels=GetCacheViewVirtualPixels(iterator->view,iterator->region.x,
    iterator->region.y+iterator->y,iterator->region.width,1,
    iterator->exception);
  if (pixels == (const PixelPacket *) NULL)
    {
      InheritException(iterator->exception,GetCacheViewException(
        iterator->view));
      return((PixelWand **) NULL);
    }
  indexes=GetCacheViewVirtualIndexQueue(iterator->view);
  for (x=0; x < (long) iterator->region.width; x++)
    PixelSetQuantumColor(iterator->pixel_wands[x],pixels+x);
  if (GetCacheViewColorspace(iterator->view) == CMYKColorspace)
    for (x=0; x < (long) iterator->region.width; x++)
      PixelSetBlackQuantum(iterator->pixel_wands[x],indexes[x]);
  if (GetCacheViewStorageClass(iterator->view) == PseudoClass)
    for (x=0; x < (long) iterator->region.width; x++)
        PixelSetIndex(iterator->pixel_wands[x],indexes[x]);
  *number_wands=iterator->region.width;
  return(iterator->pixel_wands);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t P r e v i o u s I t e r a t o r R o w                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetPreviousIteratorRow() returns the previous row as an array of pixel
%  wands from the pixel iterator.
%
%  The format of the PixelGetPreviousIteratorRow method is:
%
%      PixelWand **PixelGetPreviousIteratorRow(PixelIterator *iterator,
%        unsigned long *number_wands)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
%    o number_wands: the number of pixel wands.
%
*/

WandExport PixelWand **PixelGetPreviousRow(PixelIterator *iterator)
{
  unsigned long
    number_wands;

  return(PixelGetPreviousIteratorRow(iterator,&number_wands));
}

WandExport PixelWand **PixelGetPreviousIteratorRow(PixelIterator *iterator,
  unsigned long *number_wands)
{
  register const IndexPacket
    *indexes;

  register const PixelPacket
    *pixels;

  register long
    x;

  assert(iterator != (PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  *number_wands=0;
  if (iterator->active != MagickFalse)
    iterator->y--;
  if (PixelSetIteratorRow(iterator,iterator->y) == MagickFalse)
    return((PixelWand **) NULL);
  pixels=GetCacheViewVirtualPixels(iterator->view,iterator->region.x,
    iterator->region.y+iterator->y,iterator->region.width,1,
    iterator->exception);
  if (pixels == (const PixelPacket *) NULL)
    {
      InheritException(iterator->exception,GetCacheViewException(
        iterator->view));
      return((PixelWand **) NULL);
    }
  indexes=GetCacheViewVirtualIndexQueue(iterator->view);
  for (x=0; x < (long) iterator->region.width; x++)
    PixelSetQuantumColor(iterator->pixel_wands[x],pixels+x);
  if (GetCacheViewColorspace(iterator->view) == CMYKColorspace)
    for (x=0; x < (long) iterator->region.width; x++)
      PixelSetBlackQuantum(iterator->pixel_wands[x],indexes[x]);
  if (GetCacheViewStorageClass(iterator->view) == PseudoClass)
    for (x=0; x < (long) iterator->region.width; x++)
      PixelSetIndex(iterator->pixel_wands[x],indexes[x]);
  *number_wands=iterator->region.width;
  return(iterator->pixel_wands);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l R e s e t I t e r a t o r                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelResetIterator() resets the pixel iterator.  Use it in conjunction
%  with PixelGetNextIteratorRow() to iterate over all the pixels in a pixel
%  container.
%
%  The format of the PixelResetIterator method is:
%
%      void PixelResetIterator(PixelIterator *iterator)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
*/
WandExport void PixelResetIterator(PixelIterator *iterator)
{
  assert(iterator != (PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  iterator->active=MagickFalse;
  iterator->y=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t F i r s t I t e r a t o r R o w                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetFirstIteratorRow() sets the pixel iterator to the first pixel row.
%
%  The format of the PixelSetFirstIteratorRow method is:
%
%      void PixelSetFirstIteratorRow(PixelIterator *iterator)
%
%  A description of each parameter follows:
%
%    o iterator: the magick iterator.
%
*/
WandExport void PixelSetFirstIteratorRow(PixelIterator *iterator)
{
  assert(iterator != (PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  iterator->active=MagickFalse;
  iterator->y=iterator->region.y;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t I t e r a t o r R o w                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetIteratorRow() set the pixel iterator row.
%
%  The format of the PixelSetIteratorRow method is:
%
%      MagickBooleanType PixelSetIteratorRow(PixelIterator *iterator,
%        const long row)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
*/
WandExport MagickBooleanType PixelSetIteratorRow(PixelIterator *iterator,
  const long row)
{
  assert(iterator != (const PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  if ((row < 0) || (row >= (long) iterator->region.height))
    return(MagickFalse);
  iterator->active=MagickTrue;
  iterator->y=row;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S e t L a s t I t e r a t o r R o w                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSetLastIteratorRow() sets the pixel iterator to the last pixel row.
%
%  The format of the PixelSetLastIteratorRow method is:
%
%      void PixelSetLastIteratorRow(PixelIterator *iterator)
%
%  A description of each parameter follows:
%
%    o iterator: the magick iterator.
%
*/
WandExport void PixelSetLastIteratorRow(PixelIterator *iterator)
{
  assert(iterator != (PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  iterator->active=MagickFalse;
  iterator->y=(long) iterator->region.height-1;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l S y n c I t e r a t o r                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelSyncIterator() syncs the pixel iterator.
%
%  The format of the PixelSyncIterator method is:
%
%      MagickBooleanType PixelSyncIterator(PixelIterator *iterator)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
*/
WandExport MagickBooleanType PixelSyncIterator(PixelIterator *iterator)
{
  ExceptionInfo
    *exception;

  register IndexPacket
    *__restrict indexes;

  register long
    x;

  register PixelPacket
    *__restrict pixels;

  assert(iterator != (const PixelIterator *) NULL);
  assert(iterator->signature == WandSignature);
  if (iterator->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",iterator->name);
  if (SetCacheViewStorageClass(iterator->view,DirectClass) == MagickFalse)
    return(MagickFalse);
  exception=iterator->exception;
  pixels=GetCacheViewAuthenticPixels(iterator->view,iterator->region.x,
    iterator->region.y+iterator->y,iterator->region.width,1,exception);
  if (pixels == (PixelPacket *) NULL)
    {
      InheritException(iterator->exception,GetCacheViewException(
        iterator->view));
      return(MagickFalse);
    }
  indexes=GetCacheViewAuthenticIndexQueue(iterator->view);
  for (x=0; x < (long) iterator->region.width; x++)
    PixelGetQuantumColor(iterator->pixel_wands[x],pixels+x);
  if (GetCacheViewColorspace(iterator->view) == CMYKColorspace)
    for (x=0; x < (long) iterator->region.width; x++)
      indexes[x]=PixelGetBlackQuantum(iterator->pixel_wands[x]);
  if (SyncCacheViewAuthenticPixels(iterator->view,exception) == MagickFalse)
    {
      InheritException(iterator->exception,GetCacheViewException(
        iterator->view));
      return(MagickFalse);
    }
  return(MagickTrue);
}

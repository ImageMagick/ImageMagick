/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        DDDD   EEEEE  PPPP   RRRR   EEEEE   CCCC   AAA   TTTTT  EEEEE        %
%        D   D  E      P   P  R   R  E      C      A   A    T    E            %
%        D   D  EEE    PPPPP  RRRR   EEE    C      AAAAA    T    EEE          %
%        D   D  E      P      R R    E      C      A   A    T    E            %
%        DDDD   EEEEE  P      R  R   EEEEE   CCCC  A   A    T    EEEEE        %
%                                                                             %
%                                                                             %
%                        MagickCore Deprecated Methods                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                October 2002                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
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
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#endif
#include "MagickCore/studio.h"
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/draw-private.h"
#include "MagickCore/effect.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/fx.h"
#include "MagickCore/geometry.h"
#include "MagickCore/identify.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/magick.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/morphology.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantize.h"
#include "MagickCore/random_.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/segment.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/threshold.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k S e e k a b l e S t r e a m                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickSeekableStream() returns MagickTrue if the magick supports a
%  seekable stream.
%
%  The format of the GetMagickSeekableStream method is:
%
%      MagickBooleanType GetMagickSeekableStream(const MagickInfo *magick_info)
%
%  A description of each parameter follows:
%
%    o magick_info:  The magick info.
%
*/
MagickExport MagickBooleanType GetMagickSeekableStream(
  const MagickInfo *magick_info)
{
  assert(magick_info != (MagickInfo *) NULL);
  assert(magick_info->signature == MagickCoreSignature);
  return(((magick_info->flags & CoderSeekableStreamFlag) == 0) ? MagickFalse :
    MagickTrue);
}

#if defined(MAGICKCORE_WINDOWS_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C r o p I m a g e T o H B i t m a p                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CropImageToHBITMAP() extracts a specified region of the image and returns
%  it as a Windows HBITMAP. While the same functionality can be accomplished by
%  invoking CropImage() followed by ImageToHBITMAP(), this method is more
%  efficient since it copies pixels directly to the HBITMAP.
%
%  The format of the CropImageToHBITMAP method is:
%
%      HBITMAP CropImageToHBITMAP(Image* image,const RectangleInfo *geometry,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o geometry: Define the region of the image to crop with members
%      x, y, width, and height.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void *CropImageToHBITMAP(Image *image,
  const RectangleInfo *geometry,ExceptionInfo *exception)
{
#define CropImageTag  "Crop/Image"

  BITMAP
    bitmap;

  HBITMAP
    bitmapH;

  HANDLE
    bitmap_bitsH;

  MagickBooleanType
    proceed;

  RectangleInfo
    page;

  register const Quantum
    *p;

  register RGBQUAD
    *q;

  RGBQUAD
    *bitmap_bits;

  ssize_t
    y;

  /*
    Check crop geometry.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(geometry != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (((geometry->x+(ssize_t) geometry->width) < 0) ||
      ((geometry->y+(ssize_t) geometry->height) < 0) ||
      (geometry->x >= (ssize_t) image->columns) ||
      (geometry->y >= (ssize_t) image->rows))
    ThrowImageException(OptionError,"GeometryDoesNotContainImage");
  page=(*geometry);
  if ((page.x+(ssize_t) page.width) > (ssize_t) image->columns)
    page.width=image->columns-page.x;
  if ((page.y+(ssize_t) page.height) > (ssize_t) image->rows)
    page.height=image->rows-page.y;
  if (page.x < 0)
    {
      page.width+=page.x;
      page.x=0;
    }
  if (page.y < 0)
    {
      page.height+=page.y;
      page.y=0;
    }

  if ((page.width == 0) || (page.height == 0))
    ThrowImageException(OptionError,"GeometryDimensionsAreZero");
  /*
    Initialize crop image attributes.
  */
  bitmap.bmType         = 0;
  bitmap.bmWidth        = (LONG) page.width;
  bitmap.bmHeight       = (LONG) page.height;
  bitmap.bmWidthBytes   = bitmap.bmWidth * 4;
  bitmap.bmPlanes       = 1;
  bitmap.bmBitsPixel    = 32;
  bitmap.bmBits         = NULL;

  bitmap_bitsH=(HANDLE) GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,page.width*
    page.height*bitmap.bmBitsPixel);
  if (bitmap_bitsH == NULL)
    return(NULL);
  bitmap_bits=(RGBQUAD *) GlobalLock((HGLOBAL) bitmap_bitsH);
  if ( bitmap.bmBits == NULL )
    bitmap.bmBits = bitmap_bits;
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    SetImageColorspace(image,sRGBColorspace,exception);
  /*
    Extract crop image.
  */
  q=bitmap_bits;
  for (y=0; y < (ssize_t) page.height; y++)
  {
    register ssize_t
      x;

    p=GetVirtualPixels(image,page.x,page.y+y,page.width,1,exception);
    if (p == (const Quantum *) NULL)
      break;

    /* Transfer pixels, scaling to Quantum */
    for( x=(ssize_t) page.width ; x> 0 ; x-- )
    {
      q->rgbRed = ScaleQuantumToChar(GetPixelRed(image,p));
      q->rgbGreen = ScaleQuantumToChar(GetPixelGreen(image,p));
      q->rgbBlue = ScaleQuantumToChar(GetPixelBlue(image,p));
      q->rgbReserved = 0;
      p+=GetPixelChannels(image);
      q++;
    }
    proceed=SetImageProgress(image,CropImageTag,y,page.height);
    if (proceed == MagickFalse)
      break;
  }
  if (y < (ssize_t) page.height)
    {
      GlobalUnlock((HGLOBAL) bitmap_bitsH);
      GlobalFree((HGLOBAL) bitmap_bitsH);
      return((void *) NULL);
    }
  bitmap.bmBits=bitmap_bits;
  bitmapH=CreateBitmapIndirect(&bitmap);
  GlobalUnlock((HGLOBAL) bitmap_bitsH);
  GlobalFree((HGLOBAL) bitmap_bitsH);
  return((void *) bitmapH);
}
#endif

#if defined(MAGICKCORE_WINDOWS_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m a g e T o H B i t m a p                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImageToHBITMAP() creates a Windows HBITMAP from an image.
%
%  The format of the ImageToHBITMAP method is:
%
%      HBITMAP ImageToHBITMAP(Image *image,Exceptioninfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image to convert.
%
*/
MagickExport void *ImageToHBITMAP(Image *image,ExceptionInfo *exception)
{
  BITMAP
    bitmap;

  HANDLE
    bitmap_bitsH;

  HBITMAP
    bitmapH;

  register ssize_t
    x;

  register const Quantum
    *p;

  register RGBQUAD
    *q;

  RGBQUAD
    *bitmap_bits;

  size_t
    length;

  ssize_t
    y;

  (void) memset(&bitmap,0,sizeof(bitmap));
  bitmap.bmType=0;
  bitmap.bmWidth=(LONG) image->columns;
  bitmap.bmHeight=(LONG) image->rows;
  bitmap.bmWidthBytes=4*bitmap.bmWidth;
  bitmap.bmPlanes=1;
  bitmap.bmBitsPixel=32;
  bitmap.bmBits=NULL;
  length=bitmap.bmWidthBytes*bitmap.bmHeight;
  bitmap_bitsH=(HANDLE) GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,length);
  if (bitmap_bitsH == NULL)
    {
      char
        *message;

      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",message);
      message=DestroyString(message);
      return(NULL);
    }
  bitmap_bits=(RGBQUAD *) GlobalLock((HGLOBAL) bitmap_bitsH);
  q=bitmap_bits;
  if (bitmap.bmBits == NULL)
    bitmap.bmBits=bitmap_bits;
  (void) SetImageColorspace(image,sRGBColorspace,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      q->rgbRed=ScaleQuantumToChar(GetPixelRed(image,p));
      q->rgbGreen=ScaleQuantumToChar(GetPixelGreen(image,p));
      q->rgbBlue=ScaleQuantumToChar(GetPixelBlue(image,p));
      q->rgbReserved=0;
      p+=GetPixelChannels(image);
      q++;
    }
  }
  bitmap.bmBits=bitmap_bits;
  bitmapH=CreateBitmapIndirect(&bitmap);
  if (bitmapH == NULL)
    {
      char
        *message;

      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",message);
      message=DestroyString(message);
    }
  GlobalUnlock((HGLOBAL) bitmap_bitsH);
  GlobalFree((HGLOBAL) bitmap_bitsH);
  return((void *) bitmapH);
}
#endif

#endif

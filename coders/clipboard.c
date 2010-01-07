/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%                        Read/Write Windows Clipboard.                        %
%                                                                             %
%                              Software Design                                %
%                             Leonard Rosenthol                               %
%                                 May 2002                                    %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
#  if defined(__CYGWIN__)
#    include <windows.h>
#  else
     /* All MinGW needs ... */
#    include <wingdi.h>
#  endif
#endif
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/nt-feature.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
  Forward declarations.
*/
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
static MagickBooleanType
  WriteCLIPBOARDImage(const ImageInfo *,Image *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C L I P B O A R D I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadCLIPBOARDImage() reads an image from the system clipboard and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadCLIPBOARDImage method is:
%
%      Image *ReadCLIPBOARDImage(const ImageInfo *image_info,
%        ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
static Image *ReadCLIPBOARDImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  long
    y;

  register long
    x;

  register PixelPacket
    *q;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  {
    HBITMAP
      bitmapH;

    HPALETTE
      hPal;

    OpenClipboard(NULL);
    bitmapH=(HBITMAP) GetClipboardData(CF_BITMAP);
    hPal=(HPALETTE) GetClipboardData(CF_PALETTE);
    CloseClipboard();
    if ( bitmapH == NULL )
      ThrowReaderException(CoderError,"NoBitmapOnClipboard");
    {
      BITMAPINFO
        DIBinfo;

      BITMAP
        bitmap;

      HBITMAP
        hBitmap,
        hOldBitmap;

      HDC
        hDC,
        hMemDC;

      RGBQUAD
        *pBits,
        *ppBits;

      /* create an offscreen DC for the source */
      hMemDC=CreateCompatibleDC(NULL);
      hOldBitmap=(HBITMAP) SelectObject(hMemDC,bitmapH);
      GetObject(bitmapH,sizeof(BITMAP),(LPSTR) &bitmap);
      if ((image->columns == 0) || (image->rows == 0))
        {
          image->rows=bitmap.bmHeight;
          image->columns=bitmap.bmWidth;
        }
      /*
        Initialize the bitmap header info.
      */
      (void) ResetMagickMemory(&DIBinfo,0,sizeof(BITMAPINFO));
      DIBinfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
      DIBinfo.bmiHeader.biWidth=image->columns;
      DIBinfo.bmiHeader.biHeight=(-1)*image->rows;
      DIBinfo.bmiHeader.biPlanes=1;
      DIBinfo.bmiHeader.biBitCount=32;
      DIBinfo.bmiHeader.biCompression=BI_RGB;
      hDC=GetDC(NULL);
      if (hDC == 0)
        ThrowReaderException(CoderError,"UnableToCreateADC");
      hBitmap=CreateDIBSection(hDC,&DIBinfo,DIB_RGB_COLORS,(void **) &ppBits,
        NULL,0);
      ReleaseDC(NULL,hDC);
      if (hBitmap == 0)
        ThrowReaderException(CoderError,"UnableToCreateBitmap");
      /* create an offscreen DC */
      hDC=CreateCompatibleDC(NULL);
      if (hDC == 0)
        {
          DeleteObject(hBitmap);
          ThrowReaderException(CoderError,"UnableToCreateADC");
        }
      hOldBitmap=(HBITMAP) SelectObject(hDC,hBitmap);
      if (hOldBitmap == 0)
        {
          DeleteDC(hDC);
          DeleteObject(hBitmap);
          ThrowReaderException(CoderError,"UnableToCreateBitmap");
        }
      if (hPal != NULL)
      {
        /* Kenichi Masuko says this needed */
        SelectPalette(hDC, hPal, FALSE);
        RealizePalette(hDC);
      }
      /* bitblt from the memory to the DIB-based one */
      BitBlt(hDC,0,0,image->columns,image->rows,hMemDC,0,0,SRCCOPY);
      /* finally copy the pixels! */
      pBits=ppBits;
      for (y=0; y < (long) image->rows; y++)
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=ScaleCharToQuantum(pBits->rgbRed);
          q->green=ScaleCharToQuantum(pBits->rgbGreen);
          q->blue=ScaleCharToQuantum(pBits->rgbBlue);
          SetOpacityPixelComponent(q,OpaqueOpacity);
          pBits++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      DeleteDC(hDC);
      DeleteObject(hBitmap);
    }
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}
#endif /* MAGICKCORE_WINGDI32_DELEGATE */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r C L I P B O A R D I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterCLIPBOARDImage() adds attributes for the clipboard "image format" to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterCLIPBOARDImage method is:
%
%      unsigned long RegisterCLIPBOARDImage(void)
%
*/
ModuleExport unsigned long RegisterCLIPBOARDImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("CLIPBOARD");
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadCLIPBOARDImage;
  entry->encoder=(EncodeImageHandler *) WriteCLIPBOARDImage;
#endif
  entry->adjoin=MagickFalse;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("The system clipboard");
  entry->module=ConstantString("CLIPBOARD");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r C L I P B O A R D I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterCLIPBOARDImage() removes format registrations made by the
%  RGB module from the list of supported formats.
%
%  The format of the UnregisterCLIPBOARDImage method is:
%
%      UnregisterCLIPBOARDImage(void)
%
*/
ModuleExport void UnregisterCLIPBOARDImage(void)
{
  (void) UnregisterMagickInfo("CLIPBOARD");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e C L I P B O A R D I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteCLIPBOARDImage() writes an image to the system clipboard.
%
%  The format of the WriteCLIPBOARDImage method is:
%
%      MagickBooleanType WriteCLIPBOARDImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
static MagickBooleanType WriteCLIPBOARDImage(const ImageInfo *image_info,
  Image *image)
{
  /*
    Allocate memory for pixels.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  {
    HBITMAP
      bitmapH;

    OpenClipboard( NULL );
    EmptyClipboard();
    bitmapH=(HBITMAP) ImageToHBITMAP(image);
    SetClipboardData(CF_BITMAP,bitmapH);
    CloseClipboard();
  }
  return(MagickTrue);
}
#endif /* MAGICKCORE_WINGDI32_DELEGATE */


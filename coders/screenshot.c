/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     SSSSS   CCCC  RRRR   EEEEE  EEEEE  N   N  SSSSS  H   H   OOO   TTTTT    %
%     SS     C      R   R  E      E      NN  N  SS     H   H  O   O    T      %
%      SSS   C      RRRR   EEE    EEE    N N N   SSS   HHHHH  O   O    T      %
%        SS  C      R R    E      E      N  NN     SS  H   H  O   O    T      %
%     SSSSS   CCCC  R  R   EEEEE  EEEEE  N   N  SSSSS  H   H   OOO     T      %
%                                                                             %
%                                                                             %
%                  Takes a screenshot from the monitor(s).                    %
%                                                                             %
%                              Software Design                                %
%                                Dirk Lemstra                                 %
%                                 April 2014                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization      %
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
#    include "magick/nt-base-private.h"
#    include <wingdi.h>
#  ifndef DISPLAY_DEVICE_ACTIVE
#    define DISPLAY_DEVICE_ACTIVE    0x00000001
#  endif
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
#include "magick/module.h"
#include "magick/nt-feature.h"
#include "magick/option.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xwindow.h"
#include "magick/xwindow-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S C R E E N S H O T I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadSCREENSHOTImage() Takes a screenshot from the monitor(s).
%
%  The format of the ReadSCREENSHOTImage method is:
%
%      Image *ReadXImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadSCREENSHOTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=(Image *) NULL;
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  {
    BITMAPINFO
      bmi;

    DISPLAY_DEVICE
      device;

    HBITMAP
      bitmap,
      bitmapOld;

    HDC
      bitmapDC,
      hDC;

    Image
      *screen;

    int
      i;

    MagickBooleanType
      status;

    register PixelPacket
      *q;

    register ssize_t
      x;

    RGBTRIPLE
      *p;

    ssize_t
      y;

    assert(image_info != (const ImageInfo *) NULL);
    i=0;
    device.cb = sizeof(device);
    image=(Image *) NULL;
    while(EnumDisplayDevices(NULL,i,&device,0) && ++i)
    {
      if ((device.StateFlags & DISPLAY_DEVICE_ACTIVE) != DISPLAY_DEVICE_ACTIVE)
        continue;

      hDC=CreateDC(device.DeviceName,device.DeviceName,NULL,NULL);
      if (hDC == (HDC) NULL)
        ThrowReaderException(CoderError,"UnableToCreateDC");

      screen=AcquireImage(image_info);
      screen->columns=(size_t) GetDeviceCaps(hDC,HORZRES);
      screen->rows=(size_t) GetDeviceCaps(hDC,VERTRES);
      screen->storage_class=DirectClass;
      status=SetImageExtent(screen,screen->columns,screen->rows);
      if (status == MagickFalse)
        {
          InheritException(exception,&image->exception);
          return(DestroyImageList(image));
        }
      if (image == (Image *) NULL)
        image=screen;
      else
        AppendImageToList(&image,screen);

      bitmapDC=CreateCompatibleDC(hDC);
      if (bitmapDC == (HDC) NULL)
        {
          DeleteDC(hDC);
          ThrowReaderException(CoderError,"UnableToCreateDC");
        }
      (void) ResetMagickMemory(&bmi,0,sizeof(BITMAPINFO));
      bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth=(LONG) screen->columns;
      bmi.bmiHeader.biHeight=(-1)*(LONG) screen->rows;
      bmi.bmiHeader.biPlanes=1;
      bmi.bmiHeader.biBitCount=24;
      bmi.bmiHeader.biCompression=BI_RGB;
      bitmap=CreateDIBSection(hDC,&bmi,DIB_RGB_COLORS,(void **) &p,NULL,0);
      if (bitmap == (HBITMAP) NULL)
        {
          DeleteDC(hDC);
          DeleteDC(bitmapDC);
          ThrowReaderException(CoderError,"UnableToCreateBitmap");
        }
      bitmapOld=(HBITMAP) SelectObject(bitmapDC,bitmap);
      if (bitmapOld == (HBITMAP) NULL)
        {
          DeleteDC(hDC);
          DeleteDC(bitmapDC);
          DeleteObject(bitmap);
          ThrowReaderException(CoderError,"UnableToCreateBitmap");
        }
      BitBlt(bitmapDC,0,0,(int) screen->columns,(int) screen->rows,hDC,0,0,
        SRCCOPY);
      (void) SelectObject(bitmapDC,bitmapOld);

      for (y=0; y < (ssize_t) screen->rows; y++)
      {
        q=QueueAuthenticPixels(screen,0,y,screen->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) screen->columns; x++)
        {
          SetPixelRed(q,ScaleCharToQuantum(p->rgbtRed));
          SetPixelGreen(q,ScaleCharToQuantum(p->rgbtGreen));
          SetPixelBlue(q,ScaleCharToQuantum(p->rgbtBlue));
          SetPixelOpacity(q,OpaqueOpacity);
          p++;
          q++;
        }
        if (SyncAuthenticPixels(screen,exception) == MagickFalse)
          break;
      }

      DeleteDC(hDC);
      DeleteDC(bitmapDC);
      DeleteObject(bitmap);
    }
  }
#elif defined(MAGICKCORE_X11_DELEGATE)
  {
    const char
      *option;

    XImportInfo
      ximage_info;

    (void) exception;
    XGetImportInfo(&ximage_info);
    option=GetImageOption(image_info,"x:screen");
    if (option != (const char *) NULL)
      ximage_info.screen=IsMagickTrue(option);
    option=GetImageOption(image_info,"x:silent");
    if (option != (const char *) NULL)
      ximage_info.silent=IsMagickTrue(option);
    image=XImportImage(image_info,&ximage_info);
  }
#endif
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S C R E E N S H O T I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSCREENSHOTImage() adds attributes for the screen shot format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterScreenShotImage method is:
%
%      size_t RegisterScreenShotImage(void)
%
*/
ModuleExport size_t RegisterSCREENSHOTImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("SCREENSHOT");
  entry->decoder=(DecodeImageHandler *) ReadSCREENSHOTImage;
  entry->format_type=ImplicitFormatType;
  entry->description=ConstantString("Screen shot");
  entry->module=ConstantString("SCREENSHOT");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S C R E E N S H O T I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterScreenShotImage() removes format registrations made by the
%  screen shot module from the list of supported formats.
%
%  The format of the UnregisterSCREENSHOTImage method is:
%
%      UnregisterSCREENSHOTImage(void)
%
*/
ModuleExport void UnregisterSCREENSHOTImage(void)
{
  (void) UnregisterMagickInfo("SCREENSHOT");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            EEEEE  M   M  FFFFF                              %
%                            E      MM MM  F                                  %
%                            EEE    M M M  FFF                                %
%                            E      M   M  F                                  %
%                            EEEEE  M   M  F                                  %
%                                                                             %
%                                                                             %
%                  Read Windows Enahanced Metafile Format                     %
%                                                                             %
%                              Software Design                                %
%                              Bill Radcliffe                                 %
%                                   2001                                      %
%                               Dirk Lemstra                                  %
%                               January 2014                                  %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
*/

/*
 * Include declarations.
 */

#include "magick/studio.h"
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
#  include <gdiplus.h>
#  pragma comment(lib, "gdiplus.lib")
#endif
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/pixel.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s E F M                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsEMF() returns MagickTrue if the image format type, identified by the
%  magick string, is a Microsoft Windows Enhanced MetaFile (EMF) file.
%
%  The format of the ReadEMFImage method is:
%
%      MagickBooleanType IsEMF(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsEMF(const unsigned char *magick,const size_t length)
{
  if (length < 48)
    return(MagickFalse);
  if (memcmp(magick+40,"\040\105\115\106\000\000\001\000",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s W M F                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsWMF() returns MagickTrue if the image format type, identified by the
%  magick string, is a Windows MetaFile (WMF) file.
%
%  The format of the ReadEMFImage method is:
%
%      MagickBooleanType IsEMF(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsWMF(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\327\315\306\232",4) == 0)
    return(MagickTrue);
  if (memcmp(magick,"\001\000\011\000",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d E M F I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadEMFImage() reads an Microsoft Windows Enhanced MetaFile (EMF) or
%  Windows MetaFile (WMF) file using the Windows API and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadEMFImage method is:
%
%      Image *ReadEMFImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info..
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(MAGICKCORE_WINGDI32_DELEGATE)
static Image *ReadEMFImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Gdiplus::Bitmap
    *bitmap;

  Gdiplus::BitmapData
     bitmap_data;

  Gdiplus::GdiplusStartupInput
    startup_input;

  Gdiplus::Graphics
    *graphics;

  Gdiplus::Image
    *source;

  Gdiplus::Rect
    rect;

  GeometryInfo
    geometry_info;

  Image
    *image;

  MagickStatusType
    flags;

  register PixelPacket
    *q;

  register ssize_t
    x;

  ssize_t
    y;

  ULONG_PTR
    token;

  unsigned char
    *p;

  wchar_t
    fileName[MaxTextExtent];

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);

  image=AcquireImage(image_info);
  if (Gdiplus::GdiplusStartup(&token,&startup_input,NULL) != 
    Gdiplus::Status::Ok)
    ThrowReaderException(CoderError, "GdiplusStartupFailed");
  MultiByteToWideChar(CP_UTF8,0,image->filename,-1,fileName,MaxTextExtent);
  source=Gdiplus::Image::FromFile(fileName);
  if (source == (Gdiplus::Image *) NULL)
    {
      Gdiplus::GdiplusShutdown(token);
      ThrowReaderException(FileOpenError,"UnableToOpenFile");
    }

  image->x_resolution=source->GetHorizontalResolution();
  image->y_resolution=source->GetVerticalResolution();
  image->columns=(size_t) source->GetWidth();
  image->rows=(size_t) source->GetHeight();
  if (image_info->density != (char *) NULL)
    {
      flags=ParseGeometry(image_info->density,&geometry_info);
      image->x_resolution=geometry_info.rho;
      image->y_resolution=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->y_resolution=image->x_resolution;
      if ((image->x_resolution > 0.0) && (image->y_resolution > 0.0))
        {
          image->columns=(size_t) floor((Gdiplus::REAL) source->GetWidth() /
            source->GetHorizontalResolution() * image->x_resolution + 0.5);
          image->rows=(size_t)floor((Gdiplus::REAL) source->GetHeight() /
            source->GetVerticalResolution() * image->y_resolution + 0.5);
        }
    }

  bitmap=new Gdiplus::Bitmap((INT) image->columns,(INT) image->rows,
    PixelFormat32bppARGB);
  graphics=Gdiplus::Graphics::FromImage(bitmap);
  graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
  graphics->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
  graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
  graphics->Clear(Gdiplus::Color((BYTE) ScaleQuantumToChar(QuantumRange-
    image->background_color.opacity),(BYTE) ScaleQuantumToChar(
    image->background_color.red),(BYTE) ScaleQuantumToChar(
    image->background_color.green),(BYTE) ScaleQuantumToChar(
    image->background_color.blue)));
  graphics->DrawImage(source,0,0,(INT) image->columns,(INT) image->rows);
  delete graphics;
  delete source;

  rect=Gdiplus::Rect(0,0,(INT) image->columns,(INT) image->rows);
  if (bitmap->LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,
    &bitmap_data) != Gdiplus::Ok)
  {
    delete bitmap;
    Gdiplus::GdiplusShutdown(token);
    ThrowReaderException(FileOpenError,"UnableToReadImageData");
  }

  image->matte=MagickTrue;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=(unsigned char *) bitmap_data.Scan0+(y*abs(bitmap_data.Stride));
    if (bitmap_data.Stride < 0)
      q=GetAuthenticPixels(image,0,image->rows-y-1,image->columns,1,exception);
    else
      q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;

    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelBlue(q,ScaleCharToQuantum(*p++));
      SetPixelGreen(q,ScaleCharToQuantum(*p++));
      SetPixelRed(q,ScaleCharToQuantum(*p++));
      SetPixelAlpha(q,ScaleCharToQuantum(*p++));
      q++;
    }

    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }

  bitmap->UnlockBits(&bitmap_data);
  delete bitmap;
  Gdiplus::GdiplusShutdown(token);
  return(image);
}
#endif /* MAGICKCORE_WINGDI32_DELEGATE */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r E M F I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterEMFImage() adds attributes for the EMF image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterEMFImage method is:
%
%      size_t RegisterEMFImage(void)
%
*/
ModuleExport size_t RegisterEMFImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("EMF");
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  entry->decoder=ReadEMFImage;
#endif
  entry->description=ConstantString(
    "Windows Enhanced Meta File");
  entry->magick=(IsImageFormatHandler *) IsEMF;
  entry->blob_support=MagickFalse;
  entry->module=ConstantString("WMF");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("WMF");
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  entry->decoder=ReadEMFImage;
#endif
  entry->description=ConstantString("Windows Meta File");
  entry->magick=(IsImageFormatHandler *) IsWMF;
  entry->blob_support=MagickFalse;
  entry->module=ConstantString("WMF");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r E M F I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterEMFImage() removes format registrations made by the
%  EMF module from the list of supported formats.
%
%  The format of the UnregisterEMFImage method is:
%
%      UnregisterEMFImage(void)
%
*/
ModuleExport void UnregisterEMFImage(void)
{
  (void) UnregisterMagickInfo("EMF");
  (void) UnregisterMagickInfo("WMF");
}

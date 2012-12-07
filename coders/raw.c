/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            RRRR    AAA   W   W                              %
%                            R   R  A   A  W   W                              %
%                            RRRR   AAAAA  W W W                              %
%                            R R    A   A  WW WW                              %
%                            R  R   A   A  W   W                              %
%                                                                             %
%                                                                             %
%                       Read/Write RAW Image Format                           %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/colorspace.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteRAWImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d R A W I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadRAWImage() reads an image of raw samples and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadRAWImage method is:
%
%      Image *ReadRAWImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadRAWImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *canvas_image,
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  size_t
    length;

  ssize_t
    count,
    y;

  unsigned char
    *pixels;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(OptionError,"MustSpecifyImageSize");
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if (DiscardBlobBytes(image,image->offset) == MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  /*
    Create virtual canvas to support cropping (i.e. image.gray[100x100+10+20]).
  */
  canvas_image=CloneImage(image,image->extract_info.width,1,MagickFalse,
    exception);
  (void) SetImageVirtualPixelMethod(canvas_image,BlackVirtualPixelMethod);
  quantum_type=GrayQuantum;
  quantum_info=AcquireQuantumInfo(image_info,canvas_image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  pixels=GetQuantumPixels(quantum_info);
  if (image_info->number_scenes != 0)
    while (image->scene < image_info->scene)
    {
      /*
        Skip to next image.
      */
      image->scene++;
      length=GetQuantumExtent(canvas_image,quantum_info,quantum_type);
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
      }
    }
  scene=0;
  count=0;
  length=0;
  do
  {
    /*
      Read pixels to virtual canvas image then push to image.
    */
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if (scene == 0)
      {
        length=GetQuantumExtent(canvas_image,quantum_info,quantum_type);
        count=ReadBlob(image,length,pixels);
      }
    for (y=0; y < (ssize_t) image->extract_info.height; y++)
    {
      register const PixelPacket
        *restrict p;

      register PixelPacket
        *restrict q;

      register ssize_t
        x;

      if (count != (ssize_t) length)
        {
          ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
            image->filename);
          break;
        }
      q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        break;
      length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,quantum_info,
        quantum_type,pixels,exception);
      if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
        break;
      if (((y-image->extract_info.y) >= 0) &&
          ((y-image->extract_info.y) < (ssize_t) image->rows))
        {
          p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
            image->columns,1,exception);
          q=QueueAuthenticPixels(image,0,y-image->extract_info.y,image->columns,
            1,exception);
          if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelRed(q,GetPixelRed(p));
            SetPixelGreen(q,GetPixelGreen(p));
            SetPixelBlue(q,GetPixelBlue(p));
            p++;
            q++;
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
        }
      if (image->previous == (Image *) NULL)
        {
          status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
      count=ReadBlob(image,length,pixels);
    }
    SetQuantumImageType(image,quantum_type);
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if (count == (ssize_t) length)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
    scene++;
  } while (count == (ssize_t) length);
  quantum_info=DestroyQuantumInfo(quantum_info);
  InheritException(&image->exception,&canvas_image->exception);
  canvas_image=DestroyImage(canvas_image);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r R A W I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterRAWImage() adds attributes for the RAW image format to the list of
%  supported formats.  The attributes include the image format tag, a method to
%  read and/or write the format, whether the format supports the saving of
%  more than one frame to the same file or blob, whether the format supports
%  native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterRAWImage method is:
%
%      size_t RegisterRAWImage(void)
%
*/
ModuleExport size_t RegisterRAWImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("R");
  entry->decoder=(DecodeImageHandler *) ReadRAWImage;
  entry->encoder=(EncodeImageHandler *) WriteRAWImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Raw red samples");
  entry->module=ConstantString("RAW");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("C");
  entry->decoder=(DecodeImageHandler *) ReadRAWImage;
  entry->encoder=(EncodeImageHandler *) WriteRAWImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Raw cyan samples");
  entry->module=ConstantString("RAW");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("G");
  entry->decoder=(DecodeImageHandler *) ReadRAWImage;
  entry->encoder=(EncodeImageHandler *) WriteRAWImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Raw green samples");
  entry->module=ConstantString("RAW");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("M");
  entry->decoder=(DecodeImageHandler *) ReadRAWImage;
  entry->encoder=(EncodeImageHandler *) WriteRAWImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Raw magenta samples");
  entry->module=ConstantString("RAW");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("B");
  entry->decoder=(DecodeImageHandler *) ReadRAWImage;
  entry->encoder=(EncodeImageHandler *) WriteRAWImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Raw blue samples");
  entry->module=ConstantString("RAW");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("Y");
  entry->decoder=(DecodeImageHandler *) ReadRAWImage;
  entry->encoder=(EncodeImageHandler *) WriteRAWImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Raw yellow samples");
  entry->module=ConstantString("RAW");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("A");
  entry->decoder=(DecodeImageHandler *) ReadRAWImage;
  entry->encoder=(EncodeImageHandler *) WriteRAWImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Raw alpha samples");
  entry->module=ConstantString("RAW");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("O");
  entry->decoder=(DecodeImageHandler *) ReadRAWImage;
  entry->encoder=(EncodeImageHandler *) WriteRAWImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Raw opacity samples");
  entry->module=ConstantString("RAW");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("K");
  entry->decoder=(DecodeImageHandler *) ReadRAWImage;
  entry->encoder=(EncodeImageHandler *) WriteRAWImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Raw black samples");
  entry->module=ConstantString("RAW");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r R A W I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterRAWImage() removes format registrations made by the RAW module
%  from the list of supported formats.
%
%  The format of the UnregisterRAWImage method is:
%
%      UnregisterRAWImage(void)
%
*/
ModuleExport void UnregisterRAWImage(void)
{
  (void) UnregisterMagickInfo("R");
  (void) UnregisterMagickInfo("C");
  (void) UnregisterMagickInfo("G");
  (void) UnregisterMagickInfo("M");
  (void) UnregisterMagickInfo("B");
  (void) UnregisterMagickInfo("Y");
  (void) UnregisterMagickInfo("A");
  (void) UnregisterMagickInfo("O");
  (void) UnregisterMagickInfo("K");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e R A W I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteRAWImage() writes an image to a file as raw intensity values.
%
%  The format of the WriteRAWImage method is:
%
%      MagickBooleanType WriteRAWImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteRAWImage(const ImageInfo *image_info,Image *image)
{
  MagickOffsetType
    scene;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  MagickBooleanType
    status;

  register const PixelPacket
    *p;

  size_t
    length;

  ssize_t
    count,
    y;

  unsigned char
    *pixels;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  switch (*image->magick)
  {
    case 'A':
    case 'a':
    {
      quantum_type=AlphaQuantum;
      break;
    }
    case 'B':
    case 'b':
    {
      quantum_type=BlueQuantum;
      break;
    }
    case 'C':
    case 'c':
    {
      quantum_type=CyanQuantum;
      if (image->colorspace == CMYKColorspace)
        break;
      ThrowWriterException(ImageError,"ColorSeparatedImageRequired");
    }
    case 'g':
    case 'G':
    {
      quantum_type=GreenQuantum;
      break;
    }
    case 'I':
    case 'i':
    {
      quantum_type=IndexQuantum;
      break;
    }
    case 'K':
    case 'k':
    {
      quantum_type=BlackQuantum;
      if (image->colorspace == CMYKColorspace)
        break;
      ThrowWriterException(ImageError,"ColorSeparatedImageRequired");
    }
    case 'M':
    case 'm':
    {
      quantum_type=MagentaQuantum;
      if (image->colorspace == CMYKColorspace)
        break;
      ThrowWriterException(ImageError,"ColorSeparatedImageRequired");
    }
    case 'o':
    case 'O':
    {
      quantum_type=OpacityQuantum;
      break;
    }
    case 'R':
    case 'r':
    {
      quantum_type=RedQuantum;
      break;
    }
    case 'Y':
    case 'y':
    {
      quantum_type=YellowQuantum;
      if (image->colorspace == CMYKColorspace)
        break;
      ThrowWriterException(ImageError,"ColorSeparatedImageRequired");
    }
    default:
    {
      quantum_type=GrayQuantum;
      break;
    }
  }
  scene=0;
  do
  {
    /*
      Convert image to RAW raster pixels.
    */
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=GetQuantumPixels(quantum_info);
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
      if (p == (const PixelPacket *) NULL)
        break;
      length=ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
        quantum_type,pixels,&image->exception);
      count=WriteBlob(image,length,pixels);
      if (count != (ssize_t) length)
        break;
      if (image->previous == (Image *) NULL)
        {
          status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
    }
    quantum_info=DestroyQuantumInfo(quantum_info);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}

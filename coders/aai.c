/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             AAA    AAA   IIIII                              %
%                            A   A  A   A    I                                %
%                            AAAAA  AAAAA    I                                %
%                            A   A  A   A    I                                %
%                            A   A  A   A  IIIII                              %
%                                                                             %
%                                                                             %
%                        Read/Write AAI X Image Format                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteAAIImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d A A I I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadAAIImage() reads an AAI Dune image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadAAIImage method is:
%
%      Image *ReadAAIImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadAAIImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  register ssize_t
    x;

  register Quantum
    *q;

  register unsigned char
    *p;

  size_t
    height,
    length,
    width;

  ssize_t
    count,
    y;

  unsigned char
    *pixels;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read AAI Dune image.
  */
  width=ReadBlobLSBLong(image);
  height=ReadBlobLSBLong(image);
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if ((width == 0UL) || (height == 0UL))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  do
  {
    /*
      Convert AAI raster image to pixel packets.
    */
    image->columns=width;
    image->rows=height;
    image->depth=8;
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    pixels=(unsigned char *) AcquireQuantumMemory(image->columns,
      4*sizeof(*pixels));
    if (pixels == (unsigned char *) NULL) 
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    length=(size_t) 4*image->columns;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      count=ReadBlob(image,length,pixels);
      if (count != (ssize_t) length)
        {
          pixels=(unsigned char *) RelinquishMagickMemory(pixels);
          ThrowReaderException(CorruptImageError,"UnableToReadImageData");
        }
      p=pixels;
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
        SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
        SetPixelRed(image,ScaleCharToQuantum(*p++),q);
        if (*p == 254)
          *p=255;
        SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
        if (GetPixelAlpha(image,q) != OpaqueAlpha)
          image->alpha_trait=BlendPixelTrait;
        q+=GetPixelChannels(image);
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        break;
      if (image->previous == (Image *) NULL)
        {
          status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
    }
    pixels=(unsigned char *) RelinquishMagickMemory(pixels);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    width=ReadBlobLSBLong(image);
    height=ReadBlobLSBLong(image);
    if ((width != 0UL) && (height != 0UL))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
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
  } while ((width != 0UL) && (height != 0UL));
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r A A I I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterAAIImage() adds attributes for the AAI Dune image format to the list
%  of supported formats.  The attributes include the image format tag, a
%  method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterAAIImage method is:
%
%      size_t RegisterAAIImage(void)
%
*/
ModuleExport size_t RegisterAAIImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("AAI","AAI","AAI Dune image");
  entry->decoder=(DecodeImageHandler *) ReadAAIImage;
  entry->encoder=(EncodeImageHandler *) WriteAAIImage;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r A A I I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterAAIImage() removes format registrations made by the
%  AAI module from the list of supported formats.
%
%  The format of the UnregisterAAIImage method is:
%
%      UnregisterAAIImage(void)
%
*/
ModuleExport void UnregisterAAIImage(void)
{
  (void) UnregisterMagickInfo("AAI");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e A A I I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteAAIImage() writes an image to a file in AAI Dune image format.
%
%  The format of the WriteAAIImage method is:
%
%      MagickBooleanType WriteAAIImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WriteAAIImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  register const Quantum
    *magick_restrict p;

  register ssize_t
    x;

  register unsigned char
    *magick_restrict q;

  ssize_t
    count,
    y;

  unsigned char
    *pixels;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  do
  {
    /*
      Write AAI header.
    */
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
    (void) WriteBlobLSBLong(image,(unsigned int) image->columns);
    (void) WriteBlobLSBLong(image,(unsigned int) image->rows);
    /*
      Allocate memory for pixels.
    */
    pixels=(unsigned char *) AcquireQuantumMemory(image->columns,
      4*sizeof(*pixels));
    if (pixels == (unsigned char *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    /*
      Convert MIFF to AAI raster pixels.
    */
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        break;
      q=pixels;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
        *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
        *q++=ScaleQuantumToChar(GetPixelRed(image,p));
        *q=ScaleQuantumToChar((Quantum) (image->alpha_trait !=
          UndefinedPixelTrait ? GetPixelAlpha(image,p) : OpaqueAlpha));
        if (*q == 255)
          *q=254;
        p+=GetPixelChannels(image);
        q++;
      }
      count=WriteBlob(image,(size_t) (q-pixels),pixels);
      if (count != (ssize_t) (q-pixels))
        break;
      if (image->previous == (Image *) NULL)
        {
          status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
    }
    pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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

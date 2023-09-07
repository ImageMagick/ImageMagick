/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%                         FFFFF  L      IIIII  FFFFF                          %
%                         F      L        I    F                              %
%                         FFF    L        I    FFF                            %
%                         F      L        I    F                              %
%                         F      LLLLL  IIIII  F                              %
%                                                                             %
%                                                                             %
%                       Support FilmLight Image Format                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               November 2020                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
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
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteFL32Image(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s F L 3 2                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsFL32() returns MagickTrue if the image format type, identified by the
%  magick string, is FL32.
%
%  The format of the IsFL32 method is:
%
%      MagickBooleanType IsFL32(const unsigned char *magick,const size_t extent)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o extent: Specifies the extent of the magick string.
%
*/
static MagickBooleanType IsFL32(const unsigned char *magick,const size_t extent)
{
  if (extent < 4)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"FL32",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d F L 3 2 I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFL32Image() reads an image of raw bits in LSB order and returns it.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the ReadFL32Image method is:
%
%      Image *ReadFL32Image(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadFL32Image(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  size_t
    extent;

  ssize_t
    count,
    y;

  unsigned char
    *pixels;

  unsigned int
    magic;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  magic=ReadBlobLSBLong(image);
  if (magic != 842222662)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->depth=32;
  image->endian=LSBEndian;
  image->rows=(size_t) ReadBlobLSBLong(image);
  image->columns=(size_t) ReadBlobLSBLong(image);
  image->number_channels=(size_t) ReadBlobLSBLong(image);
  if ((image->columns == 0) || (image->rows == 0) ||
      (image->number_channels == 0) ||
      (image->number_channels >= MaxPixelChannels))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  switch (image->number_channels)
  {
    case 1:
    {
      quantum_type=GrayQuantum;
      image->colorspace=GRAYColorspace;
      break;
    }
    case 2:
    {
      image->alpha_trait=BlendPixelTrait;
      image->colorspace=GRAYColorspace;
      quantum_type=GrayAlphaQuantum;
      break;
    }
    case 3:
    {
      image->colorspace=sRGBColorspace;
      quantum_type=RGBQuantum;
      break;
    }
    case 4:
    {
      image->colorspace=sRGBColorspace;
      image->alpha_trait=BlendPixelTrait;
      quantum_type=RGBAQuantum;
      break;
    }
    default:
    {
      image->number_meta_channels=image->number_channels-3;
      quantum_type=RGBQuantum;
      break;
    }
  }
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Convert FL32 image to pixel packets.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
  extent=GetQuantumExtent(image,quantum_info,quantum_type);
  pixels=GetQuantumPixels(quantum_info);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const void
      *stream;

    Quantum
      *magick_restrict q;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    stream=ReadBlobStream(image,extent,pixels,&count);
    if (count != (ssize_t) extent)
      break;
    (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      quantum_type,(unsigned char *) stream,exception);
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
      break;
  }
  SetQuantumImageType(image,quantum_type);
  quantum_info=DestroyQuantumInfo(quantum_info);
  if (y < (ssize_t) image->rows)
    ThrowReaderException(CorruptImageError,"UnableToReadImageData");
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r F L 3 2 I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterFL32Image() adds attributes for the FL32 image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterFL32Image method is:
%
%      size_t RegisterFL32Image(void)
%
*/
ModuleExport size_t RegisterFL32Image(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("FL32","FL32","FilmLight");
  entry->decoder=(DecodeImageHandler *) ReadFL32Image;
  entry->encoder=(EncodeImageHandler *) WriteFL32Image;
  entry->magick=(IsImageFormatHandler *) IsFL32;
  entry->flags|=CoderRawSupportFlag;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r F L 3 2 I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterFL32Image() removes format registrations made by the
%  FL32 module from the list of supported formats.
%
%  The format of the UnregisterFL32Image method is:
%
%      UnregisterFL32Image(void)
%
*/
ModuleExport void UnregisterFL32Image(void)
{
  (void) UnregisterMagickInfo("FL32");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e F L 3 2 I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteFL32Image() writes an image of raw bits in LSB order to a file.
%
%  The format of the WriteFL32Image method is:
%
%      MagickBooleanType WriteFL32Image(const ImageInfo *image_info,
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
static MagickBooleanType WriteFL32Image(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  const Quantum
    *p;

  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  size_t
    channels,
    extent;

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
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  (void) WriteBlobLSBLong(image,842222662U);
  (void) WriteBlobLSBLong(image,(unsigned int) image->rows);
  (void) WriteBlobLSBLong(image,(unsigned int) image->columns);
  image->endian=LSBEndian;
  image->depth=32;
  channels=GetImageChannels(image);
  switch (channels)
  {
    case 1:
    {
      quantum_type=GrayQuantum;
      break;
    }
    case 2:
    {
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          quantum_type=GrayAlphaQuantum;
          break;
        }
      quantum_type=GrayQuantum;
      channels=1;
      break;
    }
    case 4:
    {
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          quantum_type=RGBAQuantum;
          break;
        }
      quantum_type=RGBQuantum;
      channels=3;
      break;
    }
    default:
    {
      quantum_type=RGBQuantum;
      channels=3;
      break;
    }
  }
  (void) WriteBlobLSBLong(image,(unsigned int) channels);
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ImageError,"MemoryAllocationFailed");
  status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    extent=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      quantum_type,pixels,exception);
    count=WriteBlob(image,extent,pixels);
    if (count != (ssize_t) extent)
      break;
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
  if (y < (ssize_t) image->rows)
    ThrowWriterException(CorruptImageError,"UnableToWriteImageData");
  (void) CloseBlob(image);
  return(status);
}

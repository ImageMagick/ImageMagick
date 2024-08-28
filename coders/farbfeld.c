/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%            FFFFF   AAA   RRRR   BBBB   FFFFF  EEEEE  L      DDDD            %
%            F      A   A  R   R  B   B  F      E      L      D   D           %
%            FFF    AAAAA  RRRR   BBBB   FFF    EEE    L      D   D           %
%            F      A   A  R R    B   B  F      E      L      D   D           %
%            F      A   A  R  R   BBBB   F      EEEEE  LLLLL  DDDD            %
%                                                                             %
%                                                                             %
%                       Support Farbfeld Image Format                         %
%                                                                             %
%                              Software Design                                %
%                                Dirk Lemstra                                 %
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
  WriteFARBFELDImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s F A R B F E L D                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsFARBFELD() returns MagickTrue if the image format type, identified by the
%  magick string, is Farbeld.
%
%  The format of the IsFarbeld method is:
%
%      MagickBooleanType IsFARBFELD(const unsigned char *magick,
%        const size_t extent)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o extent: Specifies the extent of the magick string.
%
*/
static MagickBooleanType IsFARBFELD(const unsigned char *magick,
  const size_t extent)
{
  if (extent < 8)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"farbfeld",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d F A R B F E L D I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFARBFELDImage() reads an image of raw bits in LSB order and returns it.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the ReadFARBFELDImage method is:
%
%      Image *ReadFARBFELDImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadFARBFELDImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  QuantumInfo
    *quantum_info;

  MagickBooleanType
    status;

  MagickSizeType
    magic;

  size_t
    extent;

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
  image->depth=16;
  image->endian=MSBEndian;
  magic=ReadBlobLongLong(image);
  if (magic != MagickULLConstant(7377303431559867492))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->columns=(size_t) ReadBlobLong(image);
  image->rows=(size_t) ReadBlobLong(image);
  image->alpha_trait=BlendPixelTrait;
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  status=SetQuantumFormat(image,quantum_info,UnsignedQuantumFormat);
  extent=GetQuantumExtent(image,quantum_info,RGBAQuantum);
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
      RGBAQuantum,(unsigned char *) stream,exception);
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
      break;
  }
  SetQuantumImageType(image,RGBAQuantum);
  quantum_info=DestroyQuantumInfo(quantum_info);
  if (y < (ssize_t) image->rows)
    ThrowReaderException(CorruptImageError,"UnableToReadImageData");
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r F A R B F E L D I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterFARBFELDImage() adds attributes for the FARBFELD image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterFARBFELDImage method is:
%
%      size_t RegisterFARBFELDImage(void)
%
*/
ModuleExport size_t RegisterFARBFELDImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("FARBFELD","FARBFELD","Farbfeld");
  entry->decoder=(DecodeImageHandler *) ReadFARBFELDImage;
  entry->encoder=(EncodeImageHandler *) WriteFARBFELDImage;
  entry->magick=(IsImageFormatHandler *) IsFARBFELD;
  entry->flags|=CoderRawSupportFlag;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("FARBFELD","FF","Farbfeld");
  entry->decoder=(DecodeImageHandler *) ReadFARBFELDImage;
  entry->encoder=(EncodeImageHandler *) WriteFARBFELDImage;
  entry->magick=(IsImageFormatHandler *) IsFARBFELD;
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
%   U n r e g i s t e r F A R B F E L D I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterFARBFELDImage() removes format registrations made by the
%  Farbfeld module from the list of supported formats.
%
%  The format of the UnregisterFARBFELDImage method is:
%
%      UnregisterFARBFELDImage(void)
%
*/
ModuleExport void UnregisterFARBFELDImage(void)
{
  (void) UnregisterMagickInfo("FARBFELD");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e F A R B F E L D I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteFARBFELDImage() writes an image of raw bits in MSB order to a file.
%
%  The format of the WriteFARBFELDImage method is:
%
%      MagickBooleanType WriteFARBFELDImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteFARBFELDImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  const Quantum
    *p;

  size_t
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
  image->endian=MSBEndian;
  (void) WriteBlobLongLong(image,MagickULLConstant(7377303431559867492));
  (void) WriteBlobLong(image,(unsigned int) image->columns);
  (void) WriteBlobLong(image,(unsigned int) image->rows);
  image->depth=16;
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ImageError,"MemoryAllocationFailed");
  status=SetQuantumFormat(image,quantum_info,UnsignedQuantumFormat);
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    extent=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      RGBAQuantum,pixels,exception);
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
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

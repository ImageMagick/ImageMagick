/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            SSSSS  FFFFF  3333                               %
%                            S      F          3                              %
%                            SSSSS  FFF      33                               %
%                                S  F          3                              %
%                            SSSSS  F      3333                               %
%                                                                             %
%                                                                             %
%                         Simple File Format Family                           %
%                                                                             %
%                              Software Design                                %
%                               Yukari Hafner                                 %
%                                 July 2025                                   %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/constitute.h"
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
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include <zlib.h>

/*
  Enumerated declarations.
*/
typedef enum
{
  SF3_PIXEL_INT8 = 0x01,
  SF3_PIXEL_INT16 = 0x02,
  SF3_PIXEL_INT32 = 0x04,
  SF3_PIXEL_INT64 = 0x08,
  SF3_PIXEL_UINT8 = 0x11,
  SF3_PIXEL_UINT16 = 0x12,
  SF3_PIXEL_UINT32 = 0x14,
  SF3_PIXEL_UINT64 = 0x18,
  SF3_PIXEL_FLOAT16 = 0x22,
  SF3_PIXEL_FLOAT32 = 0x24,
  SF3_PIXEL_FLOAT64 = 0x28
} SF3PixelFormat;

typedef enum
{
  SF3_PIXEL_V = 0x01,
  SF3_PIXEL_VA = 0x02,
  SF3_PIXEL_RGB = 0x03,
  SF3_PIXEL_RGBA = 0x04,
  SF3_PIXEL_AV = 0x12,
  SF3_PIXEL_BGR = 0x13,
  SF3_PIXEL_ABGR = 0x14,
  SF3_PIXEL_ARGB = 0x24,
  SF3_PIXEL_BGRA = 0x34,
  SF3_PIXEL_CMYK = 0x44,
  SF3_PIXEL_KYMC = 0x54,
} SF3ChannelLayout;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteSF3Image(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s S F 3                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsSF3()() returns MagickTrue if the image format type, identified by the
%  magick string, is SF3.
%
%  The format of the IsSF3 method is:
%
%      MagickBooleanType IsSF3(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static unsigned int IsSF3(const unsigned char *magick,const size_t length)
{
  /*
    The header is 16 bytes long, though the last 5 bytes aren't constant.
  */
  if (length < 16)
    return(MagickFalse);
  if (memcmp(magick,"\x81SF3\x00\xE0\xD0\r\n\n\x03",11) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S F 3 I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadSF3Image() reads a Simple File Format Family image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadSF3Image method is:
%
%      Image *ReadSF3Image(image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadSF3Image(const ImageInfo *image_info,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  Image
    *image;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  ssize_t
    count;
  
  size_t
    length;

  unsigned char
    *p;

  unsigned char
    channels,
    format;

  unsigned int
    width,
    height,
    layers;

  unsigned char
    header[16];

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
    return(DestroyImageList(image));
  /*
    Read SF3 header information.
  */
  count=ReadBlob(image,16,header);
  if ((count != 16) || (memcmp(header,"\x81SF3\x00\xE0\xD0\r\n\n\x03",11) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  width=ReadBlobLSBLong(image);
  height=ReadBlobLSBLong(image);
  layers = ReadBlobLSBLong(image);
  if (width == 0 || height == 0 || layers == 0)
    ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
  channels=ReadBlobByte(image);
  format=ReadBlobByte(image);
  
  for (unsigned int z=0; z<layers; ++z)
    {
      image->endian=LSBEndian;
      image->compression=NoCompression;
      image->orientation=TopLeftOrientation;
      image->columns=(size_t) width;
      image->rows=(size_t) height;
      image->depth=(size_t)((format & 0xF)*8);
      status=SetImageExtent(image,image->columns,image->rows,exception);
      if (status == MagickFalse)
        return(DestroyImageList(image));
      switch(channels)
        {
        case SF3_PIXEL_V:
          (void) SetImageColorspace(image,GRAYColorspace,exception);
          SetQuantumImageType(image,GrayQuantum);
          break;
        case SF3_PIXEL_VA:
          (void) SetImageColorspace(image,GRAYColorspace,exception);
          SetQuantumImageType(image,GrayAlphaQuantum);
          break;
        case SF3_PIXEL_RGB:
          (void) SetImageColorspace(image,RGBColorspace,exception);
          SetQuantumImageType(image,RGBQuantum);
          break;
        case SF3_PIXEL_RGBA:
          (void) SetImageColorspace(image,RGBColorspace,exception);
          SetQuantumImageType(image,RGBAQuantum);
          break;
        case SF3_PIXEL_AV:
          (void) SetImageColorspace(image,GRAYColorspace,exception);
          SetQuantumImageType(image,GrayAlphaQuantum);
          break;
        case SF3_PIXEL_BGR:
          (void) SetImageColorspace(image,RGBColorspace,exception);
          SetQuantumImageType(image,BGRQuantum);
          break;
        case SF3_PIXEL_ABGR:
          (void) SetImageColorspace(image,RGBColorspace,exception);
          SetQuantumImageType(image,BGRAQuantum);
          break;
        case SF3_PIXEL_ARGB:
          (void) SetImageColorspace(image,RGBColorspace,exception);
          SetQuantumImageType(image,RGBAQuantum);
          break;
        case SF3_PIXEL_BGRA:
          (void) SetImageColorspace(image,RGBColorspace,exception);
          SetQuantumImageType(image,BGRAQuantum);
          break;
        case SF3_PIXEL_CMYK:
          (void) SetImageColorspace(image,CMYKColorspace,exception);
          SetQuantumImageType(image,CMYKQuantum);
          break;
        case SF3_PIXEL_KYMC:
          (void) SetImageColorspace(image,CMYKColorspace,exception);
          SetQuantumImageType(image,CMYKQuantum);
          break;
        default:
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        }
      quantum_type=GetQuantumType(image,exception);
      quantum_info=AcquireQuantumInfo(image_info,image);
      if (quantum_info == (QuantumInfo *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      switch(format)
        {
        case SF3_PIXEL_INT8:
        case SF3_PIXEL_INT16:
        case SF3_PIXEL_INT32:
        case SF3_PIXEL_INT64:
          status=SetQuantumFormat(image,quantum_info,SignedQuantumFormat);
          break;
        case SF3_PIXEL_UINT8:
        case SF3_PIXEL_UINT16:
        case SF3_PIXEL_UINT32:
        case SF3_PIXEL_UINT64:
          status=SetQuantumFormat(image,quantum_info,UnsignedQuantumFormat);
          break;
        case SF3_PIXEL_FLOAT16:
        case SF3_PIXEL_FLOAT32:
        case SF3_PIXEL_FLOAT64:
          status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
          break;
        default:
          quantum_info=DestroyQuantumInfo(quantum_info);
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        }
      if (status == MagickFalse)
        {
          quantum_info=DestroyQuantumInfo(quantum_info);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      status=SetImageExtent(image,image->columns,image->rows,exception);
      if (status == MagickFalse){
        quantum_info=DestroyQuantumInfo(quantum_info);
        return(DestroyImageList(image));
      }
      length=image->columns*(format & 0xF)*(channels & 0xF);
      p=(unsigned char *) AcquireQuantumMemory(length, 1);
      if (p == (unsigned char *) NULL)
        {
          quantum_info=DestroyQuantumInfo(quantum_info);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      for (unsigned int y=0; y<image->rows; ++y)
        {
          count=ReadBlob(image,length,p);
          if (count != (ssize_t) length)
            {
              p=(unsigned char *) RelinquishMagickMemory(p);
              quantum_info=DestroyQuantumInfo(quantum_info);
              ThrowReaderException(CorruptImageError,"NotEnoughPixelData");
            }
          (void) GetAuthenticPixels(image,0,y,image->columns,1,exception);
          (void) ImportQuantumPixels(image,(CacheView *) NULL,
                                     quantum_info,quantum_type,p,exception);
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
        }
      if (image_info->number_scenes != 0)
        if (image->scene >= (image_info->scene+image_info->number_scenes-1))
          break;
      if (z+1 == layers)
        break;
      AcquireNextImage(image_info,image,exception);
      if (GetNextImageInList(image) == (Image *) NULL)
        {
          status=MagickFalse;
          break;
        }
      image=SyncNextImageInList(image);
      status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
                              GetBlobSize(image));
      if (status == MagickFalse)
        break;
    }
  p=(unsigned char *) RelinquishMagickMemory(p);
  quantum_info=DestroyQuantumInfo(quantum_info);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S F 3 I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSF3Image() adds properties for the SF3 image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSF3Image method is:
%
%      size_t RegisterSF3Image(void)
%
*/
ModuleExport size_t RegisterSF3Image(void)
{
  MagickInfo
    *entry;

  static const char
    SF3Note[] =
    {
      "See https://shirakumo.org/docs/sf3/ for information on the SF3 file"
      " formats."
    };

  entry=AcquireMagickInfo("SF3","SF3","Simple File Format Family Images");
  entry->decoder=(DecodeImageHandler *) ReadSF3Image;
  entry->encoder=(EncodeImageHandler *) WriteSF3Image;
  entry->magick=(IsImageFormatHandler *) IsSF3;
  entry->flags|=CoderAdjoinFlag;
  entry->flags|=CoderRawSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->format_type=ImplicitFormatType;
  entry->mime_type=ConstantString("image/x.sf3");
  entry->note=ConstantString(SF3Note);
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S F 3 I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterSF3Image() removes format registrations made by the
%  SF3 module from the list of supported formats.
%
%  The format of the UnregisterSF3Image method is:
%
%      UnregisterSF3Image(void)
%
*/
ModuleExport void UnregisterSF3Image(void)
{
  (void) UnregisterMagickInfo("SF3");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S F 3 I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteSF3Image() writes an image in the Simple File Format Family image
%  format.
%
%  The format of the WriteSF3Image method is:
%
%      MagickBooleanType WriteSF3Image(const ImageInfo *image_info,
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
static MagickBooleanType WriteSF3Image(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  unsigned char
    channels,
    format;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  QuantumFormatType
    quantum_format;

  unsigned int
    width,
    height,
    checksum;

  size_t
    number_scenes,
    scene,
    length,
    count,
    y;

  unsigned char
    *pixels;

  unsigned char
    header[16];
  
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
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) SetQuantumEndian(image,quantum_info,LSBEndian);
  (void) SetQuantumPad(image,quantum_info,0);
  number_scenes=GetImageListLength(image);
  quantum_type=GetQuantumType(image,exception);
  quantum_format=GetQuantumFormat(quantum_info);
  width=image->columns;
  height=image->rows;
  switch (quantum_type)
    {
    case GrayQuantum:
      channels = SF3_PIXEL_V;
      quantum_type = GrayQuantum;
      break;
    case AlphaQuantum:
    case GrayAlphaQuantum:
      channels = SF3_PIXEL_VA;
      quantum_type = GrayAlphaQuantum;
      break;
    case IndexQuantum:
    case RedQuantum:
    case GreenQuantum:
    case BlueQuantum:
    case RGBQuantum:
      channels = SF3_PIXEL_RGB;
      quantum_type = RGBQuantum;
      break;
    case IndexAlphaQuantum:
    case RGBOQuantum:
    case RGBAQuantum:
      channels = SF3_PIXEL_RGBA;
      quantum_type = RGBAQuantum;
      break;
    case BGROQuantum:
    case BGRAQuantum:
      channels = SF3_PIXEL_BGRA;
      quantum_type = BGRAQuantum;
      break;
    case BGRQuantum:
      channels = SF3_PIXEL_BGR;
      quantum_type = BGRQuantum;
      break;
    case CyanQuantum:
    case MagentaQuantum:
    case YellowQuantum:
    case BlackQuantum:
    case CMYKQuantum:
      channels = SF3_PIXEL_CMYK;
      quantum_type = CMYKQuantum;
      break;
    default:
      quantum_info=DestroyQuantumInfo(quantum_info);
      ThrowWriterException(CoderError,"ImageTypeNotSupported");
      break;
    }
  switch (quantum_format)
    {
    case FloatingPointQuantumFormat:
      if (image->depth <= 16)
        {
          format = SF3_PIXEL_FLOAT16;
          SetQuantumDepth(image,quantum_info,16);
        }
      else if (image->depth <= 32)
        {
          format = SF3_PIXEL_FLOAT32;
          SetQuantumDepth(image,quantum_info,32);
        }
      else
        {
          format = SF3_PIXEL_FLOAT64;
          SetQuantumDepth(image,quantum_info,64);
        }
      break;
    case SignedQuantumFormat:
      if (image->depth <= 8)
        {
          format = SF3_PIXEL_INT8;
        }
      else if (image->depth <= 16)
        {
          format = SF3_PIXEL_INT16;
          SetQuantumDepth(image,quantum_info,16);
        }
      else if (image->depth <= 32)
        {
          format = SF3_PIXEL_INT32;
          SetQuantumDepth(image,quantum_info,32);
        }
      else
        {
          format = SF3_PIXEL_INT64;
          SetQuantumDepth(image,quantum_info,64);
        }
      break;
    case UndefinedQuantumFormat:
      SetQuantumFormat(image,quantum_info,UnsignedQuantumFormat);
    case UnsignedQuantumFormat:
      if (image->depth <= 8)
        {
          format = SF3_PIXEL_UINT8;
        }
      else if (image->depth <= 16)
        {
          format = SF3_PIXEL_UINT16;
          SetQuantumDepth(image,quantum_info,16);
        }
      else if (image->depth <= 32)
        {
          format = SF3_PIXEL_UINT32;
          SetQuantumDepth(image,quantum_info,32);
        }
      else
        {
          format = SF3_PIXEL_UINT64;
          SetQuantumDepth(image,quantum_info,64);
        }
      break;
    default:
      quantum_info=DestroyQuantumInfo(quantum_info);
      ThrowWriterException(CoderError,"ImageTypeNotSupported");
      break;
    }
  /*
    Write SF3 header.
  */
  (void) WriteBlob(image,11,"\x81SF3\x00\xE0\xD0\r\n\n\x03");
  (void) WriteBlobLSBLong(image,(unsigned int) 0); // Zero CRC32 for now
  (void) WriteBlobByte(image,(unsigned char) 0);
  checksum=0;
  (void) WriteBlobLSBLong(image,(unsigned int) width);
  (void) WriteBlobLSBLong(image,(unsigned int) height);
  (void) WriteBlobLSBLong(image,(unsigned int) number_scenes);
  (void) WriteBlobByte(image,channels);
  (void) WriteBlobByte(image,format);
  if(IsBlobSeekable(image)){
    (void) SeekBlob(image,16,SEEK_SET);
    (void) ReadBlob(image,14,header);
    checksum=crc32(checksum,header,14);
  }
  /*
    Write pixels.
  */
  scene=0;
  do
  {
    const Quantum
      *magick_restrict p;
    pixels=(unsigned char *) GetQuantumPixels(quantum_info);
    for (y=0; y < (ssize_t) image->rows; y++)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,exception);
        if (p == (const Quantum *) NULL)
          break;
        length=ExportQuantumPixels(image,(CacheView *)NULL,quantum_info,
                                   quantum_type,pixels,exception);
        checksum=crc32(checksum,pixels,length);
        count=WriteBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
      }
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,number_scenes);
    if (status == MagickFalse)
      break;
  }while (image_info->adjoin != MagickFalse);
  /*
    Finish up.
   */
  quantum_info=DestroyQuantumInfo(quantum_info);
  if(IsBlobSeekable(image)){
    (void) SeekBlob(image,11,SEEK_SET);
    (void) WriteBlobLSBLong(image,checksum);
  }
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

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
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"

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
  CRC32 Implementation without dependency on zlib.
*/
static const unsigned int CRC32Table[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static unsigned int crc32(unsigned int crc, const void *buf, size_t size)
{
  const unsigned char *p = (const unsigned char*)buf;

  while (size--)
	crc = CRC32Table[(crc ^ *p++) & 0xff] ^ (crc >> 8);
  
  return crc;
}

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

  ssize_t
    count = 0;
  
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
  channels=(unsigned char) ReadBlobByte(image);
  format=(unsigned char) ReadBlobByte(image);
  
  for (unsigned int z=0; z<layers; ++z)
    {
      QuantumInfo
        *quantum_info;

      QuantumType
        quantum_type;

      size_t
        length = 0;

      ssize_t
        y;

      unsigned char
        *pixels;

      image->endian=LSBEndian;
      image->compression=NoCompression;
      image->orientation=TopLeftOrientation;
      image->columns=(size_t) width;
      image->rows=(size_t) height;
      image->depth=(size_t)((format & 0xF)*8);
      if (image_info->ping != MagickFalse)
        break;
      if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
        if (image->scene >= (image_info->scene+image_info->number_scenes-1))
          break;
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
      pixels=GetQuantumPixels(quantum_info);
      for (y=0; y < (ssize_t) image->rows; ++y)
        {
          count=ReadBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
          (void) GetAuthenticPixels(image,0,y,image->columns,1,exception);
          (void) ImportQuantumPixels(image,(CacheView *) NULL,
                                    quantum_info,quantum_type,pixels,exception);
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
        }
      quantum_info=DestroyQuantumInfo(quantum_info);
      if (y < (ssize_t) image->rows)
        {
          ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
            image->filename);
          break;
        }
      if (image_info->number_scenes != 0)
        if (image->scene >= (image_info->scene+image_info->number_scenes-1))
          break;
      if ((z+1) == layers)
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

  QuantumInfo
    *quantum_info;

  QuantumFormatType
    quantum_format;

  QuantumType
    quantum_type;

  size_t
    length,
    number_scenes;

  ssize_t
    count,
    scene,
    y;

  unsigned char
    channels,
    format,
    header[16],
    *pixels;
  
  unsigned int
    checksum;

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
  if ((image->columns > 4294967295UL) || (image->rows > 4294967295UL))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) SetQuantumEndian(image,quantum_info,LSBEndian);
  (void) SetQuantumPad(image,quantum_info,0);
  number_scenes=GetImageListLength(image);
  quantum_type=GetQuantumType(image,exception);
  quantum_format=GetQuantumFormat(quantum_info);
  switch (quantum_type)
    {
    case GrayQuantum:
      channels = SF3_PIXEL_V;
      quantum_type = GrayQuantum;
      break;
    case AlphaQuantum:
    case OpacityQuantum:
    case GrayAlphaQuantum:
      channels = SF3_PIXEL_VA;
      quantum_type = GrayAlphaQuantum;
      break;
    case IndexQuantum:
    case CbYCrQuantum:
    case MultispectralQuantum:
    case RedQuantum:
    case GreenQuantum:
    case BlueQuantum:
    case RGBPadQuantum:
    case RGBQuantum:
      channels = SF3_PIXEL_RGB;
      quantum_type = RGBQuantum;
      break;
    case UndefinedQuantum:
    case IndexAlphaQuantum:
    case CbYCrAQuantum:
    case CMYKAQuantum:
    case CMYKOQuantum:
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
          SetQuantumDepth(image,quantum_info,8);
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
    case UnsignedQuantumFormat:
      SetQuantumFormat(image,quantum_info,UnsignedQuantumFormat);
      if (image->depth <= 8)
        {
          format = SF3_PIXEL_UINT8;
          SetQuantumDepth(image,quantum_info,8);
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
  checksum=0xFFFFFFFF;
  (void) WriteBlobLSBLong(image,(unsigned int) image->columns);
  (void) WriteBlobLSBLong(image,(unsigned int) image->rows);
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
    (void) WriteBlobLSBLong(image,checksum  ^ 0xFFFFFFFF);
  }
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

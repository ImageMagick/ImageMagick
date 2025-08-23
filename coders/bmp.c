/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            BBBB   M   M  PPPP                               %
%                            B   B  MM MM  P   P                              %
%                            BBBB   M M M  PPPP                               %
%                            B   B  M   M  P                                  %
%                            BBBB   M   M  P                                  %
%                                                                             %
%                                                                             %
%             Read/Write Microsoft Windows Bitmap Image Format                %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                            Glenn Randers-Pehrson                            %
%                               December 2001                                 %
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
#include "MagickCore/cache.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile-private.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/transform.h"

/*
  Macro definitions (from Windows wingdi.h).
*/
#undef BI_JPEG
#define BI_JPEG  4
#undef BI_PNG
#define BI_PNG  5
#ifndef BI_ALPHABITFIELDS
 #define BI_ALPHABITFIELDS 6
#endif
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__MINGW32__)
#undef BI_RGB
#define BI_RGB  0
#undef BI_RLE8
#define BI_RLE8  1
#undef BI_RLE4
#define BI_RLE4  2
#undef BI_BITFIELDS
#define BI_BITFIELDS  3

#undef LCS_CALIBRATED_RBG
#define LCS_CALIBRATED_RBG  0
#undef LCS_sRGB
#define LCS_sRGB  1
#undef LCS_WINDOWS_COLOR_SPACE
#define LCS_WINDOWS_COLOR_SPACE  2
#undef PROFILE_LINKED
#define PROFILE_LINKED  3
#undef PROFILE_EMBEDDED
#define PROFILE_EMBEDDED  4

#undef LCS_GM_BUSINESS
#define LCS_GM_BUSINESS  1  /* Saturation */
#undef LCS_GM_GRAPHICS
#define LCS_GM_GRAPHICS  2  /* Relative */
#undef LCS_GM_IMAGES
#define LCS_GM_IMAGES  4  /* Perceptual */
#undef LCS_GM_ABS_COLORIMETRIC
#define LCS_GM_ABS_COLORIMETRIC  8  /* Absolute */
#endif

/*
  Enumerated declarations.
*/
typedef enum
{
  UndefinedSubtype,
  RGB555,
  RGB565,
  ARGB4444,
  ARGB1555
} BMPSubtype;

/*
  Typedef declarations.
*/
typedef struct _BMPInfo
{
  unsigned int
    file_size,
    ba_offset,
    offset_bits,
    size;

  ssize_t
    width,
    height;

  unsigned short
    planes,
    bits_per_pixel;

  unsigned int
    compression,
    image_size,
    x_pixels,
    y_pixels,
    number_colors,
    red_mask,
    green_mask,
    blue_mask,
    alpha_mask,
    colors_important;

  long
    colorspace;

  PrimaryInfo
    red_primary,
    green_primary,
    blue_primary,
    gamma_scale;
} BMPInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteBMPImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c o d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecodeImage unpacks the packed image pixels into runlength-encoded pixel
%  packets.
%
%  The format of the DecodeImage method is:
%
%      MagickBooleanType DecodeImage(Image *image,const size_t compression,
%        unsigned char *pixels,const size_t number_pixels)
%
%  A description of each parameter follows:
%
%    o image: the address of a structure of type Image.
%
%    o compression:  Zero means uncompressed.  A value of 1 means the
%      compressed pixels are runlength encoded for a 256-color bitmap.
%      A value of 2 means a 16-color bitmap.  A value of 3 means bitfields
%      encoding.
%
%    o pixels:  The address of a byte (8 bits) array of pixel data created by
%      the decoding process.
%
%    o number_pixels:  The number of pixels.
%
*/
static MagickBooleanType DecodeImage(Image *image,const size_t compression,
  unsigned char *pixels,const size_t number_pixels)
{
  int
    byte,
    count;

  ssize_t
    i,
    x,
    y;

  unsigned char
    *p,
    *q;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(pixels != (unsigned char *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) memset(pixels,0,number_pixels*sizeof(*pixels));
  byte=0;
  x=0;
  p=pixels;
  q=pixels+number_pixels;
  for (y=0; y < (ssize_t) image->rows; )
  {
    MagickBooleanType
      status;

    if ((p < pixels) || (p >= q))
      break;
    count=ReadBlobByte(image);
    if (count == EOF)
      break;
    if (count > 0)
      {
        /*
          Encoded mode.
        */
        count=(int) MagickMin((ssize_t) count,(ssize_t) (q-p));
        byte=ReadBlobByte(image);
        if (byte == EOF)
          break;
        if (compression == BI_RLE8)
          {
            for (i=0; i < (ssize_t) count; i++)
              *p++=(unsigned char) byte;
          }
        else
          {
            for (i=0; i < (ssize_t) count; i++)
              *p++=(unsigned char)
                ((i & 0x01) != 0 ? (byte & 0x0f) : ((byte >> 4) & 0x0f));
          }
        x+=count;
      }
    else
      {
        /*
          Escape mode.
        */
        count=ReadBlobByte(image);
        if (count == EOF)
          break;
        if (count == 0x01)
          break;
        switch (count)
        {
          case 0x00:
          {
            /*
              End of line.
            */
            x=0;
            y++;
            p=pixels+y*(ssize_t) image->columns;
            break;
          }
          case 0x02:
          {
            /*
              Delta mode.
            */
            byte=ReadBlobByte(image);
            if (byte == EOF)
              return(MagickFalse);
            x+=byte;
            byte=ReadBlobByte(image);
            if (byte == EOF)
              return(MagickFalse);
            y+=byte;
            p=pixels+y*(ssize_t) image->columns+x;
            break;
          }
          default:
          {
            /*
              Absolute mode.
            */
            count=(int) MagickMin((ssize_t) count,(ssize_t) (q-p));
            if (count < 0)
              break;
            if (compression == BI_RLE8)
              for (i=0; i < (ssize_t) count; i++)
              {
                byte=ReadBlobByte(image);
                if (byte == EOF)
                  break;
                *p++=(unsigned char) byte;
              }
            else
              for (i=0; i < (ssize_t) count; i++)
              {
                if ((i & 0x01) == 0)
                  {
                    byte=ReadBlobByte(image);
                    if (byte == EOF)
                      break;
                  }
                *p++=(unsigned char)
                  ((i & 0x01) != 0 ? (byte & 0x0f) : ((byte >> 4) & 0x0f));
              }
            x+=count;
            /*
              Read pad byte.
            */
            if (compression == BI_RLE8)
              {
                if ((count & 0x01) != 0)
                  if (ReadBlobByte(image) == EOF)
                    break;
              }
            else
              if (((count & 0x03) == 1) || ((count & 0x03) == 2))
                if (ReadBlobByte(image) == EOF)
                  break;
            break;
          }
        }
      }
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  (void) ReadBlobByte(image);  /* end of line */
  (void) ReadBlobByte(image);
  return((p-pixels) < (ssize_t) number_pixels ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E n c o d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncodeImage compresses pixels using a runlength encoded format.
%
%  The format of the EncodeImage method is:
%
%    static MagickBooleanType EncodeImage(Image *image,
%      const size_t bytes_per_line,const unsigned char *pixels,
%      unsigned char *compressed_pixels)
%
%  A description of each parameter follows:
%
%    o image:  The image.
%
%    o bytes_per_line: the number of bytes in a scanline of compressed pixels
%
%    o pixels:  The address of a byte (8 bits) array of pixel data created by
%      the compression process.
%
%    o compressed_pixels:  The address of a byte (8 bits) array of compressed
%      pixel data.
%
*/
static size_t EncodeImage(Image *image,const size_t bytes_per_line,
  const unsigned char *pixels,unsigned char *compressed_pixels)
{
  MagickBooleanType
    status;

  const unsigned char
    *p;

  ssize_t
    i,
    x;

  unsigned char
    *q;

  ssize_t
    y;

  /*
    Runlength encode pixels.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(pixels != (const unsigned char *) NULL);
  assert(compressed_pixels != (unsigned char *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  p=pixels;
  q=compressed_pixels;
  i=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    for (x=0; x < (ssize_t) bytes_per_line; x+=i)
    {
      /*
        Determine runlength.
      */
      for (i=1; ((x+i) < (ssize_t) bytes_per_line); i++)
        if ((i == 255) || (*(p+i) != *p))
          break;
      *q++=(unsigned char) i;
      *q++=(*p);
      p+=(ptrdiff_t) i;
    }
    /*
      End of line.
    */
    *q++=(unsigned char) 0x00;
    *q++=(unsigned char) 0x00;
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  /*
    End of bitmap.
  */
  *q++=(unsigned char) 0x00;
  *q++=(unsigned char) 0x01;
  return((size_t) (q-compressed_pixels));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s B M P                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsBMP() returns MagickTrue if the image format type, identified by the
%  magick string, is BMP.
%
%  The format of the IsBMP method is:
%
%      MagickBooleanType IsBMP(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsBMP(const unsigned char *magick,const size_t length)
{
  if (length < 2)
    return(MagickFalse);
  if ((LocaleNCompare((char *) magick,"BA",2) == 0) ||
      (LocaleNCompare((char *) magick,"BM",2) == 0) ||
      (LocaleNCompare((char *) magick,"IC",2) == 0) ||
      (LocaleNCompare((char *) magick,"PI",2) == 0) ||
      (LocaleNCompare((char *) magick,"CI",2) == 0) ||
      (LocaleNCompare((char *) magick,"CP",2) == 0))
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d B M P I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBMPImage() reads a Microsoft Windows bitmap image file, Version
%  2, 3 (for Windows or NT), or 4, and  returns it.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the ReadBMPImage method is:
%
%      image=ReadBMPImage(image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline MagickBooleanType BMPOverflowCheck(size_t x,size_t y)
{
  return((y != 0) && (x > 4294967295UL/y) ? MagickTrue : MagickFalse);
}

static Image *ReadEmbedImage(const ImageInfo *image_info,Image *image,
  const char *magick,ExceptionInfo *exception)
{
  const void
    *stream;

  Image
    *embed_image;

  ImageInfo
    *embed_info;

  MemoryInfo
    *pixel_info;

  size_t
    length;

  ssize_t
    count;

  unsigned char
    *pixels;

  /*
    Read embedded image.
  */
  length=(size_t) ((MagickOffsetType) GetBlobSize(image)-TellBlob(image));
  pixel_info=AcquireVirtualMemory(length,sizeof(*pixels));
  if (pixel_info == (MemoryInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return((Image *) NULL);
    }
  pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
  stream=ReadBlobStream(image,length,pixels,&count);
  if (count != (ssize_t) length)
    {
      pixel_info=RelinquishVirtualMemory(pixel_info);
      (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
        "ImproperImageHeader","`%s'",image->filename);
      return((Image *) NULL);
    }
  embed_info=AcquireImageInfo();
  (void) FormatLocaleString(embed_info->filename,MagickPathExtent,
    "%s:%s",magick,image_info->filename);
  embed_image=BlobToImage(embed_info,stream,(size_t) count,exception);
  embed_info=DestroyImageInfo(embed_info);
  pixel_info=RelinquishVirtualMemory(pixel_info);
  if (embed_image != (Image *) NULL)
    {
      (void) CopyMagickString(embed_image->filename,image->filename,
        MagickPathExtent);
      (void) CopyMagickString(embed_image->magick_filename,
        image->magick_filename,MagickPathExtent);
      (void) CopyMagickString(embed_image->magick,image->magick,
        MagickPathExtent);
    }
  return(embed_image);
}

static Image *ReadBMPImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  BMPInfo
    bmp_info;

  Image
    *image;

  MagickBooleanType
    ignore_filesize,
    status;

  MagickOffsetType
    offset,
    profile_data,
    profile_size,
    start_position;

  MagickSizeType
    blob_size;

  MemoryInfo
    *pixel_info;

  Quantum
    index,
    *q;

  size_t
    bit,
    bytes_per_line,
    extent,
    length;

  ssize_t
    count,
    i,
    x,
    y;

  unsigned char
    magick[12],
    *p,
    *pixels;

  unsigned int
    blue,
    green,
    offset_bits,
    red;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  image->columns=0;
  image->rows=0;
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Determine if this a BMP file.
  */
  (void) memset(&bmp_info,0,sizeof(bmp_info));
  bmp_info.ba_offset=0;
  start_position=0;
  offset_bits=0;
  count=ReadBlob(image,2,magick);
  if (count != 2)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  blob_size=GetBlobSize(image);
  do
  {
    PixelInfo
      quantum_bits;

    PixelPacket
      shift;

    /*
      Verify BMP identifier.
    */
    start_position=TellBlob(image)-2;
    bmp_info.ba_offset=0;
    while (LocaleNCompare((char *) magick,"BA",2) == 0)
    {
      bmp_info.file_size=ReadBlobLSBLong(image);
      bmp_info.ba_offset=ReadBlobLSBLong(image);
      bmp_info.offset_bits=ReadBlobLSBLong(image);
      count=ReadBlob(image,2,magick);
      if (count != 2)
        break;
    }
    if (image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  Magick: %c%c",
        magick[0],magick[1]);
    if ((count != 2) || ((LocaleNCompare((char *) magick,"BM",2) != 0) &&
        (LocaleNCompare((char *) magick,"CI",2) != 0)))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    bmp_info.file_size=ReadBlobLSBLong(image);
    (void) ReadBlobLSBLong(image);
    bmp_info.offset_bits=ReadBlobLSBLong(image);
    bmp_info.size=ReadBlobLSBLong(image);
    if (image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  BMP header size: %u",bmp_info.size);
    if (LocaleNCompare((char *) magick,"CI",2) == 0)
      {
        if ((bmp_info.size != 12) && (bmp_info.size != 40) &&
            (bmp_info.size != 64))
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    if (bmp_info.size > 124)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    /*
      Get option to bypass file size check
    */
    ignore_filesize=IsStringTrue(GetImageOption(image_info,
      "bmp:ignore-filesize"));
    if ((ignore_filesize == MagickFalse) && (bmp_info.file_size != 0) &&
        ((MagickSizeType) bmp_info.file_size > GetBlobSize(image)))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if (bmp_info.offset_bits < bmp_info.size)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    bmp_info.offset_bits=MagickMax(14+bmp_info.size,bmp_info.offset_bits);
    profile_data=0;
    profile_size=0;
    if (bmp_info.size == 12)
      {
        /*
          OS/2 BMP image file.
        */
        (void) CopyMagickString(image->magick,"BMP2",MagickPathExtent);
        bmp_info.width=(ssize_t) ((short) ReadBlobLSBShort(image));
        bmp_info.height=(ssize_t) ((short) ReadBlobLSBShort(image));
        bmp_info.planes=ReadBlobLSBShort(image);
        bmp_info.bits_per_pixel=ReadBlobLSBShort(image);
        bmp_info.x_pixels=0;
        bmp_info.y_pixels=0;
        bmp_info.number_colors=0;
        bmp_info.compression=BI_RGB;
        bmp_info.image_size=0;
        bmp_info.alpha_mask=0;
        if (image->debug != MagickFalse)
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Format: OS/2 Bitmap");
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Geometry: %.20gx%.20g",(double) bmp_info.width,(double)
              bmp_info.height);
          }
      }
    else
      {
        /*
          Microsoft Windows BMP image file.
        */
        switch (bmp_info.size)
        {
          case 40: case 52: case 56: case 64: case 78: case 108: case 124:
          {
            break;
          }
          default:
          {
            if (bmp_info.size < 64)
              ThrowReaderException(CorruptImageError,"ImproperImageHeader");
            break;
          }
        }
        bmp_info.width=(ssize_t) ReadBlobLSBSignedLong(image);
        bmp_info.height=(ssize_t) ReadBlobLSBSignedLong(image);
        bmp_info.planes=ReadBlobLSBShort(image);
        bmp_info.bits_per_pixel=ReadBlobLSBShort(image);
        bmp_info.compression=ReadBlobLSBLong(image);
        if (bmp_info.size > 16)
          {
            bmp_info.image_size=ReadBlobLSBLong(image);
            bmp_info.x_pixels=ReadBlobLSBLong(image);
            bmp_info.y_pixels=ReadBlobLSBLong(image);
            bmp_info.number_colors=ReadBlobLSBLong(image);
            bmp_info.colors_important=ReadBlobLSBLong(image);
          }
        if (image->debug != MagickFalse)
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Format: MS Windows bitmap");
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Geometry: %.20gx%.20g",(double) bmp_info.width,(double)
              bmp_info.height);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Bits per pixel: %.20g",(double) bmp_info.bits_per_pixel);
            switch (bmp_info.compression)
            {
              case BI_RGB:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_RGB");
                break;
              }
              case BI_RLE4:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_RLE4");
                break;
              }
              case BI_RLE8:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_RLE8");
                break;
              }
              case BI_BITFIELDS:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_BITFIELDS");
                break;
              }
              case BI_ALPHABITFIELDS:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_ALPHABITFIELDS");
                break;
              }
              case BI_PNG:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_PNG");
                break;
              }
              case BI_JPEG:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: BI_JPEG");
                break;
              }
              default:
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "  Compression: UNKNOWN (%u)",bmp_info.compression);
              }
            }
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "  Number of colors: %u",bmp_info.number_colors);
          }
        if ((bmp_info.height < 0) &&
          (bmp_info.compression != BI_RGB) && (bmp_info.compression != BI_BITFIELDS))
          ThrowReaderException(CoderError,"CompressNotSupported");
        if ((bmp_info.size > 40) || (bmp_info.compression == BI_BITFIELDS) ||
            (bmp_info.compression == BI_ALPHABITFIELDS))

          {
            bmp_info.red_mask=ReadBlobLSBLong(image);
            bmp_info.green_mask=ReadBlobLSBLong(image);
            bmp_info.blue_mask=ReadBlobLSBLong(image);
            if (bmp_info.compression == BI_ALPHABITFIELDS)
              bmp_info.alpha_mask=ReadBlobLSBLong(image);
            if (((bmp_info.size == 40) ||
                 (bmp_info.compression == BI_ALPHABITFIELDS)) &&
                 (bmp_info.bits_per_pixel != 16) &&
                 (bmp_info.bits_per_pixel != 32))
              ThrowReaderException(CorruptImageError,"UnsupportedBitsPerPixel");
          }
        if (bmp_info.size > 40)
          {
            double
              gamma;

            /*
              Read color management information.
            */
            bmp_info.alpha_mask=ReadBlobLSBLong(image);
            bmp_info.colorspace=ReadBlobLSBSignedLong(image);
            /*
              Decode 2^30 fixed point formatted CIE primaries.
            */
#           define BMP_DENOM ((double) 0x40000000)
            bmp_info.red_primary.x=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.red_primary.y=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.red_primary.z=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.green_primary.x=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.green_primary.y=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.green_primary.z=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.blue_primary.x=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.blue_primary.y=(double) ReadBlobLSBLong(image)/BMP_DENOM;
            bmp_info.blue_primary.z=(double) ReadBlobLSBLong(image)/BMP_DENOM;

            gamma=bmp_info.red_primary.x+bmp_info.red_primary.y+
              bmp_info.red_primary.z;
            gamma=MagickSafeReciprocal(gamma);
            bmp_info.red_primary.x*=gamma;
            bmp_info.red_primary.y*=gamma;

            gamma=bmp_info.green_primary.x+bmp_info.green_primary.y+
              bmp_info.green_primary.z;
            gamma=MagickSafeReciprocal(gamma);
            bmp_info.green_primary.x*=gamma;
            bmp_info.green_primary.y*=gamma;

            gamma=bmp_info.blue_primary.x+bmp_info.blue_primary.y+
              bmp_info.blue_primary.z;
            gamma=MagickSafeReciprocal(gamma);
            bmp_info.blue_primary.x*=gamma;
            bmp_info.blue_primary.y*=gamma;

            /*
              Decode 16^16 fixed point formatted gamma_scales.
            */
            bmp_info.gamma_scale.x=(double) ReadBlobLSBLong(image)/0x10000;
            bmp_info.gamma_scale.y=(double) ReadBlobLSBLong(image)/0x10000;
            bmp_info.gamma_scale.z=(double) ReadBlobLSBLong(image)/0x10000;

            if (bmp_info.colorspace == 0)
              {
                image->chromaticity.red_primary.x=bmp_info.red_primary.x;
                image->chromaticity.red_primary.y=bmp_info.red_primary.y;
                image->chromaticity.green_primary.x=bmp_info.green_primary.x;
                image->chromaticity.green_primary.y=bmp_info.green_primary.y;
                image->chromaticity.blue_primary.x=bmp_info.blue_primary.x;
                image->chromaticity.blue_primary.y=bmp_info.blue_primary.y;
                /*
                  Compute a single gamma from the BMP 3-channel gamma.
                */
                image->gamma=(bmp_info.gamma_scale.x+bmp_info.gamma_scale.y+
                  bmp_info.gamma_scale.z)/3.0;
              }
          }
        else
          (void) CopyMagickString(image->magick,"BMP3",MagickPathExtent);
        if (bmp_info.size > 108)
          {
            size_t
              intent;

            /*
              Read BMP Version 5 color management information.
            */
            intent=ReadBlobLSBLong(image);
            switch ((int) intent)
            {
              case LCS_GM_BUSINESS:
              {
                image->rendering_intent=SaturationIntent;
                break;
              }
              case LCS_GM_GRAPHICS:
              {
                image->rendering_intent=RelativeIntent;
                break;
              }
              case LCS_GM_IMAGES:
              {
                image->rendering_intent=PerceptualIntent;
                break;
              }
              case LCS_GM_ABS_COLORIMETRIC:
              {
                image->rendering_intent=AbsoluteIntent;
                break;
              }
            }
            profile_data=(MagickOffsetType) ReadBlobLSBLong(image);
            profile_size=(MagickOffsetType) ReadBlobLSBLong(image);
            (void) ReadBlobLSBLong(image);  /* Reserved byte */
          }
      }
    if ((ignore_filesize == MagickFalse) &&
        (MagickSizeType) bmp_info.file_size != blob_size)
      (void) ThrowMagickException(exception,GetMagickModule(),
        CorruptImageError,"LengthAndFilesizeDoNotMatch","`%s'",
        image->filename);
    if (bmp_info.width <= 0)
      ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
    if (bmp_info.height == 0)
      ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
    if (bmp_info.compression == BI_JPEG)
      {
        /*
          Read embedded JPEG image.
        */
        Image *embed_image = ReadEmbedImage(image_info,image,"jpeg",exception);
        (void) CloseBlob(image);
        image=DestroyImageList(image);
        return(embed_image);
      }
    if (bmp_info.compression == BI_PNG)
      {
        /*
          Read embedded PNG image.
        */
        Image *embed_image = ReadEmbedImage(image_info,image,"png",exception);
        (void) CloseBlob(image);
        image=DestroyImageList(image);
        return(embed_image);
      }
    if (bmp_info.planes != 1)
      ThrowReaderException(CorruptImageError,"StaticPlanesValueNotEqualToOne");
    switch (bmp_info.bits_per_pixel)
    {
      case 1: case 4: case 8: case 16: case 24: case 32: case 64:
      {
        break;
      }
      default:
        ThrowReaderException(CorruptImageError,"UnsupportedBitsPerPixel");
    }
    if ((bmp_info.bits_per_pixel < 16) &&
        (bmp_info.number_colors > (1U << bmp_info.bits_per_pixel)))
      ThrowReaderException(CorruptImageError,"UnrecognizedNumberOfColors");
    if ((MagickSizeType) bmp_info.number_colors > blob_size)
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    if ((bmp_info.compression == BI_RLE8) && (bmp_info.bits_per_pixel != 8))
      ThrowReaderException(CorruptImageError,"UnsupportedBitsPerPixel");
    if ((bmp_info.compression == BI_RLE4) && (bmp_info.bits_per_pixel != 4))
      ThrowReaderException(CorruptImageError,"UnsupportedBitsPerPixel");
    if ((bmp_info.compression == BI_BITFIELDS) &&
        (bmp_info.bits_per_pixel < 16))
      ThrowReaderException(CorruptImageError,"UnsupportedBitsPerPixel");
    switch (bmp_info.compression)
    {
      case BI_RGB:
        image->compression=NoCompression;
        break;
      case BI_RLE8:
      case BI_RLE4:
        image->compression=RLECompression;
        break;
      case BI_BITFIELDS:
        break;
      case BI_ALPHABITFIELDS:
        break;
      case BI_JPEG:
        ThrowReaderException(CoderError,"JPEGCompressNotSupported");
      case BI_PNG:
        ThrowReaderException(CoderError,"PNGCompressNotSupported");
      default:
        ThrowReaderException(CorruptImageError,"UnrecognizedImageCompression");
    }
    image->columns=(size_t) MagickAbsoluteValue(bmp_info.width);
    image->rows=(size_t) MagickAbsoluteValue(bmp_info.height);
    image->depth=bmp_info.bits_per_pixel <= 8 ? bmp_info.bits_per_pixel : 8;
    image->alpha_trait=((bmp_info.alpha_mask != 0) &&
      ((bmp_info.compression == BI_BITFIELDS) || 
       (bmp_info.compression == BI_ALPHABITFIELDS))) ? BlendPixelTrait :
       UndefinedPixelTrait;
    if (bmp_info.bits_per_pixel < 16)
      {
        size_t
          one;

        image->storage_class=PseudoClass;
        image->colors=bmp_info.number_colors;
        one=1;
        if (image->colors == 0)
          image->colors=one << bmp_info.bits_per_pixel;
      }
    image->resolution.x=(double) bmp_info.x_pixels/100.0;
    image->resolution.y=(double) bmp_info.y_pixels/100.0;
    image->units=PixelsPerCentimeterResolution;
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    if (image->storage_class == PseudoClass)
      {
        unsigned char
          *bmp_colormap;

        size_t
          packet_size;

        /*
          Read BMP raster colormap.
        */
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Reading colormap of %.20g colors",(double) image->colors);
        if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        bmp_colormap=(unsigned char *) AcquireQuantumMemory((size_t)
          image->colors,4*sizeof(*bmp_colormap));
        if (bmp_colormap == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        if ((bmp_info.size == 12) || (bmp_info.size == 64))
          packet_size=3;
        else
          packet_size=4;
        offset=SeekBlob(image,start_position+14+bmp_info.size,SEEK_SET);
        if (offset < 0)
          {
            bmp_colormap=(unsigned char *) RelinquishMagickMemory(bmp_colormap);
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          }
        count=ReadBlob(image,packet_size*image->colors,bmp_colormap);
        if (count != (ssize_t) (packet_size*image->colors))
          {
            bmp_colormap=(unsigned char *) RelinquishMagickMemory(bmp_colormap);
            ThrowReaderException(CorruptImageError,
              "InsufficientImageDataInFile");
          }
        p=bmp_colormap;
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          image->colormap[i].blue=(MagickRealType) ScaleCharToQuantum(*p++);
          image->colormap[i].green=(MagickRealType) ScaleCharToQuantum(*p++);
          image->colormap[i].red=(MagickRealType) ScaleCharToQuantum(*p++);
          if (packet_size == 4)
            p++;
        }
        bmp_colormap=(unsigned char *) RelinquishMagickMemory(bmp_colormap);
      }
    /*
      Read image data.
    */
    if (bmp_info.offset_bits == offset_bits)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    offset_bits=bmp_info.offset_bits;
    offset=SeekBlob(image,start_position+bmp_info.offset_bits,SEEK_SET);
    if (offset < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if (bmp_info.compression == BI_RLE4)
      bmp_info.bits_per_pixel<<=1;
		extent=image->columns*bmp_info.bits_per_pixel;
		bytes_per_line=4*((extent+31)/32);
    if (BMPOverflowCheck(bytes_per_line,image->rows) != MagickFalse)
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    length=bytes_per_line*image->rows;
    if ((MagickSizeType) (length/256) > blob_size)
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    extent=MagickMax(bytes_per_line,image->columns+1UL);
    if ((BMPOverflowCheck(image->rows,extent) != MagickFalse) ||
        (BMPOverflowCheck(extent,sizeof(*pixels)) != MagickFalse))
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    pixel_info=AcquireVirtualMemory(image->rows,extent*sizeof(*pixels));
    if (pixel_info == (MemoryInfo *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    if ((bmp_info.compression == BI_RGB) ||
        (bmp_info.compression == BI_BITFIELDS) ||
        (bmp_info.compression == BI_ALPHABITFIELDS))
      {
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Reading pixels (%.20g bytes)",(double) length);
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(CorruptImageError,
              "InsufficientImageDataInFile");
          }
      }
    else
      {
        /*
          Convert run-length encoded raster pixels.
        */
        status=DecodeImage(image,bmp_info.compression,pixels,
          image->columns*image->rows);
        if (status == MagickFalse)
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(CorruptImageError,
              "UnableToRunlengthDecodeImage");
          }
      }
    /*
      Convert BMP raster image to pixel packets.
    */
    if (bmp_info.compression == BI_RGB)
      {
        /*
          We should ignore the alpha value in BMP3 files but there have been
          reports about 32 bit files with alpha. We do a quick check to see if
          the alpha channel contains a value that is not zero (default value).
          If we find a non zero value we assume the program that wrote the file
          wants to use the alpha channel.
        */
        if (((image->alpha_trait & BlendPixelTrait) == 0) &&
            (bmp_info.size == 40) && (bmp_info.bits_per_pixel == 32))
          {
            bytes_per_line=4*(image->columns);
            for (y=(ssize_t) image->rows-1; y >= 0; y--)
            {
              p=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                if (*(p+3) != 0)
                  {
                    image->alpha_trait=BlendPixelTrait;
                    y=-1;
                    break;
                  }
                p+=(ptrdiff_t) 4;
              }
            }
          }
        bmp_info.alpha_mask=image->alpha_trait != UndefinedPixelTrait ?
          0xff000000U : 0U;
        bmp_info.red_mask=0x00ff0000U;
        bmp_info.green_mask=0x0000ff00U;
        bmp_info.blue_mask=0x000000ffU;
        if (bmp_info.bits_per_pixel == 16)
          {
            /*
              RGB555.
            */
            bmp_info.red_mask=0x00007c00U;
            bmp_info.green_mask=0x000003e0U;
            bmp_info.blue_mask=0x0000001fU;
          }
      }
    (void) memset(&shift,0,sizeof(shift));
    (void) memset(&quantum_bits,0,sizeof(quantum_bits));
    if ((bmp_info.bits_per_pixel == 16) || (bmp_info.bits_per_pixel == 32))
      {
        unsigned int
          sample;

        /*
          Get shift and quantum bits info from bitfield masks.
        */
        if (bmp_info.red_mask != 0)
          while (((bmp_info.red_mask << shift.red) & 0x80000000UL) == 0)
          {
            shift.red++;
            if (shift.red >= 32U)
              break;
          }
        if (bmp_info.green_mask != 0)
          while (((bmp_info.green_mask << shift.green) & 0x80000000UL) == 0)
          {
            shift.green++;
            if (shift.green >= 32U)
              break;
          }
        if (bmp_info.blue_mask != 0)
          while (((bmp_info.blue_mask << shift.blue) & 0x80000000UL) == 0)
          {
            shift.blue++;
            if (shift.blue >= 32U)
              break;
          }
        if (bmp_info.alpha_mask != 0)
          while (((bmp_info.alpha_mask << shift.alpha) & 0x80000000UL) == 0)
          {
            shift.alpha++;
            if (shift.alpha >= 32U)
              break;
          }
        sample=shift.red;
        while (((bmp_info.red_mask << sample) & 0x80000000UL) != 0)
        {
          sample++;
          if (sample >= 32U)
            break;
        }
        quantum_bits.red=(MagickRealType) (sample-shift.red);
        sample=shift.green;
        while (((bmp_info.green_mask << sample) & 0x80000000UL) != 0)
        {
          sample++;
          if (sample >= 32U)
            break;
        }
        quantum_bits.green=(MagickRealType) (sample-shift.green);
        sample=shift.blue;
        while (((bmp_info.blue_mask << sample) & 0x80000000UL) != 0)
        {
          sample++;
          if (sample >= 32U)
            break;
        }
        quantum_bits.blue=(MagickRealType) (sample-shift.blue);
        sample=shift.alpha;
        while (((bmp_info.alpha_mask << sample) & 0x80000000UL) != 0)
        {
          sample++;
          if (sample >= 32U)
            break;
        }
        quantum_bits.alpha=(MagickRealType) (sample-shift.alpha);
      }
    switch (bmp_info.bits_per_pixel)
    {
      case 1:
      {
        /*
          Convert bitmap scanline.
        */
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < ((ssize_t) image->columns-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
            {
              index=(Quantum) (((*p) & (0x80 >> bit)) != 0 ? 0x01 : 0x00);
              SetPixelIndex(image,index,q);
              q+=(ptrdiff_t) GetPixelChannels(image);
            }
            p++;
          }
          if ((image->columns % 8) != 0)
            {
              for (bit=0; bit < (image->columns % 8); bit++)
              {
                index=(Quantum) (((*p) & (0x80 >> bit)) != 0 ? 0x01 : 0x00);
                SetPixelIndex(image,index,q);
                q+=(ptrdiff_t) GetPixelChannels(image);
              }
              p++;
            }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,((MagickOffsetType)
                image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        (void) SyncImage(image,exception);
        break;
      }
      case 4:
      {
        /*
          Convert PseudoColor scanline.
        */
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < ((ssize_t) image->columns-1); x+=2)
          {
            ValidateColormapValue(image,(ssize_t) ((*p >> 4) & 0x0f),&index,
              exception);
            SetPixelIndex(image,index,q);
            q+=(ptrdiff_t) GetPixelChannels(image);
            ValidateColormapValue(image,(ssize_t) (*p & 0x0f),&index,exception);
            SetPixelIndex(image,index,q);
            q+=(ptrdiff_t) GetPixelChannels(image);
            p++;
          }
          if ((image->columns % 2) != 0)
            {
              ValidateColormapValue(image,(ssize_t) ((*p >> 4) & 0xf),&index,
                exception);
              SetPixelIndex(image,index,q);
              q+=(ptrdiff_t) GetPixelChannels(image);
              p++;
              x++;
            }
          if (x < (ssize_t) image->columns)
            break;
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,((MagickOffsetType)
                image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        (void) SyncImage(image,exception);
        break;
      }
      case 8:
      {
        /*
          Convert PseudoColor scanline.
        */
        if ((bmp_info.compression == BI_RLE8) ||
            (bmp_info.compression == BI_RLE4))
          bytes_per_line=image->columns;
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=(ssize_t) image->columns; x != 0; --x)
          {
            ValidateColormapValue(image,(ssize_t) *p++,&index,exception);
            SetPixelIndex(image,index,q);
            q+=(ptrdiff_t) GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          offset=((MagickOffsetType) image->rows-y-1);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,((MagickOffsetType)
                image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        (void) SyncImage(image,exception);
        break;
      }
      case 16:
      {
        unsigned int
          alpha,
          pixel;

        /*
          Convert bitfield encoded 16-bit PseudoColor scanline.
        */
        if ((bmp_info.compression != BI_RGB) &&
            (bmp_info.compression != BI_BITFIELDS))
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(CorruptImageError,
              "UnrecognizedImageCompression");
          }
        bytes_per_line=2*(image->columns+image->columns % 2);
        image->storage_class=DirectClass;
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            pixel=(unsigned int) (*p++);
            pixel|=(unsigned int) (*p++) << 8;
            red=((pixel & bmp_info.red_mask) << shift.red) >> 16;
            if (quantum_bits.red == 5)
              red|=((red & 0xff00) >> 5);
            if (quantum_bits.red <= 8)
              red|=((red & 0xff00) >> 8);
            green=((pixel & bmp_info.green_mask) << shift.green) >> 16;
            if (quantum_bits.green == 5)
              green|=((green & 0xff00) >> 5);
            if (quantum_bits.green == 6)
              green|=((green & 0xff00) >> 6);
            if (quantum_bits.green <= 8)
              green|=((green & 0xff00) >> 8);
            blue=((pixel & bmp_info.blue_mask) << shift.blue) >> 16;
            if (quantum_bits.blue == 5)
              blue|=((blue & 0xff00) >> 5);
            if (quantum_bits.blue <= 8)
              blue|=((blue & 0xff00) >> 8);
            SetPixelRed(image,ScaleShortToQuantum((unsigned short) red),q);
            SetPixelGreen(image,ScaleShortToQuantum((unsigned short) green),q);
            SetPixelBlue(image,ScaleShortToQuantum((unsigned short) blue),q);
            SetPixelAlpha(image,OpaqueAlpha,q);
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                alpha=((pixel & bmp_info.alpha_mask) << shift.alpha) >> 16;
                if (quantum_bits.alpha <= 8)
                  alpha|=((alpha & 0xff00) >> 8);
                SetPixelAlpha(image,ScaleShortToQuantum((unsigned short)
                  alpha),q);
              }
            q+=(ptrdiff_t) GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          offset=((MagickOffsetType) image->rows-y-1);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,((MagickOffsetType)
                image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 24:
      {
        /*
          Convert DirectColor scanline.
        */
        bytes_per_line=4*((image->columns*24+31)/32);
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
            SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
            SetPixelRed(image,ScaleCharToQuantum(*p++),q);
            SetPixelAlpha(image,OpaqueAlpha,q);
            q+=(ptrdiff_t) GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          offset=((MagickOffsetType) image->rows-y-1);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,((MagickOffsetType)
                image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 32:
      {
        /*
          Convert bitfield encoded DirectColor scanline.
        */
        if ((bmp_info.compression != BI_RGB) &&
            (bmp_info.compression != BI_BITFIELDS))
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(CorruptImageError,
              "UnrecognizedImageCompression");
          }
        bytes_per_line=4*(image->columns);
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          unsigned int
            alpha,
            pixel;

          p=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            pixel=(unsigned int) (*p++);
            pixel|=((unsigned int) *p++ << 8);
            pixel|=((unsigned int) *p++ << 16);
            pixel|=((unsigned int) *p++ << 24);
            red=((pixel & bmp_info.red_mask) << shift.red) >> 16;
            if (quantum_bits.red == 8)
              red|=(red >> 8);
            green=((pixel & bmp_info.green_mask) << shift.green) >> 16;
            if (quantum_bits.green == 8)
              green|=(green >> 8);
            blue=((pixel & bmp_info.blue_mask) << shift.blue) >> 16;
            if (quantum_bits.blue == 8)
              blue|=(blue >> 8);
            SetPixelRed(image,ScaleShortToQuantum((unsigned short) red),q);
            SetPixelGreen(image,ScaleShortToQuantum((unsigned short) green),q);
            SetPixelBlue(image,ScaleShortToQuantum((unsigned short) blue),q);
            SetPixelAlpha(image,OpaqueAlpha,q);
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                alpha=((pixel & bmp_info.alpha_mask) << shift.alpha) >> 16;
                if (quantum_bits.alpha == 8)
                  alpha|=(alpha >> 8);
                SetPixelAlpha(image,ScaleShortToQuantum(
                  (unsigned short) alpha),q);
              }
            q+=(ptrdiff_t) GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          offset=((MagickOffsetType) image->rows-y-1);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,((MagickOffsetType)
                image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 64:
      {
        /*
          Convert DirectColor scanline.
        */
        bytes_per_line=4*((image->columns*64+31)/32);
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          unsigned short
            *p16;

          p16=(unsigned short *) (pixels+((ssize_t) image->rows-y-1)*
            (ssize_t) bytes_per_line);
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelBlue(image,ScaleShortToQuantum(*p16++),q);
            SetPixelGreen(image,ScaleShortToQuantum(*p16++),q);
            SetPixelRed(image,ScaleShortToQuantum(*p16++),q);
            SetPixelAlpha(image,ScaleShortToQuantum(*p16++),q);
            q+=(ptrdiff_t) GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          offset=((MagickOffsetType) image->rows-y-1);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,((MagickOffsetType)
                image->rows-y),image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      default:
      {
        pixel_info=RelinquishVirtualMemory(pixel_info);
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    }
    pixel_info=RelinquishVirtualMemory(pixel_info);
    if (y > 0)
      break;
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    if (bmp_info.height < 0)
      {
        Image
          *flipped_image;

        /*
          Correct image orientation.
        */
        flipped_image=FlipImage(image,exception);
        if (flipped_image != (Image *) NULL)
          {
            DuplicateBlob(flipped_image,image);
            ReplaceImageInList(&image, flipped_image);
            image=flipped_image;
          }
      }
    /*
      Read embedded ICC profile
    */
    if ((bmp_info.colorspace == 0x4D424544L) && (profile_data > 0) &&
        (profile_size > 0))
      {
        StringInfo
          *profile;

        unsigned char
          *datum;

        offset=start_position+14+profile_data;
        if ((offset < TellBlob(image)) ||
            (SeekBlob(image,offset,SEEK_SET) != offset) ||
            (blob_size < (MagickSizeType) (offset+profile_size)))
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        profile=AcquireProfileStringInfo("icc",(size_t) profile_size,
          exception);
        if (profile == (StringInfo *) NULL)
          (void) SeekBlob(image,profile_size,SEEK_CUR);
        else
          {
            datum=GetStringInfoDatum(profile);
            if (ReadBlob(image,(size_t) profile_size,datum) != (ssize_t) profile_size)
              profile=DestroyStringInfo(profile);
            else
              {
                MagickOffsetType
                  profile_size_orig;

                /*
                 Trimming padded bytes.
                */
                profile_size_orig=(MagickOffsetType) datum[0] << 24;
                profile_size_orig|=(MagickOffsetType) datum[1] << 16;
                profile_size_orig|=(MagickOffsetType) datum[2] << 8;
                profile_size_orig|=(MagickOffsetType) datum[3];
                if (profile_size_orig < profile_size)
                  SetStringInfoLength(profile,(size_t) profile_size_orig);
                if (image->debug != MagickFalse)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "Profile: ICC, %u bytes",(unsigned int) profile_size_orig);
                (void) SetImageProfilePrivate(image,profile,exception);
              }
          }
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    offset=(MagickOffsetType) bmp_info.ba_offset;
    if (offset != 0)
      if ((offset < TellBlob(image)) ||
          (SeekBlob(image,offset,SEEK_SET) != offset))
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    *magick='\0';
    count=ReadBlob(image,2,magick);
    if ((count == 2) && (IsBMP(magick,2) != MagickFalse))
      {
        /*
          Acquire next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            status=MagickFalse;
            break;
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),blob_size);
        if (status == MagickFalse)
          break;
      }
  } while (IsBMP(magick,2) != MagickFalse);
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
%   R e g i s t e r B M P I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterBMPImage() adds attributes for the BMP image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterBMPImage method is:
%
%      size_t RegisterBMPImage(void)
%
*/
ModuleExport size_t RegisterBMPImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("BMP","BMP","Microsoft Windows bitmap image");
  entry->decoder=(DecodeImageHandler *) ReadBMPImage;
  entry->encoder=(EncodeImageHandler *) WriteBMPImage;
  entry->magick=(IsImageFormatHandler *) IsBMP;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->mime_type=ConstantString("image/bmp");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("BMP","BMP2","Microsoft Windows bitmap image (V2)");
  entry->decoder=(DecodeImageHandler *) ReadBMPImage;
  entry->encoder=(EncodeImageHandler *) WriteBMPImage;
  entry->magick=(IsImageFormatHandler *) IsBMP;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->mime_type=ConstantString("image/bmp");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("BMP","BMP3","Microsoft Windows bitmap image (V3)");
  entry->decoder=(DecodeImageHandler *) ReadBMPImage;
  entry->encoder=(EncodeImageHandler *) WriteBMPImage;
  entry->magick=(IsImageFormatHandler *) IsBMP;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->mime_type=ConstantString("image/bmp");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r B M P I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterBMPImage() removes format registrations made by the
%  BMP module from the list of supported formats.
%
%  The format of the UnregisterBMPImage method is:
%
%      UnregisterBMPImage(void)
%
*/
ModuleExport void UnregisterBMPImage(void)
{
  (void) UnregisterMagickInfo("BMP");
  (void) UnregisterMagickInfo("BMP2");
  (void) UnregisterMagickInfo("BMP3");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e B M P I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBMPImage() writes an image in Microsoft Windows bitmap encoded
%  image format, version 3 for Windows or (if the image has a matte channel)
%  version 4.
%
%  The format of the WriteBMPImage method is:
%
%      MagickBooleanType WriteBMPImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteBMPImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  BMPInfo
    bmp_info;

  BMPSubtype
    bmp_subtype;

  const char
    *option;

  const Quantum
    *p;

  const StringInfo
    *profile = (const StringInfo *) NULL;

  MagickBooleanType
    have_color_info,
    status;

  MagickOffsetType
    profile_data,
    profile_size,
    profile_size_pad,
    scene;

  MemoryInfo
    *pixel_info;

  size_t
    bytes_per_line,
    extent,
    number_scenes,
    type;

  ssize_t
    i,
    x,
    y;

  unsigned char
    *bmp_data,
    *pixels,
    *q;

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
  if (((image->columns << 3) != (size_t) ((int) (image->columns << 3))) ||
      ((image->rows << 3) != (size_t) ((int) (image->rows << 3))))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  type=4;
  if (LocaleCompare(image_info->magick,"BMP2") == 0)
    type=2;
  else
    if (LocaleCompare(image_info->magick,"BMP3") == 0)
      type=3;
  option=GetImageOption(image_info,"bmp:format");
  if (option != (char *) NULL)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "  Format=%s",option);
      if (LocaleCompare(option,"bmp2") == 0)
        type=2;
      if (LocaleCompare(option,"bmp3") == 0)
        type=3;
      if (LocaleCompare(option,"bmp4") == 0)
        type=4;
    }
  scene=0;
  number_scenes=GetImageListLength(image);
  do
  {
    /*
      Initialize BMP raster file header.
    */
    if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
      (void) TransformImageColorspace(image,sRGBColorspace,exception);
    (void) memset(&bmp_info,0,sizeof(bmp_info));
    bmp_info.file_size=14+12;
    if (type > 2)
      bmp_info.file_size+=28;
    bmp_info.offset_bits=bmp_info.file_size;
    bmp_info.compression=BI_RGB;
    bmp_info.red_mask=0x00ff0000U;
    bmp_info.green_mask=0x0000ff00U;
    bmp_info.blue_mask=0x000000ffU;
    bmp_info.alpha_mask=0xff000000U;
    bmp_subtype=UndefinedSubtype;
    if ((image->storage_class == PseudoClass) && (image->colors > 256))
      (void) SetImageStorageClass(image,DirectClass,exception);
    if (image->storage_class != DirectClass)
      {
        /*
          Colormapped BMP raster.
        */
        bmp_info.bits_per_pixel=8;
        if (image->colors <= 2)
          bmp_info.bits_per_pixel=1;
        else
          if (image->colors <= 16)
            bmp_info.bits_per_pixel=4;
          else
            if (image->colors <= 256)
              bmp_info.bits_per_pixel=8;
        if (image_info->compression == RLECompression)
          bmp_info.bits_per_pixel=8;
        bmp_info.number_colors=1U << bmp_info.bits_per_pixel;
        if (image->alpha_trait != UndefinedPixelTrait)
          (void) SetImageStorageClass(image,DirectClass,exception);
        else
          if ((size_t) bmp_info.number_colors < image->colors)
            (void) SetImageStorageClass(image,DirectClass,exception);
          else
            {
              bmp_info.file_size+=3*(1UL << bmp_info.bits_per_pixel);
              bmp_info.offset_bits+=3*(1UL << bmp_info.bits_per_pixel);
              if (type > 2)
                {
                  bmp_info.file_size+=(1UL << bmp_info.bits_per_pixel);
                  bmp_info.offset_bits+=(1UL << bmp_info.bits_per_pixel);
                }
            }
      }
    if (image->storage_class == DirectClass)
      {
        /*
          Full color BMP raster.
        */
        bmp_info.number_colors=0;
        option=GetImageOption(image_info,"bmp:subtype");
        if (option != (const char *) NULL)
        {
          if (image->alpha_trait != UndefinedPixelTrait)
            {
              if (LocaleNCompare(option,"ARGB4444",8) == 0)
                {
                  bmp_subtype=ARGB4444;
                  bmp_info.red_mask=0x00000f00U;
                  bmp_info.green_mask=0x000000f0U;
                  bmp_info.blue_mask=0x0000000fU;
                  bmp_info.alpha_mask=0x0000f000U;
                }
              else if (LocaleNCompare(option,"ARGB1555",8) == 0)
                {
                  bmp_subtype=ARGB1555;
                  bmp_info.red_mask=0x00007c00U;
                  bmp_info.green_mask=0x000003e0U;
                  bmp_info.blue_mask=0x0000001fU;
                  bmp_info.alpha_mask=0x00008000U;
                }
            }
          else
          {
            if (LocaleNCompare(option,"RGB555",6) == 0)
              {
                bmp_subtype=RGB555;
                bmp_info.red_mask=0x00007c00U;
                bmp_info.green_mask=0x000003e0U;
                bmp_info.blue_mask=0x0000001fU;
                bmp_info.alpha_mask=0U;
              }
            else if (LocaleNCompare(option,"RGB565",6) == 0)
              {
                bmp_subtype=RGB565;
                bmp_info.red_mask=0x0000f800U;
                bmp_info.green_mask=0x000007e0U;
                bmp_info.blue_mask=0x0000001fU;
                bmp_info.alpha_mask=0U;
              }
          }
        }
        if (bmp_subtype != UndefinedSubtype)
          {
            bmp_info.bits_per_pixel=16;
            bmp_info.compression=BI_BITFIELDS;
          }
        else
          {
            bmp_info.bits_per_pixel=(unsigned short) ((type > 3) &&
               (image->alpha_trait != UndefinedPixelTrait) ? 32 : 24);
            bmp_info.compression=(unsigned int) ((type > 3) &&
              (image->alpha_trait != UndefinedPixelTrait) ? BI_BITFIELDS : BI_RGB);
            if ((type == 3) && (image->alpha_trait != UndefinedPixelTrait))
              {
                option=GetImageOption(image_info,"bmp3:alpha");
                if (IsStringTrue(option))
                  bmp_info.bits_per_pixel=32;
              }
          }
      }
    extent=image->columns*(size_t) bmp_info.bits_per_pixel;
    bytes_per_line=4*((extent+31)/32);
    if (BMPOverflowCheck(bytes_per_line,image->rows) != MagickFalse)
      ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
    bmp_info.ba_offset=0;
    if (type > 3)
      profile=GetImageProfile(image,"icc");
    have_color_info=(image->rendering_intent != UndefinedIntent) ||
      (profile != (StringInfo *) NULL) || (image->gamma != 0.0) ?  MagickTrue :
      MagickFalse;
    if (type == 2)
      bmp_info.size=12;
    else
      if ((type == 3) || (((image->alpha_trait & BlendPixelTrait) == 0) &&
          (have_color_info == MagickFalse)))
        {
          type=3;
          bmp_info.size=40;
        }
      else
        {
          int
            extra_size;

          bmp_info.size=108;
          extra_size=68;
          if ((image->rendering_intent != UndefinedIntent) ||
              (profile != (StringInfo *) NULL))
            {
              bmp_info.size=124;
              extra_size+=16;
            }
          bmp_info.file_size+=(unsigned int) extra_size;
          bmp_info.offset_bits+=(unsigned int) extra_size;
        }
    if (((ssize_t) image->columns != (ssize_t) ((signed int) image->columns)) ||
        ((ssize_t) image->rows != (ssize_t) ((signed int) image->rows)))
      ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
    bmp_info.width=(ssize_t) image->columns;
    bmp_info.height=(ssize_t) image->rows;
    bmp_info.planes=1;
    bmp_info.image_size=(unsigned int) (bytes_per_line*image->rows);
    bmp_info.file_size+=bmp_info.image_size;
    bmp_info.x_pixels=75*39;
    bmp_info.y_pixels=75*39;
    switch (image->units)
    {
      case UndefinedResolution:
      case PixelsPerInchResolution:
      {
        bmp_info.x_pixels=CastDoubleToUInt(100.0*image->resolution.x/2.54);
        bmp_info.y_pixels=CastDoubleToUInt(100.0*image->resolution.y/2.54);
        break;
      }
      case PixelsPerCentimeterResolution:
      {
        bmp_info.x_pixels=CastDoubleToUInt(100.0*image->resolution.x);
        bmp_info.y_pixels=CastDoubleToUInt(100.0*image->resolution.y);
        break;
      }
    }
    bmp_info.colors_important=bmp_info.number_colors;
    /*
      Convert MIFF to BMP raster pixels.
    */
    extent=MagickMax(bytes_per_line,image->columns+1UL)*
      ((bmp_info.bits_per_pixel+7)/8);
    if ((BMPOverflowCheck(image->rows,extent) != MagickFalse) ||
        (BMPOverflowCheck(extent,sizeof(*pixels)) != MagickFalse))
      ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
    pixel_info=AcquireVirtualMemory(image->rows,extent*sizeof(*pixels));
    if (pixel_info == (MemoryInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    (void) memset(pixels,0,(size_t) bmp_info.image_size);
    switch (bmp_info.bits_per_pixel)
    {
      case 1:
      {
        size_t
          bit,
          byte;

        /*
          Convert PseudoClass image to a BMP monochrome image.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          ssize_t
            offset;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          q=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          bit=0;
          byte=0;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            byte<<=1;
            byte|=(size_t) (GetPixelIndex(image,p) != 0 ? 0x01 : 0x00);
            bit++;
            if (bit == 8)
              {
                *q++=(unsigned char) byte;
                bit=0;
                byte=0;
              }
             p+=(ptrdiff_t) GetPixelChannels(image);
           }
           if (bit != 0)
             {
               *q++=(unsigned char) (byte << (8-bit));
               x++;
             }
          offset=(ssize_t) (image->columns+7)/8;
          for (x=offset; x < (ssize_t) bytes_per_line; x++)
            *q++=0x00;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 4:
      {
        unsigned int
          byte,
          nibble;

        ssize_t
          offset;

        /*
          Convert PseudoClass image to a BMP monochrome image.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          q=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          nibble=0;
          byte=0;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            byte<<=4;
            byte|=((unsigned int) GetPixelIndex(image,p) & 0x0f);
            nibble++;
            if (nibble == 2)
              {
                *q++=(unsigned char) byte;
                nibble=0;
                byte=0;
              }
            p+=(ptrdiff_t) GetPixelChannels(image);
          }
          if (nibble != 0)
            {
              *q++=(unsigned char) (byte << 4);
              x++;
            }
          offset=(ssize_t) (image->columns+1)/2;
          for (x=offset; x < (ssize_t) bytes_per_line; x++)
            *q++=0x00;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 8:
      {
        /*
          Convert PseudoClass packet to BMP pixel.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          q=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *q++=(unsigned char) ((ssize_t) GetPixelIndex(image,p));
            p+=(ptrdiff_t) GetPixelChannels(image);
          }
          for ( ; x < (ssize_t) bytes_per_line; x++)
            *q++=0x00;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 16:
      {
        /*
          Convert DirectClass packet to BMP BGR888.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          q=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            unsigned short
              pixel;

            pixel=0;
            if (bmp_subtype == ARGB4444)
              {
                pixel=(unsigned short) (ScaleQuantumToAny(
                  GetPixelAlpha(image,p),15) << 12);
                pixel|=(unsigned short) (ScaleQuantumToAny(
                  GetPixelRed(image,p),15) << 8);
                pixel|=(unsigned short) (ScaleQuantumToAny(
                  GetPixelGreen(image,p),15) << 4);
                pixel|=(unsigned short) (ScaleQuantumToAny(
                  GetPixelBlue(image,p),15));
              }
            else if (bmp_subtype == RGB565)
              {
                pixel=(unsigned short) (ScaleQuantumToAny(
                  GetPixelRed(image,p),31) << 11);
                pixel|=(unsigned short) (ScaleQuantumToAny(
                  GetPixelGreen(image,p),63) << 5);
                pixel|=(unsigned short) (ScaleQuantumToAny(
                  GetPixelBlue(image,p),31));
              }
            else
              {
                if (bmp_subtype == ARGB1555)
                  pixel=(unsigned short) (ScaleQuantumToAny(
                    GetPixelAlpha(image,p),1) << 15);
                pixel|=(unsigned short) (ScaleQuantumToAny(
                  GetPixelRed(image,p),31) << 10);
                pixel|=(unsigned short) (ScaleQuantumToAny(
                  GetPixelGreen(image,p),31) << 5);
                pixel|=(unsigned short) (ScaleQuantumToAny(
                  GetPixelBlue(image,p),31));
              }
            *((unsigned short *) q)=pixel;
            q+=(ptrdiff_t) 2;
            p+=(ptrdiff_t) GetPixelChannels(image);
          }
          for (x=2L*(ssize_t) image->columns; x < (ssize_t) bytes_per_line; x++)
            *q++=0x00;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 24:
      {
        /*
          Convert DirectClass packet to BMP BGR888.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          q=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
            *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
            *q++=ScaleQuantumToChar(GetPixelRed(image,p));
            p+=(ptrdiff_t) GetPixelChannels(image);
          }
          for (x=3L*(ssize_t) image->columns; x < (ssize_t) bytes_per_line; x++)
            *q++=0x00;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 32:
      {
        /*
          Convert DirectClass packet to ARGB8888 pixel.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          q=pixels+((ssize_t) image->rows-y-1)*(ssize_t) bytes_per_line;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
            *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
            *q++=ScaleQuantumToChar(GetPixelRed(image,p));
            *q++=ScaleQuantumToChar(GetPixelAlpha(image,p));
            p+=(ptrdiff_t) GetPixelChannels(image);
          }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
    }
    if ((type > 2) && (bmp_info.bits_per_pixel == 8))
      if (image_info->compression != NoCompression)
        {
          MemoryInfo
            *rle_info;

          /*
            Convert run-length encoded raster pixels.
          */
          rle_info=AcquireVirtualMemory((size_t) (2*(bytes_per_line+2)+2),
            (image->rows+2)*sizeof(*pixels));
          if (rle_info == (MemoryInfo *) NULL)
            {
              pixel_info=RelinquishVirtualMemory(pixel_info);
              ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
            }
          bmp_data=(unsigned char *) GetVirtualMemoryBlob(rle_info);
          bmp_info.file_size-=bmp_info.image_size;
          bmp_info.image_size=(unsigned int) EncodeImage(image,bytes_per_line,
            pixels,bmp_data);
          bmp_info.file_size+=bmp_info.image_size;
          pixel_info=RelinquishVirtualMemory(pixel_info);
          pixel_info=rle_info;
          pixels=bmp_data;
          bmp_info.compression=BI_RLE8;
        }
    /*
      Write BMP for Windows, all versions, 14-byte header.
    */
    if (image->debug != MagickFalse)
      {
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "   Writing BMP version %.20g datastream",(double) type);
        if (image->storage_class == DirectClass)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "   Storage class=DirectClass");
        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "   Storage class=PseudoClass");
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "   Image depth=%.20g",(double) image->depth);
        if (image->alpha_trait != UndefinedPixelTrait)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "   Matte=True");
        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "   Matte=MagickFalse");
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "   BMP bits_per_pixel=%.20g",(double) bmp_info.bits_per_pixel);
        switch ((int) bmp_info.compression)
        {
           case BI_RGB:
           {
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "   Compression=BI_RGB");
             break;
           }
           case BI_RLE8:
           {
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "   Compression=BI_RLE8");
             break;
           }
           case BI_BITFIELDS:
           {
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "   Compression=BI_BITFIELDS");
             break;
           }
           case BI_ALPHABITFIELDS:
           {
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "   Compression=BI_BITFIELDS");
             break;
           }
           default:
           {
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "   Compression=UNKNOWN (%u)",bmp_info.compression);
             break;
           }
        }
        if (bmp_info.number_colors == 0)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "   Number_colors=unspecified");
        else
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "   Number_colors=%u",bmp_info.number_colors);
      }
    profile_data=0;
    profile_size=0;
    profile_size_pad=0;
    if (profile != (StringInfo *) NULL)
      {
        profile_data=(MagickOffsetType) bmp_info.file_size-14;  /* from head of BMP info header */
        profile_size=(MagickOffsetType) GetStringInfoLength(profile);
        if ((profile_size % 4) > 0)
          profile_size_pad=4-(profile_size%4);
        bmp_info.file_size+=(unsigned int) (profile_size+profile_size_pad);
      }
    (void) WriteBlob(image,2,(unsigned char *) "BM");
    (void) WriteBlobLSBLong(image,bmp_info.file_size);
    (void) WriteBlobLSBLong(image,bmp_info.ba_offset);  /* always 0 */
    (void) WriteBlobLSBLong(image,bmp_info.offset_bits);
    if (type == 2)
      {
        /*
          Write 12-byte version 2 bitmap header.
        */
        (void) WriteBlobLSBLong(image,bmp_info.size);
        (void) WriteBlobLSBSignedShort(image,(signed short) bmp_info.width);
        (void) WriteBlobLSBSignedShort(image,(signed short) bmp_info.height);
        (void) WriteBlobLSBShort(image,bmp_info.planes);
        (void) WriteBlobLSBShort(image,bmp_info.bits_per_pixel);
      }
    else
      {
        /*
          Write 40-byte version 3+ bitmap header.
        */
        (void) WriteBlobLSBLong(image,bmp_info.size);
        (void) WriteBlobLSBSignedLong(image,(signed int) bmp_info.width);
        (void) WriteBlobLSBSignedLong(image,(signed int) bmp_info.height);
        (void) WriteBlobLSBShort(image,bmp_info.planes);
        (void) WriteBlobLSBShort(image,bmp_info.bits_per_pixel);
        (void) WriteBlobLSBLong(image,bmp_info.compression);
        (void) WriteBlobLSBLong(image,bmp_info.image_size);
        (void) WriteBlobLSBLong(image,bmp_info.x_pixels);
        (void) WriteBlobLSBLong(image,bmp_info.y_pixels);
        (void) WriteBlobLSBLong(image,bmp_info.number_colors);
        (void) WriteBlobLSBLong(image,bmp_info.colors_important);
      }
    if ((type > 3) && ((image->alpha_trait != UndefinedPixelTrait) ||
        (have_color_info != MagickFalse)))
      {
        /*
          Write the rest of the 108-byte BMP Version 4 header.
        */
        (void) WriteBlobLSBLong(image,bmp_info.red_mask);
        (void) WriteBlobLSBLong(image,bmp_info.green_mask);
        (void) WriteBlobLSBLong(image,bmp_info.blue_mask);
        (void) WriteBlobLSBLong(image,bmp_info.alpha_mask);
        if (profile != (StringInfo *) NULL)
          (void) WriteBlobLSBLong(image,0x4D424544U);  /* PROFILE_EMBEDDED */
        else
          (void) WriteBlobLSBLong(image,0x73524742U);  /* sRGB */

        /* bounds check, assign .0 if invalid value */
        if (isgreater(image->chromaticity.red_primary.x, 1.0) ||
            !isgreater(image->chromaticity.red_primary.x, 0.0))
          image->chromaticity.red_primary.x = 0.0;
        if (isgreater(image->chromaticity.red_primary.y, 1.0) ||
            !isgreater(image->chromaticity.red_primary.y, 0.0))
          image->chromaticity.red_primary.y = 0.0;
        if (isgreater(image->chromaticity.green_primary.x, 1.0) ||
            !isgreater(image->chromaticity.green_primary.x, 0.0))
          image->chromaticity.green_primary.x = 0.0;
        if (isgreater(image->chromaticity.green_primary.y, 1.0) ||
            !isgreater(image->chromaticity.green_primary.y, 0.0))
          image->chromaticity.green_primary.y = 0.0;
        if (isgreater(image->chromaticity.blue_primary.x, 1.0) ||
            !isgreater(image->chromaticity.blue_primary.x, 0.0))
          image->chromaticity.blue_primary.x = 0.0;
        if (isgreater(image->chromaticity.blue_primary.y, 1.0) ||
            !isgreater(image->chromaticity.blue_primary.y, 0.0))
          image->chromaticity.blue_primary.y = 0.0;
        if (isgreater(bmp_info.gamma_scale.x, 1.0) ||
            !isgreater(bmp_info.gamma_scale.x, 0.0))
          bmp_info.gamma_scale.x = 0.0;
        if (isgreater(bmp_info.gamma_scale.y, 1.0) ||
            !isgreater(bmp_info.gamma_scale.y, 0.0))
          bmp_info.gamma_scale.y = 0.0;
        if (isgreater(bmp_info.gamma_scale.z, 1.0) ||
            !isgreater(bmp_info.gamma_scale.z, 0.0))
          bmp_info.gamma_scale.z = 0.0;

        (void) WriteBlobLSBLong(image,(unsigned int)
          (image->chromaticity.red_primary.x*0x40000000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          (image->chromaticity.red_primary.y*0x40000000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          ((1.000-(image->chromaticity.red_primary.x+
          image->chromaticity.red_primary.y))*0x40000000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          (image->chromaticity.green_primary.x*0x40000000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          (image->chromaticity.green_primary.y*0x40000000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          ((1.000-(image->chromaticity.green_primary.x+
          image->chromaticity.green_primary.y))*0x40000000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          (image->chromaticity.blue_primary.x*0x40000000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          (image->chromaticity.blue_primary.y*0x40000000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          ((1.000-(image->chromaticity.blue_primary.x+
          image->chromaticity.blue_primary.y))*0x40000000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          (bmp_info.gamma_scale.x*0x10000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          (bmp_info.gamma_scale.y*0x10000));
        (void) WriteBlobLSBLong(image,(unsigned int)
          (bmp_info.gamma_scale.z*0x10000));
        if ((image->rendering_intent != UndefinedIntent) ||
            (profile != (StringInfo *) NULL))
          {
            ssize_t
              intent;

            switch ((int) image->rendering_intent)
            {
              case SaturationIntent:
              {
                intent=LCS_GM_BUSINESS;
                break;
              }
              case RelativeIntent:
              {
                intent=LCS_GM_GRAPHICS;
                break;
              }
              case PerceptualIntent:
              {
                intent=LCS_GM_IMAGES;
                break;
              }
              case AbsoluteIntent:
              {
                intent=LCS_GM_ABS_COLORIMETRIC;
                break;
              }
              default:
              {
                intent=0;
                break;
              }
            }
            (void) WriteBlobLSBLong(image,(unsigned int) intent);
            (void) WriteBlobLSBLong(image,(unsigned int) profile_data);
            (void) WriteBlobLSBLong(image,(unsigned int)
              (profile_size+profile_size_pad));
            (void) WriteBlobLSBLong(image,0x00);  /* reserved */
          }
      }
    if (image->storage_class == PseudoClass)
      {
        unsigned char
          *bmp_colormap;

        /*
          Dump colormap to file.
        */
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Colormap: %.20g entries",(double) image->colors);
        bmp_colormap=(unsigned char *) AcquireQuantumMemory((size_t) 1UL <<
          bmp_info.bits_per_pixel,4*sizeof(*bmp_colormap));
        if (bmp_colormap == (unsigned char *) NULL)
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
        q=bmp_colormap;
        for (i=0; i < MagickMin((ssize_t) image->colors,(ssize_t) bmp_info.number_colors); i++)
        {
          *q++=ScaleQuantumToChar(ClampToQuantum(image->colormap[i].blue));
          *q++=ScaleQuantumToChar(ClampToQuantum(image->colormap[i].green));
          *q++=ScaleQuantumToChar(ClampToQuantum(image->colormap[i].red));
          if (type > 2)
            *q++=(unsigned char) 0x0;
        }
        for ( ; i < (ssize_t) 1UL << bmp_info.bits_per_pixel; i++)
        {
          *q++=(unsigned char) 0x00;
          *q++=(unsigned char) 0x00;
          *q++=(unsigned char) 0x00;
          if (type > 2)
            *q++=(unsigned char) 0x00;
        }
        if (type <= 2)
          (void) WriteBlob(image,(size_t) (3*(1L << bmp_info.bits_per_pixel)),
            bmp_colormap);
        else
          (void) WriteBlob(image,(size_t) (4*(1L << bmp_info.bits_per_pixel)),
            bmp_colormap);
        bmp_colormap=(unsigned char *) RelinquishMagickMemory(bmp_colormap);
      }
    if (image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "  Pixels:  %u bytes",bmp_info.image_size);
    (void) WriteBlob(image,(size_t) bmp_info.image_size,pixels);
    if (profile != (StringInfo *) NULL)
      {
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Profile:  %g bytes",(double) profile_size+profile_size_pad);
        (void) WriteBlob(image,(size_t) profile_size,
          GetStringInfoDatum(profile));
        if (profile_size_pad > 0)  /* padding for 4 bytes multiple */
          (void) WriteBlob(image,(size_t) profile_size_pad,"\0\0\0");
      }
    pixel_info=RelinquishVirtualMemory(pixel_info);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,number_scenes);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

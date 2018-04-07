/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        PPPP    AAA   L      M   M                           %
%                        P   P  A   A  L      MM MM                           %
%                        PPPP   AAAAA  L      M M M                           %
%                        P      A   A  L      M   M                           %
%                        P      A   A  LLLLL  M   M                           %
%                                                                             %
%                                                                             %
%                          Read/Write Palm Pixmap.                            %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                            Christopher R. Hawks                             %
%                               December 2001                                 %
%                                                                             %
%  Copyright 1999-2004 ImageMagick Studio LLC, a non-profit organization      %
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
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Based on pnmtopalm by Bill Janssen and ppmtobmp by Ian Goldberg.
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
#include "MagickCore/color.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/histogram.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"

/*
  Define declarations.
*/
#define PALM_IS_COMPRESSED_FLAG  0x8000
#define PALM_HAS_COLORMAP_FLAG  0x4000
#define PALM_HAS_FOUR_BYTE_FIELD  0x0200
#define PALM_HAS_TRANSPARENCY_FLAG  0x2000
#define PALM_IS_INDIRECT  0x1000
#define PALM_IS_FOR_SCREEN  0x0800
#define PALM_IS_DIRECT_COLOR  0x0400
#define PALM_COMPRESSION_SCANLINE  0x00
#define PALM_COMPRESSION_RLE  0x01
#define PALM_COMPRESSION_NONE  0xFF

/*
 The 256 color system palette for Palm Computing Devices.
*/
static const unsigned char
  PalmPalette[256][3] =
  {
    {255, 255,255}, {255, 204,255}, {255, 153,255}, {255, 102,255},
    {255,  51,255}, {255,   0,255}, {255, 255,204}, {255, 204,204},
    {255, 153,204}, {255, 102,204}, {255,  51,204}, {255,   0,204},
    {255, 255,153}, {255, 204,153}, {255, 153,153}, {255, 102,153},
    {255,  51,153}, {255,   0,153}, {204, 255,255}, {204, 204,255},
    {204, 153,255}, {204, 102,255}, {204,  51,255}, {204,   0,255},
    {204, 255,204}, {204, 204,204}, {204, 153,204}, {204, 102,204},
    {204,  51,204}, {204,   0,204}, {204, 255,153}, {204, 204,153},
    {204, 153,153}, {204, 102,153}, {204,  51,153}, {204,   0,153},
    {153, 255,255}, {153, 204,255}, {153, 153,255}, {153, 102,255},
    {153,  51,255}, {153,   0,255}, {153, 255,204}, {153, 204,204},
    {153, 153,204}, {153, 102,204}, {153,  51,204}, {153,   0,204},
    {153, 255,153}, {153, 204,153}, {153, 153,153}, {153, 102,153},
    {153,  51,153}, {153,   0,153}, {102, 255,255}, {102, 204,255},
    {102, 153,255}, {102, 102,255}, {102,  51,255}, {102,   0,255},
    {102, 255,204}, {102, 204,204}, {102, 153,204}, {102, 102,204},
    {102,  51,204}, {102,   0,204}, {102, 255,153}, {102, 204,153},
    {102, 153,153}, {102, 102,153}, {102,  51,153}, {102,   0,153},
    { 51, 255,255}, { 51, 204,255}, { 51, 153,255}, { 51, 102,255},
    { 51,  51,255}, { 51,   0,255}, { 51, 255,204}, { 51, 204,204},
    { 51, 153,204}, { 51, 102,204}, { 51,  51,204}, { 51,   0,204},
    { 51, 255,153}, { 51, 204,153}, { 51, 153,153}, { 51, 102,153},
    { 51,  51,153}, { 51,   0,153}, {  0, 255,255}, {  0, 204,255},
    {  0, 153,255}, {  0, 102,255}, {  0,  51,255}, {  0,   0,255},
    {  0, 255,204}, {  0, 204,204}, {  0, 153,204}, {  0, 102,204},
    {  0,  51,204}, {  0,   0,204}, {  0, 255,153}, {  0, 204,153},
    {  0, 153,153}, {  0, 102,153}, {  0,  51,153}, {  0,   0,153},
    {255, 255,102}, {255, 204,102}, {255, 153,102}, {255, 102,102},
    {255,  51,102}, {255,   0,102}, {255, 255, 51}, {255, 204, 51},
    {255, 153, 51}, {255, 102, 51}, {255,  51, 51}, {255,   0, 51},
    {255, 255,  0}, {255, 204,  0}, {255, 153,  0}, {255, 102,  0},
    {255,  51,  0}, {255,   0,  0}, {204, 255,102}, {204, 204,102},
    {204, 153,102}, {204, 102,102}, {204,  51,102}, {204,   0,102},
    {204, 255, 51}, {204, 204, 51}, {204, 153, 51}, {204, 102, 51},
    {204,  51, 51}, {204,   0, 51}, {204, 255,  0}, {204, 204,  0},
    {204, 153,  0}, {204, 102,  0}, {204,  51,  0}, {204,   0,  0},
    {153, 255,102}, {153, 204,102}, {153, 153,102}, {153, 102,102},
    {153,  51,102}, {153,   0,102}, {153, 255, 51}, {153, 204, 51},
    {153, 153, 51}, {153, 102, 51}, {153,  51, 51}, {153,   0, 51},
    {153, 255,  0}, {153, 204,  0}, {153, 153,  0}, {153, 102,  0},
    {153,  51,  0}, {153,   0,  0}, {102, 255,102}, {102, 204,102},
    {102, 153,102}, {102, 102,102}, {102,  51,102}, {102,   0,102},
    {102, 255, 51}, {102, 204, 51}, {102, 153, 51}, {102, 102, 51},
    {102,  51, 51}, {102,   0, 51}, {102, 255,  0}, {102, 204,  0},
    {102, 153,  0}, {102, 102,  0}, {102,  51,  0}, {102,   0,  0},
    { 51, 255,102}, { 51, 204,102}, { 51, 153,102}, { 51, 102,102},
    { 51,  51,102}, { 51,   0,102}, { 51, 255, 51}, { 51, 204, 51},
    { 51, 153, 51}, { 51, 102, 51}, { 51,  51, 51}, { 51,   0, 51},
    { 51, 255,  0}, { 51, 204,  0}, { 51, 153,  0}, { 51, 102,  0},
    { 51,  51,  0}, { 51,   0,  0}, {  0, 255,102}, {  0, 204,102},
    {  0, 153,102}, {  0, 102,102}, {  0,  51,102}, {  0,   0,102},
    {  0, 255, 51}, {  0, 204, 51}, {  0, 153, 51}, {  0, 102, 51},
    {  0,  51, 51}, {  0,   0, 51}, {  0, 255,  0}, {  0, 204,  0},
    {  0, 153,  0}, {  0, 102,  0}, {  0,  51,  0}, { 17,  17, 17},
    { 34,  34, 34}, { 68,  68, 68}, { 85,  85, 85}, {119, 119,119},
    {136, 136,136}, {170, 170,170}, {187, 187,187}, {221, 221,221},
    {238, 238,238}, {192, 192,192}, {128,   0,  0}, {128,   0,128},
    {  0, 128,  0}, {  0, 128,128}, {  0,   0,  0}, {  0,   0,  0},
    {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0},
    {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0},
    {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0},
    {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0},
    {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0},
    {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0}, {  0,   0,  0}
  };

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePALMImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i n d C o l o r                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FindColor() returns the index of the matching entry from PalmPalette for a
%  given PixelInfo.
%
%  The format of the FindColor method is:
%
%      int FindColor(PixelInfo *pixel)
%
%  A description of each parameter follows:
%
%    o int: the index of the matching color or -1 if not found/
%
%    o pixel: a pointer to the PixelInfo to be matched.
%
*/
static ssize_t FindColor(PixelInfo *packet)
{
  register ssize_t
    i;

  for (i=0; i < 256; i++)
    if (ScaleQuantumToChar(ClampToQuantum(packet->red)) == PalmPalette[i][0] &&
        ScaleQuantumToChar(ClampToQuantum(packet->green)) == PalmPalette[i][1] &&
        ScaleQuantumToChar(ClampToQuantum(packet->blue)) == PalmPalette[i][2])
      return(i);
  return(-1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P A L M I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPALMImage() reads an image of raw bites in LSB order and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadPALMImage method is:
%
%      Image *ReadPALMImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadPALMImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    totalOffset,
    seekNextDepth;

  PixelInfo
    transpix;

  Quantum
    index;

  register ssize_t
    i,
    x;

  register Quantum
    *q;

  size_t
    bits_per_pixel,
    bytes_per_row,
    flags,
    extent,
    version,
    nextDepthOffset,
    transparentIndex,
    compressionType,
    byte,
    mask,
    redbits,
    greenbits,
    bluebits,
    one,
    pad,
    size,
    bit;

  ssize_t
    count,
    y;

  unsigned char
    *last_row,
    *one_row,
    *ptr;

  unsigned short
    color16;

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
      (void) DestroyImageList(image);
      return((Image *) NULL);
    }
  totalOffset=0;
  do
  {
    image->columns=ReadBlobMSBShort(image);
    image->rows=ReadBlobMSBShort(image);
    if (EOFBlob(image) != MagickFalse)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if ((image->columns == 0) || (image->rows == 0))
      ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    (void) SetImageBackgroundColor(image,exception);
    bytes_per_row=ReadBlobMSBShort(image);
    flags=ReadBlobMSBShort(image);
    bits_per_pixel=(size_t) ReadBlobByte(image);
    if ((bits_per_pixel != 1) && (bits_per_pixel != 2) &&
        (bits_per_pixel != 4) && (bits_per_pixel != 8) &&
        (bits_per_pixel != 16))
      ThrowReaderException(CorruptImageError,"UnrecognizedBitsPerPixel");
    version=(size_t) ReadBlobByte(image);
    if ((version != 0) && (version != 1) && (version != 2))
      ThrowReaderException(CorruptImageError,"FileFormatVersionMismatch");
    nextDepthOffset=(size_t) ReadBlobMSBShort(image);
    transparentIndex=(size_t) ReadBlobByte(image);
    compressionType=(size_t) ReadBlobByte(image);
    if ((compressionType != PALM_COMPRESSION_NONE) &&
        (compressionType != PALM_COMPRESSION_SCANLINE ) &&
        (compressionType != PALM_COMPRESSION_RLE))
      ThrowReaderException(CorruptImageError,"UnrecognizedImageCompression");
    pad=ReadBlobMSBShort(image);
    (void) pad;
    /*
      Initialize image colormap.
    */
    one=1;
    if ((bits_per_pixel < 16) &&
        (AcquireImageColormap(image,one << bits_per_pixel,exception) == MagickFalse))
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    GetPixelInfo(image,&transpix);
    if (bits_per_pixel == 16)  /* Direct Color */
      {
        redbits=(size_t) ReadBlobByte(image);  /* # of bits of red */
        (void) redbits;
        greenbits=(size_t) ReadBlobByte(image);  /* # of bits of green */
        (void) greenbits;
        bluebits=(size_t) ReadBlobByte(image);  /* # of bits of blue */
        (void) bluebits;
        ReadBlobByte(image);  /* reserved by Palm */
        ReadBlobByte(image);  /* reserved by Palm */
        transpix.red=(double) (QuantumRange*ReadBlobByte(image)/31);
        transpix.green=(double) (QuantumRange*ReadBlobByte(image)/63);
        transpix.blue=(double) (QuantumRange*ReadBlobByte(image)/31);
      }
    if (bits_per_pixel == 8)
      {
        ssize_t
          index;

        if (flags & PALM_HAS_COLORMAP_FLAG)
          {
            count=(ssize_t) ReadBlobMSBShort(image);
            for (i=0; i < (ssize_t) count; i++)
            {
              ReadBlobByte(image);
              index=ConstrainColormapIndex(image,255-i,exception);
              image->colormap[index].red=(MagickRealType)
                ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
              image->colormap[index].green=(MagickRealType)
                ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
              image->colormap[index].blue=(MagickRealType)
                ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
          }
        }
      else
        for (i=0; i < (ssize_t) (1L << bits_per_pixel); i++)
        {
          index=ConstrainColormapIndex(image,255-i,exception);
          image->colormap[index].red=(MagickRealType)
            ScaleCharToQuantum(PalmPalette[i][0]);
          image->colormap[index].green=(MagickRealType)
            ScaleCharToQuantum(PalmPalette[i][1]);
          image->colormap[index].blue=(MagickRealType)
            ScaleCharToQuantum(PalmPalette[i][2]);
        }
      }
    if (flags & PALM_IS_COMPRESSED_FLAG)
      size=ReadBlobMSBShort(image);
    (void) size;
    image->storage_class=DirectClass;
    if (bits_per_pixel < 16)
      {
        image->storage_class=PseudoClass;
        image->depth=8;
      }
    if (image_info->ping != MagickFalse)
      {
        (void) CloseBlob(image);
        return(image);
      }
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    extent=MagickMax(bytes_per_row,2*image->columns);
    one_row=(unsigned char *) AcquireQuantumMemory(extent,sizeof(*one_row));
    if (one_row == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    (void) memset(one_row,0,extent*sizeof(*one_row));
    last_row=(unsigned char *) NULL;
    if (compressionType == PALM_COMPRESSION_SCANLINE)
      {
        last_row=(unsigned char *) AcquireQuantumMemory(MagickMax(bytes_per_row,
          2*image->columns),sizeof(*last_row));
        if (last_row == (unsigned char *) NULL)
          {
            one_row=(unsigned char *) RelinquishMagickMemory(one_row);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
         (void) memset(last_row,0,MagickMax(bytes_per_row,2*image->columns)*
           sizeof(*last_row));
      }
    mask=(size_t) (1U << bits_per_pixel)-1;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      if ((flags & PALM_IS_COMPRESSED_FLAG) == 0)
        {
          /* TODO move out of loop! */
          image->compression=NoCompression;
          count=ReadBlob(image,bytes_per_row,one_row);
          if (count != (ssize_t) bytes_per_row)
            break;
        }
      else
        {
          if (compressionType == PALM_COMPRESSION_RLE)
            { 
              /* TODO move out of loop! */
              image->compression=RLECompression;
              for (i=0; i < (ssize_t) bytes_per_row; )
              {
                count=(ssize_t) ReadBlobByte(image);
                if (count < 0)
                  break;
                count=MagickMin(count,(ssize_t) bytes_per_row-i);
                byte=(size_t) ReadBlobByte(image);
                (void) memset(one_row+i,(int) byte,(size_t) count);
                i+=count;
              }
          }
        else
          if (compressionType == PALM_COMPRESSION_SCANLINE)
            {
              size_t
                one;

              /* TODO move out of loop! */
              one=1;
              image->compression=FaxCompression;
              for (i=0; i < (ssize_t) bytes_per_row; i+=8)
              {
                count=(ssize_t) ReadBlobByte(image);
                if (count < 0)
                  break;
                byte=(size_t) MagickMin((ssize_t) bytes_per_row-i,8);
                for (bit=0; bit < byte; bit++)
                {
                  if ((y == 0) || (count & (one << (7 - bit))))
                    one_row[i+bit]=(unsigned char) ReadBlobByte(image);
                  else
                    one_row[i+bit]=last_row[i+bit];
                }
              }
              (void) memcpy(last_row, one_row, bytes_per_row);
            }
        }
      ptr=one_row;
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      if (bits_per_pixel == 16)
        {
          if (image->columns > (2*bytes_per_row))
            {
              one_row=(unsigned char *) RelinquishMagickMemory(one_row);
              if (compressionType == PALM_COMPRESSION_SCANLINE)
                last_row=(unsigned char *) RelinquishMagickMemory(last_row);
              ThrowReaderException(CorruptImageError,"CorruptImage");
            }
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            color16=(*ptr++ << 8);
            color16|=(*ptr++);
            SetPixelRed(image,(Quantum) ((QuantumRange*((color16 >> 11) &
              0x1f))/0x1f),q);
            SetPixelGreen(image,(Quantum) ((QuantumRange*((color16 >> 5) &
              0x3f))/0x3f),q);
            SetPixelBlue(image,(Quantum) ((QuantumRange*((color16 >> 0) &
              0x1f))/0x1f),q);
            SetPixelAlpha(image,OpaqueAlpha,q);
            q+=GetPixelChannels(image);
          }
        }
      else
        {
          bit=8-bits_per_pixel;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if ((size_t) (ptr-one_row) >= bytes_per_row)
              {
                one_row=(unsigned char *) RelinquishMagickMemory(one_row);
                if (compressionType == PALM_COMPRESSION_SCANLINE)
                  last_row=(unsigned char *) RelinquishMagickMemory(last_row);
                ThrowReaderException(CorruptImageError,"CorruptImage");
              }
            index=(Quantum) (mask-(((*ptr) & (mask << bit)) >> bit));
            SetPixelIndex(image,index,q);
            SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            if (bit)
              bit-=bits_per_pixel;
            else
              {
                ptr++;
                bit=8-bits_per_pixel;
              }
            q+=GetPixelChannels(image);
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
      }
    if (flags & PALM_HAS_TRANSPARENCY_FLAG)
      {
        ssize_t index=ConstrainColormapIndex(image,(ssize_t) (mask-
          transparentIndex),exception);
        if (bits_per_pixel != 16)
          transpix=image->colormap[index];
        (void) TransparentPaintImage(image,&transpix,(Quantum) TransparentAlpha,
          MagickFalse,exception);
      }
    one_row=(unsigned char *) RelinquishMagickMemory(one_row);
    if (compressionType == PALM_COMPRESSION_SCANLINE)
      last_row=(unsigned char *) RelinquishMagickMemory(last_row);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image. Copied from coders/pnm.c
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if (nextDepthOffset != 0)
      { 
        /*
          Skip to next image. 
        */
        totalOffset+=(MagickOffsetType) (nextDepthOffset*4);
        if (totalOffset >= (MagickOffsetType) GetBlobSize(image))
          ThrowReaderException(CorruptImageError,"ImproperImageHeader")
        else
          seekNextDepth=SeekBlob(image,totalOffset,SEEK_SET);
        if (seekNextDepth != totalOffset)
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        /*
          Allocate next image structure. Copied from coders/pnm.c
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            (void) DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  } while (nextDepthOffset != 0);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P A L M I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPALMImage() adds properties for the PALM image format to the list of
%  supported formats.  The properties include the image format tag, a method to
%  read and/or write the format, whether the format supports the saving of more
%  than one frame to the same file or blob, whether the format supports native
%  in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterPALMImage method is:
%
%      size_t RegisterPALMImage(void)
%
*/
ModuleExport size_t RegisterPALMImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("PALM","PALM","Palm pixmap");
  entry->decoder=(DecodeImageHandler *) ReadPALMImage;
  entry->encoder=(EncodeImageHandler *) WritePALMImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P A L M I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPALMImage() removes format registrations made by the PALM
%  module from the list of supported formats.
%
%  The format of the UnregisterPALMImage method is:
%
%      UnregisterPALMImage(void)
%
*/
ModuleExport void UnregisterPALMImage(void)
{
  (void) UnregisterMagickInfo("PALM");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P A L M I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePALMImage() writes an image of raw bits in LSB order to a file.
%
%  The format of the WritePALMImage method is:
%
%      MagickBooleanType WritePALMImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WritePALMImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickOffsetType
    currentOffset,
    offset,
    scene;

  MagickSizeType
    cc;

  PixelInfo
    transpix;

  QuantizeInfo
    *quantize_info;

  register ssize_t
    x;

  register const Quantum
    *p;

  register Quantum
    *q;

  ssize_t
    y;

  size_t
    count,
    bits_per_pixel,
    bytes_per_row,
    imageListLength,
    nextDepthOffset,
    one;

  unsigned char
    bit,
    byte,
    color,
    *last_row,
    *one_row,
    *ptr,
    version;

  unsigned int
    transparentIndex;

  unsigned short
    color16,
    flags;

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
  quantize_info=AcquireQuantizeInfo(image_info);
  flags=0;
  currentOffset=0;
  transparentIndex=0;
  transpix.red=0.0;
  transpix.green=0.0;
  transpix.blue=0.0;
  transpix.alpha=0.0;
  one=1;
  version=0;
  scene=0;
  imageListLength=GetImageListLength(image);
  do
  {
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
    count=GetNumberColors(image,NULL,exception);
    for (bits_per_pixel=1;  (one << bits_per_pixel) < count; bits_per_pixel*=2) ;
    if (bits_per_pixel > 16)
      bits_per_pixel=16;
    else
      if (bits_per_pixel < 16)
        (void) TransformImageColorspace(image,image->colorspace,exception);
    if (bits_per_pixel < 8)
      {
        (void) TransformImageColorspace(image,GRAYColorspace,exception);
        (void) SetImageType(image,PaletteType,exception);
        (void) SortColormapByIntensity(image,exception);
      }
    if ((image->storage_class == PseudoClass) && (image->colors > 256))
      (void) SetImageStorageClass(image,DirectClass,exception);
    if (image->storage_class == PseudoClass)
      flags|=PALM_HAS_COLORMAP_FLAG;
    else
      flags|=PALM_IS_DIRECT_COLOR;
    (void) WriteBlobMSBShort(image,(unsigned short) image->columns); /* width */
    (void) WriteBlobMSBShort(image,(unsigned short) image->rows);  /* height */
    bytes_per_row=((image->columns+(16/bits_per_pixel-1))/(16/
      bits_per_pixel))*2;
    (void) WriteBlobMSBShort(image,(unsigned short) bytes_per_row);
    if ((image_info->compression == RLECompression) ||
        (image_info->compression == FaxCompression))
      flags|=PALM_IS_COMPRESSED_FLAG;
    (void) WriteBlobMSBShort(image, flags);
    (void) WriteBlobByte(image,(unsigned char) bits_per_pixel);
    if (bits_per_pixel > 1)
      version=1;
    if ((image_info->compression == RLECompression) ||
        (image_info->compression == FaxCompression))
      version=2;
    (void) WriteBlobByte(image,version);
    (void) WriteBlobMSBShort(image,0);  /* nextDepthOffset */
    (void) WriteBlobByte(image,(unsigned char) transparentIndex);
    if (image_info->compression == RLECompression)
      (void) WriteBlobByte(image,PALM_COMPRESSION_RLE);
    else
      if (image_info->compression == FaxCompression)
        (void) WriteBlobByte(image,PALM_COMPRESSION_SCANLINE);
      else
        (void) WriteBlobByte(image,PALM_COMPRESSION_NONE);
    (void) WriteBlobMSBShort(image,0);  /* reserved */
    offset=16;
    if (bits_per_pixel == 16)
      {
        (void) WriteBlobByte(image,5);  /* # of bits of red */
        (void) WriteBlobByte(image,6);  /* # of bits of green */
        (void) WriteBlobByte(image,5);  /* # of bits of blue */
        (void) WriteBlobByte(image,0);  /* reserved by Palm */
        (void) WriteBlobMSBLong(image,0);  /* no transparent color, YET */
        offset+=8;
      }
    if (bits_per_pixel == 8)
      {
        if (flags & PALM_HAS_COLORMAP_FLAG)  /* Write out colormap */
          {
            quantize_info->dither_method=IdentifyPaletteImage(image,exception)
              == MagickFalse ? RiemersmaDitherMethod : NoDitherMethod;
            quantize_info->number_colors=image->colors;
            (void) QuantizeImage(quantize_info,image,exception);
            (void) WriteBlobMSBShort(image,(unsigned short) image->colors);
            for (count = 0; count < image->colors; count++)
            {
              (void) WriteBlobByte(image,(unsigned char) count);
              (void) WriteBlobByte(image,ScaleQuantumToChar(ClampToQuantum(
                image->colormap[count].red)));
              (void) WriteBlobByte(image,ScaleQuantumToChar(ClampToQuantum(
                image->colormap[count].green)));
              (void) WriteBlobByte(image,ScaleQuantumToChar(ClampToQuantum(
                image->colormap[count].blue)));
            }
            offset+=2+count*4;
          }
      else  /* Map colors to Palm standard colormap */
        {
          Image
            *affinity_image;

          affinity_image=ConstituteImage(256,1,"RGB",CharPixel,&PalmPalette,
            exception);
          (void) TransformImageColorspace(affinity_image,
            affinity_image->colorspace,exception);
          (void) RemapImage(quantize_info,image,affinity_image,exception);
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              SetPixelIndex(image,(Quantum) FindColor(&image->colormap[(ssize_t)
                GetPixelIndex(image,q)]),q);
              q+=GetPixelChannels(image);
            }
          }
          affinity_image=DestroyImage(affinity_image);
        }
      }
    if (flags & PALM_IS_COMPRESSED_FLAG)
      (void) WriteBlobMSBShort(image,0);  /* fill in size later */
    last_row=(unsigned char *) NULL;
    if (image_info->compression == FaxCompression)
      {
        last_row=(unsigned char *) AcquireQuantumMemory(bytes_per_row,
          sizeof(*last_row));
        if (last_row == (unsigned char *) NULL)
          {
            quantize_info=DestroyQuantizeInfo(quantize_info);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
      }
    one_row=(unsigned char *) AcquireQuantumMemory(bytes_per_row,
      sizeof(*one_row));
    if (one_row == (unsigned char *) NULL)
      {
        if (last_row != (unsigned char *) NULL) 
          last_row=(unsigned char *) RelinquishMagickMemory(last_row);
        quantize_info=DestroyQuantizeInfo(quantize_info);
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      }
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      ptr=one_row;
      (void) memset(ptr,0,bytes_per_row);
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        break;
      if (bits_per_pixel == 16)
        {
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            color16=(unsigned short) ((((31*(size_t) GetPixelRed(image,p))/
              (size_t) QuantumRange) << 11) | (((63*(size_t)
              GetPixelGreen(image,p))/(size_t) QuantumRange) << 5) |
              ((31*(size_t) GetPixelBlue(image,p))/(size_t) QuantumRange));
            if (GetPixelAlpha(image,p) == (Quantum) TransparentAlpha)
              {
                transpix.red=(MagickRealType) GetPixelRed(image,p);
                transpix.green=(MagickRealType) GetPixelGreen(image,p);
                transpix.blue=(MagickRealType) GetPixelBlue(image,p);
                transpix.alpha=(MagickRealType) GetPixelAlpha(image,p);
                flags|=PALM_HAS_TRANSPARENCY_FLAG;
              }
            *ptr++=(unsigned char) ((color16 >> 8) & 0xff);
            *ptr++=(unsigned char) (color16 & 0xff);
            p+=GetPixelChannels(image);
          }
        }
      else
        {
          byte=0x00;
          bit=(unsigned char) (8-bits_per_pixel);
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (bits_per_pixel >= 8)
              color=(unsigned char) GetPixelIndex(image,p);
            else
              color=(unsigned char) (GetPixelIndex(image,p)*
                ((one << bits_per_pixel)-1)/MagickMax(1*image->colors-1,1));
            byte|=color << bit;
            if (bit != 0)
              bit-=(unsigned char) bits_per_pixel;
            else
              {
                *ptr++=byte;
                byte=0x00;
                bit=(unsigned char) (8-bits_per_pixel);
              }
            p+=GetPixelChannels(image);
          }
          if ((image->columns % (8/bits_per_pixel)) != 0)
            *ptr++=byte;
        }
      if (image_info->compression == RLECompression)
        {
          x=0;
          while (x < (ssize_t) bytes_per_row)
          {
            byte=one_row[x];
            count=1;
            while ((one_row[++x] == byte) && (count < 255) &&
                   (x < (ssize_t) bytes_per_row))
              count++;
            (void) WriteBlobByte(image,(unsigned char) count);
            (void) WriteBlobByte(image,(unsigned char) byte);
          }
        }
      else
        if (image_info->compression == FaxCompression)
          {
            char
              tmpbuf[8],
              *tptr;
  
            for (x = 0;  x < (ssize_t) bytes_per_row;  x += 8)
            {
              tptr = tmpbuf;
              for (bit=0, byte=0; bit < (unsigned char) MagickMin(8,(ssize_t) bytes_per_row-x); bit++)
              {
                if ((y == 0) || (last_row[x + bit] != one_row[x + bit]))
                  {
                    byte |= (1 << (7 - bit));
                    *tptr++ = (char) one_row[x + bit];
                  }
              }
              (void) WriteBlobByte(image, byte);
              (void) WriteBlob(image,tptr-tmpbuf,(unsigned char *) tmpbuf);
            }
            (void) memcpy(last_row,one_row,bytes_per_row);
          }
        else
          (void) WriteBlob(image,bytes_per_row,one_row);
      }
    if (flags & PALM_HAS_TRANSPARENCY_FLAG)
      {
        offset=SeekBlob(image,currentOffset+6,SEEK_SET);
        (void) WriteBlobMSBShort(image,flags);
        offset=SeekBlob(image,currentOffset+12,SEEK_SET);
        (void) WriteBlobByte(image,(unsigned char) transparentIndex);  /* trans index */
      }
    if (bits_per_pixel == 16)
      {
        offset=SeekBlob(image,currentOffset+20,SEEK_SET);
        (void) WriteBlobByte(image,0);  /* reserved by Palm */
        (void) WriteBlobByte(image,(unsigned char) ((31*transpix.red)/
          QuantumRange));
        (void) WriteBlobByte(image,(unsigned char) ((63*transpix.green)/
          QuantumRange));
        (void) WriteBlobByte(image,(unsigned char) ((31*transpix.blue)/
          QuantumRange));
      }
    if (flags & PALM_IS_COMPRESSED_FLAG)  /* fill in size now */
      {
        offset=SeekBlob(image,currentOffset+offset,SEEK_SET);
        (void) WriteBlobMSBShort(image,(unsigned short) (GetBlobSize(image)-
          currentOffset-offset));
      }
    if (one_row != (unsigned char *) NULL) 
      one_row=(unsigned char *) RelinquishMagickMemory(one_row);
    if (last_row != (unsigned char *) NULL) 
      last_row=(unsigned char *) RelinquishMagickMemory(last_row);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    /* padding to 4 byte word */
    for (cc=(GetBlobSize(image)) % 4; cc > 0; cc--)
      (void) WriteBlobByte(image,0);
    /* write nextDepthOffset and return to end of image */
    offset=SeekBlob(image,currentOffset+10,SEEK_SET);
    nextDepthOffset=(size_t) ((GetBlobSize(image)-currentOffset)/4);
    (void) WriteBlobMSBShort(image,(unsigned short) nextDepthOffset);
    currentOffset=(MagickOffsetType) GetBlobSize(image);
    offset=SeekBlob(image,currentOffset,SEEK_SET);
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,imageListLength);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  quantize_info=DestroyQuantizeInfo(quantize_info);
  (void) CloseBlob(image);
  return(MagickTrue);
}

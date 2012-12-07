/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            SSSSS  U   U  N   N                              %
%                            SS     U   U  NN  N                              %
%                             SSS   U   U  N N N                              %
%                               SS  U   U  N  NN                              %
%                            SSSSS   UUU   N   N                              %
%                                                                             %
%                                                                             %
%                    Read/Write Sun Rasterfile Image Format                   %
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
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
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
#include "magick/string_.h"
#include "magick/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteSUNImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s S U N                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsSUN() returns MagickTrue if the image format type, identified by the
%  magick string, is SUN.
%
%  The format of the IsSUN method is:
%
%      MagickBooleanType IsSUN(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsSUN(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\131\246\152\225",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

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
%  DecodeImage unpacks the packed image pixels into  runlength-encoded pixel
%  packets.
%
%  The format of the DecodeImage method is:
%
%      MagickBooleanType DecodeImage(const unsigned char *compressed_pixels,
%        const size_t length,unsigned char *pixels)
%
%  A description of each parameter follows:
%
%    o compressed_pixels:  The address of a byte (8 bits) array of compressed
%      pixel data.
%
%    o length:  An integer value that is the total number of bytes of the
%      source image (as just read by ReadBlob)
%
%    o pixels:  The address of a byte (8 bits) array of pixel data created by
%      the uncompression process.  The number of bytes in this array
%      must be at least equal to the number columns times the number of rows
%      of the source pixels.
%
*/
static MagickBooleanType DecodeImage(const unsigned char *compressed_pixels,
  const size_t length,unsigned char *pixels,size_t maxpixels)
{
  register const unsigned char
    *l,
    *p;

  register unsigned char
    *q;

  ssize_t
    count;

  unsigned char
    byte;

  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(compressed_pixels != (unsigned char *) NULL);
  assert(pixels != (unsigned char *) NULL);
  p=compressed_pixels;
  q=pixels;
  l=q+maxpixels;
  while (((size_t) (p-compressed_pixels) < length) && (q < l))
  {
    byte=(*p++);
    if (byte != 128U)
      *q++=byte;
    else
      {
        /*
          Runlength-encoded packet: <count><byte>
        */
        count=(ssize_t) (*p++);
        if (count > 0)
          byte=(*p++);
        while ((count >= 0) && (q < l))
        {
          *q++=byte;
          count--;
        }
     }
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S U N I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadSUNImage() reads a SUN image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadSUNImage method is:
%
%      Image *ReadSUNImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadSUNImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define RMT_EQUAL_RGB  1
#define RMT_NONE  0
#define RMT_RAW  2
#define RT_STANDARD  1
#define RT_ENCODED  2
#define RT_FORMAT_RGB  3

  typedef struct _SUNInfo
  {
    unsigned int
      magic,
      width,
      height,
      depth,
      length,
      type,
      maptype,
      maplength;
  } SUNInfo;

  Image
    *image;

  int
    bit;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  register IndexPacket
    *indexes;

  register PixelPacket
    *q;

  register ssize_t
    i,
    x;

  register unsigned char
    *p;

  size_t
    length;

  ssize_t
    count,
    y;

  SUNInfo
    sun_info;

  unsigned char
    *sun_data,
    *sun_pixels;

  unsigned int
    bytes_per_line;

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
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read SUN raster header.
  */
  (void) ResetMagickMemory(&sun_info,0,sizeof(sun_info));
  sun_info.magic=ReadBlobMSBLong(image);
  do
  {
    /*
      Verify SUN identifier.
    */
    if (sun_info.magic != 0x59a66a95)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    sun_info.width=ReadBlobMSBLong(image);
    sun_info.height=ReadBlobMSBLong(image);
    sun_info.depth=ReadBlobMSBLong(image);
    sun_info.length=ReadBlobMSBLong(image);
    sun_info.type=ReadBlobMSBLong(image);
    sun_info.maptype=ReadBlobMSBLong(image);
    sun_info.maplength=ReadBlobMSBLong(image);
    image->columns=sun_info.width;
    image->rows=sun_info.height;
    if ((sun_info.depth == 0) || (sun_info.depth > 32))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    image->depth=sun_info.depth <= 8 ? sun_info.depth :
      MAGICKCORE_QUANTUM_DEPTH;
    if (sun_info.depth < 24)
      {
        size_t
          one;

        image->storage_class=PseudoClass;
        image->colors=sun_info.maplength;
        one=1;
        if (sun_info.maptype == RMT_NONE)
          image->colors=one << sun_info.depth;
        if (sun_info.maptype == RMT_EQUAL_RGB)
          image->colors=sun_info.maplength/3;
      }
    switch (sun_info.maptype)
    {
      case RMT_NONE:
      {
        if (sun_info.depth < 24)
          {
            /*
              Create linear color ramp.
            */
            if (AcquireImageColormap(image,image->colors) == MagickFalse)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        break;
      }
      case RMT_EQUAL_RGB:
      {
        unsigned char
          *sun_colormap;

        /*
          Read SUN raster colormap.
        */
        if (AcquireImageColormap(image,image->colors) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        sun_colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
          sizeof(*sun_colormap));
        if (sun_colormap == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        count=ReadBlob(image,image->colors,sun_colormap);
        for (i=0; i < (ssize_t) image->colors; i++)
          image->colormap[i].red=ScaleCharToQuantum(sun_colormap[i]);
        count=ReadBlob(image,image->colors,sun_colormap);
        for (i=0; i < (ssize_t) image->colors; i++)
          image->colormap[i].green=ScaleCharToQuantum(sun_colormap[i]);
        count=ReadBlob(image,image->colors,sun_colormap);
        for (i=0; i < (ssize_t) image->colors; i++)
          image->colormap[i].blue=ScaleCharToQuantum(sun_colormap[i]);
        sun_colormap=(unsigned char *) RelinquishMagickMemory(sun_colormap);
        break;
      }
      case RMT_RAW:
      {
        unsigned char
          *sun_colormap;

        /*
          Read SUN raster colormap.
        */
        sun_colormap=(unsigned char *) AcquireQuantumMemory(sun_info.maplength,
          sizeof(*sun_colormap));
        if (sun_colormap == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        count=ReadBlob(image,sun_info.maplength,sun_colormap);
        sun_colormap=(unsigned char *) RelinquishMagickMemory(sun_colormap);
        break;
      }
      default:
        ThrowReaderException(CoderError,"ColormapTypeNotSupported");
    }
    image->matte=sun_info.depth == 32 ? MagickTrue : MagickFalse;
    image->columns=sun_info.width;
    image->rows=sun_info.height;
    if (image_info->ping != MagickFalse)
      {
        (void) CloseBlob(image);
        return(GetFirstImageInList(image));
      }
    if ((sun_info.length*sizeof(*sun_data))/sizeof(*sun_data) !=
        sun_info.length || !sun_info.length)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    number_pixels=(MagickSizeType) image->columns*image->rows;
    if ((sun_info.type != RT_ENCODED) && (sun_info.depth >= 8) &&
        ((number_pixels*((sun_info.depth+7)/8)) > sun_info.length))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    sun_data=(unsigned char *) AcquireQuantumMemory((size_t) sun_info.length,
      sizeof(*sun_data));
    if (sun_data == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    count=(ssize_t) ReadBlob(image,sun_info.length,sun_data);
    if ((count == 0) && (sun_info.type != RT_ENCODED))
      ThrowReaderException(CorruptImageError,"UnableToReadImageData");
    sun_pixels=sun_data;
    bytes_per_line=0;
    if (sun_info.type == RT_ENCODED)
      {
        size_t
          height;

        /*
          Read run-length encoded raster pixels.
        */
        height=sun_info.height;
        bytes_per_line=sun_info.width*sun_info.depth;
        if ((height == 0) || (sun_info.width == 0) || (sun_info.depth == 0) ||
            ((bytes_per_line/sun_info.depth) != sun_info.width))
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        bytes_per_line+=15;
        bytes_per_line<<=1;
        if ((bytes_per_line >> 1) != (sun_info.width*sun_info.depth+15))
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        bytes_per_line>>=4;
        sun_pixels=(unsigned char *) AcquireQuantumMemory(height,
          bytes_per_line*sizeof(*sun_pixels));
        if (sun_pixels == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        (void) DecodeImage(sun_data,sun_info.length,sun_pixels,
          bytes_per_line*height);
        sun_data=(unsigned char *) RelinquishMagickMemory(sun_data);
      }
    /*
      Convert SUN raster image to pixel packets.
    */
    p=sun_pixels;
    if (sun_info.depth == 1)
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < ((ssize_t) image->columns-7); x+=8)
        {
          for (bit=7; bit >= 0; bit--)
            SetPixelIndex(indexes+x+7-bit,((*p) & (0x01 << bit) ?
              0x00 : 0x01));
          p++;
        }
        if ((image->columns % 8) != 0)
          {
            for (bit=7; bit >= (ssize_t) (8-(image->columns % 8)); bit--)
              SetPixelIndex(indexes+x+7-bit,(*p) & (0x01 << bit) ?
                0x00 : 0x01);
            p++;
          }
        if ((((image->columns/8)+(image->columns % 8 ? 1 : 0)) % 2) != 0)
          p++;
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
    else
      if (image->storage_class == PseudoClass)
        {
          length=image->rows*(image->columns+image->columns % 2);
          if (((sun_info.type == RT_ENCODED) &&
               (length > (bytes_per_line*image->rows))) ||
              ((sun_info.type != RT_ENCODED) && (length > sun_info.length)))
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (PixelPacket *) NULL)
              break;
            indexes=GetAuthenticIndexQueue(image);
            for (x=0; x < (ssize_t) image->columns; x++)
              SetPixelIndex(indexes+x,*p++);
            if ((image->columns % 2) != 0)
              p++;
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
        }
      else
        {
          size_t
            bytes_per_pixel;

          bytes_per_pixel=3;
          if (image->matte != MagickFalse)
            bytes_per_pixel++;
          length=image->rows*((bytes_per_line*image->columns)+
            image->columns % 2);
          if (((sun_info.type == RT_ENCODED) &&
               (length > (bytes_per_line*image->rows))) ||
              ((sun_info.type != RT_ENCODED) && (length > sun_info.length)))
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (PixelPacket *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (image->matte != MagickFalse)
                SetPixelAlpha(q,ScaleCharToQuantum(*p++));
              if (sun_info.type == RT_STANDARD)
                {
                  SetPixelBlue(q,ScaleCharToQuantum(*p++));
                  SetPixelGreen(q,ScaleCharToQuantum(*p++));
                  SetPixelRed(q,ScaleCharToQuantum(*p++));
                }
              else
                {
                  SetPixelRed(q,ScaleCharToQuantum(*p++));
                  SetPixelGreen(q,ScaleCharToQuantum(*p++));
                  SetPixelBlue(q,ScaleCharToQuantum(*p++));
                }
              if (image->colors != 0)
                {
                  SetPixelRed(q,image->colormap[(ssize_t)
                    GetPixelRed(q)].red);
                  SetPixelGreen(q,image->colormap[(ssize_t)
                    GetPixelGreen(q)].green);
                  SetPixelBlue(q,image->colormap[(ssize_t)
                    GetPixelBlue(q)].blue);
                }
              q++;
            }
            if (((bytes_per_pixel*image->columns) % 2) != 0)
              p++;
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
        }
    if (image->storage_class == PseudoClass)
      (void) SyncImage(image);
    sun_pixels=(unsigned char *) RelinquishMagickMemory(sun_pixels);
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
    sun_info.magic=ReadBlobMSBLong(image);
    if (sun_info.magic == 0x59a66a95)
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
  } while (sun_info.magic == 0x59a66a95);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S U N I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSUNImage() adds attributes for the SUN image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSUNImage method is:
%
%      size_t RegisterSUNImage(void)
%
*/
ModuleExport size_t RegisterSUNImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("RAS");
  entry->decoder=(DecodeImageHandler *) ReadSUNImage;
  entry->encoder=(EncodeImageHandler *) WriteSUNImage;
  entry->magick=(IsImageFormatHandler *) IsSUN;
  entry->description=ConstantString("SUN Rasterfile");
  entry->module=ConstantString("SUN");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("SUN");
  entry->decoder=(DecodeImageHandler *) ReadSUNImage;
  entry->encoder=(EncodeImageHandler *) WriteSUNImage;
  entry->description=ConstantString("SUN Rasterfile");
  entry->module=ConstantString("SUN");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S U N I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterSUNImage() removes format registrations made by the
%  SUN module from the list of supported formats.
%
%  The format of the UnregisterSUNImage method is:
%
%      UnregisterSUNImage(void)
%
*/
ModuleExport void UnregisterSUNImage(void)
{
  (void) UnregisterMagickInfo("RAS");
  (void) UnregisterMagickInfo("SUN");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S U N I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteSUNImage() writes an image in the SUN rasterfile format.
%
%  The format of the WriteSUNImage method is:
%
%      MagickBooleanType WriteSUNImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteSUNImage(const ImageInfo *image_info,Image *image)
{
#define RMT_EQUAL_RGB  1
#define RMT_NONE  0
#define RMT_RAW  2
#define RT_STANDARD  1
#define RT_FORMAT_RGB  3

  typedef struct _SUNInfo
  {
    unsigned int
      magic,
      width,
      height,
      depth,
      length,
      type,
      maptype,
      maplength;
  } SUNInfo;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  MagickSizeType
    number_pixels;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  ssize_t
    y;

  SUNInfo
    sun_info;

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
  scene=0;
  do
  {
    /*
      Initialize SUN raster file header.
    */
    if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
      (void) TransformImageColorspace(image,sRGBColorspace);
    sun_info.magic=0x59a66a95;
    if ((image->columns != (unsigned int) image->columns) ||
        (image->rows != (unsigned int) image->rows))
      ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
    sun_info.width=(unsigned int) image->columns;
    sun_info.height=(unsigned int) image->rows;
    sun_info.type=(unsigned int) (image->storage_class == DirectClass ?
      RT_FORMAT_RGB : RT_STANDARD);
    sun_info.maptype=RMT_NONE;
    sun_info.maplength=0;
    number_pixels=(MagickSizeType) image->columns*image->rows;
    if ((4*number_pixels) != (size_t) (4*number_pixels))
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    if (image->storage_class == DirectClass)
      {
        /*
          Full color SUN raster.
        */
        sun_info.depth=(unsigned int) image->matte ? 32U : 24U;
        sun_info.length=(unsigned int) ((image->matte ? 4 : 3)*number_pixels);
        sun_info.length+=sun_info.length & 0x01 ? (unsigned int) image->rows :
          0;
      }
    else
      if (IsMonochromeImage(image,&image->exception))
        {
          /*
            Monochrome SUN raster.
          */
          sun_info.depth=1;
          sun_info.length=(unsigned int) (((image->columns+7) >> 3)*
            image->rows);
          sun_info.length+=(unsigned int) (((image->columns/8)+(image->columns %
            8 ? 1 : 0)) % 2 ? image->rows : 0);
        }
      else
        {
          /*
            Colormapped SUN raster.
          */
          sun_info.depth=8;
          sun_info.length=(unsigned int) number_pixels;
          sun_info.length+=(unsigned int) (image->columns & 0x01 ? image->rows :
            0);
          sun_info.maptype=RMT_EQUAL_RGB;
          sun_info.maplength=(unsigned int) (3*image->colors);
        }
    /*
      Write SUN header.
    */
    (void) WriteBlobMSBLong(image,sun_info.magic);
    (void) WriteBlobMSBLong(image,sun_info.width);
    (void) WriteBlobMSBLong(image,sun_info.height);
    (void) WriteBlobMSBLong(image,sun_info.depth);
    (void) WriteBlobMSBLong(image,sun_info.length);
    (void) WriteBlobMSBLong(image,sun_info.type);
    (void) WriteBlobMSBLong(image,sun_info.maptype);
    (void) WriteBlobMSBLong(image,sun_info.maplength);
    /*
      Convert MIFF to SUN raster pixels.
    */
    x=0;
    y=0;
    if (image->storage_class == DirectClass)
      {
        register unsigned char
          *q;

        size_t
          bytes_per_pixel,
          length;

        unsigned char
          *pixels;

        /*
          Allocate memory for pixels.
        */
        bytes_per_pixel=3;
        if (image->matte != MagickFalse)
          bytes_per_pixel++;
        length=image->columns;
        pixels=(unsigned char *) AcquireQuantumMemory(length,4*sizeof(*pixels));
        if (pixels == (unsigned char *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        /*
          Convert DirectClass packet to SUN RGB pixel.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          q=pixels;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (image->matte != MagickFalse)
              *q++=ScaleQuantumToChar(GetPixelAlpha(p));
            *q++=ScaleQuantumToChar(GetPixelRed(p));
            *q++=ScaleQuantumToChar(GetPixelGreen(p));
            *q++=ScaleQuantumToChar(GetPixelBlue(p));
            p++;
          }
          if (((bytes_per_pixel*image->columns) & 0x01) != 0)
            *q++='\0';  /* pad scanline */
          (void) WriteBlob(image,(size_t) (q-pixels),pixels);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      }
    else
      if (IsMonochromeImage(image,&image->exception))
        {
          register unsigned char
            bit,
            byte;

          /*
            Convert PseudoClass image to a SUN monochrome image.
          */
          (void) SetImageType(image,BilevelType);
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            indexes=GetVirtualIndexQueue(image);
            bit=0;
            byte=0;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              byte<<=1;
              if (GetPixelIntensity(image,p) < (QuantumRange/2.0))
                byte|=0x01;
              bit++;
              if (bit == 8)
                {
                  (void) WriteBlobByte(image,byte);
                  bit=0;
                  byte=0;
                }
              p++;
            }
            if (bit != 0)
              (void) WriteBlobByte(image,(unsigned char) (byte << (8-bit)));
            if ((((image->columns/8)+
                (image->columns % 8 ? 1 : 0)) % 2) != 0)
              (void) WriteBlobByte(image,0);  /* pad scanline */
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
        }
      else
        {
          /*
            Dump colormap to file.
          */
          for (i=0; i < (ssize_t) image->colors; i++)
            (void) WriteBlobByte(image,ScaleQuantumToChar(
              image->colormap[i].red));
          for (i=0; i < (ssize_t) image->colors; i++)
            (void) WriteBlobByte(image,ScaleQuantumToChar(
              image->colormap[i].green));
          for (i=0; i < (ssize_t) image->colors; i++)
            (void) WriteBlobByte(image,ScaleQuantumToChar(
              image->colormap[i].blue));
          /*
            Convert PseudoClass packet to SUN colormapped pixel.
          */
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            indexes=GetVirtualIndexQueue(image);
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              (void) WriteBlobByte(image,(unsigned char)
                GetPixelIndex(indexes+x));
              p++;
            }
            if (image->columns & 0x01)
              (void) WriteBlobByte(image,0);  /* pad scanline */
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                  image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
        }
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

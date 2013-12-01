/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            DDDD   IIIII  BBBB                               %
%                            D   D    I    B   B                              %
%                            D   D    I    BBBB                               %
%                            D   D    I    B   B                              %
%                            DDDD   IIIII  BBBB                               %
%                                                                             %
%                                                                             %
%                   Read/Write Windows DIB Image Format                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
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
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colormap-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"

/*
  Typedef declarations.
*/
typedef struct _DIBInfo
{
  size_t
    size;

  ssize_t
    width,
    height;

  unsigned short
    planes,
    bits_per_pixel;

  size_t
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

  ssize_t
    colorspace;

  PointInfo
    red_primary,
    green_primary,
    blue_primary,
    gamma_scale;
} DIBInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteDIBImage(const ImageInfo *,Image *);

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
%  DecodeImage unpacks the packed image pixels into runlength-encoded
%  pixel packets.
%
%  The format of the DecodeImage method is:
%
%      MagickBooleanType DecodeImage(Image *image,
%        const MagickBooleanType compression,unsigned char *pixels)
%
%  A description of each parameter follows:
%
%    o image: the address of a structure of type Image.
%
%    o compression:  A value of 1 means the compressed pixels are runlength
%      encoded for a 256-color bitmap.  A value of 2 means a 16-color bitmap.
%
%    o pixels:  The address of a byte (8 bits) array of pixel data created by
%      the decoding process.
%
*/

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static MagickBooleanType DecodeImage(Image *image,
  const MagickBooleanType compression,unsigned char *pixels)
{
#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__MINGW32__) || defined(__MINGW64__)
#define BI_RGB  0
#define BI_RLE8  1
#define BI_RLE4  2
#define BI_BITFIELDS  3
#endif

  int
    count;

  ssize_t
    y;

  register ssize_t
    i,
    x;

  register unsigned char
    *p,
    *q;

  unsigned char
    byte;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(pixels != (unsigned char *) NULL);
  (void) ResetMagickMemory(pixels,0,(size_t) image->columns*image->rows*
    sizeof(*pixels));
  byte=0;
  x=0;
  p=pixels;
  q=pixels+(size_t) image->columns*image->rows;
  for (y=0; y < (ssize_t) image->rows; )
  {
    if ((p < pixels) || (p >= q))
      break;
    count=ReadBlobByte(image);
    if (count == EOF)
      break;
    if (count != 0)
      {
        count=(int) MagickMin((size_t) count,(size_t) (q-p));
        /*
          Encoded mode.
        */
        byte=(unsigned char) ReadBlobByte(image);
        if (compression == BI_RLE8)
          {
            for (i=0; i < count; i++)
              *p++=(unsigned char) byte;
          }
        else
          {
            for (i=0; i < count; i++)
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
        if (count == 0x01)
          return(MagickTrue);
        switch (count)
        {
          case 0x00:
          {
            /*
              End of line.
            */
            x=0;
            y++;
            p=pixels+y*image->columns;
            break;
          }
          case 0x02:
          {
            /*
              Delta mode.
            */
            x+=ReadBlobByte(image);
            y+=ReadBlobByte(image);
            p=pixels+y*image->columns+x;
            break;
          }
          default:
          {
            /*
              Absolute mode.
            */
            count=(int) MagickMin((size_t) count,(size_t) (q-p));
            if (compression == BI_RLE8)
              for (i=0; i < count; i++)
                *p++=(unsigned char) ReadBlobByte(image);
            else
              for (i=0; i < count; i++)
              {
                if ((i & 0x01) == 0)
                  byte=(unsigned char) ReadBlobByte(image);
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
                  (void) ReadBlobByte(image);
              }
            else
              if (((count & 0x03) == 1) || ((count & 0x03) == 2))
                (void) ReadBlobByte(image);
            break;
          }
        }
      }
    if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
      break;
  }
  (void) ReadBlobByte(image);  /* end of line */
  (void) ReadBlobByte(image);
  return(MagickTrue);
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
  ssize_t
    y;

  register const unsigned char
    *p;

  register ssize_t
    i,
    x;

  register unsigned char
    *q;

  /*
    Runlength encode pixels.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(pixels != (const unsigned char *) NULL);
  assert(compressed_pixels != (unsigned char *) NULL);
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
        if ((*(p+i) != *p) || (i == 255))
          break;
      *q++=(unsigned char) i;
      *q++=(*p);
      p+=i;
    }
    /*
      End of line.
    */
    *q++=0x00;
    *q++=0x00;
    if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
      break;
  }
  /*
    End of bitmap.
  */
  *q++=0;
  *q++=0x01;
  return((size_t) (q-compressed_pixels));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s D I B                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsDIB() returns MagickTrue if the image format type, identified by the
%  magick string, is DIB.
%
%  The format of the IsDIB method is:
%
%      MagickBooleanType IsDIB(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsDIB(const unsigned char *magick,const size_t length)
{
  if (length < 2)
    return(MagickFalse);
  if (memcmp(magick,"\050\000",2) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D I B I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDIBImage() reads a Microsoft Windows bitmap image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadDIBImage method is:
%
%      image=ReadDIBImage(image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline ssize_t MagickAbsoluteValue(const ssize_t x)
{
  if (x < 0)
    return(-x);
  return(x);
}

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static Image *ReadDIBImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  DIBInfo
    dib_info;

  Image
    *image;

  IndexPacket
    index;

  ssize_t
    bit,
    y;

  MagickBooleanType
    status;

  MemoryInfo
    *pixel_info;

  register IndexPacket
    *indexes;

  register ssize_t
    x;

  register PixelPacket
    *q;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    bytes_per_line,
    length;

  ssize_t
    count;

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
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Determine if this a DIB file.
  */
  (void) ResetMagickMemory(&dib_info,0,sizeof(dib_info));
  dib_info.size=ReadBlobLSBLong(image);
  if (dib_info.size!=40)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Microsoft Windows 3.X DIB image file.
  */
  dib_info.width=(short) ReadBlobLSBLong(image);
  dib_info.height=(short) ReadBlobLSBLong(image);
  dib_info.planes=ReadBlobLSBShort(image);
  dib_info.bits_per_pixel=ReadBlobLSBShort(image);
  dib_info.compression=ReadBlobLSBLong(image);
  dib_info.image_size=ReadBlobLSBLong(image);
  dib_info.x_pixels=ReadBlobLSBLong(image);
  dib_info.y_pixels=ReadBlobLSBLong(image);
  dib_info.number_colors=ReadBlobLSBLong(image);
  dib_info.colors_important=ReadBlobLSBLong(image);
  if ((dib_info.compression == BI_BITFIELDS) &&
      ((dib_info.bits_per_pixel == 16) || (dib_info.bits_per_pixel == 32)))
    {
      dib_info.red_mask=ReadBlobLSBLong(image);
      dib_info.green_mask=ReadBlobLSBLong(image);
      dib_info.blue_mask=ReadBlobLSBLong(image);
    }
  image->matte=dib_info.bits_per_pixel == 32 ? MagickTrue : MagickFalse;
  image->columns=(size_t) MagickAbsoluteValue(dib_info.width);
  image->rows=(size_t) MagickAbsoluteValue(dib_info.height);
  image->depth=8;
  if ((dib_info.number_colors != 0) || (dib_info.bits_per_pixel < 16))
    {
      size_t
        one;

      image->storage_class=PseudoClass;
      image->colors=dib_info.number_colors;
      one=1;
      if (image->colors == 0)
        image->colors=one << dib_info.bits_per_pixel;
    }
  if (image_info->size)
    {
      RectangleInfo
        geometry;

      MagickStatusType
        flags;

      flags=ParseAbsoluteGeometry(image_info->size,&geometry);
      if (flags & WidthValue)
        if ((geometry.width != 0) && (geometry.width < image->columns))
          image->columns=geometry.width;
      if (flags & HeightValue)
        if ((geometry.height != 0) && (geometry.height < image->rows))
          image->rows=geometry.height;
    }
  if (image->storage_class == PseudoClass)
    {
      size_t
        length,
        packet_size;

      unsigned char
        *dib_colormap;

      /*
        Read DIB raster colormap.
      */
      if (AcquireImageColormap(image,image->colors) == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      length=(size_t) image->colors;
      dib_colormap=(unsigned char *) AcquireQuantumMemory(length,
        4*sizeof(*dib_colormap));
      if (dib_colormap == (unsigned char *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      packet_size=4;
      count=ReadBlob(image,packet_size*image->colors,dib_colormap);
      if (count != (ssize_t) (packet_size*image->colors))
        ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
      p=dib_colormap;
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        image->colormap[i].blue=ScaleCharToQuantum(*p++);
        image->colormap[i].green=ScaleCharToQuantum(*p++);
        image->colormap[i].red=ScaleCharToQuantum(*p++);
        if (packet_size == 4)
          p++;
      }
      dib_colormap=(unsigned char *) RelinquishMagickMemory(dib_colormap);
    }
  /*
    Read image data.
  */
  if (dib_info.compression == BI_RLE4)
    dib_info.bits_per_pixel<<=1;
  bytes_per_line=4*((image->columns*dib_info.bits_per_pixel+31)/32);
  length=bytes_per_line*image->rows;
  pixel_info=AcquireVirtualMemory((size_t) image->rows,MagickMax(
    bytes_per_line,image->columns+256UL)*sizeof(*pixels));
  if (pixel_info == (MemoryInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
  if ((dib_info.compression == BI_RGB) ||
      (dib_info.compression == BI_BITFIELDS))
    {
      count=ReadBlob(image,length,pixels);
      if (count != (ssize_t) (length))
        ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    }
  else
    {
      /*
        Convert run-length encoded raster pixels.
      */
      status=DecodeImage(image,dib_info.compression ? MagickTrue : MagickFalse,
        pixels);
      if (status == MagickFalse)
        ThrowReaderException(CorruptImageError,"UnableToRunlengthDecodeImage");
    }
  /*
    Initialize image structure.
  */
  image->units=PixelsPerCentimeterResolution;
  image->x_resolution=(double) dib_info.x_pixels/100.0;
  image->y_resolution=(double) dib_info.y_pixels/100.0;
  /*
    Convert DIB raster image to pixel packets.
  */
  switch (dib_info.bits_per_pixel)
  {
    case 1:
    {
      /*
        Convert bitmap scanline.
      */
      for (y=(ssize_t) image->rows-1; y >= 0; y--)
      {
        p=pixels+(image->rows-y-1)*bytes_per_line;
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < ((ssize_t) image->columns-7); x+=8)
        {
          for (bit=0; bit < 8; bit++)
          {
            index=(IndexPacket) ((*p) & (0x80 >> bit) ? 0x01 : 0x00);
            SetPixelIndex(indexes+x+bit,index);
          }
          p++;
        }
        if ((image->columns % 8) != 0)
          {
            for (bit=0; bit < (ssize_t) (image->columns % 8); bit++)
            {
              index=(IndexPacket) ((*p) & (0x80 >> bit) ? 0x01 : 0x00);
              SetPixelIndex(indexes+x+bit,index);
            }
            p++;
          }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,image->rows-y-1,
              image->rows);
            if (status == MagickFalse)
              break;
          }
      }
      (void) SyncImage(image);
      break;
    }
    case 4:
    {
      /*
        Convert PseudoColor scanline.
      */
      for (y=(ssize_t) image->rows-1; y >= 0; y--)
      {
        p=pixels+(image->rows-y-1)*bytes_per_line;
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < ((ssize_t) image->columns-1); x+=2)
        {
          index=ConstrainColormapIndex(image,(*p >> 4) & 0xf);
          SetPixelIndex(indexes+x,index);
          index=ConstrainColormapIndex(image,*p & 0xf);
          SetPixelIndex(indexes+x+1,index);
          p++;
        }
        if ((image->columns % 2) != 0)
          {
            index=ConstrainColormapIndex(image,(*p >> 4) & 0xf);
            SetPixelIndex(indexes+x,index);
            p++;
          }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,image->rows-y-1,
              image->rows);
            if (status == MagickFalse)
              break;
          }
      }
      (void) SyncImage(image);
      break;
    }
    case 8:
    {
      /*
        Convert PseudoColor scanline.
      */
      if ((dib_info.compression == BI_RLE8) ||
          (dib_info.compression == BI_RLE4))
        bytes_per_line=image->columns;
      for (y=(ssize_t) image->rows-1; y >= 0; y--)
      {
        p=pixels+(image->rows-y-1)*bytes_per_line;
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          index=ConstrainColormapIndex(image,*p);
          SetPixelIndex(indexes+x,index);
          p++;
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,image->rows-y-1,
              image->rows);
            if (status == MagickFalse)
              break;
          }
      }
      (void) SyncImage(image);
      break;
    }
    case 16:
    {
      unsigned short
        word;

      /*
        Convert PseudoColor scanline.
      */
      image->storage_class=DirectClass;
      if (dib_info.compression == BI_RLE8)
        bytes_per_line=2*image->columns;
      for (y=(ssize_t) image->rows-1; y >= 0; y--)
      {
        p=pixels+(image->rows-y-1)*bytes_per_line;
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          word=(*p++);
          word|=(*p++ << 8);
          if (dib_info.red_mask == 0)
            {
              SetPixelRed(q,ScaleCharToQuantum(ScaleColor5to8(
                (unsigned char) ((word >> 10) & 0x1f))));
              SetPixelGreen(q,ScaleCharToQuantum(ScaleColor5to8(
                (unsigned char) ((word >> 5) & 0x1f))));
              SetPixelBlue(q,ScaleCharToQuantum(ScaleColor5to8(
                (unsigned char) (word & 0x1f))));
            }
          else
            {
              SetPixelRed(q,ScaleCharToQuantum(ScaleColor5to8(
                (unsigned char) ((word >> 11) & 0x1f))));
              SetPixelGreen(q,ScaleCharToQuantum(ScaleColor6to8(
                (unsigned char) ((word >> 5) & 0x3f))));
              SetPixelBlue(q,ScaleCharToQuantum(ScaleColor5to8(
                (unsigned char) (word & 0x1f))));
            }
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,image->rows-y-1,
              image->rows);
            if (status == MagickFalse)
              break;
          }
      }
      break;
    }
    case 24:
    case 32:
    {
      /*
        Convert DirectColor scanline.
      */
      for (y=(ssize_t) image->rows-1; y >= 0; y--)
      {
        p=pixels+(image->rows-y-1)*bytes_per_line;
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelBlue(q,ScaleCharToQuantum(*p++));
          SetPixelGreen(q,ScaleCharToQuantum(*p++));
          SetPixelRed(q,ScaleCharToQuantum(*p++));
          if (image->matte != MagickFalse)
            SetPixelOpacity(q,ScaleCharToQuantum(*p++));
          q++;
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,image->rows-y-1,
              image->rows);
            if (status == MagickFalse)
              break;
          }
      }
      break;
    }
    default:
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }
  pixel_info=RelinquishVirtualMemory(pixel_info);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (dib_info.height < 0)
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
          image=DestroyImage(image);
          image=flipped_image;
        }
    }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r D I B I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterDIBImage() adds attributes for the DIB image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterDIBImage method is:
%
%      size_t RegisterDIBImage(void)
%
*/
ModuleExport size_t RegisterDIBImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("DIB");
  entry->decoder=(DecodeImageHandler *) ReadDIBImage;
  entry->encoder=(EncodeImageHandler *) WriteDIBImage;
  entry->magick=(IsImageFormatHandler *) IsDIB;
  entry->adjoin=MagickFalse;
  entry->stealth=MagickTrue;
  entry->description=ConstantString(
    "Microsoft Windows 3.X Packed Device-Independent Bitmap");
  entry->module=ConstantString("DIB");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r D I B I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterDIBImage() removes format registrations made by the
%  DIB module from the list of supported formats.
%
%  The format of the UnregisterDIBImage method is:
%
%      UnregisterDIBImage(void)
%
*/
ModuleExport void UnregisterDIBImage(void)
{
  (void) UnregisterMagickInfo("DIB");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e D I B I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteDIBImage() writes an image in Microsoft Windows bitmap encoded
%  image format.
%
%  The format of the WriteDIBImage method is:
%
%      MagickBooleanType WriteDIBImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteDIBImage(const ImageInfo *image_info,Image *image)
{
  DIBInfo
    dib_info;

  MagickBooleanType
    status;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  register unsigned char
    *q;

  size_t
    bytes_per_line;

  ssize_t
    y;

  unsigned char
    *dib_data,
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
  /*
    Initialize DIB raster file header.
  */
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace);
  if (image->storage_class == DirectClass)
    {
      /*
        Full color DIB raster.
      */
      dib_info.number_colors=0;
      dib_info.bits_per_pixel=(unsigned short) (image->matte ? 32 : 24);
    }
  else
    {
      /*
        Colormapped DIB raster.
      */
      dib_info.bits_per_pixel=8;
      if (image_info->depth > 8)
        dib_info.bits_per_pixel=16;
      if (IsMonochromeImage(image,&image->exception) != MagickFalse)
        dib_info.bits_per_pixel=1;
      dib_info.number_colors=(dib_info.bits_per_pixel == 16) ? 0 :
        (1UL << dib_info.bits_per_pixel);
    }
  bytes_per_line=4*((image->columns*dib_info.bits_per_pixel+31)/32);
  dib_info.size=40;
  dib_info.width=(ssize_t) image->columns;
  dib_info.height=(ssize_t) image->rows;
  dib_info.planes=1;
  dib_info.compression=(size_t) (dib_info.bits_per_pixel == 16 ?
    BI_BITFIELDS : BI_RGB);
  dib_info.image_size=bytes_per_line*image->rows;
  dib_info.x_pixels=75*39;
  dib_info.y_pixels=75*39;
  switch (image->units)
  {
    case UndefinedResolution:
    case PixelsPerInchResolution:
    {
      dib_info.x_pixels=(size_t) (100.0*image->x_resolution/2.54);
      dib_info.y_pixels=(size_t) (100.0*image->y_resolution/2.54);
      break;
    }
    case PixelsPerCentimeterResolution:
    {
      dib_info.x_pixels=(size_t) (100.0*image->x_resolution);
      dib_info.y_pixels=(size_t) (100.0*image->y_resolution);
      break;
    }
  }
  dib_info.colors_important=dib_info.number_colors;
  /*
    Convert MIFF to DIB raster pixels.
  */
  pixels=(unsigned char *) AcquireQuantumMemory(dib_info.image_size,
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(pixels,0,dib_info.image_size);
  switch (dib_info.bits_per_pixel)
  {
    case 1:
    {
      register unsigned char
        bit,
        byte;

      /*
        Convert PseudoClass image to a DIB monochrome image.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        indexes=GetVirtualIndexQueue(image);
        q=pixels+(image->rows-y-1)*bytes_per_line;
        bit=0;
        byte=0;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          byte<<=1;
          byte|=GetPixelIndex(indexes+x) != 0 ? 0x01 : 0x00;
          bit++;
          if (bit == 8)
            {
              *q++=byte;
              bit=0;
              byte=0;
            }
           p++;
         }
         if (bit != 0)
           {
             *q++=(unsigned char) (byte << (8-bit));
             x++;
           }
        for (x=(ssize_t) (image->columns+7)/8; x < (ssize_t) bytes_per_line; x++)
          *q++=0x00;
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
      break;
    }
    case 8:
    {
      /*
        Convert PseudoClass packet to DIB pixel.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        indexes=GetVirtualIndexQueue(image);
        q=pixels+(image->rows-y-1)*bytes_per_line;
        for (x=0; x < (ssize_t) image->columns; x++)
          *q++=(unsigned char) GetPixelIndex(indexes+x);
        for ( ; x < (ssize_t) bytes_per_line; x++)
          *q++=0x00;
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
      break;
    }
    case 16:
    {
      unsigned short
        word;
      /*
        Convert PseudoClass packet to DIB pixel.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        q=pixels+(image->rows-y-1)*bytes_per_line;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          word=(unsigned short) ((ScaleColor8to5((unsigned char)
            ScaleQuantumToChar(GetPixelRed(p))) << 11) |
            (ScaleColor8to6((unsigned char) ScaleQuantumToChar(
            GetPixelGreen(p))) << 5) | (ScaleColor8to5((unsigned char)
            ScaleQuantumToChar((unsigned char) GetPixelBlue(p)) <<
            0)));
          *q++=(unsigned char)(word & 0xff);
          *q++=(unsigned char)(word >> 8);
          p++;
        }
        for (x=(ssize_t) (2*image->columns); x < (ssize_t) bytes_per_line; x++)
          *q++=0x00;
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
      break;
    }
    case 24:
    case 32:
    {
      /*
        Convert DirectClass packet to DIB RGB pixel.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        q=pixels+(image->rows-y-1)*bytes_per_line;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          *q++=ScaleQuantumToChar(GetPixelBlue(p));
          *q++=ScaleQuantumToChar(GetPixelGreen(p));
          *q++=ScaleQuantumToChar(GetPixelRed(p));
          if (image->matte != MagickFalse)
            *q++=ScaleQuantumToChar(GetPixelOpacity(p));
          p++;
        }
        if (dib_info.bits_per_pixel == 24)
          for (x=(ssize_t) (3*image->columns); x < (ssize_t) bytes_per_line; x++)
            *q++=0x00;
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
      break;
    }
  }
  if (dib_info.bits_per_pixel == 8)
    if (image_info->compression != NoCompression)
      {
        size_t
          length;

        /*
          Convert run-length encoded raster pixels.
        */
        length=2UL*(bytes_per_line+2UL)+2UL;
        dib_data=(unsigned char *) AcquireQuantumMemory(length,
          (image->rows+2UL)*sizeof(*dib_data));
        if (pixels == (unsigned char *) NULL)
          {
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
        dib_info.image_size=(size_t) EncodeImage(image,bytes_per_line,
          pixels,dib_data);
        pixels=(unsigned char *) RelinquishMagickMemory(pixels);
        pixels=dib_data;
        dib_info.compression = BI_RLE8;
      }
  /*
    Write DIB header.
  */
  (void) WriteBlobLSBLong(image,(unsigned int) dib_info.size);
  (void) WriteBlobLSBLong(image,dib_info.width);
  (void) WriteBlobLSBLong(image,(unsigned short) dib_info.height);
  (void) WriteBlobLSBShort(image,(unsigned short) dib_info.planes);
  (void) WriteBlobLSBShort(image,dib_info.bits_per_pixel);
  (void) WriteBlobLSBLong(image,(unsigned int) dib_info.compression);
  (void) WriteBlobLSBLong(image,(unsigned int) dib_info.image_size);
  (void) WriteBlobLSBLong(image,(unsigned int) dib_info.x_pixels);
  (void) WriteBlobLSBLong(image,(unsigned int) dib_info.y_pixels);
  (void) WriteBlobLSBLong(image,(unsigned int) dib_info.number_colors);
  (void) WriteBlobLSBLong(image,(unsigned int) dib_info.colors_important);
  if (image->storage_class == PseudoClass)
    {
      if (dib_info.bits_per_pixel <= 8)
        {
          unsigned char
            *dib_colormap;

          /*
            Dump colormap to file.
          */
          dib_colormap=(unsigned char *) AcquireQuantumMemory((size_t)
            (1UL << dib_info.bits_per_pixel),4*sizeof(dib_colormap));
          if (dib_colormap == (unsigned char *) NULL)
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          q=dib_colormap;
          for (i=0; i < (ssize_t) MagickMin(image->colors,dib_info.number_colors); i++)
          {
            *q++=ScaleQuantumToChar(image->colormap[i].blue);
            *q++=ScaleQuantumToChar(image->colormap[i].green);
            *q++=ScaleQuantumToChar(image->colormap[i].red);
            *q++=(Quantum) 0x0;
          }
          for ( ; i < (ssize_t) (1L << dib_info.bits_per_pixel); i++)
          {
            *q++=(Quantum) 0x0;
            *q++=(Quantum) 0x0;
            *q++=(Quantum) 0x0;
            *q++=(Quantum) 0x0;
          }
          (void) WriteBlob(image,(size_t) (4*(1 << dib_info.bits_per_pixel)),
            dib_colormap);
          dib_colormap=(unsigned char *) RelinquishMagickMemory(dib_colormap);
        }
      else
        if ((dib_info.bits_per_pixel == 16) &&
            (dib_info.compression == BI_BITFIELDS))
          {
            (void) WriteBlobLSBLong(image,0xf800);
            (void) WriteBlobLSBLong(image,0x07e0);
            (void) WriteBlobLSBLong(image,0x001f);
          }
    }
  (void) WriteBlob(image,dib_info.image_size,pixels);
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  (void) CloseBlob(image);
  return(MagickTrue);
}

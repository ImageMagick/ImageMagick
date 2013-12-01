/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        PPPP   IIIII   CCCC  TTTTT                           %
%                        P   P    I    C        T                             %
%                        PPPP     I    C        T                             %
%                        P        I    C        T                             %
%                        P      IIIII   CCCC    T                             %
%                                                                             %
%                                                                             %
%               Read/Write Apple Macintosh QuickDraw/PICT Format              %
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
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colormap-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/profile.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"
#include "magick/utility.h"

/*
  ImageMagick Macintosh PICT Methods.
*/
#define ReadPixmap(pixmap) \
{ \
  pixmap.version=(short) ReadBlobMSBShort(image); \
  pixmap.pack_type=(short) ReadBlobMSBShort(image); \
  pixmap.pack_size=ReadBlobMSBLong(image); \
  pixmap.horizontal_resolution=1UL*ReadBlobMSBShort(image); \
  (void) ReadBlobMSBShort(image); \
  pixmap.vertical_resolution=1UL*ReadBlobMSBShort(image); \
  (void) ReadBlobMSBShort(image); \
  pixmap.pixel_type=(short) ReadBlobMSBShort(image); \
  pixmap.bits_per_pixel=(short) ReadBlobMSBShort(image); \
  pixmap.component_count=(short) ReadBlobMSBShort(image); \
  pixmap.component_size=(short) ReadBlobMSBShort(image); \
  pixmap.plane_bytes=ReadBlobMSBLong(image); \
  pixmap.table=ReadBlobMSBLong(image); \
  pixmap.reserved=ReadBlobMSBLong(image); \
  if ((pixmap.bits_per_pixel <= 0) || (pixmap.bits_per_pixel > 32) || \
      (pixmap.component_count <= 0) || (pixmap.component_count > 4) || \
      (pixmap.component_size <= 0)) \
    ThrowReaderException(CorruptImageError,"ImproperImageHeader"); \
}

#define ReadRectangle(image,rectangle) \
{ \
  rectangle.top=(short) ReadBlobMSBShort(image); \
  rectangle.left=(short) ReadBlobMSBShort(image); \
  rectangle.bottom=(short) ReadBlobMSBShort(image); \
  rectangle.right=(short) ReadBlobMSBShort(image); \
  if ((rectangle.left > rectangle.right) || \
      (rectangle.top > rectangle.bottom)) \
    ThrowReaderException(CorruptImageError,"ImproperImageHeader"); \
}

typedef struct _PICTCode
{
  const char
    *name;

  ssize_t
    length;

  const char
    *description;
} PICTCode;

typedef struct _PICTPixmap
{
  short
    version,
    pack_type;

  size_t
    pack_size,
    horizontal_resolution,
    vertical_resolution;

  short
    pixel_type,
    bits_per_pixel,
    component_count,
    component_size;

  size_t
    plane_bytes,
    table,
    reserved;
} PICTPixmap;

typedef struct _PICTRectangle
{
  short
    top,
    left,
    bottom,
    right;
} PICTRectangle;

static const PICTCode
  codes[] =
  {
    /* 0x00 */ { "NOP", 0, "nop" },
    /* 0x01 */ { "Clip", 0, "clip" },
    /* 0x02 */ { "BkPat", 8, "background pattern" },
    /* 0x03 */ { "TxFont", 2, "text font (word)" },
    /* 0x04 */ { "TxFace", 1, "text face (byte)" },
    /* 0x05 */ { "TxMode", 2, "text mode (word)" },
    /* 0x06 */ { "SpExtra", 4, "space extra (fixed point)" },
    /* 0x07 */ { "PnSize", 4, "pen size (point)" },
    /* 0x08 */ { "PnMode", 2, "pen mode (word)" },
    /* 0x09 */ { "PnPat", 8, "pen pattern" },
    /* 0x0a */ { "FillPat", 8, "fill pattern" },
    /* 0x0b */ { "OvSize", 4, "oval size (point)" },
    /* 0x0c */ { "Origin", 4, "dh, dv (word)" },
    /* 0x0d */ { "TxSize", 2, "text size (word)" },
    /* 0x0e */ { "FgColor", 4, "foreground color (ssize_tword)" },
    /* 0x0f */ { "BkColor", 4, "background color (ssize_tword)" },
    /* 0x10 */ { "TxRatio", 8, "numerator (point), denominator (point)" },
    /* 0x11 */ { "Version", 1, "version (byte)" },
    /* 0x12 */ { "BkPixPat", 0, "color background pattern" },
    /* 0x13 */ { "PnPixPat", 0, "color pen pattern" },
    /* 0x14 */ { "FillPixPat", 0, "color fill pattern" },
    /* 0x15 */ { "PnLocHFrac", 2, "fractional pen position" },
    /* 0x16 */ { "ChExtra", 2, "extra for each character" },
    /* 0x17 */ { "reserved", 0, "reserved for Apple use" },
    /* 0x18 */ { "reserved", 0, "reserved for Apple use" },
    /* 0x19 */ { "reserved", 0, "reserved for Apple use" },
    /* 0x1a */ { "RGBFgCol", 6, "RGB foreColor" },
    /* 0x1b */ { "RGBBkCol", 6, "RGB backColor" },
    /* 0x1c */ { "HiliteMode", 0, "hilite mode flag" },
    /* 0x1d */ { "HiliteColor", 6, "RGB hilite color" },
    /* 0x1e */ { "DefHilite", 0, "Use default hilite color" },
    /* 0x1f */ { "OpColor", 6, "RGB OpColor for arithmetic modes" },
    /* 0x20 */ { "Line", 8, "pnLoc (point), newPt (point)" },
    /* 0x21 */ { "LineFrom", 4, "newPt (point)" },
    /* 0x22 */ { "ShortLine", 6, "pnLoc (point, dh, dv (-128 .. 127))" },
    /* 0x23 */ { "ShortLineFrom", 2, "dh, dv (-128 .. 127)" },
    /* 0x24 */ { "reserved", -1, "reserved for Apple use" },
    /* 0x25 */ { "reserved", -1, "reserved for Apple use" },
    /* 0x26 */ { "reserved", -1, "reserved for Apple use" },
    /* 0x27 */ { "reserved", -1, "reserved for Apple use" },
    /* 0x28 */ { "LongText", 0, "txLoc (point), count (0..255), text" },
    /* 0x29 */ { "DHText", 0, "dh (0..255), count (0..255), text" },
    /* 0x2a */ { "DVText", 0, "dv (0..255), count (0..255), text" },
    /* 0x2b */ { "DHDVText", 0, "dh, dv (0..255), count (0..255), text" },
    /* 0x2c */ { "reserved", -1, "reserved for Apple use" },
    /* 0x2d */ { "reserved", -1, "reserved for Apple use" },
    /* 0x2e */ { "reserved", -1, "reserved for Apple use" },
    /* 0x2f */ { "reserved", -1, "reserved for Apple use" },
    /* 0x30 */ { "frameRect", 8, "rect" },
    /* 0x31 */ { "paintRect", 8, "rect" },
    /* 0x32 */ { "eraseRect", 8, "rect" },
    /* 0x33 */ { "invertRect", 8, "rect" },
    /* 0x34 */ { "fillRect", 8, "rect" },
    /* 0x35 */ { "reserved", 8, "reserved for Apple use" },
    /* 0x36 */ { "reserved", 8, "reserved for Apple use" },
    /* 0x37 */ { "reserved", 8, "reserved for Apple use" },
    /* 0x38 */ { "frameSameRect", 0, "rect" },
    /* 0x39 */ { "paintSameRect", 0, "rect" },
    /* 0x3a */ { "eraseSameRect", 0, "rect" },
    /* 0x3b */ { "invertSameRect", 0, "rect" },
    /* 0x3c */ { "fillSameRect", 0, "rect" },
    /* 0x3d */ { "reserved", 0, "reserved for Apple use" },
    /* 0x3e */ { "reserved", 0, "reserved for Apple use" },
    /* 0x3f */ { "reserved", 0, "reserved for Apple use" },
    /* 0x40 */ { "frameRRect", 8, "rect" },
    /* 0x41 */ { "paintRRect", 8, "rect" },
    /* 0x42 */ { "eraseRRect", 8, "rect" },
    /* 0x43 */ { "invertRRect", 8, "rect" },
    /* 0x44 */ { "fillRRrect", 8, "rect" },
    /* 0x45 */ { "reserved", 8, "reserved for Apple use" },
    /* 0x46 */ { "reserved", 8, "reserved for Apple use" },
    /* 0x47 */ { "reserved", 8, "reserved for Apple use" },
    /* 0x48 */ { "frameSameRRect", 0, "rect" },
    /* 0x49 */ { "paintSameRRect", 0, "rect" },
    /* 0x4a */ { "eraseSameRRect", 0, "rect" },
    /* 0x4b */ { "invertSameRRect", 0, "rect" },
    /* 0x4c */ { "fillSameRRect", 0, "rect" },
    /* 0x4d */ { "reserved", 0, "reserved for Apple use" },
    /* 0x4e */ { "reserved", 0, "reserved for Apple use" },
    /* 0x4f */ { "reserved", 0, "reserved for Apple use" },
    /* 0x50 */ { "frameOval", 8, "rect" },
    /* 0x51 */ { "paintOval", 8, "rect" },
    /* 0x52 */ { "eraseOval", 8, "rect" },
    /* 0x53 */ { "invertOval", 8, "rect" },
    /* 0x54 */ { "fillOval", 8, "rect" },
    /* 0x55 */ { "reserved", 8, "reserved for Apple use" },
    /* 0x56 */ { "reserved", 8, "reserved for Apple use" },
    /* 0x57 */ { "reserved", 8, "reserved for Apple use" },
    /* 0x58 */ { "frameSameOval", 0, "rect" },
    /* 0x59 */ { "paintSameOval", 0, "rect" },
    /* 0x5a */ { "eraseSameOval", 0, "rect" },
    /* 0x5b */ { "invertSameOval", 0, "rect" },
    /* 0x5c */ { "fillSameOval", 0, "rect" },
    /* 0x5d */ { "reserved", 0, "reserved for Apple use" },
    /* 0x5e */ { "reserved", 0, "reserved for Apple use" },
    /* 0x5f */ { "reserved", 0, "reserved for Apple use" },
    /* 0x60 */ { "frameArc", 12, "rect, startAngle, arcAngle" },
    /* 0x61 */ { "paintArc", 12, "rect, startAngle, arcAngle" },
    /* 0x62 */ { "eraseArc", 12, "rect, startAngle, arcAngle" },
    /* 0x63 */ { "invertArc", 12, "rect, startAngle, arcAngle" },
    /* 0x64 */ { "fillArc", 12, "rect, startAngle, arcAngle" },
    /* 0x65 */ { "reserved", 12, "reserved for Apple use" },
    /* 0x66 */ { "reserved", 12, "reserved for Apple use" },
    /* 0x67 */ { "reserved", 12, "reserved for Apple use" },
    /* 0x68 */ { "frameSameArc", 4, "rect, startAngle, arcAngle" },
    /* 0x69 */ { "paintSameArc", 4, "rect, startAngle, arcAngle" },
    /* 0x6a */ { "eraseSameArc", 4, "rect, startAngle, arcAngle" },
    /* 0x6b */ { "invertSameArc", 4, "rect, startAngle, arcAngle" },
    /* 0x6c */ { "fillSameArc", 4, "rect, startAngle, arcAngle" },
    /* 0x6d */ { "reserved", 4, "reserved for Apple use" },
    /* 0x6e */ { "reserved", 4, "reserved for Apple use" },
    /* 0x6f */ { "reserved", 4, "reserved for Apple use" },
    /* 0x70 */ { "framePoly", 0, "poly" },
    /* 0x71 */ { "paintPoly", 0, "poly" },
    /* 0x72 */ { "erasePoly", 0, "poly" },
    /* 0x73 */ { "invertPoly", 0, "poly" },
    /* 0x74 */ { "fillPoly", 0, "poly" },
    /* 0x75 */ { "reserved", 0, "reserved for Apple use" },
    /* 0x76 */ { "reserved", 0, "reserved for Apple use" },
    /* 0x77 */ { "reserved", 0, "reserved for Apple use" },
    /* 0x78 */ { "frameSamePoly", 0, "poly (NYI)" },
    /* 0x79 */ { "paintSamePoly", 0, "poly (NYI)" },
    /* 0x7a */ { "eraseSamePoly", 0, "poly (NYI)" },
    /* 0x7b */ { "invertSamePoly", 0, "poly (NYI)" },
    /* 0x7c */ { "fillSamePoly", 0, "poly (NYI)" },
    /* 0x7d */ { "reserved", 0, "reserved for Apple use" },
    /* 0x7e */ { "reserved", 0, "reserved for Apple use" },
    /* 0x7f */ { "reserved", 0, "reserved for Apple use" },
    /* 0x80 */ { "frameRgn", 0, "region" },
    /* 0x81 */ { "paintRgn", 0, "region" },
    /* 0x82 */ { "eraseRgn", 0, "region" },
    /* 0x83 */ { "invertRgn", 0, "region" },
    /* 0x84 */ { "fillRgn", 0, "region" },
    /* 0x85 */ { "reserved", 0, "reserved for Apple use" },
    /* 0x86 */ { "reserved", 0, "reserved for Apple use" },
    /* 0x87 */ { "reserved", 0, "reserved for Apple use" },
    /* 0x88 */ { "frameSameRgn", 0, "region (NYI)" },
    /* 0x89 */ { "paintSameRgn", 0, "region (NYI)" },
    /* 0x8a */ { "eraseSameRgn", 0, "region (NYI)" },
    /* 0x8b */ { "invertSameRgn", 0, "region (NYI)" },
    /* 0x8c */ { "fillSameRgn", 0, "region (NYI)" },
    /* 0x8d */ { "reserved", 0, "reserved for Apple use" },
    /* 0x8e */ { "reserved", 0, "reserved for Apple use" },
    /* 0x8f */ { "reserved", 0, "reserved for Apple use" },
    /* 0x90 */ { "BitsRect", 0, "copybits, rect clipped" },
    /* 0x91 */ { "BitsRgn", 0, "copybits, rgn clipped" },
    /* 0x92 */ { "reserved", -1, "reserved for Apple use" },
    /* 0x93 */ { "reserved", -1, "reserved for Apple use" },
    /* 0x94 */ { "reserved", -1, "reserved for Apple use" },
    /* 0x95 */ { "reserved", -1, "reserved for Apple use" },
    /* 0x96 */ { "reserved", -1, "reserved for Apple use" },
    /* 0x97 */ { "reserved", -1, "reserved for Apple use" },
    /* 0x98 */ { "PackBitsRect", 0, "packed copybits, rect clipped" },
    /* 0x99 */ { "PackBitsRgn", 0, "packed copybits, rgn clipped" },
    /* 0x9a */ { "DirectBitsRect", 0, "PixMap, srcRect, dstRect, mode, PixData" },
    /* 0x9b */ { "DirectBitsRgn", 0, "PixMap, srcRect, dstRect, mode, maskRgn, PixData" },
    /* 0x9c */ { "reserved", -1, "reserved for Apple use" },
    /* 0x9d */ { "reserved", -1, "reserved for Apple use" },
    /* 0x9e */ { "reserved", -1, "reserved for Apple use" },
    /* 0x9f */ { "reserved", -1, "reserved for Apple use" },
    /* 0xa0 */ { "ShortComment", 2, "kind (word)" },
    /* 0xa1 */ { "LongComment", 0, "kind (word), size (word), data" }
  };

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePICTImage(const ImageInfo *,Image *);

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
%  DecodeImage decompresses an image via Macintosh pack bits decoding for
%  Macintosh PICT images.
%
%  The format of the DecodeImage method is:
%
%      unsigned char *DecodeImage(Image *blob,Image *image,
%        size_t bytes_per_line,const int bits_per_pixel,
%        unsigned size_t extent)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o blob,image: the address of a structure of type Image.
%
%    o bytes_per_line: This integer identifies the number of bytes in a
%      scanline.
%
%    o bits_per_pixel: the number of bits in a pixel.
%
%    o extent: the number of pixels allocated.
%
*/

static unsigned char *ExpandBuffer(unsigned char *pixels,
  MagickSizeType *bytes_per_line,const unsigned int bits_per_pixel)
{
  register ssize_t
    i;

  register unsigned char
    *p,
    *q;

  static unsigned char
    scanline[8*256];

  p=pixels;
  q=scanline;
  switch (bits_per_pixel)
  {
    case 8:
    case 16:
    case 32:
      return(pixels);
    case 4:
    {
      for (i=0; i < (ssize_t) *bytes_per_line; i++)
      {
        *q++=(*p >> 4) & 0xff;
        *q++=(*p & 15);
        p++;
      }
      *bytes_per_line*=2;
      break;
    }
    case 2:
    {
      for (i=0; i < (ssize_t) *bytes_per_line; i++)
      {
        *q++=(*p >> 6) & 0x03;
        *q++=(*p >> 4) & 0x03;
        *q++=(*p >> 2) & 0x03;
        *q++=(*p & 3);
        p++;
      }
      *bytes_per_line*=4;
      break;
    }
    case 1:
    {
      for (i=0; i < (ssize_t) *bytes_per_line; i++)
      {
        *q++=(*p >> 7) & 0x01;
        *q++=(*p >> 6) & 0x01;
        *q++=(*p >> 5) & 0x01;
        *q++=(*p >> 4) & 0x01;
        *q++=(*p >> 3) & 0x01;
        *q++=(*p >> 2) & 0x01;
        *q++=(*p >> 1) & 0x01;
        *q++=(*p & 0x01);
        p++;
      }
      *bytes_per_line*=8;
      break;
    }
    default:
      break;
  }
  return(scanline);
}

static unsigned char *DecodeImage(Image *blob,Image *image,
  size_t bytes_per_line,const unsigned int bits_per_pixel,size_t *extent)
{
  MagickSizeType
    number_pixels;

  register ssize_t
    i;

  register unsigned char
    *p,
    *q;

  size_t
    bytes_per_pixel,
    length,
    row_bytes,
    scanline_length,
    width;

  ssize_t
    count,
    j,
    y;

  unsigned char
    *pixels,
    *scanline;

  /*
    Determine pixel buffer size.
  */
  if (bits_per_pixel <= 8)
    bytes_per_line&=0x7fff;
  width=image->columns;
  bytes_per_pixel=1;
  if (bits_per_pixel == 16)
    {
      bytes_per_pixel=2;
      width*=2;
    }
  else
    if (bits_per_pixel == 32)
      width*=image->matte ? 4 : 3;
  if (bytes_per_line == 0)
    bytes_per_line=width;
  row_bytes=(size_t) (image->columns | 0x8000);
  if (image->storage_class == DirectClass)
    row_bytes=(size_t) ((4*image->columns) | 0x8000);
  /*
    Allocate pixel and scanline buffer.
  */
  pixels=(unsigned char *) AcquireQuantumMemory(image->rows,row_bytes*
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    return((unsigned char *) NULL);
  *extent=row_bytes*image->rows*sizeof(*pixels);
  (void) ResetMagickMemory(pixels,0,*extent);
  scanline=(unsigned char *) AcquireQuantumMemory(row_bytes,sizeof(*scanline));
  if (scanline == (unsigned char *) NULL)
    return((unsigned char *) NULL);
  if (bytes_per_line < 8)
    {
      /*
        Pixels are already uncompressed.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        q=pixels+y*width;
        number_pixels=bytes_per_line;
        count=ReadBlob(blob,(size_t) number_pixels,scanline);
        (void) count;
        p=ExpandBuffer(scanline,&number_pixels,bits_per_pixel);
        if ((q+number_pixels) > (pixels+(*extent)))
          {
            (void) ThrowMagickException(&image->exception,GetMagickModule(),
              CorruptImageError,"UnableToUncompressImage","`%s'",
              image->filename);
            break;
          }
        (void) CopyMagickMemory(q,p,(size_t) number_pixels);
      }
      scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      return(pixels);
    }
  /*
    Uncompress RLE pixels into uncompressed pixel buffer.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=pixels+y*width;
    if (bytes_per_line > 200)
      scanline_length=ReadBlobMSBShort(blob);
    else
      scanline_length=1UL*ReadBlobByte(blob);
    if (scanline_length >= row_bytes)
      {
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          CorruptImageError,"UnableToUncompressImage","`%s'",image->filename);
        break;
      }
    count=ReadBlob(blob,scanline_length,scanline);
    for (j=0; j < (ssize_t) scanline_length; )
      if ((scanline[j] & 0x80) == 0)
        {
          length=(size_t) ((scanline[j] & 0xff)+1);
          number_pixels=length*bytes_per_pixel;
          p=ExpandBuffer(scanline+j+1,&number_pixels,bits_per_pixel);
          if ((q-pixels+number_pixels) <= *extent)
            (void) CopyMagickMemory(q,p,(size_t) number_pixels);
          q+=number_pixels;
          j+=(ssize_t) (length*bytes_per_pixel+1);
        }
      else
        {
          length=(size_t) (((scanline[j] ^ 0xff) & 0xff)+2);
          number_pixels=bytes_per_pixel;
          p=ExpandBuffer(scanline+j+1,&number_pixels,bits_per_pixel);
          for (i=0; i < (ssize_t) length; i++)
          {
            if ((q-pixels+number_pixels) <= *extent)
              (void) CopyMagickMemory(q,p,(size_t) number_pixels);
            q+=number_pixels;
          }
          j+=(ssize_t) bytes_per_pixel+1;
        }
  }
  scanline=(unsigned char *) RelinquishMagickMemory(scanline);
  return(pixels);
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
%  EncodeImage compresses an image via Macintosh pack bits encoding
%  for Macintosh PICT images.
%
%  The format of the EncodeImage method is:
%
%      size_t EncodeImage(Image *image,const unsigned char *scanline,
%        const size_t bytes_per_line,unsigned char *pixels)
%
%  A description of each parameter follows:
%
%    o image: the address of a structure of type Image.
%
%    o scanline: A pointer to an array of characters to pack.
%
%    o bytes_per_line: the number of bytes in a scanline.
%
%    o pixels: A pointer to an array of characters where the packed
%      characters are stored.
%
*/
static size_t EncodeImage(Image *image,const unsigned char *scanline,
  const size_t bytes_per_line,unsigned char *pixels)
{
#define MaxCount  128
#define MaxPackbitsRunlength  128

  register const unsigned char
    *p;

  register ssize_t
    i;

  register unsigned char
    *q;

  size_t
    length;

  ssize_t
    count,
    repeat_count,
    runlength;

  unsigned char
    index;

  /*
    Pack scanline.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(scanline != (unsigned char *) NULL);
  assert(pixels != (unsigned char *) NULL);
  count=0;
  runlength=0;
  p=scanline+(bytes_per_line-1);
  q=pixels;
  index=(*p);
  for (i=(ssize_t) bytes_per_line-1; i >= 0; i--)
  {
    if (index == *p)
      runlength++;
    else
      {
        if (runlength < 3)
          while (runlength > 0)
          {
            *q++=(unsigned char) index;
            runlength--;
            count++;
            if (count == MaxCount)
              {
                *q++=(unsigned char) (MaxCount-1);
                count-=MaxCount;
              }
          }
        else
          {
            if (count > 0)
              *q++=(unsigned char) (count-1);
            count=0;
            while (runlength > 0)
            {
              repeat_count=runlength;
              if (repeat_count > MaxPackbitsRunlength)
                repeat_count=MaxPackbitsRunlength;
              *q++=(unsigned char) index;
              *q++=(unsigned char) (257-repeat_count);
              runlength-=repeat_count;
            }
          }
        runlength=1;
      }
    index=(*p);
    p--;
  }
  if (runlength < 3)
    while (runlength > 0)
    {
      *q++=(unsigned char) index;
      runlength--;
      count++;
      if (count == MaxCount)
        {
          *q++=(unsigned char) (MaxCount-1);
          count-=MaxCount;
        }
    }
  else
    {
      if (count > 0)
        *q++=(unsigned char) (count-1);
      count=0;
      while (runlength > 0)
      {
        repeat_count=runlength;
        if (repeat_count > MaxPackbitsRunlength)
          repeat_count=MaxPackbitsRunlength;
        *q++=(unsigned char) index;
        *q++=(unsigned char) (257-repeat_count);
        runlength-=repeat_count;
      }
    }
  if (count > 0)
    *q++=(unsigned char) (count-1);
  /*
    Write the number of and the packed length.
  */
  length=(size_t) (q-pixels);
  if (bytes_per_line > 200)
    {
      (void) WriteBlobMSBShort(image,(unsigned short) length);
      length+=2;
    }
  else
    {
      (void) WriteBlobByte(image,(unsigned char) length);
      length++;
    }
  while (q != pixels)
  {
    q--;
    (void) WriteBlobByte(image,*q);
  }
  return(length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P I C T                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPICT()() returns MagickTrue if the image format type, identified by the
%  magick string, is PICT.
%
%  The format of the ReadPICTImage method is:
%
%      MagickBooleanType IsPICT(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsPICT(const unsigned char *magick,const size_t length)
{
  /*
    Embedded OLE2 macintosh have "PICT" instead of 512 platform header.
  */
  if (length < 12)
    return(MagickFalse);
  if (memcmp(magick,"PICT",4) == 0)
    return(MagickTrue);
  if (length < 528)
    return(MagickFalse);
  if (memcmp(magick+522,"\000\021\002\377\014\000",6) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

#if !defined(macintosh)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P I C T I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPICTImage() reads an Apple Macintosh QuickDraw/PICT image file
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadPICTImage method is:
%
%      Image *ReadPICTImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t MagickMax(const size_t x,
  const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static Image *ReadPICTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    geometry[MaxTextExtent],
    header_ole[4];

  Image
    *image;

  IndexPacket
    index;

  int
    c,
    code;

  MagickBooleanType
    jpeg,
    status;

  PICTRectangle
    frame;

  PICTPixmap
    pixmap;

  register IndexPacket
    *indexes;

  register ssize_t
    x;

  register PixelPacket
    *q;

  register ssize_t
    i;

  size_t
    extent,
    length;

  ssize_t
    count,
    flags,
    j,
    version,
    y;

  StringInfo
    *profile;

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
    Read PICT header.
  */
  pixmap.bits_per_pixel=0;
  pixmap.component_count=0;
  /*
    Skip header : 512 for standard PICT and 4, ie "PICT" for OLE2
  */
  header_ole[0]=ReadBlobByte(image);
  header_ole[1]=ReadBlobByte(image);
  header_ole[2]=ReadBlobByte(image);
  header_ole[3]=ReadBlobByte(image);
  if (!((header_ole[0] == 0x50) && (header_ole[1] == 0x49) &&
      (header_ole[2] == 0x43) && (header_ole[3] == 0x54)))
    for (i=0; i < 508; i++)
      (void) ReadBlobByte(image);
  (void) ReadBlobMSBShort(image);  /* skip picture size */
  ReadRectangle(image,frame);
  while ((c=ReadBlobByte(image)) == 0) ;
  if (c != 0x11)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  version=ReadBlobByte(image);
  if (version == 2)
    {
      c=ReadBlobByte(image);
      if (c != 0xff)
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  else
    if (version != 1)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if ((frame.left < 0) || (frame.right < 0) || (frame.top < 0) ||
      (frame.bottom < 0) || (frame.left >= frame.right) ||
      (frame.top >= frame.bottom))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Create black canvas.
  */
  flags=0;
  image->depth=8;
  image->columns=1UL*(frame.right-frame.left);
  image->rows=1UL*(frame.bottom-frame.top);
  image->x_resolution=DefaultResolution;
  image->y_resolution=DefaultResolution;
  image->units=UndefinedResolution;
  /*
    Interpret PICT opcodes.
  */
  jpeg=MagickFalse;
  for (code=0; EOFBlob(image) == MagickFalse; )
  {
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if ((version == 1) || ((TellBlob(image) % 2) != 0))
      code=ReadBlobByte(image);
    if (version == 2)
      code=(int) ReadBlobMSBShort(image);
    if (code > 0xa1)
      {
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"%04X:",code);
      }
    else
      {
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  %04X %s: %s",code,codes[code].name,codes[code].description);
        switch (code)
        {
          case 0x01:
          {
            /*
              Clipping rectangle.
            */
            length=ReadBlobMSBShort(image);
            if (length != 0x000a)
              {
                for (i=0; i < (ssize_t) (length-2); i++)
                  (void) ReadBlobByte(image);
                break;
              }
            ReadRectangle(image,frame);
            if (((frame.left & 0x8000) != 0) || ((frame.top & 0x8000) != 0))
              break;
            image->columns=1UL*(frame.right-frame.left);
            image->rows=1UL*(frame.bottom-frame.top);
            (void) SetImageBackgroundColor(image);
            break;
          }
          case 0x12:
          case 0x13:
          case 0x14:
          {
            ssize_t
              pattern;

            size_t
              height,
              width;

            /*
              Skip pattern definition.
            */
            pattern=1L*ReadBlobMSBShort(image);
            for (i=0; i < 8; i++)
              (void) ReadBlobByte(image);
            if (pattern == 2)
              {
                for (i=0; i < 5; i++)
                  (void) ReadBlobByte(image);
                break;
              }
            if (pattern != 1)
              ThrowReaderException(CorruptImageError,"UnknownPatternType");
            length=ReadBlobMSBShort(image);
            ReadRectangle(image,frame);
            ReadPixmap(pixmap);
            image->depth=1UL*pixmap.component_size;
            image->x_resolution=1.0*pixmap.horizontal_resolution;
            image->y_resolution=1.0*pixmap.vertical_resolution;
            image->units=PixelsPerInchResolution;
            (void) ReadBlobMSBLong(image);
            flags=1L*ReadBlobMSBShort(image);
            length=ReadBlobMSBShort(image);
            for (i=0; i <= (ssize_t) length; i++)
              (void) ReadBlobMSBLong(image);
            width=1UL*(frame.bottom-frame.top);
            height=1UL*(frame.right-frame.left);
            if (pixmap.bits_per_pixel <= 8)
              length&=0x7fff;
            if (pixmap.bits_per_pixel == 16)
              width<<=1;
            if (length == 0)
              length=width;
            if (length < 8)
              {
                for (i=0; i < (ssize_t) (length*height); i++)
                  (void) ReadBlobByte(image);
              }
            else
              for (j=0; j < (int) height; j++)
                if (length > 200)
                  for (j=0; j < (ssize_t) ReadBlobMSBShort(image); j++)
                    (void) ReadBlobByte(image);
                else
                  for (j=0; j < (ssize_t) ReadBlobByte(image); j++)
                    (void) ReadBlobByte(image);
            break;
          }
          case 0x1b:
          {
            /*
              Initialize image background color.
            */
            image->background_color.red=(Quantum)
              ScaleShortToQuantum(ReadBlobMSBShort(image));
            image->background_color.green=(Quantum)
              ScaleShortToQuantum(ReadBlobMSBShort(image));
            image->background_color.blue=(Quantum)
              ScaleShortToQuantum(ReadBlobMSBShort(image));
            break;
          }
          case 0x70:
          case 0x71:
          case 0x72:
          case 0x73:
          case 0x74:
          case 0x75:
          case 0x76:
          case 0x77:
          {
            /*
              Skip polygon or region.
            */
            length=ReadBlobMSBShort(image);
            for (i=0; i < (ssize_t) (length-2); i++)
              (void) ReadBlobByte(image);
            break;
          }
          case 0x90:
          case 0x91:
          case 0x98:
          case 0x99:
          case 0x9a:
          case 0x9b:
          {
            ssize_t
              bytes_per_line;

            PICTRectangle
              source,
              destination;

            register unsigned char
              *p;

            size_t
              j;

            unsigned char
              *pixels;

            Image
              *tile_image;

            /*
              Pixmap clipped by a rectangle.
            */
            bytes_per_line=0;
            if ((code != 0x9a) && (code != 0x9b))
              bytes_per_line=1L*ReadBlobMSBShort(image);
            else
              {
                (void) ReadBlobMSBShort(image);
                (void) ReadBlobMSBShort(image);
                (void) ReadBlobMSBShort(image);
              }
            ReadRectangle(image,frame);
            /*
              Initialize tile image.
            */
            tile_image=CloneImage(image,1UL*(frame.right-frame.left),
              1UL*(frame.bottom-frame.top),MagickTrue,exception);
            if (tile_image == (Image *) NULL)
              return((Image *) NULL);
            if ((code == 0x9a) || (code == 0x9b) ||
                ((bytes_per_line & 0x8000) != 0))
              {
                ReadPixmap(pixmap);
                tile_image->depth=1UL*pixmap.component_size;
                tile_image->matte=pixmap.component_count == 4 ?
                  MagickTrue : MagickFalse;
                tile_image->x_resolution=(double) pixmap.horizontal_resolution;
                tile_image->y_resolution=(double) pixmap.vertical_resolution;
                tile_image->units=PixelsPerInchResolution;
                if (tile_image->matte != MagickFalse)
                  image->matte=tile_image->matte;
              }
            if ((code != 0x9a) && (code != 0x9b))
              {
                /*
                  Initialize colormap.
                */
                tile_image->colors=2;
                if ((bytes_per_line & 0x8000) != 0)
                  {
                    (void) ReadBlobMSBLong(image);
                    flags=1L*ReadBlobMSBShort(image);
                    tile_image->colors=1UL*ReadBlobMSBShort(image)+1;
                  }
                status=AcquireImageColormap(tile_image,tile_image->colors);
                if (status == MagickFalse)
                  {
                    tile_image=DestroyImage(tile_image);
                    ThrowReaderException(ResourceLimitError,
                      "MemoryAllocationFailed");
                  }
                if ((bytes_per_line & 0x8000) != 0)
                  {
                    for (i=0; i < (ssize_t) tile_image->colors; i++)
                    {
                      j=ReadBlobMSBShort(image) % tile_image->colors;
                      if ((flags & 0x8000) != 0)
                        j=(size_t) i;
                      tile_image->colormap[j].red=(Quantum)
                        ScaleShortToQuantum(ReadBlobMSBShort(image));
                      tile_image->colormap[j].green=(Quantum)
                        ScaleShortToQuantum(ReadBlobMSBShort(image));
                      tile_image->colormap[j].blue=(Quantum)
                        ScaleShortToQuantum(ReadBlobMSBShort(image));
                    }
                  }
                else
                  {
                    for (i=0; i < (ssize_t) tile_image->colors; i++)
                    {
                      tile_image->colormap[i].red=(Quantum) (QuantumRange-
                        tile_image->colormap[i].red);
                      tile_image->colormap[i].green=(Quantum) (QuantumRange-
                        tile_image->colormap[i].green);
                      tile_image->colormap[i].blue=(Quantum) (QuantumRange-
                        tile_image->colormap[i].blue);
                    }
                  }
              }
            ReadRectangle(image,source);
            ReadRectangle(image,destination);
            (void) ReadBlobMSBShort(image);
            if ((code == 0x91) || (code == 0x99) || (code == 0x9b))
              {
                /*
                  Skip region.
                */
                length=ReadBlobMSBShort(image);
                for (i=0; i < (ssize_t) (length-2); i++)
                  (void) ReadBlobByte(image);
              }
            if ((code != 0x9a) && (code != 0x9b) &&
                (bytes_per_line & 0x8000) == 0)
              pixels=DecodeImage(image,tile_image,1UL*bytes_per_line,1,&extent);
            else
              pixels=DecodeImage(image,tile_image,1UL*bytes_per_line,1U*
                pixmap.bits_per_pixel,&extent);
            if (pixels == (unsigned char *) NULL)
              {
                tile_image=DestroyImage(tile_image);
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            /*
              Convert PICT tile image to pixel packets.
            */
            p=pixels;
            for (y=0; y < (ssize_t) tile_image->rows; y++)
            {
              if (p > (pixels+extent+image->columns))
                ThrowReaderException(CorruptImageError,"NotEnoughPixelData");
              q=QueueAuthenticPixels(tile_image,0,y,tile_image->columns,1,
                exception);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=GetAuthenticIndexQueue(tile_image);
              for (x=0; x < (ssize_t) tile_image->columns; x++)
              {
                if (tile_image->storage_class == PseudoClass)
                  {
                    index=ConstrainColormapIndex(tile_image,*p);
                    SetPixelIndex(indexes+x,index);
                    SetPixelRed(q,
                      tile_image->colormap[(ssize_t) index].red);
                    SetPixelGreen(q,
                      tile_image->colormap[(ssize_t) index].green);
                    SetPixelBlue(q,
                      tile_image->colormap[(ssize_t) index].blue);
                  }
                else
                  {
                    if (pixmap.bits_per_pixel == 16)
                      {
                        i=(*p++);
                        j=(*p);
                        SetPixelRed(q,ScaleCharToQuantum(
                          (unsigned char) ((i & 0x7c) << 1)));
                        SetPixelGreen(q,ScaleCharToQuantum(
                          (unsigned char) (((i & 0x03) << 6) |
                          ((j & 0xe0) >> 2))));
                        SetPixelBlue(q,ScaleCharToQuantum(
                          (unsigned char) ((j & 0x1f) << 3)));
                      }
                    else
                      if (tile_image->matte == MagickFalse)
                        {
                          if (p > (pixels+extent+2*image->columns))
                            ThrowReaderException(CorruptImageError,
                              "NotEnoughPixelData");
                          SetPixelRed(q,ScaleCharToQuantum(*p));
                          SetPixelGreen(q,ScaleCharToQuantum(
                            *(p+tile_image->columns)));
                          SetPixelBlue(q,ScaleCharToQuantum(
                            *(p+2*tile_image->columns)));
                        }
                      else
                        {
                          if (p > (pixels+extent+3*image->columns))
                            ThrowReaderException(CorruptImageError,
                              "NotEnoughPixelData");
                          SetPixelAlpha(q,ScaleCharToQuantum(*p));
                          SetPixelRed(q,ScaleCharToQuantum(
                            *(p+tile_image->columns)));
                          SetPixelGreen(q,ScaleCharToQuantum(
                            *(p+2*tile_image->columns)));
                          SetPixelBlue(q,ScaleCharToQuantum(
                            *(p+3*tile_image->columns)));
                        }
                  }
                p++;
                q++;
              }
              if (SyncAuthenticPixels(tile_image,exception) == MagickFalse)
                break;
              if ((tile_image->storage_class == DirectClass) &&
                  (pixmap.bits_per_pixel != 16))
                {
                  p+=(pixmap.component_count-1)*tile_image->columns;
                  if (p < pixels)
                    break;
                }
              status=SetImageProgress(image,LoadImageTag,y,tile_image->rows);
              if (status == MagickFalse)
                break;
            }
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            if (jpeg == MagickFalse)
              if ((code == 0x9a) || (code == 0x9b) ||
                  ((bytes_per_line & 0x8000) != 0))
                (void) CompositeImage(image,CopyCompositeOp,tile_image,
                  destination.left,destination.top);
            tile_image=DestroyImage(tile_image);
            break;
          }
          case 0xa1:
          {
            unsigned char
              *info;

            size_t
              type;

            /*
              Comment.
            */
            type=ReadBlobMSBShort(image);
            length=ReadBlobMSBShort(image);
            if (length == 0)
              break;
            (void) ReadBlobMSBLong(image);
            length-=4;
            if (length == 0)
              break;
            info=(unsigned char *) AcquireQuantumMemory(length,sizeof(*info));
            if (info == (unsigned char *) NULL)
              break;
            count=ReadBlob(image,length,info);
            (void) count;
            switch (type)
            {
              case 0xe0:
              {
                if (length == 0)
                  break;
                profile=BlobToStringInfo((const void *) NULL,length);
                SetStringInfoDatum(profile,info);
                status=SetImageProfile(image,"icc",profile);
                profile=DestroyStringInfo(profile);
                if (status == MagickFalse)
                  ThrowReaderException(ResourceLimitError,
                    "MemoryAllocationFailed");
                break;
              }
              case 0x1f2:
              {
                if (length == 0)
                  break;
                profile=BlobToStringInfo((const void *) NULL,length);
                SetStringInfoDatum(profile,info);
                status=SetImageProfile(image,"iptc",profile);
                if (status == MagickFalse)
                  ThrowReaderException(ResourceLimitError,
                    "MemoryAllocationFailed");
                profile=DestroyStringInfo(profile);
                break;
              }
              default:
                break;
            }
            info=(unsigned char *) RelinquishMagickMemory(info);
            break;
          }
          default:
          {
            /*
              Skip to next op code.
            */
            if (codes[code].length == -1)
              (void) ReadBlobMSBShort(image);
            else
              for (i=0; i < (ssize_t) codes[code].length; i++)
                (void) ReadBlobByte(image);
          }
        }
      }
    if (code == 0xc00)
      {
        /*
          Skip header.
        */
        for (i=0; i < 24; i++)
          (void) ReadBlobByte(image);
        continue;
      }
    if (((code >= 0xb0) && (code <= 0xcf)) ||
        ((code >= 0x8000) && (code <= 0x80ff)))
      continue;
    if (code == 0x8200)
      {
        FILE
          *file;

        Image
          *tile_image;

        ImageInfo
          *read_info;

        int
          unique_file;

        /*
          Embedded JPEG.
        */
        jpeg=MagickTrue;
        read_info=CloneImageInfo(image_info);
        SetImageInfoBlob(read_info,(void *) NULL,0);
        file=(FILE *) NULL;
        unique_file=AcquireUniqueFileResource(read_info->filename);
        if (unique_file != -1)
          file=fdopen(unique_file,"wb");
        if ((unique_file == -1) || (file == (FILE *) NULL))
          {
            (void) CopyMagickString(image->filename,read_info->filename,
              MaxTextExtent);
            ThrowFileException(exception,FileOpenError,
              "UnableToCreateTemporaryFile",image->filename);
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        length=ReadBlobMSBLong(image);
        for (i=0; i < 6; i++)
          (void) ReadBlobMSBLong(image);
        ReadRectangle(image,frame);
        for (i=0; i < 122; i++)
          (void) ReadBlobByte(image);
        for (i=0; i < (ssize_t) (length-154); i++)
        {
          c=ReadBlobByte(image);
          (void) fputc(c,file);
        }
        (void) fclose(file);
        tile_image=ReadImage(read_info,exception);
        (void) RelinquishUniqueFileResource(read_info->filename);
        read_info=DestroyImageInfo(read_info);
        if (tile_image == (Image *) NULL)
          continue;
        (void) FormatLocaleString(geometry,MaxTextExtent,"%.20gx%.20g",
          (double) MagickMax(image->columns,tile_image->columns),
          (double) MagickMax(image->rows,tile_image->rows));
        (void) SetImageExtent(image,
          MagickMax(image->columns,tile_image->columns),
          MagickMax(image->rows,tile_image->rows));
        if (IssRGBCompatibleColorspace(tile_image->colorspace) == MagickFalse)
          (void) TransformImageColorspace(image,tile_image->colorspace);
        (void) CompositeImage(image,CopyCompositeOp,tile_image,frame.left,
          frame.right);
        image->compression=tile_image->compression;
        tile_image=DestroyImage(tile_image);
        continue;
      }
    if ((code == 0xff) || (code == 0xffff))
      break;
    if (((code >= 0xd0) && (code <= 0xfe)) ||
        ((code >= 0x8100) && (code <= 0xffff)))
      {
        /*
          Skip reserved.
        */
        length=ReadBlobMSBShort(image);
        for (i=0; i < (ssize_t) length; i++)
          (void) ReadBlobByte(image);
        continue;
      }
    if ((code >= 0x100) && (code <= 0x7fff))
      {
        /*
          Skip reserved.
        */
        length=(size_t) ((code >> 7) & 0xff);
        for (i=0; i < (ssize_t) length; i++)
          (void) ReadBlobByte(image);
        continue;
      }
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P I C T I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPICTImage() adds attributes for the PICT image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPICTImage method is:
%
%      size_t RegisterPICTImage(void)
%
*/
ModuleExport size_t RegisterPICTImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("PCT");
  entry->decoder=(DecodeImageHandler *) ReadPICTImage;
  entry->encoder=(EncodeImageHandler *) WritePICTImage;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Apple Macintosh QuickDraw/PICT");
  entry->magick=(IsImageFormatHandler *) IsPICT;
  entry->module=ConstantString("PICT");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PICT");
  entry->decoder=(DecodeImageHandler *) ReadPICTImage;
  entry->encoder=(EncodeImageHandler *) WritePICTImage;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Apple Macintosh QuickDraw/PICT");
  entry->magick=(IsImageFormatHandler *) IsPICT;
  entry->module=ConstantString("PICT");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P I C T I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPICTImage() removes format registrations made by the
%  PICT module from the list of supported formats.
%
%  The format of the UnregisterPICTImage method is:
%
%      UnregisterPICTImage(void)
%
*/
ModuleExport void UnregisterPICTImage(void)
{
  (void) UnregisterMagickInfo("PCT");
  (void) UnregisterMagickInfo("PICT");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P I C T I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePICTImage() writes an image to a file in the Apple Macintosh
%  QuickDraw/PICT image format.
%
%  The format of the WritePICTImage method is:
%
%      MagickBooleanType WritePICTImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WritePICTImage(const ImageInfo *image_info,
  Image *image)
{
#define MaxCount  128
#define PictCropRegionOp  0x01
#define PictEndOfPictureOp  0xff
#define PictJPEGOp  0x8200
#define PictInfoOp  0x0C00
#define PictInfoSize  512
#define PictPixmapOp  0x9A
#define PictPICTOp  0x98
#define PictVersion  0x11

  const StringInfo
    *profile;

  double
    x_resolution,
    y_resolution;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  PICTPixmap
    pixmap;

  PICTRectangle
    bounds,
    crop_rectangle,
    destination_rectangle,
    frame_rectangle,
    size_rectangle,
    source_rectangle;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  size_t
    bytes_per_line,
    count,
    storage_class;

  ssize_t
    y;

  unsigned char
    *buffer,
    *packed_scanline,
    *scanline;


  unsigned short
    base_address,
    row_bytes,
    transfer_mode;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns > 65535L) || (image->rows > 65535L))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace);
  /*
    Initialize image info.
  */
  size_rectangle.top=0;
  size_rectangle.left=0;
  size_rectangle.bottom=(short) image->rows;
  size_rectangle.right=(short) image->columns;
  frame_rectangle=size_rectangle;
  crop_rectangle=size_rectangle;
  source_rectangle=size_rectangle;
  destination_rectangle=size_rectangle;
  base_address=0xff;
  row_bytes=(unsigned short) (image->columns | 0x8000);
  bounds.top=0;
  bounds.left=0;
  bounds.bottom=(short) image->rows;
  bounds.right=(short) image->columns;
  pixmap.version=0;
  pixmap.pack_type=0;
  pixmap.pack_size=0;
  pixmap.pixel_type=0;
  pixmap.bits_per_pixel=8;
  pixmap.component_count=1;
  pixmap.component_size=8;
  pixmap.plane_bytes=0;
  pixmap.table=0;
  pixmap.reserved=0;
  transfer_mode=0;
  x_resolution=image->x_resolution != 0.0 ? image->x_resolution :
    DefaultResolution;
  y_resolution=image->y_resolution != 0.0 ? image->y_resolution :
    DefaultResolution;
  storage_class=image->storage_class;
  if (image_info->compression == JPEGCompression)
    storage_class=DirectClass;
  if ((storage_class == DirectClass) || (image->matte != MagickFalse))
    {
      pixmap.component_count=image->matte ? 4 : 3;
      pixmap.pixel_type=16;
      pixmap.bits_per_pixel=32;
      pixmap.pack_type=0x04;
      transfer_mode=0x40;
      row_bytes=(unsigned short) ((4*image->columns) | 0x8000);
    }
  /*
    Allocate memory.
  */
  bytes_per_line=image->columns;
  if ((storage_class == DirectClass) || (image->matte != MagickFalse))
    bytes_per_line*=image->matte ? 4 : 3;
  buffer=(unsigned char *) AcquireQuantumMemory(PictInfoSize,sizeof(*buffer));
  packed_scanline=(unsigned char *) AcquireQuantumMemory((size_t)
   (row_bytes+MaxCount),sizeof(*packed_scanline));
  scanline=(unsigned char *) AcquireQuantumMemory(row_bytes,sizeof(*scanline));
  if ((buffer == (unsigned char *) NULL) ||
      (packed_scanline == (unsigned char *) NULL) ||
      (scanline == (unsigned char *) NULL))
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(scanline,0,row_bytes);
  (void) ResetMagickMemory(packed_scanline,0,(size_t) (row_bytes+MaxCount));
  /*
    Write header, header size, size bounding box, version, and reserved.
  */
  (void) ResetMagickMemory(buffer,0,PictInfoSize);
  (void) WriteBlob(image,PictInfoSize,buffer);
  (void) WriteBlobMSBShort(image,0);
  (void) WriteBlobMSBShort(image,(unsigned short) size_rectangle.top);
  (void) WriteBlobMSBShort(image,(unsigned short) size_rectangle.left);
  (void) WriteBlobMSBShort(image,(unsigned short) size_rectangle.bottom);
  (void) WriteBlobMSBShort(image,(unsigned short) size_rectangle.right);
  (void) WriteBlobMSBShort(image,PictVersion);
  (void) WriteBlobMSBShort(image,0x02ff);  /* version #2 */
  (void) WriteBlobMSBShort(image,PictInfoOp);
  (void) WriteBlobMSBLong(image,0xFFFE0000UL);
  /*
    Write full size of the file, resolution, frame bounding box, and reserved.
  */
  (void) WriteBlobMSBShort(image,(unsigned short) x_resolution);
  (void) WriteBlobMSBShort(image,0x0000);
  (void) WriteBlobMSBShort(image,(unsigned short) y_resolution);
  (void) WriteBlobMSBShort(image,0x0000);
  (void) WriteBlobMSBShort(image,(unsigned short) frame_rectangle.top);
  (void) WriteBlobMSBShort(image,(unsigned short) frame_rectangle.left);
  (void) WriteBlobMSBShort(image,(unsigned short) frame_rectangle.bottom);
  (void) WriteBlobMSBShort(image,(unsigned short) frame_rectangle.right);
  (void) WriteBlobMSBLong(image,0x00000000L);
  profile=GetImageProfile(image,"iptc");
  if (profile != (StringInfo *) NULL)
    {
      (void) WriteBlobMSBShort(image,0xa1);
      (void) WriteBlobMSBShort(image,0x1f2);
      (void) WriteBlobMSBShort(image,(unsigned short)
        (GetStringInfoLength(profile)+4));
      (void) WriteBlobString(image,"8BIM");
      (void) WriteBlob(image,GetStringInfoLength(profile),
        GetStringInfoDatum(profile));
    }
  profile=GetImageProfile(image,"icc");
  if (profile != (StringInfo *) NULL)
    {
      (void) WriteBlobMSBShort(image,0xa1);
      (void) WriteBlobMSBShort(image,0xe0);
      (void) WriteBlobMSBShort(image,(unsigned short)
        (GetStringInfoLength(profile)+4));
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlob(image,GetStringInfoLength(profile),
        GetStringInfoDatum(profile));
      (void) WriteBlobMSBShort(image,0xa1);
      (void) WriteBlobMSBShort(image,0xe0);
      (void) WriteBlobMSBShort(image,4);
      (void) WriteBlobMSBLong(image,0x00000002UL);
    }
  /*
    Write crop region opcode and crop bounding box.
  */
  (void) WriteBlobMSBShort(image,PictCropRegionOp);
  (void) WriteBlobMSBShort(image,0xa);
  (void) WriteBlobMSBShort(image,(unsigned short) crop_rectangle.top);
  (void) WriteBlobMSBShort(image,(unsigned short) crop_rectangle.left);
  (void) WriteBlobMSBShort(image,(unsigned short) crop_rectangle.bottom);
  (void) WriteBlobMSBShort(image,(unsigned short) crop_rectangle.right);
  if (image_info->compression == JPEGCompression)
    {
      Image
        *jpeg_image;

      ImageInfo
        *jpeg_info;

      size_t
        length;

      unsigned char
        *blob;

      jpeg_image=CloneImage(image,0,0,MagickTrue,&image->exception);
      if (jpeg_image == (Image *) NULL)
        {
          (void) CloseBlob(image);
          return(MagickFalse);
        }
      jpeg_info=CloneImageInfo(image_info);
      (void) CopyMagickString(jpeg_info->magick,"JPEG",MaxTextExtent);
      length=0;
      blob=(unsigned char *) ImageToBlob(jpeg_info,jpeg_image,&length,
        &image->exception);
      jpeg_info=DestroyImageInfo(jpeg_info);
      if (blob == (unsigned char *) NULL)
        return(MagickFalse);
      jpeg_image=DestroyImage(jpeg_image);
      (void) WriteBlobMSBShort(image,PictJPEGOp);
      (void) WriteBlobMSBLong(image,(unsigned int) length+154);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBLong(image,0x00010000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00010000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x40000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00400000UL);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBShort(image,(unsigned short) image->rows);
      (void) WriteBlobMSBShort(image,(unsigned short) image->columns);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBShort(image,768);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00566A70UL);
      (void) WriteBlobMSBLong(image,0x65670000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000001UL);
      (void) WriteBlobMSBLong(image,0x00016170UL);
      (void) WriteBlobMSBLong(image,0x706C0000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBShort(image,768);
      (void) WriteBlobMSBShort(image,(unsigned short) image->columns);
      (void) WriteBlobMSBShort(image,(unsigned short) image->rows);
      (void) WriteBlobMSBShort(image,(unsigned short) x_resolution);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBShort(image,(unsigned short) y_resolution);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x87AC0001UL);
      (void) WriteBlobMSBLong(image,0x0B466F74UL);
      (void) WriteBlobMSBLong(image,0x6F202D20UL);
      (void) WriteBlobMSBLong(image,0x4A504547UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x00000000UL);
      (void) WriteBlobMSBLong(image,0x0018FFFFUL);
      (void) WriteBlob(image,length,blob);
      if ((length & 0x01) != 0)
        (void) WriteBlobByte(image,'\0');
      blob=(unsigned char *) RelinquishMagickMemory(blob);
    }
  /*
    Write picture opcode, row bytes, and picture bounding box, and version.
  */
  if (storage_class == PseudoClass)
    (void) WriteBlobMSBShort(image,PictPICTOp);
  else
    {
      (void) WriteBlobMSBShort(image,PictPixmapOp);
      (void) WriteBlobMSBLong(image,(size_t) base_address);
    }
  (void) WriteBlobMSBShort(image,(unsigned short) (row_bytes | 0x8000));
  (void) WriteBlobMSBShort(image,(unsigned short) bounds.top);
  (void) WriteBlobMSBShort(image,(unsigned short) bounds.left);
  (void) WriteBlobMSBShort(image,(unsigned short) bounds.bottom);
  (void) WriteBlobMSBShort(image,(unsigned short) bounds.right);
  /*
    Write pack type, pack size, resolution, pixel type, and pixel size.
  */
  (void) WriteBlobMSBShort(image,(unsigned short) pixmap.version);
  (void) WriteBlobMSBShort(image,(unsigned short) pixmap.pack_type);
  (void) WriteBlobMSBLong(image,(unsigned int) pixmap.pack_size);
  (void) WriteBlobMSBShort(image,(unsigned short) (x_resolution+0.5));
  (void) WriteBlobMSBShort(image,0x0000);
  (void) WriteBlobMSBShort(image,(unsigned short) (y_resolution+0.5));
  (void) WriteBlobMSBShort(image,0x0000);
  (void) WriteBlobMSBShort(image,(unsigned short) pixmap.pixel_type);
  (void) WriteBlobMSBShort(image,(unsigned short) pixmap.bits_per_pixel);
  /*
    Write component count, size, plane bytes, table size, and reserved.
  */
  (void) WriteBlobMSBShort(image,(unsigned short) pixmap.component_count);
  (void) WriteBlobMSBShort(image,(unsigned short) pixmap.component_size);
  (void) WriteBlobMSBLong(image,(unsigned int) pixmap.plane_bytes);
  (void) WriteBlobMSBLong(image,(unsigned int) pixmap.table);
  (void) WriteBlobMSBLong(image,(unsigned int) pixmap.reserved);
  if (storage_class == PseudoClass)
    {
      /*
        Write image colormap.
      */
      (void) WriteBlobMSBLong(image,0x00000000L);  /* color seed */
      (void) WriteBlobMSBShort(image,0L);  /* color flags */
      (void) WriteBlobMSBShort(image,(unsigned short) (image->colors-1));
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        (void) WriteBlobMSBShort(image,(unsigned short) i);
        (void) WriteBlobMSBShort(image,ScaleQuantumToShort(
          image->colormap[i].red));
        (void) WriteBlobMSBShort(image,ScaleQuantumToShort(
          image->colormap[i].green));
        (void) WriteBlobMSBShort(image,ScaleQuantumToShort(
          image->colormap[i].blue));
      }
    }
  /*
    Write source and destination rectangle.
  */
  (void) WriteBlobMSBShort(image,(unsigned short) source_rectangle.top);
  (void) WriteBlobMSBShort(image,(unsigned short) source_rectangle.left);
  (void) WriteBlobMSBShort(image,(unsigned short) source_rectangle.bottom);
  (void) WriteBlobMSBShort(image,(unsigned short) source_rectangle.right);
  (void) WriteBlobMSBShort(image,(unsigned short) destination_rectangle.top);
  (void) WriteBlobMSBShort(image,(unsigned short) destination_rectangle.left);
  (void) WriteBlobMSBShort(image,(unsigned short) destination_rectangle.bottom);
  (void) WriteBlobMSBShort(image,(unsigned short) destination_rectangle.right);
  (void) WriteBlobMSBShort(image,(unsigned short) transfer_mode);
  /*
    Write picture data.
  */
  count=0;
  if ((storage_class == PseudoClass) && (image->matte == MagickFalse))
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetVirtualIndexQueue(image);
      for (x=0; x < (ssize_t) image->columns; x++)
        scanline[x]=(unsigned char) GetPixelIndex(indexes+x);
      count+=EncodeImage(image,scanline,(size_t) (row_bytes & 0x7FFF),
        packed_scanline);
      if (image->previous == (Image *) NULL)
        {
          status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
    }
  else
    if (image_info->compression == JPEGCompression)
      {
        (void) ResetMagickMemory(scanline,0,row_bytes);
        for (y=0; y < (ssize_t) image->rows; y++)
          count+=EncodeImage(image,scanline,(size_t) (row_bytes & 0x7FFF),
            packed_scanline);
      }
    else
      {
        register unsigned char
          *blue,
          *green,
          *opacity,
          *red;

        red=scanline;
        green=scanline+image->columns;
        blue=scanline+2*image->columns;
        opacity=scanline+3*image->columns;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          red=scanline;
          green=scanline+image->columns;
          blue=scanline+2*image->columns;
          if (image->matte != MagickFalse)
            {
              opacity=scanline;
              red=scanline+image->columns;
              green=scanline+2*image->columns;
              blue=scanline+3*image->columns;
            }
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *red++=ScaleQuantumToChar(GetPixelRed(p));
            *green++=ScaleQuantumToChar(GetPixelGreen(p));
            *blue++=ScaleQuantumToChar(GetPixelBlue(p));
            if (image->matte != MagickFalse)
              *opacity++=ScaleQuantumToChar((Quantum)
                (GetPixelAlpha(p)));
            p++;
          }
          count+=EncodeImage(image,scanline,bytes_per_line & 0x7FFF,
            packed_scanline);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
      }
  if ((count & 0x01) != 0)
    (void) WriteBlobByte(image,'\0');
  (void) WriteBlobMSBShort(image,PictEndOfPictureOp);
  offset=TellBlob(image);
  offset=SeekBlob(image,512,SEEK_SET);
  (void) WriteBlobMSBShort(image,(unsigned short) offset);
  scanline=(unsigned char *) RelinquishMagickMemory(scanline);
  packed_scanline=(unsigned char *) RelinquishMagickMemory(packed_scanline);
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  (void) CloseBlob(image);
  return(MagickTrue);
}

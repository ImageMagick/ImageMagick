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
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/constitute.h"
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
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"

/*
  ImageMagick Macintosh PICT Methods.
*/
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
  WritePICTImage(const ImageInfo *,Image *,ExceptionInfo *);

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

static const unsigned char *UnpackScanline(
  const unsigned char *magick_restrict pixels,const unsigned int bits_per_pixel,
  unsigned char *scanline,MagickSizeType *bytes_per_line)
{
  register const unsigned char
    *p;

  register ssize_t
    i;

  register unsigned char
    *q;

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
  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  register const unsigned char
    *p;

  register ssize_t
    i;

  register unsigned char
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
    *scanline,
    unpack_buffer[8*256];

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
      width*=image->alpha_trait ? 4 : 3;
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
  (void) memset(pixels,0,*extent);
  scanline=(unsigned char *) AcquireQuantumMemory(row_bytes,2*
    sizeof(*scanline));
  if (scanline == (unsigned char *) NULL)
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      return((unsigned char *) NULL);
    }
  (void) memset(scanline,0,2*row_bytes*sizeof(*scanline));
  (void) memset(unpack_buffer,0,sizeof(unpack_buffer));
  status=MagickTrue;
  if (bytes_per_line < 8)
    {
      /*
        Pixels are already uncompressed.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        q=pixels+y*width*GetPixelChannels(image);
        number_pixels=bytes_per_line;
        count=ReadBlob(blob,(size_t) number_pixels,scanline);
        if (count != (ssize_t) number_pixels)
          {
            status=MagickFalse;
            break;
          }
        p=UnpackScanline(scanline,bits_per_pixel,unpack_buffer,&number_pixels);
        if ((q+number_pixels) > (pixels+(*extent)))
          {
            status=MagickFalse;
            break;
          }
        (void) memcpy(q,p,(size_t) number_pixels);
      }
      scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      if (status == MagickFalse)
        pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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
      scanline_length=(size_t) ReadBlobByte(blob);
    if ((scanline_length >= row_bytes) || (scanline_length == 0))
      {
        status=MagickFalse;
        break;
      }
    count=ReadBlob(blob,scanline_length,scanline);
    if (count != (ssize_t) scanline_length)
      {
        status=MagickFalse;
        break;
      }
    for (j=0; j < (ssize_t) scanline_length; )
      if ((scanline[j] & 0x80) == 0)
        {
          length=(size_t) ((scanline[j] & 0xff)+1);
          number_pixels=length*bytes_per_pixel;
          p=UnpackScanline(scanline+j+1,bits_per_pixel,unpack_buffer,
            &number_pixels);
          if ((q-pixels+number_pixels) <= *extent)
            (void) memcpy(q,p,(size_t) number_pixels);
          q+=number_pixels;
          j+=(ssize_t) (length*bytes_per_pixel+1);
        }
      else
        {
          length=(size_t) (((scanline[j] ^ 0xff) & 0xff)+2);
          number_pixels=bytes_per_pixel;
          p=UnpackScanline(scanline+j+1,bits_per_pixel,unpack_buffer,
            &number_pixels);
          for (i=0; i < (ssize_t) length; i++)
          {
            if ((q-pixels+number_pixels) <= *extent)
              (void) memcpy(q,p,(size_t) number_pixels);
            q+=number_pixels;
          }
          j+=(ssize_t) bytes_per_pixel+1;
        }
  }
  scanline=(unsigned char *) RelinquishMagickMemory(scanline);
  if (status == MagickFalse)
    pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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
  assert(image->signature == MagickCoreSignature);
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
  if (length < 12)
    return(MagickFalse);
  /*
    Embedded OLE2 macintosh have "PICT" instead of 512 platform header.
  */
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

static MagickBooleanType ReadPixmap(Image *image,PICTPixmap *pixmap)
{
  pixmap->version=(short) ReadBlobMSBShort(image);
  pixmap->pack_type=(short) ReadBlobMSBShort(image);
  pixmap->pack_size=ReadBlobMSBLong(image);
  pixmap->horizontal_resolution=1UL*ReadBlobMSBShort(image);
  (void) ReadBlobMSBShort(image);
  pixmap->vertical_resolution=1UL*ReadBlobMSBShort(image);
  (void) ReadBlobMSBShort(image);
  pixmap->pixel_type=(short) ReadBlobMSBShort(image);
  pixmap->bits_per_pixel=(short) ReadBlobMSBShort(image);
  pixmap->component_count=(short) ReadBlobMSBShort(image);
  pixmap->component_size=(short) ReadBlobMSBShort(image);
  pixmap->plane_bytes=ReadBlobMSBLong(image);
  pixmap->table=ReadBlobMSBLong(image);
  pixmap->reserved=ReadBlobMSBLong(image);
  if ((EOFBlob(image) != MagickFalse) || (pixmap->bits_per_pixel <= 0) ||
      (pixmap->bits_per_pixel > 32) || (pixmap->component_count <= 0) ||
      (pixmap->component_count > 4) || (pixmap->component_size <= 0))
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ReadRectangle(Image *image,PICTRectangle *rectangle)
{
  rectangle->top=(short) ReadBlobMSBShort(image);
  rectangle->left=(short) ReadBlobMSBShort(image);
  rectangle->bottom=(short) ReadBlobMSBShort(image);
  rectangle->right=(short) ReadBlobMSBShort(image);
  if (((EOFBlob(image) != MagickFalse) ||
      (((rectangle->bottom | rectangle->top |
         rectangle->right | rectangle->left ) & 0x8000) != 0) ||
      (rectangle->bottom < rectangle->top) ||
      (rectangle->right < rectangle->left)))
    return(MagickFalse);
  return(MagickTrue);
}

static Image *ReadPICTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define ThrowPICTException(exception,message) \
{ \
  if (tile_image != (Image *) NULL) \
    tile_image=DestroyImage(tile_image); \
  if (read_info != (ImageInfo *) NULL) \
    read_info=DestroyImageInfo(read_info); \
  ThrowReaderException((exception),(message)); \
}

  char
    geometry[MagickPathExtent],
    header_ole[4];

  Image
    *image,
    *tile_image;

  ImageInfo
    *read_info;

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

  Quantum
    index;

  register Quantum
    *q;

  register ssize_t
    i,
    x;

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
    Read PICT header.
  */
  read_info=(ImageInfo *) NULL;
  tile_image=(Image *) NULL;
  pixmap.bits_per_pixel=0;
  pixmap.component_count=0;
  /*
    Skip header : 512 for standard PICT and 4, ie "PICT" for OLE2.
  */
  header_ole[0]=ReadBlobByte(image);
  header_ole[1]=ReadBlobByte(image);
  header_ole[2]=ReadBlobByte(image);
  header_ole[3]=ReadBlobByte(image);
  if (!((header_ole[0] == 0x50) && (header_ole[1] == 0x49) &&
      (header_ole[2] == 0x43) && (header_ole[3] == 0x54 )))
    for (i=0; i < 508; i++)
      if (ReadBlobByte(image) == EOF)
        break;
  (void) ReadBlobMSBShort(image);  /* skip picture size */
  if (ReadRectangle(image,&frame) == MagickFalse)
    ThrowPICTException(CorruptImageError,"ImproperImageHeader");
  while ((c=ReadBlobByte(image)) == 0) ;
  if (c != 0x11)
    ThrowPICTException(CorruptImageError,"ImproperImageHeader");
  version=(ssize_t) ReadBlobByte(image);
  if (version == 2)
    {
      c=ReadBlobByte(image);
      if (c != 0xff)
        ThrowPICTException(CorruptImageError,"ImproperImageHeader");
    }
  else
    if (version != 1)
      ThrowPICTException(CorruptImageError,"ImproperImageHeader");
  if ((frame.left < 0) || (frame.right < 0) || (frame.top < 0) ||
      (frame.bottom < 0) || (frame.left >= frame.right) ||
      (frame.top >= frame.bottom))
    ThrowPICTException(CorruptImageError,"ImproperImageHeader");
  /*
    Create black canvas.
  */
  flags=0;
  image->depth=8;
  image->columns=(size_t) (frame.right-frame.left);
  image->rows=(size_t) (frame.bottom-frame.top);
  image->resolution.x=DefaultResolution;
  image->resolution.y=DefaultResolution;
  image->units=UndefinedResolution;
  if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
    if (image->scene >= (image_info->scene+image_info->number_scenes-1))
      {
        (void) CloseBlob(image);
        return(GetFirstImageInList(image));
      }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status != MagickFalse)
    status=ResetImagePixels(image,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
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
      code=ReadBlobMSBSignedShort(image);
    code&=0xffff;
    if (code < 0)
      break;
    if (code == 0)
      continue;
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
            if ((MagickSizeType) length > GetBlobSize(image))
              ThrowPICTException(CorruptImageError,
                "InsufficientImageDataInFile");
            if (length != 0x000a)
              {
                for (i=0; i < (ssize_t) (length-2); i++)
                  if (ReadBlobByte(image) == EOF)
                    break;
                break;
              }
            if (ReadRectangle(image,&frame) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            if (((frame.left & 0x8000) != 0) || ((frame.top & 0x8000) != 0))
              break;
            image->columns=(size_t) (frame.right-frame.left);
            image->rows=(size_t) (frame.bottom-frame.top);
            status=SetImageExtent(image,image->columns,image->rows,exception);
            if (status != MagickFalse)
              status=ResetImagePixels(image,exception);
            if (status == MagickFalse)
              return(DestroyImageList(image));
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
            pattern=(ssize_t) ReadBlobMSBShort(image);
            for (i=0; i < 8; i++)
              if (ReadBlobByte(image) == EOF)
                break;
            if (pattern == 2)
              {
                for (i=0; i < 5; i++)
                  if (ReadBlobByte(image) == EOF)
                    break;
                break;
              }
            if (pattern != 1)
              ThrowPICTException(CorruptImageError,"UnknownPatternType");
            length=ReadBlobMSBShort(image);
            if ((MagickSizeType) length > GetBlobSize(image))
              ThrowPICTException(CorruptImageError,
                "InsufficientImageDataInFile");
            if (ReadRectangle(image,&frame) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            if (ReadPixmap(image,&pixmap) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            image->depth=(size_t) pixmap.component_size;
            image->resolution.x=1.0*pixmap.horizontal_resolution;
            image->resolution.y=1.0*pixmap.vertical_resolution;
            image->units=PixelsPerInchResolution;
            (void) ReadBlobMSBLong(image);
            flags=(ssize_t) ReadBlobMSBShort(image);
            length=ReadBlobMSBShort(image);
            if ((MagickSizeType) length > GetBlobSize(image))
              ThrowPICTException(CorruptImageError,
                "InsufficientImageDataInFile");
            for (i=0; i <= (ssize_t) length; i++)
              (void) ReadBlobMSBLong(image);
            width=(size_t) (frame.bottom-frame.top);
            height=(size_t) (frame.right-frame.left);
            if (pixmap.bits_per_pixel <= 8)
              length&=0x7fff;
            if (pixmap.bits_per_pixel == 16)
              width<<=1;
            if (length == 0)
              length=width;
            if (length < 8)
              {
                for (i=0; i < (ssize_t) (length*height); i++)
                  if (ReadBlobByte(image) == EOF)
                    break;
              }
            else
              for (i=0; i < (ssize_t) height; i++)
              {
                if (EOFBlob(image) != MagickFalse)
                  break;
                if (length > 200)
                  {
                    for (j=0; j < (ssize_t) ReadBlobMSBShort(image); j++)
                      if (ReadBlobByte(image) == EOF)
                        break;
                  }
                else
                  for (j=0; j < (ssize_t) ReadBlobByte(image); j++)
                    if (ReadBlobByte(image) == EOF)
                      break;
              }
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
            if ((MagickSizeType) length > GetBlobSize(image))
              ThrowPICTException(CorruptImageError,
                "InsufficientImageDataInFile");
            for (i=0; i < (ssize_t) (length-2); i++)
              if (ReadBlobByte(image) == EOF)
                break;
            break;
          }
          case 0x90:
          case 0x91:
          case 0x98:
          case 0x99:
          case 0x9a:
          case 0x9b:
          {
            PICTRectangle
              source,
              destination;

            register unsigned char
              *p;

            size_t
              j;

            ssize_t
              bytes_per_line;

            unsigned char
              *pixels;

            /*
              Pixmap clipped by a rectangle.
            */
            bytes_per_line=0;
            if ((code != 0x9a) && (code != 0x9b))
              bytes_per_line=(ssize_t) ReadBlobMSBShort(image);
            else
              {
                (void) ReadBlobMSBShort(image);
                (void) ReadBlobMSBShort(image);
                (void) ReadBlobMSBShort(image);
              }
            if (ReadRectangle(image,&frame) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            /*
              Initialize tile image.
            */
            tile_image=CloneImage(image,(size_t) (frame.right-frame.left),
              (size_t) (frame.bottom-frame.top),MagickTrue,exception);
            if (tile_image == (Image *) NULL)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            status=ResetImagePixels(tile_image,exception);
            if ((code == 0x9a) || (code == 0x9b) ||
                ((bytes_per_line & 0x8000) != 0))
              {
                if (ReadPixmap(image,&pixmap) == MagickFalse)
                  ThrowPICTException(CorruptImageError,"ImproperImageHeader");
                tile_image->depth=(size_t) pixmap.component_size;
                tile_image->alpha_trait=pixmap.component_count == 4 ?
                  BlendPixelTrait : UndefinedPixelTrait;
                tile_image->resolution.x=(double) pixmap.horizontal_resolution;
                tile_image->resolution.y=(double) pixmap.vertical_resolution;
                tile_image->units=PixelsPerInchResolution;
                if (tile_image->alpha_trait != UndefinedPixelTrait)
                  (void) SetImageAlpha(tile_image,OpaqueAlpha,exception);
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
                    flags=(ssize_t) ReadBlobMSBShort(image);
                    tile_image->colors=1UL*ReadBlobMSBShort(image)+1;
                  }
                status=AcquireImageColormap(tile_image,tile_image->colors,
                  exception);
                if (status == MagickFalse)
                  ThrowPICTException(ResourceLimitError,
                    "MemoryAllocationFailed");
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
            if (EOFBlob(image) != MagickFalse)
              ThrowPICTException(CorruptImageError,
                "InsufficientImageDataInFile");
            if (ReadRectangle(image,&source) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            if (ReadRectangle(image,&destination) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            (void) ReadBlobMSBShort(image);
            if ((code == 0x91) || (code == 0x99) || (code == 0x9b))
              {
                /*
                  Skip region.
                */
                length=ReadBlobMSBShort(image);
                if ((MagickSizeType) length > GetBlobSize(image))
                  ThrowPICTException(CorruptImageError,
                    "InsufficientImageDataInFile");
                for (i=0; i < (ssize_t) (length-2); i++)
                  if (ReadBlobByte(image) == EOF)
                    break;
              }
            if ((code != 0x9a) && (code != 0x9b) &&
                (bytes_per_line & 0x8000) == 0)
              pixels=DecodeImage(image,tile_image,(size_t) bytes_per_line,1,
                &extent);
            else
              pixels=DecodeImage(image,tile_image,(size_t) bytes_per_line,
                (unsigned int) pixmap.bits_per_pixel,&extent);
            if (pixels == (unsigned char *) NULL)
              ThrowPICTException(CorruptImageError,"UnableToUncompressImage");
            /*
              Convert PICT tile image to pixel packets.
            */
            p=pixels;
            for (y=0; y < (ssize_t) tile_image->rows; y++)
            {
              if (p > (pixels+extent+image->columns))
                {
                  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
                  ThrowPICTException(CorruptImageError,"NotEnoughPixelData");
                }
              q=QueueAuthenticPixels(tile_image,0,y,tile_image->columns,1,
                exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) tile_image->columns; x++)
              {
                if (tile_image->storage_class == PseudoClass)
                  {
                    index=(Quantum) ConstrainColormapIndex(tile_image,(ssize_t)
                      *p,exception);
                    SetPixelIndex(tile_image,index,q);
                    SetPixelRed(tile_image,
                      tile_image->colormap[(ssize_t) index].red,q);
                    SetPixelGreen(tile_image,
                      tile_image->colormap[(ssize_t) index].green,q);
                    SetPixelBlue(tile_image,
                      tile_image->colormap[(ssize_t) index].blue,q);
                  }
                else
                  {
                    if (pixmap.bits_per_pixel == 16)
                      {
                        i=(ssize_t) (*p++);
                        j=(size_t) (*p);
                        SetPixelRed(tile_image,ScaleCharToQuantum(
                          (unsigned char) ((i & 0x7c) << 1)),q);
                        SetPixelGreen(tile_image,ScaleCharToQuantum(
                          (unsigned char) (((i & 0x03) << 6) |
                          ((j & 0xe0) >> 2))),q);
                        SetPixelBlue(tile_image,ScaleCharToQuantum(
                          (unsigned char) ((j & 0x1f) << 3)),q);
                      }
                    else
                      if (tile_image->alpha_trait == UndefinedPixelTrait)
                        {
                          if (p > (pixels+extent+2*image->columns))
                            ThrowPICTException(CorruptImageError,
                              "NotEnoughPixelData");
                          SetPixelRed(tile_image,ScaleCharToQuantum(*p),q);
                          SetPixelGreen(tile_image,ScaleCharToQuantum(
                            *(p+tile_image->columns)),q);
                          SetPixelBlue(tile_image,ScaleCharToQuantum(
                            *(p+2*tile_image->columns)),q);
                        }
                      else
                        {
                          if (p > (pixels+extent+3*image->columns))
                            ThrowPICTException(CorruptImageError,
                              "NotEnoughPixelData");
                          SetPixelAlpha(tile_image,ScaleCharToQuantum(*p),q);
                          SetPixelRed(tile_image,ScaleCharToQuantum(
                            *(p+tile_image->columns)),q);
                          SetPixelGreen(tile_image,ScaleCharToQuantum(
                            *(p+2*tile_image->columns)),q);
                          SetPixelBlue(tile_image,ScaleCharToQuantum(
                            *(p+3*tile_image->columns)),q);
                        }
                  }
                p++;
                q+=GetPixelChannels(tile_image);
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
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                tile_image->rows);
              if (status == MagickFalse)
                break;
            }
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            if ((jpeg == MagickFalse) && (EOFBlob(image) == MagickFalse))
              if ((code == 0x9a) || (code == 0x9b) ||
                  ((bytes_per_line & 0x8000) != 0))
                (void) CompositeImage(image,tile_image,CopyCompositeOp,
                  MagickTrue,(ssize_t) destination.left,(ssize_t)
                  destination.top,exception);
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
            if ((MagickSizeType) length > GetBlobSize(image))
              ThrowPICTException(CorruptImageError,
                "InsufficientImageDataInFile");
            if (length == 0)
              break;
            (void) ReadBlobMSBLong(image);
            length-=MagickMin(length,4);
            if (length == 0)
              break;
            info=(unsigned char *) AcquireQuantumMemory(length,sizeof(*info));
            if (info == (unsigned char *) NULL)
              break;
            count=ReadBlob(image,length,info);
            if (count != (ssize_t) length)
              {
                info=(unsigned char *) RelinquishMagickMemory(info);
                ThrowPICTException(ResourceLimitError,"UnableToReadImageData");
              }
            switch (type)
            {
              case 0xe0:
              {
                profile=BlobToStringInfo((const void *) NULL,length);
                SetStringInfoDatum(profile,info);
                status=SetImageProfile(image,"icc",profile,exception);
                profile=DestroyStringInfo(profile);
                if (status == MagickFalse)
                  {
                    info=(unsigned char *) RelinquishMagickMemory(info);
                    ThrowPICTException(ResourceLimitError,
                      "MemoryAllocationFailed");
                  }
                break;
              }
              case 0x1f2:
              {
                profile=BlobToStringInfo((const void *) NULL,length);
                SetStringInfoDatum(profile,info);
                status=SetImageProfile(image,"iptc",profile,exception);
                if (status == MagickFalse)
                  {
                    info=(unsigned char *) RelinquishMagickMemory(info);
                    ThrowPICTException(ResourceLimitError,
                      "MemoryAllocationFailed");
                  }
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
                if (ReadBlobByte(image) == EOF)
                  break;
          }
        }
      }
    if (code == 0xc00)
      {
        /*
          Skip header.
        */
        for (i=0; i < 24; i++)
          if (ReadBlobByte(image) == EOF)
            break;
        continue;
      }
    if (((code >= 0xb0) && (code <= 0xcf)) ||
        ((code >= 0x8000) && (code <= 0x80ff)))
      continue;
    if (code == 0x8200)
      {
        /*
          Embedded JPEG.
        */
        jpeg=MagickTrue;
        length=ReadBlobMSBLong(image);
        if ((MagickSizeType) length > GetBlobSize(image))
          ThrowPICTException(CorruptImageError,"InsufficientImageDataInFile");
        if (length > 154)
          {
            const void
              *stream;

            ssize_t
              count;

            unsigned char
              *pixels;

            for (i=0; i < 6; i++)
              (void) ReadBlobMSBLong(image);
            if (ReadRectangle(image,&frame) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            for (i=0; i < 122; i++)
              if (ReadBlobByte(image) == EOF)
                break;
            length-=154;
            pixels=(unsigned char *) AcquireQuantumMemory(length,
              sizeof(*pixels));
            if (pixels == (unsigned char *) NULL)
              ThrowPICTException(ResourceLimitError,"MemoryAllocationFailed");
            stream=ReadBlobStream(image,length,pixels,&count);
            if (count != (ssize_t) length)
              {
                pixels=(unsigned char *) RelinquishMagickMemory(pixels);
                ThrowPICTException(CorruptImageError,"ImproperImageHeader");
              }
            read_info=AcquireImageInfo();
            (void) FormatLocaleString(read_info->filename,MaxTextExtent,
              "jpeg:%s",image_info->filename);
            tile_image=BlobToImage(read_info,stream,count,exception);
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            read_info=DestroyImageInfo(read_info);
          }
        if (tile_image == (Image *) NULL)
          continue;
        (void) FormatLocaleString(geometry,MagickPathExtent,"%.20gx%.20g",
          (double) MagickMax(image->columns,tile_image->columns),
          (double) MagickMax(image->rows,tile_image->rows));
        (void) SetImageExtent(image,
          MagickMax(image->columns,tile_image->columns),
          MagickMax(image->rows,tile_image->rows),exception);
        (void) TransformImageColorspace(image,tile_image->colorspace,exception);
        (void) CompositeImage(image,tile_image,CopyCompositeOp,MagickTrue,
          (ssize_t) frame.left,(ssize_t) frame.right,exception);
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
        if ((MagickSizeType) length > GetBlobSize(image))
          ThrowPICTException(CorruptImageError,
            "InsufficientImageDataInFile");
        for (i=0; i < (ssize_t) length; i++)
          if (ReadBlobByte(image) == EOF)
            break;
        continue;
      }
    if ((code >= 0x100) && (code <= 0x7fff))
      {
        /*
          Skip reserved.
        */
        length=(size_t) ((code >> 7) & 0xff);
        if ((MagickSizeType) length > GetBlobSize(image))
          ThrowPICTException(CorruptImageError,
            "InsufficientImageDataInFile");
        for (i=0; i < (ssize_t) length; i++)
          if (ReadBlobByte(image) == EOF)
            break;
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

  entry=AcquireMagickInfo("PICT","PCT","Apple Macintosh QuickDraw/PICT");
  entry->decoder=(DecodeImageHandler *) ReadPICTImage;
  entry->encoder=(EncodeImageHandler *) WritePICTImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->magick=(IsImageFormatHandler *) IsPICT;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PICT","PICT","Apple Macintosh QuickDraw/PICT");
  entry->decoder=(DecodeImageHandler *) ReadPICTImage;
  entry->encoder=(EncodeImageHandler *) WritePICTImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->magick=(IsImageFormatHandler *) IsPICT;
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
static MagickBooleanType WritePICTImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
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

  register const Quantum
    *p;

  register ssize_t
    i,
    x;

  size_t
    bytes_per_line,
    count,
    row_bytes,
    storage_class;

  ssize_t
    y;

  unsigned char
    *buffer,
    *packed_scanline,
    *scanline;

  unsigned short
    base_address,
    transfer_mode;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns > 65535L) || (image->rows > 65535L))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  (void) TransformImageColorspace(image,sRGBColorspace,exception);
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
  row_bytes=image->columns;
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
  x_resolution=0.0;
  y_resolution=0.0;
  if ((image->resolution.x > MagickEpsilon) &&
      (image->resolution.y > MagickEpsilon))
    {
      x_resolution=image->resolution.x;
      y_resolution=image->resolution.y;
      if (image->units == PixelsPerCentimeterResolution)
        {
          x_resolution*=2.54;
          y_resolution*=2.54;
        }
    }
  storage_class=image->storage_class;
  if (image_info->compression == JPEGCompression)
    storage_class=DirectClass;
  if (storage_class == DirectClass)
    {
      pixmap.component_count=image->alpha_trait != UndefinedPixelTrait ? 4 : 3;
      pixmap.pixel_type=16;
      pixmap.bits_per_pixel=32;
      pixmap.pack_type=0x04;
      transfer_mode=0x40;
      row_bytes=4*image->columns;
    }
  /*
    Allocate memory.
  */
  bytes_per_line=image->columns;
  if (storage_class == DirectClass)
    bytes_per_line*=image->alpha_trait != UndefinedPixelTrait ? 4 : 3;
  if ((bytes_per_line == 0) || (bytes_per_line > 0x7FFFU) ||
      ((row_bytes+MaxCount*2U) >= 0x7FFFU))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  buffer=(unsigned char *) AcquireQuantumMemory(PictInfoSize,sizeof(*buffer));
  packed_scanline=(unsigned char *) AcquireQuantumMemory((size_t)
   (row_bytes+2*MaxCount),sizeof(*packed_scanline));
  scanline=(unsigned char *) AcquireQuantumMemory(row_bytes,sizeof(*scanline));
  if ((buffer == (unsigned char *) NULL) ||
      (packed_scanline == (unsigned char *) NULL) ||
      (scanline == (unsigned char *) NULL))
    {
      if (scanline != (unsigned char *) NULL)
        scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      if (packed_scanline != (unsigned char *) NULL)
        packed_scanline=(unsigned char *) RelinquishMagickMemory(
          packed_scanline);
      if (buffer != (unsigned char *) NULL)
        buffer=(unsigned char *) RelinquishMagickMemory(buffer);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) memset(scanline,0,row_bytes);
  (void) memset(packed_scanline,0,(size_t) (row_bytes+2*MaxCount)*
    sizeof(*packed_scanline));
  /*
    Write header, header size, size bounding box, version, and reserved.
  */
  (void) memset(buffer,0,PictInfoSize);
  (void) WriteBlob(image,PictInfoSize,buffer);
  (void) WriteBlobMSBShort(image,0);
  (void) WriteBlobMSBShort(image,(unsigned short) size_rectangle.top);
  (void) WriteBlobMSBShort(image,(unsigned short) size_rectangle.left);
  (void) WriteBlobMSBShort(image,(unsigned short) size_rectangle.bottom);
  (void) WriteBlobMSBShort(image,(unsigned short) size_rectangle.right);
  (void) WriteBlobMSBShort(image,PictVersion);
  (void) WriteBlobMSBShort(image,0x02ff);  /* version #2 */
  (void) WriteBlobMSBShort(image,PictInfoOp);
  (void) WriteBlobMSBLong(image,0xFFFE0000U);
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
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlob(image,GetStringInfoLength(profile),
        GetStringInfoDatum(profile));
      (void) WriteBlobMSBShort(image,0xa1);
      (void) WriteBlobMSBShort(image,0xe0);
      (void) WriteBlobMSBShort(image,4);
      (void) WriteBlobMSBLong(image,0x00000002U);
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

      jpeg_image=CloneImage(image,0,0,MagickTrue,exception);
      if (jpeg_image == (Image *) NULL)
        {
          (void) CloseBlob(image);
          return(MagickFalse);
        }
      jpeg_info=CloneImageInfo(image_info);
      (void) CopyMagickString(jpeg_info->magick,"JPEG",MagickPathExtent);
      length=0;
      blob=(unsigned char *) ImageToBlob(jpeg_info,jpeg_image,&length,
        exception);
      jpeg_info=DestroyImageInfo(jpeg_info);
      if (blob == (unsigned char *) NULL)
        return(MagickFalse);
      jpeg_image=DestroyImage(jpeg_image);
      (void) WriteBlobMSBShort(image,PictJPEGOp);
      (void) WriteBlobMSBLong(image,(unsigned int) length+154);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBLong(image,0x00010000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00010000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x40000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00400000U);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBShort(image,(unsigned short) image->rows);
      (void) WriteBlobMSBShort(image,(unsigned short) image->columns);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBShort(image,768);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00566A70U);
      (void) WriteBlobMSBLong(image,0x65670000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000001U);
      (void) WriteBlobMSBLong(image,0x00016170U);
      (void) WriteBlobMSBLong(image,0x706C0000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBShort(image,768);
      (void) WriteBlobMSBShort(image,(unsigned short) image->columns);
      (void) WriteBlobMSBShort(image,(unsigned short) image->rows);
      (void) WriteBlobMSBShort(image,(unsigned short) x_resolution);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBShort(image,(unsigned short) y_resolution);
      (void) WriteBlobMSBShort(image,0x0000);
      (void) WriteBlobMSBLong(image,(unsigned int) length);
      (void) WriteBlobMSBShort(image,0x0001);
      (void) WriteBlobMSBLong(image,0x0B466F74U);
      (void) WriteBlobMSBLong(image,0x6F202D20U);
      (void) WriteBlobMSBLong(image,0x4A504547U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x00000000U);
      (void) WriteBlobMSBLong(image,0x0018FFFFU);
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
      (void) WriteBlobMSBLong(image,(unsigned int) base_address);
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
  if (storage_class == PseudoClass)
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        scanline[x]=(unsigned char) GetPixelIndex(image,p);
        p+=GetPixelChannels(image);
      }
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
        (void) memset(scanline,0,row_bytes);
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
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          red=scanline;
          green=scanline+image->columns;
          blue=scanline+2*image->columns;
          if (image->alpha_trait != UndefinedPixelTrait)
            {
              opacity=scanline;
              red=scanline+image->columns;
              green=scanline+2*image->columns;
              blue=scanline+3*image->columns;
            }
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *red++=ScaleQuantumToChar(GetPixelRed(image,p));
            *green++=ScaleQuantumToChar(GetPixelGreen(image,p));
            *blue++=ScaleQuantumToChar(GetPixelBlue(image,p));
            if (image->alpha_trait != UndefinedPixelTrait)
              *opacity++=ScaleQuantumToChar((Quantum) (GetPixelAlpha(image,p)));
            p+=GetPixelChannels(image);
          }
          count+=EncodeImage(image,scanline,bytes_per_line,packed_scanline);
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

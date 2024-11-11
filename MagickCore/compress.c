/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           CCCC   OOO   M   M  PPPP   RRRR   EEEEE   SSSSS  SSSSS            %
%          C      O   O  MM MM  P   P  R   R  E       SS     SS               %
%          C      O   O  M M M  PPPP   RRRR   EEE      SSS    SSS             %
%          C      O   O  M   M  P      R R    E          SS     SS            %
%           CCCC   OOO   M   M  P      R  R   EEEEE   SSSSS  SSSSS            %
%                                                                             %
%                                                                             %
%             MagickCore Image Compression/Decompression Methods              %
%                                                                             %
%                           Software Design                                   %
%                                Cristy                                       %
%                              May  1993                                      %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/color-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/compress.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#if defined(MAGICKCORE_ZLIB_DELEGATE)
#include "zlib.h"
#endif

/*
  Typedef declarations.
*/
struct _Ascii85Info
{
  ssize_t
    offset,
    line_break;

  char
    tuple[6];

  unsigned char
    buffer[10];
};

typedef struct HuffmanTable
{
  size_t
    id,
    code,
    length,
    count;
} HuffmanTable;

/*
  Huffman coding declarations.
*/
#define TWId  23L
#define MWId  24L
#define TBId  25L
#define MBId  26L
#define EXId  27L

static const HuffmanTable
  MBTable[]=
  {
    { MBId, 0x0f, 10, 64 }, { MBId, 0xc8, 12, 128 },
    { MBId, 0xc9, 12, 192 }, { MBId, 0x5b, 12, 256 },
    { MBId, 0x33, 12, 320 }, { MBId, 0x34, 12, 384 },
    { MBId, 0x35, 12, 448 }, { MBId, 0x6c, 13, 512 },
    { MBId, 0x6d, 13, 576 }, { MBId, 0x4a, 13, 640 },
    { MBId, 0x4b, 13, 704 }, { MBId, 0x4c, 13, 768 },
    { MBId, 0x4d, 13, 832 }, { MBId, 0x72, 13, 896 },
    { MBId, 0x73, 13, 960 }, { MBId, 0x74, 13, 1024 },
    { MBId, 0x75, 13, 1088 }, { MBId, 0x76, 13, 1152 },
    { MBId, 0x77, 13, 1216 }, { MBId, 0x52, 13, 1280 },
    { MBId, 0x53, 13, 1344 }, { MBId, 0x54, 13, 1408 },
    { MBId, 0x55, 13, 1472 }, { MBId, 0x5a, 13, 1536 },
    { MBId, 0x5b, 13, 1600 }, { MBId, 0x64, 13, 1664 },
    { MBId, 0x65, 13, 1728 }, { MBId, 0x00, 0, 0 }
  };

static const HuffmanTable
  EXTable[]=
  {
    { EXId, 0x08, 11, 1792 }, { EXId, 0x0c, 11, 1856 },
    { EXId, 0x0d, 11, 1920 }, { EXId, 0x12, 12, 1984 },
    { EXId, 0x13, 12, 2048 }, { EXId, 0x14, 12, 2112 },
    { EXId, 0x15, 12, 2176 }, { EXId, 0x16, 12, 2240 },
    { EXId, 0x17, 12, 2304 }, { EXId, 0x1c, 12, 2368 },
    { EXId, 0x1d, 12, 2432 }, { EXId, 0x1e, 12, 2496 },
    { EXId, 0x1f, 12, 2560 }, { EXId, 0x00, 0, 0 }
  };

static const HuffmanTable
  MWTable[]=
  {
    { MWId, 0x1b, 5, 64 }, { MWId, 0x12, 5, 128 },
    { MWId, 0x17, 6, 192 }, { MWId, 0x37, 7, 256 },
    { MWId, 0x36, 8, 320 }, { MWId, 0x37, 8, 384 },
    { MWId, 0x64, 8, 448 }, { MWId, 0x65, 8, 512 },
    { MWId, 0x68, 8, 576 }, { MWId, 0x67, 8, 640 },
    { MWId, 0xcc, 9, 704 }, { MWId, 0xcd, 9, 768 },
    { MWId, 0xd2, 9, 832 }, { MWId, 0xd3, 9, 896 },
    { MWId, 0xd4, 9, 960 }, { MWId, 0xd5, 9, 1024 },
    { MWId, 0xd6, 9, 1088 }, { MWId, 0xd7, 9, 1152 },
    { MWId, 0xd8, 9, 1216 }, { MWId, 0xd9, 9, 1280 },
    { MWId, 0xda, 9, 1344 }, { MWId, 0xdb, 9, 1408 },
    { MWId, 0x98, 9, 1472 }, { MWId, 0x99, 9, 1536 },
    { MWId, 0x9a, 9, 1600 }, { MWId, 0x18, 6, 1664 },
    { MWId, 0x9b, 9, 1728 }, { MWId, 0x00, 0, 0 }
  };

static const HuffmanTable
  TBTable[]=
  {
    { TBId, 0x37, 10, 0 }, { TBId, 0x02, 3, 1 }, { TBId, 0x03, 2, 2 },
    { TBId, 0x02, 2, 3 }, { TBId, 0x03, 3, 4 }, { TBId, 0x03, 4, 5 },
    { TBId, 0x02, 4, 6 }, { TBId, 0x03, 5, 7 }, { TBId, 0x05, 6, 8 },
    { TBId, 0x04, 6, 9 }, { TBId, 0x04, 7, 10 }, { TBId, 0x05, 7, 11 },
    { TBId, 0x07, 7, 12 }, { TBId, 0x04, 8, 13 }, { TBId, 0x07, 8, 14 },
    { TBId, 0x18, 9, 15 }, { TBId, 0x17, 10, 16 }, { TBId, 0x18, 10, 17 },
    { TBId, 0x08, 10, 18 }, { TBId, 0x67, 11, 19 }, { TBId, 0x68, 11, 20 },
    { TBId, 0x6c, 11, 21 }, { TBId, 0x37, 11, 22 }, { TBId, 0x28, 11, 23 },
    { TBId, 0x17, 11, 24 }, { TBId, 0x18, 11, 25 }, { TBId, 0xca, 12, 26 },
    { TBId, 0xcb, 12, 27 }, { TBId, 0xcc, 12, 28 }, { TBId, 0xcd, 12, 29 },
    { TBId, 0x68, 12, 30 }, { TBId, 0x69, 12, 31 }, { TBId, 0x6a, 12, 32 },
    { TBId, 0x6b, 12, 33 }, { TBId, 0xd2, 12, 34 }, { TBId, 0xd3, 12, 35 },
    { TBId, 0xd4, 12, 36 }, { TBId, 0xd5, 12, 37 }, { TBId, 0xd6, 12, 38 },
    { TBId, 0xd7, 12, 39 }, { TBId, 0x6c, 12, 40 }, { TBId, 0x6d, 12, 41 },
    { TBId, 0xda, 12, 42 }, { TBId, 0xdb, 12, 43 }, { TBId, 0x54, 12, 44 },
    { TBId, 0x55, 12, 45 }, { TBId, 0x56, 12, 46 }, { TBId, 0x57, 12, 47 },
    { TBId, 0x64, 12, 48 }, { TBId, 0x65, 12, 49 }, { TBId, 0x52, 12, 50 },
    { TBId, 0x53, 12, 51 }, { TBId, 0x24, 12, 52 }, { TBId, 0x37, 12, 53 },
    { TBId, 0x38, 12, 54 }, { TBId, 0x27, 12, 55 }, { TBId, 0x28, 12, 56 },
    { TBId, 0x58, 12, 57 }, { TBId, 0x59, 12, 58 }, { TBId, 0x2b, 12, 59 },
    { TBId, 0x2c, 12, 60 }, { TBId, 0x5a, 12, 61 }, { TBId, 0x66, 12, 62 },
    { TBId, 0x67, 12, 63 }, { TBId, 0x00, 0, 0 }
  };

static const HuffmanTable
  TWTable[]=
  {
    { TWId, 0x35, 8, 0 }, { TWId, 0x07, 6, 1 }, { TWId, 0x07, 4, 2 },
    { TWId, 0x08, 4, 3 }, { TWId, 0x0b, 4, 4 }, { TWId, 0x0c, 4, 5 },
    { TWId, 0x0e, 4, 6 }, { TWId, 0x0f, 4, 7 }, { TWId, 0x13, 5, 8 },
    { TWId, 0x14, 5, 9 }, { TWId, 0x07, 5, 10 }, { TWId, 0x08, 5, 11 },
    { TWId, 0x08, 6, 12 }, { TWId, 0x03, 6, 13 }, { TWId, 0x34, 6, 14 },
    { TWId, 0x35, 6, 15 }, { TWId, 0x2a, 6, 16 }, { TWId, 0x2b, 6, 17 },
    { TWId, 0x27, 7, 18 }, { TWId, 0x0c, 7, 19 }, { TWId, 0x08, 7, 20 },
    { TWId, 0x17, 7, 21 }, { TWId, 0x03, 7, 22 }, { TWId, 0x04, 7, 23 },
    { TWId, 0x28, 7, 24 }, { TWId, 0x2b, 7, 25 }, { TWId, 0x13, 7, 26 },
    { TWId, 0x24, 7, 27 }, { TWId, 0x18, 7, 28 }, { TWId, 0x02, 8, 29 },
    { TWId, 0x03, 8, 30 }, { TWId, 0x1a, 8, 31 }, { TWId, 0x1b, 8, 32 },
    { TWId, 0x12, 8, 33 }, { TWId, 0x13, 8, 34 }, { TWId, 0x14, 8, 35 },
    { TWId, 0x15, 8, 36 }, { TWId, 0x16, 8, 37 }, { TWId, 0x17, 8, 38 },
    { TWId, 0x28, 8, 39 }, { TWId, 0x29, 8, 40 }, { TWId, 0x2a, 8, 41 },
    { TWId, 0x2b, 8, 42 }, { TWId, 0x2c, 8, 43 }, { TWId, 0x2d, 8, 44 },
    { TWId, 0x04, 8, 45 }, { TWId, 0x05, 8, 46 }, { TWId, 0x0a, 8, 47 },
    { TWId, 0x0b, 8, 48 }, { TWId, 0x52, 8, 49 }, { TWId, 0x53, 8, 50 },
    { TWId, 0x54, 8, 51 }, { TWId, 0x55, 8, 52 }, { TWId, 0x24, 8, 53 },
    { TWId, 0x25, 8, 54 }, { TWId, 0x58, 8, 55 }, { TWId, 0x59, 8, 56 },
    { TWId, 0x5a, 8, 57 }, { TWId, 0x5b, 8, 58 }, { TWId, 0x4a, 8, 59 },
    { TWId, 0x4b, 8, 60 }, { TWId, 0x32, 8, 61 }, { TWId, 0x33, 8, 62 },
    { TWId, 0x34, 8, 63 }, { TWId, 0x00, 0, 0 }
  };

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A S C I I 8 5 E n c o d e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ASCII85Encode() encodes data in ASCII base-85 format.  ASCII base-85
%  encoding produces five ASCII printing characters from every four bytes of
%  binary data.
%
%  The format of the ASCII85Encode method is:
%
%      void Ascii85Encode(Image *image,const size_t code)
%
%  A description of each parameter follows:
%
%    o code: a binary unsigned char to encode to ASCII 85.
%
%    o file: write the encoded ASCII character to this file.
%
%
*/
static inline void Ascii85Tuple(Ascii85Info *ascii85_info,
  const unsigned char *magick_restrict data)
{
#define MaxLineExtent  36L

  size_t
    code,
    i,
    quantum,
    x;

  code=((((size_t) data[0] << 8) | (size_t) data[1]) << 16) |
    ((size_t) data[2] << 8) | (size_t) data[3];
  if (code == 0L)
    {
      ascii85_info->tuple[0]='z';
      ascii85_info->tuple[1]='\0';
      return;
    }
  quantum=85UL*85UL*85UL*85UL;
  for (i=0; i < 4; i++)
  {
    x=(code/quantum);
    code-=quantum*x;
    ascii85_info->tuple[i]=(char) (x+(int) '!');
    quantum/=85L;
  }
  ascii85_info->tuple[4]=(char) ((code % 85L)+(int) '!');
  ascii85_info->tuple[5]='\0';
}

MagickExport void Ascii85Initialize(Image *image)
{
  /*
    Allocate image structure.
  */
  if (image->ascii85 == (Ascii85Info *) NULL)
    image->ascii85=(Ascii85Info *) AcquireMagickMemory(sizeof(*image->ascii85));
  if (image->ascii85 == (Ascii85Info *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memset(image->ascii85,0,sizeof(*image->ascii85));
  image->ascii85->line_break=(ssize_t) (MaxLineExtent << 1);
  image->ascii85->offset=0;
}

MagickExport void Ascii85Flush(Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->ascii85 != (Ascii85Info *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->ascii85->offset > 0)
    {
      image->ascii85->buffer[image->ascii85->offset]='\0';
      image->ascii85->buffer[image->ascii85->offset+1]='\0';
      image->ascii85->buffer[image->ascii85->offset+2]='\0';
      Ascii85Tuple(image->ascii85,image->ascii85->buffer);
      (void) WriteBlob(image,(size_t) image->ascii85->offset+1,
        (const unsigned char *) (*image->ascii85->tuple == 'z' ? "!!!!" :
        image->ascii85->tuple));
    }
  (void) WriteBlobByte(image,'~');
  (void) WriteBlobByte(image,'>');
  (void) WriteBlobByte(image,'\n');
}

MagickExport void Ascii85Encode(Image *image,const unsigned char code)
{
  char
    *q;

  unsigned char
    *p;

  ssize_t
    n;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->ascii85 != (Ascii85Info *) NULL);
  image->ascii85->buffer[image->ascii85->offset]=code;
  image->ascii85->offset++;
  if (image->ascii85->offset < 4)
    return;
  p=image->ascii85->buffer;
  for (n=image->ascii85->offset; n >= 4; n-=4)
  {
    Ascii85Tuple(image->ascii85,p);
    for (q=image->ascii85->tuple; *q != '\0'; q++)
    {
      image->ascii85->line_break--;
      if ((image->ascii85->line_break < 0) && (*q != '%'))
        {
          (void) WriteBlobByte(image,'\n');
          image->ascii85->line_break=2*MaxLineExtent;
        }
      (void) WriteBlobByte(image,(unsigned char) *q);
    }
    p+=(ptrdiff_t) 8;
  }
  image->ascii85->offset=n;
  p-=(ptrdiff_t)4;
  for (n=0; n < 4; n++)
    image->ascii85->buffer[n]=(*p++);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H u f f m a n D e c o d e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  HuffmanDecodeImage() uncompresses an image via Huffman-coding.
%
%  The format of the HuffmanDecodeImage method is:
%
%      MagickBooleanType HuffmanDecodeImage(Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType HuffmanDecodeImage(Image *image,
  ExceptionInfo *exception)
{
#define HashSize  1021L
#define MBHashA  293L
#define MBHashB  2695L
#define MWHashA  3510L
#define MWHashB  1178L

#define InitializeHashTable(hash,table,a,b) \
{ \
  entry=table; \
  while (entry->code != 0) \
  {  \
    hash[((entry->length+a)*(entry->code+b)) % HashSize]=(HuffmanTable *) entry; \
    entry++; \
  } \
}

#define InputBit(bit)  \
{  \
  if ((mask & 0xff) == 0)  \
    {  \
      byte=ReadBlobByte(image);  \
      if (byte == EOF)  \
        break;  \
      mask=0x80;  \
    }  \
  runlength++;  \
  bit=(size_t) ((byte & mask) != 0 ? 0x01 : 0x00); \
  mask>>=1;  \
  if (bit != 0)  \
    runlength=0;  \
}

  CacheView
    *image_view;

  const HuffmanTable
    *entry;

  HuffmanTable
    **mb_hash,
    **mw_hash;

  int
    byte,
    mask;

  MagickBooleanType
    proceed;

  Quantum
    index;

  size_t
    bit,
    code,
    length,
    null_lines,
    runlength;

  ssize_t
    count,
    i,
    y;

  unsigned char
    *p,
    *scanline;

  unsigned int
    bail,
    color;

  /*
    Allocate buffers.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->blob == (BlobInfo *) NULL)
    ThrowBinaryException(BlobError,"UnableToOpenBlob",image->filename);
  mb_hash=(HuffmanTable **) AcquireQuantumMemory(HashSize,sizeof(*mb_hash));
  mw_hash=(HuffmanTable **) AcquireQuantumMemory(HashSize,sizeof(*mw_hash));
  scanline=(unsigned char *) AcquireQuantumMemory((size_t) image->columns,
    sizeof(*scanline));
  if ((mb_hash == (HuffmanTable **) NULL) ||
      (mw_hash == (HuffmanTable **) NULL) ||
      (scanline == (unsigned char *) NULL))
    {
      if (mb_hash != (HuffmanTable **) NULL)
        mb_hash=(HuffmanTable **) RelinquishMagickMemory(mb_hash);
      if (mw_hash != (HuffmanTable **) NULL)
        mw_hash=(HuffmanTable **) RelinquishMagickMemory(mw_hash);
      if (scanline != (unsigned char *) NULL)
        scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Initialize Huffman tables.
  */
  for (i=0; i < HashSize; i++)
  {
    mb_hash[i]=(HuffmanTable *) NULL;
    mw_hash[i]=(HuffmanTable *) NULL;
  }
  InitializeHashTable(mw_hash,TWTable,MWHashA,MWHashB);
  InitializeHashTable(mw_hash,MWTable,MWHashA,MWHashB);
  InitializeHashTable(mw_hash,EXTable,MWHashA,MWHashB);
  InitializeHashTable(mb_hash,TBTable,MBHashA,MBHashB);
  InitializeHashTable(mb_hash,MBTable,MBHashA,MBHashB);
  InitializeHashTable(mb_hash,EXTable,MBHashA,MBHashB);
  /*
    Uncompress 1D Huffman to runlength encoded pixels.
  */
  byte=0;
  mask=0;
  null_lines=0;
  runlength=0;
  while (runlength < 11)
   InputBit(bit);
  do { InputBit(bit); } while ((int) bit == 0);
  image->resolution.x=204.0;
  image->resolution.y=196.0;
  image->units=PixelsPerInchResolution;
  image_view=AcquireAuthenticCacheView(image,exception);
  for (y=0; ((y < (ssize_t) image->rows) && (null_lines < 3)); )
  {
    Quantum
      *magick_restrict q;

    ssize_t
      x;

    /*
      Initialize scanline to white.
    */
    memset(scanline,0,sizeof(*scanline)*image->columns);
    /*
      Decode Huffman encoded scanline.
    */
    color=MagickTrue;
    code=0;
    count=0;
    length=0;
    runlength=0;
    x=0;
    for ( ; ; )
    {
      if (byte == EOF)
        break;
      if (x >= (ssize_t) image->columns)
        {
          while (runlength < 11)
            InputBit(bit);
          do { InputBit(bit); } while ((int) bit == 0);
          break;
        }
      bail=MagickFalse;
      do
      {
        if (runlength < 11)
          InputBit(bit)
        else
          {
            InputBit(bit);
            if ((int) bit != 0)
              {
                null_lines++;
                if (x != 0)
                  null_lines=0;
                bail=MagickTrue;
                break;
              }
          }
        code=(code << 1)+(size_t) bit;
        length++;
      } while (code == 0);
      if (bail != MagickFalse)
        break;
      if (length > 13)
        {
          while (runlength < 11)
            InputBit(bit);
          do { InputBit(bit); } while ((int) bit == 0);
          break;
        }
      if (color != MagickFalse)
        {
          if (length < 4)
            continue;
          entry=mw_hash[((length+MWHashA)*(code+MWHashB)) % HashSize];
        }
      else
        {
          if (length < 2)
            continue;
          entry=mb_hash[((length+MBHashA)*(code+MBHashB)) % HashSize];
        }
      if (entry == (const HuffmanTable *) NULL)
        continue;
      if ((entry->length != length) || (entry->code != code))
        continue;
      switch (entry->id)
      {
        case TWId:
        case TBId:
        {
          count+=(ssize_t) entry->count;
          if ((x+count) > (ssize_t) image->columns)
            count=(ssize_t) image->columns-x;
          if (count > 0)
            {
              if (color != MagickFalse)
                {
                  x+=count;
                  count=0;
                }
              else
                for ( ; count > 0; count--)
                  if ((x >= 0) && (x < (ssize_t) image->columns))
                    scanline[x++]=(unsigned char) 1;
            }
          color=(unsigned int)
            ((color == MagickFalse) ? MagickTrue : MagickFalse);
          break;
        }
        case MWId:
        case MBId:
        case EXId:
        {
          count+=(ssize_t) entry->count;
          break;
        }
        default:
          break;
      }
      code=0;
      length=0;
    }
    /*
      Transfer scanline to image pixels.
    */
    p=scanline;
    q=QueueCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      index=(Quantum) (*p++);
      SetPixelIndex(image,index,q);
      SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      break;
    proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (proceed == MagickFalse)
      break;
    y++;
  }
  image_view=DestroyCacheView(image_view);
  image->rows=(size_t) MagickMax((size_t) y-3,1);
  image->compression=FaxCompression;
  /*
    Free decoder memory.
  */
  mw_hash=(HuffmanTable **) RelinquishMagickMemory(mw_hash);
  mb_hash=(HuffmanTable **) RelinquishMagickMemory(mb_hash);
  scanline=(unsigned char *) RelinquishMagickMemory(scanline);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H u f f m a n E n c o d e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  HuffmanEncodeImage() compresses an image via Huffman-coding.
%
%  The format of the HuffmanEncodeImage method is:
%
%      MagickBooleanType HuffmanEncodeImage(const ImageInfo *image_info,
%        Image *image,Image *inject_image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info..
%
%    o image: the image.
%
%    o inject_image: inject into the image stream.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType HuffmanEncodeImage(const ImageInfo *image_info,
  Image *image,Image *inject_image,ExceptionInfo *exception)
{
#define HuffmanOutputCode(entry)  \
{  \
  mask=one << (entry->length-1);  \
  while (mask != 0)  \
  {  \
    OutputBit(((entry->code & mask) != 0 ? 1 : 0));  \
    mask>>=1;  \
  }  \
}

#define OutputBit(count)  \
{  \
DisableMSCWarning(4127) \
  if (count > 0)  \
    byte=byte | bit;  \
RestoreMSCWarning \
  bit>>=1;  \
  if ((int) (bit & 0xff) == 0)   \
    {  \
      if (LocaleCompare(image_info->magick,"FAX") == 0) \
        (void) WriteBlobByte(image,(unsigned char) byte);  \
      else \
        Ascii85Encode(image,byte); \
      byte='\0';  \
      bit=(unsigned char) 0x80;  \
    }  \
}

  const HuffmanTable
    *entry;

  int
    k,
    runlength;

  Image
    *huffman_image;

  MagickBooleanType
    proceed;

  ssize_t
    i,
    x;

  const Quantum
    *p;

  unsigned char
    *q;

  size_t
    mask,
    one,
    width;

  ssize_t
    n,
    y;

  unsigned char
    byte,
    bit,
    *scanline;

  /*
    Allocate scanline buffer.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(inject_image != (Image *) NULL);
  assert(inject_image->signature == MagickCoreSignature);
  one=1;
  width=inject_image->columns;
  if (LocaleCompare(image_info->magick,"FAX") == 0)
    width=(size_t) MagickMax(inject_image->columns,1728);
  scanline=(unsigned char *) AcquireQuantumMemory((size_t) width+1UL,
    sizeof(*scanline));
  if (scanline == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      inject_image->filename);
  (void) memset(scanline,0,width*sizeof(*scanline));
  huffman_image=CloneImage(inject_image,0,0,MagickTrue,exception);
  if (huffman_image == (Image *) NULL)
    {
      scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      return(MagickFalse);
    }
  (void) SetImageType(huffman_image,BilevelType,exception);
  byte='\0';
  bit=(unsigned char) 0x80;
  if (LocaleCompare(image_info->magick,"FAX") != 0)
    Ascii85Initialize(image);
  else
    {
      /*
        End of line.
      */
      for (k=0; k < 11; k++)
        OutputBit(0);
      OutputBit(1);
    }
  /*
    Compress to 1D Huffman pixels.
  */
  q=scanline;
  for (y=0; y < (ssize_t) huffman_image->rows; y++)
  {
    p=GetVirtualPixels(huffman_image,0,y,huffman_image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) huffman_image->columns; x++)
    {
      *q++=(unsigned char) (GetPixelIntensity(huffman_image,p) >=
        ((double) QuantumRange/2.0) ? 0 : 1);
      p+=(ptrdiff_t) GetPixelChannels(huffman_image);
    }
    /*
      Huffman encode scanline.
    */
    q=scanline;
    for (n=(ssize_t) width; n > 0; )
    {
      /*
        Output white run.
      */
      for (runlength=0; ((n > 0) && (*q == 0)); n--)
      {
        q++;
        runlength++;
      }
      if (runlength >= 64)
        {
          if (runlength < 1792)
            entry=MWTable+((runlength/64)-1);
          else
            entry=EXTable+(MagickMin((size_t) runlength,2560)-1792)/64;
          runlength-=(long) entry->count;
          HuffmanOutputCode(entry);
        }
      entry=TWTable+MagickMin((size_t) runlength,63);
      HuffmanOutputCode(entry);
      if (n != 0)
        {
          /*
            Output black run.
          */
          for (runlength=0; ((*q != 0) && (n > 0)); n--)
          {
            q++;
            runlength++;
          }
          if (runlength >= 64)
            {
              entry=MBTable+((runlength/64)-1);
              if (runlength >= 1792)
                entry=EXTable+(MagickMin((size_t) runlength,2560)-1792)/64;
              runlength-=(long) entry->count;
              HuffmanOutputCode(entry);
            }
          entry=TBTable+MagickMin((size_t) runlength,63);
          HuffmanOutputCode(entry);
        }
    }
    /*
      End of line.
    */
    for (k=0; k < 11; k++)
      OutputBit(0);
    OutputBit(1);
    q=scanline;
    if (GetPreviousImageInList(huffman_image) == (Image *) NULL)
      {
        proceed=SetImageProgress(huffman_image,LoadImageTag,
          (MagickOffsetType) y,huffman_image->rows);
        if (proceed == MagickFalse)
          break;
      }
  }
  /*
    End of page.
  */
  for (i=0; i < 6; i++)
  {
    for (k=0; k < 11; k++)
      OutputBit(0);
    OutputBit(1);
  }
  /*
    Flush bits.
  */
  if (((int) bit != 0x80) != 0)
    {
      if (LocaleCompare(image_info->magick,"FAX") == 0)
        (void) WriteBlobByte(image,byte);
      else
        Ascii85Encode(image,byte);
    }
  if (LocaleCompare(image_info->magick,"FAX") != 0)
    Ascii85Flush(image);
  huffman_image=DestroyImage(huffman_image);
  scanline=(unsigned char *) RelinquishMagickMemory(scanline);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L Z W E n c o d e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LZWEncodeImage() compresses an image via LZW-coding specific to Postscript
%  Level II or Portable Document Format.
%
%  The format of the LZWEncodeImage method is:
%
%      MagickBooleanType LZWEncodeImage(Image *image,const size_t length,
%        unsigned char *magick_restrict pixels,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o length:  A value that specifies the number of pixels to compress.
%
%    o pixels: the address of an unsigned array of characters containing the
%      pixels to compress.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType LZWEncodeImage(Image *image,const size_t length,
  unsigned char *magick_restrict pixels,ExceptionInfo *exception)
{
#define LZWClr  256UL  /* Clear Table Marker */
#define LZWEod  257UL  /* End of Data marker */
#define OutputCode(code) \
{ \
    accumulator+=code << (32-code_width-number_bits); \
    number_bits+=code_width; \
    while (number_bits >= 8) \
    { \
      (void) WriteBlobByte(image,(unsigned char) (accumulator >> 24)); \
      accumulator=accumulator << 8; \
      number_bits-=8; \
    } \
}

  typedef struct _TableType
  {
    ssize_t
      prefix,
      suffix,
      next;
  } TableType;

  ssize_t
    i;

  size_t
    accumulator,
    number_bits,
    code_width,
    last_code,
    next_index;

  ssize_t
    index;

  TableType
    *table;

  /*
    Allocate string table.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(pixels != (unsigned char *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  table=(TableType *) AcquireQuantumMemory(1UL << 12,sizeof(*table));
  if (table == (TableType *) NULL)
    ThrowBinaryException(ResourceLimitWarning,"MemoryAllocationFailed",
      image->filename);
  /*
    Initialize variables.
  */
  accumulator=0;
  code_width=9;
  number_bits=0;
  last_code=0;
  OutputCode(LZWClr);
  for (index=0; index < 256; index++)
  {
    table[index].prefix=(-1);
    table[index].suffix=(ssize_t) index;
    table[index].next=(-1);
  }
  next_index=LZWEod+1;
  code_width=9;
  last_code=(size_t) pixels[0];
  for (i=1; i < (ssize_t) length; i++)
  {
    /*
      Find string.
    */
    index=(ssize_t) last_code;
    while (index != -1)
      if ((table[index].prefix != (ssize_t) last_code) ||
          (table[index].suffix != (ssize_t) pixels[i]))
        index=table[index].next;
      else
        {
          last_code=(size_t) index;
          break;
        }
    if (last_code != (size_t) index)
      {
        /*
          Add string.
        */
        OutputCode(last_code);
        table[next_index].prefix=(ssize_t) last_code;
        table[next_index].suffix=(ssize_t) pixels[i];
        table[next_index].next=table[last_code].next;
        table[last_code].next=(ssize_t) next_index;
        next_index++;
        /*
          Did we just move up to next bit width?
        */
        if ((next_index >> code_width) != 0)
          {
            code_width++;
            if (code_width > 12)
              {
                /*
                  Did we overflow the max bit width?
                */
                code_width--;
                OutputCode(LZWClr);
                for (index=0; index < 256; index++)
                {
                  table[index].prefix=(-1);
                  table[index].suffix=index;
                  table[index].next=(-1);
                }
                next_index=LZWEod+1;
                code_width=9;
              }
            }
          last_code=(size_t) pixels[i];
      }
  }
  /*
    Flush tables.
  */
  OutputCode(last_code);
  OutputCode(LZWEod);
  if (number_bits != 0)
    (void) WriteBlobByte(image,(unsigned char) (accumulator >> 24));
  table=(TableType *) RelinquishMagickMemory(table);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a c k b i t s E n c o d e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PackbitsEncodeImage() compresses an image via Macintosh Packbits encoding
%  specific to Postscript Level II or Portable Document Format.  To ensure
%  portability, the binary Packbits bytes are encoded as ASCII Base-85.
%
%  The format of the PackbitsEncodeImage method is:
%
%      MagickBooleanType PackbitsEncodeImage(Image *image,const size_t length,
%        unsigned char *magick_restrict pixels)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o length:  A value that specifies the number of pixels to compress.
%
%    o pixels: the address of an unsigned array of characters containing the
%      pixels to compress.
%
*/
MagickExport MagickBooleanType PackbitsEncodeImage(Image *image,
  const size_t length,unsigned char *magick_restrict pixels,
  ExceptionInfo *exception)
{
  int
    count;

  ssize_t
    i,
    j;

  unsigned char
    *packbits;

  /*
    Compress pixels with Packbits encoding.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(pixels != (unsigned char *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  packbits=(unsigned char *) AcquireQuantumMemory(128UL,sizeof(*packbits));
  if (packbits == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  for (i=(ssize_t) length; i != 0; )
  {
    switch (i)
    {
      case 1:
      {
        i--;
        (void) WriteBlobByte(image,(unsigned char) 0);
        (void) WriteBlobByte(image,*pixels);
        break;
      }
      case 2:
      {
        i-=2;
        (void) WriteBlobByte(image,(unsigned char) 1);
        (void) WriteBlobByte(image,*pixels);
        (void) WriteBlobByte(image,pixels[1]);
        break;
      }
      case 3:
      {
        i-=3;
        if ((*pixels == *(pixels+1)) && (*(pixels+1) == *(pixels+2)))
          {
            (void) WriteBlobByte(image,(unsigned char) ((256-3)+1));
            (void) WriteBlobByte(image,*pixels);
            break;
          }
        (void) WriteBlobByte(image,(unsigned char) 2);
        (void) WriteBlobByte(image,*pixels);
        (void) WriteBlobByte(image,pixels[1]);
        (void) WriteBlobByte(image,pixels[2]);
        break;
      }
      default:
      {
        if ((*pixels == *(pixels+1)) && (*(pixels+1) == *(pixels+2)))
          {
            /*
              Packed run.
            */
            count=3;
            while (((ssize_t) count < i) && (*pixels == *(pixels+count)))
            {
              count++;
              if (count >= 127)
                break;
            }
            i-=count;
            (void) WriteBlobByte(image,(unsigned char) ((256-count)+1));
            (void) WriteBlobByte(image,*pixels);
            pixels+=count;
            break;
          }
        /*
          Literal run.
        */
        count=0;
        while ((*(pixels+count) != *(pixels+count+1)) ||
               (*(pixels+count+1) != *(pixels+count+2)))
        {
          packbits[count+1]=pixels[count];
          count++;
          if (((ssize_t) count >= (i-3)) || (count >= 127))
            break;
        }
        i-=count;
        *packbits=(unsigned char) (count-1);
        for (j=0; j <= (ssize_t) count; j++)
          (void) WriteBlobByte(image,packbits[j]);
        pixels+=count;
        break;
      }
    }
  }
  (void) WriteBlobByte(image,(unsigned char) 128);  /* EOD marker */
  packbits=(unsigned char *) RelinquishMagickMemory(packbits);
  return(MagickTrue);
}

#if defined(MAGICKCORE_ZLIB_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Z L I B E n c o d e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ZLIBEncodeImage compresses an image via ZLIB-coding specific to
%  Postscript Level II or Portable Document Format.
%
%  The format of the ZLIBEncodeImage method is:
%
%      MagickBooleanType ZLIBEncodeImage(Image *image,const size_t length,
%        unsigned char *magick_restrict pixels,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o file: the address of a structure of type FILE.  ZLIB encoded pixels
%      are written to this file.
%
%    o length:  A value that specifies the number of pixels to compress.
%
%    o pixels: the address of an unsigned array of characters containing the
%      pixels to compress.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static voidpf AcquireZIPMemory(voidpf context,unsigned int items,
  unsigned int size)
{
  (void) context;
  return((voidpf) AcquireQuantumMemory(items,size));
}

static void RelinquishZIPMemory(voidpf context,voidpf memory)
{
  (void) context;
  memory=RelinquishMagickMemory(memory);
}

MagickExport MagickBooleanType ZLIBEncodeImage(Image *image,const size_t length,
  unsigned char *magick_restrict pixels,ExceptionInfo *exception)
{
  int
    status;

  ssize_t
    i;

  size_t
    compress_packets;

  unsigned char
    *compress_pixels;

  z_stream
    stream;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  compress_packets=(size_t) (1.001*length+12);
  compress_pixels=(unsigned char *) AcquireQuantumMemory(compress_packets,
    sizeof(*compress_pixels));
  if (compress_pixels == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  (void) memset(&stream,0,sizeof(stream));
  stream.next_in=pixels;
  stream.avail_in=(unsigned int) length;
  stream.next_out=compress_pixels;
  stream.avail_out=(unsigned int) compress_packets;
  stream.zalloc=AcquireZIPMemory;
  stream.zfree=RelinquishZIPMemory;
  stream.opaque=(voidpf) NULL;
  status=deflateInit(&stream,(int) (image->quality ==
    UndefinedCompressionQuality ? 7 : MagickMin(image->quality/10,9)));
  if (status == Z_OK)
    {
      status=deflate(&stream,Z_FINISH);
      if (status == Z_STREAM_END)
        status=deflateEnd(&stream);
      else
        (void) deflateEnd(&stream);
      compress_packets=(size_t) stream.total_out;
    }
  if (status != Z_OK)
    ThrowBinaryException(CoderError,"UnableToZipCompressImage",image->filename)
  for (i=0; i < (ssize_t) compress_packets; i++)
    (void) WriteBlobByte(image,compress_pixels[i]);
  compress_pixels=(unsigned char *) RelinquishMagickMemory(compress_pixels);
  return(MagickTrue);
}
#else
MagickExport MagickBooleanType ZLIBEncodeImage(Image *image,
  const size_t magick_unused(length),unsigned char *magick_unused(pixels),
  ExceptionInfo *exception)
{
  magick_unreferenced(length);
  magick_unreferenced(pixels);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","'%s' (ZIP)",image->filename);
  return(MagickFalse);
}
#endif

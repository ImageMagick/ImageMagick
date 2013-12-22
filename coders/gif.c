/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             GGGG  IIIII  FFFFF                              %
%                            G        I    F                                  %
%                            G  GG    I    FFF                                %
%                            G   G    I    F                                  %
%                             GGG   IIIII  F                                  %
%                                                                             %
%                                                                             %
%            Read/Write Compuserv Graphics Interchange Format                 %
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
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colormap-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/profile.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/module.h"

/*
  Define declarations.
*/
#define MaximumLZWBits  12
#define MaximumLZWCode  (1UL << MaximumLZWBits)

/*
  Typdef declarations.
*/
typedef struct _LZWCodeInfo
{
  unsigned char
    buffer[280];

  size_t
    count,
    bit;

  MagickBooleanType
    eof;
} LZWCodeInfo;

typedef struct _LZWStack
{
  size_t
    *codes,
    *index,
    *top;
} LZWStack;

typedef struct _LZWInfo
{
  Image
    *image;

  LZWStack
    *stack;

  MagickBooleanType
    genesis;

  size_t
    data_size,
    maximum_data_value,
    clear_code,
    end_code,
    bits,
    first_code,
    last_code,
    maximum_code,
    slot,
    *table[2];

  LZWCodeInfo
    code_info;
} LZWInfo;

/*
  Forward declarations.
*/
static inline int
  GetNextLZWCode(LZWInfo *,const size_t);

static MagickBooleanType
  WriteGIFImage(const ImageInfo *,Image *);

static ssize_t
  ReadBlobBlock(Image *,unsigned char *);

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
%  DecodeImage uncompresses an image via GIF-coding.
%
%  The format of the DecodeImage method is:
%
%      MagickBooleanType DecodeImage(Image *image,const ssize_t opacity)
%
%  A description of each parameter follows:
%
%    o image: the address of a structure of type Image.
%
%    o opacity:  The colormap index associated with the transparent color.
%
*/

static LZWInfo *RelinquishLZWInfo(LZWInfo *lzw_info)
{
  if (lzw_info->table[0] != (size_t *) NULL)
    lzw_info->table[0]=(size_t *) RelinquishMagickMemory(
      lzw_info->table[0]);
  if (lzw_info->table[1] != (size_t *) NULL)
    lzw_info->table[1]=(size_t *) RelinquishMagickMemory(
      lzw_info->table[1]);
  if (lzw_info->stack != (LZWStack *) NULL)
    {
      if (lzw_info->stack->codes != (size_t *) NULL)
        lzw_info->stack->codes=(size_t *) RelinquishMagickMemory(
          lzw_info->stack->codes);
      lzw_info->stack=(LZWStack *) RelinquishMagickMemory(lzw_info->stack);
    }
  lzw_info=(LZWInfo *) RelinquishMagickMemory(lzw_info);
  return((LZWInfo *) NULL);
}

static inline void ResetLZWInfo(LZWInfo *lzw_info)
{
  size_t
    one;

  lzw_info->bits=lzw_info->data_size+1;
  one=1;
  lzw_info->maximum_code=one << lzw_info->bits;
  lzw_info->slot=lzw_info->maximum_data_value+3;
  lzw_info->genesis=MagickTrue;
}

static LZWInfo *AcquireLZWInfo(Image *image,const size_t data_size)
{
  LZWInfo
    *lzw_info;

  register ssize_t
    i;

  size_t
    one;

  lzw_info=(LZWInfo *) AcquireMagickMemory(sizeof(*lzw_info));
  if (lzw_info == (LZWInfo *) NULL)
    return((LZWInfo *) NULL);
  (void) ResetMagickMemory(lzw_info,0,sizeof(*lzw_info));
  lzw_info->image=image;
  lzw_info->data_size=data_size;
  one=1;
  lzw_info->maximum_data_value=(one << data_size)-1;
  lzw_info->clear_code=lzw_info->maximum_data_value+1;
  lzw_info->end_code=lzw_info->maximum_data_value+2;
  lzw_info->table[0]=(size_t *) AcquireQuantumMemory(MaximumLZWCode,
    sizeof(*lzw_info->table));
  lzw_info->table[1]=(size_t *) AcquireQuantumMemory(MaximumLZWCode,
    sizeof(*lzw_info->table));
  if ((lzw_info->table[0] == (size_t *) NULL) ||
      (lzw_info->table[1] == (size_t *) NULL))
    {
      lzw_info=RelinquishLZWInfo(lzw_info);
      return((LZWInfo *) NULL);
    }
  for (i=0; i <= (ssize_t) lzw_info->maximum_data_value; i++)
  {
    lzw_info->table[0][i]=0;
    lzw_info->table[1][i]=(size_t) i;
  }
  ResetLZWInfo(lzw_info);
  lzw_info->code_info.buffer[0]='\0';
  lzw_info->code_info.buffer[1]='\0';
  lzw_info->code_info.count=2;
  lzw_info->code_info.bit=8*lzw_info->code_info.count;
  lzw_info->code_info.eof=MagickFalse;
  lzw_info->genesis=MagickTrue;
  lzw_info->stack=(LZWStack *) AcquireMagickMemory(sizeof(*lzw_info->stack));
  if (lzw_info->stack == (LZWStack *) NULL)
    {
      lzw_info=RelinquishLZWInfo(lzw_info);
      return((LZWInfo *) NULL);
    }
  lzw_info->stack->codes=(size_t *) AcquireQuantumMemory(2UL*
    MaximumLZWCode,sizeof(*lzw_info->stack->codes));
  if (lzw_info->stack->codes == (size_t *) NULL)
    {
      lzw_info=RelinquishLZWInfo(lzw_info);
      return((LZWInfo *) NULL);
    }
  lzw_info->stack->index=lzw_info->stack->codes;
  lzw_info->stack->top=lzw_info->stack->codes+2*MaximumLZWCode;
  return(lzw_info);
}

static inline int GetNextLZWCode(LZWInfo *lzw_info,const size_t bits)
{
  int
    code;

  register ssize_t
    i;

  size_t
    one;

  while (((lzw_info->code_info.bit+bits) > (8*lzw_info->code_info.count)) &&
         (lzw_info->code_info.eof == MagickFalse))
  {
    ssize_t
      count;

    lzw_info->code_info.buffer[0]=lzw_info->code_info.buffer[
      lzw_info->code_info.count-2];
    lzw_info->code_info.buffer[1]=lzw_info->code_info.buffer[
      lzw_info->code_info.count-1];
    lzw_info->code_info.bit-=8*(lzw_info->code_info.count-2);
    lzw_info->code_info.count=2;
    count=ReadBlobBlock(lzw_info->image,&lzw_info->code_info.buffer[
      lzw_info->code_info.count]);
    if (count > 0)
      lzw_info->code_info.count+=count;
    else
      lzw_info->code_info.eof=MagickTrue;
  }
  if ((lzw_info->code_info.bit+bits) > (8*lzw_info->code_info.count))
    return(-1);
  code=0;
  one=1;
  for (i=0; i < (ssize_t) bits; i++)
  {
    code|=((lzw_info->code_info.buffer[lzw_info->code_info.bit/8] &
      (one << (lzw_info->code_info.bit % 8))) != 0) << i;
    lzw_info->code_info.bit++;
  }
  return(code);
}

static inline int PopLZWStack(LZWStack *stack_info)
{
  if (stack_info->index <= stack_info->codes)
    return(-1);
  stack_info->index--;
  return((int) *stack_info->index);
}

static inline void PushLZWStack(LZWStack *stack_info,const size_t value)
{
  if (stack_info->index >= stack_info->top)
    return;
  *stack_info->index=value;
  stack_info->index++;
}

static int ReadBlobLZWByte(LZWInfo *lzw_info)
{
  int
    code;

  size_t
    one,
    value;

  ssize_t
    count;

  if (lzw_info->stack->index != lzw_info->stack->codes)
    return(PopLZWStack(lzw_info->stack));
  if (lzw_info->genesis != MagickFalse)
    {
      lzw_info->genesis=MagickFalse;
      do
      {
        lzw_info->first_code=(size_t) GetNextLZWCode(lzw_info,lzw_info->bits);
        lzw_info->last_code=lzw_info->first_code;
      } while (lzw_info->first_code == lzw_info->clear_code);
      return((int) lzw_info->first_code);
    }
  code=GetNextLZWCode(lzw_info,lzw_info->bits);
  if (code < 0)
    return(code);
  if ((size_t) code == lzw_info->clear_code)
    {
      ResetLZWInfo(lzw_info);
      return(ReadBlobLZWByte(lzw_info));
    }
  if ((size_t) code == lzw_info->end_code)
    return(-1);
  if ((size_t) code < lzw_info->slot)
    value=(size_t) code;
  else
    {
      PushLZWStack(lzw_info->stack,lzw_info->first_code);
      value=lzw_info->last_code;
    }
  count=0;
  while (value > lzw_info->maximum_data_value)
  {
    if ((size_t) count > MaximumLZWCode)
      return(-1);
    count++;
    if ((size_t) value > MaximumLZWCode)
      return(-1);
    PushLZWStack(lzw_info->stack,lzw_info->table[1][value]);
    value=lzw_info->table[0][value];
  }
  lzw_info->first_code=lzw_info->table[1][value];
  PushLZWStack(lzw_info->stack,lzw_info->first_code);
  one=1;
  if (lzw_info->slot < MaximumLZWCode)
    {
      lzw_info->table[0][lzw_info->slot]=lzw_info->last_code;
      lzw_info->table[1][lzw_info->slot]=lzw_info->first_code;
      lzw_info->slot++;
      if ((lzw_info->slot >= lzw_info->maximum_code) &&
          (lzw_info->bits < MaximumLZWBits))
        {
          lzw_info->bits++;
          lzw_info->maximum_code=one << lzw_info->bits;
        }
    }
  lzw_info->last_code=(size_t) code;
  return(PopLZWStack(lzw_info->stack));
}

static MagickBooleanType DecodeImage(Image *image,const ssize_t opacity)
{
  ExceptionInfo
    *exception;

  IndexPacket
    index;

  int
    c;

  LZWInfo
    *lzw_info;

  ssize_t
    offset,
    y;

  unsigned char
    data_size;

  size_t
    pass;

  /*
    Allocate decoder tables.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  data_size=(unsigned char) ReadBlobByte(image);
  if (data_size > MaximumLZWBits)
    ThrowBinaryException(CorruptImageError,"CorruptImage",image->filename);
  lzw_info=AcquireLZWInfo(image,data_size);
  if (lzw_info == (LZWInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  exception=(&image->exception);
  pass=0;
  offset=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register ssize_t
      x;

    register PixelPacket
      *restrict q;

    q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (ssize_t) image->columns; )
    {
      c=ReadBlobLZWByte(lzw_info);
      if (c < 0)
        break;
      index=ConstrainColormapIndex(image,(size_t) c);
      SetPixelIndex(indexes+x,index);
      SetPixelRGBO(q,image->colormap+(ssize_t) index);
      SetPixelOpacity(q,(ssize_t) index == opacity ? TransparentOpacity :
        OpaqueOpacity);
      x++;
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    if (x < (ssize_t) image->columns)
      break;
    if (image->interlace == NoInterlace)
      offset++;
    else
      {
        switch (pass)
        {
          case 0:
          default:
          {
            offset+=8;
            break;
          }
          case 1:
          {
            offset+=8;
            break;
          }
          case 2:
          {
            offset+=4;
            break;
          }
          case 3:
          {
            offset+=2;
            break;
          }
        }
      if ((pass == 0) && (offset >= (ssize_t) image->rows))
        {
          pass++;
          offset=4;
        }
      if ((pass == 1) && (offset >= (ssize_t) image->rows))
        {
          pass++;
          offset=2;
        }
      if ((pass == 2) && (offset >= (ssize_t) image->rows))
        {
          pass++;
          offset=1;
        }
    }
  }
  lzw_info=RelinquishLZWInfo(lzw_info);
  if (y < (ssize_t) image->rows)
    ThrowBinaryException(CorruptImageError,"CorruptImage",image->filename);
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
%  EncodeImage compresses an image via GIF-coding.
%
%  The format of the EncodeImage method is:
%
%      MagickBooleanType EncodeImage(const ImageInfo *image_info,Image *image,
%        const size_t data_size)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the address of a structure of type Image.
%
%    o data_size:  The number of bits in the compressed packet.
%
*/
static MagickBooleanType EncodeImage(const ImageInfo *image_info,Image *image,
  const size_t data_size)
{
#define MaxCode(number_bits)  ((one << (number_bits))-1)
#define MaxHashTable  5003
#define MaxGIFBits  12UL
#define MaxGIFTable  (1UL << MaxGIFBits)
#define GIFOutputCode(code) \
{ \
  /*  \
    Emit a code. \
  */ \
  if (bits > 0) \
    datum|=(code) << bits; \
  else \
    datum=code; \
  bits+=number_bits; \
  while (bits >= 8) \
  { \
    /*  \
      Add a character to current packet. \
    */ \
    packet[length++]=(unsigned char) (datum & 0xff); \
    if (length >= 254) \
      { \
        (void) WriteBlobByte(image,(unsigned char) length); \
        (void) WriteBlob(image,length,packet); \
        length=0; \
      } \
    datum>>=8; \
    bits-=8; \
  } \
  if (free_code > max_code)  \
    { \
      number_bits++; \
      if (number_bits == MaxGIFBits) \
        max_code=MaxGIFTable; \
      else \
        max_code=MaxCode(number_bits); \
    } \
}

  IndexPacket
    index;

  register ssize_t
    i;

  short
    *hash_code,
    *hash_prefix,
    waiting_code;

  size_t
    bits,
    clear_code,
    datum,
    end_of_information_code,
    free_code,
    length,
    max_code,
    next_pixel,
    number_bits,
    one,
    pass;

  ssize_t
    displacement,
    offset,
    k,
    y;

  unsigned char
    *packet,
    *hash_suffix;

  /*
    Allocate encoder tables.
  */
  assert(image != (Image *) NULL);
  one=1;
  packet=(unsigned char *) AcquireQuantumMemory(256,sizeof(*packet));
  hash_code=(short *) AcquireQuantumMemory(MaxHashTable,sizeof(*hash_code));
  hash_prefix=(short *) AcquireQuantumMemory(MaxHashTable,sizeof(*hash_prefix));
  hash_suffix=(unsigned char *) AcquireQuantumMemory(MaxHashTable,
    sizeof(*hash_suffix));
  if ((packet == (unsigned char *) NULL) || (hash_code == (short *) NULL) ||
      (hash_prefix == (short *) NULL) ||
      (hash_suffix == (unsigned char *) NULL))
    {
      if (packet != (unsigned char *) NULL)
        packet=(unsigned char *) RelinquishMagickMemory(packet);
      if (hash_code != (short *) NULL)
        hash_code=(short *) RelinquishMagickMemory(hash_code);
      if (hash_prefix != (short *) NULL)
        hash_prefix=(short *) RelinquishMagickMemory(hash_prefix);
      if (hash_suffix != (unsigned char *) NULL)
        hash_suffix=(unsigned char *) RelinquishMagickMemory(hash_suffix);
      return(MagickFalse);
    }
  /*
    Initialize GIF encoder.
  */
  number_bits=data_size;
  max_code=MaxCode(number_bits);
  clear_code=((short) one << (data_size-1));
  end_of_information_code=clear_code+1;
  free_code=clear_code+2;
  length=0;
  datum=0;
  bits=0;
  for (i=0; i < MaxHashTable; i++)
    hash_code[i]=0;
  GIFOutputCode(clear_code);
  /*
    Encode pixels.
  */
  offset=0;
  pass=0;
  waiting_code=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const IndexPacket
      *restrict indexes;

    register const PixelPacket
      *restrict p;

    register ssize_t
      x;

    p=GetVirtualPixels(image,0,offset,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    if (y == 0)
      waiting_code=(short) (*indexes);
    for (x=(ssize_t) (y == 0 ? 1 : 0); x < (ssize_t) image->columns; x++)
    {
      /*
        Probe hash table.
      */
      index=(IndexPacket) ((size_t) GetPixelIndex(indexes+x) & 0xff);
      p++;
      k=(ssize_t) (((size_t) index << (MaxGIFBits-8))+waiting_code);
      if (k >= MaxHashTable)
        k-=MaxHashTable;
      next_pixel=MagickFalse;
      displacement=1;
      if (hash_code[k] > 0)
        {
          if ((hash_prefix[k] == waiting_code) &&
              (hash_suffix[k] == (unsigned char) index))
            {
              waiting_code=hash_code[k];
              continue;
            }
          if (k != 0)
            displacement=MaxHashTable-k;
          for ( ; ; )
          {
            k-=displacement;
            if (k < 0)
              k+=MaxHashTable;
            if (hash_code[k] == 0)
              break;
            if ((hash_prefix[k] == waiting_code) &&
                (hash_suffix[k] == (unsigned char) index))
              {
                waiting_code=hash_code[k];
                next_pixel=MagickTrue;
                break;
              }
          }
          if (next_pixel != MagickFalse)
            continue;
        }
      GIFOutputCode((size_t) waiting_code);
      if (free_code < MaxGIFTable)
        {
          hash_code[k]=(short) free_code++;
          hash_prefix[k]=waiting_code;
          hash_suffix[k]=(unsigned char) index;
        }
      else
        {
          /*
            Fill the hash table with empty entries.
          */
          for (k=0; k < MaxHashTable; k++)
            hash_code[k]=0;
          /*
            Reset compressor and issue a clear code.
          */
          free_code=clear_code+2;
          GIFOutputCode(clear_code);
          number_bits=data_size;
          max_code=MaxCode(number_bits);
        }
      waiting_code=(short) index;
    }
    if (image_info->interlace == NoInterlace)
      offset++;
    else
      switch (pass)
      {
        case 0:
        default:
        {
          offset+=8;
          if (offset >= (ssize_t) image->rows)
            {
              pass++;
              offset=4;
            }
          break;
        }
        case 1:
        {
          offset+=8;
          if (offset >= (ssize_t) image->rows)
            {
              pass++;
              offset=2;
            }
          break;
        }
        case 2:
        {
          offset+=4;
          if (offset >= (ssize_t) image->rows)
            {
              pass++;
              offset=1;
            }
          break;
        }
        case 3:
        {
          offset+=2;
          break;
        }
      }
  }
  /*
    Flush out the buffered code.
  */
  GIFOutputCode((size_t) waiting_code);
  GIFOutputCode(end_of_information_code);
  if (bits > 0)
    {
      /*
        Add a character to current packet.
      */
      packet[length++]=(unsigned char) (datum & 0xff);
      if (length >= 254)
        {
          (void) WriteBlobByte(image,(unsigned char) length);
          (void) WriteBlob(image,length,packet);
          length=0;
        }
    }
  /*
    Flush accumulated data.
  */
  if (length > 0)
    {
      (void) WriteBlobByte(image,(unsigned char) length);
      (void) WriteBlob(image,length,packet);
    }
  /*
    Free encoder memory.
  */
  hash_suffix=(unsigned char *) RelinquishMagickMemory(hash_suffix);
  hash_prefix=(short *) RelinquishMagickMemory(hash_prefix);
  hash_code=(short *) RelinquishMagickMemory(hash_code);
  packet=(unsigned char *) RelinquishMagickMemory(packet);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s G I F                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsGIF() returns MagickTrue if the image format type, identified by the
%  magick string, is GIF.
%
%  The format of the IsGIF method is:
%
%      MagickBooleanType IsGIF(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsGIF(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"GIF8",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b B l o c k                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobBlock() reads data from the image file and returns it.  The
%  amount of data is determined by first reading a count byte.  The number
%  of bytes read is returned.
%
%  The format of the ReadBlobBlock method is:
%
%      size_t ReadBlobBlock(Image *image,unsigned char *data)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o data:  Specifies an area to place the information requested from
%      the file.
%
*/
static ssize_t ReadBlobBlock(Image *image,unsigned char *data)
{
  ssize_t
    count;

  unsigned char
    block_count;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(data != (unsigned char *) NULL);
  count=ReadBlob(image,1,&block_count);
  if (count != 1)
    return(0);
  count=ReadBlob(image,(size_t) block_count,data);
  if (count != (ssize_t) block_count)
    return(0);
  return(count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d G I F I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadGIFImage() reads a Compuserve Graphics image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadGIFImage method is:
%
%      Image *ReadGIFImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static MagickBooleanType PingGIFImage(Image *image)
{
  unsigned char
    buffer[256],
    length,
    data_size;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (ReadBlob(image,1,&data_size) != 1)
    ThrowBinaryException(CorruptImageError,"CorruptImage",image->filename);
  if (data_size > MaximumLZWBits)
    ThrowBinaryException(CorruptImageError,"CorruptImage",image->filename);
  if (ReadBlob(image,1,&length) != 1)
    ThrowBinaryException(CorruptImageError,"CorruptImage",image->filename);
  while (length != 0)
  {
    if (ReadBlob(image,length,buffer) != (ssize_t) length)
      ThrowBinaryException(CorruptImageError,"CorruptImage",image->filename);
    if (ReadBlob(image,1,&length) != 1)
      ThrowBinaryException(CorruptImageError,"CorruptImage",image->filename);
  }
  return(MagickTrue);
}

static Image *ReadGIFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define BitSet(byte,bit)  (((byte) & (bit)) == (bit))
#define LSBFirstOrder(x,y)  (((y) << 8) | (x))

  Image
    *image,
    *meta_image;

  int
    number_extensionss=0;

  MagickBooleanType
    status;

  RectangleInfo
    page;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    delay,
    dispose,
    global_colors,
    image_count,
    iterations,
    one;

  ssize_t
    count,
    opacity;

  unsigned char
    background,
    c,
    flag,
    *global_colormap,
    header[MaxTextExtent],
    magick[12];

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
    Determine if this a GIF file.
  */
  count=ReadBlob(image,6,magick);
  if ((count != 6) || ((LocaleNCompare((char *) magick,"GIF87",5) != 0) &&
      (LocaleNCompare((char *) magick,"GIF89",5) != 0)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  page.width=ReadBlobLSBShort(image);
  page.height=ReadBlobLSBShort(image);
  flag=(unsigned char) ReadBlobByte(image);
  background=(unsigned char) ReadBlobByte(image);
  c=(unsigned char) ReadBlobByte(image);  /* reserved */
  one=1;
  global_colors=one << (((size_t) flag & 0x07)+1);
  global_colormap=(unsigned char *) AcquireQuantumMemory((size_t)
    MagickMax(global_colors,256),3UL*sizeof(*global_colormap));
  if (global_colormap == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (BitSet((int) flag,0x80) != 0)
    count=ReadBlob(image,(size_t) (3*global_colors),global_colormap);
  delay=0;
  dispose=0;
  iterations=1;
  opacity=(-1);
  image_count=0;
  meta_image=AcquireImage(image_info);  /* metadata container */
  for ( ; ; )
  {
    count=ReadBlob(image,1,&c);
    if (count != 1)
      break;
    if (c == (unsigned char) ';')
      break;  /* terminator */
    if (c == (unsigned char) '!')
      {
        /*
          GIF Extension block.
        */

        count=ReadBlob(image,1,&c);
        if (count != 1)
          {
            global_colormap=(unsigned char *) RelinquishMagickMemory(
              global_colormap);
            ThrowReaderException(CorruptImageError,
              "UnableToReadExtensionBlock");
          }
        switch (c)
        {
          case 0xf9:
          {
            /*
              Read graphics control extension.
            */
            while (ReadBlobBlock(image,header) != 0) ;
            dispose=(size_t) (header[0] >> 2);
            delay=(size_t) ((header[2] << 8) | header[1]);
            if ((ssize_t) (header[0] & 0x01) == 0x01)
              opacity=(ssize_t) header[3];
            break;
          }
          case 0xfe:
          {
            char
              *comments;

            size_t
              length;

            /*
              Read comment extension.
            */
            comments=AcquireString((char *) NULL);
            for (length=0; ; length+=count)
            {
              count=(ssize_t) ReadBlobBlock(image,header);
              if (count == 0)
                break;
              header[count]='\0';
              (void) ConcatenateString(&comments,(const char *) header);
            }
            (void) SetImageProperty(meta_image,"comment",comments);
            comments=DestroyString(comments);
            break;
          }
          case 0xff:
          {
            MagickBooleanType
              loop;

            /*
              Read Netscape Loop extension.
            */
            loop=MagickFalse;
            if (ReadBlobBlock(image,header) != 0)
              loop=LocaleNCompare((char *) header,"NETSCAPE2.0",11) == 0 ?
                MagickTrue : MagickFalse;
            if (loop != MagickFalse)
              {
                while (ReadBlobBlock(image,header) != 0)
                  iterations=(size_t) ((header[2] << 8) | header[1]);
                break;
              }
            else
              {
                char
                  name[MaxTextExtent];

                int
                  block_length,
                  info_length,
                  reserved_length;

                MagickBooleanType
                  i8bim,
                  icc,
                  iptc,
                  magick;

                StringInfo
                  *profile;

                unsigned char
                  *info;

                /*
                  Store GIF application extension as a generic profile.
                */
                icc=LocaleNCompare((char *) header,"ICCRGBG1012",11) == 0 ?
                  MagickTrue : MagickFalse;
                magick=LocaleNCompare((char *) header,"ImageMagick",11) == 0 ?
                  MagickTrue : MagickFalse;
                i8bim=LocaleNCompare((char *) header,"MGK8BIM0000",11) == 0 ?
                  MagickTrue : MagickFalse;
                iptc=LocaleNCompare((char *) header,"MGKIPTC0000",11) == 0 ?
                  MagickTrue : MagickFalse;
                number_extensionss++;
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "    Reading GIF application extension");
                info=(unsigned char *) AcquireQuantumMemory(255UL,
                  sizeof(*info));
                reserved_length=255;
                for (info_length=0; ; )
                {
                  block_length=(int) ReadBlobBlock(image,&info[info_length]);
                  if (block_length == 0)
                    break;
                  info_length+=block_length;
                  if (info_length > (reserved_length-255))
                    {
                       reserved_length+=4096;
                       info=(unsigned char *) ResizeQuantumMemory(info,
                         (size_t) reserved_length,sizeof(*info));
                      if (info == (unsigned char *) NULL)
                         ThrowReaderException(ResourceLimitError,
                           "MemoryAllocationFailed");
                    }
                }
                profile=BlobToStringInfo(info,(size_t) info_length);
                if (profile == (StringInfo *) NULL)
                  ThrowReaderException(ResourceLimitError,
                    "MemoryAllocationFailed");
                if (i8bim != MagickFalse)
                  (void) CopyMagickString(name,"8bim",sizeof(name));
                else if (icc != MagickFalse)
                  (void) CopyMagickString(name,"icc",sizeof(name));
                else if (iptc != MagickFalse)
                  (void) CopyMagickString(name,"iptc",sizeof(name));
                else if (magick != MagickFalse)
                  {
                    (void) CopyMagickString(name,"magick",sizeof(name));
                    image->gamma=StringToDouble((char *) info+6,(char **) NULL);
                  }
                else
                  (void) FormatLocaleString(name,sizeof(name),"gif:%.11s",
                    header);
                info=(unsigned char *) RelinquishMagickMemory(info);
                if (magick == MagickFalse)
                  (void) SetImageProfile(meta_image,name,profile);
                profile=DestroyStringInfo(profile);
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      profile name=%s",name);
              }
            break;
          }
          default:
          {
            while (ReadBlobBlock(image,header) != 0) ;
            break;
          }
        }
      }
    if (c != (unsigned char) ',')
      continue;
    if (image_count != 0)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            global_colormap=(unsigned char *) RelinquishMagickMemory(
              global_colormap);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
      }
    image_count++;
    /*
      Read image attributes.
    */
    CloneImageProperties(image,meta_image);
    DestroyImageProperties(meta_image);
    CloneImageProfiles(image,meta_image);
    DestroyImageProfiles(meta_image);
    image->storage_class=PseudoClass;
    image->compression=LZWCompression;
    page.x=(ssize_t) ReadBlobLSBShort(image);
    page.y=(ssize_t) ReadBlobLSBShort(image);
    image->columns=ReadBlobLSBShort(image);
    image->rows=ReadBlobLSBShort(image);
    image->depth=8;
    flag=(unsigned char) ReadBlobByte(image);
    image->interlace=BitSet((int) flag,0x40) != 0 ? GIFInterlace :
      NoInterlace;
    image->colors=BitSet((int) flag,0x80) == 0 ? global_colors :
      one << ((size_t) (flag & 0x07)+1);
    if (opacity >= (ssize_t) image->colors)
      opacity=(-1);
    image->page.width=page.width;
    image->page.height=page.height;
    image->page.y=page.y;
    image->page.x=page.x;
    image->delay=delay;
    image->ticks_per_second=100;
    image->dispose=(DisposeType) dispose;
    image->iterations=iterations;
    image->matte=opacity >= 0 ? MagickTrue : MagickFalse;
    delay=0;
    dispose=0;
    iterations=1;
    if ((image->columns == 0) || (image->rows == 0))
      {
        global_colormap=(unsigned char *) RelinquishMagickMemory(
          global_colormap);
        ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
      }
    /*
      Inititialize colormap.
    */
    if (AcquireImageColormap(image,image->colors) == MagickFalse)
      {
        global_colormap=(unsigned char *) RelinquishMagickMemory(
          global_colormap);
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      }
    if (BitSet((int) flag,0x80) == 0)
      {
        /*
          Use global colormap.
        */
        p=global_colormap;
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          image->colormap[i].red=ScaleCharToQuantum(*p++);
          image->colormap[i].green=ScaleCharToQuantum(*p++);
          image->colormap[i].blue=ScaleCharToQuantum(*p++);
          if (i == opacity)
            {
              image->colormap[i].opacity=(Quantum) TransparentOpacity;
              image->transparent_color=image->colormap[opacity];
            }
        }
        image->background_color=image->colormap[MagickMin(background,
          image->colors-1)];
      }
    else
      {
        unsigned char
          *colormap;

        /*
          Read local colormap.
        */
        colormap=(unsigned char *) AcquireQuantumMemory(image->colors,3*
          sizeof(*colormap));
        if (colormap == (unsigned char *) NULL)
          {
            global_colormap=(unsigned char *) RelinquishMagickMemory(
              global_colormap);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        count=ReadBlob(image,(3*image->colors)*sizeof(*colormap),colormap);
        if (count != (ssize_t) (3*image->colors))
          {
            global_colormap=(unsigned char *) RelinquishMagickMemory(
              global_colormap);
            colormap=(unsigned char *) RelinquishMagickMemory(colormap);
            ThrowReaderException(CorruptImageError,
              "InsufficientImageDataInFile");
          }
        p=colormap;
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          image->colormap[i].red=ScaleCharToQuantum(*p++);
          image->colormap[i].green=ScaleCharToQuantum(*p++);
          image->colormap[i].blue=ScaleCharToQuantum(*p++);
          if (i == opacity)
            image->colormap[i].opacity=(Quantum) TransparentOpacity;
        }
        colormap=(unsigned char *) RelinquishMagickMemory(colormap);
      }
    if (image->gamma == 1.0)
      {
        for (i=0; i < (ssize_t) image->colors; i++)
          if (IsGrayPixel(image->colormap+i) == MagickFalse)
            break;
        (void) SetImageColorspace(image,i == (ssize_t) image->colors ?
          GRAYColorspace : RGBColorspace);
      }
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    /*
      Decode image.
    */
    if (image_info->ping != MagickFalse)
      status=PingGIFImage(image);
    else
      status=DecodeImage(image,opacity);
    if ((image_info->ping == MagickFalse) && (status == MagickFalse))
      {
        global_colormap=(unsigned char *) RelinquishMagickMemory(
          global_colormap);
        ThrowReaderException(CorruptImageError,"CorruptImage");
      }
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    opacity=(-1);
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) image->scene-
      1,image->scene);
    if (status == MagickFalse)
      break;
  }
  meta_image=DestroyImage(meta_image);
  global_colormap=(unsigned char *) RelinquishMagickMemory(global_colormap);
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r G I F I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterGIFImage() adds properties for the GIF image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterGIFImage method is:
%
%      size_t RegisterGIFImage(void)
%
*/
ModuleExport size_t RegisterGIFImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("GIF");
  entry->decoder=(DecodeImageHandler *) ReadGIFImage;
  entry->encoder=(EncodeImageHandler *) WriteGIFImage;
  entry->magick=(IsImageFormatHandler *) IsGIF;
  entry->description=ConstantString("CompuServe graphics interchange format");
  entry->mime_type=ConstantString("image/gif");
  entry->module=ConstantString("GIF");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("GIF87");
  entry->decoder=(DecodeImageHandler *) ReadGIFImage;
  entry->encoder=(EncodeImageHandler *) WriteGIFImage;
  entry->magick=(IsImageFormatHandler *) IsGIF;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("CompuServe graphics interchange format");
  entry->version=ConstantString("version 87a");
  entry->mime_type=ConstantString("image/gif");
  entry->module=ConstantString("GIF");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r G I F I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterGIFImage() removes format registrations made by the
%  GIF module from the list of supported formats.
%
%  The format of the UnregisterGIFImage method is:
%
%      UnregisterGIFImage(void)
%
*/
ModuleExport void UnregisterGIFImage(void)
{
  (void) UnregisterMagickInfo("GIF");
  (void) UnregisterMagickInfo("GIF87");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e G I F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteGIFImage() writes an image to a file in the Compuserve Graphics
%  image format.
%
%  The format of the WriteGIFImage method is:
%
%      MagickBooleanType WriteGIFImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteGIFImage(const ImageInfo *image_info,Image *image)
{
  int
    c;

  ImageInfo
    *write_info;

  InterlaceType
    interlace;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  RectangleInfo
    page;

  register ssize_t
    i;

  register unsigned char
    *q;

  size_t
    bits_per_pixel,
    delay,
    length,
    one;

  ssize_t
    j,
    opacity;

  unsigned char
    *colormap,
    *global_colormap;

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
    Allocate colormap.
  */
  global_colormap=(unsigned char *) AcquireQuantumMemory(768UL,
    sizeof(*global_colormap));
  colormap=(unsigned char *) AcquireQuantumMemory(768UL,sizeof(*colormap));
  if ((global_colormap == (unsigned char *) NULL) ||
      (colormap == (unsigned char *) NULL))
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  for (i=0; i < 768; i++)
    colormap[i]=(unsigned char) 0;
  /*
    Write GIF header.
  */
  write_info=CloneImageInfo(image_info);
  if (LocaleCompare(write_info->magick,"GIF87") != 0)
    (void) WriteBlob(image,6,(unsigned char *) "GIF89a");
  else
    {
      (void) WriteBlob(image,6,(unsigned char *) "GIF87a");
      write_info->adjoin=MagickFalse;
    }
  /*
    Determine image bounding box.
  */
  page.width=image->columns;
  if (image->page.width > page.width)
    page.width=image->page.width;
  page.height=image->rows;
  if (image->page.height > page.height)
    page.height=image->page.height;
  page.x=image->page.x;
  page.y=image->page.y;
  (void) WriteBlobLSBShort(image,(unsigned short) page.width);
  (void) WriteBlobLSBShort(image,(unsigned short) page.height);
  /*
    Write images to file.
  */
  interlace=write_info->interlace;
  if ((write_info->adjoin != MagickFalse) &&
      (GetNextImageInList(image) != (Image *) NULL))
    interlace=NoInterlace;
  scene=0;
  one=1;
  do
  {
    if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
      (void) TransformImageColorspace(image,sRGBColorspace);
    opacity=(-1);
    if (IsOpaqueImage(image,&image->exception) != MagickFalse)
      {
        if ((image->storage_class == DirectClass) || (image->colors > 256))
          (void) SetImageType(image,PaletteType);
      }
    else
      {
        MagickRealType
          alpha,
          beta;

        /*
          Identify transparent colormap index.
        */
        if ((image->storage_class == DirectClass) || (image->colors > 256))
          (void) SetImageType(image,PaletteBilevelMatteType);
        for (i=0; i < (ssize_t) image->colors; i++)
          if (image->colormap[i].opacity != OpaqueOpacity)
            {
              if (opacity < 0)
                {
                  opacity=i;
                  continue;
                }
              alpha=(MagickRealType) TransparentOpacity-(MagickRealType)
                image->colormap[i].opacity;
              beta=(MagickRealType) TransparentOpacity-(MagickRealType)
                image->colormap[opacity].opacity;
              if (alpha < beta)
                opacity=i;
            }
        if (opacity == -1)
          {
            (void) SetImageType(image,PaletteBilevelMatteType);
            for (i=0; i < (ssize_t) image->colors; i++)
              if (image->colormap[i].opacity != OpaqueOpacity)
                {
                  if (opacity < 0)
                    {
                      opacity=i;
                      continue;
                    }
                  alpha=(Quantum) TransparentOpacity-(MagickRealType)
                    image->colormap[i].opacity;
                  beta=(Quantum) TransparentOpacity-(MagickRealType)
                    image->colormap[opacity].opacity;
                  if (alpha < beta)
                    opacity=i;
                }
          }
        if (opacity >= 0)
          {
            image->colormap[opacity].red=image->transparent_color.red;
            image->colormap[opacity].green=image->transparent_color.green;
            image->colormap[opacity].blue=image->transparent_color.blue;
          }
      }
    if ((image->storage_class == DirectClass) || (image->colors > 256))
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    for (bits_per_pixel=1; bits_per_pixel < 8; bits_per_pixel++)
      if ((one << bits_per_pixel) >= image->colors)
        break;
    q=colormap;
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      *q++=ScaleQuantumToChar(image->colormap[i].red);
      *q++=ScaleQuantumToChar(image->colormap[i].green);
      *q++=ScaleQuantumToChar(image->colormap[i].blue);
    }
    for ( ; i < (ssize_t) (one << bits_per_pixel); i++)
    {
      *q++=(unsigned char) 0x0;
      *q++=(unsigned char) 0x0;
      *q++=(unsigned char) 0x0;
    }
    if ((GetPreviousImageInList(image) == (Image *) NULL) ||
        (write_info->adjoin == MagickFalse))
      {
        /*
          Write global colormap.
        */
        c=0x80;
        c|=(8-1) << 4;  /* color resolution */
        c|=(bits_per_pixel-1);   /* size of global colormap */
        (void) WriteBlobByte(image,(unsigned char) c);
        for (j=0; j < (ssize_t) image->colors; j++)
          if (IsColorEqual(&image->background_color,image->colormap+j))
            break;
        (void) WriteBlobByte(image,(unsigned char)
          (j == (ssize_t) image->colors ? 0 : j));  /* background color */
        (void) WriteBlobByte(image,(unsigned char) 0x00);  /* reserved */
        length=(size_t) (3*(one << bits_per_pixel));
        (void) WriteBlob(image,length,colormap);
        for (j=0; j < 768; j++)
          global_colormap[j]=colormap[j];
      }
    if (LocaleCompare(write_info->magick,"GIF87") != 0)
      {
        /*
          Write graphics control extension.
        */
        (void) WriteBlobByte(image,(unsigned char) 0x21);
        (void) WriteBlobByte(image,(unsigned char) 0xf9);
        (void) WriteBlobByte(image,(unsigned char) 0x04);
        c=image->dispose << 2;
        if (opacity >= 0)
          c|=0x01;
        (void) WriteBlobByte(image,(unsigned char) c);
        delay=(size_t) (100*image->delay/MagickMax((size_t)
          image->ticks_per_second,1));
        (void) WriteBlobLSBShort(image,(unsigned short) delay);
        (void) WriteBlobByte(image,(unsigned char) (opacity >= 0 ? opacity :
          0));
        (void) WriteBlobByte(image,(unsigned char) 0x00);
        if ((LocaleCompare(write_info->magick,"GIF87") != 0) &&
            (GetImageProperty(image,"comment") != (const char *) NULL))
          {
            const char
              *value;

            register const char
              *p;

            size_t
              count;

            /*
              Write comment extension.
            */
            (void) WriteBlobByte(image,(unsigned char) 0x21);
            (void) WriteBlobByte(image,(unsigned char) 0xfe);
            value=GetImageProperty(image,"comment");
            for (p=value; *p != '\0'; )
            {
              count=MagickMin(strlen(p),255);
              (void) WriteBlobByte(image,(unsigned char) count);
              for (i=0; i < (ssize_t) count; i++)
                (void) WriteBlobByte(image,(unsigned char) *p++);
            }
            (void) WriteBlobByte(image,(unsigned char) 0x00);
          }
        if ((GetPreviousImageInList(image) == (Image *) NULL) &&
            (GetNextImageInList(image) != (Image *) NULL) &&
            (image->iterations != 1))
          {
            /*
              Write Netscape Loop extension.
            */
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "  Writing GIF Extension %s","NETSCAPE2.0");
            (void) WriteBlobByte(image,(unsigned char) 0x21);
            (void) WriteBlobByte(image,(unsigned char) 0xff);
            (void) WriteBlobByte(image,(unsigned char) 0x0b);
            (void) WriteBlob(image,11,(unsigned char *) "NETSCAPE2.0");
            (void) WriteBlobByte(image,(unsigned char) 0x03);
            (void) WriteBlobByte(image,(unsigned char) 0x01);
            (void) WriteBlobLSBShort(image,(unsigned short) image->iterations);
            (void) WriteBlobByte(image,(unsigned char) 0x00);
          }
        if ((image->gamma != 1.0f/2.2f))
          {
            char
              attributes[MaxTextExtent];

            ssize_t
              length;

            /*
              Write ImageMagick extension.
            */
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "  Writing GIF Extension %s","ImageMagick");
            (void) WriteBlobByte(image,(unsigned char) 0x21);
            (void) WriteBlobByte(image,(unsigned char) 0xff);
            (void) WriteBlobByte(image,(unsigned char) 0x0b);
            (void) WriteBlob(image,11,(unsigned char *) "ImageMagick");
            length=FormatLocaleString(attributes,MaxTextExtent,"gamma=%g",
              image->gamma);
            (void) WriteBlobByte(image,(unsigned char) length);
            (void) WriteBlob(image,length,(unsigned char *) attributes);
            (void) WriteBlobByte(image,(unsigned char) 0x00);
          }
        ResetImageProfileIterator(image);
        for ( ; ; )
        {
          char
            *name;

          const StringInfo
            *profile;

          name=GetNextImageProfile(image);
          if (name == (const char *) NULL)
            break;
          profile=GetImageProfile(image,name);
          if (profile != (StringInfo *) NULL)
          {
            if ((LocaleCompare(name,"ICC") == 0) ||
                (LocaleCompare(name,"ICM") == 0) ||
                (LocaleCompare(name,"IPTC") == 0) ||
                (LocaleCompare(name,"8BIM") == 0) ||
                (LocaleNCompare(name,"gif:",4) == 0))
            {
               size_t
                 length;

               ssize_t
                 offset;

               unsigned char
                 *datum;

               datum=GetStringInfoDatum(profile);
               length=GetStringInfoLength(profile);
               (void) WriteBlobByte(image,(unsigned char) 0x21);
               (void) WriteBlobByte(image,(unsigned char) 0xff);
               (void) WriteBlobByte(image,(unsigned char) 0x0b);
               if ((LocaleCompare(name,"ICC") == 0) ||
                   (LocaleCompare(name,"ICM") == 0))
                 {
                   /*
                     Write ICC extension.
                   */
                   (void) WriteBlob(image,11,(unsigned char *) "ICCRGBG1012");
                   (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                       "  Writing GIF Extension %s","ICCRGBG1012");
                 }
               else
                 if ((LocaleCompare(name,"IPTC") == 0))
                   {
                     /*
                       Write IPTC extension.
                     */
                     (void) WriteBlob(image,11,(unsigned char *) "MGKIPTC0000");
                     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                         "  Writing GIF Extension %s","MGKIPTC0000");
                   }
                 else
                   if ((LocaleCompare(name,"8BIM") == 0))
                     {
                       /*
                         Write 8BIM extension.
                       */
                        (void) WriteBlob(image,11,(unsigned char *)
                          "MGK8BIM0000");
                        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "  Writing GIF Extension %s","MGK8BIM0000");
                     }
                   else
                     {
                       char
                         extension[MaxTextExtent];

                       /*
                         Write generic extension.
                       */
                       (void) CopyMagickString(extension,name+4,
                         sizeof(extension));
                       (void) WriteBlob(image,11,(unsigned char *) extension);
                       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "  Writing GIF Extension %s",name);
                     }
               offset=0;
               while ((ssize_t) length > offset)
               {
                 size_t
                   block_length;

                 if ((length-offset) < 255)
                   block_length=length-offset;
                 else
                   block_length=255;
                 (void) WriteBlobByte(image,(unsigned char) block_length);
                 (void) WriteBlob(image,(size_t) block_length,datum+offset);
                 offset+=(ssize_t) block_length;
               }
               (void) WriteBlobByte(image,(unsigned char) 0x00);
            }
          }
        }
      }
    (void) WriteBlobByte(image,',');  /* image separator */
    /*
      Write the image header.
    */
    page.x=image->page.x;
    page.y=image->page.y;
    if ((image->page.width != 0) && (image->page.height != 0))
      page=image->page;
    (void) WriteBlobLSBShort(image,(unsigned short) (page.x < 0 ? 0 : page.x));
    (void) WriteBlobLSBShort(image,(unsigned short) (page.y < 0 ? 0 : page.y));
    (void) WriteBlobLSBShort(image,(unsigned short) image->columns);
    (void) WriteBlobLSBShort(image,(unsigned short) image->rows);
    c=0x00;
    if (interlace != NoInterlace)
      c|=0x40;  /* pixel data is interlaced */
    for (j=0; j < (ssize_t) (3*image->colors); j++)
      if (colormap[j] != global_colormap[j])
        break;
    if (j == (ssize_t) (3*image->colors))
      (void) WriteBlobByte(image,(unsigned char) c);
    else
      {
        c|=0x80;
        c|=(bits_per_pixel-1);   /* size of local colormap */
        (void) WriteBlobByte(image,(unsigned char) c);
        length=(size_t) (3*(one << bits_per_pixel));
        (void) WriteBlob(image,length,colormap);
      }
    /*
      Write the image data.
    */
    c=(int) MagickMax(bits_per_pixel,2);
    (void) WriteBlobByte(image,(unsigned char) c);
    status=EncodeImage(write_info,image,(size_t) MagickMax(bits_per_pixel,2)+1);
    if (status == MagickFalse)
      {
        global_colormap=(unsigned char *) RelinquishMagickMemory(
          global_colormap);
        colormap=(unsigned char *) RelinquishMagickMemory(colormap);
        write_info=DestroyImageInfo(write_info);
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      }
    (void) WriteBlobByte(image,(unsigned char) 0x00);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    scene++;
    status=SetImageProgress(image,SaveImagesTag,scene,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (write_info->adjoin != MagickFalse);
  (void) WriteBlobByte(image,';'); /* terminator */
  global_colormap=(unsigned char *) RelinquishMagickMemory(global_colormap);
  colormap=(unsigned char *) RelinquishMagickMemory(colormap);
  write_info=DestroyImageInfo(write_info);
  (void) CloseBlob(image);
  return(MagickTrue);
}

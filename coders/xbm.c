/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            X   X  BBBB   M   M                              %
%                             X X   B   B  MM MM                              %
%                              X    BBBB   M M M                              %
%                             X X   B   B  M   M                              %
%                            X   X  BBBB   M   M                              %
%                                                                             %
%                                                                             %
%                  Read/Write X Windows System Bitmap Format                  %
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
#include "magick/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteXBMImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s X B M                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsXBM() returns MagickTrue if the image format type, identified by the
%  magick string, is XBM.
%
%  The format of the IsXBM method is:
%
%      MagickBooleanType IsXBM(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsXBM(const unsigned char *magick,const size_t length)
{
  if (length < 7)
    return(MagickFalse);
  if (memcmp(magick,"#define",7) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X B M I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadXBMImage() reads an X11 bitmap image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadXBMImage method is:
%
%      Image *ReadXBMImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static int XBMInteger(Image *image,short int *hex_digits)
{
  int
    c,
    flag,
    value;

  value=0;
  flag=0;
  for ( ; ; )
  {
    c=ReadBlobByte(image);
    if (c == EOF)
      {
        value=(-1);
        break;
      }
    c&=0xff;
    if (isxdigit(c) != MagickFalse)
      {
        value=(int) ((size_t) value << 4)+hex_digits[c];
        flag++;
        continue;
      }
    if ((hex_digits[c]) < 0 && (flag != 0))
      break;
  }
  return(value);
}

static Image *ReadXBMImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    buffer[MaxTextExtent],
    name[MaxTextExtent];

  Image
    *image;

  MagickBooleanType
    status;

  register IndexPacket
    *indexes;

  register ssize_t
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  short int
    hex_digits[256];

  size_t
    bit,
    byte,
    bytes_per_line,
    length,
    padding,
    value,
    version;

  ssize_t
    y;

  unsigned char
    *data;

  unsigned long
    height,
    width;

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
    Read X bitmap header.
  */
  width=0;
  height=0;
  while (ReadBlobString(image,buffer) != (char *) NULL)
    if (sscanf(buffer,"#define %s %lu",name,&width) == 2)
      if ((strlen(name) >= 6) &&
          (LocaleCompare(name+strlen(name)-6,"_width") == 0))
        break;
  while (ReadBlobString(image,buffer) != (char *) NULL)
    if (sscanf(buffer,"#define %s %lu",name,&height) == 2)
      if ((strlen(name) >= 7) &&
          (LocaleCompare(name+strlen(name)-7,"_height") == 0))
        break;
  image->columns=width;
  image->rows=height;
  image->depth=8;
  image->storage_class=PseudoClass;
  image->colors=2;
  /*
    Scan until hex digits.
  */
  version=11;
  while (ReadBlobString(image,buffer) != (char *) NULL)
  {
    if (sscanf(buffer,"static short %s = {",name) == 1)
      version=10;
    else
      if (sscanf(buffer,"static unsigned char %s = {",name) == 1)
        version=11;
      else
        if (sscanf(buffer,"static char %s = {",name) == 1)
          version=11;
        else
          continue;
    p=(unsigned char *) strrchr(name,'_');
    if (p == (unsigned char *) NULL)
      p=(unsigned char *) name;
    else
      p++;
    if (LocaleCompare("bits[]",(char *) p) == 0)
      break;
  }
  if ((image->columns == 0) || (image->rows == 0) ||
      (EOFBlob(image) != MagickFalse))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Initialize image structure.
  */
  if (AcquireImageColormap(image,image->colors) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize colormap.
  */
  image->colormap[0].red=QuantumRange;
  image->colormap[0].green=QuantumRange;
  image->colormap[0].blue=QuantumRange;
  image->colormap[1].red=(Quantum) 0;
  image->colormap[1].green=(Quantum) 0;
  image->colormap[1].blue=(Quantum) 0;
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  /*
    Initialize hex values.
  */
  hex_digits[(int) '0']=0;
  hex_digits[(int) '1']=1;
  hex_digits[(int) '2']=2;
  hex_digits[(int) '3']=3;
  hex_digits[(int) '4']=4;
  hex_digits[(int) '5']=5;
  hex_digits[(int) '6']=6;
  hex_digits[(int) '7']=7;
  hex_digits[(int) '8']=8;
  hex_digits[(int) '9']=9;
  hex_digits[(int) 'A']=10;
  hex_digits[(int) 'B']=11;
  hex_digits[(int) 'C']=12;
  hex_digits[(int) 'D']=13;
  hex_digits[(int) 'E']=14;
  hex_digits[(int) 'F']=15;
  hex_digits[(int) 'a']=10;
  hex_digits[(int) 'b']=11;
  hex_digits[(int) 'c']=12;
  hex_digits[(int) 'd']=13;
  hex_digits[(int) 'e']=14;
  hex_digits[(int) 'f']=15;
  hex_digits[(int) 'x']=0;
  hex_digits[(int) ' ']=(-1);
  hex_digits[(int) ',']=(-1);
  hex_digits[(int) '}']=(-1);
  hex_digits[(int) '\n']=(-1);
  hex_digits[(int) '\t']=(-1);
  /*
    Read hex image data.
  */
  padding=0;
  if (((image->columns % 16) != 0) && ((image->columns % 16) < 9) &&
      (version == 10))
    padding=1;
  bytes_per_line=(image->columns+7)/8+padding;
  length=(size_t) image->rows;
  data=(unsigned char *) AcquireQuantumMemory(length,bytes_per_line*
    sizeof(*data));
  if (data == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  p=data;
  if (version == 10)
    for (i=0; i < (ssize_t) (bytes_per_line*image->rows); (i+=2))
    {
      value=(size_t) XBMInteger(image,hex_digits);
      *p++=(unsigned char) value;
      if ((padding == 0) || (((i+2) % bytes_per_line) != 0))
        *p++=(unsigned char) (value >> 8);
    }
  else
    for (i=0; i < (ssize_t) (bytes_per_line*image->rows); i++)
    {
      value=(size_t) XBMInteger(image,hex_digits);
      *p++=(unsigned char) value;
    }
  /*
    Convert X bitmap image to pixel packets.
  */
  p=data;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    bit=0;
    byte=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (bit == 0)
        byte=(size_t) (*p++);
      SetPixelIndex(indexes+x,(byte & 0x01) != 0 ? 0x01 : 0x00);
      bit++;
      byte>>=1;
      if (bit == 8)
        bit=0;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  data=(unsigned char *) RelinquishMagickMemory(data);
  (void) SyncImage(image);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r X B M I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterXBMImage() adds attributes for the XBM image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterXBMImage method is:
%
%      size_t RegisterXBMImage(void)
%
*/
ModuleExport size_t RegisterXBMImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("XBM");
  entry->decoder=(DecodeImageHandler *) ReadXBMImage;
  entry->encoder=(EncodeImageHandler *) WriteXBMImage;
  entry->magick=(IsImageFormatHandler *) IsXBM;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString(
    "X Windows system bitmap (black and white)");
  entry->module=ConstantString("XBM");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r X B M I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterXBMImage() removes format registrations made by the
%  XBM module from the list of supported formats.
%
%  The format of the UnregisterXBMImage method is:
%
%      UnregisterXBMImage(void)
%
*/
ModuleExport void UnregisterXBMImage(void)
{
  (void) UnregisterMagickInfo("XBM");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e X B M I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure WriteXBMImage() writes an image to a file in the X bitmap format.
%
%  The format of the WriteXBMImage method is:
%
%      MagickBooleanType WriteXBMImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%
*/
static MagickBooleanType WriteXBMImage(const ImageInfo *image_info,Image *image)
{
  char
    basename[MaxTextExtent],
    buffer[MaxTextExtent];

  MagickBooleanType
    status;

  register const PixelPacket
    *p;

  register ssize_t
    x;

  size_t
    bit,
    byte;

  ssize_t
    count,
    y;

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
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace);
  /*
    Write X bitmap header.
  */
  GetPathComponent(image->filename,BasePath,basename);
  (void) FormatLocaleString(buffer,MaxTextExtent,"#define %s_width %.20g\n",
    basename,(double) image->columns);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  (void) FormatLocaleString(buffer,MaxTextExtent,"#define %s_height %.20g\n",
    basename,(double) image->rows);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  (void) FormatLocaleString(buffer,MaxTextExtent,
    "static char %s_bits[] = {\n",basename);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  (void) CopyMagickString(buffer," ",MaxTextExtent);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  /*
    Convert MIFF to X bitmap pixels.
  */
  (void) SetImageType(image,BilevelType);
  bit=0;
  byte=0;
  count=0;
  x=0;
  y=0;
  (void) CopyMagickString(buffer," ",MaxTextExtent);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      byte>>=1;
      if (GetPixelLuma(image,p) < (QuantumRange/2.0))
        byte|=0x80;
      bit++;
      if (bit == 8)
        {
          /*
            Write a bitmap byte to the image file.
          */
          (void) FormatLocaleString(buffer,MaxTextExtent,"0x%02X, ",
            (unsigned int) (byte & 0xff));
          (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
          count++;
          if (count == 12)
            {
              (void) CopyMagickString(buffer,"\n  ",MaxTextExtent);
              (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
              count=0;
            };
          bit=0;
          byte=0;
        }
        p++;
      }
    if (bit != 0)
      {
        /*
          Write a bitmap byte to the image file.
        */
        byte>>=(8-bit);
        (void) FormatLocaleString(buffer,MaxTextExtent,"0x%02X, ",
          (unsigned int) (byte & 0xff));
        (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
        count++;
        if (count == 12)
          {
            (void) CopyMagickString(buffer,"\n  ",MaxTextExtent);
            (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
            count=0;
          };
        bit=0;
        byte=0;
      };
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  (void) CopyMagickString(buffer,"};\n",MaxTextExtent);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  (void) CloseBlob(image);
  return(MagickTrue);
}

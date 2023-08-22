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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteXBMImage(const ImageInfo *,Image *,ExceptionInfo *);

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
    c;

  unsigned int
    value;

  /*
    Skip any leading whitespace.
  */
  do
  {
    c=ReadBlobByte(image);
    if (c == EOF)
      return(-1);
  } while ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'));
  /*
    Evaluate number.
  */
  value=0;
  do
  {
    if (value <= (unsigned int) (INT_MAX/16))
      {
        value*=16;
        c&=0xff;
        if (value <= (unsigned int) ((INT_MAX-1)-hex_digits[c]))
          value+=(unsigned int) hex_digits[c];
      }
    c=ReadBlobByte(image);
    if (c == EOF)
      return(-1);
  } while (hex_digits[c & 0xff] >= 0);
  return((int) value);
}

static Image *ReadXBMImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent],
    name[MagickPathExtent];

  Image
    *image;

  int
    c;

  long
    height,
    width;

  MagickBooleanType
    status;

  ssize_t
    i,
    x;

  Quantum
    *q;

  unsigned char
    *p;

  short int
    hex_digits[256];

  ssize_t
    y;

  unsigned char
    *data;

  unsigned int
    bit,
    byte,
    bytes_per_line,
    length,
    padding,
    version;

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
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read X bitmap header.
  */
  width=0;
  height=0;
  *name='\0';
  while (ReadBlobString(image,buffer) != (char *) NULL)
    if (sscanf(buffer,"#define %1024s %ld",name,&width) == 2)
      if ((strlen(name) >= 6) &&
          (LocaleCompare(name+strlen(name)-6,"_width") == 0))
        break;
  while (ReadBlobString(image,buffer) != (char *) NULL)
    if (sscanf(buffer,"#define %1024s %ld",name,&height) == 2)
      if ((strlen(name) >= 7) &&
          (LocaleCompare(name+strlen(name)-7,"_height") == 0))
        break;
  if ((width <= 0) || (height <= 0) || (EOFBlob(image) != MagickFalse))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->columns=(size_t) width;
  image->rows=(size_t) height;
  image->depth=8;
  image->storage_class=PseudoClass;
  image->colors=2;
  /*
    Scan until hex digits.
  */
  version=11;
  while (ReadBlobString(image,buffer) != (char *) NULL)
  {
    if (sscanf(buffer,"static short %1024s = {",name) == 1)
      version=10;
    else
      if (sscanf(buffer,"static unsigned char %1024s = {",name) == 1)
        version=11;
      else
        if (sscanf(buffer,"static char %1024s = {",name) == 1)
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
  /*
    Initialize image structure.
  */
  if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize colormap.
  */
  image->colormap[0].red=(MagickRealType) QuantumRange;
  image->colormap[0].green=(MagickRealType) QuantumRange;
  image->colormap[0].blue=(MagickRealType) QuantumRange;
  image->colormap[1].red=0.0;
  image->colormap[1].green=0.0;
  image->colormap[1].blue=0.0;
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Initialize hex values.
  */
  for (i=0; i < (ssize_t) (sizeof(hex_digits)/sizeof(*hex_digits)); i++)
    hex_digits[i]=(-1);
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
  bytes_per_line=(unsigned int) (image->columns+7)/8+padding;
  length=(unsigned int) image->rows;
  data=(unsigned char *) AcquireQuantumMemory(length,bytes_per_line*
    sizeof(*data));
  if (data == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  p=data;
  if (version == 10)
    for (i=0; i < (ssize_t) (bytes_per_line*image->rows); (i+=2))
    {
      c=XBMInteger(image,hex_digits);
      if (c < 0)
        {
          data=(unsigned char *) RelinquishMagickMemory(data);
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        }
      *p++=(unsigned char) c;
      if ((padding == 0) || (((i+2) % bytes_per_line) != 0))
        *p++=(unsigned char) (c >> 8);
    }
  else
    for (i=0; i < (ssize_t) (bytes_per_line*image->rows); i++)
    {
      c=XBMInteger(image,hex_digits);
      if (c < 0)
        {
          data=(unsigned char *) RelinquishMagickMemory(data);
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        }
      *p++=(unsigned char) c;
    }
  if (EOFBlob(image) != MagickFalse)
    {
      data=(unsigned char *) RelinquishMagickMemory(data);
      ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
    }
  /*
    Convert X bitmap image to pixel packets.
  */
  p=data;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    bit=0;
    byte=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (bit == 0)
        byte=(unsigned int) (*p++);
      SetPixelIndex(image,(Quantum) ((byte & 0x01) != 0 ? 0x01 : 0x00),q);
      bit++;
      byte>>=1;
      if (bit == 8)
        bit=0;
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  data=(unsigned char *) RelinquishMagickMemory(data);
  (void) SyncImage(image,exception);
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

  entry=AcquireMagickInfo("XBM","XBM",
    "X Windows system bitmap (black and white)");
  entry->decoder=(DecodeImageHandler *) ReadXBMImage;
  entry->encoder=(EncodeImageHandler *) WriteXBMImage;
  entry->magick=(IsImageFormatHandler *) IsXBM;
  entry->flags^=CoderAdjoinFlag;
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
%  WriteXBMImage() writes an image to a file in the X bitmap format.
%
%  The format of the WriteXBMImage method is:
%
%      MagickBooleanType WriteXBMImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteXBMImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    basename[MagickPathExtent],
    buffer[MagickPathExtent];

  MagickBooleanType
    status;

  const Quantum
    *p;

  ssize_t
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
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  /*
    Write X bitmap header.
  */
  GetPathComponent(image->filename,BasePath,basename);
  (void) FormatLocaleString(buffer,MagickPathExtent,"#define %s_width %.20g\n",
    basename,(double) image->columns);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  (void) FormatLocaleString(buffer,MagickPathExtent,"#define %s_height %.20g\n",
    basename,(double) image->rows);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  (void) FormatLocaleString(buffer,MagickPathExtent,
    "static char %s_bits[] = {\n",basename);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  (void) CopyMagickString(buffer," ",MagickPathExtent);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  /*
    Convert MIFF to X bitmap pixels.
  */
  (void) SetImageType(image,BilevelType,exception);
  bit=0;
  byte=0;
  count=0;
  x=0;
  y=0;
  (void) CopyMagickString(buffer," ",MagickPathExtent);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      byte>>=1;
      if (GetPixelLuma(image,p) < ((double) QuantumRange/2))
        byte|=0x80;
      bit++;
      if (bit == 8)
        {
          /*
            Write a bitmap byte to the image file.
          */
          (void) FormatLocaleString(buffer,MagickPathExtent,"0x%02X, ",
            (unsigned int) (byte & 0xff));
          (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
          count++;
          if (count == 12)
            {
              (void) CopyMagickString(buffer,"\n  ",MagickPathExtent);
              (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
              count=0;
            };
          bit=0;
          byte=0;
        }
      p+=GetPixelChannels(image);
    }
    if (bit != 0)
      {
        /*
          Write a bitmap byte to the image file.
        */
        byte>>=(8-bit);
        (void) FormatLocaleString(buffer,MagickPathExtent,"0x%02X, ",
          (unsigned int) (byte & 0xff));
        (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
        count++;
        if (count == 12)
          {
            (void) CopyMagickString(buffer,"\n  ",MagickPathExtent);
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
  (void) CopyMagickString(buffer,"};\n",MagickPathExtent);
  (void) WriteBlob(image,strlen(buffer),(unsigned char *) buffer);
  (void) CloseBlob(image);
  return(MagickTrue);
}

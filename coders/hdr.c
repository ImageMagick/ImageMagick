/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            H   H  DDDD   RRRR                               %
%                            H   H  D   D  R   R                              %
%                            HHHHH  D   D  RRRR                               %
%                            H   H  D   D  R R                                %
%                            H   H  DDDD   R  R                               %
%                                                                             %
%                                                                             %
%                   Read/Write Radiance RGBE Image Format                     %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/colorspace.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/module.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s H D R                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsHDR() returns MagickTrue if the image format type, identified by the
%  magick string, is Radiance RGBE image format.
%
%  The format of the IsHDR method is:
%
%      MagickBooleanType IsHDR(const unsigned char *magick,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsHDR(const unsigned char *magick,
  const size_t length)
{
  if (length < 10)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"#?RADIANCE",10) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d H D R I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadHDRImage() reads the Radiance RGBE image format and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadHDRImage method is:
%
%      Image *ReadHDRImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadHDRImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    format[MaxTextExtent],
    keyword[MaxTextExtent],
    tag[MaxTextExtent],
    value[MaxTextExtent];

  double
    gamma;

  Image
    *image;

  int
    c;

  MagickBooleanType
    status,
    value_expected;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  register ssize_t
    i,
    x;

  ssize_t
    count,
    y;

  unsigned char
    *end,
    pixel[4],
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
    Decode image header.
  */
  image->columns=0;
  image->rows=0;
  *format='\0';
  c=ReadBlobByte(image);
  if (c == EOF)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  while (isgraph(c) && (image->columns == 0) && (image->rows == 0))
  {
    if (isalnum(c) == MagickFalse)
      c=ReadBlobByte(image);
    else
      {
        register char
          *p;

        /*
          Determine a keyword and its value.
        */
        p=keyword;
        do
        {
          if ((size_t) (p-keyword) < (MaxTextExtent-1))
            *p++=c;
          c=ReadBlobByte(image);
        } while (isalnum(c) || (c == '_'));
        *p='\0';
        value_expected=MagickFalse;
        while ((isspace((int) ((unsigned char) c)) != 0) || (c == '='))
        {
          if (c == '=')
            value_expected=MagickTrue;
          c=ReadBlobByte(image);
        }
        if (LocaleCompare(keyword,"Y") == 0)
          value_expected=MagickTrue;
        if (value_expected == MagickFalse)
          continue;
        p=value;
        while ((c != '\n') && (c != '\0'))
        {
          if ((size_t) (p-value) < (MaxTextExtent-1))
            *p++=c;
          c=ReadBlobByte(image);
        }
        *p='\0';
        /*
          Assign a value to the specified keyword.
        */
        switch (*keyword)
        {
          case 'F':
          case 'f':
          {
            if (LocaleCompare(keyword,"format") == 0)
              {
                (void) CopyMagickString(format,value,MaxTextExtent);
                break;
              }
            (void) FormatMagickString(tag,MaxTextExtent,"hdr:%s",keyword);
            (void) SetImageProperty(image,tag,value);
            break;
          }
          case 'G':
          case 'g':
          {
            if (LocaleCompare(keyword,"gamma") == 0)
              {
                image->gamma=StringToDouble(value);
                break;
              }
            (void) FormatMagickString(tag,MaxTextExtent,"hdr:%s",keyword);
            (void) SetImageProperty(image,tag,value);
            break;
          }
          case 'P':
          case 'p':
          {
            if (LocaleCompare(keyword,"primaries") == 0)
              {
                float
                  chromaticity[6],
                  white_point[2];

                (void) sscanf(value,"%g %g %g %g %g %g %g %g",&chromaticity[0],
                  &chromaticity[1],&chromaticity[2],&chromaticity[3],
                  &chromaticity[4],&chromaticity[5],&white_point[0],
                  &white_point[1]);
                image->chromaticity.red_primary.x=chromaticity[0];
                image->chromaticity.red_primary.y=chromaticity[1];
                image->chromaticity.green_primary.x=chromaticity[2];
                image->chromaticity.green_primary.y=chromaticity[3];
                image->chromaticity.blue_primary.x=chromaticity[4];
                image->chromaticity.blue_primary.y=chromaticity[5];
                image->chromaticity.white_point.x=white_point[0],
                image->chromaticity.white_point.y=white_point[1];
                break;
              }
            (void) FormatMagickString(tag,MaxTextExtent,"hdr:%s",keyword);
            (void) SetImageProperty(image,tag,value);
            break;
          }
          case 'Y':
          case 'y':
          {
            if (strcmp(keyword,"Y") == 0)
              {
                int
                  height,
                  width;

                (void) sscanf(value,"%d +X %d",&height,&width);
                image->columns=(size_t) width;
                image->rows=(size_t) height;
                break;
              }
            (void) FormatMagickString(tag,MaxTextExtent,"hdr:%s",keyword);
            (void) SetImageProperty(image,tag,value);
            break;
          }
          default:
          {
            (void) FormatMagickString(tag,MaxTextExtent,"hdr:%s",keyword);
            (void) SetImageProperty(image,tag,value);
            break;
          }
        }
      }
    if ((image->columns == 0) && (image->rows == 0))
      while (isspace((int) ((unsigned char) c)) != 0)
        c=ReadBlobByte(image);
  }
  if (LocaleCompare(format,"32-bit_rle_rgbe") != 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  /*
    Read RGBE pixels.
  */
  pixels=(unsigned char *) AcquireQuantumMemory(image->columns,
    4*sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    count=ReadBlob(image,4*sizeof(*pixel),pixel);
    if (count != 4)
      break;
    if ((size_t) ((((size_t) pixel[2]) << 8) | pixel[3]) != image->columns) 
      break;
    p=pixels;
    for (i=0; i < 4; i++)
    {
      end=&pixels[(i+1)*image->columns];
      while (p < end)
      {
        count=ReadBlob(image,2*sizeof(*pixel),pixel);
        if (count < 1)
          break;
        if (pixel[0] > 128)
          {
            count=(ssize_t) pixel[0]-128;
            if ((count == 0) || (count > (ssize_t) (end-p)))
              break;
            while (count-- > 0)
              *p++=pixel[1];
          }
        else
          {
            count=(ssize_t) pixel[0];
            if ((count == 0) || (count > (ssize_t) (end-p)))
              break;
            *p++=pixel[1];
            if (--count > 0)
              {
                count=ReadBlob(image,(size_t) count*sizeof(*p),p);
                if (count < 1)
                  break;
                p+=count;
              }
          }
      }
    }
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      pixel[0]=pixels[x];
      pixel[1]=pixels[x+image->columns];
      pixel[2]=pixels[x+2*image->columns];
      pixel[3]=pixels[x+3*image->columns];
      q->red=0;
      q->green=0;
      q->blue=0;
      if (pixel[3] != 0)
        {
          gamma=pow(2.0,pixel[3]-(128.0+8.0));
          q->red=ClampToQuantum(QuantumRange*gamma*pixel[0]);
          q->green=ClampToQuantum(QuantumRange*gamma*pixel[1]);
          q->blue=ClampToQuantum(QuantumRange*gamma*pixel[2]);
        }
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r H D R I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterHDRImage() adds attributes for the Radiance RGBE image format to the
%  list of supported formats.  The attributes include the image format tag, a
%  method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterHDRImage method is:
%
%      size_t RegisterHDRImage(void)
%
*/
ModuleExport size_t RegisterHDRImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("HDR");
  entry->decoder=(DecodeImageHandler *) ReadHDRImage;
  entry->description=ConstantString("Radiance RGBE image format");
  entry->module=ConstantString("HDR");
  entry->magick=(IsImageFormatHandler *) IsHDR;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r H D R I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterHDRImage() removes format registrations made by the
%  HDR module from the list of supported formats.
%
%  The format of the UnregisterHDRImage method is:
%
%      UnregisterHDRImage(void)
%
*/
ModuleExport void UnregisterHDRImage(void)
{
  (void) UnregisterMagickInfo("HDR");
}

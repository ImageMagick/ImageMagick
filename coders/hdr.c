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
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteHDRImage(const ImageInfo *,Image *,ExceptionInfo *);

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
  if (LocaleNCompare((const char *) magick,"#?RGBE",6) == 0)
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
    format[MagickPathExtent],
    keyword[MagickPathExtent],
    tag[MagickPathExtent],
    value[MagickPathExtent];

  double
    gamma;

  Image
    *image;

  int
    c;

  MagickBooleanType
    status,
    value_expected;

  register Quantum
    *q;

  register ssize_t
    i,
    x;

  register unsigned char
    *p;

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
    if (c == (int) '#')
      {
        char
          *comment;

        register char
          *p;

        size_t
          length;

        /*
          Read comment-- any text between # and end-of-line.
        */
        length=MagickPathExtent;
        comment=AcquireString((char *) NULL);
        for (p=comment; comment != (char *) NULL; p++)
        {
          c=ReadBlobByte(image);
          if ((c == EOF) || (c == (int) '\n'))
            break;
          if ((size_t) (p-comment+1) >= length)
            {
              *p='\0';
              length<<=1;
              comment=(char *) ResizeQuantumMemory(comment,length+
                MagickPathExtent,sizeof(*comment));
              if (comment == (char *) NULL)
                break;
              p=comment+strlen(comment);
            }
          *p=(char) c;
        }
        if (comment == (char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        *p='\0';
        (void) SetImageProperty(image,"comment",comment,exception);
        comment=DestroyString(comment);
        c=ReadBlobByte(image);
      }
    else
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
            if ((size_t) (p-keyword) < (MagickPathExtent-1))
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
          while ((c != '\n') && (c != '\0') && (c != EOF))
          {
            if ((size_t) (p-value) < (MagickPathExtent-1))
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
                  (void) CopyMagickString(format,value,MagickPathExtent);
                  break;
                }
              (void) FormatLocaleString(tag,MagickPathExtent,"hdr:%s",keyword);
              (void) SetImageProperty(image,tag,value,exception);
              break;
            }
            case 'G':
            case 'g':
            {
              if (LocaleCompare(keyword,"gamma") == 0)
                {
                  image->gamma=StringToDouble(value,(char **) NULL);
                  break;
                }
              (void) FormatLocaleString(tag,MagickPathExtent,"hdr:%s",keyword);
              (void) SetImageProperty(image,tag,value,exception);
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

                  int
                    count;

                  count=sscanf(value,"%g %g %g %g %g %g %g %g",&chromaticity[0],
                    &chromaticity[1],&chromaticity[2],&chromaticity[3],
                    &chromaticity[4],&chromaticity[5],&white_point[0],
                    &white_point[1]);
                  if (count == 8)
                    {
                      image->chromaticity.red_primary.x=chromaticity[0];
                      image->chromaticity.red_primary.y=chromaticity[1];
                      image->chromaticity.green_primary.x=chromaticity[2];
                      image->chromaticity.green_primary.y=chromaticity[3];
                      image->chromaticity.blue_primary.x=chromaticity[4];
                      image->chromaticity.blue_primary.y=chromaticity[5];
                      image->chromaticity.white_point.x=white_point[0],
                      image->chromaticity.white_point.y=white_point[1];
                    }
                  break;
                }
              (void) FormatLocaleString(tag,MagickPathExtent,"hdr:%s",keyword);
              (void) SetImageProperty(image,tag,value,exception);
              break;
            }
            case 'Y':
            case 'y':
            {
              char
                target[] = "Y";

              if (strcmp(keyword,target) == 0)
                {
                  int
                    height,
                    width;

                  if (sscanf(value,"%d +X %d",&height,&width) == 2)
                    {
                      image->columns=(size_t) width;
                      image->rows=(size_t) height;
                    }
                  break;
                }
              (void) FormatLocaleString(tag,MagickPathExtent,"hdr:%s",keyword);
              (void) SetImageProperty(image,tag,value,exception);
              break;
            }
            default:
            {
              (void) FormatLocaleString(tag,MagickPathExtent,"hdr:%s",keyword);
              (void) SetImageProperty(image,tag,value,exception);
              break;
            }
          }
        }
    if ((image->columns == 0) && (image->rows == 0))
      while (isspace((int) ((unsigned char) c)) != 0)
        c=ReadBlobByte(image);
  }
  if ((LocaleCompare(format,"32-bit_rle_rgbe") != 0) &&
      (LocaleCompare(format,"32-bit_rle_xyze") != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
  (void) SetImageColorspace(image,RGBColorspace,exception);
  if (LocaleCompare(format,"32-bit_rle_xyze") == 0)
    (void) SetImageColorspace(image,XYZColorspace,exception);
  image->compression=(image->columns < 8) || (image->columns > 0x7ffff) ?
    NoCompression : RLECompression;
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Read RGBE (red+green+blue+exponent) pixels.
  */
  pixels=(unsigned char *) AcquireQuantumMemory(image->columns,4*
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(pixels,0,4*image->columns*sizeof(*pixels));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    if (image->compression != RLECompression)
      {
        count=ReadBlob(image,4*image->columns*sizeof(*pixels),pixels);
        if (count != (ssize_t) (4*image->columns*sizeof(*pixels)))
          break;
      }
    else
      {
        count=ReadBlob(image,4*sizeof(*pixel),pixel);
        if (count != 4)
          break;
        if ((size_t) ((((size_t) pixel[2]) << 8) | pixel[3]) != image->columns)
          {
            (void) memcpy(pixels,pixel,4*sizeof(*pixel));
            count=ReadBlob(image,4*(image->columns-1)*sizeof(*pixels),pixels+4);
            image->compression=NoCompression;
          }
        else
          {
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
          }
      }
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    i=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (image->compression == RLECompression)
        {
          pixel[0]=pixels[x];
          pixel[1]=pixels[x+image->columns];
          pixel[2]=pixels[x+2*image->columns];
          pixel[3]=pixels[x+3*image->columns];
        }
      else
        {
          pixel[0]=pixels[i++];
          pixel[1]=pixels[i++];
          pixel[2]=pixels[i++];
          pixel[3]=pixels[i++];
        }
      SetPixelRed(image,0,q);
      SetPixelGreen(image,0,q);
      SetPixelBlue(image,0,q);
      if (pixel[3] != 0)
        {
          gamma=pow(2.0,pixel[3]-(128.0+8.0));
          SetPixelRed(image,ClampToQuantum(QuantumRange*gamma*pixel[0]),q);
          SetPixelGreen(image,ClampToQuantum(QuantumRange*gamma*pixel[1]),q);
          SetPixelBlue(image,ClampToQuantum(QuantumRange*gamma*pixel[2]),q);
        }
      q+=GetPixelChannels(image);
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

  entry=AcquireMagickInfo("HDR","HDR","Radiance RGBE image format");
  entry->decoder=(DecodeImageHandler *) ReadHDRImage;
  entry->encoder=(EncodeImageHandler *) WriteHDRImage;
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H D R I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteHDRImage() writes an image in the Radience RGBE image format.
%
%  The format of the WriteHDRImage method is:
%
%      MagickBooleanType WriteHDRImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static size_t HDRWriteRunlengthPixels(Image *image,unsigned char *pixels)
{
#define MinimumRunlength 4

  register size_t
    p,
    q;

  size_t
    runlength;

  ssize_t
    count,
    previous_count;

  unsigned char
    pixel[2];

  for (p=0; p < image->columns; )
  {
    q=p;
    runlength=0;
    previous_count=0;
    while ((runlength < MinimumRunlength) && (q < image->columns))
    {
      q+=runlength;
      previous_count=(ssize_t) runlength;
      runlength=1;
      while ((pixels[q] == pixels[q+runlength]) &&
             ((q+runlength) < image->columns) && (runlength < 127))
       runlength++;
    }
    if ((previous_count > 1) && (previous_count == (ssize_t) (q-p)))
      {
        pixel[0]=(unsigned char) (128+previous_count);
        pixel[1]=pixels[p];
        if (WriteBlob(image,2*sizeof(*pixel),pixel) < 1)
          break;
        p=q;
      }
    while (p < q)
    {
      count=(ssize_t) (q-p);
      if (count > 128)
        count=128;
      pixel[0]=(unsigned char) count;
      if (WriteBlob(image,sizeof(*pixel),pixel) < 1)
        break;
      if (WriteBlob(image,(size_t) count*sizeof(*pixel),&pixels[p]) < 1)
        break;
      p+=count;
    }
    if (runlength >= MinimumRunlength)
      {
        pixel[0]=(unsigned char) (128+runlength);
        pixel[1]=pixels[q];
        if (WriteBlob(image,2*sizeof(*pixel),pixel) < 1)
          break;
        p+=runlength;
      }
  }
  return(p);
}

static MagickBooleanType WriteHDRImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    header[MagickPathExtent];

  const char
    *property;

  MagickBooleanType
    status;

  register const Quantum
    *p;

  register ssize_t
    i,
    x;

  size_t
    length;

  ssize_t
    count,
    y;

  unsigned char
    pixel[4],
    *pixels;

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
  if (IsRGBColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,RGBColorspace,exception);
  /*
    Write header.
  */
  (void) memset(header,' ',MagickPathExtent);
  length=CopyMagickString(header,"#?RADIANCE\n",MagickPathExtent);
  (void) WriteBlob(image,length,(unsigned char *) header);
  property=GetImageProperty(image,"comment",exception);
  if ((property != (const char *) NULL) &&
      (strchr(property,'\n') == (char *) NULL))
    {
      count=FormatLocaleString(header,MagickPathExtent,"#%.*s\n",
        MagickPathExtent-3,property);
      (void) WriteBlob(image,(size_t) count,(unsigned char *) header);
    }
  property=GetImageProperty(image,"hdr:exposure",exception);
  if (property != (const char *) NULL)
    {
      count=FormatLocaleString(header,MagickPathExtent,"EXPOSURE=%g\n",
        strtod(property,(char **) NULL));
      (void) WriteBlob(image,(size_t) count,(unsigned char *) header);
    }
  if (image->gamma != 0.0)
    {
      count=FormatLocaleString(header,MagickPathExtent,"GAMMA=%g\n",
        image->gamma);
      (void) WriteBlob(image,(size_t) count,(unsigned char *) header);
    }
  count=FormatLocaleString(header,MagickPathExtent,
    "PRIMARIES=%g %g %g %g %g %g %g %g\n",
    image->chromaticity.red_primary.x,image->chromaticity.red_primary.y,
    image->chromaticity.green_primary.x,image->chromaticity.green_primary.y,
    image->chromaticity.blue_primary.x,image->chromaticity.blue_primary.y,
    image->chromaticity.white_point.x,image->chromaticity.white_point.y);
  (void) WriteBlob(image,(size_t) count,(unsigned char *) header);
  length=CopyMagickString(header,"FORMAT=32-bit_rle_rgbe\n\n",MagickPathExtent);
  (void) WriteBlob(image,length,(unsigned char *) header);
  count=FormatLocaleString(header,MagickPathExtent,"-Y %.20g +X %.20g\n",
    (double) image->rows,(double) image->columns);
  (void) WriteBlob(image,(size_t) count,(unsigned char *) header);
  /*
    Write HDR pixels.
  */
  pixels=(unsigned char *) AcquireQuantumMemory(image->columns+128,4*
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(pixels,0,4*(image->columns+128)*sizeof(*pixels));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    if ((image->columns >= 8) && (image->columns <= 0x7ffff))
      {
        pixel[0]=2;
        pixel[1]=2;
        pixel[2]=(unsigned char) (image->columns >> 8);
        pixel[3]=(unsigned char) (image->columns & 0xff);
        count=WriteBlob(image,4*sizeof(*pixel),pixel);
        if (count != (ssize_t) (4*sizeof(*pixel)))
          break;
      }
    i=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        gamma;

      pixel[0]=0;
      pixel[1]=0;
      pixel[2]=0;
      pixel[3]=0;
      gamma=QuantumScale*GetPixelRed(image,p);
      if ((QuantumScale*GetPixelGreen(image,p)) > gamma)
        gamma=QuantumScale*GetPixelGreen(image,p);
      if ((QuantumScale*GetPixelBlue(image,p)) > gamma)
        gamma=QuantumScale*GetPixelBlue(image,p);
      if (gamma > MagickEpsilon)
        {
          int
            exponent;

          gamma=frexp(gamma,&exponent)*256.0/gamma;
          pixel[0]=(unsigned char) (gamma*QuantumScale*GetPixelRed(image,p));
          pixel[1]=(unsigned char) (gamma*QuantumScale*GetPixelGreen(image,p));
          pixel[2]=(unsigned char) (gamma*QuantumScale*GetPixelBlue(image,p));
          pixel[3]=(unsigned char) (exponent+128);
        }
      if ((image->columns >= 8) && (image->columns <= 0x7ffff))
        {
          pixels[x]=pixel[0];
          pixels[x+image->columns]=pixel[1];
          pixels[x+2*image->columns]=pixel[2];
          pixels[x+3*image->columns]=pixel[3];
        }
      else
        {
          pixels[i++]=pixel[0];
          pixels[i++]=pixel[1];
          pixels[i++]=pixel[2];
          pixels[i++]=pixel[3];
        }
      p+=GetPixelChannels(image);
    }
    if ((image->columns >= 8) && (image->columns <= 0x7ffff))
      {
        for (i=0; i < 4; i++)
          length=HDRWriteRunlengthPixels(image,&pixels[i*image->columns]);
      }
    else
      {
        count=WriteBlob(image,4*image->columns*sizeof(*pixels),pixels);
        if (count != (ssize_t) (4*image->columns*sizeof(*pixels)))
          break;
      }
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  (void) CloseBlob(image);
  return(MagickTrue);
}

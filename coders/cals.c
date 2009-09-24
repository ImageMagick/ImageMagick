/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         CCCC   AAA   L      SSSSS                           %
%                        C      A   A  L      SS                              %
%                        C      AAAAA  L       SSS                            %
%                        C      A   A  L         SS                           %
%                         CCCC  A   A  LLLLL  SSSSS                           %
%                                                                             %
%                                                                             %
%                 Read/Write CALS Raster Group 1 Image Format                 %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
% The CALS raster format is a standard developed by the Computer Aided
% Acquisition and Logistics Support (CALS) office of the United States
% Department of Defense to standardize graphics data interchange for
% electronic publishing, especially in the areas of technical graphics,
% CAD/CAM, and image processing applications.
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
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#if defined(MAGICKCORE_TIFF_DELEGATE)
#if defined(MAGICKCORE_HAVE_TIFFCONF_H)
#include "tiffconf.h"
#endif
#include "tiffio.h"
#define CCITTParam  "-1"
#else
#define CCITTParam  "0"
#endif

/*
 Forward declarations.
*/
static MagickBooleanType
  WriteCALSImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s C A L S                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsCALS() returns MagickTrue if the image format type, identified by the
%  magick string, is CALS Raster Group 1.
%
%  The format of the IsCALS method is:
%
%      MagickBooleanType IsCALS(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsCALS(const unsigned char *magick,const size_t length)
{
  if (length < 128)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"version: MIL-STD-1840",21) == 0)
    return(MagickTrue);
  if (LocaleNCompare((const char *) magick,"srcdocid:",9) == 0)
    return(MagickTrue);
  if (LocaleNCompare((const char *) magick,"rorient:",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C A L S I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadCALSImage() reads an CALS Raster Group 1 image format image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadCALSImage method is:
%
%      Image *ReadCALSImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t WriteCALSLSBLong(FILE *file,const unsigned int value)
{
  unsigned char
    buffer[4];

  buffer[0]=(unsigned char) value;
  buffer[1]=(unsigned char) (value >> 8);
  buffer[2]=(unsigned char) (value >> 16);
  buffer[3]=(unsigned char) (value >> 24);
  return(fwrite(buffer,1,4,file));
}

#if defined(MAGICKCORE_TIFF_DELEGATE)
static Image *Huffman2DDecodeImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent];

  FILE
    *file;

  Image
    *huffman_image;

  ImageInfo
    *read_info;

  int
    c,
    unique_file;

  size_t
    length;

  ssize_t
    offset,
    strip_offset;

  /*
    Write CALS facsimile document wrapped as a TIFF image file.
  */
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    ThrowImageException(FileOpenError,"UnableToCreateTemporaryFile");
  length=fwrite("\111\111\052\000\010\000\000\000\016\000",1,10,file);
  length=fwrite("\376\000\003\000\001\000\000\000\000\000\000\000",1,12,file);
  length=fwrite("\000\001\004\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,image->columns);
  length=fwrite("\001\001\004\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,image->rows);
  length=fwrite("\002\001\003\000\001\000\000\000\001\000\000\000",1,12,file);
  length=fwrite("\003\001\003\000\001\000\000\000\004\000\000\000",1,12,file);
  length=fwrite("\006\001\003\000\001\000\000\000\000\000\000\000",1,12,file);
  length=fwrite("\021\001\003\000\001\000\000\000",1,8,file);
  strip_offset=10+(12*14)+4+8;
  length=WriteCALSLSBLong(file,(unsigned long) strip_offset);
  length=fwrite("\022\001\003\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,(unsigned long) image->orientation);
  length=fwrite("\025\001\003\000\001\000\000\000\001\000\000\000",1,12,file);
  length=fwrite("\026\001\004\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,image->columns);
  length=fwrite("\027\001\004\000\001\000\000\000\000\000\000\000",1,12,file);
  offset=(ssize_t) ftell(file)-4;
  length=fwrite("\032\001\005\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,(unsigned long) (strip_offset-8));
  length=fwrite("\033\001\005\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,(unsigned long) (strip_offset-8));
  length=fwrite("\050\001\003\000\001\000\000\000\002\000\000\000",1,12,file);
  length=fwrite("\000\000\000\000",1,4,file);
  length=WriteCALSLSBLong(file,image->x_resolution);
  length=WriteCALSLSBLong(file,1);
  for (length=0; (c=ReadBlobByte(image)) != EOF; length++)
    (void) fputc(c,file);
  (void) CloseBlob(image);
  offset=(ssize_t) fseek(file,(long) offset,SEEK_SET);
  length=WriteCALSLSBLong(file,(unsigned int) length);
  (void) fclose(file);
  /*
    Read TIFF image.
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) FormatMagickString(read_info->filename,MaxTextExtent,"tiff:%.1024s",
    filename);
  huffman_image=ReadImage(read_info,exception);
  if (huffman_image != (Image *) NULL)
    {
      (void) CopyMagickString(huffman_image->filename,image_info->filename,
        MaxTextExtent);
      (void) CopyMagickString(huffman_image->magick_filename,
         image_info->filename,MaxTextExtent);
      (void) CopyMagickString(huffman_image->magick,"CALS",MaxTextExtent);
    }
  read_info=DestroyImageInfo(read_info);
  (void) RelinquishUniqueFileResource(filename);
  return(huffman_image);
}
#else
static Image *Huffman2DDecodeImage(const ImageInfo *magick_unused(image_info),
  Image *image,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","`%s' (TIFF)",image->filename);
  return((Image *) NULL);
}
#endif

static Image *ReadCALSImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    header[129];

  Image
    *huffman_image,
    *image;

  MagickBooleanType
    status;

  register long
    i;

  unsigned long
    density,
    direction,
    height,
    orientation,
    pel_path,
    type,
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
    Read CALS header.
  */
  (void) ResetMagickMemory(header,0,sizeof(header));
  density=0;
  direction=0;
  orientation=1;
  pel_path=0;
  type=1;
  width=0;
  height=0;
  for (i=0; i < 16; i++)
  {
    if (ReadBlob(image,128,(unsigned char *) header) != 128)
      break;
    switch (*header)
    {
      case 'R':
      case 'r':
      {
        if (LocaleNCompare(header,"rdensty:",8) == 0)
          {
            (void) sscanf(header+8,"%lu",&density);
            break;
          }
        if (LocaleNCompare(header,"rpelcnt:",8) == 0)
          {
            (void) sscanf(header+8,"%lu,%lu",&width,&height);
            break;
          }
        if (LocaleNCompare(header,"rorient:",8) == 0)
          {
            (void) sscanf(header+8,"%lu,%lu",&pel_path,&direction);
            if (pel_path == 90)
              orientation=5;
            else
              if (pel_path == 90)
                orientation=3;
              else
                if (pel_path == 270)
                  orientation=7;
            if (direction == 90)
              orientation++;
            break;
          }
        if (LocaleNCompare(header,"rtype:",6) == 0)
          {
            (void) sscanf(header+6,"%lu",&type);
            break;
          }
        break;
      }
    }
  }
  if ((width == 0) || (height == 0) || (type == 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->columns=width;
  image->rows=height;
  image->x_resolution=(double) density;
  image->y_resolution=(double) density;
  image->orientation=(OrientationType) orientation;
  huffman_image=Huffman2DDecodeImage(image_info,image,exception);
  image=DestroyImage(image);
  (void) CloseBlob(huffman_image);
  if (huffman_image == (Image *) NULL)
    return(huffman_image);
  return(GetFirstImageInList(huffman_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r C A L S I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterCALSImage() adds attributes for the CALS Raster Group 1 image file
%  image format to the list of supported formats.  The attributes include the
%  image format tag, a method to read and/or write the format, whether the
%  format supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief description
%  of the format.
%
%  The format of the RegisterCALSImage method is:
%
%      unsigned long RegisterCALSImage(void)
%
*/
ModuleExport unsigned long RegisterCALSImage(void)
{
  MagickInfo
    *entry;

  static const char
    *CALSDescription=
    {
      "Continuous Acquisition and Life-cycle Support Type 1 Image"
    },
    *CALSNote=
    {
      "Specified in MIL-R-28002 and MIL-PRF-28002"
    };

  entry=SetMagickInfo("CAL");
  entry->decoder=(DecodeImageHandler *) ReadCALSImage;
#if defined(MAGICKCORE_TIFF_DELEGATE)
  entry->encoder=(EncodeImageHandler *) WriteCALSImage;
#endif
  entry->adjoin=MagickFalse;
  entry->magick=(IsImageFormatHandler *) IsCALS;
  entry->description=ConstantString(CALSDescription);
  entry->note=ConstantString(CALSNote);
  entry->module=ConstantString("CALS");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("CALS");
  entry->decoder=(DecodeImageHandler *) ReadCALSImage;
#if defined(MAGICKCORE_TIFF_DELEGATE)
  entry->encoder=(EncodeImageHandler *) WriteCALSImage;
#endif
  entry->adjoin=MagickFalse;
  entry->magick=(IsImageFormatHandler *) IsCALS;
  entry->description=ConstantString(CALSDescription);
  entry->note=ConstantString(CALSNote);
  entry->module=ConstantString("CALS");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r C A L S I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterCALSImage() removes format registrations made by the
%  CALS module from the list of supported formats.
%
%  The format of the UnregisterCALSImage method is:
%
%      UnregisterCALSImage(void)
%
*/
ModuleExport void UnregisterCALSImage(void)
{
  (void) UnregisterMagickInfo("CAL");
  (void) UnregisterMagickInfo("CALS");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e C A L S I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteCALSImage() writes an image to a file in CALS Raster Group 1 image
%  format.
%
%  The format of the WriteCALSImage method is:
%
%      MagickBooleanType WriteCALSImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

#if defined(MAGICKCORE_TIFF_DELEGATE)
static MagickBooleanType Huffman2DEncodeImage(const ImageInfo *image_info,
  Image *image,Image *inject_image)
{
  char
    filename[MaxTextExtent];

  FILE
    *file;

  Image
    *huffman_image;

  ImageInfo
    *write_info;

  int
    unique_file;

  MagickBooleanType
    status;

  register long
    i;

  ssize_t
    count;

  TIFF
    *tiff;

  uint16
    fillorder;

  uint32
    *byte_count,
    strip_size;

  unsigned char
    *buffer;

  /*
    Write image as CCITTFax4 TIFF image to a temporary file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(inject_image != (Image *) NULL);
  assert(inject_image->signature == MagickSignature);
  huffman_image=CloneImage(inject_image,0,0,MagickTrue,&image->exception);
  if (huffman_image == (Image *) NULL)
    return(MagickFalse);
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb"); 
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      ThrowFileException(&image->exception,FileOpenError,
        "UnableToCreateTemporaryFile",filename);
      return(MagickFalse);
    }
  (void) FormatMagickString(huffman_image->filename,MaxTextExtent,"tiff:%s",
    filename);
  write_info=CloneImageInfo(image_info);
  SetImageInfoFile(write_info,file);
  write_info->compression=Group4Compression;
  write_info->type=BilevelType;
  (void) SetImageOption(write_info,"quantum:polarity","min-is-white");
  status=WriteImage(write_info,huffman_image);
  (void) fflush(file);
  write_info=DestroyImageInfo(write_info);
  if (status == MagickFalse)
    {
      (void) RelinquishUniqueFileResource(filename);
      return(MagickFalse);
    }
  tiff=TIFFOpen(filename,"rb");
  if (tiff == (TIFF *) NULL)
    {
      huffman_image=DestroyImage(huffman_image);
      (void) fclose(file);
      (void) RelinquishUniqueFileResource(filename);
      ThrowFileException(&image->exception,FileOpenError,"UnableToOpenFile",
        image_info->filename);
      return(MagickFalse);
    }
  /*
    Allocate raw strip buffer.
  */
  byte_count=0;
  (void) TIFFGetField(tiff,TIFFTAG_STRIPBYTECOUNTS,&byte_count);
  strip_size=byte_count[0];
  for (i=1; i < (long) TIFFNumberOfStrips(tiff); i++)
    if (byte_count[i] > strip_size)
      strip_size=byte_count[i];
  buffer=(unsigned char *) AcquireQuantumMemory((size_t) strip_size,
    sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    {
      TIFFClose(tiff);
      huffman_image=DestroyImage(huffman_image);
      (void) fclose(file);
      (void) RelinquishUniqueFileResource(filename);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image_info->filename);
    }
  /*
    Compress runlength encoded to 2D Huffman pixels.
  */
  fillorder=FILLORDER_LSB2MSB;
  (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_FILLORDER,&fillorder);
  for (i=0; i < (long) TIFFNumberOfStrips(tiff); i++)
  {
    count=(ssize_t) TIFFReadRawStrip(tiff,(uint32) i,buffer,(long)
      byte_count[i]);
    if (fillorder == FILLORDER_LSB2MSB)
      TIFFReverseBits(buffer,(unsigned long) count);
    (void) WriteBlob(image,(size_t) count,buffer);
  }
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  TIFFClose(tiff);
  huffman_image=DestroyImage(huffman_image);
  (void) fclose(file);
  (void) RelinquishUniqueFileResource(filename);
  return(MagickTrue);
}
#else
static MagickBooleanType Huffman2DEncodeImage(const ImageInfo *image_info,
  Image *image,Image *inject_image)
{
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(inject_image != (Image *) NULL);
  assert(inject_image->signature == MagickSignature);
  (void) ThrowMagickException(&image->exception,GetMagickModule(),
    MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","`%s' (TIFF)",
    image->filename);
  return(MagickFalse);
}
#endif

static ssize_t WriteCALSRecord(Image *image,const char *data)
{
  char
    pad[128];

  ssize_t
    count;

  register const char
    *p;

  register long
    i;

  i=0;
  if (data != (const char *) NULL)
    {
      p=data;
      for (i=0; (i < 128) && (p[i] != '\0'); i++);
      count=WriteBlob(image,(size_t) i,(const unsigned char *) data);
    }
  if (i < 128)
    {
      i=128-i;
      (void) ResetMagickMemory(pad,' ',(const size_t) i);
      count=WriteBlob(image,(size_t) i,(const unsigned char *) pad);
    }
  return(count);
}

static MagickBooleanType WriteCALSImage(const ImageInfo *image_info,
  Image *image)
{
  char
    buffer[129];

  MagickBooleanType
    status;

  register long
    i;

  ssize_t
    count;

  unsigned long
    density,
    orient_x,
    orient_y;

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
    Create standard CALS header.
  */
  count=WriteCALSRecord(image,"srcdocid: NONE");
  count=WriteCALSRecord(image,"dstdocid: NONE");
  count=WriteCALSRecord(image,"txtfilid: NONE");
  count=WriteCALSRecord(image,"figid: NONE");
  count=WriteCALSRecord(image,"srcgph: NONE");
  count=WriteCALSRecord(image,"docls: NONE");
  count=WriteCALSRecord(image,"rtype: 1");
  orient_x=0;
  orient_y=0;
  switch (image->orientation)
  {
    case TopRightOrientation:
    {
      orient_x=180;
      orient_y=270;
      break;
    }
    case BottomRightOrientation:
    {
      orient_x=180;
      orient_y=90;
      break;
    }
    case BottomLeftOrientation:
    {
      orient_y=90;
      break;
    }
    case LeftTopOrientation:
    {
      orient_x=270;
      break;
    }
    case RightTopOrientation:
    {
      orient_x=270;
      orient_y=180;
      break;
    }
    case RightBottomOrientation:
    {
      orient_x=90;
      orient_y=180;
      break;
    }
    case LeftBottomOrientation:
    {
      orient_x=90;
      break;
    }
    default:
    {
      orient_y=270;
    }
  }
  (void) FormatMagickString(buffer,MaxTextExtent,"rorient: %03ld,%03ld",
    orient_x,orient_y);
  count=WriteCALSRecord(image,buffer);
  (void) FormatMagickString(buffer,MaxTextExtent,"rpelcnt: %06lu,%06lu",
    image->columns,image->rows);
  count=WriteCALSRecord(image,buffer);  
  density=200;
  if (image_info->density != (char *) NULL)
    {
      GeometryInfo
        geometry_info;

      (void) ParseGeometry(image_info->density,&geometry_info);
      density=(unsigned long) (geometry_info.rho+0.5);
    }
  (void) FormatMagickString(buffer,MaxTextExtent,"rdensty: %04lu",density);
  count=WriteCALSRecord(image,buffer);
  count=WriteCALSRecord(image,"notes: NONE");
  (void) ResetMagickMemory(buffer,' ',128);
  for (i=0; i < 5; i++)
    (void) WriteBlob(image,128,(unsigned char *) buffer);
  status=Huffman2DEncodeImage(image_info,image,image);
  (void) CloseBlob(image);
  return(status);
}

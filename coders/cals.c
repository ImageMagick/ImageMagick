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
%                        Read/Write CALS Image Format                         %
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
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

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
%  magick string, is CALS.
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
  if (LocaleNCompare((char *) magick,"version: MIL-STD-1840",21) == 0)
    return(MagickTrue);
  if (LocaleNCompare((char *) magick,"srcdocid:",9) == 0)
    return(MagickTrue);
  if (LocaleNCompare((char *) magick,"rorient:",8) == 0)
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
%  ReadCALSImage() reads an Automated Interchange of Technical Information,
%  MIL-STD-1840A image file and returns it.  It allocates the memory necessary
%  for the new Image structure and returns a pointer to the new image.
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

static Image *ReadCALSImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent],
    header[129];

  FILE
    *file;

  Image
    *image;

  ImageInfo
    *read_info;

  int
    c,
    unique_file;

  MagickBooleanType
    status;

  register long
    i;

  size_t
    length;

  ssize_t
    offset,
    strip_offset;

  unsigned int
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
            (void) sscanf(header+8,"%u",&density);
            break;
          }
        if (LocaleNCompare(header,"rpelcnt:",8) == 0)
          {
            (void) sscanf(header+8,"%u,%u",&width,&height);
            break;
          }
        if (LocaleNCompare(header,"rorient:",8) == 0)
          {
            (void) sscanf(header+8,"%u,%u",&pel_path,&direction);
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
            (void) sscanf(header+6,"%u",&type);
            break;
          }
        break;
      }
    }
  }
  if ((width == 0) || (height == 0) || (type == 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Write CALS facsimile document wrapped as a TIFF image file.
  */
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    ThrowReaderException(FileOpenError,"UnableToCreateTemporaryFile");
  length=fwrite("\111\111\052\000\010\000\000\000\016\000",1,10,file);
  length=fwrite("\376\000\003\000\001\000\000\000\000\000\000\000",1,12,file);
  length=fwrite("\000\001\004\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,width);
  length=fwrite("\001\001\004\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,height);
  length=fwrite("\002\001\003\000\001\000\000\000\001\000\000\000",1,12,file);
  length=fwrite("\003\001\003\000\001\000\000\000\004\000\000\000",1,12,file);
  length=fwrite("\006\001\003\000\001\000\000\000\000\000\000\000",1,12,file);
  length=fwrite("\021\001\003\000\001\000\000\000",1,8,file);
  strip_offset=10+(12*14)+4+8;
  length=WriteCALSLSBLong(file,(unsigned int) strip_offset);
  length=fwrite("\022\001\003\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,orientation);
  length=fwrite("\025\001\003\000\001\000\000\000\001\000\000\000",1,12,file);
  length=fwrite("\026\001\004\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,width);
  length=fwrite("\027\001\004\000\001\000\000\000\000\000\000\000",1,12,file);
  offset=(ssize_t) ftell(file)-4;
  length=fwrite("\032\001\005\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,(unsigned int) (strip_offset-8));
  length=fwrite("\033\001\005\000\001\000\000\000",1,8,file);
  length=WriteCALSLSBLong(file,(unsigned int) (strip_offset-8));
  length=fwrite("\050\001\003\000\001\000\000\000\002\000\000\000",1,12,file);
  length=fwrite("\000\000\000\000",1,4,file);
  length=WriteCALSLSBLong(file,density);
  length=WriteCALSLSBLong(file,1);
  for (length=0; (c=ReadBlobByte(image)) != EOF; length++)
    (void) fputc(c,file);
  (void) CloseBlob(image);
  image=DestroyImage(image);
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
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  (void) RelinquishUniqueFileResource(filename);
  if (image == (Image *) NULL)
    return(image);
  return(GetFirstImageInList(image));
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
%  RegisterCALSImage() adds attributes for the CALS X image format to the list
%  of supported formats.  The attributes include the image format tag, a
%  method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
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

  entry=SetMagickInfo("CALS");
  entry->decoder=(DecodeImageHandler *) ReadCALSImage;
  entry->adjoin=MagickFalse;
  entry->magick=(IsImageFormatHandler *) IsCALS;
  entry->description=ConstantString("Automated Interchange of Technical "
    "Information, MIL-STD-1840A");
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
  (void) UnregisterMagickInfo("CALS");
}

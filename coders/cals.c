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
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

#if defined(MAGICKCORE_TIFF_DELEGATE)
/*
 Forward declarations.
*/
static MagickBooleanType
  WriteCALSImage(const ImageInfo *,Image *);
#endif

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
static Image *ReadCALSImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent],
    header[129],
    message[MaxTextExtent];

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

  register ssize_t
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
              if (pel_path == 180)
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
  /*
    Read CALS pixels.
  */
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    ThrowImageException(FileOpenError,"UnableToCreateTemporaryFile");
  while ((c=ReadBlobByte(image)) != EOF)
    (void) fputc(c,file);
  (void) fclose(file);
  (void) CloseBlob(image);
  image=DestroyImage(image);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) FormatLocaleString(read_info->filename,MaxTextExtent,"group4:%s",
    filename);
  (void) FormatLocaleString(message,MaxTextExtent,"%lux%lu",width,height);
  read_info->size=ConstantString(message);
  (void) FormatLocaleString(message,MaxTextExtent,"%lu",density);
  read_info->density=ConstantString(message);
  read_info->orientation=(OrientationType) orientation;
  image=ReadImage(read_info,exception);
  if (image != (Image *) NULL)
    {
      (void) CopyMagickString(image->filename,image_info->filename,
        MaxTextExtent);
      (void) CopyMagickString(image->magick_filename,image_info->filename,
        MaxTextExtent);
      (void) CopyMagickString(image->magick,"CALS",MaxTextExtent);
    }
  read_info=DestroyImageInfo(read_info);
  (void) RelinquishUniqueFileResource(filename);
  return(image);
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
%      size_t RegisterCALSImage(void)
%
*/
ModuleExport size_t RegisterCALSImage(void)
{
  MagickInfo
    *entry;

  static const char
    *CALSDescription=
    {
      "Continuous Acquisition and Life-cycle Support Type 1"
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

#if defined(MAGICKCORE_TIFF_DELEGATE)
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

static ssize_t WriteCALSRecord(Image *image,const char *data)
{
  char
    pad[128];

  register const char
    *p;

  register ssize_t
    i;

  ssize_t
    count;

  i=0;
  count=0;
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
    header[129];

  Image
    *group4_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  register ssize_t
    i;

  size_t
    density,
    length,
    orient_x,
    orient_y;

  ssize_t
    count;

  unsigned char
    *group4;

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
  (void) count;
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
  (void) FormatLocaleString(header,MaxTextExtent,"rorient: %03ld,%03ld",
    (long) orient_x,(long) orient_y);
  count=WriteCALSRecord(image,header);
  (void) FormatLocaleString(header,MaxTextExtent,"rpelcnt: %06lu,%06lu",
    (unsigned long) image->columns,(unsigned long) image->rows);
  count=WriteCALSRecord(image,header);
  density=200;
  if (image_info->density != (char *) NULL)
    {
      GeometryInfo
        geometry_info;

      (void) ParseGeometry(image_info->density,&geometry_info);
      density=(size_t) floor(geometry_info.rho+0.5);
    }
  (void) FormatLocaleString(header,MaxTextExtent,"rdensty: %04lu",
    (unsigned long) density);
  count=WriteCALSRecord(image,header);
  count=WriteCALSRecord(image,"notes: NONE");
  (void) ResetMagickMemory(header,' ',128);
  for (i=0; i < 5; i++)
    (void) WriteBlob(image,128,(unsigned char *) header);
  /*
    Write CALS pixels.
  */
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->filename,"GROUP4:",MaxTextExtent);
  (void) CopyMagickString(write_info->magick,"GROUP4",MaxTextExtent);
  group4_image=CloneImage(image,0,0,MagickTrue,&image->exception);
  if (group4_image == (Image *) NULL)
    {
      (void) CloseBlob(image);
      return(MagickFalse);
    }
  group4=(unsigned char *) ImageToBlob(write_info,group4_image,&length,
    &image->exception);
  group4_image=DestroyImage(group4_image);
  if (group4 == (unsigned char *) NULL)
    {
      (void) CloseBlob(image);
      return(MagickFalse);
    }
  write_info=DestroyImageInfo(write_info);
  if (WriteBlob(image,length,group4) != (ssize_t) length)
    status=MagickFalse;
  group4=(unsigned char *) RelinquishMagickMemory(group4);
  (void) CloseBlob(image);
  return(status);
}
#endif

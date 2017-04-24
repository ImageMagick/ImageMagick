/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP    CCCC  L                                  %
%                            P   P  C      L                                  %
%                            PPPP   C      L                                  %
%                            P      C      L                                  %
%                            P       CCCC  LLLLL                              %
%                                                                             %
%                                                                             %
%                      Read/Write HP PCL Printer Format                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePCLImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P C L                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPCL() returns MagickTrue if the image format type, identified by the
%  magick string, is PCL.
%
%  The format of the IsPCL method is:
%
%      MagickBooleanType IsPCL(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsPCL(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\033E\033&",4) == 0)
    return(MagickFalse);
  if (memcmp(magick,"\033E\033",3) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P C L I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPCLImage() reads a Printer Control Language image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadPCLImage method is:
%
%      Image *ReadPCLImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadPCLImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define CropBox  "CropBox"
#define DeviceCMYK  "DeviceCMYK"
#define MediaBox  "MediaBox"
#define RenderPCLText  "  Rendering PCL...  "

  char
    command[MagickPathExtent],
    *density,
    filename[MagickPathExtent],
    geometry[MagickPathExtent],
    *options,
    input_filename[MagickPathExtent];

  const DelegateInfo
    *delegate_info;

  Image
    *image,
    *next_image;

  ImageInfo
    *read_info;

  MagickBooleanType
    cmyk,
    status;

  PointInfo
    delta;

  RectangleInfo
    bounding_box,
    page;

  register char
    *p;

  register ssize_t
    c;

  SegmentInfo
    bounds;

  size_t
    height,
    width;

  ssize_t
    count;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  /*
    Open image file.
  */
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  status=AcquireUniqueSymbolicLink(image_info->filename,input_filename);
  if (status == MagickFalse)
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        image_info->filename);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Set the page density.
  */
  delta.x=DefaultResolution;
  delta.y=DefaultResolution;
  if ((image->resolution.x == 0.0) || (image->resolution.y == 0.0))
    {
      GeometryInfo
        geometry_info;

      MagickStatusType
        flags;

      flags=ParseGeometry(PSDensityGeometry,&geometry_info);
      image->resolution.x=geometry_info.rho;
      image->resolution.y=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->resolution.y=image->resolution.x;
    }
  /*
    Determine page geometry from the PCL media box.
  */
  cmyk=image->colorspace == CMYKColorspace ? MagickTrue : MagickFalse;
  count=0;
  (void) ResetMagickMemory(&bounding_box,0,sizeof(bounding_box));
  (void) ResetMagickMemory(&bounds,0,sizeof(bounds));
  (void) ResetMagickMemory(&page,0,sizeof(page));
  (void) ResetMagickMemory(command,0,sizeof(command));
  p=command;
  for (c=ReadBlobByte(image); c != EOF; c=ReadBlobByte(image))
  {
    if (image_info->page != (char *) NULL)
      continue;
    /*
      Note PCL elements.
    */
    *p++=(char) c;
    if ((c != (int) '/') && (c != '\n') &&
        ((size_t) (p-command) < (MagickPathExtent-1)))
      continue;
    *p='\0';
    p=command;
    /*
      Is this a CMYK document?
    */
    if (LocaleNCompare(DeviceCMYK,command,strlen(DeviceCMYK)) == 0)
      cmyk=MagickTrue;
    if (LocaleNCompare(CropBox,command,strlen(CropBox)) == 0)
      {
        /*
          Note region defined by crop box.
        */
        count=(ssize_t) sscanf(command,"CropBox [%lf %lf %lf %lf",
          &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
        if (count != 4)
          count=(ssize_t) sscanf(command,"CropBox[%lf %lf %lf %lf",
            &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
      }
    if (LocaleNCompare(MediaBox,command,strlen(MediaBox)) == 0)
      {
        /*
          Note region defined by media box.
        */
        count=(ssize_t) sscanf(command,"MediaBox [%lf %lf %lf %lf",
          &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
        if (count != 4)
          count=(ssize_t) sscanf(command,"MediaBox[%lf %lf %lf %lf",
            &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
      }
    if (count != 4)
      continue;
    /*
      Set PCL render geometry.
    */
    width=(size_t) floor(bounds.x2-bounds.x1+0.5);
    height=(size_t) floor(bounds.y2-bounds.y1+0.5);
    if (width > page.width)
      page.width=width;
    if (height > page.height)
      page.height=height;
  }
  (void) CloseBlob(image);
  /*
    Render PCL with the GhostPCL delegate.
  */
  if ((page.width == 0) || (page.height == 0))
    (void) ParseAbsoluteGeometry(PSPageGeometry,&page);
  if (image_info->page != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->page,&page);
  (void) FormatLocaleString(geometry,MagickPathExtent,"%.20gx%.20g",(double)
    page.width,(double) page.height);
  if (image_info->monochrome != MagickFalse)
    delegate_info=GetDelegateInfo("pcl:mono",(char *) NULL,exception);
  else
     if (cmyk != MagickFalse)
       delegate_info=GetDelegateInfo("pcl:cmyk",(char *) NULL,exception);
     else
       delegate_info=GetDelegateInfo("pcl:color",(char *) NULL,exception);
  if (delegate_info == (const DelegateInfo *) NULL)
    return((Image *) NULL);
  if ((page.width == 0) || (page.height == 0))
    (void) ParseAbsoluteGeometry(PSPageGeometry,&page);
  if (image_info->page != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->page,&page);
  density=AcquireString("");
  options=AcquireString("");
  (void) FormatLocaleString(density,MagickPathExtent,"%gx%g",image->resolution.x,
    image->resolution.y);
  page.width=(size_t) floor(page.width*image->resolution.x/delta.x+0.5);
  page.height=(size_t) floor(page.height*image->resolution.y/delta.y+0.5);
  (void) FormatLocaleString(options,MagickPathExtent,"-g%.20gx%.20g ",(double)
     page.width,(double) page.height);
  image=DestroyImage(image);
  read_info=CloneImageInfo(image_info);
  *read_info->magick='\0';
  if (read_info->number_scenes != 0)
    {
      if (read_info->number_scenes != 1)
        (void) FormatLocaleString(options,MagickPathExtent,"-dLastPage=%.20g",
          (double) (read_info->scene+read_info->number_scenes));
      else
        (void) FormatLocaleString(options,MagickPathExtent,
          "-dFirstPage=%.20g -dLastPage=%.20g",(double) read_info->scene+1,
          (double) (read_info->scene+read_info->number_scenes));
      read_info->number_scenes=0;
      if (read_info->scenes != (char *) NULL)
        *read_info->scenes='\0';
    }
  (void) CopyMagickString(filename,read_info->filename,MagickPathExtent);
  (void) AcquireUniqueFilename(read_info->filename);
  (void) FormatLocaleString(command,MagickPathExtent,
    GetDelegateCommands(delegate_info),
    read_info->antialias != MagickFalse ? 4 : 1,
    read_info->antialias != MagickFalse ? 4 : 1,density,options,
    read_info->filename,input_filename);
  options=DestroyString(options);
  density=DestroyString(density);
  status=ExternalDelegateCommand(MagickFalse,read_info->verbose,command,
    (char *) NULL,exception) != 0 ? MagickTrue : MagickFalse;
  image=ReadImage(read_info,exception);
  (void) RelinquishUniqueFileResource(read_info->filename);
  (void) RelinquishUniqueFileResource(input_filename);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    ThrowReaderException(DelegateError,"PCLDelegateFailed");
  if (LocaleCompare(image->magick,"BMP") == 0)
    {
      Image
        *cmyk_image;

      cmyk_image=ConsolidateCMYKImages(image,exception);
      if (cmyk_image != (Image *) NULL)
        {
          image=DestroyImageList(image);
          image=cmyk_image;
        }
    }
  do
  {
    (void) CopyMagickString(image->filename,filename,MagickPathExtent);
    image->page=page;
    next_image=SyncNextImageInList(image);
    if (next_image != (Image *) NULL)
      image=next_image;
  } while (next_image != (Image *) NULL);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P C L I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPCLImage() adds attributes for the PCL image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the i file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPCLImage method is:
%
%      size_t RegisterPCLImage(void)
%
*/
ModuleExport size_t RegisterPCLImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("PCL","PCL","Printer Control Language");
  entry->decoder=(DecodeImageHandler *) ReadPCLImage;
  entry->encoder=(EncodeImageHandler *) WritePCLImage;
  entry->magick=(IsImageFormatHandler *) IsPCL;
  entry->flags^=CoderBlobSupportFlag;
  entry->flags^=CoderDecoderThreadSupportFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P C L I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPCLImage() removes format registrations made by the PCL module
%  from the list of supported formats.
%
%  The format of the UnregisterPCLImage method is:
%
%      UnregisterPCLImage(void)
%
*/
ModuleExport void UnregisterPCLImage(void)
{
  (void) UnregisterMagickInfo("PCL");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P C L I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePCLImage() writes an image in the Page Control Language encoded
%  image format.
%
%  The format of the WritePCLImage method is:
%
%      MagickBooleanType WritePCLImage(const ImageInfo *image_info,
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

static size_t PCLDeltaCompressImage(const size_t length,
  const unsigned char *previous_pixels,const unsigned char *pixels,
  unsigned char *compress_pixels)
{
  int
    delta,
    j,
    replacement;

  register ssize_t
    i,
    x;

  register unsigned char
    *q;

  q=compress_pixels;
  for (x=0; x < (ssize_t) length; )
  {
    j=0;
    for (i=0; x < (ssize_t) length; x++)
    {
      if (*pixels++ != *previous_pixels++)
        {
          i=1;
          break;
        }
      j++;
    }
    while (x < (ssize_t) length)
    {
      x++;
      if (*pixels == *previous_pixels)
        break;
      i++;
      previous_pixels++;
      pixels++;
    }
    if (i == 0)
      break;
    replacement=j >= 31 ? 31 : j;
    j-=replacement;
    delta=i >= 8 ? 8 : i;
    *q++=(unsigned char) (((delta-1) << 5) | replacement);
    if (replacement == 31)
      {
        for (replacement=255; j != 0; )
        {
          if (replacement > j)
            replacement=j;
          *q++=(unsigned char) replacement;
          j-=replacement;
        }
        if (replacement == 255)
          *q++='\0';
      }
    for (pixels-=i; i != 0; )
    {
      for (i-=delta; delta != 0; delta--)
        *q++=(*pixels++);
      if (i == 0)
        break;
      delta=i;
      if (i >= 8)
        delta=8;
      *q++=(unsigned char) ((delta-1) << 5);
    }
  }
  return((size_t) (q-compress_pixels));
}

static size_t PCLPackbitsCompressImage(const size_t length,
  const unsigned char *pixels,unsigned char *compress_pixels)
{
  int
    count;

  register ssize_t
    x;

  register unsigned char
    *q;

  ssize_t
    j;

  unsigned char
    packbits[128];

  /*
    Compress pixels with Packbits encoding.
  */
  q=compress_pixels;
  for (x=(ssize_t) length; x != 0; )
  {
    switch (x)
    {
      case 1:
      {
        x--;
        *q++=0;
        *q++=(*pixels);
        break;
      }
      case 2:
      {
        x-=2;
        *q++=1;
        *q++=(*pixels);
        *q++=pixels[1];
        break;
      }
      case 3:
      {
        x-=3;
        if ((*pixels == *(pixels+1)) && (*(pixels+1) == *(pixels+2)))
          {
            *q++=(unsigned char) ((256-3)+1);
            *q++=(*pixels);
            break;
          }
        *q++=2;
        *q++=(*pixels);
        *q++=pixels[1];
        *q++=pixels[2];
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
            while (((ssize_t) count < x) && (*pixels == *(pixels+count)))
            {
              count++;
              if (count >= 127)
                break;
            }
            x-=count;
            *q++=(unsigned char) ((256-count)+1);
            *q++=(*pixels);
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
          if (((ssize_t) count >= (x-3)) || (count >= 127))
            break;
        }
        x-=count;
        *packbits=(unsigned char) (count-1);
        for (j=0; j <= (ssize_t) count; j++)
          *q++=packbits[j];
        pixels+=count;
        break;
      }
    }
  }
  *q++=128; /* EOD marker */
  return((size_t) (q-compress_pixels));
}

static MagickBooleanType WritePCLImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent];

  CompressionType
    compression;

  const char
    *option;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  register const Quantum *p;

  register ssize_t i, x;

  register unsigned char *q;

  size_t
    density,
    length,
    one,
    packets;

  ssize_t
    y;

  unsigned char
    bits_per_pixel,
    *compress_pixels,
    *pixels,
    *previous_pixels;

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
  density=75;
  if (image_info->density != (char *) NULL)
    {
      GeometryInfo
        geometry;

      (void) ParseGeometry(image_info->density,&geometry);
      density=(size_t) geometry.rho;
    }
  scene=0;
  one=1;
  do
  {
    /*
      Initialize the printer.
    */
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
    (void) WriteBlobString(image,"\033E");  /* printer reset */
    (void) WriteBlobString(image,"\033*r3F");  /* set presentation mode */
    (void) FormatLocaleString(buffer,MagickPathExtent,"\033*r%.20gs%.20gT",
      (double) image->columns,(double) image->rows);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"\033*t%.20gR",(double)
      density);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"\033&l0E");  /* top margin 0 */
    if (SetImageMonochrome(image,exception) != MagickFalse)
      {
        /*
          Monochrome image: use default printer monochrome setup.
        */
        bits_per_pixel=1;
      }
    else
      if (image->storage_class == DirectClass)
        {
          /*
            DirectClass image.
          */
          bits_per_pixel=24;
          (void) WriteBlobString(image,"\033*v6W"); /* set color mode */
          (void) WriteBlobByte(image,0); /* RGB */
          (void) WriteBlobByte(image,3); /* direct by pixel */
          (void) WriteBlobByte(image,0); /* bits per index (ignored) */
          (void) WriteBlobByte(image,8); /* bits per red component */
          (void) WriteBlobByte(image,8); /* bits per green component */
          (void) WriteBlobByte(image,8); /* bits per blue component */
        }
      else
        {
          /*
            Colormapped image.
          */
          bits_per_pixel=8;
          (void) WriteBlobString(image,"\033*v6W"); /* set color mode... */
          (void) WriteBlobByte(image,0); /* RGB */
          (void) WriteBlobByte(image,1); /* indexed by pixel */
          (void) WriteBlobByte(image,bits_per_pixel); /* bits per index */
          (void) WriteBlobByte(image,8); /* bits per red component */
          (void) WriteBlobByte(image,8); /* bits per green component */
          (void) WriteBlobByte(image,8); /* bits per blue component */
          for (i=0; i < (ssize_t) image->colors; i++)
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,
              "\033*v%da%db%dc%.20gI",
              ScaleQuantumToChar(image->colormap[i].red),
              ScaleQuantumToChar(image->colormap[i].green),
              ScaleQuantumToChar(image->colormap[i].blue),(double) i);
            (void) WriteBlobString(image,buffer);
          }
          for (one=1; i < (ssize_t) (one << bits_per_pixel); i++)
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,"\033*v%.20gI",
              (double) i);
            (void) WriteBlobString(image,buffer);
          }
        }
    option=GetImageOption(image_info,"pcl:fit-to-page");
    if (IsStringTrue(option) != MagickFalse)
      (void) WriteBlobString(image,"\033*r3A");
    else
      (void) WriteBlobString(image,"\033*r1A");  /* start raster graphics */
    (void) WriteBlobString(image,"\033*b0Y");  /* set y offset */
    length=(image->columns*bits_per_pixel+7)/8;
    pixels=(unsigned char *) AcquireQuantumMemory(length+1,sizeof(*pixels));
    if (pixels == (unsigned char *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    (void) ResetMagickMemory(pixels,0,(length+1)*sizeof(*pixels));
    compress_pixels=(unsigned char *) NULL;
    previous_pixels=(unsigned char *) NULL;

    compression=UndefinedCompression;
    if (image_info->compression != UndefinedCompression)
      compression=image_info->compression;
    switch (compression)
    {
      case NoCompression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"\033*b0M");
        (void) WriteBlobString(image,buffer);
        break;
      }
      case RLECompression:
      {
        compress_pixels=(unsigned char *) AcquireQuantumMemory(length+256,
          sizeof(*compress_pixels));
        if (compress_pixels == (unsigned char *) NULL)
          {
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
        (void) ResetMagickMemory(compress_pixels,0,(length+256)*
          sizeof(*compress_pixels));
        (void) FormatLocaleString(buffer,MagickPathExtent,"\033*b2M");
        (void) WriteBlobString(image,buffer);
        break;
      }
      default:
      {
        compress_pixels=(unsigned char *) AcquireQuantumMemory(3*length+256,
          sizeof(*compress_pixels));
        if (compress_pixels == (unsigned char *) NULL)
          {
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
        (void) ResetMagickMemory(compress_pixels,0,(3*length+256)*
          sizeof(*compress_pixels));
        previous_pixels=(unsigned char *) AcquireQuantumMemory(length+1,
          sizeof(*previous_pixels));
        if (previous_pixels == (unsigned char *) NULL)
          {
            compress_pixels=(unsigned char *) RelinquishMagickMemory(
              compress_pixels);
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
        (void) ResetMagickMemory(previous_pixels,0,(length+1)*
          sizeof(*previous_pixels));
        (void) FormatLocaleString(buffer,MagickPathExtent,"\033*b3M");
        (void) WriteBlobString(image,buffer);
        break;
      }
    }
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        break;
      q=pixels;
      switch (bits_per_pixel)
      {
        case 1:
        {
          register unsigned char
            bit,
            byte;

          /*
            Monochrome image.
          */
          bit=0;
          byte=0;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            byte<<=1;
            if (GetPixelLuma(image,p) < (QuantumRange/2.0))
              byte|=0x01;
            bit++;
            if (bit == 8)
              {
                *q++=byte;
                bit=0;
                byte=0;
              }
            p+=GetPixelChannels(image);
          }
          if (bit != 0)
            *q++=byte << (8-bit);
          break;
        }
        case 8:
        {
          /*
            Colormapped image.
          */
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *q++=(unsigned char) GetPixelIndex(image,p);
            p+=GetPixelChannels(image);
          }
          break;
        }
        case 24:
        case 32:
        {
          /*
            Truecolor image.
          */
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *q++=ScaleQuantumToChar(GetPixelRed(image,p));
            *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
            *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
            p+=GetPixelChannels(image);
          }
          break;
        }
      }
      switch (compression)
      {
        case NoCompression:
        {
          (void) FormatLocaleString(buffer,MagickPathExtent,"\033*b%.20gW",
            (double) length);
          (void) WriteBlobString(image,buffer);
          (void) WriteBlob(image,length,pixels);
          break;
        }
        case RLECompression:
        {
          packets=PCLPackbitsCompressImage(length,pixels,compress_pixels);
          (void) FormatLocaleString(buffer,MagickPathExtent,"\033*b%.20gW",
            (double) packets);
          (void) WriteBlobString(image,buffer);
          (void) WriteBlob(image,packets,compress_pixels);
          break;
        }
        default:
        {
          if (y == 0)
            for (i=0; i < (ssize_t) length; i++)
              previous_pixels[i]=(~pixels[i]);
          packets=PCLDeltaCompressImage(length,previous_pixels,pixels,
            compress_pixels);
          (void) FormatLocaleString(buffer,MagickPathExtent,"\033*b%.20gW",
            (double) packets);
          (void) WriteBlobString(image,buffer);
          (void) WriteBlob(image,packets,compress_pixels);
          (void) CopyMagickMemory(previous_pixels,pixels,length*
            sizeof(*pixels));
          break;
        }
      }
    }
    (void) WriteBlobString(image,"\033*rB");  /* end graphics */
    switch (compression)
    {
      case NoCompression:
        break;
      case RLECompression:
      {
        compress_pixels=(unsigned char *) RelinquishMagickMemory(
          compress_pixels);
        break;
      }
      default:
      {
        previous_pixels=(unsigned char *) RelinquishMagickMemory(
          previous_pixels);
        compress_pixels=(unsigned char *) RelinquishMagickMemory(
          compress_pixels);
        break;
      }
    }
    pixels=(unsigned char *) RelinquishMagickMemory(pixels);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) WriteBlobString(image,"\033E");
  (void) CloseBlob(image);
  return(MagickTrue);
}

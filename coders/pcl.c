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
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/draw.h"
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
#include "magick/profile.h"
#include "magick/resource_.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/token.h"
#include "magick/transform.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePCLImage(const ImageInfo *,Image *);

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
  if (memcmp(magick,"\033E\033",3) == 0)
    return(MagickTrue);
  if (memcmp(magick,"\033E\033&",4) == 0)
    return(MagickFalse);
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
    command[MaxTextExtent],
    density[MaxTextExtent],
    filename[MaxTextExtent],
    geometry[MaxTextExtent],
    options[MaxTextExtent],
    input_filename[MaxTextExtent];

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

  register long
    c;

  SegmentInfo
    bounds;

  ssize_t
    count;

  unsigned long
    height,
    width;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  /*
    Open image file.
  */
  image=AcquireImage(image_info);
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
  if ((image->x_resolution == 0.0) || (image->y_resolution == 0.0))
    {
      GeometryInfo
        geometry_info;

      MagickStatusType
        flags;

      flags=ParseGeometry(PSDensityGeometry,&geometry_info);
      image->x_resolution=geometry_info.rho;
      image->y_resolution=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->y_resolution=image->x_resolution;
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
        ((size_t) (p-command) < (MaxTextExtent-1)))
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
    width=(unsigned long) (bounds.x2-bounds.x1+0.5);
    height=(unsigned long) (bounds.y2-bounds.y1+0.5);
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
  (void) FormatMagickString(geometry,MaxTextExtent,"%lux%lu",
    page.width,page.height);
  if (image_info->monochrome != MagickFalse)
    delegate_info=GetDelegateInfo("pcl:mono",(char *) NULL,exception);
  else
     if (cmyk != MagickFalse)
       delegate_info=GetDelegateInfo("pcl:cmyk",(char *) NULL,exception);
     else
       delegate_info=GetDelegateInfo("pcl:color",(char *) NULL,exception);
  if (delegate_info == (const DelegateInfo *) NULL)
    return((Image *) NULL);
  *options='\0';
  if ((page.width == 0) || (page.height == 0))
    (void) ParseAbsoluteGeometry(PSPageGeometry,&page);
  if (image_info->page != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->page,&page);
  (void) FormatMagickString(density,MaxTextExtent,"%gx%g",
    image->x_resolution,image->y_resolution);
  page.width=(unsigned long) (page.width*image->x_resolution/delta.x+0.5);
  page.height=(unsigned long) (page.height*image->y_resolution/delta.y+0.5);
  (void) FormatMagickString(options,MaxTextExtent,"-g%lux%lu ",
    page.width,page.height);
  image=DestroyImage(image);
  read_info=CloneImageInfo(image_info);
  *read_info->magick='\0';
  if (read_info->number_scenes != 0)
    {
      if (read_info->number_scenes != 1)
        (void) FormatMagickString(options,MaxTextExtent,"-dLastPage=%lu",
          read_info->scene+read_info->number_scenes);
      else
        (void) FormatMagickString(options,MaxTextExtent,
          "-dFirstPage=%lu -dLastPage=%lu",read_info->scene+1,read_info->scene+
          read_info->number_scenes);
      read_info->number_scenes=0;
      if (read_info->scenes != (char *) NULL)
        *read_info->scenes='\0';
    }
  if (read_info->authenticate != (char *) NULL)
    (void) FormatMagickString(options+strlen(options),MaxTextExtent,
      " -sPCLPassword=%s",read_info->authenticate);
  (void) CopyMagickString(filename,read_info->filename,MaxTextExtent);
  (void) AcquireUniqueFilename(read_info->filename);
  (void) FormatMagickString(command,MaxTextExtent,
    GetDelegateCommands(delegate_info),
    read_info->antialias != MagickFalse ? 4 : 1,
    read_info->antialias != MagickFalse ? 4 : 1,density,options,
    read_info->filename,input_filename);
  status=SystemCommand(MagickFalse,read_info->verbose,command,exception) != 0 ?
    MagickTrue : MagickFalse;
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

      cmyk_image=ConsolidateCMYKImages(image,&image->exception);
      if (cmyk_image != (Image *) NULL)
        {
          image=DestroyImageList(image);
          image=cmyk_image;
        }
    }
  do
  {
    (void) CopyMagickString(image->filename,filename,MaxTextExtent);
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
%      unsigned long RegisterPCLImage(void)
%
*/
ModuleExport unsigned long RegisterPCLImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("PCL");
  entry->decoder=(DecodeImageHandler *) ReadPCLImage;
  entry->encoder=(EncodeImageHandler *) WritePCLImage;
  entry->magick=(IsImageFormatHandler *) IsPCL;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=EncoderThreadSupport;
  entry->description=ConstantString("Printer Control Language");
  entry->module=ConstantString("PCL");
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
%      MagickBooleanType WritePCLImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
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

  register long
    i,
    x;

  register unsigned char
    *q;

  q=compress_pixels;
  for (x=0; x < (long) length; )
  {
    j=0;
    for (i=0; x < (long) length; x++)
    {
      if (*pixels++ != *previous_pixels++)
        {
          i=1;
          break;
        }
      j++;
    }
    for ( ; x < (long) length; x++)
    {
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

  long
    j;

  register long
    x;

  register unsigned char
    *q;

  unsigned char
    packbits[128];

  /*
    Compress pixels with Packbits encoding.
  */
  q=compress_pixels;
  for (x=(long) length; x != 0; )
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
            while (((long) count < x) && (*pixels == *(pixels+count)))
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
          if (((long) count >= (x-3)) || (count >= 127))
            break;
        }
        x-=count;
        *packbits=(unsigned char) (count-1);
        for (j=0; j <= (long) count; j++)
          *q++=packbits[j];
        pixels+=count;
        break;
      }
    }
  }
  *q++=128; /* EOD marker */
  return((size_t) (q-compress_pixels));
}

static MagickBooleanType WritePCLImage(const ImageInfo *image_info,Image *image)
{
  char
    buffer[MaxTextExtent];

  const char
    *option;

  long
    y;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    i,
    x;

  register unsigned char
    *q;

  size_t
    length,
    packets;

  unsigned char
    bits_per_pixel,
    *compress_pixels,
    *pixels,
    *previous_pixels;

  unsigned long
    density;

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
  density=75;
  if (image_info->density != (char *) NULL)
    {
      GeometryInfo
        geometry;

      (void) ParseGeometry(image_info->density,&geometry);
      density=(unsigned long) geometry.rho;
    }
  scene=0;
  do
  {
    if (image->colorspace != RGBColorspace)
      (void) TransformImageColorspace(image,RGBColorspace);
    /*
      Initialize the printer.
    */
    (void) WriteBlobString(image,"\033E");  /* printer reset */
    (void) WriteBlobString(image,"\033*r3F");  /* set presentation mode */
    (void) FormatMagickString(buffer,MaxTextExtent,"\033*r%lus%luT",
      image->columns,image->rows);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"\033*t%ldR",density);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"\033&l0E");  /* top margin 0 */
    if (IsMonochromeImage(image,&image->exception) != MagickFalse)
      {
        /*
          Monochrome image.
        */
        bits_per_pixel=1;
        (void) WriteBlobString(image,"\033*v6W"); /* set color mode... */
        (void) WriteBlobByte(image,0); /* RGB */
        (void) WriteBlobByte(image,1); /* indexed by pixel */
        (void) WriteBlobByte(image,bits_per_pixel); /* bits per index */
        (void) WriteBlobByte(image,8); /* bits per red component */
        (void) WriteBlobByte(image,8); /* bits per green component */
        (void) WriteBlobByte(image,8); /* bits per blue component */
        (void) FormatMagickString(buffer,MaxTextExtent,"\033*v0a0b0c0I");
        (void) WriteBlobString(image,buffer);
        (void) FormatMagickString(buffer,MaxTextExtent,"\033*v1a1b1c1I");
        (void) WriteBlobString(image,buffer);
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
          for (i=0; i < (long) image->colors; i++)
          {
            (void) FormatMagickString(buffer,MaxTextExtent,
              "\033*v%da%db%dc%ldI",ScaleQuantumToChar(image->colormap[i].red),
              ScaleQuantumToChar(image->colormap[i].green),
              ScaleQuantumToChar(image->colormap[i].blue),i);
            (void) WriteBlobString(image,buffer);
          }
          for ( ; i < (1L << bits_per_pixel); i++)
          {
            (void) FormatMagickString(buffer,MaxTextExtent,"\033*v%luI",i);
            (void) WriteBlobString(image,buffer);
          }
        }
    option=GetImageOption(image_info,"pcl:fit-to-page");
    if ((option != (const char *) NULL) &&
        (IsMagickTrue(option) != MagickFalse))
      (void) WriteBlobString(image,"\033*r3A");
    else
      (void) WriteBlobString(image,"\033*r1A");  /* start raster graphics */
    (void) WriteBlobString(image,"\033*b0Y");  /* set y offset */
    length=(image->columns*bits_per_pixel+7)/8;
    pixels=(unsigned char *) AcquireQuantumMemory(length,sizeof(*pixels));
    if (pixels == (unsigned char *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    compress_pixels=(unsigned char *) NULL;
    previous_pixels=(unsigned char *) NULL;
    switch (image->compression)
    {
      case NoCompression:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"\033*b0M");
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
        (void) FormatMagickString(buffer,MaxTextExtent,"\033*b2M");
        (void) WriteBlobString(image,buffer);
        break;
      }
      default:
      {
        compress_pixels=(unsigned char *) AcquireQuantumMemory(length+
          (length >> 3),sizeof(*compress_pixels));
        if (compress_pixels == (unsigned char *) NULL)
          {
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
        previous_pixels=(unsigned char *) AcquireQuantumMemory(length,
          sizeof(*previous_pixels));
        if (previous_pixels == (unsigned char *) NULL)
          {
            compress_pixels=(unsigned char *) RelinquishMagickMemory(
              compress_pixels);
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
        (void) FormatMagickString(buffer,MaxTextExtent,"\033*b3M");
        (void) WriteBlobString(image,buffer);
        break;
      }
    }
    for (y=0; y < (long) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetAuthenticIndexQueue(image);
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
          for (x=0; x < (long) image->columns; x++)
          {
            byte<<=1;
            if (PixelIntensity(p) >= ((MagickRealType) QuantumRange/2.0))
              byte|=0x01;
            bit++;
            if (bit == 8)
              {
                *q++=byte;
                bit=0;
                byte=0;
              }
            p++;
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
          for (x=0; x < (long) image->columns; x++)
            *q++=(unsigned char) indexes[x];
          break;
        }
        case 24:
        case 32:
        {
          /*
            Truecolor image.
          */
          for (x=0; x < (long) image->columns; x++)
          {
            *q++=ScaleQuantumToChar(GetRedPixelComponent(p));
            *q++=ScaleQuantumToChar(GetGreenPixelComponent(p));
            *q++=ScaleQuantumToChar(GetBluePixelComponent(p));
            p++;
          }
          break;
        }
      }
      switch (image->compression)
      {
        case NoCompression:
        {
          (void) FormatMagickString(buffer,MaxTextExtent,"\033*b%luW",
            (unsigned long) length);
          (void) WriteBlobString(image,buffer);
          (void) WriteBlob(image,length,pixels);
          break;
        }
        case RLECompression:
        {
          packets=PCLPackbitsCompressImage(length,pixels,
            compress_pixels);
          (void) FormatMagickString(buffer,MaxTextExtent,"\033*b%luW",
            (unsigned long) packets);
          (void) WriteBlobString(image,buffer);
          (void) WriteBlob(image,packets,compress_pixels);
          break;
        }
        default:
        {
          if (y == 0)
            for (i=0; i < (long) length; i++)
              previous_pixels[i]=(~pixels[i]);
          packets=PCLDeltaCompressImage(length,previous_pixels,pixels,
            compress_pixels);
          (void) FormatMagickString(buffer,MaxTextExtent,"\033*b%luW",
            (unsigned long) packets);
          (void) WriteBlobString(image,buffer);
          (void) WriteBlob(image,packets,compress_pixels);
          (void) CopyMagickMemory(previous_pixels,pixels,length*
            sizeof(*pixels));
          break;
        }
      }
    }
    (void) WriteBlobString(image,"\033*rB");  /* end graphics */
    switch (image->compression)
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

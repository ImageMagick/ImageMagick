/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        IIIII   CCCC   OOO   N   N                           %
%                          I    C      O   O  NN  N                           %
%                          I    C      O   O  N N N                           %
%                          I    C      O   O  N  NN                           %
%                        IIIII   CCCC   OOO   N   N                           %
%                                                                             %
%                                                                             %
%                   Read Microsoft Windows Icon Format                        %
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
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/nt-feature.h"
#include "magick/quantize.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
  Define declarations.
*/
#if !defined(__WINDOWS__) || defined(__MINGW32__)
#define BI_RGB  0
#define BI_RLE8  1
#define BI_BITFIELDS  3
#endif
#define MaxIcons  1024

/*
  Typedef declarations.
*/
typedef struct _IconEntry
{
  unsigned char
    width,
    height,
    colors,
    reserved;

  unsigned short int
    planes,
    bits_per_pixel;

  unsigned long
    size,
    offset;
} IconEntry;

typedef struct _IconFile
{
  short
    reserved,
    resource_type,
    count;

  IconEntry
    directory[MaxIcons];
} IconFile;

typedef struct _IconInfo
{
  unsigned long
    file_size,
    ba_offset,
    offset_bits,
    size;

  long
    width,
    height;

  unsigned short
    planes,
    bits_per_pixel;

  unsigned long
    compression,
    image_size,
    x_pixels,
    y_pixels,
    number_colors,
    red_mask,
    green_mask,
    blue_mask,
    alpha_mask,
    colors_important;

  long
    colorspace;
} IconInfo;

/*
  Forward declaractions.
*/
static MagickBooleanType
  WriteICONImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d I C O N I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadICONImage() reads a Microsoft icon image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadICONImage method is:
%
%      Image *ReadICONImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadICONImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  IconFile
    icon_file;

  IconInfo
    icon_info;

  Image
    *image;

  long
    y;

  MagickBooleanType
    status;

  register IndexPacket
    *indexes;

  register long
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  ssize_t
    count,
    offset;

  unsigned long
    bit,
    byte;

  unsigned long
    bytes_per_line,
    scanline_pad;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"%s",image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  icon_file.reserved=(short) ReadBlobLSBShort(image);
  icon_file.resource_type=(short) ReadBlobLSBShort(image);
  icon_file.count=(short) ReadBlobLSBShort(image);
  if ((icon_file.reserved != 0) ||
      ((icon_file.resource_type != 1) && (icon_file.resource_type != 2)) ||
      (icon_file.count > MaxIcons))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  for (i=0; i < icon_file.count; i++)
  {
    icon_file.directory[i].width=(unsigned char) ReadBlobByte(image);
    icon_file.directory[i].height=(unsigned char) ReadBlobByte(image);
    icon_file.directory[i].colors=(unsigned char) ReadBlobByte(image);
    icon_file.directory[i].reserved=(unsigned char) ReadBlobByte(image);
    icon_file.directory[i].planes=(unsigned short) ReadBlobLSBShort(image);
    icon_file.directory[i].bits_per_pixel=(unsigned short)
      ReadBlobLSBShort(image);
    icon_file.directory[i].size=ReadBlobLSBLong(image);
    icon_file.directory[i].offset=ReadBlobLSBLong(image);
  }
  for (i=0; i < icon_file.count; i++)
  {
    /*
      Verify Icon identifier.
    */
    offset=(ssize_t) SeekBlob(image,(MagickOffsetType)
      icon_file.directory[i].offset,SEEK_SET);
    if (offset < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    icon_info.size=ReadBlobLSBLong(image);
    icon_info.width=(unsigned char) ReadBlobLSBLong(image);
    icon_info.height=(unsigned char) ReadBlobLSBLong(image)/2;
    if ((icon_file.directory[i].width == 0) && 
        (icon_file.directory[i].height == 0))
      {
        Image
          *icon_image;

        ImageInfo
          *read_info;

        size_t
          length;

        unsigned char
          *png;

        /*
          Icon image encoded as a compressed PNG image.
        */
        length=icon_file.directory[i].size;
        png=(unsigned char *) AcquireQuantumMemory(length+12,sizeof(*png));
        if (png == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        (void) CopyMagickMemory(png,"\211PNG\r\n\032\n\000\000\000\015",12);
        count=ReadBlob(image,length,png+12);
        if (count != (ssize_t) length)
          {
            png=(unsigned char *) RelinquishMagickMemory(png);
            ThrowReaderException(CorruptImageError,
              "InsufficientImageDataInFile");
          }
        read_info=CloneImageInfo(image_info);
        (void) CopyMagickString(read_info->magick,"PNG",MaxTextExtent);
        icon_image=BlobToImage(read_info,png,length+12,exception);
        read_info=DestroyImageInfo(read_info);
        png=(unsigned char *) RelinquishMagickMemory(png);
        if (icon_image == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        DestroyBlob(icon_image);
        icon_image->blob=ReferenceBlob(image->blob);
        ReplaceImageInList(&image,icon_image);
      }
    else
      {
        icon_info.planes=ReadBlobLSBShort(image);
        icon_info.bits_per_pixel=ReadBlobLSBShort(image);
        if (icon_info.bits_per_pixel > 32)
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        icon_info.compression=ReadBlobLSBLong(image);
        icon_info.image_size=ReadBlobLSBLong(image);
        icon_info.x_pixels=ReadBlobLSBLong(image);
        icon_info.y_pixels=ReadBlobLSBLong(image);
        icon_info.number_colors=ReadBlobLSBLong(image);
        icon_info.colors_important=ReadBlobLSBLong(image);
        image->matte=MagickTrue;
        image->columns=(unsigned long) icon_file.directory[i].width;
        image->rows=(unsigned long) icon_file.directory[i].height;
        image->depth=8;
        if (image->debug != MagickFalse)
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              " scene    = %ld",i);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   size   = %ld",icon_info.size);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   width  = %d",icon_file.directory[i].width);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   height = %d",icon_file.directory[i].height);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   colors = %ld",icon_info.number_colors);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   planes = %d",icon_info.planes);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   bpp    = %d",icon_info.bits_per_pixel);
          }
      if ((icon_info.number_colors != 0) || (icon_info.bits_per_pixel <= 16))
        {
          image->storage_class=PseudoClass;
          image->colors=icon_info.number_colors;
          if (image->colors == 0)
            image->colors=1 << icon_info.bits_per_pixel;
        }
      if (image->storage_class == PseudoClass)
        {
          register long
            i;

          unsigned char
            *icon_colormap;

          unsigned long
            number_colors;

          /*
            Read Icon raster colormap.
          */
          number_colors=1UL << icon_info.bits_per_pixel;
          if (AcquireImageColormap(image,number_colors) == MagickFalse)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          icon_colormap=(unsigned char *) AcquireQuantumMemory((size_t)
            image->colors,4UL*sizeof(*icon_colormap));
          if (icon_colormap == (unsigned char *) NULL)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          count=ReadBlob(image,(size_t) (4*image->colors),icon_colormap);
          if (count != (ssize_t) (4*image->colors))
            ThrowReaderException(CorruptImageError,
              "InsufficientImageDataInFile");
          p=icon_colormap;
          for (i=0; i < (long) image->colors; i++)
          {
            image->colormap[i].blue=(Quantum) ScaleCharToQuantum(*p++);
            image->colormap[i].green=(Quantum) ScaleCharToQuantum(*p++);
            image->colormap[i].red=(Quantum) ScaleCharToQuantum(*p++);
            p++;
          }
          icon_colormap=(unsigned char *) RelinquishMagickMemory(icon_colormap);
        }
        /*
          Convert Icon raster image to pixel packets.
        */
        if ((image_info->ping != MagickFalse) &&
            (image_info->number_scenes != 0))
          if (image->scene >= (image_info->scene+image_info->number_scenes-1))
            break;
        bytes_per_line=(((image->columns*icon_info.bits_per_pixel)+31) &
          ~31) >> 3;
        scanline_pad=((((image->columns*icon_info.bits_per_pixel)+31) & ~31)-
          (image->columns*icon_info.bits_per_pixel)) >> 3;
        switch (icon_info.bits_per_pixel)
        {
          case 1:
          {
            /*
              Convert bitmap scanline.
            */
            for (y=(long) image->rows-1; y >= 0; y--)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=GetAuthenticIndexQueue(image);
              for (x=0; x < (long) (image->columns-7); x+=8)
              {
                byte=(unsigned long) ReadBlobByte(image);
                for (bit=0; bit < 8; bit++)
                  indexes[x+bit]=(IndexPacket)
                    ((byte & (0x80 >> bit)) != 0 ? 0x01 : 0x00);
              }
              if ((image->columns % 8) != 0)
                {
                  byte=(unsigned long) ReadBlobByte(image);
                  for (bit=0; bit < (image->columns % 8); bit++)
                    indexes[x+bit]=(IndexPacket)
                      ((byte & (0x80 >> bit)) != 0 ? 0x01 : 0x00);
                }
              for (x=0; x < (long) scanline_pad; x++)
                (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,image->rows-y-1,
                    image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
          case 4:
          {
            /*
              Read 4-bit Icon scanline.
            */
            for (y=(long) image->rows-1; y >= 0; y--)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=GetAuthenticIndexQueue(image);
              for (x=0; x < ((long) image->columns-1); x+=2)
              {
                byte=(unsigned long) ReadBlobByte(image);
                indexes[x]=(IndexPacket) ((byte >> 4) & 0xf);
                indexes[x+1]=(IndexPacket) ((byte) & 0xf);
              }
              if ((image->columns % 2) != 0)
                {
                  byte=(unsigned long) ReadBlobByte(image);
                  indexes[x]=(IndexPacket) ((byte >> 4) & 0xf);
                }
              for (x=0; x < (long) scanline_pad; x++)
                (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,image->rows-y-1,
                    image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
          case 8:
          {
            /*
              Convert PseudoColor scanline.
            */
            for (y=(long) image->rows-1; y >= 0; y--)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=GetAuthenticIndexQueue(image);
              for (x=0; x < (long) image->columns; x++)
              {
                byte=(unsigned long) ReadBlobByte(image);
                indexes[x]=(IndexPacket) byte;
              }
              for (x=0; x < (long) scanline_pad; x++)
                (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,image->rows-y-1,
                    image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
          case 16:
          {
            /*
              Convert PseudoColor scanline.
            */
            for (y=(long) image->rows-1; y >= 0; y--)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=GetAuthenticIndexQueue(image);
              for (x=0; x < (long) image->columns; x++)
              {
                byte=(unsigned long) ReadBlobByte(image);
                byte|=(unsigned long) (ReadBlobByte(image) << 8);
                indexes[x]=(IndexPacket) byte;
              }
              for (x=0; x < (long) scanline_pad; x++)
                (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,image->rows-y-1,
                    image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
          case 24:
          case 32:
          {
            /*
              Convert DirectColor scanline.
            */
            for (y=(long) image->rows-1; y >= 0; y--)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                q->blue=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
                q->green=ScaleCharToQuantum((unsigned char)
                  ReadBlobByte(image));
                q->red=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
                if (icon_info.bits_per_pixel == 32)
                  q->opacity=(Quantum) (QuantumRange-ScaleCharToQuantum(
                    (unsigned char) ReadBlobByte(image)));
                q++;
              }
              if (icon_info.bits_per_pixel == 24)
                for (x=0; x < (long) scanline_pad; x++)
                  (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,image->rows-y-1,
                    image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
          default:
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        }
        (void) SyncImage(image);
        if (icon_info.bits_per_pixel != 32)
          {
            /*
              Read the ICON alpha mask.
            */
            image->storage_class=DirectClass;
            for (y=(long) image->rows-1; y >= 0; y--)
            {
              q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              for (x=0; x < ((long) image->columns-7); x+=8)
              {
                byte=(unsigned long) ReadBlobByte(image);
                for (bit=0; bit < 8; bit++)
                  q[x+bit].opacity=(Quantum) (((byte & (0x80 >> bit)) != 0) ?
                    TransparentOpacity : OpaqueOpacity);
              }
              if ((image->columns % 8) != 0)
                {
                  byte=(unsigned long) ReadBlobByte(image);
                  for (bit=0; bit < (image->columns % 8); bit++)
                    q[x+bit].opacity=(Quantum) (((byte & (0x80 >> bit)) != 0) ?
                      TransparentOpacity : OpaqueOpacity);
                }
              for (x=0; x < (long) scanline_pad; x++)
                (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
            }
          }
        if (EOFBlob(image) != MagickFalse)
          {
            ThrowFileException(exception,CorruptImageError,
              "UnexpectedEndOfFile",image->filename);
            break;
          }
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if (i < (long) (icon_file.count-1))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r I C O N I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterICONImage() adds attributes for the Icon image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterICONImage method is:
%
%      unsigned long RegisterICONImage(void)
%
*/
ModuleExport unsigned long RegisterICONImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("CUR");
  entry->decoder=(DecodeImageHandler *) ReadICONImage;
  entry->encoder=(EncodeImageHandler *) WriteICONImage;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Microsoft icon");
  entry->module=ConstantString("CUR");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("ICO");
  entry->decoder=(DecodeImageHandler *) ReadICONImage;
  entry->encoder=(EncodeImageHandler *) WriteICONImage;
  entry->adjoin=MagickTrue;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Microsoft icon");
  entry->module=ConstantString("ICON");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("ICON");
  entry->decoder=(DecodeImageHandler *) ReadICONImage;
  entry->encoder=(EncodeImageHandler *) WriteICONImage;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Microsoft icon");
  entry->module=ConstantString("ICON");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r I C O N I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterICONImage() removes format registrations made by the
%  ICON module from the list of supported formats.
%
%  The format of the UnregisterICONImage method is:
%
%      UnregisterICONImage(void)
%
*/
ModuleExport void UnregisterICONImage(void)
{
  (void) UnregisterMagickInfo("CUR");
  (void) UnregisterMagickInfo("ICO");
  (void) UnregisterMagickInfo("ICON");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e I C O N I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteICONImage() writes an image in Microsoft Windows bitmap encoded
%  image format, version 3 for Windows or (if the image has a matte channel)
%  version 4.
%
%  The format of the WriteICONImage method is:
%
%      MagickBooleanType WriteICONImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteICONImage(const ImageInfo *image_info,
  Image *image)
{
  IconFile
    icon_file;

  IconInfo
    icon_info;

  Image
    *next;
  
  long
    y;

  MagickBooleanType
    status;

  MagickOffsetType
    offset,
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

  unsigned char
    bit,
    byte,
    *pixels;

  unsigned long
    bytes_per_line,
    scanline_pad;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  next=image;
  do
  {
    if ((image->columns > 256L) || (image->rows > 256L))
      ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
    scene++;
    next=SyncNextImageInList(next);
  } while ((next != (Image *) NULL) && (image_info->adjoin != MagickFalse));
  /*
    Dump out a ICON header template to be properly initialized later.
  */
  (void) WriteBlobLSBShort(image,0);
  (void) WriteBlobLSBShort(image,1);
  (void) WriteBlobLSBShort(image,(unsigned char) scene);
  (void) ResetMagickMemory(&icon_file,0,sizeof(icon_file));
  (void) ResetMagickMemory(&icon_info,0,sizeof(icon_info));
  scene=0;
  next=image;
  do
  {
    (void) WriteBlobByte(image,icon_file.directory[scene].width);
    (void) WriteBlobByte(image,icon_file.directory[scene].height);
    (void) WriteBlobByte(image,icon_file.directory[scene].colors);
    (void) WriteBlobByte(image,icon_file.directory[scene].reserved);
    (void) WriteBlobLSBShort(image,icon_file.directory[scene].planes);
    (void) WriteBlobLSBShort(image,icon_file.directory[scene].bits_per_pixel);
    (void) WriteBlobLSBLong(image,icon_file.directory[scene].size);
    (void) WriteBlobLSBLong(image,icon_file.directory[scene].offset);
    scene++;
    next=SyncNextImageInList(next);
  } while ((next != (Image *) NULL) && (image_info->adjoin != MagickFalse));
  scene=0;
  next=image;
  do
  {
    if ((next->columns == 256) && (next->rows == 256))
      {
        Image
          *write_image;

        ImageInfo
          *write_info;

        size_t
          length;

        unsigned char
          *png;

        /*
          Icon image encoded as a compressed PNG image.
        */
        write_image=CloneImage(next,0,0,MagickTrue,&image->exception);
        if (write_image == (Image *) NULL)
          return(MagickFalse);
        write_info=CloneImageInfo(image_info);
        (void) CopyMagickString(write_info->filename,"PNG:",MaxTextExtent);
        png=(unsigned char *) ImageToBlob(write_info,write_image,&length,
          &image->exception);
        write_image=DestroyImage(write_image);
        write_info=DestroyImageInfo(write_info);
        if (png == (unsigned char *) NULL)
          return(MagickFalse);
        icon_file.directory[scene].width=0;
        icon_file.directory[scene].height=0;
        icon_file.directory[scene].colors=0;
        icon_file.directory[scene].reserved=0;
        icon_file.directory[scene].planes=1;
        icon_file.directory[scene].bits_per_pixel=32;
        icon_file.directory[scene].size=(unsigned long) length;
        icon_file.directory[scene].offset=(unsigned long) TellBlob(image);
        (void) WriteBlob(image,(size_t) length,png);
        png=(unsigned char *) RelinquishMagickMemory(png);
      }
    else
      {
        /*
          Initialize ICON raster file header.
        */
        if (next->colorspace != RGBColorspace)
          (void) TransformImageColorspace(next,RGBColorspace);
        icon_info.file_size=14+12+28;
        icon_info.offset_bits=icon_info.file_size;
        icon_info.compression=BI_RGB;
        if ((next->storage_class != DirectClass) && (next->colors > 256))
          (void) SetImageStorageClass(next,DirectClass);
        if (next->storage_class == DirectClass)
          {
            /*
              Full color ICON raster.
            */
            icon_info.number_colors=0;
            icon_info.bits_per_pixel=32;
            icon_info.compression=(unsigned long) BI_RGB;
          }
        else
          {
            /*
              Colormapped ICON raster.
            */
            icon_info.bits_per_pixel=8;
            if (next->colors <= 256)
              icon_info.bits_per_pixel=8;
            if (next->colors <= 16)
              icon_info.bits_per_pixel=4;
            if (next->colors <= 2)
              icon_info.bits_per_pixel=1;
            icon_info.number_colors=1 << icon_info.bits_per_pixel;
            if (icon_info.number_colors < next->colors)
              {
                (void) SetImageStorageClass(next,DirectClass);
                icon_info.number_colors=0;
                icon_info.bits_per_pixel=(unsigned short) 24;
                icon_info.compression=(unsigned long) BI_RGB;
              }
            else
              {
                icon_info.file_size+=3*(1UL << icon_info.bits_per_pixel);
                icon_info.offset_bits+=3*(1UL << icon_info.bits_per_pixel);
                icon_info.file_size+=(1UL << icon_info.bits_per_pixel);
                icon_info.offset_bits+=(1UL << icon_info.bits_per_pixel);
              }
          }
        bytes_per_line=(((next->columns*icon_info.bits_per_pixel)+31) &
          ~31) >> 3;
        icon_info.ba_offset=0;
        icon_info.width=(long) next->columns;
        icon_info.height=(long) next->rows;
        icon_info.planes=1;
        icon_info.image_size=bytes_per_line*next->rows;
        icon_info.size=40;
        icon_info.size+=(4*icon_info.number_colors);
        icon_info.size+=icon_info.image_size;
        icon_info.size+=(((icon_info.width+31) & ~31) >> 3)*icon_info.height;
        icon_info.file_size+=icon_info.image_size;
        icon_info.x_pixels=0;
        icon_info.y_pixels=0;
        switch (next->units)
        {
          case UndefinedResolution:
          case PixelsPerInchResolution:
          {
            icon_info.x_pixels=(unsigned long) (100.0*next->x_resolution/2.54);
            icon_info.y_pixels=(unsigned long) (100.0*next->y_resolution/2.54);
            break;
          }
          case PixelsPerCentimeterResolution:
          {
            icon_info.x_pixels=(unsigned long) (100.0*next->x_resolution);
            icon_info.y_pixels=(unsigned long) (100.0*next->y_resolution);
            break;
          }
        }
        icon_info.colors_important=icon_info.number_colors;
        /*
          Convert MIFF to ICON raster pixels.
        */
        pixels=(unsigned char *) AcquireQuantumMemory((size_t)
          icon_info.image_size,sizeof(*pixels));
        if (pixels == (unsigned char *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        (void) ResetMagickMemory(pixels,0,(size_t) icon_info.image_size);
        switch (icon_info.bits_per_pixel)
        {
          case 1:
          {
            unsigned long
              bit,
              byte;

            /*
              Convert PseudoClass image to a ICON monochrome image.
            */
            for (y=0; y < (long) next->rows; y++)
            {
              p=GetVirtualPixels(next,0,y,next->columns,1,&next->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              indexes=GetVirtualIndexQueue(next);
              q=pixels+(next->rows-y-1)*bytes_per_line;
              bit=0;
              byte=0;
              for (x=0; x < (long) next->columns; x++)
              {
                byte<<=1;
                byte|=indexes[x] != 0 ? 0x01 : 0x00;
                bit++;
                if (bit == 8)
                  {
                    *q++=(unsigned char) byte;
                    bit=0;
                    byte=0;
                  }
               }
              if (bit != 0)
                *q++=(unsigned char) (byte << (8-bit));
              if (next->previous == (Image *) NULL)
                {
                  status=SetImageProgress(next,SaveImageTag,y,next->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
          case 4:
          {
            unsigned long
              nibble,
              byte;

            /*
              Convert PseudoClass image to a ICON monochrome image.
            */
            for (y=0; y < (long) next->rows; y++)
            {
              p=GetVirtualPixels(next,0,y,next->columns,1,&next->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              indexes=GetVirtualIndexQueue(next);
              q=pixels+(next->rows-y-1)*bytes_per_line;
              nibble=0;
              byte=0;
              for (x=0; x < (long) next->columns; x++)
              {
                byte<<=4;
                byte|=((unsigned long) indexes[x] & 0x0f);
                nibble++;
                if (nibble == 2)
                  {
                    *q++=(unsigned char) byte;
                    nibble=0;
                    byte=0;
                  }
               }
              if (nibble != 0)
                *q++=(unsigned char) (byte << 4);
              if (next->previous == (Image *) NULL)
                {
                  status=SetImageProgress(next,SaveImageTag,y,next->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
          case 8:
          {
            /*
              Convert PseudoClass packet to ICON pixel.
            */
            for (y=0; y < (long) next->rows; y++)
            {
              p=GetVirtualPixels(next,0,y,next->columns,1,&next->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              indexes=GetVirtualIndexQueue(next);
              q=pixels+(next->rows-y-1)*bytes_per_line;
              for (x=0; x < (long) next->columns; x++)
                *q++=(unsigned char) indexes[x];
              if (next->previous == (Image *) NULL)
                {
                  status=SetImageProgress(next,SaveImageTag,y,next->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
          case 24:
          case 32:
          {
            /*
              Convert DirectClass packet to ICON BGR888 or BGRA8888 pixel.
            */
            for (y=0; y < (long) next->rows; y++)
            {
              p=GetVirtualPixels(next,0,y,next->columns,1,&next->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              q=pixels+(next->rows-y-1)*bytes_per_line;
              for (x=0; x < (long) next->columns; x++)
              {
                *q++=ScaleQuantumToChar(p->blue);
                *q++=ScaleQuantumToChar(p->green);
                *q++=ScaleQuantumToChar(p->red);
                if (next->matte == MagickFalse)
                  *q++=ScaleQuantumToChar(QuantumRange);
                else
                  *q++=ScaleQuantumToChar(QuantumRange-p->opacity);
                p++;
              }
              if (icon_info.bits_per_pixel == 24)
                for (x=3L*(long) next->columns; x < (long) bytes_per_line; x++)
                  *q++=0x00;
              if (next->previous == (Image *) NULL)
                {
                  status=SetImageProgress(next,SaveImageTag,y,next->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
        }
        /*
          Write 40-byte version 3+ bitmap header.
        */
        icon_file.directory[scene].width=(unsigned char) icon_info.width;
        icon_file.directory[scene].height=(unsigned char) icon_info.height;
        icon_file.directory[scene].colors=(unsigned char)
          icon_info.number_colors;
        icon_file.directory[scene].reserved=0;
        icon_file.directory[scene].planes=icon_info.planes;
        icon_file.directory[scene].bits_per_pixel=icon_info.bits_per_pixel;
        icon_file.directory[scene].size=icon_info.size;
        icon_file.directory[scene].offset=(unsigned long) TellBlob(image);
        (void) WriteBlobLSBLong(image,(unsigned long) 40);
        (void) WriteBlobLSBLong(image,(unsigned long) icon_info.width);
        (void) WriteBlobLSBLong(image,(unsigned long) icon_info.height*2);
        (void) WriteBlobLSBShort(image,icon_info.planes);
        (void) WriteBlobLSBShort(image,icon_info.bits_per_pixel);
        (void) WriteBlobLSBLong(image,icon_info.compression);
        (void) WriteBlobLSBLong(image,icon_info.image_size);
        (void) WriteBlobLSBLong(image,icon_info.x_pixels);
        (void) WriteBlobLSBLong(image,icon_info.y_pixels);
        (void) WriteBlobLSBLong(image,icon_info.number_colors);
        (void) WriteBlobLSBLong(image,icon_info.colors_important);
        if (next->storage_class == PseudoClass)
          {
            unsigned char
              *icon_colormap;

            /*
              Dump colormap to file.
            */
            icon_colormap=(unsigned char *) AcquireQuantumMemory((size_t)
              (1UL << icon_info.bits_per_pixel),4UL*sizeof(*icon_colormap));
            if (icon_colormap == (unsigned char *) NULL)
              ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
            q=icon_colormap;
            for (i=0; i < (long) next->colors; i++)
            {
              *q++=ScaleQuantumToChar(next->colormap[i].blue);
              *q++=ScaleQuantumToChar(next->colormap[i].green);
              *q++=ScaleQuantumToChar(next->colormap[i].red);
              *q++=(unsigned char) 0x0;
            }
            for ( ; i < (long) (1UL << icon_info.bits_per_pixel); i++)
            {
              *q++=(unsigned char) 0x00;
              *q++=(unsigned char) 0x00;
              *q++=(unsigned char) 0x00;
              *q++=(unsigned char) 0x00;
            }
            (void) WriteBlob(image,(size_t) (4UL*(1UL <<
              icon_info.bits_per_pixel)),icon_colormap);
            icon_colormap=(unsigned char *) RelinquishMagickMemory(
              icon_colormap);
          }
        (void) WriteBlob(image,(size_t) icon_info.image_size,pixels);
        pixels=(unsigned char *) RelinquishMagickMemory(pixels);
        /*
          Write matte mask.
        */
        scanline_pad=(((next->columns+31) & ~31)-next->columns) >> 3;
        for (y=((long) next->rows - 1); y >= 0; y--)
        {
          p=GetVirtualPixels(next,0,y,next->columns,1,&next->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          bit=0;
          byte=0;
          for (x=0; x < (long) next->columns; x++)
          {
            byte<<=1;
            if ((next->matte == MagickTrue) &&
                (p->opacity == (Quantum) TransparentOpacity))
              byte|=0x01;
            bit++;
            if (bit == 8)
              {
                (void) WriteBlobByte(image,(unsigned char) byte);
                bit=0;
                byte=0;
              }
            p++;
          }
          if (bit != 0)
            (void) WriteBlobByte(image,(unsigned char) (byte << (8-bit)));
          for (i=0; i < (long) scanline_pad; i++)
            (void) WriteBlobByte(image,(unsigned char) 0);
        }
      }
    if (GetNextImageInList(next) == (Image *) NULL)
      break;
    next=SyncNextImageInList(next);
    status=SetImageProgress(next,SaveImagesTag,scene++,
      GetImageListLength(next));
    if (status == MagickFalse)
      break;
  } while ((next != (Image *) NULL) && (image_info->adjoin != MagickFalse));
  offset=SeekBlob(image,0,SEEK_SET);
  (void) WriteBlobLSBShort(image,0);
  (void) WriteBlobLSBShort(image,1);
  (void) WriteBlobLSBShort(image,(unsigned short) (scene+1));
  scene=0;
  next=image;
  do
  {
    (void) WriteBlobByte(image,icon_file.directory[scene].width);
    (void) WriteBlobByte(image,icon_file.directory[scene].height);
    (void) WriteBlobByte(image,icon_file.directory[scene].colors);
    (void) WriteBlobByte(image,icon_file.directory[scene].reserved);
    (void) WriteBlobLSBShort(image,icon_file.directory[scene].planes);
    (void) WriteBlobLSBShort(image,icon_file.directory[scene].bits_per_pixel);
    (void) WriteBlobLSBLong(image,icon_file.directory[scene].size);
    (void) WriteBlobLSBLong(image,icon_file.directory[scene].offset);
    scene++;
    next=SyncNextImageInList(next);
  } while ((next != (Image *) NULL) && (image_info->adjoin != MagickFalse));
  (void) CloseBlob(image);
  return(MagickTrue);
}

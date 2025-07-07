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
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
  Define declarations.
*/
#define IconRgbCompression (size_t) 0
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

  size_t
    size,
    offset;
} IconEntry;

typedef struct _IconDirectory
{
  size_t
    count;

  IconEntry
    **icons;
} IconDirectory;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteICONImage(const ImageInfo *,Image *,ExceptionInfo *);

static IconDirectory *RelinquishIconDirectory(IconDirectory *directory)
{
  ssize_t
    i;

  assert(directory != (IconDirectory *) NULL);

  if (directory->icons != (IconEntry **) NULL)
    {
      for (i=0; i < (ssize_t) directory->count; i++)
      {
        if (directory->icons[i] != (IconEntry *) NULL)
          directory->icons[i]=(IconEntry *) RelinquishMagickMemory(
            directory->icons[i]);
      }
      directory->icons=(IconEntry **) RelinquishMagickMemory(directory->icons);
    }
  directory=(IconDirectory *) RelinquishMagickMemory(directory);
  return(directory);
}

static IconDirectory *AcquireIconDirectory(size_t count)
{
  IconDirectory
    *directory;

  ssize_t
    i;

  directory=(IconDirectory*) AcquireMagickMemory(sizeof(*directory));
  if (directory == (IconDirectory*) NULL)
    return(directory);
  directory->icons=(IconEntry **) AcquireQuantumMemory(count,
    sizeof(*directory->icons));
  if (directory->icons == (IconEntry **) NULL)
    return(RelinquishIconDirectory(directory));
  memset(directory->icons,0,count*sizeof(*directory->icons));
  for (i=0; i < (ssize_t) count; i++)
  {
    directory->icons[i]=(IconEntry *) AcquireMagickMemory(
      sizeof(**directory->icons));
    if (directory->icons[i] == (IconEntry *) NULL)
      return(RelinquishIconDirectory(directory));
  }
  directory->count=count;
  return(directory);
}

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

static Image *Read1XImage(Image *image,ExceptionInfo *exception)
{
  size_t
    columns,
    rows;

  ssize_t
    i;

  /*
    Read Windows 1.0 Icon.
  */
  (void) ReadBlobLSBLong(image);  /* hot spot X/Y */
  columns=(size_t) ReadBlobLSBShort(image);
  rows=(size_t) (ReadBlobLSBShort(image));
  (void) ReadBlobLSBShort(image);  /* width of bitmap in bytes */
  (void) ReadBlobLSBShort(image);  /* cursor color */
  if (((rows != 32) && (rows != 64)) || ((columns != 32) && (columns != 64)))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
        "ImproperImageHeader","`%s'",image->filename);
      return(DestroyImageList(image));
    }
  /*
    Convert bitmap scanline.
  */
  if (SetImageExtent(image,columns,rows,exception) == MagickFalse)
    return(DestroyImageList(image));
  image->alpha_trait=BlendPixelTrait;
  if (AcquireImageColormap(image,3,exception) == MagickFalse)
    return(DestroyImageList(image));
  image->colormap[1].alpha=TransparentAlpha;
  for (i=0; i < 2; i++)
  {
    ssize_t
      y;

    for (y=0; y < (ssize_t) image->columns; y++)
    {
      Quantum
        *q;

      ssize_t
        x;

      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) (image->columns-7); x+=8)
      {
        size_t
          bit,
          byte;
 
        byte=(size_t) ReadBlobByte(image);
        for (bit=0; bit < 8; bit++)
        {
          Quantum
            index;

          index=(Quantum) ((byte & (0x80 >> bit)) != 0 ? (i == 0 ? 0x01 : 0x02) : 0x00);
          if (i == 0)
            SetPixelIndex(image,index,q);
          else
            if (GetPixelIndex(image,q) != 0x01)
              SetPixelIndex(image,index,q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        break;
    }
  }
  if (SyncImage(image,exception) == MagickFalse)
    return(DestroyImageList(image));
  if (CloseBlob(image) == MagickFalse)
    return(DestroyImageList(image));
  return(image);
}

static inline size_t GetICONSize(size_t directory_size,size_t image_size)
{
  if (image_size != 0)
    return(image_size);
  if (directory_size != 0)
    return(directory_size);
  return(256);
}

static Image *ReadICONImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define ThrowICONReaderException(exception,message) \
{ \
  directory=RelinquishIconDirectory(directory); \
  ThrowReaderException(exception,message) \
}

  IconDirectory
    *directory;

  Image
    *image;

  MagickBooleanType
    status;

  MagickSizeType
    extent;

  Quantum
    *q;

  short
    reserved,
    resource_type;

  size_t
    bit,
    byte,
    bytes_per_line,
    one,
    scanline_pad;

  ssize_t
    i,
    offset,
    x,
    y;

  unsigned char
    *p;

  unsigned short
    icon_count;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),"%s",image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  reserved=(short) ReadBlobLSBShort(image);
  if ((reserved == 0x0001) || (reserved == 0x0101) || (reserved == 0x0201))
    return(Read1XImage(image,exception));
  resource_type=(short) ReadBlobLSBShort(image);
  icon_count=ReadBlobLSBShort(image);
  if ((reserved != 0) || ((resource_type != 1) && (resource_type != 2)) ||
      (icon_count > MaxIcons))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  directory=AcquireIconDirectory((size_t) icon_count);
  if (directory == (IconDirectory *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  extent=0;
  for (i=0; i < (ssize_t) directory->count; i++)
  {
    directory->icons[i]->width=(unsigned char) ReadBlobByte(image);
    directory->icons[i]->height=(unsigned char) ReadBlobByte(image);
    directory->icons[i]->colors=(unsigned char) ReadBlobByte(image);
    directory->icons[i]->reserved=(unsigned char) ReadBlobByte(image);
    directory->icons[i]->planes=(unsigned short) ReadBlobLSBShort(image);
    directory->icons[i]->bits_per_pixel=(unsigned short)
      ReadBlobLSBShort(image);
    directory->icons[i]->size=ReadBlobLSBLong(image);
    directory->icons[i]->offset=ReadBlobLSBLong(image);
    if (EOFBlob(image) != MagickFalse)
      break;
    extent=MagickMax(extent,directory->icons[i]->size);
  }
  if ((EOFBlob(image) != MagickFalse) || (extent > GetBlobSize(image)))
    ThrowICONReaderException(CorruptImageError,"UnexpectedEndOfFile");
  one=1;
  for (i=0; i < (ssize_t) directory->count; i++)
  {
    size_t
      size;

    ssize_t
      count,
      height,
      width;

    unsigned short
      bits_per_pixel,
      planes;

    /*
      Verify Icon identifier.
    */
    offset=(ssize_t) SeekBlob(image,(MagickOffsetType)
      directory->icons[i]->offset,SEEK_SET);
    if (offset < 0)
      ThrowICONReaderException(CorruptImageError,"ImproperImageHeader");
    size=ReadBlobLSBLong(image);
    width=ReadBlobLSBSignedLong(image);
    height=(ReadBlobLSBSignedLong(image)/2);
    planes=ReadBlobLSBShort(image);
    bits_per_pixel=ReadBlobLSBShort(image);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    if (((planes == 18505) && (bits_per_pixel == 21060)) || 
        (size == 0x474e5089))
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
        length=directory->icons[i]->size;
        if ((length < 16) || (~length < 16))
          ThrowICONReaderException(ResourceLimitError,"MemoryAllocationFailed");
        png=(unsigned char *) AcquireQuantumMemory(length,sizeof(*png));
        if (png == (unsigned char *) NULL)
          ThrowICONReaderException(ResourceLimitError,"MemoryAllocationFailed");
        (void) memcpy(png,"\211PNG\r\n\032\n\000\000\000\015",12);
        png[12]=(unsigned char) planes;
        png[13]=(unsigned char) (planes >> 8);
        png[14]=(unsigned char) bits_per_pixel;
        png[15]=(unsigned char) (bits_per_pixel >> 8);
        count=ReadBlob(image,length-16,png+16);
        if (count != (ssize_t) (length-16))
          {
            png=(unsigned char *) RelinquishMagickMemory(png);
            ThrowICONReaderException(CorruptImageError,
                "InsufficientImageDataInFile");
          }
        read_info=CloneImageInfo(image_info);
        (void) CopyMagickString(read_info->magick,"PNG",MagickPathExtent);
        icon_image=BlobToImage(read_info,png,length,exception);
        read_info=DestroyImageInfo(read_info);
        png=(unsigned char *) RelinquishMagickMemory(png);
        if (icon_image == (Image *) NULL)
          return(DestroyImageList(image));
        DestroyBlob(icon_image);
        icon_image->blob=ReferenceBlob(image->blob);
        ReplaceImageInList(&image,icon_image);
        icon_image->scene=(size_t) i;
      }
    else
      {
        size_t
          number_colors;

        if (bits_per_pixel > 32)
          ThrowICONReaderException(CorruptImageError,"ImproperImageHeader");
        (void) ReadBlobLSBLong(image); /* compression */
        (void) ReadBlobLSBLong(image); /* image_size */
        (void) ReadBlobLSBLong(image); /* x_pixels */
        (void) ReadBlobLSBLong(image); /* y_pixels */
        number_colors=ReadBlobLSBLong(image);
        if (number_colors > GetBlobSize(image))
          ThrowICONReaderException(CorruptImageError,
            "InsufficientImageDataInFile");
        (void) ReadBlobLSBLong(image); /* colors_important */
        image->alpha_trait=BlendPixelTrait;
        image->columns=(size_t) GetICONSize( directory->icons[i]->width,
          (size_t) width);
        image->rows=(size_t) GetICONSize(directory->icons[i]->height,
          (size_t) height);
        image->depth=bits_per_pixel;
        if (image->depth > 16)
          image->depth=8;
        if (image->debug != MagickFalse)
          {
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              " scene    = %.20g",(double) i);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   size   = %.20g",(double) size);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   width  = %.20g",(double) image->columns);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   height = %.20g",(double) image->rows);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   colors = %.20g",(double ) number_colors);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   planes = %.20g",(double) planes);
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "   bpp    = %.20g",(double) bits_per_pixel);
          }
      if ((number_colors != 0) || (bits_per_pixel <= 16U))
        {
          image->storage_class=PseudoClass;
          image->colors=number_colors;
          if ((image->colors == 0) || (image->colors > 256))
            image->colors=one << bits_per_pixel;
        }
      if (image->storage_class == PseudoClass)
        {
          ssize_t
            j;

          unsigned char
            *icon_colormap;

          /*
            Read Icon raster colormap.
          */
          if (image->colors > GetBlobSize(image))
            ThrowICONReaderException(CorruptImageError,
              "InsufficientImageDataInFile");
          if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
            ThrowICONReaderException(ResourceLimitError,"MemoryAllocationFailed");
          icon_colormap=(unsigned char *) AcquireQuantumMemory((size_t)
            image->colors,4UL*sizeof(*icon_colormap));
          if (icon_colormap == (unsigned char *) NULL)
            ThrowICONReaderException(ResourceLimitError,"MemoryAllocationFailed");
          count=ReadBlob(image,(size_t) (4*image->colors),icon_colormap);
          if (count != (ssize_t) (4*image->colors))
            {
              icon_colormap=(unsigned char *) RelinquishMagickMemory(
                icon_colormap);
              ThrowICONReaderException(CorruptImageError,
                "InsufficientImageDataInFile");
            }
          p=icon_colormap;
          for (j=0; j < (ssize_t) image->colors; j++)
          {
            image->colormap[j].blue=(Quantum) ScaleCharToQuantum(*p++);
            image->colormap[j].green=(Quantum) ScaleCharToQuantum(*p++);
            image->colormap[j].red=(Quantum) ScaleCharToQuantum(*p++);
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
        status=SetImageExtent(image,image->columns,image->rows,exception);
        if (status == MagickFalse)
          return(DestroyImageList(image));
        bytes_per_line=(((image->columns*bits_per_pixel)+31U) & ~31U) >> 3;
        (void) bytes_per_line;
        scanline_pad=((((image->columns*bits_per_pixel)+31U) & ~31U)-
          (image->columns*bits_per_pixel)) >> 3;
        switch (bits_per_pixel)
        {
          case 1:
          {
            /*
              Convert bitmap scanline.
            */
            for (y=(ssize_t) image->rows-1; y >= 0; y--)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) (image->columns-7); x+=8)
              {
                byte=(size_t) ReadBlobByte(image);
                for (bit=0; bit < 8; bit++)
                {
                  SetPixelIndex(image,(Quantum) ((byte & (0x80 >> bit)) != 0 ?
                    0x01 : 0x00),q);
                  q+=(ptrdiff_t) GetPixelChannels(image);
                }
              }
              if ((image->columns % 8) != 0)
                {
                  byte=(size_t) ReadBlobByte(image);
                  for (bit=0; bit < (image->columns % 8); bit++)
                  {
                    SetPixelIndex(image,(Quantum) ((byte & (0x80 >> bit)) != 0 ?
                      0x01 : 0x00),q);
                    q+=(ptrdiff_t) GetPixelChannels(image);
                  }
                }
              for (x=0; x < (ssize_t) scanline_pad; x++)
                (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                    image->rows-y-1,(MagickSizeType) image->rows);
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
            for (y=(ssize_t) image->rows-1; y >= 0; y--)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < ((ssize_t) image->columns-1); x+=2)
              {
                byte=(size_t) ReadBlobByte(image);
                SetPixelIndex(image,(Quantum) ((byte >> 4) & 0xf),q);
                q+=(ptrdiff_t) GetPixelChannels(image);
                SetPixelIndex(image,(Quantum) ((byte) & 0xf),q);
                q+=(ptrdiff_t) GetPixelChannels(image);
              }
              if ((image->columns % 2) != 0)
                {
                  byte=(size_t) ReadBlobByte(image);
                  SetPixelIndex(image,(Quantum) ((byte >> 4) & 0xf),q);
                  q+=(ptrdiff_t) GetPixelChannels(image);
                }
              for (x=0; x < (ssize_t) scanline_pad; x++)
                (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                    image->rows-y-1,(MagickSizeType) image->rows);
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
            for (y=(ssize_t) image->rows-1; y >= 0; y--)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                byte=(size_t) ReadBlobByte(image);
                SetPixelIndex(image,(Quantum) byte,q);
                q+=(ptrdiff_t) GetPixelChannels(image);
              }
              for (x=0; x < (ssize_t) scanline_pad; x++)
                (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                    image->rows-y-1,(MagickSizeType) image->rows);
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
            for (y=(ssize_t) image->rows-1; y >= 0; y--)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                byte=(size_t) ReadBlobByte(image);
                byte|=((size_t) ReadBlobByte(image) << 8);
                SetPixelIndex(image,(Quantum) byte,q);
                q+=(ptrdiff_t) GetPixelChannels(image);
              }
              for (x=0; x < (ssize_t) scanline_pad; x++)
                (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                    image->rows-y-1,(MagickSizeType) image->rows);
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
            for (y=(ssize_t) image->rows-1; y >= 0; y--)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
                  ReadBlobByte(image)),q);
                SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
                  ReadBlobByte(image)),q);
                SetPixelRed(image,ScaleCharToQuantum((unsigned char)
                  ReadBlobByte(image)),q);
                if (bits_per_pixel == 32)
                  SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
                    ReadBlobByte(image)),q);
                q+=(ptrdiff_t) GetPixelChannels(image);
              }
              if (bits_per_pixel == 24)
                for (x=0; x < (ssize_t) scanline_pad; x++)
                  (void) ReadBlobByte(image);
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                    image->rows-y-1,(MagickSizeType) image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            break;
          }
          default:
            ThrowICONReaderException(CorruptImageError,"ImproperImageHeader");
        }
        if ((image_info->ping == MagickFalse) && (bits_per_pixel <= 16))
          (void) SyncImage(image,exception);
        if (bits_per_pixel != 32)
          {
            /*
              Read the ICON alpha mask.
            */
            image->storage_class=DirectClass;
            for (y=(ssize_t) image->rows-1; y >= 0; y--)
            {
              q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < ((ssize_t) image->columns-7); x+=8)
              {
                byte=(size_t) ReadBlobByte(image);
                for (bit=0; bit < 8; bit++)
                {
                  SetPixelAlpha(image,(((byte & (0x80 >> bit)) != 0) ?
                    TransparentAlpha : OpaqueAlpha),q);
                  q+=(ptrdiff_t) GetPixelChannels(image);
                }
              }
              if ((image->columns % 8) != 0)
                {
                  byte=(size_t) ReadBlobByte(image);
                  for (bit=0; bit < (image->columns % 8); bit++)
                  {
                    SetPixelAlpha(image,(((byte & (0x80 >> bit)) != 0) ?
                      TransparentAlpha : OpaqueAlpha),q);
                    q+=(ptrdiff_t) GetPixelChannels(image);
                  }
                }
              if ((image->columns % 32) != 0)
                for (x=0; x < (ssize_t) ((32-(image->columns % 32))/8); x++)
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
    if (i < ((ssize_t) directory->count-1))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            status=MagickFalse;
            break;
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  }
  directory=RelinquishIconDirectory(directory);
  (void) CloseBlob(image);
  if (status == MagickFalse)
    return(DestroyImageList(image));
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
%      size_t RegisterICONImage(void)
%
*/
ModuleExport size_t RegisterICONImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("ICON","CUR","Microsoft icon");
  entry->decoder=(DecodeImageHandler *) ReadICONImage;
  entry->encoder=(EncodeImageHandler *) WriteICONImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("ICON","ICO","Microsoft icon");
  entry->decoder=(DecodeImageHandler *) ReadICONImage;
  entry->encoder=(EncodeImageHandler *) WriteICONImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("ICON","ICN","Microsoft icon");
  entry->decoder=(DecodeImageHandler *) ReadICONImage;
  entry->encoder=(EncodeImageHandler *) WriteICONImage;
  entry->flags ^= CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("ICON","ICON","Microsoft icon");
  entry->decoder=(DecodeImageHandler *) ReadICONImage;
  entry->encoder=(EncodeImageHandler *) WriteICONImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
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
%  It encodes any subimage as a compressed PNG image ("BI_PNG)", only when its
%  dimensions are 256x256 and image->compression is undefined or is defined as
%  ZipCompression.
%
%  The format of the WriteICONImage method is:
%
%      MagickBooleanType WriteICONImage(const ImageInfo *image_info,
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

static Image *AutoResizeImage(const Image *image,const char *option,
  MagickOffsetType *count,ExceptionInfo *exception)
{
#define MAX_SIZES 16

  char
    *q;

  const char
    *p;

  Image
    *images,
    *resized;

  size_t
    sizes[MAX_SIZES] = { 256, 192, 128, 96, 64, 48, 40, 32, 24, 16 };

  ssize_t
    i;

  images=NULL;
  *count=0;
  i=0;
  p=option;
  while ((*p != '\0') && (i < MAX_SIZES))
  {
    size_t
      size;

    while ((isspace((int) ((unsigned char) *p)) != 0))
      p++;
    size=(size_t) strtol(p,&q,10);
    if ((p == q) || (size < 16) || (size > 512))
      return((Image *) NULL);
    p=q;
    sizes[i++]=size;
    while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == ','))
      p++;
  }
  if (i == 0)
    i=10; /* the number of sizes when they are not specified by the user */
  *count=i;
  for (i=0; i < *count; i++)
  {
    resized=ResizeImage(image,sizes[i],sizes[i],image->filter,exception);
    if (resized == (Image *) NULL)
      return(DestroyImageList(images));
    if (images == (Image *) NULL)
      images=resized;
    else
      AppendImageToList(&images,resized);
  }
  return(images);
}

static MagickBooleanType WriteICONImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
#define ThrowICONWriterException(exception,message) \
{ \
  directory=RelinquishIconDirectory(directory); \
  images=DestroyImageList(images); \
  ThrowWriterException(exception,message) \
}

  const char
    *option;

  const Quantum
    *p;

  IconDirectory
    *directory;

  Image
    *images,
    *next;
  
  MagickBooleanType
    adjoin,
    status;

  MagickOffsetType
    offset,
    scene;

  size_t
    bytes_per_line,
    number_scenes,
    scanline_pad;

  ssize_t
    i,
    x,
    y;

  unsigned char
    *pixels,
    *q;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  images=(Image *) NULL;
  adjoin=image_info->adjoin;
  option=GetImageOption(image_info,"icon:auto-resize");
  if (option != (const char *) NULL)
    {
      images=AutoResizeImage(image,option,&scene,exception);
      if (images == (Image *) NULL)
        ThrowWriterException(ImageError,"InvalidDimensions");
      adjoin=MagickTrue;
    }
  else
    {
      scene=0;
      next=image;
      do
      {
        if ((image->columns > 512L) || (image->rows > 512L))
          ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
        scene++;
        next=SyncNextImageInList(next);
      } while ((next != (Image *) NULL) && (adjoin != MagickFalse));
    }
  /*
    Dump out a ICON header template to be properly initialized later.
  */
  (void) WriteBlobLSBShort(image,0);
  (void) WriteBlobLSBShort(image,1);
  (void) WriteBlobLSBShort(image,(unsigned char) scene);
  next=(images != (Image *) NULL) ? images : image;
  number_scenes=0;
  do
  {
    number_scenes++;
    (void) WriteBlobByte(image,0); /* width */
    (void) WriteBlobByte(image,0); /* height */
    (void) WriteBlobByte(image,0); /* colors */
    (void) WriteBlobByte(image,0); /* reserved */
    (void) WriteBlobLSBShort(image,0); /* planes */
    (void) WriteBlobLSBShort(image,0); /* bits_per_pixel */
    (void) WriteBlobLSBLong(image,0); /* size */
    (void) WriteBlobLSBLong(image,0); /* offset */
    next=SyncNextImageInList(next);
  } while ((next != (Image *) NULL) && (adjoin != MagickFalse));
  scene=0;
  next=(images != (Image *) NULL) ? images : image;
  directory=AcquireIconDirectory(number_scenes);
  if (directory == (IconDirectory *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  do
  {
    size_t
      size;

    unsigned short
      bits_per_pixel,
      planes;

    if ((next->columns > 255L) && (next->rows > 255L) &&
        ((next->compression == UndefinedCompression) ||
        (next->compression == ZipCompression)))
      {
        Image
          *write_image;

        ImageInfo
          *write_info;

        size_t
          length;

        unsigned char
          *png;

        write_image=CloneImage(next,0,0,MagickTrue,exception);
        if (write_image == (Image *) NULL)
          {
            directory=RelinquishIconDirectory(directory);
            images=DestroyImageList(images);
            return(MagickFalse);
          }
        write_info=CloneImageInfo(image_info);
        length=0;
        /*
          Don't write any ancillary chunks except for gAMA,tRNS.
        */
        (void) SetImageArtifact(write_image,"png:include-chunk",
          "none,gama,tRNS");
        /*
          Only write PNG32 formatted PNG (32-bit RGBA), 8 bits per channel.
        */
        (void) CopyMagickString(write_info->magick,"PNG32",MagickPathExtent);
        png=(unsigned char *) ImageToBlob(write_info,write_image,&length,
          exception);
        write_image=DestroyImageList(write_image);
        write_info=DestroyImageInfo(write_info);
        if (png == (unsigned char *) NULL)
          {
            directory=RelinquishIconDirectory(directory);
            images=DestroyImageList(images);
            return(MagickFalse);
          }
        directory->icons[scene]->width=0;
        directory->icons[scene]->height=0;
        directory->icons[scene]->colors=0;
        directory->icons[scene]->reserved=0;
        directory->icons[scene]->planes=1;
        directory->icons[scene]->bits_per_pixel=32;
        directory->icons[scene]->size=(size_t) length;
        directory->icons[scene]->offset=(size_t) TellBlob(image);
        (void) WriteBlob(image,(size_t) length,png);
        png=(unsigned char *) RelinquishMagickMemory(png);
      }
    else
      {
        size_t
          image_size,
          number_colors,
          x_pixels,
          y_pixels;

        ssize_t
          width,
          height;

        /*
          Initialize ICON raster file header.
        */
        (void) TransformImageColorspace(next,sRGBColorspace,exception);
        if ((next->storage_class != DirectClass) && (next->colors > 256))
          (void) SetImageStorageClass(next,DirectClass,exception);
        if (next->storage_class == DirectClass)
          {
            /*
              Full color ICON raster.
            */
            number_colors=0;
            bits_per_pixel=32;
          }
        else
          {
            /*
              Colormapped ICON raster.
            */
            bits_per_pixel=8;
            if (next->colors <= 16)
              bits_per_pixel=4;
            if (next->colors <= 2)
              bits_per_pixel=1;
            number_colors=(size_t) 1 << bits_per_pixel;
            if (number_colors < next->colors)
              {
                (void) SetImageStorageClass(next,DirectClass,exception);
                number_colors=0;
                bits_per_pixel=(unsigned short) 24;
              }
          }
        bytes_per_line=(((next->columns*bits_per_pixel)+31U) &
          ~31U) >> 3;
        width=(ssize_t) next->columns;
        height=(ssize_t) next->rows;
        planes=1;
        image_size=bytes_per_line*next->rows;
        size=40;
        size+=(4*number_colors);
        size+=image_size;
        size+=(size_t) ((((width+31U) & ~31U) >> 3)*height);
        x_pixels=0;
        y_pixels=0;
        switch (next->units)
        {
          case UndefinedResolution:
          case PixelsPerInchResolution:
          {
            x_pixels=(size_t) (100.0*next->resolution.x/2.54);
            y_pixels=(size_t) (100.0*next->resolution.y/2.54);
            break;
          }
          case PixelsPerCentimeterResolution:
          {
            x_pixels=(size_t) (100.0*next->resolution.x);
            y_pixels=(size_t) (100.0*next->resolution.y);
            break;
          }
        }
        /*
          Convert MIFF to ICON raster pixels.
        */
        pixels=(unsigned char *) AcquireQuantumMemory(image_size,
          sizeof(*pixels));
        if (pixels == (unsigned char *) NULL)
          ThrowICONWriterException(ResourceLimitError,"MemoryAllocationFailed");
        (void) memset(pixels,0,image_size);
        switch (bits_per_pixel)
        {
          case 1:
          {
            size_t
              bit,
              byte;

            /*
              Convert PseudoClass image to a ICON monochrome image.
            */
            for (y=0; y < (ssize_t) next->rows; y++)
            {
              p=GetVirtualPixels(next,0,y,next->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              q=pixels+((ssize_t) next->rows-y-1)*(ssize_t) bytes_per_line;
              bit=0;
              byte=0;
              for (x=0; x < (ssize_t) next->columns; x++)
              {
                byte<<=1;
                byte|=(size_t) (GetPixelIndex(next,p) != 0 ? 0x01 : 0x00);
                bit++;
                if (bit == 8)
                  {
                    *q++=(unsigned char) byte;
                    bit=0;
                    byte=0;
                  }
                p+=(ptrdiff_t) GetPixelChannels(next);
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
            size_t
              nibble,
              byte;

            /*
              Convert PseudoClass image to a ICON monochrome image.
            */
            for (y=0; y < (ssize_t) next->rows; y++)
            {
              p=GetVirtualPixels(next,0,y,next->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              q=pixels+((ssize_t) next->rows-y-1)*(ssize_t) bytes_per_line;
              nibble=0;
              byte=0;
              for (x=0; x < (ssize_t) next->columns; x++)
              {
                byte<<=4;
                byte|=((size_t) GetPixelIndex(next,p) & 0x0f);
                nibble++;
                if (nibble == 2)
                  {
                    *q++=(unsigned char) byte;
                    nibble=0;
                    byte=0;
                  }
                p+=(ptrdiff_t) GetPixelChannels(next);
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
            for (y=0; y < (ssize_t) next->rows; y++)
            {
              p=GetVirtualPixels(next,0,y,next->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              q=pixels+((ssize_t) next->rows-y-1)*(ssize_t) bytes_per_line;
              for (x=0; x < (ssize_t) next->columns; x++)
              {
                *q++=(unsigned char) GetPixelIndex(next,p);
                p+=(ptrdiff_t) GetPixelChannels(next);
              }
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
            for (y=0; y < (ssize_t) next->rows; y++)
            {
              p=GetVirtualPixels(next,0,y,next->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              q=pixels+((ssize_t) next->rows-y-1)*(ssize_t) bytes_per_line;
              for (x=0; x < (ssize_t) next->columns; x++)
              {
                *q++=ScaleQuantumToChar(GetPixelBlue(next,p));
                *q++=ScaleQuantumToChar(GetPixelGreen(next,p));
                *q++=ScaleQuantumToChar(GetPixelRed(next,p));
                if (next->alpha_trait == UndefinedPixelTrait)
                  *q++=ScaleQuantumToChar(QuantumRange);
                else
                  *q++=ScaleQuantumToChar(GetPixelAlpha(next,p));
                p+=(ptrdiff_t) GetPixelChannels(next);
              }
              if (bits_per_pixel == 24)
                for (x=3L*(ssize_t) next->columns; x < (ssize_t) bytes_per_line; x++)
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
        directory->icons[scene]->width=(unsigned char) width;
        directory->icons[scene]->height=(unsigned char) height;
        directory->icons[scene]->colors=(unsigned char) number_colors;
        directory->icons[scene]->reserved=0;
        directory->icons[scene]->planes=planes;
        directory->icons[scene]->bits_per_pixel=bits_per_pixel;
        directory->icons[scene]->size=size;
        directory->icons[scene]->offset=(size_t) TellBlob(image);
        (void) WriteBlobLSBLong(image,(unsigned int) 40);
        (void) WriteBlobLSBLong(image,(unsigned int) width);
        (void) WriteBlobLSBLong(image,(unsigned int) height*2);
        (void) WriteBlobLSBShort(image,planes);
        (void) WriteBlobLSBShort(image,bits_per_pixel);
        (void) WriteBlobLSBLong(image,(unsigned int) IconRgbCompression);
        (void) WriteBlobLSBLong(image,(unsigned int) image_size);
        (void) WriteBlobLSBLong(image,(unsigned int) x_pixels);
        (void) WriteBlobLSBLong(image,(unsigned int) y_pixels);
        (void) WriteBlobLSBLong(image,(unsigned int) number_colors);
        (void) WriteBlobLSBLong(image,(unsigned int) number_colors);
        if (next->storage_class == PseudoClass)
          {
            unsigned char
              *icon_colormap;

            /*
              Dump colormap to file.
            */
            icon_colormap=(unsigned char *) AcquireQuantumMemory((size_t) 1UL
              << bits_per_pixel,4UL*sizeof(*icon_colormap));
            if (icon_colormap == (unsigned char *) NULL)
              ThrowICONWriterException(ResourceLimitError,
                "MemoryAllocationFailed");
            q=icon_colormap;
            for (i=0; i < (ssize_t) next->colors; i++)
            {
              *q++=ScaleQuantumToChar((Quantum) next->colormap[i].blue);
              *q++=ScaleQuantumToChar((Quantum) next->colormap[i].green);
              *q++=ScaleQuantumToChar((Quantum) next->colormap[i].red);
              *q++=(unsigned char) 0x00;
            }
            for ( ; i < (ssize_t) 1UL << bits_per_pixel; i++)
            {
              *q++=(unsigned char) 0x00;
              *q++=(unsigned char) 0x00;
              *q++=(unsigned char) 0x00;
              *q++=(unsigned char) 0x00;
            }
            (void) WriteBlob(image,(size_t) (4UL*(1UL << bits_per_pixel)),
              icon_colormap);
            icon_colormap=(unsigned char *) RelinquishMagickMemory(
              icon_colormap);
          }
        (void) WriteBlob(image,image_size,pixels);
        pixels=(unsigned char *) RelinquishMagickMemory(pixels);
        /*
          Write matte mask.
        */
        scanline_pad=(((next->columns+31U) & ~31U)-next->columns) >> 3;
        for (y=((ssize_t) next->rows - 1); y >= 0; y--)
        {
          unsigned char
            bit,
            byte;

          p=GetVirtualPixels(next,0,y,next->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          bit=0;
          byte=0;
          for (x=0; x < (ssize_t) next->columns; x++)
          {
            byte<<=1;
            if ((next->alpha_trait != UndefinedPixelTrait) &&
                (GetPixelAlpha(next,p) == (Quantum) TransparentAlpha))
              byte|=0x01;
            bit++;
            if (bit == 8)
              {
                (void) WriteBlobByte(image,(unsigned char) byte);
                bit=0;
                byte=0;
              }
            p+=(ptrdiff_t) GetPixelChannels(next);
          }
          if (bit != 0)
            (void) WriteBlobByte(image,(unsigned char) (byte << (8-bit)));
          for (i=0; i < (ssize_t) scanline_pad; i++)
            (void) WriteBlobByte(image,(unsigned char) 0);
        }
      }
    if (GetNextImageInList(next) == (Image *) NULL)
      break;
    status=SetImageProgress(next,SaveImagesTag,scene++,number_scenes);
    if (status == MagickFalse)
      break;
    next=SyncNextImageInList(next);
  } while ((next != (Image *) NULL) && (adjoin != MagickFalse));
  offset=SeekBlob(image,0,SEEK_SET);
  (void) offset;
  (void) WriteBlobLSBShort(image,0);
  (void) WriteBlobLSBShort(image,1);
  (void) WriteBlobLSBShort(image,(unsigned short) (scene+1));
  scene=0;
  next=(images != (Image *) NULL) ? images : image;
  do
  {
    (void) WriteBlobByte(image,directory->icons[scene]->width);
    (void) WriteBlobByte(image,directory->icons[scene]->height);
    (void) WriteBlobByte(image,directory->icons[scene]->colors);
    (void) WriteBlobByte(image,directory->icons[scene]->reserved);
    (void) WriteBlobLSBShort(image,directory->icons[scene]->planes);
    (void) WriteBlobLSBShort(image,directory->icons[scene]->bits_per_pixel);
    (void) WriteBlobLSBLong(image,(unsigned int)
      directory->icons[scene]->size);
    (void) WriteBlobLSBLong(image,(unsigned int)
      directory->icons[scene]->offset);
    scene++;
    next=SyncNextImageInList(next);
  } while ((next != (Image *) NULL) && (adjoin != MagickFalse));
  directory=RelinquishIconDirectory(directory);
  (void) CloseBlob(image);
  images=DestroyImageList(images);
  return(MagickTrue);
}

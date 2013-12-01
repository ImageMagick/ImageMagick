/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            SSSSS   GGGG  IIIII                              %
%                            SS     G        I                                %
%                             SSS   G  GG    I                                %
%                               SS  G   G    I                                %
%                            SSSSS   GGG   IIIII                              %
%                                                                             %
%                                                                             %
%                      Read/Write Irix RGB Image Format                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
  Typedef declaractions.
*/
typedef struct _SGIInfo
{
  unsigned short
    magic;

  unsigned char
    storage,
    bytes_per_pixel;

  unsigned short
    dimension,
    columns,
    rows,
    depth;

  size_t
    minimum_value,
    maximum_value,
    sans;

  char
    name[80];

  size_t
    pixel_format;

  unsigned char
    filler[404];
} SGIInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteSGIImage(const ImageInfo *,Image *);
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s S G I                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsSGI() returns MagickTrue if the image format type, identified by the
%  magick string, is SGI.
%
%  The format of the IsSGI method is:
%
%      MagickBooleanType IsSGI(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsSGI(const unsigned char *magick,const size_t length)
{
  if (length < 2)
    return(MagickFalse);
  if (memcmp(magick,"\001\332",2) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S G I I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadSGIImage() reads a SGI RGB image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadSGIImage method is:
%
%      Image *ReadSGIImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static MagickBooleanType SGIDecode(const size_t bytes_per_pixel,
  ssize_t number_packets,unsigned char *packets,ssize_t number_pixels,
  unsigned char *pixels)
{
  register unsigned char
    *p,
    *q;

  size_t
    pixel;

  ssize_t
    count;

  p=packets;
  q=pixels;
  if (bytes_per_pixel == 2)
    {
      for ( ; number_pixels > 0; )
      {
        if (number_packets-- == 0)
          return(MagickFalse);
        pixel=(size_t) (*p++) << 8;
        pixel|=(*p++);
        count=(ssize_t) (pixel & 0x7f);
        if (count == 0)
          break;
        if (count > (ssize_t) number_pixels)
          return(MagickFalse);
        number_pixels-=count;
        if ((pixel & 0x80) != 0)
          for ( ; count != 0; count--)
          {
            if (number_packets-- == 0)
              return(MagickFalse);
            *q=(*p++);
            *(q+1)=(*p++);
            q+=8;
          }
        else
          {
            pixel=(size_t) (*p++) << 8;
            pixel|=(*p++);
            for ( ; count != 0; count--)
            {
              if (number_packets-- == 0)
                return(MagickFalse);
              *q=(unsigned char) (pixel >> 8);
              *(q+1)=(unsigned char) pixel;
              q+=8;
            }
          }
      }
      return(MagickTrue);
    }
  for ( ; number_pixels > 0; )
  {
    if (number_packets-- == 0)
      return(MagickFalse);
    pixel=(size_t) (*p++);
    count=(ssize_t) (pixel & 0x7f);
    if (count == 0)
      break;
    if (count > (ssize_t) number_pixels)
      return(MagickFalse);
    number_pixels-=count;
    if ((pixel & 0x80) != 0)
      for ( ; count != 0; count--)
      {
        if (number_packets-- == 0)
          return(MagickFalse);
        *q=(*p++);
        q+=4;
      }
    else
      {
        if (number_packets-- == 0)
          return(MagickFalse);
        pixel=(size_t) (*p++);
        for ( ; count != 0; count--)
        {
          *q=(unsigned char) pixel;
          q+=4;
        }
      }
  }
  return(MagickTrue);
}

static Image *ReadSGIImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  MemoryInfo
    *pixel_info;

  register IndexPacket
    *indexes;

  register PixelPacket
    *q;

  register ssize_t
    i,
    x;

  register unsigned char
    *p;

  SGIInfo
    iris_info;

  size_t
    bytes_per_pixel,
    quantum;

  ssize_t
    count,
    y,
    z;

  unsigned char
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
    Read SGI raster header.
  */
  iris_info.magic=ReadBlobMSBShort(image);
  do
  {
    /*
      Verify SGI identifier.
    */
    if (iris_info.magic != 0x01DA)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.storage=(unsigned char) ReadBlobByte(image);
    switch (iris_info.storage)
    {
      case 0x00: image->compression=NoCompression; break;
      case 0x01: image->compression=RLECompression; break;
      default:
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
    iris_info.bytes_per_pixel=(unsigned char) ReadBlobByte(image);
    if ((iris_info.bytes_per_pixel == 0) || (iris_info.bytes_per_pixel > 2))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.dimension=ReadBlobMSBShort(image);
    iris_info.columns=ReadBlobMSBShort(image);
    iris_info.rows=ReadBlobMSBShort(image);
    iris_info.depth=ReadBlobMSBShort(image);
    if ((iris_info.depth == 0) || (iris_info.depth > 4))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.minimum_value=ReadBlobMSBLong(image);
    iris_info.maximum_value=ReadBlobMSBLong(image);
    iris_info.sans=ReadBlobMSBLong(image);
    (void) ReadBlob(image,sizeof(iris_info.name),(unsigned char *)
      iris_info.name);
    iris_info.name[sizeof(iris_info.name)-1]='\0';
    if (*iris_info.name != '\0')
      (void) SetImageProperty(image,"label",iris_info.name);
    iris_info.pixel_format=ReadBlobMSBLong(image);
    if (iris_info.pixel_format != 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    count=ReadBlob(image,sizeof(iris_info.filler),iris_info.filler);
    (void) count;
    image->columns=iris_info.columns;
    image->rows=iris_info.rows;
    image->depth=(size_t) MagickMin(iris_info.depth,MAGICKCORE_QUANTUM_DEPTH);
    if (iris_info.pixel_format == 0)
      image->depth=(size_t) MagickMin((size_t) 8*
        iris_info.bytes_per_pixel,MAGICKCORE_QUANTUM_DEPTH);
    if (iris_info.depth < 3)
      {
        image->storage_class=PseudoClass;
        image->colors=iris_info.bytes_per_pixel > 1 ? 65535 : 256;
      }
    if ((image_info->ping != MagickFalse)  && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    /*
      Allocate SGI pixels.
    */
    bytes_per_pixel=(size_t) iris_info.bytes_per_pixel;
    number_pixels=(MagickSizeType) iris_info.columns*iris_info.rows;
    if ((4*bytes_per_pixel*number_pixels) != ((MagickSizeType) (size_t)
        (4*bytes_per_pixel*number_pixels)))
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    pixel_info=AcquireVirtualMemory(iris_info.columns,iris_info.rows*4*
      bytes_per_pixel*sizeof(*pixels));
    if (pixel_info == (MemoryInfo *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    if ((int) iris_info.storage != 0x01)
      {
        unsigned char
          *scanline;

        /*
          Read standard image format.
        */
        scanline=(unsigned char *) AcquireQuantumMemory(iris_info.columns,
          bytes_per_pixel*sizeof(*scanline));
        if (scanline == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        for (z=0; z < (ssize_t) iris_info.depth; z++)
        {
          p=pixels+bytes_per_pixel*z;
          for (y=0; y < (ssize_t) iris_info.rows; y++)
          {
            count=ReadBlob(image,bytes_per_pixel*iris_info.columns,scanline);
            if (EOFBlob(image) != MagickFalse)
              break;
            if (bytes_per_pixel == 2)
              for (x=0; x < (ssize_t) iris_info.columns; x++)
              {
                *p=scanline[2*x];
                *(p+1)=scanline[2*x+1];
                p+=8;
              }
            else
              for (x=0; x < (ssize_t) iris_info.columns; x++)
              {
                *p=scanline[x];
                p+=4;
              }
          }
        }
        scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      }
    else
      {
        MemoryInfo
          *packet_info;

        size_t
          *runlength;

        ssize_t
          offset,
          *offsets;

        unsigned char
          *packets;

        unsigned int
          data_order;

        /*
          Read runlength-encoded image format.
        */
        offsets=(ssize_t *) AcquireQuantumMemory((size_t) iris_info.rows,
          iris_info.depth*sizeof(*offsets));
        runlength=(size_t *) AcquireQuantumMemory(iris_info.rows,
          iris_info.depth*sizeof(*runlength));
        packet_info=AcquireVirtualMemory((size_t) iris_info.columns+10UL,4UL*
          sizeof(*packets));
        if ((offsets == (ssize_t *) NULL) ||
            (runlength == (size_t *) NULL) ||
            (packet_info == (MemoryInfo *) NULL))
          {
            if (offsets == (ssize_t *) NULL)
              offsets=(ssize_t *) RelinquishMagickMemory(offsets);
            if (runlength == (size_t *) NULL)
              runlength=(size_t *) RelinquishMagickMemory(runlength);
            if (packet_info == (MemoryInfo *) NULL)
              packet_info=RelinquishVirtualMemory(packet_info);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        packets=(unsigned char *) GetVirtualMemoryBlob(packet_info);
        for (i=0; i < (ssize_t) (iris_info.rows*iris_info.depth); i++)
          offsets[i]=(int) ReadBlobMSBLong(image);
        for (i=0; i < (ssize_t) (iris_info.rows*iris_info.depth); i++)
        {
          runlength[i]=ReadBlobMSBLong(image);
          if (runlength[i] > (4*(size_t) iris_info.columns+10))
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        }
        /*
          Check data order.
        */
        offset=0;
        data_order=0;
        for (y=0; ((y < (ssize_t) iris_info.rows) && (data_order == 0)); y++)
          for (z=0; ((z < (ssize_t) iris_info.depth) && (data_order == 0)); z++)
          {
            if (offsets[y+z*iris_info.rows] < offset)
              data_order=1;
            offset=offsets[y+z*iris_info.rows];
          }
        offset=(ssize_t) TellBlob(image);
        if (data_order == 1)
          {
            for (z=0; z < (ssize_t) iris_info.depth; z++)
            {
              p=pixels;
              for (y=0; y < (ssize_t) iris_info.rows; y++)
              {
                if (offset != offsets[y+z*iris_info.rows])
                  {
                    offset=offsets[y+z*iris_info.rows];
                    offset=(ssize_t) SeekBlob(image,(ssize_t) offset,SEEK_SET);
                  }
                count=ReadBlob(image,(size_t) runlength[y+z*iris_info.rows],
                  packets);
                if (EOFBlob(image) != MagickFalse)
                  break;
                offset+=(ssize_t) runlength[y+z*iris_info.rows];
                status=SGIDecode(bytes_per_pixel,(ssize_t)
                  (runlength[y+z*iris_info.rows]/bytes_per_pixel),packets,
                  1L*iris_info.columns,p+bytes_per_pixel*z);
                if (status == MagickFalse)
                  ThrowReaderException(CorruptImageError,"ImproperImageHeader");
                p+=(iris_info.columns*4*bytes_per_pixel);
              }
            }
          }
        else
          {
            MagickOffsetType
              position;

            position=TellBlob(image);
            p=pixels;
            for (y=0; y < (ssize_t) iris_info.rows; y++)
            {
              for (z=0; z < (ssize_t) iris_info.depth; z++)
              {
                if (offset != offsets[y+z*iris_info.rows])
                  {
                    offset=offsets[y+z*iris_info.rows];
                    offset=(ssize_t) SeekBlob(image,(ssize_t) offset,SEEK_SET);
                  }
                count=ReadBlob(image,(size_t) runlength[y+z*iris_info.rows],
                  packets);
                if (EOFBlob(image) != MagickFalse)
                  break;
                offset+=(ssize_t) runlength[y+z*iris_info.rows];
                status=SGIDecode(bytes_per_pixel,(ssize_t)
                  (runlength[y+z*iris_info.rows]/bytes_per_pixel),packets,
                  1L*iris_info.columns,p+bytes_per_pixel*z);
                if (status == MagickFalse)
                  ThrowReaderException(CorruptImageError,"ImproperImageHeader");
              }
              p+=(iris_info.columns*4*bytes_per_pixel);
            }
            offset=(ssize_t) SeekBlob(image,position,SEEK_SET);
          }
        packet_info=RelinquishVirtualMemory(packet_info);
        runlength=(size_t *) RelinquishMagickMemory(runlength);
        offsets=(ssize_t *) RelinquishMagickMemory(offsets);
      }
    /*
      Initialize image structure.
    */
    image->matte=iris_info.depth == 4 ? MagickTrue : MagickFalse;
    image->columns=iris_info.columns;
    image->rows=iris_info.rows;
    /*
      Convert SGI raster image to pixel packets.
    */
    if (image->storage_class == DirectClass)
      {
        /*
          Convert SGI image to DirectClass pixel packets.
        */
        if (bytes_per_pixel == 2)
          {
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=pixels+(image->rows-y-1)*8*image->columns;
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelRed(q,ScaleShortToQuantum((unsigned short)
                  ((*(p+0) << 8) | (*(p+1)))));
                SetPixelGreen(q,ScaleShortToQuantum((unsigned short)
                  ((*(p+2) << 8) | (*(p+3)))));
                SetPixelBlue(q,ScaleShortToQuantum((unsigned short)
                  ((*(p+4) << 8) | (*(p+5)))));
                SetPixelOpacity(q,OpaqueOpacity);
                if (image->matte != MagickFalse)
                  SetPixelAlpha(q,ScaleShortToQuantum((unsigned short)
                    ((*(p+6) << 8) | (*(p+7)))));
                p+=8;
                q++;
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
          }
        else
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            p=pixels+(image->rows-y-1)*4*image->columns;
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (PixelPacket *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              SetPixelRed(q,ScaleCharToQuantum(*p));
              q->green=ScaleCharToQuantum(*(p+1));
              q->blue=ScaleCharToQuantum(*(p+2));
              SetPixelOpacity(q,OpaqueOpacity);
              if (image->matte != MagickFalse)
                SetPixelAlpha(q,ScaleCharToQuantum(*(p+3)));
              p+=4;
              q++;
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
      }
    else
      {
        /*
          Create grayscale map.
        */
        if (AcquireImageColormap(image,image->colors) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        /*
          Convert SGI image to PseudoClass pixel packets.
        */
        if (bytes_per_pixel == 2)
          {
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=pixels+(image->rows-y-1)*8*image->columns;
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=GetAuthenticIndexQueue(image);
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                quantum=(*p << 8);
                quantum|=(*(p+1));
                SetPixelIndex(indexes+x,quantum);
                p+=8;
                q++;
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
          }
        else
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            p=pixels+(image->rows-y-1)*4*image->columns;
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (PixelPacket *) NULL)
              break;
            indexes=GetAuthenticIndexQueue(image);
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              SetPixelIndex(indexes+x,*p);
              p+=4;
              q++;
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
        (void) SyncImage(image);
      }
    pixel_info=RelinquishVirtualMemory(pixel_info);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    iris_info.magic=ReadBlobMSBShort(image);
    if (iris_info.magic == 0x01DA)
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
  } while (iris_info.magic == 0x01DA);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S G I I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSGIImage() adds properties for the SGI image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSGIImage method is:
%
%      size_t RegisterSGIImage(void)
%
*/
ModuleExport size_t RegisterSGIImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("SGI");
  entry->decoder=(DecodeImageHandler *) ReadSGIImage;
  entry->encoder=(EncodeImageHandler *) WriteSGIImage;
  entry->magick=(IsImageFormatHandler *) IsSGI;
  entry->description=ConstantString("Irix RGB image");
  entry->module=ConstantString("SGI");
  entry->seekable_stream=MagickTrue;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S G I I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterSGIImage() removes format registrations made by the
%  SGI module from the list of supported formats.
%
%  The format of the UnregisterSGIImage method is:
%
%      UnregisterSGIImage(void)
%
*/
ModuleExport void UnregisterSGIImage(void)
{
  (void) UnregisterMagickInfo("SGI");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S G I I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteSGIImage() writes an image in SGI RGB encoded image format.
%
%  The format of the WriteSGIImage method is:
%
%      MagickBooleanType WriteSGIImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static size_t SGIEncode(unsigned char *pixels,size_t length,
  unsigned char *packets)
{
  short
    runlength;

  register unsigned char
    *p,
    *q;

  unsigned char
    *limit,
    *mark;

  p=pixels;
  limit=p+length*4;
  q=packets;
  while (p < limit)
  {
    mark=p;
    p+=8;
    while ((p < limit) && ((*(p-8) != *(p-4)) || (*(p-4) != *p)))
      p+=4;
    p-=8;
    length=(size_t) (p-mark) >> 2;
    while (length != 0)
    {
      runlength=(short) (length > 126 ? 126 : length);
      length-=runlength;
      *q++=(unsigned char) (0x80 | runlength);
      for ( ; runlength > 0; runlength--)
      {
        *q++=(*mark);
        mark+=4;
      }
    }
    mark=p;
    p+=4;
    while ((p < limit) && (*p == *mark))
      p+=4;
    length=(size_t) (p-mark) >> 2;
    while (length != 0)
    {
      runlength=(short) (length > 126 ? 126 : length);
      length-=runlength;
      *q++=(unsigned char) runlength;
      *q++=(*mark);
    }
  }
  *q++='\0';
  return((size_t) (q-packets));
}

static MagickBooleanType WriteSGIImage(const ImageInfo *image_info,Image *image)
{
  CompressionType
    compression;

  const char
    *value;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  MagickSizeType
    number_pixels;

  MemoryInfo
    *pixel_info;

  SGIInfo
    iris_info;

  register const PixelPacket
    *p;

  register ssize_t
    i,
    x;

  register unsigned char
    *q;

  ssize_t
    y,
    z;

  unsigned char
    *pixels,
    *packets;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns > 65535UL) || (image->rows > 65535UL))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  do
  {
    /*
      Initialize SGI raster file header.
    */
    if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
      (void) TransformImageColorspace(image,sRGBColorspace);
    (void) ResetMagickMemory(&iris_info,0,sizeof(iris_info));
    iris_info.magic=0x01DA;
    compression=image->compression;
    if (image_info->compression != UndefinedCompression)
      compression=image_info->compression;
    if (image->depth > 8)
      compression=NoCompression;
    if (compression == NoCompression)
      iris_info.storage=(unsigned char) 0x00;
    else
      iris_info.storage=(unsigned char) 0x01;
    iris_info.bytes_per_pixel=(unsigned char) (image->depth > 8 ? 2 : 1);
    iris_info.dimension=3;
    iris_info.columns=(unsigned short) image->columns;
    iris_info.rows=(unsigned short) image->rows;
    if (image->matte != MagickFalse)
      iris_info.depth=4;
    else
      {
        if ((image_info->type != TrueColorType) &&
            (IsGrayImage(image,&image->exception) != MagickFalse))
          {
            iris_info.dimension=2;
            iris_info.depth=1;
          }
        else
          iris_info.depth=3;
      }
    iris_info.minimum_value=0;
    iris_info.maximum_value=(size_t) (image->depth <= 8 ?
      1UL*ScaleQuantumToChar(QuantumRange) :
      1UL*ScaleQuantumToShort(QuantumRange));
    /*
      Write SGI header.
    */
    (void) WriteBlobMSBShort(image,iris_info.magic);
    (void) WriteBlobByte(image,iris_info.storage);
    (void) WriteBlobByte(image,iris_info.bytes_per_pixel);
    (void) WriteBlobMSBShort(image,iris_info.dimension);
    (void) WriteBlobMSBShort(image,iris_info.columns);
    (void) WriteBlobMSBShort(image,iris_info.rows);
    (void) WriteBlobMSBShort(image,iris_info.depth);
    (void) WriteBlobMSBLong(image,(unsigned int) iris_info.minimum_value);
    (void) WriteBlobMSBLong(image,(unsigned int) iris_info.maximum_value);
    (void) WriteBlobMSBLong(image,(unsigned int) iris_info.sans);
    value=GetImageProperty(image,"label");
    if (value != (const char *) NULL)
      (void) CopyMagickString(iris_info.name,value,sizeof(iris_info.name));
    (void) WriteBlob(image,sizeof(iris_info.name),(unsigned char *)
      iris_info.name);
    (void) WriteBlobMSBLong(image,(unsigned int) iris_info.pixel_format);
    (void) WriteBlob(image,sizeof(iris_info.filler),iris_info.filler);
    /*
      Allocate SGI pixels.
    */
    number_pixels=(MagickSizeType) image->columns*image->rows;
    if ((4*iris_info.bytes_per_pixel*number_pixels) !=
        ((MagickSizeType) (size_t) (4*iris_info.bytes_per_pixel*number_pixels)))
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    pixel_info=AcquireVirtualMemory((size_t) number_pixels,4*
      iris_info.bytes_per_pixel*sizeof(*pixels));
    if (pixel_info == (MemoryInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    /*
      Convert image pixels to uncompressed SGI pixels.
    */
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
      if (p == (const PixelPacket *) NULL)
        break;
      if (image->depth <= 8)
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register unsigned char
            *q;

          q=(unsigned char *) pixels;
          q+=((iris_info.rows-1)-y)*(4*iris_info.columns)+4*x;
          *q++=ScaleQuantumToChar(GetPixelRed(p));
          *q++=ScaleQuantumToChar(GetPixelGreen(p));
          *q++=ScaleQuantumToChar(GetPixelBlue(p));
          *q++=ScaleQuantumToChar(GetPixelAlpha(p));
          p++;
        }
      else
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register unsigned short
            *q;

          q=(unsigned short *) pixels;
          q+=((iris_info.rows-1)-y)*(4*iris_info.columns)+4*x;
          *q++=ScaleQuantumToShort(GetPixelRed(p));
          *q++=ScaleQuantumToShort(GetPixelGreen(p));
          *q++=ScaleQuantumToShort(GetPixelBlue(p));
          *q++=ScaleQuantumToShort(GetPixelAlpha(p));
          p++;
        }
      if (image->previous == (Image *) NULL)
        {
          status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
    }
    switch (compression)
    {
      case NoCompression:
      {
        /*
          Write uncompressed SGI pixels.
        */
        for (z=0; z < (ssize_t) iris_info.depth; z++)
        {
          for (y=0; y < (ssize_t) iris_info.rows; y++)
          {
            if (image->depth <= 8)
              for (x=0; x < (ssize_t) iris_info.columns; x++)
              {
                register unsigned char
                  *q;

                q=(unsigned char *) pixels;
                q+=y*(4*iris_info.columns)+4*x+z;
                (void) WriteBlobByte(image,*q);
              }
            else
              for (x=0; x < (ssize_t) iris_info.columns; x++)
              {
                register unsigned short
                  *q;

                q=(unsigned short *) pixels;
                q+=y*(4*iris_info.columns)+4*x+z;
                (void) WriteBlobMSBShort(image,*q);
              }
          }
        }
        break;
      }
      default:
      {
        MemoryInfo
          *packet_info;

        size_t
          length,
          number_packets,
          *runlength;

        ssize_t
          offset,
          *offsets;

        /*
          Convert SGI uncompressed pixels.
        */
        offsets=(ssize_t *) AcquireQuantumMemory(iris_info.rows*iris_info.depth,
          sizeof(*offsets));
        runlength=(size_t *) AcquireQuantumMemory(iris_info.rows,
          iris_info.depth*sizeof(*runlength));
        packet_info=AcquireVirtualMemory((2*(size_t) iris_info.columns+10)*
          image->rows,4*sizeof(*packets));
        if ((offsets == (ssize_t *) NULL) ||
            (runlength == (size_t *) NULL) ||
            (packet_info == (MemoryInfo *) NULL))
          {
            if (offsets != (ssize_t *) NULL)
              offsets=(ssize_t *) RelinquishMagickMemory(offsets);
            if (runlength != (size_t *) NULL)
              runlength=(size_t *) RelinquishMagickMemory(runlength);
            if (packet_info != (MemoryInfo *) NULL)
              packet_info=RelinquishVirtualMemory(packet_info);
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          }
        packets=(unsigned char *) GetVirtualMemoryBlob(packet_info);
        offset=512+4*2*((ssize_t) iris_info.rows*iris_info.depth);
        number_packets=0;
        q=pixels;
        for (y=0; y < (ssize_t) iris_info.rows; y++)
        {
          for (z=0; z < (ssize_t) iris_info.depth; z++)
          {
            length=SGIEncode(q+z,(size_t) iris_info.columns,packets+
              number_packets);
            number_packets+=length;
            offsets[y+z*iris_info.rows]=offset;
            runlength[y+z*iris_info.rows]=(size_t) length;
            offset+=(ssize_t) length;
          }
          q+=(iris_info.columns*4);
        }
        /*
          Write out line start and length tables and runlength-encoded pixels.
        */
        for (i=0; i < (ssize_t) (iris_info.rows*iris_info.depth); i++)
          (void) WriteBlobMSBLong(image,(unsigned int) offsets[i]);
        for (i=0; i < (ssize_t) (iris_info.rows*iris_info.depth); i++)
          (void) WriteBlobMSBLong(image,(unsigned int) runlength[i]);
        (void) WriteBlob(image,number_packets,packets);
        /*
          Relinquish resources.
        */
        offsets=(ssize_t *) RelinquishMagickMemory(offsets);
        runlength=(size_t *) RelinquishMagickMemory(runlength);
        packet_info=RelinquishVirtualMemory(packet_info);
        break;
      }
    }
    pixel_info=RelinquishVirtualMemory(pixel_info);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}

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
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
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
#include "MagickCore/module.h"

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
  WriteSGIImage(const ImageInfo *,Image *,ExceptionInfo *);
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
            if (number_packets-- == 0)
              return(MagickFalse);
            pixel=(size_t) (*p++) << 8;
            pixel|=(*p++);
            for ( ; count != 0; count--)
            {
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

  register Quantum
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
    Read SGI raster header.
  */
  (void) memset(&iris_info,0,sizeof(iris_info));
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
    if ((iris_info.dimension == 0) || (iris_info.dimension > 3))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.columns=ReadBlobMSBShort(image);
    iris_info.rows=ReadBlobMSBShort(image);
    iris_info.depth=ReadBlobMSBShort(image);
    if ((iris_info.depth == 0) || (iris_info.depth > 4))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.minimum_value=ReadBlobMSBLong(image);
    iris_info.maximum_value=ReadBlobMSBLong(image);
    iris_info.sans=ReadBlobMSBLong(image);
    count=ReadBlob(image,sizeof(iris_info.name),(unsigned char *)
      iris_info.name);
    if ((size_t) count != sizeof(iris_info.name))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.name[sizeof(iris_info.name)-1]='\0';
    if (*iris_info.name != '\0')
      (void) SetImageProperty(image,"label",iris_info.name,exception);
    iris_info.pixel_format=ReadBlobMSBLong(image);
    if (iris_info.pixel_format != 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    count=ReadBlob(image,sizeof(iris_info.filler),iris_info.filler);
    if ((size_t) count != sizeof(iris_info.filler))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    image->columns=iris_info.columns;
    image->rows=iris_info.rows;
    image->alpha_trait=iris_info.depth == 4 ? BlendPixelTrait :
      UndefinedPixelTrait;
    image->depth=(size_t) MagickMin(iris_info.depth,MAGICKCORE_QUANTUM_DEPTH);
    if (iris_info.pixel_format == 0)
      image->depth=(size_t) MagickMin((size_t) 8*iris_info.bytes_per_pixel,
        MAGICKCORE_QUANTUM_DEPTH);
    if (iris_info.depth < 3)
      {
        image->storage_class=PseudoClass;
        image->colors=(size_t) (iris_info.bytes_per_pixel > 1 ? 65535 : 256);
      }
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status != MagickFalse)
      status=ResetImagePixels(image,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
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
    (void) memset(pixels,0,iris_info.columns*iris_info.rows*4*
      bytes_per_pixel*sizeof(*pixels));
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
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        for (z=0; z < (ssize_t) iris_info.depth; z++)
        {
          p=pixels+bytes_per_pixel*z;
          for (y=0; y < (ssize_t) iris_info.rows; y++)
          {
            count=ReadBlob(image,bytes_per_pixel*iris_info.columns,scanline);
            if (count != (bytes_per_pixel*iris_info.columns))
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
          if (y < (ssize_t) iris_info.rows)
            break;
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
        if ((offsets == (ssize_t *) NULL) || (runlength == (size_t *) NULL) ||
            (packet_info == (MemoryInfo *) NULL))
          {
            offsets=(ssize_t *) RelinquishMagickMemory(offsets);
            runlength=(size_t *) RelinquishMagickMemory(runlength);
            if (packet_info != (MemoryInfo *) NULL)
              packet_info=RelinquishVirtualMemory(packet_info);
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        packets=(unsigned char *) GetVirtualMemoryBlob(packet_info);
        for (i=0; i < (ssize_t) (iris_info.rows*iris_info.depth); i++)
          offsets[i]=(ssize_t) ReadBlobMSBSignedLong(image);
        for (i=0; i < (ssize_t) (iris_info.rows*iris_info.depth); i++)
        {
          runlength[i]=ReadBlobMSBLong(image);
          if (runlength[i] > (4*(size_t) iris_info.columns+10))
            {
              packet_info=RelinquishVirtualMemory(packet_info);
              runlength=(size_t *) RelinquishMagickMemory(runlength);
              offsets=(ssize_t *) RelinquishMagickMemory(offsets);
              pixel_info=RelinquishVirtualMemory(pixel_info);
              ThrowReaderException(CorruptImageError,"ImproperImageHeader");
            }
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
                    offset=(ssize_t) SeekBlob(image,(MagickOffsetType) offset,
                      SEEK_SET);
                  }
                count=ReadBlob(image,(size_t) runlength[y+z*iris_info.rows],
                  packets);
                if (count != runlength[y+z*iris_info.rows])
                  break;
                offset+=(ssize_t) runlength[y+z*iris_info.rows];
                status=SGIDecode(bytes_per_pixel,(ssize_t)
                  (runlength[y+z*iris_info.rows]/bytes_per_pixel),packets,
                  (ssize_t) iris_info.columns,p+bytes_per_pixel*z);
                if (status == MagickFalse)
                  {
                    packet_info=RelinquishVirtualMemory(packet_info);
                    runlength=(size_t *) RelinquishMagickMemory(runlength);
                    offsets=(ssize_t *) RelinquishMagickMemory(offsets);
                    pixel_info=RelinquishVirtualMemory(pixel_info);
                    ThrowReaderException(CorruptImageError,
                      "ImproperImageHeader");
                  }
                p+=(iris_info.columns*4*bytes_per_pixel);
              }
              if (y < (ssize_t) iris_info.rows)
                break;
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
                    offset=(ssize_t) SeekBlob(image,(MagickOffsetType) offset,
                      SEEK_SET);
                  }
                count=ReadBlob(image,(size_t) runlength[y+z*iris_info.rows],
                  packets);
                if (count != runlength[y+z*iris_info.rows])
                  break;
                offset+=(ssize_t) runlength[y+z*iris_info.rows];
                status=SGIDecode(bytes_per_pixel,(ssize_t)
                  (runlength[y+z*iris_info.rows]/bytes_per_pixel),packets,
                  (ssize_t) iris_info.columns,p+bytes_per_pixel*z);
                if (status == MagickFalse)
                  {
                    packet_info=RelinquishVirtualMemory(packet_info);
                    runlength=(size_t *) RelinquishMagickMemory(runlength);
                    offsets=(ssize_t *) RelinquishMagickMemory(offsets);
                    pixel_info=RelinquishVirtualMemory(pixel_info);
                    ThrowReaderException(CorruptImageError,
                      "ImproperImageHeader");
                  }
              }
              if (z < (ssize_t) iris_info.depth)
                break;
              p+=(iris_info.columns*4*bytes_per_pixel);
            }
            offset=(ssize_t) SeekBlob(image,position,SEEK_SET);
          }
        packet_info=RelinquishVirtualMemory(packet_info);
        runlength=(size_t *) RelinquishMagickMemory(runlength);
        offsets=(ssize_t *) RelinquishMagickMemory(offsets);
      }
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
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelRed(image,ScaleShortToQuantum((unsigned short)
                  ((*(p+0) << 8) | (*(p+1)))),q);
                SetPixelGreen(image,ScaleShortToQuantum((unsigned short)
                  ((*(p+2) << 8) | (*(p+3)))),q);
                SetPixelBlue(image,ScaleShortToQuantum((unsigned short)
                  ((*(p+4) << 8) | (*(p+5)))),q);
                SetPixelAlpha(image,OpaqueAlpha,q);
                if (image->alpha_trait != UndefinedPixelTrait)
                  SetPixelAlpha(image,ScaleShortToQuantum((unsigned short)
                    ((*(p+6) << 8) | (*(p+7)))),q);
                p+=8;
                q+=GetPixelChannels(image);
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
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              SetPixelRed(image,ScaleCharToQuantum(*p),q);
              SetPixelGreen(image,ScaleCharToQuantum(*(p+1)),q);
              SetPixelBlue(image,ScaleCharToQuantum(*(p+2)),q);
              SetPixelAlpha(image,OpaqueAlpha,q);
              if (image->alpha_trait != UndefinedPixelTrait)
                SetPixelAlpha(image,ScaleCharToQuantum(*(p+3)),q);
              p+=4;
              q+=GetPixelChannels(image);
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
        if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        /*
          Convert SGI image to PseudoClass pixel packets.
        */
        if (bytes_per_pixel == 2)
          {
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=pixels+(image->rows-y-1)*8*image->columns;
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                quantum=(*p << 8);
                quantum|=(*(p+1));
                SetPixelIndex(image,(Quantum) quantum,q);
                p+=8;
                q+=GetPixelChannels(image);
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
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              SetPixelIndex(image,*p,q);
              p+=4;
              q+=GetPixelChannels(image);
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
        (void) SyncImage(image,exception);
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
        AcquireNextImage(image_info,image,exception);
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

  entry=AcquireMagickInfo("SGI","SGI","Irix RGB image");
  entry->decoder=(DecodeImageHandler *) ReadSGIImage;
  entry->encoder=(EncodeImageHandler *) WriteSGIImage;
  entry->magick=(IsImageFormatHandler *) IsSGI;
  entry->flags|=CoderDecoderSeekableStreamFlag;
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
%      MagickBooleanType WriteSGIImage(const ImageInfo *image_info,
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

static MagickBooleanType WriteSGIImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
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

  register const Quantum
    *p;

  register ssize_t
    i,
    x;

  register unsigned char
    *q;

  size_t
    imageListLength;

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
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns > 65535UL) || (image->rows > 65535UL))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  imageListLength=GetImageListLength(image);
  do
  {
    /*
      Initialize SGI raster file header.
    */
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
    (void) memset(&iris_info,0,sizeof(iris_info));
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
    if (image->alpha_trait != UndefinedPixelTrait)
      iris_info.depth=4;
    else
      {
        if ((image_info->type != TrueColorType) &&
            (SetImageGray(image,exception) != MagickFalse))
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
    value=GetImageProperty(image,"label",exception);
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
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        break;
      if (image->depth <= 8)
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register unsigned char
            *q;

          q=(unsigned char *) pixels;
          q+=((iris_info.rows-1)-y)*(4*iris_info.columns)+4*x;
          *q++=ScaleQuantumToChar(GetPixelRed(image,p));
          *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
          *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
          *q++=ScaleQuantumToChar(GetPixelAlpha(image,p));
          p+=GetPixelChannels(image);
        }
      else
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register unsigned short
            *q;

          q=(unsigned short *) pixels;
          q+=((iris_info.rows-1)-y)*(4*iris_info.columns)+4*x;
          *q++=ScaleQuantumToShort(GetPixelRed(image,p));
          *q++=ScaleQuantumToShort(GetPixelGreen(image,p));
          *q++=ScaleQuantumToShort(GetPixelBlue(image,p));
          *q++=ScaleQuantumToShort(GetPixelAlpha(image,p));
          p+=GetPixelChannels(image);
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
        offsets=(ssize_t *) AcquireQuantumMemory(iris_info.rows,
          iris_info.depth*sizeof(*offsets));
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
            pixel_info=RelinquishVirtualMemory(pixel_info);
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
    status=SetImageProgress(image,SaveImagesTag,scene++,imageListLength);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}

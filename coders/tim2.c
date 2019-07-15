/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                          TTTTT  IIIII  M   M   222                          %
%                            T      I    MM MM  2   2                         %
%                            T      I    M M M     2                          %
%                            T      I    M   M    2                           %
%                            T    IIIII  M   M  22222                         %
%                                                                             %
%                                                                             %
%                          Read PSX TIM2 Image Format                         %
%                                                                             %
%                               Software Design                               %
%                             Ramiro Balado Ordax                             %
%                                   May 2019                                  %
%                                                                             %
%                                                                             %
%  Copyright 2019-2019 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/colormap.h"
#include "MagickCore/channel.h"
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
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"


/*
 Typedef declarations
*/
typedef struct _TIM2FileHeader
{
  unsigned int
    magic_num;

  unsigned char
    format_vers,
    format_type;

  unsigned short
    image_count;

  char
    reserved[8];
} TIM2FileHeader;

typedef struct _TIM2ImageHeader
{
  unsigned int
    total_size,
    clut_size,
    image_size;

  unsigned short
    header_size,
    clut_color_count;

  unsigned char
    img_format,
    mipmap_count,
    clut_type,
    bpp_type;

  unsigned short
    width,
    height;

  MagickSizeType
    GsTex0,
    GsTex1;

  unsigned int
    GsRegs,
    GsTexClut;
} TIM2ImageHeader;

typedef enum
{
  CSM1=0,
  CSM2=1,
} CSM;

typedef enum
{
  RGBA32=0,
  RGB24=1,
  RGBA16=2,
} TIM2ColorEncoding;


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d T I M 2 I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadTIM2Image() reads a PS2 TIM image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadTIM2Image method is:
%
%      Image *ReadTIM2Image(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static inline void ReadTIM2ImageHeader(Image *image,TIM2ImageHeader *header)
{
  header->total_size=ReadBlobLSBLong(image);
  header->clut_size=ReadBlobLSBLong(image);
  header->image_size=ReadBlobLSBLong(image);
  header->header_size=ReadBlobLSBShort(image);

  header->clut_color_count=ReadBlobLSBShort(image);
  header->img_format=(unsigned char) ReadBlobByte(image);
  header->mipmap_count=(unsigned char) ReadBlobByte(image);
  header->clut_type=(unsigned char) ReadBlobByte(image);
  header->bpp_type=(unsigned char) ReadBlobByte(image);

  header->width=ReadBlobLSBShort(image);
  header->height=ReadBlobLSBShort(image);

  header->GsTex0=ReadBlobMSBLongLong(image);
  header->GsTex1=ReadBlobMSBLongLong(image);
  header->GsRegs=ReadBlobMSBLong(image);
  header->GsTexClut=ReadBlobMSBLong(image);
}

static inline Quantum GetChannelValue(unsigned int word,unsigned char channel,
  TIM2ColorEncoding ce)
{
  switch(ce)
  {
    case RGBA16:
      /* Documentation specifies padding with zeros for converting from 5 to 8 bits. */
      return ScaleCharToQuantum((word>>channel*5 & ~(~0x0<<5))<<3);
    case RGB24:
    case RGBA32:
      return ScaleCharToQuantum(word>>channel*8 & ~(~0x0<<8));
    default:
      return -1;
  }
}

static inline Quantum GetAlpha(unsigned int word,TIM2ColorEncoding ce)
{
  switch(ce)
  {
    case RGBA16:
      return ScaleCharToQuantum((word>>3*5&0x1F)==0?0:0xFF);
    case RGBA32:
      /* 0x80 -> 1.0 alpha. Multiply by 2 and clamp to 0xFF */
      return ScaleCharToQuantum(MagickMin((word>>3*8&0xFF)<<1,0xFF));
    default:
      return 0xFF;
  }
}

static inline void deshufflePalette(Image *image,PixelInfo* oldColormap)
{
  const size_t
    pages=image->colors/32,  /* Pages per CLUT */
    blocks=4,  /* Blocks per page */
    colors=8;  /* Colors per block */

  int
    page;

  size_t
    i=0;

  (void) memcpy(oldColormap,image->colormap,(size_t)image->colors*
    sizeof(*oldColormap));

  /*
   * Swap the 2nd and 3rd block in each page
   */
  for (page=0; page < pages; page++)
  {
    memcpy(&(image->colormap[i+1*colors]),&(oldColormap[i+2*colors]),colors*
      sizeof(PixelInfo));
    memcpy(&(image->colormap[i+2*colors]),&(oldColormap[i+1*colors]),colors*
      sizeof(PixelInfo));

    i+=blocks*colors;
  }
}

static MagickBooleanType ReadTIM2ImageData(const ImageInfo *image_info,
  Image *image,TIM2ImageHeader *header,char clut_depth,char bits_per_pixel,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  register ssize_t
    x;

  register Quantum
    *q;

  register unsigned char
    *p;

  size_t
    bits_per_line,
    bytes_per_line;

  ssize_t
    count,
    y;

  unsigned char
    *row_data;

  unsigned int
    word;

  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(MagickFalse);
  /*
   * User data
   */
  status=DiscardBlobBytes(image,header->header_size-48);
  if (status == MagickFalse)
    return(MagickFalse);
  /*
   * Image data
   */
  bits_per_line=image->columns*bits_per_pixel;
  bytes_per_line=bits_per_line/8 + ((bits_per_line%8==0) ? 0 : 1);
  row_data=(unsigned char*) AcquireMagickMemory(bytes_per_line);
  if (row_data == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image_info->filename);
  if (clut_depth != 0)
    {
      image->colors=header->clut_color_count;
      if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
        {
          row_data=(unsigned char *) RelinquishMagickMemory(row_data);
          ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
            image_info->filename);
        }
      switch (bits_per_pixel)
      {
        case 4:
        {
          for (y=0; y<(ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            count=ReadBlob(image,bytes_per_line,row_data);
            if (count != (ssize_t) bytes_per_line)
              {
                row_data=(unsigned char *) RelinquishMagickMemory(row_data);
                ThrowBinaryException(CorruptImageError,
                  "InsufficientImageDataInFile",image_info->filename);
              }
            p=row_data;
            for (x=0; x < ((ssize_t) image->columns-1); x+=2)
            {
              SetPixelIndex(image,(*p >> 0) & 0x0F,q);
              q+=GetPixelChannels(image);
              SetPixelIndex(image,(*p >> 4) & 0x0F,q);
              p++;
              q+=GetPixelChannels(image);
            }
            if ((image->columns % 2) != 0)
              {
                SetPixelIndex(image,(*p >> 4) & 0x0F,q);
                p++;
                q+=GetPixelChannels(image);
              }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,
                  (MagickOffsetType) y,image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
          break;
        }
        case 8:
        {
          for (y=0;y<(ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            count=ReadBlob(image,bytes_per_line,row_data);
            if (count != (ssize_t) bytes_per_line)
              {
                row_data=(unsigned char *) RelinquishMagickMemory(row_data);
                ThrowBinaryException(CorruptImageError,
                  "InsufficientImageDataInFile",image_info->filename);
              }
            p=row_data;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              SetPixelIndex(image,*p,q);
              p++;
              q+=GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,
                  (MagickOffsetType) y,image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
          break;
        }
        default:
        {
          row_data=(unsigned char *) RelinquishMagickMemory(row_data);
          ThrowBinaryException(CorruptImageError,"ImproperImageHeader",
            image_info->filename);
        }
      }
    }
  else  /* has_clut==false */
    {
      switch (bits_per_pixel)
      {
        case 16:
        {
          for (y=0; y<(ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            count=ReadBlob(image,bytes_per_line,row_data);
            if (count != (ssize_t) bytes_per_line)
              {
                row_data=(unsigned char *) RelinquishMagickMemory(row_data);
                ThrowBinaryException(CorruptImageError,
                  "InsufficientImageDataInFile",image_info->filename);
              }
            p=row_data;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              word = ((unsigned int)* p   )<<0*8 |
                      ((unsigned int)*(p+1))<<1*8;

              SetPixelRed(image,GetChannelValue(word,0,RGBA16),q);
              SetPixelGreen(image,GetChannelValue(word,1,RGBA16),q);
              SetPixelBlue(image,GetChannelValue(word,2,RGBA16),q);
              SetPixelAlpha(image,GetAlpha(word,RGBA16),q);
              q+=GetPixelChannels(image);
              p+=sizeof(unsigned short);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,
                  (MagickOffsetType) y,image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
          break;
        }
        case 24:
        {
          for (y = 0; y<(ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            count=ReadBlob(image,bytes_per_line,row_data);
            if (count != (ssize_t) bytes_per_line)
              {
                row_data=(unsigned char *) RelinquishMagickMemory(row_data);
                ThrowBinaryException(CorruptImageError,
                  "InsufficientImageDataInFile",image_info->filename);
              }
            p=row_data;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              word = (unsigned int)(* p   )<<0*8 |
                      (unsigned int)(*(p+1))<<1*8 |
                      (unsigned int)(*(p+2))<<2*8;

              SetPixelRed(image,GetChannelValue(word,0,RGB24),q);
              SetPixelGreen(image,GetChannelValue(word,1,RGB24),q);
              SetPixelBlue(image,GetChannelValue(word,2,RGB24),q);
              q+=GetPixelChannels(image);
              p+=3;
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,
                  (MagickOffsetType) y,image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
          break;
        }
        case 32:
        {  
          for (y = 0; y<(ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            count=ReadBlob(image,bytes_per_line,row_data);
            if (count != (ssize_t) bytes_per_line)
              {
                row_data=(unsigned char *) RelinquishMagickMemory(row_data);
                ThrowBinaryException(CorruptImageError,
                  "InsufficientImageDataInFile",image_info->filename);
              }
            p=row_data;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              word = ((unsigned int)* p   )<<0*8 |
                      ((unsigned int)*(p+1))<<1*8 |
                      ((unsigned int)*(p+2))<<2*8 |
                      ((unsigned int)*(p+3))<<3*8;

              SetPixelRed(image,GetChannelValue(word,0,RGBA32),q);
              SetPixelGreen(image,GetChannelValue(word,1,RGBA32),q);
              SetPixelBlue(image,GetChannelValue(word,2,RGBA32),q);
              SetPixelAlpha(image,GetAlpha(word,RGBA32),q);
              q+=GetPixelChannels(image);
              p+=4;
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,
                  (MagickOffsetType) y,image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
          break;
        }
        default:
        {
          row_data=(unsigned char *) RelinquishMagickMemory(row_data);
          ThrowBinaryException(CorruptImageError,"ImproperImageHeader",
            image_info->filename);
        }
      }
    }
  row_data=(unsigned char *) RelinquishMagickMemory(row_data);
  if ((status != MagickFalse) && (clut_depth != 0))
  {
    CSM
      csm;

    register ssize_t
      i;

    unsigned char
      *clut_data;

    /*
      * ### Read CLUT Data ###
      */
    clut_data=(unsigned char *) AcquireMagickMemory(header->clut_size);
    if (clut_data == (unsigned char *) NULL)
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image_info->filename);
    count=ReadBlob(image,header->clut_size,clut_data);
    if (count != (ssize_t) (header->clut_size))
      {
        clut_data=(unsigned char *) RelinquishMagickMemory(clut_data);
        ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
          image_info->filename);
      }
    /*
      * ### Process CLUT Data ###
      */
    p=clut_data;
    switch(clut_depth)
    {
      case 16:
      {
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          word = ((unsigned short)* p   )<<0*8 |
                  ((unsigned short)*(p+1))<<1*8;

          image->colormap[i].red=GetChannelValue(word,0,RGBA16);
          image->colormap[i].green=GetChannelValue(word,1,RGBA16);
          image->colormap[i].blue=GetChannelValue(word,2,RGBA16);
          image->colormap[i].alpha=GetAlpha(word,16);
          p+=2;
        }
        break;
      }
      case 24:
      {
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          word = ((unsigned int)* p   )<<0*8 |
                  ((unsigned int)*(p+1))<<1*8 |
                  ((unsigned int)*(p+2))<<2*8;

          image->colormap[i].red=GetChannelValue(word,0,RGB24);
          image->colormap[i].green=GetChannelValue(word,1,RGB24);
          image->colormap[i].blue=GetChannelValue(word,2,RGB24);
          p+=3;
        }
        break;
      }
      case 32:
      {
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          word = ((unsigned int)* p   )<<0*8 |
                  ((unsigned int)*(p+1))<<1*8 |
                  ((unsigned int)*(p+2))<<2*8 |
                  ((unsigned int)*(p+3))<<3*8;

          image->colormap[i].red=GetChannelValue(word,0,RGBA32);
          image->colormap[i].green=GetChannelValue(word,1,RGBA32);
          image->colormap[i].blue=GetChannelValue(word,2,RGBA32);
          image->colormap[i].alpha=GetAlpha(word,RGBA32);
          p+=4;
        }
        break;
      }
    }
    clut_data=(unsigned char *) RelinquishMagickMemory(clut_data);
    /* CSM: CLUT Storage Mode */
    switch ((int) header->clut_type>>4)  /* High 4 bits */
    {
      case 0:
        csm=CSM1;
        break;
      case 1:
        csm=CSM2;
        break;
      default:
        ThrowBinaryException(CorruptImageError,"ImproperImageHeader",
          image_info->filename);
        break;
    }
    if (csm==CSM1)
      {
        PixelInfo
          *oldColormap;

        oldColormap=(PixelInfo *) AcquireQuantumMemory((size_t)(image->colors)+1,
          sizeof(*image->colormap));
        if (oldColormap == (PixelInfo *) NULL)
          ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
            image_info->filename);
        deshufflePalette(image,oldColormap);
        RelinquishMagickMemory(oldColormap);
      }
  }
  return(status);
}

static Image *ReadTIM2Image(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  ssize_t
    count,
    str_read;

  TIM2FileHeader
    file_header;

  /*
   * Open image file.
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
   * Verify TIM2 magic number.
   */
  file_header.magic_num=ReadBlobMSBLong(image);
  if (file_header.magic_num != 0x54494D32) /* "TIM2" */
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
   * #### Read File Header ####
   */
  file_header.format_vers=ReadBlobByte(image);
  if (file_header.format_vers != 0x04)
    ThrowReaderException(CoderError,"ImageTypeNotSupported");
  file_header.format_type=ReadBlobByte(image);
  file_header.image_count=ReadBlobLSBShort(image);
  ReadBlobStream(image,8,&(file_header.reserved),&str_read);
  /*
   * Jump to first image header
   */
  switch(file_header.format_type)
  {
    case 0x00:
      if (DiscardBlobBytes(image,16) == MagickFalse)
        ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
      break;
    case 0x01:
      if (DiscardBlobBytes(image,128) == MagickFalse)
        ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
      break;
    default:
      ThrowReaderException(CoderError,"ImageTypeNotSupported");
  }
  /*
   * Process each image. Only one image supported for now
   */
  if (file_header.image_count != 1)
    ThrowReaderException(CoderError,"NumberOfImagesIsNotSupported");
  for (int i=0; i < file_header.image_count; ++i)
  {
    char
      clut_depth,
      bits_per_pixel;

    TIM2ImageHeader
      image_header;

    ReadTIM2ImageHeader(image,&image_header);
    if (image_header.mipmap_count != 1)
      ThrowReaderException(CoderError,"NumberOfImagesIsNotSupported");
    if (image_header.header_size < 48)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if ((MagickSizeType) image_header.image_size > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    if ((MagickSizeType) image_header.clut_size > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    image->columns=image_header.width;
    image->rows=image_header.height;
    clut_depth=0;
    if (image_header.clut_type !=0)
      {
        switch((int) image_header.clut_type&0x0F)  /* Low 4 bits */
        {
          case 1:
            clut_depth=16;
            break;
          case 2:
            clut_depth=24;
            break;
          case 3:
            clut_depth=32;
            break;
          default:
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
            break;
        }
      }
    switch ((int) image_header.bpp_type)
    {
      case 1:
        bits_per_pixel=16;
        break;
      case 2:
        bits_per_pixel=24;
        break;
      case 3:
        bits_per_pixel=32;
        break;
      case 4:
        bits_per_pixel=4;  /* Implies CLUT */
        break;
      case 5:
        bits_per_pixel=8;  /* Implies CLUT */
        break;
      default:
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        break;
    }
    image->depth=(clut_depth != 0) ? clut_depth : bits_per_pixel;
    if ((image->depth == 16) || (image->depth == 32))
      image->alpha_trait=BlendPixelTrait;
    if (image->ping != MagickFalse)
      {
        status=ReadTIM2ImageData(image_info,image,&image_header,clut_depth,
          bits_per_pixel,exception);
        if (status==MagickFalse)
          break;
      }
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if ((image->storage_class == PseudoClass) && (EOFBlob(image) != MagickFalse))
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
    status=SetImageProgress(image,LoadImagesTag,image->scene-1,
      image->scene);
    if (status == MagickFalse)
      break;
  }
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
%   R e g i s t e r T I M 2 I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterTIM2Image() adds attributes for the TIM2 image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterTIM2Image method is:
%
%      size_t RegisterTIM2Image(void)
%
*/
ModuleExport size_t RegisterTIM2Image(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("TIM2","TM2","PS2 TIM2");
  entry->decoder=(DecodeImageHandler *) ReadTIM2Image;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r T I M 2 I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterTIM2Image() removes format registrations made by the
%  TIM2 module from the list of supported formats.
%
%  The format of the UnregisterTIM2Image method is:
%
%      UnregisterTIM2Image(void)
%
*/
ModuleExport void UnregisterTIM2Image(void)
{
  (void) UnregisterMagickInfo("TM2");
}

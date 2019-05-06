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
%                              Software Design                                %
%                                Ramiro Balado                                %
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
  uint32_t
    magic_num;
  uint16_t
    format_type,
    image_count;
  char
    reserved[8];
} TIM2FileHeader;

typedef struct _TIM2ImageHeader
{
  uint32_t
    total_size,
    clut_size,
    image_size;
  uint16_t
    header_size,
    clut_color_count;
  uint8_t
    img_format,
    mipmap_count,
    clut_type,
    bpp_type;
  uint16_t
    width,
    height;
  uint64_t
    GsTex0,
    GsTex1;
  uint32_t
    GsRegs,
    GsTexClut;
} TIM2ImageHeader;

/*
  Static functions
*/
static inline TIM2ImageHeader ReadTIM2ImageHeader(Image *image)
{
  TIM2ImageHeader
    tim2_image_header;

  tim2_image_header.total_size =ReadBlobLSBLong(image);
  tim2_image_header.clut_size  =ReadBlobLSBLong(image);
  tim2_image_header.image_size =ReadBlobLSBLong(image);
  tim2_image_header.header_size=ReadBlobLSBShort(image);

  tim2_image_header.clut_color_count=ReadBlobLSBShort(image);
  tim2_image_header.img_format  =ReadBlobByte(image);
  tim2_image_header.mipmap_count=ReadBlobByte(image);
  tim2_image_header.clut_type   =ReadBlobByte(image);
  tim2_image_header.bpp_type    =ReadBlobByte(image);

  tim2_image_header.width =ReadBlobLSBShort(image);
  tim2_image_header.height=ReadBlobLSBShort(image);

  tim2_image_header.GsTex0=ReadBlobMSBLongLong(image);
  tim2_image_header.GsTex1=ReadBlobMSBLongLong(image);
  tim2_image_header.GsRegs   =ReadBlobMSBLong(image);
  tim2_image_header.GsTexClut=ReadBlobMSBLong(image);

  return tim2_image_header;
}

static inline Quantum GetChannelValue16(uint16_t word,uint8_t channel){
  return ScaleCharToQuantum(ScaleColor5to8(word>>channel*5 & 0x1F));
}
static inline Quantum GetChannelAlpha16(uint16_t word){
  return ScaleCharToQuantum((word>>3*5&0x1F)==0?0:0xFF);
}

static inline Quantum GetChannelValue24(uint32_t word,uint8_t channel){
  return ScaleCharToQuantum(ScaleColor6to8(word>> channel*6 & 0x3F));
}
static inline Quantum GetChannelAlpha24(uint32_t word){
  return ScaleCharToQuantum((word>>3*6&0x3F)==0?0:0xFF);
}

static inline Quantum GetChannelValue32(uint32_t word,uint8_t channel){
  return ScaleCharToQuantum((word>> channel*8) & 0xFF);
}
static inline Quantum GetChannelAlpha32(uint32_t word){
  return ScaleCharToQuantum((word>>3*8&0xFF)==0?0:0xFF);
}

static inline void deshufflePalette(Image *image,ExceptionInfo *exception){
  const uint8_t
    parts=image->colors/32,//Parts per CLUT
    blocks=4,//Blocks per part
    colors=8;//Colors per block
  size_t
    i=0;

  PixelInfo *oldColormap=(PixelInfo *)AcquireQuantumMemory((size_t)(image->colors)+1,sizeof(*image->colormap));
  (void) memcpy(oldColormap,image->colormap,(size_t)image->colors*sizeof(*oldColormap));

  /*
   * Swap the 2nd and 3rd block in each part
   */
  for(int part=0; part<parts; part++){
    memcpy(&(image->colormap[i+1*colors]),&(oldColormap[i+2*colors]),colors*sizeof(PixelInfo));
    memcpy(&(image->colormap[i+2*colors]),&(oldColormap[i+1*colors]),colors*sizeof(PixelInfo));

    i+=blocks*colors;
  }

  RelinquishMagickMemory(oldColormap);

}

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
static Image *ReadTIM2Image(const ImageInfo *image_info,ExceptionInfo *exception)
{
  TIM2FileHeader
    tim2_file_header;

  Image
    *image;

  MagickBooleanType
    status;


  ssize_t
    count,
    str_read;

  /*
   *  Open image file.
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
   *  Verify TIM2 magic number.
   */
  tim2_file_header.magic_num=ReadBlobMSBLong(image);
  if (tim2_file_header.magic_num != 0x54494D32) /*"TIM2"*/
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  /*
   * #### Read File Header ####
   */
  tim2_file_header.format_type=ReadBlobLSBShort(image);
  tim2_file_header.image_count=ReadBlobLSBShort(image);
  ReadBlobStream(image,8,&(tim2_file_header.reserved),&str_read);

  /*
   * Process each image. Only one image supported for now
   */
  if(tim2_file_header.image_count!=1)
    ThrowReaderException(CoderError,"NumberOfImagesIsNotSupported");

  for(int i=0;i<tim2_file_header.image_count;++i)
  {

    TIM2ImageHeader
      tim2_image_header;

    char
      csm=0,
      clut_depth=0,
      has_clut=0,
      bits_per_pixel=0;


    /*
     * #### Read Image Header ####
     */
    tim2_image_header=ReadTIM2ImageHeader(image);

    /*
     * ### Process Image Header ###
     */

    if(tim2_image_header.mipmap_count!=1)
      ThrowReaderException(CoderError,"NumberOfImagesIsNotSupported");

    image->columns=tim2_image_header.width;
    image->rows=tim2_image_header.height;

    has_clut=tim2_image_header.clut_type!=0;
    if(has_clut){
      //CSM: CLUT Storage Mode
      switch ((int) tim2_image_header.clut_type>>4)//High 4 bits
      {
        case 0: csm=1;break;
        case 1: csm=2;break;
        default:
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          break;

      }

      if(csm!=1)
        ThrowReaderException(CoderError,"DataStorageTypeIsNotSupported");

      // CLUT bits per color
      switch((int) tim2_image_header.clut_type&0x0F)//Low 4 bits
      {
        case 1: clut_depth=16;break;
        case 2: clut_depth=24;break;
        case 3: clut_depth=32;break;
        default:
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          break;
      }
    }

    // Bits per pixel.
    switch ((int) tim2_image_header.bpp_type)
    {
      case 1: bits_per_pixel=16;break;
      case 2: bits_per_pixel=24;break;
      case 3: bits_per_pixel=32;break;
      case 4: bits_per_pixel=4;break;// Implies CLUT
      case 5: bits_per_pixel=8;break;// Implies CLUT
      default:
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        break;
    }

    image->depth=has_clut?clut_depth:bits_per_pixel;


    {
      /*
       * ### Read Image Data ###
       */
      unsigned char
        *tim2_image_data;
      size_t
        bits_per_line,
        bytes_per_line,
        y;

      register ssize_t
        x;

      register Quantum
        *q;
      register unsigned char
        *p;

      tim2_image_data=(unsigned char*) AcquireMagickMemory(tim2_image_header.image_size);
      if (tim2_image_data == (unsigned char *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      count=ReadBlob(image,tim2_image_header.image_size,tim2_image_data);
      if (count!=(ssize_t)tim2_image_header.image_size)
      {
        tim2_image_data=(unsigned char *) RelinquishMagickMemory(tim2_image_data);
        ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
      }
      

      /*
       * ### Process Image Data ###
       */
      status=SetImageExtent(image,image->columns,image->rows,exception);
      if (status == MagickFalse)
        return(DestroyImageList(image));
      status=ResetImagePixels(image,exception);
      if (status == MagickFalse)
        return(DestroyImageList(image));

      p=tim2_image_data;
      bits_per_line=image->columns*bits_per_pixel;
      bytes_per_line=bits_per_line/8 + ((bits_per_line%8==0)?0:1);

      if(has_clut)
      {

#define SetPixelAllChannels(image,word,q,bits) \
SetPixelRed  (image,GetChannelValue##bits(word,0),q); \
SetPixelGreen(image,GetChannelValue##bits(word,1),q); \
SetPixelBlue (image,GetChannelValue##bits(word,2),q); \
SetPixelAlpha(image,GetChannelAlpha##bits(word)  ,q);

#define SyncNewPixels(image,exception,y) \
if(SyncAuthenticPixels(image,exception) == MagickFalse) break; \
if(image->previous == (Image *) NULL) \
{ \
  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,image->rows); \
  if(status == MagickFalse) break; \
}

        image->colors=tim2_image_header.clut_color_count;
        if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

        switch (bits_per_pixel)
        {
          case 4:
          {
            for (y=0; y<(ssize_t) image->rows; y++)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              p=tim2_image_data+y*bytes_per_line;
              for (x=0; x < ((ssize_t) image->columns-1); x+=2)
              {
                SetPixelIndex(image,(*p) & 0x0F,q);
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
              SyncNewPixels(image,exception,y);
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
              p=tim2_image_data+y*bytes_per_line;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelIndex(image,*p,q);
                p++;
                q+=GetPixelChannels(image);
              }
              SyncNewPixels(image,exception,y);
            }
            break;
          }
        }
      }
      else //has_clut==false
      {
        switch (bits_per_pixel)
        {
          case 16:
          {
            uint16_t word;
            for (y=0; y<(ssize_t) image->rows; y++)
            {
              p=tim2_image_data+y*bytes_per_line;
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                word = ((uint16_t)* p   )<<0*8 |
                       ((uint16_t)*(p+1))<<1*8;

                SetPixelAllChannels(image,word,q,16);
                q+=GetPixelChannels(image);
                p+=sizeof(uint16_t);
              }
              SyncNewPixels(image,exception,y);
            }
            break;
          }

          case 24:
          {
            uint32_t word;
            for (y = 0; y<(ssize_t) image->rows; y++)
            {
              p=tim2_image_data+y*bytes_per_line;
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                word = (uint32_t)(* p   )<<0*8 |
                       (uint32_t)(*(p+1))<<1*8 |
                       (uint32_t)(*(p+2))<<2*8;

                SetPixelAllChannels(image,word,q,24);
                q+=GetPixelChannels(image);
                p+=3;
              }
              SyncNewPixels(image,exception,y);
            }
            break;
          }

          case 32:
          {  
            uint32_t word;
            for (y = 0; y<(ssize_t) image->rows; y++)
            {
              p=tim2_image_data+y*bytes_per_line;
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                word = ((uint32_t)* p   )<<0*8 |
                       ((uint32_t)*(p+1))<<1*8 |
                       ((uint32_t)*(p+2))<<2*8 |
                       ((uint32_t)*(p+3))<<3*8;

                SetPixelAllChannels(image,word,q,32);
                q+=GetPixelChannels(image);
                p+=4;
              }
              SyncNewPixels(image,exception,y);
            }
            break;
          }

          default:
          {
            tim2_image_data=(unsigned char *) RelinquishMagickMemory(tim2_image_data);
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          }
        }
      }



      tim2_image_data=(unsigned char *) RelinquishMagickMemory(tim2_image_data);

    }

    if (has_clut)
    {

      /*
       * ### Read CLUT Data ###
       */

      unsigned char
        *tim2_clut_data;
      register unsigned char
        *p;
      uint8_t linear_palette;

      tim2_clut_data=(unsigned char *) AcquireMagickMemory(tim2_image_header.clut_size);
      if (tim2_clut_data == (unsigned char *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

      count=ReadBlob(image,tim2_image_header.clut_size,tim2_clut_data);
      if (count != (ssize_t) (tim2_image_header.clut_size))
        {
          tim2_clut_data=(unsigned char *) RelinquishMagickMemory(tim2_clut_data);
          ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
        }
        
      /*
       * ### Process CLUT Data ###
       */

     
      p=tim2_clut_data;
      switch(clut_depth)
      {
        case 16:
        {
          uint16_t word;
          for (i=0;i<(ssize_t)image->colors;i++){
            word = ((uint16_t)* p   )<<0*8 |
                   ((uint16_t)*(p+1))<<1*8;

            image->colormap[i].red  =GetChannelValue16(word,0);
            image->colormap[i].green=GetChannelValue16(word,1);
            image->colormap[i].blue =GetChannelValue16(word,2);
            image->colormap[i].alpha=GetChannelAlpha16(word);
            p+=2;
          }
          break;
        }

        case 24:
        {
          uint32_t word;
          for (i=0;i<(ssize_t)image->colors;i++){
            word = ((uint32_t)* p   )<<0*8 |
                   ((uint32_t)*(p+1))<<1*8 |
                   ((uint32_t)*(p+2))<<2*8;

            image->colormap[i].red  =GetChannelValue24(word,0);
            image->colormap[i].green=GetChannelValue24(word,1);
            image->colormap[i].blue =GetChannelValue24(word,2);
            image->colormap[i].alpha=GetChannelAlpha24(word);
            p+=3;
          }
          break;
        }

        case 32:
        {
          uint32_t word;
          for (i=0;i<(ssize_t)image->colors;i++){
            word = ((uint32_t)* p   )<<0*8 |
                   ((uint32_t)*(p+1))<<1*8 |
                   ((uint32_t)*(p+2))<<2*8 |
                   ((uint32_t)*(p+3))<<3*8;
           
            image->colormap[i].red  =GetChannelValue32(word,0);
            image->colormap[i].green=GetChannelValue32(word,1);
            image->colormap[i].blue =GetChannelValue32(word,2);
            image->colormap[i].alpha=GetChannelAlpha32(word);
            p+=4;
          }
          break;
        }
      }
      tim2_clut_data=(unsigned char *) RelinquishMagickMemory(tim2_clut_data);


      linear_palette = (tim2_image_header.clut_type&0x80) != 0;
      if(!linear_palette){
        deshufflePalette(image,exception);
      }
    }

    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;


    if (image->storage_class == PseudoClass)
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

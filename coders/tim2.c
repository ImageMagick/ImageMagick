/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            TTTTT  IIIII  M   M                              %
%                              T      I    MM MM                              %
%                              T      I    M M M                              %
%                              T      I    M   M                              %
%                              T    IIIII  M   M                              %
%                                                                             %
%                                                                             %
%                           Read PSX TIM Image Format                         %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d T I M I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadTIMImage() reads a PSX TIM image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  Contributed by os@scee.sony.co.uk.
%
%  The format of the ReadTIMImage method is:
%
%      Image *ReadTIMImage(const ImageInfo *image_info,ExceptionInfo *exception)
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
  typedef struct _TIM2FileHeader
  {
    uint32_t magic_num;
    uint8_t format_type;
    uint8_t format_id;
    uint16_t image_count;
    char reserved[8];
  } TIM2FileHeader;

  TIM2FileHeader
    tim2_file_header;

  typedef struct _TIM2ImageHeader
  {
    size_t
      total_size,
      clut_size,
      image_size,
      header_size,
      clut_color_count;
    char img_format;
    size_t mipmap_count;
    char
      clut_type,
      bpp_type;
    size_t
      width,
      height;
    uint64_t
      GsTex0,
      GsTex1;
    uint32_t
      GsRegs,
      GsTexClut;
  } TIM2ImageHeader;

  TIM2ImageHeader
    tim2_image_header;

  Image
    *image;

  int
    bits_per_pixel,
    has_clut=0,
    clut_depth=0,
    csm=0;//CLUT Storage Mode

  MagickBooleanType
    status;

  register ssize_t
    x;

  register Quantum
    *q;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    bytes_per_line,
    height,
    image_size,
    pixel_mode,
    width;

  ssize_t
    count,
    y,
    str_read;

  unsigned char
    *tim_pixels;

  unsigned short
    word;

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
   *  Verify TIM2 magic number.
   */
  tim2_file_header.magic_num=ReadBlobMSBLong(image);
  if (tim2_file_header.magic_num != 0x54494D32) /*"TIM2"*/
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  /*
   * Read TIM2 header.
   */
  tim2_file_header.format_type=ReadBlobByte(image);
  tim2_file_header.format_id=ReadBlobByte(image);
  tim2_file_header.image_count=ReadBlobLSBShort(image);
  ReadBlobStream(image,8,&(tim2_file_header.reserved),&str_read);

   /*
   * Read each image. Only one image supported for now
   */
  if(tim2_file_header.image_count!=1)
    ThrowReaderException(CorruptImageError,"NumberOfImagesIsNotSupported");

  for(int i=0;i<tim2_file_header.image_count;++i)
  {
    tim2_image_header.total_size=ReadBlobLSBLong(image);
    tim2_image_header.clut_size=ReadBlobLSBLong(image);
    tim2_image_header.image_size=ReadBlobLSBLong(image);
    tim2_image_header.header_size=ReadBlobLSBShort(image);
    tim2_image_header.clut_color_count=ReadBlobLSBShort(image);
    tim2_image_header.img_format=ReadBlobByte(image);
    tim2_image_header.mipmap_count=ReadBlobByte(image);
    tim2_image_header.clut_type=ReadBlobByte(image);
    tim2_image_header.bpp_type=ReadBlobByte(image);
    tim2_image_header.width=ReadBlobLSBShort(image);
    tim2_image_header.height=ReadBlobLSBShort(image);
    
    tim2_image_header.GsTex0=ReadBlobMSBLongLong(image);
    tim2_image_header.GsTex1=ReadBlobMSBLongLong(image);
    tim2_image_header.GsRegs=ReadBlobMSBLong(image);
    tim2_image_header.GsRegs=ReadBlobMSBLong(image);

    /*
    ReadBlobStream(image,8,(tim2_image_header.GsTex0),&str_read);
    ReadBlobStream(image,8,(tim2_image_header.GsTex1),&str_read);
    ReadBlobStream(image,4,(tim2_image_header.GsRegs),&str_read);
    ReadBlobStream(image,4,(tim2_image_header.GsTexClut),&str_read);
    */

    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"GsTex0:%016x",tim2_image_header.GsTex1);
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"GsTex1:%016x",tim2_image_header.GsTex1);
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"GsRegs:%08x",tim2_image_header.GsRegs);
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"GsTexCLUT:%08x",tim2_image_header.GsTexClut);

    /*
     * Obtain color encoding format
     */
    has_clut=tim2_image_header.clut_type!=0;
    if(has_clut){
      switch ((int) tim2_image_header.clut_type>>4)//High 4 bits
      {
        case 0: csm=1;break;
        case 1: csm=2;break;
      }

      switch((int) tim2_image_header.clut_type&0x0F)//Low 4 bits
      {
        case 1: clut_depth=16;break;
        case 2: clut_depth=24; break;
        case 3: clut_depth=32; break;
      }
    }

    switch ((int) tim2_image_header.bpp_type)
    {
      case 1:bits_per_pixel=16;break;
      case 2:bits_per_pixel=24;break;
      case 3:bits_per_pixel=32;break;
      case 4:bits_per_pixel=4;break;// Implies CLUT
      case 5:bits_per_pixel=8;break;// Implies CLUT
    }

    image->depth=has_clut?clut_depth:bits_per_pixel;
    image->columns=tim2_image_header.width;
    image->rows=tim2_image_header.height;


    /*
     * Read Image Data
     */

    unsigned char
      *tim2_image_data;

    tim2_image_data=(unsigned char*) AcquireMagickMemory(tim2_image_header.image_size);
    if (tim2_image_data == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    count=ReadBlob(image,tim2_image_header.image_size,tim2_image_data);
    if (count!=(ssize_t)tim2_image_header.image_size)
    {
      tim2_image_data=(unsigned char *) RelinquishMagickMemory(tim2_image_data);
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    }
    
    if (has_clut)
    {
      image->colors=tim2_image_header.clut_color_count;

      /*
       * Read CLUT Data
       */

      unsigned char
        *tim_clut_data;

      if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

      tim_clut_data=(unsigned char *) AcquireMagickMemory(tim2_image_header.clut_size);
      if (tim_clut_data == (unsigned char *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

      count=ReadBlob(image,tim2_image_header.clut_size,tim_clut_data);

      if (count != (ssize_t) (tim2_image_header.clut_size))
        {
          tim_clut_data=(unsigned char *) RelinquishMagickMemory(tim_clut_data);
          ThrowReaderException(CorruptImageError,
            "InsufficientImageDataInFile");
        }
        
      /*
       * Process CLUT Data
       */

        p=tim_clut_data;
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"CLUT Depth: %i",clut_depth);
        switch(clut_depth)
        {
          case 16:
            for (i=0;i<(ssize_t)image->colors;i++){
              uint16_t word = (uint16_t)(* p   )<<1*8 |
                              (uint16_t)(*(p+1))<<0*8;

              image->colormap[i].red  =ScaleCharToQuantum(ScaleColor4to8(word >> 3*4 & 0x0F));
              image->colormap[i].green=ScaleCharToQuantum(ScaleColor4to8(word >> 2*4 & 0x0F));
              image->colormap[i].blue =ScaleCharToQuantum(ScaleColor4to8(word >> 1*4 & 0x0F));
              image->colormap[i].alpha=ScaleCharToQuantum((word >> 0*4 & 0x0F)==0?0:0xFF);
              p+=2;
            }
            break;

          case 24:
            for (i=0;i<(ssize_t)image->colors;i++){
              uint32_t word = (uint32_t)(* p   )<<2*8 |
                              (uint32_t)(*(p+1))<<1*8 |
                              (uint32_t)(*(p+2))<<0*8;

              image->colormap[i].red  =ScaleCharToQuantum(ScaleColor6to8(word>>3*6&0x3F));
              image->colormap[i].green=ScaleCharToQuantum(ScaleColor6to8(word>>2*6&0x3F));
              image->colormap[i].blue =ScaleCharToQuantum(ScaleColor6to8(word>>1*6&0x3F));
              image->colormap[i].alpha=ScaleCharToQuantum((word>>0*6&0x3F)==0?0:0xFF);
              p+=3;
            }
            break;

          case 32:
            for (i=0;i<(ssize_t)image->colors;i++){
              uint32_t word = ((uint32_t)* p   )<<3*8 |
                              ((uint32_t)*(p+1))<<2*8 |
                              ((uint32_t)*(p+2))<<1*8 |
                              ((uint32_t)*(p+3))<<0*8;
              uint8_t red   = word>>3*8&0xFF;
              uint8_t green = word>>2*8&0xFF;
              uint8_t blue  = word>>1*8&0xFF;
             
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),"CLUT[%i]:%08x",i,word);
              image->colormap[i].red  =ScaleCharToQuantum(word>>3*8&0xFF);
              image->colormap[i].green=ScaleCharToQuantum(word>>2*8&0xFF);
              image->colormap[i].blue =ScaleCharToQuantum(word>>1*8&0xFF);
              image->colormap[i].alpha=ScaleCharToQuantum((word>>0*8&0xFF)==0?0:0xFF);
              p+=sizeof(uint32_t);
            }
            break;
      }
      tim_clut_data=(unsigned char *) RelinquishMagickMemory(tim_clut_data);

    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    /*
     * Process image data.
     */
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    status=ResetImagePixels(image,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    /*
      Convert TIM2 raster image to pixel packets.
    */
    p=tim2_image_data;
    bytes_per_line=image->columns*bits_per_pixel/8;

    if(has_clut){
      switch (bits_per_pixel)
      {
        case 4:
        {
          for (y=0;y<(ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            p=tim2_image_data+y*bytes_per_line;
            for (x=0; x < ((ssize_t) image->columns-1); x+=2)
            {
              SetPixelIndex(image,(*p) & 0x0f,q);
              q+=GetPixelChannels(image);
              SetPixelIndex(image,(*p >> 4) & 0x0f,q);
              p++;
              q+=GetPixelChannels(image);
            }
            if ((image->columns % 2) != 0)
            {
              SetPixelIndex(image,(*p >> 4) & 0x0f,q);
              p++;
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
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Pixel[%i,%i]:%02x",y,x,*p);
              SetPixelIndex(image,*p++,q);
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
        break;
      }
    }
    else
    {
      switch (bits_per_pixel)
      {
        case 16:
        {
          /*
            Convert DirectColor scanline.
          */
          for (y=(ssize_t) image->rows-1; y >= 0; y--)
          {
            p=tim_pixels+y*bytes_per_line;
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              uint16_t word = (uint16_t)(* p   )<<1*8 |
                              (uint16_t)(*(p+1))<<0*8;

              SetPixelRed  (image,ScaleCharToQuantum(ScaleColor4to8(word >> 3*4 & 0x0F)),q);
              SetPixelGreen(image,ScaleCharToQuantum(ScaleColor4to8(word >> 2*4 & 0x0F)),q);
              SetPixelBlue (image,ScaleCharToQuantum(ScaleColor4to8(word >> 1*4 & 0x0F)),q);
              SetPixelAlpha(image,ScaleCharToQuantum((word>>0*4&0x0F)==0?0:0xFF),q);
              q+=GetPixelChannels(image);
              p+=2;
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
          break;
        }
        case 24:
        {
          /*
            Convert DirectColor scanline.
          */
          for (y = 0; y<(ssize_t) image->rows; y++)
          {
            p=tim_pixels+y*bytes_per_line;
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              uint32_t word = (uint32_t)(* p   )<<2*8 |
                              (uint32_t)(*(p+1))<<1*8 |
                              (uint32_t)(*(p+2))<<0*8;

              SetPixelRed  (image,ScaleCharToQuantum(ScaleColor4to8(word >> 3*6 & 0x3F)),q);
              SetPixelGreen(image,ScaleCharToQuantum(ScaleColor4to8(word >> 2*6 & 0x3F)),q);
              SetPixelBlue (image,ScaleCharToQuantum(ScaleColor4to8(word >> 1*6 & 0x3F)),q);
              SetPixelAlpha(image,ScaleCharToQuantum((word>>0*6&0x3F)==0?0:0xFF),q);
              q+=GetPixelChannels(image);
              p+=3;
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
          break;
        }
        case 32:
        {
          for (y = 0; y<(ssize_t) image->rows; y++)
          {
            p=tim_pixels+y*bytes_per_line;
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              uint32_t word = (uint32_t)(* p   )<<3*8 |
                              (uint32_t)(*(p+1))<<2*8 |
                              (uint32_t)(*(p+2))<<1*8 |
                              (uint32_t)(*(p+3))<<0*8;
               
              SetPixelRed  (image,ScaleCharToQuantum(ScaleColor4to8(word >> 3*8 & 0xFF)),q);
              SetPixelGreen(image,ScaleCharToQuantum(ScaleColor4to8(word >> 2*8 & 0xFF)),q);
              SetPixelBlue (image,ScaleCharToQuantum(ScaleColor4to8(word >> 1*8 & 0xFF)),q);
              SetPixelAlpha(image,ScaleCharToQuantum((word>>0*8&0xFF)==0?0:0xFF),q);
              q+=GetPixelChannels(image);
              p+=4;
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
          break;
        }
        default:
        {
          tim_pixels=(unsigned char *) RelinquishMagickMemory(tim_pixels);
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        }
      }
    }

    if (image->storage_class == PseudoClass)
      (void) SyncImage(image,exception);
    tim_pixels=(unsigned char *) RelinquishMagickMemory(tim_pixels);
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
    /*
    tim_info.id=ReadBlobLSBLong(image);
    if (tim_info.id == 0x00000010)
      {
        
          Allocate next image structure.
        
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
      */
    }
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
%   R e g i s t e r T I M I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterTIMImage() adds attributes for the TIM image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterTIMImage method is:
%
%      size_t RegisterTIMImage(void)
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
%   U n r e g i s t e r T I M I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterTIMImage() removes format registrations made by the
%  TIM module from the list of supported formats.
%
%  The format of the UnregisterTIMImage method is:
%
%      UnregisterTIMImage(void)
%
*/
ModuleExport void UnregisterTIM2Image(void)
{
  (void) UnregisterMagickInfo("TM2");
}

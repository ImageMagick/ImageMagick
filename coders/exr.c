/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            EEEEE  X   X  RRRR                               %
%                            E       X X   R   R                              %
%                            EEE      X    RRRR                               %
%                            E       X X   R R                                %
%                            EEEEE  X   X  R  R                               %
%                                                                             %
%                                                                             %
%            Read/Write High Dynamic-Range (HDR) Image File Format            %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 April 2007                                  %
%                                                                             %
%                                                                             %
%  Copyright @ 2007 ImageMagick Studio LLC, a non-profit organization         %
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
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/resource_.h"
#include "MagickCore/utility.h"
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
#include <OpenEXRConfig.h>
#include <ImfCRgbaFile.h>
/*
  Version 3.1.0 was released on 2021-07-21
  We can remove backwards compatibility in a couple years.
*/
#define OPENEXR_VERSION_HACK  \
    (((OPENEXR_VERSION_MAJOR) << 24) |  \
     ((OPENEXR_VERSION_MINOR) << 16) |  \
     ((OPENEXR_VERSION_PATCH) << 8))
#if (OPENEXR_VERSION_HACK) >= 0x3010000
#define USE_OPENEXR_CORE
#include <openexr.h>
#endif

/*
  Typedef declarations.
*/
#if defined(USE_OPENEXR_CORE)
typedef struct _EXRUserData
{
  ExceptionInfo
    *exception;
} EXRUserData;
#else
typedef struct _EXRWindowInfo
{
  int
    max_x,
    max_y,
    min_x,
    min_y;
} EXRWindowInfo;
#endif

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteEXRImage(const ImageInfo *,Image *,ExceptionInfo *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s E X R                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsEXR() returns MagickTrue if the image format type, identified by the
%  magick string, is EXR.
%
%  The format of the IsEXR method is:
%
%      MagickBooleanType IsEXR(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsEXR(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\166\057\061\001",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(MAGICKCORE_OPENEXR_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d E X R I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadEXRImage reads an image in the high dynamic-range (HDR) file format
%  developed by Industrial Light & Magic.  It allocates the memory necessary
%  for the new Image structure and returns a pointer to the new image.
%
%  The format of the ReadEXRImage method is:
%
%      Image *ReadEXRImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(USE_OPENEXR_CORE)
static void EXRError(exr_const_context_t ctxt,exr_result_t magick_unused(code),
  const char* msg)
{
  EXRUserData
    *user_data;

  magick_unreferenced(code);
  exr_get_user_data(ctxt,(void **) &user_data);
  (void) ThrowMagickException(user_data->exception,GetMagickModule(),
    CoderError,"CorruptImage","`%s'",msg);
}

static Image *ReadEXRImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  exr_attr_box2i_t
    data_window;
  
  exr_chunk_info_t
    cinfo;

  exr_coding_channel_info_t
    *channel;

  exr_compression_t
    compression = EXR_COMPRESSION_NONE;

  exr_context_t
    ctxt;

  exr_context_initializer_t
    initializer = EXR_DEFAULT_CONTEXT_INITIALIZER;

  exr_decode_pipeline_t
    decoder = EXR_DECODE_PIPELINE_INITIALIZER;

  exr_result_t
    result;

  exr_storage_t
    storage_type = EXR_STORAGE_LAST_TYPE;

  EXRUserData
    user_data = { 0 };

  Image
    *image;

  int
    part_index = 0;
  
  int16_t
    c;

  int32_t
    scans_per_chunk;

  MagickBooleanType
    status;

  PixelChannel
    pixel_channels[MaxPixelChannels] = { UndefinedPixelChannel };

  size_t
    pixel_count,
    pixel_size,
    number_meta_channels;

  ssize_t
    y;
  
  uint8_t
    *chunk,
    *p;

  /*
    Open image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  user_data.exception=exception;
  initializer.error_handler_fn=EXRError;
  initializer.user_data=(void *) &user_data;
  initializer.max_image_height=(int) MagickMin(INT_MAX,
    GetMagickResourceLimit(HeightResource));
  initializer.max_image_width=(int) MagickMin(INT_MAX,
    GetMagickResourceLimit(WidthResource));
  result=exr_start_read(&ctxt,image->filename,&initializer);
  if (result != EXR_ERR_SUCCESS)
    {
      exr_finish(&ctxt);
      ThrowReaderException(BlobError,"UnableToOpenBlob");
    }
  result=exr_get_data_window(ctxt,part_index,&data_window);
  if (result == EXR_ERR_SUCCESS)
    result=exr_get_compression(ctxt,part_index,&compression);
  if (result == EXR_ERR_SUCCESS)
    result=exr_get_storage(ctxt,part_index,&storage_type);
  if ((result != EXR_ERR_SUCCESS) ||
      (data_window.min.x >= data_window.max.x) ||
      (data_window.min.y >= data_window.max.y))
    {
      exr_finish(&ctxt);
      ThrowReaderException(CorruptImageError,"CorruptImage");
    }
  if (storage_type != EXR_STORAGE_SCANLINE)
    {
      exr_finish(&ctxt);
      (void) ThrowMagickException(exception,GetMagickModule(),
        CorruptImageError,"Unsupported storage type","`%d'",
        (int) storage_type);
      image=DestroyImageList(image);
      (void) CloseBlob(image);
      return((Image *) NULL);
    }
  image->columns=(size_t) (data_window.max.x-data_window.min.x+1L);
  image->rows=(size_t) (data_window.max.y-data_window.min.y+1L);
  image->gamma=1.0;
  image->compression=NoCompression;
  switch (compression)
  {
    case EXR_COMPRESSION_RLE: image->compression=RLECompression; break;
    case EXR_COMPRESSION_ZIPS: image->compression=ZipSCompression; break;
    case EXR_COMPRESSION_ZIP: image->compression=ZipCompression; break;
    case EXR_COMPRESSION_PIZ: image->compression=PizCompression; break;
    case EXR_COMPRESSION_PXR24: image->compression=Pxr24Compression; break;
    case EXR_COMPRESSION_B44: image->compression=B44Compression; break;
    case EXR_COMPRESSION_B44A: image->compression=B44ACompression; break;
    case EXR_COMPRESSION_DWAA: image->compression=DWAACompression; break;
    case EXR_COMPRESSION_DWAB: image->compression=DWABCompression; break;
    default: break;
  }
  if (image_info->ping != MagickFalse)
    {
      exr_finish(&ctxt);
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      exr_finish(&ctxt);
      (void) CloseBlob(image);
      return(DestroyImageList(image));
    }
  result=exr_get_scanlines_per_chunk(ctxt,part_index,&scans_per_chunk);
  if (result == EXR_ERR_SUCCESS)
    result=exr_read_scanline_chunk_info(ctxt,part_index,data_window.min.y,
      &cinfo);
  if (result == EXR_ERR_SUCCESS)
    result=exr_decoding_initialize(ctxt,part_index,&cinfo,&decoder);
  if (result != EXR_ERR_SUCCESS)
    {
      exr_finish(&ctxt);
      ThrowReaderException(CorruptImageError,"CorruptImage");
    }
  if (decoder.channel_count >= (MaxPixelChannels-MetaPixelChannels))
    {
      exr_decoding_destroy(ctxt,&decoder);
      exr_finish(&ctxt);
      ThrowReaderException(CorruptImageError,"MaximumChannelsExceeded");
    }
  channel=decoder.channels;
  pixel_size=0;
  number_meta_channels=0;
  for (c = 0; c < decoder.channel_count; ++c)
  {
    char
      channel_name[MagickPathExtent];

    if ((channel->data_type != EXR_PIXEL_HALF) &&
        (channel->data_type != EXR_PIXEL_FLOAT))
      {
        status=MagickFalse;
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"Unsupported channel data type","`%d'",
          (int) channel->data_type );
        break;
      }
    pixel_size+=channel->bytes_per_element;
    if (LocaleNCompare(channel->channel_name,"R",1) == 0)
      pixel_channels[c]=RedPixelChannel;
    else if (LocaleNCompare(channel->channel_name,"G",1) == 0)
      pixel_channels[c]=GreenPixelChannel;
    else if (LocaleNCompare(channel->channel_name,"B",1) == 0)
      pixel_channels[c]=BluePixelChannel;
    else if (LocaleNCompare(channel->channel_name,"A",1) == 0)
      {
        pixel_channels[c]=AlphaPixelChannel;
        image->alpha_trait=BlendPixelTrait;
      }
    else if (LocaleNCompare(channel->channel_name,"Y",1) == 0)
      pixel_channels[c]=IndexPixelChannel; /* Gray channel */
    else
      {
        pixel_channels[c]=(PixelChannel) (MetaPixelChannels+
          number_meta_channels);
        FormatLocaleString(channel_name,MagickPathExtent,
          "exr:meta-channel.%d",(int) number_meta_channels);
        number_meta_channels++;
        SetImageProperty(image,channel_name,channel->channel_name,exception);
      }
    channel++;
  }
  if (status != MagickFalse)
    status=SetPixelMetaChannels(image,number_meta_channels,exception);
  if (status == MagickFalse)
    {
      exr_decoding_destroy(ctxt,&decoder);
      exr_finish(&ctxt);
      (void) CloseBlob(image);
      return(DestroyImageList(image));
    }
  pixel_count=scans_per_chunk*image->columns;
  chunk=(uint8_t *) AcquireQuantumMemory(pixel_size,pixel_count);
  memset(chunk,0,pixel_size*pixel_count);
  p=chunk;
  channel=decoder.channels;
  for (c = 0; c < decoder.channel_count; ++c)
  {
    channel->decode_to_ptr=p;
    channel->user_pixel_stride=(int32_t) pixel_size;
    channel->user_line_stride=(int32_t) (image->columns*pixel_size);
    p+=channel->bytes_per_element;
    channel++;
  }
  for (y=0; y < (ssize_t) image->rows; y+= (ssize_t) scans_per_chunk)
  {
    Quantum
      *q;

    ssize_t
      x;

    if (y == 0)
      result=exr_decoding_choose_default_routines(ctxt,part_index,&decoder);
    else
      {
        int
          yy = (int) y + data_window.min.y;

        result=exr_read_scanline_chunk_info(ctxt,part_index,yy,&cinfo);
        if (result == EXR_ERR_SUCCESS)
          result=exr_decoding_update(ctxt,part_index,&cinfo,&decoder);
      }
    if (result == EXR_ERR_SUCCESS)
      result=exr_decoding_run(ctxt,part_index,&decoder);
    if (result != EXR_ERR_SUCCESS)
      break;
    q=QueueAuthenticPixels(image,0,y,image->columns,scans_per_chunk,exception);
    if (q == (Quantum *) NULL)
      break;
    p=chunk;
    for (x=0; x < (ssize_t) pixel_count; x++)
    {
      channel=decoder.channels;
      for (c = 0; c < decoder.channel_count; ++c)
      {
        float
          value;

        PixelChannel
          pixel_channel;

        if (channel->data_type == EXR_PIXEL_HALF)
          value=HalfToSinglePrecision(*(unsigned short *) p);
        else
          value=*(float *) p;
        pixel_channel=pixel_channels[c];
        if (pixel_channel == IndexPixelChannel)
          SetPixelGray(image,ClampToQuantum((double) QuantumRange*value),q);
        else
          SetPixelChannel(image,pixel_channel,ClampToQuantum(
            (double) QuantumRange*value),q);
        p+=channel->bytes_per_element;
        channel++;
      }
      q+=GetPixelChannels(image);
    }
  }
  chunk=(uint8_t *) RelinquishMagickMemory(chunk);
  exr_decoding_destroy(ctxt,&decoder);
  exr_finish(&ctxt);
  (void) CloseBlob(image);
  if (result != EXR_ERR_SUCCESS)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}
#else
static Image *ReadEXRImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  const ImfHeader
    *hdr_info;

  EXRWindowInfo
    data_window,
    display_window;

  Image
    *image;

  ImfInputFile
    *file;

  ImfRgba
    *scanline;

  int
    compression;

  MagickBooleanType
    status;

  Quantum
    *q;

  size_t
    columns;

  ssize_t
    y;

  /*
    Open image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  file=ImfOpenInputFile(image->filename);
  if (file == (ImfInputFile *) NULL)
    {
      ThrowFileException(exception,BlobError,"UnableToOpenBlob",
        ImfErrorMessage());
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  hdr_info=ImfInputHeader(file);
  ImfHeaderDataWindow(hdr_info,&display_window.min_x,&display_window.min_y,
    &display_window.max_x,&display_window.max_y);
  image->columns=(size_t) (display_window.max_x-display_window.min_x+1L);
  image->rows=(size_t) (display_window.max_y-display_window.min_y+1L);
  image->alpha_trait=BlendPixelTrait;
  (void) SetImageColorspace(image,RGBColorspace,exception);
  image->gamma=1.0;
  image->compression=NoCompression;
  compression=ImfHeaderCompression(hdr_info);
  if (compression == IMF_RLE_COMPRESSION)
    image->compression=RLECompression;
  if (compression == IMF_ZIPS_COMPRESSION)
    image->compression=ZipSCompression;
  if (compression == IMF_ZIP_COMPRESSION)
    image->compression=ZipCompression;
  if (compression == IMF_PIZ_COMPRESSION)
    image->compression=PizCompression;
  if (compression == IMF_PXR24_COMPRESSION)
    image->compression=Pxr24Compression;
#if defined(IMF_B44_COMPRESSION)
  if (compression == IMF_B44_COMPRESSION)
    image->compression=B44Compression;
#endif
#if defined(IMF_B44A_COMPRESSION)
  if (compression == IMF_B44A_COMPRESSION)
    image->compression=B44ACompression;
#endif
#if defined(IMF_DWAA_COMPRESSION)
  if (compression == IMF_DWAA_COMPRESSION)
    image->compression=DWAACompression;
#endif
#if defined(IMF_DWAB_COMPRESSION)
  if (compression == IMF_DWAB_COMPRESSION)
    image->compression=DWABCompression;
#endif
  if (image_info->ping != MagickFalse)
    {
      (void) ImfCloseInputFile(file);
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      (void) ImfCloseInputFile(file);
      return(DestroyImageList(image));
    }
  ImfHeaderDataWindow(hdr_info,&data_window.min_x,&data_window.min_y,
    &data_window.max_x,&data_window.max_y);
  columns=(size_t) (data_window.max_x-data_window.min_x+1L);
  if ((display_window.min_x > data_window.max_x) ||
      (display_window.min_x+(int) image->columns <= data_window.min_x))
    scanline=(ImfRgba *) NULL;
  else
    {
      scanline=(ImfRgba *) AcquireQuantumMemory(columns,sizeof(*scanline));
      if (scanline == (ImfRgba *) NULL)
        {
          (void) ImfCloseInputFile(file);
          image=DestroyImageList(image);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
    }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    int
      yy;

    ssize_t
      x;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    yy=(int) (display_window.min_y+y);
    if ((yy < data_window.min_y) || (yy > data_window.max_y) ||
        (scanline == (ImfRgba *) NULL))
      {
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelViaPixelInfo(image,&image->background_color,q);
          q+=GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        continue;
      }
    (void) memset(scanline,0,columns*sizeof(*scanline));
    if (ImfInputSetFrameBuffer(file,scanline-data_window.min_x-(ssize_t) columns*yy,1,columns) == 0)
      {
        status=MagickFalse;
        break;
      }
    if (ImfInputReadPixels(file,yy,yy) == 0)
      {
        status=MagickFalse;
        break;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      int
        xx;

      xx=(int) (display_window.min_x+x-data_window.min_x);
      if ((xx < 0) || (display_window.min_x+(int) x > data_window.max_x))
        SetPixelViaPixelInfo(image,&image->background_color,q);
      else
        {
          SetPixelRed(image,ClampToQuantum((double) QuantumRange*
            (double) ImfHalfToFloat(scanline[xx].r)),q);
          SetPixelGreen(image,ClampToQuantum((double) QuantumRange*
            (double) ImfHalfToFloat(scanline[xx].g)),q);
          SetPixelBlue(image,ClampToQuantum((double) QuantumRange*
            (double) ImfHalfToFloat(scanline[xx].b)),q);
          SetPixelAlpha(image,ClampToQuantum((double) QuantumRange*
            (double) ImfHalfToFloat(scanline[xx].a)),q);
        }
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  scanline=(ImfRgba *) RelinquishMagickMemory(scanline);
  (void) ImfCloseInputFile(file);
  if (status == MagickFalse)
    ThrowReaderException(CorruptImageError,"UnableToReadImageData");
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}
#endif
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r E X R I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterEXRImage() adds properties for the EXR image format
%  to the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterEXRImage method is:
%
%      size_t RegisterEXRImage(void)
%
*/
ModuleExport size_t RegisterEXRImage(void)
{
  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

  *version='\0';
  entry=AcquireMagickInfo("EXR","EXR","High Dynamic-range (HDR)");
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadEXRImage;
  entry->encoder=(EncodeImageHandler *) WriteEXRImage;
#if defined( OPENEXR_PACKAGE_STRING)
  (void) FormatLocaleString(version,MagickPathExtent,OPENEXR_PACKAGE_STRING);
#endif
#endif
  entry->magick=(IsImageFormatHandler *) IsEXR;
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r E X R I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterEXRImage() removes format registrations made by the
%  EXR module from the list of supported formats.
%
%  The format of the UnregisterEXRImage method is:
%
%      UnregisterEXRImage(void)
%
*/
ModuleExport void UnregisterEXRImage(void)
{
  (void) UnregisterMagickInfo("EXR");
}

#if defined(MAGICKCORE_OPENEXR_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e E X R I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteEXRImage() writes an image to a file the in the high dynamic-range
%  (HDR) file format developed by Industrial Light & Magic.
%
%  The format of the WriteEXRImage method is:
%
%      MagickBooleanType WriteEXRImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteEXRImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  const char
    *sampling_factor,
    *value;

  ImageInfo
    *write_info;

  ImfHalf
    half_quantum;

  ImfHeader
    *hdr_info;

  ImfOutputFile
    *file;

  ImfRgba
    *scanline;

  int
    channels,
    compression,
    factors[3];

  MagickBooleanType
    status;

  const Quantum
    *p;

  ssize_t
    x;

  ssize_t
    y;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  (void) SetImageColorspace(image,RGBColorspace,exception);
  write_info=CloneImageInfo(image_info);
  (void) AcquireUniqueFilename(write_info->filename);
  hdr_info=ImfNewHeader();
  ImfHeaderSetDataWindow(hdr_info,0,0,(int) image->columns-1,(int)
    image->rows-1);
  ImfHeaderSetDisplayWindow(hdr_info,0,0,(int) image->columns-1,(int)
    image->rows-1);
  compression=IMF_NO_COMPRESSION;
  if (write_info->compression == RLECompression)
    compression=IMF_RLE_COMPRESSION;
  if (write_info->compression == ZipSCompression)
    compression=IMF_ZIPS_COMPRESSION;
  if (write_info->compression == ZipCompression)
    compression=IMF_ZIP_COMPRESSION;
  if (write_info->compression == PizCompression)
    compression=IMF_PIZ_COMPRESSION;
  if (write_info->compression == Pxr24Compression)
    compression=IMF_PXR24_COMPRESSION;
#if defined(IMF_B44_COMPRESSION)
  if (write_info->compression == B44Compression)
    compression=IMF_B44_COMPRESSION;
#endif
#if defined(IMF_B44A_COMPRESSION)
  if (write_info->compression == B44ACompression)
    compression=IMF_B44A_COMPRESSION;
#endif
#if defined(IMF_DWAA_COMPRESSION)
  if (write_info->compression == DWAACompression)
    compression=IMF_DWAA_COMPRESSION;
#endif
#if defined(IMF_DWAB_COMPRESSION)
  if (write_info->compression == DWABCompression)
    compression=IMF_DWAB_COMPRESSION;
#endif
  channels=0;
  value=GetImageOption(image_info,"exr:color-type");
  if (value != (const char *) NULL)
    {
      if (LocaleCompare(value,"RGB") == 0)
        channels=IMF_WRITE_RGB;
      else if (LocaleCompare(value,"RGBA") == 0)
        channels=IMF_WRITE_RGBA;
      else if (LocaleCompare(value,"YC") == 0)
        channels=IMF_WRITE_YC;
      else if (LocaleCompare(value,"YCA") == 0)
        channels=IMF_WRITE_YCA;
      else if (LocaleCompare(value,"Y") == 0)
        channels=IMF_WRITE_Y;
      else if (LocaleCompare(value,"YA") == 0)
        channels=IMF_WRITE_YA;
      else if (LocaleCompare(value,"R") == 0)
        channels=IMF_WRITE_R;
      else if (LocaleCompare(value,"G") == 0)
        channels=IMF_WRITE_G;
      else if (LocaleCompare(value,"B") == 0)
        channels=IMF_WRITE_B;
      else if (LocaleCompare(value,"A") == 0)
        channels=IMF_WRITE_A;
      else
        (void) ThrowMagickException(exception,GetMagickModule(),CoderWarning,
          "ignoring invalid defined exr:color-type","=%s",value);
   }
  sampling_factor=(const char *) NULL;
  factors[0]=0;
  if (image_info->sampling_factor != (char *) NULL)
    sampling_factor=image_info->sampling_factor;
  if (sampling_factor != NULL)
    {
      /*
        Sampling factors, valid values are 1x1 or 2x2.
      */
      if (sscanf(sampling_factor,"%d:%d:%d",factors,factors+1,factors+2) == 3)
        {
          if ((factors[0] == factors[1]) && (factors[1] == factors[2]))
            factors[0]=1;
          else
            if ((factors[0] == (2*factors[1])) && (factors[2] == 0))
              factors[0]=2;
        }
      else
        if (sscanf(sampling_factor,"%dx%d",factors,factors+1) == 2)
          {
            if (factors[0] != factors[1])
              factors[0]=0;
          }
      if ((factors[0] != 1) && (factors[0] != 2))
        (void) ThrowMagickException(exception,GetMagickModule(),CoderWarning,
          "ignoring sampling-factor","=%s",sampling_factor);
      else if (channels != 0)
        {
          /*
            Cross check given color type and subsampling.
          */
          factors[1]=((channels == IMF_WRITE_YCA) ||
            (channels == IMF_WRITE_YC)) ? 2 : 1;
          if (factors[0] != factors[1])
            (void) ThrowMagickException(exception,GetMagickModule(),
              CoderWarning,"sampling-factor and color type mismatch","=%s",
              sampling_factor);
        }
    }
  if (channels == 0)
    {
      /*
        If no color type given, select it now.
      */
      if (factors[0] == 2)
        channels=image->alpha_trait != UndefinedPixelTrait ? IMF_WRITE_YCA :
          IMF_WRITE_YC;
      else
        channels=image->alpha_trait != UndefinedPixelTrait ? IMF_WRITE_RGBA :
          IMF_WRITE_RGB;
    }
  ImfHeaderSetCompression(hdr_info,compression);
  ImfHeaderSetLineOrder(hdr_info,IMF_INCREASING_Y);
  file=ImfOpenOutputFile(write_info->filename,hdr_info,channels);
  ImfDeleteHeader(hdr_info);
  if (file == (ImfOutputFile *) NULL)
    {
      (void) RelinquishUniqueFileResource(write_info->filename);
      write_info=DestroyImageInfo(write_info);
      ThrowFileException(exception,BlobError,"UnableToOpenBlob",
        ImfErrorMessage());
      return(MagickFalse);
    }
  scanline=(ImfRgba *) AcquireQuantumMemory(image->columns,sizeof(*scanline));
  if (scanline == (ImfRgba *) NULL)
    {
      (void) ImfCloseOutputFile(file);
      (void) RelinquishUniqueFileResource(write_info->filename);
      write_info=DestroyImageInfo(write_info);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  memset(scanline,0,image->columns*sizeof(*scanline));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ImfFloatToHalf(QuantumScale*(double) GetPixelRed(image,p),&half_quantum);
      scanline[x].r=half_quantum;
      ImfFloatToHalf(QuantumScale*(double) GetPixelGreen(image,p),
        &half_quantum);
      scanline[x].g=half_quantum;
      ImfFloatToHalf(QuantumScale*(double) GetPixelBlue(image,p),&half_quantum);
      scanline[x].b=half_quantum;
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        ImfFloatToHalf(1.0,&half_quantum);
      else
        ImfFloatToHalf(QuantumScale*(double) GetPixelAlpha(image,p),
          &half_quantum);
      scanline[x].a=half_quantum;
      p+=GetPixelChannels(image);
    }
    ImfOutputSetFrameBuffer(file,scanline-(y*(ssize_t) image->columns),1,
      image->columns);
    ImfOutputWritePixels(file,1);
  }
  (void) ImfCloseOutputFile(file);
  scanline=(ImfRgba *) RelinquishMagickMemory(scanline);
  (void) FileToImage(image,write_info->filename,exception);
  (void) RelinquishUniqueFileResource(write_info->filename);
  write_info=DestroyImageInfo(write_info);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}
#endif

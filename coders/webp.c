/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         W   W  EEEEE  BBBB   PPPP                           %
%                         W   W  E      B   B  P   P                          %
%                         W W W  EEE    BBBB   PPPP                           %
%                         WW WW  E      B   B  P                              %
%                         W   W  EEEEE  BBBB   P                              %
%                                                                             %
%                                                                             %
%                         Read/Write WebP Image Format                        %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 March 2011                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/client.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/display.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/layer.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"
#include "MagickCore/xwindow.h"
#include "MagickCore/xwindow-private.h"
#if defined(MAGICKCORE_WEBP_DELEGATE)
#include <webp/decode.h>
#include <webp/encode.h>
#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
#include <webp/mux.h>
#include <webp/demux.h>
#endif
#endif

/*
  Forward declarations.
*/
#if defined(MAGICKCORE_WEBP_DELEGATE)
static MagickBooleanType
  WriteWEBPImage(const ImageInfo *,Image *,ExceptionInfo *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s W E B P                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsWEBP() returns MagickTrue if the image format type, identified by the
%  magick string, is WebP.
%
%  The format of the IsWEBP method is:
%
%      MagickBooleanType IsWEBP(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsWEBP(const unsigned char *magick,const size_t length)
{
  if (length < 12)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick+8,"WEBP",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(MAGICKCORE_WEBP_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d W E B P I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadWEBPImage() reads an image in the WebP image format.
%
%  The format of the ReadWEBPImage method is:
%
%      Image *ReadWEBPImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline uint32_t ReadWebPLSBWord(
  const unsigned char *magick_restrict data)
{
  const unsigned char
    *p;

  uint32_t
    value;

  p=data;
  value=(uint32_t) (*p++);
  value|=((uint32_t) (*p++)) << 8;
  value|=((uint32_t) (*p++)) << 16;
  value|=((uint32_t) (*p++)) << 24;
  return(value);
}

static MagickBooleanType IsWEBPImageLossless(const unsigned char *stream,
  const size_t length)
{
#define VP8_CHUNK_INDEX  15
#define LOSSLESS_FLAG  'L'
#define EXTENDED_HEADER  'X'
#define VP8_CHUNK_HEADER  "VP8"
#define VP8_CHUNK_HEADER_SIZE  3
#define RIFF_HEADER_SIZE  12
#define VP8X_CHUNK_SIZE  10
#define TAG_SIZE  4
#define CHUNK_SIZE_BYTES  4
#define CHUNK_HEADER_SIZE  8
#define MAX_CHUNK_PAYLOAD  (~0U-CHUNK_HEADER_SIZE-1)

  size_t
    offset;

  /*
    Read simple header.
  */
  if (length <= VP8_CHUNK_INDEX)
    return(MagickFalse);
  if (stream[VP8_CHUNK_INDEX] != EXTENDED_HEADER)
    return(stream[VP8_CHUNK_INDEX] == LOSSLESS_FLAG ? MagickTrue : MagickFalse);
  /*
    Read extended header.
  */
  offset=RIFF_HEADER_SIZE+TAG_SIZE+CHUNK_SIZE_BYTES+VP8X_CHUNK_SIZE;
  while (offset <= (length-TAG_SIZE-TAG_SIZE-4))
  {
    uint32_t
      chunk_size,
      chunk_size_pad;

    chunk_size=ReadWebPLSBWord(stream+offset+TAG_SIZE);
    if (chunk_size > MAX_CHUNK_PAYLOAD)
      break;
    chunk_size_pad=(CHUNK_HEADER_SIZE+chunk_size+1) & ~1;
    if (memcmp(stream+offset,VP8_CHUNK_HEADER,VP8_CHUNK_HEADER_SIZE) == 0)
      return(*(stream+offset+VP8_CHUNK_HEADER_SIZE) == LOSSLESS_FLAG ?
        MagickTrue : MagickFalse);
    offset+=chunk_size_pad;
  }
  return(MagickFalse);
}

static int FillBasicWEBPInfo(Image *image,const uint8_t *stream,size_t length,
  WebPDecoderConfig *configure)
{
  WebPBitstreamFeatures
    *magick_restrict features = &configure->input;

  int
    webp_status;

  webp_status=WebPGetFeatures(stream,length,features);

  if (webp_status != VP8_STATUS_OK)
    return(webp_status);

  image->columns=(size_t) features->width;
  image->rows=(size_t) features->height;
  image->depth=8;
  image->alpha_trait=features->has_alpha != 0 ? BlendPixelTrait :
        UndefinedPixelTrait;

  return(webp_status);
}

static int ReadSingleWEBPImage(Image *image,const uint8_t *stream,
  size_t length,WebPDecoderConfig *configure,ExceptionInfo *exception,
  MagickBooleanType is_first)
{
  int
    webp_status;

  unsigned char
    *p;

  size_t
    canvas_width,
    canvas_height,
    image_width,
    image_height;

  ssize_t
    x_offset,
    y_offset,
    y;

  WebPDecBuffer
    *magick_restrict webp_image;

  MagickBooleanType
    status;

  webp_image=&configure->output;
  if (is_first)
    {
      canvas_width=image->columns;
      canvas_height=image->rows;
      x_offset=image->page.x;
      y_offset=image->page.y;
      image->page.x=0;
      image->page.y=0;
    }
  else
    {
      canvas_width=0;
      canvas_height=0;
      x_offset=0;
      y_offset=0;
    }
  webp_status=FillBasicWEBPInfo(image,stream,length,configure);
  image_width=image->columns;
  image_height=image->rows;
  if (is_first)
    {
      image->columns=canvas_width;
      image->rows=canvas_height;
    }

  if (webp_status != VP8_STATUS_OK)
    return(webp_status);

  if (IsWEBPImageLossless(stream,length) != MagickFalse)
    image->quality=100;

  webp_status=WebPDecode(stream,length,configure);
  if (webp_status != VP8_STATUS_OK)
    return(webp_status);

  p=(unsigned char *) webp_image->u.RGBA.rgba;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *q;

    ssize_t
      x;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((x >= x_offset && x < (ssize_t) (x_offset+image_width)) &&
          (y >= y_offset && y < (ssize_t) (y_offset+image_height)))
        {
          SetPixelRed(image,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
          SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
          SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
        }
      else
        {
          SetPixelRed(image,0,q);
          SetPixelGreen(image,0,q);
          SetPixelBlue(image,0,q);
          SetPixelAlpha(image,0,q);
        }
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  WebPFreeDecBuffer(webp_image);
#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
  {
    StringInfo
      *profile;

    uint32_t
      webp_flags = 0;

    WebPData
     chunk,
     content;

    WebPMux
      *mux;

    /*
      Extract any profiles:
      https://developers.google.com/speed/webp/docs/container-api.
    */
    content.bytes=stream;
    content.size=length;
    mux=WebPMuxCreate(&content,0);
    (void) memset(&chunk,0,sizeof(chunk));
    WebPMuxGetFeatures(mux,&webp_flags);
    if (webp_flags & ICCP_FLAG)
      {
        WebPMuxGetChunk(mux,"ICCP",&chunk);
        profile=BlobToStringInfo(chunk.bytes,chunk.size);
        if (profile != (StringInfo *) NULL)
          {
            SetImageProfile(image,"ICC",profile,exception);
            profile=DestroyStringInfo(profile);
          }
      }
    if (webp_flags & EXIF_FLAG)
      {
        WebPMuxGetChunk(mux,"EXIF",&chunk);
        profile=BlobToStringInfo(chunk.bytes,chunk.size);
        if (profile != (StringInfo *) NULL)
          {
            SetImageProfile(image,"EXIF",profile,exception);
            profile=DestroyStringInfo(profile);
          }
      }
    if (webp_flags & XMP_FLAG)
      {
        WebPMuxGetChunk(mux,"XMP",&chunk);
        profile=BlobToStringInfo(chunk.bytes,chunk.size);
        if (profile != (StringInfo *) NULL)
          {
            SetImageProfile(image,"XMP",profile,exception);
            profile=DestroyStringInfo(profile);
          }
      }
    WebPMuxDelete(mux);
  }
#endif
  return(webp_status);
}

#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
static int ReadAnimatedWEBPImage(const ImageInfo *image_info,Image *image,
  uint8_t *stream,size_t length,WebPDecoderConfig *configure,
  ExceptionInfo *exception)
{
  Image
    *original_image;

  int
    image_count,
    webp_status;

  size_t
    canvas_width,
    canvas_height;

  WebPData
    data;

  WebPDemuxer
    *demux;

  WebPIterator
    iter;

  image_count=0;
  webp_status=0;
  original_image=image;
  webp_status=FillBasicWEBPInfo(image,stream,length,configure);
  canvas_width=image->columns;
  canvas_height=image->rows;
  data.bytes=stream;
  data.size=length;
  {
    WebPMux
      *mux;

    WebPMuxAnimParams
      params;

    WebPMuxError
      status;

    mux=WebPMuxCreate(&data,0);
    status=WebPMuxGetAnimationParams(mux,&params);
    if (status >= 0)
      image->iterations=params.loop_count;
    WebPMuxDelete(mux);
  }
  demux=WebPDemux(&data);
  if (WebPDemuxGetFrame(demux,1,&iter))
    {
      do
      {
        if (image_count != 0)
          {
            AcquireNextImage(image_info,image,exception);
            if (GetNextImageInList(image) == (Image *) NULL)
              break;
            image=SyncNextImageInList(image);
            CloneImageProperties(image,original_image);
            image->page.x=iter.x_offset;
            image->page.y=iter.y_offset;
            webp_status=ReadSingleWEBPImage(image,iter.fragment.bytes,
              iter.fragment.size,configure,exception,MagickFalse);
          }
        else
          {
            image->page.x=iter.x_offset;
            image->page.y=iter.y_offset;
            webp_status=ReadSingleWEBPImage(image,iter.fragment.bytes,
              iter.fragment.size,configure,exception,MagickTrue);
          }
        if (webp_status != VP8_STATUS_OK)
          break;
        image->page.width=canvas_width;
        image->page.height=canvas_height;
        image->ticks_per_second=100;
        image->delay=iter.duration/10;
        image->dispose=NoneDispose;
        if (iter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND)
          image->dispose=BackgroundDispose;
        (void) SetImageProperty(image,"webp:mux-blend",
          "AtopPreviousAlphaBlend",exception);
        if (iter.blend_method == WEBP_MUX_BLEND)
          (void) SetImageProperty(image,"webp:mux-blend",
            "AtopBackgroundAlphaBlend",exception);
        image_count++;
      } while (WebPDemuxNextFrame(&iter));
      WebPDemuxReleaseIterator(&iter);
    }
  WebPDemuxDelete(demux);
  return(webp_status);
}
#endif

static Image *ReadWEBPImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define ThrowWEBPException(severity,tag) \
{ \
  if (stream != (unsigned char *) NULL) \
    stream=(unsigned char*) RelinquishMagickMemory(stream); \
  if (webp_image != (WebPDecBuffer *) NULL) \
    WebPFreeDecBuffer(webp_image); \
  ThrowReaderException(severity,tag); \
}

  Image
    *image;

  int
    webp_status;

  MagickBooleanType
    status;

  size_t
    length;

  ssize_t
    count;

  unsigned char
    header[12],
    *stream;

  WebPDecoderConfig
    configure;

  WebPDecBuffer
    *magick_restrict webp_image = &configure.output;

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
  stream=(unsigned char *) NULL;
  if (WebPInitDecoderConfig(&configure) == 0)
    ThrowReaderException(ResourceLimitError,"UnableToDecodeImageFile");
  webp_image->colorspace=MODE_RGBA;
  count=ReadBlob(image,12,header);
  if (count != 12)
    ThrowWEBPException(CorruptImageError,"InsufficientImageDataInFile");
  status=IsWEBP(header,count);
  if (status == MagickFalse)
    ThrowWEBPException(CorruptImageError,"CorruptImage");
  length=(size_t) (ReadWebPLSBWord(header+4)+8);
  if (length < 12)
    ThrowWEBPException(CorruptImageError,"CorruptImage");
  if (length > GetBlobSize(image))
    ThrowWEBPException(CorruptImageError,"InsufficientImageDataInFile");
  stream=(unsigned char *) AcquireQuantumMemory(length,sizeof(*stream));
  if (stream == (unsigned char *) NULL)
    ThrowWEBPException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memcpy(stream,header,12);
  count=ReadBlob(image,length-12,stream+12);
  if (count != (ssize_t) (length-12))
    ThrowWEBPException(CorruptImageError,"InsufficientImageDataInFile");

  webp_status=FillBasicWEBPInfo(image,stream,length,&configure);
  if (webp_status == VP8_STATUS_OK) {
    if (configure.input.has_animation) {
#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
      webp_status=ReadAnimatedWEBPImage(image_info,image,stream,length,
        &configure,exception);
#else
      webp_status=VP8_STATUS_UNSUPPORTED_FEATURE;
#endif
    } else {
      webp_status=ReadSingleWEBPImage(image,stream,length,&configure,exception,MagickFalse);
    }
  }

  if (webp_status != VP8_STATUS_OK)
    switch (webp_status)
    {
      case VP8_STATUS_OUT_OF_MEMORY:
      {
        ThrowWEBPException(ResourceLimitError,"MemoryAllocationFailed");
        break;
      }
      case VP8_STATUS_INVALID_PARAM:
      {
        ThrowWEBPException(CorruptImageError,"invalid parameter");
        break;
      }
      case VP8_STATUS_BITSTREAM_ERROR:
      {
        ThrowWEBPException(CorruptImageError,"CorruptImage");
        break;
      }
      case VP8_STATUS_UNSUPPORTED_FEATURE:
      {
        ThrowWEBPException(CoderError,"DataEncodingSchemeIsNotSupported");
        break;
      }
      case VP8_STATUS_SUSPENDED:
      {
        ThrowWEBPException(CorruptImageError,"decoder suspended");
        break;
      }
      case VP8_STATUS_USER_ABORT:
      {
        ThrowWEBPException(CorruptImageError,"user abort");
        break;
      }
      case VP8_STATUS_NOT_ENOUGH_DATA:
      {
        ThrowWEBPException(CorruptImageError,"InsufficientImageDataInFile");
        break;
      }
      default:
        ThrowWEBPException(CorruptImageError,"CorruptImage");
    }

  stream=(unsigned char*) RelinquishMagickMemory(stream);
  (void) CloseBlob(image);
  return(image);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r W E B P I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterWEBPImage() adds attributes for the WebP image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterWEBPImage method is:
%
%      size_t RegisterWEBPImage(void)
%
*/
ModuleExport size_t RegisterWEBPImage(void)
{
  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

  *version='\0';
  entry=AcquireMagickInfo("WEBP","WEBP","WebP Image Format");
#if defined(MAGICKCORE_WEBP_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadWEBPImage;
  entry->encoder=(EncodeImageHandler *) WriteWEBPImage;
  (void) FormatLocaleString(version,MagickPathExtent,"libwebp %d.%d.%d [%04X]",
    (WebPGetEncoderVersion() >> 16) & 0xff,
    (WebPGetEncoderVersion() >> 8) & 0xff,
    (WebPGetEncoderVersion() >> 0) & 0xff,WEBP_ENCODER_ABI_VERSION);
#endif
  entry->mime_type=ConstantString("image/webp");
  entry->flags|=CoderDecoderSeekableStreamFlag;
#if !defined(MAGICKCORE_WEBPMUX_DELEGATE)
  entry->flags^=CoderAdjoinFlag;
#endif
  entry->magick=(IsImageFormatHandler *) IsWEBP;
  if (*version != '\0')
    entry->version=ConstantString(version);
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r W E B P I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterWEBPImage() removes format registrations made by the WebP module
%  from the list of supported formats.
%
%  The format of the UnregisterWEBPImage method is:
%
%      UnregisterWEBPImage(void)
%
*/
ModuleExport void UnregisterWEBPImage(void)
{
  (void) UnregisterMagickInfo("WEBP");
}
#if defined(MAGICKCORE_WEBP_DELEGATE)

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e W E B P I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteWEBPImage() writes an image in the WebP image format.
%
%  The format of the WriteWEBPImage method is:
%
%      MagickBooleanType WriteWEBPImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

#if WEBP_ENCODER_ABI_VERSION >= 0x0100
static int WebPEncodeProgress(int percent,const WebPPicture* picture)
{
#define EncodeImageTag  "Encode/Image"

  Image
    *image;

  MagickBooleanType
    status;

  image=(Image *) picture->user_data;
  status=SetImageProgress(image,EncodeImageTag,percent-1,100);
  return(status == MagickFalse ? 0 : 1);
}
#endif

static const char * WebPErrorCodeMessage(WebPEncodingError error_code)
{
  switch (error_code)
  {
    case VP8_ENC_OK:
      return NULL;
    case VP8_ENC_ERROR_OUT_OF_MEMORY:
      return "out of memory";
    case VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY:
      return "bitstream out of memory";
    case VP8_ENC_ERROR_NULL_PARAMETER:
      return "NULL parameter";
    case VP8_ENC_ERROR_INVALID_CONFIGURATION:
      return "invalid configuration";
    case VP8_ENC_ERROR_BAD_DIMENSION:
      return "bad dimension";
    case VP8_ENC_ERROR_PARTITION0_OVERFLOW:
      return "partition 0 overflow (> 512K)";
    case VP8_ENC_ERROR_PARTITION_OVERFLOW:
      return "partition overflow (> 16M)";
    case VP8_ENC_ERROR_BAD_WRITE:
      return "bad write";
    case VP8_ENC_ERROR_FILE_TOO_BIG:
      return "file too big (> 4GB)";
#if WEBP_ENCODER_ABI_VERSION >= 0x0100
    case VP8_ENC_ERROR_USER_ABORT:
      return "user abort";
    case VP8_ENC_ERROR_LAST:
      return "error last";
#endif
  }
  return "unknown exception";
}

static MagickBooleanType WriteSingleWEBPPicture(const ImageInfo *image_info,
  Image *image,WebPConfig *configure,WebPPicture *picture,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  uint32_t
    *magick_restrict q;

  ssize_t
    y;

  MemoryInfo
    *pixel_info;

#if WEBP_ENCODER_ABI_VERSION >= 0x0100
  picture->progress_hook=WebPEncodeProgress;
  picture->user_data=(void *) image;
#endif
  picture->width=(int) image->columns;
  picture->height=(int) image->rows;
  picture->argb_stride=(int) image->columns;
  picture->use_argb=1;

  /*
    Allocate memory for pixels.
  */
  (void) TransformImageColorspace(image,sRGBColorspace,exception);
  pixel_info=AcquireVirtualMemory(image->columns,image->rows*
    sizeof(*(picture->argb)));

  if (pixel_info == (MemoryInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  picture->argb=(uint32_t *) GetVirtualMemoryBlob(pixel_info);
  /*
    Convert image to WebP raster pixels.
  */
  status=MagickFalse;
  q=picture->argb;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      *q++=(uint32_t) (image->alpha_trait != UndefinedPixelTrait ? (uint32_t)
        ScaleQuantumToChar(GetPixelAlpha(image,p)) << 24 : 0xff000000) |
        ((uint32_t) ScaleQuantumToChar(GetPixelRed(image,p)) << 16) |
        ((uint32_t) ScaleQuantumToChar(GetPixelGreen(image,p)) << 8) |
        ((uint32_t) ScaleQuantumToChar(GetPixelBlue(image,p)));
      p+=GetPixelChannels(image);
    }
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }

  if (status != MagickFalse)
    status=(MagickBooleanType) WebPEncode(configure, picture);

  RelinquishVirtualMemory(pixel_info);

  if (status == MagickFalse)
    (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
      WebPErrorCodeMessage(picture->error_code),"`%s'",
      image->filename);

  return(status);
}

static MagickBooleanType WriteSingleWEBPImage(const ImageInfo *image_info,
  Image *image,WebPConfig *configure,WebPMemoryWriter *writer,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  WebPPicture
    picture;

  if (WebPPictureInit(&picture) == 0)
    ThrowWriterException(ResourceLimitError,"UnableToEncodeImageFile");

  picture.writer=WebPMemoryWrite;
  picture.custom_ptr=writer;

  status=WriteSingleWEBPPicture(image_info,image,configure,&picture,exception);
  WebPPictureFree(&picture);

  return(status);
}

#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
static MagickBooleanType WriteAnimatedWEBPImage(const ImageInfo *image_info,
  Image *image,WebPConfig *configure,WebPData *webp_data,
  ExceptionInfo *exception)
{
  Image
    *first_image;

  size_t
    effective_delta,
    frame_timestamp;

  WebPAnimEncoder
    *enc;

  WebPAnimEncoderOptions
    enc_options;

  WebPPicture
    picture;

  MagickBooleanType
    status;

  first_image=CoalesceImages(image,exception);
  if (first_image == (Image *) NULL)
    return(MagickFalse);
  image=first_image;
  effective_delta=0;
  frame_timestamp=0;

  (void) WebPAnimEncoderOptionsInit(&enc_options);
  if (image_info->verbose)
    enc_options.verbose=1;
  enc=WebPAnimEncoderNew((int) image->page.width,(int) image->page.height,
    &enc_options);

  status=MagickTrue;
  while (image != NULL)
  {
    status=(MagickBooleanType) WebPPictureInit(&picture);
    if (status == MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"UnableToEncodeImageFile","`%s'",image->filename);
        break;
      }

    status=WriteSingleWEBPPicture(image_info,image,configure,&picture,
      exception);
    if (status == MagickFalse)
      break;

    status=(MagickBooleanType) WebPAnimEncoderAdd(enc,&picture,
      (int) frame_timestamp,configure);
    WebPPictureFree(&picture);
    if (status == MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CoderError,WebPAnimEncoderGetError(enc),"`%s'",image->filename);
        break;
      }

    effective_delta=image->delay*1000*PerceptibleReciprocal(
      image->ticks_per_second);
    if (effective_delta < 10)
      effective_delta=100; /* Consistent with gif2webp */
    frame_timestamp+=effective_delta;

    image=GetNextImageInList(image);
  }

  if (status != MagickFalse)
    {
      // add last null frame and assemble picture.
      status=(MagickBooleanType) WebPAnimEncoderAdd(enc,(WebPPicture *) NULL,
        (int) frame_timestamp,configure);
      if (status != MagickFalse)
        status=(MagickBooleanType) WebPAnimEncoderAssemble(enc,webp_data);

      if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            CoderError,WebPAnimEncoderGetError(enc),"`%s'",
            first_image->filename);
    }

  WebPAnimEncoderDelete(enc);
  DestroyImageList(first_image);
  return(status);
}

static MagickBooleanType WriteWEBPImageProfile(Image *image,
  WebPData *webp_data,ExceptionInfo *exception) 
{
  const StringInfo
    *icc_profile,
    *exif_profile,
    *xmp_profile;
 
  WebPData
    chunk;
 
  WebPMux
    *mux;
 
  WebPMuxError
    mux_error;

  WebPMuxAnimParams
    new_params;
 
  icc_profile=GetImageProfile(image,"ICC");
  exif_profile=GetImageProfile(image,"EXIF");
  xmp_profile=GetImageProfile(image,"XMP");

  if (icc_profile == (StringInfo *) NULL 
          && exif_profile == (StringInfo *) NULL 
          && xmp_profile == (StringInfo *) NULL
          && image->iterations == 0)
    return(MagickTrue);

  mux=WebPMuxCreate(webp_data, 1);
  WebPDataClear(webp_data);

  if (mux == NULL)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"UnableToEncodeImageFile","`%s'",image->filename);

  // Clean up returned data
  memset(webp_data, 0, sizeof(*webp_data));
  mux_error=WEBP_MUX_OK;
  if (image->iterations > 0)
    {
      mux_error=WebPMuxGetAnimationParams(mux, &new_params);
      /*
        If there is only 1 frame webp_data will be created by WriteSingleWEBPImage
        and WebPMuxGetAnimationParams will return WEBP_MUX_NOT_FOUND
      */
      if (mux_error == WEBP_MUX_NOT_FOUND)
        mux_error=WEBP_MUX_OK;
      else
        if (mux_error == WEBP_MUX_OK)
          {
            new_params.loop_count=MagickMin((int) image->iterations,65535);
            mux_error=WebPMuxSetAnimationParams(mux, &new_params);
          }
    }
  if ((icc_profile != (StringInfo *) NULL) && (mux_error == WEBP_MUX_OK))
    {
      chunk.bytes=GetStringInfoDatum(icc_profile);
      chunk.size=GetStringInfoLength(icc_profile);
      mux_error=WebPMuxSetChunk(mux,"ICCP",&chunk,0);
    }
  if ((exif_profile != (StringInfo *) NULL) && (mux_error == WEBP_MUX_OK))
    {
      chunk.bytes=GetStringInfoDatum(exif_profile);
      chunk.size=GetStringInfoLength(exif_profile);
      if ((chunk.size >= 6) &&
          (chunk.bytes[0] == 'E') && (chunk.bytes[1] == 'x') &&
          (chunk.bytes[2] == 'i') && (chunk.bytes[3] == 'f') &&
          (chunk.bytes[4] == '\0') && (chunk.bytes[5] == '\0'))
        {
          chunk.bytes=GetStringInfoDatum(exif_profile)+6;
          chunk.size-=6;
        }
      mux_error=WebPMuxSetChunk(mux,"EXIF",&chunk,0);
    }
  if ((xmp_profile != (StringInfo *) NULL) && (mux_error == WEBP_MUX_OK))
    {
      chunk.bytes=GetStringInfoDatum(xmp_profile);
      chunk.size=GetStringInfoLength(xmp_profile);
      mux_error=WebPMuxSetChunk(mux,"XMP",&chunk,0);
    }
  if (mux_error == WEBP_MUX_OK)
      mux_error=WebPMuxAssemble(mux,webp_data);
  WebPMuxDelete(mux);

  if (mux_error != WEBP_MUX_OK)
    (void) ThrowMagickException(exception,GetMagickModule(),
      ResourceLimitError,"UnableToEncodeImageFile","`%s'",image->filename);
  return(MagickTrue);
}
#endif

static inline void SetBooleanOption(const ImageInfo *image_info,
  const char *option,int *setting)
{
  const char
    *value;

  value=GetImageOption(image_info,option);
  if (value != (char *) NULL)
    *setting=(int) ParseCommandOption(MagickBooleanOptions,MagickFalse,value);
}

static inline void SetIntegerOption(const ImageInfo *image_info,
  const char *option,int *setting)
{
  const char
    *value;

  value=GetImageOption(image_info,option);
  if (value != (const char *) NULL)
    *setting=StringToInteger(value);
}

static MagickBooleanType WriteWEBPImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo * exception)
{
  const char
    *value;

  MagickBooleanType
    status;

  WebPConfig
    configure;

  WebPMemoryWriter
    writer;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->columns > 16383UL) || (image->rows > 16383UL))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if (WebPConfigInit(&configure) == 0)
    ThrowWriterException(ResourceLimitError,"UnableToEncodeImageFile");
  if (image->quality != UndefinedCompressionQuality)
    configure.quality=(float) image->quality;
  if (image->quality >= 100)
    configure.lossless=1;
  SetBooleanOption(image_info,"webp:lossless",&configure.lossless);
  value=GetImageOption(image_info,"webp:image-hint");
  if (value != (char *) NULL)
    {
      if (LocaleCompare(value,"default") == 0)
        configure.image_hint=WEBP_HINT_DEFAULT;
      if (LocaleCompare(value,"photo") == 0)
        configure.image_hint=WEBP_HINT_PHOTO;
      if (LocaleCompare(value,"picture") == 0)
        configure.image_hint=WEBP_HINT_PICTURE;
#if WEBP_ENCODER_ABI_VERSION >= 0x0200
      if (LocaleCompare(value,"graph") == 0)
        configure.image_hint=WEBP_HINT_GRAPH;
#endif
    }
  SetBooleanOption(image_info,"webp:auto-filter",&configure.autofilter);
  value=GetImageOption(image_info,"webp:target-psnr");
  if (value != (char *) NULL)
    configure.target_PSNR=(float) StringToDouble(value,(char **) NULL);
  SetIntegerOption(image_info,"webp:alpha-compression",
    &configure.alpha_compression);
  SetIntegerOption(image_info,"webp:alpha-filtering",
    &configure.alpha_filtering);
  SetIntegerOption(image_info,"webp:alpha-quality",
    &configure.alpha_quality);
  SetIntegerOption(image_info,"webp:filter-strength",
    &configure.filter_strength);
  SetIntegerOption(image_info,"webp:filter-sharpness",
    &configure.filter_sharpness);
  SetIntegerOption(image_info,"webp:filter-type",&configure.filter_type);
  SetIntegerOption(image_info,"webp:method",&configure.method);
  SetIntegerOption(image_info,"webp:partitions",&configure.partitions);
  SetIntegerOption(image_info,"webp:partition-limit",
    &configure.partition_limit);
  SetIntegerOption(image_info,"webp:pass",&configure.pass);
  SetIntegerOption(image_info,"webp:preprocessing",&configure.preprocessing);
  SetIntegerOption(image_info,"webp:segments",&configure.segments);
  SetIntegerOption(image_info,"webp:show-compressed",
    &configure.show_compressed);
  SetIntegerOption(image_info,"webp:sns-strength",&configure.sns_strength);
  SetIntegerOption(image_info,"webp:target-size",&configure.target_size);
#if WEBP_ENCODER_ABI_VERSION >= 0x0201
  SetBooleanOption(image_info,"webp:emulate-jpeg-size",
    &configure.emulate_jpeg_size);
  SetBooleanOption(image_info,"webp:low-memory",&configure.low_memory);
  SetIntegerOption(image_info,"webp:thread-level",&configure.thread_level);
#endif
#if WEBP_ENCODER_ABI_VERSION >= 0x0209
  SetBooleanOption(image_info,"webp:exact",&configure.exact);
#endif
#if WEBP_ENCODER_ABI_VERSION >= 0x020e
  SetIntegerOption(image_info,"webp:near-lossless",&configure.near_lossless);
  SetBooleanOption(image_info,"webp:use-sharp-yuv",&configure.use_sharp_yuv);
#endif
  if (WebPValidateConfig(&configure) == 0)
    ThrowWriterException(ResourceLimitError,"UnableToEncodeImageFile");
#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
  {
    WebPData
      webp_data;

    memset(&webp_data,0,sizeof(webp_data));
    if ((image_info->adjoin != MagickFalse) &&
        (GetPreviousImageInList(image) == (Image *) NULL) &&
        (GetNextImageInList(image) != (Image *) NULL))
      status=WriteAnimatedWEBPImage(image_info,image,&configure,&webp_data,
        exception);
    else
      {
        WebPMemoryWriterInit(&writer);
        status=WriteSingleWEBPImage(image_info,image,&configure,&writer,
          exception);
        if (status == MagickFalse)
          WebPMemoryWriterClear(&writer);
        else
          {
            webp_data.bytes=writer.mem;
            webp_data.size=writer.size;
          }
      }
    if (status != MagickFalse)
      status=WriteWEBPImageProfile(image,&webp_data,exception);
    if (status != MagickFalse)
      (void) WriteBlob(image,webp_data.size,webp_data.bytes);
    WebPDataClear(&webp_data);
  }
#else
  WebPMemoryWriterInit(&writer);
  status=WriteSingleWEBPImage(image_info,image,&configure,&writer,
    exception);
  if (status != MagickFalse)
    (void) WriteBlob(image,writer.size,writer.mem);
  WebPMemoryWriterClear(&writer);
#endif
  (void) CloseBlob(image);
  return(status);
}
#endif

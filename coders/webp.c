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
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
  register const unsigned char
    *p;

  register uint32_t
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

  register unsigned char
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
    *magick_restrict webp_image = &configure->output;

  MagickBooleanType
    status;

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
    register Quantum
      *q;

    register ssize_t
      x;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((x >= x_offset && x < x_offset + image_width) &&
          (y >= y_offset && y < y_offset + image_height))
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
      Extract any profiles.
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
  demux=WebPDemux(&data);
  if (WebPDemuxGetFrame(demux,1,&iter)) {
    do {
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
      if (iter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND)
        image->dispose=BackgroundDispose;
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

#if !defined(MAGICKCORE_WEBPMUX_DELEGATE)
static int WebPEncodeWriter(const unsigned char *stream,size_t length,
  const WebPPicture *const picture)
{
  Image
    *image;

  image=(Image *) picture->custom_ptr;
  return(length != 0 ? (WriteBlob(image,length,stream) == (ssize_t) length) : 1);
}
#endif

typedef struct PictureMemory {
  MemoryInfo *pixel_info;
  struct PictureMemory *next;
} PictureMemory;

static MagickBooleanType WriteSingleWEBPImage(const ImageInfo *image_info,
  Image *image,WebPPicture *picture,PictureMemory *picture_memory,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status = MagickFalse;

  register uint32_t
    *magick_restrict q;

  ssize_t
    y;

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
  picture_memory->pixel_info=AcquireVirtualMemory(image->columns,image->rows*
    sizeof(*(picture->argb)));

  if (picture_memory->pixel_info == (MemoryInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  picture->argb=(uint32_t *) GetVirtualMemoryBlob(picture_memory->pixel_info);
  /*
    Convert image to WebP raster pixels.
  */
  q=picture->argb;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
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
  return status;
}

#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
static void FreePictureMemoryList (PictureMemory* head) {
  PictureMemory* next;
  while(head != NULL) {
    next = head->next;
    if(head->pixel_info != NULL)
      RelinquishVirtualMemory(head->pixel_info);
    free(head);
    head = next;
  }
}

static MagickBooleanType WriteAnimatedWEBPImage(const ImageInfo *image_info,
  Image *image,WebPConfig *configure,WebPMemoryWriter *writer_info,
  ExceptionInfo *exception)
{
  Image
    *first_image;

  PictureMemory
    *current,
    *head;

  size_t
    effective_delta = 0,
    frame_timestamp = 0;

  WebPAnimEncoder
    *enc;

  WebPAnimEncoderOptions
    enc_options;

  WebPData
    webp_data;

  WebPPicture
    picture;

  WebPAnimEncoderOptionsInit(&enc_options);
  if (image_info->verbose)
    enc_options.verbose = 1;

  image=CoalesceImages(image, exception);
  first_image=image;
  enc=WebPAnimEncoderNew((int) image->page.width,(int) image->page.height,
    &enc_options);

  head=(PictureMemory *) calloc(sizeof(*head),1);
  current=head;

  while (image != NULL)
  {
    if (WebPPictureInit(&picture) == 0)
      ThrowWriterException(ResourceLimitError,"UnableToEncodeImageFile");

    WriteSingleWEBPImage(image_info, image, &picture, current, exception);

    effective_delta = image->delay*1000/image->ticks_per_second;
    if (effective_delta < 10)
      effective_delta = 100; /* Consistent with gif2webp */
    frame_timestamp+=effective_delta;

    WebPAnimEncoderAdd(enc,&picture,(int) frame_timestamp,configure);

    image = GetNextImageInList(image);
    current->next=(PictureMemory *) calloc(sizeof(*head), 1);
    current = current->next;
  }
  webp_data.bytes=writer_info->mem;
  webp_data.size=writer_info->size;
  WebPAnimEncoderAssemble(enc, &webp_data);
  WebPMemoryWriterClear(writer_info);
  writer_info->size=webp_data.size;
  writer_info->mem=(unsigned char *) webp_data.bytes;
  WebPAnimEncoderDelete(enc);
  DestroyImageList(first_image);
  FreePictureMemoryList(head);
  return(MagickTrue);
}
#endif

static MagickBooleanType WriteWEBPImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo * exception)
{
  const char
    *value;

  int
    webp_status;

  MagickBooleanType
    status;

  WebPAuxStats
    statistics;

  WebPConfig
    configure;

#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
  WebPMemoryWriter
    writer_info;
#endif

  WebPPicture
    picture;

  PictureMemory
    memory = {0};

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
  if (WebPPictureInit(&picture) == 0)
    ThrowWriterException(ResourceLimitError,"UnableToEncodeImageFile");
#if !defined(MAGICKCORE_WEBPMUX_DELEGATE)
  picture.writer=WebPEncodeWriter;
  picture.custom_ptr=(void *) image;
#else
  WebPMemoryWriterInit(&writer_info);
  picture.writer=WebPMemoryWrite;
  picture.custom_ptr=(&writer_info);
#endif
  picture.stats=(&statistics);
  if (image->quality != UndefinedCompressionQuality)
    configure.quality=(float) image->quality;
  if (image->quality >= 100)
    configure.lossless=1;
  value=GetImageOption(image_info,"webp:lossless");
  if (value != (char *) NULL)
    configure.lossless=(int) ParseCommandOption(MagickBooleanOptions,
      MagickFalse,value);
  value=GetImageOption(image_info,"webp:method");
  if (value != (char *) NULL)
    configure.method=StringToInteger(value);
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
  value=GetImageOption(image_info,"webp:target-size");
  if (value != (char *) NULL)
    configure.target_size=StringToInteger(value);
  value=GetImageOption(image_info,"webp:target-psnr");
  if (value != (char *) NULL)
    configure.target_PSNR=(float) StringToDouble(value,(char **) NULL);
  value=GetImageOption(image_info,"webp:segments");
  if (value != (char *) NULL)
    configure.segments=StringToInteger(value);
  value=GetImageOption(image_info,"webp:sns-strength");
  if (value != (char *) NULL)
    configure.sns_strength=StringToInteger(value);
  value=GetImageOption(image_info,"webp:filter-strength");
  if (value != (char *) NULL)
    configure.filter_strength=StringToInteger(value);
  value=GetImageOption(image_info,"webp:filter-sharpness");
  if (value != (char *) NULL)
    configure.filter_sharpness=StringToInteger(value);
  value=GetImageOption(image_info,"webp:filter-type");
  if (value != (char *) NULL)
    configure.filter_type=StringToInteger(value);
  value=GetImageOption(image_info,"webp:auto-filter");
  if (value != (char *) NULL)
    configure.autofilter=(int) ParseCommandOption(MagickBooleanOptions,
      MagickFalse,value);
  value=GetImageOption(image_info,"webp:alpha-compression");
  if (value != (char *) NULL)
    configure.alpha_compression=StringToInteger(value);
  value=GetImageOption(image_info,"webp:alpha-filtering");
  if (value != (char *) NULL)
    configure.alpha_filtering=StringToInteger(value);
  value=GetImageOption(image_info,"webp:alpha-quality");
  if (value != (char *) NULL)
    configure.alpha_quality=StringToInteger(value);
  value=GetImageOption(image_info,"webp:pass");
  if (value != (char *) NULL)
    configure.pass=StringToInteger(value);
  value=GetImageOption(image_info,"webp:show-compressed");
  if (value != (char *) NULL)
    configure.show_compressed=StringToInteger(value);
  value=GetImageOption(image_info,"webp:preprocessing");
  if (value != (char *) NULL)
    configure.preprocessing=StringToInteger(value);
  value=GetImageOption(image_info,"webp:partitions");
  if (value != (char *) NULL)
    configure.partitions=StringToInteger(value);
  value=GetImageOption(image_info,"webp:partition-limit");
  if (value != (char *) NULL)
    configure.partition_limit=StringToInteger(value);
#if WEBP_ENCODER_ABI_VERSION >= 0x0201
  value=GetImageOption(image_info,"webp:emulate-jpeg-size");
  if (value != (char *) NULL)
    configure.emulate_jpeg_size=(int) ParseCommandOption(MagickBooleanOptions,
      MagickFalse,value);
  value=GetImageOption(image_info,"webp:low-memory");
  if (value != (char *) NULL)
    configure.low_memory=(int) ParseCommandOption(MagickBooleanOptions,
      MagickFalse,value);
  value=GetImageOption(image_info,"webp:thread-level");
  if (value != (char *) NULL)
    configure.thread_level=StringToInteger(value);
#endif
#if WEBP_ENCODER_ABI_VERSION >= 0x020e
  value=GetImageOption(image_info,"webp:use-sharp-yuv");
  if (value != (char *) NULL)
    configure.use_sharp_yuv=StringToInteger(value);
#endif
  if (WebPValidateConfig(&configure) == 0)
    ThrowWriterException(ResourceLimitError,"UnableToEncodeImageFile");

  WriteSingleWEBPImage(image_info,image,&picture,&memory,exception);

#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
  if ((image_info->adjoin != MagickFalse) &&
      (GetPreviousImageInList(image) == (Image *) NULL) &&
      (GetNextImageInList(image) != (Image *) NULL) &&
      (image->iterations != 1))
    WriteAnimatedWEBPImage(image_info,image,&configure,&writer_info,exception);
#endif

  webp_status=WebPEncode(&configure,&picture);
  if (webp_status == 0)
    {
      const char
        *message;

      switch (picture.error_code)
      {
        case VP8_ENC_ERROR_OUT_OF_MEMORY:
        {
          message="out of memory";
          break;
        }
        case VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY:
        {
          message="bitstream out of memory";
          break;
        }
        case VP8_ENC_ERROR_NULL_PARAMETER:
        {
          message="NULL parameter";
          break;
        }
        case VP8_ENC_ERROR_INVALID_CONFIGURATION:
        {
          message="invalid configuration";
          break;
        }
        case VP8_ENC_ERROR_BAD_DIMENSION:
        {
          message="bad dimension";
          break;
        }
        case VP8_ENC_ERROR_PARTITION0_OVERFLOW:
        {
          message="partition 0 overflow (> 512K)";
          break;
        }
        case VP8_ENC_ERROR_PARTITION_OVERFLOW:
        {
          message="partition overflow (> 16M)";
          break;
        }
        case VP8_ENC_ERROR_BAD_WRITE:
        {
          message="bad write";
          break;
        }
        case VP8_ENC_ERROR_FILE_TOO_BIG:
        {
          message="file too big (> 4GB)";
          break;
        }
#if WEBP_ENCODER_ABI_VERSION >= 0x0100
        case VP8_ENC_ERROR_USER_ABORT:
        {
          message="user abort";
          break;
        }
#endif
        default:
        {
          message="unknown exception";
          break;
        }
      }
      (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
        (char *) message,"`%s'",image->filename);
    }
#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
  {
    const StringInfo
      *profile;

    WebPData
      chunk,
      image_chunk;

    WebPMux
      *mux;

    WebPMuxError
      mux_error;

    /*
      Set image profiles (if any).
    */
    image_chunk.bytes=writer_info.mem;
    image_chunk.size=writer_info.size;
    mux_error=WEBP_MUX_OK;
    (void) memset(&chunk,0,sizeof(chunk));
    mux=WebPMuxNew();
    profile=GetImageProfile(image,"ICC");
    if ((profile != (StringInfo *) NULL) && (mux_error == WEBP_MUX_OK))
      {
        chunk.bytes=GetStringInfoDatum(profile);
        chunk.size=GetStringInfoLength(profile);
        mux_error=WebPMuxSetChunk(mux,"ICCP",&chunk,0);
      }
    profile=GetImageProfile(image,"EXIF");
    if ((profile != (StringInfo *) NULL) && (mux_error == WEBP_MUX_OK))
      {
        chunk.bytes=GetStringInfoDatum(profile);
        chunk.size=GetStringInfoLength(profile);
        mux_error=WebPMuxSetChunk(mux,"EXIF",&chunk,0);
      }
    profile=GetImageProfile(image,"XMP");
    if ((profile != (StringInfo *) NULL) && (mux_error == WEBP_MUX_OK))
      {
        chunk.bytes=GetStringInfoDatum(profile);
        chunk.size=GetStringInfoLength(profile);
        mux_error=WebPMuxSetChunk(mux,"XMP",&chunk,0);
      }
    if (mux_error != WEBP_MUX_OK)
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"UnableToEncodeImageFile","`%s'",image->filename);
    if (chunk.size != 0)
      {
        WebPData
          picture_profiles;

        /*
          Replace original container with image profile (if any).
        */
        picture_profiles.bytes=writer_info.mem;
        picture_profiles.size=writer_info.size;
        WebPMuxSetImage(mux,&image_chunk,1);
        mux_error=WebPMuxAssemble(mux,&picture_profiles);
        WebPMemoryWriterClear(&writer_info);
        writer_info.size=picture_profiles.size;
        writer_info.mem=(unsigned char *) picture_profiles.bytes;
      }
    WebPMuxDelete(mux);
  }
  (void) WriteBlob(image,writer_info.size,writer_info.mem);
#endif
  picture.argb=(uint32_t *) NULL;
  WebPPictureFree(&picture);
#if defined(MAGICKCORE_WEBPMUX_DELEGATE)
  WebPMemoryWriterClear(&writer_info);
#endif
  (void) CloseBlob(image);
  RelinquishVirtualMemory(memory.pixel_info);
  return(webp_status == 0 ? MagickFalse : MagickTrue);
}
#endif

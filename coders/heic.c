/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        H   H  EEEEE  IIIII   CCCC                           %
%                        H   H  E        I    C                               %
%                        HHHHH  EEE      I    C                               %
%                        H   H  E        I    C                               %
%                        H   H  EEEEE  IIIII   CCCC                           %
%                                                                             %
%                                                                             %
%                        Read/Write Heic Image Format                         %
%                                                                             %
%                                 Dirk Farin                                  %
%                                 April 2018                                  %
%                                                                             %
%                         Copyright 2018 Struktur AG                          %
%                                                                             %
%                               Anton Kortunov                                %
%                               December 2017                                 %
%                                                                             %
%                      Copyright 2017-2018 YANDEX LLC.                        %
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
#include "MagickCore/property.h"
#include "MagickCore/display.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/transform.h"
#include "MagickCore/distort.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"
#if defined(MAGICKCORE_HEIC_DELEGATE)
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
#include <heif.h>
#else
#include <libheif/heif.h>
#endif
#endif

#if defined(MAGICKCORE_HEIC_DELEGATE)
/*
  Define declarations.
*/
#define XmpNamespaceExtent  28

/*
  Const declarations.
*/
static const char
  xmp_namespace[] = "http://ns.adobe.com/xap/1.0/ ";

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteHEICImage(const ImageInfo *,Image *,ExceptionInfo *);

/*x
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d H E I C I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadHEICImage retrieves an image via a file descriptor, decodes the image,
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadHEICImage method is:
%
%      Image *ReadHEICImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType IsHeifSuccess(Image *image,struct heif_error *error,
  ExceptionInfo *exception)
{
  if (error->code == 0)
    return(MagickTrue);
  ThrowBinaryException(CorruptImageError,error->message,image->filename);
}

static MagickBooleanType ReadHEICColorProfile(Image *image,
  struct heif_image_handle *image_handle,ExceptionInfo *exception)
{
#if LIBHEIF_NUMERIC_VERSION >= 0x01040000
  size_t
    length;

  /*
    Read color profile.
  */
  length=heif_image_handle_get_raw_color_profile_size(image_handle);
  if (length > 0)
    {
      unsigned char
        *color_buffer;

      if ((MagickSizeType) length > GetBlobSize(image))
        ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
          image->filename);
      color_buffer=(unsigned char *) AcquireQuantumMemory(1,length);
      if (color_buffer != (unsigned char *) NULL)
        {
          struct heif_error
            error;

          error=heif_image_handle_get_raw_color_profile(image_handle,
            color_buffer);
          if (error.code == 0)
            {
              StringInfo
                *profile;

              profile=BlobToStringInfo(color_buffer,length);
              if (profile != (StringInfo*) NULL)
                {
                  (void) SetImageProfile(image,"icc",profile,exception);
                  profile=DestroyStringInfo(profile);
                }
            }
        }
      color_buffer=(unsigned char *) RelinquishMagickMemory(color_buffer);
    }
#endif
  return(MagickTrue);
}

static MagickBooleanType ReadHEICExifProfile(Image *image,
  struct heif_image_handle *image_handle,ExceptionInfo *exception)
{
  heif_item_id
    exif_id;

  int
    count;

  /*
    Read Exif profile.
  */
  count=heif_image_handle_get_list_of_metadata_block_IDs(image_handle,"Exif",
    &exif_id,1);
  if (count > 0)
    {
      size_t
        exif_size;

      unsigned char
        *exif_buffer;

      exif_size=heif_image_handle_get_metadata_size(image_handle,exif_id);
      if ((MagickSizeType) exif_size > GetBlobSize(image))
        ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
          image->filename);
      exif_buffer=(unsigned char *) AcquireQuantumMemory(1,exif_size);
      if (exif_buffer != (unsigned char *) NULL)
        {
          struct heif_error
            error;

          error=heif_image_handle_get_metadata(image_handle,
            exif_id,exif_buffer);
          if (error.code == 0)
            {
              StringInfo
                *profile;

              /*
                Skip first 4 bytes, the offset to the TIFF header.
              */
              profile=(StringInfo*) NULL;
              if (exif_size > 8)
                profile=BlobToStringInfo(exif_buffer+4,(size_t) exif_size-4);
              if (profile != (StringInfo*) NULL)
                {
                  (void) SetImageProfile(image,"exif",profile,exception);
                  profile=DestroyStringInfo(profile);
                }
            }
        }
      exif_buffer=(unsigned char *) RelinquishMagickMemory(exif_buffer);
  }
  return(MagickTrue);
}

static inline MagickBooleanType HEICSkipImage(const ImageInfo *image_info,
  Image *image)
{
  if (image_info->number_scenes == 0)
    return(MagickFalse);
  if (image->scene == 0)
    return(MagickFalse);
  if (image->scene < image_info->scene)
    return(MagickTrue);
  if (image->scene > image_info->scene+image_info->number_scenes-1)
    return(MagickTrue);
  return(MagickFalse);
}

static MagickBooleanType ReadHEICImageHandle(const ImageInfo *image_info,
  Image *image,struct heif_image_handle *image_handle,
  ExceptionInfo *exception)
{
  const uint8_t
    *p;

  int
    stride = 0;

  MagickBooleanType
    preserve_orientation,
    status;

  ssize_t
    y;

  struct heif_decoding_options
    *decode_options;

  struct heif_error
    error;

  struct heif_image
    *heif_image;

  /*
    Read HEIC image from container.
  */
  image->columns=(size_t) heif_image_handle_get_width(image_handle);
  image->rows=(size_t) heif_image_handle_get_height(image_handle);
  image->depth=8;
#if LIBHEIF_NUMERIC_VERSION > 0x01040000
  {
    int
      bits_per_pixel;

    bits_per_pixel=heif_image_handle_get_luma_bits_per_pixel(image_handle);
    if (bits_per_pixel != -1)
      image->depth=(size_t) bits_per_pixel;
  }
#endif
  if (heif_image_handle_has_alpha_channel(image_handle))
    image->alpha_trait=BlendPixelTrait;
  preserve_orientation=IsStringTrue(GetImageOption(image_info,
    "heic:preserve-orientation"));
  if (preserve_orientation == MagickFalse)
    (void) SetImageProperty(image,"exif:Orientation","1",exception);
  if (ReadHEICColorProfile(image,image_handle,exception) == MagickFalse)
    return(MagickFalse);
  if (ReadHEICExifProfile(image,image_handle,exception) == MagickFalse)
    return(MagickFalse);
  if (image_info->ping != MagickFalse)
    return(MagickTrue);
  if (HEICSkipImage(image_info,image) != MagickFalse)
    return(MagickTrue);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(MagickFalse);
  decode_options=heif_decoding_options_alloc();
#if LIBHEIF_NUMERIC_VERSION > 0x01070000
  decode_options->convert_hdr_to_8bit=1;
#endif
  if (preserve_orientation == MagickTrue)
    decode_options->ignore_transformations=1;
  error=heif_decode_image(image_handle,&heif_image,heif_colorspace_RGB,
    image->alpha_trait != UndefinedPixelTrait ? heif_chroma_interleaved_RGBA :
    heif_chroma_interleaved_RGB,decode_options);
  heif_decoding_options_free(decode_options);
  if (IsHeifSuccess(image,&error,exception) == MagickFalse)
    return(MagickFalse);
  image->columns=(size_t) heif_image_get_width(heif_image,
    heif_channel_interleaved);
  image->rows=(size_t) heif_image_get_height(heif_image
    ,heif_channel_interleaved);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      heif_image_release(heif_image);
      return(MagickFalse);
    }
  p=heif_image_get_plane_readonly(heif_image,heif_channel_interleaved,&stride);
  stride-=(int) (image->columns * (image->alpha_trait != UndefinedPixelTrait ?
    4 : 3));
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
      SetPixelRed(image,ScaleCharToQuantum((unsigned char) *(p++)),q);
      SetPixelGreen(image,ScaleCharToQuantum((unsigned char) *(p++)),q);
      SetPixelBlue(image,ScaleCharToQuantum((unsigned char) *(p++)),q);
      if (image->alpha_trait != UndefinedPixelTrait)
        SetPixelAlpha(image,ScaleCharToQuantum((unsigned char) *(p++)),q);
      q+=GetPixelChannels(image);
    }
    p+=stride;
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  heif_image_release(heif_image);
  return(MagickTrue);
}

static void ReadHEICDepthImage(const ImageInfo *image_info,Image *image,
  struct heif_image_handle *image_handle,ExceptionInfo *exception)
{
  const char
    *option;

  heif_item_id
    depth_id;

  int
    number_images;

  struct heif_error
    error;

  struct heif_image_handle
    *depth_handle;

  option=GetImageOption(image_info,"heic:depth-image");
  if (IsStringTrue(option) == MagickFalse)
    return;
  if (heif_image_handle_has_depth_image(image_handle) == 0)
    return;
  number_images=heif_image_handle_get_list_of_depth_image_IDs(image_handle,
    &depth_id,1);
  if (number_images != 1)
    return;
  error=heif_image_handle_get_depth_image_handle(image_handle,depth_id,
    &depth_handle);
  if (IsHeifSuccess(image,&error,exception) == MagickFalse)
    return;
  AcquireNextImage(image_info,image,exception);
  if (GetNextImageInList(image) != (Image *) NULL)
    {
      image=SyncNextImageInList(image);
      (void) ReadHEICImageHandle(image_info,image,depth_handle,exception);
    }
  heif_image_handle_release(depth_handle);
}

static Image *ReadHEICImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  heif_item_id
    primary_image_id;

  Image
    *image;

  MagickBooleanType
    status;

  size_t
    count,
    length;

  struct heif_context
    *heif_context;

  struct heif_error
    error;

  struct heif_image_handle
    *image_handle;

  void
    *file_data;

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
    return(DestroyImageList(image));
  if (GetBlobSize(image) > (MagickSizeType) MAGICK_SSIZE_MAX)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  length=(size_t) GetBlobSize(image);
  file_data=AcquireMagickMemory(length);
  if (file_data == (void *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (ReadBlob(image,length,file_data) != (ssize_t) length)
    {
      file_data=RelinquishMagickMemory(file_data);
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    }
#if LIBHEIF_NUMERIC_VERSION >= 0x010b0000
  if (heif_has_compatible_brand(file_data,(int) length,"avif") != MagickFalse)
    (void) CopyMagickString(image->magick,"AVIF",MagickPathExtent);
#endif
  /*
    Decode HEIF image.
  */
  heif_context=heif_context_alloc();
  error=heif_context_read_from_memory_without_copy(heif_context,file_data,
    length,NULL);
  if (IsHeifSuccess(image,&error,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      file_data=RelinquishMagickMemory(file_data);
      return(DestroyImageList(image));
    }
  error=heif_context_get_primary_image_ID(heif_context,&primary_image_id);
  if (IsHeifSuccess(image,&error,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      file_data=RelinquishMagickMemory(file_data);
      return(DestroyImageList(image));
    }
  error=heif_context_get_image_handle(heif_context,primary_image_id,
    &image_handle);
  if (IsHeifSuccess(image,&error,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      file_data=RelinquishMagickMemory(file_data);
      return(DestroyImageList(image));
    }
  status=ReadHEICImageHandle(image_info,image,image_handle,exception);
  heif_image_handle_release(image_handle);
  count=(size_t) heif_context_get_number_of_top_level_images(heif_context);
  if ((status != MagickFalse) && (count > 1))
    {
      heif_item_id
        *image_ids;

      size_t
        i;

      image_ids=(heif_item_id *) AcquireQuantumMemory((size_t) count,
        sizeof(*image_ids));
      if (image_ids == (heif_item_id *) NULL)
        {
          heif_context_free(heif_context);
          file_data=RelinquishMagickMemory(file_data);
          return(DestroyImageList(image));
        }
      (void) heif_context_get_list_of_top_level_image_IDs(heif_context,
        image_ids,(int) count);
      for (i=0; i < count; i++)
      {
        if (image_ids[i] == primary_image_id)
          continue;
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
        error=heif_context_get_image_handle(heif_context,image_ids[i],
          &image_handle);
        if (IsHeifSuccess(image,&error,exception) == MagickFalse)
          {
            status=MagickFalse;
            break;
          }
        status=ReadHEICImageHandle(image_info,image,image_handle,exception);
        heif_image_handle_release(image_handle);
        if (status == MagickFalse)
          break;
        if (image_info->number_scenes != 0)
          if (image->scene >= (image_info->scene+image_info->number_scenes-1))
            break;
      }
      image_ids=(heif_item_id *) RelinquishMagickMemory(image_ids);
    }
  error=heif_context_get_image_handle(heif_context,primary_image_id,
    &image_handle);
  if (IsHeifSuccess(image,&error,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      file_data=RelinquishMagickMemory(file_data);
      return(DestroyImageList(image));
    }
  ReadHEICDepthImage(image_info,image,image_handle,exception);
  heif_image_handle_release(image_handle);
  heif_context_free(heif_context);
  file_data=RelinquishMagickMemory(file_data);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s H E I C                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsHEIC() returns MagickTrue if the image format type, identified by the
%  magick string, is Heic.
%
%  The format of the IsHEIC method is:
%
%      MagickBooleanType IsHEIC(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsHEIC(const unsigned char *magick,const size_t length)
{
  if (length < 12)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick+4,"ftyp",4) != 0)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick+8,"avif",4) == 0)
    return(MagickTrue);
  if (LocaleNCompare((const char *) magick+8,"heic",4) == 0)
    return(MagickTrue);
  if (LocaleNCompare((const char *) magick+8,"heix",4) == 0)
    return(MagickTrue);
  if (LocaleNCompare((const char *) magick+8,"mif1",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r H E I C I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterHEICImage() adds attributes for the HEIC image format to the list of
%  supported formats.  The attributes include the image format tag, a method
%  to read and/or write the format, whether the format supports the saving of
%  more than one frame to the same file or blob, whether the format supports
%  native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterHEICImage method is:
%
%      size_t RegisterHEICImage(void)
%
*/
ModuleExport size_t RegisterHEICImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("HEIC","HEIC","High Efficiency Image Format");
#if defined(MAGICKCORE_HEIC_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadHEICImage;
#if LIBHEIF_NUMERIC_VERSION >= 0x01030000
  if (heif_have_encoder_for_format(heif_compression_HEVC))
    entry->encoder=(EncodeImageHandler *) WriteHEICImage;
#else
  entry->encoder=(EncodeImageHandler *) WriteHEICImage;
#endif
#endif
  entry->magick=(IsImageFormatHandler *) IsHEIC;
  entry->mime_type=ConstantString("image/heic");
#if defined(LIBHEIF_VERSION)
  entry->version=ConstantString(LIBHEIF_VERSION);
#endif
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("HEIC","HEIF","High Efficiency Image Format");
#if defined(MAGICKCORE_HEIC_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadHEICImage;
#if LIBHEIF_NUMERIC_VERSION >= 0x01030000
  if (heif_have_encoder_for_format(heif_compression_HEVC))
    entry->encoder=(EncodeImageHandler *) WriteHEICImage;
#else
  entry->encoder=(EncodeImageHandler *) WriteHEICImage;
#endif
#endif
  entry->magick=(IsImageFormatHandler *) IsHEIC;
  entry->mime_type=ConstantString("image/heif");
#if defined(LIBHEIF_VERSION)
  entry->version=ConstantString(LIBHEIF_VERSION);
#endif
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
#if LIBHEIF_NUMERIC_VERSION > 0x01060200
  entry=AcquireMagickInfo("HEIC","AVIF","AV1 Image File Format");
#if defined(MAGICKCORE_HEIC_DELEGATE)
  if (heif_have_decoder_for_format(heif_compression_AV1))
    entry->decoder=(DecodeImageHandler *) ReadHEICImage;
  if (heif_have_encoder_for_format(heif_compression_AV1))
    entry->encoder=(EncodeImageHandler *) WriteHEICImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsHEIC;
  entry->mime_type=ConstantString("image/avif");
#if defined(LIBHEIF_VERSION)
  entry->version=ConstantString(LIBHEIF_VERSION);
#endif
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
#endif
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r H E I C I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterHEICImage() removes format registrations made by the HEIC module
%  from the list of supported formats.
%
%  The format of the UnregisterHEICImage method is:
%
%      UnregisterHEICImage(void)
%
*/
ModuleExport void UnregisterHEICImage(void)
{
#if LIBHEIF_NUMERIC_VERSION > 0x01060200
  (void) UnregisterMagickInfo("AVIF");
#endif
  (void) UnregisterMagickInfo("HEIC");
  (void) UnregisterMagickInfo("HEIF");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H E I C I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteHEICImage() writes an HEIF image using the libheif library.
%
%  The format of the WriteHEICImage method is:
%
%      MagickBooleanType WriteHEICImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception:  return any errors or warnings in this structure.
%
*/

#if defined(MAGICKCORE_HEIC_DELEGATE)
#if LIBHEIF_NUMERIC_VERSION >= 0x01030000
static void WriteProfile(struct heif_context *context,Image *image,
  ExceptionInfo *exception)
{
  const char
    *name;

  const StringInfo
    *profile;

  ssize_t
    i;

  size_t
    length;

  struct heif_error
    error;

  struct heif_image_handle
    *image_handle;

  /*
    Get image handle.
  */
  image_handle=(struct heif_image_handle *) NULL;
  error=heif_context_get_primary_image_handle(context,&image_handle);
  if (error.code != 0)
    return;
  /*
    Save image profile as a APP marker.
  */
  ResetImageProfileIterator(image);
  for (name=GetNextImageProfile(image); name != (const char *) NULL; )
  {
    profile=GetImageProfile(image,name);
    length=GetStringInfoLength(profile);
    if (LocaleCompare(name,"EXIF") == 0)
      {
        length=GetStringInfoLength(profile);
        if (length > 65533L)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              CoderWarning,"ExifProfileSizeExceedsLimit","`%s'",
              image->filename);
            length=65533L;
          }
        (void) heif_context_add_exif_metadata(context,image_handle,
          (void*) GetStringInfoDatum(profile),(int) length);
      }
    if (LocaleCompare(name,"XMP") == 0)
      {
        StringInfo
          *xmp_profile;

        xmp_profile=StringToStringInfo(xmp_namespace);
        if (xmp_profile != (StringInfo *) NULL)
          {
            if (profile != (StringInfo *) NULL)
              ConcatenateStringInfo(xmp_profile,profile);
            GetStringInfoDatum(xmp_profile)[XmpNamespaceExtent]='\0';
            for (i=0; i < (ssize_t) GetStringInfoLength(xmp_profile); i+=65533L)
            {
              length=MagickMin(GetStringInfoLength(xmp_profile)-i,65533L);
              error=heif_context_add_XMP_metadata(context,image_handle,
                (void*) (GetStringInfoDatum(xmp_profile)+i),(int) length);
              if (error.code != 0)
                break;
            }
            xmp_profile=DestroyStringInfo(xmp_profile);
          }
      }
    if (image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "%s profile: %.20g bytes",name,(double) GetStringInfoLength(profile));
    name=GetNextImageProfile(image);
  }
  heif_image_handle_release(image_handle);
}
#endif

static struct heif_error heif_write_func(struct heif_context *context,
  const void* data,size_t size,void* userdata)
{
  Image
    *image;

  struct heif_error
    error_ok;

  (void) context;
  image=(Image*) userdata;
  (void) WriteBlob(image,size,(const unsigned char *) data);
  error_ok.code=heif_error_Ok;
  error_ok.subcode=heif_suberror_Unspecified;
  error_ok.message="ok";
  return(error_ok);
}

static MagickBooleanType WriteHEICImageYCbCr(Image *image,
  struct heif_image *heif_image,ExceptionInfo *exception)
{
  int
    bit_depth,
    p_y,
    p_cb,
    p_cr;

  MagickBooleanType
    status;

  ssize_t
    y;

  struct heif_error
    error;

  uint8_t
    *q_y,
    *q_cb,
    *q_cr;

  status=MagickTrue;
  bit_depth=8;
  error=heif_image_add_plane(heif_image,heif_channel_Y,(int) image->columns,
    (int) image->rows,bit_depth);
  status=IsHeifSuccess(image,&error,exception);
  if (status == MagickFalse)
    return(status);
  error=heif_image_add_plane(heif_image,heif_channel_Cb,
    ((int) image->columns+1)/2,((int) image->rows+1)/2,bit_depth);
  status=IsHeifSuccess(image,&error,exception);
  if (status == MagickFalse)
    return(status);
  error=heif_image_add_plane(heif_image,heif_channel_Cr,
    ((int) image->columns+1)/2,((int) image->rows+1)/2,bit_depth);
  status=IsHeifSuccess(image,&error,exception);
  if (status == MagickFalse)
    return(status);
  q_y=heif_image_get_plane(heif_image,heif_channel_Y,&p_y);
  q_cb=heif_image_get_plane(heif_image,heif_channel_Cb,&p_cb);
  q_cr=heif_image_get_plane(heif_image,heif_channel_Cr,&p_cr);
  /*
    Copy image to heif_image
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *p;

    ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        break;
      }
    if ((y & 0x01) != 0)
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        q_y[y*p_y+x]=ScaleQuantumToChar(GetPixelRed(image,p));
        p+=GetPixelChannels(image);
      }
    else
      for (x=0; x < (ssize_t) image->columns; x+=2)
      {
        q_y[y*p_y+x]=ScaleQuantumToChar(GetPixelRed(image,p));
        q_cb[y/2*p_cb+x/2]=ScaleQuantumToChar(GetPixelGreen(image,p));
        q_cr[y/2*p_cr+x/2]=ScaleQuantumToChar(GetPixelBlue(image,p));
        p+=GetPixelChannels(image);
        if ((x+1) < (ssize_t) image->columns)
          {
            q_y[y*p_y+x+1]=ScaleQuantumToChar(GetPixelRed(image,p));
            p+=GetPixelChannels(image);
          }
      }
    if (image->previous == (Image *) NULL)
      {
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
  }
  return(status);
}

static MagickBooleanType WriteHEICImageRGBA(Image *image,
  struct heif_image *heif_image,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  ssize_t
    y;

  const Quantum
    *p;

  int
    stride;

  struct heif_error
    error;

  uint8_t
    *q,
    *plane;

  status=MagickTrue;
  error=heif_image_add_plane(heif_image,heif_channel_interleaved,
    (int) image->columns,(int) image->rows,8);
  status=IsHeifSuccess(image,&error,exception);
  if (status == MagickFalse)
    return status;
  plane=heif_image_get_plane(heif_image,heif_channel_interleaved,&stride);
  /*
    Copy image to heif_image
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        break;
      }
    q=plane+(y*stride);
    for (x=0; x < (ssize_t) image->columns; x++)
      {
        *(q++)=ScaleQuantumToChar(GetPixelRed(image,p));
        *(q++)=ScaleQuantumToChar(GetPixelGreen(image,p));
        *(q++)=ScaleQuantumToChar(GetPixelBlue(image,p));
        if (image->alpha_trait != UndefinedPixelTrait)
          *(q++)=ScaleQuantumToChar(GetPixelAlpha(image,p));

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
  return(status);
}

static MagickBooleanType WriteHEICImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  struct heif_context
    *heif_context;

  struct heif_encoder
    *heif_encoder;

  struct heif_error
    error;

  struct heif_image
    *heif_image;

  struct heif_writer
    writer;

#if LIBHEIF_NUMERIC_VERSION > 0x01060200
  MagickBooleanType
    encode_avif;
#endif

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  heif_context=heif_context_alloc();
  heif_image=(struct heif_image*) NULL;
  heif_encoder=(struct heif_encoder*) NULL;
#if LIBHEIF_NUMERIC_VERSION > 0x01060200
  encode_avif=(LocaleCompare(image_info->magick,"AVIF") == 0) ?
    MagickTrue : MagickFalse;
#endif
  do
  {
#if LIBHEIF_NUMERIC_VERSION >= 0x01040000
    const StringInfo
      *profile;
#endif

    enum heif_colorspace
      colorspace;

    enum heif_chroma
      chroma;

    MagickBooleanType
      lossless;

    colorspace=heif_colorspace_YCbCr;
    lossless=image_info->quality == 100 ? MagickTrue : MagickFalse;
    chroma=lossless ? heif_chroma_444 : heif_chroma_420;
    /*
      Get encoder for the specified format.
    */
#if LIBHEIF_NUMERIC_VERSION > 0x01060200
    if (encode_avif != MagickFalse)
      error=heif_context_get_encoder_for_format(heif_context,
        heif_compression_AV1,&heif_encoder);
    else
#endif
      error=heif_context_get_encoder_for_format(heif_context,
        heif_compression_HEVC,&heif_encoder);
    status=IsHeifSuccess(image,&error,exception);
    if (status == MagickFalse)
      break;
    if (image->alpha_trait == BlendPixelTrait)
      {
        colorspace=heif_colorspace_RGB;
        chroma=heif_chroma_interleaved_RGBA;
        if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
          status=TransformImageColorspace(image,sRGBColorspace,exception);
      }
    else if (IssRGBCompatibleColorspace(image->colorspace) != MagickFalse)
      {
        colorspace=heif_colorspace_RGB;
        chroma=heif_chroma_interleaved_RGB;
      }
    else if (image->colorspace != YCbCrColorspace)
      status=TransformImageColorspace(image,YCbCrColorspace,exception);
    if (status == MagickFalse)
      break;
    /*
      Initialize HEIF encoder context.
    */
    error=heif_image_create((int) image->columns,(int) image->rows,colorspace,
      chroma,&heif_image);
    status=IsHeifSuccess(image,&error,exception);
    if (status == MagickFalse)
      break;
#if LIBHEIF_NUMERIC_VERSION >= 0x01040000
    profile=GetImageProfile(image,"icc");
    if (profile != (StringInfo *) NULL)
      (void) heif_image_set_raw_color_profile(heif_image,"prof",
        GetStringInfoDatum(profile),GetStringInfoLength(profile));
#endif
    if (colorspace == heif_colorspace_YCbCr)
      status=WriteHEICImageYCbCr(image,heif_image,exception);
    else
      status=WriteHEICImageRGBA(image,heif_image,exception);
    if (status == MagickFalse)
      break;
    /*
      Code and actually write the HEIC image
    */
    if (lossless != MagickFalse)
      error=heif_encoder_set_lossless(heif_encoder,1);
    else if (image_info->quality != UndefinedCompressionQuality)
      error=heif_encoder_set_lossy_quality(heif_encoder,(int)
        image_info->quality);
    status=IsHeifSuccess(image,&error,exception);
    if (status == MagickFalse)
      break;
#if LIBHEIF_NUMERIC_VERSION > 0x01060200
    if (encode_avif != MagickFalse)
      {
        const char
          *option;

        option=GetImageOption(image_info,"heic:speed");
        if (option != (char *) NULL)
          {
            error=heif_encoder_set_parameter(heif_encoder,"speed",option);
            status=IsHeifSuccess(image,&error,exception);
            if (status == MagickFalse)
              break;
          }
        option=GetImageOption(image_info,"heic:chroma");
        if (option != (char *) NULL)
          {
            error=heif_encoder_set_parameter(heif_encoder,"chroma",option);
            status=IsHeifSuccess(image,&error,exception);
            if (status == MagickFalse)
              break;
          }
      }
#endif
    error=heif_context_encode_image(heif_context,heif_image,heif_encoder,
      (const struct heif_encoding_options *) NULL,
      (struct heif_image_handle **) NULL);
    status=IsHeifSuccess(image,&error,exception);
    if (status == MagickFalse)
      break;
#if LIBHEIF_NUMERIC_VERSION >= 0x01030000
    if (image->profiles != (void *) NULL)
      WriteProfile(heif_context,image,exception);
#endif
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
    heif_encoder_release(heif_encoder);
    heif_encoder=(struct heif_encoder*) NULL;
    heif_image_release(heif_image);
    heif_image=(struct heif_image*) NULL;
    scene++;
  } while (image_info->adjoin != MagickFalse);
  if (status != MagickFalse)
    {
      writer.writer_api_version=1;
      writer.write=heif_write_func;
#if LIBHEIF_NUMERIC_VERSION >= 0x01030000
      if (image->profiles != (void *) NULL)
        WriteProfile(heif_context,image,exception);
#endif
      error=heif_context_write(heif_context,&writer,image);
      status=IsHeifSuccess(image,&error,exception);
    }
  if (heif_encoder != (struct heif_encoder*) NULL)
    heif_encoder_release(heif_encoder);
  if (heif_image != (struct heif_image*) NULL)
    heif_image_release(heif_image);
  heif_context_free(heif_context);
  (void) CloseBlob(image);
  return(status);
}
#endif

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
%  Copyright @ 2018 ImageMagick Studio LLC, a non-profit organization         %
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
#include "MagickCore/profile-private.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"
#if defined(MAGICKCORE_HEIC_DELEGATE)
#if defined(MAGICKCORE_HAVE_LIBHEIF_HEIF_H)
#include <libheif/heif.h>
#else
#include <heif.h>
#endif

#define HEIC_COMPUTE_NUMERIC_VERSION(major,minor,patch) \
  ((major<<24) | (minor<<16) | (patch<<8) | 0)

#endif

#if defined(MAGICKCORE_HEIC_DELEGATE)
/*
  Forward declarations.
*/
static MagickBooleanType
  WriteHEICImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
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

static inline MagickBooleanType IsHEIFSuccess(Image *image,
  struct heif_error *error,ExceptionInfo *exception)
{
  if (error->code == 0)
    return(MagickTrue);
  (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
    error->message,"(%d.%d) `%s'",error->code,error->subcode,image->filename);
  return(MagickFalse);
}

static MagickBooleanType ReadHEICColorProfile(Image *image,
  struct heif_image_handle *image_handle,ExceptionInfo *exception)
{
  size_t
    length;

  struct heif_error
    error;

  unsigned char
    *color_profile;

  /*
    Read color profile.
  */
  length=heif_image_handle_get_raw_color_profile_size(image_handle);
  if (length == 0)
    return(MagickTrue);
  if ((MagickSizeType) length > GetBlobSize(image))
    ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
      image->filename);
  color_profile=(unsigned char *) AcquireQuantumMemory(1,length);
  if (color_profile == (unsigned char *) NULL)
    return(MagickFalse);
  error=heif_image_handle_get_raw_color_profile(image_handle,color_profile);
  if (IsHEIFSuccess(image,&error,exception) != MagickFalse)
    {
      StringInfo
        *profile;

      profile=BlobToProfileStringInfo("icc",color_profile,length,exception);
      (void) SetImageProfilePrivate(image,profile,exception);
    }
  color_profile=(unsigned char *) RelinquishMagickMemory(color_profile);
  return(MagickTrue);
}

static MagickBooleanType ReadHEICExifProfile(Image *image,
  struct heif_image_handle *image_handle,ExceptionInfo *exception)
{
  heif_item_id
    id;

  int
    count;

  size_t
    length;

  StringInfo
    *exif_profile;

  struct heif_error
    error;

  /*
    Read Exif profile.
  */
  count=heif_image_handle_get_list_of_metadata_block_IDs(image_handle,"Exif",
    &id,1);
  if (count != 1)
    return(MagickTrue);
  length=heif_image_handle_get_metadata_size(image_handle,id);
  if (length <= 8)
    return(MagickTrue);
  if ((MagickSizeType) length > GetBlobSize(image))
    ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
      image->filename);
  exif_profile=AcquireProfileStringInfo("exif",length,exception);
  error=heif_image_handle_get_metadata(image_handle,id,
    GetStringInfoDatum(exif_profile));
  if ((IsHEIFSuccess(image,&error,exception) != MagickFalse) && (length > 4))
    {
      StringInfo
        *snippet = SplitStringInfo(exif_profile,4);

      unsigned char
        *datum;

      unsigned int
        offset = 0;

      /*
        Extract Exif profile.
      */
      datum=GetStringInfoDatum(snippet);
      offset|=(unsigned int) (*(datum++)) << 24;
      offset|=(unsigned int) (*(datum++)) << 16;
      offset|=(unsigned int) (*(datum++)) << 8;
      offset|=(unsigned int) (*(datum++)) << 0;
      snippet=DestroyStringInfo(snippet);
      /*
        Strip any EOI marker if payload starts with a JPEG marker.
      */
      length=GetStringInfoLength(exif_profile);
      datum=GetStringInfoDatum(exif_profile);
      if ((length > 2) &&
          ((memcmp(datum,"\xff\xd8",2) == 0) ||
           (memcmp(datum,"\xff\xe1",2) == 0)) &&
           (memcmp(datum+length-2,"\xff\xd9",2) == 0))
        SetStringInfoLength(exif_profile,length-2);
      /*
        Skip to actual Exif payload.
      */
      if (offset < GetStringInfoLength(exif_profile))
        {
          (void) DestroyStringInfo(SplitStringInfo(exif_profile,offset));
          (void) SetImageProfilePrivate(image,exif_profile,exception);
          exif_profile=(StringInfo *) NULL;
        }
    }
  if (exif_profile != (StringInfo *) NULL)
    exif_profile=DestroyStringInfo(exif_profile);
  return(MagickTrue);
}

static MagickBooleanType ReadHEICXMPProfile(Image *image,
  struct heif_image_handle *image_handle,ExceptionInfo *exception)
{
  heif_item_id
    id;

  int
    count;

  size_t
    length;

  struct heif_error
    error;

  unsigned char
    *xmp_profile;

  /*
    Read XMP profile.
  */
  count=heif_image_handle_get_list_of_metadata_block_IDs(image_handle,"mime",
    &id,1);
  if (count != 1)
    return(MagickTrue);
  length=heif_image_handle_get_metadata_size(image_handle,id);
  if (length <= 8)
    return(MagickTrue);
  if ((MagickSizeType) length > GetBlobSize(image))
    ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
      image->filename);
  xmp_profile=(unsigned char *) AcquireQuantumMemory(1,length);
  if (xmp_profile == (unsigned char *) NULL)
    return(MagickFalse);
  error=heif_image_handle_get_metadata(image_handle,id,xmp_profile);
  if (IsHEIFSuccess(image,&error,exception) != MagickFalse)
    {
      StringInfo
        *profile;

      profile=BlobToProfileStringInfo("xmp",xmp_profile,length,exception);
      (void) SetImageProfilePrivate(image,profile,exception);
    }
  xmp_profile=(unsigned char *) RelinquishMagickMemory(xmp_profile);
  return(MagickTrue);
}

static MagickBooleanType ReadHEICImageHandle(const ImageInfo *image_info,
  Image *image,struct heif_image_handle *image_handle,ExceptionInfo *exception)
{
  const uint8_t
    *p,
    *pixels;

  enum heif_channel
    channel;

  enum heif_chroma
    chroma;

  int
    bits_per_pixel,
    shift,
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
  if (heif_image_handle_has_alpha_channel(image_handle) != 0)
    image->alpha_trait=BlendPixelTrait;
  image->depth=8;
  bits_per_pixel=heif_image_handle_get_luma_bits_per_pixel(image_handle);
  if (bits_per_pixel != -1)
    image->depth=(size_t) bits_per_pixel;
  preserve_orientation=IsStringTrue(GetImageOption(image_info,
    "heic:preserve-orientation"));
  if (preserve_orientation == MagickFalse)
    (void) SetImageProperty(image,"exif:Orientation","1",exception);
  if (ReadHEICColorProfile(image,image_handle,exception) == MagickFalse)
    return(MagickFalse);
  if (ReadHEICExifProfile(image,image_handle,exception) == MagickFalse)
    return(MagickFalse);
  if (ReadHEICXMPProfile(image,image_handle,exception) == MagickFalse)
    return(MagickFalse);
  if (image_info->ping != MagickFalse)
    return(MagickTrue);
  if (HEICSkipImage(image_info,image) != MagickFalse)
    return(MagickTrue);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(MagickFalse);
  decode_options=heif_decoding_options_alloc();
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,16,0)
  {
    const char
      *option;

    option=GetImageOption(image_info,"heic:chroma-upsampling");
    if (option != (char *) NULL)
      {
        if (LocaleCompare(option,"nearest-neighbor") == 0)
          {
            decode_options->color_conversion_options.
              only_use_preferred_chroma_algorithm=1;
            decode_options->color_conversion_options.
              preferred_chroma_upsampling_algorithm=
              heif_chroma_upsampling_nearest_neighbor;
          }
        else if (LocaleCompare(option,"bilinear") == 0)
          {
            decode_options->color_conversion_options.
              only_use_preferred_chroma_algorithm=1;
            decode_options->color_conversion_options.
              preferred_chroma_upsampling_algorithm=
              heif_chroma_upsampling_bilinear;
          }
      }
    }
#endif
  if (preserve_orientation != MagickFalse)
    decode_options->ignore_transformations=1;
  if (image->alpha_trait != UndefinedPixelTrait)
    {
      chroma=heif_chroma_interleaved_RGBA;
      if (image->depth > 8)
        chroma=heif_chroma_interleaved_RRGGBBAA_LE;
    }
  else
    {
      chroma=heif_chroma_interleaved_RGB;
      if (image->depth > 8)
        chroma=heif_chroma_interleaved_RRGGBB_LE;
    }
  error=heif_decode_image(image_handle,&heif_image,heif_colorspace_RGB,chroma,
    decode_options);
  heif_decoding_options_free(decode_options);
  if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
    return(MagickFalse);
  channel=heif_channel_interleaved;
  image->columns=(size_t) heif_image_get_width(heif_image,channel);
  image->rows=(size_t) heif_image_get_height(heif_image,channel);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      heif_image_release(heif_image);
      return(MagickFalse);
    }
  pixels=heif_image_get_plane_readonly(heif_image,channel,&stride);
  if (pixels == (const uint8_t *) NULL)
    {
      heif_image_release(heif_image);
      return(MagickFalse);
    }
  shift=(int) (16-image->depth);
  if (image->depth <= 8)
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      Quantum
        *q;

      ssize_t
        x;

      /*
        Transform 8-bit image.
      */
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      p=pixels+(y*stride);
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        SetPixelRed(image,ScaleCharToQuantum((unsigned char) *(p++)),q);
        SetPixelGreen(image,ScaleCharToQuantum((unsigned char) *(p++)),q);
        SetPixelBlue(image,ScaleCharToQuantum((unsigned char) *(p++)),q);
        if (image->alpha_trait != UndefinedPixelTrait)
          SetPixelAlpha(image,ScaleCharToQuantum((unsigned char) *(p++)),q);
        q+=GetPixelChannels(image);
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        break;
    }
  else
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      Quantum
        *q;

      ssize_t
        x;

      /*
        Transform 10-bit or 12-bit image.
      */
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      p=pixels+(y*stride);
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        unsigned short pixel = (((unsigned short) *(p+1) << 8) |
          (*(p+0))) << shift; p+=2;
        SetPixelRed(image,ScaleShortToQuantum(pixel),q);
        pixel=(((unsigned short) *(p+1) << 8) | (*(p+0))) << shift; p+=2;
        SetPixelGreen(image,ScaleShortToQuantum(pixel),q);
        pixel=(((unsigned short) *(p+1) << 8) | (*(p+0))) << shift; p+=2;
        SetPixelBlue(image,ScaleShortToQuantum(pixel),q);
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            pixel=(((unsigned short) *(p+1) << 8) | (*(p+0))) << shift; p+=2;
            SetPixelAlpha(image,ScaleShortToQuantum(pixel),q);
          }
        q+=GetPixelChannels(image);
      }
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

  /*
    Read HEIF depth image.
  */
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
  if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
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
  enum heif_filetype_result
    filetype_check;

  heif_item_id
    primary_image_id;

  Image
    *image;

  int
    max_size;

  MagickBooleanType
    status;

  ssize_t
    count;

  struct heif_context
    *heif_context;

  struct heif_error
    error;

  struct heif_image_handle
    *image_handle;

  unsigned char
    magic[12];

  /*
    Open image file.
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
    return(DestroyImageList(image));
  if (ReadBlob(image,sizeof(magic),magic) != sizeof(magic))
    ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
  filetype_check=heif_check_filetype(magic,sizeof(magic));
  if (filetype_check == heif_filetype_no)
    ThrowReaderException(CoderError,"ImageTypeNotSupported");
  (void) CloseBlob(image);
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,11,0)
  if (heif_has_compatible_brand(magic,sizeof(magic), "avif") == 1)
    (void) CopyMagickString(image->magick,"AVIF",MagickPathExtent);
#endif
  /*
    Decode HEIF image.
  */
  heif_context=heif_context_alloc();
  if (heif_context == (struct heif_context *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  max_size=(int) MagickMin(MagickMin(GetMagickResourceLimit(WidthResource),
    GetMagickResourceLimit(HeightResource)),INT_MAX);
  if (max_size != INT_MAX)
    heif_context_set_maximum_image_size_limit(heif_context,max_size);
  error=heif_context_read_from_file(heif_context,image->filename,
    (const struct heif_reading_options *) NULL);
  if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      return(DestroyImageList(image));
    }
  error=heif_context_get_primary_image_ID(heif_context,&primary_image_id);
  if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      return(DestroyImageList(image));
    }
  error=heif_context_get_image_handle(heif_context,primary_image_id,
    &image_handle);
  if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      return(DestroyImageList(image));
    }
  status=ReadHEICImageHandle(image_info,image,image_handle,exception);
  heif_image_handle_release(image_handle);
  count=(ssize_t) heif_context_get_number_of_top_level_images(heif_context);
  if ((status != MagickFalse) && (count > 1))
    {
      heif_item_id
        *ids;

      ssize_t
        i;

      ids=(heif_item_id *) AcquireQuantumMemory((size_t) count,sizeof(*ids));
      if (ids == (heif_item_id *) NULL)
        {
          heif_context_free(heif_context);
          return(DestroyImageList(image));
        }
      (void) heif_context_get_list_of_top_level_image_IDs(heif_context,ids,
        (int) count);
      for (i=0; i < count; i++)
      {
        if (ids[i] == primary_image_id)
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
        error=heif_context_get_image_handle(heif_context,ids[i],&image_handle);
        if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
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
      ids=(heif_item_id *) RelinquishMagickMemory(ids);
    }
  error=heif_context_get_image_handle(heif_context,primary_image_id,
    &image_handle);
  if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      return(DestroyImageList(image));
    }
  ReadHEICDepthImage(image_info,image,image_handle,exception);
  heif_image_handle_release(image_handle);
  heif_context_free(heif_context);
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
#if defined(MAGICKCORE_HEIC_DELEGATE)
  enum heif_filetype_result
    type;

  if (length < 12)
    return(MagickFalse);
  type=heif_check_filetype(magick,(int) length);
  if (type == heif_filetype_yes_supported)
    return(MagickTrue);
#endif
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

#if defined(MAGICKCORE_HEIC_DELEGATE)
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,14,0)
  heif_init((struct heif_init_params *) NULL);
#endif
#endif
  entry=AcquireMagickInfo("HEIC","HEIC","High Efficiency Image Format");
#if defined(MAGICKCORE_HEIC_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadHEICImage;
  if (heif_have_encoder_for_format(heif_compression_HEVC))
    entry->encoder=(EncodeImageHandler *) WriteHEICImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsHEIC;
  entry->mime_type=ConstantString("image/heic");
#if defined(LIBHEIF_VERSION)
  entry->version=ConstantString(LIBHEIF_VERSION);
#endif
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("HEIC","HEIF","High Efficiency Image Format");
#if defined(MAGICKCORE_HEIC_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadHEICImage;
  if (heif_have_encoder_for_format(heif_compression_HEVC))
    entry->encoder=(EncodeImageHandler *) WriteHEICImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsHEIC;
  entry->mime_type=ConstantString("image/heif");
#if defined(LIBHEIF_VERSION)
  entry->version=ConstantString(LIBHEIF_VERSION);
#endif
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,7,0)
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
  entry->flags^=CoderBlobSupportFlag;
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
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,7,0)
  (void) UnregisterMagickInfo("AVIF");
#endif
  (void) UnregisterMagickInfo("HEIC");
  (void) UnregisterMagickInfo("HEIF");
#if defined(MAGICKCORE_HEIC_DELEGATE)
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,14,0)
  heif_deinit();
#endif
#endif
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
static void WriteProfile(struct heif_context *context,Image *image,
  ExceptionInfo *exception)
{
  const char
    *name;

  const StringInfo
    *profile;

  size_t
    length;

  ssize_t
    i;

  struct heif_error
    error;

  struct heif_image_handle
    *image_handle;

  /*
    Get image handle.
  */
  image_handle=(struct heif_image_handle *) NULL;
  error=heif_context_get_primary_image_handle(context,&image_handle);
  if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
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
      (void) heif_context_add_exif_metadata(context,image_handle,
        (void*) GetStringInfoDatum(profile),(int) length);
    if (LocaleCompare(name,"XMP") == 0)
      for (i=0; i < (ssize_t) GetStringInfoLength(profile); i+=65533L)
      {
        length=(size_t) MagickMin((ssize_t) GetStringInfoLength(profile)-i,
          65533L);
        error=heif_context_add_XMP_metadata(context,image_handle,
          (void*) (GetStringInfoDatum(profile)+i),(int) length);
        if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
          break;
      }
    if (image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "%s profile: %.20g bytes",name,(double) GetStringInfoLength(profile));
    name=GetNextImageProfile(image);
  }
  heif_image_handle_release(image_handle);
}

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
    p_cb,
    p_cr,
    p_y;

  MagickBooleanType
    status;

  ssize_t
    y;

  struct heif_error
    error;

  uint8_t
    *q_cb,
    *q_cr,
    *q_y;

  /*
    Transform HEIF YCbCr image.
  */
  status=MagickTrue;
  error=heif_image_add_plane(heif_image,heif_channel_Y,(int) image->columns,
    (int) image->rows,8);
  status=IsHEIFSuccess(image,&error,exception);
  if (status == MagickFalse)
    return(status);
  error=heif_image_add_plane(heif_image,heif_channel_Cb,
    ((int) image->columns+1)/2,((int) image->rows+1)/2,8);
  status=IsHEIFSuccess(image,&error,exception);
  if (status == MagickFalse)
    return(status);
  error=heif_image_add_plane(heif_image,heif_channel_Cr,
    ((int) image->columns+1)/2,((int) image->rows+1)/2,8);
  status=IsHEIFSuccess(image,&error,exception);
  if (status == MagickFalse)
    return(status);
  q_y=heif_image_get_plane(heif_image,heif_channel_Y,&p_y);
  q_cb=heif_image_get_plane(heif_image,heif_channel_Cb,&p_cb);
  q_cr=heif_image_get_plane(heif_image,heif_channel_Cr,&p_cr);
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
  const Quantum
    *p;

  enum heif_channel
    channel;

  int
    stride;

  MagickBooleanType
    status;

  ssize_t
    y;

  struct heif_error
    error;

  uint8_t
    *pixels,
    *q;

  /*
    Transform HEIF RGBA image.
  */
  status=MagickTrue;
  channel=heif_channel_interleaved;
  if (GetPixelChannels(image) == 1)
    channel=heif_channel_Y;
  error=heif_image_add_plane(heif_image,channel,(int) image->columns,
    (int) image->rows,8);
  status=IsHEIFSuccess(image,&error,exception);
  if (status == MagickFalse)
    return(status);
  pixels=heif_image_get_plane(heif_image,channel,&stride);
  if (pixels == (uint8_t *) NULL)
    return(MagickFalse);
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
    q=pixels+(y*stride);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      *(q++)=ScaleQuantumToChar(GetPixelRed(image,p));
      if (GetPixelChannels(image) > 1)
        {
          *(q++)=ScaleQuantumToChar(GetPixelGreen(image,p));
          *(q++)=ScaleQuantumToChar(GetPixelBlue(image,p));
          if (image->alpha_trait != UndefinedPixelTrait)
            *(q++)=ScaleQuantumToChar(GetPixelAlpha(image,p));
        }
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

static MagickBooleanType WriteHEICImageRRGGBBAA(Image *image,
  struct heif_image *heif_image,ExceptionInfo *exception)
{
  const Quantum
    *p;

  enum heif_channel
    channel = heif_channel_interleaved;

  int
    depth,
    shift,
    stride;

  MagickBooleanType
    status = MagickTrue;

  ssize_t
    y;

  struct heif_error
    error;

  uint8_t
    *pixels,
    *q;

  /*
    Transform HEIF RGBA image with depth > 8.
  */
  depth=image->depth > 10 ? 12 : 10;
  if (GetPixelChannels(image) == 1)
    channel=heif_channel_Y;
  error=heif_image_add_plane(heif_image,channel,(int) image->columns,
    (int) image->rows,depth);
  if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
    return(MagickFalse);
  status=IsHEIFSuccess(image,&error,exception);
  if (status == MagickFalse)
    return(status);
  pixels=heif_image_get_plane(heif_image,channel,&stride);
  if (pixels == (uint8_t *) NULL)
    return(MagickFalse);
  shift=(int) (16-depth);
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
    q=pixels+(y*stride);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      int pixel=ScaleQuantumToShort(GetPixelRed(image,p)) >> shift;
      *(q++)=(uint8_t) (pixel & 0xff);
      *(q++)=(uint8_t) (pixel >> 8);
      if (GetPixelChannels(image) > 1)
        {
          pixel=ScaleQuantumToShort(GetPixelGreen(image,p)) >> shift;
          *(q++)=(uint8_t) (pixel & 0xff);
          *(q++)=(uint8_t) (pixel >> 8);
          pixel=ScaleQuantumToShort(GetPixelBlue(image,p)) >> shift;
          *(q++)=(uint8_t) (pixel & 0xff);
          *(q++)=(uint8_t) (pixel >> 8);
          if (image->alpha_trait != UndefinedPixelTrait)
            {
              pixel=ScaleQuantumToShort(GetPixelAlpha(image,p)) >> shift;
              *(q++)=(uint8_t) (pixel & 0xff);
              *(q++)=(uint8_t) (pixel >> 8);
            }
        }
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
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,7,0)
    encode_avif,
#endif
    status;

  MagickOffsetType
    scene;

  struct heif_context
    *heif_context;

  struct heif_encoder
    *heif_encoder = (struct heif_encoder*) NULL;

  struct heif_error
    error;

  struct heif_image
    *heif_image = (struct heif_image*) NULL;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  heif_context=heif_context_alloc();
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,7,0)
  encode_avif=(LocaleCompare(image_info->magick,"AVIF") == 0) ? MagickTrue :
    MagickFalse;
#endif
  do
  {
    const char
      *option;

    const StringInfo
      *profile;

    enum heif_chroma
      chroma;

    enum heif_colorspace
      colorspace = heif_colorspace_YCbCr;

    MagickBooleanType
      lossless = image_info->quality >= 100 ? MagickTrue : MagickFalse;

    struct heif_encoding_options
      *options;

    /*
      Get encoder for the specified format.
    */
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,7,0)
    if (encode_avif != MagickFalse)
      error=heif_context_get_encoder_for_format(heif_context,
        heif_compression_AV1,&heif_encoder);
    else
#endif
      error=heif_context_get_encoder_for_format(heif_context,
        heif_compression_HEVC,&heif_encoder);
    if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
      break;
    status=IsHEIFSuccess(image,&error,exception);
    if (status == MagickFalse)
      break;
    chroma=lossless != MagickFalse ? heif_chroma_444 : heif_chroma_420;
    if ((image->alpha_trait & BlendPixelTrait) != 0)
      {
        if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
          status=TransformImageColorspace(image,sRGBColorspace,exception);
        colorspace=heif_colorspace_RGB;
        chroma=heif_chroma_interleaved_RGBA;
        if (image->depth > 8)
          chroma=heif_chroma_interleaved_RRGGBBAA_LE;
      }
    else
      if (IssRGBCompatibleColorspace(image->colorspace) != MagickFalse)
        {
          colorspace=heif_colorspace_RGB;
          chroma=heif_chroma_interleaved_RGB;
          if (image->depth > 8)
            chroma=heif_chroma_interleaved_RRGGBB_LE;
          if (GetPixelChannels(image) == 1)
            {
              colorspace=heif_colorspace_monochrome;
              chroma=heif_chroma_monochrome;
            }
        }
      else
        if (image->colorspace != YCbCrColorspace)
          status=TransformImageColorspace(image,YCbCrColorspace,exception);
    if (status == MagickFalse)
      break;
    /*
      Initialize HEIF encoder context.
    */
    error=heif_image_create((int) image->columns,(int) image->rows,colorspace,
      chroma,&heif_image);
    if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
      break;
    status=IsHEIFSuccess(image,&error,exception);
    if (status == MagickFalse)
      break;
    profile=GetImageProfile(image,"icc");
    if (profile != (StringInfo *) NULL)
      (void) heif_image_set_raw_color_profile(heif_image,"prof",
        GetStringInfoDatum(profile),GetStringInfoLength(profile));
    if (colorspace == heif_colorspace_YCbCr)
      status=WriteHEICImageYCbCr(image,heif_image,exception);
    else
      if (image->depth > 8)
        status=WriteHEICImageRRGGBBAA(image,heif_image,exception);
      else
        status=WriteHEICImageRGBA(image,heif_image,exception);
    if (status == MagickFalse)
      break;
    /*
      Encode HEIC image.
    */
    if (lossless != MagickFalse)
      error=heif_encoder_set_lossless(heif_encoder,1);
    else if (image_info->quality != UndefinedCompressionQuality)
      error=heif_encoder_set_lossy_quality(heif_encoder,(int)
        image_info->quality);
    status=IsHEIFSuccess(image,&error,exception);
    if (status == MagickFalse)
      break;
    option=GetImageOption(image_info,"heic:speed");
    if (option != (char *) NULL)
      {
        error=heif_encoder_set_parameter(heif_encoder,"speed",option);
        status=IsHEIFSuccess(image,&error,exception);
        if (status == MagickFalse)
          break;
      }
    option=GetImageOption(image_info,"heic:chroma");
    if (option != (char *) NULL)
      {
        error=heif_encoder_set_parameter(heif_encoder,"chroma",option);
        status=IsHEIFSuccess(image,&error,exception);
        if (status == MagickFalse)
          break;
      }
    options=heif_encoding_options_alloc();
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,16,0)
    option=GetImageOption(image_info,"heic:chroma-downsampling");
    if (option != (char *) NULL)
      {
        if (LocaleCompare(option,"nearest-neighbor") == 0)
          {
            options->color_conversion_options.
              only_use_preferred_chroma_algorithm=1;
            options->color_conversion_options.
              preferred_chroma_downsampling_algorithm=
              heif_chroma_downsampling_nearest_neighbor;
          }
        else if (LocaleCompare(option,"average") == 0)
          {
            options->color_conversion_options.
              only_use_preferred_chroma_algorithm=1;
            options->color_conversion_options.
              preferred_chroma_downsampling_algorithm=
              heif_chroma_downsampling_average;
          }
        else if (LocaleCompare(option,"sharp-yuv") == 0)
          {
            options->color_conversion_options.
              only_use_preferred_chroma_algorithm=1;
            options->color_conversion_options.
              preferred_chroma_downsampling_algorithm=
              heif_chroma_downsampling_sharp_yuv;
          }
      }
#endif
#if LIBHEIF_NUMERIC_VERSION >= HEIC_COMPUTE_NUMERIC_VERSION(1,14,0)
    if (image->orientation != UndefinedOrientation)
      options->image_orientation=(enum heif_orientation) image->orientation;
#endif
    error=heif_context_encode_image(heif_context,heif_image,heif_encoder,
      options,(struct heif_image_handle **) NULL);
    heif_encoding_options_free(options);
    if (IsHEIFSuccess(image,&error,exception) == MagickFalse)
      break;
    status=IsHEIFSuccess(image,&error,exception);
    if (status == MagickFalse)
      break;
    if (image->profiles != (void *) NULL)
      WriteProfile(heif_context,image,exception);
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
      struct heif_writer
        writer;

      writer.writer_api_version=1;
      writer.write=heif_write_func;
      error=heif_context_write(heif_context,&writer,image);
      status=IsHEIFSuccess(image,&error,exception);
    }
  if (heif_encoder != (struct heif_encoder*) NULL)
    heif_encoder_release(heif_encoder);
  if (heif_image != (struct heif_image*) NULL)
    heif_image_release(heif_image);
  heif_context_free(heif_context);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                              JJJ  X   X  L                                  %
%                               J    X X   L                                  %
%                               J     X    L                                  %
%                            J  J    X X   L                                  %
%                             JJ    X   X  LLLLL                              %
%                                                                             %
%                                                                             %
%                          JPEG XL (ISO/IEC 18181)                            %
%                                                                             %
%                               Dirk Lemstra                                  %
%                               December 2020                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
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
#include "MagickCore/option.h"
#include "MagickCore/profile-private.h"
#include "MagickCore/property.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/module.h"
#if defined(MAGICKCORE_JXL_DELEGATE)
#include <jxl/decode.h>
#include <jxl/encode.h>
#include <jxl/thread_parallel_runner.h>
#include <jxl/version.h>
#endif

/*
  Typedef declarations.
*/
typedef struct MemoryManagerInfo
{
  Image
    *image;

  ExceptionInfo
    *exception;
} MemoryManagerInfo;

#if defined(MAGICKCORE_JXL_DELEGATE)
/*
  Forward declarations.
*/
static MagickBooleanType
  WriteJXLImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s J X L                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsJXL() returns MagickTrue if the image format type, identified by the
%  magick string, is JXL.
%
%  The format of the IsJXL  method is:
%
%      MagickBooleanType IsJXL(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsJXL(const unsigned char *magick,const size_t length)
{
  JxlSignature
    signature = JxlSignatureCheck(magick,length);

  if ((signature == JXL_SIG_NOT_ENOUGH_BYTES) || (signature == JXL_SIG_INVALID))
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d J X L I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadJXLImage() reads a JXL image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadJXLImage method is:
%
%      Image *ReadJXLImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void *JXLAcquireMemory(void *opaque,size_t size)
{
  unsigned char
    *data;

  data=(unsigned char *) AcquireQuantumMemory(size,sizeof(*data));
  if (data == (unsigned char *) NULL)
    {
      MemoryManagerInfo
        *memory_manager_info;

      memory_manager_info=(MemoryManagerInfo *) opaque;
      (void) ThrowMagickException(memory_manager_info->exception,
        GetMagickModule(),CoderError,"MemoryAllocationFailed","`%s'",
        memory_manager_info->image->filename);
    }
  return(data);
}

static inline StorageType JXLDataTypeToStorageType(Image *image,
  const JxlDataType data_type,ExceptionInfo *exception)
{
  switch (data_type)
  {
    case JXL_TYPE_FLOAT:
      return(FloatPixel);
    case JXL_TYPE_FLOAT16:
      (void) SetImageProperty(image,"quantum:format","floating-point",
        exception);
      return(FloatPixel);
    case JXL_TYPE_UINT16:
      return(ShortPixel);
    case JXL_TYPE_UINT8:
      return(CharPixel);
    default:
      return(UndefinedPixel);
  }
}

static inline OrientationType JXLOrientationToOrientation(
  const JxlOrientation orientation)
{
  switch (orientation)
  {
    default:
    case JXL_ORIENT_IDENTITY:
      return(TopLeftOrientation);
    case JXL_ORIENT_FLIP_HORIZONTAL:
      return(TopRightOrientation);
    case JXL_ORIENT_ROTATE_180:
      return(BottomRightOrientation);
    case JXL_ORIENT_FLIP_VERTICAL:
      return(BottomLeftOrientation);
    case JXL_ORIENT_TRANSPOSE:
      return(LeftTopOrientation);
    case JXL_ORIENT_ROTATE_90_CW:
      return(RightTopOrientation);
    case JXL_ORIENT_ANTI_TRANSPOSE:
      return(RightBottomOrientation);
    case JXL_ORIENT_ROTATE_90_CCW:
      return(LeftBottomOrientation);
  }
}

static void JXLRelinquishMemory(void *magick_unused(opaque),void *address)
{
  magick_unreferenced(opaque);
  (void) RelinquishMagickMemory(address);
}

static inline void JXLSetMemoryManager(JxlMemoryManager *memory_manager,
  MemoryManagerInfo *memory_manager_info,Image *image,ExceptionInfo *exception)
{
  memory_manager_info->image=image;
  memory_manager_info->exception=exception;
  memory_manager->opaque=memory_manager_info;
  memory_manager->alloc=JXLAcquireMemory;
  memory_manager->free=JXLRelinquishMemory;
}

static inline void JXLSetFormat(Image *image,JxlPixelFormat *pixel_format,
  ExceptionInfo *exception)
{
  const char
    *property;

  pixel_format->num_channels=((image->alpha_trait & BlendPixelTrait) != 0) ?
    4U : 3U;
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    pixel_format->num_channels=((image->alpha_trait & BlendPixelTrait) != 0) ?
      2U : 1U;
  pixel_format->data_type=(image->depth > 16) ? JXL_TYPE_FLOAT :
    (image->depth > 8) ? JXL_TYPE_UINT16 : JXL_TYPE_UINT8;
  property=GetImageProperty(image,"quantum:format",exception);
  if (property != (char *) NULL)
    {
      QuantumFormatType format = (QuantumFormatType) ParseCommandOption(
        MagickQuantumFormatOptions,MagickFalse,property);
      if ((format == FloatingPointQuantumFormat) &&
          (pixel_format->data_type == JXL_TYPE_UINT16))
        {
          pixel_format->data_type=JXL_TYPE_FLOAT16;
          (void) SetImageProperty(image,"quantum:format","floating-point",
            exception);
        }
    }
}

static inline void JXLInitImage(Image *image,JxlBasicInfo *basic_info)
{
  image->columns=basic_info->xsize;
  image->rows=basic_info->ysize;
  image->depth=basic_info->bits_per_sample;
  if (basic_info->alpha_bits != 0)
    image->alpha_trait=BlendPixelTrait;
  image->orientation=JXLOrientationToOrientation(basic_info->orientation);
  if (basic_info->have_animation == JXL_TRUE)
    {
      if ((basic_info->animation.tps_numerator > 0) &&
          (basic_info->animation.tps_denominator > 0))
      image->ticks_per_second=basic_info->animation.tps_numerator /
        basic_info->animation.tps_denominator;
      image->iterations=basic_info->animation.num_loops;
    }
}

static inline MagickBooleanType JXLPatchExifProfile(StringInfo *exif_profile)
{
  size_t
    exif_length;

  StringInfo
    *snippet;

  unsigned char
    *exif_datum;

  unsigned int
    offset=0;

  if (GetStringInfoLength(exif_profile) < 4)
    return(MagickFalse);

  /*
    Extract and cache Exif profile.
  */
  snippet=SplitStringInfo(exif_profile,4);
  offset|=(unsigned int) (*(GetStringInfoDatum(snippet)+0)) << 24;
  offset|=(unsigned int) (*(GetStringInfoDatum(snippet)+1)) << 16;
  offset|=(unsigned int) (*(GetStringInfoDatum(snippet)+2)) << 8;
  offset|=(unsigned int) (*(GetStringInfoDatum(snippet)+3)) << 0;
  snippet=DestroyStringInfo(snippet);
  /*
    Strip any EOI marker if payload starts with a JPEG marker.
  */
  exif_length=GetStringInfoLength(exif_profile);
  exif_datum=GetStringInfoDatum(exif_profile);
  if ((exif_length > 2) && 
      ((memcmp(exif_datum,"\xff\xd8",2) == 0) ||
        (memcmp(exif_datum,"\xff\xe1",2) == 0)) &&
      (memcmp(exif_datum+exif_length-2,"\xff\xd9",2) == 0))
    SetStringInfoLength(exif_profile,exif_length-2);
  /*
    Skip to actual Exif payload.
  */
  if (offset < GetStringInfoLength(exif_profile))
    (void) DestroyStringInfo(SplitStringInfo(exif_profile,offset));
  return(MagickTrue);
}

static inline void JXLAddProfilesToImage(Image *image,
  StringInfo **exif_profile,StringInfo **xmp_profile,ExceptionInfo *exception)
{
  if (*exif_profile != (StringInfo *) NULL)
    {
      if (JXLPatchExifProfile(*exif_profile) != MagickFalse)
        {
          (void) SetImageProfilePrivate(image,*exif_profile,exception);
          *exif_profile=(StringInfo *) NULL;
        }
      else
        *exif_profile=DestroyStringInfo(*exif_profile);
    }
  if (*xmp_profile != (StringInfo *) NULL)
    {
      (void) SetImageProfilePrivate(image,*xmp_profile,exception);
      *xmp_profile=(StringInfo *) NULL;
    }
}

static Image *ReadJXLImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  JxlBasicInfo
    basic_info;

  JxlDecoder
    *jxl_info;

  JxlDecoderStatus
    events_wanted,
    jxl_status;

  JxlMemoryManager
    memory_manager;

  JxlPixelFormat
    pixel_format;

  MagickBooleanType
    status;

  MemoryManagerInfo
    memory_manager_info;

  size_t
    extent = 0,
    image_count = 0,
    input_size;

  StringInfo
    *exif_profile = (StringInfo *) NULL,
    *xmp_profile = (StringInfo *) NULL;

  unsigned char
    *pixels,
    *output_buffer = (unsigned char *) NULL;

  void
    *runner = NULL;

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
  image=AcquireImage(image_info, exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Initialize JXL delegate library.
  */
  (void) memset(&basic_info,0,sizeof(basic_info));
  (void) memset(&pixel_format,0,sizeof(pixel_format));
  JXLSetMemoryManager(&memory_manager,&memory_manager_info,image,exception);
  jxl_info=JxlDecoderCreate(&memory_manager);
  if (jxl_info == (JxlDecoder *) NULL)
    ThrowReaderException(CoderError,"MemoryAllocationFailed");
  (void) JxlDecoderSetKeepOrientation(jxl_info,JXL_TRUE);
  (void) JxlDecoderSetUnpremultiplyAlpha(jxl_info,JXL_TRUE);
  events_wanted=(JxlDecoderStatus) (JXL_DEC_BASIC_INFO | JXL_DEC_BOX |
    JXL_DEC_FRAME);
  if (image_info->ping == MagickFalse)
    {
      events_wanted=(JxlDecoderStatus) (events_wanted | JXL_DEC_FULL_IMAGE |
        JXL_DEC_COLOR_ENCODING);
      runner=JxlThreadParallelRunnerCreate(NULL,(size_t) GetMagickResourceLimit(
        ThreadResource));
      if (runner == (void *) NULL)
        {
          JxlDecoderDestroy(jxl_info);
          ThrowReaderException(CoderError,"MemoryAllocationFailed");
        }
      jxl_status=JxlDecoderSetParallelRunner(jxl_info,JxlThreadParallelRunner,
        runner);
      if (jxl_status != JXL_DEC_SUCCESS)
        {
          JxlThreadParallelRunnerDestroy(runner);
          JxlDecoderDestroy(jxl_info);
          ThrowReaderException(CoderError,"MemoryAllocationFailed");
        }
    }
  if (JxlDecoderSubscribeEvents(jxl_info,(int) events_wanted) != JXL_DEC_SUCCESS)
    {
      if (runner != NULL)
        JxlThreadParallelRunnerDestroy(runner);
      JxlDecoderDestroy(jxl_info);
      ThrowReaderException(CoderError,"UnableToReadImageData");
    }
  input_size=MagickMaxBufferExtent;
  pixels=(unsigned char *) AcquireQuantumMemory(input_size,sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    {
      if (runner != NULL)
        JxlThreadParallelRunnerDestroy(runner);
      JxlDecoderDestroy(jxl_info);
      ThrowReaderException(CoderError,"MemoryAllocationFailed");
    }
  /*
    Decode JXL byte stream.
  */
  jxl_status=JXL_DEC_NEED_MORE_INPUT;
  while ((jxl_status != JXL_DEC_SUCCESS) && (jxl_status != JXL_DEC_ERROR))
  {
    jxl_status=JxlDecoderProcessInput(jxl_info);
    switch (jxl_status)
    {
      case JXL_DEC_NEED_MORE_INPUT:
      {
        size_t
          remaining;

        ssize_t
          count;

        remaining=JxlDecoderReleaseInput(jxl_info);
        if (remaining > 0)
          (void) memmove(pixels,pixels+input_size-remaining,remaining);
        count=ReadBlob(image,input_size-remaining,pixels+remaining);
        if (count <= 0)
          {
            JxlDecoderCloseInput(jxl_info);
            break;
          }
        jxl_status=JxlDecoderSetInput(jxl_info,(const uint8_t *) pixels,
          (size_t) count);
        if (jxl_status == JXL_DEC_SUCCESS)
          jxl_status=JXL_DEC_NEED_MORE_INPUT;
        break;
      }
      case JXL_DEC_BASIC_INFO:
      {
        jxl_status=JxlDecoderGetBasicInfo(jxl_info,&basic_info);
        if (jxl_status != JXL_DEC_SUCCESS)
          break;
        if ((basic_info.have_animation == JXL_TRUE) &&
            (basic_info.animation.have_timecodes == JXL_TRUE))
          {
            /*
              We currently don't support animations with time codes.
            */
            (void) ThrowMagickException(exception,GetMagickModule(),
              MissingDelegateError,"NoDecodeDelegateForThisImageFormat","`%s'",
              image->filename);
            break;
          }
        JXLInitImage(image,&basic_info);
        jxl_status=JXL_DEC_BASIC_INFO;
        break;
      }
      case JXL_DEC_COLOR_ENCODING:
      {
        JxlColorEncoding
          color_encoding;

        size_t
          profile_size;

        StringInfo
          *profile;

        (void) memset(&color_encoding,0,sizeof(color_encoding));
        JXLSetFormat(image,&pixel_format,exception);
#if JPEGXL_NUMERIC_VERSION >= JPEGXL_COMPUTE_NUMERIC_VERSION(0,9,0)
        jxl_status=JxlDecoderGetColorAsEncodedProfile(jxl_info,
          JXL_COLOR_PROFILE_TARGET_DATA,&color_encoding);
#else
        jxl_status=JxlDecoderGetColorAsEncodedProfile(jxl_info,&pixel_format,
          JXL_COLOR_PROFILE_TARGET_DATA,&color_encoding);
#endif
        if (jxl_status == JXL_DEC_SUCCESS)
          {
            if (color_encoding.transfer_function == JXL_TRANSFER_FUNCTION_LINEAR)
              {
                image->colorspace=RGBColorspace;
                image->gamma=1.0;
              }
            if (color_encoding.color_space == JXL_COLOR_SPACE_GRAY)
              {
                image->colorspace=GRAYColorspace;
                if (color_encoding.transfer_function == JXL_TRANSFER_FUNCTION_LINEAR)
                  {
                    image->colorspace=LinearGRAYColorspace;
                    image->gamma=1.0;
                  }
              }
            if (color_encoding.white_point == JXL_WHITE_POINT_CUSTOM)
              {
                image->chromaticity.white_point.x=
                  color_encoding.white_point_xy[0];
                image->chromaticity.white_point.y=
                  color_encoding.white_point_xy[1];
              }
            if (color_encoding.primaries == JXL_PRIMARIES_CUSTOM)
              {
                image->chromaticity.red_primary.x=
                  color_encoding.primaries_red_xy[0];
                image->chromaticity.red_primary.y=
                  color_encoding.primaries_red_xy[1];
                image->chromaticity.green_primary.x=
                  color_encoding.primaries_green_xy[0];
                image->chromaticity.green_primary.y=
                  color_encoding.primaries_green_xy[1];
                image->chromaticity.blue_primary.x=
                  color_encoding.primaries_blue_xy[0];
                image->chromaticity.blue_primary.y=
                  color_encoding.primaries_blue_xy[1];
              }
            if (color_encoding.transfer_function == JXL_TRANSFER_FUNCTION_GAMMA)
              image->gamma=color_encoding.gamma;
            switch (color_encoding.rendering_intent)
            {
              case JXL_RENDERING_INTENT_PERCEPTUAL:
              {
                image->rendering_intent=PerceptualIntent;
                break;
              }
              case JXL_RENDERING_INTENT_RELATIVE:
              {
                image->rendering_intent=RelativeIntent;
                break;
              }
              case JXL_RENDERING_INTENT_SATURATION: 
              {
                image->rendering_intent=SaturationIntent;
                break;
              }
              case JXL_RENDERING_INTENT_ABSOLUTE: 
              {
                image->rendering_intent=AbsoluteIntent;
                break;
              }
              default:
              {
                image->rendering_intent=UndefinedIntent;
                break;
              }
            }
          }
        else
          if (jxl_status != JXL_DEC_ERROR)
            break;
#if JPEGXL_NUMERIC_VERSION >= JPEGXL_COMPUTE_NUMERIC_VERSION(0,9,0)
        jxl_status=JxlDecoderGetICCProfileSize(jxl_info,
          JXL_COLOR_PROFILE_TARGET_ORIGINAL,&profile_size);
#else
        jxl_status=JxlDecoderGetICCProfileSize(jxl_info,&pixel_format,
          JXL_COLOR_PROFILE_TARGET_ORIGINAL,&profile_size);
#endif
        if (jxl_status != JXL_DEC_SUCCESS)
          break;
        profile=AcquireProfileStringInfo("icc",profile_size,exception);
        if (profile != (StringInfo *) NULL)
          {
  #if JPEGXL_NUMERIC_VERSION >= JPEGXL_COMPUTE_NUMERIC_VERSION(0,9,0)
            jxl_status=JxlDecoderGetColorAsICCProfile(jxl_info,
              JXL_COLOR_PROFILE_TARGET_ORIGINAL,GetStringInfoDatum(profile),
              profile_size);
#else
            jxl_status=JxlDecoderGetColorAsICCProfile(jxl_info,&pixel_format,
              JXL_COLOR_PROFILE_TARGET_ORIGINAL,GetStringInfoDatum(profile),
              profile_size);
#endif
            if (jxl_status == JXL_DEC_SUCCESS)
              (void) SetImageProfilePrivate(image,profile,exception);
            else
              profile=DestroyStringInfo(profile);
          }
        if (jxl_status == JXL_DEC_SUCCESS)
          jxl_status=JXL_DEC_COLOR_ENCODING;
        break;
      }
      case JXL_DEC_FRAME:
      {
        if (image_count++ != 0)
          {
            JXLAddProfilesToImage(image,&exif_profile,&xmp_profile,exception);
            /*
              Allocate next image structure.
            */
            AcquireNextImage(image_info,image,exception);
            if (GetNextImageInList(image) == (Image *) NULL)
              break;
            image=SyncNextImageInList(image);
            JXLInitImage(image,&basic_info);
          }
        break;
      }
      case JXL_DEC_NEED_IMAGE_OUT_BUFFER:
      {
        status=SetImageExtent(image,image->columns,image->rows,exception);
        if (status == MagickFalse)
          {
            jxl_status=JXL_DEC_ERROR;
            break;
          }
        (void) ResetImagePixels(image,exception);
        JXLSetFormat(image,&pixel_format,exception);
        if (extent == 0)
          {
            jxl_status=JxlDecoderImageOutBufferSize(jxl_info,&pixel_format,
              &extent);
            if (jxl_status != JXL_DEC_SUCCESS)
              break;
            output_buffer=(unsigned char *) AcquireQuantumMemory(extent,
              sizeof(*output_buffer));
            if (output_buffer == (unsigned char *) NULL)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  CoderError,"MemoryAllocationFailed","`%s'",image->filename);
                break;
              }
          }
        jxl_status=JxlDecoderSetImageOutBuffer(jxl_info,&pixel_format,
          output_buffer,extent);
        if (jxl_status == JXL_DEC_SUCCESS)
          jxl_status=JXL_DEC_NEED_IMAGE_OUT_BUFFER;
        break;
      }
      case JXL_DEC_FULL_IMAGE:
      {
        const char
          *map = "RGB";

        StorageType
          type;

        if (output_buffer == (unsigned char *) NULL)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              CorruptImageError,"UnableToReadImageData","`%s'",image->filename);
            break;
          }
        type=JXLDataTypeToStorageType(image,pixel_format.data_type,exception);
        if (type == UndefinedPixel)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              CorruptImageError,"Unsupported data type","`%s'",image->filename);
            break;
          }
        if ((image->alpha_trait & BlendPixelTrait) != 0)
          map="RGBA";
        if (IsGrayColorspace(image->colorspace) != MagickFalse)
          {
            map="I";
            if ((image->alpha_trait & BlendPixelTrait) != 0)
              map="IA";
          }
        status=ImportImagePixels(image,0,0,image->columns,image->rows,map,
          type,output_buffer,exception);
        if (status == MagickFalse)
          jxl_status=JXL_DEC_ERROR;
        break;
      }
      case JXL_DEC_BOX:
      {
        JxlBoxType
          type;

        uint64_t
          size;

        (void) JxlDecoderReleaseBoxBuffer(jxl_info);
        jxl_status=JxlDecoderGetBoxType(jxl_info,type,JXL_FALSE);
        if (jxl_status != JXL_DEC_SUCCESS)
          break;
        jxl_status=JxlDecoderGetBoxSizeRaw(jxl_info,&size);
        if ((jxl_status != JXL_DEC_SUCCESS) || (size <= 8))
          break;
        size-=8;
        if (LocaleNCompare(type,"Exif",sizeof(type)) == 0)
          {
            /*
              Read Exif profile.
            */
          if (exif_profile == (StringInfo *) NULL)
            {
              exif_profile=AcquireProfileStringInfo("exif",(size_t) size,
                exception);
              if (exif_profile != (StringInfo *) NULL)
                jxl_status=JxlDecoderSetBoxBuffer(jxl_info,
                  GetStringInfoDatum(exif_profile),(size_t) size);
            }
          }
        if (LocaleNCompare(type,"xml ",sizeof(type)) == 0)
          {
            /*
              Read XMP profile.
            */
            if (xmp_profile == (StringInfo *) NULL)
              {
                xmp_profile=AcquireProfileStringInfo("xmp",(size_t) size,
                  exception);
                if (xmp_profile != (StringInfo *) NULL)
                  jxl_status=JxlDecoderSetBoxBuffer(jxl_info,
                    GetStringInfoDatum(xmp_profile),(size_t) size);
              }
          }
        if (jxl_status == JXL_DEC_SUCCESS)
          jxl_status=JXL_DEC_BOX;
        break;
      }
      case JXL_DEC_SUCCESS:
      case JXL_DEC_ERROR:
        break;
      default:
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"Unsupported status type","`%d'",jxl_status);
        jxl_status=JXL_DEC_ERROR;
        break;
      }
    }
  }
  (void) JxlDecoderReleaseBoxBuffer(jxl_info);
  JXLAddProfilesToImage(image,&exif_profile,&xmp_profile,exception);
  output_buffer=(unsigned char *) RelinquishMagickMemory(output_buffer);
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  if (runner != NULL)
    JxlThreadParallelRunnerDestroy(runner);
  JxlDecoderDestroy(jxl_info);
  if (jxl_status == JXL_DEC_ERROR)
    ThrowReaderException(CorruptImageError,"UnableToReadImageData");
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
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
%   R e g i s t e r J X L I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterJXLImage() adds properties for the JXL image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterJXLImage method is:
%
%      size_t RegisterJXLImage(void)
%
*/
ModuleExport size_t RegisterJXLImage(void)
{
  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

  *version='\0';
#if defined(MAGICKCORE_JXL_DELEGATE)
  (void) FormatLocaleString(version,MagickPathExtent,"libjxl %u.%u.%u",
    (JxlDecoderVersion()/1000000),(JxlDecoderVersion()/1000) % 1000,
    (JxlDecoderVersion() % 1000));
#endif
  entry=AcquireMagickInfo("JXL", "JXL", "JPEG XL (ISO/IEC 18181)");
#if defined(MAGICKCORE_JXL_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJXLImage;
  entry->encoder=(EncodeImageHandler *) WriteJXLImage;
  entry->magick=(IsImageFormatHandler *) IsJXL;
#endif
  entry->mime_type=ConstantString("image/jxl");
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
%   U n r e g i s t e r J X L I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterJXLImage() removes format registrations made by the
%  JXL module from the list of supported formats.
%
%  The format of the UnregisterJXLImage method is:
%
%      UnregisterJXLImage(void)
%
*/
ModuleExport void UnregisterJXLImage(void)
{
  (void) UnregisterMagickInfo("JXL");
}

#if defined(MAGICKCORE_JXL_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  W r i t e J X L I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteJXLImage() writes a JXL image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the WriteJXLImage method is:
%
%      MagickBooleanType WriteJXLImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static JxlEncoderStatus JXLWriteMetadata(const Image *image,
  JxlEncoder *jxl_info, const StringInfo *icc_profile)
{
  JxlColorEncoding
    color_encoding;

  JxlEncoderStatus
    jxl_status;

  if (icc_profile != (StringInfo *) NULL)
    {
      jxl_status=JxlEncoderSetICCProfile(jxl_info,(const uint8_t *)
        GetStringInfoDatum(icc_profile),GetStringInfoLength(icc_profile));
      return(jxl_status);
    }
  (void) memset(&color_encoding,0,sizeof(color_encoding));
  color_encoding.color_space=JXL_COLOR_SPACE_RGB;
  if (IsRGBColorspace(image->colorspace) == MagickFalse)
    JxlColorEncodingSetToSRGB(&color_encoding,
      IsGrayColorspace(image->colorspace) != MagickFalse);
  else
    JxlColorEncodingSetToLinearSRGB(&color_encoding,
      IsGrayColorspace(image->colorspace) != MagickFalse);
  jxl_status=JxlEncoderSetColorEncoding(jxl_info,&color_encoding);
  return(jxl_status);
}

static inline float JXLGetDistance(float quality)
{
  return quality >= 100.0f ? 0.0f
         : quality >= 30
             ? 0.1f + (100 - quality) * 0.09f
             : 53.0f / 3000.0f * quality * quality - 23.0f / 20.0f * quality + 25.0f;
}

static inline MagickBooleanType JXLSameFrameType(const Image *image,
  const Image *next)
{
  if (image->columns != next->columns)
    return(MagickFalse);
  if (image->rows != next->rows)
    return(MagickFalse);
  if (image->depth != next->depth)
    return(MagickFalse);
  if (image->alpha_trait != next->alpha_trait)
    return(MagickFalse);
  if (image->colorspace != next->colorspace)
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType WriteJXLImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  const char
    *option;

  const StringInfo
    *icc_profile = (StringInfo *) NULL,
    *exif_profile = (StringInfo *) NULL,
    *xmp_profile = (StringInfo *) NULL;

  JxlBasicInfo
    basic_info;

  JxlEncoder
    *jxl_info;

  JxlEncoderFrameSettings
    *frame_settings;

  JxlEncoderStatus
    jxl_status;

  JxlFrameHeader
    frame_header;

  JxlMemoryManager
    memory_manager;

  JxlPixelFormat
    pixel_format;

  MagickBooleanType
    status;

  MemoryInfo
    *pixel_info;

  MemoryManagerInfo
    memory_manager_info;

  size_t
    bytes_per_row;

  unsigned char
    *pixels;

  void
    *runner;

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
  if ((IssRGBCompatibleColorspace(image->colorspace) == MagickFalse) &&
      (IsCMYKColorspace(image->colorspace) == MagickFalse))
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  /*
    Initialize JXL delegate library.
  */
  (void) memset(&basic_info,0,sizeof(basic_info));
  (void) memset(&frame_header,0,sizeof(frame_header));
  (void) memset(&pixel_format,0,sizeof(pixel_format));
  JXLSetMemoryManager(&memory_manager,&memory_manager_info,image,exception);
  jxl_info=JxlEncoderCreate(&memory_manager);
  if (jxl_info == (JxlEncoder *) NULL)
    ThrowWriterException(CoderError,"MemoryAllocationFailed");
  runner=JxlThreadParallelRunnerCreate(NULL,(size_t) GetMagickResourceLimit(
    ThreadResource));
  if (runner == (void *) NULL)
    {
      JxlEncoderDestroy(jxl_info);
      ThrowWriterException(CoderError,"MemoryAllocationFailed");
    }
  jxl_status=JxlEncoderSetParallelRunner(jxl_info,JxlThreadParallelRunner,
    runner);
  if (jxl_status != JXL_ENC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(jxl_info);
      return(MagickFalse);
    }
  JXLSetFormat(image,&pixel_format,exception);
  JxlEncoderInitBasicInfo(&basic_info);
  basic_info.xsize=(uint32_t) image->columns;
  basic_info.ysize=(uint32_t) image->rows;
  basic_info.bits_per_sample=8;
  if (pixel_format.data_type == JXL_TYPE_UINT16)
    basic_info.bits_per_sample=16;
  else
    if (pixel_format.data_type == JXL_TYPE_FLOAT)
      {
        basic_info.bits_per_sample=32;
        basic_info.exponent_bits_per_sample=8;
      }
    else
      if (pixel_format.data_type == JXL_TYPE_FLOAT16)
        {
          basic_info.bits_per_sample=16;
          basic_info.exponent_bits_per_sample=8;
        }
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    basic_info.num_color_channels=1;
  if ((image->alpha_trait & BlendPixelTrait) != 0)
    {
      basic_info.alpha_bits=basic_info.bits_per_sample;
      basic_info.alpha_exponent_bits=basic_info.exponent_bits_per_sample;
      basic_info.num_extra_channels=1;
    }
  if (image_info->quality == 100)
    {
      basic_info.uses_original_profile=JXL_TRUE;
      icc_profile=GetImageProfile(image,"icc");
    }
  if ((image_info->adjoin != MagickFalse) &&
      (GetNextImageInList(image) != (Image *) NULL))
    {
      basic_info.have_animation=JXL_TRUE;
      basic_info.animation.num_loops=(uint32_t) image->iterations;
      basic_info.animation.tps_numerator=(uint32_t) image->ticks_per_second;
      basic_info.animation.tps_denominator=1;
      JxlEncoderInitFrameHeader(&frame_header);
      frame_header.duration=1;
    }
  jxl_status=JxlEncoderSetBasicInfo(jxl_info,&basic_info);
  if (jxl_status != JXL_ENC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(jxl_info);
      ThrowWriterException(CoderError,"UnableToWriteImageData");
    }
  frame_settings=JxlEncoderFrameSettingsCreate(jxl_info,
    (JxlEncoderFrameSettings *) NULL);
  if (frame_settings == (JxlEncoderFrameSettings *) NULL)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(jxl_info);
      ThrowWriterException(CoderError,"MemoryAllocationFailed");
    }
  if (image_info->quality == 100)
    {
      (void) JxlEncoderSetFrameDistance(frame_settings,0.f);
      (void) JxlEncoderSetFrameLossless(frame_settings,JXL_TRUE);
    }
  else if (image_info->quality != 0)
    (void) JxlEncoderSetFrameDistance(frame_settings,
      JXLGetDistance((float) image_info->quality));
  option=GetImageOption(image_info,"jxl:effort");
  if (option != (const char *) NULL)
    (void) JxlEncoderFrameSettingsSetOption(frame_settings,
      JXL_ENC_FRAME_SETTING_EFFORT,StringToInteger(option));
  option=GetImageOption(image_info,"jxl:decoding-speed");
  if (option != (const char *) NULL)
    (void) JxlEncoderFrameSettingsSetOption(frame_settings,
      JXL_ENC_FRAME_SETTING_DECODING_SPEED,StringToInteger(option));
  exif_profile=GetImageProfile(image,"exif");
  xmp_profile=GetImageProfile(image,"xmp");
  if ((exif_profile != (StringInfo *) NULL) ||
      (xmp_profile != (StringInfo *) NULL))
    {
      (void) JxlEncoderUseBoxes(jxl_info);
      if ((exif_profile != (StringInfo *) NULL) &&
          (GetStringInfoLength(exif_profile) > 6))
        {
          /*
            Add Exif profile.  Assumes "Exif\0\0" JPEG APP1 prefix.
          */
          StringInfo
            *profile;

          profile=BlobToStringInfo("\0\0\0\6",4);
          if (profile != (StringInfo *) NULL)
            {
              ConcatenateStringInfo(profile,exif_profile);
              (void) JxlEncoderAddBox(jxl_info,"Exif",
                GetStringInfoDatum(profile),GetStringInfoLength(profile),0);
              profile=DestroyStringInfo(profile);
            }
        }
      if (xmp_profile != (StringInfo *) NULL)
        {
          /*
            Add XMP profile.
          */
          (void) JxlEncoderAddBox(jxl_info,"xml ",GetStringInfoDatum(
            xmp_profile),GetStringInfoLength(xmp_profile),0);
        }
      (void) JxlEncoderCloseBoxes(jxl_info);
    }
  jxl_status=JXLWriteMetadata(image,jxl_info,icc_profile);
  if (jxl_status != JXL_ENC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(jxl_info);
      ThrowWriterException(CoderError,"UnableToWriteImageData");
    }
  /*
    Write image as a JXL stream.
  */
  bytes_per_row=image->columns*
    (((image->alpha_trait & BlendPixelTrait) != 0) ? 4 : 3)*
    ((pixel_format.data_type == JXL_TYPE_FLOAT) ? sizeof(float) :
     (pixel_format.data_type == JXL_TYPE_UINT16) ? sizeof(short) :
     sizeof(char));
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    bytes_per_row=image->columns*
      (((image->alpha_trait & BlendPixelTrait) != 0) ? 2 : 1)*
      ((pixel_format.data_type == JXL_TYPE_FLOAT) ? sizeof(float) :
       (pixel_format.data_type == JXL_TYPE_UINT16) ? sizeof(short) :
       sizeof(char));
  pixel_info=AcquireVirtualMemory(bytes_per_row,image->rows*sizeof(*pixels));
  if (pixel_info == (MemoryInfo *) NULL)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(jxl_info);
      ThrowWriterException(CoderError,"MemoryAllocationFailed");
    }
  do
  {
    Image
      *next;

    if (basic_info.have_animation == JXL_TRUE)
      {
        jxl_status=JxlEncoderSetFrameHeader(frame_settings,&frame_header);
        if (jxl_status != JXL_ENC_SUCCESS)
          break;
      }
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    if (IsGrayColorspace(image->colorspace) != MagickFalse)
      status=ExportImagePixels(image,0,0,image->columns,image->rows,
        ((image->alpha_trait & BlendPixelTrait) != 0) ? "IA" : "I",
        JXLDataTypeToStorageType(image,pixel_format.data_type,exception),
        pixels,exception);
    else
      status=ExportImagePixels(image,0,0,image->columns,image->rows,
        ((image->alpha_trait & BlendPixelTrait) != 0) ? "RGBA" : "RGB",
        JXLDataTypeToStorageType(image,pixel_format.data_type,exception),
        pixels,exception);
    if (status == MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
          "MemoryAllocationFailed","`%s'",image->filename);
        status=MagickFalse;
        break;
      }
    if (jxl_status != JXL_ENC_SUCCESS)
      break;
    jxl_status=JxlEncoderAddImageFrame(frame_settings,&pixel_format,pixels,
      bytes_per_row*image->rows);
    if (jxl_status != JXL_ENC_SUCCESS)
      break;
    next=GetNextImageInList(image);
    if (next == (Image*) NULL)
      break;
    if (JXLSameFrameType(image,next) == MagickFalse)
      {
       (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
         "FramesNotSameDimensions","`%s'",image->filename);
       status=MagickFalse;
       break;
      }
    image=SyncNextImageInList(image);
  } while (image_info->adjoin != MagickFalse);
  pixel_info=RelinquishVirtualMemory(pixel_info);
  if (jxl_status == JXL_ENC_SUCCESS)
    {
      unsigned char
        *output_buffer;

      JxlEncoderCloseInput(jxl_info);
      output_buffer=(unsigned char *) AcquireQuantumMemory(
        MagickMaxBufferExtent,sizeof(*output_buffer));
      if (output_buffer == (unsigned char *) NULL)
        {
          JxlThreadParallelRunnerDestroy(runner);
          JxlEncoderDestroy(jxl_info);
          ThrowWriterException(CoderError,"MemoryAllocationFailed");
        }
      jxl_status=JXL_ENC_NEED_MORE_OUTPUT;
      while (jxl_status == JXL_ENC_NEED_MORE_OUTPUT)
      {
        size_t
          extent;

        ssize_t
          count;

        unsigned char
          *p;

        /*
          Encode the pixel stream.
        */
        extent=MagickMaxBufferExtent;
        p=output_buffer;
        jxl_status=JxlEncoderProcessOutput(jxl_info,&p,&extent);
        count=WriteBlob(image,MagickMaxBufferExtent-extent,output_buffer);
        if (count != (ssize_t) (MagickMaxBufferExtent-extent))
          {
            jxl_status=JXL_ENC_ERROR;
            break;
          }
      }
      output_buffer=(unsigned char *) RelinquishMagickMemory(output_buffer);
    }
  JxlThreadParallelRunnerDestroy(runner);
  JxlEncoderDestroy(jxl_info);
  if (jxl_status != JXL_ENC_SUCCESS)
    ThrowWriterException(CoderError,"UnableToWriteImageData");
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}
#endif

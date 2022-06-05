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
%  Copyright @ 2020 ImageMagick Studio LLC, a non-profit organization         %
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
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/module.h"
#if defined(MAGICKCORE_JXL_DELEGATE)
#include <jxl/decode.h>
#include <jxl/encode.h>
#include <jxl/thread_parallel_runner.h>
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

static inline StorageType JXLDataTypeToStorageType(const JxlDataType data_type)
{
  switch (data_type)
  {
    case JXL_TYPE_FLOAT:
      return FloatPixel;
    case JXL_TYPE_UINT16:
      return ShortPixel;
    case JXL_TYPE_UINT8:
      return CharPixel;
    default:
      return UndefinedPixel;
  }
}

static inline OrientationType JXLOrientationToOrientation(
  const JxlOrientation orientation)
{
  switch (orientation)
  {
    default:
    case JXL_ORIENT_IDENTITY:
      return TopLeftOrientation;
    case JXL_ORIENT_FLIP_HORIZONTAL:
      return TopRightOrientation;
    case JXL_ORIENT_ROTATE_180:
      return BottomRightOrientation;
    case JXL_ORIENT_FLIP_VERTICAL:
      return BottomLeftOrientation;
    case JXL_ORIENT_TRANSPOSE:
      return LeftTopOrientation;
    case JXL_ORIENT_ROTATE_90_CW:
      return RightTopOrientation;
    case JXL_ORIENT_ANTI_TRANSPOSE:
      return RightBottomOrientation;
    case JXL_ORIENT_ROTATE_90_CCW:
      return LeftBottomOrientation;
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

static inline void JXLSetFormat(Image *image,JxlPixelFormat *pixel_format)
{
  pixel_format->num_channels=(image->alpha_trait == BlendPixelTrait) ? 4U : 3U;
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    pixel_format->num_channels=(image->alpha_trait == BlendPixelTrait) ?
      2U : 1U;
  pixel_format->data_type=(image->depth > 16) ? JXL_TYPE_FLOAT :
    (image->depth > 8) ? JXL_TYPE_UINT16 : JXL_TYPE_UINT8;
}

static Image *ReadJXLImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

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
    input_size;

  unsigned char
    *pixels,
    *output_buffer;

  void
    *runner;

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
  JXLSetMemoryManager(&memory_manager,&memory_manager_info,image,exception);
  jxl_info=JxlDecoderCreate(&memory_manager);
  if (jxl_info == (JxlDecoder *) NULL)
    ThrowReaderException(CoderError,"MemoryAllocationFailed");
  (void) JxlDecoderSetKeepOrientation(jxl_info,JXL_TRUE);
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
  events_wanted=JXL_DEC_BASIC_INFO;
  if (image_info->ping == MagickFalse)
    events_wanted|=JXL_DEC_FULL_IMAGE | JXL_DEC_COLOR_ENCODING;
  if (JxlDecoderSubscribeEvents(jxl_info,events_wanted) != JXL_DEC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlDecoderDestroy(jxl_info);
      ThrowReaderException(CoderError,"UnableToReadImageData");
    }
  input_size=MagickMaxBufferExtent;
  pixels=AcquireQuantumMemory(input_size,sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlDecoderDestroy(jxl_info);
      ThrowReaderException(CoderError,"MemoryAllocationFailed");
    }
  output_buffer=(unsigned char *) NULL;
  (void) memset(&pixel_format,0,sizeof(pixel_format));
  jxl_status=JXL_DEC_NEED_MORE_INPUT;
  while ((jxl_status != JXL_DEC_SUCCESS) && (jxl_status != JXL_DEC_ERROR))
  {
    jxl_status=JxlDecoderProcessInput(jxl_info);
    switch (jxl_status)
    {
      case JXL_DEC_NEED_MORE_INPUT:
      {
        size_t
          remaining = JxlDecoderReleaseInput(jxl_info);

        ssize_t
          count;

        if (remaining > 0)
          memmove(pixels,pixels+input_size-remaining,remaining);
        count=ReadBlob(image,input_size-remaining,pixels+remaining);
        if (count <= 0)
          {
            jxl_status=JXL_DEC_ERROR;
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
        JxlBasicInfo
          basic_info;

        (void) memset(&basic_info,0,sizeof(basic_info));
        jxl_status=JxlDecoderGetBasicInfo(jxl_info,&basic_info);
        if (jxl_status != JXL_DEC_SUCCESS)
          break;
        if (basic_info.have_animation == 1)
          {
            /*
              We don't currently support animation.
            */
            (void) ThrowMagickException(exception,GetMagickModule(),
              MissingDelegateError,"NoDecodeDelegateForThisImageFormat","`%s'",
              image->filename);
            break;
          }
        image->columns=basic_info.xsize;
        image->rows=basic_info.ysize;
        image->depth=basic_info.bits_per_sample;
        if (basic_info.alpha_bits != 0)
          image->alpha_trait=BlendPixelTrait;
        image->orientation=JXLOrientationToOrientation(basic_info.orientation);
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

        JXLSetFormat(image,&pixel_format);
        (void) memset(&color_encoding,0,sizeof(color_encoding));
        jxl_status=JxlDecoderGetColorAsEncodedProfile(jxl_info,&pixel_format,
          JXL_COLOR_PROFILE_TARGET_DATA,&color_encoding);
        if (jxl_status == JXL_DEC_SUCCESS)
          {
            if (color_encoding.transfer_function == JXL_TRANSFER_FUNCTION_LINEAR)
              image->colorspace=RGBColorspace;
            if (color_encoding.color_space == JXL_COLOR_SPACE_GRAY)
              {
                image->colorspace=GRAYColorspace;
                if (color_encoding.transfer_function == JXL_TRANSFER_FUNCTION_LINEAR)
                  image->colorspace=LinearGRAYColorspace;
              }
          }
        else if (jxl_status != JXL_DEC_ERROR)
          break;
        jxl_status=JxlDecoderGetICCProfileSize(jxl_info,&pixel_format,
          JXL_COLOR_PROFILE_TARGET_ORIGINAL,&profile_size);
        if (jxl_status != JXL_DEC_SUCCESS)
          break;
        profile=AcquireStringInfo(profile_size);
        jxl_status=JxlDecoderGetColorAsICCProfile(jxl_info,&pixel_format,
          JXL_COLOR_PROFILE_TARGET_ORIGINAL,GetStringInfoDatum(profile),
          profile_size);
        (void) SetImageProfile(image,"icc",profile,exception);
        profile=DestroyStringInfo(profile);
        if (jxl_status == JXL_DEC_SUCCESS)
          jxl_status=JXL_DEC_COLOR_ENCODING;
        break;
      }
      case JXL_DEC_NEED_IMAGE_OUT_BUFFER:
      {
        size_t
          extent;

        status=SetImageExtent(image,image->columns,image->rows,exception);
        if (status == MagickFalse)
          break;
        JXLSetFormat(image,&pixel_format);
        jxl_status=JxlDecoderImageOutBufferSize(jxl_info,&pixel_format,&extent);
        if (jxl_status != JXL_DEC_SUCCESS)
          break;
        output_buffer=AcquireQuantumMemory(extent,sizeof(*output_buffer));
        if (output_buffer == (unsigned char *) NULL)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
              "MemoryAllocationFailed","`%s'",image->filename);
            break;
          }
        jxl_status=JxlDecoderSetImageOutBuffer(jxl_info,&pixel_format,
          output_buffer,extent);
        if (jxl_status == JXL_DEC_SUCCESS)
          jxl_status=JXL_DEC_NEED_IMAGE_OUT_BUFFER;
      }
      case JXL_DEC_FULL_IMAGE:
      {
        StorageType
          type;

        if (output_buffer == (unsigned char *) NULL)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              CorruptImageError,"UnableToReadImageData","`%s'",image->filename);
            break;
          }
        type=JXLDataTypeToStorageType(pixel_format.data_type);
        if (type == UndefinedPixel)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              CorruptImageError,"Unsupported data type","`%s'",image->filename);
            break;
          }
        status=ImportImagePixels(image,0,0,image->columns,image->rows,
          image->alpha_trait == BlendPixelTrait ? "RGBA" : "RGB",type,
          output_buffer,exception);
        if (status == MagickFalse)
          jxl_status=JXL_DEC_ERROR;
        break;
      }
      case JXL_DEC_SUCCESS:
      case JXL_DEC_ERROR:
        break;
      default:
        jxl_status=JXL_DEC_ERROR;
        break;
    }
  }
  output_buffer=(unsigned char *) RelinquishMagickMemory(output_buffer);
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  JxlThreadParallelRunnerDestroy(runner);
  JxlDecoderDestroy(jxl_info);
  if (jxl_status == JXL_DEC_ERROR)
    ThrowReaderException(CorruptImageError,"UnableToReadImageData");
  (void) CloseBlob(image);
  return(image);
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
  entry->flags^=CoderAdjoinFlag;
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
  JxlEncoder *jxl_info)
{
  JxlColorEncoding
    color_encoding;

  JxlEncoderStatus
    jxl_status;

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

static MagickBooleanType WriteJXLImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  const char
    *option;

  JxlBasicInfo
    basic_info;

  JxlEncoder
    *jxl_info;

  JxlEncoderOptions
    *jxl_options;

  JxlEncoderStatus
    jxl_status;

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
  (void) memset(&pixel_format,0,sizeof(pixel_format));
  JXLSetFormat(image,&pixel_format);
  (void) memset(&basic_info,0,sizeof(basic_info));
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
  if (image->alpha_trait == BlendPixelTrait)
    basic_info.alpha_bits=basic_info.bits_per_sample;
  if (image->quality == 100)
    basic_info.uses_original_profile=JXL_TRUE;
  jxl_status=JxlEncoderSetBasicInfo(jxl_info,&basic_info);
  if (jxl_status != JXL_ENC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(jxl_info);
      ThrowWriterException(CoderError,"UnableToWriteImageData");
    }
  jxl_options=JxlEncoderOptionsCreate(jxl_info,(JxlEncoderOptions *) NULL);
  if (jxl_options == (JxlEncoderOptions *) NULL)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(jxl_info);
      ThrowWriterException(CoderError,"MemoryAllocationFailed");
    }
  if (image->quality == 100)
    (void) JxlEncoderOptionsSetLossless(jxl_options,JXL_TRUE);
  else
    {
      float
        distance;

      distance=(image_info->quality >= 30) ? 0.1f+(float) (100-MagickMin(100,
        image_info->quality))*0.09f : 6.4f+(float) pow(2.5f,(30.0-
        image_info->quality)/5.0f)/6.25f;
      (void) JxlEncoderOptionsSetDistance(jxl_options,distance);
    }
  option=GetImageOption(image_info,"jxl:effort");
  if (option != (const char *) NULL)
    (void) JxlEncoderOptionsSetEffort(jxl_options,StringToInteger(option));
  option=GetImageOption(image_info,"jxl:decoding-speed");
  if (option != (const char *) NULL)
    (void) JxlEncoderOptionsSetDecodingSpeed(jxl_options,
      StringToInteger(option));
  jxl_status=JXLWriteMetadata(image,jxl_info);
  jxl_status=JXL_ENC_SUCCESS;
  if (jxl_status != JXL_ENC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(jxl_info);
      ThrowWriterException(CoderError,"UnableToWriteImageData");
    }
  bytes_per_row=image->columns*
    ((image->alpha_trait == BlendPixelTrait) ? 4 : 3)*
    ((pixel_format.data_type == JXL_TYPE_FLOAT) ? sizeof(float) :
     (pixel_format.data_type == JXL_TYPE_UINT16) ? sizeof(short) :
     sizeof(char));
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    bytes_per_row=image->columns*
      ((image->alpha_trait == BlendPixelTrait) ? 2 : 1)*
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
  pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    status=ExportImagePixels(image,0,0,image->columns,image->rows,
      image->alpha_trait == BlendPixelTrait ? "IA" : "I",
      JXLDataTypeToStorageType(pixel_format.data_type),pixels,exception);
  else
    status=ExportImagePixels(image,0,0,image->columns,image->rows,
      image->alpha_trait == BlendPixelTrait ? "RGBA" : "RGB",
      JXLDataTypeToStorageType(pixel_format.data_type),pixels,exception);
  if (status == MagickFalse)
    {
      pixel_info=RelinquishVirtualMemory(pixel_info);
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(jxl_info);
      ThrowWriterException(CoderError,"MemoryAllocationFailed");
    }
  jxl_status=JxlEncoderAddImageFrame(jxl_options,&pixel_format,pixels,
    bytes_per_row*image->rows);
  if (jxl_status == JXL_ENC_SUCCESS)
    {
      unsigned char
        *output_buffer;

      JxlEncoderCloseInput(jxl_info);
      output_buffer=AcquireQuantumMemory(MagickMaxBufferExtent,
        sizeof(*output_buffer));
      if (output_buffer == (unsigned char *) NULL)
        {
          pixel_info=RelinquishVirtualMemory(pixel_info);
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
  pixel_info=RelinquishVirtualMemory(pixel_info);
  JxlThreadParallelRunnerDestroy(runner);
  JxlEncoderDestroy(jxl_info);
  if (jxl_status != JXL_ENC_SUCCESS)
    ThrowWriterException(CoderError,"UnableToWriteImageData");
  (void) CloseBlob(image);
  return(status);
}
#endif

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
#include "MagickCore/attribute.h"
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

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteJXLImage(const ImageInfo *,Image *,ExceptionInfo *);

#if defined(MAGICKCORE_JXL_DELEGATE)
static void *JXLAcquireMemory(void *opaque, size_t size)
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

static inline void JXLSetFormat(Image *image,JxlPixelFormat *format)
{
  format->num_channels=(image->alpha_trait == BlendPixelTrait) ? 4 : 3;
  format->data_type=(image->depth > 16) ? JXL_TYPE_FLOAT : (image->depth > 8) ?
   JXL_TYPE_UINT16 : JXL_TYPE_UINT8;
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
static inline OrientationType JXLOrientationToOrientation(
  JxlOrientation orientation)
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

static Image *ReadJXLImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  JxlPixelFormat
    format;

  JxlDecoderStatus
    events_wanted;

  JxlDecoder
    *decoder;

  JxlDecoderStatus
    decoder_status;

  JxlMemoryManager
    memory_manager;

  MagickBooleanType
    status;

  MemoryManagerInfo
    memory_manager_info;

  size_t
    input_size;

  unsigned char
    *input_buffer,
    *output_buffer;

  void
    *runner;

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
  image=AcquireImage(image_info, exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  JXLSetMemoryManager(&memory_manager,&memory_manager_info,image,exception);
  decoder=JxlDecoderCreate(&memory_manager);
  if (decoder == (JxlDecoder *) NULL)
    ThrowReaderException(CoderError,"MemoryAllocationFailed");
  runner=JxlThreadParallelRunnerCreate(NULL,(size_t) GetMagickResourceLimit(
    ThreadResource));
  if (runner == (void *) NULL)
    {
      JxlDecoderDestroy(decoder);
      ThrowReaderException(CoderError,"MemoryAllocationFailed");
    }
  decoder_status=JxlDecoderSetParallelRunner(decoder,JxlThreadParallelRunner,
    runner);
  if (decoder_status != JXL_DEC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlDecoderDestroy(decoder);
      ThrowReaderException(CoderError,"MemoryAllocationFailed");
    }
  events_wanted=JXL_DEC_BASIC_INFO;
  if (image_info->ping == MagickFalse)
    events_wanted|=JXL_DEC_FULL_IMAGE | JXL_DEC_COLOR_ENCODING;
  if (JxlDecoderSubscribeEvents(decoder,events_wanted) != JXL_DEC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlDecoderDestroy(decoder);
      ThrowReaderException(CoderError,"UnableToReadImageData");
    }
  input_size=MagickMaxBufferExtent;
  input_buffer=AcquireQuantumMemory(input_size,sizeof(*input_buffer));
  if (input_buffer == (unsigned char *) NULL)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlDecoderDestroy(decoder);
      ThrowReaderException(CoderError,"MemoryAllocationFailed");
    }
  output_buffer=(unsigned char *) NULL;
  memset(&format,0,sizeof(format));
  decoder_status=JXL_DEC_NEED_MORE_INPUT;
  while ((decoder_status != JXL_DEC_SUCCESS) &&
         (decoder_status != JXL_DEC_ERROR))
  {
    decoder_status=JxlDecoderProcessInput(decoder);
    switch (decoder_status)
    {
      case JXL_DEC_NEED_MORE_INPUT:
      {
        size_t
          remaining;

        ssize_t
          count;

        remaining=JxlDecoderReleaseInput(decoder);
        if (remaining > 0)
          memmove(input_buffer,input_buffer+input_size-remaining,remaining);
        count=ReadBlob(image,input_size-remaining,input_buffer+remaining);
        if (count <= 0)
          {
            decoder_status=JXL_DEC_SUCCESS;
            ThrowMagickException(exception,GetMagickModule(),CoderError,
              "InsufficientImageDataInFile","`%s'",image->filename);
            break;
          }
        decoder_status=JxlDecoderSetInput(decoder,(const uint8_t *) input_buffer,
          (size_t) count);
        if (decoder_status == JXL_DEC_SUCCESS)
          decoder_status=JXL_DEC_NEED_MORE_INPUT;
        break;
      }
      case JXL_DEC_BASIC_INFO:
      {
        JxlBasicInfo
          basic_info;

        decoder_status=JxlDecoderGetBasicInfo(decoder,&basic_info);
        if (decoder_status != JXL_DEC_SUCCESS)
          break;
        /* For now we dont support images with an animation */
        if (basic_info.have_animation == 1)
          {
            ThrowMagickException(exception,GetMagickModule(),
              MissingDelegateError,"NoDecodeDelegateForThisImageFormat",
              "`%s'",image->filename);
            break;
          }
        image->columns=basic_info.xsize;
        image->rows=basic_info.ysize;
        image->depth=basic_info.bits_per_sample;
        if (basic_info.alpha_bits != 0)
          image->alpha_trait=BlendPixelTrait;
        image->orientation=JXLOrientationToOrientation(basic_info.orientation);
        decoder_status=JXL_DEC_BASIC_INFO;
        break;
      }
      case JXL_DEC_COLOR_ENCODING:
      {
        size_t
          profile_size;

        StringInfo
          *profile;

        decoder_status=JxlDecoderGetICCProfileSize(decoder,&format,
          JXL_COLOR_PROFILE_TARGET_ORIGINAL,&profile_size);
        if (decoder_status != JXL_DEC_SUCCESS)
          break;
        profile=AcquireStringInfo(profile_size);
        decoder_status=JxlDecoderGetColorAsICCProfile(decoder,&format,
          JXL_COLOR_PROFILE_TARGET_ORIGINAL,GetStringInfoDatum(profile),
          profile_size);
        (void) SetImageProfile(image,"icc",profile,exception);
        DestroyStringInfo(profile);
        if (decoder_status == JXL_DEC_SUCCESS)
          decoder_status=JXL_DEC_COLOR_ENCODING;
        break;
      }
      case JXL_DEC_NEED_IMAGE_OUT_BUFFER:
      {
        size_t
          output_size;

        JXLSetFormat(image,&format);
        decoder_status=JxlDecoderImageOutBufferSize(decoder,&format,
          &output_size);
        if (decoder_status != JXL_DEC_SUCCESS)
          break;
        status=SetImageExtent(image,image->columns,image->rows,exception);
        if (status == MagickFalse)
          break;
        output_buffer=AcquireQuantumMemory(output_size,sizeof(*output_buffer));
        if (output_buffer == (unsigned char *) NULL)
          {
            ThrowMagickException(exception,GetMagickModule(),CoderError,
              "MemoryAllocationFailed","`%s'",image->filename);
            break;
          }
        decoder_status=JxlDecoderSetImageOutBuffer(decoder,&format,
          output_buffer,output_size);
        if (decoder_status == JXL_DEC_SUCCESS)
          decoder_status=JXL_DEC_NEED_IMAGE_OUT_BUFFER;
      }
      case JXL_DEC_FULL_IMAGE:
      {
        StorageType
          type;

        if (output_buffer == (unsigned char *) NULL)
          {
            ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
              "UnableToReadImageData","`%s'",image->filename);
            break;
          }
        type=JXLDataTypeToStorageType(format.data_type);
        if (type == UndefinedPixel)
          {
            ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
              "Unsupported data type","`%s'",image->filename);
            break;
          }
        status=ImportImagePixels(image,0,0,image->columns,image->rows,
          image->alpha_trait == BlendPixelTrait ? "RGBA" : "RGB",type,
          output_buffer,exception);
        if (status == MagickFalse)
          decoder_status=JXL_DEC_ERROR;
        break;
      }
      case JXL_DEC_SUCCESS:
      case JXL_DEC_ERROR:
        break;
      default:
        decoder_status=JXL_DEC_ERROR;
        break;
    }
  }
  output_buffer=(unsigned char *) RelinquishMagickMemory(output_buffer);
  input_buffer=(unsigned char *) RelinquishMagickMemory(input_buffer);
  JxlThreadParallelRunnerDestroy(runner);
  JxlDecoderDestroy(decoder);
  if (decoder_status == JXL_DEC_ERROR)
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
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("JXL", "JXL", "JPEG XL (ISO/IEC 18181)");
#if defined(MAGICKCORE_JXL_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJXLImage;
  entry->encoder=(EncodeImageHandler *) WriteJXLImage;
#endif
  entry->flags^=CoderAdjoinFlag;
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
  JxlEncoder *encoder)
{
  JxlColorEncoding
    color_encoding;

  JxlEncoderStatus
    encoder_status;

  memset(&color_encoding,0,sizeof(color_encoding));
  JxlColorEncodingSetToSRGB(&color_encoding,
    IsImageGray(image) == MagickTrue ? JXL_TRUE : JXL_FALSE);
  encoder_status=JxlEncoderSetColorEncoding(encoder,&color_encoding);
  return(encoder_status);
}

static MagickBooleanType WriteJXLImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  const char
    *option;

  JxlBasicInfo
    basic_info;

  JxlEncoder
    *encoder;

  JxlEncoderOptions
    *encoder_options;

  JxlEncoderStatus
    encoder_status;

  JxlMemoryManager
    memory_manager;

  JxlPixelFormat
    format;

  MagickBooleanType
    status;

  MemoryManagerInfo
    memory_manager_info;

  size_t
    bytes_per_row;

  unsigned char
    *input_buffer;

  void
    *runner;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  JXLSetMemoryManager(&memory_manager,&memory_manager_info,image,exception);
  encoder=JxlEncoderCreate(&memory_manager);
  if (encoder == (JxlEncoder *) NULL)
    ThrowWriterException(CoderError,"MemoryAllocationFailed");
  runner=JxlThreadParallelRunnerCreate(NULL,(size_t) GetMagickResourceLimit(
    ThreadResource));
  if (runner == (void *) NULL)
    {
      JxlEncoderDestroy(encoder);
      ThrowWriterException(CoderError,"MemoryAllocationFailed");
    }
  encoder_status=JxlEncoderSetParallelRunner(encoder,JxlThreadParallelRunner,
    runner);
  if (encoder_status != JXL_ENC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(encoder);
      return(MagickFalse);
    }
  memset(&format,0,sizeof(format));
  JXLSetFormat(image,&format);
  JxlEncoderInitBasicInfo(&basic_info);
  basic_info.xsize=(uint32_t) image->columns;
  basic_info.ysize=(uint32_t) image->rows;
  basic_info.bits_per_sample=8;
  if (format.data_type == JXL_TYPE_UINT16)
    basic_info.bits_per_sample=16;
  else if (format.data_type == JXL_TYPE_FLOAT)
    {
      basic_info.bits_per_sample=32;
      basic_info.exponent_bits_per_sample=8;
    }
  if (image->alpha_trait == BlendPixelTrait)
    basic_info.alpha_bits=basic_info.bits_per_sample;
  encoder_status=JxlEncoderSetBasicInfo(encoder,&basic_info);
  if (encoder_status != JXL_ENC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(encoder);
      ThrowWriterException(CoderError,"UnableToWriteImageData");
    }
  encoder_options=JxlEncoderOptionsCreate(encoder,(JxlEncoderOptions *) NULL);
  if (encoder_options == (JxlEncoderOptions *) NULL)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(encoder);
      ThrowWriterException(CoderError,"MemoryAllocationFailed");
    }
  if (image->quality == 100)
    JxlEncoderOptionsSetLossless(encoder_options,JXL_TRUE);
  else
    {
      float
        distance;

      distance=(image_info->quality >= 30) ? 0.1f+(float) (100-MagickMin(100,
        image_info->quality))*0.09f : 6.4f+(float) pow(2.5f,(30-
        image_info->quality)/5.0f)/6.25f;
      JxlEncoderOptionsSetDistance(encoder_options,distance);
    }
  option=GetImageOption(image_info,"jxl:effort");
  if (option != (const char *) NULL)
    JxlEncoderOptionsSetEffort(encoder_options,StringToInteger(option));
  encoder_status=JXLWriteMetadata(image,encoder);
  encoder_status=JXL_ENC_SUCCESS;
  if (encoder_status != JXL_ENC_SUCCESS)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(encoder);
      ThrowWriterException(CoderError,"UnableToWriteImageData");
    }
  bytes_per_row=image->columns*
    ((image->alpha_trait == BlendPixelTrait) ? 4 : 3)*
    ((format.data_type == JXL_TYPE_FLOAT) ? sizeof(float) :
     (format.data_type == JXL_TYPE_UINT16) ? sizeof(short) : sizeof(char));
  input_buffer=AcquireQuantumMemory(bytes_per_row,image->rows*
    sizeof(*input_buffer));
  if (input_buffer == (unsigned char *) NULL)
    {
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(encoder);
      ThrowWriterException(CoderError,"MemoryAllocationFailed");
    }
  status=ExportImagePixels(image,0,0,image->columns,image->rows,
    image->alpha_trait == BlendPixelTrait ? "RGBA" : "RGB",
    JXLDataTypeToStorageType(format.data_type),input_buffer,exception);
  if (status == MagickFalse)
    {
      input_buffer=(unsigned char *) RelinquishMagickMemory(input_buffer);
      JxlThreadParallelRunnerDestroy(runner);
      JxlEncoderDestroy(encoder);
      ThrowWriterException(CoderError,"MemoryAllocationFailed");
    }
  encoder_status=JxlEncoderAddImageFrame(encoder_options,&format,input_buffer,
    bytes_per_row*image->rows);
  if (encoder_status == JXL_ENC_SUCCESS)
    {
      unsigned char
        *output_buffer;

      output_buffer=AcquireQuantumMemory(MagickMaxBufferExtent,
        sizeof(*output_buffer));
      if (output_buffer == (unsigned char *) NULL)
        {
          input_buffer=(unsigned char *) RelinquishMagickMemory(input_buffer);
          JxlThreadParallelRunnerDestroy(runner);
          JxlEncoderDestroy(encoder);
          ThrowWriterException(CoderError,"MemoryAllocationFailed");
        }
      encoder_status=JXL_ENC_NEED_MORE_OUTPUT;
      while (encoder_status == JXL_ENC_NEED_MORE_OUTPUT)
      {
        size_t
          count;

        unsigned char
          *p;

        count=MagickMaxBufferExtent;
        p=output_buffer;
        encoder_status=JxlEncoderProcessOutput(encoder,&p,&count);
        (void) WriteBlob(image,MagickMaxBufferExtent-count,output_buffer);
      }
      output_buffer=(unsigned char *) RelinquishMagickMemory(output_buffer);
    }
  input_buffer=(unsigned char *) RelinquishMagickMemory(input_buffer);
  JxlThreadParallelRunnerDestroy(runner);
  JxlEncoderDestroy(encoder);
  if (encoder_status != JXL_ENC_SUCCESS)
    ThrowWriterException(CoderError,"UnableToWriteImageData");
  (void) CloseBlob(image);
  return(status);
}
#endif

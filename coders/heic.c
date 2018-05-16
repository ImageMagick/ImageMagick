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
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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

#if !defined(MAGICKCORE_WINDOWS_SUPPORT)
static MagickBooleanType
  WriteHEICImage(const ImageInfo *,Image *,ExceptionInfo *);
#endif

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
static MagickBooleanType IsHeifSuccess(struct heif_error *error,Image *image,
  ExceptionInfo *exception)
{
  if (error->code == 0)
    return(MagickTrue);

  ThrowBinaryException(CorruptImageError,error->message,image->filename);
}

static Image *ReadHEICImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  heif_item_id
    exif_id;

  Image
    *image;

  int
    count,
    stride_y,
    stride_cb,
    stride_cr;

  MagickBooleanType
    status;

  MagickSizeType
    length;

  ssize_t
    y;

  struct heif_context
    *heif_context;

  struct heif_error
    error;

  struct heif_image
    *heif_image;

  struct heif_image_handle
    *image_handle;

  uint8_t
    *p_y,
    *p_cb,
    *p_cr;

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
  length=GetBlobSize(image);
  file_data=AcquireMagickMemory(length);
  if (file_data == (void *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (ReadBlob(image,length,file_data) != (ssize_t) length)
    {
      file_data=RelinquishMagickMemory(file_data);
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    }
  /*
    Decode HEIF file
  */
  heif_context=heif_context_alloc();
  error=heif_context_read_from_memory(heif_context,file_data,length,NULL);
  file_data=RelinquishMagickMemory(file_data);
  if (IsHeifSuccess(&error,image,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      return(DestroyImageList(image));
    }
  image_handle=(struct heif_image_handle *) NULL;
  error=heif_context_get_primary_image_handle(heif_context,&image_handle);
  if (IsHeifSuccess(&error,image,exception) == MagickFalse)
    {
      heif_context_free(heif_context);
      return(DestroyImageList(image));
    }
  /*
    Read Exif data from HEIC file
  */
  count=heif_image_handle_get_list_of_metadata_block_IDs(image_handle,"Exif",
    &exif_id,1);
  if (count > 0)
    {
      size_t
        exif_size;

      void
        *exif_buffer;

      exif_size=heif_image_handle_get_metadata_size(image_handle,exif_id);
      if (exif_size > GetBlobSize(image))
        {
          heif_image_handle_release(image_handle);
          heif_context_free(heif_context);
          ThrowReaderException(CorruptImageError,
            "InsufficientImageDataInFile");
        }
      exif_buffer=AcquireMagickMemory(exif_size);
      error=heif_image_handle_get_metadata(image_handle,exif_id,exif_buffer);
      if (error.code == 0)
        {
          StringInfo
            *profile;

          profile=BlobToStringInfo(exif_buffer,exif_size);
          SetImageProfile(image,"exif",profile,exception);
          profile=DestroyStringInfo(profile);
      }
      exif_buffer=RelinquishMagickMemory(exif_buffer);
  }
  /*
    Set image size
   */
  image->depth=8;
  image->columns=(size_t) heif_image_handle_get_width(image_handle);
  image->rows=(size_t) heif_image_handle_get_height(image_handle);
  if (image_info->ping != MagickFalse)
    {
      image->colorspace=YCbCrColorspace;
      heif_image_handle_release(image_handle);
      heif_context_free(heif_context);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      heif_image_handle_release(image_handle);
      heif_context_free(heif_context);
      return(DestroyImageList(image));
    }
  /*
    Copy HEIF image into ImageMagick data structures
  */
  (void) SetImageColorspace(image,YCbCrColorspace,exception);
  error=heif_decode_image(image_handle,&heif_image,heif_colorspace_YCbCr,
    heif_chroma_420,NULL);
  if (IsHeifSuccess(&error,image,exception) == MagickFalse)
    {
      heif_image_handle_release(image_handle);
      heif_context_free(heif_context);
      return(DestroyImageList(image));
    }
  p_y=heif_image_get_plane(heif_image,heif_channel_Y,&stride_y);
  p_cb=heif_image_get_plane(heif_image,heif_channel_Cb,&stride_cb);
  p_cr=heif_image_get_plane(heif_image,heif_channel_Cr,&stride_cr);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *q;

    register ssize_t
      x;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (long) image->columns; x++)
    {
      SetPixelRed(image,ScaleCharToQuantum(p_y[y*stride_y + x]),q);
      SetPixelGreen(image,ScaleCharToQuantum(p_cb[(y/2)*stride_cb + x/2]),q);
      SetPixelBlue(image,ScaleCharToQuantum(p_cr[(y/2)*stride_cr + x/2]),q);
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  heif_image_release(heif_image);
  heif_image_handle_release(image_handle);
  heif_context_free(heif_context);
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
  if (LocaleNCompare((const char *) magick+8,"heic",4) == 0)
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
#if !defined(MAGICKCORE_WINDOWS_SUPPORT)
  entry->encoder=(EncodeImageHandler *) WriteHEICImage;
#endif
#endif
  entry->magick=(IsImageFormatHandler *) IsHEIC;
  entry->mime_type=ConstantString("image/x-heic");
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderDecoderThreadSupportFlag;
  (void) RegisterMagickInfo(entry);
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
  (void) UnregisterMagickInfo("HEIC");
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
#if defined(MAGICKCORE_HEIC_DELEGATE) && !defined(MAGICKCORE_WINDOWS_SUPPORT)
static struct heif_error heif_write_func(struct heif_context *ctx,const void* data,
  size_t size,void* userdata)
{
  Image
    *image;

  struct heif_error
    error_ok;

  (void) ctx;
  image=(Image*) userdata;
  (void) WriteBlob(image,size,data);
  error_ok.code=heif_error_Ok;
  error_ok.subcode=heif_suberror_Unspecified;
  error_ok.message="ok";
  return(error_ok);
}

static MagickBooleanType WriteHEICImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  long
    x,
    y;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  struct heif_context
    *heif_context;

  struct heif_encoder
    *heif_encoder;

  struct heif_image
    *heif_image;

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
  do
  {
    const Quantum
      *p;

    int
      stride_y,
      stride_cb,
      stride_cr;

    struct heif_error
      error;

    struct heif_writer
      writer;

    uint8_t
      *p_y,
      *p_cb,
      *p_cr;

    /*
      Transform colorspace to YCbCr.
    */
    if (image->colorspace != YCbCrColorspace)
      status=TransformImageColorspace(image,YCbCrColorspace,exception);
    if (status == MagickFalse)
      break;
    /*
      Initialize HEIF encoder context
    */
    error=heif_image_create((int) image->columns,(int) image->rows,
      heif_colorspace_YCbCr,heif_chroma_420,&heif_image);
    status=IsHeifSuccess(&error,image,exception);
    if (status == MagickFalse)
      break;
    error=heif_image_add_plane(heif_image,heif_channel_Y,(int) image->columns,
      (int) image->rows,8);
    status=IsHeifSuccess(&error,image,exception);
    if (status == MagickFalse)
      break;
    error=heif_image_add_plane(heif_image,heif_channel_Cb,
      ((int) image->columns+1)/2,((int) image->rows+1)/2,8);
    status=IsHeifSuccess(&error,image,exception);
    if (status == MagickFalse)
      break;
    error=heif_image_add_plane(heif_image,heif_channel_Cr,
      ((int) image->columns+1)/2,((int) image->rows+1)/2,8);
    status=IsHeifSuccess(&error,image,exception);
    if (status == MagickFalse)
      break;
    p_y=heif_image_get_plane(heif_image,heif_channel_Y,&stride_y);
    p_cb=heif_image_get_plane(heif_image,heif_channel_Cb,&stride_cb);
    p_cr=heif_image_get_plane(heif_image,heif_channel_Cr,&stride_cr);
    /*
      Copy image to heif_image
    */
    for (y=0; y < (long) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      if ((y & 1)==0)
        {
          for (x=0; x < (long) image->columns; x+=2)
            {
              p_y[y*stride_y+x]=ScaleQuantumToChar(GetPixelRed(image,p));
              p_cb[y/2*stride_cb+x/2]=ScaleQuantumToChar(GetPixelGreen(image,
                p));
              p_cr[y/2*stride_cr+x/2]=ScaleQuantumToChar(GetPixelBlue(image,
                p));
              p+=GetPixelChannels(image);

              if (x+1 < image->columns)
                {
                  p_y[y*stride_y + x+1]=ScaleQuantumToChar(GetPixelRed(image,
                    p));
                  p+=GetPixelChannels(image);
                }
            }
        }
      else
        {
          for (x=0; x < (long) image->columns; x++)
          {
            p_y[y*stride_y + x]=ScaleQuantumToChar(GetPixelRed(image,p));
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
    if (status == MagickFalse)
      break;
    /*
      Code and actually write the HEIC image
    */
    error=heif_context_get_encoder_for_format(heif_context,
      heif_compression_HEVC,&heif_encoder);
    status=IsHeifSuccess(&error,image,exception);
    if (status == MagickFalse)
      break;
    if (image_info->quality != UndefinedCompressionQuality)
      {
        error=heif_encoder_set_lossy_quality(heif_encoder,
          (int) image_info->quality);
        status=IsHeifSuccess(&error,image,exception);
        if (status == MagickFalse)
          break;
      }
    error=heif_context_encode_image(heif_context,heif_image,heif_encoder,
      (const struct heif_encoding_options*) NULL,
      (struct heif_image_handle**) NULL);
    status=IsHeifSuccess(&error,image,exception);
    if (status == MagickFalse)
      break;
    writer.writer_api_version=1;
    writer.write=heif_write_func;
    error=heif_context_write(heif_context,&writer,image);
    status=IsHeifSuccess(&error,image,exception);
    if (status == MagickFalse)
      break;
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

  if (heif_encoder != (struct heif_encoder*) NULL)
    heif_encoder_release(heif_encoder);
  if (heif_image != (struct heif_image*) NULL)
    heif_image_release(heif_image);
  heif_context_free(heif_context);

  (void) CloseBlob(image);
  return(status);
}
#endif

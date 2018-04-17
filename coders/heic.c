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
#include <libde265/de265.h>
#include <libheif/heif.h>
#endif


static MagickBooleanType
  WriteHEICImage(const ImageInfo *,Image *,ExceptionInfo *);


/*
  Typedef declarations.
*/
#if defined(MAGICKCORE_HEIC_DELEGATE)



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
static Image *ReadHEICImage(const ImageInfo *image_info,
                            ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  MagickSizeType
    length;

  ssize_t
    count;


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

  length=GetBlobSize(image);


  uint8_t* filedata = (unsigned char *) AcquireMagickMemory(length);
  if (filedata == NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
                         image->filename);

  count = ReadBlob(image, length, filedata);
  if (count != length) {
    RelinquishMagickMemory((void *)filedata);

    (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
                                "ImproperImageHeader","`%s'", "unable to read data");
    return(MagickFalse);
  }


  /*
    Decode HEIF file
  */

  struct heif_context* heif_context = heif_context_alloc();
  struct heif_error error;

  error = heif_context_read_from_memory(heif_context, filedata,length, NULL);
  if (error.code) {
  }

  RelinquishMagickMemory((void *)filedata);


  struct heif_image_handle* image_handle = NULL;
  error = heif_context_get_primary_image_handle(heif_context, &image_handle);
  if (error.code) {
  }



  /*
    Set image size
   */

  int width  = heif_image_handle_get_width(image_handle);
  int height = heif_image_handle_get_height(image_handle);


  status=SetImageExtent(image,width,height,exception);
  if (status == MagickFalse)
    goto cleanup;

  image->depth = 8;


  struct heif_image* heif_image = NULL;

  if (image_info->ping == MagickFalse)
    {
      /*
        Copy HEIF image into ImageMagick data structures
      */

      error = heif_decode_image(image_handle,
                                &heif_image,
                                heif_colorspace_YCbCr,
                                heif_chroma_420,
                                NULL);

      uint8_t* p_y;
      uint8_t* p_cb;
      uint8_t* p_cr;
      int stride_y, stride_cb, stride_cr;

      p_y  = heif_image_get_plane(heif_image, heif_channel_Y,  &stride_y);
      p_cb = heif_image_get_plane(heif_image, heif_channel_Cb, &stride_cb);
      p_cr = heif_image_get_plane(heif_image, heif_channel_Cr, &stride_cr);


      int x,y;
      Quantum* q;

      for (y=0; y < (long) height; y++)
        {
          q=QueueAuthenticPixels(image,0,y,width,1,exception);
          if (q == (Quantum *) NULL)
            break;

          for (x=0; x < (long) width; x++)
            {
              SetPixelRed(image,ScaleCharToQuantum(p_y[y*stride_y + x]),q);
              SetPixelGreen(image,ScaleCharToQuantum(p_cb[(y/2)*stride_cb + x/2]),q);
              SetPixelBlue(image,ScaleCharToQuantum(p_cr[(y/2)*stride_cr + x/2]),q);
              q+=GetPixelChannels(image);
            }

          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
        }

      SetImageColorspace(image,YCbCrColorspace,exception);
    }

  /*
    Read Exif data from HEIC file
  */

  heif_item_id exif_id;
  int nMetadata = heif_image_handle_get_list_of_metadata_block_IDs(image_handle,
                                                                   "Exif",
                                                                   &exif_id, 1);

  if (nMetadata > 0) {
    size_t exif_size = heif_image_handle_get_metadata_size(image_handle,
                                                           exif_id);

    uint8_t* exif_buffer = (unsigned char *) AcquireMagickMemory(exif_size);

    error = heif_image_handle_get_metadata(image_handle,
                                           exif_id,
                                           exif_buffer);

    StringInfo* profile = BlobToStringInfo(exif_buffer, exif_size);
    SetImageProfile(image, "exif", profile, exception);

    profile = DestroyStringInfo(profile);
    RelinquishMagickMemory(exif_buffer);
  }


 cleanup:

  if (heif_image)
    heif_image_release(heif_image);

  if (image_handle)
    heif_image_handle_release(image_handle);

  if (heif_context)
    heif_context_free(heif_context);

  return image;
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
  entry->encoder=(EncodeImageHandler *) WriteHEICImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsHEIC;
  entry->mime_type=ConstantString("image/x-heic");
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
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



static struct heif_error heif_write_func(struct heif_context* ctx,
                                         const void* data,
                                         size_t size,
                                         void* userdata)
{
  Image* image = (Image*)userdata;
  (void) WriteBlob(image, size, data);

  struct heif_error error_ok;
  error_ok.code = heif_error_Ok;
  error_ok.subcode = heif_suberror_Unspecified;
  error_ok.message = "ok";
  return error_ok;
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
static MagickBooleanType WriteHEICImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  long
    y;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  const Quantum
    *src;

  long
    x;

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


  struct heif_context* heif_context = heif_context_alloc();
  struct heif_image* heif_image = NULL;
  struct heif_encoder* heif_encoder = NULL;

  do
  {
    /* Initialize HEIF encoder context
     */

    struct heif_error error;
    error = heif_image_create(image->columns, image->rows,
                              heif_colorspace_YCbCr,
                              heif_chroma_420,
                              &heif_image);
    if (error.code) {
      goto error_cleanup;
    }

    error = heif_image_add_plane(heif_image,
                                 heif_channel_Y,
                                 image->columns, image->rows, 8);
    if (error.code) {
      goto error_cleanup;
    }

    error = heif_image_add_plane(heif_image,
                                 heif_channel_Cb,
                                 (image->columns+1)/2, (image->rows+1)/2, 8);
    if (error.code) {
      goto error_cleanup;
    }

    error = heif_image_add_plane(heif_image,
                                 heif_channel_Cr,
                                 (image->columns+1)/2, (image->rows+1)/2, 8);
    if (error.code) {
      goto error_cleanup;
    }


    uint8_t* p_y;
    uint8_t* p_cb;
    uint8_t* p_cr;
    int stride_y, stride_cb, stride_cr;

    p_y  = heif_image_get_plane(heif_image, heif_channel_Y,  &stride_y);
    p_cb = heif_image_get_plane(heif_image, heif_channel_Cb, &stride_cb);
    p_cr = heif_image_get_plane(heif_image, heif_channel_Cr, &stride_cr);

    /*
      Transform colorspace to YCbCr.
    */
    if (image->colorspace != YCbCrColorspace) {
      (void) TransformImageColorspace(image,YCbCrColorspace,exception);
    }


    /*
      Copy image to heif_image
    */

    for (y=0; y < (long) image->rows; y++)
    {
      src=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (src == (const Quantum *) NULL)
        break;

      if ((y & 1)==0)
        {
          for (x=0; x < (long) image->columns; x+=2)
            {
              p_y[y*stride_y + x] = ScaleQuantumToChar(GetPixelRed(image,src));
              p_cb[y/2*stride_cb + x/2] = ScaleQuantumToChar(GetPixelGreen(image,src));
              p_cr[y/2*stride_cr + x/2] = ScaleQuantumToChar(GetPixelBlue(image,src));
              src+=GetPixelChannels(image);

              if (x+1 < image->columns) {
                p_y[y*stride_y + x+1] = ScaleQuantumToChar(GetPixelRed(image,src));
                src+=GetPixelChannels(image);
              }
            }
        }
      else
        {
          for (x=0; x < (long) image->columns; x++)
            {
              p_y[y*stride_y + x] = ScaleQuantumToChar(GetPixelRed(image,src));
              src+=GetPixelChannels(image);
            }
        }


      if (image->previous == (Image *) NULL)
        if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
            (QuantumTick(y,image->rows) != MagickFalse))
          {
            status=image->progress_monitor(SaveImageTag,y,image->rows,
              image->client_data);
            if (status == MagickFalse)
              break;
          }
    }


    /*
      Code and actually write the HEIC image
    */

    error = heif_context_get_encoder_for_format(heif_context,
                                                heif_compression_HEVC,
                                                &heif_encoder);
    if (error.code) {
      goto error_cleanup;
    }

    error = heif_context_encode_image(heif_context,
                                      heif_image,
                                      heif_encoder,
                                      NULL,
                                      NULL);
    if (error.code) {
      goto error_cleanup;
    }

    struct heif_writer writer;
    writer.writer_api_version = 1;
    writer.write = heif_write_func;

    error = heif_context_write(heif_context, &writer, image);
    if (error.code) {
      goto error_cleanup;
    }


    heif_image_release(heif_image);
    heif_image = NULL;

    heif_encoder_release(heif_encoder);
    heif_encoder = NULL;


    if (GetNextImageInList(image) == (Image *) NULL)
      break;

    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
    scene++;
  } while (image_info->adjoin != MagickFalse);


  if (heif_context) {
    heif_context_free(heif_context);
  }

  (void) CloseBlob(image);
  return(MagickTrue);


error_cleanup:
  heif_image_release(heif_image);
  heif_encoder_release(heif_encoder);
  return MagickFalse;
}
#endif

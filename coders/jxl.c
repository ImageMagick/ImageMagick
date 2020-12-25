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
%               Read/Write JPEG XL Lossless JPEG1 Recompression               %
%                                                                             %
%                            The JPEG XL Project                              %
%                               September 2019                                %
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
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#if defined(MAGICKCORE_JXL_DELEGATE)
#include <brunsli/decode.h>
#include <brunsli/encode.h>

/*
  Typedef declarations.
*/
typedef struct JXLInfo
{
  unsigned char
    *data;

  size_t
    extent;
} JXLInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteJXLImage(const ImageInfo *,Image *,ExceptionInfo *);
#endif

#if defined(MAGICKCORE_JXL_DELEGATE)
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

static size_t BufferJXLContent(void *data,const uint8_t *buffer,size_t extent)
{
  JXLInfo
    *jxl_info;

  jxl_info=(JXLInfo *) data;
  jxl_info->data=(unsigned char *) ResizeQuantumMemory(jxl_info->data,
    jxl_info->extent+extent,sizeof(*jxl_info->data));
  if (jxl_info->data == (unsigned char *) NULL)
    return(0);
  (void) memcpy(jxl_info->data+jxl_info->extent,buffer,extent);
  jxl_info->extent+=extent;
  return(extent);
}

static Image *ReadJXLImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  JXLInfo
    jxl_info;

  MagickBooleanType
    status;

  MagickSizeType
    extent;

  ssize_t
    count;

  unsigned char
    *buffer;

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
  /*
    Read JXL image.
  */
  extent=(size_t) GetBlobSize(image);
  buffer=(unsigned char *) AcquireQuantumMemory(extent,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    status=MagickFalse;
  else
    {
      count=ReadBlob(image,extent,buffer);
      if (count != (ssize_t) extent)
        status=MagickFalse;
    }
  image=DestroyImage(image);
  /*
    Unpackage JPEG image from JXL.
  */
  jxl_info.data=(unsigned char *) NULL;
  jxl_info.extent=0;
  if (status != MagickFalse)
    {
      status=DecodeBrunsli(extent,buffer,&jxl_info,BufferJXLContent) == 1 ?
        MagickTrue : MagickFalse;
      buffer=(unsigned char *) RelinquishMagickMemory(buffer);
    }
  if (status != MagickFalse)
    {
      ImageInfo
        *write_info;

      /*
        Convert JXL format.
      */
      write_info=AcquireImageInfo();
      SetImageInfoBlob(write_info,jxl_info.data,jxl_info.extent);
      image=BlobToImage(write_info,jxl_info.data,jxl_info.extent,exception);
      if (image != (Image *) NULL)
        {
          (void) CopyMagickString(image->filename,image_info->filename,
            MagickPathExtent);
          (void) CopyMagickString(image->magick,image_info->magick,
            MagickPathExtent);
        }
      write_info=DestroyImageInfo(write_info);
    }
  if (jxl_info.data != (unsigned char *) NULL)
    jxl_info.data=(unsigned char *) RelinquishMagickMemory(jxl_info.data);
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

  entry=AcquireMagickInfo("JXL", "JXL", "JPEG XL Lossless JPEG1 Recompression");
#if defined(MAGICKCORE_JXL_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadJXLImage;
  entry->encoder=(EncodeImageHandler *) WriteJXLImage;
#endif
  entry->note=ConstantString(
    "JPEG1 recompression as specified in https://arxiv.org/pdf/1908.03565.pdf"
    " page 135. Full JPEG XL support is pending.");
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
static MagickBooleanType WriteJXLImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  Image
    *write_image;

  ImageInfo
    *write_info;

  JXLInfo
    jxl_info;

  MagickBooleanType
    status;

  size_t
    extent;

  unsigned char
    *jpeg_blob;

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
  /*
    Write image as a JPEG blob.
  */
  write_info=AcquireImageInfo();
  write_image=CloneImage(image,0,0,MagickTrue,exception);
  (void) CopyMagickString(write_image->magick,"JPG",MaxTextExtent);
  jpeg_blob=ImageToBlob(write_info,write_image,&extent,exception);
  write_image=DestroyImage(write_image);
  write_info=DestroyImageInfo(write_info);
  if (jpeg_blob == (unsigned char *) NULL)
    return(MagickFalse);
  /*
    Repackage JPEG image.
  */
  jxl_info.data=(unsigned char *) NULL;
  jxl_info.extent=0;
  status=EncodeBrunsli(extent,jpeg_blob,&jxl_info,BufferJXLContent) == 1 ?
    MagickTrue : MagickFalse;
  jpeg_blob=(unsigned char *) RelinquishMagickMemory(jpeg_blob);
  if (status != MagickFalse)
    {
      (void) WriteBlob(image,jxl_info.extent,jxl_info.data);
      (void) CloseBlob(image);
    }
  if (jxl_info.data != (unsigned char *) NULL)
    jxl_info.data=(unsigned char *) RelinquishMagickMemory(jxl_info.data);
  return(status);
}
#endif

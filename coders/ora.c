/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                              OOO   RRRR    AAA                              %
%                             O   O  R   R  A   A                             %
%                             O   O  RRRR   AAAAA                             %
%                             O   O  R  R   A   A                             %
%                              OOO   R   R  A   A                             %
%                                                                             %
%                                                                             %
%                       Read OpenRaster (.ora) files                          %
%                                                                             %
%                             OpenRaster spec:                                %
%         https://www.freedesktop.org/wiki/Specifications/OpenRaster/         %
%                                                                             %
%                                Implementer                                  %
%                           Christopher Chianelli                             %
%                                August 2020                                  %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#include "MagickCore/utility.h"

#if defined(MAGICKCORE_ZIP_DELEGATE)
#include <zip.h>
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d O R A I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadORAImage reads an ORA file in the most basic way possible: by
%  reading it as a ZIP File and extracting the mergedimage.png file from it,
%  which is then passed to ReadPNGImage.
%
%  https://www.freedesktop.org/wiki/Specifications/OpenRaster/Draft/FileLayout/
%
%  The format of the ReadORAImage method is:
%
%      Image *ReadORAImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#if defined(MAGICKCORE_PNG_DELEGATE) && defined(MAGICKCORE_ZIP_DELEGATE)
static Image *ReadORAImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    image_data_buffer[8192];

  const char
    *MERGED_IMAGE_PATH = "mergedimage.png";

  FILE
    *file;

  Image
    *image_metadata,
    *out_image;

  ImageInfo
    *read_info;

  int
    unique_file,
    zip_error;

  MagickBooleanType
    status;

  struct stat
    stat_info;

  zip_t
    *zip_archive;

  zip_file_t
    *merged_image_file;

  zip_int64_t
    read_bytes;

  zip_uint64_t
    offset;

  image_metadata=AcquireImage(image_info,exception);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) stat(image_info->filename,&stat_info);
  zip_archive=zip_open(image_info->filename,ZIP_RDONLY,&zip_error);
  if (zip_archive == NULL)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image_info->filename);
      read_info=DestroyImageInfo(read_info);
      image_metadata=DestroyImage(image_metadata);
      return((Image *) NULL);
    }
  merged_image_file=zip_fopen(zip_archive,MERGED_IMAGE_PATH,ZIP_FL_UNCHANGED);
  if (merged_image_file == NULL)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image_info->filename);
      read_info=DestroyImageInfo(read_info);
      image_metadata=DestroyImage(image_metadata);
      zip_discard(zip_archive);
      return((Image *) NULL);
    }
  /* Get a temporary file to write the mergedimage.png of the ZIP to */
  (void) CopyMagickString(read_info->magick,"PNG",MagickPathExtent);
  unique_file=AcquireUniqueFileResource(read_info->unique);
  (void) CopyMagickString(read_info->filename,read_info->unique,
    MagickPathExtent);
  file=(FILE *) NULL;
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        read_info->filename);
      if (unique_file != -1)
        (void) RelinquishUniqueFileResource(read_info->filename);
      read_info=DestroyImageInfo(read_info);
      image_metadata=DestroyImage(image_metadata);
      zip_fclose(merged_image_file);
      zip_discard(zip_archive);
      return((Image *) NULL);
    }
  /* Write the uncompressed mergedimage.png to the temporary file */
  status=MagickTrue;
  offset=0;
  while (status != MagickFalse)
  {
    read_bytes=zip_fread(merged_image_file,image_data_buffer+offset,
      sizeof(image_data_buffer)-offset);
    if (read_bytes == -1)
      status=MagickFalse;
    else if (read_bytes == 0)
      {
        /* Write up to offset of image_data_buffer to temp file */
        if (!fwrite(image_data_buffer,1,(size_t) offset,file))
          status=MagickFalse;
        break;
      }
    else if (read_bytes == (ssize_t) (sizeof(image_data_buffer)-offset))
      {
        /* Write the entirely of image_data_buffer to temp file */
        if (!fwrite(image_data_buffer,1,sizeof(image_data_buffer),file))
          status=MagickFalse;
        else
          offset=0;
      }
    else
      offset+=(zip_uint64_t) read_bytes;
  }
  (void) fclose(file);
  (void) zip_fclose(merged_image_file);
  (void) zip_discard(zip_archive);
  if (status == MagickFalse)
    {
      ThrowFileException(exception,CoderError,"UnableToReadImageData",
          read_info->filename);
      (void) RelinquishUniqueFileResource(read_info->filename);
      read_info=DestroyImageInfo(read_info);
      image_metadata=DestroyImage(image_metadata);
      return((Image *) NULL);
    }
  /* Delegate to ReadImage to read mergedimage.png */
  out_image=ReadImage(read_info,exception);
  (void) RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  /* Update fields of image from fields of png_image */
  if (out_image != NULL)
    {
      (void) CopyMagickString(out_image->filename,image_metadata->filename,
        MagickPathExtent);
      (void) CopyMagickString(out_image->magick_filename,
        image_metadata->magick_filename,MagickPathExtent);
      out_image->timestamp=time(&stat_info.st_mtime);
      (void) CopyMagickString(out_image->magick,image_metadata->magick,
        MagickPathExtent);
      out_image->extent=(MagickSizeType) stat_info.st_size;
    }
  image_metadata=DestroyImage(image_metadata);
  return(out_image);
}
#endif /* MAGICKCORE_ZIP_DELEGATE && MAGICKCORE_PNG_DELEGATE */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r O R A I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterORAImage() adds attributes for the ORA image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterORAImage method is:
%
%      size_t RegisterORAImage(void)
%
*/
ModuleExport size_t RegisterORAImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("ORA","ORA","OpenRaster format");

#if defined(MAGICKCORE_ZIP_DELEGATE) && defined(MAGICKCORE_PNG_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadORAImage;
#endif
  entry->flags^=CoderBlobSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r O R A I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterORAImage() removes format registrations made by the
%  ORA module from the list of supported formats.
%
%  The format of the UnregisterORAImage method is:
%
%      UnregisterORAImage(void)
%
*/
ModuleExport void UnregisterORAImage(void)
{
  (void) UnregisterMagickInfo("ORA");
}

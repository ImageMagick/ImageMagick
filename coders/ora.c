/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         ####### ######     #                                %
%                         #     # #     #   # #                               %
%                         #     # #     #  #   #                              %
%                         #     # ######  #     #                             %
%                         #     # #   #   #######                             %
%                         #     # #    #  #     #                             %
%                         ####### #     # #     #                             %
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
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#include "MagickCore/utility.h"
#if defined(MAGICKCORE_XML_DELEGATE)
#  if defined(MAGICKCORE_WINDOWS_SUPPORT)
#    if !defined(__MINGW32__)
#      include <win32config.h>
#    endif
#  endif
#  include <libxml/parser.h>
#  include <libxml/xmlmemory.h>
#  include <libxml/nanoftp.h>
#  include <libxml/nanohttp.h>
#endif
#if defined(MAGICKCORE_WINDOWS_SUPPORT) && \
    !defined(__MINGW32__)
#  include <urlmon.h>
#  pragma comment(lib, "urlmon.lib")
#endif
#include <zip.h>

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
%  reading it as a ZIP File and extracting the mergedimage.png file
%  from it (see https://www.freedesktop.org/wiki/Specifications/OpenRaster/Draft/FileLayout/)
%  it, which is then passed to ReadPNGImage.
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

static Image *ReadORAImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define MaxBufferExtent  8192

  char
    imageDataBuffer[MaxBufferExtent];

  const char
    *MERGED_IMAGE_PATH = "/mergedimage.png";

   FILE
    *file;

  Image
    *image;

  ImageInfo
    *read_info;

  int
    unique_file;

  image=AcquireImage(image_info,exception);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);

  zip_t
    *zipArchive;

  zip_file_t
    *mergedImageFile;

  int zipError;
  zipArchive = zip_open(image_info->filename, ZIP_RDONLY, &zipError);
  if (zipArchive == NULL) {
    ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
      read_info->filename);
    read_info=DestroyImageInfo(read_info);
    image=DestroyImage(image);
    return((Image *) NULL);
  }

  mergedImageFile = zip_fopen(zipArchive, MERGED_IMAGE_PATH, ZIP_FL_UNCHANGED);
  if (mergedImageFile == NULL) {
    ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
      read_info->filename);
    read_info=DestroyImageInfo(read_info);
    image=DestroyImage(image);
    zip_discard(zipArchive);
    return((Image *) NULL);
  }

  (void) FormatLocaleString(read_info->filename,MagickPathExtent,
            "%s.png",read_info->unique);
  *read_info->magick='\0';
  unique_file = AcquireUniqueFileResource(read_info->filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        read_info->filename);
      read_info=DestroyImageInfo(read_info);
      image=DestroyImage(image);
      zip_fclose(mergedImageFile);
      zip_discard(zipArchive);
      return((Image *) NULL);
    }


  zip_uint64_t readBytes = 0;
  zip_uint64_t offset = 0;
  do
  {
    readBytes = zip_fread(mergedImageFile, imageDataBuffer + offset, MaxBufferExtent - offset);
    if (readBytes == -1) {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
          read_info->filename);
      fclose(file);
      RelinquishUniqueFileResource(read_info->filename);
      read_info=DestroyImageInfo(read_info);
      image=DestroyImage(image);
      zip_fclose(mergedImageFile);
      zip_discard(zipArchive);
      zip_fclose(mergedImageFile);
      return((Image *) NULL);
    }
    if (readBytes == 0) {
        // Write up to offset of imageDataBuffer to temp file
        ssize_t
          success=(ssize_t) fwrite(imageDataBuffer,offset,1,file);
        if (!success) {
          ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
            read_info->filename);
          fclose(file);
          RelinquishUniqueFileResource(read_info->filename);
          read_info=DestroyImageInfo(read_info);
          image=DestroyImage(image);
          zip_fclose(mergedImageFile);
          zip_discard(zipArchive);
          zip_fclose(mergedImageFile);
          return((Image *) NULL);
        }
    }
    else if (readBytes == MaxBufferExtent - offset) {
        // Write the entirely of imageDataBuffer to temp file
        ssize_t
          success=(ssize_t) fwrite(imageDataBuffer,MaxBufferExtent,1,file);
        if (!success) {
          ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
            read_info->filename);
          fclose(file);
          RelinquishUniqueFileResource(read_info->filename);
          read_info=DestroyImageInfo(read_info);
          image=DestroyImage(image);
          zip_fclose(mergedImageFile);
          zip_discard(zipArchive);
          zip_fclose(mergedImageFile);
          return((Image *) NULL);
        }
        offset = 0;
    }
    else {
        offset += readBytes;
    }
  }
  while (readBytes > 0);

  zip_fclose(mergedImageFile);
  zip_discard(zipArchive);
  fclose(file);

  image = ReadImage(read_info, exception);

  RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  return image;
}

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
  entry->decoder=(DecodeImageHandler *) ReadORAImage;
  entry->format_type=ImplicitFormatType;
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

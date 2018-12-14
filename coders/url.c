/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            U   U  RRRR   L                                  %
%                            U   U  R   R  L                                  %
%                            U   U  RRRR   L                                  %
%                            U   U  R R    L                                  %
%                             UUU   R  R   LLLLL                              %
%                                                                             %
%                                                                             %
%                        Retrieve An Image Via URL.                           %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                              Bill Radcliffe                                 %
%                                March 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d U R L I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadURLImage retrieves an image via URL, decodes the image, and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadURLImage method is:
%
%      Image *ReadURLImage(const ImageInfo *image_info,ExceptionInfo *exception)
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

#if defined(MAGICKCORE_XML_DELEGATE) && defined(LIBXML_FTP_ENABLED)
static void GetFTPData(void *userdata,const char *data,int size)
{
  FILE
    *file;

  size_t
    length;

  file=(FILE *) userdata;
  if (file == (FILE *) NULL)
    return;
  if (size <= 0)
    return;
  length=fwrite(data,size,1,file);
  (void) length;
}
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static Image *ReadURLImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define MaxBufferExtent  8192

  char
    filename[MagickPathExtent];

  FILE
    *file;

  Image
    *image;

  ImageInfo
    *read_info;

  int
    unique_file;

  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  if (LocaleCompare(read_info->magick,"file") == 0)
    {
      (void) CopyMagickString(read_info->filename,image_info->filename+2,
        MagickPathExtent);
      *read_info->magick='\0';
      image=ReadImage(read_info,exception);
      read_info=DestroyImageInfo(read_info);
      return(GetFirstImageInList(image));
    }
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(read_info->filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        read_info->filename);
      read_info=DestroyImageInfo(read_info);
      return((Image *) NULL);
    }
  (void) CopyMagickString(filename,image_info->magick,MagickPathExtent);
  (void) ConcatenateMagickString(filename,":",MagickPathExtent);
  LocaleLower(filename);
  (void) ConcatenateMagickString(filename,image_info->filename,
    MagickPathExtent);
#if defined(MAGICKCORE_WINDOWS_SUPPORT) && \
    !defined(__MINGW32__)
  (void) fclose(file);
  if (URLDownloadToFile(NULL,filename,read_info->filename,0,NULL) != S_OK)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        filename);
      (void) RelinquishUniqueFileResource(read_info->filename);
      read_info=DestroyImageInfo(read_info);
      return((Image *) NULL);
    }
#else
#if defined(MAGICKCORE_XML_DELEGATE) && defined(LIBXML_FTP_ENABLED)
  if (LocaleCompare(read_info->magick,"ftp") == 0)
    {
      void
        *context;

      xmlNanoFTPInit();
      context=xmlNanoFTPNewCtxt(filename);
      if (context != (void *) NULL)
        {
          if (xmlNanoFTPConnect(context) >= 0)
            (void) xmlNanoFTPGet(context,GetFTPData,(void *) file,
              (char *) NULL);
          (void) xmlNanoFTPClose(context);
        }
    }
#endif
#if defined(MAGICKCORE_XML_DELEGATE) && defined(LIBXML_HTTP_ENABLED)
  if (LocaleCompare(read_info->magick,"http") == 0)
    {
      char
        buffer[MaxBufferExtent],
        *type;

      int
        bytes;

      void
        *context;

      type=(char *) NULL;
      context=xmlNanoHTTPMethod(filename,(const char *) NULL,
        (const char *) NULL,&type,(const char *) NULL,0);
      if (context != (void *) NULL)
        {
          ssize_t
            count;

          while ((bytes=xmlNanoHTTPRead(context,buffer,MaxBufferExtent)) > 0)
            count=(ssize_t) fwrite(buffer,bytes,1,file);
          (void) count;
          xmlNanoHTTPClose(context);
          xmlFree(type);
          xmlNanoHTTPCleanup();
        }
    }
#endif
  (void) fclose(file);
#endif
  *read_info->magick='\0';
  image=ReadImage(read_info,exception);
  (void) RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  if (image != (Image *) NULL)
    GetPathComponent(image_info->filename,TailPath,image->filename);
  else
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CoderError,
        "NoDataReturned","`%s'",filename);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r U R L I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterURLImage() adds attributes for the URL image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterURLImage method is:
%
%      size_t RegisterURLImage(void)
%
*/
ModuleExport size_t RegisterURLImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("URL","HTTP","Uniform Resource Locator (http://)");
#if (defined(MAGICKCORE_WINDOWS_SUPPORT) && \
    !defined(__MINGW32__)) || \
    (defined(MAGICKCORE_XML_DELEGATE) && defined(LIBXML_HTTP_ENABLED))
  entry->decoder=(DecodeImageHandler *) ReadURLImage;
#endif
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("URL","HTTPS","Uniform Resource Locator (https://)");
#if defined(MAGICKCORE_WINDOWS_SUPPORT) && \
    !defined(__MINGW32__)
  entry->decoder=(DecodeImageHandler *) ReadURLImage;
#endif
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("URL","FTP","Uniform Resource Locator (ftp://)");
#if (defined(MAGICKCORE_WINDOWS_SUPPORT) && \
    !defined(__MINGW32__)) || \
    (defined(MAGICKCORE_XML_DELEGATE) && defined(LIBXML_FTP_ENABLED))
  entry->decoder=(DecodeImageHandler *) ReadURLImage;
#endif
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("URL","FILE","Uniform Resource Locator (file://)");
  entry->decoder=(DecodeImageHandler *) ReadURLImage;
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r U R L I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterURLImage() removes format registrations made by the
%  URL module from the list of supported formats.
%
%  The format of the UnregisterURLImage method is:
%
%      UnregisterURLImage(void)
%
*/
ModuleExport void UnregisterURLImage(void)
{
  (void) UnregisterMagickInfo("HTTP");
  (void) UnregisterMagickInfo("FTP");
  (void) UnregisterMagickInfo("FILE");
}

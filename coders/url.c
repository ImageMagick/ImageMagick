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
%                                John Cristy                                  %
%                              Bill Radcliffe                                 %
%                                March 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
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
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/resource_.h"
#include "magick/string_.h"
#include "magick/utility.h"
#if defined(MAGICKCORE_XML_DELEGATE)
#  if defined(MAGICKCORE_WINDOWS_SUPPORT)
#    if defined(__MINGW32__) || defined(__MINGW64__)
#      define _MSC_VER
#    else
#      include <win32config.h>
#    endif
#  endif
#  include <libxml/parser.h>
#  include <libxml/xmlmemory.h>
#  include <libxml/nanoftp.h>
#  include <libxml/nanohttp.h>
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
    filename[MaxTextExtent];

  FILE
    *file;

  Image
    *image;

  ImageInfo
    *read_info;

  int
    unique_file;

  image=(Image *) NULL;
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(read_info->filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      read_info=DestroyImageInfo(read_info);
      (void) CopyMagickString(image->filename,read_info->filename,
        MaxTextExtent);
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        image->filename);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) CopyMagickString(filename,image_info->magick,MaxTextExtent);
  (void) ConcatenateMagickString(filename,":",MaxTextExtent);
  LocaleLower(filename);
  (void) ConcatenateMagickString(filename,image_info->filename,MaxTextExtent);
  if (LocaleCompare(read_info->magick,"file") == 0)
    {
      (void) RelinquishUniqueFileResource(read_info->filename);
      unique_file=(-1);
      (void) CopyMagickString(read_info->filename,image_info->filename+2,
        MaxTextExtent);
    }
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
  {
    ExceptionInfo
      *sans;

    ImageInfo
      *clone_info;

    /*
      Guess image format from URL.
    */
    clone_info=CloneImageInfo(image_info);
    sans=AcquireExceptionInfo();
    (void) SetImageInfo(clone_info,0,sans);
    (void) CopyMagickString(read_info->magick,clone_info->magick,MaxTextExtent);
    clone_info=DestroyImageInfo(clone_info);
    sans=DestroyExceptionInfo(sans);
  }
  image=ReadImage(read_info,exception);
  if (unique_file != -1)
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

  entry=SetMagickInfo("HTTP");
#if defined(MAGICKCORE_XML_DELEGATE) && defined(LIBXML_HTTP_ENABLED)
  entry->decoder=(DecodeImageHandler *) ReadURLImage;
#endif
  entry->description=ConstantString("Uniform Resource Locator (http://)");
  entry->module=ConstantString("URL");
  entry->stealth=MagickTrue;
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("FTP");
#if defined(MAGICKCORE_XML_DELEGATE) && defined(LIBXML_FTP_ENABLED)
  entry->decoder=(DecodeImageHandler *) ReadURLImage;
#endif
  entry->description=ConstantString("Uniform Resource Locator (ftp://)");
  entry->module=ConstantString("URL");
  entry->stealth=MagickTrue;
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("FILE");
  entry->decoder=(DecodeImageHandler *) ReadURLImage;
  entry->description=ConstantString("Uniform Resource Locator (file://)");
  entry->module=ConstantString("URL");
  entry->stealth=MagickTrue;
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

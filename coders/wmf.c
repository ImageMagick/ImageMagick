/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             W   W   M   M  FFFFF                            %
%                             W   W   MM MM  F                                %
%                             W W W   M M M  FFF                              %
%                             WW WW   M   M  F                                %
%                             W   W   M   M  F                                %
%                                                                             %
%                                                                             %
%                        Read Windows Metafile Format                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               December 2000                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/paint.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/type.h"
#include "magick/module.h"
#include "wand/MagickWand.h"

#if defined(MAGICKCORE_WMF_DELEGATE)
#include "libwmf/api.h"
#include "libwmf/eps.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d W M F I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadWMFImage() reads an Windows Metafile image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadWMFImage method is:
%
%      Image *ReadWMFImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static int WMFReadBlob(void *image)
{
  return(ReadBlobByte((Image *) image));
}

static int WMFSeekBlob(void *image,long offset)
{
  return((int) SeekBlob((Image *) image,(MagickOffsetType) offset,SEEK_SET));
}

static long WMFTellBlob(void *image)
{
  return((long) TellBlob((Image*) image));
}

static Image *ReadWMFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    filename[MaxTextExtent];

  int
    unique_file;

  FILE
    *file;

  Image
    *image;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  size_t
    flags;

  wmfAPI
    *wmf_info;

  wmfAPI_Options
    options;

  wmfD_Rect
    bounding_box;

  wmf_eps_t
    *eps_info;

  wmf_error_t
    wmf_status;

  /*
    Read WMF image.
  */
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  wmf_info=(wmfAPI *) NULL;
  flags=0;
  flags|=WMF_OPT_IGNORE_NONFATAL;
  flags|=WMF_OPT_FUNCTION;
  options.function=wmf_eps_function;
  wmf_status=wmf_api_create(&wmf_info,(unsigned long) flags,&options);
  if (wmf_status != wmf_E_None)
    {
      if (wmf_info != (wmfAPI *) NULL)
        wmf_api_destroy(wmf_info);
      ThrowReaderException(DelegateError,"UnableToInitializeWMFLibrary");
    }
  wmf_status=wmf_bbuf_input(wmf_info,WMFReadBlob,WMFSeekBlob,WMFTellBlob,
    (void *) image);
  if (wmf_status != wmf_E_None)
    {
      wmf_api_destroy(wmf_info);
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image->filename);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  wmf_status=wmf_scan(wmf_info,0,&bounding_box);
  if (wmf_status != wmf_E_None)
    {
      wmf_api_destroy(wmf_info);
      ThrowReaderException(DelegateError,"FailedToScanFile");
    }
  eps_info=WMF_EPS_GetData(wmf_info);
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      wmf_api_destroy(wmf_info);
      ThrowReaderException(FileOpenError,"UnableToCreateTemporaryFile");
    }
  eps_info->out=wmf_stream_create(wmf_info,file);
  eps_info->bbox=bounding_box;
  wmf_status=wmf_play(wmf_info,0,&bounding_box);
  if (wmf_status != wmf_E_None)
    {
      wmf_api_destroy(wmf_info);
      ThrowReaderException(DelegateError,"FailedToRenderFile");
    }
  (void) fclose(file);
  wmf_api_destroy(wmf_info);
  (void) CloseBlob(image);
  image=DestroyImage(image);
  /*
    Read EPS image.
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) FormatLocaleString(read_info->filename,MaxTextExtent,"eps:%s",
    filename);
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image != (Image *) NULL)
    {
      (void) CopyMagickString(image->filename,image_info->filename,
        MaxTextExtent);
      (void) CopyMagickString(image->magick_filename,image_info->filename,
        MaxTextExtent);
      (void) CopyMagickString(image->magick,"WMF",MaxTextExtent);
    }
  (void) RelinquishUniqueFileResource(filename);
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r W M F I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterWMFImage() adds attributes for the WMF image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterWMFImage method is:
%
%      size_t RegisterWMFImage(void)
%
*/
ModuleExport size_t RegisterWMFImage(void)
{
  MagickInfo
    *entry;

  entry = SetMagickInfo("WMZ");
#if defined(MAGICKCORE_WMF_DELEGATE)
  entry->decoder=ReadWMFImage;
#endif
  entry->description=ConstantString("Compressed Windows Meta File");
  entry->module=ConstantString("WMZ");
  entry->seekable_stream=MagickTrue;
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("WMF");
#if defined(MAGICKCORE_WMF_DELEGATE)
  entry->decoder=ReadWMFImage;
#endif
  entry->description=ConstantString("Windows Meta File");
  entry->module=ConstantString("WMF");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r W M F I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterWMFImage() removes format registrations made by the
%  WMF module from the list of supported formats.
%
%  The format of the UnregisterWMFImage method is:
%
%      UnregisterWMFImage(void)
%
*/
ModuleExport void UnregisterWMFImage(void)
{
  (void) UnregisterMagickInfo("WMZ");
  (void) UnregisterMagickInfo("WMF");
}

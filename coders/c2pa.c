/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                          CCCC  22222  PPPP    AAA                           %
%                         C         22  P   P  A   A                          %
%                         C       22    PPPP   AAAAA                          %
%                         C      22     P      A   A                          %
%                          CCCC  22222  P      A   A                          %
%                                                                             %
%                                                                             %
%                          C2PA Provenance Metadata.                          %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                              September 2026                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteC2PAImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C 2 P A I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ReadC2PAImage() reads an image that is signed with C2PA content
%  credentials.  The image is handed to the c2pa:decode delegate which
%  validates the manifest and returns an image ImageMagick natively
%  understands.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadC2PAImage method is:
%
%      Image *ReadC2PAImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadC2PAImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  ExceptionInfo
    *sans_exception;

  Image
    *image,
    *images,
    *next;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

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
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) CloseBlob(image);
  /*
    Blob support is disabled for this coder, hence the filename always
    references a seekable file on disk that the delegate can open (%i).
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  read_info->temporary=MagickFalse;
  (void) CopyMagickString(read_info->filename,image_info->filename,
    MagickPathExtent);
  (void) CopyMagickString(image->filename,image_info->filename,
    MagickPathExtent);
  status=InvokeDelegate(read_info,image,"c2pa:decode",(char *) NULL,exception);
  image=DestroyImageList(image);
  if (status == MagickFalse)
    {
      if (*read_info->unique != '\0')
        (void) RelinquishUniqueFileResource(read_info->unique);
      read_info=DestroyImageInfo(read_info);
      return((Image *) NULL);
    }
  /*
    Read the image the delegate returned in %o.
  */
  (void) CopyMagickString(read_info->filename,read_info->unique,
    MagickPathExtent);
  *read_info->magick='\0';
  sans_exception=AcquireExceptionInfo();
  (void) SetImageInfo(read_info,1,sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  images=ReadImage(read_info,exception);
  (void) RelinquishUniqueFileResource(read_info->unique);
  read_info=DestroyImageInfo(read_info);
  if (images == (Image *) NULL)
    return((Image *) NULL);
  for (next=images; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    (void) CopyMagickString(next->filename,image_info->filename,
      MagickPathExtent);
    (void) CopyMagickString(next->magick,"C2PA",MagickPathExtent);
    (void) CopyMagickString(next->magick_filename,image_info->filename,
      MagickPathExtent);
  }
  return(GetFirstImageInList(images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r C 2 P A I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterC2PAImage() adds attributes for the C2PA image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterC2PAImage method is:
%
%      size_t RegisterC2PAImage(void)
%
*/
ModuleExport size_t RegisterC2PAImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("C2PA","C2PA","C2PA Provenance Metadata");
  entry->decoder=(DecodeImageHandler *) ReadC2PAImage;
  entry->encoder=(EncodeImageHandler *) WriteC2PAImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r C 2 P A I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterC2PAImage() removes format registrations made by the
%  C2PA module from the list of supported formats.
%
%  The format of the UnregisterC2PAImage method is:
%
%      UnregisterC2PAImage(void)
%
*/
ModuleExport void UnregisterC2PAImage(void)
{
  (void) UnregisterMagickInfo("C2PA");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e C 2 P A I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteC2PAImage() writes an image and signs it with C2PA content
%  credentials.  The image is first written to a temporary file in a format
%  the C2PA delegate understands (see the c2pa:format define, PNG by default)
%  and is then handed to the c2pa:encode delegate for signing.  The signed
%  bytes the delegate returns in %o are copied verbatim to the destination so
%  the manifest and its signature remain intact.
%
%  The format of the WriteC2PAImage method is:
%
%      MagickBooleanType WriteC2PAImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WriteC2PAImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  static const char
    *credentials[] = { "c2pa:key", "c2pa:password", "c2pa:cert",
      "c2pa:manifest", (const char *) NULL };

  char
    *buffer,
    format[MagickPathExtent],
    unique[MagickPathExtent];

  const char
    *option;

  FILE
    *file;

  Image
    *write_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  size_t
    length,
    quantum;

  ssize_t
    count,
    i;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  /*
    A manifest is required to sign an image.
  */
  option=GetImageOption(image_info,"c2pa:manifest");
  if (option == (const char *) NULL)
    option=GetImageArtifact(image,"c2pa:manifest");
  if (option == (const char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "MustSpecifyImageManifest","`%s'",image->filename);
      return(MagickFalse);
    }
  /*
    Determine the intermediate image format the delegate signs.
  */
  option=GetImageOption(image_info,"c2pa:format");
  if (option == (const char *) NULL)
    option=GetImageArtifact(image,"c2pa:format");
  if (option != (const char *) NULL)
    (void) CopyMagickString(format,option,MagickPathExtent);
  else
    if ((*image->magick != '\0') &&
        (LocaleCompare(image->magick,"C2PA") != 0))
      (void) CopyMagickString(format,image->magick,MagickPathExtent);
    else
      (void) CopyMagickString(format,"PNG",MagickPathExtent);
  /*
    Write the image to a unique file the delegate signs (%i).
  */
  if (AcquireUniqueFilename(unique) == MagickFalse)
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        image->filename);
      return(MagickFalse);
    }
  write_image=CloneImage(image,0,0,MagickTrue,exception);
  if (write_image == (Image *) NULL)
    {
      (void) RelinquishUniqueFileResource(unique);
      return(MagickFalse);
    }
  write_info=CloneImageInfo(image_info);
  SetImageInfoBlob(write_info,(void *) NULL,0);
  write_info->temporary=MagickFalse;
  write_info->adjoin=MagickTrue;
  (void) CopyMagickString(write_info->magick,format,MagickPathExtent);
  (void) FormatLocaleString(write_info->filename,MagickPathExtent,"%s:%s",
    format,unique);
  (void) CopyMagickString(write_image->filename,write_info->filename,
    MagickPathExtent);
  status=WriteImage(write_info,write_image,exception);
  if (status == MagickFalse)
    {
      write_info=DestroyImageInfo(write_info);
      write_image=DestroyImage(write_image);
      (void) RelinquishUniqueFileResource(unique);
      return(MagickFalse);
    }
  /*
    Promote the signing credentials to artifacts so %[c2pa:*] resolves when
    the delegate command is interpreted.
  */
  for (i=0; credentials[i] != (const char *) NULL; i++)
  {
    option=GetImageOption(image_info,credentials[i]);
    if (option != (const char *) NULL)
      (void) SetImageArtifact(write_image,credentials[i],option);
  }
  /*
    Sign the image with the c2pa:encode delegate.
  */
  (void) CopyMagickString(write_info->filename,unique,MagickPathExtent);
  (void) CopyMagickString(write_image->filename,unique,MagickPathExtent);
  (void) CopyMagickString(write_image->magick,format,MagickPathExtent);
  status=InvokeDelegate(write_info,write_image,(char *) NULL,"c2pa:encode",
    exception);
  write_image=DestroyImage(write_image);
  (void) RelinquishUniqueFileResource(unique);
  if (status == MagickFalse)
    {
      if (*write_info->unique != '\0')
        (void) RelinquishUniqueFileResource(write_info->unique);
      write_info=DestroyImageInfo(write_info);
      return(MagickFalse);
    }
  /*
    Copy the signed image the delegate returned in %o; the bytes are written
    verbatim so the manifest and its signature are preserved.
  */
  file=fopen_utf8(write_info->unique,"rb");
  if (file == (FILE *) NULL)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        write_info->unique);
      (void) RelinquishUniqueFileResource(write_info->unique);
      write_info=DestroyImageInfo(write_info);
      return(MagickFalse);
    }
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      (void) fclose(file);
      (void) RelinquishUniqueFileResource(write_info->unique);
      write_info=DestroyImageInfo(write_info);
      return(MagickFalse);
    }
  quantum=(size_t) MagickMaxBufferExtent;
  buffer=(char *) AcquireQuantumMemory(quantum,sizeof(*buffer));
  if (buffer == (char *) NULL)
    {
      (void) fclose(file);
      (void) CloseBlob(image);
      (void) RelinquishUniqueFileResource(write_info->unique);
      write_info=DestroyImageInfo(write_info);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  for (length=fread(buffer,1,quantum,file); length != 0; )
  {
    count=0;
    for (i=0; i < (ssize_t) length; i+=count)
    {
      count=WriteBlob(image,length-(size_t) i,(unsigned char *) buffer+i);
      if (count <= 0)
        break;
    }
    if (i < (ssize_t) length)
      break;
    length=fread(buffer,1,quantum,file);
  }
  if (ferror(file) != 0)
    status=MagickFalse;
  (void) fclose(file);
  buffer=(char *) RelinquishMagickMemory(buffer);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  (void) RelinquishUniqueFileResource(write_info->unique);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

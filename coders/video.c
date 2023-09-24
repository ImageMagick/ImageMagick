/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     V   V  IIIII  DDDD   EEEEE   OOO                        %
%                     V   V    I    D   D  E      O   O                       %
%                     V   V    I    D   D  EEE    O   O                       %
%                      V V     I    D   D  E      O   O                       %
%                       V    IIIII  DDDD   EEEEE   OOO                        %
%                                                                             %
%                                                                             %
%                       Read/Write VIDEO Image Format                         %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1999                                   %
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
#include "MagickCore/delegate.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/layer.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/option.h"
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"

/*
  Global declarations.
*/
static const char
  *intermediate_formats[] =
  {
    "pam",
    "webp"
  };

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteVIDEOImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s V I D E O                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsVIDEO() returns MagickTrue if the image format type, identified by the
%  magick string, is VIDEO.
%
%  The format of the IsVIDEO method is:
%
%      MagickBooleanType IsVIDEO(const unsigned char *magick,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/

static MagickBooleanType IsPNG(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);
  if (memcmp(magick,"\211PNG\r\n\032\n",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

static MagickBooleanType IsVIDEO(const unsigned char *magick,
  const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\000\000\001\263",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d V I D E O I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadVIDEOImage() reads an binary file in the VIDEO video stream format and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadVIDEOImage method is:
%
%      Image *ReadVIDEOImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static const char *GetIntermediateFormat(const ImageInfo *image_info)
{
  const char
    *option;
 
  option=GetImageOption(image_info,"video:intermediate-format");
  if (LocaleCompare(option,"pam") == 0)
    return(intermediate_formats[0]);
#if defined(MAGICKCORE_WEBP_DELEGATE)
  return(intermediate_formats[1]);
#else
  return(intermediate_formats[0]);
#endif
}

static Image *ReadVIDEOImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  const DelegateInfo
    *delegate_info;

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
  (void) DestroyImageList(image);
  /*
    Convert VIDEO to PAM with delegate.
  */
  images=(Image *) NULL;
  read_info=CloneImageInfo(image_info);
  delegate_info=GetDelegateInfo("video:decode",(char *) NULL,exception);
  if (delegate_info != (const DelegateInfo *) NULL)
    {
      char
        command[MagickPathExtent],
        message[MagickPathExtent],
        *options;

      const char
        *intermediate_format,
        *option;

      int
        exit_code;

      options=AcquireString("");
      if (image_info->number_scenes > 0)
        (void) FormatLocaleString(options,MagickPathExtent,"-vframes %i",
          (int) image_info->number_scenes);
      option=GetImageOption(image_info,"video:vsync");
      if (option != (const char *) NULL)
        {
          FormatSanitizedDelegateOption(command,MagickPathExtent,
            " -vsync \"%s\""," -vsync '%s'",option);
          (void) ConcatenateMagickString(options,command,MagickPathExtent);
        }
      option=GetImageOption(image_info,"video:pixel-format");
      if (option != (const char *) NULL)
        {
          FormatSanitizedDelegateOption(command,MagickPathExtent,
            " -pix_fmt \"%s\""," -pix_fmt '%s'",option);
          (void) ConcatenateMagickString(options,command,MagickPathExtent);
        }
      else
        if (LocaleNCompare(image_info->magick,"APNG",MagickPathExtent) == 0)
          (void) ConcatenateMagickString(options," -pix_fmt rgba",
            MagickPathExtent);
      intermediate_format=GetIntermediateFormat(image_info);
      (void) FormatLocaleString(command,MagickPathExtent,
        " -vcodec %s -lossless 1",intermediate_format);
      (void) ConcatenateMagickString(options,command,MagickPathExtent);
      AcquireUniqueFilename(read_info->unique);
      (void) AcquireUniqueSymbolicLink(image_info->filename,
        read_info->filename);
      (void) FormatLocaleString(command,MagickPathExtent,
        GetDelegateCommands(delegate_info),read_info->filename,options,
        read_info->unique);
      options=DestroyString(options);
      (void) CopyMagickString(read_info->magick,intermediate_format,
        MagickPathExtent);
      (void) CopyMagickString(read_info->filename,read_info->unique,
        MagickPathExtent);
      exit_code=ExternalDelegateCommand(MagickFalse,image_info->verbose,
        command,message,exception);
      if (exit_code == 0)
        images=ReadImage(read_info,exception);
      else
        if (*message != '\0')
          (void) ThrowMagickException(exception,GetMagickModule(),DelegateError,
            "VideoDelegateFailed","`%s'",message);
      (void) RelinquishUniqueFileResource(read_info->filename);
      (void) RelinquishUniqueFileResource(read_info->unique);
      if (images != (Image *) NULL)
        for (next=images; next != (Image *) NULL; next=next->next)
        {
          (void) CopyMagickString(next->filename,image_info->filename,
            MagickPathExtent);
          (void) CopyMagickString(next->magick,image_info->magick,
            MagickPathExtent);
        }
    }
  read_info=DestroyImageInfo(read_info);
  return(images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r V I D E O I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterVIDEOImage() adds attributes for the VIDEO image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterVIDEOImage method is:
%
%      size_t RegisterVIDEOImage(void)
%
*/
ModuleExport size_t RegisterVIDEOImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("VIDEO","3GP","Media Container");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->flags^=CoderBlobSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","3G2","Media Container");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->flags^=CoderBlobSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","APNG","Animated Portable Network Graphics");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","AVI","Microsoft Audio/Visual Interleaved");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","FLV","Flash Video Stream");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","MKV","Multimedia Container");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","MOV","MPEG Video Stream");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","MPEG","MPEG Video Stream");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","MPG","MPEG Video Stream");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","MP4","VIDEO-4 Video Stream");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","M2V","MPEG Video Stream");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","M4V","Raw VIDEO-4 Video");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","WEBM","Open Web Media");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("VIDEO","WMV","Windows Media Video");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r V I D E O I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterVIDEOImage() removes format registrations made by the
%  BIM module from the list of supported formats.
%
%  The format of the UnregisterBIMImage method is:
%
%      UnregisterVIDEOImage(void)
%
*/
ModuleExport void UnregisterVIDEOImage(void)
{
  (void) UnregisterMagickInfo("WMV");
  (void) UnregisterMagickInfo("WEBM");
  (void) UnregisterMagickInfo("MOV");
  (void) UnregisterMagickInfo("M4V");
  (void) UnregisterMagickInfo("M2V");
  (void) UnregisterMagickInfo("MP4");
  (void) UnregisterMagickInfo("MPG");
  (void) UnregisterMagickInfo("MPEG");
  (void) UnregisterMagickInfo("MKV");
  (void) UnregisterMagickInfo("AVI");
  (void) UnregisterMagickInfo("APNG");
  (void) UnregisterMagickInfo("3G2");
  (void) UnregisterMagickInfo("3GP");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e V I D E O I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteVIDEOImage() writes an image to a file in VIDEO video stream format.
%  Lawrence Livermore National Laboratory (LLNL) contributed code to adjust
%  the VIDEO parameters to correspond to the compression quality setting.
%
%  The format of the WriteVIDEOImage method is:
%
%      MagickBooleanType WriteVIDEOImage(const ImageInfo *image_info,
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
static MagickBooleanType CopyDelegateFile(const char *source,
  const char *destination)
{
  int
    destination_file,
    source_file;

  size_t
    i,
    length,
    quantum;

  ssize_t
    count;

  struct stat
    attributes;

  unsigned char
    *buffer;

  assert(source != (const char *) NULL);
  assert(destination != (char *) NULL);
  /*
    Copy source file to destination.
  */
  if (strcmp(destination,"-") == 0)
    destination_file=fileno(stdout);
  else
    destination_file=open_utf8(destination,O_WRONLY | O_BINARY | O_CREAT |
      O_TRUNC,S_MODE);
  if (destination_file == -1)
    return(MagickFalse);
  source_file=open_utf8(source,O_RDONLY | O_BINARY,0);
  if (source_file == -1)
    {
      (void) close(destination_file);
      return(MagickFalse);
    }
  quantum=(size_t) MagickMaxBufferExtent;
  if ((fstat(source_file,&attributes) == 0) && (attributes.st_size > 0))
    quantum=(size_t) MagickMin((double) attributes.st_size,
      MagickMaxBufferExtent);
  buffer=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    {
      (void) close(source_file);
      (void) close(destination_file);
      return(MagickFalse);
    }
  length=0;
  for (i=0; ; i+=(size_t) count)
  {
    count=(ssize_t) read(source_file,buffer,quantum);
    if (count <= 0)
      break;
    length=(size_t) count;
    count=(ssize_t) write(destination_file,buffer,length);
    if ((size_t) count != length)
      break;
  }
  if (strcmp(destination,"-") != 0)
    (void) close(destination_file);
  (void) close(source_file);
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  return(i != 0 ? MagickTrue : MagickFalse);
}

static MagickBooleanType WriteVIDEOImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  char
    basename[MagickPathExtent],
    filename[MagickPathExtent];

  const char
    *intermediate_format;

  const DelegateInfo
    *delegate_info;

  double
    delay;

  Image
    *clone_images,
    *p;

  ImageInfo
    *write_info;

  int
    file;

  MagickBooleanType
    status;

  size_t
    count,
    length,
    scene;

  ssize_t
    i;

  unsigned char
    *blob;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  /*
    Write intermediate files.
  */
  clone_images=CloneImageList(image,exception);
  if (clone_images == (Image *) NULL)
    return(MagickFalse);
  file=AcquireUniqueFileResource(basename);
  if (file != -1)
    file=close(file)-1;
  (void) FormatLocaleString(clone_images->filename,MagickPathExtent,"%s",
    basename);
  count=0;
  write_info=CloneImageInfo(image_info);
  write_info->file=(FILE *) NULL;
  *write_info->magick='\0';
  status=MagickTrue;
  intermediate_format=GetIntermediateFormat(image_info);
  for (p=clone_images; p != (Image *) NULL; p=GetNextImageInList(p))
  {
    char
      previous_image[MagickPathExtent];

    blob=(unsigned char *) NULL;
    length=0;
    scene=p->scene;
    delay=100.0*p->delay/MagickMax(1.0*p->ticks_per_second,1.0);
    for (i=0; i < (ssize_t) MagickMax((1.0*delay+1.0)/3.0,1.0); i++)
    {
      p->scene=count;
      count++;
      status=MagickFalse;
      switch (i)
      {
        case 0:
        {
          Image
            *frame;

          (void) FormatLocaleString(p->filename,MagickPathExtent,"%s%.20g.%s",
            basename,(double) p->scene,intermediate_format);
          (void) FormatLocaleString(filename,MagickPathExtent,"%s%.20g.%s",
            basename,(double) p->scene,intermediate_format);
          (void) FormatLocaleString(previous_image,MagickPathExtent,
            "%s%.20g.%s",basename,(double) p->scene,intermediate_format);
          frame=CloneImage(p,0,0,MagickTrue,exception);
          if (frame == (Image *) NULL)
            break;
          status=WriteImage(write_info,frame,exception);
          frame=DestroyImage(frame);
          break;
        }
        case 1:
        {
          blob=(unsigned char *) FileToBlob(previous_image,SIZE_MAX,&length,
            exception);
          magick_fallthrough;
        }
        default:
        {
          (void) FormatLocaleString(filename,MagickPathExtent,"%s%.20g.%s",
            basename,(double) p->scene,intermediate_format);
          if (length > 0)
            status=BlobToFile(filename,blob,length,exception);
          break;
        }
      }
      if (image->debug != MagickFalse)
        {
          if (status != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "%.20g. Wrote %s file for scene %.20g:",(double) i,
              intermediate_format,(double) p->scene);
          else
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "%.20g. Failed to write %s file for scene %.20g:",(double) i,
              intermediate_format,(double) p->scene);
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"%s",filename);
        }
    }
    p->scene=scene;
    if (blob != (unsigned char *) NULL)
      blob=(unsigned char *) RelinquishMagickMemory(blob);
    if (status == MagickFalse)
      break;
  }
  /*
    Convert PAM to VIDEO.
  */
  delegate_info=GetDelegateInfo((char *) NULL,"video:encode",exception);
  if (delegate_info != (const DelegateInfo *) NULL)
    {
      char
        command[MagickPathExtent],
        message[MagickPathExtent],
        *options;

      const char
        *option;

      int
        exit_code;

      options=AcquireString("");
      (void) FormatLocaleString(options,MagickPathExtent,"-plays %i",
        (int) clone_images->iterations);
      option=GetImageOption(image_info,"video:pixel-format");
      if (option != (const char *) NULL)
        {
          FormatSanitizedDelegateOption(command,MagickPathExtent,
            " -pix_fmt \"%s\""," -pix_fmt '%s'",option);
          (void) ConcatenateMagickString(options,command,MagickPathExtent);
        }
      AcquireUniqueFilename(write_info->unique);
      (void) FormatLocaleString(command,MagickPathExtent,
        GetDelegateCommands(delegate_info),basename,intermediate_format,
        options,write_info->unique,image_info->magick);
      options=DestroyString(options);
      exit_code=ExternalDelegateCommand(MagickFalse,image_info->verbose,
        command,message,exception);
      status=exit_code == 0 ? MagickTrue : MagickFalse;
      if (status != MagickFalse)
        {
          (void) FormatLocaleString(filename,MagickPathExtent,"%s.%s",
            write_info->unique,image_info->magick);
          status=CopyDelegateFile(filename,image->filename);
          (void) RelinquishUniqueFileResource(filename);
        }
      else
        if (*message != '\0')
          (void) ThrowMagickException(exception,GetMagickModule(),
            DelegateError,"VideoDelegateFailed","`%s'",message);
      (void) RelinquishUniqueFileResource(write_info->unique);
  }
  write_info=DestroyImageInfo(write_info);
  /*
    Relinquish resources.
  */
  count=0;
  for (p=clone_images; p != (Image *) NULL; p=GetNextImageInList(p))
  {
    delay=100.0*p->delay/MagickMax(1.0*p->ticks_per_second,1.0);
    for (i=0; i < (ssize_t) MagickMax((1.0*delay+1.0)/3.0,1.0); i++)
    {
      (void) FormatLocaleString(p->filename,MagickPathExtent,"%s%.20g.%s",
        basename,(double) count++,intermediate_format);
      (void) RelinquishUniqueFileResource(p->filename);
    }
    (void) CopyMagickString(p->filename,image_info->filename,MagickPathExtent);
  }
  (void) RelinquishUniqueFileResource(basename);
  clone_images=DestroyImageList(clone_images);
  return(status);
}

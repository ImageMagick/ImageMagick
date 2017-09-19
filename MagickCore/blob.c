/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         BBBB   L       OOO   BBBB                           %
%                         B   B  L      O   O  B   B                          %
%                         BBBB   L      O   O  BBBB                           %
%                         B   B  L      O   O  B   B                          %
%                         BBBB   LLLLL   OOO   BBBB                           %
%                                                                             %
%                                                                             %
%                     MagickCore Binary Large OBjectS Methods                 %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1999                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
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
%
*/

/*
  Include declarations.
*/
#ifdef __VMS
#include  <types.h>
#include  <mman.h>
#endif
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/client.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/locale_.h"
#include "MagickCore/log.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/policy.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#if defined(MAGICKCORE_ZLIB_DELEGATE)
#include "zlib.h"
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
#include "bzlib.h"
#endif

/*
  Define declarations.
*/
#define MagickMaxBlobExtent  (8*8192)
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
# define MAP_ANONYMOUS  MAP_ANON
#endif
#if !defined(MAP_FAILED)
#define MAP_FAILED  ((void *) -1)
#endif
#if defined(__OS2__)
#include <io.h>
#define _O_BINARY O_BINARY
#endif

/*
  Typedef declarations.
*/
typedef union FileInfo
{
  FILE
    *file;

#if defined(MAGICKCORE_ZLIB_DELEGATE)
  gzFile
    gzfile;
#endif

#if defined(MAGICKCORE_BZLIB_DELEGATE)
  BZFILE
    *bzfile;
#endif
} FileInfo;

struct _BlobInfo
{
  size_t
    length,
    extent,
    quantum;

  MagickBooleanType
    mapped,
    eof;

  MagickOffsetType
    offset;

  MagickSizeType
    size;

  MagickBooleanType
    exempt,
    synchronize,
    status,
    temporary;

  StreamType
    type;

  FileInfo
    file_info;

  struct stat
    properties;

  StreamHandler
    stream;

  CustomStreamInfo
    *custom_stream;

  unsigned char
    *data;

  MagickBooleanType
    debug;

  SemaphoreInfo
    *semaphore;

  ssize_t
    reference_count;

  size_t
    signature;
};

struct _CustomStreamInfo
{
  CustomStreamHandler
    reader,
    writer;

  CustomStreamSeeker
    seeker;

  CustomStreamTeller
    teller;

  void
    *data;

  size_t
    signature;
};

/*
  Forward declarations.
*/
static int
  SyncBlob(Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e C u s t o m S t r e a m I n f o                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCustomStreamInfo() allocates the CustomStreamInfo structure.
%
%  The format of the AcquireCustomStreamInfo method is:
%
%      CustomStreamInfo *AcquireCustomStreamInfo(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport CustomStreamInfo *AcquireCustomStreamInfo(
  ExceptionInfo *magick_unused(exception))
{
  CustomStreamInfo
    *custom_stream;

  magick_unreferenced(exception);
  custom_stream=(CustomStreamInfo *) AcquireMagickMemory(
    sizeof(*custom_stream));
  if (custom_stream == (CustomStreamInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(custom_stream,0,sizeof(*custom_stream));
  custom_stream->signature=MagickCoreSignature;
  return(custom_stream);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A t t a c h B l o b                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AttachBlob() attaches a blob to the BlobInfo structure.
%
%  The format of the AttachBlob method is:
%
%      void AttachBlob(BlobInfo *blob_info,const void *blob,const size_t length)
%
%  A description of each parameter follows:
%
%    o blob_info: Specifies a pointer to a BlobInfo structure.
%
%    o blob: the address of a character stream in one of the image formats
%      understood by ImageMagick.
%
%    o length: This size_t integer reflects the length in bytes of the blob.
%
*/
MagickExport void AttachBlob(BlobInfo *blob_info,const void *blob,
  const size_t length)
{
  assert(blob_info != (BlobInfo *) NULL);
  if (blob_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  blob_info->length=length;
  blob_info->extent=length;
  blob_info->quantum=(size_t) MagickMaxBlobExtent;
  blob_info->offset=0;
  blob_info->type=BlobStream;
  blob_info->file_info.file=(FILE *) NULL;
  blob_info->data=(unsigned char *) blob;
  blob_info->mapped=MagickFalse;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A t t a c h C u s t o m S t r e a m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AttachCustomStream() attaches a CustomStreamInfo to the BlobInfo structure.
%
%  The format of the AttachCustomStream method is:
%
%      void AttachCustomStream(BlobInfo *blob_info,
%        CustomStreamInfo *custom_stream)
%
%  A description of each parameter follows:
%
%    o blob_info: specifies a pointer to a BlobInfo structure.
%
%    o custom_stream: the custom stream info.
%
*/
MagickExport void AttachCustomStream(BlobInfo *blob_info,
  CustomStreamInfo *custom_stream)
{
  assert(blob_info != (BlobInfo *) NULL);
  assert(custom_stream != (CustomStreamInfo *) NULL);
  assert(custom_stream->signature == MagickCoreSignature);
  if (blob_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  blob_info->type=CustomStream;
  blob_info->custom_stream=custom_stream;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   B l o b T o F i l e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BlobToFile() writes a blob to a file.  It returns MagickFalse if an error
%  occurs otherwise MagickTrue.
%
%  The format of the BlobToFile method is:
%
%       MagickBooleanType BlobToFile(char *filename,const void *blob,
%         const size_t length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: Write the blob to this file.
%
%    o blob: the address of a blob.
%
%    o length: This length in bytes of the blob.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType BlobToFile(char *filename,const void *blob,
  const size_t length,ExceptionInfo *exception)
{
  int
    file;

  register size_t
    i;

  ssize_t
    count;

  assert(filename != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  assert(blob != (const void *) NULL);
  if (*filename == '\0')
    file=AcquireUniqueFileResource(filename);
  else
    file=open_utf8(filename,O_RDWR | O_CREAT | O_EXCL | O_BINARY,S_MODE);
  if (file == -1)
    {
      ThrowFileException(exception,BlobError,"UnableToWriteBlob",filename);
      return(MagickFalse);
    }
  for (i=0; i < length; i+=count)
  {
    count=write(file,(const char *) blob+i,MagickMin(length-i,SSIZE_MAX));
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  file=close(file);
  if ((file == -1) || (i < length))
    {
      ThrowFileException(exception,BlobError,"UnableToWriteBlob",filename);
      return(MagickFalse);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B l o b T o I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BlobToImage() implements direct to memory image formats.  It returns the
%  blob as an image.
%
%  The format of the BlobToImage method is:
%
%      Image *BlobToImage(const ImageInfo *image_info,const void *blob,
%        const size_t length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o blob: the address of a character stream in one of the image formats
%      understood by ImageMagick.
%
%    o length: This size_t integer reflects the length in bytes of the blob.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *BlobToImage(const ImageInfo *image_info,const void *blob,
  const size_t length,ExceptionInfo *exception)
{
  const MagickInfo
    *magick_info;

  Image
    *image;

  ImageInfo
    *blob_info,
    *clone_info;

  MagickBooleanType
    status;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  if ((blob == (const void *) NULL) || (length == 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),BlobError,
        "ZeroLengthBlobNotPermitted","`%s'",image_info->filename);
      return((Image *) NULL);
    }
  blob_info=CloneImageInfo(image_info);
  blob_info->blob=(void *) blob;
  blob_info->length=length;
  if (*blob_info->magick == '\0')
    (void) SetImageInfo(blob_info,0,exception);
  magick_info=GetMagickInfo(blob_info->magick,exception);
  if (magick_info == (const MagickInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        MissingDelegateError,"NoDecodeDelegateForThisImageFormat","`%s'",
        blob_info->magick);
      blob_info=DestroyImageInfo(blob_info);
      return((Image *) NULL);
    }
  if (GetMagickBlobSupport(magick_info) != MagickFalse)
    {
      /*
        Native blob support for this image format.
      */
      (void) CopyMagickString(blob_info->filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(blob_info->magick,image_info->magick,
        MagickPathExtent);
      image=ReadImage(blob_info,exception);
      if (image != (Image *) NULL)
        (void) DetachBlob(image->blob);
      blob_info=DestroyImageInfo(blob_info);
      return(image);
    }
  /*
    Write blob to a temporary file on disk.
  */
  blob_info->blob=(void *) NULL;
  blob_info->length=0;
  *blob_info->filename='\0';
  status=BlobToFile(blob_info->filename,blob,length,exception);
  if (status == MagickFalse)
    {
      (void) RelinquishUniqueFileResource(blob_info->filename);
      blob_info=DestroyImageInfo(blob_info);
      return((Image *) NULL);
    }
  clone_info=CloneImageInfo(blob_info);
  (void) FormatLocaleString(clone_info->filename,MagickPathExtent,"%s:%s",
    blob_info->magick,blob_info->filename);
  image=ReadImage(clone_info,exception);
  if (image != (Image *) NULL)
    {
      Image
        *images;

      /*
        Restore original filenames and image format.
      */
      for (images=GetFirstImageInList(image); images != (Image *) NULL; )
      {
        (void) CopyMagickString(images->filename,image_info->filename,
          MagickPathExtent);
        (void) CopyMagickString(images->magick_filename,image_info->filename,
          MagickPathExtent);
        (void) CopyMagickString(images->magick,magick_info->name,
          MagickPathExtent);
        images=GetNextImageInList(images);
      }
    }
  clone_info=DestroyImageInfo(clone_info);
  (void) RelinquishUniqueFileResource(blob_info->filename);
  blob_info=DestroyImageInfo(blob_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o n e B l o b I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneBlobInfo() makes a duplicate of the given blob info structure, or if
%  blob info is NULL, a new one.
%
%  The format of the CloneBlobInfo method is:
%
%      BlobInfo *CloneBlobInfo(const BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
*/
MagickExport BlobInfo *CloneBlobInfo(const BlobInfo *blob_info)
{
  BlobInfo
    *clone_info;

  clone_info=(BlobInfo *) AcquireMagickMemory(sizeof(*clone_info));
  if (clone_info == (BlobInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  GetBlobInfo(clone_info);
  if (blob_info == (BlobInfo *) NULL)
    return(clone_info);
  clone_info->length=blob_info->length;
  clone_info->extent=blob_info->extent;
  clone_info->synchronize=blob_info->synchronize;
  clone_info->quantum=blob_info->quantum;
  clone_info->mapped=blob_info->mapped;
  clone_info->eof=blob_info->eof;
  clone_info->offset=blob_info->offset;
  clone_info->size=blob_info->size;
  clone_info->exempt=blob_info->exempt;
  clone_info->status=blob_info->status;
  clone_info->temporary=blob_info->temporary;
  clone_info->type=blob_info->type;
  clone_info->file_info.file=blob_info->file_info.file;
  clone_info->properties=blob_info->properties;
  clone_info->stream=blob_info->stream;
  clone_info->custom_stream=blob_info->custom_stream;
  clone_info->data=blob_info->data;
  clone_info->debug=IsEventLogging();
  clone_info->reference_count=1;
  return(clone_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o s e B l o b                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloseBlob() closes a stream associated with the image.
%
%  The format of the CloseBlob method is:
%
%      MagickBooleanType CloseBlob(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType CloseBlob(Image *image)
{
  int
    status;

  /*
    Close image file.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->blob != (BlobInfo *) NULL);
  if (image->blob->type == UndefinedStream)
    return(MagickTrue);
  status=SyncBlob(image);
  switch (image->blob->type)
  {
    case UndefinedStream:
    case StandardStream:
      break;
    case FileStream:
    case PipeStream:
    {
      if (image->blob->synchronize != MagickFalse)
        status=fsync(fileno(image->blob->file_info.file));
      status=ferror(image->blob->file_info.file);
      break;
    }
    case ZipStream:
    {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      (void) gzerror(image->blob->file_info.gzfile,&status);
#endif
      break;
    }
    case BZipStream:
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      (void) BZ2_bzerror(image->blob->file_info.bzfile,&status);
#endif
      break;
    }
    case FifoStream:
      break;
    case BlobStream:
    {
      if ((image->blob->file_info.file != (FILE *) NULL) &&
          (image->blob->synchronize != MagickFalse))
        {
          (void) fsync(fileno(image->blob->file_info.file));
          status=ferror(image->blob->file_info.file);
        }
      break;
    }
    case CustomStream:
      break;
  }
  image->blob->status=status < 0 ? MagickTrue : MagickFalse;
  image->blob->size=GetBlobSize(image);
  image->extent=image->blob->size;
  image->blob->eof=MagickFalse;
  if (image->blob->exempt != MagickFalse)
    {
      image->blob->type=UndefinedStream;
      return(image->blob->status);
    }
  switch (image->blob->type)
  {
    case UndefinedStream:
    case StandardStream:
      break;
    case FileStream:
    {
      status=fclose(image->blob->file_info.file);
      break;
    }
    case PipeStream:
    {
#if defined(MAGICKCORE_HAVE_PCLOSE)
      status=pclose(image->blob->file_info.file);
#endif
      break;
    }
    case ZipStream:
    {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      status=gzclose(image->blob->file_info.gzfile);
#endif
      break;
    }
    case BZipStream:
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      BZ2_bzclose(image->blob->file_info.bzfile);
#endif
      break;
    }
    case FifoStream:
      break;
    case BlobStream:
    {
      if (image->blob->file_info.file != (FILE *) NULL)
        status=fclose(image->blob->file_info.file);
      break;
    }
    case CustomStream:
      break;
  }
  (void) DetachBlob(image->blob);
  image->blob->status=status < 0 ? MagickTrue : MagickFalse;
  return(image->blob->status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y B l o b                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyBlob() deallocates memory associated with a blob.
%
%  The format of the DestroyBlob method is:
%
%      void DestroyBlob(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void DestroyBlob(Image *image)
{
  MagickBooleanType
    destroy;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->signature == MagickCoreSignature);
  destroy=MagickFalse;
  LockSemaphoreInfo(image->blob->semaphore);
  image->blob->reference_count--;
  assert(image->blob->reference_count >= 0);
  if (image->blob->reference_count == 0)
    destroy=MagickTrue;
  UnlockSemaphoreInfo(image->blob->semaphore);
  if (destroy == MagickFalse)
    return;
  (void) CloseBlob(image);
  if (image->blob->mapped != MagickFalse)
    {
      (void) UnmapBlob(image->blob->data,image->blob->length);
      RelinquishMagickResource(MapResource,image->blob->length);
    }
  if (image->blob->semaphore != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&image->blob->semaphore);
  image->blob->signature=(~MagickCoreSignature);
  image->blob=(BlobInfo *) RelinquishMagickMemory(image->blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y C u s t o m S t r e a m I n f o                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCustomStreamInfo() destroys memory associated with the
%  CustomStreamInfo structure.
%
%  The format of the DestroyCustomStreamInfo method is:
%
%      CustomStreamInfo *DestroyCustomStreamInfo(CustomStreamInfo *stream_info)
%
%  A description of each parameter follows:
%
%    o custom_stream: the custom stream info.
%
*/
MagickExport CustomStreamInfo *DestroyCustomStreamInfo(
  CustomStreamInfo *custom_stream)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(custom_stream != (CustomStreamInfo *) NULL);
  assert(custom_stream->signature == MagickCoreSignature);
  custom_stream->signature=(~MagickCoreSignature);
  custom_stream=(CustomStreamInfo *) RelinquishMagickMemory(custom_stream);
  return(custom_stream);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e t a c h B l o b                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DetachBlob() detaches a blob from the BlobInfo structure.
%
%  The format of the DetachBlob method is:
%
%      void *DetachBlob(BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: Specifies a pointer to a BlobInfo structure.
%
*/
MagickExport void *DetachBlob(BlobInfo *blob_info)
{
  void
    *data;

  assert(blob_info != (BlobInfo *) NULL);
  if (blob_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (blob_info->mapped != MagickFalse)
    {
      (void) UnmapBlob(blob_info->data,blob_info->length);
      blob_info->data=(unsigned char *) NULL;
      RelinquishMagickResource(MapResource,blob_info->length);
    }
  blob_info->mapped=MagickFalse;
  blob_info->length=0;
  blob_info->offset=0;
  blob_info->eof=MagickFalse;
  blob_info->exempt=MagickFalse;
  blob_info->type=UndefinedStream;
  blob_info->file_info.file=(FILE *) NULL;
  data=blob_info->data;
  blob_info->data=(unsigned char *) NULL;
  blob_info->stream=(StreamHandler) NULL;
  blob_info->custom_stream=(CustomStreamInfo *) NULL;
  return(data);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D i s a s s o c i a t e B l o b                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DisassociateBlob() disassociates the image stream.  It checks if the
%  blob of the specified image is referenced by other images. If the reference
%  count is higher then 1 a new blob is assigned to the specified image.
%
%  The format of the DisassociateBlob method is:
%
%      void DisassociateBlob(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void DisassociateBlob(Image *image)
{
  BlobInfo
    *blob;

  MagickBooleanType
    clone;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->signature == MagickCoreSignature);
  clone=MagickFalse;
  LockSemaphoreInfo(image->blob->semaphore);
  assert(image->blob->reference_count >= 0);
  if (image->blob->reference_count > 1)
    clone=MagickTrue;
  UnlockSemaphoreInfo(image->blob->semaphore);
  if (clone == MagickFalse)
    return;
  blob=CloneBlobInfo(image->blob);
  DestroyBlob(image);
  image->blob=blob;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  D i s c a r d B l o b B y t e s                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DiscardBlobBytes() discards bytes in a blob.
%
%  The format of the DiscardBlobBytes method is:
%
%      MagickBooleanType DiscardBlobBytes(Image *image,
%        const MagickSizeType length)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o length:  the number of bytes to skip.
%
*/
MagickExport MagickBooleanType DiscardBlobBytes(Image *image,
  const MagickSizeType length)
{
  register MagickOffsetType
    i;

  size_t
    quantum;

  ssize_t
    count;

  unsigned char
    buffer[16384];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (length != (MagickSizeType) ((MagickOffsetType) length))
    return(MagickFalse);
  count=0;
  for (i=0; i < (MagickOffsetType) length; i+=count)
  {
    quantum=(size_t) MagickMin(length-i,sizeof(buffer));
    (void) ReadBlobStream(image,quantum,buffer,&count);
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  return(i < (MagickOffsetType) length ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D u p l i c a t e s B l o b                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DuplicateBlob() duplicates a blob descriptor.
%
%  The format of the DuplicateBlob method is:
%
%      void DuplicateBlob(Image *image,const Image *duplicate)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o duplicate: the duplicate image.
%
*/
MagickExport void DuplicateBlob(Image *image,const Image *duplicate)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(duplicate != (Image *) NULL);
  assert(duplicate->signature == MagickCoreSignature);
  DestroyBlob(image);
  image->blob=ReferenceBlob(duplicate->blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  E O F B l o b                                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EOFBlob() returns a non-zero value when EOF has been detected reading from
%  a blob or file.
%
%  The format of the EOFBlob method is:
%
%      int EOFBlob(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport int EOFBlob(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->type != UndefinedStream);
  switch (image->blob->type)
  {
    case UndefinedStream:
    case StandardStream:
      break;
    case FileStream:
    case PipeStream:
    {
      image->blob->eof=feof(image->blob->file_info.file) != 0 ? MagickTrue :
        MagickFalse;
      break;
    }
    case ZipStream:
    {
      image->blob->eof=MagickFalse;
      break;
    }
    case BZipStream:
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      int
        status;

      status=0;
      (void) BZ2_bzerror(image->blob->file_info.bzfile,&status);
      image->blob->eof=status == BZ_UNEXPECTED_EOF ? MagickTrue : MagickFalse;
#endif
      break;
    }
    case FifoStream:
    {
      image->blob->eof=MagickFalse;
      break;
    }
    case BlobStream:
      break;
    case CustomStream:
      break;
  }
  return((int) image->blob->eof);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i l e T o B l o b                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FileToBlob() returns the contents of a file as a buffer terminated with
%  the '\0' character.  The length of the buffer (not including the extra
%  terminating '\0' character) is returned via the 'length' parameter.  Free
%  the buffer with RelinquishMagickMemory().
%
%  The format of the FileToBlob method is:
%
%      void *FileToBlob(const char *filename,const size_t extent,
%        size_t *length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o blob:  FileToBlob() returns the contents of a file as a blob.  If
%      an error occurs NULL is returned.
%
%    o filename: the filename.
%
%    o extent:  The maximum length of the blob.
%
%    o length: On return, this reflects the actual length of the blob.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void *FileToBlob(const char *filename,const size_t extent,
  size_t *length,ExceptionInfo *exception)
{
  int
    file;

  MagickOffsetType
    offset;

  register size_t
    i;

  ssize_t
    count;

  unsigned char
    *blob;

  void
    *map;

  assert(filename != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  assert(exception != (ExceptionInfo *) NULL);
  *length=0;
  file=fileno(stdin);
  if (LocaleCompare(filename,"-") != 0)
    file=open_utf8(filename,O_RDONLY | O_BINARY,0);
  if (file == -1)
    {
      ThrowFileException(exception,BlobError,"UnableToOpenFile",filename);
      return(NULL);
    }
  offset=(MagickOffsetType) lseek(file,0,SEEK_END);
  count=0;
  if ((file == fileno(stdin)) || (offset < 0) ||
      (offset != (MagickOffsetType) ((ssize_t) offset)))
    {
      size_t
        quantum;

      struct stat
        file_stats;

      /*
        Stream is not seekable.
      */
      offset=(MagickOffsetType) lseek(file,0,SEEK_SET);
      quantum=(size_t) MagickMaxBufferExtent;
      if ((fstat(file,&file_stats) == 0) && (file_stats.st_size > 0))
        quantum=(size_t) MagickMin(file_stats.st_size,MagickMaxBufferExtent);
      blob=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*blob));
      for (i=0; blob != (unsigned char *) NULL; i+=count)
      {
        count=read(file,blob+i,quantum);
        if (count <= 0)
          {
            count=0;
            if (errno != EINTR)
              break;
          }
        if (~((size_t) i) < (quantum+1))
          {
            blob=(unsigned char *) RelinquishMagickMemory(blob);
            break;
          }
        blob=(unsigned char *) ResizeQuantumMemory(blob,i+quantum+1,
          sizeof(*blob));
        if ((size_t) (i+count) >= extent)
          break;
      }
      if (LocaleCompare(filename,"-") != 0)
        file=close(file);
      if (blob == (unsigned char *) NULL)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",filename);
          return(NULL);
        }
      if (file == -1)
        {
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          ThrowFileException(exception,BlobError,"UnableToReadBlob",filename);
          return(NULL);
        }
      *length=(size_t) MagickMin(i+count,extent);
      blob[*length]='\0';
      return(blob);
    }
  *length=(size_t) MagickMin(offset,(MagickOffsetType)
    MagickMin(extent,SSIZE_MAX));
  blob=(unsigned char *) NULL;
  if (~(*length) >= (MagickPathExtent-1))
    blob=(unsigned char *) AcquireQuantumMemory(*length+MagickPathExtent,
      sizeof(*blob));
  if (blob == (unsigned char *) NULL)
    {
      file=close(file);
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",filename);
      return(NULL);
    }
  map=MapBlob(file,ReadMode,0,*length);
  if (map != (unsigned char *) NULL)
    {
      (void) memcpy(blob,map,*length);
      (void) UnmapBlob(map,*length);
    }
  else
    {
      (void) lseek(file,0,SEEK_SET);
      for (i=0; i < *length; i+=count)
      {
        count=read(file,blob+i,(size_t) MagickMin(*length-i,SSIZE_MAX));
        if (count <= 0)
          {
            count=0;
            if (errno != EINTR)
              break;
          }
      }
      if (i < *length)
        {
          file=close(file)-1;
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          ThrowFileException(exception,BlobError,"UnableToReadBlob",filename);
          return(NULL);
        }
    }
  blob[*length]='\0';
  if (LocaleCompare(filename,"-") != 0)
    file=close(file);
  if (file == -1)
    {
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      ThrowFileException(exception,BlobError,"UnableToReadBlob",filename);
    }
  return(blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i l e T o I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FileToImage() write the contents of a file to an image.
%
%  The format of the FileToImage method is:
%
%      MagickBooleanType FileToImage(Image *,const char *filename)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o filename: the filename.
%
*/

static inline ssize_t WriteBlobStream(Image *image,const size_t length,
  const void *data)
{
  MagickSizeType
    extent;

  register unsigned char
    *q;

  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->type != UndefinedStream);
  assert(data != NULL);
  if (image->blob->type != BlobStream)
    return(WriteBlob(image,length,(const unsigned char *) data));
  extent=(MagickSizeType) (image->blob->offset+(MagickOffsetType) length);
  if (extent >= image->blob->extent)
    {
      extent=image->blob->extent+image->blob->quantum+length;
      image->blob->quantum<<=1;
      if (SetBlobExtent(image,extent) == MagickFalse)
        return(0);
    }
  q=image->blob->data+image->blob->offset;
  (void) memcpy(q,data,length);
  image->blob->offset+=length;
  if (image->blob->offset >= (MagickOffsetType) image->blob->length)
    image->blob->length=(size_t) image->blob->offset;
  return((ssize_t) length);
}

MagickExport MagickBooleanType FileToImage(Image *image,const char *filename,
  ExceptionInfo *exception)
{
  int
    file;

  size_t
    length,
    quantum;

  ssize_t
    count;

  struct stat
    file_stats;

  unsigned char
    *blob;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(filename != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  file=fileno(stdin);
  if (LocaleCompare(filename,"-") != 0)
    file=open_utf8(filename,O_RDONLY | O_BINARY,0);
  if (file == -1)
    {
      ThrowFileException(exception,BlobError,"UnableToOpenBlob",filename);
      return(MagickFalse);
    }
  quantum=(size_t) MagickMaxBufferExtent;
  if ((fstat(file,&file_stats) == 0) && (file_stats.st_size > 0))
    quantum=(size_t) MagickMin(file_stats.st_size,MagickMaxBufferExtent);
  blob=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*blob));
  if (blob == (unsigned char *) NULL)
    {
      file=close(file);
      ThrowFileException(exception,ResourceLimitError,"MemoryAllocationFailed",
        filename);
      return(MagickFalse);
    }
  for ( ; ; )
  {
    count=read(file,blob,quantum);
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
    length=(size_t) count;
    count=WriteBlobStream(image,length,blob);
    if (count != (ssize_t) length)
      {
        ThrowFileException(exception,BlobError,"UnableToWriteBlob",filename);
        break;
      }
  }
  file=close(file);
  if (file == -1)
    ThrowFileException(exception,BlobError,"UnableToWriteBlob",filename);
  blob=(unsigned char *) RelinquishMagickMemory(blob);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t B l o b E r r o r                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobError() returns MagickTrue if the blob associated with the specified
%  image encountered an error.
%
%  The format of the GetBlobError method is:
%
%       MagickBooleanType GetBlobError(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType GetBlobError(const Image *image)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(image->blob->status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t B l o b F i l e H a n d l e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobFileHandle() returns the file handle associated with the image blob.
%
%  The format of the GetBlobFile method is:
%
%      FILE *GetBlobFileHandle(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport FILE *GetBlobFileHandle(const Image *image)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  return(image->blob->file_info.file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t B l o b I n f o                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobInfo() initializes the BlobInfo structure.
%
%  The format of the GetBlobInfo method is:
%
%      void GetBlobInfo(BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: Specifies a pointer to a BlobInfo structure.
%
*/
MagickExport void GetBlobInfo(BlobInfo *blob_info)
{
  assert(blob_info != (BlobInfo *) NULL);
  (void) ResetMagickMemory(blob_info,0,sizeof(*blob_info));
  blob_info->type=UndefinedStream;
  blob_info->quantum=(size_t) MagickMaxBlobExtent;
  blob_info->properties.st_mtime=time((time_t *) NULL);
  blob_info->properties.st_ctime=time((time_t *) NULL);
  blob_info->debug=IsEventLogging();
  blob_info->reference_count=1;
  blob_info->semaphore=AcquireSemaphoreInfo();
  blob_info->signature=MagickCoreSignature;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t B l o b P r o p e r t i e s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobProperties() returns information about an image blob.
%
%  The format of the GetBlobProperties method is:
%
%      const struct stat *GetBlobProperties(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport const struct stat *GetBlobProperties(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(&image->blob->properties);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  G e t B l o b S i z e                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobSize() returns the current length of the image file or blob; zero is
%  returned if the size cannot be determined.
%
%  The format of the GetBlobSize method is:
%
%      MagickSizeType GetBlobSize(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickSizeType GetBlobSize(const Image *image)
{
  MagickSizeType
    extent;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->blob != (BlobInfo *) NULL);
  extent=0;
  switch (image->blob->type)
  {
    case UndefinedStream:
    case StandardStream:
    {
      extent=image->blob->size;
      break;
    }
    case FileStream:
    {
      if (fstat(fileno(image->blob->file_info.file),&image->blob->properties) == 0)
        extent=(MagickSizeType) image->blob->properties.st_size;
      break;
    }
    case PipeStream:
    {
      extent=image->blob->size;
      break;
    }
    case ZipStream:
    case BZipStream:
    {
      MagickBooleanType
        status;

      status=GetPathAttributes(image->filename,&image->blob->properties);
      if (status != MagickFalse)
        extent=(MagickSizeType) image->blob->properties.st_size;
      break;
    }
    case FifoStream:
      break;
    case BlobStream:
    {
      extent=(MagickSizeType) image->blob->length;
      break;
    }
    case CustomStream:
    {
      if ((image->blob->custom_stream->teller != (CustomStreamTeller) NULL) &&
          (image->blob->custom_stream->seeker != (CustomStreamSeeker) NULL))
        {
          MagickOffsetType
            offset;

          offset=image->blob->custom_stream->teller(
            image->blob->custom_stream->data);
          extent=(MagickSizeType) image->blob->custom_stream->seeker(0,SEEK_END,
            image->blob->custom_stream->data);
          (void) image->blob->custom_stream->seeker(offset,SEEK_SET,
            image->blob->custom_stream->data);
        }
      break;
    }
  }
  return(extent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t B l o b S t r e a m D a t a                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobStreamData() returns the stream data for the image.
%
%  The format of the GetBlobStreamData method is:
%
%      void *GetBlobStreamData(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void *GetBlobStreamData(const Image *image)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  return(image->blob->data);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t B l o b S t r e a m H a n d l e r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobStreamHandler() returns the stream handler for the image.
%
%  The format of the GetBlobStreamHandler method is:
%
%      StreamHandler GetBlobStreamHandler(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport StreamHandler GetBlobStreamHandler(const Image *image)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(image->blob->stream);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m a g e T o B l o b                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImageToBlob() implements direct to memory image formats.  It returns the
%  image as a formatted blob and its length.  The magick member of the Image
%  structure determines the format of the returned blob (GIF, JPEG, PNG,
%  etc.).  This method is the equivalent of WriteImage(), but writes the
%  formatted "file" to a memory buffer rather than to an actual file.
%
%  The format of the ImageToBlob method is:
%
%      void *ImageToBlob(const ImageInfo *image_info,Image *image,
%        size_t *length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o length: return the actual length of the blob.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void *ImageToBlob(const ImageInfo *image_info,
  Image *image,size_t *length,ExceptionInfo *exception)
{
  const MagickInfo
    *magick_info;

  ImageInfo
    *blob_info;

  MagickBooleanType
    status;

  void
    *blob;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  *length=0;
  blob=(unsigned char *) NULL;
  blob_info=CloneImageInfo(image_info);
  blob_info->adjoin=MagickFalse;
  (void) SetImageInfo(blob_info,1,exception);
  if (*blob_info->magick != '\0')
    (void) CopyMagickString(image->magick,blob_info->magick,MagickPathExtent);
  magick_info=GetMagickInfo(image->magick,exception);
  if (magick_info == (const MagickInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        MissingDelegateError,"NoDecodeDelegateForThisImageFormat","`%s'",
        image->magick);
      blob_info=DestroyImageInfo(blob_info);
      return(blob);
    }
  (void) CopyMagickString(blob_info->magick,image->magick,MagickPathExtent);
  if (GetMagickBlobSupport(magick_info) != MagickFalse)
    {
      /*
        Native blob support for this image format.
      */
      blob_info->length=0;
      blob_info->blob=AcquireQuantumMemory(MagickMaxBlobExtent,
        sizeof(unsigned char));
      if (blob_info->blob == NULL)
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      else
        {
          (void) CloseBlob(image);
          image->blob->exempt=MagickTrue;
          *image->filename='\0';
          status=WriteImage(blob_info,image,exception);
          *length=image->blob->length;
          blob=DetachBlob(image->blob);
          if (status == MagickFalse)
            blob=RelinquishMagickMemory(blob);
          else
            blob=ResizeQuantumMemory(blob,*length+1,sizeof(unsigned char));
        }
    }
  else
    {
      char
        unique[MagickPathExtent];

      int
        file;

      /*
        Write file to disk in blob image format.
      */
      file=AcquireUniqueFileResource(unique);
      if (file == -1)
        {
          ThrowFileException(exception,BlobError,"UnableToWriteBlob",
            image_info->filename);
        }
      else
        {
          blob_info->file=fdopen(file,"wb");
          if (blob_info->file != (FILE *) NULL)
            {
              (void) FormatLocaleString(image->filename,MagickPathExtent,
                "%s:%s",image->magick,unique);
              status=WriteImage(blob_info,image,exception);
              (void) CloseBlob(image);
              (void) fclose(blob_info->file);
              if (status != MagickFalse)
                blob=FileToBlob(unique,~0UL,length,exception);
            }
          (void) RelinquishUniqueFileResource(unique);
        }
    }
  blob_info=DestroyImageInfo(blob_info);
  return(blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  I m a g e T o C u s t o m S t r e a m                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImageToCustomStream() is the equivalent of WriteImage(), but writes the
%  formatted "file" to the custom stream rather than to an actual file.
%
%  The format of the ImageToCustomStream method is:
%
%      void ImageToCustomStream(const ImageInfo *image_info,Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void ImageToCustomStream(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  const MagickInfo
    *magick_info;

  ImageInfo
    *blob_info;

  MagickBooleanType
    status;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image_info->custom_stream != (CustomStreamInfo *) NULL);
  assert(image_info->custom_stream->signature == MagickCoreSignature);
  assert(image_info->custom_stream->writer != (CustomStreamHandler) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  blob_info=CloneImageInfo(image_info);
  blob_info->adjoin=MagickFalse;
  (void) SetImageInfo(blob_info,1,exception);
  if (*blob_info->magick != '\0')
    (void) CopyMagickString(image->magick,blob_info->magick,MagickPathExtent);
  magick_info=GetMagickInfo(image->magick,exception);
  if (magick_info == (const MagickInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        MissingDelegateError,"NoEncodeDelegateForThisImageFormat","`%s'",
        image->magick);
      blob_info=DestroyImageInfo(blob_info);
      return;
    }
  (void) CopyMagickString(blob_info->magick,image->magick,MagickPathExtent);
  if (GetMagickBlobSupport(magick_info) != MagickFalse)
    {
      /*
        Native blob support for this image format.
      */
      (void) CloseBlob(image);
      *image->filename='\0';
      (void) WriteImage(blob_info,image,exception);
      (void) CloseBlob(image);
    }
  else
    {
      char
        unique[MagickPathExtent];

      int
        file;

      unsigned char
        *blob;

      /*
        Write file to disk in blob image format.
      */
      blob_info->custom_stream=(CustomStreamInfo *) NULL;
      blob=(unsigned char *) AcquireQuantumMemory(MagickMaxBufferExtent,
        sizeof(*blob));
      if (blob == (unsigned char *) NULL)
        {
          ThrowFileException(exception,BlobError,"UnableToWriteBlob",
            image_info->filename);
          blob_info=DestroyImageInfo(blob_info);
          return;
        }
      file=AcquireUniqueFileResource(unique);
      if (file == -1)
        {
          ThrowFileException(exception,BlobError,"UnableToWriteBlob",
            image_info->filename);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          blob_info=DestroyImageInfo(blob_info);
          return;
        }
      blob_info->file=fdopen(file,"wb+");
      if (blob_info->file != (FILE *) NULL)
        {
          ssize_t
            count;

          (void) FormatLocaleString(image->filename,MagickPathExtent,
            "%s:%s",image->magick,unique);
          status=WriteImage(blob_info,image,exception);
          (void) CloseBlob(image);
          if (status != MagickFalse)
            {
              (void) fseek(blob_info->file,0,SEEK_SET);
              count=(ssize_t) MagickMaxBufferExtent;
              while (count == (ssize_t) MagickMaxBufferExtent)
              {
                count=(ssize_t) fread(blob,sizeof(*blob),MagickMaxBufferExtent,
                  blob_info->file);
                (void) image_info->custom_stream->writer(blob,(size_t) count,
                  image_info->custom_stream->data);
              }
            }
          (void) fclose(blob_info->file);
        }
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      (void) RelinquishUniqueFileResource(unique);
    }
  blob_info=DestroyImageInfo(blob_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m a g e T o F i l e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImageToFile() writes an image to a file.  It returns MagickFalse if an error
%  occurs otherwise MagickTrue.
%
%  The format of the ImageToFile method is:
%
%       MagickBooleanType ImageToFile(Image *image,char *filename,
%         ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o filename: Write the image to this file.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ImageToFile(Image *image,char *filename,
  ExceptionInfo *exception)
{
  int
    file;

  register const unsigned char
    *p;

  register size_t
    i;

  size_t
    length,
    quantum;

  ssize_t
    count;

  struct stat
    file_stats;

  unsigned char
    *buffer;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->type != UndefinedStream);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",filename);
  assert(filename != (const char *) NULL);
  if (*filename == '\0')
    file=AcquireUniqueFileResource(filename);
  else
    if (LocaleCompare(filename,"-") == 0)
      file=fileno(stdout);
    else
      file=open_utf8(filename,O_RDWR | O_CREAT | O_EXCL | O_BINARY,S_MODE);
  if (file == -1)
    {
      ThrowFileException(exception,BlobError,"UnableToWriteBlob",filename);
      return(MagickFalse);
    }
  quantum=(size_t) MagickMaxBufferExtent;
  if ((fstat(file,&file_stats) == 0) && (file_stats.st_size > 0))
    quantum=(size_t) MagickMin(file_stats.st_size,MagickMaxBufferExtent);
  buffer=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    {
      file=close(file)-1;
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationError","`%s'",filename);
      return(MagickFalse);
    }
  length=0;
  p=(const unsigned char *) ReadBlobStream(image,quantum,buffer,&count);
  for (i=0; count > 0; )
  {
    length=(size_t) count;
    for (i=0; i < length; i+=count)
    {
      count=write(file,p+i,(size_t) (length-i));
      if (count <= 0)
        {
          count=0;
          if (errno != EINTR)
            break;
        }
    }
    if (i < length)
      break;
    p=(const unsigned char *) ReadBlobStream(image,quantum,buffer,&count);
  }
  if (LocaleCompare(filename,"-") != 0)
    file=close(file);
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  if ((file == -1) || (i < length))
    {
      if (file != -1)
        file=close(file);
      ThrowFileException(exception,BlobError,"UnableToWriteBlob",filename);
      return(MagickFalse);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m a g e s T o B l o b                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImagesToBlob() implements direct to memory image formats.  It returns the
%  image sequence as a blob and its length.  The magick member of the ImageInfo
%  structure determines the format of the returned blob (GIF, JPEG,  PNG, etc.)
%
%  Note, some image formats do not permit multiple images to the same image
%  stream (e.g. JPEG).  in this instance, just the first image of the
%  sequence is returned as a blob.
%
%  The format of the ImagesToBlob method is:
%
%      void *ImagesToBlob(const ImageInfo *image_info,Image *images,
%        size_t *length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o images: the image list.
%
%    o length: return the actual length of the blob.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void *ImagesToBlob(const ImageInfo *image_info,Image *images,
  size_t *length,ExceptionInfo *exception)
{
  const MagickInfo
    *magick_info;

  ImageInfo
    *blob_info;

  MagickBooleanType
    status;

  void
    *blob;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  *length=0;
  blob=(unsigned char *) NULL;
  blob_info=CloneImageInfo(image_info);
  (void) SetImageInfo(blob_info,(unsigned int) GetImageListLength(images),
    exception);
  if (*blob_info->magick != '\0')
    (void) CopyMagickString(images->magick,blob_info->magick,MagickPathExtent);
  magick_info=GetMagickInfo(images->magick,exception);
  if (magick_info == (const MagickInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        MissingDelegateError,"NoDecodeDelegateForThisImageFormat","`%s'",
        images->magick);
      blob_info=DestroyImageInfo(blob_info);
      return(blob);
    }
  if (GetMagickAdjoin(magick_info) == MagickFalse)
    {
      blob_info=DestroyImageInfo(blob_info);
      return(ImageToBlob(image_info,images,length,exception));
    }
  (void) CopyMagickString(blob_info->magick,images->magick,MagickPathExtent);
  if (GetMagickBlobSupport(magick_info) != MagickFalse)
    {
      /*
        Native blob support for this images format.
      */
      blob_info->length=0;
      blob_info->blob=(void *) AcquireQuantumMemory(MagickMaxBlobExtent,
        sizeof(unsigned char));
      if (blob_info->blob == (void *) NULL)
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",images->filename);
      else
        {
          (void) CloseBlob(images);
          images->blob->exempt=MagickTrue;
          *images->filename='\0';
          status=WriteImages(blob_info,images,images->filename,exception);
          *length=images->blob->length;
          blob=DetachBlob(images->blob);
          if (status == MagickFalse)
            blob=RelinquishMagickMemory(blob);
          else
            blob=ResizeQuantumMemory(blob,*length+1,sizeof(unsigned char));
        }
    }
  else
    {
      char
        filename[MagickPathExtent],
        unique[MagickPathExtent];

      int
        file;

      /*
        Write file to disk in blob images format.
      */
      file=AcquireUniqueFileResource(unique);
      if (file == -1)
        {
          ThrowFileException(exception,FileOpenError,"UnableToWriteBlob",
            image_info->filename);
        }
      else
        {
          blob_info->file=fdopen(file,"wb");
          if (blob_info->file != (FILE *) NULL)
            {
              (void) FormatLocaleString(filename,MagickPathExtent,"%s:%s",
                images->magick,unique);
              status=WriteImages(blob_info,images,filename,exception);
              (void) CloseBlob(images);
              (void) fclose(blob_info->file);
              if (status != MagickFalse)
                blob=FileToBlob(unique,~0UL,length,exception);
            }
          (void) RelinquishUniqueFileResource(unique);
        }
    }
  blob_info=DestroyImageInfo(blob_info);
  return(blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  I m a g e s T o C u s t o m B l o b                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImagesToCustomStream() is the equivalent of WriteImages(), but writes the
%  formatted "file" to the custom stream rather than to an actual file.
%
%  The format of the ImageToCustomStream method is:
%
%      void ImagesToCustomStream(const ImageInfo *image_info,Image *images,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o images: the image list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void ImagesToCustomStream(const ImageInfo *image_info,
  Image *images,ExceptionInfo *exception)
{
  const MagickInfo
    *magick_info;

  ImageInfo
    *blob_info;

  MagickBooleanType
    status;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  assert(image_info->custom_stream != (CustomStreamInfo *) NULL);
  assert(image_info->custom_stream->signature == MagickCoreSignature);
  assert(image_info->custom_stream->reader != (CustomStreamHandler) NULL);
  assert(image_info->custom_stream->writer != (CustomStreamHandler) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  blob_info=CloneImageInfo(image_info);
  (void) SetImageInfo(blob_info,(unsigned int) GetImageListLength(images),
    exception);
  if (*blob_info->magick != '\0')
    (void) CopyMagickString(images->magick,blob_info->magick,MagickPathExtent);
  magick_info=GetMagickInfo(images->magick,exception);
  if (magick_info == (const MagickInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        MissingDelegateError,"NoEncodeDelegateForThisImageFormat","`%s'",
        images->magick);
      blob_info=DestroyImageInfo(blob_info);
      return;
    }
  (void) CopyMagickString(blob_info->magick,images->magick,MagickPathExtent);
  if (GetMagickBlobSupport(magick_info) != MagickFalse)
    {
      /*
        Native blob support for this image format.
      */
      (void) CloseBlob(images);
      *images->filename='\0';
      (void) WriteImages(blob_info,images,images->filename,exception);
      (void) CloseBlob(images);
    }
  else
    {
      char
        filename[MagickPathExtent],
        unique[MagickPathExtent];

      int
        file;

      unsigned char
        *blob;

      /*
        Write file to disk in blob image format.
      */
      blob_info->custom_stream=(CustomStreamInfo *) NULL;
      blob=(unsigned char *) AcquireQuantumMemory(MagickMaxBufferExtent,
        sizeof(*blob));
      if (blob == (unsigned char *) NULL)
        {
          ThrowFileException(exception,BlobError,"UnableToWriteBlob",
            image_info->filename);
          blob_info=DestroyImageInfo(blob_info);
          return;
        }
      file=AcquireUniqueFileResource(unique);
      if (file == -1)
        {
          ThrowFileException(exception,BlobError,"UnableToWriteBlob",
            image_info->filename);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          blob_info=DestroyImageInfo(blob_info);
          return;
        }
      blob_info->file=fdopen(file,"wb+");
      if (blob_info->file != (FILE *) NULL)
        {
          ssize_t
            count;

          (void) FormatLocaleString(filename,MagickPathExtent,"%s:%s",
            images->magick,unique);
          status=WriteImages(blob_info,images,filename,exception);
          (void) CloseBlob(images);
          if (status != MagickFalse)
            {
              (void) fseek(blob_info->file,0,SEEK_SET);
              count=(ssize_t) MagickMaxBufferExtent;
              while (count == (ssize_t) MagickMaxBufferExtent)
              {
                count=(ssize_t) fread(blob,sizeof(*blob),MagickMaxBufferExtent,
                  blob_info->file);
                (void) image_info->custom_stream->writer(blob,(size_t) count,
                  image_info->custom_stream->data);
              }
            }
          (void) fclose(blob_info->file);
        }
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      (void) RelinquishUniqueFileResource(unique);
    }
  blob_info=DestroyImageInfo(blob_info);
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n j e c t I m a g e B l o b                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InjectImageBlob() injects the image with a copy of itself in the specified
%  format (e.g. inject JPEG into a PDF image).
%
%  The format of the InjectImageBlob method is:
%
%      MagickBooleanType InjectImageBlob(const ImageInfo *image_info,
%        Image *image,Image *inject_image,const char *format,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info..
%
%    o image: the image.
%
%    o inject_image: inject into the image stream.
%
%    o format: the image format.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType InjectImageBlob(const ImageInfo *image_info,
  Image *image,Image *inject_image,const char *format,ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent];

  FILE
    *unique_file;

  Image
    *byte_image;

  ImageInfo
    *write_info;

  int
    file;

  MagickBooleanType
    status;

  register ssize_t
    i;

  size_t
    quantum;

  ssize_t
    count;

  struct stat
    file_stats;

  unsigned char
    *buffer;

  /*
    Write inject image to a temporary file.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(inject_image != (Image *) NULL);
  assert(inject_image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  unique_file=(FILE *) NULL;
  file=AcquireUniqueFileResource(filename);
  if (file != -1)
    unique_file=fdopen(file,"wb");
  if ((file == -1) || (unique_file == (FILE *) NULL))
    {
      (void) CopyMagickString(image->filename,filename,MagickPathExtent);
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        image->filename);
      return(MagickFalse);
    }
  byte_image=CloneImage(inject_image,0,0,MagickFalse,exception);
  if (byte_image == (Image *) NULL)
    {
      (void) fclose(unique_file);
      (void) RelinquishUniqueFileResource(filename);
      return(MagickFalse);
    }
  (void) FormatLocaleString(byte_image->filename,MagickPathExtent,"%s:%s",
    format,filename);
  DestroyBlob(byte_image);
  byte_image->blob=CloneBlobInfo((BlobInfo *) NULL);
  write_info=CloneImageInfo(image_info);
  SetImageInfoFile(write_info,unique_file);
  status=WriteImage(write_info,byte_image,exception);
  write_info=DestroyImageInfo(write_info);
  byte_image=DestroyImage(byte_image);
  (void) fclose(unique_file);
  if (status == MagickFalse)
    {
      (void) RelinquishUniqueFileResource(filename);
      return(MagickFalse);
    }
  /*
    Inject into image stream.
  */
  file=open_utf8(filename,O_RDONLY | O_BINARY,0);
  if (file == -1)
    {
      (void) RelinquishUniqueFileResource(filename);
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image_info->filename);
      return(MagickFalse);
    }
  quantum=(size_t) MagickMaxBufferExtent;
  if ((fstat(file,&file_stats) == 0) && (file_stats.st_size > 0))
    quantum=(size_t) MagickMin(file_stats.st_size,MagickMaxBufferExtent);
  buffer=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    {
      (void) RelinquishUniqueFileResource(filename);
      file=close(file);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  for (i=0; ; i+=count)
  {
    count=read(file,buffer,quantum);
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
    status=WriteBlobStream(image,(size_t) count,buffer) == count ? MagickTrue :
      MagickFalse;
  }
  file=close(file);
  if (file == -1)
    ThrowFileException(exception,FileOpenError,"UnableToWriteBlob",filename);
  (void) RelinquishUniqueFileResource(filename);
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s B l o b E x e m p t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsBlobExempt() returns true if the blob is exempt.
%
%  The format of the IsBlobExempt method is:
%
%       MagickBooleanType IsBlobExempt(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsBlobExempt(const Image *image)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(image->blob->exempt);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s B l o b S e e k a b l e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsBlobSeekable() returns true if the blob is seekable.
%
%  The format of the IsBlobSeekable method is:
%
%       MagickBooleanType IsBlobSeekable(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsBlobSeekable(const Image *image)
{
  MagickBooleanType
    seekable;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  switch (image->blob->type)
  {
    case FileStream:
    case BlobStream:
    case ZipStream:
    {
      seekable=MagickTrue;
      break;
    }
    case UndefinedStream:
    case StandardStream:
    case BZipStream:
    case FifoStream:
    case PipeStream:
    default:
    {
      seekable=MagickFalse;
      break;
    }
    case CustomStream:
    {
      if ((image->blob->custom_stream->seeker != (CustomStreamSeeker) NULL) &&
          (image->blob->custom_stream->teller != (CustomStreamTeller) NULL))
        seekable=MagickTrue;
      else
        seekable=MagickFalse;
      break;
    }
  }
  return(seekable);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s B l o b T e m p o r a r y                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsBlobTemporary() returns true if the blob is temporary.
%
%  The format of the IsBlobTemporary method is:
%
%       MagickBooleanType IsBlobTemporary(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsBlobTemporary(const Image *image)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(image->blob->temporary);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  M a p B l o b                                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MapBlob() creates a mapping from a file to a binary large object.
%
%  The format of the MapBlob method is:
%
%      void *MapBlob(int file,const MapMode mode,const MagickOffsetType offset,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o file: map this file descriptor.
%
%    o mode: ReadMode, WriteMode, or IOMode.
%
%    o offset: starting at this offset within the file.
%
%    o length: the length of the mapping is returned in this pointer.
%
*/
MagickExport void *MapBlob(int file,const MapMode mode,
  const MagickOffsetType offset,const size_t length)
{
#if defined(MAGICKCORE_HAVE_MMAP)
  int
    flags,
    protection;

  void
    *map;

  /*
    Map file.
  */
  flags=0;
  if (file == -1)
#if defined(MAP_ANONYMOUS)
    flags|=MAP_ANONYMOUS;
#else
    return(NULL);
#endif
  switch (mode)
  {
    case ReadMode:
    default:
    {
      protection=PROT_READ;
      flags|=MAP_PRIVATE;
      break;
    }
    case WriteMode:
    {
      protection=PROT_WRITE;
      flags|=MAP_SHARED;
      break;
    }
    case IOMode:
    {
      protection=PROT_READ | PROT_WRITE;
      flags|=MAP_SHARED;
      break;
    }
  }
#if !defined(MAGICKCORE_HAVE_HUGEPAGES) || !defined(MAP_HUGETLB)
  map=mmap((char *) NULL,length,protection,flags,file,(off_t) offset);
#else
  map=mmap((char *) NULL,length,protection,flags | MAP_HUGETLB,file,(off_t)
    offset);
  if (map == MAP_FAILED)
    map=mmap((char *) NULL,length,protection,flags,file,(off_t) offset);
#endif
  if (map == MAP_FAILED)
    return(NULL);
  return(map);
#else
  (void) file;
  (void) mode;
  (void) offset;
  (void) length;
  return(NULL);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  M S B O r d e r L o n g                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MSBOrderLong() converts a least-significant byte first buffer of integers to
%  most-significant byte first.
%
%  The format of the MSBOrderLong method is:
%
%      void MSBOrderLong(unsigned char *buffer,const size_t length)
%
%  A description of each parameter follows.
%
%   o  buffer:  Specifies a pointer to a buffer of integers.
%
%   o  length:  Specifies the length of the buffer.
%
*/
MagickExport void MSBOrderLong(unsigned char *buffer,const size_t length)
{
  int
    c;

  register unsigned char
    *p,
    *q;

  assert(buffer != (unsigned char *) NULL);
  q=buffer+length;
  while (buffer < q)
  {
    p=buffer+3;
    c=(int) (*p);
    *p=(*buffer);
    *buffer++=(unsigned char) c;
    p=buffer+1;
    c=(int) (*p);
    *p=(*buffer);
    *buffer++=(unsigned char) c;
    buffer+=2;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  M S B O r d e r S h o r t                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MSBOrderShort() converts a least-significant byte first buffer of integers
%  to most-significant byte first.
%
%  The format of the MSBOrderShort method is:
%
%      void MSBOrderShort(unsigned char *p,const size_t length)
%
%  A description of each parameter follows.
%
%   o  p:  Specifies a pointer to a buffer of integers.
%
%   o  length:  Specifies the length of the buffer.
%
*/
MagickExport void MSBOrderShort(unsigned char *p,const size_t length)
{
  int
    c;

  register unsigned char
    *q;

  assert(p != (unsigned char *) NULL);
  q=p+length;
  while (p < q)
  {
    c=(int) (*p);
    *p=(*(p+1));
    p++;
    *p++=(unsigned char) c;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   O p e n B l o b                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenBlob() opens a file associated with the image.  A file name of '-' sets
%  the file to stdin for type 'r' and stdout for type 'w'.  If the filename
%  suffix is '.gz' or '.Z', the image is decompressed for type 'r' and
%  compressed for type 'w'.  If the filename prefix is '|', it is piped to or
%  from a system command.
%
%  The format of the OpenBlob method is:
%
%       MagickBooleanType OpenBlob(const ImageInfo *image_info,Image *image,
%        const BlobMode mode,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o mode: the mode for opening the file.
%
*/

static inline MagickBooleanType SetStreamBuffering(const ImageInfo *image_info,
  Image *image)
{
  const char
    *option;

  int
    status;

  size_t
    size;

  size=16384;
  option=GetImageOption(image_info,"stream:buffer-size");
  if (option != (const char *) NULL)
    size=StringToUnsignedLong(option);
  status=setvbuf(image->blob->file_info.file,(char *) NULL,size == 0 ?
    _IONBF : _IOFBF,size);
  return(status == 0 ? MagickTrue : MagickFalse);
}

MagickExport MagickBooleanType OpenBlob(const ImageInfo *image_info,
  Image *image,const BlobMode mode,ExceptionInfo *exception)
{
  char
    extension[MagickPathExtent],
    filename[MagickPathExtent];

  const char
    *type;

  MagickBooleanType
    status;

  PolicyRights
    rights;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image_info->blob != (void *) NULL)
    {
      if (image_info->stream != (StreamHandler) NULL)
        image->blob->stream=(StreamHandler) image_info->stream;
      AttachBlob(image->blob,image_info->blob,image_info->length);
      return(MagickTrue);
    }
  if ((image_info->custom_stream != (CustomStreamInfo *) NULL) &&
      (*image->filename == '\0'))
    {
      image->blob->type=CustomStream;
      image->blob->custom_stream=image_info->custom_stream;
      return(MagickTrue);
    }
  (void) DetachBlob(image->blob);
  switch (mode)
  {
    default: type="r"; break;
    case ReadBlobMode: type="r"; break;
    case ReadBinaryBlobMode: type="rb"; break;
    case WriteBlobMode: type="w"; break;
    case WriteBinaryBlobMode: type="w+b"; break;
    case AppendBlobMode: type="a"; break;
    case AppendBinaryBlobMode: type="a+b"; break;
  }
  if (*type != 'r')
    image->blob->synchronize=image_info->synchronize;
  if (image_info->stream != (StreamHandler) NULL)
    {
      image->blob->stream=image_info->stream;
      if (*type == 'w')
        {
          image->blob->type=FifoStream;
          return(MagickTrue);
        }
    }
  /*
    Open image file.
  */
  *filename='\0';
  (void) CopyMagickString(filename,image->filename,MagickPathExtent);
  rights=ReadPolicyRights;
  if (*type == 'w')
    rights=WritePolicyRights;
  if (IsRightsAuthorized(PathPolicyDomain,rights,filename) == MagickFalse)
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",filename);
      return(MagickFalse);
    }
  if ((LocaleCompare(filename,"-") == 0) ||
      ((*filename == '\0') && (image_info->file == (FILE *) NULL)))
    {
      image->blob->file_info.file=(*type == 'r') ? stdin : stdout;
#if defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__OS2__)
      if (strchr(type,'b') != (char *) NULL)
        setmode(fileno(image->blob->file_info.file),_O_BINARY);
#endif
      image->blob->type=StandardStream;
      image->blob->exempt=MagickTrue;
      return(SetStreamBuffering(image_info,image));
    }
  if (LocaleNCompare(filename,"fd:",3) == 0)
    {
      char
        fileMode[MagickPathExtent];

      *fileMode =(*type);
      fileMode[1]='\0';
      image->blob->file_info.file=fdopen(StringToLong(filename+3),fileMode);
#if defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__OS2__)
      if (strchr(type,'b') != (char *) NULL)
        setmode(fileno(image->blob->file_info.file),_O_BINARY);
#endif
      image->blob->type=StandardStream;
      image->blob->exempt=MagickTrue;
      return(SetStreamBuffering(image_info,image));
    }
#if defined(MAGICKCORE_HAVE_POPEN) && defined(MAGICKCORE_PIPES_SUPPORT)
  if (*filename == '|')
    {
      char
        fileMode[MagickPathExtent],
        *sanitize_command;

      /*
        Pipe image to or from a system command.
      */
#if defined(SIGPIPE)
      if (*type == 'w')
        (void) signal(SIGPIPE,SIG_IGN);
#endif
      *fileMode =(*type);
      fileMode[1]='\0';
      sanitize_command=SanitizeString(filename+1);
      image->blob->file_info.file=(FILE *) popen_utf8(sanitize_command,
        fileMode);
      sanitize_command=DestroyString(sanitize_command);
      if (image->blob->file_info.file == (FILE *) NULL)
        {
          ThrowFileException(exception,BlobError,"UnableToOpenBlob",filename);
          return(MagickFalse);
        }
      image->blob->type=PipeStream;
      image->blob->exempt=MagickTrue;
      return(SetStreamBuffering(image_info,image));
    }
#endif
  status=GetPathAttributes(filename,&image->blob->properties);
#if defined(S_ISFIFO)
  if ((status != MagickFalse) && S_ISFIFO(image->blob->properties.st_mode))
    {
      image->blob->file_info.file=(FILE *) fopen_utf8(filename,type);
      if (image->blob->file_info.file == (FILE *) NULL)
        {
          ThrowFileException(exception,BlobError,"UnableToOpenBlob",filename);
          return(MagickFalse);
        }
      image->blob->type=FileStream;
      image->blob->exempt=MagickTrue;
      return(SetStreamBuffering(image_info,image));
    }
#endif
  GetPathComponent(image->filename,ExtensionPath,extension);
  if (*type == 'w')
    {
      (void) CopyMagickString(filename,image->filename,MagickPathExtent);
      if ((image_info->adjoin == MagickFalse) ||
          (strchr(filename,'%') != (char *) NULL))
        {
          /*
            Form filename for multi-part images.
          */
          (void) InterpretImageFilename(image_info,image,image->filename,(int)
            image->scene,filename,exception);
          if ((LocaleCompare(filename,image->filename) == 0) &&
              ((GetPreviousImageInList(image) != (Image *) NULL) ||
               (GetNextImageInList(image) != (Image *) NULL)))
            {
              char
                path[MagickPathExtent];

              GetPathComponent(image->filename,RootPath,path);
              if (*extension == '\0')
                (void) FormatLocaleString(filename,MagickPathExtent,"%s-%.20g",
                  path,(double) image->scene);
              else
                (void) FormatLocaleString(filename,MagickPathExtent,
                  "%s-%.20g.%s",path,(double) image->scene,extension);
            }
          (void) CopyMagickString(image->filename,filename,MagickPathExtent);
#if defined(macintosh)
          SetApplicationType(filename,image_info->magick,'8BIM');
#endif
        }
    }
  if (image_info->file != (FILE *) NULL)
    {
      image->blob->file_info.file=image_info->file;
      image->blob->type=FileStream;
      image->blob->exempt=MagickTrue;
    }
  else
    if (*type == 'r')
      {
        image->blob->file_info.file=(FILE *) fopen_utf8(filename,type);
        if (image->blob->file_info.file != (FILE *) NULL)
          {
            size_t
              count;

            unsigned char
              magick[3];

            image->blob->type=FileStream;
            (void) SetStreamBuffering(image_info,image);
            (void) ResetMagickMemory(magick,0,sizeof(magick));
            count=fread(magick,1,sizeof(magick),image->blob->file_info.file);
            (void) fseek(image->blob->file_info.file,-((off_t) count),SEEK_CUR);
#if defined(MAGICKCORE_POSIX_SUPPORT)
            (void) fflush(image->blob->file_info.file);
#endif
            (void) LogMagickEvent(BlobEvent,GetMagickModule(),
               "  read %.20g magic header bytes",(double) count);
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (((int) magick[0] == 0x1F) && ((int) magick[1] == 0x8B) &&
                ((int) magick[2] == 0x08))
              {
                if (image->blob->file_info.file != (FILE *) NULL)
                  (void) fclose(image->blob->file_info.file);
                image->blob->file_info.file=(FILE *) NULL;
                image->blob->file_info.gzfile=gzopen(filename,type);
                if (image->blob->file_info.gzfile != (gzFile) NULL)
                  image->blob->type=ZipStream;
               }
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
            if (strncmp((char *) magick,"BZh",3) == 0)
              {
                if (image->blob->file_info.file != (FILE *) NULL)
                  (void) fclose(image->blob->file_info.file);
                image->blob->file_info.file=(FILE *) NULL;
                image->blob->file_info.bzfile=BZ2_bzopen(filename,type);
                if (image->blob->file_info.bzfile != (BZFILE *) NULL)
                  image->blob->type=BZipStream;
              }
#endif
            if (image->blob->type == FileStream)
              {
                const MagickInfo
                  *magick_info;

                ExceptionInfo
                  *sans_exception;

                size_t
                  length;

                sans_exception=AcquireExceptionInfo();
                magick_info=GetMagickInfo(image_info->magick,sans_exception);
                sans_exception=DestroyExceptionInfo(sans_exception);
                length=(size_t) image->blob->properties.st_size;
                if ((magick_info != (const MagickInfo *) NULL) &&
                    (GetMagickBlobSupport(magick_info) != MagickFalse) &&
                    (length > MagickMaxBufferExtent) &&
                    (AcquireMagickResource(MapResource,length) != MagickFalse))
                  {
                    void
                      *blob;

                    blob=MapBlob(fileno(image->blob->file_info.file),ReadMode,0,
                      length);
                    if (blob == (void *) NULL)
                      RelinquishMagickResource(MapResource,length);
                    else
                      {
                        /*
                          Format supports blobs-- use memory-mapped I/O.
                        */
                        if (image_info->file != (FILE *) NULL)
                          image->blob->exempt=MagickFalse;
                        else
                          {
                            (void) fclose(image->blob->file_info.file);
                            image->blob->file_info.file=(FILE *) NULL;
                          }
                        AttachBlob(image->blob,blob,length);
                        image->blob->mapped=MagickTrue;
                      }
                  }
              }
          }
        }
      else
#if defined(MAGICKCORE_ZLIB_DELEGATE)
        if ((LocaleCompare(extension,"Z") == 0) ||
            (LocaleCompare(extension,"gz") == 0) ||
            (LocaleCompare(extension,"wmz") == 0) ||
            (LocaleCompare(extension,"svgz") == 0))
          {
            if (mode == WriteBinaryBlobMode)
              type="wb";
            image->blob->file_info.gzfile=gzopen(filename,type);
            if (image->blob->file_info.gzfile != (gzFile) NULL)
              image->blob->type=ZipStream;
          }
        else
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
          if (LocaleCompare(extension,"bz2") == 0)
            {
              image->blob->file_info.bzfile=BZ2_bzopen(filename,type);
              if (image->blob->file_info.bzfile != (BZFILE *) NULL)
                image->blob->type=BZipStream;
            }
          else
#endif
            {
              image->blob->file_info.file=(FILE *) fopen_utf8(filename,type);
              if (image->blob->file_info.file != (FILE *) NULL)
                {
                  image->blob->type=FileStream;
                  (void) SetStreamBuffering(image_info,image);
                }
       }
  image->blob->status=MagickFalse;
  if (image->blob->type != UndefinedStream)
    image->blob->size=GetBlobSize(image);
  else
    {
      ThrowFileException(exception,BlobError,"UnableToOpenBlob",filename);
      return(MagickFalse);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   P i n g B l o b                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PingBlob() returns all the attributes of an image or image sequence except
%  for the pixels.  It is much faster and consumes far less memory than
%  BlobToImage().  On failure, a NULL image is returned and exception
%  describes the reason for the failure.
%
%  The format of the PingBlob method is:
%
%      Image *PingBlob(const ImageInfo *image_info,const void *blob,
%        const size_t length,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o blob: the address of a character stream in one of the image formats
%      understood by ImageMagick.
%
%    o length: This size_t integer reflects the length in bytes of the blob.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static size_t PingStream(const Image *magick_unused(image),
  const void *magick_unused(pixels),const size_t columns)
{
  magick_unreferenced(image);
  magick_unreferenced(pixels);
  return(columns);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport Image *PingBlob(const ImageInfo *image_info,const void *blob,
  const size_t length,ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *ping_info;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  if ((blob == (const void *) NULL) || (length == 0))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),BlobError,
        "UnrecognizedImageFormat","`%s'",image_info->magick);
      return((Image *) NULL);
    }
  ping_info=CloneImageInfo(image_info);
  ping_info->blob=(void *) AcquireQuantumMemory(length,sizeof(unsigned char));
  if (ping_info->blob == (const void *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitFatalError,"MemoryAllocationFailed","`%s'","");
      return((Image *) NULL);
    }
  (void) memcpy(ping_info->blob,blob,length);
  ping_info->length=length;
  ping_info->ping=MagickTrue;
  image=ReadStream(ping_info,&PingStream,exception);
  ping_info->blob=(void *) RelinquishMagickMemory(ping_info->blob);
  ping_info=DestroyImageInfo(ping_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b                                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlob() reads data from the blob or image file and returns it.  It
%  returns the number of bytes read. If length is zero, ReadBlob() returns
%  zero and has no other results. If length is greater than SSIZE_MAX, the
%  result is unspecified.
%
%  The format of the ReadBlob method is:
%
%      ssize_t ReadBlob(Image *image,const size_t length,void *data)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o length:  Specifies an integer representing the number of bytes to read
%      from the file.
%
%    o data:  Specifies an area to place the information requested from the
%      file.
%
*/
MagickExport ssize_t ReadBlob(Image *image,const size_t length,void *data)
{
  int
    c;

  register unsigned char
    *q;

  ssize_t
    count;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->type != UndefinedStream);
  if (length == 0)
    return(0);
  assert(data != (void *) NULL);
  count=0;
  q=(unsigned char *) data;
  switch (image->blob->type)
  {
    case UndefinedStream:
      break;
    case StandardStream:
    case FileStream:
    case PipeStream:
    {
      switch (length)
      {
        default:
        {
          count=(ssize_t) fread(q,1,length,image->blob->file_info.file);
          break;
        }
        case 4:
        {
          c=getc(image->blob->file_info.file);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 3:
        {
          c=getc(image->blob->file_info.file);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 2:
        {
          c=getc(image->blob->file_info.file);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 1:
        {
          c=getc(image->blob->file_info.file);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 0:
          break;
      }
      break;
    }
    case ZipStream:
    {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      switch (length)
      {
        default:
        {
          count=(ssize_t) gzread(image->blob->file_info.gzfile,q,
            (unsigned int) length);
          break;
        }
        case 4:
        {
          c=gzgetc(image->blob->file_info.gzfile);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 3:
        {
          c=gzgetc(image->blob->file_info.gzfile);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 2:
        {
          c=gzgetc(image->blob->file_info.gzfile);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 1:
        {
          c=gzgetc(image->blob->file_info.gzfile);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 0:
          break;
      }
#endif
      break;
    }
    case BZipStream:
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      count=(ssize_t) BZ2_bzread(image->blob->file_info.bzfile,q,(int) length);
#endif
      break;
    }
    case FifoStream:
      break;
    case BlobStream:
    {
      register const unsigned char
        *p;

      if (image->blob->offset >= (MagickOffsetType) image->blob->length)
        {
          image->blob->eof=MagickTrue;
          break;
        }
      p=image->blob->data+image->blob->offset;
      count=(ssize_t) MagickMin((MagickOffsetType) length,image->blob->length-
        image->blob->offset);
      image->blob->offset+=count;
      if (count != (ssize_t) length)
        image->blob->eof=MagickTrue;
      (void) memcpy(q,p,(size_t) count);
      break;
    }
    case CustomStream:
    {
      if (image->blob->custom_stream->reader != (CustomStreamHandler) NULL)
        count=image->blob->custom_stream->reader(q,length,
          image->blob->custom_stream->data);
      break;
    }
  }
  return(count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b B y t e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobByte() reads a single byte from the image file and returns it.
%
%  The format of the ReadBlobByte method is:
%
%      int ReadBlobByte(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport int ReadBlobByte(Image *image)
{
  register const unsigned char
    *p;

  ssize_t
    count;

  unsigned char
    buffer[1];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  p=(const unsigned char *) ReadBlobStream(image,1,buffer,&count);
  if (count != 1)
    return(EOF);
  return((int) (*p));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b D o u b l e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobDouble() reads a double value as a 64-bit quantity in the byte-order
%  specified by the endian member of the image structure.
%
%  The format of the ReadBlobDouble method is:
%
%      double ReadBlobDouble(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport double ReadBlobDouble(Image *image)
{
  union
  {
    MagickSizeType
      unsigned_value;

    double
      double_value;
  } quantum;

  quantum.double_value=0.0;
  quantum.unsigned_value=ReadBlobLongLong(image);
  return(quantum.double_value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b F l o a t                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobFloat() reads a float value as a 32-bit quantity in the byte-order
%  specified by the endian member of the image structure.
%
%  The format of the ReadBlobFloat method is:
%
%      float ReadBlobFloat(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport float ReadBlobFloat(Image *image)
{
  union
  {
    unsigned int
      unsigned_value;

    float
      float_value;
  } quantum;

  quantum.float_value=0.0;
  quantum.unsigned_value=ReadBlobLong(image);
  return(quantum.float_value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b L o n g                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobLong() reads a unsigned int value as a 32-bit quantity in the
%  byte-order specified by the endian member of the image structure.
%
%  The format of the ReadBlobLong method is:
%
%      unsigned int ReadBlobLong(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport unsigned int ReadBlobLong(Image *image)
{
  register const unsigned char
    *p;

  ssize_t
    count;

  unsigned char
    buffer[4];

  unsigned int
    value;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  *buffer='\0';
  p=(const unsigned char *) ReadBlobStream(image,4,buffer,&count);
  if (count != 4)
    return(0UL);
  if (image->endian == LSBEndian)
    {
      value=(unsigned int) (*p++);
      value|=(unsigned int) (*p++) << 8;
      value|=(unsigned int) (*p++) << 16;
      value|=(unsigned int) (*p++) << 24;
      return(value);
    }
  value=(unsigned int) (*p++) << 24;
  value|=(unsigned int) (*p++) << 16;
  value|=(unsigned int) (*p++) << 8;
  value|=(unsigned int) (*p++);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b L o n g L o n g                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobLongLong() reads a long long value as a 64-bit quantity in the
%  byte-order specified by the endian member of the image structure.
%
%  The format of the ReadBlobLongLong method is:
%
%      MagickSizeType ReadBlobLongLong(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport MagickSizeType ReadBlobLongLong(Image *image)
{
  MagickSizeType
    value;

  register const unsigned char
    *p;

  ssize_t
    count;

  unsigned char
    buffer[8];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  *buffer='\0';
  p=(const unsigned char *) ReadBlobStream(image,8,buffer,&count);
  if (count != 8)
    return(MagickULLConstant(0));
  if (image->endian == LSBEndian)
    {
      value=(MagickSizeType) (*p++);
      value|=(MagickSizeType) (*p++) << 8;
      value|=(MagickSizeType) (*p++) << 16;
      value|=(MagickSizeType) (*p++) << 24;
      value|=(MagickSizeType) (*p++) << 32;
      value|=(MagickSizeType) (*p++) << 40;
      value|=(MagickSizeType) (*p++) << 48;
      value|=(MagickSizeType) (*p++) << 56;
      return(value);
    }
  value=(MagickSizeType) (*p++) << 56;
  value|=(MagickSizeType) (*p++) << 48;
  value|=(MagickSizeType) (*p++) << 40;
  value|=(MagickSizeType) (*p++) << 32;
  value|=(MagickSizeType) (*p++) << 24;
  value|=(MagickSizeType) (*p++) << 16;
  value|=(MagickSizeType) (*p++) << 8;
  value|=(MagickSizeType) (*p++);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b S h o r t                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobShort() reads a short value as a 16-bit quantity in the byte-order
%  specified by the endian member of the image structure.
%
%  The format of the ReadBlobShort method is:
%
%      unsigned short ReadBlobShort(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport unsigned short ReadBlobShort(Image *image)
{
  register const unsigned char
    *p;

  register unsigned short
    value;

  ssize_t
    count;

  unsigned char
    buffer[2];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  *buffer='\0';
  p=(const unsigned char *) ReadBlobStream(image,2,buffer,&count);
  if (count != 2)
    return((unsigned short) 0U);
  if (image->endian == LSBEndian)
    {
      value=(unsigned short) (*p++);
      value|=(unsigned short) (*p++) << 8;
      return(value);
    }
  value=(unsigned short) ((unsigned short) (*p++) << 8);
  value|=(unsigned short) (*p++);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b L S B L o n g                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobLSBLong() reads a unsigned int value as a 32-bit quantity in
%  least-significant byte first order.
%
%  The format of the ReadBlobLSBLong method is:
%
%      unsigned int ReadBlobLSBLong(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport unsigned int ReadBlobLSBLong(Image *image)
{
  register const unsigned char
    *p;

  register unsigned int
    value;

  ssize_t
    count;

  unsigned char
    buffer[4];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  *buffer='\0';
  p=(const unsigned char *) ReadBlobStream(image,4,buffer,&count);
  if (count != 4)
    return(0U);
  value=(unsigned int) (*p++);
  value|=(unsigned int) (*p++) << 8;
  value|=(unsigned int) (*p++) << 16;
  value|=(unsigned int) (*p++) << 24;
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b L S B S i g n e d L o n g                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobLSBSignedLong() reads a signed int value as a 32-bit quantity in
%  least-significant byte first order.
%
%  The format of the ReadBlobLSBSignedLong method is:
%
%      signed int ReadBlobLSBSignedLong(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport signed int ReadBlobLSBSignedLong(Image *image)
{
  union
  {
    unsigned int
      unsigned_value;

    signed int
      signed_value;
  } quantum;

  quantum.unsigned_value=ReadBlobLSBLong(image);
  return(quantum.signed_value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b L S B S h o r t                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobLSBShort() reads a short value as a 16-bit quantity in
%  least-significant byte first order.
%
%  The format of the ReadBlobLSBShort method is:
%
%      unsigned short ReadBlobLSBShort(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport unsigned short ReadBlobLSBShort(Image *image)
{
  register const unsigned char
    *p;

  register unsigned short
    value;

  ssize_t
    count;

  unsigned char
    buffer[2];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  *buffer='\0';
  p=(const unsigned char *) ReadBlobStream(image,2,buffer,&count);
  if (count != 2)
    return((unsigned short) 0U);
  value=(unsigned short) (*p++);
  value|=(unsigned short) (*p++) << 8;
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b L S B S i g n e d S h o r t                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobLSBSignedShort() reads a signed short value as a 16-bit quantity in
%  least-significant byte-order.
%
%  The format of the ReadBlobLSBSignedShort method is:
%
%      signed short ReadBlobLSBSignedShort(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport signed short ReadBlobLSBSignedShort(Image *image)
{
  union
  {
    unsigned short
      unsigned_value;

    signed short
      signed_value;
  } quantum;

  quantum.unsigned_value=ReadBlobLSBShort(image);
  return(quantum.signed_value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b M S B L o n g                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobMSBLong() reads a unsigned int value as a 32-bit quantity in
%  most-significant byte first order.
%
%  The format of the ReadBlobMSBLong method is:
%
%      unsigned int ReadBlobMSBLong(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport unsigned int ReadBlobMSBLong(Image *image)
{
  register const unsigned char
    *p;

  register unsigned int
    value;

  ssize_t
    count;

  unsigned char
    buffer[4];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  *buffer='\0';
  p=(const unsigned char *) ReadBlobStream(image,4,buffer,&count);
  if (count != 4)
    return(0UL);
  value=(unsigned int) (*p++) << 24;
  value|=(unsigned int) (*p++) << 16;
  value|=(unsigned int) (*p++) << 8;
  value|=(unsigned int) (*p++);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b M S B L o n g L o n g                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobMSBLongLong() reads a unsigned long long value as a 64-bit quantity
%  in most-significant byte first order.
%
%  The format of the ReadBlobMSBLongLong method is:
%
%      unsigned int ReadBlobMSBLongLong(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport MagickSizeType ReadBlobMSBLongLong(Image *image)
{
  register const unsigned char
    *p;

  register MagickSizeType
    value;

  ssize_t
    count;

  unsigned char
    buffer[8];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  *buffer='\0';
  p=(const unsigned char *) ReadBlobStream(image,8,buffer,&count);
  if (count != 8)
    return(MagickULLConstant(0));
  value=(MagickSizeType) (*p++) << 56;
  value|=(MagickSizeType) (*p++) << 48;
  value|=(MagickSizeType) (*p++) << 40;
  value|=(MagickSizeType) (*p++) << 32;
  value|=(MagickSizeType) (*p++) << 24;
  value|=(MagickSizeType) (*p++) << 16;
  value|=(MagickSizeType) (*p++) << 8;
  value|=(MagickSizeType) (*p++);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b M S B S h o r t                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobMSBShort() reads a short value as a 16-bit quantity in
%  most-significant byte first order.
%
%  The format of the ReadBlobMSBShort method is:
%
%      unsigned short ReadBlobMSBShort(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport unsigned short ReadBlobMSBShort(Image *image)
{
  register const unsigned char
    *p;

  register unsigned short
    value;

  ssize_t
    count;

  unsigned char
    buffer[2];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  *buffer='\0';
  p=(const unsigned char *) ReadBlobStream(image,2,buffer,&count);
  if (count != 2)
    return((unsigned short) 0U);
  value=(unsigned short) ((*p++) << 8);
  value|=(unsigned short) (*p++);
  return((unsigned short) (value & 0xffff));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b M S B S i g n e d L o n g                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobMSBSignedLong() reads a signed int value as a 32-bit quantity in
%  most-significant byte-order.
%
%  The format of the ReadBlobMSBSignedLong method is:
%
%      signed int ReadBlobMSBSignedLong(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport signed int ReadBlobMSBSignedLong(Image *image)
{
  union
  {
    unsigned int
      unsigned_value;

    signed int
      signed_value;
  } quantum;

  quantum.unsigned_value=ReadBlobMSBLong(image);
  return(quantum.signed_value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b M S B S i g n e d S h o r t                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobMSBSignedShort() reads a signed short value as a 16-bit quantity in
%  most-significant byte-order.
%
%  The format of the ReadBlobMSBSignedShort method is:
%
%      signed short ReadBlobMSBSignedShort(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport signed short ReadBlobMSBSignedShort(Image *image)
{
  union
  {
    unsigned short
      unsigned_value;

    signed short
      signed_value;
  } quantum;

  quantum.unsigned_value=ReadBlobMSBShort(image);
  return(quantum.signed_value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b S i g n e d L o n g                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobSignedLong() reads a signed int value as a 32-bit quantity in the
%  byte-order specified by the endian member of the image structure.
%
%  The format of the ReadBlobSignedLong method is:
%
%      signed int ReadBlobSignedLong(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport signed int ReadBlobSignedLong(Image *image)
{
  union
  {
    unsigned int
      unsigned_value;

    signed int
      signed_value;
  } quantum;

  quantum.unsigned_value=ReadBlobLong(image);
  return(quantum.signed_value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b S i g n e d S h o r t                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobSignedShort() reads a signed short value as a 16-bit quantity in the
%  byte-order specified by the endian member of the image structure.
%
%  The format of the ReadBlobSignedShort method is:
%
%      signed short ReadBlobSignedShort(Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport signed short ReadBlobSignedShort(Image *image)
{
  union
  {
    unsigned short
      unsigned_value;

    signed short
      signed_value;
  } quantum;

  quantum.unsigned_value=ReadBlobShort(image);
  return(quantum.signed_value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b S t r e a m                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobStream() reads data from the blob or image file and returns it.  It
%  returns a pointer to the data buffer you supply or to the image memory
%  buffer if its supported (zero-copy). If length is zero, ReadBlobStream()
%  returns a count of zero and has no other results. If length is greater than
%  SSIZE_MAX, the result is unspecified.
%
%  The format of the ReadBlobStream method is:
%
%      const void *ReadBlobStream(Image *image,const size_t length,void *data,
%        ssize_t *count)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o length:  Specifies an integer representing the number of bytes to read
%      from the file.
%
%    o count: returns the number of bytes read.
%
%    o data:  Specifies an area to place the information requested from the
%      file.
%
*/
MagickExport const void *ReadBlobStream(Image *image,const size_t length,
  void *data,ssize_t *count)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->type != UndefinedStream);
  assert(count != (ssize_t *) NULL);
  if (image->blob->type != BlobStream)
    {
      assert(data != NULL);
      *count=ReadBlob(image,length,(unsigned char *) data);
      return(data);
    }
  if (image->blob->offset >= (MagickOffsetType) image->blob->length)
    {
      *count=0;
      image->blob->eof=MagickTrue;
      return(data);
    }
  data=image->blob->data+image->blob->offset;
  *count=(ssize_t) MagickMin((MagickOffsetType) length,image->blob->length-
    image->blob->offset);
  image->blob->offset+=(*count);
  if (*count != (ssize_t) length)
    image->blob->eof=MagickTrue;
  return(data);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d B l o b S t r i n g                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobString() reads characters from a blob or file until a newline
%  character is read or an end-of-file condition is encountered.
%
%  The format of the ReadBlobString method is:
%
%      char *ReadBlobString(Image *image,char *string)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o string: the address of a character buffer.
%
*/
MagickExport char *ReadBlobString(Image *image,char *string)
{
  register const unsigned char
    *p;

  register ssize_t
    i;

  ssize_t
    count;

  unsigned char
    buffer[1];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  for (i=0; i < (MagickPathExtent-1L); i++)
  {
    p=(const unsigned char *) ReadBlobStream(image,1,buffer,&count);
    if (count != 1)
      {
        if (i == 0)
          return((char *) NULL);
        string[i]='\0';
        break;
      }
    string[i]=(char) (*p);
    if ((string[i] == '\r') || (string[i] == '\n'))
      break;
  }
  if (string[i] == '\r')
    (void) ReadBlobStream(image,1,buffer,&count);
  string[i]='\0';
  return(string);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e f e r e n c e B l o b                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReferenceBlob() increments the reference count associated with the pixel
%  blob returning a pointer to the blob.
%
%  The format of the ReferenceBlob method is:
%
%      BlobInfo ReferenceBlob(BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob_info.
%
*/
MagickExport BlobInfo *ReferenceBlob(BlobInfo *blob)
{
  assert(blob != (BlobInfo *) NULL);
  assert(blob->signature == MagickCoreSignature);
  if (blob->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  LockSemaphoreInfo(blob->semaphore);
  blob->reference_count++;
  UnlockSemaphoreInfo(blob->semaphore);
  return(blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  S e e k B l o b                                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SeekBlob() sets the offset in bytes from the beginning of a blob or file
%  and returns the resulting offset.
%
%  The format of the SeekBlob method is:
%
%      MagickOffsetType SeekBlob(Image *image,const MagickOffsetType offset,
%        const int whence)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o offset:  Specifies an integer representing the offset in bytes.
%
%    o whence:  Specifies an integer representing how the offset is
%      treated relative to the beginning of the blob as follows:
%
%        SEEK_SET  Set position equal to offset bytes.
%        SEEK_CUR  Set position to current location plus offset.
%        SEEK_END  Set position to EOF plus offset.
%
*/
MagickExport MagickOffsetType SeekBlob(Image *image,
  const MagickOffsetType offset,const int whence)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->type != UndefinedStream);
  switch (image->blob->type)
  {
    case UndefinedStream:
      break;
    case StandardStream:
    case PipeStream:
      return(-1);
    case FileStream:
    {
      if ((offset < 0) && (whence == SEEK_SET))
        return(-1);
      if (fseek(image->blob->file_info.file,offset,whence) < 0)
        return(-1);
      image->blob->offset=TellBlob(image);
      break;
    }
    case ZipStream:
    {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      if (gzseek(image->blob->file_info.gzfile,(off_t) offset,whence) < 0)
        return(-1);
#endif
      image->blob->offset=TellBlob(image);
      break;
    }
    case BZipStream:
      return(-1);
    case FifoStream:
      return(-1);
    case BlobStream:
    {
      switch (whence)
      {
        case SEEK_SET:
        default:
        {
          if (offset < 0)
            return(-1);
          image->blob->offset=offset;
          break;
        }
        case SEEK_CUR:
        {
          if ((image->blob->offset+offset) < 0)
            return(-1);
          image->blob->offset+=offset;
          break;
        }
        case SEEK_END:
        {
          if (((MagickOffsetType) image->blob->length+offset) < 0)
            return(-1);
          image->blob->offset=image->blob->length+offset;
          break;
        }
      }
      if (image->blob->offset < (MagickOffsetType)
          ((off_t) image->blob->length))
        {
          image->blob->eof=MagickFalse;
          break;
        }
      if (image->blob->offset < (MagickOffsetType)
          ((off_t) image->blob->extent))
        break;
      if (image->blob->mapped != MagickFalse)
        {
          image->blob->eof=MagickTrue;
          return(-1);
        }
      image->blob->extent=(size_t) (image->blob->offset+image->blob->quantum);
      image->blob->quantum<<=1;
      image->blob->data=(unsigned char *) ResizeQuantumMemory(image->blob->data,
        image->blob->extent+1,sizeof(*image->blob->data));
      (void) SyncBlob(image);
      if (image->blob->data == NULL)
        {
          (void) DetachBlob(image->blob);
          return(-1);
        }
      break;
    }
    case CustomStream:
    {
      if (image->blob->custom_stream->seeker == (CustomStreamSeeker) NULL)
        return(-1);
      image->blob->offset=image->blob->custom_stream->seeker(offset,whence,
        image->blob->custom_stream->data);
      break;
    }
  }
  return(image->blob->offset);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t B l o b E x e m p t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetBlobExempt() sets the blob exempt status.
%
%  The format of the SetBlobExempt method is:
%
%      MagickBooleanType SetBlobExempt(const Image *image,
%        const MagickBooleanType exempt)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exempt: Set to true if this blob is exempt from being closed.
%
*/
MagickExport void SetBlobExempt(Image *image,const MagickBooleanType exempt)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  image->blob->exempt=exempt;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  S e t B l o b E x t e n t                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetBlobExtent() ensures enough space is allocated for the blob.  If the
%  method is successful, subsequent writes to bytes in the specified range are
%  guaranteed not to fail.
%
%  The format of the SetBlobExtent method is:
%
%      MagickBooleanType SetBlobExtent(Image *image,const MagickSizeType extent)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o extent:  the blob maximum extent.
%
*/
MagickExport MagickBooleanType SetBlobExtent(Image *image,
  const MagickSizeType extent)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->type != UndefinedStream);
  switch (image->blob->type)
  {
    case UndefinedStream:
      break;
    case StandardStream:
      return(MagickFalse);
    case FileStream:
    {
      MagickOffsetType
        offset;

      ssize_t
        count;

      if (extent != (MagickSizeType) ((off_t) extent))
        return(MagickFalse);
      offset=SeekBlob(image,0,SEEK_END);
      if (offset < 0)
        return(MagickFalse);
      if ((MagickSizeType) offset >= extent)
        break;
      offset=SeekBlob(image,(MagickOffsetType) extent-1,SEEK_SET);
      if (offset < 0)
        break;
      count=(ssize_t) fwrite((const unsigned char *) "",1,1,
        image->blob->file_info.file);
#if defined(MAGICKCORE_HAVE_POSIX_FALLOCATE)
      if (image->blob->synchronize != MagickFalse)
        {
          int
            file;

          file=fileno(image->blob->file_info.file);
          if ((file == -1) || (offset < 0))
            return(MagickFalse);
          (void) posix_fallocate(file,offset,extent-offset);
        }
#endif
      offset=SeekBlob(image,offset,SEEK_SET);
      if (count != 1)
        return(MagickFalse);
      break;
    }
    case PipeStream:
    case ZipStream:
      return(MagickFalse);
    case BZipStream:
      return(MagickFalse);
    case FifoStream:
      return(MagickFalse);
    case BlobStream:
    {
      if (extent != (MagickSizeType) ((size_t) extent))
        return(MagickFalse);
      if (image->blob->mapped != MagickFalse)
        {
          MagickOffsetType
            offset;

          ssize_t
            count;

          (void) UnmapBlob(image->blob->data,image->blob->length);
          RelinquishMagickResource(MapResource,image->blob->length);
          if (extent != (MagickSizeType) ((off_t) extent))
            return(MagickFalse);
          offset=SeekBlob(image,0,SEEK_END);
          if (offset < 0)
            return(MagickFalse);
          if ((MagickSizeType) offset >= extent)
            break;
          offset=SeekBlob(image,(MagickOffsetType) extent-1,SEEK_SET);
          count=(ssize_t) fwrite((const unsigned char *) "",1,1,
            image->blob->file_info.file);
#if defined(MAGICKCORE_HAVE_POSIX_FALLOCATE)
          if (image->blob->synchronize != MagickFalse)
            {
              int
                file;

              file=fileno(image->blob->file_info.file);
              if ((file == -1) || (offset < 0))
                return(MagickFalse);
              (void) posix_fallocate(file,offset,extent-offset);
            }
#endif
          offset=SeekBlob(image,offset,SEEK_SET);
          if (count != 1)
            return(MagickFalse);
          (void) AcquireMagickResource(MapResource,extent);
          image->blob->data=(unsigned char*) MapBlob(fileno(
            image->blob->file_info.file),WriteMode,0,(size_t) extent);
          image->blob->extent=(size_t) extent;
          image->blob->length=(size_t) extent;
          (void) SyncBlob(image);
          break;
        }
      image->blob->extent=(size_t) extent;
      image->blob->data=(unsigned char *) ResizeQuantumMemory(image->blob->data,
        image->blob->extent+1,sizeof(*image->blob->data));
      (void) SyncBlob(image);
      if (image->blob->data == (unsigned char *) NULL)
        {
          (void) DetachBlob(image->blob);
          return(MagickFalse);
        }
      break;
    }
    case CustomStream:
      break;
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  S e t C u s t o m S t r e a m D a t a                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCustomStreamData() sets the stream info data member.
%
%  The format of the SetCustomStreamData method is:
%
%      void SetCustomStreamData(CustomStreamInfo *custom_stream,void *)
%
%  A description of each parameter follows:
%
%    o custom_stream: the custom stream info.
%
%    o data: an object containing information about the custom stream.
%
*/
MagickExport void SetCustomStreamData(CustomStreamInfo *custom_stream,
  void *data)
{
  assert(custom_stream != (CustomStreamInfo *) NULL);
  assert(custom_stream->signature == MagickCoreSignature);
  custom_stream->data=data;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  S e t C u s t o m S t r e a m R e a d e r                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCustomStreamReader() sets the stream info reader member.
%
%  The format of the SetCustomStreamReader method is:
%
%      void SetCustomStreamReader(CustomStreamInfo *custom_stream,
%        CustomStreamHandler reader)
%
%  A description of each parameter follows:
%
%    o custom_stream: the custom stream info.
%
%    o reader: a function to read from the stream.
%
*/
MagickExport void SetCustomStreamReader(CustomStreamInfo *custom_stream,
  CustomStreamHandler reader)
{
  assert(custom_stream != (CustomStreamInfo *) NULL);
  assert(custom_stream->signature == MagickCoreSignature);
  custom_stream->reader=reader;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  S e t C u s t o m S t r e a m S e e k e r                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCustomStreamSeeker() sets the stream info seeker member.
%
%  The format of the SetCustomStreamReader method is:
%
%      void SetCustomStreamSeeker(CustomStreamInfo *custom_stream,
%        CustomStreamSeeker seeker)
%
%  A description of each parameter follows:
%
%    o custom_stream: the custom stream info.
%
%    o seeker: a function to seek in the custom stream.
%
*/
MagickExport void SetCustomStreamSeeker(CustomStreamInfo *custom_stream,
  CustomStreamSeeker seeker)
{
  assert(custom_stream != (CustomStreamInfo *) NULL);
  assert(custom_stream->signature == MagickCoreSignature);
  custom_stream->seeker=seeker;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  S e t C u s t o m S t r e a m T e l l e r                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCustomStreamTeller() sets the stream info teller member.
%
%  The format of the SetCustomStreamTeller method is:
%
%      void SetCustomStreamTeller(CustomStreamInfo *custom_stream,
%        CustomStreamTeller *teller)
%
%  A description of each parameter follows:
%
%    o custom_stream: the custom stream info.
%
%    o teller: a function to set the position in the stream.
%
*/
MagickExport void SetCustomStreamTeller(CustomStreamInfo *custom_stream,
  CustomStreamTeller teller)
{
  assert(custom_stream != (CustomStreamInfo *) NULL);
  assert(custom_stream->signature == MagickCoreSignature);
  custom_stream->teller=teller;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  S e t C u s t o m S t r e a m W r i t e r                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCustomStreamWriter() sets the stream info writer member.
%
%  The format of the SetCustomStreamWriter method is:
%
%      void SetCustomStreamWriter(CustomStreamInfo *custom_stream,
%        CustomStreamHandler *writer)
%
%  A description of each parameter follows:
%
%    o custom_stream: the custom stream info.
%
%    o writer: a function to write to the custom stream.
%
*/
MagickExport void SetCustomStreamWriter(CustomStreamInfo *custom_stream,
  CustomStreamHandler writer)
{
  assert(custom_stream != (CustomStreamInfo *) NULL);
  assert(custom_stream->signature == MagickCoreSignature);
  custom_stream->writer=writer;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  S y n c B l o b                                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncBlob() flushes the datastream if it is a file or synchronizes the data
%  attributes if it is an blob.
%
%  The format of the SyncBlob method is:
%
%      int SyncBlob(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
static int SyncBlob(Image *image)
{
  int
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->type != UndefinedStream);
  status=0;
  switch (image->blob->type)
  {
    case UndefinedStream:
    case StandardStream:
      break;
    case FileStream:
    case PipeStream:
    {
      status=fflush(image->blob->file_info.file);
      break;
    }
    case ZipStream:
    {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      status=gzflush(image->blob->file_info.gzfile,Z_SYNC_FLUSH);
#endif
      break;
    }
    case BZipStream:
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      status=BZ2_bzflush(image->blob->file_info.bzfile);
#endif
      break;
    }
    case FifoStream:
      break;
    case BlobStream:
      break;
    case CustomStream:
      break;
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  T e l l B l o b                                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TellBlob() obtains the current value of the blob or file position.
%
%  The format of the TellBlob method is:
%
%      MagickOffsetType TellBlob(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickOffsetType TellBlob(const Image *image)
{
  MagickOffsetType
    offset;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->type != UndefinedStream);
  offset=(-1);
  switch (image->blob->type)
  {
    case UndefinedStream:
    case StandardStream:
      break;
    case FileStream:
    {
      offset=ftell(image->blob->file_info.file);
      break;
    }
    case PipeStream:
      break;
    case ZipStream:
    {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      offset=(MagickOffsetType) gztell(image->blob->file_info.gzfile);
#endif
      break;
    }
    case BZipStream:
      break;
    case FifoStream:
      break;
    case BlobStream:
    {
      offset=image->blob->offset;
      break;
    }
    case CustomStream:
    {
      if (image->blob->custom_stream->teller != (CustomStreamTeller) NULL)
        offset=image->blob->custom_stream->teller(image->blob->custom_stream->data);
      break;
    }
  }
  return(offset);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  U n m a p B l o b                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnmapBlob() deallocates the binary large object previously allocated with
%  the MapBlob method.
%
%  The format of the UnmapBlob method is:
%
%       MagickBooleanType UnmapBlob(void *map,const size_t length)
%
%  A description of each parameter follows:
%
%    o map: the address  of the binary large object.
%
%    o length: the length of the binary large object.
%
*/
MagickExport MagickBooleanType UnmapBlob(void *map,const size_t length)
{
#if defined(MAGICKCORE_HAVE_MMAP)
  int
    status;

  status=munmap(map,length);
  return(status == -1 ? MagickFalse : MagickTrue);
#else
  (void) map;
  (void) length;
  return(MagickFalse);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C u s t o m S t r e a m T o I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CustomStreamToImage() is the equivalent of ReadImage(), but reads the
%  formatted "file" from the suplied method rather than to an actual file.
%
%  The format of the CustomStreamToImage method is:
%
%      Image *CustomStreamToImage(const ImageInfo *image_info,
%         ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *CustomStreamToImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  const MagickInfo
    *magick_info;

  Image
    *image;

  ImageInfo
    *blob_info;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(image_info->custom_stream != (CustomStreamInfo *) NULL);
  assert(image_info->custom_stream->signature == MagickCoreSignature);
  assert(image_info->custom_stream->reader != (CustomStreamHandler) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  blob_info=CloneImageInfo(image_info);
  if (*blob_info->magick == '\0')
    (void) SetImageInfo(blob_info,0,exception);
  magick_info=GetMagickInfo(blob_info->magick,exception);
  if (magick_info == (const MagickInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        MissingDelegateError,"NoDecodeDelegateForThisImageFormat","`%s'",
        blob_info->magick);
      blob_info=DestroyImageInfo(blob_info);
      return((Image *) NULL);
    }
  image=(Image *) NULL;
  if ((GetMagickBlobSupport(magick_info) != MagickFalse) ||
      (blob_info->custom_stream == (CustomStreamInfo *) NULL))
    {
      /*
        Native blob support for this image format or SetImageInfo changed the
        blob to a file.
      */
      image=ReadImage(blob_info,exception);
      if (image != (Image *) NULL)
        (void) CloseBlob(image);
    }
  else
    {
      char
        unique[MagickPathExtent];

      int
        file;

      ImageInfo
        *clone_info;

      unsigned char
        *blob;

      /*
        Write data to file on disk.
      */
      blob_info->custom_stream=(CustomStreamInfo *) NULL;
      blob=(unsigned char *) AcquireQuantumMemory(MagickMaxBufferExtent,
        sizeof(*blob));
      if (blob == (unsigned char *) NULL)
        {
          ThrowFileException(exception,BlobError,"UnableToReadBlob",
            image_info->filename);
          blob_info=DestroyImageInfo(blob_info);
          return((Image *) NULL);
        }
      file=AcquireUniqueFileResource(unique);
      if (file == -1)
        {
          ThrowFileException(exception,BlobError,"UnableToReadBlob",
            image_info->filename);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          blob_info=DestroyImageInfo(blob_info);
          return((Image *) NULL);
        }
      clone_info=CloneImageInfo(blob_info);
      blob_info->file=fdopen(file,"wb+");
      if (blob_info->file != (FILE *) NULL)
        {
          ssize_t
            count;

          count=(ssize_t) MagickMaxBufferExtent;
          while (count == (ssize_t) MagickMaxBufferExtent)
          {
            count=image_info->custom_stream->reader(blob,MagickMaxBufferExtent,
              image_info->custom_stream->data);
            count=(ssize_t) write(file,(const char *) blob,(size_t) count);
          }
          (void) fclose(blob_info->file);
          (void) FormatLocaleString(clone_info->filename,MagickPathExtent,
            "%s:%s",blob_info->magick,unique);
          image=ReadImage(clone_info,exception);
          if (image != (Image *) NULL)
            {
              Image
                *images;

              /*
                Restore original filenames and image format.
              */
              for (images=GetFirstImageInList(image); images != (Image *) NULL; )
              {
                (void) CopyMagickString(images->filename,image_info->filename,
                  MagickPathExtent);
                (void) CopyMagickString(images->magick_filename,
                  image_info->filename,MagickPathExtent);
                (void) CopyMagickString(images->magick,magick_info->name,
                  MagickPathExtent);
                (void) CloseBlob(images);
                images=GetNextImageInList(images);
              }
            }
        }
      clone_info=DestroyImageInfo(clone_info);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      (void) RelinquishUniqueFileResource(unique);
    }
  blob_info=DestroyImageInfo(blob_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlob() writes data to a blob or image file.  It returns the number of
%  bytes written.
%
%  The format of the WriteBlob method is:
%
%      ssize_t WriteBlob(Image *image,const size_t length,const void *data)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o length:  Specifies an integer representing the number of bytes to
%      write to the file.
%
%    o data:  The address of the data to write to the blob or file.
%
*/
MagickExport ssize_t WriteBlob(Image *image,const size_t length,
  const void *data)
{
  int
    c;

  register const unsigned char
    *p;

  ssize_t
    count;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(image->blob != (BlobInfo *) NULL);
  assert(image->blob->type != UndefinedStream);
  if (length == 0)
    return(0);
  assert(data != (const void *) NULL);
  count=0;
  p=(const unsigned char *) data;
  switch (image->blob->type)
  {
    case UndefinedStream:
      break;
    case StandardStream:
    case FileStream:
    case PipeStream:
    {
      switch (length)
      {
        default:
        {
          count=(ssize_t) fwrite((const char *) data,1,length,
            image->blob->file_info.file);
          break;
        }
        case 4:
        {
          c=putc((int) *p++,image->blob->file_info.file);
          if (c == EOF)
            break;
          count++;
        }
        case 3:
        {
          c=putc((int) *p++,image->blob->file_info.file);
          if (c == EOF)
            break;
          count++;
        }
        case 2:
        {
          c=putc((int) *p++,image->blob->file_info.file);
          if (c == EOF)
            break;
          count++;
        }
        case 1:
        {
          c=putc((int) *p++,image->blob->file_info.file);
          if (c == EOF)
            break;
          count++;
        }
        case 0:
          break;
      }
      break;
    }
    case ZipStream:
    {
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      switch (length)
      {
        default:
        {
          count=(ssize_t) gzwrite(image->blob->file_info.gzfile,(void *) data,
            (unsigned int) length);
          break;
        }
        case 4:
        {
          c=gzputc(image->blob->file_info.gzfile,(int) *p++);
          if (c == EOF)
            break;
          count++;
        }
        case 3:
        {
          c=gzputc(image->blob->file_info.gzfile,(int) *p++);
          if (c == EOF)
            break;
          count++;
        }
        case 2:
        {
          c=gzputc(image->blob->file_info.gzfile,(int) *p++);
          if (c == EOF)
            break;
          count++;
        }
        case 1:
        {
          c=gzputc(image->blob->file_info.gzfile,(int) *p++);
          if (c == EOF)
            break;
          count++;
        }
        case 0:
          break;
      }
#endif
      break;
    }
    case BZipStream:
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      count=(ssize_t) BZ2_bzwrite(image->blob->file_info.bzfile,(void *) data,
        (int) length);
#endif
      break;
    }
    case FifoStream:
    {
      count=(ssize_t) image->blob->stream(image,data,length);
      break;
    }
    case BlobStream:
    {
      register unsigned char
        *q;

      if ((image->blob->offset+(MagickOffsetType) length) >=
          (MagickOffsetType) image->blob->extent)
        {
          if (image->blob->mapped != MagickFalse)
            return(0);
          image->blob->extent+=length+image->blob->quantum;
          image->blob->quantum<<=1;
          image->blob->data=(unsigned char *) ResizeQuantumMemory(
            image->blob->data,image->blob->extent+1,sizeof(*image->blob->data));
          (void) SyncBlob(image);
          if (image->blob->data == (unsigned char *) NULL)
            {
              (void) DetachBlob(image->blob);
              return(0);
            }
        }
      q=image->blob->data+image->blob->offset;
      (void) memcpy(q,p,length);
      image->blob->offset+=length;
      if (image->blob->offset >= (MagickOffsetType) image->blob->length)
        image->blob->length=(size_t) image->blob->offset;
      count=(ssize_t) length;
      break;
    }
    case CustomStream:
    {
      if (image->blob->custom_stream->writer != (CustomStreamHandler) NULL)
        count=image->blob->custom_stream->writer((unsigned char *) data,
          length,image->blob->custom_stream->data);
      break;
    }
  }
  return(count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b B y t e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobByte() write an integer to a blob.  It returns the number of bytes
%  written (either 0 or 1);
%
%  The format of the WriteBlobByte method is:
%
%      ssize_t WriteBlobByte(Image *image,const unsigned char value)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o value: Specifies the value to write.
%
*/
MagickExport ssize_t WriteBlobByte(Image *image,const unsigned char value)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  return(WriteBlobStream(image,1,&value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b F l o a t                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobFloat() writes a float value as a 32-bit quantity in the byte-order
%  specified by the endian member of the image structure.
%
%  The format of the WriteBlobFloat method is:
%
%      ssize_t WriteBlobFloat(Image *image,const float value)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o value: Specifies the value to write.
%
*/
MagickExport ssize_t WriteBlobFloat(Image *image,const float value)
{
  union
  {
    unsigned int
      unsigned_value;

    float
      float_value;
  } quantum;

  quantum.unsigned_value=0U;
  quantum.float_value=value;
  return(WriteBlobLong(image,quantum.unsigned_value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b L o n g                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobLong() writes a unsigned int value as a 32-bit quantity in the
%  byte-order specified by the endian member of the image structure.
%
%  The format of the WriteBlobLong method is:
%
%      ssize_t WriteBlobLong(Image *image,const unsigned int value)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o value: Specifies the value to write.
%
*/
MagickExport ssize_t WriteBlobLong(Image *image,const unsigned int value)
{
  unsigned char
    buffer[4];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->endian == LSBEndian)
    {
      buffer[0]=(unsigned char) value;
      buffer[1]=(unsigned char) (value >> 8);
      buffer[2]=(unsigned char) (value >> 16);
      buffer[3]=(unsigned char) (value >> 24);
      return(WriteBlobStream(image,4,buffer));
    }
  buffer[0]=(unsigned char) (value >> 24);
  buffer[1]=(unsigned char) (value >> 16);
  buffer[2]=(unsigned char) (value >> 8);
  buffer[3]=(unsigned char) value;
  return(WriteBlobStream(image,4,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b L o n g L o n g                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobMSBLongLong() writes a long long value as a 64-bit quantity in the
%  byte-order specified by the endian member of the image structure.
%
%  The format of the WriteBlobLongLong method is:
%
%      ssize_t WriteBlobLongLong(Image *image,const MagickSizeType value)
%
%  A description of each parameter follows.
%
%    o value:  Specifies the value to write.
%
%    o image: the image.
%
*/
MagickExport ssize_t WriteBlobLongLong(Image *image,const MagickSizeType value)
{
  unsigned char
    buffer[8];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->endian == LSBEndian)
    {
      buffer[0]=(unsigned char) value;
      buffer[1]=(unsigned char) (value >> 8);
      buffer[2]=(unsigned char) (value >> 16);
      buffer[3]=(unsigned char) (value >> 24);
      buffer[4]=(unsigned char) (value >> 32);
      buffer[5]=(unsigned char) (value >> 40);
      buffer[6]=(unsigned char) (value >> 48);
      buffer[7]=(unsigned char) (value >> 56);
      return(WriteBlobStream(image,8,buffer));
    }
  buffer[0]=(unsigned char) (value >> 56);
  buffer[1]=(unsigned char) (value >> 48);
  buffer[2]=(unsigned char) (value >> 40);
  buffer[3]=(unsigned char) (value >> 32);
  buffer[4]=(unsigned char) (value >> 24);
  buffer[5]=(unsigned char) (value >> 16);
  buffer[6]=(unsigned char) (value >> 8);
  buffer[7]=(unsigned char) value;
  return(WriteBlobStream(image,8,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e B l o b S h o r t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobShort() writes a short value as a 16-bit quantity in the
%  byte-order specified by the endian member of the image structure.
%
%  The format of the WriteBlobShort method is:
%
%      ssize_t WriteBlobShort(Image *image,const unsigned short value)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o value:  Specifies the value to write.
%
*/
MagickExport ssize_t WriteBlobShort(Image *image,const unsigned short value)
{
  unsigned char
    buffer[2];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->endian == LSBEndian)
    {
      buffer[0]=(unsigned char) value;
      buffer[1]=(unsigned char) (value >> 8);
      return(WriteBlobStream(image,2,buffer));
    }
  buffer[0]=(unsigned char) (value >> 8);
  buffer[1]=(unsigned char) value;
  return(WriteBlobStream(image,2,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b S i g n e d L o n g                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobSignedLong() writes a signed value as a 32-bit quantity in the
%  byte-order specified by the endian member of the image structure.
%
%  The format of the WriteBlobSignedLong method is:
%
%      ssize_t WriteBlobSignedLong(Image *image,const signed int value)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o value: Specifies the value to write.
%
*/
MagickExport ssize_t WriteBlobSignedLong(Image *image,const signed int value)
{
  union
  {
    unsigned int
      unsigned_value;

    signed int
      signed_value;
  } quantum;

  unsigned char
    buffer[4];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  quantum.signed_value=value;
  if (image->endian == LSBEndian)
    {
      buffer[0]=(unsigned char) quantum.unsigned_value;
      buffer[1]=(unsigned char) (quantum.unsigned_value >> 8);
      buffer[2]=(unsigned char) (quantum.unsigned_value >> 16);
      buffer[3]=(unsigned char) (quantum.unsigned_value >> 24);
      return(WriteBlobStream(image,4,buffer));
    }
  buffer[0]=(unsigned char) (quantum.unsigned_value >> 24);
  buffer[1]=(unsigned char) (quantum.unsigned_value >> 16);
  buffer[2]=(unsigned char) (quantum.unsigned_value >> 8);
  buffer[3]=(unsigned char) quantum.unsigned_value;
  return(WriteBlobStream(image,4,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b L S B L o n g                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobLSBLong() writes a unsigned int value as a 32-bit quantity in
%  least-significant byte first order.
%
%  The format of the WriteBlobLSBLong method is:
%
%      ssize_t WriteBlobLSBLong(Image *image,const unsigned int value)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o value: Specifies the value to write.
%
*/
MagickExport ssize_t WriteBlobLSBLong(Image *image,const unsigned int value)
{
  unsigned char
    buffer[4];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  buffer[0]=(unsigned char) value;
  buffer[1]=(unsigned char) (value >> 8);
  buffer[2]=(unsigned char) (value >> 16);
  buffer[3]=(unsigned char) (value >> 24);
  return(WriteBlobStream(image,4,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e B l o b L S B S h o r t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobLSBShort() writes a unsigned short value as a 16-bit quantity in
%  least-significant byte first order.
%
%  The format of the WriteBlobLSBShort method is:
%
%      ssize_t WriteBlobLSBShort(Image *image,const unsigned short value)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o value:  Specifies the value to write.
%
*/
MagickExport ssize_t WriteBlobLSBShort(Image *image,const unsigned short value)
{
  unsigned char
    buffer[2];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  buffer[0]=(unsigned char) value;
  buffer[1]=(unsigned char) (value >> 8);
  return(WriteBlobStream(image,2,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b L S B S i g n e d L o n g                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobLSBSignedLong() writes a signed value as a 32-bit quantity in
%  least-significant byte first order.
%
%  The format of the WriteBlobLSBSignedLong method is:
%
%      ssize_t WriteBlobLSBSignedLong(Image *image,const signed int value)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o value: Specifies the value to write.
%
*/
MagickExport ssize_t WriteBlobLSBSignedLong(Image *image,const signed int value)
{
  union
  {
    unsigned int
      unsigned_value;

    signed int
      signed_value;
  } quantum;

  unsigned char
    buffer[4];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  quantum.signed_value=value;
  buffer[0]=(unsigned char) quantum.unsigned_value;
  buffer[1]=(unsigned char) (quantum.unsigned_value >> 8);
  buffer[2]=(unsigned char) (quantum.unsigned_value >> 16);
  buffer[3]=(unsigned char) (quantum.unsigned_value >> 24);
  return(WriteBlobStream(image,4,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e B l o b L S B S i g n e d S h o r t                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobLSBSignedShort() writes a signed short value as a 16-bit quantity
%  in least-significant byte first order.
%
%  The format of the WriteBlobLSBSignedShort method is:
%
%      ssize_t WriteBlobLSBSignedShort(Image *image,const signed short value)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o value:  Specifies the value to write.
%
*/
MagickExport ssize_t WriteBlobLSBSignedShort(Image *image,
  const signed short value)
{
  union
  {
    unsigned short
      unsigned_value;

    signed short
      signed_value;
  } quantum;

  unsigned char
    buffer[2];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  quantum.signed_value=value;
  buffer[0]=(unsigned char) quantum.unsigned_value;
  buffer[1]=(unsigned char) (quantum.unsigned_value >> 8);
  return(WriteBlobStream(image,2,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b M S B L o n g                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobMSBLong() writes a unsigned int value as a 32-bit quantity in
%  most-significant byte first order.
%
%  The format of the WriteBlobMSBLong method is:
%
%      ssize_t WriteBlobMSBLong(Image *image,const unsigned int value)
%
%  A description of each parameter follows.
%
%    o value:  Specifies the value to write.
%
%    o image: the image.
%
*/
MagickExport ssize_t WriteBlobMSBLong(Image *image,const unsigned int value)
{
  unsigned char
    buffer[4];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  buffer[0]=(unsigned char) (value >> 24);
  buffer[1]=(unsigned char) (value >> 16);
  buffer[2]=(unsigned char) (value >> 8);
  buffer[3]=(unsigned char) value;
  return(WriteBlobStream(image,4,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e B l o b M S B S i g n e d S h o r t                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobMSBSignedShort() writes a signed short value as a 16-bit quantity
%  in most-significant byte first order.
%
%  The format of the WriteBlobMSBSignedShort method is:
%
%      ssize_t WriteBlobMSBSignedShort(Image *image,const signed short value)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o value:  Specifies the value to write.
%
*/
MagickExport ssize_t WriteBlobMSBSignedShort(Image *image,
  const signed short value)
{
  union
  {
    unsigned short
      unsigned_value;

    signed short
      signed_value;
  } quantum;

  unsigned char
    buffer[2];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  quantum.signed_value=value;
  buffer[0]=(unsigned char) (quantum.unsigned_value >> 8);
  buffer[1]=(unsigned char) quantum.unsigned_value;
  return(WriteBlobStream(image,2,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b M S B S h o r t                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobMSBShort() writes a unsigned short value as a 16-bit quantity in
%  most-significant byte first order.
%
%  The format of the WriteBlobMSBShort method is:
%
%      ssize_t WriteBlobMSBShort(Image *image,const unsigned short value)
%
%  A description of each parameter follows.
%
%   o  value:  Specifies the value to write.
%
%   o  file:  Specifies the file to write the data to.
%
*/
MagickExport ssize_t WriteBlobMSBShort(Image *image,const unsigned short value)
{
  unsigned char
    buffer[2];

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  buffer[0]=(unsigned char) (value >> 8);
  buffer[1]=(unsigned char) value;
  return(WriteBlobStream(image,2,buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b S t r i n g                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobString() write a string to a blob.  It returns the number of
%  characters written.
%
%  The format of the WriteBlobString method is:
%
%      ssize_t WriteBlobString(Image *image,const char *string)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o string: Specifies the string to write.
%
*/
MagickExport ssize_t WriteBlobString(Image *image,const char *string)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(string != (const char *) NULL);
  return(WriteBlobStream(image,strlen(string),(const unsigned char *) string));
}

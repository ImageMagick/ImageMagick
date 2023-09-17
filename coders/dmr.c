/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            DDDD   M   M  RRRR                               %
%                            D   D  MM MM  R   R                              %
%                            D   D  M M M  RRRR                               %
%                            D   D  M   M  R R                                %
%                            DDDD   M   M  R  R                               %
%                                                                             %
%                                                                             %
%              Get or put content to a Digital Media Repository.              %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                  May 2023                                   %
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
#include "MagickCore/client.h"
#include "MagickCore/colormap.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"
#include "MagickCore/xwindow-private.h"
#if defined(MAGICKCORE_DMR_DELEGATE)
#include <MagickCache/MagickCache.h>
#endif

#if defined(MAGICKCORE_DMR_DELEGATE)
/*
  Forward declarations.
*/
static MagickBooleanType
  WriteDMRImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D M R I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDMRImage() reads an image from a digital media repository and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadDMRImage method is:
%
%      Image *ReadDMRImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadDMRImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define DestroyDMRResources() \
{ \
  if (cache != (MagickCache *) NULL) \
    cache=DestroyMagickCache(cache); \
  if (message != (char *) NULL) \
    message=DestroyString(message); \
  if (passkey != (StringInfo *) NULL) \
    passkey=DestroyStringInfo(passkey); \
  if (passphrase != (StringInfo *) NULL) \
    passphrase=DestroyStringInfo(passphrase); \
  if (path != (char *) NULL) \
    path=DestroyString(path); \
  if (resource != (MagickCacheResource *) NULL) \
    resource=DestroyMagickCacheResource(resource); \
}   
#define ThrowDMRReadException() \
{ \
  DestroyDMRResources(); \
  if (image != (Image *) NULL) \
    image=DestroyImageList(image); \
  return(image); \
}   

  char
    *message = (char *) NULL,
    *path = (char *) NULL;

  const char
    *option;

  Image
    *image = (Image *) NULL;

  MagickBooleanType
    status;

  MagickCache
    *cache = (MagickCache *) NULL;

  MagickCacheResource
    *resource = (MagickCacheResource *) NULL;

  MagickCacheResourceType
    type;

  StringInfo
    *passkey = (StringInfo *) NULL,
    *passphrase = (StringInfo *) NULL;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  option=GetImageOption(image_info,"dmr:path");
  if (option != (const char *) NULL)
    path=ConstantString(option);
  option=GetImageOption(image_info,"dmr:passkey");
  if (option != (const char *) NULL)
    {
      passkey=FileToStringInfo(option,~0UL,exception);
      if (passkey == (StringInfo *) NULL)
        {
          message=GetExceptionMessage(errno);
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "unable to read passkey","`%s': %s",option,message);
          ThrowDMRReadException();
        }
    }
  option=GetImageOption(image_info,"dmr:passphrase");
  if (option != (const char *) NULL)
    {
      passphrase=FileToStringInfo(option,~0UL,exception);
      if (passphrase == (StringInfo *) NULL)
        {
          message=GetExceptionMessage(errno);
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "unable to read passphrase","`%s': %s",option,message);
          ThrowDMRReadException();
        }
    }
  cache=AcquireMagickCache(path,passkey);
  if (cache == (MagickCache *) NULL)
    {
      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "unable to open digital media repository","`%s': %s",path,message);
      ThrowDMRReadException();
    }
  resource=AcquireMagickCacheResource(cache,image_info->filename);
  type=GetMagickCacheResourceType(resource);
  image=(Image *) NULL;
  switch (type)
  {
    case ImageResourceType:
    {
      image=GetMagickCacheResourceImage(cache,resource,(const char *) NULL);
      if (image != (Image *) NULL)
        image=CloneImageList(image,exception);
      break;
    }
    case BlobResourceType:
    {
      ImageInfo
        *read_info;

      void *blob = GetMagickCacheResourceBlob(cache,resource);
      if (blob == (void *) NULL)
        break;
      read_info=AcquireImageInfo();
      image=BlobToImage(read_info,blob,GetMagickCacheResourceExtent(
        resource),exception);
      read_info=DestroyImageInfo(read_info);
      break;
		}
    case MetaResourceType:
    {
      const char *meta = GetMagickCacheResourceMeta(cache,resource);
      if (meta == (char *) NULL)
        break;
      image=AcquireImage(image_info,exception);
      (void) SetImageExtent(image,1,1,exception);
      (void) SetImageBackgroundColor(image,exception);
      (void) SetImageProperty(image,"dmr:meta",meta,exception);
      break;
    }
    default:
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "resource type not supported","`%s'",image_info->filename);
      ThrowDMRReadException();
    }
  }
  if (image == (Image *) NULL)
    { 
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "no such resource","`%s'",image_info->filename);
      ThrowDMRReadException();
    }
  if (passphrase != (StringInfo *) NULL)
    {
      status=PasskeyDecipherImage(image,passphrase,exception);
      if (status == MagickFalse)
        ThrowDMRReadException();
    }
  DestroyDMRResources();
  return(image);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r D M R I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterDMRImage() adds attributes for the Magick Cache repository format
%  to the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format supports
%  the saving of more than one frame to the same file or blob, whether the
%  format supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterDMRImage method is:
%
%      size_t RegisterDMRImage(void)
%
*/
ModuleExport size_t RegisterDMRImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("DMR","DMR","Digital Media Repository");
#if defined(MAGICKCORE_DMR_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadDMRImage;
  entry->encoder=(EncodeImageHandler *) WriteDMRImage;
#endif
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r D M R I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterDMRImage() removes format registrations made by the MCR module
%  from the list of supported formats.
%
%  The format of the UnregisterDMRImage method is:
%
%      UnregisterDMRImage(void)
%
*/
ModuleExport void UnregisterDMRImage(void)
{
  (void) UnregisterMagickInfo("DMR");
}

#if defined(MAGICKCORE_DMR_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e D M R I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteDMRImage() puts an image to the digital media repository.
%
%  The format of the WriteDMRImage method is:
%
%      MagickBooleanType WriteDMRImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteDMRImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
#define ThrowDMRWriteException() \
{ \
  DestroyDMRResources(); \
  return(MagickFalse); \
}   

  char
    *message = (char *) NULL,
    *path = (char *) NULL;

  const char
    *option;

  MagickBooleanType
    status;

  MagickCache
    *cache = (MagickCache *) NULL;

  MagickCacheResource
    *resource = (MagickCacheResource *) NULL;

  MagickCacheResourceType
    type;

  StringInfo
    *passkey = (StringInfo *) NULL,
    *passphrase = (StringInfo *) NULL;

  time_t
    ttl = 0;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  option=GetImageOption(image_info,"dmr:path");
  if (option != (const char *) NULL)
    path=ConstantString(option);
  option=GetImageOption(image_info,"dmr:passkey");
  if (option != (const char *) NULL)
    {
      passkey=FileToStringInfo(option,~0UL,exception);
      if (passkey == (StringInfo *) NULL)
        {
          message=GetExceptionMessage(errno);
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "unable to read passkey","`%s': %s",option,message);
          ThrowDMRWriteException();
        }
    }
  option=GetImageOption(image_info,"dmr:passphrase");
  if (option != (const char *) NULL)
    {
      passphrase=FileToStringInfo(option,~0UL,exception);
      if (passphrase == (StringInfo *) NULL)
        {
          message=GetExceptionMessage(errno);
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "unable to read passphrase","`%s': %s",option,message);
          ThrowDMRWriteException();
        }
    }
  option=GetImageOption(image_info,"dmr:ttl");
  if (option != (const char *) NULL)
    {
      char
        *q;

      /*
        Time to live, absolute or relative, e.g. 1440, 2 hours, 3 days, ...
      */
      ttl=(time_t) InterpretLocaleValue(option,&q);
      if (q != option)
        {
          while (isspace((int) ((unsigned char) *q)) != 0)
            q++;
          if (LocaleNCompare(q,"second",6) == 0)
            ttl*=1;
          if (LocaleNCompare(q,"minute",6) == 0)
            ttl*=60;
          if (LocaleNCompare(q,"hour",4) == 0)
            ttl*=3600;
          if (LocaleNCompare(q,"day",3) == 0)
            ttl*=86400;
          if (LocaleNCompare(q,"week",4) == 0)
            ttl*=604800;
          if (LocaleNCompare(q,"month",5) == 0)
            ttl*=2628000;
          if (LocaleNCompare(q,"year",4) == 0)
            ttl*=31536000;
        }
    }
  cache=AcquireMagickCache(path,passkey);
  if (cache == (MagickCache *) NULL)
    {
      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "unable to open digital media repository","`%s': %s",path,message);
      ThrowDMRWriteException();
    }
  resource=AcquireMagickCacheResource(cache,image_info->filename);
  SetMagickCacheResourceTTL(resource,ttl);
  type=GetMagickCacheResourceType(resource);
  switch (type)
  {
    case ImageResourceType:
    {
      Image
        *resource_image;

      resource_image=CloneImageList(image,exception);
      if (resource_image == (Image *) NULL)
        ThrowDMRWriteException();
      if (passphrase != (StringInfo *) NULL)
        {
          status=PasskeyEncipherImage(resource_image,passphrase,exception);
          if (status == MagickFalse)
            ThrowDMRWriteException();
        }
      status=PutMagickCacheResourceImage(cache,resource,resource_image);
      resource_image=DestroyImageList(resource_image);
      break;
    }
    case BlobResourceType:
    {
      size_t
        extent;

      ImageInfo *write_info = AcquireImageInfo();
      void *blob = ImageToBlob(write_info,image,&extent,exception);
      write_info=DestroyImageInfo(write_info);
      if (blob == (void *) NULL)
        ThrowDMRWriteException();
      status=PutMagickCacheResourceBlob(cache,resource,extent,blob);
      blob=RelinquishMagickMemory(blob);
      break;
    }
    case MetaResourceType:
    {
      const char *meta = GetImageProperty(image,"dmr:meta",exception);
      if (meta == (const char *) NULL)
        ThrowDMRWriteException();
      status=PutMagickCacheResourceMeta(cache,resource,meta);
      if (status == MagickFalse)
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "no such metadata","`%s'",image->filename);
      break;
    }
    default:
    { 
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "resource type not supported","`%s'",image->filename);
      ThrowDMRWriteException();
      break;
    }
  }
  if (status == MagickFalse)
    ThrowDMRWriteException();
  DestroyDMRResources();
  return(status);
}
#endif

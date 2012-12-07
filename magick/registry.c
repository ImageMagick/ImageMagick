/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           RRRR    EEEEE    GGG   IIIII  SSSSS  TTTTT  RRRR   Y   Y          %
%           R   R   E       G        I    SS       T    R   R   Y Y           %
%           RRRR    EEE     G GGG    I     SSS     T    RRRR     Y            %
%           R R     E       G   G    I       SS    T    R R      Y            %
%           R  R    EEEEE    GGG   IIIII  SSSSS    T    R  R     Y            %
%                                                                             %
%                                                                             %
%                       MagickCore Registry Methods                           %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 March 2000                                  %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/list.h"
#include "magick/memory_.h"
#include "magick/registry.h"
#include "magick/splay-tree.h"
#include "magick/string_.h"
#include "magick/utility.h"

/*
  Typedef declarations.
*/
typedef struct _RegistryInfo
{
  RegistryType
    type;

  void
    *value;

  size_t
    signature;
} RegistryInfo;

/*
  Static declarations.
*/
static SplayTreeInfo
  *registry = (SplayTreeInfo *) NULL;

static SemaphoreInfo
  *registry_semaphore = (SemaphoreInfo *) NULL;

static volatile MagickBooleanType
  instantiate_registry = MagickFalse;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e f i n e I m a g e R e g i s t r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefineImageRegistry() associates a key/value pair with the image registry.
%
%  The format of the DefineImageRegistry method is:
%
%      MagickBooleanType DefineImageRegistry(const RegistryType type,
%        const char *option,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o type: the type.
%
%    o option: the option.
%
%    o exception: the exception.
%
*/
MagickExport MagickBooleanType DefineImageRegistry(const RegistryType type,
  const char *option,ExceptionInfo *exception)
{
  char
    key[MaxTextExtent],
    value[MaxTextExtent];

  register char
    *p;

  assert(option != (const char *) NULL);
  (void) CopyMagickString(key,option,MaxTextExtent);
  for (p=key; *p != '\0'; p++)
    if (*p == '=')
      break;
  *value='\0';
  if (*p == '=')
    (void) CopyMagickString(value,p+1,MaxTextExtent);
  *p='\0';
  return(SetImageRegistry(type,key,value,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e I m a g e R e g i s t r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteImageRegistry() deletes a key from the image registry.
%
%  The format of the DeleteImageRegistry method is:
%
%      MagickBooleanType DeleteImageRegistry(const char *key)
%
%  A description of each parameter follows:
%
%    o key: the registry.
%
*/
MagickExport MagickBooleanType DeleteImageRegistry(const char *key)
{
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",key);
  if (registry == (void *) NULL)
    return(MagickFalse);
  return(DeleteNodeFromSplayTree(registry,key));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e R e g i s t r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageRegistry() returns a value associated with an image registry key.
%
%  The format of the GetImageRegistry method is:
%
%      void *GetImageRegistry(const RegistryType type,const char *key,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o type: the type.
%
%    o key: the key.
%
%    o exception: the exception.
%
*/
MagickExport void *GetImageRegistry(const RegistryType type,const char *key,
  ExceptionInfo *exception)
{
  void
    *value;

  RegistryInfo
    *registry_info;

  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",key);
  if (registry == (void *) NULL)
    return((void *) NULL);
  registry_info=(RegistryInfo *) GetValueFromSplayTree(registry,key);
  if (registry_info == (void *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),RegistryError,
        "UnableToGetRegistryID","`%s'",key);
      return((void *) NULL);
    }
  value=(void *) NULL;
  switch (type)
  {
    case ImageRegistryType:
    {
      if (type == registry_info->type)
        value=(void *) CloneImageList((Image *) registry_info->value,exception);
      break;
    }
    case ImageInfoRegistryType:
    {
      if (type == registry_info->type)
        value=(void *) CloneImageInfo((ImageInfo *) registry_info->value);
      break;
    }
    case StringRegistryType:
    {
      switch (registry_info->type)
      {
        case ImageRegistryType:
        {
          value=(Image *) ConstantString(((Image *)
            registry_info->value)->filename);
          break;
        }
        case ImageInfoRegistryType:
        {
          value=(Image *) ConstantString(((ImageInfo *)
            registry_info->value)->filename);
          break;
        }
        case StringRegistryType:
        {
          value=(void *) ConstantString((char *) registry_info->value);
          break;
        }
        default:
          break;
      }
      break;
    }
    default:
      break;
  }
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t I m a g e R e g i s t r y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextImageRegistry() gets the next image registry value.
%
%  The format of the GetNextImageRegistry method is:
%
%      char *GetNextImageRegistry(void)
%
*/
MagickExport char *GetNextImageRegistry(void)
{
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (registry == (void *) NULL)
    return((char *) NULL);
  return((char *) GetNextKeyInSplayTree(registry));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e g i s t r y C o m p o n e n t G e n e s i s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegistryComponentGenesis() instantiates the registry component.
%
%  The format of the RegistryComponentGenesis method is:
%
%      MagickBooleanType RegistryComponentGenesis(void)
%
*/
MagickExport MagickBooleanType RegistryComponentGenesis(void)
{
  AcquireSemaphoreInfo(&registry_semaphore);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t r y C o m p o n e n t T e r m i n u s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegistryComponentTerminus() destroys the registry component.
%
%  The format of the DestroyDefines method is:
%
%      void RegistryComponentTerminus(void)
%
*/
MagickExport void RegistryComponentTerminus(void)
{
  if (registry_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&registry_semaphore);
  LockSemaphoreInfo(registry_semaphore);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (registry != (void *) NULL)
    registry=DestroySplayTree(registry);
  instantiate_registry=MagickFalse;
  UnlockSemaphoreInfo(registry_semaphore);
  DestroySemaphoreInfo(&registry_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e I m a g e R e g i s t r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveImageRegistry() removes a key from the image registry and returns its
%  value.
%
%  The format of the RemoveImageRegistry method is:
%
%      void *RemoveImageRegistry(const char *key)
%
%  A description of each parameter follows:
%
%    o key: the registry.
%
*/
MagickExport void *RemoveImageRegistry(const char *key)
{
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",key);
  if (registry == (void *) NULL)
    return((void *) NULL);
  return(RemoveNodeFromSplayTree(registry,key));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t I m a g e R e g i s t r y I t e r a t o r                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetImageRegistryIterator() resets the registry iterator.  Use it in
%  conjunction with GetNextImageRegistry() to iterate over all the values
%  in the image registry.
%
%  The format of the ResetImageRegistryIterator method is:
%
%      ResetImageRegistryIterator(void)
%
*/
MagickExport void ResetImageRegistryIterator(void)
{
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (registry == (void *) NULL)
    return;
  ResetSplayTreeIterator(registry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e R e g i s t r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageRegistry() associates a value with an image registry key.
%
%  The format of the SetImageRegistry method is:
%
%      MagickBooleanType SetImageRegistry(const RegistryType type,
%        const char *key,const void *value,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o type: the type.
%
%    o key: the key.
%
%    o value: the value.
%
%    o exception: the exception.
%
*/

static void *DestroyRegistryNode(void *registry_info)
{
  register RegistryInfo
    *p;

  p=(RegistryInfo *) registry_info;
  switch (p->type)
  {
    case StringRegistryType:
    default:
    {
      p->value=RelinquishMagickMemory(p->value);
      break;
    }
    case ImageRegistryType:
    {
      p->value=(void *) DestroyImageList((Image *) p->value);
      break;
    }
    case ImageInfoRegistryType:
    {
      p->value=(void *) DestroyImageInfo((ImageInfo *) p->value);
      break;
    }
  }
  return(RelinquishMagickMemory(p));
}

MagickExport MagickBooleanType SetImageRegistry(const RegistryType type,
  const char *key,const void *value,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  RegistryInfo
    *registry_info;

  void
    *clone_value;

  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",key);
  clone_value=(void *) NULL;
  switch (type)
  {
    case StringRegistryType:
    default:
    {
      const char
        *string;

      string=(const char *) value;
      clone_value=(void *) ConstantString(string);
      break;
    }
    case ImageRegistryType:
    {
      const Image
        *image;

      image=(const Image *) value;
      if (image->signature != MagickSignature)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),RegistryError,
            "UnableToSetRegistry","%s",key);
          return(MagickFalse);
        }
      clone_value=(void *) CloneImageList(image,exception);
      break;
    }
    case ImageInfoRegistryType:
    {
      const ImageInfo
        *image_info;

      image_info=(const ImageInfo *) value;
      if (image_info->signature != MagickSignature)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),RegistryError,
            "UnableToSetRegistry","%s",key);
          return(MagickFalse);
        }
      clone_value=(void *) CloneImageInfo(image_info);
      break;
    }
  }
  if (clone_value == (void *) NULL)
    return(MagickFalse);
  registry_info=(RegistryInfo *) AcquireMagickMemory(sizeof(*registry_info));
  if (registry_info == (RegistryInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(registry_info,0,sizeof(*registry_info));
  registry_info->type=type;
  registry_info->value=clone_value;
  registry_info->signature=MagickSignature;
  if ((registry == (SplayTreeInfo *) NULL) &&
      (instantiate_registry == MagickFalse))
    {
      if (registry_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&registry_semaphore);
      LockSemaphoreInfo(registry_semaphore);
      if ((registry == (SplayTreeInfo *) NULL) &&
          (instantiate_registry == MagickFalse))
        {
          registry=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
            DestroyRegistryNode);
          instantiate_registry=MagickTrue;
        }
      UnlockSemaphoreInfo(registry_semaphore);
    }
  status=AddValueToSplayTree(registry,ConstantString(key),registry_info);
  return(status);
}

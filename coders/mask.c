/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         M   M   AAA   SSSSS  K   K                          %
%                         MM MM  A   A  SS     K  K                           %
%                         M M M  AAAAA   SSS   KKK                            %
%                         M   M  A   A     SS  K  K                           %
%                         M   M  A   A  SSSSS  K   K                          %
%                                                                             %
%                                                                             %
%                              Write Mask File.                               %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
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

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteMASKImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d M A S K I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadMASKImage returns the image mask associated with the image.
%
%  The format of the ReadMASKImage method is:
%
%      Image *ReadMASKImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadMASKImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *read_info;

  /*
    Initialize Image structure.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) CopyMagickString(read_info->magick,"MIFF",MagickPathExtent);
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image != (Image *) NULL)
    {
      MagickBooleanType
        status;

      status=GrayscaleImage(image,image->intensity,exception);
      if (status == MagickFalse)
        image=DestroyImage(image);
    }
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M A S K I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterMASKImage() adds attributes for the MASK image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterMASKImage method is:
%
%      size_t RegisterMASKImage(void)
%
*/
ModuleExport size_t RegisterMASKImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("MASK","MASK","Image Clip Mask");
  entry->decoder=(DecodeImageHandler *) ReadMASKImage;
  entry->encoder=(EncodeImageHandler *) WriteMASKImage;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r M A S K I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterMASKImage() removes format registrations made by the
%  MASK module from the list of supported formats.
%
%  The format of the UnregisterMASKImage method is:
%
%      UnregisterMASKImage(void)
%
*/
ModuleExport void UnregisterMASKImage(void)
{
  (void) UnregisterMagickInfo("MASK");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M A S K I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteMASKImage() writes an image mask to a file.
%
%  The format of the WriteMASKImage method is:
%
%      MagickBooleanType WriteMASKImage(const ImageInfo *image_info,
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

static Image *MaskImage(const Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view,
    *mask_view;

  Image
    *mask_image;

  MagickBooleanType
    status;

  ssize_t
    y;

  mask_image=CloneImage(image,0,0,MagickTrue,exception);
  if (mask_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(mask_image,DirectClass,exception) == MagickFalse)
    {
      mask_image=DestroyImage(mask_image);
      return((Image *) NULL);
    }
  mask_image->alpha_trait=UndefinedPixelTrait;
  (void) SetImageColorspace(mask_image,GRAYColorspace,exception);
  /*
    Mask image.
  */
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  mask_view=AcquireAuthenticCacheView(mask_image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(mask_view,0,y,mask_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelChannel(mask_image,GrayPixelChannel,0,q);
      SetPixelChannel(mask_image,GrayPixelChannel,GetPixelWriteMask(image,p),q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(mask_image);
    }
    if (SyncCacheViewAuthenticPixels(mask_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  mask_view=DestroyCacheView(mask_view);
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    mask_image=DestroyImage(mask_image);
  return(mask_image);
}

static MagickBooleanType WriteMASKImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  Image
    *mask_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  mask_image=MaskImage(image,exception);
  if (mask_image == (Image *) NULL)
    return(MagickFalse);
  (void) CopyMagickString(mask_image->filename,image->filename,
    MagickPathExtent);
  write_info=CloneImageInfo(image_info);
  *write_info->magick='\0';
  (void) SetImageInfo(write_info,1,exception);
  if ((*write_info->magick == '\0') ||
      (LocaleCompare(write_info->magick,"MASK") == 0))
    (void) FormatLocaleString(mask_image->filename,MagickPathExtent,"miff:%s",
      write_info->filename);
  status=WriteImage(write_info,mask_image,exception);
  mask_image=DestroyImage(mask_image);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     BBBB    AAA   Y   Y  EEEEE  RRRR                        %
%                     B   B  A   A   Y Y   E      R   R                       %
%                     BBBB   AAAAA    Y    EEE    RRRR                        %
%                     B   B  A   A    Y    E      R  R                        %
%                     BBBB   A   A    Y    EEEEE  R   R                       %
%                                                                             %
%                                                                             %
%                     Read/Write Raw Bayer Image Format                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 2020                                   %
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
#include "MagickCore/cache.h"
#include "MagickCore/channel.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
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
#include "MagickCore/registry.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"
#include "MagickWand/MagickWand.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteBAYERImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d B A Y E R I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBAYERImage() reads an image of raw BAYER (RGGB) samples and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadBAYERImage method is:
%
%      Image *ReadBAYERImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadBAYERImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define BayerRGGBImage "bayer:moasic"
#define BayerRGBImage "bayer:rgb"

  char
    **arguments,
    command[MagickPathExtent] =  /* http://im.snibgo.com/demosaic.htm */
      "mpr:" BayerRGGBImage " "
      "( -clone 0 -define sample:offset=25 -sample 50% ) "
      "( -clone 0 "
      "  ( -clone 0 -define sample:offset=75x25 -sample 50% ) "
      "  ( -clone 0 -define sample:offset=25x75 -sample 50% ) "
      "  -delete 0 -evaluate-sequence mean ) "
      "( -clone 0 -define sample:offset=75 -sample 50% ) "
      "-delete 0 -combine -resize 200% -colorspace sRGB "
      "mpr:" BayerRGBImage;

  Image
    *image;

  ImageInfo
    *read_info;

  int
    number_arguments;

  MagickBooleanType
    status;

  ssize_t
    i;

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
  read_info=CloneImageInfo(image_info);
  (void) CopyMagickString(read_info->magick,"GRAY",MagickPathExtent);
  read_info->verbose=MagickFalse;
  image=ReadImage(read_info,exception);
  if (image == (Image *) NULL)
    return(image);
  status=SetImageRegistry(ImageRegistryType,BayerRGGBImage,image,exception);
  if (status == MagickFalse)
    {
      read_info=DestroyImageInfo(read_info);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  arguments=StringToArgv(command,&number_arguments);
  if (arguments == (char **) NULL)
    {
      read_info=DestroyImageInfo(read_info);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  status=MagickImageCommand(read_info,number_arguments,arguments,(char **) NULL,
    exception);
  (void) DeleteImageRegistry(BayerRGGBImage);
  for (i=0; i < (ssize_t) number_arguments; i++)
    arguments[i]=DestroyString(arguments[i]);
  arguments=(char **) RelinquishMagickMemory(arguments);
  read_info=DestroyImageInfo(read_info);
  image=DestroyImage(image);
  if (status == MagickFalse)
    return((Image *) NULL);
  image=GetImageRegistry(ImageRegistryType,BayerRGBImage,exception);
  if (image == (Image *) NULL)
    return(image);
  (void) DeleteImageRegistry(BayerRGBImage);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r B A Y E R I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterBAYERImage() adds attributes for the BAYER image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterBAYERImage method is:
%
%      size_t RegisterBAYERImage(void)
%
*/
ModuleExport size_t RegisterBAYERImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("BAYER","BAYER","Raw mosaiced samples");
  entry->decoder=(DecodeImageHandler *) ReadBAYERImage;
  entry->encoder=(EncodeImageHandler *) WriteBAYERImage;
  entry->flags|=CoderRawSupportFlag;
  entry->flags|=CoderEndianSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("BAYER","BAYERA","Raw mosaiced and alpha samples");
  entry->decoder=(DecodeImageHandler *) ReadBAYERImage;
  entry->encoder=(EncodeImageHandler *) WriteBAYERImage;
  entry->flags|=CoderRawSupportFlag;
  entry->flags|=CoderEndianSupportFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r B A Y E R I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterBAYERImage() removes format registrations made by the BAYER module
%  from the list of supported formats.
%
%  The format of the UnregisterBAYERImage method is:
%
%      UnregisterBAYERImage(void)
%
*/
ModuleExport void UnregisterBAYERImage(void)
{
  (void) UnregisterMagickInfo("BAYERA");
  (void) UnregisterMagickInfo("BAYER");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e B A Y E R I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBAYERImage() writes an image to a file in the BAYER, BAYERAlpha
%  rasterfile format.
%
%  The format of the WriteBAYERImage method is:
%
%      MagickBooleanType WriteBAYERImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteBAYERImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  /*
    Allocate memory for pixels.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  /*
    Mosaic processing (near future).
  */
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->magick,"GRAY",MagickPathExtent);
  status=WriteImage(write_info,image,exception);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

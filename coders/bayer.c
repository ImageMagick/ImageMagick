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
  Global defines.
*/
#define BayerBImage "bayer:B"
#define BayerG0Image "bayer:G0"
#define BayerG1Image "bayer:G1"
#define BayerRGBImage "bayer:rgb"
#define BayerRGGBImage "bayer:moasic"

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
%  ReadBAYERImage() reconstructs a full color image from the incomplete color
%  samples output from an image sensor overlaid with a color filter array
%  (CFA).  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  Reference: http://im.snibgo.com/demosaic.htm.
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
static Image *BayerSample(const Image *image, const char *offset,
  RectangleInfo geometry, ExceptionInfo *exception)
{
  Image
    *clone,
    *sample;

  clone=CloneImage(image,0,0,MagickTrue,exception);
  if (clone == (Image *) NULL)
    return(clone);
  (void) SetImageArtifact(clone,"sample:offset",offset);
  sample=SampleImage(clone,geometry.width,geometry.height,exception);
  clone=DestroyImage(clone);
  return(sample);
}

static Image *ReadBAYERImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image,
    *imageA,
    *imageB;

  ImageInfo
    *read_info;

  RectangleInfo
    geometry;

  /*
    Reconstruct a full color image from the incomplete camera sensor.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  read_info=CloneImageInfo(image_info);
  (void) FormatLocaleString(read_info->filename,MagickPathExtent,"gray:%.1024s",
    image_info->filename);
  (void) CopyMagickString(read_info->magick,"GRAY",MagickPathExtent);
  read_info->verbose=MagickFalse;
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  (void) ParseRegionGeometry(image,"50%",&geometry,exception);
  imageA=BayerSample(image,"75x25",geometry,exception);
  if (imageA == (Image *) NULL)
    return(DestroyImage(image));
  imageB=BayerSample(image,"25x75",geometry,exception);
  if (imageB == (Image *) NULL)
    {
      imageA=DestroyImage(imageA);
      return(DestroyImage(image));
    }
  AppendImageToList(&imageA,imageB);
  imageB=EvaluateImages(imageA,MeanEvaluateOperator,exception);
  imageA=DestroyImageList(imageA);
  imageA=BayerSample(image,"25",geometry,exception);
  if (imageA == (Image *) NULL)
    {
      imageB=DestroyImage(imageB);
      return(DestroyImage(image));
    }
  AppendImageToList(&imageA,imageB);
  imageB=BayerSample(image,"75",geometry,exception);
  if (imageB == (Image *) NULL)
    {
      imageA=DestroyImageList(imageA);
      return(DestroyImage(image));
    }
  AppendImageToList(&imageA,imageB);
  imageB=CombineImages(imageA,sRGBColorspace,exception);
  imageA=DestroyImageList(imageA);
  if (imageB == (Image *) NULL)
    return(DestroyImage(image));
  (void) ParseRegionGeometry(imageB,"200%",&geometry,exception);
  imageA=ResizeImage(imageB,geometry.width,geometry.height,image->filter,
    exception);
  imageB=DestroyImageList(imageB);
  if (imageA == (Image *) NULL)
    return(DestroyImage(image));
  (void) CopyMagickString(imageA->magick,image_info->magick,
    MagickPathExtent);
  (void) CopyMagickString(imageA->filename,image_info->filename,
    MagickPathExtent);
  image=DestroyImageList(image);
  return(imageA);
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
  entry->flags|=CoderRawSupportFlag | CoderEndianSupportFlag |
    CoderDecoderThreadSupportFlag | CoderEncoderThreadSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("BAYER","BAYERA","Raw mosaiced and alpha samples");
  entry->decoder=(DecodeImageHandler *) ReadBAYERImage;
  entry->encoder=(EncodeImageHandler *) WriteBAYERImage;
  entry->flags|=CoderRawSupportFlag | CoderEndianSupportFlag |
    CoderDecoderThreadSupportFlag | CoderEncoderThreadSupportFlag;
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
%  WriteBAYERImage() deconstructs a full color image into a single channel
%  RGGB raw image format.
%
%  Reference: http://im.snibgo.com/mosaic.htm.
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
  char
    **arguments;

  const char
    *command =
      "-size 2x2 xc:black "
      "-fill White -draw \"point 1,0\" "
      "-fill None "
      "+write mpr:" BayerG0Image " "
      "-roll +0+1 +write mpr:" BayerBImage " "
      "-roll -1+0 +write mpr:" BayerG1Image " "
      "+delete "
      "mpr:" BayerRGBImage " "
      "-colorspace sRGB "
      "-channel RGB "
      "-separate "
      "-compose Over "
      "( -clone 0,1 "
      "  ( +clone "
      "    -tile mpr:" BayerG0Image " "
      "    -draw \"color 0,0 reset\" "
      "  ) "
      "  -alpha off -composite "
      ") "
      "( -clone 3,1 "
      "  ( +clone "
      "    -tile mpr:" BayerG1Image " "
      "    -draw \"color 0,0 reset\" "
      "  ) "
      "  -alpha off -composite "
      ") "
      "( -clone 4,2 "
      "  ( +clone "
      "    -tile mpr:" BayerBImage " "
      "    -draw \"color 0,0 reset\" "
      "  ) "
      "  -alpha off -composite "
      ") "
      "-delete 0--2 "
      "mpr:" BayerRGGBImage;

  Image
    *bayer_image;

  ImageInfo
    *write_info;

  int
    number_arguments;

  MagickBooleanType
    status;

  ssize_t
    i;

  /*
    Deconstruct RGB image into a single channel RGGB raw image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=SetImageRegistry(ImageRegistryType,BayerRGBImage,image,exception);
  if (status == MagickFalse)
    return(MagickFalse);
  arguments=StringToArgv(command,&number_arguments);
  if (arguments == (char **) NULL)
    {
      (void) DeleteImageRegistry(BayerRGBImage);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  write_info=CloneImageInfo(image_info);
  write_info->verbose=MagickFalse;
  status=MagickImageCommand(write_info,number_arguments,arguments,
    (char **) NULL,exception);
  (void) DeleteImageRegistry(BayerG1Image);
  (void) DeleteImageRegistry(BayerG0Image);
  (void) DeleteImageRegistry(BayerBImage);
  (void) DeleteImageRegistry(BayerRGBImage);
  for (i=0; i < (ssize_t) number_arguments; i++)
    arguments[i]=DestroyString(arguments[i]);
  arguments=(char **) RelinquishMagickMemory(arguments);
  if (status == MagickFalse)
    {
      write_info=DestroyImageInfo(write_info);
      (void) DeleteImageRegistry(BayerRGGBImage);
      return(status);
    }
  bayer_image=GetImageRegistry(ImageRegistryType,BayerRGGBImage,exception);
  (void) DeleteImageRegistry(BayerRGGBImage);
  if (bayer_image != (Image *) NULL)
    {
      (void) CopyMagickString(write_info->magick,"GRAY",MagickPathExtent);
      (void) CopyMagickString(bayer_image->filename,image->filename,
        MagickPathExtent);
      status=WriteImage(write_info,bayer_image,exception);
      bayer_image=DestroyImage(bayer_image);
      write_info=DestroyImageInfo(write_info);
    }
  return(status);
}

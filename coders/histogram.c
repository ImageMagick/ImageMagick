/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%       H   H  IIIII  SSSSS  TTTTT   OOO    GGGG  RRRR    AAA   M   M         %
%       H   H    I    SS       T    O   O  G      R   R  A   A  MM MM         %
%       HHHHH    I     SSS     T    O   O  G  GG  RRRR   AAAAA  M M M         %
%       H   H    I       SS    T    O   O  G   G  R R    A   A  M   M         %
%       H   H  IIIII  SSSSS    T     OOO    GGG   R  R   A   A  M   M         %
%                                                                             %
%                                                                             %
%                          Write A Histogram Image.                           %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/histogram.h"
#include "MagickCore/image-private.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteHISTOGRAMImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r H I S T O G R A M I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterHISTOGRAMImage() adds attributes for the Histogram image format
%  to the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterHISTOGRAMImage method is:
%
%      size_t RegisterHISTOGRAMImage(void)
%
*/
ModuleExport size_t RegisterHISTOGRAMImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("HISTOGRAM","HISTOGRAM","Histogram of the image");
  entry->encoder=(EncodeImageHandler *) WriteHISTOGRAMImage;
  entry->flags^=CoderAdjoinFlag;
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r H I S T O G R A M I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterHISTOGRAMImage() removes format registrations made by the
%  HISTOGRAM module from the list of supported formats.
%
%  The format of the UnregisterHISTOGRAMImage method is:
%
%      UnregisterHISTOGRAMImage(void)
%
*/
ModuleExport void UnregisterHISTOGRAMImage(void)
{
  (void) UnregisterMagickInfo("HISTOGRAM");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H I S T O G R A M I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteHISTOGRAMImage() writes an image to a file in Histogram format.
%  The image shows a histogram of the color (or gray) values in the image.  The
%  image consists of three overlaid histograms:  a red one for the red channel,
%  a green one for the green channel, and a blue one for the blue channel.  The
%  image comment contains a list of unique pixel values and the number of times
%  each occurs in the image.
%
%  This method is strongly based on a similar one written by
%  muquit@warm.semcor.com which in turn is based on ppmhistmap of netpbm.
%
%  The format of the WriteHISTOGRAMImage method is:
%
%      MagickBooleanType WriteHISTOGRAMImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteHISTOGRAMImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
#define HistogramDensity  "256x200"

  char
    filename[MagickPathExtent];

  const char
    *option;

  const MagickInfo
    *magick_info;

  Image
    *histogram_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  PixelInfo
    *histogram;

  double
    maximum,
    scale;

  RectangleInfo
    geometry;

  const Quantum
    *p;

  Quantum
    *q,
    *r;

  ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  /*
    Allocate histogram image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  SetGeometry(image,&geometry);
  if (image_info->density == (char *) NULL)
    (void) ParseAbsoluteGeometry(HistogramDensity,&geometry);
  else
    (void) ParseAbsoluteGeometry(image_info->density,&geometry);
  histogram_image=CloneImage(image,geometry.width,geometry.height,MagickTrue,
    exception);
  if (histogram_image == (Image *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) SetImageStorageClass(histogram_image,DirectClass,exception);
  /*
    Allocate histogram count arrays.
  */
  length=MagickMax((size_t) ScaleQuantumToChar(QuantumRange)+1UL,
    histogram_image->columns);
  histogram=(PixelInfo *) AcquireQuantumMemory(length,sizeof(*histogram));
  if (histogram == (PixelInfo *) NULL)
    {
      histogram_image=DestroyImage(histogram_image);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Initialize histogram count arrays.
  */
  (void) memset(histogram,0,length*sizeof(*histogram));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToChar(GetPixelRed(image,p))].red++;
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToChar(GetPixelGreen(image,p))].green++;
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        histogram[ScaleQuantumToChar(GetPixelBlue(image,p))].blue++;
      p+=GetPixelChannels(image);
    }
  }
  maximum=histogram[0].red;
  for (x=0; x < (ssize_t) histogram_image->columns; x++)
  {
    if (((GetPixelRedTraits(image) & UpdatePixelTrait) != 0) &&
        (maximum < histogram[x].red))
      maximum=histogram[x].red;
    if (((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0) &&
        (maximum < histogram[x].green))
      maximum=histogram[x].green;
    if (((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0) &&
        (maximum < histogram[x].blue))
      maximum=histogram[x].blue;
  }
  scale=0.0;
  if (fabs((double) maximum) >= MagickEpsilon)
    scale=(double) histogram_image->rows/maximum;
  /*
    Initialize histogram image.
  */
  (void) QueryColorCompliance("#000000",AllCompliance,
    &histogram_image->background_color,exception);
  (void) SetImageBackgroundColor(histogram_image,exception);
  for (x=0; x < (ssize_t) histogram_image->columns; x++)
  {
    q=GetAuthenticPixels(histogram_image,x,0,1,histogram_image->rows,exception);
    if (q == (Quantum *) NULL)
      break;
    if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
      {
        y=CastDoubleToLong(ceil((double) histogram_image->rows-scale*
          histogram[x].red-0.5));
        r=q+y*(ssize_t) GetPixelChannels(histogram_image);
        for ( ; y < (ssize_t) histogram_image->rows; y++)
        {
          SetPixelRed(histogram_image,QuantumRange,r);
          r+=GetPixelChannels(histogram_image);
        }
      }
    if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
      {
        y=CastDoubleToLong(ceil((double) histogram_image->rows-scale*
          histogram[x].green-0.5));
        r=q+y*(ssize_t) GetPixelChannels(histogram_image);
        for ( ; y < (ssize_t) histogram_image->rows; y++)
        {
          SetPixelGreen(histogram_image,QuantumRange,r);
          r+=GetPixelChannels(histogram_image);
        }
      }
    if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
      {
        y=CastDoubleToLong(ceil((double) histogram_image->rows-scale*
          histogram[x].blue-0.5));
        r=q+y*(ssize_t) GetPixelChannels(histogram_image);
        for ( ; y < (ssize_t) histogram_image->rows; y++)
        {
          SetPixelBlue(histogram_image,QuantumRange,r);
          r+=GetPixelChannels(histogram_image);
        }
      }
    if (SyncAuthenticPixels(histogram_image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      histogram_image->rows);
    if (status == MagickFalse)
      break;
  }
  histogram=(PixelInfo *) RelinquishMagickMemory(histogram);
  option=GetImageOption(image_info,"histogram:unique-colors");
  if ((IsHistogramImage(image,exception) != MagickFalse) || 
      (IsStringTrue(option) != MagickFalse) ||
      (GetImageOption(image_info,"format") != (const char *) NULL))
    {
      FILE
        *file;

      int
        unique_file;

      /*
        Add a unique colors as an image comment.
      */
      file=(FILE *) NULL;
      unique_file=AcquireUniqueFileResource(filename);
      if (unique_file != -1)
        file=fdopen(unique_file,"wb");
      if ((unique_file != -1) && (file != (FILE *) NULL))
        {
          char
            *property;

          (void) GetNumberColors(image,file,exception);
          (void) fclose(file);
          property=FileToString(filename,~0UL,exception);
          if (property != (char *) NULL)
            {
              (void) SetImageProperty(histogram_image,"comment",property,
                exception);
              property=DestroyString(property);
            }
        }
      (void) RelinquishUniqueFileResource(filename);
    }
  /*
    Write Histogram image.
  */
  (void) CopyMagickString(histogram_image->filename,image_info->filename,
    MagickPathExtent);
  (void) ResetImagePage(histogram_image,"0x0+0+0");
  write_info=CloneImageInfo(image_info);
  *write_info->magick='\0';
  (void) SetImageInfo(write_info,1,exception);
  magick_info=GetMagickInfo(write_info->magick,exception);
  if ((magick_info == (const MagickInfo*) NULL) ||
      (LocaleCompare(magick_info->magick_module,"HISTOGRAM") == 0))
    (void) FormatLocaleString(histogram_image->filename,MagickPathExtent,
      "miff:%s",write_info->filename);
  status=WriteImage(write_info,histogram_image,exception);
  histogram_image=DestroyImage(histogram_image);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

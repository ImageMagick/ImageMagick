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
%                                John Cristy                                  %
%                                 July 1992                                   %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/histogram.h"
#include "magick/image-private.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/token.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteHISTOGRAMImage(const ImageInfo *,Image *);

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

  entry=SetMagickInfo("HISTOGRAM");
  entry->encoder=(EncodeImageHandler *) WriteHISTOGRAMImage;
  entry->adjoin=MagickFalse;
  entry->format_type=ImplicitFormatType;
  entry->description=ConstantString("Histogram of the image");
  entry->module=ConstantString("HISTOGRAM");
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
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static MagickBooleanType WriteHISTOGRAMImage(const ImageInfo *image_info,
  Image *image)
{
#define HistogramDensity  "256x200"

  ChannelType
    channel;

  char
    filename[MaxTextExtent];

  const char
    *option;

  ExceptionInfo
    *exception;

  Image
    *histogram_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  MagickPixelPacket
    *histogram;

  MagickRealType
    maximum,
    scale;

  RectangleInfo
    geometry;

  register const PixelPacket
    *p;

  register PixelPacket
    *q,
    *r;

  register ssize_t
    x;

  size_t
    length;

  ssize_t
    y;

  /*
    Allocate histogram image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  SetGeometry(image,&geometry);
  if (image_info->density == (char *) NULL)
    (void) ParseAbsoluteGeometry(HistogramDensity,&geometry);
  else
    (void) ParseAbsoluteGeometry(image_info->density,&geometry);
  histogram_image=CloneImage(image,geometry.width,geometry.height,MagickTrue,
    &image->exception);
  if (histogram_image == (Image *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) SetImageStorageClass(histogram_image,DirectClass);
  /*
    Allocate histogram count arrays.
  */
  length=MagickMax((size_t) ScaleQuantumToChar(QuantumRange)+1UL,
    histogram_image->columns);
  histogram=(MagickPixelPacket *) AcquireQuantumMemory(length,
    sizeof(*histogram));
  if (histogram == (MagickPixelPacket *) NULL)
    {
      histogram_image=DestroyImage(histogram_image);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Initialize histogram count arrays.
  */
  channel=image_info->channel;
  (void) ResetMagickMemory(histogram,0,length*sizeof(*histogram));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        histogram[ScaleQuantumToChar(GetPixelRed(p))].red++;
      if ((channel & GreenChannel) != 0)
        histogram[ScaleQuantumToChar(GetPixelGreen(p))].green++;
      if ((channel & BlueChannel) != 0)
        histogram[ScaleQuantumToChar(GetPixelBlue(p))].blue++;
      p++;
    }
  }
  maximum=histogram[0].red;
  for (x=0; x < (ssize_t) histogram_image->columns; x++)
  {
    if (((channel & RedChannel) != 0) && (maximum < histogram[x].red))
      maximum=histogram[x].red;
    if (((channel & GreenChannel) != 0) && (maximum < histogram[x].green))
      maximum=histogram[x].green;
    if (((channel & BlueChannel) != 0) && (maximum < histogram[x].blue))
      maximum=histogram[x].blue;
  }
  scale=(MagickRealType) histogram_image->rows/maximum;
  /*
    Initialize histogram image.
  */
  exception=(&image->exception);
  (void) QueryColorDatabase("#000",&histogram_image->background_color,
    &image->exception);
  (void) SetImageBackgroundColor(histogram_image);
  for (x=0; x < (ssize_t) histogram_image->columns; x++)
  {
    q=GetAuthenticPixels(histogram_image,x,0,1,histogram_image->rows,exception);
    if (q == (PixelPacket *) NULL)
      break;
    if ((channel & RedChannel) != 0)
      {
        y=(ssize_t) ceil(histogram_image->rows-scale*histogram[x].red-0.5);
        r=q+y;
        for ( ; y < (ssize_t) histogram_image->rows; y++)
        {
          SetPixelRed(r,QuantumRange);
          r++;
        }
      }
    if ((channel & GreenChannel) != 0)
      {
        y=(ssize_t) ceil(histogram_image->rows-scale*histogram[x].green-0.5);
        r=q+y;
        for ( ; y < (ssize_t) histogram_image->rows; y++)
        {
          SetPixelGreen(r,QuantumRange);
          r++;
        }
      }
    if ((channel & BlueChannel) != 0)
      {
        y=(ssize_t) ceil(histogram_image->rows-scale*histogram[x].blue-0.5);
        r=q+y;
        for ( ; y < (ssize_t) histogram_image->rows; y++)
        {
          SetPixelBlue(r,QuantumRange);
          r++;
        }
      }
    if (SyncAuthenticPixels(histogram_image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,SaveImageTag,y,histogram_image->rows);
    if (status == MagickFalse)
      break;
  }
  /*
    Relinquish resources.
  */
  histogram=(MagickPixelPacket *) RelinquishMagickMemory(histogram);
  option=GetImageOption(image_info,"histogram:unique-colors");
  if ((option == (const char *) NULL) || (IsMagickTrue(option) != MagickFalse))
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

          (void) GetNumberColors(image,file,&image->exception);
          (void) fclose(file);
          property=FileToString(filename,~0UL,&image->exception);
          if (property != (char *) NULL)
            {
              (void) SetImageProperty(histogram_image,"comment",property);
              property=DestroyString(property);
            }
        }
      (void) RelinquishUniqueFileResource(filename);
    }
  /*
    Write Histogram image.
  */
  (void) CopyMagickString(histogram_image->filename,image_info->filename,
    MaxTextExtent);
  write_info=CloneImageInfo(image_info);
  (void) SetImageInfo(write_info,1,&image->exception);
  if (LocaleCompare(write_info->magick,"HISTOGRAM") == 0)
    (void) FormatLocaleString(histogram_image->filename,MaxTextExtent,
      "miff:%s",write_info->filename);
  status=WriteImage(write_info,histogram_image);
  histogram_image=DestroyImage(histogram_image);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

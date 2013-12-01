/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                 PPPP   L       AAA   SSSSS  M   M   AAA                     %
%                 P   P  L      A   A  SS     MM MM  A   A                    %
%                 PPPP   L      AAAAA   SSS   M M M  AAAAA                    %
%                 P      L      A   A     SS  M   M  A   A                    %
%                 P      LLLLL  A   A  SSSSS  M   M  A   A                    %
%                                                                             %
%                                                                             %
%                          Read a Plasma Image.                               %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/channel.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/fx.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/random_.h"
#include "magick/random-private.h"
#include "magick/signature-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P L A S M A I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPlasmaImage creates a plasma fractal image.  The image is
%  initialized to the X server color as specified by the filename.
%
%  The format of the ReadPlasmaImage method is:
%
%      Image *ReadPlasmaImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline void PlasmaPixel(Image *image,RandomInfo *random_info,double x,
  double y)
{
  ExceptionInfo
    *exception;

  register PixelPacket
    *q;

  exception=(&image->exception);
  q=GetAuthenticPixels(image,(ssize_t) ceil(x-0.5),(ssize_t) ceil(y-0.5),1,1,
    exception);
  if (q == (PixelPacket *) NULL)
    return;
  SetPixelRed(q,ScaleShortToQuantum((unsigned short) (65535.0*
    GetPseudoRandomValue(random_info)+0.5)));
  SetPixelGreen(q,ScaleShortToQuantum((unsigned short) (65535.0*
    GetPseudoRandomValue(random_info)+0.5)));
  SetPixelBlue(q,ScaleShortToQuantum((unsigned short) (65535.0*
    GetPseudoRandomValue(random_info)+0.5)));
  (void) SyncAuthenticPixels(image,exception);
}

static Image *ReadPlasmaImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  register ssize_t
    x;

  register PixelPacket
    *q;

  register size_t
    i;

  SegmentInfo
    segment_info;

  size_t
    depth,
    max_depth;

  ssize_t
    y;

  /*
    Recursively apply plasma to the image.
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) FormatLocaleString(read_info->filename,MaxTextExtent,
    "gradient:%s",image_info->filename);
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  image->storage_class=DirectClass;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelOpacity(q,QuantumRange/2);
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  segment_info.x1=0;
  segment_info.y1=0;
  segment_info.x2=(double) image->columns-1;
  segment_info.y2=(double) image->rows-1;
  if (LocaleCompare(image_info->filename,"fractal") == 0)
    {
      RandomInfo
        *random_info;

      /*
        Seed pixels before recursion.
      */
      random_info=AcquireRandomInfo();
      PlasmaPixel(image,random_info,segment_info.x1,segment_info.y1);
      PlasmaPixel(image,random_info,segment_info.x1,(segment_info.y1+
        segment_info.y2)/2);
      PlasmaPixel(image,random_info,segment_info.x1,segment_info.y2);
      PlasmaPixel(image,random_info,(segment_info.x1+segment_info.x2)/2,
        segment_info.y1);
      PlasmaPixel(image,random_info,(segment_info.x1+segment_info.x2)/2,
        (segment_info.y1+segment_info.y2)/2);
      PlasmaPixel(image,random_info,(segment_info.x1+segment_info.x2)/2,
        segment_info.y2);
      PlasmaPixel(image,random_info,segment_info.x2,segment_info.y1);
      PlasmaPixel(image,random_info,segment_info.x2,(segment_info.y1+
        segment_info.y2)/2);
      PlasmaPixel(image,random_info,segment_info.x2,segment_info.y2);
      random_info=DestroyRandomInfo(random_info);
    }
  i=(size_t) MagickMax(image->columns,image->rows)/2;
  for (max_depth=0; i != 0; max_depth++)
    i>>=1;
  for (depth=1; ; depth++)
  {
    if (PlasmaImage(image,&segment_info,0,depth) != MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) depth,
      max_depth);
    if (status == MagickFalse)
      break;
  }
  (void) SetImageAlphaChannel(image,SetAlphaChannel);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P L A S M A I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPLASMAImage() adds attributes for the Plasma image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPLASMAImage method is:
%
%      size_t RegisterPLASMAImage(void)
%
*/
ModuleExport size_t RegisterPLASMAImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("PLASMA");
  entry->decoder=(DecodeImageHandler *) ReadPlasmaImage;
  entry->adjoin=MagickFalse;
  entry->format_type=ImplicitFormatType;
  entry->description=ConstantString("Plasma fractal image");
  entry->module=ConstantString("PLASMA");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("FRACTAL");
  entry->decoder=(DecodeImageHandler *) ReadPlasmaImage;
  entry->adjoin=MagickFalse;
  entry->format_type=ImplicitFormatType;
  entry->description=ConstantString("Plasma fractal image");
  entry->module=ConstantString("PLASMA");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P L A S M A I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPLASMAImage() removes format registrations made by the
%  PLASMA module from the list of supported formats.
%
%  The format of the UnregisterPLASMAImage method is:
%
%      UnregisterPLASMAImage(void)
%
*/
ModuleExport void UnregisterPLASMAImage(void)
{
  (void) UnregisterMagickInfo("FRACTAL");
  (void) UnregisterMagickInfo("PLASMA");
}

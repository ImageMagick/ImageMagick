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
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/visual-effects.h"

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

static inline MagickBooleanType PlasmaPixel(Image *image,
  RandomInfo *magick_restrict random_info,const double x,const double y,
  ExceptionInfo *exception)
{
  Quantum
    *q;

  q=GetAuthenticPixels(image,(ssize_t) (x+0.5),(ssize_t) (y+0.5),1,1,
    exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  SetPixelRed(image,(Quantum) ((double) QuantumRange*GetPseudoRandomValue(random_info)+
    0.5),q);
  SetPixelGreen(image,(Quantum) ((double) QuantumRange*GetPseudoRandomValue(random_info)+
    0.5),q);
  SetPixelBlue(image,(Quantum) ((double) QuantumRange*GetPseudoRandomValue(random_info)+
    0.5),q);
  return(SyncAuthenticPixels(image,exception));
}

static Image *ReadPlasmaImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *read_info;

  MagickStatusType
    status;

  ssize_t
    x;

  Quantum
    *q;

  size_t
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
  (void) FormatLocaleString(read_info->filename,MagickPathExtent,
    "gradient:%s",image_info->filename);
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageStorageClass(image,DirectClass,exception);
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) SetImageColorspace(image,sRGBColorspace,exception); 
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelAlpha(image,QuantumRange/2,q);
      q+=(ptrdiff_t) GetPixelChannels(image);
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
      (void) SetImageColorspace(image,sRGBColorspace,exception);
      random_info=AcquireRandomInfo();
      status=PlasmaPixel(image,random_info,segment_info.x1,segment_info.y1,
        exception);
      status&=(MagickStatusType) PlasmaPixel(image,random_info,segment_info.x1,
        (segment_info.y1+segment_info.y2)/2,exception);
      status&=(MagickStatusType) PlasmaPixel(image,random_info,segment_info.x1,
        segment_info.y2,exception);
      status&=(MagickStatusType) PlasmaPixel(image,random_info,(segment_info.x1+        segment_info.x2)/2,segment_info.y1,exception);
      status&=(MagickStatusType) PlasmaPixel(image,random_info,(segment_info.x1+
        segment_info.x2)/2,(segment_info.y1+segment_info.y2)/2,exception);
      status&=(MagickStatusType) PlasmaPixel(image,random_info,(segment_info.x1+
        segment_info.x2)/2,segment_info.y2,exception);
      status&=(MagickStatusType) PlasmaPixel(image,random_info,segment_info.x2,
        segment_info.y1,exception);
      status&=(MagickStatusType) PlasmaPixel(image,random_info,segment_info.x2,
        (segment_info.y1+segment_info.y2)/2,exception);
      status&=(MagickStatusType) PlasmaPixel(image,random_info,segment_info.x2,
        segment_info.y2,exception);
      random_info=DestroyRandomInfo(random_info);
      if (status == MagickFalse)
        return(image);  
    }
  i=(size_t) MagickMax(image->columns,image->rows)/2;
  for (max_depth=0; i != 0; max_depth++)
    i>>=1;
  for (depth=1; ; depth++)
  {
    if (PlasmaImage(image,&segment_info,0,depth,exception) != MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) depth,
      max_depth);
    if (status == MagickFalse)
      break;
  }
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

  entry=AcquireMagickInfo("PLASMA","PLASMA","Plasma fractal image");
  entry->decoder=(DecodeImageHandler *) ReadPlasmaImage;
  entry->flags^=CoderAdjoinFlag;
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PLASMA","FRACTAL","Plasma fractal image");
  entry->decoder=(DecodeImageHandler *) ReadPlasmaImage;
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

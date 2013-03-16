/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  SSSSS  PPPP    AAA   RRRR   SSSSS  EEEEE                   %
%                  SS     P   P  A   A  R   R  SS     E                       %
%                   SSS   PPPP   AAAAA  RRRR    SSS   EEE                     %
%                      S  P      A   A  R R       SS  E                       %
%                  SSSSS  P      A   A  R  R   SSSSS  EEEEE                   %
%                                                                             %
%                      CCCC   OOO   L       OOO   RRRR                        %
%                     C      O   O  L      O   O  R   R                       %
%                     C      O   O  L      O   O  RRRR                        %
%                     C      O   O  L      O   O  R R                         %
%                      CCCC   OOO   LLLLL   OOO   R  R                        %
%                                                                             %
%                                                                             %
%                     Write Pixels in Sparse Color Format                     %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                March 2013                                   %
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
#include "MagickCore/studio.h"
#include "MagickCore/annotate.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteSPARSE_COLORImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S P A R S E _ C O L O R I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSPARSE_COLORImage() adds attributes for the SPARSE_COLOR image
%  format.
%
%  The format of the RegisterSPARSE_COLORImage method is:
%
%      size_t RegisterSPARSE_COLORImage(void)
%
*/
ModuleExport size_t RegisterSPARSE_COLORImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("SPARSE-COLOR");
  entry->encoder=(EncodeImageHandler *) WriteSPARSE_COLORImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->stealth=MagickTrue;
  entry->description=ConstantString("Sparse-Color");
  entry->module=ConstantString("SPARSE-COLOR");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S P A R S E _ C O L O R I m a g e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterSPARSE_COLORImage() removes format registrations made by the
%  SPARSE_COLOR module from the list of supported format.
%
%  The format of the UnregisterSPARSE_COLORImage method is:
%
%      UnregisterSPARSE_COLORImage(void)
%
*/
ModuleExport void UnregisterSPARSE_COLORImage(void)
{
  (void) UnregisterMagickInfo("SPARSE-COLOR");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S P A R S E _ C O L O R I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteSPARSE_COLORImage writes the pixel values as text numbers.
%
%  The format of the WriteSPARSE_COLORImage method is:
%
%      MagickBooleanType WriteSPARSE_COLORImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteSPARSE_COLORImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  char
    buffer[MaxTextExtent],
    colorspace[MaxTextExtent],
    tuple[MaxTextExtent];

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  PixelInfo
    pixel;

  register const Quantum
    *p;

  register ssize_t
    x;

  ssize_t
    y;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  do
  {
    (void) CopyMagickString(colorspace,CommandOptionToMnemonic(
      MagickColorspaceOptions,(ssize_t) image->colorspace),MaxTextExtent);
    LocaleLower(colorspace);
    image->depth=GetImageQuantumDepth(image,MagickTrue);
    if (image->alpha_trait == BlendPixelTrait)
      (void) ConcatenateMagickString(colorspace,"a",MaxTextExtent);
    GetPixelInfo(image,&pixel);
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        GetPixelInfoPixel(image,p,&pixel);
        if (pixel.colorspace == LabColorspace)
          {
            pixel.green-=(QuantumRange+1)/2.0;
            pixel.blue-=(QuantumRange+1)/2.0;
          }
        /*
          Sparse-color format.
        */
        if (GetPixelAlpha(image,p) == (Quantum) OpaqueAlpha)
          {
            (void) QueryColorname(image,&pixel,SVGCompliance,tuple,
              exception);
            (void) FormatLocaleString(buffer,MaxTextExtent,"%.20g,%.20g,",
              (double) x,(double) y);
            (void) WriteBlobString(image,buffer);
            (void) WriteBlobString(image,tuple);
            (void) WriteBlobString(image," ");
          }
        p+=GetPixelChannels(image);
      }
      status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
        image->rows);
      if (status == MagickFalse)
        break;
    }
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}

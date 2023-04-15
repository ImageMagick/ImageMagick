/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  K   K  EEEEE  RRRR   N   N  EEEEE  L                       %
%                  K  K   E      R   R  NN  N  E      L                       %
%                  KKK    EEE    RRRR   N N N  EEE    L                       %
%                  K  K   E      R R    N  NN  E      L                       %
%                  K   K  EEEEE  R  R   N   N  EEEEE  LLLLL                   %
%                                                                             %
%                                                                             %
%                     Write the Morphology Kernel Format                      %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                October 2020                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 2020 ImageMagick Studio LLC, a non-profit organization         %
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
%  Adapted from http://im.snibgo.com/customim.htm#img2knl.c.
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
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

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteKERNELImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r K E R N E L I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterKERNELImage() adds attributes for the KERNEL image format to the
%  list of supported formats.  The attributes include the image format tag, a
%  method to read and/or write the format, whether the format supports the
%  saving of more than one frame to the same file or blob, whether the format
%  supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterKERNELImage method is:
%
%      size_t RegisterKERNELImage(void)
%
*/
ModuleExport size_t RegisterKERNELImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("KERNEL","KERNEL","Morphology Kernel");
  entry->encoder=(EncodeImageHandler *) WriteKERNELImage;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r K E R N E L I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterKERNELImage() removes format registrations made by the
%  KERNEL module from the list of supported formats.
%
%  The format of the UnregisterKERNELImage method is:
%
%      UnregisterKERNELImage(void)
%
*/
ModuleExport void UnregisterKERNELImage(void)
{
  (void) UnregisterMagickInfo("KERNEL");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e K E R N E L I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteKERNELImage() writes an image to a file in KERNEL image format.
%
%  The format of the WriteKERNELImage method is:
%
%      MagickBooleanType WriteKERNELImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteKERNELImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent];

  MagickBooleanType
    status;

  ssize_t
    y;
 
  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  /*
    Write KERNEL header.
  */
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  (void) FormatLocaleString(buffer,MagickPathExtent,"%gx%g:",(double)
    image->columns,(double) image->rows);
  (void) WriteBlobString(image,buffer);
  /*
    Convert MIFF to KERNEL raster pixels.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if ((x != 0) || (y != 0))
        (void) WriteBlobString(image,",");
      if (((image->alpha_trait != BlendPixelTrait) != 0) &&
          (GetPixelAlpha(image,p) < OpaqueAlpha/2))
        (void) WriteBlobString(image,"-");
      else
        {
          (void) FormatLocaleString(buffer,MagickPathExtent,"%.*g",
            GetMagickPrecision(),QuantumScale*GetPixelIntensity(image,p));
          (void) WriteBlobString(image,buffer);
        }
      p+=GetPixelChannels(image);
    }
    if (image->previous == (Image *) NULL)
      {
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
  }
  (void) WriteBlobString(image,"\n");
  (void) CloseBlob(image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%            GGGG  RRRR    AAA   DDDD   IIIII  EEEEE  N   N  TTTTT            %
%           G      R   R  A   A  D   D    I    E      NN  N    T              %
%           G  GG  RRRR   AAAAA  D   D    I    EEE    N N N    T              %
%           G   G  R R    A   A  D   D    I    E      N  NN    T              %
%            GGG   R  R   A   A  DDDD   IIIII  EEEEE  N   N    T              %
%                                                                             %
%                                                                             %
%                   Read An Image Filled Using Gradient.                      %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/channel.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/studio.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d G R A D I E N T I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadGRADIENTImage creates a gradient image and initializes it to
%  the color range as specified by the filename.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the ReadGRADIENTImage method is:
%
%      Image *ReadGRADIENTImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Image *ReadXCImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  PixelInfo
    pixel;

  register ssize_t
    x;

  register Quantum
    *q;

  ssize_t
    y;

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
  image=AcquireImage(image_info,exception);
  if (image->columns == 0)
    image->columns=1;
  if (image->rows == 0)
    image->rows=1;
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  (void) CopyMagickString(image->filename,image_info->filename,
    MagickPathExtent);
  if (*image_info->filename == '\0')
    pixel=image->background_color;
  else
    {
      status=QueryColorCompliance((char *) image_info->filename,AllCompliance,
        &pixel,exception);
      if (status == MagickFalse)
        {
          image=DestroyImage(image);
          return((Image *) NULL);
        }
    }
  (void) SetImageColorspace(image,pixel.colorspace,exception);
  image->alpha_trait=pixel.alpha_trait;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelViaPixelInfo(image,&pixel,q);
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  return(GetFirstImageInList(image));
}

static Image *ReadGRADIENTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    start_color[MagickPathExtent],
    stop_color[MagickPathExtent];

  Image
    *image;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  StopInfo
    *stops;

  /*
    Identify start and stop gradient colors.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  (void) CopyMagickString(start_color,"white",MagickPathExtent);
  (void) CopyMagickString(stop_color,"black",MagickPathExtent);
  if (*image_info->filename != '\0')
    {
      register char
        *p;

      (void) CopyMagickString(start_color,image_info->filename,
        MagickPathExtent);
      for (p=start_color; (*p != '-') && (*p != '\0'); p++)
        if (*p == '(')
          {
            for (p++; (*p != ')') && (*p != '\0'); p++);
            if (*p == '\0')
              break;
          }
      if (*p == '-')
        (void) CopyMagickString(stop_color,p+1,MagickPathExtent);
      *p='\0';
    }
  /*
    Create base gradient image from start color.
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) CopyMagickString(read_info->filename,start_color,MagickPathExtent);
  image=ReadXCImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Create gradient stops.
  */
  stops=(StopInfo *) AcquireQuantumMemory(2,sizeof(*stops));
  if (stops == (StopInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  stops[0].offset=0.0;
  stops[1].offset=1.0;
  status=QueryColorCompliance(start_color,AllCompliance,&stops[0].color,
    exception);
  if (status != MagickFalse)
    status=QueryColorCompliance(stop_color,AllCompliance,&stops[1].color,
      exception);
  if (status == MagickFalse)
    {
      stops=(StopInfo *) RelinquishMagickMemory(stops);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  (void) SetImageColorspace(image,stops[0].color.colorspace,exception);
  if ((stops[0].color.alpha_trait != UndefinedPixelTrait) ||
      (stops[1].color.alpha_trait != UndefinedPixelTrait))
    SetImageAlpha(image,TransparentAlpha,exception);
  /*
    Paint gradient.
  */
  status=GradientImage(image,LocaleCompare(image_info->magick,"GRADIENT") == 0 ?
    LinearGradient : RadialGradient,PadSpread,stops,2,exception);
  stops=(StopInfo *) RelinquishMagickMemory(stops);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r G R A D I E N T I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterGRADIENTImage() adds attributes for the GRADIENT image format
%  to the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterGRADIENTImage method is:
%
%      size_t RegisterGRADIENTImage(void)
%
*/
ModuleExport size_t RegisterGRADIENTImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("GRADIENT","GRADIENT",
    "Gradual linear passing from one shade to another");
  entry->decoder=(DecodeImageHandler *) ReadGRADIENTImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderRawSupportFlag;
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("GRADIENT","RADIAL-GRADIENT",
    "Gradual radial passing from one shade to another");
  entry->decoder=(DecodeImageHandler *) ReadGRADIENTImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderRawSupportFlag;
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r G R A D I E N T I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterGRADIENTImage() removes format registrations made by the
%  GRADIENT module from the list of supported formats.
%
%  The format of the UnregisterGRADIENTImage method is:
%
%      UnregisterGRADIENTImage(void)
%
*/
ModuleExport void UnregisterGRADIENTImage(void)
{
  (void) UnregisterMagickInfo("RADIAL-GRADIENT");
  (void) UnregisterMagickInfo("GRADIENT");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        JJJJJ  SSSSS   OOO   N   N                           %
%                          J    SS     O   O  NN  N                           %
%                          J     SSS   O   O  N N N                           %
%                        J J       SS  O   O  N  NN                           %
%                        JJJ    SSSSS   OOO   N   N                           %
%                                                                             %
%                                                                             %
%                  Write Info About the Image in JSON Format.                 %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                January 2014                                 %
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
#include "magick/artifact.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/colorspace.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/identify.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-accessor.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteJSONImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r J S O N I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterJSONImage() adds attributes for the JSON image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterJSONImage method is:
%
%      size_t RegisterJSONImage(void)
%
*/
ModuleExport size_t RegisterJSONImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("JSON");
  entry->encoder=(EncodeImageHandler *) WriteJSONImage;
  entry->blob_support=MagickFalse;
  entry->description=ConstantString("The image format and characteristics");
  entry->module=ConstantString("JSON");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r J S O N I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterJSONImage() removes format registrations made by the
%  JSON module from the list of supported formats.
%
%  The format of the UnregisterJSONImage method is:
%
%      UnregisterJSONImage(void)
%
*/
ModuleExport void UnregisterJSONImage(void)
{
  (void) UnregisterMagickInfo("JSON");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e J S O N I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteJSONImage writes the pixel values as text numbers.
%
%  The format of the WriteJSONImage method is:
%
%      MagickBooleanType WriteJSONImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteJSONImage(const ImageInfo *image_info,
  Image *image)
{
  const char
    *format;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  do
  {
    format=GetImageOption(image_info,"format");
    if (format == (char *) NULL)
      {
        (void) CopyMagickString(image->filename,image->magick_filename,
          MaxTextExtent);
        image->magick_columns=image->columns;
        image->magick_rows=image->rows;
        (void) IdentifyImage(image,GetBlobFileHandle(image),
          image_info->verbose);
      }
    else
      {
        char
          *text;

        text=InterpretImageProperties(image_info,image,format);
        if (text != (char *) NULL)
          {
            (void) WriteBlobString(image,text);
            text=DestroyString(text);
          }
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

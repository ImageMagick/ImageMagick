/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                          CCCC  22222  PPPP    AAA                           %
%                         C         22  P   P  A   A                          %
%                         C       22    PPPP   AAAAA                          %
%                         C      22     P      A   A                          %
%                          CCCC  22222  P      A   A                          %
%                                                                             %
%                                                                             %
%                          C2PA Provenance Metadata.                          %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                              September 2026                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
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
#include "MagickCore/option.h"
#include "MagickCore/property.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C 2 P A I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ReadC2PAImage() reads an image that is signed with C2PA content
% credentials.  The image is handed to the c2pa:decode delegate which
% validates the manifest and returns an image ImageMagick natively
% understands.  It allocates the memory necessary for the new Image structure
% and returns a pointer to the new image.
%
%  The format of the ReadC2PAImage method is:
%
%      Image *ReadC2PAImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadC2PAImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    *json,
    json_filename[MagickPathExtent];

  Image
    *image;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

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
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) CloseBlob(image);
  /*
    Leverage delegate to create the image manifest.
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  read_info->temporary=MagickFalse;
  (void) CopyMagickString(image->filename,image_info->filename,
    MagickPathExtent);
  status=InvokeDelegate(read_info,image,"c2pa:decode",(char *) NULL,exception);
  (void) FormatLocaleString(json_filename,MagickPathExtent,"%s.json",
    read_info->unique);
  read_info=DestroyImageInfo(read_info);
  image=DestroyImageList(image);
  if (status == MagickFalse)
    return((Image *) NULL);
  /*
    Read image.
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  *read_info->magick='\0';
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    return(image);
  json=FileToString(json_filename,~0UL,exception);
  if (json == (char *) NULL)
    return(image);
  (void) SetImageProperty(image,"c2pa:manifest",json,exception);
  json=DestroyString(json);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r C 2 P A I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterC2PAImage() adds attributes for the C2PA image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterC2PAImage method is:
%
%      size_t RegisterC2PAImage(void)
%
*/
ModuleExport size_t RegisterC2PAImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("C2PA","C2PA","C2PA Provenance Metadata");
  entry->decoder=(DecodeImageHandler *) ReadC2PAImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->format_type=ExplicitFormatType;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r C 2 P A I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterC2PAImage() removes format registrations made by the
%  C2PA module from the list of supported formats.
%
%  The format of the UnregisterC2PAImage method is:
%
%      UnregisterC2PAImage(void)
%
*/
ModuleExport void UnregisterC2PAImage(void)
{
  (void) UnregisterMagickInfo("C2PA");
}

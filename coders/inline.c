/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  IIIII  N   N  L      IIIII  N   N  EEEEE                   %
%                    I    NN  N  L        I    NN  N  E                       %
%                    I    N N N  L        I    N N N  EEE                     %
%                    I    N  NN  L        I    N  NN  E                       %
%                  IIIII  N   N  LLLLL  IIIII  N   N  EEEEE                   %
%                                                                             %
%                                                                             %
%                            Read Inline Images                               %
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
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/client.h"
#include "magick/display.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/option.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"
#include "magick/xwindow.h"
#include "magick/xwindow-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d I N L I N E I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadINLINEImage() reads base64-encoded inlines images.
%
%  The format of the ReadINLINEImage method is:
%
%      Image *ReadINLINEImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static Image *ReadINLINEImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  register size_t
    i;

  size_t
    quantum;

  ssize_t
    count;

  unsigned char
    *inline_image;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (LocaleNCompare(image_info->filename,"data:",5) == 0)
    return(ReadInlineImage(image_info,image_info->filename,exception));
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  quantum=MagickMin((size_t) GetBlobSize(image),MagickMaxBufferExtent);
  inline_image=(unsigned char *) AcquireQuantumMemory(quantum,
    sizeof(*inline_image));
  count=0;
  for (i=0; inline_image != (unsigned char *) NULL; i+=count)
  {
    count=(ssize_t) ReadBlob(image,quantum,inline_image+i);
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
    if (~((size_t) i) < (quantum+1))
      {
        inline_image=(unsigned char *) RelinquishMagickMemory(inline_image);
        break;
      }
    inline_image=(unsigned char *) ResizeQuantumMemory(inline_image,i+quantum+1,
      sizeof(*inline_image));
  }
  if (inline_image == (unsigned char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return((Image *) NULL);
    }
  inline_image[i+count]='\0';
  image=DestroyImageList(image);
  image=ReadInlineImage(image_info,(char *) inline_image,exception);
  inline_image=(unsigned char *) RelinquishMagickMemory(inline_image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r I N L I N E I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterINLINEImage() adds attributes for the INLINE image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterINLINEImage method is:
%
%      size_t RegisterINLINEImage(void)
%
*/
ModuleExport size_t RegisterINLINEImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("INLINE");
  entry->decoder=(DecodeImageHandler *) ReadINLINEImage;
  entry->format_type=ImplicitFormatType;
  entry->description=ConstantString("Base64-encoded inline images");
  entry->module=ConstantString("INLINE");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r I N L I N E I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterINLINEImage() removes format registrations made by the
%  INLINE module from the list of supported formats.
%
%  The format of the UnregisterINLINEImage method is:
%
%      UnregisterINLINEImage(void)
%
*/
ModuleExport void UnregisterINLINEImage(void)
{
  (void) UnregisterMagickInfo("INLINE");
}

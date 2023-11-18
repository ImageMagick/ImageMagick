/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  SSSSS  TTTTT  RRRR   IIIII  M   M   GGGG                   %
%                  SS       T    R   R    I    MM MM  G                       %
%                   SSS     T    RRRR     I    M M M  G  GG                   %
%                     SS    T    R  R     I    M   M  G   G                   %
%                  SSSSS    T    R   R  IIIII  M   M   GGG                    %
%                                                                             %
%                                                                             %
%                    String to Image and Image to Strings                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 March 2022                                  %
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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
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
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteSTRIMGImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S T R I M G I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadSTRIMGImage() encodes the filename as an image.  It allocates the
%  memory necessary for the new Image structure and returns a pointer to the
%  new image.
%
%  The format of the ReadSTRIMGImage method is:
%
%      Image *ReadSTRIMGImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadSTRIMGImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    *string;

  Image
    *image;

  MagickBooleanType
    status;

  Quantum
    *q;

  ssize_t
    x;

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
  string=InterpretImageProperties((ImageInfo *) image_info,image,
    image_info->filename,exception);
  if (string == (char *) NULL)
    return(DestroyImageList(image));
  image->depth=8;
  image->colorspace=GRAYColorspace;
  image->columns=strlen(string);
  image->rows=1;
  if (image_info->ping != MagickFalse)
    {
      string=DestroyString(string);
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      string=DestroyString(string);
      (void) CloseBlob(image);
      return(DestroyImageList(image));
    }
  /*
    Convert string to an image.
  */
  q=QueueAuthenticPixels(image,0,0,image->columns,1,exception);
  if (q == (Quantum *) NULL)
    {
      string=DestroyString(string);
      (void) CloseBlob(image);
      return(DestroyImageList(image));
    }
  for (x=0; x < (ssize_t) image->columns; x++)
    q[x]=ScaleCharToQuantum((unsigned char) string[x]);
  string=DestroyString(string);
  if (SyncAuthenticPixels(image,exception) == MagickFalse)
    {
      (void) CloseBlob(image);
      return(DestroyImageList(image));
    }
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S T R I M G I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSTRIMGImage() adds attributes for the STRIMG image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSTRIMGImage method is:
%
%      size_t RegisterSTRIMGImage(void)
%
*/
ModuleExport size_t RegisterSTRIMGImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("STRIMG","STRIMG","String to image and back");
  entry->decoder=(DecodeImageHandler *) ReadSTRIMGImage;
  entry->encoder=(EncodeImageHandler *) WriteSTRIMGImage;
  entry->flags|=CoderRawSupportFlag;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S T R I M G I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterSTRIMGImage() removes format registrations made by the
%  STRIMG module from the list of supported formats.
%
%  The format of the UnregisterSTRIMGImage method is:
%
%      UnregisterSTRIMGImage(void)
%
*/
ModuleExport void UnregisterSTRIMGImage(void)
{
  (void) UnregisterMagickInfo("STRIMG");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S T R I M G I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteSTRIMGImage() writes an image as a string.
%
%  The format of the WriteSTRIMGImage method is:
%
%      MagickBooleanType WriteSTRIMGImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteSTRIMGImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  ssize_t
    y;

  unsigned char
    *pixels;

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
    Convert image to a string.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  if (SetQuantumDepth(image,quantum_info,8) == MagickFalse)
    {
      quantum_info=DestroyQuantumInfo(quantum_info);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *p;

    size_t
      length;

    ssize_t
      count;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      GrayQuantum,pixels,exception);
    count=WriteBlob(image,length,pixels);
    if (count != (ssize_t) length)
      break;
    if (image->previous == (Image *) NULL)
      {
        status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
          image->rows);
        if (status == MagickFalse)
          break;
      }
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
  if (y < (ssize_t) image->rows)
    ThrowWriterException(CorruptImageError,"UnableToWriteImageData");
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

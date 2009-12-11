/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              SSSSS  TTTTT  EEEEE   GGGG   AAA   N   N   OOO                 %
%              SS       T    E      G      A   A  NN  N  O   O                %
%               SSS     T    EEE    G  GG  AAAAA  N N N  O   O                %
%                 SS    T    E      G   G  A   A  N  NN  O   O                %
%              SSSSS    T    EEEEE   GGG   A   A  N   N   OOO                 %
%                                                                             %
%                                                                             %
%                       Write A Steganographic Image.                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S T E G A N O I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadSTEGANOImage() reads a steganographic image hidden within another
%  image type.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadSTEGANOImage method is:
%
%      Image *ReadSTEGANOImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline unsigned long MagickMin(const unsigned long x,
  const unsigned long y)
{
  if (x < y)
    return(x);
  return(y);
}

static Image *ReadSTEGANOImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define GetBit(alpha,i) MagickMin((((unsigned long) (alpha) >> (unsigned long) \
  (i)) & 0x01),16)
#define SetBit(alpha,i,set) (alpha)=(IndexPacket) ((set) != 0 ? \
  (unsigned long) (alpha) | (1UL << (unsigned long) (i)) : (unsigned long) \
  (alpha) & ~(1UL << (unsigned long) (i)))

  Image
    *image,
    *watermark;

  ImageInfo
    *read_info;

  long
    c,
    i,
    j,
    k,
    y;

  MagickBooleanType
    status;

  PixelPacket
    pixel;

  register IndexPacket
    *indexes;

  register long
    x;

  register PixelPacket
    *q;

  unsigned long
    depth;

  /*
    Initialize Image structure.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(OptionError,"MustSpecifyImageSize");
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  *read_info->magick='\0';
  watermark=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (watermark == (Image *) NULL)
    return((Image *) NULL);
  watermark->depth=MAGICKCORE_QUANTUM_DEPTH;
  if (AcquireImageColormap(image,MaxColormapSize) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  /*
    Get hidden watermark from low-order bits of image.
  */
  c=0;
  i=0;
  j=0;
  i=MAGICKCORE_QUANTUM_DEPTH-1;
  depth=MAGICKCORE_QUANTUM_DEPTH;
  for (k=image->offset; (i >= 0) && (j < (long) depth); i--)
  {
    for (y=0; (y < (long) image->rows) && (j < (long) depth); y++)
    {
      x=0;
      for (; (x < (long) image->columns) && (j < (long) depth); x++)
      {
        if ((k/(long) watermark->columns) >= (long) watermark->rows)
          break;
        (void) GetOneVirtualPixel(watermark,k % (long) watermark->columns,
          k/(long) watermark->columns,&pixel,exception);
        q=GetAuthenticPixels(image,x,y,1,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indexes=GetAuthenticIndexQueue(image);
        switch (c)
        {
          case 0:
          {
            SetBit(*indexes,i,GetBit(pixel.red,j));
            break;
          }
          case 1:
          {
            SetBit(*indexes,i,GetBit(pixel.green,j));
            break;
          }
          case 2:
          {
            SetBit(*indexes,i,GetBit(pixel.blue,j));
            break;
          }
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        c++;
        if (c == 3)
          c=0;
        k++;
        if (k == (long) (watermark->columns*watermark->columns))
          k=0;
        if (k == image->offset)
          j++;
      }
    }
    status=SetImageProgress(image,LoadImagesTag,i,depth);
    if (status == MagickFalse)
      break;
  }
  watermark=DestroyImage(watermark);
  (void) SyncImage(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S T E G A N O I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSTEGANOImage() adds attributes for the STEGANO image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSTEGANOImage method is:
%
%      unsigned long RegisterSTEGANOImage(void)
%
*/
ModuleExport unsigned long RegisterSTEGANOImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("STEGANO");
  entry->decoder=(DecodeImageHandler *) ReadSTEGANOImage;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Steganographic image");
  entry->module=ConstantString("STEGANO");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S T E G A N O I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterSTEGANOImage() removes format registrations made by the
%  STEGANO module from the list of supported formats.
%
%  The format of the UnregisterSTEGANOImage method is:
%
%      UnregisterSTEGANOImage(void)
%
*/
ModuleExport void UnregisterSTEGANOImage(void)
{
  (void) UnregisterMagickInfo("STEGANO");
}

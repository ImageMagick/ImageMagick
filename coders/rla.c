/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            RRRR   L       AAA                               %
%                            R   R  L      A   A                              %
%                            RRRR   L      AAAAA                              %
%                            R R    L      A   A                              %
%                            R  R   LLLLL  A   A                              %
%                                                                             %
%                                                                             %
%                      Read Alias/Wavefront Image Format                      %
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
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
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
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d R L A I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadRLAImage() reads a run-length encoded Wavefront RLA image file
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  Note:  This module was contributed by Lester Vecsey (master@internexus.net).
%
%  The format of the ReadRLAImage method is:
%
%      Image *ReadRLAImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadRLAImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  typedef struct _WindowFrame
  {
    short
      left,
      right,
      bottom,
      top;
  } WindowFrame;

  typedef struct _RLAInfo
  {
    WindowFrame
      window,
      active_window;

    short
      frame,
      storage_type,
      number_channels,
      number_matte_channels,
      number_auxiliary_channels,
      revision;

    char
      gamma[16+1],
      red_primary[24+1],
      green_primary[24+1],
      blue_primary[24+1],
      white_point[24+1];

    int
      job_number;

    char
      name[128+1],
      description[128+1],
      program[64+1],
      machine[32+1],
      user[32+1],
      date[20+1],
      aspect[24+1],
      aspect_ratio[8+1],
      chan[32+1];

    short
      field;

    char
      time[12],
      filter[32];

    short
      bits_per_channel,
      matte_type,
      matte_bits,
      auxiliary_type,
      auxiliary_bits;

    char
      auxiliary[32+1],
      space[36+1];

    int
      next;
  } RLAInfo;

  Image
    *image;

  int
    channel,
    length,
    runlength;

  MagickBooleanType
    status;

  MagickOffsetType
    offset,
    *scanlines;

  ssize_t
    i,
    x;

  Quantum
    *q;

  ssize_t
    count,
    y;

  RLAInfo
    rla_info;

  unsigned char
    byte;

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
  (void) memset(&rla_info,0,sizeof(rla_info));
  rla_info.window.left=(short) ReadBlobMSBShort(image);
  rla_info.window.right=(short) ReadBlobMSBShort(image);
  rla_info.window.bottom=(short) ReadBlobMSBShort(image);
  rla_info.window.top=(short) ReadBlobMSBShort(image);
  rla_info.active_window.left=(short) ReadBlobMSBShort(image);
  rla_info.active_window.right=(short) ReadBlobMSBShort(image);
  rla_info.active_window.bottom=(short) ReadBlobMSBShort(image);
  rla_info.active_window.top=(short) ReadBlobMSBShort(image);
  rla_info.frame=(short) ReadBlobMSBShort(image);
  rla_info.storage_type=(short) ReadBlobMSBShort(image);
  rla_info.number_channels=(short) ReadBlobMSBShort(image);
  if (rla_info.number_channels < 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  rla_info.number_matte_channels=(short) ReadBlobMSBShort(image);
  if (rla_info.number_matte_channels < 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if ((rla_info.number_channels > 3) || (rla_info.number_matte_channels > 3))
    ThrowReaderException(CoderError,"Unsupported number of channels");
  if (rla_info.number_channels == 0)
    rla_info.number_channels=3;
  rla_info.number_channels+=rla_info.number_matte_channels;
  rla_info.number_auxiliary_channels=(short) ReadBlobMSBShort(image);
  rla_info.revision=(short) ReadBlobMSBShort(image);
  (void) ReadBlob(image,16,(unsigned char *) rla_info.gamma);
  (void) ReadBlob(image,24,(unsigned char *) rla_info.red_primary);
  (void) ReadBlob(image,24,(unsigned char *) rla_info.green_primary);
  (void) ReadBlob(image,24,(unsigned char *) rla_info.blue_primary);
  (void) ReadBlob(image,24,(unsigned char *) rla_info.white_point);
  rla_info.job_number=ReadBlobMSBSignedLong(image);
  (void) ReadBlob(image,128,(unsigned char *) rla_info.name);
  (void) ReadBlob(image,128,(unsigned char *) rla_info.description);
  rla_info.description[127]='\0';
  (void) ReadBlob(image,64,(unsigned char *) rla_info.program);
  (void) ReadBlob(image,32,(unsigned char *) rla_info.machine);
  (void) ReadBlob(image,32,(unsigned char *) rla_info.user);
  (void) ReadBlob(image,20,(unsigned char *) rla_info.date);
  (void) ReadBlob(image,24,(unsigned char *) rla_info.aspect);
  (void) ReadBlob(image,8,(unsigned char *) rla_info.aspect_ratio);
  (void) ReadBlob(image,32,(unsigned char *) rla_info.chan);
  rla_info.field=(short) ReadBlobMSBShort(image);
  (void) ReadBlob(image,12,(unsigned char *) rla_info.time);
  (void) ReadBlob(image,32,(unsigned char *) rla_info.filter);
  rla_info.bits_per_channel=(short) ReadBlobMSBShort(image);
  rla_info.matte_type=(short) ReadBlobMSBShort(image);
  rla_info.matte_bits=(short) ReadBlobMSBShort(image);
  rla_info.auxiliary_type=(short) ReadBlobMSBShort(image);
  rla_info.auxiliary_bits=(short) ReadBlobMSBShort(image);
  (void) ReadBlob(image,32,(unsigned char *) rla_info.auxiliary);
  count=ReadBlob(image,36,(unsigned char *) rla_info.space);
  if ((size_t) count != 36)
    ThrowReaderException(CorruptImageError,"UnableToReadImageData");
  rla_info.next=ReadBlobMSBSignedLong(image);
  /*
    Initialize image structure.
  */
  image->alpha_trait=rla_info.number_matte_channels != 0 ? BlendPixelTrait : 
    UndefinedPixelTrait;
  image->columns=(size_t) (rla_info.active_window.right-
    rla_info.active_window.left+1);
  image->rows=(size_t) (rla_info.active_window.top-
    rla_info.active_window.bottom+1);
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  scanlines=(MagickOffsetType *) AcquireQuantumMemory(image->rows,
    sizeof(*scanlines));
  if (scanlines == (MagickOffsetType *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (*rla_info.description != '\0')
    (void) SetImageProperty(image,"comment",rla_info.description,exception);
  /*
    Read offsets to each scanline data.
  */
  for (i=0; i < (ssize_t) image->rows; i++)
    scanlines[i]=(MagickOffsetType) ReadBlobMSBSignedLong(image);
  if (EOFBlob(image) != MagickFalse)
    {
      scanlines=(MagickOffsetType *) RelinquishMagickMemory(scanlines);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  /*
    Read image data.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    offset=SeekBlob(image,scanlines[(ssize_t) image->rows-y-1],SEEK_SET);
    if (offset < 0)
      {
        scanlines=(MagickOffsetType *) RelinquishMagickMemory(scanlines);
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    x=0;
    for (channel=0; channel < (int) rla_info.number_channels; channel++)
    {
      length=ReadBlobMSBSignedShort(image);
      while (length > 0)
      {
        byte=(unsigned char) ReadBlobByte(image);
        runlength=byte;
        if (byte > 127)
          runlength=byte-256;
        length--;
        if (length == 0)
          break;
        if (runlength < 0)
          {
            while (runlength < 0)
            {
              q=GetAuthenticPixels(image,x % (ssize_t) image->columns,y,1,1,
                exception);
              if (q == (Quantum *) NULL)
                break;
              byte=(unsigned char) ReadBlobByte(image);
              length--;
              switch (channel)
              {
                case 0:
                {
                  SetPixelRed(image,ScaleCharToQuantum(byte),q);
                  break;
                }
                case 1:
                {
                  SetPixelGreen(image,ScaleCharToQuantum(byte),q);
                  break;
                }
                case 2:
                {
                  SetPixelBlue(image,ScaleCharToQuantum(byte),q);
                  break;
                }
                case 3:
                default:
                {
                  SetPixelAlpha(image,ScaleCharToQuantum(byte),q);
                  break;
                }
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              x++;
              runlength++;
            }
            continue;
          }
        byte=(unsigned char) ReadBlobByte(image);
        length--;
        runlength++;
        do
        {
          q=GetAuthenticPixels(image,x % (ssize_t) image->columns,y,1,1,
            exception);
          if (q == (Quantum *) NULL)
            break;
          switch (channel)
          {
            case 0:
            {
              SetPixelRed(image,ScaleCharToQuantum(byte),q);
              break;
            }
            case 1:
            {
              SetPixelGreen(image,ScaleCharToQuantum(byte),q);
              break;
            }
            case 2:
            {
              SetPixelBlue(image,ScaleCharToQuantum(byte),q);
              break;
            }
            case 3:
            default:
            {
              SetPixelAlpha(image,ScaleCharToQuantum(byte),q);
              break;
            }
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          x++;
          runlength--;
        }
        while (runlength > 0);
      }
    }
    if ((x/(ssize_t) rla_info.number_channels) > (ssize_t) image->columns)
      {
        scanlines=(MagickOffsetType *) RelinquishMagickMemory(scanlines);
        ThrowReaderException(CorruptImageError,"CorruptImage");
      }
    if (EOFBlob(image) != MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  scanlines=(MagickOffsetType *) RelinquishMagickMemory(scanlines);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
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
%   R e g i s t e r R L A I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterRLAImage() adds attributes for the RLA image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterRLAImage method is:
%
%      size_t RegisterRLAImage(void)
%
*/
ModuleExport size_t RegisterRLAImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("RLA","RLA","Alias/Wavefront image");
  entry->decoder=(DecodeImageHandler *) ReadRLAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r R L A I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterRLAImage() removes format registrations made by the
%  RLA module from the list of supported formats.
%
%  The format of the UnregisterRLAImage method is:
%
%      UnregisterRLAImage(void)
%
*/
ModuleExport void UnregisterRLAImage(void)
{
  (void) UnregisterMagickInfo("RLA");
}

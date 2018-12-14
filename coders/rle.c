/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            RRRR   L      EEEEE                              %
%                            R   R  L      E                                  %
%                            RRRR   L      EEE                                %
%                            R R    L      E                                  %
%                            R  R   LLLLL  EEEEE                              %
%                                                                             %
%                                                                             %
%                          Read URT RLE Image Format                          %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
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
#include "MagickCore/pixel.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s R L E                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsRLE() returns MagickTrue if the image format type, identified by the
%  magick string, is RLE.
%
%  The format of the ReadRLEImage method is:
%
%      MagickBooleanType IsRLE(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static MagickBooleanType IsRLE(const unsigned char *magick,const size_t length)
{
  if (length < 2)
    return(MagickFalse);
  if (memcmp(magick,"\122\314",2) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d R L E I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadRLEImage() reads a run-length encoded Utah Raster Toolkit
%  image file and returns it.  It allocates the memory necessary for the new
%  Image structure and returns a pointer to the new image.
%
%  The format of the ReadRLEImage method is:
%
%      Image *ReadRLEImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadRLEImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define SkipLinesOp  0x01
#define SetColorOp  0x02
#define SkipPixelsOp  0x03
#define ByteDataOp  0x05
#define RunDataOp  0x06
#define EOFOp  0x07
#define ThrowRLEException(exception,message) \
{ \
  if (colormap != (unsigned char *) NULL) \
    colormap=(unsigned char *) RelinquishMagickMemory(colormap); \
  if (pixel_info != (MemoryInfo *) NULL) \
    pixel_info=RelinquishVirtualMemory(pixel_info); \
  ThrowReaderException((exception),(message)); \
}

  char
    magick[12];

  Image
    *image;

  int
    opcode,
    operand,
    status;

  MagickStatusType
    flags;

  MagickSizeType
    number_pixels;

  MemoryInfo
    *pixel_info;

  Quantum
    index;

  register ssize_t
    x;

  register Quantum
    *q;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    bits_per_pixel,
    map_length,
    number_colormaps,
    number_planes,
    number_planes_filled,
    one,
    pixel_info_length;

  ssize_t
    count,
    offset,
    y;

  unsigned char
    background_color[256],
    *colormap,
    pixel,
    plane,
    *pixels;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Determine if this a RLE file.
  */
  colormap=(unsigned char *) NULL;
  pixel_info=(MemoryInfo *) NULL;
  count=ReadBlob(image,2,(unsigned char *) magick);
  if ((count != 2) || (memcmp(magick,"\122\314",2) != 0))
    ThrowRLEException(CorruptImageError,"ImproperImageHeader");
  do
  {
    /*
      Read image header.
    */
    image->page.x=(ssize_t) ReadBlobLSBShort(image);
    image->page.y=(ssize_t) ReadBlobLSBShort(image);
    image->columns=ReadBlobLSBShort(image);
    image->rows=ReadBlobLSBShort(image);
    flags=(MagickStatusType) ReadBlobByte(image);
    image->alpha_trait=flags & 0x04 ? BlendPixelTrait : UndefinedPixelTrait;
    number_planes=(size_t) ReadBlobByte(image);
    bits_per_pixel=(size_t) ReadBlobByte(image);
    number_colormaps=(size_t) ReadBlobByte(image);
    map_length=(unsigned char) ReadBlobByte(image);
    if (map_length >= 22)
      ThrowRLEException(CorruptImageError,"ImproperImageHeader");
    if (EOFBlob(image) != MagickFalse)
      ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
    one=1;
    map_length=one << map_length;
    if ((number_planes == 0) || (number_planes == 2) || ((flags & 0x04) &&
        ((number_planes <= 2) || number_planes > 254)) || (bits_per_pixel != 8))
      ThrowRLEException(CorruptImageError,"ImproperImageHeader");
    if (number_planes > 4)
      ThrowRLEException(CorruptImageError,"ImproperImageHeader");
    if ((image->columns == 0) || (image->columns >= 32768) ||
        (image->rows == 0) || (image->rows >= 32768))
      ThrowRLEException(CorruptImageError,"ImproperImageHeader");
    if (flags & 0x02)
      {
        /*
          No background color-- initialize to black.
        */
        for (i=0; i < (ssize_t) number_planes; i++)
          background_color[i]=0;
        (void) ReadBlobByte(image);
      }
    else
      {
        /*
          Initialize background color.
        */
        p=background_color;
        for (i=0; i < (ssize_t) number_planes; i++)
          *p++=(unsigned char) ReadBlobByte(image);
      }
    if ((number_planes & 0x01) == 0)
      (void) ReadBlobByte(image);
    if (EOFBlob(image) != MagickFalse)
      ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
    if (image->alpha_trait != UndefinedPixelTrait)
      number_planes++;
    number_pixels=(MagickSizeType) image->columns*image->rows;
    if ((GetBlobSize(image) == 0) || ((((MagickSizeType) number_pixels*
         number_planes*bits_per_pixel/8)/GetBlobSize(image)) > 254))
      ThrowRLEException(CorruptImageError,"InsufficientImageDataInFile")
    if (((MagickSizeType) number_colormaps*map_length) > GetBlobSize(image))
      ThrowRLEException(CorruptImageError,"InsufficientImageDataInFile")
    if (number_colormaps != 0)
      {
        /*
          Read image colormaps.
        */
        colormap=(unsigned char *) AcquireQuantumMemory(number_colormaps,
          3*map_length*sizeof(*colormap));
        if (colormap == (unsigned char *) NULL)
          ThrowRLEException(ResourceLimitError,"MemoryAllocationFailed");
        (void) memset(colormap,0,number_colormaps*3*map_length*
          sizeof(*colormap));
        p=colormap;
        for (i=0; i < (ssize_t) number_colormaps; i++)
          for (x=0; x < (ssize_t) map_length; x++)
          {
            *p++=(unsigned char) ScaleQuantumToChar(ScaleShortToQuantum(
              ReadBlobLSBShort(image)));
            if (EOFBlob(image) != MagickFalse)
              ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
          }
      }
    if ((flags & 0x08) != 0)
      {
        char
          *comment;

        size_t
          length;

        /*
          Read image comment.
        */
        length=ReadBlobLSBShort(image);
        if (length != 0)
          {
            comment=(char *) AcquireQuantumMemory(length,sizeof(*comment));
            if (comment == (char *) NULL)
              ThrowRLEException(ResourceLimitError,"MemoryAllocationFailed");
            count=ReadBlob(image,length-1,(unsigned char *) comment);
            if (count != (ssize_t) (length-1))
              {
                comment=DestroyString(comment);
                ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
              }
            comment[length-1]='\0';
            (void) SetImageProperty(image,"comment",comment,exception);
            comment=DestroyString(comment);
            if ((length & 0x01) == 0)
              (void) ReadBlobByte(image);
          }
      }
    if (EOFBlob(image) != MagickFalse)
      ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      {
        if (colormap != (unsigned char *) NULL)
          colormap=(unsigned char *) RelinquishMagickMemory(colormap);
        if (pixel_info != (MemoryInfo *) NULL)
          pixel_info=RelinquishVirtualMemory(pixel_info);
        return(DestroyImageList(image));
      }
    /*
      Allocate RLE pixels.
    */
    number_planes_filled=(number_planes % 2 == 0) ? number_planes :
      number_planes+1;
    if ((number_pixels*number_planes_filled) != (size_t) (number_pixels*
         number_planes_filled))
      ThrowRLEException(ResourceLimitError,"MemoryAllocationFailed");
    pixel_info=AcquireVirtualMemory(image->columns,image->rows*
      MagickMax(number_planes_filled,4)*sizeof(*pixels));
    if (pixel_info == (MemoryInfo *) NULL)
      ThrowRLEException(ResourceLimitError,"MemoryAllocationFailed");
    pixel_info_length=image->columns*image->rows*
      MagickMax(number_planes_filled,4);
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    (void) memset(pixels,0,pixel_info_length);
    if ((flags & 0x01) && !(flags & 0x02))
      {
        ssize_t
          j;

        /*
          Set background color.
        */
        p=pixels;
        for (i=0; i < (ssize_t) number_pixels; i++)
        {
          if (image->alpha_trait == UndefinedPixelTrait)
            for (j=0; j < (ssize_t) number_planes; j++)
              *p++=background_color[j];
          else
            {
              for (j=0; j < (ssize_t) (number_planes-1); j++)
                *p++=background_color[j];
              *p++=0;  /* initialize matte channel */
            }
        }
      }
    /*
      Read runlength-encoded image.
    */
    plane=0;
    x=0;
    y=0;
    opcode=ReadBlobByte(image);
    if (opcode == EOF)
      {
        if (number_colormaps != 0)
          colormap=(unsigned char *) RelinquishMagickMemory(colormap);
        ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
      }
    do
    {
      switch (opcode & 0x3f)
      {
        case SkipLinesOp:
        {
          operand=ReadBlobByte(image);
          if (operand == EOF)
            ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
          if (opcode & 0x40)
            {
              operand=ReadBlobLSBSignedShort(image);
              if (operand == EOF)
                ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
            }
          x=0;
          y+=operand;
          break;
        }
        case SetColorOp:
        {
          operand=ReadBlobByte(image);
          if (operand == EOF)
            ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
          plane=(unsigned char) operand;
          if (plane == 255)
            plane=(unsigned char) (number_planes-1);
          x=0;
          break;
        }
        case SkipPixelsOp:
        {
          operand=ReadBlobByte(image);
          if (operand == EOF)
            ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
          if (opcode & 0x40)
            {
              operand=ReadBlobLSBSignedShort(image);
              if (operand == EOF)
                ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
            }
          x+=operand;
          break;
        }
        case ByteDataOp:
        {
          operand=ReadBlobByte(image);
          if (operand == EOF)
            ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
          if (opcode & 0x40)
            {
              operand=ReadBlobLSBSignedShort(image);
              if (operand == EOF)
                ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
            }
          offset=(ssize_t) (((image->rows-y-1)*image->columns*number_planes)+x*
            number_planes+plane);
          operand++;
          if ((offset < 0) ||
              ((size_t) (offset+operand*number_planes) > pixel_info_length))
            ThrowRLEException(CorruptImageError,"UnableToReadImageData");
          p=pixels+offset;
          for (i=0; i < (ssize_t) operand; i++)
          {
            pixel=(unsigned char) ReadBlobByte(image);
            if ((y < (ssize_t) image->rows) &&
                ((x+i) < (ssize_t) image->columns))
              *p=pixel;
            p+=number_planes;
          }
          if (operand & 0x01)
            (void) ReadBlobByte(image);
          x+=operand;
          break;
        }
        case RunDataOp:
        {
          operand=ReadBlobByte(image);
          if (operand == EOF)
            ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
          if (opcode & 0x40)
            {
              operand=ReadBlobLSBSignedShort(image);
              if (operand == EOF)
                ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
            }
          pixel=(unsigned char) ReadBlobByte(image);
          (void) ReadBlobByte(image);
          offset=(ssize_t) (((image->rows-y-1)*image->columns*number_planes)+x*
            number_planes+plane);
          operand++;
          if ((offset < 0) ||
              ((size_t) (offset+operand*number_planes) > pixel_info_length))
            ThrowRLEException(CorruptImageError,"UnableToReadImageData");
          p=pixels+offset;
          for (i=0; i < (ssize_t) operand; i++)
          {
            if ((y < (ssize_t) image->rows) &&
                ((x+i) < (ssize_t) image->columns))
              *p=pixel;
            p+=number_planes;
          }
          x+=operand;
          break;
        }
        default:
          break;
      }
      opcode=ReadBlobByte(image);
      if (opcode == EOF)
        ThrowRLEException(CorruptImageError,"UnexpectedEndOfFile");
    } while (((opcode & 0x3f) != EOFOp) && (opcode != EOF));
    if (number_colormaps != 0)
      {
        MagickStatusType
          mask;

        /*
          Apply colormap affineation to image.
        */
        mask=(MagickStatusType) (map_length-1);
        p=pixels;
        x=(ssize_t) number_planes;
        if (number_colormaps == 1)
          for (i=0; i < (ssize_t) number_pixels; i++)
          {
            ValidateColormapValue(image,(ssize_t) (*p & mask),&index,exception);
            *p=colormap[(ssize_t) index];
            p++;
          }
        else
          if ((number_planes >= 3) && (number_colormaps >= 3))
            for (i=0; i < (ssize_t) number_pixels; i++)
              for (x=0; x < (ssize_t) number_planes; x++)
              {
                ValidateColormapValue(image,(ssize_t) (x*map_length+
                  (*p & mask)),&index,exception);
                *p=colormap[(ssize_t) index];
                p++;
              }
        if ((i < (ssize_t) number_pixels) || (x < (ssize_t) number_planes))
          ThrowRLEException(CorruptImageError,"UnableToReadImageData");
      }
    /*
      Initialize image structure.
    */
    if (number_planes >= 3)
      {
        /*
          Convert raster image to DirectClass pixel packets.
        */
        p=pixels;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelRed(image,ScaleCharToQuantum(*p++),q);
            SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
            SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
            if (image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
      }
    else
      {
        /*
          Create colormap.
        */
        if (number_colormaps == 0)
          map_length=256;
        if (AcquireImageColormap(image,map_length,exception) == MagickFalse)
          ThrowRLEException(ResourceLimitError,"MemoryAllocationFailed");
        p=colormap;
        if (number_colormaps == 1)
          for (i=0; i < (ssize_t) image->colors; i++)
          {
            /*
              Pseudocolor.
            */
            image->colormap[i].red=(MagickRealType)
              ScaleCharToQuantum((unsigned char) i);
            image->colormap[i].green=(MagickRealType)
              ScaleCharToQuantum((unsigned char) i);
            image->colormap[i].blue=(MagickRealType)
              ScaleCharToQuantum((unsigned char) i);
          }
        else
          if (number_colormaps > 1)
            for (i=0; i < (ssize_t) image->colors; i++)
            {
              image->colormap[i].red=(MagickRealType)
                ScaleCharToQuantum(*p);
              image->colormap[i].green=(MagickRealType)
                ScaleCharToQuantum(*(p+map_length));
              image->colormap[i].blue=(MagickRealType)
                ScaleCharToQuantum(*(p+map_length*2));
              p++;
            }
        p=pixels;
        if (image->alpha_trait == UndefinedPixelTrait)
          {
            /*
              Convert raster image to PseudoClass pixel packets.
            */
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelIndex(image,(Quantum) *p++,q);
                q+=GetPixelChannels(image);
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            (void) SyncImage(image,exception);
          }
        else
          {
            /*
              Image has a matte channel-- promote to DirectClass.
            */
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                ValidateColormapValue(image,(ssize_t) *p++,&index,exception);
                SetPixelRed(image,ClampToQuantum(image->colormap[(ssize_t)
                  index].red),q);
                ValidateColormapValue(image,(ssize_t) *p++,&index,exception);
                SetPixelGreen(image,ClampToQuantum(image->colormap[(ssize_t)
                  index].green),q);
                ValidateColormapValue(image,(ssize_t) *p++,&index,exception);
                SetPixelBlue(image,ClampToQuantum(image->colormap[(ssize_t)
                  index].blue),q);
                SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
                q+=GetPixelChannels(image);
              }
              if (x < (ssize_t) image->columns)
                break;
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            image->colormap=(PixelInfo *) RelinquishMagickMemory(
              image->colormap);
            image->storage_class=DirectClass;
            image->colors=0;
          }
      }
    if (number_colormaps != 0)
      colormap=(unsigned char *) RelinquishMagickMemory(colormap);
    pixel_info=RelinquishVirtualMemory(pixel_info);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    (void) ReadBlobByte(image);
    count=ReadBlob(image,2,(unsigned char *) magick);
    if ((count != 0) && (memcmp(magick,"\122\314",2) == 0))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            status=MagickFalse;
            break;
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  } while ((count != 0) && (memcmp(magick,"\122\314",2) == 0));
  (void) CloseBlob(image);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r R L E I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterRLEImage() adds attributes for the RLE image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterRLEImage method is:
%
%      size_t RegisterRLEImage(void)
%
*/
ModuleExport size_t RegisterRLEImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("RLE","RLE","Utah Run length encoded image");
  entry->decoder=(DecodeImageHandler *) ReadRLEImage;
  entry->magick=(IsImageFormatHandler *) IsRLE;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r R L E I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterRLEImage() removes format registrations made by the
%  RLE module from the list of supported formats.
%
%  The format of the UnregisterRLEImage method is:
%
%      UnregisterRLEImage(void)
%
*/
ModuleExport void UnregisterRLEImage(void)
{
  (void) UnregisterMagickInfo("RLE");
}

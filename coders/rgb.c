/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            RRRR    GGGG  BBBB                               %
%                            R   R  G      B   B                              %
%                            RRRR   G  GG  BBBB                               %
%                            R R    G   G  B   B                              %
%                            R  R    GGG   BBBB                               %
%                                                                             %
%                                                                             %
%                     Read/Write Raw RGB Image Format                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2008 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/colorspace.h"
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
#include "magick/pixel-private.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteRGBImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d R G B I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadRGBImage() reads an image of raw RGB or RGBA samples and returns it. It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadRGBImage method is:
%
%      Image *ReadRGBImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadRGBImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *canvas_image,
    *image;

  long
    y;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register long
    i,
    j;

  Quantum
    qx[3];

  ssize_t
    count;

  size_t
    length;

  unsigned char
    *pixels;

  QuantumType
    quantum_types[4];

  char
    sfx[] = {0, 0};

  int
    channels = 3;

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
  image=AcquireImage(image_info);
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(OptionError,"MustSpecifyImageSize");
  if (image_info->interlace != PartitionInterlace)
    {
      status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
      if (status == MagickFalse)
        {
          image=DestroyImageList(image);
          return((Image *) NULL);
        }
      for (i=0; i < image->offset; i++)
        if (ReadBlobByte(image) == EOF)
          {
            ThrowFileException(exception,CorruptImageError,
              "UnexpectedEndOfFile",image->filename);
            break;
          }
    }
  /*
    Create virtual canvas to support cropping (i.e. image.rgb[100x100+10+20]).
  */
  canvas_image=CloneImage(image,image->extract_info.width,1,MagickFalse,
    exception);
  (void) SetImageVirtualPixelMethod(canvas_image,BlackVirtualPixelMethod);
  quantum_info=AcquireQuantumInfo(image_info,canvas_image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  pixels=GetQuantumPixels(quantum_info);
  quantum_type=RGBQuantum;
  if (LocaleCompare(image_info->magick,"RGBA") == 0)
    {
      quantum_type=RGBAQuantum;
      image->matte=MagickTrue;
      channels=4;
    }
  else if (LocaleCompare(image_info->magick,"BGRA") == 0)
    {
      quantum_type=BGRAQuantum;
      image->matte=MagickTrue;
      channels=4;
    }
  else if (LocaleCompare(image_info->magick,"RGBO") == 0)
    {
      quantum_type=RGBOQuantum;
      image->matte=MagickTrue;
      channels=4;
    }
  if (image_info->number_scenes != 0)
    while (image->scene < image_info->scene)
    {
      /*
        Skip to next image.
      */
      image->scene++;
      length=GetQuantumExtent(canvas_image,quantum_info,quantum_type);
      for (y=0; y < (long) image->rows; y++)
      {
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
      }
    }
  for (i=0; i < channels; i++)
  {
    switch(image_info->magick[i])
    {
      case 'R': quantum_types[i]=RedQuantum;     break;
      case 'G': quantum_types[i]=GreenQuantum;   break;
      case 'B': quantum_types[i]=BlueQuantum;    break;
      case 'A': quantum_types[i]=AlphaQuantum;   break;
      case 'O': quantum_types[i]=OpacityQuantum; break;
    }
  }
  count=0;
  length=0;
  scene=0;
  do
  {
    /*
      Read pixels to virtual canvas image then push to image.
    */
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    switch (image_info->interlace)
    {
      case NoInterlace:
      default:
      {
        /*
          No interlacing:  RGBRGBRGBRGBRGBRGB...
        */
        if (scene == 0)
          {
            length=GetQuantumExtent(canvas_image,quantum_info,quantum_type);
            count=ReadBlob(image,length,pixels);
            if (count != (ssize_t) length)
              break;
          }
        for (y=0; y < (long) image->extract_info.height; y++)
        {
          register const PixelPacket
            *restrict p;

          register long
            x;

          register PixelPacket
            *restrict q;

          if (count != (ssize_t) length)
            {
              ThrowFileException(exception,CorruptImageError,
                "UnexpectedEndOfFile",image->filename);
              break;
            }
          q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
            exception);
          if (q == (PixelPacket *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
            quantum_info,quantum_type,pixels,exception);
          if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
            break;
          if (((y-image->extract_info.y) >= 0) &&
              ((y-image->extract_info.y) < (long) image->rows))
            {
              p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=QueueAuthenticPixels(image,0,y-image->extract_info.y,
                image->columns,1,exception);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                qx[0]=GetRedPixelComponent(p);
                qx[1]=GetGreenPixelComponent(p);
                qx[2]=GetBluePixelComponent(p);
                for (i=0; i < 3; i++)
                  switch(quantum_types[i])
                  {
                    case RedQuantum:   q->red=qx[i];   break;
                    case GreenQuantum: q->green=qx[i]; break;
                    case BlueQuantum:  q->blue=qx[i];  break;
                    default:                           break;
                  }
                SetOpacityPixelComponent(q,OpaqueOpacity);
                if (image->matte != MagickFalse)
                  SetOpacityPixelComponent(q,GetOpacityPixelComponent(p));
                p++;
                q++;
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
            }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,y,image->rows);
              if (status == MagickFalse)
                break;
            }
          count=ReadBlob(image,length,pixels);
        }
        break;
      }
      case LineInterlace:
      {
        /*
          Line interlacing:  RRR...GGG...BBB...RRR...GGG...BBB...
        */
        if (scene == 0)
          {
            length=GetQuantumExtent(canvas_image,quantum_info,quantum_types[0]);
            count=ReadBlob(image,length,pixels);
          }
        for (y=0; y < (long) image->extract_info.height; y++)
        {
          register const PixelPacket
            *restrict p;

          register long
            x;

          register PixelPacket
            *restrict q;

          if (count != (ssize_t) length)
            {
              ThrowFileException(exception,CorruptImageError,
                "UnexpectedEndOfFile",image->filename);
              break;
            }
          for (i=0; i < channels; i++)
          {
            q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
              exception);
            if (q == (PixelPacket *) NULL)
              break;
            length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
              quantum_info,quantum_types[i],pixels,exception);
            if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
              break;
            if (((y-image->extract_info.y) >= 0) &&
                ((y-image->extract_info.y) < (long) image->rows))
              {
                p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,
                  0,canvas_image->columns,1,exception);
                q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                  image->columns,1,exception);
                if ((p == (const PixelPacket *) NULL) ||
                    (q == (PixelPacket *) NULL))
                  break;
                if (i == (channels - 1))
                  for (x=0; x < (long) image->columns; x++)
                  {
                    SetRedPixelComponent(q,GetRedPixelComponent(p));
                    SetGreenPixelComponent(q,GetGreenPixelComponent(p));
                    SetBluePixelComponent(q,GetBluePixelComponent(p));
                    SetOpacityPixelComponent(q,GetOpacityPixelComponent(p));
                    p++;
                    q++;
                  }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
              }
            count=ReadBlob(image,length,pixels);
          }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,y,image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case PlaneInterlace:
      {
        /*
          Plane interlacing:  RRRRRR...GGGGGG...BBBBBB...
        */
        if (scene == 0)
          {
            length=GetQuantumExtent(canvas_image,quantum_info,quantum_types[0]);
            count=ReadBlob(image,length,pixels);
          }
        for (i=0; i < channels; i++)
        {
          for (y=0; y < (long) image->extract_info.height; y++)
          {
            register const PixelPacket
              *restrict p;

            register long
              x;

            register PixelPacket
              *restrict q;

            if (count != (ssize_t) length)
              {
                ThrowFileException(exception,CorruptImageError,
                  "UnexpectedEndOfFile",image->filename);
                break;
              }
            q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
              exception);
            if (q == (PixelPacket *) NULL)
              break;
            length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
              quantum_info,quantum_types[i],pixels,exception);
            if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
              break;
            if (((y-image->extract_info.y) >= 0) &&
                ((y-image->extract_info.y) < (long) image->rows))
              {
                p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
                  canvas_image->columns,1,exception);
                q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                  image->columns,1,exception);
                if ((p == (const PixelPacket *) NULL) ||
                    (q == (PixelPacket *) NULL))
                  break;
                for (x=0; x < (long) image->columns; x++)
                {
                  switch(quantum_types[i])
                  {
                    case RedQuantum:    SetRedPixelComponent(q,GetRedPixelComponent(p));         break;
                    case GreenQuantum:  SetGreenPixelComponent(q,GetGreenPixelComponent(p));     break;
                    case BlueQuantum:   SetBluePixelComponent(q,GetBluePixelComponent(p));       break;
                    case OpacityQuantum:
                    case AlphaQuantum:  SetOpacityPixelComponent(q,GetOpacityPixelComponent(p)); break;
                    default:                                   break;
                  }
                  p++;
                  q++;
                }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
              }
            count=ReadBlob(image,length,pixels);
          }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(i+1),5);
              if (status == MagickFalse)
                break;
            }
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,5,5);
            if (status == MagickFalse)
              break;
          }
        break;
      }
      case PartitionInterlace:
      {
        /*
          Partition interlacing:  RRRRRR..., GGGGGG..., BBBBBB...
        */
        for (i=0; i < channels; i++)
        {
          sfx[0]=image_info->magick[i];
          AppendImageFormat(sfx,image->filename);
          status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
          if (status == MagickFalse)
            {
              canvas_image=DestroyImageList(canvas_image);
              image=DestroyImageList(image);
              return((Image *) NULL);
            }
          if (i == 0)
            for (j=0; j < image->offset; j++)
              if (ReadBlobByte(image) == EOF)
                {
                  ThrowFileException(exception,CorruptImageError,
                    "UnexpectedEndOfFile",image->filename);
                  break;
                }
          length=GetQuantumExtent(canvas_image,quantum_info,quantum_types[i]);
          for (j=0; j < (long) scene; j++)
            for (y=0; y < (long) image->extract_info.height; y++)
              if (ReadBlob(image,length,pixels) != (ssize_t) length)
                {
                  ThrowFileException(exception,CorruptImageError,
                    "UnexpectedEndOfFile",image->filename);
                  break;
                }
          count=ReadBlob(image,length,pixels);
          for (y=0; y < (long) image->extract_info.height; y++)
          {
            register const PixelPacket
              *restrict p;

            register long
              x;

            register PixelPacket
              *restrict q;

            if (count != (ssize_t) length)
              {
                ThrowFileException(exception,CorruptImageError,
                  "UnexpectedEndOfFile",image->filename);
                break;
              }
            q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
              exception);
            if (q == (PixelPacket *) NULL)
              break;
            length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
              quantum_info,quantum_types[i],pixels,exception);
            if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
              break;
            if (((y-image->extract_info.y) >= 0) &&
                ((y-image->extract_info.y) < (long) image->rows))
              {
                p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
                  canvas_image->columns,1,exception);
                q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                  image->columns,1,exception);
                if ((p == (const PixelPacket *) NULL) ||
                    (q == (PixelPacket *) NULL))
                  break;
                for (x=0; x < (long) image->columns; x++)
                {
                  switch(quantum_types[i])
                  {
                    case RedQuantum:    SetRedPixelComponent(q,GetRedPixelComponent(p));         break;
                    case GreenQuantum:  SetGreenPixelComponent(q,GetGreenPixelComponent(p));     break;
                    case BlueQuantum:   SetBluePixelComponent(q,GetBluePixelComponent(p));       break;
                    case OpacityQuantum:
                    case AlphaQuantum:  SetOpacityPixelComponent(q,GetOpacityPixelComponent(p)); break;
                    default:                                   break;
                  }
                  p++;
                  q++;
                }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
              }
            count=ReadBlob(image,length,pixels);
          }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(i+1),5);
              if (status == MagickFalse)
                break;
            }
          if (i != (channels-1))
            (void) CloseBlob(image);
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,5,5);
            if (status == MagickFalse)
              break;
          }
        break;
      }
    }
    SetQuantumImageType(image,quantum_type);
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if (count == (ssize_t) length)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
    scene++;
  } while (count == (ssize_t) length);
  quantum_info=DestroyQuantumInfo(quantum_info);
  InheritException(&image->exception,&canvas_image->exception);
  canvas_image=DestroyImage(canvas_image);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r R G B I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterRGBImage() adds attributes for the RGB or RGBA image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterRGBImage method is:
%
%      unsigned long RegisterRGBImage(void)
%
*/
ModuleExport unsigned long RegisterRGBImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("RGB");
  entry->decoder=(DecodeImageHandler *) ReadRGBImage;
  entry->encoder=(EncodeImageHandler *) WriteRGBImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Raw red, green, and blue samples");
  entry->module=ConstantString("RGB");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("RBG");
  entry->decoder=(DecodeImageHandler *) ReadRGBImage;
  entry->encoder=(EncodeImageHandler *) WriteRGBImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Raw red, blue, and green samples");
  entry->module=ConstantString("RGB");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("GRB");
  entry->decoder=(DecodeImageHandler *) ReadRGBImage;
  entry->encoder=(EncodeImageHandler *) WriteRGBImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Raw green, red, and blue samples");
  entry->module=ConstantString("RGB");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("GBR");
  entry->decoder=(DecodeImageHandler *) ReadRGBImage;
  entry->encoder=(EncodeImageHandler *) WriteRGBImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Raw green, blue, and red samples");
  entry->module=ConstantString("RGB");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("BRG");
  entry->decoder=(DecodeImageHandler *) ReadRGBImage;
  entry->encoder=(EncodeImageHandler *) WriteRGBImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Raw blue, red, and green samples");
  entry->module=ConstantString("RGB");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("BGR");
  entry->decoder=(DecodeImageHandler *) ReadRGBImage;
  entry->encoder=(EncodeImageHandler *) WriteRGBImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Raw blue, green, and red samples");
  entry->module=ConstantString("RGB");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("BGRA");
  entry->decoder=(DecodeImageHandler *) ReadRGBImage;
  entry->encoder=(EncodeImageHandler *) WriteRGBImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Raw blue, green, red and alpha samples");
  entry->module=ConstantString("RGB");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("RGBA");
  entry->decoder=(DecodeImageHandler *) ReadRGBImage;
  entry->encoder=(EncodeImageHandler *) WriteRGBImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Raw red, green, blue, and alpha samples");
  entry->module=ConstantString("RGB");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("RGBO");
  entry->decoder=(DecodeImageHandler *) ReadRGBImage;
  entry->encoder=(EncodeImageHandler *) WriteRGBImage;
  entry->raw=MagickTrue;
  entry->endian_support=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Raw red, green, blue, and opacity "
    "samples");
  entry->module=ConstantString("RGB");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r R G B I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterRGBImage() removes format registrations made by the
%  RGB module from the list of supported formats.
%
%  The format of the UnregisterRGBImage method is:
%
%      UnregisterRGBImage(void)
%
*/
ModuleExport void UnregisterRGBImage(void)
{
  (void) UnregisterMagickInfo("RGBO");
  (void) UnregisterMagickInfo("RGBA");
  (void) UnregisterMagickInfo("BGR");
  (void) UnregisterMagickInfo("BGRA");
  (void) UnregisterMagickInfo("BRG");
  (void) UnregisterMagickInfo("GBR");
  (void) UnregisterMagickInfo("GRB");
  (void) UnregisterMagickInfo("RBG");
  (void) UnregisterMagickInfo("RGB");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e R G B I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteRGBImage() writes an image to a file in the RGB or RGBA rasterfile
%  format.
%
%  The format of the WriteRGBImage method is:
%
%      MagickBooleanType WriteRGBImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteRGBImage(const ImageInfo *image_info,Image *image)
{
  long
    y;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type,
    quantum_types[4];

  register long
    i;

  ssize_t
    count;

  size_t
    length;

  unsigned char
    *pixels;

  unsigned long
    channels;

  /*
    Allocate memory for pixels.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image_info->interlace != PartitionInterlace)
    {
      /*
        Open output image file.
      */
      status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
      if (status == MagickFalse)
        return(status);
    }
  quantum_type=RGBQuantum;
  channels=3;
  if (LocaleCompare(image_info->magick,"RGBA") == 0)
    {
      quantum_type=RGBAQuantum;
      image->matte=MagickTrue;
      channels=4;
    }
  if (LocaleCompare(image_info->magick,"RGBO") == 0)
    {
      quantum_type=RGBOQuantum;
      image->matte=MagickTrue;
      channels=4;
    }
  for (i=0; i < (long) channels; i++)
  {
    switch (image_info->magick[i])
    {
      case 'R': quantum_types[i]=RedQuantum;     break;
      case 'G': quantum_types[i]=GreenQuantum;   break;
      case 'B': quantum_types[i]=BlueQuantum;    break;
      case 'A': quantum_types[i]=AlphaQuantum;   break;
      case 'O': quantum_types[i]=OpacityQuantum; break;
    }
  }
  scene=0;
  do
  {
    /*
      Convert MIFF to RGB raster pixels.
    */
    if (image->colorspace != RGBColorspace)
      (void) TransformImageColorspace(image,RGBColorspace);
    if ((LocaleCompare(image_info->magick,"RGBA") == 0) &&
        (image->matte == MagickFalse))
      (void) SetImageAlphaChannel(image,ResetAlphaChannel);
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=GetQuantumPixels(quantum_info);
    switch (image_info->interlace)
    {
      case NoInterlace:
      default:
      {
        CacheView
          *image_view;
          
        PixelPacket
          px;

        Quantum
          *qx[3];

        /*
          No interlacing:  RGBRGBRGBRGBRGBRGB...
        */
        image_view=AcquireCacheView(image);
        for (y=0; y < (long) image->rows; y++)
        {
          register long
            x;

          register PixelPacket
            *restrict q;

          q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
            &image->exception);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (long) image->columns; x++)
          {
            px=(*q);
            qx[0]=&(q->red);
            qx[1]=&(q->green);
            qx[2]=&(q->blue);
            for (i=0; i < 3; i++)
              switch (quantum_types[i])
              {
                case RedQuantum:   *qx[i]=px.red;   break;
                case GreenQuantum: *qx[i]=px.green; break;
                case BlueQuantum:  *qx[i]=px.blue;  break;
                default:                            break;
              }
            q++;
          }
          length=ExportQuantumPixels(image,image_view,quantum_info,quantum_type,
            pixels,&image->exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,y,image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        image_view=DestroyCacheView(image_view);
        break;
      }
      case LineInterlace:
      {
        /*
          Line interlacing:  RRR...GGG...BBB...RRR...GGG...BBB...
        */
        for (y=0; y < (long) image->rows; y++)
        {
          register const PixelPacket
            *restrict p;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          for (i=0; i < (long) channels; i++)
          {
            length=ExportQuantumPixels(image,(const CacheView *) NULL,
              quantum_info,quantum_types[i],pixels,&image->exception);
            count=WriteBlob(image,length,pixels);
            if (count != (ssize_t) length)
              break;
          }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,y,image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case PlaneInterlace:
      {
        /*
          Plane interlacing:  RRRRRR...GGGGGG...BBBBBB...
        */
        for (i=0; i < (long) channels; i++)
        {
          for (y=0; y < (long) image->rows; y++)
          {
            register const PixelPacket
              *restrict p;

            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            length=ExportQuantumPixels(image,(const CacheView *) NULL,
              quantum_info,quantum_types[i],pixels,&image->exception);
            count=WriteBlob(image,length,pixels);
            if (count != (ssize_t) length)
              break;
          }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(i+1),5);
              if (status == MagickFalse)
                break;
            }
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,SaveImageTag,5,5);
            if (status == MagickFalse)
              break;
          }
        break;
      }
      case PartitionInterlace:
      {
        char
          sfx[] = {0, 0};

        /*
          Partition interlacing:  RRRRRR..., GGGGGG..., BBBBBB...
        */
        for (i=0; i < (long) channels; i++)
        {
          sfx[0]=image_info->magick[i];
          AppendImageFormat(sfx,image->filename);
          status=OpenBlob(image_info,image,scene == 0 ? WriteBinaryBlobMode :
            AppendBinaryBlobMode,&image->exception);
          if (status == MagickFalse)
            return(status);
          for (y=0; y < (long) image->rows; y++)
          {
            register const PixelPacket
              *restrict p;

            p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
            if (p == (const PixelPacket *) NULL)
              break;
            length=ExportQuantumPixels(image,(const CacheView *) NULL,
              quantum_info,quantum_types[i],pixels,&image->exception);
            count=WriteBlob(image,length,pixels);
            if (count != (ssize_t) length)
              break;
          }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(i+1),5);
              if (status == MagickFalse)
                break;
            }
          (void) CloseBlob(image);
        }
        (void) CopyMagickString(image->filename,image_info->filename,
          MaxTextExtent);
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,SaveImageTag,5,5);
            if (status == MagickFalse)
              break;
          }
        break;
      }
    }
    quantum_info=DestroyQuantumInfo(quantum_info);
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

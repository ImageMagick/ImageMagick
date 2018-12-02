/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     Y   Y   YYYC  BBBB    YYYC  RRRR                        %
%                      Y Y   C      B   B  C      R   R                       %
%                       Y    C      BBBB   C      RRRR                        %
%                       Y    C      B   B  C      R  R                        %
%                       Y     YYYC  BBBB    YYYC  R   R                       %
%                                                                             %
%                                                                             %
%                     Read/Write Raw YCbCr Image Format                       %
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
#include "MagickCore/channel.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/constitute.h"
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
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteYCBCRImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d Y C b C r I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadYCBCRImage() reads an image of raw YCbCr or YCbCrA samples and returns
%  it. It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadYCBCRImage method is:
%
%      Image *ReadYCBCRImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadYCBCRImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  const unsigned char
    *pixels;

  Image
    *canvas_image,
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register ssize_t
    i;

  size_t
    length;

  ssize_t
    count,
    y;

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
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(OptionError,"MustSpecifyImageSize");
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  (void) SetImageColorspace(image,YCbCrColorspace,exception);
  if (image_info->interlace != PartitionInterlace)
    {
      status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
      if (status == MagickFalse)
        {
          image=DestroyImageList(image);
          return((Image *) NULL);
        }
      if (DiscardBlobBytes(image,(MagickSizeType) image->offset) == MagickFalse)
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
    }
  /*
    Create virtual canvas to support cropping (i.e. image.rgb[100x100+10+20]).
  */
  canvas_image=CloneImage(image,image->extract_info.width,1,MagickFalse,
    exception);
  if (canvas_image == (Image *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) SetImageVirtualPixelMethod(canvas_image,BlackVirtualPixelMethod,
    exception);
  quantum_info=AcquireQuantumInfo(image_info,canvas_image);
  if (quantum_info == (QuantumInfo *) NULL)
    {
      canvas_image=DestroyImage(canvas_image);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  quantum_type=RGBQuantum;
  if (LocaleCompare(image_info->magick,"YCbCrA") == 0)
    {
      quantum_type=RGBAQuantum;
      image->alpha_trait=BlendPixelTrait;
    }
  pixels=(const unsigned char *) NULL;
  if (image_info->number_scenes != 0)
    while (image->scene < image_info->scene)
    {
      /*
        Skip to next image.
      */
      image->scene++;
      length=GetQuantumExtent(canvas_image,quantum_info,quantum_type);
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        pixels=(const unsigned char *) ReadBlobStream(image,length,
          GetQuantumPixels(quantum_info),&count);
        if (count != (ssize_t) length)
          break;
      }
    }
  count=0;
  length=0;
  scene=0;
  status=MagickTrue;
  do
  {
    /*
      Read pixels to virtual canvas image then push to image.
    */
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      break;
    if (SetImageColorspace(image,YCbCrColorspace,exception) == MagickFalse)
      break;
    switch (image_info->interlace)
    {
      case NoInterlace:
      default:
      {
        /*
          No interlacing:  YCbCrYCbCrYCbCrYCbCrYCbCrYCbCr...
        */
        if (scene == 0)
          {
            length=GetQuantumExtent(canvas_image,quantum_info,quantum_type);
            pixels=(const unsigned char *) ReadBlobStream(image,length,
              GetQuantumPixels(quantum_info),&count);
          }
        for (y=0; y < (ssize_t) image->extract_info.height; y++)
        {
          register const Quantum
            *magick_restrict p;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          if (count != (ssize_t) length)
            {
              status=MagickFalse;
              ThrowFileException(exception,CorruptImageError,
                "UnexpectedEndOfFile",image->filename);
              break;
            }
          q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
            exception);
          if (q == (Quantum *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
            quantum_info,quantum_type,pixels,exception);
          if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
            break;
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (ssize_t) image->rows))
            {
              p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=QueueAuthenticPixels(image,0,y-image->extract_info.y,
                image->columns,1,exception);
              if ((p == (const Quantum *) NULL) ||
                  (q == (Quantum *) NULL))
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelRed(image,GetPixelRed(canvas_image,p),q);
                SetPixelGreen(image,GetPixelGreen(canvas_image,p),q);
                SetPixelBlue(image,GetPixelBlue(canvas_image,p),q);
                if (image->alpha_trait != UndefinedPixelTrait)
                  SetPixelAlpha(image,GetPixelAlpha(canvas_image,p),q);
                p+=GetPixelChannels(canvas_image);
                q+=GetPixelChannels(image);
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
            }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
          pixels=(const unsigned char *) ReadBlobStream(image,length,
            GetQuantumPixels(quantum_info),&count);
        }
        break;
      }
      case LineInterlace:
      {
        static QuantumType
          quantum_types[4] =
          {
            RedQuantum,
            GreenQuantum,
            BlueQuantum,
            OpacityQuantum
          };

        /*
          Line interlacing:  YYY...CbCbCb...CrCrCr...YYY...CbCbCb...CrCrCr...
        */
        if (scene == 0)
          {
            length=GetQuantumExtent(canvas_image,quantum_info,RedQuantum);
            pixels=(const unsigned char *) ReadBlobStream(image,length,
              GetQuantumPixels(quantum_info),&count);
          }
        for (y=0; y < (ssize_t) image->extract_info.height; y++)
        {
          for (i=0; i < (ssize_t) (image->alpha_trait != UndefinedPixelTrait ? 4 : 3); i++)
          {
            register const Quantum
              *magick_restrict p;

            register Quantum
              *magick_restrict q;

            register ssize_t
              x;

            if (count != (ssize_t) length)
              {
                status=MagickFalse;
                ThrowFileException(exception,CorruptImageError,
                  "UnexpectedEndOfFile",image->filename);
                break;
              }
            quantum_type=quantum_types[i];
            q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
              exception);
            if (q == (Quantum *) NULL)
              break;
            length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
              quantum_info,quantum_type,pixels,exception);
            if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
              break;
            if (((y-image->extract_info.y) >= 0) && 
                ((y-image->extract_info.y) < (ssize_t) image->rows))
              {
                p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,
                  0,canvas_image->columns,1,exception);
                q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                  image->columns,1,exception);
                if ((p == (const Quantum *) NULL) ||
                    (q == (Quantum *) NULL))
                  break;
                for (x=0; x < (ssize_t) image->columns; x++)
                {
                  switch (quantum_type)
                  {
                    case RedQuantum:
                    {
                      SetPixelRed(image,GetPixelRed(canvas_image,p),q);
                      break;
                    }
                    case GreenQuantum:
                    {
                      SetPixelGreen(image,GetPixelGreen(canvas_image,p),q);
                      break;
                    }
                    case BlueQuantum:
                    {
                      SetPixelBlue(image,GetPixelBlue(canvas_image,p),q);
                      break;
                    }
                    case OpacityQuantum:
                    {
                      SetPixelAlpha(image,GetPixelAlpha(canvas_image,p),q);
                      break;
                    }
                    default:
                      break;
                  }
                  p+=GetPixelChannels(canvas_image);
                  q+=GetPixelChannels(image);
                }
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
              }
            pixels=(const unsigned char *) ReadBlobStream(image,length,
              GetQuantumPixels(quantum_info),&count);
          }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case PlaneInterlace:
      {
        /*
          Plane interlacing:  YYYYYY...CbCbCbCbCbCb...CrCrCrCrCrCr...
        */
        if (scene == 0)
          {
            length=GetQuantumExtent(canvas_image,quantum_info,RedQuantum);
            pixels=(const unsigned char *) ReadBlobStream(image,length,
              GetQuantumPixels(quantum_info),&count);
          }
        for (y=0; y < (ssize_t) image->extract_info.height; y++)
        {
          register const Quantum
            *magick_restrict p;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          if (count != (ssize_t) length)
            {
              status=MagickFalse;
              ThrowFileException(exception,CorruptImageError,
                "UnexpectedEndOfFile",image->filename);
              break;
            }
          q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
            exception);
          if (q == (Quantum *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
            quantum_info,RedQuantum,pixels,exception);
          if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
            break;
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (ssize_t) image->rows))
            {
              p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                image->columns,1,exception);
              if ((p == (const Quantum *) NULL) ||
                  (q == (Quantum *) NULL))
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelRed(image,GetPixelRed(canvas_image,p),q);
                p+=GetPixelChannels(canvas_image);
                q+=GetPixelChannels(image);
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
            }
          pixels=(const unsigned char *) ReadBlobStream(image,length,
            GetQuantumPixels(quantum_info),&count);
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,1,5);
            if (status == MagickFalse)
              break;
          }
        for (y=0; y < (ssize_t) image->extract_info.height; y++)
        {
          register const Quantum
            *magick_restrict p;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          if (count != (ssize_t) length)
            {
              status=MagickFalse;
              ThrowFileException(exception,CorruptImageError,
                "UnexpectedEndOfFile",image->filename);
              break;
            }
          q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
            exception);
          if (q == (Quantum *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
            quantum_info,GreenQuantum,pixels,exception);
          if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
            break;
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (ssize_t) image->rows))
            {
              p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                image->columns,1,exception);
              if ((p == (const Quantum *) NULL) ||
                  (q == (Quantum *) NULL))
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelGreen(image,GetPixelGreen(canvas_image,p),q);
                p+=GetPixelChannels(canvas_image);
                q+=GetPixelChannels(image);
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
           }
          pixels=(const unsigned char *) ReadBlobStream(image,length,
            GetQuantumPixels(quantum_info),&count);
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,2,5);
            if (status == MagickFalse)
              break;
          }
        for (y=0; y < (ssize_t) image->extract_info.height; y++)
        {
          register const Quantum
            *magick_restrict p;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          if (count != (ssize_t) length)
            {
              status=MagickFalse;
              ThrowFileException(exception,CorruptImageError,
                "UnexpectedEndOfFile",image->filename);
              break;
            }
          q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
            exception);
          if (q == (Quantum *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
            quantum_info,BlueQuantum,pixels,exception);
          if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
            break;
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (ssize_t) image->rows))
            {
              p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                image->columns,1,exception);
              if ((p == (const Quantum *) NULL) ||
                  (q == (Quantum *) NULL))
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelBlue(image,GetPixelBlue(canvas_image,p),q);
                p+=GetPixelChannels(canvas_image);
                q+=GetPixelChannels(image);
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
            }
          pixels=(const unsigned char *) ReadBlobStream(image,length,
            GetQuantumPixels(quantum_info),&count);
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,3,5);
            if (status == MagickFalse)
              break;
          }
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            for (y=0; y < (ssize_t) image->extract_info.height; y++)
            {
              register const Quantum
                *magick_restrict p;

              register Quantum
                *magick_restrict q;

              register ssize_t
                x;

              if (count != (ssize_t) length)
                {
                  status=MagickFalse;
                  ThrowFileException(exception,CorruptImageError,
                    "UnexpectedEndOfFile",image->filename);
                  break;
                }
              q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
                exception);
              if (q == (Quantum *) NULL)
                break;
              length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
                quantum_info,AlphaQuantum,pixels,exception);
              if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
                break;
              if (((y-image->extract_info.y) >= 0) && 
                  ((y-image->extract_info.y) < (ssize_t) image->rows))
                {
                  p=GetVirtualPixels(canvas_image,
                    canvas_image->extract_info.x,0,canvas_image->columns,1,
                    exception);
                  q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                    image->columns,1,exception);
                  if ((p == (const Quantum *) NULL) ||
                      (q == (Quantum *) NULL))
                    break;
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    SetPixelAlpha(image,GetPixelAlpha(canvas_image,p),q);
                    p+=GetPixelChannels(canvas_image);
                    q+=GetPixelChannels(image);
                  }
                  if (SyncAuthenticPixels(image,exception) == MagickFalse)
                    break;
                }
              pixels=(const unsigned char *) ReadBlobStream(image,length,
                GetQuantumPixels(quantum_info),&count);
            }
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,4,5);
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
          Partition interlacing:  YYYYYY..., CbCbCbCbCbCb..., CrCrCrCrCrCr...
        */
        AppendImageFormat("Y",image->filename);
        status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
        if (status == MagickFalse)
          break;
        if (DiscardBlobBytes(image,(MagickSizeType) image->offset) == MagickFalse)
          {
            status=MagickFalse;
            ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
              image->filename);
            break;
          }
        length=GetQuantumExtent(canvas_image,quantum_info,RedQuantum);
        for (i=0; i < (ssize_t) scene; i++)
        {
          for (y=0; y < (ssize_t) image->extract_info.height; y++)
          {
            pixels=(const unsigned char *) ReadBlobStream(image,length,
              GetQuantumPixels(quantum_info),&count);
            if (count != (ssize_t) length)
              break;
          }
          if (count != (ssize_t) length)
            break;
        }
        pixels=(const unsigned char *) ReadBlobStream(image,length,
          GetQuantumPixels(quantum_info),&count);
        for (y=0; y < (ssize_t) image->extract_info.height; y++)
        {
          register const Quantum
            *magick_restrict p;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          if (count != (ssize_t) length)
            {
              status=MagickFalse;
              ThrowFileException(exception,CorruptImageError,
                "UnexpectedEndOfFile",image->filename);
              break;
            }
          q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
            exception);
          if (q == (Quantum *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
            quantum_info,RedQuantum,pixels,exception);
          if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
            break;
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (ssize_t) image->rows))
            {
              p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                image->columns,1,exception);
              if ((p == (const Quantum *) NULL) ||
                  (q == (Quantum *) NULL))
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelRed(image,GetPixelRed(canvas_image,p),q);
                p+=GetPixelChannels(canvas_image);
                q+=GetPixelChannels(image);
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
            }
          pixels=(const unsigned char *) ReadBlobStream(image,length,
            GetQuantumPixels(quantum_info),&count);
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,1,5);
            if (status == MagickFalse)
              break;
          }
        (void) CloseBlob(image);
        AppendImageFormat("Cb",image->filename);
        status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
        if (status == MagickFalse)
          break;
        length=GetQuantumExtent(canvas_image,quantum_info,GreenQuantum);
        for (i=0; i < (ssize_t) scene; i++)
        {
          for (y=0; y < (ssize_t) image->extract_info.height; y++)
          {
            pixels=(const unsigned char *) ReadBlobStream(image,length,
              GetQuantumPixels(quantum_info),&count);
            if (count != (ssize_t) length)
              break;
          }
          if (count != (ssize_t) length)
            break;
        }
        pixels=(const unsigned char *) ReadBlobStream(image,length,
          GetQuantumPixels(quantum_info),&count);
        for (y=0; y < (ssize_t) image->extract_info.height; y++)
        {
          register const Quantum
            *magick_restrict p;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          if (count != (ssize_t) length)
            {
              status=MagickFalse;
              ThrowFileException(exception,CorruptImageError,
                "UnexpectedEndOfFile",image->filename);
              break;
            }
          q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
            exception);
          if (q == (Quantum *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
            quantum_info,GreenQuantum,pixels,exception);
          if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
            break;
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (ssize_t) image->rows))
            {
              p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                image->columns,1,exception);
              if ((p == (const Quantum *) NULL) ||
                  (q == (Quantum *) NULL))
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelGreen(image,GetPixelGreen(canvas_image,p),q);
                p+=GetPixelChannels(canvas_image);
                q+=GetPixelChannels(image);
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
           }
          pixels=(const unsigned char *) ReadBlobStream(image,length,
            GetQuantumPixels(quantum_info),&count);
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,2,5);
            if (status == MagickFalse)
              break;
          }
        (void) CloseBlob(image);
        AppendImageFormat("Cr",image->filename);
        status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
        if (status == MagickFalse)
          break;
        length=GetQuantumExtent(canvas_image,quantum_info,BlueQuantum);
        for (i=0; i < (ssize_t) scene; i++)
        {
          for (y=0; y < (ssize_t) image->extract_info.height; y++)
          {
            pixels=(const unsigned char *) ReadBlobStream(image,length,
              GetQuantumPixels(quantum_info),&count);
            if (count != (ssize_t) length)
              break;
          }
          if (count != (ssize_t) length)
            break;
        }
        pixels=(const unsigned char *) ReadBlobStream(image,length,
          GetQuantumPixels(quantum_info),&count);
        for (y=0; y < (ssize_t) image->extract_info.height; y++)
        {
          register const Quantum
            *magick_restrict p;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          if (count != (ssize_t) length)
            {
              status=MagickFalse;
              ThrowFileException(exception,CorruptImageError,
                "UnexpectedEndOfFile",image->filename);
              break;
            }
          q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
            exception);
          if (q == (Quantum *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
            quantum_info,BlueQuantum,pixels,exception);
          if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
            break;
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (ssize_t) image->rows))
            {
              p=GetVirtualPixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                image->columns,1,exception);
              if ((p == (const Quantum *) NULL) ||
                  (q == (Quantum *) NULL))
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelBlue(image,GetPixelBlue(canvas_image,p),q);
                p+=GetPixelChannels(canvas_image);
                q+=GetPixelChannels(image);
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
           }
          pixels=(const unsigned char *) ReadBlobStream(image,length,
            GetQuantumPixels(quantum_info),&count);
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,LoadImageTag,3,5);
            if (status == MagickFalse)
              break;
          }
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            (void) CloseBlob(image);
            AppendImageFormat("A",image->filename);
            status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
            if (status == MagickFalse)
              break;
            length=GetQuantumExtent(canvas_image,quantum_info,AlphaQuantum);
            for (i=0; i < (ssize_t) scene; i++)
            {
              for (y=0; y < (ssize_t) image->extract_info.height; y++)
              {
                pixels=(const unsigned char *) ReadBlobStream(image,length,
                  GetQuantumPixels(quantum_info),&count);
                if (count != (ssize_t) length)
                  break;
              }
              if (count != (ssize_t) length)
                break;
            }
            pixels=(const unsigned char *) ReadBlobStream(image,length,
              GetQuantumPixels(quantum_info),&count);
            for (y=0; y < (ssize_t) image->extract_info.height; y++)
            {
              register const Quantum
                *magick_restrict p;

              register Quantum
                *magick_restrict q;

              register ssize_t
                x;

              if (count != (ssize_t) length)
                {
                  status=MagickFalse;
                  ThrowFileException(exception,CorruptImageError,
                    "UnexpectedEndOfFile",image->filename);
                  break;
                }
              q=GetAuthenticPixels(canvas_image,0,0,canvas_image->columns,1,
                exception);
              if (q == (Quantum *) NULL)
                break;
              length=ImportQuantumPixels(canvas_image,(CacheView *) NULL,
                quantum_info,BlueQuantum,pixels,exception);
              if (SyncAuthenticPixels(canvas_image,exception) == MagickFalse)
                break;
              if (((y-image->extract_info.y) >= 0) && 
                  ((y-image->extract_info.y) < (ssize_t) image->rows))
                {
                  p=GetVirtualPixels(canvas_image,
                    canvas_image->extract_info.x,0,canvas_image->columns,1,
                    exception);
                  q=GetAuthenticPixels(image,0,y-image->extract_info.y,
                    image->columns,1,exception);
                  if ((p == (const Quantum *) NULL) ||
                      (q == (Quantum *) NULL))
                    break;
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    SetPixelAlpha(image,GetPixelAlpha(canvas_image,p),q);
                    p+=GetPixelChannels(canvas_image);
                    q+=GetPixelChannels(image);
                  }
                  if (SyncAuthenticPixels(image,exception) == MagickFalse)
                    break;
               }
              pixels=(const unsigned char *) ReadBlobStream(image,length,
                GetQuantumPixels(quantum_info),&count);
            }
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,4,5);
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
    }
    if (status == MagickFalse)
      break;
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
    scene++;
  } while (count == (ssize_t) length);
  quantum_info=DestroyQuantumInfo(quantum_info);
  canvas_image=DestroyImage(canvas_image);
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
%   R e g i s t e r Y C b C r I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterYCBCRImage() adds attributes for the YCbCr or YCbCrA image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterYCBCRImage method is:
%
%      size_t RegisterYCBCRImage(void)
%
*/
ModuleExport size_t RegisterYCBCRImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("YCbCr","YCbCr","Raw Y, Cb, and Cr samples");
  entry->decoder=(DecodeImageHandler *) ReadYCBCRImage;
  entry->encoder=(EncodeImageHandler *) WriteYCBCRImage;
  entry->flags|=CoderRawSupportFlag;
  entry->flags|=CoderEndianSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("YCbCr","YCbCrA","Raw Y, Cb, Cr, and alpha samples");
  entry->decoder=(DecodeImageHandler *) ReadYCBCRImage;
  entry->encoder=(EncodeImageHandler *) WriteYCBCRImage;
  entry->flags|=CoderRawSupportFlag;
  entry->flags|=CoderEndianSupportFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r Y C b C r I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterYCBCRImage() removes format registrations made by the
%  YCbCr module from the list of supported formats.
%
%  The format of the UnregisterYCBCRImage method is:
%
%      UnregisterYCBCRImage(void)
%
*/
ModuleExport void UnregisterYCBCRImage(void)
{
  (void) UnregisterMagickInfo("YCbCr");
  (void) UnregisterMagickInfo("YCbCrA");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e Y C b C r I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteYCBCRImage() writes an image to a file in the YCbCr or YCbCrA
%  rasterfile format.
%
%  The format of the WriteYCBCRImage method is:
%
%      MagickBooleanType WriteYCBCRImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteYCBCRImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register const Quantum
    *p;

  size_t
    imageListLength,
    length;

  ssize_t
    count,
    y;

  unsigned char
    *pixels;

  /*
    Allocate memory for pixels.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image_info->interlace != PartitionInterlace)
    {
      /*
        Open output image file.
      */
      assert(exception != (ExceptionInfo *) NULL);
      assert(exception->signature == MagickCoreSignature);
      status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
      if (status == MagickFalse)
        return(status);
    }
  quantum_type=RGBQuantum;
  if (LocaleCompare(image_info->magick,"YCbCrA") == 0)
    {
      quantum_type=RGBAQuantum;
      image->alpha_trait=BlendPixelTrait;
    }
  scene=0;
  imageListLength=GetImageListLength(image);
  do
  {
    /*
      Convert MIFF to YCbCr raster pixels.
    */
    if (image->colorspace != YCbCrColorspace)
      (void) TransformImageColorspace(image,YCbCrColorspace,exception);
    if ((LocaleCompare(image_info->magick,"YCbCrA") == 0) &&
        (image->alpha_trait == UndefinedPixelTrait))
      (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=(unsigned char *) GetQuantumPixels(quantum_info);
    switch (image_info->interlace)
    {
      case NoInterlace:
      default:
      {
        /*
          No interlacing:  YCbCrYCbCrYCbCrYCbCrYCbCrYCbCr...
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
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
        break;
      }
      case LineInterlace:
      {
        /*
          Line interlacing:  YYY...CbCbCb...CrCrCr...YYY...CbCbCb...CrCrCr...
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            RedQuantum,pixels,exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            GreenQuantum,pixels,exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            BlueQuantum,pixels,exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
          if (quantum_type == RGBAQuantum)
            {
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                AlphaQuantum,pixels,exception);
              count=WriteBlob(image,length,pixels);
              if (count != (ssize_t) length)
                break;
            }
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case PlaneInterlace:
      {
        /*
          Plane interlacing:  YYYYYY...CbCbCbCbCbCb...CrCrCrCrCrCr...
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            RedQuantum,pixels,exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,SaveImageTag,1,5);
            if (status == MagickFalse)
              break;
          }
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            GreenQuantum,pixels,exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,SaveImageTag,2,5);
            if (status == MagickFalse)
              break;
          }
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            BlueQuantum,pixels,exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,SaveImageTag,3,5);
            if (status == MagickFalse)
              break;
          }
        if (quantum_type == RGBAQuantum)
          {
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                AlphaQuantum,pixels,exception);
              count=WriteBlob(image,length,pixels);
              if (count != (ssize_t) length)
              break;
            }
          }
        if (image_info->interlace == PartitionInterlace)
          (void) CopyMagickString(image->filename,image_info->filename,
            MagickPathExtent);
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
        /*
          Partition interlacing:  YYYYYY..., CbCbCbCbCbCb..., CrCrCrCrCrCr...
        */
        AppendImageFormat("Y",image->filename);
        status=OpenBlob(image_info,image,scene == 0 ? WriteBinaryBlobMode :
          AppendBinaryBlobMode,exception);
        if (status == MagickFalse)
          return(status);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            RedQuantum,pixels,exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,SaveImageTag,1,5);
            if (status == MagickFalse)
              break;
          }
        (void) CloseBlob(image);
        AppendImageFormat("Cb",image->filename);
        status=OpenBlob(image_info,image,scene == 0 ? WriteBinaryBlobMode :
          AppendBinaryBlobMode,exception);
        if (status == MagickFalse)
          return(status);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            GreenQuantum,pixels,exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,SaveImageTag,2,5);
            if (status == MagickFalse)
              break;
          }
        (void) CloseBlob(image);
        AppendImageFormat("Cr",image->filename);
        status=OpenBlob(image_info,image,scene == 0 ? WriteBinaryBlobMode :
          AppendBinaryBlobMode,exception);
        if (status == MagickFalse)
          return(status);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            BlueQuantum,pixels,exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
        }
        if (image->previous == (Image *) NULL)
          {
            status=SetImageProgress(image,SaveImageTag,3,5);
            if (status == MagickFalse)
              break;
          }
        if (quantum_type == RGBAQuantum)
          {
            (void) CloseBlob(image);
            AppendImageFormat("A",image->filename);
            status=OpenBlob(image_info,image,scene == 0 ? WriteBinaryBlobMode :
              AppendBinaryBlobMode,exception);
            if (status == MagickFalse)
              return(status);
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                AlphaQuantum,pixels,exception);
              count=WriteBlob(image,length,pixels);
              if (count != (ssize_t) length)
                break;
            }
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,SaveImageTag,4,5);
                if (status == MagickFalse)
                  break;
              }
          }
        (void) CloseBlob(image);
        (void) CopyMagickString(image->filename,image_info->filename,
          MagickPathExtent);
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
    status=SetImageProgress(image,SaveImagesTag,scene++,imageListLength);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}

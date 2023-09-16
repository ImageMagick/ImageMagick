/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   N   N  M   M                              %
%                            P   P  NN  N  MM MM                              %
%                            PPPP   N N N  M M M                              %
%                            P      N  NN  M   M                              %
%                            P      N   N  M   M                              %
%                                                                             %
%                                                                             %
%               Read/Write PBMPlus Portable Anymap Image Format               %
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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
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
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "coders/coders-private.h"

/*
  Typedef declarations.
*/
typedef struct _CommentInfo
{
  char
    *comment;

  size_t
    extent;
} CommentInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePNMImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P N M                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPNM() returns MagickTrue if the image format type, identified by the
%  magick string, is PNM.
%
%  The format of the IsPNM method is:
%
%      MagickBooleanType IsPNM(const unsigned char *magick,const size_t extent)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o extent: Specifies the extent of the magick string.
%
*/
static MagickBooleanType IsPNM(const unsigned char *magick,const size_t extent)
{
  if (extent < 2)
    return(MagickFalse);
  if ((*magick == (unsigned char) 'P') &&
      ((magick[1] == '1') || (magick[1] == '2') || (magick[1] == '3') ||
       (magick[1] == '4') || (magick[1] == '5') || (magick[1] == '6') ||
       (magick[1] == '7') || (magick[1] == 'F') || (magick[1] == 'f') ||
       (magick[1] == 'H') || (magick[1] == 'h')))
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P N M I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPNMImage() reads a Portable Anymap image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the ReadPNMImage method is:
%
%      Image *ReadPNMImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static int PNMComment(Image *image,CommentInfo *comment_info,
  ExceptionInfo *exception)
{
  int
    c;

  char
    *p;

  /*
    Read comment.
  */
  (void) exception;
  p=comment_info->comment+strlen(comment_info->comment);
  for (c='#'; (c != EOF) && (c != (int) '\n') && (c != (int) '\r'); p++)
  {
    if ((size_t) (p-comment_info->comment+1) >= comment_info->extent)
      {
        comment_info->extent<<=1;
        comment_info->comment=(char *) ResizeQuantumMemory(
          comment_info->comment,comment_info->extent,
          sizeof(*comment_info->comment));
        if (comment_info->comment == (char *) NULL)
          return(-1);
        p=comment_info->comment+strlen(comment_info->comment);
      }
    c=ReadBlobByte(image);
    if (c != EOF)
      {
        *p=(char) c;
        *(p+1)='\0';
      }
  }
  return(c);
}

static unsigned int PNMInteger(Image *image,CommentInfo *comment_info,
  const unsigned int base,ExceptionInfo *exception)
{
  int
    c;

  unsigned int
    value;

  /*
    Skip any leading whitespace.
  */
  do
  {
    c=ReadBlobByte(image);
    if (c == EOF)
      return(0);
    if (c == (int) '#')
      c=PNMComment(image,comment_info,exception);
  } while ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'));
  if (base == 2)
    return((unsigned int) (c-(int) '0'));
  /*
    Evaluate number.
  */
  value=0;
  while (isdigit((int) ((unsigned char) c)) != 0)
  {
    if (value <= (unsigned int) (INT_MAX/10))
      {
        value*=10;
        if (value <= (unsigned int) (INT_MAX-(c-(int) '0')))
          value+=(unsigned int) (c-(int) '0');
      }
    c=ReadBlobByte(image);
    if (c == EOF)
      return(0);
  }
  if (c == (int) '#')
    c=PNMComment(image,comment_info,exception);
  return(value);
}

static char *PNMString(Image *image,char *string,const size_t extent)
{
  int
    c;

  size_t
    i;

  for (i=0; i < (extent-1L); i++)
  {
    c=ReadBlobByte(image);
    if (c == EOF)
      {
        if (i == 0)
          return((char *) NULL);
        break;
      }
    string[i]=c;
    if (c == '\n' || c == '\r')
      break;
  }
  string[i]='\0';
  return(string);
}

static Image *ReadPNMImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define ThrowPNMException(exception,message) \
{ \
  if (comment_info.comment != (char *) NULL)  \
    comment_info.comment=DestroyString(comment_info.comment); \
  if (quantum_info != (QuantumInfo *) NULL) \
    quantum_info=DestroyQuantumInfo(quantum_info); \
  ThrowReaderException((exception),(message)); \
}

  char
    format;

  ColorspaceType
    colorspace;

  CommentInfo
    comment_info;

  const void
    *stream;

  double
    quantum_scale;

  Image
    *image;

  MagickBooleanType
    is_gray = MagickTrue,
    is_mono = MagickTrue,
    status;

  QuantumAny
    max_value;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  size_t
    depth,
    extent;

  ssize_t
    count,
    row,
    y;

  unsigned char
    *pixels;

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
  /*
    Read PNM image.
  */
  comment_info.comment=(char *) NULL;
  quantum_info=(QuantumInfo *) NULL;
  count=ReadBlob(image,1,(unsigned char *) &format);
  do
  {
    /*
      Initialize image structure.
    */
    comment_info.comment=AcquireString(NULL);
    comment_info.extent=MagickPathExtent;
    if ((count != 1) || (format != 'P'))
      ThrowPNMException(CorruptImageError,"ImproperImageHeader");
    max_value=1;
    colorspace=UndefinedColorspace;
    quantum_type=UndefinedQuantum;
    quantum_scale=1.0;
    format=(char) ReadBlobByte(image);
    if (format != '7')
      {
        /*
          PBM, PGM, PPM, and PNM.
        */
        if (ReadBlobByte(image) == '4')
          image->alpha_trait=BlendPixelTrait;
        image->columns=(size_t) PNMInteger(image,&comment_info,10,exception);
        image->rows=(size_t) PNMInteger(image,&comment_info,10,exception);
        if ((format == 'f') || (format == 'F') || (format == 'h') ||
            (format == 'H'))
          {
            char
              scale[32];

            if (PNMString(image,scale,sizeof(scale)) != (char *) NULL)
              quantum_scale=StringToDouble(scale,(char **) NULL);
          }
        else
          {
            if ((format == '1') || (format == '4'))
              max_value=1;  /* bitmap */
            else
              max_value=(QuantumAny) PNMInteger(image,&comment_info,10,
                exception);
          }
      }
    else
      {
        char
          keyword[MagickPathExtent],
          value[MagickPathExtent];

        int
          c;

        char
          *p;

        /*
          PAM.
        */
        for (c=ReadBlobByte(image); c != EOF; c=ReadBlobByte(image))
        {
          while (isspace((int) ((unsigned char) c)) != 0)
            c=ReadBlobByte(image);
          if (c == '#')
            {
              /*
                Comment.
              */
              while (c == '#')
              {
                c=PNMComment(image,&comment_info,exception);
                c=ReadBlobByte(image);
              }
              while (isspace((int) ((unsigned char) c)) != 0)
                c=ReadBlobByte(image);
            }
          p=keyword;
          do
          {
            if ((size_t) (p-keyword) < (MagickPathExtent-1))
              *p++=c;
            c=ReadBlobByte(image);
          } while (isalnum((int) ((unsigned char) c)));
          *p='\0';
          if (LocaleCompare(keyword,"endhdr") == 0)
            break;
          while (isspace((int) ((unsigned char) c)) != 0)
            c=ReadBlobByte(image);
          p=value;
          while (isalnum((int) ((unsigned char) c)) || (c == '_'))
          {
            if ((size_t) (p-value) < (MagickPathExtent-1))
              *p++=c;
            c=ReadBlobByte(image);
          }
          *p='\0';
          /*
            Assign a value to the specified keyword.
          */
          if (LocaleCompare(keyword,"depth") == 0)
            (void) StringToUnsignedLong(value);
          if (LocaleCompare(keyword,"height") == 0)
            image->rows=StringToUnsignedLong(value);
          if (LocaleCompare(keyword,"maxval") == 0)
            max_value=StringToUnsignedLong(value);
          if ((quantum_type == UndefinedQuantum) &&
              (LocaleCompare(keyword,"TUPLTYPE") == 0))
            {
              if (LocaleCompare(value,"BLACKANDWHITE") == 0)
                {
                  colorspace=GRAYColorspace;
                  quantum_type=GrayQuantum;
                }
              if (LocaleCompare(value,"BLACKANDWHITE_ALPHA") == 0)
                {
                  colorspace=GRAYColorspace;
                  image->alpha_trait=BlendPixelTrait;
                  quantum_type=GrayAlphaQuantum;
                }
              if (LocaleCompare(value,"GRAYSCALE") == 0)
                {
                  colorspace=GRAYColorspace;
                  quantum_type=GrayQuantum;
                }
              if (LocaleCompare(value,"GRAYSCALE_ALPHA") == 0)
                {
                  colorspace=GRAYColorspace;
                  image->alpha_trait=BlendPixelTrait;
                  quantum_type=GrayAlphaQuantum;
                }
              if (LocaleCompare(value,"RGB_ALPHA") == 0)
                {
                  image->alpha_trait=BlendPixelTrait;
                  quantum_type=RGBAQuantum;
                }
              if (LocaleCompare(value,"CMYK") == 0)
                {
                  colorspace=CMYKColorspace;
                  quantum_type=CMYKQuantum;
                }
              if (LocaleCompare(value,"CMYK_ALPHA") == 0)
                {
                  colorspace=CMYKColorspace;
                  image->alpha_trait=BlendPixelTrait;
                  quantum_type=CMYKAQuantum;
                }
            }
          if (LocaleCompare(keyword,"width") == 0)
            image->columns=StringToUnsignedLong(value);
        }
      }
    if (quantum_type == UndefinedQuantum)
      quantum_type=RGBQuantum;
    if ((image->columns == 0) || (image->rows == 0))
      ThrowPNMException(CorruptImageError,"NegativeOrZeroImageSize");
    if ((max_value == 0) || (max_value > 4294967295UL))
      ThrowPNMException(CorruptImageError,"ImproperImageHeader");
    for (depth=1; GetQuantumRange(depth) < max_value; depth++) ;
    image->depth=depth;
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if ((MagickSizeType) (image->columns*image->rows/8) > GetBlobSize(image))
      ThrowPNMException(CorruptImageError,"InsufficientImageDataInFile");
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      {
        if (comment_info.comment != (char *) NULL)
          comment_info.comment=DestroyString(comment_info.comment);
        if (quantum_info != (QuantumInfo *) NULL)
          quantum_info=DestroyQuantumInfo(quantum_info);
        return(DestroyImageList(image));
      }
    if (colorspace != UndefinedColorspace)
      (void) SetImageColorspace(image,colorspace,exception);
    (void) ResetImagePixels(image,exception);
    /*
      Convert PNM pixels to runextent-encoded MIFF packets.
    */
    row=0;
    y=0;
    switch (format)
    {
      case '1':
      {
        /*
          Convert PBM image to pixel packets.
        */
        (void) SetImageColorspace(image,GRAYColorspace,exception);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          ssize_t
            x;

          Quantum
            *magick_restrict q;

          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelGray(image,PNMInteger(image,&comment_info,2,exception) ==
              0 ? QuantumRange : 0,q);
            if (EOFBlob(image) != MagickFalse)
              break;
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
          if (EOFBlob(image) != MagickFalse)
            break;
        }
        image->type=BilevelType;
        break;
      }
      case '2':
      {
        Quantum
          intensity;

        /*
          Convert PGM image to pixel packets.
        */
        (void) SetImageColorspace(image,GRAYColorspace,exception);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          ssize_t
            x;

          Quantum
            *magick_restrict q;

          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            intensity=ScaleAnyToQuantum(PNMInteger(image,&comment_info,10,
              exception),max_value);
            if (EOFBlob(image) != MagickFalse)
              break;
            SetPixelGray(image,intensity,q);
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
          if (EOFBlob(image) != MagickFalse)
            break;
        }
        image->type=GrayscaleType;
        break;
      }
      case '3':
      {
        /*
          Convert PNM image to pixel packets.
        */
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          Quantum
            *magick_restrict q;

          ssize_t
            x;

          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            Quantum
              pixel;

            pixel=ScaleAnyToQuantum(PNMInteger(image,&comment_info,10,
              exception),max_value);
            if (EOFBlob(image) != MagickFalse)
              break;
            SetPixelRed(image,pixel,q);
            pixel=ScaleAnyToQuantum(PNMInteger(image,&comment_info,10,
              exception),max_value);
            SetPixelGreen(image,pixel,q);
            pixel=ScaleAnyToQuantum(PNMInteger(image,&comment_info,10,
              exception),max_value);
            SetPixelBlue(image,pixel,q);
            if ((is_gray != MagickFalse) &&
                (IsPixelGray(image,q) == MagickFalse))
              is_gray=MagickFalse;
            if ((is_mono != MagickFalse) &&
                (IsPixelMonochrome(image,q) == MagickFalse))
              is_mono=MagickFalse;
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
          if (EOFBlob(image) != MagickFalse)
            break;
        }
        if (is_gray != MagickFalse)
          image->type=GrayscaleType;
        if (is_mono != MagickFalse)
          image->type=BilevelType;
        break;
      }
      case '4':
      {
        /*
          Convert PBM raw image to pixel packets.
        */
        (void) SetImageColorspace(image,GRAYColorspace,exception);
        quantum_type=GrayQuantum;
        if (image->storage_class == PseudoClass)
          quantum_type=IndexQuantum;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowPNMException(ResourceLimitError,"MemoryAllocationFailed");
        SetQuantumMinIsWhite(quantum_info,MagickTrue);
        extent=GetQuantumExtent(image,quantum_info,quantum_type);
        pixels=GetQuantumPixels(quantum_info);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          MagickBooleanType
            sync;

          Quantum
            *magick_restrict q;

          ssize_t
            offset;

          size_t
            length;

          stream=ReadBlobStream(image,extent,pixels,&count);
          if (count != (ssize_t) extent)
            break;
          if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
              (image->previous == (Image *) NULL))
            {
              MagickBooleanType
                proceed;

              proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                row,image->rows);
              if (proceed == MagickFalse)
                break;
            }
          offset=row++;
          q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          length=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,(unsigned char *) stream,exception);
          if (length != extent)
            break;
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            break;
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        SetQuantumImageType(image,quantum_type);
        break;
      }
      case '5':
      {
        /*
          Convert PGM raw image to pixel packets.
        */
        (void) SetImageColorspace(image,GRAYColorspace,exception);
        quantum_type=GrayQuantum;
        extent=(image->depth <= 8 ? 1 : image->depth <= 16 ? 2 : 4)*
          image->columns;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowPNMException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=GetQuantumPixels(quantum_info);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const unsigned char
            *magick_restrict p;

          MagickBooleanType
            sync;

          Quantum
            *magick_restrict q;

          ssize_t
            offset,
            x;

          stream=ReadBlobStream(image,extent,pixels,&count);
          if (count != (ssize_t) extent)
            break;
          if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
              (image->previous == (Image *) NULL))
            {
              MagickBooleanType
                proceed;

              proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                row,image->rows);
              if (proceed == MagickFalse)
                break;
            }
          offset=row++;
          q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          p=(unsigned char *) stream;
          switch (image->depth)
          {
            case 8:
            case 16:
            case 32:
            {
              (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                quantum_type,(unsigned char *) stream,exception);
              break;
            }
            default:
            {
              switch (image->depth)
              {
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                {
                  unsigned char
                    pixel;

                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    p=PushCharPixel(p,&pixel);
                    SetPixelGray(image,ScaleAnyToQuantum(pixel,max_value),q);
                    q+=GetPixelChannels(image);
                  }
                  break;
                }
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                {
                  unsigned short
                    pixel;

                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    p=PushShortPixel(MSBEndian,p,&pixel);
                    SetPixelGray(image,ScaleAnyToQuantum(pixel,max_value),q);
                    q+=GetPixelChannels(image);
                  }
                  break;
                }
                default:
                {
                  unsigned int
                    pixel;

                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    p=PushLongPixel(MSBEndian,p,&pixel);
                    SetPixelGray(image,ScaleAnyToQuantum(pixel,max_value),q);
                    q+=GetPixelChannels(image);
                  }
                  break;
                }
              }
              break;
            }
          }
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            break;
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        SetQuantumImageType(image,quantum_type);
        break;
      }
      case '6':
      {
        /*
          Convert PNM raster image to pixel packets.
        */
        quantum_type=RGBQuantum;
        extent=3*(size_t) (image->depth <= 8 ? 1 : image->depth <= 16 ? 2 : 4)*
          image->columns;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowPNMException(ResourceLimitError,"MemoryAllocationFailed");
        (void) SetQuantumEndian(image,quantum_info,MSBEndian);
        pixels=GetQuantumPixels(quantum_info);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const unsigned char
            *magick_restrict p;

          MagickBooleanType
            sync;

          Quantum
            *magick_restrict q;

          ssize_t
            offset,
            x;

          stream=ReadBlobStream(image,extent,pixels,&count);
          if (count != (ssize_t) extent)
            break;
          if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
              (image->previous == (Image *) NULL))
            {
              MagickBooleanType
                proceed;

              proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                row,image->rows);
              if (proceed == MagickFalse)
                break;
            }
          offset=row++;
          q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          p=(unsigned char *) stream;
          switch (image->depth)
          {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            {
              unsigned char
                pixel;

              for (x=0; x < (ssize_t) image->columns; x++)
              {
                p=PushCharPixel(p,&pixel);
                SetPixelRed(image,ScaleAnyToQuantum(pixel,max_value),q);
                p=PushCharPixel(p,&pixel);
                SetPixelGreen(image,ScaleAnyToQuantum(pixel,max_value),q);
                p=PushCharPixel(p,&pixel);
                SetPixelBlue(image,ScaleAnyToQuantum(pixel,max_value),q);
                SetPixelAlpha(image,OpaqueAlpha,q);
                if ((is_gray != MagickFalse) &&
                    (IsPixelGray(image,q) == MagickFalse))
                  is_gray=MagickFalse;
                if ((is_mono != MagickFalse) &&
                    (IsPixelMonochrome(image,q) == MagickFalse))
                  is_mono=MagickFalse;
                q+=GetPixelChannels(image);
              }
              break;
            }
            case 8:
            {
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelRed(image,ScaleCharToQuantum(*p++),q);
                SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
                SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
                SetPixelAlpha(image,OpaqueAlpha,q);
                if ((is_gray != MagickFalse) &&
                    (IsPixelGray(image,q) == MagickFalse))
                  is_gray=MagickFalse;
                if ((is_mono != MagickFalse) &&
                    (IsPixelMonochrome(image,q) == MagickFalse))
                  is_mono=MagickFalse;
                q+=GetPixelChannels(image);
              }
              break;
            }
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            {
              unsigned short
                pixel;

              for (x=0; x < (ssize_t) image->columns; x++)
              {
                p=PushShortPixel(MSBEndian,p,&pixel);
                SetPixelRed(image,ScaleAnyToQuantum(pixel,max_value),q);
                p=PushShortPixel(MSBEndian,p,&pixel);
                SetPixelGreen(image,ScaleAnyToQuantum(pixel,max_value),q);
                p=PushShortPixel(MSBEndian,p,&pixel);
                SetPixelBlue(image,ScaleAnyToQuantum(pixel,max_value),q);
                SetPixelAlpha(image,OpaqueAlpha,q);
                if ((is_gray != MagickFalse) &&
                    (IsPixelGray(image,q) == MagickFalse))
                  is_gray=MagickFalse;
                if ((is_mono != MagickFalse) &&
                    (IsPixelMonochrome(image,q) == MagickFalse))
                  is_mono=MagickFalse;
                q+=GetPixelChannels(image);
              }
              break;
            }
            case 16:
            {
              unsigned short
                pixel;

              for (x=0; x < (ssize_t) image->columns; x++)
              {
                p=PushShortPixel(MSBEndian,p,&pixel);
                SetPixelRed(image,ScaleShortToQuantum(pixel),q);
                p=PushShortPixel(MSBEndian,p,&pixel);
                SetPixelGreen(image,ScaleShortToQuantum(pixel),q);
                p=PushShortPixel(MSBEndian,p,&pixel);
                SetPixelBlue(image,ScaleShortToQuantum(pixel),q);
                SetPixelAlpha(image,OpaqueAlpha,q);
                if ((is_gray != MagickFalse) &&
                    (IsPixelGray(image,q) == MagickFalse))
                  is_gray=MagickFalse;
                if ((is_mono != MagickFalse) &&
                    (IsPixelMonochrome(image,q) == MagickFalse))
                  is_mono=MagickFalse;
                q+=GetPixelChannels(image);
              }
              break;
            }
            case 32:
            {
              unsigned int
                pixel;

              for (x=0; x < (ssize_t) image->columns; x++)
              {
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelRed(image,ScaleLongToQuantum(pixel),q);
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelGreen(image,ScaleLongToQuantum(pixel),q);
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelBlue(image,ScaleLongToQuantum(pixel),q);
                SetPixelAlpha(image,OpaqueAlpha,q);
                if ((is_gray != MagickFalse) &&
                    (IsPixelGray(image,q) == MagickFalse))
                  is_gray=MagickFalse;
                if ((is_mono != MagickFalse) &&
                    (IsPixelMonochrome(image,q) == MagickFalse))
                  is_mono=MagickFalse;
                q+=GetPixelChannels(image);
              }
              break;
            }
            default:
            {
              unsigned int
                pixel;

              for (x=0; x < (ssize_t) image->columns; x++)
              {
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelRed(image,ScaleAnyToQuantum(pixel,max_value),q);
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelGreen(image,ScaleAnyToQuantum(pixel,max_value),q);
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelBlue(image,ScaleAnyToQuantum(pixel,max_value),q);
                SetPixelAlpha(image,OpaqueAlpha,q);
                if ((is_gray != MagickFalse) &&
                    (IsPixelGray(image,q) == MagickFalse))
                  is_gray=MagickFalse;
                if ((is_mono != MagickFalse) &&
                    (IsPixelMonochrome(image,q) == MagickFalse))
                  is_mono=MagickFalse;
                q+=GetPixelChannels(image);
              }
              break;
            }
          }
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            break;
        }
        if (is_gray != MagickFalse)
          image->type=GrayscaleType;
        if (is_mono != MagickFalse)
          image->type=BilevelType;
        quantum_info=DestroyQuantumInfo(quantum_info);
        break;
      }
      case '7':
      {
        size_t
          channels;

        /*
          Convert PAM raster image to pixel packets.
        */
        switch (quantum_type)
        {
          case GrayQuantum:
          case GrayAlphaQuantum:
          {
            channels=1;
            break;
          }
          case CMYKQuantum:
          case CMYKAQuantum:
          {
            channels=4;
            break;
          }
          default:
          {
            channels=3;
            break;
          }
        }
        if (image->alpha_trait != UndefinedPixelTrait)
          channels++;
        extent=channels*(image->depth <= 8 ? 1 : image->depth <= 16 ? 2 : 4)*
          image->columns;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowPNMException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=GetQuantumPixels(quantum_info);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const unsigned char
            *magick_restrict p;

          MagickBooleanType
            sync;

          Quantum
            *magick_restrict q;

          ssize_t
            offset,
            x;

          stream=ReadBlobStream(image,extent,pixels,&count);
          if (count != (ssize_t) extent)
            break;
          if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
              (image->previous == (Image *) NULL))
            {
              MagickBooleanType
                proceed;

              proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                row,image->rows);
              if (proceed == MagickFalse)
                break;
            }
          offset=row++;
          q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          p=(unsigned char *) stream;
          switch (image->depth)
          {
            case 8:
            case 16:
            case 32:
            {
              (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                quantum_type,(unsigned char *) stream,exception);
              break;
            }
            default:
            {
              switch (quantum_type)
              {
                case GrayQuantum:
                case GrayAlphaQuantum:
                {
                  switch (image->depth)
                  {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    {
                      unsigned char
                        pixel;
 
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushCharPixel(p,&pixel);
                        SetPixelGray(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        SetPixelAlpha(image,OpaqueAlpha,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            p=PushCharPixel(p,&pixel);
                            if (image->depth != 1)
                              SetPixelAlpha(image,ScaleAnyToQuantum(pixel,
                                max_value),q);
                            else
                              SetPixelAlpha(image,QuantumRange-
                                ScaleAnyToQuantum(pixel,max_value),q);
                          }
                        q+=GetPixelChannels(image);
                      }
                      break;
                    }
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    {
                      unsigned short
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelGray(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        SetPixelAlpha(image,OpaqueAlpha,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            p=PushShortPixel(MSBEndian,p,&pixel);
                            SetPixelAlpha(image,ScaleAnyToQuantum(pixel,
                              max_value),q);
                          }
                        q+=GetPixelChannels(image);
                      }
                      break;
                    }
                    default:
                    {
                      unsigned int
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelGray(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        SetPixelAlpha(image,OpaqueAlpha,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            p=PushLongPixel(MSBEndian,p,&pixel);
                            SetPixelAlpha(image,ScaleAnyToQuantum(pixel,
                              max_value),q);
                          }
                        q+=GetPixelChannels(image);
                      }
                    }
                  }
                  break;
                }
                case CMYKQuantum:
                case CMYKAQuantum:
                {
                  switch (image->depth)
                  {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    {
                      unsigned char
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushCharPixel(p,&pixel);
                        SetPixelRed(image,ScaleAnyToQuantum(pixel,max_value),q);
                        p=PushCharPixel(p,&pixel);
                        SetPixelGreen(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushCharPixel(p,&pixel);
                        SetPixelBlue(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushCharPixel(p,&pixel);
                        SetPixelBlack(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        SetPixelAlpha(image,OpaqueAlpha,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            p=PushCharPixel(p,&pixel);
                            SetPixelAlpha(image,ScaleAnyToQuantum(pixel,
                              max_value),q);
                          }
                        q+=GetPixelChannels(image);
                      }
                      break;
                    }
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    {
                      unsigned short
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelRed(image,ScaleAnyToQuantum(pixel,max_value),q);
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelGreen(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelBlue(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelBlack(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        SetPixelAlpha(image,OpaqueAlpha,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            p=PushShortPixel(MSBEndian,p,&pixel);
                            SetPixelAlpha(image,ScaleAnyToQuantum(pixel,
                              max_value),q);
                          }
                        q+=GetPixelChannels(image);
                      }
                      break;
                    }
                    default:
                    {
                      unsigned int
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelRed(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelGreen(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelBlue(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelBlack(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        SetPixelAlpha(image,OpaqueAlpha,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            p=PushLongPixel(MSBEndian,p,&pixel);
                            SetPixelAlpha(image,ScaleAnyToQuantum(pixel,
                              max_value),q);
                          }
                        q+=GetPixelChannels(image);
                      }
                      break;
                    }
                  }
                  break;
                }
                default:
                {
                  switch (image->depth)
                  {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    {
                      unsigned char
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushCharPixel(p,&pixel);
                        SetPixelRed(image,ScaleAnyToQuantum(pixel,max_value),q);
                        p=PushCharPixel(p,&pixel);
                        SetPixelGreen(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushCharPixel(p,&pixel);
                        SetPixelBlue(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        SetPixelAlpha(image,OpaqueAlpha,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            p=PushCharPixel(p,&pixel);
                            SetPixelAlpha(image,ScaleAnyToQuantum(pixel,
                              max_value),q);
                          }
                        q+=GetPixelChannels(image);
                      }
                      break;
                    }
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    {
                      unsigned short
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelRed(image,ScaleAnyToQuantum(pixel,max_value),q);
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelGreen(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelBlue(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        SetPixelAlpha(image,OpaqueAlpha,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            p=PushShortPixel(MSBEndian,p,&pixel);
                            SetPixelAlpha(image,ScaleAnyToQuantum(pixel,
                              max_value),q);
                          }
                        q+=GetPixelChannels(image);
                      }
                      break;
                    }
                    default:
                    {
                      unsigned int
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelRed(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelGreen(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelBlue(image,ScaleAnyToQuantum(pixel,max_value),
                          q);
                        SetPixelAlpha(image,OpaqueAlpha,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            p=PushLongPixel(MSBEndian,p,&pixel);
                            SetPixelAlpha(image,ScaleAnyToQuantum(pixel,
                              max_value),q);
                          }
                        q+=GetPixelChannels(image);
                      }
                      break;
                    }
                  }
                  break;
                }
              }
            }
          }
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            break;
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        SetQuantumImageType(image,quantum_type);
        break;
      }
      case 'F':
      case 'f':
      {
        /*
          Convert PFM raster image to pixel packets.
        */
        if (format != 'f')
          quantum_type=image->alpha_trait != UndefinedPixelTrait ? RGBAQuantum :
            RGBQuantum;
        else
          {
            (void) SetImageColorspace(image,GRAYColorspace,exception);
            quantum_type=GrayQuantum;
          }
        image->endian=quantum_scale < 0.0 ? LSBEndian : MSBEndian;
        image->depth=32;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowPNMException(ResourceLimitError,"MemoryAllocationFailed");
        status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
        if (status == MagickFalse)
          ThrowPNMException(ResourceLimitError,"MemoryAllocationFailed");
        SetQuantumScale(quantum_info,(double) QuantumRange*fabs(quantum_scale));
        extent=GetQuantumExtent(image,quantum_info,quantum_type);
        pixels=GetQuantumPixels(quantum_info);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          MagickBooleanType
            sync;

          Quantum
            *magick_restrict q;

          size_t
            length;

          ssize_t
            offset;

          stream=ReadBlobStream(image,extent,pixels,&count);
          if (count != (ssize_t) extent)
            break;
          if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
              (image->previous == (Image *) NULL))
            {
              MagickBooleanType
                proceed;

              proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                row,image->rows);
              if (proceed == MagickFalse)
                break;
            }
          offset=row++;
          q=QueueAuthenticPixels(image,0,((ssize_t) image->rows-offset-1),
            image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          length=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,(unsigned char *) stream,exception);
          if (length != extent)
            break;
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            break;
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        SetQuantumImageType(image,quantum_type);
        break;
      }
      case 'H':
      case 'h':
      {
        /*
          Convert PFM raster image to pixel packets.
        */
        if (format != 'h')
          quantum_type=image->alpha_trait != UndefinedPixelTrait ? RGBAQuantum :
            RGBQuantum;
        else
          {
            (void) SetImageColorspace(image,GRAYColorspace,exception);
            quantum_type=GrayQuantum;
          }
        image->endian=quantum_scale < 0.0 ? LSBEndian : MSBEndian;
        image->depth=16;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowPNMException(ResourceLimitError,"MemoryAllocationFailed");
        status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
        if (status == MagickFalse)
          ThrowPNMException(ResourceLimitError,"MemoryAllocationFailed");
        SetQuantumScale(quantum_info,(double) QuantumRange*fabs(quantum_scale));
        extent=GetQuantumExtent(image,quantum_info,quantum_type);
        pixels=GetQuantumPixels(quantum_info);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          MagickBooleanType
            sync;

          Quantum
            *magick_restrict q;

          size_t
            length;

          ssize_t
            offset;

          stream=ReadBlobStream(image,extent,pixels,&count);
          if (count != (ssize_t) extent)
            break;
          if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
              (image->previous == (Image *) NULL))
            {
              MagickBooleanType
                proceed;

              proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                row,image->rows);
              if (proceed == MagickFalse)
                break;
            }
          offset=row++;
          q=QueueAuthenticPixels(image,0,((ssize_t) image->rows-offset-1),
            image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          length=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,(unsigned char *) stream,exception);
          if (length != extent)
            break;
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            break;
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        SetQuantumImageType(image,quantum_type);
        break;
      }
      default:
        ThrowPNMException(CorruptImageError,"ImproperImageHeader");
    }
    if (*comment_info.comment != '\0')
      (void) SetImageProperty(image,"comment",comment_info.comment,exception);
    comment_info.comment=DestroyString(comment_info.comment);
    if (y < (ssize_t) image->rows)
      ThrowPNMException(CorruptImageError,"UnableToReadImageData");
    if (EOFBlob(image) != MagickFalse)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"UnexpectedEndOfFile","`%s'",image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if ((format == '1') || (format == '2') || (format == '3'))
      do
      {
        /*
          Skip to end of line.
        */
        count=ReadBlob(image,1,(unsigned char *) &format);
        if (count != 1)
          break;
        if (format == 'P')
          break;
      } while (format != '\n');
    count=ReadBlob(image,1,(unsigned char *) &format);
    if ((count == 1) && (format == 'P'))
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
  } while ((count == 1) && (format == 'P'));
  (void) CloseBlob(image);
  if (comment_info.comment != (char *) NULL)
    comment_info.comment=DestroyString(comment_info.comment);
  if (quantum_info != (QuantumInfo *) NULL)
    quantum_info=DestroyQuantumInfo(quantum_info);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P N M I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPNMImage() adds properties for the PNM image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPNMImage method is:
%
%      size_t RegisterPNMImage(void)
%
*/
ModuleExport size_t RegisterPNMImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("PNM","PAM","Common 2-dimensional bitmap format");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->mime_type=ConstantString("image/x-portable-anymap");
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PNM","PBM",
    "Portable bitmap format (black and white)");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->mime_type=ConstantString("image/x-portable-bitmap");
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PNM","PFM","Portable float format");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->flags|=CoderEndianSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PNM","PGM","Portable graymap format (gray scale)");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->mime_type=ConstantString("image/x-portable-greymap");
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PNM","PHM","Portable half float format");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->flags|=CoderEndianSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PNM","PNM","Portable anymap");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->magick=(IsImageFormatHandler *) IsPNM;
  entry->mime_type=ConstantString("image/x-portable-pixmap");
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PNM","PPM","Portable pixmap format (color)");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->mime_type=ConstantString("image/x-portable-pixmap");
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P N M I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPNMImage() removes format registrations made by the
%  PNM module from the list of supported formats.
%
%  The format of the UnregisterPNMImage method is:
%
%      UnregisterPNMImage(void)
%
*/
ModuleExport void UnregisterPNMImage(void)
{
  (void) UnregisterMagickInfo("PAM");
  (void) UnregisterMagickInfo("PBM");
  (void) UnregisterMagickInfo("PGM");
  (void) UnregisterMagickInfo("PNM");
  (void) UnregisterMagickInfo("PPM");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P N M I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePNMImage() writes an image to a file in the PNM rasterfile format.
%
%  The format of the WritePNMImage method is:
%
%      MagickBooleanType WritePNMImage(const ImageInfo *image_info,
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
static MagickBooleanType WritePNMImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent],
    format,
    magick[MagickPathExtent];

  const char
    *value;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  Quantum
    index;

  QuantumAny
    pixel;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  size_t
    extent,
    number_scenes,
    packet_size;

  ssize_t
    count,
    y;

  unsigned char
    *q;

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
  scene=0;
  number_scenes=GetImageListLength(image);
  do
  {
    QuantumAny
      max_value;

    /*
      Write PNM file header.
    */
    packet_size=3;
    quantum_type=RGBQuantum;
    (void) CopyMagickString(magick,image_info->magick,MagickPathExtent);
    max_value=GetQuantumRange(image->depth);
    switch (magick[1])
    {
      case 'A':
      case 'a':
      {
        format='7';
        break;
      }
      case 'B':
      case 'b':
      {
        format='4';
        if (image_info->compression == NoCompression)
          format='1';
        break;
      }
      case 'F':
      case 'f':
      {
        format='F';
        if (image_info->type == TrueColorType)
          break;
        if (IdentifyImageCoderGray(image,exception) != MagickFalse)
          format='f';
        break;
      }
      case 'G':
      case 'g':
      {
        format='5';
        if (image_info->compression == NoCompression)
          format='2';
        break;
      }
      case 'H':
      case 'h':
      {
        format='H';
        if (image_info->type == TrueColorType)
          break;
        if (IdentifyImageCoderGray(image,exception) != MagickFalse)
          format='h';
        break;
      }
      case 'N':
      case 'n':
      {
        ImageType
          type;

        format='6';
        if (image_info->type == TrueColorType)
          break;
        type=IdentifyImageCoderGrayType(image,exception);
        if (IsGrayImageType(type) != MagickFalse)
          {
            format='5';
            if (image_info->compression == NoCompression)
              format='2';
            if (type == BilevelType)
              {
                format='4';
                if (image_info->compression == NoCompression)
                  format='1';
              }
          }
        break;
      }
      default:
      {
        format='6';
        if (image_info->compression == NoCompression)
          format='3';
        break;
      }
    }
    (void) FormatLocaleString(buffer,MagickPathExtent,"P%c\n",format);
    (void) WriteBlobString(image,buffer);
    value=GetImageProperty(image,"comment",exception);
    if (value != (const char *) NULL)
      {
        const char
          *p;

        /*
          Write comments to file.
        */
        (void) WriteBlobByte(image,'#');
        for (p=value; *p != '\0'; p++)
        {
          (void) WriteBlobByte(image,(unsigned char) *p);
          if ((*p == '\n') || (*p == '\r'))
            (void) WriteBlobByte(image,'#');
        }
        (void) WriteBlobByte(image,'\n');
      }
    if (format != '7')
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g %.20g\n",
          (double) image->columns,(double) image->rows);
        (void) WriteBlobString(image,buffer);
      }
    else
      {
        char
          type[MagickPathExtent];

        /*
          PAM header.
        */
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "WIDTH %.20g\nHEIGHT %.20g\n",(double) image->columns,(double)
          image->rows);
        (void) WriteBlobString(image,buffer);
        quantum_type=GetQuantumType(image,exception);
        switch (quantum_type)
        {
          case CMYKQuantum:
          case CMYKAQuantum:
          {
            packet_size=4;
            (void) CopyMagickString(type,"CMYK",MagickPathExtent);
            break;
          }
          case GrayQuantum:
          case GrayAlphaQuantum:
          {
            packet_size=1;
            (void) CopyMagickString(type,"GRAYSCALE",MagickPathExtent);
            if (GetQuantumRange(image->depth) == 1)
              (void) CopyMagickString(type,"BLACKANDWHITE",MagickPathExtent);
            break;
          }
          default:
          {
            quantum_type=RGBQuantum;
            if (image->alpha_trait != UndefinedPixelTrait)
              quantum_type=RGBAQuantum;
            packet_size=3;
            (void) CopyMagickString(type,"RGB",MagickPathExtent);
            break;
          }
        }
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            packet_size++;
            (void) ConcatenateMagickString(type,"_ALPHA",MagickPathExtent);
          }
        if (image->depth > 32)
          image->depth=32;
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "DEPTH %.20g\nMAXVAL %.20g\n",(double) packet_size,(double)
          ((MagickOffsetType) GetQuantumRange(image->depth)));
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "TUPLTYPE %s\nENDHDR\n",type);
        (void) WriteBlobString(image,buffer);
      }
    /*
      Convert runextent encoded to PNM raster pixels.
    */
    switch (format)
    {
      case '1':
      {
        unsigned char
          pixels[2048];

        /*
          Convert image to a PBM image.
        */
        (void) SetImageType(image,BilevelType,exception);
        q=pixels;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const Quantum
            *magick_restrict p;

          ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *q++=(unsigned char) (GetPixelLuma(image,p) >= ((double)
              QuantumRange/2.0) ? '0' : '1');
            if ((q-pixels+2) >= (ssize_t) sizeof(pixels))
              {
                *q++='\n';
                (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                q=pixels;
              }
            *q++=' ';
            p+=GetPixelChannels(image);
          }
          *q++='\n';
          (void) WriteBlob(image,(size_t) (q-pixels),pixels);
          q=pixels;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        if (q != pixels)
          {
            *q++='\n';
            (void) WriteBlob(image,(size_t) (q-pixels),pixels);
          }
        break;
      }
      case '2':
      {
        unsigned char
          pixels[2048];

        /*
          Convert image to a PGM image.
        */
        if (image->depth <= 8)
          (void) WriteBlobString(image,"255\n");
        else
          if (image->depth <= 16)
            (void) WriteBlobString(image,"65535\n");
          else
            (void) WriteBlobString(image,"4294967295\n");
        q=pixels;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const Quantum
            *magick_restrict p;

          ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            index=ClampToQuantum(GetPixelLuma(image,p));
            if (image->depth <= 8)
              count=(ssize_t) FormatLocaleString(buffer,MagickPathExtent,"%u ",
                ScaleQuantumToChar(index));
            else
              if (image->depth <= 16)
                count=(ssize_t) FormatLocaleString(buffer,MagickPathExtent,
                  "%u ",ScaleQuantumToShort(index));
              else
                count=(ssize_t) FormatLocaleString(buffer,MagickPathExtent,
                  "%u ",ScaleQuantumToLong(index));
            extent=(size_t) count;
            if ((size_t) (q-pixels+(ssize_t) extent+1) >= sizeof(pixels))
              {
                *q++='\n';
                (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                q=pixels;
              }
            (void) memcpy((char *) q,buffer,extent);
            q+=extent;
            p+=GetPixelChannels(image);
          }
          *q++='\n';
          (void) WriteBlob(image,(size_t) (q-pixels),pixels);
          q=pixels;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        if (q != pixels)
          {
            *q++='\n';
            (void) WriteBlob(image,(size_t) (q-pixels),pixels);
          }
        break;
      }
      case '3':
      {
        unsigned char
          pixels[2048];

        /*
          Convert image to a PNM image.
        */
        if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
          (void) TransformImageColorspace(image,sRGBColorspace,exception);
        if (image->depth <= 8)
          (void) WriteBlobString(image,"255\n");
        else
          if (image->depth <= 16)
            (void) WriteBlobString(image,"65535\n");
          else
            (void) WriteBlobString(image,"4294967295\n");
        q=pixels;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const Quantum
            *magick_restrict p;

          ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (image->depth <= 8)
              count=(ssize_t) FormatLocaleString(buffer,MagickPathExtent,
                "%u %u %u ",ScaleQuantumToChar(GetPixelRed(image,p)),
                ScaleQuantumToChar(GetPixelGreen(image,p)),
                ScaleQuantumToChar(GetPixelBlue(image,p)));
            else
              if (image->depth <= 16)
                count=(ssize_t) FormatLocaleString(buffer,MagickPathExtent,
                  "%u %u %u ",ScaleQuantumToShort(GetPixelRed(image,p)),
                  ScaleQuantumToShort(GetPixelGreen(image,p)),
                  ScaleQuantumToShort(GetPixelBlue(image,p)));
              else
                count=(ssize_t) FormatLocaleString(buffer,MagickPathExtent,
                  "%u %u %u ",ScaleQuantumToLong(GetPixelRed(image,p)),
                  ScaleQuantumToLong(GetPixelGreen(image,p)),
                  ScaleQuantumToLong(GetPixelBlue(image,p)));
            extent=(size_t) count;
            if ((size_t) (q-pixels+(ssize_t) extent+2) >= sizeof(pixels))
              {
                *q++='\n';
                (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                q=pixels;
              }
            (void) memcpy((char *) q,buffer,extent);
            q+=extent;
            p+=GetPixelChannels(image);
          }
          *q++='\n';
          (void) WriteBlob(image,(size_t) (q-pixels),pixels);
          q=pixels;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        if (q != pixels)
          {
            *q++='\n';
            (void) WriteBlob(image,(size_t) (q-pixels),pixels);
          }
        break;
      }
      case '4':
      {
        unsigned char
          *pixels;

        /*
          Convert image to a PBM image.
        */
        (void) SetImageType(image,BilevelType,exception);
        image->depth=1;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        SetQuantumMinIsWhite(quantum_info,MagickTrue);
        (void) SetQuantumEndian(image,quantum_info,MSBEndian);
        pixels=GetQuantumPixels(quantum_info);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const Quantum
            *magick_restrict p;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          extent=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            GrayQuantum,pixels,exception);
          count=WriteBlob(image,extent,pixels);
          if (count != (ssize_t) extent)
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
        break;
      }
      case '5':
      {
        unsigned char
          *pixels;

        /*
          Convert image to a PGM image.
        */
        if (image->depth > 32)
          image->depth=32;
        (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double)
          ((MagickOffsetType) GetQuantumRange(image->depth)));
        (void) WriteBlobString(image,buffer);
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        (void) SetQuantumEndian(image,quantum_info,MSBEndian);
        pixels=GetQuantumPixels(quantum_info);
        extent=GetQuantumExtent(image,quantum_info,GrayQuantum);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const Quantum
            *magick_restrict p;

          ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          q=pixels;
          switch (image->depth)
          {
            case 8:
            case 16:
            case 32:
            {
              extent=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                GrayQuantum,pixels,exception);
              break;
            }
            default:
            {
              if (image->depth <= 8)
                {
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    if (IsPixelGray(image,p) == MagickFalse)
                      pixel=ScaleQuantumToAny(ClampToQuantum(GetPixelLuma(
                        image,p)),max_value);
                    else
                      {
                        if (image->depth == 8)
                          pixel=ScaleQuantumToChar(GetPixelRed(image,p));
                        else
                          pixel=ScaleQuantumToAny(GetPixelRed(image,p),
                            max_value);
                      }
                    q=PopCharPixel((unsigned char) pixel,q);
                    p+=GetPixelChannels(image);
                  }
                  extent=(size_t) (q-pixels);
                  break;
                }
              if (image->depth <= 16)
                {
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    if (IsPixelGray(image,p) == MagickFalse)
                      pixel=ScaleQuantumToAny(ClampToQuantum(GetPixelLuma(image,
                        p)),max_value);
                    else
                      {
                        if (image->depth == 16)
                          pixel=ScaleQuantumToShort(GetPixelRed(image,p));
                        else
                          pixel=ScaleQuantumToAny(GetPixelRed(image,p),
                            max_value);
                      }
                    q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                    p+=GetPixelChannels(image);
                  }
                  extent=(size_t) (q-pixels);
                  break;
                }
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                if (IsPixelGray(image,p) == MagickFalse)
                  pixel=ScaleQuantumToAny(ClampToQuantum(GetPixelLuma(image,p)),
                    max_value);
                else
                  {
                    if (image->depth == 16)
                      pixel=ScaleQuantumToLong(GetPixelRed(image,p));
                    else
                      pixel=ScaleQuantumToAny(GetPixelRed(image,p),max_value);
                  }
                q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                p+=GetPixelChannels(image);
              }
              extent=(size_t) (q-pixels);
              break;
            }
          }
          count=WriteBlob(image,extent,pixels);
          if (count != (ssize_t) extent)
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
        break;
      }
      case '6':
      {
        unsigned char
          *pixels;

        /*
          Convert image to a PNM image.
        */
        if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
          (void) TransformImageColorspace(image,sRGBColorspace,exception);
        if (image->depth > 32)
          image->depth=32;
        (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double)
          ((MagickOffsetType) GetQuantumRange(image->depth)));
        (void) WriteBlobString(image,buffer);
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        (void) SetQuantumEndian(image,quantum_info,MSBEndian);
        pixels=GetQuantumPixels(quantum_info);
        extent=GetQuantumExtent(image,quantum_info,quantum_type);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const Quantum
            *magick_restrict p;

          ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          q=pixels;
          switch (image->depth)
          {
            case 8:
            case 16:
            case 32:
            {
              extent=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                quantum_type,pixels,exception);
              break;
            }
            default:
            {
              if (image->depth <= 8)
                {
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    pixel=ScaleQuantumToAny(GetPixelRed(image,p),max_value);
                    q=PopCharPixel((unsigned char) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelGreen(image,p),max_value);
                    q=PopCharPixel((unsigned char) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelBlue(image,p),max_value);
                    q=PopCharPixel((unsigned char) pixel,q);
                    p+=GetPixelChannels(image);
                  }
                  extent=(size_t) (q-pixels);
                  break;
                }
              if (image->depth <= 16)
                {
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    pixel=ScaleQuantumToAny(GetPixelRed(image,p),max_value);
                    q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelGreen(image,p),max_value);
                    q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelBlue(image,p),max_value);
                    q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                    p+=GetPixelChannels(image);
                  }
                  extent=(size_t) (q-pixels);
                  break;
                }
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                pixel=ScaleQuantumToAny(GetPixelRed(image,p),max_value);
                q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                pixel=ScaleQuantumToAny(GetPixelGreen(image,p),max_value);
                q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                pixel=ScaleQuantumToAny(GetPixelBlue(image,p),max_value);
                q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                p+=GetPixelChannels(image);
              }
              extent=(size_t) (q-pixels);
              break;
            }
          }
          count=WriteBlob(image,extent,pixels);
          if (count != (ssize_t) extent)
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
        break;
      }
      case '7':
      {
        unsigned char
          *pixels;

        /*
          Convert image to a PAM.
        */
        if (image->depth > 32)
          image->depth=32;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        (void) SetQuantumEndian(image,quantum_info,MSBEndian);
        pixels=GetQuantumPixels(quantum_info);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          const Quantum
            *magick_restrict p;

          ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          q=pixels;
          switch (image->depth)
          {
            case 8:
            case 16:
            case 32:
            {
              extent=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                quantum_type,pixels,exception);
              break;
            }
            default:
            {
              switch (quantum_type)
              {
                case GrayQuantum:
                case GrayAlphaQuantum:
                {
                  if (image->depth <= 8)
                    {
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        pixel=ScaleQuantumToAny(ClampToQuantum(GetPixelLuma(
                          image,p)),max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            pixel=(unsigned char) ScaleQuantumToAny(
                              GetPixelAlpha(image,p),max_value);
                            q=PopCharPixel((unsigned char) pixel,q);
                          }
                        p+=GetPixelChannels(image);
                      }
                      break;
                    }
                  if (image->depth <= 16)
                    {
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        pixel=ScaleQuantumToAny(ClampToQuantum(GetPixelLuma(
                          image,p)),max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            pixel=(unsigned char) ScaleQuantumToAny(
                              GetPixelAlpha(image,p),max_value);
                            q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                          }
                        p+=GetPixelChannels(image);
                      }
                      break;
                    }
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    pixel=ScaleQuantumToAny(ClampToQuantum(GetPixelLuma(image,
                      p)),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    if (image->alpha_trait != UndefinedPixelTrait)
                      {
                        pixel=(unsigned char) ScaleQuantumToAny(
                          GetPixelAlpha(image,p),max_value);
                        q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                      }
                    p+=GetPixelChannels(image);
                  }
                  break;
                }
                case CMYKQuantum:
                case CMYKAQuantum:
                {
                  if (image->depth <= 8)
                    {
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        pixel=ScaleQuantumToAny(GetPixelRed(image,p),max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelGreen(image,p),
                          max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelBlue(image,p),
                          max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelBlack(image,p),
                          max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            pixel=ScaleQuantumToAny(GetPixelAlpha(image,p),
                              max_value);
                            q=PopCharPixel((unsigned char) pixel,q);
                          }
                        p+=GetPixelChannels(image);
                      }
                      break;
                    }
                  if (image->depth <= 16)
                    {
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        pixel=ScaleQuantumToAny(GetPixelRed(image,p),max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelGreen(image,p),
                          max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelBlue(image,p),
                          max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelBlack(image,p),
                          max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            pixel=ScaleQuantumToAny(GetPixelAlpha(image,p),
                              max_value);
                            q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                          }
                        p+=GetPixelChannels(image);
                      }
                      break;
                    }
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    pixel=ScaleQuantumToAny(GetPixelRed(image,p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelGreen(image,p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelBlue(image,p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelBlack(image,p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    if (image->alpha_trait != UndefinedPixelTrait)
                      {
                        pixel=ScaleQuantumToAny(GetPixelAlpha(image,p),
                          max_value);
                        q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                      }
                    p+=GetPixelChannels(image);
                  }
                  break;
                }
                default:
                {
                  if (image->depth <= 8)
                    {
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        pixel=ScaleQuantumToAny(GetPixelRed(image,p),max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelGreen(image,p),
                          max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelBlue(image,p),
                          max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            pixel=ScaleQuantumToAny(GetPixelAlpha(image,p),
                              max_value);
                            q=PopCharPixel((unsigned char) pixel,q);
                          }
                        p+=GetPixelChannels(image);
                      }
                      break;
                    }
                  if (image->depth <= 16)
                    {
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        pixel=ScaleQuantumToAny(GetPixelRed(image,p),max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelGreen(image,p),
                          max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelBlue(image,p),
                          max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        if (image->alpha_trait != UndefinedPixelTrait)
                          {
                            pixel=ScaleQuantumToAny(GetPixelAlpha(image,p),
                              max_value);
                            q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                          }
                        p+=GetPixelChannels(image);
                      }
                      break;
                    }
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    pixel=ScaleQuantumToAny(GetPixelRed(image,p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelGreen(image,p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelBlue(image,p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    if (image->alpha_trait != UndefinedPixelTrait)
                      {
                        pixel=ScaleQuantumToAny(GetPixelAlpha(image,p),
                          max_value);
                        q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                      }
                    p+=GetPixelChannels(image);
                  }
                  break;
                }
              }
              extent=(size_t) (q-pixels);
              break;
            }
          }
          count=WriteBlob(image,extent,pixels);
          if (count != (ssize_t) extent)
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
        break;
      }
      case 'F':
      case 'f':
      {
        unsigned char
          *pixels;

        (void) WriteBlobString(image,image->endian == LSBEndian ? "-1.0\n" :
          "1.0\n");
        image->depth=32;
        quantum_type=format == 'f' ? GrayQuantum : RGBQuantum;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
        if (status == MagickFalse)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=GetQuantumPixels(quantum_info);
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          const Quantum
            *magick_restrict p;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          extent=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          (void) WriteBlob(image,extent,pixels);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        break;
      }
      case 'H':
      case 'h':
      {
        unsigned char
          *pixels;

        (void) WriteBlobString(image,image->endian == LSBEndian ? "-1.0\n" :
          "1.0\n");
        image->depth=16;
        quantum_type=format == 'h' ? GrayQuantum : RGBQuantum;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
        if (status == MagickFalse)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=GetQuantumPixels(quantum_info);
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          const Quantum
            *magick_restrict p;

          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
            break;
          extent=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          (void) WriteBlob(image,extent,pixels);
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        break;
      }
      default:
        break;
    }
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,number_scenes);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}

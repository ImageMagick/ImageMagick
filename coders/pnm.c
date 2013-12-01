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
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePNMImage(const ImageInfo *,Image *);

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
       (magick[1] == '7') || (magick[1] == 'F') || (magick[1] == 'f')))
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

static inline ssize_t ConstrainPixel(Image *image,const ssize_t offset,
  const size_t extent)
{
  if ((offset < 0) || (offset > (ssize_t) extent))
    {
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        CorruptImageError,"InvalidPixel","`%s'",image->filename);
      return(0);
    }
  return(offset);
}

static void PNMComment(Image *image)
{
  int
    c;

  char
    *comment;

  register char
    *p;

  size_t
    extent;

  /*
    Read comment.
  */
  comment=AcquireString(GetImageProperty(image,"comment"));
  extent=strlen(comment);
  p=comment+strlen(comment);
  for (c='#'; (c != EOF) && (c != (int) '\n'); p++)
  {
    if ((size_t) (p-comment+1) >= extent)
      {
        extent<<=1;
        comment=(char *) ResizeQuantumMemory(comment,extent+MaxTextExtent,
          sizeof(*comment));
        if (comment == (char *) NULL)
          break;
        p=comment+strlen(comment);
      }
    c=ReadBlobByte(image);
    if (c != EOF)
      {
        *p=(char) c;
        *(p+1)='\0';
      }
  }
  if (comment == (char *) NULL)
    return;
  (void) SetImageProperty(image,"comment",comment);
  comment=DestroyString(comment);
}

static size_t PNMInteger(Image *image,const unsigned int base)
{
  int
    c;

  size_t
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
      PNMComment(image);
  } while (isdigit(c) == MagickFalse);
  if (base == 2)
    return((size_t) (c-(int) '0'));
  /*
    Evaluate number.
  */
  value=0;
  do
  {
    value*=10;
    value+=c-(int) '0';
    c=ReadBlobByte(image);
    if (c == EOF)
      return(value);
  } while (isdigit(c) != MagickFalse);
  return(value);
}

static Image *ReadPNMImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    format;

  double
    quantum_scale;

  Image
    *image;

  MagickBooleanType
    status;

  QuantumAny
    max_value;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  size_t
    depth,
    extent,
    packet_size;

  ssize_t
    count,
    row,
    y;

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
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read PNM image.
  */
  count=ReadBlob(image,1,(unsigned char *) &format);
  do
  {
    /*
      Initialize image structure.
    */
    if ((count != 1) || (format != 'P'))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    max_value=1;
    quantum_type=RGBQuantum;
    quantum_scale=1.0;
    format=(char) ReadBlobByte(image);
    if (format != '7')
      {
        /*
          PBM, PGM, PPM, and PNM.
        */
        image->columns=PNMInteger(image,10);
        image->rows=PNMInteger(image,10);
        if ((format == 'f') || (format == 'F'))
          {
            char
              scale[MaxTextExtent];

            (void) ReadBlobString(image,scale);
            quantum_scale=StringToDouble(scale,(char **) NULL);
          }
        else
          {
            if ((format == '1') || (format == '4'))
              max_value=1;  /* bitmap */
            else
              max_value=PNMInteger(image,10);
          }
      }
    else
      {
        char
          keyword[MaxTextExtent],
          value[MaxTextExtent];

        int
          c;

        register char
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
              PNMComment(image);
              c=ReadBlobByte(image);
              while (isspace((int) ((unsigned char) c)) != 0)
                c=ReadBlobByte(image);
            }
          p=keyword;
          do
          {
            if ((size_t) (p-keyword) < (MaxTextExtent-1))
              *p++=c;
            c=ReadBlobByte(image);
          } while (isalnum(c));
          *p='\0';
          if (LocaleCompare(keyword,"endhdr") == 0)
            break;
          while (isspace((int) ((unsigned char) c)) != 0)
            c=ReadBlobByte(image);
          p=value;
          while (isalnum(c) || (c == '_'))
          {
            if ((size_t) (p-value) < (MaxTextExtent-1))
              *p++=c;
            c=ReadBlobByte(image);
          }
          *p='\0';
          /*
            Assign a value to the specified keyword.
          */
          if (LocaleCompare(keyword,"depth") == 0)
            packet_size=StringToUnsignedLong(value);
          (void) packet_size;
          if (LocaleCompare(keyword,"height") == 0)
            image->rows=StringToUnsignedLong(value);
          if (LocaleCompare(keyword,"maxval") == 0)
            max_value=StringToUnsignedLong(value);
          if (LocaleCompare(keyword,"TUPLTYPE") == 0)
            {
              if (LocaleCompare(value,"BLACKANDWHITE") == 0)
                {
                  (void) SetImageColorspace(image,GRAYColorspace);
                  quantum_type=GrayQuantum;
                }
              if (LocaleCompare(value,"BLACKANDWHITE_ALPHA") == 0)
                {
                  (void) SetImageColorspace(image,GRAYColorspace);
                  image->matte=MagickTrue;
                  quantum_type=GrayAlphaQuantum;
                }
              if (LocaleCompare(value,"GRAYSCALE") == 0)
                {
                  (void) SetImageColorspace(image,GRAYColorspace);
                  quantum_type=GrayQuantum;
                }
              if (LocaleCompare(value,"GRAYSCALE_ALPHA") == 0)
                {
                  (void) SetImageColorspace(image,GRAYColorspace);
                  image->matte=MagickTrue;
                  quantum_type=GrayAlphaQuantum;
                }
              if (LocaleCompare(value,"RGB_ALPHA") == 0)
                {
                  quantum_type=RGBAQuantum;
                  image->matte=MagickTrue;
                }
              if (LocaleCompare(value,"CMYK") == 0)
                {
                  (void) SetImageColorspace(image,CMYKColorspace);
                  quantum_type=CMYKQuantum;
                }
              if (LocaleCompare(value,"CMYK_ALPHA") == 0)
                {
                  (void) SetImageColorspace(image,CMYKColorspace);
                  image->matte=MagickTrue;
                  quantum_type=CMYKAQuantum;
                }
            }
          if (LocaleCompare(keyword,"width") == 0)
            image->columns=StringToUnsignedLong(value);
        }
      }
    if ((image->columns == 0) || (image->rows == 0))
      ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
    if (max_value > 4294967295U)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    for (depth=1; GetQuantumRange(depth) < max_value; depth++) ;
    image->depth=depth;
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    /*
      Convert PNM pixels.
    */
    status=MagickTrue;
    row=0;
    switch (format)
    {
      case '1':
      {
        /*
          Convert PBM image to pixel packets.
        */
        (void) SetImageColorspace(image,GRAYColorspace);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register ssize_t
            x;

          register PixelPacket
            *restrict q;

          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelRed(q,PNMInteger(image,2) == 0 ? QuantumRange : 0);
            SetPixelGreen(q,GetPixelRed(q));
            SetPixelBlue(q,GetPixelRed(q));
            q++;
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
        image->type=BilevelType;
        break;
      }
      case '2':
      {
        size_t
          intensity;

        /*
          Convert PGM image to pixel packets.
        */
        (void) SetImageColorspace(image,GRAYColorspace);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register ssize_t
            x;

          register PixelPacket
            *restrict q;

          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            intensity=ScaleAnyToQuantum(PNMInteger(image,10),max_value);
            SetPixelRed(q,intensity);
            SetPixelGreen(q,GetPixelRed(q));
            SetPixelBlue(q,GetPixelRed(q));
            q++;
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
          register ssize_t
            x;

          register PixelPacket
            *restrict q;

          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            QuantumAny
              pixel;

            pixel=ScaleAnyToQuantum(PNMInteger(image,10),max_value);
            SetPixelRed(q,pixel);
            pixel=ScaleAnyToQuantum(PNMInteger(image,10),max_value);
            SetPixelGreen(q,pixel);
            pixel=ScaleAnyToQuantum(PNMInteger(image,10),max_value);
            SetPixelBlue(q,pixel);
            q++;
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
        break;
      }
      case '4':
      {
        /*
          Convert PBM raw image to pixel packets.
        */
        (void) SetImageColorspace(image,GRAYColorspace);
        quantum_type=GrayQuantum;
        if (image->storage_class == PseudoClass)
          quantum_type=IndexQuantum;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        SetQuantumMinIsWhite(quantum_info,MagickTrue);
        extent=GetQuantumExtent(image,quantum_info,quantum_type);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          MagickBooleanType
            sync;

          register PixelPacket
            *restrict q;

          ssize_t
            count,
            offset;

          size_t
            length;

          unsigned char
            *pixels;

          if (status == MagickFalse)
            continue;
          pixels=GetQuantumPixels(quantum_info);
          {
            count=ReadBlob(image,extent,pixels);
            if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
                (image->previous == (Image *) NULL))
              {
                MagickBooleanType
                  proceed;

                proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                  row,image->rows);
                if (proceed == MagickFalse)
                  status=MagickFalse;
              }
            offset=row++;
          }
          if (count != (ssize_t) extent)
            status=MagickFalse;
          q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          length=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          if (length != extent)
            status=MagickFalse;
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            status=MagickFalse;
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        if (status == MagickFalse)
          ThrowReaderException(CorruptImageError,"UnableToReadImageData");
        SetQuantumImageType(image,quantum_type);
        break;
      }
      case '5':
      {
        /*
          Convert PGM raw image to pixel packets.
        */
        (void) SetImageColorspace(image,GRAYColorspace);
        quantum_type=GrayQuantum;
        if (image->depth <= 8)
          extent=image->columns;
        else
          if (image->depth <= 16)
            extent=2*image->columns;
          else
            extent=4*image->columns;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          MagickBooleanType
            sync;

          register const unsigned char
            *restrict p;

          register PixelPacket
            *restrict q;

          register ssize_t
            x;

          ssize_t
            count,
            offset;

          unsigned char
            *pixels;

          if (status == MagickFalse)
            continue;
          pixels=GetQuantumPixels(quantum_info);
          {
            count=ReadBlob(image,extent,pixels);
            if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
                (image->previous == (Image *) NULL))
              {
                MagickBooleanType
                  proceed;

                proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                  row,image->rows);
                if (proceed == MagickFalse)
                  status=MagickFalse;
              }
            offset=row++;
          }
          if (count != (ssize_t) extent)
            status=MagickFalse;
          q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          p=pixels;
          switch (image->depth)
          {
            case 8:
            case 16:
            case 32:
            {
              (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
                quantum_type,pixels,exception);
              break;
            }
            default:
            {
              unsigned int
                pixel;

              if (image->depth <= 8)
                {
                  unsigned char
                    pixel;

                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    p=PushCharPixel(p,&pixel);
                    SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                    SetPixelGreen(q,GetPixelRed(q));
                    SetPixelBlue(q,GetPixelRed(q));
                    q++;
                  }
                  break;
                }
              if (image->depth <= 16)
                {
                  unsigned short
                    pixel;

                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    p=PushShortPixel(MSBEndian,p,&pixel);
                    SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                    SetPixelGreen(q,GetPixelRed(q));
                    SetPixelBlue(q,GetPixelRed(q));
                    q++;
                  }
                  break;
                }
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                SetPixelGreen(q,GetPixelRed(q));
                SetPixelBlue(q,GetPixelRed(q));
                q++;
              }
              break;
            }
          }
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            status=MagickFalse;
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        if (status == MagickFalse)
          ThrowReaderException(CorruptImageError,"UnableToReadImageData");
        SetQuantumImageType(image,quantum_type);
        break;
      }
      case '6':
      {
        /*
          Convert PNM raster image to pixel packets.
        */
        quantum_type=RGBQuantum;
        if (image->depth <= 8)
          extent=3*image->columns;
        else
          if (image->depth <= 16)
            extent=3*2*image->columns;
          else
            extent=3*4*image->columns;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        (void) SetQuantumEndian(image,quantum_info,MSBEndian);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          MagickBooleanType
            sync;

          register const unsigned char
            *restrict p;

          register PixelPacket
            *restrict q;

          register ssize_t
            x;

          ssize_t
            count,
            offset;

          unsigned char
            *pixels;

          if (status == MagickFalse)
            continue;
          pixels=GetQuantumPixels(quantum_info);
          {
            count=ReadBlob(image,extent,pixels);
            if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
                (image->previous == (Image *) NULL))
              {
                MagickBooleanType
                  proceed;

                proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                  row,image->rows);
                if (proceed == MagickFalse)
                  status=MagickFalse;
              }
            offset=row++;
          }
          if (count != (ssize_t) extent)
            status=MagickFalse;
          q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          p=pixels;
          switch (image->depth)
          {
            case 8:
            {
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelRed(q,ScaleCharToQuantum(*p++));
                SetPixelGreen(q,ScaleCharToQuantum(*p++));
                SetPixelBlue(q,ScaleCharToQuantum(*p++));
                q->opacity=OpaqueOpacity;
                q++;
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
                SetPixelRed(q,ScaleShortToQuantum(pixel));
                p=PushShortPixel(MSBEndian,p,&pixel);
                SetPixelGreen(q,ScaleShortToQuantum(pixel));
                p=PushShortPixel(MSBEndian,p,&pixel);
                SetPixelBlue(q,ScaleShortToQuantum(pixel));
                SetPixelOpacity(q,OpaqueOpacity);
                q++;
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
                SetPixelRed(q,ScaleLongToQuantum(pixel));
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelGreen(q,ScaleLongToQuantum(pixel));
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelBlue(q,ScaleLongToQuantum(pixel));
                SetPixelOpacity(q,OpaqueOpacity);
                q++;
              }
              break;
            }
            default:
            {
              unsigned int
                pixel;

              if (image->depth <= 8)
                {
                  unsigned char
                    pixel;

                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    p=PushCharPixel(p,&pixel);
                    SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                    p=PushCharPixel(p,&pixel);
                    SetPixelGreen(q,ScaleAnyToQuantum(pixel,max_value));
                    p=PushCharPixel(p,&pixel);
                    SetPixelBlue(q,ScaleAnyToQuantum(pixel,max_value));
                    SetPixelOpacity(q,OpaqueOpacity);
                    q++;
                  }
                  break;
                }
              if (image->depth <= 16)
                {
                  unsigned short
                    pixel;

                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    p=PushShortPixel(MSBEndian,p,&pixel);
                    SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                    p=PushShortPixel(MSBEndian,p,&pixel);
                    SetPixelGreen(q,ScaleAnyToQuantum(pixel,max_value));
                    p=PushShortPixel(MSBEndian,p,&pixel);
                    SetPixelBlue(q,ScaleAnyToQuantum(pixel,max_value));
                    SetPixelOpacity(q,OpaqueOpacity);
                    q++;
                  }
                  break;
                }
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelGreen(q,ScaleAnyToQuantum(pixel,max_value));
                p=PushLongPixel(MSBEndian,p,&pixel);
                SetPixelBlue(q,ScaleAnyToQuantum(pixel,max_value));
                SetPixelOpacity(q,OpaqueOpacity);
                q++;
              }
              break;
            }
            break;
          }
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            status=MagickFalse;
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        if (status == MagickFalse)
          ThrowReaderException(CorruptImageError,"UnableToReadImageData");
        break;
      }
      case '7':
      {
        register IndexPacket
          *indexes;

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
        if (image->matte != MagickFalse)
          channels++;
        if (image->depth <= 8)
          extent=channels*image->columns;
        else
          if (image->depth <= 16)
            extent=channels*2*image->columns;
          else
            extent=channels*4*image->columns;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          MagickBooleanType
            sync;

          register const unsigned char
            *restrict p;

          register ssize_t
            x;

          register PixelPacket
            *restrict q;

          ssize_t
            count,
            offset;

          unsigned char
            *pixels;

          if (status == MagickFalse)
            continue;
          pixels=GetQuantumPixels(quantum_info);
          {
            count=ReadBlob(image,extent,pixels);
            if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
                (image->previous == (Image *) NULL))
              {
                MagickBooleanType
                  proceed;

                proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                  row,image->rows);
                if (proceed == MagickFalse)
                  status=MagickFalse;
              }
            offset=row++;
          }
          if (count != (ssize_t) extent)
            status=MagickFalse;
          q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          indexes=GetAuthenticIndexQueue(image);
          p=pixels;
          switch (image->depth)
          {
            case 8:
            case 16:
            case 32:
            {
              (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
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
                  unsigned int
                    pixel;

                  if (image->depth <= 8)
                    {
                      unsigned char
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushCharPixel(p,&pixel);
                        SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                        SetPixelGreen(q,GetPixelRed(q));
                        SetPixelBlue(q,GetPixelRed(q));
                        SetPixelOpacity(q,OpaqueOpacity);
                        if (image->matte != MagickFalse)
                          {
                            p=PushCharPixel(p,&pixel);
                            SetPixelOpacity(q,ScaleAnyToQuantum(pixel,
                               max_value));
                          }
                        q++;
                      }
                      break;
                    }
                  if (image->depth <= 16)
                    {
                      unsigned short
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                        SetPixelGreen(q,GetPixelRed(q));
                        SetPixelBlue(q,GetPixelRed(q));
                        SetPixelOpacity(q,OpaqueOpacity);
                        if (image->matte != MagickFalse)
                          {
                            p=PushShortPixel(MSBEndian,p,&pixel);
                            SetPixelOpacity(q,ScaleAnyToQuantum(pixel,
                              max_value));
                          }
                        q++;
                      }
                      break;
                    }
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    p=PushLongPixel(MSBEndian,p,&pixel);
                    SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                    SetPixelGreen(q,GetPixelRed(q));
                    SetPixelBlue(q,GetPixelRed(q));
                    SetPixelOpacity(q,OpaqueOpacity);
                    if (image->matte != MagickFalse)
                      {
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelOpacity(q,ScaleAnyToQuantum(pixel,max_value));
                      }
                    q++;
                  }
                  break;
                }
                case CMYKQuantum:
                case CMYKAQuantum:
                {
                  unsigned int
                    pixel;

                  if (image->depth <= 8)
                    {
                      unsigned char
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushCharPixel(p,&pixel);
                        SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                        p=PushCharPixel(p,&pixel);
                        SetPixelGreen(q,ScaleAnyToQuantum(pixel,max_value));
                        p=PushCharPixel(p,&pixel);
                        SetPixelBlue(q,ScaleAnyToQuantum(pixel,max_value));
                        p=PushCharPixel(p,&pixel);
                        SetPixelIndex(indexes+x,ScaleAnyToQuantum(pixel,
                          max_value));
                        SetPixelOpacity(q,OpaqueOpacity);
                        if (image->matte != MagickFalse)
                          {
                            p=PushCharPixel(p,&pixel);
                            SetPixelOpacity(q,ScaleAnyToQuantum(pixel,
                              max_value));
                          }
                        q++;
                      }
                      break;
                    }
                  if (image->depth <= 16)
                    {
                      unsigned short
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelGreen(q,ScaleAnyToQuantum(pixel,max_value));
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelBlue(q,ScaleAnyToQuantum(pixel,max_value));
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelIndex(indexes+x,ScaleAnyToQuantum(pixel,
                          max_value));
                        SetPixelOpacity(q,OpaqueOpacity);
                        if (image->matte != MagickFalse)
                          {
                            p=PushShortPixel(MSBEndian,p,&pixel);
                            SetPixelOpacity(q,ScaleAnyToQuantum(pixel,
                              max_value));
                          }
                        q++;
                      }
                      break;
                    }
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    p=PushLongPixel(MSBEndian,p,&pixel);
                    SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                    p=PushLongPixel(MSBEndian,p,&pixel);
                    SetPixelGreen(q,ScaleAnyToQuantum(pixel,max_value));
                    p=PushLongPixel(MSBEndian,p,&pixel);
                    SetPixelBlue(q,ScaleAnyToQuantum(pixel,max_value));
                    p=PushLongPixel(MSBEndian,p,&pixel);
                    SetPixelIndex(indexes+x,ScaleAnyToQuantum(pixel,max_value));
                    SetPixelOpacity(q,OpaqueOpacity);
                    if (image->matte != MagickFalse)
                      {
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelOpacity(q,ScaleAnyToQuantum(pixel,max_value));
                      }
                    q++;
                  }
                  break;
                }
                default:
                {
                  unsigned int
                    pixel;

                  if (image->depth <= 8)
                    {
                      unsigned char
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushCharPixel(p,&pixel);
                        SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                        p=PushCharPixel(p,&pixel);
                        SetPixelGreen(q,ScaleAnyToQuantum(pixel,max_value));
                        p=PushCharPixel(p,&pixel);
                        SetPixelBlue(q,ScaleAnyToQuantum(pixel,max_value));
                        SetPixelOpacity(q,OpaqueOpacity);
                        if (image->matte != MagickFalse)
                          {
                            p=PushCharPixel(p,&pixel);
                            SetPixelOpacity(q,ScaleAnyToQuantum(pixel,
                              max_value));
                          }
                        q++;
                      }
                      break;
                    }
                  if (image->depth <= 16)
                    {
                      unsigned short
                        pixel;

                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelGreen(q,ScaleAnyToQuantum(pixel,max_value));
                        p=PushShortPixel(MSBEndian,p,&pixel);
                        SetPixelBlue(q,ScaleAnyToQuantum(pixel,max_value));
                        SetPixelOpacity(q,OpaqueOpacity);
                        if (image->matte != MagickFalse)
                          {
                            p=PushShortPixel(MSBEndian,p,&pixel);
                            SetPixelOpacity(q,ScaleAnyToQuantum(pixel,
                              max_value));
                          }
                        q++;
                      }
                      break;
                    }
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    p=PushLongPixel(MSBEndian,p,&pixel);
                    SetPixelRed(q,ScaleAnyToQuantum(pixel,max_value));
                    p=PushLongPixel(MSBEndian,p,&pixel);
                    SetPixelGreen(q,ScaleAnyToQuantum(pixel,max_value));
                    p=PushLongPixel(MSBEndian,p,&pixel);
                    SetPixelBlue(q,ScaleAnyToQuantum(pixel,max_value));
                    SetPixelOpacity(q,OpaqueOpacity);
                    if (image->matte != MagickFalse)
                      {
                        p=PushLongPixel(MSBEndian,p,&pixel);
                        SetPixelOpacity(q,ScaleAnyToQuantum(pixel,max_value));
                      }
                    q++;
                  }
                  break;
                }
              }
              break;
            }
          }
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            status=MagickFalse;
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        if (status == MagickFalse)
          ThrowReaderException(CorruptImageError,"UnableToReadImageData");
        SetQuantumImageType(image,quantum_type);
        break;
      }
      case 'F':
      case 'f':
      {
        /*
          Convert PFM raster image to pixel packets.
        */
        if (format == 'f')
          (void) SetImageColorspace(image,GRAYColorspace);
        quantum_type=format == 'f' ? GrayQuantum : RGBQuantum;
        image->endian=quantum_scale < 0.0 ? LSBEndian : MSBEndian;
        image->depth=32;
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        status=SetQuantumDepth(image,quantum_info,32);
        if (status == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
        if (status == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        SetQuantumScale(quantum_info,(MagickRealType) QuantumRange*
          fabs(quantum_scale));
        extent=GetQuantumExtent(image,quantum_info,quantum_type);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          MagickBooleanType
            sync;

          register PixelPacket
            *restrict q;

          ssize_t
            count,
            offset;

          size_t
            length;

          unsigned char
            *pixels;

          if (status == MagickFalse)
            continue;
          pixels=GetQuantumPixels(quantum_info);
          {
            count=ReadBlob(image,extent,pixels);
            if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
                (image->previous == (Image *) NULL))
              {
                MagickBooleanType
                  proceed;

                proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType)
                  row,image->rows);
                if (proceed == MagickFalse)
                  status=MagickFalse;
              }
            offset=row++;
          }
          if ((size_t) count != extent)
            status=MagickFalse;
          q=QueueAuthenticPixels(image,0,(ssize_t) (image->rows-offset-1),
            image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          length=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          if (length != extent)
            status=MagickFalse;
          sync=SyncAuthenticPixels(image,exception);
          if (sync == MagickFalse)
            status=MagickFalse;
        }
        quantum_info=DestroyQuantumInfo(quantum_info);
        if (status == MagickFalse)
          ThrowReaderException(CorruptImageError,"UnableToReadImageData");
        SetQuantumImageType(image,quantum_type);
        break;
      }
      default:
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
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
        if (count == 0)
          break;
        if ((count != 0) && (format == 'P'))
          break;
      } while (format != '\n');
    count=ReadBlob(image,1,(unsigned char *) &format);
    if ((count == 1) && (format == 'P'))
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
  } while ((count == 1) && (format == 'P'));
  (void) CloseBlob(image);
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

  entry=SetMagickInfo("PAM");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->description=ConstantString("Common 2-dimensional bitmap format");
  entry->mime_type=ConstantString("image/x-portable-pixmap");
  entry->module=ConstantString("PNM");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PBM");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->description=ConstantString("Portable bitmap format (black and white)");
  entry->mime_type=ConstantString("image/x-portable-bitmap");
  entry->module=ConstantString("PNM");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PFM");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->endian_support=MagickTrue;
  entry->description=ConstantString("Portable float format");
  entry->module=ConstantString("PFM");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PGM");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->description=ConstantString("Portable graymap format (gray scale)");
  entry->mime_type=ConstantString("image/x-portable-greymap");
  entry->module=ConstantString("PNM");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PNM");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->magick=(IsImageFormatHandler *) IsPNM;
  entry->description=ConstantString("Portable anymap");
  entry->mime_type=ConstantString("image/x-portable-pixmap");
  entry->module=ConstantString("PNM");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PPM");
  entry->decoder=(DecodeImageHandler *) ReadPNMImage;
  entry->encoder=(EncodeImageHandler *) WritePNMImage;
  entry->description=ConstantString("Portable pixmap format (color)");
  entry->mime_type=ConstantString("image/x-portable-pixmap");
  entry->module=ConstantString("PNM");
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
%      MagickBooleanType WritePNMImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WritePNMImage(const ImageInfo *image_info,Image *image)
{
  char
    buffer[MaxTextExtent],
    format,
    magick[MaxTextExtent];

  const char
    *value;

  IndexPacket
    index;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  QuantumAny
    pixel;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register unsigned char
    *pixels,
    *q;

  size_t
    extent,
    packet_size;

  ssize_t
    count,
    y;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  do
  {
    QuantumAny
      max_value;

    /*
      Write PNM file header.
    */
    max_value=GetQuantumRange(image->depth);
    packet_size=3;
    quantum_type=RGBQuantum;
    (void) CopyMagickString(magick,image_info->magick,MaxTextExtent);
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
        if (IsGrayImage(image,&image->exception) != MagickFalse)
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
      case 'N':
      case 'n':
      {
        if ((image_info->type != TrueColorType) &&
            (IsGrayImage(image,&image->exception) != MagickFalse))
          {
            format='5';
            if (image_info->compression == NoCompression)
              format='2';
            if (IsMonochromeImage(image,&image->exception) != MagickFalse)
              {
                format='4';
                if (image_info->compression == NoCompression)
                  format='1';
              }
            break;
          }
      }
      default:
      {
        format='6';
        if (image_info->compression == NoCompression)
          format='3';
        break;
      }
    }
    (void) FormatLocaleString(buffer,MaxTextExtent,"P%c\n",format);
    (void) WriteBlobString(image,buffer);
    value=GetImageProperty(image,"comment");
    if (value != (const char *) NULL)
      {
        register const char
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
        (void) FormatLocaleString(buffer,MaxTextExtent,"%.20g %.20g\n",
          (double) image->columns,(double) image->rows);
        (void) WriteBlobString(image,buffer);
      }
    else
      {
        char
          type[MaxTextExtent];

        /*
          PAM header.
        */
        (void) FormatLocaleString(buffer,MaxTextExtent,
          "WIDTH %.20g\nHEIGHT %.20g\n",(double) image->columns,(double)
          image->rows);
        (void) WriteBlobString(image,buffer);
        quantum_type=GetQuantumType(image,&image->exception);
        switch (quantum_type)
        {
          case CMYKQuantum:
          case CMYKAQuantum:
          {
            packet_size=4;
            (void) CopyMagickString(type,"CMYK",MaxTextExtent);
            break;
          }
          case GrayQuantum:
          case GrayAlphaQuantum:
          {
            packet_size=1;
            (void) CopyMagickString(type,"GRAYSCALE",MaxTextExtent);
            break;
          }
          default:
          {
            quantum_type=RGBQuantum;
            if (image->matte != MagickFalse)
              quantum_type=RGBAQuantum;
            packet_size=3;
            (void) CopyMagickString(type,"RGB",MaxTextExtent);
            break;
          }
        }
        if (image->matte != MagickFalse)
          {
            packet_size++;
            (void) ConcatenateMagickString(type,"_ALPHA",MaxTextExtent);
          }
        if (image->depth > 32)
          image->depth=32;
        (void) FormatLocaleString(buffer,MaxTextExtent,
          "DEPTH %.20g\nMAXVAL %.20g\n",(double) packet_size,(double)
          ((MagickOffsetType) GetQuantumRange(image->depth)));
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MaxTextExtent,"TUPLTYPE %s\nENDHDR\n",
          type);
        (void) WriteBlobString(image,buffer);
      }
    /*
      Convert to PNM raster pixels.
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
        (void) SetImageType(image,BilevelType);
        q=pixels;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register const PixelPacket
            *restrict p;

          register ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            *q++=(unsigned char) (GetPixelLuma(image,p) >= (QuantumRange/2.0) ?
              '0' : '1');
            *q++=' ';
            if ((q-pixels+2) >= 80)
              {
                *q++='\n';
                (void) WriteBlob(image,q-pixels,pixels);
                q=pixels;
              }
            p++;
          }
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
            (void) WriteBlob(image,q-pixels,pixels);
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
          register const PixelPacket
            *restrict p;

          register ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            index=ClampToQuantum(GetPixelLuma(image,p));
            if (image->depth <= 8)
              count=(ssize_t) FormatLocaleString(buffer,MaxTextExtent,"%u ",
                ScaleQuantumToChar(index));
            else
              if (image->depth <= 16)
                count=(ssize_t) FormatLocaleString(buffer,MaxTextExtent,"%u ",
                  ScaleQuantumToShort(index));
              else
                count=(ssize_t) FormatLocaleString(buffer,MaxTextExtent,"%u ",
                  ScaleQuantumToLong(index));
            extent=(size_t) count;
            (void) strncpy((char *) q,buffer,extent);
            q+=extent;
            if ((q-pixels+extent) >= 80)
              {
                *q++='\n';
                (void) WriteBlob(image,q-pixels,pixels);
                q=pixels;
              }
            p++;
          }
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
            (void) WriteBlob(image,q-pixels,pixels);
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
          (void) TransformImageColorspace(image,sRGBColorspace);
        if (image->depth <= 8)
          (void) WriteBlobString(image,"255\n");
        else
          if (image->depth <= 8)
            (void) WriteBlobString(image,"65535\n");
          else
            (void) WriteBlobString(image,"4294967295\n");
        q=pixels;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register const PixelPacket
            *restrict p;

          register ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (image->depth <= 8)
              count=(ssize_t) FormatLocaleString(buffer,MaxTextExtent,
                "%u %u %u ",ScaleQuantumToChar(GetPixelRed(p)),
                ScaleQuantumToChar(GetPixelGreen(p)),
                ScaleQuantumToChar(GetPixelBlue(p)));
            else
              if (image->depth <= 16)
                count=(ssize_t) FormatLocaleString(buffer,MaxTextExtent,
                  "%u %u %u ",ScaleQuantumToShort(GetPixelRed(p)),
                  ScaleQuantumToShort(GetPixelGreen(p)),
                  ScaleQuantumToShort(GetPixelBlue(p)));
              else
                count=(ssize_t) FormatLocaleString(buffer,MaxTextExtent,
                  "%u %u %u ",ScaleQuantumToLong(GetPixelRed(p)),
                  ScaleQuantumToLong(GetPixelGreen(p)),
                  ScaleQuantumToLong(GetPixelBlue(p)));
            extent=(size_t) count;
            (void) strncpy((char *) q,buffer,extent);
            q+=extent;
            if ((q-pixels+extent) >= 80)
              {
                *q++='\n';
                (void) WriteBlob(image,q-pixels,pixels);
                q=pixels;
              }
            p++;
          }
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
            (void) WriteBlob(image,q-pixels,pixels);
          }
        break;
      }
      case '4':
      {
        /*
          Convert image to a PBM image.
        */
        (void) SetImageType(image,BilevelType);
        image->depth=1;
        quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        quantum_info->min_is_white=MagickTrue;
        pixels=GetQuantumPixels(quantum_info);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register const PixelPacket
            *restrict p;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          extent=ExportQuantumPixels(image,(const CacheView *) NULL,
            quantum_info,GrayQuantum,pixels,&image->exception);
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
        /*
          Convert image to a PGM image.
        */
        if (image->depth > 32)
          image->depth=32;
        (void) FormatLocaleString(buffer,MaxTextExtent,"%.20g\n",(double)
          ((MagickOffsetType) GetQuantumRange(image->depth)));
        (void) WriteBlobString(image,buffer);
        quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        quantum_info->min_is_white=MagickTrue;
        pixels=GetQuantumPixels(quantum_info);
        extent=GetQuantumExtent(image,quantum_info,GrayQuantum);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register const PixelPacket
            *restrict p;

          register ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          q=pixels;
          switch (image->depth)
          {
            case 8:
            case 16:
            case 32:
            {
              extent=ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,GrayQuantum,pixels,&image->exception);
              break;
            }
            default:
            {
              if (image->depth <= 8)
                {
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    if (IsGrayPixel(p) == MagickFalse)
                      pixel=ScaleQuantumToAny(ClampToQuantum(
                        GetPixelLuma(image,p)),max_value);
                    else
                      {
                        if (image->depth == 8)
                          pixel=ScaleQuantumToChar(GetPixelRed(p));
                        else
                          pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                      }
                    q=PopCharPixel((unsigned char) pixel,q);
                    p++;
                  }
                  extent=(size_t) (q-pixels);
                  break;
                }
              if (image->depth <= 16)
                {
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    if (IsGrayPixel(p) == MagickFalse)
                      pixel=ScaleQuantumToAny(ClampToQuantum(
                        GetPixelLuma(image,p)),max_value);
                    else
                      {
                        if (image->depth == 16)
                          pixel=ScaleQuantumToShort(GetPixelRed(p));
                        else
                          pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                      }
                    q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                    p++;
                  }
                  extent=(size_t) (q-pixels);
                  break;
                }
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                if (IsGrayPixel(p) == MagickFalse)
                  pixel=ScaleQuantumToAny(ClampToQuantum(
                    GetPixelLuma(image,p)),max_value);
                else
                  {
                    if (image->depth == 32)
                      pixel=ScaleQuantumToLong(GetPixelRed(p));
                    else
                      pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                  }
                q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                p++;
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
        /*
          Convert image to a PNM image.
        */
        if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
          (void) TransformImageColorspace(image,sRGBColorspace);
        if (image->depth > 32)
          image->depth=32;
        (void) FormatLocaleString(buffer,MaxTextExtent,"%.20g\n",(double)
          ((MagickOffsetType) GetQuantumRange(image->depth)));
        (void) WriteBlobString(image,buffer);
        quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        (void) SetQuantumEndian(image,quantum_info,MSBEndian);
        pixels=GetQuantumPixels(quantum_info);
        extent=GetQuantumExtent(image,quantum_info,quantum_type);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register const PixelPacket
            *restrict p;

          register ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          q=pixels;
          switch (image->depth)
          {
            case 8:
            case 16:
            case 32:
            {
              extent=ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,quantum_type,pixels,&image->exception);
              break;
            }
            default:
            {
              if (image->depth <= 8)
                {
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                    q=PopCharPixel((unsigned char) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelGreen(p),max_value);
                    q=PopCharPixel((unsigned char) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelBlue(p),max_value);
                    q=PopCharPixel((unsigned char) pixel,q);
                    p++;
                  }
                  extent=(size_t) (q-pixels);
                  break;
                }
              if (image->depth <= 16)
                {
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                    q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelGreen(p),max_value);
                    q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelBlue(p),max_value);
                    q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                    p++;
                  }
                  extent=(size_t) (q-pixels);
                  break;
                }
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                q=PopLongPixel(MSBEndian,(unsigned short) pixel,q);
                pixel=ScaleQuantumToAny(GetPixelGreen(p),max_value);
                q=PopLongPixel(MSBEndian,(unsigned short) pixel,q);
                pixel=ScaleQuantumToAny(GetPixelBlue(p),max_value);
                q=PopLongPixel(MSBEndian,(unsigned short) pixel,q);
                p++;
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
        /*
          Convert image to a PAM.
        */
        if (image->depth > 32)
          image->depth=32;
        quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
        pixels=GetQuantumPixels(quantum_info);
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register const IndexPacket
            *restrict indexes;

          register const PixelPacket
            *restrict p;

          register ssize_t
            x;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          indexes=GetVirtualIndexQueue(image);
          q=pixels;
          switch (image->depth)
          {
            case 8:
            case 16:
            case 32:
            {
              extent=ExportQuantumPixels(image,(const CacheView *) NULL,
                quantum_info,quantum_type,pixels,&image->exception);
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
                        pixel=ScaleQuantumToAny(ClampToQuantum(
                          GetPixelLuma(image,p)),max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        if (image->matte != MagickFalse)
                          {
                            pixel=(unsigned char) ScaleQuantumToAny(
                              GetPixelOpacity(p),max_value);
                            q=PopCharPixel((unsigned char) pixel,q);
                          }
                        p++;
                      }
                      break;
                    }
                  if (image->depth <= 16)
                    {
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        pixel=ScaleQuantumToAny(ClampToQuantum(
                          GetPixelLuma(image,p)),max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        if (image->matte != MagickFalse)
                          {
                            pixel=(unsigned char) ScaleQuantumToAny(
                              GetPixelOpacity(p),max_value);
                            q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                          }
                        p++;
                      }
                      break;
                    }
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    pixel=ScaleQuantumToAny(ClampToQuantum(
                      GetPixelLuma(image,p)),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    if (image->matte != MagickFalse)
                      {
                        pixel=(unsigned char) ScaleQuantumToAny(
                          GetPixelOpacity(p),max_value);
                        q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                      }
                    p++;
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
                        pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelGreen(p),max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelBlue(p),max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelIndex(indexes+x),
                          max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        if (image->matte != MagickFalse)
                          {
                            pixel=ScaleQuantumToAny((Quantum) (QuantumRange-
                              GetPixelOpacity(p)),max_value);
                            q=PopCharPixel((unsigned char) pixel,q);
                          }
                        p++;
                      }
                      break;
                    }
                  if (image->depth <= 16)
                    {
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelGreen(p),max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelBlue(p),max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelIndex(indexes+x),
                          max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        if (image->matte != MagickFalse)
                          {
                            pixel=ScaleQuantumToAny((Quantum) (QuantumRange-
                              GetPixelOpacity(p)),max_value);
                            q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                          }
                        p++;
                      }
                      break;
                    }
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelGreen(p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelBlue(p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelIndex(indexes+x),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    if (image->matte != MagickFalse)
                      {
                        pixel=ScaleQuantumToAny((Quantum) (QuantumRange-
                          GetPixelOpacity(p)),max_value);
                        q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                      }
                    p++;
                  }
                  break;
                }
                default:
                {
                  if (image->depth <= 8)
                    {
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelGreen(p),max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelBlue(p),max_value);
                        q=PopCharPixel((unsigned char) pixel,q);
                        if (image->matte != MagickFalse)
                          {
                            pixel=ScaleQuantumToAny((Quantum) (QuantumRange-
                              GetPixelOpacity(p)),max_value);
                            q=PopCharPixel((unsigned char) pixel,q);
                          }
                        p++;
                      }
                      break;
                    }
                  if (image->depth <= 16)
                    {
                      for (x=0; x < (ssize_t) image->columns; x++)
                      {
                        pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelGreen(p),max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        pixel=ScaleQuantumToAny(GetPixelBlue(p),max_value);
                        q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                        if (image->matte != MagickFalse)
                          {
                            pixel=ScaleQuantumToAny((Quantum) (QuantumRange-
                              GetPixelOpacity(p)),max_value);
                            q=PopShortPixel(MSBEndian,(unsigned short) pixel,q);
                          }
                        p++;
                      }
                      break;
                    }
                  for (x=0; x < (ssize_t) image->columns; x++)
                  {
                    pixel=ScaleQuantumToAny(GetPixelRed(p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelGreen(p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    pixel=ScaleQuantumToAny(GetPixelBlue(p),max_value);
                    q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                    if (image->matte != MagickFalse)
                      {
                        pixel=ScaleQuantumToAny((Quantum) (QuantumRange-
                          GetPixelOpacity(p)),max_value);
                        q=PopLongPixel(MSBEndian,(unsigned int) pixel,q);
                      }
                    p++;
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
        (void) WriteBlobString(image,image->endian == LSBEndian ? "-1.0\n" :
          "1.0\n");
        image->depth=32;
        quantum_type=format == 'f' ? GrayQuantum : RGBQuantum;
        quantum_info=AcquireQuantumInfo((const ImageInfo *) NULL,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        status=SetQuantumFormat(image,quantum_info,FloatingPointQuantumFormat);
        if (status == MagickFalse)
          ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=GetQuantumPixels(quantum_info);
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          register const PixelPacket
            *restrict p;

          p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          extent=ExportQuantumPixels(image,(const CacheView *) NULL,
            quantum_info,quantum_type,pixels,&image->exception);
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            TTTTT  X   X  TTTTT                              %
%                              T     X X     T                                %
%                              T      X      T                                %
%                              T     X X     T                                %
%                              T    X   X    T                                %
%                                                                             %
%                                                                             %
%                      Render Text Onto A Canvas Image.                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/annotate.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteTXTImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s T X T                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsTXT() returns MagickTrue if the image format type, identified by the magick
%  string, is TXT.
%
%  The format of the IsTXT method is:
%
%      MagickBooleanType IsTXT(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsTXT(const unsigned char *magick,const size_t length)
{
#define MagickID  "# ImageMagick pixel enumeration:"

  char
    colorspace[MagickPathExtent];

  ssize_t
    count;

  unsigned long
    columns,
    depth,
    rows;

  if (length < 40)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,MagickID,strlen(MagickID)) != 0)
    return(MagickFalse);
  count=(ssize_t) sscanf((const char *) magick+32,"%lu,%lu,%lu,%32s",&columns,
    &rows,&depth,colorspace);
  if (count != 4)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d T E X T I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadTEXTImage() reads a text file and returns it as an image.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadTEXTImage method is:
%
%      Image *ReadTEXTImage(const ImageInfo *image_info,Image *image,
%        char *text,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o text: the text storage buffer.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadTEXTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent],
    geometry[MagickPathExtent],
    *p,
    text[MagickPathExtent];

  DrawInfo
    *draw_info;

  Image
    *image,
    *texture;

  MagickBooleanType
    status;

  PointInfo
    delta;

  RectangleInfo
    page;

  ssize_t
    offset;

  TypeMetric
    metrics;

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
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) memset(text,0,sizeof(text));
  (void) ReadBlobString(image,text);
  /*
    Set the page geometry.
  */
  delta.x=DefaultResolution;
  delta.y=DefaultResolution;
  if ((image->resolution.x == 0.0) || (image->resolution.y == 0.0))
    {
      GeometryInfo
        geometry_info;

      MagickStatusType
        flags;

      flags=ParseGeometry(PSDensityGeometry,&geometry_info);
      image->resolution.x=geometry_info.rho;
      image->resolution.y=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->resolution.y=image->resolution.x;
    }
  page.width=612;
  page.height=792;
  page.x=43;
  page.y=43;
  if (image_info->page != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->page,&page);
  /*
    Initialize Image structure.
  */
  image->columns=(size_t) floor((((double) page.width*image->resolution.x)/
    delta.x)+0.5);
  image->rows=(size_t) floor((((double) page.height*image->resolution.y)/
    delta.y)+0.5);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status != MagickFalse)
    status=ResetImagePixels(image,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  image->page.x=0;
  image->page.y=0;
  texture=(Image *) NULL;
  if (image_info->texture != (char *) NULL)
    {
      ImageInfo
        *read_info;

      read_info=CloneImageInfo(image_info);
      SetImageInfoBlob(read_info,(void *) NULL,0);
      (void) CopyMagickString(read_info->filename,image_info->texture,
        MagickPathExtent);
      texture=ReadImage(read_info,exception);
      read_info=DestroyImageInfo(read_info);
    }
  /*
    Annotate the text image.
  */
  (void) SetImageBackgroundColor(image,exception);
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  (void) CloneString(&draw_info->text,image_info->filename);
  (void) FormatLocaleString(geometry,MagickPathExtent,"%gx%g%+g%+g",(double)
    image->columns,(double) image->rows,(double) page.x,(double) page.y);
  (void) CloneString(&draw_info->geometry,geometry);
  status=GetTypeMetrics(image,draw_info,&metrics,exception);
  if (status == MagickFalse)
    {
      draw_info=DestroyDrawInfo(draw_info);
      ThrowReaderException(TypeError,"UnableToGetTypeMetrics");
    }
  page.y=(ssize_t) ceil((double) page.y+metrics.ascent-0.5);
  (void) FormatLocaleString(geometry,MagickPathExtent,"%gx%g%+g%+g",(double)
    image->columns,(double) image->rows,(double) page.x,(double) page.y);
  (void) CloneString(&draw_info->geometry,geometry);
  (void) CopyMagickString(filename,image_info->filename,MagickPathExtent);
  if (*draw_info->text != '\0')
    *draw_info->text='\0';
  p=text;
  for (offset=2*page.y; p != (char *) NULL; )
  {
    /*
      Annotate image with text.
    */
    (void) ConcatenateString(&draw_info->text,text);
    (void) ConcatenateString(&draw_info->text,"\n");
    offset+=(ssize_t) (metrics.ascent-metrics.descent);
    if (image->previous == (Image *) NULL)
      {
        status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) offset,
          image->rows);
        if (status == MagickFalse)
          break;
      }
    p=ReadBlobString(image,text);
    if ((offset < (ssize_t) image->rows) && (p != (char *) NULL))
      continue;
    if (texture != (Image *) NULL)
      {
        MagickProgressMonitor
          progress_monitor;

        progress_monitor=SetImageProgressMonitor(image,
          (MagickProgressMonitor) NULL,image->client_data);
        (void) TextureImage(image,texture,exception);
        (void) SetImageProgressMonitor(image,progress_monitor,
          image->client_data);
      }
    (void) AnnotateImage(image,draw_info,exception);
    if (p == (char *) NULL)
      break;
    /*
      Page is full-- allocate next image structure.
    */
    *draw_info->text='\0';
    offset=2*page.y;
    AcquireNextImage(image_info,image,exception);
    if (GetNextImageInList(image) == (Image *) NULL)
      {
        image=DestroyImageList(image);
        return((Image *) NULL);
      }
    image->next->columns=image->columns;
    image->next->rows=image->rows;
    image=SyncNextImageInList(image);
    (void) CopyMagickString(image->filename,filename,MagickPathExtent);
    (void) SetImageBackgroundColor(image,exception);
    status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
      GetBlobSize(image));
    if (status == MagickFalse)
      break;
  }
  if (texture != (Image *) NULL)
    {
      MagickProgressMonitor
        progress_monitor;

      progress_monitor=SetImageProgressMonitor(image,
        (MagickProgressMonitor) NULL,image->client_data);
      (void) TextureImage(image,texture,exception);
      (void) SetImageProgressMonitor(image,progress_monitor,image->client_data);
    }
  (void) AnnotateImage(image,draw_info,exception);
  if (texture != (Image *) NULL)
    texture=DestroyImage(texture);
  draw_info=DestroyDrawInfo(draw_info);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d T X T I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadTXTImage() reads a text file and returns it as an image.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadTXTImage method is:
%
%      Image *ReadTXTImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadTXTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    colorspace[MagickPathExtent],
    text[MagickPathExtent];

  double
    x_offset,
    y_offset;

  Image
    *image;

  PixelInfo
    pixel;

  MagickBooleanType
    status;

  QuantumAny
    range;

  register ssize_t
    i,
    x;

  register Quantum
    *q;

  ssize_t
    count,
    type,
    y;

  unsigned long
    depth,
    height,
    max_value,
    width;

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
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) memset(text,0,sizeof(text));
  (void) ReadBlobString(image,text);
  if (LocaleNCompare((char *) text,MagickID,strlen(MagickID)) != 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  x_offset=(-1.0);
  y_offset=(-1.0);
  do
  {
    width=0;
    height=0;
    max_value=0;
    *colorspace='\0';
    count=(ssize_t) sscanf(text+32,"%lu,%lu,%lu,%32s",&width,&height,&max_value,
      colorspace);
    if ((count != 4) || (width == 0) || (height == 0) || (max_value == 0))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    image->columns=width;
    image->rows=height;
    if ((max_value == 0) || (max_value > 4294967295UL))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    for (depth=1; (GetQuantumRange(depth)+1) < max_value; depth++) ;
    image->depth=depth;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status != MagickFalse)
      status=ResetImagePixels(image,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    LocaleLower(colorspace);
    i=(ssize_t) strlen(colorspace)-1;
    image->alpha_trait=UndefinedPixelTrait;
    if ((i > 0) && (colorspace[i] == 'a'))
      {
        colorspace[i]='\0';
        image->alpha_trait=BlendPixelTrait;
      }
    type=ParseCommandOption(MagickColorspaceOptions,MagickFalse,colorspace);
    if (type < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    (void) SetImageColorspace(image,(ColorspaceType) type,exception);
    (void) SetImageBackgroundColor(image,exception);
    GetPixelInfo(image,&pixel);
    range=GetQuantumRange(image->depth);
    status=MagickTrue;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      double
        alpha,
        black,
        blue,
        green,
        red;

      if (status == MagickFalse)
        break;
      red=0.0;
      green=0.0;
      blue=0.0;
      black=0.0;
      alpha=0.0;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        if (ReadBlobString(image,text) == (char *) NULL)
          {
            status=MagickFalse;
            break;
          }
        switch (image->colorspace)
        {
          case LinearGRAYColorspace:
          case GRAYColorspace:
          {
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                count=(ssize_t) sscanf(text,"%lf,%lf: (%lf%*[%,]%lf%*[%,]",
                  &x_offset,&y_offset,&red,&alpha);
                green=red;
                blue=red;
                break;
              }
            count=(ssize_t) sscanf(text,"%lf,%lf: (%lf%*[%,]",&x_offset,
              &y_offset,&red);
            green=red;
            blue=red;
            break;
          }
          case CMYKColorspace:
          {
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                count=(ssize_t) sscanf(text,
                  "%lf,%lf: (%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]",
                  &x_offset,&y_offset,&red,&green,&blue,&black,&alpha);
                break;
              }
            count=(ssize_t) sscanf(text,
              "%lf,%lf: (%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]",&x_offset,
              &y_offset,&red,&green,&blue,&black);
            break;
          }
          default:
          {
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                count=(ssize_t) sscanf(text,
                  "%lf,%lf: (%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]",
                  &x_offset,&y_offset,&red,&green,&blue,&alpha);
                break;
              }
            count=(ssize_t) sscanf(text,"%lf,%lf: (%lf%*[%,]%lf%*[%,]%lf%*[%,]",
              &x_offset,&y_offset,&red,&green,&blue);
            break;
          }
        }
        if (strchr(text,'%') != (char *) NULL)
          {
            red*=0.01*range;
            green*=0.01*range;
            blue*=0.01*range;
            black*=0.01*range;
            alpha*=0.01*range;
          }
        if (image->colorspace == LabColorspace)
          {
            green+=(range+1)/2.0;
            blue+=(range+1)/2.0;
          }
        pixel.red=(MagickRealType) ScaleAnyToQuantum((QuantumAny) (red+0.5),
          range);
        pixel.green=(MagickRealType) ScaleAnyToQuantum((QuantumAny) (green+0.5),
          range);
        pixel.blue=(MagickRealType) ScaleAnyToQuantum((QuantumAny) (blue+0.5),
          range);
        pixel.black=(MagickRealType) ScaleAnyToQuantum((QuantumAny) (black+0.5),
          range);
        pixel.alpha=(MagickRealType) ScaleAnyToQuantum((QuantumAny) (alpha+0.5),
          range);
        q=GetAuthenticPixels(image,(ssize_t) x_offset,(ssize_t) y_offset,1,1,
          exception);
        if (q == (Quantum *) NULL)
          continue;
        SetPixelViaPixelInfo(image,&pixel,q);
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          {
            status=MagickFalse;
            break;
          }
      }
    }
    if (status == MagickFalse)
      break;
    *text='\0';
    (void) ReadBlobString(image,text);
    if (LocaleNCompare((char *) text,MagickID,strlen(MagickID)) == 0)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
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
  } while (LocaleNCompare((char *) text,MagickID,strlen(MagickID)) == 0);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r T X T I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterTXTImage() adds attributes for the TXT image format to the
%  list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterTXTImage method is:
%
%      size_t RegisterTXTImage(void)
%
*/
ModuleExport size_t RegisterTXTImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("TXT","SPARSE-COLOR","Sparse Color");
  entry->encoder=(EncodeImageHandler *) WriteTXTImage;
  entry->flags|=CoderRawSupportFlag;
  entry->flags|=CoderEndianSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TXT","TEXT","Text");
  entry->decoder=(DecodeImageHandler *) ReadTEXTImage;
  entry->format_type=ImplicitFormatType;
  entry->flags|=CoderRawSupportFlag;
  entry->flags|=CoderEndianSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TXT","TXT","Text");
  entry->decoder=(DecodeImageHandler *) ReadTXTImage;
  entry->encoder=(EncodeImageHandler *) WriteTXTImage;
  entry->magick=(IsImageFormatHandler *) IsTXT;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r T X T I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterTXTImage() removes format registrations made by the
%  TXT module from the list of supported format.
%
%  The format of the UnregisterTXTImage method is:
%
%      UnregisterTXTImage(void)
%
*/
ModuleExport void UnregisterTXTImage(void)
{
  (void) UnregisterMagickInfo("SPARSE-COLOR");
  (void) UnregisterMagickInfo("TEXT");
  (void) UnregisterMagickInfo("TXT");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e T X T I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteTXTImage writes the pixel values as text numbers.
%
%  The format of the WriteTXTImage method is:
%
%      MagickBooleanType WriteTXTImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteTXTImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent],
    colorspace[MagickPathExtent],
    tuple[MagickPathExtent];

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  PixelInfo
    pixel;

  register const Quantum
    *p;

  register ssize_t
    x;

  size_t
    imageListLength;

  ssize_t
    y;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  imageListLength=GetImageListLength(image);
  do
  {
    ComplianceType
      compliance;

    const char
      *value;

    (void) CopyMagickString(colorspace,CommandOptionToMnemonic(
      MagickColorspaceOptions,(ssize_t) image->colorspace),MagickPathExtent);
    LocaleLower(colorspace);
    image->depth=GetImageQuantumDepth(image,MagickTrue);
    if (image->alpha_trait != UndefinedPixelTrait)
      (void) ConcatenateMagickString(colorspace,"a",MagickPathExtent);
    compliance=NoCompliance;
    value=GetImageOption(image_info,"txt:compliance");
    if (value != (char *) NULL)
      compliance=(ComplianceType) ParseCommandOption(MagickComplianceOptions,
        MagickFalse,value);
    if (LocaleCompare(image_info->magick,"SPARSE-COLOR") != 0)
      {
        size_t
          depth;

        depth=compliance == SVGCompliance ? image->depth :
          MAGICKCORE_QUANTUM_DEPTH;
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "# ImageMagick pixel enumeration: %.20g,%.20g,%.20g,%s\n",(double)
          image->columns,(double) image->rows,(double) ((MagickOffsetType)
          GetQuantumRange(depth)),colorspace);
        (void) WriteBlobString(image,buffer);
      }
    GetPixelInfo(image,&pixel);
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        GetPixelInfoPixel(image,p,&pixel);
        if (pixel.colorspace == LabColorspace)
          {
            pixel.green-=(QuantumRange+1)/2.0;
            pixel.blue-=(QuantumRange+1)/2.0;
          }
        if (LocaleCompare(image_info->magick,"SPARSE-COLOR") == 0)
          {
            /*
              Sparse-color format.
            */
            if (GetPixelAlpha(image,p) == (Quantum) OpaqueAlpha)
              {
                GetColorTuple(&pixel,MagickFalse,tuple);
                (void) FormatLocaleString(buffer,MagickPathExtent,
                  "%.20g,%.20g,",(double) x,(double) y);
                (void) WriteBlobString(image,buffer);
                (void) WriteBlobString(image,tuple);
                (void) WriteBlobString(image," ");
              }
            p+=GetPixelChannels(image);
            continue;
          }
        (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g,%.20g: ",
          (double) x,(double) y);
        (void) WriteBlobString(image,buffer);
        (void) CopyMagickString(tuple,"(",MagickPathExtent);
        if ((pixel.colorspace == LinearGRAYColorspace) ||
            (pixel.colorspace == GRAYColorspace))
          ConcatenateColorComponent(&pixel,GrayPixelChannel,compliance,tuple);
        else
          {
            ConcatenateColorComponent(&pixel,RedPixelChannel,compliance,tuple);
            (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
            ConcatenateColorComponent(&pixel,GreenPixelChannel,compliance,
              tuple);
            (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
            ConcatenateColorComponent(&pixel,BluePixelChannel,compliance,
              tuple);
          }
        if (pixel.colorspace == CMYKColorspace)
          {
            (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
            ConcatenateColorComponent(&pixel,BlackPixelChannel,compliance,
              tuple);
          }
        if (pixel.alpha_trait != UndefinedPixelTrait)
          {
            (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
            ConcatenateColorComponent(&pixel,AlphaPixelChannel,compliance,
              tuple);
          }
        (void) ConcatenateMagickString(tuple,")",MagickPathExtent);
        (void) WriteBlobString(image,tuple);
        (void) WriteBlobString(image,"  ");
        GetColorTuple(&pixel,MagickTrue,tuple);
        (void) FormatLocaleString(buffer,MagickPathExtent,"%s",tuple);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,"  ");
        (void) QueryColorname(image,&pixel,SVGCompliance,tuple,exception);
        (void) WriteBlobString(image,tuple);
        (void) WriteBlobString(image,"\n");
        p+=GetPixelChannels(image);
      }
      status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
        image->rows);
      if (status == MagickFalse)
        break;
    }
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

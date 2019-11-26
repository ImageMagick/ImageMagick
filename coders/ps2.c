/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   SSSSS  22222                              %
%                            P   P  SS        22                              %
%                            PPPP    SSS    222                               %
%                            P         SS  22                                 %
%                            P      SSSSS  22222                              %
%                                                                             %
%                                                                             %
%                      Write Postscript Level II Format                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/compress.h"
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
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/timer-private.h"
#include "MagickCore/utility.h"

/*
  Define declarations.
*/
#if defined(MAGICKCORE_TIFF_DELEGATE)
#define CCITTParam  "-1"
#else
#define CCITTParam  "0"
#endif

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePS2Image(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P S 2 I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPS2Image() adds properties for the PS2 image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPS2Image method is:
%
%      size_t RegisterPS2Image(void)
%
*/
ModuleExport size_t RegisterPS2Image(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("PS2","EPS2","Level II Encapsulated PostScript");
  entry->encoder=(EncodeImageHandler *) WritePS2Image;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/postscript");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PS2","PS2","Level II PostScript");
  entry->encoder=(EncodeImageHandler *) WritePS2Image;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/postscript");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P S 2 I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPS2Image() removes format registrations made by the
%  PS2 module from the list of supported formats.
%
%  The format of the UnregisterPS2Image method is:
%
%      UnregisterPS2Image(void)
%
*/
ModuleExport void UnregisterPS2Image(void)
{
  (void) UnregisterMagickInfo("EPS2");
  (void) UnregisterMagickInfo("PS2");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P S 2 I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePS2Image translates an image to encapsulated Postscript
%  Level II for printing.  If the supplied geometry is null, the image is
%  centered on the Postscript page.  Otherwise, the image is positioned as
%  specified by the geometry.
%
%  The format of the WritePS2Image method is:
%
%      MagickBooleanType WritePS2Image(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType Huffman2DEncodeImage(const ImageInfo *image_info,
  Image *image,Image *inject_image,ExceptionInfo *exception)
{
  Image
    *group4_image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  size_t
    length;

  unsigned char
    *group4;

  group4_image=CloneImage(inject_image,0,0,MagickTrue,exception);
  if (group4_image == (Image *) NULL)
    return(MagickFalse);
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->filename,"GROUP4:",MagickPathExtent);
  (void) CopyMagickString(write_info->magick,"GROUP4",MagickPathExtent);
  group4=(unsigned char *) ImageToBlob(write_info,group4_image,&length,
    exception);
  write_info=DestroyImageInfo(write_info);
  group4_image=DestroyImage(group4_image);
  if (group4 == (unsigned char *) NULL)
    return(MagickFalse);
  status=MagickTrue;
  if (WriteBlob(image,length,group4) != (ssize_t) length)
    status=MagickFalse;
  group4=(unsigned char *) RelinquishMagickMemory(group4);
  return(status);
}

static MagickBooleanType WritePS2Image(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  static const char
    PostscriptProlog[] =
      "%%%%BeginProlog\n"
      "%%\n"
      "%% Display a color image.  The image is displayed in color on\n"
      "%% Postscript viewers or printers that support color, otherwise\n"
      "%% it is displayed as grayscale.\n"
      "%%\n"
      "/DirectClassImage\n"
      "{\n"
      "  %%\n"
      "  %% Display a DirectClass image.\n"
      "  %%\n"
      "  colorspace 0 eq\n"
      "  {\n"
      "    /DeviceRGB setcolorspace\n"
      "    <<\n"
      "      /ImageType 1\n"
      "      /Width columns\n"
      "      /Height rows\n"
      "      /BitsPerComponent 8\n"
      "      /Decode [0 1 0 1 0 1]\n"
      "      /ImageMatrix [columns 0 0 rows neg 0 rows]\n"
      "      compression 0 gt\n"
      "      { /DataSource pixel_stream %s }\n"
      "      { /DataSource pixel_stream %s } ifelse\n"
      "    >> image\n"
      "  }\n"
      "  {\n"
      "    /DeviceCMYK setcolorspace\n"
      "    <<\n"
      "      /ImageType 1\n"
      "      /Width columns\n"
      "      /Height rows\n"
      "      /BitsPerComponent 8\n"
      "      /Decode [1 0 1 0 1 0 1 0]\n"
      "      /ImageMatrix [columns 0 0 rows neg 0 rows]\n"
      "      compression 0 gt\n"
      "      { /DataSource pixel_stream %s }\n"
      "      { /DataSource pixel_stream %s } ifelse\n"
      "    >> image\n"
      "  } ifelse\n"
      "} bind def\n"
      "\n"
      "/PseudoClassImage\n"
      "{\n"
      "  %%\n"
      "  %% Display a PseudoClass image.\n"
      "  %%\n"
      "  %% Parameters:\n"
      "  %%   colors: number of colors in the colormap.\n"
      "  %%\n"
      "  currentfile buffer readline pop\n"
      "  token pop /colors exch def pop\n"
      "  colors 0 eq\n"
      "  {\n"
      "    %%\n"
      "    %% Image is grayscale.\n"
      "    %%\n"
      "    currentfile buffer readline pop\n"
      "    token pop /bits exch def pop\n"
      "    /DeviceGray setcolorspace\n"
      "    <<\n"
      "      /ImageType 1\n"
      "      /Width columns\n"
      "      /Height rows\n"
      "      /BitsPerComponent bits\n"
      "      /Decode [0 1]\n"
      "      /ImageMatrix [columns 0 0 rows neg 0 rows]\n"
      "      compression 0 gt\n"
      "      { /DataSource pixel_stream %s }\n"
      "      {\n"
      "        /DataSource pixel_stream %s\n"
      "        <<\n"
      "           /K " CCITTParam "\n"
      "           /Columns columns\n"
      "           /Rows rows\n"
      "        >> /CCITTFaxDecode filter\n"
      "      } ifelse\n"
      "    >> image\n"
      "  }\n"
      "  {\n"
      "    %%\n"
      "    %% Parameters:\n"
      "    %%   colormap: red, green, blue color packets.\n"
      "    %%\n"
      "    /colormap colors 3 mul string def\n"
      "    currentfile colormap readhexstring pop pop\n"
      "    currentfile buffer readline pop\n"
      "    [ /Indexed /DeviceRGB colors 1 sub colormap ] setcolorspace\n"
      "    <<\n"
      "      /ImageType 1\n"
      "      /Width columns\n"
      "      /Height rows\n"
      "      /BitsPerComponent 8\n"
      "      /Decode [0 255]\n"
      "      /ImageMatrix [columns 0 0 rows neg 0 rows]\n"
      "      compression 0 gt\n"
      "      { /DataSource pixel_stream %s }\n"
      "      { /DataSource pixel_stream %s } ifelse\n"
      "    >> image\n"
      "  } ifelse\n"
      "} bind def\n"
      "\n"
      "/DisplayImage\n"
      "{\n"
      "  %%\n"
      "  %% Display a DirectClass or PseudoClass image.\n"
      "  %%\n"
      "  %% Parameters:\n"
      "  %%   x & y translation.\n"
      "  %%   x & y scale.\n"
      "  %%   label pointsize.\n"
      "  %%   image label.\n"
      "  %%   image columns & rows.\n"
      "  %%   class: 0-DirectClass or 1-PseudoClass.\n"
      "  %%   colorspace: 0-RGB or 1-CMYK.\n"
      "  %%   compression: 0-RLECompression or 1-NoCompression.\n"
      "  %%   hex color packets.\n"
      "  %%\n"
      "  gsave\n"
      "  /buffer 512 string def\n"
      "  /pixel_stream currentfile def\n"
      "\n"
      "  currentfile buffer readline pop\n"
      "  token pop /x exch def\n"
      "  token pop /y exch def pop\n"
      "  x y translate\n"
      "  currentfile buffer readline pop\n"
      "  token pop /x exch def\n"
      "  token pop /y exch def pop\n"
      "  currentfile buffer readline pop\n"
      "  token pop /pointsize exch def pop\n",
    PostscriptEpilog[] =
      "  x y scale\n"
      "  currentfile buffer readline pop\n"
      "  token pop /columns exch def\n"
      "  token pop /rows exch def pop\n"
      "  currentfile buffer readline pop\n"
      "  token pop /class exch def pop\n"
      "  currentfile buffer readline pop\n"
      "  token pop /colorspace exch def pop\n"
      "  currentfile buffer readline pop\n"
      "  token pop /compression exch def pop\n"
      "  class 0 gt { PseudoClassImage } { DirectClassImage } ifelse\n"
      "  grestore\n";

  char
    buffer[MagickPathExtent],
    date[MagickPathExtent],
    page_geometry[MagickPathExtent],
    **labels;

  CompressionType
    compression;

  const char
    *filter,
    *value;

  double
    pointsize;

  GeometryInfo
    geometry_info;

  MagickOffsetType
    scene,
    start,
    stop;

  MagickBooleanType
    progress,
    status;

  MagickOffsetType
    offset;

  MagickSizeType
    number_pixels;

  MagickStatusType
    flags;

  PointInfo
    delta,
    resolution,
    scale;

  RectangleInfo
    geometry,
    media_info,
    page_info;

  register const Quantum
    *p;

  register ssize_t
    x;

  register ssize_t
    i;

  SegmentInfo
    bounds;

  size_t
    imageListLength,
    length,
    page,
    text_size;

  ssize_t
    j,
    y;

  time_t
    timer;

  unsigned char
    *pixels;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  compression=image->compression;
  if (image_info->compression != UndefinedCompression)
    compression=image_info->compression;
  switch (compression)
  {
#if !defined(MAGICKCORE_JPEG_DELEGATE)
    case JPEGCompression:
    {
      compression=RLECompression;
      (void) ThrowMagickException(exception,GetMagickModule(),
        MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","`%s' (JPEG)",
        image->filename);
      break;
    }
#endif
    default:
      break;
  }
  (void) memset(&bounds,0,sizeof(bounds));
  page=1;
  scene=0;
  imageListLength=GetImageListLength(image);
  do
  {
    /*
      Scale relative to dots-per-inch.
    */
    delta.x=DefaultResolution;
    delta.y=DefaultResolution;
    resolution.x=image->resolution.x;
    resolution.y=image->resolution.y;
    if ((resolution.x == 0.0) || (resolution.y == 0.0))
      {
        flags=ParseGeometry(PSDensityGeometry,&geometry_info);
        resolution.x=geometry_info.rho;
        resolution.y=geometry_info.sigma;
        if ((flags & SigmaValue) == 0)
          resolution.y=resolution.x;
      }
    if (image_info->density != (char *) NULL)
      {
        flags=ParseGeometry(image_info->density,&geometry_info);
        resolution.x=geometry_info.rho;
        resolution.y=geometry_info.sigma;
        if ((flags & SigmaValue) == 0)
          resolution.y=resolution.x;
      }
    if (image->units == PixelsPerCentimeterResolution)
      {
        resolution.x=(double) (100.0*2.54*resolution.x+0.5)/100.0;
        resolution.y=(double) (100.0*2.54*resolution.y+0.5)/100.0;
      }
    SetGeometry(image,&geometry);
    (void) FormatLocaleString(page_geometry,MagickPathExtent,"%.20gx%.20g",
      (double) image->columns,(double) image->rows);
    if (image_info->page != (char *) NULL)
      (void) CopyMagickString(page_geometry,image_info->page,MagickPathExtent);
    else
      if ((image->page.width != 0) && (image->page.height != 0))
        (void) FormatLocaleString(page_geometry,MagickPathExtent,
          "%.20gx%.20g%+.20g%+.20g",(double) image->page.width,(double)
          image->page.height,(double) image->page.x,(double) image->page.y);
      else
        if ((image->gravity != UndefinedGravity) &&
            (LocaleCompare(image_info->magick,"PS") == 0))
          (void) CopyMagickString(page_geometry,PSPageGeometry,
            MagickPathExtent);
    (void) ConcatenateMagickString(page_geometry,">",MagickPathExtent);
    (void) ParseMetaGeometry(page_geometry,&geometry.x,&geometry.y,
      &geometry.width,&geometry.height);
    scale.x=PerceptibleReciprocal(resolution.x)*geometry.width*delta.x;
    geometry.width=(size_t) floor(scale.x+0.5);
    scale.y=PerceptibleReciprocal(resolution.y)*geometry.height*delta.y;
    geometry.height=(size_t) floor(scale.y+0.5);
    (void) ParseAbsoluteGeometry(page_geometry,&media_info);
    (void) ParseGravityGeometry(image,page_geometry,&page_info,exception);
    if (image->gravity != UndefinedGravity)
      {
        geometry.x=(-page_info.x);
        geometry.y=(ssize_t) (media_info.height+page_info.y-image->rows);
      }
    pointsize=12.0;
    if (image_info->pointsize != 0.0)
      pointsize=image_info->pointsize;
    text_size=0;
    value=GetImageProperty(image,"label",exception);
    if (value != (const char *) NULL)
      text_size=(size_t) (MultilineCensus(value)*pointsize+12);
    if (page == 1)
      {
        /*
          Output Postscript header.
        */
        if (LocaleCompare(image_info->magick,"PS2") == 0)
          (void) CopyMagickString(buffer,"%!PS-Adobe-3.0\n",MagickPathExtent);
        else
          (void) CopyMagickString(buffer,"%!PS-Adobe-3.0 EPSF-3.0\n",
            MagickPathExtent);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,"%%Creator: (ImageMagick)\n");
        (void) FormatLocaleString(buffer,MagickPathExtent,"%%%%Title: (%s)\n",
          image->filename);
        (void) WriteBlobString(image,buffer);
        timer=GetMagickTime();
        (void) FormatMagickTime(timer,MagickPathExtent,date);
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "%%%%CreationDate: (%s)\n",date);
        (void) WriteBlobString(image,buffer);
        bounds.x1=(double) geometry.x;
        bounds.y1=(double) geometry.y;
        bounds.x2=(double) geometry.x+geometry.width;
        bounds.y2=(double) geometry.y+geometry.height+text_size;
        if ((image_info->adjoin != MagickFalse) &&
            (GetNextImageInList(image) != (Image *) NULL))
          (void) CopyMagickString(buffer,"%%BoundingBox: (atend)\n",
            MagickPathExtent);
        else
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,
              "%%%%BoundingBox: %.20g %.20g %.20g %.20g\n",ceil(bounds.x1-0.5),
              ceil(bounds.y1-0.5),floor(bounds.x2+0.5),floor(bounds.y2+0.5));
            (void) WriteBlobString(image,buffer);
            (void) FormatLocaleString(buffer,MagickPathExtent,
              "%%%%HiResBoundingBox: %g %g %g %g\n",bounds.x1,
              bounds.y1,bounds.x2,bounds.y2);
          }
        (void) WriteBlobString(image,buffer);
        value=GetImageProperty(image,"label",exception);
        if (value != (const char *) NULL)
          (void) WriteBlobString(image,
            "%%DocumentNeededResources: font Helvetica\n");
        (void) WriteBlobString(image,"%%LanguageLevel: 2\n");
        if (LocaleCompare(image_info->magick,"PS2") != 0)
          (void) WriteBlobString(image,"%%Pages: 1\n");
        else
          {
            (void) WriteBlobString(image,"%%Orientation: Portrait\n");
            (void) WriteBlobString(image,"%%PageOrder: Ascend\n");
            if (image_info->adjoin == MagickFalse)
              (void) CopyMagickString(buffer,"%%Pages: 1\n",MagickPathExtent);
            else
              (void) FormatLocaleString(buffer,MagickPathExtent,
                "%%%%Pages: %.20g\n",(double) imageListLength);
            (void) WriteBlobString(image,buffer);
          }
        if (image->colorspace == CMYKColorspace)
          (void) WriteBlobString(image,
            "%%DocumentProcessColors: Cyan Magenta Yellow Black\n");
        (void) WriteBlobString(image,"%%EndComments\n");
        (void) WriteBlobString(image,"\n%%BeginDefaults\n");
        (void) WriteBlobString(image,"%%EndDefaults\n\n");
        /*
          Output Postscript commands.
        */
        switch (compression)
        {
          case NoCompression:
          {
            filter="/ASCII85Decode filter";
            break;
          }
          case JPEGCompression:
          {
            filter="/DCTDecode filter";
            break;
          }
          case LZWCompression:
          {
            filter="/LZWDecode filter";
            break;
          }
          case FaxCompression:
          case Group4Compression:
          {
            filter=" ";
            break;
          }
          default:
          {
            filter="/RunLengthDecode filter";
            break;
          }
        }
        (void) FormatLocaleString(buffer,MagickPathExtent,PostscriptProlog,
          filter,filter,filter,filter,filter,filter,filter,filter);
        (void) WriteBlob(image,strlen(buffer),buffer);
        value=GetImageProperty(image,"label",exception);
        if (value != (const char *) NULL)
          {
            (void) WriteBlobString(image,
              "  /Helvetica findfont pointsize scalefont setfont\n");
            for (j=(ssize_t) MultilineCensus(value)-1; j >= 0; j--)
            {
              (void) WriteBlobString(image,"  /label 512 string def\n");
              (void) WriteBlobString(image,
                "  currentfile label readline pop\n");
              (void) FormatLocaleString(buffer,MagickPathExtent,
                "  0 y %g add moveto label show pop\n",j*pointsize+12);
              (void) WriteBlobString(image,buffer);
            }
          }
        (void) WriteBlob(image,sizeof(PostscriptEpilog)-1,
          (const unsigned char *) PostscriptEpilog);
        if (LocaleCompare(image_info->magick,"PS2") == 0)
          (void) WriteBlobString(image,"  showpage\n");
        (void) WriteBlobString(image,"} bind def\n");
        (void) WriteBlobString(image,"%%EndProlog\n");
      }
    (void) FormatLocaleString(buffer,MagickPathExtent,"%%%%Page:  1 %.20g\n",
      (double) page++);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "%%%%PageBoundingBox: %.20g %.20g %.20g %.20g\n",(double) geometry.x,
      (double) geometry.y,geometry.x+(double) geometry.width,geometry.y+(double)
      (geometry.height+text_size));
    (void) WriteBlobString(image,buffer);
    if ((double) geometry.x < bounds.x1)
      bounds.x1=(double) geometry.x;
    if ((double) geometry.y < bounds.y1)
      bounds.y1=(double) geometry.y;
    if ((double) (geometry.x+geometry.width-1) > bounds.x2)
      bounds.x2=(double) geometry.x+geometry.width-1;
    if ((double) (geometry.y+(geometry.height+text_size)-1) > bounds.y2)
      bounds.y2=(double) geometry.y+(geometry.height+text_size)-1;
    value=GetImageProperty(image,"label",exception);
    if (value != (const char *) NULL)
      (void) WriteBlobString(image,"%%PageResources: font Helvetica\n");
    if (LocaleCompare(image_info->magick,"PS2") != 0)
      (void) WriteBlobString(image,"userdict begin\n");
    start=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "%%%%BeginData:%13ld %s Bytes\n",0L,
      compression == NoCompression ? "ASCII" : "Binary");
    (void) WriteBlobString(image,buffer);
    stop=TellBlob(image);
    (void) WriteBlobString(image,"DisplayImage\n");
    /*
      Output image data.
    */
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "%.20g %.20g\n%g %g\n%g\n",(double) geometry.x,(double) geometry.y,
      scale.x,scale.y,pointsize);
    (void) WriteBlobString(image,buffer);
    labels=(char **) NULL;
    value=GetImageProperty(image,"label",exception);
    if (value != (const char *) NULL)
      labels=StringToList(value);
    if (labels != (char **) NULL)
      {
        for (i=0; labels[i] != (char *) NULL; i++)
        {
          (void) FormatLocaleString(buffer,MagickPathExtent,"%s \n",
            labels[i]);
          (void) WriteBlobString(image,buffer);
          labels[i]=DestroyString(labels[i]);
        }
        labels=(char **) RelinquishMagickMemory(labels);
      }
    number_pixels=(MagickSizeType) image->columns*image->rows;
    if (number_pixels != (MagickSizeType) ((size_t) number_pixels))
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    if ((compression == FaxCompression) || (compression == Group4Compression) ||
        ((image_info->type != TrueColorType) &&
         (SetImageGray(image,exception) != MagickFalse)))
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "%.20g %.20g\n1\n%d\n",(double) image->columns,(double) image->rows,
          (int) (image->colorspace == CMYKColorspace));
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,"%d\n",(int)
          ((compression != FaxCompression) &&
           (compression != Group4Compression)));
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,"0\n");
        (void) FormatLocaleString(buffer,MagickPathExtent,"%d\n",
           (compression == FaxCompression) ||
           (compression == Group4Compression) ? 1 : 8);
        (void) WriteBlobString(image,buffer);
        switch (compression)
        {
          case FaxCompression:
          case Group4Compression:
          {
            if (LocaleCompare(CCITTParam,"0") == 0)
              {
                (void) HuffmanEncodeImage(image_info,image,image,exception);
                break;
              }
            (void) Huffman2DEncodeImage(image_info,image,image,exception);
            break;
          }
          case JPEGCompression:
          {
            status=InjectImageBlob(image_info,image,image,"jpeg",exception);
            if (status == MagickFalse)
              {
                (void) CloseBlob(image);
                return(MagickFalse);
              }
            break;
          }
          case RLECompression:
          default:
          {
            MemoryInfo
              *pixel_info;

            register unsigned char
              *q;

            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            pixel_info=AcquireVirtualMemory(length,sizeof(*pixels));
            if (pixel_info == (MemoryInfo *) NULL)
              ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
            pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
            /*
              Dump runlength encoded pixels.
            */
            q=pixels;
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                *q++=ScaleQuantumToChar(ClampToQuantum(GetPixelLuma(image,p)));
                p+=GetPixelChannels(image);
              }
              progress=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
                image->rows);
              if (progress == MagickFalse)
                break;
            }
            length=(size_t) (q-pixels);
            if (compression == LZWCompression)
              status=LZWEncodeImage(image,length,pixels,exception);
            else
              status=PackbitsEncodeImage(image,length,pixels,exception);
            pixel_info=RelinquishVirtualMemory(pixel_info);
            if (status == MagickFalse)
              {
                (void) CloseBlob(image);
                return(MagickFalse);
              }
            break;
          }
          case NoCompression:
          {
            /*
              Dump uncompressed PseudoColor packets.
            */
            Ascii85Initialize(image);
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                Ascii85Encode(image,ScaleQuantumToChar(ClampToQuantum(
                  GetPixelLuma(image,p))));
                p+=GetPixelChannels(image);
              }
              progress=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                y,image->rows);
              if (progress == MagickFalse)
                break;
            }
            Ascii85Flush(image);
            break;
          }
        }
      }
    else
      if ((image->storage_class == DirectClass) || (image->colors > 256) ||
          (compression == JPEGCompression) || (image->alpha_trait != UndefinedPixelTrait))
        {
          (void) FormatLocaleString(buffer,MagickPathExtent,
            "%.20g %.20g\n0\n%d\n",(double) image->columns,(double) image->rows,
            (int) (image->colorspace == CMYKColorspace));
          (void) WriteBlobString(image,buffer);
          (void) FormatLocaleString(buffer,MagickPathExtent,"%d\n",
            (int) (compression == NoCompression));
          (void) WriteBlobString(image,buffer);
          switch (compression)
          {
            case JPEGCompression:
            {
              status=InjectImageBlob(image_info,image,image,"jpeg",exception);
              if (status == MagickFalse)
                {
                  (void) CloseBlob(image);
                  return(MagickFalse);
                }
              break;
            }
            case RLECompression:
            default:
            {
              MemoryInfo
                *pixel_info;

              register unsigned char
                *q;

              /*
                Allocate pixel array.
              */
              length=(size_t) number_pixels;
              pixel_info=AcquireVirtualMemory(length,4*sizeof(*pixels));
              if (pixel_info == (MemoryInfo *) NULL)
                ThrowWriterException(ResourceLimitError,
                  "MemoryAllocationFailed");
              pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
              /*
                Dump runlength encoded pixels.
              */
              q=pixels;
              for (y=0; y < (ssize_t) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                for (x=0; x < (ssize_t) image->columns; x++)
                {
                  if ((image->alpha_trait != UndefinedPixelTrait) &&
                      (GetPixelAlpha(image,p) == (Quantum) TransparentAlpha))
                    {
                      *q++=ScaleQuantumToChar(QuantumRange);
                      *q++=ScaleQuantumToChar(QuantumRange);
                      *q++=ScaleQuantumToChar(QuantumRange);
                    }
                  else
                    if (image->colorspace != CMYKColorspace)
                      {
                        *q++=ScaleQuantumToChar(GetPixelRed(image,p));
                        *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
                        *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
                      }
                    else
                      {
                        *q++=ScaleQuantumToChar(GetPixelRed(image,p));
                        *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
                        *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
                        *q++=ScaleQuantumToChar(GetPixelBlack(image,p));
                      }
                  p+=GetPixelChannels(image);
                }
                progress=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                  y,image->rows);
                if (progress == MagickFalse)
                  break;
              }
              length=(size_t) (q-pixels);
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels,exception);
              else
                status=PackbitsEncodeImage(image,length,pixels,exception);
              if (status == MagickFalse)
                {
                  (void) CloseBlob(image);
                  return(MagickFalse);
                }
              pixel_info=RelinquishVirtualMemory(pixel_info);
              break;
            }
            case NoCompression:
            {
              /*
                Dump uncompressed DirectColor packets.
              */
              Ascii85Initialize(image);
              for (y=0; y < (ssize_t) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                for (x=0; x < (ssize_t) image->columns; x++)
                {
                  if ((image->alpha_trait != UndefinedPixelTrait) &&
                      (GetPixelAlpha(image,p) == (Quantum) TransparentAlpha))
                    {
                      Ascii85Encode(image,ScaleQuantumToChar((Quantum)
                        QuantumRange));
                      Ascii85Encode(image,ScaleQuantumToChar((Quantum)
                        QuantumRange));
                      Ascii85Encode(image,ScaleQuantumToChar((Quantum)
                        QuantumRange));
                    }
                  else
                    if (image->colorspace != CMYKColorspace)
                      {
                        Ascii85Encode(image,ScaleQuantumToChar(
                          GetPixelRed(image,p)));
                        Ascii85Encode(image,ScaleQuantumToChar(
                          GetPixelGreen(image,p)));
                        Ascii85Encode(image,ScaleQuantumToChar(
                          GetPixelBlue(image,p)));
                      }
                    else
                      {
                        Ascii85Encode(image,ScaleQuantumToChar(
                          GetPixelRed(image,p)));
                        Ascii85Encode(image,ScaleQuantumToChar(
                          GetPixelGreen(image,p)));
                        Ascii85Encode(image,ScaleQuantumToChar(
                          GetPixelBlue(image,p)));
                        Ascii85Encode(image,ScaleQuantumToChar(
                          GetPixelBlack(image,p)));
                      }
                  p+=GetPixelChannels(image);
                }
                progress=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                  y,image->rows);
                if (progress == MagickFalse)
                  break;
              }
              Ascii85Flush(image);
              break;
            }
          }
        }
      else
        {
          /*
            Dump number of colors and colormap.
          */
          (void) FormatLocaleString(buffer,MagickPathExtent,
            "%.20g %.20g\n1\n%d\n",(double) image->columns,(double) image->rows,
            (int) (image->colorspace == CMYKColorspace));
          (void) WriteBlobString(image,buffer);
          (void) FormatLocaleString(buffer,MagickPathExtent,"%d\n",
            (int) (compression == NoCompression));
          (void) WriteBlobString(image,buffer);
          (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double)
            image->colors);
          (void) WriteBlobString(image,buffer);
          for (i=0; i < (ssize_t) image->colors; i++)
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,"%02X%02X%02X\n",
              ScaleQuantumToChar(image->colormap[i].red),
              ScaleQuantumToChar(image->colormap[i].green),
              ScaleQuantumToChar(image->colormap[i].blue));
            (void) WriteBlobString(image,buffer);
          }
          switch (compression)
          {
            case RLECompression:
            default:
            {
              MemoryInfo
                *pixel_info;

              register unsigned char
                *q;

              /*
                Allocate pixel array.
              */
              length=(size_t) number_pixels;
              pixel_info=AcquireVirtualMemory(length,sizeof(*pixels));
              if (pixel_info == (MemoryInfo *) NULL)
                ThrowWriterException(ResourceLimitError,
                  "MemoryAllocationFailed");
              pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
              /*
                Dump runlength encoded pixels.
              */
              q=pixels;
              for (y=0; y < (ssize_t) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                for (x=0; x < (ssize_t) image->columns; x++)
                {
                  *q++=(unsigned char) GetPixelIndex(image,p);
                  p+=GetPixelChannels(image);
                }
                progress=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                  y,image->rows);
                if (progress == MagickFalse)
                  break;
              }
              length=(size_t) (q-pixels);
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels,exception);
              else
                status=PackbitsEncodeImage(image,length,pixels,exception);
              pixel_info=RelinquishVirtualMemory(pixel_info);
              if (status == MagickFalse)
                {
                  (void) CloseBlob(image);
                  return(MagickFalse);
                }
              break;
            }
            case NoCompression:
            {
              /*
                Dump uncompressed PseudoColor packets.
              */
              Ascii85Initialize(image);
              for (y=0; y < (ssize_t) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                for (x=0; x < (ssize_t) image->columns; x++)
                {
                  Ascii85Encode(image,(unsigned char) GetPixelIndex(image,p));
                  p+=GetPixelChannels(image);
                }
                progress=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                  y,image->rows);
                if (progress == MagickFalse)
                  break;
              }
              Ascii85Flush(image);
              break;
            }
          }
        }
    (void) WriteBlobByte(image,'\n');
    length=(size_t) (TellBlob(image)-stop);
    stop=TellBlob(image);
    offset=SeekBlob(image,start,SEEK_SET);
    if (offset < 0)
      ThrowWriterException(CorruptImageError,"ImproperImageHeader");
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "%%%%BeginData:%13ld %s Bytes\n",(long) length,
      compression == NoCompression ? "ASCII" : "Binary");
    (void) WriteBlobString(image,buffer);
    offset=SeekBlob(image,stop,SEEK_SET);
    (void) WriteBlobString(image,"%%EndData\n");
    if (LocaleCompare(image_info->magick,"PS2") != 0)
      (void) WriteBlobString(image,"end\n");
    (void) WriteBlobString(image,"%%PageTrailer\n");
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,imageListLength);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) WriteBlobString(image,"%%Trailer\n");
  if (page > 1)
    {
      (void) FormatLocaleString(buffer,MagickPathExtent,
        "%%%%BoundingBox: %.20g %.20g %.20g %.20g\n",ceil(bounds.x1-0.5),
        ceil(bounds.y1-0.5),floor(bounds.x2+0.5),floor(bounds.y2+0.5));
      (void) WriteBlobString(image,buffer);
      (void) FormatLocaleString(buffer,MagickPathExtent,
        "%%%%HiResBoundingBox: %g %g %g %g\n",bounds.x1,bounds.y1,
        bounds.x2,bounds.y2);
      (void) WriteBlobString(image,buffer);
    }
  (void) WriteBlobString(image,"%%EOF\n");
  (void) CloseBlob(image);
  return(MagickTrue);
}

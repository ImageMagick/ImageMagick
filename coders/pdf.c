/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   DDDD   FFFFF                              %
%                            P   P  D   D  F                                  %
%                            PPPP   D   D  FFF                                %
%                            P      D   D  F                                  %
%                            P      DDDD   F                                  %
%                                                                             %
%                                                                             %
%                   Read/Write Portable Document Format                       %
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
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/channel.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/compress.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/delegate-private.h"
#include "MagickCore/distort.h"
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
#include "MagickCore/montage.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/resize.h"
#include "MagickCore/signature.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/timer-private.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/xml-tree-private.h"
#include "coders/bytebuffer-private.h"
#include "coders/coders-private.h"
#include "coders/ghostscript-private.h"

/*
  Define declarations.
*/
#if defined(MAGICKCORE_TIFF_DELEGATE)
#define CCITTParam  "-1"
#else
#define CCITTParam  "0"
#endif

/*
  Typedef declarations.
*/
typedef struct _PDFInfo
{
  double
    angle;

  MagickBooleanType
    cmyk,
    cropbox,
    trimbox;

  SegmentInfo
    bounds;

  StringInfo
    *xmp_profile;
} PDFInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePDFImage(const ImageInfo *,Image *,ExceptionInfo *),
  WritePOCKETMODImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P D F                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPDF() returns MagickTrue if the image format type, identified by the
%  magick string, is PDF.
%
%  The format of the IsPDF method is:
%
%      MagickBooleanType IsPDF(const unsigned char *magick,const size_t offset)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o offset: Specifies the offset of the magick string.
%
*/
static MagickBooleanType IsPDF(const unsigned char *magick,const size_t offset)
{
  if (offset < 5)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"%PDF-",5) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P D F I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPDFImage() reads a Portable Document Format image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadPDFImage method is:
%
%      Image *ReadPDFImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void ReadPDFInfo(const ImageInfo *image_info,Image *image,
  PDFInfo *pdf_info,ExceptionInfo *exception)
{
#define CMYKProcessColor  "CMYKProcessColor"
#define CropBox  "CropBox"
#define DefaultCMYK  "DefaultCMYK"
#define DeviceCMYK  "DeviceCMYK"
#define MediaBox  "MediaBox"
#define PDFRotate  "Rotate"
#define SpotColor  "Separation"
#define TrimBox  "TrimBox"
#define PDFVersion  "PDF-"

  char
    version[MagickPathExtent];

  int
    c,
    percent_count;

  MagickByteBuffer
    buffer;

  char
    *p;

  ssize_t
    i;

  SegmentInfo
    bounds;

  size_t
    spotcolor;

  ssize_t
    count;

  (void) memset(&bounds,0,sizeof(bounds));
  (void) memset(pdf_info,0,sizeof(*pdf_info));
  pdf_info->cmyk=image_info->colorspace == CMYKColorspace ? MagickTrue :
    MagickFalse;
  pdf_info->cropbox=IsStringTrue(GetImageOption(image_info,"pdf:use-cropbox"));
  pdf_info->trimbox=IsStringTrue(GetImageOption(image_info,"pdf:use-trimbox"));
  *version='\0';
  spotcolor=0;
  percent_count=0;
  (void) memset(&buffer,0,sizeof(buffer));
  buffer.image=image;
  for (c=ReadMagickByteBuffer(&buffer); c != EOF; c=ReadMagickByteBuffer(&buffer))
  {
    if (c == '%')
      {
        if (*version == '\0')
          {
            i=0;
            for (c=ReadMagickByteBuffer(&buffer); c != EOF; c=ReadMagickByteBuffer(&buffer))
            {
              if ((c == '\r') || (c == '\n') || ((i+1) == MagickPathExtent))
                break;
              version[i++]=(char) c;
            }
            version[i]='\0';
            if (c == EOF)
              break;
          }
        if (++percent_count == 2)
          percent_count=0;
        else
          continue;
      }
    else
      {
        percent_count=0;
        switch(c)
        {
          case '<':
          {
            ReadGhostScriptXMPProfile(&buffer,&pdf_info->xmp_profile,exception);
            continue;
          }
          case '/':
            break;
          default:
            continue;
        }
      }
    if (CompareMagickByteBuffer(&buffer,PDFRotate,strlen(PDFRotate)) != MagickFalse)
      {
        p=GetMagickByteBufferDatum(&buffer);
        (void) MagickSscanf(p,PDFRotate" %lf",&pdf_info->angle);
      }
    if (pdf_info->cmyk == MagickFalse)
      {
        if ((CompareMagickByteBuffer(&buffer,DefaultCMYK,strlen(DefaultCMYK)) != MagickFalse) ||
            (CompareMagickByteBuffer(&buffer,DeviceCMYK,strlen(DeviceCMYK)) != MagickFalse) ||
            (CompareMagickByteBuffer(&buffer,CMYKProcessColor,strlen(CMYKProcessColor)) != MagickFalse))
          {
            pdf_info->cmyk=MagickTrue;
            continue;
          }
      }
    if (CompareMagickByteBuffer(&buffer,SpotColor,strlen(SpotColor)) != MagickFalse)
      {
        char
          name[MagickPathExtent],
          property[MagickPathExtent],
          *value;

        /*
          Note spot names.
        */
        (void) FormatLocaleString(property,MagickPathExtent,
          "pdf:SpotColor-%.20g",(double) spotcolor++);
        i=0;
        SkipMagickByteBuffer(&buffer,strlen(SpotColor)+1);
        for (c=ReadMagickByteBuffer(&buffer); c != EOF; c=ReadMagickByteBuffer(&buffer))
        {
          if ((isspace((int) ((unsigned char) c)) != 0) || (c == '/') || ((i+1) == MagickPathExtent))
            break;
          name[i++]=(char) c;
        }
        if (c == EOF)
          break;
        name[i]='\0';
        value=ConstantString(name);
        (void) SubstituteString(&value,"#20"," ");
        if (*value != '\0')
          (void) SetImageProperty(image,property,value,exception);
        value=DestroyString(value);
        continue;
      }
    if (image_info->page != (char *) NULL)
      continue;
    count=0;
    if (pdf_info->cropbox != MagickFalse)
      {
        if (CompareMagickByteBuffer(&buffer,CropBox,strlen(CropBox)) != MagickFalse)
          {
            /*
              Note region defined by crop box.
            */
            p=GetMagickByteBufferDatum(&buffer);
            count=(ssize_t) MagickSscanf(p,"CropBox [%lf %lf %lf %lf",&bounds.x1,
              &bounds.y1,&bounds.x2,&bounds.y2);
            if (count != 4)
              count=(ssize_t) MagickSscanf(p,"CropBox[%lf %lf %lf %lf",&bounds.x1,
                &bounds.y1,&bounds.x2,&bounds.y2);
          }
      }
    else
      if (pdf_info->trimbox != MagickFalse)
        {
          if (CompareMagickByteBuffer(&buffer,TrimBox,strlen(TrimBox)) != MagickFalse)
            {
              /*
                Note region defined by trim box.
              */
              p=GetMagickByteBufferDatum(&buffer);
              count=(ssize_t) MagickSscanf(p,"TrimBox [%lf %lf %lf %lf",&bounds.x1,
                &bounds.y1,&bounds.x2,&bounds.y2);
              if (count != 4)
                count=(ssize_t) MagickSscanf(p,"TrimBox[%lf %lf %lf %lf",&bounds.x1,
                  &bounds.y1,&bounds.x2,&bounds.y2);
            }
        }
      else
        if (CompareMagickByteBuffer(&buffer,MediaBox,strlen(MediaBox)) != MagickFalse)
          {
            /*
              Note region defined by media box.
            */
            p=GetMagickByteBufferDatum(&buffer);
            count=(ssize_t) MagickSscanf(p,"MediaBox [%lf %lf %lf %lf",&bounds.x1,
              &bounds.y1,&bounds.x2,&bounds.y2);
            if (count != 4)
              count=(ssize_t) MagickSscanf(p,"MediaBox[%lf %lf %lf %lf",&bounds.x1,
                &bounds.y1,&bounds.x2,&bounds.y2);
          }
    if (count != 4)
      continue;
    if ((fabs(bounds.x2-bounds.x1) <= fabs(pdf_info->bounds.x2-pdf_info->bounds.x1)) ||
        (fabs(bounds.y2-bounds.y1) <= fabs(pdf_info->bounds.y2-pdf_info->bounds.y1)))
      continue;
    pdf_info->bounds=bounds;
  }
  if (version[0] != '\0')
    (void) SetImageProperty(image,"pdf:Version",version,exception);
}

static inline void CleanupPDFInfo(PDFInfo *pdf_info)
{
  if (pdf_info->xmp_profile != (StringInfo *) NULL)
    pdf_info->xmp_profile=DestroyStringInfo(pdf_info->xmp_profile);
}

static Image *ReadPDFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    command[MagickPathExtent],
    *density,
    filename[MagickPathExtent],
    input_filename[MagickPathExtent],
    message[MagickPathExtent],
    *options,
    postscript_filename[MagickPathExtent];

  const char
    *option;

  const DelegateInfo
    *delegate_info;

  GeometryInfo
    geometry_info;

  Image
    *image,
    *next,
    *pdf_image;

  ImageInfo
    *read_info;

  int
    file;

  MagickBooleanType
    fitPage,
    status;

  MagickStatusType
    flags;

  PDFInfo
    pdf_info;

  PointInfo
    delta;

  RectangleInfo
    page;

  ssize_t
    i;

  size_t
    scene;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  /*
    Open image file.
  */
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  status=AcquireUniqueSymbolicLink(image_info->filename,input_filename);
  if (status == MagickFalse)
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        image_info->filename);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Set the page density.
  */
  delta.x=DefaultResolution;
  delta.y=DefaultResolution;
  (void) memset(&page,0,sizeof(page));
  if ((image->resolution.x == 0.0) || (image->resolution.y == 0.0))
    {
      flags=ParseGeometry(PSDensityGeometry,&geometry_info);
      if ((flags & RhoValue) != 0)
        image->resolution.x=geometry_info.rho;
      image->resolution.y=image->resolution.x;
      if ((flags & SigmaValue) != 0)
        image->resolution.y=geometry_info.sigma;
    }
  if (image_info->density != (char *) NULL)
    {
      flags=ParseGeometry(image_info->density,&geometry_info);
      if ((flags & RhoValue) != 0)
        image->resolution.x=geometry_info.rho;
      image->resolution.y=image->resolution.x;
      if ((flags & SigmaValue) != 0)
        image->resolution.y=geometry_info.sigma;
    }
  (void) ParseAbsoluteGeometry(PSPageGeometry,&page);
  if (image_info->page != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->page,&page);
  page.width=(size_t) ((ssize_t) ceil((double) (page.width*
    image->resolution.x/delta.x)-0.5));
  page.height=(size_t) ((ssize_t) ceil((double) (page.height*
    image->resolution.y/delta.y)-0.5));
  /*
    Determine page geometry from the PDF media box.
  */
  ReadPDFInfo(image_info,image,&pdf_info,exception);
  (void) CloseBlob(image);
  /*
    Set PDF render geometry.
  */
  if ((fabs(pdf_info.bounds.x2-pdf_info.bounds.x1) >= MagickEpsilon) &&
      (fabs(pdf_info.bounds.y2-pdf_info.bounds.y1) >= MagickEpsilon))
    {
      (void) FormatImageProperty(image,"pdf:HiResBoundingBox",
        "%gx%g%+.15g%+.15g",pdf_info.bounds.x2-pdf_info.bounds.x1,
        pdf_info.bounds.y2-pdf_info.bounds.y1,pdf_info.bounds.x1,
        pdf_info.bounds.y1);
      page.width=CastDoubleToSizeT(ceil((double) ((pdf_info.bounds.x2-
        pdf_info.bounds.x1)*image->resolution.x/delta.x)-0.5));
      page.height=CastDoubleToSizeT(ceil((double) ((pdf_info.bounds.y2-
        pdf_info.bounds.y1)*image->resolution.y/delta.y)-0.5));
    }
  fitPage=MagickFalse;
  option=GetImageOption(image_info,"pdf:fit-page");
  if (option != (char *) NULL)
    {
      char
        *page_geometry;

      page_geometry=GetPageGeometry(option);
      flags=ParseMetaGeometry(page_geometry,&page.x,&page.y,&page.width,
        &page.height);
      page_geometry=DestroyString(page_geometry);
      if (flags == NoValue)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "InvalidGeometry","`%s'",option);
          CleanupPDFInfo(&pdf_info);
          image=DestroyImage(image);
          return((Image *) NULL);
        }
      page.width=(size_t) ((ssize_t) ceil((double) (page.width*
        image->resolution.x/delta.x)-0.5));
      page.height=(size_t) ((ssize_t) ceil((double) (page.height*
        image->resolution.y/delta.y)-0.5));
      fitPage=MagickTrue;
    }
  if ((fabs(pdf_info.angle) == 90.0) || (fabs(pdf_info.angle) == 270.0))
    {
      size_t
        swap;

      swap=page.width;
      page.width=page.height;
      page.height=swap;
    }
  if (IssRGBCompatibleColorspace(image_info->colorspace) != MagickFalse)
    pdf_info.cmyk=MagickFalse;
  /*
    Create Ghostscript control file.
  */
  file=AcquireUniqueFileResource(postscript_filename);
  if (file == -1)
    {
      (void) RelinquishUniqueFileResource(input_filename);
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        image_info->filename);
      CleanupPDFInfo(&pdf_info);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  if (write(file," ",1) != 1)
    {
      file=close_utf8(file)-1;
      (void) RelinquishUniqueFileResource(input_filename);
      (void) RelinquishUniqueFileResource(postscript_filename);
      CleanupPDFInfo(&pdf_info);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  file=close_utf8(file)-1;
  /*
    Render Postscript with the Ghostscript delegate.
  */
  if (image_info->monochrome != MagickFalse)
    delegate_info=GetDelegateInfo("ps:mono",(char *) NULL,exception);
  else
     if (pdf_info.cmyk != MagickFalse)
       delegate_info=GetDelegateInfo("ps:cmyk",(char *) NULL,exception);
     else
       delegate_info=GetDelegateInfo("ps:alpha",(char *) NULL,exception);
  if (delegate_info == (const DelegateInfo *) NULL)
    {
      (void) RelinquishUniqueFileResource(input_filename);
      (void) RelinquishUniqueFileResource(postscript_filename);
      CleanupPDFInfo(&pdf_info);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  density=AcquireString("");
  options=AcquireString("");
  (void) FormatLocaleString(density,MagickPathExtent,"%gx%g",
    image->resolution.x,image->resolution.y);
  if (image_info->ping != MagickFalse)
    (void) FormatLocaleString(density,MagickPathExtent,"2.0x2.0");
  else
    if ((image_info->page != (char *) NULL) || (fitPage != MagickFalse))
      (void) FormatLocaleString(options,MagickPathExtent,"-g%.20gx%.20g ",
        (double) page.width,(double) page.height);
  option=GetImageOption(image_info,"pdf:printed");
  if (IsStringTrue(option) != MagickFalse)
    (void) ConcatenateMagickString(options,"-dPrinted=true ",MagickPathExtent);
  else
    (void) ConcatenateMagickString(options,"-dPrinted=false ",MagickPathExtent);
  if (fitPage != MagickFalse)
    (void) ConcatenateMagickString(options,"-dPSFitPage ",MagickPathExtent);
  if (pdf_info.cropbox != MagickFalse)
    (void) ConcatenateMagickString(options,"-dUseCropBox ",MagickPathExtent);
  if (pdf_info.trimbox != MagickFalse)
    (void) ConcatenateMagickString(options,"-dUseTrimBox ",MagickPathExtent);
  option=GetImageOption(image_info,"pdf:stop-on-error");
  if (IsStringTrue(option) != MagickFalse)
    (void) ConcatenateMagickString(options,"-dPDFSTOPONERROR ",
      MagickPathExtent);
  option=GetImageOption(image_info,"pdf:interpolate");
  if (IsStringTrue(option) != MagickFalse)
    (void) ConcatenateMagickString(options,"-dInterpolateControl=-1 ",
      MagickPathExtent);
  option=GetImageOption(image_info,"pdf:hide-annotations");
  if (IsStringTrue(option) != MagickFalse)
    (void) ConcatenateMagickString(options,"-dShowAnnots=false ",
      MagickPathExtent);
  option=GetImageOption(image_info,"authenticate");
  if (option != (char *) NULL)
    {
      char
        passphrase[MagickPathExtent];

      FormatSanitizedDelegateOption(passphrase,MagickPathExtent,
        "\"-sPDFPassword=%s\" ","-sPDFPassword='%s' ",option);
      (void) ConcatenateMagickString(options,passphrase,MagickPathExtent);
    }
  read_info=CloneImageInfo(image_info);
  *read_info->magick='\0';
  if (read_info->number_scenes != 0)
    {
      char
        pages[MagickPathExtent];

      (void) FormatLocaleString(pages,MagickPathExtent,"-dFirstPage=%.20g "
        "-dLastPage=%.20g",(double) read_info->scene+1,(double)
        (read_info->scene+read_info->number_scenes));
      (void) ConcatenateMagickString(options,pages,MagickPathExtent);
      read_info->number_scenes=0;
      if (read_info->scenes != (char *) NULL)
        *read_info->scenes='\0';
    }
  (void) CopyMagickString(filename,read_info->filename,MagickPathExtent);
  (void) AcquireUniqueFilename(filename);
  (void) RelinquishUniqueFileResource(filename);
  (void) ConcatenateMagickString(filename,"%d",MagickPathExtent);
  (void) FormatLocaleString(command,MagickPathExtent,
    GetDelegateCommands(delegate_info),
    read_info->antialias != MagickFalse ? 4 : 1,
    read_info->antialias != MagickFalse ? 4 : 1,density,options,filename,
    postscript_filename,input_filename);
  options=DestroyString(options);
  density=DestroyString(density);
  *message='\0';
  status=InvokeGhostscriptDelegate(read_info->verbose,command,message,
    exception);
  (void) RelinquishUniqueFileResource(postscript_filename);
  (void) RelinquishUniqueFileResource(input_filename);
  pdf_image=(Image *) NULL;
  if (status == MagickFalse)
    for (i=1; ; i++)
    {
      (void) InterpretImageFilename(image_info,image,filename,(int) i,
        read_info->filename,exception);
      if (IsGhostscriptRendered(read_info->filename) == MagickFalse)
        break;
      (void) RelinquishUniqueFileResource(read_info->filename);
    }
  else
    {
      next=(Image *) NULL;
      for (i=1; ; i++)
      {
        (void) InterpretImageFilename(image_info,image,filename,(int) i,
          read_info->filename,exception);
        if (IsGhostscriptRendered(read_info->filename) == MagickFalse)
          break;
        read_info->blob=NULL;
        read_info->length=0;
        next=ReadImage(read_info,exception);
        (void) RelinquishUniqueFileResource(read_info->filename);
        if (next == (Image *) NULL)
          break;
        AppendImageToList(&pdf_image,next);
      }
      /* Clean up remaining files */
      if (next == (Image *) NULL)
        {
          ssize_t
            j;

          for (j=i+1; ; j++)
            {
              (void) InterpretImageFilename(image_info,image,filename,(int) j,
                read_info->filename,exception);
              if (IsGhostscriptRendered(read_info->filename) == MagickFalse)
                break;
              (void) RelinquishUniqueFileResource(read_info->filename);
            }
        }
    }
  read_info=DestroyImageInfo(read_info);
  if (pdf_image == (Image *) NULL)
    {
      if (*message != '\0')
        (void) ThrowMagickException(exception,GetMagickModule(),DelegateError,
          "PDFDelegateFailed","`%s'",message);
      CleanupPDFInfo(&pdf_info);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  if (LocaleCompare(pdf_image->magick,"BMP") == 0)
    {
      Image
        *cmyk_image;

      cmyk_image=ConsolidateCMYKImages(pdf_image,exception);
      if (cmyk_image != (Image *) NULL)
        {
          pdf_image=DestroyImageList(pdf_image);
          pdf_image=cmyk_image;
        }
    }
  if (pdf_info.xmp_profile != (StringInfo *) NULL)
    {
      char
        *profile;

      profile=(char *) GetStringInfoDatum(pdf_info.xmp_profile);
      if (strstr(profile,"Adobe Illustrator") != (char *) NULL)
        (void) CopyMagickString(image->magick,"AI",MagickPathExtent);
      (void) SetImageProfilePrivate(image,pdf_info.xmp_profile,exception);
      pdf_info.xmp_profile=(StringInfo *) NULL;
    }
  if (image_info->number_scenes != 0)
    {
      Image
        *clone_image;

      /*
        Add place holder images to meet the subimage specification requirement.
      */
      for (i=0; i < (ssize_t) image_info->scene; i++)
      {
        clone_image=CloneImage(pdf_image,1,1,MagickTrue,exception);
        if (clone_image != (Image *) NULL)
          PrependImageToList(&pdf_image,clone_image);
      }
    }
  do
  {
    (void) CopyMagickString(pdf_image->filename,filename,MagickPathExtent);
    (void) CopyMagickString(pdf_image->magick,image->magick,MagickPathExtent);
    pdf_image->page=page;
    if (image_info->ping != MagickFalse)
      {
        pdf_image->magick_columns=page.width;
        pdf_image->magick_rows=page.height;
        pdf_image->columns=page.width;
        pdf_image->rows=page.height;
      }
    (void) CloneImageProfiles(pdf_image,image);
    (void) CloneImageProperties(pdf_image,image);
    next=SyncNextImageInList(pdf_image);
    if (next != (Image *) NULL)
      pdf_image=next;
  } while (next != (Image *) NULL);
  image=DestroyImage(image);
  scene=0;
  for (next=GetFirstImageInList(pdf_image); next != (Image *) NULL; )
  {
    next->scene=scene++;
    next=GetNextImageInList(next);
  }
  return(GetFirstImageInList(pdf_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P D F I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPDFImage() adds properties for the PDF image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPDFImage method is:
%
%      size_t RegisterPDFImage(void)
%
*/
ModuleExport size_t RegisterPDFImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("PDF","AI","Adobe Illustrator CS2");
  entry->decoder=(DecodeImageHandler *) ReadPDFImage;
  entry->encoder=(EncodeImageHandler *) WritePDFImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/pdf");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PDF","EPDF",
    "Encapsulated Portable Document Format");
  entry->decoder=(DecodeImageHandler *) ReadPDFImage;
  entry->encoder=(EncodeImageHandler *) WritePDFImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/pdf");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PDF","PDF","Portable Document Format");
  entry->decoder=(DecodeImageHandler *) ReadPDFImage;
  entry->encoder=(EncodeImageHandler *) WritePDFImage;
  entry->magick=(IsImageFormatHandler *) IsPDF;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/pdf");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PDF","PDFA","Portable Document Archive Format");
  entry->decoder=(DecodeImageHandler *) ReadPDFImage;
  entry->encoder=(EncodeImageHandler *) WritePDFImage;
  entry->magick=(IsImageFormatHandler *) IsPDF;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/pdf");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PDF","POCKETMOD","Pocketmod Personal Organizer");
  entry->decoder=(DecodeImageHandler *) ReadPDFImage;
  entry->encoder=(EncodeImageHandler *) WritePOCKETMODImage;
  entry->format_type=ImplicitFormatType;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags|=CoderEncoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/pdf");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P D F I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPDFImage() removes format registrations made by the
%  PDF module from the list of supported formats.
%
%  The format of the UnregisterPDFImage method is:
%
%      UnregisterPDFImage(void)
%
*/
ModuleExport void UnregisterPDFImage(void)
{
  (void) UnregisterMagickInfo("AI");
  (void) UnregisterMagickInfo("EPDF");
  (void) UnregisterMagickInfo("PDF");
  (void) UnregisterMagickInfo("PDFA");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P D F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePDFImage() writes an image in the Portable Document image
%  format.
%
%  The format of the WritePDFImage method is:
%
%      MagickBooleanType WritePDFImage(const ImageInfo *image_info,
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

static char *EscapeParenthesis(const char *source)
{
  char
    *destination;

  char
    *q;

  const char
    *p;

  size_t
    length;

  assert(source != (const char *) NULL);
  length=0;
  for (p=source; *p != '\0'; p++)
  {
    if ((*p == '\\') || (*p == '(') || (*p == ')'))
      {
        if (~length < 1)
          ThrowFatalException(ResourceLimitFatalError,"UnableToEscapeString");
        length++;
      }
    length++;
  }
  destination=(char *) NULL;
  if (~length >= (MagickPathExtent-1))
    destination=(char *) AcquireQuantumMemory(length+MagickPathExtent,
      sizeof(*destination));
  if (destination == (char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToEscapeString");
  *destination='\0';
  q=destination;
  for (p=source; *p != '\0'; p++)
  {
    if ((*p == '\\') || (*p == '(') || (*p == ')'))
      *q++='\\';
    *q++=(*p);
  }
  *q='\0';
  return(destination);
}

static size_t UTF8ToUTF16(const unsigned char *utf8,wchar_t *utf16)
{
  const unsigned char
    *p;

  if (utf16 != (wchar_t *) NULL)
    {
      wchar_t
        *q;

      wchar_t
        c;

      /*
        Convert UTF-8 to UTF-16.
      */
      q=utf16;
      for (p=utf8; *p != '\0'; p++)
      {
        if ((*p & 0x80) == 0)
          *q=(*p);
        else
          if ((*p & 0xE0) == 0xC0)
            {
              c=(*p);
              *q=(c & 0x1F) << 6;
              p++;
              if ((*p & 0xC0) != 0x80)
                return(0);
              *q|=(*p & 0x3F);
            }
          else
            if ((*p & 0xF0) == 0xE0)
              {
                c=(*p);
                *q=c << 12;
                p++;
                if ((*p & 0xC0) != 0x80)
                  return(0);
                c=(*p);
                *q|=(c & 0x3F) << 6;
                p++;
                if ((*p & 0xC0) != 0x80)
                  return(0);
                *q|=(*p & 0x3F);
              }
            else
              return(0);
        q++;
      }
      *q++=(wchar_t) '\0';
      return((size_t) (q-utf16));
    }
  /*
    Compute UTF-16 string length.
  */
  for (p=utf8; *p != '\0'; p++)
  {
    if ((*p & 0x80) == 0)
      ;
    else
      if ((*p & 0xE0) == 0xC0)
        {
          p++;
          if ((*p & 0xC0) != 0x80)
            return(0);
        }
      else
        if ((*p & 0xF0) == 0xE0)
          {
            p++;
            if ((*p & 0xC0) != 0x80)
              return(0);
            p++;
            if ((*p & 0xC0) != 0x80)
              return(0);
         }
       else
         return(0);
  }
  return((size_t) (p-utf8));
}

static wchar_t *ConvertUTF8ToUTF16(const unsigned char *source,size_t *length)
{
  wchar_t
    *utf16;

  *length=UTF8ToUTF16(source,(wchar_t *) NULL);
  if (*length == 0)
    {
      ssize_t
        i;

      /*
        Not UTF-8, just copy.
      */
      *length=strlen((const char *) source);
      utf16=(wchar_t *) AcquireQuantumMemory(*length+1,sizeof(*utf16));
      if (utf16 == (wchar_t *) NULL)
        return((wchar_t *) NULL);
      for (i=0; i <= (ssize_t) *length; i++)
        utf16[i]=source[i];
      return(utf16);
    }
  utf16=(wchar_t *) AcquireQuantumMemory(*length+1,sizeof(*utf16));
  if (utf16 == (wchar_t *) NULL)
    return((wchar_t *) NULL);
  *length=UTF8ToUTF16(source,utf16);
  return(utf16);
}

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
  status=MagickTrue;
  write_info=CloneImageInfo(image_info);
  (void) CopyMagickString(write_info->filename,"GROUP4:",MagickPathExtent);
  (void) CopyMagickString(write_info->magick,"GROUP4",MagickPathExtent);
  (void) SetImageArtifact(group4_image,"tiff:photometric","min-is-white");
  group4=(unsigned char *) ImageToBlob(write_info,group4_image,&length,
    exception);
  group4_image=DestroyImage(group4_image);
  write_info=DestroyImageInfo(write_info);
  if (group4 == (unsigned char *) NULL)
    return(MagickFalse);
  if (WriteBlob(image,length,group4) != (ssize_t) length)
    status=MagickFalse;
  group4=(unsigned char *) RelinquishMagickMemory(group4);
  return(status);
}

static MagickBooleanType WritePOCKETMODImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
#define PocketPageOrder  "1,2,3,4,0,7,6,5"

  const Image
    *next;

  Image
    *pages,
    *pocket_mod;

  MagickBooleanType
    status;

  ssize_t
    i;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  pocket_mod=NewImageList();
  pages=NewImageList();
  i=0;
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    Image
      *page;

    if ((i == 0) || (i == 5) || (i == 6) || (i == 7))
      page=RotateImage(next,180.0,exception);
    else
      page=CloneImage(next,0,0,MagickTrue,exception);
    if (page == (Image *) NULL)
      break;
    (void) SetImageAlphaChannel(page,RemoveAlphaChannel,exception);
    page->scene=(size_t) i++;
    AppendImageToList(&pages,page);
    if ((i == 8) || (GetNextImageInList(next) == (Image *) NULL))
      {
        Image
          *images,
          *page_layout;

        MontageInfo
          *montage_info;

        /*
          Create PocketMod page.
        */
        for (i=(ssize_t) GetImageListLength(pages); i < 8; i++)
        {
          page=CloneImage(pages,0,0,MagickTrue,exception);
          (void) QueryColorCompliance("#FFF",AllCompliance,
            &page->background_color,exception);
          (void) SetImageBackgroundColor(page,exception);
          page->scene=(size_t) i;
          AppendImageToList(&pages,page);
        }
        images=CloneImages(pages,PocketPageOrder,exception);
        pages=DestroyImageList(pages);
        if (images == (Image *) NULL)
          break;
        montage_info=CloneMontageInfo(image_info,(MontageInfo *) NULL);
        (void) CloneString(&montage_info->geometry,"877x1240+0+0");
        (void) CloneString(&montage_info->tile,"4x2");
        (void) QueryColorCompliance("#000",AllCompliance,
          &montage_info->border_color,exception);
        montage_info->border_width=2;
        page_layout=MontageImages(images,montage_info,exception);
        montage_info=DestroyMontageInfo(montage_info);
        images=DestroyImageList(images);
        if (page_layout == (Image *) NULL)
          break;
        AppendImageToList(&pocket_mod,page_layout);
        i=0;
      }
  }
  if (pocket_mod == (Image *) NULL)
    return(MagickFalse);
  status=WritePDFImage(image_info,GetFirstImageInList(pocket_mod),exception);
  pocket_mod=DestroyImageList(pocket_mod);
  return(status);
}

static const char *GetPDFAuthor(const ImageInfo *image_info)
{
  const char
    *option;

  option=GetImageOption(image_info,"pdf:author");
  if (option != (const char *) NULL)
    return(option);
  return(MagickAuthoritativeURL);
}

static const char *GetPDFCreator(const ImageInfo *image_info)
{
  const char
    *option;

  option=GetImageOption(image_info,"pdf:creator");
  if (option != (const char *) NULL)
    return(option);
  return(MagickAuthoritativeURL);
}

static const char *GetPDFProducer(const ImageInfo *image_info)
{
  const char
    *option;

  option=GetImageOption(image_info,"pdf:producer");
  if (option != (const char *) NULL)
    return(option);
  return(MagickAuthoritativeURL);
}

static const char *GetPDFTitle(const ImageInfo *image_info,
  const char *default_title)
{
  const char
    *option;

  option=GetImageOption(image_info,"pdf:title");
  if (option != (const char *) NULL)
    return(option);
  return(default_title);
}

static time_t GetPdfCreationDate(const ImageInfo *image_info,const Image* image)
{
  const char
    *option;

  option=GetImageOption(image_info,"pdf:create-epoch");
  if (option != (const char *) NULL)
    {
      time_t
        epoch;

      epoch=(time_t) StringToDouble(option,(char **) NULL);
      if (epoch > 0)
        return(epoch);
    }
  return(GetBlobProperties(image)->st_ctime);
}

static time_t GetPdfModDate(const ImageInfo *image_info,const Image* image)
{
  const char
    *option;

  option=GetImageOption(image_info,"pdf:modify-epoch");
  if (option != (const char *) NULL)
    {
      time_t
        epoch;

      epoch=(time_t) StringToDouble(option,(char **) NULL);
      if (epoch > 0)
        return(epoch);
    }
  return(GetBlobProperties(image)->st_mtime);
}

static const char *GetPDFSubject(const ImageInfo *image_info)
{
  const char
    *option;

  option=GetImageOption(image_info,"pdf:subject");
  if (option != (const char *) NULL)
    return(option);
  return("");
}

static const char *GetPDFKeywords(const ImageInfo *image_info)
{
  const char
    *option;

  option=GetImageOption(image_info,"pdf:keywords");
  if (option != (const char *) NULL)
    return(option);
  return("");
}

static void WritePDFValue(Image* image,const char *keyword,
  const char *value,const MagickBooleanType is_pdfa)
{
  char
    *escaped;

  size_t
    length;

  ssize_t
    i;

  wchar_t
    *utf16;

  if (*value == '\0')
    return;
  if (is_pdfa != MagickFalse)
    {
      escaped=EscapeParenthesis(value);
      (void) WriteBlobString(image,"/");
      (void) WriteBlobString(image,keyword);
      (void) WriteBlobString(image," (");
      (void) WriteBlobString(image,escaped);
      escaped=DestroyString(escaped);
      (void) WriteBlobString(image,")\n");
      return;
    }
  utf16=ConvertUTF8ToUTF16((const unsigned char *) value,&length);
  if (utf16 != (wchar_t *) NULL)
    {
      unsigned char
        hex_digits[16];

      hex_digits[0]='0';
      hex_digits[1]='1';
      hex_digits[2]='2';
      hex_digits[3]='3';
      hex_digits[4]='4';
      hex_digits[5]='5';
      hex_digits[6]='6';
      hex_digits[7]='7';
      hex_digits[8]='8';
      hex_digits[9]='9';
      hex_digits[10]='A';
      hex_digits[11]='B';
      hex_digits[12]='C';
      hex_digits[13]='D';
      hex_digits[14]='E';
      hex_digits[15]='F';
      (void) WriteBlobString(image,"/");
      (void) WriteBlobString(image,keyword);
      (void) WriteBlobString(image," <FEFF");
      for (i=0; i < (ssize_t) length - 1; i++)
      {
        (void) WriteBlobByte(image,hex_digits[(utf16[i] >> 12) & 0x0f]);
        (void) WriteBlobByte(image,hex_digits[(utf16[i] >> 8) & 0x0f]);
        (void) WriteBlobByte(image,hex_digits[(utf16[i] >> 4) & 0x0f]);
        (void) WriteBlobByte(image,hex_digits[utf16[i] & 0x0f]);
      }
      (void) WriteBlobString(image,">\n");
      utf16=(wchar_t *) RelinquishMagickMemory(utf16);
    }
}

static const StringInfo *GetCompatibleColorProfile(const Image* image)
{
  ColorspaceType
    colorspace;

  const StringInfo
    *icc_profile;

  colorspace=UndefinedColorspace;
  icc_profile=GetImageProfile(image,"icc");
  if (icc_profile == (const StringInfo *) NULL)
    return((const StringInfo *) NULL);
  if (GetStringInfoLength(icc_profile) > 20)
    {
      const char
        *p;

      unsigned int
        value;

      p=(const char *) GetStringInfoDatum(icc_profile)+16;
      value=(unsigned int) (*p++) << 24;
      value|=(unsigned int) (*p++) << 16;
      value|=(unsigned int) (*p++) << 8;
      value|=(unsigned int) *p;
      switch (value)
      {
        case 0x58595a20:
          colorspace=XYZColorspace;
          break;
        case 0x4c616220:
          colorspace=LabColorspace;
          break;
        case 0x4c757620:
          colorspace=LuvColorspace;
          break;
        case 0x59436272:
          colorspace=YCbCrColorspace;
          break;
        case 0x52474220:
          if ((image->colorspace == sRGBColorspace) ||
              (image->colorspace == RGBColorspace))
            return(icc_profile);
          break;
        case 0x47524159:
          colorspace=GRAYColorspace;
          break;
        case 0x48535620:
          colorspace=HSVColorspace;
          break;
        case 0x434D594B:
          colorspace=CMYKColorspace;
          break;
        case 0x434D5920:
          colorspace=CMYColorspace;
          break;
      }
    }
  if (image->colorspace == colorspace)
    return(icc_profile);
  return((const StringInfo *) NULL);
}

static MagickBooleanType WritePDFImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
#define CFormat  "/Filter [ /%s ]\n"
#define ObjectsPerImage  14
#define ThrowPDFException(exception,message) \
{ \
  if (xref != (MagickOffsetType *) NULL) \
    xref=(MagickOffsetType *) RelinquishMagickMemory(xref); \
  ThrowWriterException((exception),(message)); \
}

  static const char
    XMPProfile[]=
    {
      "<?xpacket begin=\"%s\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n"
      "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP Core 4.0-c316 44.253921, Sun Oct 01 2006 17:08:23\">\n"
      "   <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
      "      <rdf:Description rdf:about=\"\"\n"
      "            xmlns:xap=\"http://ns.adobe.com/xap/1.0/\"\n"
      "            xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
      "            xmlns:xapMM=\"http://ns.adobe.com/xap/1.0/mm/\"\n"
      "            xmlns:pdf=\"http://ns.adobe.com/pdf/1.3/\"\n"
      "            xmlns:pdfaid=\"http://www.aiim.org/pdfa/ns/id/\">\n"
      "         <xap:CreateDate>%s</xap:CreateDate>\n"
      "         <xap:ModifyDate>%s</xap:ModifyDate>\n"
      "         <xap:MetadataDate>%s</xap:MetadataDate>\n"
      "         <xap:CreatorTool>%s</xap:CreatorTool>\n"
      "         <dc:format>application/pdf</dc:format>\n"
      "         <dc:title>\n"
      "           <rdf:Alt>\n"
      "              <rdf:li xml:lang=\"x-default\">%s</rdf:li>\n"
      "           </rdf:Alt>\n"
      "         </dc:title>\n"
      "         <xapMM:DocumentID>uuid:6ec119d7-7982-4f56-808d-dfe64f5b35cf</xapMM:DocumentID>\n"
      "         <xapMM:InstanceID>uuid:a79b99b4-6235-447f-9f6c-ec18ef7555cb</xapMM:InstanceID>\n"
      "         <pdf:Producer>%s</pdf:Producer>\n"
      "         <pdf:Keywords>%s</pdf:Keywords>\n"
      "         <pdfaid:part>3</pdfaid:part>\n"
      "         <pdfaid:conformance>B</pdfaid:conformance>\n"
      "      </rdf:Description>\n"
      "   </rdf:RDF>\n"
      "</x:xmpmeta>\n"
      "<?xpacket end=\"w\"?>\n"
    },
    XMPProfileMagick[4]= { (char) -17, (char) -69, (char) -65, (char) 0 };

  char
    basename[MagickPathExtent],
    buffer[MagickPathExtent],
    **labels,
    temp[MagickPathExtent];

  CompressionType
    compression;

  const char
    *device,
    *option,
    *value;

  const Quantum
    *p;

  const StringInfo
    *icc_profile;

  double
    pointsize,
    version;

  GeometryInfo
    geometry_info;

  Image
    *next;

  MagickBooleanType
    is_pdfa,
    status;

  MagickOffsetType
    offset,
    scene,
    *xref;

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

  size_t
    channels,
    info_id,
    length,
    number_scenes,
    object,
    pages_id,
    root_id,
    text_size;

  ssize_t
    count,
    i,
    page_count,
    x,
    y;

  struct tm
    utc_time;

  time_t
    seconds;

  unsigned char
    *pixels,
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
  /*
    Allocate X ref memory.
  */
  xref=(MagickOffsetType *) AcquireQuantumMemory(2048UL,sizeof(*xref));
  if (xref == (MagickOffsetType *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(xref,0,2048UL*sizeof(*xref));
  /*
    Write Info object.
  */
  object=0;
  version=1.3;
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
    if (next->alpha_trait != UndefinedPixelTrait)
      version=1.4;
  if (image_info->compression == JPEG2000Compression)
    version=1.5;
  is_pdfa=LocaleCompare(image_info->magick,"PDFA") == 0 ? MagickTrue :
    MagickFalse;
  if (is_pdfa != MagickFalse)
    version=1.6;
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    icc_profile=GetCompatibleColorProfile(next);
    if (icc_profile != (StringInfo *) NULL)
      {
        (void) SetImageStorageClass(next,DirectClass,exception);
        version=1.7;
      }
    if ((next->colorspace != CMYKColorspace) &&
        (IssRGBCompatibleColorspace(next->colorspace) == MagickFalse))
      (void) TransformImageColorspace(next,sRGBColorspace,exception);
    (void) SetImageCoderGray(next,exception);
  }
  option=GetImageOption(image_info,"pdf:version");
  if (option != (const char *) NULL)
    {
      double
        preferred_version;

      preferred_version=StringToDouble(option,(char**) NULL);
      version=MagickMax(version,MagickMin(1.7,preferred_version));
    }
  (void) FormatLocaleString(buffer,MagickPathExtent,"%%PDF-%.2g\n",version);
  (void) WriteBlobString(image,buffer);
  if (is_pdfa != MagickFalse)
    {
      (void) WriteBlobByte(image,'%');
      (void) WriteBlobByte(image,0xe2);
      (void) WriteBlobByte(image,0xe3);
      (void) WriteBlobByte(image,0xcf);
      (void) WriteBlobByte(image,0xd3);
      (void) WriteBlobByte(image,'\n');
    }
  /*
    Write Catalog object.
  */
  xref[object++]=TellBlob(image);
  root_id=object;
  (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
    object);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"<<\n");
  if (is_pdfa == MagickFalse)
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Pages %.20g 0 R\n",
      (double) object+1);
  else
    {
      (void) FormatLocaleString(buffer,MagickPathExtent,"/Metadata %.20g 0 R\n",
        (double) object+1);
      (void) WriteBlobString(image,buffer);
      (void) FormatLocaleString(buffer,MagickPathExtent,"/Pages %.20g 0 R\n",
        (double) object+2);
    }
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"/Type /Catalog");
  option=GetImageOption(image_info,"pdf:page-direction");
  if ((option != (const char *) NULL) &&
      (LocaleCompare(option,"right-to-left") == 0))
    (void) WriteBlobString(image,"/ViewerPreferences<</PageDirection/R2L>>\n");
  (void) WriteBlobString(image,"\n");
  (void) WriteBlobString(image,">>\n");
  (void) WriteBlobString(image,"endobj\n");
  GetPathComponent(image->filename,BasePath,basename);
  if (is_pdfa != MagickFalse)
    {
      char
        create_date[MagickTimeExtent],
        *creator,
        *keywords,
        modify_date[MagickTimeExtent],
        *producer,
        timestamp[MagickTimeExtent],
        *title;

      /*
        Write XMP object.
      */
      xref[object++]=TellBlob(image);
      (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
        object);
      (void) WriteBlobString(image,buffer);
      (void) WriteBlobString(image,"<<\n");
      (void) WriteBlobString(image,"/Subtype /XML\n");
      (void) FormatMagickTime(GetPdfCreationDate(image_info,image),
        sizeof(create_date),create_date);
      (void) FormatMagickTime(GetPdfModDate(image_info,image),
        sizeof(modify_date),modify_date);
      (void) FormatMagickTime(GetMagickTime(),sizeof(timestamp),timestamp);
      creator=SubstituteXMLEntities(GetPDFCreator(image_info),MagickFalse);
      title=SubstituteXMLEntities(GetPDFTitle(image_info,basename),MagickFalse);
      producer=SubstituteXMLEntities(GetPDFProducer(image_info),MagickFalse);
      keywords=SubstituteXMLEntities(GetPDFKeywords(image_info),MagickFalse);
      i=FormatLocaleString(temp,MagickPathExtent,XMPProfile,XMPProfileMagick,
        create_date,modify_date,timestamp,creator,title,producer,keywords);
      producer=DestroyString(producer);
      title=DestroyString(title);
      creator=DestroyString(creator);
      keywords=DestroyString(keywords);
      (void) FormatLocaleString(buffer,MagickPathExtent,"/Length %.20g\n",
        (double) i);
      (void) WriteBlobString(image,buffer);
      (void) WriteBlobString(image,"/Type /Metadata\n");
      (void) WriteBlobString(image,">>\nstream\n");
      (void) WriteBlobString(image,temp);
      (void) WriteBlobString(image,"\nendstream\n");
      (void) WriteBlobString(image,"endobj\n");
    }
  /*
    Write Pages object.
  */
  xref[object++]=TellBlob(image);
  pages_id=object;
  (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
    object);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"<<\n");
  (void) WriteBlobString(image,"/Type /Pages\n");
  (void) FormatLocaleString(buffer,MagickPathExtent,"/Kids [ %.20g 0 R ",
    (double) object+1);
  (void) WriteBlobString(image,buffer);
  count=(ssize_t) (pages_id+ObjectsPerImage+1);
  page_count=1;
  if (image_info->adjoin != MagickFalse)
    {
      Image
        *kid_image;

      /*
        Predict page object id's.
      */
      kid_image=image;
      for ( ; GetNextImageInList(kid_image) != (Image *) NULL; count+=ObjectsPerImage)
      {
        page_count++;
        icc_profile=GetCompatibleColorProfile(kid_image);
        if (icc_profile != (StringInfo *) NULL)
          count+=2;
        (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 R ",(double)
          count);
        (void) WriteBlobString(image,buffer);
        kid_image=GetNextImageInList(kid_image);
      }
      xref=(MagickOffsetType *) ResizeQuantumMemory(xref,(size_t) count+2048UL,
        sizeof(*xref));
      if (xref == (MagickOffsetType *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) WriteBlobString(image,"]\n");
  (void) FormatLocaleString(buffer,MagickPathExtent,"/Count %.20g\n",(double)
    page_count);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,">>\n");
  (void) WriteBlobString(image,"endobj\n");
  scene=0;
  number_scenes=GetImageListLength(image);
  do
  {
    Image
      *tile_image;

    MagickBooleanType
      thumbnail;

    icc_profile=GetCompatibleColorProfile(image);
    compression=image->compression;
    if (image_info->compression != UndefinedCompression)
      compression=image_info->compression;
    switch (compression)
    {
      case FaxCompression:
      case Group4Compression:
      {
        if ((SetImageMonochrome(image,exception) == MagickFalse) ||
            (image->alpha_trait != UndefinedPixelTrait))
          compression=RLECompression;
        break;
      }
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
#if !defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
      case JPEG2000Compression:
      {
        compression=RLECompression;
        (void) ThrowMagickException(exception,GetMagickModule(),
          MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","`%s' (JP2)",
          image->filename);
        break;
      }
#endif
#if !defined(MAGICKCORE_ZLIB_DELEGATE)
      case ZipCompression:
      {
        compression=RLECompression;
        (void) ThrowMagickException(exception,GetMagickModule(),
          MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","`%s' (ZLIB)",
          image->filename);
        break;
      }
#endif
      case LZWCompression:
      {
        if (is_pdfa != MagickFalse)
          compression=RLECompression;  /* LZW compression is forbidden */
        break;
      }
      case NoCompression:
      {
        if (is_pdfa != MagickFalse)
          compression=RLECompression; /* ASCII 85 compression is forbidden */
        break;
      }
      default:
        break;
    }
    if (compression == JPEG2000Compression)
      if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
      (void) TransformImageColorspace(image,sRGBColorspace,exception);
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
        if ((flags & RhoValue) != 0)
          resolution.x=geometry_info.rho;
        resolution.y=resolution.x;
        if ((flags & SigmaValue) != 0)
          resolution.y=geometry_info.sigma;
      }
    if (image_info->density != (char *) NULL)
      {
        flags=ParseGeometry(image_info->density,&geometry_info);
        if ((flags & RhoValue) != 0)
          resolution.x=geometry_info.rho;
        resolution.y=resolution.x;
        if ((flags & SigmaValue) != 0)
          resolution.y=geometry_info.sigma;
      }
    if (image->units == PixelsPerCentimeterResolution)
      {
        resolution.x=(double) ((size_t) (100.0*2.54*resolution.x+0.5)/100.0);
        resolution.y=(double) ((size_t) (100.0*2.54*resolution.y+0.5)/100.0);
      }
    SetGeometry(image,&geometry);
    (void) FormatLocaleString(temp,MagickPathExtent,"%.20gx%.20g",
      (double) image->columns,(double) image->rows);
    if (image_info->page != (char *) NULL)
      (void) CopyMagickString(temp,image_info->page,MagickPathExtent);
    else
      if ((image->page.width != 0) && (image->page.height != 0))
        (void) FormatLocaleString(temp,MagickPathExtent,
          "%.20gx%.20g%+.20g%+.20g",(double) image->page.width,(double)
          image->page.height,(double) image->page.x,(double) image->page.y);
      else
        if ((image->gravity != UndefinedGravity) &&
            (LocaleCompare(image_info->magick,"PDF") == 0))
          (void) CopyMagickString(temp,PSPageGeometry,
            MagickPathExtent);
    (void) ConcatenateMagickString(temp,">",MagickPathExtent);
    (void) ParseMetaGeometry(temp,&geometry.x,&geometry.y,
      &geometry.width,&geometry.height);
    scale.x=(double) (geometry.width*delta.x)/resolution.x;
    geometry.width=CastDoubleToSizeT(scale.x+0.5);
    scale.y=(double) (geometry.height*delta.y)/resolution.y;
    geometry.height=CastDoubleToSizeT(scale.y+0.5);
    (void) ParseAbsoluteGeometry(temp,&media_info);
    (void) ParseGravityGeometry(image,temp,&page_info,exception);
    if (image->gravity != UndefinedGravity)
      {
        geometry.x=(-page_info.x);
        geometry.y=(ssize_t) media_info.height+page_info.y-(ssize_t)
          image->rows;
      }
    pointsize=12.0;
    if (image_info->pointsize != 0.0)
      pointsize=image_info->pointsize;
    text_size=0;
    value=GetImageProperty(image,"label",exception);
    if (value != (const char *) NULL)
      text_size=(size_t) (MultilineCensus(value)*pointsize+12);
    (void) text_size;
    /*
      Write Page object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    (void) WriteBlobString(image,"/Type /Page\n");
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Parent %.20g 0 R\n",
      (double) pages_id);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"/Resources <<\n");
    labels=(char **) NULL;
    value=GetImageProperty(image,"label",exception);
    if (value != (const char *) NULL)
      labels=StringToList(value);
    if (labels != (char **) NULL)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "/Font << /F%.20g %.20g 0 R >>\n",(double) image->scene,(double)
          object+4);
        (void) WriteBlobString(image,buffer);
      }
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "/XObject << /Im%.20g %.20g 0 R >>\n",(double) image->scene,(double)
      object+5);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/ProcSet %.20g 0 R >>\n",
      (double) object+3);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "/MediaBox [0 0 %g %g]\n",DefaultResolution*media_info.width*
      (double) MagickSafeReciprocal(resolution.x),(double) (
      DefaultResolution*media_info.height*MagickSafeReciprocal(resolution.y)));
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "/CropBox [0 0 %g %g]\n",DefaultResolution*media_info.width*(double)
      MagickSafeReciprocal(resolution.x),(double) (DefaultResolution*
      media_info.height*MagickSafeReciprocal(resolution.y)));
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Contents %.20g 0 R\n",
      (double) object+1);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Thumb %.20g 0 R\n",
      (double) object+(icc_profile != (StringInfo *) NULL ? 10 : 8));
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,">>\n");
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Contents object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Length %.20g 0 R\n",
      (double) object+1);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,">>\n");
    (void) WriteBlobString(image,"stream\n");
    offset=TellBlob(image);
    (void) WriteBlobString(image,"q\n");
    if (labels != (char **) NULL)
      for (i=0; labels[i] != (char *) NULL; i++)
      {
        (void) WriteBlobString(image,"BT\n");
        (void) FormatLocaleString(buffer,MagickPathExtent,"/F%.20g %g Tf\n",
          (double) image->scene,pointsize);
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g %.20g Td\n",
          (double) geometry.x,(double) (geometry.y+(ssize_t) geometry.height+
          i*pointsize+12));
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,"(%s) Tj\n",
           labels[i]);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,"ET\n");
        labels[i]=DestroyString(labels[i]);
      }
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "%g 0 0 %g %.20g %.20g cm\n",scale.x,scale.y,(double) geometry.x,
      (double) geometry.y);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Im%.20g Do\n",(double)
      image->scene);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"Q\n");
    offset=TellBlob(image)-offset;
    (void) WriteBlobString(image,"\nendstream\n");
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Length object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double)
      offset);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Procset object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    if ((compression == FaxCompression) || (compression == Group4Compression))
      (void) CopyMagickString(buffer,"[ /PDF /Text /ImageB",MagickPathExtent);
    else
      if ((image->storage_class == DirectClass) || (image->colors > 256))
        (void) CopyMagickString(buffer,"[ /PDF /Text /ImageC",MagickPathExtent);
      else
        (void) CopyMagickString(buffer,"[ /PDF /Text /ImageI",MagickPathExtent);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image," ]\n");
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Font object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    if (labels != (char **) NULL)
      {
        (void) WriteBlobString(image,"/Type /Font\n");
        (void) WriteBlobString(image,"/Subtype /Type1\n");
        (void) FormatLocaleString(buffer,MagickPathExtent,"/Name /F%.20g\n",
          (double) image->scene);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,"/BaseFont /Helvetica\n");
        (void) WriteBlobString(image,"/Encoding /MacRomanEncoding\n");
        labels=(char **) RelinquishMagickMemory(labels);
      }
    (void) WriteBlobString(image,">>\n");
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write XObject object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    (void) WriteBlobString(image,"/Type /XObject\n");
    (void) WriteBlobString(image,"/Subtype /Image\n");
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Name /Im%.20g\n",
      (double) image->scene);
    (void) WriteBlobString(image,buffer);
    switch (compression)
    {
      case NoCompression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,
          "ASCII85Decode");
        break;
      }
      case JPEGCompression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,"DCTDecode");
        if (image->colorspace != CMYKColorspace)
          break;
        (void) WriteBlobString(image,buffer);
        (void) CopyMagickString(buffer,"/Decode [1 0 1 0 1 0 1 0]\n",
          MagickPathExtent);
        break;
      }
      case JPEG2000Compression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,"JPXDecode");
        if (image->colorspace != CMYKColorspace)
          break;
        (void) WriteBlobString(image,buffer);
        (void) CopyMagickString(buffer,"/Decode [1 0 1 0 1 0 1 0]\n",
          MagickPathExtent);
        break;
      }
      case LZWCompression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,"LZWDecode");
        break;
      }
      case ZipCompression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,
          "FlateDecode");
        break;
      }
      case FaxCompression:
      case Group4Compression:
      {
        (void) CopyMagickString(buffer,"/Filter [ /CCITTFaxDecode ]\n",
          MagickPathExtent);
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,"/DecodeParms [ << "
          "/K %s /BlackIs1 false /Columns %.20g /Rows %.20g >> ]\n",CCITTParam,
          (double) image->columns,(double) image->rows);
        break;
      }
      default:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,
          "RunLengthDecode");
        break;
      }
    }
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Width %.20g\n",(double)
      image->columns);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Height %.20g\n",(double)
      image->rows);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/ColorSpace %.20g 0 R\n",
      (double) object+2);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/BitsPerComponent %d\n",
      (compression == FaxCompression) || (compression == Group4Compression) ?
      1 : 8);
    (void) WriteBlobString(image,buffer);
    if (image->alpha_trait != UndefinedPixelTrait)
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,"/SMask %.20g 0 R\n",
          (double) object+(icc_profile != (StringInfo *) NULL ? 9 : 7));
        (void) WriteBlobString(image,buffer);
      }
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Length %.20g 0 R\n",
      (double) object+1);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,">>\n");
    (void) WriteBlobString(image,"stream\n");
    offset=TellBlob(image);
    number_pixels=(MagickSizeType) image->columns*image->rows;
    if ((4*number_pixels) != (MagickSizeType) ((size_t) (4*number_pixels)))
      ThrowPDFException(ResourceLimitError,"MemoryAllocationFailed");
    if ((compression == FaxCompression) || (compression == Group4Compression) ||
        (IsImageGray(image) != MagickFalse))
      {
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
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
                (void) CloseBlob(image);
                return(MagickFalse);
              }
            break;
          }
          case JPEG2000Compression:
          {
            status=InjectImageBlob(image_info,image,image,"jp2",exception);
            if (status == MagickFalse)
              {
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
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

            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            pixel_info=AcquireVirtualMemory(length,sizeof(*pixels));
            if (pixel_info == (MemoryInfo *) NULL)
              ThrowPDFException(ResourceLimitError,"MemoryAllocationFailed");
            pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
            /*
              Dump Runlength encoded pixels.
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
                p+=(ptrdiff_t) GetPixelChannels(image);
              }
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (compression == ZipCompression)
              status=ZLIBEncodeImage(image,length,pixels,exception);
            else
#endif
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels,exception);
              else
                status=PackbitsEncodeImage(image,length,pixels,exception);
            pixel_info=RelinquishVirtualMemory(pixel_info);
            if (status == MagickFalse)
              {
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
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
                p+=(ptrdiff_t) GetPixelChannels(image);
              }
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            Ascii85Flush(image);
            break;
          }
        }
      }
    else
      if ((image->storage_class == DirectClass) || (image->colors > 256) ||
          (compression == JPEGCompression) ||
          (compression == JPEG2000Compression))
        switch (compression)
        {
          case JPEGCompression:
          {
            status=InjectImageBlob(image_info,image,image,"jpeg",exception);
            if (status == MagickFalse)
              {
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
                (void) CloseBlob(image);
                return(MagickFalse);
              }
            break;
          }
          case JPEG2000Compression:
          {
            status=InjectImageBlob(image_info,image,image,"jp2",exception);
            if (status == MagickFalse)
              {
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
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

            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            length*=image->colorspace == CMYKColorspace ? 4UL : 3UL;
            pixel_info=AcquireVirtualMemory(length,sizeof(*pixels));
            if (pixel_info == (MemoryInfo *) NULL)
              ThrowPDFException(ResourceLimitError,"MemoryAllocationFailed");
            pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
            /*
              Dump runoffset encoded pixels.
            */
            q=pixels;
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                *q++=ScaleQuantumToChar(GetPixelRed(image,p));
                *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
                *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
                if (image->colorspace == CMYKColorspace)
                  *q++=ScaleQuantumToChar(GetPixelBlack(image,p));
                p+=(ptrdiff_t) GetPixelChannels(image);
              }
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (compression == ZipCompression)
              status=ZLIBEncodeImage(image,length,pixels,exception);
            else
#endif
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels,exception);
              else
                status=PackbitsEncodeImage(image,length,pixels,exception);
            pixel_info=RelinquishVirtualMemory(pixel_info);
            if (status == MagickFalse)
              {
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
                (void) CloseBlob(image);
                return(MagickFalse);
              }
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
                Ascii85Encode(image,ScaleQuantumToChar(GetPixelRed(image,p)));
                Ascii85Encode(image,ScaleQuantumToChar(GetPixelGreen(image,p)));
                Ascii85Encode(image,ScaleQuantumToChar(GetPixelBlue(image,p)));
                if (image->colorspace == CMYKColorspace)
                  Ascii85Encode(image,ScaleQuantumToChar(
                    GetPixelBlack(image,p)));
                p+=(ptrdiff_t) GetPixelChannels(image);
              }
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            Ascii85Flush(image);
            break;
          }
        }
      else
        {
          /*
            Dump number of colors and colormap.
          */
          switch (compression)
          {
            case RLECompression:
            default:
            {
              MemoryInfo
                *pixel_info;

              /*
                Allocate pixel array.
              */
              length=(size_t) number_pixels;
              pixel_info=AcquireVirtualMemory(length,sizeof(*pixels));
              if (pixel_info == (MemoryInfo *) NULL)
                ThrowPDFException(ResourceLimitError,"MemoryAllocationFailed");
              pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
              /*
                Dump Runlength encoded pixels.
              */
              q=pixels;
              for (y=0; y < (ssize_t) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                for (x=0; x < (ssize_t) image->columns; x++)
                {
                  *q++=(unsigned char) ((ssize_t) GetPixelIndex(image,p));
                  p+=(ptrdiff_t) GetPixelChannels(image);
                }
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,SaveImageTag,
                      (MagickOffsetType) y,image->rows);
                    if (status == MagickFalse)
                      break;
                  }
              }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
              if (compression == ZipCompression)
                status=ZLIBEncodeImage(image,length,pixels,exception);
              else
#endif
                if (compression == LZWCompression)
                  status=LZWEncodeImage(image,length,pixels,exception);
                else
                  status=PackbitsEncodeImage(image,length,pixels,exception);
              pixel_info=RelinquishVirtualMemory(pixel_info);
              if (status == MagickFalse)
                {
                  xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
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
                  Ascii85Encode(image,(unsigned char) ((ssize_t)
                    GetPixelIndex(image,p)));
                  p+=(ptrdiff_t) GetPixelChannels(image);
                }
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,SaveImageTag,
                      (MagickOffsetType) y,image->rows);
                    if (status == MagickFalse)
                      break;
                  }
              }
              Ascii85Flush(image);
              break;
            }
          }
        }
    offset=TellBlob(image)-offset;
    (void) WriteBlobString(image,"\nendstream\n");
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Length object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double)
      offset);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Colorspace object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    device="DeviceRGB";
    channels=0;
    if (image->colorspace == CMYKColorspace)
      {
        device="DeviceCMYK";
        channels=4;
      }
    else
      if ((compression == FaxCompression) ||
          (compression == Group4Compression) ||
          (IsImageGray(image) != MagickFalse))
        {
          device="DeviceGray";
          channels=1;
        }
      else
        if ((image->storage_class == DirectClass) ||
            (image->colors > 256) || (compression == JPEGCompression) ||
            (compression == JPEG2000Compression))
          channels=3;
    if (icc_profile == (StringInfo *) NULL)
      {
        if (channels != 0)
          (void) FormatLocaleString(buffer,MagickPathExtent,"/%s\n",device);
        else
          (void) FormatLocaleString(buffer,MagickPathExtent,
            "[ /Indexed /%s %.20g %.20g 0 R ]\n",device,(double) image->colors-1,
            (double) object+3);
        (void) WriteBlobString(image,buffer);
      }
    else
      {
        const unsigned char
          *r;

        /*
          Write ICC profile.
        */
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "[/ICCBased %.20g 0 R]\n",(double) object+1);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,"endobj\n");
        xref[object++]=TellBlob(image);
        (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",
          (double) object);
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,"<<\n/N %.20g\n"
          "/Filter /ASCII85Decode\n/Length %.20g 0 R\n/Alternate /%s\n>>\n"
          "stream\n",(double) channels,(double) object+1,device);
        (void) WriteBlobString(image,buffer);
        offset=TellBlob(image);
        Ascii85Initialize(image);
        r=GetStringInfoDatum(icc_profile);
        for (i=0; i < (ssize_t) GetStringInfoLength(icc_profile); i++)
          Ascii85Encode(image,(unsigned char) *r++);
        Ascii85Flush(image);
        offset=TellBlob(image)-offset-1;
        (void) WriteBlobString(image,"endstream\n");
        (void) WriteBlobString(image,"endobj\n");
        /*
          Write Length object.
        */
        xref[object++]=TellBlob(image);
        (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",
          (double) object);
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double)
          offset);
        (void) WriteBlobString(image,buffer);
      }
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Thumb object.
    */
    SetGeometry(image,&geometry);
    thumbnail=IsStringTrue(GetImageOption(image_info,"pdf:thumbnail"));
    if (thumbnail == MagickFalse)
      (void) ParseMetaGeometry("1x1+0+0>",&geometry.x,&geometry.y,
        &geometry.width,&geometry.height);
    else
      (void) ParseMetaGeometry("106x106+0+0>",&geometry.x,&geometry.y,
        &geometry.width,&geometry.height);
    tile_image=ThumbnailImage(image,geometry.width,geometry.height,exception);
    if (tile_image == (Image *) NULL)
      {
        xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
        (void) CloseBlob(image);
        return(MagickFalse);
      }
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    switch (compression)
    {
      case NoCompression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,
          "ASCII85Decode");
        break;
      }
      case JPEGCompression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,"DCTDecode");
        if (tile_image->colorspace != CMYKColorspace)
          break;
        (void) WriteBlobString(image,buffer);
        (void) CopyMagickString(buffer,"/Decode [1 0 1 0 1 0 1 0]\n",
          MagickPathExtent);
        break;
      }
      case JPEG2000Compression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,"JPXDecode");
        if (tile_image->colorspace != CMYKColorspace)
          break;
        (void) WriteBlobString(image,buffer);
        (void) CopyMagickString(buffer,"/Decode [1 0 1 0 1 0 1 0]\n",
          MagickPathExtent);
        break;
      }
      case LZWCompression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,"LZWDecode");
        break;
      }
      case ZipCompression:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,
          "FlateDecode");
        break;
      }
      case FaxCompression:
      case Group4Compression:
      {
        (void) CopyMagickString(buffer,"/Filter [ /CCITTFaxDecode ]\n",
          MagickPathExtent);
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,"/DecodeParms [ "
          "<< /K %s /BlackIs1 false /Columns %.20g /Rows %.20g >> ]\n",
          CCITTParam,(double) tile_image->columns,(double) tile_image->rows);
        break;
      }
      default:
      {
        (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,
          "RunLengthDecode");
        break;
      }
    }
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Width %.20g\n",(double)
      tile_image->columns);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Height %.20g\n",(double)
      tile_image->rows);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "/ColorSpace %.20g 0 R\n",(double) object-
      (icc_profile != (StringInfo *) NULL ? 3 : 1));
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,
      "/BitsPerComponent %d\n",(compression == FaxCompression) ||
      (compression == Group4Compression) ? 1 : 8);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"/Length %.20g 0 R\n",
      (double) object+1);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,">>\n");
    (void) WriteBlobString(image,"stream\n");
    offset=TellBlob(image);
    number_pixels=(MagickSizeType) tile_image->columns*tile_image->rows;
    if ((compression == FaxCompression) ||
        (compression == Group4Compression) ||
        (SetImageCoderGray(tile_image,exception) != MagickFalse))
      {
        switch (compression)
        {
          case FaxCompression:
          case Group4Compression:
          {
            if (LocaleCompare(CCITTParam,"0") == 0)
              {
                (void) HuffmanEncodeImage(image_info,image,tile_image,
                  exception);
                break;
              }
            (void) Huffman2DEncodeImage(image_info,image,tile_image,exception);
            break;
          }
          case JPEGCompression:
          {
            status=InjectImageBlob(image_info,image,tile_image,"jpeg",
              exception);
            if (status == MagickFalse)
              {
                tile_image=DestroyImage(tile_image);
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
                (void) CloseBlob(image);
                return(MagickFalse);
              }
            break;
          }
          case JPEG2000Compression:
          {
            status=InjectImageBlob(image_info,image,tile_image,"jp2",exception);
            if (status == MagickFalse)
              {
                tile_image=DestroyImage(tile_image);
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
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

            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            pixel_info=AcquireVirtualMemory(length,sizeof(*pixels));
            if (pixel_info == (MemoryInfo *) NULL)
              {
                tile_image=DestroyImage(tile_image);
                ThrowPDFException(ResourceLimitError,"MemoryAllocationFailed");
              }
            pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
            /*
              Dump runlength encoded pixels.
            */
            q=pixels;
            for (y=0; y < (ssize_t) tile_image->rows; y++)
            {
              p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                exception);
              if (p == (const Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) tile_image->columns; x++)
              {
                *q++=ScaleQuantumToChar(ClampToQuantum(GetPixelLuma(tile_image,
                  p)));
                p+=(ptrdiff_t) GetPixelChannels(tile_image);
              }
            }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (compression == ZipCompression)
              status=ZLIBEncodeImage(image,length,pixels,exception);
            else
#endif
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels,exception);
              else
                status=PackbitsEncodeImage(image,length,pixels,exception);
            pixel_info=RelinquishVirtualMemory(pixel_info);
            if (status == MagickFalse)
              {
                tile_image=DestroyImage(tile_image);
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
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
            for (y=0; y < (ssize_t) tile_image->rows; y++)
            {
              p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                exception);
              if (p == (const Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) tile_image->columns; x++)
              {
                Ascii85Encode(image,ScaleQuantumToChar(ClampToQuantum(
                  GetPixelLuma(tile_image,p))));
                p+=(ptrdiff_t) GetPixelChannels(tile_image);
              }
            }
            Ascii85Flush(image);
            break;
          }
        }
      }
    else
      if ((tile_image->storage_class == DirectClass) ||
          (tile_image->colors > 256) || (compression == JPEGCompression) ||
          (compression == JPEG2000Compression))
        switch (compression)
        {
          case JPEGCompression:
          {
            status=InjectImageBlob(image_info,image,tile_image,"jpeg",
              exception);
            if (status == MagickFalse)
              {
                tile_image=DestroyImage(tile_image);
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
                (void) CloseBlob(image);
                return(MagickFalse);
              }
            break;
          }
          case JPEG2000Compression:
          {
            status=InjectImageBlob(image_info,image,tile_image,"jp2",exception);
            if (status == MagickFalse)
              {
                tile_image=DestroyImage(tile_image);
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
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

            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            length*=tile_image->colorspace == CMYKColorspace ? 4UL : 3UL;
            pixel_info=AcquireVirtualMemory(length,4*sizeof(*pixels));
            if (pixel_info == (MemoryInfo *) NULL)
              {
                tile_image=DestroyImage(tile_image);
                ThrowPDFException(ResourceLimitError,"MemoryAllocationFailed");
              }
            pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
            /*
              Dump runlength encoded pixels.
            */
            q=pixels;
            for (y=0; y < (ssize_t) tile_image->rows; y++)
            {
              p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                exception);
              if (p == (const Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) tile_image->columns; x++)
              {
                *q++=ScaleQuantumToChar(GetPixelRed(tile_image,p));
                *q++=ScaleQuantumToChar(GetPixelGreen(tile_image,p));
                *q++=ScaleQuantumToChar(GetPixelBlue(tile_image,p));
                if (tile_image->colorspace == CMYKColorspace)
                  *q++=ScaleQuantumToChar(GetPixelBlack(tile_image,p));
                p+=(ptrdiff_t) GetPixelChannels(tile_image);
              }
            }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (compression == ZipCompression)
              status=ZLIBEncodeImage(image,length,pixels,exception);
            else
#endif
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels,exception);
              else
                status=PackbitsEncodeImage(image,length,pixels,exception);
            pixel_info=RelinquishVirtualMemory(pixel_info);
            if (status == MagickFalse)
              {
                tile_image=DestroyImage(tile_image);
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
                (void) CloseBlob(image);
                return(MagickFalse);
              }
            break;
          }
          case NoCompression:
          {
            /*
              Dump uncompressed DirectColor packets.
            */
            Ascii85Initialize(image);
            for (y=0; y < (ssize_t) tile_image->rows; y++)
            {
              p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                exception);
              if (p == (const Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) tile_image->columns; x++)
              {
                Ascii85Encode(image,ScaleQuantumToChar(
                  GetPixelRed(tile_image,p)));
                Ascii85Encode(image,ScaleQuantumToChar(
                  GetPixelGreen(tile_image,p)));
                Ascii85Encode(image,ScaleQuantumToChar(
                  GetPixelBlue(tile_image,p)));
                if (image->colorspace == CMYKColorspace)
                  Ascii85Encode(image,ScaleQuantumToChar(
                    GetPixelBlack(tile_image,p)));
                p+=(ptrdiff_t) GetPixelChannels(tile_image);
              }
            }
            Ascii85Flush(image);
            break;
          }
        }
      else
        {
          /*
            Dump number of colors and colormap.
          */
          switch (compression)
          {
            case RLECompression:
            default:
            {
              MemoryInfo
                *pixel_info;

              /*
                Allocate pixel array.
              */
              length=(size_t) number_pixels;
              pixel_info=AcquireVirtualMemory(length,sizeof(*pixels));
              if (pixel_info == (MemoryInfo *) NULL)
                {
                  tile_image=DestroyImage(tile_image);
                  ThrowPDFException(ResourceLimitError,
                    "MemoryAllocationFailed");
                }
              pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
              /*
                Dump runlength encoded pixels.
              */
              q=pixels;
              for (y=0; y < (ssize_t) tile_image->rows; y++)
              {
                p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                  exception);
                if (p == (const Quantum *) NULL)
                  break;
                for (x=0; x < (ssize_t) tile_image->columns; x++)
                {
                  *q++=(unsigned char) ((ssize_t) GetPixelIndex(tile_image,p));
                  p+=(ptrdiff_t) GetPixelChannels(tile_image);
                }
              }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
              if (compression == ZipCompression)
                status=ZLIBEncodeImage(image,length,pixels,exception);
              else
#endif
                if (compression == LZWCompression)
                  status=LZWEncodeImage(image,length,pixels,exception);
                else
                  status=PackbitsEncodeImage(image,length,pixels,exception);
              pixel_info=RelinquishVirtualMemory(pixel_info);
              if (status == MagickFalse)
                {
                  tile_image=DestroyImage(tile_image);
                  xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
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
              for (y=0; y < (ssize_t) tile_image->rows; y++)
              {
                p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                  exception);
                if (p == (const Quantum *) NULL)
                  break;
                for (x=0; x < (ssize_t) tile_image->columns; x++)
                {
                  Ascii85Encode(image,(unsigned char) ((ssize_t)
                    GetPixelIndex(tile_image,p)));
                  p+=(ptrdiff_t) GetPixelChannels(image);
                }
              }
              Ascii85Flush(image);
              break;
            }
          }
        }
    tile_image=DestroyImage(tile_image);
    offset=TellBlob(image)-offset;
    (void) WriteBlobString(image,"\nendstream\n");
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Length object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double)
      offset);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    if ((image->storage_class == DirectClass) || (image->colors > 256) ||
        (compression == FaxCompression) || (compression == Group4Compression))
      (void) WriteBlobString(image,">>\n");
    else
      {
        /*
          Write Colormap object.
        */
        if (compression == NoCompression)
          (void) WriteBlobString(image,"/Filter [ /ASCII85Decode ]\n");
        (void) FormatLocaleString(buffer,MagickPathExtent,"/Length %.20g 0 R\n",
          (double) object+1);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,">>\n");
        (void) WriteBlobString(image,"stream\n");
        offset=TellBlob(image);
        if (compression == NoCompression)
          Ascii85Initialize(image);
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          if (compression == NoCompression)
            {
              Ascii85Encode(image,ScaleQuantumToChar(ClampToQuantum(
                image->colormap[i].red)));
              Ascii85Encode(image,ScaleQuantumToChar(ClampToQuantum(
                image->colormap[i].green)));
              Ascii85Encode(image,ScaleQuantumToChar(ClampToQuantum(
                image->colormap[i].blue)));
              continue;
            }
          (void) WriteBlobByte(image,ScaleQuantumToChar(
            ClampToQuantum(image->colormap[i].red)));
          (void) WriteBlobByte(image,ScaleQuantumToChar(
            ClampToQuantum(image->colormap[i].green)));
          (void) WriteBlobByte(image,ScaleQuantumToChar(
            ClampToQuantum(image->colormap[i].blue)));
        }
        if (compression == NoCompression)
          Ascii85Flush(image);
       offset=TellBlob(image)-offset;
       (void) WriteBlobString(image,"\nendstream\n");
      }
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Length object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double)
      offset);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write softmask object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    if ((image->alpha_trait & BlendPixelTrait) == 0)
      (void) WriteBlobString(image,">>\n");
    else
      {
        (void) WriteBlobString(image,"/Type /XObject\n");
        (void) WriteBlobString(image,"/Subtype /Image\n");
        (void) FormatLocaleString(buffer,MagickPathExtent,"/Name /Ma%.20g\n",
          (double) image->scene);
        (void) WriteBlobString(image,buffer);
        switch (compression)
        {
          case NoCompression:
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,
              "ASCII85Decode");
            break;
          }
          case LZWCompression:
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,
              "LZWDecode");
            break;
          }
          case ZipCompression:
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,
              "FlateDecode");
            break;
          }
          default:
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,CFormat,
              "RunLengthDecode");
            break;
          }
        }
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,"/Width %.20g\n",
          (double) image->columns);
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,"/Height %.20g\n",
          (double) image->rows);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,"/ColorSpace /DeviceGray\n");
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "/BitsPerComponent %d\n",(compression == FaxCompression) ||
          (compression == Group4Compression) ? 1 : 8);
        (void) WriteBlobString(image,buffer);
        (void) FormatLocaleString(buffer,MagickPathExtent,"/Length %.20g 0 R\n",
          (double) object+1);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,">>\n");
        (void) WriteBlobString(image,"stream\n");
        offset=TellBlob(image);
        number_pixels=(MagickSizeType) image->columns*image->rows;
        switch (compression)
        {
          case RLECompression:
          default:
          {
            MemoryInfo
              *pixel_info;

            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            pixel_info=AcquireVirtualMemory(length,4*sizeof(*pixels));
            if (pixel_info == (MemoryInfo *) NULL)
              {
                image=DestroyImage(image);
                ThrowPDFException(ResourceLimitError,"MemoryAllocationFailed");
              }
            pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
            /*
              Dump Runlength encoded pixels.
            */
            q=pixels;
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                *q++=ScaleQuantumToChar(GetPixelAlpha(image,p));
                p+=(ptrdiff_t) GetPixelChannels(image);
              }
            }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (compression == ZipCompression)
              status=ZLIBEncodeImage(image,length,pixels,exception);
            else
#endif
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels,exception);
              else
                status=PackbitsEncodeImage(image,length,pixels,exception);
            pixel_info=RelinquishVirtualMemory(pixel_info);
            if (status == MagickFalse)
              {
                xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
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
                Ascii85Encode(image,ScaleQuantumToChar(GetPixelAlpha(image,p)));
                p+=(ptrdiff_t) GetPixelChannels(image);
              }
            }
            Ascii85Flush(image);
            break;
          }
        }
        offset=TellBlob(image)-offset;
        (void) WriteBlobString(image,"\nendstream\n");
      }
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Length object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
      object);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double)
      offset);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,number_scenes);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  /*
    Write Metadata object.
  */
  xref[object++]=TellBlob(image);
  info_id=object;
  (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g 0 obj\n",(double)
    object);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"<<\n");
  WritePDFValue(image,"Title",GetPDFTitle(image_info,basename),is_pdfa);
  WritePDFValue(image,"Author",GetPDFAuthor(image_info),is_pdfa);
  WritePDFValue(image,"Creator",GetPDFCreator(image_info),is_pdfa);
  WritePDFValue(image,"Producer",GetPDFProducer(image_info),is_pdfa);
  WritePDFValue(image,"Subject",GetPDFSubject(image_info),is_pdfa);
  WritePDFValue(image,"Keywords",GetPDFKeywords(image_info),is_pdfa);
  seconds=GetPdfCreationDate(image_info,image);
  GetMagickUTCTime(&seconds,&utc_time);
  (void) FormatLocaleString(temp,MagickPathExtent,"D:%04d%02d%02d%02d%02d%02d",
    utc_time.tm_year+1900,utc_time.tm_mon+1,utc_time.tm_mday,
    utc_time.tm_hour,utc_time.tm_min,utc_time.tm_sec);
  (void) FormatLocaleString(buffer,MagickPathExtent,"/CreationDate (%s)\n",
    temp);
  (void) WriteBlobString(image,buffer);
  seconds=GetPdfModDate(image_info,image);
  GetMagickUTCTime(&seconds,&utc_time);
  (void) FormatLocaleString(temp,MagickPathExtent,"D:%04d%02d%02d%02d%02d%02d",
    utc_time.tm_year+1900,utc_time.tm_mon+1,utc_time.tm_mday,
    utc_time.tm_hour,utc_time.tm_min,utc_time.tm_sec);
  (void) FormatLocaleString(buffer,MagickPathExtent,"/ModDate (%s)\n",temp);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,">>\n");
  (void) WriteBlobString(image,"endobj\n");
  /*
    Write Xref object.
  */
  offset=TellBlob(image)-xref[0]+((is_pdfa != MagickFalse) ? 6 : 0)+9;
  (void) WriteBlobString(image,"xref\n");
  (void) FormatLocaleString(buffer,MagickPathExtent,"0 %.20g\n",(double)
    object+1);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"0000000000 65535 f \n");
  for (i=0; i < (ssize_t) object; i++)
  {
    (void) FormatLocaleString(buffer,MagickPathExtent,"%010lu 00000 n \n",
      (unsigned long) xref[i]);
    (void) WriteBlobString(image,buffer);
  }
  (void) WriteBlobString(image,"trailer\n");
  (void) WriteBlobString(image,"<<\n");
  (void) FormatLocaleString(buffer,MagickPathExtent,"/Size %.20g\n",(double)
    object+1);
  (void) WriteBlobString(image,buffer);
  (void) FormatLocaleString(buffer,MagickPathExtent,"/Info %.20g 0 R\n",(double)
    info_id);
  (void) WriteBlobString(image,buffer);
  (void) FormatLocaleString(buffer,MagickPathExtent,"/Root %.20g 0 R\n",(double)
    root_id);
  (void) WriteBlobString(image,buffer);
  option=GetImageOption(image_info,"pdf:no-identifier");
  if ((IsStringFalse(option) != MagickFalse) || (is_pdfa != MagickFalse))
    {
      (void) SignatureImage(image,exception);
      (void) FormatLocaleString(buffer,MagickPathExtent,"/ID [<%s> <%s>]\n",
        GetImageProperty(image,"signature",exception),
        GetImageProperty(image,"signature",exception));
      (void) WriteBlobString(image,buffer);
    }
  (void) WriteBlobString(image,">>\n");
  (void) WriteBlobString(image,"startxref\n");
  (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double) offset);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"%%EOF\n");
  xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

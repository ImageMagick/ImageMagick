/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                               PPPP   SSSSS                                  %
%                               P   P  SS                                     %
%                               PPPP    SSS                                   %
%                               P         SS                                  %
%                               P      SSSSS                                  %
%                                                                             %
%                                                                             %
%                         Read/Write Postscript Format                        %
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
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
#include "MagickCore/delegate-private.h"
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
#include "MagickCore/nt-base-private.h"
#include "MagickCore/option.h"
#include "MagickCore/profile.h"
#include "MagickCore/resource_.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/timer-private.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "coders/bytebuffer-private.h"
#include "coders/coders-private.h"
#include "coders/ghostscript-private.h"

/*
  Typedef declarations.
*/
typedef struct _PSInfo
{
  MagickBooleanType
    cmyk;

  SegmentInfo
    bounds;

  unsigned long
    columns,
    rows;

  StringInfo
    *icc_profile,
    *photoshop_profile,
    *xmp_profile;

} PSInfo;

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePSImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P S                                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPS() returns MagickTrue if the image format type, identified by the
%  magick string, is PS.
%
%  The format of the IsPS method is:
%
%      MagickBooleanType IsPS(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsPS(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"%!",2) == 0)
    return(MagickTrue);
  if (memcmp(magick,"\004%!",3) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P S I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPSImage() reads a Postscript image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer
%  to the new image.
%
%  The format of the ReadPSImage method is:
%
%      Image *ReadPSImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline int ProfileInteger(MagickByteBuffer *buffer,short int *hex_digits)
{
  int
    c,
    l,
    value;

  ssize_t
    i;

  l=0;
  value=0;
  for (i=0; i < 2; )
  {
    c=ReadMagickByteBuffer(buffer);
    if ((c == EOF) || ((c == '%') && (l == '%')))
      {
        value=(-1);
        break;
      }
    l=c;
    c&=0xff;
    if (isxdigit(c) == MagickFalse)
      continue;
    value=(int) ((size_t) value << 4)+hex_digits[c];
    i++;
  }
  return(value);
}

static void ReadPSInfo(const ImageInfo *image_info,Image *image,PSInfo *ps_info,
  ExceptionInfo *exception)
{
#define BeginDocument  "BeginDocument:"
#define EndDocument  "EndDocument:"
#define PostscriptLevel  "PS-"
#define ImageData  "ImageData:"
#define DocumentProcessColors  "DocumentProcessColors:"
#define CMYKCustomColor  "CMYKCustomColor:"
#define CMYKProcessColor  "CMYKProcessColor:"
#define DocumentCustomColors  "DocumentCustomColors:"
#define SpotColor  "+ "
#define BoundingBox  "BoundingBox:"
#define DocumentMedia  "DocumentMedia:"
#define HiResBoundingBox  "HiResBoundingBox:"
#define PageBoundingBox  "PageBoundingBox:"
#define PageMedia  "PageMedia:"
#define ICCProfile "BeginICCProfile:"
#define PhotoshopProfile  "BeginPhotoshop:"

  char
    version[MagickPathExtent];

  int
    c;

  MagickBooleanType
    new_line,
    skip,
    spot_color;

  MagickByteBuffer
    buffer;

  char
    *p;

  ssize_t
    i;

  SegmentInfo
    bounds;

  size_t
    length;

  ssize_t
    count,
    priority;

  short int
    hex_digits[256];

  unsigned long
    spotcolor;

  (void) memset(&bounds,0,sizeof(bounds));
  (void) memset(ps_info,0,sizeof(*ps_info));
  ps_info->cmyk=image_info->colorspace == CMYKColorspace ? MagickTrue :
    MagickFalse;
  /*
    Initialize hex values.
  */
  (void) memset(hex_digits,0,sizeof(hex_digits));
  hex_digits[(int) '0']=0;
  hex_digits[(int) '1']=1;
  hex_digits[(int) '2']=2;
  hex_digits[(int) '3']=3;
  hex_digits[(int) '4']=4;
  hex_digits[(int) '5']=5;
  hex_digits[(int) '6']=6;
  hex_digits[(int) '7']=7;
  hex_digits[(int) '8']=8;
  hex_digits[(int) '9']=9;
  hex_digits[(int) 'a']=10;
  hex_digits[(int) 'b']=11;
  hex_digits[(int) 'c']=12;
  hex_digits[(int) 'd']=13;
  hex_digits[(int) 'e']=14;
  hex_digits[(int) 'f']=15;
  hex_digits[(int) 'A']=10;
  hex_digits[(int) 'B']=11;
  hex_digits[(int) 'C']=12;
  hex_digits[(int) 'D']=13;
  hex_digits[(int) 'E']=14;
  hex_digits[(int) 'F']=15;
  priority=0;
  *version='\0';
  spotcolor=0;
  skip=MagickFalse;
  new_line=MagickTrue;
  (void) memset(&buffer,0,sizeof(buffer));
  buffer.image=image;
  for (c=ReadMagickByteBuffer(&buffer); c != EOF; c=ReadMagickByteBuffer(&buffer))
  {
    switch(c)
    {
      case '<':
      {
        ReadGhostScriptXMPProfile(&buffer,&ps_info->xmp_profile,exception);
        continue;
      }
      case '\n':
      case '\r':
        new_line=MagickTrue;
        continue;
      case '%':
      {
        if (new_line == MagickFalse)
          continue;
        new_line=MagickFalse;
        c=ReadMagickByteBuffer(&buffer);
        if ((c == '%') || (c == '!'))
          break;
        if (c == 'B')
          {
            buffer.offset--;
            break;
          }
        continue;
      }
      default:
        continue;
    }
    /*
      Skip %%BeginDocument thru %%EndDocument.
    */
    if (CompareMagickByteBuffer(&buffer,BeginDocument,strlen(BeginDocument)) != MagickFalse)
      skip=MagickTrue;
    if (CompareMagickByteBuffer(&buffer,EndDocument,strlen(EndDocument)) != MagickFalse)
      skip=MagickFalse;
    if (skip != MagickFalse)
      continue;
    if ((*version == '\0') &&
        (CompareMagickByteBuffer(&buffer,PostscriptLevel,strlen(PostscriptLevel)) != MagickFalse))
      {
        i=0;
        for (c=ReadMagickByteBuffer(&buffer); c != EOF; c=ReadMagickByteBuffer(&buffer))
        {
          if ((c == '\r') || (c == '\n') ||
              ((i+1) == (ssize_t) sizeof(version)))
            {
              new_line=MagickTrue;
              break;
            }
          version[i++]=(char) c;
        }
        version[i]='\0';
      }
    if (CompareMagickByteBuffer(&buffer,ImageData,strlen(ImageData)) != MagickFalse)
      {
        p=GetMagickByteBufferDatum(&buffer);
        (void) MagickSscanf(p,ImageData " %lu %lu",&ps_info->columns,&ps_info->rows);
      }
    /*
      Is this a CMYK document?
    */
    length=strlen(DocumentProcessColors);
    if (CompareMagickByteBuffer(&buffer,DocumentProcessColors,length) != MagickFalse)
      {
        p=GetMagickByteBufferDatum(&buffer);
        if ((StringLocateSubstring(p,"Cyan") != (char *) NULL) ||
            (StringLocateSubstring(p,"Magenta") != (char *) NULL) ||
            (StringLocateSubstring(p,"Yellow") != (char *) NULL))
          ps_info->cmyk=MagickTrue;
      }
    if (CompareMagickByteBuffer(&buffer,CMYKCustomColor,strlen(CMYKCustomColor)) != MagickFalse)
      ps_info->cmyk=MagickTrue;
    if (CompareMagickByteBuffer(&buffer,CMYKProcessColor,strlen(CMYKProcessColor)) != MagickFalse)
      ps_info->cmyk=MagickTrue;
    spot_color=MagickFalse;
    length=strlen(DocumentCustomColors);
    if (CompareMagickByteBuffer(&buffer,DocumentCustomColors,length) != MagickFalse)
      {
        spot_color=MagickTrue;
        SkipMagickByteBuffer(&buffer,length+1);
      }
    if (spot_color == MagickFalse)
      {
        length=strlen(CMYKCustomColor);
        if (CompareMagickByteBuffer(&buffer,CMYKCustomColor,length) != MagickFalse)
          {
            spot_color=MagickTrue;
            SkipMagickByteBuffer(&buffer,length+1);
          }
      }
    if (spot_color == MagickFalse)
      {
        length=strlen(SpotColor);
        if (CompareMagickByteBuffer(&buffer,SpotColor,length) != MagickFalse)
          {
            spot_color=MagickTrue;
            SkipMagickByteBuffer(&buffer,length+1);
          }
      }
    if (spot_color != MagickFalse)
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
        for (c=PeekMagickByteBuffer(&buffer); c != EOF; c=PeekMagickByteBuffer(&buffer))
        {
          if ((c == '\r') || (c == '\n') || ((i+1) == MagickPathExtent))
            {
              new_line=MagickTrue;
              break;
            }
          name[i++]=(char) ReadMagickByteBuffer(&buffer);
        }
        name[i]='\0';
        value=ConstantString(name);
        (void) StripMagickString(value);
        if (*value != '\0')
          (void) SetImageProperty(image,property,value,exception);
        value=DestroyString(value);
        continue;
      }
    if ((ps_info->icc_profile == (StringInfo *) NULL) &&
        (CompareMagickByteBuffer(&buffer,ICCProfile,strlen(ICCProfile)) != MagickFalse))
      {
        unsigned char
          *datum;

        /*
          Read ICC profile.
        */
        if (SkipMagickByteBufferUntilNewline(&buffer) != MagickFalse)
          {
            ps_info->icc_profile=AcquireProfileStringInfo("icc",MagickPathExtent,
              exception);
            if (ps_info->icc_profile != (StringInfo*) NULL)
              {
                datum=GetStringInfoDatum(ps_info->icc_profile);
                for (i=0; (c=ProfileInteger(&buffer,hex_digits)) != EOF; i++)
                {
                  if (i >= (ssize_t) GetStringInfoLength(ps_info->icc_profile))
                    {
                      SetStringInfoLength(ps_info->icc_profile,(size_t) i << 1);
                      datum=GetStringInfoDatum(ps_info->icc_profile);
                    }
                  datum[i]=(unsigned char) c;
                }
                SetStringInfoLength(ps_info->icc_profile,(size_t) i+1);
              }
          }
        continue;
      }
    if ((ps_info->photoshop_profile == (StringInfo *) NULL) &&
        (CompareMagickByteBuffer(&buffer,PhotoshopProfile,strlen(PhotoshopProfile)) != MagickFalse))
      {
        unsigned long
          extent;

        unsigned char
          *q;

        /*
          Read Photoshop profile.
        */
        p=GetMagickByteBufferDatum(&buffer);
        extent=0;
        count=(ssize_t) MagickSscanf(p,PhotoshopProfile " %lu",&extent);
        if ((count != 1) || (extent == 0))
          continue;
        if ((MagickSizeType) extent > GetBlobSize(image))
          continue;
        length=(size_t) extent;
        if (SkipMagickByteBufferUntilNewline(&buffer) != MagickFalse)
          {
            ps_info->photoshop_profile=AcquireProfileStringInfo("8bim",
              length+1U,exception);
            if (ps_info->icc_profile != (StringInfo*) NULL)
              {
                q=GetStringInfoDatum(ps_info->photoshop_profile);
                while (extent > 0)
                {
                  c=ProfileInteger(&buffer,hex_digits);
                  if (c == EOF)
                    break;
                  *q++=(unsigned char) c;
                  extent-=MagickMin(extent,1);
                }
                SetStringInfoLength(ps_info->photoshop_profile,length);
              }
          }
        continue;
      }
    if (image_info->page != (char *) NULL)
      continue;
    /*
      Note region defined by bounding box.
    */
    count=0;
    i=0;
    if (CompareMagickByteBuffer(&buffer,BoundingBox,strlen(BoundingBox)) != MagickFalse)
      {
        p=GetMagickByteBufferDatum(&buffer);
        count=(ssize_t) MagickSscanf(p,BoundingBox " %lf %lf %lf %lf",
          &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
        i=2;
      }
    if (CompareMagickByteBuffer(&buffer,DocumentMedia,strlen(DocumentMedia)) != MagickFalse)
      {
        p=GetMagickByteBufferDatum(&buffer);
        count=(ssize_t) MagickSscanf(p,DocumentMedia " %lf %lf %lf %lf",
          &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
        i=1;
      }
    if (CompareMagickByteBuffer(&buffer,HiResBoundingBox,strlen(HiResBoundingBox)) != MagickFalse)
      {
        p=GetMagickByteBufferDatum(&buffer);
        count=(ssize_t) MagickSscanf(p,HiResBoundingBox " %lf %lf %lf %lf",
          &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
        i=3;
      }
    if (CompareMagickByteBuffer(&buffer,PageBoundingBox,strlen(PageBoundingBox)) != MagickFalse)
      {
        p=GetMagickByteBufferDatum(&buffer);
        count=(ssize_t) MagickSscanf(p,PageBoundingBox " %lf %lf %lf %lf",
          &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
        i=1;
      }
    if (CompareMagickByteBuffer(&buffer,PageMedia,strlen(PageMedia)) != MagickFalse)
      {
        p=GetMagickByteBufferDatum(&buffer);
        count=(ssize_t) MagickSscanf(p,PageMedia " %lf %lf %lf %lf",
          &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
        i=1;
      }
    if ((count != 4) || (i < (ssize_t) priority))
      continue;
    if ((fabs(bounds.x2-bounds.x1) <= fabs(ps_info->bounds.x2-ps_info->bounds.x1)) ||
        (fabs(bounds.y2-bounds.y1) <= fabs(ps_info->bounds.y2-ps_info->bounds.y1)))
      if (i ==  (ssize_t) priority)
        continue;
    ps_info->bounds=bounds;
    priority=i;
  }
  if (version[0] != '\0')
    (void) SetImageProperty(image,"ps:Level",version,exception);
}

static inline void CleanupPSInfo(PSInfo *pdf_info)
{
  if (pdf_info->icc_profile != (StringInfo *) NULL)
    pdf_info->icc_profile=DestroyStringInfo(pdf_info->icc_profile);
  if (pdf_info->photoshop_profile != (StringInfo *) NULL)
    pdf_info->photoshop_profile=DestroyStringInfo(pdf_info->photoshop_profile);
  if (pdf_info->xmp_profile != (StringInfo *) NULL)
    pdf_info->xmp_profile=DestroyStringInfo(pdf_info->xmp_profile);
}

static Image *ReadPSImage(const ImageInfo *image_info,ExceptionInfo *exception)
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
    *postscript_image;

  ImageInfo
    *read_info;

  int
    file;

  MagickBooleanType
    crop,
    fitPage,
    status;

  MagickStatusType
    flags;

  PointInfo
    delta,
    resolution;

  PSInfo
    info;

  RectangleInfo
    page;

  ssize_t
    count,
    i;

  unsigned long
    scene;

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
  resolution=image->resolution;
  page.width=(size_t) ((ssize_t) ceil((double) (page.width*resolution.x/
    delta.x)-0.5));
  page.height=(size_t) ((ssize_t) ceil((double) (page.height*resolution.y/
    delta.y)-0.5));
  /*
    Determine page geometry from the Postscript bounding box.
  */
  ReadPSInfo(image_info,image,&info,exception);
  (void) CloseBlob(image);
  /*
    Set Postscript render geometry.
  */
  if ((fabs(info.bounds.x2-info.bounds.x1) >= MagickEpsilon) &&
      (fabs(info.bounds.y2-info.bounds.y1) >= MagickEpsilon))
    {
      (void) FormatImageProperty(image,"ps:HiResBoundingBox",
        "%gx%g%+.15g%+.15g",info.bounds.x2-info.bounds.x1,info.bounds.y2-
        info.bounds.y1,info.bounds.x1,info.bounds.y1);
      page.width=(size_t) ((ssize_t) ceil((double) ((info.bounds.x2-
        info.bounds.x1)*resolution.x/delta.x)-0.5));
      page.height=(size_t) ((ssize_t) ceil((double) ((info.bounds.y2-
        info.bounds.y1)*resolution.y/delta.y)-0.5));
    }
  fitPage=MagickFalse;
  option=GetImageOption(image_info,"eps:fit-page");
  if (option != (const char *) NULL)
    {
      char
        *page_geometry;

      page_geometry=GetPageGeometry(option);
      flags=ParseMetaGeometry(page_geometry,&page.x,&page.y,&page.width,
        &page.height);
      if (flags == NoValue)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
            "InvalidGeometry","`%s'",option);
          page_geometry=DestroyString(page_geometry);
          CleanupPSInfo(&info);
          image=DestroyImage(image);
          return((Image *) NULL);
        }
      page.width=(size_t) ((ssize_t) ceil((double) (page.width*
        image->resolution.x/delta.x)-0.5));
      page.height=(size_t) ((ssize_t) ceil((double) (page.height*
        image->resolution.y/delta.y) -0.5));
      page_geometry=DestroyString(page_geometry);
      fitPage=MagickTrue;
    }
  crop=MagickFalse;
  if (*image_info->magick == 'E')
    {
      option=GetImageOption(image_info,"eps:use-cropbox");
      if ((option == (const char *) NULL) ||
          (IsStringTrue(option) != MagickFalse))
        crop=MagickTrue;
    }
  if (IssRGBCompatibleColorspace(image_info->colorspace) != MagickFalse)
    info.cmyk=MagickFalse;
  /*
    Create Ghostscript control file.
  */
  file=AcquireUniqueFileResource(postscript_filename);
  if (file == -1)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image_info->filename);
      CleanupPSInfo(&info);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) CopyMagickString(command,"/setpagedevice {pop} bind 1 index where {"
    "dup wcheck {3 1 roll put} {pop def} ifelse} {def} ifelse\n",
    MagickPathExtent);
  count=write(file,command,(unsigned int) strlen(command));
  if (image_info->page == (char *) NULL)
    {
      char
        translate_geometry[MagickPathExtent];

      (void) FormatLocaleString(translate_geometry,MagickPathExtent,
        "%g %g translate\n",-info.bounds.x1,-info.bounds.y1);
      count=write(file,translate_geometry,(unsigned int)
        strlen(translate_geometry));
    }
  (void) count;
  file=close_utf8(file)-1;
  /*
    Render Postscript with the Ghostscript delegate.
  */
  if (image_info->monochrome != MagickFalse)
    delegate_info=GetDelegateInfo("ps:mono",(char *) NULL,exception);
  else
    if (info.cmyk != MagickFalse)
      delegate_info=GetDelegateInfo("ps:cmyk",(char *) NULL,exception);
    else
      delegate_info=GetDelegateInfo("ps:alpha",(char *) NULL,exception);
  if (delegate_info == (const DelegateInfo *) NULL)
    {
      (void) RelinquishUniqueFileResource(postscript_filename);
      CleanupPSInfo(&info);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  density=AcquireString("");
  options=AcquireString("");
  (void) FormatLocaleString(density,MagickPathExtent,"%gx%g",resolution.x,
    resolution.y);
  if (crop == MagickFalse)
    {
      if (image_info->ping != MagickFalse)
        (void) FormatLocaleString(density,MagickPathExtent,"2.0x2.0");
      else
        (void) FormatLocaleString(options,MagickPathExtent,"-g%.20gx%.20g ",
          (double) page.width,(double) page.height);
    }
  read_info=CloneImageInfo(image_info);
  *read_info->magick='\0';
  if (read_info->number_scenes != 0)
    {
      char
        pages[MagickPathExtent];

      (void) FormatLocaleString(pages,MagickPathExtent,"-dFirstPage=%.20g "
        "-dLastPage=%.20g ",(double) read_info->scene+1,(double)
        (read_info->scene+read_info->number_scenes));
      (void) ConcatenateMagickString(options,pages,MagickPathExtent);
      read_info->number_scenes=0;
      if (read_info->scenes != (char *) NULL)
        *read_info->scenes='\0';
    }
  if (*image_info->magick == 'E')
    {
      if (crop != MagickFalse)
        (void) ConcatenateMagickString(options,"-dEPSCrop ",MagickPathExtent);
      if (fitPage != MagickFalse)
        (void) ConcatenateMagickString(options,"-dEPSFitPage ",
          MagickPathExtent);
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
  (void) InterpretImageFilename(image_info,image,filename,1,
    read_info->filename,exception);
  if ((status == MagickFalse) ||
      (IsGhostscriptRendered(read_info->filename) == MagickFalse))
    {
      (void) ConcatenateMagickString(command," -c showpage",MagickPathExtent);
      status=InvokeGhostscriptDelegate(read_info->verbose,command,message,
        exception);
    }
  (void) RelinquishUniqueFileResource(postscript_filename);
  (void) RelinquishUniqueFileResource(input_filename);
  postscript_image=(Image *) NULL;
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
      AppendImageToList(&postscript_image,next);
    }
  (void) RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  if (postscript_image == (Image *) NULL)
    {
      if (*message != '\0')
        (void) ThrowMagickException(exception,GetMagickModule(),
          DelegateError,"PostscriptDelegateFailed","`%s'",message);
      CleanupPSInfo(&info);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if (LocaleCompare(postscript_image->magick,"BMP") == 0)
    {
      Image
        *cmyk_image;

      cmyk_image=ConsolidateCMYKImages(postscript_image,exception);
      if (cmyk_image != (Image *) NULL)
        {
          postscript_image=DestroyImageList(postscript_image);
          postscript_image=cmyk_image;
        }
    }
  if (info.icc_profile != (StringInfo *) NULL)
    (void) SetImageProfilePrivate(image,info.icc_profile,exception);
  if (info.photoshop_profile != (StringInfo *) NULL)
    (void) SetImageProfilePrivate(image,info.photoshop_profile,exception);
  if (info.xmp_profile != (StringInfo *) NULL)
    (void) SetImageProfilePrivate(image,info.xmp_profile,exception);
  if (image_info->number_scenes != 0)
    {
      Image
        *clone_image;

      /*
        Add place holder images to meet the subimage specification requirement.
      */
      for (i=0; i < (ssize_t) image_info->scene; i++)
      {
        clone_image=CloneImage(postscript_image,1,1,MagickTrue,exception);
        if (clone_image != (Image *) NULL)
          PrependImageToList(&postscript_image,clone_image);
      }
    }
  do
  {
    (void) CopyMagickString(postscript_image->filename,filename,
      MagickPathExtent);
    (void) CopyMagickString(postscript_image->magick,image->magick,
      MagickPathExtent);
    if (info.columns != 0)
      postscript_image->magick_columns=info.columns;
    if (info.rows != 0)
      postscript_image->magick_rows=info.rows;
    postscript_image->page=page;
    if (image_info->ping != MagickFalse)
      {
        postscript_image->magick_columns=page.width;
        postscript_image->magick_rows=page.height;
        postscript_image->columns=page.width;
        postscript_image->rows=page.height;
      }
    (void) CloneImageProfiles(postscript_image,image);
    (void) CloneImageProperties(postscript_image,image);
    next=SyncNextImageInList(postscript_image);
    if (next != (Image *) NULL)
      postscript_image=next;
  } while (next != (Image *) NULL);
  image=DestroyImageList(image);
  scene=0;
  for (next=GetFirstImageInList(postscript_image); next != (Image *) NULL; )
  {
    next->scene=scene++;
    next=GetNextImageInList(next);
  }
  return(GetFirstImageInList(postscript_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P S I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPSImage() adds properties for the PS image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPSImage method is:
%
%      size_t RegisterPSImage(void)
%
*/
ModuleExport size_t RegisterPSImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("PS","EPI",
    "Encapsulated PostScript Interchange format");
  entry->decoder=(DecodeImageHandler *) ReadPSImage;
  entry->encoder=(EncodeImageHandler *) WritePSImage;
  entry->magick=(IsImageFormatHandler *) IsPS;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/postscript");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PS","EPS","Encapsulated PostScript");
  entry->decoder=(DecodeImageHandler *) ReadPSImage;
  entry->encoder=(EncodeImageHandler *) WritePSImage;
  entry->magick=(IsImageFormatHandler *) IsPS;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/postscript");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PS","EPSF","Encapsulated PostScript");
  entry->decoder=(DecodeImageHandler *) ReadPSImage;
  entry->encoder=(EncodeImageHandler *) WritePSImage;
  entry->magick=(IsImageFormatHandler *) IsPS;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/postscript");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PS","EPSI",
    "Encapsulated PostScript Interchange format");
  entry->decoder=(DecodeImageHandler *) ReadPSImage;
  entry->encoder=(EncodeImageHandler *) WritePSImage;
  entry->magick=(IsImageFormatHandler *) IsPS;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/postscript");
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("PS","PS","PostScript");
  entry->decoder=(DecodeImageHandler *) ReadPSImage;
  entry->encoder=(EncodeImageHandler *) WritePSImage;
  entry->magick=(IsImageFormatHandler *) IsPS;
  entry->mime_type=ConstantString("application/postscript");
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P S I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPSImage() removes format registrations made by the
%  PS module from the list of supported formats.
%
%  The format of the UnregisterPSImage method is:
%
%      UnregisterPSImage(void)
%
*/
ModuleExport void UnregisterPSImage(void)
{
  (void) UnregisterMagickInfo("EPI");
  (void) UnregisterMagickInfo("EPS");
  (void) UnregisterMagickInfo("EPSF");
  (void) UnregisterMagickInfo("EPSI");
  (void) UnregisterMagickInfo("PS");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P S I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WritePSImage translates an image to encapsulated Postscript
%  Level I for printing.  If the supplied geometry is null, the image is
%  centered on the Postscript page.  Otherwise, the image is positioned as
%  specified by the geometry.
%
%  The format of the WritePSImage method is:
%
%      MagickBooleanType WritePSImage(const ImageInfo *image_info,
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

static inline unsigned char *PopHexPixel(const char hex_digits[][3],
  const size_t pixel,unsigned char *pixels)
{
  const char
    *hex;

  hex=hex_digits[pixel];
  *pixels++=(unsigned char) (*hex++ & 0xff);
  *pixels++=(unsigned char) (*hex & 0xff);
  return(pixels);
}

static MagickBooleanType WritePSImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
#define WriteRunlengthPacket(image,pixel,length,p) \
{ \
  if ((image->alpha_trait != UndefinedPixelTrait) && (length != 0) && \
      (GetPixelAlpha(image,p) == (Quantum) TransparentAlpha)) \
    { \
      q=PopHexPixel(hex_digits,0xff,q); \
      q=PopHexPixel(hex_digits,0xff,q); \
      q=PopHexPixel(hex_digits,0xff,q); \
    } \
  else \
    { \
      q=PopHexPixel(hex_digits,ScaleQuantumToChar(ClampToQuantum(pixel.red)),q); \
      q=PopHexPixel(hex_digits,ScaleQuantumToChar(ClampToQuantum(pixel.green)),q); \
      q=PopHexPixel(hex_digits,ScaleQuantumToChar(ClampToQuantum(pixel.blue)),q); \
    } \
  q=PopHexPixel(hex_digits,(size_t) MagickMin(length,0xff),q); \
}

  static const char
    hex_digits[][3] =
    {
      "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B",
      "0C", "0D", "0E", "0F", "10", "11", "12", "13", "14", "15", "16", "17",
      "18", "19", "1A", "1B", "1C", "1D", "1E", "1F", "20", "21", "22", "23",
      "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
      "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B",
      "3C", "3D", "3E", "3F", "40", "41", "42", "43", "44", "45", "46", "47",
      "48", "49", "4A", "4B", "4C", "4D", "4E", "4F", "50", "51", "52", "53",
      "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
      "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B",
      "6C", "6D", "6E", "6F", "70", "71", "72", "73", "74", "75", "76", "77",
      "78", "79", "7A", "7B", "7C", "7D", "7E", "7F", "80", "81", "82", "83",
      "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
      "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B",
      "9C", "9D", "9E", "9F", "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
      "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF", "B0", "B1", "B2", "B3",
      "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
      "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB",
      "CC", "CD", "CE", "CF", "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
      "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF", "E0", "E1", "E2", "E3",
      "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
      "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB",
      "FC", "FD", "FE", "FF"
    },
    PostscriptProlog[] =
      "%%BeginProlog\n"
      "%\n"
      "% Display a color image.  The image is displayed in color on\n"
      "% Postscript viewers or printers that support color, otherwise\n"
      "% it is displayed as grayscale.\n"
      "%\n"
      "/DirectClassPacket\n"
      "{\n"
      "  %\n"
      "  % Get a DirectClass packet.\n"
      "  %\n"
      "  % Parameters:\n"
      "  %   red.\n"
      "  %   green.\n"
      "  %   blue.\n"
      "  %   length: number of pixels minus one of this color (optional).\n"
      "  %\n"
      "  currentfile color_packet readhexstring pop pop\n"
      "  compression 0 eq\n"
      "  {\n"
      "    /number_pixels 3 def\n"
      "  }\n"
      "  {\n"
      "    currentfile byte readhexstring pop 0 get\n"
      "    /number_pixels exch 1 add 3 mul def\n"
      "  } ifelse\n"
      "  0 3 number_pixels 1 sub\n"
      "  {\n"
      "    pixels exch color_packet putinterval\n"
      "  } for\n"
      "  pixels 0 number_pixels getinterval\n"
      "} bind def\n"
      "\n"
      "/DirectClassImage\n"
      "{\n"
      "  %\n"
      "  % Display a DirectClass image.\n"
      "  %\n"
      "  systemdict /colorimage known\n"
      "  {\n"
      "    columns rows 8\n"
      "    [\n"
      "      columns 0 0\n"
      "      rows neg 0 rows\n"
      "    ]\n"
      "    { DirectClassPacket } false 3 colorimage\n"
      "  }\n"
      "  {\n"
      "    %\n"
      "    % No colorimage operator;  convert to grayscale.\n"
      "    %\n"
      "    columns rows 8\n"
      "    [\n"
      "      columns 0 0\n"
      "      rows neg 0 rows\n"
      "    ]\n"
      "    { GrayDirectClassPacket } image\n"
      "  } ifelse\n"
      "} bind def\n"
      "\n"
      "/GrayDirectClassPacket\n"
      "{\n"
      "  %\n"
      "  % Get a DirectClass packet;  convert to grayscale.\n"
      "  %\n"
      "  % Parameters:\n"
      "  %   red\n"
      "  %   green\n"
      "  %   blue\n"
      "  %   length: number of pixels minus one of this color (optional).\n"
      "  %\n"
      "  currentfile color_packet readhexstring pop pop\n"
      "  color_packet 0 get 0.299 mul\n"
      "  color_packet 1 get 0.587 mul add\n"
      "  color_packet 2 get 0.114 mul add\n"
      "  cvi\n"
      "  /gray_packet exch def\n"
      "  compression 0 eq\n"
      "  {\n"
      "    /number_pixels 1 def\n"
      "  }\n"
      "  {\n"
      "    currentfile byte readhexstring pop 0 get\n"
      "    /number_pixels exch 1 add def\n"
      "  } ifelse\n"
      "  0 1 number_pixels 1 sub\n"
      "  {\n"
      "    pixels exch gray_packet put\n"
      "  } for\n"
      "  pixels 0 number_pixels getinterval\n"
      "} bind def\n"
      "\n"
      "/GrayPseudoClassPacket\n"
      "{\n"
      "  %\n"
      "  % Get a PseudoClass packet;  convert to grayscale.\n"
      "  %\n"
      "  % Parameters:\n"
      "  %   index: index into the colormap.\n"
      "  %   length: number of pixels minus one of this color (optional).\n"
      "  %\n"
      "  currentfile byte readhexstring pop 0 get\n"
      "  /offset exch 3 mul def\n"
      "  /color_packet colormap offset 3 getinterval def\n"
      "  color_packet 0 get 0.299 mul\n"
      "  color_packet 1 get 0.587 mul add\n"
      "  color_packet 2 get 0.114 mul add\n"
      "  cvi\n"
      "  /gray_packet exch def\n"
      "  compression 0 eq\n"
      "  {\n"
      "    /number_pixels 1 def\n"
      "  }\n"
      "  {\n"
      "    currentfile byte readhexstring pop 0 get\n"
      "    /number_pixels exch 1 add def\n"
      "  } ifelse\n"
      "  0 1 number_pixels 1 sub\n"
      "  {\n"
      "    pixels exch gray_packet put\n"
      "  } for\n"
      "  pixels 0 number_pixels getinterval\n"
      "} bind def\n"
      "\n"
      "/PseudoClassPacket\n"
      "{\n"
      "  %\n"
      "  % Get a PseudoClass packet.\n"
      "  %\n"
      "  % Parameters:\n"
      "  %   index: index into the colormap.\n"
      "  %   length: number of pixels minus one of this color (optional).\n"
      "  %\n"
      "  currentfile byte readhexstring pop 0 get\n"
      "  /offset exch 3 mul def\n"
      "  /color_packet colormap offset 3 getinterval def\n"
      "  compression 0 eq\n"
      "  {\n"
      "    /number_pixels 3 def\n"
      "  }\n"
      "  {\n"
      "    currentfile byte readhexstring pop 0 get\n"
      "    /number_pixels exch 1 add 3 mul def\n"
      "  } ifelse\n"
      "  0 3 number_pixels 1 sub\n"
      "  {\n"
      "    pixels exch color_packet putinterval\n"
      "  } for\n"
      "  pixels 0 number_pixels getinterval\n"
      "} bind def\n"
      "\n"
      "/PseudoClassImage\n"
      "{\n"
      "  %\n"
      "  % Display a PseudoClass image.\n"
      "  %\n"
      "  % Parameters:\n"
      "  %   class: 0-PseudoClass or 1-Grayscale.\n"
      "  %\n"
      "  currentfile buffer readline pop\n"
      "  token pop /class exch def pop\n"
      "  class 0 gt\n"
      "  {\n"
      "    currentfile buffer readline pop\n"
      "    token pop /depth exch def pop\n"
      "    /grays columns 8 add depth sub depth mul 8 idiv string def\n"
      "    columns rows depth\n"
      "    [\n"
      "      columns 0 0\n"
      "      rows neg 0 rows\n"
      "    ]\n"
      "    { currentfile grays readhexstring pop } image\n"
      "  }\n"
      "  {\n"
      "    %\n"
      "    % Parameters:\n"
      "    %   colors: number of colors in the colormap.\n"
      "    %   colormap: red, green, blue color packets.\n"
      "    %\n"
      "    currentfile buffer readline pop\n"
      "    token pop /colors exch def pop\n"
      "    /colors colors 3 mul def\n"
      "    /colormap colors string def\n"
      "    currentfile colormap readhexstring pop pop\n"
      "    systemdict /colorimage known\n"
      "    {\n"
      "      columns rows 8\n"
      "      [\n"
      "        columns 0 0\n"
      "        rows neg 0 rows\n"
      "      ]\n"
      "      { PseudoClassPacket } false 3 colorimage\n"
      "    }\n"
      "    {\n"
      "      %\n"
      "      % No colorimage operator;  convert to grayscale.\n"
      "      %\n"
      "      columns rows 8\n"
      "      [\n"
      "        columns 0 0\n"
      "        rows neg 0 rows\n"
      "      ]\n"
      "      { GrayPseudoClassPacket } image\n"
      "    } ifelse\n"
      "  } ifelse\n"
      "} bind def\n"
      "\n"
      "/DisplayImage\n"
      "{\n"
      "  %\n"
      "  % Display a DirectClass or PseudoClass image.\n"
      "  %\n"
      "  % Parameters:\n"
      "  %   x & y translation.\n"
      "  %   x & y scale.\n"
      "  %   label pointsize.\n"
      "  %   image label.\n"
      "  %   image columns & rows.\n"
      "  %   class: 0-DirectClass or 1-PseudoClass.\n"
      "  %   compression: 0-none or 1-RunlengthEncoded.\n"
      "  %   hex color packets.\n"
      "  %\n"
      "  gsave\n"
      "  /buffer 512 string def\n"
      "  /byte 1 string def\n"
      "  /color_packet 3 string def\n"
      "  /pixels 768 string def\n"
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
      "  token pop /compression exch def pop\n"
      "  class 0 gt { PseudoClassImage } { DirectClassImage } ifelse\n"
      "  grestore\n";

  char
    buffer[MagickPathExtent],
    date[MagickTimeExtent],
    **labels,
    page_geometry[MagickPathExtent];

  CompressionType
    compression;

  const char
    *value;

  const Quantum
    *p;

  const StringInfo
    *profile;

  double
    pointsize;

  GeometryInfo
    geometry_info;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  MagickStatusType
    flags;

  PixelInfo
    pixel;

  PointInfo
    delta,
    resolution,
    scale;

  Quantum
    index;

  RectangleInfo
    geometry,
    media_info,
    page_info;

  SegmentInfo
    bounds;

  size_t
    bit,
    byte,
    number_scenes,
    length,
    page,
    text_size;

  ssize_t
    i,
    j,
    x,
    y;

  time_t
    timer;

  unsigned char
    pixels[2048],
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
  (void) memset(&bounds,0,sizeof(bounds));
  compression=image->compression;
  if (image_info->compression != UndefinedCompression)
    compression=image_info->compression;
  page=1;
  scene=0;
  number_scenes=GetImageListLength(image);
  do
  {
    ImageType
      type = UndefinedType;

    /*
      Scale relative to dots-per-inch.
    */
    if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
      (void) TransformImageColorspace(image,sRGBColorspace,exception);
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
    scale.x=MagickSafeReciprocal(resolution.x)*geometry.width*delta.x;
    geometry.width=CastDoubleToSizeT(scale.x+0.5);
    scale.y=MagickSafeReciprocal(resolution.y)*geometry.height*delta.y;
    geometry.height=CastDoubleToSizeT(scale.y+0.5);
    (void) ParseAbsoluteGeometry(page_geometry,&media_info);
    (void) ParseGravityGeometry(image,page_geometry,&page_info,exception);
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
    if (page == 1)
      {
        /*
          Output Postscript header.
        */
        if (LocaleCompare(image_info->magick,"PS") == 0)
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
        (void) FormatMagickTime(timer,sizeof(date),date);
        (void) FormatLocaleString(buffer,MagickPathExtent,
          "%%%%CreationDate: (%s)\n",date);
        (void) WriteBlobString(image,buffer);
        bounds.x1=(double) geometry.x;
        bounds.y1=(double) geometry.y;
        bounds.x2=(double) geometry.x+scale.x;
        bounds.y2=(double) geometry.y+(scale.y+text_size);
        if ((image_info->adjoin != MagickFalse) &&
            (GetNextImageInList(image) != (Image *) NULL))
          (void) CopyMagickString(buffer,"%%%%BoundingBox: (atend)\n",
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
        profile=GetImageProfile(image,"8bim");
        if (profile != (StringInfo *) NULL)
          {
            /*
              Embed Photoshop profile.
            */
            (void) FormatLocaleString(buffer,MagickPathExtent,
              "%%BeginPhotoshop: %.20g",(double) GetStringInfoLength(profile));
            (void) WriteBlobString(image,buffer);
            for (i=0; i < (ssize_t) GetStringInfoLength(profile); i++)
            {
              if ((i % 32) == 0)
                (void) WriteBlobString(image,"\n% ");
              (void) FormatLocaleString(buffer,MagickPathExtent,"%02X",
                (unsigned int) (GetStringInfoDatum(profile)[i] & 0xff));
              (void) WriteBlobString(image,buffer);
            }
            (void) WriteBlobString(image,"\n%EndPhotoshop\n");
          }
        profile=GetImageProfile(image,"xmp");
        value=GetImageProperty(image,"label",exception);
        if (value != (const char *) NULL)
          (void) WriteBlobString(image,
            "%%DocumentNeededResources: font Times-Roman\n");
        (void) WriteBlobString(image,"%%DocumentData: Clean7Bit\n");
        (void) WriteBlobString(image,"%%LanguageLevel: 1\n");
        if (LocaleCompare(image_info->magick,"PS") != 0)
          (void) WriteBlobString(image,"%%Pages: 1\n");
        else
          {
            /*
              Compute the number of pages.
            */
            (void) WriteBlobString(image,"%%Orientation: Portrait\n");
            (void) WriteBlobString(image,"%%PageOrder: Ascend\n");
            (void) FormatLocaleString(buffer,MagickPathExtent,
              "%%%%Pages: %.20g\n",image_info->adjoin != MagickFalse ?
              (double) number_scenes : 1.0);
            (void) WriteBlobString(image,buffer);
          }
        (void) WriteBlobString(image,"%%EndComments\n");
        (void) WriteBlobString(image,"\n%%BeginDefaults\n");
        (void) WriteBlobString(image,"%%EndDefaults\n\n");
        if ((LocaleCompare(image_info->magick,"EPI") == 0) ||
            (LocaleCompare(image_info->magick,"EPSI") == 0) ||
            (LocaleCompare(image_info->magick,"EPT") == 0))
          {
            Image
              *preview_image;

            /*
              Create preview image.
            */
            preview_image=CloneImage(image,0,0,MagickTrue,exception);
            if (preview_image == (Image *) NULL)
              ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
            /*
              Dump image as bitmap.
            */
            (void) FormatLocaleString(buffer,MagickPathExtent,
              "%%%%BeginPreview: %.20g %.20g %.20g %.20g\n%%  ",(double)
              preview_image->columns,(double) preview_image->rows,1.0,
              (double) ((((preview_image->columns+7) >> 3)*preview_image->rows+
              35)/36));
            (void) WriteBlobString(image,buffer);
            q=pixels;
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(preview_image,0,y,preview_image->columns,1,
                exception);
              if (p == (const Quantum *) NULL)
                break;
              bit=0;
              byte=0;
              for (x=0; x < (ssize_t) preview_image->columns; x++)
              {
                Quantum
                  luma;

                byte<<=1;
                luma=ClampToQuantum(GetPixelLuma(preview_image,p));
                if (luma >= (Quantum) (QuantumRange/2))
                  byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    q=PopHexPixel(hex_digits,byte,q);
                    if ((q-pixels+8) >= 80)
                      {
                        *q++='\n';
                        (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                        q=pixels;
                        (void) WriteBlobString(image,"%  ");
                      };
                    bit=0;
                    byte=0;
                  }
              }
              if (bit != 0)
                {
                  byte<<=(8-bit);
                  q=PopHexPixel(hex_digits,byte,q);
                  if ((q-pixels+8) >= 80)
                    {
                      *q++='\n';
                      (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                      q=pixels;
                      (void) WriteBlobString(image,"%  ");
                    };
                };
            }
            if (q != pixels)
              {
                *q++='\n';
                (void) WriteBlob(image,(size_t) (q-pixels),pixels);
              }
            (void) WriteBlobString(image,"\n%%EndPreview\n");
            preview_image=DestroyImage(preview_image);
          }
        /*
          Output Postscript commands.
        */
        (void) WriteBlob(image,sizeof(PostscriptProlog)-1,
          (const unsigned char *) PostscriptProlog);
        value=GetImageProperty(image,"label",exception);
        if (value != (const char *) NULL)
          {
            (void) WriteBlobString(image,
              "  /Times-Roman findfont pointsize scalefont setfont\n");
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
        if (LocaleCompare(image_info->magick,"PS") == 0)
          (void) WriteBlobString(image,"  showpage\n");
        (void) WriteBlobString(image,"} bind def\n");
        (void) WriteBlobString(image,"%%EndProlog\n");
      }
    (void) FormatLocaleString(buffer,MagickPathExtent,"%%%%Page:  1 %.20g\n",
      (double) (page++));
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
    if ((double) (geometry.x+(ssize_t) geometry.width-1) > bounds.x2)
      bounds.x2=(double) geometry.x+geometry.width-1;
    if ((double) (geometry.y+((ssize_t) geometry.height+(ssize_t) text_size)-1) > bounds.y2)
      bounds.y2=(double) geometry.y+(geometry.height+text_size)-1;
    value=GetImageProperty(image,"label",exception);
    if (value != (const char *) NULL)
      (void) WriteBlobString(image,"%%%%PageResources: font Times-Roman\n");
    if (LocaleCompare(image_info->magick,"PS") != 0)
      (void) WriteBlobString(image,"userdict begin\n");
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
    (void) memset(&pixel,0,sizeof(pixel));
    pixel.alpha=(MagickRealType) TransparentAlpha;
    index=(Quantum) 0;
    x=0;
    if (image_info->type != TrueColorType)
      type=IdentifyImageCoderGrayType(image,exception);
    if (IsGrayImageType(type) != MagickFalse)
      {
        if (type != BilevelType)
          {
            /*
              Dump image as grayscale.
            */
            (void) FormatLocaleString(buffer,MagickPathExtent,
              "%.20g %.20g\n1\n1\n1\n8\n",(double) image->columns,(double)
              image->rows);
            (void) WriteBlobString(image,buffer);
            q=pixels;
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                Quantum
                  luma;

                luma=(Quantum) ScaleQuantumToChar(ClampToQuantum(GetPixelLuma(
                  image,p)));
                q=PopHexPixel(hex_digits,(size_t) luma,q);
                if ((q-pixels+8) >= 80)
                  {
                    *q++='\n';
                    (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                    q=pixels;
                  }
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
            if (q != pixels)
              {
                *q++='\n';
                (void) WriteBlob(image,(size_t) (q-pixels),pixels);
              }
          }
        else
          {
            /*
              Dump image as bitmap.
            */
            (void) FormatLocaleString(buffer,MagickPathExtent,
              "%.20g %.20g\n1\n1\n1\n1\n",(double) image->columns,(double)
              image->rows);
            (void) WriteBlobString(image,buffer);
            q=pixels;
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,exception);
              if (p == (const Quantum *) NULL)
                break;
              bit=0;
              byte=0;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                Quantum
                  luma;

                byte<<=1;
                luma=ClampToQuantum(GetPixelLuma(image,p));
                if (luma >= (Quantum) (QuantumRange/2))
                  byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    q=PopHexPixel(hex_digits,byte,q);
                    if ((q-pixels+2) >= 80)
                      {
                        *q++='\n';
                        (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                        q=pixels;
                      };
                    bit=0;
                    byte=0;
                  }
                p+=(ptrdiff_t) GetPixelChannels(image);
              }
              if (bit != 0)
                {
                  byte<<=(8-bit);
                  q=PopHexPixel(hex_digits,byte,q);
                  if ((q-pixels+2) >= 80)
                    {
                      *q++='\n';
                      (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                      q=pixels;
                    }
                };
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,(MagickOffsetType)
                    y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
            if (q != pixels)
              {
                *q++='\n';
                (void) WriteBlob(image,(size_t) (q-pixels),pixels);
              }
          }
      }
    else
      if ((image->storage_class == DirectClass) ||
          (image->colors > 256) || (image->alpha_trait != UndefinedPixelTrait))
        {
          /*
            Dump DirectClass image.
          */
          (void) FormatLocaleString(buffer,MagickPathExtent,
            "%.20g %.20g\n0\n%d\n",(double) image->columns,(double) image->rows,
            compression == RLECompression ? 1 : 0);
          (void) WriteBlobString(image,buffer);
          switch (compression)
          {
            case RLECompression:
            {
              /*
                Dump runlength-encoded DirectColor packets.
              */
              q=pixels;
              for (y=0; y < (ssize_t) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                GetPixelInfoPixel(image,p,&pixel);
                length=255;
                for (x=0; x < (ssize_t) image->columns; x++)
                {
                  if ((GetPixelRed(image,p) == ClampToQuantum(pixel.red)) &&
                      (GetPixelGreen(image,p) == ClampToQuantum(pixel.green)) &&
                      (GetPixelBlue(image,p) == ClampToQuantum(pixel.blue)) &&
                      (GetPixelAlpha(image,p) == ClampToQuantum(pixel.alpha)) &&
                      (length < 255) && (x < (ssize_t) (image->columns-1)))
                    length++;
                  else
                    {
                      if (x > 0)
                        {
                          WriteRunlengthPacket(image,pixel,length,p);
                          if ((q-pixels+10) >= 80)
                            {
                              *q++='\n';
                              (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                              q=pixels;
                            }
                        }
                      length=0;
                    }
                  GetPixelInfoPixel(image,p,&pixel);
                  p+=(ptrdiff_t) GetPixelChannels(image);
                }
                WriteRunlengthPacket(image,pixel,length,p);
                if ((q-pixels+10) >= 80)
                  {
                    *q++='\n';
                    (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                    q=pixels;
                  }
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,SaveImageTag,
                      (MagickOffsetType) y,image->rows);
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
            case NoCompression:
            default:
            {
              /*
                Dump uncompressed DirectColor packets.
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
                      q=PopHexPixel(hex_digits,0xff,q);
                      q=PopHexPixel(hex_digits,0xff,q);
                      q=PopHexPixel(hex_digits,0xff,q);
                    }
                  else
                    {
                      q=PopHexPixel(hex_digits,ScaleQuantumToChar(
                        GetPixelRed(image,p)),q);
                      q=PopHexPixel(hex_digits,ScaleQuantumToChar(
                        GetPixelGreen(image,p)),q);
                      q=PopHexPixel(hex_digits,ScaleQuantumToChar(
                        GetPixelBlue(image,p)),q);
                    }
                  if ((q-pixels+6) >= 80)
                    {
                      *q++='\n';
                      (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                      q=pixels;
                    }
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
              if (q != pixels)
                {
                  *q++='\n';
                  (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                }
              break;
            }
          }
          (void) WriteBlobByte(image,'\n');
        }
      else
        {
          /*
            Dump PseudoClass image.
          */
          (void) FormatLocaleString(buffer,MagickPathExtent,
            "%.20g %.20g\n%d\n%d\n0\n",(double) image->columns,(double)
            image->rows,image->storage_class == PseudoClass ? 1 : 0,
            compression == RLECompression ? 1 : 0);
          (void) WriteBlobString(image,buffer);
          /*
            Dump number of colors and colormap.
          */
          (void) FormatLocaleString(buffer,MagickPathExtent,"%.20g\n",(double)
            image->colors);
          (void) WriteBlobString(image,buffer);
          for (i=0; i < (ssize_t) image->colors; i++)
          {
            (void) FormatLocaleString(buffer,MagickPathExtent,"%02X%02X%02X\n",
              ScaleQuantumToChar(ClampToQuantum(image->colormap[i].red)),
              ScaleQuantumToChar(ClampToQuantum(image->colormap[i].green)),
              ScaleQuantumToChar(ClampToQuantum(image->colormap[i].blue)));
            (void) WriteBlobString(image,buffer);
          }
          switch (compression)
          {
            case RLECompression:
            {
              /*
                Dump runlength-encoded PseudoColor packets.
              */
              q=pixels;
              for (y=0; y < (ssize_t) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                index=GetPixelIndex(image,p);
                length=255;
                for (x=0; x < (ssize_t) image->columns; x++)
                {
                  if ((index == GetPixelIndex(image,p)) &&
                      (length < 255) && (x < ((ssize_t) image->columns-1)))
                    length++;
                  else
                    {
                      if (x > 0)
                        {
                          q=PopHexPixel(hex_digits,(size_t) index,q);
                          q=PopHexPixel(hex_digits,(size_t)
                            MagickMin(length,0xff),q);
                          i++;
                          if ((q-pixels+6) >= 80)
                            {
                              *q++='\n';
                              (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                              q=pixels;
                            }
                        }
                      length=0;
                    }
                  index=GetPixelIndex(image,p);
                  pixel.red=(MagickRealType) GetPixelRed(image,p);
                  pixel.green=(MagickRealType) GetPixelGreen(image,p);
                  pixel.blue=(MagickRealType) GetPixelBlue(image,p);
                  pixel.alpha=(MagickRealType) GetPixelAlpha(image,p);
                  p+=(ptrdiff_t) GetPixelChannels(image);
                }
                q=PopHexPixel(hex_digits,(size_t) index,q);
                q=PopHexPixel(hex_digits,(size_t) MagickMin(length,0xff),q);
                if ((q-pixels+6) >= 80)
                  {
                    *q++='\n';
                    (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                    q=pixels;
                  }
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,SaveImageTag,
                      (MagickOffsetType) y,image->rows);
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
            case NoCompression:
            default:
            {
              /*
                Dump uncompressed PseudoColor packets.
              */
              q=pixels;
              for (y=0; y < (ssize_t) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,exception);
                if (p == (const Quantum *) NULL)
                  break;
                for (x=0; x < (ssize_t) image->columns; x++)
                {
                  q=PopHexPixel(hex_digits,(size_t) GetPixelIndex(image,p),q);
                  if ((q-pixels+4) >= 80)
                    {
                      *q++='\n';
                      (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                      q=pixels;
                    }
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
              if (q != pixels)
                {
                  *q++='\n';
                  (void) WriteBlob(image,(size_t) (q-pixels),pixels);
                }
              break;
            }
          }
          (void) WriteBlobByte(image,'\n');
        }
    if (LocaleCompare(image_info->magick,"PS") != 0)
      (void) WriteBlobString(image,"end\n");
    (void) WriteBlobString(image,"%%PageTrailer\n");
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,number_scenes);
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) WriteBlobString(image,"%%Trailer\n");
  if (page > 2)
    {
      (void) FormatLocaleString(buffer,MagickPathExtent,
        "%%%%BoundingBox: %.20g %.20g %.20g %.20g\n",ceil(bounds.x1-0.5),
        ceil(bounds.y1-0.5),floor(bounds.x2-0.5),floor(bounds.y2-0.5));
      (void) WriteBlobString(image,buffer);
      (void) FormatLocaleString(buffer,MagickPathExtent,
        "%%%%HiResBoundingBox: %g %g %g %g\n",bounds.x1,bounds.y1,bounds.x2,
        bounds.y2);
      (void) WriteBlobString(image,buffer);
    }
  (void) WriteBlobString(image,"%%EOF\n");
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}

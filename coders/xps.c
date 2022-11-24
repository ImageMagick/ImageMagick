/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            X   X  PPPP   SSSSS                              %
%                             X X   P   P  SS                                 %
%                              X    PPPP    SSS                               %
%                             X X   P         SS                              %
%                            X   X  P      SSSSS                              %
%                                                                             %
%                                                                             %
%             Read/Write Microsoft XML Paper Specification Format             %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                January 2008                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 2008 ImageMagick Studio LLC, a non-profit organization         %
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
#include "coders/bytebuffer-private.h"
#include "coders/ghostscript-private.h"

/*
  Typedef declarations.
*/
typedef struct _XPSInfo
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
} XPSInfo;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X P S I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadXPSImage() reads a Printer Control Language image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
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

static Image *ReadXPSImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    command[MagickPathExtent],
    *density,
    filename[MagickPathExtent],
    input_filename[MagickPathExtent],
    message[MagickPathExtent],
    *options;

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

  MagickBooleanType
    fitPage,
    status;

  MagickStatusType
    flags;

  PointInfo
    delta,
    resolution;

  RectangleInfo
    page;

  ssize_t
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
  fitPage=MagickFalse;
  option=GetImageOption(image_info,"xps:fit-page");
  if (option != (char *) NULL)
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
  /*
    Render Postscript with the Ghostscript delegate.
  */
  delegate_info=GetDelegateInfo("xps:color",(char *) NULL,exception);
  if (delegate_info == (const DelegateInfo *) NULL)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  density=AcquireString("");
  options=AcquireString("");
  (void) FormatLocaleString(density,MagickPathExtent,"%gx%g",resolution.x,
    resolution.y);
  if (image_info->ping != MagickFalse)
    (void) FormatLocaleString(density,MagickPathExtent,"2.0x2.0");
  else
    (void) FormatLocaleString(options,MagickPathExtent,"-g%.20gx%.20g ",
      (double) page.width,(double) page.height);
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
      option=GetImageOption(image_info,"xps:use-cropbox");
      if ((option == (const char *) NULL) ||
          (IsStringTrue(option) != MagickFalse))
        (void) ConcatenateMagickString(options,"-dEPSCrop ",MagickPathExtent);
      if (fitPage != MagickFalse)
        (void) ConcatenateMagickString(options,"-dEPSFitPage ",
          MagickPathExtent);
    }
  (void) AcquireUniqueFilename(read_info->filename);
  (void) RelinquishUniqueFileResource(read_info->filename);
  (void) ConcatenateMagickString(read_info->filename,"%d",MagickPathExtent);
  (void) CopyMagickString(filename,read_info->filename,MagickPathExtent);
  (void) FormatLocaleString(command,MagickPathExtent,
    GetDelegateCommands(delegate_info),
    read_info->antialias != MagickFalse ? 4 : 1,
    read_info->antialias != MagickFalse ? 4 : 1,density,options,
    read_info->filename,input_filename);
  options=DestroyString(options);
  density=DestroyString(density);
  *message='\0';
  status=ExternalDelegateCommand(MagickFalse,read_info->verbose,command,
    (char *) NULL,exception) != 0 ? MagickTrue : MagickFalse;
  (void) RelinquishUniqueFileResource(input_filename);
  postscript_image=(Image *) NULL;
  if (status == MagickFalse)
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
  (void) RelinquishUniqueFileResource(filename);
  read_info=DestroyImageInfo(read_info);
  if (postscript_image == (Image *) NULL)
    {
      if (*message != '\0')
        (void) ThrowMagickException(exception,GetMagickModule(),
          DelegateError,"PostscriptDelegateFailed","`%s'",message);
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
%   R e g i s t e r X P S I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterXPSImage() adds properties for the PS image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterXPSImage method is:
%
%      size_t RegisterXPSImage(void)
%
*/
ModuleExport size_t RegisterXPSImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("XPS","XPS","Microsoft XML Paper Specification");
  entry->decoder=(DecodeImageHandler *) ReadXPSImage;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->mime_type=ConstantString("application/oxps");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r X P S I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterXPSImage() removes format registrations made by the
%  XPS module from the list of supported formats.
%
%  The format of the UnregisterXPSImage method is:
%
%      UnregisterXPSImage(void)
%
*/
ModuleExport void UnregisterXPSImage(void)
{
  (void) UnregisterMagickInfo("XPS");
}

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
%                               January 2008                                  %
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
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/constitute.h"
#include "MagickCore/delegate.h"
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
#include "MagickCore/profile.h"
#include "MagickCore/property.h"
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/token.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"

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
%  The format of the ReadXPSImage method is:
%
%      Image *ReadXPSImage(const ImageInfo *image_info,ExceptionInfo *exception)
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
#define CropBox  "CropBox"
#define DeviceCMYK  "DeviceCMYK"
#define MediaBox  "MediaBox"
#define RenderXPSText  "  Rendering XPS...  "

  char
    command[MagickPathExtent],
    *density,
    filename[MagickPathExtent],
    geometry[MagickPathExtent],
    *options,
    input_filename[MagickPathExtent];

  const DelegateInfo
    *delegate_info;

  Image
    *image,
    *next_image;

  ImageInfo
    *read_info;

  MagickBooleanType
    cmyk,
    status;

  PointInfo
    delta;

  RectangleInfo
    bounding_box,
    page;

  register char
    *p;

  register ssize_t
    c;

  SegmentInfo
    bounds;

  size_t
    height,
    width;

  ssize_t
    count;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
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
  /*
    Determine page geometry from the XPS media box.
  */
  cmyk=image->colorspace == CMYKColorspace ? MagickTrue : MagickFalse;
  count=0;
  (void) ResetMagickMemory(&bounding_box,0,sizeof(bounding_box));
  (void) ResetMagickMemory(&bounds,0,sizeof(bounds));
  (void) ResetMagickMemory(&page,0,sizeof(page));
  (void) ResetMagickMemory(command,0,sizeof(command));
  p=command;
  for (c=ReadBlobByte(image); c != EOF; c=ReadBlobByte(image))
  {
    if (image_info->page != (char *) NULL)
      continue;
    /*
      Note XPS elements.
    */
    *p++=(char) c;
    if ((c != (int) '/') && (c != '\n') &&
        ((size_t) (p-command) < (MagickPathExtent-1)))
      continue;
    *p='\0';
    p=command;
    /*
      Is this a CMYK document?
    */
    if (LocaleNCompare(DeviceCMYK,command,strlen(DeviceCMYK)) == 0)
      cmyk=MagickTrue;
    if (LocaleNCompare(CropBox,command,strlen(CropBox)) == 0)
      {
        /*
          Note region defined by crop box.
        */
        count=(ssize_t) sscanf(command,"CropBox [%lf %lf %lf %lf",
          &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
        if (count != 4)
          count=(ssize_t) sscanf(command,"CropBox[%lf %lf %lf %lf",
            &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
      }
    if (LocaleNCompare(MediaBox,command,strlen(MediaBox)) == 0)
      {
        /*
          Note region defined by media box.
        */
        count=(ssize_t) sscanf(command,"MediaBox [%lf %lf %lf %lf",
          &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
        if (count != 4)
          count=(ssize_t) sscanf(command,"MediaBox[%lf %lf %lf %lf",
            &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
      }
    if (count != 4)
      continue;
    /*
      Set XPS render geometry.
    */
    width=(size_t) (floor(bounds.x2+0.5)-ceil(bounds.x1-0.5));
    height=(size_t) (floor(bounds.y2+0.5)-ceil(bounds.y1-0.5));
    if (width > page.width)
      page.width=width;
    if (height > page.height)
      page.height=height;
  }
  (void) CloseBlob(image);
  /*
    Render XPS with the GhostXPS delegate.
  */
  if ((page.width == 0) || (page.height == 0))
    (void) ParseAbsoluteGeometry(PSPageGeometry,&page);
  if (image_info->page != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->page,&page);
  (void) FormatLocaleString(geometry,MagickPathExtent,"%.20gx%.20g",(double)
    page.width,(double) page.height);
  if (image_info->monochrome != MagickFalse)
    delegate_info=GetDelegateInfo("xps:mono",(char *) NULL,exception);
  else
     if (cmyk != MagickFalse)
       delegate_info=GetDelegateInfo("xps:cmyk",(char *) NULL,exception);
     else
       delegate_info=GetDelegateInfo("xps:color",(char *) NULL,exception);
  if (delegate_info == (const DelegateInfo *) NULL)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  density=AcquireString("");
  options=AcquireString("");
  (void) FormatLocaleString(density,MagickPathExtent,"%gx%g",
    image->resolution.x,image->resolution.y);
  if ((page.width == 0) || (page.height == 0))
    (void) ParseAbsoluteGeometry(PSPageGeometry,&page);
  if (image_info->page != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->page,&page);
  page.width=(size_t) floor(page.width*image->resolution.y/delta.x+0.5);
  page.height=(size_t) floor(page.height*image->resolution.y/delta.y+0.5);
  (void) FormatLocaleString(options,MagickPathExtent,"-g%.20gx%.20g ",(double)
    page.width,(double) page.height);
  image=DestroyImage(image);
  read_info=CloneImageInfo(image_info);
  *read_info->magick='\0';
  if (read_info->number_scenes != 0)
    {
      if (read_info->number_scenes != 1)
        (void) FormatLocaleString(options,MagickPathExtent,"-dLastPage=%.20g",
          (double) (read_info->scene+read_info->number_scenes));
      else
        (void) FormatLocaleString(options,MagickPathExtent,
          "-dFirstPage=%.20g -dLastPage=%.20g",(double) read_info->scene+1,
          (double) (read_info->scene+read_info->number_scenes));
      read_info->number_scenes=0;
      if (read_info->scenes != (char *) NULL)
        *read_info->scenes='\0';
    }
  (void) CopyMagickString(filename,read_info->filename,MagickPathExtent);
  (void) AcquireUniqueFilename(read_info->filename);
  (void) FormatLocaleString(command,MagickPathExtent,
    GetDelegateCommands(delegate_info),
    read_info->antialias != MagickFalse ? 4 : 1,
    read_info->antialias != MagickFalse ? 4 : 1,density,options,
    read_info->filename,input_filename);
  options=DestroyString(options);
  density=DestroyString(density);
  status=ExternalDelegateCommand(MagickFalse,read_info->verbose,command,
    (char *) NULL,exception) != 0 ? MagickTrue : MagickFalse;
  image=ReadImage(read_info,exception);
  (void) RelinquishUniqueFileResource(read_info->filename);
  (void) RelinquishUniqueFileResource(input_filename);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    ThrowReaderException(DelegateError,"XPSDelegateFailed");
  if (LocaleCompare(image->magick,"BMP") == 0)
    {
      Image
        *cmyk_image;

      cmyk_image=ConsolidateCMYKImages(image,exception);
      if (cmyk_image != (Image *) NULL)
        {
          image=DestroyImageList(image);
          image=cmyk_image;
        }
    }
  do
  {
    (void) CopyMagickString(image->filename,filename,MagickPathExtent);
    image->page=page;
    next_image=SyncNextImageInList(image);
    if (next_image != (Image *) NULL)
      image=next_image;
  } while (next_image != (Image *) NULL);
  return(GetFirstImageInList(image));
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
%  RegisterXPSImage() adds attributes for the Microsoft XML Paper Specification 
%  format to the list of supported formats.  The attributes include the image
%  format tag, a method to read and/or write the format, whether the format
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
  entry->flags^=CoderAdjoinFlag;
  entry->flags^=CoderBlobSupportFlag;
  entry->flags^=CoderDecoderThreadSupportFlag;
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
%  UnregisterXPSImage() removes format registrations made by the XPS module
%  from the list of supported formats.
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

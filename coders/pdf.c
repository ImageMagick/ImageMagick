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
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/compress.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/delegate-private.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/resize.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"
#include "magick/utility.h"
#include "magick/module.h"
#if defined(MAGICKCORE_TIFF_DELEGATE)
#define CCITTParam  "-1"
#else
#define CCITTParam  "0"
#endif

/*
  Forward declarations.
*/
static MagickBooleanType
  WritePDFImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n v o k e P D F D e l e g a t e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InvokePDFDelegate() executes the PDF interpreter with the specified command.
%
%  The format of the InvokePDFDelegate method is:
%
%      MagickBooleanType InvokePDFDelegate(const MagickBooleanType verbose,
%        const char *command,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o verbose: A value other than zero displays the command prior to
%      executing it.
%
%    o command: the address of a character string containing the command to
%      execute.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType InvokePDFDelegate(const MagickBooleanType verbose,
  const char *command,ExceptionInfo *exception)
{
  int
    status;

#if defined(MAGICKCORE_GS_DELEGATE) || defined(__WINDOWS__)
  char
    **argv;

  const GhostInfo
    *ghost_info;

  gs_main_instance
    *interpreter;

  int
    argc,
    code;

  register long
    i;

#if defined(__WINDOWS__)
  ghost_info=NTGhostscriptDLLVectors();
#else
  GhostInfo
    ghost_info_struct;

  ghost_info=(&ghost_info_struct);
  (void) ResetMagickMemory(&ghost_info,0,sizeof(ghost_info));
  ghost_info_struct.new_instance=(int (*)(gs_main_instance **,void *))
    gsapi_new_instance;
  ghost_info_struct.init_with_args=(int (*)(gs_main_instance *,int,char **))
    gsapi_init_with_args;
  ghost_info_struct.run_string=(int (*)(gs_main_instance *,const char *,int,
    int *)) gsapi_run_string;
  ghost_info_struct.delete_instance=(void (*)(gs_main_instance *))
    gsapi_delete_instance;
  ghost_info_struct.exit=(int (*)(gs_main_instance *)) gsapi_exit;
#endif
  if (ghost_info == (GhostInfo *) NULL)
    {
      status=SystemCommand(verbose,command,exception);
      return(status == 0 ? MagickTrue : MagickFalse);
    }
  if (verbose != MagickFalse)
    {
      (void) fputs("[ghostscript library]",stdout);
      (void) fputs(strchr(command,' '),stdout);
    }
  status=(ghost_info->new_instance)(&interpreter,(void *) NULL);
  if (status < 0)
    {
      status=SystemCommand(verbose,command,exception);
      return(status == 0 ? MagickTrue : MagickFalse);
    }
  argv=StringToArgv(command,&argc);
  status=(ghost_info->init_with_args)(interpreter,argc-1,argv+1);
  if (status == 0)
    status=(ghost_info->run_string)(interpreter,"systemdict /start get exec\n",
      0,&code);
  (ghost_info->exit)(interpreter);
  (ghost_info->delete_instance)(interpreter);
#if defined(__WINDOWS__)
  NTGhostscriptUnLoadDLL();
#endif
  for (i=0; i < (long) argc; i++)
    argv[i]=DestroyString(argv[i]);
  argv=(char **) RelinquishMagickMemory(argv);
  if ((status != 0) && (status != -101))
    {
      char
        *message;

      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(exception,GetMagickModule(),DelegateError,
        "`%s': %s",command,message);
      message=DestroyString(message);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "Ghostscript returns status %d, exit code %d",status,code);
      return(MagickFalse);
    }
  return(MagickTrue);
#else
  status=SystemCommand(verbose,command,exception);
  return(status == 0 ? MagickTrue : MagickFalse);
#endif
}

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

static MagickBooleanType IsPDFRendered(const char *path)
{
  MagickBooleanType
    status;

  struct stat
    attributes;

  if ((path == (const char *) NULL) || (*path == '\0'))
    return(MagickFalse);
  status=GetPathAttributes(path,&attributes);
  if ((status != MagickFalse) && S_ISREG(attributes.st_mode) &&
      (attributes.st_size > 0))
    return(MagickTrue);
  return(MagickFalse);
}

static Image *ReadPDFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define CropBox  "CropBox"
#define DeviceCMYK  "DeviceCMYK"
#define MediaBox  "MediaBox"
#define RenderPostscriptText  "Rendering Postscript...  "
#define PDFRotate  "Rotate"
#define SpotColor  "Separation"
#define TrimBox  "TrimBox"
#define PDFVersion  "PDF-"

  char
    command[MaxTextExtent],
    density[MaxTextExtent],
    filename[MaxTextExtent],
    geometry[MaxTextExtent],
    options[MaxTextExtent],
    input_filename[MaxTextExtent],
    postscript_filename[MaxTextExtent];

  const char
    *option;

  const DelegateInfo
    *delegate_info;

  double
    angle;

  Image
    *image,
    *next,
    *pdf_image;

  ImageInfo
    *read_info;

  int
    file;

  MagickBooleanType
    cmyk,
    cropbox,
    trimbox,
    status;

  PointInfo
    delta;

  RectangleInfo
    bounding_box,
    page;

  register char
    *p;

  register int
    c;

  SegmentInfo
    bounds,
    hires_bounds;

  ssize_t
    count;

  unsigned long
    scene,
    spotcolor;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  /*
    Open image file.
  */
  image=AcquireImage(image_info);
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
  if ((image->x_resolution == 0.0) || (image->y_resolution == 0.0))
    {
      GeometryInfo
        geometry_info;

      MagickStatusType
        flags;

      flags=ParseGeometry(PSDensityGeometry,&geometry_info);
      image->x_resolution=geometry_info.rho;
      image->y_resolution=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->y_resolution=image->x_resolution;
    }
  /*
    Determine page geometry from the PDF media box.
  */
  cmyk=image_info->colorspace == CMYKColorspace ? MagickTrue : MagickFalse;
  cropbox=MagickFalse;
  option=GetImageOption(image_info,"pdf:use-cropbox");
  if (option != (const char *) NULL)
    cropbox=IsMagickTrue(option);
  trimbox=MagickFalse;
  option=GetImageOption(image_info,"pdf:use-trimbox");
  if (option != (const char *) NULL)
    trimbox=IsMagickTrue(option);
  count=0;
  spotcolor=0;
  (void) ResetMagickMemory(&bounding_box,0,sizeof(bounding_box));
  (void) ResetMagickMemory(&bounds,0,sizeof(bounds));
  (void) ResetMagickMemory(&hires_bounds,0,sizeof(hires_bounds));
  (void) ResetMagickMemory(&page,0,sizeof(page));
  (void) ResetMagickMemory(command,0,sizeof(command));
  hires_bounds.x2=0.0;
  hires_bounds.y2=0.0;
  angle=0.0;
  p=command;
  for (c=ReadBlobByte(image); c != EOF; c=ReadBlobByte(image))
  {
    /*
      Note PDF elements.
    */
    if (c == '\n')
      c=' ';
    *p++=(char) c;
    if ((c != (int) '/') && (c != (int) '%') && 
        ((size_t) (p-command) < (MaxTextExtent-1)))
      continue;
    *(--p)='\0';
    p=command;
    if (LocaleNCompare(PDFRotate,command,strlen(PDFRotate)) == 0)
      count=(ssize_t) sscanf(command,"Rotate %lf",&angle);
    /*
      Is this a CMYK document?
    */
    if (LocaleNCompare(DeviceCMYK,command,strlen(DeviceCMYK)) == 0)
      cmyk=MagickTrue;
    if (LocaleNCompare(SpotColor,command,strlen(SpotColor)) == 0)
      {
        char
          name[MaxTextExtent],
          property[MaxTextExtent],
          *value;

        register long
          i;

        /*
          Note spot names.
        */
        (void) FormatMagickString(property,MaxTextExtent,"pdf:SpotColor-%lu",
          spotcolor++);
        i=0;
        for (c=ReadBlobByte(image); c != EOF; c=ReadBlobByte(image))
        {
          if ((isspace(c) != 0) || (c == '/') || ((i+1) == MaxTextExtent))
            break;
          name[i++]=(char) c;
        }
        name[i]='\0';
        value=AcquireString(name);
        (void) SubstituteString(&value,"#20"," ");
        (void) SetImageProperty(image,property,value);
        value=DestroyString(value);
        continue;
      }
    if (LocaleNCompare(PDFVersion,command,strlen(PDFVersion)) == 0)
      (void) SetImageProperty(image,"pdf:Version",command);
    count=0;
    if (cropbox != MagickFalse)
      {
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
      }
    else
      if (trimbox != MagickFalse)
        {
          if (LocaleNCompare(TrimBox,command,strlen(TrimBox)) == 0)
            {
              /*
                Note region defined by trim box.
              */
              count=(ssize_t) sscanf(command,"TrimBox [%lf %lf %lf %lf",
                &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
              if (count != 4)
                count=(ssize_t) sscanf(command,"TrimBox[%lf %lf %lf %lf",
                  &bounds.x1,&bounds.y1,&bounds.x2,&bounds.y2);
            }
        }
      else
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
    if (((bounds.x2 > hires_bounds.x2) && (bounds.y2 > hires_bounds.y2)) ||
        ((hires_bounds.x2 == 0.0) && (hires_bounds.y2 == 0.0)))
      {
        /*
          Set PDF render geometry.
        */
        (void) FormatMagickString(geometry,MaxTextExtent,"%gx%g%+g%+g",
          bounds.x2-bounds.x1,bounds.y2-bounds.y1,bounds.x1,bounds.y1);
        (void) SetImageProperty(image,"pdf:HiResBoundingBox",geometry);
        page.width=(unsigned long) (bounds.x2-bounds.x1+0.5);
        page.height=(unsigned long) (bounds.y2-bounds.y1+0.5);
        hires_bounds=bounds;
      }
  }
  (void) CloseBlob(image);
  if ((fabs(angle) == 90.0) || (fabs(angle) == 270.0))
    {
      unsigned long
        swap;

      swap=page.width;
      page.width=page.height;
      page.height=swap;
    }
  if (image_info->colorspace == RGBColorspace)
    cmyk=MagickFalse;
  /*
    Create Ghostscript control file.
  */
  file=AcquireUniqueFileResource(postscript_filename);
  if (file == -1)
    {
      ThrowFileException(exception,FileOpenError,"UnableToCreateTemporaryFile",
        image_info->filename);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  count=write(file," ",1);
  file=close(file)-1;
  /*
    Render Postscript with the Ghostscript delegate.
  */
  if ((image_info->ping != MagickFalse) ||
      (image_info->monochrome != MagickFalse))
    delegate_info=GetDelegateInfo("ps:mono",(char *) NULL,exception);
  else
     if (cmyk != MagickFalse)
       delegate_info=GetDelegateInfo("ps:cmyk",(char *) NULL,exception);
     else
       if (LocaleCompare(image_info->magick,"AI") == 0)
         delegate_info=GetDelegateInfo("ps:alpha",(char *) NULL,exception);
       else
         delegate_info=GetDelegateInfo("ps:color",(char *) NULL,exception);
  if (delegate_info == (const DelegateInfo *) NULL)
    {
      (void) RelinquishUniqueFileResource(postscript_filename);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  *options='\0';
  (void) FormatMagickString(density,MaxTextExtent,"%gx%g",image->x_resolution,
    image->y_resolution);
  if (image_info->page != (char *) NULL)
    {
      (void) ParseAbsoluteGeometry(image_info->page,&page);
      page.width=(unsigned long) (page.width*image->x_resolution/delta.x+0.5);
      page.height=(unsigned long) (page.height*image->y_resolution/delta.y+0.5);
      (void) FormatMagickString(options,MaxTextExtent,"-g%lux%lu ",page.width,
        page.height);
    }
  if (cmyk != MagickFalse)
    (void) ConcatenateMagickString(options,"-dUseCIEColor ",MaxTextExtent);
  if (cropbox != MagickFalse)
    (void) ConcatenateMagickString(options,"-dUseCropBox ",MaxTextExtent);
  if (trimbox != MagickFalse)
    (void) ConcatenateMagickString(options,"-dUseTrimBox ",MaxTextExtent);
  read_info=CloneImageInfo(image_info);
  *read_info->magick='\0';
  if (read_info->number_scenes != 0)
    {
      char
        pages[MaxTextExtent];

      (void) FormatMagickString(pages,MaxTextExtent,"-dFirstPage=%lu "
        "-dLastPage=%lu",read_info->scene+1,read_info->scene+
        read_info->number_scenes);
      (void) ConcatenateMagickString(options,pages,MaxTextExtent);
      read_info->number_scenes=0;
      if (read_info->scenes != (char *) NULL)
        *read_info->scenes='\0';
    }
  if (read_info->authenticate != (char *) NULL)
    (void) FormatMagickString(options+strlen(options),MaxTextExtent,
      " -sPDFPassword=%s",read_info->authenticate);
  (void) CopyMagickString(filename,read_info->filename,MaxTextExtent);
  (void) AcquireUniqueFilename(read_info->filename);
  (void) FormatMagickString(command,MaxTextExtent,
    GetDelegateCommands(delegate_info),
    read_info->antialias != MagickFalse ? 4 : 1,
    read_info->antialias != MagickFalse ? 4 : 1,density,options,
    read_info->filename,postscript_filename,input_filename);
  status=InvokePDFDelegate(read_info->verbose,command,exception);
  pdf_image=(Image *) NULL;
  if ((status != MagickFalse) &&
      (IsPDFRendered(read_info->filename) != MagickFalse))
    pdf_image=ReadImage(read_info,exception);
  (void) RelinquishUniqueFileResource(postscript_filename);
  (void) RelinquishUniqueFileResource(read_info->filename);
  (void) RelinquishUniqueFileResource(input_filename);
  read_info=DestroyImageInfo(read_info);
  if (pdf_image == (Image *) NULL)
    {
      ThrowFileException(exception,DelegateError,"PostscriptDelegateFailed",
        image_info->filename);
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
  if (image_info->number_scenes != 0)
    {
      Image
        *clone_image;

      register long
        i;

      /*
        Add place holder images to meet the subimage specification requirement.
      */
      for (i=0; i < (long) image_info->scene; i++)
      {
        clone_image=CloneImage(pdf_image,1,1,MagickTrue,exception);
        if (clone_image != (Image *) NULL)
          PrependImageToList(&pdf_image,clone_image);
      }
    }
  do
  {
    (void) CopyMagickString(pdf_image->filename,filename,MaxTextExtent);
    pdf_image->page=page;
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
%      unsigned long RegisterPDFImage(void)
%
*/
ModuleExport unsigned long RegisterPDFImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("AI");
  entry->decoder=(DecodeImageHandler *) ReadPDFImage;
  entry->encoder=(EncodeImageHandler *) WritePDFImage;
  entry->adjoin=MagickFalse;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=EncoderThreadSupport;
  entry->description=ConstantString("Adobe Illustrator CS2");
  entry->module=ConstantString("PDF");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("EPDF");
  entry->decoder=(DecodeImageHandler *) ReadPDFImage;
  entry->encoder=(EncodeImageHandler *) WritePDFImage;
  entry->adjoin=MagickFalse;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=EncoderThreadSupport;
  entry->description=ConstantString("Encapsulated Portable Document Format");
  entry->module=ConstantString("PDF");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PDF");
  entry->decoder=(DecodeImageHandler *) ReadPDFImage;
  entry->encoder=(EncodeImageHandler *) WritePDFImage;
  entry->magick=(IsImageFormatHandler *) IsPDF;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=EncoderThreadSupport;
  entry->description=ConstantString("Portable Document Format");
  entry->module=ConstantString("PDF");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PDFA");
  entry->decoder=(DecodeImageHandler *) ReadPDFImage;
  entry->encoder=(EncodeImageHandler *) WritePDFImage;
  entry->magick=(IsImageFormatHandler *) IsPDF;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->thread_support=EncoderThreadSupport;
  entry->description=ConstantString("Portable Document Archive Format");
  entry->module=ConstantString("PDF");
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
%      MagickBooleanType WritePDFImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static char *EscapeParenthesis(const char *text)
{
  register char
    *p;

  register long
    i;

  static char
    buffer[MaxTextExtent];

  unsigned long
    escapes;

  escapes=0;
  p=buffer;
  for (i=0; i < (long) MagickMin(strlen(text),(MaxTextExtent-escapes-1)); i++)
  {
    if ((text[i] == '(') || (text[i] == ')'))
      {
        *p++='\\';
        escapes++;
      }
    *p++=text[i];
  }
  *p='\0';
  return(buffer);
}

static MagickBooleanType WritePDFImage(const ImageInfo *image_info,Image *image)
{
#define CFormat  "/Filter [ /%s ]\n"
#define ObjectsPerImage  14

  static const char
    XMPProfile[]=
    {
      "<?xpacket begin=\"%s\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n"
      "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP Core 4.0-c316 44.253921, Sun Oct 01 2006 17:08:23\">\n"
      "   <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
      "      <rdf:Description rdf:about=\"\"\n"
      "            xmlns:xap=\"http://ns.adobe.com/xap/1.0/\">\n"
      "         <xap:ModifyDate>%s</xap:ModifyDate>\n"
      "         <xap:CreateDate>%s</xap:CreateDate>\n"
      "         <xap:MetadataDate>%s</xap:MetadataDate>\n"
      "         <xap:CreatorTool>%s</xap:CreatorTool>\n"
      "      </rdf:Description>\n"
      "      <rdf:Description rdf:about=\"\"\n"
      "            xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
      "         <dc:format>application/pdf</dc:format>\n"
      "      </rdf:Description>\n"
      "      <rdf:Description rdf:about=\"\"\n"
      "            xmlns:xapMM=\"http://ns.adobe.com/xap/1.0/mm/\">\n"
      "         <xapMM:DocumentID>uuid:6ec119d7-7982-4f56-808d-dfe64f5b35cf</xapMM:DocumentID>\n"
      "         <xapMM:InstanceID>uuid:a79b99b4-6235-447f-9f6c-ec18ef7555cb</xapMM:InstanceID>\n"
      "      </rdf:Description>\n"
      "      <rdf:Description rdf:about=\"\"\n"
      "            xmlns:pdf=\"http://ns.adobe.com/pdf/1.3/\">\n"
      "         <pdf:Producer>%s</pdf:Producer>\n"
      "      </rdf:Description>\n"
      "      <rdf:Description rdf:about=\"\"\n"
      "            xmlns:pdfaid=\"http://www.aiim.org/pdfa/ns/id/\">\n"
      "         <pdfaid:part>1</pdfaid:part>\n"
      "         <pdfaid:conformance>B</pdfaid:conformance>\n"
      "      </rdf:Description>\n"
      "   </rdf:RDF>\n"
      "</x:xmpmeta>\n"
      "<?xpacket end=\"w\"?>\n"
    },
    XMPProfileMagick[4]= { (char) 0xef, (char) 0xbb, (char) 0xbf, (char) 0x00 };

  char
    buffer[MaxTextExtent],
    date[MaxTextExtent],
    **labels,
    page_geometry[MaxTextExtent];

  CompressionType
    compression;

  const char
    *value;

  double
    pointsize;

  GeometryInfo
    geometry_info;

  long
    count,
    y;

  Image
    *next,
    *tile_image;

  MagickBooleanType
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

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register unsigned char
    *q;

  register long
    i,
    x;

  size_t
    length;

  struct tm
    local_time;

  time_t
    seconds;

  unsigned char
    *pixels;

  unsigned long
    info_id,
    object,
    pages_id,
    root_id,
    text_size,
    version;

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
  /*
    Allocate X ref memory.
  */
  xref=(MagickOffsetType *) AcquireQuantumMemory(2048UL,sizeof(*xref));
  if (xref == (MagickOffsetType *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(xref,0,2048UL*sizeof(*xref));
  /*
    Write Info object.
  */
  object=0;
  version=3;
  if (image_info->compression == JPEG2000Compression)
    version=(unsigned long) MagickMax(version,5);
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
    if (next->matte != MagickFalse)
      version=(unsigned long) MagickMax(version,4);
  if (LocaleCompare(image_info->magick,"PDFA") == 0)
    version=(unsigned long) MagickMax(version,6);
  (void) FormatMagickString(buffer,MaxTextExtent,"%%PDF-1.%lu \n",version);
  (void) WriteBlobString(image,buffer);
  if (LocaleCompare(image_info->magick,"PDFA") == 0)
    (void) WriteBlobString(image,"%����\n");
  /*
    Write Catalog object.
  */
  xref[object++]=TellBlob(image);
  root_id=object;
  (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"<<\n");
  if (LocaleCompare(image_info->magick,"PDFA") != 0)
    (void) FormatMagickString(buffer,MaxTextExtent,"/Pages %lu 0 R\n",
      object+1);
  else
    {
      (void) FormatMagickString(buffer,MaxTextExtent,"/Metadata %lu 0 R\n",
        object+1);
      (void) WriteBlobString(image,buffer);
      (void) FormatMagickString(buffer,MaxTextExtent,"/Pages %lu 0 R\n",
        object+2);
    }
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"/Type /Catalog\n");
  (void) WriteBlobString(image,">>\n");
  (void) WriteBlobString(image,"endobj\n");
  if (LocaleCompare(image_info->magick,"PDFA") == 0)
    {
      char
        create_date[MaxTextExtent],
        modify_date[MaxTextExtent],
        timestamp[MaxTextExtent],
        xmp_profile[MaxTextExtent];

      unsigned long
        version;

      /*
        Write XMP object.
      */
      xref[object++]=TellBlob(image);
      (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
      (void) WriteBlobString(image,buffer);
      (void) WriteBlobString(image,"<<\n");
      (void) WriteBlobString(image,"/Subtype /XML\n");
      *modify_date='\0';
      value=GetImageProperty(image,"date:modify");
      if (value != (const char *) NULL)
        (void) CopyMagickString(modify_date,value,MaxTextExtent);
      *create_date='\0';
      value=GetImageProperty(image,"date:create");
      if (value != (const char *) NULL)
        (void) CopyMagickString(create_date,value,MaxTextExtent);
      (void) FormatMagickTime(time((time_t *) NULL),MaxTextExtent,timestamp);
      i=FormatMagickString(xmp_profile,MaxTextExtent,XMPProfile,
        XMPProfileMagick,modify_date,create_date,timestamp,
        GetMagickVersion(&version),GetMagickVersion(&version));
      (void) FormatMagickString(buffer,MaxTextExtent,"/Length %lu\n",1UL*i);
      (void) WriteBlobString(image,buffer);
      (void) WriteBlobString(image,"/Type /Metadata\n");
      (void) WriteBlobString(image,">>\nstream\n");
      (void) WriteBlobString(image,xmp_profile);
      (void) WriteBlobString(image,"endstream\n");
      (void) WriteBlobString(image,"endobj\n");
    }
  /*
    Write Pages object.
  */
  xref[object++]=TellBlob(image);
  pages_id=object;
  (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"<<\n");
  (void) WriteBlobString(image,"/Type /Pages\n");
  (void) FormatMagickString(buffer,MaxTextExtent,"/Kids [ %lu 0 R ",object+1);
  (void) WriteBlobString(image,buffer);
  count=(long) (pages_id+ObjectsPerImage+1);
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
        (void) FormatMagickString(buffer,MaxTextExtent,"%ld 0 R ",count);
        (void) WriteBlobString(image,buffer);
        kid_image=GetNextImageInList(kid_image);
      }
      xref=(MagickOffsetType *) ResizeQuantumMemory(xref,(size_t) count+2048UL,
        sizeof(*xref));
      if (xref == (MagickOffsetType *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) WriteBlobString(image,"]\n");
  (void) FormatMagickString(buffer,MaxTextExtent,"/Count %lu\n",
    (count-pages_id)/ObjectsPerImage);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,">>\n");
  (void) WriteBlobString(image,"endobj\n");
  scene=0;
  do
  {
    compression=image->compression;
    if (image_info->compression != UndefinedCompression)
      compression=image_info->compression;
    switch (compression)
    {
      case FaxCompression:
      case Group4Compression:
      {
        if ((IsMonochromeImage(image,&image->exception) == MagickFalse) ||
            (image->matte != MagickFalse))
          compression=RLECompression;
        break;
      }
#if !defined(MAGICKCORE_JPEG_DELEGATE)
      case JPEGCompression:
      {
        compression=RLECompression;
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","`%s' (JPEG)",
          image->filename);
        break;
      }
#endif
#if !defined(MAGICKCORE_JP2_DELEGATE)
      case JPEG2000Compression:
      {
        compression=RLECompression;
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","`%s' (JP2)",
          image->filename);
        break;
      }
#endif
#if !defined(MAGICKCORE_ZLIB_DELEGATE)
      case ZipCompression:
      {
        compression=RLECompression;
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          MissingDelegateError,"DelegateLibrarySupportNotBuiltIn","`%s' (ZLIB)",
          image->filename);
        break;
      }
#endif
      case LZWCompression:
      {
        if (LocaleCompare(image_info->magick,"PDFA") == 0)
          compression=RLECompression;  /* LZW compression is forbidden */
        break;
      }
      case NoCompression:
      {
        if (LocaleCompare(image_info->magick,"PDFA") == 0)
          compression=RLECompression; /* ASCII 85 compression is forbidden */
        break;
      }
      default:
        break;
    }
    if (compression == JPEG2000Compression)
      {
        if (image->colorspace != RGBColorspace)
          (void) TransformImageColorspace(image,RGBColorspace);
      }
    /*
      Scale relative to dots-per-inch.
    */
    delta.x=DefaultResolution;
    delta.y=DefaultResolution;
    resolution.x=image->x_resolution;
    resolution.y=image->y_resolution;
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
        resolution.x*=2.54;
        resolution.y*=2.54;
      }
    SetGeometry(image,&geometry);
    (void) FormatMagickString(page_geometry,MaxTextExtent,"%lux%lu",
      image->columns,image->rows);
    if (image_info->page != (char *) NULL)
      (void) CopyMagickString(page_geometry,image_info->page,MaxTextExtent);
    else
      if ((image->page.width != 0) && (image->page.height != 0))
        (void) FormatMagickString(page_geometry,MaxTextExtent,"%lux%lu%+ld%+ld",
          image->page.width,image->page.height,image->page.x,image->page.y);
      else
        if ((image->gravity != UndefinedGravity) &&
            (LocaleCompare(image_info->magick,"PDF") == 0))
          (void) CopyMagickString(page_geometry,PSPageGeometry,MaxTextExtent);
    (void) ConcatenateMagickString(page_geometry,">",MaxTextExtent);
    (void) ParseMetaGeometry(page_geometry,&geometry.x,&geometry.y,
      &geometry.width,&geometry.height);
    scale.x=(double) (geometry.width*delta.x)/resolution.x;
    geometry.width=(unsigned long) (scale.x+0.5);
    scale.y=(double) (geometry.height*delta.y)/resolution.y;
    geometry.height=(unsigned long) (scale.y+0.5);
    (void) ParseAbsoluteGeometry(page_geometry,&media_info);
    (void) ParseGravityGeometry(image,page_geometry,&page_info,
      &image->exception);
    if (image->gravity != UndefinedGravity)
      {
        geometry.x=(-page_info.x);
        geometry.y=(long) (media_info.height+page_info.y-image->rows);
      }
    pointsize=12.0;
    if (image_info->pointsize != 0.0)
      pointsize=image_info->pointsize;
    text_size=0;
    value=GetImageProperty(image,"Label");
    if (value != (const char *) NULL)
      text_size=(unsigned long) (MultilineCensus(value)*pointsize+12);
    /*
      Write Page object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    (void) WriteBlobString(image,"/Type /Page\n");
    (void) FormatMagickString(buffer,MaxTextExtent,"/Parent %lu 0 R\n",
      pages_id);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"/Resources <<\n");
    labels=(char **) NULL;
    value=GetImageProperty(image,"Label");
    if (value != (const char *) NULL)
      labels=StringToList(value);
    if (labels != (char **) NULL)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,
          "/Font << /F%lu %lu 0 R >>\n",image->scene,object+4);
        (void) WriteBlobString(image,buffer);
      }
    (void) FormatMagickString(buffer,MaxTextExtent,
      "/XObject << /Im%lu %lu 0 R >>\n",image->scene,object+5);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/ProcSet %lu 0 R >>\n",
      object+3);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/MediaBox [0 0 %g %g]\n",
      72.0*media_info.width/resolution.x,72.0*media_info.height/resolution.y);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/CropBox [0 0 %g %g]\n",
      72.0*media_info.width/resolution.x,72.0*media_info.height/resolution.y);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/Contents %lu 0 R\n",
      object+1);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/Thumb %lu 0 R\n",
      object+8);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,">>\n");
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Contents object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    (void) FormatMagickString(buffer,MaxTextExtent,"/Length %lu 0 R\n",
      object+1);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,">>\n");
    (void) WriteBlobString(image,"stream\n");
    offset=TellBlob(image);
    (void) WriteBlobString(image,"q\n");
    if (labels != (char **) NULL)
      for (i=0; labels[i] != (char *) NULL; i++)
      {
        (void) WriteBlobString(image,"BT\n");
        (void) FormatMagickString(buffer,MaxTextExtent,"/F%lu %g Tf\n",
          image->scene,pointsize);
        (void) WriteBlobString(image,buffer);
        (void) FormatMagickString(buffer,MaxTextExtent,"%ld %ld Td\n",
          geometry.x,(long) (geometry.y+geometry.height+i*pointsize+12));
        (void) WriteBlobString(image,buffer);
        (void) FormatMagickString(buffer,MaxTextExtent,"(%s) Tj\n",labels[i]);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,"ET\n");
        labels[i]=DestroyString(labels[i]);
      }
    (void) FormatMagickString(buffer,MaxTextExtent,"%g 0 0 %g %ld %ld cm\n",
      scale.x,scale.y,geometry.x,geometry.y);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/Im%lu Do\n",image->scene);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"Q\n");
    offset=TellBlob(image)-offset;
    (void) WriteBlobString(image,"endstream\n");
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Length object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu\n",
      (unsigned long) offset);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Procset object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    if ((image->storage_class == DirectClass) || (image->colors > 256))
      (void) CopyMagickString(buffer,"[ /PDF /Text /ImageC",MaxTextExtent);
    else
      if ((compression == FaxCompression) || (compression == Group4Compression))
        (void) CopyMagickString(buffer,"[ /PDF /Text /ImageB",MaxTextExtent);
      else
        (void) CopyMagickString(buffer,"[ /PDF /Text /ImageI",MaxTextExtent);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image," ]\n");
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Font object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    if (labels != (char **) NULL)
      {
        (void) WriteBlobString(image,"/Type /Font\n");
        (void) WriteBlobString(image,"/Subtype /Type1\n");
        (void) FormatMagickString(buffer,MaxTextExtent,"/Name /F%lu\n",
          image->scene);
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
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    (void) WriteBlobString(image,"/Type /XObject\n");
    (void) WriteBlobString(image,"/Subtype /Image\n");
    (void) FormatMagickString(buffer,MaxTextExtent,"/Name /Im%lu\n",
      image->scene);
    (void) WriteBlobString(image,buffer);
    switch (compression)
    {
      case NoCompression:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"ASCII85Decode");
        break;
      }
      case JPEGCompression:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"DCTDecode");
        if (image->colorspace != CMYKColorspace)
          break;
        (void) WriteBlobString(image,buffer);
        (void) CopyMagickString(buffer,"/Decode [1 0 1 0 1 0 1 0]\n",
          MaxTextExtent);
        break;
      }
      case JPEG2000Compression:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"JPXDecode");
        if (image->colorspace != CMYKColorspace)
          break;
        (void) WriteBlobString(image,buffer);
        (void) CopyMagickString(buffer,"/Decode [1 0 1 0 1 0 1 0]\n",
          MaxTextExtent);
        break;
      }
      case LZWCompression:
      {
         (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"LZWDecode");
         break;
      }
      case ZipCompression:
      {
         (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"FlateDecode");
         break;
      }
      case FaxCompression:
      case Group4Compression:
      {
        (void) CopyMagickString(buffer,"/Filter [ /CCITTFaxDecode ]\n",
          MaxTextExtent);
        (void) WriteBlobString(image,buffer);
        (void) FormatMagickString(buffer,MaxTextExtent,"/DecodeParms [ << "
          "/K %s /BlackIs1 true /Columns %ld /Rows %ld >> ]\n",CCITTParam,
          image->columns,image->rows);
        break;
      }
      default:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,CFormat,
          "RunLengthDecode");
        break;
      }
    }
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/Width %lu\n",
      image->columns);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/Height %lu\n",image->rows);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/ColorSpace %lu 0 R\n",
      object+2);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/BitsPerComponent %d\n",
      (compression == FaxCompression) || (compression == Group4Compression) ?
      1 : 8);
    (void) WriteBlobString(image,buffer);
    if (image->matte != MagickFalse)
      {
        (void) FormatMagickString(buffer,MaxTextExtent,"/SMask %lu 0 R\n",
          object+7);
        (void) WriteBlobString(image,buffer);
      }
    (void) FormatMagickString(buffer,MaxTextExtent,"/Length %lu 0 R\n",
      object+1);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,">>\n");
    (void) WriteBlobString(image,"stream\n");
    offset=TellBlob(image);
    number_pixels=(MagickSizeType) image->columns*image->rows;
    if ((4*number_pixels) != (MagickSizeType) ((size_t) (4*number_pixels)))
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    if ((compression == FaxCompression) || (compression == Group4Compression) ||
        ((image_info->type != TrueColorType) &&
         (IsGrayImage(image,&image->exception) != MagickFalse)))
      {
        switch (compression)
        {
          case FaxCompression:
          case Group4Compression:
          {
            if (LocaleCompare(CCITTParam,"0") == 0)
              {
                (void) HuffmanEncodeImage(image_info,image,image);
                break;
              }
            (void) Huffman2DEncodeImage(image_info,image,image);
            break;
          }
          case JPEGCompression:
          {
            status=InjectImageBlob(image_info,image,image,"jpeg",
              &image->exception);
            if (status == MagickFalse)
              ThrowWriterException(CoderError,image->exception.reason);
            break;
          }
          case JPEG2000Compression:
          {
            status=InjectImageBlob(image_info,image,image,"jp2",
              &image->exception);
            if (status == MagickFalse)
              ThrowWriterException(CoderError,image->exception.reason);
            break;
          }
          case RLECompression:
          default:
          {
            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            pixels=(unsigned char *) AcquireQuantumMemory(length,
              sizeof(*pixels));
            if (pixels == (unsigned char *) NULL)
              ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
            /*
              Dump Runlength encoded pixels.
            */
            q=pixels;
            for (y=0; y < (long) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,
                &image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                *q++=ScaleQuantumToChar(PixelIntensityToQuantum(p));
                p++;
              }
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (compression == ZipCompression)
              status=ZLIBEncodeImage(image,length,pixels);
            else
#endif
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels);
              else
                status=PackbitsEncodeImage(image,length,pixels);
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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
            for (y=0; y < (long) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,
                &image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                Ascii85Encode(image,
                  ScaleQuantumToChar(PixelIntensityToQuantum(p)));
                p++;
              }
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,y,image->rows);
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
            status=InjectImageBlob(image_info,image,image,"jpeg",
              &image->exception);
            if (status == MagickFalse)
              ThrowWriterException(CoderError,image->exception.reason);
            break;
          }
          case JPEG2000Compression:
          {
            status=InjectImageBlob(image_info,image,image,"jp2",
              &image->exception);
            if (status == MagickFalse)
              ThrowWriterException(CoderError,image->exception.reason);
            break;
          }
          case RLECompression:
          default:
          {
            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            pixels=(unsigned char *) AcquireQuantumMemory(length,
              4*sizeof(*pixels));
            length*=image->colorspace == CMYKColorspace ? 4UL : 3UL;
            if (pixels == (unsigned char *) NULL)
              ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
            /*
              Dump runoffset encoded pixels.
            */
            q=pixels;
            for (y=0; y < (long) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,
                &image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              indexes=GetVirtualIndexQueue(image);
              for (x=0; x < (long) image->columns; x++)
              {
                *q++=ScaleQuantumToChar(p->red);
                *q++=ScaleQuantumToChar(p->green);
                *q++=ScaleQuantumToChar(p->blue);
                if (image->colorspace == CMYKColorspace)
                  *q++=ScaleQuantumToChar(indexes[x]);
                p++;
              }
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,y,image->rows);
                  if (status == MagickFalse)
                    break;
                }
            }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (compression == ZipCompression)
              status=ZLIBEncodeImage(image,length,pixels);
            else
#endif
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels);
              else
                status=PackbitsEncodeImage(image,length,pixels);
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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
              Dump uncompressed DirectColor packets.
            */
            Ascii85Initialize(image);
            for (y=0; y < (long) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,
                &image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              indexes=GetVirtualIndexQueue(image);
              for (x=0; x < (long) image->columns; x++)
              {
                Ascii85Encode(image,ScaleQuantumToChar(p->red));
                Ascii85Encode(image,ScaleQuantumToChar(p->green));
                Ascii85Encode(image,ScaleQuantumToChar(p->blue));
                if (image->colorspace == CMYKColorspace)
                  Ascii85Encode(image,ScaleQuantumToChar(indexes[x]));
                p++;
              }
              if (image->previous == (Image *) NULL)
                {
                  status=SetImageProgress(image,SaveImageTag,y,image->rows);
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
              /*
                Allocate pixel array.
              */
              length=(size_t) number_pixels;
              pixels=(unsigned char *) AcquireQuantumMemory(length,
                sizeof(*pixels));
              if (pixels == (unsigned char *) NULL)
                ThrowWriterException(ResourceLimitError,
                  "MemoryAllocationFailed");
              /*
                Dump Runlength encoded pixels.
              */
              q=pixels;
              for (y=0; y < (long) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,
                  &image->exception);
                if (p == (const PixelPacket *) NULL)
                  break;
                indexes=GetVirtualIndexQueue(image);
                for (x=0; x < (long) image->columns; x++)
                  *q++=(unsigned char) indexes[x];
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,SaveImageTag,y,image->rows);
                    if (status == MagickFalse)
                      break;
                  }
              }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
              if (compression == ZipCompression)
                status=ZLIBEncodeImage(image,length,pixels);
              else
#endif
                if (compression == LZWCompression)
                  status=LZWEncodeImage(image,length,pixels);
                else
                  status=PackbitsEncodeImage(image,length,pixels);
              pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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
              for (y=0; y < (long) image->rows; y++)
              {
                p=GetVirtualPixels(image,0,y,image->columns,1,
                  &image->exception);
                if (p == (const PixelPacket *) NULL)
                  break;
                indexes=GetVirtualIndexQueue(image);
                for (x=0; x < (long) image->columns; x++)
                  Ascii85Encode(image,(unsigned char) indexes[x]);
                if (image->previous == (Image *) NULL)
                  {
                    status=SetImageProgress(image,SaveImageTag,y,image->rows);
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
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu\n",
      (unsigned long) offset);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Colorspace object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    if (image->colorspace == CMYKColorspace)
      (void) CopyMagickString(buffer,"/DeviceCMYK\n",MaxTextExtent);
    else
      if ((compression == FaxCompression) ||
          (compression == Group4Compression) ||
          ((image_info->type != TrueColorType) &&
           (IsGrayImage(image,&image->exception) != MagickFalse)))
          (void) CopyMagickString(buffer,"/DeviceGray\n",MaxTextExtent);
      else
        if ((image->storage_class == DirectClass) || (image->colors > 256) ||
            (compression == JPEGCompression) ||
            (compression == JPEG2000Compression))
          (void) CopyMagickString(buffer,"/DeviceRGB\n",MaxTextExtent);
        else
          (void) FormatMagickString(buffer,MaxTextExtent,
            "[ /Indexed /DeviceRGB %lu %lu 0 R ]\n",
            image->colors-1,object+3);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write Thumb object.
    */
    SetGeometry(image,&geometry);
    (void) ParseMetaGeometry("106x106+0+0>",&geometry.x,&geometry.y,
      &geometry.width,&geometry.height);
    tile_image=ThumbnailImage(image,geometry.width,geometry.height,
      &image->exception);
    if (tile_image == (Image *) NULL)
      ThrowWriterException(ResourceLimitError,image->exception.reason);
    xref[object++]=TellBlob(image);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    switch (compression)
    {
      case NoCompression:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"ASCII85Decode");
        break;
      }
      case JPEGCompression:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"DCTDecode");
        if (image->colorspace != CMYKColorspace)
          break;
        (void) WriteBlobString(image,buffer);
        (void) CopyMagickString(buffer,"/Decode [1 0 1 0 1 0 1 0]\n",
          MaxTextExtent);
        break;
      }
      case JPEG2000Compression:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"JPXDecode");
        if (image->colorspace != CMYKColorspace)
          break;
        (void) WriteBlobString(image,buffer);
        (void) CopyMagickString(buffer,"/Decode [1 0 1 0 1 0 1 0]\n",
          MaxTextExtent);
        break;
      }
      case LZWCompression:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"LZWDecode");
        break;
      }
      case ZipCompression:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"FlateDecode");
        break;
      }
      case FaxCompression:
      case Group4Compression:
      {
        (void) CopyMagickString(buffer,"/Filter [ /CCITTFaxDecode ]\n",
          MaxTextExtent);
        (void) WriteBlobString(image,buffer);
        (void) FormatMagickString(buffer,MaxTextExtent,"/DecodeParms [ << "
          "/K %s /BlackIs1 true /Columns %lu /Rows %lu >> ]\n",CCITTParam,
          tile_image->columns,tile_image->rows);
        break;
      }
      default:
      {
        (void) FormatMagickString(buffer,MaxTextExtent,CFormat,
          "RunLengthDecode");
        break;
      }
    }
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/Width %lu\n",
      tile_image->columns);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/Height %lu\n",
      tile_image->rows);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/ColorSpace %lu 0 R\n",
      object-1);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/BitsPerComponent %d\n",
      (compression == FaxCompression) || (compression == Group4Compression) ?
      1 : 8);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"/Length %lu 0 R\n",
      object+1);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,">>\n");
    (void) WriteBlobString(image,"stream\n");
    offset=TellBlob(image);
    number_pixels=(MagickSizeType) tile_image->columns*tile_image->rows;
    if ((compression == FaxCompression) ||
        (compression == Group4Compression) ||
        ((image_info->type != TrueColorType) &&
         (IsGrayImage(tile_image,&image->exception) != MagickFalse)))
      {
        switch (compression)
        {
          case FaxCompression:
          case Group4Compression:
          {
            if (LocaleCompare(CCITTParam,"0") == 0)
              {
                (void) HuffmanEncodeImage(image_info,image,tile_image);
                break;
              }
            (void) Huffman2DEncodeImage(image_info,image,tile_image);
            break;
          }
          case JPEGCompression:
          {
            status=InjectImageBlob(image_info,image,tile_image,"jpeg",
              &image->exception);
            if (status == MagickFalse)
              ThrowWriterException(CoderError,tile_image->exception.reason);
            break;
          }
          case JPEG2000Compression:
          {
            status=InjectImageBlob(image_info,image,tile_image,"jp2",
              &image->exception);
            if (status == MagickFalse)
              ThrowWriterException(CoderError,tile_image->exception.reason);
            break;
          }
          case RLECompression:
          default:
          {
            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            pixels=(unsigned char *) AcquireQuantumMemory(length,
              sizeof(*pixels));
            if (pixels == (unsigned char *) NULL)
              {
                tile_image=DestroyImage(tile_image);
                ThrowWriterException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            /*
              Dump Runlength encoded pixels.
            */
            q=pixels;
            for (y=0; y < (long) tile_image->rows; y++)
            {
              p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                &tile_image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              for (x=0; x < (long) tile_image->columns; x++)
              {
                *q++=ScaleQuantumToChar(PixelIntensityToQuantum(p));
                p++;
              }
            }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (compression == ZipCompression)
              status=ZLIBEncodeImage(image,length,pixels);
            else
#endif
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels);
              else
                status=PackbitsEncodeImage(image,length,pixels);
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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
            for (y=0; y < (long) tile_image->rows; y++)
            {
              p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                &tile_image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              for (x=0; x < (long) tile_image->columns; x++)
              {
                Ascii85Encode(image,
                  ScaleQuantumToChar(PixelIntensityToQuantum(p)));
                p++;
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
              &image->exception);
            if (status == MagickFalse)
              ThrowWriterException(CoderError,tile_image->exception.reason);
            break;
          }
          case JPEG2000Compression:
          {
            status=InjectImageBlob(image_info,image,tile_image,"jp2",
              &image->exception);
            if (status == MagickFalse)
              ThrowWriterException(CoderError,tile_image->exception.reason);
            break;
          }
          case RLECompression:
          default:
          {
            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            pixels=(unsigned char *) AcquireQuantumMemory(length,4*
              sizeof(*pixels));
            length*=tile_image->colorspace == CMYKColorspace ? 4UL : 3UL;
            if (pixels == (unsigned char *) NULL)
              {
                tile_image=DestroyImage(tile_image);
                ThrowWriterException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            /*
              Dump runoffset encoded pixels.
            */
            q=pixels;
            for (y=0; y < (long) tile_image->rows; y++)
            {
              p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                &tile_image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              indexes=GetVirtualIndexQueue(tile_image);
              for (x=0; x < (long) tile_image->columns; x++)
              {
                *q++=ScaleQuantumToChar(p->red);
                *q++=ScaleQuantumToChar(p->green);
                *q++=ScaleQuantumToChar(p->blue);
                if (image->colorspace == CMYKColorspace)
                  *q++=ScaleQuantumToChar(indexes[x]);
                p++;
              }
            }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (compression == ZipCompression)
              status=ZLIBEncodeImage(image,length,pixels);
            else
#endif
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels);
              else
                status=PackbitsEncodeImage(image,length,pixels);
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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
              Dump uncompressed DirectColor packets.
            */
            Ascii85Initialize(image);
            for (y=0; y < (long) tile_image->rows; y++)
            {
              p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                &tile_image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              indexes=GetVirtualIndexQueue(tile_image);
              for (x=0; x < (long) tile_image->columns; x++)
              {
                Ascii85Encode(image,ScaleQuantumToChar(p->red));
                Ascii85Encode(image,ScaleQuantumToChar(p->green));
                Ascii85Encode(image,ScaleQuantumToChar(p->blue));
                if (image->colorspace == CMYKColorspace)
                  Ascii85Encode(image,ScaleQuantumToChar(indexes[x]));
                p++;
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
              /*
                Allocate pixel array.
              */
              length=(size_t) number_pixels;
              pixels=(unsigned char *) AcquireQuantumMemory(length,
                sizeof(*pixels));
              if (pixels == (unsigned char *) NULL)
                {
                  tile_image=DestroyImage(tile_image);
                  ThrowWriterException(ResourceLimitError,
                    "MemoryAllocationFailed");
                }
              /*
                Dump Runlength encoded pixels.
              */
              q=pixels;
              for (y=0; y < (long) tile_image->rows; y++)
              {
                p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                  &tile_image->exception);
                if (p == (const PixelPacket *) NULL)
                  break;
                indexes=GetVirtualIndexQueue(tile_image);
                for (x=0; x < (long) tile_image->columns; x++)
                  *q++=(unsigned char) indexes[x];
              }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
              if (compression == ZipCompression)
                status=ZLIBEncodeImage(image,length,pixels);
              else
#endif
                if (compression == LZWCompression)
                  status=LZWEncodeImage(image,length,pixels);
                else
                  status=PackbitsEncodeImage(image,length,pixels);
              pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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
              for (y=0; y < (long) tile_image->rows; y++)
              {
                p=GetVirtualPixels(tile_image,0,y,tile_image->columns,1,
                  &tile_image->exception);
                if (p == (const PixelPacket *) NULL)
                  break;
                indexes=GetVirtualIndexQueue(tile_image);
                for (x=0; x < (long) tile_image->columns; x++)
                  Ascii85Encode(image,(unsigned char) indexes[x]);
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
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu\n",
      (unsigned long) offset);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    xref[object++]=TellBlob(image);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    if ((image->storage_class != DirectClass) && (image->colors <= 256) &&
        (compression != FaxCompression) && (compression != Group4Compression))
      {
        /*
          Write Colormap object.
        */
        (void) WriteBlobString(image,"<<\n");
        if (compression == NoCompression)
          (void) WriteBlobString(image,"/Filter [ /ASCII85Decode ]\n");
        (void) FormatMagickString(buffer,MaxTextExtent,"/Length %lu 0 R\n",
          object+1);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,">>\n");
        (void) WriteBlobString(image,"stream\n");
        offset=TellBlob(image);
        if (compression == NoCompression)
          Ascii85Initialize(image);
        for (i=0; i < (long) image->colors; i++)
        {
          if (compression == NoCompression)
            {
              Ascii85Encode(image,ScaleQuantumToChar(image->colormap[i].red));
              Ascii85Encode(image,ScaleQuantumToChar(image->colormap[i].green));
              Ascii85Encode(image,ScaleQuantumToChar(image->colormap[i].blue));
              continue;
            }
          (void) WriteBlobByte(image,
            ScaleQuantumToChar(image->colormap[i].red));
          (void) WriteBlobByte(image,
            ScaleQuantumToChar(image->colormap[i].green));
          (void) WriteBlobByte(image,
            ScaleQuantumToChar(image->colormap[i].blue));
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
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu\n",
      (unsigned long) offset);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    /*
      Write softmask object.
    */
    xref[object++]=TellBlob(image);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"<<\n");
    if (image->matte == MagickFalse)
      (void) WriteBlobString(image,">>\n");
    else
      {
        (void) WriteBlobString(image,"/Type /XObject\n");
        (void) WriteBlobString(image,"/Subtype /Image\n");
        (void) FormatMagickString(buffer,MaxTextExtent,"/Name /Ma%lu\n",
          image->scene);
        (void) WriteBlobString(image,buffer);
        switch (compression)
        {
          case NoCompression:
          {
            (void) FormatMagickString(buffer,MaxTextExtent,CFormat,
              "ASCII85Decode");
            break;
          }
          case LZWCompression:
          {
            (void) FormatMagickString(buffer,MaxTextExtent,CFormat,"LZWDecode");
            break;
          }
          case ZipCompression:
          {
            (void) FormatMagickString(buffer,MaxTextExtent,CFormat,
              "FlateDecode");
            break;
          }
          default:
          {
            (void) FormatMagickString(buffer,MaxTextExtent,CFormat,
              "RunLengthDecode");
            break;
          }
        }
        (void) WriteBlobString(image,buffer);
        (void) FormatMagickString(buffer,MaxTextExtent,"/Width %lu\n",
          image->columns);
        (void) WriteBlobString(image,buffer);
        (void) FormatMagickString(buffer,MaxTextExtent,"/Height %lu\n",
          image->rows);
        (void) WriteBlobString(image,buffer);
        (void) WriteBlobString(image,"/ColorSpace /DeviceGray\n");
        (void) FormatMagickString(buffer,MaxTextExtent,"/BitsPerComponent %d\n",
          (compression == FaxCompression) || (compression == Group4Compression)
          ? 1 : 8);
        (void) WriteBlobString(image,buffer);
        (void) FormatMagickString(buffer,MaxTextExtent,"/Length %lu 0 R\n",
          object+1);
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
            /*
              Allocate pixel array.
            */
            length=(size_t) number_pixels;
            pixels=(unsigned char *) AcquireQuantumMemory(length,
              sizeof(*pixels));
            if (pixels == (unsigned char *) NULL)
              {
                image=DestroyImage(image);
                ThrowWriterException(ResourceLimitError,
                  "MemoryAllocationFailed");
              }
            /*
              Dump Runlength encoded pixels.
            */
            q=pixels;
            for (y=0; y < (long) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,
                &image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                *q++=ScaleQuantumToChar((Quantum) (QuantumRange-p->opacity));
                p++;
              }
            }
#if defined(MAGICKCORE_ZLIB_DELEGATE)
            if (compression == ZipCompression)
              status=ZLIBEncodeImage(image,length,pixels);
            else
#endif
              if (compression == LZWCompression)
                status=LZWEncodeImage(image,length,pixels);
              else
                status=PackbitsEncodeImage(image,length,pixels);
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
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
            for (y=0; y < (long) image->rows; y++)
            {
              p=GetVirtualPixels(image,0,y,image->columns,1,
                &image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                Ascii85Encode(image,ScaleQuantumToChar((Quantum) (QuantumRange-
                  p->opacity)));
                p++;
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
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"%lu\n",(unsigned long)
      offset);
    (void) WriteBlobString(image,buffer);
    (void) WriteBlobString(image,"endobj\n");
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  /*
    Write Metadata object.
  */
  xref[object++]=TellBlob(image);
  info_id=object;
  (void) FormatMagickString(buffer,MaxTextExtent,"%lu 0 obj\n",object);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"<<\n");
  (void) FormatMagickString(buffer,MaxTextExtent,"/Title (%s)\n",
    EscapeParenthesis(image->filename));
  (void) WriteBlobString(image,buffer);
  seconds=time((time_t *) NULL);
#if defined(MAGICKCORE_HAVE_LOCALTIME_R)
  (void) localtime_r(&seconds,&local_time);
#else
  (void) memcpy(&local_time,localtime(&seconds),sizeof(local_time));
#endif
  (void) FormatMagickString(date,MaxTextExtent,"D:%04d%02d%02d%02d%02d%02d",
    local_time.tm_year+1900,local_time.tm_mon+1,local_time.tm_mday,
    local_time.tm_hour,local_time.tm_min,local_time.tm_sec);
  (void) FormatMagickString(buffer,MaxTextExtent,"/CreationDate (%s)\n",date);
  (void) WriteBlobString(image,buffer);
  (void) FormatMagickString(buffer,MaxTextExtent,"/ModDate (%s)\n",date);
  (void) WriteBlobString(image,buffer);
  (void) FormatMagickString(buffer,MaxTextExtent,"/Producer (%s)\n",
    EscapeParenthesis(GetMagickVersion((unsigned long *) NULL)));
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,">>\n");
  (void) WriteBlobString(image,"endobj\n");
  /*
    Write Xref object.
  */
  offset=TellBlob(image)-xref[0]+10;
  (void) WriteBlobString(image,"xref\n");
  (void) FormatMagickString(buffer,MaxTextExtent,"0 %lu\n",object+1);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"0000000000 65535 f \n");
  for (i=0; i < (long) object; i++)
  {
    (void) FormatMagickString(buffer,MaxTextExtent,"%010lu 00000 n \n",
      (unsigned long) xref[i]);
    (void) WriteBlobString(image,buffer);
  }
  (void) WriteBlobString(image,"trailer\n");
  (void) WriteBlobString(image,"<<\n");
  (void) FormatMagickString(buffer,MaxTextExtent,"/Size %lu\n",object+1);
  (void) WriteBlobString(image,buffer);
  (void) FormatMagickString(buffer,MaxTextExtent,"/Info %lu 0 R\n",info_id);
  (void) WriteBlobString(image,buffer);
  (void) FormatMagickString(buffer,MaxTextExtent,"/Root %lu 0 R\n",root_id);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,">>\n");
  (void) WriteBlobString(image,"startxref\n");
  (void) FormatMagickString(buffer,MaxTextExtent,"%lu\n",
    (unsigned long) offset);
  (void) WriteBlobString(image,buffer);
  (void) WriteBlobString(image,"%%EOF\n");
  xref=(MagickOffsetType *) RelinquishMagickMemory(xref);
  (void) CloseBlob(image);
  return(MagickTrue);
}

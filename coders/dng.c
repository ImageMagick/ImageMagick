/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            DDDD   N   N   GGGG                              %
%                            D   D  NN  N  GS                                 %
%                            D   D  N N N  G  GG                              %
%                            D   D  N  NN  G   G                              %
%                            DDDD   N   N   GGGG                              %
%                                                                             %
%                                                                             %
%                  Read the Digital Negative Image Format                     %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1999                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/layer.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D N G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDNGImage() reads an binary file in the Digital Negative format and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadDNGImage method is:
%
%      Image *ReadDNGImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadDNGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  ExceptionInfo
    *sans_exception;

  Image
    *image;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

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
  (void) CloseBlob(image);
  (void) DestroyImageList(image);
  /*
    Convert DNG to PPM with delegate.
  */
  image=AcquireImage(image_info);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) InvokeDelegate(read_info,image,"dng:decode",(char *) NULL,exception);
  image=DestroyImage(image);
  (void) FormatLocaleString(read_info->filename,MaxTextExtent,"%s.png",
    read_info->unique);
  sans_exception=AcquireExceptionInfo();
  image=ReadImage(read_info,sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (image == (Image *) NULL)
    {
      (void) FormatLocaleString(read_info->filename,MaxTextExtent,"%s.ppm",
        read_info->unique);
      image=ReadImage(read_info,exception);
    }
  (void) RelinquishUniqueFileResource(read_info->filename);
  if (image != (Image *) NULL)
    {
      char
        filename[MaxTextExtent],
        *xml;

      ExceptionInfo
        *sans;

      (void) CopyMagickString(image->magick,read_info->magick,MaxTextExtent);
      (void) FormatLocaleString(filename,MaxTextExtent,"%s.ufraw",
        read_info->unique);
      sans=AcquireExceptionInfo();
      xml=FileToString(filename,MaxTextExtent,sans);
      (void) RelinquishUniqueFileResource(filename);
      if (xml != (char *) NULL)
        {
          XMLTreeInfo
            *ufraw;

          /*
            Inject.
          */
          ufraw=NewXMLTree(xml,sans);
          if (ufraw != (XMLTreeInfo *) NULL)
            {
              char
                *content,
                property[MaxTextExtent];

              const char
                *tag;

              XMLTreeInfo
                *next;

              if (image->properties == (void *) NULL)
                ((Image *) image)->properties=NewSplayTree(
                  CompareSplayTreeString,RelinquishMagickMemory,
                  RelinquishMagickMemory);
              next=GetXMLTreeChild(ufraw,(const char *) NULL);
              while (next != (XMLTreeInfo *) NULL)
              {
                tag=GetXMLTreeTag(next);
                if (tag == (char *) NULL)
                  tag="unknown";
                (void) FormatLocaleString(property,MaxTextExtent,"dng:%s",tag);
                content=ConstantString(GetXMLTreeContent(next));
                StripString(content);
                if ((LocaleCompare(tag,"log") != 0) &&
                    (LocaleCompare(tag,"InputFilename") != 0) &&
                    (LocaleCompare(tag,"OutputFilename") != 0) &&
                    (LocaleCompare(tag,"OutputType") != 0) &&
                    (strlen(content) != 0))
                  (void) AddValueToSplayTree((SplayTreeInfo *)
                    ((Image *) image)->properties,ConstantString(property),
                    content);
                next=GetXMLTreeSibling(next);
              }
              ufraw=DestroyXMLTree(ufraw);
            }
          xml=DestroyString(xml);
        }
      sans=DestroyExceptionInfo(sans);
    }
  read_info=DestroyImageInfo(read_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r D N G I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterDNGImage() adds attributes for the DNG image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterDNGImage method is:
%
%      size_t RegisterDNGImage(void)
%
*/
ModuleExport size_t RegisterDNGImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("3FR");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Hasselblad CFV/H3D39II");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("ARW");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Sony Alpha Raw Image Format");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("DNG");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Digital Negative");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("CR2");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Canon Digital Camera Raw Image Format");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("CRW");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Canon Digital Camera Raw Image Format");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("DCR");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Kodak Digital Camera Raw Image File");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("ERF");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Epson RAW Format");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("KDC");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Kodak Digital Camera Raw Image Format");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("K25");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Kodak Digital Camera Raw Image Format");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("MEF");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Mamiya Raw Image File");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("MRW");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Sony (Minolta) Raw Image File");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("NEF");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Nikon Digital SLR Camera Raw Image File");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("NRW");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Nikon Digital SLR Camera Raw Image File");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("ORF");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Olympus Digital Camera Raw Image File");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PEF");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Pentax Electronic File");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("RAF");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Fuji CCD-RAW Graphic File");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("RW2");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Panasonic Lumix Raw Image");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("SRF");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Sony Raw Format");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("SR2");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Sony Raw Format 2");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("X3F");
  entry->decoder=(DecodeImageHandler *) ReadDNGImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->format_type=ExplicitFormatType;
  entry->description=ConstantString("Sigma Camera RAW Picture File");
  entry->module=ConstantString("DNG");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r D N G I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterDNGImage() removes format registrations made by the
%  BIM module from the list of supported formats.
%
%  The format of the UnregisterBIMImage method is:
%
%      UnregisterDNGImage(void)
%
*/
ModuleExport void UnregisterDNGImage(void)
{
  (void) UnregisterMagickInfo("X3F");
  (void) UnregisterMagickInfo("SR2");
  (void) UnregisterMagickInfo("SRF");
  (void) UnregisterMagickInfo("RW2");
  (void) UnregisterMagickInfo("RAF");
  (void) UnregisterMagickInfo("PEF");
  (void) UnregisterMagickInfo("ORF");
  (void) UnregisterMagickInfo("NRW");
  (void) UnregisterMagickInfo("NEF");
  (void) UnregisterMagickInfo("MRW");
  (void) UnregisterMagickInfo("MEF");
  (void) UnregisterMagickInfo("K25");
  (void) UnregisterMagickInfo("KDC");
  (void) UnregisterMagickInfo("DCR");
  (void) UnregisterMagickInfo("CRW");
  (void) UnregisterMagickInfo("CR2");
  (void) UnregisterMagickInfo("DNG");
  (void) UnregisterMagickInfo("ARW");
  (void) UnregisterMagickInfo("3FR");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            TTTTT  TTTTT  FFFFF                              %
%                              T      T    F                                  %
%                              T      T    FFF                                %
%                              T      T    F                                  %
%                              T      T    F                                  %
%                                                                             %
%                                                                             %
%             Return A Preview For A TrueType or Postscript Font              %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/type.h"
#include "MagickCore/utility.h"
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
#include <ft2build.h>
#if defined(FT_FREETYPE_H)
#  include FT_FREETYPE_H
#else
#  include <freetype/freetype.h>
#endif
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P F A                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPFA()() returns MagickTrue if the image format type, identified by the
%  magick string, is PFA.
%
%  The format of the IsPFA method is:
%
%      MagickBooleanType IsPFA(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static MagickBooleanType IsPFA(const unsigned char *magick,const size_t length)
{
  if (length < 14)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"%!PS-AdobeFont",14) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s T T F                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsTTF()() returns MagickTrue if the image format type, identified by the
%  magick string, is TTF.
%
%  The format of the IsTTF method is:
%
%      MagickBooleanType IsTTF(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static MagickBooleanType IsTTF(const unsigned char *magick,const size_t length)
{
  if (length < 5)
    return(MagickFalse);
  if (((int) magick[0] == 0x00) && ((int) magick[1] == 0x01) &&
      ((int) magick[2] == 0x00) && ((int) magick[3] == 0x00) &&
      ((int) magick[4] == 0x00))
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(MAGICKCORE_FREETYPE_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d T T F I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadTTFImage() reads a TrueType font file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadTTFImage method is:
%
%      Image *ReadTTFImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadTTFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent],
    *text;

  const char
    Text[] =
      "abcdefghijklmnopqrstuvwxyz\n"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
      "0123456789.:,;(*!?}^)#${%^&-+@\n";

  const TypeInfo
    *type_info;

  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    status;

  PixelInfo
    background_color;

  register ssize_t
    i,
    x;

  register Quantum
    *q;

  ssize_t
    y;

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
  image->columns=800;
  image->rows=480;
  type_info=GetTypeInfo(image_info->filename,exception);
  if ((type_info != (const TypeInfo *) NULL) &&
      (type_info->glyphs != (char *) NULL))
    (void) CopyMagickString(image->filename,type_info->glyphs,MagickPathExtent);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Color canvas with background color
  */
  background_color=image_info->background_color;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelViaPixelInfo(image,&background_color,q);
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  (void) CopyMagickString(image->magick,image_info->magick,MagickPathExtent);
  (void) CopyMagickString(image->filename,image_info->filename,
    MagickPathExtent);
  /*
    Prepare drawing commands
  */
  y=20;
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  draw_info->font=AcquireString("");
  (void) ImageToFile(image,draw_info->font,exception);
  ConcatenateString(&draw_info->primitive,"push graphic-context\n");
  (void) FormatLocaleString(buffer,MagickPathExtent,
    " viewbox 0 0 %.20g %.20g\n",(double) image->columns,(double) image->rows);
  ConcatenateString(&draw_info->primitive,buffer);
  ConcatenateString(&draw_info->primitive," font-size 18\n");
  (void) FormatLocaleString(buffer,MagickPathExtent," text 10,%.20g '",
    (double) y);
  ConcatenateString(&draw_info->primitive,buffer);
  text=EscapeString(Text,'"');
  ConcatenateString(&draw_info->primitive,text);
  text=DestroyString(text);
  (void) FormatLocaleString(buffer,MagickPathExtent,"'\n");
  ConcatenateString(&draw_info->primitive,buffer);
  y+=20*(ssize_t) MultilineCensus((char *) Text)+20;
  for (i=12; i <= 72; i+=6)
  {
    y+=i+12;
    ConcatenateString(&draw_info->primitive," font-size 18\n");
    (void) FormatLocaleString(buffer,MagickPathExtent,
      " text 10,%.20g '%.20g'\n",(double) y,(double) i);
    ConcatenateString(&draw_info->primitive,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent," font-size %.20g\n",
      (double) i);
    ConcatenateString(&draw_info->primitive,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent," text 50,%.20g "
      "'That which does not destroy me, only makes me stronger.'\n",(double) y);
    ConcatenateString(&draw_info->primitive,buffer);
    if (i >= 24)
      i+=6;
  }
  ConcatenateString(&draw_info->primitive,"pop graphic-context");
  (void) DrawImage(image,draw_info,exception);
  /*
    Relinquish resources.
  */
  (void) RelinquishUniqueFileResource(draw_info->font);  
  draw_info=DestroyDrawInfo(draw_info);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}
#endif /* MAGICKCORE_FREETYPE_DELEGATE */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r T T F I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterTTFImage() adds attributes for the TTF image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterTTFImage method is:
%
%      size_t RegisterTTFImage(void)
%
*/
ModuleExport size_t RegisterTTFImage(void)
{
  char
    version[MagickPathExtent];

  MagickInfo
    *entry;

  *version='\0';
#if defined(FREETYPE_MAJOR) && defined(FREETYPE_MINOR) && defined(FREETYPE_PATCH)
  (void) FormatLocaleString(version,MagickPathExtent,"Freetype %d.%d.%d",
    FREETYPE_MAJOR,FREETYPE_MINOR,FREETYPE_PATCH);
#endif
  entry=AcquireMagickInfo("TTF","DFONT","Multi-face font package");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsTTF;
  entry->flags^=CoderAdjoinFlag;
  if (*version != '\0')
    entry->version=ConstantString(version);
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TTF","PFA","Postscript Type 1 font (ASCII)");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsPFA;
  entry->flags^=CoderAdjoinFlag;
  if (*version != '\0')
    entry->version=ConstantString(version);
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TTF","PFB","Postscript Type 1 font (binary)");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsPFA;
  entry->flags^=CoderAdjoinFlag;
  if (*version != '\0')
    entry->version=ConstantString(version);
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TTF","OTF","Open Type font");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsTTF;
  entry->flags^=CoderAdjoinFlag;
  if (*version != '\0')
    entry->version=ConstantString(version);
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TTF","TTC","TrueType font collection");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsTTF;
  entry->flags^=CoderAdjoinFlag;
  if (*version != '\0')
    entry->version=ConstantString(version);
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("TTF","TTF","TrueType font");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsTTF;
  entry->flags^=CoderAdjoinFlag;
  if (*version != '\0')
    entry->version=ConstantString(version);
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r T T F I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterTTFImage() removes format registrations made by the
%  TTF module from the list of supported formats.
%
%  The format of the UnregisterTTFImage method is:
%
%      UnregisterTTFImage(void)
%
*/
ModuleExport void UnregisterTTFImage(void)
{
  (void) UnregisterMagickInfo("TTF");
  (void) UnregisterMagickInfo("TTC");
  (void) UnregisterMagickInfo("OTF");
  (void) UnregisterMagickInfo("PFA");
  (void) UnregisterMagickInfo("PFB");
  (void) UnregisterMagickInfo("PFA");
  (void) UnregisterMagickInfo("DFONT");
}

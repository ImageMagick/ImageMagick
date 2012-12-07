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
%                                John Cristy                                  %
%                                 July 1992                                   %
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
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/type.h"
#include "wand/MagickWand.h"
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
#if defined(MAGICKCORE_HAVE_FT2BUILD_H)
#  include <ft2build.h>
#endif
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
    buffer[MaxTextExtent],
    *text;

  const char
    *Text = (char *)
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

  PixelPacket
    background_color;

  register ssize_t
    i,
    x;

  register PixelPacket
    *q;

  ssize_t
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
  image->columns=800;
  image->rows=480;
  type_info=GetTypeInfo(image_info->filename,exception);
  if ((type_info != (const TypeInfo *) NULL) &&
      (type_info->glyphs != (char *) NULL))
    (void) CopyMagickString(image->filename,type_info->glyphs,MaxTextExtent);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Color canvas with background color
  */
  background_color=image_info->background_color;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
      *q++=background_color;
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  (void) CopyMagickString(image->magick,image_info->magick,MaxTextExtent);
  (void) CopyMagickString(image->filename,image_info->filename,MaxTextExtent);
  /*
    Prepare drawing commands
  */
  y=20;
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  draw_info->font=AcquireString(image->filename);
  ConcatenateString(&draw_info->primitive,"push graphic-context\n");
  (void) FormatLocaleString(buffer,MaxTextExtent," viewbox 0 0 %.20g %.20g\n",
    (double) image->columns,(double) image->rows);
  ConcatenateString(&draw_info->primitive,buffer);
  ConcatenateString(&draw_info->primitive," font-size 18\n");
  (void) FormatLocaleString(buffer,MaxTextExtent," text 10,%.20g '",(double) y);
  ConcatenateString(&draw_info->primitive,buffer);
  text=EscapeString(Text,'"');
  ConcatenateString(&draw_info->primitive,text);
  text=DestroyString(text);
  (void) FormatLocaleString(buffer,MaxTextExtent,"'\n");
  ConcatenateString(&draw_info->primitive,buffer);
  y+=20*(ssize_t) MultilineCensus((char *) Text)+20;
  for (i=12; i <= 72; i+=6)
  {
    y+=i+12;
    ConcatenateString(&draw_info->primitive," font-size 18\n");
    (void) FormatLocaleString(buffer,MaxTextExtent," text 10,%.20g '%.20g'\n",
      (double) y,(double) i);
    ConcatenateString(&draw_info->primitive,buffer);
    (void) FormatLocaleString(buffer,MaxTextExtent," font-size %.20g\n",
      (double) i);
    ConcatenateString(&draw_info->primitive,buffer);
    (void) FormatLocaleString(buffer,MaxTextExtent," text 50,%.20g "
      "'That which does not destroy me, only makes me stronger.'\n",(double) y);
    ConcatenateString(&draw_info->primitive,buffer);
    if (i >= 24)
      i+=6;
  }
  ConcatenateString(&draw_info->primitive,"pop graphic-context");
  (void) DrawImage(image,draw_info);
  /*
    Relinquish resources.
  */
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
    version[MaxTextExtent];

  MagickInfo
    *entry;

  *version='\0';
#if defined(FREETYPE_MAJOR) && defined(FREETYPE_MINOR) && defined(FREETYPE_PATCH)
  (void) FormatLocaleString(version,MaxTextExtent,"Freetype %d.%d.%d",
    FREETYPE_MAJOR,FREETYPE_MINOR,FREETYPE_PATCH);
#endif
  entry=SetMagickInfo("DFONT");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsTTF;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Multi-face font package");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->module=ConstantString("TTF");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PFA");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsPFA;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Postscript Type 1 font (ASCII)");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->module=ConstantString("TTF");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("PFB");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsPFA;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Postscript Type 1 font (binary)");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->module=ConstantString("TTF");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("OTF");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsTTF;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("Open Type font");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->module=ConstantString("TTF");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("TTC");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsTTF;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("TrueType font collection");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->module=ConstantString("TTF");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("TTF");
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadTTFImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsTTF;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("TrueType font");
  if (*version != '\0')
    entry->version=ConstantString(version);
  entry->module=ConstantString("TTF");
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

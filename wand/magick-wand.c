/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                 M   M   AAA    GGGG  IIIII   CCCC  K   K                    %
%                 MM MM  A   A  G        I    C      K  K                     %
%                 M M M  AAAAA  G GGG    I    C      KKK                      %
%                 M   M  A   A  G   G    I    C      K  K                     %
%                 M   M  A   A   GGGG  IIIII   CCCC  K   K                    %
%                                                                             %
%                        W   W   AAA   N   N  DDDD                            %
%                        W   W  A   A  NN  N  D   D                           %
%                        W W W  AAAAA  N N N  D   D                           %
%                        WW WW  A   A  N  NN  D   D                           %
%                        W   W  A   A  N   N  DDDD                            %
%                                                                             %
%                                                                             %
%                           MagickWand Wand Methods                           %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                 August 2003                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
%
*/

/*
  Include declarations.
*/
#include "wand/studio.h"
#include "wand/MagickWand.h"
#include "wand/magick-wand-private.h"
#include "wand/wand.h"

/*
  Define declarations.
*/
#define ThrowWandException(severity,tag,context) \
{ \
  (void) ThrowMagickException(wand->exception,GetMagickModule(),severity, \
    tag,"`%s'",context); \
  return(MagickFalse); \
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l e a r M a g i c k W a n d                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClearMagickWand() clears resources associated with the wand, leaving the
%  wand blank, and ready to be used for a new set of images.
%
%  The format of the ClearMagickWand method is:
%
%      void ClearMagickWand(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport void ClearMagickWand(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->quantize_info=DestroyQuantizeInfo(wand->quantize_info);
  wand->image_info=DestroyImageInfo(wand->image_info);
  wand->images=DestroyImageList(wand->images);
  wand->image_info=AcquireImageInfo();
  wand->quantize_info=CloneQuantizeInfo((QuantizeInfo *) NULL);
  wand->insert_before=MagickFalse;
  wand->image_pending=MagickFalse;
  ClearMagickException(wand->exception);
  wand->debug=IsEventLogging();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e M a g i c k W a n d                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneMagickWand() makes an exact copy of the specified wand.
%
%  The format of the CloneMagickWand method is:
%
%      MagickWand *CloneMagickWand(const MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *CloneMagickWand(const MagickWand *wand)
{
  MagickWand
    *clone_wand;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  clone_wand=(MagickWand *) AcquireMagickMemory(sizeof(*clone_wand));
  if (clone_wand == (MagickWand *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      wand->name);
  (void) ResetMagickMemory(clone_wand,0,sizeof(*clone_wand));
  clone_wand->id=AcquireWandId();
  (void) FormatLocaleString(clone_wand->name,MaxTextExtent,"%s-%.20g",
    MagickWandId,(double) clone_wand->id);
  clone_wand->exception=AcquireExceptionInfo();
  InheritException(clone_wand->exception,wand->exception);
  clone_wand->image_info=CloneImageInfo(wand->image_info);
  clone_wand->quantize_info=CloneQuantizeInfo(wand->quantize_info);
  clone_wand->images=CloneImageList(wand->images,clone_wand->exception);
  clone_wand->insert_before=MagickFalse;
  clone_wand->image_pending=MagickFalse;
  clone_wand->debug=IsEventLogging();
  if (clone_wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",clone_wand->name);
  clone_wand->signature=WandSignature;
  return(clone_wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y M a g i c k W a n d                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMagickWand() deallocates memory associated with an MagickWand.
%
%  The format of the DestroyMagickWand method is:
%
%      MagickWand *DestroyMagickWand(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *DestroyMagickWand(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->images=DestroyImageList(wand->images);
  if (wand->quantize_info != (QuantizeInfo *) NULL )
    wand->quantize_info=DestroyQuantizeInfo(wand->quantize_info);
  if (wand->image_info != (ImageInfo *) NULL )
    wand->image_info=DestroyImageInfo(wand->image_info);
  if (wand->exception != (ExceptionInfo *) NULL )
    wand->exception=DestroyExceptionInfo(wand->exception);
  RelinquishWandId(wand->id);
  wand->signature=(~WandSignature);
  wand=(MagickWand *) RelinquishMagickMemory(wand);
  return(wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M a g i c k W a n d                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMagickWand() returns MagickTrue if the wand is verified as a magick wand.
%
%  The format of the IsMagickWand method is:
%
%      MagickBooleanType IsMagickWand(const MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType IsMagickWand(const MagickWand *wand)
{
  if (wand == (const MagickWand *) NULL)
    return(MagickFalse);
  if (wand->signature != WandSignature)
    return(MagickFalse);
  if (LocaleNCompare(wand->name,MagickWandId,strlen(MagickWandId)) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C l e a r E x c e p t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickClearException() clears any exceptions associated with the wand.
%
%  The format of the MagickClearException method is:
%
%      MagickBooleanType MagickClearException(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickClearException(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  ClearMagickException(wand->exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t E x c e p t i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetException() returns the severity, reason, and description of any
%  error that occurs when using other methods in this API.
%
%  The format of the MagickGetException method is:
%
%      char *MagickGetException(const MagickWand *wand,ExceptionType *severity)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o severity: the severity of the error is returned here.
%
*/
WandExport char *MagickGetException(const MagickWand *wand,
  ExceptionType *severity)
{
  char
    *description;

  assert(wand != (const MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(severity != (ExceptionType *) NULL);
  *severity=wand->exception->severity;
  description=(char *) AcquireQuantumMemory(2UL*MaxTextExtent,
    sizeof(*description));
  if (description == (char *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "MemoryAllocationFailed","`%s'",wand->name);
      return((char *) NULL);
    }
  *description='\0';
  if (wand->exception->reason != (char *) NULL)
    (void) CopyMagickString(description,GetLocaleExceptionMessage(
      wand->exception->severity,wand->exception->reason),MaxTextExtent);
  if (wand->exception->description != (char *) NULL)
    {
      (void) ConcatenateMagickString(description," (",MaxTextExtent);
      (void) ConcatenateMagickString(description,GetLocaleExceptionMessage(
        wand->exception->severity,wand->exception->description),MaxTextExtent);
      (void) ConcatenateMagickString(description,")",MaxTextExtent);
    }
  return(description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t E x c e p t i o n T y p e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetExceptionType() returns the exception type associated with the
%  wand.  If no exception has occurred, UndefinedExceptionType is returned.
%
%  The format of the MagickGetExceptionType method is:
%
%      ExceptionType MagickGetExceptionType(const MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport ExceptionType MagickGetExceptionType(const MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(wand->exception->severity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I t e r a t o r I n d e x                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetIteratorIndex() returns the position of the iterator in the image
%  list.
%
%  The format of the MagickGetIteratorIndex method is:
%
%      ssize_t MagickGetIteratorIndex(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport ssize_t MagickGetIteratorIndex(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoIterators","`%s'",wand->name);
      return(-1);
    }
  return(GetImageIndexInList(wand->images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k Q u e r y C o n f i g u r e O p t i o n                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickQueryConfigureOption() returns the value associated with the specified
%  configure option.
%
%  The format of the MagickQueryConfigureOption function is:
%
%      char *MagickQueryConfigureOption(const char *option)
%
%  A description of each parameter follows:
%
%    o option: the option name.
%
*/
WandExport char *MagickQueryConfigureOption(const char *option)
{
  char
    *value;

  const ConfigureInfo
    **configure_info;

  ExceptionInfo
    *exception;

  size_t
    number_options;

  exception=AcquireExceptionInfo();
  configure_info=GetConfigureInfoList(option,&number_options,exception);
  exception=DestroyExceptionInfo(exception);
  if (configure_info == (const ConfigureInfo **) NULL)
    return((char *) NULL);
  value=AcquireString(configure_info[0]->value);
  configure_info=(const ConfigureInfo **)
    RelinquishMagickMemory((void *) configure_info);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k Q u e r y C o n f i g u r e O p t i o n s                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickQueryConfigureOptions() returns any configure options that match the
%  specified pattern (e.g.  "*" for all).  Options include NAME, VERSION,
%  LIB_VERSION, etc.
%
%  The format of the MagickQueryConfigureOptions function is:
%
%      char **MagickQueryConfigureOptions(const char *pattern,
%        size_t *number_options)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_options:  Returns the number of configure options in the list.
%
%
*/
WandExport char **MagickQueryConfigureOptions(const char *pattern,
  size_t *number_options)
{
  char
    **options;

  ExceptionInfo
    *exception;

  exception=AcquireExceptionInfo();
  options=GetConfigureList(pattern,number_options,exception);
  exception=DestroyExceptionInfo(exception);
  return(options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k Q u e r y F o n t M e t r i c s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickQueryFontMetrics() returns a 13 element array representing the
%  following font metrics:
%
%    Element Description
%    -------------------------------------------------
%          0 character width
%          1 character height
%          2 ascender
%          3 descender
%          4 text width
%          5 text height
%          6 maximum horizontal advance
%          7 bounding box: x1
%          8 bounding box: y1
%          9 bounding box: x2
%         10 bounding box: y2
%         11 origin: x
%         12 origin: y
%
%  The format of the MagickQueryFontMetrics method is:
%
%      double *MagickQueryFontMetrics(MagickWand *wand,
%        const DrawingWand *drawing_wand,const char *text)
%
%  A description of each parameter follows:
%
%    o wand: the Magick wand.
%
%    o drawing_wand: the drawing wand.
%
%    o text: the text.
%
*/
WandExport double *MagickQueryFontMetrics(MagickWand *wand,
  const DrawingWand *drawing_wand,const char *text)
{
  double
    *font_metrics;

  DrawInfo
    *draw_info;

  MagickBooleanType
    status;

  TypeMetric
    metrics;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(drawing_wand != (const DrawingWand *) NULL);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((double *) NULL);
    }
  font_metrics=(double *) AcquireQuantumMemory(13UL,sizeof(*font_metrics));
  if (font_metrics == (double *) NULL)
    return((double *) NULL);
  draw_info=PeekDrawingWand(drawing_wand);
  if (draw_info == (DrawInfo *) NULL)
    {
      font_metrics=(double *) RelinquishMagickMemory(font_metrics);
      return((double *) NULL);
    }
  (void) CloneString(&draw_info->text,text);
  (void) ResetMagickMemory(&metrics,0,sizeof(metrics));
  status=GetTypeMetrics(wand->images,draw_info,&metrics);
  draw_info=DestroyDrawInfo(draw_info);
  if (status == MagickFalse)
    {
      InheritException(wand->exception,&wand->images->exception);
      font_metrics=(double *) RelinquishMagickMemory(font_metrics);
      return((double *) NULL);
    }
  font_metrics[0]=metrics.pixels_per_em.x;
  font_metrics[1]=metrics.pixels_per_em.y;
  font_metrics[2]=metrics.ascent;
  font_metrics[3]=metrics.descent;
  font_metrics[4]=metrics.width;
  font_metrics[5]=metrics.height;
  font_metrics[6]=metrics.max_advance;
  font_metrics[7]=metrics.bounds.x1;
  font_metrics[8]=metrics.bounds.y1;
  font_metrics[9]=metrics.bounds.x2;
  font_metrics[10]=metrics.bounds.y2;
  font_metrics[11]=metrics.origin.x;
  font_metrics[12]=metrics.origin.y;
  return(font_metrics);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k Q u e r y M u l t i l i n e F o n t M e t r i c s             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickQueryMultilineFontMetrics() returns a 13 element array representing the
%  following font metrics:
%
%    Element Description
%    -------------------------------------------------
%          0 character width
%          1 character height
%          2 ascender
%          3 descender
%          4 text width
%          5 text height
%          6 maximum horizontal advance
%          7 bounding box: x1
%          8 bounding box: y1
%          9 bounding box: x2
%         10 bounding box: y2
%         11 origin: x
%         12 origin: y
%
%  This method is like MagickQueryFontMetrics() but it returns the maximum text
%  width and height for multiple lines of text.
%
%  The format of the MagickQueryFontMetrics method is:
%
%      double *MagickQueryMultilineFontMetrics(MagickWand *wand,
%        const DrawingWand *drawing_wand,const char *text)
%
%  A description of each parameter follows:
%
%    o wand: the Magick wand.
%
%    o drawing_wand: the drawing wand.
%
%    o text: the text.
%
*/
WandExport double *MagickQueryMultilineFontMetrics(MagickWand *wand,
  const DrawingWand *drawing_wand,const char *text)
{
  double
    *font_metrics;

  DrawInfo
    *draw_info;

  MagickBooleanType
    status;

  TypeMetric
    metrics;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(drawing_wand != (const DrawingWand *) NULL);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((double *) NULL);
    }
  font_metrics=(double *) AcquireQuantumMemory(13UL,sizeof(*font_metrics));
  if (font_metrics == (double *) NULL)
    return((double *) NULL);
  draw_info=PeekDrawingWand(drawing_wand);
  if (draw_info == (DrawInfo *) NULL)
    {
      font_metrics=(double *) RelinquishMagickMemory(font_metrics);
      return((double *) NULL);
    }
  (void) CloneString(&draw_info->text,text);
  (void) ResetMagickMemory(&metrics,0,sizeof(metrics));
  status=GetMultilineTypeMetrics(wand->images,draw_info,&metrics);
  draw_info=DestroyDrawInfo(draw_info);
  if (status == MagickFalse)
    {
      InheritException(wand->exception,&wand->images->exception);
      font_metrics=(double *) RelinquishMagickMemory(font_metrics);
      return((double *) NULL);
    }
  font_metrics[0]=metrics.pixels_per_em.x;
  font_metrics[1]=metrics.pixels_per_em.y;
  font_metrics[2]=metrics.ascent;
  font_metrics[3]=metrics.descent;
  font_metrics[4]=metrics.width;
  font_metrics[5]=metrics.height;
  font_metrics[6]=metrics.max_advance;
  font_metrics[7]=metrics.bounds.x1;
  font_metrics[8]=metrics.bounds.y1;
  font_metrics[9]=metrics.bounds.x2;
  font_metrics[10]=metrics.bounds.y2;
  font_metrics[11]=metrics.origin.x;
  font_metrics[12]=metrics.origin.y;
  return(font_metrics);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k Q u e r y F o n t s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickQueryFonts() returns any font that match the specified pattern (e.g.
%  "*" for all).
%
%  The format of the MagickQueryFonts function is:
%
%      char **MagickQueryFonts(const char *pattern,size_t *number_fonts)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_fonts:  Returns the number of fonts in the list.
%
%
*/
WandExport char **MagickQueryFonts(const char *pattern,
  size_t *number_fonts)
{
  char
    **fonts;

  ExceptionInfo
    *exception;

  exception=AcquireExceptionInfo();
  fonts=GetTypeList(pattern,number_fonts,exception);
  exception=DestroyExceptionInfo(exception);
  return(fonts);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k Q u e r y F o r m a t s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickQueryFonts() returns any image formats that match the specified
%  pattern (e.g.  "*" for all).
%
%  The format of the MagickQueryFonts function is:
%
%      char **MagickQueryFonts(const char *pattern,
%        size_t *number_formats)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_formats:  This integer returns the number of image formats in the
%      list.
%
*/
WandExport char **MagickQueryFormats(const char *pattern,
  size_t *number_formats)
{
  char
    **formats;

  ExceptionInfo
    *exception;

  exception=AcquireExceptionInfo();
  formats=GetMagickList(pattern,number_formats,exception);
  exception=DestroyExceptionInfo(exception);
  return(formats);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e l i n q u i s h M e m o r y                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRelinquishMemory() relinquishes memory resources returned by such
%  methods as MagickIdentifyImage(), MagickGetException(), etc.
%
%  The format of the MagickRelinquishMemory method is:
%
%      void *MagickRelinquishMemory(void *resource)
%
%  A description of each parameter follows:
%
%    o resource: Relinquish the memory associated with this resource.
%
*/
WandExport void *MagickRelinquishMemory(void *memory)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  return(RelinquishMagickMemory(memory));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e s e t I t e r a t o r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickResetIterator() resets the wand iterator.
%
%  It is typically used either before iterating though images, or before
%  calling specific functions such as  MagickAppendImages() to append all
%  images together.
%
%  Afterward you can use MagickNextImage() to iterate over all the images
%  in a wand container, starting with the first image.
%
%  Using this before MagickAddImages() or MagickReadImages() will cause
%  new images to be inserted between the first and second image.
%
%  The format of the MagickResetIterator method is:
%
%      void MagickResetIterator(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport void MagickResetIterator(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->images=GetFirstImageInList(wand->images);
  wand->insert_before=MagickFalse; /* Insert/add after current (first) image */
  wand->image_pending=MagickTrue;  /* NextImage will set first image */
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t F i r s t I t e r a t o r                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetFirstIterator() sets the wand iterator to the first image.
%
%  After using any images added to the wand using MagickAddImage() or
%  MagickReadImage() will be prepended before any image in the wand.
%
%  Also the current image has been set to the first image (if any) in the
%  Magick Wand.  Using MagickNextImage() will then set teh current image
%  to the second image in the list (if present).
%
%  This operation is similar to MagickResetIterator() but differs in how
%  MagickAddImage(), MagickReadImage(), and MagickNextImage() behaves
%  afterward.
%
%  The format of the MagickSetFirstIterator method is:
%
%      void MagickSetFirstIterator(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport void MagickSetFirstIterator(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->images=GetFirstImageInList(wand->images);
  wand->insert_before=MagickTrue;   /* Insert/add before the first image */
  wand->image_pending=MagickFalse;  /* NextImage will set next image */
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I t e r a t o r I n d e x                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetIteratorIndex() set the iterator to the given position in the
%  image list specified with the index parameter.  A zero index will set
%  the first image as current, and so on.  Negative indexes can be used
%  to specify an image relative to the end of the images in the wand, with
%  -1 being the last image in the wand.
%
%  If the index is invalid (range too large for number of images in wand)
%  the function will return MagickFalse, but no 'exception' will be raised,
%  as it is not actually an error.  In that case the current image will not
%  change.
%
%  After using any images added to the wand using MagickAddImage() or
%  MagickReadImage() will be added after the image indexed, regardless
%  of if a zero (first image in list) or negative index (from end) is used.
%
%  Jumping to index 0 is similar to MagickResetIterator() but differs in how
%  MagickNextImage() behaves afterward.
%
%  The format of the MagickSetIteratorIndex method is:
%
%      MagickBooleanType MagickSetIteratorIndex(MagickWand *wand,
%        const ssize_t index)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o index: the scene number.
%
*/
WandExport MagickBooleanType MagickSetIteratorIndex(MagickWand *wand,
  const ssize_t index)
{
  Image
    *image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return(MagickFalse);
  image=GetImageFromList(wand->images,index);
  if (image == (Image *) NULL)
    {
      InheritException(wand->exception,&wand->images->exception);
      return(MagickFalse);
    }
  wand->images=image;
  wand->insert_before=MagickFalse;  /* Insert/Add after (this) image */
  wand->image_pending=MagickFalse;  /* NextImage will set next image */
  return(MagickTrue);
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t L a s t I t e r a t o r                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetLastIterator() sets the wand iterator to the last image.
%
%  The last image is actually the current image, and the next use of
%  MagickPreviousImage() will not change this allowing this function to be
%  used to iterate over the images in the reverse direction. In this sense it
%  is more like  MagickResetIterator() than MagickSetFirstIterator().
%
%  Typically this function is used before MagickAddImage(), MagickReadImage()
%  functions to ensure new images are appended to the very end of wand's image
%  list.
%
%  The format of the MagickSetLastIterator method is:
%
%      void MagickSetLastIterator(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport void MagickSetLastIterator(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->images=GetLastImageInList(wand->images);
  wand->insert_before=MagickFalse;  /* Insert/add after current (last) image */
  wand->image_pending=MagickTrue;   /* PreviousImage will return last image */
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W a n d G e n e s i s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWandGenesis() initializes the MagickWand environment.
%
%  The format of the MagickWandGenesis method is:
%
%      void MagickWandGenesis(void)
%
*/
WandExport void MagickWandGenesis(void)
{
  if (IsMagickInstantiated() == MagickFalse)
    MagickCoreGenesis((char *) NULL,MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W a n d T e r m i n u s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWandTerminus() terminates the MagickWand environment.
%
%  The format of the MaickWandTerminus method is:
%
%      void MagickWandTerminus(void)
%
*/
WandExport void MagickWandTerminus(void)
{
  DestroyWandIds();
  MagickCoreTerminus();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w M a g i c k W a n d                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewMagickWand() returns a wand required for all other methods in the API.
%  A fatal exception is thrown if there is not enough memory to allocate the
%  wand.   Use DestroyMagickWand() to dispose of the wand when it is no longer
%  needed.
%
%  The format of the NewMagickWand method is:
%
%      MagickWand *NewMagickWand(void)
%
*/
WandExport MagickWand *NewMagickWand(void)
{
  const char
    *quantum;

  MagickWand
    *wand;

  size_t
    depth;

  depth=MAGICKCORE_QUANTUM_DEPTH;
  quantum=GetMagickQuantumDepth(&depth);
  if (depth != MAGICKCORE_QUANTUM_DEPTH)
    ThrowWandFatalException(WandError,"QuantumDepthMismatch",quantum);
  wand=(MagickWand *) AcquireMagickMemory(sizeof(*wand));
  if (wand == (MagickWand *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  (void) ResetMagickMemory(wand,0,sizeof(*wand));
  wand->id=AcquireWandId();
  (void) FormatLocaleString(wand->name,MaxTextExtent,"%s-%.20g",MagickWandId,
    (double) wand->id);
  wand->images=NewImageList();
  wand->image_info=AcquireImageInfo();
  wand->exception=AcquireExceptionInfo();
  wand->quantize_info=CloneQuantizeInfo((QuantizeInfo *) NULL);
  wand->debug=IsEventLogging();
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  wand->signature=WandSignature;
  return(wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w M a g i c k W a n d F r o m I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewMagickWandFromImage() returns a wand with an image.
%
%  The format of the NewMagickWandFromImage method is:
%
%      MagickWand *NewMagickWandFromImage(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
WandExport MagickWand *NewMagickWandFromImage(const Image *image)
{
  MagickWand
    *wand;

  wand=NewMagickWand();
  wand->images=CloneImage(image,0,0,MagickTrue,wand->exception);
  return(wand);
}

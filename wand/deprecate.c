/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        DDDD   EEEEE  PPPP   RRRR   EEEEE   CCCC   AAA   TTTTT  EEEEE        %
%        D   D  E      P   P  R   R  E      C      A   A    T    E            %
%        D   D  EEE    PPPPP  RRRR   EEE    C      AAAAA    T    EEE          %
%        D   D  E      P      R R    E      C      A   A    T    E            %
%        DDDD   EEEEE  P      R  R   EEEEE   CCCC  A   A    T    EEEEE        %
%                                                                             %
%                                                                             %
%                       MagickWand Deprecated Methods                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                October 2002                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A v e r a g e I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAverageImages() average a set of images.
%
%  The format of the MagickAverageImages method is:
%
%      MagickWand *MagickAverageImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/

static MagickWand *CloneMagickWandFromImages(const MagickWand *wand,
  Image *images)
{
  MagickWand
    *clone_wand;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  clone_wand=(MagickWand *) AcquireAlignedMemory(1,sizeof(*clone_wand));
  if (clone_wand == (MagickWand *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      images->filename);
  (void) ResetMagickMemory(clone_wand,0,sizeof(*clone_wand));
  clone_wand->id=AcquireWandId();
  (void) FormatMagickString(clone_wand->name,MaxTextExtent,"%s-%lu",
    MagickWandId,clone_wand->id);
  clone_wand->exception=AcquireExceptionInfo();
  InheritException(clone_wand->exception,wand->exception);
  clone_wand->image_info=CloneImageInfo(wand->image_info);
  clone_wand->quantize_info=CloneQuantizeInfo(wand->quantize_info);
  clone_wand->images=images;
  clone_wand->debug=IsEventLogging();
  if (clone_wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",clone_wand->name);
  clone_wand->signature=WandSignature;
  return(clone_wand);
}

WandExport MagickWand *MagickAverageImages(MagickWand *wand)
{
  Image
    *average_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  average_image=EvaluateImages(wand->images,MeanEvaluateOperator,
    wand->exception);
  if (average_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,average_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C l i p P a t h I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickClipPathImage() clips along the named paths from the 8BIM profile, if
%  present. Later operations take effect inside the path.  Id may be a number
%  if preceded with #, to work on a numbered path, e.g., "#1" to use the first
%  path.
%
%  The format of the MagickClipPathImage method is:
%
%      MagickBooleanType MagickClipPathImage(MagickWand *wand,
%        const char *pathname,const MagickBooleanType inside)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o pathname: name of clipping path resource. If name is preceded by #, use
%      clipping path numbered by name.
%
%    o inside: if non-zero, later operations take effect inside clipping path.
%      Otherwise later operations take effect outside clipping path.
%
*/
WandExport MagickBooleanType MagickClipPathImage(MagickWand *wand,
  const char *pathname,const MagickBooleanType inside)
{
  return(MagickClipImagePath(wand,pathname,inside));
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w G e t F i l l A l p h a                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawGetFillAlpha() returns the alpha used when drawing using the fill
%  color or fill texture.  Fully opaque is 1.0.
%
%  The format of the DrawGetFillAlpha method is:
%
%      double DrawGetFillAlpha(const DrawingWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
*/
WandExport double DrawGetFillAlpha(const DrawingWand *wand)
{
  return(DrawGetFillOpacity(wand));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w G e t S t r o k e A l p h a                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawGetStrokeAlpha() returns the alpha of stroked object outlines.
%
%  The format of the DrawGetStrokeAlpha method is:
%
%      double DrawGetStrokeAlpha(const DrawingWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
*/
WandExport double DrawGetStrokeAlpha(const DrawingWand *wand)
{
  return(DrawGetStrokeOpacity(wand));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w P e e k G r a p h i c W a n d                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawPeekGraphicWand() returns the current drawing wand.
%
%  The format of the PeekDrawingWand method is:
%
%      DrawInfo *DrawPeekGraphicWand(const DrawingWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
*/
WandExport DrawInfo *DrawPeekGraphicWand(const DrawingWand *wand)
{
  return(PeekDrawingWand(wand));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w P o p G r a p h i c C o n t e x t                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawPopGraphicContext() destroys the current drawing wand and returns to the
%  previously pushed drawing wand. Multiple drawing wands may exist. It is an
%  error to attempt to pop more drawing wands than have been pushed, and it is
%  proper form to pop all drawing wands which have been pushed.
%
%  The format of the DrawPopGraphicContext method is:
%
%      MagickBooleanType DrawPopGraphicContext(DrawingWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
*/
WandExport void DrawPopGraphicContext(DrawingWand *wand)
{
  (void) PopDrawingWand(wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w P u s h G r a p h i c C o n t e x t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawPushGraphicContext() clones the current drawing wand to create a new
%  drawing wand.  The original drawing wand(s) may be returned to by
%  invoking PopDrawingWand().  The drawing wands are stored on a drawing wand
%  stack.  For every Pop there must have already been an equivalent Push.
%
%  The format of the DrawPushGraphicContext method is:
%
%      MagickBooleanType DrawPushGraphicContext(DrawingWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
*/
WandExport void DrawPushGraphicContext(DrawingWand *wand)
{
  (void) PushDrawingWand(wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w S e t F i l l A l p h a                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawSetFillAlpha() sets the alpha to use when drawing using the fill
%  color or fill texture.  Fully opaque is 1.0.
%
%  The format of the DrawSetFillAlpha method is:
%
%      void DrawSetFillAlpha(DrawingWand *wand,const double fill_alpha)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
%    o fill_alpha: fill alpha
%
*/
WandExport void DrawSetFillAlpha(DrawingWand *wand,const double fill_alpha)
{
  DrawSetFillOpacity(wand,fill_alpha);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w S e t S t r o k e A l p h a                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DrawSetStrokeAlpha() specifies the alpha of stroked object outlines.
%
%  The format of the DrawSetStrokeAlpha method is:
%
%      void DrawSetStrokeAlpha(DrawingWand *wand,const double stroke_alpha)
%
%  A description of each parameter follows:
%
%    o wand: the drawing wand.
%
%    o stroke_alpha: stroke alpha.  The value 1.0 is opaque.
%
*/
WandExport void DrawSetStrokeAlpha(DrawingWand *wand,const double stroke_alpha)
{
  DrawSetStrokeOpacity(wand,stroke_alpha);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o l o r F l o o d f i l l I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickColorFloodfillImage() changes the color value of any pixel that matches
%  target and is an immediate neighbor.  If the method FillToBorderMethod is
%  specified, the color value is changed for any neighbor pixel that does not
%  match the bordercolor member of image.
%
%  The format of the MagickColorFloodfillImage method is:
%
%      MagickBooleanType MagickColorFloodfillImage(MagickWand *wand,
%        const PixelWand *fill,const double fuzz,const PixelWand *bordercolor,
%        const long x,const long y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o fill: the floodfill color pixel wand.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
%    o bordercolor: the border color pixel wand.
%
%    o x,y: the starting location of the operation.
%
*/
WandExport MagickBooleanType MagickColorFloodfillImage(MagickWand *wand,
  const PixelWand *fill,const double fuzz,const PixelWand *bordercolor,
  const long x,const long y)
{
  DrawInfo
    *draw_info;

  MagickBooleanType
    status;

  PixelPacket
    target;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  draw_info=CloneDrawInfo(wand->image_info,(DrawInfo *) NULL);
  PixelGetQuantumColor(fill,&draw_info->fill);
  (void) GetOneVirtualPixel(wand->images,x % wand->images->columns,
    y % wand->images->rows,&target,wand->exception);
  if (bordercolor != (PixelWand *) NULL)
    PixelGetQuantumColor(bordercolor,&target);
  wand->images->fuzz=fuzz;
  status=ColorFloodfillImage(wand->images,draw_info,target,x,y,
    bordercolor != (PixelWand *) NULL ? FillToBorderMethod : FloodfillMethod);
  if (status == MagickFalse)
    InheritException(wand->exception,&wand->images->exception);
  draw_info=DestroyDrawInfo(draw_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D e s c r i b e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDescribeImage() identifies an image by printing its attributes to the
%  file.  Attributes include the image width, height, size, and others.
%
%  The format of the MagickDescribeImage method is:
%
%      const char *MagickDescribeImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport char *MagickDescribeImage(MagickWand *wand)
{
  return(MagickIdentifyImage(wand));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k F l a t t e n I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickFlattenImages() merges a sequence of images.  This useful for
%  combining Photoshop layers into a single image.
%
%  The format of the MagickFlattenImages method is:
%
%      MagickWand *MagickFlattenImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickFlattenImages(MagickWand *wand)
{
  Image
    *flatten_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  flatten_image=FlattenImages(wand->images,wand->exception);
  if (flatten_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,flatten_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e A t t r i b u t e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageAttribute() returns a value associated with the specified
%  property.  Use MagickRelinquishMemory() to free the value when you are
%  finished with it.
%
%  The format of the MagickGetImageAttribute method is:
%
%      char *MagickGetImageAttribute(MagickWand *wand,const char *property)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o property: the property.
%
*/
WandExport char *MagickGetImageAttribute(MagickWand *wand,const char *property)
{
  return(MagickGetImageProperty(wand,property));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k G e t I m a g e I n d e x                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageIndex() returns the index of the current image.
%
%  The format of the MagickGetImageIndex method is:
%
%      long MagickGetImageIndex(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport long MagickGetImageIndex(MagickWand *wand)
{
  return(MagickGetIteratorIndex(wand));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k G e t I m a g e C h a n n e l E x t r e m a                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageChannelExtrema() gets the extrema for one or more image
%  channels.
%
%  The format of the MagickGetImageChannelExtrema method is:
%
%      MagickBooleanType MagickGetImageChannelExtrema(MagickWand *wand,
%        const ChannelType channel,unsigned long *minima,unsigned long *maxima)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the image channel(s).
%
%    o minima:  The minimum pixel value for the specified channel(s).
%
%    o maxima:  The maximum pixel value for the specified channel(s).
%
*/
WandExport MagickBooleanType MagickGetImageChannelExtrema(MagickWand *wand,
  const ChannelType channel,unsigned long *minima,unsigned long *maxima)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=GetImageChannelExtrema(wand->images,channel,minima,maxima,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k G e t I m a g e E x t r e m a                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageExtrema() gets the extrema for the image.
%
%  The format of the MagickGetImageExtrema method is:
%
%      MagickBooleanType MagickGetImageExtrema(MagickWand *wand,
%        unsigned long *minima,unsigned long *maxima)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o minima:  The minimum pixel value for the specified channel(s).
%
%    o maxima:  The maximum pixel value for the specified channel(s).
%
*/
WandExport MagickBooleanType MagickGetImageExtrema(MagickWand *wand,
  unsigned long *minima,unsigned long *maxima)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=GetImageExtrema(wand->images,minima,maxima,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e M a t t e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageMatte() returns MagickTrue if the image has a matte channel
%  otherwise MagickFalse.
%
%  The format of the MagickGetImageMatte method is:
%
%      unsigned long MagickGetImageMatte(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickGetImageMatte(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(wand->images->matte);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e P i x e l s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImagePixels() extracts pixel data from an image and returns it to
%  you.  The method returns MagickTrue on success otherwise MagickFalse if an
%  error is encountered.  The data is returned as char, short int, int, long,
%  float, or double in the order specified by map.
%
%  Suppose you want to extract the first scanline of a 640x480 image as
%  character data in red-green-blue order:
%
%      MagickGetImagePixels(wand,0,0,640,1,"RGB",CharPixel,pixels);
%
%  The format of the MagickGetImagePixels method is:
%
%      MagickBooleanType MagickGetImagePixels(MagickWand *wand,
%        const long x,const long y,const unsigned long columns,
%        const unsigned long rows,const char *map,const StorageType storage,
%        void *pixels)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x, y, columns, rows:  These values define the perimeter
%      of a region of pixels you want to extract.
%
%    o map:  This string reflects the expected ordering of the pixel array.
%      It can be any combination or order of R = red, G = green, B = blue,
%      A = alpha (0 is transparent), O = opacity (0 is opaque), C = cyan,
%      Y = yellow, M = magenta, K = black, I = intensity (for grayscale),
%      P = pad.
%
%    o storage: Define the data type of the pixels.  Float and double types are
%      expected to be normalized [0..1] otherwise [0..QuantumRange].  Choose from
%      these types: CharPixel, DoublePixel, FloatPixel, IntegerPixel,
%      LongPixel, QuantumPixel, or ShortPixel.
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
*/
WandExport MagickBooleanType MagickGetImagePixels(MagickWand *wand,
  const long x,const long y,const unsigned long columns,
  const unsigned long rows,const char *map,const StorageType storage,
  void *pixels)
{
  return(MagickExportImagePixels(wand,x,y,columns,rows,map,storage,pixels));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e S i z e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageSize() returns the image length in bytes.
%
%  The format of the MagickGetImageSize method is:
%
%      MagickBooleanType MagickGetImageSize(MagickWand *wand,
%        MagickSizeType *length)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o length: the image length in bytes.
%
*/
WandExport MagickSizeType MagickGetImageSize(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(GetBlobSize(wand->images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M a p I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMapImage() replaces the colors of an image with the closest color
%  from a reference image.
%
%  The format of the MagickMapImage method is:
%
%      MagickBooleanType MagickMapImage(MagickWand *wand,
%        const MagickWand *map_wand,const MagickBooleanType dither)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o map: the map wand.
%
%    o dither: Set this integer value to something other than zero to dither
%      the mapped image.
%
*/
WandExport MagickBooleanType MagickMapImage(MagickWand *wand,
  const MagickWand *map_wand,const MagickBooleanType dither)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) || (map_wand->images == (Image *) NULL))
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=MapImage(wand->images,map_wand->images,dither);
  if (status == MagickFalse)
    InheritException(wand->exception,&wand->images->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M a t t e F l o o d f i l l I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMatteFloodfillImage() changes the transparency value of any pixel that
%  matches target and is an immediate neighbor.  If the method
%  FillToBorderMethod is specified, the transparency value is changed for any
%  neighbor pixel that does not match the bordercolor member of image.
%
%  The format of the MagickMatteFloodfillImage method is:
%
%      MagickBooleanType MagickMatteFloodfillImage(MagickWand *wand,
%        const double alpha,const double fuzz,const PixelWand *bordercolor,
%        const long x,const long y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o alpha: the level of transparency: 1.0 is fully opaque and 0.0 is fully
%      transparent.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
%    o bordercolor: the border color pixel wand.
%
%    o x,y: the starting location of the operation.
%
*/
WandExport MagickBooleanType MagickMatteFloodfillImage(MagickWand *wand,
  const double alpha,const double fuzz,const PixelWand *bordercolor,
  const long x,const long y)
{
  DrawInfo
    *draw_info;

  MagickBooleanType
    status;

  PixelPacket
    target;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  draw_info=CloneDrawInfo(wand->image_info,(DrawInfo *) NULL);
  (void) GetOneVirtualPixel(wand->images,x % wand->images->columns,
    y % wand->images->rows,&target,wand->exception);
  if (bordercolor != (PixelWand *) NULL)
    PixelGetQuantumColor(bordercolor,&target);
  wand->images->fuzz=fuzz;
  status=MatteFloodfillImage(wand->images,target,ClampToQuantum(
    (MagickRealType) QuantumRange-QuantumRange*alpha),x,y,bordercolor !=
    (PixelWand *) NULL ? FillToBorderMethod : FloodfillMethod);
  if (status == MagickFalse)
    InheritException(wand->exception,&wand->images->exception);
  draw_info=DestroyDrawInfo(draw_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M a x i m u m I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMaximumImages() returns the maximum intensity of an image sequence.
%
%  The format of the MagickMaximumImages method is:
%
%      MagickWand *MagickMaximumImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickMaximumImages(MagickWand *wand)
{
  Image
    *maximum_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  maximum_image=EvaluateImages(wand->images,MaxEvaluateOperator,
    wand->exception);
  if (maximum_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,maximum_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M i n i m u m I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMinimumImages() returns the minimum intensity of an image sequence.
%
%  The format of the MagickMinimumImages method is:
%
%      MagickWand *MagickMinimumImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickMinimumImages(MagickWand *wand)
{
  Image
    *minimum_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  minimum_image=EvaluateImages(wand->images,MinEvaluateOperator,
    wand->exception);
  if (minimum_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,minimum_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M o s a i c I m a g e s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMosaicImages() inlays an image sequence to form a single coherent
%  picture.  It returns a wand with each image in the sequence composited at
%  the location defined by the page offset of the image.
%
%  The format of the MagickMosaicImages method is:
%
%      MagickWand *MagickMosaicImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickMosaicImages(MagickWand *wand)
{
  Image
    *mosaic_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  mosaic_image=MosaicImages(wand->images,wand->exception);
  if (mosaic_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,mosaic_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k O p a q u e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickOpaqueImage() changes any pixel that matches color with the color
%  defined by fill.
%
%  The format of the MagickOpaqueImage method is:
%
%      MagickBooleanType MagickOpaqueImage(MagickWand *wand,
%        const PixelWand *target,const PixelWand *fill,const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the channel(s).
%
%    o target: Change this target color to the fill color within the image.
%
%    o fill: the fill pixel wand.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
*/
WandExport MagickBooleanType MagickOpaqueImage(MagickWand *wand,
  const PixelWand *target,const PixelWand *fill,const double fuzz)
{
  return(MagickPaintOpaqueImage(wand,target,fill,fuzz));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P a i n t F l o o d f i l l I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPaintFloodfillImage() changes the color value of any pixel that matches
%  target and is an immediate neighbor.  If the method FillToBorderMethod is
%  specified, the color value is changed for any neighbor pixel that does not
%  match the bordercolor member of image.
%
%  The format of the MagickPaintFloodfillImage method is:
%
%      MagickBooleanType MagickPaintFloodfillImage(MagickWand *wand,
%        const ChannelType channel,const PixelWand *fill,const double fuzz,
%        const PixelWand *bordercolor,const long x,const long y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the channel(s).
%
%    o fill: the floodfill color pixel wand.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
%    o bordercolor: the border color pixel wand.
%
%    o x,y: the starting location of the operation.
%
*/
WandExport MagickBooleanType MagickPaintFloodfillImage(MagickWand *wand,
  const ChannelType channel,const PixelWand *fill,const double fuzz,
  const PixelWand *bordercolor,const long x,const long y)
{
  MagickBooleanType
    status;

  status=MagickFloodfillPaintImage(wand,channel,fill,fuzz,bordercolor,x,y,
    MagickFalse);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P a i n t O p a q u e I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPaintOpaqueImage() changes any pixel that matches color with the color
%  defined by fill.
%
%  The format of the MagickPaintOpaqueImage method is:
%
%      MagickBooleanType MagickPaintOpaqueImage(MagickWand *wand,
%        const PixelWand *target,const PixelWand *fill,const double fuzz)
%      MagickBooleanType MagickPaintOpaqueImageChannel(MagickWand *wand,
%        const ChannelType channel,const PixelWand *target,
%        const PixelWand *fill,const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the channel(s).
%
%    o target: Change this target color to the fill color within the image.
%
%    o fill: the fill pixel wand.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
*/

WandExport MagickBooleanType MagickPaintOpaqueImage(MagickWand *wand,
  const PixelWand *target,const PixelWand *fill,const double fuzz)
{
  return(MagickPaintOpaqueImageChannel(wand,DefaultChannels,target,fill,fuzz));
}

WandExport MagickBooleanType MagickPaintOpaqueImageChannel(MagickWand *wand,
  const ChannelType channel,const PixelWand *target,const PixelWand *fill,
  const double fuzz)
{
  MagickBooleanType
    status;

  status=MagickOpaquePaintImageChannel(wand,channel,target,fill,fuzz,
    MagickFalse);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P a i n t T r a n s p a r e n t I m a g e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPaintTransparentImage() changes any pixel that matches color with the
%  color defined by fill.
%
%  The format of the MagickPaintTransparentImage method is:
%
%      MagickBooleanType MagickPaintTransparentImage(MagickWand *wand,
%        const PixelWand *target,const double alpha,const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o target: Change this target color to specified opacity value within
%      the image.
%
%    o alpha: the level of transparency: 1.0 is fully opaque and 0.0 is fully
%      transparent.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
*/
WandExport MagickBooleanType MagickPaintTransparentImage(MagickWand *wand,
  const PixelWand *target,const double alpha,const double fuzz)
{
  return(MagickTransparentPaintImage(wand,target,alpha,fuzz,MagickFalse));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e A t t r i b u t e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageAttribute() associates a property with an image.
%
%  The format of the MagickSetImageAttribute method is:
%
%      MagickBooleanType MagickSetImageAttribute(MagickWand *wand,
%        const char *property,const char *value)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o property: the property.
%
%    o value: the value.
%
*/
WandExport MagickBooleanType MagickSetImageAttribute(MagickWand *wand,
  const char *property,const char *value)
{
  return(SetImageProperty(wand->images,property,value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e I n d e x                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageIndex() set the current image to the position of the list
%  specified with the index parameter.
%
%  The format of the MagickSetImageIndex method is:
%
%      MagickBooleanType MagickSetImageIndex(MagickWand *wand,const long index)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o index: the scene number.
%
*/
WandExport MagickBooleanType MagickSetImageIndex(MagickWand *wand,
  const long index)
{
  return(MagickSetIteratorIndex(wand,index));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k S e t I m a g e O p t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageOption() associates one or options with a particular image
%  format (.e.g MagickSetImageOption(wand,"jpeg","perserve","yes").
%
%  The format of the MagickSetImageOption method is:
%
%      MagickBooleanType MagickSetImageOption(MagickWand *wand,
%        const char *format,const char *key,const char *value)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o format: the image format.
%
%    o key:  The key.
%
%    o value:  The value.
%
*/
WandExport MagickBooleanType MagickSetImageOption(MagickWand *wand,
  const char *format,const char *key,const char *value)
{
  char
    option[MaxTextExtent];

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  (void) FormatMagickString(option,MaxTextExtent,"%s:%s=%s",format,key,value);
  return(DefineImageOption(wand->image_info,option));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T r a n s p a r e n t I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickTransparentImage() changes any pixel that matches color with the
%  color defined by fill.
%
%  The format of the MagickTransparentImage method is:
%
%      MagickBooleanType MagickTransparentImage(MagickWand *wand,
%        const PixelWand *target,const double alpha,const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o target: Change this target color to specified opacity value within
%      the image.
%
%    o alpha: the level of transparency: 1.0 is fully opaque and 0.0 is fully
%      transparent.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
*/
WandExport MagickBooleanType MagickTransparentImage(MagickWand *wand,
  const PixelWand *target,const double alpha,const double fuzz)
{
  return(MagickPaintTransparentImage(wand,target,alpha,fuzz));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e g i o n O f I n t e r e s t I m a g e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRegionOfInterestImage() extracts a region of the image and returns it
%  as a new wand.
%
%  The format of the MagickRegionOfInterestImage method is:
%
%      MagickWand *MagickRegionOfInterestImage(MagickWand *wand,
%        const unsigned long width,const unsigned long height,const long x,
%        const long y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o width: the region width.
%
%    o height: the region height.
%
%    o x: the region x offset.
%
%    o y: the region y offset.
%
*/
WandExport MagickWand *MagickRegionOfInterestImage(MagickWand *wand,
  const unsigned long width,const unsigned long height,const long x,
  const long y)
{
  return(MagickGetImageRegion(wand,width,height,x,y));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e P i x e l s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImagePixels() accepts pixel datand stores it in the image at the
%  location you specify.  The method returns MagickFalse on success otherwise
%  MagickTrue if an error is encountered.  The pixel data can be either char,
%  short int, int, long, float, or double in the order specified by map.
%
%  Suppose your want to upload the first scanline of a 640x480 image from
%  character data in red-green-blue order:
%
%      MagickSetImagePixels(wand,0,0,640,1,"RGB",CharPixel,pixels);
%
%  The format of the MagickSetImagePixels method is:
%
%      MagickBooleanType MagickSetImagePixels(MagickWand *wand,
%        const long x,const long y,const unsigned long columns,
%        const unsigned long rows,const char *map,const StorageType storage,
%        const void *pixels)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x, y, columns, rows:  These values define the perimeter of a region
%      of pixels you want to define.
%
%    o map:  This string reflects the expected ordering of the pixel array.
%      It can be any combination or order of R = red, G = green, B = blue,
%      A = alpha (0 is transparent), O = opacity (0 is opaque), C = cyan,
%      Y = yellow, M = magenta, K = black, I = intensity (for grayscale),
%      P = pad.
%
%    o storage: Define the data type of the pixels.  Float and double types are
%      expected to be normalized [0..1] otherwise [0..QuantumRange].  Choose from
%      these types: CharPixel, ShortPixel, IntegerPixel, LongPixel, FloatPixel,
%      or DoublePixel.
%
%    o pixels: This array of values contain the pixel components as defined by
%      map and type.  You must preallocate this array where the expected
%      length varies depending on the values of width, height, map, and type.
%
*/
WandExport MagickBooleanType MagickSetImagePixels(MagickWand *wand,
  const long x,const long y,const unsigned long columns,
  const unsigned long rows,const char *map,const StorageType storage,
  const void *pixels)
{
  return(MagickImportImagePixels(wand,x,y,columns,rows,map,storage,pixels));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W r i t e I m a g e B l o b                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWriteImageBlob() implements direct to memory image formats.  It
%  returns the image as a blob and its length.   Use MagickSetFormat() to
%  set the format of the returned blob (GIF, JPEG,  PNG, etc.).
%
%  Use MagickRelinquishMemory() to free the blob when you are done with it.
%
%  The format of the MagickWriteImageBlob method is:
%
%      unsigned char *MagickWriteImageBlob(MagickWand *wand,size_t *length)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o length: the length of the blob.
%
*/
WandExport unsigned char *MagickWriteImageBlob(MagickWand *wand,size_t *length)
{
  return(MagickGetImageBlob(wand,length));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l G e t N e x t R o w                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelGetNextRow() returns the next row as an array of pixel wands from the
%  pixel iterator.
%
%  The format of the PixelGetNextRow method is:
%
%      PixelWand **PixelGetNextRow(PixelIterator *iterator,
%        unsigned long *number_wands)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
%    o number_wands: the number of pixel wands.
%
*/
WandExport PixelWand **PixelGetNextRow(PixelIterator *iterator)
{
  unsigned long
    number_wands;

  return(PixelGetNextIteratorRow(iterator,&number_wands));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P i x e l I t e r a t o r G e t E x c e p t i o n                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelIteratorGetException() returns the severity, reason, and description of
%  any error that occurs when using other methods in this API.
%
%  The format of the PixelIteratorGetException method is:
%
%      char *PixelIteratorGetException(const Pixeliterator *iterator,
%        ExceptionType *severity)
%
%  A description of each parameter follows:
%
%    o iterator: the pixel iterator.
%
%    o severity: the severity of the error is returned here.
%
*/
WandExport char *PixelIteratorGetException(const PixelIterator *iterator,
  ExceptionType *severity)
{
  return(PixelGetIteratorException(iterator,severity));
}
#endif

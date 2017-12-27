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
%                     IIIII  M   M   AAA    GGGG  EEEEE                       %
%                       I    MM MM  A   A  G      E                           %
%                       I    M M M  AAAAA  G  GG  EEE                         %
%                       I    M   M  A   A  G   G  E                           %
%                     IIIII  M   M  A   A   GGGG  EEEEE                       %
%                                                                             %
%                                                                             %
%                          MagickWand Image Methods                           %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                 August 2003                                 %
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
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/magick-wand-private.h"
#include "MagickWand/wand.h"
#include "MagickWand/pixel-wand-private.h"
#include "MagickCore/image-private.h"

/*
  Define declarations.
*/
#define MagickWandId  "MagickWand"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o n e M a g i c k W a n d F r o m I m a g e s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneMagickWandFromImages() clones the magick wand and inserts a new image
%  list.
%
%  The format of the CloneMagickWandFromImages method is:
%
%      MagickWand *CloneMagickWandFromImages(const MagickWand *wand,
%        Image *images)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o images: replace the image list with these image(s).
%
*/
static MagickWand *CloneMagickWandFromImages(const MagickWand *wand,
  Image *images)
{
  MagickWand
    *clone_wand;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  clone_wand=(MagickWand *) AcquireMagickMemory(sizeof(*clone_wand));
  if (clone_wand == (MagickWand *) NULL)
    ThrowWandFatalException(ResourceLimitFatalError,"MemoryAllocationFailed",
      images->filename);
  (void) ResetMagickMemory(clone_wand,0,sizeof(*clone_wand));
  clone_wand->id=AcquireWandId();
  (void) FormatLocaleString(clone_wand->name,MagickPathExtent,"%s-%.20g",
    MagickWandId,(double) clone_wand->id);
  clone_wand->exception=AcquireExceptionInfo();
  InheritException(clone_wand->exception,wand->exception);
  clone_wand->image_info=CloneImageInfo(wand->image_info);
  clone_wand->images=images;
  clone_wand->debug=IsEventLogging();
  clone_wand->signature=MagickWandSignature;
  if (clone_wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",clone_wand->name);
  return(clone_wand);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e F r o m M a g i c k W a n d                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageFromMagickWand() returns the current image from the magick wand.
%
%  The format of the GetImageFromMagickWand method is:
%
%      Image *GetImageFromMagickWand(const MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport Image *GetImageFromMagickWand(const MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((Image *) NULL);
    }
  return(wand->images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A d a p t i v e S h a r p e n I m a g e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAdaptiveBlurImage() adaptively blurs the image by blurring
%  less intensely near image edges and more intensely far from edges. We
%  blur the image with a Gaussian operator of the given radius and standard
%  deviation (sigma).  For reasonable results, radius should be larger than
%  sigma.  Use a radius of 0 and MagickAdaptiveBlurImage() selects a
%  suitable radius for you.
%
%  The format of the MagickAdaptiveBlurImage method is:
%
%      MagickBooleanType MagickAdaptiveBlurImage(MagickWand *wand,
%        const double radius,const double sigma)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
*/
WandExport MagickBooleanType MagickAdaptiveBlurImage(MagickWand *wand,
  const double radius,const double sigma)
{
  Image
    *sharp_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  sharp_image=AdaptiveBlurImage(wand->images,radius,sigma,wand->exception);
  if (sharp_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,sharp_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A d a p t i v e R e s i z e I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAdaptiveResizeImage() adaptively resize image with data dependent
%  triangulation.
%
%      MagickBooleanType MagickAdaptiveResizeImage(MagickWand *wand,
%        const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
*/
WandExport MagickBooleanType MagickAdaptiveResizeImage(MagickWand *wand,
  const size_t columns,const size_t rows)
{
  Image
    *resize_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  resize_image=AdaptiveResizeImage(wand->images,columns,rows,wand->exception);
  if (resize_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,resize_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A d a p t i v e S h a r p e n I m a g e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAdaptiveSharpenImage() adaptively sharpens the image by sharpening
%  more intensely near image edges and less intensely far from edges. We
%  sharpen the image with a Gaussian operator of the given radius and standard
%  deviation (sigma).  For reasonable results, radius should be larger than
%  sigma.  Use a radius of 0 and MagickAdaptiveSharpenImage() selects a
%  suitable radius for you.
%
%  The format of the MagickAdaptiveSharpenImage method is:
%
%      MagickBooleanType MagickAdaptiveSharpenImage(MagickWand *wand,
%        const double radius,const double sigma)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
*/
WandExport MagickBooleanType MagickAdaptiveSharpenImage(MagickWand *wand,
  const double radius,const double sigma)
{
  Image
    *sharp_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  sharp_image=AdaptiveSharpenImage(wand->images,radius,sigma,wand->exception);
  if (sharp_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,sharp_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A d a p t i v e T h r e s h o l d I m a g e                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAdaptiveThresholdImage() selects an individual threshold for each pixel
%  based on the range of intensity values in its local neighborhood.  This
%  allows for thresholding of an image whose global intensity histogram
%  doesn't contain distinctive peaks.
%
%  The format of the AdaptiveThresholdImage method is:
%
%      MagickBooleanType MagickAdaptiveThresholdImage(MagickWand *wand,
%        const size_t width,const size_t height,const double bias)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o width: the width of the local neighborhood.
%
%    o height: the height of the local neighborhood.
%
%    o offset: the mean bias.
%
*/
WandExport MagickBooleanType MagickAdaptiveThresholdImage(MagickWand *wand,
  const size_t width,const size_t height,const double bias)
{
  Image
    *threshold_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  threshold_image=AdaptiveThresholdImage(wand->images,width,height,bias,
    wand->exception);
  if (threshold_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,threshold_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A d d I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAddImage() adds a clone of the images from the second wand and
%  inserts them into the first wand.
%
%  Use MagickSetLastIterator(), to append new images into an existing wand,
%  current image will be set to last image so later adds with also be
%  appened to end of wand.
%
%  Use MagickSetFirstIterator() to prepend new images into wand, any more
%  images added will also be prepended before other images in the wand.
%  However the order of a list of new images will not change.
%
%  Otherwise the new images will be inserted just after the current image,
%  and any later image will also be added after this current image but
%  before the previously added images.  Caution is advised when multiple
%  image adds are inserted into the middle of the wand image list.
%
%  The format of the MagickAddImage method is:
%
%      MagickBooleanType MagickAddImage(MagickWand *wand,
%        const MagickWand *add_wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o add_wand: A wand that contains the image list to be added
%
*/
static inline MagickBooleanType InsertImageInWand(MagickWand *wand,
  Image *images)
{
  if (wand->images == (Image *) NULL)
    {
      /*
        No images in wand, just add them, set current as appropriate.
      */
      if (wand->insert_before != MagickFalse)
        wand->images=GetFirstImageInList(images);
      else
        wand->images=GetLastImageInList(images);
      return(MagickTrue);
    }
  /* user jumped to first image, so prepend new images - remain active */
  if ((wand->insert_before != MagickFalse) &&
       (wand->images->previous == (Image *) NULL))
    {
      PrependImageToList(&wand->images,images);
      wand->images=GetFirstImageInList(images);
      return(MagickTrue);
    }
  /*
    Note you should never have 'insert_before' true when current image is not
    the first image in the wand!  That is no insert before current image, only
    after current image
  */
  if (wand->images->next == (Image *) NULL)
    {
      /*
        At last image, append new images.
      */
      InsertImageInList(&wand->images,images);
      wand->images=GetLastImageInList(images);
      return(MagickTrue);
    }
  /*
    Insert new images, just after the current image.
  */
  InsertImageInList(&wand->images,images);
  return(MagickTrue);
}

WandExport MagickBooleanType MagickAddImage(MagickWand *wand,
  const MagickWand *add_wand)
{
  Image
    *images;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(add_wand != (MagickWand *) NULL);
  assert(add_wand->signature == MagickWandSignature);
  if (add_wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",add_wand->name);
  /*
    Clone images in second wand, and insert into first.
  */
  images=CloneImageList(add_wand->images,wand->exception);
  if (images == (Image *) NULL)
    return(MagickFalse);
  return(InsertImageInWand(wand,images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a g i c k A d d N o i s e I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAddNoiseImage() adds random noise to the image.
%
%  The format of the MagickAddNoiseImage method is:
%
%      MagickBooleanType MagickAddNoiseImage(MagickWand *wand,
%        const NoiseType noise_type,const double attenuate)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o noise_type:  The type of noise: Uniform, Gaussian, Multiplicative,
%      Impulse, Laplacian, or Poisson.
%
%    o attenuate:  attenuate the random distribution.
%
*/
WandExport MagickBooleanType MagickAddNoiseImage(MagickWand *wand,
  const NoiseType noise_type,const double attenuate)
{
  Image
    *noise_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  noise_image=AddNoiseImage(wand->images,noise_type,attenuate,wand->exception);
  if (noise_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,noise_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A f f i n e T r a n s f o r m I m a g e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAffineTransformImage() transforms an image as dictated by the affine
%  matrix of the drawing wand.
%
%  The format of the MagickAffineTransformImage method is:
%
%      MagickBooleanType MagickAffineTransformImage(MagickWand *wand,
%        const DrawingWand *drawing_wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o drawing_wand: the draw wand.
%
*/
WandExport MagickBooleanType MagickAffineTransformImage(MagickWand *wand,
  const DrawingWand *drawing_wand)
{
  DrawInfo
    *draw_info;

  Image
    *affine_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  draw_info=PeekDrawingWand(drawing_wand);
  if (draw_info == (DrawInfo *) NULL)
    return(MagickFalse);
  affine_image=AffineTransformImage(wand->images,&draw_info->affine,
    wand->exception);
  draw_info=DestroyDrawInfo(draw_info);
  if (affine_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,affine_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A n n o t a t e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAnnotateImage() annotates an image with text.
%
%  The format of the MagickAnnotateImage method is:
%
%      MagickBooleanType MagickAnnotateImage(MagickWand *wand,
%        const DrawingWand *drawing_wand,const double x,const double y,
%        const double angle,const char *text)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o drawing_wand: the draw wand.
%
%    o x: x ordinate to left of text
%
%    o y: y ordinate to text baseline
%
%    o angle: rotate text relative to this angle.
%
%    o text: text to draw
%
*/
WandExport MagickBooleanType MagickAnnotateImage(MagickWand *wand,
  const DrawingWand *drawing_wand,const double x,const double y,
  const double angle,const char *text)
{
  char
    geometry[MagickPathExtent];

  DrawInfo
    *draw_info;

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  draw_info=PeekDrawingWand(drawing_wand);
  if (draw_info == (DrawInfo *) NULL)
    return(MagickFalse);
  (void) CloneString(&draw_info->text,text);
  (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",x,y);
  draw_info->affine.sx=cos((double) DegreesToRadians(fmod(angle,360.0)));
  draw_info->affine.rx=sin((double) DegreesToRadians(fmod(angle,360.0)));
  draw_info->affine.ry=(-sin((double) DegreesToRadians(fmod(angle,360.0))));
  draw_info->affine.sy=cos((double) DegreesToRadians(fmod(angle,360.0)));
  (void) CloneString(&draw_info->geometry,geometry);
  status=AnnotateImage(wand->images,draw_info,wand->exception);
  draw_info=DestroyDrawInfo(draw_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A n i m a t e I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAnimateImages() animates an image or image sequence.
%
%  The format of the MagickAnimateImages method is:
%
%      MagickBooleanType MagickAnimateImages(MagickWand *wand,
%        const char *server_name)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o server_name: the X server name.
%
*/
WandExport MagickBooleanType MagickAnimateImages(MagickWand *wand,
  const char *server_name)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  (void) CloneString(&wand->image_info->server_name,server_name);
  status=AnimateImages(wand->image_info,wand->images,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A p p e n d I m a g e s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAppendImages() append the images in a wand from the current image
%  onwards, creating a new wand with the single image result.  This is
%  affected by the gravity and background settings of the first image.
%
%  Typically you would call either MagickResetIterator() or
%  MagickSetFirstImage() before calling this function to ensure that all
%  the images in the wand's image list will be appended together.
%
%  The format of the MagickAppendImages method is:
%
%      MagickWand *MagickAppendImages(MagickWand *wand,
%        const MagickBooleanType stack)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o stack: By default, images are stacked left-to-right. Set stack to
%      MagickTrue to stack them top-to-bottom.
%
*/
WandExport MagickWand *MagickAppendImages(MagickWand *wand,
  const MagickBooleanType stack)
{
  Image
    *append_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  append_image=AppendImages(wand->images,stack,wand->exception);
  if (append_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,append_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A u t o G a m m a I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAutoGammaImage() extracts the 'mean' from the image and adjust the
%  image to try make set its gamma appropriatally.
%
%  The format of the MagickAutoGammaImage method is:
%
%      MagickBooleanType MagickAutoGammaImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickAutoGammaImage(MagickWand *wand)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=AutoGammaImage(wand->images,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A u t o L e v e l I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAutoLevelImage() adjusts the levels of a particular image channel by
%  scaling the minimum and maximum values to the full quantum range.
%
%  The format of the MagickAutoLevelImage method is:
%
%      MagickBooleanType MagickAutoLevelImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickAutoLevelImage(MagickWand *wand)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=AutoLevelImage(wand->images,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k A u t o O r i e n t I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickAutoOrientImage() adjusts an image so that its orientation is suitable
$  for viewing (i.e. top-left orientation).
%
%  The format of the MagickAutoOrientImage method is:
%
%      MagickBooleanType MagickAutoOrientImage(MagickWand *image)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickAutoOrientImage(MagickWand *wand)
{

  Image
    *orient_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  orient_image=AutoOrientImage(wand->images,wand->images->orientation,
    wand->exception);
  if (orient_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,orient_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k B l a c k T h r e s h o l d I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickBlackThresholdImage() is like MagickThresholdImage() but  forces all
%  pixels below the threshold into black while leaving all pixels above the
%  threshold unchanged.
%
%  The format of the MagickBlackThresholdImage method is:
%
%      MagickBooleanType MagickBlackThresholdImage(MagickWand *wand,
%        const PixelWand *threshold)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o threshold: the pixel wand.
%
*/
WandExport MagickBooleanType MagickBlackThresholdImage(MagickWand *wand,
  const PixelWand *threshold)
{
  char
    thresholds[MagickPathExtent];

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  (void) FormatLocaleString(thresholds,MagickPathExtent,
    QuantumFormat "," QuantumFormat "," QuantumFormat "," QuantumFormat,
    PixelGetRedQuantum(threshold),PixelGetGreenQuantum(threshold),
    PixelGetBlueQuantum(threshold),PixelGetAlphaQuantum(threshold));
  status=BlackThresholdImage(wand->images,thresholds,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k B l u e S h i f t I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickBlueShiftImage() mutes the colors of the image to simulate a scene at
%  nighttime in the moonlight.
%
%  The format of the MagickBlueShiftImage method is:
%
%      MagickBooleanType MagickBlueShiftImage(MagickWand *wand,
%        const double factor)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o factor: the blue shift factor (default 1.5)
%
*/
WandExport MagickBooleanType MagickBlueShiftImage(MagickWand *wand,
  const double factor)
{
  Image
    *shift_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  shift_image=BlueShiftImage(wand->images,factor,wand->exception);
  if (shift_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,shift_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k B l u r I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickBlurImage() blurs an image.  We convolve the image with a
%  gaussian operator of the given radius and standard deviation (sigma).
%  For reasonable results, the radius should be larger than sigma.  Use a
%  radius of 0 and BlurImage() selects a suitable radius for you.
%
%  The format of the MagickBlurImage method is:
%
%      MagickBooleanType MagickBlurImage(MagickWand *wand,const double radius,
%        const double sigma)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the , in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the , in pixels.
%
*/
WandExport MagickBooleanType MagickBlurImage(MagickWand *wand,
  const double radius,const double sigma)
{
  Image
    *blur_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  blur_image=BlurImage(wand->images,radius,sigma,wand->exception);
  if (blur_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,blur_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k B o r d e r I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickBorderImage() surrounds the image with a border of the color defined
%  by the bordercolor pixel wand.
%
%  The format of the MagickBorderImage method is:
%
%      MagickBooleanType MagickBorderImage(MagickWand *wand,
%        const PixelWand *bordercolor,const size_t width,
%        const size_t height,const CompositeOperator compose)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o bordercolor: the border color pixel wand.
%
%    o width: the border width.
%
%    o height: the border height.
%
%    o compose: the composite operator.
%
*/
WandExport MagickBooleanType MagickBorderImage(MagickWand *wand,
  const PixelWand *bordercolor,const size_t width,const size_t height,
  const CompositeOperator compose)
{
  Image
    *border_image;

  RectangleInfo
    border_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  border_info.width=width;
  border_info.height=height;
  border_info.x=0;
  border_info.y=0;
  PixelGetQuantumPacket(bordercolor,&wand->images->border_color);
  border_image=BorderImage(wand->images,&border_info,compose,wand->exception);
  if (border_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,border_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k B r i g h t n e s s C o n t r a s t S t r e t c h I m a g e   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Use MagickBrightnessContrastImage() to change the brightness and/or contrast
%  of an image.  It converts the brightness and contrast parameters into slope
%  and intercept and calls a polynomical function to apply to the image.

%
%  The format of the MagickBrightnessContrastImage method is:
%
%      MagickBooleanType MagickBrightnessContrastImage(MagickWand *wand,
%        const double brightness,const double contrast)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o brightness: the brightness percent (-100 .. 100).
%
%    o contrast: the contrast percent (-100 .. 100).
%
*/
WandExport MagickBooleanType MagickBrightnessContrastImage(
  MagickWand *wand,const double brightness,const double contrast)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=BrightnessContrastImage(wand->images,brightness,contrast,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C h a n n e l F x I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickChannelFxImage() applies a channel expression to the specified image.
%  The expression consists of one or more channels, either mnemonic or numeric
%  (e.g. red, 1), separated by actions as follows:
%
%    <=>     exchange two channels (e.g. red<=>blue)
%    =>      transfer a channel to another (e.g. red=>green)
%    ,       separate channel operations (e.g. red, green)
%    |       read channels from next input image (e.g. red | green)
%    ;       write channels to next output image (e.g. red; green; blue)
%
%  A channel without a operation symbol implies extract. For example, to create
%  3 grayscale images from the red, green, and blue channels of an image, use:
%
%    -channel-fx "red; green; blue"
%
%  The format of the MagickChannelFxImage method is:
%
%      MagickWand *MagickChannelFxImage(MagickWand *wand,const char *expression)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o expression: the expression.
%
*/
WandExport MagickWand *MagickChannelFxImage(MagickWand *wand,
  const char *expression)
{
  Image
    *fx_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  fx_image=ChannelFxImage(wand->images,expression,wand->exception);
  if (fx_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,fx_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C h a r c o a l I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCharcoalImage() simulates a charcoal drawing.
%
%  The format of the MagickCharcoalImage method is:
%
%      MagickBooleanType MagickCharcoalImage(MagickWand *wand,
%        const double radius,const double sigma)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
*/
WandExport MagickBooleanType MagickCharcoalImage(MagickWand *wand,
  const double radius,const double sigma)
{
  Image
    *charcoal_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  charcoal_image=CharcoalImage(wand->images,radius,sigma,wand->exception);
  if (charcoal_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,charcoal_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C h o p I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickChopImage() removes a region of an image and collapses the image to
%  occupy the removed portion
%
%  The format of the MagickChopImage method is:
%
%      MagickBooleanType MagickChopImage(MagickWand *wand,
%        const size_t width,const size_t height,const ssize_t x,
%        const ssize_t y)
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
%
*/
WandExport MagickBooleanType MagickChopImage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y)
{
  Image
    *chop_image;

  RectangleInfo
    chop;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  chop.width=width;
  chop.height=height;
  chop.x=x;
  chop.y=y;
  chop_image=ChopImage(wand->images,&chop,wand->exception);
  if (chop_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,chop_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C l a m p I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickClampImage() restricts the color range from 0 to the quantum depth.
%
%  The format of the MagickClampImage method is:
%
%      MagickBooleanType MagickClampImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the channel.
%
*/
WandExport MagickBooleanType MagickClampImage(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(ClampImage(wand->images,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C l i p I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickClipImage() clips along the first path from the 8BIM profile, if
%  present.
%
%  The format of the MagickClipImage method is:
%
%      MagickBooleanType MagickClipImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickClipImage(MagickWand *wand)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=ClipImage(wand->images,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C l i p I m a g e P a t h                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickClipImagePath() clips along the named paths from the 8BIM profile, if
%  present. Later operations take effect inside the path.  Id may be a number
%  if preceded with #, to work on a numbered path, e.g., "#1" to use the first
%  path.
%
%  The format of the MagickClipImagePath method is:
%
%      MagickBooleanType MagickClipImagePath(MagickWand *wand,
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
WandExport MagickBooleanType MagickClipImagePath(MagickWand *wand,
  const char *pathname,const MagickBooleanType inside)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=ClipImagePath(wand->images,pathname,inside,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C l u t I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickClutImage() replaces colors in the image from a color lookup table.
%
%  The format of the MagickClutImage method is:
%
%      MagickBooleanType MagickClutImage(MagickWand *wand,
%        const MagickWand *clut_wand,const PixelInterpolateMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o clut_image: the clut image.
%
%    o method: the pixel interpolation method.
%
*/
WandExport MagickBooleanType MagickClutImage(MagickWand *wand,
  const MagickWand *clut_wand,const PixelInterpolateMethod method)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) || (clut_wand->images == (Image *) NULL))
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=ClutImage(wand->images,clut_wand->images,method,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o a l e s c e I m a g e s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCoalesceImages() composites a set of images while respecting any page
%  offsets and disposal methods.  GIF, MIFF, and MNG animation sequences
%  typically start with an image background and each subsequent image
%  varies in size and offset.  MagickCoalesceImages() returns a new sequence
%  where each image in the sequence is the same size as the first and
%  composited with the next image in the sequence.
%
%  The format of the MagickCoalesceImages method is:
%
%      MagickWand *MagickCoalesceImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickCoalesceImages(MagickWand *wand)
{
  Image
    *coalesce_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  coalesce_image=CoalesceImages(wand->images,wand->exception);
  if (coalesce_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,coalesce_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o l o r D e c i s i o n I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickColorDecisionListImage() accepts a lightweight Color Correction
%  Collection (CCC) file which solely contains one or more color corrections
%  and applies the color correction to the image.  Here is a sample CCC file:
%
%    <ColorCorrectionCollection xmlns="urn:ASC:CDL:v1.2">
%          <ColorCorrection id="cc03345">
%                <SOPNode>
%                     <Slope> 0.9 1.2 0.5 </Slope>
%                     <Offset> 0.4 -0.5 0.6 </Offset>
%                     <Power> 1.0 0.8 1.5 </Power>
%                </SOPNode>
%                <SATNode>
%                     <Saturation> 0.85 </Saturation>
%                </SATNode>
%          </ColorCorrection>
%    </ColorCorrectionCollection>
%
%  which includes the offset, slope, and power for each of the RGB channels
%  as well as the saturation.
%
%  The format of the MagickColorDecisionListImage method is:
%
%      MagickBooleanType MagickColorDecisionListImage(MagickWand *wand,
%        const char *color_correction_collection)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o color_correction_collection: the color correction collection in XML.
%
*/
WandExport MagickBooleanType MagickColorDecisionListImage(MagickWand *wand,
  const char *color_correction_collection)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=ColorDecisionListImage(wand->images,color_correction_collection,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o l o r i z e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickColorizeImage() blends the fill color with each pixel in the image.
%
%  The format of the MagickColorizeImage method is:
%
%      MagickBooleanType MagickColorizeImage(MagickWand *wand,
%        const PixelWand *colorize,const PixelWand *blend)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o colorize: the colorize pixel wand.
%
%    o alpha: the alpha pixel wand.
%
*/
WandExport MagickBooleanType MagickColorizeImage(MagickWand *wand,
  const PixelWand *colorize,const PixelWand *blend)
{
  char
    percent_blend[MagickPathExtent];

  Image
    *colorize_image;

  PixelInfo
    target;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  GetPixelInfo(wand->images,&target);
  if (target.colorspace != CMYKColorspace)
    (void) FormatLocaleString(percent_blend,MagickPathExtent,
      "%g,%g,%g,%g",(double) (100.0*QuantumScale*
      PixelGetRedQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetGreenQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetBlueQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetAlphaQuantum(blend)));
  else
    (void) FormatLocaleString(percent_blend,MagickPathExtent,
      "%g,%g,%g,%g,%g",(double) (100.0*QuantumScale*
      PixelGetRedQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetGreenQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetBlueQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetBlackQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetAlphaQuantum(blend)));
  target=PixelGetPixel(colorize);
  colorize_image=ColorizeImage(wand->images,percent_blend,&target,
    wand->exception);
  if (colorize_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,colorize_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o l o r M a t r i x I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickColorMatrixImage() apply color transformation to an image. The method
%  permits saturation changes, hue rotation, luminance to alpha, and various
%  other effects.  Although variable-sized transformation matrices can be used,
%  typically one uses a 5x5 matrix for an RGBA image and a 6x6 for CMYKA
%  (or RGBA with offsets).  The matrix is similar to those used by Adobe Flash
%  except offsets are in column 6 rather than 5 (in support of CMYKA images)
%  and offsets are normalized (divide Flash offset by 255).
%
%  The format of the MagickColorMatrixImage method is:
%
%      MagickBooleanType MagickColorMatrixImage(MagickWand *wand,
%        const KernelInfo *color_matrix)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o color_matrix:  the color matrix.
%
*/
WandExport MagickBooleanType MagickColorMatrixImage(MagickWand *wand,
  const KernelInfo *color_matrix)
{
  Image
    *color_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (color_matrix == (const KernelInfo *) NULL)
    return(MagickFalse);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  color_image=ColorMatrixImage(wand->images,color_matrix,wand->exception);
  if (color_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,color_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o m b i n e I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCombineImages() combines one or more images into a single image.  The
%  grayscale value of the pixels of each image in the sequence is assigned in
%  order to the specified  hannels of the combined image.   The typical
%  ordering would be image 1 => Red, 2 => Green, 3 => Blue, etc.
%
%  The format of the MagickCombineImages method is:
%
%      MagickWand *MagickCombineImages(MagickWand *wand,
%        const ColorspaceType colorspace)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o colorspace: the colorspace.
%
*/
WandExport MagickWand *MagickCombineImages(MagickWand *wand,
  const ColorspaceType colorspace)
{
  Image
    *combine_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  combine_image=CombineImages(wand->images,colorspace,wand->exception);
  if (combine_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,combine_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o m m e n t I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCommentImage() adds a comment to your image.
%
%  The format of the MagickCommentImage method is:
%
%      MagickBooleanType MagickCommentImage(MagickWand *wand,
%        const char *comment)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o comment: the image comment.
%
*/
WandExport MagickBooleanType MagickCommentImage(MagickWand *wand,
  const char *comment)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=SetImageProperty(wand->images,"comment",comment,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o m p a r e I m a g e L a y e r s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCompareImagesLayers() compares each image with the next in a sequence
%  and returns the maximum bounding region of any pixel differences it
%  discovers.
%
%  The format of the MagickCompareImagesLayers method is:
%
%      MagickWand *MagickCompareImagesLayers(MagickWand *wand,
%        const LayerMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o method: the compare method.
%
*/
WandExport MagickWand *MagickCompareImagesLayers(MagickWand *wand,
  const LayerMethod method)
{
  Image
    *layers_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  layers_image=CompareImagesLayers(wand->images,method,wand->exception);
  if (layers_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,layers_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o m p a r e I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCompareImages() compares an image to a reconstructed image and returns
%  the specified difference image.
%
%  The format of the MagickCompareImages method is:
%
%      MagickWand *MagickCompareImages(MagickWand *wand,
%        const MagickWand *reference,const MetricType metric,
%        double *distortion)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o reference: the reference wand.
%
%    o metric: the metric.
%
%    o distortion: the computed distortion between the images.
%
*/
WandExport MagickWand *MagickCompareImages(MagickWand *wand,
  const MagickWand *reference,const MetricType metric,double *distortion)
{
  Image
    *compare_image;


  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) || (reference->images == (Image *) NULL))
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((MagickWand *) NULL);
    }
  compare_image=CompareImages(wand->images,reference->images,metric,distortion,
    wand->exception);
  if (compare_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,compare_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o m p o s i t e I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCompositeImage() composite one image onto another at the specified
%  offset.
%
%  The format of the MagickCompositeImage method is:
%
%      MagickBooleanType MagickCompositeImage(MagickWand *wand,
%        const MagickWand *source_wand,const CompositeOperator compose,
%        const MagickBooleanType clip_to_self,const ssize_t x,const ssize_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand holding the destination images
%
%    o source_image: the magick wand holding source image.
%
%    o compose: This operator affects how the composite is applied to the
%      image.  The default is Over.  These are some of the compose methods
%      availble.
%
%        OverCompositeOp       InCompositeOp         OutCompositeOp
%        AtopCompositeOp       XorCompositeOp        PlusCompositeOp
%        MinusCompositeOp      AddCompositeOp        SubtractCompositeOp
%        DifferenceCompositeOp BumpmapCompositeOp    CopyCompositeOp
%        DisplaceCompositeOp
%
%    o clip_to_self: set to MagickTrue to limit composition to area composed.
%
%    o x: the column offset of the composited image.
%
%    o y: the row offset of the composited image.
%
*/
WandExport MagickBooleanType MagickCompositeImage(MagickWand *wand,
  const MagickWand *source_wand,const CompositeOperator compose,
  const MagickBooleanType clip_to_self,const ssize_t x,const ssize_t y)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) ||
      (source_wand->images == (Image *) NULL))
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=CompositeImage(wand->images,source_wand->images,compose,clip_to_self,
    x,y,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o m p o s i t e I m a g e G r a v i t y                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCompositeImageGravity() composite one image onto another using the
%  specified gravity.
%
%  The format of the MagickCompositeImageGravity method is:
%
%      MagickBooleanType MagickCompositeImageGravity(MagickWand *wand,
%        const MagickWand *source_wand,const CompositeOperator compose,
%        const GravityType gravity)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand holding the destination images
%
%    o source_image: the magick wand holding source image.
%
%    o compose: This operator affects how the composite is applied to the
%      image.  The default is Over.  These are some of the compose methods
%      availble.
%
%        OverCompositeOp       InCompositeOp         OutCompositeOp
%        AtopCompositeOp       XorCompositeOp        PlusCompositeOp
%        MinusCompositeOp      AddCompositeOp        SubtractCompositeOp
%        DifferenceCompositeOp BumpmapCompositeOp    CopyCompositeOp
%        DisplaceCompositeOp
%
%    o gravity: positioning gravity (NorthWestGravity, NorthGravity,
%               NorthEastGravity, WestGravity, CenterGravity,
%               EastGravity, SouthWestGravity, SouthGravity,
%               SouthEastGravity)
%
*/
WandExport MagickBooleanType MagickCompositeImageGravity(MagickWand *wand,
  const MagickWand *source_wand,const CompositeOperator compose,
  const GravityType gravity)
{
  MagickBooleanType
    status;

  RectangleInfo
    geometry;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) ||
      (source_wand->images == (Image *) NULL))
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  SetGeometry(source_wand->images,&geometry);
  GravityAdjustGeometry(wand->images->columns,wand->images->rows,gravity,
    &geometry);
  status=CompositeImage(wand->images,source_wand->images,compose,MagickTrue,
    geometry.x,geometry.y,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o m p o s i t e L a y e r s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCompositeLayers() composite the images in the source wand over the
%  images in the destination wand in sequence, starting with the current
%  image in both lists.
%
%  Each layer from the two image lists are composted together until the end of
%  one of the image lists is reached.  The offset of each composition is also
%  adjusted to match the virtual canvas offsets of each layer. As such the
%  given offset is relative to the virtual canvas, and not the actual image.
%
%  Composition uses given x and y offsets, as the 'origin' location of the
%  source images virtual canvas (not the real image) allowing you to compose a
%  list of 'layer images' into the destiantioni images.  This makes it well
%  sutiable for directly composing 'Clears Frame Animations' or 'Coaleased
%  Animations' onto a static or other 'Coaleased Animation' destination image
%  list.  GIF disposal handling is not looked at.
%
%  Special case:- If one of the image sequences is the last image (just a
%  single image remaining), that image is repeatally composed with all the
%  images in the other image list.  Either the source or destination lists may
%  be the single image, for this situation.
%
%  In the case of a single destination image (or last image given), that image
%  will ve cloned to match the number of images remaining in the source image
%  list.
%
%  This is equivelent to the "-layer Composite" Shell API operator.
%
%  The format of the MagickCompositeLayers method is:
%
%      MagickBooleanType MagickCompositeLayers(MagickWand *wand,
%        const MagickWand *source_wand, const CompositeOperator compose,
%        const ssize_t x,const ssize_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand holding destaintion images
%
%    o source_wand: the wand holding the source images
%
%    o compose, x, y:  composition arguments
%
*/
WandExport MagickBooleanType MagickCompositeLayers(MagickWand *wand,
  const MagickWand *source_wand,const CompositeOperator compose,
  const ssize_t x,const ssize_t y)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) ||
      (source_wand->images == (Image *) NULL))
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  CompositeLayers(wand->images,compose,source_wand->images,x,y,wand->exception);
  status=MagickTrue;  /* FUTURE: determine status from exceptions */
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o n t r a s t I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickContrastImage() enhances the intensity differences between the lighter
%  and darker elements of the image.  Set sharpen to a value other than 0 to
%  increase the image contrast otherwise the contrast is reduced.
%
%  The format of the MagickContrastImage method is:
%
%      MagickBooleanType MagickContrastImage(MagickWand *wand,
%        const MagickBooleanType sharpen)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o sharpen: Increase or decrease image contrast.
%
%
*/
WandExport MagickBooleanType MagickContrastImage(MagickWand *wand,
  const MagickBooleanType sharpen)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=ContrastImage(wand->images,sharpen,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o n t r a s t S t r e t c h I m a g e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickContrastStretchImage() enhances the contrast of a color image by
%  adjusting the pixels color to span the entire range of colors available.
%  You can also reduce the influence of a particular channel with a gamma
%  value of 0.
%
%  The format of the MagickContrastStretchImage method is:
%
%      MagickBooleanType MagickContrastStretchImage(MagickWand *wand,
%        const double black_point,const double white_point)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o black_point: the black point.
%
%    o white_point: the white point.
%
*/
WandExport MagickBooleanType MagickContrastStretchImage(MagickWand *wand,
  const double black_point,const double white_point)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=ContrastStretchImage(wand->images,black_point,white_point,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o n v o l v e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickConvolveImage() applies a custom convolution kernel to the image.
%
%  The format of the MagickConvolveImage method is:
%
%      MagickBooleanType MagickConvolveImage(MagickWand *wand,
%        const KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o kernel: An array of doubles representing the convolution kernel.
%
*/
WandExport MagickBooleanType MagickConvolveImage(MagickWand *wand,
  const KernelInfo *kernel)
{
  Image
    *filter_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (kernel == (const KernelInfo *) NULL)
    return(MagickFalse);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  filter_image=ConvolveImage(wand->images,kernel,wand->exception);
  if (filter_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,filter_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C r o p I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCropImage() extracts a region of the image.
%
%  The format of the MagickCropImage method is:
%
%      MagickBooleanType MagickCropImage(MagickWand *wand,
%        const size_t width,const size_t height,const ssize_t x,const ssize_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o width: the region width.
%
%    o height: the region height.
%
%    o x: the region x-offset.
%
%    o y: the region y-offset.
%
*/
WandExport MagickBooleanType MagickCropImage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,const ssize_t y)
{
  Image
    *crop_image;

  RectangleInfo
    crop;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  crop.width=width;
  crop.height=height;
  crop.x=x;
  crop.y=y;
  crop_image=CropImage(wand->images,&crop,wand->exception);
  if (crop_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,crop_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C y c l e C o l o r m a p I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCycleColormapImage() displaces an image's colormap by a given number
%  of positions.  If you cycle the colormap a number of times you can produce
%  a psychodelic effect.
%
%  The format of the MagickCycleColormapImage method is:
%
%      MagickBooleanType MagickCycleColormapImage(MagickWand *wand,
%        const ssize_t displace)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o pixel_wand: the pixel wand.
%
*/
WandExport MagickBooleanType MagickCycleColormapImage(MagickWand *wand,
  const ssize_t displace)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=CycleColormapImage(wand->images,displace,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C o n s t i t u t e I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickConstituteImage() adds an image to the wand comprised of the pixel
%  data you supply.  The pixel data must be in scanline order top-to-bottom.
%  The data can be char, short int, int, float, or double.  Float and double
%  require the pixels to be normalized [0..1], otherwise [0..Max],  where Max
%  is the maximum value the type can accomodate (e.g. 255 for char).  For
%  example, to create a 640x480 image from unsigned red-green-blue character
%  data, use
%
%      MagickConstituteImage(wand,640,480,"RGB",CharPixel,pixels);
%
%  The format of the MagickConstituteImage method is:
%
%      MagickBooleanType MagickConstituteImage(MagickWand *wand,
%        const size_t columns,const size_t rows,const char *map,
%        const StorageType storage,void *pixels)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o columns: width in pixels of the image.
%
%    o rows: height in pixels of the image.
%
%    o map:  This string reflects the expected ordering of the pixel array.
%      It can be any combination or order of R = red, G = green, B = blue,
%      A = alpha (0 is transparent), O = alpha (0 is opaque), C = cyan,
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
%
*/
WandExport MagickBooleanType MagickConstituteImage(MagickWand *wand,
  const size_t columns,const size_t rows,const char *map,
  const StorageType storage,const void *pixels)
{
  Image
    *images;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  images=ConstituteImage(columns,rows,map,storage,pixels,wand->exception);
  if (images == (Image *) NULL)
    return(MagickFalse);
  return(InsertImageInWand(wand,images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D e c i p h e r I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDecipherImage() converts cipher pixels to plain pixels.
%
%  The format of the MagickDecipherImage method is:
%
%      MagickBooleanType MagickDecipherImage(MagickWand *wand,
%        const char *passphrase)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o passphrase: the passphrase.
%
*/
WandExport MagickBooleanType MagickDecipherImage(MagickWand *wand,
  const char *passphrase)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(DecipherImage(wand->images,passphrase,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D e c o n s t r u c t I m a g e s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDeconstructImages() compares each image with the next in a sequence
%  and returns the maximum bounding region of any pixel differences it
%  discovers.
%
%  The format of the MagickDeconstructImages method is:
%
%      MagickWand *MagickDeconstructImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickDeconstructImages(MagickWand *wand)
{
  Image
    *deconstruct_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  deconstruct_image=CompareImagesLayers(wand->images,CompareAnyLayer,
    wand->exception);
  if (deconstruct_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,deconstruct_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a g i c k D e s k e w I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDeskewImage() removes skew from the image.  Skew is an artifact that
%  occurs in scanned images because of the camera being misaligned,
%  imperfections in the scanning or surface, or simply because the paper was
%  not placed completely flat when scanned.
%
%  The format of the MagickDeskewImage method is:
%
%      MagickBooleanType MagickDeskewImage(MagickWand *wand,
%        const double threshold)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o threshold: separate background from foreground.
%
*/
WandExport MagickBooleanType MagickDeskewImage(MagickWand *wand,
  const double threshold)
{
  Image
    *sepia_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  sepia_image=DeskewImage(wand->images,threshold,wand->exception);
  if (sepia_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,sepia_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a g i c k D e s p e c k l e I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDespeckleImage() reduces the speckle noise in an image while
%  perserving the edges of the original image.
%
%  The format of the MagickDespeckleImage method is:
%
%      MagickBooleanType MagickDespeckleImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickDespeckleImage(MagickWand *wand)
{
  Image
    *despeckle_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  despeckle_image=DespeckleImage(wand->images,wand->exception);
  if (despeckle_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,despeckle_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D e s t r o y I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDestroyImage() dereferences an image, deallocating memory associated
%  with the image if the reference count becomes zero.
%
%  The format of the MagickDestroyImage method is:
%
%      Image *MagickDestroyImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
WandExport Image *MagickDestroyImage(Image *image)
{
  return(DestroyImage(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D i s p l a y I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDisplayImage() displays an image.
%
%  The format of the MagickDisplayImage method is:
%
%      MagickBooleanType MagickDisplayImage(MagickWand *wand,
%        const char *server_name)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o server_name: the X server name.
%
*/
WandExport MagickBooleanType MagickDisplayImage(MagickWand *wand,
  const char *server_name)
{
  Image
    *image;

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  image=CloneImage(wand->images,0,0,MagickTrue,wand->exception);
  if (image == (Image *) NULL)
    return(MagickFalse);
  (void) CloneString(&wand->image_info->server_name,server_name);
  status=DisplayImages(wand->image_info,image,wand->exception);
  image=DestroyImage(image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D i s p l a y I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDisplayImages() displays an image or image sequence.
%
%  The format of the MagickDisplayImages method is:
%
%      MagickBooleanType MagickDisplayImages(MagickWand *wand,
%        const char *server_name)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o server_name: the X server name.
%
*/
WandExport MagickBooleanType MagickDisplayImages(MagickWand *wand,
  const char *server_name)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  (void) CloneString(&wand->image_info->server_name,server_name);
  status=DisplayImages(wand->image_info,wand->images,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D i s t o r t I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDistortImage() distorts an image using various distortion methods, by
%  mapping color lookups of the source image to a new destination image
%  usally of the same size as the source image, unless 'bestfit' is set to
%  true.
%
%  If 'bestfit' is enabled, and distortion allows it, the destination image is
%  adjusted to ensure the whole source 'image' will just fit within the final
%  destination image, which will be sized and offset accordingly.  Also in
%  many cases the virtual offset of the source image will be taken into
%  account in the mapping.
%
%  The format of the MagickDistortImage method is:
%
%      MagickBooleanType MagickDistortImage(MagickWand *wand,
%        const DistortMethod method,const size_t number_arguments,
%        const double *arguments,const MagickBooleanType bestfit)
%
%  A description of each parameter follows:
%
%    o image: the image to be distorted.
%
%    o method: the method of image distortion.
%
%        ArcDistortion always ignores the source image offset, and always
%        'bestfit' the destination image with the top left corner offset
%        relative to the polar mapping center.
%
%        Bilinear has no simple inverse mapping so it does not allow 'bestfit'
%        style of image distortion.
%
%        Affine, Perspective, and Bilinear, do least squares fitting of the
%        distortion when more than the minimum number of control point pairs
%        are provided.
%
%        Perspective, and Bilinear, falls back to a Affine distortion when less
%        that 4 control point pairs are provided. While Affine distortions let
%        you use any number of control point pairs, that is Zero pairs is a
%        no-Op (viewport only) distrotion, one pair is a translation and two
%        pairs of control points do a scale-rotate-translate, without any
%        shearing.
%
%    o number_arguments: the number of arguments given for this distortion
%      method.
%
%    o arguments: the arguments for this distortion method.
%
%    o bestfit: Attempt to resize destination to fit distorted source.
%
*/
WandExport MagickBooleanType MagickDistortImage(MagickWand *wand,
  const DistortMethod method,const size_t number_arguments,
  const double *arguments,const MagickBooleanType bestfit)
{
  Image
    *distort_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  distort_image=DistortImage(wand->images,method,number_arguments,arguments,
    bestfit,wand->exception);
  if (distort_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,distort_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D r a w I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickDrawImage() renders the drawing wand on the current image.
%
%  The format of the MagickDrawImage method is:
%
%      MagickBooleanType MagickDrawImage(MagickWand *wand,
%        const DrawingWand *drawing_wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o drawing_wand: the draw wand.
%
*/
WandExport MagickBooleanType MagickDrawImage(MagickWand *wand,
  const DrawingWand *drawing_wand)
{
  char
    *primitive;

  DrawInfo
    *draw_info;

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  draw_info=PeekDrawingWand(drawing_wand);
  if ((draw_info == (DrawInfo *) NULL) ||
      (draw_info->primitive == (char *) NULL))
    return(MagickFalse);
  primitive=AcquireString(draw_info->primitive);
  draw_info=DestroyDrawInfo(draw_info);
  draw_info=CloneDrawInfo(wand->image_info,(DrawInfo *) NULL);
  draw_info->primitive=primitive;
  status=DrawImage(wand->images,draw_info,wand->exception);
  draw_info=DestroyDrawInfo(draw_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k E d g e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickEdgeImage() enhance edges within the image with a convolution filter
%  of the given radius.  Use a radius of 0 and Edge() selects a suitable
%  radius for you.
%
%  The format of the MagickEdgeImage method is:
%
%      MagickBooleanType MagickEdgeImage(MagickWand *wand,const double radius)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the pixel neighborhood.
%
*/
WandExport MagickBooleanType MagickEdgeImage(MagickWand *wand,
  const double radius)
{
  Image
    *edge_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  edge_image=EdgeImage(wand->images,radius,wand->exception);
  if (edge_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,edge_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k E m b o s s I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickEmbossImage() returns a grayscale image with a three-dimensional
%  effect.  We convolve the image with a Gaussian operator of the given radius
%  and standard deviation (sigma).  For reasonable results, radius should be
%  larger than sigma.  Use a radius of 0 and Emboss() selects a suitable
%  radius for you.
%
%  The format of the MagickEmbossImage method is:
%
%      MagickBooleanType MagickEmbossImage(MagickWand *wand,const double radius,
%        const double sigma)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
*/
WandExport MagickBooleanType MagickEmbossImage(MagickWand *wand,
  const double radius,const double sigma)
{
  Image
    *emboss_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  emboss_image=EmbossImage(wand->images,radius,sigma,wand->exception);
  if (emboss_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,emboss_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k E n c i p h e r I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickEncipherImage() converts plaint pixels to cipher pixels.
%
%  The format of the MagickEncipherImage method is:
%
%      MagickBooleanType MagickEncipherImage(MagickWand *wand,
%        const char *passphrase)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o passphrase: the passphrase.
%
*/
WandExport MagickBooleanType MagickEncipherImage(MagickWand *wand,
  const char *passphrase)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(EncipherImage(wand->images,passphrase,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k E n h a n c e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickEnhanceImage() applies a digital filter that improves the quality of a
%  noisy image.
%
%  The format of the MagickEnhanceImage method is:
%
%      MagickBooleanType MagickEnhanceImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickEnhanceImage(MagickWand *wand)
{
  Image
    *enhance_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  enhance_image=EnhanceImage(wand->images,wand->exception);
  if (enhance_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,enhance_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k E q u a l i z e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickEqualizeImage() equalizes the image histogram.
%
%  The format of the MagickEqualizeImage method is:
%
%      MagickBooleanType MagickEqualizeImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the image channel(s).
%
*/
WandExport MagickBooleanType MagickEqualizeImage(MagickWand *wand)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=EqualizeImage(wand->images,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k E v a l u a t e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickEvaluateImage() applys an arithmetic, relational, or logical
%  expression to an image.  Use these operators to lighten or darken an image,
%  to increase or decrease contrast in an image, or to produce the "negative"
%  of an image.
%
%  The format of the MagickEvaluateImage method is:
%
%      MagickBooleanType MagickEvaluateImage(MagickWand *wand,
%        const MagickEvaluateOperator operator,const double value)
%      MagickBooleanType MagickEvaluateImages(MagickWand *wand,
%        const MagickEvaluateOperator operator)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o op: A channel operator.
%
%    o value: A value value.
%
*/

WandExport MagickWand *MagickEvaluateImages(MagickWand *wand,
  const MagickEvaluateOperator op)
{
  Image
    *evaluate_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  evaluate_image=EvaluateImages(wand->images,op,wand->exception);
  if (evaluate_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,evaluate_image));
}

WandExport MagickBooleanType MagickEvaluateImage(MagickWand *wand,
  const MagickEvaluateOperator op,const double value)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=EvaluateImage(wand->images,op,value,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k E x p o r t I m a g e P i x e l s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickExportImagePixels() extracts pixel data from an image and returns it
%  to you.  The method returns MagickTrue on success otherwise MagickFalse if
%  an error is encountered.  The data is returned as char, short int, int,
%  ssize_t, float, or double in the order specified by map.
%
%  Suppose you want to extract the first scanline of a 640x480 image as
%  character data in red-green-blue order:
%
%      MagickExportImagePixels(wand,0,0,640,1,"RGB",CharPixel,pixels);
%
%  The format of the MagickExportImagePixels method is:
%
%      MagickBooleanType MagickExportImagePixels(MagickWand *wand,
%        const ssize_t x,const ssize_t y,const size_t columns,
%        const size_t rows,const char *map,const StorageType storage,
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
%      A = alpha (0 is transparent), O = alpha (0 is opaque), C = cyan,
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
WandExport MagickBooleanType MagickExportImagePixels(MagickWand *wand,
  const ssize_t x,const ssize_t y,const size_t columns,
  const size_t rows,const char *map,const StorageType storage,
  void *pixels)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=ExportImagePixels(wand->images,x,y,columns,rows,map,
    storage,pixels,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k E x t e n t I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickExtentImage() extends the image as defined by the geometry, gravity,
%  and wand background color.  Set the (x,y) offset of the geometry to move
%  the original wand relative to the extended wand.
%
%  The format of the MagickExtentImage method is:
%
%      MagickBooleanType MagickExtentImage(MagickWand *wand,const size_t width,
%        const size_t height,const ssize_t x,const ssize_t y)
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
WandExport MagickBooleanType MagickExtentImage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,const ssize_t y)
{
  Image
    *extent_image;

  RectangleInfo
    extent;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  extent.width=width;
  extent.height=height;
  extent.x=x;
  extent.y=y;
  extent_image=ExtentImage(wand->images,&extent,wand->exception);
  if (extent_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,extent_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k F l i p I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickFlipImage() creates a vertical mirror image by reflecting the pixels
%  around the central x-axis.
%
%  The format of the MagickFlipImage method is:
%
%      MagickBooleanType MagickFlipImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickFlipImage(MagickWand *wand)
{
  Image
    *flip_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  flip_image=FlipImage(wand->images,wand->exception);
  if (flip_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,flip_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k F l o o d f i l l P a i n t I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickFloodfillPaintImage() changes the color value of any pixel that matches
%  target and is an immediate neighbor.  If the method FillToBorderMethod is
%  specified, the color value is changed for any neighbor pixel that does not
%  match the bordercolor member of image.
%
%  The format of the MagickFloodfillPaintImage method is:
%
%      MagickBooleanType MagickFloodfillPaintImage(MagickWand *wand,
%        const PixelWand *fill,const double fuzz,const PixelWand *bordercolor,
%        const ssize_t x,const ssize_t y,const MagickBooleanType invert)
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
%    o invert: paint any pixel that does not match the target color.
%
*/
WandExport MagickBooleanType MagickFloodfillPaintImage(MagickWand *wand,
  const PixelWand *fill,const double fuzz,const PixelWand *bordercolor,
  const ssize_t x,const ssize_t y,const MagickBooleanType invert)
{
  DrawInfo
    *draw_info;

  MagickBooleanType
    status;

  PixelInfo
    target;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  draw_info=CloneDrawInfo(wand->image_info,(DrawInfo *) NULL);
  PixelGetQuantumPacket(fill,&draw_info->fill);
  (void) GetOneVirtualPixelInfo(wand->images,TileVirtualPixelMethod,x %
    wand->images->columns,y % wand->images->rows,&target,wand->exception);
  if (bordercolor != (PixelWand *) NULL)
    PixelGetMagickColor(bordercolor,&target);
  wand->images->fuzz=fuzz;
  status=FloodfillPaintImage(wand->images,draw_info,&target,x,y,invert,
    wand->exception);
  draw_info=DestroyDrawInfo(draw_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k F l o p I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickFlopImage() creates a horizontal mirror image by reflecting the pixels
%  around the central y-axis.
%
%  The format of the MagickFlopImage method is:
%
%      MagickBooleanType MagickFlopImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickFlopImage(MagickWand *wand)
{
  Image
    *flop_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  flop_image=FlopImage(wand->images,wand->exception);
  if (flop_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,flop_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k F o u r i e r T r a n s f o r m I m a g e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickForwardFourierTransformImage() implements the discrete Fourier
%  transform (DFT) of the image either as a magnitude / phase or real /
%  imaginary image pair.
%
%  The format of the MagickForwardFourierTransformImage method is:
%
%      MagickBooleanType MagickForwardFourierTransformImage(MagickWand *wand,
%        const MagickBooleanType magnitude)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o magnitude: if true, return as magnitude / phase pair otherwise a real /
%      imaginary image pair.
%
*/
WandExport MagickBooleanType MagickForwardFourierTransformImage(
  MagickWand *wand,const MagickBooleanType magnitude)
{
  Image
    *forward_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  forward_image=ForwardFourierTransformImage(wand->images,magnitude,
    wand->exception);
  if (forward_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,forward_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k F r a m e I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickFrameImage() adds a simulated three-dimensional border around the
%  image.  The width and height specify the border width of the vertical and
%  horizontal sides of the frame.  The inner and outer bevels indicate the
%  width of the inner and outer shadows of the frame.
%
%  The format of the MagickFrameImage method is:
%
%      MagickBooleanType MagickFrameImage(MagickWand *wand,
%        const PixelWand *matte_color,const size_t width,
%        const size_t height,const ssize_t inner_bevel,
%        const ssize_t outer_bevel,const CompositeOperator compose)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o matte_color: the frame color pixel wand.
%
%    o width: the border width.
%
%    o height: the border height.
%
%    o inner_bevel: the inner bevel width.
%
%    o outer_bevel: the outer bevel width.
%
%    o compose: the composite operator.
%
*/
WandExport MagickBooleanType MagickFrameImage(MagickWand *wand,
  const PixelWand *matte_color,const size_t width,const size_t height,
  const ssize_t inner_bevel,const ssize_t outer_bevel,
  const CompositeOperator compose)
{
  Image
    *frame_image;

  FrameInfo
    frame_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  (void) ResetMagickMemory(&frame_info,0,sizeof(frame_info));
  frame_info.width=wand->images->columns+2*width;
  frame_info.height=wand->images->rows+2*height;
  frame_info.x=(ssize_t) width;
  frame_info.y=(ssize_t) height;
  frame_info.inner_bevel=inner_bevel;
  frame_info.outer_bevel=outer_bevel;
  PixelGetQuantumPacket(matte_color,&wand->images->matte_color);
  frame_image=FrameImage(wand->images,&frame_info,compose,wand->exception);
  if (frame_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,frame_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k F u n c t i o n I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickFunctionImage() applys an arithmetic, relational, or logical
%  expression to an image.  Use these operators to lighten or darken an image,
%  to increase or decrease contrast in an image, or to produce the "negative"
%  of an image.
%
%  The format of the MagickFunctionImage method is:
%
%      MagickBooleanType MagickFunctionImage(MagickWand *wand,
%        const MagickFunction function,const size_t number_arguments,
%        const double *arguments)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o function: the image function.
%
%    o number_arguments: the number of function arguments.
%
%    o arguments: the function arguments.
%
*/
WandExport MagickBooleanType MagickFunctionImage(MagickWand *wand,
  const MagickFunction function,const size_t number_arguments,
  const double *arguments)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=FunctionImage(wand->images,function,number_arguments,arguments,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k F x I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickFxImage() evaluate expression for each pixel in the image.
%
%  The format of the MagickFxImage method is:
%
%      MagickWand *MagickFxImage(MagickWand *wand,const char *expression)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o expression: the expression.
%
*/
WandExport MagickWand *MagickFxImage(MagickWand *wand,const char *expression)
{
  Image
    *fx_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  fx_image=FxImage(wand->images,expression,wand->exception);
  if (fx_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,fx_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G a m m a I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGammaImage() gamma-corrects an image.  The same image viewed on
%  different devices will have perceptual differences in the way the image's
%  intensities are represented on the screen.  Specify individual gamma levels
%  for the red, green, and blue channels, or adjust all three with the gamma
%  parameter.  Values typically range from 0.8 to 2.3.
%
%  You can also reduce the influence of a particular channel with a gamma
%  value of 0.
%
%  The format of the MagickGammaImage method is:
%
%      MagickBooleanType MagickGammaImage(MagickWand *wand,const double gamma)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o level: Define the level of gamma correction.
%
*/
WandExport MagickBooleanType MagickGammaImage(MagickWand *wand,
  const double gamma)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=GammaImage(wand->images,gamma,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G a u s s i a n B l u r I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGaussianBlurImage() blurs an image.  We convolve the image with a
%  Gaussian operator of the given radius and standard deviation (sigma).
%  For reasonable results, the radius should be larger than sigma.  Use a
%  radius of 0 and MagickGaussianBlurImage() selects a suitable radius for you.
%
%  The format of the MagickGaussianBlurImage method is:
%
%      MagickBooleanType MagickGaussianBlurImage(MagickWand *wand,
%        const double radius,const double sigma)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
*/
WandExport MagickBooleanType MagickGaussianBlurImage(MagickWand *wand,
  const double radius,const double sigma)
{
  Image
    *blur_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  blur_image=GaussianBlurImage(wand->images,radius,sigma,wand->exception);
  if (blur_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,blur_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImage() gets the image at the current image index.
%
%  The format of the MagickGetImage method is:
%
%      MagickWand *MagickGetImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickGetImage(MagickWand *wand)
{
  Image
    *image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((MagickWand *) NULL);
    }
  image=CloneImage(wand->images,0,0,MagickTrue,wand->exception);
  if (image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e A l p h a C h a n n e l                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageAlphaChannel() returns MagickFalse if the image alpha channel
%  is not activated.  That is, the image is RGB rather than RGBA or CMYK rather
%  than CMYKA.
%
%  The format of the MagickGetImageAlphaChannel method is:
%
%      MagickBooleanType MagickGetImageAlphaChannel(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickGetImageAlphaChannel(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(GetImageAlphaChannel(wand->images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e C l i p M a s k                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageMask() gets the image clip mask at the current image index.
%
%  The format of the MagickGetImageMask method is:
%
%      MagickWand *MagickGetImageMask(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o type: type of mask, ReadPixelMask or WritePixelMask.
%
*/
WandExport MagickWand *MagickGetImageMask(MagickWand *wand,
  const PixelMask type)
{
  Image
    *image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((MagickWand *) NULL);
    }
  image=GetImageMask(wand->images,type,wand->exception);
  if (image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e B a c k g r o u n d C o l o r                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageBackgroundColor() returns the image background color.
%
%  The format of the MagickGetImageBackgroundColor method is:
%
%      MagickBooleanType MagickGetImageBackgroundColor(MagickWand *wand,
%        PixelWand *background_color)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o background_color: Return the background color.
%
*/
WandExport MagickBooleanType MagickGetImageBackgroundColor(MagickWand *wand,
  PixelWand *background_color)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  PixelSetPixelColor(background_color,&wand->images->background_color);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e B l o b                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageBlob() implements direct to memory image formats.  It returns
%  the image as a blob (a formatted "file" in memory) and its length, starting
%  from the current position in the image sequence.  Use MagickSetImageFormat()
%  to set the format to write to the blob (GIF, JPEG,  PNG, etc.).
%
%  Utilize MagickResetIterator() to ensure the write is from the beginning of
%  the image sequence.
%
%  Use MagickRelinquishMemory() to free the blob when you are done with it.
%
%  The format of the MagickGetImageBlob method is:
%
%      unsigned char *MagickGetImageBlob(MagickWand *wand,size_t *length)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o length: the length of the blob.
%
*/
WandExport unsigned char *MagickGetImageBlob(MagickWand *wand,size_t *length)
{
  unsigned char
    *blob;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((unsigned char *) NULL);
    }
  blob=(unsigned char *) ImageToBlob(wand->image_info,wand->images,length,
    wand->exception);
  return(blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e s B l o b                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageBlob() implements direct to memory image formats.  It
%  returns the image sequence as a blob and its length.  The format of the image
%  determines the format of the returned blob (GIF, JPEG,  PNG, etc.).  To
%  return a different image format, use MagickSetImageFormat().
%
%  Note, some image formats do not permit multiple images to the same image
%  stream (e.g. JPEG).  in this instance, just the first image of the
%  sequence is returned as a blob.
%
%  The format of the MagickGetImagesBlob method is:
%
%      unsigned char *MagickGetImagesBlob(MagickWand *wand,size_t *length)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o length: the length of the blob.
%
*/
WandExport unsigned char *MagickGetImagesBlob(MagickWand *wand,size_t *length)
{
  unsigned char
    *blob;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((unsigned char *) NULL);
    }
  blob=(unsigned char *) ImagesToBlob(wand->image_info,GetFirstImageInList(
    wand->images),length,wand->exception);
  return(blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e B l u e P r i m a r y                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageBluePrimary() returns the chromaticy blue primary point for the
%  image.
%
%  The format of the MagickGetImageBluePrimary method is:
%
%      MagickBooleanType MagickGetImageBluePrimary(MagickWand *wand,double *x,
%        double *y,double *z)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x: the chromaticity blue primary x-point.
%
%    o y: the chromaticity blue primary y-point.
%
%    o z: the chromaticity blue primary z-point.
%
*/
WandExport MagickBooleanType MagickGetImageBluePrimary(MagickWand *wand,
  double *x,double *y,double *z)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  *x=wand->images->chromaticity.blue_primary.x;
  *y=wand->images->chromaticity.blue_primary.y;
  *z=wand->images->chromaticity.blue_primary.z;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e B o r d e r C o l o r                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageBorderColor() returns the image border color.
%
%  The format of the MagickGetImageBorderColor method is:
%
%      MagickBooleanType MagickGetImageBorderColor(MagickWand *wand,
%        PixelWand *border_color)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o border_color: Return the border color.
%
*/
WandExport MagickBooleanType MagickGetImageBorderColor(MagickWand *wand,
  PixelWand *border_color)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  PixelSetPixelColor(border_color,&wand->images->border_color);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e F e a t u r e s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageFeatures() returns features for each channel in the
%  image in each of four directions (horizontal, vertical, left and right
%  diagonals) for the specified distance.  The features include the angular
%  second moment, contrast, correlation, sum of squares: variance, inverse
%  difference moment, sum average, sum varience, sum entropy, entropy,
%  difference variance, difference entropy, information measures of
%  correlation 1, information measures of correlation 2, and maximum
%  correlation coefficient.  You can access the red channel contrast, for
%  example, like this:
%
%      channel_features=MagickGetImageFeatures(wand,1);
%      contrast=channel_features[RedPixelChannel].contrast[0];
%
%  Use MagickRelinquishMemory() to free the statistics buffer.
%
%  The format of the MagickGetImageFeatures method is:
%
%      ChannelFeatures *MagickGetImageFeatures(MagickWand *wand,
%        const size_t distance)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o distance: the distance.
%
*/
WandExport ChannelFeatures *MagickGetImageFeatures(MagickWand *wand,
  const size_t distance)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((ChannelFeatures *) NULL);
    }
  return(GetImageFeatures(wand->images,distance,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e K u r t o s i s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageKurtosis() gets the kurtosis and skewness of one or
%  more image channels.
%
%  The format of the MagickGetImageKurtosis method is:
%
%      MagickBooleanType MagickGetImageKurtosis(MagickWand *wand,
%        double *kurtosis,double *skewness)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o kurtosis:  The kurtosis for the specified channel(s).
%
%    o skewness:  The skewness for the specified channel(s).
%
*/
WandExport MagickBooleanType MagickGetImageKurtosis(MagickWand *wand,
  double *kurtosis,double *skewness)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=GetImageKurtosis(wand->images,kurtosis,skewness,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e M e a n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageMean() gets the mean and standard deviation of one or more
%  image channels.
%
%  The format of the MagickGetImageMean method is:
%
%      MagickBooleanType MagickGetImageMean(MagickWand *wand,double *mean,
%        double *standard_deviation)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the image channel(s).
%
%    o mean:  The mean pixel value for the specified channel(s).
%
%    o standard_deviation:  The standard deviation for the specified channel(s).
%
*/
WandExport MagickBooleanType MagickGetImageMean(MagickWand *wand,double *mean,
  double *standard_deviation)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=GetImageMean(wand->images,mean,standard_deviation,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e R a n g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageRange() gets the range for one or more image channels.
%
%  The format of the MagickGetImageRange method is:
%
%      MagickBooleanType MagickGetImageRange(MagickWand *wand,double *minima,
%        double *maxima)
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
WandExport MagickBooleanType MagickGetImageRange(MagickWand *wand,
  double *minima,double *maxima)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=GetImageRange(wand->images,minima,maxima,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e S t a t i s t i c s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageStatistics() returns statistics for each channel in the
%  image.  The statistics include the channel depth, its minima and
%  maxima, the mean, the standard deviation, the kurtosis and the skewness.
%  You can access the red channel mean, for example, like this:
%
%      channel_statistics=MagickGetImageStatistics(wand);
%      red_mean=channel_statistics[RedPixelChannel].mean;
%
%  Use MagickRelinquishMemory() to free the statistics buffer.
%
%  The format of the MagickGetImageStatistics method is:
%
%      ChannelStatistics *MagickGetImageStatistics(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport ChannelStatistics *MagickGetImageStatistics(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((ChannelStatistics *) NULL);
    }
  return(GetImageStatistics(wand->images,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e C o l o r m a p C o l o r                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageColormapColor() returns the color of the specified colormap
%  index.
%
%  The format of the MagickGetImageColormapColor method is:
%
%      MagickBooleanType MagickGetImageColormapColor(MagickWand *wand,
%        const size_t index,PixelWand *color)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o index: the offset into the image colormap.
%
%    o color: Return the colormap color in this wand.
%
*/
WandExport MagickBooleanType MagickGetImageColormapColor(MagickWand *wand,
  const size_t index,PixelWand *color)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if ((wand->images->colormap == (PixelInfo *) NULL) ||
      (index >= wand->images->colors))
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "InvalidColormapIndex","`%s'",wand->name);
      return(MagickFalse);
    }
  PixelSetPixelColor(color,wand->images->colormap+index);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e C o l o r s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageColors() gets the number of unique colors in the image.
%
%  The format of the MagickGetImageColors method is:
%
%      size_t MagickGetImageColors(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport size_t MagickGetImageColors(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(0);
    }
  return(GetNumberColors(wand->images,(FILE *) NULL,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e C o l o r s p a c e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageColorspace() gets the image colorspace.
%
%  The format of the MagickGetImageColorspace method is:
%
%      ColorspaceType MagickGetImageColorspace(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport ColorspaceType MagickGetImageColorspace(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedColorspace);
    }
  return(wand->images->colorspace);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e C o m p o s e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageCompose() returns the composite operator associated with the
%  image.
%
%  The format of the MagickGetImageCompose method is:
%
%      CompositeOperator MagickGetImageCompose(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport CompositeOperator MagickGetImageCompose(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedCompositeOp);
    }
  return(wand->images->compose);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e C o m p r e s s i o n                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageCompression() gets the image compression.
%
%  The format of the MagickGetImageCompression method is:
%
%      CompressionType MagickGetImageCompression(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport CompressionType MagickGetImageCompression(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedCompression);
    }
  return(wand->images->compression);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e C o m p r e s s i o n Q u a l i t y           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageCompressionQuality() gets the image compression quality.
%
%  The format of the MagickGetImageCompressionQuality method is:
%
%      size_t MagickGetImageCompressionQuality(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport size_t MagickGetImageCompressionQuality(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(0UL);
    }
  return(wand->images->quality);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e D e l a y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageDelay() gets the image delay.
%
%  The format of the MagickGetImageDelay method is:
%
%      size_t MagickGetImageDelay(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport size_t MagickGetImageDelay(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(wand->images->delay);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e D e p t h                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageDepth() gets the image depth.
%
%  The format of the MagickGetImageDepth method is:
%
%      size_t MagickGetImageDepth(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport size_t MagickGetImageDepth(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(wand->images->depth);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e D i s p o s e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageDispose() gets the image disposal method.
%
%  The format of the MagickGetImageDispose method is:
%
%      DisposeType MagickGetImageDispose(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport DisposeType MagickGetImageDispose(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedDispose);
    }
  return((DisposeType) wand->images->dispose);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e D i s t o r t i o n                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageDistortion() compares an image to a reconstructed image and
%  returns the specified distortion metric.
%
%  The format of the MagickGetImageDistortion method is:
%
%      MagickBooleanType MagickGetImageDistortion(MagickWand *wand,
%        const MagickWand *reference,const MetricType metric,
%        double *distortion)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o reference: the reference wand.
%
%    o metric: the metric.
%
%    o distortion: the computed distortion between the images.
%
*/
WandExport MagickBooleanType MagickGetImageDistortion(MagickWand *wand,
  const MagickWand *reference,const MetricType metric,double *distortion)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) || (reference->images == (Image *) NULL))
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=GetImageDistortion(wand->images,reference->images,metric,distortion,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e D i s t o r t i o n s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageDistortions() compares one or more pixel channels of an
%  image to a reconstructed image and returns the specified distortion metrics.
%
%  Use MagickRelinquishMemory() to free the metrics when you are done with them.
%
%  The format of the MagickGetImageDistortion method is:
%
%      double *MagickGetImageDistortion(MagickWand *wand,
%        const MagickWand *reference,const MetricType metric)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o reference: the reference wand.
%
%    o metric: the metric.
%
*/
WandExport double *MagickGetImageDistortions(MagickWand *wand,
  const MagickWand *reference,const MetricType metric)
{
  double
    *channel_distortion;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(reference != (MagickWand *) NULL);
  assert(reference->signature == MagickWandSignature);
  if ((wand->images == (Image *) NULL) || (reference->images == (Image *) NULL))
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((double *) NULL);
    }
  channel_distortion=GetImageDistortions(wand->images,reference->images,
    metric,wand->exception);
  return(channel_distortion);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e E n d i a n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageEndian() gets the image endian.
%
%  The format of the MagickGetImageEndian method is:
%
%      EndianType MagickGetImageEndian(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport EndianType MagickGetImageEndian(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedEndian);
    }
  return(wand->images->endian);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e F i l e n a m e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageFilename() returns the filename of a particular image in a
%  sequence.
%
%  The format of the MagickGetImageFilename method is:
%
%      char *MagickGetImageFilename(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport char *MagickGetImageFilename(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((char *) NULL);
    }
  return(AcquireString(wand->images->filename));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e F o r m a t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageFormat() returns the format of a particular image in a
%  sequence.
%
%  The format of the MagickGetImageFormat method is:
%
%      char *MagickGetImageFormat(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport char *MagickGetImageFormat(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((char *) NULL);
    }
  return(AcquireString(wand->images->magick));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e F u z z                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageFuzz() gets the image fuzz.
%
%  The format of the MagickGetImageFuzz method is:
%
%      double MagickGetImageFuzz(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport double MagickGetImageFuzz(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(0.0);
    }
  return(wand->images->fuzz);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e G a m m a                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageGamma() gets the image gamma.
%
%  The format of the MagickGetImageGamma method is:
%
%      double MagickGetImageGamma(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport double MagickGetImageGamma(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(0.0);
    }
  return(wand->images->gamma);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e G r a v i t y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageGravity() gets the image gravity.
%
%  The format of the MagickGetImageGravity method is:
%
%      GravityType MagickGetImageGravity(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport GravityType MagickGetImageGravity(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedGravity);
    }
  return(wand->images->gravity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e G r e e n P r i m a r y                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageGreenPrimary() returns the chromaticy green primary point.
%
%  The format of the MagickGetImageGreenPrimary method is:
%
%      MagickBooleanType MagickGetImageGreenPrimary(MagickWand *wand,double *x,
%        double *y,double *z)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x: the chromaticity green primary x-point.
%
%    o y: the chromaticity green primary y-point.
%
%    o z: the chromaticity green primary z-point.
%
*/
WandExport MagickBooleanType MagickGetImageGreenPrimary(MagickWand *wand,
  double *x,double *y,double *z)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  *x=wand->images->chromaticity.green_primary.x;
  *y=wand->images->chromaticity.green_primary.y;
  *z=wand->images->chromaticity.green_primary.z;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e H e i g h t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageHeight() returns the image height.
%
%  The format of the MagickGetImageHeight method is:
%
%      size_t MagickGetImageHeight(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport size_t MagickGetImageHeight(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(wand->images->rows);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e H i s t o g r a m                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageHistogram() returns the image histogram as an array of
%  PixelWand wands.
%
%  The format of the MagickGetImageHistogram method is:
%
%      PixelWand **MagickGetImageHistogram(MagickWand *wand,
%        size_t *number_colors)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o number_colors: the number of unique colors in the image and the number
%      of pixel wands returned.
%
*/
WandExport PixelWand **MagickGetImageHistogram(MagickWand *wand,
  size_t *number_colors)
{
  PixelInfo
    *histogram;

  PixelWand
    **pixel_wands;

  register ssize_t
    i;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((PixelWand **) NULL);
    }
  histogram=GetImageHistogram(wand->images,number_colors,wand->exception);
  if (histogram == (PixelInfo *) NULL)
    return((PixelWand **) NULL);
  pixel_wands=NewPixelWands(*number_colors);
  for (i=0; i < (ssize_t) *number_colors; i++)
  {
    PixelSetPixelColor(pixel_wands[i],&histogram[i]);
    PixelSetColorCount(pixel_wands[i],(size_t) histogram[i].count);
  }
  histogram=(PixelInfo *) RelinquishMagickMemory(histogram);
  return(pixel_wands);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e I n t e r l a c e S c h e m e                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageInterlaceScheme() gets the image interlace scheme.
%
%  The format of the MagickGetImageInterlaceScheme method is:
%
%      InterlaceType MagickGetImageInterlaceScheme(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport InterlaceType MagickGetImageInterlaceScheme(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedInterlace);
    }
  return(wand->images->interlace);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e I n t e r p o l a t e M e t h o d             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageInterpolateMethod() returns the interpolation method for the
%  sepcified image.
%
%  The format of the MagickGetImageInterpolateMethod method is:
%
%      PixelInterpolateMethod MagickGetImageInterpolateMethod(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport PixelInterpolateMethod MagickGetImageInterpolateMethod(
  MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedInterpolatePixel);
    }
  return(wand->images->interpolate);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e I t e r a t i o n s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageIterations() gets the image iterations.
%
%  The format of the MagickGetImageIterations method is:
%
%      size_t MagickGetImageIterations(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport size_t MagickGetImageIterations(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(wand->images->iterations);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e L e n g t h                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageLength() returns the image length in bytes.
%
%  The format of the MagickGetImageLength method is:
%
%      MagickBooleanType MagickGetImageLength(MagickWand *wand,
%        MagickSizeType *length)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o length: the image length in bytes.
%
*/
WandExport MagickBooleanType MagickGetImageLength(MagickWand *wand,
  MagickSizeType *length)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  *length=GetBlobSize(wand->images);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e M a t t e C o l o r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageMatteColor() returns the image matte color.
%
%  The format of the MagickGetImageMatteColor method is:
%
%      MagickBooleanType MagickGetImageMatteColor(MagickWand *wand,
%        PixelWand *matte_color)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o matte_color: return the alpha color.
%
*/
WandExport MagickBooleanType MagickGetImageMatteColor(MagickWand *wand,
  PixelWand *matte_color)
{
  assert(wand != (MagickWand *)NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *)NULL)
    ThrowWandException(WandError, "ContainsNoImages", wand->name);
  PixelSetPixelColor(matte_color,&wand->images->matte_color);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e O r i e n t a t i o n                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageOrientation() returns the image orientation.
%
%  The format of the MagickGetImageOrientation method is:
%
%      OrientationType MagickGetImageOrientation(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport OrientationType MagickGetImageOrientation(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedOrientation);
    }
  return(wand->images->orientation);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e P a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImagePage() returns the page geometry associated with the image.
%
%  The format of the MagickGetImagePage method is:
%
%      MagickBooleanType MagickGetImagePage(MagickWand *wand,
%        size_t *width,size_t *height,ssize_t *x,ssize_t *y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o width: the page width.
%
%    o height: the page height.
%
%    o x: the page x-offset.
%
%    o y: the page y-offset.
%
*/
WandExport MagickBooleanType MagickGetImagePage(MagickWand *wand,
  size_t *width,size_t *height,ssize_t *x,ssize_t *y)
{
  assert(wand != (const MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  *width=wand->images->page.width;
  *height=wand->images->page.height;
  *x=wand->images->page.x;
  *y=wand->images->page.y;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e P i x e l C o l o r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImagePixelColor() returns the color of the specified pixel.
%
%  The format of the MagickGetImagePixelColor method is:
%
%      MagickBooleanType MagickGetImagePixelColor(MagickWand *wand,
%        const ssize_t x,const ssize_t y,PixelWand *color)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x,y: the pixel offset into the image.
%
%    o color: Return the colormap color in this wand.
%
*/
WandExport MagickBooleanType MagickGetImagePixelColor(MagickWand *wand,
  const ssize_t x,const ssize_t y,PixelWand *color)
{
  register const Quantum
    *p;

  CacheView
    *image_view;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  image_view=AcquireVirtualCacheView(wand->images,wand->exception);
  p=GetCacheViewVirtualPixels(image_view,x,y,1,1,wand->exception);
  if (p == (const Quantum *) NULL)
    {
      image_view=DestroyCacheView(image_view);
      return(MagickFalse);
    }
  PixelSetQuantumPixel(wand->images,p,color);
  image_view=DestroyCacheView(image_view);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e R e d P r i m a r y                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageRedPrimary() returns the chromaticy red primary point.
%
%  The format of the MagickGetImageRedPrimary method is:
%
%      MagickBooleanType MagickGetImageRedPrimary(MagickWand *wand,double *x,
%        double *y, double *z)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x: the chromaticity red primary x-point.
%
%    o y: the chromaticity red primary y-point.
%
%    o z: the chromaticity red primary z-point.
%
*/
WandExport MagickBooleanType MagickGetImageRedPrimary(MagickWand *wand,
  double *x,double *y,double *z)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  *x=wand->images->chromaticity.red_primary.x;
  *y=wand->images->chromaticity.red_primary.y;
  *z=wand->images->chromaticity.red_primary.z;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e R e g i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageRegion() extracts a region of the image and returns it as a
%  a new wand.
%
%  The format of the MagickGetImageRegion method is:
%
%      MagickWand *MagickGetImageRegion(MagickWand *wand,
%        const size_t width,const size_t height,const ssize_t x,
%        const ssize_t y)
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
WandExport MagickWand *MagickGetImageRegion(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y)
{
  Image
    *region_image;

  RectangleInfo
    region;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  region.width=width;
  region.height=height;
  region.x=x;
  region.y=y;
  region_image=CropImage(wand->images,&region,wand->exception);
  if (region_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,region_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e R e n d e r i n g I n t e n t                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageRenderingIntent() gets the image rendering intent.
%
%  The format of the MagickGetImageRenderingIntent method is:
%
%      RenderingIntent MagickGetImageRenderingIntent(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport RenderingIntent MagickGetImageRenderingIntent(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedIntent);
    }
  return((RenderingIntent) wand->images->rendering_intent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e R e s o l u t i o n                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageResolution() gets the image X and Y resolution.
%
%  The format of the MagickGetImageResolution method is:
%
%      MagickBooleanType MagickGetImageResolution(MagickWand *wand,double *x,
%        double *y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x: the image x-resolution.
%
%    o y: the image y-resolution.
%
*/
WandExport MagickBooleanType MagickGetImageResolution(MagickWand *wand,
  double *x,double *y)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  *x=wand->images->resolution.x;
  *y=wand->images->resolution.y;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e S c e n e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageScene() gets the image scene.
%
%  The format of the MagickGetImageScene method is:
%
%      size_t MagickGetImageScene(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport size_t MagickGetImageScene(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(wand->images->scene);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e S i g n a t u r e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageSignature() generates an SHA-256 message digest for the image
%  pixel stream.
%
%  The format of the MagickGetImageSignature method is:
%
%      char *MagickGetImageSignature(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport char *MagickGetImageSignature(MagickWand *wand)
{
  const char
    *value;

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((char *) NULL);
    }
  status=SignatureImage(wand->images,wand->exception);
  if (status == MagickFalse)
    return((char *) NULL);
  value=GetImageProperty(wand->images,"signature",wand->exception);
  if (value == (const char *) NULL)
    return((char *) NULL);
  return(AcquireString(value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e T i c k s P e r S e c o n d                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageTicksPerSecond() gets the image ticks-per-second.
%
%  The format of the MagickGetImageTicksPerSecond method is:
%
%      size_t MagickGetImageTicksPerSecond(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport size_t MagickGetImageTicksPerSecond(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return((size_t) wand->images->ticks_per_second);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e T y p e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageType() gets the potential image type:
%
%        Bilevel        Grayscale       GrayscaleMatte
%        Palette        PaletteMatte    TrueColor
%        TrueColorMatte ColorSeparation ColorSeparationMatte
%
%  The format of the MagickGetImageType method is:
%
%      ImageType MagickGetImageType(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport ImageType MagickGetImageType(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedType);
    }
  return(GetImageType(wand->images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e U n i t s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageUnits() gets the image units of resolution.
%
%  The format of the MagickGetImageUnits method is:
%
%      ResolutionType MagickGetImageUnits(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport ResolutionType MagickGetImageUnits(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedResolution);
    }
  return(wand->images->units);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e V i r t u a l P i x e l M e t h o d           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageVirtualPixelMethod() returns the virtual pixel method for the
%  sepcified image.
%
%  The format of the MagickGetImageVirtualPixelMethod method is:
%
%      VirtualPixelMethod MagickGetImageVirtualPixelMethod(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport VirtualPixelMethod MagickGetImageVirtualPixelMethod(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedVirtualPixelMethod);
    }
  return(GetImageVirtualPixelMethod(wand->images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e W h i t e P o i n t                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageWhitePoint() returns the chromaticy white point.
%
%  The format of the MagickGetImageWhitePoint method is:
%
%      MagickBooleanType MagickGetImageWhitePoint(MagickWand *wand,double *x,
%        double *y,double *z)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x: the chromaticity white x-point.
%
%    o y: the chromaticity white y-point.
%
%    o z: the chromaticity white z-point.
%
*/
WandExport MagickBooleanType MagickGetImageWhitePoint(MagickWand *wand,
  double *x,double *y,double *z)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  *x=wand->images->chromaticity.white_point.x;
  *y=wand->images->chromaticity.white_point.y;
  *z=wand->images->chromaticity.white_point.z;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e W i d t h                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageWidth() returns the image width.
%
%  The format of the MagickGetImageWidth method is:
%
%      size_t MagickGetImageWidth(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport size_t MagickGetImageWidth(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(wand->images->columns);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t N u m b e r I m a g e s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetNumberImages() returns the number of images associated with a
%  magick wand.
%
%  The format of the MagickGetNumberImages method is:
%
%      size_t MagickGetNumberImages(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport size_t MagickGetNumberImages(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(GetImageListLength(wand->images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k I m a g e G e t T o t a l I n k D e n s i t y                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageTotalInkDensity() gets the image total ink density.
%
%  The format of the MagickGetImageTotalInkDensity method is:
%
%      double MagickGetImageTotalInkDensity(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport double MagickGetImageTotalInkDensity(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(0.0);
    }
  return(GetImageTotalInkDensity(wand->images,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k H a l d C l u t I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickHaldClutImage() replaces colors in the image from a Hald color lookup
%  table.   A Hald color lookup table is a 3-dimensional color cube mapped to 2
%  dimensions.  Create it with the HALD coder.  You can apply any color
%  transformation to the Hald image and then use this method to apply the
%  transform to the image.
%
%  The format of the MagickHaldClutImage method is:
%
%      MagickBooleanType MagickHaldClutImage(MagickWand *wand,
%        const MagickWand *hald_wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o hald_image: the hald CLUT image.
%
*/
WandExport MagickBooleanType MagickHaldClutImage(MagickWand *wand,
  const MagickWand *hald_wand)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) || (hald_wand->images == (Image *) NULL))
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=HaldClutImage(wand->images,hald_wand->images,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k H a s N e x t I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickHasNextImage() returns MagickTrue if the wand has more images when
%  traversing the list in the forward direction
%
%  The format of the MagickHasNextImage method is:
%
%      MagickBooleanType MagickHasNextImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickHasNextImage(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if (GetNextImageInList(wand->images) == (Image *) NULL)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k H a s P r e v i o u s I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickHasPreviousImage() returns MagickTrue if the wand has more images when
%  traversing the list in the reverse direction
%
%  The format of the MagickHasPreviousImage method is:
%
%      MagickBooleanType MagickHasPreviousImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickHasPreviousImage(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if (GetPreviousImageInList(wand->images) == (Image *) NULL)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k I d e n t i f y I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickIdentifyImage() identifies an image by printing its attributes to the
%  file.  Attributes include the image width, height, size, and others.
%
%  The format of the MagickIdentifyImage method is:
%
%      const char *MagickIdentifyImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport char *MagickIdentifyImage(MagickWand *wand)
{
  char
    *description,
    filename[MagickPathExtent];

  FILE
    *file;

  int
    unique_file;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((char *) NULL);
    }
  description=(char *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  file=(FILE *) NULL;
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "UnableToCreateTemporaryFile","`%s'",wand->name);
      return((char *) NULL);
    }
  (void) IdentifyImage(wand->images,file,MagickTrue,wand->exception);
  (void) fclose(file);
  description=FileToString(filename,~0UL,wand->exception);
  (void) RelinquishUniqueFileResource(filename);
  return(description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k I d e n t i f y I m a g e T y p e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickIdentifyImageType() gets the potential image type:
%
%        Bilevel        Grayscale       GrayscaleMatte
%        Palette        PaletteMatte    TrueColor
%        TrueColorMatte ColorSeparation ColorSeparationMatte
%
%  To ensure the image type matches its potential, use MagickSetImageType():
%
%    (void) MagickSetImageType(wand,MagickIdentifyImageType(wand));
%
%  The format of the MagickIdentifyImageType method is:
%
%      ImageType MagickIdentifyImageType(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport ImageType MagickIdentifyImageType(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return(UndefinedType);
    }
  return(IdentifyImageType(wand->images,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k I m p l o d e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickImplodeImage() creates a new image that is a copy of an existing
%  one with the image pixels "implode" by the specified percentage.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the MagickImplodeImage method is:
%
%      MagickBooleanType MagickImplodeImage(MagickWand *wand,
%        const double radius,const PixelInterpolateMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o amount: Define the extent of the implosion.
%
%    o method: the pixel interpolation method.
%
*/
WandExport MagickBooleanType MagickImplodeImage(MagickWand *wand,
  const double amount,const PixelInterpolateMethod method)
{
  Image
    *implode_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  implode_image=ImplodeImage(wand->images,amount,method,wand->exception);
  if (implode_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,implode_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k I m p o r t I m a g e P i x e l s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickImportImagePixels() accepts pixel datand stores it in the image at the
%  location you specify.  The method returns MagickFalse on success otherwise
%  MagickTrue if an error is encountered.  The pixel data can be either char,
%  short int, int, ssize_t, float, or double in the order specified by map.
%
%  Suppose your want to upload the first scanline of a 640x480 image from
%  character data in red-green-blue order:
%
%      MagickImportImagePixels(wand,0,0,640,1,"RGB",CharPixel,pixels);
%
%  The format of the MagickImportImagePixels method is:
%
%      MagickBooleanType MagickImportImagePixels(MagickWand *wand,
%        const ssize_t x,const ssize_t y,const size_t columns,
%        const size_t rows,const char *map,const StorageType storage,
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
%      A = alpha (0 is transparent), O = alpha (0 is opaque), C = cyan,
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
WandExport MagickBooleanType MagickImportImagePixels(MagickWand *wand,
  const ssize_t x,const ssize_t y,const size_t columns,const size_t rows,
  const char *map,const StorageType storage,const void *pixels)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=ImportImagePixels(wand->images,x,y,columns,rows,map,storage,pixels,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k I n t e r p o l a t i v e R e s i z e I m a g e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickInterpolativeResizeImage() resize image using a interpolative
%  method.
%
%      MagickBooleanType MagickInterpolativeResizeImage(MagickWand *wand,
%        const size_t columns,const size_t rows,
%        const PixelInterpolateMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
%    o interpolate: the pixel interpolation method.
%
*/
WandExport MagickBooleanType MagickInterpolativeResizeImage(MagickWand *wand,
  const size_t columns,const size_t rows,const PixelInterpolateMethod method)
{
  Image
    *resize_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  resize_image=InterpolativeResizeImage(wand->images,columns,rows,method,
    wand->exception);
  if (resize_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,resize_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k I n v e r s e F o u r i e r T r a n s f o r m I m a g e       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickInverseFourierTransformImage() implements the inverse discrete
%  Fourier transform (DFT) of the image either as a magnitude / phase or real /
%  imaginary image pair.
%
%  The format of the MagickInverseFourierTransformImage method is:
%
%      MagickBooleanType MagickInverseFourierTransformImage(
%        MagickWand *magnitude_wand,MagickWand *phase_wand,
%        const MagickBooleanType magnitude)
%
%  A description of each parameter follows:
%
%    o magnitude_wand: the magnitude or real wand.
%
%    o phase_wand: the phase or imaginary wand.
%
%    o magnitude: if true, return as magnitude / phase pair otherwise a real /
%      imaginary image pair.
%
*/
WandExport MagickBooleanType MagickInverseFourierTransformImage(
  MagickWand *magnitude_wand,MagickWand *phase_wand,
  const MagickBooleanType magnitude)
{
  Image
    *inverse_image;

  MagickWand
    *wand;

  assert(magnitude_wand != (MagickWand *) NULL);
  assert(magnitude_wand->signature == MagickWandSignature);
  if (magnitude_wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",
      magnitude_wand->name);
  wand=magnitude_wand;
  if (magnitude_wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",
      magnitude_wand->name);
  assert(phase_wand != (MagickWand *) NULL);
  assert(phase_wand->signature == MagickWandSignature);
  inverse_image=InverseFourierTransformImage(magnitude_wand->images,
    phase_wand->images,magnitude,wand->exception);
  if (inverse_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,inverse_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k L a b e l I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickLabelImage() adds a label to your image.
%
%  The format of the MagickLabelImage method is:
%
%      MagickBooleanType MagickLabelImage(MagickWand *wand,const char *label)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o label: the image label.
%
*/
WandExport MagickBooleanType MagickLabelImage(MagickWand *wand,
  const char *label)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=SetImageProperty(wand->images,"label",label,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k L e v e l I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickLevelImage() adjusts the levels of an image by scaling the colors
%  falling between specified white and black points to the full available
%  quantum range. The parameters provided represent the black, mid, and white
%  points. The black point specifies the darkest color in the image. Colors
%  darker than the black point are set to zero. Mid point specifies a gamma
%  correction to apply to the image.  White point specifies the lightest color
%  in the image. Colors brighter than the white point are set to the maximum
%  quantum value.
%
%  The format of the MagickLevelImage method is:
%
%      MagickBooleanType MagickLevelImage(MagickWand *wand,
%        const double black_point,const double gamma,const double white_point)
%      MagickBooleanType MagickLevelImage(MagickWand *wand,
%        const ChannelType channel,const double black_point,const double gamma,
%        const double white_point)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: Identify which channel to level: RedPixelChannel,
%      GreenPixelChannel, etc.
%
%    o black_point: the black point.
%
%    o gamma: the gamma.
%
%    o white_point: the white point.
%
*/
WandExport MagickBooleanType MagickLevelImage(MagickWand *wand,
  const double black_point,const double gamma,const double white_point)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=LevelImage(wand->images,black_point,white_point,gamma,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k L i n e a r S t r e t c h I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickLinearStretchImage() stretches with saturation the image intensity.
%
%  You can also reduce the influence of a particular channel with a gamma
%  value of 0.
%
%  The format of the MagickLinearStretchImage method is:
%
%      MagickBooleanType MagickLinearStretchImage(MagickWand *wand,
%        const double black_point,const double white_point)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o black_point: the black point.
%
%    o white_point: the white point.
%
*/
WandExport MagickBooleanType MagickLinearStretchImage(MagickWand *wand,
  const double black_point,const double white_point)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=LinearStretchImage(wand->images,black_point,white_point,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k L i q u i d R e s c a l e I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickLiquidRescaleImage() rescales image with seam carving.
%
%      MagickBooleanType MagickLiquidRescaleImage(MagickWand *wand,
%        const size_t columns,const size_t rows,
%        const double delta_x,const double rigidity)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
%    o delta_x: maximum seam transversal step (0 means straight seams).
%
%    o rigidity: introduce a bias for non-straight seams (typically 0).
%
*/
WandExport MagickBooleanType MagickLiquidRescaleImage(MagickWand *wand,
  const size_t columns,const size_t rows,const double delta_x,
  const double rigidity)
{
  Image
    *rescale_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  rescale_image=LiquidRescaleImage(wand->images,columns,rows,delta_x,
    rigidity,wand->exception);
  if (rescale_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,rescale_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a g i c k L o c a l C o n t r a s t I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickLocalContrastImage() attempts to increase the appearance of
%  large-scale light-dark transitions. Local contrast enhancement works
%  similarly to sharpening with an unsharp mask, however the mask is instead
%  created using an image with a greater blur distance.
%
%      MagickBooleanType MagickLocalContrastImage(MagickWand *wand,
%        const double radius,const double strength)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the Gaussian, in pixels, not counting
%      the center pixel.
%
%    o strength: the strength of the blur mask in percentage.
%
*/
WandExport MagickBooleanType MagickLocalContrastImage(MagickWand *wand,
  const double radius, const double strength)
{
  Image
    *contrast_image;

  assert(wand != (MagickWand *)NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s", wand->name);
  if (wand->images == (Image *)NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  contrast_image=LocalContrastImage(wand->images,radius,strength,
    wand->exception);
  if (contrast_image == (Image *)NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,contrast_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M a g n i f y I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMagnifyImage() is a convenience method that scales an image
%  proportionally to twice its original size.
%
%  The format of the MagickMagnifyImage method is:
%
%      MagickBooleanType MagickMagnifyImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickMagnifyImage(MagickWand *wand)
{
  Image
    *magnify_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  magnify_image=MagnifyImage(wand->images,wand->exception);
  if (magnify_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,magnify_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M e r g e I m a g e L a y e r s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMergeImageLayers() composes all the image layers from the current
%  given image onward to produce a single image of the merged layers.
%
%  The inital canvas's size depends on the given LayerMethod, and is
%  initialized using the first images background color.  The images
%  are then compositied onto that image in sequence using the given
%  composition that has been assigned to each individual image.
%
%  The format of the MagickMergeImageLayers method is:
%
%      MagickWand *MagickMergeImageLayers(MagickWand *wand,
%        const LayerMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o method: the method of selecting the size of the initial canvas.
%
%        MergeLayer: Merge all layers onto a canvas just large enough
%           to hold all the actual images. The virtual canvas of the
%           first image is preserved but otherwise ignored.
%
%        FlattenLayer: Use the virtual canvas size of first image.
%           Images which fall outside this canvas is clipped.
%           This can be used to 'fill out' a given virtual canvas.
%
%        MosaicLayer: Start with the virtual canvas of the first image,
%           enlarging left and right edges to contain all images.
%           Images with negative offsets will be clipped.
%
*/
WandExport MagickWand *MagickMergeImageLayers(MagickWand *wand,
  const LayerMethod method)
{
  Image
    *mosaic_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  mosaic_image=MergeImageLayers(wand->images,method,wand->exception);
  if (mosaic_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,mosaic_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M i n i f y I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMinifyImage() is a convenience method that scales an image
%  proportionally to one-half its original size
%
%  The format of the MagickMinifyImage method is:
%
%      MagickBooleanType MagickMinifyImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickMinifyImage(MagickWand *wand)
{
  Image
    *minify_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  minify_image=MinifyImage(wand->images,wand->exception);
  if (minify_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,minify_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M o d u l a t e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickModulateImage() lets you control the brightness, saturation, and hue
%  of an image.  Hue is the percentage of absolute rotation from the current
%  position.  For example 50 results in a counter-clockwise rotation of 90
%  degrees, 150 results in a clockwise rotation of 90 degrees, with 0 and 200
%  both resulting in a rotation of 180 degrees.
%
%  To increase the color brightness by 20% and decrease the color saturation by
%  10% and leave the hue unchanged, use: 120,90,100.
%
%  The format of the MagickModulateImage method is:
%
%      MagickBooleanType MagickModulateImage(MagickWand *wand,
%        const double brightness,const double saturation,const double hue)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o brightness: the percent change in brighness.
%
%    o saturation: the percent change in saturation.
%
%    o hue: the percent change in hue.
%
*/
WandExport MagickBooleanType MagickModulateImage(MagickWand *wand,
  const double brightness,const double saturation,const double hue)
{
  char
    modulate[MagickPathExtent];

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  (void) FormatLocaleString(modulate,MagickPathExtent,"%g,%g,%g",
    brightness,saturation,hue);
  status=ModulateImage(wand->images,modulate,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M o n t a g e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMontageImage() creates a composite image by combining several
%  separate images. The images are tiled on the composite image with the name
%  of the image optionally appearing just below the individual tile.
%
%  The format of the MagickMontageImage method is:
%
%      MagickWand *MagickMontageImage(MagickWand *wand,
%        const DrawingWand drawing_wand,const char *tile_geometry,
%        const char *thumbnail_geometry,const MontageMode mode,
%        const char *frame)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o drawing_wand: the drawing wand.  The font name, size, and color are
%      obtained from this wand.
%
%    o tile_geometry: the number of tiles per row and page (e.g. 6x4+0+0).
%
%    o thumbnail_geometry: Preferred image size and border size of each
%      thumbnail (e.g. 120x120+4+3>).
%
%    o mode: Thumbnail framing mode: Frame, Unframe, or Concatenate.
%
%    o frame: Surround the image with an ornamental border (e.g. 15x15+3+3).
%      The frame color is that of the thumbnail's matte color.
%
*/
WandExport MagickWand *MagickMontageImage(MagickWand *wand,
  const DrawingWand *drawing_wand,const char *tile_geometry,
  const char *thumbnail_geometry,const MontageMode mode,const char *frame)
{
  char
    *font;

  Image
    *montage_image;

  MontageInfo
    *montage_info;

  PixelWand
    *pixel_wand;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  montage_info=CloneMontageInfo(wand->image_info,(MontageInfo *) NULL);
  switch (mode)
  {
    case FrameMode:
    {
      (void) CloneString(&montage_info->frame,"15x15+3+3");
      montage_info->shadow=MagickTrue;
      break;
    }
    case UnframeMode:
    {
      montage_info->frame=(char *) NULL;
      montage_info->shadow=MagickFalse;
      montage_info->border_width=0;
      break;
    }
    case ConcatenateMode:
    {
      montage_info->frame=(char *) NULL;
      montage_info->shadow=MagickFalse;
      (void) CloneString(&montage_info->geometry,"+0+0");
      montage_info->border_width=0;
      break;
    }
    default:
      break;
  }
  font=DrawGetFont(drawing_wand);
  if (font != (char *) NULL)
    (void) CloneString(&montage_info->font,font);
  if (frame != (char *) NULL)
    (void) CloneString(&montage_info->frame,frame);
  montage_info->pointsize=DrawGetFontSize(drawing_wand);
  pixel_wand=NewPixelWand();
  DrawGetFillColor(drawing_wand,pixel_wand);
  PixelGetQuantumPacket(pixel_wand,&montage_info->fill);
  DrawGetStrokeColor(drawing_wand,pixel_wand);
  PixelGetQuantumPacket(pixel_wand,&montage_info->stroke);
  pixel_wand=DestroyPixelWand(pixel_wand);
  if (thumbnail_geometry != (char *) NULL)
    (void) CloneString(&montage_info->geometry,thumbnail_geometry);
  if (tile_geometry != (char *) NULL)
    (void) CloneString(&montage_info->tile,tile_geometry);
  montage_image=MontageImageList(wand->image_info,montage_info,wand->images,
    wand->exception);
  montage_info=DestroyMontageInfo(montage_info);
  if (montage_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,montage_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M o r p h I m a g e s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMorphImages() method morphs a set of images.  Both the image pixels
%  and size are linearly interpolated to give the appearance of a
%  meta-morphosis from one image to the next.
%
%  The format of the MagickMorphImages method is:
%
%      MagickWand *MagickMorphImages(MagickWand *wand,
%        const size_t number_frames)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o number_frames: the number of in-between images to generate.
%
*/
WandExport MagickWand *MagickMorphImages(MagickWand *wand,
  const size_t number_frames)
{
  Image
    *morph_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  morph_image=MorphImages(wand->images,number_frames,wand->exception);
  if (morph_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,morph_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M o r p h o l o g y I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMorphologyImage() applies a user supplied kernel to the image
%  according to the given mophology method.
%
%  The format of the MagickMorphologyImage method is:
%
%      MagickBooleanType MagickMorphologyImage(MagickWand *wand,
%        MorphologyMethod method,const ssize_t iterations,KernelInfo *kernel)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o method: the morphology method to be applied.
%
%    o iterations: apply the operation this many times (or no change).
%      A value of -1 means loop until no change found.  How this is applied
%      may depend on the morphology method.  Typically this is a value of 1.
%
%    o kernel: An array of doubles representing the morphology kernel.
%
*/
WandExport MagickBooleanType MagickMorphologyImage(MagickWand *wand,
  MorphologyMethod method,const ssize_t iterations,KernelInfo *kernel)
{
  Image
    *morphology_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (kernel == (const KernelInfo *) NULL)
    return(MagickFalse);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  morphology_image=MorphologyImage(wand->images,method,iterations,kernel,
    wand->exception);
  if (morphology_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,morphology_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k M o t i o n B l u r I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickMotionBlurImage() simulates motion blur.  We convolve the image with a
%  Gaussian operator of the given radius and standard deviation (sigma).
%  For reasonable results, radius should be larger than sigma.  Use a
%  radius of 0 and MotionBlurImage() selects a suitable radius for you.
%  Angle gives the angle of the blurring motion.
%
%  The format of the MagickMotionBlurImage method is:
%
%      MagickBooleanType MagickMotionBlurImage(MagickWand *wand,
%        const double radius,const double sigma,const double angle)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the Gaussian, in pixels, not counting
%      the center pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o angle: Apply the effect along this angle.
%
*/
WandExport MagickBooleanType MagickMotionBlurImage(MagickWand *wand,
  const double radius,const double sigma,const double angle)
{
  Image
    *blur_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  blur_image=MotionBlurImage(wand->images,radius,sigma,angle,wand->exception);
  if (blur_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,blur_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k N e g a t e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickNegateImage() negates the colors in the reference image.  The
%  Grayscale option means that only grayscale values within the image are
%  negated.
%
%  You can also reduce the influence of a particular channel with a gamma
%  value of 0.
%
%  The format of the MagickNegateImage method is:
%
%      MagickBooleanType MagickNegateImage(MagickWand *wand,
%        const MagickBooleanType gray)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o gray: If MagickTrue, only negate grayscale pixels within the image.
%
*/
WandExport MagickBooleanType MagickNegateImage(MagickWand *wand,
  const MagickBooleanType gray)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=NegateImage(wand->images,gray,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k N e w I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickNewImage() adds a blank image canvas of the specified size and
%  background color to the wand.
%
%  The format of the MagickNewImage method is:
%
%      MagickBooleanType MagickNewImage(MagickWand *wand,
%        const size_t columns,const size_t rows,
%        const PixelWand *background)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o width: the image width.
%
%    o height: the image height.
%
%    o background: the image color.
%
*/
WandExport MagickBooleanType MagickNewImage(MagickWand *wand,const size_t width,
  const size_t height,const PixelWand *background)
{
  Image
    *images;

  PixelInfo
    pixel;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  PixelGetMagickColor(background,&pixel);
  images=NewMagickImage(wand->image_info,width,height,&pixel,wand->exception);
  if (images == (Image *) NULL)
    return(MagickFalse);
  return(InsertImageInWand(wand,images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k N e x t I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickNextImage() sets the next image in the wand as the current image.
%
%  It is typically used after MagickResetIterator(), after which its first use
%  will set the first image as the current image (unless the wand is empty).
%
%  It will return MagickFalse when no more images are left to be returned
%  which happens when the wand is empty, or the current image is the last
%  image.
%
%  When the above condition (end of image list) is reached, the iterator is
%  automaticall set so that you can start using MagickPreviousImage() to
%  again iterate over the images in the reverse direction, starting with the
%  last image (again).  You can jump to this condition immeditally using
%  MagickSetLastIterator().
%
%  The format of the MagickNextImage method is:
%
%      MagickBooleanType MagickNextImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickNextImage(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->insert_before=MagickFalse; /* Inserts is now appended */
  if (wand->image_pending != MagickFalse)
    {
      wand->image_pending=MagickFalse;
      return(MagickTrue);
    }
  if (GetNextImageInList(wand->images) == (Image *) NULL)
    {
      wand->image_pending=MagickTrue; /* No image, PreviousImage re-gets */
      return(MagickFalse);
    }
  wand->images=GetNextImageInList(wand->images);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k N o r m a l i z e I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickNormalizeImage() enhances the contrast of a color image by adjusting
%  the pixels color to span the entire range of colors available
%
%  You can also reduce the influence of a particular channel with a gamma
%  value of 0.
%
%  The format of the MagickNormalizeImage method is:
%
%      MagickBooleanType MagickNormalizeImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickNormalizeImage(MagickWand *wand)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=NormalizeImage(wand->images,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k O i l P a i n t I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickOilPaintImage() applies a special effect filter that simulates an oil
%  painting.  Each pixel is replaced by the most frequent color occurring
%  in a circular region defined by radius.
%
%  The format of the MagickOilPaintImage method is:
%
%      MagickBooleanType MagickOilPaintImage(MagickWand *wand,
%        const double radius,const double sigma)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the circular neighborhood.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
*/
WandExport MagickBooleanType MagickOilPaintImage(MagickWand *wand,
  const double radius,const double sigma)
{
  Image
    *paint_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  paint_image=OilPaintImage(wand->images,radius,sigma,wand->exception);
  if (paint_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,paint_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k O p a q u e P a i n t I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickOpaquePaintImage() changes any pixel that matches color with the color
%  defined by fill.
%
%  The format of the MagickOpaquePaintImage method is:
%
%      MagickBooleanType MagickOpaquePaintImage(MagickWand *wand,
%        const PixelWand *target,const PixelWand *fill,const double fuzz,
%        const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
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
%    o invert: paint any pixel that does not match the target color.
%
*/
WandExport MagickBooleanType MagickOpaquePaintImage(MagickWand *wand,
  const PixelWand *target,const PixelWand *fill,const double fuzz,
  const MagickBooleanType invert)
{
  MagickBooleanType
    status;

  PixelInfo
    fill_pixel,
    target_pixel;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  PixelGetMagickColor(target,&target_pixel);
  PixelGetMagickColor(fill,&fill_pixel);
  wand->images->fuzz=fuzz;
  status=OpaquePaintImage(wand->images,&target_pixel,&fill_pixel,invert,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k O p t i m i z e I m a g e L a y e r s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickOptimizeImageLayers() compares each image the GIF disposed forms of the
%  previous image in the sequence.  From this it attempts to select the
%  smallest cropped image to replace each frame, while preserving the results
%  of the animation.
%
%  The format of the MagickOptimizeImageLayers method is:
%
%      MagickWand *MagickOptimizeImageLayers(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickWand *MagickOptimizeImageLayers(MagickWand *wand)
{
  Image
    *optimize_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  optimize_image=OptimizeImageLayers(wand->images,wand->exception);
  if (optimize_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,optimize_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k O p t i m i z e I m a g e T r a n s p a r e n c y             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickOptimizeImageTransparency() takes a frame optimized GIF animation, and
%  compares the overlayed pixels against the disposal image resulting from all
%  the previous frames in the animation.  Any pixel that does not change the
%  disposal image (and thus does not effect the outcome of an overlay) is made
%  transparent.
%
%  WARNING: This modifies the current images directly, rather than generate
%  a new image sequence.
%  The format of the MagickOptimizeImageTransparency method is:
%
%      MagickBooleanType MagickOptimizeImageTransparency(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickOptimizeImageTransparency(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return(MagickFalse);
  OptimizeImageTransparency(wand->images,wand->exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a g i c k O r d e r e d D i t h e r I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickOrderedDitherImage() performs an ordered dither based on a number
%  of pre-defined dithering threshold maps, but over multiple intensity levels,
%  which can be different for different channels, according to the input
%  arguments.
%
%  The format of the MagickOrderedDitherImage method is:
%
%      MagickBooleanType MagickOrderedDitherImage(MagickWand *wand,
%        const char *threshold_map)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold_map: A string containing the name of the threshold dither
%      map to use, followed by zero or more numbers representing the number of
%      color levels tho dither between.
%
%      Any level number less than 2 is equivalent to 2, and means only binary
%      dithering will be applied to each color channel.
%
%      No numbers also means a 2 level (bitmap) dither will be applied to all
%      channels, while a single number is the number of levels applied to each
%      channel in sequence.  More numbers will be applied in turn to each of
%      the color channels.
%
%      For example: "o3x3,6" generates a 6 level posterization of the image
%      with a ordered 3x3 diffused pixel dither being applied between each
%      level. While checker,8,8,4 will produce a 332 colormaped image with
%      only a single checkerboard hash pattern (50% grey) between each color
%      level, to basically double the number of color levels with a bare
%      minimim of dithering.
%
*/
WandExport MagickBooleanType MagickOrderedDitherImage(MagickWand *wand,
  const char *threshold_map)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=OrderedDitherImage(wand->images,threshold_map,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P i n g I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPingImage() is the same as MagickReadImage() except the only valid
%  information returned is the image width, height, size, and format.  It
%  is designed to efficiently obtain this information from a file without
%  reading the entire image sequence into memory.
%
%  The format of the MagickPingImage method is:
%
%      MagickBooleanType MagickPingImage(MagickWand *wand,const char *filename)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o filename: the image filename.
%
*/
WandExport MagickBooleanType MagickPingImage(MagickWand *wand,
  const char *filename)
{
  Image
    *images;

  ImageInfo
    *ping_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  ping_info=CloneImageInfo(wand->image_info);
  if (filename != (const char *) NULL)
    (void) CopyMagickString(ping_info->filename,filename,MagickPathExtent);
  images=PingImage(ping_info,wand->exception);
  ping_info=DestroyImageInfo(ping_info);
  if (images == (Image *) NULL)
    return(MagickFalse);
  return(InsertImageInWand(wand,images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P i n g I m a g e B l o b                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPingImageBlob() pings an image or image sequence from a blob.
%
%  The format of the MagickPingImageBlob method is:
%
%      MagickBooleanType MagickPingImageBlob(MagickWand *wand,
%        const void *blob,const size_t length)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o blob: the blob.
%
%    o length: the blob length.
%
*/
WandExport MagickBooleanType MagickPingImageBlob(MagickWand *wand,
  const void *blob,const size_t length)
{
  Image
    *images;

  ImageInfo
    *read_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  read_info=CloneImageInfo(wand->image_info);
  SetImageInfoBlob(read_info,blob,length);
  images=PingImage(read_info,wand->exception);
  read_info=DestroyImageInfo(read_info);
  if (images == (Image *) NULL)
    return(MagickFalse);
  return(InsertImageInWand(wand,images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P i n g I m a g e F i l e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPingImageFile() pings an image or image sequence from an open file
%  descriptor.
%
%  The format of the MagickPingImageFile method is:
%
%      MagickBooleanType MagickPingImageFile(MagickWand *wand,FILE *file)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o file: the file descriptor.
%
*/
WandExport MagickBooleanType MagickPingImageFile(MagickWand *wand,FILE *file)
{
  Image
    *images;

  ImageInfo
    *read_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  assert(file != (FILE *) NULL);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  read_info=CloneImageInfo(wand->image_info);
  SetImageInfoFile(read_info,file);
  images=PingImage(read_info,wand->exception);
  read_info=DestroyImageInfo(read_info);
  if (images == (Image *) NULL)
    return(MagickFalse);
  return(InsertImageInWand(wand,images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P o l a r o i d I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPolaroidImage() simulates a Polaroid picture.
%
%  The format of the MagickPolaroidImage method is:
%
%      MagickBooleanType MagickPolaroidImage(MagickWand *wand,
%        const DrawingWand *drawing_wand,const char *caption,const double angle,
%        const PixelInterpolateMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o drawing_wand: the draw wand.
%
%    o caption: the Polaroid caption.
%
%    o angle: Apply the effect along this angle.
%
%    o method: the pixel interpolation method.
%
*/
WandExport MagickBooleanType MagickPolaroidImage(MagickWand *wand,
  const DrawingWand *drawing_wand,const char *caption,const double angle,
  const PixelInterpolateMethod method)
{
  DrawInfo
    *draw_info;

  Image
    *polaroid_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  draw_info=PeekDrawingWand(drawing_wand);
  if (draw_info == (DrawInfo *) NULL)
    return(MagickFalse);
  polaroid_image=PolaroidImage(wand->images,draw_info,caption,angle,method,
    wand->exception);
  if (polaroid_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,polaroid_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P o s t e r i z e I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPosterizeImage() reduces the image to a limited number of color level.
%
%  The format of the MagickPosterizeImage method is:
%
%      MagickBooleanType MagickPosterizeImage(MagickWand *wand,
%        const size_t levels,const DitherMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o levels: Number of color levels allowed in each channel.  Very low values
%      (2, 3, or 4) have the most visible effect.
%
%    o method: choose the dither method: UndefinedDitherMethod,
%      NoDitherMethod, RiemersmaDitherMethod, or FloydSteinbergDitherMethod.
%
*/
WandExport MagickBooleanType MagickPosterizeImage(MagickWand *wand,
  const size_t levels,const DitherMethod dither)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=PosterizeImage(wand->images,levels,dither,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P r e v i e w I m a g e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPreviewImages() tiles 9 thumbnails of the specified image with an
%  image processing operation applied at varying strengths.  This helpful
%  to quickly pin-point an appropriate parameter for an image processing
%  operation.
%
%  The format of the MagickPreviewImages method is:
%
%      MagickWand *MagickPreviewImages(MagickWand *wand,
%        const PreviewType preview)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o preview: the preview type.
%
*/
WandExport MagickWand *MagickPreviewImages(MagickWand *wand,
  const PreviewType preview)
{
  Image
    *preview_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  preview_image=PreviewImage(wand->images,preview,wand->exception);
  if (preview_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,preview_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k P r e v i o u s I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickPreviousImage() sets the previous image in the wand as the current
%  image.
%
%  It is typically used after MagickSetLastIterator(), after which its first
%  use will set the last image as the current image (unless the wand is empty).
%
%  It will return MagickFalse when no more images are left to be returned
%  which happens when the wand is empty, or the current image is the first
%  image.  At that point the iterator is than reset to again process images in
%  the forward direction, again starting with the first image in list. Images
%  added at this point are prepended.
%
%  Also at that point any images added to the wand using MagickAddImages() or
%  MagickReadImages() will be prepended before the first image. In this sense
%  the condition is not quite exactly the same as MagickResetIterator().
%
%  The format of the MagickPreviousImage method is:
%
%      MagickBooleanType MagickPreviousImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickPreviousImage(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if (wand->image_pending != MagickFalse)
    {
      wand->image_pending=MagickFalse;  /* image returned no longer pending */
      return(MagickTrue);
    }
  if (GetPreviousImageInList(wand->images) == (Image *) NULL)
    {
      wand->image_pending=MagickTrue;   /* Next now re-gets first image */
      wand->insert_before=MagickTrue;   /* insert/add prepends new images */
      return(MagickFalse);
    }
  wand->images=GetPreviousImageInList(wand->images);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k Q u a n t i z e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickQuantizeImage() analyzes the colors within a reference image and
%  chooses a fixed number of colors to represent the image.  The goal of the
%  algorithm is to minimize the color difference between the input and output
%  image while minimizing the processing time.
%
%  The format of the MagickQuantizeImage method is:
%
%      MagickBooleanType MagickQuantizeImage(MagickWand *wand,
%        const size_t number_colors,const ColorspaceType colorspace,
%        const size_t treedepth,const DitherMethod dither_method,
%        const MagickBooleanType measure_error)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o number_colors: the number of colors.
%
%    o colorspace: Perform color reduction in this colorspace, typically
%      RGBColorspace.
%
%    o treedepth: Normally, this integer value is zero or one.  A zero or
%      one tells Quantize to choose a optimal tree depth of Log4(number_colors).%      A tree of this depth generally allows the best representation of the
%      reference image with the least amount of memory and the fastest
%      computational speed.  In some cases, such as an image with low color
%      dispersion (a few number of colors), a value other than
%      Log4(number_colors) is required.  To expand the color tree completely,
%      use a value of 8.
%
%    o dither_method: choose from UndefinedDitherMethod, NoDitherMethod,
%      RiemersmaDitherMethod, FloydSteinbergDitherMethod.
%
%    o measure_error: A value other than zero measures the difference between
%      the original and quantized images.  This difference is the total
%      quantization error.  The error is computed by summing over all pixels
%      in an image the distance squared in RGB space between each reference
%      pixel value and its quantized value.
%
*/
WandExport MagickBooleanType MagickQuantizeImage(MagickWand *wand,
  const size_t number_colors,const ColorspaceType colorspace,
  const size_t treedepth,const DitherMethod dither_method,
  const MagickBooleanType measure_error)
{
  MagickBooleanType
    status;

  QuantizeInfo
    *quantize_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  quantize_info=CloneQuantizeInfo((QuantizeInfo *) NULL);
  quantize_info->number_colors=number_colors;
  quantize_info->dither_method=dither_method;
  quantize_info->tree_depth=treedepth;
  quantize_info->colorspace=colorspace;
  quantize_info->measure_error=measure_error;
  status=QuantizeImage(quantize_info,wand->images,wand->exception);
  quantize_info=DestroyQuantizeInfo(quantize_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k Q u a n t i z e I m a g e s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickQuantizeImages() analyzes the colors within a sequence of images and
%  chooses a fixed number of colors to represent the image.  The goal of the
%  algorithm is to minimize the color difference between the input and output
%  image while minimizing the processing time.
%
%  The format of the MagickQuantizeImages method is:
%
%      MagickBooleanType MagickQuantizeImages(MagickWand *wand,
%        const size_t number_colors,const ColorspaceType colorspace,
%        const size_t treedepth,const DitherMethod dither_method,
%        const MagickBooleanType measure_error)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o number_colors: the number of colors.
%
%    o colorspace: Perform color reduction in this colorspace, typically
%      RGBColorspace.
%
%    o treedepth: Normally, this integer value is zero or one.  A zero or
%      one tells Quantize to choose a optimal tree depth of Log4(number_colors).%      A tree of this depth generally allows the best representation of the
%      reference image with the least amount of memory and the fastest
%      computational speed.  In some cases, such as an image with low color
%      dispersion (a few number of colors), a value other than
%      Log4(number_colors) is required.  To expand the color tree completely,
%      use a value of 8.
%
%    o dither_method: choose from these dither methods: NoDitherMethod,
%      RiemersmaDitherMethod, or FloydSteinbergDitherMethod.
%
%    o measure_error: A value other than zero measures the difference between
%      the original and quantized images.  This difference is the total
%      quantization error.  The error is computed by summing over all pixels
%      in an image the distance squared in RGB space between each reference
%      pixel value and its quantized value.
%
*/
WandExport MagickBooleanType MagickQuantizeImages(MagickWand *wand,
  const size_t number_colors,const ColorspaceType colorspace,
  const size_t treedepth,const DitherMethod dither_method,
  const MagickBooleanType measure_error)
{
  MagickBooleanType
    status;

  QuantizeInfo
    *quantize_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  quantize_info=CloneQuantizeInfo((QuantizeInfo *) NULL);
  quantize_info->number_colors=number_colors;
  quantize_info->dither_method=dither_method;
  quantize_info->tree_depth=treedepth;
  quantize_info->colorspace=colorspace;
  quantize_info->measure_error=measure_error;
  status=QuantizeImages(quantize_info,wand->images,wand->exception);
  quantize_info=DestroyQuantizeInfo(quantize_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R o t a t i o n a l B l u r I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRotationalBlurImage() rotational blurs an image.
%
%  The format of the MagickRotationalBlurImage method is:
%
%      MagickBooleanType MagickRotationalBlurImage(MagickWand *wand,
%        const double angle)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o angle: the angle of the blur in degrees.
%
*/
WandExport MagickBooleanType MagickRotationalBlurImage(MagickWand *wand,
  const double angle)
{
  Image
    *blur_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  blur_image=RotationalBlurImage(wand->images,angle,wand->exception);
  if (blur_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,blur_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R a i s e I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRaiseImage() creates a simulated three-dimensional button-like effect
%  by lightening and darkening the edges of the image.  Members width and
%  height of raise_info define the width of the vertical and horizontal
%  edge of the effect.
%
%  The format of the MagickRaiseImage method is:
%
%      MagickBooleanType MagickRaiseImage(MagickWand *wand,
%        const size_t width,const size_t height,const ssize_t x,
%        const ssize_t y,const MagickBooleanType raise)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o width,height,x,y:  Define the dimensions of the area to raise.
%
%    o raise: A value other than zero creates a 3-D raise effect,
%      otherwise it has a lowered effect.
%
*/
WandExport MagickBooleanType MagickRaiseImage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y,const MagickBooleanType raise)
{
  MagickBooleanType
    status;

  RectangleInfo
    raise_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  raise_info.width=width;
  raise_info.height=height;
  raise_info.x=x;
  raise_info.y=y;
  status=RaiseImage(wand->images,&raise_info,raise,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R a n d o m T h r e s h o l d I m a g e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRandomThresholdImage() changes the value of individual pixels based on
%  the intensity of each pixel compared to threshold.  The result is a
%  high-contrast, two color image.
%
%  The format of the MagickRandomThresholdImage method is:
%
%      MagickBooleanType MagickRandomThresholdImage(MagickWand *wand,
%        const double low,const double high)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o low,high: Specify the high and low thresholds. These values range from
%      0 to QuantumRange.
%
*/
WandExport MagickBooleanType MagickRandomThresholdImage(MagickWand *wand,
  const double low,const double high)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(RandomThresholdImage(wand->images,low,high,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e a d I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickReadImage() reads an image or image sequence.  The images are inserted
%  jjust before the current image pointer position.
%
%  Use MagickSetFirstIterator(), to insert new images before all the current
%  images in the wand, MagickSetLastIterator() to append add to the end,
%  MagickSetIteratorIndex() to place images just after the given index.
%
%  The format of the MagickReadImage method is:
%
%      MagickBooleanType MagickReadImage(MagickWand *wand,const char *filename)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o filename: the image filename.
%
*/
WandExport MagickBooleanType MagickReadImage(MagickWand *wand,
  const char *filename)
{
  Image
    *images;

  ImageInfo
    *read_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  read_info=CloneImageInfo(wand->image_info);
  if (filename != (const char *) NULL)
    (void) CopyMagickString(read_info->filename,filename,MagickPathExtent);
  images=ReadImage(read_info,wand->exception);
  read_info=DestroyImageInfo(read_info);
  if (images == (Image *) NULL)
    return(MagickFalse);
  return(InsertImageInWand(wand,images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e a d I m a g e B l o b                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickReadImageBlob() reads an image or image sequence from a blob.
%  In all other respects it is like MagickReadImage().
%
%  The format of the MagickReadImageBlob method is:
%
%      MagickBooleanType MagickReadImageBlob(MagickWand *wand,
%        const void *blob,const size_t length)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o blob: the blob.
%
%    o length: the blob length.
%
*/
WandExport MagickBooleanType MagickReadImageBlob(MagickWand *wand,
  const void *blob,const size_t length)
{
  Image
    *images;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  images=BlobToImage(wand->image_info,blob,length,wand->exception);
  if (images == (Image *) NULL)
    return(MagickFalse);
  return(InsertImageInWand(wand,images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e a d I m a g e F i l e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickReadImageFile() reads an image or image sequence from an already
%  opened file descriptor.  Otherwise it is like MagickReadImage().
%
%  The format of the MagickReadImageFile method is:
%
%      MagickBooleanType MagickReadImageFile(MagickWand *wand,FILE *file)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o file: the file descriptor.
%
*/
WandExport MagickBooleanType MagickReadImageFile(MagickWand *wand,FILE *file)
{
  Image
    *images;

  ImageInfo
    *read_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  assert(file != (FILE *) NULL);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  read_info=CloneImageInfo(wand->image_info);
  SetImageInfoFile(read_info,file);
  images=ReadImage(read_info,wand->exception);
  read_info=DestroyImageInfo(read_info);
  if (images == (Image *) NULL)
    return(MagickFalse);
  return(InsertImageInWand(wand,images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e m a p I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRemapImage() replaces the colors of an image with the closest color
%  from a reference image.
%
%  The format of the MagickRemapImage method is:
%
%      MagickBooleanType MagickRemapImage(MagickWand *wand,
%        const MagickWand *remap_wand,const DitherMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o affinity: the affinity wand.
%
%    o method: choose from these dither methods: NoDitherMethod,
%      RiemersmaDitherMethod, or FloydSteinbergDitherMethod.
%
*/
WandExport MagickBooleanType MagickRemapImage(MagickWand *wand,
  const MagickWand *remap_wand,const DitherMethod dither_method)
{
  MagickBooleanType
    status;

  QuantizeInfo
    *quantize_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) ||
      (remap_wand->images == (Image *) NULL))
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  quantize_info=AcquireQuantizeInfo(wand->image_info);
  quantize_info->dither_method=dither_method;
  status=RemapImage(quantize_info,wand->images,remap_wand->images,
    wand->exception);
  quantize_info=DestroyQuantizeInfo(quantize_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e m o v e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRemoveImage() removes an image from the image list.
%
%  The format of the MagickRemoveImage method is:
%
%      MagickBooleanType MagickRemoveImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o insert: the splice wand.
%
*/
WandExport MagickBooleanType MagickRemoveImage(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  DeleteImageFromList(&wand->images);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e s a m p l e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickResampleImage() resample image to desired resolution.
%
%    Bessel   Blackman   Box
%    Catrom   Cubic      Gaussian
%    Hanning  Hermite    Lanczos
%    Mitchell Point      Quandratic
%    Sinc     Triangle
%
%  Most of the filters are FIR (finite impulse response), however, Bessel,
%  Gaussian, and Sinc are IIR (infinite impulse response).  Bessel and Sinc
%  are windowed (brought down to zero) with the Blackman filter.
%
%  The format of the MagickResampleImage method is:
%
%      MagickBooleanType MagickResampleImage(MagickWand *wand,
%        const double x_resolution,const double y_resolution,
%        const FilterType filter)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x_resolution: the new image x resolution.
%
%    o y_resolution: the new image y resolution.
%
%    o filter: Image filter to use.
%
*/
WandExport MagickBooleanType MagickResampleImage(MagickWand *wand,
  const double x_resolution,const double y_resolution,const FilterType filter)
{
  Image
    *resample_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  resample_image=ResampleImage(wand->images,x_resolution,y_resolution,filter,
    wand->exception);
  if (resample_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,resample_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e s e t I m a g e P a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickResetImagePage() resets the Wand page canvas and position.
%
%  The format of the MagickResetImagePage method is:
%
%      MagickBooleanType MagickResetImagePage(MagickWand *wand,
%        const char *page)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o page: the relative page specification.
%
*/
WandExport MagickBooleanType MagickResetImagePage(MagickWand *wand,
  const char *page)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if ((page == (char *) NULL) || (*page == '\0'))
    {
      (void) ParseAbsoluteGeometry("0x0+0+0",&wand->images->page);
      return(MagickTrue);
    }
  return(ResetImagePage(wand->images,page));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R e s i z e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickResizeImage() scales an image to the desired dimensions with one of
%  these filters:
%
%    Bessel   Blackman   Box
%    Catrom   Cubic      Gaussian
%    Hanning  Hermite    Lanczos
%    Mitchell Point      Quandratic
%    Sinc     Triangle
%
%  Most of the filters are FIR (finite impulse response), however, Bessel,
%  Gaussian, and Sinc are IIR (infinite impulse response).  Bessel and Sinc
%  are windowed (brought down to zero) with the Blackman filter.
%
%  The format of the MagickResizeImage method is:
%
%      MagickBooleanType MagickResizeImage(MagickWand *wand,
%        const size_t columns,const size_t rows,const FilterType filter)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
%    o filter: Image filter to use.
%
*/
WandExport MagickBooleanType MagickResizeImage(MagickWand *wand,
  const size_t columns,const size_t rows,const FilterType filter)
{
  Image
    *resize_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  resize_image=ResizeImage(wand->images,columns,rows,filter,wand->exception);
  if (resize_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,resize_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R o l l I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRollImage() offsets an image as defined by x and y.
%
%  The format of the MagickRollImage method is:
%
%      MagickBooleanType MagickRollImage(MagickWand *wand,const ssize_t x,
%        const size_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x: the x offset.
%
%    o y: the y offset.
%
%
*/
WandExport MagickBooleanType MagickRollImage(MagickWand *wand,
  const ssize_t x,const ssize_t y)
{
  Image
    *roll_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  roll_image=RollImage(wand->images,x,y,wand->exception);
  if (roll_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,roll_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k R o t a t e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickRotateImage() rotates an image the specified number of degrees. Empty
%  triangles left over from rotating the image are filled with the
%  background color.
%
%  The format of the MagickRotateImage method is:
%
%      MagickBooleanType MagickRotateImage(MagickWand *wand,
%        const PixelWand *background,const double degrees)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o background: the background pixel wand.
%
%    o degrees: the number of degrees to rotate the image.
%
%
*/
WandExport MagickBooleanType MagickRotateImage(MagickWand *wand,
  const PixelWand *background,const double degrees)
{
  Image
    *rotate_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  PixelGetQuantumPacket(background,&wand->images->background_color);
  rotate_image=RotateImage(wand->images,degrees,wand->exception);
  if (rotate_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,rotate_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S a m p l e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSampleImage() scales an image to the desired dimensions with pixel
%  sampling.  Unlike other scaling methods, this method does not introduce
%  any additional color into the scaled image.
%
%  The format of the MagickSampleImage method is:
%
%      MagickBooleanType MagickSampleImage(MagickWand *wand,
%        const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
%
*/
WandExport MagickBooleanType MagickSampleImage(MagickWand *wand,
  const size_t columns,const size_t rows)
{
  Image
    *sample_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  sample_image=SampleImage(wand->images,columns,rows,wand->exception);
  if (sample_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,sample_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S c a l e I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickScaleImage() scales the size of an image to the given dimensions.
%
%  The format of the MagickScaleImage method is:
%
%      MagickBooleanType MagickScaleImage(MagickWand *wand,
%        const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
%
*/
WandExport MagickBooleanType MagickScaleImage(MagickWand *wand,
  const size_t columns,const size_t rows)
{
  Image
    *scale_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  scale_image=ScaleImage(wand->images,columns,rows,wand->exception);
  if (scale_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,scale_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e g m e n t I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSegmentImage() segments an image by analyzing the histograms of the
%  color components and identifying units that are homogeneous with the fuzzy
%  C-means technique.
%
%  The format of the SegmentImage method is:
%
%      MagickBooleanType MagickSegmentImage(MagickWand *wand,
%        const ColorspaceType colorspace,const MagickBooleanType verbose,
%        const double cluster_threshold,const double smooth_threshold)
%
%  A description of each parameter follows.
%
%    o wand: the wand.
%
%    o colorspace: the image colorspace.
%
%    o verbose:  Set to MagickTrue to print detailed information about the
%      identified classes.
%
%    o cluster_threshold:  This represents the minimum number of pixels
%      contained in a hexahedra before it can be considered valid (expressed as
%      a percentage).
%
%    o smooth_threshold: the smoothing threshold eliminates noise in the second
%      derivative of the histogram.  As the value is increased, you can expect a
%      smoother second derivative.
%
*/
MagickExport MagickBooleanType MagickSegmentImage(MagickWand *wand,
  const ColorspaceType colorspace,const MagickBooleanType verbose,
  const double cluster_threshold,const double smooth_threshold)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=SegmentImage(wand->images,colorspace,verbose,cluster_threshold,
    smooth_threshold,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e l e c t i v e B l u r I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSelectiveBlurImage() selectively blur an image within a contrast
%  threshold. It is similar to the unsharpen mask that sharpens everything with
%  contrast above a certain threshold.
%
%  The format of the MagickSelectiveBlurImage method is:
%
%      MagickBooleanType MagickSelectiveBlurImage(MagickWand *wand,
%        const double radius,const double sigma,const double threshold)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the gaussian, in pixels.
%
%    o threshold: only pixels within this contrast threshold are included
%      in the blur operation.
%
*/
WandExport MagickBooleanType MagickSelectiveBlurImage(MagickWand *wand,
  const double radius,const double sigma,const double threshold)
{
  Image
    *blur_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  blur_image=SelectiveBlurImage(wand->images,radius,sigma,threshold,
    wand->exception);
  if (blur_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,blur_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e p a r a t e I m a g e C h a n n e l                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSeparateImage() separates a channel from the image and returns a
%  grayscale image.  A channel is a particular color component of each pixel
%  in the image.
%
%  The format of the MagickSeparateImage method is:
%
%      MagickBooleanType MagickSeparateImage(MagickWand *wand,
%        const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the channel.
%
*/
WandExport MagickBooleanType MagickSeparateImage(MagickWand *wand,
  const ChannelType channel)
{
  Image
    *separate_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  separate_image=SeparateImage(wand->images,channel,wand->exception);
  if (separate_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,separate_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a g i c k S e p i a T o n e I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSepiaToneImage() applies a special effect to the image, similar to the
%  effect achieved in a photo darkroom by sepia toning.  Threshold ranges from
%  0 to QuantumRange and is a measure of the extent of the sepia toning.  A
%  threshold of 80% is a good starting point for a reasonable tone.
%
%  The format of the MagickSepiaToneImage method is:
%
%      MagickBooleanType MagickSepiaToneImage(MagickWand *wand,
%        const double threshold)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o threshold:  Define the extent of the sepia toning.
%
*/
WandExport MagickBooleanType MagickSepiaToneImage(MagickWand *wand,
  const double threshold)
{
  Image
    *sepia_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  sepia_image=SepiaToneImage(wand->images,threshold,wand->exception);
  if (sepia_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,sepia_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImage() replaces the last image returned by MagickSetIteratorIndex(),
%  MagickNextImage(), MagickPreviousImage() with the images from the specified
%  wand.
%
%  The format of the MagickSetImage method is:
%
%      MagickBooleanType MagickSetImage(MagickWand *wand,
%        const MagickWand *set_wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o set_wand: the set_wand wand.
%
*/
WandExport MagickBooleanType MagickSetImage(MagickWand *wand,
  const MagickWand *set_wand)
{
  Image
    *images;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(set_wand != (MagickWand *) NULL);
  assert(set_wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",set_wand->name);
  if (set_wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  images=CloneImageList(set_wand->images,wand->exception);
  if (images == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,images);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e A l p h a C h a n n e l                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageAlphaChannel() activates, deactivates, resets, or sets the
%  alpha channel.
%
%  The format of the MagickSetImageAlphaChannel method is:
%
%      MagickBooleanType MagickSetImageAlphaChannel(MagickWand *wand,
%        const AlphaChannelOption alpha_type)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o alpha_type: the alpha channel type: ActivateAlphaChannel,
%      DeactivateAlphaChannel, OpaqueAlphaChannel, or SetAlphaChannel.
%
*/
WandExport MagickBooleanType MagickSetImageAlphaChannel(MagickWand *wand,
  const AlphaChannelOption alpha_type)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(SetImageAlphaChannel(wand->images,alpha_type,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e B a c k g r o u n d C o l o r                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageBackgroundColor() sets the image background color.
%
%  The format of the MagickSetImageBackgroundColor method is:
%
%      MagickBooleanType MagickSetImageBackgroundColor(MagickWand *wand,
%        const PixelWand *background)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o background: the background pixel wand.
%
*/
WandExport MagickBooleanType MagickSetImageBackgroundColor(MagickWand *wand,
  const PixelWand *background)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  PixelGetQuantumPacket(background,&wand->images->background_color);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e B l u e P r i m a r y                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageBluePrimary() sets the image chromaticity blue primary point.
%
%  The format of the MagickSetImageBluePrimary method is:
%
%      MagickBooleanType MagickSetImageBluePrimary(MagickWand *wand,
%        const double x,const double y,const double z)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x: the blue primary x-point.
%
%    o y: the blue primary y-point.
%
%    o z: the blue primary z-point.
%
*/
WandExport MagickBooleanType MagickSetImageBluePrimary(MagickWand *wand,
  const double x,const double y,const double z)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->chromaticity.blue_primary.x=x;
  wand->images->chromaticity.blue_primary.y=y;
  wand->images->chromaticity.blue_primary.z=z;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e B o r d e r C o l o r                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageBorderColor() sets the image border color.
%
%  The format of the MagickSetImageBorderColor method is:
%
%      MagickBooleanType MagickSetImageBorderColor(MagickWand *wand,
%        const PixelWand *border)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o border: the border pixel wand.
%
*/
WandExport MagickBooleanType MagickSetImageBorderColor(MagickWand *wand,
  const PixelWand *border)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  PixelGetQuantumPacket(border,&wand->images->border_color);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e C h a n n e l M a s k                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageChannelMask() sets image channel mask.
%
%  The format of the MagickSetImageChannelMask method is:
%
%      ChannelType MagickSetImageChannelMask(MagickWand *wand,
%        const ChannelType channel_mask)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel_mask: the channel_mask wand.
%
*/
WandExport ChannelType MagickSetImageChannelMask(MagickWand *wand,
  const ChannelType channel_mask)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  return(SetImageChannelMask(wand->images,channel_mask));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e M a s k                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageMask() sets image clip mask.
%
%  The format of the MagickSetImageMask method is:
%
%      MagickBooleanType MagickSetImageMask(MagickWand *wand,
%        const PixelMask type,const MagickWand *clip_mask)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o type: type of mask, ReadPixelMask or WritePixelMask.
%
%    o clip_mask: the clip_mask wand.
%
*/
WandExport MagickBooleanType MagickSetImageMask(MagickWand *wand,
  const PixelMask type,const MagickWand *clip_mask)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  assert(clip_mask != (MagickWand *) NULL);
  assert(clip_mask->signature == MagickWandSignature);
  if (clip_mask->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",clip_mask->name);
  if (clip_mask->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",clip_mask->name);
  return(SetImageMask(wand->images,type,clip_mask->images,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e C o l o r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageColor() set the entire wand canvas to the specified color.
%
%  The format of the MagickSetImageColor method is:
%
%      MagickBooleanType MagickSetImageColor(MagickWand *wand,
%        const PixelWand *color)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o background: the image color.
%
*/
WandExport MagickBooleanType MagickSetImageColor(MagickWand *wand,
  const PixelWand *color)
{
  PixelInfo
    pixel;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  PixelGetMagickColor(color,&pixel);
  return(SetImageColor(wand->images,&pixel,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e C o l o r m a p C o l o r                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageColormapColor() sets the color of the specified colormap
%  index.
%
%  The format of the MagickSetImageColormapColor method is:
%
%      MagickBooleanType MagickSetImageColormapColor(MagickWand *wand,
%        const size_t index,const PixelWand *color)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o index: the offset into the image colormap.
%
%    o color: Return the colormap color in this wand.
%
*/
WandExport MagickBooleanType MagickSetImageColormapColor(MagickWand *wand,
  const size_t index,const PixelWand *color)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if ((wand->images->colormap == (PixelInfo *) NULL) ||
      (index >= wand->images->colors))
    ThrowWandException(WandError,"InvalidColormapIndex",wand->name);
  PixelGetQuantumPacket(color,wand->images->colormap+index);
  return(SyncImage(wand->images,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e C o l o r s p a c e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageColorspace() sets the image colorspace. But does not modify
%  the image data.
%
%  The format of the MagickSetImageColorspace method is:
%
%      MagickBooleanType MagickSetImageColorspace(MagickWand *wand,
%        const ColorspaceType colorspace)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o colorspace: the image colorspace:   UndefinedColorspace, RGBColorspace,
%      GRAYColorspace, TransparentColorspace, OHTAColorspace, XYZColorspace,
%      YCbCrColorspace, YCCColorspace, YIQColorspace, YPbPrColorspace,
%      YPbPrColorspace, YUVColorspace, CMYKColorspace, sRGBColorspace,
%      HSLColorspace, or HWBColorspace.
%
*/
WandExport MagickBooleanType MagickSetImageColorspace(MagickWand *wand,
  const ColorspaceType colorspace)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(SetImageColorspace(wand->images,colorspace,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e C o m p o s e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageCompose() sets the image composite operator, useful for
%  specifying how to composite the image thumbnail when using the
%  MagickMontageImage() method.
%
%  The format of the MagickSetImageCompose method is:
%
%      MagickBooleanType MagickSetImageCompose(MagickWand *wand,
%        const CompositeOperator compose)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o compose: the image composite operator.
%
*/
WandExport MagickBooleanType MagickSetImageCompose(MagickWand *wand,
  const CompositeOperator compose)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->compose=compose;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e C o m p r e s s i o n                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageCompression() sets the image compression.
%
%  The format of the MagickSetImageCompression method is:
%
%      MagickBooleanType MagickSetImageCompression(MagickWand *wand,
%        const CompressionType compression)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o compression: the image compression type.
%
*/
WandExport MagickBooleanType MagickSetImageCompression(MagickWand *wand,
  const CompressionType compression)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->compression=compression;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e C o m p r e s s i o n Q u a l i t y           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageCompressionQuality() sets the image compression quality.
%
%  The format of the MagickSetImageCompressionQuality method is:
%
%      MagickBooleanType MagickSetImageCompressionQuality(MagickWand *wand,
%        const size_t quality)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o quality: the image compression tlityype.
%
*/
WandExport MagickBooleanType MagickSetImageCompressionQuality(MagickWand *wand,
  const size_t quality)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->quality=quality;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e D e l a y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageDelay() sets the image delay.
%
%  The format of the MagickSetImageDelay method is:
%
%      MagickBooleanType MagickSetImageDelay(MagickWand *wand,
%        const size_t delay)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o delay: the image delay in ticks-per-second units.
%
*/
WandExport MagickBooleanType MagickSetImageDelay(MagickWand *wand,
  const size_t delay)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->delay=delay;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e D e p t h                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageDepth() sets the image depth.
%
%  The format of the MagickSetImageDepth method is:
%
%      MagickBooleanType MagickSetImageDepth(MagickWand *wand,
%        const size_t depth)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o depth: the image depth in bits: 8, 16, or 32.
%
*/
WandExport MagickBooleanType MagickSetImageDepth(MagickWand *wand,
  const size_t depth)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(SetImageDepth(wand->images,depth,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e D i s p o s e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageDispose() sets the image disposal method.
%
%  The format of the MagickSetImageDispose method is:
%
%      MagickBooleanType MagickSetImageDispose(MagickWand *wand,
%        const DisposeType dispose)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o dispose: the image disposeal type.
%
*/
WandExport MagickBooleanType MagickSetImageDispose(MagickWand *wand,
  const DisposeType dispose)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->dispose=dispose;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e E n d i a n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageEndian() sets the image endian method.
%
%  The format of the MagickSetImageEndian method is:
%
%      MagickBooleanType MagickSetImageEndian(MagickWand *wand,
%        const EndianType endian)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o endian: the image endian type.
%
*/
WandExport MagickBooleanType MagickSetImageEndian(MagickWand *wand,
  const EndianType endian)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->endian=endian;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e E x t e n t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageExtent() sets the image size (i.e. columns & rows).
%
%  The format of the MagickSetImageExtent method is:
%
%      MagickBooleanType MagickSetImageExtent(MagickWand *wand,
%        const size_t columns,const unsigned rows)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o columns:  The image width in pixels.
%
%    o rows:  The image height in pixels.
%
*/
WandExport MagickBooleanType MagickSetImageExtent(MagickWand *wand,
  const size_t columns,const size_t rows)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(SetImageExtent(wand->images,columns,rows,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e F i l e n a m e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageFilename() sets the filename of a particular image in a
%  sequence.
%
%  The format of the MagickSetImageFilename method is:
%
%      MagickBooleanType MagickSetImageFilename(MagickWand *wand,
%        const char *filename)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o filename: the image filename.
%
*/
WandExport MagickBooleanType MagickSetImageFilename(MagickWand *wand,
  const char *filename)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if (filename != (const char *) NULL)
    (void) CopyMagickString(wand->images->filename,filename,MagickPathExtent);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e F o r m a t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageFormat() sets the format of a particular image in a
%  sequence.
%
%  The format of the MagickSetImageFormat method is:
%
%      MagickBooleanType MagickSetImageFormat(MagickWand *wand,
%        const char *format)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o format: the image format.
%
*/
WandExport MagickBooleanType MagickSetImageFormat(MagickWand *wand,
  const char *format)
{
  const MagickInfo
    *magick_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if ((format == (char *) NULL) || (*format == '\0'))
    {
      *wand->images->magick='\0';
      return(MagickTrue);
    }
  magick_info=GetMagickInfo(format,wand->exception);
  if (magick_info == (const MagickInfo *) NULL)
    return(MagickFalse);
  ClearMagickException(wand->exception);
  (void) CopyMagickString(wand->images->magick,format,MagickPathExtent);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e F u z z                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageFuzz() sets the image fuzz.
%
%  The format of the MagickSetImageFuzz method is:
%
%      MagickBooleanType MagickSetImageFuzz(MagickWand *wand,
%        const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o fuzz: the image fuzz.
%
*/
WandExport MagickBooleanType MagickSetImageFuzz(MagickWand *wand,
  const double fuzz)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->fuzz=fuzz;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e G a m m a                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageGamma() sets the image gamma.
%
%  The format of the MagickSetImageGamma method is:
%
%      MagickBooleanType MagickSetImageGamma(MagickWand *wand,
%        const double gamma)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o gamma: the image gamma.
%
*/
WandExport MagickBooleanType MagickSetImageGamma(MagickWand *wand,
  const double gamma)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->gamma=gamma;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e G r a v i t y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageGravity() sets the image gravity type.
%
%  The format of the MagickSetImageGravity method is:
%
%      MagickBooleanType MagickSetImageGravity(MagickWand *wand,
%        const GravityType gravity)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o gravity: positioning gravity (NorthWestGravity, NorthGravity,
%               NorthEastGravity, WestGravity, CenterGravity,
%               EastGravity, SouthWestGravity, SouthGravity,
%               SouthEastGravity)
%
*/
WandExport MagickBooleanType MagickSetImageGravity(MagickWand *wand,
  const GravityType gravity)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->gravity=gravity;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e G r e e n P r i m a r y                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageGreenPrimary() sets the image chromaticity green primary
%  point.
%
%  The format of the MagickSetImageGreenPrimary method is:
%
%      MagickBooleanType MagickSetImageGreenPrimary(MagickWand *wand,
%        const double x,const double y,const double z)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x: the green primary x-point.
%
%    o y: the green primary y-point.
%
%    o z: the green primary z-point.
%
*/
WandExport MagickBooleanType MagickSetImageGreenPrimary(MagickWand *wand,
  const double x,const double y,const double z)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->chromaticity.green_primary.x=x;
  wand->images->chromaticity.green_primary.y=y;
  wand->images->chromaticity.green_primary.z=z;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e I n t e r l a c e S c h e m e                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageInterlaceScheme() sets the image interlace scheme.
%
%  The format of the MagickSetImageInterlaceScheme method is:
%
%      MagickBooleanType MagickSetImageInterlaceScheme(MagickWand *wand,
%        const InterlaceType interlace)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o interlace: the image interlace scheme: NoInterlace, LineInterlace,
%      PlaneInterlace, PartitionInterlace.
%
*/
WandExport MagickBooleanType MagickSetImageInterlaceScheme(MagickWand *wand,
  const InterlaceType interlace)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->interlace=interlace;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e I n t e r p o l a t e M e t h o d             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageInterpolateMethod() sets the image interpolate pixel method.
%
%  The format of the MagickSetImageInterpolateMethod method is:
%
%      MagickBooleanType MagickSetImageInterpolateMethod(MagickWand *wand,
%        const PixelInterpolateMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o method: the image interpole pixel methods: choose from Undefined,
%      Average, Bicubic, Bilinear, Filter, Integer, Mesh, NearestNeighbor.
%
*/

WandExport MagickBooleanType MagickSetImagePixelInterpolateMethod(
  MagickWand *wand,const PixelInterpolateMethod method)
{
  return(MagickSetImageInterpolateMethod(wand,method));
}

WandExport MagickBooleanType MagickSetImageInterpolateMethod(
  MagickWand *wand,const PixelInterpolateMethod method)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->interpolate=method;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e I t e r a t i o n s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageIterations() sets the image iterations.
%
%  The format of the MagickSetImageIterations method is:
%
%      MagickBooleanType MagickSetImageIterations(MagickWand *wand,
%        const size_t iterations)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o delay: the image delay in 1/100th of a second.
%
*/
WandExport MagickBooleanType MagickSetImageIterations(MagickWand *wand,
  const size_t iterations)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->iterations=iterations;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e M a t t e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageMatte() sets the image matte channel.
%
%  The format of the MagickSetImageMatte method is:
%
%      MagickBooleanType MagickSetImageMatte(MagickWand *wand,
%        const MagickBooleanType *matte)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o matte: Set to MagickTrue to enable the image matte channel otherwise
%      MagickFalse.
%
*/
WandExport MagickBooleanType MagickSetImageMatte(MagickWand *wand,
  const MagickBooleanType matte)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if (matte == MagickFalse)
    wand->images->alpha_trait=UndefinedPixelTrait;
  else
    {
      if (wand->images->alpha_trait == UndefinedPixelTrait)
        (void) SetImageAlpha(wand->images,OpaqueAlpha,wand->exception);
      wand->images->alpha_trait=BlendPixelTrait;
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e M a t t e C o l o r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageMatteColor() sets the image alpha color.
%
%  The format of the MagickSetImageMatteColor method is:
%
%      MagickBooleanType MagickSetImageMatteColor(MagickWand *wand,
%        const PixelWand *matte)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o matte: the alpha pixel wand.
%
*/
WandExport MagickBooleanType MagickSetImageMatteColor(MagickWand *wand,
  const PixelWand *alpha)
{
  assert(wand != (MagickWand *)NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent, GetMagickModule(), "%s", wand->name);
  if (wand->images == (Image *)NULL)
    ThrowWandException(WandError, "ContainsNoImages", wand->name);
  PixelGetQuantumPacket(alpha,&wand->images->matte_color);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e O p a c i t y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageAlpha() sets the image to the specified alpha level.
%
%  The format of the MagickSetImageAlpha method is:
%
%      MagickBooleanType MagickSetImageAlpha(MagickWand *wand,
%        const double alpha)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o alpha: the level of transparency: 1.0 is fully opaque and 0.0 is fully
%      transparent.
%
*/
WandExport MagickBooleanType MagickSetImageAlpha(MagickWand *wand,
  const double alpha)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=SetImageAlpha(wand->images,ClampToQuantum(QuantumRange*alpha),
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e O r i e n t a t i o n                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageOrientation() sets the image orientation.
%
%  The format of the MagickSetImageOrientation method is:
%
%      MagickBooleanType MagickSetImageOrientation(MagickWand *wand,
%        const OrientationType orientation)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o orientation: the image orientation type.
%
*/
WandExport MagickBooleanType MagickSetImageOrientation(MagickWand *wand,
  const OrientationType orientation)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->orientation=orientation;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e P a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImagePage() sets the page geometry of the image.
%
%  The format of the MagickSetImagePage method is:
%
%      MagickBooleanType MagickSetImagePage(MagickWand *wand,const size_t width,%        const size_t height,const ssize_t x,const ssize_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o width: the page width.
%
%    o height: the page height.
%
%    o x: the page x-offset.
%
%    o y: the page y-offset.
%
*/
WandExport MagickBooleanType MagickSetImagePage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->page.width=width;
  wand->images->page.height=height;
  wand->images->page.x=x;
  wand->images->page.y=y;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e P r o g r e s s M o n i t o r                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageProgressMonitor() sets the wand image progress monitor to the
%  specified method and returns the previous progress monitor if any.  The
%  progress monitor method looks like this:
%
%    MagickBooleanType MagickProgressMonitor(const char *text,
%      const MagickOffsetType offset,const MagickSizeType span,
%      void *client_data)
%
%  If the progress monitor returns MagickFalse, the current operation is
%  interrupted.
%
%  The format of the MagickSetImageProgressMonitor method is:
%
%      MagickProgressMonitor MagickSetImageProgressMonitor(MagickWand *wand
%        const MagickProgressMonitor progress_monitor,void *client_data)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o progress_monitor: Specifies a pointer to a method to monitor progress
%      of an image operation.
%
%    o client_data: Specifies a pointer to any client data.
%
*/
WandExport MagickProgressMonitor MagickSetImageProgressMonitor(MagickWand *wand,
  const MagickProgressMonitor progress_monitor,void *client_data)
{
  MagickProgressMonitor
    previous_monitor;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((MagickProgressMonitor) NULL);
    }
  previous_monitor=SetImageProgressMonitor(wand->images,
    progress_monitor,client_data);
  return(previous_monitor);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e R e d P r i m a r y                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageRedPrimary() sets the image chromaticity red primary point.
%
%  The format of the MagickSetImageRedPrimary method is:
%
%      MagickBooleanType MagickSetImageRedPrimary(MagickWand *wand,
%        const double x,const double y,const double z)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x: the red primary x-point.
%
%    o y: the red primary y-point.
%
%    o z: the red primary z-point.
%
*/
WandExport MagickBooleanType MagickSetImageRedPrimary(MagickWand *wand,
  const double x,const double y,const double z)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->chromaticity.red_primary.x=x;
  wand->images->chromaticity.red_primary.y=y;
  wand->images->chromaticity.red_primary.z=z;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e R e n d e r i n g I n t e n t                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageRenderingIntent() sets the image rendering intent.
%
%  The format of the MagickSetImageRenderingIntent method is:
%
%      MagickBooleanType MagickSetImageRenderingIntent(MagickWand *wand,
%        const RenderingIntent rendering_intent)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o rendering_intent: the image rendering intent: UndefinedIntent,
%      SaturationIntent, PerceptualIntent, AbsoluteIntent, or RelativeIntent.
%
*/
WandExport MagickBooleanType MagickSetImageRenderingIntent(MagickWand *wand,
  const RenderingIntent rendering_intent)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->rendering_intent=rendering_intent;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e R e s o l u t i o n                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageResolution() sets the image resolution.
%
%  The format of the MagickSetImageResolution method is:
%
%      MagickBooleanType MagickSetImageResolution(MagickWand *wand,
%        const double x_resolution,const double y_resolution)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x_resolution: the image x resolution.
%
%    o y_resolution: the image y resolution.
%
*/
WandExport MagickBooleanType MagickSetImageResolution(MagickWand *wand,
  const double x_resolution,const double y_resolution)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->resolution.x=x_resolution;
  wand->images->resolution.y=y_resolution;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e S c e n e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageScene() sets the image scene.
%
%  The format of the MagickSetImageScene method is:
%
%      MagickBooleanType MagickSetImageScene(MagickWand *wand,
%        const size_t scene)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o delay: the image scene number.
%
*/
WandExport MagickBooleanType MagickSetImageScene(MagickWand *wand,
  const size_t scene)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->scene=scene;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e T i c k s P e r S e c o n d                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageTicksPerSecond() sets the image ticks-per-second.
%
%  The format of the MagickSetImageTicksPerSecond method is:
%
%      MagickBooleanType MagickSetImageTicksPerSecond(MagickWand *wand,
%        const ssize_t ticks_per-second)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o ticks_per_second: the units to use for the image delay.
%
*/
WandExport MagickBooleanType MagickSetImageTicksPerSecond(MagickWand *wand,
  const ssize_t ticks_per_second)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->ticks_per_second=ticks_per_second;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e T y p e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageType() sets the image type.
%
%  The format of the MagickSetImageType method is:
%
%      MagickBooleanType MagickSetImageType(MagickWand *wand,
%        const ImageType image_type)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o image_type: the image type:   UndefinedType, BilevelType, GrayscaleType,
%      GrayscaleAlphaType, PaletteType, PaletteAlphaType, TrueColorType,
%      TrueColorAlphaType, ColorSeparationType, ColorSeparationAlphaType,
%      or OptimizeType.
%
*/
WandExport MagickBooleanType MagickSetImageType(MagickWand *wand,
  const ImageType image_type)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(SetImageType(wand->images,image_type,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e U n i t s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageUnits() sets the image units of resolution.
%
%  The format of the MagickSetImageUnits method is:
%
%      MagickBooleanType MagickSetImageUnits(MagickWand *wand,
%        const ResolutionType units)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o units: the image units of resolution : UndefinedResolution,
%      PixelsPerInchResolution, or PixelsPerCentimeterResolution.
%
*/
WandExport MagickBooleanType MagickSetImageUnits(MagickWand *wand,
  const ResolutionType units)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->units=units;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e V i r t u a l P i x e l M e t h o d           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageVirtualPixelMethod() sets the image virtual pixel method.
%
%  The format of the MagickSetImageVirtualPixelMethod method is:
%
%      VirtualPixelMethod MagickSetImageVirtualPixelMethod(MagickWand *wand,
%        const VirtualPixelMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o method: the image virtual pixel method : UndefinedVirtualPixelMethod,
%      ConstantVirtualPixelMethod,  EdgeVirtualPixelMethod,
%      MirrorVirtualPixelMethod, or TileVirtualPixelMethod.
%
*/
WandExport VirtualPixelMethod MagickSetImageVirtualPixelMethod(MagickWand *wand,
  const VirtualPixelMethod method)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return(UndefinedVirtualPixelMethod);
  return(SetImageVirtualPixelMethod(wand->images,method,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e W h i t e P o i n t                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageWhitePoint() sets the image chromaticity white point.
%
%  The format of the MagickSetImageWhitePoint method is:
%
%      MagickBooleanType MagickSetImageWhitePoint(MagickWand *wand,
%        const double x,const double y,const double z)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o x: the white x-point.
%
%    o y: the white y-point.
%
%    o z: the white z-point.
%
*/
WandExport MagickBooleanType MagickSetImageWhitePoint(MagickWand *wand,
  const double x,const double y,const double z)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->chromaticity.white_point.x=x;
  wand->images->chromaticity.white_point.y=y;
  wand->images->chromaticity.white_point.z=z;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S h a d e I m a g e C h a n n e l                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickShadeImage() shines a distant light on an image to create a
%  three-dimensional effect. You control the positioning of the light with
%  azimuth and elevation; azimuth is measured in degrees off the x axis
%  and elevation is measured in pixels above the Z axis.
%
%  The format of the MagickShadeImage method is:
%
%      MagickBooleanType MagickShadeImage(MagickWand *wand,
%        const MagickBooleanType gray,const double azimuth,
%        const double elevation)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o gray: A value other than zero shades the intensity of each pixel.
%
%    o azimuth, elevation:  Define the light source direction.
%
*/
WandExport MagickBooleanType MagickShadeImage(MagickWand *wand,
  const MagickBooleanType gray,const double asimuth,const double elevation)
{
  Image
    *shade_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  shade_image=ShadeImage(wand->images,gray,asimuth,elevation,wand->exception);
  if (shade_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,shade_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S h a d o w I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickShadowImage() simulates an image shadow.
%
%  The format of the MagickShadowImage method is:
%
%      MagickBooleanType MagickShadowImage(MagickWand *wand,const double alpha,
%        const double sigma,const ssize_t x,const ssize_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o alpha: percentage transparency.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o x: the shadow x-offset.
%
%    o y: the shadow y-offset.
%
*/
WandExport MagickBooleanType MagickShadowImage(MagickWand *wand,
  const double alpha,const double sigma,const ssize_t x,const ssize_t y)
{
  Image
    *shadow_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  shadow_image=ShadowImage(wand->images,alpha,sigma,x,y,wand->exception);
  if (shadow_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,shadow_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S h a r p e n I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSharpenImage() sharpens an image.  We convolve the image with a
%  Gaussian operator of the given radius and standard deviation (sigma).
%  For reasonable results, the radius should be larger than sigma.  Use a
%  radius of 0 and MagickSharpenImage() selects a suitable radius for you.
%
%  The format of the MagickSharpenImage method is:
%
%      MagickBooleanType MagickSharpenImage(MagickWand *wand,
%        const double radius,const double sigma)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
*/
WandExport MagickBooleanType MagickSharpenImage(MagickWand *wand,
  const double radius,const double sigma)
{
  Image
    *sharp_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  sharp_image=SharpenImage(wand->images,radius,sigma,wand->exception);
  if (sharp_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,sharp_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S h a v e I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickShaveImage() shaves pixels from the image edges.  It allocates the
%  memory necessary for the new Image structure and returns a pointer to the
%  new image.
%
%  The format of the MagickShaveImage method is:
%
%      MagickBooleanType MagickShaveImage(MagickWand *wand,
%        const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
%
*/
WandExport MagickBooleanType MagickShaveImage(MagickWand *wand,
  const size_t columns,const size_t rows)
{
  Image
    *shave_image;

  RectangleInfo
    shave_info;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  shave_info.width=columns;
  shave_info.height=rows;
  shave_info.x=0;
  shave_info.y=0;
  shave_image=ShaveImage(wand->images,&shave_info,wand->exception);
  if (shave_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,shave_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S h e a r I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickShearImage() slides one edge of an image along the X or Y axis,
%  creating a parallelogram.  An X direction shear slides an edge along the X
%  axis, while a Y direction shear slides an edge along the Y axis.  The amount
%  of the shear is controlled by a shear angle.  For X direction shears, x_shear
%  is measured relative to the Y axis, and similarly, for Y direction shears
%  y_shear is measured relative to the X axis.  Empty triangles left over from
%  shearing the image are filled with the background color.
%
%  The format of the MagickShearImage method is:
%
%      MagickBooleanType MagickShearImage(MagickWand *wand,
%        const PixelWand *background,const double x_shear,const double y_shear)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o background: the background pixel wand.
%
%    o x_shear: the number of degrees to shear the image.
%
%    o y_shear: the number of degrees to shear the image.
%
*/
WandExport MagickBooleanType MagickShearImage(MagickWand *wand,
  const PixelWand *background,const double x_shear,const double y_shear)
{
  Image
    *shear_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  PixelGetQuantumPacket(background,&wand->images->background_color);
  shear_image=ShearImage(wand->images,x_shear,y_shear,wand->exception);
  if (shear_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,shear_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S i g m o i d a l C o n t r a s t I m a g e                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSigmoidalContrastImage() adjusts the contrast of an image with a
%  non-linear sigmoidal contrast algorithm.  Increase the contrast of the
%  image using a sigmoidal transfer function without saturating highlights or
%  shadows.  Contrast indicates how much to increase the contrast (0 is none;
%  3 is typical; 20 is pushing it); mid-point indicates where midtones fall in
%  the resultant image (0 is white; 50% is middle-gray; 100% is black).  Set
%  sharpen to MagickTrue to increase the image contrast otherwise the contrast
%  is reduced.
%
%  The format of the MagickSigmoidalContrastImage method is:
%
%      MagickBooleanType MagickSigmoidalContrastImage(MagickWand *wand,
%        const MagickBooleanType sharpen,const double alpha,const double beta)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o sharpen: Increase or decrease image contrast.
%
%    o alpha: strength of the contrast, the larger the number the more
%      'threshold-like' it becomes.
%
%    o beta: midpoint of the function as a color value 0 to QuantumRange.
%
*/
WandExport MagickBooleanType MagickSigmoidalContrastImage(
  MagickWand *wand,const MagickBooleanType sharpen,const double alpha,
  const double beta)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=SigmoidalContrastImage(wand->images,sharpen,alpha,beta,
    wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S i m i l a r i t y I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSimilarityImage() compares the reference image of the image and
%  returns the best match offset.  In addition, it returns a similarity image
%  such that an exact match location is completely white and if none of the
%  pixels match, black, otherwise some gray level in-between.
%
%  The format of the MagickSimilarityImage method is:
%
%      MagickWand *MagickSimilarityImage(MagickWand *wand,
%        const MagickWand *reference,const MetricType metric,
%        const double similarity_threshold,RectangeInfo *offset,
%        double *similarity)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o reference: the reference wand.
%
%    o metric: the metric.
%
%    o similarity_threshold: minimum distortion for (sub)image match.
%
%    o offset: the best match offset of the reference image within the image.
%
%    o similarity: the computed similarity between the images.
%
*/
WandExport MagickWand *MagickSimilarityImage(MagickWand *wand,
  const MagickWand *reference,const MetricType metric,
  const double similarity_threshold,RectangleInfo *offset,double *similarity)
{
  Image
    *similarity_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) || (reference->images == (Image *) NULL))
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((MagickWand *) NULL);
    }
  similarity_image=SimilarityImage(wand->images,reference->images,metric,
    similarity_threshold,offset,similarity,wand->exception);
  if (similarity_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,similarity_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S k e t c h I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSketchImage() simulates a pencil sketch.  We convolve the image with
%  a Gaussian operator of the given radius and standard deviation (sigma).
%  For reasonable results, radius should be larger than sigma.  Use a
%  radius of 0 and SketchImage() selects a suitable radius for you.
%  Angle gives the angle of the blurring motion.
%
%  The format of the MagickSketchImage method is:
%
%      MagickBooleanType MagickSketchImage(MagickWand *wand,
%        const double radius,const double sigma,const double angle)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the Gaussian, in pixels, not counting
%      the center pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o angle: apply the effect along this angle.
%
*/
WandExport MagickBooleanType MagickSketchImage(MagickWand *wand,
  const double radius,const double sigma,const double angle)
{
  Image
    *sketch_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  sketch_image=SketchImage(wand->images,radius,sigma,angle,wand->exception);
  if (sketch_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,sketch_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S m u s h I m a g e s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSmushImages() takes all images from the current image pointer to the
%  end of the image list and smushs them to each other top-to-bottom if the
%  stack parameter is true, otherwise left-to-right.
%
%  The format of the MagickSmushImages method is:
%
%      MagickWand *MagickSmushImages(MagickWand *wand,
%        const MagickBooleanType stack,const ssize_t offset)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o stack: By default, images are stacked left-to-right. Set stack to
%      MagickTrue to stack them top-to-bottom.
%
%    o offset: minimum distance in pixels between images.
%
*/
WandExport MagickWand *MagickSmushImages(MagickWand *wand,
  const MagickBooleanType stack,const ssize_t offset)
{
  Image
    *smush_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    return((MagickWand *) NULL);
  smush_image=SmushImages(wand->images,stack,offset,wand->exception);
  if (smush_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,smush_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a g i c k S o l a r i z e I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSolarizeImage() applies a special effect to the image, similar to the
%  effect achieved in a photo darkroom by selectively exposing areas of photo
%  sensitive paper to light.  Threshold ranges from 0 to QuantumRange and is a
%  measure of the extent of the solarization.
%
%  The format of the MagickSolarizeImage method is:
%
%      MagickBooleanType MagickSolarizeImage(MagickWand *wand,
%        const double threshold)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o threshold:  Define the extent of the solarization.
%
*/
WandExport MagickBooleanType MagickSolarizeImage(MagickWand *wand,
  const double threshold)
{
  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  status=SolarizeImage(wand->images,threshold,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S p a r s e C o l o r I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSparseColorImage(), given a set of coordinates, interpolates the
%  colors found at those coordinates, across the whole image, using various
%  methods.
%
%  The format of the MagickSparseColorImage method is:
%
%      MagickBooleanType MagickSparseColorImage(MagickWand *wand,
%        const SparseColorMethod method,const size_t number_arguments,
%        const double *arguments)
%
%  A description of each parameter follows:
%
%    o image: the image to be sparseed.
%
%    o method: the method of image sparseion.
%
%        ArcSparseColorion will always ignore source image offset, and always
%        'bestfit' the destination image with the top left corner offset
%        relative to the polar mapping center.
%
%        Bilinear has no simple inverse mapping so will not allow 'bestfit'
%        style of image sparseion.
%
%        Affine, Perspective, and Bilinear, will do least squares fitting of
%        the distrotion when more than the minimum number of control point
%        pairs are provided.
%
%        Perspective, and Bilinear, will fall back to a Affine sparseion when
%        less than 4 control point pairs are provided. While Affine sparseions
%        will let you use any number of control point pairs, that is Zero pairs
%        is a No-Op (viewport only) distrotion, one pair is a translation and
%        two pairs of control points will do a scale-rotate-translate, without
%        any shearing.
%
%    o number_arguments: the number of arguments given for this sparseion
%      method.
%
%    o arguments: the arguments for this sparseion method.
%
*/
WandExport MagickBooleanType MagickSparseColorImage(MagickWand *wand,
  const SparseColorMethod method,const size_t number_arguments,
  const double *arguments)
{
  Image
    *sparse_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  sparse_image=SparseColorImage(wand->images,method,number_arguments,arguments,
    wand->exception);
  if (sparse_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,sparse_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S p l i c e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSpliceImage() splices a solid color into the image.
%
%  The format of the MagickSpliceImage method is:
%
%      MagickBooleanType MagickSpliceImage(MagickWand *wand,
%        const size_t width,const size_t height,const ssize_t x,
%        const ssize_t y)
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
WandExport MagickBooleanType MagickSpliceImage(MagickWand *wand,
  const size_t width,const size_t height,const ssize_t x,
  const ssize_t y)
{
  Image
    *splice_image;

  RectangleInfo
    splice;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  splice.width=width;
  splice.height=height;
  splice.x=x;
  splice.y=y;
  splice_image=SpliceImage(wand->images,&splice,wand->exception);
  if (splice_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,splice_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S p r e a d I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSpreadImage() is a special effects method that randomly displaces each
%  pixel in a block defined by the radius parameter.
%
%  The format of the MagickSpreadImage method is:
%
%      MagickBooleanType MagickSpreadImage(MagickWand *wand,
%        const PixelInterpolateMethod method,const double radius)
%        
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o method:  intepolation method.
%
%    o radius:  Choose a random pixel in a neighborhood of this extent.
%
*/
WandExport MagickBooleanType MagickSpreadImage(MagickWand *wand,
  const PixelInterpolateMethod method,const double radius)
{
  Image
    *spread_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  spread_image=SpreadImage(wand->images,method,radius,wand->exception);
  if (spread_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,spread_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S t a t i s t i c I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickStatisticImage() replace each pixel with corresponding statistic from
%  the neighborhood of the specified width and height.
%
%  The format of the MagickStatisticImage method is:
%
%      MagickBooleanType MagickStatisticImage(MagickWand *wand,
%        const StatisticType type,const double width,const size_t height)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o type: the statistic type (e.g. median, mode, etc.).
%
%    o width: the width of the pixel neighborhood.
%
%    o height: the height of the pixel neighborhood.
%
*/
WandExport MagickBooleanType MagickStatisticImage(MagickWand *wand,
  const StatisticType type,const size_t width,const size_t height)
{
  Image
    *statistic_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  statistic_image=StatisticImage(wand->images,type,width,height,
    wand->exception);
  if (statistic_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,statistic_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S t e g a n o I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSteganoImage() hides a digital watermark within the image.
%  Recover the hidden watermark later to prove that the authenticity of
%  an image.  Offset defines the start position within the image to hide
%  the watermark.
%
%  The format of the MagickSteganoImage method is:
%
%      MagickWand *MagickSteganoImage(MagickWand *wand,
%        const MagickWand *watermark_wand,const ssize_t offset)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o watermark_wand: the watermark wand.
%
%    o offset: Start hiding at this offset into the image.
%
*/
WandExport MagickWand *MagickSteganoImage(MagickWand *wand,
  const MagickWand *watermark_wand,const ssize_t offset)
{
  Image
    *stegano_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) ||
      (watermark_wand->images == (Image *) NULL))
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((MagickWand *) NULL);
    }
  wand->images->offset=offset;
  stegano_image=SteganoImage(wand->images,watermark_wand->images,
    wand->exception);
  if (stegano_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,stegano_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S t e r e o I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickStereoImage() composites two images and produces a single image that
%  is the composite of a left and right image of a stereo pair
%
%  The format of the MagickStereoImage method is:
%
%      MagickWand *MagickStereoImage(MagickWand *wand,
%        const MagickWand *offset_wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o offset_wand: Another image wand.
%
*/
WandExport MagickWand *MagickStereoImage(MagickWand *wand,
  const MagickWand *offset_wand)
{
  Image
    *stereo_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) ||
      (offset_wand->images == (Image *) NULL))
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((MagickWand *) NULL);
    }
  stereo_image=StereoImage(wand->images,offset_wand->images,wand->exception);
  if (stereo_image == (Image *) NULL)
    return((MagickWand *) NULL);
  return(CloneMagickWandFromImages(wand,stereo_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S t r i p I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickStripImage() strips an image of all profiles and comments.
%
%  The format of the MagickStripImage method is:
%
%      MagickBooleanType MagickStripImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickStripImage(MagickWand *wand)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(StripImage(wand->images,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S w i r l I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSwirlImage() swirls the pixels about the center of the image, where
%  degrees indicates the sweep of the arc through which each pixel is moved.
%  You get a more dramatic effect as the degrees move from 1 to 360.
%
%  The format of the MagickSwirlImage method is:
%
%      MagickBooleanType MagickSwirlImage(MagickWand *wand,const double degrees,
%        const PixelInterpolateMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o degrees: Define the tightness of the swirling effect.
%
%    o method: the pixel interpolation method.
%
*/
WandExport MagickBooleanType MagickSwirlImage(MagickWand *wand,
  const double degrees,const PixelInterpolateMethod method)
{
  Image
    *swirl_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  swirl_image=SwirlImage(wand->images,degrees,method,wand->exception);
  if (swirl_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,swirl_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T e x t u r e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickTextureImage() repeatedly tiles the texture image across and down the
%  image canvas.
%
%  The format of the MagickTextureImage method is:
%
%      MagickWand *MagickTextureImage(MagickWand *wand,
%        const MagickWand *texture_wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o texture_wand: the texture wand
%
*/
WandExport MagickWand *MagickTextureImage(MagickWand *wand,
  const MagickWand *texture_wand)
{
  Image
    *texture_image;

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if ((wand->images == (Image *) NULL) ||
      (texture_wand->images == (Image *) NULL))
    {
      (void) ThrowMagickException(wand->exception,GetMagickModule(),WandError,
        "ContainsNoImages","`%s'",wand->name);
      return((MagickWand *) NULL);
    }
  texture_image=CloneImage(wand->images,0,0,MagickTrue,wand->exception);
  if (texture_image == (Image *) NULL)
    return((MagickWand *) NULL);
  status=TextureImage(texture_image,texture_wand->images,wand->exception);
  if (status == MagickFalse)
    {
      texture_image=DestroyImage(texture_image);
      return((MagickWand *) NULL);
    }
  return(CloneMagickWandFromImages(wand,texture_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T h r e s h o l d I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickThresholdImage() changes the value of individual pixels based on
%  the intensity of each pixel compared to threshold.  The result is a
%  high-contrast, two color image.
%
%  The format of the MagickThresholdImage method is:
%
%      MagickBooleanType MagickThresholdImage(MagickWand *wand,
%        const double threshold)
%      MagickBooleanType MagickThresholdImageChannel(MagickWand *wand,
%        const ChannelType channel,const double threshold)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o channel: the image channel(s).
%
%    o threshold: Define the threshold value.
%
*/
WandExport MagickBooleanType MagickThresholdImage(MagickWand *wand,
  const double threshold)
{
  MagickBooleanType
    status;

  status=MagickThresholdImageChannel(wand,DefaultChannels,threshold);
  return(status);
}

WandExport MagickBooleanType MagickThresholdImageChannel(MagickWand *wand,
  const ChannelType channel,const double threshold)
{
  MagickBooleanType
    status;

  ChannelType
    channel_mask;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  channel_mask=SetImageChannelMask(wand->images,channel);
  status=BilevelImage(wand->images,threshold,wand->exception);
  (void) SetImageChannelMask(wand->images,channel_mask);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T h u m b n a i l I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickThumbnailImage()  changes the size of an image to the given dimensions
%  and removes any associated profiles.  The goal is to produce small low cost
%  thumbnail images suited for display on the Web.
%
%  The format of the MagickThumbnailImage method is:
%
%      MagickBooleanType MagickThumbnailImage(MagickWand *wand,
%        const size_t columns,const size_t rows)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
*/
WandExport MagickBooleanType MagickThumbnailImage(MagickWand *wand,
  const size_t columns,const size_t rows)
{
  Image
    *thumbnail_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  thumbnail_image=ThumbnailImage(wand->images,columns,rows,wand->exception);
  if (thumbnail_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,thumbnail_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T i n t I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickTintImage() applies a color vector to each pixel in the image.  The
%  length of the vector is 0 for black and white and at its maximum for the
%  midtones.  The vector weighting function is
%  f(x)=(1-(4.0*((x-0.5)*(x-0.5)))).
%
%  The format of the MagickTintImage method is:
%
%      MagickBooleanType MagickTintImage(MagickWand *wand,
%        const PixelWand *tint,const PixelWand *blend)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o tint: the tint pixel wand.
%
%    o alpha: the alpha pixel wand.
%
*/
WandExport MagickBooleanType MagickTintImage(MagickWand *wand,
  const PixelWand *tint,const PixelWand *blend)
{
  char
    percent_blend[MagickPathExtent];

  Image
    *tint_image;

  PixelInfo
    target;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if (wand->images->colorspace != CMYKColorspace)
    (void) FormatLocaleString(percent_blend,MagickPathExtent,
      "%g,%g,%g,%g",(double) (100.0*QuantumScale*
      PixelGetRedQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetGreenQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetBlueQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetAlphaQuantum(blend)));
  else
    (void) FormatLocaleString(percent_blend,MagickPathExtent,
      "%g,%g,%g,%g,%g",(double) (100.0*QuantumScale*
      PixelGetCyanQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetMagentaQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetYellowQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetBlackQuantum(blend)),(double) (100.0*QuantumScale*
      PixelGetAlphaQuantum(blend)));
  target=PixelGetPixel(tint);
  tint_image=TintImage(wand->images,percent_blend,&target,wand->exception);
  if (tint_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,tint_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T r a n s f o r m I m a g e C o l o r s p a c e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickTransformImageColorspace() transform the image colorspace, setting
%  the images colorspace while transforming the images data to that
%  colorspace.
%
%  The format of the MagickTransformImageColorspace method is:
%
%      MagickBooleanType MagickTransformImageColorspace(MagickWand *wand,
%        const ColorspaceType colorspace)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o colorspace: the image colorspace:   UndefinedColorspace,
%      sRGBColorspace, RGBColorspace, GRAYColorspace,
%      OHTAColorspace, XYZColorspace, YCbCrColorspace,
%      YCCColorspace, YIQColorspace, YPbPrColorspace,
%      YPbPrColorspace, YUVColorspace, CMYKColorspace,
%      HSLColorspace, HWBColorspace.
%
*/
WandExport MagickBooleanType MagickTransformImageColorspace(MagickWand *wand,
  const ColorspaceType colorspace)
{
  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  return(TransformImageColorspace(wand->images,colorspace,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T r a n s p a r e n t P a i n t I m a g e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickTransparentPaintImage() changes any pixel that matches color with the
%  color defined by fill.
%
%  The format of the MagickTransparentPaintImage method is:
%
%      MagickBooleanType MagickTransparentPaintImage(MagickWand *wand,
%        const PixelWand *target,const double alpha,const double fuzz,
%        const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o target: Change this target color to specified alpha value within
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
%    o invert: paint any pixel that does not match the target color.
%
*/
WandExport MagickBooleanType MagickTransparentPaintImage(MagickWand *wand,
  const PixelWand *target,const double alpha,const double fuzz,
  const MagickBooleanType invert)
{
  MagickBooleanType
    status;

  PixelInfo
    target_pixel;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  PixelGetMagickColor(target,&target_pixel);
  wand->images->fuzz=fuzz;
  status=TransparentPaintImage(wand->images,&target_pixel,ClampToQuantum(
    QuantumRange*alpha),invert,wand->exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T r a n s p o s e I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickTransposeImage() creates a vertical mirror image by reflecting the
%  pixels around the central x-axis while rotating them 90-degrees.
%
%  The format of the MagickTransposeImage method is:
%
%      MagickBooleanType MagickTransposeImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickTransposeImage(MagickWand *wand)
{
  Image
    *transpose_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  transpose_image=TransposeImage(wand->images,wand->exception);
  if (transpose_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,transpose_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T r a n s v e r s e I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickTransverseImage() creates a horizontal mirror image by reflecting the
%  pixels around the central y-axis while rotating them 270-degrees.
%
%  The format of the MagickTransverseImage method is:
%
%      MagickBooleanType MagickTransverseImage(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickTransverseImage(MagickWand *wand)
{
  Image
    *transverse_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  transverse_image=TransverseImage(wand->images,wand->exception);
  if (transverse_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,transverse_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k T r i m I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickTrimImage() remove edges that are the background color from the image.
%
%  The format of the MagickTrimImage method is:
%
%      MagickBooleanType MagickTrimImage(MagickWand *wand,const double fuzz)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o fuzz: By default target must match a particular pixel color
%      exactly.  However, in many cases two colors may differ by a small amount.
%      The fuzz member of image defines how much tolerance is acceptable to
%      consider two colors as the same.  For example, set fuzz to 10 and the
%      color red at intensities of 100 and 102 respectively are now interpreted
%      as the same color for the purposes of the floodfill.
%
*/
WandExport MagickBooleanType MagickTrimImage(MagickWand *wand,const double fuzz)
{
  Image
    *trim_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wand->images->fuzz=fuzz;
  trim_image=TrimImage(wand->images,wand->exception);
  if (trim_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,trim_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k U n i q u e I m a g e C o l o r s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickUniqueImageColors() discards all but one of any pixel color.
%
%  The format of the MagickUniqueImageColors method is:
%
%      MagickBooleanType MagickUniqueImageColors(MagickWand *wand)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
*/
WandExport MagickBooleanType MagickUniqueImageColors(MagickWand *wand)
{
  Image
    *unique_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  unique_image=UniqueImageColors(wand->images,wand->exception);
  if (unique_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,unique_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k U n s h a r p M a s k I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickUnsharpMaskImage() sharpens an image.  We convolve the image with a
%  Gaussian operator of the given radius and standard deviation (sigma).
%  For reasonable results, radius should be larger than sigma.  Use a radius
%  of 0 and UnsharpMaskImage() selects a suitable radius for you.
%
%  The format of the MagickUnsharpMaskImage method is:
%
%      MagickBooleanType MagickUnsharpMaskImage(MagickWand *wand,
%        const double radius,const double sigma,const double gain,
%        const double threshold)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o gain: the percentage of the difference between the original and the
%      blur image that is added back into the original.
%
%    o threshold: the threshold in pixels needed to apply the diffence gain.
%
*/
WandExport MagickBooleanType MagickUnsharpMaskImage(MagickWand *wand,
  const double radius,const double sigma,const double gain,
  const double threshold)
{
  Image
    *unsharp_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  unsharp_image=UnsharpMaskImage(wand->images,radius,sigma,gain,threshold,
    wand->exception);
  if (unsharp_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,unsharp_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k V i g n e t t e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickVignetteImage() softens the edges of the image in vignette style.
%
%  The format of the MagickVignetteImage method is:
%
%      MagickBooleanType MagickVignetteImage(MagickWand *wand,
%        const double radius,const double sigma,const ssize_t x,
%        const ssize_t y)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o radius: the radius.
%
%    o sigma: the sigma.
%
%    o x, y:  Define the x and y ellipse offset.
%
*/
WandExport MagickBooleanType MagickVignetteImage(MagickWand *wand,
  const double radius,const double sigma,const ssize_t x,const ssize_t y)
{
  Image
    *vignette_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  vignette_image=VignetteImage(wand->images,radius,sigma,x,y,wand->exception);
  if (vignette_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,vignette_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W a v e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWaveImage()  creates a "ripple" effect in the image by shifting
%  the pixels vertically along a sine wave whose amplitude and wavelength
%  is specified by the given parameters.
%
%  The format of the MagickWaveImage method is:
%
%      MagickBooleanType MagickWaveImage(MagickWand *wand,
%        const double amplitude,const double wave_length,
%        const PixelInterpolateMethod method)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o amplitude, wave_length:  Define the amplitude and wave length of the
%      sine wave.
%
%    o method: the pixel interpolation method.
%
*/
WandExport MagickBooleanType MagickWaveImage(MagickWand *wand,
  const double amplitude,const double wave_length,
  const PixelInterpolateMethod method)
{
  Image
    *wave_image;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  wave_image=WaveImage(wand->images,amplitude,wave_length,method,
    wand->exception);
  if (wave_image == (Image *) NULL)
    return(MagickFalse);
  ReplaceImageInList(&wand->images,wave_image);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W h i t e T h r e s h o l d I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWhiteThresholdImage() is like ThresholdImage() but  force all pixels
%  above the threshold into white while leaving all pixels below the threshold
%  unchanged.
%
%  The format of the MagickWhiteThresholdImage method is:
%
%      MagickBooleanType MagickWhiteThresholdImage(MagickWand *wand,
%        const PixelWand *threshold)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o threshold: the pixel wand.
%
*/
WandExport MagickBooleanType MagickWhiteThresholdImage(MagickWand *wand,
  const PixelWand *threshold)
{
  char
    thresholds[MagickPathExtent];

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  (void) FormatLocaleString(thresholds,MagickPathExtent,
    QuantumFormat "," QuantumFormat "," QuantumFormat "," QuantumFormat,
    PixelGetRedQuantum(threshold),PixelGetGreenQuantum(threshold),
    PixelGetBlueQuantum(threshold),PixelGetAlphaQuantum(threshold));
  return(WhiteThresholdImage(wand->images,thresholds,wand->exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W r i t e I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWriteImage() writes an image to the specified filename.  If the
%  filename parameter is NULL, the image is written to the filename set
%  by MagickReadImage() or MagickSetImageFilename().
%
%  The format of the MagickWriteImage method is:
%
%      MagickBooleanType MagickWriteImage(MagickWand *wand,
%        const char *filename)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o filename: the image filename.
%
%
*/
WandExport MagickBooleanType MagickWriteImage(MagickWand *wand,
  const char *filename)
{
  Image
    *image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  if (filename != (const char *) NULL)
    (void) CopyMagickString(wand->images->filename,filename,MagickPathExtent);
  image=CloneImage(wand->images,0,0,MagickTrue,wand->exception);
  if (image == (Image *) NULL)
    return(MagickFalse);
  write_info=CloneImageInfo(wand->image_info);
  write_info->adjoin=MagickTrue;
  status=WriteImage(write_info,image,wand->exception);
  image=DestroyImage(image);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W r i t e I m a g e F i l e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWriteImageFile() writes an image to an open file descriptor.
%
%  The format of the MagickWriteImageFile method is:
%
%      MagickBooleanType MagickWriteImageFile(MagickWand *wand,FILE *file)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o file: the file descriptor.
%
*/
WandExport MagickBooleanType MagickWriteImageFile(MagickWand *wand,FILE *file)
{
  Image
    *image;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  assert(file != (FILE *) NULL);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  image=CloneImage(wand->images,0,0,MagickTrue,wand->exception);
  if (image == (Image *) NULL)
    return(MagickFalse);
  write_info=CloneImageInfo(wand->image_info);
  SetImageInfoFile(write_info,file);
  write_info->adjoin=MagickTrue;
  status=WriteImage(write_info,image,wand->exception);
  write_info=DestroyImageInfo(write_info);
  image=DestroyImage(image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W r i t e I m a g e s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWriteImages() writes an image or image sequence.
%
%  The format of the MagickWriteImages method is:
%
%      MagickBooleanType MagickWriteImages(MagickWand *wand,
%        const char *filename,const MagickBooleanType adjoin)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o filename: the image filename.
%
%    o adjoin: join images into a single multi-image file.
%
*/
WandExport MagickBooleanType MagickWriteImages(MagickWand *wand,
  const char *filename,const MagickBooleanType adjoin)
{
  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  write_info=CloneImageInfo(wand->image_info);
  write_info->adjoin=adjoin;
  status=WriteImages(write_info,wand->images,filename,wand->exception);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k W r i t e I m a g e s F i l e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickWriteImagesFile() writes an image sequence to an open file descriptor.
%
%  The format of the MagickWriteImagesFile method is:
%
%      MagickBooleanType MagickWriteImagesFile(MagickWand *wand,FILE *file)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o file: the file descriptor.
%
*/
WandExport MagickBooleanType MagickWriteImagesFile(MagickWand *wand,FILE *file)
{
  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *) NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  write_info=CloneImageInfo(wand->image_info);
  SetImageInfoFile(write_info,file);
  write_info->adjoin=MagickTrue;
  status=WriteImages(write_info,wand->images,(const char *) NULL,
    wand->exception);
  write_info=DestroyImageInfo(write_info);
  return(status);
}

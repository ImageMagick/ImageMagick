/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                     L       AAA   Y   Y  EEEEE  RRRR                        %
%                     L      A   A   Y Y   E      R   R                       %
%                     L      AAAAA    Y    EEE    RRRR                        %
%                     L      A   A    Y    E      R R                         %
%                     LLLLL  A   A    Y    EEEEE  R  R                        %
%                                                                             %
%                      MagickCore Image Layering Methods                      %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                              Anthony Thyssen                                %
%                               January 2006                                  %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/composite.h"
#include "magick/effect.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/layer.h"
#include "magick/list.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/profile.h"
#include "magick/resource_.h"
#include "magick/resize.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/transform.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     C l e a r B o u n d s                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClearBounds() Clear the area specified by the bounds in an image to
%  transparency.  This typically used to handle Background Disposal
%  for the previous frame in an animation sequence.
%
%  WARNING: no bounds checks are performed, except for the null or
%  missed image, for images that don't change. in all other cases
%  bound must fall within the image.
%
%  The format is:
%
%      void ClearBounds(Image *image,RectangleInfo *bounds)
%
%  A description of each parameter follows:
%
%    o image: the image to had the area cleared in
%
%    o bounds: the area to be clear within the imag image
%
*/
static void ClearBounds(Image *image,RectangleInfo *bounds)
{
  ExceptionInfo
    *exception;

  long
    y;

  if (bounds->x < 0)
    return;
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel);
  exception=(&image->exception);
  for (y=0; y < (long) bounds->height; y++)
  {
    register long
      x;

    register PixelPacket
      *restrict q;

    q=GetAuthenticPixels(image,bounds->x,bounds->y+y,bounds->width,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (long) bounds->width; x++)
    {
      q->opacity=(Quantum) TransparentOpacity;
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     I s B o u n d s C l e a r e d                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsBoundsCleared() tests whether any pixel in the bounds given, gets cleared
%  when going from the first image to the second image.  This typically used
%  to check if a proposed disposal method will work successfully to generate
%  the second frame image from the first disposed form of the previous frame.
%
%  The format is:
%
%      MagickBooleanType IsBoundsCleared(const Image *image1,
%        const Image *image2,RectangleInfo bounds,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image1, image 2: the images to check for cleared pixels
%
%    o bounds: the area to be clear within the imag image
%
%    o exception: return any errors or warnings in this structure.
%
%  WARNING: no bounds checks are performed, except for the null or
%  missed image, for images that don't change. in all other cases
%  bound must fall within the image.
%
*/
static MagickBooleanType IsBoundsCleared(const Image *image1,
  const Image *image2,RectangleInfo *bounds,ExceptionInfo *exception)
{
  long
    y;

  register long
    x;

  register const PixelPacket
    *p,
    *q;

  if ( bounds->x< 0 ) return(MagickFalse);

  for (y=0; y < (long) bounds->height; y++)
  {
    p=GetVirtualPixels(image1,bounds->x,bounds->y+y,bounds->width,1,
      exception);
    q=GetVirtualPixels(image2,bounds->x,bounds->y+y,bounds->width,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < (long) bounds->width; x++)
    {
      if ((p->opacity <= (long) (QuantumRange/2)) &&
          (q->opacity > (long) (QuantumRange/2)))
        break;
      p++;
      q++;
    }
    if (x < (long) bounds->width)
      break;
  }
  return(y < (long) bounds->height ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o a l e s c e I m a g e s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CoalesceImages() composites a set of images while respecting any page
%  offsets and disposal methods.  GIF, MIFF, and MNG animation sequences
%  typically start with an image background and each subsequent image
%  varies in size and offset.  A new image sequence is returned with all
%  images the same size as the first images virtual canvas and composited
%  with the next image in the sequence.
%
%  The format of the CoalesceImages method is:
%
%      Image *CoalesceImages(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image sequence.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *CoalesceImages(const Image *image,ExceptionInfo *exception)
{
  Image
    *coalesce_image,
    *dispose_image,
    *previous;

  register Image
    *next;

  RectangleInfo
    bounds;

  /*
    Coalesce the image sequence.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  /* initialise first image */
  next=GetFirstImageInList(image);
  bounds=next->page;
  if (bounds.width == 0)
    {
      bounds.width=next->columns;
      if (bounds.x > 0)
        bounds.width+=bounds.x;
    }
  if (bounds.height == 0)
    {
      bounds.height=next->rows;
      if (bounds.y > 0)
        bounds.height+=bounds.y;
    }
  bounds.x=0;
  bounds.y=0;
  coalesce_image=CloneImage(next,bounds.width,bounds.height,MagickTrue,
    exception);
  if (coalesce_image == (Image *) NULL)
    return((Image *) NULL);
  coalesce_image->page=bounds;
  coalesce_image->dispose=NoneDispose;
  coalesce_image->background_color.opacity=(Quantum) TransparentOpacity;
  (void) SetImageBackgroundColor(coalesce_image);
  /*
    Coalesce rest of the images.
  */
  dispose_image=CloneImage(coalesce_image,0,0,MagickTrue,exception);
  (void) CompositeImage(coalesce_image,CopyCompositeOp,next,next->page.x,
    next->page.y);
  next=GetNextImageInList(next);
  for ( ; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    /*
      Determine the bounds that was overlaid in the previous image.
    */
    previous=GetPreviousImageInList(next);
    bounds=previous->page;
    bounds.width=previous->columns;
    bounds.height=previous->rows;
    if (bounds.x < 0)
      {
        bounds.width+=bounds.x;
        bounds.x=0;
      }
    if ((long) (bounds.x+bounds.width) > (long) coalesce_image->columns)
      bounds.width=coalesce_image->columns-bounds.x;
    if (bounds.y < 0)
      {
        bounds.height+=bounds.y;
        bounds.y=0;
      }
    if ((long) (bounds.y+bounds.height) > (long) coalesce_image->rows)
      bounds.height=coalesce_image->rows-bounds.y;
    /*
      Replace the dispose image with the new coalesced image.
    */
    if (GetPreviousImageInList(next)->dispose != PreviousDispose)
      {
        dispose_image=DestroyImage(dispose_image);
        dispose_image=CloneImage(coalesce_image,0,0,MagickTrue,exception);
        if (dispose_image == (Image *) NULL)
          {
            coalesce_image=DestroyImageList(coalesce_image);
            return((Image *) NULL);
          }
      }
    /*
      Clear the overlaid area of the coalesced bounds for background disposal
    */
    if (next->previous->dispose == BackgroundDispose)
      ClearBounds(dispose_image, &bounds);
    /*
      Next image is the dispose image, overlaid with next frame in sequence.
    */
    coalesce_image->next=CloneImage(dispose_image,0,0,MagickTrue,exception);
    coalesce_image->next->previous=coalesce_image;
    previous=coalesce_image;
    coalesce_image=GetNextImageInList(coalesce_image);
    coalesce_image->matte=MagickTrue;
    (void) CompositeImage(coalesce_image,next->matte != MagickFalse ?
      OverCompositeOp : CopyCompositeOp,next,next->page.x,next->page.y);
    (void) CloneImageProfiles(coalesce_image,next);
    (void) CloneImageProperties(coalesce_image,next);
    (void) CloneImageArtifacts(coalesce_image,next);
    coalesce_image->page=previous->page;
    /*
      If a pixel goes opaque to transparent, use background dispose.
    */
    if (IsBoundsCleared(previous,coalesce_image,&bounds,exception))
      coalesce_image->dispose=BackgroundDispose;
    else
      coalesce_image->dispose=NoneDispose;
    previous->dispose=coalesce_image->dispose;
  }
  dispose_image=DestroyImage(dispose_image);
  return(GetFirstImageInList(coalesce_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     D i s p o s e I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DisposeImages() returns the coalesced frames of a GIF animation as it would
%  appear after the GIF dispose method of that frame has been applied.  That
%  is it returned the appearance of each frame before the next is overlaid.
%
%  The format of the DisposeImages method is:
%
%      Image *DisposeImages(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image sequence.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *DisposeImages(const Image *image,ExceptionInfo *exception)
{
  Image
    *dispose_image,
    *dispose_images;

  register Image
    *next;

  /*
    Run the image through the animation sequence
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  next=GetFirstImageInList(image);
  dispose_image=CloneImage(next,next->page.width,next->page.height,MagickTrue,
    exception);
  if (dispose_image == (Image *) NULL)
    return((Image *) NULL);
  dispose_image->page=next->page;
  dispose_image->page.x=0;
  dispose_image->page.y=0;
  dispose_image->dispose=NoneDispose;
  dispose_image->background_color.opacity=(Quantum) TransparentOpacity;
  (void) SetImageBackgroundColor(dispose_image);
  dispose_images=NewImageList();
  for ( ; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    Image
      *current_image;

    /*
      Overlay this frame's image over the previous disposal image.
    */
    current_image=CloneImage(dispose_image,0,0,MagickTrue,exception);
    if (current_image == (Image *) NULL)
      {
        dispose_images=DestroyImageList(dispose_images);
        dispose_image=DestroyImage(dispose_image);
        return((Image *) NULL);
      }
    (void) CompositeImage(current_image,next->matte != MagickFalse ?
      OverCompositeOp : CopyCompositeOp,next,next->page.x,next->page.y);
    /*
      Handle Background dispose: image is displayed for the delay period.
    */
    if (next->dispose == BackgroundDispose)
      {
        RectangleInfo
          bounds;

        bounds=next->page;
        bounds.width=next->columns;
        bounds.height=next->rows;
        if (bounds.x < 0)
          {
            bounds.width+=bounds.x;
            bounds.x=0;
          }
        if ((long) (bounds.x+bounds.width) > (long) current_image->columns)
          bounds.width=current_image->columns-bounds.x;
        if (bounds.y < 0)
          {
            bounds.height+=bounds.y;
            bounds.y=0;
          }
        if ((long) (bounds.y+bounds.height) > (long) current_image->rows)
          bounds.height=current_image->rows-bounds.y;
        ClearBounds(current_image,&bounds);
      }
    /*
      Select the appropriate previous/disposed image.
    */
    if (next->dispose == PreviousDispose)
      current_image=DestroyImage(current_image);
    else
      {
        dispose_image=DestroyImage(dispose_image);
        dispose_image=current_image;
      }
    {
      Image
        *dispose;

      /*
        Save the dispose image just calculated for return.
      */
      dispose=CloneImage(dispose_image,0,0,MagickTrue,exception);
      if (dispose == (Image *) NULL)
        {
          dispose_images=DestroyImageList(dispose_images);
          dispose_image=DestroyImage(dispose_image);
          return((Image *) NULL);
        }
      (void) CloneImageProfiles(dispose,next);
      (void) CloneImageProperties(dispose,next);
      (void) CloneImageArtifacts(dispose,next);
      dispose->page.x=0;
      dispose->page.y=0;
      dispose->dispose=next->dispose;
      AppendImageToList(&dispose_images,dispose);
    }
  }
  dispose_image=DestroyImage(dispose_image);
  return(GetFirstImageInList(dispose_images));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     C o m p a r e P i x e l s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ComparePixels() Compare the two pixels and return true if the pixels
%  differ according to the given LayerType comparision method.
%
%  This currently only used internally by CompareImageBounds(). It is
%  doubtful that this sub-routine will be useful outside this module.
%
%  The format of the ComparePixels method is:
%
%      MagickBooleanType *ComparePixels(const ImageLayerMethod method,
%        const MagickPixelPacket *p,const MagickPixelPacket *q)
%
%  A description of each parameter follows:
%
%    o method: What differences to look for. Must be one of
%              CompareAnyLayer, CompareClearLayer, CompareOverlayLayer.
%
%    o p, q: the pixels to test for appropriate differences.
%
*/

static MagickBooleanType ComparePixels(const ImageLayerMethod method,
  const MagickPixelPacket *p,const MagickPixelPacket *q)
{
  MagickRealType
    o1,
    o2;

  /*
    Any change in pixel values
  */
  if (method == CompareAnyLayer)
    return(IsMagickColorSimilar(p,q) == MagickFalse ? MagickTrue : MagickFalse);

  o1 = (p->matte != MagickFalse) ? p->opacity : OpaqueOpacity;
  o2 = (q->matte != MagickFalse) ? q->opacity : OpaqueOpacity;

  /*
    Pixel goes from opaque to transprency
  */
  if (method == CompareClearLayer)
    return((MagickBooleanType) ( (o1 <= ((MagickRealType) QuantumRange/2.0)) &&
      (o2 > ((MagickRealType) QuantumRange/2.0)) ) );

  /*
    overlay would change first pixel by second
  */
  if (method == CompareOverlayLayer)
    {
      if (o2 > ((MagickRealType) QuantumRange/2.0))
        return MagickFalse;
      return((MagickBooleanType) (IsMagickColorSimilar(p,q) == MagickFalse));
    }
  return(MagickFalse);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     C o m p a r e I m a g e B o u n d s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompareImageBounds() Given two images return the smallest rectangular area
%  by which the two images differ, accourding to the given 'Compare...'
%  layer method.
%
%  This currently only used internally in this module, but may eventually
%  be used by other modules.
%
%  The format of the CompareImageBounds method is:
%
%      RectangleInfo *CompareImageBounds(const ImageLayerMethod method,
%        const Image *image1, const Image *image2, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o method: What differences to look for. Must be one of
%              CompareAnyLayer, CompareClearLayer, CompareOverlayLayer.
%
%    o image1, image2: the two images to compare.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static RectangleInfo CompareImageBounds(const Image *image1,const Image *image2,
  const ImageLayerMethod method,ExceptionInfo *exception)
{
  RectangleInfo
    bounds;

  MagickPixelPacket
    pixel1,
    pixel2;

  register const IndexPacket
    *indexes1,
    *indexes2;

  register const PixelPacket
    *p,
    *q;

  long
    y;

  register long
    x;

  /*
    Set bounding box of the differences between images
  */
  GetMagickPixelPacket(image1,&pixel1);
  GetMagickPixelPacket(image2,&pixel2);
  for (x=0; x < (long) image1->columns; x++)
  {
    p=GetVirtualPixels(image1,x,0,1,image1->rows,exception);
    q=GetVirtualPixels(image2,x,0,1,image2->rows,exception);
    if ((p == (const PixelPacket *) NULL) ||
        (q == (const PixelPacket *) NULL))
      break;
    indexes1=GetVirtualIndexQueue(image1);
    indexes2=GetVirtualIndexQueue(image2);
    for (y=0; y < (long) image1->rows; y++)
    {
      SetMagickPixelPacket(image1,p,indexes1+x,&pixel1);
      SetMagickPixelPacket(image2,q,indexes2+x,&pixel2);
      if (ComparePixels(method,&pixel1,&pixel2))
        break;
      p++;
      q++;
    }
    if (y < (long) image1->rows)
      break;
  }
  if (x >= (long) image1->columns)
    {
      /*
        Images are identical, return a null image.
      */
      bounds.x=-1;
      bounds.y=-1;
      bounds.width=1;
      bounds.height=1;
      return(bounds);
    }
  bounds.x=x;
  for (x=(long) image1->columns-1; x >= 0; x--)
  {
    p=GetVirtualPixels(image1,x,0,1,image1->rows,exception);
    q=GetVirtualPixels(image2,x,0,1,image2->rows,exception);
    if ((p == (const PixelPacket *) NULL) ||
        (q == (const PixelPacket *) NULL))
      break;
    indexes1=GetVirtualIndexQueue(image1);
    indexes2=GetVirtualIndexQueue(image2);
    for (y=0; y < (long) image1->rows; y++)
    {
      SetMagickPixelPacket(image1,p,indexes1+x,&pixel1);
      SetMagickPixelPacket(image2,q,indexes2+x,&pixel2);
      if (ComparePixels(method,&pixel1,&pixel2))
        break;
      p++;
      q++;
    }
    if (y < (long) image1->rows)
      break;
  }
  bounds.width=(unsigned long) (x-bounds.x+1);
  for (y=0; y < (long) image1->rows; y++)
  {
    p=GetVirtualPixels(image1,0,y,image1->columns,1,exception);
    q=GetVirtualPixels(image2,0,y,image2->columns,1,exception);
    if ((p == (const PixelPacket *) NULL) ||
        (q == (const PixelPacket *) NULL))
      break;
    indexes1=GetVirtualIndexQueue(image1);
    indexes2=GetVirtualIndexQueue(image2);
    for (x=0; x < (long) image1->columns; x++)
    {
      SetMagickPixelPacket(image1,p,indexes1+x,&pixel1);
      SetMagickPixelPacket(image2,q,indexes2+x,&pixel2);
      if (ComparePixels(method,&pixel1,&pixel2))
        break;
      p++;
      q++;
    }
    if (x < (long) image1->columns)
      break;
  }
  bounds.y=y;
  for (y=(long) image1->rows-1; y >= 0; y--)
  {
    p=GetVirtualPixels(image1,0,y,image1->columns,1,exception);
    q=GetVirtualPixels(image2,0,y,image2->columns,1,exception);
    if ((p == (const PixelPacket *) NULL) ||
        (q == (const PixelPacket *) NULL))
      break;
    indexes1=GetVirtualIndexQueue(image1);
    indexes2=GetVirtualIndexQueue(image2);
    for (x=0; x < (long) image1->columns; x++)
    {
      SetMagickPixelPacket(image1,p,indexes1+x,&pixel1);
      SetMagickPixelPacket(image2,q,indexes2+x,&pixel2);
      if (ComparePixels(method,&pixel1,&pixel2))
        break;
      p++;
      q++;
    }
    if (x < (long) image1->columns)
      break;
  }
  bounds.height=(unsigned long) (y-bounds.y+1);
  return(bounds);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o m p a r e I m a g e L a y e r s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompareImageLayers() compares each image with the next in a sequence and
%  returns the minimum bounding region of all the pixel differences (of the
%  ImageLayerMethod specified) it discovers.
%
%  Images do NOT have to be the same size, though it is best that all the
%  images are 'coalesced' (images are all the same size, on a flattened
%  canvas, so as to represent exactly how an specific frame should look).
%
%  No GIF dispose methods are applied, so GIF animations must be coalesced
%  before applying this image operator to find differences to them.
%
%  The format of the CompareImageLayers method is:
%
%      Image *CompareImageLayers(const Image *images,
%        const ImageLayerMethod method,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o method: the layers type to compare images with. Must be one of...
%              CompareAnyLayer, CompareClearLayer, CompareOverlayLayer.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *CompareImageLayers(const Image *image,
  const ImageLayerMethod method, ExceptionInfo *exception)
{
  Image
    *image_a,
    *image_b,
    *layers;

  RectangleInfo
    *bounds;

  register const Image
    *next;

  register long
    i;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  assert((method == CompareAnyLayer) ||
         (method == CompareClearLayer) ||
         (method == CompareOverlayLayer));
  /*
    Allocate bounds memory.
  */
  next=GetFirstImageInList(image);
  bounds=(RectangleInfo *) AcquireQuantumMemory((size_t)
    GetImageListLength(next),sizeof(*bounds));
  if (bounds == (RectangleInfo *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Set up first comparision images.
  */
  image_a=CloneImage(next,next->page.width,next->page.height,
    MagickTrue,exception);
  if (image_a == (Image *) NULL)
    {
      bounds=(RectangleInfo *) RelinquishMagickMemory(bounds);
      return((Image *) NULL);
    }
  image_a->background_color.opacity=(Quantum) TransparentOpacity;
  (void) SetImageBackgroundColor(image_a);
  image_a->page=next->page;
  image_a->page.x=0;
  image_a->page.y=0;
  (void) CompositeImage(image_a,CopyCompositeOp,next,next->page.x,next->page.y);
  /*
    Compute the bounding box of changes for the later images
  */
  i=0;
  next=GetNextImageInList(next);
  for ( ; next != (const Image *) NULL; next=GetNextImageInList(next))
  {
    image_b=CloneImage(image_a,0,0,MagickTrue,exception);
    if (image_b == (Image *) NULL)
      {
        image_a=DestroyImage(image_a);
        bounds=(RectangleInfo *) RelinquishMagickMemory(bounds);
        return((Image *) NULL);
      }
    (void) CompositeImage(image_a,CopyCompositeOp,next,next->page.x,
                           next->page.y);
    bounds[i]=CompareImageBounds(image_b,image_a,method,exception);

    image_b=DestroyImage(image_b);
    i++;
  }
  image_a=DestroyImage(image_a);
  /*
    Clone first image in sequence.
  */
  next=GetFirstImageInList(image);
  layers=CloneImage(next,0,0,MagickTrue,exception);
  if (layers == (Image *) NULL)
    {
      bounds=(RectangleInfo *) RelinquishMagickMemory(bounds);
      return((Image *) NULL);
    }
  /*
    Deconstruct the image sequence.
  */
  i=0;
  next=GetNextImageInList(next);
  for ( ; next != (const Image *) NULL; next=GetNextImageInList(next))
  {
    image_a=CloneImage(next,0,0,MagickTrue,exception);
    if (image_a == (Image *) NULL)
      break;
    image_b=CropImage(image_a,&bounds[i],exception);
    image_a=DestroyImage(image_a);
    if (image_b == (Image *) NULL)
      break;
    AppendImageToList(&layers,image_b);
    i++;
  }
  bounds=(RectangleInfo *) RelinquishMagickMemory(bounds);
  if (next != (Image *) NULL)
    {
      layers=DestroyImageList(layers);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(layers));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     D e c o n s t r u c t I m a g e s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeconstructImages() compares each image with the next in a sequence and
%  returns the minimum bounding region of all differences from the first image.
%
%  This function is deprecated in favor of the more universal
%  CompareImageLayers() function.
%
%  The format of the DeconstructImages method is:
%
%      Image *DeconstructImages(const Image *images, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image *DeconstructImages(const Image *images,
  ExceptionInfo *exception)
{
  return(CompareImageLayers(images,CompareAnyLayer,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     O p t i m i z e L a y e r F r a m e s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OptimizeLayerFrames() compares each image the GIF disposed forms of the
%  previous image in the sequence.  From this it attempts to select the
%  smallest cropped image to replace each frame, while preserving the results
%  of the animation.
%
%  Note that this not easy, and may require the expandsion of the bounds
%  of previous frame, to clear pixels for the next animation frame,
%  using GIF Background Dispose method.
%
%  Currently this only used internally, with external wrappers below.
%
%  The format of the OptimizeLayerFrames method is:
%
%      static Image *OptimizeLayerFrames(const Image *image,
%               const ImageLayerMethod method, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o method: the layers type to optimize with. Must be one of...
%             OptimizeImageLayer, or  OptimizePlusLayer
%
%    o exception: return any errors or warnings in this structure.
%
*/
/*
  Define a 'fake' dispose method where the frame is duplicated, with a
  extra zero time delay frame which does a BackgroundDisposal to clear the
  pixels that need to be cleared.
*/
#define DupDispose  ((DisposeType)9)
/*
  Another 'fake' dispose method used to removed frames that don't change.
*/
#define DelDispose  ((DisposeType)8)

static Image *OptimizeLayerFrames(const Image *image,
  const ImageLayerMethod method, ExceptionInfo *exception)
{
  ExceptionInfo
    *sans_exception;

  Image
    *prev_image,
    *dup_image,
    *bgnd_image,
    *optimized_image;

  RectangleInfo
    try_bounds,
    bgnd_bounds,
    dup_bounds,
    *bounds;

  MagickBooleanType
    add_frames,
    try_cleared,
    cleared;

  DisposeType
    *disposals;

  register const Image
    *next;

  register long
    i;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  assert(method == OptimizeLayer ||
         method == OptimizeImageLayer ||
         method == OptimizePlusLayer);

  /*
    Are we allowed to add/remove frames from animation
  */
  add_frames=method == OptimizePlusLayer ? MagickTrue : MagickFalse;
  /*
    Ensure  all the images are the same size
  */
  next=GetFirstImageInList(image);
  for (; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if ((next->columns != image->columns) || (next->rows != image->rows))
      ThrowImageException(OptionError,"ImagesAreNotTheSameSize");
    /*
      FUTURE: also check they are fully coalesced (full page settings)
    */
  }
  /*
    Allocate memory (times 2 if we allow frame additions)
  */
  next=GetFirstImageInList(image);
  bounds=(RectangleInfo *) AcquireQuantumMemory((size_t)
    GetImageListLength(next),(add_frames != MagickFalse ? 2UL : 1UL)*
    sizeof(*bounds));
  if (bounds == (RectangleInfo *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  disposals=(DisposeType *) AcquireQuantumMemory((size_t)
    GetImageListLength(image),(add_frames != MagickFalse ? 2UL : 1UL)*
    sizeof(*disposals));
  if (disposals == (DisposeType *) NULL)
    {
      bounds=(RectangleInfo *) RelinquishMagickMemory(bounds);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Initialise Previous Image as fully transparent
  */
  prev_image=CloneImage(next,next->page.width,next->page.height,
    MagickTrue,exception);
  if (prev_image == (Image *) NULL)
    {
      bounds=(RectangleInfo *) RelinquishMagickMemory(bounds);
      disposals=(DisposeType *) RelinquishMagickMemory(disposals);
      return((Image *) NULL);
    }
  prev_image->page=next->page;  /* ERROR: <-- should not be need, but is! */
  prev_image->page.x=0;
  prev_image->page.y=0;
  prev_image->dispose=NoneDispose;

  prev_image->background_color.opacity=(Quantum) TransparentOpacity;
  (void) SetImageBackgroundColor(prev_image);
  /*
    Figure out the area of overlay of the first frame
    No pixel could be cleared as all pixels are already cleared.
  */
  disposals[0]=NoneDispose;
  bounds[0]=CompareImageBounds(prev_image,next,CompareAnyLayer,exception);
  /*
    Compute the bounding box of changes for each pair of images.
  */
  i=1;
  bgnd_image=(Image *)NULL;
  dup_image=(Image *)NULL;
  dup_bounds.width=0;
  dup_bounds.height=0;
  dup_bounds.x=0;
  dup_bounds.y=0;
  next=GetNextImageInList(next);
  for ( ; next != (const Image *) NULL; next=GetNextImageInList(next))
  {
    /*
      Assume none disposal is the best
    */
    bounds[i]=CompareImageBounds(next->previous,next,CompareAnyLayer,exception);
    cleared=IsBoundsCleared(next->previous,next,&bounds[i],exception);
    disposals[i-1]=NoneDispose;
    if ( bounds[i].x < 0 ) {
      /*
        Image frame is exactly the same as the previous frame!
        If not adding frames leave it to be cropped down to a null image.
        Otherwise mark previous image for deleted, transfering its crop bounds
        to the current image.
      */
      if ( add_frames && i>=2 ) {
        disposals[i-1]=DelDispose;
        disposals[i]=NoneDispose;
        bounds[i]=bounds[i-1];
        i++;
        continue;
      }
    }
    else
      {
        /*
          Compare a none disposal against a previous disposal
        */
        try_bounds=CompareImageBounds(prev_image,next,CompareAnyLayer,exception);
        try_cleared=IsBoundsCleared(prev_image,next,&try_bounds,exception);
        if ( (!try_cleared && cleared ) ||
                try_bounds.width * try_bounds.height
                    <  bounds[i].width * bounds[i].height )
          {
            cleared=try_cleared;
            bounds[i]=try_bounds;
            disposals[i-1]=PreviousDispose;
          }

        /*
          If we are allowed lets try a complex frame duplication.
          It is useless if the previous image already clears pixels correctly.
          This method will always clear all the pixels that need to be cleared.
        */
        dup_bounds.width=dup_bounds.height=0;
        if ( add_frames )
          {
            dup_image=CloneImage(next->previous,next->previous->page.width,
                next->previous->page.height,MagickTrue,exception);
            if (dup_image == (Image *) NULL)
              {
                bounds=(RectangleInfo *) RelinquishMagickMemory(bounds);
                disposals=(DisposeType *) RelinquishMagickMemory(disposals);
                prev_image=DestroyImage(prev_image);
                return((Image *) NULL);
              }
            dup_bounds=CompareImageBounds(dup_image,next,CompareClearLayer,exception);
            ClearBounds(dup_image,&dup_bounds);
            try_bounds=CompareImageBounds(dup_image,next,CompareAnyLayer,exception);
            if ( cleared ||
                   dup_bounds.width*dup_bounds.height
                      +try_bounds.width*try_bounds.height
                   < bounds[i].width * bounds[i].height )
              {
                cleared=MagickFalse;
                bounds[i]=try_bounds;
                disposals[i-1]=DupDispose;
                /* to be finalised later, if found to be optimial */
              }
            else
              dup_bounds.width=dup_bounds.height=0;
          }

        /*
          Now compare against a simple background disposal
        */
        bgnd_image=CloneImage(next->previous,next->previous->page.width,
          next->previous->page.height,MagickTrue,exception);
        if (bgnd_image == (Image *) NULL)
          {
            bounds=(RectangleInfo *) RelinquishMagickMemory(bounds);
            disposals=(DisposeType *) RelinquishMagickMemory(disposals);
            prev_image=DestroyImage(prev_image);
            if ( disposals[i-1] == DupDispose )
              bgnd_image=DestroyImage(bgnd_image);
            return((Image *) NULL);
          }
        bgnd_bounds=bounds[i-1];
        ClearBounds(bgnd_image,&bgnd_bounds);
        try_bounds=CompareImageBounds(bgnd_image,next,CompareAnyLayer,exception);

        try_cleared=IsBoundsCleared(bgnd_image,next,&try_bounds,exception);
        if ( try_cleared )
          {
            /*
              Straight background disposal failed to clear pixels needed!
              Lets try expanding the disposal area of the previous frame, to
              include the pixels that are cleared.  This guaranteed
              to work, though may not be the most optimized solution.
            */
            try_bounds=CompareImageBounds(prev_image,next,CompareClearLayer,exception);
            if ( bgnd_bounds.x < 0 )
              bgnd_bounds = try_bounds;
            else
              {
                if ( try_bounds.x < bgnd_bounds.x )
                  {
                     bgnd_bounds.width+= bgnd_bounds.x-try_bounds.x;
                     if ( bgnd_bounds.width < try_bounds.width )
                       bgnd_bounds.width = try_bounds.width;
                     bgnd_bounds.x = try_bounds.x;
                  }
                else
                  {
                     try_bounds.width += try_bounds.x - bgnd_bounds.x;
                     if ( bgnd_bounds.width < try_bounds.width )
                       bgnd_bounds.width = try_bounds.width;
                  }
                if ( try_bounds.y < bgnd_bounds.y )
                  {
                     bgnd_bounds.height += bgnd_bounds.y - try_bounds.y;
                     if ( bgnd_bounds.height < try_bounds.height )
                       bgnd_bounds.height = try_bounds.height;
                     bgnd_bounds.y = try_bounds.y;
                  }
                else
                  {
                    try_bounds.height += try_bounds.y - bgnd_bounds.y;
                     if ( bgnd_bounds.height < try_bounds.height )
                       bgnd_bounds.height = try_bounds.height;
                  }
              }
            ClearBounds(bgnd_image,&bgnd_bounds);
            try_bounds=CompareImageBounds(bgnd_image,next,CompareAnyLayer,exception);
          }
        /*
          Test if this background dispose is smaller than any of the
          other methods we tryed before this (including duplicated frame)
        */
        if ( cleared ||
              bgnd_bounds.width*bgnd_bounds.height
                +try_bounds.width*try_bounds.height
              < bounds[i-1].width*bounds[i-1].height
                  +dup_bounds.width*dup_bounds.height
                  +bounds[i].width*bounds[i].height )
          {
            cleared=MagickFalse;
            bounds[i-1]=bgnd_bounds;
            bounds[i]=try_bounds;
            if ( disposals[i-1] == DupDispose )
              dup_image=DestroyImage(dup_image);
            disposals[i-1]=BackgroundDispose;
          }
      }
    /*
       Finalise choice of dispose, set new prev_image,
       and junk any extra images as appropriate,
    */
    if ( disposals[i-1] == DupDispose )
      {
         if (bgnd_image != (Image *) NULL)
           bgnd_image=DestroyImage(bgnd_image);
         prev_image=DestroyImage(prev_image);
         prev_image=dup_image, dup_image=(Image *) NULL;
         bounds[i+1]=bounds[i];
         bounds[i]=dup_bounds;
         disposals[i-1]=DupDispose;
         disposals[i]=BackgroundDispose;
         i++;
      }
    else
      {
        if ( disposals[i-1] != PreviousDispose )
          prev_image=DestroyImage(prev_image);
        if ( disposals[i-1] == BackgroundDispose )
          prev_image=bgnd_image,  bgnd_image=(Image *)NULL;
        else if (bgnd_image != (Image *) NULL)
          bgnd_image=DestroyImage(bgnd_image);
        if ( dup_image != (Image *) NULL)
          dup_image=DestroyImage(dup_image);
        if ( disposals[i-1] == NoneDispose )
          {
            prev_image=CloneImage(next->previous,next->previous->page.width,
              next->previous->page.height,MagickTrue,exception);
            if (prev_image == (Image *) NULL)
              {
                bounds=(RectangleInfo *) RelinquishMagickMemory(bounds);
                disposals=(DisposeType *) RelinquishMagickMemory(disposals);
                return((Image *) NULL);
              }
          }
      }
    disposals[i]=disposals[i-1];
    i++;
  }
  prev_image=DestroyImage(prev_image);
  /*
    Optimize all images in sequence.
  */
  sans_exception=AcquireExceptionInfo();
  i=0;
  next=GetFirstImageInList(image);
  optimized_image=NewImageList();
  while ( next != (const Image *) NULL )
  {
#if 0 /*  For debuging */
    printf("image %ld :- %d  %ldx%ld%+ld%+ld\n", i, disposals[i],
           bounds[i].width, bounds[i].height, bounds[i].x, bounds[i].y );
#endif
    prev_image=CloneImage(next,0,0,MagickTrue,exception);
    if (prev_image == (Image *) NULL)
      break;
    if ( disposals[i] == DelDispose ) {
      unsigned long time = 0;
      while ( disposals[i] == DelDispose ) {
        time += next->delay*1000/next->ticks_per_second;
        next=GetNextImageInList(next);
        i++;
      }
      time += next->delay*1000/next->ticks_per_second;
      prev_image->ticks_per_second = 100L;
      prev_image->delay = time*prev_image->ticks_per_second/1000;
    }
    bgnd_image=CropImage(prev_image,&bounds[i],sans_exception);
    prev_image=DestroyImage(prev_image);
    if (bgnd_image == (Image *) NULL)
      break;
    bgnd_image->dispose=disposals[i];
    if ( disposals[i] == DupDispose ) {
      bgnd_image->delay=0;
      bgnd_image->dispose=NoneDispose;
    }
    else
      next=GetNextImageInList(next);
    AppendImageToList(&optimized_image,bgnd_image);
    i++;
  }
  sans_exception=DestroyExceptionInfo(sans_exception);
  bounds=(RectangleInfo *) RelinquishMagickMemory(bounds);
  disposals=(DisposeType *) RelinquishMagickMemory(disposals);
  if (next != (Image *) NULL)
    {
      optimized_image=DestroyImageList(optimized_image);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(optimized_image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     O p t i m i z e I m a g e L a y e r s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OptimizeImageLayers() compares each image the GIF disposed forms of the
%  previous image in the sequence.  From this it attempts to select the
%  smallest cropped image to replace each frame, while preserving the results
%  of the GIF animation.
%
%  The format of the OptimizeImageLayers method is:
%
%      Image *OptimizeImageLayers(const Image *image,
%               ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *OptimizeImageLayers(const Image *image,
  ExceptionInfo *exception)
{
  return(OptimizeLayerFrames(image,OptimizeImageLayer,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     O p t i m i z e P l u s I m a g e L a y e r s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OptimizeImagePlusLayers() is exactly as OptimizeImageLayers(), but may
%  also add or even remove extra frames in the animation, if it improves
%  the total number of pixels in the resulting GIF animation.
%
%  The format of the OptimizePlusImageLayers method is:
%
%      Image *OptimizePlusImageLayers(const Image *image,
%               ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *OptimizePlusImageLayers(const Image *image,
  ExceptionInfo *exception)
{
  return OptimizeLayerFrames(image, OptimizePlusLayer, exception);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     O p t i m i z e I m a g e T r a n s p a r e n c y                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OptimizeImageTransparency() takes a frame optimized GIF animation, and
%  compares the overlayed pixels against the disposal image resulting from all
%  the previous frames in the animation.  Any pixel that does not change the
%  disposal image (and thus does not effect the outcome of an overlay) is made
%  transparent.
%
%  WARNING: This modifies the current images directly, rather than generate
%  a new image sequence.
%
%  The format of the OptimizeImageTransperency method is:
%
%      void OptimizeImageTransperency(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image sequence
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void OptimizeImageTransparency(const Image *image,
     ExceptionInfo *exception)
{
  Image
    *dispose_image;

  register Image
    *next;

  /*
    Run the image through the animation sequence
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  next=GetFirstImageInList(image);
  dispose_image=CloneImage(next,next->page.width,next->page.height,
    MagickTrue,exception);
  if (dispose_image == (Image *) NULL)
    return;
  dispose_image->page=next->page;
  dispose_image->page.x=0;
  dispose_image->page.y=0;
  dispose_image->dispose=NoneDispose;
  dispose_image->background_color.opacity=(Quantum) TransparentOpacity;
  (void) SetImageBackgroundColor(dispose_image);

  while ( next != (Image *) NULL )
  {
    Image
      *current_image;

    /*
      Overlay this frame's image over the previous disposal image
    */
    current_image=CloneImage(dispose_image,0,0,MagickTrue,exception);
    if (current_image == (Image *) NULL)
      {
        dispose_image=DestroyImage(dispose_image);
        return;
      }
    (void) CompositeImage(current_image,next->matte != MagickFalse ?
      OverCompositeOp : CopyCompositeOp, next,next->page.x,next->page.y);
    /*
      At this point the image would be displayed, for the delay period
    **
      Work out the disposal of the previous image
    */
    if (next->dispose == BackgroundDispose)
      {
        RectangleInfo
          bounds=next->page;

        bounds.width=next->columns;
        bounds.height=next->rows;
        if (bounds.x < 0)
          {
            bounds.width+=bounds.x;
            bounds.x=0;
          }
        if ((long) (bounds.x+bounds.width) > (long) current_image->columns)
          bounds.width=current_image->columns-bounds.x;
        if (bounds.y < 0)
          {
            bounds.height+=bounds.y;
            bounds.y=0;
          }
        if ((long) (bounds.y+bounds.height) > (long) current_image->rows)
          bounds.height=current_image->rows-bounds.y;
        ClearBounds(current_image, &bounds);
      }
    if (next->dispose != PreviousDispose)
      {
        dispose_image=DestroyImage(dispose_image);
        dispose_image=current_image;
      }
    else
      current_image=DestroyImage(current_image);

    /*
      Optimize Transparency of the next frame (if present)
    */
    next=GetNextImageInList(next);
    if ( next != (Image *) NULL ) {
      (void) CompositeImage(next, ChangeMaskCompositeOp,
          dispose_image, -(next->page.x), -(next->page.y) );
    }
  }
  dispose_image=DestroyImage(dispose_image);
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R e m o v e D u p l i c a t e L a y e r s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveDuplicateLayers() removes any image that is exactly the same as the
%  next image in the given image list.  Image size and virtual canvas offset
%  must also match, though not the virtual canvas size itself.
%
%  No check is made with regards to image disposal setting, though it is the
%  dispose setting of later image that is kept.  Also any time delays are also
%  added together. As such coalesced image animations should still produce the
%  same result, though with duplicte frames merged into a single frame.
%
%  The format of the RemoveDuplicateLayers method is:
%
%      void RemoveDuplicateLayers(Image **image, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image list
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void RemoveDuplicateLayers(Image **images,
     ExceptionInfo *exception)
{
  register Image
    *curr,
    *next;

  RectangleInfo
    bounds;

  assert((*images) != (const Image *) NULL);
  assert((*images)->signature == MagickSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*images)->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  curr=GetFirstImageInList(*images);
  for (; (next=GetNextImageInList(curr)) != (Image *) NULL; curr=next)
  {
    if ( curr->columns != next->columns || curr->rows != next->rows
         || curr->page.x != next->page.x || curr->page.y != next->page.y )
      continue;
    bounds=CompareImageBounds(curr,next,CompareAnyLayer,exception);
    if ( bounds.x < 0 ) {
      /*
        the two images are the same, merge time delays and delete one.
      */
      unsigned long time;
      time = curr->delay*1000/curr->ticks_per_second;
      time += next->delay*1000/next->ticks_per_second;
      next->ticks_per_second = 100L;
      next->delay = time*curr->ticks_per_second/1000;
      next->iterations = curr->iterations;
      *images = curr;
      (void) DeleteImageFromList(images);
    }
  }
  *images = GetFirstImageInList(*images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R e m o v e Z e r o D e l a y L a y e r s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveZeroDelayLayers() removes any image that as a zero delay time. Such
%  images generally represent intermediate or partial updates in GIF
%  animations used for file optimization.  They are not ment to be displayed
%  to users of the animation.  Viewable images in an animation should have a
%  time delay of 3 or more centi-seconds (hundredths of a second).
%
%  However if all the frames have a zero time delay, then either the animation
%  is as yet incomplete, or it is not a GIF animation.  This a non-sensible
%  situation, so no image will be removed and a 'Zero Time Animation' warning
%  (exception) given.
%
%  No warning will be given if no image was removed because all images had an
%  appropriate non-zero time delay set.
%
%  Due to the special requirements of GIF disposal handling, GIF animations
%  should be coalesced first, before calling this function, though that is not
%  a requirement.
%
%  The format of the RemoveZeroDelayLayers method is:
%
%      void RemoveZeroDelayLayers(Image **image, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image list
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void RemoveZeroDelayLayers(Image **images,
     ExceptionInfo *exception)
{
  Image
    *i;

  assert((*images) != (const Image *) NULL);
  assert((*images)->signature == MagickSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*images)->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  i=GetFirstImageInList(*images);
  for ( ; i != (Image *) NULL; i=GetNextImageInList(i))
    if ( i->delay != 0L ) break;
  if ( i == (Image *) NULL ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
       "ZeroTimeAnimation","`%s'",GetFirstImageInList(*images)->filename);
    return;
  }
  i=GetFirstImageInList(*images);
  while ( i != (Image *) NULL )
  {
    if ( i->delay == 0L ) {
      (void) DeleteImageFromList(&i);
      *images=i;
    }
    else
      i=GetNextImageInList(i);
  }
  *images=GetFirstImageInList(*images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o m p o s i t e L a y e r s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompositeLayers() compose first image sequence (source) over the second
%  image sequence (destination), using the given compose method and offsets.
%
%  The pointers to the image list does not have to be the start of that image
%  list, but may start somewhere in the middle.  Each layer from the two image
%  lists are composted together until the end of one of the image lists is
%  reached.  The offset of each composition is also adjusted to match the
%  virtual canvas offsets of each layer. As such the given offset is relative
%  to the virtual canvas, and not the actual image.
%
%  No GIF disposal handling is performed, so GIF animations should be
%  coalesced before use.  However this not a requirement, and individual
%  layer images may have any size or offset, for special compositions.
%
%  Special case:- If one of the image sequences is just a single image that
%  image is repeatally composed with all the images in the other image list.
%  Either the source or destination lists may be the single image, for this
%  situation.
%
%  The destination list will be expanded as needed to match number of source
%  image overlaid (from current position to end of list).
%
%  The format of the CompositeLayers method is:
%
%      void CompositeLayers(Image *destination,
%          const CompositeOperator compose, Image *source,
%          const long x_offset, const long y_offset,
%          ExceptionInfo *exception);
%
%  A description of each parameter follows:
%
%    o destination: the destination images and results
%
%    o source: source image(s) for the layer composition
%
%    o compose, x_offset, y_offset:  arguments passed on to CompositeImages()
%
%    o exception: return any errors or warnings in this structure.
%
*/
static inline void CompositeCanvas(Image *destination,
     const CompositeOperator compose, Image *source,
     long x_offset, long y_offset )
{
  x_offset += source->page.x - destination->page.x;
  y_offset += source->page.y - destination->page.y;
  (void) CompositeImage(destination, compose, source, x_offset, y_offset);
}

MagickExport void CompositeLayers(Image *destination,
      const CompositeOperator compose, Image *source,
      const long x_offset, const long y_offset,
      ExceptionInfo *exception)
{
  assert(destination != (Image *) NULL);
  assert(destination->signature == MagickSignature);
  assert(source != (Image *) NULL);
  assert(source->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (source->debug != MagickFalse || destination->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s - %s",
         source->filename, destination->filename);

  /*
    Overlay single source image over destation image/list
  */
  if ( source->previous == (Image *) NULL && source->next == (Image *) NULL )
    while ( destination != (Image *) NULL )
    {
      CompositeCanvas(destination, compose, source, x_offset, y_offset);
      destination=GetNextImageInList(destination);
    }

  /*
    Overlay source image list over single destination
    Generating multiple clones of destination image to match source list.
    Original Destination image becomes first image of generated list.
    As such the image list pointer does not require any change in caller.
    Some animation attributes however also needs coping in this case.
  */
  else if ( destination->previous == (Image *) NULL &&
            destination->next == (Image *) NULL )
  {
    Image *dest = CloneImage(destination,0,0,MagickTrue,exception);

    CompositeCanvas(destination, compose, source, x_offset, y_offset);
    /* copy source image attributes ? */
    source=GetNextImageInList(source);

    while ( source != (Image *) NULL )
    {
      AppendImageToList(&destination,
           CloneImage(dest,0,0,MagickTrue,exception));
      destination=GetLastImageInList(destination);

      CompositeCanvas(destination, compose, source, x_offset, y_offset);
      destination->delay = source->delay;
      destination->iterations = source->iterations;
      source=GetNextImageInList(source);
    }
    dest=DestroyImage(dest);
  }

  /*
    Overlay a source image list over a destination image list
    until either list runs out of images. (Does not repeat)
  */
  else
    while ( source != (Image *) NULL && destination != (Image *) NULL )
    {
      CompositeCanvas(destination, compose, source, x_offset, y_offset);
      source=GetNextImageInList(source);
      destination=GetNextImageInList(destination);
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M e r g e I m a g e L a y e r s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MergeImageLayers() composes all the image layers from the current given
%  image onward to produce a single image of the merged layers.
%
%  The inital canvas's size depends on the given ImageLayerMethod, and is
%  initialized using the first images background color.  The images
%  are then compositied onto that image in sequence using the given
%  composition that has been assigned to each individual image.
%
%  The format of the MergeImageLayers is:
%
%      Image *MergeImageLayers(const Image *image,
%        const ImageLayerMethod method, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image list to be composited together
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
%        TrimBoundsLayer: Determine the overall bounds of all the image
%           layers just as in "MergeLayer", then adjust the the canvas
%           and offsets to be relative to those bounds, without overlaying
%           the images.
%
%           WARNING: a new image is not returned, the original image
%           sequence page data is modified instead.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *MergeImageLayers(Image *image,
  const ImageLayerMethod method,ExceptionInfo *exception)
{
#define MergeLayersTag  "Merge/Layers"

  Image
    *canvas;

  MagickBooleanType
    proceed;

  MagickOffsetType
    scene;

  RectangleInfo
    page;

  unsigned long
    width,
    height;

  register const Image
    *next;

  unsigned long
    number_images;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  /*
    Determine canvas image size, and its virtual canvas size and offset
  */
  page=image->page;
  width=image->columns;
  height=image->rows;
  switch (method)
  {
    case TrimBoundsLayer:
    case MergeLayer:
    default:
    {
      next = GetNextImageInList(image);
      for ( ; next != (Image *) NULL;  next=GetNextImageInList(next)) {
        if ( page.x > next->page.x ) {
             width += page.x-next->page.x;
             page.x = next->page.x;
        }
        if ( page.y > next->page.y ) {
             height += page.y-next->page.y;
             page.y = next->page.y;
        }
        if ( width < (next->page.x + next->columns - page.x) )
           width = (unsigned long) next->page.x + next->columns - page.x;
        if ( height < (next->page.y + next->rows - page.y) )
           height = (unsigned long) next->page.y + next->rows - page.y;
      }
      break;
    }
    case FlattenLayer:
    {
      if ( page.width > 0 )
        width=page.width;
      if ( page.height > 0 )
        height=page.height;
      page.x=0;
      page.y=0;
      break;
    }
    case MosaicLayer:
    {
      if ( page.width > 0 )
        width=page.width;
      if ( page.height > 0 )
        height=page.height;
      for (next=image; next != (Image *) NULL; next=GetNextImageInList(next)) {
        if (method == MosaicLayer) {
          page.x=next->page.x;
          page.y=next->page.y;
          if ( width < (next->page.x + next->columns) )
             width = (unsigned long) next->page.x + next->columns;
          if ( height < (next->page.y + next->rows) )
             height = (unsigned long) next->page.y + next->rows;
        }
      }
      page.width=width;
      page.height=height;
      page.x=0;
      page.y=0;
    }
    break;
  }
  /* set virtual canvas size if not defined */
  if ( page.width == 0 )
    page.width = (page.x < 0) ? width : width+page.x;
  if ( page.height == 0 )
    page.height = (page.y < 0) ? height : height+page.y;

  /*
    Handle "TrimBoundsLayer" method seperatally to normal 'layer merge'
  */
  if ( method == TrimBoundsLayer ) {
    number_images=GetImageListLength(image);
    for (scene=0; scene < (long) number_images; scene++)
    {
      image->page.x -= page.x;
      image->page.y -= page.y;
      image->page.width = width;
      image->page.height = height;
      proceed=SetImageProgress(image,MergeLayersTag,scene,number_images);
      if (proceed == MagickFalse)
        break;
      image=GetNextImageInList(image);
    }
    return((Image *) NULL);
  }

  /*
    Create canvas size of width and height, and background color.
  */
  canvas=CloneImage(image,width,height,MagickTrue,exception);
  if (canvas == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageBackgroundColor(canvas);
  canvas->page=page;
  canvas->dispose=UndefinedDispose;

  /*
    Compose images onto canvas, with progress monitor
  */
  number_images=GetImageListLength(image);
  for (scene=0; scene < (long) number_images; scene++)
  {
    (void) CompositeImage(canvas,image->compose,image,image->page.x-
      canvas->page.x,image->page.y-canvas->page.y);
    proceed=SetImageProgress(image,MergeLayersTag,scene,number_images);
    if (proceed == MagickFalse)
      break;
    image=GetNextImageInList(image);
  }
  return(canvas);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      PPPP    AAA   IIIII  N   N  TTTTT                      %
%                      P   P  A   A    I    NN  N    T                        %
%                      PPPP   AAAAA    I    N N N    T                        %
%                      P      A   A    I    N  NN    T                        %
%                      P      A   A  IIIII  N   N    T                        %
%                                                                             %
%                                                                             %
%                        Methods to Paint on an Image                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1998                                   %
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
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/draw.h"
#include "magick/draw-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/paint.h"
#include "magick/pixel-private.h"
#include "magick/string_.h"
#include "magick/thread-private.h"

static inline double MagickMax(const double x,const double y)
{
    return( x > y ? x : y);
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F l o o d f i l l P a i n t I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FloodfillPaintImage() changes the color value of any pixel that matches
%  target and is an immediate neighbor.  If the method FillToBorderMethod is
%  specified, the color value is changed for any neighbor pixel that does not
%  match the bordercolor member of image.
%
%  By default target must match a particular pixel color exactly.
%  However, in many cases two colors may differ by a small amount.  The
%  fuzz member of image defines how much tolerance is acceptable to
%  consider two colors as the same.  For example, set fuzz to 10 and the
%  color red at intensities of 100 and 102 respectively are now
%  interpreted as the same color for the purposes of the floodfill.
%
%  The format of the FloodfillPaintImage method is:
%
%      MagickBooleanType FloodfillPaintImage(Image *image,
%        const ChannelType channel,const DrawInfo *draw_info,
%        const MagickPixelPacket target,const long x_offset,const long y_offset,
%        const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel(s).
%
%    o draw_info: the draw info.
%
%    o target: the RGB value of the target color.
%
%    o x_offset,y_offset: the starting location of the operation.
%
%    o invert: paint any pixel that does not match the target color.
%
*/
MagickExport MagickBooleanType FloodfillPaintImage(Image *image,
  const ChannelType channel,const DrawInfo *draw_info,
  const MagickPixelPacket *target,const long x_offset,const long y_offset,
  const MagickBooleanType invert)
{
#define MaxStacksize  (1UL << 15)
#define PushSegmentStack(up,left,right,delta) \
{ \
  if (s >= (segment_stack+MaxStacksize)) \
    ThrowBinaryException(DrawError,"SegmentStackOverflow",image->filename) \
  else \
    { \
      if ((((up)+(delta)) >= 0) && (((up)+(delta)) < (long) image->rows)) \
        { \
          s->x1=(double) (left); \
          s->y1=(double) (up); \
          s->x2=(double) (right); \
          s->y2=(double) (delta); \
          s++; \
        } \
    } \
}

  ExceptionInfo
    *exception;

  Image
    *floodplane_image;

  long
    offset,
    start,
    x,
    x1,
    x2,
    y;

  MagickBooleanType
    skip;

  MagickPixelPacket
    fill,
    pixel;

  PixelPacket
    fill_color;

  register SegmentInfo
    *s;

  SegmentInfo
    *segment_stack;

  /*
    Check boundary conditions.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(draw_info != (DrawInfo *) NULL);
  assert(draw_info->signature == MagickSignature);
  if ((x_offset < 0) || (x_offset >= (long) image->columns))
    return(MagickFalse);
  if ((y_offset < 0) || (y_offset >= (long) image->rows))
    return(MagickFalse);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel);
  /*
    Set floodfill state.
  */
  floodplane_image=CloneImage(image,0,0,MagickTrue,&image->exception);
  if (floodplane_image == (Image *) NULL)
    return(MagickFalse);
  (void) SetImageAlphaChannel(floodplane_image,OpaqueAlphaChannel);
  segment_stack=(SegmentInfo *) AcquireQuantumMemory(MaxStacksize,
    sizeof(*segment_stack));
  if (segment_stack == (SegmentInfo *) NULL)
    {
      floodplane_image=DestroyImage(floodplane_image);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Push initial segment on stack.
  */
  exception=(&image->exception);
  x=x_offset;
  y=y_offset;
  start=0;
  s=segment_stack;
  PushSegmentStack(y,x,x,1);
  PushSegmentStack(y+1,x,x,-1);
  GetMagickPixelPacket(image,&fill);
  GetMagickPixelPacket(image,&pixel);
  while (s > segment_stack)
  {
    register const IndexPacket
      *__restrict indexes;

    register const PixelPacket
      *__restrict p;

    register long
      x;

    register PixelPacket
      *__restrict q;

    /*
      Pop segment off stack.
    */
    s--;
    x1=(long) s->x1;
    x2=(long) s->x2;
    offset=(long) s->y2;
    y=(long) s->y1+offset;
    /*
      Recolor neighboring pixels.
    */
    p=GetVirtualPixels(image,0,y,(unsigned long) (x1+1),1,exception);
    q=GetAuthenticPixels(floodplane_image,0,y,(unsigned long) (x1+1),1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    indexes=GetVirtualIndexQueue(image);
    p+=x1;
    q+=x1;
    for (x=x1; x >= 0; x--)
    {
      if (q->opacity == (Quantum) TransparentOpacity)
        break;
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      if (IsMagickColorSimilar(&pixel,target) == invert)
        break;
      q->opacity=(Quantum) TransparentOpacity;
      p--;
      q--;
    }
    if (SyncAuthenticPixels(floodplane_image,exception) == MagickFalse)
      break;
    skip=x >= x1 ? MagickTrue : MagickFalse;
    if (skip == MagickFalse)
      {
        start=x+1;
        if (start < x1)
          PushSegmentStack(y,start,x1-1,-offset);
        x=x1+1;
      }
    do
    {
      if (skip == MagickFalse)
        {
          if (x < (long) image->columns)
            {
              p=GetVirtualPixels(image,x,y,image->columns-x,1,exception);
              q=GetAuthenticPixels(floodplane_image,x,y,image->columns-x,1,
                exception);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              indexes=GetVirtualIndexQueue(image);
              for ( ; x < (long) image->columns; x++)
              {
                if (q->opacity == (Quantum) TransparentOpacity)
                  break;
                SetMagickPixelPacket(image,p,indexes+x,&pixel);
                if (IsMagickColorSimilar(&pixel,target) == invert)
                  break;
                q->opacity=(Quantum) TransparentOpacity;
                p++;
                q++;
              }
              if (SyncAuthenticPixels(floodplane_image,exception) == MagickFalse)
                break;
            }
          PushSegmentStack(y,start,x-1,offset);
          if (x > (x2+1))
            PushSegmentStack(y,x2+1,x-1,-offset);
        }
      skip=MagickFalse;
      x++;
      if (x <= x2)
        {
          p=GetVirtualPixels(image,x,y,(unsigned long) (x2-x+1),1,exception);
          q=GetAuthenticPixels(floodplane_image,x,y,(unsigned long) (x2-x+1),1,
            exception);
          if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
            break;
          indexes=GetVirtualIndexQueue(image);
          for ( ; x <= x2; x++)
          {
            if (q->opacity == (Quantum) TransparentOpacity)
              break;
            SetMagickPixelPacket(image,p,indexes+x,&pixel);
            if (IsMagickColorSimilar(&pixel,target) != invert)
              break;
            p++;
            q++;
          }
        }
      start=x;
    } while (x <= x2);
  }
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *__restrict p;

    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    /*
      Tile fill color onto floodplane.
    */
    p=GetVirtualPixels(floodplane_image,0,y,image->columns,1,exception);
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    indexes=GetAuthenticIndexQueue(image);
    for (x=0; x < (long) image->columns; x++)
    {
      if (p->opacity != OpaqueOpacity)
        {
          (void) GetFillColor(draw_info,x,y,&fill_color);
          SetMagickPixelPacket(image,&fill_color,(IndexPacket *) NULL,&fill);
          if (image->colorspace == CMYKColorspace)
            ConvertRGBToCMYK(&fill);
          if ((channel & RedChannel) != 0)
            q->red=RoundToQuantum(fill.red);
          if ((channel & GreenChannel) != 0)
            q->green=RoundToQuantum(fill.green);
          if ((channel & BlueChannel) != 0)
            q->blue=RoundToQuantum(fill.blue);
          if ((channel & OpacityChannel) != 0)
            q->opacity=RoundToQuantum(fill.opacity);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            indexes[x]=RoundToQuantum(fill.index);
        }
      p++;
      q++;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  segment_stack=(SegmentInfo *) RelinquishMagickMemory(segment_stack);
  floodplane_image=DestroyImage(floodplane_image);
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     G r a d i e n t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GradientImage() applies a continuously smooth color transitions along a
%  vector from one color to another.
%
%  Note, the interface of this method will change in the future to support
%  more than one transistion.
%
%  The format of the GradientImage method is:
%
%      MagickBooleanType GradientImage(Image *image,const GradientType type,
%        const SpreadMethod method,const PixelPacket *start_color,
%        const PixelPacket *stop_color)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o type: the gradient type: linear or radial.
%
%    o spread: the gradient spread meathod: pad, reflect, or repeat.
%
%    o start_color: the start color.
%
%    o stop_color: the stop color.
%
% This provides a good example of making use of the DrawGradientImage
% function and the gradient structure in draw_info.
*/
MagickExport MagickBooleanType GradientImage(Image *image,
  const GradientType type,const SpreadMethod method,
  const PixelPacket *start_color,const PixelPacket *stop_color)
{
  DrawInfo
    *draw_info;

  GradientInfo
    *gradient;

  MagickBooleanType
    status;

  register long
    i;

  /*
    Set gradient start-stop end points.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(start_color != (const PixelPacket *) NULL);
  assert(stop_color != (const PixelPacket *) NULL);
  draw_info=AcquireDrawInfo();
  gradient=(&draw_info->gradient);
  gradient->type=type;
  gradient->bounding_box.width=image->columns;
  gradient->bounding_box.height=image->rows;
  gradient->gradient_vector.x2=(double) image->columns-1.0;
  gradient->gradient_vector.y2=(double) image->rows-1.0;
  if ((type == LinearGradient) && (gradient->gradient_vector.y2 != 0.0))
    gradient->gradient_vector.x2=0.0;
  gradient->center.x=(double) gradient->gradient_vector.x2/2.0;
  gradient->center.y=(double) gradient->gradient_vector.y2/2.0;
  gradient->radius=MagickMax(gradient->center.x,gradient->center.y);
  gradient->spread=method;
  /*
    Define the gradient to fill between the stops.
  */
  gradient->number_stops=2;
  gradient->stops=(StopInfo *) AcquireQuantumMemory(gradient->number_stops,
    sizeof(*gradient->stops));
  if (gradient->stops == (StopInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  (void) ResetMagickMemory(gradient->stops,0,gradient->number_stops*
    sizeof(*gradient->stops));
  for (i=0; i < (long) gradient->number_stops; i++)
    GetMagickPixelPacket(image,&gradient->stops[i].color);
  SetMagickPixelPacket(image,start_color,(IndexPacket *) NULL,
    &gradient->stops[0].color);
  gradient->stops[0].offset=0.0;
  SetMagickPixelPacket(image,stop_color,(IndexPacket *) NULL,
    &gradient->stops[1].color);
  gradient->stops[1].offset=1.0;
  /*
    Draw a gradient on the image.
  */
  status=DrawGradientImage(image,draw_info);
  draw_info=DestroyDrawInfo(draw_info);
  if ((start_color->opacity == OpaqueOpacity) &&
      (stop_color->opacity == OpaqueOpacity))
    image->matte=MagickFalse;
  if ((IsGrayPixel(start_color) != MagickFalse) &&
      (IsGrayPixel(stop_color) != MagickFalse))
    image->type=GrayscaleType;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     O i l P a i n t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OilPaintImage() applies a special effect filter that simulates an oil
%  painting.  Each pixel is replaced by the most frequent color occurring
%  in a circular region defined by radius.
%
%  The format of the OilPaintImage method is:
%
%      Image *OilPaintImage(const Image *image,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the circular neighborhood.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static unsigned long **DestroyHistogramThreadSet(unsigned long **histogram)
{
  register long
    i;

  assert(histogram != (unsigned long **) NULL);
  for (i=0; i < (long) GetOpenMPMaximumThreads(); i++)
    if (histogram[i] != (unsigned long *) NULL)
      histogram[i]=(unsigned long *) RelinquishMagickMemory(histogram[i]);
  histogram=(unsigned long **) RelinquishAlignedMemory(histogram);
  return(histogram);
}

static unsigned long **AcquireHistogramThreadSet(const size_t count)
{
  register long
    i;

  unsigned long
    **histogram,
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  histogram=(unsigned long **) AcquireAlignedMemory(number_threads,
    sizeof(*histogram));
  if (histogram == (unsigned long **) NULL)
    return((unsigned long **) NULL);
  (void) ResetMagickMemory(histogram,0,number_threads*sizeof(*histogram));
  for (i=0; i < (long) number_threads; i++)
  {
    histogram[i]=(unsigned long *) AcquireQuantumMemory(count,
      sizeof(**histogram));
    if (histogram[i] == (unsigned long *) NULL)
      return(DestroyHistogramThreadSet(histogram));
  }
  return(histogram);
}

MagickExport Image *OilPaintImage(const Image *image,const double radius,
  ExceptionInfo *exception)
{
#define NumberPaintBins  256
#define OilPaintImageTag  "OilPaint/Image"

  Image
    *paint_image;

  long
    progress,
    y;

  MagickBooleanType
    status;

  unsigned long
    **histograms,
    width;

  CacheView
    *image_view,
    *paint_view;

  /*
    Initialize painted image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,0.5);
  if ((image->columns < width) || (image->rows < width))
    ThrowImageException(OptionError,"ImageSmallerThanRadius");
  paint_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if (paint_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(paint_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&paint_image->exception);
      paint_image=DestroyImage(paint_image);
      return((Image *) NULL);
    }
  histograms=AcquireHistogramThreadSet(NumberPaintBins);
  if (histograms == (unsigned long **) NULL)
    {
      paint_image=DestroyImage(paint_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Oil paint image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireCacheView(image);
  paint_view=AcquireCacheView(paint_image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *__restrict indexes;

    register const PixelPacket
      *__restrict p;

    register IndexPacket
      *__restrict paint_indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    register unsigned long
      *histogram;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((long) width/2L),y-(long) (width/
      2L),image->columns+width,width,exception);
    q=QueueCacheViewAuthenticPixels(paint_view,0,y,paint_image->columns,1,
      exception);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    paint_indexes=GetCacheViewAuthenticIndexQueue(paint_view);
    histogram=histograms[GetOpenMPThreadId()];
    for (x=0; x < (long) image->columns; x++)
    {
      long
        j,
        k,
        v;

      register long
        i,
        u;

      unsigned long
        count;

      /*
        Assign most frequent color.
      */
      i=0;
      j=0;
      count=0;
      (void) ResetMagickMemory(histogram,0,NumberPaintBins*sizeof(*histogram));
      for (v=0; v < (long) width; v++)
      {
        for (u=0; u < (long) width; u++)
        {
          k=(long) ScaleQuantumToChar(PixelIntensityToQuantum(p+u+i));
          histogram[k]++;
          if (histogram[k] > count)
            {
              j=i+u;
              count=histogram[k];
            }
        }
        i+=image->columns+width;
      }
      *q=(*(p+j));
      if (image->colorspace == CMYKColorspace)
        paint_indexes[x]=indexes[x+j];
      p++;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(paint_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_OilPaintImage)
#endif
        proceed=SetImageProgress(image,OilPaintImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  paint_view=DestroyCacheView(paint_view);
  image_view=DestroyCacheView(image_view);
  histograms=DestroyHistogramThreadSet(histograms);
  if (status == MagickFalse)
    paint_image=DestroyImage(paint_image);
  return(paint_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     O p a q u e P a i n t I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpaquePaintImage() changes any pixel that matches color with the color
%  defined by fill.
%
%  By default color must match a particular pixel color exactly.  However,
%  in many cases two colors may differ by a small amount.  Fuzz defines
%  how much tolerance is acceptable to consider two colors as the same.
%  For example, set fuzz to 10 and the color red at intensities of 100 and
%  102 respectively are now interpreted as the same color.
%
%  The format of the OpaquePaintImage method is:
%
%      MagickBooleanType OpaquePaintImage(Image *image,
%        const PixelPacket *target,const PixelPacket *fill,
%        const MagickBooleanType invert)
%      MagickBooleanType OpaquePaintImageChannel(Image *image,
%        const ChannelType channel,const PixelPacket *target,
%        const PixelPacket *fill,const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel(s).
%
%    o target: the RGB value of the target color.
%
%    o fill: the replacement color.
%
%    o invert: paint any pixel that does not match the target color.
%
*/

MagickExport MagickBooleanType OpaquePaintImage(Image *image,
  const MagickPixelPacket *target,const MagickPixelPacket *fill,
  const MagickBooleanType invert)
{
  return(OpaquePaintImageChannel(image,AllChannels,target,fill,invert));
}

MagickExport MagickBooleanType OpaquePaintImageChannel(Image *image,
  const ChannelType channel,const MagickPixelPacket *target,
  const MagickPixelPacket *fill,const MagickBooleanType invert)
{
#define OpaquePaintImageTag  "Opaque/Image"

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(target != (MagickPixelPacket *) NULL);
  assert(fill != (MagickPixelPacket *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  /*
    Make image color opaque.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  GetMagickPixelPacket(image,&zero);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    pixel=zero;
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,q,indexes+x,&pixel);
      if (IsMagickColorSimilar(&pixel,target) != invert)
        {
          if ((channel & RedChannel) != 0)
            q->red=RoundToQuantum(fill->red);
          if ((channel & GreenChannel) != 0)
            q->green=RoundToQuantum(fill->green);
          if ((channel & BlueChannel) != 0)
            q->blue=RoundToQuantum(fill->blue);
          if ((channel & OpacityChannel) != 0)
            q->opacity=RoundToQuantum(fill->opacity);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            indexes[x]=RoundToQuantum(fill->index);
        }
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_OpaquePaintImageChannel)
#endif
        proceed=SetImageProgress(image,OpaquePaintImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     T r a n s p a r e n t P a i n t I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransparentPaintImage() changes the opacity value associated with any pixel
%  that matches color to the value defined by opacity.
%
%  By default color must match a particular pixel color exactly.  However,
%  in many cases two colors may differ by a small amount.  Fuzz defines
%  how much tolerance is acceptable to consider two colors as the same.
%  For example, set fuzz to 10 and the color red at intensities of 100 and
%  102 respectively are now interpreted as the same color.
%
%  The format of the TransparentPaintImage method is:
%
%      MagickBooleanType TransparentPaintImage(Image *image,
%        const MagickPixelPacket *target,const Quantum opacity,
%        const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o target: the target color.
%
%    o opacity: the replacement opacity value.
%
%    o invert: paint any pixel that does not match the target color.
%
*/
MagickExport MagickBooleanType TransparentPaintImage(Image *image,
  const MagickPixelPacket *target,const Quantum opacity,
  const MagickBooleanType invert)
{
#define TransparentPaintImageTag  "Transparent/Image"

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    zero;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(target != (MagickPixelPacket *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel);
  /*
    Make image color transparent.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  GetMagickPixelPacket(image,&zero);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    pixel=zero;
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,q,indexes+x,&pixel);
      if (IsMagickColorSimilar(&pixel,target) != invert)
        q->opacity=opacity;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_TransparentPaintImage)
#endif
        proceed=SetImageProgress(image,TransparentPaintImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     T r a n s p a r e n t P a i n t I m a g e C h r o m a                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransparentPaintImageChroma() changes the opacity value associated with any
%  pixel that matches color to the value defined by opacity.
%
%  As there is one fuzz value for the all the channels, the
%  TransparentPaintImage() API is not suitable for the operations like chroma,
%  where the tolerance for similarity of two color component (RGB) can be
%  different, Thus we define this method take two target pixels (one
%  low and one hight) and all the pixels of an image which are lying between
%  these two pixels are made transparent.
%
%  The format of the TransparentPaintImage method is:
%
%      MagickBooleanType TransparentPaintImage(Image *image,
%        const MagickPixelPacket *low,const MagickPixelPacket *hight,
%        const Quantum opacity,const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o low: the low target color.
%
%    o high: the high target color.
%
%    o opacity: the replacement opacity value.
%
%    o invert: paint any pixel that does not match the target color.
%
*/
MagickExport MagickBooleanType TransparentPaintImageChroma(Image *image,
  const MagickPixelPacket *low,const MagickPixelPacket *high,
  const Quantum opacity,const MagickBooleanType invert)
{
#define TransparentPaintImageTag  "Transparent/Image"

  ExceptionInfo
    *exception;

  long
    progress,
    y;

  MagickBooleanType
    status;

  CacheView
    *image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(high != (MagickPixelPacket *) NULL);
  assert(low != (MagickPixelPacket *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,ResetAlphaChannel);
  /*
    Make image color transparent.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,1) shared(progress,status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      match;

    MagickPixelPacket
      pixel;

    register IndexPacket
      *__restrict indexes;

    register long
      x;

    register PixelPacket
      *__restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    GetMagickPixelPacket(image,&pixel);
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,q,indexes+x,&pixel);
      match=((pixel.red >= low->red) && (pixel.red <= high->red) &&
        (pixel.green >= low->green) && (pixel.green <= high->green) &&
        (pixel.blue  >= low->blue) && (pixel.blue <= high->blue)) ?
        MagickTrue : MagickFalse;
      if (match != invert)
        q->opacity=opacity;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_TransparentPaintImageChroma)
#endif
        proceed=SetImageProgress(image,TransparentPaintImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

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
%                                   Cristy                                    %
%                                 July 1998                                   %
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
*/

/*
 Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/channel.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/draw-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/gem-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/resource_.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"

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
%  By default target must match a particular pixel color exactly.  However,
%  in many cases two colors may differ by a small amount.  The fuzz member of
%  image defines how much tolerance is acceptable to consider two colors as
%  the same.  For example, set fuzz to 10 and the color red at intensities of
%  100 and 102 respectively are now interpreted as the same color for the
%  purposes of the floodfill.
%
%  The format of the FloodfillPaintImage method is:
%
%      MagickBooleanType FloodfillPaintImage(Image *image,
%        const DrawInfo *draw_info,const PixelInfo target,
%        const ssize_t x_offset,const ssize_t y_offset,
%        const MagickBooleanType invert,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o draw_info: the draw info.
%
%    o target: the RGB value of the target color.
%
%    o x_offset,y_offset: the starting location of the operation.
%
%    o invert: paint any pixel that does not match the target color.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType FloodfillPaintImage(Image *image,
  const DrawInfo *draw_info,const PixelInfo *target,const ssize_t x_offset,
  const ssize_t y_offset,const MagickBooleanType invert,
  ExceptionInfo *exception)
{
#define MaxStacksize  262144UL
#define PushSegmentStack(up,left,right,delta) \
{ \
  if (s >= (segment_stack+MaxStacksize)) \
    ThrowBinaryException(DrawError,"SegmentStackOverflow",image->filename) \
  else \
    { \
      if ((((up)+(delta)) >= 0) && (((up)+(delta)) < (ssize_t) image->rows)) \
        { \
          s->x1=(double) (left); \
          s->y1=(double) (up); \
          s->x2=(double) (right); \
          s->y2=(double) (delta); \
          s++; \
        } \
    } \
}

  CacheView
    *floodplane_view,
    *image_view;

  Image
    *floodplane_image;

  MagickBooleanType
    skip,
    status;

  MemoryInfo
    *segment_info;

  PixelInfo
    fill_color,
    pixel;

  register SegmentInfo
    *s;

  SegmentInfo
    *segment_stack;

  ssize_t
    offset,
    start,
    x,
    x1,
    x2,
    y;

  /*
    Check boundary conditions.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(draw_info != (DrawInfo *) NULL);
  assert(draw_info->signature == MagickSignature);
  if ((x_offset < 0) || (x_offset >= (ssize_t) image->columns))
    return(MagickFalse);
  if ((y_offset < 0) || (y_offset >= (ssize_t) image->rows))
    return(MagickFalse);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  if ((image->alpha_trait == UndefinedPixelTrait) &&
      (draw_info->fill.alpha_trait != UndefinedPixelTrait))
    (void) SetImageAlpha(image,OpaqueAlpha,exception);
  /*
    Set floodfill state.
  */
  floodplane_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (floodplane_image == (Image *) NULL)
    return(MagickFalse);
  floodplane_image->alpha_trait=UndefinedPixelTrait;
  floodplane_image->colorspace=GRAYColorspace;
  (void) QueryColorCompliance("#000",AllCompliance,
    &floodplane_image->background_color,exception);
  (void) SetImageBackgroundColor(floodplane_image,exception);
  segment_info=AcquireVirtualMemory(MaxStacksize,sizeof(*segment_stack));
  if (segment_info == (MemoryInfo *) NULL)
    {
      floodplane_image=DestroyImage(floodplane_image);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  segment_stack=(SegmentInfo *) GetVirtualMemoryBlob(segment_info);
  /*
    Push initial segment on stack.
  */
  status=MagickTrue;
  x=x_offset;
  y=y_offset;
  start=0;
  s=segment_stack;
  PushSegmentStack(y,x,x,1);
  PushSegmentStack(y+1,x,x,-1);
  GetPixelInfo(image,&pixel);
  image_view=AcquireVirtualCacheView(image,exception);
  floodplane_view=AcquireAuthenticCacheView(floodplane_image,exception);
  while (s > segment_stack)
  {
    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    /*
      Pop segment off stack.
    */
    s--;
    x1=(ssize_t) s->x1;
    x2=(ssize_t) s->x2;
    offset=(ssize_t) s->y2;
    y=(ssize_t) s->y1+offset;
    /*
      Recolor neighboring pixels.
    */
    p=GetCacheViewVirtualPixels(image_view,0,y,(size_t) (x1+1),1,exception);
    q=GetCacheViewAuthenticPixels(floodplane_view,0,y,(size_t) (x1+1),1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      break;
    p+=x1*GetPixelChannels(image);
    q+=x1*GetPixelChannels(floodplane_image);
    for (x=x1; x >= 0; x--)
    {
      if (GetPixelGray(floodplane_image,q) != 0)
        break;
      GetPixelInfoPixel(image,p,&pixel);
      if (IsFuzzyEquivalencePixelInfo(&pixel,target) == invert)
        break;
      SetPixelGray(floodplane_image,QuantumRange,q);
      p-=GetPixelChannels(image);
      q-=GetPixelChannels(floodplane_image);
    }
    if (SyncCacheViewAuthenticPixels(floodplane_view,exception) == MagickFalse)
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
          if (x < (ssize_t) image->columns)
            {
              p=GetCacheViewVirtualPixels(image_view,x,y,image->columns-x,1,
                exception);
              q=GetCacheViewAuthenticPixels(floodplane_view,x,y,image->columns-
                x,1,exception);
              if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
                break;
              for ( ; x < (ssize_t) image->columns; x++)
              {
                if (GetPixelGray(floodplane_image,q) != 0)
                  break;
                GetPixelInfoPixel(image,p,&pixel);
                if (IsFuzzyEquivalencePixelInfo(&pixel,target) == invert)
                  break;
                SetPixelGray(floodplane_image,QuantumRange,q);
                p+=GetPixelChannels(image);
                q+=GetPixelChannels(floodplane_image);
              }
              status=SyncCacheViewAuthenticPixels(floodplane_view,exception);
              if (status == MagickFalse)
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
          p=GetCacheViewVirtualPixels(image_view,x,y,(size_t) (x2-x+1),1,
            exception);
          q=GetCacheViewAuthenticPixels(floodplane_view,x,y,(size_t) (x2-x+1),1,
            exception);
          if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
            break;
          for ( ; x <= x2; x++)
          {
            if (GetPixelGray(floodplane_image,q) != 0)
              break;
            GetPixelInfoPixel(image,p,&pixel);
            if (IsFuzzyEquivalencePixelInfo(&pixel,target) != invert)
              break;
            p+=GetPixelChannels(image);
            q+=GetPixelChannels(floodplane_image);
          }
        }
      start=x;
    } while (x <= x2);
  }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    /*
      Tile fill color onto floodplane.
    */
    p=GetCacheViewVirtualPixels(floodplane_view,0,y,image->columns,1,exception);
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelGray(floodplane_image,p) != 0)
        {
          (void) GetFillColor(draw_info,x,y,&fill_color,exception);
          SetPixelInfoPixel(image,&fill_color,q);
        }
      p+=GetPixelChannels(floodplane_image);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      break;
  }
  floodplane_view=DestroyCacheView(floodplane_view);
  image_view=DestroyCacheView(image_view);
  segment_info=RelinquishVirtualMemory(segment_info);
  floodplane_image=DestroyImage(floodplane_image);
  return(y == (ssize_t) image->rows ? MagickTrue : MagickFalse);
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
%        const SpreadMethod method,const PixelInfo *start_color,
%        const PixelInfo *stop_color,ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/

static inline double MagickMax(const double x,const double y)
{
  return(x > y ? x : y);
}

MagickExport MagickBooleanType GradientImage(Image *image,
  const GradientType type,const SpreadMethod method,
  const PixelInfo *start_color,const PixelInfo *stop_color,
  ExceptionInfo *exception)
{
  DrawInfo
    *draw_info;

  GradientInfo
    *gradient;

  MagickBooleanType
    status;

  register ssize_t
    i;

  /*
    Set gradient start-stop end points.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(start_color != (const PixelInfo *) NULL);
  assert(stop_color != (const PixelInfo *) NULL);
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
  for (i=0; i < (ssize_t) gradient->number_stops; i++)
    GetPixelInfo(image,&gradient->stops[i].color);
  gradient->stops[0].color=(*start_color);
  gradient->stops[0].offset=0.0;
  gradient->stops[1].color=(*stop_color);
  gradient->stops[1].offset=1.0;
  /*
    Draw a gradient on the image.
  */
  (void) SetImageColorspace(image,start_color->colorspace,exception);
  status=DrawGradientImage(image,draw_info,exception);
  draw_info=DestroyDrawInfo(draw_info);
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
%        const double sigma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the circular neighborhood.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static size_t **DestroyHistogramThreadSet(size_t **histogram)
{
  register ssize_t
    i;

  assert(histogram != (size_t **) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (histogram[i] != (size_t *) NULL)
      histogram[i]=(size_t *) RelinquishMagickMemory(histogram[i]);
  histogram=(size_t **) RelinquishMagickMemory(histogram);
  return(histogram);
}

static size_t **AcquireHistogramThreadSet(const size_t count)
{
  register ssize_t
    i;

  size_t
    **histogram,
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  histogram=(size_t **) AcquireQuantumMemory(number_threads,sizeof(*histogram));
  if (histogram == (size_t **) NULL)
    return((size_t **) NULL);
  (void) ResetMagickMemory(histogram,0,number_threads*sizeof(*histogram));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    histogram[i]=(size_t *) AcquireQuantumMemory(count,sizeof(**histogram));
    if (histogram[i] == (size_t *) NULL)
      return(DestroyHistogramThreadSet(histogram));
  }
  return(histogram);
}

MagickExport Image *OilPaintImage(const Image *image,const double radius,
  const double sigma,ExceptionInfo *exception)
{
#define NumberPaintBins  256
#define OilPaintImageTag  "OilPaint/Image"

  CacheView
    *image_view,
    *paint_view;

  Image
    *linear_image,
    *paint_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  size_t
    **histograms,
    width;

  ssize_t
    center,
    y;

  /*
    Initialize painted image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,sigma);
  linear_image=CloneImage(image,0,0,MagickTrue,exception);
  paint_image=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  if ((linear_image == (Image *) NULL) || (paint_image == (Image *) NULL))
    {
      if (linear_image != (Image *) NULL)
        linear_image=DestroyImage(linear_image);
      if (paint_image != (Image *) NULL)
        linear_image=DestroyImage(paint_image);
      return((Image *) NULL);
    }
  if (SetImageStorageClass(paint_image,DirectClass,exception) == MagickFalse)
    {
      linear_image=DestroyImage(linear_image);
      paint_image=DestroyImage(paint_image);
      return((Image *) NULL);
    }
  histograms=AcquireHistogramThreadSet(NumberPaintBins);
  if (histograms == (size_t **) NULL)
    {
      linear_image=DestroyImage(linear_image);
      paint_image=DestroyImage(paint_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Oil paint image.
  */
  status=MagickTrue;
  progress=0;
  center=(ssize_t) GetPixelChannels(linear_image)*(linear_image->columns+width)*
    (width/2L)+GetPixelChannels(linear_image)*(width/2L);
  image_view=AcquireVirtualCacheView(linear_image,exception);
  paint_view=AcquireAuthenticCacheView(paint_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(linear_image,paint_image,linear_image->rows,1)
#endif
  for (y=0; y < (ssize_t) linear_image->rows; y++)
  {
    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register size_t
      *histogram;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,-((ssize_t) width/2L),y-(ssize_t)
      (width/2L),linear_image->columns+width,width,exception);
    q=QueueCacheViewAuthenticPixels(paint_view,0,y,paint_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    histogram=histograms[GetOpenMPThreadId()];
    for (x=0; x < (ssize_t) linear_image->columns; x++)
    {
      register ssize_t
        i,
        u;

      size_t
        count;

      ssize_t
        j,
        k,
        n,
        v;

      /*
        Assign most frequent color.
      */
      k=0;
      j=0;
      count=0;
      (void) ResetMagickMemory(histogram,0,NumberPaintBins* sizeof(*histogram));
      for (v=0; v < (ssize_t) width; v++)
      {
        for (u=0; u < (ssize_t) width; u++)
        {
          n=(ssize_t) ScaleQuantumToChar(ClampToQuantum(GetPixelIntensity(
            linear_image,p+GetPixelChannels(linear_image)*(u+k))));
          histogram[n]++;
          if (histogram[n] > count)
            {
              j=k+u;
              count=histogram[n];
            }
        }
        k+=(ssize_t) (linear_image->columns+width);
      }
      for (i=0; i < (ssize_t) GetPixelChannels(linear_image); i++)
      {
        PixelChannel channel=GetPixelChannelChannel(linear_image,i);
        PixelTrait traits=GetPixelChannelTraits(linear_image,channel);
        PixelTrait paint_traits=GetPixelChannelTraits(paint_image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (paint_traits == UndefinedPixelTrait))
          continue;
        if (((paint_traits & CopyPixelTrait) != 0) ||
            (GetPixelReadMask(linear_image,p) == 0))
          {
            SetPixelChannel(paint_image,channel,p[center+i],q);
            continue;
          }
        SetPixelChannel(paint_image,channel,p[j*GetPixelChannels(linear_image)+
          i],q);
      }
      p+=GetPixelChannels(linear_image);
      q+=GetPixelChannels(paint_image);
    }
    if (SyncCacheViewAuthenticPixels(paint_view,exception) == MagickFalse)
      status=MagickFalse;
    if (linear_image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_OilPaintImage)
#endif
        proceed=SetImageProgress(linear_image,OilPaintImageTag,progress++,
          linear_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  paint_view=DestroyCacheView(paint_view);
  image_view=DestroyCacheView(image_view);
  histograms=DestroyHistogramThreadSet(histograms);
  linear_image=DestroyImage(linear_image);
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
%  defined by fill argument.
%
%  By default color must match a particular pixel color exactly.  However, in
%  many cases two colors may differ by a small amount.  Fuzz defines how much
%  tolerance is acceptable to consider two colors as the same.  For example,
%  set fuzz to 10 and the color red at intensities of 100 and 102 respectively
%  are now interpreted as the same color.
%
%  The format of the OpaquePaintImage method is:
%
%      MagickBooleanType OpaquePaintImage(Image *image,const PixelInfo *target,
%        const PixelInfo *fill,const MagickBooleanType invert,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o target: the RGB value of the target color.
%
%    o fill: the replacement color.
%
%    o invert: paint any pixel that does not match the target color.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType OpaquePaintImage(Image *image,
  const PixelInfo *target,const PixelInfo *fill,const MagickBooleanType invert,
  ExceptionInfo *exception)
{
#define OpaquePaintImageTag  "Opaque/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    conform_fill,
    conform_target,
    zero;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(target != (PixelInfo *) NULL);
  assert(fill != (PixelInfo *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  ConformPixelInfo(image,fill,&conform_fill,exception);
  ConformPixelInfo(image,target,&conform_target,exception);
  /*
    Make image color opaque.
  */
  status=MagickTrue;
  progress=0;
  GetPixelInfo(image,&zero);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    PixelInfo
      pixel;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    pixel=zero;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      GetPixelInfoPixel(image,q,&pixel);
      if (IsFuzzyEquivalencePixelInfo(&pixel,&conform_target) != invert)
        SetPixelInfoPixel(image,&conform_fill,q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_OpaquePaintImage)
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
%  By default color must match a particular pixel color exactly.  However, in
%  many cases two colors may differ by a small amount.  Fuzz defines how much
%  tolerance is acceptable to consider two colors as the same.  For example,
%  set fuzz to 10 and the color red at intensities of 100 and 102 respectively
%  are now interpreted as the same color.
%
%  The format of the TransparentPaintImage method is:
%
%      MagickBooleanType TransparentPaintImage(Image *image,
%        const PixelInfo *target,const Quantum opacity,
%        const MagickBooleanType invert,ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType TransparentPaintImage(Image *image,
  const PixelInfo *target,const Quantum opacity,const MagickBooleanType invert,
  ExceptionInfo *exception)
{
#define TransparentPaintImageTag  "Transparent/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  PixelInfo
    zero;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(target != (PixelInfo *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if (image->alpha_trait == UndefinedPixelTrait)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
  /*
    Make image color transparent.
  */
  status=MagickTrue;
  progress=0;
  GetPixelInfo(image,&zero);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    PixelInfo
      pixel;

    register ssize_t
      x;

    register Quantum
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    pixel=zero;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      GetPixelInfoPixel(image,q,&pixel);
      if (IsFuzzyEquivalencePixelInfo(&pixel,target) != invert)
        SetPixelAlpha(image,opacity,q);
      q+=GetPixelChannels(image);
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
%  As there is one fuzz value for the all the channels, TransparentPaintImage()
%  is not suitable for the operations like chroma, where the tolerance for
%  similarity of two color component (RGB) can be different. Thus we define
%  this method to take two target pixels (one low and one high) and all the
%  pixels of an image which are lying between these two pixels are made
%  transparent.
%
%  The format of the TransparentPaintImageChroma method is:
%
%      MagickBooleanType TransparentPaintImageChroma(Image *image,
%        const PixelInfo *low,const PixelInfo *high,const Quantum opacity,
%        const MagickBooleanType invert,ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType TransparentPaintImageChroma(Image *image,
  const PixelInfo *low,const PixelInfo *high,const Quantum opacity,
  const MagickBooleanType invert,ExceptionInfo *exception)
{
#define TransparentPaintImageTag  "Transparent/Image"

  CacheView
    *image_view;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(high != (PixelInfo *) NULL);
  assert(low != (PixelInfo *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
    return(MagickFalse);
  if (image->alpha_trait == UndefinedPixelTrait)
    (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
  /*
    Make image color transparent.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    magick_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    MagickBooleanType
      match;

    PixelInfo
      pixel;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    GetPixelInfo(image,&pixel);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      GetPixelInfoPixel(image,q,&pixel);
      match=((pixel.red >= low->red) && (pixel.red <= high->red) &&
        (pixel.green >= low->green) && (pixel.green <= high->green) &&
        (pixel.blue  >= low->blue) && (pixel.blue <= high->blue)) ? MagickTrue :
        MagickFalse;
      if (match != invert)
        SetPixelAlpha(image,opacity,q);
      q+=GetPixelChannels(image);
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

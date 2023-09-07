/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%         AAA   TTTTT  TTTTT  RRRR   IIIII  BBBB   U   U  TTTTT  EEEEE        %
%        A   A    T      T    R   R    I    B   B  U   U    T    E            %
%        AAAAA    T      T    RRRR     I    BBBB   U   U    T    EEE          %
%        A   A    T      T    R R      I    B   B  U   U    T    E            %
%        A   A    T      T    R  R   IIIII  BBBB    UUU     T    EEEEE        %
%                                                                             %
%                                                                             %
%                    MagickCore Get / Set Image Attributes                    %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                October 2002                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 2002 ImageMagick Studio LLC, a non-profit organization         %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/channel.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/draw-private.h"
#include "MagickCore/effect.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/histogram.h"
#include "MagickCore/identify.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/memory_.h"
#include "MagickCore/magick.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/segment.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/threshold.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e B o u n d i n g B o x                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageBoundingBox() returns the bounding box of an image canvas.
%
%  The format of the GetImageBoundingBox method is:
%
%      RectangleInfo GetImageBoundingBox(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o bounds: Method GetImageBoundingBox returns the bounding box of an
%      image canvas.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _EdgeInfo
{
  double
    left,
    right,
    top,
    bottom;
} EdgeInfo;

static double GetEdgeBackgroundCensus(const Image *image,
  const CacheView *image_view,const GravityType gravity,const size_t width,
  const size_t height,const ssize_t x_offset,const ssize_t y_offset,
  ExceptionInfo *exception)
{
  CacheView
    *edge_view;

  const char
    *artifact;

  const Quantum
    *p;

  double
    census;

  Image
    *edge_image;

  PixelInfo
    background,
    pixel;

  RectangleInfo
    edge_geometry;

  ssize_t
    y;

  /*
    Determine the percent of image background for this edge.
  */
  switch (gravity)
  {
    case NorthWestGravity:
    case NorthGravity:
    default:
    {
      p=GetCacheViewVirtualPixels(image_view,0,0,1,1,exception);
      break;
    }
    case NorthEastGravity:
    case EastGravity:
    {
      p=GetCacheViewVirtualPixels(image_view,(ssize_t) image->columns-1,0,1,1,
        exception);
      break;
    }
    case SouthEastGravity:
    case SouthGravity:
    {
      p=GetCacheViewVirtualPixels(image_view,(ssize_t) image->columns-1,
        (ssize_t) image->rows-1,1,1,exception);
      break;
    }
    case SouthWestGravity:
    case WestGravity:
    {
      p=GetCacheViewVirtualPixels(image_view,0,(ssize_t) image->rows-1,1,1,
        exception);
      break;
    }
  }
  if (p == (const Quantum *) NULL)
    return(0.0);
  GetPixelInfoPixel(image,p,&background);
  artifact=GetImageArtifact(image,"background");
  if (artifact != (const char *) NULL)
    (void) QueryColorCompliance(artifact,AllCompliance,&background,exception);
  artifact=GetImageArtifact(image,"trim:background-color");
  if (artifact != (const char *) NULL)
    (void) QueryColorCompliance(artifact,AllCompliance,&background,exception);
  edge_geometry.width=width;
  edge_geometry.height=height;
  edge_geometry.x=x_offset;
  edge_geometry.y=y_offset;
  GravityAdjustGeometry(image->columns,image->rows,gravity,&edge_geometry);
  edge_image=CropImage(image,&edge_geometry,exception);
  if (edge_image == (Image *) NULL)
    return(0.0);
  census=0.0;
  edge_view=AcquireVirtualCacheView(edge_image,exception);
  for (y=0; y < (ssize_t) edge_image->rows; y++)
  {
    ssize_t
      x;

    p=GetCacheViewVirtualPixels(edge_view,0,y,edge_image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) edge_image->columns; x++)
    {
      GetPixelInfoPixel(edge_image,p,&pixel);
      if (IsFuzzyEquivalencePixelInfo(&pixel,&background) == MagickFalse)
        census++;
      p+=GetPixelChannels(edge_image);
    }
  }
  census/=((double) edge_image->columns*edge_image->rows);
  edge_view=DestroyCacheView(edge_view);
  edge_image=DestroyImage(edge_image);
  return(census);
}

static inline double GetMinEdgeBackgroundCensus(const EdgeInfo *edge)
{
  double
    census;

  census=MagickMin(MagickMin(MagickMin(edge->left,edge->right),edge->top),
    edge->bottom);
  return(census);
}

static RectangleInfo GetEdgeBoundingBox(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *edge_view;

  const char
    *artifact;

  double
    background_census,
    percent_background;

  EdgeInfo
    edge,
    vertex;

  Image
    *edge_image;

  RectangleInfo
    bounds;

  /*
    Get the image bounding box.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  SetGeometry(image,&bounds);
  edge_image=CloneImage(image,0,0,MagickTrue,exception);
  if (edge_image == (Image *) NULL)
    return(bounds);
  (void) ParseAbsoluteGeometry("0x0+0+0",&edge_image->page);
  (void) memset(&vertex,0,sizeof(vertex));
  edge_view=AcquireVirtualCacheView(edge_image,exception);
  edge.left=GetEdgeBackgroundCensus(edge_image,edge_view,WestGravity,
    1,0,0,0,exception);
  edge.right=GetEdgeBackgroundCensus(edge_image,edge_view,EastGravity,
    1,0,0,0,exception);
  edge.top=GetEdgeBackgroundCensus(edge_image,edge_view,NorthGravity,
    0,1,0,0,exception);
  edge.bottom=GetEdgeBackgroundCensus(edge_image,edge_view,SouthGravity,
    0,1,0,0,exception);
  percent_background=1.0;
  artifact=GetImageArtifact(edge_image,"trim:percent-background");
  if (artifact != (const char *) NULL)
    percent_background=StringToDouble(artifact,(char **) NULL)/100.0;
  percent_background=MagickMin(MagickMax(1.0-percent_background,MagickEpsilon),
    1.0);
  background_census=GetMinEdgeBackgroundCensus(&edge);
  for ( ; background_census < percent_background;
          background_census=GetMinEdgeBackgroundCensus(&edge))
  {
    if ((bounds.width == 0) || (bounds.height == 0))
      break;
    if (fabs(edge.left-background_census) < MagickEpsilon)
      {
        /*
          Trim left edge.
        */
        vertex.left++;
        bounds.width--;
        edge.left=GetEdgeBackgroundCensus(edge_image,edge_view,
          NorthWestGravity,1,bounds.height,(ssize_t) vertex.left,(ssize_t)
          vertex.top,exception);
        edge.top=GetEdgeBackgroundCensus(edge_image,edge_view,
          NorthWestGravity,bounds.width,1,(ssize_t) vertex.left,(ssize_t)
          vertex.top,exception);
        edge.bottom=GetEdgeBackgroundCensus(edge_image,edge_view,
          SouthWestGravity,bounds.width,1,(ssize_t) vertex.left,(ssize_t)
          vertex.bottom,exception);
        continue;
      }
    if (fabs(edge.right-background_census) < MagickEpsilon)
      {
        /*
          Trim right edge.
        */
        vertex.right++;
        bounds.width--;
        edge.right=GetEdgeBackgroundCensus(edge_image,edge_view,
          NorthEastGravity,1,bounds.height,(ssize_t) vertex.right,(ssize_t)
          vertex.top,exception);
        edge.top=GetEdgeBackgroundCensus(edge_image,edge_view,
          NorthWestGravity,bounds.width,1,(ssize_t) vertex.left,(ssize_t)
          vertex.top,exception);
        edge.bottom=GetEdgeBackgroundCensus(edge_image,edge_view,
          SouthWestGravity,bounds.width,1,(ssize_t) vertex.left,(ssize_t)
          vertex.bottom,exception);
        continue;
      }
    if (fabs(edge.top-background_census) < MagickEpsilon)
      {
        /*
          Trim top edge.
        */
        vertex.top++;
        bounds.height--;
        edge.left=GetEdgeBackgroundCensus(edge_image,edge_view,
          NorthWestGravity,1,bounds.height,(ssize_t) vertex.left,(ssize_t)
          vertex.top,exception);
        edge.right=GetEdgeBackgroundCensus(edge_image,edge_view,
          NorthEastGravity,1,bounds.height,(ssize_t) vertex.right,(ssize_t)
          vertex.top,exception);
        edge.top=GetEdgeBackgroundCensus(edge_image,edge_view,
          NorthWestGravity,bounds.width,1,(ssize_t) vertex.left,(ssize_t)
          vertex.top,exception);
        continue;
      }
    if (fabs(edge.bottom-background_census) < MagickEpsilon)
      {
        /*
          Trim bottom edge.
        */
        vertex.bottom++;
        bounds.height--;
        edge.left=GetEdgeBackgroundCensus(edge_image,edge_view,
          NorthWestGravity,1,bounds.height,(ssize_t) vertex.left,(ssize_t)
          vertex.top,exception);
        edge.right=GetEdgeBackgroundCensus(edge_image,edge_view,
          NorthEastGravity,1,bounds.height,(ssize_t) vertex.right,(ssize_t)
          vertex.top,exception);
        edge.bottom=GetEdgeBackgroundCensus(edge_image,edge_view,
          SouthWestGravity,bounds.width,1,(ssize_t) vertex.left,(ssize_t)
          vertex.bottom,exception);
        continue;
      }
  }
  edge_view=DestroyCacheView(edge_view);
  edge_image=DestroyImage(edge_image);
  bounds.x=(ssize_t) vertex.left;
  bounds.y=(ssize_t) vertex.top;
  if ((bounds.width == 0) || (bounds.height == 0))
    (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
      "GeometryDoesNotContainImage","`%s'",image->filename);
  return(bounds);
}

MagickExport RectangleInfo GetImageBoundingBox(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  const char
    *artifact;

  const Quantum
    *p;

  MagickBooleanType
    status;

  PixelInfo
    target[4],
    zero;

  RectangleInfo
    bounds;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  artifact=GetImageArtifact(image,"trim:percent-background");
  if (artifact != (const char *) NULL)
    return(GetEdgeBoundingBox(image,exception));
  artifact=GetImageArtifact(image,"trim:edges");
  if (artifact == (const char *) NULL)
    {
      bounds.width=(size_t) (image->columns == 1 ? 1 : 0);
      bounds.height=(size_t) (image->rows == 1 ? 1 : 0);
      bounds.x=(ssize_t) image->columns;
      bounds.y=(ssize_t) image->rows;
    }
  else
    {
      char
        *edges,
        *q,
        *r;

      bounds.width=(size_t) image->columns;
      bounds.height=(size_t) image->rows;
      bounds.x=0;
      bounds.y=0;
      edges=AcquireString(artifact);
      r=edges;
      while ((q=StringToken(",",&r)) != (char *) NULL)
      {
        if (LocaleCompare(q,"north") == 0)
          bounds.y=(ssize_t) image->rows;
        if (LocaleCompare(q,"east") == 0)
          bounds.width=0;
        if (LocaleCompare(q,"south") == 0)
          bounds.height=0;
        if (LocaleCompare(q,"west") == 0)
          bounds.x=(ssize_t) image->columns;
      }
      edges=DestroyString(edges);
    }
  GetPixelInfo(image,&target[0]);
  image_view=AcquireVirtualCacheView(image,exception);
  p=GetCacheViewVirtualPixels(image_view,0,0,1,1,exception);
  if (p == (const Quantum *) NULL)
    {
      image_view=DestroyCacheView(image_view);
      return(bounds);
    }
  GetPixelInfoPixel(image,p,&target[0]);
  GetPixelInfo(image,&target[1]);
  p=GetCacheViewVirtualPixels(image_view,(ssize_t) image->columns-1,0,1,1,
    exception);
  if (p != (const Quantum *) NULL)
    GetPixelInfoPixel(image,p,&target[1]);
  GetPixelInfo(image,&target[2]);
  p=GetCacheViewVirtualPixels(image_view,0,(ssize_t) image->rows-1,1,1,
    exception);
  if (p != (const Quantum *) NULL)
    GetPixelInfoPixel(image,p,&target[2]);
  GetPixelInfo(image,&target[3]);
  p=GetCacheViewVirtualPixels(image_view,(ssize_t) image->columns-1,(ssize_t)
    image->rows-1,1,1,exception);
  if (p != (const Quantum *) NULL)
    GetPixelInfoPixel(image,p,&target[3]);
  status=MagickTrue;
  GetPixelInfo(image,&zero);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict q;

    PixelInfo
      pixel;

    RectangleInfo
      bounding_box;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
#  pragma omp critical (MagickCore_GetImageBoundingBox)
#endif
    bounding_box=bounds;
    q=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (q == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    pixel=zero;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      GetPixelInfoPixel(image,q,&pixel);
      if ((x < bounding_box.x) &&
          (IsFuzzyEquivalencePixelInfo(&pixel,&target[0]) == MagickFalse))
        bounding_box.x=x;
      if ((x > (ssize_t) bounding_box.width) &&
          (IsFuzzyEquivalencePixelInfo(&pixel,&target[1]) == MagickFalse))
        bounding_box.width=(size_t) x;
      if ((y < bounding_box.y) &&
          (IsFuzzyEquivalencePixelInfo(&pixel,&target[0]) == MagickFalse))
        bounding_box.y=y;
      if ((y > (ssize_t) bounding_box.height) &&
          (IsFuzzyEquivalencePixelInfo(&pixel,&target[2]) == MagickFalse))
        bounding_box.height=(size_t) y;
      if ((x < (ssize_t) bounding_box.width) &&
          (y > (ssize_t) bounding_box.height) &&
          (IsFuzzyEquivalencePixelInfo(&pixel,&target[3]) == MagickFalse))
        {
          bounding_box.width=(size_t) x;
          bounding_box.height=(size_t) y;
        }
      q+=GetPixelChannels(image);
    }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
#  pragma omp critical (MagickCore_GetImageBoundingBox)
#endif
    {
      if (bounding_box.x < bounds.x)
        bounds.x=bounding_box.x;
      if (bounding_box.y < bounds.y)
        bounds.y=bounding_box.y;
      if (bounding_box.width > bounds.width)
        bounds.width=bounding_box.width;
      if (bounding_box.height > bounds.height)
        bounds.height=bounding_box.height;
    }
  }
  image_view=DestroyCacheView(image_view);
  if ((bounds.width == 0) || (bounds.height == 0))
    (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
      "GeometryDoesNotContainImage","`%s'",image->filename);
  else
    {
      bounds.width-=(size_t) (bounds.x-1);
      bounds.height-=(size_t) (bounds.y-1);
    }
  return(bounds);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C o n v e x H u l l                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageConvexHull() returns the convex hull points of an image canvas.
%
%  The format of the GetImageConvexHull method is:
%
%      PointInfo *GetImageConvexHull(const Image *image,
%        size_t number_vertices,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o number_vertices: the number of vertices in the convex hull.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static double LexicographicalOrder(PointInfo *a,PointInfo *b,PointInfo *c)
{
  /*
    Order by x-coordinate, and in case of a tie, by y-coordinate.
  */
  return((b->x-a->x)*(c->y-a->y)-(b->y-a->y)*(c->x-a->x));
}

static PixelInfo GetEdgeBackgroundColor(const Image *image,
  const CacheView *image_view,ExceptionInfo *exception)
{
  const char
    *artifact;

  double
    census[4],
    edge_census;

  PixelInfo
    background[4],
    edge_background;

  ssize_t
    i;

  /*
    Most dominant color of edges/corners is the background color of the image.
  */
  memset(&edge_background,0,sizeof(edge_background));
  artifact=GetImageArtifact(image,"convex-hull:background-color");
  if (artifact == (const char *) NULL)
    artifact=GetImageArtifact(image,"background");
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static)
#endif
  for (i=0; i < 4; i++)
  {
    CacheView
      *edge_view;

    GravityType
      gravity;

    Image
      *edge_image;

    PixelInfo
      pixel;

    RectangleInfo
      edge_geometry;

    const Quantum
      *p;

    ssize_t
      y;

    census[i]=0.0;
    (void) memset(&edge_geometry,0,sizeof(edge_geometry));
    switch (i)
    {
      case 0:
      default:
      {
        p=GetCacheViewVirtualPixels(image_view,0,(ssize_t) image->rows-1,1,1,
          exception);
        gravity=WestGravity;
        edge_geometry.width=1;
        edge_geometry.height=0;
        break;
      }
      case 1:
      {
        p=GetCacheViewVirtualPixels(image_view,(ssize_t) image->columns-1,0,1,1,
          exception);
        gravity=EastGravity;
        edge_geometry.width=1;
        edge_geometry.height=0;
        break;
      }
      case 2:
      {
        p=GetCacheViewVirtualPixels(image_view,0,0,1,1,exception);
        gravity=NorthGravity;
        edge_geometry.width=0;
        edge_geometry.height=1;
        break;
      }
      case 3:
      {
        p=GetCacheViewVirtualPixels(image_view,(ssize_t) image->columns-1,
          (ssize_t) image->rows-1,1,1,exception);
        gravity=SouthGravity;
        edge_geometry.width=0;
        edge_geometry.height=1;
        break;
      }
    }
    GetPixelInfoPixel(image,p,background+i);
    if (artifact != (const char *) NULL)
      (void) QueryColorCompliance(artifact,AllCompliance,background+i,
        exception);
    GravityAdjustGeometry(image->columns,image->rows,gravity,&edge_geometry);
    edge_image=CropImage(image,&edge_geometry,exception);
    if (edge_image == (Image *) NULL)
      continue;
    edge_view=AcquireVirtualCacheView(edge_image,exception);
    for (y=0; y < (ssize_t) edge_image->rows; y++)
    {
      ssize_t
        x;

      p=GetCacheViewVirtualPixels(edge_view,0,y,edge_image->columns,1,
        exception);
      if (p == (const Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) edge_image->columns; x++)
      {
        GetPixelInfoPixel(edge_image,p,&pixel);
        if (IsFuzzyEquivalencePixelInfo(&pixel,background+i) == MagickFalse)
          census[i]++;
        p+=GetPixelChannels(edge_image);
      }
    }
    edge_view=DestroyCacheView(edge_view);
    edge_image=DestroyImage(edge_image);
  }
  edge_census=(-1.0);
  for (i=0; i < 4; i++)
    if (census[i] > edge_census)
      {
        edge_background=background[i];
        edge_census=census[i];
      }
  return(edge_background);
}

void TraceConvexHull(PointInfo *vertices,size_t number_vertices,
  PointInfo ***monotone_chain,size_t *chain_length)
{
  PointInfo
    **chain;

  size_t
    demark,
    n;

  ssize_t
    i;

  /*
    Construct the upper and lower hulls: rightmost to leftmost counterclockwise.
  */
  chain=(*monotone_chain);
  n=0;
  for (i=0; i < (ssize_t) number_vertices; i++)
  {
    while ((n >= 2) &&
           (LexicographicalOrder(chain[n-2],chain[n-1],&vertices[i]) <= 0.0))
      n--;
    chain[n++]=(&vertices[i]);
  }
  demark=n+1;
  for (i=(ssize_t) number_vertices-2; i >= 0; i--)
  {
    while ((n >= demark) &&
           (LexicographicalOrder(chain[n-2],chain[n-1],&vertices[i]) <= 0.0))
      n--;
    chain[n++]=(&vertices[i]);
  }
  *chain_length=n;
}

MagickExport PointInfo *GetImageConvexHull(const Image *image,
  size_t *number_vertices,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  MemoryInfo
    *monotone_info,
    *vertices_info;

  PixelInfo
    background;

  PointInfo
    *convex_hull,
    **monotone_chain,
    *vertices;

  size_t
    n;

  ssize_t
    y;

  /*
    Identify convex hull vertices of image foreground object(s).
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *number_vertices=0;
  vertices_info=AcquireVirtualMemory(image->columns,image->rows*
    sizeof(*vertices));
  monotone_info=AcquireVirtualMemory(2*image->columns,2*
    image->rows*sizeof(*monotone_chain));
  if ((vertices_info == (MemoryInfo *) NULL) ||
      (monotone_info == (MemoryInfo *) NULL))
    {
      if (monotone_info != (MemoryInfo *) NULL)
        monotone_info=(MemoryInfo *) RelinquishVirtualMemory(monotone_info);
      if (vertices_info != (MemoryInfo *) NULL)
        vertices_info=RelinquishVirtualMemory(vertices_info);
      return((PointInfo *) NULL);
    }
  vertices=(PointInfo *) GetVirtualMemoryBlob(vertices_info);
  monotone_chain=(PointInfo **) GetVirtualMemoryBlob(monotone_info);
  image_view=AcquireVirtualCacheView(image,exception);
  background=GetEdgeBackgroundColor(image,image_view,exception);
  status=MagickTrue;
  n=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *p;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      PixelInfo
        pixel;

      GetPixelInfoPixel(image,p,&pixel);
      if (IsFuzzyEquivalencePixelInfo(&pixel,&background) == MagickFalse)
        {
          vertices[n].x=(double) x;
          vertices[n].y=(double) y;
          n++;
        }
      p+=GetPixelChannels(image);
    }
  }
  image_view=DestroyCacheView(image_view);
  /*
    Return the convex hull of the image foreground object(s).
  */
  TraceConvexHull(vertices,n,&monotone_chain,number_vertices);
  convex_hull=(PointInfo *) AcquireQuantumMemory(*number_vertices,
    sizeof(*convex_hull));
  if (convex_hull != (PointInfo *) NULL)
    for (n=0; n < *number_vertices; n++)
      convex_hull[n]=(*monotone_chain[n]);
  monotone_info=RelinquishVirtualMemory(monotone_info);
  vertices_info=RelinquishVirtualMemory(vertices_info);
  return(convex_hull);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e D e p t h                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageDepth() returns the depth of a particular image channel.
%
%  The format of the GetImageDepth method is:
%
%      size_t GetImageDepth(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport size_t GetImageDepth(const Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    i;

  size_t
    *current_depth,
    depth,
    number_threads;

  ssize_t
    y;

  /*
    Compute image depth.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  current_depth=(size_t *) AcquireQuantumMemory(number_threads,
    sizeof(*current_depth));
  if (current_depth == (size_t *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  status=MagickTrue;
  for (i=0; i < (ssize_t) number_threads; i++)
    current_depth[i]=1;
  if ((image->storage_class == PseudoClass) &&
      ((image->alpha_trait & BlendPixelTrait) == 0))
    {
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        const int
          id = GetOpenMPThreadId();

        while (current_depth[id] < MAGICKCORE_QUANTUM_DEPTH)
        {
          MagickBooleanType
            atDepth;

          QuantumAny
            range;

          atDepth=MagickTrue;
          range=GetQuantumRange(current_depth[id]);
          if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
            if (IsPixelAtDepth(ClampToQuantum(image->colormap[i].red),range) == MagickFalse)
              atDepth=MagickFalse;
          if ((atDepth != MagickFalse) &&
              (GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
            if (IsPixelAtDepth(ClampToQuantum(image->colormap[i].green),range) == MagickFalse)
              atDepth=MagickFalse;
          if ((atDepth != MagickFalse) &&
              (GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
            if (IsPixelAtDepth(ClampToQuantum(image->colormap[i].blue),range) == MagickFalse)
              atDepth=MagickFalse;
          if ((atDepth != MagickFalse))
            break;
          current_depth[id]++;
        }
      }
      depth=current_depth[0];
      for (i=1; i < (ssize_t) number_threads; i++)
        if (depth < current_depth[i])
          depth=current_depth[i];
      current_depth=(size_t *) RelinquishMagickMemory(current_depth);
      return(depth);
    }
  image_view=AcquireVirtualCacheView(image,exception);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  DisableMSCWarning(4127)
  if ((1UL*QuantumRange) <= MaxMap)
  RestoreMSCWarning
    {
      size_t
        *depth_map;

      /*
        Scale pixels to desired (optimized with depth map).
      */
      depth_map=(size_t *) AcquireQuantumMemory(MaxMap+1,sizeof(*depth_map));
      if (depth_map == (size_t *) NULL)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
      for (i=0; i <= (ssize_t) MaxMap; i++)
      {
        for (depth=1; depth < (size_t) MAGICKCORE_QUANTUM_DEPTH; depth++)
        {
          Quantum
            pixel;

          QuantumAny
            range;

          range=GetQuantumRange(depth);
          pixel=(Quantum) i;
          if (pixel == ScaleAnyToQuantum(ScaleQuantumToAny(pixel,range),range))
            break;
        }
        depth_map[i]=depth;
      }
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        const int
          id = GetOpenMPThreadId();

        const Quantum
          *magick_restrict p;

        ssize_t
          x;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
        if (p == (const Quantum *) NULL)
          continue;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          ssize_t
            j;

          for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,j);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            if (depth_map[ScaleQuantumToMap(p[j])] > current_depth[id])
              current_depth[id]=depth_map[ScaleQuantumToMap(p[j])];
          }
          p+=GetPixelChannels(image);
        }
        if (current_depth[id] == MAGICKCORE_QUANTUM_DEPTH)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      depth=current_depth[0];
      for (i=1; i < (ssize_t) number_threads; i++)
        if (depth < current_depth[i])
          depth=current_depth[i];
      depth_map=(size_t *) RelinquishMagickMemory(depth_map);
      current_depth=(size_t *) RelinquishMagickMemory(current_depth);
      return(depth);
    }
#endif
  /*
    Compute pixel depth.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const int
      id = GetOpenMPThreadId();

    const Quantum
      *magick_restrict p;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      continue;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        j;

      for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
      {
        PixelChannel
          channel;

        PixelTrait
          traits;

        channel=GetPixelChannelChannel(image,j);
        traits=GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        while (current_depth[id] < MAGICKCORE_QUANTUM_DEPTH)
        {
          QuantumAny
            range;

          range=GetQuantumRange(current_depth[id]);
          if (p[j] == ScaleAnyToQuantum(ScaleQuantumToAny(p[j],range),range))
            break;
          current_depth[id]++;
        }
      }
      p+=GetPixelChannels(image);
    }
    if (current_depth[id] == MAGICKCORE_QUANTUM_DEPTH)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  depth=current_depth[0];
  for (i=1; i < (ssize_t) number_threads; i++)
    if (depth < current_depth[i])
      depth=current_depth[i];
  current_depth=(size_t *) RelinquishMagickMemory(current_depth);
  return(depth);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e M i n i m u m B o u n d i n g B o x                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageMinimumBoundingBox() returns the points that form the minimum
%  bounding box around the image foreground objects with the "Rotating
%  Calipers" algorithm.  The method also returns these properties:
%  minimum-bounding-box:area, minimum-bounding-box:width,
%  minimum-bounding-box:height, and minimum-bounding-box:angle.
%
%  The format of the GetImageMinimumBoundingBox method is:
%
%      PointInfo *GetImageMinimumBoundingBox(Image *image,
%        size_t number_vertices,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o number_vertices: the number of vertices in the bounding box.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _CaliperInfo
{
  double
    area,
    width,
    height,
    projection;

  ssize_t
    p,
    q,
    v;
} CaliperInfo;

static inline double getAngle(PointInfo *p,PointInfo *q)
{
  /*
    Get the angle between line (p,q) and horizontal axis, in degrees.
  */
  return(RadiansToDegrees(atan2(q->y-p->y,q->x-p->x)));
}

static inline double getDistance(PointInfo *p,PointInfo *q)
{
  double
    distance;

  distance=hypot(p->x-q->x,p->y-q->y);
  return(distance*distance);
}

static inline double getProjection(PointInfo *p,PointInfo *q,PointInfo *v)
{
  double
    distance;

  /*
    Projection of vector (x,y) - p into a line passing through p and q.
  */
  distance=getDistance(p,q);
  if (distance < MagickEpsilon)
    return(INFINITY);
  return((q->x-p->x)*(v->x-p->x)+(v->y-p->y)*(q->y-p->y))/sqrt(distance);
}

static inline double getFeretDiameter(PointInfo *p,PointInfo *q,PointInfo *v)
{
  double
    distance;

  /*
    Distance from a point (x,y) to a line passing through p and q.
  */
  distance=getDistance(p,q);
  if (distance < MagickEpsilon)
    return(INFINITY);
  return((q->x-p->x)*(v->y-p->y)-(v->x-p->x)*(q->y-p->y))/sqrt(distance);
}

MagickExport PointInfo *GetImageMinimumBoundingBox(Image *image,
  size_t *number_vertices,ExceptionInfo *exception)
{
  CaliperInfo
    caliper_info;

  const char
    *artifact;

  double
    angle,
    diameter,
    distance;

  PointInfo
    *bounding_box,
    *vertices;

  size_t
    number_hull_vertices;

  ssize_t
    i;

  /*
    Generate the minimum bounding box with the "Rotating Calipers" algorithm.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *number_vertices=0;
  vertices=GetImageConvexHull(image,&number_hull_vertices,exception);
  if (vertices == (PointInfo *) NULL)
    return((PointInfo *) NULL);
  *number_vertices=4;
  bounding_box=(PointInfo *) AcquireQuantumMemory(*number_vertices,
    sizeof(*bounding_box));
  if (bounding_box == (PointInfo *) NULL)
    {
      vertices=(PointInfo *) RelinquishMagickMemory(vertices);
      return((PointInfo *) NULL);
    }
  caliper_info.area=2.0*image->columns*image->rows;
  caliper_info.width=(double) image->columns+image->rows;
  caliper_info.height=0.0;
  caliper_info.projection=0.0;
  caliper_info.p=(-1);
  caliper_info.q=(-1);
  caliper_info.v=(-1);
  for (i=0; i < (ssize_t) number_hull_vertices; i++)
  {
    double
      area = 0.0,
      max_projection = 0.0,
      min_diameter = -1.0,
      min_projection = 0.0;

    ssize_t
      j,
      k;

    ssize_t
      p = -1,
      q = -1,
      v = -1;

    for (j=0; j < (ssize_t) number_hull_vertices; j++)
    {
      diameter=fabs(getFeretDiameter(&vertices[i],
        &vertices[(i+1) % (ssize_t) number_hull_vertices],&vertices[j]));
      if (min_diameter < diameter)
        {
          min_diameter=diameter;
          p=i;
          q=(i+1) % (ssize_t) number_hull_vertices;
          v=j;
        }
    }
    for (k=0; k < (ssize_t) number_hull_vertices; k++)
    {
      double
        projection;

      /*
        Rotating calipers.
      */
      projection=getProjection(&vertices[p],&vertices[q],&vertices[k]);
      min_projection=MagickMin(min_projection,projection);
      max_projection=MagickMax(max_projection,projection);
    }
    area=min_diameter*(max_projection-min_projection);
    if (caliper_info.area > area)
      {
        caliper_info.area=area;
        caliper_info.width=min_diameter;
        caliper_info.height=max_projection-min_projection;
        caliper_info.projection=max_projection;
        caliper_info.p=p;
        caliper_info.q=q;
        caliper_info.v=v;
      }
  }
  /*
    Initialize minimum bounding box.
  */
  diameter=getFeretDiameter(&vertices[caliper_info.p],
    &vertices[caliper_info.q],&vertices[caliper_info.v]);
  angle=atan2(vertices[caliper_info.q].y-vertices[caliper_info.p].y,
    vertices[caliper_info.q].x-vertices[caliper_info.p].x);
  bounding_box[0].x=vertices[caliper_info.p].x+cos(angle)*
    caliper_info.projection;
  bounding_box[0].y=vertices[caliper_info.p].y+sin(angle)*
    caliper_info.projection;
  bounding_box[1].x=floor(bounding_box[0].x+cos(angle+MagickPI/2.0)*diameter+
    0.5);
  bounding_box[1].y=floor(bounding_box[0].y+sin(angle+MagickPI/2.0)*diameter+
    0.5);
  bounding_box[2].x=floor(bounding_box[1].x+cos(angle)*(-caliper_info.height)+
    0.5);
  bounding_box[2].y=floor(bounding_box[1].y+sin(angle)*(-caliper_info.height)+
    0.5);
  bounding_box[3].x=floor(bounding_box[2].x+cos(angle+MagickPI/2.0)*(-diameter)+
    0.5);
  bounding_box[3].y=floor(bounding_box[2].y+sin(angle+MagickPI/2.0)*(-diameter)+
    0.5);
  /*
    Export minimum bounding box properties.
  */
  (void) FormatImageProperty(image,"minimum-bounding-box:area","%.*g",
    GetMagickPrecision(),caliper_info.area);
  (void) FormatImageProperty(image,"minimum-bounding-box:width","%.*g",
    GetMagickPrecision(),caliper_info.width);
  (void) FormatImageProperty(image,"minimum-bounding-box:height","%.*g",
    GetMagickPrecision(),caliper_info.height);
  (void) FormatImageProperty(image,"minimum-bounding-box:_p","%.*g,%.*g",
    GetMagickPrecision(),vertices[caliper_info.p].x,
    GetMagickPrecision(),vertices[caliper_info.p].y);
  (void) FormatImageProperty(image,"minimum-bounding-box:_q","%.*g,%.*g",
    GetMagickPrecision(),vertices[caliper_info.q].x,
    GetMagickPrecision(),vertices[caliper_info.q].y);
  (void) FormatImageProperty(image,"minimum-bounding-box:_v","%.*g,%.*g",
    GetMagickPrecision(),vertices[caliper_info.v].x,
    GetMagickPrecision(),vertices[caliper_info.v].y);
  /*
    Find smallest angle to origin.
  */
  distance=hypot(bounding_box[0].x,bounding_box[0].y);
  angle=getAngle(&bounding_box[0],&bounding_box[1]);
  for (i=1; i < 4; i++)
  {
    double d = hypot(bounding_box[i].x,bounding_box[i].y);
    if (d < distance)
      {
        distance=d;
        angle=getAngle(&bounding_box[i],&bounding_box[(i+1) % 4]);
      }
  }
  artifact=GetImageArtifact(image,"minimum-bounding-box:orientation");
  if (artifact != (const char *) NULL)
    {
      double
        length,
        q_length,
        p_length;

      PointInfo
        delta,
        point;

      /*
        Find smallest perpendicular distance from edge to origin.
      */
      point=bounding_box[0];
      for (i=1; i < 4; i++)
      {
        if (bounding_box[i].x < point.x)
          point.x=bounding_box[i].x;
        if (bounding_box[i].y < point.y)
          point.y=bounding_box[i].y;
      }
      for (i=0; i < 4; i++)
      {
        bounding_box[i].x-=point.x;
        bounding_box[i].y-=point.y;
      }
      for (i=0; i < 4; i++)
      {
        double
          d,
          intercept,
          slope;

        delta.x=bounding_box[(i+1) % 4].x-bounding_box[i].x;
        delta.y=bounding_box[(i+1) % 4].y-bounding_box[i].y;
        slope=delta.y*PerceptibleReciprocal(delta.x);
        intercept=bounding_box[(i+1) % 4].y-slope*bounding_box[i].x;
        d=fabs((slope*bounding_box[i].x-bounding_box[i].y+intercept)*
          PerceptibleReciprocal(sqrt(slope*slope+1.0)));
        if ((i == 0) || (d < distance))
          {
            distance=d;
            point=delta;
          }
      }
      angle=RadiansToDegrees(atan(point.y*PerceptibleReciprocal(point.x)));
      length=hypot(point.x,point.y);
      p_length=fabs((double) MagickMax(caliper_info.width,caliper_info.height)-
        length);
      q_length=fabs(length-(double) MagickMin(caliper_info.width,
        caliper_info.height));
      if (LocaleCompare(artifact,"landscape") == 0)
        {
          if (p_length > q_length)
            angle+=(angle < 0.0) ? 90.0 : -90.0;
        }
      else
        if (LocaleCompare(artifact,"portrait") == 0)
          {
            if (p_length < q_length)
              angle+=(angle >= 0.0) ? 90.0 : -90.0;
          }
    }
  (void) FormatImageProperty(image,"minimum-bounding-box:angle","%.*g",
    GetMagickPrecision(),angle);
  (void) FormatImageProperty(image,"minimum-bounding-box:unrotate","%.*g",
    GetMagickPrecision(),-angle);
  vertices=(PointInfo *) RelinquishMagickMemory(vertices);
  return(bounding_box);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e Q u a n t u m D e p t h                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageQuantumDepth() returns the depth of the image rounded to a legal
%  quantum depth: 8, 16, or 32.
%
%  The format of the GetImageQuantumDepth method is:
%
%      size_t GetImageQuantumDepth(const Image *image,
%        const MagickBooleanType constrain)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o constrain: A value other than MagickFalse, constrains the depth to
%      a maximum of MAGICKCORE_QUANTUM_DEPTH.
%
*/
MagickExport size_t GetImageQuantumDepth(const Image *image,
  const MagickBooleanType constrain)
{
  size_t
    depth;

  depth=image->depth;
  if (depth <= 8)
    depth=8;
  else
    if (depth <= 16)
      depth=16;
    else
      if (depth <= 32)
        depth=32;
      else
        if (depth <= 64)
          depth=64;
  if (constrain != MagickFalse)
    depth=(size_t) MagickMin((double) depth,(double) MAGICKCORE_QUANTUM_DEPTH);
  return(depth);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e T y p e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageType() returns the type of image:
%
%        Bilevel         Grayscale        GrayscaleMatte
%        Palette         PaletteMatte     TrueColor
%        TrueColorMatte  ColorSeparation  ColorSeparationMatte
%
%  The format of the GetImageType method is:
%
%      ImageType GetImageType(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport ImageType GetImageType(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->colorspace == CMYKColorspace)
    {
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        return(ColorSeparationType);
      return(ColorSeparationAlphaType);
    }
  if (IsImageMonochrome(image) != MagickFalse)
    return(BilevelType);
  if (IsImageGray(image) != MagickFalse)
    {
      if (image->alpha_trait != UndefinedPixelTrait)
        return(GrayscaleAlphaType);
      return(GrayscaleType);
    }
  if (IsPaletteImage(image) != MagickFalse)
    {
      if (image->alpha_trait != UndefinedPixelTrait)
        return(PaletteAlphaType);
      return(PaletteType);
    }
  if (image->alpha_trait != UndefinedPixelTrait)
    return(TrueColorAlphaType);
  return(TrueColorType);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I d e n t i f y I m a g e G r a y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IdentifyImageGray() returns grayscale if all the pixels in the image have
%  the same red, green, and blue intensities, and bi-level is the intensity is
%  either 0 or QuantumRange. Otherwise undefined is returned.
%
%  The format of the IdentifyImageGray method is:
%
%      ImageType IdentifyImageGray(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ImageType IdentifyImageGray(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  ImageType
    type;

  const Quantum
    *p;

  ssize_t
    x;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (IsImageGray(image) != MagickFalse)
    return(image->type);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    return(UndefinedType);
  type=BilevelType;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (IsPixelGray(image,p) == MagickFalse)
        {
          type=UndefinedType;
          break;
        }
      if ((type == BilevelType) && (IsPixelMonochrome(image,p) == MagickFalse))
        type=GrayscaleType;
      p+=GetPixelChannels(image);
    }
    if (type == UndefinedType)
      break;
  }
  image_view=DestroyCacheView(image_view);
  if ((type == GrayscaleType) && (image->alpha_trait != UndefinedPixelTrait))
    type=GrayscaleAlphaType;
  return(type);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I d e n t i f y I m a g e M o n o c h r o m e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IdentifyImageMonochrome() returns MagickTrue if all the pixels in the image
%  have the same red, green, and blue intensities and the intensity is either
%  0 or QuantumRange.
%
%  The format of the IdentifyImageMonochrome method is:
%
%      MagickBooleanType IdentifyImageMonochrome(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IdentifyImageMonochrome(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    bilevel;

  ssize_t
    x;

  const Quantum
    *p;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->type == BilevelType)
    return(MagickTrue);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    return(MagickFalse);
  bilevel=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (IsPixelMonochrome(image,p) == MagickFalse)
        {
          bilevel=MagickFalse;
          break;
        }
      p+=GetPixelChannels(image);
    }
    if (bilevel == MagickFalse)
      break;
  }
  image_view=DestroyCacheView(image_view);
  return(bilevel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I d e n t i f y I m a g e T y p e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IdentifyImageType() returns the potential type of image:
%
%        Bilevel         Grayscale        GrayscaleMatte
%        Palette         PaletteMatte     TrueColor
%        TrueColorMatte  ColorSeparation  ColorSeparationMatte
%
%  To ensure the image type matches its potential, use SetImageType():
%
%    (void) SetImageType(image,IdentifyImageType(image,exception),exception);
%
%  The format of the IdentifyImageType method is:
%
%      ImageType IdentifyImageType(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ImageType IdentifyImageType(const Image *image,
  ExceptionInfo *exception)
{
  ImageType
    type;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->colorspace == CMYKColorspace)
    {
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        return(ColorSeparationType);
      return(ColorSeparationAlphaType);
    }
  type=IdentifyImageGray(image,exception);
  if (IsGrayImageType(type))
    return(type);
  if (IdentifyPaletteImage(image,exception) != MagickFalse)
    {
      if (image->alpha_trait != UndefinedPixelTrait)
        return(PaletteAlphaType);
      return(PaletteType);
    }
  if (image->alpha_trait != UndefinedPixelTrait)
    return(TrueColorAlphaType);
  return(TrueColorType);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s I m a g e G r a y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImageGray() returns MagickTrue if the type of the image is grayscale or
%  bi-level.
%
%  The format of the IsImageGray method is:
%
%      MagickBooleanType IsImageGray(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsImageGray(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsGrayImageType(image->type) != MagickFalse)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s I m a g e M o n o c h r o m e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImageMonochrome() returns MagickTrue if type of the image is bi-level.
%
%  The format of the IsImageMonochrome method is:
%
%      MagickBooleanType IsImageMonochrome(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsImageMonochrome(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->type == BilevelType)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s I m a g e O p a q u e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImageOpaque() returns MagickTrue if none of the pixels in the image have
%  an alpha value other than OpaqueAlpha (QuantumRange).
%
%  Will return true immediately is alpha channel is not available.
%
%  The format of the IsImageOpaque method is:
%
%      MagickBooleanType IsImageOpaque(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsImageOpaque(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  const Quantum
    *p;

  ssize_t
    x;

  ssize_t
    y;

  /*
    Determine if image is opaque.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->alpha_trait & BlendPixelTrait) == 0)
    return(MagickTrue);
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (GetPixelAlpha(image,p) != OpaqueAlpha)
        break;
      p+=GetPixelChannels(image);
    }
    if (x < (ssize_t) image->columns)
      break;
  }
  image_view=DestroyCacheView(image_view);
  return(y < (ssize_t) image->rows ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e D e p t h                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageDepth() sets the depth of the image.
%
%  The format of the SetImageDepth method is:
%
%      MagickBooleanType SetImageDepth(Image *image,const size_t depth,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o depth: the image depth.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageDepth(Image *image,
  const size_t depth,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  QuantumAny
    range;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickCoreSignature);
  if (depth >= MAGICKCORE_QUANTUM_DEPTH)
    {
      image->depth=depth;
      return(MagickTrue);
    }
  range=GetQuantumRange(depth);
  if (image->storage_class == PseudoClass)
    {
      ssize_t
        i;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->colors,1)
#endif
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].red=(double) ScaleAnyToQuantum(ScaleQuantumToAny(
            ClampPixel(image->colormap[i].red),range),range);
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].green=(double) ScaleAnyToQuantum(ScaleQuantumToAny(
            ClampPixel(image->colormap[i].green),range),range);
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].blue=(double) ScaleAnyToQuantum(ScaleQuantumToAny(
            ClampPixel(image->colormap[i].blue),range),range);
        if ((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0)
          image->colormap[i].alpha=(double) ScaleAnyToQuantum(ScaleQuantumToAny(
            ClampPixel(image->colormap[i].alpha),range),range);
      }
    }
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  DisableMSCWarning(4127)
  if ((1UL*QuantumRange) <= MaxMap)
  RestoreMSCWarning
    {
      Quantum
        *depth_map;

      ssize_t
        i;

      /*
        Scale pixels to desired (optimized with depth map).
      */
      depth_map=(Quantum *) AcquireQuantumMemory(MaxMap+1,sizeof(*depth_map));
      if (depth_map == (Quantum *) NULL)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
      for (i=0; i <= (ssize_t) MaxMap; i++)
        depth_map[i]=ScaleAnyToQuantum(ScaleQuantumToAny((Quantum) i,range),
          range);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        ssize_t
          x;

        Quantum
          *magick_restrict q;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          ssize_t
            j;

          for (j=0; j < (ssize_t) GetPixelChannels(image); j++)
          {
            PixelChannel
              channel;

            PixelTrait
              traits;

            channel=GetPixelChannelChannel(image,j);
            traits=GetPixelChannelTraits(image,channel);
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            q[j]=depth_map[ScaleQuantumToMap(q[j])];
          }
          q+=GetPixelChannels(image);
        }
        if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
          {
            status=MagickFalse;
            continue;
          }
      }
      image_view=DestroyCacheView(image_view);
      depth_map=(Quantum *) RelinquishMagickMemory(depth_map);
      if (status != MagickFalse)
        image->depth=depth;
      return(status);
    }
#endif
  /*
    Scale pixels to desired depth.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    ssize_t
      x;

    Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel
          channel;

        PixelTrait
          traits;

        channel=GetPixelChannelChannel(image,i);
        traits=GetPixelChannelTraits(image,channel);
        if ((traits & UpdatePixelTrait) == 0)
          continue;
        q[i]=ScaleAnyToQuantum(ScaleQuantumToAny(ClampPixel((MagickRealType)
          q[i]),range),range);
      }
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      {
        status=MagickFalse;
        continue;
      }
  }
  image_view=DestroyCacheView(image_view);
  if (status != MagickFalse)
    image->depth=depth;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e T y p e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageType() sets the type of image.  Choose from these types:
%
%        Bilevel        Grayscale       GrayscaleMatte
%        Palette        PaletteMatte    TrueColor
%        TrueColorMatte ColorSeparation ColorSeparationMatte
%        OptimizeType
%
%  The format of the SetImageType method is:
%
%      MagickBooleanType SetImageType(Image *image,const ImageType type,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o type: Image type.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageType(Image *image,const ImageType type,
  ExceptionInfo *exception)
{
  const char
    *artifact;

  ImageInfo
    *image_info;

  MagickBooleanType
    status;

  QuantizeInfo
    *quantize_info;

  assert(image != (Image *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickCoreSignature);
  status=MagickTrue;
  image_info=AcquireImageInfo();
  image_info->dither=image->dither;
  artifact=GetImageArtifact(image,"dither");
  if (artifact != (const char *) NULL)
    (void) SetImageOption(image_info,"dither",artifact);
  switch (type)
  {
    case BilevelType:
    {
      if (IsGrayImageType(image->type) == MagickFalse)
        status=TransformImageColorspace(image,GRAYColorspace,exception);
      (void) NormalizeImage(image,exception);
      (void) BilevelImage(image,(double) QuantumRange/2.0,exception);
      quantize_info=AcquireQuantizeInfo(image_info);
      quantize_info->number_colors=2;
      quantize_info->colorspace=GRAYColorspace;
      status=QuantizeImage(quantize_info,image,exception);
      quantize_info=DestroyQuantizeInfo(quantize_info);
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case GrayscaleType:
    {
      if (IsGrayImageType(image->type) == MagickFalse)
        status=TransformImageColorspace(image,GRAYColorspace,exception);
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case GrayscaleAlphaType:
    {
      if (IsGrayImageType(image->type) == MagickFalse)
        status=TransformImageColorspace(image,GRAYColorspace,exception);
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      break;
    }
    case PaletteType:
    {
      if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
        status=TransformImageColorspace(image,sRGBColorspace,exception);
      if ((image->storage_class == DirectClass) || (image->colors > 256))
        {
          quantize_info=AcquireQuantizeInfo(image_info);
          quantize_info->number_colors=256;
          status=QuantizeImage(quantize_info,image,exception);
          quantize_info=DestroyQuantizeInfo(quantize_info);
        }
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case PaletteBilevelAlphaType:
    {
      ChannelType
        channel_mask;

      if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
        status=TransformImageColorspace(image,sRGBColorspace,exception);
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      channel_mask=SetImageChannelMask(image,AlphaChannel);
      (void) BilevelImage(image,(double) QuantumRange/2.0,exception);
      (void) SetImageChannelMask(image,channel_mask);
      quantize_info=AcquireQuantizeInfo(image_info);
      status=QuantizeImage(quantize_info,image,exception);
      quantize_info=DestroyQuantizeInfo(quantize_info);
      break;
    }
    case PaletteAlphaType:
    {
      if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
        status=TransformImageColorspace(image,sRGBColorspace,exception);
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      quantize_info=AcquireQuantizeInfo(image_info);
      quantize_info->colorspace=TransparentColorspace;
      status=QuantizeImage(quantize_info,image,exception);
      quantize_info=DestroyQuantizeInfo(quantize_info);
      break;
    }
    case TrueColorType:
    {
      if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
        status=TransformImageColorspace(image,sRGBColorspace,exception);
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass,exception);
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case TrueColorAlphaType:
    {
      if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
        status=TransformImageColorspace(image,sRGBColorspace,exception);
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass,exception);
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        (void) SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      break;
    }
    case ColorSeparationType:
    {
      if (image->colorspace != CMYKColorspace)
        status=TransformImageColorspace(image,CMYKColorspace,exception);
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass,exception);
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case ColorSeparationAlphaType:
    {
      if (image->colorspace != CMYKColorspace)
        status=TransformImageColorspace(image,CMYKColorspace,exception);
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass,exception);
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        status=SetImageAlphaChannel(image,OpaqueAlphaChannel,exception);
      break;
    }
    case OptimizeType:
    case UndefinedType:
      break;
  }
  image_info=DestroyImageInfo(image_info);
  if (status == MagickFalse)
    return(status);
  image->type=type;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                   V   V  IIIII  SSSSS  IIIII   OOO   N   N                  %
%                   V   V    I    SS       I    O   O  NN  N                  %
%                   V   V    I     SSS     I    O   O  N N N                  %
%                    V V     I       SS    I    O   O  N  NN                  %
%                     V    IIIII  SSSSS  IIIII   OOO   N   N                  %
%                                                                             %
%                                                                             %
%                      MagickCore Computer Vision Methods                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               September 2014                                %
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

#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/blob.h"
#include "magick/cache-view.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/constitute.h"
#include "magick/decorate.h"
#include "magick/distort.h"
#include "magick/draw.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/effect.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/matrix.h"
#include "magick/memory_.h"
#include "magick/memory-private.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/montage.h"
#include "magick/morphology.h"
#include "magick/morphology-private.h"
#include "magick/opencl-private.h"
#include "magick/paint.h"
#include "magick/pixel-accessor.h"
#include "magick/pixel-private.h"
#include "magick/property.h"
#include "magick/quantum.h"
#include "magick/resource_.h"
#include "magick/signature-private.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/token.h"
#include "magick/vision.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o n n e c t e d C o m p o n e n t s I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConnectedComponentsImage() returns the connected-components of the image
%  uniquely labeled.  Choose from 4 or 8-way connectivity.
%
%  The format of the ConnectedComponentsImage method is:
%
%      Image *ConnectedComponentsImage(const Image *image,
%        const size_t connectivity,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o connectivity: how many neighbors to visit.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType ConnectedComponentsStatistics(const Image *image,
  const size_t number_objects,ExceptionInfo *exception)
{
  typedef struct _CCStatistic
  {
    ssize_t
      area;
  } CCStatistic;

  CacheView
    *image_view;

  CCStatistic
    statistic;

  MagickBooleanType
    status;

  MatrixInfo
    *statistics;

  register ssize_t
    i;

  ssize_t
    y;

  statistics=AcquireMatrixInfo(number_objects,1,sizeof(CCStatistic),exception);
  if (statistics == (MatrixInfo *) NULL)
    return(MagickFalse);
  (void) NullMatrix(statistics);
  (void) fprintf(stdout,
    "Connected components (id bounding-box centroid area):\n");
  status=MagickTrue;
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register const PixelPacket
      *restrict p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      status=GetMatrixElement(statistics,(ssize_t) p->red,0,&statistic);
      statistic.area++;
      status=SetMatrixElement(statistics,(ssize_t) p->red,0,&statistic);
      p++;
    }
  }
  image_view=DestroyCacheView(image_view);
  for (i=0; i < (ssize_t) number_objects; i++)
  {
    if (status == MagickFalse)
      break;
    status=GetMatrixElement(statistics,i,0,&statistic);
    (void) fprintf(stdout,
      "  %.20g: %.20gx%.20g%+.20g%+.20g %.20gx%.20g %.20g\n",(double) i,
      0.0,0.0,0.0,0.0,0.0,0.0,(double) statistic.area);
    /* WxH+X+Y is the bounding box relative to the upper left corner */
    /* Cx,Xy is the centroid */
  }
  statistics=DestroyMatrixInfo(statistics);
  return(status);
}

static inline ssize_t MagickMax(const ssize_t x,const ssize_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline ssize_t MagickMin(const ssize_t x,const ssize_t y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport Image *ConnectedComponentsImage(const Image *image,
  const size_t connectivity,ExceptionInfo *exception)
{
#define ConnectedComponentsImageTag  "ConnectedComponents/Image"

  CacheView
    *image_view,
    *component_view;

  const char
    *artifact;

  Image
    *component_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MatrixInfo
    *equivalences;

  size_t
    size;

  ssize_t
    n,
    offset,
    y;

  /*
    Initialize connected components image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  component_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (component_image == (Image *) NULL)
    return((Image *) NULL);
  component_image->depth=MAGICKCORE_QUANTUM_DEPTH;
  component_image->colorspace=GRAYColorspace;
  if (SetImageStorageClass(component_image,DirectClass) == MagickFalse)
    {
      component_image=DestroyImage(component_image);
      return((Image *) NULL);
    }
  /*
    Initialize connected components equivalences.
  */
  size=image->columns*image->rows;
  if (image->columns != (size/image->rows))
    {
      component_image=DestroyImage(component_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  equivalences=AcquireMatrixInfo(size,1,sizeof(ssize_t),exception);
  if (equivalences == (MatrixInfo *) NULL)
    { 
      component_image=DestroyImage(component_image);
      return((Image *) NULL);
    }
  for (n=0; n < (ssize_t) (image->columns*image->rows); n++)
    status=SetMatrixElement(equivalences,n,0,&n);
  /*
    Find connected components.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  for (n=0; n < (ssize_t) (connectivity > 4 ? 4 : 2); n++)
  {
    ssize_t
      connect4[2][2] = { { -1,  0 }, {  0, -1 } },
      connect8[4][2] = { { -1, -1 }, { -1,  0 }, { -1,  1 }, {  0, -1 } }, 
      dx,
      dy,
      start,
      stop;

    if (status == MagickFalse)
      continue;
    dy=connectivity > 4 ? connect8[n][0] : connect4[n][0];
    dx=connectivity > 4 ? connect8[n][1] : connect4[n][1];
    start=MagickMax(0,-dy);
    stop=MagickMin((ssize_t) image->rows,(ssize_t) image->rows-dy);
    for (y=start; y < stop; y++)
    {
      register const PixelPacket
        *restrict p;

      register ssize_t
        x;

      ssize_t
        offset,
        start,
        stop;

      if (status == MagickFalse)
        continue;
      p=GetCacheViewVirtualPixels(image_view,-1,y-1,image->columns+2,3,
        exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      start=MagickMax(0,-dx);
      stop=MagickMin((ssize_t) image->columns,(ssize_t) image->columns-dx);
      p+=image->columns+start+3;
      offset=y*image->columns+start;
      for (x=start; x < stop; x++)
      {
        const PixelPacket
          *neighbor;

        neighbor=p+dy*(image->columns+2)+dx;
        if (IsColorSimilar(image,p,neighbor) != MagickFalse)
          {
            ssize_t
              object,
              root,
              rx,
              ry;

            /*
              Find root.
            */
            rx=offset;
            status=GetMatrixElement(equivalences,rx,0,&object);
            while (object != rx) {
              rx=object;
              status=GetMatrixElement(equivalences,rx,0,&object);
            }
            ry=offset+dy*image->columns+dx;
            status=GetMatrixElement(equivalences,ry,0,&object);
            while (object != ry) {
              ry=object;
              status=GetMatrixElement(equivalences,ry,0,&object);
            }
            if (rx < ry)
              {
                status=SetMatrixElement(equivalences,ry,0,&rx);
                root=rx;
              }
            else
              {
                status=SetMatrixElement(equivalences,rx,0,&ry);
                root=ry;
              }
            /*
              Compress path.
            */
            rx=offset;
            status=GetMatrixElement(equivalences,rx,0,&object);
            while (object != root) {
              status=GetMatrixElement(equivalences,rx,0,&object);
              status=SetMatrixElement(equivalences,rx,0,&root);
            }
            ry=offset+dy*image->columns+dx;
            status=GetMatrixElement(equivalences,ry,0,&object);
            while (object != root) {
              status=GetMatrixElement(equivalences,ry,0,&object);
              status=SetMatrixElement(equivalences,ry,0,&root);
            }
            status=SetMatrixElement(equivalences,offset,0,&root);
          }
        offset++;
        p++;
      }
    }
  }
  image_view=DestroyCacheView(image_view);
  /*
    Label connected components.
  */
  offset=0;
  n=0;
  component_view=AcquireAuthenticCacheView(component_image,exception);
  for (y=0; y < (ssize_t) component_image->rows; y++)
  {
    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=QueueCacheViewAuthenticPixels(component_view,0,y,component_image->columns,
      1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) component_image->columns; x++)
    {
      ssize_t
        object;

      status=GetMatrixElement(equivalences,offset,0,&object);
      if (object == offset)
        { 
          object=n++;
          status=SetMatrixElement(equivalences,offset,0,&object);
        }
      else
        { 
          status=GetMatrixElement(equivalences,object,0,&object);
          status=SetMatrixElement(equivalences,offset,0,&object);
        }
      q->red=(Quantum) (object > (ssize_t) QuantumRange ? (ssize_t)
        QuantumRange : object);
      q->green=q->red;
      q->blue=q->red;
      q++;
      offset++;
    }
    if (SyncCacheViewAuthenticPixels(component_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,ConnectedComponentsImageTag,progress++,
          image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  component_view=DestroyCacheView(component_view);
  equivalences=DestroyMatrixInfo(equivalences);
  artifact=GetImageArtifact(image,"connected-components:verbose");
  if (IsMagickTrue(artifact))
    status=ConnectedComponentsStatistics(component_image,(size_t) n,exception);
  if (status == MagickFalse)
    component_image=DestroyImage(component_image);
  return(component_image);
}

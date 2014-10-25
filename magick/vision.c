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

typedef struct _CCObject
{
  ssize_t
    id;

  RectangleInfo
    bounding_box;

  PointInfo
    centroid;

  ssize_t
    area;
} CCObject;

static int CCObjectCompare(const void *x,const void *y)
{
  CCObject
    *p,
    *q;

  p=(CCObject *) x;
  q=(CCObject *) y;
  return((int) (q->area-(ssize_t) p->area));
}

static MagickBooleanType ConnectedComponentsStatistics(const Image *image,
  const size_t number_objects,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  CCObject
    *object;

  MagickBooleanType
    status;

  register ssize_t
    i;

  ssize_t
    y;

  object=(CCObject *) AcquireQuantumMemory(number_objects,sizeof(*object));
  if (object == (CCObject *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  (void) ResetMagickMemory(object,0,number_objects*sizeof(*object));
  for (i=0; i < (ssize_t) number_objects; i++)
  {
    object[i].id=i;
    object[i].bounding_box.x=(ssize_t) image->columns;
    object[i].bounding_box.y=(ssize_t) image->rows;
  }
  (void) fprintf(stdout,"Objects (id: bounding-box centroid area):\n");
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
      i=(ssize_t) p->red;
      if (x < object[i].bounding_box.x)
        object[i].bounding_box.x=x;
      if (x > (ssize_t) object[i].bounding_box.width)
        object[i].bounding_box.width=(size_t) x;
      if (y < object[i].bounding_box.y)
        object[i].bounding_box.y=y;
      if (y > (ssize_t) object[i].bounding_box.height)
        object[i].bounding_box.height=(size_t) y;
      object[i].area++;
      p++;
    }
  }
  for (i=0; i < (ssize_t) number_objects; i++)
  {
    object[i].bounding_box.width-=(object[i].bounding_box.x-1);
    object[i].bounding_box.height-=(object[i].bounding_box.y-1);
  }
  for (i=0; i < (ssize_t) number_objects; i++)
  {
    for (y=0; y < (ssize_t) object[i].bounding_box.height; y++)
    {
      register const PixelPacket
        *restrict p;

      register ssize_t
        x;

      if (status == MagickFalse)
        continue;
      p=GetCacheViewVirtualPixels(image_view,object[i].bounding_box.x,
        object[i].bounding_box.y+y,object[i].bounding_box.width,1,exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      for (x=0; x < (ssize_t) object[i].bounding_box.width; x++)
      {
        if ((ssize_t) p->red == i)
          {
            object[i].centroid.x+=x;
            object[i].centroid.y+=y;
          }
        p++;
      }
    }
    object[i].centroid.x=(double) object[i].bounding_box.x+object[i].centroid.x/
      object[i].area;
    object[i].centroid.y=(double) object[i].bounding_box.y+object[i].centroid.y/
      object[i].area;
  }
  image_view=DestroyCacheView(image_view);
  qsort((void *) object,number_objects,sizeof(*object),CCObjectCompare);
  for (i=0; i < (ssize_t) number_objects; i++)
  {
    if (status == MagickFalse)
      break;
    (void) fprintf(stdout,
      "  %.20g: %.20gx%.20g%+.20g%+.20g %+.1f,%+.1f %.20g\n",(double)
      object[i].id,(double) object[i].bounding_box.width,(double)
      object[i].bounding_box.height,(double) object[i].bounding_box.x,
      (double) object[i].bounding_box.y,object[i].centroid.x,
      object[i].centroid.y,(double) object[i].area);
  }
  object=(CCObject *) RelinquishMagickMemory(object);
  return(status);
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
      dy;

    if (status == MagickFalse)
      continue;
    dy=connectivity > 4 ? connect8[n][0] : connect4[n][0];
    dx=connectivity > 4 ? connect8[n][1] : connect4[n][1];
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      register const PixelPacket
        *restrict p;

      register ssize_t
        x;

      if (status == MagickFalse)
        continue;
      p=GetCacheViewVirtualPixels(image_view,0,y-1,image->columns,3,
        exception);
      if (p == (const PixelPacket *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      p+=image->columns;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        ssize_t
          neighbor_offset,
          object,
          ox,
          oy,
          pixel_offset,
          root;

        /*
          Is neighbor an authentic pixel and a different color than the pixel?
        */
        neighbor_offset=dy*image->columns+dx;
        if (((x+dx) < 0) || ((x+dx) >= (ssize_t) image->columns) ||
            ((y+dy) < 0) || ((y+dy) >= (ssize_t) image->rows) ||
            (IsColorSimilar(image,p,p+neighbor_offset) == MagickFalse))
          {
            p++;
            continue;
          }
        /*
          Resolve this equivalence.
        */
        pixel_offset=y*image->columns+x;
        ox=pixel_offset;
        status=GetMatrixElement(equivalences,ox,0,&object);
        while (object != ox) {
          ox=object;
          status=GetMatrixElement(equivalences,ox,0,&object);
        }
        oy=pixel_offset+neighbor_offset;
        status=GetMatrixElement(equivalences,oy,0,&object);
        while (object != oy) {
          oy=object;
          status=GetMatrixElement(equivalences,oy,0,&object);
        }
        if (ox < oy)
          {
            status=SetMatrixElement(equivalences,oy,0,&ox);
            root=ox;
          }
        else
          {
            status=SetMatrixElement(equivalences,ox,0,&oy);
            root=oy;
          }
        ox=pixel_offset;
        status=GetMatrixElement(equivalences,ox,0,&object);
        while (object != root) {
          status=GetMatrixElement(equivalences,ox,0,&object);
          status=SetMatrixElement(equivalences,ox,0,&root);
        }
        oy=pixel_offset+neighbor_offset;
        status=GetMatrixElement(equivalences,oy,0,&object);
        while (object != root) {
          status=GetMatrixElement(equivalences,oy,0,&object);
          status=SetMatrixElement(equivalences,oy,0,&root);
        }
        status=SetMatrixElement(equivalences,y*image->columns+x,0,&root);
        p++;
      }
    }
  }
  image_view=DestroyCacheView(image_view);
  /*
    Label connected components.
  */
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
        object,
        offset;

      offset=y*image->columns+x;
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

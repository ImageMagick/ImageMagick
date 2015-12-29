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
%  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/colormap.h"
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
#include "magick/string-private.h"
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
%    o connectivity: how many neighbors to visit, choose from 4 or 8.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _CCObjectInfo
{
  ssize_t
    id;

  RectangleInfo
    bounding_box;

  MagickPixelPacket
    color;

  PointInfo
    centroid;

  double
    area,
    census;
} CCObjectInfo;

static int CCObjectInfoCompare(const void *x,const void *y)
{
  CCObjectInfo
    *p,
    *q;

  p=(CCObjectInfo *) x;
  q=(CCObjectInfo *) y;
  return((int) (q->area-(ssize_t) p->area));
}

MagickExport Image *ConnectedComponentsImage(const Image *image,
  const size_t connectivity,ExceptionInfo *exception)
{
#define ConnectedComponentsImageTag  "ConnectedComponents/Image"

  CacheView
    *image_view,
    *component_view;

  CCObjectInfo
    *object;

  char
    *p;

  const char
    *artifact;

  double
    area_threshold;

  Image
    *component_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MatrixInfo
    *equivalences;

  register ssize_t
    i;

  size_t
    size;

  ssize_t
    first,
    last,
    n,
    step,
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
  if (AcquireImageColormap(component_image,MaxColormapSize) == MagickFalse)
    {
      component_image=DestroyImage(component_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
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
    (void) SetMatrixElement(equivalences,n,0,&n);
  object=(CCObjectInfo *) AcquireQuantumMemory(MaxColormapSize,sizeof(*object));
  if (object == (CCObjectInfo *) NULL)
    {
      equivalences=DestroyMatrixInfo(equivalences);
      component_image=DestroyImage(component_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  (void) ResetMagickMemory(object,0,MaxColormapSize*sizeof(*object));
  for (i=0; i < (ssize_t) MaxColormapSize; i++)
  {
    object[i].id=i;
    object[i].bounding_box.x=(ssize_t) image->columns;
    object[i].bounding_box.y=(ssize_t) image->rows;
    GetMagickPixelPacket(image,&object[i].color);
  }
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
        *magick_restrict p;

      register ssize_t
        x;

      if (status == MagickFalse)
        continue;
      p=GetCacheViewVirtualPixels(image_view,0,y-1,image->columns,3,exception);
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
          offset,
          ox,
          oy,
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
        offset=y*image->columns+x;
        ox=offset;
        status=GetMatrixElement(equivalences,ox,0,&object);
        while (object != ox)
        {
          ox=object;
          status=GetMatrixElement(equivalences,ox,0,&object);
        }
        oy=offset+neighbor_offset;
        status=GetMatrixElement(equivalences,oy,0,&object);
        while (object != oy)
        {
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
        ox=offset;
        status=GetMatrixElement(equivalences,ox,0,&object);
        while (object != root)
        {
          status=GetMatrixElement(equivalences,ox,0,&object);
          status=SetMatrixElement(equivalences,ox,0,&root);
        }
        oy=offset+neighbor_offset;
        status=GetMatrixElement(equivalences,oy,0,&object);
        while (object != root)
        {
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
  image_view=AcquireVirtualCacheView(image,exception);
  component_view=AcquireAuthenticCacheView(component_image,exception);
  for (y=0; y < (ssize_t) component_image->rows; y++)
  {
    register const IndexPacket
      *magick_restrict indexes;

    register const PixelPacket
      *magick_restrict p;

    register IndexPacket
      *magick_restrict component_indexes;

    register PixelPacket
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(component_view,0,y,component_image->columns,
      1,exception);
    if ((p == (const PixelPacket *) NULL) ||
        (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    component_indexes=GetCacheViewAuthenticIndexQueue(component_view);
    for (x=0; x < (ssize_t) component_image->columns; x++)
    {
      ssize_t
        id,
        offset;

      offset=y*image->columns+x;
      status=GetMatrixElement(equivalences,offset,0,&id);
      if (id == offset)
        {
          id=n++;
          if (n > (ssize_t) MaxColormapSize)
            break;
          status=SetMatrixElement(equivalences,offset,0,&id);
        }
      else
        {
          status=GetMatrixElement(equivalences,id,0,&id);
          status=SetMatrixElement(equivalences,offset,0,&id);
        }
      if (x < object[id].bounding_box.x)
        object[id].bounding_box.x=x;
      if (x > (ssize_t) object[id].bounding_box.width)
        object[id].bounding_box.width=(size_t) x;
      if (y < object[id].bounding_box.y)
        object[id].bounding_box.y=y;
      if (y > (ssize_t) object[id].bounding_box.height)
        object[id].bounding_box.height=(size_t) y;
      object[id].color.red+=p->red;
      object[id].color.green+=p->green;
      object[id].color.blue+=p->blue;
      if (image->matte != MagickFalse)
        object[id].color.opacity+=p->opacity;
      if (image->colorspace == CMYKColorspace)
        object[id].color.index+=indexes[x];
      object[id].centroid.x+=x;
      object[id].centroid.y+=y;
      object[id].area++;
      component_indexes[x]=(IndexPacket) id;
      p++;
      q++;
    }
    if (n > (ssize_t) MaxColormapSize)
      break;
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
  image_view=DestroyCacheView(image_view);
  equivalences=DestroyMatrixInfo(equivalences);
  if (n > (ssize_t) MaxColormapSize)
    {
      object=(CCObjectInfo *) RelinquishMagickMemory(object);
      component_image=DestroyImage(component_image);
      ThrowImageException(ResourceLimitError,"TooManyObjects");
    }
  component_image->colors=(size_t) n;
  for (i=0; i < (ssize_t) component_image->colors; i++)
  {
    object[i].bounding_box.width-=(object[i].bounding_box.x-1);
    object[i].bounding_box.height-=(object[i].bounding_box.y-1);
    object[i].color.red=object[i].color.red/object[i].area;
    object[i].color.green=object[i].color.green/object[i].area;
    object[i].color.blue=object[i].color.blue/object[i].area;
    if (image->matte != MagickFalse)
      object[i].color.opacity=object[i].color.opacity/object[i].area;
    if (image->colorspace == CMYKColorspace)
      object[i].color.index=object[i].color.index/object[i].area;
    object[i].centroid.x=object[i].centroid.x/object[i].area;
    object[i].centroid.y=object[i].centroid.y/object[i].area;
  }
  artifact=GetImageArtifact(image,"connected-components:area-threshold");
  area_threshold=0.0;
  if (artifact != (const char *) NULL)
    area_threshold=StringToDouble(artifact,(char **) NULL);
  if (area_threshold > 0.0)
    {
      /*
        Merge objects below area threshold.
      */
      component_view=AcquireAuthenticCacheView(component_image,exception);
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        double
          census;

        RectangleInfo
          bounding_box;

        register ssize_t
          j;

        size_t
          id;

        if (status == MagickFalse)
          continue;
        if ((double) object[i].area >= area_threshold)
          continue;
        for (j=0; j < (ssize_t) component_image->colors; j++)
          object[j].census=0;
        bounding_box=object[i].bounding_box;
        for (y=0; y < (ssize_t) bounding_box.height+2; y++)
        {
          register const IndexPacket
            *magick_restrict indexes;

          register const PixelPacket
            *magick_restrict p;

          register ssize_t
            x;

          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x-1,
            bounding_box.y+y-1,bounding_box.width+2,1,exception);
          if (p == (const PixelPacket *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          indexes=GetCacheViewVirtualIndexQueue(component_view);
          for (x=0; x < (ssize_t) bounding_box.width+2; x++)
          {
            j=(ssize_t) indexes[x];
            if (j != i)
              object[j].census++;
          }
        }
        census=0;
        id=0;
        for (j=0; j < (ssize_t) component_image->colors; j++)
          if (census < object[j].census)
            {
              census=object[j].census;
              id=(size_t) j;
            }
        object[id].area+=object[i].area;
        for (y=0; y < (ssize_t) bounding_box.height; y++)
        {
          register IndexPacket
            *magick_restrict component_indexes;

          register PixelPacket
            *magick_restrict q;

          register ssize_t
            x;

          if (status == MagickFalse)
            continue;
          q=GetCacheViewAuthenticPixels(component_view,bounding_box.x,
            bounding_box.y+y,bounding_box.width,1,exception);
          if (q == (PixelPacket *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          component_indexes=GetCacheViewAuthenticIndexQueue(component_view);
          for (x=0; x < (ssize_t) bounding_box.width; x++)
          {
            if ((ssize_t) component_indexes[x] == i)
              component_indexes[x]=(IndexPacket) id;
          }
          if (SyncCacheViewAuthenticPixels(component_view,exception) == MagickFalse)
            status=MagickFalse;
        }
      }
      (void) SyncImage(component_image);
    }
  artifact=GetImageArtifact(image,"connected-components:mean-color");
  if (IsMagickTrue(artifact) != MagickFalse)
    {
      /*
        Replace object with mean color.
      */
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        component_image->colormap[i].red=ClampToQuantum(object[i].color.red);
        component_image->colormap[i].green=ClampToQuantum(
          object[i].color.green);
        component_image->colormap[i].blue=ClampToQuantum(object[i].color.blue);
        component_image->colormap[i].opacity=ClampToQuantum(
          object[i].color.opacity);
      }
    }
  artifact=GetImageArtifact(image,"connected-components:keep");
  if (artifact != (const char *) NULL)
    {
      /*
        Keep these objects (make others transparent).
      */
      for (i=0; i < (ssize_t) component_image->colors; i++)
        object[i].census=0;
      for (p=(char *) artifact; *p != '\0';)
      {
        while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == ','))
          p++;
        first=strtol(p,&p,10);
        if (first < 0)
          first+=(long) component_image->colors;
        last=first;
        while (isspace((int) ((unsigned char) *p)) != 0)
          p++;
        if (*p == '-')
          {
            last=strtol(p+1,&p,10);
            if (last < 0)
              last+=(long) component_image->colors;
          }
        for (step=first > last ? -1 : 1; first != (last+step); first+=step)
          object[first].census++;
      }
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        if (object[i].census != 0)
          continue;
        component_image->matte=MagickTrue;
        component_image->colormap[i].opacity=TransparentOpacity;
      }
    }
  artifact=GetImageArtifact(image,"connected-components:remove");
  if (artifact != (const char *) NULL)
    {
      /*
        Remove these objects (make them transparent).
      */
      for (p=(char *) artifact; *p != '\0';)
      {
        while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == ','))
          p++;
        first=strtol(p,&p,10);
        if (first < 0)
          first+=(long) component_image->colors;
        last=first;
        while (isspace((int) ((unsigned char) *p)) != 0)
          p++;
        if (*p == '-')
          {
            last=strtol(p+1,&p,10);
            if (last < 0)
              last+=(long) component_image->colors;
          }
        for (step=first > last ? -1 : 1; first != (last+step); first+=step)
        {
          component_image->matte=MagickTrue;
          component_image->colormap[first].opacity=TransparentOpacity;
        }
      }
    }
  (void) SyncImage(component_image);
  artifact=GetImageArtifact(image,"connected-components:verbose");
  if (IsMagickTrue(artifact) != MagickFalse)
    {
      /*
        Report statistics on unique objects.
      */
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        object[i].bounding_box.width=0;
        object[i].bounding_box.height=0;
        object[i].bounding_box.x=(ssize_t) component_image->columns;
        object[i].bounding_box.y=(ssize_t) component_image->rows;
        object[i].centroid.x=0;
        object[i].centroid.y=0;
        object[i].area=0;
      }
      component_view=AcquireVirtualCacheView(component_image,exception);
      for (y=0; y < (ssize_t) component_image->rows; y++)
      {
        register const IndexPacket
          *indexes;

        register const PixelPacket
          *magick_restrict p;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(component_view,0,y,
          component_image->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        indexes=GetCacheViewVirtualIndexQueue(component_view);
        for (x=0; x < (ssize_t) component_image->columns; x++)
        {
          size_t
            id;

          id=indexes[x];
          if (x < object[id].bounding_box.x)
            object[id].bounding_box.x=x;
          if (x > (ssize_t) object[id].bounding_box.width)
            object[id].bounding_box.width=(size_t) x;
          if (y < object[id].bounding_box.y)
            object[id].bounding_box.y=y;
          if (y > (ssize_t) object[id].bounding_box.height)
            object[id].bounding_box.height=(size_t) y;
          object[id].centroid.x+=x;
          object[id].centroid.y+=y;
          object[id].area++;
        }
      }
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        object[i].bounding_box.width-=(object[i].bounding_box.x-1);
        object[i].bounding_box.height-=(object[i].bounding_box.y-1);
        object[i].centroid.x=object[i].centroid.x/object[i].area;
        object[i].centroid.y=object[i].centroid.y/object[i].area;
      }
      component_view=DestroyCacheView(component_view);
      qsort((void *) object,component_image->colors,sizeof(*object),
        CCObjectInfoCompare);
      (void) fprintf(stdout,
        "Objects (id: bounding-box centroid area mean-color):\n");
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        char
          mean_color[MaxTextExtent];

        if (status == MagickFalse)
          break;
        if (object[i].area < MagickEpsilon)
          continue;
        GetColorTuple(&object[i].color,MagickFalse,mean_color);
        (void) fprintf(stdout,
          "  %.20g: %.20gx%.20g%+.20g%+.20g %.1f,%.1f %.20g %s\n",(double)
          object[i].id,(double) object[i].bounding_box.width,(double)
          object[i].bounding_box.height,(double) object[i].bounding_box.x,
          (double) object[i].bounding_box.y,object[i].centroid.x,
          object[i].centroid.y,(double) object[i].area,mean_color);
      }
    }
  object=(CCObjectInfo *) RelinquishMagickMemory(object);
  return(component_image);
}

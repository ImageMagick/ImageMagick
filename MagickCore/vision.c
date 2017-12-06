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
*/

#include "MagickCore/studio.h"
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/effect.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/matrix.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/morphology.h"
#include "MagickCore/morphology-private.h"
#include "MagickCore/opencl-private.h"
#include "MagickCore/paint.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/resource_.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/token.h"
#include "MagickCore/vision.h"

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
%  uniquely labeled.  The returned connected components image colors member
%  defines the number of unique objects.  Choose from 4 or 8-way connectivity.
%
%  You are responsible for freeing the connected components objects resources
%  with this statement;
%
%    objects = (CCObjectInfo *) RelinquishMagickMemory(objects);
%
%  The format of the ConnectedComponentsImage method is:
%
%      Image *ConnectedComponentsImage(const Image *image,
%        const size_t connectivity,CCObjectInfo **objects,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o connectivity: how many neighbors to visit, choose from 4 or 8.
%
%    o objects: return the attributes of each unique object.
%
%    o exception: return any errors or warnings in this structure.
%
*/

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
  const size_t connectivity,CCObjectInfo **objects,ExceptionInfo *exception)
{
#define ConnectedComponentsImageTag  "ConnectedComponents/Image"

  CacheView
    *image_view,
    *component_view;

  CCObjectInfo
    *object;

  char
    *c;

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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (objects != (CCObjectInfo **) NULL)
    *objects=(CCObjectInfo *) NULL;
  component_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (component_image == (Image *) NULL)
    return((Image *) NULL);
  component_image->depth=MAGICKCORE_QUANTUM_DEPTH;
  if (AcquireImageColormap(component_image,MaxColormapSize,exception) == MagickFalse)
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
    GetPixelInfo(image,&object[i].color);
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
      register const Quantum
        *magick_restrict p;

      register ssize_t
        x;

      if (status == MagickFalse)
        continue;
      p=GetCacheViewVirtualPixels(image_view,0,y-1,image->columns,3,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      p+=GetPixelChannels(image)*image->columns;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        PixelInfo
          pixel,
          target;

        ssize_t
          neighbor_offset,
          obj,
          offset,
          ox,
          oy,
          root;

        /*
          Is neighbor an authentic pixel and a different color than the pixel?
        */
        GetPixelInfoPixel(image,p,&pixel);
        if (((x+dx) < 0) || ((x+dx) >= (ssize_t) image->columns) ||
            ((y+dy) < 0) || ((y+dy) >= (ssize_t) image->rows))
          {
            p+=GetPixelChannels(image);
            continue;
          }
        neighbor_offset=dy*(GetPixelChannels(image)*image->columns)+dx*
          GetPixelChannels(image);
        GetPixelInfoPixel(image,p+neighbor_offset,&target);
        if (IsFuzzyEquivalencePixelInfo(&pixel,&target) == MagickFalse)
          {
            p+=GetPixelChannels(image);
            continue;
          }
        /*
          Resolve this equivalence.
        */
        offset=y*image->columns+x;
        neighbor_offset=dy*image->columns+dx;
        ox=offset;
        status=GetMatrixElement(equivalences,ox,0,&obj);
        while (obj != ox)
        {
          ox=obj;
          status=GetMatrixElement(equivalences,ox,0,&obj);
        }
        oy=offset+neighbor_offset;
        status=GetMatrixElement(equivalences,oy,0,&obj);
        while (obj != oy)
        {
          oy=obj;
          status=GetMatrixElement(equivalences,oy,0,&obj);
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
        status=GetMatrixElement(equivalences,ox,0,&obj);
        while (obj != root)
        {
          status=GetMatrixElement(equivalences,ox,0,&obj);
          status=SetMatrixElement(equivalences,ox,0,&root);
        }
        oy=offset+neighbor_offset;
        status=GetMatrixElement(equivalences,oy,0,&obj);
        while (obj != root)
        {
          status=GetMatrixElement(equivalences,oy,0,&obj);
          status=SetMatrixElement(equivalences,oy,0,&root);
        }
        status=SetMatrixElement(equivalences,y*image->columns+x,0,&root);
        p+=GetPixelChannels(image);
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
    register const Quantum
      *magick_restrict p;

    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(component_view,0,y,component_image->columns,
      1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) component_image->columns; x++)
    {
      ssize_t
        id,
        offset;

      offset=y*image->columns+x;
      status=GetMatrixElement(equivalences,offset,0,&id);
      if (id != offset)
        status=GetMatrixElement(equivalences,id,0,&id);
      else
        {
          id=n++;
          if (id >= (ssize_t) MaxColormapSize)
            break;
        }
      status=SetMatrixElement(equivalences,offset,0,&id);
      if (x < object[id].bounding_box.x)
        object[id].bounding_box.x=x;
      if (x >= (ssize_t) object[id].bounding_box.width)
        object[id].bounding_box.width=(size_t) x;
      if (y < object[id].bounding_box.y)
        object[id].bounding_box.y=y;
      if (y >= (ssize_t) object[id].bounding_box.height)
        object[id].bounding_box.height=(size_t) y;
      object[id].color.red+=QuantumScale*GetPixelRed(image,p);
      object[id].color.green+=QuantumScale*GetPixelGreen(image,p);
      object[id].color.blue+=QuantumScale*GetPixelBlue(image,p);
      if (image->alpha_trait != UndefinedPixelTrait)
        object[id].color.alpha+=QuantumScale*GetPixelAlpha(image,p);
      if (image->colorspace == CMYKColorspace)
        object[id].color.black+=QuantumScale*GetPixelBlack(image,p);
      object[id].centroid.x+=x;
      object[id].centroid.y+=y;
      object[id].area++;
      SetPixelIndex(component_image,(Quantum) id,q);
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(component_image);
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
    object[i].color.red=QuantumRange*(object[i].color.red/object[i].area);
    object[i].color.green=QuantumRange*(object[i].color.green/object[i].area);
    object[i].color.blue=QuantumRange*(object[i].color.blue/object[i].area);
    if (image->alpha_trait != UndefinedPixelTrait)
      object[i].color.alpha=QuantumRange*(object[i].color.alpha/object[i].area);
    if (image->colorspace == CMYKColorspace)
      object[i].color.black=QuantumRange*(object[i].color.black/object[i].area);
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
        Merge object below area threshold.
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
          register const Quantum
            *magick_restrict p;

          register ssize_t
            x;

          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x-1,
            bounding_box.y+y-1,bounding_box.width+2,1,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          for (x=0; x < (ssize_t) bounding_box.width+2; x++)
          {
            j=(ssize_t) GetPixelIndex(component_image,p);
            if (j != i)
              object[j].census++;
            p+=GetPixelChannels(component_image);
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
          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          if (status == MagickFalse)
            continue;
          q=GetCacheViewAuthenticPixels(component_view,bounding_box.x,
            bounding_box.y+y,bounding_box.width,1,exception);
          if (q == (Quantum *) NULL)
            {
              status=MagickFalse;
              continue;
            }
          for (x=0; x < (ssize_t) bounding_box.width; x++)
          {
            if ((ssize_t) GetPixelIndex(component_image,q) == i)
              SetPixelIndex(component_image,(Quantum) id,q);
            q+=GetPixelChannels(component_image);
          }
          if (SyncCacheViewAuthenticPixels(component_view,exception) == MagickFalse)
            status=MagickFalse;
        }
      }
      component_view=DestroyCacheView(component_view);
      (void) SyncImage(component_image,exception);
    }
  artifact=GetImageArtifact(image,"connected-components:mean-color");
  if (IsStringTrue(artifact) != MagickFalse)
    {
      /*
        Replace object with mean color.
      */
      for (i=0; i < (ssize_t) component_image->colors; i++)
        component_image->colormap[i]=object[i].color;
    }
  artifact=GetImageArtifact(image,"connected-components:keep");
  if (artifact != (const char *) NULL)
    {
      /*
        Keep these object (make others transparent).
      */
      for (i=0; i < (ssize_t) component_image->colors; i++)
        object[i].census=0;
      for (c=(char *) artifact; *c != '\0';)
      {
        while ((isspace((int) ((unsigned char) *c)) != 0) || (*c == ','))
          c++;
        first=strtol(c,&c,10);
        if (first < 0)
          first+=(long) component_image->colors;
        last=first;
        while (isspace((int) ((unsigned char) *c)) != 0)
          c++;
        if (*c == '-')
          {
            last=strtol(c+1,&c,10);
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
        component_image->alpha_trait=BlendPixelTrait;
        component_image->colormap[i].alpha=TransparentAlpha;
      }
    }
  artifact=GetImageArtifact(image,"connected-components:remove");
  if (artifact != (const char *) NULL)
    {
      /*
        Remove these object (make them transparent).
      */
      for (c=(char *) artifact; *c != '\0';)
      {
        while ((isspace((int) ((unsigned char) *c)) != 0) || (*c == ','))
          c++;
        first=strtol(c,&c,10);
        if (first < 0)
          first+=(long) component_image->colors;
        last=first;
        while (isspace((int) ((unsigned char) *c)) != 0)
          c++;
        if (*c == '-')
          {
            last=strtol(c+1,&c,10);
            if (last < 0)
              last+=(long) component_image->colors;
          }
        for (step=first > last ? -1 : 1; first != (last+step); first+=step)
        {
          component_image->alpha_trait=BlendPixelTrait;
          component_image->colormap[first].alpha=TransparentAlpha;
        }
      }
    }
  (void) SyncImage(component_image,exception);
  artifact=GetImageArtifact(image,"connected-components:verbose");
  if ((IsStringTrue(artifact) != MagickFalse) ||
      (objects != (CCObjectInfo **) NULL))
    {
      /*
        Report statistics on unique object.
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
        register const Quantum
          *magick_restrict p;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        p=GetCacheViewVirtualPixels(component_view,0,y,
          component_image->columns,1,exception);
        if (p == (const Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) component_image->columns; x++)
        {
          size_t
            id;

          id=GetPixelIndex(component_image,p);
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
          p+=GetPixelChannels(component_image);
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
      if (objects == (CCObjectInfo **) NULL)
        {
          (void) fprintf(stdout,
            "Objects (id: bounding-box centroid area mean-color):\n");
          for (i=0; i < (ssize_t) component_image->colors; i++)
          {
            char
              mean_color[MagickPathExtent];

            if (status == MagickFalse)
              break;
            if (object[i].area <= area_threshold)
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
    }
  if (objects == (CCObjectInfo **) NULL)
    object=(CCObjectInfo *) RelinquishMagickMemory(object);
  else
    *objects=object;
  return(component_image);
}

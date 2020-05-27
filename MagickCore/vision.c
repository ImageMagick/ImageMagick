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
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
    *component_view,
    *image_view,
    *object_view;

  CCObjectInfo
    *object;

  char
    *c;

  const char
    *artifact,
    *metrics[CCMaxMetrics];

  double
    max_threshold,
    min_threshold;

  Image
    *component_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  MatrixInfo
    *equivalences;

  RectangleInfo
    bounding_box;

  register ssize_t
    i;

  size_t
    size;

  ssize_t
    background_id,
    connect4[2][2] = { { -1,  0 }, {  0, -1 } },
    connect8[4][2] = { { -1, -1 }, { -1,  0 }, { -1,  1 }, {  0, -1 } },
    dx,
    dy,
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
  component_image=CloneImage(image,0,0,MagickTrue,exception);
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
  (void) memset(object,0,MaxColormapSize*sizeof(*object));
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
    if (status == MagickFalse)
      continue;
    dx=connectivity > 4 ? connect8[n][1] : connect4[n][1];
    dy=connectivity > 4 ? connect8[n][0] : connect4[n][0];
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
  /*
    Label connected components.
  */
  n=0;
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

        progress++;
        proceed=SetImageProgress(image,ConnectedComponentsImageTag,progress,
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
  background_id=0;
  min_threshold=0.0;
  max_threshold=0.0;
  component_image->colors=(size_t) n;
  for (i=0; i < (ssize_t) component_image->colors; i++)
  {
    object[i].bounding_box.width-=(object[i].bounding_box.x-1);
    object[i].bounding_box.height-=(object[i].bounding_box.y-1);
    object[i].color.red/=(QuantumScale*object[i].area);
    object[i].color.green/=(QuantumScale*object[i].area);
    object[i].color.blue/=(QuantumScale*object[i].area);
    if (image->alpha_trait != UndefinedPixelTrait)
      object[i].color.alpha/=(QuantumScale*object[i].area);
    if (image->colorspace == CMYKColorspace)
      object[i].color.black/=(QuantumScale*object[i].area);
    object[i].centroid.x/=object[i].area;
    object[i].centroid.y/=object[i].area;
    max_threshold+=object[i].area;
    if (object[i].area > object[background_id].area)
      background_id=i;
  }
  max_threshold+=MagickEpsilon;
  n=(-1);
  artifact=GetImageArtifact(image,"connected-components:background-id");
  if (artifact != (const char *) NULL)
    background_id=(ssize_t) StringToDouble(artifact,(char **) NULL);
  artifact=GetImageArtifact(image,"connected-components:area-threshold");
  if (artifact != (const char *) NULL)
    {
      /*
        Merge any object not within the min and max area threshold.
      */
      (void) sscanf(artifact,"%lf%*[ -]%lf",&min_threshold,&max_threshold);
      for (i=0; i < (ssize_t) component_image->colors; i++)
        if (((object[i].area < min_threshold) ||
             (object[i].area >= max_threshold)) && (i != background_id))
          object[i].merge=MagickTrue;
    }
  artifact=GetImageArtifact(image,"connected-components:keep-colors");
  if (artifact != (const char *) NULL)
    {
      register const char
        *p;

      /*
        Keep selected objects based on color, merge others.
      */
      for (i=0; i < (ssize_t) component_image->colors; i++)
        object[i].merge=MagickTrue;
      for (p=artifact;  ; )
      {
        char
          color[MagickPathExtent];

        PixelInfo
          pixel;

        register const char
          *q;

        for (q=p; *q != '\0'; q++)
          if (*q == ';')
            break;
        (void) CopyMagickString(color,p,(size_t) MagickMin(q-p+1,
          MagickPathExtent));
        (void) QueryColorCompliance(color,AllCompliance,&pixel,exception);
        for (i=0; i < (ssize_t) component_image->colors; i++)
          if (IsFuzzyEquivalencePixelInfo(&object[i].color,&pixel) != MagickFalse)
            object[i].merge=MagickFalse;
        if (*q == '\0')
          break;
        p=q+1;
      }
    }
  artifact=GetImageArtifact(image,"connected-components:keep-ids");
  if (artifact == (const char *) NULL)
    artifact=GetImageArtifact(image,"connected-components:keep");
  if (artifact != (const char *) NULL)
    {
      /*
        Keep selected objects based on id, merge others.
      */
      for (i=0; i < (ssize_t) component_image->colors; i++)
        object[i].merge=MagickTrue;
      for (c=(char *) artifact; *c != '\0'; )
      {
        while ((isspace((int) ((unsigned char) *c)) != 0) || (*c == ','))
          c++;
        first=(ssize_t) strtol(c,&c,10);
        if (first < 0)
          first+=(ssize_t) component_image->colors;
        last=first;
        while (isspace((int) ((unsigned char) *c)) != 0)
          c++;
        if (*c == '-')
          {
            last=(ssize_t) strtol(c+1,&c,10);
            if (last < 0)
              last+=(ssize_t) component_image->colors;
          }
        step=(ssize_t) (first > last ? -1 : 1);
        for ( ; first != (last+step); first+=step)
          object[first].merge=MagickFalse;
      }
    }
  artifact=GetImageArtifact(image,"connected-components:keep-top");
  if (artifact != (const char *) NULL)
    {
      CCObjectInfo
        *top_objects;

      ssize_t
        top_ids;

      /*
        Keep top objects.
      */
      top_ids=(ssize_t) StringToDouble(artifact,(char **) NULL);
      top_objects=(CCObjectInfo *) AcquireQuantumMemory(component_image->colors,
        sizeof(*top_objects));
      if (top_objects == (CCObjectInfo *) NULL)
        {
          object=(CCObjectInfo *) RelinquishMagickMemory(object);
          component_image=DestroyImage(component_image);
          ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
        }
      (void) memcpy(top_objects,object,component_image->colors*sizeof(*object));
      qsort((void *) top_objects,component_image->colors,sizeof(*top_objects),
        CCObjectInfoCompare);
      for (i=top_ids+1; i < (ssize_t) component_image->colors; i++)
        object[top_objects[i].id].merge=MagickTrue;
      top_objects=(CCObjectInfo *) RelinquishMagickMemory(top_objects);
    }
  artifact=GetImageArtifact(image,"connected-components:remove-colors");
  if (artifact != (const char *) NULL)
    {
      register const char
        *p;

      /*
        Remove selected objects based on color, keep others.
      */
      for (p=artifact;  ; )
      {
        char
          color[MagickPathExtent];

        PixelInfo
          pixel;

        register const char
          *q;

        for (q=p; *q != '\0'; q++)
          if (*q == ';')
            break;
        (void) CopyMagickString(color,p,(size_t) MagickMin(q-p+1,
          MagickPathExtent));
        (void) QueryColorCompliance(color,AllCompliance,&pixel,exception);
        for (i=0; i < (ssize_t) component_image->colors; i++)
          if (IsFuzzyEquivalencePixelInfo(&object[i].color,&pixel) != MagickFalse)
            object[i].merge=MagickTrue;
        if (*q == '\0')
          break;
        p=q+1;
      }
    }
  artifact=GetImageArtifact(image,"connected-components:remove-ids");
  if (artifact == (const char *) NULL)
    artifact=GetImageArtifact(image,"connected-components:remove");
  if (artifact != (const char *) NULL)
    for (c=(char *) artifact; *c != '\0'; )
    {
      /*
        Remove selected objects based on id, keep others.
      */
      while ((isspace((int) ((unsigned char) *c)) != 0) || (*c == ','))
        c++;
      first=(ssize_t) strtol(c,&c,10);
      if (first < 0)
        first+=(ssize_t) component_image->colors;
      last=first;
      while (isspace((int) ((unsigned char) *c)) != 0)
        c++;
      if (*c == '-')
        {
          last=(ssize_t) strtol(c+1,&c,10);
          if (last < 0)
            last+=(ssize_t) component_image->colors;
        }
      step=(ssize_t) (first > last ? -1 : 1);
      for ( ; first != (last+step); first+=step)
        object[first].merge=MagickTrue;
    }
  artifact=GetImageArtifact(image,"connected-components:perimeter-threshold");
  if (artifact != (const char *) NULL)
    {
      /*
        Merge any object not within the min and max perimeter threshold.
      */
      (void) sscanf(artifact,"%lf%*[ -]%lf",&min_threshold,&max_threshold);
      metrics[++n]="perimeter";
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(dynamic) shared(status) \
        magick_number_threads(component_image,component_image,component_image->colors,1)
#endif
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        CacheView
          *component_view;

        RectangleInfo
          bounding_box;

        size_t
          pattern[4] = { 1, 0, 0, 0 };

        ssize_t
          y;

        /*
          Compute perimeter of each object.
        */
        if (status == MagickFalse)
          continue;
        component_view=AcquireAuthenticCacheView(component_image,exception);
        bounding_box=object[i].bounding_box;
        for (y=(-1); y < (ssize_t) bounding_box.height+1; y++)
        {
          register const Quantum
            *magick_restrict p;

          register ssize_t
            x;

          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x-1,
            bounding_box.y+y,bounding_box.width+2,2,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (x=(-1); x < (ssize_t) bounding_box.width+1; x++)
          {
            Quantum
              pixels[4];

            register ssize_t
              v;

            size_t
              foreground;

            /*
              An Algorithm for Calculating Objects’ Shape Features in Binary
              Images, Lifeng He, Yuyan Chao.
            */
            foreground=0;
            for (v=0; v < 2; v++)
            {
              register ssize_t
                u;

              for (u=0; u < 2; u++)
              {
                ssize_t
                  offset;

                offset=v*(bounding_box.width+2)*
                  GetPixelChannels(component_image)+u*
                  GetPixelChannels(component_image);
                pixels[2*v+u]=GetPixelIndex(component_image,p+offset);
                if ((ssize_t) pixels[2*v+u] == i)
                  foreground++;
              }
            }
            if (foreground == 1)
              pattern[1]++;
            else
              if (foreground == 2)
                {
                  if ((((ssize_t) pixels[0] == i) &&
                       ((ssize_t) pixels[3] == i)) ||
                      (((ssize_t) pixels[1] == i) &&
                       ((ssize_t) pixels[2] == i)))
                    pattern[0]++;  /* diagonal */
                  else
                    pattern[2]++;
                }
              else
                if (foreground == 3)
                  pattern[3]++;
            p+=GetPixelChannels(component_image);
          }
        }
        component_view=DestroyCacheView(component_view);
        object[i].metric[n]=ceil(MagickSQ1_2*pattern[1]+1.0*pattern[2]+
          MagickSQ1_2*pattern[3]+MagickSQ2*pattern[0]-0.5);
      }
      for (i=0; i < (ssize_t) component_image->colors; i++)
        if (((object[i].metric[n] < min_threshold) ||
             (object[i].metric[n] >= max_threshold)) && (i != background_id))
          object[i].merge=MagickTrue;
    }
  artifact=GetImageArtifact(image,"connected-components:circularity-threshold");
  if (artifact != (const char *) NULL)
    {
      /*
        Merge any object not within the min and max circularity threshold.
      */
      (void) sscanf(artifact,"%lf%*[ -]%lf",&min_threshold,&max_threshold);
      metrics[++n]="circularity";
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(dynamic) shared(status) \
        magick_number_threads(component_image,component_image,component_image->colors,1)
#endif
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        CacheView
          *component_view;

        RectangleInfo
          bounding_box;

        size_t
          pattern[4] = { 1, 0, 0, 0 };

        ssize_t
          y;

        /*
          Compute perimeter of each object.
        */
        if (status == MagickFalse)
          continue;
        component_view=AcquireAuthenticCacheView(component_image,exception);
        bounding_box=object[i].bounding_box;
        for (y=(-1); y < (ssize_t) bounding_box.height; y++)
        {
          register const Quantum
            *magick_restrict p;

          register ssize_t
            x;

          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x-1,
            bounding_box.y+y,bounding_box.width+2,2,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (x=(-1); x < (ssize_t) bounding_box.width; x++)
          {
            Quantum
              pixels[4];

            register ssize_t
              v;

            size_t
              foreground;

            /*
              An Algorithm for Calculating Objects’ Shape Features in Binary
              Images, Lifeng He, Yuyan Chao.
            */
            foreground=0;
            for (v=0; v < 2; v++)
            {
              register ssize_t
                u;

              for (u=0; u < 2; u++)
              {
                ssize_t
                  offset;

                offset=v*(bounding_box.width+2)*
                  GetPixelChannels(component_image)+u*
                  GetPixelChannels(component_image);
                pixels[2*v+u]=GetPixelIndex(component_image,p+offset);
                if ((ssize_t) pixels[2*v+u] == i)
                  foreground++;
              }
            }
            if (foreground == 1)
              pattern[1]++;
            else
              if (foreground == 2)
                {
                  if ((((ssize_t) pixels[0] == i) &&
                       ((ssize_t) pixels[3] == i)) ||
                      (((ssize_t) pixels[1] == i) &&
                       ((ssize_t) pixels[2] == i)))
                    pattern[0]++;  /* diagonal */
                  else
                    pattern[2]++;
                }
              else
                if (foreground == 3)
                  pattern[3]++;
            p+=GetPixelChannels(component_image);
          }
        }
        component_view=DestroyCacheView(component_view);
        object[i].metric[n]=ceil(MagickSQ1_2*pattern[1]+1.0*pattern[2]+
          MagickSQ1_2*pattern[3]+MagickSQ2*pattern[0]-0.5);
        object[i].metric[n]=4.0*MagickPI*object[i].area/(object[i].metric[n]*
          object[i].metric[n]);
      }
      for (i=0; i < (ssize_t) component_image->colors; i++)
        if (((object[i].metric[n] < min_threshold) ||
             (object[i].metric[n] >= max_threshold)) && (i != background_id))
          object[i].merge=MagickTrue;
    }
  artifact=GetImageArtifact(image,"connected-components:diameter-threshold");
  if (artifact != (const char *) NULL)
    {
      /*
        Merge any object not within the min and max diameter threshold.
      */
      (void) sscanf(artifact,"%lf%*[ -]%lf",&min_threshold,&max_threshold);
      metrics[++n]="diameter";
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        object[i].metric[n]=ceil(sqrt(4.0*object[i].area/MagickPI)-0.5);
        if (((object[i].metric[n] < min_threshold) ||
             (object[i].metric[n] >= max_threshold)) && (i != background_id))
          object[i].merge=MagickTrue;
      }
    }
  artifact=GetImageArtifact(image,"connected-components:major-axis-threshold");
  if (artifact != (const char *) NULL)
    {
      /*
        Merge any object not within the min and max ellipse major threshold.
      */
      (void) sscanf(artifact,"%lf%*[ -]%lf",&min_threshold,&max_threshold);
      metrics[++n]="major-axis";
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(dynamic) shared(status) \
        magick_number_threads(component_image,component_image,component_image->colors,1)
#endif
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        CacheView
          *component_view;

        double
          M00 = 0.0,
          M01 = 0.0,
          M02 = 0.0,
          M10 = 0.0,
          M11 = 0.0,
          M20 = 0.0;

        PointInfo
          centroid = { 0.0, 0.0 };

        RectangleInfo
          bounding_box;

        register const Quantum
          *magick_restrict p;

        register ssize_t
          x;

        ssize_t
          y;

        /*
          Compute ellipse major axis of each object.
        */
        if (status == MagickFalse)
          continue;
        component_view=AcquireAuthenticCacheView(component_image,exception);
        bounding_box=object[i].bounding_box;
        for (y=0; y < (ssize_t) bounding_box.height; y++)
        {
          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x,
            bounding_box.y+y,bounding_box.width,1,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (x=0; x < (ssize_t) bounding_box.width; x++)
          {
            if ((ssize_t) GetPixelIndex(component_image,p) == i)
              {
                M00++;
                M10+=x;
                M01+=y;
              }
            p+=GetPixelChannels(component_image);
          }
        }
        centroid.x=M10*PerceptibleReciprocal(M00);
        centroid.y=M01*PerceptibleReciprocal(M00);
        for (y=0; y < (ssize_t) bounding_box.height; y++)
        {
          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x,
            bounding_box.y+y,bounding_box.width,1,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (x=0; x < (ssize_t) bounding_box.width; x++)
          {
            if ((ssize_t) GetPixelIndex(component_image,p) == i)
              {
                M11+=(x-centroid.x)*(y-centroid.y);
                M20+=(x-centroid.x)*(x-centroid.x);
                M02+=(y-centroid.y)*(y-centroid.y);
              }
            p+=GetPixelChannels(component_image);
          }
        }
        component_view=DestroyCacheView(component_view);
        object[i].metric[n]=sqrt((2.0*PerceptibleReciprocal(M00))*((M20+M02)+
          sqrt(4.0*M11*M11+(M20-M02)*(M20-M02))));
      }
      for (i=0; i < (ssize_t) component_image->colors; i++)
        if (((object[i].metric[n] < min_threshold) ||
             (object[i].metric[n] >= max_threshold)) && (i != background_id))
          object[i].merge=MagickTrue;
    }
  artifact=GetImageArtifact(image,"connected-components:minor-axis-threshold");
  if (artifact != (const char *) NULL)
    {
      /*
        Merge any object not within the min and max ellipse minor threshold.
      */
      (void) sscanf(artifact,"%lf%*[ -]%lf",&min_threshold,&max_threshold);
      metrics[++n]="minor-axis";
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(dynamic) shared(status) \
        magick_number_threads(component_image,component_image,component_image->colors,1)
#endif
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        CacheView
          *component_view;

        double
          M00 = 0.0,
          M01 = 0.0,
          M02 = 0.0,
          M10 = 0.0,
          M11 = 0.0,
          M20 = 0.0;

        PointInfo
          centroid = { 0.0, 0.0 };

        RectangleInfo
          bounding_box;

        register const Quantum
          *magick_restrict p;

        register ssize_t
          x;

        ssize_t
          y;

        /*
          Compute ellipse major axis of each object.
        */
        if (status == MagickFalse)
          continue;
        component_view=AcquireAuthenticCacheView(component_image,exception);
        bounding_box=object[i].bounding_box;
        for (y=0; y < (ssize_t) bounding_box.height; y++)
        {
          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x,
            bounding_box.y+y,bounding_box.width,1,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (x=0; x < (ssize_t) bounding_box.width; x++)
          {
            if ((ssize_t) GetPixelIndex(component_image,p) == i)
              {
                M00++;
                M10+=x;
                M01+=y;
              }
            p+=GetPixelChannels(component_image);
          }
        }
        centroid.x=M10*PerceptibleReciprocal(M00);
        centroid.y=M01*PerceptibleReciprocal(M00);
        for (y=0; y < (ssize_t) bounding_box.height; y++)
        {
          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x,
            bounding_box.y+y,bounding_box.width,1,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (x=0; x < (ssize_t) bounding_box.width; x++)
          {
            if ((ssize_t) GetPixelIndex(component_image,p) == i)
              {
                M11+=(x-centroid.x)*(y-centroid.y);
                M20+=(x-centroid.x)*(x-centroid.x);
                M02+=(y-centroid.y)*(y-centroid.y);
              }
            p+=GetPixelChannels(component_image);
          }
        }
        component_view=DestroyCacheView(component_view);
        object[i].metric[n]=sqrt((2.0*PerceptibleReciprocal(M00))*((M20+M02)-
          sqrt(4.0*M11*M11+(M20-M02)*(M20-M02))));
      }
      for (i=0; i < (ssize_t) component_image->colors; i++)
        if (((object[i].metric[n] < min_threshold) ||
             (object[i].metric[n] >= max_threshold)) && (i != background_id))
          object[i].merge=MagickTrue;
    }
  artifact=GetImageArtifact(image,
    "connected-components:eccentricity-threshold");
  if (artifact != (const char *) NULL)
    {
      /*
        Merge any object not within the min and max eccentricity threshold.
      */
      (void) sscanf(artifact,"%lf%*[ -]%lf",&min_threshold,&max_threshold);
      metrics[++n]="eccentricy";
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(dynamic) shared(status) \
        magick_number_threads(component_image,component_image,component_image->colors,1)
#endif
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        CacheView
          *component_view;

        double
          M00 = 0.0,
          M01 = 0.0,
          M02 = 0.0,
          M10 = 0.0,
          M11 = 0.0,
          M20 = 0.0;

        PointInfo
          centroid = { 0.0, 0.0 },
          ellipse_axis = { 0.0, 0.0 };

        RectangleInfo
          bounding_box;

        register const Quantum
          *magick_restrict p;

        register ssize_t
          x;

        ssize_t
          y;

        /*
          Compute eccentricity of each object.
        */
        if (status == MagickFalse)
          continue;
        component_view=AcquireAuthenticCacheView(component_image,exception);
        bounding_box=object[i].bounding_box;
        for (y=0; y < (ssize_t) bounding_box.height; y++)
        {
          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x,
            bounding_box.y+y,bounding_box.width,1,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (x=0; x < (ssize_t) bounding_box.width; x++)
          {
            if ((ssize_t) GetPixelIndex(component_image,p) == i)
              {
                M00++;
                M10+=x;
                M01+=y;
              }
            p+=GetPixelChannels(component_image);
          }
        }
        centroid.x=M10*PerceptibleReciprocal(M00);
        centroid.y=M01*PerceptibleReciprocal(M00);
        for (y=0; y < (ssize_t) bounding_box.height; y++)
        {
          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x,
            bounding_box.y+y,bounding_box.width,1,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (x=0; x < (ssize_t) bounding_box.width; x++)
          {
            if ((ssize_t) GetPixelIndex(component_image,p) == i)
              {
                M11+=(x-centroid.x)*(y-centroid.y);
                M20+=(x-centroid.x)*(x-centroid.x);
                M02+=(y-centroid.y)*(y-centroid.y);
              }
            p+=GetPixelChannels(component_image);
          }
        }
        component_view=DestroyCacheView(component_view);
        ellipse_axis.x=sqrt((2.0*PerceptibleReciprocal(M00))*((M20+M02)+
          sqrt(4.0*M11*M11+(M20-M02)*(M20-M02))));
        ellipse_axis.y=sqrt((2.0*PerceptibleReciprocal(M00))*((M20+M02)-
          sqrt(4.0*M11*M11+(M20-M02)*(M20-M02))));
        object[i].metric[n]=sqrt(1.0-(ellipse_axis.y*ellipse_axis.y*
          PerceptibleReciprocal(ellipse_axis.x*ellipse_axis.x)));
      }
      for (i=0; i < (ssize_t) component_image->colors; i++)
        if (((object[i].metric[n] < min_threshold) ||
             (object[i].metric[n] >= max_threshold)) && (i != background_id))
          object[i].merge=MagickTrue;
    }
  artifact=GetImageArtifact(image,"connected-components:angle-threshold");
  if (artifact != (const char *) NULL)
    {
      /*
        Merge any object not within the min and max ellipse angle threshold.
      */
      (void) sscanf(artifact,"%lf%*[ -]%lf",&min_threshold,&max_threshold);
      metrics[++n]="angle";
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(dynamic) shared(status) \
        magick_number_threads(component_image,component_image,component_image->colors,1)
#endif
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        CacheView
          *component_view;

        double
          M00 = 0.0,
          M01 = 0.0,
          M02 = 0.0,
          M10 = 0.0,
          M11 = 0.0,
          M20 = 0.0;

        PointInfo
          centroid = { 0.0, 0.0 };

        RectangleInfo
          bounding_box;

        register const Quantum
          *magick_restrict p;

        register ssize_t
          x;

        ssize_t
          y;

        /*
          Compute ellipse angle of each object.
        */
        if (status == MagickFalse)
          continue;
        component_view=AcquireAuthenticCacheView(component_image,exception);
        bounding_box=object[i].bounding_box;
        for (y=0; y < (ssize_t) bounding_box.height; y++)
        {
          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x,
            bounding_box.y+y,bounding_box.width,1,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (x=0; x < (ssize_t) bounding_box.width; x++)
          {
            if ((ssize_t) GetPixelIndex(component_image,p) == i)
              {
                M00++;
                M10+=x;
                M01+=y;
              }
            p+=GetPixelChannels(component_image);
          }
        }
        centroid.x=M10*PerceptibleReciprocal(M00);
        centroid.y=M01*PerceptibleReciprocal(M00);
        for (y=0; y < (ssize_t) bounding_box.height; y++)
        {
          if (status == MagickFalse)
            continue;
          p=GetCacheViewVirtualPixels(component_view,bounding_box.x,
            bounding_box.y+y,bounding_box.width,1,exception);
          if (p == (const Quantum *) NULL)
            {
              status=MagickFalse;
              break;
            }
          for (x=0; x < (ssize_t) bounding_box.width; x++)
          {
            if ((ssize_t) GetPixelIndex(component_image,p) == i)
              {
                M11+=(x-centroid.x)*(y-centroid.y);
                M20+=(x-centroid.x)*(x-centroid.x);
                M02+=(y-centroid.y)*(y-centroid.y);
              }
            p+=GetPixelChannels(component_image);
          }
        }
        component_view=DestroyCacheView(component_view);
        object[i].metric[n]=RadiansToDegrees(1.0/2.0*atan(2.0*M11*
          PerceptibleReciprocal(M20-M02)));
        if (fabs(M11) < 0.0)
           {
             if ((fabs(M20-M02) >= 0.0) && ((M20-M02) < 0.0))
               object[i].metric[n]+=90.0;
           }
         else
           if (M11 < 0.0)
             {
               if (fabs(M20-M02) >= 0.0)
                 {
                   if ((M20-M02) < 0.0)
                     object[i].metric[n]+=90.0;
                   else
                     object[i].metric[n]+=180.0;
                 }
             }
           else
             if ((fabs(M20-M02) >= 0.0) && ((M20-M02) < 0.0))
               object[i].metric[n]+=90.0;
      }
      for (i=0; i < (ssize_t) component_image->colors; i++)
        if (((object[i].metric[n] < min_threshold) ||
             (object[i].metric[n] >= max_threshold)) && (i != background_id))
          object[i].merge=MagickTrue;
    }
  /*
    Merge any object not within the min and max area threshold.
  */
  component_view=AcquireAuthenticCacheView(component_image,exception);
  object_view=AcquireVirtualCacheView(component_image,exception);
  for (i=0; i < (ssize_t) component_image->colors; i++)
  {
    register ssize_t
      j;

    size_t
      id;

    if (status == MagickFalse)
      continue;
    if ((object[i].merge == MagickFalse) || (i == background_id))
      continue;  /* keep object */
    /*
      Merge this object.
    */
    for (j=0; j < (ssize_t) component_image->colors; j++)
      object[j].census=0;
    bounding_box=object[i].bounding_box;
    for (y=0; y < (ssize_t) bounding_box.height; y++)
    {
      register const Quantum
        *magick_restrict p;

      register ssize_t
        x;

      if (status == MagickFalse)
        continue;
      p=GetCacheViewVirtualPixels(component_view,bounding_box.x,
        bounding_box.y+y,bounding_box.width,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      for (x=0; x < (ssize_t) bounding_box.width; x++)
      {
        register ssize_t
          n;

        if (status == MagickFalse)
          continue;
        j=(ssize_t) GetPixelIndex(component_image,p);
        if (j == i)
          for (n=0; n < (ssize_t) (connectivity > 4 ? 4 : 2); n++)
          {
            register const Quantum
              *p;

            /*
              Compute area of adjacent objects.
            */
            if (status == MagickFalse)
              continue;
            dx=connectivity > 4 ? connect8[n][1] : connect4[n][1];
            dy=connectivity > 4 ? connect8[n][0] : connect4[n][0];
            p=GetCacheViewVirtualPixels(object_view,bounding_box.x+x+dx,
              bounding_box.y+y+dy,1,1,exception);
            if (p == (const Quantum *) NULL)
              {
                status=MagickFalse;
                break;
              }
            j=(ssize_t) GetPixelIndex(component_image,p);
            if (j != i)
              object[j].census++;
          }
        p+=GetPixelChannels(component_image);
      }
    }
    /*
      Merge with object of greatest adjacent area.
    */
    id=0;
    for (j=1; j < (ssize_t) component_image->colors; j++)
      if (object[j].census > object[id].census)
        id=(size_t) j;
    object[id].area+=object[i].area;
    object[i].area=0.0;
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
  object_view=DestroyCacheView(object_view);
  component_view=DestroyCacheView(component_view);
  artifact=GetImageArtifact(image,"connected-components:mean-color");
  if (IsStringTrue(artifact) != MagickFalse)
    {
      /*
        Replace object with mean color.
      */
      for (i=0; i < (ssize_t) component_image->colors; i++)
        component_image->colormap[i]=object[i].color;
    }
  (void) SyncImage(component_image,exception);
  artifact=GetImageArtifact(image,"connected-components:verbose");
  if ((IsStringTrue(artifact) != MagickFalse) ||
      (objects != (CCObjectInfo **) NULL))
    {
      /*
        Report statistics on each unique object.
      */
      for (i=0; i < (ssize_t) component_image->colors; i++)
      {
        object[i].bounding_box.width=0;
        object[i].bounding_box.height=0;
        object[i].bounding_box.x=(ssize_t) component_image->columns;
        object[i].bounding_box.y=(ssize_t) component_image->rows;
        object[i].centroid.x=0;
        object[i].centroid.y=0;
        object[i].census=object[i].area == 0.0 ? 0.0 : 1.0;
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
        p=GetCacheViewVirtualPixels(component_view,0,y,component_image->columns,
          1,exception);
        if (p == (const Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) component_image->columns; x++)
        {
          size_t
            id;

          id=(size_t) GetPixelIndex(component_image,p);
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
          register ssize_t
            j;

          artifact=GetImageArtifact(image,
            "connected-components:exclude-header");
          if (IsStringTrue(artifact) == MagickFalse)
            {
              (void) fprintf(stdout,
                "Objects (id: bounding-box centroid area mean-color");
              for (j=0; j <= n; j++)
                (void) fprintf(stdout," %s",metrics[j]);
              (void) fprintf(stdout,"):\n");
            }
          for (i=0; i < (ssize_t) component_image->colors; i++)
            if (object[i].census > 0.0)
              {
                char
                  mean_color[MagickPathExtent];

                GetColorTuple(&object[i].color,MagickFalse,mean_color);
                (void) fprintf(stdout,
                  "  %.20g: %.20gx%.20g%+.20g%+.20g %.1f,%.1f %.*g %s",
                  (double) object[i].id,(double) object[i].bounding_box.width,
                  (double) object[i].bounding_box.height,(double)
                  object[i].bounding_box.x,(double) object[i].bounding_box.y,
                  object[i].centroid.x,object[i].centroid.y,
                  GetMagickPrecision(),(double) object[i].area,mean_color);
                for (j=0; j <= n; j++)
                  (void) fprintf(stdout," %.*g",GetMagickPrecision(),
                    object[i].metric[j]);
                (void) fprintf(stdout,"\n");
              }
        }
    }
  if (objects == (CCObjectInfo **) NULL)
    object=(CCObjectInfo *) RelinquishMagickMemory(object);
  else
    *objects=object;
  return(component_image);
}

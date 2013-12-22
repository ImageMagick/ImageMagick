/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%      H   H   IIIII   SSSSS  TTTTT   OOO    GGGG  RRRR    AAA   M   M        %
%      H   H     I     SS       T    O   O  G      R   R  A   A  MM MM        %
%      HHHHH     I      SSS     T    O   O  G  GG  RRRR   AAAAA  M M M        %
%      H   H     I        SS    T    O   O  G   G  R R    A   A  M   M        %
%      H   H   IIIII   SSSSS    T     OOO    GGG   R  R   A   A  M   M        %
%                                                                             %
%                                                                             %
%                        MagickCore Histogram Methods                         %
%                                                                             %
%                              Software Design                                %
%                              Anthony Thyssen                                %
%                               Fred Weinhaus                                 %
%                                August 2009                                  %
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
#include "magick/studio.h"
#include "magick/cache-view.h"
#include "magick/color-private.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/histogram.h"
#include "magick/image.h"
#include "magick/list.h"
#include "magick/memory_.h"
#include "magick/monitor-private.h"
#include "magick/pixel-private.h"
#include "magick/prepress.h"
#include "magick/quantize.h"
#include "magick/registry.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/statistic.h"
#include "magick/string_.h"

/*
  Define declarations.
*/
#define MaxTreeDepth  8
#define NodesInAList  1536

/*
  Typedef declarations.
*/
typedef struct _NodeInfo
{
  struct _NodeInfo
    *child[16];

  ColorPacket
    *list;

  MagickSizeType
    number_unique;

  size_t
    level;
} NodeInfo;

typedef struct _Nodes
{
  NodeInfo
    nodes[NodesInAList];

  struct _Nodes
    *next;
} Nodes;

typedef struct _CubeInfo
{
  NodeInfo
    *root;

  ssize_t
    x;

  MagickOffsetType
    progress;

  size_t
    colors,
    free_nodes;

  NodeInfo
    *node_info;

  Nodes
    *node_queue;
} CubeInfo;

/*
  Forward declarations.
*/
static CubeInfo
  *GetCubeInfo(void);

static NodeInfo
  *GetNodeInfo(CubeInfo *,const size_t);

static void
  DestroyColorCube(const Image *,NodeInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l a s s i f y I m a g e C o l o r s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClassifyImageColors() builds a populated CubeInfo tree for the specified
%  image.  The returned tree should be deallocated using DestroyCubeInfo()
%  once it is no longer needed.
%
%  The format of the ClassifyImageColors() method is:
%
%      CubeInfo *ClassifyImageColors(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t ColorToNodeId(const Image *image,
  const MagickPixelPacket *pixel,size_t index)
{
  size_t
    id;

  id=(size_t) (
    ((ScaleQuantumToChar(ClampToQuantum(pixel->red)) >> index) & 0x01) |
    ((ScaleQuantumToChar(ClampToQuantum(pixel->green)) >> index) & 0x01) << 1 |
    ((ScaleQuantumToChar(ClampToQuantum(pixel->blue)) >> index) & 0x01) << 2);
  if (image->matte != MagickFalse)
    id|=((ScaleQuantumToChar(ClampToQuantum(pixel->opacity)) >> index) &
      0x01) << 3;
  return(id);
}

static CubeInfo *ClassifyImageColors(const Image *image,
  ExceptionInfo *exception)
{
#define EvaluateImageTag  "  Compute image colors...  "

  CacheView
    *image_view;

  CubeInfo
    *cube_info;

  MagickBooleanType
    proceed;

  MagickPixelPacket
    pixel,
    target;

  NodeInfo
    *node_info;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register size_t
    id,
    index,
    level;

  register ssize_t
    i,
    x;

  ssize_t
    y;

  /*
    Initialize color description tree.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cube_info=GetCubeInfo();
  if (cube_info == (CubeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(cube_info);
    }
  GetMagickPixelPacket(image,&pixel);
  GetMagickPixelPacket(image,&target);
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      /*
        Start at the root and proceed level by level.
      */
      node_info=cube_info->root;
      index=MaxTreeDepth-1;
      for (level=1; level < MaxTreeDepth; level++)
      {
        SetMagickPixelPacket(image,p,indexes+x,&pixel);
        id=ColorToNodeId(image,&pixel,index);
        if (node_info->child[id] == (NodeInfo *) NULL)
          {
            node_info->child[id]=GetNodeInfo(cube_info,level);
            if (node_info->child[id] == (NodeInfo *) NULL)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed","`%s'",
                  image->filename);
                return(0);
              }
          }
        node_info=node_info->child[id];
        index--;
      }
      for (i=0; i < (ssize_t) node_info->number_unique; i++)
      {
        SetMagickPixelPacket(image,&node_info->list[i].pixel,
          &node_info->list[i].index,&target);
        if (IsMagickColorEqual(&pixel,&target) != MagickFalse)
          break;
      }
      if (i < (ssize_t) node_info->number_unique)
        node_info->list[i].count++;
      else
        {
          if (node_info->number_unique == 0)
            node_info->list=(ColorPacket *) AcquireMagickMemory(
              sizeof(*node_info->list));
          else
            node_info->list=(ColorPacket *) ResizeQuantumMemory(node_info->list,
              (size_t) (i+1),sizeof(*node_info->list));
          if (node_info->list == (ColorPacket *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'",
                image->filename);
              return(0);
            }
          node_info->list[i].pixel=(*p);
          if ((image->colorspace == CMYKColorspace) ||
              (image->storage_class == PseudoClass))
            node_info->list[i].index=GetPixelIndex(indexes+x);
          node_info->list[i].count=1;
          node_info->number_unique++;
          cube_info->colors++;
        }
      p++;
    }
    proceed=SetImageProgress(image,EvaluateImageTag,(MagickOffsetType) y,
      image->rows);
    if (proceed == MagickFalse)
      break;
  }
  image_view=DestroyCacheView(image_view);
  return(cube_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e f i n e I m a g e H i s t o g r a m                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefineImageHistogram() traverses the color cube tree and notes each colormap
%  entry.  A colormap entry is any node in the color cube tree where the
%  of unique colors is not zero.
%
%  The format of the DefineImageHistogram method is:
%
%      DefineImageHistogram(const Image *image,NodeInfo *node_info,
%        ColorPacket **unique_colors)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o node_info: the address of a structure of type NodeInfo which points to a
%      node in the color cube tree that is to be pruned.
%
%    o histogram: the image histogram.
%
*/
static void DefineImageHistogram(const Image *image,NodeInfo *node_info,
  ColorPacket **histogram)
{
  register ssize_t
    i;

  size_t
    number_children;

  /*
    Traverse any children.
  */
  number_children=image->matte == MagickFalse ? 8UL : 16UL;
  for (i=0; i < (ssize_t) number_children; i++)
    if (node_info->child[i] != (NodeInfo *) NULL)
      DefineImageHistogram(image,node_info->child[i],histogram);
  if (node_info->level == (MaxTreeDepth-1))
    {
      register ColorPacket
        *p;

      p=node_info->list;
      for (i=0; i < (ssize_t) node_info->number_unique; i++)
      {
        (*histogram)->pixel=p->pixel;
        (*histogram)->index=p->index;
        (*histogram)->count=p->count;
        (*histogram)++;
        p++;
      }
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y C u b e I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCubeInfo() deallocates memory associated with a CubeInfo structure.
%
%  The format of the DestroyCubeInfo method is:
%
%      DestroyCubeInfo(const Image *image,CubeInfo *cube_info)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o cube_info: the address of a structure of type CubeInfo.
%
*/
static CubeInfo *DestroyCubeInfo(const Image *image,CubeInfo *cube_info)
{
  register Nodes
    *nodes;

  /*
    Release color cube tree storage.
  */
  DestroyColorCube(image,cube_info->root);
  do
  {
    nodes=cube_info->node_queue->next;
    cube_info->node_queue=(Nodes *)
      RelinquishMagickMemory(cube_info->node_queue);
    cube_info->node_queue=nodes;
  } while (cube_info->node_queue != (Nodes *) NULL);
  return((CubeInfo *) RelinquishMagickMemory(cube_info));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  D e s t r o y C o l o r C u b e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyColorCube() traverses the color cube tree and frees the list of
%  unique colors.
%
%  The format of the DestroyColorCube method is:
%
%      void DestroyColorCube(const Image *image,const NodeInfo *node_info)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o node_info: the address of a structure of type NodeInfo which points to a
%      node in the color cube tree that is to be pruned.
%
*/
static void DestroyColorCube(const Image *image,NodeInfo *node_info)
{
  register ssize_t
    i;

  size_t
    number_children;

  /*
    Traverse any children.
  */
  number_children=image->matte == MagickFalse ? 8UL : 16UL;
  for (i=0; i < (ssize_t) number_children; i++)
    if (node_info->child[i] != (NodeInfo *) NULL)
      DestroyColorCube(image,node_info->child[i]);
  if (node_info->list != (ColorPacket *) NULL)
    node_info->list=(ColorPacket *) RelinquishMagickMemory(node_info->list);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t C u b e I n f o                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCubeInfo() initializes the CubeInfo data structure.
%
%  The format of the GetCubeInfo method is:
%
%      cube_info=GetCubeInfo()
%
%  A description of each parameter follows.
%
%    o cube_info: A pointer to the Cube structure.
%
*/
static CubeInfo *GetCubeInfo(void)
{
  CubeInfo
    *cube_info;

  /*
    Initialize tree to describe color cube.
  */
  cube_info=(CubeInfo *) AcquireMagickMemory(sizeof(*cube_info));
  if (cube_info == (CubeInfo *) NULL)
    return((CubeInfo *) NULL);
  (void) ResetMagickMemory(cube_info,0,sizeof(*cube_info));
  /*
    Initialize root node.
  */
  cube_info->root=GetNodeInfo(cube_info,0);
  if (cube_info->root == (NodeInfo *) NULL)
    return((CubeInfo *) NULL);
  return(cube_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t I m a g e H i s t o g r a m                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageHistogram() returns the unique colors in an image.
%
%  The format of the GetImageHistogram method is:
%
%      size_t GetImageHistogram(const Image *image,
%        size_t *number_colors,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o file:  Write a histogram of the color distribution to this file handle.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport ColorPacket *GetImageHistogram(const Image *image,
  size_t *number_colors,ExceptionInfo *exception)
{
  ColorPacket
    *histogram;

  CubeInfo
    *cube_info;

  *number_colors=0;
  histogram=(ColorPacket *) NULL;
  cube_info=ClassifyImageColors(image,exception);
  if (cube_info != (CubeInfo *) NULL)
    {
      histogram=(ColorPacket *) AcquireQuantumMemory((size_t) cube_info->colors,
        sizeof(*histogram));
      if (histogram == (ColorPacket *) NULL)
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      else
        {
          ColorPacket
            *root;

          *number_colors=cube_info->colors;
          root=histogram;
          DefineImageHistogram(image,cube_info->root,&root);
        }
    }
  cube_info=DestroyCubeInfo(image,cube_info);
  return(histogram);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  G e t N o d e I n f o                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNodeInfo() allocates memory for a new node in the color cube tree and
%  presets all fields to zero.
%
%  The format of the GetNodeInfo method is:
%
%      NodeInfo *GetNodeInfo(CubeInfo *cube_info,const size_t level)
%
%  A description of each parameter follows.
%
%    o cube_info: A pointer to the CubeInfo structure.
%
%    o level: Specifies the level in the storage_class the node resides.
%
*/
static NodeInfo *GetNodeInfo(CubeInfo *cube_info,const size_t level)
{
  NodeInfo
    *node_info;

  if (cube_info->free_nodes == 0)
    {
      Nodes
        *nodes;

      /*
        Allocate a new nodes of nodes.
      */
      nodes=(Nodes *) AcquireMagickMemory(sizeof(*nodes));
      if (nodes == (Nodes *) NULL)
        return((NodeInfo *) NULL);
      nodes->next=cube_info->node_queue;
      cube_info->node_queue=nodes;
      cube_info->node_info=nodes->nodes;
      cube_info->free_nodes=NodesInAList;
    }
  cube_info->free_nodes--;
  node_info=cube_info->node_info++;
  (void) ResetMagickMemory(node_info,0,sizeof(*node_info));
  node_info->level=level;
  return(node_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s H i s t o g r a m I m a g e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsHistogramImage() returns MagickTrue if the image has 1024 unique colors or
%  less.
%
%  The format of the IsHistogramImage method is:
%
%      MagickBooleanType IsHistogramImage(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsHistogramImage(const Image *image,
  ExceptionInfo *exception)
{
#define MaximumUniqueColors  1024

  CacheView
    *image_view;

  CubeInfo
    *cube_info;

  MagickPixelPacket
    pixel,
    target;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    x;

  register NodeInfo
    *node_info;

  register ssize_t
    i;

  size_t
    id,
    index,
    level;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->storage_class == PseudoClass) && (image->colors <= 256))
    return(MagickTrue);
  if (image->storage_class == PseudoClass)
    return(MagickFalse);
  /*
    Initialize color description tree.
  */
  cube_info=GetCubeInfo();
  if (cube_info == (CubeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  GetMagickPixelPacket(image,&pixel);
  GetMagickPixelPacket(image,&target);
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      /*
        Start at the root and proceed level by level.
      */
      node_info=cube_info->root;
      index=MaxTreeDepth-1;
      for (level=1; level < MaxTreeDepth; level++)
      {
        SetMagickPixelPacket(image,p,indexes+x,&pixel);
        id=ColorToNodeId(image,&pixel,index);
        if (node_info->child[id] == (NodeInfo *) NULL)
          {
            node_info->child[id]=GetNodeInfo(cube_info,level);
            if (node_info->child[id] == (NodeInfo *) NULL)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed","`%s'",
                  image->filename);
                break;
              }
          }
        node_info=node_info->child[id];
        index--;
      }
      if (level < MaxTreeDepth)
        break;
      for (i=0; i < (ssize_t) node_info->number_unique; i++)
      {
        SetMagickPixelPacket(image,&node_info->list[i].pixel,
          &node_info->list[i].index,&target);
        if (IsMagickColorEqual(&pixel,&target) != MagickFalse)
          break;
      }
      if (i < (ssize_t) node_info->number_unique)
        node_info->list[i].count++;
      else
        {
          /*
            Add this unique color to the color list.
          */
          if (node_info->number_unique == 0)
            node_info->list=(ColorPacket *) AcquireMagickMemory(
              sizeof(*node_info->list));
          else
            node_info->list=(ColorPacket *) ResizeQuantumMemory(node_info->list,
              (size_t) (i+1),sizeof(*node_info->list));
          if (node_info->list == (ColorPacket *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'",
                image->filename);
              break;
            }
          node_info->list[i].pixel=(*p);
          if ((image->colorspace == CMYKColorspace) ||
              (image->storage_class == PseudoClass))
            node_info->list[i].index=GetPixelIndex(indexes+x);
          node_info->list[i].count=1;
          node_info->number_unique++;
          cube_info->colors++;
          if (cube_info->colors > MaximumUniqueColors)
            break;
        }
      p++;
    }
    if (x < (ssize_t) image->columns)
      break;
  }
  image_view=DestroyCacheView(image_view);
  cube_info=DestroyCubeInfo(image,cube_info);
  return(y < (ssize_t) image->rows ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s P a l e t t e I m a g e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPaletteImage() returns MagickTrue if the image is PseudoClass and has 256
%  unique colors or less.
%
%  The format of the IsPaletteImage method is:
%
%      MagickBooleanType IsPaletteImage(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsPaletteImage(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  CubeInfo
    *cube_info;

  MagickPixelPacket
    pixel,
    target;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register ssize_t
    x;

  register NodeInfo
    *node_info;

  register ssize_t
    i;

  size_t
    id,
    index,
    level;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((image->storage_class == PseudoClass) && (image->colors <= 256))
    return(MagickTrue);
  if (image->storage_class == PseudoClass)
    return(MagickFalse);
  /*
    Initialize color description tree.
  */
  cube_info=GetCubeInfo();
  if (cube_info == (CubeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(MagickFalse);
    }
  GetMagickPixelPacket(image,&pixel);
  GetMagickPixelPacket(image,&target);
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      /*
        Start at the root and proceed level by level.
      */
      node_info=cube_info->root;
      index=MaxTreeDepth-1;
      for (level=1; level < MaxTreeDepth; level++)
      {
        SetMagickPixelPacket(image,p,indexes+x,&pixel);
        id=ColorToNodeId(image,&pixel,index);
        if (node_info->child[id] == (NodeInfo *) NULL)
          {
            node_info->child[id]=GetNodeInfo(cube_info,level);
            if (node_info->child[id] == (NodeInfo *) NULL)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed","`%s'",
                  image->filename);
                break;
              }
          }
        node_info=node_info->child[id];
        index--;
      }
      if (level < MaxTreeDepth)
        break;
      for (i=0; i < (ssize_t) node_info->number_unique; i++)
      {
        SetMagickPixelPacket(image,&node_info->list[i].pixel,
          &node_info->list[i].index,&target);
        if (IsMagickColorEqual(&pixel,&target) != MagickFalse)
          break;
      }
      if (i < (ssize_t) node_info->number_unique)
        node_info->list[i].count++;
      else
        {
          /*
            Add this unique color to the color list.
          */
          if (node_info->number_unique == 0)
            node_info->list=(ColorPacket *) AcquireMagickMemory(
              sizeof(*node_info->list));
          else
            node_info->list=(ColorPacket *) ResizeQuantumMemory(node_info->list,
              (size_t) (i+1),sizeof(*node_info->list));
          if (node_info->list == (ColorPacket *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'",
                image->filename);
              break;
            }
          node_info->list[i].pixel=(*p);
          if ((image->colorspace == CMYKColorspace) ||
              (image->storage_class == PseudoClass))
            node_info->list[i].index=GetPixelIndex(indexes+x);
          node_info->list[i].count=1;
          node_info->number_unique++;
          cube_info->colors++;
          if (cube_info->colors > 256)
            break;
        }
      p++;
    }
    if (x < (ssize_t) image->columns)
      break;
  }
  image_view=DestroyCacheView(image_view);
  cube_info=DestroyCubeInfo(image,cube_info);
  return(y < (ssize_t) image->rows ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M i n M a x S t r e t c h I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MinMaxStretchImage() uses the exact minimum and maximum values found in
%  each of the channels given, as the BlackPoint and WhitePoint to linearly
%  stretch the colors (and histogram) of the image.  The stretch points are
%  also moved further inward by the adjustment values given.
%
%  If the adjustment values are both zero this function is equivalent to a
%  perfect normalization (or autolevel) of the image.
%
%  Each channel is stretched independantally of each other (producing color
%  distortion) unless the special 'SyncChannels' flag is also provided in the
%  channels setting. If this flag is present the minimum and maximum point
%  will be extracted from all the given channels, and those channels will be
%  stretched by exactly the same amount (preventing color distortion).
%
%  In the special case that only ONE value is found in a channel of the image
%  that value is not stretched, that value is left as is.
%
%  The 'SyncChannels' is turned on in the 'DefaultChannels' setting by
%  default.
%
%  The format of the MinMaxStretchImage method is:
%
%      MagickBooleanType MinMaxStretchImage(Image *image,
%        const ChannelType channel, const double black_adjust,
%        const double white_adjust)
%
%  A description of each parameter follows:
%
%    o image: The image to auto-level
%
%    o channel: The channels to auto-level.  If the special 'SyncChannels'
%      flag is set, all the given channels are stretched by the same amount.
%
%    o black_adjust, white_adjust:  Move the Black/White Point inward
%      from the minimum and maximum points by this color value.
%
*/

MagickExport MagickBooleanType MinMaxStretchImage(Image *image,
  const ChannelType channel,const double black_value,const double white_value)
{
  double
    min,
    max;

  MagickStatusType
    status;

  status=MagickTrue;
  if ((channel & SyncChannels) != 0)
    {
      /*
        Auto-level all channels equally.
      */
      (void) GetImageChannelRange(image,channel,&min,&max,&image->exception);
      min+=black_value;
      max-=white_value;
      if (fabs(min-max) >= MagickEpsilon)
        status&=LevelImageChannel(image,channel,min,max,1.0);
      return(status != 0 ? MagickTrue : MagickFalse);
    }
  /*
    Auto-level each channel separately.
  */
  if ((channel & RedChannel) != 0)
    {
      (void) GetImageChannelRange(image,RedChannel,&min,&max,&image->exception);
      min+=black_value;
      max-=white_value;
      if (fabs(min-max) >= MagickEpsilon)
        status&=LevelImageChannel(image,RedChannel,min,max,1.0);
    }
  if ((channel & GreenChannel) != 0)
    {
      (void) GetImageChannelRange(image,GreenChannel,&min,&max,
        &image->exception);
      min+=black_value;
      max-=white_value;
      if (fabs(min-max) >= MagickEpsilon)
        status&=LevelImageChannel(image,GreenChannel,min,max,1.0);
    }
  if ((channel & BlueChannel) != 0)
    {
      (void) GetImageChannelRange(image,BlueChannel,&min,&max,
        &image->exception);
      min+=black_value;
      max-=white_value;
      if (fabs(min-max) >= MagickEpsilon)
        status&=LevelImageChannel(image,BlueChannel,min,max,1.0);
    }
  if (((channel & OpacityChannel) != 0) &&
       (image->matte != MagickFalse))
    {
      (void) GetImageChannelRange(image,OpacityChannel,&min,&max,
        &image->exception);
      min+=black_value;
      max-=white_value;
      if (fabs(min-max) >= MagickEpsilon)
        status&=LevelImageChannel(image,OpacityChannel,min,max,1.0);
    }
  if (((channel & IndexChannel) != 0) &&
       (image->colorspace == CMYKColorspace))
    {
      (void) GetImageChannelRange(image,IndexChannel,&min,&max,
        &image->exception);
      min+=black_value;
      max-=white_value;
      if (fabs(min-max) >= MagickEpsilon)
        status&=LevelImageChannel(image,IndexChannel,min,max,1.0);
    }
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t N u m b e r C o l o r s                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNumberColors() returns the number of unique colors in an image.
%
%  The format of the GetNumberColors method is:
%
%      size_t GetNumberColors(const Image *image,FILE *file,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o file:  Write a histogram of the color distribution to this file handle.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int HistogramCompare(const void *x,const void *y)
{
  const ColorPacket
    *color_1,
    *color_2;

  color_1=(const ColorPacket *) x;
  color_2=(const ColorPacket *) y;
  if (color_2->pixel.red != color_1->pixel.red)
    return((int) color_1->pixel.red-(int) color_2->pixel.red);
  if (color_2->pixel.green != color_1->pixel.green)
    return((int) color_1->pixel.green-(int) color_2->pixel.green);
  if (color_2->pixel.blue != color_1->pixel.blue)
    return((int) color_1->pixel.blue-(int) color_2->pixel.blue);
  return((int) color_2->count-(int) color_1->count);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport size_t GetNumberColors(const Image *image,FILE *file,
  ExceptionInfo *exception)
{
#define HistogramImageTag  "Histogram/Image"

  char
    color[MaxTextExtent],
    hex[MaxTextExtent],
    tuple[MaxTextExtent];

  ColorPacket
    *histogram;

  MagickBooleanType
    status;

  MagickPixelPacket
    pixel;

  register ColorPacket
    *p;

  register ssize_t
    i;

  size_t
    number_colors;

  number_colors=0;
  if (file == (FILE *) NULL)
    {
      CubeInfo
        *cube_info;

      cube_info=ClassifyImageColors(image,exception);
      if (cube_info != (CubeInfo *) NULL)
        number_colors=cube_info->colors;
      cube_info=DestroyCubeInfo(image,cube_info);
      return(number_colors);
    }
  histogram=GetImageHistogram(image,&number_colors,exception);
  if (histogram == (ColorPacket *) NULL)
    return(number_colors);
  qsort((void *) histogram,(size_t) number_colors,sizeof(*histogram),
    HistogramCompare);
  GetMagickPixelPacket(image,&pixel);
  p=histogram;
  status=MagickTrue;
  for (i=0; i < (ssize_t) number_colors; i++)
  {
    SetMagickPixelPacket(image,&p->pixel,&p->index,&pixel);
    (void) CopyMagickString(tuple,"(",MaxTextExtent);
    ConcatenateColorComponent(&pixel,RedChannel,X11Compliance,tuple);
    (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
    ConcatenateColorComponent(&pixel,GreenChannel,X11Compliance,tuple);
    (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
    ConcatenateColorComponent(&pixel,BlueChannel,X11Compliance,tuple);
    if (pixel.colorspace == CMYKColorspace)
      {
        (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
        ConcatenateColorComponent(&pixel,IndexChannel,X11Compliance,tuple);
      }
    if (pixel.matte != MagickFalse)
      {
        (void) ConcatenateMagickString(tuple,",",MaxTextExtent);
        ConcatenateColorComponent(&pixel,OpacityChannel,X11Compliance,tuple);
      }
    (void) ConcatenateMagickString(tuple,")",MaxTextExtent);
    (void) QueryMagickColorname(image,&pixel,SVGCompliance,color,exception);
    GetColorTuple(&pixel,MagickTrue,hex);
    (void) FormatLocaleFile(file,"%10.20g",(double) ((MagickOffsetType)
      p->count));
    (void) FormatLocaleFile(file,": %s %s %s\n",tuple,hex,color);
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,HistogramImageTag,(MagickOffsetType) i,
          number_colors);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
    p++;
  }
  (void) fflush(file);
  histogram=(ColorPacket *) RelinquishMagickMemory(histogram);
  if (status == MagickFalse)
    return(0);
  return(number_colors);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  U n i q u e I m a g e C o l o r s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UniqueImageColors() returns the unique colors of an image.
%
%  The format of the UniqueImageColors method is:
%
%      Image *UniqueImageColors(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void UniqueColorsToImage(Image *unique_image,CacheView *unique_view,
  CubeInfo *cube_info,const NodeInfo *node_info,ExceptionInfo *exception)
{
#define UniqueColorsImageTag  "UniqueColors/Image"

  MagickBooleanType
    status;

  register ssize_t
    i;

  size_t
    number_children;

  /*
    Traverse any children.
  */
  number_children=unique_image->matte == MagickFalse ? 8UL : 16UL;
  for (i=0; i < (ssize_t) number_children; i++)
    if (node_info->child[i] != (NodeInfo *) NULL)
      UniqueColorsToImage(unique_image,unique_view,cube_info,
        node_info->child[i],exception);
  if (node_info->level == (MaxTreeDepth-1))
    {
      register ColorPacket
        *p;

      register IndexPacket
        *restrict indexes;

      register PixelPacket
        *restrict q;

      status=MagickTrue;
      p=node_info->list;
      for (i=0; i < (ssize_t) node_info->number_unique; i++)
      {
        q=QueueCacheViewAuthenticPixels(unique_view,cube_info->x,0,1,1,
          exception);
        if (q == (PixelPacket *) NULL)
          continue;
        indexes=GetCacheViewAuthenticIndexQueue(unique_view);
        *q=p->pixel;
        if (unique_image->colorspace == CMYKColorspace)
          *indexes=p->index;
        if (SyncCacheViewAuthenticPixels(unique_view,exception) == MagickFalse)
          break;
        cube_info->x++;
        p++;
      }
      if (unique_image->progress_monitor != (MagickProgressMonitor) NULL)
        {
          MagickBooleanType
            proceed;

          proceed=SetImageProgress(unique_image,UniqueColorsImageTag,
            cube_info->progress,cube_info->colors);
          if (proceed == MagickFalse)
            status=MagickFalse;
        }
      cube_info->progress++;
      if (status == MagickFalse)
        return;
    }
}

MagickExport Image *UniqueImageColors(const Image *image,
  ExceptionInfo *exception)
{
  CacheView
    *unique_view;

  CubeInfo
    *cube_info;

  Image
    *unique_image;

  cube_info=ClassifyImageColors(image,exception);
  if (cube_info == (CubeInfo *) NULL)
    return((Image *) NULL);
  unique_image=CloneImage(image,cube_info->colors,1,MagickTrue,exception);
  if (unique_image == (Image *) NULL)
    return(unique_image);
  if (SetImageStorageClass(unique_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&unique_image->exception);
      unique_image=DestroyImage(unique_image);
      return((Image *) NULL);
    }
  unique_view=AcquireVirtualCacheView(unique_image,exception);
  UniqueColorsToImage(unique_image,unique_view,cube_info,cube_info->root,
    exception);
  unique_view=DestroyCacheView(unique_view);
  if (cube_info->colors < MaxColormapSize)
    {
      QuantizeInfo
        *quantize_info;

      quantize_info=AcquireQuantizeInfo((ImageInfo *) NULL);
      quantize_info->number_colors=MaxColormapSize;
      quantize_info->dither=MagickFalse;
      quantize_info->tree_depth=8;
      (void) QuantizeImage(quantize_info,unique_image);
      quantize_info=DestroyQuantizeInfo(quantize_info);
    }
  cube_info=DestroyCubeInfo(image,cube_info);
  return(unique_image);
}

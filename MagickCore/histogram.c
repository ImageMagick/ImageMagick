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

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color-private.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/histogram.h"
#include "MagickCore/image.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/list.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/prepress.h"
#include "MagickCore/quantize.h"
#include "MagickCore/registry.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"

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

  PixelInfo
    *list;

  size_t
    extent;

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
  const PixelInfo *pixel,size_t index)
{
  size_t
    id;

  id=(size_t) (
    ((ScaleQuantumToChar(ClampToQuantum(pixel->red)) >> index) & 0x01) |
    ((ScaleQuantumToChar(ClampToQuantum(pixel->green)) >> index) & 0x01) << 1 |
    ((ScaleQuantumToChar(ClampToQuantum(pixel->blue)) >> index) & 0x01) << 2);
  if (image->alpha_trait != UndefinedPixelTrait)
    id|=((ScaleQuantumToChar(ClampToQuantum(pixel->alpha)) >> index) &
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

  PixelInfo
    pixel;

  NodeInfo
    *node_info;

  register const Quantum
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
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cube_info=GetCubeInfo();
  if (cube_info == (CubeInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(cube_info);
    }
  GetPixelInfo(image,&pixel);
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      /*
        Start at the root and proceed level by level.
      */
      node_info=cube_info->root;
      index=MaxTreeDepth-1;
      for (level=1; level < MaxTreeDepth; level++)
      {
        GetPixelInfoPixel(image,p,&pixel);
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
        if (IsPixelInfoEquivalent(&pixel,node_info->list+i) != MagickFalse)
          break;
      if (i < (ssize_t) node_info->number_unique)
        node_info->list[i].count++;
      else
        {
          if (node_info->number_unique == 0)
            {
              node_info->extent=1;
              node_info->list=(PixelInfo *) AcquireQuantumMemory(
                node_info->extent,sizeof(*node_info->list));
            }
          else
            if (i >= (ssize_t) node_info->extent)
              {
                node_info->extent<<=1;
                node_info->list=(PixelInfo *) ResizeQuantumMemory(
                  node_info->list,node_info->extent,sizeof(*node_info->list));
              }
          if (node_info->list == (PixelInfo *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'",
                image->filename);
              return(0);
            }
          node_info->list[i]=pixel;
          node_info->list[i].red=(double) GetPixelRed(image,p);
          node_info->list[i].green=(double) GetPixelGreen(image,p);
          node_info->list[i].blue=(double) GetPixelBlue(image,p);
          if (image->colorspace == CMYKColorspace)
            node_info->list[i].black=(double) GetPixelBlack(image,p);
          node_info->list[i].alpha=(double) GetPixelAlpha(image,p);
          node_info->list[i].count=1;
          node_info->number_unique++;
          cube_info->colors++;
        }
      p+=GetPixelChannels(image);
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
%        PixelInfo **unique_colors)
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
  PixelInfo **histogram)
{
  register ssize_t
    i;

  size_t
    number_children;

  /*
    Traverse any children.
  */
  number_children=image->alpha_trait == UndefinedPixelTrait ? 8UL : 16UL;
  for (i=0; i < (ssize_t) number_children; i++)
    if (node_info->child[i] != (NodeInfo *) NULL)
      DefineImageHistogram(image,node_info->child[i],histogram);
  if (node_info->level == (MaxTreeDepth-1))
    {
      register PixelInfo
        *p;

      p=node_info->list;
      for (i=0; i < (ssize_t) node_info->number_unique; i++)
      {
        **histogram=(*p);
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
  number_children=image->alpha_trait == UndefinedPixelTrait ? 8UL : 16UL;
  for (i=0; i < (ssize_t) number_children; i++)
    if (node_info->child[i] != (NodeInfo *) NULL)
      DestroyColorCube(image,node_info->child[i]);
  if (node_info->list != (PixelInfo *) NULL)
    node_info->list=(PixelInfo *) RelinquishMagickMemory(node_info->list);
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
MagickExport PixelInfo *GetImageHistogram(const Image *image,
  size_t *number_colors,ExceptionInfo *exception)
{
  PixelInfo
    *histogram;

  CubeInfo
    *cube_info;

  *number_colors=0;
  histogram=(PixelInfo *) NULL;
  cube_info=ClassifyImageColors(image,exception);
  if (cube_info != (CubeInfo *) NULL)
    {
      histogram=(PixelInfo *) AcquireQuantumMemory((size_t) cube_info->colors+1,
        sizeof(*histogram));
      if (histogram == (PixelInfo *) NULL)
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      else
        {
          PixelInfo
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
%  I d e n t i f y P a l e t t e I m a g e                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IdentifyPaletteImage() returns MagickTrue if the image has 256 unique colors
%  or less.
%
%  The format of the IdentifyPaletteImage method is:
%
%      MagickBooleanType IdentifyPaletteImage(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType CheckImageColors(const Image *image,
  ExceptionInfo *exception,size_t max_colors)
{
  CacheView
    *image_view;

  CubeInfo
    *cube_info;

  PixelInfo
    pixel,
    target;

  register const Quantum
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

  if (image->storage_class == PseudoClass)
    return((image->colors <= max_colors) ? MagickTrue : MagickFalse);
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
  GetPixelInfo(image,&pixel);
  GetPixelInfo(image,&target);
  image_view=AcquireVirtualCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      /*
        Start at the root and proceed level by level.
      */
      node_info=cube_info->root;
      index=MaxTreeDepth-1;
      for (level=1; level < MaxTreeDepth; level++)
      {
        GetPixelInfoPixel(image,p,&pixel);
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
        target=node_info->list[i];
        if (IsPixelInfoEquivalent(&pixel,&target) != MagickFalse)
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
            node_info->list=(PixelInfo *) AcquireMagickMemory(
              sizeof(*node_info->list));
          else
            node_info->list=(PixelInfo *) ResizeQuantumMemory(node_info->list,
              (size_t) (i+1),sizeof(*node_info->list));
          if (node_info->list == (PixelInfo *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                ResourceLimitError,"MemoryAllocationFailed","`%s'",
                image->filename);
              break;
            }
          GetPixelInfo(image,&node_info->list[i]);
          node_info->list[i].red=(double) GetPixelRed(image,p);
          node_info->list[i].green=(double) GetPixelGreen(image,p);
          node_info->list[i].blue=(double) GetPixelBlue(image,p);
          if (image->colorspace == CMYKColorspace)
            node_info->list[i].black=(double) GetPixelBlack(image,p);
          node_info->list[i].alpha=(double) GetPixelAlpha(image,p);
          node_info->list[i].count=1;
          node_info->number_unique++;
          cube_info->colors++;
          if (cube_info->colors > max_colors)
            break;
        }
      p+=GetPixelChannels(image);
    }
    if (x < (ssize_t) image->columns)
      break;
  }
  image_view=DestroyCacheView(image_view);
  cube_info=DestroyCubeInfo(image,cube_info);
  return(y < (ssize_t) image->rows ? MagickFalse : MagickTrue);
}

MagickExport MagickBooleanType IdentifyPaletteImage(const Image *image,
  ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(CheckImageColors(image,exception,256));
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

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(CheckImageColors(image,exception,MaximumUniqueColors));
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
%      MagickBooleanType IsPaletteImage(const Image *image)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsPaletteImage(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class != PseudoClass)
    return(MagickFalse);
  return((image->colors <= 256) ? MagickTrue : MagickFalse);
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
%      MagickBooleanType MinMaxStretchImage(Image *image,const double black,
%        const double white,const double gamma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image to auto-level
%
%    o black, white:  move the black / white point inward from the minimum and
%      maximum points by this color value.
%
%    o gamma: the gamma.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType MinMaxStretchImage(Image *image,
  const double black,const double white,const double gamma,
  ExceptionInfo *exception)
{
  double
    min,
    max;

  register ssize_t
    i;

  MagickStatusType
    status;

  status=MagickTrue;
  if (image->channel_mask == DefaultChannels)
    {
      /*
        Auto-level all channels equally.
      */
      (void) GetImageRange(image,&min,&max,exception);
      min+=black;
      max-=white;
      if (fabs(min-max) >= MagickEpsilon)
        status&=LevelImage(image,min,max,gamma,exception);
      return(status != 0 ? MagickTrue : MagickFalse);
    }
  /*
    Auto-level each channel.
  */
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    ChannelType
      channel_mask;

    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if ((traits & UpdatePixelTrait) == 0)
      continue;
    channel_mask=SetImageChannelMask(image,(ChannelType) (1 << i));
    status&=GetImageRange(image,&min,&max,exception);
    min+=black;
    max-=white;
    if (fabs(min-max) >= MagickEpsilon)
      status&=LevelImage(image,min,max,gamma,exception);
    (void) SetImageChannelMask(image,channel_mask);
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
  const PixelInfo
    *color_1,
    *color_2;

  color_1=(const PixelInfo *) x;
  color_2=(const PixelInfo *) y;
  if (color_2->red != color_1->red)
    return((int) color_1->red-(int) color_2->red);
  if (color_2->green != color_1->green)
    return((int) color_1->green-(int) color_2->green);
  if (color_2->blue != color_1->blue)
    return((int) color_1->blue-(int) color_2->blue);
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
    color[MagickPathExtent],
    hex[MagickPathExtent],
    tuple[MagickPathExtent];

  PixelInfo
    *histogram;

  MagickBooleanType
    status;

  PixelInfo
    pixel;

  register PixelInfo
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
  if (histogram == (PixelInfo *) NULL)
    return(number_colors);
  qsort((void *) histogram,(size_t) number_colors,sizeof(*histogram),
    HistogramCompare);
  GetPixelInfo(image,&pixel);
  p=histogram;
  status=MagickTrue;
  for (i=0; i < (ssize_t) number_colors; i++)
  {
    pixel=(*p);
    (void) CopyMagickString(tuple,"(",MagickPathExtent);
    ConcatenateColorComponent(&pixel,RedPixelChannel,X11Compliance,tuple);
    (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
    ConcatenateColorComponent(&pixel,GreenPixelChannel,X11Compliance,tuple);
    (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
    ConcatenateColorComponent(&pixel,BluePixelChannel,X11Compliance,tuple);
    if (pixel.colorspace == CMYKColorspace)
      {
        (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
        ConcatenateColorComponent(&pixel,BlackPixelChannel,X11Compliance,
          tuple);
      }
    if (pixel.alpha_trait != UndefinedPixelTrait)
      {
        (void) ConcatenateMagickString(tuple,",",MagickPathExtent);
        ConcatenateColorComponent(&pixel,AlphaPixelChannel,X11Compliance,
          tuple);
      }
    (void) ConcatenateMagickString(tuple,")",MagickPathExtent);
    (void) QueryColorname(image,&pixel,SVGCompliance,color,exception);
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
  histogram=(PixelInfo *) RelinquishMagickMemory(histogram);
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
  number_children=unique_image->alpha_trait == UndefinedPixelTrait ? 8UL : 16UL;
  for (i=0; i < (ssize_t) number_children; i++)
    if (node_info->child[i] != (NodeInfo *) NULL)
      UniqueColorsToImage(unique_image,unique_view,cube_info,
        node_info->child[i],exception);
  if (node_info->level == (MaxTreeDepth-1))
    {
      register PixelInfo
        *p;

      register Quantum
        *magick_restrict q;

      status=MagickTrue;
      p=node_info->list;
      for (i=0; i < (ssize_t) node_info->number_unique; i++)
      {
        q=QueueCacheViewAuthenticPixels(unique_view,cube_info->x,0,1,1,
          exception);
        if (q == (Quantum *) NULL)
          continue;
        SetPixelRed(unique_image,ClampToQuantum(p->red),q);
        SetPixelGreen(unique_image,ClampToQuantum(p->green),q);
        SetPixelBlue(unique_image,ClampToQuantum(p->blue),q);
        SetPixelAlpha(unique_image,ClampToQuantum(p->alpha),q);
        if (unique_image->colorspace == CMYKColorspace)
          SetPixelBlack(unique_image,ClampToQuantum(p->black),q);
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
  if (SetImageStorageClass(unique_image,DirectClass,exception) == MagickFalse)
    {
      unique_image=DestroyImage(unique_image);
      return((Image *) NULL);
    }
  unique_view=AcquireAuthenticCacheView(unique_image,exception);
  UniqueColorsToImage(unique_image,unique_view,cube_info,cube_info->root,
    exception);
  unique_view=DestroyCacheView(unique_view);
  cube_info=DestroyCubeInfo(image,cube_info);
  return(unique_image);
}

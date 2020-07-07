/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                   AAA   SSSSS  H   H  L       AAA   RRRR                    %
%                  A   A  SS     H   H  L      A   A  R   R                   %
%                  AAAAA   SSS   HHHHH  L      AAAAA  RRRR                    %
%                  A   A     SS  H   H  L      A   A  R  R                    %
%                  A   A  SSSSS  H   H  LLLLL  A   A  R   R                   %
%                                                                             %
%                                                                             %
%                           Write Ashlar Images                               %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 2020                                   %
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

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/annotate.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/client.h"
#include "MagickCore/constitute.h"
#include "MagickCore/display.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/option.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"
#include "MagickCore/xwindow.h"
#include "MagickCore/xwindow-private.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteASHLARImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r A S H L A R I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterASHLARImage() adds attributes for the ASHLAR image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterASHLARImage method is:
%
%      size_t RegisterASHLARImage(void)
%
*/
ModuleExport size_t RegisterASHLARImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("ASHLAR","ASHLAR",
   "Image sequence laid out in continuous irregular courses");
  entry->encoder=(EncodeImageHandler *) WriteASHLARImage;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r A S H L A R I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterASHLARImage() removes format registrations made by the
%  ASHLAR module from the list of supported formats.
%
%  The format of the UnregisterASHLARImage method is:
%
%      UnregisterASHLARImage(void)
%
*/
ModuleExport void UnregisterASHLARImage(void)
{
  (void) UnregisterMagickInfo("ASHLAR");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e A S H L A R I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteASHLARImage() writes an image to a file in ASHLAR format.
%
%  The format of the WriteASHLARImage method is:
%
%      MagickBooleanType WriteASHLARImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct _NodeInfo
{
  ssize_t
    x,
    y;

  struct _NodeInfo
    *next;
} NodeInfo;

typedef struct _AshlarInfo
{
  size_t
    width,
    height;

  ssize_t
    align;

  size_t
    number_nodes;

  MagickBooleanType
    best_fit;

  NodeInfo
    *current,
    *free,
    head,
    sentinal;
} AshlarInfo;

typedef struct _CanvasInfo
{
  ssize_t
    id;

  size_t
    width,
    height;

  ssize_t
    x,
    y,
    order;
} CanvasInfo;

typedef struct _TileInfo
{
  ssize_t
    x,
    y;

  NodeInfo
    **previous;
} TileInfo;

static ssize_t FindMinimumTileLocation(NodeInfo *first,const ssize_t x,
  const size_t width,ssize_t *excess)
{
  NodeInfo
    *node;

  ssize_t
    extent,
    y;

  /*
    Find minimum y location if it starts at x.
  */
  *excess=0;
  y=0;
  extent=0;
  node=first;
  while (node->x < (x+width))
  {
    if (node->y > y)
      {
        *excess+=extent*(node->y-y);
        y=node->y;
        if (node->x < x)
          extent+=node->next->x-x;
        else
          extent+=node->next->x-node->x;
      }
    else
      {
        size_t delta = (size_t) (node->next->x-node->x);
        if ((delta+extent) > width)
          delta=width-extent;
        *excess+=delta*(y-node->y);
        extent+=delta;
      }
    node=node->next;
  }
  return(y);
}

static TileInfo AssignBestTileLocation(AshlarInfo *ashlar_info,
  size_t width,size_t height)
{
  NodeInfo
    *node,
    **previous,
    *tail;

  ssize_t
    min_excess;

  TileInfo
    tile;

  /*
    Align along left edge.
  */
  tile.previous=(NodeInfo **) NULL;
  width=(width+ashlar_info->align-1);
  width-=width % ashlar_info->align;
  if ((width > ashlar_info->width) || (height > ashlar_info->height))
    {
      /*
        Tile can't fit, bail.
      */
      tile.x=0;
      tile.y=0;
      return(tile);
    }
  tile.x=(ssize_t) SSIZE_MAX;
  tile.y=(ssize_t) SSIZE_MAX;
  min_excess=(ssize_t) SSIZE_MAX;
  node=ashlar_info->current;
  previous=(&ashlar_info->current);
  while ((width+node->x) <= ashlar_info->width)
  {
    ssize_t
      excess,
      y;

    y=FindMinimumTileLocation(node,node->x,width,&excess);
    if (ashlar_info->best_fit == MagickFalse)
      {
        if (y < tile.y)
          {
            tile.y=y;
            tile.previous=previous;
          }
      }
    else
      {
        if ((height+y)  <= ashlar_info->height)
          if ((y < tile.y) || ((y == tile.y) && (excess < min_excess)))
            {
              tile.y=y;
              tile.previous=previous;
              min_excess=excess;
            }
      }
    previous=(&node->next);
    node=node->next;
  }
  tile.x=(tile.previous == (NodeInfo **) NULL) ? 0 : (*tile.previous)->x;
  if (ashlar_info->best_fit != MagickFalse)
    {
      /*
        Align along both left and right edges.
      */
      tail=ashlar_info->current;
      node=ashlar_info->current;
      previous=(&ashlar_info->current);
      while (tail->x < (ssize_t) width)
        tail=tail->next;
      while (tail != (NodeInfo *) NULL)
      {
        ssize_t
          excess,
          x,
          y;

        x=tail->x-width;
        while (node->next->x <= x)
        {
          previous=(&node->next);
          node=node->next;
        }
        y=FindMinimumTileLocation(node,x,width,&excess);
        if ((height+y) <= ashlar_info->height)
          {
            if (y <= tile.y)
              if ((y < tile.y) || (excess < min_excess) ||
                  ((excess == min_excess) && (x < tile.x)))
                {
                  tile.x=x;
                  tile.y=y;
                  min_excess=excess;
                  tile.previous=previous;
               }
         }
       tail=tail->next;
    }
  }
  return(tile);
}

static TileInfo AssignTileLocation(AshlarInfo *ashlar_info,const size_t width,
  const size_t height)
{
  NodeInfo
    *current,
    *node;

  TileInfo
    tile;

  /*
    Find the best location in the canvas for this tile.
  */
  tile=AssignBestTileLocation(ashlar_info,width,height);
  if ((tile.previous == (NodeInfo **) NULL) ||
      ((tile.y+height) > (ssize_t) ashlar_info->height) ||
      (ashlar_info->free == (NodeInfo *) NULL))
    {
      tile.previous=(NodeInfo **) NULL;
      return(tile);
    }
   /*
     Create a new node.
   */
   node=ashlar_info->free;
   node->x=(ssize_t) tile.x;
   node->y=(ssize_t) (tile.y+height);
   ashlar_info->free=node->next;
   /*
     Insert node.
   */
   current=(*tile.previous);
   if (current->x >= tile.x)
     *tile.previous=node;
   else
     {
       NodeInfo *next = current->next;
       current->next=node;
       current=next;
     }
   while ((current->next != (NodeInfo *) NULL) &&
          (current->next->x <= (tile.x+width)))
   {
     /*
       Push current node to free list.
     */
     NodeInfo *next = current->next;
     current->next=ashlar_info->free;
     ashlar_info->free=current;
     current=next;
   }
   node->next=current;
   if (current->x < (tile.x+width))
     current->x=(ssize_t) (tile.x+width);
   return(tile);
}

static int CompareTileHeight(const void *p_tile,const void *q_tile)
{
  const CanvasInfo
    *p,
    *q;

  p=(const CanvasInfo *) p_tile;
  q=(const CanvasInfo *) q_tile;
  if (p->height > q->height)
    return(-1);
  if (p->height < q->height)
    return(1);
  return((p->width > q->width) ? -1 : (p->width < q->width) ? 1 : 0);
}

static int RestoreTileOrder(const void *p_tile,const void *q_tile)
{
  const CanvasInfo
    *p,
    *q;

  p=(const CanvasInfo *) p_tile;
  q=(const CanvasInfo *) q_tile;
  return((p->order < q->order) ? -1 : (p->order > q->order) ? 1 : 0);
}

static MagickBooleanType PackAshlarTiles(AshlarInfo *ashlar_info,
  CanvasInfo *tiles,const size_t number_tiles)
{
  MagickBooleanType
    status;

  register ssize_t
    i;

  /*
    Pack tiles so they fit the canvas with minimum excess.
  */
  for (i=0; i < (ssize_t) number_tiles; i++)
    tiles[i].order=(i);
  qsort((void *) tiles,number_tiles,sizeof(*tiles),CompareTileHeight);
  for (i=0; i < (ssize_t) number_tiles; i++)
  {
    tiles[i].x=0;
    tiles[i].y=0;
    if ((tiles[i].width != 0) && (tiles[i].height != 0))
      {
        TileInfo
          tile_info;

        tile_info=AssignTileLocation(ashlar_info,tiles[i].width,
          tiles[i].height);
        tiles[i].x=(ssize_t) tile_info.x;
        tiles[i].y=(ssize_t) tile_info.y;
        if (tile_info.previous == (NodeInfo **) NULL)
          {
            tiles[i].x=(ssize_t) SSIZE_MAX;
            tiles[i].y=(ssize_t) SSIZE_MAX;
          }
      }
  }
  qsort((void *) tiles,number_tiles,sizeof(*tiles),RestoreTileOrder);
  status=MagickTrue;
  for (i=0; i < (ssize_t) number_tiles; i++)
  {
    tiles[i].order=(ssize_t) ((tiles[i].x != (ssize_t) SSIZE_MAX) ||
      (tiles[i].y != (ssize_t) SSIZE_MAX) ? 1 : 0);
    if (tiles[i].order == 0)
      status=MagickFalse;
  }
  return(status);  /* return true if room is found for all tiles */
}

static MagickBooleanType WriteASHLARImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  AshlarInfo
    ashlar_info;

  CanvasInfo
    *tiles;

  const char
    *value;

  Image
    *ashlar_image,
    *next;

  ImageInfo
    *write_info;

  MagickBooleanType
    status;

  NodeInfo
    *nodes;

  RectangleInfo
    extent,
    geometry;

  ssize_t
    i,
    n;

  /*
    Convert image sequence laid out in continuous irregular courses.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image_info->extract != (char *) NULL)
    (void) ParseAbsoluteGeometry(image_info->extract,&geometry);
  else
    {
      /*
        Determine a sane canvas size and border width.
      */
      (void) ParseAbsoluteGeometry("0x0+0+0",&geometry);
      for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
      {
        geometry.width+=next->columns;
        geometry.height+=next->rows;
      }
      geometry.width=(size_t) geometry.width/7;
      geometry.height=(size_t) geometry.height/7;
      geometry.x=(ssize_t) pow((double) geometry.width,0.25);
      geometry.y=(ssize_t) pow((double) geometry.height,0.25);
    }
  /*
    Initialize image tiles.
  */
  ashlar_image=AcquireImage(image_info,exception);
  status=SetImageExtent(ashlar_image,geometry.width,geometry.height,exception);
  if (status == MagickFalse)
    {
      ashlar_image=DestroyImageList(ashlar_image);
      return(MagickFalse);
    }
  (void) SetImageBackgroundColor(ashlar_image,exception);
  tiles=(CanvasInfo *) AcquireQuantumMemory(GetImageListLength(image),
    sizeof(*tiles));
  ashlar_info.number_nodes=2*geometry.width;
  nodes=(NodeInfo *) AcquireQuantumMemory(ashlar_info.number_nodes,
    sizeof(*nodes));
  if ((tiles == (CanvasInfo *) NULL) || (nodes == (NodeInfo *) NULL))
    {
      if (tiles != (CanvasInfo *) NULL)
        tiles=(CanvasInfo *) RelinquishMagickMemory(tiles);
      if (nodes != (NodeInfo *) NULL)
        nodes=(NodeInfo *) RelinquishMagickMemory(tiles);
      ashlar_image=DestroyImageList(ashlar_image);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Interate until we find a tile size that fits the canvas.
  */
  value=GetImageOption(image_info,"ashlar:best-fit");
  for (i=20; i > 0; i--)
  {
    register ssize_t
      j;

    n=0;
    for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
    {
      tiles[n].id=n;
      tiles[n].width=(size_t) (0.05*i*next->columns+2*geometry.x);
      tiles[n].height=(size_t) (0.05*i*next->rows+2*geometry.y);
      n++;
    }
    for (j=0; j < (ssize_t) ashlar_info.number_nodes-1; j++)
      nodes[j].next=nodes+j+1;
    nodes[j].next=(NodeInfo *) NULL;
    ashlar_info.best_fit=IsStringTrue(value) != MagickFalse ? MagickTrue :
      MagickFalse;
    ashlar_info.free=nodes;
    ashlar_info.current=(&ashlar_info.head);
    ashlar_info.width=geometry.width;
    ashlar_info.height=geometry.height;
    ashlar_info.align=(ssize_t) ((ashlar_info.width+ashlar_info.number_nodes-1)/
      ashlar_info.number_nodes);
    ashlar_info.head.x=0;
    ashlar_info.head.y=0;
    ashlar_info.head.next=(&ashlar_info.sentinal);
    ashlar_info.sentinal.x=(ssize_t) geometry.width;
    ashlar_info.sentinal.y=(ssize_t) SSIZE_MAX;
    ashlar_info.sentinal.next=(NodeInfo *) NULL;
    status=PackAshlarTiles(&ashlar_info,tiles,(size_t) n);
    if (status != MagickFalse)
      break;
  }
  /*
    Determine layout of images tiles on the canvas.
  */
  value=GetImageOption(image_info,"label");
  extent.width=0;
  extent.height=0;
  for (i=0; i < n; i++)
  {
    Image
      *tile_image;

    if ((tiles[i].x == (ssize_t) SSIZE_MAX) ||
        (tiles[i].y == (ssize_t) SSIZE_MAX))
      continue;
    tile_image=ResizeImage(GetImageFromList(image,tiles[i].id),(size_t)
      (tiles[i].width-2*geometry.x),(size_t) (tiles[i].height-2*geometry.y),
      image->filter,exception);
    if (tile_image == (Image *) NULL)
      continue;
    (void) CompositeImage(ashlar_image,tile_image,image->compose,MagickTrue,
      tiles[i].x+geometry.x,tiles[i].y+geometry.y,exception);
    if (value != (const char *) NULL)
      {
        char
          *label,
          offset[MagickPathExtent];

        DrawInfo
          *draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);

        label=InterpretImageProperties((ImageInfo *) image_info,tile_image,
          value,exception);
        if (label != (const char *) NULL)
          {
            (void) CloneString(&draw_info->text,label);
            draw_info->pointsize=1.8*geometry.y;
            (void) FormatLocaleString(offset,MagickPathExtent,"%+g%+g",(double)
              tiles[i].x+geometry.x,(double) tiles[i].height+tiles[i].y+
              geometry.y/2.0);
            (void) CloneString(&draw_info->geometry,offset);
            (void) AnnotateImage(ashlar_image,draw_info,exception);
          }
      }
    if ((tiles[i].width+tiles[i].x) > extent.width)
      extent.width=(size_t) (tiles[i].width+tiles[i].x);
    if ((tiles[i].height+tiles[i].y) > extent.height)
      extent.height=(size_t) (tiles[i].height+tiles[i].y);
    tile_image=DestroyImage(tile_image);
  }
  (void) SetImageExtent(ashlar_image,extent.width,extent.height,exception);
  nodes=(NodeInfo *) RelinquishMagickMemory(nodes);
  tiles=(CanvasInfo *) RelinquishMagickMemory(tiles);
  /*
    Write ASHLAR canvas.
  */
  (void) CopyMagickString(ashlar_image->filename,image_info->filename,
    MagickPathExtent);
  write_info=CloneImageInfo(image_info);
  *write_info->magick='\0';
  (void) SetImageInfo(write_info,1,exception);
  if ((*write_info->magick == '\0') ||
      (LocaleCompare(write_info->magick,"ASHLAR") == 0))
    (void) FormatLocaleString(ashlar_image->filename,MagickPathExtent,
      "miff:%s",write_info->filename);
  status=WriteImage(write_info,ashlar_image,exception);
  ashlar_image=DestroyImage(ashlar_image);
  write_info=DestroyImageInfo(write_info);
  return(MagickTrue);
}

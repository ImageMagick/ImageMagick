/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               M   M   OOO   N   N  TTTTT   AAA    GGGG  EEEEE               %
%               MM MM  O   O  NN  N    T    A   A  G      E                   %
%               M M M  O   O  N N N    T    AAAAA  G  GG  EEE                 %
%               M   M  O   O  N  NN    T    A   A  G   G  E                   %
%               M   M   OOO   N   N    T    A   A   GGG   EEEEE               %
%                                                                             %
%                                                                             %
%                MagickCore Methods to Create Image Thumbnails                %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
#include "MagickCore/annotate.h"
#include "MagickCore/client.h"
#include "MagickCore/color.h"
#include "MagickCore/composite.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/quantize.h"
#include "MagickCore/property.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string_.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"
#include "MagickCore/visual-effects.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e M o n t a g e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneMontageInfo() makes a copy of the given montage info structure.  If
%  NULL is specified, a new image info structure is created initialized to
%  default values.
%
%  The format of the CloneMontageInfo method is:
%
%      MontageInfo *CloneMontageInfo(const ImageInfo *image_info,
%        const MontageInfo *montage_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o montage_info: the montage info.
%
*/
MagickExport MontageInfo *CloneMontageInfo(const ImageInfo *image_info,
  const MontageInfo *montage_info)
{
  MontageInfo
    *clone_info;

  clone_info=(MontageInfo *) AcquireCriticalMemory(sizeof(*clone_info));
  GetMontageInfo(image_info,clone_info);
  if (montage_info == (MontageInfo *) NULL)
    return(clone_info);
  if (montage_info->geometry != (char *) NULL)
    clone_info->geometry=AcquireString(montage_info->geometry);
  if (montage_info->tile != (char *) NULL)
    clone_info->tile=AcquireString(montage_info->tile);
  if (montage_info->title != (char *) NULL)
    clone_info->title=AcquireString(montage_info->title);
  if (montage_info->frame != (char *) NULL)
    clone_info->frame=AcquireString(montage_info->frame);
  if (montage_info->texture != (char *) NULL)
    clone_info->texture=AcquireString(montage_info->texture);
  if (montage_info->font != (char *) NULL)
    clone_info->font=AcquireString(montage_info->font);
  clone_info->pointsize=montage_info->pointsize;
  clone_info->border_width=montage_info->border_width;
  clone_info->shadow=montage_info->shadow;
  clone_info->fill=montage_info->fill;
  clone_info->stroke=montage_info->stroke;
  clone_info->matte_color=montage_info->matte_color;
  clone_info->background_color=montage_info->background_color;
  clone_info->border_color=montage_info->border_color;
  clone_info->gravity=montage_info->gravity;
  (void) CopyMagickString(clone_info->filename,montage_info->filename,
    MagickPathExtent);
  clone_info->debug=IsEventLogging();
  return(clone_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y M o n t a g e I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMontageInfo() deallocates memory associated with montage_info.
%
%  The format of the DestroyMontageInfo method is:
%
%      MontageInfo *DestroyMontageInfo(MontageInfo *montage_info)
%
%  A description of each parameter follows:
%
%    o montage_info: Specifies a pointer to an MontageInfo structure.
%
%
*/
MagickExport MontageInfo *DestroyMontageInfo(MontageInfo *montage_info)
{
  assert(montage_info != (MontageInfo *) NULL);
  assert(montage_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  if (montage_info->geometry != (char *) NULL)
    montage_info->geometry=(char *)
      RelinquishMagickMemory(montage_info->geometry);
  if (montage_info->tile != (char *) NULL)
    montage_info->tile=DestroyString(montage_info->tile);
  if (montage_info->title != (char *) NULL)
    montage_info->title=DestroyString(montage_info->title);
  if (montage_info->frame != (char *) NULL)
    montage_info->frame=DestroyString(montage_info->frame);
  if (montage_info->texture != (char *) NULL)
    montage_info->texture=(char *) RelinquishMagickMemory(
      montage_info->texture);
  if (montage_info->font != (char *) NULL)
    montage_info->font=DestroyString(montage_info->font);
  montage_info->signature=(~MagickCoreSignature);
  montage_info=(MontageInfo *) RelinquishMagickMemory(montage_info);
  return(montage_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M o n t a g e I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMontageInfo() initializes montage_info to default values.
%
%  The format of the GetMontageInfo method is:
%
%      void GetMontageInfo(const ImageInfo *image_info,
%        MontageInfo *montage_info)
%
%  A description of each parameter follows:
%
%    o image_info: a structure of type ImageInfo.
%
%    o montage_info: Specifies a pointer to a MontageInfo structure.
%
*/
MagickExport void GetMontageInfo(const ImageInfo *image_info,
  MontageInfo *montage_info)
{
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(montage_info != (MontageInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  (void) memset(montage_info,0,sizeof(*montage_info));
  (void) CopyMagickString(montage_info->filename,image_info->filename,
    MagickPathExtent);
  montage_info->geometry=AcquireString(DefaultTileGeometry);
  if (image_info->font != (char *) NULL)
    montage_info->font=AcquireString(image_info->font);
  montage_info->gravity=CenterGravity;
  montage_info->pointsize=image_info->pointsize;
  montage_info->fill.alpha=(MagickRealType) OpaqueAlpha;
  montage_info->stroke.alpha=(MagickRealType) TransparentAlpha;
  montage_info->matte_color=image_info->matte_color;
  montage_info->background_color=image_info->background_color;
  montage_info->border_color=image_info->border_color;
  montage_info->debug=IsEventLogging();
  montage_info->signature=MagickCoreSignature;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M o n t a g e I m a g e L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MontageImageList() is a layout manager that lets you tile one or more
%  thumbnails across an image canvas.
%
%  The format of the MontageImageList method is:
%
%      Image *MontageImageList(const ImageInfo *image_info,
%        const MontageInfo *montage_info,Image *images,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o montage_info: Specifies a pointer to a MontageInfo structure.
%
%    o images: Specifies a pointer to an array of Image structures.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void GetMontageGeometry(char *geometry,const size_t number_images,
  ssize_t *x_offset,ssize_t *y_offset,size_t *tiles_per_column,
  size_t *tiles_per_row)
{
  *tiles_per_column=0;
  *tiles_per_row=0;
  (void) GetGeometry(geometry,x_offset,y_offset,tiles_per_row,tiles_per_column);
  if ((*tiles_per_column == 0) && (*tiles_per_row == 0))
    *tiles_per_column=(size_t) sqrt((double) number_images);
  if ((*tiles_per_column == 0) && (*tiles_per_row != 0))
    *tiles_per_column=(size_t) ceil((double) number_images/(*tiles_per_row));
  if ((*tiles_per_row == 0) && (*tiles_per_column != 0))
    *tiles_per_row=(size_t) ceil((double) number_images/(*tiles_per_column));
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int SceneCompare(const void *x,const void *y)
{
  Image
    **image_1,
    **image_2;

  image_1=(Image **) x;
  image_2=(Image **) y;
  return((int) ((*image_1)->scene-(*image_2)->scene));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport Image *MontageImages(const Image *images,
  const MontageInfo *montage_info,ExceptionInfo *exception)
{
  Image
    *montage_image;

  ImageInfo
    *image_info;

  image_info=AcquireImageInfo();
  montage_image=MontageImageList(image_info,montage_info,images,exception);
  image_info=DestroyImageInfo(image_info);
  return(montage_image);
}

MagickExport Image *MontageImageList(const ImageInfo *image_info,
  const MontageInfo *montage_info,const Image *images,ExceptionInfo *exception)
{
#define MontageImageTag  "Montage/Image"
#define TileImageTag  "Tile/Image"

  char
    tile_geometry[MagickPathExtent],
    *title;

  const char
    *value;

  DrawInfo
    *draw_info;

  FrameInfo
    frame_info;

  Image
    *image,
    **image_list,
    **primary_list,
    *montage,
    *texture,
    *tile_image,
    *thumbnail;

  ImageInfo
    *clone_info;

  MagickBooleanType
    concatenate,
    proceed,
    status;

  MagickOffsetType
    tiles;

  MagickProgressMonitor
    progress_monitor;

  MagickStatusType
    flags;

  ssize_t
    i;

  RectangleInfo
    bounds,
    geometry,
    extract_info;

  size_t
    border_width,
    extent,
    height,
    images_per_page,
    max_height,
    number_images,
    number_lines,
    sans,
    tiles_per_column,
    tiles_per_page,
    tiles_per_row,
    title_offset,
    total_tiles,
    width;

  ssize_t
    bevel_width,
    tile,
    x,
    x_offset,
    y,
    y_offset;

  TypeMetric
    metrics;

  /*
    Create image tiles.
  */
  assert(images != (Image *) NULL);
  assert(images->signature == MagickCoreSignature);
  assert(montage_info != (MontageInfo *) NULL);
  assert(montage_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  number_images=GetImageListLength(images);
  primary_list=ImageListToArray(images,exception);
  if (primary_list == (Image **) NULL)
    return((Image *) NULL);
  image_list=primary_list;
  image=image_list[0];
  thumbnail=NewImageList();
  for (i=0; i < (ssize_t) number_images; i++)
  {
    image=CloneImage(image_list[i],0,0,MagickTrue,exception);
    if (image == (Image *) NULL)
      break;
    (void) ParseAbsoluteGeometry("0x0+0+0",&image->page);
    progress_monitor=SetImageProgressMonitor(image,(MagickProgressMonitor) NULL,
      image->client_data);
    flags=ParseRegionGeometry(image,montage_info->geometry,&geometry,exception);
    thumbnail=ThumbnailImage(image,geometry.width,geometry.height,exception);
    if (thumbnail == (Image *) NULL)
      break;
    image_list[i]=thumbnail;
    (void) SetImageProgressMonitor(image,progress_monitor,image->client_data);
    proceed=SetImageProgress(image,TileImageTag,(MagickOffsetType) i,
      number_images);
    if (proceed == MagickFalse)
      break;
    image=DestroyImage(image);
  }
  if (i < (ssize_t) number_images)
    {
      if (image != (Image *) NULL)
        image=DestroyImage(image);
      if (thumbnail == (Image *) NULL)
        i--;
      for (tile=0; (ssize_t) tile <= i; tile++)
        if (image_list[tile] != (Image *) NULL)
          image_list[tile]=DestroyImage(image_list[tile]);
      primary_list=(Image **) RelinquishMagickMemory(primary_list);
      return((Image *) NULL);
    }
  /*
    Sort image list by increasing tile number.
  */
  for (i=0; i < (ssize_t) number_images; i++)
    if (image_list[i]->scene == 0)
      break;
  if (i == (ssize_t) number_images)
    qsort((void *) image_list,(size_t) number_images,sizeof(*image_list),
      SceneCompare);
  /*
    Determine tiles per row and column.
  */
  tiles_per_column=(size_t) sqrt((double) number_images);
  tiles_per_row=(size_t) ceil((double) number_images/tiles_per_column);
  x_offset=0;
  y_offset=0;
  if (montage_info->tile != (char *) NULL)
    GetMontageGeometry(montage_info->tile,number_images,&x_offset,&y_offset,
      &tiles_per_column,&tiles_per_row);
  /*
    Determine tile sizes.
  */
  concatenate=MagickFalse;
  SetGeometry(image_list[0],&extract_info);
  extract_info.x=(ssize_t) montage_info->border_width;
  extract_info.y=(ssize_t) montage_info->border_width;
  if (montage_info->geometry != (char *) NULL)
    {
      /*
        Initialize tile geometry.
      */
      flags=GetGeometry(montage_info->geometry,&extract_info.x,&extract_info.y,
        &extract_info.width,&extract_info.height);
      concatenate=((flags & RhoValue) == 0) && ((flags & SigmaValue) == 0) ?
        MagickTrue : MagickFalse;
    }
  border_width=montage_info->border_width;
  bevel_width=0;
  (void) memset(&frame_info,0,sizeof(frame_info));
  if (montage_info->frame != (char *) NULL)
    {
      char
        absolute_geometry[MagickPathExtent];

      frame_info.width=extract_info.width;
      frame_info.height=extract_info.height;
      (void) FormatLocaleString(absolute_geometry,MagickPathExtent,"%s!",
        montage_info->frame);
      flags=ParseMetaGeometry(absolute_geometry,&frame_info.outer_bevel,
        &frame_info.inner_bevel,&frame_info.width,&frame_info.height);
      if ((flags & HeightValue) == 0)
        frame_info.height=frame_info.width;
      if ((flags & XiValue) == 0)
        frame_info.outer_bevel=(ssize_t) frame_info.width/2-1;
      if ((flags & PsiValue) == 0)
        frame_info.inner_bevel=frame_info.outer_bevel;
      frame_info.x=(ssize_t) frame_info.width;
      frame_info.y=(ssize_t) frame_info.height;
      bevel_width=(ssize_t) MagickMax(frame_info.inner_bevel,
        frame_info.outer_bevel);
      border_width=(size_t) MagickMax((ssize_t) frame_info.width,
        (ssize_t) frame_info.height);
    }
  for (i=0; i < (ssize_t) number_images; i++)
  {
    if (image_list[i]->columns > extract_info.width)
      extract_info.width=image_list[i]->columns;
    if (image_list[i]->rows > extract_info.height)
      extract_info.height=image_list[i]->rows;
  }
  /*
    Initialize draw attributes.
  */
  clone_info=CloneImageInfo(image_info);
  clone_info->background_color=montage_info->background_color;
  clone_info->border_color=montage_info->border_color;
  draw_info=CloneDrawInfo(clone_info,(DrawInfo *) NULL);
  if (montage_info->font != (char *) NULL)
    (void) CloneString(&draw_info->font,montage_info->font);
  if (montage_info->pointsize != 0.0)
    draw_info->pointsize=montage_info->pointsize;
  draw_info->gravity=CenterGravity;
  draw_info->stroke=montage_info->stroke;
  draw_info->fill=montage_info->fill;
  draw_info->text=AcquireString("");
  (void) GetTypeMetrics(image_list[0],draw_info,&metrics,exception);
  texture=NewImageList();
  if (montage_info->texture != (char *) NULL)
    {
      (void) CopyMagickString(clone_info->filename,montage_info->texture,
        MagickPathExtent);
      texture=ReadImage(clone_info,exception);
    }
  /*
    Determine the number of lines in an next label.
  */
  title=InterpretImageProperties(clone_info,image_list[0],montage_info->title,
    exception);
  title_offset=0;
  if (montage_info->title != (char *) NULL)
    title_offset=(size_t) (2*(metrics.ascent-metrics.descent)*
      MultilineCensus(title)+2*extract_info.y);
  number_lines=0;
  for (i=0; i < (ssize_t) number_images; i++)
  {
    value=GetImageProperty(image_list[i],"label",exception);
    if (value == (const char *) NULL)
      continue;
    if (MultilineCensus(value) > number_lines)
      number_lines=MultilineCensus(value);
  }
  /*
    Allocate next structure.
  */
  tile_image=AcquireImage((ImageInfo *) NULL,exception);
  montage=AcquireImage(clone_info,exception);
  montage->background_color=montage_info->background_color;
  montage->scene=0;
  images_per_page=(number_images-1)/(tiles_per_row*tiles_per_column)+1;
  tiles=0;
  total_tiles=(size_t) number_images;
  for (i=0; i < (ssize_t) images_per_page; i++)
  {
    /*
      Determine bounding box.
    */
    tiles_per_page=tiles_per_row*tiles_per_column;
    x_offset=0;
    y_offset=0;
    if (montage_info->tile != (char *) NULL)
      GetMontageGeometry(montage_info->tile,number_images,&x_offset,&y_offset,
        &sans,&sans);
    tiles_per_page=tiles_per_row*tiles_per_column;
    y_offset+=(ssize_t) title_offset;
    max_height=0;
    bounds.width=0;
    bounds.height=0;
    width=0;
    for (tile=0; tile < (ssize_t) tiles_per_page; tile++)
    {
      if (tile < (ssize_t) number_images)
        {
          width=concatenate != MagickFalse ? image_list[tile]->columns :
            extract_info.width;
          if (image_list[tile]->rows > max_height)
            max_height=image_list[tile]->rows;
        }
      x_offset+=((ssize_t) width+2*(extract_info.x+(ssize_t) border_width));
      if (x_offset > (ssize_t) bounds.width)
        bounds.width=(size_t) x_offset;
      if (((tile+1) == (ssize_t) tiles_per_page) ||
          (((tile+1) % (ssize_t) tiles_per_row) == 0))
        {
          x_offset=0;
          if (montage_info->tile != (char *) NULL)
            GetMontageGeometry(montage_info->tile,number_images,&x_offset,&y,
              &sans,&sans);
          height=concatenate != MagickFalse ? max_height : extract_info.height;
          y_offset+=((ssize_t) height+(extract_info.y+(ssize_t) border_width)*2+
            (metrics.ascent-metrics.descent+4)*(ssize_t) number_lines+
            (montage_info->shadow != MagickFalse ? 4 : 0));
          if (y_offset > (ssize_t) bounds.height)
            bounds.height=(size_t) y_offset;
          max_height=0;
        }
    }
    if (montage_info->shadow != MagickFalse)
      bounds.width+=4;
    /*
      Initialize montage image.
    */
    (void) CopyMagickString(montage->filename,montage_info->filename,
      MagickPathExtent);
    montage->columns=(size_t) MagickMax((ssize_t) bounds.width,1);
    montage->rows=(size_t) MagickMax((ssize_t) bounds.height,1);
    (void) SetImageBackgroundColor(montage,exception);
    /*
      Set montage geometry.
    */
    montage->montage=AcquireString((char *) NULL);
    tile=0;
    extent=1;
    while (tile < MagickMin((ssize_t) tiles_per_page,(ssize_t) number_images))
    {
      extent+=strlen(image_list[tile]->filename)+1;
      tile++;
    }
    montage->directory=(char *) AcquireQuantumMemory(extent,
      sizeof(*montage->directory));
    if ((montage->montage == (char *) NULL) ||
        (montage->directory == (char *) NULL))
      {
        if (montage->montage != (char *) NULL)
          montage->montage=(char *) RelinquishMagickMemory(montage->montage);
        if (montage->directory != (char *) NULL)
          montage->directory=(char *) RelinquishMagickMemory(
            montage->directory);
        ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
      }
    x_offset=0;
    y_offset=0;
    if (montage_info->tile != (char *) NULL)
      GetMontageGeometry(montage_info->tile,number_images,&x_offset,&y_offset,
        &sans,&sans);
    y_offset+=(ssize_t) title_offset;
    (void) FormatLocaleString(montage->montage,MagickPathExtent,
      "%.20gx%.20g%+.20g%+.20g",(double) ((ssize_t) extract_info.width+
      (extract_info.x+(ssize_t) border_width)*2),(double) ((ssize_t)
      extract_info.height+(extract_info.y+(ssize_t) border_width)*2+(double)
      ((metrics.ascent-metrics.descent+4)*number_lines+
      (montage_info->shadow != MagickFalse ? 4 : 0))),(double) x_offset,
      (double) y_offset);
    *montage->directory='\0';
    tile=0;
    while (tile < MagickMin((ssize_t) tiles_per_page,(ssize_t) number_images))
    {
      if (strchr(image_list[tile]->filename,(int) '\xff') == (char *) NULL)
        (void) ConcatenateMagickString(montage->directory,
          image_list[tile]->filename,extent);
      else
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "InvalidArgument","'%s'",image_list[tile]->filename);
      (void) ConcatenateMagickString(montage->directory,"\xff",extent);
      tile++;
    }
    progress_monitor=SetImageProgressMonitor(montage,(MagickProgressMonitor)
      NULL,montage->client_data);
    if (texture != (Image *) NULL)
      (void) TextureImage(montage,texture,exception);
    if (montage_info->title != (char *) NULL)
      {
        DrawInfo
          *draw_clone_info;

        TypeMetric
          tile_metrics;

        /*
          Annotate composite image with title.
        */
        draw_clone_info=CloneDrawInfo(image_info,draw_info);
        draw_clone_info->gravity=CenterGravity;
        draw_clone_info->pointsize*=2.0;
        (void) GetTypeMetrics(image_list[0],draw_clone_info,&tile_metrics,
          exception);
        (void) FormatLocaleString(tile_geometry,MagickPathExtent,
          "%.20gx%.20g%+.20g%+.20g",(double) montage->columns,(double)
          (tile_metrics.ascent-tile_metrics.descent),0.0,
          (double) extract_info.y+4);
        (void) CloneString(&draw_clone_info->geometry,tile_geometry);
        (void) CloneString(&draw_clone_info->text,title);
        (void) AnnotateImage(montage,draw_clone_info,exception);
        draw_clone_info=DestroyDrawInfo(draw_clone_info);
      }
    (void) SetImageProgressMonitor(montage,progress_monitor,
      montage->client_data);
    /*
      Copy tile to the composite.
    */
    x_offset=0;
    y_offset=0;
    if (montage_info->tile != (char *) NULL)
      GetMontageGeometry(montage_info->tile,number_images,&x_offset,&y_offset,
        &sans,&sans);
    x_offset+=extract_info.x;
    y_offset+=(ssize_t) title_offset+extract_info.y;
    max_height=0;
    status=MagickTrue;
    for (tile=0; tile < MagickMin((ssize_t) tiles_per_page,(ssize_t) number_images); tile++)
    {
      /*
        Copy this tile to the composite.
      */
      image=CloneImage(image_list[tile],0,0,MagickTrue,exception);
      if (image == (Image *) NULL)
        ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
      progress_monitor=SetImageProgressMonitor(image,
        (MagickProgressMonitor) NULL,image->client_data);
      width=concatenate != MagickFalse ? image->columns : extract_info.width;
      if (image->rows > max_height)
        max_height=image->rows;
      height=concatenate != MagickFalse ? max_height : extract_info.height;
      if (border_width != 0)
        {
          Image
            *border_image;

          RectangleInfo
            border_info;

          /*
            Put a border around the image.
          */
          border_info.width=border_width;
          border_info.height=border_width;
          if (montage_info->frame != (char *) NULL)
            {
              border_info.width=(width-image->columns+1)/2;
              border_info.height=(height-image->rows+1)/2;
            }
          border_image=BorderImage(image,&border_info,image->compose,exception);
          if (border_image != (Image *) NULL)
            {
              image=DestroyImage(image);
              image=border_image;
            }
          if ((montage_info->frame != (char *) NULL) &&
              (image->compose == DstOutCompositeOp))
            {
              (void) SetPixelChannelMask(image,AlphaChannel);
              (void) NegateImage(image,MagickFalse,exception);
              (void) SetPixelChannelMask(image,DefaultChannels);
            }
        }
      /*
        Gravitate as specified by the tile gravity.
      */
      tile_image->columns=width;
      tile_image->rows=height;
      tile_image->gravity=montage_info->gravity;
      if (image->gravity != UndefinedGravity)
        tile_image->gravity=image->gravity;
      (void) FormatLocaleString(tile_geometry,MagickPathExtent,
        "%.20gx%.20g+0+0",(double) image->columns,(double) image->rows);
      flags=ParseGravityGeometry(tile_image,tile_geometry,&geometry,exception);
      x=geometry.x+(ssize_t) border_width;
      y=geometry.y+(ssize_t) border_width;
      if ((montage_info->frame != (char *) NULL) && (bevel_width > 0))
        {
          FrameInfo
            frame_clone;

          Image
            *frame_image;

          /*
            Put an ornamental border around this tile.
          */
          frame_clone=frame_info;
          frame_clone.width=width+2*frame_info.width;
          frame_clone.height=height+2*frame_info.height;
          value=GetImageProperty(image,"label",exception);
          if (value != (const char *) NULL)
            frame_clone.height+=(size_t) ((metrics.ascent-metrics.descent+4)*
              MultilineCensus(value));
          frame_image=FrameImage(image,&frame_clone,image->compose,exception);
          if (frame_image != (Image *) NULL)
            {
              image=DestroyImage(image);
              image=frame_image;
            }
          x=0;
          y=0;
        }
      if (LocaleCompare(image->magick,"NULL") != 0)
        {
          /*
            Composite background with tile.
          */
          if (montage_info->shadow != MagickFalse)
            {
              Image
                *shadow_image;

              /*
                Shadow image.
              */
              (void) QueryColorCompliance("#0000",AllCompliance,
                &image->background_color,exception);
              shadow_image=ShadowImage(image,30.0,5.0,5,5,exception);
              if (shadow_image != (Image *) NULL)
                {
                  (void) CompositeImage(shadow_image,image,OverCompositeOp,
                    MagickTrue,0,0,exception);
                  image=DestroyImage(image);
                  image=shadow_image;
                }
          }
          (void) CompositeImage(montage,image,image->compose,MagickTrue,
            x_offset+x,y_offset+y,exception);
          value=GetImageProperty(image,"label",exception);
          if (value != (const char *) NULL)
            {
              /*
                Annotate composite tile with label.
              */
              (void) FormatLocaleString(tile_geometry,MagickPathExtent,
                "%.20gx%.20g%+.20g%+.20g",(double) ((montage_info->frame ?
                image->columns : width)-2*border_width),(double)
                (metrics.ascent-metrics.descent+4)*MultilineCensus(value),
                (double) (x_offset+(ssize_t) border_width),(double)
                ((montage_info->frame ? y_offset+(ssize_t) height+(ssize_t)
                border_width+4 : y_offset+(ssize_t) extract_info.height+
                (ssize_t) border_width+
                (montage_info->shadow != MagickFalse ? 4 : 0))+bevel_width));
              (void) CloneString(&draw_info->geometry,tile_geometry);
              (void) CloneString(&draw_info->text,value);
              (void) AnnotateImage(montage,draw_info,exception);
            }
        }
      x_offset+=(ssize_t) width+2*(extract_info.x+(ssize_t) border_width);
      if (((tile+1) == (ssize_t) tiles_per_page) ||
          (((tile+1) % (ssize_t) tiles_per_row) == 0))
        {
          x_offset=extract_info.x;
          y_offset+=((ssize_t) height+(extract_info.y+(ssize_t) border_width)*2+
            (metrics.ascent-metrics.descent+4)*number_lines+
            (montage_info->shadow != MagickFalse ? 4 : 0));
          max_height=0;
        }
      if (images->progress_monitor != (MagickProgressMonitor) NULL)
        {
          proceed=SetImageProgress(image,MontageImageTag,tiles,total_tiles);
          if (proceed == MagickFalse)
            status=MagickFalse;
        }
      image_list[tile]=DestroyImage(image_list[tile]);
      image=DestroyImage(image);
      tiles++;
    }
    (void) status;
    if ((i+1) < (ssize_t) images_per_page)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(clone_info,montage,exception);
        if (GetNextImageInList(montage) == (Image *) NULL)
          {
            montage=DestroyImageList(montage);
            return((Image *) NULL);
          }
        montage=GetNextImageInList(montage);
        montage->background_color=montage_info->background_color;
        image_list+=tiles_per_page;
        number_images-=tiles_per_page;
      }
  }
  tile_image=DestroyImage(tile_image);
  if (texture != (Image *) NULL)
    texture=DestroyImage(texture);
  title=DestroyString(title);
  primary_list=(Image **) RelinquishMagickMemory(primary_list);
  draw_info=DestroyDrawInfo(draw_info);
  clone_info=DestroyImageInfo(clone_info);
  return(GetFirstImageInList(montage));
}

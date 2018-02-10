/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            X   X   CCCC  FFFFF                              %
%                             X X   C      F                                  %
%                              X    C      FFF                                %
%                             X X   C      F                                  %
%                            X   X   CCCC  F                                  %
%                                                                             %
%                                                                             %
%                        Read GIMP XCF Image Format                           %
%                                                                             %
%                              Software Design                                %
%                              Leonard Rosenthol                              %
%                               November 2001                                 %
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
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/composite.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/pixel.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

/*
  Typedef declarations.
*/
typedef enum
{
  GIMP_RGB,
  GIMP_GRAY,
  GIMP_INDEXED
} GimpImageBaseType;

typedef enum
{
  PROP_END                   =  0,
  PROP_COLORMAP              =  1,
  PROP_ACTIVE_LAYER          =  2,
  PROP_ACTIVE_CHANNEL        =  3,
  PROP_SELECTION             =  4,
  PROP_FLOATING_SELECTION    =  5,
  PROP_OPACITY               =  6,
  PROP_MODE                  =  7,
  PROP_VISIBLE               =  8,
  PROP_LINKED                =  9,
  PROP_PRESERVE_TRANSPARENCY = 10,
  PROP_APPLY_MASK            = 11,
  PROP_EDIT_MASK             = 12,
  PROP_SHOW_MASK             = 13,
  PROP_SHOW_MASKED           = 14,
  PROP_OFFSETS               = 15,
  PROP_COLOR                 = 16,
  PROP_COMPRESSION           = 17,
  PROP_GUIDES                = 18,
  PROP_RESOLUTION            = 19,
  PROP_TATTOO                = 20,
  PROP_PARASITES             = 21,
  PROP_UNIT                  = 22,
  PROP_PATHS                 = 23,
  PROP_USER_UNIT             = 24
} PropType;

typedef enum
{
  COMPRESS_NONE              =  0,
  COMPRESS_RLE               =  1,
  COMPRESS_ZLIB              =  2,  /* unused */
  COMPRESS_FRACTAL           =  3   /* unused */
} XcfCompressionType;

typedef struct
{
  size_t
    width,
    height,
    image_type,
    bytes_per_pixel;

  int
    compression;

  size_t
    file_size;

  size_t
    number_layers;
} XCFDocInfo;

typedef struct
{
  char
    name[1024];

  unsigned int
    active;

  size_t
    width,
    height,
    type,
    alpha,
    visible,
    linked,
    preserve_trans,
    apply_mask,
    show_mask,
    edit_mask,
    floating_offset;

  ssize_t
    offset_x,
    offset_y;

  size_t
    mode,
    tattoo;

  Image
    *image;
} XCFLayerInfo;

#define TILE_WIDTH   64
#define TILE_HEIGHT  64

typedef struct
{
  unsigned char
    red,
    green,
    blue,
    alpha;
} XCFPixelInfo;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s X C F                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsXCF() returns MagickTrue if the image format type, identified by the
%  magick string, is XCF (GIMP native format).
%
%  The format of the IsXCF method is:
%
%      MagickBooleanType IsXCF(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static MagickBooleanType IsXCF(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"gimp xcf",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

typedef enum
{
  GIMP_NORMAL_MODE,
  GIMP_DISSOLVE_MODE,
  GIMP_BEHIND_MODE,
  GIMP_MULTIPLY_MODE,
  GIMP_SCREEN_MODE,
  GIMP_OVERLAY_MODE,
  GIMP_DIFFERENCE_MODE,
  GIMP_ADDITION_MODE,
  GIMP_SUBTRACT_MODE,
  GIMP_DARKEN_ONLY_MODE,
  GIMP_LIGHTEN_ONLY_MODE,
  GIMP_HUE_MODE,
  GIMP_SATURATION_MODE,
  GIMP_COLOR_MODE,
  GIMP_VALUE_MODE,
  GIMP_DIVIDE_MODE,
  GIMP_DODGE_MODE,
  GIMP_BURN_MODE,
  GIMP_HARDLIGHT_MODE
} GimpLayerModeEffects;

/*
  Simple utility routine to convert between PSD blending modes and
  ImageMagick compositing operators
*/
static CompositeOperator GIMPBlendModeToCompositeOperator(
  size_t blendMode)
{
  switch ( blendMode )
  {
    case GIMP_NORMAL_MODE:       return(OverCompositeOp);
    case GIMP_DISSOLVE_MODE:     return(DissolveCompositeOp);
    case GIMP_MULTIPLY_MODE:     return(MultiplyCompositeOp);
    case GIMP_SCREEN_MODE:       return(ScreenCompositeOp);
    case GIMP_OVERLAY_MODE:      return(OverlayCompositeOp);
    case GIMP_DIFFERENCE_MODE:   return(DifferenceCompositeOp);
    case GIMP_ADDITION_MODE:     return(ModulusAddCompositeOp);
    case GIMP_SUBTRACT_MODE:     return(ModulusSubtractCompositeOp);
    case GIMP_DARKEN_ONLY_MODE:  return(DarkenCompositeOp);
    case GIMP_LIGHTEN_ONLY_MODE: return(LightenCompositeOp);
    case GIMP_HUE_MODE:          return(HueCompositeOp);
    case GIMP_SATURATION_MODE:   return(SaturateCompositeOp);
    case GIMP_COLOR_MODE:        return(ColorizeCompositeOp);
    case GIMP_DODGE_MODE:        return(ColorDodgeCompositeOp);
    case GIMP_BURN_MODE:         return(ColorBurnCompositeOp);
    case GIMP_HARDLIGHT_MODE:    return(HardLightCompositeOp);
    case GIMP_DIVIDE_MODE:       return(DivideDstCompositeOp);
    /* these are the ones we don't support...yet */
    case GIMP_BEHIND_MODE:       return(OverCompositeOp);
    case GIMP_VALUE_MODE:        return(OverCompositeOp);
    default:                     return(OverCompositeOp);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d B l o b S t r i n g W i t h L o n g S i z e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobStringWithLongSize reads characters from a blob or file
%  starting with a ssize_t length byte and then characters to that length
%
%  The format of the ReadBlobStringWithLongSize method is:
%
%      char *ReadBlobStringWithLongSize(Image *image,char *string)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o string: the address of a character buffer.
%
*/

static char *ReadBlobStringWithLongSize(Image *image,char *string,size_t max,
  ExceptionInfo *exception)
{
  int
    c;

  MagickOffsetType
    offset;

  register ssize_t
    i;

  size_t
    length;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(max != 0);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=ReadBlobMSBLong(image);
  for (i=0; i < (ssize_t) MagickMin(length,max-1); i++)
  {
    c=ReadBlobByte(image);
    if (c == EOF)
      return((char *) NULL);
    string[i]=(char) c;
  }
  string[i]='\0';
  offset=SeekBlob(image,(MagickOffsetType) (length-i),SEEK_CUR);
  if (offset < 0)
    (void) ThrowMagickException(exception,GetMagickModule(),
      CorruptImageError,"ImproperImageHeader","`%s'",image->filename);
  return(string);
}

static MagickBooleanType load_tile(Image *image,Image *tile_image,
  XCFDocInfo *inDocInfo,XCFLayerInfo *inLayerInfo,size_t data_length,
  ExceptionInfo *exception)
{
  ssize_t
    y;

  register ssize_t
    x;

  register Quantum
    *q;

  size_t
    extent;

  ssize_t
    count;

  unsigned char
    *graydata;

  XCFPixelInfo
    *xcfdata,
    *xcfodata;

  extent=0;
  if (inDocInfo->image_type == GIMP_GRAY)
    extent=tile_image->columns*tile_image->rows*sizeof(*graydata);
  else
    if (inDocInfo->image_type == GIMP_RGB)
      extent=tile_image->columns*tile_image->rows*sizeof(*xcfdata);
  if (extent > data_length)
    ThrowBinaryException(CorruptImageError,"NotEnoughPixelData",
      image->filename);
  xcfdata=(XCFPixelInfo *) AcquireQuantumMemory(MagickMax(data_length,
    tile_image->columns*tile_image->rows),sizeof(*xcfdata));
  if (xcfdata == (XCFPixelInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  xcfodata=xcfdata;
  graydata=(unsigned char *) xcfdata;  /* used by gray and indexed */
  count=ReadBlob(image,data_length,(unsigned char *) xcfdata);
  if (count != (ssize_t) data_length)
    {
      xcfodata=(XCFPixelInfo *) RelinquishMagickMemory(xcfodata);
      ThrowBinaryException(CorruptImageError,"NotEnoughPixelData",
        image->filename);
    }
  for (y=0; y < (ssize_t) tile_image->rows; y++)
  {
    q=GetAuthenticPixels(tile_image,0,y,tile_image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    if (inDocInfo->image_type == GIMP_GRAY)
      {
        for (x=0; x < (ssize_t) tile_image->columns; x++)
        {
          SetPixelGray(tile_image,ScaleCharToQuantum(*graydata),q);
          SetPixelAlpha(tile_image,ScaleCharToQuantum((unsigned char)
            inLayerInfo->alpha),q);
          graydata++;
          q+=GetPixelChannels(tile_image);
        }
      }
    else
      if (inDocInfo->image_type == GIMP_RGB)
        {
          for (x=0; x < (ssize_t) tile_image->columns; x++)
          {
            SetPixelRed(tile_image,ScaleCharToQuantum(xcfdata->red),q);
            SetPixelGreen(tile_image,ScaleCharToQuantum(xcfdata->green),q);
            SetPixelBlue(tile_image,ScaleCharToQuantum(xcfdata->blue),q);
            SetPixelAlpha(tile_image,xcfdata->alpha == 255U ? TransparentAlpha :
              ScaleCharToQuantum((unsigned char) inLayerInfo->alpha),q);
            xcfdata++;
            q+=GetPixelChannels(tile_image);
          }
        }
     if (SyncAuthenticPixels(tile_image,exception) == MagickFalse)
       break;
  }
  xcfodata=(XCFPixelInfo *) RelinquishMagickMemory(xcfodata);
  return MagickTrue;
}

static MagickBooleanType load_tile_rle(Image *image,Image *tile_image,
  XCFDocInfo *inDocInfo,XCFLayerInfo *inLayerInfo,size_t data_length,
  ExceptionInfo *exception)
{
  MagickOffsetType
    size;

  Quantum
    alpha;

  register Quantum
    *q;

  size_t
    length;

  ssize_t
    bytes_per_pixel,
    count,
    i,
    j;

  unsigned char
    data,
    pixel,
    *xcfdata,
    *xcfodata,
    *xcfdatalimit;

  bytes_per_pixel=(ssize_t) inDocInfo->bytes_per_pixel;
  xcfdata=(unsigned char *) AcquireQuantumMemory(data_length,sizeof(*xcfdata));
  if (xcfdata == (unsigned char *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  xcfodata=xcfdata;
  count=ReadBlob(image, (size_t) data_length, xcfdata);
  xcfdatalimit = xcfodata+count-1;
  alpha=ScaleCharToQuantum((unsigned char) inLayerInfo->alpha);
  for (i=0; i < (ssize_t) bytes_per_pixel; i++)
  {
    q=GetAuthenticPixels(tile_image,0,0,tile_image->columns,tile_image->rows,
      exception);
    if (q == (Quantum *) NULL)
      continue;
    size=(MagickOffsetType) tile_image->rows*tile_image->columns;
    while (size > 0)
    {
      if (xcfdata > xcfdatalimit)
        goto bogus_rle;
      pixel=(*xcfdata++);
      length=(size_t) pixel;
      if (length >= 128)
        {
          length=255-(length-1);
          if (length == 128)
            {
              if (xcfdata >= xcfdatalimit)
                goto bogus_rle;
              length=(size_t) ((*xcfdata << 8) + xcfdata[1]);
              xcfdata+=2;
            }
          size-=length;
          if (size < 0)
            goto bogus_rle;
          if (&xcfdata[length-1] > xcfdatalimit)
            goto bogus_rle;
          while (length-- > 0)
          {
            data=(*xcfdata++);
            switch (i)
            {
              case 0:
              {
                if (inDocInfo->image_type == GIMP_GRAY)
                  SetPixelGray(tile_image,ScaleCharToQuantum(data),q);
                else
                  {
                    SetPixelRed(tile_image,ScaleCharToQuantum(data),q);
                    SetPixelGreen(tile_image,ScaleCharToQuantum(data),q);
                    SetPixelBlue(tile_image,ScaleCharToQuantum(data),q);
                  }
                SetPixelAlpha(tile_image,alpha,q);
                break;
              }
              case 1:
              {
                if (inDocInfo->image_type == GIMP_GRAY)
                  SetPixelAlpha(tile_image,ScaleCharToQuantum(data),q);
                else
                  SetPixelGreen(tile_image,ScaleCharToQuantum(data),q);
                break;
              }
              case 2:
              {
                SetPixelBlue(tile_image,ScaleCharToQuantum(data),q);
                break;
              }
              case 3:
              {
                SetPixelAlpha(tile_image,ScaleCharToQuantum(data),q);
                break;
              }
            }
            q+=GetPixelChannels(tile_image);
          }
        }
      else
        {
          length+=1;
          if (length == 128)
            {
              if (xcfdata >= xcfdatalimit)
                goto bogus_rle;
              length=(size_t) ((*xcfdata << 8) + xcfdata[1]);
              xcfdata+=2;
            }
          size-=length;
          if (size < 0)
            goto bogus_rle;
          if (xcfdata > xcfdatalimit)
            goto bogus_rle;
          pixel=(*xcfdata++);
          for (j=0; j < (ssize_t) length; j++)
          {
            data=pixel;
            switch (i)
            {
              case 0:
              {
                if (inDocInfo->image_type == GIMP_GRAY)
                  SetPixelGray(tile_image,ScaleCharToQuantum(data),q);
                else
                  {
                    SetPixelRed(tile_image,ScaleCharToQuantum(data),q);
                    SetPixelGreen(tile_image,ScaleCharToQuantum(data),q);
                    SetPixelBlue(tile_image,ScaleCharToQuantum(data),q);
                  }
                SetPixelAlpha(tile_image,alpha,q);
                break;
              }
              case 1:
              {
                if (inDocInfo->image_type == GIMP_GRAY)
                  SetPixelAlpha(tile_image,ScaleCharToQuantum(data),q);
                else
                  SetPixelGreen(tile_image,ScaleCharToQuantum(data),q);
                break;
              }
              case 2:
              {
                SetPixelBlue(tile_image,ScaleCharToQuantum(data),q);
                break;
              }
              case 3:
              {
                SetPixelAlpha(tile_image,ScaleCharToQuantum(data),q);
                break;
              }
            }
            q+=GetPixelChannels(tile_image);
          }
        }
    }
    if (SyncAuthenticPixels(tile_image,exception) == MagickFalse)
      break;
  }
  xcfodata=(unsigned char *) RelinquishMagickMemory(xcfodata);
  return(MagickTrue);

  bogus_rle:
    if (xcfodata != (unsigned char *) NULL)
      xcfodata=(unsigned char *) RelinquishMagickMemory(xcfodata);
  return(MagickFalse);
}

static MagickBooleanType load_level(Image *image,XCFDocInfo *inDocInfo,
  XCFLayerInfo *inLayerInfo,ExceptionInfo *exception)
{
  int
    destLeft = 0,
    destTop = 0;

  Image*
    tile_image;

  MagickBooleanType
    status;

  MagickOffsetType
    saved_pos,
    offset,
    offset2;

  register ssize_t
    i;

  size_t
    width,
    height,
    ntiles,
    ntile_rows,
    ntile_cols,
    tile_image_width,
    tile_image_height;

  /* start reading the data */
  width=ReadBlobMSBLong(image);
  height=ReadBlobMSBLong(image);

  /* read in the first tile offset.
   *  if it is '0', then this tile level is empty
   *  and we can simply return.
   */
  offset=(MagickOffsetType) ReadBlobMSBLong(image);
  if (offset == 0)
    return(MagickTrue);
  /* Initialise the reference for the in-memory tile-compression
   */
  ntile_rows=(height+TILE_HEIGHT-1)/TILE_HEIGHT;
  ntile_cols=(width+TILE_WIDTH-1)/TILE_WIDTH;
  ntiles=ntile_rows*ntile_cols;
  for (i = 0; i < (ssize_t) ntiles; i++)
  {
    status=MagickFalse;
    if (offset == 0)
      ThrowBinaryException(CorruptImageError,"NotEnoughTiles",image->filename);
    /* save the current position as it is where the
     *  next tile offset is stored.
     */
    saved_pos=TellBlob(image);
    /* read in the offset of the next tile so we can calculate the amount
       of data needed for this tile*/
    offset2=(MagickOffsetType)ReadBlobMSBLong(image);
    if ((MagickSizeType) offset2 > GetBlobSize(image))
      ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
        image->filename);
    /* if the offset is 0 then we need to read in the maximum possible
       allowing for negative compression */
    if (offset2 == 0)
      offset2=(MagickOffsetType) (offset + TILE_WIDTH * TILE_WIDTH * 4* 1.5);
    /* seek to the tile offset */
    if (SeekBlob(image, offset, SEEK_SET) != offset)
      ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
        image->filename);

      /*
        Allocate the image for the tile.  NOTE: the last tile in a row or
        column may not be a full tile!
      */
      tile_image_width=(size_t) (destLeft == (int) ntile_cols-1 ?
        (int) width % TILE_WIDTH : TILE_WIDTH);
      if (tile_image_width == 0)
        tile_image_width=TILE_WIDTH;
      tile_image_height = (size_t) (destTop == (int) ntile_rows-1 ?
        (int) height % TILE_HEIGHT : TILE_HEIGHT);
      if (tile_image_height == 0)
        tile_image_height=TILE_HEIGHT;
      tile_image=CloneImage(inLayerInfo->image,tile_image_width,
        tile_image_height,MagickTrue,exception);
      if (tile_image == (Image *) NULL)
        ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
          image->filename);
      (void) SetImageBackgroundColor(tile_image,exception);

      /* read in the tile */
      switch (inDocInfo->compression)
      {
        case COMPRESS_NONE:
          if (load_tile(image,tile_image,inDocInfo,inLayerInfo,(size_t) (offset2-offset),exception) == 0)
            status=MagickTrue;
          break;
        case COMPRESS_RLE:
          if (load_tile_rle (image,tile_image,inDocInfo,inLayerInfo,
              (int) (offset2-offset),exception) == 0)
            status=MagickTrue;
          break;
        case COMPRESS_ZLIB:
          tile_image=DestroyImage(tile_image);
          ThrowBinaryException(CoderError,"ZipCompressNotSupported",
            image->filename)
        case COMPRESS_FRACTAL:
          tile_image=DestroyImage(tile_image);
          ThrowBinaryException(CoderError,"FractalCompressNotSupported",
            image->filename)
      }

      /* composite the tile onto the layer's image, and then destroy it */
      (void) CompositeImage(inLayerInfo->image,tile_image,CopyCompositeOp,
        MagickTrue,destLeft * TILE_WIDTH,destTop*TILE_HEIGHT,exception);
      tile_image=DestroyImage(tile_image);

      /* adjust tile position */
      destLeft++;
      if (destLeft >= (int) ntile_cols)
        {
          destLeft = 0;
          destTop++;
        }
      if (status != MagickFalse)
        return(MagickFalse);
      /* restore the saved position so we'll be ready to
       *  read the next offset.
       */
      offset=SeekBlob(image, saved_pos, SEEK_SET);
      /* read in the offset of the next tile */
      offset=(MagickOffsetType) ReadBlobMSBLong(image);
    }
  if (offset != 0)
    ThrowBinaryException(CorruptImageError,"CorruptImage",image->filename)
  return(MagickTrue);
}

static MagickBooleanType load_hierarchy(Image *image,XCFDocInfo *inDocInfo,
   XCFLayerInfo *inLayer, ExceptionInfo *exception)
{
  MagickOffsetType
    saved_pos,
    offset,
    junk;

  size_t
    width,
    height,
    bytes_per_pixel;

  width=ReadBlobMSBLong(image);
  (void) width;
  height=ReadBlobMSBLong(image);
  (void) height;
  bytes_per_pixel=inDocInfo->bytes_per_pixel=ReadBlobMSBLong(image);
  (void) bytes_per_pixel;

  /* load in the levels...we make sure that the number of levels
   *  calculated when the TileManager was created is the same
   *  as the number of levels found in the file.
   */
  offset=(MagickOffsetType) ReadBlobMSBLong(image);  /* top level */

  /* discard offsets for layers below first, if any.
   */
  do
  {
    junk=(MagickOffsetType) ReadBlobMSBLong(image);
  }
  while (junk != 0);

  /* save the current position as it is where the
   *  next level offset is stored.
   */
  saved_pos=TellBlob(image);

  /* seek to the level offset */
  if (SeekBlob(image, offset, SEEK_SET) != offset)
    ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
      image->filename);

  /* read in the level */
  if (load_level (image, inDocInfo, inLayer, exception) == 0)
    return(MagickFalse);
  /* restore the saved position so we'll be ready to
   *  read the next offset.
   */
  offset=SeekBlob(image, saved_pos, SEEK_SET);
  return(MagickTrue);
}

static void InitXCFImage(XCFLayerInfo *outLayer,ExceptionInfo *exception)
{
  outLayer->image->page.x=outLayer->offset_x;
  outLayer->image->page.y=outLayer->offset_y;
  outLayer->image->page.width=outLayer->width;
  outLayer->image->page.height=outLayer->height;
  (void) SetImageProperty(outLayer->image,"label",(char *)outLayer->name,
    exception);
}

static MagickBooleanType ReadOneLayer(const ImageInfo *image_info,Image* image,
  XCFDocInfo* inDocInfo,XCFLayerInfo *outLayer,const ssize_t layer,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  unsigned int
    foundPropEnd = 0;

  size_t
    hierarchy_offset,
    layer_mask_offset;

  /* clear the block! */
  (void) ResetMagickMemory( outLayer, 0, sizeof( XCFLayerInfo ) );
  /* read in the layer width, height, type and name */
  outLayer->width = ReadBlobMSBLong(image);
  outLayer->height = ReadBlobMSBLong(image);
  outLayer->type = ReadBlobMSBLong(image);
  (void) ReadBlobStringWithLongSize(image, outLayer->name,
    sizeof(outLayer->name),exception);
  if (EOFBlob(image) != MagickFalse)
    ThrowBinaryException(CorruptImageError,"InsufficientImageDataInFile",
      image->filename);
  /* read the layer properties! */
  foundPropEnd = 0;
  while ( (foundPropEnd == MagickFalse) && (EOFBlob(image) == MagickFalse) ) {
  PropType    prop_type = (PropType) ReadBlobMSBLong(image);
  size_t  prop_size = ReadBlobMSBLong(image);
    switch (prop_type)
    {
    case PROP_END:
      foundPropEnd = 1;
      break;
    case PROP_ACTIVE_LAYER:
      outLayer->active = 1;
      break;
    case PROP_FLOATING_SELECTION:
      outLayer->floating_offset = ReadBlobMSBLong(image);
      break;
    case PROP_OPACITY:
      outLayer->alpha = ReadBlobMSBLong(image);
      break;
    case PROP_VISIBLE:
      outLayer->visible = ReadBlobMSBLong(image);
      break;
    case PROP_LINKED:
      outLayer->linked = ReadBlobMSBLong(image);
      break;
    case PROP_PRESERVE_TRANSPARENCY:
      outLayer->preserve_trans = ReadBlobMSBLong(image);
      break;
    case PROP_APPLY_MASK:
      outLayer->apply_mask = ReadBlobMSBLong(image);
      break;
    case PROP_EDIT_MASK:
      outLayer->edit_mask = ReadBlobMSBLong(image);
      break;
    case PROP_SHOW_MASK:
      outLayer->show_mask = ReadBlobMSBLong(image);
      break;
    case PROP_OFFSETS:
      outLayer->offset_x = ReadBlobMSBSignedLong(image);
      outLayer->offset_y = ReadBlobMSBSignedLong(image);
      break;
    case PROP_MODE:
      outLayer->mode = ReadBlobMSBLong(image);
      break;
    case PROP_TATTOO:
      outLayer->preserve_trans = ReadBlobMSBLong(image);
      break;
     case PROP_PARASITES:
     {
       if (DiscardBlobBytes(image,prop_size) == MagickFalse)
         ThrowFileException(exception,CorruptImageError,
           "UnexpectedEndOfFile",image->filename);

        /*
       ssize_t base = info->cp;
       GimpParasite *p;
       while (info->cp - base < prop_size)
       {
       p = xcf_load_parasite(info);
       gimp_drawable_parasite_attach(GIMP_DRAWABLE(layer), p);
       gimp_parasite_free(p);
       }
       if (info->cp - base != prop_size)
       g_message ("Error detected while loading a layer's parasites");
       */
     }
     break;
    default:
      /* g_message ("unexpected/unknown layer property: %d (skipping)",
         prop_type); */

      {
      int buf[16];
      ssize_t amount;

      /* read over it... */
      while ((prop_size > 0) && (EOFBlob(image) == MagickFalse))
        {
        amount = (ssize_t) MagickMin(16, prop_size);
        amount = ReadBlob(image, (size_t) amount, (unsigned char *) &buf);
        if (!amount)
          ThrowBinaryException(CorruptImageError,"CorruptImage",
            image->filename);
        prop_size -= (size_t) MagickMin(16, (size_t) amount);
        }
      }
      break;
    }
  }

  if (foundPropEnd == MagickFalse)
    return(MagickFalse);
  /* allocate the image for this layer */
  if (image_info->number_scenes != 0)
    {
      ssize_t
        scene;

      scene=inDocInfo->number_layers-layer-1;
      if (scene > (ssize_t) (image_info->scene+image_info->number_scenes-1))
        {
          outLayer->image=CloneImage(image,0,0,MagickTrue,exception);
          if (outLayer->image == (Image *) NULL)
            return(MagickFalse);
          InitXCFImage(outLayer,exception);
          return(MagickTrue);
        }
    }
  outLayer->image=CloneImage(image,outLayer->width, outLayer->height,MagickTrue,
    exception);
  if (outLayer->image == (Image *) NULL)
    return(MagickFalse);
  status=SetImageExtent(outLayer->image,outLayer->image->columns,
    outLayer->image->rows,exception);
  if (status == MagickFalse)
    {
      outLayer->image=DestroyImageList(outLayer->image);
      return(MagickFalse);
    }
  /* clear the image based on the layer opacity */
  outLayer->image->background_color.alpha=
    ScaleCharToQuantum((unsigned char) outLayer->alpha);
  (void) SetImageBackgroundColor(outLayer->image,exception);

  InitXCFImage(outLayer,exception);

  /* set the compositing mode */
  outLayer->image->compose = GIMPBlendModeToCompositeOperator( outLayer->mode );
  if ( outLayer->visible == MagickFalse )
    {
      /* BOGUS: should really be separate member var! */
      outLayer->image->compose = NoCompositeOp;
    }

  /* read the hierarchy and layer mask offsets */
  hierarchy_offset = ReadBlobMSBLong(image);
  layer_mask_offset = ReadBlobMSBLong(image);

  /* read in the hierarchy */
  offset=SeekBlob(image, (MagickOffsetType) hierarchy_offset, SEEK_SET);
  if (offset != (MagickOffsetType) hierarchy_offset)
    (void) ThrowMagickException(exception,GetMagickModule(),
      CorruptImageError,"InvalidImageHeader","`%s'",image->filename);
  if (load_hierarchy (image, inDocInfo, outLayer, exception) == 0)
    return(MagickFalse);

  /* read in the layer mask */
  if (layer_mask_offset != 0)
    {
      offset=SeekBlob(image, (MagickOffsetType) layer_mask_offset, SEEK_SET);

#if 0  /* BOGUS: support layer masks! */
      layer_mask = xcf_load_layer_mask (info, gimage);
      if (layer_mask == 0)
  goto error;

      /* set the offsets of the layer_mask */
      GIMP_DRAWABLE (layer_mask)->offset_x = GIMP_DRAWABLE (layer)->offset_x;
      GIMP_DRAWABLE (layer_mask)->offset_y = GIMP_DRAWABLE (layer)->offset_y;

      gimp_layer_add_mask (layer, layer_mask, MagickFalse);

      layer->mask->apply_mask = apply_mask;
      layer->mask->edit_mask  = edit_mask;
      layer->mask->show_mask  = show_mask;
#endif
  }

  /* attach the floating selection... */
#if 0  /* BOGUS: we may need to read this, even if we don't support it! */
  if (add_floating_sel)
    {
      GimpLayer *floating_sel;

      floating_sel = info->floating_sel;
      floating_sel_attach (floating_sel, GIMP_DRAWABLE (layer));
    }
#endif

  return MagickTrue;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X C F I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadXCFImage() reads a GIMP (GNU Image Manipulation Program) image
%  file and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadXCFImage method is:
%
%      image=ReadXCFImage(image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadXCFImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    magick[14];

  Image
    *image;

  int
    foundPropEnd = 0;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  register ssize_t
    i;

  size_t
    image_type,
    length;

  ssize_t
    count;

  XCFDocInfo
    doc_info;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  count=ReadBlob(image,14,(unsigned char *) magick);
  if ((count != 14) ||
      (LocaleNCompare((char *) magick,"gimp xcf",8) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  (void) ResetMagickMemory(&doc_info,0,sizeof(XCFDocInfo));
  doc_info.width=ReadBlobMSBLong(image);
  doc_info.height=ReadBlobMSBLong(image);
  if ((doc_info.width > 262144) || (doc_info.height > 262144))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  doc_info.image_type=ReadBlobMSBLong(image);
  /*
    Initialize image attributes.
  */
  image->columns=doc_info.width;
  image->rows=doc_info.height;
  image_type=doc_info.image_type;
  doc_info.file_size=GetBlobSize(image);
  image->compression=NoCompression;
  image->depth=8;
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  if (image_type == GIMP_RGB)
    SetImageColorspace(image,sRGBColorspace,exception);
  else
    if (image_type == GIMP_GRAY)
      SetImageColorspace(image,GRAYColorspace,exception);
    else
      if (image_type == GIMP_INDEXED)
        ThrowReaderException(CoderError,"ColormapTypeNotSupported");
  (void) SetImageBackgroundColor(image,exception);
  (void) SetImageAlpha(image,OpaqueAlpha,exception);
  /*
    Read properties.
  */
  while ((foundPropEnd == MagickFalse) && (EOFBlob(image) == MagickFalse))
  {
    PropType prop_type = (PropType) ReadBlobMSBLong(image);
    size_t prop_size = ReadBlobMSBLong(image);

    switch (prop_type)
    {
      case PROP_END:
        foundPropEnd=1;
        break;
      case PROP_COLORMAP:
      {
        /* Cannot rely on prop_size here--the value is set incorrectly
           by some Gimp versions.
        */
        size_t num_colours = ReadBlobMSBLong(image);
        if (DiscardBlobBytes(image,3*num_colours) == MagickFalse)
          ThrowFileException(exception,CorruptImageError,
            "UnexpectedEndOfFile",image->filename);
    /*
      if (info->file_version == 0)
      {
        gint i;

        g_message (_("XCF warning: version 0 of XCF file format\n"
           "did not save indexed colormaps correctly.\n"
           "Substituting grayscale map."));
        info->cp +=
          xcf_read_int32 (info->fp, (guint32*) &gimage->num_cols, 1);
        gimage->cmap = g_new (guchar, gimage->num_cols*3);
        xcf_seek_pos (info, info->cp + gimage->num_cols);
        for (i = 0; i<gimage->num_cols; i++)
          {
            gimage->cmap[i*3+0] = i;
            gimage->cmap[i*3+1] = i;
            gimage->cmap[i*3+2] = i;
          }
      }
      else
      {
        info->cp +=
          xcf_read_int32 (info->fp, (guint32*) &gimage->num_cols, 1);
        gimage->cmap = g_new (guchar, gimage->num_cols*3);
        info->cp +=
          xcf_read_int8 (info->fp,
                   (guint8*) gimage->cmap, gimage->num_cols*3);
      }
     */
        break;
      }
      case PROP_COMPRESSION:
      {
        doc_info.compression = ReadBlobByte(image);
        if ((doc_info.compression != COMPRESS_NONE) &&
            (doc_info.compression != COMPRESS_RLE) &&
            (doc_info.compression != COMPRESS_ZLIB) &&
            (doc_info.compression != COMPRESS_FRACTAL))
          ThrowReaderException(CorruptImageError,"UnrecognizedImageCompression");
      }
      break;

      case PROP_GUIDES:
      {
         /* just skip it - we don't care about guides */
        if (DiscardBlobBytes(image,prop_size) == MagickFalse)
          ThrowFileException(exception,CorruptImageError,
            "UnexpectedEndOfFile",image->filename);
      }
      break;

    case PROP_RESOLUTION:
      {
        /* float xres = (float) */ (void) ReadBlobMSBLong(image);
        /* float yres = (float) */ (void) ReadBlobMSBLong(image);

        /*
        if (xres < GIMP_MIN_RESOLUTION || xres > GIMP_MAX_RESOLUTION ||
            yres < GIMP_MIN_RESOLUTION || yres > GIMP_MAX_RESOLUTION)
        {
        g_message ("Warning, resolution out of range in XCF file");
        xres = gimage->gimp->config->default_xresolution;
        yres = gimage->gimp->config->default_yresolution;
        }
        */


        /* BOGUS: we don't write these yet because we aren't
              reading them properly yet :(
              image->resolution.x = xres;
              image->resolution.y = yres;
        */
      }
      break;

    case PROP_TATTOO:
      {
        /* we need to read it, even if we ignore it */
        /*size_t  tattoo_state = */ (void) ReadBlobMSBLong(image);
      }
      break;

    case PROP_PARASITES:
      {
        /* BOGUS: we may need these for IPTC stuff */
        if (DiscardBlobBytes(image,prop_size) == MagickFalse)
          ThrowFileException(exception,CorruptImageError,
            "UnexpectedEndOfFile",image->filename);
        /*
      gssize_t         base = info->cp;
      GimpParasite *p;

      while (info->cp - base < prop_size)
        {
          p = xcf_load_parasite (info);
          gimp_image_parasite_attach (gimage, p);
          gimp_parasite_free (p);
        }
      if (info->cp - base != prop_size)
        g_message ("Error detected while loading an image's parasites");
      */
          }
      break;

    case PROP_UNIT:
      {
        /* BOGUS: ignore for now... */
      /*size_t unit =  */ (void) ReadBlobMSBLong(image);
      }
      break;

    case PROP_PATHS:
      {
      /* BOGUS: just skip it for now */
        if (DiscardBlobBytes(image,prop_size) == MagickFalse)
          ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
            image->filename);

        /*
      PathList *paths = xcf_load_bzpaths (gimage, info);
      gimp_image_set_paths (gimage, paths);
      */
      }
      break;

    case PROP_USER_UNIT:
      {
        char  unit_string[1000];
        /*BOGUS: ignored for now */
        /*float  factor = (float) */ (void) ReadBlobMSBLong(image);
        /* size_t digits =  */ (void) ReadBlobMSBLong(image);
        for (i=0; i<5; i++)
         (void) ReadBlobStringWithLongSize(image, unit_string,
           sizeof(unit_string),exception);
      }
     break;

      default:
      {
        int buf[16];
        ssize_t amount;

      /* read over it... */
      while ((prop_size > 0) && (EOFBlob(image) == MagickFalse))
      {
        amount=(ssize_t) MagickMin(16, prop_size);
        amount=(ssize_t) ReadBlob(image,(size_t) amount,(unsigned char *) &buf);
        if (!amount)
          ThrowReaderException(CorruptImageError,"CorruptImage");
        prop_size -= (size_t) MagickMin(16,(size_t) amount);
      }
    }
    break;
  }
  }
  if (foundPropEnd == MagickFalse)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
    {
      ; /* do nothing, were just pinging! */
    }
  else
    {
      int
        current_layer = 0,
        foundAllLayers = MagickFalse,
        number_layers = 0;

      MagickOffsetType
        oldPos=TellBlob(image);

      XCFLayerInfo
        *layer_info;

      /* 
        the read pointer
      */
      do
      {
        ssize_t offset = ReadBlobMSBSignedLong(image);
        if (offset == 0)
          foundAllLayers=MagickTrue;
        else
          number_layers++;
        if (EOFBlob(image) != MagickFalse)
          {
            ThrowFileException(exception,CorruptImageError,
              "UnexpectedEndOfFile",image->filename);
            break;
          }
    } while (foundAllLayers == MagickFalse);
    if (AcquireMagickResource(ListLengthResource,number_layers) == MagickFalse)
      ThrowReaderException(ResourceLimitError,"ListLengthExceedsLimit");
    doc_info.number_layers=number_layers;
    offset=SeekBlob(image,oldPos,SEEK_SET); /* restore the position! */
    if (offset < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    /* allocate our array of layer info blocks */
    length=(size_t) number_layers;
    layer_info=(XCFLayerInfo *) AcquireQuantumMemory(length,
      sizeof(*layer_info));
    if (layer_info == (XCFLayerInfo *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    (void) ResetMagickMemory(layer_info,0,number_layers*sizeof(XCFLayerInfo));
    for ( ; ; )
    {
      MagickBooleanType
        layer_ok;

      MagickOffsetType
        offset,
        saved_pos;

      /* read in the offset of the next layer */
      offset=(MagickOffsetType) ReadBlobMSBLong(image);
      /* if the offset is 0 then we are at the end
      *  of the layer list.
      */
      if (offset == 0)
        break;
      /* save the current position as it is where the
      *  next layer offset is stored.
      */
      saved_pos=TellBlob(image);
      /* seek to the layer offset */
      layer_ok=MagickFalse;
      if (SeekBlob(image,offset,SEEK_SET) == offset)
        {
          /* read in the layer */
          layer_ok=ReadOneLayer(image_info,image,&doc_info,
            &layer_info[current_layer],current_layer,exception);
        }
      if (layer_ok == MagickFalse)
        {
          ssize_t j;

          for (j=0; j <= current_layer; j++)
            if (layer_info[j].image != (Image *) NULL)
              layer_info[j].image=DestroyImage(layer_info[j].image);
          layer_info=(XCFLayerInfo *) RelinquishMagickMemory(layer_info);
          ThrowReaderException(CorruptImageError,"NotEnoughPixelData");
        }
      /* restore the saved position so we'll be ready to
      *  read the next offset.
      */
      offset=SeekBlob(image, saved_pos, SEEK_SET);
      current_layer++;
    }
#if 0
        {
        /* NOTE: XCF layers are REVERSED from composite order! */
        signed int  j;
        for (j=number_layers-1; j>=0; j--) {
          /* BOGUS: need to consider layer blending modes!! */

          if ( layer_info[j].visible ) { /* only visible ones, please! */
            CompositeImage(image, OverCompositeOp, layer_info[j].image,
                     layer_info[j].offset_x, layer_info[j].offset_y );
             layer_info[j].image =DestroyImage( layer_info[j].image );

            /* If we do this, we'll get REAL gray images! */
            if ( image_type == GIMP_GRAY ) {
              QuantizeInfo  qi;
              GetQuantizeInfo(&qi);
              qi.colorspace = GRAYColorspace;
              QuantizeImage( &qi, layer_info[j].image );
            }
          }
        }
      }
#else
      {
        /* NOTE: XCF layers are REVERSED from composite order! */
        ssize_t  j;

        /* now reverse the order of the layers as they are put
           into subimages
        */
        for (j=(ssize_t) number_layers-1; j >= 0; j--)
          AppendImageToList(&image,layer_info[j].image);
      }
#endif

    layer_info=(XCFLayerInfo *) RelinquishMagickMemory(layer_info);

#if 0  /* BOGUS: do we need the channels?? */
    while (MagickTrue)
    {
      /* read in the offset of the next channel */
      info->cp += xcf_read_int32 (info->fp, &offset, 1);

      /* if the offset is 0 then we are at the end
      *  of the channel list.
      */
      if (offset == 0)
        break;

      /* save the current position as it is where the
      *  next channel offset is stored.
      */
      saved_pos = info->cp;

      /* seek to the channel offset */
      xcf_seek_pos (info, offset);

      /* read in the layer */
      channel = xcf_load_channel (info, gimage);
      if (channel == 0)
        goto error;

      num_successful_elements++;

      /* add the channel to the image if its not the selection */
      if (channel != gimage->selection_mask)
        gimp_image_add_channel (gimage, channel, -1);

      /* restore the saved position so we'll be ready to
      *  read the next offset.
      */
      xcf_seek_pos (info, saved_pos);
    }
#endif
  }

  (void) CloseBlob(image);
  if (GetNextImageInList(image) != (Image *) NULL)
    DestroyImage(RemoveFirstImageFromList(&image));
  if (image_type == GIMP_GRAY)
    image->type=GrayscaleType;
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r X C F I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterXCFImage() adds attributes for the XCF image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterXCFImage method is:
%
%      size_t RegisterXCFImage(void)
%
*/
ModuleExport size_t RegisterXCFImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("XCF","XCF","GIMP image");
  entry->decoder=(DecodeImageHandler *) ReadXCFImage;
  entry->magick=(IsImageFormatHandler *) IsXCF;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r X C F I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterXCFImage() removes format registrations made by the
%  XCF module from the list of supported formats.
%
%  The format of the UnregisterXCFImage method is:
%
%      UnregisterXCFImage(void)
%
*/
ModuleExport void UnregisterXCFImage(void)
{
  (void) UnregisterMagickInfo("XCF");
}

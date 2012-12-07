/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            FFFFF  PPPP   X   X                              %
%                            F      P   P   X X                               %
%                            FFF    PPPP     X                                %
%                            F      P       X X                               %
%                            F      P      X   X                              %
%                                                                             %
%                                                                             %
%                     Read/Write FlashPIX Image Format                        %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#if defined(MAGICKCORE_FPX_DELEGATE)
#if !defined(vms) && !defined(macintosh) && !defined(MAGICKCORE_WINDOWS_SUPPORT)
#include <fpxlib.h>
#else
#include "Fpxlib.h"
#endif
#endif

#if defined(MAGICKCORE_FPX_DELEGATE)
/*
  Forward declarations.
*/
static MagickBooleanType
  WriteFPXImage(const ImageInfo *,Image *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s F P X                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsFPX() returns MagickTrue if the image format type, identified by the
%  magick string, is FPX.
%
%  The format of the IsFPX method is:
%
%      MagickBooleanType IsFPX(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsFPX(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\320\317\021\340",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

#if defined(MAGICKCORE_FPX_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d F P X I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFPXImage() reads a FlashPix image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.  This method was contributed by BillR@corbis.com.
%
%  The format of the ReadFPXImage method is:
%
%      Image *ReadFPXImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadFPXImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  FPXColorspace
    colorspace;

  FPXImageComponentDesc
    *alpha_component,
    *blue_component,
    *green_component,
    *red_component;

  FPXImageDesc
    fpx_info;

  FPXImageHandle
    *flashpix;

  FPXStatus
    fpx_status;

  FPXSummaryInformation
    summary_info;

  Image
    *image;

  IndexPacket
    index;

  MagickBooleanType
    status;

  register IndexPacket
    *indexes;

  register ssize_t
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *a,
    *b,
    *g,
    *r;

  size_t
    memory_limit;

  ssize_t
    y;

  unsigned char
    *pixels;

  unsigned int
    height,
    tile_width,
    tile_height,
    width;

  size_t
    scene;

  /*
    Open image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) CloseBlob(image);
  /*
    Initialize FPX toolkit.
  */
  fpx_status=FPX_InitSystem();
  if (fpx_status != FPX_OK)
    ThrowReaderException(CoderError,"UnableToInitializeFPXLibrary");
  memory_limit=20000000;
  fpx_status=FPX_SetToolkitMemoryLimit(&memory_limit);
  if (fpx_status != FPX_OK)
    {
      FPX_ClearSystem();
      ThrowReaderException(CoderError,"UnableToInitializeFPXLibrary");
    }
  tile_width=64;
  tile_height=64;
  flashpix=(FPXImageHandle *) NULL;
  {
#if defined(macintosh)
    FSSpec
      fsspec;

    FilenameToFSSpec(image->filename,&fsspec);
    fpx_status=FPX_OpenImageByFilename((const FSSpec &) fsspec,(char *) NULL,
#else
    fpx_status=FPX_OpenImageByFilename(image->filename,(char *) NULL,
#endif
      &width,&height,&tile_width,&tile_height,&colorspace,&flashpix);
  }
  if (fpx_status == FPX_LOW_MEMORY_ERROR)
    {
      FPX_ClearSystem();
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  if (fpx_status != FPX_OK)
    {
      FPX_ClearSystem();
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        image->filename);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if (colorspace.numberOfComponents == 0)
    {
      FPX_ClearSystem();
      ThrowReaderException(CorruptImageError,"ImageTypeNotSupported");
    }
  if (image_info->view == (char *) NULL)
    {
      float
        aspect_ratio;

      /*
        Get the aspect ratio.
      */
      aspect_ratio=(float) width/height;
      fpx_status=FPX_GetImageResultAspectRatio(flashpix,&aspect_ratio);
      if (fpx_status != FPX_OK)
        ThrowReaderException(DelegateError,"UnableToReadAspectRatio");
      if (width != (size_t) floor((aspect_ratio*height)+0.5))
        Swap(width,height);
    }
  fpx_status=FPX_GetSummaryInformation(flashpix,&summary_info);
  if (fpx_status != FPX_OK)
    {
      FPX_ClearSystem();
      ThrowReaderException(DelegateError,"UnableToReadSummaryInfo");
    }
  if (summary_info.title_valid)
    if ((summary_info.title.length != 0) &&
        (summary_info.title.ptr != (unsigned char *) NULL))
      {
        char
          *label;

        /*
          Note image label.
        */
        label=(char *) NULL;
        if (~summary_info.title.length >= (MaxTextExtent-1))
          label=(char *) AcquireQuantumMemory(summary_info.title.length+
            MaxTextExtent,sizeof(*label));
        if (label == (char *) NULL)
          {
            FPX_ClearSystem();
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        (void) CopyMagickString(label,(char *) summary_info.title.ptr,
          summary_info.title.length+1);
        (void) SetImageProperty(image,"label",label);
        label=DestroyString(label);
      }
  if (summary_info.comments_valid)
    if ((summary_info.comments.length != 0) &&
        (summary_info.comments.ptr != (unsigned char *) NULL))
      {
        char
          *comments;

        /*
          Note image comment.
        */
        comments=(char *) NULL;
        if (~summary_info.comments.length >= (MaxTextExtent-1))
          comments=(char *) AcquireQuantumMemory(summary_info.comments.length+
            MaxTextExtent,sizeof(*comments));
        if (comments == (char *) NULL)
          {
            FPX_ClearSystem();
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        (void) CopyMagickString(comments,(char *) summary_info.comments.ptr,
          summary_info.comments.length+1);
        (void) SetImageProperty(image,"comment",comments);
        comments=DestroyString(comments);
      }
  /*
    Determine resolution by scene specification.
  */
  for (i=1; ; i++)
    if (((width >> i) < tile_width) || ((height >> i) < tile_height))
      break;
  scene=i;
  if (image_info->number_scenes != 0)
    while (scene > image_info->scene)
    {
      width>>=1;
      height>>=1;
      scene--;
    }
  if (image_info->size != (char *) NULL)
    while ((width > image->columns) || (height > image->rows))
    {
      width>>=1;
      height>>=1;
      scene--;
    }
  image->depth=8;
  image->columns=width;
  image->rows=height;
  if ((colorspace.numberOfComponents % 2) == 0)
    image->matte=MagickTrue;
  if (colorspace.numberOfComponents == 1)
    {
      /*
        Create linear colormap.
      */
      if (AcquireImageColormap(image,MaxColormapSize) == MagickFalse)
        {
          FPX_ClearSystem();
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
    }
  if (image_info->ping != MagickFalse)
    {
      (void) FPX_CloseImage(flashpix);
      FPX_ClearSystem();
      return(GetFirstImageInList(image));
    }
  /*
    Allocate memory for the image and pixel buffer.
  */
  pixels=(unsigned char *) AcquireQuantumMemory(image->columns,(tile_height+
    1UL)*colorspace.numberOfComponents*sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    {
      FPX_ClearSystem();
      (void) FPX_CloseImage(flashpix);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Initialize FlashPix image description.
  */
  fpx_info.numberOfComponents=colorspace.numberOfComponents;
  for (i=0; i < 4; i++)
  {
    fpx_info.components[i].myColorType.myDataType=DATA_TYPE_UNSIGNED_BYTE;
    fpx_info.components[i].horzSubSampFactor=1;
    fpx_info.components[i].vertSubSampFactor=1;
    fpx_info.components[i].columnStride=fpx_info.numberOfComponents;
    fpx_info.components[i].lineStride=image->columns*
      fpx_info.components[i].columnStride;
    fpx_info.components[i].theData=pixels+i;
  }
  fpx_info.components[0].myColorType.myColor=fpx_info.numberOfComponents > 2 ?
    NIFRGB_R : MONOCHROME;
  red_component=(&fpx_info.components[0]);
  fpx_info.components[1].myColorType.myColor=fpx_info.numberOfComponents > 2 ?
    NIFRGB_G : ALPHA;
  green_component=(&fpx_info.components[1]);
  fpx_info.components[2].myColorType.myColor=NIFRGB_B;
  blue_component=(&fpx_info.components[2]);
  fpx_info.components[3].myColorType.myColor=ALPHA;
  alpha_component=(&fpx_info.components[fpx_info.numberOfComponents-1]);
  FPX_SetResampleMethod(FPX_LINEAR_INTERPOLATION);
  /*
    Initialize image pixels.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    indexes=GetAuthenticIndexQueue(image);
    if ((y % tile_height) == 0)
      {
        /*
          Read FPX image tile (with or without viewing affine)..
        */
        if (image_info->view != (char *) NULL)
          fpx_status=FPX_ReadImageRectangle(flashpix,0,y,image->columns,y+
            tile_height-1,scene,&fpx_info);
        else
          fpx_status=FPX_ReadImageTransformRectangle(flashpix,0.0F,
            (float) y/image->rows,(float) image->columns/image->rows,
            (float) (y+tile_height-1)/image->rows,(ssize_t) image->columns,
            (ssize_t) tile_height,&fpx_info);
        if (fpx_status == FPX_LOW_MEMORY_ERROR)
          {
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            (void) FPX_CloseImage(flashpix);
            FPX_ClearSystem();
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
      }
    /*
      Transfer a FPX pixels.
    */
    r=red_component->theData+(y % tile_height)*red_component->lineStride;
    g=green_component->theData+(y % tile_height)*green_component->lineStride;
    b=blue_component->theData+(y % tile_height)*blue_component->lineStride;
    a=alpha_component->theData+(y % tile_height)*alpha_component->lineStride;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (fpx_info.numberOfComponents > 2)
        {
          SetPixelRed(q,ScaleCharToQuantum(*r));
          SetPixelGreen(q,ScaleCharToQuantum(*g));
          SetPixelBlue(q,ScaleCharToQuantum(*b));
        }
      else
        {
          index=ScaleCharToQuantum(*r);
          SetPixelIndex(indexes+x,index);
          SetPixelRed(q,index);
          SetPixelGreen(q,index);
          SetPixelBlue(q,index);
        }
      SetPixelOpacity(q,OpaqueOpacity);
      if (image->matte != MagickFalse)
        SetPixelAlpha(q,ScaleCharToQuantum(*a));
      q++;
      r+=red_component->columnStride;
      g+=green_component->columnStride;
      b+=blue_component->columnStride;
      a+=alpha_component->columnStride;
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  (void) FPX_CloseImage(flashpix);
  FPX_ClearSystem();
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r F P X I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterFPXImage() adds attributes for the FPX image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterFPXImage method is:
%
%      size_t RegisterFPXImage(void)
%
*/
ModuleExport size_t RegisterFPXImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("FPX");
#if defined(MAGICKCORE_FPX_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadFPXImage;
  entry->encoder=(EncodeImageHandler *) WriteFPXImage;
#endif
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->blob_support=MagickFalse;
  entry->magick=(IsImageFormatHandler *) IsFPX;
  entry->description=ConstantString("FlashPix Format");
  entry->module=ConstantString("FPX");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r F P X I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterFPXImage() removes format registrations made by the
%  FPX module from the list of supported formats.
%
%  The format of the UnregisterFPXImage method is:
%
%      UnregisterFPXImage(void)
%
*/
ModuleExport void UnregisterFPXImage(void)
{
  (void) UnregisterMagickInfo("FPX");
}

#if defined(MAGICKCORE_FPX_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e F P X I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteFPXImage() writes an image in the FlashPix image format.  This
%  method was contributed by BillR@corbis.com.
%
%  The format of the WriteFPXImage method is:
%
%      MagickBooleanType WriteFPXImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

static void ColorTwistMultiply(FPXColorTwistMatrix first,
  FPXColorTwistMatrix second,FPXColorTwistMatrix *color_twist)
{
  /*
    Matrix multiply.
  */
  assert(color_twist != (FPXColorTwistMatrix *) NULL);
  color_twist->byy=(first.byy*second.byy)+(first.byc1*second.bc1y)+
    (first.byc2*second.bc2y)+(first.dummy1_zero*second.dummy4_zero);
  color_twist->byc1=(first.byy*second.byc1)+(first.byc1*second.bc1c1)+
    (first.byc2*second.bc2c1)+(first.dummy1_zero*second.dummy5_zero);
  color_twist->byc2=(first.byy*second.byc2)+(first.byc1*second.bc1c2)+
    (first.byc2*second.bc2c2)+(first.dummy1_zero*second.dummy6_zero);
  color_twist->dummy1_zero=(first.byy*second.dummy1_zero)+
    (first.byc1*second.dummy2_zero)+(first.byc2*second.dummy3_zero)+
    (first.dummy1_zero*second.dummy7_one);
  color_twist->bc1y=(first.bc1y*second.byy)+(first.bc1c1*second.bc1y)+
    (first.bc1c2*second.bc2y)+(first.dummy2_zero*second.dummy4_zero);
  color_twist->bc1c1=(first.bc1y*second.byc1)+(first.bc1c1*second.bc1c1)+
    (first.bc1c2*second.bc2c1)+(first.dummy2_zero*second.dummy5_zero);
  color_twist->bc1c2=(first.bc1y*second.byc2)+(first.bc1c1*second.bc1c2)+
    (first.bc1c2*second.bc2c2)+(first.dummy2_zero*second.dummy6_zero);
  color_twist->dummy2_zero=(first.bc1y*second.dummy1_zero)+
    (first.bc1c1*second.dummy2_zero)+(first.bc1c2*second.dummy3_zero)+
    (first.dummy2_zero*second.dummy7_one);
  color_twist->bc2y=(first.bc2y*second.byy)+(first.bc2c1*second.bc1y)+
    (first.bc2c2*second.bc2y)+(first.dummy3_zero*second.dummy4_zero);
  color_twist->bc2c1=(first.bc2y*second.byc1)+(first.bc2c1*second.bc1c1)+
    (first.bc2c2*second.bc2c1)+(first.dummy3_zero*second.dummy5_zero);
  color_twist->bc2c2=(first.bc2y*second.byc2)+(first.bc2c1*second.bc1c2)+
    (first.bc2c2*second.bc2c2)+(first.dummy3_zero*second.dummy6_zero);
  color_twist->dummy3_zero=(first.bc2y*second.dummy1_zero)+
    (first.bc2c1*second.dummy2_zero)+(first.bc2c2*second.dummy3_zero)+
    (first.dummy3_zero*second.dummy7_one);
  color_twist->dummy4_zero=(first.dummy4_zero*second.byy)+
    (first.dummy5_zero*second.bc1y)+(first.dummy6_zero*second.bc2y)+
    (first.dummy7_one*second.dummy4_zero);
  color_twist->dummy5_zero=(first.dummy4_zero*second.byc1)+
    (first.dummy5_zero*second.bc1c1)+(first.dummy6_zero*second.bc2c1)+
    (first.dummy7_one*second.dummy5_zero);
  color_twist->dummy6_zero=(first.dummy4_zero*second.byc2)+
    (first.dummy5_zero*second.bc1c2)+(first.dummy6_zero*second.bc2c2)+
    (first.dummy7_one*second.dummy6_zero);
  color_twist->dummy7_one=(first.dummy4_zero*second.dummy1_zero)+
    (first.dummy5_zero*second.dummy2_zero)+
    (first.dummy6_zero*second.dummy3_zero)+(first.dummy7_one*second.dummy7_one);
}

static void SetBrightness(double brightness,FPXColorTwistMatrix *color_twist)
{
  FPXColorTwistMatrix
    effect,
    result;

  /*
    Set image brightness in color twist matrix.
  */
  assert(color_twist != (FPXColorTwistMatrix *) NULL);
  brightness=sqrt((double) brightness);
  effect.byy=brightness;
  effect.byc1=0.0;
  effect.byc2=0.0;
  effect.dummy1_zero=0.0;
  effect.bc1y=0.0;
  effect.bc1c1=brightness;
  effect.bc1c2=0.0;
  effect.dummy2_zero=0.0;
  effect.bc2y=0.0;
  effect.bc2c1=0.0;
  effect.bc2c2=brightness;
  effect.dummy3_zero=0.0;
  effect.dummy4_zero=0.0;
  effect.dummy5_zero=0.0;
  effect.dummy6_zero=0.0;
  effect.dummy7_one=1.0;
  ColorTwistMultiply(*color_twist,effect,&result);
  *color_twist=result;
}

static void SetColorBalance(double red,double green,double blue,
  FPXColorTwistMatrix *color_twist)
{
  FPXColorTwistMatrix
    blue_effect,
    green_effect,
    result,
    rgb_effect,
    rg_effect,
    red_effect;

  /*
    Set image color balance in color twist matrix.
  */
  assert(color_twist != (FPXColorTwistMatrix *) NULL);
  red=sqrt((double) red)-1.0;
  green=sqrt((double) green)-1.0;
  blue=sqrt((double) blue)-1.0;
  red_effect.byy=1.0;
  red_effect.byc1=0.0;
  red_effect.byc2=0.299*red;
  red_effect.dummy1_zero=0.0;
  red_effect.bc1y=(-0.299)*red;
  red_effect.bc1c1=1.0-0.299*red;
  red_effect.bc1c2=(-0.299)*red;
  red_effect.dummy2_zero=0.0;
  red_effect.bc2y=0.701*red;
  red_effect.bc2c1=0.0;
  red_effect.bc2c2=1.0+0.402*red;
  red_effect.dummy3_zero=0.0;
  red_effect.dummy4_zero=0.0;
  red_effect.dummy5_zero=0.0;
  red_effect.dummy6_zero=0.0;
  red_effect.dummy7_one=1.0;
  green_effect.byy=1.0;
  green_effect.byc1=(-0.114)*green;
  green_effect.byc2=(-0.299)*green;
  green_effect.dummy1_zero=0.0;
  green_effect.bc1y=(-0.587)*green;
  green_effect.bc1c1=1.0-0.473*green;
  green_effect.bc1c2=0.299*green;
  green_effect.dummy2_zero=0.0;
  green_effect.bc2y=(-0.587)*green;
  green_effect.bc2c1=0.114*green;
  green_effect.bc2c2=1.0-0.288*green;
  green_effect.dummy3_zero=0.0;
  green_effect.dummy4_zero=0.0;
  green_effect.dummy5_zero=0.0;
  green_effect.dummy6_zero=0.0;
  green_effect.dummy7_one=1.0;
  blue_effect.byy=1.0;
  blue_effect.byc1=0.114*blue;
  blue_effect.byc2=0.0;
  blue_effect.dummy1_zero=0.0;
  blue_effect.bc1y=0.886*blue;
  blue_effect.bc1c1=1.0+0.772*blue;
  blue_effect.bc1c2=0.0;
  blue_effect.dummy2_zero=0.0;
  blue_effect.bc2y=(-0.114)*blue;
  blue_effect.bc2c1=(-0.114)*blue;
  blue_effect.bc2c2=1.0-0.114*blue;
  blue_effect.dummy3_zero=0.0;
  blue_effect.dummy4_zero=0.0;
  blue_effect.dummy5_zero=0.0;
  blue_effect.dummy6_zero=0.0;
  blue_effect.dummy7_one=1.0;
  ColorTwistMultiply(red_effect,green_effect,&rg_effect);
  ColorTwistMultiply(rg_effect,blue_effect,&rgb_effect);
  ColorTwistMultiply(*color_twist,rgb_effect,&result);
  *color_twist=result;
}

static void SetSaturation(double saturation,FPXColorTwistMatrix *color_twist)
{
  FPXColorTwistMatrix
    effect,
    result;

  /*
    Set image saturation in color twist matrix.
  */
  assert(color_twist != (FPXColorTwistMatrix *) NULL);
  effect.byy=1.0;
  effect.byc1=0.0;
  effect.byc2=0.0;
  effect.dummy1_zero=0.0;
  effect.bc1y=0.0;
  effect.bc1c1=saturation;
  effect.bc1c2=0.0;
  effect.dummy2_zero=0.0;
  effect.bc2y=0.0;
  effect.bc2c1=0.0;
  effect.bc2c2=saturation;
  effect.dummy3_zero=0.0;
  effect.dummy4_zero=0.0;
  effect.dummy5_zero=0.0;
  effect.dummy6_zero=0.0;
  effect.dummy7_one=1.0;
  ColorTwistMultiply(*color_twist,effect,&result);
  *color_twist=result;
}

static MagickBooleanType WriteFPXImage(const ImageInfo *image_info,Image *image)
{
  FPXBackground
    background_color;

  FPXColorspace
    colorspace =
    {
      TRUE, 4,
      {
        { NIFRGB_R, DATA_TYPE_UNSIGNED_BYTE },
        { NIFRGB_G, DATA_TYPE_UNSIGNED_BYTE },
        { NIFRGB_B, DATA_TYPE_UNSIGNED_BYTE },
        { ALPHA, DATA_TYPE_UNSIGNED_BYTE }
      }
    };

  const char
    *comment,
    *label;

  FPXCompressionOption
    compression;

  FPXImageDesc
    fpx_info;

  FPXImageHandle
    *flashpix;

  FPXStatus
    fpx_status;

  FPXSummaryInformation
    summary_info;

  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register const PixelPacket
    *p;

  register ssize_t
    i;

  size_t
    length,
    memory_limit;

  ssize_t
    y;

  unsigned char
    *pixels;

  unsigned int
    tile_height,
    tile_width;

  /*
    Open input file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace);
  (void) CloseBlob(image);
  /*
    Initialize FPX toolkit.
  */
  image->depth=8;
  memory_limit=20000000;
  fpx_status=FPX_SetToolkitMemoryLimit(&memory_limit);
  if (fpx_status != FPX_OK)
    ThrowWriterException(DelegateError,"UnableToInitializeFPXLibrary");
  tile_width=64;
  tile_height=64;
  colorspace.numberOfComponents=3;
  if (image->matte != MagickFalse)
    colorspace.numberOfComponents=4;
  if ((image_info->type != TrueColorType) &&
      IsGrayImage(image,&image->exception))
    {
      colorspace.numberOfComponents=1;
      colorspace.theComponents[0].myColor=MONOCHROME;
    }
  background_color.color1_value=0;
  background_color.color2_value=0;
  background_color.color3_value=0;
  background_color.color4_value=0;
  compression=NONE;
  if (image->compression == JPEGCompression)
    compression=JPEG_UNSPECIFIED;
  if (image_info->compression == JPEGCompression)
    compression=JPEG_UNSPECIFIED;
  {
#if defined(macintosh)
    FSSpec
      fsspec;

    FilenameToFSSpec(filename,&fsspec);
    fpx_status=FPX_CreateImageByFilename((const FSSpec &) fsspec,image->columns,
#else
    fpx_status=FPX_CreateImageByFilename(image->filename,image->columns,
#endif
      image->rows,tile_width,tile_height,colorspace,background_color,
      compression,&flashpix);
  }
  if (fpx_status != FPX_OK)
    return(status);
  if (compression == JPEG_UNSPECIFIED)
    {
      /*
        Initialize the compression by quality for the entire image.
      */
      fpx_status=FPX_SetJPEGCompression(flashpix,(unsigned short)
        image->quality == UndefinedCompressionQuality ? 75 : image->quality);
      if (fpx_status != FPX_OK)
        ThrowWriterException(DelegateError,"UnableToSetJPEGLevel");
    }
  /*
    Set image summary info.
  */
  summary_info.title_valid=MagickFalse;
  summary_info.subject_valid=MagickFalse;
  summary_info.author_valid=MagickFalse;
  summary_info.comments_valid=MagickFalse;
  summary_info.keywords_valid=MagickFalse;
  summary_info.OLEtemplate_valid=MagickFalse;
  summary_info.last_author_valid=MagickFalse;
  summary_info.rev_number_valid=MagickFalse;
  summary_info.edit_time_valid=MagickFalse;
  summary_info.last_printed_valid=MagickFalse;
  summary_info.create_dtm_valid=MagickFalse;
  summary_info.last_save_dtm_valid=MagickFalse;
  summary_info.page_count_valid=MagickFalse;
  summary_info.word_count_valid=MagickFalse;
  summary_info.char_count_valid=MagickFalse;
  summary_info.thumbnail_valid=MagickFalse;
  summary_info.appname_valid=MagickFalse;
  summary_info.security_valid=MagickFalse;
  label=GetImageProperty(image,"label");
  if (label != (const char *) NULL)
    {
      size_t
        length;

      /*
        Note image label.
      */
      summary_info.title_valid=MagickTrue;
      length=strlen(label);
      summary_info.title.length=length;
      if (~length >= (MaxTextExtent-1))
        summary_info.title.ptr=(unsigned char *) AcquireQuantumMemory(
          length+MaxTextExtent,sizeof(*summary_info.title.ptr));
      if (summary_info.title.ptr == (unsigned char *) NULL)
        ThrowWriterException(DelegateError,"UnableToSetImageTitle");
      (void) CopyMagickString((char *) summary_info.title.ptr,label,
        MaxTextExtent);
    }
  comment=GetImageProperty(image,"comment");
  if (comment != (const char *) NULL)
    {
      /*
        Note image comment.
      */
      summary_info.comments_valid=MagickTrue;
      summary_info.comments.ptr=(unsigned char *) AcquireString(comment);
      summary_info.comments.length=strlen(comment);
    }
  fpx_status=FPX_SetSummaryInformation(flashpix,&summary_info);
  if (fpx_status != FPX_OK)
    ThrowWriterException(DelegateError,"UnableToSetSummaryInfo");
  /*
    Initialize FlashPix image description.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  pixels=GetQuantumPixels(quantum_info);
  fpx_info.numberOfComponents=colorspace.numberOfComponents;
  for (i=0; i < (ssize_t) fpx_info.numberOfComponents; i++)
  {
    fpx_info.components[i].myColorType.myDataType=DATA_TYPE_UNSIGNED_BYTE;
    fpx_info.components[i].horzSubSampFactor=1;
    fpx_info.components[i].vertSubSampFactor=1;
    fpx_info.components[i].columnStride=fpx_info.numberOfComponents;
    fpx_info.components[i].lineStride=
      image->columns*fpx_info.components[i].columnStride;
    fpx_info.components[i].theData=pixels+i;
  }
  fpx_info.components[0].myColorType.myColor=fpx_info.numberOfComponents != 1
    ? NIFRGB_R : MONOCHROME;
  fpx_info.components[1].myColorType.myColor=NIFRGB_G;
  fpx_info.components[2].myColorType.myColor=NIFRGB_B;
  fpx_info.components[3].myColorType.myColor=ALPHA;
  /*
    Write image pixelss.
  */
  quantum_type=RGBQuantum;
  if (image->matte != MagickFalse)
    quantum_type=RGBAQuantum;
  if (fpx_info.numberOfComponents == 1)
    quantum_type=GrayQuantum;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    length=ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
      quantum_type,pixels,&image->exception);
    fpx_status=FPX_WriteImageLine(flashpix,&fpx_info);
    if (fpx_status != FPX_OK)
      break;
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
  if (image_info->view != (char *) NULL)
    {
      FPXAffineMatrix
        affine;

      FPXColorTwistMatrix
        color_twist;

      FPXContrastAdjustment
        contrast;

      FPXFilteringValue
        sharpen;

      FPXResultAspectRatio
        aspect_ratio;

      FPXROI
        view_rect;

      MagickBooleanType
        affine_valid,
        aspect_ratio_valid,
        color_twist_valid,
        contrast_valid,
        sharpen_valid,
        view_rect_valid;

      /*
        Initialize default viewing parameters.
      */
      contrast=1.0;
      contrast_valid=MagickFalse;
      color_twist.byy=1.0;
      color_twist.byc1=0.0;
      color_twist.byc2=0.0;
      color_twist.dummy1_zero=0.0;
      color_twist.bc1y=0.0;
      color_twist.bc1c1=1.0;
      color_twist.bc1c2=0.0;
      color_twist.dummy2_zero=0.0;
      color_twist.bc2y=0.0;
      color_twist.bc2c1=0.0;
      color_twist.bc2c2=1.0;
      color_twist.dummy3_zero=0.0;
      color_twist.dummy4_zero=0.0;
      color_twist.dummy5_zero=0.0;
      color_twist.dummy6_zero=0.0;
      color_twist.dummy7_one=1.0;
      color_twist_valid=MagickFalse;
      sharpen=0.0;
      sharpen_valid=MagickFalse;
      aspect_ratio=(double) image->columns/image->rows;
      aspect_ratio_valid=MagickFalse;
      view_rect.left=(float) 0.1;
      view_rect.width=aspect_ratio-0.2;
      view_rect.top=(float) 0.1;
      view_rect.height=(float) 0.8; /* 1.0-0.2 */
      view_rect_valid=MagickFalse;
      affine.a11=1.0;
      affine.a12=0.0;
      affine.a13=0.0;
      affine.a14=0.0;
      affine.a21=0.0;
      affine.a22=1.0;
      affine.a23=0.0;
      affine.a24=0.0;
      affine.a31=0.0;
      affine.a32=0.0;
      affine.a33=1.0;
      affine.a34=0.0;
      affine.a41=0.0;
      affine.a42=0.0;
      affine.a43=0.0;
      affine.a44=1.0;
      affine_valid=MagickFalse;
      if (0)
        {
          /*
            Color color twist.
          */
          SetBrightness(0.5,&color_twist);
          SetSaturation(0.5,&color_twist);
          SetColorBalance(0.5,1.0,1.0,&color_twist);
          color_twist_valid=MagickTrue;
        }
      if (affine_valid)
        {
          fpx_status=FPX_SetImageAffineMatrix(flashpix,&affine);
          if (fpx_status != FPX_OK)
            ThrowWriterException(DelegateError,"UnableToSetAffineMatrix");
        }
      if (aspect_ratio_valid)
        {
          fpx_status=FPX_SetImageResultAspectRatio(flashpix,&aspect_ratio);
          if (fpx_status != FPX_OK)
            ThrowWriterException(DelegateError,"UnableToSetAspectRatio");
        }
      if (color_twist_valid)
        {
          fpx_status=FPX_SetImageColorTwistMatrix(flashpix,&color_twist);
          if (fpx_status != FPX_OK)
            ThrowWriterException(DelegateError,"UnableToSetColorTwist");
        }
      if (contrast_valid)
        {
          fpx_status=FPX_SetImageContrastAdjustment(flashpix,&contrast);
          if (fpx_status != FPX_OK)
            ThrowWriterException(DelegateError,"UnableToSetContrast");
        }
      if (sharpen_valid)
        {
          fpx_status=FPX_SetImageFilteringValue(flashpix,&sharpen);
          if (fpx_status != FPX_OK)
            ThrowWriterException(DelegateError,"UnableToSetFilteringValue");
        }
      if (view_rect_valid)
        {
          fpx_status=FPX_SetImageROI(flashpix,&view_rect);
          if (fpx_status != FPX_OK)
            ThrowWriterException(DelegateError,"UnableToSetRegionOfInterest");
        }
    }
  (void) FPX_CloseImage(flashpix);
  FPX_ClearSystem();
  return(MagickTrue);
}
#endif

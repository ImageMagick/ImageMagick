/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               CCCC  H   H   AAA   N   N  N   N  EEEEE   L                   %
%              C      H   H  A   A  NN  N  NN  N  E       L                   %
%              C      HHHHH  AAAAA  N N N  N N N  RRR     L                   %
%              C      H   H  A   A  N  NN  N  NN  E       L                   %
%               CCCC  H   H  A   A  N   N  N   N  EEEEE   LLLLL               %
%                                                                             %
%                                                                             %
%                      MagickCore Image Channel Methods                       %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               December 2003                                 %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/cache-private.h"
#include "magick/channel.h"
#include "magick/color-private.h"
#include "magick/colorspace-private.h"
#include "magick/composite-private.h"
#include "magick/exception-private.h"
#include "magick/enhance.h"
#include "magick/image.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-accessor.h"
#include "magick/resource_.h"
#include "magick/string-private.h"
#include "magick/thread-private.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o m b i n e I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CombineImages() combines one or more images into a single image.  The
%  grayscale value of the pixels of each image in the sequence is assigned in
%  order to the specified channels of the combined image.   The typical
%  ordering would be image 1 => Red, 2 => Green, 3 => Blue, etc.
%
%  The format of the CombineImages method is:
%
%      Image *CombineImages(const Image *image,const ChannelType channel,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *CombineImages(const Image *image,const ChannelType channel,
  ExceptionInfo *exception)
{
#define CombineImageTag  "Combine/Image"

  CacheView
    *combine_view;

  const Image
    *next;

  Image
    *combine_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  /*
    Ensure the image are the same size.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if ((next->columns != image->columns) || (next->rows != image->rows))
      ThrowImageException(OptionError,"ImagesAreNotTheSameSize");
  }
  combine_image=CloneImage(image,0,0,MagickTrue,exception);
  if (combine_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(combine_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&combine_image->exception);
      combine_image=DestroyImage(combine_image);
      return((Image *) NULL);
    }
  if (IsGrayColorspace(image->colorspace) != MagickFalse)
    (void) SetImageColorspace(combine_image,RGBColorspace);
  if ((channel & OpacityChannel) != 0)
    combine_image->matte=MagickTrue;
  (void) SetImageBackgroundColor(combine_image);
  /*
    Combine images.
  */
  status=MagickTrue;
  progress=0;
  combine_view=AcquireAuthenticCacheView(combine_image,exception);
  for (y=0; y < (ssize_t) combine_image->rows; y++)
  {
    CacheView
      *image_view;

    const Image
      *next;

    PixelPacket
      *pixels;

    register const PixelPacket
      *restrict p;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewAuthenticPixels(combine_view,0,y,combine_image->columns,
      1,exception);
    if (pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    next=image;
    if (((channel & RedChannel) != 0) && (next != (Image *) NULL))
      {
        image_view=AcquireVirtualCacheView(next,exception);
        p=GetCacheViewVirtualPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          continue;
        q=pixels;
        for (x=0; x < (ssize_t) combine_image->columns; x++)
        {
          SetPixelRed(q,PixelIntensityToQuantum(image,p));
          p++;
          q++;
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
    if (((channel & GreenChannel) != 0) && (next != (Image *) NULL))
      {
        image_view=AcquireVirtualCacheView(next,exception);
        p=GetCacheViewVirtualPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          continue;
        q=pixels;
        for (x=0; x < (ssize_t) combine_image->columns; x++)
        {
          SetPixelGreen(q,PixelIntensityToQuantum(image,p));
          p++;
          q++;
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
    if (((channel & BlueChannel) != 0) && (next != (Image *) NULL))
      {
        image_view=AcquireVirtualCacheView(next,exception);
        p=GetCacheViewVirtualPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          continue;
        q=pixels;
        for (x=0; x < (ssize_t) combine_image->columns; x++)
        {
          SetPixelBlue(q,PixelIntensityToQuantum(image,p));
          p++;
          q++;
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
    if (((channel & OpacityChannel) != 0) && (next != (Image *) NULL))
      {
        image_view=AcquireVirtualCacheView(next,exception);
        p=GetCacheViewVirtualPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          continue;
        q=pixels;
        for (x=0; x < (ssize_t) combine_image->columns; x++)
        {
          SetPixelAlpha(q,PixelIntensityToQuantum(image,p));
          p++;
          q++;
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
    if (((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace) && (next != (Image *) NULL))
      {
        IndexPacket
          *indexes;

        image_view=AcquireVirtualCacheView(next,exception);
        p=GetCacheViewVirtualPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          continue;
        indexes=GetCacheViewAuthenticIndexQueue(combine_view);
        for (x=0; x < (ssize_t) combine_image->columns; x++)
        {
          SetPixelIndex(indexes+x,PixelIntensityToQuantum(image,p));
          p++;
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
    if (SyncCacheViewAuthenticPixels(combine_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

        proceed=SetImageProgress(image,CombineImageTag,progress++,
          combine_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  combine_view=DestroyCacheView(combine_view);
  if (IsGrayColorspace(combine_image->colorspace) != MagickFalse)
    (void) TransformImageColorspace(combine_image,RGBColorspace);
  if (status == MagickFalse)
    combine_image=DestroyImage(combine_image);
  return(combine_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e A l p h a C h a n n e l                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageAlphaChannel() returns MagickFalse if the image alpha channel is
%  not activated.  That is, the image is RGB rather than RGBA or CMYK rather
%  than CMYKA.
%
%  The format of the GetImageAlphaChannel method is:
%
%      MagickBooleanType GetImageAlphaChannel(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType GetImageAlphaChannel(const Image *image)
{
  assert(image != (const Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  return(image->matte);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S e p a r a t e I m a g e C h a n n e l                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SeparateImageChannel() separates a channel from the image and returns it as
%  a grayscale image.  A channel is a particular color component of each pixel
%  in the image.
%
%  The format of the SeparateImageChannel method is:
%
%      MagickBooleanType SeparateImageChannel(Image *image,
%        const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: Identify which channel to extract: RedChannel, GreenChannel,
%      BlueChannel, OpacityChannel, CyanChannel, MagentaChannel,
%      YellowChannel, or BlackChannel.
%
*/
MagickExport MagickBooleanType SeparateImageChannel(Image *image,
  const ChannelType channel)
{
#define SeparateImageTag  "Separate/Image"

  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (channel == GrayChannels)
    image->matte=MagickTrue;
  /*
    Separate image channels.
  */
  status=MagickTrue;
  progress=0;
  exception=(&image->exception);
  image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(progress,status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register IndexPacket
      *restrict indexes;

    register PixelPacket
      *restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    switch (channel)
    {
      case RedChannel:
      {
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelGreen(q,GetPixelRed(q));
          SetPixelBlue(q,GetPixelRed(q));
          q++;
        }
        break;
      }
      case GreenChannel:
      {
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRed(q,GetPixelGreen(q));
          SetPixelBlue(q,GetPixelGreen(q));
          q++;
        }
        break;
      }
      case BlueChannel:
      {
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRed(q,GetPixelBlue(q));
          SetPixelGreen(q,GetPixelBlue(q));
          q++;
        }
        break;
      }
      case OpacityChannel:
      {
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRed(q,GetPixelOpacity(q));
          SetPixelGreen(q,GetPixelOpacity(q));
          SetPixelBlue(q,GetPixelOpacity(q));
          q++;
        }
        break;
      }
      case BlackChannel:
      {
        if ((image->storage_class != PseudoClass) &&
            (image->colorspace != CMYKColorspace))
          break;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRed(q,GetPixelIndex(indexes+x));
          SetPixelGreen(q,GetPixelIndex(indexes+x));
          SetPixelBlue(q,GetPixelIndex(indexes+x));
          q++;
        }
        break;
      }
      case TrueAlphaChannel:
      {
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRed(q,GetPixelAlpha(q));
          SetPixelGreen(q,GetPixelAlpha(q));
          SetPixelBlue(q,GetPixelAlpha(q));
          q++;
        }
        break;
      }
      case GrayChannels:
      {
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelAlpha(q,PixelIntensityToQuantum(image,q));
          q++;
        }
        break;
      }
      default:
        break;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_SeparateImageChannel)
#endif
        proceed=SetImageProgress(image,SeparateImageTag,progress++,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  if (channel != GrayChannels)
    image->matte=MagickFalse;
  if (IssRGBColorspace(image->colorspace) == MagickFalse)
    (void) SetImageColorspace(image,GRAYColorspace);
  else
    (void) TransformImageColorspace(image,GRAYColorspace);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S e p a r a t e I m a g e s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SeparateImages() returns a separate grayscale image for each channel
%  specified.
%
%  The format of the SeparateImages method is:
%
%      MagickBooleanType SeparateImages(const Image *image,
%        const ChannelType channel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: Identify which channels to extract: RedChannel, GreenChannel,
%      BlueChannel, OpacityChannel, CyanChannel, MagentaChannel,
%      YellowChannel, or BlackChannel.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SeparateImages(const Image *image,const ChannelType channel,
  ExceptionInfo *exception)
{
  Image
    *images,
    *separate_image;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  images=NewImageList();
  if ((channel & RedChannel) != 0)
    {
      separate_image=CloneImage(image,0,0,MagickTrue,exception);
      (void) SeparateImageChannel(separate_image,RedChannel);
      AppendImageToList(&images,separate_image);
    }
  if ((channel & GreenChannel) != 0)
    {
      separate_image=CloneImage(image,0,0,MagickTrue,exception);
      (void) SeparateImageChannel(separate_image,GreenChannel);
      AppendImageToList(&images,separate_image);
    }
  if ((channel & BlueChannel) != 0)
    {
      separate_image=CloneImage(image,0,0,MagickTrue,exception);
      (void) SeparateImageChannel(separate_image,BlueChannel);
      AppendImageToList(&images,separate_image);
    }
  if (((channel & BlackChannel) != 0) && (image->colorspace == CMYKColorspace))
    {
      separate_image=CloneImage(image,0,0,MagickTrue,exception);
      (void) SeparateImageChannel(separate_image,BlackChannel);
      AppendImageToList(&images,separate_image);
    }
  if ((channel & AlphaChannel) != 0)
    {
      separate_image=CloneImage(image,0,0,MagickTrue,exception);
      (void) SeparateImageChannel(separate_image,TrueAlphaChannel);
      AppendImageToList(&images,separate_image);
    }
  return(images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e A l p h a C h a n n e l                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageAlphaChannel() activates, deactivates, resets, or sets the alpha
%  channel.
%
%  The format of the SetImageAlphaChannel method is:
%
%      MagickBooleanType SetImageAlphaChannel(Image *image,
%        const AlphaChannelType alpha_type)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o alpha_type:  The alpha channel type: ActivateAlphaChannel,
%      CopyAlphaChannel, DeactivateAlphaChannel, ExtractAlphaChannel,
%      OpaqueAlphaChannel, ResetAlphaChannel, SetAlphaChannel,
%      ShapeAlphaChannel, and TransparentAlphaChannel.
%
*/
MagickExport MagickBooleanType SetImageAlphaChannel(Image *image,
  const AlphaChannelType alpha_type)
{
  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  status=MagickTrue;
  switch (alpha_type)
  {
    case ActivateAlphaChannel:
    {
      image->matte=MagickTrue;
      break;
    }
    case BackgroundAlphaChannel:
    {
      CacheView
        *image_view;

      ExceptionInfo
        *exception;

      IndexPacket
        index;

      MagickBooleanType
        status;

      MagickPixelPacket
        background;

      PixelPacket
        pixel;

      ssize_t
        y;

      /*
        Set transparent pixels to background color.
      */
      if (image->matte == MagickFalse)
        break;
      if (SetImageStorageClass(image,DirectClass) == MagickFalse)
        break;
      GetMagickPixelPacket(image,&background);
      SetMagickPixelPacket(image,&image->background_color,(const IndexPacket *)
        NULL,&background);
      if (image->colorspace == CMYKColorspace)
        ConvertRGBToCMYK(&background);
      index=0;
      SetPixelPacket(image,&background,&pixel,&index);
      status=MagickTrue;
      exception=(&image->exception);
      image_view=AcquireAuthenticCacheView(image,exception);
      #if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp parallel for schedule(static,4) shared(status) \
          dynamic_number_threads(image,image->columns,image->rows,1)
      #endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        register IndexPacket
          *restrict indexes;

        register PixelPacket
          *restrict q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          if (q->opacity == TransparentOpacity)
            {
              SetPixelRed(q,pixel.red);
              SetPixelGreen(q,pixel.green);
              SetPixelBlue(q,pixel.blue);
            }
          q++;
        }
        if (image->colorspace == CMYKColorspace)
          {
            indexes=GetCacheViewAuthenticIndexQueue(image_view);
            for (x=0; x < (ssize_t) image->columns; x++)
              SetPixelIndex(indexes+x,index);
          }
        if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      return(status);
    }
    case CopyAlphaChannel:
    case ShapeAlphaChannel:
    {
      /*
        Special usage case for SeparateImageChannel(): copy grayscale color to
        the alpha channel.
      */
      status=SeparateImageChannel(image,GrayChannels);
      image->matte=MagickTrue; /* make sure transparency is now on! */
      if (alpha_type == ShapeAlphaChannel)
        {
          MagickPixelPacket
            background;

          /*
            Reset all color channels to background color.
          */
          GetMagickPixelPacket(image,&background);
          SetMagickPixelPacket(image,&(image->background_color),(IndexPacket *)
            NULL,&background);
          (void) LevelColorsImage(image,&background,&background,MagickTrue);
        }
      break;
    }
    case DeactivateAlphaChannel:
    {
      image->matte=MagickFalse;
      break;
    }
    case ExtractAlphaChannel:
    {
      status=SeparateImageChannel(image,TrueAlphaChannel);
      image->matte=MagickFalse;
      break;
    }
    case RemoveAlphaChannel:
    case FlattenAlphaChannel:
    {
      CacheView
        *image_view;

      ExceptionInfo
        *exception;

      IndexPacket
        index;

      MagickBooleanType
        status;

      MagickPixelPacket
        background;

      PixelPacket
        pixel;

      ssize_t
        y;

      /*
        Flatten image pixels over the background pixels.
      */
      if (image->matte == MagickFalse)
        break;
      if (SetImageStorageClass(image,DirectClass) == MagickFalse)
        break;
      GetMagickPixelPacket(image,&background);
      SetMagickPixelPacket(image,&image->background_color,(const IndexPacket *)
        NULL,&background);
      if (image->colorspace == CMYKColorspace)
        ConvertRGBToCMYK(&background);
      index=0;
      SetPixelPacket(image,&background,&pixel,&index);
      status=MagickTrue;
      exception=(&image->exception);
      image_view=AcquireAuthenticCacheView(image,exception);
      #if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp parallel for schedule(static,4) shared(status) \
          dynamic_number_threads(image,image->columns,image->rows,1)
      #endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        register IndexPacket
          *restrict indexes;

        register PixelPacket
          *restrict q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (PixelPacket *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          MagickRealType
            gamma,
            opacity;

          gamma=1.0-QuantumScale*QuantumScale*q->opacity*pixel.opacity;
          opacity=(MagickRealType) QuantumRange*(1.0-gamma);
          gamma=PerceptibleReciprocal(gamma);
          q->red=ClampToQuantum(gamma*MagickOver_((MagickRealType) q->red,
            (MagickRealType) q->opacity,(MagickRealType) pixel.red,
            (MagickRealType) pixel.opacity));
          q->green=ClampToQuantum(gamma*MagickOver_((MagickRealType) q->green,
            (MagickRealType) q->opacity,(MagickRealType) pixel.green,
            (MagickRealType) pixel.opacity));
          q->blue=ClampToQuantum(gamma*MagickOver_((MagickRealType) q->blue,
            (MagickRealType) q->opacity,(MagickRealType) pixel.blue,
            (MagickRealType) pixel.opacity));
          q->opacity=ClampToQuantum(opacity);
          q++;
        }
        if (image->colorspace == CMYKColorspace)
          {
            indexes=GetCacheViewAuthenticIndexQueue(image_view);
            for (x=0; x < (ssize_t) image->columns; x++)
              SetPixelIndex(indexes+x,index);
          }
        if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      return(status);
    }
    case ResetAlphaChannel: /* deprecated */
    case OpaqueAlphaChannel:
    {
      status=SetImageOpacity(image,OpaqueOpacity);
      break;
    }
    case SetAlphaChannel:
    {
      if (image->matte == MagickFalse)
        status=SetImageOpacity(image,OpaqueOpacity);
      break;
    }
    case TransparentAlphaChannel:
    {
      status=SetImageOpacity(image,TransparentOpacity);
      break;
    }
    case UndefinedAlphaChannel:
      break;
  }
  if (status == MagickFalse)
    return(status);
  return(SyncImagePixelCache(image,&image->exception));
}

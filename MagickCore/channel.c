/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               CCCC  H   H   AAA   N   N  N   N  EEEEE   L                   %
%              C      H   H  A   A  NN  N  NN  N  E       L                   %
%              C      HHHHH  AAAAA  N N N  N N N  EEE     L                   %
%              C      H   H  A   A  N  NN  N  NN  E       L                   %
%               CCCC  H   H  A   A  N   N  N   N  EEEEE   LLLLL               %
%                                                                             %
%                                                                             %
%                      MagickCore Image Channel Methods                       %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               December 2003                                 %
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
#include "MagickCore/cache-private.h"
#include "MagickCore/channel.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/enhance.h"
#include "MagickCore/image.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/resource_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C h a n n e l F x I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ChannelFxImage() applies a channel expression to the specified image.  The
%  expression consists of one or more channels, either mnemonic or numeric (e.g.
%  r, red, 0), separated by actions as follows:
%
%    <=>     exchange two channels (e.g. red<=>blue)
%    =>      copy one channel to another channel (e.g. red=>green)
%    =       assign a constant value to a channel (e.g. red=50%)
%    ,       write new image channels in the specified order (e.g. red, green)
%    |       add a new output image for the next set of channel operations
%    ;       move to the next input image for the source of channel data
%
%  For example, to create 3 grayscale images from the red, green, and blue
%  channels of an image, use:
%
%    -channel-fx "red; green; blue"
%
%  A channel without an operation symbol implies separate (i.e, semicolon).
%
%  The format of the ChannelFxImage method is:
%
%      Image *ChannelFxImage(const Image *image,const char *expression,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o expression: A channel expression.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef enum
{
  ExtractChannelOp,
  AssignChannelOp,
  ExchangeChannelOp,
  TransferChannelOp
} ChannelFx;

static MagickBooleanType ChannelImage(Image *destination_image,
  const PixelChannel destination_channel,const ChannelFx channel_op,
  const Image *source_image,const PixelChannel source_channel,
  const Quantum pixel,ExceptionInfo *exception)
{
  CacheView
    *source_view,
    *destination_view;

  MagickBooleanType
    status = MagickTrue;

  size_t
    height,
    width;

  ssize_t
    y;

  /*
    Copy source channel to destination.
  */
  height=MagickMin(source_image->rows,destination_image->rows);
  width=MagickMin(source_image->columns,destination_image->columns);
  source_view=AcquireVirtualCacheView(source_image,exception);
  destination_view=AcquireAuthenticCacheView(destination_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    magick_number_threads(source_image,source_image,height,1)
#endif
  for (y=0; y < (ssize_t) height; y++)
  {
    PixelTrait
      destination_traits,
      source_traits;

    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(source_view,0,y,source_image->columns,1,
      exception);
    q=GetCacheViewAuthenticPixels(destination_view,0,y,
      destination_image->columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    destination_traits=GetPixelChannelTraits(destination_image,
      destination_channel);
    source_traits=GetPixelChannelTraits(source_image,source_channel);
    if ((destination_traits == UndefinedPixelTrait) ||
        (source_traits == UndefinedPixelTrait))
      continue;
    for (x=0; x < (ssize_t) width; x++)
    {
      if (channel_op == AssignChannelOp)
        SetPixelChannel(destination_image,destination_channel,pixel,q);
      else
        SetPixelChannel(destination_image,destination_channel,
          GetPixelChannel(source_image,source_channel,p),q);
      p+=GetPixelChannels(source_image);
      q+=GetPixelChannels(destination_image);
    }
    if (SyncCacheViewAuthenticPixels(destination_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  destination_view=DestroyCacheView(destination_view);
  source_view=DestroyCacheView(source_view);
  return(status);
}

MagickExport Image *ChannelFxImage(const Image *image,const char *expression,
  ExceptionInfo *exception)
{
#define ChannelFxImageTag  "ChannelFx/Image"

  ChannelFx
    channel_op = ExtractChannelOp;

  ChannelType
    channel_mask;

  char
    token[MagickPathExtent] = "";

  const char
    *p;

  const Image
    *source_image;

  double
    pixel = 0.0;

  Image
    *destination_image;

  MagickBooleanType
    status = MagickTrue;

  PixelChannel
    source_channel,
    destination_channel = RedPixelChannel;

  ssize_t
    channels = 0;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  p=expression;
  source_image=image;
  destination_image=CloneImage(image,0,0,MagickTrue,exception);
  if (destination_image == (Image *) NULL)
    return((Image *) NULL);
  if (expression == (const char *) NULL)
    return(destination_image);
  status=SetImageStorageClass(destination_image,DirectClass,exception);
  if (status == MagickFalse)
    {
      destination_image=GetLastImageInList(destination_image);
      return((Image *) NULL);
    }
  channel_mask=destination_image->channel_mask;
  (void) GetNextToken(p,&p,MagickPathExtent,token);
  while (*token != '\0')
  {
    PixelTrait
      traits;

    ssize_t
      i;

    /*
      Interpret channel expression.
    */
    switch (*token)
    {
      case ',':
      {
        (void) GetNextToken(p,&p,MagickPathExtent,token);
        break;
      }
      case '|':
      {
        source_image=GetFirstImageInList(source_image);
        if (GetNextImageInList(source_image) != (Image *) NULL)
          source_image=GetNextImageInList(source_image);
        (void) GetNextToken(p,&p,MagickPathExtent,token);
        break;
      }
      case ';':
      {
        Image
          *canvas;

        (void) SetPixelChannelMask(destination_image,channel_mask);
        if ((channel_op == ExtractChannelOp) && (channels == 1))
          {
            (void) SetPixelMetaChannels(destination_image,0,exception);
            (void) SetImageColorspace(destination_image,GRAYColorspace,
              exception);
          }
        canvas=CloneImage(source_image,0,0,MagickTrue,exception);
        if (canvas == (Image *) NULL)
          {
            destination_image=DestroyImageList(destination_image);
            return(destination_image);
          }
        AppendImageToList(&destination_image,canvas);
        destination_image=GetLastImageInList(destination_image);
        status=SetImageStorageClass(destination_image,DirectClass,exception);
        if (status == MagickFalse)
          {
            destination_image=GetLastImageInList(destination_image);
            return((Image *) NULL);
          }
        (void) GetNextToken(p,&p,MagickPathExtent,token);
        channels=0;
        destination_channel=RedPixelChannel;
        channel_mask=destination_image->channel_mask;
        break;
      }
      default:
        break;
    }
    i=ParsePixelChannelOption(token);
    source_channel=(PixelChannel) i;
    traits=GetPixelChannelTraits(source_image,source_channel);
    if (traits == UndefinedPixelTrait)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"MissingImageChannel","`%s'",token);
        destination_image=DestroyImageList(destination_image);
        return(destination_image);
      }
    channel_op=ExtractChannelOp;
    (void) GetNextToken(p,&p,MagickPathExtent,token);
    if (*token == '<')
      {
        channel_op=ExchangeChannelOp;
        (void) GetNextToken(p,&p,MagickPathExtent,token);
      }
    if (*token == '=')
      {
        if (channel_op != ExchangeChannelOp)
          channel_op=AssignChannelOp;
        (void) GetNextToken(p,&p,MagickPathExtent,token);
      }
    if (*token == '>')
      {
        if (channel_op != ExchangeChannelOp)
          channel_op=TransferChannelOp;
        (void) GetNextToken(p,&p,MagickPathExtent,token);
      }
    switch (channel_op)
    {
      case AssignChannelOp:
      case ExchangeChannelOp:
      case TransferChannelOp:
      {
        if (channel_op == AssignChannelOp)
          pixel=StringToDoubleInterval(token,(double) QuantumRange+1.0);
        else
          {
            i=ParsePixelChannelOption(token);
            if (LocaleCompare(token,"alpha") == 0)
              destination_image->alpha_trait=BlendPixelTrait;
            if (i < 0)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  OptionError,"UnrecognizedChannelType","`%s'",token);
                destination_image=DestroyImageList(destination_image);
                return(destination_image);
              }
          }
        destination_channel=(PixelChannel) i;
        if (image->colorspace != UndefinedColorspace)
          switch (destination_channel)
          {
            case RedPixelChannel:
            case GreenPixelChannel:
            case BluePixelChannel:
            case BlackPixelChannel:
            case AlphaPixelChannel:
            case IndexPixelChannel:
              break;
            case CompositeMaskPixelChannel:
            {
              destination_image->channels=(ChannelType)
                (destination_image->channels | CompositeMaskChannel);
              break;
            }
            case ReadMaskPixelChannel:
            {
              destination_image->channels=(ChannelType)
                (destination_image->channels | ReadMaskChannel);
              break;
            }
            case WriteMaskPixelChannel:
            {
              destination_image->channels=(ChannelType)
                (destination_image->channels | WriteMaskChannel);
              break;
            }
            case MetaPixelChannels:
            default:
            {
              traits=GetPixelChannelTraits(destination_image,
                destination_channel);
              if (traits != UndefinedPixelTrait)
                break;
              (void) SetPixelMetaChannels(destination_image,
                GetPixelMetaChannels(destination_image)+1,exception);
              traits=GetPixelChannelTraits(destination_image,
               destination_channel);
              if (traits == UndefinedPixelTrait)
                {
                  (void) ThrowMagickException(exception,GetMagickModule(),
                    CorruptImageError,"MissingImageChannel","`%s'",token);
                  destination_image=DestroyImageList(destination_image);
                  return(destination_image);
                }
              break;
            }
          }
        channel_mask=(ChannelType) (channel_mask |
          (MagickLLConstant(1) << ParseChannelOption(token)));
        (void) GetNextToken(p,&p,MagickPathExtent,token);
        break;
      }
      default:
        break;
    }
    status=ChannelImage(destination_image,destination_channel,channel_op,
      source_image,source_channel,ClampToQuantum(pixel),exception);
    if (status == MagickFalse)
      {
        destination_image=DestroyImageList(destination_image);
        break;
      }
    channels++;
    if (channel_op == ExchangeChannelOp)
      {
        status=ChannelImage(destination_image,source_channel,channel_op,
          source_image,destination_channel,ClampToQuantum(pixel),exception);
        if (status == MagickFalse)
          {
            destination_image=DestroyImageList(destination_image);
            break;
          }
        channels++;
      }
    switch (channel_op)
    {
      case ExtractChannelOp:
      {
        channel_mask=(ChannelType) (channel_mask |
          (MagickLLConstant(1) << destination_channel));
        destination_channel=(PixelChannel) (destination_channel+1);
        break;
      }
      default:
        break;
    }
    status=SetImageProgress(source_image,ChannelFxImageTag,p-expression,
      strlen(expression));
    if (status == MagickFalse)
      break;
  }
  if (destination_image == (Image *) NULL)
    return(destination_image);
  (void) SetPixelChannelMask(destination_image,channel_mask);
  if ((channel_op == ExtractChannelOp) && (channels == 1))
    {
      (void) SetPixelMetaChannels(destination_image,0,exception);
      (void) SetImageColorspace(destination_image,GRAYColorspace,exception);
    }
  return(GetFirstImageInList(destination_image));
}

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
%      Image *CombineImages(const Image *images,const ColorspaceType colorspace,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the image sequence.
%
%    o colorspace: the image colorspace.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *CombineImages(const Image *image,
  const ColorspaceType colorspace,ExceptionInfo *exception)
{
#define CombineImageTag  "Combine/Image"

  CacheView
    *combine_view;

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
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  combine_image=CloneImage(image,0,0,MagickTrue,exception);
  if (combine_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(combine_image,DirectClass,exception) == MagickFalse)
    {
      combine_image=DestroyImage(combine_image);
      return((Image *) NULL);
    }
  if (colorspace != UndefinedColorspace)
    (void) SetImageColorspace(combine_image,colorspace,exception);
  else
    if (fabs(image->gamma-1.0) <= MagickEpsilon)
      (void) SetImageColorspace(combine_image,RGBColorspace,exception);
    else
      (void) SetImageColorspace(combine_image,sRGBColorspace,exception);
  switch (combine_image->colorspace)
  {
    case UndefinedColorspace:
    case sRGBColorspace:
    {
      if (GetImageListLength(image) > 3)
        combine_image->alpha_trait=BlendPixelTrait;
      break;
    }
    case LinearGRAYColorspace:
    case GRAYColorspace:
    {
      if (GetImageListLength(image) > 1)
        combine_image->alpha_trait=BlendPixelTrait;
      break;
    }
    case CMYKColorspace:
    {
      if (GetImageListLength(image) > 4)
        combine_image->alpha_trait=BlendPixelTrait;
      break;
    }
    default:
      break;
  }
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

    Quantum
      *pixels;

    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      i;

    if (status == MagickFalse)
      continue;
    pixels=GetCacheViewAuthenticPixels(combine_view,0,y,combine_image->columns,
      1,exception);
    if (pixels == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    next=image;
    for (i=0; i < (ssize_t) GetPixelChannels(combine_image); i++)
    {
      ssize_t
        x;

      PixelChannel channel = GetPixelChannelChannel(combine_image,i);
      PixelTrait traits = GetPixelChannelTraits(combine_image,channel);
      if (traits == UndefinedPixelTrait)
        continue;
      if (next == (Image *) NULL)
        continue;
      image_view=AcquireVirtualCacheView(next,exception);
      p=GetCacheViewVirtualPixels(image_view,0,y,next->columns,1,exception);
      if (p == (const Quantum *) NULL)
        continue;
      q=pixels;
      for (x=0; x < (ssize_t) combine_image->columns; x++)
      {
        if (x < (ssize_t) next->columns)
          {
            q[i]=GetPixelIntensity(next,p);
            p+=GetPixelChannels(next);
          }
        q+=GetPixelChannels(combine_image);
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

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,CombineImageTag,progress,
          combine_image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  combine_view=DestroyCacheView(combine_view);
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
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickCoreSignature);
  return(image->alpha_trait != UndefinedPixelTrait ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S e p a r a t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SeparateImage() separates a channel from the image and returns it as a
%  grayscale image.
%
%  The format of the SeparateImage method is:
%
%      Image *SeparateImage(const Image *image,const ChannelType channel,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the image channel.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SeparateImage(const Image *image,
  const ChannelType channel_type,ExceptionInfo *exception)
{
#define GetChannelBit(mask,bit)  (((size_t) (mask) >> (size_t) (bit)) & 0x01)
#define SeparateImageTag  "Separate/Image"

  CacheView
    *image_view,
    *separate_view;

  Image
    *separate_image;

  MagickBooleanType
    status;

  MagickOffsetType
    progress;

  ssize_t
    y;

  /*
    Initialize separate image attributes.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  separate_image=CloneImage(image,0,0,MagickTrue,exception);
  if (separate_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(separate_image,DirectClass,exception) == MagickFalse)
    {
      separate_image=DestroyImage(separate_image);
      return((Image *) NULL);
    }
  separate_image->alpha_trait=UndefinedPixelTrait;
  (void) SetImageColorspace(separate_image,GRAYColorspace,exception);
  separate_image->gamma=image->gamma;
  /*
    Separate image.
  */
  status=MagickTrue;
  progress=0;
  image_view=AcquireVirtualCacheView(image,exception);
  separate_view=AcquireAuthenticCacheView(separate_image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(progress,status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *magick_restrict p;

    Quantum
      *magick_restrict q;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    q=QueueCacheViewAuthenticPixels(separate_view,0,y,separate_image->columns,1,
      exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        i;

      SetPixelChannel(separate_image,GrayPixelChannel,(Quantum) 0,q);
      for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
      {
        PixelChannel channel = GetPixelChannelChannel(image,i);
        PixelTrait traits = GetPixelChannelTraits(image,channel);
        if ((traits == UndefinedPixelTrait) ||
            (GetChannelBit(channel_type,channel) == 0))
          continue;
        SetPixelChannel(separate_image,GrayPixelChannel,p[i],q);
      }
      p+=GetPixelChannels(image);
      q+=GetPixelChannels(separate_image);
    }
    if (SyncCacheViewAuthenticPixels(separate_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        progress++;
        proceed=SetImageProgress(image,SeparateImageTag,progress,image->rows);
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  separate_view=DestroyCacheView(separate_view);
  image_view=DestroyCacheView(image_view);
  (void) SetImageChannelMask(separate_image,AllChannels);
  if (status == MagickFalse)
    separate_image=DestroyImage(separate_image);
  return(separate_image);
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
%      Image *SeparateImages(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport Image *SeparateImages(const Image *image,ExceptionInfo *exception)
{
  Image
    *images,
    *separate_image;

  ssize_t
    i;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  images=NewImageList();
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if ((traits == UndefinedPixelTrait) || ((traits & UpdatePixelTrait) == 0))
      continue;
    separate_image=SeparateImage(image,(ChannelType)
      (MagickLLConstant(1) << channel),exception);
    if (separate_image != (Image *) NULL)
      AppendImageToList(&images,separate_image);
  }
  if (images == (Image *) NULL)
    images=SeparateImage(image,UndefinedChannel,exception);
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
%        const AlphaChannelOption alpha_type,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o alpha_type:  The alpha channel type: ActivateAlphaChannel,
%      AssociateAlphaChannel, CopyAlphaChannel, DeactivateAlphaChannel,
%      DisassociateAlphaChannel,  ExtractAlphaChannel, OffAlphaChannel,
%      OnAlphaChannel, OpaqueAlphaChannel, SetAlphaChannel, ShapeAlphaChannel,
%      and TransparentAlphaChannel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline void FlattenPixelInfo(const Image *image,const PixelInfo *p,
  const double alpha,const Quantum *q,const double beta,Quantum *composite)
{
  double
    Da,
    gamma,
    Sa;

  ssize_t
    i;

  /*
    Compose pixel p over pixel q with the given alpha.
  */
  Sa=QuantumScale*alpha;
  Da=QuantumScale*beta,
  gamma=Sa*(-Da)+Sa+Da;
  gamma=PerceptibleReciprocal(gamma);
  for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
  {
    PixelChannel channel = GetPixelChannelChannel(image,i);
    PixelTrait traits = GetPixelChannelTraits(image,channel);
    if (traits == UndefinedPixelTrait)
      continue;
    switch (channel)
    {
      case RedPixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((double) q[i],beta,
          (double) p->red,alpha));
        break;
      }
      case GreenPixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((double) q[i],beta,
          (double) p->green,alpha));
        break;
      }
      case BluePixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((double) q[i],beta,
          (double) p->blue,alpha));
        break;
      }
      case BlackPixelChannel:
      {
        composite[i]=ClampToQuantum(gamma*MagickOver_((double) q[i],beta,
          (double) p->black,alpha));
        break;
      }
      case AlphaPixelChannel:
      {
        composite[i]=ClampToQuantum((double) QuantumRange*(Sa*(-Da)+Sa+Da));
        break;
      }
      default:
        break;
    }
  }
}

MagickExport MagickBooleanType SetImageAlphaChannel(Image *image,
  const AlphaChannelOption alpha_type,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickCoreSignature);
  status=MagickTrue;
  switch (alpha_type)
  {
    case ActivateAlphaChannel:
    {
      if ((image->alpha_trait & BlendPixelTrait) != 0)
        return(status);
      image->alpha_trait=BlendPixelTrait;
      break;
    }
    case AssociateAlphaChannel:
    {
      /*
        Associate alpha.
      */
      status=SetImageStorageClass(image,DirectClass,exception);
      if (status == MagickFalse)
        break;
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        Quantum
          *magick_restrict q;

        ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          double
            gamma;

          ssize_t
            i;

          gamma=QuantumScale*(double) GetPixelAlpha(image,q);
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if (channel == AlphaPixelChannel)
              continue;
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            q[i]=ClampToQuantum(gamma*(double) q[i]);
          }
          q+=GetPixelChannels(image);
        }
        if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      image->alpha_trait=CopyPixelTrait;
      return(status);
    }
    case BackgroundAlphaChannel:
    {
      /*
        Set transparent pixels to background color.
      */
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        break;
      status=SetImageStorageClass(image,DirectClass,exception);
      if (status == MagickFalse)
        break;
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        Quantum
          *magick_restrict q;

        ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          if (GetPixelAlpha(image,q) == TransparentAlpha)
            {
              SetPixelViaPixelInfo(image,&image->background_color,q);
              SetPixelChannel(image,AlphaPixelChannel,TransparentAlpha,q);
            }
          q+=GetPixelChannels(image);
        }
        if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      return(status);
    }
    case CopyAlphaChannel:
    {
      image->alpha_trait=UpdatePixelTrait;
      status=CompositeImage(image,image,IntensityCompositeOp,MagickTrue,0,0,
        exception);
      break;
    }
    case DeactivateAlphaChannel:
    {
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        status=SetImageAlpha(image,OpaqueAlpha,exception);
      image->alpha_trait=CopyPixelTrait;
      break;
    }
    case DisassociateAlphaChannel:
    {
      /*
        Disassociate alpha.
      */
      status=SetImageStorageClass(image,DirectClass,exception);
      if (status == MagickFalse)
        break;
      image->alpha_trait=BlendPixelTrait;
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        Quantum
          *magick_restrict q;

        ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          double
            gamma,
            Sa;

          ssize_t
            i;

          Sa=QuantumScale*(double) GetPixelAlpha(image,q);
          gamma=PerceptibleReciprocal(Sa);
          for (i=0; i < (ssize_t) GetPixelChannels(image); i++)
          {
            PixelChannel channel = GetPixelChannelChannel(image,i);
            PixelTrait traits = GetPixelChannelTraits(image,channel);
            if (channel == AlphaPixelChannel)
              continue;
            if ((traits & UpdatePixelTrait) == 0)
              continue;
            q[i]=ClampToQuantum(gamma*(double) q[i]);
          }
          q+=GetPixelChannels(image);
        }
        if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      image->alpha_trait=UndefinedPixelTrait;
      return(status);
    }
    case DiscreteAlphaChannel:
    {
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        status=SetImageAlpha(image,OpaqueAlpha,exception);
      image->alpha_trait=UpdatePixelTrait;
      break;
    }
    case ExtractAlphaChannel:
    {
      status=CompositeImage(image,image,AlphaCompositeOp,MagickTrue,0,0,
        exception);
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case OffAlphaChannel:
    {
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        return(status);
      image->alpha_trait=UndefinedPixelTrait;
      break;
    }
    case OnAlphaChannel:
    {
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        status=SetImageAlpha(image,OpaqueAlpha,exception);
      image->alpha_trait=BlendPixelTrait;
      break;
    }
    case OpaqueAlphaChannel:
    {
      status=SetImageAlpha(image,OpaqueAlpha,exception);
      break;
    }
    case RemoveAlphaChannel:
    {
      /*
        Remove transparency.
      */
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        break;
      status=SetImageStorageClass(image,DirectClass,exception);
      if (status == MagickFalse)
        break;
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        Quantum
          *magick_restrict q;

        ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          FlattenPixelInfo(image,&image->background_color,
            image->background_color.alpha,q,(double) GetPixelAlpha(image,q),q);
          q+=GetPixelChannels(image);
        }
        if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      image->alpha_trait=image->background_color.alpha_trait;
      break;
    }
    case SetAlphaChannel:
    {
      if ((image->alpha_trait & BlendPixelTrait) == 0)
        status=SetImageAlpha(image,OpaqueAlpha,exception);
      break;
    }
    case ShapeAlphaChannel:
    {
      PixelInfo
        background;

      /*
        Remove transparency.
      */
      ConformPixelInfo(image,&image->background_color,&background,exception);
      background.alpha_trait=BlendPixelTrait;
      image->alpha_trait=BlendPixelTrait;
      status=SetImageStorageClass(image,DirectClass,exception);
      if (status == MagickFalse)
        break;
      image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        PixelInfo
          pixel;

        Quantum
          *magick_restrict q;

        ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        pixel=background;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          pixel.alpha=GetPixelIntensity(image,q);
          SetPixelViaPixelInfo(image,&pixel,q);
          q+=GetPixelChannels(image);
        }
        if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
      break;
    }
    case TransparentAlphaChannel:
    {
      status=SetImageAlpha(image,TransparentAlpha,exception);
      break;
    }
    case UndefinedAlphaChannel:
      break;
  }
  if (status == MagickFalse)
    return(status);
  (void) SetPixelChannelMask(image,image->channel_mask);
  return(SyncImagePixelCache(image,exception));
}

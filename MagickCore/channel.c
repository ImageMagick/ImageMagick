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
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/studio.h"
#include "MagickCore/image.h"
#include "MagickCore/list.h"
#include "MagickCore/log.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C h a n n e l O p e r a t i o n I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ChannelOperationImage() applies a channel expression to the specified image.
%  The expression consists of one or more channels, either mnemonic or numeric
%  (e.g. red, 1), separated by certain operation symbols as follows:
%
%    <=>     exchange two channels (e.g. red<=>blue)
%    =>      transfer a channel to another (e.g. red=>green)
%    ,       separate channel operations (e.g. red, green)
%    |       read channels from next input image (e.g. red | green)
%    ;       write channels to next output image (e.g. red; green; blue)
%
%  A channel without a operation symbol implies extract. For example, to create
%  3 grayscale images from the red, green, and blue channels of an image, use:
%
%    -channel-ops "red; green; blue"
%
%  The format of the ChannelOperationImage method is:
%
%      Image *ChannelOperationImage(const Image *image,
%        const char *expression,ExceptionInfo *exception)
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
  ExchangeChannelOp,
  TransferChannelOp
} ChannelOperation;

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static MagickBooleanType ChannelImage(Image *destination_image,
  const Image *source_image,const ChannelOperation channel_op,
  const PixelChannel source_channel,const PixelChannel destination_channel,
  ExceptionInfo *exception)
{
  CacheView
    *source_view,
    *destination_view;

  MagickBooleanType
    status;

  size_t
    height;

  ssize_t
    y;

  status=MagickTrue;
  source_view=AcquireCacheView(source_image);
  destination_view=AcquireCacheView(destination_image);
  height=MagickMin(source_image->rows,destination_image->rows);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status)
#endif
  for (y=0; y < (ssize_t) height; y++)
  {
    register const Quantum
      *restrict p;

    register Quantum
      *restrict q;

    register ssize_t
      x;

    size_t
      width;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(source_view,0,y,source_image->columns,1,
      exception);
    q=QueueCacheViewAuthenticPixels(destination_view,0,y,
      destination_image->columns,1,exception);
    if ((p == (const Quantum *) NULL) || (q == (Quantum *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    width=MagickMin(source_image->columns,destination_image->columns);
    for (x=0; x < (ssize_t) width; x++)
    {
      PixelTrait
        destination_traits,
        source_traits;

      ssize_t
        offset;

      source_traits=GetPixelChannelMapTraits(source_image,source_channel);
      destination_traits=GetPixelChannelMapTraits(destination_image,
        destination_channel);
      if ((source_traits == UndefinedPixelTrait) ||
          (destination_traits == UndefinedPixelTrait))
        continue;
      offset=GetPixelChannelMapOffset(source_image,source_channel);
      SetPixelChannel(destination_image,destination_channel,p[offset],q);
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

MagickExport Image *ChannelOperationImage(const Image *image,
  const char *expression,ExceptionInfo *exception)
{
#define ChannelOperationImageTag  "ChannelOperation/Image"

  char
    token[MaxTextExtent];

  ChannelOperation
    channel_op;

  const char
    *p;

  const Image
    *source_image;

  Image
    *destination_image;

  PixelChannel
    source_channel,
    destination_channel;

  ssize_t
    channels;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  source_image=image;
  destination_image=CloneImage(source_image,0,0,MagickTrue,exception);
  if (destination_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageBackgroundColor(destination_image,exception) == MagickFalse)
    {
      destination_image=GetLastImageInList(destination_image);
      return((Image *) NULL);
    }
  if (expression == (const char *) NULL)
    return(destination_image);
  destination_channel=RedPixelChannel;
  p=(char *) expression;
  GetMagickToken(p,&p,token);
  for (channels=0; *p != '\0'; )
  {
    MagickBooleanType
      status;

    ssize_t
      i;

    /*
      Interpret channel expression.
    */
    if (*token == ',')
      {
        destination_channel=(PixelChannel) ((ssize_t) destination_channel+1);
        GetMagickToken(p,&p,token);
      }
    if (*token == '|')
      {
        if (GetNextImageInList(source_image) != (Image *) NULL)
          source_image=GetNextImageInList(source_image);
        else
          source_image=GetFirstImageInList(source_image);
        GetMagickToken(p,&p,token);
      }
    if (*token == ';')
      {
        Image
          *canvas;

        if (channels == 1)
          destination_image->colorspace=GRAYColorspace;
        canvas=CloneImage(source_image,0,0,MagickTrue,exception);
        if (canvas == (Image *) NULL)
          {
            destination_image=GetLastImageInList(destination_image);
            return((Image *) NULL);
          }
        AppendImageToList(&destination_image,canvas);
        destination_image=GetLastImageInList(destination_image);
        if (SetImageBackgroundColor(destination_image,exception) == MagickFalse)
          {
            destination_image=GetLastImageInList(destination_image);
            return((Image *) NULL);
          }
        GetMagickToken(p,&p,token);
        channels=0;
        destination_channel=RedPixelChannel;
      }
    i=ParsePixelChannelOption(token);
    if (i < 0)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "UnableToParseExpression","`%s'",p);
        destination_image=DestroyImageList(destination_image);
        break;
      }
    source_channel=(PixelChannel) i;
    channel_op=ExtractChannelOp;
    GetMagickToken(p,&p,token);
    if (*token == '<')
      {
        channel_op=ExchangeChannelOp;
        GetMagickToken(p,&p,token);
      }
    if (*token == '=')
      GetMagickToken(p,&p,token);
    if (*token == '>')
      {
        if (channel_op != ExchangeChannelOp)
          channel_op=TransferChannelOp;
        GetMagickToken(p,&p,token);
      }
    if (channel_op != ExtractChannelOp)
      {
        i=ParsePixelChannelOption(token);
        if (i < 0)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
              "UnableToParseExpression","`%s'",p);
            destination_image=DestroyImageList(destination_image);
            break;
          }
        destination_channel=(PixelChannel) i;
      }
    status=ChannelImage(destination_image,source_image,channel_op,
      source_channel,destination_channel,exception);
    if (status == MagickFalse)
      {
        destination_image=DestroyImageList(destination_image);
        break;
      }
    channels++;
    status=SetImageProgress(source_image,ChannelOperationImageTag,p-expression,
      strlen(expression));
    if (status == MagickFalse)
      break;
  }
  if (channels == 1)
    destination_image->colorspace=GRAYColorspace;
  return(destination_image);
}

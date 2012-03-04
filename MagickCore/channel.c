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
#include "MagickCore/option.h"
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
%
%  The format of the ChannelOperationImage method is:
%
%      Image *ChannelOperationImage(const Image *images,
%        const char *expression,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o images: the images.
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

static MagickBooleanType ChannelImage(Image *channel_image,const Image *image,
  const ChannelOperation channel_op,const PixelChannel p_channel,
  const PixelChannel q_channel,ExceptionInfo *exception)
{
  return(MagickTrue);
}

MagickExport Image *ChannelOperationImage(const Image *images,
  const char *expression,ExceptionInfo *exception)
{
  char
    token[MaxTextExtent];

  ChannelOperation
    channel_op;

  const char
    *p;

  Image
    *channel_images;

  PixelChannel
    p_channel,
    q_channel;

  assert(images != (Image *) NULL);
  assert(images->signature == MagickSignature);
  if (images->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",images->filename);
  channel_images=CloneImage(images,images->columns,images->columns,MagickTrue,
     exception);
  p=(char *) expression;
  GetMagickToken(p,&p,token);
  for (q_channel=RedPixelChannel; *p != '\0'; )
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
        q_channel=(PixelChannel) ((ssize_t) q_channel+1);
        GetMagickToken(p,&p,token);
      }
    if (*token == '|')
      {
        if (GetNextImageInList(images) != (Image *) NULL)
          images=GetNextImageInList(images);
        else
          images=GetFirstImageInList(images);
        GetMagickToken(p,&p,token);
      }
    if (*token == ';')
      {
        AppendImageToList(&channel_images,CloneImage(images,
          channel_images->columns,channel_images->rows,MagickTrue,exception));
        channel_images=GetLastImageInList(channel_images);
        GetMagickToken(p,&p,token);
      }
    i=ParsePixelChannelOption(token);
    if (i < 0)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
          "UnableToParseExpression","`%s'",p);
        channel_images=DestroyImageList(channel_images);
        break;
      }
    p_channel=(PixelChannel) i;
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
            channel_images=DestroyImageList(channel_images);
            break;
          }
        q_channel=(PixelChannel) i;
      }
    status=ChannelImage(channel_images,images,channel_op,p_channel,q_channel,
      exception);
    if (status == MagickFalse)
      {
        channel_images=DestroyImageList(channel_images);
        break;
      }
  }
  return(channel_images);
}

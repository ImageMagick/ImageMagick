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
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/histogram.h"
#include "magick/image.h"
#include "magick/list.h"
#include "magick/memory_.h"
#include "magick/prepress.h"
#include "magick/registry.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/statistic.h"
#include "magick/string_.h"

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
%  If the adjustment values are both zero this function is equivelent to a
%  perfect normalization (or autolevel) of the image.
%
%  Each channel is stretched independantally of each other (producing color
%  distortion) unless the special 'SyncChannels' flag is also provided in the
%  channels setting. If this flag is present the minimum and maximum point
%  will be extracted from all the given channels, and those channels will be
%  stretched by exactly the same amount (preventing color distortion).
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
     const ChannelType channel, const double black_value,
     const double white_value)
{
  double
    min,max;

  MagickStatusType
    status;

  if ((channel & SyncChannels) != 0 )
    {
      /*
        autolevel all channels equally
      */
      GetImageChannelRange(image, channel, &min, &max, &image->exception);
      min += black_value;  max -= white_value;
      return LevelImageChannel(image, channel, min, max, 1.0);
    }

  /*
    autolevel each channel separateally
  */
  status = MagickTrue;
  if ((channel & RedChannel) != 0)
    {
      GetImageChannelRange(image, RedChannel, &min, &max, &image->exception);
      min += black_value;  max -= white_value;
      status = status && LevelImageChannel(image, RedChannel, min, max, 1.0);
    }
  if ((channel & GreenChannel) != 0)
    {
      GetImageChannelRange(image, GreenChannel, &min, &max, &image->exception);
      min += black_value;  max -= white_value;
      status = status && LevelImageChannel(image, GreenChannel, min, max, 1.0);
    }
  if ((channel & BlueChannel) != 0)
    {
      GetImageChannelRange(image, BlueChannel, &min, &max, &image->exception);
      min += black_value;  max -= white_value;
      status = status && LevelImageChannel(image, BlueChannel, min, max, 1.0);
    }
  if (((channel & OpacityChannel) != 0) &&
      (image->matte == MagickTrue))
    {
      GetImageChannelRange(image, OpacityChannel, &min, &max, &image->exception);
      min += black_value;  max -= white_value;
      status = status && LevelImageChannel(image, OpacityChannel, min, max, 1.0);
    }
  if (((channel & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace))
    {
      GetImageChannelRange(image, IndexChannel, &min, &max, &image->exception);
      min += black_value;  max -= white_value;
      status = status && LevelImageChannel(image, IndexChannel, min, max, 1.0);
    }
  return(status != 0 ? MagickTrue : MagickFalse);
}


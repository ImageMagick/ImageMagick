/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         W   W   AAA   N   N  DDDD                           %
%                         W   W  A   A  NN  N  D   D                          %
%                         W W W  AAAAA  N N N  D   D                          %
%                         WW WW  A   A  N  NN  D   D                          %
%                         W   W  A   A  N   N  DDDD                           %
%                                                                             %
%                                                                             %
%                         MagickWand Support Methods                          %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 May  2004                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wand/studio.h"
#include "wand/MagickWand.h"
#include "wand/magick-wand-private.h"
#include "wand/wand.h"

static SplayTreeInfo
  *wand_ids = (SplayTreeInfo *) NULL;

static MagickBooleanType
  instantiate_wand = MagickFalse;

static SemaphoreInfo
  *wand_semaphore = (SemaphoreInfo *) NULL;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e W a n d I d                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireWandId() returns a unique wand id.
%
%  The format of the AcquireWandId() method is:
%
%      size_t AcquireWandId()
%
*/
WandExport size_t AcquireWandId(void)
{
  static size_t
    id = 0;

  if (wand_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&wand_semaphore);
  LockSemaphoreInfo(wand_semaphore);
  if ((wand_ids == (SplayTreeInfo *) NULL) && (instantiate_wand == MagickFalse))
    {
      wand_ids=NewSplayTree((int (*)(const void *,const void *)) NULL,
        (void *(*)(void *)) NULL,(void *(*)(void *)) NULL);
      instantiate_wand=MagickTrue;
    }
  id++;
  (void) AddValueToSplayTree(wand_ids,(const void *) id,(const void *) id);
  UnlockSemaphoreInfo(wand_semaphore);
  return(id);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    D e s t r o y W a n d I d s                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyWandIds() deallocates memory associated with the wand id's.
%
%  The format of the DestroyWandIds() method is:
%
%      void DestroyWandIds(void)
%
%  A description of each parameter follows:
%
*/
WandExport void DestroyWandIds(void)
{
  if (wand_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&wand_semaphore);
  LockSemaphoreInfo(wand_semaphore);
  if (wand_ids != (SplayTreeInfo *) NULL)
    wand_ids=DestroySplayTree(wand_ids);
  instantiate_wand=MagickFalse;
  UnlockSemaphoreInfo(wand_semaphore);
  DestroySemaphoreInfo(&wand_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    R e l i n q u i s h W a n d I d                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishWandId() relinquishes a unique wand id.
%
%  The format of the RelinquishWandId() method is:
%
%      void RelinquishWandId(const size_t *id)
%
%  A description of each parameter follows:
%
%    o id: a unique wand id.
%
*/
WandExport void RelinquishWandId(const size_t id)
{
  LockSemaphoreInfo(wand_semaphore);
  if (wand_ids != (SplayTreeInfo *) NULL)
    (void) DeleteNodeByValueFromSplayTree(wand_ids,(const void *) id);
  UnlockSemaphoreInfo(wand_semaphore);
}

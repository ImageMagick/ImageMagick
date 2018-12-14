/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               M   M   OOO   N   N  IIIII  TTTTT   OOO   RRRR                %
%               MM MM  O   O  NN  N    I      T    O   O  R   R               %
%               M M M  O   O  N N N    I      T    O   O  RRRR                %
%               M   M  O   O  N  NN    I      T    O   O  R R                 %
%               M   M   OOO   N   N  IIIII    T     OOO   R  R                %
%                                                                             %
%                                                                             %
%                     MagickCore Progress Monitor Methods                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               December 1995                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/image.h"
#include "MagickCore/log.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"

/*
  Static declarations.
*/
static SemaphoreInfo
  *monitor_semaphore = (SemaphoreInfo *) NULL;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M o n i t o r C o m p o n e n t G e n e s i s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MonitorComponentGenesis() instantiates the monitor component.
%
%  The format of the MonitorComponentGenesis method is:
%
%      MagickBooleanMonitor MonitorComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType MonitorComponentGenesis(void)
{
  if (monitor_semaphore == (SemaphoreInfo *) NULL)
    monitor_semaphore=AcquireSemaphoreInfo();
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M o n i t o r C o m p o n e n t T e r m i n u s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MonitorComponentTerminus() destroy monitor component.
%
%  The format of the MonitorComponentTerminus method is:
%
%      void MonitorComponentTerminus(void)
%
*/
MagickPrivate void MonitorComponentTerminus(void)
{
  if (monitor_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&monitor_semaphore);
  LockSemaphoreInfo(monitor_semaphore);
  UnlockSemaphoreInfo(monitor_semaphore);
  RelinquishSemaphoreInfo(&monitor_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e P r o g r e s s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageProgress() returns the progress of an image processing operation.
%
%  The format of the SetImageProgress method is:
%
%    MagickBooleanType SetImageProgress(const char *text,
%      const MagickOffsetType offset,const MagickSizeType extent)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o text: description of the image processing operation.
%
%    o offset: the offset relative to the extent parameter.
%
%    o extent: the extent of the progress.
%
*/
MagickExport MagickBooleanType SetImageProgress(const Image *image,
  const char *tag,const MagickOffsetType offset,const MagickSizeType extent)
{
  char
    message[MagickPathExtent];

  MagickBooleanType
    status;

  if (image->progress_monitor == (MagickProgressMonitor) NULL)
    return(MagickTrue);
  (void) FormatLocaleString(message,MagickPathExtent,"%s/%s",tag,
    image->filename);
  if (monitor_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&monitor_semaphore);
  LockSemaphoreInfo(monitor_semaphore);
  status=image->progress_monitor(message,offset,extent,image->client_data);
  UnlockSemaphoreInfo(monitor_semaphore);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e P r o g r e s s M o n i t o r                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageProgressMonitor() sets the image progress monitor to the specified
%  method and returns the previous progress monitor if any.  The progress
%  monitor method looks like this:
%
%    MagickBooleanType MagickProgressMonitor(const char *text,
%      const MagickOffsetType offset,const MagickSizeType extent,
%      void *client_data)
%
%  If the progress monitor returns MagickFalse, the current operation is
%  interrupted.
%
%  The format of the SetImageProgressMonitor method is:
%
%      MagickProgressMonitor SetImageProgressMonitor(Image *image,
%        const MagickProgressMonitor progress_monitor,void *client_data)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o progress_monitor: Specifies a pointer to a method to monitor progress of
%      an image operation.
%
%    o client_data: Specifies a pointer to any client data.
%
*/
MagickExport MagickProgressMonitor SetImageProgressMonitor(Image *image,
  const MagickProgressMonitor progress_monitor,void *client_data)
{
  MagickProgressMonitor
    previous_monitor;

  previous_monitor=image->progress_monitor;
  image->progress_monitor=progress_monitor;
  image->client_data=client_data;
  return(previous_monitor);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e I n f o P r o g r e s s M o n i t o r                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageInfoProgressMonitor() sets the image_info progress monitor to the
%  specified method and returns the previous progress monitor if any.  The
%  progress monitor method looks like this:
%
%    MagickBooleanType MagickProgressMonitor(const char *text,
%      const MagickOffsetType offset,const MagickSizeType extent,
%      void *client_data)
%
%  If the progress monitor returns MagickFalse, the current operation is
%  interrupted.
%
%  The format of the SetImageInfoProgressMonitor method is:
%
%      MagickProgressMonitor SetImageInfoProgressMonitor(ImageInfo *image_info,
%        const MagickProgressMonitor progress_monitor,void *client_data)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o progress_monitor: Specifies a pointer to a method to monitor progress of
%      an image operation.
%
%    o client_data: Specifies a pointer to any client data.
%
*/
MagickExport MagickProgressMonitor SetImageInfoProgressMonitor(
  ImageInfo *image_info,const MagickProgressMonitor progress_monitor,
  void *client_data)
{
  MagickProgressMonitor
    previous_monitor;

  previous_monitor=image_info->progress_monitor;
  image_info->progress_monitor=progress_monitor;
  image_info->client_data=client_data;
  return(previous_monitor);
}

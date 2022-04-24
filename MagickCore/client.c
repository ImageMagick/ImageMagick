/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  CCCC  L      IIIII  EEEEE  N   N  TTTTT                    %
%                 C      L        I    E      NN  N    T                      %
%                 C      L        I    EEE    N N N    T                      %
%                 C      L        I    E      N  NN    T                      %
%                  CCCC  LLLLL  IIIII  EEEEE  N   N    T                      %
%                                                                             %
%                                                                             %
%                         MagickCore Client Methods                           %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                               March 2003                                    %
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
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/client.h"
#include "MagickCore/log.h"
#include "MagickCore/string_.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C l i e n t N a m e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetClientName returns the current client name.
%
%  The format of the GetClientName method is:
%
%      const char *GetClientName(void)
%
*/
MagickExport const char *GetClientName(void)
{
  return(SetClientName((const char *) NULL));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C l i e n t P a t h                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetClientPath returns the current client name.
%
%  The format of the GetClientPath method is:
%
%      const char *GetClientPath(void)
%
*/
MagickExport const char *GetClientPath(void)
{
  return(SetClientPath((const char *) NULL));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C l i e n t N a m e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetClientName sets the client name and returns it.
%
%  The format of the SetClientName method is:
%
%      const char *SetClientName(const char *name)
%
%  A description of each parameter follows:
%
%    o name: Specifies the new client name.
%
*/
MagickExport const char *SetClientName(const char *name)
{
  static char
    client_name[256] = "";

  if ((name != (char *) NULL) && (*name != '\0'))
    {
      (void) CopyMagickString(client_name,name,sizeof(client_name));
      (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),"%s",client_name);
    }
  return(*client_name == '\0' ? "Magick" : client_name);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C l i e n t P a t h                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetClientPath() sets the client path if the name is specified.  Otherwise
%  the current client path is returned. A zero-length string is returned if
%  the client path has never been set.
%
%  The format of the SetClientPath method is:
%
%      const char *SetClientPath(const char *path)
%
%  A description of each parameter follows:
%
%    o path: Specifies the new client path.
%
*/
MagickExport const char *SetClientPath(const char *path)
{
  static char
    client_path[MagickPathExtent] = "";

  if ((path != (char *) NULL) && (*path != '\0'))
    {
      (void) CopyMagickString(client_path,path,MagickPathExtent);
      (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),"%s",path);
    }
  return(client_path);
}

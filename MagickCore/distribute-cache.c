/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%    DDDD    IIIII   SSSSS  TTTTT  RRRR   IIIII  BBBB   U   U  TTTTT  EEEEE   %
%    D   D     I     SS       T    R   R    I    B   B  U   U    T    E       %
%    D   D     I      SSS     T    RRRR     I    BBBB   U   U    T    EEE     %
%    D   D     I        SS    T    R R      I    B   B  U   U    T    E       %
%    DDDDA   IIIII   SSSSS    T    R  R   IIIII  BBBB    UUU     T    EEEEE   %
%                                                                             %
%                      CCCC   AAA    CCCC  H   H  EEEEE                       %
%                     C      A   A  C      H   H  E                           %
%                     C      AAAAA  C      HHHHH  EEE                         %
%                     C      A   A  C      H   H  E                           %
%                      CCCC  A   A   CCCC  H   H  EEEEE                       %
%                                                                             %
%                                                                             %
%                 MagickCore Distributed Pixel Cache Methods                  %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                January 2013                                 %
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
% A distributed pixel cache is an extension of the traditional pixel cache
% available on a single host.  The distributed pixel cache may span multiple
% servers so that it can grow in size and transactional capacity to support
% very large images. Start up the pixel cache server on one or more machines.
% When you read or operate on an image and the local pixel cache resources are
% exhausted, ImageMagick contacts one or more of these remote pixel servers to
% store or retrieve pixels.
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/memory_.h"
#if defined(MAGICKCORE_HAVE_SOCKET)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   P i x e l C a c h e S e r v e r                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PixelCacheServer() waits on the specified port for commands to create, read,
%  update, or destroy a pixel cache.
%
%  The format of the PixelCacheServer() method is:
%
%      void PixelCacheServer(const size_t port)
%
%  A description of each parameter follows:
%
%    o port: connect the distributed pixel cache at this port.
%
*/
MagickExport void PixelCacheServer(const size_t port)
{
#if defined(MAGICKCORE_HAVE_SOCKET)
  char
    buffer[MaxTextExtent];

  int
    cache_socket,
    cache_client,
    status;

  socklen_t
    length,
    one;

  ssize_t
    count;

  struct sockaddr_in
    address;

  cache_socket=socket(AF_INET,SOCK_STREAM,0);
  if (cache_socket == -1)
    {
      perror("Distributed pixel cache: server socket");
      exit(1);
    }
  one=1;
  status=setsockopt(cache_socket,SOL_SOCKET,SO_REUSEADDR,&one,(socklen_t)
    sizeof(one));
  if (status == -1)
    {
      perror("Distributed pixel cache: server setsockopt");
      exit(1);
    }
  (void) ResetMagickMemory(&address,0,sizeof(address));
  address.sin_family=AF_INET;
  address.sin_port=htons(port);
  address.sin_addr.s_addr=INADDR_ANY;
  status=bind(cache_socket,(struct sockaddr *) &address,(socklen_t)
    sizeof(address));
  if (status == -1)
    {
      perror("Distributed pixel cache: server bind");
      exit(1);
    }
  status=listen(cache_socket,5);
  if (status == -1)
    {
      perror("Distributed pixel cache: server listen");
      exit(1);
    }
  (void) fprintf(stdout,
    "Distributed pixel cache server:  waiting for client on port %d\n",(int)
    port);
  (void) fflush(stdout);
  for ( ; ; )
  {
    length=(socklen_t) sizeof(address);
    cache_client=accept(cache_socket,(struct sockaddr *) &address,&length);
    (void) fprintf(stdout,"Connection from (%s, %d)\n",
      inet_ntoa(address.sin_addr),(int) ntohs(address.sin_port));
    count=recv(cache_client,buffer,1,0);
    buffer[count]='\0';
    switch (*buffer)
    {
      case 'c':
      {
        /*
          Create cache.
        */
        break;
      }
      case 'r':
      {
        /*
          Read cache.
        */
        break;
      }
      case 'u':
      {
        /*
          Update cache.
        */
        break;
      }
      case 'd':
      {
        /*
          Delete cache.
        */
        break;
      }
    }
  }
#else
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","'%s' (socket)",image_info->filename);
#endif
}

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
#include "MagickCore/distribute-cache.h"
#include "MagickCore/distribute-cache-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/locale_.h"
#include "MagickCore/memory_.h"
#include "MagickCore/registry.h"
#include "MagickCore/string_.h"
#if defined(MAGICKCORE_HAVE_SOCKET)
#include <sys/socket.h>
#include <netdb.h>
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e D i s t r i b u t e C a c h e I n f o                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireDistributeCacheInfo() allocates the DistributeCacheInfo structure.
%
%  The format of the AcquireDistributeCacheInfo method is:
%
%      DistributeCacheInfo *AcquireDistributeCacheInfo(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickPrivate DistributeCacheInfo *AcquireDistributeCacheInfo(
  ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_SOCKET) && defined(MAGICKCORE_HAVE_PTHREAD)
  DistributeCacheInfo
    *distribute_cache_info;

  distribute_cache_info=(DistributeCacheInfo *) NULL;
  distribute_cache_info=(DistributeCacheInfo *) AcquireMagickMemory(
    sizeof(*distribute_cache_info));
  if (distribute_cache_info == (DistributeCacheInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(distribute_cache_info,0,
    sizeof(*distribute_cache_info));
  distribute_cache_info->signature=MagickSignature;
  return(distribute_cache_info);
#else
  return((DistributeCacheInfo *) NULL);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y D i s t r i b u t e C a c h e I n f o                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyDistributeCacheInfo() deallocates memory associated with an
%  DistributeCacheInfo structure.
%
%  The format of the DestroyDistributeCacheInfo method is:
%
%      DistributeCacheInfo *DestroyDistributeCacheInfo(
%        DistributeCacheInfo *distribute_cache_info)
%
%  A description of each parameter follows:
%
%    o distribute_cache_info: the distributed cache info.
%
*/
MagickPrivate DistributeCacheInfo *DestroyDistributeCacheInfo(
  DistributeCacheInfo *distribute_cache_info)
{
  assert(distribute_cache_info != (DistributeCacheInfo *) NULL);
  assert(distribute_cache_info->signature == MagickSignature);
#if defined(MAGICKCORE_HAVE_SOCKET)
#endif
  distribute_cache_info->signature=(~MagickSignature);
  distribute_cache_info=(DistributeCacheInfo *)
    RelinquishMagickMemory(distribute_cache_info);
  return(distribute_cache_info);
}

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
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void PixelCacheServer(const size_t port,ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_SOCKET) && defined(MAGICKCORE_HAVE_PTHREAD)
  char
    message[MaxTextExtent],
    service[MaxTextExtent];

  int
    file,
    status;

  register struct addrinfo
    *p;

  ssize_t
    count;

  socklen_t
    length;

  struct addrinfo
    hints,
    *result;

  struct sockaddr_storage
    address;

  (void) ResetMagickMemory(&hints,0,sizeof(hints));
  hints.ai_family=AF_UNSPEC;    /* allow IPv4 or IPv6 */
  hints.ai_socktype=SOCK_DGRAM; /* datagram socket */
  hints.ai_flags=AI_PASSIVE;    /* for wildcard IP address */
  hints.ai_protocol=0;          /* any protocol */
  (void) FormatLocaleString(service,MaxTextExtent,"%.20g",(double) port);
  status=getaddrinfo((const char *) NULL,service,&hints,&result);
  if (status != 0)
    {
      (void) fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(status));
      exit(EXIT_FAILURE);
    }
  for (p=result; p != (struct addrinfo *) NULL; p=p->ai_next)
  {
    file=socket(p->ai_family, p->ai_socktype,p->ai_protocol);
    if (file == -1)
      continue;
    if (bind(file,p->ai_addr,p->ai_addrlen) == 0)
      break;
    (void) close(file);
  }
  if (p == NULL)
    {
      (void) fprintf(stderr,"Could not bind\n");
      exit(EXIT_FAILURE);
    }
  freeaddrinfo(result);
  for ( ; ; )
  {
    char
      host[NI_MAXHOST],
      service[NI_MAXSERV];

    ssize_t
      bytes;

    length=(socklen_t) sizeof(struct sockaddr_storage);
    count=recvfrom(file,message,MaxTextExtent,0,(struct sockaddr *) &address,
      &length);
    if (count == -1)
      continue;
    status=getnameinfo((struct sockaddr *) &address,length,host,NI_MAXHOST,
      service,NI_MAXSERV,NI_NUMERICSERV);
    if (status == 0)
      (void) printf("received %ld bytes from %s:%s\n",(long) count,host,
        service);
    else
      (void) fprintf(stderr,"getnameinfo: %s\n", gai_strerror(status));
    bytes=sendto(file,message,(size_t) count,0,(struct sockaddr *) &address,
      length);
    if (bytes != count)
      (void) fprintf(stderr,"error sending response\n");
  }
#else
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","'%s' (socket)",
    "distributed pixel cache");
#endif
}

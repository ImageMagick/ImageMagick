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
%                                   Cristy                                    %
%                                January 2013                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
% very large images.  Start up the pixel cache server on one or more machines.
% When you read or operate on an image and the local pixel cache resources are
% exhausted, ImageMagick contacts one or more of these remote pixel servers to
% store or retrieve pixels.
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/distribute-cache.h"
#include "MagickCore/distribute-cache-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/locale_.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/pixel.h"
#include "MagickCore/policy.h"
#include "MagickCore/random_.h"
#include "MagickCore/registry.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/version.h"
#include "MagickCore/version-private.h"
#undef MAGICKCORE_HAVE_DISTRIBUTE_CACHE
#if defined(MAGICKCORE_HAVE_SOCKET) && defined(MAGICKCORE_THREAD_SUPPORT)
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define CHAR_TYPE_CAST
#define CLOSE_SOCKET(socket) (void) close(socket)
#define HANDLER_RETURN_TYPE void *
#define HANDLER_RETURN_VALUE (void *) NULL
#define SOCKET_TYPE int
#define LENGTH_TYPE size_t
#define MAGICKCORE_HAVE_DISTRIBUTE_CACHE
#elif defined(MAGICKCORE_WINDOWS_SUPPORT) && !defined(__MINGW32__)
#define CHAR_TYPE_CAST (char *)
#define CLOSE_SOCKET(socket) (void) closesocket(socket)
#define HANDLER_RETURN_TYPE DWORD WINAPI
#define HANDLER_RETURN_VALUE 0
#define SOCKET_TYPE SOCKET
#define LENGTH_TYPE int
#define MAGICKCORE_HAVE_DISTRIBUTE_CACHE
#else
#ifdef __VMS
#define CLOSE_SOCKET(socket) (void) close(socket)
#else
#define CLOSE_SOCKET(socket) 
#endif
#define HANDLER_RETURN_TYPE  void *
#define HANDLER_RETURN_VALUE  (void *) NULL
#define SOCKET_TYPE  int
#undef send
#undef recv
#define send(file,buffer,length,flags)  0
#define recv(file,buffer,length,flags)  0
#endif

/*
  Define declarations.
*/
#define DPCHostname  "127.0.0.1"
#define DPCPendingConnections  10
#define DPCPort  6668
#define DPCSessionKeyLength  8
#ifndef MSG_NOSIGNAL
#  define MSG_NOSIGNAL 0
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e D i s t r i b u t e C a c h e I n f o                       %
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

static inline MagickOffsetType dpc_read(int file,const MagickSizeType length,
  unsigned char *magick_restrict message)
{
  register MagickOffsetType
    i;

  ssize_t
    count;

#if !defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
  magick_unreferenced(file);
  magick_unreferenced(message);
#endif

  count=0;
  for (i=0; i < (MagickOffsetType) length; i+=count)
  {
    count=recv(file,CHAR_TYPE_CAST message+i,(LENGTH_TYPE) MagickMin(length-i,
      (MagickSizeType) SSIZE_MAX),0);
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  return(i);
}

static int ConnectPixelCacheServer(const char *hostname,const int port,
  size_t *session_key,ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
  char
    service[MagickPathExtent],
    *shared_secret;

  int
    status;

  SOCKET_TYPE
    client_socket;

  ssize_t
    count;

  struct addrinfo
    hint,
    *result;

  unsigned char
    secret[MagickPathExtent];

  /*
    Connect to distributed pixel cache and get session key.
  */
  *session_key=0;
  shared_secret=GetPolicyValue("cache:shared-secret");
  if (shared_secret == (char *) NULL)
    {
      shared_secret=DestroyString(shared_secret);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s'","shared secret expected");
      return(-1);
    }
  shared_secret=DestroyString(shared_secret);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  NTInitializeWinsock(MagickTrue);
#endif
  (void) memset(&hint,0,sizeof(hint));
  hint.ai_family=AF_INET;
  hint.ai_socktype=SOCK_STREAM;
  hint.ai_flags=AI_PASSIVE;
  (void) FormatLocaleString(service,MagickPathExtent,"%d",port);
  status=getaddrinfo(hostname,service,&hint,&result);
  if (status != 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s'",hostname);
      return(-1);
    }
  client_socket=socket(result->ai_family,result->ai_socktype,
    result->ai_protocol);
  if (client_socket == -1)
    {
      freeaddrinfo(result);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s'",hostname);
      return(-1);
    }
  status=connect(client_socket,result->ai_addr,(socklen_t) result->ai_addrlen);
  if (status == -1)
    {
      CLOSE_SOCKET(client_socket);
      freeaddrinfo(result);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s'",hostname);
      return(-1);
    }
  count=recv(client_socket,CHAR_TYPE_CAST secret,MagickPathExtent,0);
  if (count != -1)
    {
      StringInfo
        *nonce;

      nonce=AcquireStringInfo((size_t) count);
      (void) memcpy(GetStringInfoDatum(nonce),secret,(size_t) count);
      *session_key=GetMagickSignature(nonce);
      nonce=DestroyStringInfo(nonce);
    }
  if (*session_key == 0)
    {
      CLOSE_SOCKET(client_socket);
      client_socket=(SOCKET_TYPE) (-1);
    }
  freeaddrinfo(result);
  return(client_socket);
#else
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","distributed pixel cache");
  return(MagickFalse);
#endif
}

static char *GetHostname(int *port,ExceptionInfo *exception)
{
  char
    *host,
    *hosts,
    **hostlist;

  int
    argc;

  register ssize_t
    i;

  static size_t
    id = 0;

  /*
    Parse host list (e.g. 192.168.100.1:6668,192.168.100.2:6668).
  */
  hosts=(char *) GetImageRegistry(StringRegistryType,"cache:hosts",exception);
  if (hosts == (char *) NULL)
    {
      *port=DPCPort;
      return(AcquireString(DPCHostname));
    }
  (void) SubstituteString(&hosts,","," ");
  hostlist=StringToArgv(hosts,&argc);
  hosts=DestroyString(hosts);
  if (hostlist == (char **) NULL)
    {
      *port=DPCPort;
      return(AcquireString(DPCHostname));
    }
  hosts=AcquireString(hostlist[(id++ % (argc-1))+1]);
  for (i=0; i < (ssize_t) argc; i++)
    hostlist[i]=DestroyString(hostlist[i]);
  hostlist=(char **) RelinquishMagickMemory(hostlist);
  (void) SubstituteString(&hosts,":"," ");
  hostlist=StringToArgv(hosts,&argc);
  if (hostlist == (char **) NULL)
    {
      *port=DPCPort;
      return(AcquireString(DPCHostname));
    }
  host=AcquireString(hostlist[1]);
  if (hostlist[2] == (char *) NULL)
    *port=DPCPort;
  else
    *port=StringToLong(hostlist[2]);
  for (i=0; i < (ssize_t) argc; i++)
    hostlist[i]=DestroyString(hostlist[i]);
  hostlist=(char **) RelinquishMagickMemory(hostlist);
  return(host);
}

MagickPrivate DistributeCacheInfo *AcquireDistributeCacheInfo(
  ExceptionInfo *exception)
{
  char
    *hostname;

  DistributeCacheInfo
    *server_info;

  size_t
    session_key;

  /*
    Connect to the distributed pixel cache server.
  */
  server_info=(DistributeCacheInfo *) AcquireCriticalMemory(
    sizeof(*server_info));
  (void) memset(server_info,0,sizeof(*server_info));
  server_info->signature=MagickCoreSignature;
  server_info->port=0;
  hostname=GetHostname(&server_info->port,exception);
  session_key=0;
  server_info->file=ConnectPixelCacheServer(hostname,server_info->port,
    &session_key,exception);
  if (server_info->file == -1)
    server_info=DestroyDistributeCacheInfo(server_info);
  else
    {
      server_info->session_key=session_key;
      (void) CopyMagickString(server_info->hostname,hostname,MagickPathExtent);
      server_info->debug=IsEventLogging();
    }
  hostname=DestroyString(hostname);
  return(server_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y D i s t r i b u t e C a c h e I n f o                       %
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
%        DistributeCacheInfo *server_info)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
*/
MagickPrivate DistributeCacheInfo *DestroyDistributeCacheInfo(
  DistributeCacheInfo *server_info)
{
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  if (server_info->file > 0)
    CLOSE_SOCKET(server_info->file);
  server_info->signature=(~MagickCoreSignature);
  server_info=(DistributeCacheInfo *) RelinquishMagickMemory(server_info);
  return(server_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D i s t r i b u t e P i x e l C a c h e S e r v e r                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DistributePixelCacheServer() waits on the specified port for commands to
%  create, read, update, or destroy a pixel cache.
%
%  The format of the DistributePixelCacheServer() method is:
%
%      void DistributePixelCacheServer(const int port)
%
%  A description of each parameter follows:
%
%    o port: connect the distributed pixel cache at this port.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType DestroyDistributeCache(SplayTreeInfo *registry,
  const size_t session_key)
{
  /*
    Destroy distributed pixel cache.
  */
  return(DeleteNodeFromSplayTree(registry,(const void *) session_key));
}

static inline MagickOffsetType dpc_send(int file,const MagickSizeType length,
  const unsigned char *magick_restrict message)
{
  MagickOffsetType
    count;

  register MagickOffsetType
    i;

#if !defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
  magick_unreferenced(file);
  magick_unreferenced(message);
#endif

  /*
    Ensure a complete message is sent.
  */
  count=0;
  for (i=0; i < (MagickOffsetType) length; i+=count)
  {
    count=(MagickOffsetType) send(file,CHAR_TYPE_CAST message+i,(LENGTH_TYPE)
      MagickMin(length-i,(MagickSizeType) SSIZE_MAX),MSG_NOSIGNAL);
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  return(i);
}

static MagickBooleanType OpenDistributeCache(SplayTreeInfo *registry,int file,
  const size_t session_key,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    count;

  MagickSizeType
    length;

  register unsigned char
    *p;

  unsigned char
    message[MagickPathExtent];

  /*
    Open distributed pixel cache.
  */
  image=AcquireImage((ImageInfo *) NULL,exception);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(image->storage_class)+sizeof(image->colorspace)+
    sizeof(image->alpha_trait)+sizeof(image->channels)+sizeof(image->columns)+
    sizeof(image->rows)+sizeof(image->number_channels)+MaxPixelChannels*
    sizeof(*image->channel_map)+sizeof(image->metacontent_extent);
  count=dpc_read(file,length,message);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  /*
    Deserialize the image attributes.
  */
  p=message;
  (void) memcpy(&image->storage_class,p,sizeof(image->storage_class));
  p+=sizeof(image->storage_class);
  (void) memcpy(&image->colorspace,p,sizeof(image->colorspace));
  p+=sizeof(image->colorspace);
  (void) memcpy(&image->alpha_trait,p,sizeof(image->alpha_trait));
  p+=sizeof(image->alpha_trait);
  (void) memcpy(&image->channels,p,sizeof(image->channels));
  p+=sizeof(image->channels);
  (void) memcpy(&image->channels,p,sizeof(image->channels));
  p+=sizeof(image->channels);
  (void) memcpy(&image->columns,p,sizeof(image->columns));
  p+=sizeof(image->columns);
  (void) memcpy(&image->rows,p,sizeof(image->rows));
  p+=sizeof(image->rows);
  (void) memcpy(&image->number_channels,p,sizeof(image->number_channels));
  p+=sizeof(image->number_channels);
  (void) memcpy(image->channel_map,p,MaxPixelChannels*
    sizeof(*image->channel_map));
  p+=MaxPixelChannels*sizeof(*image->channel_map);
  (void) memcpy(&image->metacontent_extent,p,sizeof(image->metacontent_extent));
  p+=sizeof(image->metacontent_extent);
  if (SyncImagePixelCache(image,exception) == MagickFalse)
    return(MagickFalse);
  status=AddValueToSplayTree(registry,(const void *) session_key,image);
  return(status);
}

static MagickBooleanType ReadDistributeCacheMetacontent(SplayTreeInfo *registry,
  int file,const size_t session_key,ExceptionInfo *exception)
{
  const unsigned char
    *metacontent;

  Image
    *image;

  MagickOffsetType
    count;

  MagickSizeType
    length;

  RectangleInfo
    region;

  register const Quantum
    *p;

  register unsigned char
    *q;

  unsigned char
    message[MagickPathExtent];

  /*
    Read distributed pixel cache metacontent.
  */
  image=(Image *) GetValueFromSplayTree(registry,(const void *) session_key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=dpc_read(file,length,message);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  q=message;
  (void) memcpy(&region.width,q,sizeof(region.width));
  q+=sizeof(region.width);
  (void) memcpy(&region.height,q,sizeof(region.height));
  q+=sizeof(region.height);
  (void) memcpy(&region.x,q,sizeof(region.x));
  q+=sizeof(region.x);
  (void) memcpy(&region.y,q,sizeof(region.y));
  q+=sizeof(region.y);
  (void) memcpy(&length,q,sizeof(length));
  q+=sizeof(length);
  p=GetVirtualPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  metacontent=(const unsigned char *) GetVirtualMetacontent(image);
  count=dpc_send(file,length,metacontent);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ReadDistributeCachePixels(SplayTreeInfo *registry,
  int file,const size_t session_key,ExceptionInfo *exception)
{
  Image
    *image;

  MagickOffsetType
    count;

  MagickSizeType
    length;

  RectangleInfo
    region;

  register const Quantum
    *p;

  register unsigned char
    *q;

  unsigned char
    message[MagickPathExtent];

  /*
    Read distributed pixel cache pixels.
  */
  image=(Image *) GetValueFromSplayTree(registry,(const void *) session_key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=dpc_read(file,length,message);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  q=message;
  (void) memcpy(&region.width,q,sizeof(region.width));
  q+=sizeof(region.width);
  (void) memcpy(&region.height,q,sizeof(region.height));
  q+=sizeof(region.height);
  (void) memcpy(&region.x,q,sizeof(region.x));
  q+=sizeof(region.x);
  (void) memcpy(&region.y,q,sizeof(region.y));
  q+=sizeof(region.y);
  (void) memcpy(&length,q,sizeof(length));
  q+=sizeof(length);
  p=GetVirtualPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  count=dpc_send(file,length,(unsigned char *) p);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  return(MagickTrue);
}

static void *RelinquishImageRegistry(void *image)
{
  return((void *) DestroyImageList((Image *) image));
}

static MagickBooleanType WriteDistributeCacheMetacontent(
  SplayTreeInfo *registry,int file,const size_t session_key,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickOffsetType
    count;

  MagickSizeType
    length;

  RectangleInfo
    region;

  register Quantum
    *q;

  register unsigned char
    *p;

  unsigned char
    message[MagickPathExtent],
    *metacontent;

  /*
    Write distributed pixel cache metacontent.
  */
  image=(Image *) GetValueFromSplayTree(registry,(const void *) session_key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=dpc_read(file,length,message);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  p=message;
  (void) memcpy(&region.width,p,sizeof(region.width));
  p+=sizeof(region.width);
  (void) memcpy(&region.height,p,sizeof(region.height));
  p+=sizeof(region.height);
  (void) memcpy(&region.x,p,sizeof(region.x));
  p+=sizeof(region.x);
  (void) memcpy(&region.y,p,sizeof(region.y));
  p+=sizeof(region.y);
  (void) memcpy(&length,p,sizeof(length));
  p+=sizeof(length);
  q=GetAuthenticPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  metacontent=(unsigned char *) GetAuthenticMetacontent(image);
  count=dpc_read(file,length,metacontent);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  return(SyncAuthenticPixels(image,exception));
}

static MagickBooleanType WriteDistributeCachePixels(SplayTreeInfo *registry,
  int file,const size_t session_key,ExceptionInfo *exception)
{
  Image
    *image;

  MagickOffsetType
    count;

  MagickSizeType
    length;

  RectangleInfo
    region;

  register Quantum
    *q;

  register unsigned char
    *p;

  unsigned char
    message[MagickPathExtent];

  /*
    Write distributed pixel cache pixels.
  */
  image=(Image *) GetValueFromSplayTree(registry,(const void *) session_key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=dpc_read(file,length,message);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  p=message;
  (void) memcpy(&region.width,p,sizeof(region.width));
  p+=sizeof(region.width);
  (void) memcpy(&region.height,p,sizeof(region.height));
  p+=sizeof(region.height);
  (void) memcpy(&region.x,p,sizeof(region.x));
  p+=sizeof(region.x);
  (void) memcpy(&region.y,p,sizeof(region.y));
  p+=sizeof(region.y);
  (void) memcpy(&length,p,sizeof(length));
  p+=sizeof(length);
  q=GetAuthenticPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  count=dpc_read(file,length,(unsigned char *) q);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  return(SyncAuthenticPixels(image,exception));
}

static HANDLER_RETURN_TYPE DistributePixelCacheClient(void *socket)
{
  char
    *shared_secret;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  MagickOffsetType
    count;

  register unsigned char
    *p;

  RandomInfo
    *random_info;

  size_t
    key,
    session_key;

  SOCKET_TYPE
    client_socket;

  SplayTreeInfo
    *registry;

  StringInfo
    *secret;

  unsigned char
    command,
    session[2*MagickPathExtent];

  /*
    Distributed pixel cache client.
  */
  shared_secret=GetPolicyValue("cache:shared-secret");
  if (shared_secret == (char *) NULL)
    ThrowFatalException(CacheFatalError,"shared secret expected");
  p=session;
  (void) CopyMagickString((char *) p,shared_secret,MagickPathExtent);
  p+=strlen(shared_secret);
  shared_secret=DestroyString(shared_secret);
  random_info=AcquireRandomInfo();
  secret=GetRandomKey(random_info,DPCSessionKeyLength);
  (void) memcpy(p,GetStringInfoDatum(secret),DPCSessionKeyLength);
  session_key=GetMagickSignature(secret);
  random_info=DestroyRandomInfo(random_info);
  exception=AcquireExceptionInfo();
  registry=NewSplayTree((int (*)(const void *,const void *)) NULL,
    (void *(*)(void *)) NULL,RelinquishImageRegistry);
  client_socket=(*(SOCKET_TYPE *) socket);
  count=dpc_send(client_socket,DPCSessionKeyLength,GetStringInfoDatum(secret));
  secret=DestroyStringInfo(secret);
  for ( ; ; )
  {
    count=dpc_read(client_socket,1,(unsigned char *) &command);
    if (count <= 0)
      break;
    count=dpc_read(client_socket,sizeof(key),(unsigned char *) &key);
    if ((count != (MagickOffsetType) sizeof(key)) || (key != session_key))
      break;
    status=MagickFalse;
    switch (command)
    {
      case 'o':
      {
        status=OpenDistributeCache(registry,client_socket,session_key,
          exception);
        count=dpc_send(client_socket,sizeof(status),(unsigned char *) &status);
        break;
      }
      case 'r':
      {
        status=ReadDistributeCachePixels(registry,client_socket,session_key,
          exception);
        break;
      }
      case 'R':
      {
        status=ReadDistributeCacheMetacontent(registry,client_socket,
          session_key,exception);
        break;
      }
      case 'w':
      {
        status=WriteDistributeCachePixels(registry,client_socket,session_key,
          exception);
        break;
      }
      case 'W':
      {
        status=WriteDistributeCacheMetacontent(registry,client_socket,
          session_key,exception);
        break;
      }
      case 'd':
      {
        status=DestroyDistributeCache(registry,session_key);
        break;
      }
      default:
        break;
    }
    if (status == MagickFalse)
      break;
    if (command == 'd')
      break;
  }
  count=dpc_send(client_socket,sizeof(status),(unsigned char *) &status);
  CLOSE_SOCKET(client_socket);
  exception=DestroyExceptionInfo(exception);
  registry=DestroySplayTree(registry);
  return(HANDLER_RETURN_VALUE);
}

MagickExport void DistributePixelCacheServer(const int port,
  ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
  char
    service[MagickPathExtent];

  int
    status;

#if defined(MAGICKCORE_THREAD_SUPPORT)
  pthread_attr_t
    attributes;

  pthread_t
    threads;
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
  DWORD
    threadID;
#else
  Not implemented!
#endif

  register struct addrinfo
    *p;

  SOCKET_TYPE
    server_socket;

  struct addrinfo
    hint,
    *result;

  struct sockaddr_in
    address;

  /*
    Launch distributed pixel cache server.
  */
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  magick_unreferenced(exception);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  NTInitializeWinsock(MagickFalse);
#endif
  (void) memset(&hint,0,sizeof(hint));
  hint.ai_family=AF_INET;
  hint.ai_socktype=SOCK_STREAM;
  hint.ai_flags=AI_PASSIVE;
  (void) FormatLocaleString(service,MagickPathExtent,"%d",port);
  status=getaddrinfo((const char *) NULL,service,&hint,&result);
  if (status != 0)
    ThrowFatalException(CacheFatalError,"UnableToListen");
  server_socket=(SOCKET_TYPE) 0;
  for (p=result; p != (struct addrinfo *) NULL; p=p->ai_next)
  {
    int
      one;

    server_socket=socket(p->ai_family,p->ai_socktype,p->ai_protocol);
    if (server_socket == -1)
      continue;
    one=1;
    status=setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,
      CHAR_TYPE_CAST &one,(socklen_t) sizeof(one));
    if (status == -1)
      {
        CLOSE_SOCKET(server_socket);
        continue;
      }
    status=bind(server_socket,p->ai_addr,(socklen_t) p->ai_addrlen);
    if (status == -1)
      {
        CLOSE_SOCKET(server_socket);
        continue;
      }
    break;
  }
  if (p == (struct addrinfo *) NULL)
    ThrowFatalException(CacheFatalError,"UnableToBind");
  freeaddrinfo(result);
  status=listen(server_socket,DPCPendingConnections);
  if (status != 0)
    ThrowFatalException(CacheFatalError,"UnableToListen");
#if defined(MAGICKCORE_THREAD_SUPPORT)
  pthread_attr_init(&attributes);
#endif
  for ( ; ; )
  {
    SOCKET_TYPE
      client_socket;

    socklen_t
      length;

    length=(socklen_t) sizeof(address);
    client_socket=accept(server_socket,(struct sockaddr *) &address,&length);
    if (client_socket == -1)
      ThrowFatalException(CacheFatalError,"UnableToEstablishConnection");
#if defined(MAGICKCORE_THREAD_SUPPORT)
    status=pthread_create(&threads,&attributes,DistributePixelCacheClient,
      (void *) &client_socket);
    if (status == -1)
      ThrowFatalException(CacheFatalError,"UnableToCreateClientThread");
#elif defined(MAGICKCORE_WINDOWS_SUPPORT)
    if (CreateThread(0,0,DistributePixelCacheClient,(void*) &client_socket,0,
        &threadID) == (HANDLE) NULL)
      ThrowFatalException(CacheFatalError,"UnableToCreateClientThread");
#else
    Not implemented!
#endif
  }
#else
  magick_unreferenced(port);
  ThrowFatalException(MissingDelegateError,"DelegateLibrarySupportNotBuiltIn");
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t D i s t r i b u t e C a c h e F i l e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDistributeCacheFile() returns the file associated with this
%  DistributeCacheInfo structure.
%
%  The format of the GetDistributeCacheFile method is:
%
%      int GetDistributeCacheFile(const DistributeCacheInfo *server_info)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
*/
MagickPrivate int GetDistributeCacheFile(const DistributeCacheInfo *server_info)
{
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  return(server_info->file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t D i s t r i b u t e C a c h e H o s t n a m e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDistributeCacheHostname() returns the hostname associated with this
%  DistributeCacheInfo structure.
%
%  The format of the GetDistributeCacheHostname method is:
%
%      const char *GetDistributeCacheHostname(
%        const DistributeCacheInfo *server_info)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
*/
MagickPrivate const char *GetDistributeCacheHostname(
  const DistributeCacheInfo *server_info)
{
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  return(server_info->hostname);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t D i s t r i b u t e C a c h e P o r t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDistributeCachePort() returns the port associated with this
%  DistributeCacheInfo structure.
%
%  The format of the GetDistributeCachePort method is:
%
%      int GetDistributeCachePort(const DistributeCacheInfo *server_info)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
*/
MagickPrivate int GetDistributeCachePort(const DistributeCacheInfo *server_info)
{
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  return(server_info->port);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   O p e n D i s t r i b u t e P i x e l C a c h e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenDistributePixelCache() opens a pixel cache on a remote server.
%
%  The format of the OpenDistributePixelCache method is:
%
%      MagickBooleanType *OpenDistributePixelCache(
%        DistributeCacheInfo *server_info,Image *image)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
%    o image: the image.
%
*/
MagickPrivate MagickBooleanType OpenDistributePixelCache(
  DistributeCacheInfo *server_info,Image *image)
{
  MagickBooleanType
#ifdef __VMS
     status=MagickTrue;
#else
    status;
#endif

  MagickOffsetType
    count;

  register unsigned char
    *p;

  unsigned char
    message[MagickPathExtent];

  /*
    Open distributed pixel cache.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  p=message;
  *p++='o';  /* open */
  /*
    Serialize image attributes (see ValidatePixelCacheMorphology()).
  */
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  (void) memcpy(p,&image->storage_class,sizeof(image->storage_class));
  p+=sizeof(image->storage_class);
  (void) memcpy(p,&image->colorspace,sizeof(image->colorspace));
  p+=sizeof(image->colorspace);
  (void) memcpy(p,&image->alpha_trait,sizeof(image->alpha_trait));
  p+=sizeof(image->alpha_trait);
  (void) memcpy(p,&image->channels,sizeof(image->channels));
  p+=sizeof(image->channels);
  (void) memcpy(p,&image->columns,sizeof(image->columns));
  p+=sizeof(image->columns);
  (void) memcpy(p,&image->rows,sizeof(image->rows));
  p+=sizeof(image->rows);
  (void) memcpy(p,&image->number_channels,sizeof(image->number_channels));
  p+=sizeof(image->number_channels);
  (void) memcpy(p,image->channel_map,MaxPixelChannels*
    sizeof(*image->channel_map));
  p+=MaxPixelChannels*sizeof(*image->channel_map);
  (void) memcpy(p,&image->metacontent_extent,sizeof(image->metacontent_extent));
  p+=sizeof(image->metacontent_extent);
  count=dpc_send(server_info->file,p-message,message);
  if (count != (MagickOffsetType) (p-message))
    return(MagickFalse);
  status=MagickFalse;
  count=dpc_read(server_info->file,sizeof(status),(unsigned char *) &status);
  if (count != (MagickOffsetType) sizeof(status))
    return(MagickFalse);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d D i s t r i b u t e P i x e l C a c h e M e t a c o n t e n t     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDistributePixelCacheMetacontents() reads metacontent from the specified
%  region of the distributed pixel cache.
%
%  The format of the ReadDistributePixelCacheMetacontents method is:
%
%      MagickOffsetType ReadDistributePixelCacheMetacontents(
%        DistributeCacheInfo *server_info,const RectangleInfo *region,
%        const MagickSizeType length,unsigned char *metacontent)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
%    o image: the image.
%
%    o region: read the metacontent from this region of the image.
%
%    o length: the length in bytes of the metacontent.
%
%    o metacontent: read these metacontent from the pixel cache.
%
*/
MagickPrivate MagickOffsetType ReadDistributePixelCacheMetacontent(
  DistributeCacheInfo *server_info,const RectangleInfo *region,
  const MagickSizeType length,unsigned char *metacontent)
{
  MagickOffsetType
    count;

  register unsigned char
    *p;

  unsigned char
    message[MagickPathExtent];

  /*
    Read distributed pixel cache metacontent.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(metacontent != (unsigned char *) NULL);
  if (length > (MagickSizeType) SSIZE_MAX)
    return(-1);
  p=message;
  *p++='R';
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=sizeof(length);
  count=dpc_send(server_info->file,p-message,message);
  if (count != (MagickOffsetType) (p-message))
    return(-1);
  return(dpc_read(server_info->file,length,metacontent));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e a d D i s t r i b u t e P i x e l C a c h e P i x e l s               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDistributePixelCachePixels() reads pixels from the specified region of
%  the distributed pixel cache.
%
%  The format of the ReadDistributePixelCachePixels method is:
%
%      MagickOffsetType ReadDistributePixelCachePixels(
%        DistributeCacheInfo *server_info,const RectangleInfo *region,
%        const MagickSizeType length,unsigned char *magick_restrict pixels)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
%    o image: the image.
%
%    o region: read the pixels from this region of the image.
%
%    o length: the length in bytes of the pixels.
%
%    o pixels: read these pixels from the pixel cache.
%
*/
MagickPrivate MagickOffsetType ReadDistributePixelCachePixels(
  DistributeCacheInfo *server_info,const RectangleInfo *region,
  const MagickSizeType length,unsigned char *magick_restrict pixels)
{
  MagickOffsetType
    count;

  register unsigned char
    *p;

  unsigned char
    message[MagickPathExtent];

  /*
    Read distributed pixel cache pixels.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(pixels != (unsigned char *) NULL);
  if (length > (MagickSizeType) SSIZE_MAX)
    return(-1);
  p=message;
  *p++='r';
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=sizeof(length);
  count=dpc_send(server_info->file,p-message,message);
  if (count != (MagickOffsetType) (p-message))
    return(-1);
  return(dpc_read(server_info->file,length,pixels));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e l i n q u i s h D i s t r i b u t e P i x e l C a c h e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishDistributePixelCache() frees resources acquired with
%  OpenDistributePixelCache().
%
%  The format of the RelinquishDistributePixelCache method is:
%
%      MagickBooleanType RelinquishDistributePixelCache(
%        DistributeCacheInfo *server_info)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
*/
MagickPrivate MagickBooleanType RelinquishDistributePixelCache(
  DistributeCacheInfo *server_info)
{
  MagickBooleanType
#ifdef __VMS
     status = MagickTrue;
#else
    status;
#endif

  MagickOffsetType
    count;

  register unsigned char
    *p;

  unsigned char
    message[MagickPathExtent];

  /*
    Delete distributed pixel cache.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  p=message;
  *p++='d';
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  count=dpc_send(server_info->file,p-message,message);
  if (count != (MagickOffsetType) (p-message))
    return(MagickFalse);
  count=dpc_read(server_info->file,sizeof(status),(unsigned char *) &status);
  if (count != (MagickOffsetType) sizeof(status))
    return(MagickFalse);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e D i s t r i b u t e P i x e l C a c h e M e t a c o n t e n t   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteDistributePixelCacheMetacontents() writes image metacontent to the
%  specified region of the distributed pixel cache.
%
%  The format of the WriteDistributePixelCacheMetacontents method is:
%
%      MagickOffsetType WriteDistributePixelCacheMetacontents(
%        DistributeCacheInfo *server_info,const RectangleInfo *region,
%        const MagickSizeType length,const unsigned char *metacontent)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
%    o image: the image.
%
%    o region: write the metacontent to this region of the image.
%
%    o length: the length in bytes of the metacontent.
%
%    o metacontent: write these metacontent to the pixel cache.
%
*/
MagickPrivate MagickOffsetType WriteDistributePixelCacheMetacontent(
  DistributeCacheInfo *server_info,const RectangleInfo *region,
  const MagickSizeType length,const unsigned char *metacontent)
{
  MagickOffsetType
    count;

  register unsigned char
    *p;

  unsigned char
    message[MagickPathExtent];

  /*
    Write distributed pixel cache metacontent.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(metacontent != (unsigned char *) NULL);
  if (length > (MagickSizeType) SSIZE_MAX)
    return(-1);
  p=message;
  *p++='W';
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=sizeof(length);
  count=dpc_send(server_info->file,p-message,message);
  if (count != (MagickOffsetType) (p-message))
    return(-1);
  return(dpc_send(server_info->file,length,metacontent));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   W r i t e D i s t r i b u t e P i x e l C a c h e P i x e l s             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteDistributePixelCachePixels() writes image pixels to the specified
%  region of the distributed pixel cache.
%
%  The format of the WriteDistributePixelCachePixels method is:
%
%      MagickBooleanType WriteDistributePixelCachePixels(
%        DistributeCacheInfo *server_info,const RectangleInfo *region,
%        const MagickSizeType length,
%        const unsigned char *magick_restrict pixels)
%
%  A description of each parameter follows:
%
%    o server_info: the distributed cache info.
%
%    o image: the image.
%
%    o region: write the pixels to this region of the image.
%
%    o length: the length in bytes of the pixels.
%
%    o pixels: write these pixels to the pixel cache.
%
*/
MagickPrivate MagickOffsetType WriteDistributePixelCachePixels(
  DistributeCacheInfo *server_info,const RectangleInfo *region,
  const MagickSizeType length,const unsigned char *magick_restrict pixels)
{
  MagickOffsetType
    count;

  register unsigned char
    *p;

  unsigned char
    message[MagickPathExtent];

  /*
    Write distributed pixel cache pixels.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(pixels != (const unsigned char *) NULL);
  if (length > (MagickSizeType) SSIZE_MAX)
    return(-1);
  p=message;
  *p++='w';
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=sizeof(length);
  count=dpc_send(server_info->file,p-message,message);
  if (count != (MagickOffsetType) (p-message))
    return(-1);
  return(dpc_send(server_info->file,length,pixels));
}

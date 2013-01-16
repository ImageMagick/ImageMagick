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
#include "MagickCore/locale_.h"
#include "MagickCore/memory_.h"
#include "MagickCore/policy.h"
#include "MagickCore/random_.h"
#include "MagickCore/registry.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#if defined(MAGICKCORE_HAVE_SOCKET)
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

/*
  Define declarations.
*/
#define DPCHostname  "127.0.0.1"
#define DPCPort  6668
#define DPCSessionKeyLength  8

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

static MagickSizeType CRC64(const unsigned char *message,const size_t length)
{
  MagickSizeType
    crc;

  register ssize_t
    i;

  static MagickBooleanType
    crc_initial = MagickFalse;

  static MagickSizeType
    crc_xor[256];

  if (crc_initial == MagickFalse)
    {
      MagickSizeType
        alpha;

      for (i=0; i < 256; i++)
      {
        register ssize_t
          j;

        alpha=(MagickSizeType) i;
        for (j=0; j < 8; j++)
        {
          if ((alpha & 0x01) == 0)
            alpha>>=1;
          else
            alpha=(MagickSizeType) ((alpha >> 1) ^
              MagickULLConstant(0xd800000000000000));
        }
        crc_xor[i]=alpha;
      }
      crc_initial=MagickTrue;
    }
  crc=0;
  for (i=0; i < (ssize_t) length; i++)
    crc=crc_xor[(crc ^ message[i]) & 0xff] ^ (crc >> 8);
  return(crc);
}

static int ConnectPixelCacheServer(const char *hostname,const int port,
  MagickSizeType *session_key,ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_SOCKET)
  char
    secret[MaxTextExtent];

  const char
    *shared_secret;

  int
    client_socket,
    status;

  ssize_t
    count;

  struct hostent
    *host;

  struct sockaddr_in
    address;

  unsigned char
    session[MaxTextExtent];

  /*
    Connect to distributed pixel cache and get session key.
  */
  *session_key=0;
  shared_secret=GetPolicyValue("shared-secret");
  if (shared_secret == (const char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s'","shared secret expected");
      return(-1);
    }
  (void) CopyMagickString((char *) session,shared_secret,MaxTextExtent-
    DPCSessionKeyLength);
  host=gethostbyname(hostname);
  client_socket=socket(AF_INET,SOCK_STREAM,0);
  if (client_socket == -1)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s'",hostname);
      return(-1);
    }
  (void) ResetMagickMemory(&address,0,sizeof(address));
  address.sin_family=AF_INET;
  address.sin_port=htons((uint16_t) port);
  address.sin_addr=(*((struct in_addr *) host->h_addr));
  status=connect(client_socket,(struct sockaddr *) &address,(socklen_t)
    sizeof(struct sockaddr));
  if (status == -1)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s'",hostname);
      return(-1);
    }
  count=read(client_socket,secret,MaxTextExtent);
  if (count != -1)
    {
      (void) memcpy(session+strlen(shared_secret),secret,(size_t) count);
      *session_key=CRC64(session,strlen(shared_secret)+count);
    }
  if (*session_key == 0)
    {
      close(client_socket);
      client_socket=(-1);
    }
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
  hosts=(char *) GetImageRegistry(StringRegistryType,"cache:hosts",
    exception);
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
    *distribute_cache_info;

  MagickSizeType
    session_key;

  distribute_cache_info=(DistributeCacheInfo *) AcquireMagickMemory(
    sizeof(*distribute_cache_info));
  if (distribute_cache_info == (DistributeCacheInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(distribute_cache_info,0,
    sizeof(*distribute_cache_info));
  distribute_cache_info->signature=MagickSignature;
  /*
    Contact pixel cache server.
  */
  distribute_cache_info->port=0;
  hostname=GetHostname(&distribute_cache_info->port,exception);
  session_key=0;
  distribute_cache_info->file=ConnectPixelCacheServer(hostname,
    distribute_cache_info->port,&session_key,exception);
  distribute_cache_info->session_key=session_key;
  (void) CopyMagickString(distribute_cache_info->hostname,hostname,
    MaxTextExtent);
  hostname=DestroyString(hostname);
  if (distribute_cache_info->file == -1)
    distribute_cache_info=DestroyDistributeCacheInfo(distribute_cache_info);
  return(distribute_cache_info);
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
  distribute_cache_info->signature=(~MagickSignature);
  distribute_cache_info=(DistributeCacheInfo *) RelinquishMagickMemory(
    distribute_cache_info);
  return(distribute_cache_info);
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
%      void DistributePixelCacheServer(const size_t port)
%
%  A description of each parameter follows:
%
%    o port: connect the distributed pixel cache at this port.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType CreateDistributeCache(SplayTreeInfo *image_registry,
  int file,const MagickSizeType session_key)
{
  ExceptionInfo
    *exception;

  Image
    *image;

  MagickBooleanType
    status;

  ssize_t
    count;

  exception=AcquireExceptionInfo();
  image=AcquireImage((ImageInfo *) NULL,exception);
  exception=DestroyExceptionInfo(exception);
  count=read(file,&image->columns,sizeof(image->columns));
  if (count != (ssize_t) sizeof(image->columns))
    return(MagickFalse);
  count=read(file,&image->rows,sizeof(image->rows));
  if (count != (ssize_t) sizeof(image->rows))
    return(MagickFalse);
  count=read(file,&image->number_channels,sizeof(image->number_channels));
  if (count != (ssize_t) sizeof(image->number_channels))
    return(MagickFalse);
  status=AddValueToSplayTree(image_registry,(const void *) session_key,image);
  return(status);
}

static MagickBooleanType DestroyDistributeCache(SplayTreeInfo *image_registry,
  int file,const MagickSizeType session_key)
{
  char
    key[MaxTextExtent];

  (void) FormatLocaleString(key,MaxTextExtent,"%.20g",(double) session_key);
  return(DeleteImageRegistry(key));
}

static MagickBooleanType ReadDistributeCache(SplayTreeInfo *image_registry,
  int file,const MagickSizeType session_key)
{
  ExceptionInfo
    *exception;

  Image
    *image;

  RectangleInfo
    region;

  register const Quantum
    *p;

  size_t
    length;

  ssize_t
    count;

  image=(Image *) GetValueFromSplayTree(image_registry,(const void *)
    session_key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  count=read(file,&region.width,sizeof(region.width));
  if (count != (ssize_t) sizeof(region.width))
    return(MagickFalse);
  count=read(file,&region.height,sizeof(region.height));
  if (count != (ssize_t) sizeof(region.height))
    return(MagickFalse);
  count=read(file,&region.x,sizeof(region.x));
  if (count != (ssize_t) sizeof(region.x))
    return(MagickFalse);
  count=read(file,&region.y,sizeof(region.y));
  if (count != (ssize_t) sizeof(region.y))
    return(MagickFalse);
  count=read(file,&length,sizeof(length));
  if (count != (ssize_t) sizeof(length))
    return(MagickFalse);
  exception=AcquireExceptionInfo();
  p=GetVirtualPixels(image,region.x,region.y,region.width,region.height,
    exception);
  exception=DestroyExceptionInfo(exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  count=write(file,p,length);
  if (count != (ssize_t) length)
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType WriteDistributeCache(SplayTreeInfo *image_registry,
  int file,const MagickSizeType session_key)
{
  ExceptionInfo
    *exception;

  Image
    *image;

  MagickBooleanType
    status;

  RectangleInfo
    region;

  register Quantum
    *p;

  size_t
    length;

  ssize_t
    count;

  image=(Image *) GetValueFromSplayTree(image_registry,(const void *)
    session_key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  count=read(file,&region.width,sizeof(region.width));
  if (count != (ssize_t) sizeof(region.width))
    return(MagickFalse);
  count=read(file,&region.height,sizeof(region.height));
  if (count != (ssize_t) sizeof(region.height))
    return(MagickFalse);
  count=read(file,&region.x,sizeof(region.x));
  if (count != (ssize_t) sizeof(region.x))
    return(MagickFalse);
  count=read(file,&region.y,sizeof(region.y));
  if (count != (ssize_t) sizeof(region.y))
    return(MagickFalse);
  count=read(file,&length,sizeof(length));
  if (count != (ssize_t) sizeof(length))
    return(MagickFalse);
  exception=AcquireExceptionInfo();
  p=GetAuthenticPixels(image,region.x,region.y,region.width,region.height,
    exception);
  exception=DestroyExceptionInfo(exception);
  if (p == (Quantum *) NULL)
    return(MagickFalse);
  count=read(file,p,length);
  if (count != (ssize_t) length)
    return(MagickFalse);
  status=SyncAuthenticPixels(image,exception);
  return(status);
}

static void *DistributePixelCacheClient(void *socket)
{
  const char
    *shared_secret;

  int
    client_socket;

  MagickBooleanType
    status;

  MagickSizeType
    key,
    session_key;

  RandomInfo
    *random_info;

  SplayTreeInfo
    *image_registry;

  ssize_t
    count;

  StringInfo
    *secret;

  unsigned char
    command,
    session[MaxTextExtent];

  /*
    Generate session key.
  */
  shared_secret=GetPolicyValue("shared-secret");
  if (shared_secret == (const char *) NULL)
    ThrowFatalException(CacheFatalError,"shared secret expected");
  (void) CopyMagickString((char *) session,shared_secret,MaxTextExtent-
    DPCSessionKeyLength);
  random_info=AcquireRandomInfo();
  secret=GetRandomKey(random_info,DPCSessionKeyLength);
  (void) memcpy(session+strlen(shared_secret),GetStringInfoDatum(secret),
    DPCSessionKeyLength);
  session_key=CRC64(session,strlen(shared_secret)+DPCSessionKeyLength);
  random_info=DestroyRandomInfo(random_info);
  image_registry=NewSplayTree((int (*)(const void *,const void *)) NULL,
    (void *(*)(void *)) NULL,(void *(*)(void *)) NULL);
  client_socket=(*(int *) socket);
  count=write(client_socket,GetStringInfoDatum(secret),DPCSessionKeyLength);
  secret=DestroyStringInfo(secret);
  for ( ; ; )
  {
    count=read(client_socket,&command,1);
    if (count <= 0)
      break;
    count=read(client_socket,&key,sizeof(key));
    if ((count != (ssize_t) sizeof(key)) && (key != session_key))
      break;
    status=MagickFalse;
    switch (command)
    {
      case 'c':
      {
        status=CreateDistributeCache(image_registry,client_socket,session_key);
        break;
      }
      case 'r':
      {
        status=ReadDistributeCache(image_registry,client_socket,session_key);
        break;
      }
      case 'u':
      {
        status=WriteDistributeCache(image_registry,client_socket,session_key);
        break;
      }
      case 'd':
      {
        status=DestroyDistributeCache(image_registry,client_socket,session_key);
        break;
      }
      default:
        break;
    }
    count=write(client_socket,&status,sizeof(status));
    if (count != (ssize_t) sizeof(status))
      break;
  }
  return((void *) NULL);
}

MagickExport void DistributePixelCacheServer(const size_t port,
  ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_SOCKET) && defined(MAGICKCORE_THREAD_SUPPORT)
  int
    server_socket,
    status;

  pthread_attr_t
    attributes;

  pthread_t
    threads;

  struct sockaddr_in
    address;

  /*
    Launch distributed pixel cache server.
  */
  server_socket=socket(AF_INET,SOCK_STREAM,0);
  address.sin_family=AF_INET;
  address.sin_port=htons(port);
  address.sin_addr.s_addr=htonl(INADDR_ANY);
  status=bind(server_socket,(struct sockaddr *) &address,(socklen_t)
    sizeof(address));
  if (status != 0)
    ThrowFatalException(CacheFatalError,"UnableToBind");
  status=listen(server_socket,32);
  if (status != 0)
    ThrowFatalException(CacheFatalError,"UnableToListen");
  pthread_attr_init(&attributes);
  for ( ; ; )
  {
    int
      client_socket;

    socklen_t
      length;

    length=(socklen_t) sizeof(address);
    client_socket=accept(server_socket,(struct sockaddr *) &address,&length);
    if (client_socket == -1)
      ThrowFatalException(CacheFatalError,"UnableToEstablishConnection");
    status=pthread_create(&threads,&attributes,DistributePixelCacheClient,
      (void *) &client_socket);
    if (status == -1)
      ThrowFatalException(CacheFatalError,"UnableToCreateClientThread");
  }
  (void) close(server_socket);
#else
  ThrowFatalException(MissingDelegateError,"distributed pixel cache");
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   O p e n D i s t r i b u t e P i x e l C a c h e                           %
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
%        DistributeCacheInfo *distribute_cache_info,Image *image)
%
%  A description of each parameter follows:
%
%    o distribute_cache_info: the distributed cache info.
%
%    o image: the image.
%
*/
MagickPrivate MagickBooleanType OpenDistributePixelCache(
  DistributeCacheInfo *distribute_cache_info,Image *image)
{
  int
    file;

  MagickBooleanType
    status;

  MagickSizeType
    session_key;

  ssize_t
    count;

  assert(distribute_cache_info != (DistributeCacheInfo *) NULL);
  assert(distribute_cache_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  file=distribute_cache_info->file;
  session_key=distribute_cache_info->session_key;
  count=write(file,"c",1);
  if (count != 1)
    return(MagickFalse);
  count=write(file,&session_key,sizeof(session_key));
  if (count != (ssize_t) sizeof(session_key))
    return(MagickFalse);
  count=write(file,&image->columns,sizeof(image->columns));
  if (count != (ssize_t) sizeof(image->columns))
    return(MagickFalse);
  count=write(file,&image->rows,sizeof(image->rows));
  if (count != (ssize_t) sizeof(image->rows))
    return(MagickFalse);
  count=write(file,&image->number_channels,sizeof(image->number_channels));
  if (count != (ssize_t) sizeof(image->number_channels))
    return(MagickFalse);
  count=read(file,&status,sizeof(status));
  if (count != (ssize_t) sizeof(status))
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D i s t r i b u t e P i x e l C a c h e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDistributePixelCache() reads pixels from the specified region of the
%  distributed pixel cache.
%
%  The format of the ReadDistributePixelCache method is:
%
%      MagickBooleanType *ReadDistributePixelCache(
%        DistributeCacheInfo *distribute_cache_info,const RectangleInfo *region,
%        const MagickSizeType length,Quantum *pixels)
%
%  A description of each parameter follows:
%
%    o distribute_cache_info: the distributed cache info.
%
%    o image: the image.
%
%    o region: read the pixels from this region of the image.
%
%    o length: write the pixels to this region of the image.
%
%    o pixels: read these pixels from the pixel cache.
%
*/
MagickPrivate MagickBooleanType ReadDistributePixelCache(
  DistributeCacheInfo *distribute_cache_info,const RectangleInfo *region,
  const MagickSizeType length,Quantum *pixels)
{
  int
    file;

  MagickBooleanType
    status;

  MagickSizeType
    session_key;

  ssize_t
    count;

  assert(distribute_cache_info != (DistributeCacheInfo *) NULL);
  assert(distribute_cache_info->signature == MagickSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(pixels != (Quantum *) NULL);
  file=distribute_cache_info->file;
  session_key=distribute_cache_info->session_key;
  count=write(file,"r",1);
  if (count != 1)
    return(MagickFalse);
  count=write(file,&session_key,sizeof(session_key));
  if (count != (ssize_t) sizeof(session_key))
    return(MagickFalse);
  count=write(file,&region->width,sizeof(region->width));
  if (count != (ssize_t) sizeof(region->width))
    return(MagickFalse);
  count=write(file,&region->height,sizeof(region->height));
  if (count != (ssize_t) sizeof(region->height))
    return(MagickFalse);
  count=write(file,&region->x,sizeof(region->x));
  if (count != (ssize_t) sizeof(region->x))
    return(MagickFalse);
  count=write(file,&region->y,sizeof(region->y));
  if (count != (ssize_t) sizeof(region->y))
    return(MagickFalse);
  if (length != ((size_t) length))
    return(MagickFalse);
  count=write(file,&length,sizeof(length));
  if (count != (ssize_t) sizeof(length))
    return(MagickFalse);
  count=read(file,(unsigned char *) pixels,(size_t) length);
  if (count != (ssize_t) length)
    return(MagickFalse);
  count=read(file,&status,sizeof(status));
  if (count != (ssize_t) sizeof(status))
    return(MagickFalse);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h D i s t r i b u t e P i x e l C a c h e               %
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
%      void RelinquishDistributePixelCache(
%        DistributeCacheInfo *distribute_cache_info)
%
%  A description of each parameter follows:
%
%    o distribute_cache_info: the distributed cache info.
%
*/
MagickPrivate void RelinquishDistributePixelCache(
  DistributeCacheInfo *distribute_cache_info)
{
  int
    file;

  MagickSizeType
    session_key;

  assert(distribute_cache_info != (DistributeCacheInfo *) NULL);
  assert(distribute_cache_info->signature == MagickSignature);
  file=distribute_cache_info->file;
  session_key=distribute_cache_info->session_key;
  (void) write(file,"c",1);
  (void) write(file,&session_key,sizeof(session_key));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e D i s t r i b u t e P i x e l C a c h e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteDistributePixelCache() writes image pixels to the specified region of
%  the distributed pixel cache.
%
%
%  The format of the WriteDistributePixelCache method is:
%
%      MagickBooleanType *WriteDistributePixelCache(
%        DistributeCacheInfo *distribute_cache_info,const RectangleInfo *region,
%        const MagickSizeType length,const Quantum *pixels)
%
%  A description of each parameter follows:
%
%    o distribute_cache_info: the distributed cache info.
%
%    o image: the image.
%
%    o region: write the pixels to this region of the image.
%
%    o length: write the pixels to this region of the image.
%
%    o pixels: write these pixels to the pixel cache.
%
*/
MagickPrivate MagickBooleanType WriteDistributePixelCache(
  DistributeCacheInfo *distribute_cache_info,const RectangleInfo *region,
  const MagickSizeType length,const Quantum *pixels)
{
  int
    file;

  MagickBooleanType
    status;

  MagickSizeType
    session_key;

  ssize_t
    count;

  assert(distribute_cache_info != (DistributeCacheInfo *) NULL);
  assert(distribute_cache_info->signature == MagickSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(pixels != (Quantum *) NULL);
  file=distribute_cache_info->file;
  session_key=distribute_cache_info->session_key;
  count=write(file,"u",1);
  if (count != 1)
    return(MagickFalse);
  count=write(file,&session_key,sizeof(session_key));
  if (count != (ssize_t) sizeof(session_key))
    return(MagickFalse);
  count=write(file,&region->width,sizeof(region->width));
  if (count != (ssize_t) sizeof(region->width))
    return(MagickFalse);
  count=write(file,&region->height,sizeof(region->height));
  if (count != (ssize_t) sizeof(region->height))
    return(MagickFalse);
  count=write(file,&region->x,sizeof(region->x));
  if (count != (ssize_t) sizeof(region->x))
    return(MagickFalse);
  count=write(file,&region->y,sizeof(region->y));
  if (count != (ssize_t) sizeof(region->y))
    return(MagickFalse);
  if (length != ((size_t) length))
    return(MagickFalse);
  count=write(file,&length,sizeof(length));
  if (count != (ssize_t) sizeof(length))
    return(MagickFalse);
  count=write(file,(unsigned char *) pixels,(size_t) length);
  if (count != (ssize_t) length)
    return(MagickFalse);
  count=read(file,&status,sizeof(status));
  if (count != (ssize_t) sizeof(status))
    return(MagickFalse);
  return(status != 0 ? MagickTrue : MagickFalse);
}

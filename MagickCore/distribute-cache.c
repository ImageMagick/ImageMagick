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
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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
#include "MagickCore/nt-base-private.h"
#include "MagickCore/pixel.h"
#include "MagickCore/policy.h"
#include "MagickCore/random_.h"
#include "MagickCore/registry.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility-private.h"
#include "MagickCore/version.h"
#include "MagickCore/version-private.h"
#define SOCKET_TYPE int
#undef MAGICKCORE_HAVE_DISTRIBUTE_CACHE
#if defined(MAGICKCORE_DPC_SUPPORT)
#if defined(MAGICKCORE_HAVE_SOCKET) && defined(MAGICKCORE_THREAD_SUPPORT)
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define CLOSE_SOCKET(socket) (void) close_utf8(socket)
#define HANDLER_RETURN_TYPE void *
#define HANDLER_RETURN_VALUE (void *) NULL
#define SOCKET_TYPE int
#define LENGTH_TYPE size_t
#define MAGICKCORE_HAVE_DISTRIBUTE_CACHE 1
#elif defined(_MSC_VER)
#define CLOSE_SOCKET(socket) (void) closesocket(socket)
#define HANDLER_RETURN_TYPE DWORD WINAPI
#define HANDLER_RETURN_VALUE 0
#define LENGTH_TYPE int
#define MAGICKCORE_HAVE_DISTRIBUTE_CACHE 1
#define MAGICKCORE_HAVE_WINSOCK2 1
#endif
#endif

/*
  Define declarations.
*/
#define DPCHostname  "127.0.0.1"
#define DPCMacLength  32
#define DPCMaxClientWorkers  128
#define DPCMaxUnauthenticatedClientWorkers  16
#define DPCPendingConnections  10
#define DPCPort  6668
#define DPCScratchExtent  16384  /* bytes; send-side encrypt buffer */
#define DPCSessionKeyLength  16
#ifndef MSG_NOSIGNAL
#  define MSG_NOSIGNAL 0
#endif

/*
  Typedef declarations.
*/
typedef struct
{
  uint32_t
    state[8];

  uint64_t
    count;

  unsigned char
    buffer[64];
} SHA256Hash;

/*
  Static declarations.
*/
#ifdef MAGICKCORE_HAVE_WINSOCK2
static SemaphoreInfo
  *winsock_semaphore = (SemaphoreInfo *) NULL;

static WSADATA
  *wsaData = (WSADATA*) NULL;
#endif

#if defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
static SemaphoreInfo
  *dpc_semaphore = (SemaphoreInfo *) NULL;

static char
  *dpc_shared_secret = (char *) NULL;

static size_t
  dpc_clients = 0,
  dpc_unauthenticated_clients = 0;
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

#if !defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
static inline MagickOffsetType dpc_receive(SOCKET_TYPE magick_unused(file),
  const MagickSizeType magick_unused(length),
  unsigned char *magick_restrict magick_unused(message))
{
  magick_unreferenced(file);
  magick_unreferenced(length);
  magick_unreferenced(message);
  return(-1);
}
#else
static void SetSocketNoDelay(SOCKET_TYPE file)
{
  int
    one = 1;

  /*
    Disable Nagle.
  */
  (void) setsockopt(file,IPPROTO_TCP,TCP_NODELAY,(const char *) &one,
    (socklen_t) sizeof(one));
}

static void SetSocketReceiveTimeout(SOCKET_TYPE file,double seconds)
{
  /*
    Set a receive timeout.
  */
#if defined(MAGICKCORE_HAVE_WINSOCK2)
  {
    DWORD timeout = (DWORD) (seconds*1000.0);
    (void) setsockopt(file,SOL_SOCKET,SO_RCVTIMEO,
      (const char *) &timeout,sizeof(timeout));
  }
#else
  {
    struct timeval tv;
    tv.tv_sec=(int) seconds;
    tv.tv_usec=(int) ((seconds-(double) tv.tv_sec)*1000000.0);
    (void) setsockopt(file,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
  }
#endif
}

static inline MagickOffsetType dpc_receive(SOCKET_TYPE file,
  const MagickSizeType length,unsigned char *magick_restrict message)
{
  MagickOffsetType offset = 0;
  while (offset < (MagickOffsetType) length)
  {
    MagickSizeType chunk = MagickMin(length-offset,(MagickSizeType)
      MagickMaxBufferExtent);
    ssize_t count = recv(file,(char *) message+offset,(LENGTH_TYPE) chunk,0);
    if (count > 0)
      offset+=(MagickOffsetType) count;
    else
      if (count == 0)
        return(0);
      else
        {
#ifdef MAGICKCORE_HAVE_WINSOCK2
          int wsa_err = WSAGetLastError();
          if ((wsa_err == WSAEINTR) || (wsa_err == WSAEWOULDBLOCK))
            continue;
          if (wsa_err == WSAECONNRESET)
            return(-2);
#else
          if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
            continue;
          if (errno == ECONNRESET)
            return(-2);
#endif
          return(-1);
        }
  }
  return(offset);
}

static inline MagickOffsetType dpc_receive_deadline(SOCKET_TYPE file,
  MagickSizeType length,unsigned char *message,double deadline)
{
  /*
    Enforce an absolute deadline for authentication and command reads.
  */
  MagickOffsetType offset = 0;
  while (offset < (MagickOffsetType) length)
  {
    double
      now = (double) time((time_t *) NULL),
      remaining = deadline-now;

    MagickSizeType
      chunk;

    ssize_t
      count;

    if (now >= deadline)
      return(-1);
    if (remaining < 0.001)
      remaining=0.001;
    SetSocketReceiveTimeout(file,remaining);
    chunk=MagickMin(length-offset,(MagickSizeType) MagickMaxBufferExtent);
    count=recv(file,(char *) message+offset,(LENGTH_TYPE) chunk,0);
    if (count > 0)
      offset+=(MagickOffsetType) count;
    else
      if (count == 0)
        return(0);
      else
        {
#ifdef MAGICKCORE_HAVE_WINSOCK2
          int wsa_err = WSAGetLastError();
          if ((wsa_err == WSAEINTR) || (wsa_err == WSAEWOULDBLOCK))
            continue;
          if (wsa_err == WSAECONNRESET)
            return(-2);
          return(-1);
#else
          if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
            continue;
          if (errno == ECONNRESET)
            return(-2);
          return(-1);
#endif
        }
  }
  return(offset);
}
#endif

#if defined(MAGICKCORE_HAVE_WINSOCK2)
static void InitializeWinsock2(MagickBooleanType use_lock)
{
  if (use_lock != MagickFalse)
    {
      if (winsock_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&winsock_semaphore);
      LockSemaphoreInfo(winsock_semaphore);
    }
  if (wsaData == (WSADATA *) NULL)
    {
      wsaData=(WSADATA *) AcquireMagickMemory(sizeof(WSADATA));
      if (WSAStartup(MAKEWORD(2,2),wsaData) != 0)
        ThrowFatalException(CacheFatalError,"WSAStartup failed");
    }
  if (use_lock != MagickFalse)
    UnlockSemaphoreInfo(winsock_semaphore);
}
#endif

#if !defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
static int ConnectPixelCacheServer(const char *magick_unused(hostname),
  const int magick_unused(port),uint64_t *magick_unused(session_key),
  ExceptionInfo *exception)
{
  magick_unreferenced(hostname);
  magick_unreferenced(port);
  magick_unreferenced(session_key);
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","distributed pixel cache");
  return(MagickFalse);
}
#else
static inline uint64_t ROTL(uint64_t x,int b)
{
  return((x << b) | (x >> (64-b)));
}

static inline uint64_t U8TO64_LE(const uint8_t *p)
{
  return(((uint64_t) p[0] << 0)  | ((uint64_t) p[1] << 8)  |
         ((uint64_t) p[2] << 16) | ((uint64_t) p[3] << 24) |
         ((uint64_t) p[4] << 32) | ((uint64_t) p[5] << 40) |
         ((uint64_t) p[6] << 48) | ((uint64_t) p[7] << 56));
}

static inline uint64_t SIPHash24(const uint8_t key[16],const uint8_t *message,
  const size_t length)
{
  const uint8_t
    *end = message+length-(length % 8);

  size_t
    i;

  uint64_t
    b = ((uint64_t) length) << 56,
    k0 = U8TO64_LE(key),
    k1 = U8TO64_LE(key+8),
    m,
    v0 = 0x736f6d6570736575ULL ^ k0,
    v1 = 0x646f72616e646f6dULL ^ k1,
    v2 = 0x6c7967656e657261ULL ^ k0,
    v3 = 0x7465646279746573ULL ^ k1;

  for ( ; message != end; message+=8)
  {
    m=U8TO64_LE(message);
    v3^=m;
    for (i=0; i < 2; i++)
    {
      v0+=v1; v1=ROTL(v1,13); v1^=v0; v0=ROTL(v0,32);
      v2+=v3; v3=ROTL(v3,16); v3^=v2;
      v0+=v3; v3=ROTL(v3,21); v3^=v0;
      v2+=v1; v1=ROTL(v1,17); v1^=v2; v2=ROTL(v2,32);
    }
    v0^=m;
  }
  switch (length & 0x07)
  {
    case 7: b|=((uint64_t) message[6]) << 48; magick_fallthrough;
    case 6: b|=((uint64_t) message[5]) << 40; magick_fallthrough;
    case 5: b|=((uint64_t) message[4]) << 32; magick_fallthrough;
    case 4: b|=((uint64_t) message[3]) << 24; magick_fallthrough;
    case 3: b|=((uint64_t) message[2]) << 16; magick_fallthrough;
    case 2: b|=((uint64_t) message[1]) << 8; magick_fallthrough;
    case 1: b|=((uint64_t) message[0]); magick_fallthrough;
    default: break;
  }
  v3^=b;
  for (i=0; i < 2; i++)
  {
    v0+=v1; v1=ROTL(v1,13); v1^=v0; v0=ROTL(v0,32);
    v2+=v3; v3=ROTL(v3,16); v3^=v2;
    v0+=v3; v3=ROTL(v3,21); v3^=v0;
    v2+=v1; v1=ROTL(v1,17); v1^=v2; v2=ROTL(v2,32);
  }
  v0^=b;
  v2^=0xff;
  for (i=0; i < 4; i++)
  {
    v0+=v1; v1=ROTL(v1,13); v1^=v0; v0=ROTL(v0,32);
    v2+=v3; v3=ROTL(v3,16); v3^=v2;
    v0+=v3; v3=ROTL(v3,21); v3^=v0;
    v2+=v1; v1=ROTL(v1,17); v1^=v2; v2=ROTL(v2,32);
  }
  return(v0^v1^v2^v3);
}

static inline void DeriveSIPKeyFromSecret(const char *shared_secret,
  uint8_t key[16])
{
  size_t
    i,
    length;

  uint64_t
    k0 = 0x0706050403020100ULL,
    k1 = 0x0f0e0d0c0b0a0908ULL;

  length=strlen(shared_secret);
  for (i=0; i < length; i++)
  {
    uint8_t
      b = (uint8_t) shared_secret[i];

    k0^=b;
    k0*=0x100000001b3ULL;
    k1^=(uint64_t) b << ((i & 7)*8);
    k1=(k1 << 5) | (k1 >> (64-5));
  }
  (void) memcpy(key,&k0,8);
  (void) memcpy(key+8,&k1,8);
}

static inline uint64_t GenerateSessionKey(const char *shared_secret,
  const unsigned char *nonce,size_t length)
{
  uint8_t
    key[16];

  DeriveSIPKeyFromSecret(shared_secret,key);
  return(SIPHash24(key,nonce,length));
}

static int ConnectPixelCacheServer(const char *hostname,const int port,
  uint64_t *session_key,ExceptionInfo *exception)
{
#if defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
  char
    *message,
    service[MagickPathExtent],
    *shared_secret;

  int
    status;

  SOCKET_TYPE
    client_socket;

  ssize_t
    count;

  struct addrinfo
    hints,
    *result;

  unsigned char
    nonce[DPCSessionKeyLength];

  /*
    Connect to distributed pixel cache server and get session key.
  */
  *session_key=0;
#if defined(MAGICKCORE_HAVE_WINSOCK2)
  InitializeWinsock2(MagickTrue);
#endif
  (void) memset(&hints,0,sizeof(hints));
  hints.ai_family=AF_INET;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_flags=AI_PASSIVE;
  (void) FormatLocaleString(service,MagickPathExtent,"%d",port);
  status=getaddrinfo(hostname,service,&hints,&result);
  if (status != 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s': %s",hostname,gai_strerror(status));
      return(-1);
    }
  client_socket=(SOCKET_TYPE) socket(result->ai_family,result->ai_socktype,
    result->ai_protocol);
  if (client_socket == -1)
    {
      freeaddrinfo(result);
      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s': %s",hostname,message);
      message=DestroyString(message);
      return(-1);
    }
  status=connect(client_socket,result->ai_addr,(socklen_t) result->ai_addrlen);
  freeaddrinfo(result);
  if (status == -1)
    {
      CLOSE_SOCKET(client_socket);
      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s': %s",hostname,message);
      message=DestroyString(message);
      return(-1);
    }
  SetSocketNoDelay(client_socket);
  /*
    Receive server nonce.
  */
  count=recv(client_socket,(char *) nonce,sizeof(nonce),0);
  if (count != (ssize_t) sizeof(nonce))
    {
      CLOSE_SOCKET(client_socket);
      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s': %s",hostname,message);
      message=DestroyString(message);
      return(-1);
    }
  /*
    Compute keyed hash(shared_secret,nonce).
  */
  shared_secret=GetPolicyValue("cache:shared-secret");
  if (shared_secret == (const char *) NULL)
    {
      CLOSE_SOCKET(client_socket);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s': shared secret required",hostname);
      return(-1);
    }
  *session_key=GenerateSessionKey(shared_secret,nonce,sizeof(nonce));
  shared_secret=DestroyString(shared_secret);
  /*
    Send keyed hash back to client.
  */
  count=send(client_socket,(char *) session_key,sizeof(*session_key),
    MSG_NOSIGNAL);
  if (count != (ssize_t) sizeof(*session_key))
    {
      CLOSE_SOCKET(client_socket);
      (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
        "DistributedPixelCache","'%s': authentication failed",hostname);
      return(-1);
    }
  return((int) client_socket);
#else
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","distributed pixel cache");
  return(-1);
#endif
}
#endif

static char *GetHostname(int *port,ExceptionInfo *exception)
{
  char
    *host,
    *hosts,
    **hostlist;

  int
    argc;

  ssize_t
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
  if ((hostlist == (char **) NULL) || ((argc-1) == 0))
    {
      *port=DPCPort;
      return(AcquireString(DPCHostname));
    }
  {
    size_t host_count = (size_t) argc-1;
    size_t index = (id++ % host_count)+1;
    hosts=AcquireString(hostlist[index]);
  }
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
    *port=(int) StringToLong(hostlist[2]);
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

  uint64_t
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
      server_info->debug=(GetLogEventMask() & CacheEvent) != 0 ? MagickTrue :
        MagickFalse;
    }
  hostname=DestroyString(hostname);
  return(server_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o m p u t e H M A C S H A 2 5 6 M a c                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ComputeHMACSHA256Mac() opens a pixel cache on a remote server.
%
%  The format of the ComputeHMACSHA256Mac method is:
%
%      int ComputeHMACSHA256Mac(const void *key,size_t key_length,
%        const void *message,size_t message_length,unsigned char *mac,
%        size_t mac_length)
%
%  A description of each parameter follows:
%
%    o key: the HMAC key.
%
%    o key_length: HMAC key length in bytes.
%
%    o message: the message.
%
%    o message_length: the message length in bytes.
%
%    o mac: the mac is computed and returned in this buffer.
%
%    o mac_length: the mac length in bytes .  mac must be at least this length.
%
*/

static void InitializeSHA256Hash(SHA256Hash *sha256_hash)
{
  sha256_hash->count=0;
  sha256_hash->state[0]=0x6a09e667;
  sha256_hash->state[1]=0xbb67ae85;
  sha256_hash->state[2]=0x3c6ef372;
  sha256_hash->state[3]=0xa54ff53a;
  sha256_hash->state[4]=0x510e527f;
  sha256_hash->state[5]=0x9b05688c;
  sha256_hash->state[6]=0x1f83d9ab;
  sha256_hash->state[7]=0x5be0cd19;
}

static void TransformSHA256Hash(SHA256Hash *sha256_hash,
  const unsigned char data[])
{
#define EP0(x)  (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x)  (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define CH(x,y,z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define ROTRIGHT(a,b)  (((a) >> (b)) | ((a) << (32-(b))))
#define SIG0(x)  (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x)  (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

  static const uint32_t K[64] =
  {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
  };

  uint32_t
    a,
    b,
    c,
    d,
    e,
    f,
    g,
    h,
    i,
    j,
    m[64];

  j=0;
  for (i=0; i < 16; i++, j+=4)
    m[i]=((uint32_t) data[j] << 24) | ((uint32_t) data[j+1] << 16) |
      ((uint32_t) data[j+2] << 8) | ((uint32_t) data[j+3]);
  for ( ; i < 64; i++)
    m[i]=SIG1(m[i-2])+m[i-7]+SIG0(m[i-15])+m[i-16];
  a=sha256_hash->state[0];
  b=sha256_hash->state[1];
  c=sha256_hash->state[2];
  d=sha256_hash->state[3];
  e=sha256_hash->state[4];
  f=sha256_hash->state[5];
  g=sha256_hash->state[6];
  h=sha256_hash->state[7];
  for (i=0; i < 64; i++)
  {
    uint32_t
      alpha = h+EP1(e)+CH(e,f,g)+K[i]+m[i],
      beta = EP0(a)+MAJ(a,b,c);

    h=g;
    g=f;
    f=e;
    e=d+alpha;
    d=c;
    c=b;
    b=a;
    a=alpha+beta;
  }
  sha256_hash->state[0]+=a;
  sha256_hash->state[1]+=b;
  sha256_hash->state[2]+=c;
  sha256_hash->state[3]+=d;
  sha256_hash->state[4]+=e;
  sha256_hash->state[5]+=f;
  sha256_hash->state[6]+=g;
  sha256_hash->state[7]+=h;
}

static void UpdateSHA256Hash(SHA256Hash *sha256_hash,const unsigned char *data,
  const size_t data_length)
{
  size_t
    i,
    index = (size_t) ((sha256_hash->count/8) % 64);

  sha256_hash->count+=(uint64_t) data_length*8;
  for (i=0; i < data_length; i++)
  {
    sha256_hash->buffer[index++]=data[i];
    if (index == 64)
      {
        TransformSHA256Hash(sha256_hash,sha256_hash->buffer);
        index=0;
      }
  }
}

static void FinalizeSHA256Hash(SHA256Hash *sha256_hash,unsigned char hash[32])
{
  size_t
     i,
     index = (size_t) ((sha256_hash->count/8) % 64),
     pad_length = (index < 56) ? (56-index) : (120-index);

  uint64_t
    bit_count = sha256_hash->count;  /* message length before padding */

  unsigned char
    pad[64];

  pad[0]=0x80;
  (void) memset(pad+1,0,pad_length-1);
  UpdateSHA256Hash(sha256_hash,pad,pad_length);
  for (i=0; i < 8; i++)
    pad[i]=(unsigned char) (bit_count >> ((7-i)*8));
  UpdateSHA256Hash(sha256_hash, pad, 8);
  for (i=0; i < 32; i++)
    hash[i]=(unsigned char) (sha256_hash->state[i/4] >> ((3-(i % 4))*8));
}

static void ComputeSHA256Hash(const unsigned char *data,size_t length,
  unsigned char hash[32])
{
  SHA256Hash
    sha256_hash;

  /*
    Hash a buffer in one shot.
  */
  InitializeSHA256Hash(&sha256_hash);
  UpdateSHA256Hash(&sha256_hash,data,length);
  FinalizeSHA256Hash(&sha256_hash,hash);
}

static int ComputeHMACSHA256Mac(const void *key,size_t key_length,
  const void *message,size_t message_length,unsigned char *mac,
  size_t mac_length)
{
  SHA256Hash
    sha256_hash;

  size_t
    i;

  unsigned char
    full_hash[32],
    inner_hash[32],
    inner_pad[64],
    k_prime[64]={0},
    outer_pad[64];

  /*
    If key is longer than 64 bytes, hash it down first.
  */
  if ((mac == (unsigned char *) NULL)  || (mac_length == 0))
    return(-1);
  if (((key == (const void *) NULL) && (key_length != 0)) ||
      ((message == (const void *) NULL) && (message_length != 0)))
    return(-1);
  if (key_length > 64)
    ComputeSHA256Hash((unsigned char *) key,key_length,k_prime);
  else
    (void) memcpy(k_prime,key,key_length);
  /*
    Prepare inner pad and outer pad masks.
  */
  for (i=0; i < 64; i++)
  {
    inner_pad[i]=k_prime[i] ^ 0x36;
    outer_pad[i]=k_prime[i] ^ 0x5c;
  }
  /*
    Inner hash: SHA256(inner pad || message)
  */
  InitializeSHA256Hash(&sha256_hash);
  UpdateSHA256Hash(&sha256_hash,inner_pad,64);
  UpdateSHA256Hash(&sha256_hash,(unsigned char *) message,message_length);
  FinalizeSHA256Hash(&sha256_hash,inner_hash);
  /*
    Outer hash: SHA256(outer pad || inner hash)
  */
  InitializeSHA256Hash(&sha256_hash);
  UpdateSHA256Hash(&sha256_hash,outer_pad,64);
  UpdateSHA256Hash(&sha256_hash,inner_hash,32);
  FinalizeSHA256Hash(&sha256_hash,full_hash);
  /*
    Truncate or copy to destination buffer based on requested mac length.
  */
  (void) memcpy(mac,full_hash,(mac_length < 32) ? mac_length : 32);
  return(0);
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
#if defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
  if (server_info->file >= 0)
    CLOSE_SOCKET(server_info->file);
#endif
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

#if !defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
static inline MagickOffsetType dpc_send(SOCKET_TYPE magick_unused(file),
  const MagickSizeType magick_unused(length),
  const void *magick_restrict magick_unused(message))
{
  magick_unreferenced(file);
  magick_unreferenced(length);
  magick_unreferenced(message);
  return(-1);
}
#else
static inline MagickOffsetType dpc_send(SOCKET_TYPE file,
  const MagickSizeType length,const void *magick_restrict message)
{
  MagickOffsetType
    i;

  ssize_t
    count;

  /*
    Ensure a complete message is sent.
  */
  count=0;
  for (i=0; i < (MagickOffsetType) length; i+=count)
  {
    count=(ssize_t) send(file,(const char *) message+i,(LENGTH_TYPE) MagickMin(
      length-(MagickSizeType) i,(MagickSizeType) MagickMaxBufferExtent),
      MSG_NOSIGNAL);
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  return(i);
}
#endif

static const char *GetDPCSharedSecret(void)
{
#if defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
  /*
    The server loads the secret at start-up; a client loads it on first use.
  */
  if (dpc_shared_secret == (char *) NULL)
    {
      if (dpc_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&dpc_semaphore);
      LockSemaphoreInfo(dpc_semaphore);
      if (dpc_shared_secret == (char *) NULL)
        dpc_shared_secret=GetPolicyValue("cache:shared-secret");
      UnlockSemaphoreInfo(dpc_semaphore);
    }
  return(dpc_shared_secret);
#else
  return((const char *) NULL);
#endif
}

static int DeriveDPCKey(const uint64_t session_key,const char label,
  unsigned char key[DPCMacLength])
{
  const char
    *shared_secret;

  unsigned char
    info[sizeof(session_key)+1];

  /*
    key = HMAC-SHA-256(shared secret, session key || label).  The session key
    travels in the clear, so the shared secret is what keeps this private.
  */
  shared_secret=GetDPCSharedSecret();
  if (shared_secret == (const char *) NULL)
    return(-1);
  (void) memcpy(info,&session_key,sizeof(session_key));
  info[sizeof(session_key)]=(unsigned char) label;
  return(ComputeHMACSHA256Mac(shared_secret,strlen(shared_secret),info,
    sizeof(info),key,DPCMacLength));
}

static inline uint64_t LoadDPCWord(const unsigned char *p)
{
  return(((uint64_t) p[0]) | ((uint64_t) p[1] << 8) |
    ((uint64_t) p[2] << 16) | ((uint64_t) p[3] << 24) |
    ((uint64_t) p[4] << 32) | ((uint64_t) p[5] << 40) |
    ((uint64_t) p[6] << 48) | ((uint64_t) p[7] << 56));
}

static void SplitMix64Keystream(const unsigned char *stream_key,
  const unsigned char *mac,const MagickSizeType offset,unsigned char *data,
  const size_t length)
{
  size_t
    i;

  uint64_t
    block,
    seed,
    z;

  /*
    XOR data with the SplitMix64 keystream starting at byte 'offset'.
  */
  seed=LoadDPCWord(stream_key) ^ LoadDPCWord(mac);
  block=(uint64_t) (offset >> 3);
  z=0;
  for (i=0; i < length; i++)
  {
    size_t
      j = (size_t) ((offset+i) & 7);

    if ((i == 0) || (j == 0))
      {
        block=(uint64_t) ((offset+i) >> 3);
        z=seed+(block+1)*0x9e3779b97f4a7c15ULL;
        z=(z ^ (z >> 30))*0xbf58476d1ce4e5b9ULL;
        z=(z ^ (z >> 27))*0x94d049bb133111ebULL;
        z^=z >> 31;
      }
    data[i]^=(unsigned char) (z >> (j*8));
  }
}

static inline void WipeDPCKey(unsigned char *key,const size_t length)
{
  volatile unsigned char
    *p = key;

  size_t
    i;

  for (i=0; i < length; i++)
    p[i]=0;
}

static inline int mac_compare(const unsigned char *a,const unsigned char *b,
  const size_t length)
{
  size_t
    i;

  volatile unsigned char
    difference = 0;

  /*
    Compare in time independent of where (or whether) the buffers differ.
  */
  for (i=0; i < length; i++)
    difference|=(unsigned char) (a[i] ^ b[i]);
  return(difference == 0 ? 0 : -1);
}

static inline MagickOffsetType dpc_send_mac(SOCKET_TYPE file,
  const MagickSizeType length,const void *magick_restrict message,
  uint64_t session_key)
{
  const unsigned char
    *p = (const unsigned char *) message;

  MagickOffsetType
    status;

  MagickSizeType
    offset;

  unsigned char
    mac[DPCMacLength],
    mac_key[DPCMacLength],
    scratch[DPCScratchExtent],
    stream_key[DPCMacLength];

  /*
    MAC the plaintext, then send MAC || (plaintext XOR keystream).
  */
  if (length > (MagickSizeType) MAGICK_SSIZE_MAX)
    return(-1);
  if ((DeriveDPCKey(session_key,'m',mac_key) != 0) ||
      (DeriveDPCKey(session_key,'e',stream_key) != 0) ||
      (ComputeHMACSHA256Mac(mac_key,sizeof(mac_key),message,(size_t) length, mac,sizeof(mac)) != 0))
    {
      WipeDPCKey(mac_key,sizeof(mac_key));
      WipeDPCKey(stream_key,sizeof(stream_key));
      return(-1);  /* key derivation or HMAC computation failed */
    }
  WipeDPCKey(mac_key,sizeof(mac_key));
  status=dpc_send(file,sizeof(mac),mac);
  if (status != (MagickOffsetType) sizeof(mac))
    {
      WipeDPCKey(stream_key,sizeof(stream_key));
      return(-1);
    }
  /*
    Encrypt into a scratch buffer: the caller's buffer may be the live cache.
  */
  for (offset=0; offset < length; )
  {
    size_t
      chunk = (size_t) MagickMin(length-offset,(MagickSizeType)
        sizeof(scratch));

    (void) memcpy(scratch,p+offset,chunk);
    SplitMix64Keystream(stream_key,mac,offset,scratch,chunk);
    status=dpc_send(file,(MagickSizeType) chunk,scratch);
    if (status != (MagickOffsetType) chunk)
      break;
    offset+=(MagickSizeType) chunk;
  }
  WipeDPCKey(stream_key,sizeof(stream_key));
  WipeDPCKey(scratch,sizeof(scratch));
  if (offset != length)
    return(-1);
  return((MagickOffsetType) length);
}

static inline MagickOffsetType dpc_receive_mac(SOCKET_TYPE file,
  const MagickSizeType length,unsigned char *magick_restrict message,
  uint64_t session_key)
{
  MagickOffsetType
    count;

  unsigned char
    expected_mac[DPCMacLength],
    mac_key[DPCMacLength],
    received_mac[DPCMacLength],
    stream_key[DPCMacLength];

  int
    status;

  /*
    Read MAC || ciphertext, decrypt in place, then verify the MAC.
  */
  if (length > (MagickSizeType) MAGICK_SSIZE_MAX)
    return(-1);
  count=dpc_receive(file,sizeof(received_mac),received_mac);
  if (count != (MagickOffsetType) sizeof(received_mac))
    return(-1);
  count=dpc_receive(file,length,message);
  if (count != (MagickOffsetType) length)
    return(count);
  status=DeriveDPCKey(session_key,'e',stream_key);
  if (status == 0)
    {
      SplitMix64Keystream(stream_key,received_mac,0,message,(size_t) length);
      status=DeriveDPCKey(session_key,'m',mac_key);
    }
  if (status == 0)
    status=ComputeHMACSHA256Mac(mac_key,sizeof(mac_key),message,
      (size_t) length,expected_mac,sizeof(expected_mac));
  WipeDPCKey(mac_key,sizeof(mac_key));
  WipeDPCKey(stream_key,sizeof(stream_key));
  if (status != 0)
    return(-1);  /* key derivation or HMAC computation failed */
  if (mac_compare(received_mac,expected_mac,sizeof(expected_mac)) != 0)
    return(-1);  /* HMAC verification failed */
  return(count);
}

#if !defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
MagickExport void DistributePixelCacheServer(const int magick_unused(port),
  ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(port);
  magick_unreferenced(exception);
  ThrowFatalException(MissingDelegateError,"DelegateLibrarySupportNotBuiltIn");
}
#else
static MagickBooleanType DestroyDistributeCache(SplayTreeInfo *registry,
  const uint64_t session_key)
{
  MagickAddressType
    key = (MagickAddressType) session_key;

  /*
    Destroy distributed pixel cache.
  */
  return(DeleteNodeFromSplayTree(registry,(const void *) key));
}

static MagickBooleanType OpenDistributeCache(SplayTreeInfo *registry,
  SOCKET_TYPE file,const uint64_t session_key,ExceptionInfo *exception)
{
  Image
    *image;

  MagickAddressType
    key = (MagickAddressType) session_key;

  MagickBooleanType
    status;

  MagickOffsetType
    count;

  MagickSizeType
    length;

  unsigned char
    message[MagickPathExtent],
    *p;

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
  count=dpc_receive(file,length,message);
  if (count != (MagickOffsetType) length)
    {
      image=DestroyImage(image);
      return(MagickFalse);
    }
  /*
    Deserialize the image attributes.
  */
  p=message;
  (void) memcpy(&image->storage_class,p,sizeof(image->storage_class));
  p+=(ptrdiff_t) sizeof(image->storage_class);
  (void) memcpy(&image->colorspace,p,sizeof(image->colorspace));
  p+=(ptrdiff_t) sizeof(image->colorspace);
  (void) memcpy(&image->alpha_trait,p,sizeof(image->alpha_trait));
  p+=(ptrdiff_t) sizeof(image->alpha_trait);
  (void) memcpy(&image->channels,p,sizeof(image->channels));
  p+=(ptrdiff_t) sizeof(image->channels);
  (void) memcpy(&image->columns,p,sizeof(image->columns));
  p+=(ptrdiff_t) sizeof(image->columns);
  (void) memcpy(&image->rows,p,sizeof(image->rows));
  p+=(ptrdiff_t) sizeof(image->rows);
  (void) memcpy(&image->number_channels,p,sizeof(image->number_channels));
  p+=(ptrdiff_t) sizeof(image->number_channels);
  (void) memcpy(image->channel_map,p,MaxPixelChannels*
    sizeof(*image->channel_map));
  p+=(ptrdiff_t) MaxPixelChannels*sizeof(*image->channel_map);
  (void) memcpy(&image->metacontent_extent,p,sizeof(image->metacontent_extent));
  p+=(ptrdiff_t) sizeof(image->metacontent_extent);
  if (SyncImagePixelCache(image,exception) == MagickFalse)
    {
      image=DestroyImage(image);
      return(MagickFalse);
    }
  status=AddValueToSplayTree(registry,(const void *) key,image);
  if (status == MagickFalse)
    {
      image=DestroyImage(image);
      return(MagickFalse);
    }
  return(status);
}

static inline MagickBooleanType ValidateDistributedPixelCache(
  const RectangleInfo *region,const size_t per_pixel,
  const MagickSizeType length)
{
  size_t
    extent = 0,
    pixels = 0;

  if (HeapOverflowSanityCheckGetSize(region->width,region->height,&pixels) != MagickFalse)
    return(MagickFalse);
  if (HeapOverflowSanityCheckGetSize(pixels,per_pixel,&extent) != MagickFalse)
    return(MagickFalse);
  if (length > (MagickSizeType) extent)
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ReadDistributeCacheMetacontent(SplayTreeInfo *registry,
  SOCKET_TYPE file,const uint64_t session_key,ExceptionInfo *exception)
{
  const Quantum
    *p;

  const unsigned char
    *metacontent;

  Image
    *image;

  MagickAddressType
    key = (MagickAddressType) session_key;

  MagickOffsetType
    count;

  MagickSizeType
    length;

  RectangleInfo
    region;

  size_t
    per_pixel;

  unsigned char
    message[MagickPathExtent],
    *q;

  /*
    Read distributed pixel cache metacontent.
  */
  image=(Image *) GetValueFromSplayTree(registry,(const void *) key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=dpc_receive(file,length,message);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  q=message;
  (void) memcpy(&region.width,q,sizeof(region.width));
  q+=(ptrdiff_t) sizeof(region.width);
  (void) memcpy(&region.height,q,sizeof(region.height));
  q+=(ptrdiff_t) sizeof(region.height);
  (void) memcpy(&region.x,q,sizeof(region.x));
  q+=(ptrdiff_t) sizeof(region.x);
  (void) memcpy(&region.y,q,sizeof(region.y));
  q+=(ptrdiff_t) sizeof(region.y);
  (void) memcpy(&length,q,sizeof(length));
  q+=(ptrdiff_t) sizeof(length);
  per_pixel=image->number_meta_channels*sizeof(Quantum);
  if (ValidateDistributedPixelCache(&region,per_pixel,length) == MagickFalse)
    return(MagickFalse);
  p=GetVirtualPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  metacontent=(const unsigned char *) GetVirtualMetacontent(image);
  count=dpc_send_mac(file,length,metacontent,session_key);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  return(MagickTrue);
}

static MagickBooleanType ReadDistributeCachePixels(SplayTreeInfo *registry,
  SOCKET_TYPE file,const uint64_t session_key,ExceptionInfo *exception)
{
  const Quantum
    *p;

  Image
    *image;

  MagickAddressType
    key = (MagickAddressType) session_key;

  MagickOffsetType
    count;

  MagickSizeType
    length;

  RectangleInfo
    region;

  size_t
    per_pixel;

  unsigned char
    message[MagickPathExtent],
    *q;

  /*
    Read distributed pixel cache pixels.
  */
  image=(Image *) GetValueFromSplayTree(registry,(const void *) key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=dpc_receive(file,length,message);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  q=message;
  (void) memcpy(&region.width,q,sizeof(region.width));
  q+=(ptrdiff_t) sizeof(region.width);
  (void) memcpy(&region.height,q,sizeof(region.height));
  q+=(ptrdiff_t) sizeof(region.height);
  (void) memcpy(&region.x,q,sizeof(region.x));
  q+=(ptrdiff_t) sizeof(region.x);
  (void) memcpy(&region.y,q,sizeof(region.y));
  q+=(ptrdiff_t) sizeof(region.y);
  (void) memcpy(&length,q,sizeof(length));
  per_pixel=image->number_channels*sizeof(Quantum);
  if (ValidateDistributedPixelCache(&region,per_pixel,length) == MagickFalse)
    return(MagickFalse);
  q+=(ptrdiff_t) sizeof(length);
  p=GetVirtualPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (p == (const Quantum *) NULL)
    return(MagickFalse);
  count=dpc_send_mac(file,length,p,session_key);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  return(MagickTrue);
}

static void *RelinquishImageRegistry(void *image)
{
  return((void *) DestroyImageList((Image *) image));
}

static MagickBooleanType WriteDistributeCacheMetacontent(
  SplayTreeInfo *registry,SOCKET_TYPE file,const uint64_t session_key,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickAddressType
    key = (MagickAddressType) session_key;

  MagickOffsetType
    count;

  MagickSizeType
    length;

  Quantum
    *q;

  RectangleInfo
    region;

  size_t
    per_pixel;

  unsigned char
    message[MagickPathExtent],
    *metacontent,
    *p;

  /*
    Write distributed pixel cache metacontent.
  */
  image=(Image *) GetValueFromSplayTree(registry,(const void *) key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=dpc_receive(file,length,message);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  p=message;
  (void) memcpy(&region.width,p,sizeof(region.width));
  p+=(ptrdiff_t) sizeof(region.width);
  (void) memcpy(&region.height,p,sizeof(region.height));
  p+=(ptrdiff_t) sizeof(region.height);
  (void) memcpy(&region.x,p,sizeof(region.x));
  p+=(ptrdiff_t) sizeof(region.x);
  (void) memcpy(&region.y,p,sizeof(region.y));
  p+=(ptrdiff_t) sizeof(region.y);
  (void) memcpy(&length,p,sizeof(length));
  per_pixel=image->number_meta_channels*sizeof(Quantum);
  if (ValidateDistributedPixelCache(&region,per_pixel,length) == MagickFalse)
    return(MagickFalse);
  p+=(ptrdiff_t) sizeof(length);
  q=GetAuthenticPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  metacontent=(unsigned char *) GetAuthenticMetacontent(image);
  count=dpc_receive_mac(file,length,metacontent,session_key);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  return(SyncAuthenticPixels(image,exception));
}

static MagickBooleanType WriteDistributeCachePixels(SplayTreeInfo *registry,
  SOCKET_TYPE file,const uint64_t session_key,ExceptionInfo *exception)
{
  Image
    *image;

  MagickAddressType
    key = (MagickAddressType) session_key;

  MagickOffsetType
    count;

  MagickSizeType
    length;

  Quantum
    *q;

  RectangleInfo
    region;

  size_t
    per_pixel;

  unsigned char
    message[MagickPathExtent],
    *p;

  /*
    Write distributed pixel cache pixels.
  */
  image=(Image *) GetValueFromSplayTree(registry,(const void *) key);
  if (image == (Image *) NULL)
    return(MagickFalse);
  length=sizeof(region.width)+sizeof(region.height)+sizeof(region.x)+
    sizeof(region.y)+sizeof(length);
  count=dpc_receive(file,length,message);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  p=message;
  (void) memcpy(&region.width,p,sizeof(region.width));
  p+=(ptrdiff_t) sizeof(region.width);
  (void) memcpy(&region.height,p,sizeof(region.height));
  p+=(ptrdiff_t) sizeof(region.height);
  (void) memcpy(&region.x,p,sizeof(region.x));
  p+=(ptrdiff_t) sizeof(region.x);
  (void) memcpy(&region.y,p,sizeof(region.y));
  p+=(ptrdiff_t) sizeof(region.y);
  (void) memcpy(&length,p,sizeof(length));
  per_pixel=image->number_channels*sizeof(Quantum);
  if (ValidateDistributedPixelCache(&region,per_pixel,length) == MagickFalse)
    return(MagickFalse);
  p+=(ptrdiff_t) sizeof(length);
  q=GetAuthenticPixels(image,region.x,region.y,region.width,region.height,
    exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  count=dpc_receive_mac(file,length,(unsigned char *) q,session_key);
  if (count != (MagickOffsetType) length)
    return(MagickFalse);
  return(SyncAuthenticPixels(image,exception));
}

static void LockDPCSemaphore(void)
{
  if (dpc_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&dpc_semaphore);
  LockSemaphoreInfo(dpc_semaphore);
}

static void AuthenticateDPCClient(void)
{
  LockDPCSemaphore();
  if (dpc_unauthenticated_clients != 0)
    dpc_unauthenticated_clients--;
  UnlockSemaphoreInfo(dpc_semaphore);
}

static void RelinquishDPCClient(const MagickBooleanType unauthenticated)
{
  LockDPCSemaphore();
  if ((unauthenticated != MagickFalse) && (dpc_unauthenticated_clients != 0))
    dpc_unauthenticated_clients--;
  if (dpc_clients != 0)
    dpc_clients--;
  UnlockSemaphoreInfo(dpc_semaphore);
}

static HANDLER_RETURN_TYPE DistributePixelCacheConnectionHandler(
  void *socket_arg)
{
  ExceptionInfo
    *exception;

  MagickBooleanType
    authenticated = MagickFalse,
    status = MagickFalse;

  MagickOffsetType
    count;

  RandomInfo
    *random_info;

  SOCKET_TYPE
    client_socket,
    *client_socket_ptr = (SOCKET_TYPE *) socket_arg;

  SplayTreeInfo
    *registry;

  StringInfo
    *entropy;

  uint64_t
    key,
    session_key;

  unsigned char
    command,
    nonce[DPCSessionKeyLength];

  client_socket=(*client_socket_ptr);
  client_socket_ptr=(SOCKET_TYPE *) RelinquishMagickMemory(client_socket_ptr);
  /*
    Generate random nonce.
  */
  random_info=AcquireRandomInfo();
  entropy=GetRandomKey(random_info,sizeof(nonce));
  (void) memcpy(nonce,GetStringInfoDatum(entropy),sizeof(nonce));
  entropy=DestroyStringInfo(entropy);
  random_info=DestroyRandomInfo(random_info);
  /*
    Derive session key.
  */
  session_key=GenerateSessionKey(dpc_shared_secret,nonce,sizeof(nonce));
  /*
    Send nonce to client.
  */
  count=dpc_send(client_socket,sizeof(nonce),nonce);
  if (count != (MagickOffsetType) sizeof(nonce))
    {
      CLOSE_SOCKET(client_socket);
      RelinquishDPCClient(MagickTrue);
      return(HANDLER_RETURN_VALUE);
    }
  /*
    Receive client's keyed hash.
  */
  count=dpc_receive_deadline(client_socket,sizeof(key),(unsigned char *) &key,
    (double) time((time_t *) NULL)+5.0);
  if ((count != (MagickOffsetType) sizeof(key)) || (key != session_key))
    {
      CLOSE_SOCKET(client_socket);
      RelinquishDPCClient(MagickTrue);
      return(HANDLER_RETURN_VALUE);
    }
  AuthenticateDPCClient();
  authenticated=MagickTrue;
  exception=AcquireExceptionInfo();
  registry=NewSplayTree((int (*)(const void *,const void *)) NULL,
    (void *(*)(void *)) NULL,RelinquishImageRegistry);
  /*
    Command loop.
  */
  for (status=MagickFalse; ; )
  {
    /*
      Each command must echo the authenticated session key.
    */
    count=dpc_receive_deadline(client_socket,1,(unsigned char *) &command,
      (double) time((time_t *) NULL)+5.0);
    if (count <= 0)
      break;
    count=dpc_receive_deadline(client_socket,sizeof(key),(unsigned char *) &key,
      (double) time((time_t *) NULL)+5.0);
    if ((count != (MagickOffsetType) sizeof(key)) || (key != session_key))
      break;
    switch (command)
    {
      case 'o':
      {
        status=OpenDistributeCache(registry,client_socket,session_key,
          exception);
        dpc_send(client_socket,sizeof(status),&status);
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
    if ((status == MagickFalse) || (command == 'd'))
      break;
  }
  count=dpc_send(client_socket,sizeof(status),&status);
  CLOSE_SOCKET(client_socket);
  exception=DestroyExceptionInfo(exception);
  registry=DestroySplayTree(registry);
  RelinquishDPCClient(authenticated == MagickFalse ? MagickTrue :
    MagickFalse);
  return(HANDLER_RETURN_VALUE);
}

static size_t GetDPCPolicyLimit(const char *name,const size_t default_limit)
{
  char
    policy_name[MagickPathExtent],
    *policy_value;

  size_t
    limit;

  limit=default_limit;
  (void) FormatLocaleString(policy_name,MagickPathExtent,"cache:%s",name);
  policy_value=GetPolicyValue(policy_name);
  if (policy_value != (char *) NULL)
    {
      char
        *q;

      unsigned long
        policy_limit;

      errno=0;
      policy_limit=strtoul(policy_value,&q,10);
      if ((errno == 0) && (q != policy_value) && (policy_limit != 0))
        limit=(size_t) policy_limit;
      policy_value=DestroyString(policy_value);
    }
  return(limit);
}

static size_t GetMaxDPCClients(void)
{
  return(GetDPCPolicyLimit("max-dpc-clients",DPCMaxClientWorkers));
}

static size_t GetMaxDPCUnauthenticatedClients(void)
{
  size_t
    client_workers,
    unauthenticated_workers;

  client_workers=GetMaxDPCClients();
  unauthenticated_workers=GetDPCPolicyLimit("max-dpc-unauthenticated-clients",
    DPCMaxUnauthenticatedClientWorkers);
  return(MagickMin(unauthenticated_workers,client_workers));
}

static MagickBooleanType AcquireDPCClientWorker(void)
{
  MagickBooleanType
    status;

  size_t
    max_dpc_clients,
    unauthenticated_worker_limit;

  status=MagickFalse;
  max_dpc_clients=GetMaxDPCClients();
  unauthenticated_worker_limit=GetMaxDPCUnauthenticatedClients();
  LockDPCSemaphore();
  if ((dpc_clients < max_dpc_clients) &&
      (dpc_unauthenticated_clients < unauthenticated_worker_limit))
    {
      dpc_clients++;
      dpc_unauthenticated_clients++;
      status=MagickTrue;
    }
  UnlockSemaphoreInfo(dpc_semaphore);
  return(status);
}

MagickExport void DistributePixelCacheServer(const int port,
  ExceptionInfo *exception)
{
  char
    service[MagickPathExtent];

  int
    status;

#if defined(MAGICKCORE_THREAD_SUPPORT)
  pthread_attr_t
    attributes;

  pthread_t
    thread_id;
#elif defined(_MSC_VER)
  DWORD
    threadID;
#else
  Not implemented!
#endif

  SOCKET_TYPE
    server_socket;

  struct addrinfo
    *p;

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
  dpc_shared_secret=GetPolicyValue("cache:shared-secret");
  if (dpc_shared_secret == (char *) NULL)
    ThrowFatalException(CacheFatalError,"no shared secret");
#if defined(MAGICKCORE_HAVE_WINSOCK2)
  InitializeWinsock2(MagickFalse);
#endif
  (void) memset(&hint,0,sizeof(hint));
  hint.ai_family=AF_INET;
  hint.ai_socktype=SOCK_STREAM;
  hint.ai_flags=AI_PASSIVE;
  FormatLocaleString(service,MagickPathExtent,"%d",port);
  status=getaddrinfo(NULL,service,&hint,&result);
  if (status != 0)
    ThrowFatalException(CacheFatalError,"UnableToListen");
  server_socket=(SOCKET_TYPE) 0;
  for (p=result; p != NULL; p=p->ai_next)
  {
    int
      one = 1;

    server_socket=(SOCKET_TYPE) socket(p->ai_family,p->ai_socktype,
      p->ai_protocol);
    if (server_socket == -1)
      continue;
    status=setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,(char *) &one,
      (socklen_t) sizeof(one));
    if (status == -1)
      {
        CLOSE_SOCKET(server_socket);
        continue;
      }
    SetSocketReceiveTimeout(server_socket,5.0);
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
  pthread_attr_setdetachstate(&attributes,PTHREAD_CREATE_DETACHED);
#endif
  for ( ; ; )
  {
    SOCKET_TYPE
      *client_socket_ptr;

    socklen_t
      length = (socklen_t) sizeof(address);

    client_socket_ptr=(SOCKET_TYPE *) AcquireMagickMemory(sizeof(SOCKET_TYPE));
    if (client_socket_ptr == NULL)
      continue;  /* skip connection */
    *client_socket_ptr=(SOCKET_TYPE) accept(server_socket,(struct sockaddr *)
      &address,&length);
    if (*client_socket_ptr == -1)
      {
        client_socket_ptr=(SOCKET_TYPE *) RelinquishMagickMemory(
          client_socket_ptr);
        continue;
      }
    if (AcquireDPCClientWorker() == MagickFalse)
      {
        CLOSE_SOCKET(*client_socket_ptr);
        client_socket_ptr=(SOCKET_TYPE *) RelinquishMagickMemory(
          client_socket_ptr);
        continue;
      }
    SetSocketReceiveTimeout(*client_socket_ptr,5.0);
    SetSocketNoDelay(*client_socket_ptr);
    if (status == -1)
      {
        CLOSE_SOCKET(*client_socket_ptr);
        RelinquishDPCClient(MagickTrue);
        client_socket_ptr=(SOCKET_TYPE *) RelinquishMagickMemory(
          client_socket_ptr);
        continue;
      }
#if defined(MAGICKCORE_THREAD_SUPPORT)
    status=pthread_create(&thread_id,&attributes,
      DistributePixelCacheConnectionHandler,(void *) client_socket_ptr);
    if (status != 0)
      {
        CLOSE_SOCKET(*client_socket_ptr);
        RelinquishDPCClient(MagickTrue);
        RelinquishMagickMemory(client_socket_ptr);
        continue;
      }
#elif defined(_MSC_VER)
    if (CreateThread(0,0,DistributePixelCacheConnectionHandler,(void*) client_socket_ptr,0,&threadID) == (HANDLE) NULL)
      {
        CLOSE_SOCKET(*client_socket_ptr);
        RelinquishDPCClient(MagickTrue);
        RelinquishMagickMemory(client_socket_ptr);
        continue;
      }
#else
    Not implemented!
#endif
  }
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  D i s t r i b u t e C a c h e T e r m i n u s                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DistributeCacheTerminus() destroys the Distributed Cache.
%
*/
MagickPrivate void DistributeCacheTerminus(void)
{
#ifdef MAGICKCORE_HAVE_WINSOCK2
  if (winsock_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&winsock_semaphore);
  LockSemaphoreInfo(winsock_semaphore);
  if (wsaData != (WSADATA *) NULL)
    {
      WSACleanup();
      wsaData=(WSADATA *) RelinquishMagickMemory((void *) wsaData);
    }
  UnlockSemaphoreInfo(winsock_semaphore);
  RelinquishSemaphoreInfo(&winsock_semaphore);
#endif
#if defined(MAGICKCORE_HAVE_DISTRIBUTE_CACHE)
  if (dpc_shared_secret != (char *) NULL)
    dpc_shared_secret=DestroyString(dpc_shared_secret);
  if (dpc_semaphore != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&dpc_semaphore);
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
    status;

  MagickOffsetType
    count;

  unsigned char
    message[MagickPathExtent],
    *p;

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
  p+=(ptrdiff_t) sizeof(server_info->session_key);
  (void) memcpy(p,&image->storage_class,sizeof(image->storage_class));
  p+=(ptrdiff_t) sizeof(image->storage_class);
  (void) memcpy(p,&image->colorspace,sizeof(image->colorspace));
  p+=(ptrdiff_t) sizeof(image->colorspace);
  (void) memcpy(p,&image->alpha_trait,sizeof(image->alpha_trait));
  p+=(ptrdiff_t) sizeof(image->alpha_trait);
  (void) memcpy(p,&image->channels,sizeof(image->channels));
  p+=(ptrdiff_t) sizeof(image->channels);
  (void) memcpy(p,&image->columns,sizeof(image->columns));
  p+=(ptrdiff_t) sizeof(image->columns);
  (void) memcpy(p,&image->rows,sizeof(image->rows));
  p+=(ptrdiff_t) sizeof(image->rows);
  (void) memcpy(p,&image->number_channels,sizeof(image->number_channels));
  p+=(ptrdiff_t) sizeof(image->number_channels);
  (void) memcpy(p,image->channel_map,MaxPixelChannels*
    sizeof(*image->channel_map));
  p+=(ptrdiff_t) MaxPixelChannels*sizeof(*image->channel_map);
  (void) memcpy(p,&image->metacontent_extent,sizeof(image->metacontent_extent));
  p+=(ptrdiff_t) sizeof(image->metacontent_extent);
  count=dpc_send(server_info->file,(MagickSizeType) (p-message),message);
  if (count != (MagickOffsetType) (p-message))
    return(MagickFalse);
  status=MagickFalse;
  count=dpc_receive(server_info->file,sizeof(status),(unsigned char *) &status);
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

  unsigned char
    message[MagickPathExtent],
    *p;

  /*
    Read distributed pixel cache metacontent.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(metacontent != (unsigned char *) NULL);
  if (length > (MagickSizeType) MAGICK_SSIZE_MAX)
    return(-1);
  p=message;
  *p++='R';
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=(ptrdiff_t) sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=(ptrdiff_t) sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=(ptrdiff_t) sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=(ptrdiff_t) sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=(ptrdiff_t) sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=(ptrdiff_t) sizeof(length);
  count=dpc_send(server_info->file,(MagickSizeType) (p-message),message);
  if (count != (MagickOffsetType) (p-message))
    return(-1);
  return(dpc_receive_mac(server_info->file,length,metacontent,
    server_info->session_key));
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

  unsigned char
    message[MagickPathExtent],
    *p;

  /*
    Read distributed pixel cache pixels.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(pixels != (unsigned char *) NULL);
  if (length > (MagickSizeType) MAGICK_SSIZE_MAX)
    return(-1);
  p=message;
  *p++='r';
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=(ptrdiff_t) sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=(ptrdiff_t) sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=(ptrdiff_t) sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=(ptrdiff_t) sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=(ptrdiff_t) sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=(ptrdiff_t) sizeof(length);
  count=dpc_send(server_info->file,(MagickSizeType) (p-message),message);
  if (count != (MagickOffsetType) (p-message))
    return(-1);
  return(dpc_receive_mac(server_info->file,length,pixels,
    server_info->session_key));
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
    status;

  MagickOffsetType
    count;

  unsigned char
    message[MagickPathExtent],
    *p;

  /*
    Delete distributed pixel cache.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  p=message;
  *p++='d';
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=(ptrdiff_t) sizeof(server_info->session_key);
  count=dpc_send(server_info->file,(MagickSizeType) (p-message),message);
  if (count != (MagickOffsetType) (p-message))
    return(MagickFalse);
  status=MagickFalse;
  count=dpc_receive(server_info->file,sizeof(status),(unsigned char *) &status);
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

  unsigned char
    message[MagickPathExtent],
    *p;

  /*
    Write distributed pixel cache metacontent.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(metacontent != (unsigned char *) NULL);
  if (length > (MagickSizeType) MAGICK_SSIZE_MAX)
    return(-1);
  p=message;
  *p++='W';
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=(ptrdiff_t) sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=(ptrdiff_t) sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=(ptrdiff_t) sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=(ptrdiff_t) sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=(ptrdiff_t) sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=(ptrdiff_t) sizeof(length);
  count=dpc_send(server_info->file,(MagickSizeType) (p-message),message);
  if (count != (MagickOffsetType) (p-message))
    return(-1);
  return(dpc_send_mac(server_info->file,length,metacontent,
    server_info->session_key));
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

  unsigned char
    message[MagickPathExtent],
    *p;

  /*
    Write distributed pixel cache pixels.
  */
  assert(server_info != (DistributeCacheInfo *) NULL);
  assert(server_info->signature == MagickCoreSignature);
  assert(region != (RectangleInfo *) NULL);
  assert(pixels != (const unsigned char *) NULL);
  if (length > (MagickSizeType) MAGICK_SSIZE_MAX)
    return(-1);
  p=message;
  *p++='w';
  (void) memcpy(p,&server_info->session_key,sizeof(server_info->session_key));
  p+=(ptrdiff_t) sizeof(server_info->session_key);
  (void) memcpy(p,&region->width,sizeof(region->width));
  p+=(ptrdiff_t) sizeof(region->width);
  (void) memcpy(p,&region->height,sizeof(region->height));
  p+=(ptrdiff_t) sizeof(region->height);
  (void) memcpy(p,&region->x,sizeof(region->x));
  p+=(ptrdiff_t) sizeof(region->x);
  (void) memcpy(p,&region->y,sizeof(region->y));
  p+=(ptrdiff_t) sizeof(region->y);
  (void) memcpy(p,&length,sizeof(length));
  p+=(ptrdiff_t) sizeof(length);
  count=dpc_send(server_info->file,(MagickSizeType) (p-message),message);
  if (count != (MagickOffsetType) (p-message))
    return(-1);
  return(dpc_send_mac(server_info->file,length,pixels,
    server_info->session_key));
}

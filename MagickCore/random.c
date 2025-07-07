/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                 RRRR    AAA   N   N  DDDD    OOO   M   M                    %
%                 R   R  A   A  NN  N  D   D  O   O  MM MM                    %
%                 RRRR   AAAAA  N N N  D   D  O   O  M M M                    %
%                 R R    A   A  N  NN  D   D  O   O  M   M                    %
%                 R  R   A   A  N   N  DDDD    OOO   M   M                    %
%                                                                             %
%                                                                             %
%               MagickCore Methods to Generate Random Numbers                 %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                              December 2001                                  %
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
%  The generation of random numbers is too important to be left to chance.
%                               -- Tom Christiansen <tchrist@mox.perl.com>
%
%
*/

/*
  Include declarations.
*/
#if defined(__VMS)
#include <time.h>
#endif
#if defined(__MINGW32__)
#include <sys/time.h>
#endif
#include "MagickCore/studio.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/signature-private.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/timer-private.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"
#if defined(MAGICKCORE_HAVE_GETENTROPY)
#include <sys/random.h>
#endif
/*
  Define declarations.
*/
#define PseudoRandomHash  SHA256Hash
#define RandomEntropyLevel  9
#define RandomFilename  "reservoir.xdm"
#define RandomFiletype  "random"
#define RandomProtocolMajorVersion  1
#define RandomProtocolMinorVersion  0

/*
  Typedef declarations.
*/
struct _RandomInfo
{
  SignatureInfo
    *signature_info;

  StringInfo
    *nonce,
    *reservoir;

  size_t
    i;

  MagickSizeType
    seed[4];

  double
    normalize;

  unsigned long
    secret_key;

  unsigned short
    protocol_major,
    protocol_minor;

  SemaphoreInfo
    *semaphore;

  time_t
    timestamp;

  size_t
    signature;
};

/*
  External declarations.
*/
#if defined(__APPLE__) && !defined(TARGET_OS_IPHONE)
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#endif

#if !defined(MAGICKCORE_WINDOWS_SUPPORT)
extern char
  **environ;
#endif

/*
  Global declarations.
*/
static SemaphoreInfo
  *random_semaphore = (SemaphoreInfo *) NULL;

static unsigned long
  secret_key = ~0UL;

static MagickBooleanType
  gather_true_random = MagickFalse;

/*
  Forward declarations.
*/
static StringInfo
  *GenerateEntropicChaos(RandomInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e R a n d o m I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireRandomInfo() allocates the RandomInfo structure.
%
%  The format of the AcquireRandomInfo method is:
%
%      RandomInfo *AcquireRandomInfo(void)
%
*/
MagickExport RandomInfo *AcquireRandomInfo(void)
{
  const StringInfo
    *digest;

  RandomInfo
    *random_info;

  StringInfo
    *entropy,
    *key,
    *nonce;

  random_info=(RandomInfo *) AcquireCriticalMemory(sizeof(*random_info));
  (void) memset(random_info,0,sizeof(*random_info));
  random_info->signature_info=AcquireSignatureInfo();
  random_info->nonce=AcquireStringInfo(2*GetSignatureDigestsize(
    random_info->signature_info));
  ResetStringInfo(random_info->nonce);
  random_info->reservoir=AcquireStringInfo(GetSignatureDigestsize(
    random_info->signature_info));
  ResetStringInfo(random_info->reservoir);
  random_info->normalize=(double) (1.0/(MagickULLConstant(~0) >> 11));
  random_info->seed[0]=MagickULLConstant(0x76e15d3efefdcbbf);
  random_info->seed[1]=MagickULLConstant(0xc5004e441c522fb3);
  random_info->seed[2]=MagickULLConstant(0x77710069854ee241);
  random_info->seed[3]=MagickULLConstant(0x39109bb02acbe635);
  random_info->secret_key=secret_key;
  random_info->protocol_major=RandomProtocolMajorVersion;
  random_info->protocol_minor=RandomProtocolMinorVersion;
  random_info->semaphore=AcquireSemaphoreInfo();
  random_info->timestamp=GetMagickTime();
  random_info->signature=MagickCoreSignature;
  /*
    Seed random nonce.
  */
  nonce=GenerateEntropicChaos(random_info);
  if (nonce == (StringInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  InitializeSignature(random_info->signature_info);
  UpdateSignature(random_info->signature_info,nonce);
  FinalizeSignature(random_info->signature_info);
  SetStringInfoLength(nonce,(GetSignatureDigestsize(
    random_info->signature_info)+1)/2);
  SetStringInfo(nonce,GetSignatureDigest(random_info->signature_info));
  SetStringInfo(random_info->nonce,nonce);
  nonce=DestroyStringInfo(nonce);
  /*
    Seed random reservoir with entropic data.
  */
  entropy=GenerateEntropicChaos(random_info);
  if (entropy == (StringInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  UpdateSignature(random_info->signature_info,entropy);
  FinalizeSignature(random_info->signature_info);
  SetStringInfo(random_info->reservoir,GetSignatureDigest(
    random_info->signature_info));
  entropy=DestroyStringInfo(entropy);
  /*
    Seed pseudo random number generator.
  */
  if (random_info->secret_key == ~0UL)
    {
      key=GetRandomKey(random_info,sizeof(random_info->seed));
      (void) memcpy(random_info->seed,GetStringInfoDatum(key),
        sizeof(random_info->seed));
      key=DestroyStringInfo(key);
    }
  else
    {
      SignatureInfo
        *signature_info;

      signature_info=AcquireSignatureInfo();
      key=AcquireStringInfo(sizeof(random_info->secret_key));
      SetStringInfoDatum(key,(unsigned char *) &random_info->secret_key);
      UpdateSignature(signature_info,key);
      key=DestroyStringInfo(key);
      FinalizeSignature(signature_info);
      digest=GetSignatureDigest(signature_info);
      (void) memcpy(random_info->seed,GetStringInfoDatum(digest),
        MagickMin((size_t) GetSignatureDigestsize(signature_info),
        sizeof(random_info->seed)));
      signature_info=DestroySignatureInfo(signature_info);
    }
  return(random_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y R a n d o m I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyRandomInfo() deallocates memory associated with the random
%  reservoir.
%
%  The format of the DestroyRandomInfo method is:
%
%      RandomInfo *DestroyRandomInfo(RandomInfo *random_info)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
*/
MagickExport RandomInfo *DestroyRandomInfo(RandomInfo *random_info)
{
  assert(random_info != (RandomInfo *) NULL);
  assert(random_info->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  LockSemaphoreInfo(random_info->semaphore);
  if (random_info->reservoir != (StringInfo *) NULL)
    random_info->reservoir=DestroyStringInfo(random_info->reservoir);
  if (random_info->nonce != (StringInfo *) NULL)
    random_info->nonce=DestroyStringInfo(random_info->nonce);
  if (random_info->signature_info != (SignatureInfo *) NULL)
    random_info->signature_info=DestroySignatureInfo(
      random_info->signature_info);
  (void) memset(random_info->seed,0,sizeof(random_info->seed));
  random_info->signature=(~MagickCoreSignature);
  UnlockSemaphoreInfo(random_info->semaphore);
  RelinquishSemaphoreInfo(&random_info->semaphore);
  random_info=(RandomInfo *) RelinquishMagickMemory(random_info);
  return(random_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e n e r a t e E n t r o p i c C h a o s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GenerateEntropicChaos() generate entropic chaos used to initialize the
%  random reservoir.
%
%  The format of the GenerateEntropicChaos method is:
%
%      StringInfo *GenerateEntropicChaos(RandomInfo *random_info)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
*/

#if !defined(MAGICKCORE_WINDOWS_SUPPORT)
static ssize_t ReadRandom(int file,unsigned char *source,size_t length)
{
  unsigned char
    *q;

  ssize_t
    offset,
    count;

  offset=0;
  for (q=source; length != 0; length-=(size_t) count)
  {
    count=(ssize_t) read(file,q,length);
    if (count <= 0)
      {
        count=0;
        if (errno == EINTR)
          continue;
        return(-1);
      }
    q+=(ptrdiff_t) count;
    offset+=count;
  }
  return(offset);
}
#endif

static StringInfo *GenerateEntropicChaos(RandomInfo *random_info)
{
#define MaxEntropyExtent  64  /* max permitted: 256 */

  MagickThreadType
    tid;

  StringInfo
    *chaos,
    *entropy;

  ssize_t
    pid;

  time_t
    nanoseconds,
    seconds;

  /*
    Initialize random reservoir.
  */
  entropy=AcquireStringInfo(0);
  LockSemaphoreInfo(random_info->semaphore);
#if defined(MAGICKCORE_HAVE_GETENTROPY)
  {
    int
      status;
    
    SetStringInfoLength(entropy,MaxEntropyExtent);
    status=getentropy(GetStringInfoDatum(entropy),MaxEntropyExtent);
    if (status == 0)
      {
        UnlockSemaphoreInfo(random_info->semaphore);
        return(entropy);
      }
  }
#endif
  chaos=AcquireStringInfo(sizeof(unsigned char *));
  SetStringInfoDatum(chaos,(unsigned char *) &entropy);
  ConcatenateStringInfo(entropy,chaos);
  SetStringInfoDatum(chaos,(unsigned char *) entropy);
  ConcatenateStringInfo(entropy,chaos);
  pid=(ssize_t) getpid();
  SetStringInfoLength(chaos,sizeof(pid));
  SetStringInfoDatum(chaos,(unsigned char *) &pid);
  ConcatenateStringInfo(entropy,chaos);
  tid=GetMagickThreadId();
  SetStringInfoLength(chaos,sizeof(tid));
  SetStringInfoDatum(chaos,(unsigned char *) &tid);
  ConcatenateStringInfo(entropy,chaos);
#if defined(MAGICKCORE_HAVE_SYSCONF) && defined(_SC_PHYS_PAGES)
  {
    ssize_t
      pages;

    pages=(ssize_t) sysconf(_SC_PHYS_PAGES);
    SetStringInfoLength(chaos,sizeof(pages));
    SetStringInfoDatum(chaos,(unsigned char *) &pages);
    ConcatenateStringInfo(entropy,chaos);
  }
#endif
#if defined(MAGICKCORE_HAVE_GETRUSAGE) && defined(RUSAGE_SELF)
  {
    struct rusage
      usage;

    if (getrusage(RUSAGE_SELF,&usage) == 0)
      {
        SetStringInfoLength(chaos,sizeof(usage));
        SetStringInfoDatum(chaos,(unsigned char *) &usage);
      }
  }
#endif
  seconds=time((time_t *) 0);
  nanoseconds=0;
#if defined(MAGICKCORE_HAVE_GETTIMEOFDAY)
  {
    struct timeval
      timer;

    if (gettimeofday(&timer,(struct timezone *) NULL) == 0)
      {
        seconds=timer.tv_sec;
        nanoseconds=1000*timer.tv_usec;
      }
  }
#endif
#if defined(MAGICKCORE_HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME_HR)
  {
    struct timespec
      timer;

    if (clock_gettime(CLOCK_REALTIME_HR,&timer) == 0)
      {
        seconds=timer.tv_sec;
        nanoseconds=timer.tv_nsec;
      }
  }
#endif
  SetStringInfoLength(chaos,sizeof(seconds));
  SetStringInfoDatum(chaos,(unsigned char *) &seconds);
  ConcatenateStringInfo(entropy,chaos);
  SetStringInfoLength(chaos,sizeof(nanoseconds));
  SetStringInfoDatum(chaos,(unsigned char *) &nanoseconds);
  ConcatenateStringInfo(entropy,chaos);
  nanoseconds=0;
#if defined(MAGICKCORE_HAVE_CLOCK)
  nanoseconds=clock();
#endif
#if defined(MAGICKCORE_HAVE_TIMES)
  {
    struct tms
      timer;

    (void) times(&timer);
    nanoseconds=timer.tms_utime+timer.tms_stime;
  }
#endif
  SetStringInfoLength(chaos,sizeof(nanoseconds));
  SetStringInfoDatum(chaos,(unsigned char *) &nanoseconds);
  ConcatenateStringInfo(entropy,chaos);
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  {
    double
      nt_seconds;

    LARGE_INTEGER
      nt_nanoseconds;

    /*
      Not cryptographically strong but better than nothing.
    */
    nt_seconds=NTElapsedTime()+NTElapsedTime();
    SetStringInfoLength(chaos,sizeof(nt_seconds));
    SetStringInfoDatum(chaos,(unsigned char *) &nt_seconds);
    ConcatenateStringInfo(entropy,chaos);
    if (QueryPerformanceCounter(&nt_nanoseconds) != 0)
      {
        SetStringInfoLength(chaos,sizeof(nt_nanoseconds));
        SetStringInfoDatum(chaos,(unsigned char *) &nt_nanoseconds);
        ConcatenateStringInfo(entropy,chaos);
      }
    /*
      Our best hope for true entropy.
    */
    SetStringInfoLength(chaos,MaxEntropyExtent);
    (void) NTGatherRandomData(MaxEntropyExtent,GetStringInfoDatum(chaos));
    ConcatenateStringInfo(entropy,chaos);
  }
#else
  {
    char
      *filename;

    int
      file;

    ssize_t
      count;

    StringInfo
      *device;

    /*
      Not cryptographically strong but better than nothing.
    */
    if (environ != (char **) NULL)
      {
        ssize_t
          i;

        /*
          Squeeze some entropy from the sometimes unpredictable environment.
        */
        for (i=0; environ[i] != (char *) NULL; i++)
        {
          SetStringInfoLength(chaos,strlen(environ[i]));
          SetStringInfoDatum(chaos,(unsigned char *) environ[i]);
          ConcatenateStringInfo(entropy,chaos);
        }
      }
    filename=AcquireString("/dev/urandom");
    device=StringToStringInfo(filename);
    device=DestroyStringInfo(device);
    file=open_utf8(filename,O_RDONLY | O_BINARY,0);
    filename=DestroyString(filename);
    if (file != -1)
      {
        SetStringInfoLength(chaos,MaxEntropyExtent);
        count=ReadRandom(file,GetStringInfoDatum(chaos),MaxEntropyExtent);
        (void) close_utf8(file);
        SetStringInfoLength(chaos,(size_t) count);
        ConcatenateStringInfo(entropy,chaos);
      }
    if (gather_true_random != MagickFalse)
      {
        /*
          Our best hope for true entropy.
        */
        filename=AcquireString("/dev/random");
        device=StringToStringInfo(filename);
        device=DestroyStringInfo(device);
        file=open_utf8(filename,O_RDONLY | O_BINARY,0);
        filename=DestroyString(filename);
        if (file == -1)
          {
            filename=AcquireString("/dev/srandom");
            device=StringToStringInfo(filename);
            device=DestroyStringInfo(device);
            file=open_utf8(filename,O_RDONLY | O_BINARY,0);
          }
        if (file != -1)
          {
            SetStringInfoLength(chaos,MaxEntropyExtent);
            count=ReadRandom(file,GetStringInfoDatum(chaos),MaxEntropyExtent);
            (void) close_utf8(file);
            SetStringInfoLength(chaos,(size_t) count);
            ConcatenateStringInfo(entropy,chaos);
          }
      }
  }
#endif
  chaos=DestroyStringInfo(chaos);
  UnlockSemaphoreInfo(random_info->semaphore);
  return(entropy);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P s e u d o R a n d o m V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPseudoRandomValue() is a Xoshiro generator that returns a non-negative
%  double-precision floating-point value uniformly distributed over the
%  interval [0.0, 1.0) with a 2 to the 256th-1 period.
%
%  The format of the GetPseudoRandomValue method is:
%
%      double GetPseudoRandomValue(RandomInfo *randon_info)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
*/
MagickExport double GetPseudoRandomValue(
  RandomInfo *magick_restrict random_info)
{
#define RandomROTL(x,k) (((x) << (k)) | ((x) >> (64-(k))))

  const MagickSizeType
    alpha = (random_info->seed[1] << 17),
    value = (random_info->seed[0]+random_info->seed[3]);

  random_info->seed[2]^=random_info->seed[0];
  random_info->seed[3]^=random_info->seed[1];
  random_info->seed[1]^=random_info->seed[2];
  random_info->seed[0]^=random_info->seed[3];
  random_info->seed[2]^=alpha;
  random_info->seed[3]=RandomROTL(random_info->seed[3],45);
  return((double) ((value >> 11)*random_info->normalize));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t R a n d o m I n f o N o r m a l i z e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetRandomInfoNormalize() returns the random normalize value.
%
%  The format of the GetRandomInfoNormalize method is:
%
%      double GetRandomInfoNormalize(const RandomInfo *random_info)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
*/
MagickPrivate double GetRandomInfoNormalize(const RandomInfo *random_info)
{
  assert(random_info != (const RandomInfo *) NULL);
  return(random_info->normalize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t R a n d o m I n f o S e e d                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetRandomInfoSeed() returns the random seed.
%
%  The format of the GetRandomInfoSeed method is:
%
%      unsigned long *GetRandomInfoSeed(RandomInfo *random_info)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
*/
MagickPrivate unsigned long *GetRandomInfoSeed(RandomInfo *random_info)
{
  assert(random_info != (RandomInfo *) NULL);
  return((unsigned long *) random_info->seed);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t R a n d o m K e y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetRandomKey() gets a random key from the reservoir.
%
%  The format of the GetRandomKey method is:
%
%      StringInfo *GetRandomKey(RandomInfo *random_info,const size_t length)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
%    o length: the key length.
%
*/
MagickExport StringInfo *GetRandomKey(RandomInfo *random_info,
  const size_t length)
{
  StringInfo
    *key;

  assert(random_info != (RandomInfo *) NULL);
  key=AcquireStringInfo(length);
  SetRandomKey(random_info,length,GetStringInfoDatum(key));
  return(key);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t R a n d o m S e c r e t K e y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetRandomSecretKey() returns the random secret key.
%
%  The format of the GetRandomSecretKey method is:
%
%      unsigned long GetRandomSecretKey(const RandomInfo *random_info)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
*/
MagickExport unsigned long GetRandomSecretKey(const RandomInfo *random_info)
{
  return(random_info->secret_key);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t R a n d o m V a l u e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetRandomValue() return a non-negative double-precision floating-point
%  value uniformly distributed over the interval [0.0, 1.0) with a 2 to the
%  128th-1 period (not cryptographically strong).
%
%  The format of the GetRandomValue method is:
%
%      double GetRandomValue(void)
%
*/
MagickExport double GetRandomValue(RandomInfo *random_info)
{
  unsigned long
    key,
    range;

  range=(~0UL);
  do
  {
    SetRandomKey(random_info,sizeof(key),(unsigned char *) &key);
  } while (key == range);
  return((double) key/range);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R a n d o m C o m p o n e n t G e n e s i s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RandomComponentGenesis() instantiates the random component.
%
%  The format of the RandomComponentGenesis method is:
%
%      MagickBooleanType RandomComponentGenesis(void)
%
*/
MagickPrivate MagickBooleanType RandomComponentGenesis(void)
{
  if (random_semaphore == (SemaphoreInfo *) NULL)
    random_semaphore=AcquireSemaphoreInfo();
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R a n d o m C o m p o n e n t T e r m i n u s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RandomComponentTerminus() destroys the random component.
%
%  The format of the RandomComponentTerminus method is:
%
%      RandomComponentTerminus(void)
%
*/
MagickPrivate void RandomComponentTerminus(void)
{
  if (random_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&random_semaphore);
  RelinquishSemaphoreInfo(&random_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R a n d o m K e y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetRandomKey() sets a random key from the reservoir.
%
%  The format of the SetRandomKey method is:
%
%      void SetRandomKey(RandomInfo *random_info,const size_t length,
%        unsigned char *key)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
%    o length: the key length.
%
%    o key: the key.
%
*/

static inline void IncrementRandomNonce(StringInfo *nonce)
{
  ssize_t
    i;

  unsigned char
    *datum;

  datum=GetStringInfoDatum(nonce);
  for (i=(ssize_t) (GetStringInfoLength(nonce)-1); i != 0; i--)
  {
    datum[i]++;
    if (datum[i] != 0)
      return;
  }
  ThrowFatalException(RandomFatalError,"SequenceWrapError");
}

MagickExport void SetRandomKey(RandomInfo *random_info,const size_t length,
  unsigned char *key)
{
  size_t
    i;

  unsigned char
    *p;

  SignatureInfo
    *signature_info;

  unsigned char
    *datum;

  assert(random_info != (RandomInfo *) NULL);
  if (length == 0)
    return;
  LockSemaphoreInfo(random_info->semaphore);
  signature_info=random_info->signature_info;
  datum=GetStringInfoDatum(random_info->reservoir);
  i=length;
  for (p=key; (i != 0) && (random_info->i != 0); i--)
  {
    *p++=datum[random_info->i];
    random_info->i++;
    if (random_info->i == GetSignatureDigestsize(signature_info))
      random_info->i=0;
  }
  while (i >= GetSignatureDigestsize(signature_info))
  {
    InitializeSignature(signature_info);
    UpdateSignature(signature_info,random_info->nonce);
    FinalizeSignature(signature_info);
    IncrementRandomNonce(random_info->nonce);
    (void) memcpy(p,GetStringInfoDatum(GetSignatureDigest(
      signature_info)),GetSignatureDigestsize(signature_info));
    p+=(ptrdiff_t) GetSignatureDigestsize(signature_info);
    i-=GetSignatureDigestsize(signature_info);
  }
  if (i != 0)
    {
      InitializeSignature(signature_info);
      UpdateSignature(signature_info,random_info->nonce);
      FinalizeSignature(signature_info);
      IncrementRandomNonce(random_info->nonce);
      SetStringInfo(random_info->reservoir,GetSignatureDigest(signature_info));
      random_info->i=i;
      datum=GetStringInfoDatum(random_info->reservoir);
      while (i-- != 0)
        p[i]=datum[i];
    }
  UnlockSemaphoreInfo(random_info->semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R a n d o m S e c r e t K e y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetRandomSecretKey() sets the pseudo-random number generator secret key.
%
%  The format of the SetRandomSecretKey method is:
%
%      void SetRandomSecretKey(const unsigned long key)
%
%  A description of each parameter follows:
%
%    o key: the secret key.
%
*/
MagickExport void SetRandomSecretKey(const unsigned long key)
{
  secret_key=key;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R a n d o m T r u e R a n d o m                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetRandomTrueRandom() declares your intentions to use true random numbers.
%  True random numbers are encouraged but may not always be practical because
%  your application may block while entropy is gathered from your environment.
%
%  The format of the SetRandomTrueRandom method is:
%
%      void SetRandomTrueRandom(const MagickBooleanType true_random)
%
%  A description of each parameter follows:
%
%    o true_random: declare your intentions to use true-random number.
%
*/
MagickExport void SetRandomTrueRandom(const MagickBooleanType true_random)
{
  gather_true_random=true_random;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                   OOO   PPPP   EEEEE  N   N   CCCC  L                       %
%                  O   O  P   P  E      NN  N  C      L                       %
%                  O   O  PPPP   EEE    N N N  C      L                       %
%                  O   O  P      E      N  NN  C      L                       %
%                   OOO   P      EEEEE  N   N   CCCC  LLLLL                   %
%                                                                             %
%                                                                             %
%                         MagickCore OpenCL Methods                           %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 March 2000                                  %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/accelerate-kernels-private.h"
#include "MagickCore/artifact.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/color.h"
#include "MagickCore/compare.h"
#include "MagickCore/constitute.h"
#include "MagickCore/configure.h"
#include "MagickCore/distort.h"
#include "MagickCore/draw.h"
#include "MagickCore/effect.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/fx.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/layer.h"
#include "MagickCore/locale_.h"
#include "MagickCore/mime-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/memory-private.h"
#include "MagickCore/monitor.h"
#include "MagickCore/montage.h"
#include "MagickCore/morphology.h"
#include "MagickCore/nt-base.h"
#include "MagickCore/nt-base-private.h"
#include "MagickCore/opencl.h"
#include "MagickCore/opencl-private.h"
#include "MagickCore/option.h"
#include "MagickCore/policy.h"
#include "MagickCore/property.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/resample.h"
#include "MagickCore/resource_.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"

#if defined(MAGICKCORE_OPENCL_SUPPORT)
#if defined(MAGICKCORE_LTDL_DELEGATE)
#include "ltdl.h"
#endif

/*
  Define declarations.
*/
#define IMAGEMAGICK_PROFILE_FILE "ImagemagickOpenCLDeviceProfile.xml"

/*
  Typedef declarations.
*/
typedef struct
{
  long long freq;
  long long clocks;
  long long start;
} AccelerateTimer;

typedef struct
{
  char
    *name,
    *platform_name,
    *vendor_name,
    *version;

  cl_uint
    max_clock_frequency,
    max_compute_units;

  double
    score;
} MagickCLDeviceBenchmark;

/*
  Forward declarations.
*/

static MagickBooleanType
  HasOpenCLDevices(MagickCLEnv,ExceptionInfo *),
  LoadOpenCLLibrary(void);

static MagickCLDevice
  RelinquishMagickCLDevice(MagickCLDevice);

static MagickCLEnv
  RelinquishMagickCLEnv(MagickCLEnv);

static void
  BenchmarkOpenCLDevices(MagickCLEnv);

/* OpenCL library */
MagickLibrary
  *openCL_library;

/* Default OpenCL environment */
MagickCLEnv
  default_CLEnv;
MagickThreadType
  test_thread_id=0;
SemaphoreInfo
  *openCL_lock;

/* Cached location of the OpenCL cache files */
char
  *cache_directory;
SemaphoreInfo
  *cache_directory_lock;

static inline MagickBooleanType IsSameOpenCLDevice(MagickCLDevice a,
  MagickCLDevice b)
{
  if ((LocaleCompare(a->platform_name,b->platform_name) == 0) &&
      (LocaleCompare(a->vendor_name,b->vendor_name) == 0) &&
      (LocaleCompare(a->name,b->name) == 0) &&
      (LocaleCompare(a->version,b->version) == 0) &&
      (a->max_clock_frequency == b->max_clock_frequency) &&
      (a->max_compute_units == b->max_compute_units))
    return(MagickTrue);

  return(MagickFalse);
}

static inline MagickBooleanType IsBenchmarkedOpenCLDevice(MagickCLDevice a,
  MagickCLDeviceBenchmark *b)
{
  if ((LocaleCompare(a->platform_name,b->platform_name) == 0) &&
      (LocaleCompare(a->vendor_name,b->vendor_name) == 0) &&
      (LocaleCompare(a->name,b->name) == 0) &&
      (LocaleCompare(a->version,b->version) == 0) &&
      (a->max_clock_frequency == b->max_clock_frequency) &&
      (a->max_compute_units == b->max_compute_units))
    return(MagickTrue);

  return(MagickFalse);
}

static inline void RelinquishMagickCLDevices(MagickCLEnv clEnv)
{
  size_t
    i;

  if (clEnv->devices != (MagickCLDevice *) NULL)
    {
      for (i = 0; i < clEnv->number_devices; i++)
        clEnv->devices[i]=RelinquishMagickCLDevice(clEnv->devices[i]);
      clEnv->devices=(MagickCLDevice *) RelinquishMagickMemory(clEnv->devices);
    }
  clEnv->number_devices=0;
}

static inline MagickBooleanType MagickCreateDirectory(const char *path)
{
  int
    status;

#ifdef MAGICKCORE_WINDOWS_SUPPORT
  status=mkdir(path);
#else
  status=mkdir(path,0777);
#endif
  return(status == 0 ? MagickTrue : MagickFalse);
}

static inline void InitAccelerateTimer(AccelerateTimer *timer)
{
#ifdef _WIN32
  QueryPerformanceFrequency((LARGE_INTEGER*)&timer->freq);
#else
  timer->freq=(long long)1.0E3;
#endif
  timer->clocks=0;
  timer->start=0;
}

static inline double ReadAccelerateTimer(AccelerateTimer *timer)
{
  return (double)timer->clocks/(double)timer->freq;
}

static inline void StartAccelerateTimer(AccelerateTimer* timer)
{
#ifdef _WIN32
  QueryPerformanceCounter((LARGE_INTEGER*)&timer->start);
#else
  struct timeval
    s;
  gettimeofday(&s,0);
  timer->start=(long long)s.tv_sec*(long long)1.0E3+(long long)s.tv_usec/
    (long long)1.0E3;
#endif
}

static inline void StopAccelerateTimer(AccelerateTimer *timer)
{
  long long
    n;

  n=0;
#ifdef _WIN32
  QueryPerformanceCounter((LARGE_INTEGER*)&(n));
#else
  struct timeval
    s;
  gettimeofday(&s,0);
  n=(long long)s.tv_sec*(long long)1.0E3+(long long)s.tv_usec/
    (long long)1.0E3;
#endif
  n-=timer->start;
  timer->start=0;
  timer->clocks+=n;
}

static const char *GetOpenCLCacheDirectory()
{
  if (cache_directory == (char *) NULL)
    {
      if (cache_directory_lock == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&cache_directory_lock);
      LockSemaphoreInfo(cache_directory_lock);
      if (cache_directory == (char *) NULL)
        {
          char
            *home,
            path[MagickPathExtent],
            *temp;

          MagickBooleanType
            status;

          struct stat
            attributes;

          temp=(char *) NULL;
          home=GetEnvironmentValue("MAGICK_OPENCL_CACHE_DIR");
          if (home == (char *) NULL)
            {
              home=GetEnvironmentValue("XDG_CACHE_HOME");
#if defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__MINGW32__)
              if (home == (char *) NULL)
                home=GetEnvironmentValue("LOCALAPPDATA");
              if (home == (char *) NULL)
                home=GetEnvironmentValue("APPDATA");
              if (home == (char *) NULL)
                home=GetEnvironmentValue("USERPROFILE");
#endif
            }

          if (home != (char *) NULL)
            {
              /* first check if $HOME exists */
              (void) FormatLocaleString(path,MagickPathExtent,"%s",home);
              status=GetPathAttributes(path,&attributes);
              if (status == MagickFalse)
                status=MagickCreateDirectory(path);

              /* first check if $HOME/ImageMagick exists */
              if (status != MagickFalse)
                {
                  (void) FormatLocaleString(path,MagickPathExtent,
                    "%s%sImageMagick",home,DirectorySeparator);

                  status=GetPathAttributes(path,&attributes);
                  if (status == MagickFalse)
                    status=MagickCreateDirectory(path);
                }

              if (status != MagickFalse)
                {
                  temp=(char*) AcquireCriticalMemory(strlen(path)+1);
                  CopyMagickString(temp,path,strlen(path)+1);
                }
              home=DestroyString(home);
            }
          else
            {
              home=GetEnvironmentValue("HOME");
              if (home != (char *) NULL)
                {
                  /* first check if $HOME/.cache exists */
                  (void) FormatLocaleString(path,MagickPathExtent,"%s%s.cache",
                    home,DirectorySeparator);
                  status=GetPathAttributes(path,&attributes);
                  if (status == MagickFalse)
                    status=MagickCreateDirectory(path);

                  /* first check if $HOME/.cache/ImageMagick exists */
                  if (status != MagickFalse)
                    {
                      (void) FormatLocaleString(path,MagickPathExtent,
                        "%s%s.cache%sImageMagick",home,DirectorySeparator,
                        DirectorySeparator);
                      status=GetPathAttributes(path,&attributes);
                      if (status == MagickFalse)
                        status=MagickCreateDirectory(path);
                    }

                  if (status != MagickFalse)
                    {
                      temp=(char*) AcquireCriticalMemory(strlen(path)+1);
                      CopyMagickString(temp,path,strlen(path)+1);
                    }
                  home=DestroyString(home);
                }
            }
          if (temp == (char *) NULL)
            {
              temp=AcquireString("?");
              (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
                "Cannot use cache directory: \"%s\"",path);
            }
          else
            (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
              "Using cache directory: \"%s\"",temp);
          cache_directory=temp;
        }
      UnlockSemaphoreInfo(cache_directory_lock);
    }
  if (*cache_directory == '?')
    return((const char *) NULL);
  return(cache_directory);
}

static void SelectOpenCLDevice(MagickCLEnv clEnv,cl_device_type type)
{
  MagickCLDevice
    device;

  size_t
    i,
    j;

  (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
    "Selecting device for type: %d",(int) type);
  for (i = 0; i < clEnv->number_devices; i++)
    clEnv->devices[i]->enabled=MagickFalse;

  for (i = 0; i < clEnv->number_devices; i++)
  {
    device=clEnv->devices[i];
    if (device->type != type)
      continue;

    device->enabled=MagickTrue;
    (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
      "Selected device: %s",device->name);
    for (j = i+1; j < clEnv->number_devices; j++)
    {
      MagickCLDevice
        other_device;

      other_device=clEnv->devices[j];
      if (IsSameOpenCLDevice(device,other_device))
        other_device->enabled=MagickTrue;
    }
  }
}

static size_t StringSignature(const char* string)
{
  size_t
    n,
    i,
    j,
    signature,
    stringLength;

  union
  {
    const char* s;
    const size_t* u;
  } p;

  stringLength=(size_t) strlen(string);
  signature=stringLength;
  n=stringLength/sizeof(size_t);
  p.s=string;
  for (i = 0; i < n; i++)
    signature^=p.u[i];
  if (n * sizeof(size_t) != stringLength)
    {
      char
        padded[4];

      j=n*sizeof(size_t);
      for (i = 0; i < 4; i++, j++)
      {
        if (j < stringLength)
          padded[i]=p.s[j];
        else
          padded[i]=0;
      }
      p.s=padded;
      signature^=p.u[0];
    }
  return(signature);
}

static void DestroyMagickCLCacheInfo(MagickCLCacheInfo info)
{
  ssize_t
    i;

  for (i=0; i < (ssize_t) info->event_count; i++)
    openCL_library->clReleaseEvent(info->events[i]);
  info->events=(cl_event *) RelinquishMagickMemory(info->events);
  if (info->buffer != (cl_mem) NULL)
    openCL_library->clReleaseMemObject(info->buffer);
  RelinquishSemaphoreInfo(&info->events_semaphore);
  ReleaseOpenCLDevice(info->device);
  RelinquishMagickMemory(info);
}

/*
  Provide call to OpenCL library methods
*/

MagickPrivate cl_mem CreateOpenCLBuffer(MagickCLDevice device,
  cl_mem_flags flags,size_t size,void *host_ptr)
{
  return(openCL_library->clCreateBuffer(device->context,flags,size,host_ptr,
    (cl_int *) NULL));
}

MagickPrivate void ReleaseOpenCLKernel(cl_kernel kernel)
{
  (void) openCL_library->clReleaseKernel(kernel);
}

MagickPrivate void ReleaseOpenCLMemObject(cl_mem memobj)
{
  (void) openCL_library->clReleaseMemObject(memobj);
}

MagickPrivate void RetainOpenCLMemObject(cl_mem memobj)
{
  (void) openCL_library->clRetainMemObject(memobj);
}

MagickPrivate cl_int SetOpenCLKernelArg(cl_kernel kernel,size_t arg_index,
  size_t arg_size,const void *arg_value)
{
  return(openCL_library->clSetKernelArg(kernel,(cl_uint) arg_index,arg_size,
    arg_value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e M a g i c k C L C a c h e I n f o                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMagickCLCacheInfo() acquires an OpenCL cache info structure.
%
%  The format of the AcquireMagickCLCacheInfo method is:
%
%      MagickCLCacheInfo AcquireMagickCLCacheInfo(MagickCLDevice device,
%        Quantum *pixels,const MagickSizeType length)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
%
%    o pixels: the pixel buffer of the image.
%
%    o length: the length of the pixel buffer.
%
*/

MagickPrivate MagickCLCacheInfo AcquireMagickCLCacheInfo(MagickCLDevice device,
  Quantum *pixels,const MagickSizeType length)
{
  cl_int
    status;

  MagickCLCacheInfo
    info;

  info=(MagickCLCacheInfo) AcquireCriticalMemory(sizeof(*info));
  (void) memset(info,0,sizeof(*info));
  LockSemaphoreInfo(openCL_lock);
  device->requested++;
  UnlockSemaphoreInfo(openCL_lock);
  info->device=device;
  info->length=length;
  info->pixels=pixels;
  info->events_semaphore=AcquireSemaphoreInfo();
  info->buffer=openCL_library->clCreateBuffer(device->context,
    CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,(size_t) length,(void *) pixels,
    &status);
  if (status == CL_SUCCESS)
    return(info);
  DestroyMagickCLCacheInfo(info);
  return((MagickCLCacheInfo) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e M a g i c k C L D e v i c e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMagickCLDevice() acquires an OpenCL device
%
%  The format of the AcquireMagickCLDevice method is:
%
%      MagickCLDevice AcquireMagickCLDevice()
%
*/

static MagickCLDevice AcquireMagickCLDevice()
{
  MagickCLDevice
    device;

  device=(MagickCLDevice) AcquireMagickMemory(sizeof(*device));
  if (device != NULL)
  {
    (void) memset(device,0,sizeof(*device));
    ActivateSemaphoreInfo(&device->lock);
    device->score=MAGICKCORE_OPENCL_UNDEFINED_SCORE;
    device->command_queues_index=-1;
    device->enabled=MagickTrue;
  }
  return(device);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e M a g i c k C L E n v                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AcquireMagickCLEnv() allocates the MagickCLEnv structure
%
*/

static MagickCLEnv AcquireMagickCLEnv(void)
{
  const char
    *option;

  MagickCLEnv
    clEnv;

  clEnv=(MagickCLEnv) AcquireMagickMemory(sizeof(*clEnv));
  if (clEnv != (MagickCLEnv) NULL)
  {
    (void) memset(clEnv,0,sizeof(*clEnv));
    ActivateSemaphoreInfo(&clEnv->lock);
    clEnv->cpu_score=MAGICKCORE_OPENCL_UNDEFINED_SCORE;
    clEnv->enabled=MagickFalse;
    option=getenv("MAGICK_OCL_DEVICE");
    if (option != (const char *) NULL)
      {
        if ((IsStringTrue(option) != MagickFalse) ||
            (strcmp(option,"GPU") == 0) ||
            (strcmp(option,"CPU") == 0))
          clEnv->enabled=MagickTrue;
      }
  }
  return clEnv;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e O p e n C L C o m m a n d Q u e u e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireOpenCLCommandQueue() acquires an OpenCL command queue
%
%  The format of the AcquireOpenCLCommandQueue method is:
%
%      cl_command_queue AcquireOpenCLCommandQueue(MagickCLDevice device)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
%
*/

MagickPrivate cl_command_queue AcquireOpenCLCommandQueue(MagickCLDevice device)
{
  cl_command_queue
    queue;

  cl_command_queue_properties
    properties;

  assert(device != (MagickCLDevice) NULL);
  LockSemaphoreInfo(device->lock);
  if ((device->profile_kernels == MagickFalse) &&
      (device->command_queues_index >= 0))
  {
    queue=device->command_queues[device->command_queues_index--];
    UnlockSemaphoreInfo(device->lock);
  }
  else
  {
    UnlockSemaphoreInfo(device->lock);
    properties=0;
    if (device->profile_kernels != MagickFalse)
      properties=CL_QUEUE_PROFILING_ENABLE;
    queue=openCL_library->clCreateCommandQueue(device->context,
      device->deviceID,properties,(cl_int *) NULL);
  }
  return(queue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e O p e n C L K e r n e l                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireOpenCLKernel() acquires an OpenCL kernel
%
%  The format of the AcquireOpenCLKernel method is:
%
%      cl_kernel AcquireOpenCLKernel(MagickCLEnv clEnv,
%        MagickOpenCLProgram program, const char* kernelName)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
%
%    o program: the OpenCL program module that the kernel belongs to.
%
%    o kernelName:  the name of the kernel
%
*/

MagickPrivate cl_kernel AcquireOpenCLKernel(MagickCLDevice device,
  const char *kernel_name)
{
  cl_kernel
    kernel;

  assert(device != (MagickCLDevice) NULL);
  (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),"Using kernel: %s",
    kernel_name);
  kernel=openCL_library->clCreateKernel(device->program,kernel_name,
    (cl_int *) NULL);
  return(kernel);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A u t o S e l e c t O p e n C L D e v i c e s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AutoSelectOpenCLDevices() determines the best device based on the
%  information from the micro-benchmark.
%
%  The format of the AutoSelectOpenCLDevices method is:
%
%      void AcquireOpenCLKernel(MagickCLEnv clEnv,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
static void LoadOpenCLDeviceBenchmark(MagickCLEnv clEnv,const char *xml)
{
  char
    keyword[MagickPathExtent],
    *token;

  const char
    *q;

  MagickCLDeviceBenchmark
    *device_benchmark;

  size_t
    i,
    extent;

  if (xml == (char *) NULL)
    return;
  device_benchmark=(MagickCLDeviceBenchmark *) NULL;
  token=AcquireString(xml);
  extent=strlen(token)+MagickPathExtent;
  for (q=(char *) xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    (void) GetNextToken(q,&q,extent,token);
    if (*token == '\0')
      break;
    (void) CopyMagickString(keyword,token,MagickPathExtent);
    if (LocaleNCompare(keyword,"<!DOCTYPE",9) == 0)
      {
        /*
          Doctype element.
        */
        while ((LocaleNCompare(q,"]>",2) != 0) && (*q != '\0'))
          (void) GetNextToken(q,&q,extent,token);
        continue;
      }
    if (LocaleNCompare(keyword,"<!--",4) == 0)
      {
        /*
          Comment element.
        */
        while ((LocaleNCompare(q,"->",2) != 0) && (*q != '\0'))
          (void) GetNextToken(q,&q,extent,token);
        continue;
      }
    if (LocaleCompare(keyword,"<device") == 0)
      {
        /*
          Device element.
        */
        device_benchmark=(MagickCLDeviceBenchmark *) AcquireQuantumMemory(1,
          sizeof(*device_benchmark));
        if (device_benchmark == (MagickCLDeviceBenchmark *) NULL)
          break;
        (void) memset(device_benchmark,0,sizeof(*device_benchmark));
        device_benchmark->score=MAGICKCORE_OPENCL_UNDEFINED_SCORE;
        continue;
      }
    if (device_benchmark == (MagickCLDeviceBenchmark *) NULL)
      continue;
    if (LocaleCompare(keyword,"/>") == 0)
      {
        if (device_benchmark->score != MAGICKCORE_OPENCL_UNDEFINED_SCORE)
          {
            if (LocaleCompare(device_benchmark->name,"CPU") == 0)
              clEnv->cpu_score=device_benchmark->score;
            else
              {
                MagickCLDevice
                  device;

                /*
                  Set the score for all devices that match this device.
                */
                for (i = 0; i < clEnv->number_devices; i++)
                {
                  device=clEnv->devices[i];
                  if (IsBenchmarkedOpenCLDevice(device,device_benchmark))
                    device->score=device_benchmark->score;
                }
              }
          }

        device_benchmark->platform_name=(char *) RelinquishMagickMemory(
          device_benchmark->platform_name);
        device_benchmark->vendor_name=(char *) RelinquishMagickMemory(
          device_benchmark->vendor_name);
        device_benchmark->name=(char *) RelinquishMagickMemory(
          device_benchmark->name);
        device_benchmark->version=(char *) RelinquishMagickMemory(
          device_benchmark->version);
        device_benchmark=(MagickCLDeviceBenchmark *) RelinquishMagickMemory(
          device_benchmark);
        continue;
      }
    (void) GetNextToken(q,(const char **) NULL,extent,token);
    if (*token != '=')
      continue;
    (void) GetNextToken(q,&q,extent,token);
    (void) GetNextToken(q,&q,extent,token);
    switch (*keyword)
    {
      case 'M':
      case 'm':
      {
        if (LocaleCompare((char *) keyword,"maxClockFrequency") == 0)
          {
            device_benchmark->max_clock_frequency=StringToInteger(token);
            break;
          }
        if (LocaleCompare((char *) keyword,"maxComputeUnits") == 0)
          {
            device_benchmark->max_compute_units=StringToInteger(token);
            break;
          }
        break;
      }
      case 'N':
      case 'n':
      {
        if (LocaleCompare((char *) keyword,"name") == 0)
          device_benchmark->name=ConstantString(token);
        break;
      }
      case 'P':
      case 'p':
      {
        if (LocaleCompare((char *) keyword,"platform") == 0)
          device_benchmark->platform_name=ConstantString(token);
        break;
      }
      case 'S':
      case 's':
      {
        if (LocaleCompare((char *) keyword,"score") == 0)
          device_benchmark->score=StringToDouble(token,(char **) NULL);
        break;
      }
      case 'V':
      case 'v':
      {
        if (LocaleCompare((char *) keyword,"vendor") == 0)
          device_benchmark->vendor_name=ConstantString(token);
        if (LocaleCompare((char *) keyword,"version") == 0)
          device_benchmark->version=ConstantString(token);
        break;
      }
      default:
        break;
    }
  }
  token=(char *) RelinquishMagickMemory(token);
  device_benchmark=(MagickCLDeviceBenchmark *) RelinquishMagickMemory(
    device_benchmark);
}

static MagickBooleanType CanWriteProfileToFile(const char *filename)
{
  FILE
    *profileFile;

  profileFile=fopen(filename,"ab");

  if (profileFile == (FILE *) NULL)
    {
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
        "Unable to save profile to: \"%s\"",filename);
      return(MagickFalse);
    }

  fclose(profileFile);
  return(MagickTrue);
}
#endif

static MagickBooleanType LoadOpenCLBenchmarks(MagickCLEnv clEnv)
{
#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
  char
    filename[MagickPathExtent];

  StringInfo
    *option;

  (void) FormatLocaleString(filename,MagickPathExtent,"%s%s%s",
    GetOpenCLCacheDirectory(),DirectorySeparator,IMAGEMAGICK_PROFILE_FILE);

  /*
    We don't run the benchmark when we can not write out a device profile. The
    first GPU device will be used.
  */
  if (CanWriteProfileToFile(filename) == MagickFalse)
#endif
    {
      size_t
        i;

      for (i = 0; i < clEnv->number_devices; i++)
        clEnv->devices[i]->score=1.0;

      SelectOpenCLDevice(clEnv,CL_DEVICE_TYPE_GPU);
      return(MagickFalse);
    }
#if !MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
  option=ConfigureFileToStringInfo(filename);
  LoadOpenCLDeviceBenchmark(clEnv,(const char *) GetStringInfoDatum(option));
  option=DestroyStringInfo(option);
  return(MagickTrue);
#endif
}

static void AutoSelectOpenCLDevices(MagickCLEnv clEnv)
{
  const char
    *option;

  double
    best_score;

  MagickBooleanType
    benchmark;

  size_t
    i;

  option=getenv("MAGICK_OCL_DEVICE");
  if (option != (const char *) NULL)
    {
      if (strcmp(option,"GPU") == 0)
        SelectOpenCLDevice(clEnv,CL_DEVICE_TYPE_GPU);
      else if (strcmp(option,"CPU") == 0)
        SelectOpenCLDevice(clEnv,CL_DEVICE_TYPE_CPU);
    }

  if (LoadOpenCLBenchmarks(clEnv) == MagickFalse)
    return;

  benchmark=MagickFalse;
  if (clEnv->cpu_score == MAGICKCORE_OPENCL_UNDEFINED_SCORE)
    benchmark=MagickTrue;
  else
    {
      for (i = 0; i < clEnv->number_devices; i++)
      {
        if (clEnv->devices[i]->score == MAGICKCORE_OPENCL_UNDEFINED_SCORE)
        {
          benchmark=MagickTrue;
          break;
        }
      }
    }

  if (benchmark != MagickFalse)
    BenchmarkOpenCLDevices(clEnv);

  best_score=clEnv->cpu_score;
  for (i = 0; i < clEnv->number_devices; i++)
    best_score=MagickMin(clEnv->devices[i]->score,best_score);

  for (i = 0; i < clEnv->number_devices; i++)
  {
    if (clEnv->devices[i]->score != best_score)
      clEnv->devices[i]->enabled=MagickFalse;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B e n c h m a r k O p e n C L D e v i c e s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BenchmarkOpenCLDevices() benchmarks the OpenCL devices and the CPU to help
%  the automatic selection of the best device.
%
%  The format of the BenchmarkOpenCLDevices method is:
%
%    void BenchmarkOpenCLDevices(MagickCLEnv clEnv,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
%
%    o exception: return any errors or warnings
*/

static double RunOpenCLBenchmark(MagickBooleanType is_cpu)
{
  AccelerateTimer
    timer;

  ExceptionInfo
    *exception;

  Image
    *inputImage;

  ImageInfo
    *imageInfo;

  size_t
    i;

  exception=AcquireExceptionInfo();
  imageInfo=AcquireImageInfo();
  CloneString(&imageInfo->size,"2048x1536");
  CopyMagickString(imageInfo->filename,"xc:none",MagickPathExtent);
  inputImage=ReadImage(imageInfo,exception);
  if (inputImage == (Image *) NULL)
    return(0.0);

  InitAccelerateTimer(&timer);

  for (i=0; i<=2; i++)
  {
    Image
      *blurredImage,
      *resizedImage,
      *unsharpedImage;

    if (i > 0)
      StartAccelerateTimer(&timer);

    blurredImage=BlurImage(inputImage,10.0f,3.5f,exception);
    unsharpedImage=UnsharpMaskImage(blurredImage,2.0f,2.0f,50.0f,10.0f,
      exception);
    resizedImage=ResizeImage(unsharpedImage,640,480,LanczosFilter,
      exception);

    /*
      We need this to get a proper performance benchmark, the operations
      are executed asynchronous.
    */
    if (is_cpu == MagickFalse)
      {
        CacheInfo
          *cache_info;

        cache_info=(CacheInfo *) resizedImage->cache;
        if (cache_info->opencl != (MagickCLCacheInfo) NULL)
          openCL_library->clWaitForEvents(cache_info->opencl->event_count,
            cache_info->opencl->events);
      }

    if (i > 0)
      StopAccelerateTimer(&timer);

    if (blurredImage != (Image *) NULL)
      DestroyImage(blurredImage);
    if (unsharpedImage != (Image *) NULL)
      DestroyImage(unsharpedImage);
    if (resizedImage != (Image *) NULL)
      DestroyImage(resizedImage);
  }
  DestroyImage(inputImage);
  return(ReadAccelerateTimer(&timer));
}

static void RunDeviceBenchmark(MagickCLEnv clEnv,MagickCLEnv testEnv,
  MagickCLDevice device)
{
  testEnv->devices[0]=device;
  default_CLEnv=testEnv;
  device->score=RunOpenCLBenchmark(MagickFalse);
  default_CLEnv=clEnv;
  testEnv->devices[0]=(MagickCLDevice) NULL;
}

static void CacheOpenCLBenchmarks(MagickCLEnv clEnv)
{
  char
    filename[MagickPathExtent];

  FILE
    *cache_file;

  MagickCLDevice
    device;

  size_t
    i,
    j;

  (void) FormatLocaleString(filename,MagickPathExtent,"%s%s%s",
    GetOpenCLCacheDirectory(),DirectorySeparator,
    IMAGEMAGICK_PROFILE_FILE);

  cache_file=fopen_utf8(filename,"wb");
  if (cache_file == (FILE *) NULL)
    return;
  fwrite("<devices>\n",sizeof(char),10,cache_file);
  fprintf(cache_file,"  <device name=\"CPU\" score=\"%.4g\"/>\n",
    clEnv->cpu_score);
  for (i = 0; i < clEnv->number_devices; i++)
  {
    MagickBooleanType
      duplicate;

    device=clEnv->devices[i];
    duplicate=MagickFalse;
    for (j = 0; j < i; j++)
    {
      if (IsSameOpenCLDevice(clEnv->devices[j],device))
      {
        duplicate=MagickTrue;
        break;
      }
    }

    if (duplicate)
      continue;

    if (device->score != MAGICKCORE_OPENCL_UNDEFINED_SCORE)
      fprintf(cache_file,"  <device platform=\"%s\" vendor=\"%s\" name=\"%s\"\
 version=\"%s\" maxClockFrequency=\"%d\" maxComputeUnits=\"%d\"\
 score=\"%.4g\"/>\n",
        device->platform_name,device->vendor_name,device->name,device->version,
        (int)device->max_clock_frequency,(int)device->max_compute_units,
        device->score);
  }
  fwrite("</devices>",sizeof(char),10,cache_file);

  fclose(cache_file);
}

static void BenchmarkOpenCLDevices(MagickCLEnv clEnv)
{
  MagickCLDevice
    device;

  MagickCLEnv
    testEnv;

  size_t
    i,
    j;

  (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
    "Starting benchmark");
  testEnv=AcquireMagickCLEnv();
  testEnv->library=openCL_library;
  testEnv->devices=(MagickCLDevice *) AcquireCriticalMemory(
    sizeof(MagickCLDevice));
  testEnv->number_devices=1;
  testEnv->benchmark_thread_id=GetMagickThreadId();
  testEnv->initialized=MagickTrue;

  for (i = 0; i < clEnv->number_devices; i++)
    clEnv->devices[i]->score=MAGICKCORE_OPENCL_UNDEFINED_SCORE;

  for (i = 0; i < clEnv->number_devices; i++)
  {
    device=clEnv->devices[i];
    if (device->score == MAGICKCORE_OPENCL_UNDEFINED_SCORE)
      RunDeviceBenchmark(clEnv,testEnv,device);

    /* Set the score on all the other devices that are the same */
    for (j = i+1; j < clEnv->number_devices; j++)
    {
      MagickCLDevice
        other_device;

      other_device=clEnv->devices[j];
      if (IsSameOpenCLDevice(device,other_device))
        other_device->score=device->score;
    }
  }

  testEnv->enabled=MagickFalse;
  default_CLEnv=testEnv;
  clEnv->cpu_score=RunOpenCLBenchmark(MagickTrue);
  default_CLEnv=clEnv;

  testEnv=RelinquishMagickCLEnv(testEnv);
  CacheOpenCLBenchmarks(clEnv);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p i l e O p e n C L K e r n e l                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompileOpenCLKernel() compiles the kernel for the specified device. The
%  kernel will be cached on disk to reduce the compilation time.
%
%  The format of the CompileOpenCLKernel method is:
%
%      MagickBooleanType AcquireOpenCLKernel(MagickCLDevice clEnv,
%        unsigned int signature,const char *kernel,const char *options,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
%
%    o kernel: the source code of the kernel.
%
%    o options: options for the compiler.
%
%    o signature: a number to uniquely identify the kernel
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void CacheOpenCLKernel(MagickCLDevice device,char *filename,
  ExceptionInfo *exception)
{
  cl_uint
    status;

  size_t
    binaryProgramSize;

  unsigned char
    *binaryProgram;

  status=openCL_library->clGetProgramInfo(device->program,
    CL_PROGRAM_BINARY_SIZES,sizeof(size_t),&binaryProgramSize,NULL);
  if (status != CL_SUCCESS)
    return;
  binaryProgram=(unsigned char*) AcquireQuantumMemory(1,binaryProgramSize);
  if (binaryProgram == (unsigned char *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",filename);
      return;
    }
  status=openCL_library->clGetProgramInfo(device->program,
    CL_PROGRAM_BINARIES,sizeof(unsigned char*),&binaryProgram,NULL);
  if (status == CL_SUCCESS)
    {
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
        "Creating cache file: \"%s\"",filename);
      (void) BlobToFile(filename,binaryProgram,binaryProgramSize,exception);
    }
  binaryProgram=(unsigned char *) RelinquishMagickMemory(binaryProgram);
}

static MagickBooleanType LoadCachedOpenCLKernels(MagickCLDevice device,
  const char *filename)
{
  cl_int
    binaryStatus,
    status;

  ExceptionInfo
    *sans_exception;

  size_t
    length;

  unsigned char
    *binaryProgram;

  sans_exception=AcquireExceptionInfo();
  binaryProgram=(unsigned char *) FileToBlob(filename,SIZE_MAX,&length,
    sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (binaryProgram == (unsigned char *) NULL)
    return(MagickFalse);
  (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
    "Loaded cached kernels: \"%s\"",filename);
  device->program=openCL_library->clCreateProgramWithBinary(device->context,1,
    &device->deviceID,&length,(const unsigned char**)&binaryProgram,
    &binaryStatus,&status);
  binaryProgram=(unsigned char *) RelinquishMagickMemory(binaryProgram);
  return((status != CL_SUCCESS) || (binaryStatus != CL_SUCCESS) ? MagickFalse :
    MagickTrue);
}

static void LogOpenCLBuildFailure(MagickCLDevice device,const char *kernel,
  ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent],
    *log;

  size_t
    log_size;

  (void) FormatLocaleString(filename,MagickPathExtent,"%s%s%s",
    GetOpenCLCacheDirectory(),DirectorySeparator,"magick_badcl.cl");

  (void) remove_utf8(filename);
  (void) BlobToFile(filename,kernel,strlen(kernel),exception);

  openCL_library->clGetProgramBuildInfo(device->program,device->deviceID,
    CL_PROGRAM_BUILD_LOG,0,NULL,&log_size);
  log=(char*)AcquireCriticalMemory(log_size);
  openCL_library->clGetProgramBuildInfo(device->program,device->deviceID,
    CL_PROGRAM_BUILD_LOG,log_size,log,&log_size);

  (void) FormatLocaleString(filename,MagickPathExtent,"%s%s%s",
    GetOpenCLCacheDirectory(),DirectorySeparator,"magick_badcl.log");

  (void) remove_utf8(filename);
  (void) BlobToFile(filename,log,log_size,exception);
  log=(char*)RelinquishMagickMemory(log);
}

static MagickBooleanType CompileOpenCLKernel(MagickCLDevice device,
  const char *kernel,const char *options,size_t signature,
  ExceptionInfo *exception)
{
  char
    deviceName[MagickPathExtent],
    filename[MagickPathExtent],
    *ptr;

  cl_int
    status;

  MagickBooleanType
    loaded;

  size_t
    length;

  (void) CopyMagickString(deviceName,device->name,MagickPathExtent);
  ptr=deviceName;
  /* Strip out illegal characters for file names */
  while (*ptr != '\0')
  {
    if ((*ptr == ' ') || (*ptr == '\\') || (*ptr == '/') || (*ptr == ':') ||
        (*ptr == '*') || (*ptr == '?') || (*ptr == '"') || (*ptr == '<') ||
        (*ptr == '>' || *ptr == '|'))
      *ptr = '_';
    ptr++;
  }
  (void) FormatLocaleString(filename,MagickPathExtent,
    "%s%s%s_%s_%08x_%.20g.bin",GetOpenCLCacheDirectory(),
    DirectorySeparator,"magick_opencl",deviceName,(unsigned int) signature,
    (double) sizeof(char*)*8);
  loaded=LoadCachedOpenCLKernels(device,filename);
  if (loaded == MagickFalse)
    {
      /* Binary CL program unavailable, compile the program from source */
      length=strlen(kernel);
      device->program=openCL_library->clCreateProgramWithSource(
        device->context,1,&kernel,&length,&status);
      if (status != CL_SUCCESS)
        return(MagickFalse);
    }

  status=openCL_library->clBuildProgram(device->program,1,&device->deviceID,
    options,NULL,NULL);
  if (status != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception,GetMagickModule(),DelegateWarning,
      "clBuildProgram failed.","(%d)",(int)status);
    LogOpenCLBuildFailure(device,kernel,exception);
    return(MagickFalse);
  }

  /* Save the binary to a file to avoid re-compilation of the kernels */
  if (loaded == MagickFalse)
    CacheOpenCLKernel(device,filename,exception);

  return(MagickTrue);
}

static cl_event* CopyOpenCLEvents(MagickCLCacheInfo first,
  MagickCLCacheInfo second,cl_uint *event_count)
{
  cl_event
    *events;

  size_t
    i;

  size_t
    j;

  assert(first != (MagickCLCacheInfo) NULL);
  assert(event_count != (cl_uint *) NULL);
  events=(cl_event *) NULL;
  LockSemaphoreInfo(first->events_semaphore);
  if (second != (MagickCLCacheInfo) NULL)
    LockSemaphoreInfo(second->events_semaphore);
  *event_count=first->event_count;
  if (second != (MagickCLCacheInfo) NULL)
    *event_count+=second->event_count;
  if (*event_count > 0)
    {
      events=(cl_event *) AcquireQuantumMemory(*event_count,sizeof(*events));
      if (events == (cl_event *) NULL)
        *event_count=0;
      else
        {
          j=0;
          for (i=0; i < first->event_count; i++, j++)
            events[j]=first->events[i];
          if (second != (MagickCLCacheInfo) NULL)
            {
              for (i=0; i < second->event_count; i++, j++)
                events[j]=second->events[i];
            }
        }
    }
  UnlockSemaphoreInfo(first->events_semaphore);
  if (second != (MagickCLCacheInfo) NULL)
    UnlockSemaphoreInfo(second->events_semaphore);
  return(events);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o p y M a g i c k C L C a c h e I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CopyMagickCLCacheInfo() copies the memory from the device into host memory.
%
%  The format of the CopyMagickCLCacheInfo method is:
%
%      void CopyMagickCLCacheInfo(MagickCLCacheInfo info)
%
%  A description of each parameter follows:
%
%    o info: the OpenCL cache info.
%
*/
MagickPrivate MagickCLCacheInfo CopyMagickCLCacheInfo(MagickCLCacheInfo info)
{
  cl_command_queue
    queue;

  cl_event
    *events;

  cl_uint
    event_count;

  Quantum
    *pixels;

  if (info == (MagickCLCacheInfo) NULL)
    return((MagickCLCacheInfo) NULL);
  events=CopyOpenCLEvents(info,(MagickCLCacheInfo) NULL,&event_count);
  if (events != (cl_event *) NULL)
    {
      queue=AcquireOpenCLCommandQueue(info->device);
      pixels=(Quantum *) openCL_library->clEnqueueMapBuffer(queue,info->buffer,
        CL_TRUE,CL_MAP_READ | CL_MAP_WRITE,0,info->length,event_count,events,
        (cl_event *) NULL,(cl_int *) NULL);
      assert(pixels == info->pixels);
      ReleaseOpenCLCommandQueue(info->device,queue);
      events=(cl_event *) RelinquishMagickMemory(events);
    }
  return(RelinquishMagickCLCacheInfo(info,MagickFalse));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D u m p O p e n C L P r o f i l e D a t a                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DumpOpenCLProfileData() dumps the kernel profile data.
%
%  The format of the DumpProfileData method is:
%
%      void DumpProfileData()
%
*/

MagickPrivate void DumpOpenCLProfileData()
{
#define OpenCLLog(message) \
   fwrite(message,sizeof(char),strlen(message),log); \
   fwrite("\n",sizeof(char),1,log);

  char
    buf[4096],
    filename[MagickPathExtent],
    indent[160];

  FILE
    *log;

  size_t
    i,
    j;

  if (default_CLEnv == (MagickCLEnv) NULL)
    return;

  for (i = 0; i < default_CLEnv->number_devices; i++)
    if (default_CLEnv->devices[i]->profile_kernels != MagickFalse)
      break;
  if (i == default_CLEnv->number_devices)
    return;

  (void) FormatLocaleString(filename,MagickPathExtent,"%s%s%s",
    GetOpenCLCacheDirectory(),DirectorySeparator,"ImageMagickOpenCL.log");

  log=fopen_utf8(filename,"wb");
  if (log == (FILE *) NULL)
    return;
  for (i = 0; i < default_CLEnv->number_devices; i++)
  {
    MagickCLDevice
      device;

    device=default_CLEnv->devices[i];
    if ((device->profile_kernels == MagickFalse) ||
        (device->profile_records == (KernelProfileRecord *) NULL))
      continue;

    OpenCLLog("====================================================");
    fprintf(log,"Device:  %s\n",device->name);
    fprintf(log,"Version: %s\n",device->version);
    OpenCLLog("====================================================");
    OpenCLLog("                     average   calls     min     max");
    OpenCLLog("                     -------   -----     ---     ---");
    j=0;
    while (device->profile_records[j] != (KernelProfileRecord) NULL)
    {
      KernelProfileRecord
        profile;

      profile=device->profile_records[j];
      (void) CopyMagickString(indent,"                              ",
        sizeof(indent));
      CopyMagickString(indent,profile->kernel_name,MagickMin(strlen(
        profile->kernel_name),strlen(indent)));
      (void) FormatLocaleString(buf,sizeof(buf),"%s %7d %7d %7d %7d",indent,
        (int) (profile->total/profile->count),(int) profile->count,
        (int) profile->min,(int) profile->max);
      OpenCLLog(buf);
      j++;
    }
    OpenCLLog("====================================================");
    fwrite("\n\n",sizeof(char),2,log);
  }
  fclose(log);
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E n q u e u e O p e n C L K e r n e l                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EnqueueOpenCLKernel() enques the specified kernel and registers the OpenCL
%  events with the images.
%
%  The format of the EnqueueOpenCLKernel method is:
%
%      MagickBooleanType EnqueueOpenCLKernel(cl_kernel kernel,cl_uint work_dim,
%        const size_t *global_work_offset,const size_t *global_work_size,
%        const size_t *local_work_size,const Image *input_image,
%        const Image *output_image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o kernel: the OpenCL kernel.
%
%    o work_dim: the number of dimensions used to specify the global work-items
%                and work-items in the work-group.
%
%    o offset: can be used to specify an array of work_dim unsigned values
%              that describe the offset used to calculate the global ID of a
%              work-item.
%
%    o gsize: points to an array of work_dim unsigned values that describe the
%             number of global work-items in work_dim dimensions that will
%             execute the kernel function.
%
%    o lsize: points to an array of work_dim unsigned values that describe the
%             number of work-items that make up a work-group that will execute
%             the kernel specified by kernel.
%
%    o input_image: the input image of the operation.
%
%    o output_image: the output or secondary image of the operation.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType RegisterCacheEvent(MagickCLCacheInfo info,
  cl_event event)
{
  assert(info != (MagickCLCacheInfo) NULL);
  assert(event != (cl_event) NULL);
  if (openCL_library->clRetainEvent(event) != CL_SUCCESS)
    {
      openCL_library->clWaitForEvents(1,&event);
      return(MagickFalse);
    }
  LockSemaphoreInfo(info->events_semaphore);
  if (info->events == (cl_event *) NULL)
    {
      info->events=(cl_event *) AcquireMagickMemory(sizeof(*info->events));
      info->event_count=1;
    }
  else
    info->events=(cl_event *) ResizeQuantumMemory(info->events,
      ++info->event_count,sizeof(*info->events));
  if (info->events == (cl_event *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  info->events[info->event_count-1]=event;
  UnlockSemaphoreInfo(info->events_semaphore);
  return(MagickTrue);
}

MagickPrivate MagickBooleanType EnqueueOpenCLKernel(cl_command_queue queue,
  cl_kernel kernel,cl_uint work_dim,const size_t *offset,const size_t *gsize,
  const size_t *lsize,const Image *input_image,const Image *output_image,
  MagickBooleanType flush,ExceptionInfo *exception)
{
  CacheInfo
    *output_info,
    *input_info;

  cl_event
    event,
    *events;

  cl_int
    status;

  cl_uint
    event_count;

  assert(input_image != (const Image *) NULL);
  input_info=(CacheInfo *) input_image->cache;
  assert(input_info != (CacheInfo *) NULL);
  assert(input_info->opencl != (MagickCLCacheInfo) NULL);
  output_info=(CacheInfo *) NULL;
  if (output_image == (const Image *) NULL)
    events=CopyOpenCLEvents(input_info->opencl,(MagickCLCacheInfo) NULL,
      &event_count);
  else
    {
      output_info=(CacheInfo *) output_image->cache;
      assert(output_info != (CacheInfo *) NULL);
      assert(output_info->opencl != (MagickCLCacheInfo) NULL);
      events=CopyOpenCLEvents(input_info->opencl,output_info->opencl,
        &event_count);
    }
  status=openCL_library->clEnqueueNDRangeKernel(queue,kernel,work_dim,offset,
    gsize,lsize,event_count,events,&event);
  /* This can fail due to memory issues and calling clFinish might help. */
  if ((status != CL_SUCCESS) && (event_count > 0))
    {
      openCL_library->clFinish(queue);
      status=openCL_library->clEnqueueNDRangeKernel(queue,kernel,work_dim,
        offset,gsize,lsize,event_count,events,&event);
    }
  events=(cl_event *) RelinquishMagickMemory(events);
  if (status != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(input_info->opencl->device,exception,
        GetMagickModule(),ResourceLimitWarning,
        "clEnqueueNDRangeKernel failed.","'%s'",".");
      return(MagickFalse);
    }
  if (flush != MagickFalse)
    openCL_library->clFlush(queue);
  if (RecordProfileData(input_info->opencl->device,kernel,event) == MagickFalse)
    {
      if (RegisterCacheEvent(input_info->opencl,event) != MagickFalse)
        {
          if (output_info != (CacheInfo *) NULL)
            (void) RegisterCacheEvent(output_info->opencl,event);
        }
    }
  openCL_library->clReleaseEvent(event);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t C u r r e n t O p e n C L E n v                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCurrentOpenCLEnv() returns the current OpenCL env
%
%  The format of the GetCurrentOpenCLEnv method is:
%
%      MagickCLEnv GetCurrentOpenCLEnv()
%
*/

MagickPrivate MagickCLEnv GetCurrentOpenCLEnv(void)
{
  if (default_CLEnv != (MagickCLEnv) NULL)
  {
    if ((default_CLEnv->benchmark_thread_id != (MagickThreadType) 0) &&
        (default_CLEnv->benchmark_thread_id != GetMagickThreadId()))
      return((MagickCLEnv) NULL);
    else
      return(default_CLEnv);
  }

  if (GetOpenCLCacheDirectory() == (char *) NULL)
    return((MagickCLEnv) NULL);

  if (openCL_lock == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&openCL_lock);

  LockSemaphoreInfo(openCL_lock);
  if (default_CLEnv == (MagickCLEnv) NULL)
    default_CLEnv=AcquireMagickCLEnv();
  UnlockSemaphoreInfo(openCL_lock);

  return(default_CLEnv);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O p e n C L D e v i c e B e n c h m a r k D u r a t i o n           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLDeviceBenchmarkScore() returns the score of the benchmark for the
%  device. The score is determined by the duration of the micro benchmark so
%  that means a lower score is better than a higher score.
%
%  The format of the GetOpenCLDeviceBenchmarkScore method is:
%
%      double GetOpenCLDeviceBenchmarkScore(const MagickCLDevice device)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
*/

MagickExport double GetOpenCLDeviceBenchmarkScore(
  const MagickCLDevice device)
{
  if (device == (MagickCLDevice) NULL)
    return(MAGICKCORE_OPENCL_UNDEFINED_SCORE);
  return(device->score);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O p e n C L D e v i c e E n a b l e d                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLDeviceEnabled() returns true if the device is enabled.
%
%  The format of the GetOpenCLDeviceEnabled method is:
%
%      MagickBooleanType GetOpenCLDeviceEnabled(const MagickCLDevice device)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
*/

MagickExport MagickBooleanType GetOpenCLDeviceEnabled(
  const MagickCLDevice device)
{
  if (device == (MagickCLDevice) NULL)
    return(MagickFalse);
  return(device->enabled);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O p e n C L D e v i c e N a m e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLDeviceName() returns the name of the device.
%
%  The format of the GetOpenCLDeviceName method is:
%
%      const char *GetOpenCLDeviceName(const MagickCLDevice device)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
*/

MagickExport const char *GetOpenCLDeviceName(const MagickCLDevice device)
{
  if (device == (MagickCLDevice) NULL)
    return((const char *) NULL);
  return(device->name);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O p e n C L D e v i c e V e n d o r N a m e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLDeviceVendorName() returns the vendor name of the device.
%
%  The format of the GetOpenCLDeviceVendorName method is:
%
%      const char *GetOpenCLDeviceVendorName(const MagickCLDevice device)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
*/

MagickExport const char *GetOpenCLDeviceVendorName(const MagickCLDevice device)
{
  if (device == (MagickCLDevice) NULL)
    return((const char *) NULL);
  return(device->vendor_name);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O p e n C L D e v i c e s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLDevices() returns the devices of the OpenCL environment at sets the
%  value of length to the number of devices that are available.
%
%  The format of the GetOpenCLDevices method is:
%
%      const MagickCLDevice *GetOpenCLDevices(size_t *length,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o length: the number of device.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport MagickCLDevice *GetOpenCLDevices(size_t *length,
  ExceptionInfo *exception)
{
  MagickCLEnv
    clEnv;

  clEnv=GetCurrentOpenCLEnv();
  if (clEnv == (MagickCLEnv) NULL)
    {
      if (length != (size_t *) NULL)
        *length=0;
      return((MagickCLDevice *) NULL);
    }
  InitializeOpenCL(clEnv,exception);
  if (length != (size_t *) NULL)
    *length=clEnv->number_devices;
  return(clEnv->devices);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O p e n C L D e v i c e T y p e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLDeviceType() returns the type of the device.
%
%  The format of the GetOpenCLDeviceType method is:
%
%      MagickCLDeviceType GetOpenCLDeviceType(const MagickCLDevice device)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
*/

MagickExport MagickCLDeviceType GetOpenCLDeviceType(
  const MagickCLDevice device)
{
  if (device == (MagickCLDevice) NULL)
    return(UndefinedCLDeviceType);
  if (device->type == CL_DEVICE_TYPE_GPU)
    return(GpuCLDeviceType);
  if (device->type == CL_DEVICE_TYPE_CPU)
    return(CpuCLDeviceType);
  return(UndefinedCLDeviceType);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O p e n C L D e v i c e V e r s i o n                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLDeviceVersion() returns the version of the device.
%
%  The format of the GetOpenCLDeviceName method is:
%
%      const char *GetOpenCLDeviceVersion(MagickCLDevice device)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
*/

MagickExport const char *GetOpenCLDeviceVersion(const MagickCLDevice device)
{
  if (device == (MagickCLDevice) NULL)
    return((const char *) NULL);
  return(device->version);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O p e n C L E n a b l e d                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLEnabled() returns true if OpenCL acceleration is enabled.
%
%  The format of the GetOpenCLEnabled method is:
%
%      MagickBooleanType GetOpenCLEnabled()
%
*/

MagickExport MagickBooleanType GetOpenCLEnabled(void)
{
  MagickCLEnv
    clEnv;

  clEnv=GetCurrentOpenCLEnv();
  if (clEnv == (MagickCLEnv) NULL)
    return(MagickFalse);
  return(clEnv->enabled);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O p e n C L K e r n e l P r o f i l e R e c o r d s                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLKernelProfileRecords() returns the profile records for the
%  specified device and sets length to the number of profile records.
%
%  The format of the GetOpenCLKernelProfileRecords method is:
%
%      const KernelProfileRecord *GetOpenCLKernelProfileRecords(size *length)
%
%  A description of each parameter follows:
%
%    o length: the number of profiles records.
*/

MagickExport const KernelProfileRecord *GetOpenCLKernelProfileRecords(
  const MagickCLDevice device,size_t *length)
{
  if ((device == (const MagickCLDevice) NULL) || (device->profile_records ==
      (KernelProfileRecord *) NULL))
  {
    if (length != (size_t *) NULL)
      *length=0;
    return((const KernelProfileRecord *) NULL);
  }
  if (length != (size_t *) NULL)
    {
      *length=0;
      LockSemaphoreInfo(device->lock);
      while (device->profile_records[*length] != (KernelProfileRecord) NULL)
        *length=*length+1;
      UnlockSemaphoreInfo(device->lock);
    }
  return(device->profile_records);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H a s O p e n C L D e v i c e s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  HasOpenCLDevices() checks if the OpenCL environment has devices that are
%  enabled and compiles the kernel for the device when necessary. False will be
%  returned if no enabled devices could be found
%
%  The format of the HasOpenCLDevices method is:
%
%    MagickBooleanType HasOpenCLDevices(MagickCLEnv clEnv,
%      ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType HasOpenCLDevices(MagickCLEnv clEnv,
  ExceptionInfo *exception)
{
  char
    *accelerateKernelsBuffer,
    options[MagickPathExtent];

  MagickBooleanType
    status;

  size_t
    i;

  size_t
    signature;

  /* Check if there are enabled devices */
  for (i = 0; i < clEnv->number_devices; i++)
  {
    if ((clEnv->devices[i]->enabled != MagickFalse))
      break;
  }
  if (i == clEnv->number_devices)
    return(MagickFalse);

  /* Check if we need to compile a kernel for one of the devices */
  status=MagickTrue;
  for (i = 0; i < clEnv->number_devices; i++)
  {
    if ((clEnv->devices[i]->enabled != MagickFalse) &&
        (clEnv->devices[i]->program == (cl_program) NULL))
    {
      status=MagickFalse;
      break;
    }
  }
  if (status != MagickFalse)
    return(MagickTrue);

  /* Get additional options */
  (void) FormatLocaleString(options,MagickPathExtent,CLOptions,
    (float)QuantumRange,(float)QuantumScale,(float)CLCharQuantumScale,
    (float)MagickEpsilon,(float)MagickPI,(unsigned int)MaxMap,
    (unsigned int)MAGICKCORE_QUANTUM_DEPTH);

  signature=StringSignature(options);
  accelerateKernelsBuffer=(char*) AcquireQuantumMemory(1,
    strlen(accelerateKernels)+strlen(accelerateKernels2)+1);
  if (accelerateKernelsBuffer == (char*) NULL)
    return(MagickFalse);
  (void) FormatLocaleString(accelerateKernelsBuffer,strlen(accelerateKernels)+
    strlen(accelerateKernels2)+1,"%s%s",accelerateKernels,accelerateKernels2);
  signature^=StringSignature(accelerateKernelsBuffer);

  status=MagickTrue;
  for (i = 0; i < clEnv->number_devices; i++)
  {
    MagickCLDevice
      device;

    size_t
      device_signature;

    device=clEnv->devices[i];
    if ((device->enabled == MagickFalse) ||
        (device->program != (cl_program) NULL))
      continue;

    LockSemaphoreInfo(device->lock);
    if (device->program != (cl_program) NULL)
    {
      UnlockSemaphoreInfo(device->lock);
      continue;
    }
    device_signature=signature;
    device_signature^=StringSignature(device->platform_name);
    status=CompileOpenCLKernel(device,accelerateKernelsBuffer,options,
      device_signature,exception);
    UnlockSemaphoreInfo(device->lock);
    if (status == MagickFalse)
      break;
  }
  accelerateKernelsBuffer=(char *) RelinquishMagickMemory(
    accelerateKernelsBuffer);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e O p e n C L                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeOpenCL() is used to initialize the OpenCL environment. This method
%  makes sure the devices are properly initialized and benchmarked.
%
%  The format of the InitializeOpenCL method is:
%
%    MagickBooleanType InitializeOpenCL(ExceptionInfo exception)
%
%  A description of each parameter follows:
%
%    o exception: return any errors or warnings in this structure.
%
*/

static cl_uint GetOpenCLDeviceCount(MagickCLEnv clEnv,cl_platform_id platform)
{
  char
    version[MagickPathExtent];

  cl_uint
    num;

  if (clEnv->library->clGetPlatformInfo(platform,CL_PLATFORM_VERSION,
        MagickPathExtent,version,NULL) != CL_SUCCESS)
    return(0);
  if (strncmp(version,"OpenCL 1.0 ",11) == 0)
    return(0);
  if (clEnv->library->clGetDeviceIDs(platform,
        CL_DEVICE_TYPE_CPU|CL_DEVICE_TYPE_GPU,0,NULL,&num) != CL_SUCCESS)
    return(0);
  return(num);
}

static inline char *GetOpenCLPlatformString(cl_platform_id platform,
  cl_platform_info param_name)
{
  char
    *value;

  size_t
    length;

  openCL_library->clGetPlatformInfo(platform,param_name,0,NULL,&length);
  value=(char *) AcquireCriticalMemory(length*sizeof(*value));
  openCL_library->clGetPlatformInfo(platform,param_name,length,value,NULL);
  return(value);
}

static inline char *GetOpenCLDeviceString(cl_device_id device,
  cl_device_info param_name)
{
  char
    *value;

  size_t
    length;

  openCL_library->clGetDeviceInfo(device,param_name,0,NULL,&length);
  value=(char *) AcquireCriticalMemory(length*sizeof(*value));
  openCL_library->clGetDeviceInfo(device,param_name,length,value,NULL);
  return(value);
}

static void LoadOpenCLDevices(MagickCLEnv clEnv)
{
  cl_context_properties
    properties[3];

  cl_device_id
    *devices;

  cl_int
    status;

  cl_platform_id
    *platforms;

  cl_uint
    i,
    j,
    next,
    number_devices,
    number_platforms;

  number_platforms=0;
  if (openCL_library->clGetPlatformIDs(0,NULL,&number_platforms) != CL_SUCCESS)
    return;
  if (number_platforms == 0)
    return;
  platforms=(cl_platform_id *) AcquireQuantumMemory(1,number_platforms*
    sizeof(cl_platform_id));
  if (platforms == (cl_platform_id *) NULL)
    return;
  if (openCL_library->clGetPlatformIDs(number_platforms,platforms,NULL) != CL_SUCCESS)
    {
       platforms=(cl_platform_id *) RelinquishMagickMemory(platforms);
       return;
    }
  for (i = 0; i < number_platforms; i++)
  {
    number_devices=GetOpenCLDeviceCount(clEnv,platforms[i]);
    if (number_devices == 0)
      platforms[i]=(cl_platform_id) NULL;
    else
      clEnv->number_devices+=number_devices;
  }
  if (clEnv->number_devices == 0)
    {
      platforms=(cl_platform_id *) RelinquishMagickMemory(platforms);
      return;
    }
  clEnv->devices=(MagickCLDevice *) AcquireQuantumMemory(clEnv->number_devices,
    sizeof(MagickCLDevice));
  if (clEnv->devices == (MagickCLDevice *) NULL)
    {
      RelinquishMagickCLDevices(clEnv);
      platforms=(cl_platform_id *) RelinquishMagickMemory(platforms);
      return;
    }
  (void) memset(clEnv->devices,0,clEnv->number_devices*sizeof(MagickCLDevice));
  devices=(cl_device_id *) AcquireQuantumMemory(clEnv->number_devices,
    sizeof(cl_device_id));
  if (devices == (cl_device_id *) NULL)
    {
      platforms=(cl_platform_id *) RelinquishMagickMemory(platforms);
      RelinquishMagickCLDevices(clEnv);
      return;
    }
  (void) memset(devices,0,clEnv->number_devices*sizeof(cl_device_id));
  clEnv->number_contexts=(size_t) number_platforms;
  clEnv->contexts=(cl_context *) AcquireQuantumMemory(clEnv->number_contexts,
    sizeof(cl_context));
  if (clEnv->contexts == (cl_context *) NULL)
    {
      devices=(cl_device_id *) RelinquishMagickMemory(devices);
      platforms=(cl_platform_id *) RelinquishMagickMemory(platforms);
      RelinquishMagickCLDevices(clEnv);
      return;
    }
  (void) memset(clEnv->contexts,0,clEnv->number_contexts*sizeof(cl_context));
  next=0;
  for (i = 0; i < number_platforms; i++)
  {
    if (platforms[i] == (cl_platform_id) NULL)
      continue;

    status=clEnv->library->clGetDeviceIDs(platforms[i],CL_DEVICE_TYPE_CPU |
      CL_DEVICE_TYPE_GPU,(cl_uint) clEnv->number_devices,devices,&number_devices);
    if (status != CL_SUCCESS)
      continue;

    properties[0]=CL_CONTEXT_PLATFORM;
    properties[1]=(cl_context_properties) platforms[i];
    properties[2]=0;
    clEnv->contexts[i]=openCL_library->clCreateContext(properties,number_devices,
      devices,NULL,NULL,&status);
    if (status != CL_SUCCESS)
      continue;

    for (j = 0; j < number_devices; j++,next++)
    {
      MagickCLDevice
        device;

      device=AcquireMagickCLDevice();
      if (device == (MagickCLDevice) NULL)
        break;

      device->context=clEnv->contexts[i];
      device->deviceID=devices[j];

      device->platform_name=GetOpenCLPlatformString(platforms[i],
        CL_PLATFORM_NAME);

      device->vendor_name=GetOpenCLPlatformString(platforms[i],
        CL_PLATFORM_VENDOR);

      device->name=GetOpenCLDeviceString(devices[j],CL_DEVICE_NAME);

      device->version=GetOpenCLDeviceString(devices[j],CL_DRIVER_VERSION);

      openCL_library->clGetDeviceInfo(devices[j],CL_DEVICE_MAX_CLOCK_FREQUENCY,
        sizeof(cl_uint),&device->max_clock_frequency,NULL);

      openCL_library->clGetDeviceInfo(devices[j],CL_DEVICE_MAX_COMPUTE_UNITS,
        sizeof(cl_uint),&device->max_compute_units,NULL);

      openCL_library->clGetDeviceInfo(devices[j],CL_DEVICE_TYPE,
        sizeof(cl_device_type),&device->type,NULL);

      openCL_library->clGetDeviceInfo(devices[j],CL_DEVICE_LOCAL_MEM_SIZE,
        sizeof(cl_ulong),&device->local_memory_size,NULL);

      clEnv->devices[next]=device;
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
        "Found device: %s (%s)",device->name,device->platform_name);
    }
  }
  if (next != clEnv->number_devices)
    RelinquishMagickCLDevices(clEnv);
  platforms=(cl_platform_id *) RelinquishMagickMemory(platforms);
  devices=(cl_device_id *) RelinquishMagickMemory(devices);
}

MagickPrivate MagickBooleanType InitializeOpenCL(MagickCLEnv clEnv,
  ExceptionInfo *exception)
{
  LockSemaphoreInfo(clEnv->lock);
  if (clEnv->initialized != MagickFalse)
    {
      UnlockSemaphoreInfo(clEnv->lock);
      return(HasOpenCLDevices(clEnv,exception));
    }
  if (LoadOpenCLLibrary() != MagickFalse)
    {
      clEnv->library=openCL_library;
      LoadOpenCLDevices(clEnv);
      if (clEnv->number_devices > 0)
        AutoSelectOpenCLDevices(clEnv);
    }
  clEnv->initialized=MagickTrue;
  UnlockSemaphoreInfo(clEnv->lock);
  return(HasOpenCLDevices(clEnv,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o a d O p e n C L L i b r a r y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadOpenCLLibrary() load and binds the OpenCL library.
%
%  The format of the LoadOpenCLLibrary method is:
%
%    MagickBooleanType LoadOpenCLLibrary(void)
%
*/

void *OsLibraryGetFunctionAddress(void *library,const char *functionName)
{
  if ((library == (void *) NULL) || (functionName == (const char *) NULL))
    return (void *) NULL;
  return lt_dlsym(library,functionName);
}

static MagickBooleanType BindOpenCLFunctions()
{
#ifdef MAGICKCORE_HAVE_OPENCL_CL_H
#define BIND(X) openCL_library->X= &X;
#else
  (void) memset(openCL_library,0,sizeof(MagickLibrary));
#ifdef MAGICKCORE_WINDOWS_SUPPORT
  openCL_library->library=(void *)lt_dlopen("OpenCL.dll");
#else
  openCL_library->library=(void *)lt_dlopen("libOpenCL.so");
#endif
#define BIND(X) \
  if ((openCL_library->X=(MAGICKpfn_##X)OsLibraryGetFunctionAddress(openCL_library->library,#X)) == NULL) \
    return(MagickFalse);
#endif

  if (openCL_library->library == (void*) NULL)
    return(MagickFalse);

  BIND(clGetPlatformIDs);
  BIND(clGetPlatformInfo);

  BIND(clGetDeviceIDs);
  BIND(clGetDeviceInfo);

  BIND(clCreateBuffer);
  BIND(clReleaseMemObject);
  BIND(clRetainMemObject);

  BIND(clCreateContext);
  BIND(clReleaseContext);

  BIND(clCreateCommandQueue);
  BIND(clReleaseCommandQueue);
  BIND(clFlush);
  BIND(clFinish);

  BIND(clCreateProgramWithSource);
  BIND(clCreateProgramWithBinary);
  BIND(clReleaseProgram);
  BIND(clBuildProgram);
  BIND(clGetProgramBuildInfo);
  BIND(clGetProgramInfo);

  BIND(clCreateKernel);
  BIND(clReleaseKernel);
  BIND(clSetKernelArg);
  BIND(clGetKernelInfo);

  BIND(clEnqueueReadBuffer);
  BIND(clEnqueueMapBuffer);
  BIND(clEnqueueUnmapMemObject);
  BIND(clEnqueueNDRangeKernel);

  BIND(clGetEventInfo);
  BIND(clWaitForEvents);
  BIND(clReleaseEvent);
  BIND(clRetainEvent);
  BIND(clSetEventCallback);

  BIND(clGetEventProfilingInfo);

  return(MagickTrue);
}

static MagickBooleanType LoadOpenCLLibrary(void)
{
  openCL_library=(MagickLibrary *) AcquireMagickMemory(sizeof(MagickLibrary));
  if (openCL_library == (MagickLibrary *) NULL)
    return(MagickFalse);

  if (BindOpenCLFunctions() == MagickFalse)
    {
      openCL_library=(MagickLibrary *)RelinquishMagickMemory(openCL_library);
      return(MagickFalse);
    }

  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   O p e n C L T e r m i n u s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenCLTerminus() destroys the OpenCL component.
%
%  The format of the OpenCLTerminus method is:
%
%      OpenCLTerminus(void)
%
*/

MagickPrivate void OpenCLTerminus()
{
  DumpOpenCLProfileData();
  if (cache_directory != (char *) NULL)
    cache_directory=DestroyString(cache_directory);
  if (cache_directory_lock != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&cache_directory_lock);
  if (default_CLEnv != (MagickCLEnv) NULL)
    default_CLEnv=RelinquishMagickCLEnv(default_CLEnv);
  if (openCL_lock != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&openCL_lock);
  if (openCL_library != (MagickLibrary *) NULL)
    {
      if (openCL_library->library != (void *) NULL)
#if defined(MAGICKCORE_DLOPEN)
        (void) dlclose(openCL_library->library);
#else
        (void) lt_dlclose(openCL_library->library);
#endif
      openCL_library=(MagickLibrary *) RelinquishMagickMemory(openCL_library);
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   O p e n C L T h r o w M a g i c k E x c e p t i o n                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenCLThrowMagickException logs an OpenCL exception as determined by the log
%  configuration file.  If an error occurs, MagickFalse is returned
%  otherwise MagickTrue.
%
%  The format of the OpenCLThrowMagickException method is:
%
%      MagickBooleanType OpenCLThrowMagickException(ExceptionInfo *exception,
%        const char *module,const char *function,const size_t line,
%        const ExceptionType severity,const char *tag,const char *format,...)
%
%  A description of each parameter follows:
%
%    o exception: the exception info.
%
%    o filename: the source module filename.
%
%    o function: the function name.
%
%    o line: the line number of the source module.
%
%    o severity: Specifies the numeric error category.
%
%    o tag: the locale tag.
%
%    o format: the output format.
%
*/

MagickPrivate MagickBooleanType OpenCLThrowMagickException(
  MagickCLDevice device,ExceptionInfo *exception,const char *module,
  const char *function,const size_t line,const ExceptionType severity,
  const char *tag,const char *format,...)
{
  MagickBooleanType
    status;

  assert(device != (MagickCLDevice) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  (void) exception;
  status=MagickTrue;
  if (severity != 0)
  {
    if (device->type == CL_DEVICE_TYPE_CPU)
    {
      /* Workaround for Intel OpenCL CPU runtime bug */
      /* Turn off OpenCL when a problem is detected! */
      if (strncmp(device->platform_name,"Intel",5) == 0)
        default_CLEnv->enabled=MagickFalse;
    }
  }

#ifdef OPENCLLOG_ENABLED
  {
    va_list
      operands;
    va_start(operands,format);
    status=ThrowMagickExceptionList(exception,module,function,line,severity,tag,
      format,operands);
    va_end(operands);
  }
#else
  magick_unreferenced(module);
  magick_unreferenced(function);
  magick_unreferenced(line);
  magick_unreferenced(tag);
  magick_unreferenced(format);
#endif

  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e c o r d P r o f i l e D a t a                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RecordProfileData() records profile data.
%
%  The format of the RecordProfileData method is:
%
%      void RecordProfileData(MagickCLDevice device,ProfiledKernels kernel,
%        cl_event event)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device that did the operation.
%
%    o event: the event that contains the profiling data.
%
*/

MagickPrivate MagickBooleanType RecordProfileData(MagickCLDevice device,
  cl_kernel kernel,cl_event event)
{
  char
    *name;

  cl_int
    status;

  cl_ulong
    elapsed,
    end,
    start;

  KernelProfileRecord
    profile_record;

  size_t
    i,
    length;

  if (device->profile_kernels == MagickFalse)
    return(MagickFalse);
  status=openCL_library->clWaitForEvents(1,&event);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  status=openCL_library->clGetKernelInfo(kernel,CL_KERNEL_FUNCTION_NAME,0,NULL,
    &length);
  if (status != CL_SUCCESS)
    return(MagickTrue);
  name=(char *) AcquireQuantumMemory(length,sizeof(*name));
  if (name == (char *) NULL)
    return(MagickTrue);
  start=end=elapsed=0;
  status=openCL_library->clGetKernelInfo(kernel,CL_KERNEL_FUNCTION_NAME,length,
    name,(size_t *) NULL);
  status|=openCL_library->clGetEventProfilingInfo(event,
    CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&start,NULL);
  status|=openCL_library->clGetEventProfilingInfo(event,
    CL_PROFILING_COMMAND_END,sizeof(cl_ulong),&end,NULL);
  if (status != CL_SUCCESS)
    {
      name=DestroyString(name);
      return(MagickTrue);
    }
  start/=1000; /* usecs */
  end/=1000;   
  elapsed=end-start;
  LockSemaphoreInfo(device->lock);
  i=0;
  profile_record=(KernelProfileRecord) NULL;
  if (device->profile_records != (KernelProfileRecord *) NULL)
    {
      while (device->profile_records[i] != (KernelProfileRecord) NULL)
      {
        if (LocaleCompare(device->profile_records[i]->kernel_name,name) == 0)
          {
            profile_record=device->profile_records[i];
            break;
          }
        i++;
      }
    }
  if (profile_record != (KernelProfileRecord) NULL)
    name=DestroyString(name);
  else
    {
      profile_record=(KernelProfileRecord) AcquireCriticalMemory(
        sizeof(*profile_record));
      (void) memset(profile_record,0,sizeof(*profile_record));
      profile_record->kernel_name=name;
      device->profile_records=(KernelProfileRecord *) ResizeQuantumMemory(
        device->profile_records,(i+2),sizeof(*device->profile_records));
      if (device->profile_records == (KernelProfileRecord *) NULL)
        ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
      device->profile_records[i]=profile_record;
      device->profile_records[i+1]=(KernelProfileRecord) NULL;
    }
  if ((elapsed < profile_record->min) || (profile_record->count == 0))
    profile_record->min=elapsed;
  if (elapsed > profile_record->max)
    profile_record->max=elapsed;
  profile_record->total+=elapsed;
  profile_record->count+=1;
  UnlockSemaphoreInfo(device->lock);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e l e a s e O p e n C L C o m m a n d Q u e u e                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReleaseOpenCLCommandQueue() releases the OpenCL command queue
%
%  The format of the ReleaseOpenCLCommandQueue method is:
%
%      void ReleaseOpenCLCommandQueue(MagickCLDevice device,
%        cl_command_queue queue)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
%
%    o queue: the OpenCL queue to be released.
*/

MagickPrivate void ReleaseOpenCLCommandQueue(MagickCLDevice device,
  cl_command_queue queue)
{
  if (queue == (cl_command_queue) NULL)
    return;

  assert(device != (MagickCLDevice) NULL);
  LockSemaphoreInfo(device->lock);
  if ((device->profile_kernels != MagickFalse) ||
      (device->command_queues_index >= MAGICKCORE_OPENCL_COMMAND_QUEUES-1))
    {
      UnlockSemaphoreInfo(device->lock);
      openCL_library->clFinish(queue);
      (void) openCL_library->clReleaseCommandQueue(queue);
    }
  else
    {
      openCL_library->clFlush(queue);
      device->command_queues[++device->command_queues_index]=queue;
      UnlockSemaphoreInfo(device->lock);
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e l e a s e  M a g i c k C L D e v i c e                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReleaseOpenCLDevice() returns the OpenCL device to the environment
%
%  The format of the ReleaseOpenCLDevice method is:
%
%      void ReleaseOpenCLDevice(MagickCLDevice device)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device to be released.
%
*/

MagickPrivate void ReleaseOpenCLDevice(MagickCLDevice device)
{
  assert(device != (MagickCLDevice) NULL);
  LockSemaphoreInfo(openCL_lock);
  device->requested--;
  UnlockSemaphoreInfo(openCL_lock);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e l i n q u i s h M a g i c k C L C a c h e I n f o                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishMagickCLCacheInfo() frees memory acquired with
%  AcquireMagickCLCacheInfo()
%
%  The format of the RelinquishMagickCLCacheInfo method is:
%
%      MagickCLCacheInfo RelinquishMagickCLCacheInfo(MagickCLCacheInfo info,
%        const MagickBooleanType relinquish_pixels)
%
%  A description of each parameter follows:
%
%    o info: the OpenCL cache info.
%
%    o relinquish_pixels: the pixels will be relinquish when set to true.
%
*/

static void CL_API_CALL DestroyMagickCLCacheInfoAndPixels(
  cl_event magick_unused(event),
  cl_int magick_unused(event_command_exec_status),void *user_data)
{
  MagickCLCacheInfo
    info;

  Quantum
    *pixels;

  ssize_t
    i;

  magick_unreferenced(event);
  magick_unreferenced(event_command_exec_status);
  info=(MagickCLCacheInfo) user_data;
  for (i=(ssize_t)info->event_count-1; i >= 0; i--)
  {
    cl_int
      event_status;

    cl_uint
      status;

    status=openCL_library->clGetEventInfo(info->events[i],
      CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(event_status),&event_status,
      NULL);
    if ((status == CL_SUCCESS) && (event_status > CL_COMPLETE))
      {
        openCL_library->clSetEventCallback(info->events[i],CL_COMPLETE,
          &DestroyMagickCLCacheInfoAndPixels,info);
        return;
      }
  }
  pixels=info->pixels;
  RelinquishMagickResource(MemoryResource,info->length);
  DestroyMagickCLCacheInfo(info);
  (void) RelinquishAlignedMemory(pixels);
}

MagickPrivate MagickCLCacheInfo RelinquishMagickCLCacheInfo(
  MagickCLCacheInfo info,const MagickBooleanType relinquish_pixels)
{
  if (info == (MagickCLCacheInfo) NULL)
    return((MagickCLCacheInfo) NULL);
  if (relinquish_pixels != MagickFalse)
    DestroyMagickCLCacheInfoAndPixels((cl_event) NULL,0,info);
  else
    DestroyMagickCLCacheInfo(info);
  return((MagickCLCacheInfo) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h M a g i c k C L D e v i c e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishMagickCLDevice() releases the OpenCL device
%
%  The format of the RelinquishMagickCLDevice method is:
%
%      MagickCLDevice RelinquishMagickCLDevice(MagickCLDevice device)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device to be released.
%
*/

static MagickCLDevice RelinquishMagickCLDevice(MagickCLDevice device)
{
  if (device == (MagickCLDevice) NULL)
    return((MagickCLDevice) NULL);

  device->platform_name=(char *) RelinquishMagickMemory(device->platform_name);
  device->vendor_name=(char *) RelinquishMagickMemory(device->vendor_name);
  device->name=(char *) RelinquishMagickMemory(device->name);
  device->version=(char *) RelinquishMagickMemory(device->version);
  if (device->program != (cl_program) NULL)
    (void) openCL_library->clReleaseProgram(device->program);
  while (device->command_queues_index >= 0)
    (void) openCL_library->clReleaseCommandQueue(
      device->command_queues[device->command_queues_index--]);
  RelinquishSemaphoreInfo(&device->lock);
  return((MagickCLDevice) RelinquishMagickMemory(device));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h M a g i c k C L E n v                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishMagickCLEnv() releases the OpenCL environment
%
%  The format of the RelinquishMagickCLEnv method is:
%
%      MagickCLEnv RelinquishMagickCLEnv(MagickCLEnv device)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment to be released.
%
*/

static MagickCLEnv RelinquishMagickCLEnv(MagickCLEnv clEnv)
{
  if (clEnv == (MagickCLEnv) NULL)
    return((MagickCLEnv) NULL);

  RelinquishSemaphoreInfo(&clEnv->lock);
  RelinquishMagickCLDevices(clEnv);
  if (clEnv->contexts != (cl_context *) NULL)
    {
      ssize_t
        i;

      for (i=0; i < clEnv->number_contexts; i++)
        if (clEnv->contexts[i] != (cl_context) NULL)
          (void) openCL_library->clReleaseContext(clEnv->contexts[i]);
      clEnv->contexts=(cl_context *) RelinquishMagickMemory(clEnv->contexts);
    }
  return((MagickCLEnv) RelinquishMagickMemory(clEnv));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e q u e s t O p e n C L D e v i c e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RequestOpenCLDevice() returns one of the enabled OpenCL devices.
%
%  The format of the RequestOpenCLDevice method is:
%
%      MagickCLDevice RequestOpenCLDevice(MagickCLEnv clEnv)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
*/

MagickPrivate MagickCLDevice RequestOpenCLDevice(MagickCLEnv clEnv)
{
  MagickCLDevice
    device;

  double
    score,
    best_score;

  size_t
    i;

  if (clEnv == (MagickCLEnv) NULL)
    return((MagickCLDevice) NULL);

  if (clEnv->number_devices == 1)
  {
    if (clEnv->devices[0]->enabled)
      return(clEnv->devices[0]);
    else
      return((MagickCLDevice) NULL);
  }

  device=(MagickCLDevice) NULL;
  best_score=0.0;
  LockSemaphoreInfo(openCL_lock);
  for (i = 0; i < clEnv->number_devices; i++)
  {
    if (clEnv->devices[i]->enabled == MagickFalse)
      continue;

    score=clEnv->devices[i]->score+(clEnv->devices[i]->score*
      clEnv->devices[i]->requested);
    if ((device == (MagickCLDevice) NULL) || (score < best_score))
    {
      device=clEnv->devices[i];
      best_score=score;
    }
  }
  if (device != (MagickCLDevice)NULL)
    device->requested++;
  UnlockSemaphoreInfo(openCL_lock);

  return(device);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t O p e n C L D e v i c e E n a b l e d                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetOpenCLDeviceEnabled() can be used to enable or disabled the device.
%
%  The format of the SetOpenCLDeviceEnabled method is:
%
%      void SetOpenCLDeviceEnabled(MagickCLDevice device,
%        MagickBooleanType value)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
%
%    o value: determines if the device should be enabled or disabled.
*/

MagickExport void SetOpenCLDeviceEnabled(MagickCLDevice device,
  const MagickBooleanType value)
{
  if (device == (MagickCLDevice) NULL)
    return;
  device->enabled=value;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t O p e n C L K e r n e l P r o f i l e E n a b l e d                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetOpenCLKernelProfileEnabled() can be used to enable or disabled the
%  kernel profiling of a device.
%
%  The format of the SetOpenCLKernelProfileEnabled method is:
%
%      void SetOpenCLKernelProfileEnabled(MagickCLDevice device,
%        MagickBooleanType value)
%
%  A description of each parameter follows:
%
%    o device: the OpenCL device.
%
%    o value: determines if kernel profiling for the device should be enabled
%             or disabled.
*/

MagickExport void SetOpenCLKernelProfileEnabled(MagickCLDevice device,
  const MagickBooleanType value)
{
  if (device == (MagickCLDevice) NULL)
    return;
  device->profile_kernels=value;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t O p e n C L E n a b l e d                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetOpenCLEnabled() can be used to enable or disable OpenCL acceleration.
%
%  The format of the SetOpenCLEnabled method is:
%
%      void SetOpenCLEnabled(MagickBooleanType)
%
%  A description of each parameter follows:
%
%    o value: specify true to enable OpenCL acceleration
*/

MagickExport MagickBooleanType SetOpenCLEnabled(const MagickBooleanType value)
{
  MagickCLEnv
    clEnv;

  clEnv=GetCurrentOpenCLEnv();
  if (clEnv == (MagickCLEnv) NULL)
    return(MagickFalse);
  clEnv->enabled=value;
  return(clEnv->enabled);
}

#else

MagickExport double GetOpenCLDeviceBenchmarkScore(
  const MagickCLDevice magick_unused(device))
{
  magick_unreferenced(device);
  return(0.0);
}

MagickExport MagickBooleanType GetOpenCLDeviceEnabled(
  const MagickCLDevice magick_unused(device))
{
  magick_unreferenced(device);
  return(MagickFalse);
}

MagickExport const char *GetOpenCLDeviceName(
  const MagickCLDevice magick_unused(device))
{
  magick_unreferenced(device);
  return((const char *) NULL);
}

MagickExport MagickCLDevice *GetOpenCLDevices(size_t *length,
  ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(exception);
  if (length != (size_t *) NULL)
    *length=0;
  return((MagickCLDevice *) NULL);
}

MagickExport MagickCLDeviceType GetOpenCLDeviceType(
  const MagickCLDevice magick_unused(device))
{
  magick_unreferenced(device);
  return(UndefinedCLDeviceType);
}

MagickExport const KernelProfileRecord *GetOpenCLKernelProfileRecords(
  const MagickCLDevice magick_unused(device),size_t *length)
{
  magick_unreferenced(device);
  if (length != (size_t *) NULL)
    *length=0;
  return((const KernelProfileRecord *) NULL);
}

MagickExport const char *GetOpenCLDeviceVersion(
  const MagickCLDevice magick_unused(device))
{
  magick_unreferenced(device);
  return((const char *) NULL);
}

MagickExport MagickBooleanType GetOpenCLEnabled(void)
{
  return(MagickFalse);
}

MagickExport void SetOpenCLDeviceEnabled(
  MagickCLDevice magick_unused(device),
  const MagickBooleanType magick_unused(value))
{
  magick_unreferenced(device);
  magick_unreferenced(value);
}

MagickExport MagickBooleanType SetOpenCLEnabled(
  const MagickBooleanType magick_unused(value))
{
  magick_unreferenced(value);
  return(MagickFalse);
}

MagickExport void SetOpenCLKernelProfileEnabled(
  MagickCLDevice magick_unused(device),
  const MagickBooleanType magick_unused(value))
{
  magick_unreferenced(device);
  magick_unreferenced(value);
}
#endif

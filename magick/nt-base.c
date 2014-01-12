/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                 N   N  TTTTT                                %
%                                 NN  N    T                                  %
%                                 N N N    T                                  %
%                                 N  NN    T                                  %
%                                 N   N    T                                  %
%                                                                             %
%                                                                             %
%                   Windows NT Utility Methods for MagickCore                 %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                December 1996                                %
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
#include "magick/studio.h"
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
#include "magick/client.h"
#include "magick/exception-private.h"
#include "magick/locale_.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/resource_.h"
#include "magick/timer.h"
#include "magick/string_.h"
#include "magick/utility.h"
#include "magick/version.h"
#if defined(MAGICKCORE_LTDL_DELEGATE)
#  include "ltdl.h"
#endif
#include "magick/nt-base.h"
#if defined(MAGICKCORE_CIPHER_SUPPORT)
#include <ntsecapi.h>
#include <wincrypt.h>
#endif

/*
  Define declarations.
*/
#if !defined(MAP_FAILED)
#define MAP_FAILED      ((void *) -1)
#endif

/*
  Static declarations.
*/
#if !defined(MAGICKCORE_LTDL_DELEGATE)
static char
  *lt_slsearchpath = (char *) NULL;
#endif

static GhostInfo
  ghost_info;

static void
  *ghost_handle = (void *) NULL;

struct
{
  const HKEY
    hkey;

  const char
    *name;
}
const registry_roots[2] =
{
  { HKEY_CURRENT_USER,  "HKEY_CURRENT_USER"  },
  { HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE" }
};

/*
  External declarations.
*/
#if !defined(MAGICKCORE_WINDOWS_SUPPORT)
extern "C" BOOL WINAPI
  DllMain(HINSTANCE handle,DWORD reason,LPVOID lpvReserved);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D l l M a i n                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DllMain() is an entry point to the DLL which is called when processes and
%  threads are initialized and terminated, or upon calls to the Windows
%  LoadLibrary and FreeLibrary functions.
%
%  The function returns TRUE of it succeeds, or FALSE if initialization fails.
%
%  The format of the DllMain method is:
%
%    BOOL WINAPI DllMain(HINSTANCE handle,DWORD reason,LPVOID lpvReserved)
%
%  A description of each parameter follows:
%
%    o handle: handle to the DLL module
%
%    o reason: reason for calling function:
%
%      DLL_PROCESS_ATTACH - DLL is being loaded into virtual address
%                           space of current process.
%      DLL_THREAD_ATTACH - Indicates that the current process is
%                          creating a new thread.  Called under the
%                          context of the new thread.
%      DLL_THREAD_DETACH - Indicates that the thread is exiting.
%                          Called under the context of the exiting
%                          thread.
%      DLL_PROCESS_DETACH - Indicates that the DLL is being unloaded
%                           from the virtual address space of the
%                           current process.
%
%    o lpvReserved: Used for passing additional info during DLL_PROCESS_ATTACH
%                   and DLL_PROCESS_DETACH.
%
*/
#if defined(_DLL) && defined( ProvideDllMain )
BOOL WINAPI DllMain(HINSTANCE handle,DWORD reason,LPVOID lpvReserved)
{
  magick_unreferenced(lpvReserved);

  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
    {
      char
        *module_path;

      ssize_t
        count;

      module_path=(char *) AcquireQuantumMemory(MaxTextExtent,
        sizeof(*module_path));
      if (module_path == (char *) NULL)
        return(FALSE);
      count=(ssize_t) GetModuleFileName(handle,module_path,MaxTextExtent);
      if (count != 0)
        {
          char
            *path;

          for ( ; count > 0; count--)
            if (module_path[count] == '\\')
              {
                module_path[count+1]='\0';
                break;
              }
          MagickCoreGenesis(module_path,MagickFalse);
          path=(char *) AcquireQuantumMemory(16UL*MaxTextExtent,sizeof(*path));
          if (path == (char *) NULL)
            {
              module_path=DestroyString(module_path);
              return(FALSE);
            }
          count=(ssize_t) GetEnvironmentVariable("PATH",path,16*MaxTextExtent);
          if ((count != 0) && (strstr(path,module_path) == (char *) NULL))
            {
              if ((strlen(module_path)+count+1) < (16*MaxTextExtent-1))
                {
                  char
                    *variable;

                  variable=(char *) AcquireQuantumMemory(16UL*MaxTextExtent,
                    sizeof(*variable));
                  if (variable == (char *) NULL)
                    {
                      path=DestroyString(path);
                      module_path=DestroyString(module_path);
                      return(FALSE);
                    }
                  (void) FormatLocaleString(variable,16*MaxTextExtent,
                    "%s;%s",module_path,path);
                  SetEnvironmentVariable("PATH",variable);
                  variable=DestroyString(variable);
                }
            }
          path=DestroyString(path);
        }
      module_path=DestroyString(module_path);
      break;
    }
    case DLL_PROCESS_DETACH:
    {
      MagickCoreTerminus();
      break;
    }
    default:
      break;
  }
  return(TRUE);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x i t                                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Exit() calls TerminateProcess for Win95.
%
%  The format of the exit method is:
%
%      int Exit(int status)
%
%  A description of each parameter follows:
%
%    o status: an integer value representing the status of the terminating
%      process.
%
*/
MagickExport int Exit(int status)
{
  if (IsWindows95())
    {
      TerminateProcess(GetCurrentProcess(),(unsigned int) status);
      return(0);
    }
  exit(status);
}

#if !defined(__MINGW32__) && !defined(__MINGW64__)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   g e t t i m e o f d a y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The gettimeofday() method get the time of day.
%
%  The format of the gettimeofday method is:
%
%      int gettimeofday(struct timeval *time_value,struct timezone *time_zone)
%
%  A description of each parameter follows:
%
%    o time_value: the time value.
%
%    o time_zone: the time zone.
%
*/
MagickExport int gettimeofday (struct timeval *time_value,
  struct timezone *time_zone)
{
#define EpochFiletime  MagickLLConstant(116444736000000000)

  static int
    is_tz_set;

  if (time_value != (struct timeval *) NULL)
    {
      FILETIME
        file_time;

      __int64
        time;

      LARGE_INTEGER
        date_time;

      GetSystemTimeAsFileTime(&file_time);
      date_time.LowPart=file_time.dwLowDateTime;
      date_time.HighPart=file_time.dwHighDateTime;
      time=date_time.QuadPart;
      time-=EpochFiletime;
      time/=10;
      time_value->tv_sec=(ssize_t) (time / 1000000);
      time_value->tv_usec=(ssize_t) (time % 1000000);
    }
  if (time_zone != (struct timezone *) NULL)
    {
      if (is_tz_set == 0)
        {
          _tzset();
          is_tz_set++;
        }
      time_zone->tz_minuteswest=_timezone/60;
      time_zone->tz_dsttime=_daylight;
    }
  return(0);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s W i n d o w s 9 5                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsWindows95() returns true if the system is Windows 95.
%
%  The format of the IsWindows95 method is:
%
%      int IsWindows95()
%
*/
MagickExport int IsWindows95()
{
  OSVERSIONINFO
    version_info;

  version_info.dwOSVersionInfoSize=sizeof(version_info);
  if (GetVersionEx(&version_info) &&
      (version_info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS))
    return(1);
  return(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T A r g v T o U T F 8                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTArgvToUTF8() converts the wide command line arguments to UTF-8 to ensure
%  compatibility with Linux.
%
%  The format of the NTArgvToUTF8 method is:
%
%      char **NTArgvToUTF8(const int argc,wchar_t **argv)
%
%  A description of each parameter follows:
%
%    o argc: the number of command line arguments.
%
%    o argv:  the  wide-character command line arguments.
%
*/
MagickExport char **NTArgvToUTF8(const int argc,wchar_t **argv)
{
  char
    **utf8;

  ssize_t
    i;

  utf8=(char **) AcquireQuantumMemory(argc,sizeof(*utf8));
  if (utf8 == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"UnableToConvertStringToARGV");
  for (i=0; i < (ssize_t) argc; i++)
  {
    ssize_t
      count;

    count=WideCharToMultiByte(CP_UTF8,0,argv[i],-1,NULL,0,NULL,NULL);
    if (count < 0)
      count=0;
    utf8[i]=(char *) AcquireQuantumMemory(count+1,sizeof(**utf8));
    if (utf8[i] == (char *) NULL)
      {
        for (i--; i >= 0; i--)
          utf8[i]=DestroyString(utf8[i]);
        utf8=(char **) RelinquishMagickMemory(utf8);
        ThrowFatalException(ResourceLimitFatalError,
          "UnableToConvertStringToARGV");
      }
    count=WideCharToMultiByte(CP_UTF8,0,argv[i],-1,utf8[i],count,NULL,NULL);
    utf8[i][count]=0;
  }
  return(utf8);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T C l o s e D i r e c t o r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTCloseDirectory() closes the named directory stream and frees the DIR
%  structure.
%
%  The format of the NTCloseDirectory method is:
%
%      int NTCloseDirectory(DIR *entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
*/
MagickExport int NTCloseDirectory(DIR *entry)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(entry != (DIR *) NULL);
  FindClose(entry->hSearch);
  entry=(DIR *) RelinquishMagickMemory(entry);
  return(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T C l o s e L i b r a r y                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTCloseLibrary() unloads the module associated with the passed handle.
%
%  The format of the NTCloseLibrary method is:
%
%      void NTCloseLibrary(void *handle)
%
%  A description of each parameter follows:
%
%    o handle: Specifies a handle to a previously loaded dynamic module.
%
*/
MagickExport int NTCloseLibrary(void *handle)
{
  if (IsWindows95())
    return(FreeLibrary((HINSTANCE) handle));
  return(!(FreeLibrary((HINSTANCE) handle)));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T C o n t r o l H a n d l e r                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTControlHandler() registers a control handler that is activated when, for
%  example, a ctrl-c is received.
%
%  The format of the NTControlHandler method is:
%
%      int NTControlHandler(void)
%
*/

static BOOL ControlHandler(DWORD type)
{
  (void) type;
  AsynchronousResourceComponentTerminus();
  return(FALSE);
}

MagickExport int NTControlHandler(void)
{
  return(SetConsoleCtrlHandler((PHANDLER_ROUTINE) ControlHandler,TRUE));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T E l a p s e d T i m e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTElapsedTime() returns the elapsed time (in seconds) since the last call to
%  StartTimer().
%
%  The format of the ElapsedTime method is:
%
%      double NTElapsedTime(void)
%
*/
MagickExport double NTElapsedTime(void)
{
  union
  {
    FILETIME
      filetime;

    __int64
      filetime64;
  } elapsed_time;

  SYSTEMTIME
    system_time;

  GetSystemTime(&system_time);
  SystemTimeToFileTime(&system_time,&elapsed_time.filetime);
  return((double) 1.0e-7*elapsed_time.filetime64);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   N T E r r o r H a n d l e r                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTErrorHandler() displays an error reason and then terminates the program.
%
%  The format of the NTErrorHandler method is:
%
%      void NTErrorHandler(const ExceptionType severity,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o severity: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
MagickExport void NTErrorHandler(const ExceptionType severity,
  const char *reason,const char *description)
{
  char
    buffer[3*MaxTextExtent],
    *message;

  (void) severity;
  if (reason == (char *) NULL)
    {
      MagickCoreTerminus();
      exit(0);
    }
  message=GetExceptionMessage(errno);
  if ((description != (char *) NULL) && errno)
    (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s (%s) [%s].\n",
      GetClientName(),reason,description,message);
  else
    if (description != (char *) NULL)
      (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s (%s).\n",
        GetClientName(),reason,description);
    else
      if (errno != 0)
        (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s [%s].\n",
          GetClientName(),reason,message);
      else
        (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s.\n",
          GetClientName(),reason);
  message=DestroyString(message);
  (void) MessageBox(NULL,buffer,"ImageMagick Exception",MB_OK | MB_TASKMODAL |
    MB_SETFOREGROUND | MB_ICONEXCLAMATION);
  MagickCoreTerminus();
  exit(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T E x i t L i b r a r y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTExitLibrary() exits the dynamic module loading subsystem.
%
%  The format of the NTExitLibrary method is:
%
%      int NTExitLibrary(void)
%
*/
MagickExport int NTExitLibrary(void)
{
  return(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G a t h e r R a n d o m D a t a                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGatherRandomData() gathers random data and returns it.
%
%  The format of the GatherRandomData method is:
%
%      MagickBooleanType NTGatherRandomData(const size_t length,
%        unsigned char *random)
%
%  A description of each parameter follows:
%
%    length: the length of random data buffer
%
%    random: the random data is returned here.
%
*/
MagickExport MagickBooleanType NTGatherRandomData(const size_t length,
  unsigned char *random)
{
#if defined(MAGICKCORE_CIPHER_SUPPORT) && defined(_MSC_VER) && (_MSC_VER > 1200)
  HCRYPTPROV
    handle;

  int
    status;

  handle=(HCRYPTPROV) NULL;
  status=CryptAcquireContext(&handle,NULL,MS_DEF_PROV,PROV_RSA_FULL,
    (CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET));
  if (status == 0)
    status=CryptAcquireContext(&handle,NULL,MS_DEF_PROV,PROV_RSA_FULL,
      (CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET));
  if (status == 0)
    return(MagickFalse);
  status=CryptGenRandom(handle,(DWORD) length,random);
  if (status == 0)
    {
      status=CryptReleaseContext(handle,0);
      return(MagickFalse);
    }
  status=CryptReleaseContext(handle,0);
  if (status == 0)
    return(MagickFalse);
#else
  (void) random;
  (void) length;
#endif
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G e t E x e c u t i o n P a t h                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGetExecutionPath() returns the execution path of a program.
%
%  The format of the GetExecutionPath method is:
%
%      MagickBooleanType NTGetExecutionPath(char *path,const size_t extent)
%
%  A description of each parameter follows:
%
%    o path: the pathname of the executable that started the process.
%
%    o extent: the maximum extent of the path.
%
*/
MagickExport MagickBooleanType NTGetExecutionPath(char *path,
  const size_t extent)
{
  GetModuleFileName(0,path,(DWORD) extent);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G e t L a s t E r r o r                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGetLastError() returns the last error that occurred.
%
%  The format of the NTGetLastError method is:
%
%      char *NTGetLastError(void)
%
*/
char *NTGetLastError(void)
{
  char
    *reason;

  int
    status;

  LPVOID
    buffer;

  status=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),
    MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR) &buffer,0,NULL);
  if (!status)
    reason=AcquireString("An unknown error occurred");
  else
    {
      reason=AcquireString((const char *) buffer);
      LocalFree(buffer);
    }
  return(reason);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G e t L i b r a r y E r r o r                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Lt_dlerror() returns a pointer to a string describing the last error
%  associated with a lt_dl method.  Note that this function is not thread
%  safe so it should only be used under the protection of a lock.
%
%  The format of the NTGetLibraryError method is:
%
%      const char *NTGetLibraryError(void)
%
*/
MagickExport const char *NTGetLibraryError(void)
{
  static char
    last_error[MaxTextExtent];

  char
    *error;

  *last_error='\0';
  error=NTGetLastError();
  if (error)
    (void) CopyMagickString(last_error,error,MaxTextExtent);
  error=DestroyString(error);
  return(last_error);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G e t L i b r a r y S y m b o l                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGetLibrarySymbol() retrieve the procedure address of the method
%  specified by the passed character string.
%
%  The format of the NTGetLibrarySymbol method is:
%
%      void *NTGetLibrarySymbol(void *handle,const char *name)
%
%  A description of each parameter follows:
%
%    o handle: Specifies a handle to the previously loaded dynamic module.
%
%    o name: Specifies the procedure entry point to be returned.
%
*/
void *NTGetLibrarySymbol(void *handle,const char *name)
{
  LPFNDLLFUNC1
    lpfnDllFunc1;

  lpfnDllFunc1=(LPFNDLLFUNC1) GetProcAddress((HINSTANCE) handle,name);
  if (!lpfnDllFunc1)
    return((void *) NULL);
  return((void *) lpfnDllFunc1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G e t M o d u l e P a t h                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGetModulePath() returns the path of the specified module.
%
%  The format of the GetModulePath method is:
%
%      MagickBooleanType NTGetModulePath(const char *module,char *path)
%
%  A description of each parameter follows:
%
%    modith: the module name.
%
%    path: the module path is returned here.
%
*/
MagickExport MagickBooleanType NTGetModulePath(const char *module,char *path)
{
  char
    module_path[MaxTextExtent];

  HMODULE
    handle;

  ssize_t
    length;

  *path='\0';
  handle=GetModuleHandle(module);
  if (handle == (HMODULE) NULL)
    return(MagickFalse);
  length=GetModuleFileName(handle,module_path,MaxTextExtent);
  if (length != 0)
    GetPathComponent(module_path,HeadPath,path);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G h o s t s c r i p t D L L                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGhostscriptDLL() returns the path to the most recent Ghostscript version
%  DLL.  The method returns TRUE on success otherwise FALSE.
%
%  The format of the NTGhostscriptDLL method is:
%
%      int NTGhostscriptDLL(char *path,int length)
%
%  A description of each parameter follows:
%
%    o path: return the Ghostscript DLL path here.
%
%    o length: the buffer length.
%
*/

static int NTGetRegistryValue(HKEY root,const char *key,DWORD flags,const char *name,
  char *value,int *length)
{
  BYTE
    byte,
    *p;

  DWORD
    extent,
    type;

  HKEY
    hkey;

  LONG
    status;

  /*
    Get a registry value: key = root\\key, named value = name.
  */
  if (RegOpenKeyExA(root,key,0,KEY_READ | flags,&hkey) != ERROR_SUCCESS)
    return(1);  /* no match */
  p=(BYTE *) value;
  type=REG_SZ;
  extent=(*length);
  if (p == (BYTE *) NULL)
    p=(&byte);  /* ERROR_MORE_DATA only if value is NULL */
  status=RegQueryValueExA(hkey,(char *) name,0,&type,p,&extent);
  RegCloseKey(hkey);
  if (status == ERROR_SUCCESS)
    {
      *length=extent;
      return(0);  /* return the match */
    }
  if (status == ERROR_MORE_DATA)
    {
      *length=extent;
      return(-1);  /* buffer not large enough */
    }
  return(1);  /* not found */
}

static int NTLocateGhostscript(DWORD flags,int *root_index,
  const char **product_family,int *major_version,int *minor_version)
{
  int
    i;

  MagickBooleanType
    status;

  static const char
    *products[4] =
    {
      "GPL Ghostscript",
      "GNU Ghostscript",
      "AFPL Ghostscript",
      "Aladdin Ghostscript"
    };

  /*
    Find the most recent version of Ghostscript.
  */
  status=MagickFalse;
  *root_index=0;
  *product_family=NULL;
  *major_version=5;
  *minor_version=49; /* min version of Ghostscript is 5.50 */
  for (i=0; i < (ssize_t) (sizeof(products)/sizeof(products[0])); i++)
  {
    char
      key[MaxTextExtent];

    HKEY
      hkey;

    int
      j;

    REGSAM
      mode;

    (void) FormatLocaleString(key,MaxTextExtent,"SOFTWARE\\%s",products[i]);
    for (j=0; j < 2; j++)
    {
      mode=KEY_READ | flags;
      if (RegOpenKeyExA(registry_roots[j].hkey,key,0,mode,&hkey) == 
            ERROR_SUCCESS)
        {
          DWORD
            extent;

          int
            k;

          /*
            Now enumerate the keys.
          */
          extent=sizeof(key)/sizeof(char);
          for (k=0; RegEnumKeyA(hkey,k,key,extent) == ERROR_SUCCESS; k++)
          {
            int
              major,
              minor;

            major=0;
            minor=0;
            if (sscanf(key,"%d.%d",&major,&minor) != 2)
              continue;
            if ((major > *major_version) || ((major == *major_version) &&
                (minor > *minor_version)))
              {
                *root_index=j;
                *product_family=products[i];
                *major_version=major;
                *minor_version=minor;
                status=MagickTrue;
              }
         }
         (void) RegCloseKey(hkey);
       }
    }
  }
  if (status == MagickFalse)
    {
      *major_version=0;
      *minor_version=0;
    }
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),"Ghostscript (%s) "
    "version %d.%02d",*product_family,*major_version,*minor_version);
  return(status);
}

static int NTGhostscriptGetString(const char *name,BOOL *is_64_bit,
  char *value,const size_t length)
{
  char
    key[MaxTextExtent];

  int
    extent;

  static const char
    *product_family=(const char *) NULL;

  static BOOL
    is_64_bit_version=FALSE;

  static int
    flags=0,
    major_version=0,
    minor_version=0,
    root_index=0;

  /*
    Get a string from the installed Ghostscript.
  */
  *value='\0';
  if (product_family == NULL)
  {
    flags=0;
#if defined(KEY_WOW64_32KEY)
#if defined(_WIN64)
    flags=KEY_WOW64_64KEY;
#else
    flags=KEY_WOW64_32KEY;
#endif
    (void) NTLocateGhostscript(flags,&root_index,&product_family,
      &major_version,&minor_version);
    if (product_family == NULL)
#if defined(_WIN64)
      flags=KEY_WOW64_32KEY;
    else
      is_64_bit_version=TRUE;
#else
      flags=KEY_WOW64_64KEY;
#endif
#endif
  }
  if (product_family == NULL)
  {
    (void) NTLocateGhostscript(flags,&root_index,&product_family,
      &major_version,&minor_version);
  }
  if (product_family == NULL)
    return(FALSE);
  if (is_64_bit != NULL)
    *is_64_bit=is_64_bit_version;
  (void) FormatLocaleString(key,MaxTextExtent,"SOFTWARE\\%s\\%d.%02d",
    product_family,major_version,minor_version);
  extent=(int) length;
  if (NTGetRegistryValue(registry_roots[root_index].hkey,key,flags,name,value,
     &extent) == 0)
    {
      (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
        "registry: \"%s\\%s\\%s\"=\"%s\"",registry_roots[root_index].name,key,
        name,value);
      return(TRUE);
    }
  return(FALSE);
}

MagickExport int NTGhostscriptDLL(char *path,int length)
{
  static char
    dll[MaxTextExtent] = { "" };

  static BOOL
    is_64_bit_version;

  *path='\0';
  if ((*dll == '\0') &&
      (NTGhostscriptGetString("GS_DLL",&is_64_bit_version,dll,sizeof(dll)) == FALSE))
    return(FALSE);
#if defined(_WIN64)
  if (!is_64_bit_version)
    return(FALSE);
#else
  if (is_64_bit_version)
    return(FALSE);
#endif
  (void) CopyMagickString(path,dll,length);
  return(TRUE);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G h o s t s c r i p t D L L V e c t o r s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGhostscriptDLLVectors() returns a GhostInfo structure that includes
%  function vectors to invoke Ghostscript DLL functions. A null pointer is
%  returned if there is an error when loading the DLL or retrieving the
%  function vectors.
%
%  The format of the NTGhostscriptDLLVectors method is:
%
%      const GhostInfo *NTGhostscriptDLLVectors(void)
%
*/
MagickExport const GhostInfo *NTGhostscriptDLLVectors(void)
{
  if (NTGhostscriptLoadDLL() == FALSE)
    return((GhostInfo *) NULL);
  return(&ghost_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G h o s t s c r i p t E X E                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGhostscriptEXE() obtains the path to the latest Ghostscript executable.
%  The method returns FALSE if a full path value is not obtained and returns
%  a default path of gswin32c.exe.
%
%  The format of the NTGhostscriptEXE method is:
%
%      int NTGhostscriptEXE(char *path,int length)
%
%  A description of each parameter follows:
%
%    o path: return the Ghostscript executable path here.
%
%    o length: length of buffer.
%
*/
MagickExport int NTGhostscriptEXE(char *path,int length)
{
  register char
    *p;

  static char
    program[MaxTextExtent] = { "" };

  static BOOL
    is_64_bit_version = FALSE;

  (void) CopyMagickString(path,"gswin32c.exe",length);
  if ((*program == '\0') &&
      (NTGhostscriptGetString("GS_DLL",&is_64_bit_version,program,sizeof(program)) == FALSE))
    return(FALSE);
  p=strrchr(program,'\\');
  if (p != (char *) NULL)
    {
      p++;
      *p='\0';
      (void) ConcatenateMagickString(program,is_64_bit_version ?
        "gswin64c.exe" : "gswin32c.exe",sizeof(program));
    }
  (void) CopyMagickString(path,program,length);
  return(TRUE);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G h o s t s c r i p t F o n t s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGhostscriptFonts() obtains the path to the Ghostscript fonts.  The method
%  returns FALSE if it cannot determine the font path.
%
%  The format of the NTGhostscriptFonts method is:
%
%      int NTGhostscriptFonts(char *path, int length)
%
%  A description of each parameter follows:
%
%    o path: return the font path here.
%
%    o length: length of the path buffer.
%
*/
MagickExport int NTGhostscriptFonts(char *path,int length)
{
  char
    buffer[MaxTextExtent],
    filename[MaxTextExtent];

  register char
    *p,
    *q;

  *path='\0';
  if (NTGhostscriptGetString("GS_LIB",NULL,buffer,MaxTextExtent) == FALSE)
    return(FALSE);
  for (p=buffer-1; p != (char *) NULL; p=strchr(p+1,DirectoryListSeparator))
  {
    (void) CopyMagickString(path,p+1,length+1);
    q=strchr(path,DirectoryListSeparator);
    if (q != (char *) NULL)
      *q='\0';
    (void) FormatLocaleString(filename,MaxTextExtent,"%s%sfonts.dir",path,
      DirectorySeparator);
    if (IsPathAccessible(filename) != MagickFalse)
      return(TRUE);
  }
  (void) FormatLocaleString(filename,MaxTextExtent,"c:%sgs%sfonts%sfonts.dir",
    DirectorySeparator,DirectorySeparator,DirectorySeparator);
  if (IsPathAccessible(filename) != MagickFalse)
    return(TRUE);
  return(FALSE);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G h o s t s c r i p t L o a d D L L                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGhostscriptLoadDLL() attempts to load the Ghostscript DLL and returns
%  TRUE if it succeeds.
%
%  The format of the NTGhostscriptLoadDLL method is:
%
%      int NTGhostscriptLoadDLL(void)
%
*/
MagickExport int NTGhostscriptLoadDLL(void)
{
  char
    path[MaxTextExtent];

  if (ghost_handle != (void *) NULL)
    return(TRUE);
  if (NTGhostscriptDLL(path,sizeof(path)) == FALSE)
    return(FALSE);
  ghost_handle=lt_dlopen(path);
  if (ghost_handle == (void *) NULL)
    return(FALSE);
  (void) ResetMagickMemory((void *) &ghost_info,0,sizeof(GhostInfo));
  ghost_info.exit=(int (MagickDLLCall *)(gs_main_instance*))
    lt_dlsym(ghost_handle,"gsapi_exit");
  ghost_info.init_with_args=(int (MagickDLLCall *)(gs_main_instance *,int,
    char **)) (lt_dlsym(ghost_handle,"gsapi_init_with_args"));
  ghost_info.new_instance=(int (MagickDLLCall *)(gs_main_instance **,void *)) (
    lt_dlsym(ghost_handle,"gsapi_new_instance"));
  ghost_info.run_string=(int (MagickDLLCall *)(gs_main_instance *,const char *,
    int,int *)) (lt_dlsym(ghost_handle,"gsapi_run_string"));
  ghost_info.delete_instance=(void (MagickDLLCall *) (gs_main_instance *)) (
    lt_dlsym(ghost_handle,"gsapi_delete_instance"));
  if ((ghost_info.exit == NULL) || (ghost_info.init_with_args == NULL) ||
      (ghost_info.new_instance == NULL) || (ghost_info.run_string == NULL) ||
      (ghost_info.delete_instance == NULL))
    return(FALSE);
  return(TRUE);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T G h o s t s c r i p t U n L o a d D L L                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTGhostscriptUnLoadDLL() unloads the Ghostscript DLL and returns TRUE if
%  it succeeds.
%
%  The format of the NTGhostscriptUnLoadDLL method is:
%
%      int NTGhostscriptUnLoadDLL(void)
%
*/
MagickExport int NTGhostscriptUnLoadDLL(void)
{
  int
    status;

  if (ghost_handle == (void *) NULL)
    return(FALSE);
  status=lt_dlclose(ghost_handle);
  ghost_handle=(void *) NULL;
  (void) ResetMagickMemory((void *) &ghost_info,0,sizeof(GhostInfo));
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T I n i t i a l i z e L i b r a r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTInitializeLibrary() initializes the dynamic module loading subsystem.
%
%  The format of the NTInitializeLibrary method is:
%
%      int NTInitializeLibrary(void)
%
*/
MagickExport int NTInitializeLibrary(void)
{
  return(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  N T M a p M e m o r y                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Mmap() emulates the Unix method of the same name.
%
%  The format of the NTMapMemory method is:
%
%    MagickExport void *NTMapMemory(char *address,size_t length,int protection,
%      int access,int file,MagickOffsetType offset)
%
*/
MagickExport void *NTMapMemory(char *address,size_t length,int protection,
  int flags,int file,MagickOffsetType offset)
{
  DWORD
    access_mode,
    high_length,
    high_offset,
    low_length,
    low_offset,
    protection_mode;

  HANDLE
    file_handle,
    map_handle;

  void
    *map;

  (void) address;
  access_mode=0;
  file_handle=INVALID_HANDLE_VALUE;
  low_length=(DWORD) (length & 0xFFFFFFFFUL);
  high_length=(DWORD) ((((MagickOffsetType) length) >> 32) & 0xFFFFFFFFUL);
  map_handle=INVALID_HANDLE_VALUE;
  map=(void *) NULL;
  low_offset=(DWORD) (offset & 0xFFFFFFFFUL);
  high_offset=(DWORD) ((offset >> 32) & 0xFFFFFFFFUL);
  protection_mode=0;
  if (protection & PROT_WRITE)
    {
      access_mode=FILE_MAP_WRITE;
      if (!(flags & MAP_PRIVATE))
        protection_mode=PAGE_READWRITE;
      else
        {
          access_mode=FILE_MAP_COPY;
          protection_mode=PAGE_WRITECOPY;
        }
    }
  else
    if (protection & PROT_READ)
      {
        access_mode=FILE_MAP_READ;
        protection_mode=PAGE_READONLY;
      }
  if ((file == -1) && (flags & MAP_ANONYMOUS))
    file_handle=INVALID_HANDLE_VALUE;
  else
    file_handle=(HANDLE) _get_osfhandle(file);
  map_handle=CreateFileMapping(file_handle,0,protection_mode,high_length,
    low_length,0);
  if (map_handle)
    {
      map=(void *) MapViewOfFile(map_handle,access_mode,high_offset,low_offset,
        length);
      CloseHandle(map_handle);
    }
  if (map == (void *) NULL)
    return((void *) ((char *) MAP_FAILED));
  return((void *) ((char *) map));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T O p e n D i r e c t o r y                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTOpenDirectory() opens the directory named by filename and associates a
%  directory stream with it.
%
%  The format of the NTOpenDirectory method is:
%
%      DIR *NTOpenDirectory(const char *path)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
*/
MagickExport DIR *NTOpenDirectory(const char *path)
{
  wchar_t
    file_specification[MaxTextExtent];

  DIR
    *entry;

  size_t
    length;

  assert(path != (const char *) NULL);
  length=MultiByteToWideChar(CP_UTF8,0,path,-1,file_specification,
    MaxTextExtent);
  if (length == 0)
    return((DIR *) NULL);
  if(wcsncat(file_specification,(const wchar_t*) DirectorySeparator,
       MaxTextExtent) == (wchar_t*)NULL)
    return((DIR *) NULL);
  entry=(DIR *) AcquireMagickMemory(sizeof(DIR));
  if (entry != (DIR *) NULL)
    {
      entry->firsttime=TRUE;
      entry->hSearch=FindFirstFileW(file_specification,&entry->Win32FindData);
    }
  if (entry->hSearch == INVALID_HANDLE_VALUE)
    {
      if(wcsncat(file_specification,L"*.*",
        MaxTextExtent) == (wchar_t*)NULL)
        {
          entry=(DIR *) RelinquishMagickMemory(entry);
          return((DIR *) NULL);
        }
      entry->hSearch=FindFirstFileW(file_specification,&entry->Win32FindData);
      if (entry->hSearch == INVALID_HANDLE_VALUE)
        {
          entry=(DIR *) RelinquishMagickMemory(entry);
          return((DIR *) NULL);
        }
    }
  return(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T O p e n L i b r a r y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTOpenLibrary() loads a dynamic module into memory and returns a handle that
%  can be used to access the various procedures in the module.
%
%  The format of the NTOpenLibrary method is:
%
%      void *NTOpenLibrary(const char *filename)
%
%  A description of each parameter follows:
%
%    o path: Specifies a pointer to string representing dynamic module that
%      is to be loaded.
%
*/

static inline const char *GetSearchPath(void)
{
#if defined(MAGICKCORE_LTDL_DELEGATE)
  return(lt_dlgetsearchpath());
#else
  return(lt_slsearchpath);
#endif
}

static UINT ChangeErrorMode(void)
{
  typedef UINT
    (CALLBACK *GETERRORMODE)(void);

  GETERRORMODE
    getErrorMode;

  HMODULE
    handle;

  UINT
    mode;

  mode=SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX;

  handle=GetModuleHandle("kernel32.dll");
  if (handle == (HMODULE) NULL)
    return SetErrorMode(mode);

  getErrorMode=(GETERRORMODE) NTGetLibrarySymbol(handle,"GetErrorMode");
  if (getErrorMode != (GETERRORMODE) NULL)
    mode=getErrorMode() | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX;

  return SetErrorMode(mode);
}

MagickExport void *NTOpenLibrary(const char *filename)
{
#define MaxPathElements 31

  char
    buffer[MaxTextExtent];

  int
    index;

  register const char
    *p,
    *q;

  register int
    i;

  UINT
    mode;

  void
    *handle;

  mode=ChangeErrorMode();
  handle=(void *) LoadLibraryEx(filename,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
  if ((handle != (void *) NULL) || (GetSearchPath() == (char *) NULL))
    {
      SetErrorMode(mode);
      return(handle);
    }
  p=(char *) GetSearchPath();
  index=0;
  while (index < MaxPathElements)
  {
    q=strchr(p,DirectoryListSeparator);
    if (q == (char *) NULL)
      {
        (void) CopyMagickString(buffer,p,MaxTextExtent);
        (void) ConcatenateMagickString(buffer,"\\",MaxTextExtent);
        (void) ConcatenateMagickString(buffer,filename,MaxTextExtent);
        handle=(void *) LoadLibraryEx(buffer,NULL,
          LOAD_WITH_ALTERED_SEARCH_PATH);
        break;
      }
    i=q-p;
    (void) CopyMagickString(buffer,p,i+1);
    (void) ConcatenateMagickString(buffer,"\\",MaxTextExtent);
    (void) ConcatenateMagickString(buffer,filename,MaxTextExtent);
    handle=(void *) LoadLibraryEx(buffer,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
    if (handle != (void *) NULL)
      break;
    p=q+1;
  }
  SetErrorMode(mode);
  return(handle);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    N T R e a d D i r e c t o r y                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTReadDirectory() returns a pointer to a structure representing the
%  directory entry at the current position in the directory stream to which
%  entry refers.
%
%  The format of the NTReadDirectory
%
%      NTReadDirectory(entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
*/
MagickExport struct dirent *NTReadDirectory(DIR *entry)
{
  int
    status;

  size_t
    length;

  if (entry == (DIR *) NULL)
    return((struct dirent *) NULL);
  if (!entry->firsttime)
    {
      status=FindNextFileW(entry->hSearch,&entry->Win32FindData);
      if (status == 0)
        return((struct dirent *) NULL);
    }
  length=WideCharToMultiByte(CP_UTF8,0,entry->Win32FindData.cFileName,-1,
    entry->file_info.d_name,sizeof(entry->file_info.d_name),NULL,NULL);
  if (length == 0)
    return((struct dirent *) NULL);
  entry->firsttime=FALSE;
  entry->file_info.d_namlen=(int) strlen(entry->file_info.d_name);
  return(&entry->file_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T R e g i s t r y K e y L o o k u p                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTRegistryKeyLookup() returns ImageMagick installation path settings
%  stored in the Windows Registry.  Path settings are specific to the
%  installed ImageMagick version so that multiple Image Magick installations
%  may coexist.
%
%  Values are stored in the registry under a base path path similar to
%  "HKEY_LOCAL_MACHINE/SOFTWARE\ImageMagick\6.7.4\Q:16" or
%  "HKEY_CURRENT_USER/SOFTWARE\ImageMagick\6.7.4\Q:16". The provided subkey
%  is appended to this base path to form the full key.
%
%  The format of the NTRegistryKeyLookup method is:
%
%      unsigned char *NTRegistryKeyLookup(const char *subkey)
%
%  A description of each parameter follows:
%
%    o subkey: Specifies a string that identifies the registry object.
%      Currently supported sub-keys include: "BinPath", "ConfigurePath",
%      "LibPath", "CoderModulesPath", "FilterModulesPath", "SharePath".
%
*/
MagickExport unsigned char *NTRegistryKeyLookup(const char *subkey)
{
  char
    package_key[MaxTextExtent];

  DWORD
    size,
    type;

  HKEY
    registry_key;

  LONG
    status;

  unsigned char
    *value;

  /*
    Look-up base key.
  */
  (void) FormatLocaleString(package_key,MaxTextExtent,"SOFTWARE\\%s\\%s\\Q:%d",
    MagickPackageName,MagickLibVersionText,MAGICKCORE_QUANTUM_DEPTH);
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),"%s",package_key);
  registry_key=(HKEY) INVALID_HANDLE_VALUE;
  status=RegOpenKeyExA(HKEY_LOCAL_MACHINE,package_key,0,KEY_READ,&registry_key);
  if (status != ERROR_SUCCESS)
    status=RegOpenKeyExA(HKEY_CURRENT_USER,package_key,0,KEY_READ,
      &registry_key);
  if (status != ERROR_SUCCESS)
    {
      registry_key=(HKEY) INVALID_HANDLE_VALUE;
      return((unsigned char *) NULL);
    }
  /*
    Look-up sub key.
  */
  size=32;
  value=(unsigned char *) AcquireQuantumMemory(size,sizeof(*value));
  if (value == (unsigned char *) NULL)
    {
      RegCloseKey(registry_key);
      return((unsigned char *) NULL);
    }
  (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),"%s",subkey);
  status=RegQueryValueExA(registry_key,subkey,0,&type,value,&size);
  if ((status == ERROR_MORE_DATA) && (type == REG_SZ))
    {
      value=(unsigned char *) ResizeQuantumMemory(value,size,sizeof(*value));
      if (value == (BYTE *) NULL)
        {
          RegCloseKey(registry_key);
          return((unsigned char *) NULL);
        }
      status=RegQueryValueExA(registry_key,subkey,0,&type,value,&size);
    }
  RegCloseKey(registry_key);
  if ((type != REG_SZ) || (status != ERROR_SUCCESS))
    value=(unsigned char *) RelinquishMagickMemory(value);
  return((unsigned char *) value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T R e p o r t E v e n t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTReportEvent() reports an event.
%
%  The format of the NTReportEvent method is:
%
%      MagickBooleanType NTReportEvent(const char *event,
%        const MagickBooleanType error)
%
%  A description of each parameter follows:
%
%    o event: the event.
%
%    o error: MagickTrue the event is an error.
%
*/
MagickExport MagickBooleanType NTReportEvent(const char *event,
  const MagickBooleanType error)
{
  const char
    *events[1];

  HANDLE
    handle;

  WORD
    type;

  handle=RegisterEventSource(NULL,MAGICKCORE_PACKAGE_NAME);
  if (handle == NULL)
    return(MagickFalse);
  events[0]=event;
  type=error ? EVENTLOG_ERROR_TYPE : EVENTLOG_WARNING_TYPE;
  ReportEvent(handle,type,0,0,NULL,1,0,events,NULL);
  DeregisterEventSource(handle);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T R e s o u r c e T o B l o b                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTResourceToBlob() returns a blob containing the contents of the resource
%  in the current executable specified by the id parameter. This currently
%  used to retrieve MGK files tha have been embedded into the various command
%  line utilities.
%
%  The format of the NTResourceToBlob method is:
%
%      unsigned char *NTResourceToBlob(const char *id)
%
%  A description of each parameter follows:
%
%    o id: Specifies a string that identifies the resource.
%
*/
MagickExport unsigned char *NTResourceToBlob(const char *id)
{
#ifndef MAGICKCORE_LIBRARY_NAME
  char
    path[MaxTextExtent];
#endif

  DWORD
    length;

  HGLOBAL
    global;

  HMODULE
    handle;

  HRSRC
    resource;

  unsigned char
    *blob,
    *value;

  assert(id != (const char *) NULL);
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",id);
#ifdef MAGICKCORE_LIBRARY_NAME
  handle=GetModuleHandle(MAGICKCORE_LIBRARY_NAME);
#else
  (void) FormatLocaleString(path,MaxTextExtent,"%s%s%s",GetClientPath(),
    DirectorySeparator,GetClientName());
  if (IsPathAccessible(path) != MagickFalse)
    handle=GetModuleHandle(path);
  else
    handle=GetModuleHandle(0);
#endif
  if (!handle)
    return((unsigned char *) NULL);
  resource=FindResource(handle,id,"IMAGEMAGICK");
  if (!resource)
    return((unsigned char *) NULL);
  global=LoadResource(handle,resource);
  if (!global)
    return((unsigned char *) NULL);
  length=SizeofResource(handle,resource);
  value=(unsigned char *) LockResource(global);
  if (!value)
    {
      FreeResource(global);
      return((unsigned char *) NULL);
    }
  blob=(unsigned char *) AcquireQuantumMemory(length+MaxTextExtent,
    sizeof(*blob));
  if (blob != (unsigned char *) NULL)
    {
      (void) CopyMagickMemory(blob,value,length);
      blob[length]='\0';
    }
  UnlockResource(global);
  FreeResource(global);
  return(blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T S e e k D i r e c t o r y                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTSeekDirectory() sets the position of the next NTReadDirectory() operation
%  on the directory stream.
%
%  The format of the NTSeekDirectory method is:
%
%      void NTSeekDirectory(DIR *entry,ssize_t position)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
%    o position: specifies the position associated with the directory
%      stream.
%
*/
MagickExport void NTSeekDirectory(DIR *entry,ssize_t position)
{
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(entry != (DIR *) NULL);
  (void) entry;
  (void) position;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T S e t S e a r c h P a t h                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTSetSearchPath() sets the current locations that the subsystem should
%  look at to find dynamically loadable modules.
%
%  The format of the NTSetSearchPath method is:
%
%      int NTSetSearchPath(const char *path)
%
%  A description of each parameter follows:
%
%    o path: Specifies a pointer to string representing the search path
%      for DLL's that can be dynamically loaded.
%
*/
MagickExport int NTSetSearchPath(const char *path)
{
#if defined(MAGICKCORE_LTDL_DELEGATE)
  lt_dlsetsearchpath(path);
#else
  if (lt_slsearchpath != (char *) NULL)
    lt_slsearchpath=DestroyString(lt_slsearchpath);
  if (path != (char *) NULL)
    lt_slsearchpath=AcquireString(path);
#endif
  return(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  N T S y n c M e m o r y                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTSyncMemory() emulates the Unix method of the same name.
%
%  The format of the NTSyncMemory method is:
%
%      int NTSyncMemory(void *address,size_t length,int flags)
%
%  A description of each parameter follows:
%
%    o address: the address of the binary large object.
%
%    o length: the length of the binary large object.
%
%    o flags: Option flags (ignored for Windows).
%
*/
MagickExport int NTSyncMemory(void *address,size_t length,int flags)
{
  (void) flags;
  if (FlushViewOfFile(address,length) == MagickFalse)
    return(-1);
  return(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T S y s t e m C o m m a n d                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTSystemCommand() executes the specified command and waits until it
%  terminates.  The returned value is the exit status of the command.
%
%  The format of the NTSystemCommand method is:
%
%      int NTSystemCommand(MagickFalse,const char *command)
%
%  A description of each parameter follows:
%
%    o command: This string is the command to execute.
%
*/
MagickExport int NTSystemCommand(const char *command)
{
  char
    local_command[MaxTextExtent];

  DWORD
    child_status;

  int
    status;

  MagickBooleanType
    asynchronous;

  PROCESS_INFORMATION
    process_info;

  STARTUPINFO
    startup_info;

  if (command == (char *) NULL)
    return(-1);
  GetStartupInfo(&startup_info);
  startup_info.dwFlags=STARTF_USESHOWWINDOW;
  startup_info.wShowWindow=SW_SHOWMINNOACTIVE;
  (void) CopyMagickString(local_command,command,MaxTextExtent);
  asynchronous=command[strlen(command)-1] == '&' ? MagickTrue : MagickFalse;
  if (asynchronous != MagickFalse)
    {
      local_command[strlen(command)-1]='\0';
      startup_info.wShowWindow=SW_SHOWDEFAULT;
    }
  else
    if (command[strlen(command)-1] == '|')
      local_command[strlen(command)-1]='\0';
    else
      startup_info.wShowWindow=SW_HIDE;
  status=CreateProcess((LPCTSTR) NULL,local_command,
    (LPSECURITY_ATTRIBUTES) NULL,(LPSECURITY_ATTRIBUTES) NULL,(BOOL) FALSE,
    (DWORD) NORMAL_PRIORITY_CLASS,(LPVOID) NULL,(LPCSTR) NULL,&startup_info,
    &process_info);
  if (status == 0)
    return(-1);
  if (asynchronous)
    return(status == 0);
  status=WaitForSingleObject(process_info.hProcess,INFINITE);
  if (status != WAIT_OBJECT_0)
    return(status);
  status=GetExitCodeProcess(process_info.hProcess,&child_status);
  if (status == 0)
    return(-1);
  CloseHandle(process_info.hProcess);
  CloseHandle(process_info.hThread);
  return((int) child_status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T S y s t e m C o n i f i g u r a t i o n                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTSystemConfiguration() provides a way for the application to determine
%  values for system limits or options at runtime.
%
%  The format of the exit method is:
%
%      ssize_t NTSystemConfiguration(int name)
%
%  A description of each parameter follows:
%
%    o name: _SC_PAGE_SIZE or _SC_PHYS_PAGES.
%
*/
MagickExport ssize_t NTSystemConfiguration(int name)
{
  switch (name)
  {
    case _SC_PAGESIZE:
    {
      SYSTEM_INFO
        system_info;

      GetSystemInfo(&system_info);
      return(system_info.dwPageSize);
    }
    case _SC_PHYS_PAGES:
    {
      HMODULE
        handle;

      LPFNDLLFUNC2
        module;

      NTMEMORYSTATUSEX
        status;

      SYSTEM_INFO
        system_info;

      handle=GetModuleHandle("kernel32.dll");
      if (handle == (HMODULE) NULL)
        return(0L);
      GetSystemInfo(&system_info);
      module=(LPFNDLLFUNC2) NTGetLibrarySymbol(handle,"GlobalMemoryStatusEx");
      if (module == (LPFNDLLFUNC2) NULL)
        {
          MEMORYSTATUS
            status;

          GlobalMemoryStatus(&status);
          return((ssize_t) status.dwTotalPhys/system_info.dwPageSize/4);
        }
      status.dwLength=sizeof(status);
      if (module(&status) == 0)
        return(0L);
      return((ssize_t) status.ullTotalPhys/system_info.dwPageSize/4);
    }
    case _SC_OPEN_MAX:
      return(2048);
    default:
      break;
  }
  return(-1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T T e l l D i r e c t o r y                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTTellDirectory() returns the current location associated with the named
%  directory stream.
%
%  The format of the NTTellDirectory method is:
%
%      ssize_t NTTellDirectory(DIR *entry)
%
%  A description of each parameter follows:
%
%    o entry: Specifies a pointer to a DIR structure.
%
*/
MagickExport ssize_t NTTellDirectory(DIR *entry)
{
  assert(entry != (DIR *) NULL);
  (void) entry;
  return(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T T r u n c a t e F i l e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTTruncateFile() truncates a file to a specified length.
%
%  The format of the NTTruncateFile method is:
%
%      int NTTruncateFile(int file,off_t length)
%
%  A description of each parameter follows:
%
%    o file: the file.
%
%    o length: the file length.
%
*/
MagickExport int NTTruncateFile(int file,off_t length)
{
  DWORD
    file_pointer;

  HANDLE
    file_handle;

  long
    high,
    low;

  file_handle=(HANDLE) _get_osfhandle(file);
  if (file_handle == (HANDLE) -1L)
    return(-1);
  low=(long) (length & 0xffffffffUL);
  high=(long) ((((MagickOffsetType) length) >> 32) & 0xffffffffUL);
  file_pointer=SetFilePointer(file_handle,low,&high,FILE_BEGIN);
  if ((file_pointer == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
    return(-1);
  if (SetEndOfFile(file_handle) == 0)
    return(-1);
  return(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  N T U n m a p M e m o r y                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTUnmapMemory() emulates the Unix munmap method.
%
%  The format of the NTUnmapMemory method is:
%
%      int NTUnmapMemory(void *map,size_t length)
%
%  A description of each parameter follows:
%
%    o map: the address of the binary large object.
%
%    o length: the length of the binary large object.
%
*/
MagickExport int NTUnmapMemory(void *map,size_t length)
{
  (void) length;
  if (UnmapViewOfFile(map) == 0)
    return(-1);
  return(0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T U s e r T i m e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTUserTime() returns the total time the process has been scheduled (e.g.
%  seconds) since the last call to StartTimer().
%
%  The format of the UserTime method is:
%
%      double NTUserTime(void)
%
*/
MagickExport double NTUserTime(void)
{
  DWORD
    status;

  FILETIME
    create_time,
    exit_time;

  OSVERSIONINFO
    OsVersionInfo;

  union
  {
    FILETIME
      filetime;

    __int64
      filetime64;
  } kernel_time;

  union
  {
    FILETIME
      filetime;

    __int64
      filetime64;
  } user_time;

  OsVersionInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
  GetVersionEx(&OsVersionInfo);
  if (OsVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT)
    return(NTElapsedTime());
  status=GetProcessTimes(GetCurrentProcess(),&create_time,&exit_time,
    &kernel_time.filetime,&user_time.filetime);
  if (status != TRUE)
    return(0.0);
  return((double) 1.0e-7*(kernel_time.filetime64+user_time.filetime64));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N T W a r n i n g H a n d l e r                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTWarningHandler() displays a warning reason.
%
%  The format of the NTWarningHandler method is:
%
%      void NTWarningHandler(const ExceptionType severity,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o severity: Specifies the numeric warning category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
MagickExport void NTWarningHandler(const ExceptionType severity,
  const char *reason,const char *description)
{
  char
    buffer[2*MaxTextExtent];

  (void) severity;
  if (reason == (char *) NULL)
    return;
  if (description == (char *) NULL)
    (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s.\n",GetClientName(),
      reason);
  else
    (void) FormatLocaleString(buffer,MaxTextExtent,"%s: %s (%s).\n",
      GetClientName(),reason,description);
  (void) MessageBox(NULL,buffer,"ImageMagick Warning",MB_OK | MB_TASKMODAL |
    MB_SETFOREGROUND | MB_ICONINFORMATION);
}
#endif

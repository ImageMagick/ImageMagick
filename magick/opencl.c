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
%                                John Cristy                                  %
%                                 March 2000                                  %
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
%
%
*/
 
/*
Include declarations.
*/
#include <string.h>
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/compare.h"
#include "magick/constitute.h"
#include "magick/distort.h"
#include "magick/draw.h"
#include "magick/effect.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/fx.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/layer.h"
#include "magick/mime-private.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/montage.h"
#include "magick/morphology.h"
#include "magick/opencl.h"
#include "magick/opencl-private.h"
#include "magick/option.h"
#include "magick/policy.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/quantum.h"
#include "magick/resample.h"
#include "magick/resource_.h"
#include "magick/splay-tree.h"
#include "magick/semaphore.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"

#ifdef MAGICKCORE_CLPERFMARKER
#include "CLPerfMarker.h"
#endif


#if defined(MAGICKCORE_OPENCL_SUPPORT)

struct _MagickCLEnv {
  MagickBooleanType OpenCLInitialized;  /* whether OpenCL environment is initialized. */
  MagickBooleanType OpenCLDisabled;	/* whether if OpenCL has been explicitely disabled. */

  /*OpenCL objects */
  cl_platform_id platform;
  cl_device_type deviceType;
  cl_device_id device;
  cl_context context;

  cl_program programs[MAGICK_OPENCL_NUM_PROGRAMS]; /* one program object maps one kernel source file */

  SemaphoreInfo* lock;
};


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e M a g i c k O p e n C L E n v                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% AcquireMagickOpenCLEnv() allocates the MagickCLEnv structure 
%
*/

MagickExport MagickCLEnv AcquireMagickOpenCLEnv()
{
  MagickCLEnv clEnv;
  clEnv = (MagickCLEnv) AcquireMagickMemory(sizeof(struct _MagickCLEnv));
  if (clEnv != NULL)
  {
    memset(clEnv, 0, sizeof(struct _MagickCLEnv));
    AcquireSemaphoreInfo(&clEnv->lock);
  }
  return clEnv;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e l i n q u i s h M a g i c k O p e n C L E n v                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishMagickOpenCLEnv() destroy the MagickCLEnv structure
%
%  The format of the RelinquishMagickOpenCLEnv method is:
%
%      MagickBooleanType RelinquishMagickOpenCLEnv(MagickCLEnv clEnv)
%
%  A description of each parameter follows:
%
%    o clEnv: MagickCLEnv structure to destroy
%
*/

MagickExport MagickBooleanType RelinquishMagickOpenCLEnv(MagickCLEnv clEnv)
{
  if (clEnv != (MagickCLEnv)NULL)
  {
    RelinquishSemaphoreInfo(clEnv->lock);
    RelinquishMagickMemory(clEnv);
    return MagickTrue;
  }
  return MagickFalse;
}


/*
* Default OpenCL environment
*/
MagickCLEnv defaultCLEnv;
SemaphoreInfo* defaultCLEnvLock;


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t D e f a u l t O p e n C L E n v                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetDefaultOpenCLEnv() returns the default OpenCL env
%
%  The format of the GetDefaultOpenCLEnv method is:
%
%      MagickCLEnv GetDefaultOpenCLEnv(ExceptionInfo* exception)
%
%  A description of each parameter follows:
%
%    o exception: return any errors or warnings.
%
*/

MagickExport MagickCLEnv GetDefaultOpenCLEnv(ExceptionInfo* exception)
{ 
  if (defaultCLEnv == NULL)
  {
    if (defaultCLEnvLock == NULL)
    {
      AcquireSemaphoreInfo(&defaultCLEnvLock);
    }
    LockSemaphoreInfo(defaultCLEnvLock);
    defaultCLEnv = AcquireMagickOpenCLEnv();
    if (defaultCLEnv == NULL)
    {
      (void) ThrowMagickException(exception, GetMagickModule(), ResourceLimitError,
        "AcquireMagickMemory failed.",".");
      return NULL;
    }
    UnlockSemaphoreInfo(defaultCLEnvLock); 
  }
  return defaultCLEnv; 
}



/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t D e f a u l t O p e n C L E n v                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetDefaultOpenCLEnv() sets the new OpenCL environment as default 
%  and returns the old OpenCL environment
%  
%  The format of the SetDefaultOpenCLEnv() method is:
%
%      MagickCLEnv SetDefaultOpenCLEnv(MagickCLEnv clEnv)
%
%  A description of each parameter follows:
%
%    o clEnv: the new default OpenCL environment.
%
*/
MagickExport MagickCLEnv SetDefaultOpenCLEnv(MagickCLEnv clEnv)     
{

  MagickCLEnv oldEnv;

  if (defaultCLEnvLock == NULL)
  {
    AcquireSemaphoreInfo(&defaultCLEnvLock);
  }
  LockSemaphoreInfo(defaultCLEnvLock);
  oldEnv = defaultCLEnv;
  defaultCLEnv = clEnv;
  UnlockSemaphoreInfo(defaultCLEnvLock);

  return oldEnv;
} 



/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t M a g i c k O p e n C L E n v P a r a m                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetMagickOpenCLEnvParam() sets the parameters in the OpenCL environment  
%  
%  The format of the SetMagickOpenCLEnvParam() method is:
%
%      MagickBooleanType SetMagickOpenCLEnvParam(MagickCLEnv clEnv, 
%        MagickOpenCLEnvParam param, size_t dataSize, void* data, 
%        ExceptionInfo* exception)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
%    
%    o param: the parameter to be set.
%
%    o dataSize: the data size of the parameter value.
%
%    o data:  the pointer to the new parameter value
%
%    o exception: return any errors or warnings
%
*/

MagickExport
  MagickBooleanType SetMagickOpenCLEnvParam(MagickCLEnv clEnv, MagickOpenCLEnvParam param
                                          , size_t dataSize, void* data, ExceptionInfo* exception)
{
  MagickBooleanType status = MagickFalse;

  if (clEnv == NULL
    || data == NULL)
    goto cleanup;

  switch(param)
  {


  case MAGICK_OPENCL_ENV_PARAM_DEVICE:
    if (dataSize != sizeof(clEnv->device))
      goto cleanup;

    LockSemaphoreInfo(clEnv->lock);
    clEnv->device = *((cl_device_id*)data);
    clEnv->OpenCLInitialized = MagickFalse;
    UnlockSemaphoreInfo(clEnv->lock); 
    status = MagickTrue;
    break;

  case MAGICK_OPENCL_ENV_PARAM_OPENCL_DISABLED:
    if (dataSize != sizeof(clEnv->OpenCLDisabled))
      goto cleanup;

    LockSemaphoreInfo(clEnv->lock);
    clEnv->OpenCLDisabled =  *((MagickBooleanType*)data);
    UnlockSemaphoreInfo(clEnv->lock);
    status = MagickTrue;
    break;

  case MAGICK_OPENCL_ENV_PARAM_OPENCL_INITIALIZED:
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleWarning, "SetMagickOpenCLEnvParm cannot modify the OpenCL initialization state.", "'%s'", ".");
    break;

  default:
    goto cleanup;
  };

cleanup:
  return status;
}



/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M a g i c k O p e n C L E n v P a r a m                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMagickOpenCLEnvParam() gets the parameters in the OpenCL environment  
%  
%  The format of the GetMagickOpenCLEnvParam() method is:
%
%      MagickBooleanType GetMagickOpenCLEnvParam(MagickCLEnv clEnv, 
%        MagickOpenCLEnvParam param, size_t dataSize, void* data, 
%        ExceptionInfo* exception)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
%    
%    o param: the parameter to be returned.
%
%    o dataSize: the data size of the parameter value.
%
%    o data:  the location where the returned parameter value will be stored 
%
%    o exception: return any errors or warnings
%
*/

MagickExport
  MagickBooleanType GetMagickOpenCLEnvParam(MagickCLEnv clEnv, MagickOpenCLEnvParam param
                                          , size_t dataSize, void* data, ExceptionInfo* exception)
{
  MagickBooleanType status;
  status = MagickFalse;

  if (clEnv == NULL
    || data == NULL)
    goto cleanup;

  switch(param)
  {
  case MAGICK_OPENCL_ENV_PARAM_DEVICE:
    if (dataSize != sizeof(cl_device_id))
      goto cleanup;
    *((cl_device_id*)data) = clEnv->device;
    status = MagickTrue;
    break;

  case MAGICK_OPENCL_ENV_PARAM_OPENCL_DISABLED:
    if (dataSize != sizeof(clEnv->OpenCLDisabled))
      goto cleanup;
    *((MagickBooleanType*)data) = clEnv->OpenCLDisabled;
    status = MagickTrue;
    break;

  case MAGICK_OPENCL_ENV_PARAM_OPENCL_INITIALIZED:
    if (dataSize != sizeof(clEnv->OpenCLDisabled))
      goto cleanup;
    *((MagickBooleanType*)data) = clEnv->OpenCLInitialized;
    status = MagickTrue;
    break;

  default:
    goto cleanup;
  };

cleanup:
  return status;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t O p e n C L C o n t e x t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLContext() returns the OpenCL context  
%  
%  The format of the GetOpenCLContext() method is:
%
%      cl_context GetOpenCLContext(MagickCLEnv clEnv) 
%
%  A description of each parameter follows:
%
%    o clEnv: OpenCL environment
%
*/

MagickExport
cl_context GetOpenCLContext(MagickCLEnv clEnv) {
  if (clEnv == NULL)
    return NULL;
  else
    return clEnv->context;
}

static char* getBinaryCLProgramName(MagickCLEnv clEnv, MagickOpenCLProgram prog, unsigned int signature)
{
  char* name;
  char buffer[256];
  char deviceName[128];
  const char* prefix = "magick_opencl";
  clGetDeviceInfo(clEnv->device, CL_DEVICE_NAME, 128, deviceName, NULL);
  sprintf(buffer, "./%s_%s_%02d_%08x.bin", prefix, deviceName, (unsigned int)prog, signature);
  name = strdup(buffer);
  return name;
}

static MagickBooleanType saveBinaryCLProgram(MagickCLEnv clEnv, MagickOpenCLProgram prog, unsigned int signature, ExceptionInfo* exception)
{
  MagickBooleanType saveSuccessful;
  cl_int clStatus;
  size_t binaryProgramSize;
  unsigned char* binaryProgram;
  char* binaryFileName;
  FILE* fileHandle;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  binaryProgram = NULL;
  binaryFileName = NULL;
  fileHandle = NULL;
  saveSuccessful = MagickFalse;

  clStatus = clGetProgramInfo(clEnv->programs[prog], CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &binaryProgramSize, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleFatalError, "clGetProgramInfo failed.", "'%s'", ".");
    goto cleanup;
  }

  binaryProgram = (unsigned char*) AcquireMagickMemory(binaryProgramSize);
  clStatus = clGetProgramInfo(clEnv->programs[prog], CL_PROGRAM_BINARIES, sizeof(char*), &binaryProgram, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleFatalError, "clGetProgramInfo failed.", "'%s'", ".");
    goto cleanup;
  }

  binaryFileName = getBinaryCLProgramName(clEnv, prog, signature);
  fileHandle = fopen(binaryFileName, "wb");
  if (fileHandle != NULL)
  {
    fwrite(binaryProgram, sizeof(char), binaryProgramSize, fileHandle);
    saveSuccessful = MagickTrue;
  }
  else
  {
    (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
      "Saving binary kernel failed.", "'%s'", ".");
  }

cleanup:
  if (fileHandle != NULL)
    fclose(fileHandle);
  if (binaryProgram != NULL)
    RelinquishMagickMemory(binaryProgram);
  if (binaryFileName != NULL)
    free(binaryFileName);

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif

  return saveSuccessful;
}

static MagickBooleanType loadBinaryCLProgram(MagickCLEnv clEnv, MagickOpenCLProgram prog, unsigned int signature, ExceptionInfo* exception)
{
  MagickBooleanType loadSuccessful;
  unsigned char* binaryProgram;
  char* binaryFileName;
  FILE* fileHandle;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  binaryProgram = NULL;
  binaryFileName = NULL;
  fileHandle = NULL;
  loadSuccessful = MagickFalse;

  binaryFileName = getBinaryCLProgramName(clEnv, prog, signature);
  fileHandle = fopen(binaryFileName, "rb");
  if (fileHandle != NULL)
  {
    int b_error;
    size_t length;
    cl_int clStatus;
    cl_int clBinaryStatus;

    b_error = 0 ;
    length = 0;
    b_error |= fseek( fileHandle, 0, SEEK_END ) < 0;
    b_error |= ( length = ftell( fileHandle ) ) <= 0;
    b_error |= fseek( fileHandle, 0, SEEK_SET ) < 0;
    if( b_error )
      goto cleanup;

    binaryProgram = (unsigned char*)AcquireMagickMemory(length);
    if (binaryProgram == NULL)
      goto cleanup;

    memset(binaryProgram, 0, length);
    b_error |= fread(binaryProgram, 1, length, fileHandle) != length;

    clEnv->programs[prog] = clCreateProgramWithBinary(clEnv->context, 1, &clEnv->device, &length, (const unsigned char**)&binaryProgram, &clBinaryStatus, &clStatus);
    if (clStatus != CL_SUCCESS
        || clBinaryStatus != CL_SUCCESS)
      goto cleanup;

    loadSuccessful = MagickTrue;
  }

cleanup:
  if (fileHandle != NULL)
    fclose(fileHandle);
  if (binaryFileName != NULL)
    free(binaryFileName);
  if (binaryProgram != NULL)
    RelinquishMagickMemory(binaryProgram);

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif

  return loadSuccessful;
}

static unsigned int stringSignature(const char* string)
{
  unsigned int stringLength;
  unsigned int n,i,j;
  unsigned int signature;
  union
  {
    const char* s;
    const unsigned int* u;
  }p;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  stringLength = strlen(string);
  signature = stringLength;
  n = stringLength/sizeof(unsigned int);
  p.s = string;
  for (i = 0; i < n; i++)
  {
    signature^=p.u[i];
  }
  if (n * sizeof(unsigned int) != stringLength)
  {
    char padded[4];
    j = n * sizeof(unsigned int);
    for (i = 0; i < 4; i++,j++)
    {
      if (j < stringLength)
        padded[i] = p.s[j];
      else
        padded[i] = 0;
    }
    p.s = padded;
    signature^=p.u[0];
  }

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif

  return signature;
}

/* OpenCL kernels for accelerate.c */
extern const char *accelerateKernels, *accelerateKernels2;

static MagickBooleanType CompileOpenCLKernels(MagickCLEnv clEnv, ExceptionInfo* exception) 
{
  MagickBooleanType status = MagickFalse;
  cl_int clStatus;
  unsigned int i;
  char* accelerateKernelsBuffer = NULL;

  /* The index of the program strings in this array has to match the value of the enum MagickOpenCLProgram */
  const char* MagickOpenCLProgramStrings[MAGICK_OPENCL_NUM_PROGRAMS]; 

  char options[MaxTextExtent];
  unsigned int optionsSignature;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  /* Get additional options */
  (void) FormatLocaleString(options, MaxTextExtent, CLOptions, (float)QuantumRange,
    (float)QuantumScale, (float)CLCharQuantumScale, (float)MagickEpsilon, (float)MagickPI, (unsigned int)MaxMap, (unsigned int)MAGICKCORE_QUANTUM_DEPTH);

  /*
  if (getenv("MAGICK_OCL_DEF"))
  {
    strcat(options," ");
    strcat(options,getenv("MAGICK_OCL_DEF"));
  }
  */

  /*
  if (getenv("MAGICK_OCL_BUILD"))
    printf("options: %s\n", options);
  */

  optionsSignature = stringSignature(options);

  /* get all the OpenCL program strings here */
  accelerateKernelsBuffer = AcquireMagickMemory(strlen(accelerateKernels)+strlen(accelerateKernels2)+1);
  sprintf(accelerateKernelsBuffer,"%s%s",accelerateKernels,accelerateKernels2);
  MagickOpenCLProgramStrings[MAGICK_OPENCL_ACCELERATE] = accelerateKernelsBuffer;

  for (i = 0; i < MAGICK_OPENCL_NUM_PROGRAMS; i++) 
  {
    MagickBooleanType loadSuccessful = MagickFalse;
    unsigned int programSignature = stringSignature(MagickOpenCLProgramStrings[i]) ^ optionsSignature;

    /* try to load the binary first */
    if (!getenv("MAGICK_OCL_REC"))
      loadSuccessful = loadBinaryCLProgram(clEnv, (MagickOpenCLProgram)i, programSignature, exception);

    if (loadSuccessful == MagickFalse)
    {
      /* Binary CL program unavailable, compile the program from source */
      size_t programLength = strlen(MagickOpenCLProgramStrings[i]);
      clEnv->programs[i] = clCreateProgramWithSource(clEnv->context, 1, &(MagickOpenCLProgramStrings[i]), &programLength, &clStatus);
      if (clStatus!=CL_SUCCESS)
      {
        (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
          "clCreateProgramWithSource failed.", "(%d)", (int)clStatus);

        goto cleanup;
      }
    }

    clStatus = clBuildProgram(clEnv->programs[i], 1, &clEnv->device, options, NULL, NULL);
    if (clStatus!=CL_SUCCESS)
    {
      (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
        "clBuildProgram failed.", "(%d)", (int)clStatus);

      if (loadSuccessful == MagickFalse)
      {
        /*  dump the source into a file */
        FILE* fileHandle = fopen("magick_badcl.cl", "wb");	
        if (fileHandle != NULL)
        {
          fwrite(MagickOpenCLProgramStrings[i], sizeof(char), strlen(MagickOpenCLProgramStrings[i]), fileHandle);
          fclose(fileHandle);
        }

        /* dump the build log */
        {
          char* log;
          size_t logSize;
          clGetProgramBuildInfo(clEnv->programs[i], clEnv->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
          log = (char*)AcquireMagickMemory(logSize);
          clGetProgramBuildInfo(clEnv->programs[i], clEnv->device, CL_PROGRAM_BUILD_LOG, logSize, log, &logSize); 
          fileHandle = fopen("magick_badcl_build.log", "wb");	
          if (fileHandle != NULL)
          {
            const char* buildOptionsTitle = "build options: ";
            fwrite(buildOptionsTitle, sizeof(char), strlen(buildOptionsTitle), fileHandle);
            fwrite(options, sizeof(char), strlen(options), fileHandle);
            fwrite("\n",sizeof(char), 1, fileHandle);
            fwrite(log, sizeof(char), logSize, fileHandle);
            fclose(fileHandle);
          }
          RelinquishMagickMemory(log);
        }
      }
      goto cleanup;
    }

    if (loadSuccessful == MagickFalse)
    {
      /* Save the binary to a file to avoid re-compilation of the kernels in the future */
      saveBinaryCLProgram(clEnv, (MagickOpenCLProgram)i, programSignature, exception);
    }

  }
  status = MagickTrue;

cleanup:

  if (accelerateKernelsBuffer!=NULL) RelinquishMagickMemory(accelerateKernelsBuffer);

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif

  return status;
}

static MagickBooleanType InitOpenCLPlatformDevice(MagickCLEnv clEnv, ExceptionInfo* exception) {
  int i,j;
  cl_int status;
  cl_uint numPlatforms = 0;
  cl_platform_id *platforms = NULL;
  char* MAGICK_OCL_DEVICE = NULL;
  MagickBooleanType OpenCLAvailable = MagickFalse;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  /* check if there's an environment variable overriding the device selection */
  MAGICK_OCL_DEVICE = getenv("MAGICK_OCL_DEVICE");
  if (MAGICK_OCL_DEVICE != NULL)
  {
    if (strcmp(MAGICK_OCL_DEVICE, "CPU") == 0)
    {
      clEnv->deviceType = CL_DEVICE_TYPE_CPU;
    }
    else if (strcmp(MAGICK_OCL_DEVICE, "GPU") == 0)
    {
      clEnv->deviceType = CL_DEVICE_TYPE_GPU;
    }
    else if (strcmp(MAGICK_OCL_DEVICE, "OFF") == 0)
    {
      /* OpenCL disabled */
      goto cleanup;
    }
  }
  else if (clEnv->deviceType == 0) {
    clEnv->deviceType = CL_DEVICE_TYPE_ALL;
  }

  if (clEnv->device != NULL)
  {
    status = clGetDeviceInfo(clEnv->device, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &clEnv->platform, NULL);
    if (status != CL_SUCCESS) {
      (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
          "Failed to get OpenCL platform from the selected device.", "(%d)", status);
    }
    goto cleanup;
  }
  else if (clEnv->platform != NULL)
  {
    numPlatforms = 1;
    platforms = (cl_platform_id *) AcquireMagickMemory(numPlatforms * sizeof(cl_platform_id));
    if (platforms == (cl_platform_id *) NULL)
    {
      (void) ThrowMagickException(exception, GetMagickModule(), ResourceLimitError,
        "AcquireMagickMemory failed.",".");
      goto cleanup;
    }
    platforms[0] = clEnv->platform;
  }
  else
  {
    clEnv->device = NULL;

    /* Get the number of OpenCL platforms available */
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS)
    {
      (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning, 
        "clGetplatformIDs failed.", "(%d)", status);
      goto cleanup;
    }

    /* No OpenCL available, just leave */
    if (numPlatforms == 0) {
      goto cleanup;
    }

    platforms = (cl_platform_id *) AcquireMagickMemory(numPlatforms * sizeof(cl_platform_id));
    if (platforms == (cl_platform_id *) NULL)
    {
      (void) ThrowMagickException(exception, GetMagickModule(), ResourceLimitError,
        "AcquireMagickMemory failed.",".");
      goto cleanup;
    }

    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
    if (status != CL_SUCCESS)
    {
      (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
        "clGetPlatformIDs failed.", "(%d)", status);
      goto cleanup;
    }
  }

  /* Device selection */
  clEnv->device = NULL;
  for (j = 0; j < 2; j++) 
  {

    cl_device_type deviceType;
    if (clEnv->deviceType == CL_DEVICE_TYPE_ALL)
    {
      if (j == 0)
        deviceType = CL_DEVICE_TYPE_GPU;
      else
        deviceType = CL_DEVICE_TYPE_CPU;
    }
    else if (j == 1)
    {
      break;
    }
    else
      deviceType = clEnv->deviceType;

    for (i = 0; i < numPlatforms; i++)
    {
      cl_uint numDevices;
      status = clGetDeviceIDs(platforms[i], deviceType, 1, &(clEnv->device), &numDevices);
      if (status != CL_SUCCESS)
      {
        (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
          "clGetPlatformIDs failed.", "(%d)", status);
        goto cleanup;
      }
      if (clEnv->device != NULL)
      {
        clEnv->platform = platforms[i];
  goto cleanup;
      }
    }
  }

cleanup:
  if (platforms!=NULL)
    RelinquishMagickMemory(platforms);

  OpenCLAvailable = (clEnv->platform!=NULL
          && clEnv->device!=NULL)?MagickTrue:MagickFalse;

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif

  return OpenCLAvailable;
}

static MagickBooleanType EnableOpenCLInternal(MagickCLEnv clEnv) {
  if (clEnv->OpenCLInitialized == MagickTrue
    && clEnv->platform != NULL
    && clEnv->device != NULL) {
      clEnv->OpenCLDisabled = MagickFalse;
      return MagickTrue;
  }
  clEnv->OpenCLDisabled = MagickTrue;
  return MagickFalse;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t O p e n C L E n v                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitOpenCLEnv() initialize the OpenCL environment
%
%  The format of the RelinquishMagickOpenCLEnv method is:
%
%      MagickBooleanType InitOpenCLEnv(MagickCLEnv clEnv, ExceptionInfo* exception)
%
%  A description of each parameter follows:
%
%    o clEnv: OpenCL environment structure
%
%    o exception: return any errors or warnings.
%
*/

MagickExport
MagickBooleanType InitOpenCLEnv(MagickCLEnv clEnv, ExceptionInfo* exception) {
  MagickBooleanType status = MagickFalse;
  cl_int clStatus;
  cl_context_properties cps[3];

  if (clEnv == NULL)
    return MagickFalse;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  LockSemaphoreInfo(clEnv->lock);

  clEnv->OpenCLInitialized = MagickTrue;
  if (clEnv->OpenCLDisabled == MagickTrue)
    goto cleanup;

  clEnv->OpenCLDisabled = MagickTrue;
  /* setup the OpenCL platform and device */
  status = InitOpenCLPlatformDevice(clEnv, exception);
  if (status == MagickFalse) {
    /* No OpenCL device available */
    goto cleanup;
  }

  /* create an OpenCL context */
  cps[0] = CL_CONTEXT_PLATFORM;
  cps[1] = (cl_context_properties)clEnv->platform;
  cps[2] = 0;
  clEnv->context = clCreateContext(cps, 1, &(clEnv->device), NULL, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
        "clCreateContext failed.", "(%d)", clStatus);
    status = MagickFalse;
    goto cleanup;
  }

  status = CompileOpenCLKernels(clEnv, exception);
  if (status == MagickFalse) {
   (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
        "clCreateCommandQueue failed.", "(%d)", status);

    status = MagickFalse;
    goto cleanup;
  }

  status = EnableOpenCLInternal(clEnv);

cleanup:
  UnlockSemaphoreInfo(clEnv->lock);


#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif

  return status;
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
%      cl_command_queue AcquireOpenCLCommandQueue(MagickCLEnv clEnv)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
%
*/

MagickExport
cl_command_queue AcquireOpenCLCommandQueue(MagickCLEnv clEnv)
{
  if (clEnv != NULL)
    return clCreateCommandQueue(clEnv->context, clEnv->device, 0, NULL);
  else
    return NULL;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e l i n q u i s h O p e n C L C o m m a n d Q u e u e                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishOpenCLCommandQueue() releases the OpenCL command queue
%
%  The format of the RelinquishOpenCLCommandQueue method is:
%
%      MagickBooleanType RelinquishOpenCLCommandQueue(MagickCLEnv clEnv,
%        cl_command_queue queue)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
%
%    o queue: the OpenCL queue to be released.
%
%
*/
MagickExport
MagickBooleanType RelinquishOpenCLCommandQueue(MagickCLEnv clEnv, cl_command_queue queue)
{
  if (clEnv != NULL)
  {
    return ((clReleaseCommandQueue(queue) == CL_SUCCESS) ? MagickTrue:MagickFalse);
  }
  else
    return MagickFalse;
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

MagickExport
  cl_kernel AcquireOpenCLKernel(MagickCLEnv clEnv, MagickOpenCLProgram program, const char* kernelName)
{
  cl_int clStatus;
  cl_kernel kernel = NULL;
  if (clEnv != NULL && kernelName!=NULL)
  {
    kernel = clCreateKernel(clEnv->programs[program], kernelName, &clStatus);
  }
  return kernel;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e l i n q u i s h O p e n C L K e r n e l                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishOpenCLKernel() releases an OpenCL kernel
%
%  The format of the RelinquishOpenCLKernel method is:
%
%    MagickBooleanType RelinquishOpenCLKernel(MagickCLEnv clEnv,
%      cl_kernel kernel)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
%
%    o kernel: the OpenCL kernel object to be released.
%
%
*/

MagickExport
  MagickBooleanType RelinquishOpenCLKernel(MagickCLEnv clEnv, cl_kernel kernel)
{
  MagickBooleanType status = MagickFalse;
  if (clEnv != NULL && kernel != NULL)
  {
    status = ((clReleaseKernel(kernel) == CL_SUCCESS)?MagickTrue:MagickFalse);
  }
  return status;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t O p e n C L D e v i c e L o c a l M e m o r y S i z e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOpenCLDeviceLocalMemorySize() returns local memory size of the device
%
%  The format of the GetOpenCLDeviceLocalMemorySize method is:
%
%    unsigned long GetOpenCLDeviceLocalMemorySize(MagickCLEnv clEnv)
%
%  A description of each parameter follows:
%
%    o clEnv: the OpenCL environment.
%
%
*/

MagickExport
 unsigned long GetOpenCLDeviceLocalMemorySize(MagickCLEnv clEnv)
{
  cl_ulong localMemorySize;
  clGetDeviceInfo(clEnv->device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &localMemorySize, NULL);
  return (unsigned long)localMemorySize;
}

#else

struct _MagickCLEnv {
  MagickBooleanType OpenCLInitialized;  /* whether OpenCL environment is initialized. */
};

extern MagickExport MagickCLEnv AcquireMagickOpenCLEnv()
{
  return NULL;
}

extern MagickExport MagickBooleanType RelinquishMagickOpenCLEnv(
  MagickCLEnv magick_unused(clEnv))
{
  magick_unreferenced(clEnv);

  return MagickFalse;
}

/*
* Return the OpenCL environment
*/ 
MagickExport MagickCLEnv GetDefaultOpenCLEnv(
  ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(exception);

  return (MagickCLEnv) NULL;
}

MagickExport MagickCLEnv SetDefaultOpenCLEnv(
  MagickCLEnv magick_unused(clEnv))
{
  magick_unreferenced(clEnv);

  return (MagickCLEnv) NULL;
} 

MagickExport MagickBooleanType SetMagickOpenCLEnvParam(
  MagickCLEnv magick_unused(clEnv),MagickOpenCLEnvParam magick_unused(param),
  size_t magick_unused(dataSize),void *magick_unused(data),
  ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(clEnv);
  magick_unreferenced(param);
  magick_unreferenced(dataSize);
  magick_unreferenced(data);
  magick_unreferenced(exception);

  return MagickFalse;
}

MagickExport MagickBooleanType GetMagickOpenCLEnvParam(
  MagickCLEnv magick_unused(clEnv),MagickOpenCLEnvParam magick_unused(param),
  size_t magick_unused(dataSize),void *magick_unused(data),
  ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(clEnv);
  magick_unreferenced(param);
  magick_unreferenced(dataSize);
  magick_unreferenced(data);
  magick_unreferenced(exception);

  return MagickFalse;
}

MagickExport MagickBooleanType InitOpenCLEnv(MagickCLEnv magick_unused(clEnv),
  ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(clEnv);
  magick_unreferenced(exception);

  return MagickFalse;
}

MagickExport cl_command_queue AcquireOpenCLCommandQueue(
  MagickCLEnv magick_unused(clEnv))
{
  magick_unreferenced(clEnv);

  return (cl_command_queue) NULL;
}

MagickExport MagickBooleanType RelinquishCommandQueue(
  MagickCLEnv magick_unused(clEnv),cl_command_queue magick_unused(queue))
{
  magick_unreferenced(clEnv);
  magick_unreferenced(queue);

  return MagickFalse;
}

MagickExport cl_kernel AcquireOpenCLKernel(
  MagickCLEnv magick_unused(clEnv),MagickOpenCLProgram magick_unused(program),
  const char *magick_unused(kernelName))
{
  magick_unreferenced(clEnv);
  magick_unreferenced(program);
  magick_unreferenced(kernelName);

  return (cl_kernel)NULL;
}

MagickExport MagickBooleanType RelinquishOpenCLKernel(
  MagickCLEnv magick_unused(clEnv),cl_kernel magick_unused(kernel))
{
  magick_unreferenced(clEnv);
  magick_unreferenced(kernel);

  return MagickFalse;
}

MagickExport unsigned long GetOpenCLDeviceLocalMemorySize(
  MagickCLEnv magick_unused(clEnv))
{
  magick_unreferenced(clEnv);

  return 0;
}

#endif /* MAGICKCORE_OPENCL_SUPPORT */

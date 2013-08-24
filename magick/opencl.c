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
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"

#ifdef MAGICKCORE_CLPERFMARKER
#include "CLPerfMarker.h"
#endif

#if defined(MAGICKCORE_OPENCL_SUPPORT)

#if defined(MAGICKCORE_HDRI_SUPPORT)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable -DMAGICKCORE_HDRI_SUPPORT=1 "\
  "-DCLQuantum=float -DCLPixelType=float4 -DQuantumRange=%f " \
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  " -DMaxMap=%u"
#define CLPixelPacket  cl_float4
#define CLCharQuantumScale 1.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 8)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=uchar -DCLPixelType=uchar4 -DQuantumRange=%f " \
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u "
#define CLPixelPacket  cl_uchar4
#define CLCharQuantumScale 1.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 16)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=ushort -DCLPixelType=ushort4 -DQuantumRange=%f "\
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u "
#define CLPixelPacket  cl_ushort4
#define CLCharQuantumScale 257.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 32)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=uint -DCLPixelType=uint4 -DQuantumRange=%f "\
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u "
#define CLPixelPacket  cl_uint4
#define CLCharQuantumScale 16843009.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 64)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=ussize_t -DCLPixelType=ussize_t4 -DQuantumRange=%f "\
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u "
#define CLPixelPacket  cl_uint4
#define CLCharQuantumScale 72340172838076673.0f
#endif

static SemaphoreInfo* gpu_env_semaphore = (SemaphoreInfo*)NULL;
GPUEnv gpu_env;

/* 
* Release OpenCL buffers.
*/
MagickBooleanType ReleaseCLBuffers(KernelEnv *env)
{
  if (env->filtered_pixels != (cl_mem) NULL)
    clReleaseMemObject(env->filtered_pixels);
  if (env->pixels != (cl_mem) NULL)
    clReleaseMemObject(env->pixels);
  if (env->filter != (cl_mem) NULL)
    clReleaseMemObject(env->filter);
  if (env->buffer3 != (cl_mem) NULL)
    clReleaseMemObject(env->buffer3);
  if (env->map != (cl_mem) NULL)
    clReleaseMemObject(env->map);
  if (env->offset != (cl_mem) NULL)
    clReleaseMemObject(env->offset);
  if (env->cossin_theta != (cl_mem) NULL)
    clReleaseMemObject(env->cossin_theta);
  return(MagickTrue);
}

#if !defined(MAGICKCORE_WINDOWS_SUPPORT)
#define __stdcall __attribute__((__stdcall__))
#endif

/*
static void __stdcall CLNotify(const char *message,const void *data,size_t length,
                              void *user_context)
{
  ExceptionInfo
    *exception;

  (void) data;
  (void) length;
  exception=(ExceptionInfo *) user_context;
  (void) ThrowMagickException(exception,GetMagickModule(),DelegateWarning,
    "DelegateFailed","`%s'",message);
}
*/

/* 
* Initialize OpenCL environemnt.
*/
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I n i t i a l i z e C L E n v                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeCLEnv() prepare the envionment for running opencl programs.
%
%  The format of the InitializeCLEnv method is:
%
%      MagickBooleanType InitializeCLEnv(GPUEnv *gpu_info,
%        ExceptionInfo * exception)
%
%  A description of each parameter follows:
%
%    o gpu_info: gpu information.
%
%    o exception: return any errors or warnings in this structure.
%
*/


static MagickBooleanType initCLPlatform(GPUEnv* gpu_info, ExceptionInfo *exception)
{
  int i,t;
  cl_int status;
  cl_uint numPlatforms;
  cl_platform_id *platforms;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD("clGetPlatformIDs #1","");
#endif

  /*
  * Have a look at the available platforms.
  */
  status = clGetPlatformIDs(0, NULL, &numPlatforms);

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif

  if (status != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning, 
      "clGetplatformIDs failed.", "(%d)", status);
    return MagickFalse;
  }


  gpu_info->platform = NULL;    
  if ( 0 < numPlatforms )
  {
    platforms = (cl_platform_id *) AcquireMagickMemory(numPlatforms * sizeof(cl_platform_id));
    if (platforms == (cl_platform_id *) NULL)
    {
      (void) ThrowMagickException(exception, GetMagickModule(), ResourceLimitError,
        "AcquireMagickMemory failed.",".");
      return(MagickFalse);
    }

#ifdef MAGICKCORE_CLPERFMARKER
    clBeginPerfMarkerAMD("clGetPlatformIDs #2","");
#endif
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
#ifdef MAGICKCORE_CLPERFMARKER
    clEndPerfMarkerAMD();
#endif

    if (status != CL_SUCCESS)
    {
      (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
        "clGetPlatformIDs failed.", "(%d)", status);
      return(MagickFalse);
    }

    if (gpu_info->deviceType == 0)
      gpu_info->deviceType = CL_DEVICE_TYPE_GPU;


    for (t = 0; t < 2; t++)
    {
      cl_device_type deviceType;
      if (gpu_info->deviceType != 0)
      {
        deviceType = gpu_info->deviceType;
      }
      else
      {
        switch(t)
        {
        case 0:
          deviceType = CL_DEVICE_TYPE_GPU;
          break;
        case 1:
        default:
          deviceType = CL_DEVICE_TYPE_CPU;
          break;
        }
      }

      for (i = 0; i < numPlatforms; i++)
      {
        cl_uint numDevices;
        status = clGetDeviceIDs(platforms[i], deviceType, 0, NULL, &numDevices);
        if (numDevices!=0)
        {
          gpu_info->deviceType = deviceType;
          gpu_info->platform = platforms[i];
          break;
        }
      }
      if (gpu_info->deviceType!=0)
        break;
    }
    RelinquishMagickMemory(platforms);
  }
  return MagickTrue;
}


static MagickBooleanType initCLDevice(GPUEnv* gpu_info, ExceptionInfo* exception)
{
  cl_int status;
  cl_context_properties cps[3];
  
  status = clGetDeviceIDs(gpu_info->platform, gpu_info->deviceType, 1, &(gpu_info->device), NULL);
  if (status != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
				"clGetDeviceIDs failed.", "(%d)", status);
    return(MagickFalse);
  }

  cps[0] = CL_CONTEXT_PLATFORM;
  cps[1] = (cl_context_properties)gpu_info->platform;
  cps[2] = 0;
  gpu_info->context = clCreateContext(cps, 1, &(gpu_info->device), NULL, NULL, &status);
  if (status != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
				"clCreateContext failed.", "(%d)", status);
    return(MagickFalse);
  }

  /* Create OpenCL command queue. */
  gpu_info->command_queue = clCreateCommandQueue(gpu_info->context, gpu_info->device, 0, &status);
  if (status != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
				"clCreateCommandQueue failed.", "(%d)", status);
    return(MagickFalse);
  }

  return MagickTrue;
}

MagickBooleanType InitializeCLEnv(GPUEnv *gpu_info, ExceptionInfo *exception)
{
  MagickBooleanType status;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  /* initialize the platform if a platform hasn't been specified */
  if (gpu_info->platform==NULL)
  {
    status = initCLPlatform(gpu_info,exception);
    if (status==MagickFalse)
      return MagickFalse;

    // no OpenCL platform found, quit
    if (gpu_info->platform==NULL)
      return MagickTrue;
  }
  status = initCLDevice(gpu_info, exception);
  return status;
}


/* 
* Register kernel wrapper.
*/
MagickBooleanType RegisterKernelWrapper(const char *kernel_name,
                                        cl_kernel_function function)
{
  int i;
  for (i=0; i < gpu_env.kernel_count; i++)
  {
    if (strcasecmp(kernel_name, gpu_env.kernel_names[i])==0)
    {
      gpu_env.kernel_functions[i] = function;
      return(MagickTrue);
    }
  }
  return(MagickFalse);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     G e t K e r n e l E n v A n d f u n c                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetKernelEnvAndfunc() gets opencl environment information of a kernel
%
%  The format of the GetKernelEnvAndfunc method is:
%
%      MagickBooleanType GetKernelEnvAndfunc(const char *kernel_name, 
%        KernelEnv *env, cl_kernel_function *function)
%
%  A description of each parameter follows:
%
%    o kernel_name: name of the kernel.
%
%    o env: kernel environment.
%
%    o function: function content.
%
*/

MagickBooleanType GetKernelEnvAndfunc(const char *kernel_name, KernelEnv *env, 
                                      cl_kernel_function *function)
{
  int i;
  for (i=0; i < gpu_env.kernel_count; i++)
  {
    if (strcasecmp(kernel_name, gpu_env.kernel_names[i])==0)
    {
      env->device = gpu_env.device;
      env->context = gpu_env.context;
      env->command_queue= gpu_env.command_queue;
      env->program = gpu_env.programs[0];
      env->kernel = gpu_env.kernels[i];
      *function = gpu_env.kernel_functions[i];
      return(MagickTrue);
    }    
  }
  return(MagickFalse);
}



/* 
* Release OpenCL environment.
*/
MagickBooleanType ReleaseCLEnv(const GPUEnv* env, ExceptionInfo *exception)
{
  cl_int status;
  int i;
  MagickBooleanType error = MagickFalse;

  for (i=0; i<env->kernel_count; i++)
  { 
    status = clReleaseKernel(env->kernels[i]);
    if (status != CL_SUCCESS)
    {
      (void) ThrowMagickException(
        exception, GetMagickModule(), DelegateWarning,
        "clReleaseKernel failed.", "(%d)", status);
      error = MagickTrue;
    }
  }
  for (i=0; i<env->file_count; i++)
  {
    status = clReleaseProgram(env->programs[i]);
    if (status != CL_SUCCESS)
    {
      (void) ThrowMagickException(
        exception, GetMagickModule(), DelegateWarning,
        "clReleaseProgram failed.", "(%d)", status);
      error = MagickTrue;
    }
  }
  if (env->command_queue != NULL)
  {
    status = clReleaseCommandQueue(env->command_queue);
    if (status != CL_SUCCESS)
    {
      (void) ThrowMagickException(
        exception, GetMagickModule(), DelegateWarning,
        "clReleaseCommandQueue failed.", "(%d)", status);
      error = MagickTrue;
    }
  }

  if (env->context != NULL)
  {
    status = clReleaseContext(env->context);
    if (status != CL_SUCCESS)
    {
      (void) ThrowMagickException(
        exception, GetMagickModule(), DelegateWarning,
        "clReleaseContext failed.", "(%d)", status);
      error = MagickTrue;
    }
  }
  return(!error);
}


static MagickBooleanType IsCachedOfKernelProgram(const GPUEnv *gpu_env, const char * cl_file_name)
{
  int i ;
  for (i=0; i < gpu_env->file_count; i++)
  {
    if (strcasecmp(gpu_env->kernel_filename[i], cl_file_name)==0)
    {
      if(gpu_env->programs[i] != NULL)
        return(MagickTrue);
    }    
  }

  return(MagickFalse);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s G e n e r a t e d C L B i n a r y B y C L S o u r c e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsGeneratedCLBinaryByCLSource() compiles opencl kernel file in to a binary 
%  executable  on current device.
%
%  The format of the IsGeneratedCLBinaryByCLSource method is:
%
%      MagickBooleanType IsGenerateCLBinaryByCLSource(cl_context context,
%        const char * cl_file_name, FILE** fhandle).
%
%  A description of each parameter follows:
%
%    o context: context of opencl environment.
%
%    o cl_file_name: file name.
%
%    o fhandle: pointer to file pointer.
%
*/

MagickBooleanType IsGeneratedCLBinaryByCLSource(cl_context context , const char * cl_file_name)
{
  int i = 0;
  cl_int status;
  size_t numDevices;
  cl_device_id *devices;
  FILE *fd = NULL;
  MagickBooleanType binaryExisted;

  status = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(numDevices), &numDevices, NULL);
  if(status != CL_SUCCESS)
    return MagickFalse;

  devices = (cl_device_id *)malloc(sizeof(cl_device_id) * numDevices);
  if(devices == NULL)
    return MagickFalse;

  /* grab the handles to all of the devices in the context. */
  status = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * numDevices, devices, NULL);

  binaryExisted = numDevices>0?MagickTrue:MagickFalse;
  /* dump out each binary into its own separate file. */
  for(i = 0; i < numDevices; i++)
  {
    char fileName[1024] = {0};
    if(devices[i] != 0)
    {
      char deviceName[1024];
      status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
      sprintf(fileName, "./%s-%s.bin", cl_file_name, deviceName);
      fd = fopen(fileName, "rb");
      if (fd!=NULL)
      {
        fclose(fd);
      }
      else
      {
        binaryExisted = MagickFalse;
        break;
      }
    }
  }

  if(devices != NULL)
  {
    free(devices);
    devices = NULL;
  }
  return binaryExisted;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o n v e r t T o S t r i n g                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvertToString() reads kernel functions in accelerate.cl file to memory.   
%
%  The format of the ConvertToString method is:
%
%      MagickBooleanType ConvertToString(const char * filename,
%        char **source, GPUEnv *gpu_info, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the file to  read.
%
%    o source: the memory stores kernel function.
%
%    o gpu_info: gpu infomation.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickBooleanType ConvertToString(char **source, char* accelerate_kernels[])
{
  int i,j;
  size_t str_size = 0;
  for (i = 0; accelerate_kernels[i]!=NULL; i++) {
    str_size+=strlen(accelerate_kernels[i]);
  }
  *source = (char *) AcquireMagickMemory(sizeof(char) * str_size + 1);
  strcpy(*source, accelerate_kernels[0]);
  for (j = 1; j < i; j++) {
    strncat(*source, accelerate_kernels[j], strlen(accelerate_kernels[j]));
  }

  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     w r i t e B i n a ry T o F i l e                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  writeBinaryToFile() writes opencl environment information to a binary file
%
%  The format of the writeBinaryToFile method is:
%
%      MagickBooleanType writeBinaryToFile(const char* fileName,
%        const char * birary, size_t numBytes)
%
%  A description of each parameter follows:
%
%    o filename: file to written.
%
%    o birary: environment infomation.
%
%    o numBytes: length of birary.
%
*/
MagickBooleanType writeBinaryToFile(const char* fileName, const char* birary, size_t numBytes)
{
  FILE *output = NULL;
  output = fopen(fileName, "wb");
  if(output == NULL)
    return MagickFalse;

  fwrite(birary, sizeof(char), numBytes, output);
  fclose(output);

  return MagickTrue;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     G e n e r a t e C L B i n a r y F i l e F ro m C L S o u r c e          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GenerateCLBinaryFileFromCLSource() creates binary file to save open opencl 
%  environment information by opencl kernel code.
%
%  The format of the GenerateCLBinaryFileFromCLSource method is:
%
%      MagickBooleanType GenerateCLBinaryFileFromCLSource(cl_program program,
%        const char * cl_file_name)
%
%  A description of each parameter follows:
%
%    o program: kernel function.
%
%    o cl_file_name: binary file name.
%
*/
MagickBooleanType GenerateCLBinaryFileFromCLSource( cl_program program , const char * cl_file_name)
{
  int i = 0;
  cl_int status;
  size_t *binarySizes,numDevices;
  cl_device_id *devices;
  char **binaries;

  status = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES,sizeof(numDevices),&numDevices,NULL);
  if (status!=CL_SUCCESS)
    return MagickFalse;

  devices = (cl_device_id *)malloc( sizeof(cl_device_id) * numDevices );
  if(devices == NULL)
    return MagickFalse;

  /* grab the handles to all of the devices in the program. */
  status = clGetProgramInfo(program, CL_PROGRAM_DEVICES, sizeof(cl_device_id) * numDevices,devices,NULL);
  if (status!=CL_SUCCESS)
    return MagickFalse;


  /* figure out the sizes of each of the binaries. */
  binarySizes = (size_t*)malloc( sizeof(size_t) * numDevices );
  status = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,sizeof(size_t) * numDevices, binarySizes, NULL);
  if (status!=CL_SUCCESS)
    return MagickFalse;


  /* copy over all of the generated binaries. */
  binaries = (char **)malloc( sizeof(char *) * numDevices );
  if(binaries == NULL)
    return MagickFalse;

  for(i = 0; i < numDevices; i++)
  {
    if(binarySizes[i] != 0)
    {
      binaries[i] = (char *)malloc( sizeof(char) * binarySizes[i]);
      if(binaries[i] == NULL)
        return MagickFalse;
    }
    else
      binaries[i] = NULL;
  }
  status = clGetProgramInfo(program, CL_PROGRAM_BINARIES,sizeof(char *) * numDevices, binaries, NULL);
  if (status!=CL_SUCCESS)
    return MagickFalse;


  /* dump out each binary into its own separate file. */
  for(i = 0; i < numDevices; i++)
  {
    char fileName[256] = {0};
    if(binarySizes[i] != 0)
    {
      char deviceName[1024];
      status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 
        sizeof(deviceName), deviceName, NULL);

      sprintf(fileName, "./%s-%s.bin", cl_file_name, deviceName);
      if(!writeBinaryToFile(fileName, binaries[i], binarySizes[i]))
        return MagickFalse;
    }
  }

  // Release all resouces and memory
  for(i = 0; i < numDevices; i++)
  {
    if(binaries[i] != NULL)
    {
      free(binaries[i]);
      binaries[i] = NULL;
    }
  }

  if(binaries != NULL)
  {
    free(binaries);
    binaries = NULL;
  }

  if(binarySizes != NULL)
  {
    free(binarySizes);
    binarySizes = NULL;
  }

  if(devices != NULL)
  {
    free(devices);
    devices = NULL;
  }

  return MagickTrue;
}

/*
* Compile OpenCL kernel file.
*/
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o m p i l e C L f i  l e                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompileCLfile() compiles kernel functions.
%
%  The format of the CompileCLfile method is:
%
%      MagickBooleanType CompileCLfile(const char *filename, GPUEnv *gpu_info,
%        const char *build_option, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: opencl kernel file name.
%
%    o gpu_info: gpu environment information.
%
%    o build_option: options when build opencl kernel.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickBooleanType CompileCLfile(const char *filename, char* accelerate_kernel[],
                                GPUEnv *gpu_info, 
                                const char *build_option, 
                                ExceptionInfo *exception)
{
  cl_int status;
  size_t length;
  char *source_str;
  const char *source ;
  size_t source_size[1];
  char *buildLog = NULL;
  int  binaryExisted ;
  size_t numDevices;
  int idx ;

  if (IsCachedOfKernelProgram(gpu_info, filename) == MagickTrue)
    return (MagickTrue);

  idx = gpu_info->file_count;

  binaryExisted = IsGeneratedCLBinaryByCLSource(gpu_info->context,filename);
  
  numDevices = 1;
  if (binaryExisted == MagickTrue)
  {
    int b_error;
    
    int i;
    cl_device_id *devices;
    unsigned char** binaries;
    size_t* binariesLengths;
    cl_int* binariesStatus;
    

     /* grab the handles to all of the devices in the context. */
    devices = (cl_device_id *) AcquireMagickMemory( sizeof(cl_device_id) * numDevices );
    if (devices == (cl_device_id*)NULL)
      return MagickFalse;
    status = clGetContextInfo(gpu_info->context, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * numDevices, devices, NULL);
    if (status!=CL_SUCCESS)
      return MagickFalse;

    binaries = (unsigned char**) AcquireMagickMemory(sizeof(unsigned char*)*numDevices);
    binariesLengths = (size_t*) AcquireMagickMemory(sizeof(size_t)*numDevices);
    for (i = 0; i < numDevices; i++) {
      char fileName[1024];
      char deviceName[1024];
      FILE *fd = NULL;

      status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
      sprintf(fileName, "./%s-%s.bin", filename, deviceName);
      fd = fopen(fileName, "rb");

      if (fd==NULL) return MagickFalse;

      b_error = 0 ;
      length = 0;
      b_error |= fseek( fd, 0, SEEK_END ) < 0;
      b_error |= ( length = ftell( fd ) ) <= 0;
      b_error |= fseek( fd, 0, SEEK_SET ) < 0;
      if( b_error )
        return  MagickFalse;

      binaries[i] = (unsigned char*)AcquireMagickMemory(length);
      binariesLengths[i] = length;
      if (!binaries[i])
        return  MagickFalse;
      memset(binaries[i], 0, length);
      b_error |= fread(binaries[i], 1, length, fd ) != length;
      fclose(fd);
    }
    binariesStatus = (cl_int*) AcquireMagickMemory(numDevices*sizeof(cl_int));
    gpu_info->programs[idx] = clCreateProgramWithBinary(gpu_info->context, numDevices, devices,
      binariesLengths, (const unsigned char**)binaries, binariesStatus, &status);
    if (status != CL_SUCCESS)
      return MagickFalse;
    for (i = 0; i < numDevices; i++)
    {
      if (binariesStatus[i]!=CL_SUCCESS)
        return MagickFalse;
    }

    // clean up
    RelinquishMagickMemory(devices);
    for (i = 0; i < numDevices; i++)
    {
      RelinquishMagickMemory(binaries[i]);
    }
    RelinquishMagickMemory(binaries);
    RelinquishMagickMemory(binariesLengths);
    RelinquishMagickMemory(binariesStatus);
  }
  else
  {
    status = ConvertToString(&source_str, accelerate_kernel);

    if (status == MagickFalse)
    {
      (void) ThrowMagickException(
        exception, GetMagickModule(), DelegateWarning,
        "ConvertToString failed.", "(%d)", status);
      return(MagickFalse);  
    }
    source = source_str;
    source_size[0] = strlen(source);

    /* create a CL program using the kernel source */
    gpu_info->programs[idx] = clCreateProgramWithSource(
      gpu_info->context, 1, &source, source_size, &status);
  }


  if ((gpu_info->programs[idx] == (cl_program)NULL) || (status != CL_SUCCESS))
  {
    (void) ThrowMagickException(
      exception, GetMagickModule(), DelegateWarning,
      "clCreateProgramWithSource failed.", "(%d)", status);
    return(MagickFalse);
  }

  /* create a cl program executable for all the devices specified */
  status = clBuildProgram(gpu_info->programs[idx], 1, &(gpu_info->device), 
    build_option, NULL, NULL);
  if (status != CL_SUCCESS)
  {
    FILE *fd1;
    status = clGetProgramBuildInfo(
      gpu_info->programs[idx], gpu_info->device,
      CL_PROGRAM_BUILD_LOG, 0, NULL, &length);
    if (status != CL_SUCCESS)
    {
      (void) ThrowMagickException(
        exception, GetMagickModule(), DelegateWarning, 
        "clGetProgramBuildInfo failed.", "(%d)", status);
      return(MagickFalse);
    }
    buildLog = (char *) AcquireMagickMemory(length);
    if (buildLog == (char *) NULL)
    {
      (void) ThrowMagickException(
        exception, GetMagickModule(), ResourceLimitError,
        "AcquireMagickMemory failed.",".");
      return(MagickFalse);
    }
    status = clGetProgramBuildInfo(
      gpu_info->programs[idx], gpu_info->device,
      CL_PROGRAM_BUILD_LOG, length, buildLog, &length);

    fd1 =fopen("kernel-build.log" , "w+");
    if(fd1 != NULL){
      fwrite(buildLog, sizeof(char), length, fd1);
      fclose(fd1);
    }

    fd1 =fopen("bad-kernel.cl" , "w+");
    if(fd1 != NULL){
      fwrite(source, sizeof(char), strlen(source), fd1);
      fclose(fd1);
    }


    fd1 =fopen("bad-kernel_compile_option.log" , "w+");
    if(fd1 != NULL){
      fwrite(build_option, sizeof(char), strlen(build_option), fd1);
      fclose(fd1);
    }

    (void) ThrowMagickException(
      exception, GetMagickModule(), DelegateWarning,
      "failed to build OpenCL progrm.", "(%s)", buildLog);
    buildLog = DestroyString(buildLog);
    return(MagickFalse);
  }

  strcpy(gpu_env.kernel_filename[idx] , filename);

  if(binaryExisted == MagickFalse)
    GenerateCLBinaryFileFromCLSource(gpu_env.programs[idx], filename);

  gpu_info->file_count += 1 ;

  return(MagickTrue);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I n i t C L K e r n e l E n v                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitCLKernelEnv() initializes opencl kernel environment.
%
%  The format of the InitCLKernelEnv method is:
%
%      MagickBooleanType InitCLKernelEnv(char *build_option, 
%                                        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o build_option: options of build kernels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickBooleanType InitCLKernelEnv(const char *build_option,
                                  const char *filename, char* accelerate_kernels[],
                                  ExceptionInfo *exception)
{
  MagickBooleanType initCLStatus = MagickTrue;

  MagickBooleanType status = MagickFalse;
  cl_int clStatus;
  cl_uint numKernels = 0;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  if (gpu_env_semaphore == (SemaphoreInfo*)NULL)
    AcquireSemaphoreInfo(&gpu_env_semaphore);
  LockSemaphoreInfo(gpu_env_semaphore);

  if(!gpu_env.isInited)
  {
    char options[MaxTextExtent];
    /* Get additional options */
    (void) FormatLocaleString(
      options, MaxTextExtent, CLOptions, (float)QuantumRange,
      (float)QuantumScale, (float)CLCharQuantumScale,
      (float)MagickEpsilon, (float)MagickPI, (unsigned int)MaxMap);
    if (build_option != NULL)
      strcat(options, build_option);


    /*initialize devices, context, comand_queue*/
    memset(&gpu_env , 0, sizeof(gpu_env));
    status = InitializeCLEnv(&gpu_env, exception);
    if (status == MagickFalse)
    {
      (void) ThrowMagickException(
        exception, GetMagickModule(), DelegateWarning,
        "InitializeCLEnv failed.", ".");
     
      initCLStatus = MagickFalse;
      goto unlock;
    }
    if (gpu_env.platform == NULL) 
    { 
      // no OpenCL platform found
      gpu_env.isInited = 1;
      goto unlock;
    }

    /*initialize program, kernel_name, kernel_count*/
    status = CompileCLfile(filename, accelerate_kernels, &gpu_env, options, exception);
    if (status == MagickFalse)
    {
      (void) ThrowMagickException(
        exception, GetMagickModule(), DelegateWarning,
        "CompileCLfile failed.", ".");
       initCLStatus = MagickFalse;
       goto unlock;  
    }

    /* Get the name of all the kernels */
    {
      cl_kernel kernels[MAX_NUM_KERNEL];
      cl_uint num_kernels = MAX_NUM_KERNEL;
      clStatus = clCreateKernelsInProgram(gpu_env.programs[gpu_env.file_count-1], num_kernels, kernels, &numKernels);
      if (clStatus!=CL_SUCCESS)
      {
        (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
          "clGetProgramInfo with CL_PROGRAM_NUM_KERNELS failed.", ".");
        
        initCLStatus = MagickFalse;
        goto unlock;  
      }


      if (numKernels > 0) 
      {
        int i;
        for (i = 0; i < numKernels; i++)
        {
          char kernelName[256];
          clStatus = clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, 256, kernelName, NULL);
          if (clStatus!=CL_SUCCESS)
          {
            (void) ThrowMagickException(exception, GetMagickModule(), DelegateWarning,
              "clGetKernelInfo with CL_KERNEL_FUNCTION_NAME failed.", ".");
            initCLStatus = MagickFalse;
            goto unlock;  
          }
          strcpy(gpu_env.kernel_names[gpu_env.kernel_count++], kernelName);
	  clReleaseKernel(kernels[i]);
        }
      }
    }

    /* initialize kernel */
    if (gpu_env.kernel_count == 0)
    {
      (void) ThrowMagickException(
        exception, GetMagickModule(), DelegateWarning, "No kernels.", ".");
      initCLStatus = MagickFalse;
      goto unlock;  
    }
    gpu_env.isInited = 1;
  }
  
unlock:
  UnlockSemaphoreInfo(gpu_env_semaphore);

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif
  return(initCLStatus);
}



/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R u n C L K e r n e l                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RunCLKernel() runs opencl kernel function
%
%  The format of the RunCLKernel method is:
%
%      MagickBooleanType RunCLKernel(const char *kernel_name, void **userdata)
%
%  A description of each parameter follows:
%
%    o kernel_name: name of kernel.
%
%    o userdata: user data.
%
*/
MagickBooleanType RunCLKernel(const char *kernel_name, void **userdata)
{
  KernelEnv env;
  cl_kernel_function function;
  MagickBooleanType status ;
  (void) ResetMagickMemory(&env, 0, sizeof(KernelEnv));
  status = GetKernelEnvAndfunc(kernel_name, &env, &function);
  strcpy(env.kernel_name, kernel_name);
  if (status == MagickTrue)
  {
    return(function(userdata, &env));
  }
  return(MagickFalse);
}

MagickBooleanType AccelerateFunctionCL( cl_kernel_function function,
                                       const char* filename, char* accelerate_kernels[],
                                       char * kernelName, 
                                       void **usrdata, 
                                       ExceptionInfo *exception)
{
  if (!gpu_env.isInited)
  {
    char *build_option = NULL;
    InitCLKernelEnv(build_option, filename, accelerate_kernels, exception);
  }
  if (gpu_env.platform == NULL)
     return MagickFalse;
  RegisterKernelWrapper(kernelName, function);
  return(RunCLKernel(kernelName, usrdata));
}


#endif // MAGICKCORE_OPENCL_SUPPORT

/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore OpenCL public methods.
*/
#ifndef _MAGICKCORE_OPENCL_H
#define _MAGICKCORE_OPENCL_H

/*
  Include declarations.
*/
#include <stdlib.h>
#include "magick/morphology.h"

#if defined(MAGICKCORE_OPENCL_SUPPORT)
#include <CL/cl.h>
#include "magick/opencl-private.h"
#endif


#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
  

#if defined(MAGICKCORE_OPENCL_SUPPORT)

typedef struct
{
  cl_context context;
  cl_device_id device;
  cl_command_queue command_queue;
  cl_program program;

  cl_kernel kernel;
  char kernel_name[64];

  cl_mem pixels;
  cl_uint rows;
  cl_uint columns;
  cl_mem filtered_pixels;

  cl_uint channel;
  char is_RGBA_BGRA;

  cl_mem filter;
  cl_uint width;
  cl_uint height;
  cl_uint matte;

  cl_int x_offset[4];  /* Despeckle */
  cl_int y_offset[4];  /* Despeckle */
  cl_mem offset;      /* MotionBlur */
  cl_mem cossin_theta;     /* RadialBlur */
  cl_mem map;
  cl_mem buffer3;
  cl_uint buffer3_length;
  size_t map_length;
  void *YCCMap;
}KernelEnv;

typedef MagickBooleanType (* cl_kernel_function) (void **userdata, KernelEnv *kenv);

typedef struct
{
  int isInited;  // whether OpenCL environment is initialized.

  //share vb in all modules in ImageMagick
  cl_platform_id platform;
  cl_device_type deviceType;
  cl_device_id device;
  cl_context context;

  cl_command_queue command_queue;
  cl_kernel kernels[MAX_NUM_CLFILE];
  cl_program programs[MAX_NUM_CLFILE]; //one program object maps one kernel source file
  char kernel_filename[MAX_NUM_CLFILE][256];   //the max len of kernel file name is 256
  cl_int file_count; // only one kernel file

  //following vb to be use by CORE_magick module
  char kernel_names[MAX_NUM_KERNEL][MAX_KERNEL_STRING_LEN+1];
  cl_kernel_function kernel_functions[MAX_NUM_KERNEL];
  cl_int kernel_count;
}GPUEnv;


extern GPUEnv gpu_env;

MagickBooleanType InitializeCLEnv(GPUEnv *gpu_info, ExceptionInfo *exception);

MagickBooleanType CompileCLfile(const char *filename, char* accelerate_kernel[],
                                GPUEnv *gpu_info, 
                                const char *build_option, 
                                ExceptionInfo *exception);

MagickBooleanType GetKernelEnvAndfunc(const char *kernel_name, KernelEnv *env, 
                                      cl_kernel_function *function);

MagickBooleanType RegisterKernelWrapper(const char *kernel_name,
                                        cl_kernel_function function);


MagickBooleanType AccelerateFunctionCL( cl_kernel_function function, 
				       const char* filename, char* accelerate_kernels[],
                                       char * kernelName, 
                                       void **usrdata, 
                                       ExceptionInfo *exception);

MagickBooleanType ReleaseCLEnv(const GPUEnv* env, ExceptionInfo *exception);
MagickBooleanType ReleaseCLBuffers(KernelEnv *env);

#endif //MAGICKCORE_OPENCL_SUPPORT

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

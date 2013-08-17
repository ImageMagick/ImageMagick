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

MagickCore OpenCL private methods.
*/
#ifndef _MAGICKCORE_OPENCL_PRIVATE_H
#define _MAGICKCORE_OPENCL_PRIVATE_H

/*
Include declarations.
*/
#include "magick/studio.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_OPENCL_SUPPORT)

#define MAX_KERNEL_STRING_LEN  64              /* maximum length of kernel name */
#define MAX_NUM_CLFILE         50              /* maximum number of .cl files */
#define MAX_NUM_KERNEL       200             /* maximum number of kernels */
#define CL_QUEUE_THREAD_HANDLE_AMD 0x403E      /* OpenCL handle optimization */
#define MAX_USRDATA_ARGS       32              /* maximum number of arguments */
#define HISTOGRAM_BIN_SIZE     256             /* number of bins in histogram */

  extern ModuleExport MagickBooleanType AccelerateGetCLEnvInfo(
    const char* build_option, void** device, void** context, 
    void** command_queue, void** program, MagickBooleanType* download_pels);

  extern  MagickBooleanType InitCLKernelEnv(
    const char *build_option,  const char *filename, char* accelerate_kernels[], ExceptionInfo *exception);

#endif /* MAGICKCORE_OPENCL_SUPPORT */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

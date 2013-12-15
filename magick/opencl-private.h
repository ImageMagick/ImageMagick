/*
Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
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
#include "magick/opencl.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(MAGICKCORE_OPENCL_SUPPORT)
  typedef void* cl_platform_id;
  typedef void* cl_device_id;
  typedef void* cl_context;
  typedef void* cl_command_queue;
  typedef void* cl_kernel;
  typedef struct { unsigned char t[8]; } cl_device_type; /* 64-bit */
#endif

#if defined(MAGICKCORE_HDRI_SUPPORT)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable -DMAGICKCORE_HDRI_SUPPORT=1 "\
  "-DCLQuantum=float -DCLSignedQuantum=float -DCLPixelType=float4 -DQuantumRange=%f " \
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  " -DMaxMap=%u -DMAGICKCORE_QUANTUM_DEPTH=%u"
#define CLPixelPacket  cl_float4
#define CLCharQuantumScale 1.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 8)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=uchar -DCLSignedQuantum=char -DCLPixelType=uchar4 -DQuantumRange=%f " \
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u -DMAGICKCORE_QUANTUM_DEPTH=%u"
#define CLPixelPacket  cl_uchar4
#define CLCharQuantumScale 1.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 16)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=ushort -DCLSignedQuantum=short -DCLPixelType=ushort4 -DQuantumRange=%f "\
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u -DMAGICKCORE_QUANTUM_DEPTH=%u"
#define CLPixelPacket  cl_ushort4
#define CLCharQuantumScale 257.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 32)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=uint -DCLSignedQuantum=int -DCLPixelType=uint4 -DQuantumRange=%f "\
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u -DMAGICKCORE_QUANTUM_DEPTH=%u"
#define CLPixelPacket  cl_uint4
#define CLCharQuantumScale 16843009.0f
#elif (MAGICKCORE_QUANTUM_DEPTH == 64)
#define CLOptions "-cl-single-precision-constant -cl-mad-enable " \
  "-DCLQuantum=ulong -DCLSignedQuantum=long -DCLPixelType=ulong4 -DQuantumRange=%f "\
  "-DQuantumScale=%f -DCharQuantumScale=%f -DMagickEpsilon=%f -DMagickPI=%f "\
  "-DMaxMap=%u -DMAGICKCORE_QUANTUM_DEPTH=%u"
#define CLPixelPacket  cl_ulong4
#define CLCharQuantumScale 72340172838076673.0f
#endif

extern MagickPrivate cl_context 
  GetOpenCLContext(MagickCLEnv);

extern MagickPrivate cl_kernel 
  AcquireOpenCLKernel(MagickCLEnv, MagickOpenCLProgram, const char*);

extern MagickPrivate cl_command_queue 
  AcquireOpenCLCommandQueue(MagickCLEnv);

extern MagickPrivate MagickBooleanType 
  OpenCLThrowMagickException(ExceptionInfo *,
    const char *,const char *,const size_t,
    const ExceptionType,const char *,const char *,...),
  RelinquishOpenCLCommandQueue(MagickCLEnv, cl_command_queue),
  RelinquishOpenCLKernel(MagickCLEnv, cl_kernel);

extern MagickPrivate unsigned long 
  GetOpenCLDeviceLocalMemorySize(MagickCLEnv),
  GetOpenCLDeviceMaxMemAllocSize(MagickCLEnv);

extern MagickPrivate const char* 
  GetOpenCLCachedFilesDirectory();

extern MagickPrivate void 
  OpenCLLog(const char*);

/* #define OPENCLLOG_ENABLED 1 */
static inline void OpenCLLogException(const char* function, 
                        const unsigned int line, 
                        ExceptionInfo* exception) {
#ifdef OPENCLLOG_ENABLED
  if (exception->severity!=0) {
    char message[MaxTextExtent];
    /*  dump the source into a file */
    (void) FormatLocaleString(message,MaxTextExtent,"%s:%d Exception(%d):%s "
        ,function,line,exception->severity,exception->reason);
    OpenCLLog(message);
  }
#else
  magick_unreferenced(function);
  magick_unreferenced(line);
  magick_unreferenced(exception);
#endif
}


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

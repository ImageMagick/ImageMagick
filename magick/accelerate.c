/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     AAA     CCCC    CCCC  EEEEE  L      EEEEE  RRRR    AAA   TTTTT  EEEEE   %
%    A   A   C       C      E      L      E      R   R  A   A    T    E       %
%    AAAAA   C       C      EEE    L      EEE    RRRR   AAAAA    T    EEE     %
%    A   A   C       C      E      L      E      R R    A   A    T    E       %
%    A   A    CCCC    CCCC  EEEEE  LLLLL  EEEEE  R  R   A   A    T    EEEEE   %
%                                                                             %
%                                                                             %
%                       MagickCore Acceleration Methods                       %
%                                                                             %
%                              Software Design                                %
%                               John Cristy                                   %
%                               January 2010                                  %
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
% Morphology is the the application of various kernals, of any size and even
% shape, to a image in various ways (typically binary, but not always).
%
% Convolution (weighted sum or average) is just one specific type of
% accelerate. Just one that is very common for image bluring and sharpening
% effects.  Not only 2D Gaussian blurring, but also 2-pass 1D Blurring.
%
% This module provides not only a general accelerate function, and the ability
% to apply more advanced or iterative morphologies, but also functions for the
% generation of many different types of kernel arrays from user supplied
% arguments. Prehaps even the generation of a kernel from a small image.
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/accelerate.h"
#include "magick/artifact.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/cache-view.h"
#include "magick/color-private.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/hashmap.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/memory_.h"
#include "magick/monitor-private.h"
#include "magick/accelerate.h"
#include "magick/option.h"
#include "magick/prepress.h"
#include "magick/quantize.h"
#include "magick/registry.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/token.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e C o n v o l v e I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AccelerateConvolveImage() applies a custom convolution kernel to the image.
%  It is accelerated by taking advantage of speed-ups offered by executing in
%  concert across heterogeneous platforms consisting of CPUs, GPUs, and other
%  processors.
%
%  The format of the AccelerateConvolveImage method is:
%
%      Image *AccelerateConvolveImage(const Image *image,
%        const KernelInfo *kernel,Image *convolve_image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o kernel: the convolution kernel.
%
%    o convole_image: the convoleed image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(MAGICKCORE_OPENCL_SUPPORT)

#if defined(MAGICKCORE_HDRI_SUPPORT)
#define CLOptions "-DMAGICKCORE_HDRI_SUPPORT=1 -DCLQuantum=float " \
  "-DCLPixelType=float4 -DQuantumRange=%g -DMagickEpsilon=%g"
#define CLPixelPacket  cl_float4
#else
#if (MAGICKCORE_QUANTUM_DEPTH == 8)
#define CLOptions "-DCLQuantum=uchar -DCLPixelType=uchar4 " \
  "-DQuantumRange=%g -DMagickEpsilon=%g"
#define CLPixelPacket  cl_uchar4
#elif (MAGICKCORE_QUANTUM_DEPTH == 16)
#define CLOptions "-DCLQuantum=ushort -DCLPixelType=ushort4 " \
  "-DQuantumRange=%g -DMagickEpsilon=%g"
#define CLPixelPacket  cl_ushort4
#elif (MAGICKCORE_QUANTUM_DEPTH == 32)
#define CLOptions "-DCLQuantum=uint -DCLPixelType=uint4 " \
  "-DQuantumRange=%g -DMagickEpsilon=%g"
#define CLPixelPacket  cl_uint4
#elif (MAGICKCORE_QUANTUM_DEPTH == 64)
#define CLOptions "-DCLQuantum=ussize_t -DCLPixelType=ussize_t4 " \
  "-DQuantumRange=%g -DMagickEpsilon=%g"
#define CLPixelPacket  cl_ulong4
#endif
#endif

typedef struct _ConvolveInfo
{
  cl_context
    context;

  cl_device_id
    *devices;

  cl_command_queue
    command_queue;

  cl_kernel
    kernel;

  cl_program
    program;

  cl_mem
    pixels,
    convolve_pixels;

  cl_ulong
    width,
    height;

  cl_uint
    matte;

  cl_mem
    filter;
} ConvolveInfo;

static char
  *ConvolveKernel =
    "static inline long ClampToCanvas(const long offset,const unsigned long range)\n"
    "{\n"
    "  if (offset < 0L)\n"
    "    return(0L);\n"
    "  if (offset >= range)\n"
    "    return((long) (range-1L));\n"
    "  return(offset);\n"
    "}\n"
    "\n"
    "static inline CLQuantum ClampToQuantum(const float value)\n"
    "{\n"
    "#if defined(MAGICKCORE_HDRI_SUPPORT)\n"
    "  return((CLQuantum) value);\n"
    "#else\n"
    "  if (value < 0.0)\n"
    "    return((CLQuantum) 0);\n"
    "  if (value >= (float) QuantumRange)\n"
    "    return((CLQuantum) QuantumRange);\n"
    "  return((CLQuantum) (value+0.5));\n"
    "#endif\n"
    "}\n"
    "\n"
    "static inline float PerceptibleReciprocal(const float x)\n"
    "{\n"
    "  float sign = x < (float) 0.0 ? (float) -1.0 : (float) 1.0;\n"
    "  return((sign*x) >= MagickEpsilon ? (float) 1.0/x : sign*((float) 1.0/\n"
    "    MagickEpsilon));\n"
    "}\n"
    "\n"
    "__kernel void Convolve(const __global CLPixelType *input,\n"
    "  __constant float *filter,const unsigned long width,const unsigned long height,\n"
    "  const unsigned int matte,__global CLPixelType *output)\n"
    "{\n"
    "  const unsigned long columns = get_global_size(0);\n"
    "  const unsigned long rows = get_global_size(1);\n"
    "\n"
    "  const long x = get_global_id(0);\n"
    "  const long y = get_global_id(1);\n"
    "\n"
    "  const float scale = (1.0/QuantumRange);\n"
    "  const long mid_width = (width-1)/2;\n"
    "  const long mid_height = (height-1)/2;\n"
    "  float4 sum = { 0.0, 0.0, 0.0, 0.0 };\n"
    "  float gamma = 0.0;\n"
    "  register unsigned long i = 0;\n"
    "\n"
    "  int method = 0;\n"
    "  if (matte != false)\n"
    "    method=1;\n"
    "  if ((x >= width) && (x < (columns-width-1)) &&\n"
    "      (y >= height) && (y < (rows-height-1)))\n"
    "    {\n"
    "      method=2;\n"
    "      if (matte != false)\n"
    "        method=3;\n"
    "    }\n"
    "  switch (method)\n"
    "  {\n"
    "    case 0:\n"
    "    {\n"
    "      for (long v=(-mid_height); v <= mid_height; v++)\n"
    "      {\n"
    "        for (long u=(-mid_width); u <= mid_width; u++)\n"
    "        {\n"
    "          const long index=ClampToCanvas(y+v,rows)*columns+\n"
    "            ClampToCanvas(x+u,columns);\n"
    "          sum.x+=filter[i]*input[index].x;\n"
    "          sum.y+=filter[i]*input[index].y;\n"
    "          sum.z+=filter[i]*input[index].z;\n"
    "          gamma+=filter[i];\n"
    "          i++;\n"
    "        }\n"
    "      }\n"
    "      break;\n"
    "    }\n"
    "    case 1:\n"
    "    {\n"
    "      for (long v=(-mid_height); v <= mid_height; v++)\n"
    "      {\n"
    "        for (long u=(-mid_width); u <= mid_width; u++)\n"
    "        {\n"
    "          const unsigned long index=ClampToCanvas(y+v,rows)*columns+\n"
    "            ClampToCanvas(x+u,columns);\n"
    "          const float alpha=scale*input[index].w;\n"
    "          sum.x+=alpha*filter[i]*input[index].x;\n"
    "          sum.y+=alpha*filter[i]*input[index].y;\n"
    "          sum.z+=alpha*filter[i]*input[index].z;\n"
    "          sum.w+=filter[i]*input[index].w;\n"
    "          gamma+=alpha*filter[i];\n"
    "          i++;\n"
    "        }\n"
    "      }\n"
    "      break;\n"
    "    }\n"
    "    case 2:\n"
    "    {\n"
    "      for (long v=(-mid_height); v <= mid_height; v++)\n"
    "      {\n"
    "        for (long u=(-mid_width); u <= mid_width; u++)\n"
    "        {\n"
    "          const unsigned long index=(y+v)*columns+(x+u);\n"
    "          sum.x+=filter[i]*input[index].x;\n"
    "          sum.y+=filter[i]*input[index].y;\n"
    "          sum.z+=filter[i]*input[index].z;\n"
    "          gamma+=filter[i];\n"
    "          i++;\n"
    "        }\n"
    "      }\n"
    "      break;\n"
    "    }\n"
    "    case 3:\n"
    "    {\n"
    "      for (long v=(-mid_height); v <= mid_height; v++)\n"
    "      {\n"
    "        for (long u=(-mid_width); u <= mid_width; u++)\n"
    "        {\n"
    "          const unsigned long index=(y+v)*columns+(x+u);\n"
    "          const float alpha=scale*input[index].w;\n"
    "          sum.x+=alpha*filter[i]*input[index].x;\n"
    "          sum.y+=alpha*filter[i]*input[index].y;\n"
    "          sum.z+=alpha*filter[i]*input[index].z;\n"
    "          sum.w+=filter[i]*input[index].w;\n"
    "          gamma+=alpha*filter[i];\n"
    "          i++;\n"
    "        }\n"
    "      }\n"
    "      break;\n"
    "    }\n"
    "  }\n"
    "  gamma=PerceptibleReciprocal(gamma);\n"
    "  const unsigned long index = y*columns+x;\n"
    "  output[index].x=ClampToQuantum(gamma*sum.x);\n"
    "  output[index].y=ClampToQuantum(gamma*sum.y);\n"
    "  output[index].z=ClampToQuantum(gamma*sum.z);\n"
    "  if (matte == false)\n"
    "    output[index].w=input[index].w;\n"
    "  else\n"
    "    output[index].w=ClampToQuantum(sum.w);\n"
    "}\n";

static void ConvolveNotify(const char *message,const void *data,size_t length,
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

static MagickBooleanType BindConvolveParameters(ConvolveInfo *convolve_info,
  const Image *image,const void *pixels,float *filter,const size_t width,
  const size_t height,void *convolve_pixels)
{
  cl_int
    status;

  register cl_uint
    i;

  size_t
    length;

  /*
    Allocate OpenCL buffers.
  */
  length=image->columns*image->rows;
  convolve_info->pixels=clCreateBuffer(convolve_info->context,(cl_mem_flags)
    (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR),length*sizeof(CLPixelPacket),
    (void *) pixels,&status);
  if ((convolve_info->pixels == (cl_mem) NULL) || (status != CL_SUCCESS))
    return(MagickFalse);
  length=width*height;
  convolve_info->filter=clCreateBuffer(convolve_info->context,(cl_mem_flags)
    (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR),length*sizeof(cl_float),filter,
    &status);
  if ((convolve_info->filter == (cl_mem) NULL) || (status != CL_SUCCESS))
    return(MagickFalse);
  length=image->columns*image->rows;
  convolve_info->convolve_pixels=clCreateBuffer(convolve_info->context,
    (cl_mem_flags) (CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR),length*
    sizeof(CLPixelPacket),convolve_pixels,&status);
  if ((convolve_info->convolve_pixels == (cl_mem) NULL) ||
      (status != CL_SUCCESS))
    return(MagickFalse);
  /*
    Bind OpenCL buffers.
  */
  i=0;
  status=clSetKernelArg(convolve_info->kernel,i++,sizeof(cl_mem),(void *)
    &convolve_info->pixels);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  status=clSetKernelArg(convolve_info->kernel,i++,sizeof(cl_mem),(void *)
    &convolve_info->filter);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  convolve_info->width=(cl_ulong) width;
  status=clSetKernelArg(convolve_info->kernel,i++,sizeof(cl_ulong),(void *)
    &convolve_info->width);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  convolve_info->height=(cl_ulong) height;
  status=clSetKernelArg(convolve_info->kernel,i++,sizeof(cl_ulong),(void *)
    &convolve_info->height);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  convolve_info->matte=(cl_uint) image->matte;
  status=clSetKernelArg(convolve_info->kernel,i++,sizeof(cl_uint),(void *)
    &convolve_info->matte);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  status=clSetKernelArg(convolve_info->kernel,i++,sizeof(cl_mem),(void *)
    &convolve_info->convolve_pixels);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  status=clFinish(convolve_info->command_queue);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  return(MagickTrue);
}

static void DestroyConvolveBuffers(ConvolveInfo *convolve_info)
{
  cl_int
    status;

  status=0;
  if (convolve_info->convolve_pixels != (cl_mem) NULL)
    status=clReleaseMemObject(convolve_info->convolve_pixels);
  if (convolve_info->pixels != (cl_mem) NULL)
    status=clReleaseMemObject(convolve_info->pixels);
  if (convolve_info->filter != (cl_mem) NULL)
    status=clReleaseMemObject(convolve_info->filter);
  (void) status;
}

static ConvolveInfo *DestroyConvolveInfo(ConvolveInfo *convolve_info)
{
  cl_int
    status;

  status=0;
  if (convolve_info->kernel != (cl_kernel) NULL)
    status=clReleaseKernel(convolve_info->kernel);
  if (convolve_info->program != (cl_program) NULL)
    status=clReleaseProgram(convolve_info->program);
  if (convolve_info->command_queue != (cl_command_queue) NULL)
    status=clReleaseCommandQueue(convolve_info->command_queue);
  if (convolve_info->context != (cl_context) NULL)
    status=clReleaseContext(convolve_info->context);
  (void) status;
  convolve_info=(ConvolveInfo *) RelinquishMagickMemory(convolve_info);
  return(convolve_info);
}

static MagickBooleanType EnqueueConvolveKernel(ConvolveInfo *convolve_info,
  const Image *image,const void *pixels,float *filter,const size_t width,
  const size_t height,void *convolve_pixels)
{
  cl_int
    status;

  size_t
    global_work_size[2],
    length;

  length=image->columns*image->rows;
  status=clEnqueueWriteBuffer(convolve_info->command_queue,
    convolve_info->pixels,CL_TRUE,0,length*sizeof(CLPixelPacket),pixels,0,NULL,
    NULL);
  length=width*height;
  status=clEnqueueWriteBuffer(convolve_info->command_queue,
    convolve_info->filter,CL_TRUE,0,length*sizeof(cl_float),filter,0,NULL,
    NULL);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  global_work_size[0]=image->columns;
  global_work_size[1]=image->rows;
  status=clEnqueueNDRangeKernel(convolve_info->command_queue,
    convolve_info->kernel,2,NULL,global_work_size,NULL,0,NULL,NULL);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  length=image->columns*image->rows;
  status=clEnqueueReadBuffer(convolve_info->command_queue,
    convolve_info->convolve_pixels,CL_TRUE,0,length*sizeof(CLPixelPacket),
    convolve_pixels,0,NULL,NULL);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  status=clFinish(convolve_info->command_queue);
  if (status != CL_SUCCESS)
    return(MagickFalse);
  return(MagickTrue);
}

static ConvolveInfo *GetConvolveInfo(const Image *image,const char *name,
  const char *source,ExceptionInfo *exception)
{
  char
    options[MaxTextExtent];

  cl_context_properties
    context_properties[3];

  cl_int
    status;

  cl_platform_id
    platforms[1];

  cl_uint
    number_platforms;

  ConvolveInfo
    *convolve_info;

  size_t
    length,
    lengths[] = { strlen(source) };

  /*
    Create OpenCL info.
  */
  convolve_info=(ConvolveInfo *) AcquireMagickMemory(sizeof(*convolve_info));
  if (convolve_info == (ConvolveInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return((ConvolveInfo *) NULL);
    }
  (void) ResetMagickMemory(convolve_info,0,sizeof(*convolve_info));
  /*
    Create OpenCL context.
  */
  status=clGetPlatformIDs(0,(cl_platform_id *) NULL,&number_platforms);
  if ((status == CL_SUCCESS) && (number_platforms > 0))
    status=clGetPlatformIDs(1,platforms,NULL);
  if (status != CL_SUCCESS)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),DelegateWarning,
        "failed to create OpenCL context","`%s' (%d)",image->filename,status);
      convolve_info=DestroyConvolveInfo(convolve_info);
      return((ConvolveInfo *) NULL);
    }
  context_properties[0]=CL_CONTEXT_PLATFORM;
  context_properties[1]=(cl_context_properties) platforms[0];
  context_properties[2]=0;
  convolve_info->context=clCreateContextFromType(context_properties,
    (cl_device_type) CL_DEVICE_TYPE_GPU,ConvolveNotify,exception,&status);
  if ((convolve_info->context == (cl_context) NULL) || (status != CL_SUCCESS))
    convolve_info->context=clCreateContextFromType(context_properties,
      (cl_device_type) CL_DEVICE_TYPE_CPU,ConvolveNotify,exception,&status);
  if ((convolve_info->context == (cl_context) NULL) || (status != CL_SUCCESS))
    convolve_info->context=clCreateContextFromType(context_properties,
      (cl_device_type) CL_DEVICE_TYPE_DEFAULT,ConvolveNotify,exception,&status);
  if ((convolve_info->context == (cl_context) NULL) || (status != CL_SUCCESS))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),DelegateWarning,
        "failed to create OpenCL context","`%s' (%d)",image->filename,status);
      convolve_info=DestroyConvolveInfo(convolve_info);
      return((ConvolveInfo *) NULL);
    }
  /*
    Detect OpenCL devices.
  */
  status=clGetContextInfo(convolve_info->context,CL_CONTEXT_DEVICES,0,NULL,
    &length);
  if ((status != CL_SUCCESS) || (length == 0))
    {
      convolve_info=DestroyConvolveInfo(convolve_info);
      return((ConvolveInfo *) NULL);
    }
  convolve_info->devices=(cl_device_id *) AcquireMagickMemory(length);
  if (convolve_info->devices == (cl_device_id *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      convolve_info=DestroyConvolveInfo(convolve_info);
      return((ConvolveInfo *) NULL);
    }
  status=clGetContextInfo(convolve_info->context,CL_CONTEXT_DEVICES,length,
    convolve_info->devices,NULL);
  if (status != CL_SUCCESS)
    {
      convolve_info=DestroyConvolveInfo(convolve_info);
      return((ConvolveInfo *) NULL);
    }
  if (image->debug != MagickFalse)
    {
      char
        attribute[MaxTextExtent];

      size_t
        length;

      clGetDeviceInfo(convolve_info->devices[0],CL_DEVICE_NAME,
        sizeof(attribute),attribute,&length);
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),"Name: %s",
        attribute);
      clGetDeviceInfo(convolve_info->devices[0],CL_DEVICE_VENDOR,
        sizeof(attribute),attribute,&length);
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),"Vendor: %s",
        attribute);
      clGetDeviceInfo(convolve_info->devices[0],CL_DEVICE_VERSION,
        sizeof(attribute),attribute,&length);
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),
        "Driver Version: %s",attribute);
      clGetDeviceInfo(convolve_info->devices[0],CL_DEVICE_PROFILE,
        sizeof(attribute),attribute,&length);
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),"Profile: %s",
        attribute);
      clGetDeviceInfo(convolve_info->devices[0],CL_DRIVER_VERSION,
        sizeof(attribute),attribute,&length);
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),"Driver: %s",
        attribute);
      clGetDeviceInfo(convolve_info->devices[0],CL_DEVICE_EXTENSIONS,
        sizeof(attribute),attribute,&length);
      (void) LogMagickEvent(AccelerateEvent,GetMagickModule(),"Extensions: %s",
        attribute);
    }
  /*
    Create OpenCL command queue.
  */
  convolve_info->command_queue=clCreateCommandQueue(convolve_info->context,
    convolve_info->devices[0],0,&status);
  if ((convolve_info->command_queue == (cl_command_queue) NULL) ||
      (status != CL_SUCCESS))
    {
      convolve_info=DestroyConvolveInfo(convolve_info);
      return((ConvolveInfo *) NULL);
    }
  /*
    Build OpenCL program.
  */
  convolve_info->program=clCreateProgramWithSource(convolve_info->context,1,
    &source,lengths,&status);
  if ((convolve_info->program == (cl_program) NULL) || (status != CL_SUCCESS))
    {
      convolve_info=DestroyConvolveInfo(convolve_info);
      return((ConvolveInfo *) NULL);
    }
  (void) FormatLocaleString(options,MaxTextExtent,CLOptions,(float)
    QuantumRange,MagickEpsilon);
  status=clBuildProgram(convolve_info->program,1,convolve_info->devices,options,
    NULL,NULL);
  if ((convolve_info->program == (cl_program) NULL) || (status != CL_SUCCESS))
    {
      char
        *log;

      status=clGetProgramBuildInfo(convolve_info->program,
        convolve_info->devices[0],CL_PROGRAM_BUILD_LOG,0,NULL,&length);
      log=(char *) AcquireMagickMemory(length);
      if (log == (char *) NULL)
        {
          convolve_info=DestroyConvolveInfo(convolve_info);
          return((ConvolveInfo *) NULL);
        }
      status=clGetProgramBuildInfo(convolve_info->program,
        convolve_info->devices[0],CL_PROGRAM_BUILD_LOG,length,log,&length);
      (void) ThrowMagickException(exception,GetMagickModule(),DelegateWarning,
        "failed to build OpenCL program","`%s' (%s)",image->filename,log);
      log=DestroyString(log);
      convolve_info=DestroyConvolveInfo(convolve_info);
      return((ConvolveInfo *) NULL);
    }
  /*
    Get a kernel object.
  */
  convolve_info->kernel=clCreateKernel(convolve_info->program,name,&status);
  if ((convolve_info->kernel == (cl_kernel) NULL) || (status != CL_SUCCESS))
    {
      convolve_info=DestroyConvolveInfo(convolve_info);
      return((ConvolveInfo *) NULL);
    }
  return(convolve_info);
}

#endif

MagickExport MagickBooleanType AccelerateConvolveImage(const Image *image,
  const KernelInfo *kernel,Image *convolve_image,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(kernel != (KernelInfo *) NULL);
  assert(kernel->signature == MagickSignature);
  assert(convolve_image != (Image *) NULL);
  assert(convolve_image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((image->storage_class != DirectClass) ||
      (image->colorspace == CMYKColorspace))
    return(MagickFalse);
  if ((GetImageVirtualPixelMethod(image) != UndefinedVirtualPixelMethod) &&
      (GetImageVirtualPixelMethod(image) != EdgeVirtualPixelMethod))
    return(MagickFalse);
#if !defined(MAGICKCORE_OPENCL_SUPPORT)
  return(MagickFalse);
#else
  {
    const void
      *pixels;

    float
      *filter;

    ConvolveInfo
      *convolve_info;

    MagickBooleanType
      status;

    MagickSizeType
      length;

    register ssize_t
      i;

    void
      *convolve_pixels;

    convolve_info=GetConvolveInfo(image,"Convolve",ConvolveKernel,exception);
    if (convolve_info == (ConvolveInfo *) NULL)
      return(MagickFalse);
    pixels=AcquirePixelCachePixels(image,&length,exception);
    if (pixels == (const void *) NULL)
      {
        convolve_info=DestroyConvolveInfo(convolve_info);
        (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
          "UnableToReadPixelCache","`%s'",image->filename);
        return(MagickFalse);
      }
    convolve_pixels=GetPixelCachePixels(convolve_image,&length,exception);
    if (convolve_pixels == (void *) NULL)
      {
        convolve_info=DestroyConvolveInfo(convolve_info);
        (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
          "UnableToReadPixelCache","`%s'",image->filename);
        return(MagickFalse);
      }
    filter=(float *) AcquireQuantumMemory(kernel->width,kernel->height*
      sizeof(*filter));
    if (filter == (float *) NULL)
      {
        DestroyConvolveBuffers(convolve_info);
        convolve_info=DestroyConvolveInfo(convolve_info);
        (void) ThrowMagickException(exception,GetMagickModule(),
          ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
        return(MagickFalse);
      }
    for (i=0; i < (ssize_t) (kernel->width*kernel->height); i++)
      filter[i]=(float) kernel->values[i];
    status=BindConvolveParameters(convolve_info,image,pixels,filter,
      kernel->width,kernel->height,convolve_pixels);
    if (status == MagickFalse)
      {
        filter=(float *) RelinquishMagickMemory(filter);
        DestroyConvolveBuffers(convolve_info);
        convolve_info=DestroyConvolveInfo(convolve_info);
        return(MagickFalse);
      }
    status=EnqueueConvolveKernel(convolve_info,image,pixels,filter,
      kernel->width,kernel->height,convolve_pixels);
    filter=(float *) RelinquishMagickMemory(filter);
    if (status == MagickFalse)
      {
        DestroyConvolveBuffers(convolve_info);
        convolve_info=DestroyConvolveInfo(convolve_info);
        return(MagickFalse);
      }
    DestroyConvolveBuffers(convolve_info);
    convolve_info=DestroyConvolveInfo(convolve_info);
    return(MagickTrue);
  }
#endif
}

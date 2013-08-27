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
*/

/*
Include declarations.
*/
#include "magick/studio.h"
#include "magick/accelerate.h"
#include "magick/accelerate-private.h"
#include "magick/artifact.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/cache-view.h"
#include "magick/color-private.h"
#include "magick/delegate-private.h"
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
#include "magick/opencl.h"
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

#ifdef MAGICKCORE_CLPERFMARKER
#include "CLPerfMarker.h"
#endif

#if defined(MAGICKCORE_OPENCL_SUPPORT)
#include <CL/cl.h>

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


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    A c c e l e r a t e C o n v o l v e I m a g e _ K e r n e l W r a p p e r%
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AccelerateConvolveImage_KernelWrapper() accelerates the convolve command.
%
%  The format of the AccelerateConvolveImage_KernelWrapper method is:
%
%      MagickBooleanType AccelerateConvolveImage_KernelWrapper(
%        void ** usrdata, KernelEnv *env)
%
%  A description of each parameter follows:
%
%    o usrdata: image data and other information
%
%    o env: kernel environment.
%
*/

#define ALIGNED(pointer,type) ((((long)(pointer)) & (sizeof(type)-1)) == 0)

static MagickBooleanType BindParameters_Convolve(
  KernelEnv *env, const void *pixels, void *filtered_pixels, 
  const void *filter, const unsigned int matte, const ChannelType channel, const size_t* localGroupSize,
  const MagickBooleanType use_ocl_shared)
{
  cl_int status;
  register cl_uint i;
  size_t length;
  cl_mem_flags mem_flags;
  void* host_ptr;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  /* Create and initialize OpenCL buffers. */
  if (use_ocl_shared == MagickFalse)
  {
    length = env->columns * env->rows;

    // If the host pointer is aligned to the size of CLPixelPacket, 
    // then use the host buffer directly from the GPU; otherwise, 
    // create a buffer on the GPU and copy the data over
    if (ALIGNED(pixels,CLPixelPacket)) 
    {
      mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
      host_ptr = (void*)pixels;
    }
    else 
    {
      mem_flags = CL_MEM_READ_ONLY;
      host_ptr = NULL;
    }
    // create a CL buffer from image pixel buffer
    env->pixels=clCreateBuffer(env->context, mem_flags, length * sizeof(CLPixelPacket), host_ptr, &status);
    if (status != CL_SUCCESS)
      return(MagickFalse);
  }

#ifdef O_CL_SHARE_BUFFER
  else
  {
    env->pixels = (cl_mem) ocl_JPEGdcomp_GetUncomMemRGB();
  }
#endif

  // create a CL buffer for the image filter
  length = env->width * env->height;
  env->filter=clCreateBuffer(env->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,length * sizeof(float), (void *) filter, &status);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  // output image
  length = env->columns * env->rows;
  if (ALIGNED(pixels,CLPixelPacket))
  {
      mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
      host_ptr = (void*) filtered_pixels;
  }
  else 
  {
      mem_flags = CL_MEM_WRITE_ONLY;
      host_ptr = NULL;
  }
  env->filtered_pixels=clCreateBuffer(env->context, mem_flags
                                    , length * sizeof(CLPixelPacket), host_ptr, &status);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  /*
  Bind OpenCL buffers.
  */
  i=0;
  status=clSetKernelArg(env->kernel,i++,sizeof(cl_mem),(void *)&env->pixels);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  status=clSetKernelArg(env->kernel,i++,sizeof(cl_mem),(void *)&env->filtered_pixels);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  status=clSetKernelArg(env->kernel,i++,sizeof(unsigned int),(void *)&env->columns);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  status=clSetKernelArg(env->kernel,i++,sizeof(unsigned int),(void *)&env->rows);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  status=clSetKernelArg(env->kernel,i++,sizeof(cl_mem),(void *)&env->filter);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  status=clSetKernelArg(env->kernel,i++,sizeof(unsigned int),(void *)&env->width);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  status=clSetKernelArg(env->kernel,i++,sizeof(unsigned int),(void *)&env->height);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  status=clSetKernelArg(env->kernel,i++,sizeof(unsigned int),(void *)&matte);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  status=clSetKernelArg(env->kernel,i++,sizeof(unsigned int),(void *)&channel);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  status=clSetKernelArg(env->kernel,i++, (localGroupSize[0] + env->width-1)*(localGroupSize[1] + env->height-1)*sizeof(CLPixelPacket),NULL);
  if (status != CL_SUCCESS)
    return(MagickFalse);

  status=clSetKernelArg(env->kernel,i++, env->width*env->height*sizeof(float),NULL);
  if (status != CL_SUCCESS)
    return(MagickFalse);

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif

  return(MagickTrue);
}


static MagickBooleanType EnqueueKernel_Convolve(
  KernelEnv *env, const void *pixels, void *filtered_pixels, 
  const void *filter, const size_t* localGroupSize, const MagickBooleanType use_ocl_shared)
{
  cl_int status;
  size_t global_work_size[2];
  size_t local_work_size[2];
  size_t length;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  local_work_size[0] = localGroupSize[0];
  local_work_size[1] = localGroupSize[1];

  // pad the global size to a multiple of the local work size dimension
  global_work_size[0] = ((env->columns + local_work_size[0] - 1)/local_work_size[0]) * local_work_size[0];
  global_work_size[1] = ((env->rows + local_work_size[1] - 1)/local_work_size[1]) * local_work_size[1];

  /* Set input data */
  length = env->columns * env->rows;
  if (use_ocl_shared == MagickFalse)
  {
    if (!ALIGNED(pixels,CLPixelPacket)) 
    {
      status = clEnqueueWriteBuffer(env->command_queue, env->pixels, CL_TRUE
                                    , 0,length * sizeof(CLPixelPacket), pixels, 0, NULL, NULL);
      if (status != CL_SUCCESS)
        return(MagickFalse);

      clFlush(env->command_queue);
    }
  }

  status = clEnqueueNDRangeKernel(env->command_queue, env->kernel, 2, NULL, global_work_size, local_work_size
                                  , 0, NULL, NULL);

  if (status != CL_SUCCESS)
    return(MagickFalse);

  length = env->columns * env->rows;
  if (ALIGNED(filtered_pixels,CLPixelPacket))
  {
    clEnqueueMapBuffer(env->command_queue, env->filtered_pixels, CL_TRUE, CL_MAP_READ
                      , 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &status);
  }
  else
  {
    status = clEnqueueReadBuffer(env->command_queue, env->filtered_pixels, CL_TRUE
                                , 0, length * sizeof(CLPixelPacket), filtered_pixels, 0, NULL, NULL);
  }


  if (status != CL_SUCCESS)
    return(MagickFalse);

  if (use_ocl_shared == MagickTrue)
  {
    clEnqueueCopyBuffer(env->command_queue, env->filtered_pixels, env->pixels, 0, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL);
  }
  status = clFinish(env->command_queue);
  if (status != CL_SUCCESS)
    return(MagickFalse); 

  if (use_ocl_shared == MagickTrue)
  {
    env->pixels = (cl_mem) NULL;
  }

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif
  return(MagickTrue);
}


MagickBooleanType AccelerateConvolveImage_KernelWrapper(
  void ** usrdata, KernelEnv *env)
{
  const void *pixels;
  MagickBooleanType status;
  MagickSizeType length;
  float *filter;
  size_t i;
  cl_int clStatus;
  void *filtered_pixels;

  const Image *image= (const Image *) usrdata[0];
  const KernelInfo *kernel = (const KernelInfo *) usrdata[1];
  Image *convolve_image = (Image *) usrdata[2];
  ExceptionInfo *exception= (ExceptionInfo *) usrdata[3];
  const ChannelType channel = *((ChannelType*) usrdata[4]); 
  
    MagickBooleanType use_ocl_shared = MagickFalse;
  size_t localGroupSize[2];
  size_t localMemoryRequirement;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif
  
  if (image->debug != MagickFalse)
    (void)LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  assert(kernel != (KernelInfo *) NULL);
  assert(kernel->signature == MagickSignature);
  assert(convolve_image != (Image *) NULL);
  assert(convolve_image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  if (env == (KernelEnv *) NULL)
    return(MagickFalse);

  localGroupSize[0] = 16;
  localGroupSize[1] = 16;
  localMemoryRequirement = (localGroupSize[0]+kernel->width-1) * (localGroupSize[1]+kernel->height-1) * sizeof(CLPixelPacket)
    + kernel->width*kernel->height*sizeof(float);
  if (localMemoryRequirement > 16384)
  {
    localGroupSize[0] = 8;
    localGroupSize[1] = 8;
  }

#ifdef O_CL_SHARE_BUFFER
  if (ocl_JPEGdcomp_GetUncomMemRGB() != (void *) NULL)
    use_ocl_shared = MagickTrue;
#endif
  pixels = NULL;
  if (use_ocl_shared == MagickFalse)
  {
    pixels = AcquirePixelCachePixels(image, &length, exception);
    if (pixels == (const void *) NULL)
    {
      (void) ThrowMagickException(
        exception,GetMagickModule(),CacheError,
        "UnableToReadPixelCache.","`%s'",image->filename);
      return(MagickFalse);
    }
  }

  /* double kernel -> float kernel */
  filter = (float *) AcquireMagickMemory(kernel->width * kernel->height * sizeof(float));
  if (filter == (float *) NULL)
  {
    (void) ThrowMagickException(
      exception, GetMagickModule(), ResourceLimitError,
      "MemoryAllocationFailed.", "'%s'", image->filename);
    return(MagickFalse);
  }
  for (i=0; i < kernel->width*kernel->height; i++)
    filter[i] = (float) kernel->values[i];

  filtered_pixels = GetPixelCachePixels(convolve_image, &length, exception);
  if (filtered_pixels == (void *) NULL)
  {
    (void) ThrowMagickException(
      exception,GetMagickModule(),CacheError,
      "UnableToReadPixelCache.","`%s'",image->filename);
    filter = (float *) RelinquishMagickMemory(filter);
    return(MagickFalse);
  }
  env->columns = (unsigned int) image->columns;
  env->rows = (unsigned int) image->rows;
  env->width = (unsigned int) kernel->width;
  env->height = (unsigned int) kernel->height;
  env->kernel = clCreateKernel(env->program, env->kernel_name, &clStatus);

  status = BindParameters_Convolve(env, pixels, filtered_pixels, (void *)filter, (unsigned int) image->matte, channel, localGroupSize, use_ocl_shared);
  if (status == MagickFalse)
    (void) ThrowMagickException(
    exception, GetMagickModule(), DelegateWarning,
    "BindConvolveOptimizedParameter failed.", "'%s'", image->filename);
  else
  {
    status=EnqueueKernel_Convolve(env, pixels, filtered_pixels, (void*)filter, localGroupSize, use_ocl_shared);
    if (status == MagickFalse)
      (void) ThrowMagickException(
      exception, GetMagickModule(), DelegateWarning,
      "EnqueueConvolveOptimizedKernel failed.", "'%s'", image->filename);
  }
  ReleaseCLBuffers(env);
  filter = (float *) RelinquishMagickMemory(filter);

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif


  return(status);
}


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
%  AccelerateConvolveImage() accelerates the convolve command.
%
%  The format of the AccelerateConvolveImage method is:
%
%      MagickBooleanType AccelerateConvolveImage(const Image *image,
%        const KernelInfo *kernel,Image *filtered_image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: input image.
%
%    o kernel: kernel information.
%
%    o filtered_image: convolve image.
%
%    o exception: return any errors or warnings in this structure.
%
*/


static MagickBooleanType checkAccelerateConvolveCondition(const Image* image, const ChannelType channel, ExceptionInfo *exception) 
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  /* check if the image's colorspace is supported */
  if (image->colorspace != RGBColorspace
    && image->colorspace != sRGBColorspace)
    return MagickFalse;
  
  /* check if the channel is supported */
  if (((channel&RedChannel) == 0)
	|| ((channel&GreenChannel) == 0)
	|| ((channel&BlueChannel) == 0))
  {
    return MagickFalse;
  }
  

  /* check if if the virtual pixel method is compatible with the OpenCL implementation */
  if ((GetImageVirtualPixelMethod(image) != UndefinedVirtualPixelMethod)&&
      (GetImageVirtualPixelMethod(image) != EdgeVirtualPixelMethod))
    return MagickFalse;

  return MagickTrue;
}

MagickExport MagickBooleanType AccelerateConvolveImage(const Image *image,
  const KernelInfo *kernel,Image *convolve_image,ExceptionInfo *exception)
{
  /* legacy, do not use */
  return(MagickFalse);
}

MagickExport Image *AccelerateConvolveImageChannel(const Image *image,
  const ChannelType channel,const KernelInfo *kernel,ExceptionInfo *exception)
{
  MagickBooleanType status;
  Image* filtered_image = NULL;

#ifdef MAGICKCORE_CLPERFMARKER
  clBeginPerfMarkerAMD(__FUNCTION__,"");
#endif

  status = checkAccelerateConvolveCondition(image, channel, exception);
  if (status == MagickTrue) 
  {
    filtered_image = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
    assert(filtered_image != NULL);

    status = SetImageStorageClass(filtered_image,DirectClass);
    if (status == MagickTrue)
    {
      void * usrdata[MAX_USRDATA_ARGS];
      usrdata[0] = (void *) image;
      usrdata[1] = (void *) kernel;
      usrdata[2] = (void *) filtered_image;
      usrdata[3] = (void *) exception;
      usrdata[4] = (void *) &channel;

      status = AccelerateFunctionCL( &AccelerateConvolveImage_KernelWrapper, "accelerate", accelerate_kernels,
                                     "ConvolveOptimized" , usrdata, exception);
    }
    if (status == MagickFalse)
    {
      DestroyImage(filtered_image);
      filtered_image = (Image*)NULL;      
    }
  }

#ifdef MAGICKCORE_CLPERFMARKER
  clEndPerfMarkerAMD();
#endif
  return filtered_image;
}



#else  // MAGICKCORE_OPENCL_SUPPORT 

MagickExport MagickBooleanType AccelerateConvolveImage(const Image *image,
  const KernelInfo *kernel,Image *convolve_image,ExceptionInfo *exception)
{
  /* legacy, do not use */
  return(MagickFalse);
}

MagickExport Image *AccelerateConvolveImageChannel(const Image *image,
  const ChannelType channel,const KernelInfo *kernel,ExceptionInfo *exception)
{
  return NULL;
}

#endif // MAGICKCORE_OPENCL_SUPPORT

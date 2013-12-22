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
%                                  Cristy                                     %
%                               SiuChi Chan                                   %
%                               Guansong Zhang                                %
%                               January 2010                                  %
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
#include "magick/opencl-private.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/prepress.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/random-private.h"
#include "magick/registry.h"
#include "magick/resize.h"
#include "magick/resize-private.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/token.h"

#ifdef MAGICKCORE_CLPERFMARKER
#include "CLPerfMarker.h"
#endif

#define MAGICK_MAX(x,y) (((x) >= (y))?(x):(y))
#define MAGICK_MIN(x,y) (((x) <= (y))?(x):(y))

#if defined(MAGICKCORE_OPENCL_SUPPORT)

#define ALIGNED(pointer,type) ((((long)(pointer)) & (sizeof(type)-1)) == 0)
/*#define ALIGNED(pointer,type) (0) */

static MagickBooleanType checkOpenCLEnvironment(ExceptionInfo* exception)
{
  MagickBooleanType flag;

  MagickCLEnv clEnv;
  clEnv = GetDefaultOpenCLEnv();

  GetMagickOpenCLEnvParam(clEnv, MAGICK_OPENCL_ENV_PARAM_OPENCL_DISABLED
    , sizeof(MagickBooleanType), &flag, exception);
  if (flag != MagickFalse)
    return MagickFalse;

  GetMagickOpenCLEnvParam(clEnv, MAGICK_OPENCL_ENV_PARAM_OPENCL_INITIALIZED
    , sizeof(MagickBooleanType), &flag, exception);
  if (flag == MagickFalse)
  {
    if(InitOpenCLEnv(clEnv, exception) == MagickFalse)
      return MagickFalse;

    GetMagickOpenCLEnvParam(clEnv, MAGICK_OPENCL_ENV_PARAM_OPENCL_DISABLED
      , sizeof(MagickBooleanType), &flag, exception);
    if (flag != MagickFalse)
      return MagickFalse;
  }

  return MagickTrue;
}


static MagickBooleanType checkAccelerateCondition(const Image* image, const ChannelType channel)
{
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


static Image* ComputeConvolveImage(const Image* inputImage, const ChannelType channel, const KernelInfo *kernel, ExceptionInfo *exception)
{
  MagickBooleanType outputReady;
  MagickCLEnv clEnv;

  cl_int clStatus;
  size_t global_work_size[2];
  size_t localGroupSize[2];
  size_t localMemoryRequirement;
  Image* filteredImage;
  MagickSizeType length;
  const void *inputPixels;
  void *filteredPixels;
  cl_mem_flags mem_flags;
  float* kernelBufferPtr;
  unsigned kernelSize;
  unsigned int i;
  void *hostPtr;
  unsigned int matte, filterWidth, filterHeight, imageWidth, imageHeight;

  cl_context context;
  cl_kernel clkernel;
  cl_mem inputImageBuffer, filteredImageBuffer, convolutionKernel;
  cl_ulong deviceLocalMemorySize;
  cl_device_id device;

  cl_command_queue queue;

  /* intialize all CL objects to NULL */
  context = NULL;
  inputImageBuffer = NULL;
  filteredImageBuffer = NULL;
  convolutionKernel = NULL;
  clkernel = NULL;
  queue = NULL;
  device = NULL;

  filteredImage = NULL;
  outputReady = MagickFalse;
  
  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);

  inputPixels = NULL;
  inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
  if (inputPixels == (const void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
    goto cleanup;
  }

  /* Create and initialize OpenCL buffers. */

  /* If the host pointer is aligned to the size of CLPixelPacket, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  filteredImage = CloneImage(inputImage,inputImage->columns,inputImage->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredPixels = GetPixelCachePixels(filteredImage, &length, exception);
  if (filteredPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
    goto cleanup;
  }

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
    hostPtr = filteredPixels;
  }
  else 
  {
    mem_flags = CL_MEM_WRITE_ONLY;
    hostPtr = NULL;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  filteredImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  kernelSize = kernel->width * kernel->height;
  convolutionKernel = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, kernelSize * sizeof(float), NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  queue = AcquireOpenCLCommandQueue(clEnv);

  kernelBufferPtr = (float*)clEnqueueMapBuffer(queue, convolutionKernel, CL_TRUE, CL_MAP_WRITE, 0, kernelSize * sizeof(float)
          , 0, NULL, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueMapBuffer failed.",".");
    goto cleanup;
  }
  for (i = 0; i < kernelSize; i++)
  {
    kernelBufferPtr[i] = (float) kernel->values[i];
  }
  clStatus = clEnqueueUnmapMemObject(queue, convolutionKernel, kernelBufferPtr, 0, NULL, NULL);
 if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.", "'%s'", ".");
    goto cleanup;
  }
  clFlush(queue);

  /* Compute the local memory requirement for a 16x16 workgroup.
     If it's larger than 16k, reduce the workgroup size to 8x8 */
  localGroupSize[0] = 16;
  localGroupSize[1] = 16;
  localMemoryRequirement = (localGroupSize[0]+kernel->width-1) * (localGroupSize[1]+kernel->height-1) * sizeof(CLPixelPacket)
    + kernel->width*kernel->height*sizeof(float);
  if (localMemoryRequirement > 16384)
  {


    localGroupSize[0] = 8;
    localGroupSize[1] = 8;

    localMemoryRequirement = (localGroupSize[0]+kernel->width-1) * (localGroupSize[1]+kernel->height-1) * sizeof(CLPixelPacket)
      + kernel->width*kernel->height*sizeof(float);
  }

  GetMagickOpenCLEnvParam(clEnv, MAGICK_OPENCL_ENV_PARAM_DEVICE, sizeof(cl_device_id), &device, exception);
  clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &deviceLocalMemorySize, NULL);
  if (localMemoryRequirement <= deviceLocalMemorySize) 
  {
    /* get the OpenCL kernel */
    clkernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "ConvolveOptimized");
    if (clkernel == NULL)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
      goto cleanup;
    }

    /* set the kernel arguments */
    i = 0;
    clStatus =clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
    imageWidth = inputImage->columns;
    imageHeight = inputImage->rows;
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&imageWidth);
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&imageHeight);
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&convolutionKernel);
    filterWidth = kernel->width;
    filterHeight = kernel->height;
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&filterWidth);
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&filterHeight);
    matte = (inputImage->matte==MagickTrue)?1:0;
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&matte);
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(ChannelType),(void *)&channel);
    clStatus|=clSetKernelArg(clkernel,i++, (localGroupSize[0] + kernel->width-1)*(localGroupSize[1] + kernel->height-1)*sizeof(CLPixelPacket),NULL);
    clStatus|=clSetKernelArg(clkernel,i++, kernel->width*kernel->height*sizeof(float),NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }

    /* pad the global size to a multiple of the local work size dimension */
    global_work_size[0] = ((inputImage->columns + localGroupSize[0]  - 1)/localGroupSize[0] ) * localGroupSize[0] ;
    global_work_size[1] = ((inputImage->rows + localGroupSize[1] - 1)/localGroupSize[1]) * localGroupSize[1];

    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, clkernel, 2, NULL, global_work_size, localGroupSize, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
  }
  else
  {
    /* get the OpenCL kernel */
    clkernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "Convolve");
    if (clkernel == NULL)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
      goto cleanup;
    }

    /* set the kernel arguments */
    i = 0;
    clStatus =clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&convolutionKernel);
    filterWidth = kernel->width;
    filterHeight = kernel->height;
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&filterWidth);
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&filterHeight);
    matte = (inputImage->matte==MagickTrue)?1:0;
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&matte);
    clStatus|=clSetKernelArg(clkernel,i++,sizeof(ChannelType),(void *)&channel);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }

    global_work_size[0] = inputImage->columns;
    global_work_size[1] = inputImage->rows;

    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, clkernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
  }
  clFlush(queue);

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  /* everything is fine! :) */
  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (inputImageBuffer != NULL)
    clReleaseMemObject(inputImageBuffer);

  if (filteredImageBuffer != NULL)
    clReleaseMemObject(filteredImageBuffer);

  if (convolutionKernel != NULL)
    clReleaseMemObject(convolutionKernel);

  if (clkernel != NULL)
    RelinquishOpenCLKernel(clEnv, clkernel);

  if (queue != NULL)
    RelinquishOpenCLCommandQueue(clEnv, queue);

  if (outputReady == MagickFalse)
  {
    if (filteredImage != NULL)
    {
      DestroyImage(filteredImage);
      filteredImage = NULL;
    }
  }

  return filteredImage;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o n v o l v e I m a g e  w i t h  O p e n C L                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvolveImage() applies a custom convolution kernel to the image.
%
%  The format of the ConvolveImage method is:
%
%      Image *ConvolveImage(const Image *image,const size_t order,
%        const double *kernel,ExceptionInfo *exception)
%      Image *ConvolveImageChannel(const Image *image,const ChannelType channel,
%        const size_t order,const double *kernel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o kernel: kernel info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport Image* AccelerateConvolveImageChannel(const Image *image, const ChannelType channel, const KernelInfo *kernel, ExceptionInfo *exception)
{
  MagickBooleanType status;
  Image* filteredImage = NULL;

  assert(image != NULL);
  assert(kernel != (KernelInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);

  status = checkOpenCLEnvironment(exception);
  if (status == MagickFalse)
    return NULL;

  status = checkAccelerateCondition(image, channel);
  if (status == MagickFalse)
    return NULL;

  filteredImage = ComputeConvolveImage(image, channel, kernel, exception);
  return filteredImage;
}

static MagickBooleanType ComputeFunctionImage(Image *image, const ChannelType channel,const MagickFunction function,
  const size_t number_parameters,const double *parameters, ExceptionInfo *exception)
{
  MagickBooleanType status;

  MagickCLEnv clEnv;

  MagickSizeType length;
  void* pixels;
  float* parametersBufferPtr;

  cl_int clStatus;
  cl_context context;
  cl_kernel clkernel;
  cl_command_queue queue;
  cl_mem_flags mem_flags;
  cl_mem imageBuffer;
  cl_mem parametersBuffer;
  size_t globalWorkSize[2];

  unsigned int i;

  status = MagickFalse;

  context = NULL;
  clkernel = NULL;
  queue = NULL;
  imageBuffer = NULL;
  parametersBuffer = NULL;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);

  pixels = GetPixelCachePixels(image, &length, exception);
  if (pixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), CacheWarning,
      "GetPixelCachePixels failed.",
      "'%s'", image->filename);
    goto cleanup;
  }


  if (ALIGNED(pixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = image->columns * image->rows;
  imageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)pixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  parametersBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, number_parameters * sizeof(float), NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  queue = AcquireOpenCLCommandQueue(clEnv);

  parametersBufferPtr = (float*)clEnqueueMapBuffer(queue, parametersBuffer, CL_TRUE, CL_MAP_WRITE, 0, number_parameters * sizeof(float)
                , 0, NULL, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueMapBuffer failed.",".");
    goto cleanup;
  }
  for (i = 0; i < number_parameters; i++)
  {
    parametersBufferPtr[i] = (float)parameters[i];
  }
  clStatus = clEnqueueUnmapMemObject(queue, parametersBuffer, parametersBufferPtr, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.", "'%s'", ".");
    goto cleanup;
  }
  clFlush(queue);

  clkernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "FunctionImage");
  if (clkernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  /* set the kernel arguments */
  i = 0;
  clStatus =clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clSetKernelArg(clkernel,i++,sizeof(ChannelType),(void *)&channel);
  clStatus|=clSetKernelArg(clkernel,i++,sizeof(MagickFunction),(void *)&function);
  clStatus|=clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&number_parameters);
  clStatus|=clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&parametersBuffer);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  globalWorkSize[0] = image->columns;
  globalWorkSize[1] = image->rows;
  /* launch the kernel */
  clStatus = clEnqueueNDRangeKernel(queue, clkernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  clFlush(queue);


  if (ALIGNED(pixels,CLPixelPacket)) 
  {
    length = image->columns * image->rows;
    clEnqueueMapBuffer(queue, imageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = image->columns * image->rows;
    clStatus = clEnqueueReadBuffer(queue, imageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), pixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }
  status = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);
  
  if (clkernel != NULL) RelinquishOpenCLKernel(clEnv, clkernel);
  if (queue != NULL) RelinquishOpenCLCommandQueue(clEnv, queue);
  if (imageBuffer != NULL) clReleaseMemObject(imageBuffer);
  if (parametersBuffer != NULL) clReleaseMemObject(parametersBuffer);

  return status;
}



MagickExport MagickBooleanType 
  AccelerateFunctionImage(Image *image, const ChannelType channel,const MagickFunction function,
  const size_t number_parameters,const double *parameters, ExceptionInfo *exception)
{
  MagickBooleanType status;

  status = MagickFalse;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  status = checkOpenCLEnvironment(exception);
  if (status != MagickFalse)
  {
    status = checkAccelerateCondition(image, channel);
    if (status != MagickFalse)
    {
      status = ComputeFunctionImage(image, channel, function, number_parameters, parameters, exception);
    }
  }
  return status;
}


static MagickBooleanType splitImage(const Image* inputImage)
{
  MagickBooleanType split;

  MagickCLEnv clEnv;
  unsigned long allocSize;
  unsigned long tempSize;

  clEnv = GetDefaultOpenCLEnv();
 
  allocSize = GetOpenCLDeviceMaxMemAllocSize(clEnv);
  tempSize = inputImage->columns * inputImage->rows * 4 * 4;

  /*
  printf("alloc size: %lu\n", allocSize);
  printf("temp size: %lu\n", tempSize);
  */

  split = ((tempSize > allocSize) ? MagickTrue:MagickFalse);

  return split;
}

static Image* ComputeBlurImage(const Image* inputImage, const ChannelType channel, const double radius, const double sigma, ExceptionInfo *exception)
{
  MagickBooleanType outputReady;
  Image* filteredImage;
  MagickCLEnv clEnv;

  cl_int clStatus;

  const void *inputPixels;
  void *filteredPixels;
  cl_mem_flags mem_flags;

  cl_context context;
  cl_mem inputImageBuffer, tempImageBuffer, filteredImageBuffer, imageKernelBuffer;
  cl_kernel blurRowKernel, blurColumnKernel;
  cl_command_queue queue;

  void* hostPtr;
  float* kernelBufferPtr;
  MagickSizeType length;

  char geometry[MaxTextExtent];
  KernelInfo* kernel = NULL;
  unsigned int kernelWidth;
  unsigned int imageColumns, imageRows;

  unsigned int i;

  context = NULL;
  filteredImage = NULL;
  inputImageBuffer = NULL;
  tempImageBuffer = NULL;
  filteredImageBuffer = NULL;
  imageKernelBuffer = NULL;
  blurRowKernel = NULL;
  blurColumnKernel = NULL;
  queue = NULL;

  outputReady = MagickFalse;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  /* Create and initialize OpenCL buffers. */
  {
    inputPixels = NULL;
    inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
    if (inputPixels == (const void *) NULL)
    {
      (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
      goto cleanup;
    }
    /* If the host pointer is aligned to the size of CLPixelPacket, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
    if (ALIGNED(inputPixels,CLPixelPacket)) 
    {
      mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
    }
    else 
    {
      mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
    }
    /* create a CL buffer from image pixel buffer */
    length = inputImage->columns * inputImage->rows;
    inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  /* create output */
  {
    filteredImage = CloneImage(inputImage,inputImage->columns,inputImage->rows,MagickTrue,exception);
    assert(filteredImage != NULL);
    if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
      goto cleanup;
    }
    filteredPixels = GetPixelCachePixels(filteredImage, &length, exception);
    if (filteredPixels == (void *) NULL)
    {
      (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
      goto cleanup;
    }

    if (ALIGNED(filteredPixels,CLPixelPacket)) 
    {
      mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
      hostPtr = filteredPixels;
    }
    else 
    {
      mem_flags = CL_MEM_WRITE_ONLY;
      hostPtr = NULL;
    }
    /* create a CL buffer from image pixel buffer */
    length = inputImage->columns * inputImage->rows;
    filteredImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  /* create processing kernel */
  {
    (void) FormatLocaleString(geometry,MaxTextExtent,"blur:%.20gx%.20g;blur:%.20gx%.20g+90",radius,sigma,radius,sigma);
    kernel=AcquireKernelInfo(geometry);
    if (kernel == (KernelInfo *) NULL)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "MemoryAllocationFailed.",".");
      goto cleanup;
    }

    imageKernelBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, kernel->width * sizeof(float), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
    kernelBufferPtr = (float*)clEnqueueMapBuffer(queue, imageKernelBuffer, CL_TRUE, CL_MAP_WRITE, 0, kernel->width * sizeof(float), 0, NULL, NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueMapBuffer failed.",".");
      goto cleanup;
    }

    for (i = 0; i < kernel->width; i++)
    {
      kernelBufferPtr[i] = (float) kernel->values[i];
    }

    clStatus = clEnqueueUnmapMemObject(queue, imageKernelBuffer, kernelBufferPtr, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.", "'%s'", ".");
      goto cleanup;
    }
  }

  {

    /* create temp buffer */
    {
      length = inputImage->columns * inputImage->rows;
      tempImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, length * 4 * sizeof(float), NULL, &clStatus);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
        goto cleanup;
      }
    }

    /* get the OpenCL kernels */
    {
      blurRowKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "BlurRow");
      if (blurRowKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };

      blurColumnKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "BlurColumn");
      if (blurColumnKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };
    }

    {
      /* need logic to decide this value */
      int chunkSize = 256;

      {
        imageColumns = inputImage->columns;
        imageRows = inputImage->rows;

        /* set the kernel arguments */
        i = 0;
        clStatus=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(ChannelType),&channel);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
        kernelWidth = kernel->width;
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageRows);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(CLPixelPacket)*(chunkSize+kernel->width),(void *)NULL);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
          goto cleanup;
        }
      }

      /* launch the kernel */
      {
        size_t gsize[2];
        size_t wsize[2];

        gsize[0] = chunkSize*((inputImage->columns+chunkSize-1)/chunkSize);
        gsize[1] = inputImage->rows;
        wsize[0] = chunkSize;
        wsize[1] = 1;

        clStatus = clEnqueueNDRangeKernel(queue, blurRowKernel, 2, NULL, gsize, wsize, 0, NULL, NULL);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
          goto cleanup;
        }
        clFlush(queue);
      }
    }

    {
      /* need logic to decide this value */
      int chunkSize = 256;

      {
        imageColumns = inputImage->columns;
        imageRows = inputImage->rows;

        /* set the kernel arguments */
        i = 0;
        clStatus=clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
        clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
        clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(ChannelType),&channel);
        clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
        kernelWidth = kernel->width;
        clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
        clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
        clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&imageRows);
        clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(cl_float4)*(chunkSize+kernel->width),(void *)NULL);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
          goto cleanup;
        }
      }

      /* launch the kernel */
      {
        size_t gsize[2];
        size_t wsize[2];

        gsize[0] = inputImage->columns;
        gsize[1] = chunkSize*((inputImage->rows+chunkSize-1)/chunkSize);
        wsize[0] = 1;
        wsize[1] = chunkSize;

        clStatus = clEnqueueNDRangeKernel(queue, blurColumnKernel, 2, NULL, gsize, wsize, 0, NULL, NULL);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
          goto cleanup;
        }
        clFlush(queue);
      }
    }

  }

  /* get result */ 
  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (inputImageBuffer!=NULL)     clReleaseMemObject(inputImageBuffer);
  if (tempImageBuffer!=NULL)      clReleaseMemObject(tempImageBuffer);
  if (filteredImageBuffer!=NULL)  clReleaseMemObject(filteredImageBuffer);
  if (imageKernelBuffer!=NULL)    clReleaseMemObject(imageKernelBuffer);
  if (blurRowKernel!=NULL)        RelinquishOpenCLKernel(clEnv, blurRowKernel);
  if (blurColumnKernel!=NULL)     RelinquishOpenCLKernel(clEnv, blurColumnKernel);
  if (queue != NULL)              RelinquishOpenCLCommandQueue(clEnv, queue);
  if (kernel!=NULL)               DestroyKernelInfo(kernel);
  if (outputReady == MagickFalse)
  {
    if (filteredImage != NULL)
    {
      DestroyImage(filteredImage);
      filteredImage = NULL;
    }
  }
  return filteredImage;
}

static Image* ComputeBlurImageSection(const Image* inputImage, const ChannelType channel, const double radius, const double sigma, ExceptionInfo *exception)
{
  MagickBooleanType outputReady;
  Image* filteredImage;
  MagickCLEnv clEnv;

  cl_int clStatus;

  const void *inputPixels;
  void *filteredPixels;
  cl_mem_flags mem_flags;

  cl_context context;
  cl_mem inputImageBuffer, tempImageBuffer, filteredImageBuffer, imageKernelBuffer;
  cl_kernel blurRowKernel, blurColumnKernel;
  cl_command_queue queue;

  void* hostPtr;
  float* kernelBufferPtr;
  MagickSizeType length;

  char geometry[MaxTextExtent];
  KernelInfo* kernel = NULL;
  unsigned int kernelWidth;
  unsigned int imageColumns, imageRows;

  unsigned int i;

  context = NULL;
  filteredImage = NULL;
  inputImageBuffer = NULL;
  tempImageBuffer = NULL;
  filteredImageBuffer = NULL;
  imageKernelBuffer = NULL;
  blurRowKernel = NULL;
  blurColumnKernel = NULL;
  queue = NULL;

  outputReady = MagickFalse;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  /* Create and initialize OpenCL buffers. */
  {
    inputPixels = NULL;
    inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
    if (inputPixels == (const void *) NULL)
    {
      (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
      goto cleanup;
    }
    /* If the host pointer is aligned to the size of CLPixelPacket, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
    if (ALIGNED(inputPixels,CLPixelPacket)) 
    {
      mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
    }
    else 
    {
      mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
    }
    /* create a CL buffer from image pixel buffer */
    length = inputImage->columns * inputImage->rows;
    inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  /* create output */
  {
    filteredImage = CloneImage(inputImage,inputImage->columns,inputImage->rows,MagickTrue,exception);
    assert(filteredImage != NULL);
    if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
      goto cleanup;
    }
    filteredPixels = GetPixelCachePixels(filteredImage, &length, exception);
    if (filteredPixels == (void *) NULL)
    {
      (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
      goto cleanup;
    }

    if (ALIGNED(filteredPixels,CLPixelPacket)) 
    {
      mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
      hostPtr = filteredPixels;
    }
    else 
    {
      mem_flags = CL_MEM_WRITE_ONLY;
      hostPtr = NULL;
    }
    /* create a CL buffer from image pixel buffer */
    length = inputImage->columns * inputImage->rows;
    filteredImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  /* create processing kernel */
  {
    (void) FormatLocaleString(geometry,MaxTextExtent,"blur:%.20gx%.20g;blur:%.20gx%.20g+90",radius,sigma,radius,sigma);
    kernel=AcquireKernelInfo(geometry);
    if (kernel == (KernelInfo *) NULL)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "MemoryAllocationFailed.",".");
      goto cleanup;
    }

    imageKernelBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, kernel->width * sizeof(float), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
    kernelBufferPtr = (float*)clEnqueueMapBuffer(queue, imageKernelBuffer, CL_TRUE, CL_MAP_WRITE, 0, kernel->width * sizeof(float), 0, NULL, NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueMapBuffer failed.",".");
      goto cleanup;
    }

    for (i = 0; i < kernel->width; i++)
    {
      kernelBufferPtr[i] = (float) kernel->values[i];
    }

    clStatus = clEnqueueUnmapMemObject(queue, imageKernelBuffer, kernelBufferPtr, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.", "'%s'", ".");
      goto cleanup;
    }
  }

  {
    unsigned int offsetRows;
    unsigned int sec;

    /* create temp buffer */
    {
      length = inputImage->columns * (inputImage->rows / 2 + 1 + (kernel->width-1) / 2);
      tempImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, length * 4 * sizeof(float), NULL, &clStatus);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
        goto cleanup;
      }
    }

    /* get the OpenCL kernels */
    {
      blurRowKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "BlurRowSection");
      if (blurRowKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };

      blurColumnKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "BlurColumnSection");
      if (blurColumnKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };
    }

    for (sec = 0; sec < 2; sec++)
    {
      {
        /* need logic to decide this value */
        int chunkSize = 256;

        {
          imageColumns = inputImage->columns;
          if (sec == 0)
            imageRows = inputImage->rows / 2 + (kernel->width-1) / 2;
          else
            imageRows = (inputImage->rows - inputImage->rows / 2) + (kernel->width-1) / 2;

          offsetRows = sec * inputImage->rows / 2;

          kernelWidth = kernel->width;

          /* set the kernel arguments */
          i = 0;
          clStatus=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
          clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
          clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(ChannelType),&channel);
          clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
          clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
          clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
          clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageRows);
          clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(CLPixelPacket)*(chunkSize+kernel->width),(void *)NULL);
          clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&offsetRows);
          clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&sec);
          if (clStatus != CL_SUCCESS)
          {
            (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
            goto cleanup;
          }
        }

        /* launch the kernel */
        {
          size_t gsize[2];
          size_t wsize[2];

          gsize[0] = chunkSize*((imageColumns+chunkSize-1)/chunkSize);
          gsize[1] = imageRows;
          wsize[0] = chunkSize;
          wsize[1] = 1;

          clStatus = clEnqueueNDRangeKernel(queue, blurRowKernel, 2, NULL, gsize, wsize, 0, NULL, NULL);
          if (clStatus != CL_SUCCESS)
          {
            (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
            goto cleanup;
          }
          clFlush(queue);
        }
      }

      {
        /* need logic to decide this value */
        int chunkSize = 256;

        {
          imageColumns = inputImage->columns;
          if (sec == 0)
            imageRows = inputImage->rows / 2;
          else
            imageRows = (inputImage->rows - inputImage->rows / 2);

          offsetRows = sec * inputImage->rows / 2;

          kernelWidth = kernel->width;

          /* set the kernel arguments */
          i = 0;
          clStatus=clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
          clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
          clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(ChannelType),&channel);
          clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
          clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
          clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
          clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&imageRows);
          clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(cl_float4)*(chunkSize+kernel->width),(void *)NULL);
          clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&offsetRows);
          clStatus|=clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&sec);
          if (clStatus != CL_SUCCESS)
          {
            (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
            goto cleanup;
          }
        }

        /* launch the kernel */
        {
          size_t gsize[2];
          size_t wsize[2];

          gsize[0] = imageColumns;
          gsize[1] = chunkSize*((imageRows+chunkSize-1)/chunkSize);
          wsize[0] = 1;
          wsize[1] = chunkSize;

          clStatus = clEnqueueNDRangeKernel(queue, blurColumnKernel, 2, NULL, gsize, wsize, 0, NULL, NULL);
          if (clStatus != CL_SUCCESS)
          {
            (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
            goto cleanup;
          }
          clFlush(queue);
        }
      }
    }

  }

  /* get result */
  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (inputImageBuffer!=NULL)     clReleaseMemObject(inputImageBuffer);
  if (tempImageBuffer!=NULL)      clReleaseMemObject(tempImageBuffer);
  if (filteredImageBuffer!=NULL)  clReleaseMemObject(filteredImageBuffer);
  if (imageKernelBuffer!=NULL)    clReleaseMemObject(imageKernelBuffer);
  if (blurRowKernel!=NULL)        RelinquishOpenCLKernel(clEnv, blurRowKernel);
  if (blurColumnKernel!=NULL)     RelinquishOpenCLKernel(clEnv, blurColumnKernel);
  if (queue != NULL)              RelinquishOpenCLCommandQueue(clEnv, queue);
  if (kernel!=NULL)               DestroyKernelInfo(kernel);
  if (outputReady == MagickFalse)
  {
    if (filteredImage != NULL)
    {
      DestroyImage(filteredImage);
      filteredImage = NULL;
    }
  }
  return filteredImage;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     B l u r I m a g e  w i t h  O p e n C L                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BlurImage() blurs an image.  We convolve the image with a Gaussian operator
%  of the given radius and standard deviation (sigma).  For reasonable results,
%  the radius should be larger than sigma.  Use a radius of 0 and BlurImage()
%  selects a suitable radius for you.
%
%  The format of the BlurImage method is:
%
%      Image *BlurImage(const Image *image,const double radius,
%        const double sigma,ExceptionInfo *exception)
%      Image *BlurImageChannel(const Image *image,const ChannelType channel,
%        const double radius,const double sigma,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport
Image* AccelerateBlurImage(const Image *image, const ChannelType channel, const double radius, const double sigma,ExceptionInfo *exception)
{
  MagickBooleanType status;
  Image* filteredImage = NULL;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  status = checkOpenCLEnvironment(exception);
  if (status == MagickFalse)
    return NULL;

  status = checkAccelerateCondition(image, channel);
  if (status == MagickFalse)
    return NULL;

  if (splitImage(image) && (image->rows / 2 > radius)) 
    filteredImage = ComputeBlurImageSection(image, channel, radius, sigma, exception);
  else
    filteredImage = ComputeBlurImage(image, channel, radius, sigma, exception);

  return filteredImage;
}


static Image* ComputeRadialBlurImage(const Image *inputImage, const ChannelType channel, const double angle, ExceptionInfo *exception)
{

  MagickBooleanType outputReady;
  Image* filteredImage;
  MagickCLEnv clEnv;

  cl_int clStatus;
  size_t global_work_size[2];

  cl_context context;
  cl_mem_flags mem_flags;
  cl_mem inputImageBuffer, filteredImageBuffer, sinThetaBuffer, cosThetaBuffer;
  cl_kernel radialBlurKernel;
  cl_command_queue queue;

  const void *inputPixels;
  void *filteredPixels;
  void* hostPtr;
  float* sinThetaPtr;
  float* cosThetaPtr;
  MagickSizeType length;
  unsigned int matte;
  MagickPixelPacket bias;
  cl_float4 biasPixel;
  cl_float2 blurCenter;
  float blurRadius;
  unsigned int cossin_theta_size;
  float offset, theta;

  unsigned int i;

  outputReady = MagickFalse;
  context = NULL;
  filteredImage = NULL;
  inputImageBuffer = NULL;
  filteredImageBuffer = NULL;
  sinThetaBuffer = NULL;
  cosThetaBuffer = NULL;
  queue = NULL;
  radialBlurKernel = NULL;


  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);


  /* Create and initialize OpenCL buffers. */

  inputPixels = NULL;
  inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
  if (inputPixels == (const void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
    goto cleanup;
  }

  /* If the host pointer is aligned to the size of CLPixelPacket, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }


  filteredImage = CloneImage(inputImage,inputImage->columns,inputImage->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredPixels = GetPixelCachePixels(filteredImage, &length, exception);
  if (filteredPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
    goto cleanup;
  }

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
    hostPtr = filteredPixels;
  }
  else 
  {
    mem_flags = CL_MEM_WRITE_ONLY;
    hostPtr = NULL;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  filteredImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  blurCenter.s[0] = (float) (inputImage->columns-1)/2.0;
  blurCenter.s[1] = (float) (inputImage->rows-1)/2.0;
  blurRadius=hypot(blurCenter.s[0],blurCenter.s[1]);
  cossin_theta_size=(unsigned int) fabs(4.0*DegreesToRadians(angle)*sqrt((double)blurRadius)+2UL);

  /* create a buffer for sin_theta and cos_theta */
  sinThetaBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, cossin_theta_size * sizeof(float), NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }
  cosThetaBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, cossin_theta_size * sizeof(float), NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }


  queue = AcquireOpenCLCommandQueue(clEnv);
  sinThetaPtr = (float*) clEnqueueMapBuffer(queue, sinThetaBuffer, CL_TRUE, CL_MAP_WRITE, 0, cossin_theta_size*sizeof(float), 0, NULL, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueuemapBuffer failed.",".");
    goto cleanup;
  }

  cosThetaPtr = (float*) clEnqueueMapBuffer(queue, cosThetaBuffer, CL_TRUE, CL_MAP_WRITE, 0, cossin_theta_size*sizeof(float), 0, NULL, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueuemapBuffer failed.",".");
    goto cleanup;
  }

  theta=DegreesToRadians(angle)/(MagickRealType) (cossin_theta_size-1);
  offset=theta*(MagickRealType) (cossin_theta_size-1)/2.0;
  for (i=0; i < (ssize_t) cossin_theta_size; i++)
  {
    cosThetaPtr[i]=(float)cos((double) (theta*i-offset));
    sinThetaPtr[i]=(float)sin((double) (theta*i-offset));
  }
 
  clStatus = clEnqueueUnmapMemObject(queue, sinThetaBuffer, sinThetaPtr, 0, NULL, NULL);
  clStatus |= clEnqueueUnmapMemObject(queue, cosThetaBuffer, cosThetaPtr, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.", "'%s'", ".");
    goto cleanup;
  }

  /* get the OpenCL kernel */
  radialBlurKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "RadialBlur");
  if (radialBlurKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  
  /* set the kernel arguments */
  i = 0;
  clStatus=clSetKernelArg(radialBlurKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
  clStatus|=clSetKernelArg(radialBlurKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);

  GetMagickPixelPacket(inputImage,&bias);
  biasPixel.s[0] = bias.red;
  biasPixel.s[1] = bias.green;
  biasPixel.s[2] = bias.blue;
  biasPixel.s[3] = bias.opacity;
  clStatus|=clSetKernelArg(radialBlurKernel,i++,sizeof(cl_float4), &biasPixel);
  clStatus|=clSetKernelArg(radialBlurKernel,i++,sizeof(ChannelType), &channel);

  matte = (inputImage->matte != MagickFalse)?1:0;
  clStatus|=clSetKernelArg(radialBlurKernel,i++,sizeof(unsigned int), &matte);

  clStatus=clSetKernelArg(radialBlurKernel,i++,sizeof(cl_float2), &blurCenter);

  clStatus|=clSetKernelArg(radialBlurKernel,i++,sizeof(cl_mem),(void *)&cosThetaBuffer);
  clStatus|=clSetKernelArg(radialBlurKernel,i++,sizeof(cl_mem),(void *)&sinThetaBuffer);
  clStatus|=clSetKernelArg(radialBlurKernel,i++,sizeof(unsigned int), &cossin_theta_size);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }


  global_work_size[0] = inputImage->columns;
  global_work_size[1] = inputImage->rows;
  /* launch the kernel */
  clStatus = clEnqueueNDRangeKernel(queue, radialBlurKernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  clFlush(queue);

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }
  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (filteredImageBuffer!=NULL)  clReleaseMemObject(filteredImageBuffer);
  if (inputImageBuffer!=NULL)     clReleaseMemObject(inputImageBuffer);
  if (sinThetaBuffer!=NULL)       clReleaseMemObject(sinThetaBuffer);
  if (cosThetaBuffer!=NULL)       clReleaseMemObject(cosThetaBuffer);
  if (radialBlurKernel!=NULL)     RelinquishOpenCLKernel(clEnv, radialBlurKernel);
  if (queue != NULL)              RelinquishOpenCLCommandQueue(clEnv, queue);
  if (outputReady == MagickFalse)
  {
    if (filteredImage != NULL)
    {
      DestroyImage(filteredImage);
      filteredImage = NULL;
    }
  }
  return filteredImage;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     R a d i a l B l u r I m a g e  w i t h  O p e n C L                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RadialBlurImage() applies a radial blur to the image.
%
%  Andrew Protano contributed this effect.
%
%  The format of the RadialBlurImage method is:
%
%    Image *RadialBlurImage(const Image *image,const double angle,
%      ExceptionInfo *exception)
%    Image *RadialBlurImageChannel(const Image *image,const ChannelType channel,
%      const double angle,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o angle: the angle of the radial blur.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport
Image* AccelerateRadialBlurImage(const Image *image, const ChannelType channel, const double angle, ExceptionInfo *exception)
{
  MagickBooleanType status;
  Image* filteredImage;
  

  assert(image != NULL);
  assert(exception != NULL);

  status = checkOpenCLEnvironment(exception);
  if (status == MagickFalse)
    return NULL;

  status = checkAccelerateCondition(image, channel);
  if (status == MagickFalse)
    return NULL;

  filteredImage = ComputeRadialBlurImage(image, channel, angle, exception);
  return filteredImage;
}



static Image* ComputeUnsharpMaskImage(const Image *inputImage, const ChannelType channel,const double radius,const double sigma, 
          const double gain,const double threshold,ExceptionInfo *exception)
{
  MagickBooleanType outputReady = MagickFalse;
  Image* filteredImage = NULL;
  MagickCLEnv clEnv = NULL;

  cl_int clStatus;

  const void *inputPixels;
  void *filteredPixels;
  cl_mem_flags mem_flags;

  KernelInfo *kernel = NULL;
  char geometry[MaxTextExtent];

  cl_context context = NULL;
  cl_mem inputImageBuffer = NULL;
  cl_mem filteredImageBuffer = NULL;
  cl_mem tempImageBuffer = NULL;
  cl_mem imageKernelBuffer = NULL;
  cl_kernel blurRowKernel = NULL;
  cl_kernel unsharpMaskBlurColumnKernel = NULL;
  cl_command_queue queue = NULL;

  void* hostPtr;
  float* kernelBufferPtr;
  MagickSizeType length;
  unsigned int kernelWidth;
  float fGain;
  float fThreshold;
  unsigned int imageColumns, imageRows;
  int chunkSize;
  unsigned int i;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  /* Create and initialize OpenCL buffers. */
  {
    inputPixels = NULL;
    inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
    if (inputPixels == (const void *) NULL)
    {
      (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
      goto cleanup;
    }

    /* If the host pointer is aligned to the size of CLPixelPacket, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
    if (ALIGNED(inputPixels,CLPixelPacket)) 
    {
      mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
    }
    else 
    {
      mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
    }
    /* create a CL buffer from image pixel buffer */
    length = inputImage->columns * inputImage->rows;
    inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  /* create output */
  {
    filteredImage = CloneImage(inputImage,inputImage->columns,inputImage->rows,MagickTrue,exception);
    assert(filteredImage != NULL);
    if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
      goto cleanup;
    }
    filteredPixels = GetPixelCachePixels(filteredImage, &length, exception);
    if (filteredPixels == (void *) NULL)
    {
      (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
      goto cleanup;
    }

    if (ALIGNED(filteredPixels,CLPixelPacket)) 
    {
      mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
      hostPtr = filteredPixels;
    }
    else 
    {
      mem_flags = CL_MEM_WRITE_ONLY;
      hostPtr = NULL;
    }

    /* create a CL buffer from image pixel buffer */
    length = inputImage->columns * inputImage->rows;
    filteredImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  /* create the blur kernel */
  {
    (void) FormatLocaleString(geometry,MaxTextExtent,"blur:%.20gx%.20g;blur:%.20gx%.20g+90",radius,sigma,radius,sigma);
    kernel=AcquireKernelInfo(geometry);
    if (kernel == (KernelInfo *) NULL)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireKernelInfo failed.",".");
      goto cleanup;
    }

    imageKernelBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, kernel->width * sizeof(float), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }


    kernelBufferPtr = (float*)clEnqueueMapBuffer(queue, imageKernelBuffer, CL_TRUE, CL_MAP_WRITE, 0, kernel->width * sizeof(float), 0, NULL, NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueMapBuffer failed.",".");
      goto cleanup;
    }
    for (i = 0; i < kernel->width; i++)
    {
      kernelBufferPtr[i] = (float) kernel->values[i];
    }
    clStatus = clEnqueueUnmapMemObject(queue, imageKernelBuffer, kernelBufferPtr, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.", "'%s'", ".");
      goto cleanup;
    }
  }

  {
    /* create temp buffer */
    {
      length = inputImage->columns * inputImage->rows;
      tempImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, length * 4 * sizeof(float), NULL, &clStatus);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
        goto cleanup;
      }
    }

    /* get the opencl kernel */
    {
      blurRowKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "BlurRow");
      if (blurRowKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };

      unsharpMaskBlurColumnKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "UnsharpMaskBlurColumn");
      if (unsharpMaskBlurColumnKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };
    }

    {
      chunkSize = 256;

      imageColumns = inputImage->columns;
      imageRows = inputImage->rows;

      kernelWidth = kernel->width;

      /* set the kernel arguments */
      i = 0;
      clStatus=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
      clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
      clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(ChannelType),&channel);
      clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
      clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
      clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
      clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageRows);
      clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(CLPixelPacket)*(chunkSize+kernel->width),(void *)NULL);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
        goto cleanup;
      }
    }

    /* launch the kernel */
    {
      size_t gsize[2];
      size_t wsize[2];

      gsize[0] = chunkSize*((inputImage->columns+chunkSize-1)/chunkSize);
      gsize[1] = inputImage->rows;
      wsize[0] = chunkSize;
      wsize[1] = 1;

      clStatus = clEnqueueNDRangeKernel(queue, blurRowKernel, 2, NULL, gsize, wsize, 0, NULL, NULL);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
        goto cleanup;
      }
      clFlush(queue);
    }


    {
      chunkSize = 256;
      imageColumns = inputImage->columns;
      imageRows = inputImage->rows;
      kernelWidth = kernel->width;
      fGain = (float)gain;
      fThreshold = (float)threshold;

      i = 0;
      clStatus=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&imageRows);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++, (chunkSize+kernelWidth-1)*sizeof(cl_float4),NULL);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++, kernelWidth*sizeof(float),NULL);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(ChannelType),&channel);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(float),(void *)&fGain);
      clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(float),(void *)&fThreshold);

      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
        goto cleanup;
      }
    }

    /* launch the kernel */
    {
      size_t gsize[2];
      size_t wsize[2];

      gsize[0] = inputImage->columns;
      gsize[1] = chunkSize*((inputImage->rows+chunkSize-1)/chunkSize);
      wsize[0] = 1;
      wsize[1] = chunkSize;

      clStatus = clEnqueueNDRangeKernel(queue, unsharpMaskBlurColumnKernel, 2, NULL, gsize, wsize, 0, NULL, NULL);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
        goto cleanup;
      }
      clFlush(queue);
    }

  }

  /* get result */
  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  outputReady = MagickTrue;
  
cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (kernel != NULL)			      kernel=DestroyKernelInfo(kernel);
  if (inputImageBuffer!=NULL)		      clReleaseMemObject(inputImageBuffer);
  if (filteredImageBuffer!=NULL)              clReleaseMemObject(filteredImageBuffer);
  if (tempImageBuffer!=NULL)                  clReleaseMemObject(tempImageBuffer);
  if (imageKernelBuffer!=NULL)                clReleaseMemObject(imageKernelBuffer);
  if (blurRowKernel!=NULL)                    RelinquishOpenCLKernel(clEnv, blurRowKernel);
  if (unsharpMaskBlurColumnKernel!=NULL)      RelinquishOpenCLKernel(clEnv, unsharpMaskBlurColumnKernel);
  if (queue != NULL)                          RelinquishOpenCLCommandQueue(clEnv, queue);
  if (outputReady == MagickFalse)
  {
    if (filteredImage != NULL)
    {
      DestroyImage(filteredImage);
      filteredImage = NULL;
    }
  }
  return filteredImage;
}


static Image* ComputeUnsharpMaskImageSection(const Image *inputImage, const ChannelType channel,const double radius,const double sigma, 
          const double gain,const double threshold,ExceptionInfo *exception)
{
  MagickBooleanType outputReady = MagickFalse;
  Image* filteredImage = NULL;
  MagickCLEnv clEnv = NULL;

  cl_int clStatus;

  const void *inputPixels;
  void *filteredPixels;
  cl_mem_flags mem_flags;

  KernelInfo *kernel = NULL;
  char geometry[MaxTextExtent];

  cl_context context = NULL;
  cl_mem inputImageBuffer = NULL;
  cl_mem filteredImageBuffer = NULL;
  cl_mem tempImageBuffer = NULL;
  cl_mem imageKernelBuffer = NULL;
  cl_kernel blurRowKernel = NULL;
  cl_kernel unsharpMaskBlurColumnKernel = NULL;
  cl_command_queue queue = NULL;

  void* hostPtr;
  float* kernelBufferPtr;
  MagickSizeType length;
  unsigned int kernelWidth;
  float fGain;
  float fThreshold;
  unsigned int imageColumns, imageRows;
  int chunkSize;
  unsigned int i;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  /* Create and initialize OpenCL buffers. */
  {
    inputPixels = NULL;
    inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
    if (inputPixels == (const void *) NULL)
    {
      (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
      goto cleanup;
    }

    /* If the host pointer is aligned to the size of CLPixelPacket, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
    if (ALIGNED(inputPixels,CLPixelPacket)) 
    {
      mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
    }
    else 
    {
      mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
    }
    /* create a CL buffer from image pixel buffer */
    length = inputImage->columns * inputImage->rows;
    inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  /* create output */
  {
    filteredImage = CloneImage(inputImage,inputImage->columns,inputImage->rows,MagickTrue,exception);
    assert(filteredImage != NULL);
    if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
      goto cleanup;
    }
    filteredPixels = GetPixelCachePixels(filteredImage, &length, exception);
    if (filteredPixels == (void *) NULL)
    {
      (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
      goto cleanup;
    }

    if (ALIGNED(filteredPixels,CLPixelPacket)) 
    {
      mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
      hostPtr = filteredPixels;
    }
    else 
    {
      mem_flags = CL_MEM_WRITE_ONLY;
      hostPtr = NULL;
    }

    /* create a CL buffer from image pixel buffer */
    length = inputImage->columns * inputImage->rows;
    filteredImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  /* create the blur kernel */
  {
    (void) FormatLocaleString(geometry,MaxTextExtent,"blur:%.20gx%.20g;blur:%.20gx%.20g+90",radius,sigma,radius,sigma);
    kernel=AcquireKernelInfo(geometry);
    if (kernel == (KernelInfo *) NULL)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireKernelInfo failed.",".");
      goto cleanup;
    }

    imageKernelBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, kernel->width * sizeof(float), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }


    kernelBufferPtr = (float*)clEnqueueMapBuffer(queue, imageKernelBuffer, CL_TRUE, CL_MAP_WRITE, 0, kernel->width * sizeof(float), 0, NULL, NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueMapBuffer failed.",".");
      goto cleanup;
    }
    for (i = 0; i < kernel->width; i++)
    {
      kernelBufferPtr[i] = (float) kernel->values[i];
    }
    clStatus = clEnqueueUnmapMemObject(queue, imageKernelBuffer, kernelBufferPtr, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.", "'%s'", ".");
      goto cleanup;
    }
  }

  {
    unsigned int offsetRows;
    unsigned int sec;

    /* create temp buffer */
    {
      length = inputImage->columns * (inputImage->rows / 2 + 1 + (kernel->width-1) / 2);
      tempImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, length * 4 * sizeof(float), NULL, &clStatus);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
        goto cleanup;
      }
    }

    /* get the opencl kernel */
    {
      blurRowKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "BlurRowSection");
      if (blurRowKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };

      unsharpMaskBlurColumnKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "UnsharpMaskBlurColumnSection");
      if (unsharpMaskBlurColumnKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };
    }

    for (sec = 0; sec < 2; sec++)
    {
      {
        chunkSize = 256;

        imageColumns = inputImage->columns;
        if (sec == 0)
          imageRows = inputImage->rows / 2 + (kernel->width-1) / 2;
        else
          imageRows = (inputImage->rows - inputImage->rows / 2) + (kernel->width-1) / 2;

        offsetRows = sec * inputImage->rows / 2;

        kernelWidth = kernel->width;

        /* set the kernel arguments */
        i = 0;
        clStatus=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(ChannelType),&channel);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageRows);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(CLPixelPacket)*(chunkSize+kernel->width),(void *)NULL);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&offsetRows);
        clStatus|=clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&sec);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
          goto cleanup;
        }
      }
      /* launch the kernel */
      {
        size_t gsize[2];
        size_t wsize[2];

        gsize[0] = chunkSize*((imageColumns+chunkSize-1)/chunkSize);
        gsize[1] = imageRows;
        wsize[0] = chunkSize;
        wsize[1] = 1;

        clStatus = clEnqueueNDRangeKernel(queue, blurRowKernel, 2, NULL, gsize, wsize, 0, NULL, NULL);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
          goto cleanup;
        }
        clFlush(queue);
      }


      {
        chunkSize = 256;

        imageColumns = inputImage->columns;
        if (sec == 0)
          imageRows = inputImage->rows / 2;
        else
          imageRows = (inputImage->rows - inputImage->rows / 2);

        offsetRows = sec * inputImage->rows / 2;

        kernelWidth = kernel->width;

        fGain = (float)gain;
        fThreshold = (float)threshold;

        i = 0;
        clStatus=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&imageRows);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++, (chunkSize+kernelWidth-1)*sizeof(cl_float4),NULL);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++, kernelWidth*sizeof(float),NULL);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(ChannelType),&channel);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(float),(void *)&fGain);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(float),(void *)&fThreshold);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&offsetRows);
        clStatus|=clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&sec);

        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
          goto cleanup;
        }
      }

      /* launch the kernel */
      {
        size_t gsize[2];
        size_t wsize[2];

        gsize[0] = imageColumns;
        gsize[1] = chunkSize*((imageRows+chunkSize-1)/chunkSize);
        wsize[0] = 1;
        wsize[1] = chunkSize;

        clStatus = clEnqueueNDRangeKernel(queue, unsharpMaskBlurColumnKernel, 2, NULL, gsize, wsize, 0, NULL, NULL);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
          goto cleanup;
        }
        clFlush(queue);
      }
    }
  }

  /* get result */
  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  outputReady = MagickTrue;
  
cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (kernel != NULL)			      kernel=DestroyKernelInfo(kernel);
  if (inputImageBuffer!=NULL)		      clReleaseMemObject(inputImageBuffer);
  if (filteredImageBuffer!=NULL)              clReleaseMemObject(filteredImageBuffer);
  if (tempImageBuffer!=NULL)                  clReleaseMemObject(tempImageBuffer);
  if (imageKernelBuffer!=NULL)                clReleaseMemObject(imageKernelBuffer);
  if (blurRowKernel!=NULL)                    RelinquishOpenCLKernel(clEnv, blurRowKernel);
  if (unsharpMaskBlurColumnKernel!=NULL)      RelinquishOpenCLKernel(clEnv, unsharpMaskBlurColumnKernel);
  if (queue != NULL)                          RelinquishOpenCLCommandQueue(clEnv, queue);
  if (outputReady == MagickFalse)
  {
    if (filteredImage != NULL)
    {
      DestroyImage(filteredImage);
      filteredImage = NULL;
    }
  }
  return filteredImage;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     U n s h a r p M a s k I m a g e  w i t h  O p e n C L                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnsharpMaskImage() sharpens one or more image channels.  We convolve the
%  image with a Gaussian operator of the given radius and standard deviation
%  (sigma).  For reasonable results, radius should be larger than sigma.  Use a
%  radius of 0 and UnsharpMaskImage() selects a suitable radius for you.
%
%  The format of the UnsharpMaskImage method is:
%
%    Image *UnsharpMaskImage(const Image *image,const double radius,
%      const double sigma,const double amount,const double threshold,
%      ExceptionInfo *exception)
%    Image *UnsharpMaskImageChannel(const Image *image,
%      const ChannelType channel,const double radius,const double sigma,
%      const double gain,const double threshold,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel type.
%
%    o radius: the radius of the Gaussian, in pixels, not counting the center
%      pixel.
%
%    o sigma: the standard deviation of the Gaussian, in pixels.
%
%    o gain: the percentage of the difference between the original and the
%      blur image that is added back into the original.
%
%    o threshold: the threshold in pixels needed to apply the diffence gain.
%
%    o exception: return any errors or warnings in this structure.
%
*/


MagickExport
Image* AccelerateUnsharpMaskImage(const Image *image, const ChannelType channel,const double radius,const double sigma, 
          const double gain,const double threshold,ExceptionInfo *exception)
{
  MagickBooleanType status;
  Image* filteredImage;
  

  assert(image != NULL);
  assert(exception != NULL);

  status = checkOpenCLEnvironment(exception);
  if (status == MagickFalse)
    return NULL;

  status = checkAccelerateCondition(image, channel);
  if (status == MagickFalse)
    return NULL;

  if (splitImage(image) && (image->rows / 2 > radius)) 
    filteredImage = ComputeUnsharpMaskImageSection(image,channel,radius,sigma,gain,threshold,exception);
  else
    filteredImage = ComputeUnsharpMaskImage(image,channel,radius,sigma,gain,threshold,exception);
  return filteredImage;

}

static MagickBooleanType resizeHorizontalFilter(cl_mem inputImage
                                 , const unsigned int inputImageColumns, const unsigned int inputImageRows, const unsigned int matte
                                 , cl_mem resizedImage, const unsigned int resizedColumns, const unsigned int resizedRows
                                 , const ResizeFilter* resizeFilter, cl_mem resizeFilterCubicCoefficients, const float xFactor
                                 , MagickCLEnv clEnv, cl_command_queue queue, ExceptionInfo *exception)
{
  MagickBooleanType status = MagickFalse;

  float scale, support;
  unsigned int i;
  cl_kernel horizontalKernel = NULL;
  cl_int clStatus;
  size_t global_work_size[2];
  size_t local_work_size[2];
  int resizeFilterType, resizeWindowType;
  float resizeFilterScale, resizeFilterSupport, resizeFilterWindowSupport, resizeFilterBlur;
  size_t totalLocalMemorySize;
  size_t imageCacheLocalMemorySize, pixelAccumulatorLocalMemorySize
        , weightAccumulatorLocalMemorySize, gammaAccumulatorLocalMemorySize;
  size_t deviceLocalMemorySize;
  int cacheRangeStart, cacheRangeEnd, numCachedPixels;
  
  const unsigned int workgroupSize = 256;
  unsigned int pixelPerWorkgroup;
  unsigned int chunkSize;

  /*
  Apply filter to resize vertically from image to resize image.
  */
  scale=MAGICK_MAX(1.0/xFactor+MagickEpsilon,1.0);
  support=scale*GetResizeFilterSupport(resizeFilter);
  if (support < 0.5)
  {
    /*
    Support too small even for nearest neighbour: Reduce to point
    sampling.
    */
    support=(MagickRealType) 0.5;
    scale=1.0;
  }
  scale=PerceptibleReciprocal(scale);

  if (resizedColumns < workgroupSize) 
  {
    chunkSize = 32;
    pixelPerWorkgroup = 32;
  }
  else
  {
    chunkSize = workgroupSize;
    pixelPerWorkgroup = workgroupSize;
  }

  /* get the local memory size supported by the device */
  deviceLocalMemorySize = GetOpenCLDeviceLocalMemorySize(clEnv);

DisableMSCWarning(4127)
  while(1)
RestoreMSCWarning
  {
    /* calculate the local memory size needed per workgroup */
    cacheRangeStart = (int) (((0 + 0.5)/xFactor+MagickEpsilon)-support+0.5);
    cacheRangeEnd = (int) ((((pixelPerWorkgroup-1) + 0.5)/xFactor+MagickEpsilon)+support+0.5);
    numCachedPixels = cacheRangeEnd - cacheRangeStart + 1;
    imageCacheLocalMemorySize = numCachedPixels * sizeof(CLPixelPacket);
    totalLocalMemorySize = imageCacheLocalMemorySize;

    /* local size for the pixel accumulator */
    pixelAccumulatorLocalMemorySize = chunkSize * sizeof(cl_float4);
    totalLocalMemorySize+=pixelAccumulatorLocalMemorySize;

    /* local memory size for the weight accumulator */
    weightAccumulatorLocalMemorySize = chunkSize * sizeof(float);
    totalLocalMemorySize+=weightAccumulatorLocalMemorySize;

    /* local memory size for the gamma accumulator */
    if (matte == 0)
      gammaAccumulatorLocalMemorySize = sizeof(float);
    else
      gammaAccumulatorLocalMemorySize = chunkSize * sizeof(float);
    totalLocalMemorySize+=gammaAccumulatorLocalMemorySize;

    if (totalLocalMemorySize <= deviceLocalMemorySize)
      break;
    else
    {
      pixelPerWorkgroup = pixelPerWorkgroup/2;
      chunkSize = chunkSize/2;
      if (pixelPerWorkgroup == 0
          || chunkSize == 0)
      {
        /* quit, fallback to CPU */
        goto cleanup;
      }
    }
  }

  resizeFilterType = (int)GetResizeFilterWeightingType(resizeFilter);
  resizeWindowType = (int)GetResizeFilterWindowWeightingType(resizeFilter);


  if (resizeFilterType == SincFastWeightingFunction
    && resizeWindowType == SincFastWeightingFunction)
  {
    horizontalKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "ResizeHorizontalFilterSinc");
  }
  else
  {
    horizontalKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "ResizeHorizontalFilter");
  }
  if (horizontalKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  i = 0;
  clStatus = clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&inputImage);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&inputImageColumns);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&inputImageRows);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&matte);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&xFactor);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&resizedImage);

  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&resizedColumns);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&resizedRows);

  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(int), (void*)&resizeFilterType);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(int), (void*)&resizeWindowType);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&resizeFilterCubicCoefficients);

  resizeFilterScale = (float) GetResizeFilterScale(resizeFilter);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterScale);

  resizeFilterSupport = (float) GetResizeFilterSupport(resizeFilter);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterSupport);

  resizeFilterWindowSupport = (float) GetResizeFilterWindowSupport(resizeFilter);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterWindowSupport);

  resizeFilterBlur = (float) GetResizeFilterBlur(resizeFilter);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterBlur);


  clStatus |= clSetKernelArg(horizontalKernel, i++, imageCacheLocalMemorySize, NULL);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(int), &numCachedPixels);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), &pixelPerWorkgroup);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), &chunkSize);
  

  clStatus |= clSetKernelArg(horizontalKernel, i++, pixelAccumulatorLocalMemorySize, NULL);
  clStatus |= clSetKernelArg(horizontalKernel, i++, weightAccumulatorLocalMemorySize, NULL);
  clStatus |= clSetKernelArg(horizontalKernel, i++, gammaAccumulatorLocalMemorySize, NULL);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  global_work_size[0] = (resizedColumns+pixelPerWorkgroup-1)/pixelPerWorkgroup*workgroupSize;
  global_work_size[1] = resizedRows;

  local_work_size[0] = workgroupSize;
  local_work_size[1] = 1;
  clStatus = clEnqueueNDRangeKernel(queue, horizontalKernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  clFlush(queue);
  status = MagickTrue;


cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (horizontalKernel != NULL) RelinquishOpenCLKernel(clEnv, horizontalKernel);

  return status;
}


static MagickBooleanType resizeVerticalFilter(cl_mem inputImage
                                 , const unsigned int inputImageColumns, const unsigned int inputImageRows, const unsigned int matte
                                 , cl_mem resizedImage, const unsigned int resizedColumns, const unsigned int resizedRows
                                 , const ResizeFilter* resizeFilter, cl_mem resizeFilterCubicCoefficients, const float yFactor
                                 , MagickCLEnv clEnv, cl_command_queue queue, ExceptionInfo *exception)
{
  MagickBooleanType status = MagickFalse;

  float scale, support;
  unsigned int i;
  cl_kernel horizontalKernel = NULL;
  cl_int clStatus;
  size_t global_work_size[2];
  size_t local_work_size[2];
  int resizeFilterType, resizeWindowType;
  float resizeFilterScale, resizeFilterSupport, resizeFilterWindowSupport, resizeFilterBlur;
  size_t totalLocalMemorySize;
  size_t imageCacheLocalMemorySize, pixelAccumulatorLocalMemorySize
        , weightAccumulatorLocalMemorySize, gammaAccumulatorLocalMemorySize;
  size_t deviceLocalMemorySize;
  int cacheRangeStart, cacheRangeEnd, numCachedPixels;
  
  const unsigned int workgroupSize = 256;
  unsigned int pixelPerWorkgroup;
  unsigned int chunkSize;

  /*
  Apply filter to resize vertically from image to resize image.
  */
  scale=MAGICK_MAX(1.0/yFactor+MagickEpsilon,1.0);
  support=scale*GetResizeFilterSupport(resizeFilter);
  if (support < 0.5)
  {
    /*
    Support too small even for nearest neighbour: Reduce to point
    sampling.
    */
    support=(MagickRealType) 0.5;
    scale=1.0;
  }
  scale=PerceptibleReciprocal(scale);

  if (resizedRows < workgroupSize) 
  {
    chunkSize = 32;
    pixelPerWorkgroup = 32;
  }
  else
  {
    chunkSize = workgroupSize;
    pixelPerWorkgroup = workgroupSize;
  }

  /* get the local memory size supported by the device */
  deviceLocalMemorySize = GetOpenCLDeviceLocalMemorySize(clEnv);

DisableMSCWarning(4127)
  while(1)
RestoreMSCWarning
  {
    /* calculate the local memory size needed per workgroup */
    cacheRangeStart = (int) (((0 + 0.5)/yFactor+MagickEpsilon)-support+0.5);
    cacheRangeEnd = (int) ((((pixelPerWorkgroup-1) + 0.5)/yFactor+MagickEpsilon)+support+0.5);
    numCachedPixels = cacheRangeEnd - cacheRangeStart + 1;
    imageCacheLocalMemorySize = numCachedPixels * sizeof(CLPixelPacket);
    totalLocalMemorySize = imageCacheLocalMemorySize;

    /* local size for the pixel accumulator */
    pixelAccumulatorLocalMemorySize = chunkSize * sizeof(cl_float4);
    totalLocalMemorySize+=pixelAccumulatorLocalMemorySize;

    /* local memory size for the weight accumulator */
    weightAccumulatorLocalMemorySize = chunkSize * sizeof(float);
    totalLocalMemorySize+=weightAccumulatorLocalMemorySize;

    /* local memory size for the gamma accumulator */
    if (matte == 0)
      gammaAccumulatorLocalMemorySize = sizeof(float);
    else
      gammaAccumulatorLocalMemorySize = chunkSize * sizeof(float);
    totalLocalMemorySize+=gammaAccumulatorLocalMemorySize;

    if (totalLocalMemorySize <= deviceLocalMemorySize)
      break;
    else
    {
      pixelPerWorkgroup = pixelPerWorkgroup/2;
      chunkSize = chunkSize/2;
      if (pixelPerWorkgroup == 0
          || chunkSize == 0)
      {
        /* quit, fallback to CPU */
        goto cleanup;
      }
    }
  }

  resizeFilterType = (int)GetResizeFilterWeightingType(resizeFilter);
  resizeWindowType = (int)GetResizeFilterWindowWeightingType(resizeFilter);

  if (resizeFilterType == SincFastWeightingFunction
    && resizeWindowType == SincFastWeightingFunction)
    horizontalKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "ResizeVerticalFilterSinc");
  else 
    horizontalKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "ResizeVerticalFilter");

  if (horizontalKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  i = 0;
  clStatus = clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&inputImage);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&inputImageColumns);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&inputImageRows);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&matte);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&yFactor);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&resizedImage);

  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&resizedColumns);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&resizedRows);

  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(int), (void*)&resizeFilterType);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(int), (void*)&resizeWindowType);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&resizeFilterCubicCoefficients);

  resizeFilterScale = (float) GetResizeFilterScale(resizeFilter);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterScale);

  resizeFilterSupport = (float) GetResizeFilterSupport(resizeFilter);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterSupport);

  resizeFilterWindowSupport = (float) GetResizeFilterWindowSupport(resizeFilter);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterWindowSupport);

  resizeFilterBlur = (float) GetResizeFilterBlur(resizeFilter);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterBlur);


  clStatus |= clSetKernelArg(horizontalKernel, i++, imageCacheLocalMemorySize, NULL);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(int), &numCachedPixels);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), &pixelPerWorkgroup);
  clStatus |= clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), &chunkSize);
  

  clStatus |= clSetKernelArg(horizontalKernel, i++, pixelAccumulatorLocalMemorySize, NULL);
  clStatus |= clSetKernelArg(horizontalKernel, i++, weightAccumulatorLocalMemorySize, NULL);
  clStatus |= clSetKernelArg(horizontalKernel, i++, gammaAccumulatorLocalMemorySize, NULL);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  global_work_size[0] = resizedColumns;
  global_work_size[1] = (resizedRows+pixelPerWorkgroup-1)/pixelPerWorkgroup*workgroupSize;

  local_work_size[0] = 1;
  local_work_size[1] = workgroupSize;
  clStatus = clEnqueueNDRangeKernel(queue, horizontalKernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  clFlush(queue);
  status = MagickTrue;


cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (horizontalKernel != NULL) RelinquishOpenCLKernel(clEnv, horizontalKernel);

  return status;
}



static Image* ComputeResizeImage(const Image* inputImage, const size_t resizedColumns, const size_t resizedRows
        , const ResizeFilter* resizeFilter, ExceptionInfo *exception)
{

  MagickBooleanType outputReady = MagickFalse;
  Image* filteredImage = NULL;
  MagickCLEnv clEnv = NULL;

  cl_int clStatus;
  MagickBooleanType status;
  const void *inputPixels;
  void* filteredPixels;
  void* hostPtr;
  const MagickRealType* resizeFilterCoefficient;
  float* mappedCoefficientBuffer;
  float xFactor, yFactor;
  MagickSizeType length;

  cl_mem_flags mem_flags;
  cl_context context = NULL;
  cl_mem inputImageBuffer = NULL;
  cl_mem tempImageBuffer = NULL;
  cl_mem filteredImageBuffer = NULL;
  cl_mem cubicCoefficientsBuffer = NULL;
  cl_command_queue queue = NULL;

  unsigned int i;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);

  /* Create and initialize OpenCL buffers. */
  inputPixels = NULL;
  inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
  if (inputPixels == (const void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
    goto cleanup;
  }

  /* If the host pointer is aligned to the size of CLPixelPacket, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  cubicCoefficientsBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, 7 * sizeof(float), NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }
  queue = AcquireOpenCLCommandQueue(clEnv);
  mappedCoefficientBuffer = (float*)clEnqueueMapBuffer(queue, cubicCoefficientsBuffer, CL_TRUE, CL_MAP_WRITE, 0, 7 * sizeof(float)
          , 0, NULL, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueMapBuffer failed.",".");
    goto cleanup;
  }
  resizeFilterCoefficient = GetResizeFilterCoefficient(resizeFilter);
  for (i = 0; i < 7; i++)
  {
    mappedCoefficientBuffer[i] = (float) resizeFilterCoefficient[i];
  }
  clStatus = clEnqueueUnmapMemObject(queue, cubicCoefficientsBuffer, mappedCoefficientBuffer, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.", "'%s'", ".");
    goto cleanup;
  }

  filteredImage = CloneImage(inputImage,resizedColumns,resizedRows,MagickTrue,exception);
  if (filteredImage == NULL)
    goto cleanup;

  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredPixels = GetPixelCachePixels(filteredImage, &length, exception);
  if (filteredPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
    goto cleanup;
  }

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
    hostPtr = filteredPixels;
  }
  else 
  {
    mem_flags = CL_MEM_WRITE_ONLY;
    hostPtr = NULL;
  }

  /* create a CL buffer from image pixel buffer */
  length = filteredImage->columns * filteredImage->rows;
  filteredImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  xFactor=(float) resizedColumns/(float) inputImage->columns;
  yFactor=(float) resizedRows/(float) inputImage->rows;
  if (xFactor > yFactor)
  {

    length = resizedColumns*inputImage->rows;
    tempImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, length*sizeof(CLPixelPacket), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
    
    status = resizeHorizontalFilter(inputImageBuffer, inputImage->columns, inputImage->rows, (inputImage->matte != MagickFalse)?1:0
          , tempImageBuffer, resizedColumns, inputImage->rows
          , resizeFilter, cubicCoefficientsBuffer
          , xFactor, clEnv, queue, exception);
    if (status != MagickTrue)
      goto cleanup;
    
    status = resizeVerticalFilter(tempImageBuffer, resizedColumns, inputImage->rows, (inputImage->matte != MagickFalse)?1:0
       , filteredImageBuffer, resizedColumns, resizedRows
       , resizeFilter, cubicCoefficientsBuffer
       , yFactor, clEnv, queue, exception);
    if (status != MagickTrue)
      goto cleanup;
  }
  else
  {
    length = inputImage->columns*resizedRows;
    tempImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, length*sizeof(CLPixelPacket), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }

    status = resizeVerticalFilter(inputImageBuffer, inputImage->columns, inputImage->rows, (inputImage->matte != MagickFalse)?1:0
       , tempImageBuffer, inputImage->columns, resizedRows
       , resizeFilter, cubicCoefficientsBuffer
       , yFactor, clEnv, queue, exception);
    if (status != MagickTrue)
      goto cleanup;

    status = resizeHorizontalFilter(tempImageBuffer, inputImage->columns, resizedRows, (inputImage->matte != MagickFalse)?1:0
       , filteredImageBuffer, resizedColumns, resizedRows
       , resizeFilter, cubicCoefficientsBuffer
       , xFactor, clEnv, queue, exception);
    if (status != MagickTrue)
      goto cleanup;
  }
  length = resizedColumns*resizedRows;
  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    clStatus = clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }
  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (inputImageBuffer!=NULL)		  clReleaseMemObject(inputImageBuffer);
  if (tempImageBuffer!=NULL)		  clReleaseMemObject(tempImageBuffer);
  if (filteredImageBuffer!=NULL)	  clReleaseMemObject(filteredImageBuffer);
  if (cubicCoefficientsBuffer!=NULL)      clReleaseMemObject(cubicCoefficientsBuffer);
  if (queue != NULL)  	                  RelinquishOpenCLCommandQueue(clEnv, queue);
  if (outputReady == MagickFalse)
  {
    if (filteredImage != NULL)
    {
      DestroyImage(filteredImage);
      filteredImage = NULL;
    }
  }

  return filteredImage;
}

const ResizeWeightingFunctionType supportedResizeWeighting[] = 
{
  BoxWeightingFunction
  ,TriangleWeightingFunction
  ,HanningWeightingFunction
  ,HammingWeightingFunction
  ,BlackmanWeightingFunction
  ,CubicBCWeightingFunction
  ,SincWeightingFunction
  ,SincFastWeightingFunction
  ,LastWeightingFunction
};

static MagickBooleanType gpuSupportedResizeWeighting(ResizeWeightingFunctionType f)
{
  MagickBooleanType supported = MagickFalse;
  unsigned int i;
  for (i = 0; ;i++)
  {
    if (supportedResizeWeighting[i] == LastWeightingFunction)
      break;
    if (supportedResizeWeighting[i] == f)
    {
      supported = MagickTrue;
      break;
    }
  }
  return supported;
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c c e l e r a t e R e s i z e I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AccelerateResizeImage() is an OpenCL implementation of ResizeImage()
%
%  AccelerateResizeImage() scales an image to the desired dimensions, using the given
%  filter (see AcquireFilterInfo()).
%
%  If an undefined filter is given the filter defaults to Mitchell for a
%  colormapped image, a image with a matte channel, or if the image is
%  enlarged.  Otherwise the filter defaults to a Lanczos.
%
%  AccelerateResizeImage() was inspired by Paul Heckbert's "zoom" program.
%
%  The format of the AccelerateResizeImage method is:
%
%      Image *ResizeImage(Image *image,const size_t columns,
%        const size_t rows, const ResizeFilter* filter,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the scaled image.
%
%    o rows: the number of rows in the scaled image.
%
%    o filter: Image filter to use.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport
Image* AccelerateResizeImage(const Image* image, const size_t resizedColumns, const size_t resizedRows
          , const ResizeFilter* resizeFilter, ExceptionInfo *exception) 
{
  MagickBooleanType status;
  Image* filteredImage;

  assert(image != NULL);
  assert(resizeFilter != NULL);

  status = checkOpenCLEnvironment(exception);
  if (status == MagickFalse)
    return NULL;

  status = checkAccelerateCondition(image, AllChannels);
  if (status == MagickFalse)
    return NULL;

  if (gpuSupportedResizeWeighting(GetResizeFilterWeightingType(resizeFilter)) == MagickFalse
    || gpuSupportedResizeWeighting(GetResizeFilterWindowWeightingType(resizeFilter)) == MagickFalse)
    return NULL;

  filteredImage = ComputeResizeImage(image,resizedColumns,resizedRows,resizeFilter,exception);
  return filteredImage;

}


static MagickBooleanType ComputeContrastImage(Image *inputImage, const MagickBooleanType sharpen, ExceptionInfo *exception)
{
  MagickBooleanType outputReady = MagickFalse;
  MagickCLEnv clEnv = NULL;

  cl_int clStatus;
  size_t global_work_size[2];

  void *inputPixels = NULL;
  MagickSizeType length;
  unsigned int uSharpen;
  unsigned int i;

  cl_mem_flags mem_flags;
  cl_context context = NULL;
  cl_mem inputImageBuffer = NULL;
  cl_kernel filterKernel = NULL;
  cl_command_queue queue = NULL;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);

  /* Create and initialize OpenCL buffers. */
  inputPixels = GetPixelCachePixels(inputImage, &length, exception);
  if (inputPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
    goto cleanup;
  }

  /* If the host pointer is aligned to the size of CLPixelPacket, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }
  
  filterKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "Contrast");
  if (filterKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  i = 0;
  clStatus=clSetKernelArg(filterKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);

  uSharpen = (sharpen == MagickFalse)?0:1;
  clStatus|=clSetKernelArg(filterKernel,i++,sizeof(cl_uint),&uSharpen);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  global_work_size[0] = inputImage->columns;
  global_work_size[1] = inputImage->rows;
  /* launch the kernel */
  queue = AcquireOpenCLCommandQueue(clEnv);
  clStatus = clEnqueueNDRangeKernel(queue, filterKernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  clFlush(queue);

  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, inputImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, inputImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), inputPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }
  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (inputImageBuffer!=NULL)		      clReleaseMemObject(inputImageBuffer);
  if (filterKernel!=NULL)                     RelinquishOpenCLKernel(clEnv, filterKernel);
  if (queue != NULL)                          RelinquishOpenCLCommandQueue(clEnv, queue);
  return outputReady;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o n t r a s t I m a g e  w i t h  O p e n C L                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ContrastImage() enhances the intensity differences between the lighter and
%  darker elements of the image.  Set sharpen to a MagickTrue to increase the
%  image contrast otherwise the contrast is reduced.
%
%  The format of the ContrastImage method is:
%
%      MagickBooleanType ContrastImage(Image *image,
%        const MagickBooleanType sharpen)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o sharpen: Increase or decrease image contrast.
%
*/

MagickExport
MagickBooleanType AccelerateContrastImage(Image* image, const MagickBooleanType sharpen, ExceptionInfo* exception)
{
  MagickBooleanType status;

  assert(image != NULL);
  assert(exception != NULL);

  status = checkOpenCLEnvironment(exception);
  if (status == MagickFalse)
    return MagickFalse;

  status = checkAccelerateCondition(image, AllChannels);
  if (status == MagickFalse)
    return MagickFalse;

  status = ComputeContrastImage(image,sharpen,exception);
  return status;
}



MagickBooleanType ComputeModulateImage(Image* image, double percent_brightness, double percent_hue, double percent_saturation, ColorspaceType colorspace, ExceptionInfo* exception)
{
  register ssize_t
    i;

  cl_float
    bright,
    hue,
    saturation;

  cl_int color;

  MagickBooleanType outputReady;

  MagickCLEnv clEnv;

  void *inputPixels;

  MagickSizeType length;

  cl_context context;
  cl_command_queue queue;
  cl_kernel modulateKernel; 

  cl_mem inputImageBuffer;
  cl_mem_flags mem_flags;

  cl_int clStatus;

  Image * inputImage = image;

  inputImageBuffer = NULL;
  modulateKernel = NULL; 

  assert(inputImage != (Image *) NULL);
  assert(inputImage->signature == MagickSignature);
  if (inputImage->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",inputImage->filename);

  /*
   * initialize opencl env
   */
  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  outputReady = MagickFalse;

  /* Create and initialize OpenCL buffers.
   inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
   assume this  will get a writable image
   */
  inputPixels = GetPixelCachePixels(inputImage, &length, exception);
  if (inputPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
    goto cleanup;
  }

  /* If the host pointer is aligned to the size of CLPixelPacket, 
   then use the host buffer directly from the GPU; otherwise, 
   create a buffer on the GPU and copy the data over
   */
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  modulateKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "Modulate");
  if (modulateKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  bright=percent_brightness;
  hue=percent_hue;
  saturation=percent_saturation;
  color=colorspace;

  i = 0;
  clStatus=clSetKernelArg(modulateKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
  clStatus|=clSetKernelArg(modulateKernel,i++,sizeof(cl_float),&bright);
  clStatus|=clSetKernelArg(modulateKernel,i++,sizeof(cl_float),&hue);
  clStatus|=clSetKernelArg(modulateKernel,i++,sizeof(cl_float),&saturation);
  clStatus|=clSetKernelArg(modulateKernel,i++,sizeof(cl_float),&color);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
    printf("no kernel\n");
    goto cleanup;
  }

  {
    size_t global_work_size[2];
    global_work_size[0] = inputImage->columns;
    global_work_size[1] = inputImage->rows;
    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, modulateKernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    clFlush(queue);
  }

  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, inputImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, inputImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), inputPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (inputPixels) {
    //ReleasePixelCachePixels();
    inputPixels = NULL;
  }

  if (inputImageBuffer!=NULL)		      
    clReleaseMemObject(inputImageBuffer);
  if (modulateKernel!=NULL)                     
    RelinquishOpenCLKernel(clEnv, modulateKernel);
  if (queue != NULL)                          
    RelinquishOpenCLCommandQueue(clEnv, queue);

  return outputReady;

}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M o d u l a t e I m a g e  w i t h  O p e n C L                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ModulateImage() lets you control the brightness, saturation, and hue
%  of an image.  Modulate represents the brightness, saturation, and hue
%  as one parameter (e.g. 90,150,100).  If the image colorspace is HSL, the
%  modulation is lightness, saturation, and hue.  For HWB, use blackness,
%  whiteness, and hue. And for HCL, use chrome, luma, and hue.
%
%  The format of the ModulateImage method is:
%
%      MagickBooleanType ModulateImage(Image *image,const char *modulate)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o percent_*: Define the percent change in brightness, saturation, and
%      hue.
%
*/

MagickExport
MagickBooleanType AccelerateModulateImage(Image* image, double percent_brightness, double percent_hue, double percent_saturation, ColorspaceType colorspace, ExceptionInfo* exception)
{
  MagickBooleanType status;

  assert(image != NULL);
  assert(exception != NULL);

  status = checkOpenCLEnvironment(exception);
  if (status == MagickFalse)
    return MagickFalse;

  status = checkAccelerateCondition(image, AllChannels);
  if (status == MagickFalse)
    return MagickFalse;

  if ((colorspace != HSLColorspace && colorspace != UndefinedColorspace))
    return MagickFalse;


  status = ComputeModulateImage(image,percent_brightness, percent_hue, percent_saturation, colorspace, exception);
  return status;
}


MagickExport MagickBooleanType ComputeEqualizeImage(Image *inputImage, const ChannelType channel, ExceptionInfo * _exception)
{
#define EqualizeImageTag  "Equalize/Image"

  ExceptionInfo
    *exception=_exception;

  FloatPixelPacket
    white,
    black,
    intensity,
    *map;

  cl_uint4
    *histogram;

  PixelPacket
    *equalize_map;

  register ssize_t
    i;

  Image * image = inputImage;

  MagickBooleanType outputReady;
  MagickCLEnv clEnv;

  cl_int clStatus;
  size_t global_work_size[2];

  void *inputPixels;
  cl_mem_flags mem_flags;

  cl_context context;
  cl_mem inputImageBuffer;
  cl_mem histogramBuffer;
  cl_mem equalizeMapBuffer;
  cl_kernel histogramKernel; 
  cl_kernel equalizeKernel; 
  cl_command_queue queue;
  cl_int colorspace;

  void* hostPtr;

  MagickSizeType length;

  inputPixels = NULL;
  inputImageBuffer = NULL;
  histogramBuffer = NULL;
  histogramKernel = NULL; 
  equalizeKernel = NULL; 
  context = NULL;
  queue = NULL;
  outputReady = MagickFalse;

  assert(inputImage != (Image *) NULL);
  assert(inputImage->signature == MagickSignature);
  if (inputImage->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",inputImage->filename);

  /*
    Allocate and initialize histogram arrays.
  */
  histogram=(cl_uint4 *) AcquireQuantumMemory(MaxMap+1UL, sizeof(*histogram));
  if (histogram == (cl_uint4 *) NULL)
      ThrowBinaryException(ResourceLimitWarning,"MemoryAllocationFailed", image->filename);

  /* reset histogram */
  (void) ResetMagickMemory(histogram,0,(MaxMap+1)*sizeof(*histogram));

  /*
   * initialize opencl env
   */
  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  /* Create and initialize OpenCL buffers. */
  /* inputPixels = AcquirePixelCachePixels(inputImage, &length, exception); */
  /* assume this  will get a writable image */
  inputPixels = GetPixelCachePixels(inputImage, &length, exception);

  if (inputPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
    goto cleanup;
  }
  /* If the host pointer is aligned to the size of CLPixelPacket, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }
  
  /* If the host pointer is aligned to the size of cl_uint, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
  if (ALIGNED(histogram,cl_uint4)) 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR;
    hostPtr = histogram;
  }
  else 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR;
    hostPtr = histogram;
  }
  /* create a CL buffer for histogram  */
  length = (MaxMap+1); 
  histogramBuffer = clCreateBuffer(context, mem_flags, length * sizeof(cl_uint4), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  switch (inputImage->colorspace)
  {
  case RGBColorspace:
    colorspace = 1;
    break;
  case sRGBColorspace:
    colorspace = 0;
    break;
  default:
    {
    /* something is wrong, as we checked in checkAccelerateCondition */
    }
  }

  /* get the OpenCL kernel */
  histogramKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "Histogram");
  if (histogramKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  /* set the kernel arguments */
  i = 0;
  clStatus=clSetKernelArg(histogramKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
  clStatus|=clSetKernelArg(histogramKernel,i++,sizeof(ChannelType),&channel);
  clStatus|=clSetKernelArg(histogramKernel,i++,sizeof(cl_int),&colorspace);
  clStatus|=clSetKernelArg(histogramKernel,i++,sizeof(cl_mem),(void *)&histogramBuffer);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  /* launch the kernel */
  global_work_size[0] = inputImage->columns;
  global_work_size[1] = inputImage->rows;

  clStatus = clEnqueueNDRangeKernel(queue, histogramKernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  clFlush(queue);

  /* read from the kenel output */
  if (ALIGNED(histogram,cl_uint4)) 
  {
    length = (MaxMap+1); 
    clEnqueueMapBuffer(queue, histogramBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(cl_uint4), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = (MaxMap+1); 
    clStatus = clEnqueueReadBuffer(queue, histogramBuffer, CL_TRUE, 0, length * sizeof(cl_uint4), histogram, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  /* unmap, don't block gpu to use this buffer again.  */
  if (ALIGNED(histogram,cl_uint4))
  {
    clStatus = clEnqueueUnmapMemObject(queue, histogramBuffer, histogram, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.", "'%s'", ".");
      goto cleanup;
    }
  }

  if (getenv("TEST")) {
    unsigned int i;
    for (i=0; i<(MaxMap+1UL); i++) 
    {
      printf("histogram %d: red %d\n", i, histogram[i].s[2]);
      printf("histogram %d: green %d\n", i, histogram[i].s[1]);
      printf("histogram %d: blue %d\n", i, histogram[i].s[0]);
      printf("histogram %d: opacity %d\n", i, histogram[i].s[3]);
    }
  }

  /* cpu stuff */
  equalize_map=(PixelPacket *) AcquireQuantumMemory(MaxMap+1UL, sizeof(*equalize_map));
  if (equalize_map == (PixelPacket *) NULL)
      ThrowBinaryException(ResourceLimitWarning,"MemoryAllocationFailed", image->filename);

  map=(FloatPixelPacket *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*map));
  if (map == (FloatPixelPacket *) NULL)
      ThrowBinaryException(ResourceLimitWarning,"MemoryAllocationFailed", image->filename);

  /*
    Integrate the histogram to get the equalization map.
  */
  (void) ResetMagickMemory(&intensity,0,sizeof(intensity));
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if ((channel & SyncChannels) != 0)
      {
        intensity.red+=histogram[i].s[2];
        map[i]=intensity;
        continue;
      }
    if ((channel & RedChannel) != 0)
      intensity.red+=histogram[i].s[2];
    if ((channel & GreenChannel) != 0)
      intensity.green+=histogram[i].s[1];
    if ((channel & BlueChannel) != 0)
      intensity.blue+=histogram[i].s[0];
    if ((channel & OpacityChannel) != 0)
      intensity.opacity+=histogram[i].s[3];
    if (((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace))
    {
      printf("something here\n");
      /*intensity.index+=histogram[i].index; */
    }
    map[i]=intensity;
  }
  black=map[0];
  white=map[(int) MaxMap];
  (void) ResetMagickMemory(equalize_map,0,(MaxMap+1)*sizeof(*equalize_map));
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if ((channel & SyncChannels) != 0)
      {
        if (white.red != black.red)
          equalize_map[i].red=ScaleMapToQuantum((MagickRealType) ((MaxMap*
            (map[i].red-black.red))/(white.red-black.red)));
        continue;
      }
    if (((channel & RedChannel) != 0) && (white.red != black.red))
      equalize_map[i].red=ScaleMapToQuantum((MagickRealType) ((MaxMap*
        (map[i].red-black.red))/(white.red-black.red)));
    if (((channel & GreenChannel) != 0) && (white.green != black.green))
      equalize_map[i].green=ScaleMapToQuantum((MagickRealType) ((MaxMap*
        (map[i].green-black.green))/(white.green-black.green)));
    if (((channel & BlueChannel) != 0) && (white.blue != black.blue))
      equalize_map[i].blue=ScaleMapToQuantum((MagickRealType) ((MaxMap*
        (map[i].blue-black.blue))/(white.blue-black.blue)));
    if (((channel & OpacityChannel) != 0) && (white.opacity != black.opacity))
      equalize_map[i].opacity=ScaleMapToQuantum((MagickRealType) ((MaxMap*
        (map[i].opacity-black.opacity))/(white.opacity-black.opacity)));
    /*
    if ((((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace)) &&
        (white.index != black.index))
      equalize_map[i].index=ScaleMapToQuantum((MagickRealType) ((MaxMap*
        (map[i].index-black.index))/(white.index-black.index)));
    */
  }

  histogram=(cl_uint4 *) RelinquishMagickMemory(histogram);
  map=(FloatPixelPacket *) RelinquishMagickMemory(map);

  if (image->storage_class == PseudoClass)
  {
      /*
        Equalize colormap.
      */
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        if ((channel & SyncChannels) != 0)
          {
            if (white.red != black.red)
              {
                image->colormap[i].red=equalize_map[
                  ScaleQuantumToMap(image->colormap[i].red)].red;
                image->colormap[i].green=equalize_map[
                  ScaleQuantumToMap(image->colormap[i].green)].red;
                image->colormap[i].blue=equalize_map[
                  ScaleQuantumToMap(image->colormap[i].blue)].red;
                image->colormap[i].opacity=equalize_map[
                  ScaleQuantumToMap(image->colormap[i].opacity)].red;
              }
            continue;
          }
        if (((channel & RedChannel) != 0) && (white.red != black.red))
          image->colormap[i].red=equalize_map[
            ScaleQuantumToMap(image->colormap[i].red)].red;
        if (((channel & GreenChannel) != 0) && (white.green != black.green))
          image->colormap[i].green=equalize_map[
            ScaleQuantumToMap(image->colormap[i].green)].green;
        if (((channel & BlueChannel) != 0) && (white.blue != black.blue))
          image->colormap[i].blue=equalize_map[
            ScaleQuantumToMap(image->colormap[i].blue)].blue;
        if (((channel & OpacityChannel) != 0) &&
            (white.opacity != black.opacity))
          image->colormap[i].opacity=equalize_map[
            ScaleQuantumToMap(image->colormap[i].opacity)].opacity;
      }
  }

  /*
    Equalize image.
  */

  /* GPU can work on this again, image and equalize map as input
    image:        uchar4 (CLPixelPacket)
    equalize_map: uchar4 (PixelPacket)
    black, white: float4 (FloatPixelPacket) */

  if (inputImageBuffer!=NULL)		      
    clReleaseMemObject(inputImageBuffer);
 
  /* If the host pointer is aligned to the size of CLPixelPacket, 
     then use the host buffer directly from the GPU; otherwise, 
     create a buffer on the GPU and copy the data over */
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  /* Create and initialize OpenCL buffers. */
  if (ALIGNED(equalize_map, PixelPacket)) 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
    hostPtr = equalize_map;
  }
  else 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR;
    hostPtr = equalize_map;
  }
  /* create a CL buffer for eqaulize_map  */
  length = (MaxMap+1); 
  equalizeMapBuffer = clCreateBuffer(context, mem_flags, length * sizeof(PixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  /* get the OpenCL kernel */
  equalizeKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "Equalize");
  if (equalizeKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  /* set the kernel arguments */
  i = 0;
  clStatus=clSetKernelArg(equalizeKernel,i++,sizeof(cl_mem),(void *)&inputImageBuffer);
  clStatus|=clSetKernelArg(equalizeKernel,i++,sizeof(ChannelType),&channel);
  clStatus|=clSetKernelArg(equalizeKernel,i++,sizeof(cl_mem),(void *)&equalizeMapBuffer);
  clStatus|=clSetKernelArg(equalizeKernel,i++,sizeof(FloatPixelPacket),&white);
  clStatus|=clSetKernelArg(equalizeKernel,i++,sizeof(FloatPixelPacket),&black);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  /* launch the kernel */
  global_work_size[0] = inputImage->columns;
  global_work_size[1] = inputImage->rows;

  clStatus = clEnqueueNDRangeKernel(queue, equalizeKernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  clFlush(queue);

  /* read the data back */
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, inputImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, inputImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), inputPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  outputReady = MagickTrue;
  
  equalize_map=(PixelPacket *) RelinquishMagickMemory(equalize_map);

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (inputPixels) {
    /*ReleasePixelCachePixels();*/
    inputPixels = NULL;
  }

  if (inputImageBuffer!=NULL)		      
    clReleaseMemObject(inputImageBuffer);
  if (histogramBuffer!=NULL)		      
    clReleaseMemObject(histogramBuffer);
  if (histogramKernel!=NULL)                     
    RelinquishOpenCLKernel(clEnv, histogramKernel);
  if (queue != NULL)                          
    RelinquishOpenCLCommandQueue(clEnv, queue);

  return outputReady;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     E q u a l i z e I m a g e  w i t h  O p e n C L                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EqualizeImage() applies a histogram equalization to the image.
%
%  The format of the EqualizeImage method is:
%
%      MagickBooleanType EqualizeImage(Image *image)
%      MagickBooleanType EqualizeImageChannel(Image *image,
%        const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
*/


MagickExport
MagickBooleanType AccelerateEqualizeImage(Image* image, const ChannelType channel, ExceptionInfo* exception)
{
  MagickBooleanType status;

  assert(image != NULL);
  assert(exception != NULL);

  status = checkOpenCLEnvironment(exception);
  if (status == MagickFalse)
    return MagickFalse;

  status = checkAccelerateCondition(image, channel);
  if (status == MagickFalse)
    return MagickFalse;

  /* ensure this is the only pass get in for now. */
  if ((channel & SyncChannels) == 0)
    return MagickFalse;

  if (image->colorspace != sRGBColorspace)
    return MagickFalse;

  status = ComputeEqualizeImage(image,channel,exception);
  return status;
}


static Image* ComputeDespeckleImage(const Image* inputImage, ExceptionInfo* exception)
{

  MagickBooleanType outputReady = MagickFalse;
  MagickCLEnv clEnv = NULL;

  cl_int clStatus;
  size_t global_work_size[2];

  const void *inputPixels = NULL;
  Image* filteredImage = NULL;
  void *filteredPixels = NULL;
  void *hostPtr;
  MagickSizeType length;

  cl_mem_flags mem_flags;
  cl_context context = NULL;
  cl_mem inputImageBuffer = NULL;
  cl_mem tempImageBuffer[2];
  cl_mem filteredImageBuffer = NULL;
  cl_command_queue queue = NULL;
  cl_kernel hullPass1 = NULL;
  cl_kernel hullPass2 = NULL;

  unsigned int imageWidth, imageHeight;
  int matte;
  int k;

  static const int 
    X[4] = {0, 1, 1,-1},
    Y[4] = {1, 0, 1, 1};

  tempImageBuffer[0] = tempImageBuffer[1] = NULL;
  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);
 
  inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
  if (inputPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
    goto cleanup;
  }

  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  mem_flags = CL_MEM_READ_WRITE;
  length = inputImage->columns * inputImage->rows;
  for (k = 0; k < 2; k++)
  {
    tempImageBuffer[k] = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  filteredImage = CloneImage(inputImage,inputImage->columns,inputImage->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredPixels = GetPixelCachePixels(filteredImage, &length, exception);
  if (filteredPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
    goto cleanup;
  }

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
    hostPtr = filteredPixels;
  }
  else 
  {
    mem_flags = CL_MEM_WRITE_ONLY;
    hostPtr = NULL;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  filteredImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  hullPass1 = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "HullPass1");
  hullPass2 = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "HullPass2");

  clStatus =clSetKernelArg(hullPass1,0,sizeof(cl_mem),(void *)&inputImageBuffer);
  clStatus |=clSetKernelArg(hullPass1,1,sizeof(cl_mem),(void *)(tempImageBuffer+1));
  imageWidth = inputImage->columns;
  clStatus |=clSetKernelArg(hullPass1,2,sizeof(unsigned int),(void *)&imageWidth);
  imageHeight = inputImage->rows;
  clStatus |=clSetKernelArg(hullPass1,3,sizeof(unsigned int),(void *)&imageHeight);
  matte = (inputImage->matte==MagickFalse)?0:1;
  clStatus |=clSetKernelArg(hullPass1,6,sizeof(int),(void *)&matte);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  clStatus = clSetKernelArg(hullPass2,0,sizeof(cl_mem),(void *)(tempImageBuffer+1));
  clStatus |=clSetKernelArg(hullPass2,1,sizeof(cl_mem),(void *)tempImageBuffer);
  imageWidth = inputImage->columns;
  clStatus |=clSetKernelArg(hullPass2,2,sizeof(unsigned int),(void *)&imageWidth);
  imageHeight = inputImage->rows;
  clStatus |=clSetKernelArg(hullPass2,3,sizeof(unsigned int),(void *)&imageHeight);
  matte = (inputImage->matte==MagickFalse)?0:1;
  clStatus |=clSetKernelArg(hullPass2,6,sizeof(int),(void *)&matte);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }


  global_work_size[0] = inputImage->columns;
  global_work_size[1] = inputImage->rows;

  
  for (k = 0; k < 4; k++)
  {
    cl_int2 offset;
    int polarity;

    
    offset.s[0] = X[k];
    offset.s[1] = Y[k];
    polarity = 1;
    clStatus = clSetKernelArg(hullPass1,4,sizeof(cl_int2),(void *)&offset);
    clStatus|= clSetKernelArg(hullPass1,5,sizeof(int),(void *)&polarity);
    clStatus|=clSetKernelArg(hullPass2,4,sizeof(cl_int2),(void *)&offset);
    clStatus|=clSetKernelArg(hullPass2,5,sizeof(int),(void *)&polarity);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }
    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }  
    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }  


    if (k == 0)
      clStatus =clSetKernelArg(hullPass1,0,sizeof(cl_mem),(void *)(tempImageBuffer));
    offset.s[0] = -X[k];
    offset.s[1] = -Y[k];
    polarity = 1;
    clStatus = clSetKernelArg(hullPass1,4,sizeof(cl_int2),(void *)&offset);
    clStatus|= clSetKernelArg(hullPass1,5,sizeof(int),(void *)&polarity);
    clStatus|=clSetKernelArg(hullPass2,4,sizeof(cl_int2),(void *)&offset);
    clStatus|=clSetKernelArg(hullPass2,5,sizeof(int),(void *)&polarity);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }
    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }  
    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }  

    offset.s[0] = -X[k];
    offset.s[1] = -Y[k];
    polarity = -1;
    clStatus = clSetKernelArg(hullPass1,4,sizeof(cl_int2),(void *)&offset);
    clStatus|= clSetKernelArg(hullPass1,5,sizeof(int),(void *)&polarity);
    clStatus|=clSetKernelArg(hullPass2,4,sizeof(cl_int2),(void *)&offset);
    clStatus|=clSetKernelArg(hullPass2,5,sizeof(int),(void *)&polarity);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }
    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }  
    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }  

    offset.s[0] = X[k];
    offset.s[1] = Y[k];
    polarity = -1;
    clStatus = clSetKernelArg(hullPass1,4,sizeof(cl_int2),(void *)&offset);
    clStatus|= clSetKernelArg(hullPass1,5,sizeof(int),(void *)&polarity);
    clStatus|=clSetKernelArg(hullPass2,4,sizeof(cl_int2),(void *)&offset);
    clStatus|=clSetKernelArg(hullPass2,5,sizeof(int),(void *)&polarity);

    if (k == 3)
      clStatus |=clSetKernelArg(hullPass2,1,sizeof(cl_mem),(void *)&filteredImageBuffer);

    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }
    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }  
    /* launch the kernel */
    clStatus = clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }  
  }

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (queue != NULL)                          RelinquishOpenCLCommandQueue(clEnv, queue);
  if (inputImageBuffer!=NULL)		      clReleaseMemObject(inputImageBuffer);
  for (k = 0; k < 2; k++)
  {
    if (tempImageBuffer[k]!=NULL)	      clReleaseMemObject(tempImageBuffer[k]);
  }
  if (filteredImageBuffer!=NULL)	      clReleaseMemObject(filteredImageBuffer);
  if (hullPass1!=NULL)			      RelinquishOpenCLKernel(clEnv, hullPass1);
  if (hullPass2!=NULL)			      RelinquishOpenCLKernel(clEnv, hullPass2);
  if (outputReady == MagickFalse)
  {
    if (filteredImage != NULL)
    {
      DestroyImage(filteredImage);
      filteredImage = NULL;
    }
  }
  return filteredImage;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     D e s p e c k l e I m a g e  w i t h  O p e n C L                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DespeckleImage() reduces the speckle noise in an image while perserving the
%  edges of the original image.  A speckle removing filter uses a complementary 
%  hulling technique (raising pixels that are darker than their surrounding
%  neighbors, then complementarily lowering pixels that are brighter than their
%  surrounding neighbors) to reduce the speckle index of that image (reference
%  Crimmins speckle removal).
%
%  The format of the DespeckleImage method is:
%
%      Image *DespeckleImage(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

MagickExport
Image* AccelerateDespeckleImage(const Image* image, ExceptionInfo* exception)
{
  MagickBooleanType status;
  Image* newImage = NULL;

  assert(image != NULL);
  assert(exception != NULL);

  status = checkOpenCLEnvironment(exception);
  if (status == MagickFalse)
    return NULL;

  status = checkAccelerateCondition(image, AllChannels);
  if (status == MagickFalse)
    return NULL;

  newImage = ComputeDespeckleImage(image,exception);
  return newImage;
}

static Image* ComputeAddNoiseImage(const Image* inputImage, 
         const ChannelType channel, const NoiseType noise_type,
         ExceptionInfo *exception) 
{
  MagickBooleanType outputReady = MagickFalse;
  MagickCLEnv clEnv = NULL;

  cl_int clStatus;
  size_t global_work_size[2];

  const void *inputPixels = NULL;
  Image* filteredImage = NULL;
  void *filteredPixels = NULL;
  void *hostPtr;
  unsigned int inputColumns, inputRows;
  float attenuate;
  float *randomNumberBufferPtr = NULL;
  MagickSizeType length;
  unsigned int numRandomNumberPerPixel;
  unsigned int numRowsPerKernelLaunch;
  unsigned int numRandomNumberPerBuffer;
  unsigned int r;
  unsigned int k;
  int i;

  RandomInfo **restrict random_info;
  const char *option;
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  unsigned long key;
#endif

  cl_mem_flags mem_flags;
  cl_context context = NULL;
  cl_mem inputImageBuffer = NULL;
  cl_mem randomNumberBuffer = NULL;
  cl_mem filteredImageBuffer = NULL;
  cl_command_queue queue = NULL;
  cl_kernel addNoiseKernel = NULL;


  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);
 
  inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
  if (inputPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
    goto cleanup;
  }

  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }


  filteredImage = CloneImage(inputImage,inputImage->columns,inputImage->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredPixels = GetPixelCachePixels(filteredImage, &length, exception);
  if (filteredPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
    goto cleanup;
  }

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
    hostPtr = filteredPixels;
  }
  else 
  {
    mem_flags = CL_MEM_WRITE_ONLY;
    hostPtr = NULL;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  filteredImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  /* find out how many random numbers needed by pixel */
  numRandomNumberPerPixel = 0;
  {
    unsigned int numRandPerChannel = 0;
    switch (noise_type)
    {
    case UniformNoise:
    case ImpulseNoise:
    case LaplacianNoise:
    case RandomNoise:
    default:
      numRandPerChannel = 1;
      break;
    case GaussianNoise:
    case MultiplicativeGaussianNoise:
    case PoissonNoise:
      numRandPerChannel = 2;
      break;
    };

    if ((channel & RedChannel) != 0)
      numRandomNumberPerPixel+=numRandPerChannel;
    if ((channel & GreenChannel) != 0)
      numRandomNumberPerPixel+=numRandPerChannel;
    if ((channel & BlueChannel) != 0)
      numRandomNumberPerPixel+=numRandPerChannel;
    if ((channel & OpacityChannel) != 0)
      numRandomNumberPerPixel+=numRandPerChannel;
  }

  numRowsPerKernelLaunch = 512;
  /* create a buffer for random numbers */
  numRandomNumberPerBuffer = (inputImage->columns*numRowsPerKernelLaunch)*numRandomNumberPerPixel;
  randomNumberBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, numRandomNumberPerBuffer*sizeof(float)
                                      , NULL, &clStatus);


  /* set up the random number generators */
  attenuate=1.0;
  option=GetImageArtifact(inputImage,"attenuate");
  if (option != (char *) NULL)
    attenuate=StringToDouble(option,(char **) NULL);
  random_info=AcquireRandomInfoThreadSet();
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  key=GetRandomSecretKey(random_info[0]);
#endif

  addNoiseKernel = AcquireOpenCLKernel(clEnv,MAGICK_OPENCL_ACCELERATE,"AddNoiseImage");

  k = 0;
  clSetKernelArg(addNoiseKernel,k++,sizeof(cl_mem),(void *)&inputImageBuffer);
  clSetKernelArg(addNoiseKernel,k++,sizeof(cl_mem),(void *)&filteredImageBuffer);
  inputColumns = inputImage->columns;
  clSetKernelArg(addNoiseKernel,k++,sizeof(unsigned int),(void *)&inputColumns);
  inputRows = inputImage->rows;
  clSetKernelArg(addNoiseKernel,k++,sizeof(unsigned int),(void *)&inputRows);
  clSetKernelArg(addNoiseKernel,k++,sizeof(ChannelType),(void *)&channel);
  clSetKernelArg(addNoiseKernel,k++,sizeof(NoiseType),(void *)&noise_type);
  attenuate=1.0f;
  option=GetImageArtifact(inputImage,"attenuate");
  if (option != (char *) NULL)
    attenuate=(float)StringToDouble(option,(char **) NULL);
  clSetKernelArg(addNoiseKernel,k++,sizeof(float),(void *)&attenuate);
  clSetKernelArg(addNoiseKernel,k++,sizeof(cl_mem),(void *)&randomNumberBuffer);
  clSetKernelArg(addNoiseKernel,k++,sizeof(unsigned int),(void *)&numRandomNumberPerPixel);

  global_work_size[0] = inputColumns;
  for (r = 0; r < inputRows; r+=numRowsPerKernelLaunch) 
  {
    /* Generate random numbers in the buffer */
    randomNumberBufferPtr = (float*)clEnqueueMapBuffer(queue, randomNumberBuffer, CL_TRUE, CL_MAP_WRITE, 0
      , numRandomNumberPerBuffer*sizeof(float), 0, NULL, NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueMapBuffer failed.",".");
      goto cleanup;
    }

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) \
    num_threads((key == ~0UL) == 0 ? 1 : (size_t) GetMagickResourceLimit(ThreadResource))
#endif
    for (i = 0; i < numRandomNumberPerBuffer; i++)
    {
      const int id = GetOpenMPThreadId();
      randomNumberBufferPtr[i] = (float)GetPseudoRandomValue(random_info[id]);
    }

    clStatus = clEnqueueUnmapMemObject(queue, randomNumberBuffer, randomNumberBufferPtr, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.",".");
      goto cleanup;
    }

    /* set the row offset */
    clSetKernelArg(addNoiseKernel,k,sizeof(unsigned int),(void *)&r);
    global_work_size[1] = MAGICK_MIN(numRowsPerKernelLaunch, inputRows - r);
    clEnqueueNDRangeKernel(queue,addNoiseKernel,2,NULL,global_work_size,NULL,0,NULL,NULL);
  }

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (queue!=NULL)                  RelinquishOpenCLCommandQueue(clEnv, queue);
  if (addNoiseKernel!=NULL)         RelinquishOpenCLKernel(clEnv, addNoiseKernel);
  if (inputImageBuffer!=NULL)		    clReleaseMemObject(inputImageBuffer);
  if (randomNumberBuffer!=NULL)     clReleaseMemObject(randomNumberBuffer);
  if (filteredImageBuffer!=NULL)	  clReleaseMemObject(filteredImageBuffer);
  if (outputReady == MagickFalse
      && filteredImage != NULL) 
  {
      DestroyImage(filteredImage);
      filteredImage = NULL;
  }
  return filteredImage;
}


static Image* ComputeAddNoiseImageOptRandomNum(const Image* inputImage, 
         const ChannelType channel, const NoiseType noise_type,
         ExceptionInfo *exception) 
{
  MagickBooleanType outputReady = MagickFalse;
  MagickCLEnv clEnv = NULL;

  cl_int clStatus;
  size_t global_work_size[2];
  size_t random_work_size;

  const void *inputPixels = NULL;
  Image* filteredImage = NULL;
  void *filteredPixels = NULL;
  void *hostPtr;
  unsigned int inputColumns, inputRows;
  float attenuate;
  MagickSizeType length;
  unsigned int numRandomNumberPerPixel;
  unsigned int numRowsPerKernelLaunch;
  unsigned int numRandomNumberPerBuffer;
  unsigned int numRandomNumberGenerators;
  unsigned int initRandom;
  float fNormalize;
  unsigned int r;
  unsigned int k;
  int i;
  const char *option;

  cl_mem_flags mem_flags;
  cl_context context = NULL;
  cl_mem inputImageBuffer = NULL;
  cl_mem randomNumberBuffer = NULL;
  cl_mem filteredImageBuffer = NULL;
  cl_mem randomNumberSeedsBuffer = NULL;
  cl_command_queue queue = NULL;
  cl_kernel addNoiseKernel = NULL;
  cl_kernel randomNumberGeneratorKernel = NULL;


  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);
 
  inputPixels = AcquirePixelCachePixels(inputImage, &length, exception);
  if (inputPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",inputImage->filename);
    goto cleanup;
  }

  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  inputImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }


  filteredImage = CloneImage(inputImage,inputImage->columns,inputImage->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredPixels = GetPixelCachePixels(filteredImage, &length, exception);
  if (filteredPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
    goto cleanup;
  }

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR;
    hostPtr = filteredPixels;
  }
  else 
  {
    mem_flags = CL_MEM_WRITE_ONLY;
    hostPtr = NULL;
  }
  /* create a CL buffer from image pixel buffer */
  length = inputImage->columns * inputImage->rows;
  filteredImageBuffer = clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
    goto cleanup;
  }

  /* find out how many random numbers needed by pixel */
  numRandomNumberPerPixel = 0;
  {
    unsigned int numRandPerChannel = 0;
    switch (noise_type)
    {
    case UniformNoise:
    case ImpulseNoise:
    case LaplacianNoise:
    case RandomNoise:
    default:
      numRandPerChannel = 1;
      break;
    case GaussianNoise:
    case MultiplicativeGaussianNoise:
    case PoissonNoise:
      numRandPerChannel = 2;
      break;
    };

    if ((channel & RedChannel) != 0)
      numRandomNumberPerPixel+=numRandPerChannel;
    if ((channel & GreenChannel) != 0)
      numRandomNumberPerPixel+=numRandPerChannel;
    if ((channel & BlueChannel) != 0)
      numRandomNumberPerPixel+=numRandPerChannel;
    if ((channel & OpacityChannel) != 0)
      numRandomNumberPerPixel+=numRandPerChannel;
  }

  numRowsPerKernelLaunch = 512;

  /* create a buffer for random numbers */
  numRandomNumberPerBuffer = (inputImage->columns*numRowsPerKernelLaunch)*numRandomNumberPerPixel;
  randomNumberBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, numRandomNumberPerBuffer*sizeof(float)
    , NULL, &clStatus);

  {
    /* setup the random number generators */
    unsigned long* seeds;
    numRandomNumberGenerators = 512;
    randomNumberSeedsBuffer = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR|CL_MEM_READ_WRITE
                                            , numRandomNumberGenerators * 4 * sizeof(unsigned long), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clCreateBuffer failed.",".");
      goto cleanup;
    }
    seeds = (unsigned long*) clEnqueueMapBuffer(queue, randomNumberSeedsBuffer, CL_TRUE, CL_MAP_WRITE, 0
                                                , numRandomNumberGenerators*4*sizeof(unsigned long), 0, NULL, NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueMapBuffer failed.",".");
      goto cleanup;
    }

    for (i = 0; i < numRandomNumberGenerators; i++) {
      RandomInfo* randomInfo = AcquireRandomInfo();
      const unsigned long* s = GetRandomInfoSeed(randomInfo);

      if (i == 0)
        fNormalize = GetRandomInfoNormalize(randomInfo);

      seeds[i*4] = s[0];
      randomInfo = DestroyRandomInfo(randomInfo);
    }

    clStatus = clEnqueueUnmapMemObject(queue, randomNumberSeedsBuffer, seeds, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueueUnmapMemObject failed.",".");
      goto cleanup;
    }

    randomNumberGeneratorKernel = AcquireOpenCLKernel(clEnv,MAGICK_OPENCL_ACCELERATE
                                                        ,"randomNumberGeneratorKernel");
    
    k = 0;
    clSetKernelArg(randomNumberGeneratorKernel,k++,sizeof(cl_mem),(void *)&randomNumberSeedsBuffer);
    clSetKernelArg(randomNumberGeneratorKernel,k++,sizeof(float),(void *)&fNormalize);
    clSetKernelArg(randomNumberGeneratorKernel,k++,sizeof(cl_mem),(void *)&randomNumberBuffer);
    initRandom = 1;
    clSetKernelArg(randomNumberGeneratorKernel,k++,sizeof(unsigned int),(void *)&initRandom);
    clSetKernelArg(randomNumberGeneratorKernel,k++,sizeof(unsigned int),(void *)&numRandomNumberPerBuffer);

    random_work_size = numRandomNumberGenerators;
  }

  addNoiseKernel = AcquireOpenCLKernel(clEnv,MAGICK_OPENCL_ACCELERATE,"AddNoiseImage");
  k = 0;
  clSetKernelArg(addNoiseKernel,k++,sizeof(cl_mem),(void *)&inputImageBuffer);
  clSetKernelArg(addNoiseKernel,k++,sizeof(cl_mem),(void *)&filteredImageBuffer);
  inputColumns = inputImage->columns;
  clSetKernelArg(addNoiseKernel,k++,sizeof(unsigned int),(void *)&inputColumns);
  inputRows = inputImage->rows;
  clSetKernelArg(addNoiseKernel,k++,sizeof(unsigned int),(void *)&inputRows);
  clSetKernelArg(addNoiseKernel,k++,sizeof(ChannelType),(void *)&channel);
  clSetKernelArg(addNoiseKernel,k++,sizeof(NoiseType),(void *)&noise_type);
  attenuate=1.0f;
  option=GetImageArtifact(inputImage,"attenuate");
  if (option != (char *) NULL)
    attenuate=(float)StringToDouble(option,(char **) NULL);
  clSetKernelArg(addNoiseKernel,k++,sizeof(float),(void *)&attenuate);
  clSetKernelArg(addNoiseKernel,k++,sizeof(cl_mem),(void *)&randomNumberBuffer);
  clSetKernelArg(addNoiseKernel,k++,sizeof(unsigned int),(void *)&numRandomNumberPerPixel);

  global_work_size[0] = inputColumns;
  for (r = 0; r < inputRows; r+=numRowsPerKernelLaunch) 
  {
    size_t generator_local_size = 64;
    /* Generate random numbers in the buffer */
    clEnqueueNDRangeKernel(queue,randomNumberGeneratorKernel,1,NULL
                            ,&random_work_size,&generator_local_size,0,NULL,NULL);
    if (initRandom != 0)
    {
      /* make sure we only do init once */
      initRandom = 0;
      clSetKernelArg(randomNumberGeneratorKernel,3,sizeof(unsigned int),(void *)&initRandom);
    }

    /* set the row offset */
    clSetKernelArg(addNoiseKernel,k,sizeof(unsigned int),(void *)&r);
    global_work_size[1] = MAGICK_MIN(numRowsPerKernelLaunch, inputRows - r);
    clEnqueueNDRangeKernel(queue,addNoiseKernel,2,NULL,global_work_size,NULL,0,NULL,NULL);
  }

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = inputImage->columns * inputImage->rows;
    clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = inputImage->columns * inputImage->rows;
    clStatus = clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (queue!=NULL)                  RelinquishOpenCLCommandQueue(clEnv, queue);
  if (addNoiseKernel!=NULL)         RelinquishOpenCLKernel(clEnv, addNoiseKernel);
  if (randomNumberGeneratorKernel!=NULL) RelinquishOpenCLKernel(clEnv, randomNumberGeneratorKernel);
  if (inputImageBuffer!=NULL)		    clReleaseMemObject(inputImageBuffer);
  if (randomNumberBuffer!=NULL)     clReleaseMemObject(randomNumberBuffer);
  if (filteredImageBuffer!=NULL)	  clReleaseMemObject(filteredImageBuffer);
  if (randomNumberSeedsBuffer!=NULL) clReleaseMemObject(randomNumberSeedsBuffer);
  if (outputReady == MagickFalse
      && filteredImage != NULL) 
  {
      DestroyImage(filteredImage);
      filteredImage = NULL;
  }
  return filteredImage;
}



MagickExport 
Image* AccelerateAddNoiseImage(const Image *image, const ChannelType channel,
          const NoiseType noise_type,ExceptionInfo *exception) 
{
  MagickBooleanType status;
  Image* filteredImage = NULL;

  assert(image != NULL);
  assert(exception != NULL);

  status = checkOpenCLEnvironment(exception);
  if (status == MagickFalse)
    return NULL;

  status = checkAccelerateCondition(image, channel);
  if (status == MagickFalse)
    return NULL;

DisableMSCWarning(4127)
  if (sizeof(unsigned long) == 4)
RestoreMSCWarning
    filteredImage = ComputeAddNoiseImageOptRandomNum(image,channel,noise_type,exception);
  else
    filteredImage = ComputeAddNoiseImage(image,channel,noise_type,exception);
  
  return filteredImage;
}


#else  /* MAGICKCORE_OPENCL_SUPPORT  */

MagickExport Image *AccelerateConvolveImageChannel(
  const Image *magick_unused(image),const ChannelType magick_unused(channel),
  const KernelInfo *magick_unused(kernel),
  ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(image);
  magick_unreferenced(channel);
  magick_unreferenced(kernel);
  magick_unreferenced(exception);

  return NULL;
}

MagickExport MagickBooleanType AccelerateFunctionImage(
  Image *magick_unused(image),const ChannelType magick_unused(channel),
  const MagickFunction magick_unused(function),
  const size_t magick_unused(number_parameters),
  const double *magick_unused(parameters),
  ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(image);
  magick_unreferenced(channel);
  magick_unreferenced(function);
  magick_unreferenced(number_parameters);
  magick_unreferenced(parameters);
  magick_unreferenced(exception);

  return MagickFalse;
}

MagickExport Image *AccelerateBlurImage(const Image *magick_unused(image),
  const ChannelType magick_unused(channel),const double magick_unused(radius),
  const double magick_unused(sigma),ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(image);
  magick_unreferenced(channel);
  magick_unreferenced(radius);
  magick_unreferenced(sigma);
  magick_unreferenced(exception);

  return NULL;
}

MagickExport Image *AccelerateRadialBlurImage(
  const Image *magick_unused(image),const ChannelType magick_unused(channel),
  const double magick_unused(angle),ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(image);
  magick_unreferenced(channel);
  magick_unreferenced(angle);
  magick_unreferenced(exception);

  return NULL;
}


MagickExport Image *AccelerateUnsharpMaskImage(
  const Image *magick_unused(image),const ChannelType magick_unused(channel),
  const double magick_unused(radius),const double magick_unused(sigma),
  const double magick_unused(gain),const double magick_unused(threshold),
  ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(image);
  magick_unreferenced(channel);
  magick_unreferenced(radius);
  magick_unreferenced(sigma);
  magick_unreferenced(gain);
  magick_unreferenced(threshold);
  magick_unreferenced(exception);

  return NULL;
}


MagickExport MagickBooleanType AccelerateContrastImage(
  Image* magick_unused(image),const MagickBooleanType magick_unused(sharpen),
  ExceptionInfo* magick_unused(exception))
{
  magick_unreferenced(image);
  magick_unreferenced(sharpen);
  magick_unreferenced(exception);

  return MagickFalse;
}

MagickExport MagickBooleanType AccelerateEqualizeImage(
  Image* magick_unused(image), const ChannelType magick_unused(channel),
  ExceptionInfo* magick_unused(exception))
{
  magick_unreferenced(image);
  magick_unreferenced(channel);
  magick_unreferenced(exception);

  return MagickFalse;
}

MagickExport Image *AccelerateDespeckleImage(const Image* magick_unused(image),
  ExceptionInfo* magick_unused(exception))
{
  magick_unreferenced(image);
  magick_unreferenced(exception);

  return NULL;
}

MagickExport Image *AccelerateResizeImage(const Image* magick_unused(image),
  const size_t magick_unused(resizedColumns),
  const size_t magick_unused(resizedRows),
  const ResizeFilter* magick_unused(resizeFilter),
  ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(image);
  magick_unreferenced(resizedColumns);
  magick_unreferenced(resizedRows);
  magick_unreferenced(resizeFilter);
  magick_unreferenced(exception);

  return NULL;
}


MagickExport
MagickBooleanType AccelerateModulateImage(
  Image* image, double percent_brightness, double percent_hue, 
  double percent_saturation, ColorspaceType colorspace, ExceptionInfo* exception)
{
  magick_unreferenced(image);
  magick_unreferenced(percent_brightness);
  magick_unreferenced(percent_hue);
  magick_unreferenced(percent_saturation);
  magick_unreferenced(colorspace);
  magick_unreferenced(exception);
  return(MagickFalse);
}

MagickExport Image *AccelerateAddNoiseImage(const Image *image, 
  const ChannelType channel, const NoiseType noise_type,ExceptionInfo *exception) 
{
  magick_unreferenced(image);
  magick_unreferenced(channel);
  magick_unreferenced(noise_type);
  magick_unreferenced(exception);
  return NULL;
}

#endif /* MAGICKCORE_OPENCL_SUPPORT */

MagickExport MagickBooleanType AccelerateConvolveImage(
  const Image *magick_unused(image),const KernelInfo *magick_unused(kernel),
  Image *magick_unused(convolve_image),ExceptionInfo *magick_unused(exception))
{
  magick_unreferenced(image);
  magick_unreferenced(kernel);
  magick_unreferenced(convolve_image);
  magick_unreferenced(exception);

  /* legacy, do not use */
  return(MagickFalse);
}


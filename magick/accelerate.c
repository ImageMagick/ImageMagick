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
%                              Guansong Zhang                                 %
%                               January 2010                                  %
%                               Dirk Lemstra                                  %
%                                 May 2016                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/accelerate-private.h"
#include "magick/accelerate-kernels-private.h"
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

/*
  Define declarations.
*/
#define ALIGNED(pointer,type) ((((size_t)(pointer)) & (sizeof(type)-1)) == 0)

/*
  Static declarations.
*/
static const ResizeWeightingFunctionType supportedResizeWeighting[] = 
{
  BoxWeightingFunction,
  TriangleWeightingFunction,
  HanningWeightingFunction,
  HammingWeightingFunction,
  BlackmanWeightingFunction,
  CubicBCWeightingFunction,
  SincWeightingFunction,
  SincFastWeightingFunction,
  LastWeightingFunction
};

/*
  Forward declarations.
*/
static Image *ComputeUnsharpMaskImageSingle(const Image *image,
  const double radius,const double sigma,const double gain,
  const double threshold,int blurOnly, ExceptionInfo *exception);

/*
  Helper functions.
*/

static MagickBooleanType checkAccelerateCondition(const Image* image,
  const ChannelType channel)
{
  /* check if the image's colorspace is supported */
  if (image->colorspace != RGBColorspace &&
      image->colorspace != sRGBColorspace &&
      image->colorspace != GRAYColorspace)
    return(MagickFalse);

  /* check if the channel is supported */
  if (((channel & RedChannel) == 0) ||
      ((channel & GreenChannel) == 0) ||
      ((channel & BlueChannel) == 0))
    return(MagickFalse);

  /* check if the virtual pixel method is compatible with the OpenCL implementation */
  if ((GetImageVirtualPixelMethod(image) != UndefinedVirtualPixelMethod) &&
      (GetImageVirtualPixelMethod(image) != EdgeVirtualPixelMethod))
    return(MagickFalse);

  /* check if the image has clip_mask / mask */
  if ((image->clip_mask != (Image *) NULL) || (image->mask != (Image *) NULL))
    return(MagickFalse);

  return(MagickTrue);
}

static MagickBooleanType checkHistogramCondition(Image *image,
  const ChannelType channel)
{
  /* ensure this is the only pass get in for now. */
  if ((channel & SyncChannels) == 0)
    return MagickFalse;

  if (image->intensity == Rec601LuminancePixelIntensityMethod ||
      image->intensity == Rec709LuminancePixelIntensityMethod)
    return MagickFalse;

  if (image->colorspace != sRGBColorspace)
    return MagickFalse;

  return MagickTrue;
}

static MagickBooleanType checkOpenCLEnvironment(ExceptionInfo* exception)
{
  MagickBooleanType
    flag;

  MagickCLEnv
    clEnv;

  clEnv=GetDefaultOpenCLEnv();

  GetMagickOpenCLEnvParam(clEnv,MAGICK_OPENCL_ENV_PARAM_OPENCL_DISABLED,
    sizeof(MagickBooleanType),&flag,exception);
  if (flag != MagickFalse)
    return(MagickFalse);

  GetMagickOpenCLEnvParam(clEnv,MAGICK_OPENCL_ENV_PARAM_OPENCL_INITIALIZED,
    sizeof(MagickBooleanType),&flag,exception);
  if (flag == MagickFalse)
    {
      if (InitOpenCLEnv(clEnv,exception) == MagickFalse)
        return(MagickFalse);

      GetMagickOpenCLEnvParam(clEnv,MAGICK_OPENCL_ENV_PARAM_OPENCL_DISABLED,
        sizeof(MagickBooleanType),&flag,exception);
      if (flag != MagickFalse)
        return(MagickFalse);
    }

  return(MagickTrue);
}

/* pad the global workgroup size to the next multiple of 
   the local workgroup size */
inline static unsigned int padGlobalWorkgroupSizeToLocalWorkgroupSize(
  const unsigned int orgGlobalSize,const unsigned int localGroupSize) 
{
  return ((orgGlobalSize+(localGroupSize-1))/localGroupSize*localGroupSize);
}

static MagickBooleanType paramMatchesValue(MagickCLEnv clEnv,
  MagickOpenCLEnvParam param,const char *value,ExceptionInfo *exception)
{
  char
    *val;

  MagickBooleanType
    status;

  status=GetMagickOpenCLEnvParam(clEnv,param,sizeof(val),&val,exception);
  if (status != MagickFalse)
    {
      status=strcmp(value,val) == 0 ? MagickTrue : MagickFalse;
      RelinquishMagickMemory(val);
    }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e A d d N o i s e I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static Image *ComputeAddNoiseImage(const Image *image,
  const ChannelType channel,const NoiseType noise_type,
  ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    inputPixelCount,
    pixelsPerWorkitem,
    clStatus;

  cl_uint
    event_count,
    seed0,
    seed1;

  cl_kernel
    addNoiseKernel;

  cl_event
    event;

  cl_mem
    filteredImageBuffer,
    imageBuffer;

  const char
    *option;

  const cl_event
    *events;

  float
    attenuate;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  Image
    *filteredImage;

  RandomInfo
    **magick_restrict random_info;

  size_t
    global_work_size[1],
    local_work_size[1];

  unsigned int
    k,
    numRandomNumberPerPixel;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  unsigned long
    key;
#endif

  outputReady = MagickFalse;
  clEnv = NULL;
  filteredImage = NULL;
  context = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  queue = NULL;
  addNoiseKernel = NULL;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  imageBuffer = GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  filteredImage = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredImageBuffer = GetAuthenticOpenCLBuffer(filteredImage,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
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

  /* set up the random number generators */
  attenuate=1.0;
  option=GetImageArtifact(image,"attenuate");
  if (option != (char *) NULL)
    attenuate=StringToDouble(option,(char **) NULL);
  random_info=AcquireRandomInfoThreadSet();
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  key=GetRandomSecretKey(random_info[0]);
  (void) key;
#endif

  addNoiseKernel = AcquireOpenCLKernel(clEnv,MAGICK_OPENCL_ACCELERATE,"AddNoise");

  {
    cl_uint computeUnitCount;
    cl_uint workItemCount;
    clEnv->library->clGetDeviceInfo(clEnv->device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &computeUnitCount, NULL);
    workItemCount = computeUnitCount * 2 * 256;			// 256 work items per group, 2 groups per CU
    inputPixelCount = (cl_int) (image->columns * image->rows);
    pixelsPerWorkitem = (inputPixelCount + workItemCount - 1) / workItemCount;
    pixelsPerWorkitem = ((pixelsPerWorkitem + 3) / 4) * 4;

    local_work_size[0] = 256;
    global_work_size[0] = workItemCount;
  }
  {
    RandomInfo* randomInfo = AcquireRandomInfo();
	const unsigned long* s = GetRandomInfoSeed(randomInfo);
	seed0 = s[0];
	GetPseudoRandomValue(randomInfo);
	seed1 = s[0];
	randomInfo = DestroyRandomInfo(randomInfo);
  }

  k = 0;
  clEnv->library->clSetKernelArg(addNoiseKernel,k++,sizeof(cl_mem),(void *)&imageBuffer);
  clEnv->library->clSetKernelArg(addNoiseKernel,k++,sizeof(cl_mem),(void *)&filteredImageBuffer);
  clEnv->library->clSetKernelArg(addNoiseKernel,k++,sizeof(cl_uint),(void *)&inputPixelCount);
  clEnv->library->clSetKernelArg(addNoiseKernel,k++,sizeof(cl_uint),(void *)&pixelsPerWorkitem);  
  clEnv->library->clSetKernelArg(addNoiseKernel,k++,sizeof(ChannelType),(void *)&channel);
  clEnv->library->clSetKernelArg(addNoiseKernel,k++,sizeof(NoiseType),(void *)&noise_type);
  attenuate=1.0f;
  option=GetImageArtifact(image,"attenuate");
  if (option != (char *) NULL)
    attenuate=(float)StringToDouble(option,(char **) NULL);
  clEnv->library->clSetKernelArg(addNoiseKernel,k++,sizeof(float),(void *)&attenuate);
  clEnv->library->clSetKernelArg(addNoiseKernel,k++,sizeof(cl_uint),(void *)&seed0);
  clEnv->library->clSetKernelArg(addNoiseKernel,k++,sizeof(cl_uint),(void *)&seed1);
  clEnv->library->clSetKernelArg(addNoiseKernel,k++,sizeof(unsigned int),(void *)&numRandomNumberPerPixel);

  events=GetOpenCLEvents(image,&event_count);
  clStatus=clEnv->library->clEnqueueNDRangeKernel(queue,addNoiseKernel,1,NULL,global_work_size,NULL,event_count,events,&event);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  if (RecordProfileData(clEnv,AddNoiseKernel,event) == MagickFalse)
    {
      AddOpenCLEvent(image,event);
      AddOpenCLEvent(filteredImage,event);
    }
  clEnv->library->clReleaseEvent(event);
  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (queue!=NULL)                  RelinquishOpenCLCommandQueue(clEnv, queue);
  if (addNoiseKernel!=NULL)         RelinquishOpenCLKernel(clEnv, addNoiseKernel);
  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);

  return(filteredImage);
}

MagickPrivate Image *AccelerateAddNoiseImage(const Image *image,
  const ChannelType channel,const NoiseType noise_type,
  ExceptionInfo *exception) 
{
  Image
    *filteredImage;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, channel) == MagickFalse))
    return NULL;

  filteredImage = ComputeAddNoiseImage(image,channel,noise_type,exception);
  
  return(filteredImage);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e B l u r I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static Image *ComputeBlurImage(const Image* image,const ChannelType channel,
  const double radius,const double sigma,ExceptionInfo *exception)
{
  char
    geometry[MaxTextExtent];

  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    clStatus;

  cl_kernel
    blurColumnKernel,
    blurRowKernel;

  cl_event
    event;

  cl_mem
    filteredImageBuffer,
    imageBuffer,
    imageKernelBuffer,
    tempImageBuffer;

  cl_uint
    event_count;

  const cl_event
    *events;

  float
    *kernelBufferPtr;

  Image
    *filteredImage;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  MagickSizeType
    length;

  KernelInfo
    *kernel;

  unsigned int
    i,
    imageColumns,
    imageRows,
    kernelWidth;

  context = NULL;
  filteredImage = NULL;
  imageBuffer = NULL;
  tempImageBuffer = NULL;
  filteredImageBuffer = NULL;
  imageKernelBuffer = NULL;
  blurRowKernel = NULL;
  blurColumnKernel = NULL;
  queue = NULL;
  kernel = NULL;

  outputReady = MagickFalse;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  imageBuffer = GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  filteredImage = CloneImage(image, image->columns, image->rows, MagickTrue, exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage, DirectClass) != MagickTrue)
  {
	  (void)OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
	  goto cleanup;
  }
  filteredImageBuffer = GetAuthenticOpenCLBuffer(filteredImage,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
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

	{
		kernelBufferPtr = (float *)AcquireMagickMemory(kernel->width * sizeof(float));
		for (i = 0; i < kernel->width; i++)
			kernelBufferPtr[i] = (float)kernel->values[i];

		imageKernelBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, kernel->width * sizeof(float), kernelBufferPtr, &clStatus);
		RelinquishMagickMemory(kernelBufferPtr);
		if (clStatus != CL_SUCCESS)
		{
			(void)OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.", ".");
			goto cleanup;
		}
	}
  }

  {

    /* create temp buffer */
    {
      length = image->columns * image->rows;
      tempImageBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_WRITE, length * 4 * sizeof(float), NULL, &clStatus);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
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
        imageColumns = (unsigned int) image->columns;
        imageRows = (unsigned int) image->rows;

        /* set the kernel arguments */
        i = 0;
        clStatus=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
        clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
        clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(ChannelType),&channel);
        clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
        kernelWidth = (unsigned int) kernel->width;
        clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
        clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
        clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageRows);
        clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(CLPixelPacket)*(chunkSize+kernel->width),(void *) NULL);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
          goto cleanup;
        }
      }

      /* launch the kernel */
      {
        size_t gsize[2];
        size_t wsize[2];

        gsize[0] = chunkSize*((image->columns+chunkSize-1)/chunkSize);
        gsize[1] = image->rows;
        wsize[0] = chunkSize;
        wsize[1] = 1;

        events=GetOpenCLEvents(image,&event_count);
        clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, blurRowKernel, 2, NULL, gsize, wsize, event_count, events, &event);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
          goto cleanup;
        }
        if (RecordProfileData(clEnv,BlurRowKernel,event) == MagickFalse)
          {
            AddOpenCLEvent(image,event);
            AddOpenCLEvent(filteredImage,event);
          }
        clEnv->library->clReleaseEvent(event);
      }
    }

    {
      /* need logic to decide this value */
      int chunkSize = 256;

      {
        imageColumns = (unsigned int) image->columns;
        imageRows = (unsigned int) image->rows;

        /* set the kernel arguments */
        i = 0;
        clStatus=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
        clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
        clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(ChannelType),&channel);
        clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
        kernelWidth = (unsigned int) kernel->width;
        clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
        clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
        clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&imageRows);
        clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(cl_float4)*(chunkSize+kernel->width),(void *) NULL);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
          goto cleanup;
        }
      }

      /* launch the kernel */
      {
        size_t gsize[2];
        size_t wsize[2];

        gsize[0] = image->columns;
        gsize[1] = chunkSize*((image->rows+chunkSize-1)/chunkSize);
        wsize[0] = 1;
        wsize[1] = chunkSize;

        events=GetOpenCLEvents(image,&event_count);
        clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, blurColumnKernel, 2, NULL, gsize, wsize, event_count, events, &event);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
          goto cleanup;
        }
        if (RecordProfileData(clEnv,BlurColumnKernel,event) == MagickFalse)
          {
            AddOpenCLEvent(image,event);
            AddOpenCLEvent(filteredImage,event);
          }
        clEnv->library->clReleaseEvent(event);
      }
    }

  }

  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (tempImageBuffer!=NULL)      clEnv->library->clReleaseMemObject(tempImageBuffer);
  if (imageKernelBuffer!=NULL)    clEnv->library->clReleaseMemObject(imageKernelBuffer);
  if (blurRowKernel!=NULL)        RelinquishOpenCLKernel(clEnv, blurRowKernel);
  if (blurColumnKernel!=NULL)     RelinquishOpenCLKernel(clEnv, blurColumnKernel);
  if (queue != NULL)              RelinquishOpenCLCommandQueue(clEnv, queue);
  if (kernel!=NULL)               DestroyKernelInfo(kernel);
  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);
  return(filteredImage);
}

MagickPrivate Image* AccelerateBlurImage(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  ExceptionInfo *exception)
{
  Image
    *filteredImage;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, channel) == MagickFalse))
    return NULL;

  filteredImage=ComputeBlurImage(image, channel, radius, sigma, exception);
  return(filteredImage);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e C o m p o s i t e I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static MagickBooleanType LaunchCompositeKernel(const Image *image,
  MagickCLEnv clEnv,cl_command_queue queue,cl_mem imageBuffer,
  const unsigned int inputWidth,const unsigned int inputHeight,
  const unsigned int inputMatte,const ChannelType channel,
  const CompositeOperator compose,const cl_mem compositeImageBuffer,
  const unsigned int compositeWidth,const unsigned int compositeHeight,
  const unsigned int compositeMatte,const float destination_dissolve,
  const float source_dissolve)
{
  cl_int
    clStatus;

  cl_kernel
    compositeKernel;

  cl_event
    event;

  cl_uint
    event_count;

  const cl_event
    *events;

  int
    k;

  size_t
    global_work_size[2],
    local_work_size[2];

  unsigned int
    composeOp;

  compositeKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE,
    "Composite");

  k = 0;
  clStatus = clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(cl_mem), (void*)&imageBuffer);
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(unsigned int), (void*)&inputWidth);
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(unsigned int), (void*)&inputHeight);
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(unsigned int), (void*)&inputMatte);
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(cl_mem), (void*)&compositeImageBuffer);
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(unsigned int), (void*)&compositeWidth);
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(unsigned int), (void*)&compositeHeight);
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(unsigned int), (void*)&compositeMatte);
  composeOp = (unsigned int)compose;
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(unsigned int), (void*)&composeOp);
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(ChannelType), (void*)&channel);
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(float), (void*)&destination_dissolve);
  clStatus |= clEnv->library->clSetKernelArg(compositeKernel, k++, sizeof(float), (void*)&source_dissolve);

  if (clStatus != CL_SUCCESS)
    return MagickFalse;

  local_work_size[0] = 64;
  local_work_size[1] = 1;

  global_work_size[0] = padGlobalWorkgroupSizeToLocalWorkgroupSize(inputWidth,
    (unsigned int)local_work_size[0]);
  global_work_size[1] = inputHeight;
  events=GetOpenCLEvents(image,&event_count);
  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, compositeKernel, 2, NULL,
    global_work_size, local_work_size, event_count, events, &event);
  if (clStatus == CL_SUCCESS)
    AddOpenCLEvent(image,event);
  clEnv->library->clReleaseEvent(event);

  RelinquishOpenCLKernel(clEnv, compositeKernel);

  return((clStatus == CL_SUCCESS) ? MagickTrue : MagickFalse);
}

static MagickBooleanType ComputeCompositeImage(Image *image,
  const ChannelType channel, const CompositeOperator compose,
  const Image *compositeImage, const ssize_t magick_unused(x_offset),
  const ssize_t magick_unused(y_offset), const float destination_dissolve,
  const float source_dissolve, ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_context
    context;

  cl_mem
    compositeImageBuffer,
    imageBuffer;

  MagickBooleanType
    outputReady,
    status;

  MagickCLEnv
    clEnv;

  magick_unreferenced(x_offset);
  magick_unreferenced(y_offset);

  status = MagickFalse;
  outputReady = MagickFalse;
  imageBuffer = NULL;
  compositeImageBuffer = NULL;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  imageBuffer = GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  compositeImageBuffer = GetAuthenticOpenCLBuffer(compositeImage,exception);
  if (compositeImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  status = LaunchCompositeKernel(image,clEnv, queue, imageBuffer,
    (unsigned int)image->columns,
    (unsigned int)image->rows,
    (unsigned int)image->matte,
    channel, compose, compositeImageBuffer,
    (unsigned int)compositeImage->columns,
    (unsigned int)compositeImage->rows,
    (unsigned int)compositeImage->matte,
    destination_dissolve, source_dissolve);

  if (status == MagickFalse)
    goto cleanup;

  outputReady = MagickTrue;

cleanup:

  if (queue != NULL)
    RelinquishOpenCLCommandQueue(clEnv, queue);

  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateCompositeImage(Image *image,
  const ChannelType channel, const CompositeOperator compose,
  const Image *composite, const ssize_t x_offset, const ssize_t y_offset,
  const float destination_dissolve, const float source_dissolve,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *)NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
    (checkAccelerateCondition(image, channel) == MagickFalse))
    return(MagickFalse);

  /* only support zero offset and
  images with the size for now */
  if (x_offset != 0
    || y_offset != 0
    || image->columns != composite->columns
    || image->rows != composite->rows)
    return MagickFalse;

  switch (compose) {
  case ColorDodgeCompositeOp:
  case BlendCompositeOp:
    break;
  default:
    // unsupported compose operator, quit
    return MagickFalse;
  };

  status = ComputeCompositeImage(image, channel, compose, composite,
    x_offset, y_offset, destination_dissolve, source_dissolve, exception);

  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e C o n t r a s t I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static MagickBooleanType ComputeContrastImage(Image *image,
  const MagickBooleanType sharpen,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    clStatus;

  cl_kernel
    filterKernel;

  cl_event
    event;

  cl_mem
    imageBuffer;

  cl_uint
    event_count;

  const cl_event
    *events;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  size_t
    global_work_size[2];

  unsigned int
    i,
    uSharpen;

  outputReady = MagickFalse;
  clEnv = NULL;
  context = NULL;
  imageBuffer = NULL;
  filterKernel = NULL;
  queue = NULL;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);

  /* Create and initialize OpenCL buffers. */
  imageBuffer=GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }
  
  filterKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "Contrast");
  if (filterKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  i = 0;
  clStatus=clEnv->library->clSetKernelArg(filterKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);

  uSharpen = (sharpen == MagickFalse)?0:1;
  clStatus|=clEnv->library->clSetKernelArg(filterKernel,i++,sizeof(cl_uint),&uSharpen);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  global_work_size[0] = image->columns;
  global_work_size[1] = image->rows;
  /* launch the kernel */
  queue = AcquireOpenCLCommandQueue(clEnv);
  events=GetOpenCLEvents(image,&event_count);
  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, filterKernel, 2, NULL, global_work_size, NULL, event_count, events, &event);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  if (RecordProfileData(clEnv,ContrastKernel,event) == MagickFalse)
    AddOpenCLEvent(image,event);
  clEnv->library->clReleaseEvent(event);
  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (filterKernel!=NULL)                     RelinquishOpenCLKernel(clEnv, filterKernel);
  if (queue != NULL)                          RelinquishOpenCLCommandQueue(clEnv, queue);
  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateContrastImage(Image *image,
  const MagickBooleanType sharpen,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, AllChannels) == MagickFalse))
    return(MagickFalse);

  status = ComputeContrastImage(image,sharpen,exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e C o n t r a s t S t r e t c h I m a g e             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static MagickBooleanType LaunchHistogramKernel(MagickCLEnv clEnv,
  cl_command_queue queue,cl_mem imageBuffer,cl_mem histogramBuffer,
  Image *image,const ChannelType channel,ExceptionInfo *exception)
{
  MagickBooleanType
    outputReady;

  cl_event
    event;

  cl_int
    clStatus,
    colorspace,
    method;

  cl_kernel
    histogramKernel;

  cl_uint
    event_count;

  const cl_event
    *events;

  register ssize_t
    i;

  size_t
    global_work_size[2];

  histogramKernel = NULL; 

  outputReady = MagickFalse;
  method = image->intensity;
  colorspace = image->colorspace;

  /* get the OpenCL kernel */
  histogramKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "Histogram");
  if (histogramKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  /* set the kernel arguments */
  i = 0;
  clStatus=clEnv->library->clSetKernelArg(histogramKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(histogramKernel,i++,sizeof(ChannelType),&channel);
  clStatus|=clEnv->library->clSetKernelArg(histogramKernel,i++,sizeof(cl_int),&method);
  clStatus|=clEnv->library->clSetKernelArg(histogramKernel,i++,sizeof(cl_int),&colorspace);
  clStatus|=clEnv->library->clSetKernelArg(histogramKernel,i++,sizeof(cl_mem),(void *)&histogramBuffer);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  /* launch the kernel */
  global_work_size[0] = image->columns;
  global_work_size[1] = image->rows;

  events=GetOpenCLEvents(image,&event_count);
  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, histogramKernel, 2, NULL, global_work_size, NULL, event_count, events, &event);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  if (RecordProfileData(clEnv,HistogramKernel,event) == MagickFalse)
    AddOpenCLEvent(image,event);
  clEnv->library->clReleaseEvent(event);

  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);
 
  if (histogramKernel!=NULL)                     
    RelinquishOpenCLKernel(clEnv, histogramKernel);

  return(outputReady);
}

MagickPrivate MagickBooleanType ComputeContrastStretchImageChannel(Image *image,
  const ChannelType channel,const double black_point,const double white_point, 
  ExceptionInfo *exception) 
{
#define ContrastStretchImageTag  "ContrastStretch/Image"
#define MaxRange(color)  ((MagickRealType) ScaleQuantumToMap((Quantum) (color)))
  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    clStatus;

  cl_mem
    histogramBuffer,
    imageBuffer,
    stretchMapBuffer;

  cl_kernel
    histogramKernel,
    stretchKernel;

  cl_event
    event;

  cl_uint
    event_count;

  cl_uint4
    *histogram;

  const cl_event
    *events;

  double
    intensity;

  cl_float4
    black,
    white;

  MagickBooleanType
    outputReady,
    status;

  MagickCLEnv
    clEnv;

  MagickSizeType
    length;

  PixelPacket
    *stretch_map;

  register ssize_t
    i;

  size_t
    global_work_size[2];

  histogram=NULL;
  stretch_map=NULL;
  imageBuffer = NULL;
  histogramBuffer = NULL;
  stretchMapBuffer = NULL;
  histogramKernel = NULL; 
  stretchKernel = NULL; 
  context = NULL;
  queue = NULL;
  outputReady = MagickFalse;


  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  //exception=(&image->exception);

  /*
   * initialize opencl env
   */
  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  /*
    Allocate and initialize histogram arrays.
  */
  length = (MaxMap+1); 
  histogram=(cl_uint4 *) AcquireQuantumMemory(length, sizeof(*histogram));

  if (histogram == (cl_uint4 *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed", image->filename);
 
  /* reset histogram */
  (void) ResetMagickMemory(histogram,0,length*sizeof(*histogram));

  /*
  if (SetImageGray(image,exception) != MagickFalse)
    (void) SetImageColorspace(image,GRAYColorspace);
  */

  status=MagickTrue;

  imageBuffer=GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  /* create a CL buffer for histogram  */
  histogramBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, length * sizeof(cl_uint4), histogram, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  status = LaunchHistogramKernel(clEnv, queue, imageBuffer, histogramBuffer, image, channel, exception);
  if (status == MagickFalse)
    goto cleanup;

  /* this blocks, should be fixed it in the future */
  events=GetOpenCLEvents(image,&event_count);
  clEnv->library->clEnqueueMapBuffer(queue, histogramBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(cl_uint4), event_count, events, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  /* unmap, don't block gpu to use this buffer again.  */
  clStatus = clEnv->library->clEnqueueUnmapMemObject(queue, histogramBuffer, histogram, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueUnmapMemObject failed.", "'%s'", ".");
    goto cleanup;
  }

  /* CPU stuff */
  /*
     Find the histogram boundaries by locating the black/white levels.
  */
  black.z=0.0;
  white.z=MaxRange(QuantumRange);
  if ((channel & RedChannel) != 0)
  {
    intensity=0.0;
    for (i=0; i <= (ssize_t) MaxMap; i++)
    {
      intensity+=histogram[i].s[2];
      if (intensity > black_point)
        break;
    }
    black.z=(MagickRealType) i;
    intensity=0.0;
    for (i=(ssize_t) MaxMap; i != 0; i--)
    {
      intensity+=histogram[i].s[2];
      if (intensity > ((double) image->columns*image->rows-white_point))
        break;
    }
    white.z=(MagickRealType) i;
  }
  black.y=0.0;
  white.y=MaxRange(QuantumRange);
  if ((channel & GreenChannel) != 0)
  {
    intensity=0.0;
    for (i=0; i <= (ssize_t) MaxMap; i++)
    {
      intensity+=histogram[i].s[2];
      if (intensity > black_point)
        break;
    }
    black.y=(MagickRealType) i;
    intensity=0.0;
    for (i=(ssize_t) MaxMap; i != 0; i--)
    {
      intensity+=histogram[i].s[2];
      if (intensity > ((double) image->columns*image->rows-white_point))
        break;
    }
    white.y=(MagickRealType) i;
  }
  black.x=0.0;
  white.x=MaxRange(QuantumRange);
  if ((channel & BlueChannel) != 0)
  {
    intensity=0.0;
    for (i=0; i <= (ssize_t) MaxMap; i++)
    {
      intensity+=histogram[i].s[2];
      if (intensity > black_point)
        break;
    }
    black.x=(MagickRealType) i;
    intensity=0.0;
    for (i=(ssize_t) MaxMap; i != 0; i--)
    {
      intensity+=histogram[i].s[2];
      if (intensity > ((double) image->columns*image->rows-white_point))
        break;
    }
    white.x=(MagickRealType) i;
  }
  black.w=0.0;
  white.w=MaxRange(QuantumRange);
  if ((channel & OpacityChannel) != 0)
  {
    intensity=0.0;
    for (i=0; i <= (ssize_t) MaxMap; i++)
    {
      intensity+=histogram[i].s[2];
      if (intensity > black_point)
        break;
    }
    black.w=(MagickRealType) i;
    intensity=0.0;
    for (i=(ssize_t) MaxMap; i != 0; i--)
    {
      intensity+=histogram[i].s[2];
      if (intensity > ((double) image->columns*image->rows-white_point))
        break;
    }
    white.w=(MagickRealType) i;
  }
  /*
  black.index=0.0;
  white.index=MaxRange(QuantumRange);
  if (((channel & IndexChannel) != 0) && (image->colorspace == CMYKColorspace))
  {
    intensity=0.0;
    for (i=0; i <= (ssize_t) MaxMap; i++)
    {
      intensity+=histogram[i].index;
      if (intensity > black_point)
        break;
    }
    black.index=(MagickRealType) i;
    intensity=0.0;
    for (i=(ssize_t) MaxMap; i != 0; i--)
    {
      intensity+=histogram[i].index;
      if (intensity > ((double) image->columns*image->rows-white_point))
        break;
    }
    white.index=(MagickRealType) i;
  }
  */


  stretch_map=(PixelPacket *) AcquireQuantumMemory(length,
    sizeof(*stretch_map));

  if (stretch_map == (PixelPacket *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
 
  /*
    Stretch the histogram to create the stretched image mapping.
  */
  (void) ResetMagickMemory(stretch_map,0,length*sizeof(*stretch_map));
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if ((channel & RedChannel) != 0)
    {
      if (i < (ssize_t) black.z)
        stretch_map[i].red=(Quantum) 0;
      else
        if (i > (ssize_t) white.z)
          stretch_map[i].red=QuantumRange;
        else
          if (black.z != white.z)
            stretch_map[i].red=ScaleMapToQuantum((MagickRealType) (MaxMap*
                  (i-black.z)/(white.z-black.z)));
    }
    if ((channel & GreenChannel) != 0)
    {
      if (i < (ssize_t) black.y)
        stretch_map[i].green=0;
      else
        if (i > (ssize_t) white.y)
          stretch_map[i].green=QuantumRange;
        else
          if (black.y != white.y)
            stretch_map[i].green=ScaleMapToQuantum((MagickRealType) (MaxMap*
                  (i-black.y)/(white.y-black.y)));
    }
    if ((channel & BlueChannel) != 0)
    {
      if (i < (ssize_t) black.x)
        stretch_map[i].blue=0;
      else
        if (i > (ssize_t) white.x)
          stretch_map[i].blue= QuantumRange;
        else
          if (black.x != white.x)
            stretch_map[i].blue=ScaleMapToQuantum((MagickRealType) (MaxMap*
                  (i-black.x)/(white.x-black.x)));
    }
    if ((channel & OpacityChannel) != 0)
    {
      if (i < (ssize_t) black.w)
        stretch_map[i].opacity=0;
      else
        if (i > (ssize_t) white.w)
          stretch_map[i].opacity=QuantumRange;
        else
          if (black.w != white.w)
            stretch_map[i].opacity=ScaleMapToQuantum((MagickRealType) (MaxMap*
                  (i-black.w)/(white.w-black.w)));
    }
    /*
    if (((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace))
    {
      if (i < (ssize_t) black.index)
        stretch_map[i].index=0;
      else
        if (i > (ssize_t) white.index)
          stretch_map[i].index=QuantumRange;
        else
          if (black.index != white.index)
            stretch_map[i].index=ScaleMapToQuantum((MagickRealType) (MaxMap*
                  (i-black.index)/(white.index-black.index)));
    }
    */
  }

  /*
    Stretch the image.
  */
  if (((channel & OpacityChannel) != 0) || (((channel & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace)))
    image->storage_class=DirectClass;
  if (image->storage_class == PseudoClass)
  {
    /*
       Stretch colormap.
       */
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      if ((channel & RedChannel) != 0)
      {
        if (black.z != white.z)
          image->colormap[i].red=stretch_map[
            ScaleQuantumToMap(image->colormap[i].red)].red;
      }
      if ((channel & GreenChannel) != 0)
      {
        if (black.y != white.y)
          image->colormap[i].green=stretch_map[
            ScaleQuantumToMap(image->colormap[i].green)].green;
      }
      if ((channel & BlueChannel) != 0)
      {
        if (black.x != white.x)
          image->colormap[i].blue=stretch_map[
            ScaleQuantumToMap(image->colormap[i].blue)].blue;
      }
      if ((channel & OpacityChannel) != 0)
      {
        if (black.w != white.w)
          image->colormap[i].opacity=stretch_map[
            ScaleQuantumToMap(image->colormap[i].opacity)].opacity;
      }
    }
  }


  /* create a CL buffer for stretch_map  */
  stretchMapBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, length, stretch_map, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  /* get the OpenCL kernel */
  stretchKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "ContrastStretch");
  if (stretchKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  /* set the kernel arguments */
  i = 0;
  clStatus=clEnv->library->clSetKernelArg(stretchKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(stretchKernel,i++,sizeof(ChannelType),&channel);
  clStatus|=clEnv->library->clSetKernelArg(stretchKernel,i++,sizeof(cl_mem),(void *)&stretchMapBuffer);
  clStatus|=clEnv->library->clSetKernelArg(stretchKernel,i++,sizeof(cl_float4),&white);
  clStatus|=clEnv->library->clSetKernelArg(stretchKernel,i++,sizeof(cl_float4),&black);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  /* launch the kernel */
  global_work_size[0] = image->columns;
  global_work_size[1] = image->rows;

  events=GetOpenCLEvents(image,&event_count);
  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, stretchKernel, 2, NULL, global_work_size, NULL, event_count, events, &event);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  if (RecordProfileData(clEnv,ContrastStretchKernel,event) == MagickFalse)
    AddOpenCLEvent(image, event);
  clEnv->library->clReleaseEvent(event);

  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (stretchMapBuffer!=NULL)
    clEnv->library->clReleaseMemObject(stretchMapBuffer);
  if (stretch_map!=NULL)
    stretch_map=(PixelPacket *) RelinquishMagickMemory(stretch_map);


  if (histogramBuffer!=NULL)
    clEnv->library->clReleaseMemObject(histogramBuffer);
  if (histogram!=NULL)
    histogram=(cl_uint4 *) RelinquishMagickMemory(histogram);


  if (histogramKernel!=NULL)                     
    RelinquishOpenCLKernel(clEnv, histogramKernel);
  if (stretchKernel!=NULL)                     
    RelinquishOpenCLKernel(clEnv, stretchKernel);

  if (queue != NULL)                          
    RelinquishOpenCLCommandQueue(clEnv, queue);

  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateContrastStretchImageChannel(
  Image *image,const ChannelType channel,const double black_point,
  const double white_point,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, channel) == MagickFalse) ||
      (checkHistogramCondition(image, channel) == MagickFalse))
    return(MagickFalse);

  status=ComputeContrastStretchImageChannel(image,channel, black_point, white_point, exception);
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
*/

static Image *ComputeConvolveImage(const Image* image,
  const ChannelType channel,const KernelInfo *kernel,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_context
    context;

  cl_kernel
    clkernel;

  cl_event
    event;

  cl_int
    clStatus;

  cl_mem
    convolutionKernel,
    filteredImageBuffer,
    imageBuffer;

  cl_uint
    event_count;

  cl_ulong
    deviceLocalMemorySize;

  const cl_event
    *events;

  float
    *kernelBufferPtr;

  Image
    *filteredImage;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  size_t
    global_work_size[3],
    localGroupSize[3],
    localMemoryRequirement;

  unsigned
    kernelSize;

  unsigned int
    filterHeight,
    filterWidth,
    i,
    imageHeight,
    imageWidth,
    matte;

  /* intialize all CL objects to NULL */
  context = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  convolutionKernel = NULL;
  clkernel = NULL;
  queue = NULL;

  filteredImage = NULL;
  outputReady = MagickFalse;
  
  clEnv = GetDefaultOpenCLEnv();

  /* Work around an issue on NVIDIA devices */
  if (paramMatchesValue(clEnv,MAGICK_OPENCL_ENV_PARAM_PLATFORM_VENDOR,
      "NVIDIA Corporation",exception) != MagickFalse)
    goto cleanup;

  context = GetOpenCLContext(clEnv);

  imageBuffer = GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  filteredImage = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredImageBuffer=GetAuthenticOpenCLBuffer(filteredImage,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  kernelSize = (unsigned int) (kernel->width * kernel->height);
  convolutionKernel = clEnv->library->clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, kernelSize * sizeof(float), NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  queue = AcquireOpenCLCommandQueue(clEnv);

  kernelBufferPtr = (float*)clEnv->library->clEnqueueMapBuffer(queue, convolutionKernel, CL_TRUE, CL_MAP_WRITE, 0, kernelSize * sizeof(float)
          , 0, NULL, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueMapBuffer failed.",".");
    goto cleanup;
  }
  for (i = 0; i < kernelSize; i++)
  {
    kernelBufferPtr[i] = (float) kernel->values[i];
  }
  clStatus = clEnv->library->clEnqueueUnmapMemObject(queue, convolutionKernel, kernelBufferPtr, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueUnmapMemObject failed.", "'%s'", ".");
    goto cleanup;
  }

  deviceLocalMemorySize = GetOpenCLDeviceLocalMemorySize(clEnv);

  /* Compute the local memory requirement for a 16x16 workgroup.
     If it's larger than 16k, reduce the workgroup size to 8x8 */
  localGroupSize[0] = 16;
  localGroupSize[1] = 16;
  localMemoryRequirement = (localGroupSize[0]+kernel->width-1) * (localGroupSize[1]+kernel->height-1) * sizeof(CLPixelPacket)
    + kernel->width*kernel->height*sizeof(float);

  if (localMemoryRequirement > deviceLocalMemorySize)
  {
    localGroupSize[0] = 8;
    localGroupSize[1] = 8;
    localMemoryRequirement = (localGroupSize[0]+kernel->width-1) * (localGroupSize[1]+kernel->height-1) * sizeof(CLPixelPacket)
      + kernel->width*kernel->height*sizeof(float);
  }
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
    clStatus =clEnv->library->clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
    imageWidth = (unsigned int) image->columns;
    imageHeight = (unsigned int) image->rows;
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&imageWidth);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&imageHeight);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&convolutionKernel);
    filterWidth = (unsigned int) kernel->width;
    filterHeight = (unsigned int) kernel->height;
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&filterWidth);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&filterHeight);
    matte = (image->matte==MagickTrue)?1:0;
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&matte);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(ChannelType),(void *)&channel);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++, (localGroupSize[0] + kernel->width-1)*(localGroupSize[1] + kernel->height-1)*sizeof(CLPixelPacket),NULL);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++, kernel->width*kernel->height*sizeof(float),NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }

    /* pad the global size to a multiple of the local work size dimension */
    global_work_size[0] = ((image->columns + localGroupSize[0]  - 1)/localGroupSize[0] ) * localGroupSize[0] ;
    global_work_size[1] = ((image->rows + localGroupSize[1] - 1)/localGroupSize[1]) * localGroupSize[1];

    /* launch the kernel */
    events = GetOpenCLEvents(image, &event_count);
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, clkernel, 2, NULL, global_work_size, localGroupSize, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    if (RecordProfileData(clEnv,ConvolveKernel,event) == MagickFalse)
      {
        AddOpenCLEvent(image, event);
        AddOpenCLEvent(filteredImage, event);
      }
    clEnv->library->clReleaseEvent(event);
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
    clStatus =clEnv->library->clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
    imageWidth = (unsigned int) image->columns;
    imageHeight = (unsigned int) image->rows;
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&imageWidth);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&imageHeight);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&convolutionKernel);
    filterWidth = (unsigned int) kernel->width;
    filterHeight = (unsigned int) kernel->height;
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&filterWidth);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&filterHeight);
    matte = (image->matte==MagickTrue)?1:0;
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&matte);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(ChannelType),(void *)&channel);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }

    localGroupSize[0] = 8;
    localGroupSize[1] = 8;
    global_work_size[0] = (image->columns + (localGroupSize[0]-1))/localGroupSize[0] * localGroupSize[0];
    global_work_size[1] = (image->rows    + (localGroupSize[1]-1))/localGroupSize[1] * localGroupSize[1];
    events=GetOpenCLEvents(image,&event_count);
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, clkernel, 2, NULL, global_work_size, localGroupSize, event_count, events, &event);
    
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    if (RecordProfileData(clEnv,ConvolveKernel,event) == MagickFalse)
      {
        AddOpenCLEvent(image,event);
        AddOpenCLEvent(filteredImage,event);
      }
    clEnv->library->clReleaseEvent(event);
  }

  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (convolutionKernel != NULL)
    clEnv->library->clReleaseMemObject(convolutionKernel);

  if (clkernel != NULL)
    RelinquishOpenCLKernel(clEnv, clkernel);

  if (queue != NULL)
    RelinquishOpenCLCommandQueue(clEnv, queue);

  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);

  return(filteredImage);
}

MagickPrivate Image *AccelerateConvolveImageChannel(const Image *image,
  const ChannelType channel,const KernelInfo *kernel,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  assert(image != NULL);
  assert(kernel != (KernelInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, channel) == MagickFalse))
    return NULL;

  filteredImage=ComputeConvolveImage(image, channel, kernel, exception);
  return(filteredImage);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e D e s p e c k l e I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static Image *ComputeDespeckleImage(const Image *image,
  ExceptionInfo*exception)
{
  static const int 
    X[4] = {0, 1, 1,-1},
    Y[4] = {1, 0, 1, 1};

  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    clStatus;

  cl_kernel
    hullPass1,
    hullPass2;

  cl_event
    event;

  cl_mem
    filteredImageBuffer,
    imageBuffer,
    tempImageBuffer[2];

  cl_uint
    event_count;

  const cl_event
    *events;

  Image
    *filteredImage;

  int
    k,
    matte;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  size_t
    global_work_size[2];

  unsigned int
    imageHeight,
    imageWidth;

  outputReady = MagickFalse;
  clEnv = NULL;
  filteredImage = NULL;
  context = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  hullPass1 = NULL;
  hullPass2 = NULL;
  queue = NULL;
  tempImageBuffer[0] = tempImageBuffer[1] = NULL;
  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  imageBuffer = GetAuthenticOpenCLBuffer(image, exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  filteredImage = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredImageBuffer = GetAuthenticOpenCLBuffer(filteredImage, exception);
  if (filteredImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  hullPass1 = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "HullPass1");
  hullPass2 = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "HullPass2");

  clStatus =clEnv->library->clSetKernelArg(hullPass1,0,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus |=clEnv->library->clSetKernelArg(hullPass1,1,sizeof(cl_mem),(void *)(tempImageBuffer+1));
  imageWidth = (unsigned int) image->columns;
  clStatus |=clEnv->library->clSetKernelArg(hullPass1,2,sizeof(unsigned int),(void *)&imageWidth);
  imageHeight = (unsigned int) image->rows;
  clStatus |=clEnv->library->clSetKernelArg(hullPass1,3,sizeof(unsigned int),(void *)&imageHeight);
  matte = (image->matte==MagickFalse)?0:1;
  clStatus |=clEnv->library->clSetKernelArg(hullPass1,6,sizeof(int),(void *)&matte);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  clStatus = clEnv->library->clSetKernelArg(hullPass2,0,sizeof(cl_mem),(void *)(tempImageBuffer+1));
  clStatus |=clEnv->library->clSetKernelArg(hullPass2,1,sizeof(cl_mem),(void *)tempImageBuffer);
  imageWidth = (unsigned int) image->columns;
  clStatus |=clEnv->library->clSetKernelArg(hullPass2,2,sizeof(unsigned int),(void *)&imageWidth);
  imageHeight = (unsigned int) image->rows;
  clStatus |=clEnv->library->clSetKernelArg(hullPass2,3,sizeof(unsigned int),(void *)&imageHeight);
  matte = (image->matte==MagickFalse)?0:1;
  clStatus |=clEnv->library->clSetKernelArg(hullPass2,6,sizeof(int),(void *)&matte);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }


  global_work_size[0] = image->columns;
  global_work_size[1] = image->rows;

  events=GetOpenCLEvents(image,&event_count);
  for (k = 0; k < 4; k++)
  {
    cl_int2 offset;
    int polarity;

    
    offset.s[0] = X[k];
    offset.s[1] = Y[k];
    polarity = 1;
    clStatus = clEnv->library->clSetKernelArg(hullPass1,4,sizeof(cl_int2),(void *)&offset);
    clStatus|= clEnv->library->clSetKernelArg(hullPass1,5,sizeof(int),(void *)&polarity);
    clStatus|=clEnv->library->clSetKernelArg(hullPass2,4,sizeof(cl_int2),(void *)&offset);
    clStatus|=clEnv->library->clSetKernelArg(hullPass2,5,sizeof(int),(void *)&polarity);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }
    /* launch the kernel */
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    RecordProfileData(clEnv,HullPass1Kernel,event);
    clEnv->library->clReleaseEvent(event);
    /* launch the kernel */
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    RecordProfileData(clEnv,HullPass2Kernel,event);
    clEnv->library->clReleaseEvent(event);


    if (k == 0)
      clStatus =clEnv->library->clSetKernelArg(hullPass1,0,sizeof(cl_mem),(void *)(tempImageBuffer));
    offset.s[0] = -X[k];
    offset.s[1] = -Y[k];
    polarity = 1;
    clStatus = clEnv->library->clSetKernelArg(hullPass1,4,sizeof(cl_int2),(void *)&offset);
    clStatus|= clEnv->library->clSetKernelArg(hullPass1,5,sizeof(int),(void *)&polarity);
    clStatus|=clEnv->library->clSetKernelArg(hullPass2,4,sizeof(cl_int2),(void *)&offset);
    clStatus|=clEnv->library->clSetKernelArg(hullPass2,5,sizeof(int),(void *)&polarity);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }
    /* launch the kernel */
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    RecordProfileData(clEnv,HullPass1Kernel,event);
    clEnv->library->clReleaseEvent(event);
    /* launch the kernel */
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    RecordProfileData(clEnv,HullPass2Kernel,event);
    clEnv->library->clReleaseEvent(event);

    offset.s[0] = -X[k];
    offset.s[1] = -Y[k];
    polarity = -1;
    clStatus = clEnv->library->clSetKernelArg(hullPass1,4,sizeof(cl_int2),(void *)&offset);
    clStatus|= clEnv->library->clSetKernelArg(hullPass1,5,sizeof(int),(void *)&polarity);
    clStatus|=clEnv->library->clSetKernelArg(hullPass2,4,sizeof(cl_int2),(void *)&offset);
    clStatus|=clEnv->library->clSetKernelArg(hullPass2,5,sizeof(int),(void *)&polarity);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }
    /* launch the kernel */
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    RecordProfileData(clEnv,HullPass1Kernel,event);
    clEnv->library->clReleaseEvent(event);
    /* launch the kernel */
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    RecordProfileData(clEnv,HullPass2Kernel,event);
    clEnv->library->clReleaseEvent(event);

    offset.s[0] = X[k];
    offset.s[1] = Y[k];
    polarity = -1;
    clStatus = clEnv->library->clSetKernelArg(hullPass1,4,sizeof(cl_int2),(void *)&offset);
    clStatus|= clEnv->library->clSetKernelArg(hullPass1,5,sizeof(int),(void *)&polarity);
    clStatus|=clEnv->library->clSetKernelArg(hullPass2,4,sizeof(cl_int2),(void *)&offset);
    clStatus|=clEnv->library->clSetKernelArg(hullPass2,5,sizeof(int),(void *)&polarity);

    if (k == 3)
      clStatus |=clEnv->library->clSetKernelArg(hullPass2,1,sizeof(cl_mem),(void *)&filteredImageBuffer);

    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
      goto cleanup;
    }
    /* launch the kernel */
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    RecordProfileData(clEnv,HullPass1Kernel,event);
    clEnv->library->clReleaseEvent(event);
    /* launch the kernel */
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    if ((k == 3) && (RecordProfileData(clEnv,HullPass2Kernel,event) == MagickFalse))
      {
        AddOpenCLEvent(image,event);
        AddOpenCLEvent(filteredImage,event);
      }
    clEnv->library->clReleaseEvent(event);
  }

  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (queue != NULL)                          RelinquishOpenCLCommandQueue(clEnv, queue);
  for (k = 0; k < 2; k++)
  {
    if (tempImageBuffer[k]!=NULL)	      clEnv->library->clReleaseMemObject(tempImageBuffer[k]);
  }
  if (hullPass1!=NULL)			      RelinquishOpenCLKernel(clEnv, hullPass1);
  if (hullPass2!=NULL)			      RelinquishOpenCLKernel(clEnv, hullPass2);
  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);
  return(filteredImage);
}

MagickPrivate Image *AccelerateDespeckleImage(const Image* image,
  ExceptionInfo* exception)
{
  Image
    *filteredImage;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, AllChannels) == MagickFalse))
    return NULL;

  filteredImage=ComputeDespeckleImage(image,exception);
  return(filteredImage);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e E q u a l i z e I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

MagickPrivate MagickBooleanType ComputeEqualizeImage(Image *image,
  const ChannelType channel,ExceptionInfo *exception)
{
#define EqualizeImageTag  "Equalize/Image"

  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    clStatus;

  cl_mem
    equalizeMapBuffer,
    histogramBuffer,
    imageBuffer;

  cl_kernel
    equalizeKernel,
    histogramKernel;

  cl_event
    event;

  cl_uint
    event_count;

  cl_uint4
    *histogram;

  const cl_event
    *events;

  cl_float4
    white,
    black,
    intensity,
    *map;

  MagickBooleanType
    outputReady,
    status;

  MagickCLEnv
    clEnv;

  MagickSizeType
    length;

  PixelPacket
    *equalize_map;

  register ssize_t
    i;

  size_t
    global_work_size[2];

  map=NULL;
  histogram=NULL;
  equalize_map=NULL;
  imageBuffer = NULL;
  histogramBuffer = NULL;
  equalizeMapBuffer = NULL;
  histogramKernel = NULL; 
  equalizeKernel = NULL; 
  context = NULL;
  queue = NULL;
  outputReady = MagickFalse;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  /*
   * initialize opencl env
   */
  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  /*
    Allocate and initialize histogram arrays.
  */
  length=MaxMap+1UL;
  histogram=(cl_uint4 *) AcquireQuantumMemory(length, sizeof(*histogram));
  if (histogram == (cl_uint4 *) NULL)
      ThrowBinaryException(ResourceLimitWarning,"MemoryAllocationFailed", image->filename);

  /* reset histogram */
  (void) ResetMagickMemory(histogram,0,length*sizeof(*histogram));

  imageBuffer = GetAuthenticOpenCLBuffer(image, exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  /* create a CL buffer for histogram  */
  histogramBuffer = clEnv->library->clCreateBuffer(context,  CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, length * sizeof(cl_uint4), histogram, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  status = LaunchHistogramKernel(clEnv, queue, imageBuffer, histogramBuffer, image, channel, exception);
  if (status == MagickFalse)
    goto cleanup;

  /* this blocks, should be fixed it in the future */
  events=GetOpenCLEvents(image,&event_count);
  clEnv->library->clEnqueueMapBuffer(queue, histogramBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(cl_uint4), event_count, events, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", "'%s'", ".");
    goto cleanup;
  }

  /* unmap, don't block gpu to use this buffer again.  */
  clStatus = clEnv->library->clEnqueueUnmapMemObject(queue, histogramBuffer, histogram, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueUnmapMemObject failed.", "'%s'", ".");
    goto cleanup;
  }

  /* CPU stuff */
  equalize_map=(PixelPacket *) AcquireQuantumMemory(length, sizeof(*equalize_map));
  if (equalize_map == (PixelPacket *) NULL)
    ThrowBinaryException(ResourceLimitWarning,"MemoryAllocationFailed", image->filename);

  map=(cl_float4 *) AcquireQuantumMemory(length,sizeof(*map));
  if (map == (cl_float4 *) NULL)
    ThrowBinaryException(ResourceLimitWarning,"MemoryAllocationFailed", image->filename);

  /*
    Integrate the histogram to get the equalization map.
  */
  (void) ResetMagickMemory(&intensity,0,sizeof(intensity));
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if ((channel & SyncChannels) != 0)
    {
      intensity.z+=histogram[i].s[2];
      map[i]=intensity;
      continue;
    }
    if ((channel & RedChannel) != 0)
      intensity.z+=histogram[i].s[2];
    if ((channel & GreenChannel) != 0)
      intensity.y+=histogram[i].s[1];
    if ((channel & BlueChannel) != 0)
      intensity.x+=histogram[i].s[0];
    if ((channel & OpacityChannel) != 0)
      intensity.w+=histogram[i].s[3];
    /*
    if (((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace))
    {
      intensity.index+=histogram[i].index; 
    }
    */
    map[i]=intensity;
  }
  black=map[0];
  white=map[(int) MaxMap];
  (void) ResetMagickMemory(equalize_map,0,length*sizeof(*equalize_map));
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if ((channel & SyncChannels) != 0)
    {
      if (white.z != black.z)
        equalize_map[i].red=ScaleMapToQuantum((MagickRealType) ((MaxMap*
                (map[i].z-black.z))/(white.z-black.z)));
      continue;
    }
    if (((channel & RedChannel) != 0) && (white.z != black.z))
      equalize_map[i].red=ScaleMapToQuantum((MagickRealType) ((MaxMap*
              (map[i].z-black.z))/(white.z-black.z)));
    if (((channel & GreenChannel) != 0) && (white.y != black.y))
      equalize_map[i].green=ScaleMapToQuantum((MagickRealType) ((MaxMap*
              (map[i].y-black.y))/(white.y-black.y)));
    if (((channel & BlueChannel) != 0) && (white.x != black.x))
      equalize_map[i].blue=ScaleMapToQuantum((MagickRealType) ((MaxMap*
              (map[i].x-black.x))/(white.x-black.x)));
    if (((channel & OpacityChannel) != 0) && (white.w != black.w))
      equalize_map[i].opacity=ScaleMapToQuantum((MagickRealType) ((MaxMap*
              (map[i].w-black.w))/(white.w-black.w)));
    /*
    if ((((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace)) &&
        (white.index != black.index))
      equalize_map[i].index=ScaleMapToQuantum((MagickRealType) ((MaxMap*
              (map[i].index-black.index))/(white.index-black.index)));
    */
  }

  if (image->storage_class == PseudoClass)
  {
    /*
       Equalize colormap.
       */
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      if ((channel & SyncChannels) != 0)
      {
        if (white.z != black.z)
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
      if (((channel & RedChannel) != 0) && (white.z != black.z))
        image->colormap[i].red=equalize_map[
          ScaleQuantumToMap(image->colormap[i].red)].red;
      if (((channel & GreenChannel) != 0) && (white.y != black.y))
        image->colormap[i].green=equalize_map[
          ScaleQuantumToMap(image->colormap[i].green)].green;
      if (((channel & BlueChannel) != 0) && (white.x != black.x))
        image->colormap[i].blue=equalize_map[
          ScaleQuantumToMap(image->colormap[i].blue)].blue;
      if (((channel & OpacityChannel) != 0) &&
          (white.w != black.w))
        image->colormap[i].opacity=equalize_map[
          ScaleQuantumToMap(image->colormap[i].opacity)].opacity;
    }
  }

  /* create a CL buffer for eqaulize_map  */
  equalizeMapBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, length * sizeof(PixelPacket), equalize_map, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
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
  clStatus=clEnv->library->clSetKernelArg(equalizeKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(equalizeKernel,i++,sizeof(ChannelType),&channel);
  clStatus|=clEnv->library->clSetKernelArg(equalizeKernel,i++,sizeof(cl_mem),(void *)&equalizeMapBuffer);
  clStatus|=clEnv->library->clSetKernelArg(equalizeKernel,i++,sizeof(cl_float4),&white);
  clStatus|=clEnv->library->clSetKernelArg(equalizeKernel,i++,sizeof(cl_float4),&black);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  /* launch the kernel */
  global_work_size[0] = image->columns;
  global_work_size[1] = image->rows;

  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, equalizeKernel, 2, NULL, global_work_size, NULL, 0, NULL, &event);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  if (RecordProfileData(clEnv,EqualizeKernel,event) == MagickFalse)
    AddOpenCLEvent(image,event);
  clEnv->library->clReleaseEvent(event);

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (map!=NULL)
    map=(cl_float4 *) RelinquishMagickMemory(map);

  if (equalizeMapBuffer!=NULL)
    clEnv->library->clReleaseMemObject(equalizeMapBuffer);
  if (equalize_map!=NULL)
    equalize_map=(PixelPacket *) RelinquishMagickMemory(equalize_map);

  if (histogramBuffer!=NULL)		      
    clEnv->library->clReleaseMemObject(histogramBuffer);
  if (histogram!=NULL)
    histogram=(cl_uint4 *) RelinquishMagickMemory(histogram);

  if (histogramKernel!=NULL)                     
    RelinquishOpenCLKernel(clEnv, histogramKernel);
  if (equalizeKernel!=NULL)                     
    RelinquishOpenCLKernel(clEnv, equalizeKernel);

  if (queue != NULL)                          
    RelinquishOpenCLCommandQueue(clEnv, queue);

  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateEqualizeImage(Image *image,
  const ChannelType channel,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, channel) == MagickFalse) ||
      (checkHistogramCondition(image, channel) == MagickFalse))
    return(MagickFalse);

  status=ComputeEqualizeImage(image,channel,exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e F u n c t i o n I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static MagickBooleanType ComputeFunctionImage(Image *image,
  const ChannelType channel,const MagickFunction function,
  const size_t number_parameters,const double *parameters,
  ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    clStatus;

  cl_kernel
    clkernel;

  cl_event
    event;

  cl_mem
    imageBuffer,
    parametersBuffer;

  const cl_event
    *events;

  float
    *parametersBufferPtr;

  MagickBooleanType
    status;

  MagickCLEnv
    clEnv;

  size_t
    globalWorkSize[2];

  unsigned int
    event_count,
    i;

  status = MagickFalse;

  context = NULL;
  clkernel = NULL;
  queue = NULL;
  imageBuffer = NULL;
  parametersBuffer = NULL;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);

  queue = AcquireOpenCLCommandQueue(clEnv);

  imageBuffer = GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }


  {
	  parametersBufferPtr = (float*)AcquireMagickMemory(number_parameters * sizeof(float));

	  for (i = 0; i < number_parameters; i++)
		  parametersBufferPtr[i] = (float)parameters[i];

	  parametersBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, number_parameters * sizeof(float), parametersBufferPtr, &clStatus);
	  parametersBufferPtr=RelinquishMagickMemory(parametersBufferPtr);
  }

  clkernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "ComputeFunction");
  if (clkernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  /* set the kernel arguments */
  i = 0;
  clStatus =clEnv->library->clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(ChannelType),(void *)&channel);
  clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(MagickFunction),(void *)&function);
  clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&number_parameters);
  clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(cl_mem),(void *)&parametersBuffer);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  globalWorkSize[0] = image->columns;
  globalWorkSize[1] = image->rows;
  /* launch the kernel */
  events=GetOpenCLEvents(image,&event_count);
  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, clkernel, 2, NULL, globalWorkSize, NULL, event_count, events, &event);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  if (RecordProfileData(clEnv,ComputeFunctionKernel,event) == MagickFalse)
    AddOpenCLEvent(image,event);
  clEnv->library->clReleaseEvent(event);
  status = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (clkernel != NULL) RelinquishOpenCLKernel(clEnv, clkernel);
  if (queue != NULL) RelinquishOpenCLCommandQueue(clEnv, queue);
  if (parametersBuffer != NULL) clEnv->library->clReleaseMemObject(parametersBuffer);

  return(status);
}

MagickPrivate MagickBooleanType AccelerateFunctionImage(Image *image,
  const ChannelType channel,const MagickFunction function,
  const size_t number_parameters,const double *parameters,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, channel) == MagickFalse))
    return(MagickFalse);

  status=ComputeFunctionImage(image, channel, function, number_parameters, parameters, exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e G r a y s c a l e I m a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

MagickBooleanType ComputeGrayscaleImage(Image *image,
  const PixelIntensityMethod method,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    clStatus,
    intensityMethod;

  cl_int
    colorspace;

  cl_kernel
    grayscaleKernel;

  cl_event
    event;

  cl_mem
    imageBuffer;

  cl_uint
    event_count;

  const cl_event
    *events;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  register ssize_t
    i;

  imageBuffer = NULL;
  grayscaleKernel = NULL; 

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  /*
   * initialize opencl env
   */
  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  outputReady = MagickFalse;

  imageBuffer = GetAuthenticOpenCLBuffer(image, exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  intensityMethod = method;
  colorspace = image->colorspace;

  grayscaleKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "Grayscale");
  if (grayscaleKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  i = 0;
  clStatus=clEnv->library->clSetKernelArg(grayscaleKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(grayscaleKernel,i++,sizeof(cl_int),&intensityMethod);
  clStatus|=clEnv->library->clSetKernelArg(grayscaleKernel,i++,sizeof(cl_int),&colorspace);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    printf("no kernel\n");
    goto cleanup;
  }

  {
    size_t global_work_size[2];
    global_work_size[0] = image->columns;
    global_work_size[1] = image->rows;
    /* launch the kernel */
    events=GetOpenCLEvents(image,&event_count);
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, grayscaleKernel, 2, NULL, global_work_size, NULL, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    if (RecordProfileData(clEnv,GrayScaleKernel,event) == MagickFalse)
      AddOpenCLEvent(image,event);
    clEnv->library->clReleaseEvent(event);
  }

  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (grayscaleKernel!=NULL)                     
    RelinquishOpenCLKernel(clEnv, grayscaleKernel);
  if (queue != NULL)                          
    RelinquishOpenCLCommandQueue(clEnv, queue);

  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateGrayscaleImage(Image* image,
  const PixelIntensityMethod method,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, AllChannels) == MagickFalse))
    return(MagickFalse);

  if (method == Rec601LuminancePixelIntensityMethod || method == Rec709LuminancePixelIntensityMethod)
    return(MagickFalse);

  if (image->colorspace != sRGBColorspace)
    return(MagickFalse);

  status=ComputeGrayscaleImage(image,method,exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e L o c a l C o n t r a s t I m a g e                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static Image *ComputeLocalContrastImage(const Image *image,
  const double radius,const double strength,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    clStatus,
    iRadius;

  cl_kernel
    blurRowKernel,
    blurColumnKernel;

  cl_event
    event;

  cl_mem
    filteredImageBuffer,
    imageBuffer,
    tempImageBuffer;

  const cl_event
    *events;

  Image
    *filteredImage;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  MagickSizeType
    length;

  unsigned int
    event_count,
    i,
    imageColumns,
    imageRows,
    passes;

  clEnv = NULL;
  filteredImage = NULL;
  context = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  tempImageBuffer = NULL;
  blurRowKernel = NULL;
  blurColumnKernel = NULL;
  queue = NULL;
  outputReady = MagickFalse;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  imageBuffer = GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  /* create output */
  {
    filteredImage = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
    assert(filteredImage != NULL);
    if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
      goto cleanup;
    }

    filteredImageBuffer = GetAuthenticOpenCLBuffer(filteredImage,exception);
    if (filteredImageBuffer == (cl_mem) NULL)
    {
      (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
      goto cleanup;
    }
  }

  {
    /* create temp buffer */
    {
      length = image->columns * image->rows;
      tempImageBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_WRITE, length * sizeof(float), NULL, &clStatus);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
        goto cleanup;
      }
    }

    /* get the opencl kernel */
    {
      blurRowKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "LocalContrastBlurRow");
      if (blurRowKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };

      blurColumnKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "LocalContrastBlurApplyColumn");
      if (blurColumnKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };
    }

    {
      imageColumns = (unsigned int) image->columns;
      imageRows = (unsigned int) image->rows;
      iRadius = (cl_int) (image->rows > image->columns ? image->rows : image->columns) * 0.002f * fabs(radius); // Normalized radius, 100% gives blur radius of 20% of the largest dimension

      passes = (((1.0f * imageColumns) * imageColumns * iRadius) + 3999999999) / 4000000000.0f;
      passes = (passes < 1) ? 1: passes;

      /* set the kernel arguments */
      i = 0;
      clStatus=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(cl_int),(void *)&iRadius);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageRows);
      
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
        goto cleanup;
      }
    }

    /* launch the kernel */
    {
      int x;
      for (x = 0; x < passes; ++x) {
        size_t gsize[2];
        size_t wsize[2];
        size_t goffset[2];

        gsize[0] = 256;
        gsize[1] = (image->rows + passes - 1) / passes;
        wsize[0] = 256;
        wsize[1] = 1;
        goffset[0] = 0;
        goffset[1] = x * gsize[1];

        events=GetOpenCLEvents(image,&event_count);
        clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, blurRowKernel, 2, goffset, gsize, wsize, event_count, events, &event);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
          goto cleanup;
        }
        clEnv->library->clFlush(queue);
        if (RecordProfileData(clEnv,LocalContrastBlurRowKernel,event) == MagickFalse)
          {
            AddOpenCLEvent(image,event);
            AddOpenCLEvent(filteredImage, event);
          }
        clEnv->library->clReleaseEvent(event);
      }
    }

    {
      cl_float FStrength = strength;
      i = 0;
      clStatus=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&iRadius);
      clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(cl_float),(void *)&FStrength);
      clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
      clStatus|=clEnv->library->clSetKernelArg(blurColumnKernel,i++,sizeof(unsigned int),(void *)&imageRows);

      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
        goto cleanup;
      }
    }

    /* launch the kernel */
    {
      int x;
      for (x = 0; x < passes; ++x) {
        size_t gsize[2];
        size_t wsize[2];
        size_t goffset[2];

        gsize[0] = ((image->columns + 3) / 4) * 4;
        gsize[1] = ((((image->rows + 63) / 64) + (passes + 1)) / passes) * 64;
        wsize[0] = 4;
        wsize[1] = 64;
        goffset[0] = 0;
        goffset[1] = x * gsize[1];

        events=GetOpenCLEvents(image,&event_count);
        clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, blurColumnKernel, 2, goffset, gsize, wsize, event_count, events, &event);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
          goto cleanup;
        }
        clEnv->library->clFlush(queue);
        if (RecordProfileData(clEnv, LocalContrastBlurApplyColumnKernel, event) == MagickFalse)
          {
            AddOpenCLEvent(image,event);
            AddOpenCLEvent(filteredImage,event);
          }
        clEnv->library->clReleaseEvent(event);
      }
    }
  }

  outputReady = MagickTrue;


cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (tempImageBuffer!=NULL)                  clEnv->library->clReleaseMemObject(tempImageBuffer);
  if (blurRowKernel!=NULL)                    RelinquishOpenCLKernel(clEnv, blurRowKernel);
  if (blurColumnKernel!=NULL)                 RelinquishOpenCLKernel(clEnv, blurColumnKernel);
  if (queue != NULL)                          RelinquishOpenCLCommandQueue(clEnv, queue);
  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);
  return(filteredImage);
}

MagickPrivate Image *AccelerateLocalContrastImage(const Image *image,
  const double radius,const double strength,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse))
    return NULL;

  filteredImage=ComputeLocalContrastImage(image,radius,strength,exception);

  return(filteredImage);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e M o d u l a t e I m a g e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

MagickBooleanType ComputeModulateImage(Image *image,
  double percent_brightness, double percent_hue, double percent_saturation,
  ColorspaceType colorspace, ExceptionInfo *exception)
{
  cl_float
    bright,
    hue,
    saturation;

  cl_context
    context;

  cl_command_queue
    queue;

  cl_int
    color,
    clStatus;

  cl_kernel
    modulateKernel;

  cl_event
    event;

  cl_mem
    imageBuffer;

  const cl_event
    *events;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  register ssize_t
    i;

  unsigned int
    event_count;

  imageBuffer = NULL;
  modulateKernel = NULL;
  event_count = 0;

  assert(image != (Image *)NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent, GetMagickModule(), "%s", image->filename);

  /*
  * initialize opencl env
  */
  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  outputReady = MagickFalse;

  imageBuffer = GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  modulateKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "Modulate");
  if (modulateKernel == NULL)
  {
    (void)OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  bright = percent_brightness;
  hue = percent_hue;
  saturation = percent_saturation;
  color = colorspace;

  i = 0;
  clStatus = clEnv->library->clSetKernelArg(modulateKernel, i++, sizeof(cl_mem), (void *)&imageBuffer);
  clStatus |= clEnv->library->clSetKernelArg(modulateKernel, i++, sizeof(cl_float), &bright);
  clStatus |= clEnv->library->clSetKernelArg(modulateKernel, i++, sizeof(cl_float), &hue);
  clStatus |= clEnv->library->clSetKernelArg(modulateKernel, i++, sizeof(cl_float), &saturation);
  clStatus |= clEnv->library->clSetKernelArg(modulateKernel, i++, sizeof(cl_float), &color);
  if (clStatus != CL_SUCCESS)
  {
    (void)OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    printf("no kernel\n");
    goto cleanup;
  }

  {
    size_t global_work_size[2];
    global_work_size[0] = image->columns;
    global_work_size[1] = image->rows;
    /* launch the kernel */
    events=GetOpenCLEvents(image,&event_count);
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, modulateKernel, 2, NULL, global_work_size, NULL, event_count, events, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void)OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
      goto cleanup;
    }
    if (RecordProfileData(clEnv, ModulateKernel, event) == MagickFalse)
      AddOpenCLEvent(image,event);
    clEnv->library->clReleaseEvent(event);
  }

  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__, __LINE__, exception);

  if (modulateKernel != NULL)
    RelinquishOpenCLKernel(clEnv, modulateKernel);
  if (queue != NULL)
    RelinquishOpenCLCommandQueue(clEnv, queue);

  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateModulateImage(Image *image,
  double percent_brightness, double percent_hue, double percent_saturation,
  ColorspaceType colorspace, ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *)NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
    (checkAccelerateCondition(image, AllChannels) == MagickFalse))
    return(MagickFalse);

  if ((colorspace != HSLColorspace && colorspace != UndefinedColorspace))
    return(MagickFalse);

  status = ComputeModulateImage(image, percent_brightness, percent_hue, percent_saturation, colorspace, exception);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e M o t i o n B l u r I m a g e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static Image* ComputeMotionBlurImage(const Image *image,
  const ChannelType channel,const double *kernel,const size_t width, 
  const OffsetInfo *offset,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_context
    context;

  cl_float4
    biasPixel;

  cl_int
    clStatus;

  cl_kernel
    motionBlurKernel;

  cl_event
    event;

  cl_mem
    filteredImageBuffer,
    imageBuffer,
    imageKernelBuffer, 
    offsetBuffer;

  cl_uint
    event_count;

  const cl_event
    *events;

  float
    *kernelBufferPtr;

  Image
    *filteredImage;

  int
    *offsetBufferPtr;

  MagickBooleanType
    outputReady;

  MagickCLEnv
   clEnv;

  MagickPixelPacket
    bias;

  size_t
    global_work_size[2],
    local_work_size[2];

  unsigned int
    i,
    imageHeight,
    imageWidth,
    matte;

  outputReady = MagickFalse;
  context = NULL;
  filteredImage = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  imageKernelBuffer = NULL;
  motionBlurKernel = NULL;
  queue = NULL;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);

  imageBuffer = GetAuthenticOpenCLBuffer(image, exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  filteredImage = CloneImage(image,image->columns,image->rows,
    MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), 
      ResourceLimitError, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredImageBuffer = GetAuthenticOpenCLBuffer(filteredImage, exception);
  if (filteredImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  imageKernelBuffer = clEnv->library->clCreateBuffer(context, 
    CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, width * sizeof(float), NULL,
    &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), 
      ResourceLimitError, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  queue = AcquireOpenCLCommandQueue(clEnv);
  events=GetOpenCLEvents(image,&event_count);
  /* this blocks, should be fixed it in the future */
  kernelBufferPtr = (float*)clEnv->library->clEnqueueMapBuffer(queue, imageKernelBuffer, 
    CL_TRUE, CL_MAP_WRITE, 0, width * sizeof(float), event_count, events, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), 
      ResourceLimitError, "clEnv->library->clEnqueueMapBuffer failed.",".");
    goto cleanup;
  }
  for (i = 0; i < width; i++)
  {
    kernelBufferPtr[i] = (float) kernel[i];
  }
  clStatus = clEnv->library->clEnqueueUnmapMemObject(queue, imageKernelBuffer, kernelBufferPtr,
    0, NULL, NULL);
 if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleFatalError, 
      "clEnv->library->clEnqueueUnmapMemObject failed.", "'%s'", ".");
    goto cleanup;
  }

  offsetBuffer = clEnv->library->clCreateBuffer(context, 
    CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, width * sizeof(cl_int2), NULL,
    &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), 
      ResourceLimitError, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  offsetBufferPtr = (int*)clEnv->library->clEnqueueMapBuffer(queue, offsetBuffer, CL_TRUE, 
    CL_MAP_WRITE, 0, width * sizeof(cl_int2), 0, NULL, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), 
      ResourceLimitError, "clEnv->library->clEnqueueMapBuffer failed.",".");
    goto cleanup;
  }
  for (i = 0; i < width; i++)
  {
    offsetBufferPtr[2*i] = (int)offset[i].x;
    offsetBufferPtr[2*i+1] = (int)offset[i].y;
  }
  clStatus = clEnv->library->clEnqueueUnmapMemObject(queue, offsetBuffer, offsetBufferPtr, 0, 
    NULL, NULL);
 if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleFatalError,
      "clEnv->library->clEnqueueUnmapMemObject failed.", "'%s'", ".");
    goto cleanup;
  }


 // get the OpenCL kernel
  motionBlurKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, 
    "MotionBlur");
  if (motionBlurKernel == NULL)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleFatalError,
      "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  
  // set the kernel arguments
  i = 0;
  clStatus=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(cl_mem),
    (void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(cl_mem),
    (void *)&filteredImageBuffer);
  imageWidth = (unsigned int) image->columns;
  imageHeight = (unsigned int) image->rows;
  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(unsigned int),
    &imageWidth);
  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(unsigned int),
    &imageHeight);
  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(cl_mem),
    (void *)&imageKernelBuffer);
  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(unsigned int),
    &width);
  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(cl_mem),
    (void *)&offsetBuffer);

  GetMagickPixelPacket(image,&bias);
  biasPixel.s[0] = bias.red;
  biasPixel.s[1] = bias.green;
  biasPixel.s[2] = bias.blue;
  biasPixel.s[3] = bias.opacity;
  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(cl_float4), &biasPixel);

  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(ChannelType), &channel);
  matte = (image->matte != MagickFalse)?1:0;
  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(unsigned int), &matte);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleFatalError,
      "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  // launch the kernel
  local_work_size[0] = 16;
  local_work_size[1] = 16;
  global_work_size[0] = (size_t)padGlobalWorkgroupSizeToLocalWorkgroupSize(
                                (unsigned int) image->columns,(unsigned int) local_work_size[0]);
  global_work_size[1] = (size_t)padGlobalWorkgroupSizeToLocalWorkgroupSize(
                                (unsigned int) image->rows,(unsigned int) local_work_size[1]);
  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, motionBlurKernel, 2, NULL, 
    global_work_size, local_work_size, 0, NULL, &event);

  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleFatalError,
      "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  if (RecordProfileData(clEnv,MotionBlurKernel,event) == MagickFalse)
    {
      AddOpenCLEvent(image, event);
      AddOpenCLEvent(filteredImage, event);
    }
  clEnv->library->clReleaseEvent(event);

  outputReady = MagickTrue;

cleanup:

  if (imageKernelBuffer!=NULL)    clEnv->library->clReleaseMemObject(imageKernelBuffer);
  if (motionBlurKernel!=NULL)  RelinquishOpenCLKernel(clEnv, motionBlurKernel);
  if (queue != NULL)           RelinquishOpenCLCommandQueue(clEnv, queue);
  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);

  return(filteredImage);
}

MagickPrivate Image *AccelerateMotionBlurImage(const Image *image,
  const ChannelType channel,const double* kernel,const size_t width,
  const OffsetInfo *offset,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  assert(image != NULL);
  assert(kernel != (double *) NULL);
  assert(offset != (OffsetInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, channel) == MagickFalse))
    return NULL;

  filteredImage=ComputeMotionBlurImage(image, channel, kernel, width,
    offset, exception);
  return(filteredImage);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e R a d i a l B l u r I m a g e                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static Image *ComputeRadialBlurImage(const Image *image,
  const ChannelType channel,const double angle,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_context
    context;

  cl_float2
    blurCenter;

  cl_float4
    biasPixel;

  cl_int
    clStatus;

  cl_mem
    cosThetaBuffer,
    filteredImageBuffer,
    imageBuffer,
    sinThetaBuffer;

  cl_kernel
    radialBlurKernel;

  cl_event
    event;

  cl_uint
    event_count;

  const cl_event
    *events;

  float
    blurRadius,
    *cosThetaPtr,
    offset,
    *sinThetaPtr,
    theta;

  Image
    *filteredImage;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  MagickPixelPacket
    bias;

  size_t
    global_work_size[2];

  unsigned int
    cossin_theta_size,
    i,
    matte;

  outputReady = MagickFalse;
  context = NULL;
  filteredImage = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  sinThetaBuffer = NULL;
  cosThetaBuffer = NULL;
  queue = NULL;
  radialBlurKernel = NULL;


  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);

  imageBuffer = GetAuthenticOpenCLBuffer(image, exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  filteredImage = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredImageBuffer = GetAuthenticOpenCLBuffer(filteredImage, exception);
  if (filteredImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  blurCenter.s[0] = (float) (image->columns-1)/2.0;
  blurCenter.s[1] = (float) (image->rows-1)/2.0;
  blurRadius=hypot(blurCenter.s[0],blurCenter.s[1]);
  cossin_theta_size=(unsigned int) fabs(4.0*DegreesToRadians(angle)*sqrt((double)blurRadius)+2UL);

  /* create a buffer for sin_theta and cos_theta */
  sinThetaBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, cossin_theta_size * sizeof(float), NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }
  cosThetaBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, cossin_theta_size * sizeof(float), NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  queue = AcquireOpenCLCommandQueue(clEnv);
  events=GetOpenCLEvents(image,&event_count);
  /* this blocks, should be fixed it in the future */
  sinThetaPtr = (float*) clEnv->library->clEnqueueMapBuffer(queue, sinThetaBuffer, CL_TRUE, CL_MAP_WRITE, 0, cossin_theta_size*sizeof(float), event_count, events, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnqueuemapBuffer failed.",".");
    goto cleanup;
  }

  cosThetaPtr = (float*) clEnv->library->clEnqueueMapBuffer(queue, cosThetaBuffer, CL_TRUE, CL_MAP_WRITE, 0, cossin_theta_size*sizeof(float), 0, NULL, NULL, &clStatus);
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
 
  clStatus = clEnv->library->clEnqueueUnmapMemObject(queue, sinThetaBuffer, sinThetaPtr, 0, NULL, NULL);
  clStatus |= clEnv->library->clEnqueueUnmapMemObject(queue, cosThetaBuffer, cosThetaPtr, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueUnmapMemObject failed.", "'%s'", ".");
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
  clStatus=clEnv->library->clSetKernelArg(radialBlurKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(radialBlurKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);

  GetMagickPixelPacket(image,&bias);
  biasPixel.s[0] = bias.red;
  biasPixel.s[1] = bias.green;
  biasPixel.s[2] = bias.blue;
  biasPixel.s[3] = bias.opacity;
  clStatus|=clEnv->library->clSetKernelArg(radialBlurKernel,i++,sizeof(cl_float4), &biasPixel);
  clStatus|=clEnv->library->clSetKernelArg(radialBlurKernel,i++,sizeof(ChannelType), &channel);

  matte = (image->matte != MagickFalse)?1:0;
  clStatus|=clEnv->library->clSetKernelArg(radialBlurKernel,i++,sizeof(unsigned int), &matte);

  clStatus=clEnv->library->clSetKernelArg(radialBlurKernel,i++,sizeof(cl_float2), &blurCenter);

  clStatus|=clEnv->library->clSetKernelArg(radialBlurKernel,i++,sizeof(cl_mem),(void *)&cosThetaBuffer);
  clStatus|=clEnv->library->clSetKernelArg(radialBlurKernel,i++,sizeof(cl_mem),(void *)&sinThetaBuffer);
  clStatus|=clEnv->library->clSetKernelArg(radialBlurKernel,i++,sizeof(unsigned int), &cossin_theta_size);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }


  global_work_size[0] = image->columns;
  global_work_size[1] = image->rows;
  /* launch the kernel */
  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, radialBlurKernel, 2, NULL, global_work_size, NULL, 0, NULL, &event);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  if (RecordProfileData(clEnv,RadialBlurKernel,event) == MagickFalse)
    {
      AddOpenCLEvent(image,event);
      AddOpenCLEvent(filteredImage,event);
    }
  clEnv->library->clReleaseEvent(event);

  outputReady = MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (sinThetaBuffer!=NULL)       clEnv->library->clReleaseMemObject(sinThetaBuffer);
  if (cosThetaBuffer!=NULL)       clEnv->library->clReleaseMemObject(cosThetaBuffer);
  if (radialBlurKernel!=NULL)     RelinquishOpenCLKernel(clEnv, radialBlurKernel);
  if (queue != NULL)              RelinquishOpenCLCommandQueue(clEnv, queue);
  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);
  return filteredImage;
}

MagickPrivate Image *AccelerateRadialBlurImage(const Image *image,
  const ChannelType channel,const double angle,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, channel) == MagickFalse))
    return NULL;

  filteredImage=ComputeRadialBlurImage(image, channel, angle, exception);
  return filteredImage;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e R e s i z e I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static MagickBooleanType resizeHorizontalFilter(const Image *image,
  const Image *filteredImage,cl_mem imageBuffer,const unsigned int imageColumns,
  const unsigned int imageRows,const unsigned int matte,cl_mem resizedImage,
  const unsigned int resizedColumns,const unsigned int resizedRows,
  const ResizeFilter *resizeFilter,cl_mem resizeFilterCubicCoefficients,
  const float xFactor,MagickCLEnv clEnv,cl_command_queue queue,
  ExceptionInfo *exception)
{
  cl_kernel
    horizontalKernel;

  cl_event
    event;

  cl_int
    clStatus;

  cl_uint
    event_count;

  const cl_event
    *events;

  const unsigned int
    workgroupSize = 256;

  float
    resizeFilterScale,
    resizeFilterSupport,
    resizeFilterWindowSupport,
    resizeFilterBlur,
    scale,
    support;

  int
    cacheRangeStart,
    cacheRangeEnd,
    numCachedPixels,
    resizeFilterType,
    resizeWindowType;

  MagickBooleanType
    status = MagickFalse;

  size_t
    deviceLocalMemorySize,
    gammaAccumulatorLocalMemorySize,
    global_work_size[2],
    imageCacheLocalMemorySize,
    pixelAccumulatorLocalMemorySize,
    local_work_size[2],
    totalLocalMemorySize,
    weightAccumulatorLocalMemorySize;

  unsigned int
    chunkSize,
    i,
    pixelPerWorkgroup;

  horizontalKernel = NULL;
  status = MagickFalse;

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

  horizontalKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "ResizeHorizontalFilter");
  if (horizontalKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  i = 0;
  clStatus = clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&imageBuffer);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&imageColumns);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&imageRows);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&matte);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&xFactor);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&resizedImage);

  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&resizedColumns);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&resizedRows);

  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(int), (void*)&resizeFilterType);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(int), (void*)&resizeWindowType);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&resizeFilterCubicCoefficients);

  resizeFilterScale = (float) GetResizeFilterScale(resizeFilter);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterScale);

  resizeFilterSupport = (float) GetResizeFilterSupport(resizeFilter);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterSupport);

  resizeFilterWindowSupport = (float) GetResizeFilterWindowSupport(resizeFilter);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterWindowSupport);

  resizeFilterBlur = (float) GetResizeFilterBlur(resizeFilter);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterBlur);


  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, imageCacheLocalMemorySize, NULL);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(int), &numCachedPixels);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), &pixelPerWorkgroup);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), &chunkSize);
  

  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, pixelAccumulatorLocalMemorySize, NULL);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, weightAccumulatorLocalMemorySize, NULL);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, gammaAccumulatorLocalMemorySize, NULL);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  global_work_size[0] = (resizedColumns+pixelPerWorkgroup-1)/pixelPerWorkgroup*workgroupSize;
  global_work_size[1] = resizedRows;

  local_work_size[0] = workgroupSize;
  local_work_size[1] = 1;
  events=GetOpenCLEvents(image,&event_count);
  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, horizontalKernel, 2, NULL, global_work_size, local_work_size, event_count, events, &event);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  if (RecordProfileData(clEnv,ResizeHorizontalKernel,event) == MagickFalse)
    {
      AddOpenCLEvent(image,event);
      AddOpenCLEvent(filteredImage,event);
    }
  clEnv->library->clReleaseEvent(event);
  status = MagickTrue;


cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (horizontalKernel != NULL) RelinquishOpenCLKernel(clEnv, horizontalKernel);

  return(status);
}

static MagickBooleanType resizeVerticalFilter(const Image *image,
  const Image *filteredImage,cl_mem imageBuffer,const unsigned int imageColumns,
  const unsigned int imageRows,const unsigned int matte,cl_mem resizedImage,
  const unsigned int resizedColumns,const unsigned int resizedRows,
  const ResizeFilter *resizeFilter,cl_mem resizeFilterCubicCoefficients,
  const float yFactor,MagickCLEnv clEnv,cl_command_queue queue,
  ExceptionInfo *exception)
{
  cl_kernel
    horizontalKernel;

  cl_event
    event;

  cl_int
    clStatus;

  cl_uint
    event_count;

  const cl_event
    *events;

  const unsigned int
    workgroupSize = 256;

  float
    resizeFilterScale,
    resizeFilterSupport,
    resizeFilterWindowSupport,
    resizeFilterBlur,
    scale,
    support;

  int
    cacheRangeStart,
    cacheRangeEnd,
    numCachedPixels,
    resizeFilterType,
    resizeWindowType;

  MagickBooleanType
    status = MagickFalse;

  size_t
    deviceLocalMemorySize,
    gammaAccumulatorLocalMemorySize,
    global_work_size[2],
    imageCacheLocalMemorySize,
    pixelAccumulatorLocalMemorySize,
    local_work_size[2],
    totalLocalMemorySize,
    weightAccumulatorLocalMemorySize;

  unsigned int
    chunkSize,
    i,
    pixelPerWorkgroup;

  horizontalKernel = NULL;
  status = MagickFalse;

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

  horizontalKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "ResizeVerticalFilter");
  if (horizontalKernel == NULL)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  }

  i = 0;
  clStatus = clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&imageBuffer);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&imageColumns);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&imageRows);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&matte);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&yFactor);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&resizedImage);

  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&resizedColumns);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), (void*)&resizedRows);

  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(int), (void*)&resizeFilterType);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(int), (void*)&resizeWindowType);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(cl_mem), (void*)&resizeFilterCubicCoefficients);

  resizeFilterScale = (float) GetResizeFilterScale(resizeFilter);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterScale);

  resizeFilterSupport = (float) GetResizeFilterSupport(resizeFilter);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterSupport);

  resizeFilterWindowSupport = (float) GetResizeFilterWindowSupport(resizeFilter);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterWindowSupport);

  resizeFilterBlur = (float) GetResizeFilterBlur(resizeFilter);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(float), (void*)&resizeFilterBlur);


  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, imageCacheLocalMemorySize, NULL);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(int), &numCachedPixels);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), &pixelPerWorkgroup);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, sizeof(unsigned int), &chunkSize);
  

  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, pixelAccumulatorLocalMemorySize, NULL);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, weightAccumulatorLocalMemorySize, NULL);
  clStatus |= clEnv->library->clSetKernelArg(horizontalKernel, i++, gammaAccumulatorLocalMemorySize, NULL);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
    goto cleanup;
  }

  global_work_size[0] = resizedColumns;
  global_work_size[1] = (resizedRows+pixelPerWorkgroup-1)/pixelPerWorkgroup*workgroupSize;

  local_work_size[0] = 1;
  local_work_size[1] = workgroupSize;
  events=GetOpenCLEvents(image,&event_count);
  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, horizontalKernel, 2, NULL, global_work_size, local_work_size, event_count, events, &event);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
    goto cleanup;
  }
  if (RecordProfileData(clEnv,ResizeVerticalKernel,event) == MagickFalse)
    {
      AddOpenCLEvent(image,event);
      AddOpenCLEvent(filteredImage,event);
    }
  clEnv->library->clReleaseEvent(event);
  status = MagickTrue;


cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (horizontalKernel != NULL) RelinquishOpenCLKernel(clEnv, horizontalKernel);

  return(status);
}

static Image *ComputeResizeImage(const Image* image,
  const size_t resizedColumns,const size_t resizedRows,
  const ResizeFilter *resizeFilter,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_int
    clStatus;

  cl_context
    context;

  cl_mem
    cubicCoefficientsBuffer,
    filteredImageBuffer,
    imageBuffer,
    tempImageBuffer;

  const MagickRealType
    *resizeFilterCoefficient;

  float
    coefficientBuffer[7],
    xFactor,
    yFactor;

  MagickBooleanType
    outputReady,
    status;

  MagickCLEnv
    clEnv;

  MagickSizeType
    length;

  Image
    *filteredImage;

  size_t
    i;

  outputReady = MagickFalse;
  filteredImage = NULL;
  clEnv = NULL;
  context = NULL;
  imageBuffer = NULL;
  tempImageBuffer = NULL;
  filteredImageBuffer = NULL;
  cubicCoefficientsBuffer = NULL;
  queue = NULL;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  imageBuffer=GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  filteredImage=CloneImage(image,resizedColumns,resizedRows,MagickTrue,exception);
  if (filteredImage == NULL)
    goto cleanup;
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredImageBuffer=GetAuthenticOpenCLBuffer(filteredImage,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  resizeFilterCoefficient=GetResizeFilterCoefficient(resizeFilter);
  for (i = 0; i < 7; i++)
    coefficientBuffer[i]=(float) resizeFilterCoefficient[i];

  cubicCoefficientsBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(coefficientBuffer), coefficientBuffer, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  xFactor=(float) resizedColumns/(float) image->columns;
  yFactor=(float) resizedRows/(float) image->rows;
  if (xFactor > yFactor)
  {

    length = resizedColumns*image->rows;
    tempImageBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_WRITE, length*sizeof(CLPixelPacket), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
      goto cleanup;
    }
    
    status = resizeHorizontalFilter(image,filteredImage,imageBuffer, (unsigned int) image->columns, (unsigned int) image->rows, (image->matte != MagickFalse)?1:0
          , tempImageBuffer, (unsigned int) resizedColumns, (unsigned int) image->rows
          , resizeFilter, cubicCoefficientsBuffer
          , xFactor, clEnv, queue, exception);
    if (status != MagickTrue)
      goto cleanup;
    
    status = resizeVerticalFilter(image,filteredImage,tempImageBuffer, (unsigned int) resizedColumns, (unsigned int) image->rows, (image->matte != MagickFalse)?1:0
       , filteredImageBuffer, (unsigned int) resizedColumns, (unsigned int) resizedRows
       , resizeFilter, cubicCoefficientsBuffer
       , yFactor, clEnv, queue, exception);
    if (status != MagickTrue)
      goto cleanup;
  }
  else
  {
    length = image->columns*resizedRows;
    tempImageBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_WRITE, length*sizeof(CLPixelPacket), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
      goto cleanup;
    }

    status = resizeVerticalFilter(image,filteredImage,imageBuffer, (unsigned int) image->columns, (unsigned int) image->rows, (image->matte != MagickFalse)?1:0
       , tempImageBuffer, (unsigned int) image->columns, (unsigned int) resizedRows
       , resizeFilter, cubicCoefficientsBuffer
       , yFactor, clEnv, queue, exception);
    if (status != MagickTrue)
      goto cleanup;

    status = resizeHorizontalFilter(image,filteredImage,tempImageBuffer, (unsigned int) image->columns, (unsigned int) resizedRows, (image->matte != MagickFalse)?1:0
       , filteredImageBuffer, (unsigned int) resizedColumns, (unsigned int) resizedRows
       , resizeFilter, cubicCoefficientsBuffer
       , xFactor, clEnv, queue, exception);
    if (status != MagickTrue)
      goto cleanup;
  }
  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (tempImageBuffer!=NULL)		  clEnv->library->clReleaseMemObject(tempImageBuffer);
  if (cubicCoefficientsBuffer!=NULL)      clEnv->library->clReleaseMemObject(cubicCoefficientsBuffer);
  if (queue != NULL)  	                  RelinquishOpenCLCommandQueue(clEnv, queue);
  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);
  return(filteredImage);
}

static MagickBooleanType gpuSupportedResizeWeighting(
  ResizeWeightingFunctionType f)
{
  unsigned int
    i;

  for (i = 0; ;i++)
  {
    if (supportedResizeWeighting[i] == LastWeightingFunction)
      break;
    if (supportedResizeWeighting[i] == f)
      return(MagickTrue);
  }
  return(MagickFalse);
}

MagickPrivate Image *AccelerateResizeImage(const Image *image,
  const size_t resizedColumns,const size_t resizedRows,
  const ResizeFilter *resizeFilter,ExceptionInfo *exception) 
{
  Image
    *filteredImage;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, AllChannels) == MagickFalse))
    return NULL;

  if (gpuSupportedResizeWeighting(GetResizeFilterWeightingType(resizeFilter)) == MagickFalse ||
      gpuSupportedResizeWeighting(GetResizeFilterWindowWeightingType(resizeFilter)) == MagickFalse)
    return NULL;

  filteredImage=ComputeResizeImage(image,resizedColumns,resizedRows,resizeFilter,exception);
  return(filteredImage);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e U n s h a r p M a s k I m a g e                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static Image *ComputeUnsharpMaskImage(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  const double gain,const double threshold,ExceptionInfo *exception)
{
  char
    geometry[MaxTextExtent];

  cl_command_queue
    queue;

  cl_context
    context;

  cl_event
    event;

  cl_int
    clStatus;

  cl_kernel
    blurRowKernel,
    unsharpMaskBlurColumnKernel;

  cl_mem
    filteredImageBuffer,
    imageBuffer,
    imageKernelBuffer,
    tempImageBuffer;

  cl_uint
    event_count;

  const cl_event
    *events;

  float
    fGain,
    fThreshold,
    *kernelBufferPtr;

  Image
    *filteredImage;

  int
    chunkSize;

  KernelInfo
    *kernel;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  MagickSizeType
    length;

  unsigned int
    imageColumns,
    imageRows,
    kernelWidth;

  size_t
    i;

  clEnv = NULL;
  filteredImage = NULL;
  kernel = NULL;
  context = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  tempImageBuffer = NULL;
  imageKernelBuffer = NULL;
  blurRowKernel = NULL;
  unsharpMaskBlurColumnKernel = NULL;
  queue = NULL;
  outputReady = MagickFalse;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  imageBuffer=GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  filteredImage = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }
  filteredImageBuffer=GetAuthenticOpenCLBuffer(filteredImage,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
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

    kernelBufferPtr=AcquireQuantumMemory(kernel->width,sizeof(float));
    for (i = 0; i < kernel->width; i++)
      kernelBufferPtr[i]=(float) kernel->values[i];

    imageKernelBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, kernel->width * sizeof(float), kernelBufferPtr, &clStatus);
    kernelBufferPtr=RelinquishMagickMemory(kernelBufferPtr);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  {
    /* create temp buffer */
    {
      length = image->columns * image->rows;
      tempImageBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_READ_WRITE, length * 4 * sizeof(float), NULL, &clStatus);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
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

      imageColumns = (unsigned int) image->columns;
      imageRows = (unsigned int) image->rows;

      kernelWidth = (unsigned int) kernel->width;

      /* set the kernel arguments */
      i = 0;
      clStatus=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(ChannelType),&channel);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(unsigned int),(void *)&imageRows);
      clStatus|=clEnv->library->clSetKernelArg(blurRowKernel,i++,sizeof(CLPixelPacket)*(chunkSize+kernel->width),(void *) NULL);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
        goto cleanup;
      }
    }

    /* launch the kernel */
    {
      size_t gsize[2];
      size_t wsize[2];

      gsize[0] = chunkSize*((image->columns+chunkSize-1)/chunkSize);
      gsize[1] = image->rows;
      wsize[0] = chunkSize;
      wsize[1] = 1;

      events=GetOpenCLEvents(image,&event_count);
      clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, blurRowKernel, 2, NULL, gsize, wsize, event_count, events, NULL);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
        goto cleanup;
      }
    }


    {
      chunkSize = 256;
      imageColumns = (unsigned int) image->columns;
      imageRows = (unsigned int) image->rows;
      kernelWidth = (unsigned int) kernel->width;
      fGain = (float) gain;
      fThreshold = (float) threshold;

      i = 0;
      clStatus=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&imageRows);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++, (chunkSize+kernelWidth-1)*sizeof(cl_float4),NULL);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++, kernelWidth*sizeof(float),NULL);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(ChannelType),&channel);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(float),(void *)&fGain);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(float),(void *)&fThreshold);

      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
        goto cleanup;
      }
    }

    /* launch the kernel */
    {
      size_t gsize[2];
      size_t wsize[2];

      gsize[0] = image->columns;
      gsize[1] = chunkSize*((image->rows+chunkSize-1)/chunkSize);
      wsize[0] = 1;
      wsize[1] = chunkSize;

      clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, unsharpMaskBlurColumnKernel, 2, NULL, gsize, wsize, event_count, events, &event);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
        goto cleanup;
      }
      if (RecordProfileData(clEnv,UnsharpMaskBlurColumnKernel,event) == MagickFalse)
        {
          AddOpenCLEvent(image,event);
          AddOpenCLEvent(filteredImage,event);
        }
      clEnv->library->clReleaseEvent(event);
    }

  }

  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (kernel != NULL)			      kernel=DestroyKernelInfo(kernel);
  if (tempImageBuffer!=NULL)                  clEnv->library->clReleaseMemObject(tempImageBuffer);
  if (imageKernelBuffer!=NULL)                clEnv->library->clReleaseMemObject(imageKernelBuffer);
  if (blurRowKernel!=NULL)                    RelinquishOpenCLKernel(clEnv, blurRowKernel);
  if (unsharpMaskBlurColumnKernel!=NULL)      RelinquishOpenCLKernel(clEnv, unsharpMaskBlurColumnKernel);
  if (queue != NULL)                          RelinquishOpenCLCommandQueue(clEnv, queue);
  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);
  return(filteredImage);
}

static Image *ComputeUnsharpMaskImageSingle(const Image *image,
  const double radius,const double sigma,const double gain,
  const double threshold,int blurOnly, ExceptionInfo *exception)
{
  char
    geometry[MaxTextExtent];

  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    justBlur,
    clStatus;

  cl_kernel
    unsharpMaskKernel;

  cl_event
    event;

  cl_mem
    filteredImageBuffer,
    imageBuffer,
    imageKernelBuffer;

  const cl_event
    *events;

  float
    fGain,
    fThreshold;

  Image
    *filteredImage;

  KernelInfo
    *kernel;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  unsigned int
    event_count,
    i,
    imageColumns,
    imageRows,
    kernelWidth;

  clEnv = NULL;
  filteredImage = NULL;
  kernel = NULL;
  context = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  imageKernelBuffer = NULL;
  unsharpMaskKernel = NULL;
  queue = NULL;
  outputReady = MagickFalse;

  clEnv = GetDefaultOpenCLEnv();
  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  /* Create and initialize OpenCL buffers. */

  imageBuffer = GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  /* create output */
  {
    filteredImage = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
    assert(filteredImage != NULL);
    if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
    {
      (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
      goto cleanup;
    }

    filteredImageBuffer = GetAuthenticOpenCLBuffer(filteredImage,exception);
    if (filteredImageBuffer == (cl_mem) NULL)
    {
      (void) OpenCLThrowMagickException(exception,GetMagickModule(),
        ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
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

	{
		float *kernelBufferPtr = (float *) AcquireQuantumMemory(kernel->width, sizeof(float));
		for (i = 0; i < kernel->width; i++)
			kernelBufferPtr[i] = (float)kernel->values[i];

		imageKernelBuffer = clEnv->library->clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, kernel->width * sizeof(float), kernelBufferPtr, &clStatus);
		RelinquishMagickMemory(kernelBufferPtr);
		if (clStatus != CL_SUCCESS)
		{
		  (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
		  goto cleanup;
		}
	}
  }

  {
    /* get the opencl kernel */
    {
      unsharpMaskKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "UnsharpMask");
      if (unsharpMaskKernel == NULL)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
        goto cleanup;
      };
    }

    {
      imageColumns = (unsigned int) image->columns;
      imageRows = (unsigned int) image->rows;
      kernelWidth = (unsigned int) kernel->width;
      fGain = (float) gain;
      fThreshold = (float) threshold;
	  justBlur = blurOnly;

      /* set the kernel arguments */
      i = 0;
      clStatus=clEnv->library->clSetKernelArg(unsharpMaskKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskKernel,i++,sizeof(unsigned int),(void *)&kernelWidth);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskKernel,i++,sizeof(unsigned int),(void *)&imageColumns);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskKernel,i++,sizeof(unsigned int),(void *)&imageRows);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskKernel,i++,sizeof(cl_float4)*(8 * (32 + kernel->width)),(void *) NULL);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskKernel,i++,sizeof(float),(void *)&fGain);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskKernel,i++,sizeof(float),(void *)&fThreshold);
      clStatus|=clEnv->library->clSetKernelArg(unsharpMaskKernel,i++,sizeof(cl_uint),(void *)&justBlur);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", "'%s'", ".");
        goto cleanup;
      }
    }

    /* launch the kernel */
    {
      size_t gsize[2];
      size_t wsize[2];

      gsize[0] = ((image->columns + 7) / 8) * 8;
      gsize[1] = ((image->rows + 31) / 32) * 32;
      wsize[0] = 8;
      wsize[1] = 32;

      events=GetOpenCLEvents(image,&event_count);
      clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, unsharpMaskKernel, 2, NULL, gsize, wsize, event_count, events, &event);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
        goto cleanup;
      }
      if (RecordProfileData(clEnv,UnsharpMaskKernel,event) == MagickFalse)
        {
          AddOpenCLEvent(image,event);
          AddOpenCLEvent(filteredImage, event);
        }
      clEnv->library->clReleaseEvent(event);
    }
  }

  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__,__LINE__,exception);

  if (kernel != NULL)			      kernel=DestroyKernelInfo(kernel);
  if (imageKernelBuffer!=NULL)                clEnv->library->clReleaseMemObject(imageKernelBuffer);
  if (unsharpMaskKernel!=NULL)                RelinquishOpenCLKernel(clEnv, unsharpMaskKernel);
  if (queue != NULL)                          RelinquishOpenCLCommandQueue(clEnv, queue);
  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);
  return(filteredImage);
}

MagickPrivate Image *AccelerateUnsharpMaskImage(const Image *image,
  const ChannelType channel,const double radius,const double sigma,
  const double gain,const double threshold,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkOpenCLEnvironment(exception) == MagickFalse) ||
      (checkAccelerateCondition(image, channel) == MagickFalse))
    return NULL;

  if (radius < 12.1)
    filteredImage = ComputeUnsharpMaskImageSingle(image,radius,sigma,gain,threshold, 0, exception);
  else
    filteredImage = ComputeUnsharpMaskImage(image,channel,radius,sigma,gain,threshold,exception);

  return(filteredImage);
}

static Image *ComputeWaveletDenoiseImage(const Image *image,
  const double threshold,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_context
    context;

  cl_int
    clStatus;

  cl_kernel
    denoiseKernel;

  cl_event
    event;

  cl_mem
    filteredImageBuffer,
    imageBuffer;

  const cl_event
    *events;

  Image
    *filteredImage;

  MagickBooleanType
    outputReady;

  MagickCLEnv
    clEnv;

  unsigned int
    event_count,
    i,
    passes;

  clEnv = NULL;
  filteredImage = NULL;
  context = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  denoiseKernel = NULL;
  queue = NULL;
  outputReady = MagickFalse;

  clEnv = GetDefaultOpenCLEnv();

  /* Work around an issue on low end Intel devices */
  if (paramMatchesValue(clEnv,MAGICK_OPENCL_ENV_PARAM_DEVICE_NAME,
      "Intel(R) HD Graphics",exception) != MagickFalse)
    goto cleanup;

  context = GetOpenCLContext(clEnv);
  queue = AcquireOpenCLCommandQueue(clEnv);

  imageBuffer = GetAuthenticOpenCLBuffer(image,exception);
  if (imageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  /* create output */
  filteredImage = CloneImage(image, image->columns, image->rows, MagickTrue, exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", "'%s'", ".");
    goto cleanup;
  }

  filteredImageBuffer = GetAuthenticOpenCLBuffer(filteredImage,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(exception,GetMagickModule(),
      ResourceLimitWarning,"GetAuthenticOpenCLBuffer failed.",".");
    goto cleanup;
  }

  /* get the opencl kernel */
  denoiseKernel = AcquireOpenCLKernel(clEnv, MAGICK_OPENCL_ACCELERATE, "WaveletDenoise");
  if (denoiseKernel == NULL)
  {
    (void)OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", "'%s'", ".");
    goto cleanup;
  };

  // Process image
  {
    int x;
    const int PASSES = 5;
    cl_int width = (cl_int)image->columns;
    cl_int height = (cl_int)image->rows;
    cl_float thresh = threshold;

    passes = (((1.0f * image->columns) * image->rows) + 1999999.0f) / 2000000.0f;
    passes = (passes < 1) ? 1 : passes;

    /* set the kernel arguments */
    i = 0;
    clStatus |= clEnv->library->clSetKernelArg(denoiseKernel, i++, sizeof(cl_mem), (void *)&imageBuffer);
    clStatus |= clEnv->library->clSetKernelArg(denoiseKernel, i++, sizeof(cl_mem), (void *)&filteredImageBuffer);
    clStatus |= clEnv->library->clSetKernelArg(denoiseKernel, i++, sizeof(cl_float), (void *)&thresh);
    clStatus |= clEnv->library->clSetKernelArg(denoiseKernel, i++, sizeof(cl_int), (void *)&PASSES);
    clStatus |= clEnv->library->clSetKernelArg(denoiseKernel, i++, sizeof(cl_int), (void *)&width);
    clStatus |= clEnv->library->clSetKernelArg(denoiseKernel, i++, sizeof(cl_int), (void *)&height);

    for (x = 0; x < passes; ++x)
    {
      const int TILESIZE = 64;
      const int PAD = 1 << (PASSES - 1);
      const int SIZE = TILESIZE - 2 * PAD;

      size_t gsize[2];
      size_t wsize[2];
      size_t goffset[2];

      gsize[0] = ((width + (SIZE - 1)) / SIZE) * TILESIZE;
      gsize[1] = ((((height + (SIZE - 1)) / SIZE) + passes - 1) / passes) * 4;
      wsize[0] = TILESIZE;
      wsize[1] = 4;
      goffset[0] = 0;
      goffset[1] = x * gsize[1];

      events=GetOpenCLEvents(image,&event_count);
      clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, denoiseKernel, 2, goffset, gsize, wsize, event_count, events, &event);
      if (clStatus != CL_SUCCESS)
      {
        (void)OpenCLThrowMagickException(exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", "'%s'", ".");
        goto cleanup;
      }
      clEnv->library->clFlush(queue);
      if (RecordProfileData(clEnv, WaveletDenoiseKernel, event) == MagickFalse)
        {
          AddOpenCLEvent(image, event);
          AddOpenCLEvent(filteredImage, event);
        }
      clEnv->library->clReleaseEvent(event);
    }
  }

  outputReady=MagickTrue;

cleanup:
  OpenCLLogException(__FUNCTION__, __LINE__, exception);

  if (denoiseKernel != NULL)		RelinquishOpenCLKernel(clEnv, denoiseKernel);
  if (queue != NULL)				RelinquishOpenCLCommandQueue(clEnv, queue);
  if ((outputReady == MagickFalse) && (filteredImage != NULL))
    filteredImage=(Image *) DestroyImage(filteredImage);
  return(filteredImage);
}

MagickPrivate Image *AccelerateWaveletDenoiseImage(const Image *image,
  const double threshold,ExceptionInfo *exception)
{
  Image
  *filteredImage;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *)NULL);

  if ((checkAccelerateCondition(image,DefaultChannels) == MagickFalse) ||
      (checkOpenCLEnvironment(exception) == MagickFalse))
    return (Image *) NULL;

  filteredImage=ComputeWaveletDenoiseImage(image,threshold,exception);

  return(filteredImage);
}

#endif /* MAGICKCORE_OPENCL_SUPPORT */
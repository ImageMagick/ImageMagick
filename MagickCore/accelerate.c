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
%                                April 2016                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/studio.h"
#include "MagickCore/accelerate-private.h"
#include "MagickCore/accelerate-kernels-private.h"
#include "MagickCore/artifact.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/color-private.h"
#include "MagickCore/delegate-private.h"
#include "MagickCore/enhance.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/list.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/opencl.h"
#include "MagickCore/opencl-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/prepress.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/random_.h"
#include "MagickCore/random-private.h"
#include "MagickCore/registry.h"
#include "MagickCore/resize.h"
#include "MagickCore/resize-private.h"
#include "MagickCore/semaphore.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"

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
  HannWeightingFunction,
  HammingWeightingFunction,
  BlackmanWeightingFunction,
  CubicBCWeightingFunction,
  SincWeightingFunction,
  SincFastWeightingFunction,
  LastWeightingFunction
};

/*
  Helper functions.
*/
static MagickBooleanType checkAccelerateCondition(const Image* image)
{
  /* check if the image's colorspace is supported */
  if (image->colorspace != RGBColorspace &&
      image->colorspace != sRGBColorspace &&
      image->colorspace != GRAYColorspace)
    return(MagickFalse);

  /* check if the virtual pixel method is compatible with the OpenCL implementation */
  if ((GetImageVirtualPixelMethod(image) != UndefinedVirtualPixelMethod) &&
      (GetImageVirtualPixelMethod(image) != EdgeVirtualPixelMethod))
    return(MagickFalse);

  /* check if the image has read / write mask */
  if (image->read_mask != MagickFalse || image->write_mask != MagickFalse)
    return(MagickFalse);

  if (image->number_channels > 4)
    return(MagickFalse);

  /* check if pixel order is R */
  if (GetPixelChannelOffset(image,RedPixelChannel) != 0)
    return(MagickFalse);

  if (image->number_channels == 1)
    return(MagickTrue);

  /* check if pixel order is RA */
  if ((image->number_channels == 2) &&
      (GetPixelChannelOffset(image,AlphaPixelChannel) == 1))
    return(MagickTrue);

  if (image->number_channels == 2)
    return(MagickFalse);

  /* check if pixel order is RGB */
  if ((GetPixelChannelOffset(image,GreenPixelChannel) != 1) ||
      (GetPixelChannelOffset(image,BluePixelChannel) != 2))
    return(MagickFalse);

  if (image->number_channels == 3)
    return(MagickTrue);

  /* check if pixel order is RGBA */
  if (GetPixelChannelOffset(image,AlphaPixelChannel) != 3)
    return(MagickFalse);

  return(MagickTrue);
}

static MagickBooleanType checkAccelerateConditionRGBA(const Image* image)
{
  if (checkAccelerateCondition(image) == MagickFalse)
    return(MagickFalse);

  /* the order will be RGBA if the image has 4 channels */
  if (image->number_channels != 4)
    return(MagickFalse);

  if ((GetPixelRedTraits(image) == UndefinedPixelTrait) ||
      (GetPixelGreenTraits(image) == UndefinedPixelTrait) ||
      (GetPixelBlueTraits(image) == UndefinedPixelTrait) ||
      (GetPixelAlphaTraits(image) == UndefinedPixelTrait))
    return(MagickFalse);

  return(MagickTrue);
}

static MagickBooleanType checkPixelIntensity(const Image *image,
  const PixelIntensityMethod method)
{
  /* EncodePixelGamma and DecodePixelGamma are not supported */
  if ((method == Rec601LumaPixelIntensityMethod) ||
      (method == Rec709LumaPixelIntensityMethod))
    {
      if (image->colorspace == RGBColorspace)
        return(MagickFalse);
    }

  if ((method == Rec601LuminancePixelIntensityMethod) ||
      (method == Rec709LuminancePixelIntensityMethod))
    {
      if (image->colorspace == sRGBColorspace)
        return(MagickFalse);
    }

  return(MagickTrue);
}

static MagickBooleanType checkHistogramCondition(const Image *image,
  const PixelIntensityMethod method)
{
  /* ensure this is the only pass get in for now. */
  if ((image->channel_mask & SyncChannels) == 0)
    return MagickFalse;

  return(checkPixelIntensity(image,method));
}

static MagickCLEnv getOpenCLEnvironment(ExceptionInfo* exception)
{
  MagickCLEnv
    clEnv;

  clEnv=GetCurrentOpenCLEnv();
  if (clEnv == (MagickCLEnv) NULL)
    return((MagickCLEnv) NULL);

  if (clEnv->enabled == MagickFalse)
    return((MagickCLEnv) NULL);

  if (InitializeOpenCL(clEnv,exception) == MagickFalse)
    return((MagickCLEnv) NULL);

  return(clEnv);
}

static Image *cloneImage(const Image* image,ExceptionInfo *exception)
{
  Image
    *clone;

  if (((image->channel_mask & RedChannel) != 0) &&
      ((image->channel_mask & GreenChannel) != 0) &&
      ((image->channel_mask & BlueChannel) != 0) &&
      ((image->channel_mask & AlphaChannel) != 0))
    clone=CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  else
    {
      clone=CloneImage(image,0,0,MagickTrue,exception);
      if (clone != (Image *) NULL)
        SyncImagePixelCache(clone,exception);
    }
  return(clone);
}

/* pad the global workgroup size to the next multiple of 
   the local workgroup size */
inline static unsigned int padGlobalWorkgroupSizeToLocalWorkgroupSize(
  const unsigned int orgGlobalSize,const unsigned int localGroupSize) 
{
  return ((orgGlobalSize+(localGroupSize-1))/localGroupSize*localGroupSize);
}

static cl_mem createKernelInfo(MagickCLDevice device,const double radius,
  const double sigma,cl_uint *width,ExceptionInfo *exception)
{
  char
    geometry[MagickPathExtent];

  cl_int
    status;

  cl_mem
    imageKernelBuffer;

  float
    *kernelBufferPtr;

  KernelInfo
    *kernel;

  ssize_t
    i;

  (void) FormatLocaleString(geometry,MagickPathExtent,
    "blur:%.20gx%.20g;blur:%.20gx%.20g+90",radius,sigma,radius,sigma);
  kernel=AcquireKernelInfo(geometry,exception);
  if (kernel == (KernelInfo *) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireKernelInfo failed.",".");
    return((cl_mem) NULL);
  }
  kernelBufferPtr=(float *)AcquireMagickMemory(kernel->width*
    sizeof(*kernelBufferPtr));
  for (i = 0; i < (ssize_t) kernel->width; i++)
    kernelBufferPtr[i] = (float)kernel->values[i];
  imageKernelBuffer=CreateOpenCLBuffer(device,CL_MEM_COPY_HOST_PTR |
    CL_MEM_READ_ONLY,kernel->width*sizeof(*kernelBufferPtr),kernelBufferPtr);
  *width=(cl_uint) kernel->width;
  kernelBufferPtr=(float *) RelinquishMagickMemory(kernelBufferPtr);
  kernel=DestroyKernelInfo(kernel);
  if (imageKernelBuffer == (cl_mem) NULL)
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"CreateOpenCLBuffer failed.",".");
  return(imageKernelBuffer);
}

static MagickBooleanType LaunchHistogramKernel(MagickCLEnv clEnv,
  MagickCLDevice device,cl_command_queue queue,cl_mem imageBuffer,
  cl_mem histogramBuffer,Image *image,const ChannelType channel,
  ExceptionInfo *exception)
{
  MagickBooleanType
    outputReady;

  cl_int
    clStatus;

  cl_kernel
    histogramKernel;

  cl_event
    event;

  cl_uint
    colorspace,
    method;

  register ssize_t
    i;

  size_t
    global_work_size[2];

  histogramKernel = NULL; 

  outputReady = MagickFalse;
  colorspace = image->colorspace;
  method = image->intensity;

  /* get the OpenCL kernel */
  histogramKernel = AcquireOpenCLKernel(device,"Histogram");
  if (histogramKernel == NULL)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", ".");
    goto cleanup;
  }

  /* set the kernel arguments */
  i = 0;
  clStatus=clEnv->library->clSetKernelArg(histogramKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(histogramKernel,i++,sizeof(ChannelType),&channel);
  clStatus|=clEnv->library->clSetKernelArg(histogramKernel,i++,sizeof(cl_uint),&colorspace);
  clStatus|=clEnv->library->clSetKernelArg(histogramKernel,i++,sizeof(cl_uint),&method);
  clStatus|=clEnv->library->clSetKernelArg(histogramKernel,i++,sizeof(cl_mem),(void *)&histogramBuffer);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
    goto cleanup;
  }

  /* launch the kernel */
  global_work_size[0] = image->columns;
  global_work_size[1] = image->rows;

  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, histogramKernel, 2, NULL, global_work_size, NULL, 0, NULL, &event);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
    goto cleanup;
  }
  RecordProfileData(device,histogramKernel,event);

  outputReady = MagickTrue;

cleanup:
 
  if (histogramKernel!=NULL)
    ReleaseOpenCLKernel(histogramKernel);

  return(outputReady);
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

static Image *ComputeAddNoiseImage(const Image *image,MagickCLEnv clEnv,
  const NoiseType noise_type,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_float
    attenuate;

  cl_int
    status;

  cl_kernel
    addNoiseKernel;

  cl_mem
    filteredImageBuffer,
    imageBuffer;

  cl_uint
    bufferLength,
    inputPixelCount,
    number_channels,
    numRandomNumberPerPixel,
    pixelsPerWorkitem,
    seed0,
    seed1,
    workItemCount;

  const char
    *option;

  const unsigned long
    *s;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  Image
    *filteredImage;

  RandomInfo
    *randomInfo;

  size_t
    gsize[1],
    i,
    lsize[1],
    numRandPerChannel;

  filteredImage=NULL;
  addNoiseKernel=NULL;
  outputReady=MagickFalse;

  device=RequestOpenCLDevice(clEnv);
  queue=AcquireOpenCLCommandQueue(device);
  if (queue == (cl_command_queue) NULL)
    goto cleanup;
  filteredImage=cloneImage(image,exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;
  if (filteredImage->number_channels != image->number_channels)
    goto cleanup;
  imageBuffer=GetAuthenticOpenCLBuffer(image,device,exception);
  if (imageBuffer == (cl_mem) NULL)
    goto cleanup;
  filteredImageBuffer=GetAuthenticOpenCLBuffer(filteredImage,device,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
    goto cleanup;

  /* find out how many random numbers needed by pixel */
  numRandPerChannel=0;
  numRandomNumberPerPixel=0;
  switch (noise_type)
  {
    case UniformNoise:
    case ImpulseNoise:
    case LaplacianNoise:
    case RandomNoise:
    default:
      numRandPerChannel=1;
      break;
    case GaussianNoise:
    case MultiplicativeGaussianNoise:
    case PoissonNoise:
      numRandPerChannel=2;
      break;
  };
  if (GetPixelRedTraits(image) != UndefinedPixelTrait)
    numRandomNumberPerPixel+=(cl_uint) numRandPerChannel;
  if (GetPixelGreenTraits(image) != UndefinedPixelTrait)
    numRandomNumberPerPixel+=(cl_uint) numRandPerChannel;
  if (GetPixelBlueTraits(image) != UndefinedPixelTrait)
    numRandomNumberPerPixel+=(cl_uint) numRandPerChannel;
  if (GetPixelAlphaTraits(image) != UndefinedPixelTrait)
    numRandomNumberPerPixel+=(cl_uint) numRandPerChannel;

  addNoiseKernel=AcquireOpenCLKernel(device,"AddNoise");
  if (addNoiseKernel == (cl_kernel) NULL)
  {
    (void)OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  /* 256 work items per group, 2 groups per CU */
  workItemCount=device->max_compute_units*2*256;
  inputPixelCount=(cl_int) (image->columns*image->rows);
  pixelsPerWorkitem=(inputPixelCount+workItemCount-1)/workItemCount;
  pixelsPerWorkitem=((pixelsPerWorkitem+3)/4)*4;
  lsize[0]=256;
  gsize[0]=workItemCount;

  randomInfo=AcquireRandomInfo();
  s=GetRandomInfoSeed(randomInfo);
  seed0=s[0];
  (void) GetPseudoRandomValue(randomInfo);
  seed1=s[0];
  randomInfo=DestroyRandomInfo(randomInfo);

  number_channels=(cl_uint) image->number_channels;
  bufferLength=(cl_uint) (image->columns*image->rows*image->number_channels);
  attenuate=1.0f;
  option=GetImageArtifact(image,"attenuate");
  if (option != (char *) NULL)
    attenuate=(float)StringToDouble(option,(char **) NULL);

  i=0;
  status =SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  status|=SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(cl_uint),(void *)&number_channels);
  status|=SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(ChannelType),(void *)&image->channel_mask);
  status|=SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(cl_uint),(void *)&bufferLength);
  status|=SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(cl_uint),(void *)&pixelsPerWorkitem);
  status|=SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(NoiseType),(void *)&noise_type);
  status|=SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(cl_float),(void *)&attenuate);
  status|=SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(cl_uint),(void *)&seed0);
  status|=SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(cl_uint),(void *)&seed1);
  status|=SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(cl_uint),(void *)&numRandomNumberPerPixel);
  status|=SetOpenCLKernelArg(addNoiseKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"clSetKernelArg failed.",".");
    goto cleanup;
  }

  outputReady=EnqueueOpenCLKernel(queue,addNoiseKernel,1,(const size_t *) NULL,gsize,
    lsize,image,filteredImage,MagickFalse,exception);

cleanup:

  if (addNoiseKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(addNoiseKernel);
  if (queue != (cl_command_queue) NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != (MagickCLDevice) NULL)
    ReleaseOpenCLDevice(device);
  if ((outputReady == MagickFalse) && (filteredImage != (Image *) NULL))
    filteredImage=DestroyImage(filteredImage);

  return(filteredImage);
}

MagickPrivate Image *AccelerateAddNoiseImage(const Image *image,
  const NoiseType noise_type,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateCondition(image) == MagickFalse)
    return((Image *) NULL);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return((Image *) NULL);

  filteredImage=ComputeAddNoiseImage(image,clEnv,noise_type,exception);
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

static Image *ComputeBlurImage(const Image* image,MagickCLEnv clEnv,
  const double radius,const double sigma,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_int
    status;

  cl_kernel
    blurColumnKernel,
    blurRowKernel;

  cl_mem
    filteredImageBuffer,
    imageBuffer,
    imageKernelBuffer,
    tempImageBuffer;

  cl_uint
    imageColumns,
    imageRows,
    kernelWidth,
    number_channels;

  Image
    *filteredImage;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  MagickSizeType
    length;

  size_t
    chunkSize=256,
    gsize[2],
    i,
    lsize[2];

  filteredImage=NULL;
  tempImageBuffer=NULL;
  imageKernelBuffer=NULL;
  blurRowKernel=NULL;
  blurColumnKernel=NULL;
  outputReady=MagickFalse;

  device=RequestOpenCLDevice(clEnv);
  queue=AcquireOpenCLCommandQueue(device);
  filteredImage=cloneImage(image,exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;
  if (filteredImage->number_channels != image->number_channels)
    goto cleanup;
  imageBuffer=GetAuthenticOpenCLBuffer(image,device,exception);
  if (imageBuffer == (cl_mem) NULL)
    goto cleanup;
  filteredImageBuffer=GetAuthenticOpenCLBuffer(filteredImage,device,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
    goto cleanup;

  imageKernelBuffer=createKernelInfo(device,radius,sigma,&kernelWidth,
    exception);
  if (imageKernelBuffer == (cl_mem) NULL)
    goto cleanup;

  length=image->columns*image->rows;
  tempImageBuffer=CreateOpenCLBuffer(device,CL_MEM_READ_WRITE,length*
    sizeof(cl_float4),(void *) NULL);
  if (tempImageBuffer == (cl_mem) NULL)
    goto cleanup;

  blurRowKernel=AcquireOpenCLKernel(device,"BlurRow");
  if (blurRowKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  number_channels=(cl_uint) image->number_channels;
  imageColumns=(cl_uint) image->columns;
  imageRows=(cl_uint) image->rows;

  i=0;
  status =SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_uint),&number_channels);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(ChannelType),&image->channel_mask);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_uint),(void *)&kernelWidth);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_uint),(void *)&imageColumns);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_uint),(void *)&imageRows);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_float4)*(chunkSize+kernelWidth),(void *) NULL);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"SetOpenCLKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=chunkSize*((image->columns+chunkSize-1)/chunkSize);
  gsize[1]=image->rows;
  lsize[0]=chunkSize;
  lsize[1]=1;

  outputReady=EnqueueOpenCLKernel(queue,blurRowKernel,2,(size_t *) NULL,gsize,
    lsize,image,filteredImage,MagickFalse,exception);
  if (outputReady == MagickFalse)
    goto cleanup;

  blurColumnKernel=AcquireOpenCLKernel(device,"BlurColumn");
  if (blurColumnKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  i=0;
  status =SetOpenCLKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
  status|=SetOpenCLKernelArg(blurColumnKernel,i++,sizeof(cl_uint),&number_channels);
  status|=SetOpenCLKernelArg(blurColumnKernel,i++,sizeof(ChannelType),&image->channel_mask);
  status|=SetOpenCLKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
  status|=SetOpenCLKernelArg(blurColumnKernel,i++,sizeof(cl_uint),(void *)&kernelWidth);
  status|=SetOpenCLKernelArg(blurColumnKernel,i++,sizeof(cl_uint),(void *)&imageColumns);
  status|=SetOpenCLKernelArg(blurColumnKernel,i++,sizeof(cl_uint),(void *)&imageRows);
  status|=SetOpenCLKernelArg(blurColumnKernel,i++,sizeof(cl_float4)*(chunkSize+kernelWidth),(void *) NULL);
  status|=SetOpenCLKernelArg(blurColumnKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"SetOpenCLKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=image->columns;
  gsize[1]=chunkSize*((image->rows+chunkSize-1)/chunkSize);
  lsize[0]=1;
  lsize[1]=chunkSize;

  outputReady=EnqueueOpenCLKernel(queue,blurColumnKernel,2,(size_t *) NULL,gsize,
    lsize,image,filteredImage,MagickFalse,exception);

cleanup:

  if (tempImageBuffer != (cl_mem) NULL)
    ReleaseOpenCLMemObject(tempImageBuffer);
  if (imageKernelBuffer != (cl_mem) NULL)
    ReleaseOpenCLMemObject(imageKernelBuffer);
  if (blurRowKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(blurRowKernel);
  if (blurColumnKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(blurColumnKernel);
  if (queue != (cl_command_queue) NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != (MagickCLDevice) NULL)
    ReleaseOpenCLDevice(device);
  if ((outputReady == MagickFalse) && (filteredImage != (Image *) NULL))
    filteredImage=DestroyImage(filteredImage);

  return(filteredImage);
}

MagickPrivate Image* AccelerateBlurImage(const Image *image,
  const double radius,const double sigma,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateCondition(image) == MagickFalse)
    return((Image *) NULL);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return((Image *) NULL);

  filteredImage=ComputeBlurImage(image,clEnv,radius,sigma,exception);
  return(filteredImage);
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

static MagickBooleanType ComputeContrastImage(Image *image,MagickCLEnv clEnv,
  const MagickBooleanType sharpen,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_int
    status,
    sign;

  cl_kernel
    contrastKernel;

  cl_event
    event;

  cl_mem
    imageBuffer;

  cl_uint
    number_channels;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  size_t
    gsize[2],
    i;

  contrastKernel=NULL;
  outputReady=MagickFalse;

  device=RequestOpenCLDevice(clEnv);
  queue=AcquireOpenCLCommandQueue(device);
  imageBuffer=GetAuthenticOpenCLBuffer(image,device,exception);
  if (imageBuffer == (cl_mem) NULL)
    goto cleanup;

  contrastKernel=AcquireOpenCLKernel(device,"Contrast");
  if (contrastKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  number_channels=(cl_uint) image->number_channels;
  sign=sharpen != MagickFalse ? 1 : -1;

  i=0;
  status =SetOpenCLKernelArg(contrastKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  status|=SetOpenCLKernelArg(contrastKernel,i++,sizeof(cl_uint),&number_channels);
  status|=SetOpenCLKernelArg(contrastKernel,i++,sizeof(cl_int),&sign);
  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"SetOpenCLKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=image->columns;
  gsize[1]=image->rows;

  outputReady=EnqueueOpenCLKernel(queue,contrastKernel,2,(const size_t *) NULL,
    gsize,(const size_t *) NULL,image,(Image *) NULL,MagickFalse,exception);

cleanup:

  if (contrastKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(contrastKernel);
  if (queue != (cl_command_queue) NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != (MagickCLDevice) NULL)
    ReleaseOpenCLDevice(device);

  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateContrastImage(Image *image,
  const MagickBooleanType sharpen,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateCondition(image) == MagickFalse)
    return(MagickFalse);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return(MagickFalse);

  status=ComputeContrastImage(image,clEnv,sharpen,exception);
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

static MagickBooleanType ComputeContrastStretchImage(Image *image,
  MagickCLEnv clEnv,const double black_point,const double white_point,
  ExceptionInfo *exception)
{
#define ContrastStretchImageTag  "ContrastStretch/Image"
#define MaxRange(color)  ((cl_float) ScaleQuantumToMap((Quantum) (color)))

  CacheView
    *image_view;

  cl_command_queue
    queue;

  cl_int
    clStatus;

  cl_mem_flags
    mem_flags;

  cl_mem
    histogramBuffer,
    imageBuffer,
    stretchMapBuffer;

  cl_kernel
    histogramKernel,
    stretchKernel;

  cl_event
    event;

  cl_uint4
    *histogram;

  double
    intensity;

  cl_float4
    black,
    white;

  MagickBooleanType
    outputReady,
    status;

  MagickCLDevice
    device;

  MagickSizeType
    length;

  PixelPacket
    *stretch_map;

  register ssize_t
    i;

  size_t
    global_work_size[2];

  void
    *hostPtr,
    *inputPixels;

  histogram=NULL;
  stretch_map=NULL;
  inputPixels = NULL;
  imageBuffer = NULL;
  histogramBuffer = NULL;
  stretchMapBuffer = NULL;
  histogramKernel = NULL; 
  stretchKernel = NULL; 
  queue = NULL;
  outputReady = MagickFalse;


  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  //exception=(&image->exception);

  /*
   * initialize opencl env
   */
  device = RequestOpenCLDevice(clEnv);
  queue = AcquireOpenCLCommandQueue(device);

  /*
    Allocate and initialize histogram arrays.
  */
  histogram=(cl_uint4 *) AcquireQuantumMemory(MaxMap+1UL, sizeof(*histogram));

  if (histogram == (cl_uint4 *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed", image->filename);
 
  /* reset histogram */
  (void) ResetMagickMemory(histogram,0,(MaxMap+1)*sizeof(*histogram));

  /*
  if (IsGrayImage(image,exception) != MagickFalse)
    (void) SetImageColorspace(image,GRAYColorspace);
  */

  status=MagickTrue;


  /*
    Form histogram.
  */
  /* Create and initialize OpenCL buffers. */
  /* inputPixels = AcquirePixelCachePixels(image, &length, exception); */
  /* assume this  will get a writable image */
  image_view=AcquireAuthenticCacheView(image,exception);
  inputPixels=GetCacheViewAuthenticPixels(image_view,0,0,image->columns,image->rows,exception);

  if (inputPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",image->filename);
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
  length = image->columns * image->rows;
  imageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
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
  histogramBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(cl_uint4), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  status = LaunchHistogramKernel(clEnv, device, queue, imageBuffer, histogramBuffer, image, image->channel_mask,exception);
  if (status == MagickFalse)
    goto cleanup;

  /* read from the kenel output */
  if (ALIGNED(histogram,cl_uint4)) 
  {
    length = (MaxMap+1); 
    clEnv->library->clEnqueueMapBuffer(queue, histogramBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(cl_uint4), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = (MaxMap+1); 
    clStatus = clEnv->library->clEnqueueReadBuffer(queue, histogramBuffer, CL_TRUE, 0, length * sizeof(cl_uint4), histogram, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", ".");
    goto cleanup;
  }

  /* unmap, don't block gpu to use this buffer again.  */
  if (ALIGNED(histogram,cl_uint4))
  {
    clStatus = clEnv->library->clEnqueueUnmapMemObject(queue, histogramBuffer, histogram, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueUnmapMemObject failed.", ".");
      goto cleanup;
    }
  }

  /* recreate input buffer later, in case image updated */
#ifdef RECREATEBUFFER 
  if (imageBuffer!=NULL)		      
    clEnv->library->clReleaseMemObject(imageBuffer);
#endif

  /* CPU stuff */
  /*
     Find the histogram boundaries by locating the black/white levels.
  */
  black.x=0.0;
  white.x=MaxRange(QuantumRange);
  if ((image->channel_mask & RedChannel) != 0)
  {
    intensity=0.0;
    for (i=0; i <= (ssize_t) MaxMap; i++)
    {
      intensity+=histogram[i].s[2];
      if (intensity > black_point)
        break;
    }
    black.x=(cl_float) i;
    intensity=0.0;
    for (i=(ssize_t) MaxMap; i != 0; i--)
    {
      intensity+=histogram[i].s[2];
      if (intensity > ((double) image->columns*image->rows-white_point))
        break;
    }
    white.x=(cl_float) i;
  }
  black.y=0.0;
  white.y=MaxRange(QuantumRange);
  if ((image->channel_mask & GreenChannel) != 0)
  {
    intensity=0.0;
    for (i=0; i <= (ssize_t) MaxMap; i++)
    {
      intensity+=histogram[i].s[2];
      if (intensity > black_point)
        break;
    }
    black.y=(cl_float) i;
    intensity=0.0;
    for (i=(ssize_t) MaxMap; i != 0; i--)
    {
      intensity+=histogram[i].s[2];
      if (intensity > ((double) image->columns*image->rows-white_point))
        break;
    }
    white.y=(cl_float) i;
  }
  black.z=0.0;
  white.z=MaxRange(QuantumRange);
  if ((image->channel_mask & BlueChannel) != 0)
  {
    intensity=0.0;
    for (i=0; i <= (ssize_t) MaxMap; i++)
    {
      intensity+=histogram[i].s[2];
      if (intensity > black_point)
        break;
    }
    black.z=(cl_float) i;
    intensity=0.0;
    for (i=(ssize_t) MaxMap; i != 0; i--)
    {
      intensity+=histogram[i].s[2];
      if (intensity > ((double) image->columns*image->rows-white_point))
        break;
    }
    white.z=(cl_float) i;
  }
  black.w=0.0;
  white.w=MaxRange(QuantumRange);
  if ((image->channel_mask & AlphaChannel) != 0)
  {
    intensity=0.0;
    for (i=0; i <= (ssize_t) MaxMap; i++)
    {
      intensity+=histogram[i].s[2];
      if (intensity > black_point)
        break;
    }
    black.w=(cl_float) i;
    intensity=0.0;
    for (i=(ssize_t) MaxMap; i != 0; i--)
    {
      intensity+=histogram[i].s[2];
      if (intensity > ((double) image->columns*image->rows-white_point))
        break;
    }
    white.w=(cl_float) i;
  }

  stretch_map=(PixelPacket *) AcquireQuantumMemory(MaxMap+1UL,
    sizeof(*stretch_map));

  if (stretch_map == (PixelPacket *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
 
  /*
    Stretch the histogram to create the stretched image mapping.
  */
  (void) ResetMagickMemory(stretch_map,0,(MaxMap+1)*sizeof(*stretch_map));
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if ((image->channel_mask & RedChannel) != 0)
    {
      if (i < (ssize_t) black.x)
        stretch_map[i].red=(Quantum) 0;
      else
        if (i > (ssize_t) white.x)
          stretch_map[i].red=QuantumRange;
        else
          if (black.x != white.x)
            stretch_map[i].red=ScaleMapToQuantum((MagickRealType) (MaxMap*
                  (i-black.x)/(white.x-black.x)));
    }
    if ((image->channel_mask & GreenChannel) != 0)
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
    if ((image->channel_mask & BlueChannel) != 0)
    {
      if (i < (ssize_t) black.z)
        stretch_map[i].blue=0;
      else
        if (i > (ssize_t) white.z)
          stretch_map[i].blue= QuantumRange;
        else
          if (black.z != white.z)
            stretch_map[i].blue=ScaleMapToQuantum((MagickRealType) (MaxMap*
                  (i-black.z)/(white.z-black.z)));
    }
    if ((image->channel_mask & AlphaChannel) != 0)
    {
      if (i < (ssize_t) black.w)
        stretch_map[i].alpha=0;
      else
        if (i > (ssize_t) white.w)
          stretch_map[i].alpha=QuantumRange;
        else
          if (black.w != white.w)
            stretch_map[i].alpha=ScaleMapToQuantum((MagickRealType) (MaxMap*
                  (i-black.w)/(white.w-black.w)));
    }
  }

  /*
    Stretch the image.
  */
  if (((image->channel_mask & AlphaChannel) != 0) || (((image->channel_mask & IndexChannel) != 0) &&
      (image->colorspace == CMYKColorspace)))
    image->storage_class=DirectClass;
  if (image->storage_class == PseudoClass)
  {
    /*
       Stretch colormap.
       */
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      if ((image->channel_mask & RedChannel) != 0)
      {
        if (black.x != white.x)
          image->colormap[i].red=stretch_map[
            ScaleQuantumToMap(image->colormap[i].red)].red;
      }
      if ((image->channel_mask & GreenChannel) != 0)
      {
        if (black.y != white.y)
          image->colormap[i].green=stretch_map[
            ScaleQuantumToMap(image->colormap[i].green)].green;
      }
      if ((image->channel_mask & BlueChannel) != 0)
      {
        if (black.z != white.z)
          image->colormap[i].blue=stretch_map[
            ScaleQuantumToMap(image->colormap[i].blue)].blue;
      }
      if ((image->channel_mask & AlphaChannel) != 0)
      {
        if (black.w != white.w)
          image->colormap[i].alpha=stretch_map[
            ScaleQuantumToMap(image->colormap[i].alpha)].alpha;
      }
    }
  }

  /*
    Stretch image.
  */


  /* GPU can work on this again, image and equalize map as input
    image:        uchar4 (CLPixelPacket)
    stretch_map:  uchar4 (PixelPacket)
    black, white: float4 (FloatPixelPacket) */

#ifdef RECREATEBUFFER 
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
  length = image->columns * image->rows;
  imageBuffer = clEnv->library->clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }
#endif

  /* Create and initialize OpenCL buffers. */
  if (ALIGNED(stretch_map, PixelPacket)) 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
    hostPtr = stretch_map;
  }
  else 
  {
    mem_flags = CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR;
    hostPtr = stretch_map;
  }
  /* create a CL buffer for stretch_map  */
  length = (MaxMap+1); 
  stretchMapBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(PixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  /* get the OpenCL kernel */
  stretchKernel = AcquireOpenCLKernel(device,"ContrastStretch");
  if (stretchKernel == NULL)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", ".");
    goto cleanup;
  }

  /* set the kernel arguments */
  i = 0;
  clStatus=clEnv->library->clSetKernelArg(stretchKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(stretchKernel,i++,sizeof(ChannelType),&image->channel_mask);
  clStatus|=clEnv->library->clSetKernelArg(stretchKernel,i++,sizeof(cl_mem),(void *)&stretchMapBuffer);
  clStatus|=clEnv->library->clSetKernelArg(stretchKernel,i++,sizeof(cl_float4),&white);
  clStatus|=clEnv->library->clSetKernelArg(stretchKernel,i++,sizeof(cl_float4),&black);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
    goto cleanup;
  }

  /* launch the kernel */
  global_work_size[0] = image->columns;
  global_work_size[1] = image->rows;

  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, stretchKernel, 2, NULL, global_work_size, NULL, 0, NULL, &event);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
    goto cleanup;
  }
  RecordProfileData(device,stretchKernel,event);

  /* read the data back */
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    length = image->columns * image->rows;
    clEnv->library->clEnqueueMapBuffer(queue, imageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = image->columns * image->rows;
    clStatus = clEnv->library->clEnqueueReadBuffer(queue, imageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), inputPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", ".");
    goto cleanup;
  }

  outputReady=SyncCacheViewAuthenticPixels(image_view,exception);

cleanup:

  image_view=DestroyCacheView(image_view);

  if (imageBuffer!=NULL)		      
    clEnv->library->clReleaseMemObject(imageBuffer);

  if (stretchMapBuffer!=NULL)
    clEnv->library->clReleaseMemObject(stretchMapBuffer);
  if (stretch_map!=NULL)
    stretch_map=(PixelPacket *) RelinquishMagickMemory(stretch_map);
  if (histogramBuffer!=NULL)
    clEnv->library->clReleaseMemObject(histogramBuffer);
  if (histogram!=NULL)
    histogram=(cl_uint4 *) RelinquishMagickMemory(histogram);
  if (histogramKernel!=NULL)
    ReleaseOpenCLKernel(histogramKernel);
  if (stretchKernel!=NULL)
    ReleaseOpenCLKernel(stretchKernel);
  if (queue != NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != NULL)
    ReleaseOpenCLDevice(device);

  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateContrastStretchImage(
  Image *image,const double black_point,const double white_point,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkAccelerateConditionRGBA(image) == MagickFalse) ||
      (checkHistogramCondition(image,image->intensity) == MagickFalse))
    return(MagickFalse);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return(MagickFalse);

  status=ComputeContrastStretchImage(image,clEnv,black_point,white_point,
    exception);
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

static Image *ComputeConvolveImage(const Image* image,MagickCLEnv clEnv,
  const KernelInfo *kernel,ExceptionInfo *exception)
{
  CacheView
    *filteredImage_view,
    *image_view;

  cl_command_queue
    queue;

  cl_event
    event;

  cl_kernel
    clkernel;

  cl_int
    clStatus;

  cl_mem
    convolutionKernel,
    filteredImageBuffer,
    imageBuffer;

  cl_mem_flags
    mem_flags;

  const void
    *inputPixels;

  float
    *kernelBufferPtr;

  Image
    *filteredImage;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  MagickSizeType
    length;

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

  void
    *filteredPixels,
    *hostPtr;

  /* intialize all CL objects to NULL */
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  convolutionKernel = NULL;
  clkernel = NULL;
  queue = NULL;

  filteredImage = NULL;
  filteredImage_view = NULL;
  outputReady = MagickFalse;

  device = RequestOpenCLDevice(clEnv);

  image_view=AcquireAuthenticCacheView(image,exception);
  inputPixels=GetCacheViewAuthenticPixels(image_view,0,0,image->columns,image->rows,exception);
  if (inputPixels == (const void *) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",image->filename);
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
  length = image->columns * image->rows;
  imageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  filteredImage = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass,exception) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", ".");
    goto cleanup;
  }
  filteredImage_view=AcquireAuthenticCacheView(filteredImage,exception);
  filteredPixels=GetCacheViewAuthenticPixels(filteredImage_view,0,0,filteredImage->columns,filteredImage->rows,exception);
  if (filteredPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
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
  length = image->columns * image->rows;
  filteredImageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  kernelSize = (unsigned int) (kernel->width * kernel->height);
  convolutionKernel = clEnv->library->clCreateBuffer(device->context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, kernelSize * sizeof(float), NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  queue = AcquireOpenCLCommandQueue(device);

  kernelBufferPtr = (float*)clEnv->library->clEnqueueMapBuffer(queue, convolutionKernel, CL_TRUE, CL_MAP_WRITE, 0, kernelSize * sizeof(float)
          , 0, NULL, NULL, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueMapBuffer failed.",".");
    goto cleanup;
  }
  for (i = 0; i < kernelSize; i++)
  {
    kernelBufferPtr[i] = (float) kernel->values[i];
  }
  clStatus = clEnv->library->clEnqueueUnmapMemObject(queue, convolutionKernel, kernelBufferPtr, 0, NULL, NULL);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueUnmapMemObject failed.", ".");
    goto cleanup;
  }

  /* Compute the local memory requirement for a 16x16 workgroup.
     If it's larger than 16k, reduce the workgroup size to 8x8 */
  localGroupSize[0] = 16;
  localGroupSize[1] = 16;
  localMemoryRequirement = (localGroupSize[0]+kernel->width-1) * (localGroupSize[1]+kernel->height-1) * sizeof(CLPixelPacket)
    + kernel->width*kernel->height*sizeof(float);

  if (localMemoryRequirement > device->local_memory_size)
  {
    localGroupSize[0] = 8;
    localGroupSize[1] = 8;
    localMemoryRequirement = (localGroupSize[0]+kernel->width-1) * (localGroupSize[1]+kernel->height-1) * sizeof(CLPixelPacket)
      + kernel->width*kernel->height*sizeof(float);
  }
  if (localMemoryRequirement <= device->local_memory_size)
  {
    /* get the OpenCL kernel */
    clkernel = AcquireOpenCLKernel(device,"ConvolveOptimized");
    if (clkernel == NULL)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", ".");
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
    matte = (image->alpha_trait > CopyPixelTrait)?1:0;
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&matte);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(ChannelType),(void *)&image->channel_mask);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++, (localGroupSize[0] + kernel->width-1)*(localGroupSize[1] + kernel->height-1)*sizeof(CLPixelPacket),NULL);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++, kernel->width*kernel->height*sizeof(float),NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
      goto cleanup;
    }

    /* pad the global size to a multiple of the local work size dimension */
    global_work_size[0] = ((image->columns + localGroupSize[0]  - 1)/localGroupSize[0] ) * localGroupSize[0] ;
    global_work_size[1] = ((image->rows + localGroupSize[1] - 1)/localGroupSize[1]) * localGroupSize[1];

    /* launch the kernel */
    clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, clkernel, 2, NULL, global_work_size, localGroupSize, 0, NULL, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
    RecordProfileData(device,clkernel,event);
  }
  else
  {
    /* get the OpenCL kernel */
    clkernel = AcquireOpenCLKernel(device,"Convolve");
    if (clkernel == NULL)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", ".");
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
    matte = (image->alpha_trait > CopyPixelTrait)?1:0;
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(unsigned int),(void *)&matte);
    clStatus|=clEnv->library->clSetKernelArg(clkernel,i++,sizeof(ChannelType),(void *)&image->channel_mask);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
      goto cleanup;
    }

    localGroupSize[0] = 8;
    localGroupSize[1] = 8;
    global_work_size[0] = (image->columns + (localGroupSize[0]-1))/localGroupSize[0] * localGroupSize[0];
    global_work_size[1] = (image->rows    + (localGroupSize[1]-1))/localGroupSize[1] * localGroupSize[1];
	clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, clkernel, 2, NULL, global_work_size, localGroupSize, 0, NULL, &event);
    
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
  }
  RecordProfileData(device,clkernel,event);

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = image->columns * image->rows;
    clEnv->library->clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = image->columns * image->rows;
    clStatus = clEnv->library->clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", ".");
    goto cleanup;
  }

  outputReady=SyncCacheViewAuthenticPixels(filteredImage_view,exception);

cleanup:

  image_view=DestroyCacheView(image_view);
  if (filteredImage_view != NULL)
    filteredImage_view=DestroyCacheView(filteredImage_view);
  if (imageBuffer != NULL)
    clEnv->library->clReleaseMemObject(imageBuffer);
  if (filteredImageBuffer != NULL)
    clEnv->library->clReleaseMemObject(filteredImageBuffer);
  if (convolutionKernel != NULL)
    clEnv->library->clReleaseMemObject(convolutionKernel);
  if (clkernel != NULL)
    ReleaseOpenCLKernel(clkernel);
  if (queue != NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != NULL)
    ReleaseOpenCLDevice(device);
  if (outputReady == MagickFalse)
  {
    if (filteredImage != NULL)
    {
      DestroyImage(filteredImage);
      filteredImage = NULL;
    }
  }

  return(filteredImage);
}

MagickPrivate Image *AccelerateConvolveImage(const Image *image,
  const KernelInfo *kernel,ExceptionInfo *exception)
{
  /* Temporary disabled due to access violation

  Image
    *filteredImage;

  assert(image != NULL);
  assert(kernel != (KernelInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  if ((checkAccelerateConditionRGBA(image) == MagickFalse) ||
      (checkOpenCLEnvironment(exception) == MagickFalse))
    return((Image *) NULL);

  filteredImage=ComputeConvolveImage(image,kernel,exception);
  return(filteredImage);
  */
  magick_unreferenced(image);
  magick_unreferenced(kernel);
  magick_unreferenced(exception);
  return((Image *)NULL);
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

static Image *ComputeDespeckleImage(const Image *image,MagickCLEnv clEnv,
  ExceptionInfo*exception)
{
  static const int 
    X[4] = {0, 1, 1,-1},
    Y[4] = {1, 0, 1, 1};

  CacheView
    *filteredImage_view,
    *image_view;

  cl_command_queue
    queue;

  cl_int
    clStatus;

  cl_kernel
    hullPass1,
    hullPass2;

  cl_event
    event;

  cl_mem_flags
    mem_flags;

  cl_mem
    filteredImageBuffer,
    imageBuffer,
    tempImageBuffer[2];

  const void
    *inputPixels;

  Image
    *filteredImage;

  int
    k,
    matte;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  MagickSizeType
    length;

  size_t
    global_work_size[2];

  unsigned int
    imageHeight,
    imageWidth;

  void
    *filteredPixels,
    *hostPtr;

  outputReady = MagickFalse;
  inputPixels = NULL;
  filteredImage = NULL;
  filteredImage_view = NULL;
  filteredPixels = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  hullPass1 = NULL;
  hullPass2 = NULL;
  queue = NULL;
  tempImageBuffer[0] = tempImageBuffer[1] = NULL;

  device = RequestOpenCLDevice(clEnv);
  queue = AcquireOpenCLCommandQueue(device);
 
  image_view=AcquireAuthenticCacheView(image,exception);
  inputPixels=GetCacheViewAuthenticPixels(image_view,0,0,image->columns,image->rows,exception);
  if (inputPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",image->filename);
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
  length = image->columns * image->rows;
  imageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  mem_flags = CL_MEM_READ_WRITE;
  length = image->columns * image->rows;
  for (k = 0; k < 2; k++)
  {
    tempImageBuffer[k] = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(CLPixelPacket), NULL, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  filteredImage = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass,exception) != MagickTrue)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", ".");
    goto cleanup;
  }
  filteredImage_view=AcquireAuthenticCacheView(filteredImage,exception);
  filteredPixels=GetCacheViewAuthenticPixels(filteredImage_view,0,0,filteredImage->columns,filteredImage->rows,exception);
  if (filteredPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
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
  length = image->columns * image->rows;
  filteredImageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  hullPass1 = AcquireOpenCLKernel(device,"HullPass1");
  hullPass2 = AcquireOpenCLKernel(device,"HullPass2");

  clStatus =clEnv->library->clSetKernelArg(hullPass1,0,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus |=clEnv->library->clSetKernelArg(hullPass1,1,sizeof(cl_mem),(void *)(tempImageBuffer+1));
  imageWidth = (unsigned int) image->columns;
  clStatus |=clEnv->library->clSetKernelArg(hullPass1,2,sizeof(unsigned int),(void *)&imageWidth);
  imageHeight = (unsigned int) image->rows;
  clStatus |=clEnv->library->clSetKernelArg(hullPass1,3,sizeof(unsigned int),(void *)&imageHeight);
  matte = (image->alpha_trait > CopyPixelTrait)?1:0;
  clStatus |=clEnv->library->clSetKernelArg(hullPass1,6,sizeof(int),(void *)&matte);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
    goto cleanup;
  }

  clStatus = clEnv->library->clSetKernelArg(hullPass2,0,sizeof(cl_mem),(void *)(tempImageBuffer+1));
  clStatus |=clEnv->library->clSetKernelArg(hullPass2,1,sizeof(cl_mem),(void *)tempImageBuffer);
  imageWidth = (unsigned int) image->columns;
  clStatus |=clEnv->library->clSetKernelArg(hullPass2,2,sizeof(unsigned int),(void *)&imageWidth);
  imageHeight = (unsigned int) image->rows;
  clStatus |=clEnv->library->clSetKernelArg(hullPass2,3,sizeof(unsigned int),(void *)&imageHeight);
  matte = (image->alpha_trait > CopyPixelTrait)?1:0;
  clStatus |=clEnv->library->clSetKernelArg(hullPass2,6,sizeof(int),(void *)&matte);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
    goto cleanup;
  }


  global_work_size[0] = image->columns;
  global_work_size[1] = image->rows;

  
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
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
      goto cleanup;
    }
    /* launch the kernel */
	clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, 0, NULL, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
    RecordProfileData(device,hullPass1,event);

    /* launch the kernel */
	clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, 0, NULL, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
    RecordProfileData(device,hullPass2,event);

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
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
      goto cleanup;
    }
    /* launch the kernel */
	clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, 0, NULL, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
    RecordProfileData(device,hullPass1,event);

    /* launch the kernel */
	clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, 0, NULL, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
    RecordProfileData(device,hullPass2,event);

    offset.s[0] = -X[k];
    offset.s[1] = -Y[k];
    polarity = -1;
    clStatus = clEnv->library->clSetKernelArg(hullPass1,4,sizeof(cl_int2),(void *)&offset);
    clStatus|= clEnv->library->clSetKernelArg(hullPass1,5,sizeof(int),(void *)&polarity);
    clStatus|=clEnv->library->clSetKernelArg(hullPass2,4,sizeof(cl_int2),(void *)&offset);
    clStatus|=clEnv->library->clSetKernelArg(hullPass2,5,sizeof(int),(void *)&polarity);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
      goto cleanup;
    }
    /* launch the kernel */
	clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, 0, NULL, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
    RecordProfileData(device,hullPass1,event);

    /* launch the kernel */
	clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, 0, NULL, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
    RecordProfileData(device,hullPass2,event);

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
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
      goto cleanup;
    }
    /* launch the kernel */
	clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass1, 2, NULL, global_work_size, NULL, 0, NULL, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
    RecordProfileData(device,hullPass1,event);

    /* launch the kernel */
	clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, hullPass2, 2, NULL, global_work_size, NULL, 0, NULL, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
    RecordProfileData(device,hullPass2,event);
  }

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = image->columns * image->rows;
    clEnv->library->clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = image->columns * image->rows;
    clStatus = clEnv->library->clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", ".");
    goto cleanup;
  }

  outputReady=SyncCacheViewAuthenticPixels(filteredImage_view,exception);

cleanup:

  image_view=DestroyCacheView(image_view);
  if (filteredImage_view != NULL)
    filteredImage_view=DestroyCacheView(filteredImage_view);

  if (queue != NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != NULL)
    ReleaseOpenCLDevice(device);
  if (imageBuffer!=NULL)
    clEnv->library->clReleaseMemObject(imageBuffer);
  for (k = 0; k < 2; k++)
  {
    if (tempImageBuffer[k]!=NULL)
      clEnv->library->clReleaseMemObject(tempImageBuffer[k]);
  }
  if (filteredImageBuffer!=NULL)
    clEnv->library->clReleaseMemObject(filteredImageBuffer);
  if (hullPass1!=NULL)
    ReleaseOpenCLKernel(hullPass1);
  if (hullPass2!=NULL)
    ReleaseOpenCLKernel(hullPass2);
  if (outputReady == MagickFalse && filteredImage != NULL)
    filteredImage=DestroyImage(filteredImage);

  return(filteredImage);
}

MagickPrivate Image *AccelerateDespeckleImage(const Image* image,
  ExceptionInfo* exception)
{
  Image
    *filteredImage;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateConditionRGBA(image) == MagickFalse)
    return((Image *) NULL);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return((Image *) NULL);

  filteredImage=ComputeDespeckleImage(image,clEnv,exception);
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

static MagickBooleanType ComputeEqualizeImage(Image *image,MagickCLEnv clEnv,
  ExceptionInfo *exception)
{
#define EqualizeImageTag  "Equalize/Image"

  CacheView
    *image_view;

  cl_command_queue
    queue;

  cl_int
    clStatus;

  cl_mem_flags
    mem_flags;

  cl_mem
    equalizeMapBuffer,
    histogramBuffer,
    imageBuffer;

  cl_kernel
    equalizeKernel,
    histogramKernel;

  cl_event
    event;

  cl_uint4
    *histogram;

  cl_float4
    white,
    black,
    intensity,
    *map;

  MagickBooleanType
    outputReady,
    status;

  MagickCLDevice
    device;

  MagickSizeType
    length;

  PixelPacket
    *equalize_map;

  register ssize_t
    i;

  size_t
    global_work_size[2];

  void
    *hostPtr,
    *inputPixels;

  map=NULL;
  histogram=NULL;
  equalize_map=NULL;
  inputPixels = NULL;
  imageBuffer = NULL;
  histogramBuffer = NULL;
  equalizeMapBuffer = NULL;
  histogramKernel = NULL; 
  equalizeKernel = NULL; 
  queue = NULL;
  outputReady = MagickFalse;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  /*
   * initialize opencl env
   */
  device = RequestOpenCLDevice(clEnv);
  queue = AcquireOpenCLCommandQueue(device);

  /*
    Allocate and initialize histogram arrays.
  */
  histogram=(cl_uint4 *) AcquireQuantumMemory(MaxMap+1UL, sizeof(*histogram));
  if (histogram == (cl_uint4 *) NULL)
      ThrowBinaryException(ResourceLimitWarning,"MemoryAllocationFailed", image->filename);

  /* reset histogram */
  (void) ResetMagickMemory(histogram,0,(MaxMap+1)*sizeof(*histogram));

  /* Create and initialize OpenCL buffers. */
  /* inputPixels = AcquirePixelCachePixels(image, &length, exception); */
  /* assume this  will get a writable image */
  image_view=AcquireAuthenticCacheView(image,exception);
  inputPixels=GetCacheViewAuthenticPixels(image_view,0,0,image->columns,image->rows,exception);

  if (inputPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",image->filename);
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
  length = image->columns * image->rows;
  imageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
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
  histogramBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(cl_uint4), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  status = LaunchHistogramKernel(clEnv, device, queue, imageBuffer, histogramBuffer, image, image->channel_mask, exception);
  if (status == MagickFalse)
    goto cleanup;

  /* read from the kenel output */
  if (ALIGNED(histogram,cl_uint4)) 
  {
    length = (MaxMap+1); 
    clEnv->library->clEnqueueMapBuffer(queue, histogramBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(cl_uint4), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = (MaxMap+1); 
    clStatus = clEnv->library->clEnqueueReadBuffer(queue, histogramBuffer, CL_TRUE, 0, length * sizeof(cl_uint4), histogram, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", ".");
    goto cleanup;
  }

  /* unmap, don't block gpu to use this buffer again.  */
  if (ALIGNED(histogram,cl_uint4))
  {
    clStatus = clEnv->library->clEnqueueUnmapMemObject(queue, histogramBuffer, histogram, 0, NULL, NULL);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueUnmapMemObject failed.", ".");
      goto cleanup;
    }
  }

  /* recreate input buffer later, in case image updated */
#ifdef RECREATEBUFFER 
  if (imageBuffer!=NULL)		      
    clEnv->library->clReleaseMemObject(imageBuffer);
#endif
 
  /* CPU stuff */
  equalize_map=(PixelPacket *) AcquireQuantumMemory(MaxMap+1UL, sizeof(*equalize_map));
  if (equalize_map == (PixelPacket *) NULL)
    ThrowBinaryException(ResourceLimitWarning,"MemoryAllocationFailed", image->filename);

  map=(cl_float4 *) AcquireQuantumMemory(MaxMap+1UL,sizeof(*map));
  if (map == (cl_float4 *) NULL)
    ThrowBinaryException(ResourceLimitWarning,"MemoryAllocationFailed", image->filename);

  /*
    Integrate the histogram to get the equalization map.
  */
  (void) ResetMagickMemory(&intensity,0,sizeof(intensity));
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if ((image->channel_mask & SyncChannels) != 0)
    {
      intensity.x+=histogram[i].s[2];
      map[i]=intensity;
      continue;
    }
    if ((image->channel_mask & RedChannel) != 0)
      intensity.x+=histogram[i].s[2];
    if ((image->channel_mask & GreenChannel) != 0)
      intensity.y+=histogram[i].s[1];
    if ((image->channel_mask & BlueChannel) != 0)
      intensity.z+=histogram[i].s[0];
    if ((image->channel_mask & AlphaChannel) != 0)
      intensity.w+=histogram[i].s[3];
    map[i]=intensity;
  }
  black=map[0];
  white=map[(int) MaxMap];
  (void) ResetMagickMemory(equalize_map,0,(MaxMap+1)*sizeof(*equalize_map));
  for (i=0; i <= (ssize_t) MaxMap; i++)
  {
    if ((image->channel_mask & SyncChannels) != 0)
    {
      if (white.x != black.x)
        equalize_map[i].red=ScaleMapToQuantum((MagickRealType) ((MaxMap*
                (map[i].x-black.x))/(white.x-black.x)));
      continue;
    }
    if (((image->channel_mask & RedChannel) != 0) && (white.x != black.x))
      equalize_map[i].red=ScaleMapToQuantum((MagickRealType) ((MaxMap*
              (map[i].x-black.x))/(white.x-black.x)));
    if (((image->channel_mask & GreenChannel) != 0) && (white.y != black.y))
      equalize_map[i].green=ScaleMapToQuantum((MagickRealType) ((MaxMap*
              (map[i].y-black.y))/(white.y-black.y)));
    if (((image->channel_mask & BlueChannel) != 0) && (white.z != black.z))
      equalize_map[i].blue=ScaleMapToQuantum((MagickRealType) ((MaxMap*
              (map[i].z-black.z))/(white.z-black.z)));
    if (((image->channel_mask & AlphaChannel) != 0) && (white.w != black.w))
      equalize_map[i].alpha=ScaleMapToQuantum((MagickRealType) ((MaxMap*
              (map[i].w-black.w))/(white.w-black.w)));
  }

  if (image->storage_class == PseudoClass)
  {
    /*
       Equalize colormap.
       */
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      if ((image->channel_mask & SyncChannels) != 0)
      {
        if (white.x != black.x)
        {
          image->colormap[i].red=equalize_map[
            ScaleQuantumToMap(image->colormap[i].red)].red;
          image->colormap[i].green=equalize_map[
            ScaleQuantumToMap(image->colormap[i].green)].red;
          image->colormap[i].blue=equalize_map[
            ScaleQuantumToMap(image->colormap[i].blue)].red;
          image->colormap[i].alpha=equalize_map[
            ScaleQuantumToMap(image->colormap[i].alpha)].red;
        }
        continue;
      }
      if (((image->channel_mask & RedChannel) != 0) && (white.x != black.x))
        image->colormap[i].red=equalize_map[
          ScaleQuantumToMap(image->colormap[i].red)].red;
      if (((image->channel_mask & GreenChannel) != 0) && (white.y != black.y))
        image->colormap[i].green=equalize_map[
          ScaleQuantumToMap(image->colormap[i].green)].green;
      if (((image->channel_mask & BlueChannel) != 0) && (white.z != black.z))
        image->colormap[i].blue=equalize_map[
          ScaleQuantumToMap(image->colormap[i].blue)].blue;
      if (((image->channel_mask & AlphaChannel) != 0) && (white.w != black.w))
        image->colormap[i].alpha=equalize_map[
          ScaleQuantumToMap(image->colormap[i].alpha)].alpha;
    }
  }

  /*
    Equalize image.
  */

  /* GPU can work on this again, image and equalize map as input
    image:        uchar4 (CLPixelPacket)
    equalize_map: uchar4 (PixelPacket)
    black, white: float4 (FloatPixelPacket) */

#ifdef RECREATEBUFFER 
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
  length = image->columns * image->rows;
  imageBuffer = clEnv->library->clCreateBuffer(context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }
#endif

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
  equalizeMapBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(PixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  /* get the OpenCL kernel */
  equalizeKernel = AcquireOpenCLKernel(device,"Equalize");
  if (equalizeKernel == NULL)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", ".");
    goto cleanup;
  }

  /* set the kernel arguments */
  i = 0;
  clStatus=clEnv->library->clSetKernelArg(equalizeKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(equalizeKernel,i++,sizeof(ChannelType),&image->channel_mask);
  clStatus|=clEnv->library->clSetKernelArg(equalizeKernel,i++,sizeof(cl_mem),(void *)&equalizeMapBuffer);
  clStatus|=clEnv->library->clSetKernelArg(equalizeKernel,i++,sizeof(cl_float4),&white);
  clStatus|=clEnv->library->clSetKernelArg(equalizeKernel,i++,sizeof(cl_float4),&black);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
    goto cleanup;
  }

  /* launch the kernel */
  global_work_size[0] = image->columns;
  global_work_size[1] = image->rows;

  clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, equalizeKernel, 2, NULL, global_work_size, NULL, 0, NULL, &event);

  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
    goto cleanup;
  }
  RecordProfileData(device,equalizeKernel,event);

  /* read the data back */
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    length = image->columns * image->rows;
    clEnv->library->clEnqueueMapBuffer(queue, imageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = image->columns * image->rows;
    clStatus = clEnv->library->clEnqueueReadBuffer(queue, imageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), inputPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", ".");
    goto cleanup;
  }

  outputReady=SyncCacheViewAuthenticPixels(image_view,exception);

cleanup:

  image_view=DestroyCacheView(image_view);

  if (imageBuffer!=NULL)
    clEnv->library->clReleaseMemObject(imageBuffer);
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
    ReleaseOpenCLKernel(histogramKernel);
  if (equalizeKernel!=NULL)
    ReleaseOpenCLKernel(equalizeKernel);
  if (queue != NULL)
    ReleaseOpenCLCommandQueue(device, queue);
  if (device != NULL)
    ReleaseOpenCLDevice(device);

  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateEqualizeImage(Image *image,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkAccelerateConditionRGBA(image) == MagickFalse) ||
      (checkHistogramCondition(image,image->intensity) == MagickFalse))
    return(MagickFalse);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return(MagickFalse);

  status=ComputeEqualizeImage(image,clEnv,exception);
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

static MagickBooleanType ComputeFunctionImage(Image *image,MagickCLEnv clEnv,
  const MagickFunction function,const size_t number_parameters,
  const double *parameters,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_int
    status;

  cl_kernel
    functionKernel;

  cl_mem
    imageBuffer,
    parametersBuffer;

  cl_uint
    number_params,
    number_channels;

  float
    *parametersBufferPtr;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  size_t
    gsize[2],
    i;

  outputReady=MagickFalse;

  functionKernel=NULL;
  parametersBuffer=NULL;

  device=RequestOpenCLDevice(clEnv);
  queue=AcquireOpenCLCommandQueue(device);
  imageBuffer=GetAuthenticOpenCLBuffer(image,device,exception);
  if (imageBuffer == (cl_mem) NULL)
    goto cleanup;

  parametersBufferPtr=(float *) AcquireQuantumMemory(number_parameters,
    sizeof(float));
  if (parametersBufferPtr == (float *) NULL)
    goto cleanup;
  for (i=0; i<number_parameters; i++)
    parametersBufferPtr[i]=(float) parameters[i];
  parametersBuffer=CreateOpenCLBuffer(device,CL_MEM_READ_ONLY |
    CL_MEM_COPY_HOST_PTR,number_parameters*sizeof(*parametersBufferPtr),
    parametersBufferPtr);
  parametersBufferPtr=RelinquishMagickMemory(parametersBufferPtr);
  if (parametersBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"CreateOpenCLBuffer failed.",".");
    goto cleanup;
  }

  functionKernel=AcquireOpenCLKernel(device,"ComputeFunction");
  if (functionKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  number_channels=(cl_uint) image->number_channels;
  number_params=(cl_uint) number_parameters;

  i=0;
  status =SetOpenCLKernelArg(functionKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  status|=SetOpenCLKernelArg(functionKernel,i++,sizeof(cl_uint),(void *)&number_channels);
  status|=SetOpenCLKernelArg(functionKernel,i++,sizeof(ChannelType),(void *)&image->channel_mask);
  status|=SetOpenCLKernelArg(functionKernel,i++,sizeof(MagickFunction),(void *)&function);
  status|=SetOpenCLKernelArg(functionKernel,i++,sizeof(cl_uint),(void *)&number_params);
  status|=SetOpenCLKernelArg(functionKernel,i++,sizeof(cl_mem),(void *)&parametersBuffer);
  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"SetOpenCLKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=image->columns;
  gsize[1]=image->rows;
  outputReady=EnqueueOpenCLKernel(queue,functionKernel,2,(const size_t *) NULL,
    gsize,(const size_t *) NULL,image,(const Image *) NULL,MagickFalse,
    exception);

cleanup:

  if (parametersBuffer != (cl_mem) NULL)
    ReleaseOpenCLMemObject(parametersBuffer);
  if (functionKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(functionKernel);
  if (queue != (cl_command_queue) NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != (MagickCLDevice) NULL)
    ReleaseOpenCLDevice(device);
  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateFunctionImage(Image *image,
  const MagickFunction function,const size_t number_parameters,
  const double *parameters,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateCondition(image) == MagickFalse)
    return(MagickFalse);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return(MagickFalse);

  status=ComputeFunctionImage(image,clEnv,function,number_parameters,
    parameters,exception);
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

static MagickBooleanType ComputeGrayscaleImage(Image *image,MagickCLEnv clEnv,
  const PixelIntensityMethod method,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_int
    status;

  cl_kernel
    grayscaleKernel;

  cl_mem
    imageBuffer;

  cl_uint
    number_channels,
    colorspace,
    intensityMethod;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  size_t
    gsize[2],
    i;

  outputReady=MagickFalse;
  grayscaleKernel=NULL;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  device=RequestOpenCLDevice(clEnv);
  queue=AcquireOpenCLCommandQueue(device);
  imageBuffer=GetAuthenticOpenCLBuffer(image,device,exception);
  if (imageBuffer == (cl_mem) NULL)
    goto cleanup;

  grayscaleKernel=AcquireOpenCLKernel(device,"Grayscale");
  if (grayscaleKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  number_channels=(cl_uint) image->number_channels;
  intensityMethod=(cl_uint) method;
  colorspace=(cl_uint) image->colorspace;

  i=0;
  status =SetOpenCLKernelArg(grayscaleKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  status|=SetOpenCLKernelArg(grayscaleKernel,i++,sizeof(cl_uint),&number_channels);
  status|=SetOpenCLKernelArg(grayscaleKernel,i++,sizeof(cl_uint),&colorspace);
  status|=SetOpenCLKernelArg(grayscaleKernel,i++,sizeof(cl_uint),&intensityMethod);
  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"SetOpenCLKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=image->columns;
  gsize[1]=image->rows;
  outputReady=EnqueueOpenCLKernel(queue,grayscaleKernel,2,
    (const size_t *) NULL,gsize,(const size_t *) NULL,image,(Image *) NULL,
    MagickFalse,exception);

cleanup:

  if (grayscaleKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(grayscaleKernel);
  if (queue != (cl_command_queue) NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != (MagickCLDevice) NULL)
    ReleaseOpenCLDevice(device);

  return(outputReady);
}

MagickPrivate MagickBooleanType AccelerateGrayscaleImage(Image* image,
  const PixelIntensityMethod method,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if ((checkAccelerateCondition(image) == MagickFalse) ||
      (checkPixelIntensity(image,method) == MagickFalse))
    return(MagickFalse);

  if (image->number_channels < 3)
    return(MagickFalse);

  if ((GetPixelRedTraits(image) == UndefinedPixelTrait) ||
      (GetPixelGreenTraits(image) == UndefinedPixelTrait) ||
      (GetPixelBlueTraits(image) == UndefinedPixelTrait))
    return(MagickFalse);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return(MagickFalse);

  status=ComputeGrayscaleImage(image,clEnv,method,exception);
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

static Image *ComputeLocalContrastImage(const Image *image,MagickCLEnv clEnv,
  const double radius,const double strength,ExceptionInfo *exception)
{
  CacheView
    *filteredImage_view,
    *image_view;

  cl_command_queue
    queue;

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
    imageKernelBuffer,
    tempImageBuffer;

  cl_mem_flags
    mem_flags;

  const void
    *inputPixels;

  Image
    *filteredImage;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  MagickSizeType
    length;

  void
    *filteredPixels,
    *hostPtr;

  unsigned int
    i,
    imageColumns,
    imageRows,
    passes;

  filteredImage = NULL;
  filteredImage_view = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  tempImageBuffer = NULL;
  imageKernelBuffer = NULL;
  blurRowKernel = NULL;
  blurColumnKernel = NULL;
  queue = NULL;
  outputReady = MagickFalse;

  device = RequestOpenCLDevice(clEnv);
  queue = AcquireOpenCLCommandQueue(device);

  /* Create and initialize OpenCL buffers. */
  {
    image_view=AcquireAuthenticCacheView(image,exception);
    inputPixels=GetCacheViewAuthenticPixels(image_view,0,0,image->columns,image->rows,exception);
    if (inputPixels == (const void *) NULL)
    {
      (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",image->filename);
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
    length = image->columns * image->rows;
    imageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  /* create output */
  {
    filteredImage = CloneImage(image,image->columns,image->rows,MagickTrue,exception);
    assert(filteredImage != NULL);
    if (SetImageStorageClass(filteredImage,DirectClass,exception) != MagickTrue)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "CloneImage failed.", ".");
      goto cleanup;
    }
    filteredImage_view=AcquireAuthenticCacheView(filteredImage,exception);
    filteredPixels=GetCacheViewAuthenticPixels(filteredImage_view,0,0,filteredImage->columns,filteredImage->rows,exception);
    if (filteredPixels == (void *) NULL)
    {
      (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),CacheWarning, "UnableToReadPixelCache.","`%s'",filteredImage->filename);
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
    length = image->columns * image->rows;
    filteredImageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(CLPixelPacket), hostPtr, &clStatus);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
      goto cleanup;
    }
  }

  {
    /* create temp buffer */
    {
      length = image->columns * image->rows;
      tempImageBuffer = clEnv->library->clCreateBuffer(device->context, CL_MEM_READ_WRITE, length * sizeof(float), NULL, &clStatus);
      if (clStatus != CL_SUCCESS)
      {
        (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
        goto cleanup;
      }
    }

    /* get the opencl kernel */
    {
      blurRowKernel = AcquireOpenCLKernel(device,"LocalContrastBlurRow");
      if (blurRowKernel == NULL)
      {
        (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", ".");
        goto cleanup;
      };

      blurColumnKernel = AcquireOpenCLKernel(device,"LocalContrastBlurApplyColumn");
      if (blurColumnKernel == NULL)
      {
        (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", ".");
        goto cleanup;
      };
    }

    {
      imageColumns = (unsigned int) image->columns;
      imageRows = (unsigned int) image->rows;
      iRadius = (cl_int) (image->rows > image->columns ? image->rows : image->columns) * 0.002f * fabs(radius); // Normalized radius, 100% gives blur radius of 20% of the largest dimension

      passes = (((1.0f * imageRows) * imageColumns * iRadius) + 3999999999) / 4000000000.0f;
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
        (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
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

        clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, blurRowKernel, 2, goffset, gsize, wsize, 0, NULL, &event);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
          goto cleanup;
        }
        clEnv->library->clFlush(queue);
        RecordProfileData(device,blurRowKernel,event);
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
        (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
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

        clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, blurColumnKernel, 2, goffset, gsize, wsize, 0, NULL, &event);
        if (clStatus != CL_SUCCESS)
        {
          (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
          goto cleanup;
        }
        clEnv->library->clFlush(queue);
        RecordProfileData(device,blurColumnKernel,event);
      }
    }
  }

  /* get result */
  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = image->columns * image->rows;
    clEnv->library->clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = image->columns * image->rows;
    clStatus = clEnv->library->clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", ".");
    goto cleanup;
  }

  outputReady=SyncCacheViewAuthenticPixels(filteredImage_view,exception);

cleanup:

  image_view=DestroyCacheView(image_view);
  if (filteredImage_view != NULL)
    filteredImage_view=DestroyCacheView(filteredImage_view);

  if (imageBuffer!=NULL)
    clEnv->library->clReleaseMemObject(imageBuffer);
  if (filteredImageBuffer!=NULL)
    clEnv->library->clReleaseMemObject(filteredImageBuffer);
  if (tempImageBuffer!=NULL)
    clEnv->library->clReleaseMemObject(tempImageBuffer);
  if (imageKernelBuffer!=NULL)
    clEnv->library->clReleaseMemObject(imageKernelBuffer);
  if (blurRowKernel!=NULL)
    ReleaseOpenCLKernel(blurRowKernel);
  if (blurColumnKernel!=NULL)
    ReleaseOpenCLKernel(blurColumnKernel);
  if (queue != NULL)
    ReleaseOpenCLCommandQueue(device, queue);
  if (device != NULL)
    ReleaseOpenCLDevice(device);
  if (outputReady == MagickFalse)
  {
    if (filteredImage != NULL)
    {
      DestroyImage(filteredImage);
      filteredImage = NULL;
    }
  }

  return(filteredImage);
}

MagickPrivate Image *AccelerateLocalContrastImage(const Image *image,
  const double radius,const double strength,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateConditionRGBA(image) == MagickFalse)
    return((Image *) NULL);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return((Image *) NULL);

  filteredImage=ComputeLocalContrastImage(image,clEnv,radius,strength,
    exception);
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

static MagickBooleanType ComputeModulateImage(Image *image,MagickCLEnv clEnv,
  const double percent_brightness,const double percent_hue,
  const double percent_saturation,const ColorspaceType colorspace,
  ExceptionInfo *exception)
{
  CacheView
    *image_view;

  cl_float
    bright,
    hue,
    saturation;

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

  cl_mem_flags
    mem_flags;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  MagickSizeType
    length;

  register ssize_t
    i;

  void
    *inputPixels;

  inputPixels = NULL;
  imageBuffer = NULL;
  modulateKernel = NULL; 

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  /*
   * initialize opencl env
   */
  device = RequestOpenCLDevice(clEnv);
  queue = AcquireOpenCLCommandQueue(device);

  outputReady = MagickFalse;

  /* Create and initialize OpenCL buffers.
   inputPixels = AcquirePixelCachePixels(image, &length, exception);
   assume this  will get a writable image
   */
  image_view=AcquireAuthenticCacheView(image,exception);
  inputPixels=GetCacheViewAuthenticPixels(image_view,0,0,image->columns,image->rows,exception);
  if (inputPixels == (void *) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),CacheWarning,"UnableToReadPixelCache.","`%s'",image->filename);
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
  length = image->columns * image->rows;
  imageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  modulateKernel = AcquireOpenCLKernel(device, "Modulate");
  if (modulateKernel == NULL)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "AcquireOpenCLKernel failed.", ".");
    goto cleanup;
  }

  bright=percent_brightness;
  hue=percent_hue;
  saturation=percent_saturation;
  color=colorspace;

  i = 0;
  clStatus=clEnv->library->clSetKernelArg(modulateKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  clStatus|=clEnv->library->clSetKernelArg(modulateKernel,i++,sizeof(cl_float),&bright);
  clStatus|=clEnv->library->clSetKernelArg(modulateKernel,i++,sizeof(cl_float),&hue);
  clStatus|=clEnv->library->clSetKernelArg(modulateKernel,i++,sizeof(cl_float),&saturation);
  clStatus|=clEnv->library->clSetKernelArg(modulateKernel,i++,sizeof(cl_float),&color);
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clSetKernelArg failed.", ".");
    goto cleanup;
  }

  {
    size_t global_work_size[2];
    global_work_size[0] = image->columns;
    global_work_size[1] = image->rows;
    /* launch the kernel */
	clStatus = clEnv->library->clEnqueueNDRangeKernel(queue, modulateKernel, 2, NULL, global_work_size, NULL, 0, NULL, &event);
    if (clStatus != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
      goto cleanup;
    }
    RecordProfileData(device,modulateKernel,event);
  }

  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    length = image->columns * image->rows;
    clEnv->library->clEnqueueMapBuffer(queue, imageBuffer, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, NULL, &clStatus);
  }
  else 
  {
    length = image->columns * image->rows;
    clStatus = clEnv->library->clEnqueueReadBuffer(queue, imageBuffer, CL_TRUE, 0, length * sizeof(CLPixelPacket), inputPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception, GetMagickModule(), ResourceLimitWarning, "Reading output image from CL buffer failed.", ".");
    goto cleanup;
  }

  outputReady=SyncCacheViewAuthenticPixels(image_view,exception);

cleanup:

  image_view=DestroyCacheView(image_view);

  if (imageBuffer!=NULL)
    clEnv->library->clReleaseMemObject(imageBuffer);
  if (modulateKernel!=NULL)
    ReleaseOpenCLKernel(modulateKernel);
  if (queue != NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != NULL)
    ReleaseOpenCLDevice(device);

  return outputReady;

}

MagickPrivate MagickBooleanType AccelerateModulateImage(Image *image,
  const double percent_brightness,const double percent_hue,
  const double percent_saturation,const ColorspaceType colorspace,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateConditionRGBA(image) == MagickFalse)
    return(MagickFalse);

  if ((colorspace != HSLColorspace) && (colorspace != UndefinedColorspace))
    return(MagickFalse);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return(MagickFalse);

  status=ComputeModulateImage(image,clEnv,percent_brightness,percent_hue,
    percent_saturation,colorspace,exception);
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

static Image* ComputeMotionBlurImage(const Image *image,MagickCLEnv clEnv,
  const double *kernel,const size_t width,const OffsetInfo *offset,
  ExceptionInfo *exception)
{
  CacheView
    *filteredImage_view,
    *image_view;

  cl_command_queue
    queue;

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

  cl_mem_flags
    mem_flags;

  const void
    *inputPixels;

  float
    *kernelBufferPtr;

  Image
    *filteredImage;

  int
    *offsetBufferPtr;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  PixelInfo
    bias;

  MagickSizeType
    length;

  size_t
    global_work_size[2],
    local_work_size[2];

  unsigned int
    i,
    imageHeight,
    imageWidth,
    matte;

  void
    *filteredPixels,
    *hostPtr;

  outputReady = MagickFalse;
  filteredImage = NULL;
  filteredImage_view = NULL;
  imageBuffer = NULL;
  filteredImageBuffer = NULL;
  imageKernelBuffer = NULL;
  motionBlurKernel = NULL;
  queue = NULL;

  device = RequestOpenCLDevice(clEnv);

  /* Create and initialize OpenCL buffers. */

  image_view=AcquireAuthenticCacheView(image,exception);
  inputPixels=GetCacheViewAuthenticPixels(image_view,0,0,image->columns,image->rows,exception);
  if (inputPixels == (const void *) NULL)
  {
    (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
      "UnableToReadPixelCache.","`%s'",image->filename);
    goto cleanup;
  }

  // If the host pointer is aligned to the size of CLPixelPacket, 
  // then use the host buffer directly from the GPU; otherwise, 
  // create a buffer on the GPU and copy the data over
  if (ALIGNED(inputPixels,CLPixelPacket)) 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR;
  }
  else 
  {
    mem_flags = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
  }
  // create a CL buffer from image pixel buffer
  length = image->columns * image->rows;
  imageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, 
    length * sizeof(CLPixelPacket), (void*)inputPixels, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(),
      ResourceLimitError, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }


  filteredImage = CloneImage(image,image->columns,image->rows,
    MagickTrue,exception);
  assert(filteredImage != NULL);
  if (SetImageStorageClass(filteredImage,DirectClass,exception) != MagickTrue)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), 
      ResourceLimitError, "CloneImage failed.", ".");
    goto cleanup;
  }
  filteredImage_view=AcquireAuthenticCacheView(filteredImage,exception);
  filteredPixels=GetCacheViewAuthenticPixels(filteredImage_view,0,0,filteredImage->columns,filteredImage->rows,exception);
  if (filteredPixels == (void *) NULL)
  {
    (void) ThrowMagickException(exception,GetMagickModule(),CacheError, 
      "UnableToReadPixelCache.","`%s'",filteredImage->filename);
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
  // create a CL buffer from image pixel buffer
  length = image->columns * image->rows;
  filteredImageBuffer = clEnv->library->clCreateBuffer(device->context, mem_flags, 
    length * sizeof(CLPixelPacket), hostPtr, &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), 
      ResourceLimitError, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }


  imageKernelBuffer = clEnv->library->clCreateBuffer(device->context, 
    CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR, width * sizeof(float), NULL,
    &clStatus);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), 
      ResourceLimitError, "clEnv->library->clCreateBuffer failed.",".");
    goto cleanup;
  }

  queue = AcquireOpenCLCommandQueue(device);
  kernelBufferPtr = (float*)clEnv->library->clEnqueueMapBuffer(queue, imageKernelBuffer, 
    CL_TRUE, CL_MAP_WRITE, 0, width * sizeof(float), 0, NULL, NULL, &clStatus);
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
      "clEnv->library->clEnqueueUnmapMemObject failed.", ".");
    goto cleanup;
  }

  offsetBuffer = clEnv->library->clCreateBuffer(device->context, 
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
      "clEnv->library->clEnqueueUnmapMemObject failed.", ".");
    goto cleanup;
  }


 // get the OpenCL kernel
  motionBlurKernel = AcquireOpenCLKernel(device,"MotionBlur");
  if (motionBlurKernel == NULL)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleFatalError,
      "AcquireOpenCLKernel failed.", ".");
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

  GetPixelInfo(image,&bias);
  biasPixel.s[0] = bias.red;
  biasPixel.s[1] = bias.green;
  biasPixel.s[2] = bias.blue;
  biasPixel.s[3] = bias.alpha;
  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(cl_float4), &biasPixel);

  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(ChannelType), &image->channel_mask);
  matte = (image->alpha_trait > CopyPixelTrait)?1:0;
  clStatus|=clEnv->library->clSetKernelArg(motionBlurKernel,i++,sizeof(unsigned int), &matte);
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleFatalError,
      "clEnv->library->clSetKernelArg failed.", ".");
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
      "clEnv->library->clEnqueueNDRangeKernel failed.", ".");
    goto cleanup;
  }
  RecordProfileData(device,motionBlurKernel,event);

  if (ALIGNED(filteredPixels,CLPixelPacket)) 
  {
    length = image->columns * image->rows;
    clEnv->library->clEnqueueMapBuffer(queue, filteredImageBuffer, CL_TRUE, 
      CL_MAP_READ|CL_MAP_WRITE, 0, length * sizeof(CLPixelPacket), 0, NULL, 
      NULL, &clStatus);
  }
  else 
  {
    length = image->columns * image->rows;
    clStatus = clEnv->library->clEnqueueReadBuffer(queue, filteredImageBuffer, CL_TRUE, 0, 
      length * sizeof(CLPixelPacket), filteredPixels, 0, NULL, NULL);
  }
  if (clStatus != CL_SUCCESS)
  {
    (void) ThrowMagickException(exception, GetMagickModule(), ModuleFatalError,
      "Reading output image from CL buffer failed.", ".");
    goto cleanup;
  }
  outputReady=SyncCacheViewAuthenticPixels(filteredImage_view,exception);

cleanup:

  image_view=DestroyCacheView(image_view);
  if (filteredImage_view != NULL)
    filteredImage_view=DestroyCacheView(filteredImage_view);

  if (filteredImageBuffer!=NULL)
    clEnv->library->clReleaseMemObject(filteredImageBuffer);
  if (imageBuffer!=NULL)
    clEnv->library->clReleaseMemObject(imageBuffer);
  if (imageKernelBuffer!=NULL)
    clEnv->library->clReleaseMemObject(imageKernelBuffer);
  if (motionBlurKernel!=NULL)
    ReleaseOpenCLKernel(motionBlurKernel);
  if (queue != NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != NULL)
    ReleaseOpenCLDevice(device);
  if (outputReady == MagickFalse && filteredImage != NULL)
    filteredImage=DestroyImage(filteredImage);

  return(filteredImage);
}

MagickPrivate Image *AccelerateMotionBlurImage(const Image *image,
  const double* kernel,const size_t width,const OffsetInfo *offset,
  ExceptionInfo *exception)
{
  Image
    *filteredImage;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(kernel != (double *) NULL);
  assert(offset != (OffsetInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateConditionRGBA(image) == MagickFalse)
    return((Image *) NULL);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return((Image *) NULL);

  filteredImage=ComputeMotionBlurImage(image,clEnv,kernel,width,offset,
    exception);
  return(filteredImage);
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

static MagickBooleanType resizeHorizontalFilter(MagickCLDevice device,
  cl_command_queue queue,const Image *image,Image *filteredImage,
  cl_mem imageBuffer,cl_uint number_channels,cl_uint columns,cl_uint rows,
  cl_mem resizedImageBuffer,cl_uint resizedColumns,cl_uint resizedRows,
  const ResizeFilter *resizeFilter,cl_mem resizeFilterCubicCoefficients,
  const float xFactor,ExceptionInfo *exception)
{
  cl_kernel
    horizontalKernel;

  cl_int
    status;

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
    outputReady;

  size_t
    gammaAccumulatorLocalMemorySize,
    gsize[2],
    i,
    imageCacheLocalMemorySize,
    pixelAccumulatorLocalMemorySize,
    lsize[2],
    totalLocalMemorySize,
    weightAccumulatorLocalMemorySize;

  unsigned int
    chunkSize,
    pixelPerWorkgroup;

  horizontalKernel=NULL;
  outputReady=MagickFalse;

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
    support=(float) 0.5;
    scale=1.0;
  }
  scale=PerceptibleReciprocal(scale);

  if (resizedColumns < workgroupSize) 
  {
    chunkSize=32;
    pixelPerWorkgroup=32;
  }
  else
  {
    chunkSize=workgroupSize;
    pixelPerWorkgroup=workgroupSize;
  }

DisableMSCWarning(4127)
  while(1)
RestoreMSCWarning
  {
    /* calculate the local memory size needed per workgroup */
    cacheRangeStart=(int) (((0 + 0.5)/xFactor+MagickEpsilon)-support+0.5);
    cacheRangeEnd=(int) ((((pixelPerWorkgroup-1) + 0.5)/xFactor+
      MagickEpsilon)+support+0.5);
    numCachedPixels=cacheRangeEnd-cacheRangeStart+1;
    imageCacheLocalMemorySize=numCachedPixels*sizeof(CLQuantum)*
      number_channels;
    totalLocalMemorySize=imageCacheLocalMemorySize;

    /* local size for the pixel accumulator */
    pixelAccumulatorLocalMemorySize=chunkSize*sizeof(cl_float4);
    totalLocalMemorySize+=pixelAccumulatorLocalMemorySize;

    /* local memory size for the weight accumulator */
    weightAccumulatorLocalMemorySize=chunkSize*sizeof(float);
    totalLocalMemorySize+=weightAccumulatorLocalMemorySize;

    /* local memory size for the gamma accumulator */
    if ((number_channels == 4) || (number_channels == 2))
      gammaAccumulatorLocalMemorySize=chunkSize*sizeof(float);
    else
      gammaAccumulatorLocalMemorySize=sizeof(float);
    totalLocalMemorySize+=gammaAccumulatorLocalMemorySize;

    if (totalLocalMemorySize <= device->local_memory_size)
      break;
    else
    {
      pixelPerWorkgroup=pixelPerWorkgroup/2;
      chunkSize=chunkSize/2;
      if ((pixelPerWorkgroup == 0) || (chunkSize == 0))
      {
        /* quit, fallback to CPU */
        goto cleanup;
      }
    }
  }

  resizeFilterType=(int)GetResizeFilterWeightingType(resizeFilter);
  resizeWindowType=(int)GetResizeFilterWindowWeightingType(resizeFilter);

  horizontalKernel=AcquireOpenCLKernel(device,"ResizeHorizontalFilter");
  if (horizontalKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.", ".");
    goto cleanup;
  }

  resizeFilterScale=(float) GetResizeFilterScale(resizeFilter);
  resizeFilterSupport=(float) GetResizeFilterSupport(resizeFilter);
  resizeFilterWindowSupport=(float) GetResizeFilterWindowSupport(resizeFilter);
  resizeFilterBlur=(float) GetResizeFilterBlur(resizeFilter);

  i=0;
  status =SetOpenCLKernelArg(horizontalKernel,i++,sizeof(cl_mem),(void*)&imageBuffer);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(cl_uint),(void*)&number_channels);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(cl_uint),(void*)&columns);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(cl_uint),(void*)&rows);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(cl_mem),(void*)&resizedImageBuffer);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(cl_uint),(void*)&resizedColumns);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(cl_uint),(void*)&resizedRows);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(float),(void*)&xFactor);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(int),(void*)&resizeFilterType);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(int),(void*)&resizeWindowType);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(cl_mem),(void*)&resizeFilterCubicCoefficients);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(float),(void*)&resizeFilterScale);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(float),(void*)&resizeFilterSupport);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(float),(void*)&resizeFilterWindowSupport);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(float),(void*)&resizeFilterBlur);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,imageCacheLocalMemorySize,NULL);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(int),&numCachedPixels);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(unsigned int),&pixelPerWorkgroup);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,sizeof(unsigned int),&chunkSize);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,pixelAccumulatorLocalMemorySize,NULL);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,weightAccumulatorLocalMemorySize,NULL);
  status|=SetOpenCLKernelArg(horizontalKernel,i++,gammaAccumulatorLocalMemorySize,NULL);

  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"SetOpenCLKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=(resizedColumns+pixelPerWorkgroup-1)/pixelPerWorkgroup*
    workgroupSize;
  gsize[1]=resizedRows;
  lsize[0]=workgroupSize;
  lsize[1]=1;
  outputReady=EnqueueOpenCLKernel(queue,horizontalKernel,2,
    (const size_t *) NULL,gsize,lsize,image,filteredImage,MagickFalse,
    exception);

cleanup:

  if (horizontalKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(horizontalKernel);

  return(outputReady);
}

static MagickBooleanType resizeVerticalFilter(MagickCLDevice device,
  cl_command_queue queue,const Image *image,Image * filteredImage,
  cl_mem imageBuffer,cl_uint number_channels,cl_uint columns,cl_uint rows,
  cl_mem resizedImageBuffer,cl_uint resizedColumns,cl_uint resizedRows,
  const ResizeFilter *resizeFilter,cl_mem resizeFilterCubicCoefficients,
  const float yFactor,ExceptionInfo *exception)
{
  cl_kernel
    verticalKernel;

  cl_int
    status;

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
    outputReady;

  size_t
    gammaAccumulatorLocalMemorySize,
    gsize[2],
    i,
    imageCacheLocalMemorySize,
    pixelAccumulatorLocalMemorySize,
    lsize[2],
    totalLocalMemorySize,
    weightAccumulatorLocalMemorySize;

  unsigned int
    chunkSize,
    pixelPerWorkgroup;

  verticalKernel=NULL;
  outputReady=MagickFalse;

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
    support=(float) 0.5;
    scale=1.0;
  }
  scale=PerceptibleReciprocal(scale);

  if (resizedRows < workgroupSize) 
  {
    chunkSize=32;
    pixelPerWorkgroup=32;
  }
  else
  {
    chunkSize=workgroupSize;
    pixelPerWorkgroup=workgroupSize;
  }

DisableMSCWarning(4127)
  while(1)
RestoreMSCWarning
  {
    /* calculate the local memory size needed per workgroup */
    cacheRangeStart=(int) (((0 + 0.5)/yFactor+MagickEpsilon)-support+0.5);
    cacheRangeEnd=(int) ((((pixelPerWorkgroup-1) + 0.5)/yFactor+
      MagickEpsilon)+support+0.5);
    numCachedPixels=cacheRangeEnd-cacheRangeStart+1;
    imageCacheLocalMemorySize=numCachedPixels*sizeof(CLQuantum)*
      number_channels;
    totalLocalMemorySize=imageCacheLocalMemorySize;

    /* local size for the pixel accumulator */
    pixelAccumulatorLocalMemorySize=chunkSize*sizeof(cl_float4);
    totalLocalMemorySize+=pixelAccumulatorLocalMemorySize;

    /* local memory size for the weight accumulator */
    weightAccumulatorLocalMemorySize=chunkSize*sizeof(float);
    totalLocalMemorySize+=weightAccumulatorLocalMemorySize;

    /* local memory size for the gamma accumulator */
    if ((number_channels == 4) || (number_channels == 2))
      gammaAccumulatorLocalMemorySize=chunkSize*sizeof(float);
    else
      gammaAccumulatorLocalMemorySize=sizeof(float);
    totalLocalMemorySize+=gammaAccumulatorLocalMemorySize;

    if (totalLocalMemorySize <= device->local_memory_size)
      break;
    else
    {
      pixelPerWorkgroup=pixelPerWorkgroup/2;
      chunkSize=chunkSize/2;
      if ((pixelPerWorkgroup == 0) || (chunkSize == 0))
      {
        /* quit, fallback to CPU */
        goto cleanup;
      }
    }
  }

  resizeFilterType=(int)GetResizeFilterWeightingType(resizeFilter);
  resizeWindowType=(int)GetResizeFilterWindowWeightingType(resizeFilter);

  verticalKernel=AcquireOpenCLKernel(device,"ResizeVerticalFilter");
  if (verticalKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  resizeFilterScale=(float) GetResizeFilterScale(resizeFilter);
  resizeFilterSupport=(float) GetResizeFilterSupport(resizeFilter);
  resizeFilterBlur=(float) GetResizeFilterBlur(resizeFilter);
  resizeFilterWindowSupport=(float) GetResizeFilterWindowSupport(resizeFilter);

  i=0;
  status =SetOpenCLKernelArg(verticalKernel,i++,sizeof(cl_mem),(void*)&imageBuffer);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(cl_uint),(void*)&number_channels);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(cl_uint),(void*)&columns);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(cl_uint),(void*)&rows);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(cl_mem),(void*)&resizedImageBuffer);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(cl_uint),(void*)&resizedColumns);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(cl_uint),(void*)&resizedRows);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(float),(void*)&yFactor);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(int),(void*)&resizeFilterType);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(int),(void*)&resizeWindowType);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(cl_mem),(void*)&resizeFilterCubicCoefficients);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(float),(void*)&resizeFilterScale);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(float),(void*)&resizeFilterSupport);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(float),(void*)&resizeFilterWindowSupport);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(float),(void*)&resizeFilterBlur);
  status|=SetOpenCLKernelArg(verticalKernel,i++,imageCacheLocalMemorySize, NULL);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(int), &numCachedPixels);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(unsigned int), &pixelPerWorkgroup);
  status|=SetOpenCLKernelArg(verticalKernel,i++,sizeof(unsigned int), &chunkSize);
  status|=SetOpenCLKernelArg(verticalKernel,i++,pixelAccumulatorLocalMemorySize, NULL);
  status|=SetOpenCLKernelArg(verticalKernel,i++,weightAccumulatorLocalMemorySize, NULL);
  status|=SetOpenCLKernelArg(verticalKernel,i++,gammaAccumulatorLocalMemorySize, NULL);

  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"SetOpenCLKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=resizedColumns;
  gsize[1]=(resizedRows+pixelPerWorkgroup-1)/pixelPerWorkgroup*
    workgroupSize;
  lsize[0]=1;
  lsize[1]=workgroupSize;
  outputReady=EnqueueOpenCLKernel(queue,verticalKernel,2,(const size_t *) NULL,
    gsize,lsize,image,filteredImage,MagickFalse,exception);

cleanup:

  if (verticalKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(verticalKernel);

  return(outputReady);
}

static Image *ComputeResizeImage(const Image* image,MagickCLEnv clEnv,
  const size_t resizedColumns,const size_t resizedRows,
  const ResizeFilter *resizeFilter,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_mem
    cubicCoefficientsBuffer,
    filteredImageBuffer,
    imageBuffer,
    tempImageBuffer;

  cl_uint
    number_channels;

  const double
    *resizeFilterCoefficient;

  float
    coefficientBuffer[7],
    xFactor,
    yFactor;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  MagickSizeType
    length;

  Image
    *filteredImage;

  size_t
    i;

  filteredImage=NULL;
  tempImageBuffer=NULL;
  cubicCoefficientsBuffer=NULL;
  outputReady=MagickFalse;

  device=RequestOpenCLDevice(clEnv);
  queue=AcquireOpenCLCommandQueue(device);
  filteredImage=CloneImage(image,resizedColumns,resizedRows,MagickTrue,
    exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;
  if (filteredImage->number_channels != image->number_channels)
    goto cleanup;
  imageBuffer=GetAuthenticOpenCLBuffer(image,device,exception);
  if (imageBuffer == (cl_mem) NULL)
    goto cleanup;
  filteredImageBuffer=GetAuthenticOpenCLBuffer(filteredImage,device,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
    goto cleanup;

  resizeFilterCoefficient=GetResizeFilterCoefficient(resizeFilter);
  for (i = 0; i < 7; i++)
    coefficientBuffer[i]=(float) resizeFilterCoefficient[i];
  cubicCoefficientsBuffer=CreateOpenCLBuffer(device,CL_MEM_COPY_HOST_PTR |
    CL_MEM_READ_ONLY,7*sizeof(*resizeFilterCoefficient),&coefficientBuffer);
  if (cubicCoefficientsBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"CreateOpenCLBuffer failed.",".");
    goto cleanup;
  }

  number_channels=(cl_uint) image->number_channels;
  xFactor=(float) resizedColumns/(float) image->columns;
  yFactor=(float) resizedRows/(float) image->rows;
  if (xFactor > yFactor)
  {
    length=resizedColumns*image->rows*number_channels;
    tempImageBuffer=CreateOpenCLBuffer(device,CL_MEM_READ_WRITE,length*
      sizeof(CLQuantum),(void *) NULL);
    if (tempImageBuffer == (cl_mem) NULL)
    {
      (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
        ResourceLimitWarning,"CreateOpenCLBuffer failed.",".");
      goto cleanup;
    }

    outputReady=resizeHorizontalFilter(device,queue,image,filteredImage,
      imageBuffer,number_channels,(cl_uint) image->columns,
      (cl_uint) image->rows,tempImageBuffer,(cl_uint) resizedColumns,
      (cl_uint) image->rows,resizeFilter,cubicCoefficientsBuffer,xFactor,
      exception);
    if (outputReady == MagickFalse)
      goto cleanup;

    outputReady=resizeVerticalFilter(device,queue,image,filteredImage,
      tempImageBuffer,number_channels,(cl_uint) resizedColumns,
      (cl_uint) image->rows,filteredImageBuffer,(cl_uint) resizedColumns,
      (cl_uint) resizedRows,resizeFilter,cubicCoefficientsBuffer,yFactor,
      exception);
    if (outputReady == MagickFalse)
      goto cleanup;
  }
  else
  {
    length=image->columns*resizedRows*number_channels;
    tempImageBuffer=CreateOpenCLBuffer(device,CL_MEM_READ_WRITE,length*
      sizeof(CLQuantum),(void *) NULL);
    if (tempImageBuffer == (cl_mem) NULL)
    {
      (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
        ResourceLimitWarning,"CreateOpenCLBuffer failed.",".");
      goto cleanup;
    }

    outputReady=resizeVerticalFilter(device,queue,image,filteredImage,
      imageBuffer,number_channels,(cl_uint) image->columns,
      (cl_int) image->rows,tempImageBuffer,(cl_uint) image->columns,
      (cl_uint) resizedRows,resizeFilter,cubicCoefficientsBuffer,yFactor,
      exception);
    if (outputReady == MagickFalse)
      goto cleanup;

    outputReady=resizeHorizontalFilter(device,queue,image,filteredImage,
      tempImageBuffer,number_channels,(cl_uint) image->columns,
      (cl_uint) resizedRows,filteredImageBuffer,(cl_uint) resizedColumns,
      (cl_uint) resizedRows,resizeFilter,cubicCoefficientsBuffer,xFactor,
      exception);
    if (outputReady == MagickFalse)
      goto cleanup;
  }

cleanup:

  if (tempImageBuffer != (cl_mem) NULL)
    ReleaseOpenCLMemObject(tempImageBuffer);
  if (cubicCoefficientsBuffer != (cl_mem) NULL)
    ReleaseOpenCLMemObject(cubicCoefficientsBuffer);
  if (queue != (cl_command_queue) NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != (MagickCLDevice) NULL)
    ReleaseOpenCLDevice(device);
  if ((outputReady == MagickFalse) && (filteredImage != (Image *) NULL))
    filteredImage=DestroyImage(filteredImage);

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

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateCondition(image) == MagickFalse)
    return((Image *) NULL);

  if ((gpuSupportedResizeWeighting(GetResizeFilterWeightingType(
         resizeFilter)) == MagickFalse) ||
      (gpuSupportedResizeWeighting(GetResizeFilterWindowWeightingType(
         resizeFilter)) == MagickFalse))
    return((Image *) NULL);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return((Image *) NULL);

  filteredImage=ComputeResizeImage(image,clEnv,resizedColumns,resizedRows,
    resizeFilter,exception);
  return(filteredImage);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A c c e l e r a t e R o t a t i o n a l B l u r I m a g e               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static Image* ComputeRotationalBlurImage(const Image *image,MagickCLEnv clEnv,
  const double angle,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_float2
    blurCenter;

  cl_int
    status;

  cl_mem
    cosThetaBuffer,
    filteredImageBuffer,
    imageBuffer,
    sinThetaBuffer;

  cl_kernel
    rotationalBlurKernel;

  cl_uint
    cossin_theta_size,
    number_channels;

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

  MagickCLDevice
    device;

  size_t
    gsize[2],
    i;

  filteredImage=NULL;
  sinThetaBuffer=NULL;
  cosThetaBuffer=NULL;
  rotationalBlurKernel=NULL;
  outputReady=MagickFalse;

  device=RequestOpenCLDevice(clEnv);
  queue=AcquireOpenCLCommandQueue(device);
  filteredImage=cloneImage(image,exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;
  if (filteredImage->number_channels != image->number_channels)
    goto cleanup;
  imageBuffer=GetAuthenticOpenCLBuffer(image,device,exception);
  if (imageBuffer == (cl_mem) NULL)
    goto cleanup;
  filteredImageBuffer=GetAuthenticOpenCLBuffer(filteredImage,device,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
    goto cleanup;

  blurCenter.x=(float) (image->columns-1)/2.0;
  blurCenter.y=(float) (image->rows-1)/2.0;
  blurRadius=hypot(blurCenter.x,blurCenter.y);
  cossin_theta_size=(unsigned int) fabs(4.0*DegreesToRadians(angle)*sqrt(
    (double) blurRadius)+2UL);

  cosThetaPtr=AcquireQuantumMemory(cossin_theta_size,sizeof(float));
  if (cosThetaPtr == (float *) NULL)
    goto cleanup;
  sinThetaPtr=AcquireQuantumMemory(cossin_theta_size,sizeof(float));
  if (sinThetaPtr == (float *) NULL)
  {
    cosThetaPtr=RelinquishMagickMemory(cosThetaPtr);
    goto cleanup;
  }

  theta=DegreesToRadians(angle)/(double) (cossin_theta_size-1);
  offset=theta*(float) (cossin_theta_size-1)/2.0;
  for (i=0; i < (ssize_t) cossin_theta_size; i++)
  {
    cosThetaPtr[i]=(float)cos((double) (theta*i-offset));
    sinThetaPtr[i]=(float)sin((double) (theta*i-offset));
  }

  sinThetaBuffer=CreateOpenCLBuffer(device,CL_MEM_READ_ONLY |
    CL_MEM_COPY_HOST_PTR,cossin_theta_size*sizeof(float),sinThetaPtr);
  sinThetaPtr=RelinquishMagickMemory(sinThetaPtr);
  cosThetaBuffer=CreateOpenCLBuffer(device,CL_MEM_READ_ONLY |
    CL_MEM_COPY_HOST_PTR,cossin_theta_size*sizeof(float),cosThetaPtr);
  cosThetaPtr=RelinquishMagickMemory(cosThetaPtr);
  if ((sinThetaBuffer == (cl_mem) NULL) || (cosThetaBuffer == (cl_mem) NULL))
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"CreateOpenCLBuffer failed.",".");
    goto cleanup;
  }

  rotationalBlurKernel=AcquireOpenCLKernel(device,"RotationalBlur");
  if (rotationalBlurKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  number_channels=(cl_uint) image->number_channels;

  i=0;
  status =SetOpenCLKernelArg(rotationalBlurKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  status|=SetOpenCLKernelArg(rotationalBlurKernel,i++,sizeof(cl_uint),&number_channels);
  status|=SetOpenCLKernelArg(rotationalBlurKernel,i++,sizeof(ChannelType), &image->channel_mask);
  status|=SetOpenCLKernelArg(rotationalBlurKernel,i++,sizeof(cl_float2), &blurCenter);
  status|=SetOpenCLKernelArg(rotationalBlurKernel,i++,sizeof(cl_mem),(void *)&cosThetaBuffer);
  status|=SetOpenCLKernelArg(rotationalBlurKernel,i++,sizeof(cl_mem),(void *)&sinThetaBuffer);
  status|=SetOpenCLKernelArg(rotationalBlurKernel,i++,sizeof(cl_uint), &cossin_theta_size);
  status|=SetOpenCLKernelArg(rotationalBlurKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"clEnv->library->clSetKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=image->columns;
  gsize[1]=image->rows;
  outputReady=EnqueueOpenCLKernel(queue,rotationalBlurKernel,2,
    (const size_t *) NULL,gsize,(const size_t *) NULL,image,filteredImage,
    MagickFalse,exception);

cleanup:

  if (sinThetaBuffer != (cl_mem) NULL)
    ReleaseOpenCLMemObject(sinThetaBuffer);
  if (cosThetaBuffer != (cl_mem) NULL)
    ReleaseOpenCLMemObject(cosThetaBuffer);
  if (rotationalBlurKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(rotationalBlurKernel);
  if (queue != (cl_command_queue) NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != (MagickCLDevice) NULL)
    ReleaseOpenCLDevice(device);
  if ((outputReady == MagickFalse) && (filteredImage != (Image *) NULL))
    filteredImage=DestroyImage(filteredImage);

  return(filteredImage);
}

MagickPrivate Image* AccelerateRotationalBlurImage(const Image *image,
  const double angle,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateCondition(image) == MagickFalse)
    return((Image *) NULL);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return((Image *) NULL);

  filteredImage=ComputeRotationalBlurImage(image,clEnv,angle,exception);
  return filteredImage;
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

static Image *ComputeUnsharpMaskImage(const Image *image,MagickCLEnv clEnv,
  const double radius,const double sigma,const double gain,
  const double threshold,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_int
    status;

  cl_kernel
    blurRowKernel,
    unsharpMaskBlurColumnKernel;

  cl_mem
    filteredImageBuffer,
    imageBuffer,
    imageKernelBuffer,
    tempImageBuffer;

  cl_uint
    imageColumns,
    imageRows,
    kernelWidth,
    number_channels;

  float
    fGain,
    fThreshold;

  Image
    *filteredImage;

  int
    chunkSize;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  MagickSizeType
    length;

  size_t
    gsize[2],
    i,
    lsize[2];

  filteredImage=NULL;
  tempImageBuffer=NULL;
  imageKernelBuffer=NULL;
  blurRowKernel=NULL;
  unsharpMaskBlurColumnKernel=NULL;
  outputReady=MagickFalse;

  device=RequestOpenCLDevice(clEnv);
  queue=AcquireOpenCLCommandQueue(device);
  filteredImage=cloneImage(image,exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;
  if (filteredImage->number_channels != image->number_channels)
    goto cleanup;
  imageBuffer=GetAuthenticOpenCLBuffer(image,device,exception);
  if (imageBuffer == (cl_mem) NULL)
    goto cleanup;
  filteredImageBuffer=GetAuthenticOpenCLBuffer(filteredImage,device,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
    goto cleanup;

  imageKernelBuffer=createKernelInfo(device,radius,sigma,&kernelWidth,
    exception);

  length=image->columns*image->rows;
  tempImageBuffer=CreateOpenCLBuffer(device,CL_MEM_READ_WRITE,length*
    sizeof(cl_float4),NULL);
  if (tempImageBuffer == (cl_mem) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"CreateOpenCLBuffer failed.",".");
    goto cleanup;
  }

  blurRowKernel=AcquireOpenCLKernel(device,"BlurRow");
  if (blurRowKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  unsharpMaskBlurColumnKernel=AcquireOpenCLKernel(device,
    "UnsharpMaskBlurColumn");
  if (unsharpMaskBlurColumnKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  number_channels=(cl_uint) image->number_channels;
  imageColumns=(cl_uint) image->columns;
  imageRows=(cl_uint) image->rows;

  chunkSize = 256;

  i=0;
  status =SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_uint),&number_channels);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(ChannelType),&image->channel_mask);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_uint),(void *)&kernelWidth);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_uint),(void *)&imageColumns);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_uint),(void *)&imageRows);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_float4)*(chunkSize+kernelWidth),(void *) NULL);
  status|=SetOpenCLKernelArg(blurRowKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"clEnv->library->clSetKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=chunkSize*((image->columns+chunkSize-1)/chunkSize);
  gsize[1]=image->rows;
  lsize[0]=chunkSize;
  lsize[1]=1;
  outputReady=EnqueueOpenCLKernel(queue,blurRowKernel,2,
    (const size_t *) NULL,gsize,lsize,image,filteredImage,MagickFalse,
    exception);

  chunkSize=256;
  fGain=(float) gain;
  fThreshold=(float) threshold;

  i=0;
  status =SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&tempImageBuffer);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_uint),&number_channels);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(ChannelType),&image->channel_mask);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_uint),(void *)&imageColumns);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_uint),(void *)&imageRows);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,(chunkSize+kernelWidth-1)*sizeof(cl_float4),NULL);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,kernelWidth*sizeof(float),NULL);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_uint),(void *)&kernelWidth);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(float),(void *)&fGain);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(float),(void *)&fThreshold);
  status|=SetOpenCLKernelArg(unsharpMaskBlurColumnKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"clEnv->library->clSetKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=image->columns;
  gsize[1]=chunkSize*((image->rows+chunkSize-1)/chunkSize);
  lsize[0]=1;
  lsize[1]=chunkSize;
  outputReady=EnqueueOpenCLKernel(queue,unsharpMaskBlurColumnKernel,2,
    (const size_t *) NULL,gsize,lsize,image,filteredImage,MagickFalse,
    exception);

cleanup:

  if (tempImageBuffer != (cl_mem) NULL)
    ReleaseOpenCLMemObject(tempImageBuffer);
  if (imageKernelBuffer != (cl_mem) NULL)
    ReleaseOpenCLMemObject(imageKernelBuffer);
  if (blurRowKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(blurRowKernel);
  if (unsharpMaskBlurColumnKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(unsharpMaskBlurColumnKernel);
  if (queue != (cl_command_queue) NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != (MagickCLDevice) NULL)
    ReleaseOpenCLDevice(device);
  if ((outputReady == MagickFalse) && (filteredImage != (Image *) NULL))
    filteredImage=DestroyImage(filteredImage);

  return(filteredImage);
}

static Image *ComputeUnsharpMaskImageSingle(const Image *image,
  MagickCLEnv clEnv,const double radius,const double sigma,const double gain,
  const double threshold,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  cl_int
    status;

  cl_kernel
    unsharpMaskKernel;

  cl_mem
    filteredImageBuffer,
    imageBuffer,
    imageKernelBuffer;

  cl_uint
    imageColumns,
    imageRows,
    kernelWidth,
    number_channels;

  float
    fGain,
    fThreshold;

  Image
    *filteredImage;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  size_t
    gsize[2],
    i,
    lsize[2];

  filteredImage=NULL;
  imageKernelBuffer=NULL;
  unsharpMaskKernel=NULL;
  outputReady=MagickFalse;

  device=RequestOpenCLDevice(clEnv);
  queue=AcquireOpenCLCommandQueue(device);
  filteredImage=cloneImage(image,exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;
  if (filteredImage->number_channels != image->number_channels)
    goto cleanup;
  imageBuffer=GetAuthenticOpenCLBuffer(image,device,exception);
  if (imageBuffer == (cl_mem) NULL)
    goto cleanup;
  filteredImageBuffer=GetAuthenticOpenCLBuffer(filteredImage,device,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
    goto cleanup;

  imageKernelBuffer=createKernelInfo(device,radius,sigma,&kernelWidth,
    exception);

  unsharpMaskKernel=AcquireOpenCLKernel(device,"UnsharpMask");
  if (unsharpMaskKernel == NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  imageColumns=(cl_uint) image->columns;
  imageRows=(cl_uint) image->rows;
  number_channels=(cl_uint) image->number_channels;
  fGain=(float) gain;
  fThreshold=(float) threshold;

  i=0;
  status =SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  status|=SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(cl_uint),(void *)&number_channels);
  status|=SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(ChannelType),(void *)&image->channel_mask);
  status|=SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(cl_mem),(void *)&imageKernelBuffer);
  status|=SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(cl_uint),(void *)&kernelWidth);
  status|=SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(cl_uint),(void *)&imageColumns);
  status|=SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(cl_uint),(void *)&imageRows);
  status|=SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(cl_float4)*(8 * (32 + kernelWidth)),(void *) NULL);
  status|=SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(float),(void *)&fGain);
  status|=SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(float),(void *)&fThreshold);
  status|=SetOpenCLKernelArg(unsharpMaskKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
  if (status != CL_SUCCESS)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"SetOpenCLKernelArg failed.",".");
    goto cleanup;
  }

  gsize[0]=((image->columns + 7) / 8)*8;
  gsize[1]=((image->rows + 31) / 32)*32;
  lsize[0]=8;
  lsize[1]=32;
  outputReady=EnqueueOpenCLKernel(queue,unsharpMaskKernel,2,(const size_t *) NULL,
    gsize,lsize,image,filteredImage,MagickFalse,exception);

cleanup:

  if (imageKernelBuffer != (cl_mem) NULL)
    ReleaseOpenCLMemObject(imageKernelBuffer);
  if (unsharpMaskKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(unsharpMaskKernel);
  if (queue != (cl_command_queue) NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != (MagickCLDevice) NULL)
    ReleaseOpenCLDevice(device);
  if ((outputReady == MagickFalse) && (filteredImage != (Image *) NULL))
    filteredImage=DestroyImage(filteredImage);

  return(filteredImage);
}

MagickPrivate Image *AccelerateUnsharpMaskImage(const Image *image,
  const double radius,const double sigma,const double gain,
  const double threshold,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *) NULL);

  if (checkAccelerateCondition(image) == MagickFalse)
    return((Image *) NULL);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return((Image *) NULL);

  if (radius < 12.1)
    filteredImage=ComputeUnsharpMaskImageSingle(image,clEnv,radius,sigma,gain,
      threshold,exception);
  else
    filteredImage=ComputeUnsharpMaskImage(image,clEnv,radius,sigma,gain,
      threshold,exception);
  return(filteredImage);
}

static Image *ComputeWaveletDenoiseImage(const Image *image,MagickCLEnv clEnv,
  const double threshold,ExceptionInfo *exception)
{
  cl_command_queue
    queue;

  const cl_int
    PASSES=5;

  const int
    TILESIZE=64,
    PAD=1<<(PASSES-1),
    SIZE=TILESIZE-2*PAD;

  cl_float
    thresh;

  cl_int
    status;

  cl_kernel
    denoiseKernel;

  cl_mem
    filteredImageBuffer,
    imageBuffer;

  cl_uint
    number_channels,
    width,
    height,
    max_channels;

  Image
    *filteredImage;

  MagickBooleanType
    outputReady;

  MagickCLDevice
    device;

  size_t
    goffset[2],
    gsize[2],
    i,
    lsize[2],
    passes,
    x;

  filteredImage=NULL;
  denoiseKernel=NULL;
  queue=NULL;
  outputReady=MagickFalse;

  device=RequestOpenCLDevice(clEnv);
  /* Work around an issue on low end Intel devices */
  if (strcmp("Intel(R) HD Graphics",device->name) == 0)
    goto cleanup;
  queue=AcquireOpenCLCommandQueue(device);
  filteredImage=CloneImage(image,image->columns,image->rows,MagickTrue,
    exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;
  if (filteredImage->number_channels != image->number_channels)
    goto cleanup;
  imageBuffer=GetAuthenticOpenCLBuffer(image,device,exception);
  if (imageBuffer == (cl_mem) NULL)
    goto cleanup;
  filteredImageBuffer=GetAuthenticOpenCLBuffer(filteredImage,device,exception);
  if (filteredImageBuffer == (cl_mem) NULL)
    goto cleanup;

  denoiseKernel=AcquireOpenCLKernel(device,"WaveletDenoise");
  if (denoiseKernel == (cl_kernel) NULL)
  {
    (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
      ResourceLimitWarning,"AcquireOpenCLKernel failed.",".");
    goto cleanup;
  }

  number_channels=(cl_uint)image->number_channels;
  width=(cl_uint)image->columns;
  height=(cl_uint)image->rows;
  max_channels=number_channels;
  if ((max_channels == 4) || (max_channels == 2))
    max_channels=max_channels-1;
  thresh=threshold;
  passes=(((1.0f*image->columns)*image->rows)+1999999.0f)/2000000.0f;
  passes=(passes < 1) ? 1 : passes;

  i=0;
  status =SetOpenCLKernelArg(denoiseKernel,i++,sizeof(cl_mem),(void *)&imageBuffer);
  status|=SetOpenCLKernelArg(denoiseKernel,i++,sizeof(cl_mem),(void *)&filteredImageBuffer);
  status|=SetOpenCLKernelArg(denoiseKernel,i++,sizeof(cl_uint),(void *)&number_channels);
  status|=SetOpenCLKernelArg(denoiseKernel,i++,sizeof(cl_uint),(void *)&max_channels);
  status|=SetOpenCLKernelArg(denoiseKernel,i++,sizeof(cl_float),(void *)&thresh);
  status|=SetOpenCLKernelArg(denoiseKernel,i++,sizeof(cl_int),(void *)&PASSES);
  status|=SetOpenCLKernelArg(denoiseKernel,i++,sizeof(cl_uint),(void *)&width);
  status|=SetOpenCLKernelArg(denoiseKernel,i++,sizeof(cl_uint),(void *)&height);
  if (status != CL_SUCCESS)
    {
      (void) OpenCLThrowMagickException(device,exception,GetMagickModule(),
        ResourceLimitWarning,"SetOpenCLKernelArg failed.",".");
      goto cleanup;
    }

  for (x = 0; x < passes; ++x)
  {
    gsize[0]=((width+(SIZE-1))/SIZE)*TILESIZE;
    gsize[1]=((((height+(SIZE-1))/SIZE)+passes-1)/passes)*4;
    lsize[0]=TILESIZE;
    lsize[1]=4;
    goffset[0]=0;
    goffset[1]=x*gsize[1];

    outputReady=EnqueueOpenCLKernel(queue,denoiseKernel,2,goffset,gsize,lsize,
      image,filteredImage,MagickTrue,exception);
    if (outputReady == MagickFalse)
      break;
  }

cleanup:

  if (denoiseKernel != (cl_kernel) NULL)
    ReleaseOpenCLKernel(denoiseKernel);
  if (queue != (cl_command_queue) NULL)
    ReleaseOpenCLCommandQueue(device,queue);
  if (device != (MagickCLDevice) NULL)
    ReleaseOpenCLDevice(device);
  if ((outputReady == MagickFalse) && (filteredImage != (Image *) NULL))
    filteredImage=DestroyImage(filteredImage);

  return(filteredImage);
}

MagickPrivate Image *AccelerateWaveletDenoiseImage(const Image *image,
  const double threshold,ExceptionInfo *exception)
{
  Image
    *filteredImage;

  MagickCLEnv
    clEnv;

  assert(image != NULL);
  assert(exception != (ExceptionInfo *)NULL);

  if (checkAccelerateCondition(image) == MagickFalse)
    return((Image *) NULL);

  clEnv=getOpenCLEnvironment(exception);
  if (clEnv == (MagickCLEnv) NULL)
    return((Image *) NULL);

  filteredImage=ComputeWaveletDenoiseImage(image,clEnv,threshold,exception);

  return(filteredImage);
}
#endif /* MAGICKCORE_OPENCL_SUPPORT */

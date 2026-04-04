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
%                    MagickCore Metal Acceleration Methods                     %
%                                                                             %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#include "MagickCore/studio.h"
#include "MagickCore/accelerate-private.h"
#include "MagickCore/accelerate-kernels-metal.h"
#include "MagickCore/artifact.h"
#include "MagickCore/cache.h"
#include "MagickCore/cache-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/memory_.h"
#include "MagickCore/metal-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resize-private.h"

#if defined(MAGICKCORE_METAL_SUPPORT)

/*
  AcquirePixelCacheBuffer — creates an MTLBuffer from the image's pixel cache.
  Tries zero-copy first (newBufferWithBytesNoCopy), falls back to a copy.
  Sets *is_nocopy to indicate which path was taken.
*/
static void *AcquirePixelCacheBuffer(MagickMetalDevice device, Image *image,
  MagickBooleanType *is_nocopy, ExceptionInfo *exception)
{
  MagickSizeType
    cache_length;

  void
    *pixels,
    *buffer;

  pixels = GetPixelCachePixels(image, &cache_length, exception);
  if (pixels == NULL)
    return NULL;

  /* Try zero-copy first */
  buffer = AcquireMetalBufferNoCopy(device, pixels, (size_t) cache_length);
  if (buffer != NULL)
  {
    *is_nocopy = MagickTrue;
    return buffer;
  }

  /* Fall back to copy */
  buffer = AcquireMetalBuffer(device, (size_t) cache_length, pixels);
  *is_nocopy = MagickFalse;
  return buffer;
}

/*
  SyncPixelCacheFromBuffer — copies GPU results back to the pixel cache
  when we couldn't use the zero-copy path.
*/
static void SyncPixelCacheFromBuffer(Image *image, void *metalBuffer,
  ExceptionInfo *exception)
{
  MagickSizeType
    cache_length;

  void
    *pixels,
    *gpu_data;

  pixels = GetPixelCachePixels(image, &cache_length, exception);
  gpu_data = GetMetalBufferContents(metalBuffer);
  if (pixels != NULL && gpu_data != NULL)
    memcpy(pixels, gpu_data, (size_t) cache_length);
}

MagickPrivate MagickBooleanType AccelerateContrastImageMetal(Image *image,
  const MagickBooleanType sharpen, ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *imageBuffer,
    *kernel;

  MagickBooleanType
    status,
    is_nocopy;

  status = MagickFalse;
  imageBuffer = NULL;
  kernel = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return MagickFalse;

  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return MagickFalse;

  /* Get writable pixel cache access */
  if (GetAuthenticPixels(image, 0, 0, image->columns, image->rows,
      exception) == (Quantum *) NULL)
    return MagickFalse;

  imageBuffer = AcquirePixelCacheBuffer(device, image, &is_nocopy, exception);
  if (imageBuffer == NULL)
    return MagickFalse;

  kernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalContrastKernelSrc, "Contrast");
  if (kernel == NULL)
    goto cleanup;

  {
    int
      sign_val;

    uint
      columns,
      number_channels;

    float
      quantum_scale;

    MetalKernelArg
      args[5];

    size_t
      local_work_size[2],
      global_work_size[2];

    sign_val = sharpen ? 1 : -1;
    columns = (uint) image->columns;
    number_channels = (uint) image->number_channels;
    quantum_scale = (float) QuantumScale;

    args[0] = MetalBufferArg(imageBuffer);
    args[1] = MetalBytesArg(&sign_val, sizeof(sign_val));
    args[2] = MetalBytesArg(&columns, sizeof(columns));
    args[3] = MetalBytesArg(&number_channels, sizeof(number_channels));
    args[4] = MetalBytesArg(&quantum_scale, sizeof(quantum_scale));

    local_work_size[0] = 16;
    local_work_size[1] = 16;
    global_work_size[0] = local_work_size[0] *
      ((image->columns + local_work_size[0] - 1) / local_work_size[0]);
    global_work_size[1] = local_work_size[1] *
      ((image->rows + local_work_size[1] - 1) / local_work_size[1]);

    status = EnqueueMetalKernel2DEx(device, kernel, global_work_size,
      local_work_size, 0, args, 5);
  }

  if (status == MagickTrue)
  {
    if (is_nocopy == MagickFalse)
      SyncPixelCacheFromBuffer(image, imageBuffer, exception);
    SyncAuthenticPixels(image, exception);
  }

cleanup:
  if (imageBuffer != NULL)
    RelinquishMetalBuffer(imageBuffer);
  return status;
}

MagickPrivate MagickBooleanType AccelerateBlurImageMetal(const Image *image,
  const double radius, const double sigma, ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *imageBuffer,
    *tempBuffer,
    *outputBuffer,
    *filterBuffer,
    *blurRowKernel,
    *blurColumnKernel;

  MagickBooleanType
    status,
    is_nocopy;

  float
    *filter;

  size_t
    width;

  status = MagickFalse;
  imageBuffer = NULL;
  tempBuffer = NULL;
  outputBuffer = NULL;
  filterBuffer = NULL;
  filter = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return MagickFalse;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return MagickFalse;

  if (image->rows < 3 || image->columns < 3)
    return MagickFalse;

  /* Get writable pixel cache and create GPU buffer */
  if (GetAuthenticPixels((Image *) image, 0, 0, image->columns, image->rows,
      exception) == (Quantum *) NULL)
    return MagickFalse;

  imageBuffer = AcquirePixelCacheBuffer(device, (Image *) image, &is_nocopy,
    exception);
  if (imageBuffer == NULL)
    return MagickFalse;

  {
    MagickSizeType
      cache_length;

    GetPixelCachePixels((Image *) image, &cache_length, exception);
    tempBuffer = AcquireMetalBuffer(device, (size_t) cache_length, NULL);
    outputBuffer = AcquireMetalBuffer(device, (size_t) cache_length, NULL);
    if (tempBuffer == NULL || outputBuffer == NULL)
      goto cleanup;
  }

  /* Calculate kernel width */
  if (radius > 0.0)
    width = (size_t)(2.0 * ceil(radius) + 1.0);
  else
    width = (size_t)(2.0 * ceil(3.0 * sigma) + 1.0);
  if (width < 3) width = 3;
  if ((width % 2) == 0) width++;

  filter = (float *) AcquireMagickMemory(width * sizeof(float));
  if (filter == NULL)
    goto cleanup;
  {
    double
      normalize,
      x;

    ssize_t
      half_width,
      i;

    half_width = (ssize_t)(width - 1) / 2;
    normalize = 0.0;
    for (i = 0; i < (ssize_t) width; i++)
    {
      x = (double)(i - half_width);
      filter[i] = (float) exp(-(x * x) / (2.0 * sigma * sigma));
      normalize += filter[i];
    }
    for (i = 0; i < (ssize_t) width; i++)
      filter[i] /= (float) normalize;
  }

  filterBuffer = AcquireMetalBuffer(device, width * sizeof(float), filter);
  if (filterBuffer == NULL)
    goto cleanup;

  blurRowKernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalBlurKernelSrc, "BlurRow");
  blurColumnKernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalBlurKernelSrc, "BlurColumn");
  if (blurRowKernel == NULL || blurColumnKernel == NULL)
    goto cleanup;

  {
    uint
      width_uint,
      columns_uint,
      rows_uint,
      nc;

    size_t
      chunkSize,
      shared_mem,
      row_local[2],
      row_global[2],
      col_local[2],
      col_global[2];

    MetalKernelArg
      row_args[7],
      col_args[7];

    width_uint = (uint) width;
    columns_uint = (uint) image->columns;
    rows_uint = (uint) image->rows;
    nc = (uint) image->number_channels;
    chunkSize = 256;
    shared_mem = (chunkSize + width) * nc * sizeof(float);

    /* BlurRow: image -> temp */
    row_args[0] = MetalBufferArg(imageBuffer);
    row_args[1] = MetalBufferArg(tempBuffer);
    row_args[2] = MetalBufferArg(filterBuffer);
    row_args[3] = MetalBytesArg(&width_uint, sizeof(width_uint));
    row_args[4] = MetalBytesArg(&columns_uint, sizeof(columns_uint));
    row_args[5] = MetalBytesArg(&rows_uint, sizeof(rows_uint));
    row_args[6] = MetalBytesArg(&nc, sizeof(nc));

    row_local[0] = chunkSize;
    row_local[1] = 1;
    row_global[0] = chunkSize *
      ((image->columns + chunkSize - 1) / chunkSize);
    row_global[1] = image->rows;

    status = EnqueueMetalKernel2DEx(device, blurRowKernel, row_global,
      row_local, shared_mem, row_args, 7);
    if (status != MagickTrue) goto cleanup;

    /* BlurColumn: temp -> output */
    col_args[0] = MetalBufferArg(tempBuffer);
    col_args[1] = MetalBufferArg(outputBuffer);
    col_args[2] = MetalBufferArg(filterBuffer);
    col_args[3] = MetalBytesArg(&width_uint, sizeof(width_uint));
    col_args[4] = MetalBytesArg(&columns_uint, sizeof(columns_uint));
    col_args[5] = MetalBytesArg(&rows_uint, sizeof(rows_uint));
    col_args[6] = MetalBytesArg(&nc, sizeof(nc));

    col_local[0] = 1;
    col_local[1] = chunkSize;
    col_global[0] = image->columns;
    col_global[1] = chunkSize *
      ((image->rows + chunkSize - 1) / chunkSize);

    status = EnqueueMetalKernel2DEx(device, blurColumnKernel, col_global,
      col_local, shared_mem, col_args, 7);
  }

  if (status == MagickTrue)
  {
    MagickSizeType
      cache_length;

    void
      *pixels,
      *gpu_data;

    /* Copy output back to pixel cache */
    pixels = GetPixelCachePixels((Image *) image, &cache_length, exception);
    gpu_data = GetMetalBufferContents(outputBuffer);
    if (pixels != NULL && gpu_data != NULL)
      memcpy(pixels, gpu_data, (size_t) cache_length);
    SyncAuthenticPixels((Image *) image, exception);
  }

cleanup:
  if (outputBuffer != NULL) RelinquishMetalBuffer(outputBuffer);
  if (tempBuffer != NULL) RelinquishMetalBuffer(tempBuffer);
  if (imageBuffer != NULL) RelinquishMetalBuffer(imageBuffer);
  if (filterBuffer != NULL) RelinquishMetalBuffer(filterBuffer);
  if (filter != NULL) RelinquishMagickMemory(filter);
  return status;
}

MagickPrivate MagickBooleanType AccelerateGrayscaleImageMetal(Image *image,
  const PixelIntensityMethod method, ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *imageBuffer,
    *kernel;

  MagickBooleanType
    status,
    is_nocopy;

  status = MagickFalse;
  imageBuffer = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return MagickFalse;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return MagickFalse;

  if (GetAuthenticPixels(image, 0, 0, image->columns, image->rows,
      exception) == (Quantum *) NULL)
    return MagickFalse;

  imageBuffer = AcquirePixelCacheBuffer(device, image, &is_nocopy, exception);
  if (imageBuffer == NULL)
    return MagickFalse;

  kernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalGrayscaleKernelSrc, "Grayscale");
  if (kernel == NULL)
    goto cleanup;

  {
    uint
      colorspace,
      method_uint,
      columns,
      number_channels;

    float
      quantum_scale;

    MetalKernelArg
      args[6];

    size_t
      local_work_size[2],
      global_work_size[2];

    colorspace = (uint) image->colorspace;
    method_uint = (uint) method;
    columns = (uint) image->columns;
    number_channels = (uint) image->number_channels;
    quantum_scale = (float) QuantumScale;

    args[0] = MetalBufferArg(imageBuffer);
    args[1] = MetalBytesArg(&colorspace, sizeof(colorspace));
    args[2] = MetalBytesArg(&method_uint, sizeof(method_uint));
    args[3] = MetalBytesArg(&columns, sizeof(columns));
    args[4] = MetalBytesArg(&number_channels, sizeof(number_channels));
    args[5] = MetalBytesArg(&quantum_scale, sizeof(quantum_scale));

    local_work_size[0] = 16;
    local_work_size[1] = 16;
    global_work_size[0] = local_work_size[0] *
      ((image->columns + local_work_size[0] - 1) / local_work_size[0]);
    global_work_size[1] = local_work_size[1] *
      ((image->rows + local_work_size[1] - 1) / local_work_size[1]);

    status = EnqueueMetalKernel2DEx(device, kernel, global_work_size,
      local_work_size, 0, args, 6);
  }

  if (status == MagickTrue)
  {
    if (is_nocopy == MagickFalse)
      SyncPixelCacheFromBuffer(image, imageBuffer, exception);
    SyncAuthenticPixels(image, exception);
  }

cleanup:
  if (imageBuffer != NULL)
    RelinquishMetalBuffer(imageBuffer);
  return status;
}

MagickPrivate MagickBooleanType AccelerateFunctionImageMetal(Image *image,
  const MagickFunction function, const size_t number_parameters,
  const double *parameters, ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *imageBuffer,
    *paramsBuffer,
    *kernel;

  MagickBooleanType
    status,
    is_nocopy;

  float
    *float_params;

  size_t
    i;

  status = MagickFalse;
  imageBuffer = NULL;
  paramsBuffer = NULL;
  is_nocopy = MagickFalse;
  float_params = NULL;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return MagickFalse;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return MagickFalse;

  if (GetAuthenticPixels(image, 0, 0, image->columns, image->rows,
      exception) == (Quantum *) NULL)
    return MagickFalse;

  imageBuffer = AcquirePixelCacheBuffer(device, image, &is_nocopy, exception);
  if (imageBuffer == NULL)
    return MagickFalse;

  /* Convert double parameters to float for GPU */
  float_params = (float *) AcquireMagickMemory(
    number_parameters * sizeof(float));
  if (float_params == NULL)
    goto cleanup;
  for (i = 0; i < number_parameters; i++)
    float_params[i] = (float) parameters[i];

  paramsBuffer = AcquireMetalBuffer(device,
    number_parameters * sizeof(float), float_params);
  if (paramsBuffer == NULL)
    goto cleanup;

  kernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalFunctionKernelSrc, "ComputeFunction");
  if (kernel == NULL)
    goto cleanup;

  {
    uint
      func,
      num_params,
      columns,
      number_channels;

    float
      quantum_scale;

    MetalKernelArg
      args[7];

    size_t
      local_work_size[2],
      global_work_size[2];

    func = (uint) function;
    num_params = (uint) number_parameters;
    columns = (uint) image->columns;
    number_channels = (uint) image->number_channels;
    quantum_scale = (float) QuantumScale;

    args[0] = MetalBufferArg(imageBuffer);
    args[1] = MetalBytesArg(&func, sizeof(func));
    args[2] = MetalBytesArg(&num_params, sizeof(num_params));
    args[3] = MetalBufferArg(paramsBuffer);
    args[4] = MetalBytesArg(&columns, sizeof(columns));
    args[5] = MetalBytesArg(&number_channels, sizeof(number_channels));
    args[6] = MetalBytesArg(&quantum_scale, sizeof(quantum_scale));

    local_work_size[0] = 16;
    local_work_size[1] = 16;
    global_work_size[0] = local_work_size[0] *
      ((image->columns + local_work_size[0] - 1) / local_work_size[0]);
    global_work_size[1] = local_work_size[1] *
      ((image->rows + local_work_size[1] - 1) / local_work_size[1]);

    status = EnqueueMetalKernel2DEx(device, kernel, global_work_size,
      local_work_size, 0, args, 7);
  }

  if (status == MagickTrue)
  {
    if (is_nocopy == MagickFalse)
      SyncPixelCacheFromBuffer(image, imageBuffer, exception);
    SyncAuthenticPixels(image, exception);
  }

cleanup:
  if (paramsBuffer != NULL)
    RelinquishMetalBuffer(paramsBuffer);
  if (imageBuffer != NULL)
    RelinquishMetalBuffer(imageBuffer);
  if (float_params != NULL)
    RelinquishMagickMemory(float_params);
  return status;
}

MagickPrivate MagickBooleanType AccelerateModulateImageMetal(Image *image,
  const double percent_brightness, const double percent_hue,
  const double percent_saturation, const ColorspaceType colorspace,
  ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *imageBuffer,
    *kernel;

  MagickBooleanType
    status,
    is_nocopy;

  status = MagickFalse;
  imageBuffer = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return MagickFalse;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return MagickFalse;

  if (GetAuthenticPixels(image, 0, 0, image->columns, image->rows,
      exception) == (Quantum *) NULL)
    return MagickFalse;

  imageBuffer = AcquirePixelCacheBuffer(device, image, &is_nocopy, exception);
  if (imageBuffer == NULL)
    return MagickFalse;

  kernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalModulateKernelSrc, "Modulate");
  if (kernel == NULL)
    goto cleanup;

  {
    float
      brightness,
      hue,
      saturation,
      quantum_scale;

    uint
      columns,
      number_channels;

    MetalKernelArg
      args[7];

    size_t
      local_work_size[2],
      global_work_size[2];

    brightness = (float) percent_brightness;
    hue = (float) percent_hue;
    saturation = (float) percent_saturation;
    columns = (uint) image->columns;
    number_channels = (uint) image->number_channels;
    quantum_scale = (float) QuantumScale;

    args[0] = MetalBufferArg(imageBuffer);
    args[1] = MetalBytesArg(&brightness, sizeof(brightness));
    args[2] = MetalBytesArg(&hue, sizeof(hue));
    args[3] = MetalBytesArg(&saturation, sizeof(saturation));
    args[4] = MetalBytesArg(&columns, sizeof(columns));
    args[5] = MetalBytesArg(&number_channels, sizeof(number_channels));
    args[6] = MetalBytesArg(&quantum_scale, sizeof(quantum_scale));

    local_work_size[0] = 16;
    local_work_size[1] = 16;
    global_work_size[0] = local_work_size[0] *
      ((image->columns + local_work_size[0] - 1) / local_work_size[0]);
    global_work_size[1] = local_work_size[1] *
      ((image->rows + local_work_size[1] - 1) / local_work_size[1]);

    status = EnqueueMetalKernel2DEx(device, kernel, global_work_size,
      local_work_size, 0, args, 7);
  }

  if (status == MagickTrue)
  {
    if (is_nocopy == MagickFalse)
      SyncPixelCacheFromBuffer(image, imageBuffer, exception);
    SyncAuthenticPixels(image, exception);
  }

cleanup:
  if (imageBuffer != NULL)
    RelinquishMetalBuffer(imageBuffer);
  return status;
}

MagickPrivate Image *AccelerateMotionBlurImageMetal(const Image *image,
  const double *kernel, const size_t width, const OffsetInfo *offset,
  ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *inputBuffer,
    *outputBuffer,
    *filterBuffer,
    *offsetBuffer,
    *pipeline;

  MagickBooleanType
    status,
    is_nocopy;

  Image
    *filteredImage;

  float
    *float_filter;

  int
    *int_offsets;

  size_t
    i;

  inputBuffer = NULL;
  outputBuffer = NULL;
  filterBuffer = NULL;
  offsetBuffer = NULL;
  filteredImage = NULL;
  float_filter = NULL;
  int_offsets = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return (Image *) NULL;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return (Image *) NULL;

  inputBuffer = AcquirePixelCacheBuffer(device, (Image *) image, &is_nocopy,
    exception);
  if (inputBuffer == NULL)
    return (Image *) NULL;

  filteredImage = CloneImage(image, 0, 0, MagickTrue, exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;

  {
    MagickSizeType
      cache_length;

    void
      *out_pixels;

    out_pixels = GetPixelCachePixels(filteredImage, &cache_length, exception);
    if (out_pixels == NULL)
      goto cleanup;
    outputBuffer = AcquireMetalBuffer(device, (size_t) cache_length, NULL);
    if (outputBuffer == NULL)
      goto cleanup;
  }

  /* Convert double filter to float */
  float_filter = (float *) AcquireMagickMemory(width * sizeof(float));
  if (float_filter == NULL)
    goto cleanup;
  for (i = 0; i < width; i++)
    float_filter[i] = (float) kernel[i];
  filterBuffer = AcquireMetalBuffer(device, width * sizeof(float),
    float_filter);
  if (filterBuffer == NULL)
    goto cleanup;

  /* Convert OffsetInfo array to int2 (x,y pairs) */
  int_offsets = (int *) AcquireMagickMemory(width * 2 * sizeof(int));
  if (int_offsets == NULL)
    goto cleanup;
  for (i = 0; i < width; i++)
  {
    int_offsets[i * 2] = (int) offset[i].x;
    int_offsets[i * 2 + 1] = (int) offset[i].y;
  }
  offsetBuffer = AcquireMetalBuffer(device, width * 2 * sizeof(int),
    int_offsets);
  if (offsetBuffer == NULL)
    goto cleanup;

  pipeline = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalMotionBlurKernelSrc, "MotionBlur");
  if (pipeline == NULL)
    goto cleanup;

  {
    uint
      imageWidth,
      imageHeight,
      width_uint,
      number_channels,
      has_alpha;

    MetalKernelArg
      args[9];

    size_t
      local_work_size[2],
      global_work_size[2];

    imageWidth = (uint) image->columns;
    imageHeight = (uint) image->rows;
    width_uint = (uint) width;
    number_channels = (uint) image->number_channels;
    has_alpha = (image->number_channels == 4 ||
      image->number_channels == 2) ? 1 : 0;

    args[0] = MetalBufferArg(inputBuffer);
    args[1] = MetalBufferArg(outputBuffer);
    args[2] = MetalBufferArg(filterBuffer);
    args[3] = MetalBufferArg(offsetBuffer);
    args[4] = MetalBytesArg(&imageWidth, sizeof(imageWidth));
    args[5] = MetalBytesArg(&imageHeight, sizeof(imageHeight));
    args[6] = MetalBytesArg(&width_uint, sizeof(width_uint));
    args[7] = MetalBytesArg(&number_channels, sizeof(number_channels));
    args[8] = MetalBytesArg(&has_alpha, sizeof(has_alpha));

    local_work_size[0] = 16;
    local_work_size[1] = 16;
    global_work_size[0] = local_work_size[0] *
      ((image->columns + local_work_size[0] - 1) / local_work_size[0]);
    global_work_size[1] = local_work_size[1] *
      ((image->rows + local_work_size[1] - 1) / local_work_size[1]);

    status = EnqueueMetalKernel2DEx(device, pipeline, global_work_size,
      local_work_size, 0, args, 9);
  }

  if (status == MagickTrue)
  {
    /* Copy output buffer to filtered image's pixel cache */
    if (GetAuthenticPixels(filteredImage, 0, 0, filteredImage->columns,
        filteredImage->rows, exception) != (Quantum *) NULL)
    {
      SyncPixelCacheFromBuffer(filteredImage, outputBuffer, exception);
      SyncAuthenticPixels(filteredImage, exception);
    }
  }
  else
  {
    filteredImage = DestroyImage(filteredImage);
    filteredImage = (Image *) NULL;
  }

cleanup:
  if (offsetBuffer != NULL) RelinquishMetalBuffer(offsetBuffer);
  if (filterBuffer != NULL) RelinquishMetalBuffer(filterBuffer);
  if (outputBuffer != NULL) RelinquishMetalBuffer(outputBuffer);
  if (inputBuffer != NULL) RelinquishMetalBuffer(inputBuffer);
  if (float_filter != NULL) RelinquishMagickMemory(float_filter);
  if (int_offsets != NULL) RelinquishMagickMemory(int_offsets);
  return filteredImage;
}

MagickPrivate Image *AccelerateRotationalBlurImageMetal(const Image *image,
  const double angle, ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *inputBuffer,
    *outputBuffer,
    *cosThetaBuffer,
    *sinThetaBuffer,
    *pipeline;

  MagickBooleanType
    status,
    is_nocopy;

  Image
    *filteredImage;

  float
    *cosTheta,
    *sinTheta;

  inputBuffer = NULL;
  outputBuffer = NULL;
  cosThetaBuffer = NULL;
  sinThetaBuffer = NULL;
  filteredImage = NULL;
  cosTheta = NULL;
  sinTheta = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return (Image *) NULL;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return (Image *) NULL;

  inputBuffer = AcquirePixelCacheBuffer(device, (Image *) image, &is_nocopy,
    exception);
  if (inputBuffer == NULL)
    return (Image *) NULL;

  filteredImage = CloneImage(image, 0, 0, MagickTrue, exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;

  {
    MagickSizeType
      cache_length;

    void
      *out_pixels;

    out_pixels = GetPixelCachePixels(filteredImage, &cache_length, exception);
    if (out_pixels == NULL)
      goto cleanup;
    outputBuffer = AcquireMetalBuffer(device, (size_t) cache_length, NULL);
    if (outputBuffer == NULL)
      goto cleanup;
  }

  /* Build sin/cos lookup tables */
  {
    float
      blurCenter[2],
      blurCenter_x,
      blurCenter_y,
      blurRadius,
      off,
      theta;

    uint
      columns,
      cossin_theta_size,
      number_channels,
      rows;

    MetalKernelArg
      args[9];

    size_t
      i,
      local_work_size[2],
      global_work_size[2];

    blurCenter_x = (float)((image->columns - 1) / 2.0);
    blurCenter_y = (float)((image->rows - 1) / 2.0);
    blurRadius = (float) hypot(blurCenter_x, blurCenter_y);
    cossin_theta_size = (uint) fabs(4.0 * DegreesToRadians(angle) *
      sqrt((double) blurRadius) + 2.0);
    if (cossin_theta_size < 2)
      cossin_theta_size = 2;

    cosTheta = (float *) AcquireMagickMemory(cossin_theta_size * sizeof(float));
    sinTheta = (float *) AcquireMagickMemory(cossin_theta_size * sizeof(float));
    if (cosTheta == NULL || sinTheta == NULL)
      goto cleanup;

    theta = (float)(DegreesToRadians(angle) /
      (double)(cossin_theta_size - 1));
    off = theta * (float)((cossin_theta_size - 1) / 2.0);
    for (i = 0; i < cossin_theta_size; i++)
    {
      cosTheta[i] = (float) cos((double)(theta * i - off));
      sinTheta[i] = (float) sin((double)(theta * i - off));
    }

    cosThetaBuffer = AcquireMetalBuffer(device,
      cossin_theta_size * sizeof(float), cosTheta);
    sinThetaBuffer = AcquireMetalBuffer(device,
      cossin_theta_size * sizeof(float), sinTheta);
    if (cosThetaBuffer == NULL || sinThetaBuffer == NULL)
      goto cleanup;

    pipeline = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
      metalRotationalBlurKernelSrc, "RotationalBlur");
    if (pipeline == NULL)
      goto cleanup;

    blurCenter[0] = blurCenter_x;
    blurCenter[1] = blurCenter_y;
    columns = (uint) image->columns;
    rows = (uint) image->rows;
    number_channels = (uint) image->number_channels;

    args[0] = MetalBufferArg(inputBuffer);
    args[1] = MetalBufferArg(outputBuffer);
    args[2] = MetalBufferArg(cosThetaBuffer);
    args[3] = MetalBufferArg(sinThetaBuffer);
    args[4] = MetalBytesArg(&cossin_theta_size, sizeof(cossin_theta_size));
    args[5] = MetalBytesArg(blurCenter, sizeof(blurCenter));
    args[6] = MetalBytesArg(&columns, sizeof(columns));
    args[7] = MetalBytesArg(&rows, sizeof(rows));
    args[8] = MetalBytesArg(&number_channels, sizeof(number_channels));

    local_work_size[0] = 16;
    local_work_size[1] = 16;
    global_work_size[0] = local_work_size[0] *
      ((image->columns + local_work_size[0] - 1) / local_work_size[0]);
    global_work_size[1] = local_work_size[1] *
      ((image->rows + local_work_size[1] - 1) / local_work_size[1]);

    status = EnqueueMetalKernel2DEx(device, pipeline, global_work_size,
      local_work_size, 0, args, 9);
  }

  if (status == MagickTrue)
  {
    if (GetAuthenticPixels(filteredImage, 0, 0, filteredImage->columns,
        filteredImage->rows, exception) != (Quantum *) NULL)
    {
      SyncPixelCacheFromBuffer(filteredImage, outputBuffer, exception);
      SyncAuthenticPixels(filteredImage, exception);
    }
  }
  else
  {
    filteredImage = DestroyImage(filteredImage);
    filteredImage = (Image *) NULL;
  }

cleanup:
  if (sinThetaBuffer != NULL) RelinquishMetalBuffer(sinThetaBuffer);
  if (cosThetaBuffer != NULL) RelinquishMetalBuffer(cosThetaBuffer);
  if (outputBuffer != NULL) RelinquishMetalBuffer(outputBuffer);
  if (inputBuffer != NULL) RelinquishMetalBuffer(inputBuffer);
  if (cosTheta != NULL) RelinquishMagickMemory(cosTheta);
  if (sinTheta != NULL) RelinquishMagickMemory(sinTheta);
  return filteredImage;
}

MagickPrivate Image *AccelerateDespeckleImageMetal(const Image *image,
  ExceptionInfo *exception)
{
  static const int X[4] = {0, 1, 1, -1};
  static const int Y[4] = {1, 0, 1, 1};

  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *bufferA,
    *bufferB,
    *hullPass1,
    *hullPass2;

  MagickBooleanType
    status,
    is_nocopy;

  Image
    *filteredImage;

  bufferA = NULL;
  bufferB = NULL;
  filteredImage = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return (Image *) NULL;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return (Image *) NULL;

  filteredImage = CloneImage(image, 0, 0, MagickTrue, exception);
  if (filteredImage == (Image *) NULL)
    return (Image *) NULL;

  if (GetAuthenticPixels(filteredImage, 0, 0, filteredImage->columns,
      filteredImage->rows, exception) == (Quantum *) NULL)
    goto cleanup_fail;

  /* Buffer A starts with the image data */
  bufferA = AcquirePixelCacheBuffer(device, filteredImage, &is_nocopy,
    exception);
  if (bufferA == NULL)
    goto cleanup_fail;

  {
    MagickSizeType
      cache_length;

    GetPixelCachePixels(filteredImage, &cache_length, exception);
    bufferB = AcquireMetalBuffer(device, (size_t) cache_length, NULL);
    if (bufferB == NULL)
      goto cleanup_fail;
  }

  hullPass1 = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalDespeckleKernelSrc, "HullPass1");
  hullPass2 = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalDespeckleKernelSrc, "HullPass2");
  if (hullPass1 == NULL || hullPass2 == NULL)
    goto cleanup_fail;

  {
    uint
      imageWidth,
      imageHeight,
      number_channels;

    float
      threshold,
      increment;

    size_t
      local_work_size[2],
      global_work_size[2];

    int
      k,
      offsets[2][2],
      p,
      polarities[2];

    imageWidth = (uint) image->columns;
    imageHeight = (uint) image->rows;
    number_channels = (uint) image->number_channels;
    /* Q16 HDRI: ScaleCharToQuantum(2) = 514.0, ScaleCharToQuantum(1) = 257.0 */
    threshold = (float)(QuantumRange / 65535.0 * 514.0);
    increment = (float)(QuantumRange / 65535.0 * 257.0);

    local_work_size[0] = 16;
    local_work_size[1] = 16;
    global_work_size[0] = local_work_size[0] *
      ((image->columns + local_work_size[0] - 1) / local_work_size[0]);
    global_work_size[1] = local_work_size[1] *
      ((image->rows + local_work_size[1] - 1) / local_work_size[1]);

    status = MagickTrue;
    for (k = 0; k < 4 && status == MagickTrue; k++)
    {
      offsets[0][0] = X[k];
      offsets[0][1] = Y[k];
      offsets[1][0] = -X[k];
      offsets[1][1] = -Y[k];
      polarities[0] = 1;
      polarities[1] = -1;

      for (p = 0; p < 2 && status == MagickTrue; p++)
      {
        /* Pass 1: A -> B */
        {
          int
            offset_val[2],
            polarity;

          MetalKernelArg
            args[9];

          offset_val[0] = offsets[p][0];
          offset_val[1] = offsets[p][1];
          polarity = polarities[p];
          args[0] = MetalBufferArg(bufferA);
          args[1] = MetalBufferArg(bufferB);
          args[2] = MetalBytesArg(&imageWidth, sizeof(imageWidth));
          args[3] = MetalBytesArg(&imageHeight, sizeof(imageHeight));
          args[4] = MetalBytesArg(offset_val, sizeof(offset_val));
          args[5] = MetalBytesArg(&polarity, sizeof(polarity));
          args[6] = MetalBytesArg(&number_channels, sizeof(number_channels));
          args[7] = MetalBytesArg(&threshold, sizeof(threshold));
          args[8] = MetalBytesArg(&increment, sizeof(increment));
          status = EnqueueMetalKernel2DEx(device, hullPass1, global_work_size,
            local_work_size, 0, args, 9);
        }

        if (status != MagickTrue) break;

        /* Pass 2: B -> A */
        {
          int
            offset_val[2],
            polarity;

          MetalKernelArg
            args[9];

          offset_val[0] = -offsets[p][0];
          offset_val[1] = -offsets[p][1];
          polarity = polarities[p];
          args[0] = MetalBufferArg(bufferB);
          args[1] = MetalBufferArg(bufferA);
          args[2] = MetalBytesArg(&imageWidth, sizeof(imageWidth));
          args[3] = MetalBytesArg(&imageHeight, sizeof(imageHeight));
          args[4] = MetalBytesArg(offset_val, sizeof(offset_val));
          args[5] = MetalBytesArg(&polarity, sizeof(polarity));
          args[6] = MetalBytesArg(&number_channels, sizeof(number_channels));
          args[7] = MetalBytesArg(&threshold, sizeof(threshold));
          args[8] = MetalBytesArg(&increment, sizeof(increment));
          status = EnqueueMetalKernel2DEx(device, hullPass2, global_work_size,
            local_work_size, 0, args, 9);
        }
      }
    }
  }

  if (status == MagickTrue)
  {
    if (is_nocopy == MagickFalse)
      SyncPixelCacheFromBuffer(filteredImage, bufferA, exception);
    SyncAuthenticPixels(filteredImage, exception);
  }
  else
    goto cleanup_fail;

  if (bufferB != NULL) RelinquishMetalBuffer(bufferB);
  if (bufferA != NULL) RelinquishMetalBuffer(bufferA);
  return filteredImage;

cleanup_fail:
  if (bufferB != NULL) RelinquishMetalBuffer(bufferB);
  if (bufferA != NULL) RelinquishMetalBuffer(bufferA);
  if (filteredImage != NULL) filteredImage = DestroyImage(filteredImage);
  return (Image *) NULL;
}

MagickPrivate Image *AccelerateUnsharpMaskImageMetal(const Image *image,
  const double radius, const double sigma, const double gain,
  const double threshold, ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *inputBuffer,
    *tempBuffer,
    *blurredBuffer,
    *outputBuffer,
    *blurRowKernel,
    *blurColumnKernel,
    *unsharpKernel,
    *filterBuffer;

  MagickBooleanType
    status,
    is_nocopy;

  Image
    *filteredImage;

  float
    *filter;

  size_t
    width;

  inputBuffer = NULL;
  tempBuffer = NULL;
  blurredBuffer = NULL;
  outputBuffer = NULL;
  filterBuffer = NULL;
  filteredImage = NULL;
  filter = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return (Image *) NULL;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return (Image *) NULL;

  inputBuffer = AcquirePixelCacheBuffer(device, (Image *) image, &is_nocopy,
    exception);
  if (inputBuffer == NULL)
    return (Image *) NULL;

  filteredImage = CloneImage(image, 0, 0, MagickTrue, exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;

  {
    MagickSizeType
      cache_length;

    GetPixelCachePixels((Image *) image, &cache_length, exception);
    tempBuffer = AcquireMetalBuffer(device, (size_t) cache_length, NULL);
    blurredBuffer = AcquireMetalBuffer(device, (size_t) cache_length, NULL);
    outputBuffer = AcquireMetalBuffer(device, (size_t) cache_length, NULL);
    if (tempBuffer == NULL || blurredBuffer == NULL || outputBuffer == NULL)
      goto cleanup;
  }

  /* Calculate Gaussian kernel */
  if (radius > 0.0)
    width = (size_t)(2.0 * ceil(radius) + 1.0);
  else
    width = (size_t)(2.0 * ceil(3.0 * sigma) + 1.0);
  if (width < 3) width = 3;
  if ((width % 2) == 0) width++;

  filter = (float *) AcquireMagickMemory(width * sizeof(float));
  if (filter == NULL)
    goto cleanup;
  {
    double
      normalize,
      x;

    ssize_t
      half_width,
      i;

    half_width = (ssize_t)(width - 1) / 2;
    normalize = 0.0;
    for (i = 0; i < (ssize_t) width; i++)
    {
      x = (double)(i - half_width);
      filter[i] = (float) exp(-(x * x) / (2.0 * sigma * sigma));
      normalize += filter[i];
    }
    for (i = 0; i < (ssize_t) width; i++)
      filter[i] /= (float) normalize;
  }

  filterBuffer = AcquireMetalBuffer(device, width * sizeof(float), filter);
  if (filterBuffer == NULL)
    goto cleanup;

  blurRowKernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalBlurKernelSrc, "BlurRow");
  blurColumnKernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalBlurKernelSrc, "BlurColumn");
  unsharpKernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalUnsharpMaskApplyKernelSrc, "UnsharpMaskApply");
  if (blurRowKernel == NULL || blurColumnKernel == NULL ||
      unsharpKernel == NULL)
    goto cleanup;

  {
    uint
      width_uint,
      columns_uint,
      rows_uint,
      nc;

    float
      fGain,
      fThreshold,
      qRange;

    size_t
      chunkSize,
      shared_mem,
      row_local[2],
      row_global[2],
      col_local[2],
      col_global[2],
      apply_local[2],
      apply_global[2];

    MetalKernelArg
      row_args[7],
      col_args[7],
      apply_args[9];

    width_uint = (uint) width;
    columns_uint = (uint) image->columns;
    rows_uint = (uint) image->rows;
    nc = (uint) image->number_channels;
    chunkSize = 256;
    shared_mem = (chunkSize + width) * nc * sizeof(float);

    /* Pass 1: BlurRow input -> temp */
    row_args[0] = MetalBufferArg(inputBuffer);
    row_args[1] = MetalBufferArg(tempBuffer);
    row_args[2] = MetalBufferArg(filterBuffer);
    row_args[3] = MetalBytesArg(&width_uint, sizeof(width_uint));
    row_args[4] = MetalBytesArg(&columns_uint, sizeof(columns_uint));
    row_args[5] = MetalBytesArg(&rows_uint, sizeof(rows_uint));
    row_args[6] = MetalBytesArg(&nc, sizeof(nc));

    row_local[0] = chunkSize;
    row_local[1] = 1;
    row_global[0] = chunkSize *
      ((image->columns + chunkSize - 1) / chunkSize);
    row_global[1] = image->rows;

    status = EnqueueMetalKernel2DEx(device, blurRowKernel, row_global,
      row_local, shared_mem, row_args, 7);

    if (status != MagickTrue) goto cleanup;

    /* Pass 2: BlurColumn temp -> blurred */
    col_args[0] = MetalBufferArg(tempBuffer);
    col_args[1] = MetalBufferArg(blurredBuffer);
    col_args[2] = MetalBufferArg(filterBuffer);
    col_args[3] = MetalBytesArg(&width_uint, sizeof(width_uint));
    col_args[4] = MetalBytesArg(&columns_uint, sizeof(columns_uint));
    col_args[5] = MetalBytesArg(&rows_uint, sizeof(rows_uint));
    col_args[6] = MetalBytesArg(&nc, sizeof(nc));

    col_local[0] = 1;
    col_local[1] = chunkSize;
    col_global[0] = image->columns;
    col_global[1] = chunkSize *
      ((image->rows + chunkSize - 1) / chunkSize);

    status = EnqueueMetalKernel2DEx(device, blurColumnKernel, col_global,
      col_local, shared_mem, col_args, 7);

    if (status != MagickTrue) goto cleanup;

    /* Pass 3: UnsharpMask apply (now using raw Quantum data) */
    fGain = (float) gain;
    fThreshold = (float) threshold;
    qRange = (float) QuantumRange;

    apply_args[0] = MetalBufferArg(inputBuffer);
    apply_args[1] = MetalBufferArg(blurredBuffer);
    apply_args[2] = MetalBufferArg(outputBuffer);
    apply_args[3] = MetalBytesArg(&columns_uint, sizeof(columns_uint));
    apply_args[4] = MetalBytesArg(&rows_uint, sizeof(rows_uint));
    apply_args[5] = MetalBytesArg(&nc, sizeof(nc));
    apply_args[6] = MetalBytesArg(&fGain, sizeof(fGain));
    apply_args[7] = MetalBytesArg(&fThreshold, sizeof(fThreshold));
    apply_args[8] = MetalBytesArg(&qRange, sizeof(qRange));

    apply_local[0] = 16;
    apply_local[1] = 16;
    apply_global[0] = apply_local[0] *
      ((image->columns + apply_local[0] - 1) / apply_local[0]);
    apply_global[1] = apply_local[1] *
      ((image->rows + apply_local[1] - 1) / apply_local[1]);

    status = EnqueueMetalKernel2DEx(device, unsharpKernel, apply_global,
      apply_local, 0, apply_args, 9);
  }

  if (status == MagickTrue)
  {
    if (GetAuthenticPixels(filteredImage, 0, 0, filteredImage->columns,
        filteredImage->rows, exception) != (Quantum *) NULL)
    {
      SyncPixelCacheFromBuffer(filteredImage, outputBuffer, exception);
      SyncAuthenticPixels(filteredImage, exception);
    }
  }
  else
  {
    filteredImage = DestroyImage(filteredImage);
    filteredImage = (Image *) NULL;
  }

cleanup:
  if (outputBuffer != NULL) RelinquishMetalBuffer(outputBuffer);
  if (blurredBuffer != NULL) RelinquishMetalBuffer(blurredBuffer);
  if (tempBuffer != NULL) RelinquishMetalBuffer(tempBuffer);
  if (inputBuffer != NULL) RelinquishMetalBuffer(inputBuffer);
  if (filterBuffer != NULL) RelinquishMetalBuffer(filterBuffer);
  if (filter != NULL) RelinquishMagickMemory(filter);
  return filteredImage;
}

MagickPrivate Image *AccelerateLocalContrastImageMetal(const Image *image,
  const double radius, const double strength, ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *inputBuffer,
    *tmpBuffer,
    *outputBuffer,
    *blurRowKernel,
    *applyKernel;

  MagickBooleanType
    status,
    is_nocopy;

  Image
    *filteredImage;

  inputBuffer = NULL;
  tmpBuffer = NULL;
  outputBuffer = NULL;
  filteredImage = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return (Image *) NULL;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return (Image *) NULL;

  inputBuffer = AcquirePixelCacheBuffer(device, (Image *) image, &is_nocopy,
    exception);
  if (inputBuffer == NULL)
    return (Image *) NULL;

  filteredImage = CloneImage(image, 0, 0, MagickTrue, exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;

  {
    MagickSizeType
      cache_length;

    GetPixelCachePixels((Image *) image, &cache_length, exception);
    /* tmpBuffer stores per-pixel luminance blur (1 float per pixel) */
    tmpBuffer = AcquireMetalBuffer(device,
      image->columns * image->rows * sizeof(float), NULL);
    outputBuffer = AcquireMetalBuffer(device, (size_t) cache_length, NULL);
    if (tmpBuffer == NULL || outputBuffer == NULL)
      goto cleanup;
  }

  blurRowKernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalLocalContrastKernelSrc, "LocalContrastBlurRow");
  applyKernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalLocalContrastKernelSrc, "LocalContrastBlurApplyColumn");
  if (blurRowKernel == NULL || applyKernel == NULL)
    goto cleanup;

  {
    int
      iRadius;

    uint
      imageWidth,
      imageHeight,
      number_channels;

    float
      fStrength;

    size_t
      max_dim,
      row_local[2],
      row_global[2],
      col_local[2],
      col_global[2];

    MetalKernelArg
      row_args[6],
      col_args[8];

    max_dim = image->columns > image->rows ? image->columns : image->rows;
    iRadius = (int)(max_dim * 0.002 * fabs(radius));
    if (iRadius < 1) iRadius = 1;
    imageWidth = (uint) image->columns;
    imageHeight = (uint) image->rows;
    number_channels = (uint) image->number_channels;

    /* Pass 1: BlurRow -- compute blurred luminance */
    row_args[0] = MetalBufferArg(inputBuffer);
    row_args[1] = MetalBufferArg(tmpBuffer);
    row_args[2] = MetalBytesArg(&iRadius, sizeof(iRadius));
    row_args[3] = MetalBytesArg(&imageWidth, sizeof(imageWidth));
    row_args[4] = MetalBytesArg(&imageHeight, sizeof(imageHeight));
    row_args[5] = MetalBytesArg(&number_channels, sizeof(number_channels));

    row_local[0] = 16;
    row_local[1] = 16;
    row_global[0] = row_local[0] *
      ((image->columns + row_local[0] - 1) / row_local[0]);
    row_global[1] = row_local[1] *
      ((image->rows + row_local[1] - 1) / row_local[1]);

    status = EnqueueMetalKernel2DEx(device, blurRowKernel, row_global,
      row_local, 0, row_args, 6);

    if (status != MagickTrue) goto cleanup;

    /* Pass 2: BlurApplyColumn -- vertical blur + apply contrast */
    fStrength = (float) strength;

    col_args[0] = MetalBufferArg(inputBuffer);
    col_args[1] = MetalBufferArg(outputBuffer);
    col_args[2] = MetalBufferArg(tmpBuffer);
    col_args[3] = MetalBytesArg(&iRadius, sizeof(iRadius));
    col_args[4] = MetalBytesArg(&fStrength, sizeof(fStrength));
    col_args[5] = MetalBytesArg(&imageWidth, sizeof(imageWidth));
    col_args[6] = MetalBytesArg(&imageHeight, sizeof(imageHeight));
    col_args[7] = MetalBytesArg(&number_channels, sizeof(number_channels));

    col_local[0] = 16;
    col_local[1] = 16;
    col_global[0] = col_local[0] *
      ((image->columns + col_local[0] - 1) / col_local[0]);
    col_global[1] = col_local[1] *
      ((image->rows + col_local[1] - 1) / col_local[1]);

    status = EnqueueMetalKernel2DEx(device, applyKernel, col_global,
      col_local, 0, col_args, 8);
  }

  if (status == MagickTrue)
  {
    if (GetAuthenticPixels(filteredImage, 0, 0, filteredImage->columns,
        filteredImage->rows, exception) != (Quantum *) NULL)
    {
      SyncPixelCacheFromBuffer(filteredImage, outputBuffer, exception);
      SyncAuthenticPixels(filteredImage, exception);
    }
  }
  else
  {
    filteredImage = DestroyImage(filteredImage);
    filteredImage = (Image *) NULL;
  }

cleanup:
  if (outputBuffer != NULL) RelinquishMetalBuffer(outputBuffer);
  if (tmpBuffer != NULL) RelinquishMetalBuffer(tmpBuffer);
  if (inputBuffer != NULL) RelinquishMetalBuffer(inputBuffer);
  return filteredImage;
}

/*
  ContrastStretch — compute histogram and stretch map on CPU, apply on GPU.
*/
MagickPrivate MagickBooleanType AccelerateContrastStretchImageMetal(
  Image *image, const double black_point, const double white_point,
  ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *imageBuffer,
    *mapBuffer,
    *kernel;

  MagickBooleanType
    status,
    is_nocopy;

  float
    *stretch_map;

  size_t
    map_size;

  status = MagickFalse;
  imageBuffer = NULL;
  mapBuffer = NULL;
  stretch_map = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return MagickFalse;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return MagickFalse;

  /* Build histogram on CPU */
  map_size = (size_t) MaxMap + 1;
  {
    double
      black_val,
      cumsum,
      *histogram,
      intensity,
      total,
      white_val;

    float
      val;

    const Quantum
      *p;

    size_t
      i,
      idx;

    ssize_t
      si,
      x,
      y;

    histogram = (double *) AcquireQuantumMemory(map_size, sizeof(double));
    stretch_map = (float *) AcquireQuantumMemory(map_size, sizeof(float));
    if (histogram == NULL || stretch_map == NULL)
    {
      if (histogram) RelinquishMagickMemory(histogram);
      if (stretch_map) RelinquishMagickMemory(stretch_map);
      return MagickFalse;
    }
    (void) memset(histogram, 0, map_size * sizeof(double));

    for (y = 0; y < (ssize_t) image->rows; y++)
    {
      p = GetVirtualPixels(image, 0, y, image->columns, 1, exception);
      if (p == (const Quantum *) NULL) break;
      for (x = 0; x < (ssize_t) image->columns; x++)
      {
        intensity = GetPixelIntensity(image, p);
        idx = ScaleQuantumToMap(ClampToQuantum(intensity));
        histogram[idx] += 1.0;
        p += GetPixelChannels(image);
      }
    }

    /* Find black and white points */
    total = (double)(image->columns * image->rows);
    black_val = 0.0;
    white_val = (double) MaxMap;
    cumsum = 0.0;
    for (i = 0; i < map_size; i++)
    {
      cumsum += histogram[i];
      if (cumsum > black_point) { black_val = (double) i; break; }
    }
    cumsum = 0.0;
    for (si = (ssize_t)(map_size - 1); si >= 0; si--)
    {
      cumsum += histogram[si];
      if (cumsum > (total - white_point))
        { white_val = (double) si; break; }
    }

    /* Build stretch map */
    for (i = 0; i < map_size; i++)
    {
      if (black_val == white_val)
        val = (float) i / (float)(map_size - 1);
      else if ((double) i < black_val)
        val = 0.0f;
      else if ((double) i > white_val)
        val = 1.0f;
      else
        val = (float)((double)(i - black_val) / (white_val - black_val));
      stretch_map[i] = val * (float) QuantumRange;
    }

    RelinquishMagickMemory(histogram);
  }

  if (GetAuthenticPixels(image, 0, 0, image->columns, image->rows,
      exception) == (Quantum *) NULL)
    goto cleanup;

  imageBuffer = AcquirePixelCacheBuffer(device, image, &is_nocopy, exception);
  if (imageBuffer == NULL)
    goto cleanup;

  mapBuffer = AcquireMetalBuffer(device,
    map_size * sizeof(float), stretch_map);
  if (mapBuffer == NULL)
    goto cleanup;

  kernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalContrastStretchKernelSrc, "ContrastStretchApply");
  if (kernel == NULL)
    goto cleanup;

  {
    uint
      columns,
      nc,
      ms;

    MetalKernelArg
      args[5];

    size_t
      local[2],
      global[2];

    columns = (uint) image->columns;
    nc = (uint) image->number_channels;
    ms = (uint) map_size;

    args[0] = MetalBufferArg(imageBuffer);
    args[1] = MetalBufferArg(mapBuffer);
    args[2] = MetalBytesArg(&columns, sizeof(columns));
    args[3] = MetalBytesArg(&nc, sizeof(nc));
    args[4] = MetalBytesArg(&ms, sizeof(ms));

    local[0] = 16;
    local[1] = 16;
    global[0] = local[0] * ((image->columns + local[0] - 1) / local[0]);
    global[1] = local[1] * ((image->rows + local[1] - 1) / local[1]);

    status = EnqueueMetalKernel2DEx(device, kernel, global, local, 0, args, 5);
  }

  if (status == MagickTrue)
  {
    if (is_nocopy == MagickFalse)
      SyncPixelCacheFromBuffer(image, imageBuffer, exception);
    SyncAuthenticPixels(image, exception);
  }

cleanup:
  if (mapBuffer != NULL) RelinquishMetalBuffer(mapBuffer);
  if (imageBuffer != NULL) RelinquishMetalBuffer(imageBuffer);
  if (stretch_map != NULL) RelinquishMagickMemory(stretch_map);
  return status;
}

/*
  Equalize — compute histogram and equalization map on CPU, apply on GPU.
*/
MagickPrivate MagickBooleanType AccelerateEqualizeImageMetal(Image *image,
  ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *imageBuffer,
    *mapBuffer,
    *kernel;

  MagickBooleanType
    status,
    is_nocopy;

  float
    *equalize_map;

  size_t
    map_size;

  status = MagickFalse;
  imageBuffer = NULL;
  mapBuffer = NULL;
  equalize_map = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return MagickFalse;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return MagickFalse;

  map_size = (size_t) MaxMap + 1;
  {
    double
      black,
      *cumulative,
      *histogram,
      intensity,
      white;

    float
      val;

    const Quantum
      *p;

    size_t
      i,
      idx;

    ssize_t
      x,
      y;

    histogram = (double *) AcquireQuantumMemory(map_size, sizeof(double));
    cumulative = (double *) AcquireQuantumMemory(map_size, sizeof(double));
    equalize_map = (float *) AcquireQuantumMemory(map_size, sizeof(float));
    if (histogram == NULL || cumulative == NULL || equalize_map == NULL)
    {
      if (histogram) RelinquishMagickMemory(histogram);
      if (cumulative) RelinquishMagickMemory(cumulative);
      if (equalize_map) RelinquishMagickMemory(equalize_map);
      return MagickFalse;
    }
    (void) memset(histogram, 0, map_size * sizeof(double));

    for (y = 0; y < (ssize_t) image->rows; y++)
    {
      p = GetVirtualPixels(image, 0, y, image->columns, 1, exception);
      if (p == (const Quantum *) NULL) break;
      for (x = 0; x < (ssize_t) image->columns; x++)
      {
        intensity = GetPixelIntensity(image, p);
        idx = ScaleQuantumToMap(ClampToQuantum(intensity));
        histogram[idx] += 1.0;
        p += GetPixelChannels(image);
      }
    }

    /* Cumulative sum */
    cumulative[0] = histogram[0];
    for (i = 1; i < map_size; i++)
      cumulative[i] = cumulative[i - 1] + histogram[i];

    black = cumulative[0];
    white = cumulative[map_size - 1];

    /* Build equalization map */
    for (i = 0; i < map_size; i++)
    {
      if (white == black)
        val = (float) i / (float)(map_size - 1) * (float) QuantumRange;
      else
        val = (float)((cumulative[i] - black) / (white - black)) *
          (float) QuantumRange;
      equalize_map[i] = val;
    }

    RelinquishMagickMemory(histogram);
    RelinquishMagickMemory(cumulative);
  }

  if (GetAuthenticPixels(image, 0, 0, image->columns, image->rows,
      exception) == (Quantum *) NULL)
    goto cleanup;

  imageBuffer = AcquirePixelCacheBuffer(device, image, &is_nocopy, exception);
  if (imageBuffer == NULL)
    goto cleanup;

  mapBuffer = AcquireMetalBuffer(device,
    map_size * sizeof(float), equalize_map);
  if (mapBuffer == NULL)
    goto cleanup;

  kernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalEqualizeKernelSrc, "EqualizeApply");
  if (kernel == NULL)
    goto cleanup;

  {
    uint
      columns,
      nc,
      ms;

    MetalKernelArg
      args[5];

    size_t
      local[2],
      global[2];

    columns = (uint) image->columns;
    nc = (uint) image->number_channels;
    ms = (uint) map_size;

    args[0] = MetalBufferArg(imageBuffer);
    args[1] = MetalBufferArg(mapBuffer);
    args[2] = MetalBytesArg(&columns, sizeof(columns));
    args[3] = MetalBytesArg(&nc, sizeof(nc));
    args[4] = MetalBytesArg(&ms, sizeof(ms));

    local[0] = 16;
    local[1] = 16;
    global[0] = local[0] * ((image->columns + local[0] - 1) / local[0]);
    global[1] = local[1] * ((image->rows + local[1] - 1) / local[1]);

    status = EnqueueMetalKernel2DEx(device, kernel, global, local, 0, args, 5);
  }

  if (status == MagickTrue)
  {
    if (is_nocopy == MagickFalse)
      SyncPixelCacheFromBuffer(image, imageBuffer, exception);
    SyncAuthenticPixels(image, exception);
  }

cleanup:
  if (mapBuffer != NULL) RelinquishMetalBuffer(mapBuffer);
  if (imageBuffer != NULL) RelinquishMetalBuffer(imageBuffer);
  if (equalize_map != NULL) RelinquishMagickMemory(equalize_map);
  return status;
}

/*
  Resize — separable horizontal/vertical resampling.
*/
MagickPrivate Image *AccelerateResizeImageMetal(const Image *image,
  const size_t resizedColumns, const size_t resizedRows,
  const ResizeFilter *resizeFilter, ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *inputBuffer,
    *tempBuffer,
    *outputBuffer,
    *coeffBuffer,
    *hKernel,
    *vKernel;

  MagickBooleanType
    status,
    is_nocopy;

  Image
    *filteredImage;

  inputBuffer = NULL;
  tempBuffer = NULL;
  outputBuffer = NULL;
  coeffBuffer = NULL;
  filteredImage = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return (Image *) NULL;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return (Image *) NULL;

  inputBuffer = AcquirePixelCacheBuffer(device, (Image *) image, &is_nocopy,
    exception);
  if (inputBuffer == NULL)
    return (Image *) NULL;

  filteredImage = CloneImage(image, resizedColumns, resizedRows, MagickTrue,
    exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;

  {
    /* Temp buffer for intermediate resize */
    size_t
      nc,
      out_length,
      temp_length;

    nc = image->number_channels;
    temp_length = resizedColumns * image->rows * nc;
    out_length = resizedColumns * resizedRows * nc;
    if (temp_length < image->columns * resizedRows * nc)
      temp_length = image->columns * resizedRows * nc;

    tempBuffer = AcquireMetalBuffer(device, temp_length * sizeof(float), NULL);
    outputBuffer = AcquireMetalBuffer(device, out_length * sizeof(float), NULL);
    if (tempBuffer == NULL || outputBuffer == NULL)
      goto cleanup;
  }

  /* Get filter coefficients */
  {
    const double
      *coeffs;

    float
      fcoeffs[7];

    int
      i;

    coeffs = GetResizeFilterCoefficient(resizeFilter);
    for (i = 0; i < 7; i++)
      fcoeffs[i] = (float) coeffs[i];
    coeffBuffer = AcquireMetalBuffer(device, 7 * sizeof(float), fcoeffs);
    if (coeffBuffer == NULL)
      goto cleanup;
  }

  hKernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalResizeKernelSrc, "ResizeHorizontalFilter");
  vKernel = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalResizeKernelSrc, "ResizeVerticalFilter");
  if (hKernel == NULL || vKernel == NULL)
    goto cleanup;

  {
    float
      filterBlur,
      filterScale,
      filterSupport,
      xFactor,
      yFactor;

    int
      filterType,
      windowType;

    uint
      nc;

    xFactor = (float) resizedColumns / (float) image->columns;
    yFactor = (float) resizedRows / (float) image->rows;
    filterType = (int) GetResizeFilterWeightingType(resizeFilter);
    windowType = (int) GetResizeFilterWindowWeightingType(resizeFilter);
    filterScale = (float) GetResizeFilterScale(resizeFilter);
    filterSupport = (float) GetResizeFilterSupport(resizeFilter);
    filterBlur = (float) GetResizeFilterBlur(resizeFilter);
    nc = (uint) image->number_channels;

    /* Horizontal: input(cols x rows) -> temp(resizedCols x rows) */
    {
      uint
        inCols,
        inRows,
        outCols;

      MetalKernelArg
        args[13];

      size_t
        local[2],
        global[2];

      inCols = (uint) image->columns;
      inRows = (uint) image->rows;
      outCols = (uint) resizedColumns;

      args[0] = MetalBufferArg(inputBuffer);
      args[1] = MetalBufferArg(tempBuffer);
      args[2] = MetalBytesArg(&inCols, sizeof(inCols));
      args[3] = MetalBytesArg(&inRows, sizeof(inRows));
      args[4] = MetalBytesArg(&outCols, sizeof(outCols));
      args[5] = MetalBytesArg(&nc, sizeof(nc));
      args[6] = MetalBytesArg(&xFactor, sizeof(xFactor));
      args[7] = MetalBytesArg(&filterType, sizeof(filterType));
      args[8] = MetalBytesArg(&windowType, sizeof(windowType));
      args[9] = MetalBufferArg(coeffBuffer);
      args[10] = MetalBytesArg(&filterScale, sizeof(filterScale));
      args[11] = MetalBytesArg(&filterSupport, sizeof(filterSupport));
      args[12] = MetalBytesArg(&filterBlur, sizeof(filterBlur));

      local[0] = 16;
      local[1] = 16;
      global[0] = local[0] *
        ((resizedColumns + local[0] - 1) / local[0]);
      global[1] = local[1] *
        ((image->rows + local[1] - 1) / local[1]);

      status = EnqueueMetalKernel2DEx(device, hKernel, global, local, 0,
        args, 13);
      if (status != MagickTrue) goto cleanup;
    }

    /* Vertical: temp(resizedCols x rows) -> output(resizedCols x resizedRows) */
    {
      uint
        inCols,
        inRows,
        outRows;

      MetalKernelArg
        args[13];

      size_t
        local[2],
        global[2];

      inCols = (uint) resizedColumns;
      inRows = (uint) image->rows;
      outRows = (uint) resizedRows;

      args[0] = MetalBufferArg(tempBuffer);
      args[1] = MetalBufferArg(outputBuffer);
      args[2] = MetalBytesArg(&inCols, sizeof(inCols));
      args[3] = MetalBytesArg(&inRows, sizeof(inRows));
      args[4] = MetalBytesArg(&outRows, sizeof(outRows));
      args[5] = MetalBytesArg(&nc, sizeof(nc));
      args[6] = MetalBytesArg(&yFactor, sizeof(yFactor));
      args[7] = MetalBytesArg(&filterType, sizeof(filterType));
      args[8] = MetalBytesArg(&windowType, sizeof(windowType));
      args[9] = MetalBufferArg(coeffBuffer);
      args[10] = MetalBytesArg(&filterScale, sizeof(filterScale));
      args[11] = MetalBytesArg(&filterSupport, sizeof(filterSupport));
      args[12] = MetalBytesArg(&filterBlur, sizeof(filterBlur));

      local[0] = 16;
      local[1] = 16;
      global[0] = local[0] *
        ((resizedColumns + local[0] - 1) / local[0]);
      global[1] = local[1] *
        ((resizedRows + local[1] - 1) / local[1]);

      status = EnqueueMetalKernel2DEx(device, vKernel, global, local, 0,
        args, 13);
    }
  }

  if (status == MagickTrue)
  {
    if (GetAuthenticPixels(filteredImage, 0, 0, filteredImage->columns,
        filteredImage->rows, exception) != (Quantum *) NULL)
    {
      SyncPixelCacheFromBuffer(filteredImage, outputBuffer, exception);
      SyncAuthenticPixels(filteredImage, exception);
    }
  }
  else
  {
    filteredImage = DestroyImage(filteredImage);
    filteredImage = (Image *) NULL;
  }

cleanup:
  if (coeffBuffer != NULL) RelinquishMetalBuffer(coeffBuffer);
  if (outputBuffer != NULL) RelinquishMetalBuffer(outputBuffer);
  if (tempBuffer != NULL) RelinquishMetalBuffer(tempBuffer);
  if (inputBuffer != NULL) RelinquishMetalBuffer(inputBuffer);
  return filteredImage;
}

/*
  WaveletDenoise — tile-based multi-pass wavelet denoising.
*/
MagickPrivate Image *AccelerateWaveletDenoiseImageMetal(const Image *image,
  const double threshold, ExceptionInfo *exception)
{
  MagickMetalEnv
    env;

  MagickMetalDevice
    device;

  void
    *inputBuffer,
    *outputBuffer,
    *pipeline;

  MagickBooleanType
    status,
    is_nocopy;

  Image
    *filteredImage;

  inputBuffer = NULL;
  outputBuffer = NULL;
  filteredImage = NULL;
  is_nocopy = MagickFalse;

  env = GetMagickMetalEnv();
  if (env == (MagickMetalEnv) NULL || !env->enabled)
    return (Image *) NULL;
  device = AcquireMagickMetalDevice(env);
  if (device == (MagickMetalDevice) NULL)
    return (Image *) NULL;

  inputBuffer = AcquirePixelCacheBuffer(device, (Image *) image, &is_nocopy,
    exception);
  if (inputBuffer == NULL)
    return (Image *) NULL;

  filteredImage = CloneImage(image, 0, 0, MagickTrue, exception);
  if (filteredImage == (Image *) NULL)
    goto cleanup;

  {
    MagickSizeType
      cache_length;

    GetPixelCachePixels(filteredImage, &cache_length, exception);
    if (GetAuthenticPixels(filteredImage, 0, 0, filteredImage->columns,
        filteredImage->rows, exception) == (Quantum *) NULL)
      goto cleanup;
    outputBuffer = AcquirePixelCacheBuffer(device, filteredImage, &is_nocopy,
      exception);
    if (outputBuffer == NULL)
      goto cleanup;
  }

  pipeline = AcquireMetalKernelWithHelpers(device, metalHelperFunctions,
    metalWaveletDenoiseKernelSrc, "WaveletDenoise");
  if (pipeline == NULL)
    goto cleanup;

  {
    const int
      PASSES = 5,
      TILESIZE = 64,
      SIZE = 64 - 2 * (1 << (5 - 1));

    uint
      nc,
      max_channels,
      width,
      height;

    float
      thresh;

    int
      passes;

    size_t
      lsize[2],
      num_tiles_x,
      num_tiles_y,
      gsize[2];

    MetalKernelArg
      args[8];

    nc = (uint) image->number_channels;
    max_channels = (nc == 4) ? 3 : ((nc == 2) ? 1 : nc);
    width = (uint) image->columns;
    height = (uint) image->rows;
    thresh = (float) threshold;
    passes = PASSES;

    lsize[0] = (size_t) TILESIZE;
    lsize[1] = 4;
    num_tiles_x = (image->columns + SIZE - 1) / SIZE;
    num_tiles_y = (image->rows + SIZE - 1) / SIZE;

    args[0] = MetalBufferArg(inputBuffer);
    args[1] = MetalBufferArg(outputBuffer);
    args[2] = MetalBytesArg(&nc, sizeof(nc));
    args[3] = MetalBytesArg(&max_channels, sizeof(max_channels));
    args[4] = MetalBytesArg(&thresh, sizeof(thresh));
    args[5] = MetalBytesArg(&passes, sizeof(passes));
    args[6] = MetalBytesArg(&width, sizeof(width));
    args[7] = MetalBytesArg(&height, sizeof(height));

    gsize[0] = num_tiles_x * TILESIZE;
    gsize[1] = num_tiles_y * 4;

    status = EnqueueMetalKernel2DEx(device, pipeline, gsize, lsize, 0,
      args, 8);
  }

  if (status == MagickTrue)
  {
    if (is_nocopy == MagickFalse)
      SyncPixelCacheFromBuffer(filteredImage, outputBuffer, exception);
    SyncAuthenticPixels(filteredImage, exception);
  }
  else
  {
    filteredImage = DestroyImage(filteredImage);
    filteredImage = (Image *) NULL;
  }

cleanup:
  if (outputBuffer != NULL) RelinquishMetalBuffer(outputBuffer);
  if (inputBuffer != NULL) RelinquishMetalBuffer(inputBuffer);
  return filteredImage;
}

#endif

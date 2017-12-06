/*
  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore OpenCL public methods.
*/
#ifndef MAGICKCORE_OPENCL_H
#define MAGICKCORE_OPENCL_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedCLDeviceType,
  CpuCLDeviceType,
  GpuCLDeviceType
} MagickCLDeviceType;

typedef struct _KernelProfileRecord
{
  char
    *kernel_name;

  unsigned long
    count,
    max,
    min,
    total;
}* KernelProfileRecord;

typedef struct _MagickCLDevice* MagickCLDevice;

extern MagickExport const char
  *GetOpenCLDeviceName(const MagickCLDevice),
  *GetOpenCLDeviceVendorName(const MagickCLDevice),
  *GetOpenCLDeviceVersion(const MagickCLDevice);

extern MagickExport const KernelProfileRecord
  *GetOpenCLKernelProfileRecords(const MagickCLDevice,size_t *);

extern MagickExport double
  GetOpenCLDeviceBenchmarkScore(const MagickCLDevice);

extern MagickExport MagickCLDevice
  *GetOpenCLDevices(size_t *,ExceptionInfo *);

extern MagickExport MagickCLDeviceType
  GetOpenCLDeviceType(const MagickCLDevice);

extern MagickExport MagickBooleanType
  GetOpenCLDeviceEnabled(const MagickCLDevice),
  GetOpenCLEnabled(void),
  SetOpenCLEnabled(const MagickBooleanType);

extern MagickExport void
  SetOpenCLDeviceEnabled(MagickCLDevice,
    const MagickBooleanType),
  SetOpenCLKernelProfileEnabled(MagickCLDevice,
    const MagickBooleanType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

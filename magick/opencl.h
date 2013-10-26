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


#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

// OpenCL program modules
typedef enum {
  MAGICK_OPENCL_ACCELERATE = 0
  ,MAGICK_OPENCL_NUM_PROGRAMS   /* !!! This has to be the last entry !!! */
} MagickOpenCLProgram;


typedef struct _MagickCLEnv* MagickCLEnv;

extern MagickExport
  MagickCLEnv AcquireMagickOpenCLEnv();

extern MagickExport
  MagickBooleanType RelinquishMagickOpenCLEnv(MagickCLEnv);

extern MagickExport
  MagickCLEnv GetDefaultOpenCLEnv();

extern MagickExport
  MagickCLEnv SetDefaultOpenCLEnv(MagickCLEnv);


/* Parameter type accepted by SetMagickOpenCLEnvParm and GetMagickOpenCLEnvParm */
typedef enum {
    MAGICK_OPENCL_ENV_PARAM_DEVICE                  /* cl_device_id (from OpenCL) */
  , MAGICK_OPENCL_ENV_PARAM_OPENCL_DISABLED         /* MagickBooleanType */
  , MAGICK_OPENCL_ENV_PARAM_OPENCL_INITIALIZED      /* MagickBooleanType */
} MagickOpenCLEnvParam;

extern MagickExport
  MagickBooleanType SetMagickOpenCLEnvParam(MagickCLEnv, MagickOpenCLEnvParam, size_t, void*, ExceptionInfo*);

extern MagickExport
  MagickBooleanType GetMagickOpenCLEnvParam(MagickCLEnv, MagickOpenCLEnvParam, size_t, void*, ExceptionInfo*);


extern MagickExport
  MagickBooleanType InitOpenCLEnv(MagickCLEnv, ExceptionInfo*);


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

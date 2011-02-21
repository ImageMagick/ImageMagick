/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore version methods.
*/
#ifndef _MAGICKCORE_VERSION_H
#define _MAGICKCORE_VERSION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  Define declarations.
*/
#define MagickPackageName "ImageMagick"
#define MagickCopyright  "Copyright (C) 1999-2011 ImageMagick Studio LLC"
#define MagickLibVersion  0x667
#define MagickLibVersionText  "6.6.7"
#define MagickLibVersionNumber  4,0,1
#define MagickLibAddendum  "-9"
#define MagickLibInterface  4
#define MagickLibMinInterface  4
#define MagickReleaseDate  "2011-02-21"
#define MagickChangeDate   "20110215"
#define MagickAuthoritativeURL  "http://www.imagemagick.org"
#if defined(MAGICKCORE_OPENMP_SUPPORT)
#define MagickOpenMPFeature  "OpenMP "
#else
#define MagickOpenMPFeature  " "
#endif
#if defined(MAGICKCORE_OPENCL_SUPPORT)
#define MagickOpenCLFeature  "OpenCL "
#else
#define MagickOpenCLFeature  " "
#endif
#if defined(MAGICKCORE_HDRI_SUPPORT)
#define MagickHDRIFeature  "HDRI "
#else
#define MagickHDRIFeature  " "
#endif
#define MagickFeatures MagickOpenMPFeature MagickOpenCLFeature MagickHDRIFeature
#define MagickHomeURL  "file:///usr/local/share/doc/ImageMagick-6.6.7/index.html"
#if (MAGICKCORE_QUANTUM_DEPTH == 8)
#define MagickQuantumDepth  "Q8"
#define MagickQuantumRange  "255"
#elif (MAGICKCORE_QUANTUM_DEPTH == 16)
#define MagickQuantumDepth  "Q16"
#define MagickQuantumRange  "65535"
#elif (MAGICKCORE_QUANTUM_DEPTH == 32)
#define MagickQuantumDepth  "Q32"
#define MagickQuantumRange  "4294967295"
#elif (MAGICKCORE_QUANTUM_DEPTH == 64)
#define MagickQuantumDepth  "Q64"
#define MagickQuantumRange  "18446744073709551615"
#else
#define MagickQuantumDepth  "Q?"
#define MagickQuantumRange  "?"
#endif
#define MagickVersion  \
  MagickPackageName " " MagickLibVersionText MagickLibAddendum " " \
  MagickReleaseDate " " MagickQuantumDepth " " MagickAuthoritativeURL

extern MagickExport char
  *GetMagickHomeURL(void);

extern MagickExport const char
  *GetMagickCopyright(void),
  *GetMagickFeatures(void),
  *GetMagickPackageName(void),
  *GetMagickQuantumDepth(size_t *),
  *GetMagickQuantumRange(size_t *),
  *GetMagickReleaseDate(void),
  *GetMagickVersion(size_t *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

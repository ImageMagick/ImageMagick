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

  MagickCore delegates private methods.
*/
#ifndef _MAGICKCORE_DELEGATE_PRIVATE_H
#define _MAGICKCORE_DELEGATE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(MAGICKCORE_GS_DELEGATE)
#include "ghostscript/iapi.h"
#include "ghostscript/ierrors.h"
#endif

#ifndef gs_main_instance_DEFINED
# define gs_main_instance_DEFINED
typedef struct gs_main_instance_s
  gs_main_instance;
#endif

#if !defined(MagickDLLCall)
#  if defined(MAGICKCORE_WINDOWS_SUPPORT)
#    define MagickDLLCall __stdcall
#  else
#    define MagickDLLCall
#  endif
#endif

typedef struct _GhostInfo
{
  int
    (MagickDLLCall *exit)(gs_main_instance *);

  int
    (MagickDLLCall *init_with_args)(gs_main_instance *,int,char **);

  int
    (MagickDLLCall *new_instance)(gs_main_instance **,void *);

  int
    (MagickDLLCall *run_string)(gs_main_instance *,const char *,int,int *);

  void
    (MagickDLLCall *delete_instance)(gs_main_instance *);
} GhostInfo;

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

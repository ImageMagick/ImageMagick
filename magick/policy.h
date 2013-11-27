/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image color methods.
*/
#ifndef _MAGICKCORE_POLICY_H
#define _MAGICKCORE_POLICY_H

#include "magick/pixel.h"
#include "magick/exception.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedPolicyDomain,
  CoderPolicyDomain,
  DelegatePolicyDomain,
  FilterPolicyDomain,
  PathPolicyDomain,
  ResourcePolicyDomain,
  SystemPolicyDomain
} PolicyDomain;

typedef enum
{
  UndefinedPolicyRights = 0x00,
  NoPolicyRights = 0x00,
  ReadPolicyRights = 0x01,
  WritePolicyRights = 0x02,
  ExecutePolicyRights = 0x04
} PolicyRights;

typedef struct _PolicyInfo
  PolicyInfo;

extern MagickExport char
  *GetPolicyValue(const char *name),
  **GetPolicyList(const char *,size_t *,ExceptionInfo *);

extern MagickExport const PolicyInfo
  **GetPolicyInfoList(const char *,size_t *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  IsRightsAuthorized(const PolicyDomain,const PolicyRights,const char *),
  ListPolicyInfo(FILE *,ExceptionInfo *),
  PolicyComponentGenesis(void);

extern MagickExport void
  PolicyComponentTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

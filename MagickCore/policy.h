/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore security policy methods.
*/
#ifndef MAGICKCORE_POLICY_H
#define MAGICKCORE_POLICY_H

#include "MagickCore/pixel.h"
#include "MagickCore/exception.h"

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
  SystemPolicyDomain,
  CachePolicyDomain,
  ModulePolicyDomain
} PolicyDomain;

typedef enum
{
  UndefinedPolicyRights = 0x00,
  NoPolicyRights = 0x00,
  ReadPolicyRights = 0x01,
  WritePolicyRights = 0x02,
  ExecutePolicyRights = 0x04,
  AllPolicyRights = 0xff
} PolicyRights;

typedef struct _PolicyInfo
  PolicyInfo;

extern MagickExport char
  *GetPolicyValue(const char *),
  **GetPolicyList(const char *,size_t *,ExceptionInfo *);

extern MagickExport const PolicyInfo
  **GetPolicyInfoList(const char *,size_t *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  IsRightsAuthorized(const PolicyDomain,const PolicyRights,const char *),
  ListPolicyInfo(FILE *,ExceptionInfo *),
  SetMagickSecurityPolicy(const char *,ExceptionInfo *),
  SetMagickSecurityPolicyValue(const PolicyDomain,const char *,const char *,
    ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

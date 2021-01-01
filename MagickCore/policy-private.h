/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private policy methods.
*/
#ifndef MAGICKCORE_POLICY_PRIVATE_H
#define MAGICKCORE_POLICY_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if MAGICKCORE_ZERO_CONFIGURATION_SUPPORT
/*
  Zero configuration security policy.  Discussion @
  https://imagemagick.org/script/security-policy.php.
*/
static const char
  *ZeroConfigurationPolicy = \
"<policymap> \
</policymap>";
#endif

extern MagickPrivate MagickBooleanType
  PolicyComponentGenesis(void);

extern MagickPrivate void
  PolicyComponentTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

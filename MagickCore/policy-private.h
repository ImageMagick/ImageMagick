/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/license/

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

#include "MagickCore/utility-private.h"

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

static inline MagickBooleanType IsPathContainsSymlink(const char *path)
{
  char
    partial[MagickPathExtent];

  const char
    *p;

  ssize_t
    offset = 0;

  if (path == (const char *) NULL)
    return(MagickFalse);
  *partial='\0';
  p=path;
  if (*p == *DirectorySeparator)
    {
      /*
        Path starts with a directory separator, include it.
      */
      if ((offset+1) >= (ssize_t) sizeof(partial))
        return(MagickFalse);
      partial[offset++]=(*p++);
      partial[offset]='\0';
    }
  while (*p != '\0')
  {
    char
      component[MagickPathExtent];

    ssize_t
      i = 0;

    /*
      Copy next component into a temporary buffer.
    */
    while ((*p != '\0') && (*p != *DirectorySeparator) &&
           ((i+1) < (ssize_t) sizeof(component)))
      component[i++]=(*p++);
    component[i]='\0';
    if (i == 0)
      {
        /*
          skip repeated separators.
        */
        if (*p == *DirectorySeparator)
          p++;
        continue;
      }
    if ((offset > 0) && (partial[offset-1] != *DirectorySeparator))
      {
        /*
          Append separator if needed.
        */
        if ((offset+1) >= (ssize_t) sizeof(partial))
          return MagickFalse;
        partial[offset++]=(*DirectorySeparator);
        partial[offset]='\0';
      }
    /*
      Append component.
    */
    if ((offset+i) >= (ssize_t) sizeof(partial))
      return(MagickFalse);
    (void) memcpy(partial+offset,component,i);
    offset+=i;
    partial[offset]='\0';
    if (*p != '\0')
      {
        /*
          Check whether this prefix is a symlink.
        */
        if (is_symlink_utf8(partial) != MagickFalse)
          return(MagickTrue);
      }
    /*
      Skip separator.
    */
    if (*p == *DirectorySeparator)
      p++;
  }
  return(MagickFalse);
}

static inline MagickBooleanType IsPathAuthorized(const PolicyRights rights,
  const char *filename)
{
  MagickBooleanType symlink_follow_allowed = IsRightsAuthorizedByName(
    SystemPolicyDomain,"symlink",rights,"follow");
  MagickBooleanType status =
   ((IsRightsAuthorized(PathPolicyDomain,rights,filename) != MagickFalse) &&
   ((symlink_follow_allowed != MagickFalse) ||
    (is_symlink_utf8(filename) == MagickFalse))) ? MagickTrue : MagickFalse;
  if ((status != MagickFalse) && (symlink_follow_allowed == MagickFalse))
    {
      if ((is_symlink_utf8(filename) != MagickFalse) ||
          (IsPathContainsSymlink(filename) != MagickFalse))
        status=MagickFalse;
    }
  return(status);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

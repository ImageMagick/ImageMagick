/*
  Copyright @ 2022 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private image f/x methods.
*/
#ifndef MAGICKCORE_FX_PRIVATE_H
#define MAGICKCORE_FX_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _FxInfo
  FxInfo;

extern MagickPrivate FxInfo
  *AcquireFxInfo(const Image *,const char *,ExceptionInfo *),
  *DestroyFxInfo(FxInfo *);

extern MagickPrivate MagickBooleanType
  FxEvaluateChannelExpression(FxInfo *,const PixelChannel,const ssize_t,
   const ssize_t,double *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

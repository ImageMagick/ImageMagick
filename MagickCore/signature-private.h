/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore digital signature methods.
*/
#ifndef MAGICKCORE_SIGNATURE_PRIVATE_H
#define MAGICKCORE_SIGNATURE_PRIVATE_H

#include "MagickCore/string_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MagickSignatureSize  64

typedef struct _SignatureInfo
  SignatureInfo;

extern MagickPrivate SignatureInfo
  *AcquireSignatureInfo(void),
  *DestroySignatureInfo(SignatureInfo *);

extern MagickPrivate const StringInfo
  *GetSignatureDigest(const SignatureInfo *);

extern MagickPrivate unsigned int
  GetSignatureBlocksize(const SignatureInfo *),
  GetSignatureDigestsize(const SignatureInfo *);

extern MagickPrivate void
  InitializeSignature(SignatureInfo *),
  FinalizeSignature(SignatureInfo *),
  SetSignatureDigest(SignatureInfo *,const StringInfo *),
  UpdateSignature(SignatureInfo *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

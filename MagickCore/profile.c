/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               PPPP   RRRR    OOO   FFFFF  IIIII  L      EEEEE               %
%               P   P  R   R  O   O  F        I    L      E                   %
%               PPPP   RRRR   O   O  FFF      I    L      EEE                 %
%               P      R R    O   O  F        I    L      E                   %
%               P      R  R    OOO   F      IIIII  LLLLL  EEEEE               %
%                                                                             %
%                                                                             %
%                       MagickCore Image Profile Methods                      %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/configure.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/linked-list.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/option-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/profile.h"
#include "MagickCore/profile-private.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#if defined(MAGICKCORE_LCMS_DELEGATE)
#if defined(MAGICKCORE_HAVE_LCMS_LCMS2_H)
#include <wchar.h>
#include <lcms/lcms2.h>
#else
#include <wchar.h>
#include "lcms2.h"
#endif
#endif
#if defined(MAGICKCORE_XML_DELEGATE)
#  if defined(MAGICKCORE_WINDOWS_SUPPORT)
#    if !defined(__MINGW32__)
#      include <win32config.h>
#    endif
#  endif
#  include <libxml/parser.h>
#  include <libxml/tree.h>
#endif

/*
  Definitions
*/
#define LCMSHDRI
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  #if (MAGICKCORE_QUANTUM_DEPTH == 8)
  #undef LCMSHDRI
  #define LCMSScaleSource(pixel)  ScaleQuantumToShort(pixel)
  #define LCMSScaleTarget(pixel)  ScaleShortToQuantum(pixel)
  typedef unsigned short
    LCMSType;
  #elif (MAGICKCORE_QUANTUM_DEPTH == 16)
  #undef LCMSHDRI
  #define LCMSScaleSource(pixel)  (pixel)
  #define LCMSScaleTarget(pixel)  (pixel)
  typedef unsigned short
    LCMSType;
  #endif
#endif

#if defined(LCMSHDRI)
#define LCMSScaleSource(pixel)  (source_scale*QuantumScale*(pixel))
#define LCMSScaleTarget(pixel) ClampToQuantum(target_scale*QuantumRange*(pixel))
typedef double
  LCMSType;
#endif

/*
  Forward declarations
*/
static MagickBooleanType
  SetImageProfileInternal(Image *,const char *,const StringInfo *,
    const MagickBooleanType,ExceptionInfo *);

static void
  WriteTo8BimProfile(Image *,const char*,const StringInfo *);

/*
  Typedef declarations
*/
struct _ProfileInfo
{
  char
    *name;

  size_t
    length;

  unsigned char
    *info;

  size_t
    signature;
};

typedef struct _CMSExceptionInfo
{
  Image
    *image;

  ExceptionInfo
    *exception;
} CMSExceptionInfo;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e P r o f i l e s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImageProfiles() clones one or more image profiles.
%
%  The format of the CloneImageProfiles method is:
%
%      MagickBooleanType CloneImageProfiles(Image *image,
%        const Image *clone_image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o clone_image: the clone image.
%
*/
MagickExport MagickBooleanType CloneImageProfiles(Image *image,
  const Image *clone_image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(clone_image != (const Image *) NULL);
  assert(clone_image->signature == MagickCoreSignature);
  if (clone_image->profiles != (void *) NULL)
    {
      if (image->profiles != (void *) NULL)
        DestroyImageProfiles(image);
      image->profiles=CloneSplayTree((SplayTreeInfo *) clone_image->profiles,
        (void *(*)(void *)) ConstantString,(void *(*)(void *)) CloneStringInfo);
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e l e t e I m a g e P r o f i l e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeleteImageProfile() deletes a profile from the image by its name.
%
%  The format of the DeleteImageProfile method is:
%
%      MagickBooleanTyupe DeleteImageProfile(Image *image,const char *name)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o name: the profile name.
%
*/
MagickExport MagickBooleanType DeleteImageProfile(Image *image,const char *name)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->profiles == (SplayTreeInfo *) NULL)
    return(MagickFalse);
  WriteTo8BimProfile(image,name,(StringInfo *) NULL);
  return(DeleteNodeFromSplayTree((SplayTreeInfo *) image->profiles,name));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e P r o f i l e s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImageProfiles() releases memory associated with an image profile map.
%
%  The format of the DestroyProfiles method is:
%
%      void DestroyImageProfiles(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void DestroyImageProfiles(Image *image)
{
  if (image->profiles != (SplayTreeInfo *) NULL)
    image->profiles=DestroySplayTree((SplayTreeInfo *) image->profiles);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e P r o f i l e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageProfile() gets a profile associated with an image by name.
%
%  The format of the GetImageProfile method is:
%
%      const StringInfo *GetImageProfile(const Image *image,const char *name)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o name: the profile name.
%
*/
MagickExport const StringInfo *GetImageProfile(const Image *image,
  const char *name)
{
  const StringInfo
    *profile;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->profiles == (SplayTreeInfo *) NULL)
    return((StringInfo *) NULL);
  profile=(const StringInfo *) GetValueFromSplayTree((SplayTreeInfo *)
    image->profiles,name);
  return(profile);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t I m a g e P r o f i l e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextImageProfile() gets the next profile name for an image.
%
%  The format of the GetNextImageProfile method is:
%
%      char *GetNextImageProfile(const Image *image)
%
%  A description of each parameter follows:
%
%    o hash_info: the hash info.
%
*/
MagickExport char *GetNextImageProfile(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->profiles == (SplayTreeInfo *) NULL)
    return((char *) NULL);
  return((char *) GetNextKeyInSplayTree((SplayTreeInfo *) image->profiles));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P r o f i l e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ProfileImage() associates, applies, or removes an ICM, IPTC, or generic
%  profile with / to / from an image.  If the profile is NULL, it is removed
%  from the image otherwise added or applied.  Use a name of '*' and a profile
%  of NULL to remove all profiles from the image.
%
%  ICC and ICM profiles are handled as follows: If the image does not have
%  an associated color profile, the one you provide is associated with the
%  image and the image pixels are not transformed.  Otherwise, the colorspace
%  transform defined by the existing and new profile are applied to the image
%  pixels and the new profile is associated with the image.
%
%  The format of the ProfileImage method is:
%
%      MagickBooleanType ProfileImage(Image *image,const char *name,
%        const void *datum,const size_t length,const MagickBooleanType clone)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o name: Name of profile to add or remove: ICC, IPTC, or generic profile.
%
%    o datum: the profile data.
%
%    o length: the length of the profile.
%
%    o clone: should be MagickFalse.
%
*/

#if defined(MAGICKCORE_LCMS_DELEGATE)
static LCMSType **DestroyPixelThreadSet(LCMSType **pixels)
{
  register ssize_t
    i;

  if (pixels != (LCMSType **) NULL)
    return((LCMSType **) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (pixels[i] != (LCMSType *) NULL)
      pixels[i]=(LCMSType *) RelinquishMagickMemory(pixels[i]);
  pixels=(LCMSType **) RelinquishMagickMemory(pixels);
  return(pixels);
}

static LCMSType **AcquirePixelThreadSet(const size_t columns,
  const size_t channels)
{
  LCMSType
    **pixels;

  register ssize_t
    i;

  size_t
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  pixels=(LCMSType **) AcquireQuantumMemory(number_threads,sizeof(*pixels));
  if (pixels == (LCMSType **) NULL)
    return((LCMSType **) NULL);
  (void) memset(pixels,0,number_threads*sizeof(*pixels));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    pixels[i]=(LCMSType *) AcquireQuantumMemory(columns,channels*
      sizeof(**pixels));
    if (pixels[i] == (LCMSType *) NULL)
      return(DestroyPixelThreadSet(pixels));
  }
  return(pixels);
}

static cmsHTRANSFORM *DestroyTransformThreadSet(cmsHTRANSFORM *transform)
{
  register ssize_t
    i;

  assert(transform != (cmsHTRANSFORM *) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (transform[i] != (cmsHTRANSFORM) NULL)
      cmsDeleteTransform(transform[i]);
  transform=(cmsHTRANSFORM *) RelinquishMagickMemory(transform);
  return(transform);
}

static cmsHTRANSFORM *AcquireTransformThreadSet(Image *image,
  const cmsHPROFILE source_profile,const cmsUInt32Number source_type,
  const cmsHPROFILE target_profile,const cmsUInt32Number target_type,
  const int intent,const cmsUInt32Number flags,
  CMSExceptionInfo *cms_exception)
{
  cmsHTRANSFORM
    *transform;

  register ssize_t
    i;

  size_t
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  transform=(cmsHTRANSFORM *) AcquireQuantumMemory(number_threads,
    sizeof(*transform));
  if (transform == (cmsHTRANSFORM *) NULL)
    return((cmsHTRANSFORM *) NULL);
  (void) memset(transform,0,number_threads*sizeof(*transform));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    transform[i]=cmsCreateTransformTHR((cmsContext) cms_exception,
      source_profile,source_type,target_profile,target_type,intent,flags);
    if (transform[i] == (cmsHTRANSFORM) NULL)
      return(DestroyTransformThreadSet(transform));
  }
  return(transform);
}
#endif

#if defined(MAGICKCORE_LCMS_DELEGATE)
static void CMSExceptionHandler(cmsContext context,cmsUInt32Number severity,
  const char *message)
{
  CMSExceptionInfo
    *cms_exception;

  ExceptionInfo
    *exception;

  Image
    *image;

  cms_exception=(CMSExceptionInfo *) context;
  if (cms_exception == (CMSExceptionInfo *) NULL)
    return;
  exception=cms_exception->exception;
  if (exception == (ExceptionInfo *) NULL)
    return;
  image=cms_exception->image;
  if (image == (Image *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageWarning,
        "UnableToTransformColorspace","`%s'","unknown context");
      return;
    }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TransformEvent,GetMagickModule(),"lcms: #%u, %s",
      severity,message != (char *) NULL ? message : "no message");
  (void) ThrowMagickException(exception,GetMagickModule(),ImageWarning,
    "UnableToTransformColorspace","`%s'",image->filename);
}
#endif

static MagickBooleanType SetsRGBImageProfile(Image *image,
  ExceptionInfo *exception)
{
  static unsigned char
    sRGBProfile[] =
    {
      0x00, 0x00, 0x0c, 0x8c, 0x61, 0x72, 0x67, 0x6c, 0x02, 0x20, 0x00, 0x00,
      0x6d, 0x6e, 0x74, 0x72, 0x52, 0x47, 0x42, 0x20, 0x58, 0x59, 0x5a, 0x20,
      0x07, 0xde, 0x00, 0x01, 0x00, 0x06, 0x00, 0x16, 0x00, 0x0f, 0x00, 0x3a,
      0x61, 0x63, 0x73, 0x70, 0x4d, 0x53, 0x46, 0x54, 0x00, 0x00, 0x00, 0x00,
      0x49, 0x45, 0x43, 0x20, 0x73, 0x52, 0x47, 0x42, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf6, 0xd6,
      0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xd3, 0x2d, 0x61, 0x72, 0x67, 0x6c,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11,
      0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x01, 0x50, 0x00, 0x00, 0x00, 0x99,
      0x63, 0x70, 0x72, 0x74, 0x00, 0x00, 0x01, 0xec, 0x00, 0x00, 0x00, 0x67,
      0x64, 0x6d, 0x6e, 0x64, 0x00, 0x00, 0x02, 0x54, 0x00, 0x00, 0x00, 0x70,
      0x64, 0x6d, 0x64, 0x64, 0x00, 0x00, 0x02, 0xc4, 0x00, 0x00, 0x00, 0x88,
      0x74, 0x65, 0x63, 0x68, 0x00, 0x00, 0x03, 0x4c, 0x00, 0x00, 0x00, 0x0c,
      0x76, 0x75, 0x65, 0x64, 0x00, 0x00, 0x03, 0x58, 0x00, 0x00, 0x00, 0x67,
      0x76, 0x69, 0x65, 0x77, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x24,
      0x6c, 0x75, 0x6d, 0x69, 0x00, 0x00, 0x03, 0xe4, 0x00, 0x00, 0x00, 0x14,
      0x6d, 0x65, 0x61, 0x73, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x24,
      0x77, 0x74, 0x70, 0x74, 0x00, 0x00, 0x04, 0x1c, 0x00, 0x00, 0x00, 0x14,
      0x62, 0x6b, 0x70, 0x74, 0x00, 0x00, 0x04, 0x30, 0x00, 0x00, 0x00, 0x14,
      0x72, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x04, 0x44, 0x00, 0x00, 0x00, 0x14,
      0x67, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x04, 0x58, 0x00, 0x00, 0x00, 0x14,
      0x62, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x04, 0x6c, 0x00, 0x00, 0x00, 0x14,
      0x72, 0x54, 0x52, 0x43, 0x00, 0x00, 0x04, 0x80, 0x00, 0x00, 0x08, 0x0c,
      0x67, 0x54, 0x52, 0x43, 0x00, 0x00, 0x04, 0x80, 0x00, 0x00, 0x08, 0x0c,
      0x62, 0x54, 0x52, 0x43, 0x00, 0x00, 0x04, 0x80, 0x00, 0x00, 0x08, 0x0c,
      0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f,
      0x73, 0x52, 0x47, 0x42, 0x20, 0x49, 0x45, 0x43, 0x36, 0x31, 0x39, 0x36,
      0x36, 0x2d, 0x32, 0x2e, 0x31, 0x20, 0x28, 0x45, 0x71, 0x75, 0x69, 0x76,
      0x61, 0x6c, 0x65, 0x6e, 0x74, 0x20, 0x74, 0x6f, 0x20, 0x77, 0x77, 0x77,
      0x2e, 0x73, 0x72, 0x67, 0x62, 0x2e, 0x63, 0x6f, 0x6d, 0x20, 0x31, 0x39,
      0x39, 0x38, 0x20, 0x48, 0x50, 0x20, 0x70, 0x72, 0x6f, 0x66, 0x69, 0x6c,
      0x65, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x3f, 0x73, 0x52, 0x47, 0x42, 0x20, 0x49, 0x45, 0x43, 0x36, 0x31,
      0x39, 0x36, 0x36, 0x2d, 0x32, 0x2e, 0x31, 0x20, 0x28, 0x45, 0x71, 0x75,
      0x69, 0x76, 0x61, 0x6c, 0x65, 0x6e, 0x74, 0x20, 0x74, 0x6f, 0x20, 0x77,
      0x77, 0x77, 0x2e, 0x73, 0x72, 0x67, 0x62, 0x2e, 0x63, 0x6f, 0x6d, 0x20,
      0x31, 0x39, 0x39, 0x38, 0x20, 0x48, 0x50, 0x20, 0x70, 0x72, 0x6f, 0x66,
      0x69, 0x6c, 0x65, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x74, 0x65, 0x78, 0x74, 0x00, 0x00, 0x00, 0x00, 0x43, 0x72, 0x65, 0x61,
      0x74, 0x65, 0x64, 0x20, 0x62, 0x79, 0x20, 0x47, 0x72, 0x61, 0x65, 0x6d,
      0x65, 0x20, 0x57, 0x2e, 0x20, 0x47, 0x69, 0x6c, 0x6c, 0x2e, 0x20, 0x52,
      0x65, 0x6c, 0x65, 0x61, 0x73, 0x65, 0x64, 0x20, 0x69, 0x6e, 0x74, 0x6f,
      0x20, 0x74, 0x68, 0x65, 0x20, 0x70, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20,
      0x64, 0x6f, 0x6d, 0x61, 0x69, 0x6e, 0x2e, 0x20, 0x4e, 0x6f, 0x20, 0x57,
      0x61, 0x72, 0x72, 0x61, 0x6e, 0x74, 0x79, 0x2c, 0x20, 0x55, 0x73, 0x65,
      0x20, 0x61, 0x74, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x6f, 0x77, 0x6e,
      0x20, 0x72, 0x69, 0x73, 0x6b, 0x2e, 0x00, 0x00, 0x64, 0x65, 0x73, 0x63,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x49, 0x45, 0x43, 0x20,
      0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x69,
      0x65, 0x63, 0x2e, 0x63, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x16, 0x49, 0x45, 0x43, 0x20, 0x68, 0x74, 0x74,
      0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x65, 0x63, 0x2e,
      0x63, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2e,
      0x49, 0x45, 0x43, 0x20, 0x36, 0x31, 0x39, 0x36, 0x36, 0x2d, 0x32, 0x2e,
      0x31, 0x20, 0x44, 0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, 0x20, 0x52, 0x47,
      0x42, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x75, 0x72, 0x20, 0x73, 0x70, 0x61,
      0x63, 0x65, 0x20, 0x2d, 0x20, 0x73, 0x52, 0x47, 0x42, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x49, 0x45, 0x43,
      0x20, 0x36, 0x31, 0x39, 0x36, 0x36, 0x2d, 0x32, 0x2e, 0x31, 0x20, 0x44,
      0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, 0x20, 0x52, 0x47, 0x42, 0x20, 0x63,
      0x6f, 0x6c, 0x6f, 0x75, 0x72, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x20,
      0x2d, 0x20, 0x73, 0x52, 0x47, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x73, 0x69, 0x67, 0x20, 0x00, 0x00, 0x00, 0x00,
      0x43, 0x52, 0x54, 0x20, 0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x0d, 0x49, 0x45, 0x43, 0x36, 0x31, 0x39, 0x36, 0x36,
      0x2d, 0x32, 0x2e, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x0d, 0x49, 0x45, 0x43, 0x36, 0x31, 0x39, 0x36, 0x36,
      0x2d, 0x32, 0x2e, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x76, 0x69, 0x65, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0xa4, 0x7c,
      0x00, 0x14, 0x5f, 0x30, 0x00, 0x10, 0xce, 0x02, 0x00, 0x03, 0xed, 0xb2,
      0x00, 0x04, 0x13, 0x0a, 0x00, 0x03, 0x5c, 0x67, 0x00, 0x00, 0x00, 0x01,
      0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x0a, 0x3d,
      0x00, 0x50, 0x00, 0x00, 0x00, 0x57, 0x1e, 0xb8, 0x6d, 0x65, 0x61, 0x73,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x02, 0x8f, 0x00, 0x00, 0x00, 0x02, 0x58, 0x59, 0x5a, 0x20,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x51, 0x00, 0x01, 0x00, 0x00,
      0x00, 0x01, 0x16, 0xcc, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6f, 0xa0,
      0x00, 0x00, 0x38, 0xf5, 0x00, 0x00, 0x03, 0x90, 0x58, 0x59, 0x5a, 0x20,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x97, 0x00, 0x00, 0xb7, 0x87,
      0x00, 0x00, 0x18, 0xd9, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x24, 0x9f, 0x00, 0x00, 0x0f, 0x84, 0x00, 0x00, 0xb6, 0xc4,
      0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
      0x00, 0x00, 0x00, 0x05, 0x00, 0x0a, 0x00, 0x0f, 0x00, 0x14, 0x00, 0x19,
      0x00, 0x1e, 0x00, 0x23, 0x00, 0x28, 0x00, 0x2d, 0x00, 0x32, 0x00, 0x37,
      0x00, 0x3b, 0x00, 0x40, 0x00, 0x45, 0x00, 0x4a, 0x00, 0x4f, 0x00, 0x54,
      0x00, 0x59, 0x00, 0x5e, 0x00, 0x63, 0x00, 0x68, 0x00, 0x6d, 0x00, 0x72,
      0x00, 0x77, 0x00, 0x7c, 0x00, 0x81, 0x00, 0x86, 0x00, 0x8b, 0x00, 0x90,
      0x00, 0x95, 0x00, 0x9a, 0x00, 0x9f, 0x00, 0xa4, 0x00, 0xa9, 0x00, 0xae,
      0x00, 0xb2, 0x00, 0xb7, 0x00, 0xbc, 0x00, 0xc1, 0x00, 0xc6, 0x00, 0xcb,
      0x00, 0xd0, 0x00, 0xd5, 0x00, 0xdb, 0x00, 0xe0, 0x00, 0xe5, 0x00, 0xeb,
      0x00, 0xf0, 0x00, 0xf6, 0x00, 0xfb, 0x01, 0x01, 0x01, 0x07, 0x01, 0x0d,
      0x01, 0x13, 0x01, 0x19, 0x01, 0x1f, 0x01, 0x25, 0x01, 0x2b, 0x01, 0x32,
      0x01, 0x38, 0x01, 0x3e, 0x01, 0x45, 0x01, 0x4c, 0x01, 0x52, 0x01, 0x59,
      0x01, 0x60, 0x01, 0x67, 0x01, 0x6e, 0x01, 0x75, 0x01, 0x7c, 0x01, 0x83,
      0x01, 0x8b, 0x01, 0x92, 0x01, 0x9a, 0x01, 0xa1, 0x01, 0xa9, 0x01, 0xb1,
      0x01, 0xb9, 0x01, 0xc1, 0x01, 0xc9, 0x01, 0xd1, 0x01, 0xd9, 0x01, 0xe1,
      0x01, 0xe9, 0x01, 0xf2, 0x01, 0xfa, 0x02, 0x03, 0x02, 0x0c, 0x02, 0x14,
      0x02, 0x1d, 0x02, 0x26, 0x02, 0x2f, 0x02, 0x38, 0x02, 0x41, 0x02, 0x4b,
      0x02, 0x54, 0x02, 0x5d, 0x02, 0x67, 0x02, 0x71, 0x02, 0x7a, 0x02, 0x84,
      0x02, 0x8e, 0x02, 0x98, 0x02, 0xa2, 0x02, 0xac, 0x02, 0xb6, 0x02, 0xc1,
      0x02, 0xcb, 0x02, 0xd5, 0x02, 0xe0, 0x02, 0xeb, 0x02, 0xf5, 0x03, 0x00,
      0x03, 0x0b, 0x03, 0x16, 0x03, 0x21, 0x03, 0x2d, 0x03, 0x38, 0x03, 0x43,
      0x03, 0x4f, 0x03, 0x5a, 0x03, 0x66, 0x03, 0x72, 0x03, 0x7e, 0x03, 0x8a,
      0x03, 0x96, 0x03, 0xa2, 0x03, 0xae, 0x03, 0xba, 0x03, 0xc7, 0x03, 0xd3,
      0x03, 0xe0, 0x03, 0xec, 0x03, 0xf9, 0x04, 0x06, 0x04, 0x13, 0x04, 0x20,
      0x04, 0x2d, 0x04, 0x3b, 0x04, 0x48, 0x04, 0x55, 0x04, 0x63, 0x04, 0x71,
      0x04, 0x7e, 0x04, 0x8c, 0x04, 0x9a, 0x04, 0xa8, 0x04, 0xb6, 0x04, 0xc4,
      0x04, 0xd3, 0x04, 0xe1, 0x04, 0xf0, 0x04, 0xfe, 0x05, 0x0d, 0x05, 0x1c,
      0x05, 0x2b, 0x05, 0x3a, 0x05, 0x49, 0x05, 0x58, 0x05, 0x67, 0x05, 0x77,
      0x05, 0x86, 0x05, 0x96, 0x05, 0xa6, 0x05, 0xb5, 0x05, 0xc5, 0x05, 0xd5,
      0x05, 0xe5, 0x05, 0xf6, 0x06, 0x06, 0x06, 0x16, 0x06, 0x27, 0x06, 0x37,
      0x06, 0x48, 0x06, 0x59, 0x06, 0x6a, 0x06, 0x7b, 0x06, 0x8c, 0x06, 0x9d,
      0x06, 0xaf, 0x06, 0xc0, 0x06, 0xd1, 0x06, 0xe3, 0x06, 0xf5, 0x07, 0x07,
      0x07, 0x19, 0x07, 0x2b, 0x07, 0x3d, 0x07, 0x4f, 0x07, 0x61, 0x07, 0x74,
      0x07, 0x86, 0x07, 0x99, 0x07, 0xac, 0x07, 0xbf, 0x07, 0xd2, 0x07, 0xe5,
      0x07, 0xf8, 0x08, 0x0b, 0x08, 0x1f, 0x08, 0x32, 0x08, 0x46, 0x08, 0x5a,
      0x08, 0x6e, 0x08, 0x82, 0x08, 0x96, 0x08, 0xaa, 0x08, 0xbe, 0x08, 0xd2,
      0x08, 0xe7, 0x08, 0xfb, 0x09, 0x10, 0x09, 0x25, 0x09, 0x3a, 0x09, 0x4f,
      0x09, 0x64, 0x09, 0x79, 0x09, 0x8f, 0x09, 0xa4, 0x09, 0xba, 0x09, 0xcf,
      0x09, 0xe5, 0x09, 0xfb, 0x0a, 0x11, 0x0a, 0x27, 0x0a, 0x3d, 0x0a, 0x54,
      0x0a, 0x6a, 0x0a, 0x81, 0x0a, 0x98, 0x0a, 0xae, 0x0a, 0xc5, 0x0a, 0xdc,
      0x0a, 0xf3, 0x0b, 0x0b, 0x0b, 0x22, 0x0b, 0x39, 0x0b, 0x51, 0x0b, 0x69,
      0x0b, 0x80, 0x0b, 0x98, 0x0b, 0xb0, 0x0b, 0xc8, 0x0b, 0xe1, 0x0b, 0xf9,
      0x0c, 0x12, 0x0c, 0x2a, 0x0c, 0x43, 0x0c, 0x5c, 0x0c, 0x75, 0x0c, 0x8e,
      0x0c, 0xa7, 0x0c, 0xc0, 0x0c, 0xd9, 0x0c, 0xf3, 0x0d, 0x0d, 0x0d, 0x26,
      0x0d, 0x40, 0x0d, 0x5a, 0x0d, 0x74, 0x0d, 0x8e, 0x0d, 0xa9, 0x0d, 0xc3,
      0x0d, 0xde, 0x0d, 0xf8, 0x0e, 0x13, 0x0e, 0x2e, 0x0e, 0x49, 0x0e, 0x64,
      0x0e, 0x7f, 0x0e, 0x9b, 0x0e, 0xb6, 0x0e, 0xd2, 0x0e, 0xee, 0x0f, 0x09,
      0x0f, 0x25, 0x0f, 0x41, 0x0f, 0x5e, 0x0f, 0x7a, 0x0f, 0x96, 0x0f, 0xb3,
      0x0f, 0xcf, 0x0f, 0xec, 0x10, 0x09, 0x10, 0x26, 0x10, 0x43, 0x10, 0x61,
      0x10, 0x7e, 0x10, 0x9b, 0x10, 0xb9, 0x10, 0xd7, 0x10, 0xf5, 0x11, 0x13,
      0x11, 0x31, 0x11, 0x4f, 0x11, 0x6d, 0x11, 0x8c, 0x11, 0xaa, 0x11, 0xc9,
      0x11, 0xe8, 0x12, 0x07, 0x12, 0x26, 0x12, 0x45, 0x12, 0x64, 0x12, 0x84,
      0x12, 0xa3, 0x12, 0xc3, 0x12, 0xe3, 0x13, 0x03, 0x13, 0x23, 0x13, 0x43,
      0x13, 0x63, 0x13, 0x83, 0x13, 0xa4, 0x13, 0xc5, 0x13, 0xe5, 0x14, 0x06,
      0x14, 0x27, 0x14, 0x49, 0x14, 0x6a, 0x14, 0x8b, 0x14, 0xad, 0x14, 0xce,
      0x14, 0xf0, 0x15, 0x12, 0x15, 0x34, 0x15, 0x56, 0x15, 0x78, 0x15, 0x9b,
      0x15, 0xbd, 0x15, 0xe0, 0x16, 0x03, 0x16, 0x26, 0x16, 0x49, 0x16, 0x6c,
      0x16, 0x8f, 0x16, 0xb2, 0x16, 0xd6, 0x16, 0xfa, 0x17, 0x1d, 0x17, 0x41,
      0x17, 0x65, 0x17, 0x89, 0x17, 0xae, 0x17, 0xd2, 0x17, 0xf7, 0x18, 0x1b,
      0x18, 0x40, 0x18, 0x65, 0x18, 0x8a, 0x18, 0xaf, 0x18, 0xd5, 0x18, 0xfa,
      0x19, 0x20, 0x19, 0x45, 0x19, 0x6b, 0x19, 0x91, 0x19, 0xb7, 0x19, 0xdd,
      0x1a, 0x04, 0x1a, 0x2a, 0x1a, 0x51, 0x1a, 0x77, 0x1a, 0x9e, 0x1a, 0xc5,
      0x1a, 0xec, 0x1b, 0x14, 0x1b, 0x3b, 0x1b, 0x63, 0x1b, 0x8a, 0x1b, 0xb2,
      0x1b, 0xda, 0x1c, 0x02, 0x1c, 0x2a, 0x1c, 0x52, 0x1c, 0x7b, 0x1c, 0xa3,
      0x1c, 0xcc, 0x1c, 0xf5, 0x1d, 0x1e, 0x1d, 0x47, 0x1d, 0x70, 0x1d, 0x99,
      0x1d, 0xc3, 0x1d, 0xec, 0x1e, 0x16, 0x1e, 0x40, 0x1e, 0x6a, 0x1e, 0x94,
      0x1e, 0xbe, 0x1e, 0xe9, 0x1f, 0x13, 0x1f, 0x3e, 0x1f, 0x69, 0x1f, 0x94,
      0x1f, 0xbf, 0x1f, 0xea, 0x20, 0x15, 0x20, 0x41, 0x20, 0x6c, 0x20, 0x98,
      0x20, 0xc4, 0x20, 0xf0, 0x21, 0x1c, 0x21, 0x48, 0x21, 0x75, 0x21, 0xa1,
      0x21, 0xce, 0x21, 0xfb, 0x22, 0x27, 0x22, 0x55, 0x22, 0x82, 0x22, 0xaf,
      0x22, 0xdd, 0x23, 0x0a, 0x23, 0x38, 0x23, 0x66, 0x23, 0x94, 0x23, 0xc2,
      0x23, 0xf0, 0x24, 0x1f, 0x24, 0x4d, 0x24, 0x7c, 0x24, 0xab, 0x24, 0xda,
      0x25, 0x09, 0x25, 0x38, 0x25, 0x68, 0x25, 0x97, 0x25, 0xc7, 0x25, 0xf7,
      0x26, 0x27, 0x26, 0x57, 0x26, 0x87, 0x26, 0xb7, 0x26, 0xe8, 0x27, 0x18,
      0x27, 0x49, 0x27, 0x7a, 0x27, 0xab, 0x27, 0xdc, 0x28, 0x0d, 0x28, 0x3f,
      0x28, 0x71, 0x28, 0xa2, 0x28, 0xd4, 0x29, 0x06, 0x29, 0x38, 0x29, 0x6b,
      0x29, 0x9d, 0x29, 0xd0, 0x2a, 0x02, 0x2a, 0x35, 0x2a, 0x68, 0x2a, 0x9b,
      0x2a, 0xcf, 0x2b, 0x02, 0x2b, 0x36, 0x2b, 0x69, 0x2b, 0x9d, 0x2b, 0xd1,
      0x2c, 0x05, 0x2c, 0x39, 0x2c, 0x6e, 0x2c, 0xa2, 0x2c, 0xd7, 0x2d, 0x0c,
      0x2d, 0x41, 0x2d, 0x76, 0x2d, 0xab, 0x2d, 0xe1, 0x2e, 0x16, 0x2e, 0x4c,
      0x2e, 0x82, 0x2e, 0xb7, 0x2e, 0xee, 0x2f, 0x24, 0x2f, 0x5a, 0x2f, 0x91,
      0x2f, 0xc7, 0x2f, 0xfe, 0x30, 0x35, 0x30, 0x6c, 0x30, 0xa4, 0x30, 0xdb,
      0x31, 0x12, 0x31, 0x4a, 0x31, 0x82, 0x31, 0xba, 0x31, 0xf2, 0x32, 0x2a,
      0x32, 0x63, 0x32, 0x9b, 0x32, 0xd4, 0x33, 0x0d, 0x33, 0x46, 0x33, 0x7f,
      0x33, 0xb8, 0x33, 0xf1, 0x34, 0x2b, 0x34, 0x65, 0x34, 0x9e, 0x34, 0xd8,
      0x35, 0x13, 0x35, 0x4d, 0x35, 0x87, 0x35, 0xc2, 0x35, 0xfd, 0x36, 0x37,
      0x36, 0x72, 0x36, 0xae, 0x36, 0xe9, 0x37, 0x24, 0x37, 0x60, 0x37, 0x9c,
      0x37, 0xd7, 0x38, 0x14, 0x38, 0x50, 0x38, 0x8c, 0x38, 0xc8, 0x39, 0x05,
      0x39, 0x42, 0x39, 0x7f, 0x39, 0xbc, 0x39, 0xf9, 0x3a, 0x36, 0x3a, 0x74,
      0x3a, 0xb2, 0x3a, 0xef, 0x3b, 0x2d, 0x3b, 0x6b, 0x3b, 0xaa, 0x3b, 0xe8,
      0x3c, 0x27, 0x3c, 0x65, 0x3c, 0xa4, 0x3c, 0xe3, 0x3d, 0x22, 0x3d, 0x61,
      0x3d, 0xa1, 0x3d, 0xe0, 0x3e, 0x20, 0x3e, 0x60, 0x3e, 0xa0, 0x3e, 0xe0,
      0x3f, 0x21, 0x3f, 0x61, 0x3f, 0xa2, 0x3f, 0xe2, 0x40, 0x23, 0x40, 0x64,
      0x40, 0xa6, 0x40, 0xe7, 0x41, 0x29, 0x41, 0x6a, 0x41, 0xac, 0x41, 0xee,
      0x42, 0x30, 0x42, 0x72, 0x42, 0xb5, 0x42, 0xf7, 0x43, 0x3a, 0x43, 0x7d,
      0x43, 0xc0, 0x44, 0x03, 0x44, 0x47, 0x44, 0x8a, 0x44, 0xce, 0x45, 0x12,
      0x45, 0x55, 0x45, 0x9a, 0x45, 0xde, 0x46, 0x22, 0x46, 0x67, 0x46, 0xab,
      0x46, 0xf0, 0x47, 0x35, 0x47, 0x7b, 0x47, 0xc0, 0x48, 0x05, 0x48, 0x4b,
      0x48, 0x91, 0x48, 0xd7, 0x49, 0x1d, 0x49, 0x63, 0x49, 0xa9, 0x49, 0xf0,
      0x4a, 0x37, 0x4a, 0x7d, 0x4a, 0xc4, 0x4b, 0x0c, 0x4b, 0x53, 0x4b, 0x9a,
      0x4b, 0xe2, 0x4c, 0x2a, 0x4c, 0x72, 0x4c, 0xba, 0x4d, 0x02, 0x4d, 0x4a,
      0x4d, 0x93, 0x4d, 0xdc, 0x4e, 0x25, 0x4e, 0x6e, 0x4e, 0xb7, 0x4f, 0x00,
      0x4f, 0x49, 0x4f, 0x93, 0x4f, 0xdd, 0x50, 0x27, 0x50, 0x71, 0x50, 0xbb,
      0x51, 0x06, 0x51, 0x50, 0x51, 0x9b, 0x51, 0xe6, 0x52, 0x31, 0x52, 0x7c,
      0x52, 0xc7, 0x53, 0x13, 0x53, 0x5f, 0x53, 0xaa, 0x53, 0xf6, 0x54, 0x42,
      0x54, 0x8f, 0x54, 0xdb, 0x55, 0x28, 0x55, 0x75, 0x55, 0xc2, 0x56, 0x0f,
      0x56, 0x5c, 0x56, 0xa9, 0x56, 0xf7, 0x57, 0x44, 0x57, 0x92, 0x57, 0xe0,
      0x58, 0x2f, 0x58, 0x7d, 0x58, 0xcb, 0x59, 0x1a, 0x59, 0x69, 0x59, 0xb8,
      0x5a, 0x07, 0x5a, 0x56, 0x5a, 0xa6, 0x5a, 0xf5, 0x5b, 0x45, 0x5b, 0x95,
      0x5b, 0xe5, 0x5c, 0x35, 0x5c, 0x86, 0x5c, 0xd6, 0x5d, 0x27, 0x5d, 0x78,
      0x5d, 0xc9, 0x5e, 0x1a, 0x5e, 0x6c, 0x5e, 0xbd, 0x5f, 0x0f, 0x5f, 0x61,
      0x5f, 0xb3, 0x60, 0x05, 0x60, 0x57, 0x60, 0xaa, 0x60, 0xfc, 0x61, 0x4f,
      0x61, 0xa2, 0x61, 0xf5, 0x62, 0x49, 0x62, 0x9c, 0x62, 0xf0, 0x63, 0x43,
      0x63, 0x97, 0x63, 0xeb, 0x64, 0x40, 0x64, 0x94, 0x64, 0xe9, 0x65, 0x3d,
      0x65, 0x92, 0x65, 0xe7, 0x66, 0x3d, 0x66, 0x92, 0x66, 0xe8, 0x67, 0x3d,
      0x67, 0x93, 0x67, 0xe9, 0x68, 0x3f, 0x68, 0x96, 0x68, 0xec, 0x69, 0x43,
      0x69, 0x9a, 0x69, 0xf1, 0x6a, 0x48, 0x6a, 0x9f, 0x6a, 0xf7, 0x6b, 0x4f,
      0x6b, 0xa7, 0x6b, 0xff, 0x6c, 0x57, 0x6c, 0xaf, 0x6d, 0x08, 0x6d, 0x60,
      0x6d, 0xb9, 0x6e, 0x12, 0x6e, 0x6b, 0x6e, 0xc4, 0x6f, 0x1e, 0x6f, 0x78,
      0x6f, 0xd1, 0x70, 0x2b, 0x70, 0x86, 0x70, 0xe0, 0x71, 0x3a, 0x71, 0x95,
      0x71, 0xf0, 0x72, 0x4b, 0x72, 0xa6, 0x73, 0x01, 0x73, 0x5d, 0x73, 0xb8,
      0x74, 0x14, 0x74, 0x70, 0x74, 0xcc, 0x75, 0x28, 0x75, 0x85, 0x75, 0xe1,
      0x76, 0x3e, 0x76, 0x9b, 0x76, 0xf8, 0x77, 0x56, 0x77, 0xb3, 0x78, 0x11,
      0x78, 0x6e, 0x78, 0xcc, 0x79, 0x2a, 0x79, 0x89, 0x79, 0xe7, 0x7a, 0x46,
      0x7a, 0xa5, 0x7b, 0x04, 0x7b, 0x63, 0x7b, 0xc2, 0x7c, 0x21, 0x7c, 0x81,
      0x7c, 0xe1, 0x7d, 0x41, 0x7d, 0xa1, 0x7e, 0x01, 0x7e, 0x62, 0x7e, 0xc2,
      0x7f, 0x23, 0x7f, 0x84, 0x7f, 0xe5, 0x80, 0x47, 0x80, 0xa8, 0x81, 0x0a,
      0x81, 0x6b, 0x81, 0xcd, 0x82, 0x30, 0x82, 0x92, 0x82, 0xf4, 0x83, 0x57,
      0x83, 0xba, 0x84, 0x1d, 0x84, 0x80, 0x84, 0xe3, 0x85, 0x47, 0x85, 0xab,
      0x86, 0x0e, 0x86, 0x72, 0x86, 0xd7, 0x87, 0x3b, 0x87, 0x9f, 0x88, 0x04,
      0x88, 0x69, 0x88, 0xce, 0x89, 0x33, 0x89, 0x99, 0x89, 0xfe, 0x8a, 0x64,
      0x8a, 0xca, 0x8b, 0x30, 0x8b, 0x96, 0x8b, 0xfc, 0x8c, 0x63, 0x8c, 0xca,
      0x8d, 0x31, 0x8d, 0x98, 0x8d, 0xff, 0x8e, 0x66, 0x8e, 0xce, 0x8f, 0x36,
      0x8f, 0x9e, 0x90, 0x06, 0x90, 0x6e, 0x90, 0xd6, 0x91, 0x3f, 0x91, 0xa8,
      0x92, 0x11, 0x92, 0x7a, 0x92, 0xe3, 0x93, 0x4d, 0x93, 0xb6, 0x94, 0x20,
      0x94, 0x8a, 0x94, 0xf4, 0x95, 0x5f, 0x95, 0xc9, 0x96, 0x34, 0x96, 0x9f,
      0x97, 0x0a, 0x97, 0x75, 0x97, 0xe0, 0x98, 0x4c, 0x98, 0xb8, 0x99, 0x24,
      0x99, 0x90, 0x99, 0xfc, 0x9a, 0x68, 0x9a, 0xd5, 0x9b, 0x42, 0x9b, 0xaf,
      0x9c, 0x1c, 0x9c, 0x89, 0x9c, 0xf7, 0x9d, 0x64, 0x9d, 0xd2, 0x9e, 0x40,
      0x9e, 0xae, 0x9f, 0x1d, 0x9f, 0x8b, 0x9f, 0xfa, 0xa0, 0x69, 0xa0, 0xd8,
      0xa1, 0x47, 0xa1, 0xb6, 0xa2, 0x26, 0xa2, 0x96, 0xa3, 0x06, 0xa3, 0x76,
      0xa3, 0xe6, 0xa4, 0x56, 0xa4, 0xc7, 0xa5, 0x38, 0xa5, 0xa9, 0xa6, 0x1a,
      0xa6, 0x8b, 0xa6, 0xfd, 0xa7, 0x6e, 0xa7, 0xe0, 0xa8, 0x52, 0xa8, 0xc4,
      0xa9, 0x37, 0xa9, 0xa9, 0xaa, 0x1c, 0xaa, 0x8f, 0xab, 0x02, 0xab, 0x75,
      0xab, 0xe9, 0xac, 0x5c, 0xac, 0xd0, 0xad, 0x44, 0xad, 0xb8, 0xae, 0x2d,
      0xae, 0xa1, 0xaf, 0x16, 0xaf, 0x8b, 0xb0, 0x00, 0xb0, 0x75, 0xb0, 0xea,
      0xb1, 0x60, 0xb1, 0xd6, 0xb2, 0x4b, 0xb2, 0xc2, 0xb3, 0x38, 0xb3, 0xae,
      0xb4, 0x25, 0xb4, 0x9c, 0xb5, 0x13, 0xb5, 0x8a, 0xb6, 0x01, 0xb6, 0x79,
      0xb6, 0xf0, 0xb7, 0x68, 0xb7, 0xe0, 0xb8, 0x59, 0xb8, 0xd1, 0xb9, 0x4a,
      0xb9, 0xc2, 0xba, 0x3b, 0xba, 0xb5, 0xbb, 0x2e, 0xbb, 0xa7, 0xbc, 0x21,
      0xbc, 0x9b, 0xbd, 0x15, 0xbd, 0x8f, 0xbe, 0x0a, 0xbe, 0x84, 0xbe, 0xff,
      0xbf, 0x7a, 0xbf, 0xf5, 0xc0, 0x70, 0xc0, 0xec, 0xc1, 0x67, 0xc1, 0xe3,
      0xc2, 0x5f, 0xc2, 0xdb, 0xc3, 0x58, 0xc3, 0xd4, 0xc4, 0x51, 0xc4, 0xce,
      0xc5, 0x4b, 0xc5, 0xc8, 0xc6, 0x46, 0xc6, 0xc3, 0xc7, 0x41, 0xc7, 0xbf,
      0xc8, 0x3d, 0xc8, 0xbc, 0xc9, 0x3a, 0xc9, 0xb9, 0xca, 0x38, 0xca, 0xb7,
      0xcb, 0x36, 0xcb, 0xb6, 0xcc, 0x35, 0xcc, 0xb5, 0xcd, 0x35, 0xcd, 0xb5,
      0xce, 0x36, 0xce, 0xb6, 0xcf, 0x37, 0xcf, 0xb8, 0xd0, 0x39, 0xd0, 0xba,
      0xd1, 0x3c, 0xd1, 0xbe, 0xd2, 0x3f, 0xd2, 0xc1, 0xd3, 0x44, 0xd3, 0xc6,
      0xd4, 0x49, 0xd4, 0xcb, 0xd5, 0x4e, 0xd5, 0xd1, 0xd6, 0x55, 0xd6, 0xd8,
      0xd7, 0x5c, 0xd7, 0xe0, 0xd8, 0x64, 0xd8, 0xe8, 0xd9, 0x6c, 0xd9, 0xf1,
      0xda, 0x76, 0xda, 0xfb, 0xdb, 0x80, 0xdc, 0x05, 0xdc, 0x8a, 0xdd, 0x10,
      0xdd, 0x96, 0xde, 0x1c, 0xde, 0xa2, 0xdf, 0x29, 0xdf, 0xaf, 0xe0, 0x36,
      0xe0, 0xbd, 0xe1, 0x44, 0xe1, 0xcc, 0xe2, 0x53, 0xe2, 0xdb, 0xe3, 0x63,
      0xe3, 0xeb, 0xe4, 0x73, 0xe4, 0xfc, 0xe5, 0x84, 0xe6, 0x0d, 0xe6, 0x96,
      0xe7, 0x1f, 0xe7, 0xa9, 0xe8, 0x32, 0xe8, 0xbc, 0xe9, 0x46, 0xe9, 0xd0,
      0xea, 0x5b, 0xea, 0xe5, 0xeb, 0x70, 0xeb, 0xfb, 0xec, 0x86, 0xed, 0x11,
      0xed, 0x9c, 0xee, 0x28, 0xee, 0xb4, 0xef, 0x40, 0xef, 0xcc, 0xf0, 0x58,
      0xf0, 0xe5, 0xf1, 0x72, 0xf1, 0xff, 0xf2, 0x8c, 0xf3, 0x19, 0xf3, 0xa7,
      0xf4, 0x34, 0xf4, 0xc2, 0xf5, 0x50, 0xf5, 0xde, 0xf6, 0x6d, 0xf6, 0xfb,
      0xf7, 0x8a, 0xf8, 0x19, 0xf8, 0xa8, 0xf9, 0x38, 0xf9, 0xc7, 0xfa, 0x57,
      0xfa, 0xe7, 0xfb, 0x77, 0xfc, 0x07, 0xfc, 0x98, 0xfd, 0x29, 0xfd, 0xba,
      0xfe, 0x4b, 0xfe, 0xdc, 0xff, 0x6d, 0xff, 0xff
    };

  StringInfo
    *profile;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (GetImageProfile(image,"icc") != (const StringInfo *) NULL)
    return(MagickFalse);
  profile=AcquireStringInfo(sizeof(sRGBProfile));
  SetStringInfoDatum(profile,sRGBProfile);
  status=SetImageProfile(image,"icc",profile,exception);
  profile=DestroyStringInfo(profile);
  return(status);
}

MagickExport MagickBooleanType ProfileImage(Image *image,const char *name,
  const void *datum,const size_t length,ExceptionInfo *exception)
{
#define ProfileImageTag  "Profile/Image"
#define ThrowProfileException(severity,tag,context) \
{ \
  if (source_profile != (cmsHPROFILE) NULL) \
    (void) cmsCloseProfile(source_profile); \
  if (target_profile != (cmsHPROFILE) NULL) \
    (void) cmsCloseProfile(target_profile); \
  ThrowBinaryException(severity,tag,context); \
}

  MagickBooleanType
    status;

  StringInfo
    *profile;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(name != (const char *) NULL);
  if ((datum == (const void *) NULL) || (length == 0))
    {
      char
        *next;

      /*
        Delete image profile(s).
      */
      ResetImageProfileIterator(image);
      for (next=GetNextImageProfile(image); next != (const char *) NULL; )
      {
        if (IsOptionMember(next,name) != MagickFalse)
          {
            (void) DeleteImageProfile(image,next);
            ResetImageProfileIterator(image);
          }
        next=GetNextImageProfile(image);
      }
      return(MagickTrue);
    }
  /*
    Add a ICC, IPTC, or generic profile to the image.
  */
  status=MagickTrue;
  profile=AcquireStringInfo((size_t) length);
  SetStringInfoDatum(profile,(unsigned char *) datum);
  if ((LocaleCompare(name,"icc") != 0) && (LocaleCompare(name,"icm") != 0))
    status=SetImageProfile(image,name,profile,exception);
  else
    {
      const StringInfo
        *icc_profile;

      icc_profile=GetImageProfile(image,"icc");
      if ((icc_profile != (const StringInfo *) NULL) &&
          (CompareStringInfo(icc_profile,profile) == 0))
        {
          const char
            *value;

          value=GetImageProperty(image,"exif:ColorSpace",exception);
          (void) value;
          if (LocaleCompare(value,"1") != 0)
            (void) SetsRGBImageProfile(image,exception);
          value=GetImageProperty(image,"exif:InteroperabilityIndex",exception);
          if (LocaleCompare(value,"R98.") != 0)
            (void) SetsRGBImageProfile(image,exception);
          /* Future.
          value=GetImageProperty(image,"exif:InteroperabilityIndex",exception);
          if (LocaleCompare(value,"R03.") != 0)
            (void) SetAdobeRGB1998ImageProfile(image,exception);
          */
          icc_profile=GetImageProfile(image,"icc");
        }
      if ((icc_profile != (const StringInfo *) NULL) &&
          (CompareStringInfo(icc_profile,profile) == 0))
        {
          profile=DestroyStringInfo(profile);
          return(MagickTrue);
        }
#if !defined(MAGICKCORE_LCMS_DELEGATE)
      (void) ThrowMagickException(exception,GetMagickModule(),
        MissingDelegateWarning,"DelegateLibrarySupportNotBuiltIn",
        "'%s' (LCMS)",image->filename);
#else
      {
        cmsHPROFILE
          source_profile;

        CMSExceptionInfo
          cms_exception;

        /*
          Transform pixel colors as defined by the color profiles.
        */
        cmsSetLogErrorHandler(CMSExceptionHandler);
        cms_exception.image=image;
        cms_exception.exception=exception;
        (void) cms_exception;
        source_profile=cmsOpenProfileFromMemTHR((cmsContext) &cms_exception,
          GetStringInfoDatum(profile),(cmsUInt32Number)
          GetStringInfoLength(profile));
        if (source_profile == (cmsHPROFILE) NULL)
          ThrowBinaryException(ResourceLimitError,
            "ColorspaceColorProfileMismatch",name);
        if ((cmsGetDeviceClass(source_profile) != cmsSigLinkClass) &&
            (icc_profile == (StringInfo *) NULL))
          status=SetImageProfile(image,name,profile,exception);
        else
          {
            CacheView
              *image_view;

            ColorspaceType
              source_colorspace,
              target_colorspace;

            cmsColorSpaceSignature
              signature;

            cmsHPROFILE
              target_profile;

            cmsHTRANSFORM
              *magick_restrict transform;

            cmsUInt32Number
              flags,
              source_type,
              target_type;

            int
              intent;

            LCMSType
              **magick_restrict source_pixels,
              **magick_restrict target_pixels;

#if defined(LCMSHDRI)
            LCMSType
              source_scale,
              target_scale;
#endif

            MagickOffsetType
              progress;

            size_t
              source_channels,
              target_channels;

            ssize_t
              y;

            target_profile=(cmsHPROFILE) NULL;
            if (icc_profile != (StringInfo *) NULL)
              {
                target_profile=source_profile;
                source_profile=cmsOpenProfileFromMemTHR((cmsContext)
                  &cms_exception,GetStringInfoDatum(icc_profile),
                  (cmsUInt32Number) GetStringInfoLength(icc_profile));
                if (source_profile == (cmsHPROFILE) NULL)
                  ThrowProfileException(ResourceLimitError,
                    "ColorspaceColorProfileMismatch",name);
              }
#if defined(LCMSHDRI)
            source_scale=1.0;
#endif
            source_colorspace=sRGBColorspace;
            source_channels=3;
            switch (cmsGetColorSpace(source_profile))
            {
              case cmsSigCmykData:
              {
                source_colorspace=CMYKColorspace;
                source_channels=4;
#if defined(LCMSHDRI)
                source_type=(cmsUInt32Number) TYPE_CMYK_DBL;
                source_scale=100.0;
#else
                source_type=(cmsUInt32Number) TYPE_CMYK_16;
#endif
                break;
              }
              case cmsSigGrayData:
              {
                source_colorspace=GRAYColorspace;
                source_channels=1;
#if defined(LCMSHDRI)
                source_type=(cmsUInt32Number) TYPE_GRAY_DBL;
#else
                source_type=(cmsUInt32Number) TYPE_GRAY_16;
#endif
                break;
              }
              case cmsSigLabData:
              {
                source_colorspace=LabColorspace;
#if defined(LCMSHDRI)
                source_type=(cmsUInt32Number) TYPE_Lab_DBL;
                source_scale=100.0;
#else
                source_type=(cmsUInt32Number) TYPE_Lab_16;
#endif
                break;
              }
#if !defined(LCMSHDRI)
              case cmsSigLuvData:
              {
                source_colorspace=YUVColorspace;
                source_type=(cmsUInt32Number) TYPE_YUV_16;
                break;
              }
#endif
              case cmsSigRgbData:
              {
                source_colorspace=sRGBColorspace;
#if defined(LCMSHDRI)
                source_type=(cmsUInt32Number) TYPE_RGB_DBL;
#else
                source_type=(cmsUInt32Number) TYPE_RGB_16;
#endif
                break;
              }
              case cmsSigXYZData:
              {
                source_colorspace=XYZColorspace;
#if defined(LCMSHDRI)
                source_type=(cmsUInt32Number) TYPE_XYZ_DBL;
#else
                source_type=(cmsUInt32Number) TYPE_XYZ_16;
#endif
                break;
              }
#if !defined(LCMSHDRI)
              case cmsSigYCbCrData:
              {
                source_colorspace=YUVColorspace;
                source_type=(cmsUInt32Number) TYPE_YCbCr_16;
                break;
              }
#endif
              default:
                ThrowProfileException(ImageError,
                  "ColorspaceColorProfileMismatch",name);
            }
            (void) source_colorspace;
            signature=cmsGetPCS(source_profile);
            if (target_profile != (cmsHPROFILE) NULL)
              signature=cmsGetColorSpace(target_profile);
#if defined(LCMSHDRI)
            target_scale=1.0;
#endif
            target_channels=3;
            switch (signature)
            {
              case cmsSigCmykData:
              {
                target_colorspace=CMYKColorspace;
                target_channels=4;
#if defined(LCMSHDRI)
                target_type=(cmsUInt32Number) TYPE_CMYK_DBL;
                target_scale=0.01;
#else
                target_type=(cmsUInt32Number) TYPE_CMYK_16;
#endif
                break;
              }
              case cmsSigGrayData:
              {
                target_colorspace=GRAYColorspace;
                target_channels=1;
#if defined(LCMSHDRI)
                target_type=(cmsUInt32Number) TYPE_GRAY_DBL;
#else
                target_type=(cmsUInt32Number) TYPE_GRAY_16;
#endif
                break;
              }
              case cmsSigLabData:
              {
                target_colorspace=LabColorspace;
#if defined(LCMSHDRI)
                target_type=(cmsUInt32Number) TYPE_Lab_DBL;
                target_scale=0.01;
#else
                target_type=(cmsUInt32Number) TYPE_Lab_16;
#endif
                break;
              }
#if !defined(LCMSHDRI)
              case cmsSigLuvData:
              {
                target_colorspace=YUVColorspace;
                target_type=(cmsUInt32Number) TYPE_YUV_16;
                break;
              }
#endif
              case cmsSigRgbData:
              {
                target_colorspace=sRGBColorspace;
#if defined(LCMSHDRI)
                target_type=(cmsUInt32Number) TYPE_RGB_DBL;
#else
                target_type=(cmsUInt32Number) TYPE_RGB_16;
#endif
                break;
              }
              case cmsSigXYZData:
              {
                target_colorspace=XYZColorspace;
#if defined(LCMSHDRI)
                target_type=(cmsUInt32Number) TYPE_XYZ_DBL;
#else
                target_type=(cmsUInt32Number) TYPE_XYZ_16;
#endif
                break;
              }
#if !defined(LCMSHDRI)
              case cmsSigYCbCrData:
              {
                target_colorspace=YUVColorspace;
                target_type=(cmsUInt32Number) TYPE_YCbCr_16;
                break;
              }
#endif
              default:
                ThrowProfileException(ImageError,
                  "ColorspaceColorProfileMismatch",name);
            }
            switch (image->rendering_intent)
            {
              case AbsoluteIntent: intent=INTENT_ABSOLUTE_COLORIMETRIC; break;
              case PerceptualIntent: intent=INTENT_PERCEPTUAL; break;
              case RelativeIntent: intent=INTENT_RELATIVE_COLORIMETRIC; break;
              case SaturationIntent: intent=INTENT_SATURATION; break;
              default: intent=INTENT_PERCEPTUAL; break;
            }
            flags=cmsFLAGS_HIGHRESPRECALC;
#if defined(cmsFLAGS_BLACKPOINTCOMPENSATION)
            if (image->black_point_compensation != MagickFalse)
              flags|=cmsFLAGS_BLACKPOINTCOMPENSATION;
#endif
            transform=AcquireTransformThreadSet(image,source_profile,
              source_type,target_profile,target_type,intent,flags,
	      &cms_exception);
            if (transform == (cmsHTRANSFORM *) NULL)
              ThrowProfileException(ImageError,"UnableToCreateColorTransform",
                name);
            /*
              Transform image as dictated by the source & target image profiles.
            */
            source_pixels=AcquirePixelThreadSet(image->columns,source_channels);
            target_pixels=AcquirePixelThreadSet(image->columns,target_channels);
            if ((source_pixels == (LCMSType **) NULL) ||
                (target_pixels == (LCMSType **) NULL))
              {
                target_pixels=DestroyPixelThreadSet(target_pixels);
                source_pixels=DestroyPixelThreadSet(source_pixels);
                transform=DestroyTransformThreadSet(transform);
                ThrowProfileException(ResourceLimitError,
                  "MemoryAllocationFailed",image->filename);
              }
            if (SetImageStorageClass(image,DirectClass,exception) == MagickFalse)
              {
                target_pixels=DestroyPixelThreadSet(target_pixels);
                source_pixels=DestroyPixelThreadSet(source_pixels);
                transform=DestroyTransformThreadSet(transform);
                if (source_profile != (cmsHPROFILE) NULL)
                  (void) cmsCloseProfile(source_profile);
                if (target_profile != (cmsHPROFILE) NULL)
                  (void) cmsCloseProfile(target_profile);
                return(MagickFalse);
              }
            if (target_colorspace == CMYKColorspace)
              (void) SetImageColorspace(image,target_colorspace,exception);
            progress=0;
            image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp parallel for schedule(static) shared(status) \
              magick_number_threads(image,image,image->rows,1)
#endif
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              const int
                id = GetOpenMPThreadId();

              MagickBooleanType
                sync;

              register LCMSType
                *p;

              register Quantum
                *magick_restrict q;

              register ssize_t
                x;

              if (status == MagickFalse)
                continue;
              q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
                exception);
              if (q == (Quantum *) NULL)
                {
                  status=MagickFalse;
                  continue;
                }
              p=source_pixels[id];
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                *p++=LCMSScaleSource(GetPixelRed(image,q));
                if (source_channels > 1)
                  {
                    *p++=LCMSScaleSource(GetPixelGreen(image,q));
                    *p++=LCMSScaleSource(GetPixelBlue(image,q));
                  }
                if (source_channels > 3)
                  *p++=LCMSScaleSource(GetPixelBlack(image,q));
                q+=GetPixelChannels(image);
              }
              cmsDoTransform(transform[id],source_pixels[id],target_pixels[id],
                (unsigned int) image->columns);
              p=target_pixels[id];
              q-=GetPixelChannels(image)*image->columns;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                if (target_channels == 1)
                  SetPixelGray(image,LCMSScaleTarget(*p),q);
                else
                  SetPixelRed(image,LCMSScaleTarget(*p),q);
                p++;
                if (target_channels > 1)
                  {
                    SetPixelGreen(image,LCMSScaleTarget(*p),q);
                    p++;
                    SetPixelBlue(image,LCMSScaleTarget(*p),q);
                    p++;
                  }
                if (target_channels > 3)
                  {
                    SetPixelBlack(image,LCMSScaleTarget(*p),q);
                    p++;
                  }
                q+=GetPixelChannels(image);
              }
              sync=SyncCacheViewAuthenticPixels(image_view,exception);
              if (sync == MagickFalse)
                status=MagickFalse;
              if (image->progress_monitor != (MagickProgressMonitor) NULL)
                {
                  MagickBooleanType
                    proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
                  #pragma omp atomic
#endif
                  progress++;
                  proceed=SetImageProgress(image,ProfileImageTag,progress,
                    image->rows);
                  if (proceed == MagickFalse)
                    status=MagickFalse;
                }
            }
            image_view=DestroyCacheView(image_view);
            (void) SetImageColorspace(image,target_colorspace,exception);
            switch (signature)
            {
              case cmsSigRgbData:
              {
                image->type=image->alpha_trait == UndefinedPixelTrait ?
                  TrueColorType : TrueColorAlphaType;
                break;
              }
              case cmsSigCmykData:
              {
                image->type=image->alpha_trait == UndefinedPixelTrait ?
                  ColorSeparationType : ColorSeparationAlphaType;
                break;
              }
              case cmsSigGrayData:
              {
                image->type=image->alpha_trait == UndefinedPixelTrait ?
                  GrayscaleType : GrayscaleAlphaType;
                break;
              }
              default:
                break;
            }
            target_pixels=DestroyPixelThreadSet(target_pixels);
            source_pixels=DestroyPixelThreadSet(source_pixels);
            transform=DestroyTransformThreadSet(transform);
            if ((status != MagickFalse) &&
                (cmsGetDeviceClass(source_profile) != cmsSigLinkClass))
              status=SetImageProfile(image,name,profile,exception);
            if (target_profile != (cmsHPROFILE) NULL)
              (void) cmsCloseProfile(target_profile);
          }
        (void) cmsCloseProfile(source_profile);
      }
#endif
    }
  profile=DestroyStringInfo(profile);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e I m a g e P r o f i l e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveImageProfile() removes a named profile from the image and returns its
%  value.
%
%  The format of the RemoveImageProfile method is:
%
%      void *RemoveImageProfile(Image *image,const char *name)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o name: the profile name.
%
*/
MagickExport StringInfo *RemoveImageProfile(Image *image,const char *name)
{
  StringInfo
    *profile;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->profiles == (SplayTreeInfo *) NULL)
    return((StringInfo *) NULL);
  WriteTo8BimProfile(image,name,(StringInfo *) NULL);
  profile=(StringInfo *) RemoveNodeFromSplayTree((SplayTreeInfo *)
    image->profiles,name);
  return(profile);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t P r o f i l e I t e r a t o r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetImageProfileIterator() resets the image profile iterator.  Use it in
%  conjunction with GetNextImageProfile() to iterate over all the profiles
%  associated with an image.
%
%  The format of the ResetImageProfileIterator method is:
%
%      ResetImageProfileIterator(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void ResetImageProfileIterator(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->profiles == (SplayTreeInfo *) NULL)
    return;
  ResetSplayTreeIterator((SplayTreeInfo *) image->profiles);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e P r o f i l e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageProfile() adds a named profile to the image.  If a profile with the
%  same name already exists, it is replaced.  This method differs from the
%  ProfileImage() method in that it does not apply CMS color profiles.
%
%  The format of the SetImageProfile method is:
%
%      MagickBooleanType SetImageProfile(Image *image,const char *name,
%        const StringInfo *profile)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o name: the profile name, for example icc, exif, and 8bim (8bim is the
%      Photoshop wrapper for iptc profiles).
%
%    o profile: A StringInfo structure that contains the named profile.
%
*/

static void *DestroyProfile(void *profile)
{
  return((void *) DestroyStringInfo((StringInfo *) profile));
}

static inline const unsigned char *ReadResourceByte(const unsigned char *p,
  unsigned char *quantum)
{
  *quantum=(*p++);
  return(p);
}

static inline const unsigned char *ReadResourceLong(const unsigned char *p,
  unsigned int *quantum)
{
  *quantum=(unsigned int) (*p++) << 24;
  *quantum|=(unsigned int) (*p++) << 16;
  *quantum|=(unsigned int) (*p++) << 8;
  *quantum|=(unsigned int) (*p++);
  return(p);
}

static inline const unsigned char *ReadResourceShort(const unsigned char *p,
  unsigned short *quantum)
{
  *quantum=(unsigned short) (*p++) << 8;
  *quantum|=(unsigned short) (*p++);
  return(p);
}

static inline void WriteResourceLong(unsigned char *p,
  const unsigned int quantum)
{
  unsigned char
    buffer[4];

  buffer[0]=(unsigned char) (quantum >> 24);
  buffer[1]=(unsigned char) (quantum >> 16);
  buffer[2]=(unsigned char) (quantum >> 8);
  buffer[3]=(unsigned char) quantum;
  (void) memcpy(p,buffer,4);
}

static void WriteTo8BimProfile(Image *image,const char *name,
  const StringInfo *profile)
{
  const unsigned char
    *datum,
    *q;

  register const unsigned char
    *p;

  size_t
    length;

  StringInfo
    *profile_8bim;

  ssize_t
    count;

  unsigned char
    length_byte;

  unsigned int
    value;

  unsigned short
    id,
    profile_id;

  if (LocaleCompare(name,"icc") == 0)
    profile_id=0x040f;
  else
    if (LocaleCompare(name,"iptc") == 0)
      profile_id=0x0404;
    else
      if (LocaleCompare(name,"xmp") == 0)
        profile_id=0x0424;
      else
        return;
  profile_8bim=(StringInfo *) GetValueFromSplayTree((SplayTreeInfo *)
    image->profiles,"8bim");
  if (profile_8bim == (StringInfo *) NULL)
    return;
  datum=GetStringInfoDatum(profile_8bim);
  length=GetStringInfoLength(profile_8bim);
  for (p=datum; p < (datum+length-16); )
  {
    q=p;
    if (LocaleNCompare((char *) p,"8BIM",4) != 0)
      break;
    p+=4;
    p=ReadResourceShort(p,&id);
    p=ReadResourceByte(p,&length_byte);
    p+=length_byte;
    if (((length_byte+1) & 0x01) != 0)
      p++;
    if (p > (datum+length-4))
      break;
    p=ReadResourceLong(p,&value);
    count=(ssize_t) value;
    if ((count & 0x01) != 0)
      count++;
    if ((count < 0) || (p > (datum+length-count)) || (count > (ssize_t) length))
      break;
    if (id != profile_id)
      p+=count;
    else
      {
        size_t
          extent,
          offset;

        ssize_t
          extract_extent;

        StringInfo
          *extract_profile;

        extract_extent=0;
        extent=(datum+length)-(p+count);
        if (profile == (StringInfo *) NULL)
          {
            offset=(q-datum);
            extract_profile=AcquireStringInfo(offset+extent);
            (void) memcpy(extract_profile->datum,datum,offset);
          }
        else
          {
            offset=(p-datum);
            extract_extent=profile->length;
            if ((extract_extent & 0x01) != 0)
              extract_extent++;
            extract_profile=AcquireStringInfo(offset+extract_extent+extent);
            (void) memcpy(extract_profile->datum,datum,offset-4);
            WriteResourceLong(extract_profile->datum+offset-4,(unsigned int)
              profile->length);
            (void) memcpy(extract_profile->datum+offset,
              profile->datum,profile->length);
          }
        (void) memcpy(extract_profile->datum+offset+extract_extent,
          p+count,extent);
        (void) AddValueToSplayTree((SplayTreeInfo *) image->profiles,
          ConstantString("8bim"),CloneStringInfo(extract_profile));
        extract_profile=DestroyStringInfo(extract_profile);
        break;
      }
  }
}

static void GetProfilesFromResourceBlock(Image *image,
  const StringInfo *resource_block,ExceptionInfo *exception)
{
  const unsigned char
    *datum;

  register const unsigned char
    *p;

  size_t
    length;

  ssize_t
    count;

  StringInfo
    *profile;

  unsigned char
    length_byte;

  unsigned int
    value;

  unsigned short
    id;

  datum=GetStringInfoDatum(resource_block);
  length=GetStringInfoLength(resource_block);
  for (p=datum; p < (datum+length-16); )
  {
    if (LocaleNCompare((char *) p,"8BIM",4) != 0)
      break;
    p+=4;
    p=ReadResourceShort(p,&id);
    p=ReadResourceByte(p,&length_byte);
    p+=length_byte;
    if (((length_byte+1) & 0x01) != 0)
      p++;
    if (p > (datum+length-4))
      break;
    p=ReadResourceLong(p,&value);
    count=(ssize_t) value;
    if ((p > (datum+length-count)) || (count > (ssize_t) length) || (count < 0))
      break;
    switch (id)
    {
      case 0x03ed:
      {
        unsigned int
          resolution;

        unsigned short
          units;

        /*
          Resolution.
        */
        if (count < 10)
          break;
        p=ReadResourceLong(p,&resolution);
        image->resolution.x=((double) resolution)/65536.0;
        p=ReadResourceShort(p,&units)+2;
        p=ReadResourceLong(p,&resolution)+4;
        image->resolution.y=((double) resolution)/65536.0;
        /*
          Values are always stored as pixels per inch.
        */
        if ((ResolutionType) units != PixelsPerCentimeterResolution)
          image->units=PixelsPerInchResolution;
        else
          {
            image->units=PixelsPerCentimeterResolution;
            image->resolution.x/=2.54;
            image->resolution.y/=2.54;
          }
        break;
      }
      case 0x0404:
      {
        /*
          IPTC Profile
        */
        profile=AcquireStringInfo(count);
        SetStringInfoDatum(profile,p);
        (void) SetImageProfileInternal(image,"iptc",profile,MagickTrue,
          exception);
        profile=DestroyStringInfo(profile);
        p+=count;
        break;
      }
      case 0x040c:
      {
        /*
          Thumbnail.
        */
        p+=count;
        break;
      }
      case 0x040f:
      {
        /*
          ICC Profile.
        */
        profile=AcquireStringInfo(count);
        SetStringInfoDatum(profile,p);
        (void) SetImageProfileInternal(image,"icc",profile,MagickTrue,
          exception);
        profile=DestroyStringInfo(profile);
        p+=count;
        break;
      }
      case 0x0422:
      {
        /*
          EXIF Profile.
        */
        profile=AcquireStringInfo(count);
        SetStringInfoDatum(profile,p);
        (void) SetImageProfileInternal(image,"exif",profile,MagickTrue,
          exception);
        profile=DestroyStringInfo(profile);
        p+=count;
        break;
      }
      case 0x0424:
      {
        /*
          XMP Profile.
        */
        profile=AcquireStringInfo(count);
        SetStringInfoDatum(profile,p);
        (void) SetImageProfileInternal(image,"xmp",profile,MagickTrue,
          exception);
        profile=DestroyStringInfo(profile);
        p+=count;
        break;
      }
      default:
      {
        p+=count;
        break;
      }
    }
    if ((count & 0x01) != 0)
      p++;
  }
}

#if defined(MAGICKCORE_XML_DELEGATE)
static MagickBooleanType ValidateXMPProfile(const StringInfo *profile)
{
  xmlDocPtr
    document;

  /*
    Parse XML profile.
  */
  document=xmlReadMemory((const char *) GetStringInfoDatum(profile),(int)
    GetStringInfoLength(profile),"xmp.xml",NULL,XML_PARSE_NOERROR |
    XML_PARSE_NOWARNING);
  if (document == (xmlDocPtr) NULL)
    return(MagickFalse);
  xmlFreeDoc(document);
  return(MagickTrue);
}
#else
static unsigned char *FindNeedleInHaystack(unsigned char *haystack,
  const char *needle)
{
  size_t
    length;

  unsigned char
    *c;

  length=strlen(needle);
  for (c=haystack; *c != '\0'; c++)
    if (LocaleNCompare((const char *) c,needle,length) == 0)
      return(c);
  return((unsigned char *) NULL);
}

static MagickBooleanType ValidateXMPProfile(const StringInfo *profile)
{
  unsigned char
    *p;

  p=FindNeedleInHaystack(GetStringInfoDatum(profile),"x:xmpmeta");
  if (p == (unsigned char *) NULL)
    p=FindNeedleInHaystack(GetStringInfoDatum(profile),"rdf:RDF");
  return(p == (unsigned char *) NULL ? MagickFalse : MagickTrue);
}
#endif

static MagickBooleanType SetImageProfileInternal(Image *image,const char *name,
  const StringInfo *profile,const MagickBooleanType recursive,
  ExceptionInfo *exception)
{
  char
    key[MagickPathExtent],
    property[MagickPathExtent];

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if ((LocaleCompare(name,"xmp") == 0) &&
      (ValidateXMPProfile(profile) == MagickFalse))
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageWarning,
        "CorruptImageProfile","`%s'",name);
      return(MagickTrue);
    }
  if (image->profiles == (SplayTreeInfo *) NULL)
    image->profiles=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
      DestroyProfile);
  (void) CopyMagickString(key,name,MagickPathExtent);
  LocaleLower(key);
  status=AddValueToSplayTree((SplayTreeInfo *) image->profiles,
    ConstantString(key),CloneStringInfo(profile));
  if (status != MagickFalse)
    {
      if (LocaleCompare(name,"8bim") == 0)
        GetProfilesFromResourceBlock(image,profile,exception);
      else
        if (recursive == MagickFalse)
          WriteTo8BimProfile(image,name,profile);
    }
  /*
    Inject profile into image properties.
  */
  (void) FormatLocaleString(property,MagickPathExtent,"%s:*",name);
  (void) GetImageProperty(image,property,exception);
  return(status);
}

MagickExport MagickBooleanType SetImageProfile(Image *image,const char *name,
  const StringInfo *profile,ExceptionInfo *exception)
{
  return(SetImageProfileInternal(image,name,profile,MagickFalse,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S y n c I m a g e P r o f i l e s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncImageProfiles() synchronizes image properties with the image profiles.
%  Currently we only support updating the EXIF resolution and orientation.
%
%  The format of the SyncImageProfiles method is:
%
%      MagickBooleanType SyncImageProfiles(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/

static inline int ReadProfileByte(unsigned char **p,size_t *length)
{
  int
    c;

  if (*length < 1)
    return(EOF);
  c=(int) (*(*p)++);
  (*length)--;
  return(c);
}

static inline signed short ReadProfileShort(const EndianType endian,
  unsigned char *buffer)
{
  union
  {
    unsigned int
      unsigned_value;

    signed int
      signed_value;
  } quantum;

  unsigned short
    value;

  if (endian == LSBEndian)
    {
      value=(unsigned short) buffer[1] << 8;
      value|=(unsigned short) buffer[0];
      quantum.unsigned_value=value & 0xffff;
      return(quantum.signed_value);
    }
  value=(unsigned short) buffer[0] << 8;
  value|=(unsigned short) buffer[1];
  quantum.unsigned_value=value & 0xffff;
  return(quantum.signed_value);
}

static inline signed int ReadProfileLong(const EndianType endian,
  unsigned char *buffer)
{
  union
  {
    unsigned int
      unsigned_value;

    signed int
      signed_value;
  } quantum;

  unsigned int
    value;

  if (endian == LSBEndian)
    {
      value=(unsigned int) buffer[3] << 24;
      value|=(unsigned int) buffer[2] << 16;
      value|=(unsigned int) buffer[1] << 8;
      value|=(unsigned int) buffer[0];
      quantum.unsigned_value=value & 0xffffffff;
      return(quantum.signed_value);
    }
  value=(unsigned int) buffer[0] << 24;
  value|=(unsigned int) buffer[1] << 16;
  value|=(unsigned int) buffer[2] << 8;
  value|=(unsigned int) buffer[3];
  quantum.unsigned_value=value & 0xffffffff;
  return(quantum.signed_value);
}

static inline signed int ReadProfileMSBLong(unsigned char **p,size_t *length)
{
  signed int
    value;

  if (*length < 4)
    return(0);
  value=ReadProfileLong(MSBEndian,*p);
  (*length)-=4;
  *p+=4;
  return(value);
}

static inline signed short ReadProfileMSBShort(unsigned char **p,
  size_t *length)
{
  signed short
    value;

  if (*length < 2)
    return(0);
  value=ReadProfileShort(MSBEndian,*p);
  (*length)-=2;
  *p+=2;
  return(value);
}

static inline void WriteProfileLong(const EndianType endian,
  const size_t value,unsigned char *p)
{
  unsigned char
    buffer[4];

  if (endian == LSBEndian)
    {
      buffer[0]=(unsigned char) value;
      buffer[1]=(unsigned char) (value >> 8);
      buffer[2]=(unsigned char) (value >> 16);
      buffer[3]=(unsigned char) (value >> 24);
      (void) memcpy(p,buffer,4);
      return;
    }
  buffer[0]=(unsigned char) (value >> 24);
  buffer[1]=(unsigned char) (value >> 16);
  buffer[2]=(unsigned char) (value >> 8);
  buffer[3]=(unsigned char) value;
  (void) memcpy(p,buffer,4);
}

static void WriteProfileShort(const EndianType endian,
  const unsigned short value,unsigned char *p)
{
  unsigned char
    buffer[2];

  if (endian == LSBEndian)
    {
      buffer[0]=(unsigned char) value;
      buffer[1]=(unsigned char) (value >> 8);
      (void) memcpy(p,buffer,2);
      return;
    }
  buffer[0]=(unsigned char) (value >> 8);
  buffer[1]=(unsigned char) value;
  (void) memcpy(p,buffer,2);
}

static MagickBooleanType Sync8BimProfile(Image *image,StringInfo *profile)
{
  size_t
    length;

  ssize_t
    count;

  unsigned char
    *p;

  unsigned short
    id;

  length=GetStringInfoLength(profile);
  p=GetStringInfoDatum(profile);
  while (length != 0)
  {
    if (ReadProfileByte(&p,&length) != 0x38)
      continue;
    if (ReadProfileByte(&p,&length) != 0x42)
      continue;
    if (ReadProfileByte(&p,&length) != 0x49)
      continue;
    if (ReadProfileByte(&p,&length) != 0x4D)
      continue;
    if (length < 7)
      return(MagickFalse);
    id=ReadProfileMSBShort(&p,&length);
    count=(ssize_t) ReadProfileByte(&p,&length);
    if ((count >= (ssize_t) length) || (count < 0))
      return(MagickFalse);
    p+=count;
    length-=count;
    if ((*p & 0x01) == 0)
      (void) ReadProfileByte(&p,&length);
    count=(ssize_t) ReadProfileMSBLong(&p,&length);
    if ((count > (ssize_t) length) || (count < 0))
      return(MagickFalse);
    if ((id == 0x3ED) && (count == 16))
      {
        if (image->units == PixelsPerCentimeterResolution)
          WriteProfileLong(MSBEndian,(unsigned int) (image->resolution.x*2.54*
            65536.0),p);
        else
          WriteProfileLong(MSBEndian,(unsigned int) (image->resolution.x*
            65536.0),p);
        WriteProfileShort(MSBEndian,(unsigned short) image->units,p+4);
        if (image->units == PixelsPerCentimeterResolution)
          WriteProfileLong(MSBEndian,(unsigned int) (image->resolution.y*2.54*
            65536.0),p+8);
        else
          WriteProfileLong(MSBEndian,(unsigned int) (image->resolution.y*
            65536.0),p+8);
        WriteProfileShort(MSBEndian,(unsigned short) image->units,p+12);
      }
    p+=count;
    length-=count;
  }
  return(MagickTrue);
}

MagickBooleanType SyncExifProfile(Image *image,StringInfo *profile)
{
#define MaxDirectoryStack  16
#define EXIF_DELIMITER  "\n"
#define EXIF_NUM_FORMATS  12
#define TAG_EXIF_OFFSET  0x8769
#define TAG_INTEROP_OFFSET  0xa005

  typedef struct _DirectoryInfo
  {
    unsigned char
      *directory;

    size_t
      entry;
  } DirectoryInfo;

  DirectoryInfo
    directory_stack[MaxDirectoryStack];

  EndianType
    endian;

  size_t
    entry,
    length,
    number_entries;

  SplayTreeInfo
    *exif_resources;

  ssize_t
    id,
    level,
    offset;

  static int
    format_bytes[] = {0, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8};

  unsigned char
    *directory,
    *exif;

  /*
    Set EXIF resolution tag.
  */
  length=GetStringInfoLength(profile);
  exif=GetStringInfoDatum(profile);
  if (length < 16)
    return(MagickFalse);
  id=(ssize_t) ReadProfileShort(LSBEndian,exif);
  if ((id != 0x4949) && (id != 0x4D4D))
    {
      while (length != 0)
      {
        if (ReadProfileByte(&exif,&length) != 0x45)
          continue;
        if (ReadProfileByte(&exif,&length) != 0x78)
          continue;
        if (ReadProfileByte(&exif,&length) != 0x69)
          continue;
        if (ReadProfileByte(&exif,&length) != 0x66)
          continue;
        if (ReadProfileByte(&exif,&length) != 0x00)
          continue;
        if (ReadProfileByte(&exif,&length) != 0x00)
          continue;
        break;
      }
      if (length < 16)
        return(MagickFalse);
      id=(ssize_t) ReadProfileShort(LSBEndian,exif);
    }
  endian=LSBEndian;
  if (id == 0x4949)
    endian=LSBEndian;
  else
    if (id == 0x4D4D)
      endian=MSBEndian;
    else
      return(MagickFalse);
  if (ReadProfileShort(endian,exif+2) != 0x002a)
    return(MagickFalse);
  /*
    This the offset to the first IFD.
  */
  offset=(ssize_t) ReadProfileLong(endian,exif+4);
  if ((offset < 0) || ((size_t) offset >= length))
    return(MagickFalse);
  directory=exif+offset;
  level=0;
  entry=0;
  exif_resources=NewSplayTree((int (*)(const void *,const void *)) NULL,
    (void *(*)(void *)) NULL,(void *(*)(void *)) NULL);
  do
  {
    if (level > 0)
      {
        level--;
        directory=directory_stack[level].directory;
        entry=directory_stack[level].entry;
      }
    if ((directory < exif) || (directory > (exif+length-2)))
      break;
    /*
      Determine how many entries there are in the current IFD.
    */
    number_entries=ReadProfileShort(endian,directory);
    for ( ; entry < number_entries; entry++)
    {
      int
        components;

      register unsigned char
        *p,
        *q;

      size_t
        number_bytes;

      ssize_t
        format,
        tag_value;

      q=(unsigned char *) (directory+2+(12*entry));
      if (q > (exif+length-12))
        break;  /* corrupt EXIF */
      if (GetValueFromSplayTree(exif_resources,q) == q)
        break;
      (void) AddValueToSplayTree(exif_resources,q,q);
      tag_value=(ssize_t) ReadProfileShort(endian,q);
      format=(ssize_t) ReadProfileShort(endian,q+2);
      if ((format < 0) || ((format-1) >= EXIF_NUM_FORMATS))
        break;
      components=(int) ReadProfileLong(endian,q+4);
      if (components < 0)
        break;  /* corrupt EXIF */
      number_bytes=(size_t) components*format_bytes[format];
      if ((ssize_t) number_bytes < components)
        break;  /* prevent overflow */
      if (number_bytes <= 4)
        p=q+8;
      else
        {
          /*
            The directory entry contains an offset.
          */
          offset=(ssize_t) ReadProfileLong(endian,q+8);
          if ((offset < 0) || ((size_t) (offset+number_bytes) > length))
            continue;
          if (~length < number_bytes)
            continue;  /* prevent overflow */
          p=(unsigned char *) (exif+offset);
        }
      switch (tag_value)
      {
        case 0x011a:
        {
          (void) WriteProfileLong(endian,(size_t) (image->resolution.x+0.5),p);
          if (number_bytes == 8)
            (void) WriteProfileLong(endian,1UL,p+4);
          break;
        }
        case 0x011b:
        {
          (void) WriteProfileLong(endian,(size_t) (image->resolution.y+0.5),p);
          if (number_bytes == 8)
            (void) WriteProfileLong(endian,1UL,p+4);
          break;
        }
        case 0x0112:
        {
          if (number_bytes == 4)
            {
              (void) WriteProfileLong(endian,(size_t) image->orientation,p);
              break;
            }
          (void) WriteProfileShort(endian,(unsigned short) image->orientation,
            p);
          break;
        }
        case 0x0128:
        {
          if (number_bytes == 4)
            {
              (void) WriteProfileLong(endian,(size_t) (image->units+1),p);
              break;
            }
          (void) WriteProfileShort(endian,(unsigned short) (image->units+1),p);
          break;
        }
        default:
          break;
      }
      if ((tag_value == TAG_EXIF_OFFSET) || (tag_value == TAG_INTEROP_OFFSET))
        {
          offset=(ssize_t) ReadProfileLong(endian,p);
          if (((size_t) offset < length) && (level < (MaxDirectoryStack-2)))
            {
              directory_stack[level].directory=directory;
              entry++;
              directory_stack[level].entry=entry;
              level++;
              directory_stack[level].directory=exif+offset;
              directory_stack[level].entry=0;
              level++;
              if ((directory+2+(12*number_entries)) > (exif+length))
                break;
              offset=(ssize_t) ReadProfileLong(endian,directory+2+(12*
                number_entries));
              if ((offset != 0) && ((size_t) offset < length) &&
                  (level < (MaxDirectoryStack-2)))
                {
                  directory_stack[level].directory=exif+offset;
                  directory_stack[level].entry=0;
                  level++;
                }
            }
          break;
        }
    }
  } while (level > 0);
  exif_resources=DestroySplayTree(exif_resources);
  return(MagickTrue);
}

MagickPrivate MagickBooleanType SyncImageProfiles(Image *image)
{
  MagickBooleanType
    status;

  StringInfo
    *profile;

  status=MagickTrue;
  profile=(StringInfo *) GetImageProfile(image,"8BIM");
  if (profile != (StringInfo *) NULL)
    if (Sync8BimProfile(image,profile) == MagickFalse)
      status=MagickFalse;
  profile=(StringInfo *) GetImageProfile(image,"EXIF");
  if (profile != (StringInfo *) NULL)
    if (SyncExifProfile(image,profile) == MagickFalse)
      status=MagickFalse;
  return(status);
}

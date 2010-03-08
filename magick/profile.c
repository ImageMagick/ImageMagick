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
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
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
#include "magick/studio.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/configure.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/image.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantum.h"
#include "magick/quantum-private.h"
#include "magick/splay-tree.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/token.h"
#include "magick/utility.h"
#if defined(MAGICKCORE_LCMS_DELEGATE)
#if defined(MAGICKCORE_HAVE_LCMS_LCMS_H)
#include <lcms/lcms.h>
#else
#include "lcms.h"
#endif
#endif

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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(clone_image != (const Image *) NULL);
  assert(clone_image->signature == MagickSignature);
  image->color_profile.length=clone_image->color_profile.length;
  image->color_profile.info=clone_image->color_profile.info;
  image->iptc_profile.length=clone_image->iptc_profile.length;
  image->iptc_profile.info=clone_image->iptc_profile.info;
  if (clone_image->profiles != (void *) NULL)
    image->profiles=CloneSplayTree((SplayTreeInfo *) clone_image->profiles,
      (void *(*)(void *)) ConstantString,(void *(*)(void *)) CloneStringInfo);
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->profiles == (SplayTreeInfo *) NULL)
    return(MagickFalse);
  if (LocaleCompare(name,"icc") == 0)
    {
      /*
        Continue to support deprecated color profile for now.
      */
      image->color_profile.length=0;
      image->color_profile.info=(unsigned char *) NULL;
    }
  if (LocaleCompare(name,"iptc") == 0)
    {
      /*
        Continue to support deprecated IPTC profile for now.
      */
      image->iptc_profile.length=0;
      image->iptc_profile.info=(unsigned char *) NULL;
    }
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
  char
    key[MaxTextExtent];

  const StringInfo
    *profile;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->profiles == (SplayTreeInfo *) NULL)
    return((StringInfo *) NULL);
  (void) CopyMagickString(key,name,MaxTextExtent);
  profile=(const StringInfo *) GetValueFromSplayTree((SplayTreeInfo *)
    image->profiles,key);
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
  assert(image->signature == MagickSignature);
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

static unsigned short **DestroyPixelThreadSet(unsigned short **pixels)
{
  register long
    i;

  assert(pixels != (unsigned short **) NULL);
  for (i=0; i < (long) GetOpenMPMaximumThreads(); i++)
    if (pixels[i] != (unsigned short *) NULL)
      pixels[i]=(unsigned short *) RelinquishMagickMemory(pixels[i]);
  pixels=(unsigned short **) RelinquishAlignedMemory(pixels);
  return(pixels);
}

static unsigned short **AcquirePixelThreadSet(const size_t columns,
  const size_t channels)
{
  register long
    i;

  unsigned short
    **pixels;

  unsigned long
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  pixels=(unsigned short **) AcquireAlignedMemory(number_threads,
    sizeof(*pixels));
  if (pixels == (unsigned short **) NULL)
    return((unsigned short **) NULL);
  (void) ResetMagickMemory(pixels,0,number_threads*sizeof(*pixels));
  for (i=0; i < (long) number_threads; i++)
  {
    pixels[i]=(unsigned short *) AcquireQuantumMemory(columns,channels*
      sizeof(**pixels));
    if (pixels[i] == (unsigned short *) NULL)
      return(DestroyPixelThreadSet(pixels));
  }
  return(pixels);
}

static cmsHTRANSFORM *DestroyTransformThreadSet(cmsHTRANSFORM *transform)
{
  register long
    i;

  assert(transform != (cmsHTRANSFORM *) NULL);
  for (i=0; i < (long) GetOpenMPMaximumThreads(); i++)
    if (transform[i] != (cmsHTRANSFORM) NULL)
      cmsDeleteTransform(transform[i]);
  transform=(cmsHTRANSFORM *) RelinquishAlignedMemory(transform);
  return(transform);
}

static cmsHTRANSFORM *AcquireTransformThreadSet(
  const cmsHPROFILE source_profile,const DWORD source_type,
  const cmsHPROFILE target_profile,const DWORD target_type,const int intent,
  const DWORD flags)
{
  cmsHTRANSFORM
    *transform;

  register long
    i;

  unsigned long
    number_threads;

  number_threads=GetOpenMPMaximumThreads();
  transform=(cmsHTRANSFORM *) AcquireAlignedMemory(number_threads,
    sizeof(*transform));
  if (transform == (cmsHTRANSFORM *) NULL)
    return((cmsHTRANSFORM *) NULL);
  (void) ResetMagickMemory(transform,0,number_threads*sizeof(*transform));
  for (i=0; i < (long) number_threads; i++)
  {
    transform[i]=cmsCreateTransform(source_profile,source_type,target_profile,
      target_type,intent,flags);
    if (transform[i] == (cmsHTRANSFORM) NULL)
      return(DestroyTransformThreadSet(transform));
  }
  return(transform);
}
#endif

static MagickBooleanType SetAdobeRGB1998ImageProfile(Image *image)
{
  static unsigned char
    AdobeRGB1998Profile[] =
    {
      0x00, 0x00, 0x02, 0x30, 0x41, 0x44, 0x42, 0x45, 0x02, 0x10, 0x00,
      0x00, 0x6d, 0x6e, 0x74, 0x72, 0x52, 0x47, 0x42, 0x20, 0x58, 0x59,
      0x5a, 0x20, 0x07, 0xd0, 0x00, 0x08, 0x00, 0x0b, 0x00, 0x13, 0x00,
      0x33, 0x00, 0x3b, 0x61, 0x63, 0x73, 0x70, 0x41, 0x50, 0x50, 0x4c,
      0x00, 0x00, 0x00, 0x00, 0x6e, 0x6f, 0x6e, 0x65, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0xf6, 0xd6, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x00, 0xd3, 0x2d, 0x41, 0x44, 0x42, 0x45, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a,
      0x63, 0x70, 0x72, 0x74, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00,
      0x32, 0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x01, 0x30, 0x00, 0x00,
      0x00, 0x6b, 0x77, 0x74, 0x70, 0x74, 0x00, 0x00, 0x01, 0x9c, 0x00,
      0x00, 0x00, 0x14, 0x62, 0x6b, 0x70, 0x74, 0x00, 0x00, 0x01, 0xb0,
      0x00, 0x00, 0x00, 0x14, 0x72, 0x54, 0x52, 0x43, 0x00, 0x00, 0x01,
      0xc4, 0x00, 0x00, 0x00, 0x0e, 0x67, 0x54, 0x52, 0x43, 0x00, 0x00,
      0x01, 0xd4, 0x00, 0x00, 0x00, 0x0e, 0x62, 0x54, 0x52, 0x43, 0x00,
      0x00, 0x01, 0xe4, 0x00, 0x00, 0x00, 0x0e, 0x72, 0x58, 0x59, 0x5a,
      0x00, 0x00, 0x01, 0xf4, 0x00, 0x00, 0x00, 0x14, 0x67, 0x58, 0x59,
      0x5a, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x00, 0x14, 0x62, 0x58,
      0x59, 0x5a, 0x00, 0x00, 0x02, 0x1c, 0x00, 0x00, 0x00, 0x14, 0x74,
      0x65, 0x78, 0x74, 0x00, 0x00, 0x00, 0x00, 0x43, 0x6f, 0x70, 0x79,
      0x72, 0x69, 0x67, 0x68, 0x74, 0x20, 0x32, 0x30, 0x30, 0x30, 0x20,
      0x41, 0x64, 0x6f, 0x62, 0x65, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65,
      0x6d, 0x73, 0x20, 0x49, 0x6e, 0x63, 0x6f, 0x72, 0x70, 0x6f, 0x72,
      0x61, 0x74, 0x65, 0x64, 0x00, 0x00, 0x00, 0x64, 0x65, 0x73, 0x63,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x41, 0x64, 0x6f,
      0x62, 0x65, 0x20, 0x52, 0x47, 0x42, 0x20, 0x28, 0x31, 0x39, 0x39,
      0x38, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0xf3, 0x51, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x01, 0x16, 0xcc, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x02, 0x33, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x33, 0x00, 0x00,
      0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x01, 0x02, 0x33, 0x00, 0x00, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x9c, 0x18, 0x00, 0x00, 0x4f, 0xa5, 0x00,
      0x00, 0x04, 0xfc, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x34, 0x8d, 0x00, 0x00, 0xa0, 0x2c, 0x00, 0x00, 0x0f,
      0x95, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x26, 0x31, 0x00, 0x00, 0x10, 0x2f, 0x00, 0x00, 0xbe, 0x9c
    };

  StringInfo
    *profile;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (GetImageProfile(image,"icm") != (const StringInfo *) NULL)
    return(MagickFalse);
  profile=AcquireStringInfo(sizeof(AdobeRGB1998Profile));
  SetStringInfoDatum(profile,AdobeRGB1998Profile);
  status=SetImageProfile(image,"icm",profile);
  profile=DestroyStringInfo(profile);
  return(status);
}

static MagickBooleanType SetsRGBImageProfile(Image *image)
{
  static unsigned char
    sRGBProfile[] =
    {
      0x00, 0x00, 0x0c, 0x48, 0x4c, 0x69, 0x6e, 0x6f, 0x02, 0x10, 0x00,
      0x00, 0x6d, 0x6e, 0x74, 0x72, 0x52, 0x47, 0x42, 0x20, 0x58, 0x59,
      0x5a, 0x20, 0x07, 0xce, 0x00, 0x02, 0x00, 0x09, 0x00, 0x06, 0x00,
      0x31, 0x00, 0x00, 0x61, 0x63, 0x73, 0x70, 0x4d, 0x53, 0x46, 0x54,
      0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x43, 0x20, 0x73, 0x52, 0x47,
      0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0xf6, 0xd6, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x00, 0xd3, 0x2d, 0x48, 0x50, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11,
      0x63, 0x70, 0x72, 0x74, 0x00, 0x00, 0x01, 0x50, 0x00, 0x00, 0x00,
      0x33, 0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x01, 0x84, 0x00, 0x00,
      0x00, 0x6c, 0x77, 0x74, 0x70, 0x74, 0x00, 0x00, 0x01, 0xf0, 0x00,
      0x00, 0x00, 0x14, 0x62, 0x6b, 0x70, 0x74, 0x00, 0x00, 0x02, 0x04,
      0x00, 0x00, 0x00, 0x14, 0x72, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x02,
      0x18, 0x00, 0x00, 0x00, 0x14, 0x67, 0x58, 0x59, 0x5a, 0x00, 0x00,
      0x02, 0x2c, 0x00, 0x00, 0x00, 0x14, 0x62, 0x58, 0x59, 0x5a, 0x00,
      0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x14, 0x64, 0x6d, 0x6e, 0x64,
      0x00, 0x00, 0x02, 0x54, 0x00, 0x00, 0x00, 0x70, 0x64, 0x6d, 0x64,
      0x64, 0x00, 0x00, 0x02, 0xc4, 0x00, 0x00, 0x00, 0x88, 0x76, 0x75,
      0x65, 0x64, 0x00, 0x00, 0x03, 0x4c, 0x00, 0x00, 0x00, 0x86, 0x76,
      0x69, 0x65, 0x77, 0x00, 0x00, 0x03, 0xd4, 0x00, 0x00, 0x00, 0x24,
      0x6c, 0x75, 0x6d, 0x69, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00,
      0x14, 0x6d, 0x65, 0x61, 0x73, 0x00, 0x00, 0x04, 0x0c, 0x00, 0x00,
      0x00, 0x24, 0x74, 0x65, 0x63, 0x68, 0x00, 0x00, 0x04, 0x30, 0x00,
      0x00, 0x00, 0x0c, 0x72, 0x54, 0x52, 0x43, 0x00, 0x00, 0x04, 0x3c,
      0x00, 0x00, 0x08, 0x0c, 0x67, 0x54, 0x52, 0x43, 0x00, 0x00, 0x04,
      0x3c, 0x00, 0x00, 0x08, 0x0c, 0x62, 0x54, 0x52, 0x43, 0x00, 0x00,
      0x04, 0x3c, 0x00, 0x00, 0x08, 0x0c, 0x74, 0x65, 0x78, 0x74, 0x00,
      0x00, 0x00, 0x00, 0x43, 0x6f, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68,
      0x74, 0x20, 0x28, 0x63, 0x29, 0x20, 0x31, 0x39, 0x39, 0x38, 0x20,
      0x48, 0x65, 0x77, 0x6c, 0x65, 0x74, 0x74, 0x2d, 0x50, 0x61, 0x63,
      0x6b, 0x61, 0x72, 0x64, 0x20, 0x43, 0x6f, 0x6d, 0x70, 0x61, 0x6e,
      0x79, 0x00, 0x00, 0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x12, 0x73, 0x52, 0x47, 0x42, 0x20, 0x49, 0x45,
      0x43, 0x36, 0x31, 0x39, 0x36, 0x36, 0x2d, 0x32, 0x2e, 0x31, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12,
      0x73, 0x52, 0x47, 0x42, 0x20, 0x49, 0x45, 0x43, 0x36, 0x31, 0x39,
      0x36, 0x36, 0x2d, 0x32, 0x2e, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0xf3, 0x51, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x16, 0xcc, 0x58,
      0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x59, 0x5a,
      0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6f, 0xa2, 0x00, 0x00,
      0x38, 0xf5, 0x00, 0x00, 0x03, 0x90, 0x58, 0x59, 0x5a, 0x20, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x99, 0x00, 0x00, 0xb7, 0x85,
      0x00, 0x00, 0x18, 0xda, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x24, 0xa0, 0x00, 0x00, 0x0f, 0x84, 0x00, 0x00,
      0xb6, 0xcf, 0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x16, 0x49, 0x45, 0x43, 0x20, 0x68, 0x74, 0x74, 0x70,
      0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x65, 0x63, 0x2e,
      0x63, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x16, 0x49, 0x45, 0x43, 0x20, 0x68, 0x74, 0x74, 0x70,
      0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x65, 0x63, 0x2e,
      0x63, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x2e, 0x49, 0x45, 0x43, 0x20, 0x36, 0x31,
      0x39, 0x36, 0x36, 0x2d, 0x32, 0x2e, 0x31, 0x20, 0x44, 0x65, 0x66,
      0x61, 0x75, 0x6c, 0x74, 0x20, 0x52, 0x47, 0x42, 0x20, 0x63, 0x6f,
      0x6c, 0x6f, 0x75, 0x72, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x20,
      0x2d, 0x20, 0x73, 0x52, 0x47, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x49, 0x45, 0x43, 0x20,
      0x36, 0x31, 0x39, 0x36, 0x36, 0x2d, 0x32, 0x2e, 0x31, 0x20, 0x44,
      0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, 0x20, 0x52, 0x47, 0x42, 0x20,
      0x63, 0x6f, 0x6c, 0x6f, 0x75, 0x72, 0x20, 0x73, 0x70, 0x61, 0x63,
      0x65, 0x20, 0x2d, 0x20, 0x73, 0x52, 0x47, 0x42, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x65, 0x73,
      0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x52, 0x65,
      0x66, 0x65, 0x72, 0x65, 0x6e, 0x63, 0x65, 0x20, 0x56, 0x69, 0x65,
      0x77, 0x69, 0x6e, 0x67, 0x20, 0x43, 0x6f, 0x6e, 0x64, 0x69, 0x74,
      0x69, 0x6f, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x49, 0x45, 0x43, 0x36,
      0x31, 0x39, 0x36, 0x36, 0x2d, 0x32, 0x2e, 0x31, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x52, 0x65,
      0x66, 0x65, 0x72, 0x65, 0x6e, 0x63, 0x65, 0x20, 0x56, 0x69, 0x65,
      0x77, 0x69, 0x6e, 0x67, 0x20, 0x43, 0x6f, 0x6e, 0x64, 0x69, 0x74,
      0x69, 0x6f, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x49, 0x45, 0x43, 0x36,
      0x31, 0x39, 0x36, 0x36, 0x2d, 0x32, 0x2e, 0x31, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x76, 0x69, 0x65, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13,
      0xa4, 0xfe, 0x00, 0x14, 0x5f, 0x2e, 0x00, 0x10, 0xcf, 0x14, 0x00,
      0x03, 0xed, 0xcc, 0x00, 0x04, 0x13, 0x0b, 0x00, 0x03, 0x5c, 0x9e,
      0x00, 0x00, 0x00, 0x01, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x4c, 0x09, 0x56, 0x00, 0x50, 0x00, 0x00, 0x00, 0x57,
      0x1f, 0xe7, 0x6d, 0x65, 0x61, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
      0x8f, 0x00, 0x00, 0x00, 0x02, 0x73, 0x69, 0x67, 0x20, 0x00, 0x00,
      0x00, 0x00, 0x43, 0x52, 0x54, 0x20, 0x63, 0x75, 0x72, 0x76, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x05,
      0x00, 0x0a, 0x00, 0x0f, 0x00, 0x14, 0x00, 0x19, 0x00, 0x1e, 0x00,
      0x23, 0x00, 0x28, 0x00, 0x2d, 0x00, 0x32, 0x00, 0x37, 0x00, 0x3b,
      0x00, 0x40, 0x00, 0x45, 0x00, 0x4a, 0x00, 0x4f, 0x00, 0x54, 0x00,
      0x59, 0x00, 0x5e, 0x00, 0x63, 0x00, 0x68, 0x00, 0x6d, 0x00, 0x72,
      0x00, 0x77, 0x00, 0x7c, 0x00, 0x81, 0x00, 0x86, 0x00, 0x8b, 0x00,
      0x90, 0x00, 0x95, 0x00, 0x9a, 0x00, 0x9f, 0x00, 0xa4, 0x00, 0xa9,
      0x00, 0xae, 0x00, 0xb2, 0x00, 0xb7, 0x00, 0xbc, 0x00, 0xc1, 0x00,
      0xc6, 0x00, 0xcb, 0x00, 0xd0, 0x00, 0xd5, 0x00, 0xdb, 0x00, 0xe0,
      0x00, 0xe5, 0x00, 0xeb, 0x00, 0xf0, 0x00, 0xf6, 0x00, 0xfb, 0x01,
      0x01, 0x01, 0x07, 0x01, 0x0d, 0x01, 0x13, 0x01, 0x19, 0x01, 0x1f,
      0x01, 0x25, 0x01, 0x2b, 0x01, 0x32, 0x01, 0x38, 0x01, 0x3e, 0x01,
      0x45, 0x01, 0x4c, 0x01, 0x52, 0x01, 0x59, 0x01, 0x60, 0x01, 0x67,
      0x01, 0x6e, 0x01, 0x75, 0x01, 0x7c, 0x01, 0x83, 0x01, 0x8b, 0x01,
      0x92, 0x01, 0x9a, 0x01, 0xa1, 0x01, 0xa9, 0x01, 0xb1, 0x01, 0xb9,
      0x01, 0xc1, 0x01, 0xc9, 0x01, 0xd1, 0x01, 0xd9, 0x01, 0xe1, 0x01,
      0xe9, 0x01, 0xf2, 0x01, 0xfa, 0x02, 0x03, 0x02, 0x0c, 0x02, 0x14,
      0x02, 0x1d, 0x02, 0x26, 0x02, 0x2f, 0x02, 0x38, 0x02, 0x41, 0x02,
      0x4b, 0x02, 0x54, 0x02, 0x5d, 0x02, 0x67, 0x02, 0x71, 0x02, 0x7a,
      0x02, 0x84, 0x02, 0x8e, 0x02, 0x98, 0x02, 0xa2, 0x02, 0xac, 0x02,
      0xb6, 0x02, 0xc1, 0x02, 0xcb, 0x02, 0xd5, 0x02, 0xe0, 0x02, 0xeb,
      0x02, 0xf5, 0x03, 0x00, 0x03, 0x0b, 0x03, 0x16, 0x03, 0x21, 0x03,
      0x2d, 0x03, 0x38, 0x03, 0x43, 0x03, 0x4f, 0x03, 0x5a, 0x03, 0x66,
      0x03, 0x72, 0x03, 0x7e, 0x03, 0x8a, 0x03, 0x96, 0x03, 0xa2, 0x03,
      0xae, 0x03, 0xba, 0x03, 0xc7, 0x03, 0xd3, 0x03, 0xe0, 0x03, 0xec,
      0x03, 0xf9, 0x04, 0x06, 0x04, 0x13, 0x04, 0x20, 0x04, 0x2d, 0x04,
      0x3b, 0x04, 0x48, 0x04, 0x55, 0x04, 0x63, 0x04, 0x71, 0x04, 0x7e,
      0x04, 0x8c, 0x04, 0x9a, 0x04, 0xa8, 0x04, 0xb6, 0x04, 0xc4, 0x04,
      0xd3, 0x04, 0xe1, 0x04, 0xf0, 0x04, 0xfe, 0x05, 0x0d, 0x05, 0x1c,
      0x05, 0x2b, 0x05, 0x3a, 0x05, 0x49, 0x05, 0x58, 0x05, 0x67, 0x05,
      0x77, 0x05, 0x86, 0x05, 0x96, 0x05, 0xa6, 0x05, 0xb5, 0x05, 0xc5,
      0x05, 0xd5, 0x05, 0xe5, 0x05, 0xf6, 0x06, 0x06, 0x06, 0x16, 0x06,
      0x27, 0x06, 0x37, 0x06, 0x48, 0x06, 0x59, 0x06, 0x6a, 0x06, 0x7b,
      0x06, 0x8c, 0x06, 0x9d, 0x06, 0xaf, 0x06, 0xc0, 0x06, 0xd1, 0x06,
      0xe3, 0x06, 0xf5, 0x07, 0x07, 0x07, 0x19, 0x07, 0x2b, 0x07, 0x3d,
      0x07, 0x4f, 0x07, 0x61, 0x07, 0x74, 0x07, 0x86, 0x07, 0x99, 0x07,
      0xac, 0x07, 0xbf, 0x07, 0xd2, 0x07, 0xe5, 0x07, 0xf8, 0x08, 0x0b,
      0x08, 0x1f, 0x08, 0x32, 0x08, 0x46, 0x08, 0x5a, 0x08, 0x6e, 0x08,
      0x82, 0x08, 0x96, 0x08, 0xaa, 0x08, 0xbe, 0x08, 0xd2, 0x08, 0xe7,
      0x08, 0xfb, 0x09, 0x10, 0x09, 0x25, 0x09, 0x3a, 0x09, 0x4f, 0x09,
      0x64, 0x09, 0x79, 0x09, 0x8f, 0x09, 0xa4, 0x09, 0xba, 0x09, 0xcf,
      0x09, 0xe5, 0x09, 0xfb, 0x0a, 0x11, 0x0a, 0x27, 0x0a, 0x3d, 0x0a,
      0x54, 0x0a, 0x6a, 0x0a, 0x81, 0x0a, 0x98, 0x0a, 0xae, 0x0a, 0xc5,
      0x0a, 0xdc, 0x0a, 0xf3, 0x0b, 0x0b, 0x0b, 0x22, 0x0b, 0x39, 0x0b,
      0x51, 0x0b, 0x69, 0x0b, 0x80, 0x0b, 0x98, 0x0b, 0xb0, 0x0b, 0xc8,
      0x0b, 0xe1, 0x0b, 0xf9, 0x0c, 0x12, 0x0c, 0x2a, 0x0c, 0x43, 0x0c,
      0x5c, 0x0c, 0x75, 0x0c, 0x8e, 0x0c, 0xa7, 0x0c, 0xc0, 0x0c, 0xd9,
      0x0c, 0xf3, 0x0d, 0x0d, 0x0d, 0x26, 0x0d, 0x40, 0x0d, 0x5a, 0x0d,
      0x74, 0x0d, 0x8e, 0x0d, 0xa9, 0x0d, 0xc3, 0x0d, 0xde, 0x0d, 0xf8,
      0x0e, 0x13, 0x0e, 0x2e, 0x0e, 0x49, 0x0e, 0x64, 0x0e, 0x7f, 0x0e,
      0x9b, 0x0e, 0xb6, 0x0e, 0xd2, 0x0e, 0xee, 0x0f, 0x09, 0x0f, 0x25,
      0x0f, 0x41, 0x0f, 0x5e, 0x0f, 0x7a, 0x0f, 0x96, 0x0f, 0xb3, 0x0f,
      0xcf, 0x0f, 0xec, 0x10, 0x09, 0x10, 0x26, 0x10, 0x43, 0x10, 0x61,
      0x10, 0x7e, 0x10, 0x9b, 0x10, 0xb9, 0x10, 0xd7, 0x10, 0xf5, 0x11,
      0x13, 0x11, 0x31, 0x11, 0x4f, 0x11, 0x6d, 0x11, 0x8c, 0x11, 0xaa,
      0x11, 0xc9, 0x11, 0xe8, 0x12, 0x07, 0x12, 0x26, 0x12, 0x45, 0x12,
      0x64, 0x12, 0x84, 0x12, 0xa3, 0x12, 0xc3, 0x12, 0xe3, 0x13, 0x03,
      0x13, 0x23, 0x13, 0x43, 0x13, 0x63, 0x13, 0x83, 0x13, 0xa4, 0x13,
      0xc5, 0x13, 0xe5, 0x14, 0x06, 0x14, 0x27, 0x14, 0x49, 0x14, 0x6a,
      0x14, 0x8b, 0x14, 0xad, 0x14, 0xce, 0x14, 0xf0, 0x15, 0x12, 0x15,
      0x34, 0x15, 0x56, 0x15, 0x78, 0x15, 0x9b, 0x15, 0xbd, 0x15, 0xe0,
      0x16, 0x03, 0x16, 0x26, 0x16, 0x49, 0x16, 0x6c, 0x16, 0x8f, 0x16,
      0xb2, 0x16, 0xd6, 0x16, 0xfa, 0x17, 0x1d, 0x17, 0x41, 0x17, 0x65,
      0x17, 0x89, 0x17, 0xae, 0x17, 0xd2, 0x17, 0xf7, 0x18, 0x1b, 0x18,
      0x40, 0x18, 0x65, 0x18, 0x8a, 0x18, 0xaf, 0x18, 0xd5, 0x18, 0xfa,
      0x19, 0x20, 0x19, 0x45, 0x19, 0x6b, 0x19, 0x91, 0x19, 0xb7, 0x19,
      0xdd, 0x1a, 0x04, 0x1a, 0x2a, 0x1a, 0x51, 0x1a, 0x77, 0x1a, 0x9e,
      0x1a, 0xc5, 0x1a, 0xec, 0x1b, 0x14, 0x1b, 0x3b, 0x1b, 0x63, 0x1b,
      0x8a, 0x1b, 0xb2, 0x1b, 0xda, 0x1c, 0x02, 0x1c, 0x2a, 0x1c, 0x52,
      0x1c, 0x7b, 0x1c, 0xa3, 0x1c, 0xcc, 0x1c, 0xf5, 0x1d, 0x1e, 0x1d,
      0x47, 0x1d, 0x70, 0x1d, 0x99, 0x1d, 0xc3, 0x1d, 0xec, 0x1e, 0x16,
      0x1e, 0x40, 0x1e, 0x6a, 0x1e, 0x94, 0x1e, 0xbe, 0x1e, 0xe9, 0x1f,
      0x13, 0x1f, 0x3e, 0x1f, 0x69, 0x1f, 0x94, 0x1f, 0xbf, 0x1f, 0xea,
      0x20, 0x15, 0x20, 0x41, 0x20, 0x6c, 0x20, 0x98, 0x20, 0xc4, 0x20,
      0xf0, 0x21, 0x1c, 0x21, 0x48, 0x21, 0x75, 0x21, 0xa1, 0x21, 0xce,
      0x21, 0xfb, 0x22, 0x27, 0x22, 0x55, 0x22, 0x82, 0x22, 0xaf, 0x22,
      0xdd, 0x23, 0x0a, 0x23, 0x38, 0x23, 0x66, 0x23, 0x94, 0x23, 0xc2,
      0x23, 0xf0, 0x24, 0x1f, 0x24, 0x4d, 0x24, 0x7c, 0x24, 0xab, 0x24,
      0xda, 0x25, 0x09, 0x25, 0x38, 0x25, 0x68, 0x25, 0x97, 0x25, 0xc7,
      0x25, 0xf7, 0x26, 0x27, 0x26, 0x57, 0x26, 0x87, 0x26, 0xb7, 0x26,
      0xe8, 0x27, 0x18, 0x27, 0x49, 0x27, 0x7a, 0x27, 0xab, 0x27, 0xdc,
      0x28, 0x0d, 0x28, 0x3f, 0x28, 0x71, 0x28, 0xa2, 0x28, 0xd4, 0x29,
      0x06, 0x29, 0x38, 0x29, 0x6b, 0x29, 0x9d, 0x29, 0xd0, 0x2a, 0x02,
      0x2a, 0x35, 0x2a, 0x68, 0x2a, 0x9b, 0x2a, 0xcf, 0x2b, 0x02, 0x2b,
      0x36, 0x2b, 0x69, 0x2b, 0x9d, 0x2b, 0xd1, 0x2c, 0x05, 0x2c, 0x39,
      0x2c, 0x6e, 0x2c, 0xa2, 0x2c, 0xd7, 0x2d, 0x0c, 0x2d, 0x41, 0x2d,
      0x76, 0x2d, 0xab, 0x2d, 0xe1, 0x2e, 0x16, 0x2e, 0x4c, 0x2e, 0x82,
      0x2e, 0xb7, 0x2e, 0xee, 0x2f, 0x24, 0x2f, 0x5a, 0x2f, 0x91, 0x2f,
      0xc7, 0x2f, 0xfe, 0x30, 0x35, 0x30, 0x6c, 0x30, 0xa4, 0x30, 0xdb,
      0x31, 0x12, 0x31, 0x4a, 0x31, 0x82, 0x31, 0xba, 0x31, 0xf2, 0x32,
      0x2a, 0x32, 0x63, 0x32, 0x9b, 0x32, 0xd4, 0x33, 0x0d, 0x33, 0x46,
      0x33, 0x7f, 0x33, 0xb8, 0x33, 0xf1, 0x34, 0x2b, 0x34, 0x65, 0x34,
      0x9e, 0x34, 0xd8, 0x35, 0x13, 0x35, 0x4d, 0x35, 0x87, 0x35, 0xc2,
      0x35, 0xfd, 0x36, 0x37, 0x36, 0x72, 0x36, 0xae, 0x36, 0xe9, 0x37,
      0x24, 0x37, 0x60, 0x37, 0x9c, 0x37, 0xd7, 0x38, 0x14, 0x38, 0x50,
      0x38, 0x8c, 0x38, 0xc8, 0x39, 0x05, 0x39, 0x42, 0x39, 0x7f, 0x39,
      0xbc, 0x39, 0xf9, 0x3a, 0x36, 0x3a, 0x74, 0x3a, 0xb2, 0x3a, 0xef,
      0x3b, 0x2d, 0x3b, 0x6b, 0x3b, 0xaa, 0x3b, 0xe8, 0x3c, 0x27, 0x3c,
      0x65, 0x3c, 0xa4, 0x3c, 0xe3, 0x3d, 0x22, 0x3d, 0x61, 0x3d, 0xa1,
      0x3d, 0xe0, 0x3e, 0x20, 0x3e, 0x60, 0x3e, 0xa0, 0x3e, 0xe0, 0x3f,
      0x21, 0x3f, 0x61, 0x3f, 0xa2, 0x3f, 0xe2, 0x40, 0x23, 0x40, 0x64,
      0x40, 0xa6, 0x40, 0xe7, 0x41, 0x29, 0x41, 0x6a, 0x41, 0xac, 0x41,
      0xee, 0x42, 0x30, 0x42, 0x72, 0x42, 0xb5, 0x42, 0xf7, 0x43, 0x3a,
      0x43, 0x7d, 0x43, 0xc0, 0x44, 0x03, 0x44, 0x47, 0x44, 0x8a, 0x44,
      0xce, 0x45, 0x12, 0x45, 0x55, 0x45, 0x9a, 0x45, 0xde, 0x46, 0x22,
      0x46, 0x67, 0x46, 0xab, 0x46, 0xf0, 0x47, 0x35, 0x47, 0x7b, 0x47,
      0xc0, 0x48, 0x05, 0x48, 0x4b, 0x48, 0x91, 0x48, 0xd7, 0x49, 0x1d,
      0x49, 0x63, 0x49, 0xa9, 0x49, 0xf0, 0x4a, 0x37, 0x4a, 0x7d, 0x4a,
      0xc4, 0x4b, 0x0c, 0x4b, 0x53, 0x4b, 0x9a, 0x4b, 0xe2, 0x4c, 0x2a,
      0x4c, 0x72, 0x4c, 0xba, 0x4d, 0x02, 0x4d, 0x4a, 0x4d, 0x93, 0x4d,
      0xdc, 0x4e, 0x25, 0x4e, 0x6e, 0x4e, 0xb7, 0x4f, 0x00, 0x4f, 0x49,
      0x4f, 0x93, 0x4f, 0xdd, 0x50, 0x27, 0x50, 0x71, 0x50, 0xbb, 0x51,
      0x06, 0x51, 0x50, 0x51, 0x9b, 0x51, 0xe6, 0x52, 0x31, 0x52, 0x7c,
      0x52, 0xc7, 0x53, 0x13, 0x53, 0x5f, 0x53, 0xaa, 0x53, 0xf6, 0x54,
      0x42, 0x54, 0x8f, 0x54, 0xdb, 0x55, 0x28, 0x55, 0x75, 0x55, 0xc2,
      0x56, 0x0f, 0x56, 0x5c, 0x56, 0xa9, 0x56, 0xf7, 0x57, 0x44, 0x57,
      0x92, 0x57, 0xe0, 0x58, 0x2f, 0x58, 0x7d, 0x58, 0xcb, 0x59, 0x1a,
      0x59, 0x69, 0x59, 0xb8, 0x5a, 0x07, 0x5a, 0x56, 0x5a, 0xa6, 0x5a,
      0xf5, 0x5b, 0x45, 0x5b, 0x95, 0x5b, 0xe5, 0x5c, 0x35, 0x5c, 0x86,
      0x5c, 0xd6, 0x5d, 0x27, 0x5d, 0x78, 0x5d, 0xc9, 0x5e, 0x1a, 0x5e,
      0x6c, 0x5e, 0xbd, 0x5f, 0x0f, 0x5f, 0x61, 0x5f, 0xb3, 0x60, 0x05,
      0x60, 0x57, 0x60, 0xaa, 0x60, 0xfc, 0x61, 0x4f, 0x61, 0xa2, 0x61,
      0xf5, 0x62, 0x49, 0x62, 0x9c, 0x62, 0xf0, 0x63, 0x43, 0x63, 0x97,
      0x63, 0xeb, 0x64, 0x40, 0x64, 0x94, 0x64, 0xe9, 0x65, 0x3d, 0x65,
      0x92, 0x65, 0xe7, 0x66, 0x3d, 0x66, 0x92, 0x66, 0xe8, 0x67, 0x3d,
      0x67, 0x93, 0x67, 0xe9, 0x68, 0x3f, 0x68, 0x96, 0x68, 0xec, 0x69,
      0x43, 0x69, 0x9a, 0x69, 0xf1, 0x6a, 0x48, 0x6a, 0x9f, 0x6a, 0xf7,
      0x6b, 0x4f, 0x6b, 0xa7, 0x6b, 0xff, 0x6c, 0x57, 0x6c, 0xaf, 0x6d,
      0x08, 0x6d, 0x60, 0x6d, 0xb9, 0x6e, 0x12, 0x6e, 0x6b, 0x6e, 0xc4,
      0x6f, 0x1e, 0x6f, 0x78, 0x6f, 0xd1, 0x70, 0x2b, 0x70, 0x86, 0x70,
      0xe0, 0x71, 0x3a, 0x71, 0x95, 0x71, 0xf0, 0x72, 0x4b, 0x72, 0xa6,
      0x73, 0x01, 0x73, 0x5d, 0x73, 0xb8, 0x74, 0x14, 0x74, 0x70, 0x74,
      0xcc, 0x75, 0x28, 0x75, 0x85, 0x75, 0xe1, 0x76, 0x3e, 0x76, 0x9b,
      0x76, 0xf8, 0x77, 0x56, 0x77, 0xb3, 0x78, 0x11, 0x78, 0x6e, 0x78,
      0xcc, 0x79, 0x2a, 0x79, 0x89, 0x79, 0xe7, 0x7a, 0x46, 0x7a, 0xa5,
      0x7b, 0x04, 0x7b, 0x63, 0x7b, 0xc2, 0x7c, 0x21, 0x7c, 0x81, 0x7c,
      0xe1, 0x7d, 0x41, 0x7d, 0xa1, 0x7e, 0x01, 0x7e, 0x62, 0x7e, 0xc2,
      0x7f, 0x23, 0x7f, 0x84, 0x7f, 0xe5, 0x80, 0x47, 0x80, 0xa8, 0x81,
      0x0a, 0x81, 0x6b, 0x81, 0xcd, 0x82, 0x30, 0x82, 0x92, 0x82, 0xf4,
      0x83, 0x57, 0x83, 0xba, 0x84, 0x1d, 0x84, 0x80, 0x84, 0xe3, 0x85,
      0x47, 0x85, 0xab, 0x86, 0x0e, 0x86, 0x72, 0x86, 0xd7, 0x87, 0x3b,
      0x87, 0x9f, 0x88, 0x04, 0x88, 0x69, 0x88, 0xce, 0x89, 0x33, 0x89,
      0x99, 0x89, 0xfe, 0x8a, 0x64, 0x8a, 0xca, 0x8b, 0x30, 0x8b, 0x96,
      0x8b, 0xfc, 0x8c, 0x63, 0x8c, 0xca, 0x8d, 0x31, 0x8d, 0x98, 0x8d,
      0xff, 0x8e, 0x66, 0x8e, 0xce, 0x8f, 0x36, 0x8f, 0x9e, 0x90, 0x06,
      0x90, 0x6e, 0x90, 0xd6, 0x91, 0x3f, 0x91, 0xa8, 0x92, 0x11, 0x92,
      0x7a, 0x92, 0xe3, 0x93, 0x4d, 0x93, 0xb6, 0x94, 0x20, 0x94, 0x8a,
      0x94, 0xf4, 0x95, 0x5f, 0x95, 0xc9, 0x96, 0x34, 0x96, 0x9f, 0x97,
      0x0a, 0x97, 0x75, 0x97, 0xe0, 0x98, 0x4c, 0x98, 0xb8, 0x99, 0x24,
      0x99, 0x90, 0x99, 0xfc, 0x9a, 0x68, 0x9a, 0xd5, 0x9b, 0x42, 0x9b,
      0xaf, 0x9c, 0x1c, 0x9c, 0x89, 0x9c, 0xf7, 0x9d, 0x64, 0x9d, 0xd2,
      0x9e, 0x40, 0x9e, 0xae, 0x9f, 0x1d, 0x9f, 0x8b, 0x9f, 0xfa, 0xa0,
      0x69, 0xa0, 0xd8, 0xa1, 0x47, 0xa1, 0xb6, 0xa2, 0x26, 0xa2, 0x96,
      0xa3, 0x06, 0xa3, 0x76, 0xa3, 0xe6, 0xa4, 0x56, 0xa4, 0xc7, 0xa5,
      0x38, 0xa5, 0xa9, 0xa6, 0x1a, 0xa6, 0x8b, 0xa6, 0xfd, 0xa7, 0x6e,
      0xa7, 0xe0, 0xa8, 0x52, 0xa8, 0xc4, 0xa9, 0x37, 0xa9, 0xa9, 0xaa,
      0x1c, 0xaa, 0x8f, 0xab, 0x02, 0xab, 0x75, 0xab, 0xe9, 0xac, 0x5c,
      0xac, 0xd0, 0xad, 0x44, 0xad, 0xb8, 0xae, 0x2d, 0xae, 0xa1, 0xaf,
      0x16, 0xaf, 0x8b, 0xb0, 0x00, 0xb0, 0x75, 0xb0, 0xea, 0xb1, 0x60,
      0xb1, 0xd6, 0xb2, 0x4b, 0xb2, 0xc2, 0xb3, 0x38, 0xb3, 0xae, 0xb4,
      0x25, 0xb4, 0x9c, 0xb5, 0x13, 0xb5, 0x8a, 0xb6, 0x01, 0xb6, 0x79,
      0xb6, 0xf0, 0xb7, 0x68, 0xb7, 0xe0, 0xb8, 0x59, 0xb8, 0xd1, 0xb9,
      0x4a, 0xb9, 0xc2, 0xba, 0x3b, 0xba, 0xb5, 0xbb, 0x2e, 0xbb, 0xa7,
      0xbc, 0x21, 0xbc, 0x9b, 0xbd, 0x15, 0xbd, 0x8f, 0xbe, 0x0a, 0xbe,
      0x84, 0xbe, 0xff, 0xbf, 0x7a, 0xbf, 0xf5, 0xc0, 0x70, 0xc0, 0xec,
      0xc1, 0x67, 0xc1, 0xe3, 0xc2, 0x5f, 0xc2, 0xdb, 0xc3, 0x58, 0xc3,
      0xd4, 0xc4, 0x51, 0xc4, 0xce, 0xc5, 0x4b, 0xc5, 0xc8, 0xc6, 0x46,
      0xc6, 0xc3, 0xc7, 0x41, 0xc7, 0xbf, 0xc8, 0x3d, 0xc8, 0xbc, 0xc9,
      0x3a, 0xc9, 0xb9, 0xca, 0x38, 0xca, 0xb7, 0xcb, 0x36, 0xcb, 0xb6,
      0xcc, 0x35, 0xcc, 0xb5, 0xcd, 0x35, 0xcd, 0xb5, 0xce, 0x36, 0xce,
      0xb6, 0xcf, 0x37, 0xcf, 0xb8, 0xd0, 0x39, 0xd0, 0xba, 0xd1, 0x3c,
      0xd1, 0xbe, 0xd2, 0x3f, 0xd2, 0xc1, 0xd3, 0x44, 0xd3, 0xc6, 0xd4,
      0x49, 0xd4, 0xcb, 0xd5, 0x4e, 0xd5, 0xd1, 0xd6, 0x55, 0xd6, 0xd8,
      0xd7, 0x5c, 0xd7, 0xe0, 0xd8, 0x64, 0xd8, 0xe8, 0xd9, 0x6c, 0xd9,
      0xf1, 0xda, 0x76, 0xda, 0xfb, 0xdb, 0x80, 0xdc, 0x05, 0xdc, 0x8a,
      0xdd, 0x10, 0xdd, 0x96, 0xde, 0x1c, 0xde, 0xa2, 0xdf, 0x29, 0xdf,
      0xaf, 0xe0, 0x36, 0xe0, 0xbd, 0xe1, 0x44, 0xe1, 0xcc, 0xe2, 0x53,
      0xe2, 0xdb, 0xe3, 0x63, 0xe3, 0xeb, 0xe4, 0x73, 0xe4, 0xfc, 0xe5,
      0x84, 0xe6, 0x0d, 0xe6, 0x96, 0xe7, 0x1f, 0xe7, 0xa9, 0xe8, 0x32,
      0xe8, 0xbc, 0xe9, 0x46, 0xe9, 0xd0, 0xea, 0x5b, 0xea, 0xe5, 0xeb,
      0x70, 0xeb, 0xfb, 0xec, 0x86, 0xed, 0x11, 0xed, 0x9c, 0xee, 0x28,
      0xee, 0xb4, 0xef, 0x40, 0xef, 0xcc, 0xf0, 0x58, 0xf0, 0xe5, 0xf1,
      0x72, 0xf1, 0xff, 0xf2, 0x8c, 0xf3, 0x19, 0xf3, 0xa7, 0xf4, 0x34,
      0xf4, 0xc2, 0xf5, 0x50, 0xf5, 0xde, 0xf6, 0x6d, 0xf6, 0xfb, 0xf7,
      0x8a, 0xf8, 0x19, 0xf8, 0xa8, 0xf9, 0x38, 0xf9, 0xc7, 0xfa, 0x57,
      0xfa, 0xe7, 0xfb, 0x77, 0xfc, 0x07, 0xfc, 0x98, 0xfd, 0x29, 0xfd,
      0xba, 0xfe, 0x4b, 0xfe, 0xdc, 0xff, 0x6d, 0xff, 0xff
    };

  StringInfo
    *profile;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (GetImageProfile(image,"icm") != (const StringInfo *) NULL)
    return(MagickFalse);
  profile=AcquireStringInfo(sizeof(sRGBProfile));
  SetStringInfoDatum(profile,sRGBProfile);
  status=SetImageProfile(image,"icm",profile);
  profile=DestroyStringInfo(profile);
  return(status);
}
#if defined(MAGICKCORE_LCMS_DELEGATE)
#if defined(LCMS_VERSION) && (LCMS_VERSION > 1010)
static int LCMSErrorHandler(int severity,const char *message)
{
  (void) LogMagickEvent(TransformEvent,GetMagickModule(),"lcms: #%d, %s",
    severity,message != (char *) NULL ? message : "no message");
  return(1);
}
#endif
#endif

MagickExport MagickBooleanType ProfileImage(Image *image,const char *name,
  const void *datum,const size_t length,
  const MagickBooleanType magick_unused(clone))
{
#define ProfileImageTag  "Profile/Image"
#define ThrowProfileException(severity,tag,context) \
{ \
  (void) cmsCloseProfile(source_profile); \
  (void) cmsCloseProfile(target_profile); \
  ThrowBinaryException(severity,tag,context); \
}

  MagickBooleanType
    status;

  StringInfo
    *profile;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(name != (const char *) NULL);
  if ((datum == (const void *) NULL) || (length == 0))
    {
      char
        **arguments,
        *names;

      int
        number_arguments;

      register long
        i;

      /*
        Delete image profile(s).
      */
      names=ConstantString(name);
      (void) SubstituteString(&names,","," ");
      arguments=StringToArgv(names,&number_arguments);
      names=DestroyString(names);
      if (arguments == (char **) NULL)
        return(MagickTrue);
      ResetImageProfileIterator(image);
      for (name=GetNextImageProfile(image); name != (const char *) NULL; )
      {
        for (i=1; i < number_arguments; i++)
        {
          if ((*arguments[i] == '!') &&
              (LocaleCompare(name,arguments[i]+1) == 0))
            break;
          if (GlobExpression(name,arguments[i],MagickTrue) != MagickFalse)
            {
              (void) DeleteImageProfile(image,name);
              ResetImageProfileIterator(image);
              break;
            }
        }
        name=GetNextImageProfile(image);
      }
      for (i=0; i < number_arguments; i++)
        arguments[i]=DestroyString(arguments[i]);
      arguments=(char **) RelinquishMagickMemory(arguments);
      return(MagickTrue);
    }
  /*
    Add a ICC, IPTC, or generic profile to the image.
  */
  profile=AcquireStringInfo((size_t) length);
  SetStringInfoDatum(profile,(unsigned char *) datum);
  if ((LocaleCompare(name,"icc") == 0) || (LocaleCompare(name,"icm") == 0))
    {
      const StringInfo
        *icc_profile;

      icc_profile=GetImageProfile(image,"icc");
      if ((icc_profile != (const StringInfo *) NULL) &&
          (CompareStringInfo(icc_profile,profile) == 0))
        {
          const char
            *value;

          value=GetImageProperty(image,"exif:ColorSpace");
          if (LocaleCompare(value,"1") != 0)
            (void) SetsRGBImageProfile(image);
          value=GetImageProperty(image,"exif:InteroperabilityIndex");
          if (LocaleCompare(value,"R98.") != 0)
            (void) SetsRGBImageProfile(image);
          value=GetImageProperty(image,"exif:InteroperabilityIndex");
          if (LocaleCompare(value,"R03.") != 0)
            (void) SetAdobeRGB1998ImageProfile(image);
          icc_profile=GetImageProfile(image,"icc");
        }
      if ((icc_profile != (const StringInfo *) NULL) &&
          (CompareStringInfo(icc_profile,profile) == 0))
        {
          profile=DestroyStringInfo(profile);
          return(MagickTrue);
        }
#if !defined(MAGICKCORE_LCMS_DELEGATE)
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        MissingDelegateWarning,"DelegateLibrarySupportNotBuiltIn","`%s' (LCMS)",
        image->filename);
#else
      if (icc_profile != (StringInfo *) NULL)
        {
          CacheView
            *image_view;

          ColorspaceType
            source_colorspace,
            target_colorspace;

          cmsHPROFILE
            source_profile,
            target_profile;

          cmsHTRANSFORM
            *restrict transform;

          DWORD
            flags,
            source_type,
            target_type;

          ExceptionInfo
            *exception;

          int
            intent;

          long
            progress,
            y;

          MagickBooleanType
            status;

          size_t
            length,
            source_channels,
            target_channels;

          unsigned short
            **restrict source_pixels,
            **restrict target_pixels;

          /*
            Transform pixel colors as defined by the color profiles.
          */
#if defined(LCMS_VERSION) && (LCMS_VERSION > 1010)
          cmsSetErrorHandler(LCMSErrorHandler);
#else
          (void) cmsErrorAction(LCMS_ERROR_SHOW);
#endif
          source_profile=cmsOpenProfileFromMem(GetStringInfoDatum(icc_profile),
            (DWORD) GetStringInfoLength(icc_profile));
          target_profile=cmsOpenProfileFromMem(GetStringInfoDatum(profile),
            (DWORD) GetStringInfoLength(profile));
          if ((source_profile == (cmsHPROFILE) NULL) ||
              (target_profile == (cmsHPROFILE) NULL))
            ThrowBinaryException(ResourceLimitError,
              "ColorspaceColorProfileMismatch",name);
          switch (cmsGetColorSpace(source_profile))
          {
            case icSigCmykData:
            {
              source_colorspace=CMYKColorspace;
              source_type=(DWORD) TYPE_CMYK_16;
              source_channels=4;
              break;
            }
            case icSigGrayData:
            {
              source_colorspace=GRAYColorspace;
              source_type=(DWORD) TYPE_GRAY_16;
              source_channels=1;
              break;
            }
            case icSigLabData:
            {
              source_colorspace=LabColorspace;
              source_type=(DWORD) TYPE_Lab_16;
              source_channels=3;
              break;
            }
            case icSigLuvData:
            {
              source_colorspace=YUVColorspace;
              source_type=(DWORD) TYPE_YUV_16;
              source_channels=3;
              break;
            }
            case icSigRgbData:
            {
              source_colorspace=RGBColorspace;
              source_type=(DWORD) TYPE_RGB_16;
              source_channels=3;
              break;
            }
            case icSigXYZData:
            {
              source_colorspace=XYZColorspace;
              source_type=(DWORD) TYPE_XYZ_16;
              source_channels=3;
              break;
            }
            case icSigYCbCrData:
            {
              source_colorspace=YCbCrColorspace;
              source_type=(DWORD) TYPE_YCbCr_16;
              source_channels=3;
              break;
            }
            default:
            {
              source_colorspace=UndefinedColorspace;
              source_type=(DWORD) TYPE_RGB_16;
              source_channels=3;
              break;
            }
          }
          switch (cmsGetColorSpace(target_profile))
          {
            case icSigCmykData:
            {
              target_colorspace=CMYKColorspace;
              target_type=(DWORD) TYPE_CMYK_16;
              target_channels=4;
              break;
            }
            case icSigLabData:
            {
              target_colorspace=LabColorspace;
              target_type=(DWORD) TYPE_Lab_16;
              target_channels=3;
              break;
            }
            case icSigGrayData:
            {
              target_colorspace=GRAYColorspace;
              target_type=(DWORD) TYPE_GRAY_16;
              target_channels=1;
              break;
            }
            case icSigLuvData:
            {
              target_colorspace=YUVColorspace;
              target_type=(DWORD) TYPE_YUV_16;
              target_channels=3;
              break;
            }
            case icSigRgbData:
            {
              target_colorspace=RGBColorspace;
              target_type=(DWORD) TYPE_RGB_16;
              target_channels=3;
              break;
            }
            case icSigXYZData:
            {
              target_colorspace=XYZColorspace;
              target_type=(DWORD) TYPE_XYZ_16;
              target_channels=3;
              break;
            }
            case icSigYCbCrData:
            {
              target_colorspace=YCbCrColorspace;
              target_type=(DWORD) TYPE_YCbCr_16;
              target_channels=3;
              break;
            }
            default:
            {
              target_colorspace=UndefinedColorspace;
              target_type=(DWORD) TYPE_RGB_16;
              target_channels=3;
              break;
            }
          }
          exception=(&image->exception);
          if ((source_colorspace == UndefinedColorspace) ||
              (target_colorspace == UndefinedColorspace))
            ThrowProfileException(ImageError,"ColorspaceColorProfileMismatch",
              name);
           if ((source_colorspace == GRAYColorspace) &&
               (IsGrayImage(image,exception) == MagickFalse))
            ThrowProfileException(ImageError,"ColorspaceColorProfileMismatch",
              name);
           if ((source_colorspace == CMYKColorspace) &&
               (image->colorspace != CMYKColorspace))
            ThrowProfileException(ImageError,"ColorspaceColorProfileMismatch",
              name);
           if ((source_colorspace == XYZColorspace) &&
               (image->colorspace != XYZColorspace))
            ThrowProfileException(ImageError,"ColorspaceColorProfileMismatch",
              name);
           if ((source_colorspace == YCbCrColorspace) &&
               (image->colorspace != YCbCrColorspace))
            ThrowProfileException(ImageError,"ColorspaceColorProfileMismatch",
              name);
           if ((source_colorspace != CMYKColorspace) &&
               (source_colorspace != GRAYColorspace) &&
               (source_colorspace != LabColorspace) &&
               (source_colorspace != XYZColorspace) &&
               (source_colorspace != YCbCrColorspace) &&
               (image->colorspace != RGBColorspace))
            ThrowProfileException(ImageError,"ColorspaceColorProfileMismatch",
              name);
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
          transform=AcquireTransformThreadSet(source_profile,source_type,
            target_profile,target_type,intent,flags);
          (void) cmsCloseProfile(source_profile);
          if (transform == (cmsHTRANSFORM *) NULL)
            ThrowBinaryException(ImageError,"UnableToCreateColorTransform",
              name);
          /*
            Transform image as dictated by the source and target image profiles.
          */
          length=(size_t) image->columns;
          source_pixels=AcquirePixelThreadSet(image->columns,source_channels);
          target_pixels=AcquirePixelThreadSet(image->columns,target_channels);
          if ((source_pixels == (unsigned short **) NULL) ||
              (target_pixels == (unsigned short **) NULL))
            {
              transform=DestroyTransformThreadSet(transform);
              (void) cmsCloseProfile(target_profile);
              ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
                image->filename);
            }
          if (SetImageStorageClass(image,DirectClass) == MagickFalse)
            {
              target_pixels=DestroyPixelThreadSet(target_pixels);
              source_pixels=DestroyPixelThreadSet(source_pixels);
              transform=DestroyTransformThreadSet(transform);
              (void) cmsCloseProfile(target_profile);
              return(MagickFalse);
            }
          if (target_colorspace == CMYKColorspace)
            (void) SetImageColorspace(image,target_colorspace);
          status=MagickTrue;
          progress=0;
          image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
          #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
          for (y=0; y < (long) image->rows; y++)
          {
            MagickBooleanType
              sync;

            register IndexPacket
              *restrict indexes;

            register long
              id,
              x;

            register PixelPacket
              *restrict q;

            register unsigned short
              *p;

            if (status == MagickFalse)
              continue;
            q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
              exception);
            if (q == (PixelPacket *) NULL)
              {
                status=MagickFalse;
                continue;
              }
            indexes=GetCacheViewAuthenticIndexQueue(image_view);
            id=GetOpenMPThreadId();
            p=source_pixels[id];
            for (x=0; x < (long) image->columns; x++)
            {
              *p++=ScaleQuantumToShort(q->red);
              if (source_channels > 1)
                {
                  *p++=ScaleQuantumToShort(q->green);
                  *p++=ScaleQuantumToShort(q->blue);
                }
              if (source_channels > 3)
                *p++=ScaleQuantumToShort(indexes[x]);
              q++;
            }
            cmsDoTransform(transform[id],source_pixels[id],target_pixels[id],
              (unsigned int) image->columns);
            p=target_pixels[id];
            q-=image->columns;
            for (x=0; x < (long) image->columns; x++)
            {
              q->red=ScaleShortToQuantum(*p);
              q->green=q->red;
              q->blue=q->red;
              p++;
              if (target_channels > 1)
                {
                  q->green=ScaleShortToQuantum(*p);
                  p++;
                  q->blue=ScaleShortToQuantum(*p);
                  p++;
                }
              if (target_channels > 3)
                {
                  indexes[x]=ScaleShortToQuantum(*p);
                  p++;
                }
              q++;
            }
            sync=SyncCacheViewAuthenticPixels(image_view,exception);
            if (sync == MagickFalse)
              status=MagickFalse;
            if (image->progress_monitor != (MagickProgressMonitor) NULL)
              {
                MagickBooleanType
                  proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp critical (MagickCore_ProfileImage)
#endif
                proceed=SetImageProgress(image,ProfileImageTag,progress++,
                  image->rows);
                if (proceed == MagickFalse)
                  status=MagickFalse;
              }
          }
          image_view=DestroyCacheView(image_view);
          (void) SetImageColorspace(image,target_colorspace);
          switch (cmsGetColorSpace(target_profile))
          {
            case icSigRgbData:
            {
              image->type=image->matte == MagickFalse ? TrueColorType :
                TrueColorMatteType;
              break;
            }
            case icSigCmykData:
            {
              image->type=image->matte == MagickFalse ? ColorSeparationType :
                ColorSeparationMatteType;
              break;
            }
            case icSigGrayData:
            {
              image->type=image->matte == MagickFalse ? GrayscaleType :
                GrayscaleMatteType;
              break;
            }
            default:
              break;
          }
          target_pixels=DestroyPixelThreadSet(target_pixels);
          source_pixels=DestroyPixelThreadSet(source_pixels);
          transform=DestroyTransformThreadSet(transform);
          (void) cmsCloseProfile(target_profile);
        }
#endif
    }
  status=SetImageProfile(image,name,profile);
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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->profiles == (SplayTreeInfo *) NULL)
    return((StringInfo *) NULL);
  if (LocaleCompare(name,"icc") == 0)
    {
      /*
        Continue to support deprecated color profile for now.
      */
      image->color_profile.length=0;
      image->color_profile.info=(unsigned char *) NULL;
    }
  if (LocaleCompare(name,"iptc") == 0)
    {
      /*
        Continue to support deprecated IPTC profile for now.
      */
      image->iptc_profile.length=0;
      image->iptc_profile.info=(unsigned char *) NULL;
    }
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
  assert(image->signature == MagickSignature);
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

static inline const unsigned char *ReadResourceBytes(const unsigned char *p,
  const ssize_t count,unsigned char *quantum)
{
  register ssize_t
    i;

  for (i=0; i < count; i++)
    *quantum++=(*p++);
  return(p);
}

static inline const unsigned char *ReadResourceLong(const unsigned char *p,
  unsigned long *quantum)
{
  *quantum=(unsigned long) (*p++ << 24);
  *quantum|=(unsigned long) (*p++ << 16);
  *quantum|=(unsigned long) (*p++ << 8);
  *quantum|=(unsigned long) (*p++ << 0);
  return(p);
}

static inline const unsigned char *ReadResourceShort(const unsigned char *p,
  unsigned short *quantum)
{
  *quantum=(unsigned short) (*p++ << 8);
  *quantum|=(unsigned short) (*p++ << 0);
  return(p);
}

static MagickBooleanType GetProfilesFromResourceBlock(Image *image,
  const StringInfo *resource_block)
{
  const unsigned char
    *datum;

  register const unsigned char
    *p;

  size_t
    length;

  StringInfo
    *profile;

  unsigned char
    length_byte;

  unsigned long
    count;

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
    p=ReadResourceLong(p,&count);
    if ((p > (datum+length-count)) || (count > length))
      break;
    switch (id)
    {
      case 0x03ed:
      {
        unsigned short
          resolution;

        /*
          Resolution.
        */
        p=ReadResourceShort(p,&resolution)+6;
        image->x_resolution=(double) resolution;
        p=ReadResourceShort(p,&resolution)+6;
        image->y_resolution=(double) resolution;
        break;
      }
      case 0x0404:
      {
        /*
          IPTC Profile
        */
        profile=AcquireStringInfo(count);
        SetStringInfoDatum(profile,p);
        (void) SetImageProfile(image,"iptc",profile);
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
        (void) SetImageProfile(image,"icc",profile);
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
        (void) SetImageProfile(image,"exif",profile);
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
        (void) SetImageProfile(image,"xmp",profile);
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
  return(MagickTrue);
}

MagickExport MagickBooleanType SetImageProfile(Image *image,const char *name,
  const StringInfo *profile)
{
  char
    key[MaxTextExtent],
    property[MaxTextExtent];

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->profiles == (SplayTreeInfo *) NULL)
    image->profiles=NewSplayTree(CompareSplayTreeString,RelinquishMagickMemory,
      DestroyProfile);
  (void) CopyMagickString(key,name,MaxTextExtent);
  status=AddValueToSplayTree((SplayTreeInfo *) image->profiles,
    ConstantString(key),CloneStringInfo(profile));
  if ((status != MagickFalse) &&
      ((LocaleCompare(name,"icc") == 0) || (LocaleCompare(name,"icm") == 0)))
    {
      const StringInfo
        *icc_profile;

      /*
        Continue to support deprecated color profile member.
      */
      icc_profile=GetImageProfile(image,name);
      if (icc_profile != (const StringInfo *) NULL)
        {
          image->color_profile.length=GetStringInfoLength(icc_profile);
          image->color_profile.info=GetStringInfoDatum(icc_profile);
        }
    }
  if ((status != MagickFalse) &&
      ((LocaleCompare(name,"iptc") == 0) || (LocaleCompare(name,"8bim") == 0)))
    {
      const StringInfo
        *iptc_profile;

      /*
        Continue to support deprecated IPTC profile member.
      */
      iptc_profile=GetImageProfile(image,name);
      if (iptc_profile != (const StringInfo *) NULL)
        {
          image->iptc_profile.length=GetStringInfoLength(iptc_profile);
          image->iptc_profile.info=GetStringInfoDatum(iptc_profile);
        }
      (void) GetProfilesFromResourceBlock(image,profile);
    }
  /*
    Inject profile into image properties.
  */
  (void) FormatMagickString(property,MaxTextExtent,"%s:sans",name);
  (void) GetImageProperty(image,property);
  return(status);
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

static inline unsigned short ReadProfileShort(const EndianType endian,
  unsigned char *buffer)
{
  unsigned short
    value;

  if (endian == MSBEndian)
    {
      value=(unsigned short) ((((unsigned char *) buffer)[0] << 8) |
        ((unsigned char *) buffer)[1]);
      return((unsigned short) (value & 0xffff));
    }
  value=(unsigned short) ((buffer[1] << 8) | buffer[0]);
  return((unsigned short) (value & 0xffff));
}

static inline unsigned long ReadProfileLong(const EndianType endian,
  unsigned char *buffer)
{
  unsigned long
    value;

  if (endian == MSBEndian)
    {
      value=(unsigned long) ((buffer[0] << 24) | (buffer[1] << 16) |
        (buffer[2] << 8) | buffer[3]);
      return((unsigned long) (value & 0xffffffff));
    }
  value=(unsigned long) ((buffer[3] << 24) | (buffer[2] << 16) |
    (buffer[1] << 8 ) | (buffer[0]));
  return((unsigned long) (value & 0xffffffff));
}

static inline void WriteProfileLong(const EndianType endian,
  const unsigned long value,unsigned char *p)
{
  unsigned char
    buffer[4];

  if (endian == MSBEndian)
    {
      buffer[0]=(unsigned char) (value >> 24);
      buffer[1]=(unsigned char) (value >> 16);
      buffer[2]=(unsigned char) (value >> 8);
      buffer[3]=(unsigned char) value;
      (void) CopyMagickMemory(p,buffer,4);
      return;
    }
  buffer[0]=(unsigned char) value;
  buffer[1]=(unsigned char) (value >> 8);
  buffer[2]=(unsigned char) (value >> 16);
  buffer[3]=(unsigned char) (value >> 24);
  (void) CopyMagickMemory(p,buffer,4);
}

static void WriteProfileShort(const EndianType endian,
  const unsigned short value,unsigned char *p)
{
  unsigned char
    buffer[2];

  if (endian == MSBEndian)
    {
      buffer[0]=(unsigned char) (value >> 8);
      buffer[1]=(unsigned char) value;
      (void) CopyMagickMemory(p,buffer,2);
      return;
    }
  buffer[0]=(unsigned char) value;
  buffer[1]=(unsigned char) (value >> 8);
  (void) CopyMagickMemory(p,buffer,2);
}

MagickExport MagickBooleanType SyncImageProfiles(Image *image)
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

    unsigned long
      entry;
  } DirectoryInfo;

  DirectoryInfo
    directory_stack[MaxDirectoryStack];

  EndianType
    endian;

  long
    id,
    level;

  size_t
    length;

  ssize_t
    offset;

  static int
    format_bytes[] = {0, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8};

  StringInfo
    *profile;

  unsigned char
    *directory,
    *exif;

  unsigned long
    entry,
    number_entries;

  /*
    Set EXIF resolution tag.
  */
  profile=(StringInfo *) GetImageProfile(image,"EXIF");
  if (profile == (StringInfo *) NULL)
    return(MagickTrue);
  length=GetStringInfoLength(profile);
  exif=GetStringInfoDatum(profile);
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
  id=(int) ReadProfileShort(LSBEndian,exif);
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
  if ((size_t) offset >= length)
    return(MagickFalse);
  directory=exif+offset;
  level=0;
  entry=0;
  do
  {
    if (level > 0)
      {
        level--;
        directory=directory_stack[level].directory;
        entry=directory_stack[level].entry;
      }
    /*
      Determine how many entries there are in the current IFD.
    */
    number_entries=ReadProfileShort(endian,directory);
    for ( ; entry < number_entries; entry++)
    {
      long
        components,
        format,
        tag_value;

      register unsigned char
        *p,
        *q;

      size_t
        number_bytes;

      q=(unsigned char *) (directory+2+(12*entry));
      tag_value=(long) ReadProfileShort(endian,q);
      format=(long) ReadProfileShort(endian,q+2);
      if ((format-1) >= EXIF_NUM_FORMATS)
        break;
      components=(long) ReadProfileLong(endian,q+4);
      number_bytes=(size_t) components*format_bytes[format];
      if (number_bytes <= 4)
        p=q+8;
      else
        {
          ssize_t
            offset;

          /*
            The directory entry contains an offset.
          */
          offset=(ssize_t) ReadProfileLong(endian,q+8);
          if ((size_t) (offset+number_bytes) > length)
            continue;
          p=(unsigned char *) (exif+offset);
        }
      switch (tag_value)
      {
        case 0x011a:
        {
          (void) WriteProfileLong(endian,(unsigned long)
            (image->x_resolution+0.5),p);
          (void) WriteProfileLong(endian,1UL,p+4);
          break;
        }
        case 0x011b:
        {
          (void) WriteProfileLong(endian,(unsigned long)
            (image->y_resolution+0.5),p);
          (void) WriteProfileLong(endian,1UL,p+4);
          break;
        }
        case 0x0112:
        {
          (void) WriteProfileShort(endian,(unsigned short)
            image->orientation,p);
          break;
        }
        case 0x0128:
        {
          (void) WriteProfileShort(endian,(unsigned short)
            (image->units+1),p);
          break;
        }
        default:
          break;
      }
      if ((tag_value == TAG_EXIF_OFFSET) || (tag_value == TAG_INTEROP_OFFSET))
        {
          size_t
            offset;

          offset=(size_t) ReadProfileLong(endian,p);
          if ((offset < length) && (level < (MaxDirectoryStack-2)))
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
              offset=(size_t) ReadProfileLong(endian,directory+2+(12*
                number_entries));
              if ((offset != 0) && (offset < length) &&
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
  return(MagickTrue);
}

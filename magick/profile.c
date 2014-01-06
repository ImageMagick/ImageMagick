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
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/colorspace-private.h"
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
#include "magick/resource_.h"
#include "magick/splay-tree.h"
#include "magick/string_.h"
#include "magick/thread-private.h"
#include "magick/token.h"
#include "magick/utility.h"
#if defined(MAGICKCORE_LCMS_DELEGATE)
#if defined(MAGICKCORE_HAVE_LCMS_LCMS2_H)
#include <wchar.h>
#include <lcms/lcms2.h>
#elif defined(MAGICKCORE_HAVE_LCMS2_H)
#include <wchar.h>
#include "lcms2.h"
#elif defined(MAGICKCORE_HAVE_LCMS_LCMS_H)
#include <lcms/lcms.h>
#else
#include "lcms.h"
#endif
#endif

/*
  Define declarations.
*/
#if !defined(LCMS_VERSION) || (LCMS_VERSION < 2000)
#define cmsSigCmykData icSigCmykData
#define cmsSigGrayData icSigGrayData
#define cmsSigLabData icSigLabData
#define cmsSigLuvData icSigLuvData
#define cmsSigRgbData icSigRgbData
#define cmsSigXYZData icSigXYZData
#define cmsSigYCbCrData icSigYCbCrData
#define cmsSigLinkClass icSigLinkClass
#define cmsColorSpaceSignature icColorSpaceSignature
#define cmsUInt32Number  DWORD
#define cmsSetLogErrorHandler(handler)  cmsSetErrorHandler(handler)
#define cmsCreateTransformTHR(context,source_profile,source_type, \
  target_profile,target_type,intent,flags)  cmsCreateTransform(source_profile, \
  source_type,target_profile,target_type,intent,flags);
#define cmsOpenProfileFromMemTHR(context,profile,length) \
  cmsOpenProfileFromMem(profile,length)
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
  register ssize_t
    i;

  assert(pixels != (unsigned short **) NULL);
  for (i=0; i < (ssize_t) GetMagickResourceLimit(ThreadResource); i++)
    if (pixels[i] != (unsigned short *) NULL)
      pixels[i]=(unsigned short *) RelinquishMagickMemory(pixels[i]);
  pixels=(unsigned short **) RelinquishMagickMemory(pixels);
  return(pixels);
}

static unsigned short **AcquirePixelThreadSet(const size_t columns,
  const size_t channels)
{
  register ssize_t
    i;

  unsigned short
    **pixels;

  size_t
    number_threads;

  number_threads=(size_t) GetMagickResourceLimit(ThreadResource);
  pixels=(unsigned short **) AcquireQuantumMemory(number_threads,
    sizeof(*pixels));
  if (pixels == (unsigned short **) NULL)
    return((unsigned short **) NULL);
  (void) ResetMagickMemory(pixels,0,number_threads*sizeof(*pixels));
  for (i=0; i < (ssize_t) number_threads; i++)
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
  const int intent,const cmsUInt32Number flags)
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
  (void) ResetMagickMemory(transform,0,number_threads*sizeof(*transform));
  for (i=0; i < (ssize_t) number_threads; i++)
  {
    transform[i]=cmsCreateTransformTHR(image,source_profile,source_type,
      target_profile,target_type,intent,flags);
    if (transform[i] == (cmsHTRANSFORM) NULL)
      return(DestroyTransformThreadSet(transform));
  }
  return(transform);
}
#endif

#if defined(MAGICKCORE_LCMS_DELEGATE)
#if defined(LCMS_VERSION) && (LCMS_VERSION >= 2000)
static void LCMSExceptionHandler(cmsContext context,cmsUInt32Number severity,
  const char *message)
{
  Image
    *image;

  (void) LogMagickEvent(TransformEvent,GetMagickModule(),"lcms: #%u, %s",
    severity,message != (char *) NULL ? message : "no message");
  image=(Image *) context;
  if (image != (Image *) NULL)
    (void) ThrowMagickException(&image->exception,GetMagickModule(),
      ImageWarning,"UnableToTransformColorspace","`%s'",image->filename);

}
#else
static int LCMSExceptionHandler(int severity,const char *message)
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

  magick_unreferenced(clone);

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

      register ssize_t
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
        for (i=1; i < (ssize_t) number_arguments; i++)
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
      for (i=0; i < (ssize_t) number_arguments; i++)
        arguments[i]=DestroyString(arguments[i]);
      arguments=(char **) RelinquishMagickMemory(arguments);
      return(MagickTrue);
    }
  /*
    Add a ICC, IPTC, or generic profile to the image.
  */
  status=MagickTrue;
  profile=AcquireStringInfo((size_t) length);
  SetStringInfoDatum(profile,(unsigned char *) datum);
  if ((LocaleCompare(name,"icc") != 0) && (LocaleCompare(name,"icm") != 0))
    status=SetImageProfile(image,name,profile);
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

          value=GetImageProperty(image,"exif:ColorSpace");
          (void) value;
          /* Future.
          if (LocaleCompare(value,"1") != 0)
            (void) SetsRGBImageProfile(image);
          value=GetImageProperty(image,"exif:InteroperabilityIndex");
          if (LocaleCompare(value,"R98.") != 0)
            (void) SetsRGBImageProfile(image);
          value=GetImageProperty(image,"exif:InteroperabilityIndex");
          if (LocaleCompare(value,"R03.") != 0)
            (void) SetAdobeRGB1998ImageProfile(image);
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
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        MissingDelegateWarning,"DelegateLibrarySupportNotBuiltIn","`%s' (LCMS)",
        image->filename);
#else
      {
        cmsHPROFILE
          source_profile;

        /*
          Transform pixel colors as defined by the color profiles.
        */
        cmsSetLogErrorHandler(LCMSExceptionHandler);
        source_profile=cmsOpenProfileFromMemTHR(image,
          GetStringInfoDatum(profile),(cmsUInt32Number)
          GetStringInfoLength(profile));
        if (source_profile == (cmsHPROFILE) NULL)
          ThrowBinaryException(ResourceLimitError,
            "ColorspaceColorProfileMismatch",name);
        if ((cmsGetDeviceClass(source_profile) != cmsSigLinkClass) &&
            (icc_profile == (StringInfo *) NULL))
          status=SetImageProfile(image,name,profile);
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
              *restrict transform;

            cmsUInt32Number
              flags,
              source_type,
              target_type;

            ExceptionInfo
              *exception;

            int
              intent;

            MagickBooleanType
              status;

            MagickOffsetType
              progress;

            size_t
              source_channels,
              target_channels;

            ssize_t
              y;

            unsigned short
              **restrict source_pixels,
              **restrict target_pixels;

            exception=(&image->exception);
            target_profile=(cmsHPROFILE) NULL;
            if (icc_profile != (StringInfo *) NULL)
              {
                target_profile=source_profile;
                source_profile=cmsOpenProfileFromMemTHR(image,
                  GetStringInfoDatum(icc_profile),(cmsUInt32Number)
                  GetStringInfoLength(icc_profile));
                if (source_profile == (cmsHPROFILE) NULL)
                  ThrowProfileException(ResourceLimitError,
                    "ColorspaceColorProfileMismatch",name);
              }
            switch (cmsGetColorSpace(source_profile))
            {
              case cmsSigCmykData:
              {
                source_colorspace=CMYKColorspace;
                source_type=(cmsUInt32Number) TYPE_CMYK_16;
                source_channels=4;
                break;
              }
              case cmsSigGrayData:
              {
                source_colorspace=GRAYColorspace;
                source_type=(cmsUInt32Number) TYPE_GRAY_16;
                source_channels=1;
                break;
              }
              case cmsSigLabData:
              {
                source_colorspace=LabColorspace;
                source_type=(cmsUInt32Number) TYPE_Lab_16;
                source_channels=3;
                break;
              }
              case cmsSigLuvData:
              {
                source_colorspace=YUVColorspace;
                source_type=(cmsUInt32Number) TYPE_YUV_16;
                source_channels=3;
                break;
              }
              case cmsSigRgbData:
              {
                source_colorspace=sRGBColorspace;
                source_type=(cmsUInt32Number) TYPE_RGB_16;
                source_channels=3;
                break;
              }
              case cmsSigXYZData:
              {
                source_colorspace=XYZColorspace;
                source_type=(cmsUInt32Number) TYPE_XYZ_16;
                source_channels=3;
                break;
              }
              case cmsSigYCbCrData:
              {
                source_colorspace=YCbCrColorspace;
                source_type=(cmsUInt32Number) TYPE_YCbCr_16;
                source_channels=3;
                break;
              }
              default:
              {
                source_colorspace=UndefinedColorspace;
                source_type=(cmsUInt32Number) TYPE_RGB_16;
                source_channels=3;
                break;
              }
            }
            signature=cmsGetPCS(source_profile);
            if (target_profile != (cmsHPROFILE) NULL)
              signature=cmsGetColorSpace(target_profile);
            switch (signature)
            {
              case cmsSigCmykData:
              {
                target_colorspace=CMYKColorspace;
                target_type=(cmsUInt32Number) TYPE_CMYK_16;
                target_channels=4;
                break;
              }
              case cmsSigLabData:
              {
                target_colorspace=LabColorspace;
                target_type=(cmsUInt32Number) TYPE_Lab_16;
                target_channels=3;
                break;
              }
              case cmsSigGrayData:
              {
                target_colorspace=GRAYColorspace;
                target_type=(cmsUInt32Number) TYPE_GRAY_16;
                target_channels=1;
                break;
              }
              case cmsSigLuvData:
              {
                target_colorspace=YUVColorspace;
                target_type=(cmsUInt32Number) TYPE_YUV_16;
                target_channels=3;
                break;
              }
              case cmsSigRgbData:
              {
                target_colorspace=sRGBColorspace;
                target_type=(cmsUInt32Number) TYPE_RGB_16;
                target_channels=3;
                break;
              }
              case cmsSigXYZData:
              {
                target_colorspace=XYZColorspace;
                target_type=(cmsUInt32Number) TYPE_XYZ_16;
                target_channels=3;
                break;
              }
              case cmsSigYCbCrData:
              {
                target_colorspace=YCbCrColorspace;
                target_type=(cmsUInt32Number) TYPE_YCbCr_16;
                target_channels=3;
                break;
              }
              default:
              {
                target_colorspace=UndefinedColorspace;
                target_type=(cmsUInt32Number) TYPE_RGB_16;
                target_channels=3;
                break;
              }
            }
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
                 (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse))
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
            transform=AcquireTransformThreadSet(image,source_profile,
              source_type,target_profile,target_type,intent,flags);
            if (transform == (cmsHTRANSFORM *) NULL)
              ThrowProfileException(ImageError,"UnableToCreateColorTransform",
                name);
            /*
              Transform image as dictated by the source & target image profiles.
            */
            source_pixels=AcquirePixelThreadSet(image->columns,source_channels);
            target_pixels=AcquirePixelThreadSet(image->columns,target_channels);
            if ((source_pixels == (unsigned short **) NULL) ||
                (target_pixels == (unsigned short **) NULL))
              {
                transform=DestroyTransformThreadSet(transform);
                ThrowProfileException(ResourceLimitError,
                  "MemoryAllocationFailed",image->filename);
              }
            if (SetImageStorageClass(image,DirectClass) == MagickFalse)
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
              (void) SetImageColorspace(image,target_colorspace);
            status=MagickTrue;
            progress=0;
            image_view=AcquireAuthenticCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
            #pragma omp parallel for schedule(static,4) shared(status) \
              magick_threads(image,image,image->rows,1)
#endif
            for (y=0; y < (ssize_t) image->rows; y++)
            {
              const int
                id = GetOpenMPThreadId();

              MagickBooleanType
                sync;

              register IndexPacket
                *restrict indexes;

              register ssize_t
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
              p=source_pixels[id];
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                *p++=ScaleQuantumToShort(GetPixelRed(q));
                if (source_channels > 1)
                  {
                    *p++=ScaleQuantumToShort(GetPixelGreen(q));
                    *p++=ScaleQuantumToShort(GetPixelBlue(q));
                  }
                if (source_channels > 3)
                  *p++=ScaleQuantumToShort(GetPixelIndex(indexes+x));
                q++;
              }
              cmsDoTransform(transform[id],source_pixels[id],target_pixels[id],
                (unsigned int) image->columns);
              p=target_pixels[id];
              q-=image->columns;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                SetPixelRed(q,ScaleShortToQuantum(*p));
                SetPixelGreen(q,GetPixelRed(q));
                SetPixelBlue(q,GetPixelRed(q));
                p++;
                if (target_channels > 1)
                  {
                    SetPixelGreen(q,ScaleShortToQuantum(*p));
                    p++;
                    SetPixelBlue(q,ScaleShortToQuantum(*p));
                    p++;
                  }
                if (target_channels > 3)
                  {
                    SetPixelIndex(indexes+x,ScaleShortToQuantum(*p));
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
            switch (signature)
            {
              case cmsSigRgbData:
              {
                image->type=image->matte == MagickFalse ? TrueColorType :
                  TrueColorMatteType;
                break;
              }
              case cmsSigCmykData:
              {
                image->type=image->matte == MagickFalse ? ColorSeparationType :
                  ColorSeparationMatteType;
                break;
              }
              case cmsSigGrayData:
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
            if (cmsGetDeviceClass(source_profile) != cmsSigLinkClass)
              status=SetImageProfile(image,name,profile);
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
  size_t *quantum)
{
  *quantum=(size_t) (*p++ << 24);
  *quantum|=(size_t) (*p++ << 16);
  *quantum|=(size_t) (*p++ << 8);
  *quantum|=(size_t) (*p++ << 0);
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

  size_t
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
  (void) FormatLocaleString(property,MaxTextExtent,"%s:*",name);
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

  if (endian == LSBEndian)
    {
      value=(unsigned short) ((buffer[1] << 8) | buffer[0]);
      return((unsigned short) (value & 0xffff));
    }
  value=(unsigned short) ((((unsigned char *) buffer)[0] << 8) |
    ((unsigned char *) buffer)[1]);
  return((unsigned short) (value & 0xffff));
}

static inline size_t ReadProfileLong(const EndianType endian,
  unsigned char *buffer)
{
  size_t
    value;

  if (endian == LSBEndian)
    {
      value=(size_t) ((buffer[3] << 24) | (buffer[2] << 16) |
        (buffer[1] << 8 ) | (buffer[0]));
      return((size_t) (value & 0xffffffff));
    }
  value=(size_t) ((buffer[0] << 24) | (buffer[1] << 16) |
    (buffer[2] << 8) | buffer[3]);
  return((size_t) (value & 0xffffffff));
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
      (void) CopyMagickMemory(p,buffer,4);
      return;
    }
  buffer[0]=(unsigned char) (value >> 24);
  buffer[1]=(unsigned char) (value >> 16);
  buffer[2]=(unsigned char) (value >> 8);
  buffer[3]=(unsigned char) value;
  (void) CopyMagickMemory(p,buffer,4);
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
      (void) CopyMagickMemory(p,buffer,2);
      return;
    }
  buffer[0]=(unsigned char) (value >> 8);
  buffer[1]=(unsigned char) value;
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

  StringInfo
    *profile;

  unsigned char
    *directory,
    *exif;

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
  id=(ssize_t) ReadProfileShort(LSBEndian,exif);
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
  offset=(ssize_t) ((int) ReadProfileLong(endian,exif+4));
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
    /*
      Determine how many entries there are in the current IFD.
    */
    number_entries=ReadProfileShort(endian,directory);
    for ( ; entry < number_entries; entry++)
    {
      register unsigned char
        *p,
        *q;

      size_t
        number_bytes;

      ssize_t
        components,
        format,
        tag_value;

      q=(unsigned char *) (directory+2+(12*entry));
      if (GetValueFromSplayTree(exif_resources,q) == q)
        break;
      (void) AddValueToSplayTree(exif_resources,q,q);
      tag_value=(ssize_t) ReadProfileShort(endian,q);
      format=(ssize_t) ReadProfileShort(endian,q+2);
      if ((format-1) >= EXIF_NUM_FORMATS)
        break;
      components=(ssize_t) ((int) ReadProfileLong(endian,q+4));
      number_bytes=(size_t) components*format_bytes[format];
      if ((ssize_t) number_bytes < components)
        break;  /* prevent overflow */
      if (number_bytes <= 4)
        p=q+8;
      else
        {
          ssize_t
            offset;

          /*
            The directory entry contains an offset.
          */
          offset=(ssize_t) ((int) ReadProfileLong(endian,q+8));
          if ((ssize_t) (offset+number_bytes) < offset)
            continue;  /* prevent overflow */
          if ((size_t) (offset+number_bytes) > length)
            continue;
          p=(unsigned char *) (exif+offset);
        }
      switch (tag_value)
      {
        case 0x011a:
        {
          (void) WriteProfileLong(endian,(size_t) (image->x_resolution+0.5),p);
          (void) WriteProfileLong(endian,1UL,p+4);
          break;
        }
        case 0x011b:
        {
          (void) WriteProfileLong(endian,(size_t) (image->y_resolution+0.5),p);
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
          ssize_t
            offset;

          offset=(ssize_t) ((int) ReadProfileLong(endian,p));
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
              offset=(ssize_t) ((int) ReadProfileLong(endian,directory+2+(12*
                number_entries)));
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

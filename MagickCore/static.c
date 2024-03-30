/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  SSSSS  TTTTT   AAA   TTTTT  IIIII   CCCC                   %
%                  SS       T    A   A    T      I    C                       %
%                   SSS     T    AAAAA    T      I    C                       %
%                     SS    T    A   A    T      I    C                       %
%                  SSSSS    T    A   A    T    IIIII   CCCC                   %
%                                                                             %
%                                                                             %
%                          MagickCore Static Methods                          %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 March 2000                                  %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/coder.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/module.h"
#include "MagickCore/policy.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "coders/coders.h"

/*
  Define declarations.
*/
#define AddMagickCoder(coder)  { #coder, MagickFalse, \
  Register ## coder ## Image, Unregister ## coder ## Image },

/*
  ImageMagick module stub.
*/
ModuleExport size_t RegisterUndefinedImage(void)
{
  return(MagickImageCoderSignature);
}

ModuleExport void UnregisterUndefinedImage(void)
{
}

/*
  ImageMagick modules.
*/
static struct
{
  const char
    *module;

  MagickBooleanType
    registered;

  size_t
    (*register_module)(void);

  void
    (*unregister_module)(void);
} MagickModules[] = {
#if !defined(MAGICKCORE_BUILD_MODULES)
  #include "coders/coders-list.h"
#endif
  { (const char *) NULL, MagickFalse, RegisterUndefinedImage, UnregisterUndefinedImage }
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n v o k e S t a t i c I m a g e F i l t e r                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InvokeStaticImageFilter() invokes a static image filter.
%
%  The format of the InvokeStaticImageFilter method is:
%
%      MagickBooleanType InvokeStaticImageFilter(const char *tag,Image **image,
%        const int argc,const char **argv)
%
%  A description of each parameter follows:
%
%    o tag: the module tag.
%
%    o image: the image.
%
%    o argc: the number of elements in the argument vector.
%
%    o argv: A text array containing the command line arguments.
%
%    o argv: A text array containing the command line arguments.
%
%    o exception: return any errors or warnings in this structure.
%
*/
#if defined(MAGICKCORE_MODULES_SUPPORT)
MagickExport MagickBooleanType InvokeStaticImageFilter(const char *tag,
  Image **image,const int argc,const char **argv,ExceptionInfo *exception)
{
  PolicyRights
    rights;

  assert(image != (Image **) NULL);
  assert((*image)->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  rights=ReadPolicyRights;
  if (IsRightsAuthorized(FilterPolicyDomain,rights,tag) == MagickFalse)
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",tag);
      return(MagickFalse);
    }
#if defined(MAGICKCORE_MODULES_SUPPORT)
  (void) tag;
  (void) argc;
  (void) argv;
  (void) exception;
#else
  {
    extern size_t
      analyzeImage(Image **,const int,char **,ExceptionInfo *);

    ImageFilterHandler
      *image_filter;

    image_filter=(ImageFilterHandler *) NULL;
    if (LocaleCompare("analyze",tag) == 0)
      image_filter=(ImageFilterHandler *) analyzeImage;
    if (image_filter == (ImageFilterHandler *) NULL)
      (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
        "UnableToLoadModule","`%s'",tag);
    else
      {
        size_t
          signature;

        if ((*image)->debug != MagickFalse)
          (void) LogMagickEvent(TransformEvent,GetMagickModule(),
            "Invoking \"%s\" static image filter",tag);
        signature=image_filter(image,argc,argv,exception);
        if ((*image)->debug != MagickFalse)
          (void) LogMagickEvent(TransformEvent,GetMagickModule(),
            "\"%s\" completes",tag);
        if (signature != MagickImageFilterSignature)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),ModuleError,
              "ImageFilterSignatureMismatch","'%s': %8lx != %8lx",tag,
              (unsigned long) signature,(unsigned long)
              MagickImageFilterSignature);
            return(MagickFalse);
          }
      }
  }
#endif
  return(MagickTrue);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S t a t i c M o d u l e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterStaticModule() statically registers a module.
%
%  The format of the RegisterStaticModule method is:
%
%      MagickBooleanType RegisterStaticModule(const char module,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o module: the want to register.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType RegisterStaticModule(const char *module,
  ExceptionInfo *exception)
{
  char
    module_name[MagickPathExtent];

  PolicyRights
    rights;

  const CoderInfo
    *p;

  size_t
    extent;

  ssize_t
    i;

  /*
    Assign module name from alias.
  */
  assert(module != (const char *) NULL);
  (void) CopyMagickString(module_name,module,MagickPathExtent);
  p=GetCoderInfo(module,exception);
  if (p != (CoderInfo *) NULL)
    (void) CopyMagickString(module_name,p->name,MagickPathExtent);
  rights=(PolicyRights) (ReadPolicyRights | WritePolicyRights);
  if (IsRightsAuthorized(ModulePolicyDomain,rights,module_name) == MagickFalse)
    {
      errno=EPERM;
      (void) ThrowMagickException(exception,GetMagickModule(),PolicyError,
        "NotAuthorized","`%s'",module);
      return(MagickFalse);
    }
  extent=sizeof(MagickModules)/sizeof(MagickModules[0]);
  for (i=0; i < (ssize_t) extent; i++)
    if (LocaleCompare(MagickModules[i].module,module_name) == 0)
      {
        if (MagickModules[i].registered == MagickFalse)
          {
            (void) (MagickModules[i].register_module)();
            MagickModules[i].registered=MagickTrue;
          }
        return(MagickTrue);
      }
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S t a t i c M o d u l e s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterStaticModules() statically registers all the available module
%  handlers.
%
%  The format of the RegisterStaticModules method is:
%
%      (void) RegisterStaticModules(void)
%
*/
MagickExport void RegisterStaticModules(void)
{
  PolicyRights
    rights;

  size_t
    extent;

  ssize_t
    i;

  rights=(PolicyRights) (ReadPolicyRights | WritePolicyRights);
  extent=sizeof(MagickModules)/sizeof(MagickModules[0]);
  for (i=0; i < (ssize_t) extent; i++)
  {
    if (MagickModules[i].registered == MagickFalse)
      {
        if (IsRightsAuthorized(ModulePolicyDomain,rights,MagickModules[i].module) == MagickFalse)
          continue;
        (void) (MagickModules[i].register_module)();
        MagickModules[i].registered=MagickTrue;
      }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S t a t i c M o d u l e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterStaticModule() statically unregisters the named module.
%
%  The format of the UnregisterStaticModule method is:
%
%      MagickBooleanType UnregisterStaticModule(const char *module)
%
%  A description of each parameter follows:
%
%    o module: the module we want to unregister.
%
*/
MagickExport MagickBooleanType UnregisterStaticModule(const char *module)
{
  size_t
    extent;

  ssize_t
    i;

  extent=sizeof(MagickModules)/sizeof(MagickModules[0]);
  for (i=0; i < (ssize_t) extent; i++)
    if (LocaleCompare(MagickModules[i].module,module) == 0)
      {
        if (MagickModules[i].registered != MagickFalse)
          {
            (MagickModules[i].unregister_module)();
            MagickModules[i].registered=MagickFalse;
          }
        return(MagickTrue);
      }
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S t a t i c M o d u l e s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterStaticModules() statically unregisters all the available module
%  handlers.
%
%  The format of the UnregisterStaticModules method is:
%
%      UnregisterStaticModules(void)
%
*/
MagickExport void UnregisterStaticModules(void)
{
  size_t
    extent;

  ssize_t
    i;

  extent=sizeof(MagickModules)/sizeof(MagickModules[0]);
  for (i=0; i < (ssize_t) extent; i++)
  {
    if (MagickModules[i].registered != MagickFalse)
      {
        (MagickModules[i].unregister_module)();
        MagickModules[i].registered=MagickFalse;
      }
  }
}

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
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
} MagickModules[] =
{
#if !defined(MAGICKCORE_MODULES_SUPPORT)
  { "AAI", MagickFalse, RegisterAAIImage, UnregisterAAIImage },
  { "ART", MagickFalse, RegisterARTImage, UnregisterARTImage },
  { "AVS", MagickFalse, RegisterAVSImage, UnregisterAVSImage },
  { "BGR", MagickFalse, RegisterBGRImage, UnregisterBGRImage },
  { "BMP", MagickFalse, RegisterBMPImage, UnregisterBMPImage },
  { "BRAILLE", MagickFalse, RegisterBRAILLEImage, UnregisterBRAILLEImage },
  { "CALS", MagickFalse, RegisterCALSImage, UnregisterCALSImage },
  { "CAPTION", MagickFalse, RegisterCAPTIONImage, UnregisterCAPTIONImage },
  { "CIN", MagickFalse, RegisterCINImage, UnregisterCINImage },
  { "CIP", MagickFalse, RegisterCIPImage, UnregisterCIPImage },
  { "CLIP", MagickFalse, RegisterCLIPImage, UnregisterCLIPImage },
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  { "CLIPBOARD", MagickFalse, RegisterCLIPBOARDImage, UnregisterCLIPBOARDImage },
#endif
  { "CMYK", MagickFalse, RegisterCMYKImage, UnregisterCMYKImage },
  { "CUT", MagickFalse, RegisterCUTImage, UnregisterCUTImage },
  { "DCM", MagickFalse, RegisterDCMImage, UnregisterDCMImage },
  { "DDS", MagickFalse, RegisterDDSImage, UnregisterDDSImage },
  { "DEBUG", MagickFalse, RegisterDEBUGImage, UnregisterDEBUGImage },
  { "DIB", MagickFalse, RegisterDIBImage, UnregisterDIBImage },
#if defined(MAGICKCORE_DJVU_DELEGATE)
  { "DJVU", MagickFalse, RegisterDJVUImage, UnregisterDJVUImage },
#endif
  { "DNG", MagickFalse, RegisterDNGImage, UnregisterDNGImage },
#if defined(MAGICKCORE_DPS_DELEGATE)
  { "DPS", MagickFalse, RegisterDPSImage, UnregisterDPSImage },
#endif
  { "DPX", MagickFalse, RegisterDPXImage, UnregisterDPXImage },
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  { "EMF", MagickFalse, RegisterEMFImage, UnregisterEMFImage },
#endif
#if defined(MAGICKCORE_TIFF_DELEGATE)
  { "EPT", MagickFalse, RegisterEPTImage, UnregisterEPTImage },
#endif
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
  { "EXR", MagickFalse, RegisterEXRImage, UnregisterEXRImage },
#endif
  { "FAX", MagickFalse, RegisterFAXImage, UnregisterFAXImage },
  { "FD", MagickFalse, RegisterFDImage, UnregisterFDImage },
  { "FITS", MagickFalse, RegisterFITSImage, UnregisterFITSImage },
#if defined(MAGICKCORE_FLIF_DELEGATE)
  { "FLIF", MagickFalse, RegisterFLIFImage, UnregisterFLIFImage },
#endif
#if defined(MAGICKCORE_FPX_DELEGATE)
  { "FPX", MagickFalse, RegisterFPXImage, UnregisterFPXImage },
#endif
  { "GIF", MagickFalse, RegisterGIFImage, UnregisterGIFImage },
  { "GRAY", MagickFalse, RegisterGRAYImage, UnregisterGRAYImage },
  { "GRADIENT", MagickFalse, RegisterGRADIENTImage, UnregisterGRADIENTImage },
  { "HALD", MagickFalse, RegisterHALDImage, UnregisterHALDImage },
  { "HDR", MagickFalse, RegisterHDRImage, UnregisterHDRImage },
#if defined(MAGICKCORE_HEIC_DELEGATE)
  { "HEIC", MagickFalse, RegisterHEICImage, UnregisterHEICImage },
#endif
  { "HISTOGRAM", MagickFalse, RegisterHISTOGRAMImage, UnregisterHISTOGRAMImage },
  { "HRZ", MagickFalse, RegisterHRZImage, UnregisterHRZImage },
  { "HTML", MagickFalse, RegisterHTMLImage, UnregisterHTMLImage },
  { "ICON", MagickFalse, RegisterICONImage, UnregisterICONImage },
  { "INFO", MagickFalse, RegisterINFOImage, UnregisterINFOImage },
  { "INLINE", MagickFalse, RegisterINLINEImage, UnregisterINLINEImage },
  { "IPL", MagickFalse, RegisterIPLImage, UnregisterIPLImage },
#if defined(MAGICKCORE_JBIG_DELEGATE)
  { "JBIG", MagickFalse, RegisterJBIGImage, UnregisterJBIGImage },
#endif
  { "JNX", MagickFalse, RegisterJNXImage, UnregisterJNXImage },
#if defined(MAGICKCORE_JPEG_DELEGATE)
  { "JPEG", MagickFalse, RegisterJPEGImage, UnregisterJPEGImage },
#endif
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  { "JP2", MagickFalse, RegisterJP2Image, UnregisterJP2Image },
#endif
  { "JSON", MagickFalse, RegisterJSONImage, UnregisterJSONImage },
  { "LABEL", MagickFalse, RegisterLABELImage, UnregisterLABELImage },
  { "MAC", MagickFalse, RegisterMACImage, UnregisterMACImage },
  { "MAGICK", MagickFalse, RegisterMAGICKImage, UnregisterMAGICKImage },
  { "MAP", MagickFalse, RegisterMAPImage, UnregisterMAPImage },
  { "MAT", MagickFalse, RegisterMATImage, UnregisterMATImage },
  { "MATTE", MagickFalse, RegisterMATTEImage, UnregisterMATTEImage },
  { "MASK", MagickFalse, RegisterMASKImage, UnregisterMASKImage },
  { "META", MagickFalse, RegisterMETAImage, UnregisterMETAImage },
  { "MIFF", MagickFalse, RegisterMIFFImage, UnregisterMIFFImage },
  { "MONO", MagickFalse, RegisterMONOImage, UnregisterMONOImage },
  { "MPC", MagickFalse, RegisterMPCImage, UnregisterMPCImage },
  { "MPEG", MagickFalse, RegisterMPEGImage, UnregisterMPEGImage },
  { "MPR", MagickFalse, RegisterMPRImage, UnregisterMPRImage },
  { "MSL", MagickFalse, RegisterMSLImage, UnregisterMSLImage },
  { "MTV", MagickFalse, RegisterMTVImage, UnregisterMTVImage },
  { "MVG", MagickFalse, RegisterMVGImage, UnregisterMVGImage },
  { "NULL", MagickFalse, RegisterNULLImage, UnregisterNULLImage },
  { "OTB", MagickFalse, RegisterOTBImage, UnregisterOTBImage },
  { "PALM", MagickFalse, RegisterPALMImage, UnregisterPALMImage },
  { "PANGO", MagickFalse, RegisterPANGOImage, UnregisterPANGOImage },
  { "PATTERN", MagickFalse, RegisterPATTERNImage, UnregisterPATTERNImage },
  { "PCD", MagickFalse, RegisterPCDImage, UnregisterPCDImage },
  { "PCL", MagickFalse, RegisterPCLImage, UnregisterPCLImage },
  { "PCX", MagickFalse, RegisterPCXImage, UnregisterPCXImage },
  { "PDB", MagickFalse, RegisterPDBImage, UnregisterPDBImage },
  { "PDF", MagickFalse, RegisterPDFImage, UnregisterPDFImage },
  { "PES", MagickFalse, RegisterPESImage, UnregisterPESImage },
  { "PGX", MagickFalse, RegisterPGXImage, UnregisterPGXImage },
  { "PICT", MagickFalse, RegisterPICTImage, UnregisterPICTImage },
  { "PIX", MagickFalse, RegisterPIXImage, UnregisterPIXImage },
  { "PLASMA", MagickFalse, RegisterPLASMAImage, UnregisterPLASMAImage },
#if defined(MAGICKCORE_PNG_DELEGATE)
  { "PNG", MagickFalse, RegisterPNGImage, UnregisterPNGImage },
#endif
  { "PNM", MagickFalse, RegisterPNMImage, UnregisterPNMImage },
  { "PS", MagickFalse, RegisterPSImage, UnregisterPSImage },
  { "PS2", MagickFalse, RegisterPS2Image, UnregisterPS2Image },
  { "PS3", MagickFalse, RegisterPS3Image, UnregisterPS3Image },
  { "PSD", MagickFalse, RegisterPSDImage, UnregisterPSDImage },
  { "PWP", MagickFalse, RegisterPWPImage, UnregisterPWPImage },
  { "RAW", MagickFalse, RegisterRAWImage, UnregisterRAWImage },
  { "RGB", MagickFalse, RegisterRGBImage, UnregisterRGBImage },
  { "RGF", MagickFalse, RegisterRGFImage, UnregisterRGFImage },
  { "RLA", MagickFalse, RegisterRLAImage, UnregisterRLAImage },
  { "RLE", MagickFalse, RegisterRLEImage, UnregisterRLEImage },
  { "SCR", MagickFalse, RegisterSCRImage, UnregisterSCRImage },
  { "SCREENSHOT", MagickFalse, RegisterSCREENSHOTImage, UnregisterSCREENSHOTImage },
  { "SCT", MagickFalse, RegisterSCTImage, UnregisterSCTImage },
  { "SFW", MagickFalse, RegisterSFWImage, UnregisterSFWImage },
  { "SGI", MagickFalse, RegisterSGIImage, UnregisterSGIImage },
  { "SIXEL", MagickFalse, RegisterSIXELImage, UnregisterSIXELImage },
  { "STEGANO", MagickFalse, RegisterSTEGANOImage, UnregisterSTEGANOImage },
  { "SUN", MagickFalse, RegisterSUNImage, UnregisterSUNImage },
  { "SVG", MagickFalse, RegisterSVGImage, UnregisterSVGImage },
  { "TGA", MagickFalse, RegisterTGAImage, UnregisterTGAImage },
  { "THUMBNAIL", MagickFalse, RegisterTHUMBNAILImage, UnregisterTHUMBNAILImage },
#if defined(MAGICKCORE_TIFF_DELEGATE)
  { "TIFF", MagickFalse, RegisterTIFFImage, UnregisterTIFFImage },
#endif
  { "TILE", MagickFalse, RegisterTILEImage, UnregisterTILEImage },
  { "TIM", MagickFalse, RegisterTIMImage, UnregisterTIMImage },
  { "TTF", MagickFalse, RegisterTTFImage, UnregisterTTFImage },
  { "TXT", MagickFalse, RegisterTXTImage, UnregisterTXTImage },
  { "UIL", MagickFalse, RegisterUILImage, UnregisterUILImage },
  { "URL", MagickFalse, RegisterURLImage, UnregisterURLImage },
  { "UYVY", MagickFalse, RegisterUYVYImage, UnregisterUYVYImage },
  { "VICAR", MagickFalse, RegisterVICARImage, UnregisterVICARImage },
  { "VID", MagickFalse, RegisterVIDImage, UnregisterVIDImage },
  { "VIFF", MagickFalse, RegisterVIFFImage, UnregisterVIFFImage },
  { "VIPS", MagickFalse, RegisterVIPSImage, UnregisterVIPSImage },
  { "WBMP", MagickFalse, RegisterWBMPImage, UnregisterWBMPImage },
#if defined(MAGICKCORE_WEBP_DELEGATE)
  { "WEBP", MagickFalse, RegisterWEBPImage, UnregisterWEBPImage },
#endif
#if defined(MAGICKCORE_WMF_DELEGATE) || defined(MAGICKCORE_WMFLITE_DELEGATE)
  { "WMF", MagickFalse, RegisterWMFImage, UnregisterWMFImage },
#endif
  { "WPG", MagickFalse, RegisterWPGImage, UnregisterWPGImage },
#if defined(MAGICKCORE_X11_DELEGATE)
  { "X", MagickFalse, RegisterXImage, UnregisterXImage },
#endif
  { "XBM", MagickFalse, RegisterXBMImage, UnregisterXBMImage },
  { "XC", MagickFalse, RegisterXCImage, UnregisterXCImage },
  { "XCF", MagickFalse, RegisterXCFImage, UnregisterXCFImage },
  { "XPM", MagickFalse, RegisterXPMImage, UnregisterXPMImage },
  { "XPS", MagickFalse, RegisterXPSImage, UnregisterXPSImage },
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  { "XTRN", MagickFalse, RegisterXTRNImage, UnregisterXTRNImage },
#endif
#if defined(MAGICKCORE_X11_DELEGATE)
  { "XWD", MagickFalse, RegisterXWDImage, UnregisterXWDImage },
#endif
  { "YCBCR", MagickFalse, RegisterYCBCRImage, UnregisterYCBCRImage },
  { "YUV", MagickFalse, RegisterYUVImage, UnregisterYUVImage },
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
  if ((*image)->debug != MagickFalse)
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
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "Invoking \"%s\" static image filter",tag);
        signature=image_filter(image,argc,argv,exception);
        if ((*image)->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"\"%s\" completes",
            tag);
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

  register const CoderInfo
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
  size_t
    extent;

  ssize_t
    i;

  extent=sizeof(MagickModules)/sizeof(MagickModules[0]);
  for (i=0; i < (ssize_t) extent; i++)
  {
    if (MagickModules[i].registered == MagickFalse)
      {
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

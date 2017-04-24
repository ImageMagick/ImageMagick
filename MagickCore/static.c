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
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
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
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/module.h"
#include "MagickCore/policy.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"

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
#if defined(MAGICKCORE_BUILD_MODULES)
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
%   R e g i s t e r S t a t i c M o d u l e s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  (void) RegisterStaticModules() statically registers all the available module
%  handlers.
%
%  The format of the RegisterStaticModules method is:
%
%      (void) RegisterStaticModules(void)
%
*/
MagickExport void RegisterStaticModules(void)
{
#if !defined(MAGICKCORE_BUILD_MODULES)
  (void) RegisterAAIImage();
  (void) RegisterARTImage();
  (void) RegisterAVSImage();
  (void) RegisterBGRImage();
  (void) RegisterBMPImage();
  (void) RegisterBRAILLEImage();
  (void) RegisterCALSImage();
  (void) RegisterCAPTIONImage();
  (void) RegisterCINImage();
  (void) RegisterCIPImage();
  (void) RegisterCLIPImage();
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  (void) RegisterCLIPBOARDImage();
#endif
  (void) RegisterCMYKImage();
  (void) RegisterCUTImage();
  (void) RegisterDCMImage();
  (void) RegisterDDSImage();
  (void) RegisterDEBUGImage();
  (void) RegisterDIBImage();
#if defined(MAGICKCORE_DJVU_DELEGATE)
  (void) RegisterDJVUImage();
#endif
  (void) RegisterDNGImage();
#if defined(MAGICKCORE_DPS_DELEGATE)
  (void) RegisterDPSImage();
#endif
  (void) RegisterDPXImage();
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  (void) RegisterEMFImage();
#endif
#if defined(MAGICKCORE_TIFF_DELEGATE)
  (void) RegisterEPTImage();
#endif
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
  (void) RegisterEXRImage();
#endif
  (void) RegisterFAXImage();
  (void) RegisterFDImage();
  (void) RegisterFITSImage();
#if defined(MAGICKCORE_FLIF_DELEGATE)
  (void) RegisterFLIFImage();
#endif
#if defined(MAGICKCORE_FPX_DELEGATE)
  (void) RegisterFPXImage();
#endif
  (void) RegisterGIFImage();
  (void) RegisterGRAYImage();
  (void) RegisterGRADIENTImage();
  (void) RegisterHALDImage();
  (void) RegisterHDRImage();
  (void) RegisterHISTOGRAMImage();
  (void) RegisterHRZImage();
  (void) RegisterHTMLImage();
  (void) RegisterICONImage();
  (void) RegisterINFOImage();
  (void) RegisterINLINEImage();
  (void) RegisterIPLImage();
#if defined(MAGICKCORE_JBIG_DELEGATE)
  (void) RegisterJBIGImage();
#endif
  (void) RegisterJNXImage();
#if defined(MAGICKCORE_JPEG_DELEGATE)
  (void) RegisterJPEGImage();
#endif
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  (void) RegisterJP2Image();
#endif
  (void) RegisterJSONImage();
  (void) RegisterLABELImage();
  (void) RegisterMACImage();
  (void) RegisterMAGICKImage();
  (void) RegisterMAPImage();
  (void) RegisterMATImage();
  (void) RegisterMATTEImage();
  (void) RegisterMASKImage();
  (void) RegisterMETAImage();
  (void) RegisterMIFFImage();
  (void) RegisterMONOImage();
  (void) RegisterMPCImage();
  (void) RegisterMPEGImage();
  (void) RegisterMPRImage();
  (void) RegisterMSLImage();
  (void) RegisterMTVImage();
  (void) RegisterMVGImage();
  (void) RegisterNULLImage();
  (void) RegisterOTBImage();
  (void) RegisterPALMImage();
  (void) RegisterPANGOImage();
  (void) RegisterPATTERNImage();
  (void) RegisterPCDImage();
  (void) RegisterPCLImage();
  (void) RegisterPCXImage();
  (void) RegisterPDBImage();
  (void) RegisterPDFImage();
  (void) RegisterPESImage();
  (void) RegisterPGXImage();
  (void) RegisterPICTImage();
  (void) RegisterPIXImage();
  (void) RegisterPLASMAImage();
#if defined(MAGICKCORE_PNG_DELEGATE)
  (void) RegisterPNGImage();
#endif
  (void) RegisterPNMImage();
  (void) RegisterPSImage();
  (void) RegisterPS2Image();
  (void) RegisterPS3Image();
  (void) RegisterPSDImage();
  (void) RegisterPWPImage();
  (void) RegisterRAWImage();
  (void) RegisterRGBImage();
  (void) RegisterRGFImage();
  (void) RegisterRLAImage();
  (void) RegisterRLEImage();
  (void) RegisterSCRImage();
  (void) RegisterSCREENSHOTImage();
  (void) RegisterSCTImage();
  (void) RegisterSFWImage();
  (void) RegisterSGIImage();
  (void) RegisterSIXELImage();
  (void) RegisterSTEGANOImage();
  (void) RegisterSUNImage();
  (void) RegisterSVGImage();
  (void) RegisterTGAImage();
  (void) RegisterTHUMBNAILImage();
#if defined(MAGICKCORE_TIFF_DELEGATE)
  (void) RegisterTIFFImage();
#endif
  (void) RegisterTILEImage();
  (void) RegisterTIMImage();
  (void) RegisterTTFImage();
  (void) RegisterTXTImage();
  (void) RegisterUILImage();
  (void) RegisterURLImage();
  (void) RegisterUYVYImage();
  (void) RegisterVICARImage();
  (void) RegisterVIDImage();
  (void) RegisterVIFFImage();
  (void) RegisterVIPSImage();
  (void) RegisterWBMPImage();
#if defined(MAGICKCORE_WEBP_DELEGATE)
  (void) RegisterWEBPImage();
#endif
#if defined(MAGICKCORE_WMF_DELEGATE) || defined(MAGICKCORE_WMFLITE_DELEGATE)
  (void) RegisterWMFImage();
#endif
  (void) RegisterWPGImage();
#if defined(MAGICKCORE_X11_DELEGATE)
  (void) RegisterXImage();
#endif
  (void) RegisterXBMImage();
  (void) RegisterXCImage();
  (void) RegisterXCFImage();
  (void) RegisterXPMImage();
  (void) RegisterXPSImage();
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  (void) RegisterXTRNImage();
#endif
#if defined(MAGICKCORE_X11_DELEGATE)
  (void) RegisterXWDImage();
#endif
  (void) RegisterYCBCRImage();
  (void) RegisterYUVImage();
#endif
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
#if !defined(MAGICKCORE_BUILD_MODULES)
  UnregisterAAIImage();
  UnregisterARTImage();
  UnregisterAVSImage();
  UnregisterBGRImage();
  UnregisterBMPImage();
  UnregisterBRAILLEImage();
  UnregisterCALSImage();
  UnregisterCAPTIONImage();
  UnregisterCINImage();
  UnregisterCIPImage();
  UnregisterCLIPImage();
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  UnregisterCLIPBOARDImage();
#endif
  UnregisterCMYKImage();
  UnregisterCUTImage();
  UnregisterDCMImage();
  UnregisterDDSImage();
  UnregisterDEBUGImage();
  UnregisterDIBImage();
#if defined(MAGICKCORE_DJVU_DELEGATE)
  UnregisterDJVUImage();
#endif
  UnregisterDNGImage();
#if defined(MAGICKCORE_DPS_DELEGATE)
  UnregisterDPSImage();
#endif
  UnregisterDPXImage();
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  UnregisterEMFImage();
#endif
#if defined(MAGICKCORE_TIFF_DELEGATE)
  UnregisterEPTImage();
#endif
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
  UnregisterEXRImage();
#endif
  UnregisterFAXImage();
  UnregisterFDImage();
  UnregisterFITSImage();
#if defined(MAGICKCORE_FLIF_DELEGATE)
  UnregisterFLIFImage();
#endif
#if defined(MAGICKCORE_FPX_DELEGATE)
  UnregisterFPXImage();
#endif
  UnregisterGIFImage();
  UnregisterGRAYImage();
  UnregisterGRADIENTImage();
  UnregisterHALDImage();
  UnregisterHDRImage();
  UnregisterHISTOGRAMImage();
  UnregisterHRZImage();
  UnregisterHTMLImage();
  UnregisterICONImage();
  UnregisterINFOImage();
  UnregisterINLINEImage();
  UnregisterIPLImage();
#if defined(MAGICKCORE_JBIG_DELEGATE)
  UnregisterJBIGImage();
#endif
  UnregisterJNXImage();
#if defined(MAGICKCORE_JPEG_DELEGATE)
  UnregisterJPEGImage();
#endif
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  UnregisterJP2Image();
#endif
  UnregisterJSONImage();
  UnregisterLABELImage();
  UnregisterMACImage();
  UnregisterMAGICKImage();
  UnregisterMAPImage();
  UnregisterMASKImage();
  UnregisterMATImage();
  UnregisterMATTEImage();
  UnregisterMETAImage();
  UnregisterMIFFImage();
  UnregisterMONOImage();
  UnregisterMPCImage();
  UnregisterMPEGImage();
  UnregisterMPRImage();
  UnregisterMSLImage();
  UnregisterMTVImage();
  UnregisterMVGImage();
  UnregisterNULLImage();
  UnregisterOTBImage();
  UnregisterPALMImage();
  UnregisterPANGOImage();
  UnregisterPATTERNImage();
  UnregisterPCDImage();
  UnregisterPCLImage();
  UnregisterPCXImage();
  UnregisterPDBImage();
  UnregisterPDFImage();
  UnregisterPESImage();
  UnregisterPGXImage();
  UnregisterPICTImage();
  UnregisterPIXImage();
  UnregisterPLASMAImage();
#if defined(MAGICKCORE_PNG_DELEGATE)
  UnregisterPNGImage();
#endif
  UnregisterPNMImage();
  UnregisterPSImage();
  UnregisterPS2Image();
  UnregisterPS3Image();
  UnregisterPSDImage();
  UnregisterPWPImage();
  UnregisterRAWImage();
  UnregisterRGBImage();
  UnregisterRGFImage();
  UnregisterRLAImage();
  UnregisterRLEImage();
  UnregisterSCRImage();
  UnregisterSCREENSHOTImage();
  UnregisterSCTImage();
  UnregisterSFWImage();
  UnregisterSGIImage();
  UnregisterSIXELImage();
  UnregisterSTEGANOImage();
  UnregisterSUNImage();
  UnregisterSVGImage();
  UnregisterTGAImage();
  UnregisterTHUMBNAILImage();
#if defined(MAGICKCORE_TIFF_DELEGATE)
  UnregisterTIFFImage();
#endif
  UnregisterTILEImage();
  UnregisterTIMImage();
  UnregisterTTFImage();
  UnregisterTXTImage();
  UnregisterUILImage();
  UnregisterURLImage();
  UnregisterUYVYImage();
  UnregisterVICARImage();
  UnregisterVIDImage();
  UnregisterVIFFImage();
  UnregisterVIPSImage();
  UnregisterWBMPImage();
#if defined(MAGICKCORE_WEBP_DELEGATE)
  UnregisterWEBPImage();
#endif
#if defined(MAGICKCORE_WMF_DELEGATE) || defined(MAGICKCORE_WMFLITE_DELEGATE)
  UnregisterWMFImage();
#endif
  UnregisterWPGImage();
#if defined(MAGICKCORE_X11_DELEGATE)
  UnregisterXImage();
#endif
  UnregisterXBMImage();
  UnregisterXCImage();
  UnregisterXCFImage();
  UnregisterXPMImage();
  UnregisterXPSImage();
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  UnregisterXTRNImage();
#endif
#if defined(MAGICKCORE_X11_DELEGATE)
  UnregisterXWDImage();
#endif
  UnregisterYCBCRImage();
  UnregisterYUVImage();
#endif
}

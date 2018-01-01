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
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/module.h"
#include "MagickCore/policy.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#if !defined(MAGICKCORE_BUILD_MODULES)
static const struct
{
  char
    *name;

  size_t
    (*register_module)(void);

  void
    (*unregister_module)(void);
} MagickModules[] =
{
  { "AAI", RegisterAAIImage, UnregisterAAIImage },
  { "ART", RegisterARTImage, UnregisterARTImage },
  { "AVS", RegisterAVSImage, UnregisterAVSImage },
  { "BGR", RegisterBGRImage, UnregisterBGRImage },
  { "BMP", RegisterBMPImage, UnregisterBMPImage },
  { "BRAILLE", RegisterBRAILLEImage, UnregisterBRAILLEImage },
  { "CALS", RegisterCALSImage, UnregisterCALSImage },
  { "CAPTION", RegisterCAPTIONImage, UnregisterCAPTIONImage },
  { "CIN", RegisterCINImage, UnregisterCINImage },
  { "CIP", RegisterCIPImage, UnregisterCIPImage },
  { "CLIP", RegisterCLIPImage, UnregisterCLIPImage },
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  { "CLIPBOARD", RegisterCLIPBOARDImage, UnregisterCLIPBOARDImage },
#endif
  { "CMYK", RegisterCMYKImage, UnregisterCMYKImage },
  { "CUT", RegisterCUTImage, UnregisterCUTImage },
  { "DCM", RegisterDCMImage, UnregisterDCMImage },
  { "DDS", RegisterDDSImage, UnregisterDDSImage },
  { "DEBUG", RegisterDEBUGImage, UnregisterDEBUGImage },
  { "DIB", RegisterDIBImage, UnregisterDIBImage },
#if defined(MAGICKCORE_DJVU_DELEGATE)
  { "DJVU", RegisterDJVUImage, UnregisterDJVUImage },
#endif
  { "DNG", RegisterDNGImage, UnregisterDNGImage },
#if defined(MAGICKCORE_DPS_DELEGATE)
  { "DPS", RegisterDPSImage, UnregisterDPSImage },
#endif
  { "DPX", RegisterDPXImage, UnregisterDPXImage },
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  { "EMF", RegisterEMFImage, UnregisterEMFImage },
#endif
#if defined(MAGICKCORE_TIFF_DELEGATE)
  { "EPT", RegisterEPTImage, UnregisterEPTImage },
#endif
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
  { "EXR", RegisterEXRImage, UnregisterEXRImage },
#endif
  { "FAX", RegisterFAXImage, UnregisterFAXImage },
  { "FD", RegisterFDImage, UnregisterFDImage },
  { "FITS", RegisterFITSImage, UnregisterFITSImage },
#if defined(MAGICKCORE_FLIF_DELEGATE)
  { "FLIF", RegisterFLIFImage, UnregisterFLIFImage },
#endif
#if defined(MAGICKCORE_FPX_DELEGATE)
  { "FPX", RegisterFPXImage, UnregisterFPXImage },
#endif
  { "GIF", RegisterGIFImage, UnregisterGIFImage },
  { "GRAY", RegisterGRAYImage, UnregisterGRAYImage },
  { "GRADIENT", RegisterGRADIENTImage, UnregisterGRADIENTImage },
  { "HALD", RegisterHALDImage, UnregisterHALDImage },
  { "HDR", RegisterHDRImage, UnregisterHDRImage },
#if defined(MAGICKCORE_HEIC_DELEGATE)
  { "HEIC", RegisterHEICImage, UnregisterHEICImage },
#endif
  { "HISTOGRAM", RegisterHISTOGRAMImage, UnregisterHISTOGRAMImage },
  { "HRZ", RegisterHRZImage, UnregisterHRZImage },
  { "HTML", RegisterHTMLImage, UnregisterHTMLImage },
  { "ICON", RegisterICONImage, UnregisterICONImage },
  { "INFO", RegisterINFOImage, UnregisterINFOImage },
  { "INLINE", RegisterINLINEImage, UnregisterINLINEImage },
  { "IPL", RegisterIPLImage, UnregisterIPLImage },
#if defined(MAGICKCORE_JBIG_DELEGATE)
  { "JBIG", RegisterJBIGImage, UnregisterJBIGImage },
#endif
  { "JNX", RegisterJNXImage, UnregisterJNXImage },
#if defined(MAGICKCORE_JPEG_DELEGATE)
  { "JPEG", RegisterJPEGImage, UnregisterJPEGImage },
#endif
#if defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  { "JP2", RegisterJP2Image, UnregisterJP2Image },
#endif
  { "JSON", RegisterJSONImage, UnregisterJSONImage },
  { "LABEL", RegisterLABELImage, UnregisterLABELImage },
  { "MAC", RegisterMACImage, UnregisterMACImage },
  { "MAGICK", RegisterMAGICKImage, UnregisterMAGICKImage },
  { "MAP", RegisterMAPImage, UnregisterMAPImage },
  { "MAT", RegisterMATImage, UnregisterMATImage },
  { "MATTE", RegisterMATTEImage, UnregisterMATTEImage },
  { "MASK", RegisterMASKImage, UnregisterMASKImage },
  { "META", RegisterMETAImage, UnregisterMETAImage },
  { "MIFF", RegisterMIFFImage, UnregisterMIFFImage },
  { "MONO", RegisterMONOImage, UnregisterMONOImage },
  { "MPC", RegisterMPCImage, UnregisterMPCImage },
  { "MPEG", RegisterMPEGImage, UnregisterMPEGImage },
  { "MPR", RegisterMPRImage, UnregisterMPRImage },
  { "MSL", RegisterMSLImage, UnregisterMSLImage },
  { "MTV", RegisterMTVImage, UnregisterMTVImage },
  { "MVG", RegisterMVGImage, UnregisterMVGImage },
  { "NULL", RegisterNULLImage, UnregisterNULLImage },
  { "OTB", RegisterOTBImage, UnregisterOTBImage },
  { "PALM", RegisterPALMImage, UnregisterPALMImage },
  { "PANGO", RegisterPANGOImage, UnregisterPANGOImage },
  { "PATTERN", RegisterPATTERNImage, UnregisterPATTERNImage },
  { "PCD", RegisterPCDImage, UnregisterPCDImage },
  { "PCL", RegisterPCLImage, UnregisterPCLImage },
  { "PCX", RegisterPCXImage, UnregisterPCXImage },
  { "PDB", RegisterPDBImage, UnregisterPDBImage },
  { "PDF", RegisterPDFImage, UnregisterPDFImage },
  { "PES", RegisterPESImage, UnregisterPESImage },
  { "PGX", RegisterPGXImage, UnregisterPGXImage },
  { "PICT", RegisterPICTImage, UnregisterPICTImage },
  { "PIX", RegisterPIXImage, UnregisterPIXImage },
  { "PLASMA", RegisterPLASMAImage, UnregisterPLASMAImage },
#if defined(MAGICKCORE_PNG_DELEGATE)
  { "PNG", RegisterPNGImage, UnregisterPNGImage },
#endif
  { "PNM", RegisterPNMImage, UnregisterPNMImage },
  { "PS", RegisterPSImage, UnregisterPSImage },
  { "PS2", RegisterPS2Image, UnregisterPS2Image },
  { "PS3", RegisterPS3Image, UnregisterPS3Image },
  { "PSD", RegisterPSDImage, UnregisterPSDImage },
  { "PWP", RegisterPWPImage, UnregisterPWPImage },
  { "RAW", RegisterRAWImage, UnregisterRAWImage },
  { "RGB", RegisterRGBImage, UnregisterRGBImage },
  { "RGF", RegisterRGFImage, UnregisterRGFImage },
  { "RLA", RegisterRLAImage, UnregisterRLAImage },
  { "RLE", RegisterRLEImage, UnregisterRLEImage },
  { "SCR", RegisterSCRImage, UnregisterSCRImage },
  { "SCREENSHOT", RegisterSCREENSHOTImage, UnregisterSCREENSHOTImage },
  { "SCT", RegisterSCTImage, UnregisterSCTImage },
  { "SFW", RegisterSFWImage, UnregisterSFWImage },
  { "SGI", RegisterSGIImage, UnregisterSGIImage },
  { "SIXEL", RegisterSIXELImage, UnregisterSIXELImage },
  { "STEGANO", RegisterSTEGANOImage, UnregisterSTEGANOImage },
  { "SUN", RegisterSUNImage, UnregisterSUNImage },
  { "SVG", RegisterSVGImage, UnregisterSVGImage },
  { "TGA", RegisterTGAImage, UnregisterTGAImage },
  { "THUMBNAIL", RegisterTHUMBNAILImage, UnregisterTHUMBNAILImage },
#if defined(MAGICKCORE_TIFF_DELEGATE)
  { "TIFF", RegisterTIFFImage, UnregisterTIFFImage },
#endif
  { "TILE", RegisterTILEImage, UnregisterTILEImage },
  { "TIM", RegisterTIMImage, UnregisterTIMImage },
  { "TTF", RegisterTTFImage, UnregisterTTFImage },
  { "TXT", RegisterTXTImage, UnregisterTXTImage },
  { "UIL", RegisterUILImage, UnregisterUILImage },
  { "URL", RegisterURLImage, UnregisterURLImage },
  { "UYVY", RegisterUYVYImage, UnregisterUYVYImage },
  { "VICAR", RegisterVICARImage, UnregisterVICARImage },
  { "VID", RegisterVIDImage, UnregisterVIDImage },
  { "VIFF", RegisterVIFFImage, UnregisterVIFFImage },
  { "VIPS", RegisterVIPSImage, UnregisterVIPSImage },
  { "WBMP", RegisterWBMPImage, UnregisterWBMPImage },
#if defined(MAGICKCORE_WEBP_DELEGATE)
  { "WEBP", RegisterWEBPImage, UnregisterWEBPImage },
#endif
#if defined(MAGICKCORE_WMF_DELEGATE) || defined(MAGICKCORE_WMFLITE_DELEGATE)
  { "WMF", RegisterWMFImage, UnregisterWMFImage },
#endif
  { "WPG", RegisterWPGImage, UnregisterWPGImage },
#if defined(MAGICKCORE_X11_DELEGATE)
  { "X", RegisterXImage, UnregisterXImage },
#endif
  { "XBM", RegisterXBMImage, UnregisterXBMImage },
  { "XC", RegisterXCImage, UnregisterXCImage },
  { "XCF", RegisterXCFImage, UnregisterXCFImage },
  { "XPM", RegisterXPMImage, UnregisterXPMImage },
  { "XPS", RegisterXPSImage, UnregisterXPSImage },
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  { "XTRN", RegisterXTRNImage, UnregisterXTRNImage },
#endif
#if defined(MAGICKCORE_X11_DELEGATE)
  { "XWD", RegisterXWDImage, UnregisterXWDImage },
#endif
  { "YCBCR", RegisterYCBCRImage, UnregisterYCBCRImage },
  { "YUV", RegisterYUVImage, UnregisterYUVImage }
};
#endif

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
  ssize_t
    i;

  for (i=0; i < (ssize_t) (sizeof(MagickModules)/sizeof(MagickModules[0]));i++)
    (void) (MagickModules[i].register_module)();
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
  ssize_t
    i;

  for (i=0; i < (ssize_t) (sizeof(MagickModules)/sizeof(MagickModules[0]));i++)
    (MagickModules[i].unregister_module)();
#endif
}

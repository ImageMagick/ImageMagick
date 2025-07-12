/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include "coders/coders-private.h"

AddMagickCoder(AAI)
AddMagickCoder(ART)
AddMagickCoder(ASHLAR)
AddMagickCoder(AVS)
AddMagickCoder(BAYER)
AddMagickCoder(BGR)
AddMagickCoder(BMP)
AddMagickCoder(BRAILLE)
AddMagickCoder(CALS)
AddMagickCoder(CAPTION)
AddMagickCoder(CIN)
AddMagickCoder(CIP)
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  AddMagickCoder(CLIPBOARD)
#endif
AddMagickCoder(CLIP)
AddMagickCoder(CMYK)
AddMagickCoder(CUBE)
AddMagickCoder(CUT)
AddMagickCoder(DCM)
AddMagickCoder(DDS)
AddMagickCoder(DEBUG)
AddMagickCoder(DIB)
#if defined(MAGICKCORE_DJVU_DELEGATE)
  AddMagickCoder(DJVU)
#endif
AddMagickCoder(DNG)
#if defined(MAGICKCORE_GVC_DELEGATE)
  AddMagickCoder(DOT)
#endif
#if defined(MAGICKCORE_DPS_DELEGATE)
  AddMagickCoder(DPS)
#endif
AddMagickCoder(DPX)
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  AddMagickCoder(EMF)
#endif
#if defined(MAGICKCORE_TIFF_DELEGATE)
  AddMagickCoder(EPT)
#endif
#if defined(MAGICKCORE_OPENEXR_DELEGATE)
  AddMagickCoder(EXR)
#endif
AddMagickCoder(FARBFELD)
AddMagickCoder(FAX)
AddMagickCoder(FITS)
AddMagickCoder(FL32)
#if defined(MAGICKCORE_FLIF_DELEGATE)
  AddMagickCoder(FLIF)
#endif
#if defined(MAGICKCORE_FPX_DELEGATE)
  AddMagickCoder(FPX)
#endif
AddMagickCoder(FTXT)
AddMagickCoder(GIF)
AddMagickCoder(GRADIENT)
AddMagickCoder(GRAY)
AddMagickCoder(HALD)
AddMagickCoder(HDR)
#if defined(MAGICKCORE_HEIC_DELEGATE)
  AddMagickCoder(HEIC)
#endif
AddMagickCoder(HISTOGRAM)
AddMagickCoder(HRZ)
AddMagickCoder(HTML)
AddMagickCoder(ICON)
AddMagickCoder(INFO)
AddMagickCoder(INLINE)
AddMagickCoder(IPL)
#if defined(MAGICKCORE_JBIG_DELEGATE)
  AddMagickCoder(JBIG)
#endif
AddMagickCoder(JNX)
#if defined(MAGICKCORE_JP2_DELEGATE) || defined(MAGICKCORE_LIBOPENJP2_DELEGATE)
  AddMagickCoder(JP2)
#endif
#if defined(MAGICKCORE_JPEG_DELEGATE)
  AddMagickCoder(JPEG)
#endif
AddMagickCoder(JSON)
#if defined(MAGICKCORE_JXL_DELEGATE)
  AddMagickCoder(JXL)
#endif
AddMagickCoder(LABEL)
AddMagickCoder(MAC)
AddMagickCoder(MAGICK)
AddMagickCoder(MAP)
AddMagickCoder(MASK)
AddMagickCoder(MAT)
AddMagickCoder(MATTE)
AddMagickCoder(META)
AddMagickCoder(MIFF)
AddMagickCoder(MONO)
AddMagickCoder(MPC)
AddMagickCoder(MPR)
AddMagickCoder(MSL)
AddMagickCoder(MTV)
AddMagickCoder(MVG)
AddMagickCoder(NULL)
AddMagickCoder(ORA)
AddMagickCoder(OTB)
AddMagickCoder(PALM)
AddMagickCoder(PANGO)
AddMagickCoder(PATTERN)
AddMagickCoder(PCD)
AddMagickCoder(PCL)
AddMagickCoder(PCX)
AddMagickCoder(PDB)
AddMagickCoder(PDF)
AddMagickCoder(PES)
AddMagickCoder(PGX)
AddMagickCoder(PICT)
AddMagickCoder(PIX)
AddMagickCoder(PLASMA)
#if defined(MAGICKCORE_PNG_DELEGATE)
  AddMagickCoder(PNG)
#endif
AddMagickCoder(PNM)
AddMagickCoder(PS2)
AddMagickCoder(PS3)
AddMagickCoder(PS)
AddMagickCoder(PSD)
AddMagickCoder(PWP)
AddMagickCoder(QOI)
AddMagickCoder(RAW)
AddMagickCoder(RGB)
AddMagickCoder(RGF)
AddMagickCoder(RLA)
AddMagickCoder(RLE)
AddMagickCoder(SCR)
AddMagickCoder(SCREENSHOT)
AddMagickCoder(SCT)
AddMagickCoder(SF3)
AddMagickCoder(SFW)
AddMagickCoder(SGI)
AddMagickCoder(SIXEL)
AddMagickCoder(STEGANO)
AddMagickCoder(STRIMG)
AddMagickCoder(SUN)
AddMagickCoder(SVG)
AddMagickCoder(TGA)
AddMagickCoder(THUMBNAIL)
#if defined(MAGICKCORE_TIFF_DELEGATE)
  AddMagickCoder(TIFF)
#endif
AddMagickCoder(TILE)
AddMagickCoder(TIM)
AddMagickCoder(TIM2)
#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  AddMagickCoder(TTF)
#endif
AddMagickCoder(TXT)
#if defined(MAGICKCORE_UHDR_DELEGATE)
  AddMagickCoder(UHDR)
#endif
AddMagickCoder(UIL)
AddMagickCoder(URL)
AddMagickCoder(UYVY)
AddMagickCoder(VICAR)
AddMagickCoder(VID)
AddMagickCoder(VIDEO)
AddMagickCoder(VIFF)
AddMagickCoder(VIPS)
AddMagickCoder(WBMP)
#if defined(MAGICKCORE_WEBP_DELEGATE)
  AddMagickCoder(WEBP)
#endif
#if defined(MAGICKCORE_WMF_DELEGATE) || defined(MAGICKCORE_WMFLITE_DELEGATE)
  AddMagickCoder(WMF)
#endif
AddMagickCoder(WPG)
AddMagickCoder(XBM)
#if defined(MAGICKCORE_X11_DELEGATE)
  AddMagickCoder(X)
#endif
AddMagickCoder(XC)
AddMagickCoder(XCF)
AddMagickCoder(XPM)
AddMagickCoder(XPS)
#if defined(MAGICKCORE_X11_DELEGATE)
  AddMagickCoder(XWD)
#endif
AddMagickCoder(YAML)
AddMagickCoder(YCBCR)
AddMagickCoder(YUV)
